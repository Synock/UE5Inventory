# Inventory Plugin - Integration Guide

## Additional Documentation

- **[Interface Implementation Guide](./Interface_Implementation_Guide.md)** - Detailed reference for all plugin interfaces
- **[Component Architecture Guide](./Component_Architecture_Guide.md)** - Complete reference for all inventory components
- **[Item System Guide](./Item_System_Guide.md)** - Creating and configuring items
- **[Replication System Guide](./Replication_System_Guide.md)** - Multiplayer synchronization and networking

## Table of Contents

1. [Migration from Previous Version](#migration-from-previous-version)
2. [Overview](#overview)
3. [Installation](#installation)
4. [Architecture](#architecture)
5. [Step-by-Step Integration](#step-by-step-integration)
   - [Step 1: GameInstance](#step-1-gameinstance---item-registry)
   - [Step 2: GameMode](#step-2-gamemode---item-spawning)
   - [Step 3: PlayerController](#step-3-playercontroller---inventory-components-and-net-component)
   - [Step 4: Character](#step-4-character---equipment)
   - [Step 5: HUD — Window Registry](#step-5-hud--window-registry)
6. [Inventory Initialization](#inventory-initialization)
7. [Item Creation](#item-creation)
8. [UI Integration](#ui-integration)
9. [Multiplayer](#multiplayer)
10. [Customizing Server Behavior](#customizing-server-behavior)
11. [Troubleshooting](#troubleshooting)

---

## Migration from Previous Version

This section covers breaking changes when upgrading from the previous `master` branch. If you are integrating the plugin for the first time, skip to [Overview](#overview).

### 1. Server RPCs Moved to `UInventoryNetComponent` (**Major**)

Previously, `IInventoryPlayerInterface` declared all `Server_*` methods as **pure virtual** functions (with `//UFUNCTION(Server, Reliable)` comments). You had to redeclare every one as a `UFUNCTION(Server, Reliable, WithValidation)` in your PlayerController and write both `_Implementation` and `_Validate` stubs.

Now, a new `UInventoryNetComponent` ActorComponent owns all Server RPCs. The `Server_*` methods on the interface are still present as **non-pure virtuals** that forward to the net component. Selected mutation RPCs use `WithValidation`; session-close and several workflow RPCs are reliable Server RPCs without a validation function. Review the declarations in `InventoryNetComponent.h` before treating the defaults as a complete game-specific anti-cheat policy.

**What to do:**

1. **Add the component** in your PlayerController constructor:
   ```cpp
   InventoryNetComponent = CreateDefaultSubobject<UInventoryNetComponent>(TEXT("InventoryNetComponent"));
   // No SetNetAddressable/SetIsReplicated needed — the component handles it internally.
   ```

2. **Implement the new pure virtual** `GetInventoryNetComponent()`:
   ```cpp
   UInventoryNetComponent* AYourPlayerController::GetInventoryNetComponent()
   {
       return InventoryNetComponent;
   }
   ```

3. **Remove all `Server_*` UFUNCTION declarations and implementations** from your PlayerController:
   - `Server_PlayerMoveItem`, `Server_PlayerUnequipItem`, `Server_PlayerEquipItemFromInventory`, `Server_PlayerSwapEquipment`, `Server_PlayerAutoEquipItem`, `Server_TransferCoinTo`, `Server_DropItemFromInventory`, `Server_DropItemFromEquipment`
   - `Server_LootActor`, `Server_StopLooting`, `Server_PlayerLootItem`, `Server_PlayerEquipItemFromLoot`, `Server_PlayerAutoLootAll`
   - `Server_MerchantTrade`, `Server_StopMerchantTrade`, `Server_PlayerBuyFromMerchant`, `Server_PlayerSellToMerchant`
   - `Server_CancelStagingArea`, `Server_TransferStagingToActor`, `Server_MoveEquipmentToStagingArea`, `Server_MoveInventoryItemToStagingArea`
   - `Server_PlayerAddKeyFromInventory`, `Server_PlayerRemoveKeyToInventory`
   
   This includes removing every `_Implementation()` and `_Validate()` function body for the above.

4. **Move interaction state** to the net component. `MerchantActor`, `LootedActor`, and `RepairerActor` are owner-only replicated properties on `UInventoryNetComponent`. `TransactionBoolean` also lives there, but is local transaction-guard state and is **not replicated**. Your interface getter/setter implementations should delegate:
   ```cpp
   bool AYourPC::GetTransactionBoolean() { return InventoryNetComponent->TransactionBoolean; }
   void AYourPC::SetTransactionBoolean(bool V) { InventoryNetComponent->TransactionBoolean = V; }
   AActor* AYourPC::GetMerchantActor() { return InventoryNetComponent->MerchantActor.Get(); }
   void AYourPC::SetMerchantActor(AActor* A) { InventoryNetComponent->MerchantActor = A; }
   AActor* AYourPC::GetLootedActor() { return InventoryNetComponent->LootedActor.Get(); }
   void AYourPC::SetLootedActor(AActor* A) { InventoryNetComponent->LootedActor = A; }
   ```

5. **Optionally subclass** `UInventoryNetComponent` if you had game-specific logic (chat messages, backend saves, combat checks, etc.) in your old `Server_*` implementations. Override only the `Handle*()` / `Validate*()` methods you need. See [Customizing Server Behavior](#customizing-server-behavior).

### 2. New Systems: Trade, Repair, Field Repair, Drop

The plugin now includes implementations or RPC routing for several systems that were previously left to the consumer:

| System | Components / Classes | Notes |
|--------|---------------------|-------|
| **Player Trading** | `UTradeComponent`, Trade RPCs on `UInventoryNetComponent`, `UI_TradeWidget` | Player-to-player trade with item + coin offers |
| **NPC Repair** | `URepairComponent`, `IRepairInterface`, Repair RPCs on `UInventoryNetComponent`, `UI_RepairWidget` | NPC-based equipment repair with durability costs |
| **Field Repair** | `UFieldRepairComponent`, `IFieldRepairInterface`, `UInventoryItemFieldRepair`, `UI_FieldRepairWidget` | Self-repair using consumable repair kits |
| **Item Dropping** | `DropItemFromInventory()`, `DropItemFromEquipment()` wrappers + RPCs | Routing is provided, but a game-specific `HandleDropItemFrom*` override is required to spawn safely |

If you had custom implementations for any of these, you can keep your logic by overriding the corresponding `Handle*` methods on your `UInventoryNetComponent` subclass.

### 3. Durability System (**New**)

Equipment items now support durability tracking:

- `UInventoryItemEquipable` gains `TotalDurability` and `DurabilityModifier` properties (via `IInventoryItemDurableInterface`).
- `PlayerRemoveItem()` now **returns `float`** (the removed item's durability) instead of `void`. Update any override signatures accordingly.
- `PlayerAddItemWithDurability()` is a new method for adding items with a specific durability value.
- `EquipItemWithDurability()` added to `IEquipmentInterface`.
- `SpawnItemFromActor()` on `IInventoryGameModeInterface` now accepts an optional `float Durability` parameter.
- `FInventoryItemAdd` delegate changed from 3 params to **4 params** (added `float Durability`). Update any bound callbacks.
- `AddItemAt()` on `UInventoryComponent` now accepts an optional `float Durability` parameter.

### 4. Item Interface Refactor

Items now implement interfaces for type-safe access:

- `UInventoryItemBase` implements `IInventoryItemInterface` (getters for ID, name, weight, etc.)
- `UInventoryItemEquipable` implements `IInventoryItemEquipableInterface`, `IInventoryItemWeaponInterface`, and `IInventoryItemDurableInterface`
- New item interfaces: `IInventoryItemBagInterface`, `IInventoryItemAmmoInterface`, `IInventoryItemAmmoBagInterface`, `IInventoryItemFieldRepairInterface`, `IInventoryItemActivatableInterface`, `IInventoryItemBookInterface`, `IInventoryItemFoodInterface`, `IInventoryItemDrinkInterface`

If you were casting items to concrete classes, consider using the interfaces instead for better decoupling.

### 5. `FItemContainerLine` Moved to Plugin

`FItemContainerLine` (the DataTable row struct for item registration) is now defined in the plugin. If you previously defined this struct in your project, add a redirect:

```ini
[CoreRedirects]
+StructRedirects=(OldName="/Script/YourProject.ItemContainerLine",NewName="/Script/InventoryPlugin.ItemContainerLine")
```

### 6. `IInventoryGameInstanceInterface` — New Optional Overrides

Four new optional methods for coin icon textures:
```cpp
virtual UTexture2D* GetCopperCoinIconTexture() const;  // default: nullptr
virtual UTexture2D* GetSilverCoinIconTexture() const;
virtual UTexture2D* GetGoldCoinIconTexture() const;
virtual UTexture2D* GetPlatinumCoinIconTexture() const;
```
These have default implementations returning `nullptr`, so no action required unless you want custom coin icons.

### 7. `IInventoryGameModeInterface` — Signature Changes

- `SpawnItemFromActor()` now takes an optional `float Durability = 100.0f` parameter.
- `SpawnItemFromActorRaw()` now takes an optional `float Durability = 100.0f` parameter.
- `SpawnCoinsFromActor()` changed from **pure virtual** to **virtual with default implementation**. If you were only forwarding to a default spawn, you can remove your override.

### 8. `IInventoryHUDInterface` — Events and Window Registry (**Major**)

HUD display events have been converted from `BlueprintImplementableEvent` (pure Blueprint) to `BlueprintNativeEvent` with C++ default implementations. The defaults dispatch through a window-registry pattern. Blueprint event overrides are optional, but the game must still return a valid HUD object, retain registered windows, and explicitly connect replicated session state to the HUD when that behavior is desired.

**What changed:**

| Before | After |
|--------|-------|
| `BlueprintImplementableEvent` — must implement in Blueprint | `BlueprintNativeEvent` — C++ default implementation is available |
| `HandleBag(EBagSlot, UBagWidget*)` — pure virtual | `HandleBag(EBagSlot, const TScriptInterface<IInventoryBagWindowInterface>&)` — NativeEvent with default |
| No window registry | `Register*Window()` / `Get*Window()` pairs for each window type |
| No lazy widget creation | `Get*WindowClass()` NativeEvents for on-demand instantiation |

**New events added** (all now `BlueprintNativeEvent`):

- `DisplayRepairScreen(AActor*)` / `HideRepairScreen()` / `OnRepairTransactionComplete()` — NPC repair UI
- `OpenTradeWindow()` / `CloseTradeWindow()` — trade UI
- `DisplayFieldRepairScreen(int32, EBagSlot, int32)` / `HideFieldRepairScreen()` / `NotifyFieldRepairFinished(...)` — field repair UI
- `DisplayItemDescriptionWithDurability(...)` — item tooltip with durability bar
- `LockInventorySlot(EBagSlot, int32, bool)` / `LockEquipmentSlot(EEquipmentSlot, bool)` — slot locking during field repair

For the full integration pattern — window interface contracts, registry accessor implementation, custom window widget example, and lazy creation — see [Step 5: HUD — Window Registry](#step-5-hud--window-registry).

### 9. Enum Changes

- **`EBagSlot`**: Added `Quiver = 7` before `LastValidBag` (now `= 8`). If you had hardcoded `LastValidBag = 7`, update accordingly.
- **`EAmmoType`**: Added `SmallBolts` and `GreatBolts` entries.
- **`EEquipmentSocket`**: Added `RangedSheath`.

### 10. Delegate Signature Changes

- `FInventoryItemAdd` changed from `ThreeParams(EBagSlot, int32, int32)` to `FourParams(EBagSlot, int32, int32, float)` — added `Durability`.
- New delegates: `FInventoryItemDurabilityUpdate`, `FInventoryBagUsageChanged`.

### 11. Ammo System

New ammo-related methods added to interfaces:
- `IInventoryPlayerInterface`: `CanSpendAmmo(EAmmoType)`, `SpendAmmo(EAmmoType)`
- `IEquipmentInterface`: `HasCompatibleAmmoEquipped(EAmmoType)`, `RemoveAmmoEquipped(EAmmoType)`
- `UInventoryComponent`: `HasCompatibleAmmoInQuiver(EAmmoType)`, `RemoveAmmoFromQuiver(EAmmoType)`

These have default implementations and don't require changes unless you use ranged weapons with ammo.

### 12. Merchant — Refuse Items

`IMerchantInterface` now supports merchants that can refuse certain items. Check if your merchant implementations need updating.

### Quick Migration Checklist

- [ ] Add `UInventoryNetComponent` to PlayerController constructor
- [ ] Implement `GetInventoryNetComponent()` pure virtual
- [ ] Remove all `Server_*` UFUNCTION declarations and `_Implementation`/`_Validate` bodies from PlayerController
- [ ] Delegate `TransactionBoolean`, `MerchantActor`, and `LootedActor` getters/setters to the net component
- [ ] Enable replication on each data component; do not add redundant replicated component-pointer properties for constructor-created default subobjects
- [ ] Update `PlayerRemoveItem` overrides to return `float` instead of `void`
- [ ] Update `FInventoryItemAdd` delegate bindings (now 4 params)
- [ ] Optionally subclass `UInventoryNetComponent` for game-specific Handle/Validate overrides
- [ ] Update any hardcoded `EBagSlot::LastValidBag` references
- [ ] Add window registry storage and `Register*Window` / `Get*Window` accessors to your HUD C++ class (see [Step 5](#step-5-hud--window-registry))
- [ ] Implement `Get*WindowClass()` overrides for any game-specific draggable window wrappers
- [ ] Update any `HandleBag` override signature from `(EBagSlot, UBagWidget*)` to `(EBagSlot, const TScriptInterface<IInventoryBagWindowInterface>&)`
- [ ] Implement the appropriate window interface on each custom game-side window widget

---

## Overview

The Inventory Plugin is a fully replicated, grid-based inventory and equipment system for Unreal Engine 5.

### Features

- **Grid-based inventory** with variable bag sizes
- **Full replication** with server authority
- **Equipment system** with skeletal mesh attachment and sheath/unsheath
- **Weight management** and encumbrance
- **Currency system** (Copper, Silver, Gold, Platinum - 10x conversion per tier)
- **Loot system** for pickable items and coins
- **Merchant system** with buy/sell and dynamic inventory
- **Banking system** for persistent storage
- **Trading system** between players
- **Repair system** (NPC repair + field repair kits)
- **Durability tracking** for equipment
- **Keyring system** for key items

### Prerequisites

- Unreal Engine 5.0+
- C++ project
- Four framework classes: `GameInstance`, `GameMode`, `PlayerController`, `Character`

### Integration Responsibility Matrix

| System | Required consumer work | Plugin default |
|--------|------------------------|----------------|
| Core inventory | Implement the GameInstance, GameMode, and PlayerController interfaces; create and replicate the inventory, purse, and staging components | Grid storage, item movement, equip/unequip RPC handling |
| Equipment | Implement `IEquipmentInterface`; create and replicate `UEquipmentComponent`; connect visual meshes | Equipment state and automatic bag/quiver activation on equip and deactivation on unequip |
| HUD | Return a valid HUD interface/object from the PlayerController and retain registered window widgets | Dispatch and lazy creation for bag, loot, merchant, repair, and field-repair windows |
| Bank | Create bank item/coin components and override `GetBankComponent()` / `GetBankCoin()` | Optional interface defaults return `nullptr` |
| Keyring | Create a keyring component and override `GetKeyring()` | Optional interface default returns `nullptr` |
| Player trade | Create `UTradeComponent`, override `GetLocalTradeComponent()`, and register a trade window | Replicated trade state and Server RPC handling; no lazy trade-window creation |
| Repair | Provide repair actors/interfaces and the desired repair window | NPC and field-repair handlers plus lazy repair-window creation |
| World dropping | Override both drop handlers in a net-component subclass and provide a safe spawn/rollback policy | Current defaults remove the item/equipment and log a warning; they do not spawn a world actor |

> **Known code limitation — durability on default spawns:** `SpawnItemFromActor(..., Durability)` and `SpawnItemFromActorRaw(..., Durability)` currently accept a durability argument but do not pass it to the spawned `ADroppedItem`. Until the source implementation is corrected, custom drop code must explicitly call `SetDurability()` (or use a corrected game-side spawn function). Do not rely on the argument alone to preserve durability.

---

## Installation

### 1. Add the Plugin

**Git Submodule (recommended):**
```bash
cd YourProject/Plugins
git submodule add https://github.com/Synock/UE5Inventory.git UE5Inventory
```

**Or manually copy** the plugin into `YourProject/Plugins/UE5Inventory`.

### 2. Enable in `.uproject`

```json
{
    "Plugins": [
        {
            "Name": "InventoryPlugin",
            "Enabled": true
        }
    ]
}
```

### 3. Add Module Dependency

In your project's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "InventoryPlugin"
});
```

### 4. Regenerate and Compile

Right-click `.uproject` -> Generate Visual Studio project files, then build.

---

## Architecture

```
GameInstance (IInventoryGameInstanceInterface)
    +-- Item Registry (ItemID -> UInventoryItemBase*)

GameMode (IInventoryGameModeInterface)
    +-- Item Spawning (ADroppedItem / ADroppedCoins)
    +-- Item Lookup (delegates to GameInstance)

PlayerController (IInventoryPlayerInterface)
    +-- UInventoryNetComponent  (Server RPCs, replicated interaction state)
    +-- UInventoryComponent     (bag storage)
    +-- UCoinComponent          (player currency)
    +-- UStagingAreaComponent   (trade/merchant staging)
    +-- UCoinComponent          (staging coins)
    +-- UBankComponent          (bank storage)
    +-- UCoinComponent          (bank coins)
    +-- UKeyringComponent       (key items, optional)
    +-- UTradeComponent         (trade state, optional)
    +-- UFieldRepairComponent   (field repair, optional)

Character (IEquipmentInterface + IInventoryModularCharacterInterface)
    +-- UEquipmentComponent     (worn items + visual meshes)
```

Inventory mutations are server-authoritative. The `UInventoryNetComponent` owns the Server RPCs for inventory operations (loot, equip, trade, merchant, repair, etc.). Selected RPCs use `WithValidation`; others rely on their handler guards. Data components replicate their own state via `ReplicatedUsing` callbacks that fire delegates for UI updates. The net component's `LootedActor`, `MerchantActor`, and `RepairerActor` references replicate owner-only; `TransactionBoolean` is local-only.

---

## Step-by-Step Integration

> For detailed interface documentation, see the [Interface Implementation Guide](./Interface_Implementation_Guide.md).

### Step 1: GameInstance - Item Registry

Your `GameInstance` implements `IInventoryGameInstanceInterface` to serve as the global item lookup table.

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/InventoryGameInstanceInterface.h"
#include "YourGameInstance.generated.h"

UCLASS()
class YOURPROJECT_API UYourGameInstance : public UGameInstance,
    public IInventoryGameInstanceInterface
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    // IInventoryGameInstanceInterface
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;

    // Optional coin icon overrides (return nullptr to use widget defaults)
    virtual UTexture2D* GetCopperCoinIconTexture() const override;
    virtual UTexture2D* GetSilverCoinIconTexture() const override;
    virtual UTexture2D* GetGoldCoinIconTexture() const override;
    virtual UTexture2D* GetPlatinumCoinIconTexture() const override;

protected:
    UPROPERTY()
    TMap<int32, UInventoryItemBase*> ItemLUT;

    // Assign in the Blueprint derived from this class
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* CopperCoinIcon = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* SilverCoinIcon = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* GoldCoinIcon = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* PlatinumCoinIcon = nullptr;

    // DataTable for bulk item registration (row struct: FItemContainerLine)
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Items")
    UDataTable* ItemDataTable = nullptr;

    // Manual item list for items not in the DataTable
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Items")
    TArray<UInventoryItemBase*> ItemTable;

    void SetupInternals();
};
```

**Implementation:**
```cpp
#include "YourGameInstance.h"
#include "Items/InventoryItemBase.h"

void UYourGameInstance::Init()
{
    Super::Init();
    SetupInternals();
}

void UYourGameInstance::SetupInternals()
{
    // Register items from DataTable (uses FItemContainerLine, built into plugin)
    if (ItemDataTable)
    {
        for (auto& Row : ItemDataTable->GetRowMap())
        {
            FItemContainerLine* ItemRow = reinterpret_cast<FItemContainerLine*>(Row.Value);
            if (ItemRow && ItemRow->Item)
                ItemLUT.Add(ItemRow->Item->ItemID, ItemRow->Item);
        }
    }

    // Register items from manual array
    for (auto& Item : ItemTable)
    {
        if (Item)
            ItemLUT.Add(Item->ItemID, Item);
    }

    UE_LOG(LogTemp, Log, TEXT("Registered %d items"), ItemLUT.Num());
}

UInventoryItemBase* UYourGameInstance::FetchItemFromID(int32 ID)
{
    if (UInventoryItemBase** Found = ItemLUT.Find(ID))
        return *Found;
    return nullptr;
}

void UYourGameInstance::RegisterItem(UInventoryItemBase* NewItem)
{
    if (NewItem && NewItem->ItemID >= 0)
        ItemLUT.Add(NewItem->ItemID, NewItem);
}

UTexture2D* UYourGameInstance::GetCopperCoinIconTexture() const { return CopperCoinIcon; }
UTexture2D* UYourGameInstance::GetSilverCoinIconTexture() const { return SilverCoinIcon; }
UTexture2D* UYourGameInstance::GetGoldCoinIconTexture() const { return GoldCoinIcon; }
UTexture2D* UYourGameInstance::GetPlatinumCoinIconTexture() const { return PlatinumCoinIcon; }
```

**DataTable Setup** (in editor):
1. Create a DataTable with row struct `FItemContainerLine` (provided by the plugin)
2. Add rows pointing to your `UInventoryItemBase` Data Assets
3. Assign the DataTable to your GameInstance Blueprint's `ItemDataTable` property

---

### Step 2: GameMode - Item Spawning

Your `GameMode` implements `IInventoryGameModeInterface`. The spawn methods (`SpawnItemFromActor`, `SpawnCoinsFromActor`, etc.) have **default implementations** in the interface - you only need to override `FetchItemFromID` and `RegisterItem`.

This is sufficient for the basic built-in dropped-actor types. Override the spawn methods if the game needs custom actor subclasses or transactional persistence. The current item spawn defaults also ignore their `Durability` argument; see the known limitation above.

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InventoryGameModeInterface.h"
#include "YourGameMode.generated.h"

UCLASS()
class YOURPROJECT_API AYourGameMode : public AGameModeBase,
    public IInventoryGameModeInterface
{
    GENERATED_BODY()

public:
    // Required overrides
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;
};
```

**Implementation:**
```cpp
#include "YourGameMode.h"
#include "YourGameInstance.h"

UInventoryItemBase* AYourGameMode::FetchItemFromID(int32 ID)
{
    if (auto* GI = Cast<UYourGameInstance>(GetGameInstance()))
        return GI->FetchItemFromID(ID);
    return nullptr;
}

void AYourGameMode::RegisterItem(UInventoryItemBase* NewItem)
{
    if (auto* GI = Cast<UYourGameInstance>(GetGameInstance()))
        GI->RegisterItem(NewItem);
}
```

---

### Step 3: PlayerController - Inventory Components and Net Component

Your `PlayerController` implements `IInventoryPlayerInterface` and creates its inventory-related components, including the `UInventoryNetComponent` that owns the plugin's inventory Server RPCs.

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "YourPlayerController.generated.h"

class UInventoryComponent;
class UInventoryNetComponent;
class UCoinComponent;
class UStagingAreaComponent;
class UBankComponent;
class UKeyringComponent;
class UTradeComponent;
class UUserWidget;
class IInventoryHUDInterface;

UCLASS()
class YOURPROJECT_API AYourPlayerController : public APlayerController,
    public IInventoryPlayerInterface
{
    GENERATED_BODY()

public:
    AYourPlayerController();

    // IInventoryPlayerInterface - required overrides
    virtual UInventoryNetComponent* GetInventoryNetComponent() override;
    virtual UInventoryComponent* GetInventoryComponent() override;
    virtual const UInventoryComponent* GetInventoryComponentConst() const override;
    virtual UCoinComponent* GetCoinComponent() override;
    virtual const UCoinComponent* GetCoinComponentConst() const override;
    virtual AActor* GetInventoryOwningActor() override;
    virtual AActor const* GetInventoryOwningActorConst() const override;
    virtual bool GetTransactionBoolean() override;
    virtual void SetTransactionBoolean(bool Value) override;
    virtual AActor* GetMerchantActor() override;
    virtual const AActor* GetMerchantActorConst() const override;
    virtual void SetMerchantActor(AActor* Actor) override;
    virtual AActor* GetLootedActor() override;
    virtual const AActor* GetLootedActorConst() const override;
    virtual void SetLootedActor(AActor* Actor) override;
    virtual IInventoryHUDInterface* GetInventoryHUDInterface() override;
    virtual UObject* GetInventoryHUDObject() override;
    virtual UCoinComponent* GetStagingAreaCoin() override;
    virtual UStagingAreaComponent* GetStagingAreaItems() override;
    virtual FOnWeightChanged& GetWeightChangedDelegate() override;

    // Optional systems shown in this example. Their interface defaults return nullptr,
    // so override the getters whenever the corresponding component is installed.
    virtual UCoinComponent* GetBankCoin() const override;
    virtual UBankComponent* GetBankComponent() const override;
    virtual UKeyringComponent* GetKeyring() const override;
    virtual UTradeComponent* GetLocalTradeComponent() const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryNetComponent> InventoryNetComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UCoinComponent> CoinPurse;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UStagingAreaComponent> StagingAreaItems;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UCoinComponent> StagingAreaCoin;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UBankComponent> BankComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UCoinComponent> BankCoin;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UKeyringComponent> KeyringComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UTradeComponent> TradeComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory|UI")
    TSubclassOf<UUserWidget> InventoryHUDClass;
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> InventoryHUD;

    UPROPERTY(BlueprintAssignable)
    FOnWeightChanged WeightDispatcher;

    virtual void BeginPlay() override;
};
```

**Implementation:**
```cpp
#include "YourPlayerController.h"
#include "Components/InventoryNetComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/CoinComponent.h"
#include "Components/StagingAreaComponent.h"
#include "Components/BankComponent.h"
#include "Components/KeyringComponent.h"
#include "Components/TradeComponent.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "Blueprint/UserWidget.h"

AYourPlayerController::AYourPlayerController()
{
    // Net component owns inventory Server RPCs, owner-only actor references, and local transaction state
    InventoryNetComponent = CreateDefaultSubobject<UInventoryNetComponent>(TEXT("InventoryNetComponent"));

    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
    Inventory->SetNetAddressable();
    Inventory->SetIsReplicated(true);

    CoinPurse = CreateDefaultSubobject<UCoinComponent>(TEXT("CoinPurse"));
    CoinPurse->SetNetAddressable();
    CoinPurse->SetIsReplicated(true);

    StagingAreaItems = CreateDefaultSubobject<UStagingAreaComponent>(TEXT("StagingAreaItems"));
    StagingAreaItems->SetNetAddressable();
    StagingAreaItems->SetIsReplicated(true);

    StagingAreaCoin = CreateDefaultSubobject<UCoinComponent>(TEXT("StagingAreaCoin"));
    StagingAreaCoin->SetNetAddressable();
    StagingAreaCoin->SetIsReplicated(true);

    BankComponent = CreateDefaultSubobject<UBankComponent>(TEXT("BankComponent"));
    BankComponent->SetNetAddressable();
    BankComponent->SetIsReplicated(true);

    BankCoin = CreateDefaultSubobject<UCoinComponent>(TEXT("BankCoin"));
    BankCoin->SetNetAddressable();
    BankCoin->SetIsReplicated(true);

    KeyringComponent = CreateDefaultSubobject<UKeyringComponent>(TEXT("KeyringComponent"));
    KeyringComponent->SetNetAddressable();
    KeyringComponent->SetIsReplicated(true);

    // UTradeComponent, like UInventoryNetComponent, enables replication by default.
    TradeComponent = CreateDefaultSubobject<UTradeComponent>(TEXT("TradeComponent"));
}

void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // HUD widgets are local-only. The PlayerController getters below are the bridge used by
    // plugin components and interface wrappers to dispatch UI events.
    if (IsLocalController() && InventoryHUDClass)
    {
        InventoryHUD = CreateWidget<UUserWidget>(this, InventoryHUDClass);
        if (InventoryHUD && InventoryHUD->GetClass()->ImplementsInterface(UInventoryHUDInterface::StaticClass()))
        {
            InventoryHUD->AddToViewport();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InventoryHUDClass must implement IInventoryHUDInterface"));
            InventoryHUD = nullptr;
        }
    }
}

// Interface getters - delegate to components
UInventoryNetComponent* AYourPlayerController::GetInventoryNetComponent()
{
    return InventoryNetComponent;
}

UInventoryComponent* AYourPlayerController::GetInventoryComponent() { return Inventory; }
const UInventoryComponent* AYourPlayerController::GetInventoryComponentConst() const { return Inventory; }
UCoinComponent* AYourPlayerController::GetCoinComponent() { return CoinPurse; }
const UCoinComponent* AYourPlayerController::GetCoinComponentConst() const { return CoinPurse; }
AActor* AYourPlayerController::GetInventoryOwningActor() { return GetPawn(); }
AActor const* AYourPlayerController::GetInventoryOwningActorConst() const { return GetPawn(); }

// Transaction state, merchant, and looted actor delegate to the net component
bool AYourPlayerController::GetTransactionBoolean()
{
    return InventoryNetComponent ? InventoryNetComponent->TransactionBoolean : false;
}

void AYourPlayerController::SetTransactionBoolean(bool Value)
{
    if (InventoryNetComponent)
        InventoryNetComponent->TransactionBoolean = Value;
}

AActor* AYourPlayerController::GetMerchantActor()
{
    return InventoryNetComponent ? InventoryNetComponent->MerchantActor.Get() : nullptr;
}

const AActor* AYourPlayerController::GetMerchantActorConst() const
{
    return InventoryNetComponent ? InventoryNetComponent->MerchantActor.Get() : nullptr;
}

void AYourPlayerController::SetMerchantActor(AActor* Actor)
{
    if (InventoryNetComponent)
        InventoryNetComponent->MerchantActor = Actor;
}

AActor* AYourPlayerController::GetLootedActor()
{
    return InventoryNetComponent ? InventoryNetComponent->LootedActor.Get() : nullptr;
}

const AActor* AYourPlayerController::GetLootedActorConst() const
{
    return InventoryNetComponent ? InventoryNetComponent->LootedActor.Get() : nullptr;
}

void AYourPlayerController::SetLootedActor(AActor* Actor)
{
    if (InventoryNetComponent)
        InventoryNetComponent->LootedActor = Actor;
}

UCoinComponent* AYourPlayerController::GetStagingAreaCoin() { return StagingAreaCoin; }
UStagingAreaComponent* AYourPlayerController::GetStagingAreaItems() { return StagingAreaItems; }
FOnWeightChanged& AYourPlayerController::GetWeightChangedDelegate() { return WeightDispatcher; }

IInventoryHUDInterface* AYourPlayerController::GetInventoryHUDInterface()
{
    return Cast<IInventoryHUDInterface>(InventoryHUD.Get());
}

UObject* AYourPlayerController::GetInventoryHUDObject()
{
    return InventoryHUD.Get();
}

UCoinComponent* AYourPlayerController::GetBankCoin() const { return BankCoin; }
UBankComponent* AYourPlayerController::GetBankComponent() const { return BankComponent; }
UKeyringComponent* AYourPlayerController::GetKeyring() const { return KeyringComponent; }
UTradeComponent* AYourPlayerController::GetLocalTradeComponent() const { return TradeComponent; }
```

**Key points:**
- `MerchantActor`, `LootedActor`, and `RepairerActor` live on `UInventoryNetComponent` as owner-only replicated properties. `TransactionBoolean` is stored there too, but is local-only. The PlayerController interface getters delegate to the component.
- `UInventoryNetComponent` calls `SetIsReplicatedByDefault(true)` in its own constructor, so you do **not** need to call `SetNetAddressable()` or `SetIsReplicated(true)` on it.
- Constructor-created replicated ActorComponents replicate their internal properties after `SetIsReplicated(true)` (or `SetIsReplicatedByDefault(true)`). Replicating the owning class's component pointer with `DOREPLIFETIME` is redundant for these default subobjects.
- `GetInventoryHUDInterface()` and `GetInventoryHUDObject()` must refer to the same live UObject. Several plugin call paths dispatch directly through these getters.
- If bank, keyring, or trade is omitted, do not create the component and leave its optional getter at the `nullptr` default. Do not create a component while forgetting to override its getter.

---

### Step 4: Character - Equipment

Your `Character` implements `IEquipmentInterface` (and optionally `IInventoryModularCharacterInterface` for modular mesh support).

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/InventoryModularCharacterInterface.h"
#include "YourCharacter.generated.h"

class UEquipmentComponent;

UCLASS()
class YOURPROJECT_API AYourCharacter : public ACharacter,
    public IEquipmentInterface,
    public IInventoryModularCharacterInterface
{
    GENERATED_BODY()

public:
    AYourCharacter(const FObjectInitializer& ObjectInitializer);

    // IEquipmentInterface
    virtual UEquipmentComponent* GetEquipmentComponent() override;
    virtual const UEquipmentComponent* GetEquipmentComponentConst() const override;

    // IInventoryModularCharacterInterface (optional overrides — all default to nullptr)
    // Override only the body-part getters you use for modular mesh swapping, e.g.:
    //   virtual USkeletalMeshComponent* GetHeadComponent() override;
    //   virtual USkeletalMeshComponent* GetTorsoComponent() override;
    //   virtual USkeletalMeshComponent* GetArmsComponent() override;
    //   virtual USkeletalMeshComponent* GetHandsComponent() override;
    //   virtual USkeletalMeshComponent* GetLegsComponent() override;
    //   virtual USkeletalMeshComponent* GetFootComponent() override;
    // See IInventoryModularCharacterInterface for the full list.

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TObjectPtr<UEquipmentComponent> Equipment;

    virtual void BeginPlay() override;
};
```

**Implementation:**
```cpp
#include "YourCharacter.h"
#include "Components/EquipmentComponent.h"

AYourCharacter::AYourCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Equipment = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));
    Equipment->SetNetAddressable();
    Equipment->SetIsReplicated(true);
    // Do NOT call Equipment->UpdateMasterMeshComponent here — GetMesh() components are
    // not fully initialized during the constructor. Call it in BeginPlay() instead.
}

void AYourCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Standalone and listen-server hosts also render equipment. Skip visual setup only
    // on a dedicated server, which has no local scene to render.
    if (GetNetMode() != NM_DedicatedServer)
    {
        Equipment->UpdateMasterMeshComponent(GetMesh());
    }
}

UEquipmentComponent* AYourCharacter::GetEquipmentComponent() { return Equipment; }
const UEquipmentComponent* AYourCharacter::GetEquipmentComponentConst() const { return Equipment; }
```

---

### Step 5: HUD — Window Registry

Your HUD widget implements `IInventoryHUDInterface` and acts as the plugin's UI dispatch point. The display methods are `BlueprintNativeEvent`s with C++ defaults, but the registry accessors themselves default to empty/no-op implementations. Your HUD must retain and return registered windows for later hide, refresh, and reuse calls.

#### How the dispatch model works

Each displayed widget has an interface the plugin dispatches through. For a lazy-capable window such as loot, `DisplayLootScreen`:

1. Calls `GetLootWindow()` on the HUD to retrieve the registered window.
2. If none is registered and `GetLootWindowClass()` returns a valid class, it instantiates the widget lazily and registers it automatically.
3. Calls `Execute_InitLootWindow` / `Execute_ShowLootWindow` on the window widget.

Your responsibility is to (a) store the window widgets in GC-tracked properties, (b) expose them through the registry accessors, and (c) ensure each widget class implements the corresponding interface.

The lifecycle is not identical for every UI type:

- **Lazy-created and registered:** bag, loot, merchant, repair, and field repair.
- **Must be created and registered by the game:** main inventory, keyring, and trade.
- **Created per display rather than kept in this registry:** item descriptions and books/notes.

#### Window interface contracts

| Interface | Purpose | Built-in implementation |
|-----------|---------|------------------------|
| `IInventoryWindowInterface` | Main inventory panel (equipment + grids) | — (game-side) |
| `IInventoryBagWindowInterface` | Single bag popup window | `UBagWidget` |
| `IInventoryLootWindowInterface` | Loot window | `ULootScreenWidget` |
| `IInventoryMerchantWindowInterface` | Merchant window | — (game-side) |
| `IInventoryRepairWindowInterface` | NPC repair window | — (game-side) |
| `ITradeWindowInterface` | Player-to-player trade window | — (game-side) |
| `IKeyringWindowInterface` | Keyring window | `UKeyringWidget` |
| `IFieldRepairWidgetInterface` | Field repair window | `UFieldRepairWidget` |
| `IInventoryBookWidgetInterface` | Readable book display | `UInventoryBookWidget` |
| `IInventoryItemDescriptionWidgetInterface` | Item description tooltip | `UItemDescriptionWidget` |

#### HUD C++ class — storage and registry accessors

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/InventoryHUDInterface.h"  // pulls in all window interface headers
#include "YourHUDWidget.generated.h"

UCLASS()
class YOURPROJECT_API UYourHUDWidget : public UUserWidget,
    public IInventoryHUDInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // IInventoryHUDInterface window registry
    virtual TScriptInterface<IInventoryWindowInterface>         GetInventoryWindow() const override         { return InventoryWindow; }
    virtual void RegisterInventoryWindow(TScriptInterface<IInventoryWindowInterface> W) override            { InventoryWindow = W; }
    virtual TScriptInterface<IKeyringWindowInterface>           GetKeyringWindow() const override           { return KeyringWindow; }
    virtual void RegisterKeyringWindow(TScriptInterface<IKeyringWindowInterface> W) override                { KeyringWindow = W; }
    virtual TScriptInterface<IInventoryLootWindowInterface>     GetLootWindow() const override              { return LootWindow; }
    virtual void RegisterLootWindow(TScriptInterface<IInventoryLootWindowInterface> W) override             { LootWindow = W; }
    virtual TScriptInterface<IInventoryMerchantWindowInterface> GetMerchantWindow() const override         { return MerchantWindow; }
    virtual void RegisterMerchantWindow(TScriptInterface<IInventoryMerchantWindowInterface> W) override    { MerchantWindow = W; }
    virtual TScriptInterface<IInventoryRepairWindowInterface>   GetRepairWindow() const override           { return RepairWindow; }
    virtual void RegisterRepairWindow(TScriptInterface<IInventoryRepairWindowInterface> W) override        { RepairWindow = W; }
    virtual TScriptInterface<ITradeWindowInterface>             GetTradeWindow() const override            { return TradeWindow; }
    virtual void RegisterTradeWindow(TScriptInterface<ITradeWindowInterface> W) override                   { TradeWindow = W; }
    virtual TScriptInterface<IFieldRepairWidgetInterface>       GetFieldRepairWindow() const override      { return FieldRepairWindow; }
    virtual void RegisterFieldRepairWindow(TScriptInterface<IFieldRepairWidgetInterface> W) override       { FieldRepairWindow = W; }

    // Bag slot registry
    virtual TScriptInterface<IInventoryBagWindowInterface> GetBagWindowForSlot(EBagSlot Slot) const override
    {
        const auto* Found = BagWindowMap.Find(Slot);
        return Found ? *Found : TScriptInterface<IInventoryBagWindowInterface>();
    }
    virtual void RegisterBagWindowForSlot(EBagSlot Slot,
                                          TScriptInterface<IInventoryBagWindowInterface> W) override
        { BagWindowMap.Add(Slot, W); }
    virtual TArray<EBagSlot> GetRegisteredBagSlots() const override
    {
        TArray<EBagSlot> Out;
        BagWindowMap.GetKeys(Out);
        return Out;
    }

    // Use a custom wrapper when assigned; otherwise preserve the plugin's built-in default.
    virtual TSubclassOf<UUserWidget> GetLootWindowClass_Implementation() const override
        { return LootWindowClass ? LootWindowClass : IInventoryHUDInterface::GetLootWindowClass_Implementation(); }
    virtual TSubclassOf<UUserWidget> GetMerchantWindowClass_Implementation() const override
        { return MerchantWindowClass ? MerchantWindowClass : IInventoryHUDInterface::GetMerchantWindowClass_Implementation(); }
    virtual TSubclassOf<UUserWidget> GetBagWindowClass_Implementation() const override
        { return BagWindowClass ? BagWindowClass : IInventoryHUDInterface::GetBagWindowClass_Implementation(); }
    virtual TSubclassOf<UUserWidget> GetRepairWindowClass_Implementation() const override
        { return RepairWindowClass ? RepairWindowClass : IInventoryHUDInterface::GetRepairWindowClass_Implementation(); }
    virtual TSubclassOf<UUserWidget> GetFieldRepairWidgetClass_Implementation() const override
        { return FieldRepairWindowClass ? FieldRepairWindowClass : IInventoryHUDInterface::GetFieldRepairWidgetClass_Implementation(); }

protected:
    // Window storage
    UPROPERTY(Transient)
    TScriptInterface<IInventoryWindowInterface>         InventoryWindow;
    UPROPERTY(Transient)
    TScriptInterface<IKeyringWindowInterface>           KeyringWindow;
    UPROPERTY(Transient)
    TScriptInterface<IInventoryLootWindowInterface>     LootWindow;
    UPROPERTY(Transient)
    TScriptInterface<IInventoryMerchantWindowInterface> MerchantWindow;
    UPROPERTY(Transient)
    TScriptInterface<IInventoryRepairWindowInterface>   RepairWindow;
    UPROPERTY(Transient)
    TScriptInterface<ITradeWindowInterface>             TradeWindow;
    UPROPERTY(Transient)
    TScriptInterface<IFieldRepairWidgetInterface>       FieldRepairWindow;
    UPROPERTY(Transient)
    TMap<EBagSlot, TScriptInterface<IInventoryBagWindowInterface>> BagWindowMap;

    // Assign in the Blueprint subclass. Inventory is pre-created below; the other
    // classes override plugin lazy-creation defaults when a game-specific wrapper is wanted.
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> InventoryWindowClass;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> LootWindowClass;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> MerchantWindowClass;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> BagWindowClass;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> RepairWindowClass;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Windows")
    TSubclassOf<UUserWidget> FieldRepairWindowClass;
};
```

**Implementation — registering windows in `NativeConstruct`:**

Pre-create windows that should exist before first user interaction. Windows opened infrequently (loot, merchant, repair) can rely on lazy creation via the `Get*WindowClass()` overrides above.

```cpp
void UYourHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Pre-create the main inventory window
    if (!InventoryWindow.GetObject() && InventoryWindowClass &&
        InventoryWindowClass->ImplementsInterface(UInventoryWindowInterface::StaticClass()))
    {
        if (UUserWidget* W = CreateWidget<UUserWidget>(GetOwningPlayer(), InventoryWindowClass))
        {
            W->AddToViewport();
            RegisterInventoryWindow(TScriptInterface<IInventoryWindowInterface>(W));
            IInventoryWindowInterface::Execute_HideInventoryWindow(W);
        }
    }

    // Pre-create a bag window for each bag slot the player can equip
    if (BagWindowClass &&
        BagWindowClass->ImplementsInterface(UInventoryBagWindowInterface::StaticClass()))
    {
        for (EBagSlot Slot : { EBagSlot::WaistBag1, EBagSlot::WaistBag2,
                              EBagSlot::BackPack1, EBagSlot::BackPack2 })
        {
            if (GetBagWindowForSlot(Slot).GetObject())
                continue;

            if (UUserWidget* BagW = CreateWidget<UUserWidget>(GetOwningPlayer(), BagWindowClass))
            {
                BagW->AddToViewport();
                RegisterBagWindowForSlot(Slot, TScriptInterface<IInventoryBagWindowInterface>(BagW));
                IInventoryBagWindowInterface::Execute_HideBagWindow(BagW);
            }
        }
    }

    // Loot / merchant / repair use lazy creation — GetLootWindowClass() etc. handle instantiation
    // on the first DisplayLootScreen / DisplayMerchantScreen / DisplayRepairScreen call.
}
```

#### Implementing a custom window widget

Any widget registered with the plugin must implement the corresponding interface. The built-in plugin widgets (`UBagWidget`, `ULootScreenWidget`, `UKeyringWidget`, `UInventoryBookWidget`, `UFieldRepairWidget`, `UItemDescriptionWidget`) already do so. For game-specific wrappers (e.g., a draggable frame), implement the interface and delegate to the inner plugin widget:

```cpp
UCLASS()
class YOURPROJECT_API UYourInventoryWindow : public UUserWidget,
    public IInventoryWindowInterface
{
    GENERATED_BODY()

    UPROPERTY(meta = (BindWidget))
    UInventoryEquipmentWidget* EquipmentPanel;

    UPROPERTY(meta = (BindWidget))
    UInventoryBagsWidget* BagsPanel;

public:
    virtual void ShowInventoryWindow_Implementation() override
        { SetVisibility(ESlateVisibility::SelfHitTestInvisible); }
    virtual void HideInventoryWindow_Implementation() override
        { SetVisibility(ESlateVisibility::Collapsed); }
    virtual bool IsInventoryWindowVisible_Implementation() const override
        { return GetVisibility() != ESlateVisibility::Collapsed; }
    virtual void RefreshInventoryEquipments_Implementation() override
        { if (EquipmentPanel) EquipmentPanel->ForceRefresh(); }
    virtual void RefreshInventoryGrids_Implementation() override
        { if (BagsPanel) BagsPanel->Refresh(); }
};
```

---

## Inventory Initialization

### Component-Side: Bag Lifecycle

Pockets (`Pocket1`, `Pocket2`) are **pre-initialized** at construction time (3×2 grid, `EItemSize::Medium`, validity = `true`) and require no explicit setup. Call `BagSet()` only when you want to override the defaults (e.g. a larger pocket size for a specific character class).

All other bag slots are constructed but start inactive. The default `IEquipmentInterface` flow activates them automatically when the corresponding bag item is equipped and deactivates them on unequip. For quivers, it also applies and clears the ammo-type restriction.

If a character class needs non-default pockets, add the override to the existing PlayerController `BeginPlay()` from Step 3 (do not define a second `BeginPlay()`):

```cpp
if (HasAuthority() && Inventory)
{
    Inventory->BagSet(EBagSlot::Pocket1, true, 4, 3, EItemSize::Giant, 1.0f);
}
```

Create and register the keyring and trade windows in the same guarded manner when those optional systems are enabled; neither has a lazy `Get*WindowClass()` path. The guards matter because `NativeConstruct()` may run again if a widget is removed and re-added.

Do **not** repeat `BagSet()` after a normal `IEquipmentInterface::EquipItem` call. `HandleEquipmentEffect()` already maps the equipment slot, configures the grid, converts the item's reduction value to the storage ratio with `Clamp(1 - GetWeightReduction(), 0, 1)`, and performs quiver-specific setup. A second manual call is redundant and can apply the wrong weight ratio. If you override `HandleEquipmentEffect()` or `HandleUnEquipmentEffect()`, call the base implementation to retain this lifecycle.

### Widget-Side: Initializing `UInventoryGridWidget`

#### Option A — Blueprint-configurable (no code required for player bags)

Set the `Bag ID` property directly on the widget in the Blueprint editor (Details panel → **Inventory | Bag → Bag ID**). `UInventoryGridWidget` will call `InitData` automatically in `NativeConstruct` using `GetOwningPlayerPawn()` as the owner.

This works for all player-owned slots: `Pocket1`, `Pocket2`, `WaistBag1`, `WaistBag2`, `BackPack1`, `BackPack2`, `BankPool`, `Quiver`.

> **Note:** Auto-init is skipped for `LootPool` because its owner is a world actor, not the player pawn. Call `InitData` explicitly for loot grids (see Option B).

#### Option B — Explicit `InitData` call (required for `LootPool`, optional for all others)

```cpp
// Player bags — called from parent widget's InitData (e.g. UInventoryBagWidget)
PocketGrid1->InitData(GetOwningPlayerPawn(), EBagSlot::Pocket1);

// Loot pool — must pass the specific lootable actor
LootGrid->InitData(LootableActor, EBagSlot::LootPool);
```

`InitData` reads the actual grid dimensions from `UInventoryComponent` (respecting any `BagSet()` overrides), so pocket size stays consistent between the component and the widget.

---

## Item Creation

> For detailed item configuration, see the [Item System Guide](./Item_System_Guide.md).

1. Right-click in Content Browser -> **Miscellaneous** -> **Data Asset**
2. Select the appropriate parent class (e.g., `InventoryItemBase`, `InventoryItemEquipable`, `InventoryItemBag`)
3. Configure properties (ItemID, Name, Icon, Width, Height, etc.)
4. Add to your DataTable or `ItemTable` array in the GameInstance Blueprint

---

## UI Integration

The plugin provides ready-made UMG widgets under the `/InventoryPlugin/UI/` content mount. Key widgets:

| Widget | Purpose | Implements interface |
|--------|---------|---------------------|
| `UI_BagWidget` | Single bag popup window | `IInventoryBagWindowInterface` |
| `UI_InventoryGrid` | Grid-based item layout | — |
| `UI_EquipmentSlot` | Single equipment slot | — |
| `UI_Purse` | Currency display | — |
| `UI_LootWidget` | Loot window | `IInventoryLootWindowInterface` |
| `UI_MerchantSellwidget` | Merchant buy/sell / default merchant window | `IInventoryMerchantWindowInterface` |
| `UI_MerchantItemList` | Merchant item list | — |
| `UI_BankWidget` | Bank storage | — |
| `UI_TradeWidget` | Player-to-player trade (inner) | — |
| `UI_RepairWidget` | NPC repair / default repair window | `IInventoryRepairWindowInterface` |
| `UI_BookWidget` | Readable book display | `IInventoryBookWidgetInterface` |
| `UI_KeyringWidget` | Keyring display | `IKeyringWindowInterface` |
| `UI_FieldRepairWidget` | Field repair (inner) | `IFieldRepairWidgetInterface` |

Core widget behavior is implemented in C++. Customize it by subclassing the C++ widget and overriding its virtual/`BlueprintNativeEvent` methods; Blueprint subclasses may still provide layout, bindings, animations, and NativeEvent overrides.

### Connecting the HUD

The window registry from [Step 5](#step-5-hud--window-registry) connects the plugin's C++ dispatch to your UI. Once the PlayerController returns that HUD object and the HUD retains its registered windows, calls such as `DisplayLootScreen`, `DisplayMerchantScreen`, and `DisplayBag` use the C++ defaults without requiring Blueprint event-graph glue.

Bind component delegates for data-driven UI updates:

```cpp
// In your HUD or widget NativeConstruct
Inventory->FullInventoryDispatcher.AddDynamic(this, &UMyWidget::OnInventoryChanged);
Inventory->InventoryItemAdd.AddDynamic(this, &UMyWidget::OnItemAdded);
Equipment->EquipmentDispatcher.AddDynamic(this, &UMyWidget::OnEquipmentChanged);
CoinPurse->PurseDispatcher.AddDynamic(this, &UMyWidget::OnCoinChanged);
```

### Window lifecycle

When `DisplayLootScreen(LootActor)` is called, the C++ default:

1. Calls `GetLootWindow()` on the HUD.
2. If empty, instantiates `GetLootWindowClass()` and registers it via `RegisterLootWindow()`.
3. Calls `Execute_InitLootWindow(LootActor)` on the window.
4. Calls `Execute_ShowLootWindow()`.
5. Positions the window near the mouse cursor on first creation.

Merchant, repair, field repair, and bag windows use the same registered/lazy-created pattern. Trade requires a pre-registered window. The main inventory and keyring also require registration. Item descriptions and books are created per display and are not stored in these registry accessors.

### `UInventoryGridWidget` — editor-friendly initialization

Set the **Bag ID** property in the Blueprint editor (Details → **Inventory | Bag → Bag ID**); `UInventoryGridWidget` calls `InitData` automatically in `NativeConstruct` using the owning player pawn. Explicit `InitData` is only required for `LootPool` grids where the owner is a world actor:

```cpp
// Player grid — Bag ID set via Blueprint editor, no code needed
// LootPool grid — must call explicitly
LootGrid->InitData(LootableActor, EBagSlot::LootPool);
```

---

## Multiplayer

All inventory mutations happen on the server via `UInventoryNetComponent`'s Server RPCs. Client code calls public wrapper methods on `IInventoryPlayerInterface` (e.g., `PlayerUnequipItem()`, `PlayerRequestTrade()`), which forward to the appropriate `Server_*` RPC on the net component.

**Replication flow:**
1. Client triggers UI action (e.g., drag item to equipment slot)
2. Interface wrapper calls `Server_*` RPC on `UInventoryNetComponent`
3. Server validates the request (`Validate*` virtual), then executes it (`Handle*` virtual)
4. Server modifies component state (e.g., `UInventoryComponent`, `UEquipmentComponent`)
5. `ReplicatedUsing` callback fires on clients -> delegates broadcast -> UI updates

**Loot ownership contract:** `ULootPoolComponent::Items` is owner-only. After a custom lootable accepts `StartLooting(Looter)` and reports itself as being looted, `UInventoryNetComponent` temporarily assigns the loot actor to the looting PlayerController, flushes dormancy, and restores the previous owner when the session stops. Custom lootables must implement consistent `StartLooting` / `StopLooting` acceptance state; consuming game actors should not independently retarget ownership around this plugin-managed session.

**Component setup checklist:**
- `CreateDefaultSubobject<>()` in constructor for all components
- `SetNetAddressable()` + `SetIsReplicated(true)` on data components (`UInventoryComponent`, `UCoinComponent`, etc.)
- `UInventoryNetComponent` handles its own replication setup (`SetIsReplicatedByDefault(true)`)
- Do not mark constructor-created component pointer properties as replicated merely to replicate the component. The component's own replication setting and its internal `GetLifetimeReplicatedProps()` control its state.
- `LootedActor`, `MerchantActor`, and `RepairerActor` replicate owner-only from the net component. Their default `OnRep_*` functions are empty; override them (or drive the HUD through another explicit client-side path) if replicated session changes should open/close UI.

---

## Customizing Server Behavior

`UInventoryNetComponent` owns the inventory Server RPCs. Mutation RPCs that declare `WithValidation` dispatch through `Validate*()` and `Handle*()` methods; several workflow RPCs declare only `Server, Reliable` and dispatch directly to handlers.

- **`Handle*()`** - Performs the actual inventory mutation. Override to add game-specific logic (chat messages, backend saves, lore tracking, etc.).
- **`Validate*()`** - Runs anti-cheat checks before execution. Override to add extra validation; call `Super::Validate*()` to keep built-in checks.

### Standard Behavior (No Subclass Needed)

If the default inventory behavior is sufficient, use `UInventoryNetComponent` directly:

```cpp
// In PlayerController constructor
InventoryNetComponent = CreateDefaultSubobject<UInventoryNetComponent>(TEXT("InventoryNetComponent"));
```

This supplies the plugin's default inventory mutations, session state, and RPC routing. It does **not** provide a safe world-drop implementation: both default drop handlers remove the source item/equipment and log that no world actor was spawned. It also does not drive the HUD from replicated actor references because the three default `OnRep_*` methods are empty.

### Custom Behavior (Subclass)

To add game-specific behavior, create a subclass and override only the methods you need:

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "Components/InventoryNetComponent.h"
#include "MyInventoryNetComponent.generated.h"

UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class YOURPROJECT_API UMyInventoryNetComponent : public UInventoryNetComponent
{
    GENERATED_BODY()

protected:
    // Add chat message when player loots an item
    virtual void HandlePlayerLootItem(int32 InTopLeft, EBagSlot InSlot,
                                      int32 InItemId, int32 OutTopLeft) override;

    // Spawn a world item when dropping from inventory
    virtual void HandleDropItemFromInventory(int32 TopLeft, EBagSlot Slot,
                                             FVector DropLocation) override;

    // Drive HUD when loot/merchant/repair windows open or close
    virtual void OnRep_LootedActor() override;
    virtual void OnRep_MerchantActor() override;
    virtual void OnRep_RepairerActor() override;
};
```

**Implementation pattern:**
```cpp
#include "Actors/DroppedItem.h"
#include "Interfaces/InventoryGameModeInterface.h"

void UMyInventoryNetComponent::HandlePlayerLootItem(int32 InTopLeft, EBagSlot InSlot,
                                                     int32 InItemId, int32 OutTopLeft)
{
    // Call base to perform the actual loot operation
    Super::HandlePlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);

    // Add game-specific behavior after the base operation
    NotifyGroupChat(InItemId);  // your custom logic
}

void UMyInventoryNetComponent::HandleDropItemFromInventory(int32 TopLeft, EBagSlot Slot,
                                                            FVector DropLocation)
{
    // Replace the destructive plugin default with a game-specific spawn + rollback policy.
    IInventoryPlayerInterface* PlayerInv = GetPlayerInterface();
    if (!PlayerInv)
        return;

    const int32 ItemID = PlayerInv->PlayerGetItem(TopLeft, Slot);
    if (ItemID <= 0)
        return;

    IInventoryGameModeInterface* GMI =
        Cast<IInventoryGameModeInterface>(GetWorld()->GetAuthGameMode());
    if (!GMI)
        return;

    // PlayerRemoveItem returns the stored durability. If spawning fails, restore the item
    // to its original slot rather than silently deleting it.
    const float Durability = PlayerInv->PlayerRemoveItem(TopLeft, Slot);
    ADroppedItem* Spawned = GMI->SpawnItemFromActor(GetOwner(), ItemID, DropLocation,
                                                    true, Durability);
    if (!Spawned)
    {
        PlayerInv->PlayerAddItemWithDurability(TopLeft, Slot, ItemID, Durability);
        return;
    }

    // Work around the current default GameMode implementation ignoring its Durability argument.
    Spawned->SetDurability(Durability);
}

void UMyInventoryNetComponent::OnRep_LootedActor()
{
    Super::OnRep_LootedActor();

    // Drive your HUD from the replicated state change
    // LootedActor != nullptr -> show loot window
    // LootedActor == nullptr -> hide loot window
}
```

The rollback above assumes the original slot remains available during this synchronous handler. Production projects may prefer a single game-specific helper that validates the source item, spawns and initializes the actor with durability, removes the source only after successful initialization, and destroys the actor if removal cannot be committed. Apply the same policy to `HandleDropItemFromEquipment`.

Then use your subclass in the PlayerController constructor:

```cpp
InventoryNetComponent = CreateDefaultSubobject<UMyInventoryNetComponent>(TEXT("InventoryNetComponent"));
```

### Available Handle/Validate Overrides

| Category | Handle Methods | Validate Methods |
|----------|---------------|-----------------|
| **Inventory** | `HandlePlayerMoveItem`, `HandlePlayerUnequipItem`, `HandlePlayerEquipItemFromInventory`, `HandlePlayerSwapEquipment`, `HandlePlayerAutoEquipItem`, `HandleTransferCoinTo`, `HandleDropItemFromInventory`, `HandleDropItemFromEquipment` | `ValidatePlayerMoveItem`, `ValidatePlayerUnequipItem`, `ValidatePlayerEquipItemFromInventory`, `ValidatePlayerSwapEquipment`, `ValidatePlayerAutoEquipItem`, `ValidateTransferCoinTo`, `ValidateDropItemFromInventory`, `ValidateDropItemFromEquipment` |
| **Loot** | `HandleLootActor`, `HandleStopLooting`, `HandlePlayerLootItem`, `HandlePlayerEquipItemFromLoot`, `HandlePlayerAutoLootAll` | `ValidateLootActor`, `ValidatePlayerLootItem`, `ValidatePlayerEquipItemFromLoot` |
| **Merchant** | `HandleMerchantTrade`, `HandleStopMerchantTrade`, `HandlePlayerBuyFromMerchant`, `HandlePlayerSellToMerchant` | `ValidatePlayerBuyFromMerchant`, `ValidatePlayerSellToMerchant` |
| **Repair** | `HandleRepairTrade`, `HandleStopRepairTrade`, `HandlePlayerRepairEquipment`, `HandlePlayerRepairAllEquipment` | `ValidatePlayerRepairEquipment`, `ValidatePlayerRepairAllEquipment` |
| **Staging** | `HandleCancelStagingArea`, `HandleTransferStagingToActor`, `HandleMoveEquipmentToStagingArea`, `HandleMoveInventoryItemToStagingArea` | `ValidateTransferStagingToActor`, `ValidateMoveEquipmentToStagingArea`, `ValidateMoveInventoryItemToStagingArea` |
| **Keys** | `HandlePlayerAddKeyFromInventory`, `HandlePlayerRemoveKeyToInventory` | `ValidatePlayerAddKeyFromInventory`, `ValidatePlayerRemoveKeyToInventory` |
| **Trade** | `HandlePlayerRequestTrade`, `HandlePlayerRequestTradeWithItem`, `HandlePlayerAcceptTradeRequest`, `HandlePlayerDeclineTradeRequest`, `HandlePlayerAddItemToTrade`, `HandlePlayerRemoveItemFromTrade`, `HandlePlayerSetTradeCoin`, `HandlePlayerToggleTradeAcceptance`, `HandlePlayerCancelTrade` | `ValidatePlayerRequestTrade`, `ValidatePlayerRequestTradeWithItem`, `ValidatePlayerAcceptTradeRequest`, `ValidatePlayerAddItemToTrade`, `ValidatePlayerSetTradeCoin` |
| **Replication** | `OnRep_LootedActor`, `OnRep_MerchantActor`, `OnRep_RepairerActor` | - |

### Helper Methods

`UInventoryNetComponent` provides helpers available to subclasses:

```cpp
IInventoryPlayerInterface* GetPlayerInterface() const;  // Cached owner interface (set in BeginPlay)
IEquipmentInterface*       GetEquipmentInterface() const; // Equipment from owning actor's pawn
UTradeComponent*           GetTradeComponent() const;     // Trade component from owner
```

---

## Troubleshooting

### Items Not Appearing

1. Item registered? `FetchItemFromID()` should return non-null
2. Bag initialized? `BagSet()` called with valid dimensions
3. Index valid? `TopLeftID < Width * Height`
4. Server authority? RPCs are server-only - the interface wrappers handle calling them

### Equipment Not Visible

1. `EquipmentMesh` set on the item Data Asset? (must be `USkeletalMesh`)
2. `Equipment->UpdateMasterMeshComponent(GetMesh())` called in Character `BeginPlay()` on every rendering instance (standalone, listen-server host, and clients; skip only dedicated servers)?
3. `UEquipmentComponent` replication enabled on the component?

### Replication Not Working

1. `SetReplicates(true)` on owning actor
2. `SetIsReplicated(true)` + `SetNetAddressable()` on data components
3. `UInventoryNetComponent` created via `CreateDefaultSubobject` (handles its own replication)
4. Each component's internal replicated fields are registered in that component's `GetLifetimeReplicatedProps()`; redundant component-pointer replication is not required for default subobjects
5. Mutations happen on the server
6. For owner-only state, the owning actor/component has the expected network owner

### Loot/Merchant/Repair Windows Not Opening

The C++ defaults on `IInventoryHUDInterface` dispatch through registered window instances. Check:

1. **Window registered?** If `GetLootWindow()` / `GetMerchantWindow()` / `GetRepairWindow()` return empty, `Register*Window()` was never called after widget creation. Follow [Step 5](#step-5-hud--window-registry).
2. **Widget class set?** If relying on lazy creation, override `GetLootWindowClass_Implementation()` etc. in your HUD C++ class to return a valid `TSubclassOf<UUserWidget>`.
3. **Interface implemented?** The class returned by `Get*WindowClass()` must implement the corresponding window interface (e.g. `IInventoryLootWindowInterface`). Missing interface → plugin logs a warning and skips the call.
4. **Replicated state is not driving the HUD?** The default `OnRep_LootedActor`, `OnRep_MerchantActor`, and `OnRep_RepairerActor` are no-ops. Override the relevant function in your `UInventoryNetComponent` subclass and call the HUD methods explicitly:

```cpp
void UMyInventoryNetComponent::OnRep_LootedActor()
{
    Super::OnRep_LootedActor();

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    IInventoryPlayerInterface* PI = Cast<IInventoryPlayerInterface>(PC);
    if (!PI) return;

    if (UObject* HUDObj = PI->GetInventoryHUDObject())
    {
        if (LootedActor)
            IInventoryHUDInterface::Execute_DisplayLootScreen(HUDObj, LootedActor);
        else
            IInventoryHUDInterface::Execute_HideLootScreen(HUDObj);
    }
}
```

### Bag Slot <-> Equipment Slot Mapping

Use the built-in static helpers:

```cpp
EBagSlot BagSlot = UInventoryComponent::GetBagSlotFromInventory(EquipmentSlot);
EEquipmentSlot EquipSlot = UInventoryComponent::GetInventorySlotFromBagSlot(BagSlot);
```

### Broken DataTables After Plugin Update

If `FItemContainerLine` was previously defined in your project, add to `Config/DefaultEngine.ini`:

```ini
[CoreRedirects]
+StructRedirects=(OldName="/Script/YourProject.ItemContainerLine",NewName="/Script/InventoryPlugin.ItemContainerLine")
```

---

## Summary

1. Implement **four interfaces**: `IInventoryGameInstanceInterface` (GameInstance), `IInventoryGameModeInterface` (GameMode), `IInventoryPlayerInterface` (PlayerController), `IEquipmentInterface` (Character)
2. Create **components** in constructors:
   - `UInventoryNetComponent` (or your subclass) for Server RPCs — handles its own replication
   - Data components (`UInventoryComponent`, `UCoinComponent`, etc.) with `SetNetAddressable()` + `SetIsReplicated(true)`
3. Let each replicated component register its own internal replicated fields; do not redundantly replicate default-subobject pointer properties
4. Implement **`GetInventoryNetComponent()`** in your PlayerController to return the net component
5. Delegate **`GetMerchantActor()`** and **`GetLootedActor()`** to the net component's owner-only replicated references, and **`GetTransactionBoolean()`** to its local transaction guard
6. Create **items** as Data Assets, register via DataTable in GameInstance
7. Use the pre-initialized pockets as-is or override their dimensions in `BeginPlay()`; equipment automatically manages equipped bag and quiver slots
8. Implement the **HUD window registry** in your `IInventoryHUDInterface` class: storage fields, `Register*Window` / `Get*Window` accessors, `Get*WindowClass` overrides for lazy creation, and window interface implementations on each custom widget (see [Step 5](#step-5-hud--window-registry))
9. Bind **component delegates** for data-driven UI updates
10. Optionally **subclass** `UInventoryNetComponent` to override `Handle*`/`Validate*` for game-specific behavior

For deeper details, see the linked guides above.
