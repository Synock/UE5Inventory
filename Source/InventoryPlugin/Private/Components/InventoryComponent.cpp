#include "Components/InventoryComponent.h"
#include "BagStorage.h"
#include <Net/UnrealNetwork.h>

#include "InventoryUtilities.h"
#include "Items/Interfaces/InventoryItemAmmoBagInterface.h"
#include "Items/Interfaces/InventoryItemAmmoInterface.h"


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
		Bag->BagDispatcher.AddUniqueDynamic(this, &UInventoryComponent::DoBroadcastChange);
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
		BagData.Bag->BagDispatcher.AddUniqueDynamic(this, &UInventoryComponent::DoBroadcastChange);
		//BagData.Bag->BagUsageStorageChanged.AddUniqueDynamic(this, &UInventoryComponent::InventoryBagUsageChange);

		if (GetOwnerRole() == ROLE_Authority)
		{
			//UE_LOG(LogTemp, Error, TEXT("Adding bag slot %d"), BagData.Slot);
			BagLUT.Emplace(BagData.Slot, BagData.Bag);
			BagData.Bag->SetIsReplicated(true);
			BagData.Bag->SetNetAddressable();
			BagData.Bag->BagStorageDispatcher_Server.AddUniqueDynamic(this, &UInventoryComponent::DoServerBroadcastChange);

			if (BagData.Slot == EBagSlot::Quiver)
			{
				BagData.Bag->BagUsageStorageChanged.AddUniqueDynamic(this, &UInventoryComponent::InventoryBagUsageChange);
			}

			DoServerBroadcastChange();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UInventoryComponent::GetBagConst(EBagSlot WantedBagSlot) const
{
	return GetRelatedBag(WantedBagSlot)->GetBagConst();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::SetItemLockState(EBagSlot BagSlot, int32 TopLeft, bool bLocked)
{
	UBagStorage* Bag = GetRelatedBag(BagSlot);
	if (Bag)
	{
		Bag->SetItemLockState(TopLeft, bLocked);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::OnRep_ReplicatedBags()
{
	BagLUT.Empty();

	for (auto& BagData : VariableBags)
	{
		//UE_LOG(LogTemp, Error, TEXT("Repping bag slot %d"), BagData.Slot);
		BagLUT.Emplace(BagData.Slot, BagData.Bag);
		//BagData.Bag->BagUsageStorageChanged.AddUniqueDynamic(this, &UInventoryComponent::InventoryBagUsageChange);
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

void UInventoryComponent::InventoryBagUsageChange(EBagSlot ConsideredBag, float BagUsage)
{
	InventoryBagUsageChanged.Broadcast(ConsideredBag, BagUsage);
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
	InventoryItemRemove.Broadcast(ConsideredBag, TopLeftIndex);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::UpdateItemDurability(EBagSlot BagSlot, int32 TopLeft, int32 ItemID, float NewDurability)
{
	UBagStorage* Bag = GetRelatedBag(BagSlot);
	if (!Bag)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateItemDurability: Invalid bag slot %d"), static_cast<int32>(BagSlot));
		return false;
	}

	bool bSuccess = Bag->UpdateItemDurability(TopLeft, ItemID, NewDurability);

	// Broadcast delegate if update was successful
	if (bSuccess)
	{
		InventoryItemDurabilityUpdate.Broadcast(BagSlot, ItemID, TopLeft, NewDurability);
	}

	return bSuccess;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::AddItemAt_Implementation(EBagSlot ConsideredBag, int32 ItemID, int32 TopLeftIndex, float Durability)
{
	GetRelatedBag(ConsideredBag)->AddItemAt(ItemID, TopLeftIndex, Durability);
	InventoryItemAdd.Broadcast(ConsideredBag, ItemID, TopLeftIndex, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::BagSet(EBagSlot ConsideredBag, bool InputValidity, int32 InputWidth, int32 InputHeight,
                                 EItemSize InputMaxStoreSize, float WeightReduction)
{
	if (ConsideredBag == EBagSlot::Pocket1 || ConsideredBag == EBagSlot::Pocket2)
	{
		check(false);
		return;
	}

	GetRelatedBag(ConsideredBag)->InitializeData(ConsideredBag, InputWidth, InputHeight,
	                                             InputMaxStoreSize, WeightReduction);
	GetRelatedBag(ConsideredBag)->SetBagValidity(InputValidity);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::QuiverSpecificSetup(EBagSlot ConsideredBag, EAmmoType NewAmmoType)
{
	GetRelatedBag(ConsideredBag)->InitializeQuiverData(NewAmmoType);
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

bool UInventoryComponent::HasAnyItem(const TArray<int32>& ItemID)
{
	for (int32 ID : ItemID)
	{
		if (HasItem(ID))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::HasItem(int32 ItemID)
{
	for (auto& BagData : VariableBags)
	{
		if (BagData.Bag->HasItem(ItemID))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::HasItems(int32 ItemId, int32 ItemAmount)
{
	int32 FoundAmount = 0;
	for (auto& BagData : VariableBags)
	{
		FoundAmount += BagData.Bag->CountItems(ItemId);

		if (FoundAmount >= ItemAmount)
			return true;
	}
	return FoundAmount >= ItemAmount;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::RemoveItemIfPossible(int32 ItemID)
{
	for (auto& BagData : VariableBags)
	{
		if (const int32 TopLeftItemID = BagData.Bag->GetFirstTopLeftID(ItemID); TopLeftItemID >= 0)
		{
			RemoveItem(BagData.Bag->GetBagSlot(), TopLeftItemID);
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::PlayerRemoveAnyItemIfPossible(const TArray<int32>& ItemID)
{
	for (int32 ID : ItemID)
	{
		if (RemoveItemIfPossible(ID))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

EBagSlot UInventoryComponent::FindSuitableSlot(const UInventoryItemBase* Item, int32& OutputTopLeftID) const
{
	for (const auto& Bag : VariableBags)
	{
		if (Bag.Bag->IsValidBag())
		{

			if (Item->ItemSize > Bag.Bag->GetMaxStoreSize())
				continue;

			GridBagSolver Solver = Bag.Bag->GetSolver();
			OutputTopLeftID = Solver.GetFirstValidTopLeft(Item);

			if (const IInventoryItemAmmoBagInterface* Quiver = Cast<IInventoryItemAmmoBagInterface>(Bag.Bag); Quiver)
			{
				if (const IInventoryItemAmmoInterface* Ammo = Cast<IInventoryItemAmmoInterface>(Item); Ammo)
				{
					if (Quiver->GetAmmoType() != Ammo->GetAmmoType())
						continue; // Skip if the item is not compatible with the quiver's ammo type
				}
				else
					continue; // Skip if the item is not compatible with the quiver's ammo type
			}

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
	case EBagSlot::Quiver: return EEquipmentSlot::Ammo;
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
	case EEquipmentSlot::Ammo: return EBagSlot::Quiver;
	default: return EBagSlot::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

TArray<int32> UInventoryComponent::GetAllItems() const
{
	TArray<int32> ItemList;
	for (auto& Bag : VariableBags)
	{
		for (auto& BagItem : Bag.Bag->GetBagConst())
		{
			ItemList.Add(BagItem.ItemID);
		}
	}
	return ItemList;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::RemoveAllItems()
{
	for (auto& Bag : VariableBags)
	{
		TArray<FMinimalItemStorage> LocalBagCopy = Bag.Bag->GetBagConst();
		for (auto& BagItem : LocalBagCopy)
		{
			RemoveItem(Bag.Slot, BagItem.TopLeftID);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::ClearAllBags()
{
	BagLUT.Reset();
	VariableBags.Reset();
}
//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::IsBagValid(EBagSlot InputSlot) const
{
	return BagLUT.Contains(InputSlot);
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

const UBagStorage* UInventoryComponent::GetRelatedBagConst(EBagSlot InputSlot) const
{
	return GetRelatedBag(InputSlot);
}

//----------------------------------------------------------------------------------------------------------------------

float UInventoryComponent::GetBagUsage(EBagSlot Quiver)
{
	if (auto Bag = GetRelatedBagConst(Quiver))
	{
		return Bag->GetBagSlotUsage();
	}

	return 0.f;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::HasCompatibleAmmoInQuiver(EAmmoType Ammo) const
{
	auto AllItemsInQuiver = GetBagConst(EBagSlot::Quiver);

	for (auto& QuiverItem : AllItemsInQuiver)
	{
		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(QuiverItem.ItemID, GetWorld());
		if (const IInventoryItemAmmoInterface* AmmoItem = Cast<IInventoryItemAmmoInterface>(Item))
		{
			if (AmmoItem->GetAmmoType() == Ammo)
			{
				return true; // Found compatible ammo
			}
		}
	}

	return false; // No compatible ammo found
}

//----------------------------------------------------------------------------------------------------------------------

TScriptInterface<IInventoryItemAmmoInterface> UInventoryComponent::RemoveAmmoFromQuiver(EAmmoType Ammo)
{
	auto AllItemsInQuiver = GetBagConst(EBagSlot::Quiver);

	for (auto& QuiverItem : AllItemsInQuiver)
	{
		UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(QuiverItem.ItemID, GetWorld());
		if (IInventoryItemAmmoInterface* AmmoItem = Cast<IInventoryItemAmmoInterface>(Item))
		{
			if (AmmoItem->GetAmmoType() == Ammo)
			{
				RemoveItem(EBagSlot::Quiver, QuiverItem.TopLeftID);
				return TScriptInterface<IInventoryItemAmmoInterface>(Item); // Found compatible ammo
			}
		}
	}

	return nullptr; // No compatible ammo found
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryComponent::CanReceiveAllItems(TArray<UInventoryItemBase*> ItemArray)
{
	if (ItemArray.Num() == 0)
		return true;

	// Lazily create solvers only when we need them for a specific bag
	TMap<EBagSlot, GridBagSolver> TempSolvers;

	// Try to place each item
	for (UInventoryItemBase* Item : ItemArray)
	{
		if (!Item)
			continue;

		bool bItemPlaced = false;

		// Try to find a suitable bag for this item
		for (const auto& BagData : VariableBags)
		{
			if (!BagData.Bag->IsValidBag())
				continue;

			// Check item size compatibility - early exit before creating solver
			if (Item->ItemSize > BagData.Bag->GetMaxStoreSize())
				continue;

			// Check quiver compatibility if applicable - early exit before creating solver
			if (const IInventoryItemAmmoBagInterface* Quiver = Cast<IInventoryItemAmmoBagInterface>(BagData.Bag))
			{
				if (const IInventoryItemAmmoInterface* Ammo = Cast<IInventoryItemAmmoInterface>(Item))
				{
					if (Quiver->GetAmmoType() != Ammo->GetAmmoType())
						continue; // Skip if ammo type doesn't match
				}
				else
				{
					continue; // Skip if item is not ammo but bag is a quiver
				}
			}

			// Lazily get or create the temporary solver for this bag
			GridBagSolver* TempSolver = TempSolvers.Find(BagData.Slot);
			if (!TempSolver)
			{
				// First time accessing this bag - create solver with current state
				GridBagSolver NewSolver = BagData.Bag->GetSolver();
				TempSolvers.Add(BagData.Slot, NewSolver);
				TempSolver = TempSolvers.Find(BagData.Slot);
			}

			// Try to find a valid position in this bag
			int32 TopLeftID = TempSolver->GetFirstValidTopLeft(Item);
			if (TopLeftID != -1)
			{
				// Item can be placed here, record it in the temporary solver
				TempSolver->RecordData(Item, TopLeftID);
				bItemPlaced = true;
				break;
			}
		}

		// If this item couldn't be placed anywhere, we can't receive all items
		if (!bItemPlaced)
			return false;
	}

	// All items were successfully placed in the simulation
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Bags content are only relevant to the user
	DOREPLIFETIME_CONDITION(UInventoryComponent, VariableBags, COND_OwnerOnly);
}
