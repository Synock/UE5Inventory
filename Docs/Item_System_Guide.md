# Item System Guide

## Overview

Items in the InventoryPlugin are **`UPrimaryDataAsset`** subclasses — editor-authored data definitions, not spawned actors. Items are registered in a **`DataTable`** using the `FItemContainerLine` row struct, then loaded into a lookup table (typically in your `GameInstance`).

At runtime, inventory slots store only an `FMinimalItemStorage` (ItemID, grid position, durability, lock state). The full item definition is resolved from the lookup table when needed.

---

## Class Hierarchy

```
UInventoryItemBase (UPrimaryDataAsset, IInventoryItemInterface)
├── UInventoryItemEquipable (+IInventoryItemEquipableInterface, +IInventoryItemWeaponInterface, +IInventoryItemDurableInterface)
│   ├── UInventoryItemWeapon (marker subclass — no additional properties)
│   ├── UInventoryItemBag (+IInventoryItemBagInterface)
│   │   └── UInventoryItemAmmoBag (+IInventoryItemAmmoBagInterface)
│   ├── UInventoryItemActionnable (activatable/consumable items)
│   └── UInventoryItemFieldRepair (+IInventoryItemFieldRepairInterface)
└── UInventoryItemKey (key items with KeyID)
```

---

## UInventoryItemBase

Base class for all items. Implements `IInventoryItemInterface`.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `ItemID` | `int32` | -1 | Unique identifier |
| `Name` | `FString` | | Display name |
| `Width` | `uint8` | 1 | Grid width |
| `Height` | `uint8` | 1 | Grid height |
| `Icon` | `UTexture2D*` | nullptr | Inventory icon |
| `Mesh` | `UStaticMesh*` | nullptr | World/dropped mesh |
| `OverrideMaterial` | `FMaterialOverride` | | Material override spec |
| `ItemSize` | `EItemSize` | Tiny | Size category (Tiny/Small/Medium/Large/Giant) |
| `Description` | `FString` | | Item description |
| `LoreItem` | `bool` | false | Unique lore item (tracked by `ULoreItemManagerComponent`) |
| `MagicItem` | `bool` | false | Magical item flag |
| `Temporary` | `bool` | false | Auto-removes on logout/death |
| `BaseValue` | `float` | 0 | Base monetary value |
| `Weight` | `float` | 0 | Item weight |

### FMaterialOverride

```cpp
struct FMaterialOverride
{
    UMaterialInstance* OverrideMaterial = nullptr;
    int32 MaterialID = 0;
    FLinearColor TintColor = FLinearColor::White;
    float TintIntensity = 1.0f;
};
```

### EItemSize

`Tiny`, `Small`, `Medium`, `Large`, `Giant` — used for bag size restrictions. A bag with `BagSize = Medium` can store items of size Medium or smaller.

---

## UInventoryItemEquipable

Extends `UInventoryItemBase` with equipment slot targeting, visual meshes, and durability.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `Equipable` | `bool` | false | Whether this item can be equipped |
| `EquipableSlotBitMask` | `int32` (bitmask) | 0 | Bitfield of valid `EEquipmentSlot` values |
| `MultiSlotItem` | `bool` | false | If true, occupies all slots in the bitmask simultaneously |
| `Shield` | `bool` | false | Shield flag |
| `Weapon` | `bool` | false | Weapon flag |
| `EquipmentMesh` | `USkeletalMesh*` | nullptr | Skeletal mesh rendered on the character |
| `EquipmentMeshMaterialOverride` | `TArray<FMaterialOverride>` | | Per-slot material overrides for equipment mesh |
| `Unsheathable` | `bool` | false | Can be drawn/sheathed visually |
| `TotalDurability` | `float` | 100.0 | Maximum durability |
| `DurabilityModifier` | `float` | 1.0 | Multiplier for durability loss calculations |

### Interfaces Implemented

- **`IInventoryItemEquipableInterface`**: `IsEquipable()`, `GetEquipableSlotBitMask()`, `IsMultiSlotItem()`, `IsShield()`, `IsWeapon()`, `GetEquipmentMesh()`, `GetEquipmentMeshMaterialOverride()`, `IsUnsheathable()`
- **`IInventoryItemWeaponInterface`**: Weapon-related queries
- **`IInventoryItemDurableInterface`**: `GetTotalDurability()`, `GetDurabilityModifier()`

---

## UInventoryItemWeapon

Empty marker subclass of `UInventoryItemEquipable`. No additional properties. Use as a type filter or extend in your project for weapon-specific data.

---

## UInventoryItemBag

Bag items that, when equipped in bag slots, activate corresponding `EBagSlot` storage.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `Bag` | `bool` | false | Whether this works as a bag |
| `BagSize` | `EItemSize` | Giant | Maximum item size this bag can hold |
| `BagWidth` | `uint8` | 1 | Grid width of bag interior |
| `BagHeight` | `uint8` | 1 | Grid height of bag interior |
| `WeightReduction` | `float` | 0.0 | Weight reduction factor (0.0 = none, 1.0 = weightless) |

Implements `IInventoryItemBagInterface`.

### UInventoryItemAmmoBag

Extends `UInventoryItemBag` with ammo-type restriction and fill-level visual meshes.

