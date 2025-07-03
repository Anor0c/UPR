// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"
#include "AutoTargetSpringArm.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Camera, HideCategories = (Mobility), meta = (BlueprintSpawnableComponent), MinimalAPI)
class  UAutoTargetSpringArm : public USpringArmComponent
{
	GENERATED_BODY()
public : 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoTargetSystem")
	FVector CameraTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoTargetSystem")
	bool bShouldTarget = false;
	
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FRotator TargetRotation;
	
};
