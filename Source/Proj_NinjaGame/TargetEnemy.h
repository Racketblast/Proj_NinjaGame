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

};
