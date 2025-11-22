// Fill out your copyright notice in the Description page of Project Settings.


#include "SprinklerSwitch.h"

#include "EnemyHandler.h"
#include "MeleeEnemy.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraActor.h"

ASprinklerSwitch::ASprinklerSwitch()
{
	EnemyHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EnemyHitBox"));
	EnemyHitBox->SetupAttachment(RootComponent);
	WaterAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WaterAudio"));
	WaterAudio->SetupAttachment(RootComponent);
	
	EnemyHitBox->OnComponentBeginOverlap.AddDynamic(this, &ASprinklerSwitch::EnemyBeginOverlap);
}

void ASprinklerSwitch::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);

	TurnPowerOnOff();
}

void ASprinklerSwitch::BeginPlay()
{
	Super::BeginPlay();
	WaterAudio->SetSound(SprinklerSound);
}

void ASprinklerSwitch::TurnPowerOnOff()
{
	TurnWaterOnOff(bPowerOn);
	
	ReduceEnemyHearingRange(bPowerOn);

	SendClosetEnemy();
	
	bPowerOn = !bPowerOn;
}

void ASprinklerSwitch::TurnWaterOnOff(bool bOnOff)
{
	if (Sprinklers.Num() <= 0)
		return;
	
	if (bOnOff)
	{
		WaterAudio->Play();
		for (auto Sprinkler : Sprinklers)
		{
			Sprinkler->GetNiagaraComponent()->Activate();
		}
	}
	else
	{
		WaterAudio->Stop();
		for (auto Sprinkler : Sprinklers)
		{
			Sprinkler->GetNiagaraComponent()->Deactivate();
		}
	}
}

void ASprinklerSwitch::ReduceEnemyHearingRange(bool bOnOff)
{
	if (!EnemyHandler)
		return;
	
	for (auto EnemyActor : EnemyHandler->GetAllEnemies())
	{
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor))
		{
			Enemy->ReduceEnemyHearingRange(bOnOff);
		}
	}
}

void ASprinklerSwitch::SendClosetEnemy()
{
	if (!EnemyHandler)
		return;
	if (bPowerOn)
	{
		if (AMeleeEnemy* Enemy = EnemyHandler->GetClosestEnemyToLocation(GetActorLocation()))
		{
			if (AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController()))
			{
				AI->SetCurrentMission(EEnemyMission::Sprinkler);
				Enemy->OnSuspiciousLocation.Broadcast(EnemyHitBox->GetComponentLocation()); 
			}
		}
	}
}

void ASprinklerSwitch::EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		if (AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController()))
		{
			if (AI->GetCurrentMission() == EEnemyMission::Sprinkler)
			{
				if (bPowerOn)
				{
					AI->SetCurrentMission(EEnemyMission::Patrol);
				}
				else
				{
					AI->SetCurrentMission(EEnemyMission::Patrol);
					TurnPowerOnOff();
				}
			}
		}
	}
}
