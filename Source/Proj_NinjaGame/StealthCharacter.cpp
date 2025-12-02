// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthCharacter.h"

#include "EngineUtils.h"
#include "SoundUtility.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "KunaiWeapon.h"
#include "PlayerUseInterface.h"
#include "MeleeAIController.h"
#include "MeleeWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Proj_NinjaGame.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "Components/BoxComponent.h"
#include "MeleeEnemy.h"
#include "SmokeBombWeapon.h"
#include "ThrowableObject.h"
#include "ThrowingMarker.h"
#include "Navigation/PathFollowingComponent.h"


// Sets default values
AStealthCharacter::AStealthCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), "ROOT");
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	
	PlayerMeleeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerMeleeBox"));
	PlayerMeleeBox->SetupAttachment(FirstPersonCameraComponent);
	
	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = SneakWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

	CurrentStamina = MaxStamina;
}

void AStealthCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Spara input, för sprint
	MoveInputRight = MovementVector.X;
	MoveInputForward = MovementVector.Y;

	if (MovementVector.Y > 0.0f)
	{
		bMovingForward = true;
	}
	// pass the axis values to the move input
	Move(MovementVector.X, MovementVector.Y);
}

void AStealthCharacter::EndMoveInput()
{
	bMovingForward = false;

	MoveInputForward = 0.f;
	MoveInputRight = 0.f;
}

void AStealthCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	Look(LookAxisVector.X, LookAxisVector.Y);
}

void AStealthCharacter::Look(float Yaw, float Pitch)
{
	if (GetController())
	{
		
		if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
		{
			Yaw *= GI->SensitivityScale;
			Pitch *= GI->SensitivityScale;
		}
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AStealthCharacter::Move(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		if (CurrentMovementState == EPlayerMovementState::Climb && GetCharacterMovement()->MovementMode == MOVE_Flying)
		{
			AddMovementInput(GetActorRightVector(), Right);
			AddMovementInput(GetActorUpVector(), Forward);
			AddMovementInput(GetActorForwardVector(), Forward);
		}
		else
		{
			AddMovementInput(GetActorRightVector(), Right);
			AddMovementInput(GetActorForwardVector(), Forward);
		}
		
		float NoiseLevel = 1.0f;
		
		switch (CurrentMovementState)
		{
		case EPlayerMovementState::Crouch:
			NoiseLevel *= SneakNoiseMultiplier;
			break;
		case EPlayerMovementState::Run:
			NoiseLevel *= SprintNoiseMultiplier;
			break;
		default:
			break;
		}

		USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel); 
	}
}

bool AStealthCharacter::CanJumpInternal_Implementation() const
{
	if (IsCrouched() && JumpCurrentCount < JumpMaxCount)
	{
		return true;
	}
	return JumpIsAllowedInternal();
}

void AStealthCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
	bHoldingJump = true;

	/*UE_LOG(LogTemp, Warning, TEXT("CurrentMovementState: %s"), *MovementStateToString(CurrentMovementState));
	UE_LOG(LogTemp, Warning, TEXT("RememberedJumpState: %s"), *MovementStateToString(RememberedJumpState));*/
}

void AStealthCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
	
	bHoldingJump = false;
}

void AStealthCharacter::Attack()
{
	if(bIsHiding == true) return;
	
	if (bIsAiming)
	{
		UE_LOG(LogTemp, Display, TEXT("Ranged attack"));
		//ThrowingWeapon
		if (HeldThrowableWeapon)
		{
			if (bCanThrow)
			{
				if (CurrentInteractState == EPlayerInteractState::None || CurrentInteractState == EPlayerInteractState::Throw || CurrentInteractState == EPlayerInteractState::Interact)
				{
					CurrentInteractState = EPlayerInteractState::Throw;
				}
			}
		}
	}
	else
	{
		//MeleeWeapon
		if (CurrentMeleeWeapon->bCanMeleeAttack)
		{
			if (CurrentMovementState != EPlayerMovementState::Run && CurrentMovementState != EPlayerMovementState::Climb)
			{
				if (CurrentInteractState == EPlayerInteractState::None || CurrentInteractState == EPlayerInteractState::Attack || CurrentInteractState == EPlayerInteractState::Interact)
				{
					CurrentInteractState = EPlayerInteractState::Attack;
					if (CurrentMeleeWeapon->SwingSound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), CurrentMeleeWeapon->SwingSound, GetActorLocation());
					}
					UE_LOG(LogTemp, Display, TEXT("Melee attack"));
					TArray<AActor*> HitActors;
					PlayerMeleeBox->GetOverlappingActors(HitActors);
					for (auto HitActor : HitActors)
					{
						if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitActor))
						{
							if (Enemy->bCanBeAssassinated && !Enemy->CanSeePlayer())
							{
								CurrentMeleeWeapon->bAssassinatingEnemy = true;
								CurrentMeleeWeapon->bCanMeleeAttack = false;
								break;
							}
						}
					}
					
					if (!CurrentMeleeWeapon->bAssassinatingEnemy)
					{
						CurrentMeleeWeapon->bMeleeAttacking = true;
						CurrentMeleeWeapon->bCanMeleeAttack = false;
					}
				}
			}
		}
	}
}
void AStealthCharacter::StartThrow()
{
	if(bIsHiding == true) return;
	
	if (ThrowSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ThrowSound, GetActorLocation());
	}
	HeldThrowableWeapon->Throw(this);
	bCanThrow = true;
}

