

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BaseRunner.generated.h"




UENUM(BlueprintType)
enum class ECapacityTypeInternal : uint8
{
	Horizontal = 0,
	Vertical = 1,
	Ultimate = 2,
	//ShortTrick = 3, 
	//MediumTrick = 4,
	//LongTrick = 5
};

UENUM(BlueprintType)
enum class EPlayableRunner : uint8
{
	Adler = 0,
	DDos =1,
	Shred = 2,
	Ikigai = 3
};

UENUM(BlueprintType)
enum class EClimbHeight : uint8
{
	NoTraversal = 0,
	Low = 1,
	High = 2
};

UENUM(BlueprintType)
enum class ERunnerAbilities : uint8
{
	Vertical = 0,
	Horizontal = 1,
	Ultimate = 2,
	ShortTrick = 3,
	MediumTrick = 4,
	LongTrick = 5
};

UENUM(BlueprintType)
enum class ERunnerMovementMode : uint8
{
	Idle = 0,
	Running = 1,
	Jumping = 2,
	AirDashing = 3,
	WallRunning = 4,
	Grinding = 5,
	TraversalAction = 6,
	Falling = 7,
	WallJumping = 8
};

UENUM(BlueprintType)
enum class EStartDashQuality : uint8
{
	NoDash = 0,
	BadDash = 1,
	GoodDash = 2,
	PerfectDash = 3
};

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	Left = 0,
	Right = 1,
	None = 2,
	Both = 3
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelAllAbilitiesSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutOfBoundsSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCancelSpecificAbility, ECapacityTypeInternal, CapacityToCancel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityLaunchedSignature,ECapacityTypeInternal, LaunchedAbility,TSoftObjectPtr<ABaseRunner>, Runner );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboUpdatedSignature, int32, CurrentCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExpectedSpeedUpdatedSignature, float, ExpectedSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExpectedAdrenalineUpdatedSignature, float, ExpectedAdrenaline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrindEndedSignature, AActor*, GrindRail);

UCLASS()
class UPR_FINAL_API ABaseRunner : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
#pragma region InheritedAttributes


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Sets default values for this character's properties
	ABaseRunner();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraBoom")
	class UAutoTargetSpringArm* AutoTargetCameraBoom;  */
#pragma endregion 
#pragma region Core
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Core")
	EPlayableRunner RunnerTypeEnum;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Core")
	int32 PlayerIndex = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Core")
	float DefaultGravityScale = 1.75f;

	//Augmente FallingTime et applique la gravité lue dans GravityCurve. A appeler a chaque frame pendant la chute
	UFUNCTION(BlueprintCallable, Category = "Core")
	void GravityAcceleration();

	//Temps passé en chute, utilisé comme abscisse de GravityCurve
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Core")
	double FallingTime = 0.0;
	//GravityScale en fonction de FallingTime
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Core")
	TObjectPtr<class UCurveFloat> GravityCurve;

#pragma endregion
#pragma region Camera
public :
	//Pendant un grind ou un wallrun, aligne progressivement la camera sur le yaw du mesh. Se rappelle a chaque tick jusqu'a la fin du lerp
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetCam();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	bool CanMoveCamera = true;
	//Progression du lerp de ResetCam, entre 0 et 1
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float ResetCameraRotationAlpha = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float ResetCamSpeed = 1.5f;

	//Blend la longueur du bras et le FOV entre le palier de vitesse precedent et le palier actuel. A appeler a chaque frame pendant le blend
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SpeedCapCamera();

	//Longueur du bras de camera pour chaque palier de vitesse
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	TArray<float> TargetArmLengthArray = {300.0f, 375.0f, 450.0f, 525.0f, 600.0f, 675.0f, 1200.0f};
	//FOV de la camera pour chaque palier de vitesse
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	TArray<float> CameraFOVCaps = {106.0f, 106.0f, 108.0f, 110.0f, 112.0f, 114.0f, 120.0f};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	int32 CurrentSpeedCapIndex = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	int32 OldSpeedCapIndex = 0;
	//Temps ecoulé depuis le debut du blend, utilisé directement comme alpha des lerps
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float CurrentCameraBlendTime = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float MaxCameraBlendTime = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	bool IsCameraBlending = false;
#pragma endregion Camera
#pragma region AbilitySystem
protected :
	//Composant Gameplay Ability System
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayAbilitySystem", meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent* GASComponent;
           
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return GASComponent;
	}
           
   	//Doit etre transient pour éviter qu'il disparaisse sur les children
   	//Attribute set utilisé par le runner
   	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "GameplayAbilitySystem", meta = (AllowPrivateAccess = "true"))
   	const class URunnerAttributeSet* RunnerAttributeSet;

	
