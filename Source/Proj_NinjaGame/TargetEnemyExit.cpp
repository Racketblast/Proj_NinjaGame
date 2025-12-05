// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetEnemyExit.h"

#include "Enemy.h"
#include "TargetEnemy.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"

// Sets default values
ATargetEnemyExit::ATargetEnemyExit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EnemyHitBox"));
	RootComponent = EnemyHitBox;
	EnemyHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnemyHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EnemyHitBox->OnComponentBeginOverlap.AddDynamic(this, &ATargetEnemyExit::EnemyBeginOverlap);
}

void ATargetEnemyExit::PlayerLoses()
{
	if (LoseScreen)
	{
		if (UUserWidget* LoseWidget = CreateWidget<UUserWidget>(GetWorld(), LoseScreen))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player loses"));
			LoseWidget->AddToViewport();
		}
	}
}

void ATargetEnemyExit::EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ATargetEnemy* TargetEnemy = Cast<ATargetEnemy>(OtherActor))
	{
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(TargetEnemy->GetController()))
		{
			if (AI->GetCurrentState() == EEnemyState::Chasing)
			{
				PlayerLoses();
			}
		}
	}
}


