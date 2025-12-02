// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyCard.h"

#include "StealthCharacter.h"

AKeyCard::AKeyCard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AKeyCard::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (!Player->KeyCards.Contains(this))
	{
		Player->KeyCards.Add(this);
	}
	
	Destroy();
}

bool AKeyCard::ContainsDoor(class ADoor* Door)
{
	return DoorsToUnlock.Contains(Door);
}

void AKeyCard::BeginPlay()
{
	Super::BeginPlay();
}
