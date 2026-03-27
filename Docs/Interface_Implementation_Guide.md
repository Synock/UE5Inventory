# Interface Implementation Guide

## Overview

The InventoryPlugin requires four mandatory interfaces and provides several optional ones. This guide covers each interface's responsibility and the methods you must implement.

---

## Mandatory Interfaces

### IInventoryGameInstanceInterface

**Header**: `Interfaces/InventoryGameInstanceInterface.h`
**Implement on**: Your `UGameInstance` subclass

Serves as the global item registry. The plugin uses this to look up item definitions by ID.

**Pure Virtual (must implement):**

| Method | Signature | Purpose |
|--------|-----------|---------|
| `FetchItemFromID` | `UInventoryItemBase* FetchItemFromID(int32 ID)` | Look up item by ID |
| `RegisterItem` | `void RegisterItem(UInventoryItemBase* NewItem)` | Add item to registry |

**Virtual with defaults (optional overrides):**

| Method | Purpose | Default |
|--------|---------|---------|
| `GetCopperCoinIconTexture` | Copper coin icon for UI | Returns `nullptr` |
| `GetSilverCoinIconTexture` | Silver coin icon for UI | Returns `nullptr` |
| `GetGoldCoinIconTexture` | Gold coin icon for UI | Returns `nullptr` |
| `GetPlatinumCoinIconTexture` | Platinum coin icon for UI | Returns `nullptr` |

See the Integration Guide's Step 1 for a complete implementation example.

---

### IInventoryGameModeInterface

**Header**: `Interfaces/InventoryGameModeInterface.h`
**Implement on**: Your `AGameModeBase` subclass

Handles server-side item spawning and forwards item lookups to the GameInstance.

**Pure Virtual (must implement):**

| Method | Signature | Purpose |
|--------|-----------|---------|
| `FetchItemFromID` | `UInventoryItemBase* FetchItemFromID(int32 ID)` | Delegates to GameInstance |
| `RegisterItem` | `void RegisterItem(UInventoryItemBase* NewItem)` | Delegates to GameInstance |

**Virtual with default implementations:**

| Method | Purpose |
|--------|---------|
| `SpawnItemFromActor(AActor*, uint32 ItemID, FVector, bool, float)` | Spawn `ADroppedItem` from an ItemID |
| `SpawnItemFromActorRaw(AActor*, UInventoryItemBase*, float)` | Spawn `ADroppedItem` from an item pointer |
| `SpawnCoinsFromActor(AActor*, FCoinValue, FVector, bool)` | Spawn `ADroppedCoins` |
| `GetItemSpawnLocation(AActor*, FVector, bool)` | Calculate ground-clamped spawn position |
| `CanSpawnItem(UInventoryItemBase*)` | Check if an item is allowed to spawn |
| `DelayedLoreItemValidation(UInventoryItemBase*, ULootPoolComponent*)` | Post-delay lore item check |

The default spawn implementations handle basic actor spawning. Override them if you need custom spawn logic (e.g., different actor classes, spawn effects, item lifetime management).

---

### IInventoryPlayerInterface

**Header**: `Interfaces/InventoryPlayerInterface.h`
**Implement on**: Your `APlayerController` subclass

Provides access to the player's inventory components and interaction state. This is a large interface — the plugin's internal systems (merchants, trading, looting, banking) cast your PlayerController to this interface to access components.

**Pure Virtual (must implement):**

| Method | Purpose |
|--------|---------|
| `GetInventoryComponent()` / `GetInventoryComponentConst()` | Access player's bag storage |
| `GetCoinComponent()` / `GetCoinComponentConst()` | Access player's currency |
| `GetInventoryOwningActor()` / `GetInventoryOwningActorConst()` | Get the pawn that visually owns the inventory |
| `GetTransactionBoolean()` / `SetTransactionBoolean(bool)` | Lock flag during active transactions |
| `GetMerchantActor()` / `GetMerchantActorConst()` / `SetMerchantActor(AActor*)` | Current merchant interaction target |
| `GetLootedActor()` / `GetLootedActorConst()` / `SetLootedActor(AActor*)` | Current loot interaction target |

The interface also contains many Server RPCs for inventory operations (move, equip, drop, trade, bank, merchant, repair, etc.) that have **default implementations**. You only need to override them if you want custom behavior.

See the Integration Guide's Step 3 for a complete implementation example.

---

### IEquipmentInterface

**Header**: `Interfaces/EquipmentInterface.h`
**Implement on**: Your `ACharacter` subclass

Manages equipment slot operations and visual mesh attachment on the character.

**Pure Virtual (must implement):**

