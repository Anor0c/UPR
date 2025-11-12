// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SplineMeshBase.generated.h"

UCLASS()
class UPR_FINAL_API ASplineMeshBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplineMeshBase();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AddMeshFunction")
	USplineMeshComponent* AddBodyMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AddMeshFunction")
	USplineMeshComponent* AddStartMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AddMeshFunction")
	USplineMeshComponent* AddEndMesh();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings")
	float MeshLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings")
	FTransform OffsetTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings")
	TEnumAsByte<ESplineMeshAxis::Type> Axis = ESplineMeshAxis::Z ;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings|Meshes")
	UStaticMesh* BodyMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings|Meshes")
	UStaticMesh* StartMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshSettings|Meshes")
	UStaticMesh* EndMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineSettings")
	bool HasStartAndEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineSettings")
	float SpaceBetweenMeshes; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USplineComponent* Spline;

	UPROPERTY()
	class USplineMeshComponent* BodySplineMesh;
	

private :
	TArray<FVector> SplinePoints;
	TArray<USplineMeshComponent> SplineMeshes; 
};