void AStealthCharacter::StopThrow()
{
	if (CurrentInteractState == EPlayerInteractState::Throw)
	{
		CurrentInteractState = EPlayerInteractState::None;
	}
}

void AStealthCharacter::ChangeWeapon()
{
	if (bIsHiding || bIsAiming || AmountOfOwnWeapon <= 0)
		return;
	
	if (HeldThrowableWeapon->bIsOwnThrowWeapon)
	{
		if (LastHeldWeapon != nullptr)
		{
			EquipThrowWeapon(LastHeldWeapon);
		}
	}
	else
	{
		if (CurrentOwnThrowWeapon != nullptr)
		{
			EquipThrowWeapon(CurrentOwnThrowWeapon);
		}
	}
}

void AStealthCharacter::EquipThrowWeapon(TSubclassOf<AThrowableWeapon> EquipWeapon)
{
	
	if (HeldThrowableWeapon)
	{
		HeldThrowableWeapon->Destroy();
	}
	
	HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(EquipWeapon);

	HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
}

void AStealthCharacter::DropWeapon()
{
	if (bIsHiding || bIsAiming || !HeldThrowableWeapon)
		return;

	HeldThrowableWeapon->Drop(this);
}

void AStealthCharacter::AimStart()
{
	if(bIsHiding == true) return;
	
	if (HeldThrowableWeapon)
	{
		GetWorld()->GetTimerManager().ClearTimer(AimEndTimer);
		bIsAiming = true;
		
		if (CurrentMovementState == EPlayerMovementState::Run ) 
		{
			StopSprint();
		}
		
		if (MarkerClass && !SpawnedMarker)
		{
			FVector SpawnLoc = GetActorLocation();
			FRotator SpawnRot = FRotator::ZeroRotator;
			SpawnedMarker = GetWorld()->SpawnActor<AThrowingMarker>(MarkerClass, SpawnLoc, SpawnRot);
			UpdateSpawnMarkerMesh();
			
			HeldThrowableWeapon->ThrownWeaponObject;
		}
	}
}

void AStealthCharacter::AimEndAction()
{
	if (CurrentMovementState == EPlayerMovementState::Run)
	{
		AimEndFunction();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(AimEndTimer, this, &AStealthCharacter::AimEndFunction, AimEndTimerSeconds, false);
	}
}

void AStealthCharacter::AimEndFunction()
{
	bIsAiming = false;
	
	// Clean up
	if (SpawnedMarker)
	{
		SpawnedMarker->Destroy();
		SpawnedMarker = nullptr;
	}
}

void AStealthCharacter::UpdateSpawnMarkerMesh()
{
	if (HeldThrowableWeapon->ThrownWeaponObject && HeldThrowableWeapon && SpawnedMarker)
	{
		AThrowableObject* DefaultActor = HeldThrowableWeapon->ThrownWeaponObject->GetDefaultObject<AThrowableObject>();
				
		if (DefaultActor->StaticMeshComponent->GetStaticMesh())
		{
			SpawnedMarker->SetMarkerScale(DefaultActor->StaticMeshComponent->GetRelativeScale3D());
			SpawnedMarker->SetMarkerMesh(DefaultActor->StaticMeshComponent->GetStaticMesh());
		}
	}
}

