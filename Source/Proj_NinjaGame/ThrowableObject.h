// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "ThrowableObject.generated.h"

class UGeometryCollection;
class AThrowableWeapon;
/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AThrowableObject : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AThrowableObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
	virtual void ShowInteractable_Implementation(bool bShow) override;

	UPROPERTY(BlueprintReadWrite)
	bool Thrown = false;
	UPROPERTY(BlueprintReadWrite)
	float DealtDamage = 5.f;
	UPROPERTY(BlueprintReadWrite)
	FVector ThrowVelocity = {0,0,0};
	UPROPERTY(BlueprintReadWrite)
	bool bBreaksOnImpact = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AThrowableWeapon> ThrowableWeapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ImpactGroundSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ImpactEnemySound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundAttenuation* ThrowableAttenuation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UGeometryCollection* ImpactDebris;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AFieldSystemActor> FieldActorClass;
	UPROPERTY(BlueprintReadWrite)
	AFieldSystemActor* FieldActor;
	
	UFUNCTION(BlueprintCallable)
	void SpawnFieldActor();
	
	UFUNCTION()
	void  ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void HandlePickup(class AStealthCharacter* Player);

	virtual void DestroyObject();
};

