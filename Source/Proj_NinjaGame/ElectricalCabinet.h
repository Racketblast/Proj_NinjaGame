// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "ElectricalCabinet.generated.h"
/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AElectricalCabinet : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AElectricalCabinet();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bPowerOn = true;

	void TurnPowerOnOff();

	void TurnLightsOnOff(bool bOnOff);
	
	void TurnCamerasOnOff(bool bOnOff);
	
	void ReduceEnemySight(bool bOnOff);
	
	void SendClosetEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affected")
	TArray<class ALight*> LightsToTurnOff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affected")
	class AEnemyHandler* EnemyHandler;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* EnemyHitBox;

	
	UFUNCTION()
	void EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// För att skicka en ny fiende ifall den första misslyckas med att sätta på den igen. 
	FTimerHandle RetryEnemySendTimer;

	UPROPERTY(EditAnywhere, Category="Electrical Cabinet")
	float RetrySendInterval = 60.f;
	
	void RetrySendEnemy();
};
