// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeleeWeapon.generated.h"

class AEnemy;

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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AssassinateEnemy();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanMeleeAttack = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bMeleeAttacking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAssassinatingEnemy = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MeleeDamage = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MeleeBoxLength = 32.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MeleeBoxWidth = 32.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MeleeBoxHeight = 32.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AFieldSystemActor> FieldActorClass;
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AFieldSystemActor* FieldActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* HitSound;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SwingSound;
	
protected:
	UPROPERTY(BlueprintReadWrite)
	class AStealthCharacter* Player;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SpawnFieldActor();
	
	AEnemy* GetEnemyClosestToCrosshair(const TArray<AActor*>& HitActors);
	
	FTimerHandle MeleeAttackingTimer;
	UPROPERTY()
	TArray<AActor*> ActorsHit;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
};
