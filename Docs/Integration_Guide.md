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

Now, a new `UInventoryNetComponent` ActorComponent owns all the Server RPCs with full default implementations and built-in anti-cheat validation. The `Server_*` methods on the interface are still present as **non-pure virtuals** that simply forward to the net component.

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

4. **Move replicated interaction state** to the net component. `TransactionBoolean`, `MerchantActor`, `LootedActor`, and `RepairerActor` now live as replicated properties on `UInventoryNetComponent`. Your interface getter/setter implementations should delegate:
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

The plugin now includes full implementations for several systems that were previously left to the consumer:

| System | Components / Classes | Notes |
|--------|---------------------|-------|
| **Player Trading** | `UTradeComponent`, Trade RPCs on `UInventoryNetComponent`, `UI_TradeWidget` | Player-to-player trade with item + coin offers |
| **NPC Repair** | `URepairComponent`, `IRepairInterface`, Repair RPCs on `UInventoryNetComponent`, `UI_RepairWidget` | NPC-based equipment repair with durability costs |
| **Field Repair** | `UFieldRepairComponent`, `IFieldRepairInterface`, `UInventoryItemFieldRepair`, `UI_FieldRepairWidget` | Self-repair using consumable repair kits |
| **Item Dropping** | `DropItemFromInventory()`, `DropItemFromEquipment()` wrappers + RPCs | Drop items from inventory or equipment into the world |

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

### 8. `IInventoryHUDInterface` — New Events

New `BlueprintImplementableEvent` methods to implement in your HUD:

- `DisplayRepairScreen(AActor*)` / `HideRepairScreen()` / `OnRepairTransactionComplete()` — for NPC repair UI
- `OpenTradeWindow()` / `CloseTradeWindow()` — for trade UI
- `DisplayFieldRepairScreen(int32, EBagSlot, int32)` / `HideFieldRepairScreen()` / `NotifyFieldRepairFinished(...)` — for field repair UI
- `DisplayItemDescriptionWithDurability(...)` — item tooltip with durability bar
- `LockInventorySlot(EBagSlot, int32, bool)` / `LockEquipmentSlot(EEquipmentSlot, bool)` — slot locking during field repair

These are `BlueprintImplementableEvent` so they won't cause compile errors, but the new UI features won't work until you implement them in your HUD Blueprint.

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
- [ ] Delegate `TransactionBoolean`, `MerchantActor`, `LootedActor` getters/setters to net component
- [ ] Add `DOREPLIFETIME` for `InventoryNetComponent` in `GetLifetimeReplicatedProps()`
- [ ] Update `PlayerRemoveItem` overrides to return `float` instead of `void`
- [ ] Update `FInventoryItemAdd` delegate bindings (now 4 params)
- [ ] Optionally subclass `UInventoryNetComponent` for game-specific Handle/Validate overrides
- [ ] Implement new HUD events in Blueprint if using repair/trade/field repair features
- [ ] Update any hardcoded `EBagSlot::LastValidBag` references

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

All state changes are server-authoritative. The `UInventoryNetComponent` owns every `UFUNCTION(Server, Reliable)` RPC for inventory operations (loot, equip, trade, merchant, repair, etc.). Data components replicate via `ReplicatedUsing` callbacks that fire delegates for UI updates.

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

Your `PlayerController` implements `IInventoryPlayerInterface` and creates all inventory-related components, including the `UInventoryNetComponent` that owns every Server RPC.

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

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UInventoryNetComponent* InventoryNetComponent;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* Inventory;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UCoinComponent* CoinPurse;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UStagingAreaComponent* StagingAreaItems;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UCoinComponent* StagingAreaCoin;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UBankComponent* BankComponent;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    UCoinComponent* BankCoin;

    UPROPERTY(BlueprintAssignable)
    FOnWeightChanged WeightDispatcher;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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
#include "Net/UnrealNetwork.h"

AYourPlayerController::AYourPlayerController()
{
    // Net component owns all Server RPCs and replicated interaction state
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
}

