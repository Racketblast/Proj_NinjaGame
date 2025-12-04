// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableObject.h"

#include "NiagaraComponent.h"
#include "StealthGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	SparkleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkleComponent"));
	SparkleComponent->SetupAttachment(StaticMeshComponent);
	SparkleComponent->SetAutoActivate(false);
}

void AInteractableObject::Use_Implementation(class AStealthCharacter* Player)
{
	IPlayerUseInterface::Use_Implementation(Player);
	
	if (!Player) return;
	
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
	}
}

void AInteractableObject::ShowInteractable_Implementation(bool bShow)
{
	IPlayerUseInterface::ShowInteractable_Implementation(bShow);

	bPlayerLookingAtThis = bShow;
	
	StaticMeshComponent->SetRenderCustomDepth(bShow);
	TArray<USceneComponent*> SceneChildren;
	StaticMeshComponent->GetChildrenComponents(true, SceneChildren);
	for (USceneComponent* Child : SceneChildren)
	{
		if (UStaticMeshComponent* ChildMesh = Cast<UStaticMeshComponent>(Child))
		{
			ChildMesh->SetRenderCustomDepth(bShow);
		}
	}
	
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		if (bShow)
		{
			if (HoverSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HoverSound, GetActorLocation());
			}
	
			GI->CurrentInteractText = InteractText;

			GI->InteractTextOverride = bOverrideInteractText;
		}
		else
		{
			GI->CurrentInteractText = "";
			
			GI->InteractTextOverride = false;
		}
	}
}

void AInteractableObject::UpdateShowInteractable_Implementation()
{
	IPlayerUseInterface::UpdateShowInteractable_Implementation();

	StaticMeshComponent->SetRenderCustomDepth(bPlayerLookingAtThis);
	TArray<USceneComponent*> SceneChildren;
	StaticMeshComponent->GetChildrenComponents(true, SceneChildren);
	for (USceneComponent* Child : SceneChildren)
	{
		if (UStaticMeshComponent* ChildMesh = Cast<UStaticMeshComponent>(Child))
		{
			ChildMesh->SetRenderCustomDepth(bPlayerLookingAtThis);
		}
	}
	
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		if (bPlayerLookingAtThis)
		{
			if (HoverSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HoverSound, GetActorLocation());
			}
	
			GI->CurrentInteractText = InteractText;
			
			GI->InteractTextOverride = bOverrideInteractText;
		}
		else
		{
			GI->CurrentInteractText = "";

			GI->InteractTextOverride = bOverrideInteractText;
		}
	}
}

void AInteractableObject::TurnOnVFX(bool bCond)
{
	if (bShouldShowVFX)
	{
		SparkleComponent->SetActive(bCond);
	}
	else
	{
		SparkleComponent->SetActive(false);
	}
}

// Called when the game starts or when spawned
void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();

	
	ChangeSparkleBasedOnSize();
}

void AInteractableObject::ChangeSparkleBasedOnSize()
{
	if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
	{
		FVector LocalOrigin;
		FVector BoxExtent;

		StaticMeshComponent->GetStaticMesh()->GetBounds().GetBox().GetCenterAndExtents(LocalOrigin, BoxExtent);
		BoxExtent = BoxExtent * StaticMeshComponent->GetComponentScale();

		float SpriteSize = FMath::Clamp(BoxExtent.Size() / 4.0f, 10.0f, 30.0f);
		float SpawnRate = FMath::Clamp(BoxExtent.Size() / 20.0f, 2.0f, 6.0f);
		
		if (SparkleComponent)
		{
			SparkleComponent->SetVariableFloat(TEXT("SpriteSize"), SpriteSize);
			SparkleComponent->SetVariableFloat(TEXT("SpawnRate"), SpawnRate);
		}
	}
}

