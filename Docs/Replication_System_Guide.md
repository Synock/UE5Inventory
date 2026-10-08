# Replication System Guide

## Overview

The InventoryPlugin uses Unreal Engine's standard property replication with a **server-authoritative** model. The server owns all inventory/equipment/coin state. Clients receive updates via `ReplicatedUsing` callbacks that broadcast delegates for UI binding.

Key design principles:
- **Server RPCs** for state-changing operations (add item, remove item)
- **`ReplicatedUsing`** for state synchronization to clients
- **`OnRep_*` callbacks** that broadcast delegates for UI updates
- **`COND_OwnerOnly`** for private data (bag contents, trade state)
- **No condition** for publicly visible data (equipment appearance, merchant pools)

---

## Replication Flow

```
Client Request → Server RPC → Server Modifies State → Property Replicates → OnRep_ Fires → Delegate Broadcasts → UI Updates
```

### Example: Adding an Item

1. Client calls `UInventoryComponent::AddItemAt(EBagSlot, ItemID, TopLeftID, Durability)` — a `Server, reliable` RPC
2. Server validates and adds `FMinimalItemStorage` to the `UBagStorage::Items` array
3. `Items` (marked `ReplicatedUsing = OnRep_BagData`) replicates to the owning client
4. `UBagStorage::OnRep_BagData()` fires on the client
5. `BagDispatcher` delegate broadcasts — UI rebuilds bag display

---

## Replicated Properties by Component

### UBagStorage

All properties use `DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)` — only the owning player sees bag contents.

| Property | Type | Replication | OnRep |
|----------|------|-------------|-------|
| `Items` | `TArray<FMinimalItemStorage>` | `ReplicatedUsing = OnRep_BagData` | `OnRep_BagData()` → `BagDispatcher` |
| `Width` | `int` | `Replicated` | — |
| `Height` | `int` | `Replicated` | — |
| `MaxStoreSize` | `EItemSize` | `Replicated` | — |
| `LocalBagSlot` | `EBagSlot` | `Replicated` | — |
| `BagValidity` | `bool` | `Replicated` | — |
| `BagWeight` | `float` | `Replicated` | — |
| `BagSlotUsage` | `int32` | `Replicated` | — |
| `WeightReductionRatio` | `float` | `Replicated` | — |
| `IsQuiver` | `bool` | `Replicated` | — |
| `AmmoTypeLimitation` | `EAmmoType` | `Replicated` | — |

**Server RPCs:**
```cpp
UFUNCTION(Server, reliable)
virtual void AddItemAt(int32 ItemID, int32 TopLeftIndex, float Durability = 100.0f);

UFUNCTION(Server, reliable)
void RemoveItem(int32 TopLeftIndex);
```

### UInventoryComponent

Wraps multiple `UBagStorage` subobjects. The `VariableBags` array maps `EBagSlot` → `UBagStorage*`.

| Property | Type | Replication | OnRep |
|----------|------|-------------|-------|
| `VariableBags` | `TArray<FVariableBagStorage>` | `COND_OwnerOnly`, `ReplicatedUsing` | `OnRep_ReplicatedBags()` |

`OnRep_ReplicatedBags()` rebuilds the internal `TMap<EBagSlot, UBagStorage*>` lookup table (`BagLUT`).

**Server RPCs:**
```cpp
UFUNCTION(Server, reliable)
void AddItemAt(EBagSlot Bag, int32 ItemID, int32 TopLeftIndex, float Durability = 100.0f);

UFUNCTION(Server, reliable)
void RemoveItem(EBagSlot Bag, int32 TopLeftIndex);
```

### UEquipmentComponent

Equipment is visible to all players (no `COND_OwnerOnly`) — everyone sees worn gear.

| Property | Type | Replication | OnRep |
|----------|------|-------------|-------|
| `Equipment` | `TArray<const UInventoryItemEquipable*>` | `ReplicatedUsing = OnRep_ItemList` | → `EquipmentDispatcher` |
| `EquipmentDurability` | `TArray<float>` | `ReplicatedUsing = OnRep_EquipmentDurability` | → `EquipmentDispatcher` |
| `IsHoldingATwoHandedWeapon` | `bool` | `Replicated` | — |
| `PrimaryWeaponComponent` | `USkeletalMeshComponent*` | `Replicated` | — |
| `SecondaryWeaponComponent` | `USkeletalMeshComponent*` | `Replicated` | — |
| `PrimaryWeaponSheath` | `USkeletalMeshComponent*` | `Replicated` | — |
| `SecondaryWeaponSheath` | `USkeletalMeshComponent*` | `Replicated` | — |
| `BackWeaponSheath` | `USkeletalMeshComponent*` | `Replicated` | — |
| `RangedWeaponSheath` | `USkeletalMeshComponent*` | `Replicated` | — |

