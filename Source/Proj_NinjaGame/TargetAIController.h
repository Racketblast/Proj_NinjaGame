// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.h"
#include "TargetAIController.generated.h"
/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ATargetAIController : public AEnemyAIController
{
	GENERATED_BODY()
	ATargetAIController();
	virtual void BeginPlay() override;
protected:
	virtual void HandleChasing(float DeltaSeconds) override;
	virtual void HandleSearching(float DeltaSeconds) override;
	virtual void HandleAlert(float DeltaSeconds) override;
	virtual void HandlePatrolling(float DeltaSeconds) override;
	virtual void StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation) override;
	virtual void StartChasing() override;
	virtual void StopChasing() override;

	FVector GetClosetExit();
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class ATargetEnemy* TargetEnemy;

	bool bIsRunningAway = false;
};
