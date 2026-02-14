# Component Architecture Guide

## Overview

The InventoryPlugin uses a component-based architecture. Components are created in their owner's constructor with `CreateDefaultSubobject<>()`, marked for replication with `SetNetAddressable()` and `SetIsReplicated(true)`, and registered via `DOREPLIFETIME()`.

---

## Component Summary

| Component | Typical Owner | Purpose | Replicated |
|-----------|---------------|---------|------------|
| `UInventoryComponent` | PlayerController | Grid-based bag storage | Yes |
| `UEquipmentComponent` | Character | Worn equipment with visual meshes | Yes |
| `UCoinComponent` | Multiple | Currency storage (Copper/Silver/Gold/Platinum) | Yes |
| `UStagingAreaComponent` | PlayerController | Temporary storage for trades/merchants | Yes |
| `UBankComponent` | PlayerController | Persistent bank storage | Yes |
| `UMerchantComponent` | Merchant NPC | Merchant item pools + pricing | Yes |
| `URepairComponent` | Repairer NPC | Repair cost calculations | Yes |
| `UFieldRepairComponent` | PlayerController | Player-performed field repairs | Yes |
| `UTradeComponent` | PlayerController | Player-to-player trade state | Yes |
| `UKeyringComponent` | PlayerController | Key item management | Yes |
| `ULootPoolComponent` | Lootable actors | Loot table with randomized drops | Yes |
| `ULoreItemManagerComponent` | GameMode | Tracks unique/lore item spawns | Yes |

---

## Core Components

### UInventoryComponent

**Header**: `Components/InventoryComponent.h`

Manages grid-based bag storage with multiple bag slots. Internally, each bag is a `UBagStorage` subobject, tracked in a `TArray<FVariableBagStorage>` (replicated) and a `TMap<EBagSlot, UBagStorage*>` lookup table.

#### Bag Slots (`EBagSlot`)

| Slot | Value | Notes |
|------|-------|-------|
| `Pocket1` | 1 | Always available by default |
| `Pocket2` | 2 | Always available by default |
| `WaistBag1` | 3 | Activated when waist bag equipped |
| `WaistBag2` | 4 | Activated when waist bag equipped |
| `BackPack1` | 5 | Activated when backpack equipped |
| `BackPack2` | 6 | Activated when backpack equipped |
| `Quiver` | 7 | Ammo-type restricted |
| `LootPool` | 20 | Internal: loot windows |
| `StagingArea` | 21 | Internal: trade/merchant |
| `BankPool` | 22 | Internal: bank storage |

#### Key Methods

```cpp
// Initialize a bag slot
void BagSet(EBagSlot Slot, bool Active, int32 Width, int32 Height,
            EItemSize MaxStoreSize, float WeightReduction);

// Quiver-specific ammo type restriction
void QuiverSpecificSetup(EBagSlot Slot, EAmmoType AmmoType);

// Item operations (Server RPCs)
void AddItemAt(EBagSlot Bag, int32 ItemID, int32 TopLeftIndex, float Durability = 100.0f);
void RemoveItem(EBagSlot Bag, int32 TopLeftIndex);

// Queries
int32 GetItemAtIndex(EBagSlot Bag, int32 TopLeftIndex) const;
const TArray<FMinimalItemStorage>& GetBagConst(EBagSlot Slot) const;
EBagSlot FindSuitableSlot(const UInventoryItemBase* Item, int32& OutputTopLeftID) const;
bool HasItem(int32 ItemID);
bool HasItems(int32 ItemID, int32 Amount);
float GetTotalWeight() const;

// Item durability update
bool UpdateItemDurability(EBagSlot Slot, int32 TopLeftIndex, int32 ItemID, float NewDurability);

// Lock state
void SetItemLockState(EBagSlot Slot, int32 TopLeftIndex, bool bLocked);

// Slot conversion helpers (static)
static EEquipmentSlot GetInventorySlotFromBagSlot(EBagSlot Bag);
static EBagSlot GetBagSlotFromInventory(EEquipmentSlot Slot);
```