void AYourPlayerController::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AYourPlayerController, InventoryNetComponent);
    DOREPLIFETIME(AYourPlayerController, Inventory);
    DOREPLIFETIME(AYourPlayerController, CoinPurse);
    DOREPLIFETIME(AYourPlayerController, StagingAreaItems);
    DOREPLIFETIME(AYourPlayerController, StagingAreaCoin);
    DOREPLIFETIME(AYourPlayerController, BankComponent);
    DOREPLIFETIME(AYourPlayerController, BankCoin);
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
```

**Key points:**
- `TransactionBoolean`, `MerchantActor`, `LootedActor`, and `RepairerActor` live on `UInventoryNetComponent` as replicated properties. The interface getters delegate to the component.
- `UInventoryNetComponent` calls `SetIsReplicatedByDefault(true)` in its own constructor, so you do **not** need to call `SetNetAddressable()` or `SetIsReplicated(true)` on it.

---

### Step 4: Character - Equipment

Your `Character` implements `IEquipmentInterface` and `IInventoryModularCharacterInterface`. The equipment interface owns the `UEquipmentComponent`; the modular character interface controls how the component renders equipped items visually.

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

    // IEquipmentInterface — required
    virtual UEquipmentComponent* GetEquipmentComponent() override;
    virtual const UEquipmentComponent* GetEquipmentComponentConst() const override;

    // IInventoryModularCharacterInterface — body-part accessors (all default to nullptr)
    // Override the ones your character exposes as modular mesh components:
    virtual USkeletalMeshComponent* GetHeadComponent() override;
    // virtual USkeletalMeshComponent* GetTorsoComponent() override;
    // virtual USkeletalMeshComponent* GetArmsComponent() override;
    // ... etc. See IInventoryModularCharacterInterface for the full list.

    // IInventoryModularCharacterInterface — overlay mesh resolver
    // Called by UEquipmentComponent on every equip/unequip.
    // Return a mesh to create a dedicated overlay component for that slot,
    // or nullptr to handle the slot through your own path (e.g. merged mesh).
    // The plugin's default returns Item->EquipmentMesh for all slots.
    virtual USkeletalMesh* GetEquipmentOverlayMesh(
        EEquipmentSlot Slot, const UInventoryItemEquipable* Item) const override;

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Equipment")
    UEquipmentComponent* Equipment;

    // Called by EquipmentDispatcher / EquipmentDispatcher_Server when
    // the full equipment state must be re-evaluated (race change, skin change, etc.)
    UFUNCTION()
    void UpdateMeshFromInternal();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Implementation:**
```cpp
AYourCharacter::AYourCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Equipment = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));
    Equipment->SetNetAddressable();
    Equipment->SetIsReplicated(true);
}

void AYourCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
        Equipment->UpdateMasterMeshComponent(GetMesh());

    // Full visual rebuild whenever the server or client marks equipment as changed.
    Equipment->EquipmentDispatcher_Server.AddUniqueDynamic(
        this, &AYourCharacter::UpdateMeshFromInternal);
    Equipment->EquipmentDispatcher.AddUniqueDynamic(
        this, &AYourCharacter::UpdateMeshFromInternal);

    UpdateMeshFromInternal();
}

void AYourCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AYourCharacter, Equipment);
}

