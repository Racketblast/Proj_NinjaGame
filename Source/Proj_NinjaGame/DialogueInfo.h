// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJ_NINJAGAME_API FDialogueInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* DialogueSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 DialogueFlag = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName NextDialogue = "";
	
	FDialogueInfo() {}

	FDialogueInfo(USoundBase* InSound, int32 InDialogueFlag = 0, FName InNextDialogue = "")
		: DialogueSound(InSound), DialogueFlag(InDialogueFlag), NextDialogue(InNextDialogue)
	{}
};
