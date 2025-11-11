// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthCharacter.generated.h"

class AKunaiWeapon;
class AThrowableWeapon;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

//DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3

UCLASS()
class PROJ_NINJAGAME_API AStealthCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AStealthCharacter();
	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* UseAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* KunaiAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;
	
	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Look(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Move(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	void Attack();

	UFUNCTION(BlueprintCallable, Category="Input")
	void EquipKunai();

	UFUNCTION(BlueprintCallable, Category="Input")
	void AimStart();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	void AimEnd();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	void Die();

	UFUNCTION(BlueprintCallable, Category="Input")
	void Use();
	
	void CheckForUse();
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 3.f;

	//Use Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 UseDistance = 300;
	UPROPERTY(BlueprintReadWrite)
	bool bShowUseWidget = false;
	UPROPERTY(BlueprintReadWrite)
	AActor* LastUseTarget;

	//Weapon variables
	UPROPERTY(BlueprintReadWrite)
	bool bIsAiming;
public:
	UPROPERTY(BlueprintReadWrite)
	AThrowableWeapon* HeldThrowableWeapon = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AThrowableWeapon> LastHeldWeapon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	int AmountOfKunai = 3;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AKunaiWeapon> KunaiWeapon;
	
	//Melee maybe
	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	int32 MeleeDistance = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	float MeleeDamage = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	float MeleeHitsPerSecond = 0.8;*/
	// Sneak 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stealth", meta = (AllowPrivateAccess = "true"))
	bool bIsSneaking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StealthCrouch;

	void ToggleSneak();

	// speed 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float NormalWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SneakWalkSpeed = 250.0f;

	// för SoundUtility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SneakNoiseMultiplier = 0.1f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	UPROPERTY(BlueprintReadWrite)
	bool bHasCompletedTheMission = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsInCombat = false;
};
