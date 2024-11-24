// Copyright 2023 Maximilien (Synock) Guislain


#include "Loot/RandomizedEquipmentPool.h"
#include "Items/InventoryItemBase.h"

TArray<int32> URandomizedLootPool::GetRandomisedItems() const
{
	TArray<int32> LootPoolItems;
	for (auto& ItemData : LootPool)
	{
		const float ProbaStatus = FMath::RandRange(0.f, 1.f);
		if (ProbaStatus <= ItemData.Probability)
		{
			LootPoolItems.Add(ItemData.Item->ItemID);
		}
	}

	for (auto& AlternativeElement : AlternativeSet)
	{
		const float ProbaStatus = FMath::RandRange(0.f, 1.f);
		if (ProbaStatus <= AlternativeElement.ExistenceProbability)
		{
			const UInventoryItemBase* Item = AlternativeElement.GetItem();

			if (!Item)
				continue;

			LootPoolItems.Add(Item->ItemID);
		}
	}

	return LootPoolItems;
}

//----------------------------------------------------------------------------------------------------------------------

float URandomizedLootPool::GetRandomisedCoin() const
{
	float LootCoin = 0.f;
	if (FMath::RandRange(0.f, 1.f) < ProbabilityCoin)
	{
		LootCoin = FMath::RandRange(MinCoin, MaxCoin);
	}
	return LootCoin;
}
