# Component Architecture Guide

## Overview

The InventoryPlugin uses a **component-based architecture** where functionality is distributed across specialized, reusable components. This guide provides in-depth explanations of each component, their interactions, and implementation patterns.

## Table of Contents

1. [Architecture Philosophy](#architecture-philosophy)
2. [Component Overview](#component-overview)
3. [Core Components](#core-components)
   - [UInventoryComponent](#uinventorycomponent)
   - [UEquipmentComponent](#uequipmentcomponent)
   - [UCoinComponent](#ucoincomponent)
4. [Extended Components](#extended-components)
   - [UStagingAreaComponent](#ustagingareacomponent)
   - [UBankComponent](#ubankcomponent)
   - [UMerchantComponent](#umerchantcomponent)
   - [URepairComponent](#urepaircomponent)
   - [UFieldRepairComponent](#ufieldrepaircomponent)
5. [Component Interactions](#component-interactions)
6. [Replication Patterns](#replication-patterns)
7. [Best Practices](#best-practices)

---

## Architecture Philosophy

### Component Principles

**1. Single Responsibility**: Each component handles one aspect of inventory
**2. Composition over Inheritance**: Combine components to create complex behaviors
**3. Interface-Driven**: Components communicate through well-defined interfaces
**4. Data-Oriented**: Components primarily manage data, business logic in interfaces

### Component Lifetime

```
┌──────────────────────────────────────────────────────────┐
│                    Component Lifecycle                    │
├──────────────────────────────────────────────────────────┤
│                                                            │
│  1. Construction (in owner's constructor)                 │
│     └─► CreateDefaultSubobject<>()                       │
│                                                            │
│  2. Registration                                          │
│     ├─► SetNetAddressable() [for replication]           │
│     └─► SetIsReplicated(true)                           │
│                                                            │
│  3. Initialization (BeginPlay)                           │
│     └─► Setup initial state, bind delegates             │
│                                                            │
│  4. Runtime                                               │
│     ├─► State mutations (server only)                   │
│     └─► Replication to clients                          │
│                                                            │
│  5. Cleanup (EndPlay/Destroy)                            │
│     └─► Clear delegates, free resources                 │
│                                                            │
└──────────────────────────────────────────────────────────┘
```

### Ownership Model

```
PlayerController (IInventoryPlayerInterface)
│
├─► UInventoryComponent        [Player's bags]
├─► UCoinComponent             [Player's purse]
├─► UStagingAreaComponent      [Temporary trade storage]
├─► UCoinComponent             [Staging area coins]
├─► UBankComponent             [Persistent bank storage]
└─► UCoinComponent             [Bank coins]

Character (IEquipmentInterface)
│
└─► UEquipmentComponent        [Worn equipment]

Merchant NPC (IMerchantInterface)
│
├─► UMerchantComponent         [Merchant inventory]
└─► UCoinComponent             [Merchant's cash]

Repairer NPC (IRepairInterface)
│
└─► URepairComponent           [Repair pricing logic]

Player (Field Repairs)
│
└─► UFieldRepairComponent      [Player repair capability]
```

---

## Component Overview

| Component | Owner | Purpose | Replicated |
|-----------|-------|---------|------------|
| `UInventoryComponent` | PlayerController | Grid-based bag storage | ✅ Yes |
| `UEquipmentComponent` | Character | Worn equipment slots | ✅ Yes |
| `UCoinComponent` | Multiple | Currency storage | ✅ Yes |
| `UStagingAreaComponent` | PlayerController | Temp trade/merchant storage | ✅ Yes |
| `UBankComponent` | PlayerController | Persistent bank storage | ✅ Yes |
| `UMerchantComponent` | Merchant NPC | Merchant inventory + pricing | ✅ Yes |
| `URepairComponent` | Repairer NPC | Repair cost calculations | ✅ Yes |
| `UFieldRepairComponent` | PlayerController | Player-performed repairs | ✅ Yes |

---

## Core Components

### UInventoryComponent

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Components/InventoryComponent.h`

#### Purpose

Manages **grid-based bag storage** with multiple bag slots, item placement, weight tracking, and rotation.

#### Key Features

- **8 bag slots**: Pocket1, Pocket2, WaistBag1/2, BackPack1/2, Quiver, PersonalBag
- **Grid-based placement**: Each bag has Width × Height grid
- **Item rotation**: Items can be rotated 90° for better fit
- **Weight management**: Tracks total weight with bag weight reduction
- **Quiver support**: Special slot for arrows/ammo with type filtering
- **Full replication**: All bag contents replicate to clients

#### Data Structures

```cpp
// Single item in a bag
USTRUCT(BlueprintType)
struct FMinimalItemStorage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 ItemID = -1;                    // Item definition ID
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 TopLeftIndex = -1;               // Grid position (row * Width + col)
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    float Durability = 100.0f;             // Item condition (0-100)
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    bool bRotated = false;                 // 90° rotation flag
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    FVector2D Coordinates = FVector2D(-1); // UI position (row, col)
};

// Bag configuration
USTRUCT(BlueprintType)
struct FBagProperties
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    bool bBagActive = false;               // Bag is available
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 BagWidth = 0;                    // Grid width
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 BagHeight = 0;                   // Grid height
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    EItemSize MaxItemSize = EItemSize::Tiny; // Size restriction
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    float WeightReduction = 1.0f;          // Weight multiplier (0.0-1.0)
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    EAmmoType QuiverAmmoType = EAmmoType::Unknown; // For quiver only
};

// Complete inventory state (replicated)
USTRUCT(BlueprintType)
struct FFullInventoryStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    TMap<EBagSlot, TArray<FMinimalItemStorage>> AllBags;
    
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    TMap<EBagSlot, FBagProperties> BagProperties;
};
```

#### Key Methods

```cpp
class UInventoryComponent : public UActorComponent
{
public:
    // Bag configuration
    void BagSet(EBagSlot BagSlot, bool bActive, int32 Width, int32 Height, 
                EItemSize MaxSize, float WeightReduction);
    void QuiverSpecificSetup(EBagSlot BagSlot, EAmmoType AmmoType);
    
    // Item operations
    bool AddItemAt(EBagSlot BagSlot, int32 ItemID, int32 TopLeftIndex, 
                   float Durability = 100.0f, bool bRotated = false);
    bool AddItemAnyAvailableSlot(int32 ItemID, float Durability = 100.0f);
    bool RemoveItemAtSlot(EBagSlot BagSlot, int32 TopLeftIndex);
    bool MoveItem(EBagSlot FromBag, int32 FromIndex, EBagSlot ToBag, 
                  int32 ToIndex, bool bRotated = false);
    
    // Query operations
    FMinimalItemStorage GetItemAtSlot(EBagSlot BagSlot, int32 TopLeftIndex) const;
    TArray<FMinimalItemStorage> GetBagConst(EBagSlot BagSlot) const;
    bool IsSlotOccupied(EBagSlot BagSlot, int32 TopLeftIndex) const;
    bool CanItemFitAt(EBagSlot BagSlot, int32 ItemID, int32 TopLeftIndex, 
                      bool bRotated = false) const;
    
    // Weight calculations
    float GetTotalWeight() const;
    float GetBagWeight(EBagSlot BagSlot) const;
    float GetMaxCarryWeight() const;
    
    // Bag queries
    FBagProperties GetBagProperties(EBagSlot BagSlot) const;
    bool IsBagActive(EBagSlot BagSlot) const;
    
    // Delegates (fire on state changes)
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FInventoryFullDispatcher FullInventoryDispatcher;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FInventoryItemAddDispatcher InventoryItemAdd;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FInventoryItemRemoveDispatcher InventoryItemRemove;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FInventoryItemMoveDispatcher InventoryItemMove;

protected:
    // Replicated inventory state
    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBags)
    FFullInventoryStruct ReplicatedBags;
    
    UFUNCTION()
    void OnRep_ReplicatedBags();
};
```

#### Usage Example

```cpp
// Initialize bags
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority() && Inventory)
    {
        // Setup default pockets (2x 3x2 bags)
        Inventory->BagSet(EBagSlot::Pocket1, true, 3, 2, EItemSize::Giant, 1.0f);
        Inventory->BagSet(EBagSlot::Pocket2, true, 3, 2, EItemSize::Giant, 1.0f);
        
        // Setup quiver for arrows
        Inventory->BagSet(EBagSlot::Quiver, true, 2, 4, EItemSize::Small, 1.0f);
        Inventory->QuiverSpecificSetup(EBagSlot::Quiver, EAmmoType::Arrows);
    }
}

// Add item to inventory
UFUNCTION(Server, Reliable, WithValidation)
void Server_AddItemToInventory(int32 ItemID);

void AYourPlayerController::Server_AddItemToInventory_Implementation(int32 ItemID)
{
    if (Inventory)
    {
        // Try to add to any available slot
        bool bSuccess = Inventory->AddItemAnyAvailableSlot(ItemID, 100.0f);
        
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Added item %d to inventory"), ItemID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory full, cannot add item %d"), ItemID);
            // Optionally spawn item on ground
        }
    }
}

bool AYourPlayerController::Server_AddItemToInventory_Validate(int32 ItemID)
{
    return ItemID > 0;
}
```

#### Grid Indexing

```
Grid Layout (3x2 bag):
┌─────┬─────┬─────┐
│  0  │  1  │  2  │  Row 0
├─────┼─────┼─────┤
│  3  │  4  │  5  │  Row 1
└─────┴─────┴─────┘

Index = (Row * Width) + Column
Coordinates = FVector2D(Row, Column)

Example: Item at Row 1, Col 2
  Index = (1 * 3) + 2 = 5
  Coordinates = (1, 2)
```

#### Item Placement Algorithm

```cpp
bool UInventoryComponent::CanItemFitAt(EBagSlot BagSlot, int32 ItemID, 
                                       int32 TopLeftIndex, bool bRotated) const
{
    // Get item definition
    UInventoryItemBase* Item = FetchItemFromID(ItemID);
    if (!Item) return false;
    
    // Get bag properties
    FBagProperties BagProps = GetBagProperties(BagSlot);
    if (!BagProps.bBagActive) return false;
    
    // Check size restriction
    if (Item->ItemSize > BagProps.MaxItemSize) return false;
    
    // Get item dimensions (handle rotation)
    int32 ItemWidth = bRotated ? Item->Height : Item->Width;
    int32 ItemHeight = bRotated ? Item->Width : Item->Height;
    
    // Calculate top-left coordinates
    int32 StartRow = TopLeftIndex / BagProps.BagWidth;
    int32 StartCol = TopLeftIndex % BagProps.BagWidth;
    
    // Check if item fits within bag bounds
    if (StartCol + ItemWidth > BagProps.BagWidth) return false;
    if (StartRow + ItemHeight > BagProps.BagHeight) return false;
    
    // Check each occupied cell
    for (int32 Row = StartRow; Row < StartRow + ItemHeight; ++Row)
    {
        for (int32 Col = StartCol; Col < StartCol + ItemWidth; ++Col)
        {
            int32 CellIndex = (Row * BagProps.BagWidth) + Col;
            
            // Check if cell is occupied by another item
            if (IsSlotOccupied(BagSlot, CellIndex))
                return false;
        }
    }
    
    return true;
}
```

---

### UEquipmentComponent

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Components/EquipmentComponent.h`

#### Purpose

Manages **worn equipment** with visual mesh attachment, durability tracking, and stat effects.

#### Key Features

- **24 equipment slots**: Head, Chest, Weapons, Rings, Bags, etc.
- **Visual attachment**: Meshes attach to character skeleton sockets
- **Durability system**: Equipment degrades over time
- **Material overrides**: Custom materials per item
- **Multi-slot items**: Two-handed weapons occupy multiple slots
- **Stat effects**: Equipment modifies character attributes

#### Data Structures

```cpp
// Equipment slot enum
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Unknown = 0,
    
    // Armor
    Head = 1,
    Chest = 2,
    Arms = 3,
    Hands = 4,
    Waist = 5,
    Legs = 6,
    Feet = 7,
    Shoulders = 9,
    
    // Accessories
    Neck = 8,
    Back = 10,
    Finger1 = 15,
    Finger2 = 16,
    Ear1 = 17,
    Ear2 = 18,
    Wrist1 = 19,
    Wrist2 = 20,
    
    // Weapons
    PrimaryWeapon = 11,
    SecondaryWeapon = 12,
    RangedWeapon = 13,
    Ammo = 14,
    
    // Bags
    WaistBag1 = 21,
    WaistBag2 = 22,
    BackPack1 = 23,
    BackPack2 = 24,
    
    Last = 25
};

// Equipment durability tracking
USTRUCT(BlueprintType)
struct FEquipmentDurability
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Equipment")
    float CurrentDurability = 100.0f;
    
    UPROPERTY(BlueprintReadWrite, Category = "Equipment")
    float MaxDurability = 100.0f;
};

// Complete equipment state (replicated)
USTRUCT(BlueprintType)
struct FFullEquipmentStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Equipment")
    TMap<EEquipmentSlot, int32> EquippedItemIDs;
    
    UPROPERTY(BlueprintReadWrite, Category = "Equipment")
    TMap<EEquipmentSlot, FEquipmentDurability> EquipmentDurabilities;
};
```

#### Key Methods

```cpp
class UEquipmentComponent : public UActorComponent
{
public:
    // Equipment operations
    void EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot Slot);
    void EquipItemWithDurability(const UInventoryItemEquipable* Item, 
                                  EEquipmentSlot Slot, float Durability);
    void RemoveItem(EEquipmentSlot Slot);
    void UnequipItem(EEquipmentSlot Slot);
    
    // Query operations
    const UInventoryItemEquipable* GetItemAtSlot(EEquipmentSlot Slot) const;
    const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const;
    bool IsSlotOccupied(EEquipmentSlot Slot) const;
    EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const;
    
    // Durability management
    bool GetEquipmentDurability(EEquipmentSlot Slot, float& OutCurrent, 
                                float& OutMax) const;
    void SetEquipmentDurability(EEquipmentSlot Slot, float NewDurability);
    void ApplyDurabilityLoss(EEquipmentSlot Slot, float DamageAmount);
    void RepairEquipment(EEquipmentSlot Slot, float RepairAmount);
    
    // Visual updates
    void UpdateMasterMeshComponent(USkeletalMeshComponent* MeshComp);
    void RefreshEquipmentVisuals();
    void AttachItemMesh(EEquipmentSlot Slot, const UInventoryItemEquipable* Item);
    void DetachItemMesh(EEquipmentSlot Slot);
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FEquipmentChangedDispatcher OnEquipmentChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FDurabilityChangedDispatcher OnDurabilityChanged;

protected:
    // Replicated equipment state
    UPROPERTY(ReplicatedUsing = OnRep_Equipment)
    FFullEquipmentStruct ReplicatedEquipment;
    
    UFUNCTION()
    void OnRep_Equipment();
    
    // Visual mesh components
    UPROPERTY()
    TMap<EEquipmentSlot, UStaticMeshComponent*> AttachedMeshes;
    
    UPROPERTY()
    USkeletalMeshComponent* MasterMeshComponent;
};
```

#### Usage Example

```cpp
// Equip a weapon
void AYourCharacter::EquipWeapon(int32 WeaponID)
{
    if (!HasAuthority())
    {
        Server_EquipWeapon(WeaponID);
        return;
    }
    
    if (Equipment)
    {
        // Get item from registry
        UInventoryItemBase* Item = FetchItemFromID(WeaponID);
        UInventoryItemEquipable* Weapon = Cast<UInventoryItemEquipable>(Item);
        
        if (Weapon)
        {
            // Determine slot (primary or two-handed)
            EEquipmentSlot TargetSlot = Weapon->MultiSlotItem ? 
                EEquipmentSlot::PrimaryWeapon : 
                EEquipmentSlot::PrimaryWeapon;
            
            // Unequip existing item
            if (Equipment->IsSlotOccupied(TargetSlot))
            {
                Equipment->RemoveItem(TargetSlot);
            }
            
            // Equip new weapon
            Equipment->EquipItem(Weapon, TargetSlot);
            
            UE_LOG(LogTemp, Log, TEXT("Equipped weapon: %s"), *Weapon->Name);
        }
    }
}

// Apply durability damage
void AYourCharacter::OnTakeDamage(float Damage)
{
    if (!HasAuthority() || !Equipment)
        return;
    
    // Damage armor proportionally
    TArray<EEquipmentSlot> ArmorSlots = {
        EEquipmentSlot::Head,
        EEquipmentSlot::Chest,
        EEquipmentSlot::Arms,
        EEquipmentSlot::Legs
    };
    
    for (EEquipmentSlot Slot : ArmorSlots)
    {
        if (Equipment->IsSlotOccupied(Slot))
        {
            // Apply 1% durability loss per 10 damage
            float DurabilityLoss = Damage * 0.1f;
            Equipment->ApplyDurabilityLoss(Slot, DurabilityLoss);
            
            // Check if broken
            float CurrentDur, MaxDur;
            if (Equipment->GetEquipmentDurability(Slot, CurrentDur, MaxDur))
            {
                if (CurrentDur <= 0.0f)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Equipment slot %d broke!"), (int32)Slot);
                    Equipment->RemoveItem(Slot);
                }
            }
        }
    }
}
```

#### Visual Attachment System

```cpp
void UEquipmentComponent::AttachItemMesh(EEquipmentSlot Slot, 
                                         const UInventoryItemEquipable* Item)
{
    if (!Item || !Item->EquipmentMesh || !MasterMeshComponent)
        return;
    
    // Detach old mesh if exists
    DetachItemMesh(Slot);
    
    // Create new mesh component
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(
        GetOwner(), 
        *FString::Printf(TEXT("EquipMesh_%d"), (int32)Slot)
    );
    
    MeshComp->SetStaticMesh(Item->EquipmentMesh);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetCastShadow(true);
    MeshComp->RegisterComponent();
    
    // Apply material overrides
    for (const FMaterialOverride& Override : Item->EquipmentMeshMaterialOverride)
    {
        if (Override.Material)
        {
            MeshComp->SetMaterial(Override.MaterialID, Override.Material);
        }
    }
    
    // Attach to skeleton socket
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        true // Weld simulated bodies
    );
    
    MeshComp->AttachToComponent(
        MasterMeshComponent,
        AttachRules,
        Item->AttachmentSocket
    );
    
    // Store reference
    AttachedMeshes.Add(Slot, MeshComp);
    
    UE_LOG(LogTemp, Verbose, TEXT("Attached mesh for slot %d to socket %s"),
        (int32)Slot, *Item->AttachmentSocket.ToString());
}

void UEquipmentComponent::DetachItemMesh(EEquipmentSlot Slot)
{
    if (UStaticMeshComponent** MeshComp = AttachedMeshes.Find(Slot))
    {
        if (*MeshComp && (*MeshComp)->IsValidLowLevel())
        {
            (*MeshComp)->DestroyComponent();
        }
        AttachedMeshes.Remove(Slot);
    }
}
```

---

### UCoinComponent

**File**: `Plugins/InventoryPlugin/Source/InventoryPlugin/Public/Components/CoinComponent.h`

#### Purpose

Manages **four-tier currency** (Copper, Silver, Gold, Platinum) with automatic conversion and arithmetic.

#### Key Features

- **Four currency tiers**: 100 Copper = 1 Silver, 100 Silver = 1 Gold, 100 Gold = 1 Platinum
- **Automatic conversion**: Overflow converts to higher tier
- **Arithmetic operations**: Add, subtract, multiply, divide
- **Comparison operators**: <, >, ==, !=
- **Replicated**: Currency syncs to clients
- **Transaction safety**: Validates sufficient funds before deduction

#### Data Structures

```cpp
USTRUCT(BlueprintType)
struct FCoinValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int32 Copper = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int32 Silver = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int32 Gold = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int32 Platinum = 0;
    
    // Conversion helper
    int32 GetTotalCopper() const
    {
        return Copper + (Silver * 100) + (Gold * 10000) + (Platinum * 1000000);
    }
    
    // Arithmetic operators
    FCoinValue operator+(const FCoinValue& Other) const;
    FCoinValue operator-(const FCoinValue& Other) const;
    bool operator>=(const FCoinValue& Other) const;
    bool operator<=(const FCoinValue& Other) const;
};
```

#### Key Methods

```cpp
class UCoinComponent : public UActorComponent
{
public:
    // Add currency
    void AddCopper(int32 Amount);
    void AddSilver(int32 Amount);
    void AddGold(int32 Amount);
    void AddPlatinum(int32 Amount);
    void AddCoinValue(const FCoinValue& Value);
    
    // Remove currency (returns false if insufficient funds)
    bool RemoveCopper(int32 Amount);
    bool RemoveSilver(int32 Amount);
    bool RemoveGold(int32 Amount);
    bool RemovePlatinum(int32 Amount);
    bool RemoveCoinValue(const FCoinValue& Value);
    
    // Query operations
    FCoinValue GetCoinValue() const;
    int32 GetTotalCopper() const;
    bool HasSufficientFunds(const FCoinValue& Required) const;
    bool IsEmpty() const;
    
    // Transfer operations
    bool TransferTo(UCoinComponent* Target, const FCoinValue& Amount);
    bool TransferAllTo(UCoinComponent* Target);
    
    // Delegate
    UPROPERTY(BlueprintAssignable, Category = "Currency")
    FCoinChangedDispatcher OnCoinChanged;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CoinValue)
    FCoinValue CurrentCoinValue;
    
    UFUNCTION()
    void OnRep_CoinValue();
    
    // Auto-convert overflow (100 copper → 1 silver, etc.)
    void NormalizeCurrency();
};
```

#### Usage Example

```cpp
// Add loot coins
void AYourPlayerController::AddLootMoney(int32 CopperAmount)
{
    if (!HasAuthority() || !CoinPurse)
        return;
    
    CoinPurse->AddCopper(CopperAmount);
    
    UE_LOG(LogTemp, Log, TEXT("Added %d copper to player purse"), CopperAmount);
}

// Purchase item from merchant
bool AYourPlayerController::PurchaseItem(int32 ItemID, const FCoinValue& Price)
{
    if (!HasAuthority() || !CoinPurse)
        return false;
    
    // Check if player has enough money
    if (!CoinPurse->HasSufficientFunds(Price))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient funds to purchase item %d"), ItemID);
        return false;
    }
    
    // Deduct price
    if (!CoinPurse->RemoveCoinValue(Price))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to deduct purchase price"));
        return false;
    }
    
    // Add item to inventory
    if (Inventory)
    {
        Inventory->AddItemAnyAvailableSlot(ItemID, 100.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Purchased item %d for %dpp %dgp %dsp %dcp"),
        ItemID, Price.Platinum, Price.Gold, Price.Silver, Price.Copper);
    
    return true;
}

// Currency normalization
void UCoinComponent::NormalizeCurrency()
{
    // Convert 100 copper to 1 silver
    if (CurrentCoinValue.Copper >= 100)
    {
        int32 ExtraSilver = CurrentCoinValue.Copper / 100;
        CurrentCoinValue.Silver += ExtraSilver;
        CurrentCoinValue.Copper %= 100;
    }
    
    // Convert 100 silver to 1 gold
    if (CurrentCoinValue.Silver >= 100)
    {
        int32 ExtraGold = CurrentCoinValue.Silver / 100;
        CurrentCoinValue.Gold += ExtraGold;
        CurrentCoinValue.Silver %= 100;
    }
    
    // Convert 100 gold to 1 platinum
    if (CurrentCoinValue.Gold >= 100)
    {
        int32 ExtraPlatinum = CurrentCoinValue.Gold / 100;
        CurrentCoinValue.Platinum += ExtraPlatinum;
        CurrentCoinValue.Gold %= 100;
    }
}
```

---

## Extended Components

### UStagingAreaComponent

**Purpose**: Temporary storage for trades, merchant transactions, and mail.

**Use Cases**:
- Player-to-player trading
- Merchant buy/sell staging
- Mail composition

**Key Features**:
- Separate from main inventory
- Can be cleared/committed atomically
- Tracks item sources for rollback

### UBankComponent

**Purpose**: Persistent storage across sessions (larger than inventory).

**Use Cases**:
- Long-term item storage
- Shared bank between characters (optional)
- Safe storage in towns

**Key Features**:
- Larger grid (e.g., 10×10 per bag slot)
- Persists to database
- Access restricted to bank NPCs/zones

### UMerchantComponent

**Purpose**: Merchant inventory and pricing logic.

**Use Cases**:
- NPC merchants
- Player-owned shops
- Vending machines

**Key Features**:
- Static inventory (always available)
- Dynamic inventory (restocks over time)
- Buy/sell ratio configuration
- Merchant cash pool

### URepairComponent

**Purpose**: Repair cost calculations for NPC repairers.

**Key Features**:
- Cost based on durability loss
- Pricing modifiers per item type
- Reputation discounts

### UFieldRepairComponent

**Purpose**: Player-performed field repairs using repair kits.

**Key Features**:
- Limited repair capability (max 50% restoration)
- Consumes repair kit items
- Skill-based success chance

---

## Component Interactions

### Example: Player Loots Corpse

```
1. Player interacts with corpse (ILootableInterface)
   └─► Server validates corpse has loot

2. Server opens loot window
   └─► Player's StagingAreaComponent populated with corpse items

3. Player drags item from staging to inventory
   ├─► Client: UI drag event
   ├─► Client→Server: RPC with (ItemID, FromSlot, ToSlot)
   ├─► Server: Validate move
   ├─► Server: StagingAreaComponent removes item
   ├─► Server: InventoryComponent adds item
   └─► Server→All Clients: Replication updates UI

4. Player closes loot window
   ├─► Server: Clear StagingAreaComponent
   └─► Server: Destroy corpse if empty
```

### Example: Player Buys from Merchant

```
1. Player interacts with merchant (IMerchantInterface)
   └─► Server validates merchant has item

2. Player clicks "buy" on item
   ├─► Client→Server: RPC with (ItemID, Quantity)
   ├─► Server: MerchantComponent checks stock
   ├─► Server: Calculate price (BaseValue × SellRatio)
   ├─► Server: CoinComponent validates funds
   ├─► Server: CoinComponent removes price
   ├─► Server: MerchantComponent transfers item
   ├─► Server: InventoryComponent adds item
   └─► Server→Clients: Replication updates

3. If player sells to merchant
   ├─► Server: InventoryComponent removes item
   ├─► Server: MerchantComponent adds item (if buying)
   ├─► Server: Calculate sell price (BaseValue × BuyRatio)
   ├─► Server: CoinComponent adds payment
   └─► Server→Clients: Replication updates
```

---

## Replication Patterns

### Struct Replication

All components use `ReplicatedUsing` for delta updates:

```cpp
UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBags)
FFullInventoryStruct ReplicatedBags;

UFUNCTION()
void OnRep_ReplicatedBags()
{
    // Fire delegates to update UI
    FullInventoryDispatcher.Broadcast();
}
```

### Subobject Replication

Components must be marked for replication:

```cpp
Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
Inventory->SetNetAddressable();  // Required for subobject replication
Inventory->SetIsReplicated(true);
```

### Authority Pattern

All mutations happen on server:

```cpp
// Client calls RPC
void AYourPC::Client_MoveItem(...)
{
    if (!HasAuthority())
    {
        Server_MoveItem(...);
        return;
    }
    
    // Server executes
    Inventory->MoveItem(...);
}
```

---

## Best Practices

### ✅ DO

- **Create components in constructor**: Use `CreateDefaultSubobject<>()`
- **Set replication flags**: `SetIsReplicated(true)` and `SetNetAddressable()`
- **Check authority**: All mutations must check `HasAuthority()`
- **Use const getters**: Read-only operations use `const` methods
- **Fire delegates**: Notify UI of state changes via delegates
- **Validate inputs**: Check for null pointers, invalid IDs, etc.

### ❌ DON'T

- **Create components at runtime**: Components must exist before `BeginPlay`
- **Modify on client**: All state changes go through server RPCs
- **Replicate functions**: Use `ReplicatedUsing` on properties instead
- **Forget null checks**: Always validate component pointers
- **Block on operations**: Keep component methods fast (< 1ms)

---

## Related Documentation

- [Interface Implementation Guide](./Interface_Implementation_Guide.md) - How to implement required interfaces
- [Replication System Guide](./Replication_System_Guide.md) - Deep dive into networking
- [Integration Guide](./Integration_Guide.md) - Step-by-step setup

---

*Last Updated: 2026-01-30*
