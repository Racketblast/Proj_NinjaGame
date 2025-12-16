// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EnumSpecificKeycard.h"
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
	//Hide UMETA(DisplayName = "Hide")
};

UENUM(BlueprintType)
enum class EPlayerInteractState : uint8
{
	None    UMETA(DisplayName = "None"),
	Attack     UMETA(DisplayName = "Attack"),
	Throw  UMETA(DisplayName = "Throw"),
	Interact  UMETA(DisplayName = "Interact")
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
public:
	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	EPlayerInteractState CurrentInteractState = EPlayerInteractState::None;
	
protected:
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
	UInputAction* ChangeWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DropAction;
	
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

	FString MovementStateToString(EPlayerMovementState State);

	UFUNCTION(BlueprintCallable, Category="Input")
	void Attack();
	
	UFUNCTION(BlueprintCallable)
	void StartThrow();
	UFUNCTION(BlueprintCallable)
	void StopThrow();

	UFUNCTION(BlueprintCallable, Category="Input")
	void ChangeWeapon();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DropWeapon();

	UFUNCTION(BlueprintCallable, Category="Input")
	void AimStart();
public:
	UFUNCTION(BlueprintCallable, Category="Input")
	void AimEndAction();
	UFUNCTION(BlueprintCallable, Category="Input")
	void AimEndFunction();
	
	UFUNCTION(BlueprintCallable)
	void EquipThrowWeapon(TSubclassOf<AThrowableWeapon> EquipWeapon);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	void Die();
	
	FTimerHandle TempHandle; // Används i Die funktionen

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	UAudioComponent* PlayerVoiceAudioComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* JumpSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* TakeDamageSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	UAudioComponent* PlayerActionAudioComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* ThrowSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* ClimbSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* AttackSound;
	//Use
	UFUNCTION(BlueprintCallable, Category="Input")
	void Use();
	UFUNCTION(BlueprintCallable)
	void StopUse();
	
	void CheckForUse();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* PlayerInteractSphere;
	UFUNCTION()
	void OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Used to stop showing weapons for different cases
	UFUNCTION(BlueprintCallable)
	void ShowActors(AActor* Actor, bool bShow);
	UFUNCTION(BlueprintCallable)
	void ShowWeaponActors(bool bShow);

	//Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 3.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bIsDead = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Stats")
	TSubclassOf<UUserWidget> DeathScreenWidget;
public:
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	
protected:

	//Use Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 UseDistance = 300;
	UPROPERTY(BlueprintReadWrite)
	bool bShowUseWidget = false;
	UPROPERTY(BlueprintReadWrite)
	AActor* LastUseTarget;

	//Weapon variables
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	bool bIsAiming;
	FTimerHandle AimEndTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AimEndTimerSeconds = 0.1f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanAssassinate;
	UPROPERTY(VisibleAnywhere)
	TArray<class AEnemy*> EnemiesInAssassinationRange;
	UFUNCTION()
	void OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void OnMeleeBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void CheckForCanAssassinate();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanThrow = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float CameraForwardMultiplier = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AThrowableWeapon* HeldThrowableWeapon = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AThrowableWeapon> LastHeldWeapon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int AmountOfOwnWeapon = 3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int MaxAmountOfOwnWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AThrowableWeapon> KunaiWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AThrowableWeapon> SmokeBombWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AThrowableWeapon> CurrentOwnThrowWeapon;
	
protected:
	//Projectile marker
	UPROPERTY(BlueprintReadWrite, Category = "Prediction")
	class AThrowingMarker* SpawnedMarker;
	UPROPERTY(EditDefaultsOnly, Category="Prediction")
	class USplineComponent* ThrowSpline;
	
	UPROPERTY(EditDefaultsOnly, Category="Prediction")
	UStaticMesh* PredictionMesh;

	TArray<class USplineMeshComponent*> PredictionMeshSegments;
	
	void ClearPredictionMeshes();
	void BuildSplineMeshes(UMaterialInterface* PredictionMaterial = nullptr);
	
	UPROPERTY(EditDefaultsOnly, Category = "Prediction")
	TSubclassOf<AThrowingMarker> MarkerClass;
	
    void UpdateProjectilePrediction();
	
	//Melee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMeleeWeapon> MeleeWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AMeleeWeapon* CurrentMeleeWeapon;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* PlayerMeleeBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TArray<SpecificKeycard> KeyCards;
protected:
	
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

	void LoopSprint();

	void StopSprint();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* SprintAction;

	// FOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float SprintFOV = 80.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float AimFOV = 80.0f; 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float NormalFOV = 90.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float FOVInterpSpeed = 5.0f; // hur snabbt kameran övergår mellan FOV-värden

	//Is this needed, from here
	// Sprint Kamera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float CameraBobAmplitude = 1.1f; // hur mycket kameran gungar/skakar

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Sprint")
	float CameraBobSpeed = 8.0f; // hur snabbt gungningen sker

	float BobTimer = 0.0f;
	//down to here


	// speed 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SneakWalkSpeed = 450.0f;

	// för SoundUtility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SneakNoiseMultiplier = 0.1f;

	//Climbing
	virtual bool CanCrouch() const override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bIsClimbing = false;
	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bHitLedge = false;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bClimbCapsuleShrunk = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Climb")
	float ClimbRange = 45.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bMovingForward = false;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	bool bHoldingJump = false;

	UPROPERTY(BlueprintReadWrite, Category = "Climb")
	EPlayerMovementState RememberedClimbState;

	void Climb();
	void ExitClimb();

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
	
	bool GetIsDead() const { return bIsDead; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	EPlayerMovementState GetPlayerMovementState() const { return CurrentMovementState; }
	
	AThrowingMarker* GetThrowingMarker() const { return SpawnedMarker; }
	
	UPROPERTY(BlueprintReadWrite)
	bool bHasCompletedTheMission = false;


	// För HideSpot
	void SetCustomCameraLocation(USceneComponent* NewCameraComponent);
	void ResetToNormalCamera();

private:
	float MoveInputForward = 0.f;
	float MoveInputRight = 0.f;

	float PendingYawInput = 0.f;
	float PendingPitchInput  = 0.f;
	float HideLookSpeed = 2.0f;

public:
	// För HideSpot
	bool bIsHiddenFromEnemy = false;
	bool bIsHiding = false;
	
	float HideMinPitch;
	float HideMaxPitch;
	float HideMinYaw;
	float HideMaxYaw;

	FRotator HideBaseRotation;

	// För fienden
	FVector GetLeftArmVisionPoint() const;

	FVector GetRightArmVisionPoint() const; 

};
