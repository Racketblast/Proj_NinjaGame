// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPoint.generated.h"

UCLASS()
class PROJ_NINJAGAME_API APatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APatrolPoint();

	
	// Om denna punkt ska ge fienden en rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bUseCustomRotation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta=(EditCondition="bUseCustomRotation"))
	FRotator CustomRotation;
};
