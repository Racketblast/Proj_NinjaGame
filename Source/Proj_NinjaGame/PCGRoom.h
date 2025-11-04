// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	None            UMETA(DisplayName = "None"),
	Start          UMETA(DisplayName = "Start"),
	End            UMETA(DisplayName = "End"),
};

class PROJ_NINJAGAME_API PCGRoom
{
public:
	PCGRoom();
	~PCGRoom();
	
	int Entrances = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int Enemies = 0;
};
