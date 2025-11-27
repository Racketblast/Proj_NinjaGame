// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "DoorNavLink.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ADoorNavLink : public ANavLinkProxy
{
	GENERATED_BODY()
public:
	ADoorNavLink();
	
	UPROPERTY(EditAnywhere)
	class ADoor* DoorAttached = nullptr;
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void OpenDoor(AActor* MovingActor, const FVector& Destination);
};