void AStealthCharacter::UpdateProjectilePrediction()
{
	if (!FirstPersonCameraComponent)
        return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
	
    FPredictProjectilePathParams PredictParams;
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector() * CameraForwardMultiplier;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		End = HitResult.Location;
	}
	
    PredictParams.StartLocation = End;
    PredictParams.LaunchVelocity = FirstPersonCameraComponent->GetForwardVector() * HeldThrowableWeapon->ThrowSpeed;
	if (UStaticMesh* WeaponMesh = HeldThrowableWeapon->StaticMeshComponent->GetStaticMesh())
	{
		FVector BoundsExtent = WeaponMesh->GetBounds().BoxExtent;
		//Should be better if I could predict the mesh instead of just the absolutes
		PredictParams.ProjectileRadius = BoundsExtent.GetAbsMax();
	}
	else
	{
		PredictParams.ProjectileRadius = 5.f;
	}
    PredictParams.MaxSimTime = 2.f; // Max simulated tim of travel per second
    PredictParams.bTraceWithCollision = true; //If hit something
    PredictParams.SimFrequency = 15.f; //How many checks per second
    PredictParams.TraceChannel = ECC_Camera;
    PredictParams.ActorsToIgnore.Add(this);

    FPredictProjectilePathResult PredictResult;
    UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(PredictResult.HitResult.GetActor()))
	{
		if (PredictResult.HitResult.Component == Enemy->GetHeadComponent())
		{
			SpawnedMarker->SetHeadMaterial();
		}
		else
		{
			SpawnedMarker->SetEnemyMaterial();
		}
	}
	else
	{
		SpawnedMarker->SetGroundMaterial();
	}

	SpawnedMarker->SetActorRotation(FirstPersonCameraComponent->GetComponentRotation());
	SpawnedMarker->SetActorLocation(PredictResult.HitResult.Location);
}

void AStealthCharacter::UpdateStaminaStart(float InStamina)
{
	UpdateStaminaAmount = InStamina;
	if (UpdateStaminaAmount >= 0)
	{
		//Regain Stamina
		GetWorld()->GetTimerManager().SetTimer(StaminaTimer, this, &AStealthCharacter::UpdateStaminaLoop, StaminaRefreshRate, true, RegainStaminaStartTime);
	}
	else
	{
		//Lose Stamina
		GetWorld()->GetTimerManager().SetTimer(StaminaTimer, this, &AStealthCharacter::UpdateStaminaLoop, StaminaRefreshRate, true);
	}
}

void AStealthCharacter::UpdateStaminaLoop()
{
	CurrentStamina += UpdateStaminaAmount;

	if (CurrentStamina < 0)
	{
		CurrentStamina = 0;
	}
	else if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}

	if (CurrentStamina == 0)
	{
		if (CurrentMovementState == EPlayerMovementState::Run)
		{
			StopSprint();
		}
		UpdateStaminaStart(RegainStaminaAmount);
	}
	else if (CurrentStamina == MaxStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaTimer);
	}
}


// Called when the game starts or when spawned
void AStealthCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	
	if (FirstPersonCameraComponent)
	{
		NormalFOV = FirstPersonCameraComponent->FieldOfView;
	}
	
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GI->CurrentOwnThrowWeapon)
		{
			CurrentOwnThrowWeapon = GI->CurrentOwnThrowWeapon;
		}
	}
	
	if (CurrentOwnThrowWeapon && AmountOfOwnWeapon > 1)
	{
		EquipThrowWeapon(CurrentOwnThrowWeapon);
	}
	else
	{
		//If we have nothing in the GameInstance
		if (KunaiWeapon && AmountOfOwnWeapon > 1)
		{
			CurrentOwnThrowWeapon = KunaiWeapon;
			if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
			{
				GI->CurrentOwnThrowWeapon = CurrentOwnThrowWeapon;
				GI->CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::Kunai;
			}
			
			EquipThrowWeapon(CurrentOwnThrowWeapon);
		}
	}
	
	if (MeleeWeapon)
	{
		if (CurrentMeleeWeapon)
		{
			HeldThrowableWeapon->Destroy();
		}
		CurrentMeleeWeapon = GetWorld()->SpawnActor<AMeleeWeapon>(MeleeWeapon);
		CurrentMeleeWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
	}

	// För HideSpot
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			HideMinPitch = Cam->ViewPitchMin;
			HideMaxPitch = Cam->ViewPitchMax;
			HideMinYaw = Cam->ViewYawMin;
			HideMaxYaw = Cam->ViewYawMax;
		}
	}

	MaxAmountOfOwnWeapon = AmountOfOwnWeapon;
	
	PlayerMeleeBox->OnComponentBeginOverlap.AddDynamic(this, &AStealthCharacter::OnMeleeBoxBeginOverlap);
	PlayerMeleeBox->OnComponentEndOverlap.AddDynamic(this, &AStealthCharacter::OnMeleeBoxEndOverlap);
}

void AStealthCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	UE_LOG(LogTemp, Display, TEXT("Landed"));
	
	if (CurrentMovementState == EPlayerMovementState::Climb)
	{
		CurrentMovementState = RememberedClimbState;

		if (CurrentMovementState == EPlayerMovementState::Walk)
		{
			if (CanUnCrouch())
			{
				if (bClimbCapsuleShrunk)
				{
					UnCrouch(false);
				}
			}
			else
			{
				ToggleSneak();
			}
		}

		bClimbCapsuleShrunk = false;
		bHitLedge = false;
	}
	
	float NoiseLevel;
		
	if (CurrentMovementState == EPlayerMovementState::Crouch)
	{
		NoiseLevel = 1.0f * SneakNoiseMultiplier;
	}
	else
	{
		NoiseLevel = 2.0f; // ändrade från 1.5
	}
	USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForUse();
	CheckForCanAssassinate();
	
	//Gives a reticle for where the throwable object will go
	if (bIsAiming && HeldThrowableWeapon)
	{
		UpdateProjectilePrediction();
	}

	//Climbing
	Climb();


	// Sluta springa ifall spelaren inte har någon movement input
	if (CurrentMovementState == EPlayerMovementState::Run)
	{
		if (FMath::IsNearlyZero(MoveInputForward) && FMath::IsNearlyZero(MoveInputRight))
		{
			StopSprint();
		}
	}

	if(bIsHiding)
	{
		StopSprint();
	}

	if (FirstPersonCameraComponent)
	{
		// Ändrar FOV när spelaren springer 
		float TargetFOV = (CurrentMovementState == EPlayerMovementState::Run) ? SprintFOV : NormalFOV;
		if (bIsAiming)
		{
			TargetFOV = (bIsAiming) ? AimFOV : NormalFOV;
		}
		
		float NewFOV = FMath::FInterpTo(
			FirstPersonCameraComponent->FieldOfView,
			TargetFOV,
			DeltaTime,
			FOVInterpSpeed
		);
		FirstPersonCameraComponent->SetFieldOfView(NewFOV);
	}
}


// Called to bind functionality to input
void AStealthCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AStealthCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AStealthCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthCharacter::MoveInput);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AStealthCharacter::EndMoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStealthCharacter::LookInput);

		//Use
		EnhancedInputComponent->BindAction(UseAction, ETriggerEvent::Triggered, this, &AStealthCharacter::Use);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Triggered, this, &AStealthCharacter::DropWeapon);

		//Attacks
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AStealthCharacter::Attack);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AStealthCharacter::AimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AStealthCharacter::AimEndAction);
		
		EnhancedInputComponent->BindAction(ChangeWeaponAction, ETriggerEvent::Triggered, this, &AStealthCharacter::ChangeWeapon);
		
		//Sneak
		EnhancedInputComponent->BindAction(StealthCrouch, ETriggerEvent::Started, this, &AStealthCharacter::ToggleSneak);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AStealthCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Ongoing, this, &AStealthCharacter::LoopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AStealthCharacter::StopSprint);

	}
	else
	{
		UE_LOG(LogProj_NinjaGame, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AStealthCharacter::Use()
{
	UE_LOG(LogTemp, Warning, TEXT("Player use"));
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + FirstPersonCameraComponent->GetForwardVector() * UseDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TRACE_CHANNEL_INTERACT, Params))
	{
		if (AActor* Actor = HitResult.GetActor())
		{
			if (Actor->GetClass()->ImplementsInterface(UPlayerUseInterface::StaticClass()))
			{
				if (CurrentInteractState == EPlayerInteractState::None || CurrentInteractState == EPlayerInteractState::Interact)
				{
					CurrentInteractState = EPlayerInteractState::Interact;
		
					IPlayerUseInterface::Execute_Use(Actor, this);
				}
			}
		}
	}
}

