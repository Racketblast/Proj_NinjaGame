// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "ExtractObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AExtractObject : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AExtractObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	class AEnemyHandler* EnemyHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	class AMissionHandler* MissionHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TSubclassOf<UUserWidget> ExitWidgetClass;
};
