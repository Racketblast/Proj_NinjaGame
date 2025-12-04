// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "InteractableObject.h"
#include "SprinklerSwitch.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ASprinklerSwitch : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ASprinklerSwitch();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bPowerOn = true;

	void TurnPowerOnOff();

	void TurnWaterOnOff(bool bOnOff);
	
	void ReduceEnemyHearingRange(bool bOnOff);
	
	void SendClosetEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AEnemyHandler* EnemyHandler;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* EnemyHitBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAudioComponent* WaterAudio;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* SprinklerSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class ANiagaraActor*> Sprinklers;
	
	UFUNCTION()
	void EnemyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// För att skicka en ny fiende ifall den första misslyckas med att sätta på den igen. 
	FTimerHandle RetryEnemySendTimer;

	UPROPERTY(EditAnywhere, Category="Electrical Cabinet")
	float RetrySendInterval = 60.f;
	
	void RetrySendEnemy();

	// Fienden som sist skickades 
	UPROPERTY()
	AEnemy* LastSentEnemy = nullptr;
};
