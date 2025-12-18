// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupWidget.generated.h"

enum class EAchievementId : uint8;
/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API UPopupWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AchievementPopup(EAchievementId Id);
};
