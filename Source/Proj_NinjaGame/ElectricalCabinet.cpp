// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectricalCabinet.h"

#include "EnemyHandler.h"
#include "SecurityCamera.h"
#include "Components/BoxComponent.h"
#include "Components/LightComponent.h"
#include "Engine/Light.h"
#include "Storage/Nodes/FileEntry.h"

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
	

	// starta timer
	if (bPowerOn)
	{
		//UE_LOG(LogTemp, Error, TEXT("TurnPowerOnOff, start timer"));
		GetWorld()->GetTimerManager().SetTimer(
			RetryEnemySendTimer, 
			this, 
			&AElectricalCabinet::RetrySendEnemy, 
			RetrySendInterval, 
			true
		);
	}
	else // stoppa timer
	{
		//UE_LOG(LogTemp, Error, TEXT("TurnPowerOnOff, stop timer"));
		GetWorld()->GetTimerManager().ClearTimer(RetryEnemySendTimer);
	}

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
		if (AEnemy* Enemy = Cast<AEnemy>(EnemyActor))
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
		if (AEnemy* Enemy = EnemyHandler->GetClosestEnemyToLocation(GetActorLocation()))
		{
			if (AEnemyAIController* AI = Cast<AEnemyAIController>(Enemy->GetController()))
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
	if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
	{
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(Enemy->GetController()))
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


void AElectricalCabinet::RetrySendEnemy()
{
	// Om den är på igen så stoppa timern
	if (bPowerOn)
	{
		GetWorld()->GetTimerManager().ClearTimer(RetryEnemySendTimer);
		return;
	}

	if (!EnemyHandler)
		return;

	//UE_LOG(LogTemp, Error, TEXT("RetrySendEnemy"));
	
	if (AEnemy* Enemy = EnemyHandler->GetClosestEnemyToLocation(GetActorLocation()))
	{
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(Enemy->GetController()))
		{
			AI->SetCurrentMission(EEnemyMission::Electrical);
			//Enemy->OnSuspiciousLocation.Broadcast(EnemyHitBox->GetComponentLocation()); 
			AI->AssignMission(EEnemyMission::Electrical, EnemyHitBox->GetComponentLocation());
		}
	}
}