| Method | Signature | Purpose |
|--------|-----------|---------|
| `GetEquipmentComponent` | `UEquipmentComponent* GetEquipmentComponent()` | Access equipment component |
| `GetEquipmentComponentConst` | `const UEquipmentComponent* GetEquipmentComponentConst() const` | Const access |

**Virtual with default implementations:**

| Method | Purpose |
|--------|---------|
| `GetAllEquipment()` | Get all equipped items |
| `GetEquippedItem(EEquipmentSlot)` | Get item in a specific slot |
| `GetEquipmentDurability(EEquipmentSlot, float&)` | Query slot durability |
| `EquipItem(EEquipmentSlot, int32 ItemId)` | Equip by item ID |
| `EquipItemWithDurability(EEquipmentSlot, int32, float)` | Equip with specific durability |
| `TryAutoEquip(int32 ItemId, EEquipmentSlot&)` | Auto-find slot for item |
| `UnequipItem(EEquipmentSlot)` | Remove from slot |

The default implementations handle item lookup via `UInventoryUtilities`, equipment component operations, and visual mesh updates. Override to add custom effects (sounds, animations, stat application).

---

## Optional Interfaces

### IInventoryModularCharacterInterface

**Header**: `Interfaces/InventoryModularCharacterInterface.h`
**Implement on**: Your `ACharacter` subclass (alongside `IEquipmentInterface`)

Provides the `UEquipmentComponent` with two things:
1. Named access to the character's individual skeletal mesh components (head, torso, arms, etc.) so the plugin can attach static slot-specific meshes.
2. A resolver for **dynamic overlay meshes** — skeletal mesh components managed at runtime in `VariableMeshesMap` inside `UEquipmentComponent`.

---

#### Body-Part Component Accessors

These return the `USkeletalMeshComponent*` that represents each modular body part. All have default implementations that return `nullptr`. Override only the ones your character actually exposes.

| Method | Slot it serves | Notes |
|--------|---------------|-------|
| `GetHeadComponent()` | Head (base mesh) | Used to attach the base head, not the helmet |
| `GetHelmetComponent()` | `EEquipmentSlot::Head` | Static slot-based helmet attachment (see note below) |
| `GetTorsoComponent()` | `EEquipmentSlot::Torso` | |
| `GetArmsComponent()` | `EEquipmentSlot::Arms` | |
| `GetHandsComponent()` | `EEquipmentSlot::Hands` | |
| `GetLegsComponent()` | `EEquipmentSlot::Legs` | |
| `GetFootComponent()` | `EEquipmentSlot::Feet` | |
| `GetShoulderPadComponent()` | `EEquipmentSlot::Shoulders` | |
| `GetNeckComponent()` | `EEquipmentSlot::Neck` | |
| `GetRightBracerComponent()` | `EEquipmentSlot::WristR` | |
| `GetLeftBracerComponent()` | `EEquipmentSlot::WristL` | |
| `GetBackComponent()` | `EEquipmentSlot::Back` | |

`GetEquipmentComponentFromSlot(EEquipmentSlot)` dispatches to the above methods automatically and is used by the plugin's default `SetEquipment` implementation.

---

#### Dynamic Overlay Mesh: `GetEquipmentOverlayMesh`

```cpp
virtual USkeletalMesh* GetEquipmentOverlayMesh(
    EEquipmentSlot Slot,
    const UInventoryItemEquipable* Item) const;
```

**This is the primary integration point for visual overlay equipment.**

When `EquipItem`, `EquipItemWithDurability`, `RemoveItem`, or `OnRep_ItemList` runs, `UEquipmentComponent` calls this method for the affected slot. If it returns a non-null mesh, the component immediately creates or updates a dedicated `USkeletalMeshComponent` for that slot inside its internal `VariableMeshesMap` — the same mechanism used for cloth physics pieces. The component is attached to the character mesh and given `SetLeaderPoseComponent` so it follows the skeleton automatically. Returning `nullptr` suppresses the overlay for that slot; the game is then responsible for handling the visual through its own path (e.g., merged skeletal mesh).

**Default implementation** (plugin fallback):
```cpp
// Returns Item->EquipmentMesh — raw asset, no race/gender correction.
// Works generically for any character that has not overridden this method.
```

