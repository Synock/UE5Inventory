// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/BankComponent.h"

#include "BagStorage.h"
#include "InventoryUtilities.h"
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
	TArray<FInventoryItem> ItemArray;
	ItemArray.Reserve(Items.Num());

	for (FMinimalItemStorage& Item : Items)
	{
		ItemArray.Add(UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld()));
	}

	ItemArray.Sort([](const FInventoryItem& Item1, const FInventoryItem& Item2) {
				return  Item1.Size >  Item2.Size;
			});

	GridBagSolver Solver(Width, Height);
	for (auto& Item : ItemArray)
	{
		const int32 TopLeft = Solver.GetFirstValidTopLeft(Item);

		if (TopLeft >= 0)
		{
			Items.Add({Item.ItemID, TopLeft});
			Solver.RecordData(Item, TopLeft);
		}
	}
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
}

//----------------------------------------------------------------------------------------------------------------------

void UBankComponent::AddItem_Implementation(int32 ItemID, int32 TopLeftIndex)
{
	Items.Add({ItemID, TopLeftIndex});
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