// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
*/

UENUM(BlueprintType)
enum class SpecificKeycard : uint8
{
	None    UMETA(DisplayName = "None"),
	Blue     UMETA(DisplayName = "Blue"),
	Green  UMETA(DisplayName = "Green"),
	Yellow  UMETA(DisplayName = "Yellow"),
	Wrench  UMETA(DisplayName = "Wrench"),
};
