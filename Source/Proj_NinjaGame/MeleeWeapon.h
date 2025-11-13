// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeleeWeapon.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AMeleeWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeleeWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartMeleeAttack();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void MeleeAttackEnd();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanMeleeAttack = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bMeleeAttacking = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MeleeDamage = 2;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MeleeAttackLoop();
	
	FTimerHandle MeleeAttackingTimer;
	TArray<AActor*> ActorsHit;
	
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* StartOfBladePos;
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* EndOfBladePos;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
};
