# Interface Implementation Guide

## Overview

The InventoryPlugin requires four mandatory interfaces to be implemented in your project. This document provides in-depth explanations of each interface, their responsibilities, and implementation patterns.

## Table of Contents

1. [Interface Overview](#interface-overview)
2. [IInventoryGameInstanceInterface](#iinventorygameinstanceinterface)
3. [IInventoryGameModeInterface](#iinventorygamemodeinterface)
4. [IInventoryPlayerInterface](#iinventoryplayerinterface)
5. [IEquipmentInterface](#iequipmentinterface)
6. [Common Implementation Patterns](#common-implementation-patterns)
7. [Validation and Testing](#validation-and-testing)

---

## Interface Overview

### Architecture Pattern

The plugin uses **Interface Segregation Principle** to separate concerns:

```
┌─────────────────────────────────────────────────────────────┐
│                     Game Architecture                        │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  GameInstance (IInventoryGameInstanceInterface)              │
│      └─► Item Registry (ItemID → Item Lookup)               │
│                                                               │
│  GameMode (IInventoryGameModeInterface)                      │
│      └─► Item Spawning (Server Authority)                   │
│                                                               │
│  PlayerController (IInventoryPlayerInterface)                │
│      ├─► InventoryComponent                                  │
│      ├─► CoinComponent                                       │
│      ├─► StagingAreaComponent                               │
│      └─► BankComponent                                       │
│                                                               │
│  Character (IEquipmentInterface)                             │
│      └─► EquipmentComponent                                  │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Why Interfaces?

**1. Decoupling**: Plugin code doesn't need to know your specific class hierarchy
**2. Flexibility**: You can implement interfaces on any class (multiple inheritance in C++)
**3. Testability**: Easy to create mock implementations for testing
**4. Modularity**: Each interface has a single, well-defined responsibility

---

## IInventoryGameInstanceInterface

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Interfaces/InventoryGameInstanceInterface.h`

### Responsibility

The **Item Registry** - maintains a global lookup table mapping `ItemID` (int32) to item definitions (`UInventoryItemBase*`).

### Why GameInstance?

- **Persistent across levels**: GameInstance survives level transitions
- **Single source of truth**: Only one registry for the entire game
- **Accessible everywhere**: `UGameplayStatics::GetGameInstance()` available globally
- **Pre-loads items**: Can initialize registry at game startup

### Interface Definition

```cpp
class IInventoryGameInstanceInterface
{
    GENERATED_BODY()

public:
    // Fetch an item by its unique ID
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) = 0;
    
    // Register a new item in the global registry
    virtual void RegisterItem(UInventoryItemBase* NewItem) = 0;
    
    // Coin icon accessors (optional)
    virtual UTexture2D* GetCopperCoinIconTexture() const = 0;
    virtual UTexture2D* GetSilverCoinIconTexture() const = 0;
    virtual UTexture2D* GetGoldCoinIconTexture() const = 0;
    virtual UTexture2D* GetPlatinumCoinIconTexture() const = 0;
};
```

### Detailed Implementation

#### Header File

```cpp
// YourGameInstance.h
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
    UYourGameInstance();
    
    // UGameInstance overrides
    virtual void Init() override;
    virtual void Shutdown() override;

    // IInventoryGameInstanceInterface Implementation
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;
    
    virtual UTexture2D* GetCopperCoinIconTexture() const override;
    virtual UTexture2D* GetSilverCoinIconTexture() const override;
    virtual UTexture2D* GetGoldCoinIconTexture() const override;
    virtual UTexture2D* GetPlatinumCoinIconTexture() const override;

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void LoadItemsFromDataTable(UDataTable* ItemTable);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void LoadItemsFromDirectory(const FString& DirectoryPath);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TArray<int32> GetAllRegisteredItemIDs() const;
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetRegisteredItemCount() const;

protected:
    // Core registry: ItemID → Item mapping
    UPROPERTY()
    TMap<int32, UInventoryItemBase*> ItemLUT;
    
    // Coin display icons (configured in editor)
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* CopperCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* SilverCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* GoldCoinIcon;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Currency")
    UTexture2D* PlatinumCoinIcon;
    
    // Optional: DataTable reference for automatic loading
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Items")
    UDataTable* DefaultItemTable;
    
private:
    void InitializeDefaultItems();
    void ValidateItemRegistry();
};
```

#### Implementation File

```cpp
// YourGameInstance.cpp
#include "YourGameInstance.h"
#include "Items/InventoryItemBase.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"

UYourGameInstance::UYourGameInstance()
{
    CopperCoinIcon = nullptr;
    SilverCoinIcon = nullptr;
    GoldCoinIcon = nullptr;
    PlatinumCoinIcon = nullptr;
    DefaultItemTable = nullptr;
}

void UYourGameInstance::Init()
{
    Super::Init();
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Inventory System..."));
    
    // Initialize default items
    InitializeDefaultItems();
    
    // Load from DataTable if configured
    if (DefaultItemTable)
    {
        LoadItemsFromDataTable(DefaultItemTable);
    }
    
    // Validate registry
    ValidateItemRegistry();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory System initialized with %d items"), 
        ItemLUT.Num());
}

void UYourGameInstance::Shutdown()
{
    // Clean up registry
    ItemLUT.Empty();
    
    Super::Shutdown();
}

UInventoryItemBase* UYourGameInstance::FetchItemFromID(int32 ID)
{
    // Fast lookup - O(1) hash table access
    if (UInventoryItemBase** FoundItem = ItemLUT.Find(ID))
    {
        return *FoundItem;
    }
    
    // Item not found - log warning
    UE_LOG(LogTemp, Warning, TEXT("FetchItemFromID: Item ID %d not registered"), ID);
    return nullptr;
}

void UYourGameInstance::RegisterItem(UInventoryItemBase* NewItem)
{
    // Validation checks
    if (!NewItem)
    {
        UE_LOG(LogTemp, Error, TEXT("RegisterItem: Attempted to register null item"));
        return;
    }
    
    if (NewItem->ItemID <= 0)
    {
        UE_LOG(LogTemp, Warning, 
            TEXT("RegisterItem: Item '%s' has invalid ItemID: %d"), 
            *NewItem->Name, NewItem->ItemID);
        return;
    }
    
    // Check for duplicate IDs
    if (ItemLUT.Contains(NewItem->ItemID))
    {
        UInventoryItemBase* ExistingItem = ItemLUT[NewItem->ItemID];
        UE_LOG(LogTemp, Warning, 
            TEXT("RegisterItem: ItemID %d collision! Replacing '%s' with '%s'"),
            NewItem->ItemID, 
            ExistingItem ? *ExistingItem->Name : TEXT("Unknown"),
            *NewItem->Name);
    }
    
    // Register item
    ItemLUT.Add(NewItem->ItemID, NewItem);
    
    UE_LOG(LogTemp, Verbose, 
        TEXT("Registered item: '%s' (ID: %d, Type: %s)"), 
        *NewItem->Name, 
        NewItem->ItemID,
        *NewItem->GetClass()->GetName());
}

void UYourGameInstance::LoadItemsFromDataTable(UDataTable* ItemTable)
{
    if (!ItemTable)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadItemsFromDataTable: ItemTable is null"));
        return;
    }
    
    // DataTable row structure should contain UInventoryItemBase* field
    TArray<FName> RowNames = ItemTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        // Assuming your DataTable has a struct with Item field
        // Adjust based on your actual DataTable structure
        uint8* RowData = ItemTable->FindRowUnchecked(RowName);
        if (RowData)
        {
            // Parse your row structure here
            // Example: FItemTableRow* Row = (FItemTableRow*)RowData;
            // RegisterItem(Row->Item);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d items from DataTable '%s'"), 
        RowNames.Num(), *ItemTable->GetName());
}

void UYourGameInstance::LoadItemsFromDirectory(const FString& DirectoryPath)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    TArray<FAssetData> AssetData;
    FARFilter Filter;
    Filter.PackagePaths.Add(*DirectoryPath);
    Filter.ClassPaths.Add(UInventoryItemBase::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;
    
    AssetRegistry.GetAssets(Filter, AssetData);
    
    for (const FAssetData& Asset : AssetData)
    {
        UInventoryItemBase* Item = Cast<UInventoryItemBase>(Asset.GetAsset());
        if (Item)
        {
            RegisterItem(Item);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d items from directory '%s'"), 
        AssetData.Num(), *DirectoryPath);
}

TArray<int32> UYourGameInstance::GetAllRegisteredItemIDs() const
{
    TArray<int32> ItemIDs;
    ItemLUT.GetKeys(ItemIDs);
    return ItemIDs;
}

int32 UYourGameInstance::GetRegisteredItemCount() const
{
    return ItemLUT.Num();
}

void UYourGameInstance::InitializeDefaultItems()
{
    // Load hardcoded essential items if needed
    // Example: Load from specific asset paths
    
    // Load copper coin icon (example)
    if (!CopperCoinIcon)
    {
        CopperCoinIcon = LoadObject<UTexture2D>(nullptr, 
            TEXT("/InventoryPlugin/Icons/CopperPiece.CopperPiece"));
    }
    
    // Load other coin icons
    if (!SilverCoinIcon)
    {
        SilverCoinIcon = LoadObject<UTexture2D>(nullptr, 
            TEXT("/InventoryPlugin/Icons/SilverPiece.SilverPiece"));
    }
    
    if (!GoldCoinIcon)
    {
        GoldCoinIcon = LoadObject<UTexture2D>(nullptr, 
            TEXT("/InventoryPlugin/Icons/GoldPiece.GoldPiece"));
    }
    
    if (!PlatinumCoinIcon)
    {
        PlatinumCoinIcon = LoadObject<UTexture2D>(nullptr, 
            TEXT("/InventoryPlugin/Icons/PlatinumPiece.PlatinumPiece"));
    }
}

void UYourGameInstance::ValidateItemRegistry()
{
    // Check for ItemID conflicts
    TSet<int32> UsedIDs;
    TArray<int32> DuplicateIDs;
    
    for (const auto& Pair : ItemLUT)
    {
        if (UsedIDs.Contains(Pair.Key))
        {
            DuplicateIDs.Add(Pair.Key);
        }
        UsedIDs.Add(Pair.Key);
    }
    
    if (DuplicateIDs.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Found %d duplicate ItemIDs!"), DuplicateIDs.Num());
        for (int32 ID : DuplicateIDs)
        {
            UE_LOG(LogTemp, Error, TEXT("  - Duplicate ItemID: %d"), ID);
        }
    }
    
    // Validate item data integrity
    for (const auto& Pair : ItemLUT)
    {
        UInventoryItemBase* Item = Pair.Value;
        if (!Item)
        {
            UE_LOG(LogTemp, Error, TEXT("Registry contains null item at ID %d"), Pair.Key);
            continue;
        }
        
        // Check for missing icons
        if (!Item->Icon)
        {
            UE_LOG(LogTemp, Warning, TEXT("Item '%s' (ID: %d) missing icon"), 
                *Item->Name, Item->ItemID);
        }
        
        // Check for invalid dimensions
        if (Item->Width <= 0 || Item->Height <= 0)
        {
            UE_LOG(LogTemp, Error, 
                TEXT("Item '%s' (ID: %d) has invalid dimensions: %dx%d"),
                *Item->Name, Item->ItemID, Item->Width, Item->Height);
        }
    }
}

// Coin icon getters
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

### Best Practices

**✅ DO:**
- Register all items during `Init()` before gameplay starts
- Use unique ItemIDs across your entire game (consider ID ranges by category)
- Validate items after registration (check for missing icons, invalid dimensions)
- Use `const` accessors when reading from registry
- Log registration events for debugging

**❌ DON'T:**
- Register items during gameplay (performance impact)
- Use negative or zero ItemIDs
- Register duplicate ItemIDs without warning
- Register null items
- Modify items after registration (items should be immutable)

### Testing

```cpp
// Test case: Item registration and retrieval
void TestItemRegistry()
{
    UYourGameInstance* GI = Cast<UYourGameInstance>(
        UGameplayStatics::GetGameInstance(GetWorld()));
    
    // Create test item
    UInventoryItemBase* TestItem = NewObject<UInventoryItemBase>();
    TestItem->ItemID = 99999;
    TestItem->Name = TEXT("Test Item");
    TestItem->Width = 1;
    TestItem->Height = 1;
    
    // Register
    GI->RegisterItem(TestItem);
    
    // Retrieve
    UInventoryItemBase* Retrieved = GI->FetchItemFromID(99999);
    check(Retrieved == TestItem);
    check(Retrieved->Name == TEXT("Test Item"));
    
    UE_LOG(LogTemp, Log, TEXT("Item registry test passed"));
}
```

[Continue to IInventoryGameModeInterface →](./Interface_Implementation_Guide.md#iinventorygamemodeinterface)

---

## IInventoryGameModeInterface

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Interfaces/InventoryGameModeInterface.h`

### Responsibility

**Server-side Item Spawning** - handles spawning physical items in the world with server authority.

### Why GameMode?

- **Server-only class**: GameMode only exists on server (never on clients)
- **Server authority**: All spawning must be authoritative to prevent cheating
- **World access**: Has direct access to `UWorld` for actor spawning
- **Centralized spawning**: Single point of control for all loot/item drops

### Interface Definition

```cpp
class IInventoryGameModeInterface
{
    GENERATED_BODY()

public:
    // Spawn a dropped item from ItemID
    virtual ADroppedItem* SpawnItemFromActor(
        AActor* SpawningActor, 
        uint32 ItemID, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true, 
        float Durability = 100.0f
    ) = 0;
    
    // Spawn a dropped item from item object directly
    virtual ADroppedItem* SpawnItemFromActorRaw(
        AActor* SpawningActor, 
        UInventoryItemBase* ItemToSpawn, 
        float Durability = 100.0f
    ) = 0;
    
    // Spawn dropped coins
    virtual ADroppedCoins* SpawnCoinsFromActor(
        AActor* SpawningActor, 
        const FCoinValue& CoinValue, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true
    ) = 0;
    
    // Calculate safe spawn location
    virtual FVector GetItemSpawnLocation(
        AActor* SpawningActor, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true
    ) = 0;
    
    // Delegate to GameInstance for item lookup
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) = 0;
    virtual void RegisterItem(UInventoryItemBase* NewItem) = 0;
};
```

### Detailed Implementation

#### Header File

```cpp
// YourGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InventoryGameModeInterface.h"
#include "YourGameMode.generated.h"

class ADroppedItem;
class ADroppedCoins;
class UInventoryItemBase;
struct FCoinValue;

UCLASS()
class YOURPROJECT_API AYourGameMode : public AGameModeBase, 
    public IInventoryGameModeInterface
{
    GENERATED_BODY()

public:
    AYourGameMode();

    // IInventoryGameModeInterface Implementation
    virtual ADroppedItem* SpawnItemFromActor(
        AActor* SpawningActor, 
        uint32 ItemID, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true, 
        float Durability = 100.0f
    ) override;
    
    virtual ADroppedItem* SpawnItemFromActorRaw(
        AActor* SpawningActor, 
        UInventoryItemBase* ItemToSpawn, 
        float Durability = 100.0f
    ) override;
    
    virtual ADroppedCoins* SpawnCoinsFromActor(
        AActor* SpawningActor, 
        const FCoinValue& CoinValue, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true
    ) override;
    
    virtual FVector GetItemSpawnLocation(
        AActor* SpawningActor, 
        const FVector& DesiredDropLocation, 
        bool ClampOnGround = true
    ) override;
    
    virtual UInventoryItemBase* FetchItemFromID(int32 ID) override;
    virtual void RegisterItem(UInventoryItemBase* NewItem) override;

    // Cleanup helpers
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CleanupOldDroppedItems(float MaxAgeSeconds = 300.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetActiveDroppedItemCount() const;

protected:
    // Track spawned items for cleanup
    UPROPERTY()
    TArray<ADroppedItem*> ActiveDroppedItems;
    
    UPROPERTY()
    TArray<ADroppedCoins*> ActiveDroppedCoins;
    
    // Spawn settings
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Spawning")
    float ItemSpawnHeightOffset = 50.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Spawning")
    float MaxItemLifetimeSeconds = 300.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Spawning")
    bool bAutoCleanupOldItems = true;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Spawning")
    float CleanupCheckInterval = 30.0f;
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    FTimerHandle CleanupTimerHandle;
    void PeriodicCleanup();
};
```

#### Implementation File

```cpp
// YourGameMode.cpp
#include "YourGameMode.h"
#include "Actors/PickableItem.h"
#include "Actors/PickableCoins.h"
#include "Items/InventoryItemBase.h"
#include "YourGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AYourGameMode::AYourGameMode()
{
    ItemSpawnHeightOffset = 50.0f;
    MaxItemLifetimeSeconds = 300.0f; // 5 minutes
    bAutoCleanupOldItems = true;
    CleanupCheckInterval = 30.0f; // 30 seconds
}

void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Start periodic cleanup timer
    if (bAutoCleanupOldItems)
    {
        GetWorldTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &AYourGameMode::PeriodicCleanup,
            CleanupCheckInterval,
            true // Loop
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("GameMode: Inventory spawning system initialized"));
}

void AYourGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear cleanup timer
    if (CleanupTimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    // Cleanup all dropped items
    for (ADroppedItem* Item : ActiveDroppedItems)
    {
        if (Item && !Item->IsPendingKill())
        {
            Item->Destroy();
        }
    }
    ActiveDroppedItems.Empty();
    
    for (ADroppedCoins* Coins : ActiveDroppedCoins)
    {
        if (Coins && !Coins->IsPendingKill())
        {
            Coins->Destroy();
        }
    }
    ActiveDroppedCoins.Empty();
    
    Super::EndPlay(EndPlayReason);
}

ADroppedItem* AYourGameMode::SpawnItemFromActor(
    AActor* SpawningActor, 
    uint32 ItemID, 
    const FVector& DesiredDropLocation, 
    bool ClampOnGround, 
    float Durability)
{
    // Authority check
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: Must be called on server"));
        return nullptr;
    }
    
    // Validate spawning actor
    if (!SpawningActor)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: SpawningActor is null"));
        return nullptr;
    }
    
    // Validate ItemID
    if (ItemID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: Invalid ItemID %d"), ItemID);
        return nullptr;
    }
    
    // Fetch item definition
    UInventoryItemBase* ItemToSpawn = FetchItemFromID(ItemID);
    if (!ItemToSpawn)
    {
        UE_LOG(LogTemp, Error, 
            TEXT("SpawnItemFromActor: Item %d not found in registry"), ItemID);
        return nullptr;
    }
    
    // Calculate spawn location
    FVector SpawnLocation = GetItemSpawnLocation(
        SpawningActor, 
        DesiredDropLocation, 
        ClampOnGround
    );
    
    // Setup spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Cast<APawn>(SpawningActor);
    SpawnParams.SpawnCollisionHandlingOverride = 
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // Spawn the pickable item actor
    APickableItem* DroppedItem = GetWorld()->SpawnActor<APickableItem>(
        APickableItem::StaticClass(),
        SpawnLocation,
        SpawningActor->GetActorRotation(),
        SpawnParams
    );
    
    if (!DroppedItem)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActor: Failed to spawn actor"));
        return nullptr;
    }
    
    // Configure item
    DroppedItem->SetReplicates(true);
    DroppedItem->InitializeFromItemWithDurability(ItemToSpawn, Durability);
    
    // Track for cleanup
    ActiveDroppedItems.Add(DroppedItem);
    
    UE_LOG(LogTemp, Log, 
        TEXT("Spawned item '%s' (ID: %d, Durability: %.1f) at %s"), 
        *ItemToSpawn->Name, 
        ItemID, 
        Durability,
        *SpawnLocation.ToString());
    
    // Debug visualization (only in development builds)
#if !UE_BUILD_SHIPPING
    if (CVarShowItemSpawns->GetBool())
    {
        DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 12, FColor::Green, false, 5.0f);
    }
#endif
    
    return DroppedItem;
}

ADroppedItem* AYourGameMode::SpawnItemFromActorRaw(
    AActor* SpawningActor, 
    UInventoryItemBase* ItemToSpawn, 
    float Durability)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnItemFromActorRaw: Must be called on server"));
        return nullptr;
    }
    
    if (!SpawningActor || !ItemToSpawn)
    {
        UE_LOG(LogTemp, Error, 
            TEXT("SpawnItemFromActorRaw: Invalid parameters (Actor: %s, Item: %s)"),
            SpawningActor ? TEXT("Valid") : TEXT("Null"),
            ItemToSpawn ? TEXT("Valid") : TEXT("Null"));
        return nullptr;
    }
    
    // Use actor's location as spawn point
    FVector SpawnLocation = SpawningActor->GetActorLocation();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Cast<APawn>(SpawningActor);
    SpawnParams.SpawnCollisionHandlingOverride = 
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ADroppedItem* DroppedItem = GetWorld()->SpawnActor<ADroppedItem>(
        ADroppedItem::StaticClass(),
        SpawnLocation,
        SpawningActor->GetActorRotation(),
        SpawnParams
    );
    
    if (DroppedItem)
    {
        DroppedItem->SetReplicates(true);
        DroppedItem->InitializeFromItemWithDurability(ItemToSpawn, Durability, false);
        ActiveDroppedItems.Add(DroppedItem);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned raw item '%s' (Durability: %.1f)"), 
            *ItemToSpawn->Name, Durability);
    }
    
    return DroppedItem;
}

ADroppedCoins* AYourGameMode::SpawnCoinsFromActor(
    AActor* SpawningActor, 
    const FCoinValue& CoinValue, 
    const FVector& DesiredDropLocation, 
    bool ClampOnGround)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCoinsFromActor: Must be called on server"));
        return nullptr;
    }
    
    if (!SpawningActor)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCoinsFromActor: SpawningActor is null"));
        return nullptr;
    }
    
    // Don't spawn if coin value is zero
    if (CoinValue.GetTotalCopper() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnCoinsFromActor: CoinValue is zero or negative"));
        return nullptr;
    }
    
    // Calculate spawn location
    FVector SpawnLocation = GetItemSpawnLocation(
        SpawningActor, 
        DesiredDropLocation, 
        ClampOnGround
    );
    
    // Setup spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Cast<APawn>(SpawningActor);
    SpawnParams.SpawnCollisionHandlingOverride = 
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // Spawn coins
    APickableCoins* Coins = GetWorld()->SpawnActor<APickableCoins>(
        APickableCoins::StaticClass(),
        SpawnLocation,
        SpawningActor->GetActorRotation(),
        SpawnParams
    );
    
    if (!Coins)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCoinsFromActor: Failed to spawn coin actor"));
        return nullptr;
    }
    
    // Configure coins
    Coins->SetReplicates(true);
    Coins->InitializeFromCoinValue(CoinValue);
    
    // Track for cleanup
    ActiveDroppedCoins.Add(Coins);
    
    UE_LOG(LogTemp, Log, 
        TEXT("Spawned coins: %dpp %dgp %dsp %dcp (Total: %d copper) at %s"),
        CoinValue.Platinum, CoinValue.Gold, CoinValue.Silver, CoinValue.Copper,
        CoinValue.GetTotalCopper(),
        *SpawnLocation.ToString());
    
    return Coins;
}

FVector AYourGameMode::GetItemSpawnLocation(
    AActor* SpawningActor, 
    const FVector& DesiredDropLocation, 
    bool ClampOnGround)
{
    if (!SpawningActor)
    {
        return DesiredDropLocation;
    }
    
    FVector SpawnLocation = DesiredDropLocation;
    
    if (ClampOnGround)
    {
        // Trace downward to find ground
        FHitResult HitResult;
        FVector TraceStart = DesiredDropLocation + FVector(0, 0, 500.0f);
        FVector TraceEnd = DesiredDropLocation - FVector(0, 0, 5000.0f);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(SpawningActor);
        QueryParams.bTraceComplex = false;
        
        if (GetWorld()->LineTraceSingleByChannel(
            HitResult, 
            TraceStart, 
            TraceEnd, 
            ECC_Visibility, 
            QueryParams))
        {
            // Found ground, place item slightly above it
            SpawnLocation = HitResult.Location + FVector(0, 0, ItemSpawnHeightOffset);
            
            UE_LOG(LogTemp, Verbose, 
                TEXT("Ground found at Z=%.1f, spawning at Z=%.1f"),
                HitResult.Location.Z, SpawnLocation.Z);
        }
        else
        {
            // No ground found, use desired location
            UE_LOG(LogTemp, Warning, 
                TEXT("No ground found below %s, using desired location"),
                *DesiredDropLocation.ToString());
        }
    }
    
    return SpawnLocation;
}

UInventoryItemBase* AYourGameMode::FetchItemFromID(int32 ID)
{
    // Delegate to GameInstance
    UYourGameInstance* GI = Cast<UYourGameInstance>(GetGameInstance());
    if (GI)
    {
        return GI->FetchItemFromID(ID);
    }
    
    UE_LOG(LogTemp, Error, TEXT("FetchItemFromID: GameInstance is invalid"));
    return nullptr;
}

void AYourGameMode::RegisterItem(UInventoryItemBase* NewItem)
{
    // Delegate to GameInstance
    UYourGameInstance* GI = Cast<UYourGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->RegisterItem(NewItem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RegisterItem: GameInstance is invalid"));
    }
}

void AYourGameMode::CleanupOldDroppedItems(float MaxAgeSeconds)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 CleanedUpCount = 0;
    
    // Cleanup items
    for (int32 i = ActiveDroppedItems.Num() - 1; i >= 0; --i)
    {
        ADroppedItem* Item = ActiveDroppedItems[i];
        
        if (!Item || Item->IsPendingKill())
        {
            ActiveDroppedItems.RemoveAtSwap(i);
            continue;
        }
        
        float ItemAge = CurrentTime - Item->GetSpawnTime();
        if (ItemAge >= MaxAgeSeconds)
        {
            Item->Destroy();
            ActiveDroppedItems.RemoveAtSwap(i);
            CleanedUpCount++;
        }
    }
    
    // Cleanup coins
    for (int32 i = ActiveDroppedCoins.Num() - 1; i >= 0; --i)
    {
        ADroppedCoins* Coins = ActiveDroppedCoins[i];
        
        if (!Coins || Coins->IsPendingKill())
        {
            ActiveDroppedCoins.RemoveAtSwap(i);
            continue;
        }
        
        float CoinAge = CurrentTime - Coins->GetSpawnTime();
        if (CoinAge >= MaxAgeSeconds)
        {
            Coins->Destroy();
            ActiveDroppedCoins.RemoveAtSwap(i);
            CleanedUpCount++;
        }
    }
    
    if (CleanedUpCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d old dropped items"), CleanedUpCount);
    }
}

int32 AYourGameMode::GetActiveDroppedItemCount() const
{
    return ActiveDroppedItems.Num() + ActiveDroppedCoins.Num();
}

void AYourGameMode::PeriodicCleanup()
{
    CleanupOldDroppedItems(MaxItemLifetimeSeconds);
}
```

### Best Practices

**✅ DO:**
- Always check `HasAuthority()` before spawning
- Validate all input parameters (Actor, ItemID, Location)
- Track spawned items for cleanup
- Use `ClampOnGround` for outdoor spawns
- Log spawn events for debugging
- Set appropriate lifetime for items

**❌ DON'T:**
- Call spawn functions on client
- Spawn items with invalid ItemIDs
- Spawn items without replication enabled
- Forget to clean up old items (memory leak)
- Spawn items inside geometry (use collision handling)

[Continue to IInventoryPlayerInterface →](./Interface_Implementation_Guide.md#iinventoryplayerinterface)

---

## IInventoryPlayerInterface

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Interfaces/InventoryPlayerInterface.h`

### Responsibility

**Player Inventory Access** - provides access to all player-owned inventory components (bags, coins, bank, staging area).

### Why PlayerController?

- **Persistent across character respawns**: Inventory survives character death
- **Network ownership**: PlayerController is owned by client, components replicate correctly
- **One per player**: Each player has exactly one controller
- **Accessible from UI**: Widgets can easily access via `GetOwningPlayer()`

### Interface Definition

```cpp
class IInventoryPlayerInterface
{
    GENERATED_BODY()

public:
    // Inventory component access
    virtual UInventoryComponent* GetInventoryComponent() = 0;
    virtual const UInventoryComponent* GetInventoryComponentConst() const = 0;
    
    // Coin component access
    virtual UCoinComponent* GetCoinComponent() = 0;
    virtual const UCoinComponent* GetCoinComponentConst() const = 0;
    
    // Staging area access (for trading/merchants)
    virtual UStagingAreaComponent* GetStagingAreaComponent() = 0;
    virtual const UStagingAreaComponent* GetStagingAreaComponentConst() const = 0;
    
    virtual UCoinComponent* GetStagingAreaCoinComponent() = 0;
    virtual const UCoinComponent* GetStagingAreaCoinComponentConst() const = 0;
    
    // Bank access
    virtual UBankComponent* GetBankComponent() = 0;
    virtual const UBankComponent* GetBankComponentConst() const = 0;
    
    virtual UCoinComponent* GetBankCoinComponent() = 0;
    virtual const UCoinComponent* GetBankCoinComponentConst() const = 0;
    
    // Owning actor (usually the controlled pawn)
    virtual AActor* GetInventoryOwningActor() = 0;
    virtual AActor const* GetInventoryOwningActorConst() const = 0;
    
    // Transaction state
    virtual bool GetTransactionBoolean() = 0;
    virtual void SetTransactionBoolean(bool Value) = 0;
    
    // Merchant interaction
    virtual AActor* GetMerchantActor() = 0;
    virtual const AActor* GetMerchantActorConst() const = 0;
    virtual void SetMerchantActor(AActor* Actor) = 0;
    
    // Looting interaction
    virtual AActor* GetLootedActor() = 0;
    virtual const AActor* GetLootedActorConst() const = 0;
    virtual void SetLootedActor(AActor* Actor) = 0;
    
    // Helper functions
    virtual TArray<FMinimalItemStorage> GetAllItemsInBag(EBagSlot Slot) = 0;
    virtual FString GetInventoryOwnerName() const = 0;
};
```

### Detailed Implementation

Due to length constraints, see the [full Integration Guide](./Integration_Guide.md#step-3-implement-playercontroller-interface) for the complete PlayerController implementation.

### Key Concepts

**Component Lifecycle**:
```cpp
// Components are created in constructor (server only)
AYourPlayerController::AYourPlayerController()
{
    if (HasAuthority())
    {
        // Create replicated components
        Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
        Inventory->SetNetAddressable(); // Required for subobject replication
        Inventory->SetIsReplicated(true);
        
        // ... create other components
    }
}
```

**Replication Setup**:
```cpp
void AYourPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Replicate all inventory components
    DOREPLIFETIME(AYourPlayerController, Inventory);
    DOREPLIFETIME(AYourPlayerController, CoinPurse);
    DOREPLIFETIME(AYourPlayerController, StagingAreaItems);
    DOREPLIFETIME(AYourPlayerController, StagingAreaCoin);
    DOREPLIFETIME(AYourPlayerController, BankComponent);
    DOREPLIFETIME(AYourPlayerController, BankCoin);
    
    // Replicate transaction state
    DOREPLIFETIME(AYourPlayerController, bInTransaction);
    DOREPLIFETIME(AYourPlayerController, CurrentMerchantActor);
    DOREPLIFETIME(AYourPlayerController, CurrentLootedActor);
}
```

[Continue to IEquipmentInterface →](./Interface_Implementation_Guide.md#iequipmentinterface)

---

## IEquipmentInterface

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Interfaces/EquipmentInterface.h`

### Responsibility

**Equipment Management** - handles items worn by the character with visual attachment to mesh.

### Why Character?

- **Visual representation**: Equipment attaches to character's skeletal mesh
- **Combat integration**: Equipment affects character stats, abilities, appearance
- **Death persistence**: Equipment can drop from corpse on death
- **Per-character**: Each character can have different equipment

### Interface Definition

```cpp
class IEquipmentInterface
{
    GENERATED_BODY()

public:
    // Equipment component access
    virtual UEquipmentComponent* GetEquipmentComponent() = 0;
    virtual const UEquipmentComponent* GetEquipmentComponentConst() const = 0;
    
    // Equip/Unequip operations
    virtual void EquipItem(EEquipmentSlot InSlot, int32 InItemId) = 0;
    virtual void EquipItemWithDurability(EEquipmentSlot InSlot, int32 InItemId, float Durability) = 0;
    virtual void UnequipItem(EEquipmentSlot OutSlot) = 0;
    
    // Query operations
    virtual const UInventoryItemEquipable* GetEquippedItem(EEquipmentSlot Slot) const = 0;
    virtual const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const = 0;
    virtual bool GetEquipmentDurability(EEquipmentSlot Slot, float& OutDurability) const = 0;
    
    // Slot management
    virtual bool TryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment) const = 0;
    virtual EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const = 0;
    virtual void SwapEquipment(EEquipmentSlot DroppedInSlot, EEquipmentSlot DraggedOutSlot) = 0;
    
    // Equipment effects (override in subclasses)
    virtual void HandleEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem) = 0;
    virtual void HandleUnEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem) = 0;
    
    // Utility
    virtual float GetTotalWeight() const = 0;
    virtual UStaticMesh* GetPreferedMesh(UStaticMesh* OriginalMesh) const = 0;
    virtual TArray<FMaterialOverride> GetMaterialOverridesForSlot(EEquipmentSlot Slot) const = 0;
    
    // Ammo compatibility
    virtual bool HasCompatibleAmmoEquipped(EAmmoType AmmoType) const = 0;
    virtual TScriptInterface<IInventoryItemAmmoInterface> RemoveAmmoEquipped(EAmmoType AmmoType) = 0;
};
```

### Detailed Implementation

See [Integration Guide - Step 4](./Integration_Guide.md#step-4-implement-character-equipment-interface) for complete Character implementation.

### Equipment Slots

```cpp
// EEquipmentSlot enum (defined in plugin)
enum class EEquipmentSlot : uint8
{
    Unknown = 0,
    Head = 1,
    Chest = 2,
    Arms = 3,
    Hands = 4,
    Waist = 5,
    Legs = 6,
    Feet = 7,
    Neck = 8,
    Shoulders = 9,
    Back = 10,
    PrimaryWeapon = 11,
    SecondaryWeapon = 12,
    RangedWeapon = 13,
    Ammo = 14,
    Finger1 = 15,
    Finger2 = 16,
    Ear1 = 17,
    Ear2 = 18,
    Wrist1 = 19,
    Wrist2 = 20,
    WaistBag1 = 21,
    WaistBag2 = 22,
    BackPack1 = 23,
    BackPack2 = 24,
    Last = 25
};
```

### Visual Attachment

Equipment items specify socket names for attachment:

```cpp
// In item definition
UPROPERTY(EditAnywhere, Category = "Equipment")
FName AttachmentSocket = NAME_None; // e.g., "head_socket", "hand_r_socket"

// Equipment component handles attachment
void UEquipmentComponent::AttachItemMesh(EEquipmentSlot Slot, UInventoryItemEquipable* Item)
{
    if (!Item || !Item->EquipmentMesh)
        return;
    
    // Create mesh component
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(GetOwner());
    MeshComp->SetStaticMesh(Item->EquipmentMesh);
    MeshComp->RegisterComponent();
    
    // Attach to skeleton
    if (MasterMeshComponent)
    {
        MeshComp->AttachToComponent(
            MasterMeshComponent,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            Item->AttachmentSocket
        );
    }
    
    // Store reference
    AttachedMeshes.Add(Slot, MeshComp);
}
```

---

## Common Implementation Patterns

### Pattern 1: Interface Casting

```cpp
// From any actor, get inventory interface
AActor* SomeActor = ...;
if (IInventoryPlayerInterface* PlayerInv = Cast<IInventoryPlayerInterface>(SomeActor))
{
    UInventoryComponent* Inventory = PlayerInv->GetInventoryComponent();
    // Use inventory
}
```

### Pattern 2: Safe Component Access

```cpp
// Always check for nullptr
UInventoryComponent* Inv = GetInventoryComponent();
if (Inv)
{
    Inv->AddItemAt(EBagSlot::Pocket1, 1000, 0);
}
else
{
    UE_LOG(LogTemp, Error, TEXT("Inventory component is null"));
}
```

### Pattern 3: Const Correctness

```cpp
// Use const getters for read-only operations
const UInventoryComponent* Inv = GetInventoryComponentConst();
if (Inv)
{
    float TotalWeight = Inv->GetTotalWeight();
    // Can't modify, only read
}
```

### Pattern 4: Authority Checks

```cpp
// Always check authority before mutations
void AMyCharacter::EquipWeapon(int32 WeaponID)
{
    if (!HasAuthority())
    {
        // Client: call server RPC
        Server_EquipWeapon(WeaponID);
        return;
    }
    
    // Server: execute
    EquipItem(EEquipmentSlot::PrimaryWeapon, WeaponID);
}

UFUNCTION(Server, Reliable, WithValidation)
void Server_EquipWeapon(int32 WeaponID);
```

---

## Validation and Testing

### Interface Checklist

- [ ] GameInstance implements `IInventoryGameInstanceInterface`
- [ ] GameMode implements `IInventoryGameModeInterface`
- [ ] PlayerController implements `IInventoryPlayerInterface`
- [ ] Character implements `IEquipmentInterface`
- [ ] All components are created with `CreateDefaultSubobject` in constructor
- [ ] All components call `SetIsReplicated(true)` and `SetNetAddressable()`
- [ ] `GetLifetimeReplicatedProps` includes all components
- [ ] Authority checks before all mutations
- [ ] Null checks before dereferencing pointers

### Test Cases

**1. Item Registration Test**
```cpp
void TestItemRegistry()
{
    UYourGameInstance* GI = GetGameInstance<UYourGameInstance>();
    check(GI);
    
    // Register test item
    UInventoryItemBase* Item = NewObject<UInventoryItemBase>();
    Item->ItemID = 99999;
    GI->RegisterItem(Item);
    
    // Retrieve
    UInventoryItemBase* Retrieved = GI->FetchItemFromID(99999);
    check(Retrieved == Item);
}
```

**2. Item Spawning Test**
```cpp
void TestItemSpawning()
{
    AYourGameMode* GM = GetWorld()->GetAuthGameMode<AYourGameMode>();
    check(GM && GM->HasAuthority());
    
    // Spawn test item
    FVector SpawnLoc = FVector(0, 0, 100);
    ADroppedItem* Item = GM->SpawnItemFromActor(this, 1000, SpawnLoc);
    
    check(Item != nullptr);
    check(Item->GetActorLocation().Equals(SpawnLoc, 10.0f));
}
```

**3. Inventory Add/Remove Test**
```cpp
void TestInventoryOperations()
{
    AYourPlayerController* PC = GetController<AYourPlayerController>();
    check(PC);
    
    UInventoryComponent* Inv = PC->GetInventoryComponent();
    check(Inv);
    
    // Add item
    bool bAdded = Inv->AddItemAt(EBagSlot::Pocket1, 1000, 0);
    check(bAdded);
    
    // Verify item exists
    FMinimalItemStorage Item = Inv->GetItemAtSlot(EBagSlot::Pocket1, 0);
    check(Item.ItemID == 1000);
    
    // Remove item
    bool bRemoved = Inv->RemoveItemAtSlot(EBagSlot::Pocket1, 0);
    check(bRemoved);
}
```

**4. Equipment Test**
```cpp
void TestEquipment()
{
    AYourCharacter* Character = GetPawn<AYourCharacter>();
    check(Character);
    
    // Equip helmet
    Character->EquipItem(EEquipmentSlot::Head, 2001);
    
    // Verify equipped
    const UInventoryItemEquipable* Helmet = Character->GetEquippedItem(EEquipmentSlot::Head);
    check(Helmet);
    check(Helmet->ItemID == 2001);
    
    // Unequip
    Character->UnequipItem(EEquipmentSlot::Head);
    
    // Verify removed
    const UInventoryItemEquipable* NoHelmet = Character->GetEquippedItem(EEquipmentSlot::Head);
    check(NoHelmet == nullptr);
}
```

---

## Related Documentation

- [Component Architecture Guide](./Component_Architecture_Guide.md) - Deep dive into component design
- [Replication System Guide](./Replication_System_Guide.md) - How multiplayer synchronization works
- [Integration Guide](./Integration_Guide.md) - Main integration walkthrough

---

## Troubleshooting

**Problem**: "Interface not implemented" compile error

**Solution**: Ensure you inherit from the interface in header:
```cpp
class AYourGameMode : public AGameModeBase, public IInventoryGameModeInterface
```

**Problem**: Components are null at runtime

**Solution**: Check that components are created in constructor with `CreateDefaultSubobject`:
```cpp
AYourPlayerController::AYourPlayerController()
{
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}
```

**Problem**: Changes don't replicate to clients

**Solution**: Verify replication setup:
```cpp
// 1. Component setup
Inventory->SetIsReplicated(true);
Inventory->SetNetAddressable();

// 2. Property replication
DOREPLIFETIME(AYourPlayerController, Inventory);

// 3. Authority check
if (HasAuthority())
{
    // Only modify on server
}
```

---

*Last Updated: 2026-01-30*

