// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "MissionSelectObject.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AMissionSelectObject : public AInteractableObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMissionSelectObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
	
	void SetMarkerWidgetVisibility(bool bCond);
	void FadeMarkerWidget(bool bShow);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TSubclassOf<UUserWidget> MissionWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UWidgetComponent* MarkerWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USphereComponent* MarkerDisappearSphere;
	UFUNCTION()
	void OnDisappearSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void OnDisappearSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
