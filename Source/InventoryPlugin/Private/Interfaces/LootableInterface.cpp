// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/LootableInterface.h"
#include "Components/CoinComponent.h"
#include "Components/LootPoolComponent.h"
#include "Interfaces/PurseInterface.h"
#include "GameFramework/Actor.h"

void ILootableInterface::InitLootPool(const TArray<int32>& LootableItems)
{
	GetLootPoolComponent()->Init(LootableItems);
}

//----------------------------------------------------------------------------------------------------------------------

void ILootableInterface::InitCashPool(const FCoinValue& CoinValue)
{
	GetCoinComponent()->AddCoins(CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void ILootableInterface::StartLooting(AActor* Looter)
{
	if (GetIsBeingLooted())
		return;

	IPurseInterface* Purse = Cast<IPurseInterface>(Looter);
	if (!Purse)
		return;

	SetIsBeingLooted(true);
	Purse->ReceiveCoinAmount(GetCoinComponent()->GetPurseContent());
	GetCoinComponent()->ClearPurse();
}

//----------------------------------------------------------------------------------------------------------------------

void ILootableInterface::StopLooting(AActor* Looter)
{
	SetIsBeingLooted(false);
	if (GetIsDestroyable())
	{
		if (GetLootPoolComponent()->IsEmpty())
			DestroyLootActor();
	}
}

int32 ILootableInterface::GetItemData(int32 TopLeftID) const
{
	return GetLootPoolComponentConst()->GetItemAtIndex(TopLeftID);
}

void ILootableInterface::RemoveItem(int32 TopLeftID)
{
	GetLootPoolComponent()->RemoveItem(TopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

FOnLootPoolChangedDelegate& ILootableInterface::GetLootPoolDelegate()
{
	return GetLootPoolComponent()->LootPoolDispatcher;
}