void AStealthCharacter::StopUse()
{
	if (CurrentInteractState == EPlayerInteractState::Interact)
	{
		CurrentInteractState = EPlayerInteractState::None;
	}
}

void AStealthCharacter::CheckForUse()
{
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + FirstPersonCameraComponent->GetForwardVector() * UseDistance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TRACE_CHANNEL_INTERACT, Params))
	{
		if (AActor* Actor = HitResult.GetActor())
		{
			if (Actor->GetClass()->ImplementsInterface(UPlayerUseInterface::StaticClass()))
			{
				if (LastUseTarget != Actor)
				{
					if (LastUseTarget)
					{
						IPlayerUseInterface::Execute_ShowInteractable(LastUseTarget, false);
					}
					LastUseTarget = Actor;
					IPlayerUseInterface::Execute_ShowInteractable(LastUseTarget, true);
					bShowUseWidget = true;
				}
				return;
			}
		}
	}

	if (bShowUseWidget)
	{
		IPlayerUseInterface::Execute_ShowInteractable(LastUseTarget, false);
		bShowUseWidget = false;
	}

	LastUseTarget = nullptr;
}

void AStealthCharacter::ShowActors(AActor* Actor, bool bShow)
{
	TArray<UMeshComponent*> MeshComps;
	Actor->GetComponents<UMeshComponent>(MeshComps);

	
	for (UMeshComponent* MeshComp : MeshComps)
	{
		if (bShow)
		{
			if (!MeshComp) continue;
			MeshComp->SetVisibility(true, true);
			MeshComp->SetHiddenInGame(false, true);
		}
		else
		{
			if (!MeshComp) continue;
			MeshComp->SetVisibility(false, true);
			MeshComp->SetHiddenInGame(true, true);
		}
	}
}

void AStealthCharacter::ShowWeaponActors(bool bShow)
{
	if (CurrentMeleeWeapon)
	{
		ShowActors(CurrentMeleeWeapon, bShow);
	}
	if (HeldThrowableWeapon)
	{
		ShowActors(HeldThrowableWeapon, bShow);
	}
}

bool AStealthCharacter::CanCrouch() const
{
	if (CurrentMovementState == EPlayerMovementState::Climb)
		return true;
	return Super::CanCrouch();
}

void AStealthCharacter::Climb()
{
	if (CurrentStamina > 0)
	{
		FVector Start = GetActorLocation();
		FVector End = Start + GetActorForwardVector() * ClimbRange;
		
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.0f,0, 5.f);
		
		//If we are at a wall
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
		{
			if (CurrentMovementState == EPlayerMovementState::Climb)
			{
				FVector HalfCapsuleHeight = {0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
				FVector StartEdge = GetActorLocation();
				FVector EndEdge = StartEdge + HalfCapsuleHeight + GetActorForwardVector() * ClimbRange;

				//DrawDebugLine(GetWorld(), StartEdge, EndEdge, FColor::Green, false, 0.0f, 0, 5.f);
				
				//If we are at a ledge
				if (!GetWorld()->LineTraceSingleByChannel(HitResult, StartEdge, EndEdge, ECC_Visibility, Params))
				{
					if (!bHitLedge)
					{
						UE_LOG(LogTemp, Warning, TEXT("At ledge"));
						bHitLedge = true;
						
						ExitClimb();
					}
				}
			}
			
			//If we are holding forward and jump
			if (bMovingForward && bHoldingJump)
			{
				if (!bIsClimbing && !bHitLedge)
				{
					if (GetMovementComponent()->IsFalling())
					{
						//Player is now Climbing
						if (CurrentMovementState != EPlayerMovementState::Climb)
						{
							if (CurrentMovementState == EPlayerMovementState::Run)
							{
								RememberedClimbState = EPlayerMovementState::Walk;
							}
							else
							{
								RememberedClimbState = CurrentMovementState;
							}
						}
						StopSprint();
						//Just makes the character as small as when you are crouching
						if (!bClimbCapsuleShrunk)
						{
							UE_LOG(LogTemp, Warning, TEXT("Climb capsule shrunk"));
							Crouch();
							bClimbCapsuleShrunk = true;
						}
						
						UE_LOG(LogTemp, Warning, TEXT("Climbing"));
						UpdateStaminaStart(ClimbStaminaAmount);
						GetMovementComponent()->Velocity = {};
						CurrentMovementState = EPlayerMovementState::Climb;
						bIsClimbing = true;

						ShowWeaponActors(false);
						
						GetCharacterMovement()->SetMovementMode(MOVE_Flying);
					}
				}
			}
			else
			{
				ExitClimb();
			}
		}
		//If we are not looking at a wall
		else
		{
			ExitClimb();
		}
	}
	//Drop if stamina is not enough
	else
	{
		ExitClimb();
	}
}

