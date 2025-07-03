// Fill out your copyright notice in the Description page of Project Settings.


#include "RunrSaveGame.h"
#include "UPR_Final.h"



TArray<float> URunrSaveGame::OrganizeTimes(TArray<float> TimetoOrganize)
{
	TimetoOrganize.Sort([](const float A, const float B)
		{
			return A > B;
		});
	return TimetoOrganize;
}

URunrSaveGame::URunrSaveGame()
{
	UserIndex = 0; 
	SaveSlotName = "DefaultSaveSlot";
}