**NetMulticast RPCs** (visual updates visible to all):
```cpp
UFUNCTION(NetMulticast, reliable)
void UpdateEquipment(USkeletalMeshComponent* Socket, USkeletalMesh* Mesh,
                     const TArray<FMaterialOverride>& MaterialOverride);
```

Game-specific presentation such as equipped lights, particles, and sounds is reconstructed by the owning project from
the replicated `Equipment` array. InventoryPlugin owns generic equipment state and mesh/socket presentation only.

### UCoinComponent

| Property | Type | Replication | OnRep |
|----------|------|-------------|-------|
| `PurseContent` | `FCoinValue` | `ReplicatedUsing = OnRep_PurseData` | → `PurseDispatcher` |

No replication condition — uses default `DOREPLIFETIME`.

### UTradeComponent

All trade data uses `COND_OwnerOnly`.

| Property | Replication | OnRep |
|----------|-------------|-------|
| `TradePartner` | `COND_OwnerOnly, ReplicatedUsing` | `OnRep_TradePartner()` |
| `OurOffer` | `COND_OwnerOnly, ReplicatedUsing` | `OnRep_OurOffer()` |
| `TheirOffer` | `COND_OwnerOnly, ReplicatedUsing` | `OnRep_TheirOffer()` |
| `bIsTrading` | `COND_OwnerOnly, ReplicatedUsing` | `OnRep_IsTrading()` |
| `OurCoinOffer` | `COND_OwnerOnly, Replicated` | — |

### Other Components

| Component | Key Replicated Property | Condition | OnRep |
|-----------|------------------------|-----------|-------|
| `UStagingAreaComponent` | `StagingAreaItems` | Default | `OnRep_StagingAreaItems()` |
| `UMerchantComponent` | `StaticMerchantPool` | Default | — |
| `UMerchantComponent` | `DynamicMerchantPool` | Default | `OnRep_DynamicPool()` |
| `ULootPoolComponent` | `Items` | Default | `OnRep_LootPool()` |
| `UKeyringComponent` | `KeyringData` | Default | `OnRep_KeyringChanged()` |
| `UBankComponent` | `Items` | Default | `OnRep_BankPool()` |
| `UFieldRepairComponent` | `ActiveFieldRepair` | `COND_OwnerOnly` | `OnRep_ActiveFieldRepair()` |

---

## Delegate Pattern

Each component exposes paired delegates:

| Delegate Suffix | Fires On | Purpose |
|-----------------|----------|---------|
| `*Dispatcher` | Client (inside OnRep) | UI update trigger |
| `*Dispatcher_Server` | Server (after state change) | Server-side game logic |

**Example — Inventory:**
- `FullInventoryDispatcher` — client: full bag data received
- `FullInventoryDispatcher_Server` — server: inventory was modified
- `InventoryItemAdd` — client: specific item was added (includes ItemID, TopLeftID, Durability)
- `InventoryItemRemove` — client: specific item was removed

**Example — Equipment:**
- `EquipmentDispatcher` — client: equipment list replicated
- `EquipmentDispatcher_Server` — server: equipment changed
- `ItemEquipedDispatcher_Server` — server: specific item equipped (slot + item pointer)
- `ItemUnEquipedDispatcher_Server` — server: specific item unequipped

**Example — Coins:**
- `PurseDispatcher` — client: purse replicated
- `PurseDispatcher_Server` — server: purse modified

---

## UI Binding

Bind to client-side delegates in your widget initialization:

```cpp
// In your inventory widget
void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UInventoryComponent* Inv = GetOwningPlayerController()->FindComponentByClass<UInventoryComponent>())
    {
        Inv->FullInventoryDispatcher.AddDynamic(this, &UInventoryWidget::RebuildInventory);
        Inv->InventoryItemAdd.AddDynamic(this, &UInventoryWidget::OnItemAdded);
        Inv->InventoryItemRemove.AddDynamic(this, &UInventoryWidget::OnItemRemoved);
    }
}
```

---

## Replication Conditions Summary

| Condition | Used For | Visibility |
|-----------|----------|------------|
| `COND_OwnerOnly` | Bag contents, trade state, field repair | Only owning player |
| Default (no condition) | Equipment, merchant pools, loot pools, coins | All clients |

**Design rationale**: Inventory contents are private — other players shouldn't see what's in your bags. Equipment appearance is public — everyone sees your worn gear. Merchant/loot pools are public — all nearby players can browse.
