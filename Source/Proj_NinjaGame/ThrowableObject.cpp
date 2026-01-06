// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"

#include "AchievementSubsystem.h"
#include "BreakableObject.h"
#include "MeleeEnemy.h"
#include "SecurityCamera.h"
#include "SmokeBombObject.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "SoundUtility.h"
#include "ThrowingMarker.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Field/FieldSystemActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

AThrowableObject::AThrowableObject()
{
	ThrowCollision = CreateDefaultSubobject<UBoxComponent>("ThrowCollision");
	RootComponent = ThrowCollision;
	ThrowCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ThrowCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComponent->SetupAttachment(ThrowCollision);
	StaticMeshComponent->SetGenerateOverlapEvents(true);

	
	BreakVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BreakVFX"));
	BreakVFXComponent->SetupAttachment(RootComponent);
	BreakVFXComponent->bAutoActivate = false;
}

void AThrowableObject::Use_Implementation(class AStealthCharacter* Player)
{
	IPlayerUseInterface::Use_Implementation(Player);
	
	if (!Player) return;
	
	if (Thrown) return;
	
	HandlePickup(Player);

	if (Player->GetThrowingMarker())
	{
		Player->GetThrowingMarker()->UpdateSpawnMarkerMesh(Player->HeldThrowableWeapon->ThrownWeaponObject);
	}
}

void AThrowableObject::ShowInteractable_Implementation(bool bShow)
{
	IPlayerUseInterface::ShowInteractable_Implementation(bShow);
	
	if (Thrown) return;
	
	StaticMeshComponent->SetRenderCustomDepth(bShow);
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		if (bShow)
		{
			if (HoverSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HoverSound, GetActorLocation());
			}
			
			GI->CurrentInteractText = InteractText;
		}
		else
		{
			GI->CurrentInteractText = "";
		}
	}
}

void AThrowableObject::SpawnFieldActor()
{
	if (!FieldActorClass)
		return;
	
	FieldActor = GetWorld()->SpawnActor<AFieldSystemActor>(
		FieldActorClass,
		GetActorLocation(),
		GetActorRotation()
	);
}