#### Delegates

| Delegate | Parameters | Fires When |
|----------|------------|------------|
| `FullInventoryDispatcher` | None | Full inventory replication received (client) |
| `FullInventoryDispatcher_Server` | None | Inventory changed (server) |
| `InventoryItemAdd` | `EBagSlot, int32 ItemID, int32 TopLeftID, float Durability` | Item added |
| `InventoryItemRemove` | `EBagSlot, int32 TopLeftID` | Item removed |
| `InventoryItemDurabilityUpdate` | `EBagSlot, int32 ItemID, int32 TopLeftID, float NewDurability` | Durability changed |
| `InventoryBagUsageChanged` | `EBagSlot, float BagUsage` | Bag fill ratio changed |

#### Item Storage Struct (`FMinimalItemStorage`)

```cpp
struct FMinimalItemStorage
{
    int32 ItemID = -1;        // Item definition ID
    int32 TopLeftID = 0;      // Grid position index
    float Durability = 100.0f; // Current durability
    bool bIsLocked = false;    // UI lock state (transient)
};
```

Grid indexing: `TopLeftID = Row * BagWidth + Column`

---

### UEquipmentComponent

**Header**: `Components/EquipmentComponent.h`

Manages worn equipment with visual skeletal mesh attachment, weapon sheath/unsheath, durability tracking, and accessory meshes.

#### Equipment Slots (`EEquipmentSlot`)

| Slot | Value | Category |
|------|-------|----------|
| `Primary` | 1 | Weapon |
| `Secondary` | 2 | Weapon / Shield |
| `Range` | 3 | Ranged weapon |
| `Ammo` | 4 | Ammunition bag |
| `Head` | 5 | Armor |
| `Face` | 6 | Armor |
| `EarL` / `EarR` | 7 / 8 | Accessory |
| `Neck` | 9 | Accessory |
| `Shoulders` | 10 | Armor |
| `Back` | 11 | Armor |
| `Torso` | 12 | Armor |
| `WristL` / `WristR` | 13 / 14 | Accessory |
| `Hands` | 15 | Armor |
| `FingerL` / `FingerR` | 16 / 17 | Accessory |
| `Waist` | 18 | Armor |
| `Legs` | 19 | Armor |
| `Foot` | 20 | Armor |
| `Arms` | 21 | Armor |
| `WaistBag1` / `WaistBag2` | 22 / 23 | Bag |
| `BackPack1` / `BackPack2` | 24 / 25 | Bag |

#### Key Methods

```cpp
// Equipment operations
void EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot Slot);
void EquipItemWithDurability(const UInventoryItemEquipable* Item, EEquipmentSlot Slot, float Durability);
bool RemoveItem(EEquipmentSlot Slot);
void RemoveAll();

// Queries
const UInventoryItemEquipable* GetItemAtSlot(EEquipmentSlot Slot) const;
const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const;
bool IsSlotEmpty(EEquipmentSlot Slot);
EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const;
float GetTotalWeight() const;

// Durability
bool GetEquipmentDurability(EEquipmentSlot Slot, float& OutDurability) const;
void SetEquipmentDurability(EEquipmentSlot Slot, float Durability);
void ReduceEquipmentDurability(EEquipmentSlot Slot, float Reduction);

// Lock state (client-side UI only)
void SetEquipmentLockState(EEquipmentSlot Slot, bool bLocked);
bool GetEquipmentLockState(EEquipmentSlot Slot) const;

// Visual mesh management
void UpdateMasterMeshComponent(USkeletalMeshComponent* Mesh);
void UnsheathMelee();
void SheathMelee();
void UnsheathRanged();
void SheathRanged();
```

#### Delegates

