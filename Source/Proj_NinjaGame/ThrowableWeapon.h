// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableWeapon.generated.h"

class AStealthCharacter;

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3

UCLASS()
class PROJ_NINJAGAME_API AThrowableWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowableWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void Throw(AStealthCharacter* Player);
	void ThrowObjectLogic(AStealthCharacter* Player);
	
	UFUNCTION(BlueprintCallable)
	virtual void Drop(AStealthCharacter* Player);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowDamage = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowSpeed = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBreakOnImpact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOwnThrowWeapon = false;
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AThrowableObject> ThrownWeaponObject;
protected:
	
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* SceneRootComponent;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
