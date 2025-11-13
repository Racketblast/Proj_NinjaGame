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
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Navigation/PathFollowingComponent.h"


// Sets default values
AStealthCharacter::AStealthCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = SneakWalkSpeed;

	CurrentStamina = MaxStamina;
}

void AStealthCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

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
		if (CurrentMovementState == EPlayerMovementState::Climb)
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

void AStealthCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
	bHoldingJump = true;
}

void AStealthCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
	
	bHoldingJump = false;
	
	float NoiseLevel;
		
	if (CurrentMovementState == EPlayerMovementState::Crouch)
	{
		NoiseLevel = 1.0f * SneakNoiseMultiplier;
	}
	else
	{
		NoiseLevel = 1.5f;
	}
	USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
}

void AStealthCharacter::Attack()
{
	if (bIsAiming)
	{
		UE_LOG(LogTemp, Display, TEXT("Ranged attack"));
		//ThrowingWeapon
		if (HeldThrowableWeapon)
		{
			if (ThrowSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ThrowSound, GetActorLocation());
			}
			HeldThrowableWeapon->Throw(this);
		}
	}
	else
	{
		//MeleeWeapon
		if (CurrentMeleeWeapon->bCanMeleeAttack)
		{
			if (CurrentMovementState != EPlayerMovementState::Run && CurrentMovementState != EPlayerMovementState::Climb)
			{
				if (CurrentStamina + AttackStaminaAmount >= 0)
				{
					UE_LOG(LogTemp, Display, TEXT("Melee attack"));
					CurrentStamina += AttackStaminaAmount;
					CurrentMeleeWeapon->bCanMeleeAttack = false;
					CurrentMeleeWeapon->bMeleeAttacking = true;
					
					UpdateStaminaStart(RegainStaminaAmount);
				}
			}
		}
	}
}

void AStealthCharacter::EquipKunai()
{
	if (!bIsAiming)
	{
		if (AmountOfKunai > 0)
		{
			if (AKunaiWeapon* Kunai = Cast<AKunaiWeapon>(HeldThrowableWeapon))
			{
				if (LastHeldWeapon != nullptr)
				{
					if (HeldThrowableWeapon)
					{
						HeldThrowableWeapon->Destroy();
					}
					UE_LOG(LogTemp, Display, TEXT("Unequipping Kunai"));
					HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(LastHeldWeapon);
					HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
				}
			}
			else
			{
				if (KunaiWeapon != nullptr)
				{
					if (HeldThrowableWeapon)
					{
						HeldThrowableWeapon->Destroy();
					}
					UE_LOG(LogTemp, Display, TEXT("Equipping Kunai"));
					HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(KunaiWeapon);
					HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
				}
			}
		}
	}
}

void AStealthCharacter::AimStart()
{
	if (CurrentMovementState != EPlayerMovementState::Run ) 
	{
		if (HeldThrowableWeapon)
		{
			bIsAiming = true;
	
			if (MarkerClass && !SpawnedMarker)
			{
				FVector SpawnLoc = GetActorLocation();
				FRotator SpawnRot = FRotator::ZeroRotator;
				SpawnedMarker = GetWorld()->SpawnActor<AActor>(MarkerClass, SpawnLoc, SpawnRot);
			}
		}
	}
}

void AStealthCharacter::AimEnd()
{
	bIsAiming = false;
	
	// Clean up
	if (SpawnedMarker)
	{
		SpawnedMarker->Destroy();
		SpawnedMarker = nullptr;
	}
}

