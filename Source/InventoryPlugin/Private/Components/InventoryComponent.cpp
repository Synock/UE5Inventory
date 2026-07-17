#include "Components/InventoryComponent.h"
#include "InventoryPlugin.h"
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
		{
			Bag->InitializeData(BagSlotValue, 3, 2, EItemSize::Medium);
			Bag->SetBagValidity(true); // Pockets are always available — mark valid at construction
		}

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
			//UE_LOG(LogInventoryPlugin, Error, TEXT("Adding bag slot %d"), BagData.Slot);
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

namespace
{
	// Shared empty bag list returned when bag is not found — avoids a crash on null dereference.
	static const TArray<FMinimalItemStorage> GEmptyBagContent;
}

const TArray<FMinimalItemStorage>& UInventoryComponent::GetBagConst(EBagSlot WantedBagSlot) const
{
	const UBagStorage* Bag = GetRelatedBag(WantedBagSlot);
	if (!Bag)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("GetBagConst: bag for slot %d not found, returning empty"),
		       static_cast<int32>(WantedBagSlot));
		return GEmptyBagContent;
	}
	return Bag->GetBagConst();
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
		// Validate bag pointer before adding to LUT
		if (!BagData.Bag)
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("OnRep_ReplicatedBags: Null bag encountered for slot %d, skipping"),
			       static_cast<int32>(BagData.Slot));
			continue;
		}

		//UE_LOG(LogInventoryPlugin, Error, TEXT("Repping bag slot %d"), BagData.Slot);
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
	const UBagStorage* Bag = GetRelatedBag(ConsideredBag);
	if (!Bag)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("GetItemAtIndex: bag for slot %d not found"),
		       static_cast<int32>(ConsideredBag));
		return -1;
	}
	return Bag->GetItemAtIndex(ID);
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
	// SECURITY: Authority check to prevent client manipulation
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UpdateItemDurability called on client - ignoring (potential cheat attempt)"));
		return false;
	}

	UBagStorage* Bag = GetRelatedBag(BagSlot);
	if (!Bag)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UpdateItemDurability: Invalid bag slot %d"), static_cast<int32>(BagSlot));
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
	UBagStorage* Bag = GetRelatedBag(ConsideredBag);
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
	if (!Bag || !Item || !CanPlaceItemAt(ConsideredBag, Item, TopLeftIndex))
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("AddItemAt rejected invalid or occupied destination: bag=%d topLeft=%d item=%d"),
			static_cast<int32>(ConsideredBag), TopLeftIndex, ItemID);
		return;
	}
	Bag->AddItemAt(ItemID, TopLeftIndex, Durability);
	InventoryItemAdd.Broadcast(ConsideredBag, ItemID, TopLeftIndex, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::BagSet(EBagSlot ConsideredBag, bool InputValidity, int32 InputWidth, int32 InputHeight,
                                 EItemSize InputMaxStoreSize, float WeightReduction)
{

	UBagStorage* Bag = GetRelatedBag(ConsideredBag);
	if (!Bag)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("BagSet: Failed to get bag for slot %d"), static_cast<int32>(ConsideredBag));
		return;
	}

	Bag->InitializeData(ConsideredBag, InputWidth, InputHeight, InputMaxStoreSize, WeightReduction);
	Bag->SetBagValidity(InputValidity);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryComponent::QuiverSpecificSetup(EBagSlot ConsideredBag, EAmmoType NewAmmoType)
{
	// Validate this is actually a quiver slot
	if (ConsideredBag != EBagSlot::Quiver)
	{
		UE_LOG(LogInventoryPlugin, Warning,
		       TEXT("QuiverSpecificSetup called on non-quiver bag slot %d - this may cause unexpected behavior"),
		       static_cast<int32>(ConsideredBag));
	}

	UBagStorage* Bag = GetRelatedBag(ConsideredBag);
	if (!Bag)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("QuiverSpecificSetup: Failed to get bag for slot %d"), static_cast<int32>(ConsideredBag));
		return;
	}

	Bag->InitializeQuiverData(NewAmmoType);
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

