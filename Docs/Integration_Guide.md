# Inventory Plugin - Complete Integration Guide

## In-Depth Documentation

For detailed explanations of mandatory plugin components, see these guides:

- **[Interface Implementation Guide](./Interface_Implementation_Guide.md)** - Deep dive into the four required interfaces (GameInstance, GameMode, PlayerController, Character)
- **[Component Architecture Guide](./Component_Architecture_Guide.md)** - Complete reference for all inventory components and their interactions
- **[Item System Guide](./Item_System_Guide.md)** - Creating, configuring, and managing items (weapons, armor, consumables, etc.)
- **[Item Architecture Best Practices](../../../Docs/ITEM_ARCHITECTURE_BEST_PRACTICES.md)** - ⭐ IMPORTANT: Understanding production vs demo item classes
- **[Replication System Guide](./Replication_System_Guide.md)** - Understanding multiplayer synchronization, RPCs, and networking patterns

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Installation](#installation)
4. [Core Concepts](#core-concepts)
5. [Step-by-Step Integration](#step-by-step-integration)
6. [Component Setup](#component-setup)
7. [Item Definition](#item-definition)
8. [UI Integration](#ui-integration)
9. [Multiplayer Considerations](#multiplayer-considerations)
10. [Testing](#testing)
11. [Troubleshooting](#troubleshooting)
12. [Advanced Features](#advanced-features)

---

## Overview

The Inventory Plugin is a fully replicated, grid-based inventory and equipment system for Unreal Engine 5. It provides a complete economic and item management solution for MMORPGs and multiplayer games.

### Core Features

- **Grid-based inventory** with variable bag sizes and item rotation
- **Full replication** for multiplayer with server authority
- **Equipment system** with multiple slots and visual attachment
- **Weight management** and encumbrance system
- **Monetary system** (Copper, Silver, Gold, Platinum)
- **Loot system** for pickable items and corpses
- **Merchant system** with buying/selling and dynamic inventory
- **Banking system** for persistent storage across sessions
- **Trading system** between players with secure transactions
- **Repair system** with NPC repairers and field repair kits
- **Durability tracking** for equipment with damage over time

### Plugin Philosophy & Design

The plugin follows a **component-based architecture** where functionality is separated into specialized, reusable components. This design allows:

1. **Modularity**: Each system (inventory, equipment, merchants) is independent
2. **Interface-driven**: All interactions go through well-defined interfaces
3. **Server authority**: All state changes happen on server, then replicate
4. **Extensibility**: Systems can be subclassed and customized in C++ or Blueprint

### Architecture Principles

**Interface Segregation**: The plugin defines multiple focused interfaces rather than one monolithic interface:
- `IInventoryGameInstanceInterface` - Item registry (GameInstance)
- `IInventoryGameModeInterface` - Item spawning (GameMode)
- `IInventoryPlayerInterface` - Player inventory access (PlayerController)
- `IEquipmentInterface` - Equipment management (Character)
- `IMerchantInterface` - Merchant transactions (Merchant NPCs)
- `IRepairInterface` - Repair services (Repair NPCs)
- `ILootableInterface` - Lootable actors (Corpses, chests)

**Component Composition**: Functionality is provided through components that can be mixed and matched:
- `UInventoryComponent` - Bag-based item storage
- `UEquipmentComponent` - Worn equipment slots
- `UCoinComponent` - Currency storage
- `UMerchantComponent` - Merchant item pools (static + dynamic)
- `URepairComponent` - Repair cost calculations
- `UBankComponent` - Persistent bank storage
- `UStagingAreaComponent` - Temporary trade/merchant storage
- `UFieldRepairComponent` - Player-performed field repairs

**Data-Driven Items**: All items are `UPrimaryDataAsset` objects configured in the editor:
- No hardcoded item definitions in code
- Easy for designers to create new items
- Hot-reloadable during development
- Items can be loaded from databases or DataTables

**Replication Strategy**: The plugin uses Unreal's built-in replication:
- Components replicate via `UPROPERTY(Replicated)`
- Arrays use `ReplicatedUsing` for delta updates
- Delegates notify UI when changes occur (both server and client)
- Minimal network traffic through efficient batching

### Key Design Decisions

1. **Server Authority**: All inventory mutations occur on server. Clients send RPCs, server validates and applies changes, then replicates to all clients. This prevents cheating and ensures consistency.

2. **Grid-Based Storage**: Items occupy grid cells (Width × Height). This provides:
   - Visual spatial organization (like Diablo, Path of Exile)
   - Natural item size differentiation (sword = 1×3, potion = 1×1)
   - Bag management gameplay (Tetris-like optimization)

3. **Component Ownership**: Inventory/equipment components live on **PlayerController** (not Character) because:
   - Persists across character respawns
   - Accessible even when character is dead
   - Simplifies save/load (one controller per player)
   - Equipment component lives on Character for visual attachment

4. **Monetary System**: Four-tier currency (Copper/Silver/Gold/Platinum) with automatic conversion:
   - Realistic medieval economy simulation
   - Prevents integer overflow for large amounts
   - Familiar to RPG players

5. **Durability as Float**: Equipment durability is 0-100 float (not integer) to allow:
   - Gradual wear over time
   - Percentage-based repairs
   - Fine-grained damage calculations

---

## Prerequisites

### Engine Requirements
- Unreal Engine 5.0 or later
- C++ project (plugin has C++ components)
- Multiplayer-enabled project (optional but recommended)

### Knowledge Requirements
- Basic C++ programming
- Understanding of Unreal's replication system
- Familiarity with interfaces and component architecture
- Basic understanding of UMG widgets

### Project Structure
Your project should have:
- A `GameMode` class (will implement `IInventoryGameModeInterface`)
- A `PlayerController` class (will implement `IInventoryPlayerInterface`)
- A `Character` class (will implement `IEquipmentInterface`)
- A `GameInstance` class (will implement `IInventoryGameInstanceInterface`)

---

## Installation

### Method 1: Git Submodule (Recommended)
```bash
cd YourProject/Plugins
git submodule add https://github.com/Synock/UE5Inventory.git InventoryPlugin
git submodule update --init --recursive
```

### Method 2: Manual Copy
1. Download the plugin from GitHub
2. Create a `Plugins` folder in your project root (if it doesn't exist)
3. Extract the plugin to `YourProject/Plugins/InventoryPlugin`

### Enable the Plugin

**Step 1: Add to your `.uproject` file**
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

**Step 2: Add to your module's `Build.cs` file**
```csharp
// YourProject.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "InventoryPlugin"  // Add this line
});
```

**Step 3: Regenerate project files**
```bash
# Windows
GenerateProjectFiles.bat

# Right-click on .uproject and select "Generate Visual Studio project files"
```

**Step 4: Compile**
- Open your project in your IDE (Visual Studio, Rider)
- Build the project
- Launch the editor

---

## Core Concepts

### Architecture Overview

```
GameInstance (IInventoryGameInstanceInterface)
    └── Item Registry (ItemID → UInventoryItemBase*)
    
GameMode (IInventoryGameModeInterface)
    ├── Item Spawning (Server Authority)
    └── Item Fetch (Central Registry)
    
PlayerController (IInventoryPlayerInterface)
    ├── InventoryComponent (bags storage)
    ├── CoinComponent (player currency)
    ├── StagingAreaComponent (temporary storage)
    └── BankComponent (persistent storage)
    
Character (IEquipmentInterface)
    └── EquipmentComponent (worn items)
```

### Key Interfaces

| Interface | Implemented By | Purpose |
|-----------|----------------|---------|
| `IInventoryGameInstanceInterface` | GameInstance | Item registry and lookup |
| `IInventoryGameModeInterface` | GameMode | Item spawning and server authority |
| `IInventoryPlayerInterface` | PlayerController | Player inventory access |
| `IEquipmentInterface` | Character | Equipment management |
| `IInventoryHUDInterface` | HUD Widget | UI updates and interactions |

### Equipment vs Inventory

- **Equipment**: Items worn by character (armor, weapons, accessories)
  - Stored in `UEquipmentComponent`
  - Attached to specific body slots (`EEquipmentSlot`)
  - Visual representation on character mesh
  
- **Inventory**: Items carried in bags
  - Stored in `UInventoryComponent`
  - Grid-based placement (`EBagSlot`)
  - Weight-limited
  - Can contain equipment items not currently worn

---

## Step-by-Step Integration

### Step 1: Implement GameInstance Interface

> **📖 For in-depth explanation of this interface, see: [Interface Implementation Guide - IInventoryGameInstanceInterface](./Interface_Implementation_Guide.md#iinventorygameinstanceinterface)**

**File: `YourGameInstance.h`**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/InventoryGameInstanceInterface.h"
#include "YourGameInstance.generated.h"

UCLASS()
class YOURPROJECT_API UYourGameInstance : public UGameInstance, public IInventoryGameInstanceInterface
{
    GENERATED_BODY()

public:
    // IInventoryGameInstanceInterface Implementation
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;
    
    // Optional: For coin icon display
    virtual UTexture2D* GetCopperCoinIconTexture() const override;
    virtual UTexture2D* GetSilverCoinIconTexture() const override;
    virtual UTexture2D* GetGoldCoinIconTexture() const override;
    virtual UTexture2D* GetPlatinumCoinIconTexture() const override;

protected:
    // Item lookup table
    UPROPERTY()
    TMap<int32, UInventoryItemBase*> ItemLUT;
    
    // Coin textures (assign in editor)
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* CopperCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* SilverCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* GoldCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* PlatinumCoinIcon;
};
```

**File: `YourGameInstance.cpp`**
```cpp
#include "YourGameInstance.h"
#include "Items/InventoryItemBase.h"

UInventoryItemBase* UYourGameInstance::FetchItemFromID(int32 ID)
{
    if (ItemLUT.Contains(ID))
    {
        return ItemLUT[ID];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Item ID %d not found in registry"), ID);
    return nullptr;
}

void UYourGameInstance::RegisterItem(UInventoryItemBase* NewItem)
{
    if (!NewItem)
    {
        UE_LOG(LogTemp, Error, TEXT("Attempted to register null item"));
        return;
    }
    
    if (NewItem->ItemID < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item %s has invalid ItemID: %d"), 
            *NewItem->Name, NewItem->ItemID);
        return;
    }
    
    if (ItemLUT.Contains(NewItem->ItemID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item ID %d already registered, overwriting"), 
            NewItem->ItemID);
    }
    
    ItemLUT.Add(NewItem->ItemID, NewItem);
    UE_LOG(LogTemp, Log, TEXT("Registered item: %s (ID: %d)"), 
        *NewItem->Name, NewItem->ItemID);
}

UTexture2D* UYourGameInstance::GetCopperCoinIconTexture() const
{
    return CopperCoinIcon;
}

UTexture2D* UYourGameInstance::GetSilverCoinIconTexture() const
{
    return SilverCoinIcon;
}

UTexture2D* UYourGameInstance::GetGoldCoinIconTexture() const
{
    return GoldCoinIcon;
}

UTexture2D* UYourGameInstance::GetPlatinumCoinIconTexture() const
{
    return PlatinumCoinIcon;
}
```

#### Item Registration from DataTable (Production Example)

For projects with many items, you can use DataTables to register items automatically. Here's my real-world implementation:

**Step 1: Use FItemContainerLine (Built into Plugin)**

The plugin provides `FItemContainerLine` struct for DataTable rows:

**File: `InventoryPlugin/Public/Items/InventoryItemBase.h`** (already included in plugin)
```cpp
USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FItemContainerLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
    UInventoryItemBase* Item = nullptr;
};
```

> **💡 Note**: This struct is built into the InventoryPlugin - you don't need to create it yourself!

**Step 2: Add DataTable Property to GameInstance**

**File: `YourGameInstance.h`**
```cpp
UCLASS()
class YOURPROJECT_API UYourGameInstance : public UGameInstance, public IInventoryGameInstanceInterface
{
    GENERATED_BODY()

public:
    // Override Init to auto-populate item registry
    virtual void Init() override;

    // IInventoryGameInstanceInterface Implementation
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;
    
    // Setup function to populate ItemLUT from DataTables and arrays
    UFUNCTION(BlueprintCallable)
    void SetupInternals();

protected:
    // Editor-editable DataTable for bulk item registration
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    UDataTable* ItemDataTable = nullptr;

    // Manual item array (for items not in DataTable)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    TArray<UInventoryItemBase*> ItemTable;

    // Runtime lookup table (populated by SetupInternals)
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TMap<int32, UInventoryItemBase*> ItemLUT;
    
    // ... other properties ...
};
```

**Step 3: Implement Automatic Registration**

**File: `YourGameInstance.cpp`**
```cpp
void UYourGameInstance::Init()
{
    Super::Init();
    
    // Populate item registry from DataTables and arrays
    SetupInternals();
}

void UYourGameInstance::SetupInternals()
{
    // Register items from DataTable (editor-configured)
    if (ItemDataTable)
    {
        for (auto& Item : ItemDataTable->GetRowMap())
        {
            FItemContainerLine* ItemRow = reinterpret_cast<FItemContainerLine*>(Item.Value);
            if (ItemRow && ItemRow->Item)
            {
                ItemLUT.Add(ItemRow->Item->ItemID, ItemRow->Item);
            }
        }
    }

    // Register items from manual array (for special items, debug items, etc.)
    for (auto& Item : ItemTable)
    {
        if (Item)
        {
            ItemLUT.Add(Item->ItemID, Item);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d items in GameInstance"), ItemLUT.Num());
}
```

**Step 4: Create DataTable in Editor**

1. In Unreal Editor, create a new DataTable asset
2. Set Row Structure to `FItemContainerLine`
3. Name it `DT_Items` (or similar)
4. Add rows for each item:
   - Row Name: `Item_Sword_Iron` (descriptive name)
   - Item: Select your UInventoryItemBase Data Asset

5. Assign the DataTable to your GameInstance Blueprint:
   - Open your GameInstance Blueprint (e.g., `BP_MainGameInstance`)
   - Set `Item Data Table` to your new DataTable
   - Items will be auto-registered on game start (no Blueprint nodes needed!)

> **💡 Best Practice**: Use DataTables for the bulk of your items (weapons, armor, consumables) and the `ItemTable` array for special cases (debug items, quest items that need code references)

**DataTable Registration Workflow:**

```
Editor (Design Time)                     Runtime (Game Start)
━━━━━━━━━━━━━━━━━━━━                    ━━━━━━━━━━━━━━━━━━━━━
                                        
1. Create Data Assets                   UYourGameInstance::Init()
   ├─ DA_Item_Sword                              │
   ├─ DA_Item_Potion                             ├─> SetupInternals()
   └─ DA_Item_Helmet                             │       │
                                                 │       ├─> Loop ItemDataTable
2. Create DataTable                              │       │   ├─ Get FItemContainerLine rows
   └─ DT_Items                                   │       │   └─ ItemLUT.Add(Item->ItemID, Item)
      (Row Structure: FItemContainerLine)        │       │
                                                 │       └─> Loop ItemTable array
3. Add Items to DataTable                        │           └─ ItemLUT.Add(Item->ItemID, Item)
   ├─ Row_Sword → DA_Item_Sword                  │
   ├─ Row_Potion → DA_Item_Potion                └─> Items ready for use
   └─ Row_Helmet → DA_Item_Helmet                    (accessible via FetchItemFromID)

4. Assign DT_Items to GameInstance
   └─ BP_MainGameInstance::ItemDataTable
```

This approach allows designers to add/modify items in the editor without touching code!

#### Troubleshooting: Broken DataTables After Plugin Update

If you had `FItemContainerLine` in your own project code before it was moved into the plugin, existing DataTables may show errors like:

```
"Struct '/Script/YourProject.ItemContainerLine' not found"
```

**Solution:** Add a CoreRedirect in `Config/DefaultEngine.ini`:

```ini
[CoreRedirects]
; Redirect old FItemContainerLine location to plugin
+StructRedirects=(OldName="/Script/YourProject.ItemContainerLine",NewName="/Script/InventoryPlugin.ItemContainerLine")
```

Replace `YourProject` with your actual module name. This automatically updates all DataTable references when the editor loads.

> **💡 Note**: After adding the redirect, restart the editor. Your DataTables will automatically fix themselves!

---

### Step 2: Implement GameMode Interface

> **📖 For in-depth explanation of server-side spawning, see: [Interface Implementation Guide - IInventoryGameModeInterface](./Interface_Implementation_Guide.md#iinventorygamemodeinterface)**

**File: `YourGameMode.h`**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InventoryGameModeInterface.h"
#include "YourGameMode.generated.h"

UCLASS()
class YOURPROJECT_API AYourGameMode : public AGameModeBase, public IInventoryGameModeInterface
{
    GENERATED_BODY()

public:
    // IInventoryGameModeInterface Implementation
    virtual ADroppedItem* SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID, 
        const FVector& DesiredDropLocation, bool ClampOnGround = true, 
        float Durability = 100.0f) override;
    
    virtual ADroppedItem* SpawnItemFromActorRaw(AActor* SpawningActor, 
        UInventoryItemBase* ItemToSpawn, float Durability = 100.0f) override;
    
    virtual ADroppedCoins* SpawnCoinsFromActor(AActor* SpawningActor, 
        const FCoinValue& CoinValue, const FVector& DesiredDropLocation, 
        bool ClampOnGround = true) override;
    
    virtual FVector GetItemSpawnLocation(AActor* SpawningActor, 
        const FVector& DesiredDropLocation, bool ClampOnGround = true) override;
    
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;

protected:
    // Optional: Track spawned items for cleanup
    UPROPERTY()
    TArray<ADroppedItem*> ActiveDroppedItems;
};
```

**File: `YourGameMode.cpp`**
```cpp
#include "YourGameMode.h"
#include "Actors/DroppedItem.h"
#include "Actors/DroppedCoins.h"
#include "Items/InventoryItemBase.h"
#include "YourGameInstance.h"
#include "Kismet/GameplayStatics.h"

ADroppedItem* AYourGameMode::SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID, 
    const FVector& DesiredDropLocation, bool ClampOnGround, float Durability)
{
    if (!SpawningActor)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: SpawningActor is null"));
        return nullptr;
    }
    
    if (ItemID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: Invalid ItemID %d"), ItemID);
        return nullptr;
    }
    
    // Get item definition from GameInstance
    UInventoryItemBase* ItemToSpawn = FetchItemFromID(ItemID);
    if (!ItemToSpawn)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: Item %d not found"), ItemID);
        return nullptr;
    }
    
    // Calculate spawn location
    FVector SpawnLocation = GetItemSpawnLocation(SpawningActor, DesiredDropLocation, ClampOnGround);
    
    // Spawn the item actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ADroppedItem* Item = GetWorld()->SpawnActor<ADroppedItem>(
        SpawnLocation, 
        SpawningActor->GetActorRotation(), 
        SpawnParams
    );
    
    if (Item)
    {
        Item->SetReplicates(true);
        Item->InitializeFromItemWithDurability(ItemToSpawn, Durability);
        ActiveDroppedItems.Add(Item);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned item %s (ID: %d) at %s"), 
            *ItemToSpawn->Name, ItemID, *SpawnLocation.ToString());
    }
    
    return Item;
}

ADroppedItem* AYourGameMode::SpawnItemFromActorRaw(AActor* SpawningActor, 
    UInventoryItemBase* ItemToSpawn, float Durability)
{
    if (!SpawningActor || !ItemToSpawn)
        return nullptr;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ADroppedItem* Item = SpawningActor->GetWorld()->SpawnActor<ADroppedItem>(
        SpawningActor->GetActorLocation(), 
        SpawningActor->GetActorRotation(),
        SpawnParams
    );
    
    if (Item)
    {
        Item->SetReplicates(true);
        Item->InitializeFromItemWithDurability(ItemToSpawn, Durability, false);
        ActiveDroppedItems.Add(Item);
    }
    
    return Item;
}

ADroppedCoins* AYourGameMode::SpawnCoinsFromActor(AActor* SpawningActor, 
    const FCoinValue& CoinValue, const FVector& DesiredDropLocation, bool ClampOnGround)
{
    if (!SpawningActor)
        return nullptr;
    
    FVector SpawnLocation = GetItemSpawnLocation(SpawningActor, DesiredDropLocation, ClampOnGround);
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ADroppedCoins* Coins = GetWorld()->SpawnActor<ADroppedCoins>(
        SpawnLocation, 
        SpawningActor->GetActorRotation(), 
        SpawnParams
    );
    
    if (Coins)
    {
        Coins->SetReplicates(true);
        Coins->InitializeFromCoinValue(CoinValue);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned coins: %d PP, %d GP, %d SP, %d CP"), 
            CoinValue.Platinum, CoinValue.Gold, CoinValue.Silver, CoinValue.Copper);
    }
    
    return Coins;
}

FVector AYourGameMode::GetItemSpawnLocation(AActor* SpawningActor, 
    const FVector& DesiredDropLocation, bool ClampOnGround)
{
    FVector SpawnLocation = DesiredDropLocation;
    
    if (ClampOnGround)
    {
        // Trace downward to find ground
        FHitResult HitResult;
        FVector TraceStart = DesiredDropLocation + FVector(0, 0, 200);
        FVector TraceEnd = DesiredDropLocation - FVector(0, 0, 5000);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(SpawningActor);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
            ECC_Visibility, QueryParams))
        {
            SpawnLocation = HitResult.Location + FVector(0, 0, 50); // Slight offset above ground
        }
    }
    
    return SpawnLocation;
}

UInventoryItemBase* AYourGameMode::FetchItemFromID(int32 ID)
{
    UYourGameInstance* GameInstance = Cast<UYourGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        return GameInstance->FetchItemFromID(ID);
    }
    
    return nullptr;
}

void AYourGameMode::RegisterItem(UInventoryItemBase* NewItem)
{
    // GameMode typically delegates to GameInstance
    UYourGameInstance* GameInstance = Cast<UYourGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        GameInstance->RegisterItem(NewItem);
    }
}
```

---

### Step 3: Implement PlayerController Interface

> **📖 For in-depth explanation of player inventory components, see: [Interface Implementation Guide - IInventoryPlayerInterface](./Interface_Implementation_Guide.md#iinventoryplayerinterface)**

**File: `YourPlayerController.h`**
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
class YOURPROJECT_API AYourPlayerController : public APlayerController, public IInventoryPlayerInterface
{
    GENERATED_BODY()

public:
    AYourPlayerController();

    // IInventoryPlayerInterface Implementation
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
    
    // Additional inventory methods
    virtual TArray<FMinimalItemStorage> GetAllItemsInBag(EBagSlot Slot) override;
    virtual FString GetInventoryOwnerName() const override;

protected:
    // Replicated inventory components
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
    
    // Transaction state
    UPROPERTY(Replicated)
    bool bInTransaction;
    
    UPROPERTY(Replicated)
    AActor* CurrentMerchantActor;
    
    UPROPERTY(Replicated)
    AActor* CurrentLootedActor;
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**File: `YourPlayerController.cpp`**
```cpp
#include "YourPlayerController.h"
#include "Components/InventoryComponent.h"
#include "Components/CoinComponent.h"
#include "Components/StagingAreaComponent.h"
#include "Components/BankComponent.h"
#include "Net/UnrealNetwork.h"

AYourPlayerController::AYourPlayerController()
{
    // Create replicated inventory components
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
    
    bInTransaction = false;
    CurrentMerchantActor = nullptr;
    CurrentLootedActor = nullptr;
}

void AYourPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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

UInventoryComponent* AYourPlayerController::GetInventoryComponent()
{
    return Inventory;
}

const UInventoryComponent* AYourPlayerController::GetInventoryComponentConst() const
{
    return Inventory;
}

UCoinComponent* AYourPlayerController::GetCoinComponent()
{
    return CoinPurse;
}

const UCoinComponent* AYourPlayerController::GetCoinComponentConst() const
{
    return CoinPurse;
}

AActor* AYourPlayerController::GetInventoryOwningActor()
{
    return GetPawn();
}

AActor const* AYourPlayerController::GetInventoryOwningActorConst() const
{
    return GetPawn();
}

bool AYourPlayerController::GetTransactionBoolean()
{
    return bInTransaction;
}

void AYourPlayerController::SetTransactionBoolean(bool Value)
{
    bInTransaction = Value;
}

AActor* AYourPlayerController::GetMerchantActor()
{
    return CurrentMerchantActor;
}

const AActor* AYourPlayerController::GetMerchantActorConst() const
{
    return CurrentMerchantActor;
}

void AYourPlayerController::SetMerchantActor(AActor* Actor)
{
    CurrentMerchantActor = Actor;
}

AActor* AYourPlayerController::GetLootedActor()
{
    return CurrentLootedActor;
}

const AActor* AYourPlayerController::GetLootedActorConst() const
{
    return CurrentLootedActor;
}

void AYourPlayerController::SetLootedActor(AActor* Actor)
{
    CurrentLootedActor = Actor;
}

TArray<FMinimalItemStorage> AYourPlayerController::GetAllItemsInBag(EBagSlot Slot)
{
    if (Inventory)
    {
        return Inventory->GetBagConst(Slot);
    }
    return TArray<FMinimalItemStorage>();
}

FString AYourPlayerController::GetInventoryOwnerName() const
{
    if (APawn* ControlledPawn = GetPawn())
    {
        return ControlledPawn->GetName();
    }
    return TEXT("Unknown");
}
```

---

### Step 4: Implement Character Equipment Interface

> **📖 For in-depth explanation of equipment management and visual attachment, see: [Interface Implementation Guide - IEquipmentInterface](./Interface_Implementation_Guide.md#iequipmentinterface)**

**File: `YourCharacter.h`**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/InventoryModularCharacterInterface.h"
#include "YourCharacter.generated.h"

class UEquipmentComponent;
class UInventoryComponent;
class UCoinComponent;

UCLASS()
class YOURPROJECT_API AYourCharacter : public ACharacter, 
    public IEquipmentInterface, 
    public IInventoryModularCharacterInterface
{
    GENERATED_BODY()

public:
    AYourCharacter(const FObjectInitializer& ObjectInitializer);

    // IEquipmentInterface Implementation
    virtual void EquipItem(EEquipmentSlot InSlot, int32 InItemId) override;
    virtual void UnequipItem(EEquipmentSlot OutSlot) override;
    virtual UEquipmentComponent* GetEquipmentComponent() override;
    virtual const UEquipmentComponent* GetEquipmentComponentConst() const override;
    
    // IInventoryModularCharacterInterface Implementation (optional)
    virtual USkeletalMeshComponent* GetMasterMeshComponent() override;

protected:
    // Equipment component (replicated)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Equipment")
    UEquipmentComponent* Equipment;
    
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**File: `YourCharacter.cpp`**
```cpp
#include "YourCharacter.h"
#include "Components/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"

AYourCharacter::AYourCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Create equipment component
    Equipment = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));
    Equipment->SetNetAddressable();
    Equipment->SetIsReplicated(true);
    
    // Link equipment to character mesh
    Equipment->UpdateMasterMeshComponent(GetMesh());
}

void AYourCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Additional initialization if needed
}

void AYourCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AYourCharacter, Equipment);
}

void AYourCharacter::EquipItem(EEquipmentSlot InSlot, int32 InItemId)
{
    if (Equipment)
    {
        // Fetch item from game instance
        UGameInstance* GI = GetGameInstance();
        if (IInventoryGameInstanceInterface* InventoryGI = Cast<IInventoryGameInstanceInterface>(GI))
        {
            UInventoryItemBase* Item = InventoryGI->FetchItemFromID(InItemId);
            if (Item)
            {
                UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(Item);
                if (EquipableItem)
                {
                    Equipment->EquipItem(EquipableItem, InSlot);
                }
            }
        }
    }
}

void AYourCharacter::UnequipItem(EEquipmentSlot OutSlot)
{
    if (Equipment)
    {
        Equipment->UnequipItem(OutSlot);
    }
}

UEquipmentComponent* AYourCharacter::GetEquipmentComponent()
{
    return Equipment;
}

const UEquipmentComponent* AYourCharacter::GetEquipmentComponentConst() const
{
    return Equipment;
}

USkeletalMeshComponent* AYourCharacter::GetMasterMeshComponent()
{
    return GetMesh();
}
```

---

## Component Setup

> **📖 For comprehensive component documentation, see:**
> - **[Component Architecture Guide](./Component_Architecture_Guide.md)** - Deep dive into all inventory components
> - **[Replication System Guide](./Replication_System_Guide.md)** - How components replicate in multiplayer

### Initialize Inventory Bags

In your PlayerController's `BeginPlay` or after possessing a character:

```cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority() && Inventory)
    {
        // Initialize default pockets (2x 3x2 bags)
        Inventory->BagSet(EBagSlot::Pocket1, true, 3, 2, EItemSize::Giant, 1.0f);
        Inventory->BagSet(EBagSlot::Pocket2, true, 3, 2, EItemSize::Giant, 1.0f);
        
        // Initialize bag slots (empty until bags are equipped)
        Inventory->BagSet(EBagSlot::WaistBag1, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::WaistBag2, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::BackPack1, false, 0, 0, EItemSize::Tiny, 1.0f);
        Inventory->BagSet(EBagSlot::BackPack2, false, 0, 0, EItemSize::Tiny, 1.0f);
        
        // Initialize quiver (optional)
        Inventory->QuiverSpecificSetup(EBagSlot::Quiver, EAmmoType::Arrows);
    }
    
    if (CoinPurse)
    {
        // Start with some money (optional)
        CoinPurse->AddCopper(100);
    }
}
```

### Equipment Bag Items

When a bag item is equipped, expand the inventory:

```cpp
void AYourCharacter::EquipItem(EEquipmentSlot InSlot, int32 InItemId)
{
    // ...existing equip logic...
    
    // If it's a bag item, expand inventory
    if (InSlot == EEquipmentSlot::WaistBag1 || InSlot == EEquipmentSlot::WaistBag2 || 
        InSlot == EEquipmentSlot::BackPack1 || InSlot == EEquipmentSlot::BackPack2)
    {
        UInventoryItemBag* BagItem = Cast<UInventoryItemBag>(Item);
        if (BagItem && Inventory)
        {
            EBagSlot CorrespondingBagSlot = ConvertEquipmentSlotToBagSlot(InSlot);
            Inventory->BagSet(
                CorrespondingBagSlot, 
                true, 
                BagItem->BagWidth, 
                BagItem->BagHeight,
                BagItem->MaximumItemSizeToContain,
                BagItem->WeightReductionFactor
            );
        }
    }
}
```

---

## Item Definition

> **📖 For comprehensive item creation documentation, see: [Item System Guide](./Item_System_Guide.md)**

### Create Item Assets

**Step 1: Create a C++ Item Class (Optional)**

If you need custom item types, inherit from `UInventoryItemBase`:

```cpp
// YourCustomItem.h
#pragma once

#include "CoreMinimal.h"
#include "Items/InventoryItemBase.h"
#include "YourCustomItem.generated.h"

UCLASS()
class YOURPROJECT_API UYourCustomItem : public UInventoryItemBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom")
    int32 CustomProperty;
};
```

**Step 2: Create Data Assets in Editor**

1. Right-click in Content Browser → **Miscellaneous** → **Data Asset**
2. Select `InventoryItemBase` (or your custom class)
3. Name it `DA_Item_YourItemName`
4. Configure properties:
   - **ItemID**: Unique integer (e.g., 1000)
   - **Name**: Display name
   - **Width/Height**: Grid size (e.g., 2x2)
   - **Icon**: UI texture
   - **Mesh**: 3D mesh for dropped item
   - **ItemSize**: Tiny/Small/Medium/Large/Giant
   - **BaseValue**: Price in copper
   - **Weight**: Encumbrance weight
   - **LoreItem**: Check if unique/quest item
   - **MagicItem**: Check if magical

**Step 3: Register Items in GameInstance**

```cpp
void UYourGameInstance::Init()
{
    Super::Init();
    
    // SetupInternals is called automatically to populate items from DataTable and arrays
    // (See detailed DataTable implementation in Step 1 above)
    SetupInternals();
    
    // OR manually register specific items:
    RegisterItem(LoadObject<UInventoryItemBase>(nullptr, TEXT("/Game/Items/DA_Item_Sword.DA_Item_Sword")));
    RegisterItem(LoadObject<UInventoryItemBase>(nullptr, TEXT("/Game/Items/DA_Item_Potion.DA_Item_Potion")));
}
```

> **💡 Recommended Approach**: Use the DataTable-based `SetupInternals()` method shown in the GameInstance implementation section above. This allows you to manage hundreds of items through the editor without hardcoding paths.

### Equipment Items

Equipment items use `UInventoryItemEquipable`:

```cpp
// Create in editor as Data Asset
- ItemID: 2001
- Name: "Iron Helmet"
- CompatibleSlots: Head (bitmask)
- ArmorClass: 15
- Durability: 100
- Mesh: SM_Helmet
- AttachmentSocket: HeadSocket
```

### Weapon Items

Weapons use `UInventoryItemWeapon`:

```cpp
// Weapon properties
- WeaponType: OneHandedSword
- DamageMin/Max: 10-15
- AttackSpeed: 2.5
- Range: 150
```

---

## UI Integration

### Create Inventory Widget

**Blueprint: `WBP_PlayerInventory`**

1. Add `UI_InventoryGrid` widgets for each bag slot
2. Add `UI_Purse` widget for currency display
3. Bind to player controller's `IInventoryPlayerInterface`

**C++ HUD Implementation:**

```cpp
void AYourPlayerController::CreateInventoryWidget()
{
    if (!InventoryWidgetClass)
        return;
    
    InventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
    
    if (InventoryWidget)
    {
        InventoryWidget->AddToViewport();
        
        // Bind to inventory component delegates
        if (Inventory)
        {
            Inventory->FullInventoryDispatcher.AddDynamic(this, &AYourPlayerController::OnInventoryChanged);
            Inventory->InventoryItemAdd.AddDynamic(this, &AYourPlayerController::OnItemAdded);
            Inventory->InventoryItemRemove.AddDynamic(this, &AYourPlayerController::OnItemRemoved);
        }
    }
}
```

### Equipment Widget

Display equipped items using `UI_EquipmentSlot` widgets for each `EEquipmentSlot`.

---

## Multiplayer Considerations

> **📖 For complete replication documentation, see: [Replication System Guide](./Replication_System_Guide.md)**

### Server Authority

**Critical: All inventory changes MUST happen on server**

```cpp
// WRONG - Client-side only
void ClientFunction()
{
    Inventory->AddItemAt(EBagSlot::Pocket1, 1000, 0);  // Won't replicate!
}

// CORRECT - Server RPC
UFUNCTION(Server, Reliable, WithValidation)
void Server_AddItem(EBagSlot Bag, int32 ItemID, int32 TopLeft);

void AYourPlayerController::Server_AddItem_Implementation(EBagSlot Bag, int32 ItemID, int32 TopLeft)
{
    if (Inventory)
    {
        Inventory->AddItemAt(Bag, ItemID, TopLeft, 100.0f);
    }
}

bool AYourPlayerController::Server_AddItem_Validate(EBagSlot Bag, int32 ItemID, int32 TopLeft)
{
    return ItemID > 0 && TopLeft >= 0;
}
```

### Replication Flow

1. **Client**: User drags item in UI
2. **Client**: Calls `Server_MoveItem(FromBag, FromSlot, ToBag, ToSlot)`
3. **Server**: Validates move, updates `InventoryComponent`
4. **Server**: Component replicates to all clients
5. **All Clients**: `OnRep_` functions fire, UI updates

### Network Optimization

- Components are already set to replicate
- Use `SetNetAddressable()` for subobject replication
- Inventory changes batch automatically via `OnRep_ReplicatedBags`

---

## Testing

### Unit Tests

```cpp
// Test item registration
void TestItemRegistry()
{
    UYourGameInstance* GI = GetGameInstance();
    
    UInventoryItemBase* Item = NewObject<UInventoryItemBase>();
    Item->ItemID = 9999;
    Item->Name = "Test Item";
    
    GI->RegisterItem(Item);
    
    UInventoryItemBase* Retrieved = GI->FetchItemFromID(9999);
    check(Retrieved == Item);
}
```

### Integration Tests

1. **Add Item**: Call `Inventory->AddItemAt()`, verify UI updates
2. **Move Item**: Drag item in UI, verify server replication
3. **Equip Item**: Equip weapon, verify mesh appears on character
4. **Drop Item**: Drop item, verify pickable actor spawns
5. **Loot Item**: Open corpse, take item, verify inventory updated
6. **Buy from Merchant**: Purchase item, verify coin deduction

### Multiplayer Tests

1. **Two Clients**: Verify inventory changes replicate
2. **Trading**: Trade items between players
3. **Looting**: Ensure only one player can loot at a time
4. **Banking**: Verify bank storage persists across sessions

---

## Troubleshooting

### Items Not Appearing in Inventory

**Problem**: Added item doesn't show in UI

**Solutions**:
1. Verify item is registered: `FetchItemFromID()` returns non-null
2. Check bag is initialized: `BagSet()` called with valid dimensions
3. Ensure `TopLeftIndex` fits in grid: `Index < (Width * Height)`
4. Verify replication: Use `HasAuthority()` check before adding
5. Check UI bindings: Widget correctly subscribed to `FullInventoryDispatcher`

### Equipment Not Visible

**Problem**: Equipped item doesn't appear on character

**Solutions**:
1. Verify mesh is set on item: `Item->Mesh` is not null
2. Check socket exists: Skeleton has socket matching `AttachmentSocket`
3. Ensure component initialized: `Equipment->UpdateMasterMeshComponent()` called
4. Verify replication: `Equipment` component is replicated
5. Check material overrides: Materials are compatible with mesh

### Replication Issues

**Problem**: Changes on server don't appear on clients

**Solutions**:
1. Verify `SetReplicates(true)` on actor
2. Check `SetIsReplicated(true)` on components
3. Ensure `SetNetAddressable()` called for subobjects
4. Use `DOREPLIFETIME` in `GetLifetimeReplicatedProps()`
5. Verify server authority: All changes happen via `Server_` RPCs

### Weight/Encumbrance

**Problem**: Can carry unlimited weight

**Solution**: Implement weight checking:

```cpp
bool AYourPlayerController::CanAddItem(UInventoryItemBase* Item)
{
    if (!Inventory || !Item)
        return false;
    
    float CurrentWeight = Inventory->GetTotalWeight();
    float MaxWeight = GetMaxCarryWeight(); // Your calculation
    
    return (CurrentWeight + Item->Weight) <= MaxWeight;
}
```

### Bag Slot Conversion

**Problem**: Need to map `EEquipmentSlot` to `EBagSlot`

**Solution**:

```cpp
EBagSlot ConvertEquipmentSlotToBagSlot(EEquipmentSlot EquipSlot)
{
    switch (EquipSlot)
    {
        case EEquipmentSlot::WaistBag1: return EBagSlot::WaistBag1;
        case EEquipmentSlot::WaistBag2: return EBagSlot::WaistBag2;
        case EEquipmentSlot::BackPack1: return EBagSlot::BackPack1;
        case EEquipmentSlot::BackPack2: return EBagSlot::BackPack2;
        case EEquipmentSlot::Ammo: return EBagSlot::Quiver;
        default: return EBagSlot::Unknown;
    }
}
```

---

## Advanced Features

### Banking System

```cpp
// Open bank
void AYourPlayerController::OpenBank()
{
    // Move items to BankComponent
    if (BankComponent)
    {
        BankComponent->LoadBankContents(); // Load from database
    }
}
```

### Merchant System

```cpp
// Setup merchant
void AMerchantNPC::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        MerchantComponent->AddStaticInventoryItem(1000, 99); // Bread, unlimited
        CashContent_Merchant->AddGold(100); // Starting cash
        MerchantComponent->SetBuyRatio(0.5f); // Buys at 50% value
        MerchantComponent->SetSellRatio(1.2f); // Sells at 120% value
    }
}
```

### Durability System

Equipment items support durability:

```cpp
// Damage equipment
void AYourCharacter::DamageEquipment(EEquipmentSlot Slot, float DamageAmount)
{
    if (Equipment)
    {
        Equipment->ApplyDurabilityLoss(Slot, DamageAmount);
        
        float CurrentDurability, MaxDurability;
        if (Equipment->GetEquipmentDurability(Slot, CurrentDurability, MaxDurability))
        {
            if (CurrentDurability <= 0)
            {
                // Item broken
                UnequipItem(Slot);
            }
        }
    }
}
```

---

## Summary

You now have a fully functional inventory system integrated into your project. Key takeaways:

1. **Four interfaces** must be implemented: GameInstance, GameMode, PlayerController, Character
2. **Server authority** is mandatory for all inventory changes
3. **Components** handle storage: `InventoryComponent`, `EquipmentComponent`, `CoinComponent`
4. **Items** are `UPrimaryDataAsset` objects registered in GameInstance
5. **UI** binds to component delegates for automatic updates
6. **Replication** is built-in but requires proper setup

For questions or issues, refer to:
- Plugin README: `Plugins/UE5Inventory/README.md`
- Example project: https://github.com/Synock/UE5PluginIntegration
