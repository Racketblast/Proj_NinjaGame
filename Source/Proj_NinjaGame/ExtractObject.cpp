// Fill out your copyright notice in the Description page of Project Settings.


#include "ExtractObject.h"

#include "EnemyHandler.h"
#include "ObjectiveMarkerWidget.h"
#include "StealthCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SGameLayerManager.h"

AExtractObject::AExtractObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarkerWidget"));
	MarkerWidget->SetupAttachment(RootComponent);
	MarkerWidget->SetVisibility(false);
	MarkerDisappearSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MarkerDisappearSphere"));
	MarkerDisappearSphere->SetupAttachment(RootComponent);
	MarkerDisappearSphere->InitSphereRadius(300.f);
	MarkerDisappearSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	MarkerDisappearSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MarkerDisappearSphere->OnComponentBeginOverlap.AddDynamic(this, &AExtractObject::OnDisappearSphereBeginOverlap);
	MarkerDisappearSphere->OnComponentEndOverlap.AddDynamic(this, &AExtractObject::OnDisappearSphereEndOverlap);
}

void AExtractObject::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (EnemyHandler)
	{
		if (!EnemyHandler->GetEnemySeesPlayer())
		{
			if (ExitWidgetClass)
			{
				if (UUserWidget* ExitWidget = CreateWidget<UUserWidget>(GetWorld(), ExitWidgetClass))
				{
					ExitWidget->AddToViewport();
				}
			}
		}
	}
	else
	{
		if (ExitWidgetClass)
		{
			if (UUserWidget* ExitWidget = CreateWidget<UUserWidget>(GetWorld(), ExitWidgetClass))
			{
				ExitWidget->AddToViewport();
			}
		}
	}
}

void AExtractObject::SetMarkerWidgetVisibility(bool bCond)
{
	if (MarkerWidget && MarkerWidget->GetWidget())
	{
		MarkerWidget->SetVisibility(bCond);
	}
	
	if (bCond)
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (MarkerDisappearSphere->IsOverlappingActor(Player))
		{
			FadeMarkerWidget(false);
		}
	}
}

void AExtractObject::FadeMarkerWidget(bool bShow)
{
	if (MarkerWidget && MarkerWidget->GetWidget())
	{
		if (UObjectiveMarkerWidget* Marker = Cast<UObjectiveMarkerWidget>(MarkerWidget->GetWidget()))
		{
			if (bShow)
			{
				Marker->PlayFadeIn();
			}
			else
			{
				Marker->PlayFadeOut();
			}
		}
	}
}

void AExtractObject::BeginPlay()
{
	Super::BeginPlay();

	if (MarkerWidget && MarkerWidget->GetWidget())
	{
		if (UObjectiveMarkerWidget* Marker = Cast<UObjectiveMarkerWidget>(MarkerWidget->GetWidget()))
		{
			Marker->OriginLocation = GetActorLocation();
		}
	}
}

void AExtractObject::OnDisappearSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		FadeMarkerWidget(false);
	}
}

void AExtractObject::OnDisappearSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		FadeMarkerWidget(true);
	}
}