void AThrowableObject::ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Thrown)
		return;

	SetShowVFX(true);
	
	Thrown = false;
	ChangeToThrowCollision(false);
	SpawnFieldActor();
	
	ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AThrowableObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
	{
		if (!Enemy->GetIsDead())
		{
			/*if (Hit.Component == Enemy->GetHeadComponent())
			{
				UGameplayStatics::ApplyPointDamage(
					Enemy,
					Enemy->GetHealth(),
					GetVelocity().GetSafeNormal(),
					Hit,
					UGameplayStatics::GetPlayerController(this,0),
					this,
					UDamageType::StaticClass()
				);
			}*/
			if (Hit.Component == Enemy->GetHeadComponent())
			{
				if (Enemy->DoesHaveHelmet())        
				{
					Enemy->RemoveHelmet();          
				}
				else
				{
					UGameplayStatics::ApplyPointDamage(
						Enemy,
						Enemy->GetHealth(),         
						GetVelocity().GetSafeNormal(),
						Hit,
						UGameplayStatics::GetPlayerController(this,0),
						this,
						UDamageType::StaticClass()
					);
					if (UGameInstance* GI = GetGameInstance())
					{
						if (UAchievementSubsystem* Achievements = GI->GetSubsystem<UAchievementSubsystem>())
						{
							Achievements->OnHeadShot();
						}
					}
				}
			}
			else
			{
				UGameplayStatics::ApplyPointDamage(
					Enemy,
					DealtDamage,
					GetVelocity().GetSafeNormal(),
					Hit,
					UGameplayStatics::GetPlayerController(this,0),
					this,
					UDamageType::StaticClass()
				);

				// Stun
				if (ShouldApplyDefaultStun())
				{
					if (!Cast<ASmokeBombObject>(this))
					{
						if (Enemy)
						{
							AMeleeAIController* EnemyController = Cast<AMeleeAIController>(Enemy->GetController());

							if (EnemyController)
							{
								// Stunna fienden i 1 sekunder 
								EnemyController->StunEnemy(1.0f, EEnemyState::Chasing); 
							}
						}
					}
				}
			}
		
			if (ImpactEnemySound)
			{
			
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
			else if (ImpactGroundSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
		
			DestroyObject();
		}
		else
		{
			DestroyObject();
		}
	}
	else if (ASecurityCamera* Camera = Cast<ASecurityCamera>(OtherActor)) 
	{
		if (!Camera->GetIsDead())
		{
			UGameplayStatics::ApplyPointDamage(
				Camera,
				DealtDamage,
				GetVelocity().GetSafeNormal(),
				Hit,
				UGameplayStatics::GetPlayerController(this, 0),
				this,
				UDamageType::StaticClass()
			);

			if (ImpactEnemySound)
			{
			
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
			else if (ImpactGroundSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
		
			DestroyObject();
		}
	}
	else if (ABreakableObject* Breakable = Cast<ABreakableObject>(OtherActor))
	{
		Breakable->BreakObject();
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		if (bBreaksOnImpact)
		{
			DestroyObject();
		}
		else
		{
			ThrowCollision->SetSimulatePhysics(true);
		}
	}
	else if (Cast<AStealthCharacter>(OtherActor))
	{
		//Quick fix so that you can hit an enemy if they are in your face
		Thrown = true;
	}
	else
	{
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		if (bBreaksOnImpact)
		{
			DestroyObject();
		}
		else
		{
			ThrowCollision->SetSimulatePhysics(true);
		}
	}

	//Sound för fienden
	float NoiseLevel = 4.0f;

	USoundUtility::ReportNoise(GetWorld(), Hit.ImpactPoint, NoiseLevel, this);
}

void AThrowableObject::HandlePickup(AStealthCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Pickup"));

	//Drop item if I have something in my inventory
	if (Player->LastHeldWeapon)
	{
		Player->HeldThrowableWeapon->Destroy();
		AThrowableWeapon* DropWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->LastHeldWeapon);
		DropWeapon->Drop(Player);
	}
	
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
	}
	if (ThrowableWeapon)
	{
		if (Player->HeldThrowableWeapon)
		{
			Player->HeldThrowableWeapon->Destroy();
		}
		Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
		Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
		Player->LastHeldWeapon = ThrowableWeapon;
	}
		
	Destroy();
}

void AThrowableObject::ChangeToThrowCollision(bool bCond)
{
	if (bCond)
	{
		StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		
		ThrowCollision->SetCollisionResponseToAllChannels(ECR_Block);
		ThrowCollision->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Ignore);
		ThrowCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		ThrowCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		ThrowCollision->SetCollisionResponseToChannel(TRACE_CHANNEL_CLIMB, ECR_Ignore);
	}
	else
	{
		ThrowCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		
		StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Block);
	}
}

void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
}

void AThrowableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Thrown) return;
	
	FVector Start = GetActorLocation();

	ThrowVelocity.Z += GravityZ * DeltaTime;

	FVector NextPos = Start + ThrowVelocity * DeltaTime;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		NextPos,
		ThrowCollision->GetComponentQuat(),
		ECC_Camera,
		FCollisionShape::MakeBox(ThrowCollision->GetScaledBoxExtent()),
		Params))
	{
		SetActorLocation(Hit.Location);
		ThrowableOnComponentHit(ThrowCollision, Hit.GetActor(), Hit.GetComponent(), FVector::ZeroVector, Hit);
	}

	SetActorLocation(NextPos);
}


void AThrowableObject::DestroyObject()
{
	SetLifeSpan(10);
	StaticMeshComponent->SetStaticMesh(nullptr);
	ThrowCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	if (ImpactDebris)
	{
		UGeometryCollectionComponent* GeoComp =
		NewObject<UGeometryCollectionComponent>(this, UGeometryCollectionComponent::StaticClass());

		if (GeoComp)
		{
			FTransform MeshTransform = StaticMeshComponent->GetComponentTransform();
			
			GeoComp->SetupAttachment(StaticMeshComponent);
			GeoComp->SetWorldTransform(MeshTransform);
			GeoComp->RegisterComponent();

			GeoComp->SetRelativeTransform(FTransform::Identity);

			GeoComp->SetRestCollection(ImpactDebris);

			GeoComp->SetPerLevelCollisionProfileNames({"Debris","Debris","Debris"});
			GeoComp->SetCanEverAffectNavigation(false);
		}
		
		if (BreakVFXComponent && BreakVFXComponent->GetAsset())
		{
			BreakVFXComponent->Activate();
		}
	}
}