public:

	//Event Delegate permettant d'annuler toutes les Capacité et tricks (GameplayAbility) active a ce moment
	UPROPERTY(BlueprintAssignable, Category = "AbilityDelegates")
	FOnOutOfBoundsSignature OnOutOfBoundsDelegate;
	//Implémene le delegate precedent
	UFUNCTION(BlueprintCallable, Category = "AbilityDelegates")
	virtual void OnOutOfBounds();


	UPROPERTY(BlueprintAssignable, Category = "AbilityDelegates")
	FOnAbilityLaunchedSignature OnAbilityLaunchedDelegate;
	UFUNCTION(BlueprintCallable, Category = "AbilityDelegates")
	virtual void OnAbilityLaunched(ECapacityTypeInternal LaunchedAbility);
	
	//Event Delegate permettant d'annuler toutes les Capacité et tricks (GameplayAbility) active a ce moment
	UPROPERTY(BlueprintAssignable, Category = "AbilityDelegates")
		FOnCancelAllAbilitiesSignature OnCancelAllAbilitiesDelegate;
	//Implémene le delegate precedent
	UFUNCTION(BlueprintCallable, Category = "AbilityDelegates")
		virtual void CancelAllAbility();
	
	//Similaire a l'event précédant, mais peret de cibler une GameplayAbility spécifique
	UPROPERTY(BlueprintAssignable, Category = "AbilityDelegates")
		FOnCancelSpecificAbility OnCancelSpecificAbilityDelegate;
	//Implémene le delegate precedent
	UFUNCTION(BlueprintCallable, Category = "AbilityDelegates")
		virtual void CancelSpecific(ECapacityTypeInternal CapacityToCancel);	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseRunnerVar")
		TMap<ECapacityTypeInternal, bool> CanAbility ;
	
#pragma endregion

	
#pragma region Input
	
public :
	// Move Input Functions and properties
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D PlayerMoveInput;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool JumpInputPressed = false; 
	
	
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Input")
	 void OnMoveInput(FVector2D Input);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Input")
	void OnMoveInputCompleted();

	//Look Input Functions and Properties 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D PlayerLookInput;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Input")
	void OnLookInput(FVector2D Input);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Input")
	void OnStopLookInput();
#pragma endregion
	
#pragma region Traversal
public : 
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	EClimbHeight TryClimb(FVector& OutPos);
	//Bloque le runner pendant la traversal action puis le teleporte sur LocationTarget. Se rappelle a chaque tick jusqu'a la fin du timer
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PerformClimb(FVector LocationTarget, EClimbHeight InClimbHeight);
	
	//Detecte un mur a gauche et a droite du runner et lance le wallrun si un seul cote touche
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void TryWallRun();
	//Colle le runner au mur tant qu'il detecte un mur et va assez vite. Se rappelle a chaque tick
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void PerformWallRun(EWallRunSide ConnectedSide, FHitResult InHit);
	//finit le wall run
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void EndWallRun();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	EClimbHeight ClimbHeight; 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	EWallRunSide WallConnectedSide; 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	ERunnerMovementMode MovementMode;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	ERunnerAbilities RunnerAbility;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	EStartDashQuality StartDashQuality;
	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool IsWallRunning = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool IsGrinding = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool CanTraversalAction = true;
	//Position de depart de chaque ray du peigne de detection, relative au runner
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	TArray<FVector> TraversalDetectionVerticalPositions = {
		FVector(0.0f, 0.0f,-20.0f ),
		FVector(0.0f, 0.0f,0.0f ),
		FVector(0.0f, 0.0f,20.0f ),
		FVector(0.0f, 0.0f,40.0f ),
		FVector(0.0f, 0.0f, 60.0f),
		FVector(0.0f, 0.0f, 85.0f),
		FVector(0.0f, 0.0f, 110.0f),
		FVector(0.0f, 0.0f, 135.0f),
		FVector(0.0f, 0.0f, 135.0f)
	   };
	//Longueur des rays du peigne de detection
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float TraversalDetectionLength = 75.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	FVector TraversalTarget = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	double TraversalActionTimer = 0.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	double MaxTraversalActionTimer = 1.0;
	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool CanWallRun = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	FVector DetectBehind; 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	FVector DetectForwards;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float WallRunCapsuleRadius = 50.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float WallRunCapsuleHalfHeight = 90.0f;
	//Decalage lateral des capsules de detection du mur
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float CapsuleXYOffset = 20.0f;
	//Multiplicateur du decalage lateral de la capsule de detection avant
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float WallRunForwardDetectOffest = 3.0f;
	//Decalage vertical des capsules de detection du mur
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float CapsuleZOffset = 0.0f;
	//Impulsion vers l'avant au debut du wallrun
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	float WallJumpImpulse = 300.0f;
	//Dernier hit du mur sur lequel le runner court
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	FHitResult WallRunInfo;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool JumpPossible = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool IsFalling = false;
	//Delai avant de pouvoir refaire un wallrun apres en etre sorti
	FTimerHandle WallRunCooldownHandle;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	bool CanMove = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	bool DEBUGCanPrint = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug", meta = (DisplayName = "DEBUG DrawType"))
	TEnumAsByte<EDrawDebugTrace::Type> DEBUGDrawType = EDrawDebugTrace::None;
