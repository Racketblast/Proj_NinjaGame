// Fill out your copyright notice in the Description page of Project Settings.


#include "SmokeBombObject.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

ASmokeBombObject::ASmokeBombObject()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	SmokeComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeComponent"));
    SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokeComponent->SetupAttachment(SphereComp);
	SmokeComponent->SetAutoActivate(false);
}

void ASmokeBombObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                                       UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SmokeComponent->SetAsset(SmokeEffect);
	SmokeComponent->Activate();
}
