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
	Cleared_Mission4,

	Platinum_Mission1,
	Platinum_Mission2,
	Platinum_Mission3,
	Platinum_Mission4,

	UnSeen_Mission1,
	UnSeen_Mission2,
	UnSeen_Mission3,
	UnSeen_Mission4,

	Killed_OneHundred_Enemies,
	Remove_Twenty_Helmets_From_Enemies,
	Headshot_Fifty_Enemies,
	Backstab_Fifty_Enemies,
	Use_The_Toilet
};

