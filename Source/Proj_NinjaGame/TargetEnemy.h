// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "MeleeAIController.h"
#include "TargetEnemy.generated.h"

class UBoxComponent;
class UAnimMontage;

UCLASS()
class PROJ_NINJAGAME_API ATargetEnemy : public AEnemy
{
	GENERATED_BODY()

public:
	ATargetEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class ATargetEnemyExit*> RunTowardsExits;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE class AMissionHandler* GetMissionHandler() const { return MissionHandler; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMissionHandler(AMissionHandler* NewMissionHandler) { MissionHandler = NewMissionHandler; }

	// För bodyguard
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetEnemyDied, FVector, DeathLocation);
	UPROPERTY(BlueprintAssignable)
	FOnTargetEnemyDied OnTargetEnemyDied;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMissionHandler* MissionHandler;
	virtual void Die() override;
};
