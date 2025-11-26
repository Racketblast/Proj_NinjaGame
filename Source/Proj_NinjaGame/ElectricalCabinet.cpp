// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectricalCabinet.h"

#include "EnemyHandler.h"
#include "SecurityCamera.h"
#include "Components/BoxComponent.h"
#include "Components/LightComponent.h"
#include "Engine/Light.h"

AElectricalCabinet::AElectricalCabinet()
{
	EnemyHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EnemyHitBox"));
	EnemyHitBox->SetupAttachment(RootComponent);

	EnemyHitBox->OnComponentBeginOverlap.AddDynamic(this, &AElectricalCabinet::EnemyBeginOverlap);
}

void AElectricalCabinet::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	TurnPowerOnOff();
}

void AElectricalCabinet::BeginPlay()
{
	Super::BeginPlay();
}

void AElectricalCabinet::TurnPowerOnOff()
{
	TurnLightsOnOff(bPowerOn);

	TurnCamerasOnOff(bPowerOn);
	
	ReduceEnemySight(bPowerOn);

	SendClosetEnemy();
	
	bPowerOn = !bPowerOn;
}

void AElectricalCabinet::TurnLightsOnOff(bool bOnOff)
{
	if (LightsToTurnOff.Num() <= 0)
		return;
	
	for (auto Light : LightsToTurnOff)
	{
		if (Light)
		{
			Light->GetLightComponent()->SetVisibility(!bOnOff);
		}
	}
}


void AElectricalCabinet::TurnCamerasOnOff(bool bOnOff)
{
	if (!EnemyHandler)
		return;
	
	for (auto CameraActor : EnemyHandler->GetAllCameras())
	{
		if (ASecurityCamera* Camera = Cast<ASecurityCamera>(CameraActor))
		{
			if (bOnOff)
			{
				Camera->DisableCamera();
			}
			else
			{
				Camera->ActivateCamera();
			}
		}
	}
}

void AElectricalCabinet::ReduceEnemySight(bool bOnOff)
{
	if (!EnemyHandler)
		return;
	
	for (auto EnemyActor : EnemyHandler->GetAllEnemies())
	{
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor))
		{
			Enemy->ReduceEnemyRange(bOnOff);
		}
	}
}

void AElectricalCabinet::SendClosetEnemy()
{
	if (!EnemyHandler)
		return;

	if (bPowerOn)
	{
		if (AMeleeEnemy* Enemy = EnemyHandler->GetClosestEnemyToLocation(GetActorLocation()))
		{
			if (AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController()))
			{
				AI->SetCurrentMission(EEnemyMission::Electrical);
				//Enemy->OnSuspiciousLocation.Broadcast(EnemyHitBox->GetComponentLocation()); 
				AI->AssignMission(EEnemyMission::Electrical, EnemyHitBox->GetComponentLocation());
			}
		}
	}
}

void AElectricalCabinet::EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		if (AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController()))
		{
			if (AI->GetCurrentMission() == EEnemyMission::Electrical)
			{
				if (bPowerOn)
				{
					//AI->SetCurrentMission(EEnemyMission::Patrol);
				}
				else
				{
					//AI->SetCurrentMission(EEnemyMission::Patrol);
					TurnPowerOnOff();
				}
			}
		}
	}
}
