// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RunnerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UPR_FINAL_API URunnerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	// Liste des attributs pour le character jouable
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Speed;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, Speed)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, MaxSpeed)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Acceleration;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, Acceleration)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Adrenaline;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, Adrenaline)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxAdrenaline;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, MaxAdrenaline)

		UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AdrenalineGainRate;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, AdrenalineGainRate)
};