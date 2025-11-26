// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "Door.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ADoor : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ADoor();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Interact")
	bool bOpen = false;
	void OpenCloseDoor();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	bool bNeedsToBeUnlocked = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Interact")
	bool bIsMoving = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	FRotator OpenDoorRotation{0,90,0};
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Interact")
	FRotator ClosedDoorRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	float DoorSpeed = 90.f;
	
	FRotator DoorTargetRotation;
	
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	USoundBase* UnlockSound;
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	USoundBase* LockedSound;
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	USoundBase* DoorOpenSound;
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	UAudioComponent* LockSoundComponent;
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	UAudioComponent* DoorSoundComponent;

	void UpdateNavMeshOnce();
	bool CanPushCharacter(ACharacter* Character, FVector PushDir, float PushDistance);
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* DoorMesh;
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* DoorHitBox;
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* DoorNavLinkPos;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ADoorNavLink> DoorNavLinkClass;
	UPROPERTY()
	ADoorNavLink* DoorNavLink;
	UPROPERTY(EditDefaultsOnly)
	class UNavModifierComponent* DoorNavModifierComponent;
	
	UFUNCTION()
	void DoorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
	void DoorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(BlueprintReadWrite, Category="Interact")
	ACharacter* BlockingCharacter = nullptr;
};
