// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowingMarker.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AThrowingMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowingMarker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetMarkerScale(FVector Size);
	void SetMarkerMesh(UStaticMesh* Mesh);
	void SetMarkerRelativeLocation(FVector Location);
	void AddVFXMarkerRelativeLocation(FVector Location);
	void UpdateSpawnMarkerMesh(TSubclassOf<class AThrowableObject> ObjectClass);
	void SetGroundMaterial();
	void SetEnemyMaterial();
	void SetHeadMaterial();
	UMaterialInterface* GetMeshMaterial() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Marker")
	USceneComponent* MarkerRootComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Marker")
	UStaticMeshComponent* MarkerMesh;
	UPROPERTY(EditDefaultsOnly, Category = "Marker")
	class UNiagaraComponent* MarkerVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UNiagaraSystem* GroundHitVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UNiagaraSystem* EnemyHitVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class UNiagaraSystem* HeadHitVFX;

	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UMaterial* GroundHitMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UMaterial* EnemyHitMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UMaterial* HeadHitMaterial;
};


