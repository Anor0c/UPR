

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

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EPlayableRunner RunnerTypeEnum; 
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
	bool CanMove = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	bool DEBUGCanPrint = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug", meta = (DisplayName = "DEBUG DrawType"))
	TEnumAsByte<EDrawDebugTrace::Type> DEBUGDrawType = EDrawDebugTrace::None;
#pragma endregion Traversal
};


