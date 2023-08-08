// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/AbstractDroppedItem.h"

#include "Components/CapsuleComponent.h"

AAbstractDroppedItem::AAbstractDroppedItem()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticItem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticItemMesh"));
	StaticItem->SetIsReplicated(true);
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetIsReplicated(true);
	SetRootComponent(StaticItem);

	CapsuleComponent->SetHiddenInGame(false);
	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetupAttachment(StaticItem);

	StaticItem->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

	// Put that how you actually want it
	//CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	//StaticItem->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);


}

void AAbstractDroppedItem::BeginPlay()
{
	Super::BeginPlay();

}
