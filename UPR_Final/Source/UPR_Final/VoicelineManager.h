// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoundCue.generated.h"
#include "VoicelineManager.generated.h"

//Le Include de Soundcue est requis pour la build meme si rider le flag comme inutile

UENUM(BlueprintType, meta=(ScriptName="InternalDialoguable"))
enum EDialogAbleCharacters
{
	None = 0,
	Adler = 1,
	Shred = 2,
	DDos = 3,
	Major = 4
};

UENUM()
enum PlayerNumberIndicator
{
	PlayerNumberIrrelevant,
	Solo,
	Multi

};

UENUM()
enum EDialogPart
{
	DialogPartIrrelevant,
	Talking,
	Answer
};
UENUM(BlueprintType)
enum EVoicelineSituation
{
	Miscellaneous=0,
	StartRace=1,
	EndRace=2,
	DemiTour=3,
	UpPlace=4,
	DownPlace=5,
	OutOfBounds=6,
	Spell_Vertical=7,
	Spell_Horizontal=8,
	Spell_Ultimate=9
};
USTRUCT(BlueprintType)
struct FVoicelineData: public FTableRowBase
{
	//Struct qui gere les replique 
	GENERATED_BODY()
	
	//Fichier son  de la replique
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundCue* VoicelineCue = nullptr;
	
	//Priorité de la réplique: si 2 répliques sont lancé en meme temps,la plus ahute priorité gagne
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 Priority = 0;
	
	//Probabilité qua la réplique soit vrament jouée quand elle est appelée
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin=0.0f, ClampMax = 100.0f))
	float PlayProbability = 100.0f;
	
};
USTRUCT(BlueprintType)
struct FVoicelinePlayInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OptionalName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EVoicelineSituation>  PlaySituation = Miscellaneous;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<PlayerNumberIndicator> PlayerNumberIndic = PlayerNumberIrrelevant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EDialogPart> DialogPart = DialogPartIrrelevant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EDialogAbleCharacters>  TalkingTo = None;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPR_FINAL_API UVoicelineManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVoicelineManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (RequiredAssetDataTags = "RowStructure=/Script/UPR_Final.VoicelineData"))
	UDataTable* VoicelineTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess=true))
	FVoicelinePlayInfo NextVoicelineFilters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FVoicelineData LastVoicelineInfo; 
	

};