| Property | Type | Description |
|----------|------|-------------|
| `AmmoType` | `EAmmoType` | Accepted ammo type (Throwable/SmallBolts/Bolts/GreatBolts/Arrows) |
| `SingleAmmoMesh` | `UStaticMesh*` | Mesh when bag has few items |
| `MidAmmoMesh` | `UStaticMesh*` | Mesh when bag is partially full |
| `FullAmmoMesh` | `UStaticMesh*` | Mesh when bag is full |

Implements `IInventoryItemAmmoBagInterface`.

---

## UInventoryItemActionnable

Activatable items (food, drinks, books, etc.).

| Property | Type | Description |
|----------|------|-------------|
| `Actionnable` | `bool` | Whether this item can be activated |
| `NeedToBeEquipped` | `bool` | Must be equipped before use |
| `HungerValue` | `float` | Hunger restoration (game-defined) |
| `ThirstValue` | `float` | Thirst restoration (game-defined) |
| `BookText` | `FString` | Readable text content |

---

## UInventoryItemFieldRepair

Consumable repair kits used via `UFieldRepairComponent`. Uses durability as charge count.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `ChargeConsumption` | `int32` | 1 | Charges used per repair |
| `MinRepairPercentage` | `float` | 0.05 | Minimum repair % per use |
| `MaxRepairPercentage` | `float` | 0.20 | Maximum repair % per use |
| `MinDurabilityThreshold` | `float` | 0.33 | Target must be at/above this % |
| `MaxDurabilityThreshold` | `float` | 0.75 | Repairs cannot exceed this % |
| `RepairDuration` | `float` | 3.0 | Seconds to complete (0 = instant) |
| `WeaponsOnly` | `bool` | false | Restrict to weapons only |
| `ArmorOnly` | `bool` | false | Restrict to armor only |
| `ShieldsOnly` | `bool` | false | Restrict to shields only |
| `AllowedEquipmentSlotBitMask` | `int32` | 0 | Restrict to specific slots (0 = all) |
| `RepairStartSound` | `USoundBase*` | nullptr | Audio feedback |
| `RepairCompleteSound` | `USoundBase*` | nullptr | Audio feedback |
| `RepairFailSound` | `USoundBase*` | nullptr | Audio feedback |
| `RepairParticleEffect` | `UParticleSystem*` | nullptr | Visual feedback |

Implements `IInventoryItemFieldRepairInterface`.

---

## UInventoryItemKey

Key items for lock/unlock systems via `UKeyringComponent`.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `KeyID` | `int32` | -1 | Unique key identifier for matching locks |

---

## Runtime Item Storage

### FMinimalItemStorage

The actual stored representation of an item in a bag slot:

```cpp
struct FMinimalItemStorage
{
    int32 ItemID = -1;         // References item definition via lookup table
    int32 TopLeftID = 0;       // Grid position: Row * BagWidth + Column
    float Durability = 100.0f; // Current durability percentage
    bool bIsLocked = false;    // Transient: UI lock (not replicated)
};
```

### FItemContainerLine

DataTable row structure for item registration:

```cpp
USTRUCT(BlueprintType)
struct FItemContainerLine : public FTableRowBase
{
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
    UInventoryItemBase* Item = nullptr;
};
```

---

## Item Registration

1. **Create item assets**: Right-click in Content Browser → Miscellaneous → Data Asset → select the appropriate item class
2. **Create a DataTable**: Row Structure = `FItemContainerLine`
3. **Add rows**: Each row references one item asset
4. **Load in GameInstance**: Populate a `TMap<int32, UInventoryItemBase*>` from the DataTable

```cpp
// In your GameInstance (implementing IInventoryGameInstanceInterface)
void UMyGameInstance::LoadItems()
{
    for (auto& Row : ItemDataTable->GetRowMap())
    {
        FItemContainerLine* ItemRow = reinterpret_cast<FItemContainerLine*>(Row.Value);
        if (ItemRow && ItemRow->Item)
            ItemLUT.Add(ItemRow->Item->ItemID, ItemRow->Item);
    }
}

// Interface implementation
const UInventoryItemBase* UMyGameInstance::GetItemFromID(int32 ItemID) const
{
    if (const auto* Found = ItemLUT.Find(ItemID))
        return *Found;
    return nullptr;
}
```

---

## Item Interfaces

Each item class implements one or more interfaces. These allow the plugin's components to query item properties polymorphically without casting.

| Interface | Implemented By | Purpose |
|-----------|----------------|---------|
| `IInventoryItemInterface` | `UInventoryItemBase` | Core item data (ID, name, size, weight, value) |
| `IInventoryItemEquipableInterface` | `UInventoryItemEquipable` | Equipment slot targeting, mesh, sheath |
| `IInventoryItemWeaponInterface` | `UInventoryItemEquipable` | Weapon queries |
| `IInventoryItemDurableInterface` | `UInventoryItemEquipable` | Durability values |
| `IInventoryItemBagInterface` | `UInventoryItemBag` | Bag dimensions, size restriction, weight reduction |
| `IInventoryItemAmmoBagInterface` | `UInventoryItemAmmoBag` | Ammo type, fill-level meshes |
| `IInventoryItemFieldRepairInterface` | `UInventoryItemFieldRepair` | Repair logic, thresholds, charges |