| Delegate | Parameters | Fires When |
|----------|------------|------------|
| `EquipmentDispatcher` | None | Equipment fully replicated (client) |
| `EquipmentDispatcher_Server` | None | Equipment changed (server) |
| `ItemEquipedDispatcher_Server` | `EEquipmentSlot, const UInventoryItemEquipable*` | Item equipped (server) |
| `ItemUnEquipedDispatcher_Server` | `EEquipmentSlot, const UInventoryItemEquipable*` | Item unequipped (server) |
| `EquipmentDurabilityChangedDispatcher_Server` | `EEquipmentSlot, float` | Durability changed (server) |
| `DurabilityWarningDispatcher` | `EEquipmentSlot, float, const UInventoryItemEquipable*` | Low durability warning |

---

### UCoinComponent

**Header**: `Components/CoinComponent.h`

Manages four-tier currency storage. Conversion ratio: **10x per tier** (10 Copper = 1 Silver, 10 Silver = 1 Gold, 10 Gold = 1 Platinum).

#### Currency Struct (`FCoinValue`)

```cpp
struct FCoinValue
{
    int32 CopperPieces = 0;
    int32 SilverPieces = 0;
    int32 GoldPieces = 0;
    int32 PlatinumPieces = 0;

    void ReduceCoinAmount();                // Convert overflow to higher tiers
    float ToFloat() const;                   // Total value as float (in copper)
    bool IsEmpty() const;

    static bool CanPay(const FCoinValue& Available, const FCoinValue& Needed);
    static bool CanPayWithChange(const FCoinValue& Available, const FCoinValue& Needed);
    static bool RetrieveValue(FCoinValue& Available, FCoinValue& Needed);
};
```

#### Key Methods

```cpp
const FCoinValue& GetCoinValue() const;
void EditCoinContent(int32 CP, int32 SP, int32 GP, int32 PP);  // Add amounts
void AddCoins(const FCoinValue& Value);
void RemoveCoins(const FCoinValue& Value);
void PayAndAdjust(const FCoinValue& Cost);       // Pay with automatic change-making
void PayAndAdjustSimple(const FCoinValue& Cost);  // Simple subtraction
```

#### Delegates

| Delegate | Fires When |
|----------|------------|
| `PurseDispatcher` | Currency replicated (client) |
| `PurseDispatcher_Server` | Currency changed (server) |

---

## Extended Components

### UStagingAreaComponent

Temporary item storage used during trades, merchant transactions, and mail. Content can be committed or rolled back atomically.

### UBankComponent

Persistent storage for long-term item keeping. Functionally similar to `UInventoryComponent` but intended for bank NPC / bank zone access.

### UMerchantComponent

Manages a merchant's item pool (static items that always appear + dynamic items that restock). Works with `UCoinComponent` for the merchant's cash reserves.

### URepairComponent

Calculates repair costs for equipment based on durability loss and item value. Used by repair NPC actors implementing `IRepairInterface`.

### UFieldRepairComponent

Enables players to perform limited equipment repairs using `UInventoryItemFieldRepair` kit items. Tracks active repairs with `FActiveFieldRepair` state.

### UTradeComponent

Manages player-to-player trade state: offered items, offered coins, acceptance status, and trade execution.

### UKeyringComponent

Manages key items (`UInventoryItemKey`) using `FKeyItemPair` entries. Used for door/lock systems.

### ULootPoolComponent

Manages loot tables for lootable actors. Works with `URandomizedLootPool` data assets for randomized drops using `FItemProbability` and `FItemAlternative`.

### ULoreItemManagerComponent

Tracks which unique/lore items have been spawned to prevent duplicates. Typically lives on the GameMode.

---

## Component Setup Pattern

All components follow the same creation pattern:

```cpp
// In owner's constructor:
MyComponent = CreateDefaultSubobject<UComponentType>(TEXT("Name"));
MyComponent->SetNetAddressable();
MyComponent->SetIsReplicated(true);

// In GetLifetimeReplicatedProps:
DOREPLIFETIME(AOwnerClass, MyComponent);
```

Replication callbacks follow the `ReplicatedUsing` pattern — the component's internal replicated properties trigger `OnRep_*` functions which broadcast delegates for UI.
