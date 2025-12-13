
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

	if (TopLeft < 0 || TopLeft >= Width * Height)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridBagSolver::RecordData - TopLeft %d out of bounds"), TopLeft);
		return;
	}

	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridBagSolver::RecordData - Item is null"));
		return;
	}

	const int SX = TopLeft % Width;
	const int SY = TopLeft / Width;

	for (int y = SY; y < SY + Item->Height; ++y)
	{
		for (int x = SX; x < SX + Item->Width; ++x)
		{
			// Validate bounds before writing
			if (x < 0 || x >= Width || y < 0 || y >= Height)
			{
				UE_LOG(LogTemp, Warning, TEXT("GridBagSolver::RecordData - Item extends beyond bag bounds"));
				return;
			}

			const int ID = x + y * Width;
			if (ID >= 0 && ID < Grid.Num())
			{
				Grid[ID] = Item;
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
                                 EItemSize InputMaxStoreSize, float InputWeightReduction)
{
	// CRITICAL FIX 1.6: Actually prevent reinitialization if bag contains items
	if (BagValidity && Items.Num() > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot re-initialize non-empty bag slot %d - has %d items"),
			static_cast<int32>(InputBagSlot), Items.Num());
		return false;
	}

	LocalBagSlot = InputBagSlot;
	Width = InputWidth;
	Height = InputHeight;
	MaxStoreSize = InputMaxStoreSize;
	WeightReductionRatio = InputWeightReduction;

	BagValidity = true;
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::InitializeQuiverData(EAmmoType AmmoType)
{
	if (AmmoType != EAmmoType::Unknown)
	{
		IsQuiver = true;
		AmmoTypeLimitation = AmmoType;
	}
	else
	{
		IsQuiver = false;
		AmmoTypeLimitation = EAmmoType::Unknown;
	}
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UBagStorage::GetBagConst() const
{
	return Items;
}

//----------------------------------------------------------------------------------------------------------------------

float UBagStorage::GetBagSlotUsage() const
{
	float Usage = static_cast<float>(BagSlotUsage) / (Width * Height);
	return Usage;
}

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

	for (const auto& Item : Items)
	{
		if (Item.ItemID == ItemID)
			return Item.TopLeftID;
	}

	return -1;
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

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem called without authority"));
		return;
	}

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

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
	if (!Item)
		return;

	//update the weight
	BagWeight -= Item->Weight;
	BagStorageDispatcher_Server.Broadcast();

	BagSlotUsage -= (Item->Width * Item->Height);
	float UsageRatio = static_cast<float>(BagSlotUsage) / (Width * Height);
	BagUsageStorageChanged.Broadcast(LocalBagSlot, UsageRatio);
}

//----------------------------------------------------------------------------------------------------------------------

void UBagStorage::AddItemAt_Implementation(int32 ItemID, int32 TopLeftIndex, float Durability)
{

	// Validate authority
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemAt called without authority"));
		return;
	}

	// Validate bag is initialized and valid
	if (!BagValidity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to add item to invalid bag slot %d"), LocalBagSlot);
		return;
	}

	// Validate item exists in game registry
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to add invalid item ID: %d"), ItemID);
		return;
	}

	// Validate item size doesn't exceed bag capacity
	if (Item->ItemSize > MaxStoreSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item size %d exceeds bag max size %d"),
			static_cast<int32>(Item->ItemSize), static_cast<int32>(MaxStoreSize));
		return;
	}

	// Validate position is within bag bounds
	if (TopLeftIndex < 0 || TopLeftIndex >= Width * Height)
	{
		UE_LOG(LogTemp, Warning, TEXT("TopLeftIndex %d out of bounds for bag %dx%d"),
			TopLeftIndex, Width, Height);
		return;
	}

	// Validate position is actually available (no overlap with existing items)
	GridBagSolver Solver = GetSolver();
	if (!Solver.IsRoomAvailable(Item, TopLeftIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("No room at position %d for item %d (size %dx%d)"),
			TopLeftIndex, ItemID, Item->Width, Item->Height);
		return;
	}

	// CRITICAL FIX 1.5: Validate durability range
	if (!FMath::IsFinite(Durability))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid durability value (NaN/Inf), defaulting to 100"));
		Durability = 100.0f;
	}
	Durability = FMath::Clamp(Durability, 0.0f, 100.0f);

	// All validations passed - safe to add item
	FMinimalItemStorage NewItem;
	NewItem.ItemID = ItemID;
	NewItem.TopLeftID = TopLeftIndex;
	NewItem.Durability = Durability;

	Items.Add(NewItem);


	//update the weight
	BagWeight += Item->Weight;
	BagStorageDispatcher_Server.Broadcast();
	BagSlotUsage += (Item->Width * Item->Height);
	float UsageRatio = static_cast<float>(BagSlotUsage) / (Width * Height);
	BagUsageStorageChanged.Broadcast(LocalBagSlot, UsageRatio);
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
	DOREPLIFETIME_CONDITION(UBagStorage, BagSlotUsage, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, WeightReductionRatio, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, IsQuiver, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UBagStorage, AmmoTypeLimitation, COND_OwnerOnly);
}
