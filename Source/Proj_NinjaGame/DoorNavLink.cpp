// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorNavLink.h"

#include "Door.h"

ADoorNavLink::ADoorNavLink()
{
}

void ADoorNavLink::BeginPlay()
{
	Super::BeginPlay();
	
	CopyEndPointsFromSimpleLinkToSmartLink();
	OnSmartLinkReached.AddDynamic(this, &ADoorNavLink::OpenDoor);
}

void ADoorNavLink::OpenDoor(AActor* MovingActor, const FVector& Destination)
{
	if (!DoorAttached)
		return;
	
	if (!DoorAttached->bOpen)
	{
		DoorAttached->OpenCloseDoor();
	}
}
