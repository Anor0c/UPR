// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineMeshBase.h"

#include "Components/SplineComponent.h"

// Sets default values
ASplineMeshBase::ASplineMeshBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

}

void ASplineMeshBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	USplineMeshComponent* CurrentSplineMesh = nullptr; 
	SplinePoints.Empty();
	SplineMeshes.Empty();
	// je dois initier ma spline avant cette ligne 
	int SplineMeshSlots =  floor(Spline->GetSplineLength()/MeshLength)-1; 
	for (int i = 0; i < SplineMeshSlots; i++)
	{
		if (i==0&&HasStartAndEnd)
		{
			CurrentSplineMesh = AddStartMesh(); 
		}
		else if (i==SplineMeshSlots&&HasStartAndEnd)
		{
			CurrentSplineMesh = AddEndMesh();
		}
		else
		{
			CurrentSplineMesh = AddBodyMesh(); 
		}
		//CurrentSplineMesh->ForwardAxis = Axis; 
	}
	
}

// Called when the game starts or when spawned
void ASplineMeshBase::BeginPlay()
{
	Super::BeginPlay();
	
}

USplineMeshComponent* ASplineMeshBase::AddStartMesh_Implementation()
{
	return Cast<USplineMeshComponent>(AddComponent("StartMesh", true, OffsetTransform, BodySplineMesh, false )); 
}

USplineMeshComponent* ASplineMeshBase::AddBodyMesh_Implementation()
{
	return Cast<USplineMeshComponent> (AddComponent("BodyMesh", true, OffsetTransform, BodySplineMesh, false )); 
}

USplineMeshComponent* ASplineMeshBase::AddEndMesh_Implementation()
{
	return Cast<USplineMeshComponent> (AddComponent("EndMesh", true, OffsetTransform, BodySplineMesh, false));
}



// Called every frame
void ASplineMeshBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

