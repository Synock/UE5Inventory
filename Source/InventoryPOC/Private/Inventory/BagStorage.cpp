// Copyright 2021 Maximilien (Synock) Guislain


#include "Inventory/BagStorage.h"
#include "CommonUtilities.h"
#include <Net/UnrealNetwork.h>


GridBagSolver::GridBagSolver(int32 InputWidth, int32 InputHeight): Width(InputWidth), Height(InputHeight)
{
	Grid.Init(nullptr, Width * Height);
}

//----------------------------------------------------------------------------------------------------------------------

void GridBagSolver::RecordData(const FBareItem& Item, int32 TopLeft)
{
	if (TopLeft >= 0)
	{
		const int SX = TopLeft % Width;
		const int SY = TopLeft / Width;

		for (int y = SY; y < SY + Item.Height; ++y)
		{
			for (int x = SX; x < SX + Item.Width; ++x)
			{
				Grid[x + y * Width] = &Item;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool GridBagSolver::IsRoomAvailable(const FBareItem& Item, int TopLeftIndex)
{
	const int SX = TopLeftIndex % Width;
	const int SY = TopLeftIndex / Width;

	for (int y = SY; y < SY + Item.Height; ++y)
	{
		for (int x = SX; x < SX + Item.Width; ++x)
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

int32 GridBagSolver::GetFirstValidTopLeft(const FBareItem& Item)
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
	UE_LOG(LogTemp, Log, TEXT("CALL FROM OnRep_BagData"));
}

//----------------------------------------------------------------------------------------------------------------------

bool UBagStorage::InitializeData(BagSlot InputBagSlot, int32 InputWidth, int32 InputHeight, EItemSize InputMaxStoreSize)
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
		FBareItem LocalItem = UCommonUtilities::GetItemFromID(Item.ItemID, GetWorld());
		Solver.RecordData(LocalItem, Item.TopLeftID); //we don't care about pointer validity
	}

	return Solver;
}

//----------------------------------------------------------------------------------------------------------------------

int64 UBagStorage::GetItemAtIndex(int32 ID) const
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
	int64 ItemID = 0;
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
	BagWeight -= UCommonUtilities::GetItemFromID(ItemID, GetWorld()).Weight;
	BagStorageDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::AddItemAt_Implementation(int64 ItemID, int32 TopLeftIndex)
{
	Items.Add({ItemID, TopLeftIndex});

	//update the weight
	BagWeight += UCommonUtilities::GetItemFromID(ItemID, GetWorld()).Weight;
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
