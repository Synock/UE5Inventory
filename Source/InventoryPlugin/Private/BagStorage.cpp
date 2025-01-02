// Copyright 2022 Maximilien (Synock) Guislain


#include "BagStorage.h"

#include "InventoryUtilities.h"
#include <Net/UnrealNetwork.h>

#include "Items/InventoryItemBase.h"


GridBagSolver::GridBagSolver(int32 InputWidth, int32 InputHeight): Width(InputWidth), Height(InputHeight)
{
	Grid.Init(nullptr, Width * Height);
}

//----------------------------------------------------------------------------------------------------------------------

void GridBagSolver::RecordData(const UInventoryItemBase* Item, int32 TopLeft)
{
	if (TopLeft >= 0)
	{
		const int SX = TopLeft % Width;
		const int SY = TopLeft / Width;

		for (int y = SY; y < SY + Item->Height; ++y)
		{
			for (int x = SX; x < SX + Item->Width; ++x)
			{
				Grid[x + y * Width] = Item;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool GridBagSolver::IsRoomAvailable(const UInventoryItemBase* Item, int TopLeftIndex)
{
	const int SX = TopLeftIndex % Width;
	const int SY = TopLeftIndex / Width;

	for (int y = SY; y < SY + Item->Height; ++y)
	{
		for (int x = SX; x < SX + Item->Width; ++x)
		{
			if (x >= Width || x < 0)
				return false;

			if (y >= Height || y < 0)
				return false;

			const int ID = x + y * Width;

			if (ID < 0 || ID >= Width * Height)
				return false;

			if (Grid[ID] != nullptr) //only look for empty stuff
			{
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

int32 GridBagSolver::GetFirstValidTopLeft(const UInventoryItemBase* Item)
{
	for (int32 i = 0; i < Grid.Num(); ++i)
	{
		if (IsRoomAvailable(Item, i))
			return i;
	}

	return -1;
}

//----------------------------------------------------------------------------------------------------------------------


// Sets default values for this component's properties
UBagStorage::UBagStorage()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::OnRep_BagData()
{
	BagDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

bool UBagStorage::InitializeData(EBagSlot InputBagSlot, int32 InputWidth, int32 InputHeight,
                                 EItemSize InputMaxStoreSize)
{
	if (BagValidity)
	{
		if (Items.Num() > 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Bag was not empty when re-inited %d"), InputBagSlot);
			return false;
		}
	}

	LocalBagSlot = InputBagSlot;
	Width = InputWidth;
	Height = InputHeight;
	MaxStoreSize = InputMaxStoreSize;

	BagValidity = true;
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UBagStorage::GetBagConst() const
{
	return Items;
}

//----------------------------------------------------------------------------------------------------------------------

GridBagSolver UBagStorage::GetSolver() const
{
	GridBagSolver Solver(Width, Height);

	for (auto& Item : Items)
	{
		const UInventoryItemBase* LocalItem = UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld());

		Solver.RecordData(LocalItem, Item.TopLeftID); //we don't care about pointer validity
	}

	return Solver;
}

//----------------------------------------------------------------------------------------------------------------------

bool UBagStorage::HasItem(int32 ItemID)
{
	for (auto& Item : Items)
	{
		if (Item.ItemID == ItemID)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

int32 UBagStorage::CountItems(int32 ItemID)
{
	int32 Count = 0;
	for (auto& Item : Items)
	{
		if (Item.ItemID == ItemID)
			++Count;
	}

	return Count;
}

//----------------------------------------------------------------------------------------------------------------------

int32 UBagStorage::GetFirstTopLeftID(int32 ItemID)
{
	volatile int32 TopLeftID = -1;
	for (const auto& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			TopLeftID = Item.TopLeftID;
			break;
		}
	}

	return TopLeftID;
}

//----------------------------------------------------------------------------------------------------------------------

int32 UBagStorage::GetItemAtIndex(int32 ID) const
{
	for (auto& Item : Items)
	{
		if (Item.TopLeftID == ID)
			return Item.ItemID;
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::RemoveItem_Implementation(int32 TopLeftIndex)
{
	auto& Bag = Items;
	int32 ID = 0;
	int32 ItemID = 0;
	for (const auto& Item : Bag)
	{
		if (Item.TopLeftID == TopLeftIndex)
		{
			ItemID = Item.ItemID;
			Bag.RemoveAt(ID);
			break;
		}

		++ID;
	}

	//update the weight
	BagWeight -= UInventoryUtilities::GetItemFromID(ItemID, GetWorld())->Weight;
	BagStorageDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::AddItemAt_Implementation(int32 ItemID, int32 TopLeftIndex)
{
	Items.Add({ItemID, TopLeftIndex});

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
	if (!Item)
		return;

	//update the weight
	BagWeight += Item->Weight;
	BagStorageDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME_CONDITION(UBagStorage, Items, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, Width, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, Height, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, MaxStoreSize, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, LocalBagSlot, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, BagValidity, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, BagWeight, COND_OwnerOnly);
}