UEquipmentComponent* AYourCharacter::GetEquipmentComponent() { return Equipment; }
const UEquipmentComponent* AYourCharacter::GetEquipmentComponentConst() const { return Equipment; }
```

---

#### Implementing `GetEquipmentOverlayMesh`

The component calls this on every `EquipItem`, `RemoveItem`, and `OnRep_ItemList`. Return a `USkeletalMesh*` to have the component manage a dedicated overlay `USkeletalMeshComponent` for that slot, or `nullptr` to leave it to your `UpdateMeshFromInternal` path (e.g. merged skeletal mesh).

The **default implementation** (inherited, no override needed) returns `Item->EquipmentMesh` for every slot — suitable for simple characters that do not need race/gender correction or a merged mesh path.

For characters with a modular body system:

```cpp
USkeletalMesh* AYourCharacter::GetEquipmentOverlayMesh(
    EEquipmentSlot Slot, const UInventoryItemEquipable* Item) const
{
    if (!Item || !Item->EquipmentMesh)
        return nullptr;

    switch (Slot)
    {
    // Overlay slots: plugin creates a USkeletalMeshComponent on top of the body.
    case EEquipmentSlot::Shoulders:
    case EEquipmentSlot::Neck:
    case EEquipmentSlot::Back:
    case EEquipmentSlot::Face:
    case EEquipmentSlot::WristR:
        return GetRaceCorrectedMesh(Item->EquipmentMesh);  // your race/gender lookup

    case EEquipmentSlot::WristL:
        // Use a separate left-arm mesh asset — do NOT use negative scale.
        // SetLeaderPoseComponent binds bones by name; mirroring via scale
        // would make the left bracer animate with the wrong arm.
        return GetMirrorMesh(GetRaceCorrectedMesh(Item->EquipmentMesh));

    // These slots are handled by a merged skeletal mesh in UpdateMeshFromInternal.
    // Returning nullptr prevents the plugin from creating a duplicate overlay.
    case EEquipmentSlot::Head:   // helmet UVs must match face mesh
    case EEquipmentSlot::Torso:
    case EEquipmentSlot::Legs:
    case EEquipmentSlot::Arms:
    case EEquipmentSlot::Hands:
    default:
        return nullptr;
    }
}
```

---

#### `UpdateMeshFromInternal` and `TryUpdateDynamicMeshes`

`UpdateMeshFromInternal` is your game-side full rebuild function, bound to `EquipmentDispatcher` and `EquipmentDispatcher_Server`. It is responsible for the **merged skeletal mesh path** (slots returning `nullptr` from `GetEquipmentOverlayMesh`) and calls `TryUpdateDynamicMeshes` at the end to sync any remaining overlay slots.

For slots that return a mesh from `GetEquipmentOverlayMesh`, route them directly into `ClothMeshParts` (the map passed to `TryUpdateDynamicMeshes`) rather than the merged mesh set. A lambda helper keeps this clean:

```cpp
void AYourCharacter::UpdateMeshFromInternal()
{
    // ... build MeshParts (merged) and ClothMeshParts (overlay) ...

    // Overlay-eligible slots: query GetEquipmentOverlayMesh.
    // Non-null → overlay component; null → fall back to merged path.
    auto FillOrOverlay = [&](EEquipmentSlot Slot)
    {
        const UInventoryItemEquipable* Item = Equipment->GetItemAtSlot(Slot);
        if (USkeletalMesh* OverlayMesh = GetEquipmentOverlayMesh(Slot, Item))
        {
            ClothMeshParts.Emplace(Slot, OverlayMesh);
            if (Item)
                ClothOverride.Emplace(Slot, GetMaterialOverridesForSlot(Slot));
        }
        else
        {
            FillUpData(Slot, MeshParts, OverrideMap, ClothMeshParts, ClothOverride, ModularBody);
        }
    };

    FillOrOverlay(EEquipmentSlot::Shoulders);
    FillOrOverlay(EEquipmentSlot::Neck);
    FillOrOverlay(EEquipmentSlot::Back);
    FillOrOverlay(EEquipmentSlot::Face);
    FillOrOverlay(EEquipmentSlot::WristL);
    FillOrOverlay(EEquipmentSlot::WristR);

    // Merged slots (always):
    // FillUpData(EEquipmentSlot::Head, ...);
    // FillUpData(EEquipmentSlot::Torso, ...);
    // ...

    // Sync overlay components; also removes components for unequipped slots.
    Equipment->TryUpdateDynamicMeshes(ClothMeshParts, ClothOverride);
}
```

> **Note**: `TryUpdateDynamicMeshes` is called by `UpdateMeshFromInternal` for a full sync. The plugin also calls `UpdateSingleOverlayMesh` internally on each individual `EquipItem`/`RemoveItem`, so the overlay appears immediately without waiting for the full rebuild. Both paths are idempotent — calling them in sequence is safe.

---



## Inventory Initialization

Initialize bags in `PlayerController::BeginPlay()`:

```cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && Inventory)
    {
        // Default pockets (always available)
        Inventory->BagSet(EBagSlot::Pocket1, true, 3, 2, EItemSize::Giant, 1.0f);
        Inventory->BagSet(EBagSlot::Pocket2, true, 3, 2, EItemSize::Giant, 1.0f);

        // Bag slots start inactive - activated when bag items are equipped
        Inventory->BagSet(EBagSlot::WaistBag1, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::WaistBag2, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::BackPack1, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::BackPack2, false, 0, 0, EItemSize::Tiny, 1.0f);
    }
}
```

When a bag item is equipped, activate its corresponding inventory slot:

```cpp
// After equipping a bag item, expand the inventory
UInventoryItemBag* BagItem = Cast<UInventoryItemBag>(EquippedItem);
if (BagItem && BagItem->IsBag())
{
    EBagSlot BagSlot = UInventoryComponent::GetBagSlotFromInventory(EquipmentSlot);
    Inventory->BagSet(BagSlot, true, BagItem->GetBagWidth(), BagItem->GetBagHeight(),
                      BagItem->GetBagSize(), BagItem->GetWeightReduction());
}
```

---

## Item Creation

> For detailed item configuration, see the [Item System Guide](./Item_System_Guide.md).

1. Right-click in Content Browser -> **Miscellaneous** -> **Data Asset**
2. Select the appropriate parent class (e.g., `InventoryItemBase`, `InventoryItemEquipable`, `InventoryItemBag`)
3. Configure properties (ItemID, Name, Icon, Width, Height, etc.)
4. Add to your DataTable or `ItemTable` array in the GameInstance Blueprint

---

## UI Integration

The plugin provides ready-made UMG widgets in `Plugins/UE5Inventory/Content/UI/`. Key widgets:

| Widget | Purpose |
|--------|---------|
| `UI_BagWidget` | Single bag display |
| `UI_InventoryGrid` | Grid-based item layout |
| `UI_EquipmentSlot` | Single equipment slot |
| `UI_Purse` | Currency display |
| `UI_LootWidget` | Loot window |
| `UI_MerchantSellWidget` | Merchant buy/sell |
| `UI_BankWidget` | Bank storage |
| `UI_TradeWidget` | Player-to-player trade |
| `UI_RepairWidget` | NPC repair |

Bind to component delegates for automatic UI updates:

```cpp
// In your HUD or widget initialization
Inventory->FullInventoryDispatcher.AddDynamic(this, &UMyWidget::OnInventoryChanged);
Inventory->InventoryItemAdd.AddDynamic(this, &UMyWidget::OnItemAdded);
Equipment->EquipmentDispatcher.AddDynamic(this, &UMyWidget::OnEquipmentChanged);
CoinPurse->PurseDispatcher.AddDynamic(this, &UMyWidget::OnCoinChanged);
```

To respond to loot/merchant/repair window changes, override the `OnRep_*` methods in your `UInventoryNetComponent` subclass (see [Customizing Server Behavior](#customizing-server-behavior)) and drive your HUD from there.

---

## Multiplayer

All inventory mutations happen on the server via `UInventoryNetComponent`'s Server RPCs. Client code calls public wrapper methods on `IInventoryPlayerInterface` (e.g., `PlayerUnequipItem()`, `PlayerRequestTrade()`), which forward to the appropriate `Server_*` RPC on the net component.

**Replication flow:**
1. Client triggers UI action (e.g., drag item to equipment slot)
2. Interface wrapper calls `Server_*` RPC on `UInventoryNetComponent`
3. Server validates the request (`Validate*` virtual), then executes it (`Handle*` virtual)
4. Server modifies component state (e.g., `UInventoryComponent`, `UEquipmentComponent`)
5. `ReplicatedUsing` callback fires on clients -> delegates broadcast -> UI updates

**Component setup checklist:**
- `CreateDefaultSubobject<>()` in constructor for all components
- `SetNetAddressable()` + `SetIsReplicated(true)` on data components (`UInventoryComponent`, `UCoinComponent`, etc.)
- `UInventoryNetComponent` handles its own replication setup (`SetIsReplicatedByDefault(true)`)
- `DOREPLIFETIME()` in `GetLifetimeReplicatedProps()` for all replicated component UPROPERTYs

---

## Customizing Server Behavior

`UInventoryNetComponent` owns every `UFUNCTION(Server, Reliable, WithValidation)` RPC. Each RPC dispatches to a pair of `virtual` methods:

- **`Handle*()`** - Performs the actual inventory mutation. Override to add game-specific logic (chat messages, backend saves, lore tracking, etc.).
- **`Validate*()`** - Runs anti-cheat checks before execution. Override to add extra validation; call `Super::Validate*()` to keep built-in checks.

### Standard Behavior (No Subclass Needed)

If the default inventory behavior is sufficient, use `UInventoryNetComponent` directly:

```cpp
// In PlayerController constructor
InventoryNetComponent = CreateDefaultSubobject<UInventoryNetComponent>(TEXT("InventoryNetComponent"));
```

This gives you working loot, equip, trade, merchant, repair, staging, and key operations out of the box.

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
    // Replace the default behavior with a game-specific one
    IInventoryPlayerInterface* PlayerInv = GetPlayerInterface();
    if (!PlayerInv)
        return;

    const int32 ItemID = PlayerInv->PlayerGetItem(TopLeft, Slot);
    PlayerInv->PlayerRemoveItem(TopLeft, Slot);

    // Spawn dropped item via GameMode
    if (auto* GMI = Cast<IInventoryGameModeInterface>(GetWorld()->GetAuthGameMode()))
        GMI->SpawnItemFromActor(GetOwner(), ItemID, DropLocation);
}

void UMyInventoryNetComponent::OnRep_LootedActor()
{
    // Drive your HUD from the replicated state change
    // LootedActor != nullptr -> show loot window
    // LootedActor == nullptr -> hide loot window
}
```

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
2. `Equipment->UpdateMasterMeshComponent(GetMesh())` called in Character `BeginPlay()` (client-side, i.e. inside a `!HasAuthority()` block)?
3. Component replicated? Check `DOREPLIFETIME`

