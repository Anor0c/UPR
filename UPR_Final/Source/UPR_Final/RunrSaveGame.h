// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RunrSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UPR_FINAL_API URunrSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "Saved")
	TArray<float> SoloTimeScores;
	
	UPROPERTY(BlueprintReadWrite, Category = "Saved")
	FString SaveSlotName; 

	UPROPERTY(BlueprintReadWrite, Category = "Saved")
	int32 UserIndex;

	UFUNCTION(BlueprintCallable, Category = "Saved")
	TArray<float> OrganizeTimes(TArray<float> TimeToOrganize); 

	URunrSaveGame();

};
