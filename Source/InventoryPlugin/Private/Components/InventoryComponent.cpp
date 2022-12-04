// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/InventoryComponent.h"
#include "BagStorage.h"
#include <Net/UnrealNetwork.h>


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	//Initialize all the bags in the enum, starting from the first valid up to the last valid
	for (uint32_t BagID = 1; BagID < static_cast<uint32_t>(EBagSlot::LastValidBag); ++BagID)
	{
		const EBagSlot BagSlotValue = static_cast<EBagSlot>(BagID);
		FString BagName = "Bag" + FString::FromInt(BagID);
		UBagStorage* Bag = CreateDefaultSubobject<UBagStorage>(*BagName);
		VariableBags.Add({BagSlotValue, Bag});

		if (BagSlotValue == EBagSlot::Pocket1 || BagSlotValue == EBagSlot::Pocket2)
			Bag->InitializeData(BagSlotValue, 3, 2, EItemSize::Medium);

		Bag->SetBagSlot(BagSlotValue);
		Bag->SetNetAddressable();
		Bag->SetIsReplicated(true);
		Bag->BagDispatcher.AddDynamic(this, &UInventoryComponent::DoBroadcastChange);
		Bag->BagStorageDispatcher_Server.AddDynamic(this, &UInventoryComponent::DoServerBroadcastChange);
	}

	//Ensure the LUT is updated locally
	OnRep_ReplicatedBags();
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	for (auto& BagData : VariableBags)
	{
		BagData.Bag->BagDispatcher.AddDynamic(this, &UInventoryComponent::DoBroadcastChange);
	}
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UInventoryComponent::GetBagConst(EBagSlot WantedBagSlot) const
{
	return GetRelatedBag(WantedBagSlot)->GetBagConst();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::OnRep_ReplicatedBags()
{
	BagLUT.Empty();

	for (auto& BagData : VariableBags)
	{
		//UE_LOG(LogTemp, Error, TEXT("Repping bag slot %d"), BagData.Slot);
		BagLUT.Emplace(BagData.Slot, BagData.Bag);
	}
}

void UInventoryComponent::DoBroadcastChange()
{
	FullInventoryDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::DoServerBroadcastChange()
{
	FullInventoryDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

int32 UInventoryComponent::GetItemAtIndex(EBagSlot ConsideredBag, int32 ID) const
{
	return GetRelatedBag(ConsideredBag)->GetItemAtIndex(ID);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::RemoveItem_Implementation(EBagSlot ConsideredBag, int32 TopLeftIndex)
{
	GetRelatedBag(ConsideredBag)->RemoveItem(TopLeftIndex);
	InventoryItemRemove.Broadcast(ConsideredBag,TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::AddItemAt_Implementation(EBagSlot ConsideredBag, int32 ItemID, int32 TopLeftIndex)
{
	GetRelatedBag(ConsideredBag)->AddItemAt(ItemID, TopLeftIndex);
	InventoryItemAdd.Broadcast(ConsideredBag,ItemID,TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::BagSet(EBagSlot ConsideredBag, bool InputValidity, int32 InputWidth, int32 InputHeight,
                                 EItemSize InputMaxStoreSize)
{
	if (ConsideredBag == EBagSlot::Pocket1 || ConsideredBag == EBagSlot::Pocket2)
	{
		check(false);
		return;
	}

	GetRelatedBag(ConsideredBag)->InitializeData(ConsideredBag, InputWidth, InputHeight,
	                                             InputMaxStoreSize);
	GetRelatedBag(ConsideredBag)->SetBagValidity(InputValidity);
}

//----------------------------------------------------------------------------------------------------------------------

float UInventoryComponent::GetTotalWeight() const
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

bool UInventoryComponent::HasItem(int32 ItemID)
{
	for (auto& BagData : VariableBags)
	{
		if(BagData.Bag->HasItem(ItemID))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

EBagSlot UInventoryComponent::FindSuitableSlot(const FInventoryItem& Item, int32& OutputTopLeftID) const
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

	return EBagSlot::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

EEquipmentSlot UInventoryComponent::GetInventorySlotFromBagSlot(EBagSlot ConsideredBag)
{
	switch (ConsideredBag)
	{
	case EBagSlot::WaistBag1: return EEquipmentSlot::WaistBag1;
	case EBagSlot::WaistBag2: return EEquipmentSlot::WaistBag2;
	case EBagSlot::BackPack1: return EEquipmentSlot::BackPack1;
	case EBagSlot::BackPack2: return EEquipmentSlot::BackPack2;
	default: return EEquipmentSlot::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

EBagSlot UInventoryComponent::GetBagSlotFromInventory(EEquipmentSlot ConsideredInventory)
{
	switch (ConsideredInventory)
	{
	case EEquipmentSlot::WaistBag1: return EBagSlot::WaistBag1;
	case EEquipmentSlot::WaistBag2: return EBagSlot::WaistBag2;
	case EEquipmentSlot::BackPack1: return EBagSlot::BackPack1;
	case EEquipmentSlot::BackPack2: return EBagSlot::BackPack2;
	default: return EBagSlot::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

UBagStorage* UInventoryComponent::GetRelatedBag(EBagSlot InputSlot) const
{
	if (BagLUT.Contains(InputSlot))
		return BagLUT.FindRef(InputSlot);

	check(false);
	UE_LOG(LogTemp, Error, TEXT("Cannot find related bag"));
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Bags content are only relevant to the user
	DOREPLIFETIME_CONDITION(UInventoryComponent, VariableBags, COND_OwnerOnly);
}
