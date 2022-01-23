// Copyright 2021 Maximilien (Synock) Guislain


#include "Inventory/EquipmentComponent.h"
#include <Net/UnrealNetwork.h>

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	FBareItem invalidItem;
	invalidItem.Key = -1;
	Equipment.Init(invalidItem, 32);
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(UEquipmentComponent, Equipment);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::OnRep_ItemList()
{
	EquipmentDispatcher.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Equipment list modified"));
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipItem(const FBareItem& Item, InventorySlot InSlot)
{
	UE_LOG(LogTemp, Log, TEXT("Equipping item %s (%d) in slot %d"), *Item.Name, Item.Key, InSlot);

	if (Equipment[static_cast<int>(InSlot)].Key < 0)
	{
		Equipment[static_cast<int>(InSlot)] = Item;
		EquipmentDispatcher_Server.Broadcast();
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::IsSlotEmpty(InventorySlot InSlot)
{
	if (Equipment[static_cast<int>(InSlot)].Key < 0)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FBareItem> UEquipmentComponent::GetAllEquipment() const
{
	return Equipment;
}

//----------------------------------------------------------------------------------------------------------------------

const FBareItem& UEquipmentComponent::GetItemAtSlot(InventorySlot InSlot)
{
	return Equipment[static_cast<int>(InSlot)];
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::RemoveItem(InventorySlot InSlot)
{
	if (IsSlotEmpty(InSlot))
		return false;

	FBareItem InvalidItem;
	InvalidItem.Key = -1;
	Equipment[static_cast<int>(InSlot)] = InvalidItem;
	EquipmentDispatcher_Server.Broadcast();
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

float UEquipmentComponent::GetTotalWeight()
{
	float TotalWeight = 0.f;
	for (const auto& Item : Equipment)
	{
		if (Item.Key >= 0)
			TotalWeight += Item.Weight;
	}
	return TotalWeight;
}

//----------------------------------------------------------------------------------------------------------------------

InventorySlot UEquipmentComponent::FindSuitableSlot(const FBareItem& Item)
{
	for (size_t i = 1; i < Equipment.Num(); ++i)
	{
		if (Equipment[i].Key < 0)
		{
			const int32 LocalAcceptableBitMask = std::pow(2., static_cast<double>(i));
			const InventorySlot CurrentSlot = static_cast<InventorySlot>(i);
			if (Item.EquipableSlotBitMask & LocalAcceptableBitMask)
			{
				if(Item.TwoSlotsItem)
				{
					if(CurrentSlot == InventorySlot::WaistBag1 || CurrentSlot == InventorySlot::BackPack1)
						return CurrentSlot;
				}
				else
					return CurrentSlot;
			}
		}
	}

	return InventorySlot::Unknown;
}