void AStealthCharacter::UpdateProjectilePrediction()
{
	if (!FirstPersonCameraComponent)
        return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
	
	
    FPredictProjectilePathParams PredictParams;
    PredictParams.StartLocation = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector() * CameraForwardMultiplier;
    PredictParams.LaunchVelocity = FirstPersonCameraComponent->GetForwardVector() * HeldThrowableWeapon->ThrowSpeed;
	if (UStaticMesh* WeaponMesh = HeldThrowableWeapon->StaticMeshComponent->GetStaticMesh())
	{
		FVector BoundsExtent = WeaponMesh->GetBounds().BoxExtent;
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

	SpawnedMarker->SetActorLocation(PredictResult.HitResult.Location);
}

void AStealthCharacter::UpdateStaminaStart(float InStamina)
{
	UpdateStaminaAmount = InStamina;
	if (UpdateStaminaAmount >= 0)
	{
		//Regain Stamina
		GetWorld()->GetTimerManager().SetTimer(StaminaTimer, this, &AStealthCharacter::UpdateStaminaLoop, StaminaRefreshRate, true, 3);
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

	if (FirstPersonCameraComponent)
	{
		NormalFOV = FirstPersonCameraComponent->FieldOfView;
	}
	
	if (KunaiWeapon && AmountOfKunai > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equip Kunai"));
		if (HeldThrowableWeapon)
		{
			HeldThrowableWeapon->Destroy();
		}
		HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(KunaiWeapon);
		HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
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
}

void AStealthCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	UE_LOG(LogTemp, Display, TEXT("Landed"));
	
	bCanClimb = true;
}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForUse();
	
	//Gives a reticle for where the throwable object will go
	if (bIsAiming && HeldThrowableWeapon)
	{
		UpdateProjectilePrediction();
	}

	//Climbing
	Climb();

	if (FirstPersonCameraComponent)
	{
		// Ändrar FOV när spelaren springer 
		float TargetFOV = (CurrentMovementState == EPlayerMovementState::Run) ? SprintFOV : NormalFOV;
		
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

		//Attacks
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AStealthCharacter::Attack);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AStealthCharacter::AimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AStealthCharacter::AimEnd);
		
		EnhancedInputComponent->BindAction(KunaiAction, ETriggerEvent::Triggered, this, &AStealthCharacter::EquipKunai);
		
		//Sneak
		EnhancedInputComponent->BindAction(StealthCrouch, ETriggerEvent::Started, this, &AStealthCharacter::ToggleSneak);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AStealthCharacter::StartSprint);
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
				IPlayerUseInterface::Execute_Use(Actor, this);
			}
		}
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

void AStealthCharacter::Climb()
{
	if (bCanClimb)
	{
		if (CurrentStamina > 0)
		{
			FVector Start = GetActorLocation();
			FVector End = Start + GetActorForwardVector() * 45;
			
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			
			//If we are at a wall
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
			{
				if (CurrentMovementState == EPlayerMovementState::Climb)
				{
					//If we are at a ledge
					FVector HalfCapsuleHeight = {0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
					FVector StartEdge = GetActorLocation();
					FVector EndEdge = StartEdge + HalfCapsuleHeight + GetActorForwardVector() * 45;
					
					if (!GetWorld()->LineTraceSingleByChannel(HitResult, StartEdge, EndEdge, ECC_Visibility, Params))
					{
						UpdateStaminaStart(RegainStaminaAmount);
						ToggleSneak();
						GetCharacterMovement()->SetMovementMode(MOVE_Falling);
						LaunchCharacter({0,0,500},true, true);
						bCanClimb = false;
						return;
					}
				}
				
				if (bMovingForward && bHoldingJump)
				{
					if (CurrentMovementState != EPlayerMovementState::Climb)
					{
						if (GetMovementComponent()->IsFalling())
						{
							UpdateStaminaStart(ClimbStaminaAmount);
							GetMovementComponent()->Velocity = {};
							CurrentMovementState = EPlayerMovementState::Climb;
							GetCharacterMovement()->SetMovementMode(MOVE_Flying);
						}
					}
				}
				else
				{
					if (CurrentMovementState == EPlayerMovementState::Climb)
					{
						UpdateStaminaStart(RegainStaminaAmount);
						CurrentMovementState = EPlayerMovementState::Walk;
						GetCharacterMovement()->SetMovementMode(MOVE_Falling);
						LaunchCharacter({0,0,500},true, true);
					}
				}
			}
			else
			{
				if (CurrentMovementState == EPlayerMovementState::Climb)
				{
					UpdateStaminaStart(RegainStaminaAmount);
					CurrentMovementState = EPlayerMovementState::Walk;
					GetCharacterMovement()->SetMovementMode(MOVE_Falling);
					LaunchCharacter({0,0,500},true, true);
				}
			}
		}
		//Drop if stamina is not enough
		else
		{
			if (CurrentMovementState == EPlayerMovementState::Climb)
			{
				UpdateStaminaStart(RegainStaminaAmount);
				CurrentMovementState = EPlayerMovementState::Walk;
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
				LaunchCharacter({0,0,500},true, true);
			}
		}
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
		// Säkerhetsstopp innan leveln laddas om, hade en krash tidigare så lade till detta för att stopa krasshen. 
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
	});
}

void AStealthCharacter::ToggleSneak()
{	
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

bool AStealthCharacter::CanUnCrouch()
{
	const float DeltaZ = GetDefaultHalfHeight() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector CheckLocation = GetActorLocation() + FVector(0, 0, DeltaZ);

	TArray<UPrimitiveComponent*> Overlaps;

	// We only care about blocking geometry, so use ECC_Pawn or Visibility/WorldStatic as needed
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

	return !bHit;
}

void AStealthCharacter::StartSprint()
{
	if (CurrentStamina > 0)
	{
		AimEnd();
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
