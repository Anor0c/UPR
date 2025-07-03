

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "BaseRunner.generated.h"


UENUM(BlueprintType)
enum ECapacityTypeInternal
{
	Horizontal = 0,
	Vertical = 1,
	Ultimate = 2
};

UENUM(BlueprintType)
enum EPlayableRunner
{
	Adler_Runner = 0,
	DDos_Runner =1,
	Shred_Runner = 2,
	Ikigai_Runner = 3
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
	TEnumAsByte<EPlayableRunner> RunnerTypeEnum; 
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
	virtual void OnAbilityLaunched(TEnumAsByte<ECapacityTypeInternal> LaunchedAbility);
	
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
		TMap<TEnumAsByte<ECapacityTypeInternal>, bool> CanAbility ;
	
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
};


