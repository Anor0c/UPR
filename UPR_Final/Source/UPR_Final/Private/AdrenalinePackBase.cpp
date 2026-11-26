// Fill out your copyright notice in the Description page of Project Settings.


#include "UPR_Final/Public/AdrenalinePackBase.h"

#include "NiagaraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UPR_Final/BaseRunner.h"
#include "TimerManager.h"

// Sets default values
AAdrenalinePackBase::AAdrenalinePackBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
	
}

// Called when the game starts or when spawned
void AAdrenalinePackBase::BeginPlay()
{
	Super::BeginPlay();

	InstancedStaticMeshComponent->AddInstances(SpawnTransforms, true, true, true);
	for (int32 i = 0; i < SpawnTransforms.Num(); i++)
	{
		SpawnedNiagaras.Add(GetWorld()->SpawnActor<ANiagaraActor>());
		SpawnedNiagaras[i]->SetActorTransform(SpawnTransforms[i]);
		SpawnedNiagaras[i]->GetNiagaraComponent()->SetAsset(NiagaraSystem); 
	}
	OnActorBeginOverlap.AddDynamic(this, &AAdrenalinePackBase::OnPackBeginOverlap);
}

void AAdrenalinePackBase::OnPackBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	ABaseRunner* Runner = Cast<ABaseRunner>(OtherActor);
	if (!Runner)
	{
		return;
	}

	// Actor overlap doesn't say which instance was touched, so query the instances around the runner's capsule
	const UCapsuleComponent* Capsule = Runner->GetCapsuleComponent();
	const TArray<int32> TouchedInstances = InstancedStaticMeshComponent->GetInstancesOverlappingSphere(
		Capsule->GetComponentLocation(), Capsule->GetScaledCapsuleHalfHeight(), true);

	for (const int32 InstanceIndex : TouchedInstances)
	{
		if (HiddenInstances.Contains(InstanceIndex))
		{
			continue;
		}
		HiddenInstances.Add(InstanceIndex);
		Runner->UpdateAdrenaline(Magnitude); 

		FTimerHandle ShowTimerHandle;
		GetWorldTimerManager().SetTimer(ShowTimerHandle,
			FTimerDelegate::CreateUObject(this, &AAdrenalinePackBase::ShowInstance, InstanceIndex),
			HiddenDuration, false);
	}
}

void AAdrenalinePackBase::ShowInstance(int32 InstanceIndex)
{
	HiddenInstances.Remove(InstanceIndex);
}

// Called every frame
void AAdrenalinePackBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentRot += DeltaTime * RotationSpeed;
	BobbleVar += DeltaTime * BobbleSpeed;
	BobbleHeight = FMath::Sin(BobbleVar) * BobbleIntensity;

	// SpawnTransforms are the base pose of each instance (added in BeginPlay), so the bobble never drifts
	const int32 InstanceCount = FMath::Min(SpawnTransforms.Num(), InstancedStaticMeshComponent->GetInstanceCount());
	if (InstanceCount == 0)
	{
		return;
	}

	TArray<FTransform> NewTransforms;
	NewTransforms.Reserve(InstanceCount);
	for (int32 Index = 0; Index < InstanceCount; Index++)
	{
		const FTransform& BaseTransform = SpawnTransforms[Index];

		FVector Location = BaseTransform.GetLocation();
		Location.Z += BobbleHeight;
		FRotator Rotation = BaseTransform.Rotator();
		Rotation.Yaw += CurrentRot;

		// ISM has no per-instance visibility: a zero scale hides the instance and removes its collision
		const FVector Scale = HiddenInstances.Contains(Index) ? FVector::ZeroVector : BaseTransform.GetScale3D();

		NewTransforms.Add(FTransform(Rotation, Location, Scale));
	}

	// Single batched update instead of one call per instance, render state marked dirty once
	InstancedStaticMeshComponent->BatchUpdateInstancesTransforms(0, NewTransforms, true, true);
}