#pragma endregion Traversal

#pragma region Grind
public :
	//Termine le grind en cours : relance le runner vers l'avant puis broadcast OnGrindEnded.
	//L'override Blueprint coupe GrindFX et l'Audio
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Grind")
	void EndGrind();
	//Fait avancer le runner le long de la spline de RailToGrind et termine le grind au bout du rail. A appeler a chaque frame pendant le grind.
	//L'override Blueprint gere l'Audio du grind
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Grind")
	void PerformGrind();

	//Appelé a la fin d'un grind, avec le rail qui vient d'etre quitté
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Grind")
	FOnGrindEndedSignature OnGrindEnded;

	//Rail sur lequel le runner grind
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	TObjectPtr<AActor> RailToGrind;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	float GrindSpeed = 100.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	bool CanAirDash = true;


	//GrindSpeed en fonction de CurrentSpeedCap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	TObjectPtr<class UCurveFloat> GrindSpeedCurve;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	float CurrentSpeedCap = 0.0f;
	//Hauteur du runner au dessus de la spline, le long de son up vector
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	float GrindZOffset = 150.0f;
	//Position du runner sur la spline du rail
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	float DistanceAlongSpline = 0.0f;
	//Vrai si le runner avance dans le sens de la spline
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	bool IsFolllowingSplineDirection = true;

	//Accroche le runner au rail, le place sur la spline et choisit le sens du grind selon la camera.
	//L'override Blueprint gere GrindFX et l'Audio
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Grind")
	void TryGrind(AActor* InRailToGrind);

	//Point de la spline le plus proche du runner au debut du grind
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	FVector RunnerSplineLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	FRotator SplineRotationAtRunnerLocation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grind")
	float GrindCamBlendAmount = 0.0f;
#pragma endregion Grind

#pragma region Tricks
public :
	//Incremente le combo, termine le trick en cours et broadcast ComboUpdated
	UFUNCTION(BlueprintCallable, Category = "Tricks")
	void IncrementCombo();
	UFUNCTION(BlueprintCallable, Category = "Tricks")
	void UpdateComboValues(float speedAdded, float adrenalineAdded); 
	
	
	//Appelé a chaque fois que le combo de tricks change
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Tricks")
	FOnComboUpdatedSignature ComboUpdated;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	int32 TrickCombo = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	bool IsTricking = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	float CurrentComboExpectedSpeedValue = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	float CurrentComboExpectedAdrenaline = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	float SpeedComboScaling = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tricks")
	float AdrenalineComboScaling = 1.0f;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Tricks")
	FExpectedSpeedUpdatedSignature ExpectedSpeedUpdated;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Tricks")
	FExpectedAdrenalineUpdatedSignature ExpectedAdrenalineUpdated;

	
#pragma endregion Tricks
};