**Typical game-side override:**
```cpp
USkeletalMesh* AYourCharacter::GetEquipmentOverlayMesh(
    EEquipmentSlot Slot, const UInventoryItemEquipable* Item) const
{
    if (!Item || !Item->EquipmentMesh)
        return nullptr;

    switch (Slot)
    {
    // Overlay slots: return race/gender-corrected mesh
    case EEquipmentSlot::Shoulders:
    case EEquipmentSlot::Neck:
    case EEquipmentSlot::Back:
    case EEquipmentSlot::Face:
    case EEquipmentSlot::WristR:
        return GameInstance->GetCorrectEquipmentMesh(
            Item->EquipmentMesh, RaceId, bIsFemale);

    case EEquipmentSlot::WristL:
    {
        USkeletalMesh* Corrected = GameInstance->GetCorrectEquipmentMesh(
            Item->EquipmentMesh, RaceId, bIsFemale);
        // Mirror via a separate pre-authored left-arm mesh asset.
        // Do NOT use negative scale: SetLeaderPoseComponent binds bones by name,
        // so a WristR mesh with negative scale would animate with the wrong arm
        // in any non-symmetrical animation.
        return GameInstance->GetMirrorEquipmentMesh(Corrected);
    }

    // Head is handled by the merged skeletal mesh path — suppress overlay.
    // Body-part replacement slots (Torso, Legs, Arms, Hands, Feet) are also merged.
    default:
        return nullptr;
    }
}
```

---

#### Overlay vs. Merged Mesh: When to Use Each

| Approach | How it works | Use for |
|----------|-------------|---------|
| **Overlay** (`GetEquipmentOverlayMesh` returns non-null) | Plugin creates a separate `USkeletalMeshComponent` with `SetLeaderPoseComponent`. Supports cloth physics. Attaches on equip, detaches on unequip without a full mesh rebuild. | Shoulders, neck, back, face, bracers — pieces worn *on top* of the body |
| **Merged** (`GetEquipmentOverlayMesh` returns `nullptr`) | Game gathers meshes for all equipped slots and calls `USkeletalMergingLibrary::MergeMeshes`. Full rebuild on any change. No cloth physics. | Head (helmet texture must match face mesh UVs), torso, legs, arms, hands, feet — pieces that *replace* a body segment |

The full rebuild (`UpdateMeshFromInternal`) still calls `TryUpdateDynamicMeshes` at the end, so `VariableMeshesMap` is always consistent with the current equipment state whether the update came from the plugin's per-equip path or the game's full rebuild path.

---

#### Minimal Implementation Example

```cpp
// Header
class YOURPROJECT_API AYourCharacter : public ACharacter,
    public IEquipmentInterface,
    public IInventoryModularCharacterInterface
{
    // ...
    virtual USkeletalMesh* GetEquipmentOverlayMesh(
        EEquipmentSlot Slot, const UInventoryItemEquipable* Item) const override;
};

// Source — only override GetEquipmentOverlayMesh if you need race/gender correction
// or want specific slots to stay in the merged path.
// If you do not override it, the default returns Item->EquipmentMesh for all slots.
```

---

### IMerchantInterface

**Header**: `Interfaces/MerchantInterface.h`
**Implement on**: Merchant NPC actors

Provides access to `UMerchantComponent` and `UCoinComponent` for buy/sell operations.

---

### IRepairInterface

**Header**: `Interfaces/RepairInterface.h`
**Implement on**: Repair NPC actors

Provides access to `URepairComponent` for equipment repair pricing and execution.

---

### ILootableInterface

**Header**: `Interfaces/LootableInterface.h`
**Implement on**: Lootable actors (corpses, chests)

Provides access to `ULootPoolComponent` and loot state management.

---

### ITradeInterface

**Header**: `Interfaces/TradeInterface.h`
**Implement on**: Your `APlayerController` (if trading is needed)

Provides access to `UTradeComponent` for player-to-player trading.

---

### IFieldRepairInterface

**Header**: `Interfaces/FieldRepairInterface.h`
**Implement on**: Your `APlayerController` (if field repair is needed)

Provides access to `UFieldRepairComponent` for player-performed equipment repair using repair kits.

---

### IPurseInterface

**Header**: `Interfaces/PurseInterface.h`
**Implement on**: Any actor that holds currency

Generic interface for coin access, used by merchants and other coin-holding actors.

---

### IInventoryHUDInterface

**Header**: `Interfaces/InventoryHUDInterface.h`
**Implement on**: Your HUD widget

Provides control over inventory UI elements (open/close windows, refresh displays).

---

### IInventoryGameStateInterface

**Header**: `Interfaces/InventoryGameStateInterface.h`
**Implement on**: Your `AGameStateBase` subclass (if needed)

Optional interface for game-state level inventory coordination.

---

### IInventoryInterface

**Header**: `Interfaces/InventoryInterface.h`

General-purpose inventory access interface, distinct from `IInventoryPlayerInterface`.
