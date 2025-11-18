// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "StealthCharacter.generated.h"

class AMeleeWeapon;
class AKunaiWeapon;
class AThrowableWeapon;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

//DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
	Walk    UMETA(DisplayName = "Walk"),
	Run     UMETA(DisplayName = "Run"),
	Crouch  UMETA(DisplayName = "Crouch"),
	Climb  UMETA(DisplayName = "Climb")
};

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

	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	EPlayerMovementState CurrentMovementState = EPlayerMovementState::Walk;

	virtual bool CanJumpInternal_Implementation() const override;

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
	void EndMoveInput();

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
public:
	UFUNCTION(BlueprintCallable, Category="Input")
	void AimEnd();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	void Die();
	FTimerHandle TempHandle; // Används i Die funktionen

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	USoundBase* ThrowSound;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float CameraForwardMultiplier = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AThrowableWeapon* HeldThrowableWeapon = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AThrowableWeapon> LastHeldWeapon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int AmountOfKunai = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AKunaiWeapon> KunaiWeapon;
protected:

	//Projectile marker
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AActor* SpawnedMarker;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> MarkerClass;
	
    void UpdateProjectilePrediction();
	
	//Melee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMeleeWeapon> MeleeWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AMeleeWeapon* CurrentMeleeWeapon;
	
	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	int32 MeleeDistance = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	float MeleeDamage = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	float MeleeHitsPerSecond = 0.8;*/
	
	// Sneak
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StealthCrouch;

	void ToggleSneak();

	bool CanUnCrouch();

	// Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintNoiseMultiplier = 4.0f;

	void StartSprint();

	void StopSprint();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SprintAction;

	// Sprint FOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float SprintFOV = 80.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float NormalFOV = 70.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float FOVInterpSpeed = 5.0f; // hur snabbt kameran övergår mellan FOV-värden

	// Sprint Kamera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float CameraBobAmplitude = 1.1f; // hur mycket kameran gungar/skakar

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float CameraBobSpeed = 8.0f; // hur snabbt gungningen sker

	float BobTimer = 0.0f;


	// speed 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SneakWalkSpeed = 450.0f;

	// för SoundUtility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SneakNoiseMultiplier = 0.1f;

	//Climbing
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Climb")
	bool bCanClimb = true;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bMovingForward = false;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bHoldingJump = false;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	EPlayerMovementState RememberedClimbState;

	void Climb();

	//Player Stamina
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stamina")
	float CurrentStamina;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stamina")
	float UpdateStaminaAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRefreshRate = 0.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float RegainStaminaStartTime = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float RegainStaminaAmount = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float SprintStaminaAmount = -2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float ClimbStaminaAmount = -4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float AttackStaminaAmount = -20;

	FTimerHandle StaminaTimer;
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UpdateStaminaStart(float InStamina);
	UFUNCTION()
	void UpdateStaminaLoop();

	virtual void Landed(const FHitResult& Hit) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	EPlayerMovementState GetPlayerMovementState() const { return CurrentMovementState; }
	
	UPROPERTY(BlueprintReadWrite)
	bool bHasCompletedTheMission = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsInCombat = false;

private:
	float MoveInputForward = 0.f;
	float MoveInputRight = 0.f;
};
