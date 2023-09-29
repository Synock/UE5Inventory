// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/InventoryLightSourceActor.h"

AInventoryLightSourceActor::AInventoryLightSourceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WearableLightComponent = CreateDefaultSubobject<UPointLightComponent>("WearableLight");
	//WearableLightComponent->SetNetAddressable();
	//WearableLightComponent->SetIsReplicated(true);
	WearableLightComponent->RecreateRenderState_Concurrent();
	WearableLightComponent->SetVisibility(true);

	StaticItem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticLightObject"));

	SetRootComponent(StaticItem);
	WearableLightComponent->SetupAttachment(StaticItem);
	//StaticItem->SetIsReplicated(true);
}

void AInventoryLightSourceActor::BeginPlay()
{
	Super::BeginPlay();
}
