// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/DroppedCoins.h"


ADroppedCoins::ADroppedCoins()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADroppedCoins::InitializeFromCoinValue(const FCoinValue& Value)
{
	InsideValue = Value;
}

void ADroppedCoins::BeginPlay()
{
	Super::BeginPlay();
}
