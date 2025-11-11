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
#include "GameFramework/CharacterMovementComponent.h"
#include "Proj_NinjaGame.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"


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
}

void AStealthCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	Move(MovementVector.X, MovementVector.Y);
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
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
		
		float NoiseLevel = 1.0f;
		
		if (bIsSneaking)
		{
			NoiseLevel = 1.0f * SneakNoiseMultiplier;
		}
		else if (bIsSprinting)
		{
			NoiseLevel *= SprintNoiseMultiplier;
		}

		USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel); 
	}
}

void AStealthCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AStealthCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();

	float NoiseLevel;
		
	if (bIsSneaking)
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
		UE_LOG(LogTemp, Display, TEXT("Aiming"));
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
		UE_LOG(LogTemp, Display, TEXT("No Aiming"));
		//MeleeWeapon
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
					HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
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
					HeldThrowableWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
				}
			}
		}
	}
}

void AStealthCharacter::AimStart()
{
	if (!bIsSprinting)
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
    PredictParams.LaunchVelocity = FirstPersonCameraComponent->GetForwardVector() * HeldThrowableWeapon->ThrowSpeed + GetVelocity();
	if (UStaticMesh* WeaponMesh = HeldThrowableWeapon->StaticMeshComponent->GetStaticMesh())
	{
		FVector BoundsExtent = WeaponMesh->GetBounds().BoxExtent;
		PredictParams.ProjectileRadius = BoundsExtent.GetAbsMax();
	}
	else
	{
		PredictParams.ProjectileRadius = 5.f;
	}
    PredictParams.MaxSimTime = 1.f; // Max simulated tim of travel per second
    PredictParams.bTraceWithCollision = true; //If hit something
    PredictParams.SimFrequency = 15.f; //How many checks per second
    PredictParams.TraceChannel = ECC_Visibility;
    PredictParams.ActorsToIgnore.Add(this);

    FPredictProjectilePathResult PredictResult;
    UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	SpawnedMarker->SetActorLocation(PredictResult.HitResult.Location);
}

// Called when the game starts or when spawned
void AStealthCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FirstPersonCameraComponent)
	{
		DefaultCameraRelativeLocation = FirstPersonCameraComponent->GetRelativeLocation();
		TargetCameraBaseLocation = DefaultCameraRelativeLocation;
		NormalFOV = FirstPersonCameraComponent->FieldOfView;
	}
}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForUse();
	
	if (bIsAiming && HeldThrowableWeapon)
	{
		UpdateProjectilePrediction();
	}

	if (FirstPersonCameraComponent)
	{
		// Ändrar FOV när spelaren springer 
		float TargetFOV = bIsSprinting ? SprintFOV : NormalFOV;
		
		float NewFOV = FMath::FInterpTo(
			FirstPersonCameraComponent->FieldOfView,
			TargetFOV,
			DeltaTime,
			FOVInterpSpeed
		);
		FirstPersonCameraComponent->SetFieldOfView(NewFOV);
		
		// övergång mellan stå och crouch 
		FVector SmoothedBaseLocation = FMath::VInterpTo(
			FirstPersonCameraComponent->GetRelativeLocation(),
			TargetCameraBaseLocation,
			DeltaTime,
			8.0f 
		);

		FVector FinalCameraLocation = SmoothedBaseLocation;
		
		// Skakar kameran lite när spelaren springer
		if (bIsSprinting && GetVelocity().Size() > 100.f)
		{
			BobTimer += DeltaTime * CameraBobSpeed;
			FinalCameraLocation.Z += FMath::Sin(BobTimer) * CameraBobAmplitude;
		}
		else
		{
			BobTimer = 0.0f;
		}
		
		FirstPersonCameraComponent->SetRelativeLocation(FinalCameraLocation);
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
			}
		}

		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
	});
}

void AStealthCharacter::ToggleSneak()
{
	bIsSneaking = !bIsSneaking;

	if (bIsSneaking)
	{
		// Crouchläge
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = SneakWalkSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("Player is now sneaking."));
		TargetCameraBaseLocation = DefaultCameraRelativeLocation + CrouchCameraOffset;

	}
	else
	{
		// Normalläge 
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("Player stopped sneaking."));
		TargetCameraBaseLocation = DefaultCameraRelativeLocation;
	}
}



void AStealthCharacter::StartSprint()
{
	if (!bIsSneaking) // för att inte kunna springa när man är i Crouch läge 
	{
		AimEnd();
		
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("Player started sprinting."));
	}
}

void AStealthCharacter::StopSprint()
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("Player stopped sprinting."));
	}
}
