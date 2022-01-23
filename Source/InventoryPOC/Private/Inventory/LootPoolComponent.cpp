// Copyright 2021 Maximilien (Synock) Guislain

#include "Inventory/LootPoolComponent.h"

#include <Net/UnrealNetwork.h>

#include "CommonUtilities.h"
#include "Inventory/BagStorage.h"


// Sets default values for this component's properties
ULootPoolComponent::ULootPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void ULootPoolComponent::Init_Implementation(const TArray<int64>& LootableItems)
{

	TArray<FBareItem> ItemArray;
	ItemArray.Reserve(LootableItems.Num());
	for (int64 ItemID : LootableItems)
	{
		ItemArray.Add(UCommonUtilities::GetItemFromID(ItemID,GetWorld()));
	}

	int32 W = 8;
	int32 H = 8;

	GridBagSolver Solver(W, H);
	for (auto& Item : ItemArray)
	{
		int32 TopLeft = Solver.GetFirstValidTopLeft(Item);

		if (TopLeft >= 0)
		{
			Items.Add({Item.Key, TopLeft});
			Solver.RecordData(Item, TopLeft);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootPoolComponent::OnRep_LootPool()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_LootPool"));
	LootPoolDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootPoolComponent::AddItem_Implementation(int64 ItemID, int32 TopLeftIndex)
{
	Items.Add({ItemID, TopLeftIndex});
}

//----------------------------------------------------------------------------------------------------------------------

int64 ULootPoolComponent::GetItemAtIndex(int32 ID) const
{
	for (auto& Item : Items)
	{
		if (Item.TopLeftID == ID)
			return Item.ItemID;
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------------------------

void ULootPoolComponent::RemoveItem_Implementation(int32 TopLeftIndex)
{
	auto& Bag = Items;
	int32 ID = 0;
	for (const auto& Item : Bag)
	{
		if (Item.TopLeftID == TopLeftIndex)
		{
			Bag.RemoveAt(ID);
			break;
		}

		++ID;
	}
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void ULootPoolComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootPoolComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULootPoolComponent, Items);
}
