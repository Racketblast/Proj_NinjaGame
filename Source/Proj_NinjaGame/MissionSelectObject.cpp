// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionSelectObject.h"

#include "ObjectiveMarkerWidget.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMissionSelectObject::AMissionSelectObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarkerWidget"));
	MarkerWidget->SetupAttachment(RootComponent);
	MarkerWidget->SetVisibility(true);
	MarkerDisappearSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MarkerDisappearSphere"));
	MarkerDisappearSphere->SetupAttachment(RootComponent);
	MarkerDisappearSphere->InitSphereRadius(300.f);
	MarkerDisappearSphere->OnComponentBeginOverlap.AddDynamic(this, &AMissionSelectObject::OnDisappearSphereBeginOverlap);
	MarkerDisappearSphere->OnComponentEndOverlap.AddDynamic(this, &AMissionSelectObject::OnDisappearSphereEndOverlap);
}

void AMissionSelectObject::Use_Implementation(AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (MissionWidgetClass)
	{
		if (UUserWidget* MissionWidget = CreateWidget<UUserWidget>(GetWorld(), MissionWidgetClass))
		{
			MissionWidget->AddToViewport();
		}
	}
}

void AMissionSelectObject::SetMarkerWidgetVisibility(bool bCond)
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

void AMissionSelectObject::FadeMarkerWidget(bool bShow)
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

// Called when the game starts or when spawned
void AMissionSelectObject::BeginPlay()
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

void AMissionSelectObject::OnDisappearSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		FadeMarkerWidget(false);
	}
}

void AMissionSelectObject::OnDisappearSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		FadeMarkerWidget(true);
	}
}

