// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerUseInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UPlayerUseInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJ_NINJAGAME_API IPlayerUseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerInteraction")
	void Use(class AStealthCharacter* Player);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerInteraction")
	void ShowInteractable(bool bShow);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerInteraction")
	void UpdateShowInteractable();
};