bool UInventoryComponent::HasItem(int32 ItemID) const
{
	for (const auto& BagData : VariableBags)
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
		// CRITICAL FIX: Validate bag pointer before using
		if (!Bag.Bag)
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("FindSuitableSlot: Bag is null for slot %d"), static_cast<int32>(Bag.Slot));
			continue;
		}

		if (Bag.Bag->IsValidBag())
		{

			if (Item->ItemSize > Bag.Bag->GetMaxStoreSize())
				continue;

			GridBagSolver Solver = Bag.Bag->GetSolver();
			ApplyReservations(Bag.Slot, Solver);
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

bool UInventoryComponent::CanPlaceItemAt(EBagSlot BagSlot, const UInventoryItemBase* Item, int32 TopLeft) const
{
	const UBagStorage* Bag = GetRelatedBagConst(BagSlot);
	if (!Bag || !Bag->IsValidBag() || !Item || TopLeft < 0 || Item->ItemSize > Bag->GetMaxStoreSize())
		return false;
	GridBagSolver Solver = Bag->GetSolver();
	ApplyReservations(BagSlot, Solver);
	return Solver.IsRoomAvailable(Item, TopLeft);
}

bool UInventoryComponent::ReserveItemFootprint(const FGuid& ReservationId, EBagSlot BagSlot,
	const UInventoryItemBase* Item, int32 TopLeft)
{
	if (!ReservationId.IsValid() || ItemFootprintReservations.Contains(ReservationId) ||
		!CanPlaceItemAt(BagSlot, Item, TopLeft))
		return false;
	const UBagStorage* Bag = GetRelatedBagConst(BagSlot);
	if (!Bag)
		return false;
	FItemFootprintReservation Reservation;
	Reservation.BagSlot = BagSlot;
	const int32 StartX = TopLeft % Bag->GetWidth();
	const int32 StartY = TopLeft / Bag->GetWidth();
	for (int32 Y = StartY; Y < StartY + Item->Height; ++Y)
		for (int32 X = StartX; X < StartX + Item->Width; ++X)
			Reservation.Cells.Add(X + Y * Bag->GetWidth());
	ItemFootprintReservations.Add(ReservationId, MoveTemp(Reservation));
	return true;
}

void UInventoryComponent::ReleaseItemFootprint(const FGuid& ReservationId)
{
	ItemFootprintReservations.Remove(ReservationId);
}

bool UInventoryComponent::HasItemFootprintReservation(const FGuid& ReservationId) const
{
	return ItemFootprintReservations.Contains(ReservationId);
}

bool UInventoryComponent::HasReservationsInBag(EBagSlot BagSlot) const
{
	for (const TPair<FGuid, FItemFootprintReservation>& Pair : ItemFootprintReservations)
		if (Pair.Value.BagSlot == BagSlot)
			return true;
	return false;
}

bool UInventoryComponent::IsCellReserved(EBagSlot BagSlot, int32 Cell, const FGuid& IgnoredReservation) const
{
	for (const TPair<FGuid, FItemFootprintReservation>& Pair : ItemFootprintReservations)
		if (Pair.Key != IgnoredReservation && Pair.Value.BagSlot == BagSlot && Pair.Value.Cells.Contains(Cell))
			return true;
	return false;
}

float UInventoryComponent::GetEffectiveItemWeight(EBagSlot BagSlot, const UInventoryItemBase* Item) const
{
	const UBagStorage* Bag = GetRelatedBagConst(BagSlot);
	return Item && Bag ? Item->GetWeight() * Bag->GetWeightReductionRatio() : 0.0f;
}

void UInventoryComponent::ApplyReservations(EBagSlot BagSlot, GridBagSolver& Solver) const
{
	for (const TPair<FGuid, FItemFootprintReservation>& Pair : ItemFootprintReservations)
		if (Pair.Value.BagSlot == BagSlot)
			for (const int32 Cell : Pair.Value.Cells)
				Solver.RecordBlockedCell(Cell);
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
	if (!BagLUT.Contains(InputSlot))
		return false;

	const UBagStorage* Bag = BagLUT.FindRef(InputSlot);
	if (!Bag)
		return false;

	return Bag->IsValidBag();
}

//----------------------------------------------------------------------------------------------------------------------

UBagStorage* UInventoryComponent::GetRelatedBag(EBagSlot InputSlot) const
{
	if (BagLUT.Contains(InputSlot))
		return BagLUT.FindRef(InputSlot);

	// CRITICAL FIX: Don't crash in shipping builds - return nullptr and log error
	UE_LOG(LogInventoryPlugin, Error, TEXT("Cannot find related bag for slot %d - bag may not be initialized or was removed"),
	       static_cast<int32>(InputSlot));
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
	TArray<FInventoryDeliveryDestination> Placements;
	return FindPlacementsForItems(ItemArray, Placements);
}

bool UInventoryComponent::FindPlacementsForItems(const TArray<UInventoryItemBase*>& ItemArray,
	TArray<FInventoryDeliveryDestination>& OutPlacements) const
{
	OutPlacements.Reset();
	OutPlacements.Reserve(ItemArray.Num());

	// Lazily create solvers only when we need them for a specific bag
	TMap<EBagSlot, GridBagSolver> TempSolvers;

	// Try to place each item
	for (UInventoryItemBase* Item : ItemArray)
	{
		if (!Item)
			return false;

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
				ApplyReservations(BagData.Slot, NewSolver);
				TempSolvers.Add(BagData.Slot, NewSolver);
				TempSolver = TempSolvers.Find(BagData.Slot);
			}

			// Try to find a valid position in this bag
			int32 TopLeftID = TempSolver->GetFirstValidTopLeft(Item);
			if (TopLeftID != -1)
			{
				// Item can be placed here, record it in the temporary solver
				TempSolver->RecordData(Item, TopLeftID);
				OutPlacements.Add(FInventoryDeliveryDestination::MakeBag(BagData.Slot, TopLeftID));
				bItemPlaced = true;
				break;
			}
		}

		// If this item couldn't be placed anywhere, we can't receive all items
		if (!bItemPlaced)
		{
			OutPlacements.Reset();
			return false;
		}
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
