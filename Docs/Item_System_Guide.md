# Item System Guide

## Overview

The InventoryPlugin uses a **data-driven item system** where all items are `UPrimaryDataAsset` objects configured in the Unreal Editor. This guide explains how to create, configure, and manage items.

## Table of Contents

1. [Item Class Hierarchy](#item-class-hierarchy)
2. [Creating Items](#creating-items)
3. [Item Types](#item-types)
4. [Item Properties](#item-properties)
5. [Item Registration](#item-registration)
6. [Item Spawning](#item-spawning)
7. [Best Practices](#best-practices)

---

## Item Class Hierarchy

```
UPrimaryDataAsset
└── UInventoryItemBase (Base item class)
    ├── UInventoryItemEquipable (Wearable items)
    │   ├── UInventoryItemArmor (Armor pieces)
    │   ├── UInventoryItemWeapon (Weapons)
    │   │   ├── UInventoryItemMeleeWeapon (Melee weapons)
    │   │   └── UInventoryItemRangedWeapon (Bows, crossbows)
    │   └── UInventoryItemBag (Bags/containers)
    │       └── UInventoryItemAmmoBag (Quivers)
    ├── UInventoryItemConsumable (Potions, food)
    ├── UInventoryItemBook (Readable books)
    ├── UInventoryItemCraftingMaterial (Crafting components)
    └── UInventoryItemQuestItem (Quest-specific items)
```

---

## Creating Items

### Method 1: In-Editor (Recommended)

**Step 1: Create Data Asset**
1. Right-click in Content Browser
2. **Miscellaneous** → **Data Asset**
3. Select parent class (e.g., `InventoryItemWeapon`)
4. Name it `DA_Item_YourItemName` (convention: `DA_Item_*`)

**Step 2: Configure Properties**
1. Double-click to open asset
2. Set **ItemID** (unique integer)
3. Set **Name** (display name)
4. Configure all required properties
5. Save asset

**Step 3: Register Item**
- Item will be auto-registered if using directory loading
- Or manually register in `GameInstance::Init()`

### Method 2: C++ (Advanced)

```cpp
// Create custom item class
UCLASS()
class YOURPROJECT_API UCustomItem : public UInventoryItemBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom")
    int32 CustomProperty;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom")
    FString CustomString;
    
    // Override behavior
    virtual bool CanBeUsed() const override;
    virtual void OnItemUsed(AActor* User) override;
};

// Implementation
bool UCustomItem::CanBeUsed() const
{
    return CustomProperty > 0;
}

void UCustomItem::OnItemUsed(AActor* User)
{
    Super::OnItemUsed(User);
    
    UE_LOG(LogTemp, Log, TEXT("Used custom item: %s"), *Name);
    // Custom logic here
}
```

---

## Item Types

### Base Item (UInventoryItemBase)

**Purpose**: Generic non-equipable items (quest items, materials, etc.)

**Key Properties**:
```cpp
UCLASS(BlueprintType)
class UInventoryItemBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Core identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 ItemID = -1;                          // Unique identifier
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FString Name = TEXT("Unnamed Item");        // Display name
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
    FString Description = TEXT("");             // Tooltip text
    
    // Visual properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
    UTexture2D* Icon = nullptr;                 // Inventory icon
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
    UStaticMesh* Mesh = nullptr;                // 3D mesh for dropped item
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
    UStaticMesh* GroundPlacedMesh = nullptr;    // Alternative mesh for ground
    
    // Physical properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties")
    int32 Width = 1;                            // Grid width (cells)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties")
    int32 Height = 1;                           // Grid height (cells)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties")
    float Weight = 0.1f;                        // Encumbrance weight
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties")
    EItemSize ItemSize = EItemSize::Small;      // Size category
    
    // Economic properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
    int32 BaseValue = 0;                        // Price in copper
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
    bool bCanBeSold = true;                     // Merchant will buy
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
    bool bCanBeTraded = true;                   // Can trade between players
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
    bool bCanBeDropped = true;                  // Can drop on ground
    
    // Special flags
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Special")
    bool bLoreItem = false;                     // Unique/quest item
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Special")
    bool bMagicItem = false;                    // Has magical properties
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Special")
    bool bStackable = false;                    // Can stack multiple
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Special", meta = (EditCondition = "bStackable"))
    int32 MaxStackSize = 1;                     // Max stack count
};
```

**Example: Quest Item**
```
ItemID: 5001
Name: "Ancient Amulet"
Description: "A mysterious amulet pulsing with dark energy. Quest item."
Icon: T_Icon_Amulet
Mesh: SM_Amulet
Width: 1
Height: 1
Weight: 0.5
ItemSize: Small
BaseValue: 0 (quest items have no value)
bCanBeSold: false
bCanBeTraded: false
bCanBeDropped: false
bLoreItem: true
```

---

### Equipable Item (UInventoryItemEquipable)

**Purpose**: Items that can be worn/wielded by characters.

**Additional Properties**:
```cpp
UCLASS(BlueprintType)
class UInventoryItemEquipable : public UInventoryItemBase
{
    GENERATED_BODY()

public:
    // Equipment slots (bitmask)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    int32 EquipableSlotBitMask = 0;             // Which slots can equip this
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    bool MultiSlotItem = false;                 // Occupies multiple slots (2H weapon)
    
    // Visual attachment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
    UStaticMesh* EquipmentMesh = nullptr;       // Mesh when equipped
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
    FName AttachmentSocket = NAME_None;         // Skeleton socket name
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
    TArray<FMaterialOverride> EquipmentMeshMaterialOverride; // Custom materials
    
    // Durability
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
    float MaxDurability = 100.0f;               // Max condition
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
    float DurabilityLossPerHit = 0.1f;          // Degradation rate
    
    // Stats (override in subclasses)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
    int32 ArmorClass = 0;                       // Armor value
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
    TMap<FString, int32> BonusStats;            // Str, Dex, Int, etc.
};
```

**Slot Bitmask Reference**:
```cpp
// Calculate bitmask for multiple slots
int32 CalculateSlotBitmask(TArray<EEquipmentSlot> Slots)
{
    int32 Bitmask = 0;
    for (EEquipmentSlot Slot : Slots)
    {
        Bitmask |= (1 << static_cast<int32>(Slot));
    }
    return Bitmask;
}

// Examples:
// Head only: 1 << 1 = 2
// Chest only: 1 << 2 = 4
// Head + Chest: (1 << 1) | (1 << 2) = 6
// Primary + Secondary (2H weapon): (1 << 11) | (1 << 12) = 6144
```

**Example: Iron Helmet**
```
ItemID: 2001
Name: "Iron Helmet"
Description: "A sturdy iron helmet. Provides +15 armor."
Icon: T_Icon_Helmet_Iron
Mesh: SM_Helmet_Iron_Dropped
Width: 2
Height: 2
Weight: 5.0
ItemSize: Medium
BaseValue: 500
EquipableSlotBitMask: 2 (Head slot only)
MultiSlotItem: false
EquipmentMesh: SM_Helmet_Iron_Worn
AttachmentSocket: "head_socket"
MaxDurability: 100.0
ArmorClass: 15
BonusStats: {"STR": 1}
```

---

### Weapon Item (UInventoryItemWeapon)

**Purpose**: Weapons with damage, attack speed, and range.

**Additional Properties**:
```cpp
UCLASS(BlueprintType)
class UInventoryItemWeapon : public UInventoryItemEquipable
{
    GENERATED_BODY()

public:
    // Weapon type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    EWeaponType WeaponType = EWeaponType::OneHandedSword;
    
    // Damage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Damage")
    int32 DamageMin = 1;                        // Minimum damage
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Damage")
    int32 DamageMax = 5;                        // Maximum damage
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Damage")
    EDamageType DamageType = EDamageType::Physical; // Damage type
    
    // Attack properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Attack")
    float AttackSpeed = 2.5f;                   // Attacks per second
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Attack")
    float Range = 150.0f;                       // Attack range (cm)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Attack")
    float KnockbackForce = 0.0f;                // Knockback strength
    
    // Skill requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Requirements")
    int32 RequiredLevel = 1;                    // Min character level
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Requirements")
    int32 RequiredStrength = 0;                 // Min STR stat
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Requirements")
    TMap<ESkillType, int32> RequiredSkills;     // Skill requirements
    
    // Ammunition (for ranged weapons)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    EAmmoType RequiredAmmoType = EAmmoType::None;
};
```

**Example: Longsword**
```
ItemID: 3001
Name: "Steel Longsword"
Description: "A well-balanced longsword. Deals 15-25 slashing damage."
Icon: T_Icon_Sword_Long
Mesh: SM_Sword_Long_Dropped
Width: 1
Height: 4
Weight: 3.5
ItemSize: Large
BaseValue: 1000
EquipableSlotBitMask: 2048 (Primary weapon slot)
MultiSlotItem: false
EquipmentMesh: SM_Sword_Long_Equipped
AttachmentSocket: "hand_r_socket"
MaxDurability: 150.0
WeaponType: OneHandedSword
DamageMin: 15
DamageMax: 25
DamageType: Slashing
AttackSpeed: 2.2
Range: 180.0
RequiredLevel: 10
RequiredStrength: 15
```

**Example: Two-Handed Greatsword**
```
ItemID: 3002
Name: "Greatsword"
Description: "A massive two-handed blade. Deals 30-50 slashing damage."
Width: 2
Height: 5
Weight: 8.0
ItemSize: Giant
EquipableSlotBitMask: 6144 (Primary + Secondary slots)
MultiSlotItem: true  ← Occupies both hands
WeaponType: TwoHandedSword
DamageMin: 30
DamageMax: 50
AttackSpeed: 1.5
RequiredStrength: 25
```

---

### Bag Item (UInventoryItemBag)

**Purpose**: Containers that expand inventory space.

**Additional Properties**:
```cpp
UCLASS(BlueprintType)
class UInventoryItemBag : public UInventoryItemEquipable
{
    GENERATED_BODY()

public:
    // Bag dimensions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag")
    int32 BagWidth = 4;                         // Grid width
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag")
    int32 BagHeight = 4;                        // Grid height
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag")
    int32 BagSize = 16;                         // Total cells (Width × Height)
    
    // Size restriction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag")
    EItemSize MaximumItemSizeToContain = EItemSize::Giant;
    
    // Weight reduction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag")
    float WeightReductionFactor = 1.0f;         // 0.0-1.0 (0.5 = 50% reduction)
};
```

**Example: Small Pouch**
```
ItemID: 4001
Name: "Small Pouch"
Description: "A small leather pouch. Provides 3×2 storage."
Icon: T_Icon_Bag_Small
Width: 1
Height: 2
Weight: 0.5
ItemSize: Small
BaseValue: 50
EquipableSlotBitMask: 2097152 (WaistBag1)
BagWidth: 3
BagHeight: 2
BagSize: 6
MaximumItemSizeToContain: Medium
WeightReductionFactor: 1.0 (no reduction)
```

**Example: Enchanted Backpack**
```
ItemID: 4002
Name: "Enchanted Backpack of Holding"
Description: "A magical backpack. Provides 8×8 storage with 50% weight reduction."
Icon: T_Icon_Bag_Enchanted
BaseValue: 5000
EquipableSlotBitMask: 8388608 (BackPack1)
BagWidth: 8
BagHeight: 8
BagSize: 64
MaximumItemSizeToContain: Giant
WeightReductionFactor: 0.5 ← Items weigh 50% less
bMagicItem: true
```

---

## Item Properties

### Item Size Categories

```cpp
UENUM(BlueprintType)
enum class EItemSize : uint8
{
    Tiny = 0,       // Rings, coins, gems
    Small = 1,      // Potions, scrolls, daggers
    Medium = 2,     // Swords, helmets, books
    Large = 3,      // Two-handed weapons, shields
    Giant = 4       // Massive items, furniture
};
```

**Bag Restrictions**: Bags have `MaximumItemSizeToContain`:
- Small pouch (Medium) cannot hold Large/Giant items
- Backpack (Giant) can hold all sizes

### Material Overrides

Custom materials for equipped items:

```cpp
USTRUCT(BlueprintType)
struct FMaterialOverride
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaterialID = 0;                       // Material slot index
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* Material = nullptr;     // Override material
};
```

**Example**:
```
Item: "Red Cloak"
EquipmentMesh: SM_Cloak_Generic
EquipmentMeshMaterialOverride:
  - MaterialID: 0
    Material: M_Cloak_Red
  - MaterialID: 1
    Material: M_Cloak_Trim_Gold
```

---

## Item Registration

### Automatic Registration (Recommended)

Load all items from a directory at startup:

```cpp
void UYourGameInstance::Init()
{
    Super::Init();
    
    // Load all items from Content/Items directory
    LoadItemsFromDirectory(TEXT("/Game/Items"));
}

void UYourGameInstance::LoadItemsFromDirectory(const FString& DirectoryPath)
{
    FAssetRegistryModule& AssetRegistryModule = 
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // Find all UInventoryItemBase assets
    TArray<FAssetData> AssetData;
    FARFilter Filter;
    Filter.PackagePaths.Add(*DirectoryPath);
    Filter.ClassPaths.Add(UInventoryItemBase::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;
    
    AssetRegistry.GetAssets(Filter, AssetData);
    
    // Register each item
    for (const FAssetData& Asset : AssetData)
    {
        UInventoryItemBase* Item = Cast<UInventoryItemBase>(Asset.GetAsset());
        if (Item)
        {
            RegisterItem(Item);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d items from %s"), 
        AssetData.Num(), *DirectoryPath);
}
```

### Manual Registration

For specific items or database-driven systems:

```cpp
void UYourGameInstance::Init()
{
    Super::Init();
    
    // Register hardcoded items
    RegisterItem(LoadObject<UInventoryItemBase>(nullptr, 
        TEXT("/Game/Items/DA_Item_Sword.DA_Item_Sword")));
    
    RegisterItem(LoadObject<UInventoryItemBase>(nullptr, 
        TEXT("/Game/Items/DA_Item_Potion.DA_Item_Potion")));
    
    // Load from database (custom implementation)
    LoadItemsFromDatabase();
}
```

### DataTable Registration

Using a DataTable for item metadata:

```cpp
USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UInventoryItemBase* Item = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRegister = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Category = TEXT("General");
};

void UYourGameInstance::LoadItemsFromDataTable(UDataTable* ItemTable)
{
    if (!ItemTable)
        return;
    
    TArray<FItemTableRow*> AllRows;
    ItemTable->GetAllRows<FItemTableRow>(TEXT("LoadItems"), AllRows);
    
    for (FItemTableRow* Row : AllRows)
    {
        if (Row && Row->Item && Row->bAutoRegister)
        {
            RegisterItem(Row->Item);
        }
    }
}
```

---

## Item Spawning

### Server-Side Spawning

Always spawn items through GameMode:

```cpp
// Spawn item on ground
void AYourCharacter::DropItem(int32 ItemID)
{
    if (!HasAuthority())
    {
        Server_DropItem(ItemID);
        return;
    }
    
    // Get GameMode
    AYourGameMode* GM = GetWorld()->GetAuthGameMode<AYourGameMode>();
    if (!GM)
        return;
    
    // Calculate drop location
    FVector DropLocation = GetActorLocation() + 
        GetActorForwardVector() * 100.0f;
    
    // Spawn dropped item
    ADroppedItem* DroppedItem = GM->SpawnItemFromActor(
        this,
        ItemID,
        DropLocation,
        true,  // Clamp to ground
        100.0f // Full durability
    );
    
    if (DroppedItem)
    {
        UE_LOG(LogTemp, Log, TEXT("Dropped item %d"), ItemID);
    }
}

UFUNCTION(Server, Reliable, WithValidation)
void Server_DropItem(int32 ItemID);

void AYourCharacter::Server_DropItem_Implementation(int32 ItemID)
{
    DropItem(ItemID);
}

bool AYourCharacter::Server_DropItem_Validate(int32 ItemID)
{
    return ItemID > 0;
}
```

### Loot Spawning

Spawn multiple items from loot tables:

```cpp
USTRUCT(BlueprintType)
struct FLootEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemID = -1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DropChance = 1.0f;  // 0.0-1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinQuantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxQuantity = 1;
};

void AYourNPC::SpawnLootOnDeath()
{
    if (!HasAuthority())
        return;
    
    AYourGameMode* GM = GetWorld()->GetAuthGameMode<AYourGameMode>();
    if (!GM)
        return;
    
    FVector CorpseLocation = GetActorLocation();
    
    // Roll loot table
    for (const FLootEntry& Entry : LootTable)
    {
        // Check drop chance
        float Roll = FMath::FRand();
        if (Roll > Entry.DropChance)
            continue;
        
        // Calculate quantity
        int32 Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
        
        // Spawn items
        for (int32 i = 0; i < Quantity; ++i)
        {
            FVector ItemLocation = CorpseLocation + 
                FMath::VRand() * 100.0f; // Random offset
            
            GM->SpawnItemFromActor(this, Entry.ItemID, ItemLocation);
        }
    }
    
    // Spawn coins
    if (CoinDrop.GetTotalCopper() > 0)
    {
        GM->SpawnCoinsFromActor(this, CoinDrop, CorpseLocation);
    }
}
```

---

## Best Practices

### Item ID Organization

Use ID ranges for different item types:

```
1-999:        Quest items
1000-1999:    Consumables (potions, food)
2000-2999:    Armor
3000-3999:    Weapons
4000-4999:    Bags
5000-5999:    Crafting materials
6000-6999:    Books
10000+:       Special/event items
```

### Naming Conventions

- **Data Assets**: `DA_Item_<Name>` (e.g., `DA_Item_IronHelmet`)
- **Textures**: `T_Icon_<Type>_<Name>` (e.g., `T_Icon_Armor_IronHelmet`)
- **Meshes**: `SM_<Type>_<Name>_<State>` (e.g., `SM_Helmet_Iron_Dropped`, `SM_Helmet_Iron_Worn`)
- **Materials**: `M_<Type>_<Name>` (e.g., `M_Armor_Iron`)

### Performance Optimization

**✅ DO:**
- Use texture atlases for item icons (reduce draw calls)
- Keep item descriptions short (< 200 characters)
- Use LODs for 3D item meshes
- Preload frequently used items at startup

**❌ DON'T:**
- Load all items synchronously (use async loading)
- Create unique materials for every item (use material instances)
- Store large textures in item definitions (use references)

### Validation

Add validation to catch errors early:

```cpp
void UYourGameInstance::ValidateItemRegistry()
{
    TSet<int32> UsedIDs;
    TArray<FString> Errors;
    
    for (const auto& Pair : ItemLUT)
    {
        UInventoryItemBase* Item = Pair.Value;
        if (!Item)
        {
            Errors.Add(FString::Printf(TEXT("Null item at ID %d"), Pair.Key));
            continue;
        }
        
        // Check ItemID matches registry key
        if (Item->ItemID != Pair.Key)
        {
            Errors.Add(FString::Printf(
                TEXT("Item '%s' has mismatched ID: Registry=%d, Item=%d"),
                *Item->Name, Pair.Key, Item->ItemID));
        }
        
        // Check for missing icon
        if (!Item->Icon)
        {
            Errors.Add(FString::Printf(
                TEXT("Item '%s' (ID=%d) missing icon"),
                *Item->Name, Item->ItemID));
        }
        
        // Check for invalid dimensions
        if (Item->Width <= 0 || Item->Height <= 0)
        {
            Errors.Add(FString::Printf(
                TEXT("Item '%s' (ID=%d) has invalid dimensions: %dx%d"),
                *Item->Name, Item->ItemID, Item->Width, Item->Height));
        }
        
        // Check for duplicate IDs
        if (UsedIDs.Contains(Item->ItemID))
        {
            Errors.Add(FString::Printf(
                TEXT("Duplicate ItemID: %d (Item: '%s')"),
                Item->ItemID, *Item->Name));
        }
        UsedIDs.Add(Item->ItemID);
    }
    
    // Log all errors
    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Item validation failed with %d errors:"), 
            Errors.Num());
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Item validation passed: %d items OK"), 
            ItemLUT.Num());
    }
}
```

---

## Related Documentation

- [Interface Implementation Guide](./Interface_Implementation_Guide.md) - How to fetch and register items
- [Component Architecture Guide](./Component_Architecture_Guide.md) - How components store items
- [Integration Guide](./Integration_Guide.md) - Full system setup

---

*Last Updated: 2026-01-30*
