// Copyright 2024 Maximilien (Synock) Guislain


#include "Components/LoreItemManagerComponent.h"

#include "Items/InventoryItemBase.h"

ULoreItemManagerComponent::ULoreItemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULoreItemManagerComponent::CanSpawnItem(const UInventoryItemBase* InventoryItem) const
{
	return !(KnownLoreItems.Contains(InventoryItem->ItemID) || DelayedLootPools.Contains(InventoryItem->ItemID));
}

bool ULoreItemManagerComponent::DelayedSpawnItem(const UInventoryItemBase* InventoryItem, ULootPoolComponent* Requester)
{
	if (!CanSpawnItem(InventoryItem))
		return true;

	//DelayedLootPools.Add(InventoryItem->ItemID,Requester);

	return false;
}

void ULoreItemManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

