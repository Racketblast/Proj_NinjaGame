// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableWeapon.generated.h"

class AStealthCharacter;

UCLASS()
class PROJ_NINJAGAME_API AThrowableWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowableWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void Throw(AStealthCharacter* Player);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AThrowableObject> ThrownWeaponObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowDamage = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBreakOnImpact = true;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
