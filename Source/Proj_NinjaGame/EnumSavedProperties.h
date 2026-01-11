// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
*/
UENUM(BlueprintType)
enum class EMission : uint8
{
	None				UMETA(DisplayName = "None"),
	TutorialMission		UMETA(DisplayName = "TutorialMission"),
	FirstMission		UMETA(DisplayName = "FirstMission"),
	SecondMission		UMETA(DisplayName = "SecondMission"),
	ThirdMission		UMETA(DisplayName = "ThirdMission"),
	FourthMission		UMETA(DisplayName = "FourthMission"),
	FifthMission		UMETA(DisplayName = "FifthMission"),
};

UENUM(BlueprintType)
enum class EPlayerOwnThrowWeapon : uint8
{
	None    UMETA(DisplayName = "None"),
	Kunai     UMETA(DisplayName = "Kunai"),
	SmokeBomb  UMETA(DisplayName = "SmokeBomb"),
};
