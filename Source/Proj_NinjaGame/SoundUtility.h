// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SoundUtility.generated.h"


UCLASS()
class PROJ_NINJAGAME_API USoundUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void ReportNoise(UWorld* World, FVector Location, float Loudness = 1.0f);
	
	/** Spelar ett ljud på en viss position, jag använder inte detta nu, men kanske kan använda det senare */
	UFUNCTION(BlueprintCallable, Category = "Sound")
	static void PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Volume = 1.f, float Pitch = 1.f);
	
};
