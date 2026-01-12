// Fill out your copyright notice in the Description page of Project Settings.


#include "AchievementTriggerBox.h"

#include "AchievementSubsystem.h"
#include "StealthCharacter.h"


AAchievementTriggerBox::AAchievementTriggerBox()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


void AAchievementTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAchievementTriggerBox::OnTriggerBegin);
}


void AAchievementTriggerBox::OnTriggerBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bTriggerOnlyOnce && bHasTriggered)
		return;
	
	if (!OtherActor || !OtherActor->IsA<AStealthCharacter>())
		return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAchievementSubsystem* AchievementSubsystem = GI->GetSubsystem<UAchievementSubsystem>())
		{
			AchievementSubsystem->UnlockAchievement(AchievementID); 
			bHasTriggered = true;
		}
	}
}

