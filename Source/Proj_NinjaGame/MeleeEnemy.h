// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MeleeEnemy.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AMeleeEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AMeleeEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float VisionRange = 1500.f;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float VisionAngle = 45.f;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float LoseSightTime = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "AI|Patrol")
	float WaitTimeAtPoint = 2.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Patrol")
	float SearchTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 500.f;

	UPROPERTY()
	APawn* PlayerPawn;

	bool bCanSeePlayer = false;

	void CheckPlayerVisibility();

public:
	FORCEINLINE const TArray<AActor*>& GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE bool CanSeePlayer() const { return bCanSeePlayer; }
	FORCEINLINE float GetLoseSightTime() const { return LoseSightTime; }
	FORCEINLINE float GetWaitTimeAtPoint() const { return WaitTimeAtPoint; }
	FORCEINLINE float GetSearchTime() const { return SearchTime; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }

	void UpdateLastSeenPlayerLocation();
	FVector GetLastSeenPlayerLocation() const { return LastSeenPlayerLocation; }

private:
	FVector LastSeenPlayerLocation;

};
