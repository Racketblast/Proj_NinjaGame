// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectricalCabinet.h"

#include "EnemyHandler.h"
#include "SecurityCamera.h"
#include "Components/LightComponent.h"
#include "Engine/Light.h"

AElectricalCabinet::AElectricalCabinet()
{
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
	for (auto Light : LightsToTurnOff)
	{
		Light->GetLightComponent()->SetVisibility(!bOnOff);
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
	
	UE_LOG(LogTemp, Warning, TEXT("Sending enemy"));
	if (AMeleeEnemy* Enemy = EnemyHandler->GetClosestEnemyToLocation(GetActorLocation()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Closest Enemy: %s"), *Enemy->GetActorNameOrLabel());
		if (AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController()))
		{
			Enemy->OnSuspiciousLocation.Broadcast(GetActorLocation()); 
		}
	}
}