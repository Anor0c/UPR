

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Traversal")
	EClimbHeight ClimbHeight; 
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	EClimbHeight TryClimb(FVector& OutPos); 
#pragma endregion Traversal
};


