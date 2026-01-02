// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeEnemy.h"
#include "TargetEnemy.h"
#include "BodyguardEnemy.generated.h"


UCLASS()
class PROJ_NINJAGAME_API ABodyguardEnemy : public AMeleeEnemy
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bodyguard")
	ATargetEnemy* ProtectedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bodyguard")
	float FollowDistance = 250.f;
};
