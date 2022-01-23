// Copyright 2021 Maximilien (Synock) Guislain


#include "Inventory/FullInventoryComponent.h"
#include "Inventory/BagStorage.h"
#include <Net/UnrealNetwork.h>


// Sets default values for this component's properties
UFullInventoryComponent::UFullInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	//Initialize all the bags in the enum, starting from the first valid up to the last valid
	for (uint32_t BagID = 1; BagID < static_cast<uint32_t>(BagSlot::LastValidBag); ++BagID)
	{
		const BagSlot BagSlotValue = static_cast<BagSlot>(BagID);
		FString BagName = "Bag" + FString::FromInt(BagID);
		UBagStorage* Bag = CreateDefaultSubobject<UBagStorage>(*BagName);
		VariableBags.Add({BagSlotValue, Bag});

		if (BagSlotValue == BagSlot::Pocket1 || BagSlotValue == BagSlot::Pocket2)
			Bag->InitializeData(BagSlotValue, 3, 2, EItemSize::Medium);

		Bag->SetBagSlot(BagSlotValue);
		Bag->SetNetAddressable();
		Bag->SetIsReplicated(true);
		Bag->BagDispatcher.AddDynamic(this, &UFullInventoryComponent::DoBroadcastChange);
		Bag->BagStorageDispatcher_Server.AddDynamic(this, &UFullInventoryComponent::DoServerBroadcastChange);
	}

	//Ensure the LUT is updated locally
	OnRep_ReplicatedBags();
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UFullInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UFullInventoryComponent::GetBagConst(BagSlot WantedBagSlot) const
{
	return GetRelatedBag(WantedBagSlot)->GetBagConst();
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::OnRep_ReplicatedBags()
{
	BagLUT.Empty();

	for (auto& BagData : VariableBags)
	{
		UE_LOG(LogTemp, Log, TEXT("Repping bag slot %d"), BagData.Slot);
		BagLUT.Emplace(BagData.Slot, BagData.Bag);
	}
}

void UFullInventoryComponent::DoBroadcastChange()
{
	FullInventoryDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::DoServerBroadcastChange()
{
	FullInventoryDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

int64 UFullInventoryComponent::GetItemAtIndex(BagSlot ConsideredBag, int32 ID) const
{
	return GetRelatedBag(ConsideredBag)->GetItemAtIndex(ID);
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::RemoveItem_Implementation(BagSlot ConsideredBag, int32 TopLeftIndex)
{
	GetRelatedBag(ConsideredBag)->RemoveItem(TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::AddItemAt_Implementation(BagSlot ConsideredBag, int64 ItemID, int32 TopLeftIndex)
{
	GetRelatedBag(ConsideredBag)->AddItemAt(ItemID, TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::BagSet(BagSlot ConsideredBag, bool InputValidity, int32 InputWidth, int32 InputHeight,
                                     EItemSize InputMaxStoreSize)
{
	if (ConsideredBag == BagSlot::Pocket1 || ConsideredBag == BagSlot::Pocket2)
	{
		check(false);
		return;
	}

	GetRelatedBag(ConsideredBag)->InitializeData(ConsideredBag, InputWidth, InputHeight,
	                                             InputMaxStoreSize);
	GetRelatedBag(ConsideredBag)->SetBagValidity(InputValidity);
}

//----------------------------------------------------------------------------------------------------------------------

float UFullInventoryComponent::GetTotalWeight() const
{
	float TotalWeight = 0.f;

	for (const auto& Bag : VariableBags)
	{
		if (Bag.Bag->IsValidBag())
			TotalWeight += Bag.Bag->GetBagWeight();
	}

	return TotalWeight;
}

//----------------------------------------------------------------------------------------------------------------------

BagSlot UFullInventoryComponent::FindSuitableSlot(const FBareItem& Item, int32& OutputTopLeftID)
{
	for (const auto& Bag : VariableBags)
	{
		if (Bag.Bag->IsValidBag())
		{
			GridBagSolver Solver = Bag.Bag->GetSolver();
			OutputTopLeftID = Solver.GetFirstValidTopLeft(Item);

			if (OutputTopLeftID != -1)
				return Bag.Slot;
		}
	}

	return BagSlot::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

InventorySlot UFullInventoryComponent::GetInventorySlotFromBagSlot(BagSlot ConsideredBag)
{
	switch (ConsideredBag)
	{
	case BagSlot::WaistBag1: return InventorySlot::WaistBag1;
	case BagSlot::WaistBag2: return InventorySlot::WaistBag2;
	case BagSlot::BackPack1: return InventorySlot::BackPack1;
	case BagSlot::BackPack2: return InventorySlot::BackPack2;
	default: return InventorySlot::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

BagSlot UFullInventoryComponent::GetBagSlotFromInventory(InventorySlot ConsideredInventory)
{
	switch (ConsideredInventory)
	{
	case InventorySlot::WaistBag1: return BagSlot::WaistBag1;
	case InventorySlot::WaistBag2: return BagSlot::WaistBag2;
	case InventorySlot::BackPack1: return BagSlot::BackPack1;
	case InventorySlot::BackPack2: return BagSlot::BackPack2;
	default: return BagSlot::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

UBagStorage* UFullInventoryComponent::GetRelatedBag(BagSlot InputSlot) const
{
	if (BagLUT.Contains(InputSlot))
		return BagLUT.FindRef(InputSlot);

	check(false);
	UE_LOG(LogTemp, Error, TEXT("Cannot find related bag"));
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UFullInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Bags content are only relevant to the user
	DOREPLIFETIME_CONDITION(UFullInventoryComponent, VariableBags, COND_OwnerOnly);
}
