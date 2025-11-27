// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorNavLink.h"

#include "Door.h"
#include "NavLinkCustomComponent.h"

ADoorNavLink::ADoorNavLink()
{
}

void ADoorNavLink::BeginPlay()
{
	Super::BeginPlay();
	
	UNavLinkCustomComponent* SmartLink = GetSmartLinkComp();
	if (!SmartLink)
	{
		return;
	}

    
	if (PointLinks.Num() > 0)
	{
		const FNavigationLink& Link = PointLinks[0];

		SmartLink->SetLinkData(Link.Left, Link.Right, Link.Direction);
	}

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
