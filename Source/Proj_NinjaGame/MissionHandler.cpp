// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionHandler.h"

// Sets default values
AMissionHandler::AMissionHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMissionHandler::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMissionHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