### Replication Not Working

1. `SetReplicates(true)` on owning actor
2. `SetIsReplicated(true)` + `SetNetAddressable()` on data components
3. `UInventoryNetComponent` created via `CreateDefaultSubobject` (handles its own replication)
4. `DOREPLIFETIME()` in `GetLifetimeReplicatedProps()` for all replicated UPROPERTYs
5. Making changes on server, not client

### Loot/Merchant/Repair Windows Not Opening

The `OnRep_LootedActor`, `OnRep_MerchantActor`, and `OnRep_RepairerActor` callbacks on `UInventoryNetComponent` are empty by default. Override them in your subclass to drive your HUD:

```cpp
void UMyInventoryNetComponent::OnRep_LootedActor()
{
    if (LootedActor)
        ShowLootWindow(LootedActor);
    else
        HideLootWindow();
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
   - `UInventoryNetComponent` (or your subclass) for Server RPCs - handles its own replication
   - Data components (`UInventoryComponent`, `UCoinComponent`, etc.) with `SetNetAddressable()` + `SetIsReplicated(true)`
3. Register **`DOREPLIFETIME`** for all replicated component UPROPERTYs
4. Implement **`GetInventoryNetComponent()`** in your PlayerController to return the net component
5. Delegate **`GetMerchantActor()`**, **`GetLootedActor()`**, **`GetTransactionBoolean()`** to the net component's replicated properties
6. Create **items** as Data Assets, register via DataTable in GameInstance
7. Initialize **bags** in `BeginPlay()` with `BagSet()`
8. Bind **UI widgets** to component delegates
9. Optionally **subclass** `UInventoryNetComponent` to override `Handle*`/`Validate*` for game-specific behavior

For deeper details, see the linked guides above.
