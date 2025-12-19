// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetEnemyExit.generated.h"

UCLASS()
class PROJ_NINJAGAME_API ATargetEnemyExit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATargetEnemyExit();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* EnemyHitBox;

	UFUNCTION(BlueprintCallable)
	void PlayerLoses();
protected:
	UFUNCTION()
	void EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TSubclassOf<UUserWidget> LoseScreen;

	bool bPlayerHasLost = false;
};
