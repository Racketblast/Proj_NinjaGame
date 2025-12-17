// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealthGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AStealthGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AStealthGameModeBase();
protected:
	virtual void BeginPlay() override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TSubclassOf<UUserWidget> EndGameWidgetClass;
};
