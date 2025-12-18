// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumSavedProperties.h"
#include "GameFramework/Actor.h"
#include "TrophyHubActor.generated.h"

UENUM(BlueprintType)
enum class ETrophyTier : uint8
{
	None,
	Bronze,
	Silver,
	Gold,
	Platinum
};

USTRUCT(BlueprintType)
struct FTrophyScoreThresholds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Bronze = 100;

	UPROPERTY(EditAnywhere)
	int32 Silver = 200;

	UPROPERTY(EditAnywhere)
	int32 Gold = 300;

	UPROPERTY(EditAnywhere)
	int32 Platinum = 400;
};

UCLASS()
class PROJ_NINJAGAME_API ATrophyHubActor : public AActor
{
	GENERATED_BODY()
	
public:
	ATrophyHubActor();
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category="Trophies")
	TMap<EMission, FTrophyScoreThresholds> MissionThresholds;

	UPROPERTY(EditAnywhere, Category="Trophies")
	TMap<ETrophyTier, TSubclassOf<AActor>> TrophyActors;

	UPROPERTY(EditAnywhere, Category="Trophies")
	TMap<EMission, AActor*> MissionTrophyAnchors;

private:
	ETrophyTier GetTrophyTierForScore(int32 Score, const FTrophyScoreThresholds& Thresholds) const;

};
