// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AchievementId.generated.h"


UENUM(BlueprintType)
enum class EAchievementId : uint8
{
	NoDamage_Mission1,
	NoDamage_Mission2,
	NoDamage_Mission3,
	NoDamage_Mission4,

	Cleared_Mission1,
	Cleared_Mission2,
	Cleared_Mission3,
	Cleared_Mission4
};

