// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API UEnemyMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddNewEnemyMarker(AActor* Actor);
};
