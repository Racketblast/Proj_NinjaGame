// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API UObjectiveMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OriginLocation;
	
	UFUNCTION(BlueprintCallable)
	void PlayFadeIn();
	UFUNCTION(BlueprintCallable)
	void PlayFadeOut();

protected:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeAnimation;
	UFUNCTION(BlueprintCallable)
	float GetCurrentFadeTime() const;
};