void AStealthCharacter::ExitClimb()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;
		
		ShowWeaponActors(true);
		
		UpdateStaminaStart(RegainStaminaAmount);
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		LaunchCharacter({0,0,500},true, true);
	}
}

float AStealthCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("Player took damage. Player Health: %f"), Health);

	if (Health <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AStealthCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Player died!"));

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		GetWorld()->GetTimerManager().SetTimer(
			TempHandle,
			[this]() {
				// Säkerhetsstopp innan leveln laddas om, hade en krasch tidigare så lade till detta för att stoppa kraschen. 
				for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
				{
					if (AAIController* AICon = *It)
					{
						AICon->StopMovement();
						AICon->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(AICon);
						AICon->GetWorldTimerManager().ClearAllTimersForObject(AICon);
						AICon->UnPossess();
					}
				}
				UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
			},
			0.2f, false);
	});
}

void AStealthCharacter::ToggleSneak()
{
	if(bIsHiding == true) return;
	
	if (CurrentMovementState == EPlayerMovementState::Crouch)
	{
		if (CanUnCrouch())
		{
			// Gå tillbaka till gångläge
			UnCrouch();
			GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
			CurrentMovementState = EPlayerMovementState::Walk;
			//UE_LOG(LogTemp, Warning, TEXT("Player stopped sneaking."));
		}
	}
	else
	{
		if (CurrentMovementState != EPlayerMovementState::Climb)
		{
			if (CurrentMovementState == EPlayerMovementState::Run)
			{
				UpdateStaminaStart(RegainStaminaAmount);
			}
		
			// Aktivera crouch-läge
			Crouch();
			GetCharacterMovement()->MaxWalkSpeedCrouched = SneakWalkSpeed;
			GetCharacterMovement()->MaxWalkSpeed = SneakWalkSpeed;
			CurrentMovementState = EPlayerMovementState::Crouch;
			//UE_LOG(LogTemp, Warning, TEXT("Player is now sneaking."));
		}
	}
}

bool AStealthCharacter::CanUnCrouch()
{
	const float DeltaZ = GetDefaultHalfHeight() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector CheckLocation = GetActorLocation() + FVector(0, 0, DeltaZ);

	TArray<UPrimitiveComponent*> Overlaps;

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn),
		UEngineTypes::ConvertToObjectType(ECC_PhysicsBody)
	};

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	bool bHit = UKismetSystemLibrary::CapsuleOverlapComponents(
		GetWorld(),
		CheckLocation,
		GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
		GetDefaultHalfHeight(),
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		Overlaps
		);
	
	if (!bHit)
	{
		return true;
	}

	//Looks for if we hit a collider that blocks uncrouch
	for (UPrimitiveComponent* Comp : Overlaps)
	{
		if (!Comp)
			continue;

		if (Comp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
			continue;

		ECollisionResponse Response = Comp->GetCollisionResponseToChannel(ECC_Pawn);

		if (Response == ECR_Block)
		{
			return false;
		}
	}

	return true;
}

void AStealthCharacter::StartSprint()
{
	if(bIsHiding == true) return;
	
	if (CurrentMovementState != EPlayerMovementState::Climb)
	{
		// Kolla om spelaren faktiskt försöker röra sig
		FVector2D MovementInput(MoveInputForward, MoveInputRight);
		
		if (MovementInput.IsNearlyZero())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Cannot sprint without MovementInput."));
			return;
		}
	
		if (CurrentStamina > 0)
		{
			if (CurrentStamina > 0)
			{
				// Om spelaren är i crouch så lämna det läget först
				if (CurrentMovementState == EPlayerMovementState::Crouch)
				{
					if (CanUnCrouch())
					{
						UnCrouch();
						UpdateStaminaStart(SprintStaminaAmount);
				
						GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
						CurrentMovementState = EPlayerMovementState::Run;
						AimEndAction();
					}
				}
				else
				{
					UpdateStaminaStart(SprintStaminaAmount);
	
					GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
					CurrentMovementState = EPlayerMovementState::Run;
					AimEndAction();
				}
	
				//UE_LOG(LogTemp, Warning, TEXT("Player started sprinting."));
			}
		}
	}
}

