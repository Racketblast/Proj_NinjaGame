// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectricalCabinet.h"

AElectricalCabinet::AElectricalCabinet()
{
}

void AElectricalCabinet::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	TurnPowerOnOff();
}

void AElectricalCabinet::BeginPlay()
{
	Super::BeginPlay();
}

void AElectricalCabinet::TurnPowerOnOff()
{
}
