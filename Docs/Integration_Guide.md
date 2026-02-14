# Inventory Plugin - Integration Guide

## Additional Documentation

- **[Interface Implementation Guide](./Interface_Implementation_Guide.md)** - Detailed reference for all plugin interfaces
- **[Component Architecture Guide](./Component_Architecture_Guide.md)** - Complete reference for all inventory components
- **[Item System Guide](./Item_System_Guide.md)** - Creating and configuring items
- **[Replication System Guide](./Replication_System_Guide.md)** - Multiplayer synchronization and networking

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Architecture](#architecture)
4. [Step-by-Step Integration](#step-by-step-integration)
5. [Inventory Initialization](#inventory-initialization)
6. [Item Creation](#item-creation)
7. [UI Integration](#ui-integration)
8. [Multiplayer](#multiplayer)
9. [Troubleshooting](#troubleshooting)

---

## Overview

The Inventory Plugin is a fully replicated, grid-based inventory and equipment system for Unreal Engine 5.

### Features

- **Grid-based inventory** with variable bag sizes
- **Full replication** with server authority
- **Equipment system** with skeletal mesh attachment and sheath/unsheath
- **Weight management** and encumbrance
- **Currency system** (Copper, Silver, Gold, Platinum — 10x conversion per tier)
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

Right-click `.uproject` → Generate Visual Studio project files, then build.

---

## Architecture

```
GameInstance (IInventoryGameInstanceInterface)
    └── Item Registry (ItemID → UInventoryItemBase*)

GameMode (IInventoryGameModeInterface)
    ├── Item Spawning (ADroppedItem / ADroppedCoins)
    └── Item Lookup (delegates to GameInstance)

PlayerController (IInventoryPlayerInterface)
    ├── UInventoryComponent    (bag storage)
    ├── UCoinComponent         (player currency)
    ├── UStagingAreaComponent  (trade/merchant staging)
    ├── UCoinComponent         (staging coins)
    ├── UBankComponent         (bank storage)
    └── UCoinComponent         (bank coins)

Character (IEquipmentInterface + IInventoryModularCharacterInterface)
    └── UEquipmentComponent    (worn items + visual meshes)
```

All state changes are server-authoritative. Components replicate via `ReplicatedUsing` callbacks that fire delegates for UI updates.

---

## Step-by-Step Integration

> For detailed interface documentation, see the [Interface Implementation Guide](./Interface_Implementation_Guide.md).

### Step 1: GameInstance — Item Registry

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

    // Optional coin icon overrides
    virtual UTexture2D* GetCopperCoinIconTexture() const override;
    virtual UTexture2D* GetSilverCoinIconTexture() const override;
    virtual UTexture2D* GetGoldCoinIconTexture() const override;
    virtual UTexture2D* GetPlatinumCoinIconTexture() const override;

protected:
    UPROPERTY()
    TMap<int32, UInventoryItemBase*> ItemLUT;

    // Assign in the Blueprint derived from this class
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* CopperCoinIcon;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* SilverCoinIcon;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* GoldCoinIcon;
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* PlatinumCoinIcon;

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
            {
                ItemLUT.Add(ItemRow->Item->ItemID, ItemRow->Item);
            }
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

### Step 2: GameMode — Item Spawning

Your `GameMode` implements `IInventoryGameModeInterface`. The spawn methods (`SpawnItemFromActor`, `SpawnCoinsFromActor`, etc.) have **default implementations** in the interface — you only need to override `FetchItemFromID` and `RegisterItem`.

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

### Step 3: PlayerController — Inventory Components

Your `PlayerController` implements `IInventoryPlayerInterface` and creates all inventory-related components.

**Header:**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "YourPlayerController.generated.h"

class UInventoryComponent;
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

    // IInventoryPlayerInterface — required overrides
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

protected:
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

    UPROPERTY(Replicated)
    bool bInTransaction = false;
    UPROPERTY(Replicated)
    AActor* CurrentMerchantActor = nullptr;
    UPROPERTY(Replicated)
    AActor* CurrentLootedActor = nullptr;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Implementation:**
```cpp
#include "YourPlayerController.h"
#include "Components/InventoryComponent.h"
#include "Components/CoinComponent.h"
#include "Components/StagingAreaComponent.h"
#include "Components/BankComponent.h"
#include "Net/UnrealNetwork.h"

AYourPlayerController::AYourPlayerController()
{
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
    DOREPLIFETIME(AYourPlayerController, Inventory);
    DOREPLIFETIME(AYourPlayerController, CoinPurse);
    DOREPLIFETIME(AYourPlayerController, StagingAreaItems);
    DOREPLIFETIME(AYourPlayerController, StagingAreaCoin);
    DOREPLIFETIME(AYourPlayerController, BankComponent);
    DOREPLIFETIME(AYourPlayerController, BankCoin);
    DOREPLIFETIME(AYourPlayerController, bInTransaction);
    DOREPLIFETIME(AYourPlayerController, CurrentMerchantActor);
    DOREPLIFETIME(AYourPlayerController, CurrentLootedActor);
}

// Interface getters — straightforward delegation
UInventoryComponent* AYourPlayerController::GetInventoryComponent() { return Inventory; }
const UInventoryComponent* AYourPlayerController::GetInventoryComponentConst() const { return Inventory; }
UCoinComponent* AYourPlayerController::GetCoinComponent() { return CoinPurse; }
const UCoinComponent* AYourPlayerController::GetCoinComponentConst() const { return CoinPurse; }
AActor* AYourPlayerController::GetInventoryOwningActor() { return GetPawn(); }
AActor const* AYourPlayerController::GetInventoryOwningActorConst() const { return GetPawn(); }
bool AYourPlayerController::GetTransactionBoolean() { return bInTransaction; }
void AYourPlayerController::SetTransactionBoolean(bool Value) { bInTransaction = Value; }
AActor* AYourPlayerController::GetMerchantActor() { return CurrentMerchantActor; }
const AActor* AYourPlayerController::GetMerchantActorConst() const { return CurrentMerchantActor; }
void AYourPlayerController::SetMerchantActor(AActor* Actor) { CurrentMerchantActor = Actor; }
AActor* AYourPlayerController::GetLootedActor() { return CurrentLootedActor; }
const AActor* AYourPlayerController::GetLootedActorConst() const { return CurrentLootedActor; }
void AYourPlayerController::SetLootedActor(AActor* Actor) { CurrentLootedActor = Actor; }
```

---

### Step 4: Character — Equipment

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

    // IInventoryModularCharacterInterface (optional)
    virtual USkeletalMeshComponent* GetMasterMeshComponent() override;

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Equipment")
    UEquipmentComponent* Equipment;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Implementation:**
```cpp
#include "YourCharacter.h"
#include "Components/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"

AYourCharacter::AYourCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Equipment = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));
    Equipment->SetNetAddressable();
    Equipment->SetIsReplicated(true);
    Equipment->UpdateMasterMeshComponent(GetMesh());
}

void AYourCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AYourCharacter, Equipment);
}

UEquipmentComponent* AYourCharacter::GetEquipmentComponent() { return Equipment; }
const UEquipmentComponent* AYourCharacter::GetEquipmentComponentConst() const { return Equipment; }
USkeletalMeshComponent* AYourCharacter::GetMasterMeshComponent() { return GetMesh(); }
```

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

        // Bag slots start inactive — activated when bag items are equipped
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

1. Right-click in Content Browser → **Miscellaneous** → **Data Asset**
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

---

## Multiplayer

All inventory mutations must happen on the server. The `UInventoryComponent::AddItemAt()` and `RemoveItem()` methods are already declared as `Server, Reliable` RPCs.

**Replication flow:**
1. Client triggers UI action
2. Server RPC is called (e.g., `AddItemAt`)
3. Server validates and modifies component state
4. `ReplicatedUsing` callback fires on clients → delegates broadcast → UI updates

**Component setup checklist:**
- `CreateDefaultSubobject<>()` in constructor
- `SetNetAddressable()` on each component
- `SetIsReplicated(true)` on each component
- `DOREPLIFETIME()` in `GetLifetimeReplicatedProps()`

---

## Troubleshooting

### Items Not Appearing

1. Item registered? `FetchItemFromID()` should return non-null
2. Bag initialized? `BagSet()` called with valid dimensions
3. Index valid? `TopLeftID < Width * Height`
4. Server authority? `AddItemAt` is a Server RPC — call it, don't check `HasAuthority()` yourself

### Equipment Not Visible

1. `EquipmentMesh` set on the item Data Asset? (must be `USkeletalMesh`)
2. `Equipment->UpdateMasterMeshComponent(GetMesh())` called in Character constructor?
3. Component replicated? Check `DOREPLIFETIME`

### Replication Not Working

1. `SetReplicates(true)` on owning actor
2. `SetIsReplicated(true)` + `SetNetAddressable()` on component
3. `DOREPLIFETIME()` in `GetLifetimeReplicatedProps()`
4. Making changes on server, not client

### Bag Slot ↔ Equipment Slot Mapping

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
2. Create **components** in constructors with `SetNetAddressable()` + `SetIsReplicated(true)`
3. Register **`DOREPLIFETIME`** for all replicated components
4. Create **items** as Data Assets, register via DataTable in GameInstance
5. Initialize **bags** in `BeginPlay()` with `BagSet()`
6. Bind **UI widgets** to component delegates

For deeper details, see the linked guides above.
