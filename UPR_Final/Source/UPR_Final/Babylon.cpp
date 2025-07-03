// Fill out your copyright notice in the Description page of Project Settings.


#include "Babylon.h"

/*bool UBabylon::CanPrintDebug()
{
	return true;
}*/

bool UBabylon::IsInEditor()
{
#if WITH_EDITOR
		return true;
	#else
		return false;
#endif
	
}

TEnumAsByte<EDialogAbleCharacters> UBabylon::PlayableToDialogRunner(TEnumAsByte<EPlayableRunner> Playable)
{
	switch (Playable)
	{
	case EPlayableRunner::Adler_Runner:
		return TEnumAsByte<EDialogAbleCharacters>(1);
	case EPlayableRunner::DDos_Runner :
		return TEnumAsByte<EDialogAbleCharacters>(3);
	case EPlayableRunner::Shred_Runner :
		return TEnumAsByte<EDialogAbleCharacters>(2);
	case EPlayableRunner::Ikigai_Runner :
		return TEnumAsByte<EDialogAbleCharacters>(0);
		default:
			return TEnumAsByte<EDialogAbleCharacters>(0);
	}
}

FName UBabylon::FindRunnerName(FString InString)
{
	if (InString.Contains("Adler"))
	{
		return FName("Adler");
	}
	if (InString.Contains("Shred"))
	{
		return FName("Shred");
	}
	if (InString.Contains("DDos"))
	{
		return FName("DDos");
	}
	if (InString.Contains("Major"))
	{
		return FName("Major");
	}
	else
	{
		return FName("NONAME");
	}
	
}


/*bool UBabylon::IsReachableAI(FVector InLocation)
{
	
}*/






