// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseRunner.h"
#include "AbilitySystemComponent.h"
#include "AutoTargetSpringArm.h"
#include "RunnerAttributeSet.h"


// Sets default values
ABaseRunner::ABaseRunner()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Setup GASComponent
	GASComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("GameplayAbilitySystemComponent"));

	//Setup CameraBoom
	/*AutoTargetCameraBoom = CreateDefaultSubobject<UAutoTargetSpringArm>(TEXT("Camera Boom"));
	AutoTargetCameraBoom->TargetArmLength = 375.0f;
	AutoTargetCameraBoom->bAutoActivate = true;
	AutoTargetCameraBoom->SocketOffset = FVector(0.0f,0.0f,110.0f);*/
}

// Called when the game starts or when spawned
void ABaseRunner::BeginPlay()
{
	if (IsValid(GASComponent))
	{
		RunnerAttributeSet = GASComponent->GetSet<URunnerAttributeSet>();
	}

	Super::BeginPlay();

}

// Called every frame
void ABaseRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseRunner::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void ABaseRunner::CancelSpecific(ECapacityTypeInternal CapacityToCancel)
{
	OnCancelSpecificAbilityDelegate.Broadcast(CapacityToCancel);
}

void ABaseRunner::OnOutOfBounds()
{
	OnOutOfBoundsDelegate.Broadcast();
}

void ABaseRunner::OnAbilityLaunched(TEnumAsByte<ECapacityTypeInternal> LaunchedAbility)
{
	OnAbilityLaunchedDelegate.Broadcast(LaunchedAbility, this);  
}

void ABaseRunner::CancelAllAbility()
{
	OnCancelAllAbilitiesDelegate.Broadcast();
}





