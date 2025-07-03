// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoicelineManager.h"
#include "BaseRunner.h"
#include "DSP/Filter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Babylon.generated.h"

/**
 * 
 */
class UNavigationPath;
UCLASS()
class UPR_FINAL_API UBabylon : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public :

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, category = "Babylon")
	bool CanPrintDebug; 
	//Check si on est dans l'éditeur ou non
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Babylon")	
	static bool IsInEditor();

	//Convertit de EPlayableCharacters a EDialogAbleCharacters
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Babylon")
	static TEnumAsByte<EDialogAbleCharacters>PlayableToDialogRunner (TEnumAsByte<EPlayableRunner> Playable);

	//Trouve les nom des Runners dans un string
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Babylon")
	static FName FindRunnerName (FString InString);


	

	
};
