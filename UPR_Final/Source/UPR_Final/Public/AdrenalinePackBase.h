// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraActor.h"

#include "AdrenalinePackBase.generated.h"

UCLASS()
class UPR_FINAL_API AAdrenalinePackBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAdrenalinePackBase();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	TArray<FTransform> SpawnTransforms;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	UNiagaraSystem* NiagaraSystem;
	
	TArray<ANiagaraActor*> SpawnedNiagaras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	float RotationSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	float BobbleSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	float BobbleIntensity = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	float Magnitude =1.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Parametres")
	float CurrentRot;

	UPROPERTY(BlueprintReadWrite, Category = "Parametres")
	float BobbleVar;

	UPROPERTY(BlueprintReadWrite, Category = "Parametres")
	float BobbleHeight;

	// Time in seconds a picked-up pack stays hidden before reappearing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parametres")
	float HiddenDuration = 3.0f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPackBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	void ShowInstance(int32 InstanceIndex);

	// Instances currently hidden; Tick gives them a zero scale
	TSet<int32> HiddenInstances;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
