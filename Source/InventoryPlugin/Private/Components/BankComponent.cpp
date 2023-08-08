// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/BankComponent.h"

#include "BagStorage.h"
#include "InventoryUtilities.h"
#include "Items/InventoryItemBase.h"
#include "Net/UnrealNetwork.h"

UBankComponent::UBankComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::OnRep_BankPool()
{
	BankPoolDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

int32 UBankComponent::GetItemAtIndex(int32 ID) const
{
	for (auto& Item : Items)
	{
		if (Item.TopLeftID == ID)
			return Item.ItemID;
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::Reorganize_Implementation()
{
	struct ItemSorter
	{
		int32 ItemId;
		uint8 ItemSize;
	};

	TArray<ItemSorter> ItemArray;
	ItemArray.Reserve(Items.Num());

	for (FMinimalItemStorage& Item : Items)
	{
		ItemArray.Add({
			Item.ItemID, static_cast<uint8>(UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld())->ItemSize)
		});
	}

	ItemArray.Sort([](const ItemSorter& Item1, const ItemSorter& Item2)
	{
		return Item1.ItemSize > Item2.ItemSize;
	});

	Items.Empty();

	GridBagSolver Solver(Width, Height);
	for (auto& Item : ItemArray)
	{
		const auto ActualItem = UInventoryUtilities::GetItemFromID(Item.ItemId, GetWorld());
		const int32 TopLeft = Solver.GetFirstValidTopLeft(ActualItem);

		if (TopLeft >= 0)
		{
			Items.Add({Item.ItemId, TopLeft});
			Solver.RecordData(ActualItem, TopLeft);
		}
	}

	BankReorganizeDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::RemoveItem_Implementation(int32 TopLeftIndex)
{
	int32 ID = 0;
	for (const auto& Item : Items)
	{
		if (Item.TopLeftID == TopLeftIndex)
		{
			Items.RemoveAt(ID);
			break;
		}

		++ID;
	}

	BankItemRemoveDispatcher.Broadcast(TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::AddItem_Implementation(int32 ItemID, int32 TopLeftIndex)
{
	Items.Add({ItemID, TopLeftIndex});
	BankItemAddDispatcher.Broadcast(ItemID, TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBankComponent, Items);
}
