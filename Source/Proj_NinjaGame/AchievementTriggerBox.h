// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AchievementId.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "AchievementTriggerBox.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AAchievementTriggerBox : public AActor
{
	GENERATED_BODY()
	
public:	
	AAchievementTriggerBox();

protected:
	virtual void BeginPlay() override;

	/** Trigger box */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievement")
	UBoxComponent* TriggerBox;

	/** Which achievement this trigger unlocks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	EAchievementId AchievementID;

	/** If true, trigger only once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	bool bTriggerOnlyOnce = true;

private:
	bool bHasTriggered = false;

	UFUNCTION()
	void OnTriggerBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

};