void AStealthCharacter::LoopSprint()
{
	if (CurrentMovementState != EPlayerMovementState::Run)
	{
		if(bIsHiding == true) return;
		
		if(bIsAiming == true) return;
	
		if (CurrentMovementState != EPlayerMovementState::Climb)
		{
			// Kolla om spelaren faktiskt försöker röra sig
			FVector2D MovementInput(MoveInputForward, MoveInputRight);
		
			if (MovementInput.IsNearlyZero())
			{
				//UE_LOG(LogTemp, Warning, TEXT("Cannot sprint without MovementInput."));
				return;
			}
	
			if (CurrentStamina > 0)
			{
				if (CurrentStamina > 0)
				{
					// Om spelaren är i crouch så lämna det läget först
					if (CurrentMovementState == EPlayerMovementState::Crouch)
					{
						if (CanUnCrouch())
						{
							UnCrouch();
							UpdateStaminaStart(SprintStaminaAmount);
				
							GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
							CurrentMovementState = EPlayerMovementState::Run;
						}
					}
					else
					{
						UpdateStaminaStart(SprintStaminaAmount);
	
						GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
						CurrentMovementState = EPlayerMovementState::Run;
					}
	
					//UE_LOG(LogTemp, Warning, TEXT("Player started sprinting."));
				}
			}
		}
	}
}

void AStealthCharacter::StopSprint()
{
	if (CurrentMovementState == EPlayerMovementState::Run)
	{
		UpdateStaminaStart(RegainStaminaAmount);
		
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
		CurrentMovementState = EPlayerMovementState::Walk;
		//UE_LOG(LogTemp, Warning, TEXT("Player stopped sprinting."));
	}
}

// För debugging 
FString AStealthCharacter::MovementStateToString(EPlayerMovementState State)
{
	switch (State)
	{
	case EPlayerMovementState::Walk:   return TEXT("Walk");
	case EPlayerMovementState::Run:    return TEXT("Run");
	case EPlayerMovementState::Crouch: return TEXT("Crouch");
	case EPlayerMovementState::Climb:  return TEXT("Climb");
	default: return TEXT("Unknown");
	}
}

// För HideSpot
void AStealthCharacter::SetCustomCameraLocation(USceneComponent* NewCameraComponent)
{
	if (!NewCameraComponent) return;
	FirstPersonCameraComponent->AttachToComponent(NewCameraComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AStealthCharacter::ResetToNormalCamera()
{
	
	FirstPersonCameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ROOT"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
}


void AStealthCharacter::OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		EnemiesInAssassinationRange.AddUnique(Enemy);
		CheckForCanAssassinate();
	}
}

void AStealthCharacter::OnMeleeBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		EnemiesInAssassinationRange.Remove(Enemy);
		CheckForCanAssassinate();
	}
}

void AStealthCharacter::CheckForCanAssassinate()
{
	if (!PlayerMeleeBox)
		return;
	
	bCanAssassinate = false;

	if (EnemiesInAssassinationRange.Num() == 0)
		return;

	for (AMeleeEnemy* Enemy  : EnemiesInAssassinationRange)
	{
		if (!Enemy) continue;

		if (!Enemy->CanSeePlayer() && Enemy->bCanBeAssassinated)
		{
			bCanAssassinate = true;
			return;
		}
	}

	//Old code that can do the whole check by itself, but is more expensive
	/*if (!PlayerMeleeBox)
		return;
	
	bCanAssassinate = false;

	TArray<AActor*> Overlaps;
	PlayerMeleeBox->GetOverlappingActors(Overlaps, AMeleeEnemy::StaticClass());
	if (Overlaps.Num() == 0)
		return;

	for (AActor* OverlapActor : Overlaps)
	{
		if (AMeleeEnemy* MeleeEnemy = Cast<AMeleeEnemy>(OverlapActor))
		{
			if (!MeleeEnemy->CanSeePlayer() && MeleeEnemy->bCanBeAssassinated)
			{
				bCanAssassinate = true;
			}
		}
	}*/
}