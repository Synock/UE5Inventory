# Replication System Guide

## Overview

The InventoryPlugin uses Unreal Engine's built-in replication system to synchronize inventory state across server and clients. This guide explains how replication works, common patterns, and troubleshooting.

## Table of Contents

1. [Replication Fundamentals](#replication-fundamentals)
2. [Component Replication](#component-replication)
3. [Property Replication](#property-replication)
4. [RPC Patterns](#rpc-patterns)
5. [Optimization Strategies](#optimization-strategies)
6. [Common Issues](#common-issues)
7. [Testing Multiplayer](#testing-multiplayer)

---

## Replication Fundamentals

### Authority Model

Unreal uses a **server-authoritative** model:

```
┌────────────────────────────────────────────────────────┐
│                  Authority Hierarchy                    │
├────────────────────────────────────────────────────────┤
│                                                          │
│  SERVER (Authority = true)                              │
│    └─► Owns all actors and components                  │
│    └─► Executes gameplay logic                         │
│    └─► Validates all client requests                   │
│    └─► Replicates state changes to clients             │
│                                                          │
│  CLIENTS (Authority = false)                            │
│    └─► Receive replicated state                        │
│    └─► Display UI based on state                       │
│    └─► Send input via RPCs to server                   │
│    └─► Cannot directly modify replicated properties    │
│                                                          │
└────────────────────────────────────────────────────────┘
```

### Replication Flow

```
1. Client Action (e.g., drag item in UI)
   └─► Client detects UI event

2. Client → Server RPC
   └─► Server_MoveItem(FromBag, FromIndex, ToBag, ToIndex)

3. Server Validation
   ├─► Check authority: HasAuthority() == true
   ├─► Validate parameters: ItemID > 0, Slots valid
   ├─► Validate game state: Player owns item, target slot empty
   └─► Execute if valid, reject if invalid

4. Server State Mutation
   └─► Component->MoveItem(...) modifies replicated property

5. Server → All Clients Replication
   ├─► ReplicatedBags property changes
   ├─► OnRep_ReplicatedBags() fires on all clients
   └─► UI updates on all clients

6. UI Update
   └─► Delegates fire → Widgets refresh
```

---

## Component Replication

### Subobject Registration

Components must be registered for replication:

```cpp
// In PlayerController constructor
AYourPlayerController::AYourPlayerController()
{
    // ONLY create components on server
    if (HasAuthority())
    {
        // Create component
        Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
        
        // Mark as replicated
        Inventory->SetIsReplicated(true);
        
        // Enable subobject replication (CRITICAL)
        Inventory->SetNetAddressable();
    }
}
```

**Why `SetNetAddressable()`?**

Subobjects (components owned by actors) don't replicate by default. `SetNetAddressable()` assigns a stable network ID so clients can reference the same component instance.

### Component Lifetime

```cpp
void AYourPlayerController::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Register component for replication
    DOREPLIFETIME(AYourPlayerController, Inventory);
    DOREPLIFETIME(AYourPlayerController, CoinPurse);
    // ... other components
}
```

**Replication Conditions**:

```cpp
// Replicate to owner only (player can't see others' inventory)
DOREPLIFETIME_CONDITION(AYourPlayerController, Inventory, COND_OwnerOnly);

// Replicate to all clients
DOREPLIFETIME(AYourPlayerController, Inventory);

// Skip initial replication (load from save file instead)
DOREPLIFETIME_CONDITION(AYourPlayerController, Inventory, COND_InitialOnly);
```

---

## Property Replication

### ReplicatedUsing Pattern

The plugin uses `ReplicatedUsing` for efficient delta updates:

```cpp
class UInventoryComponent : public UActorComponent
{
protected:
    // Replicated property with callback
    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBags)
    FFullInventoryStruct ReplicatedBags;
    
    // Callback fires on clients when property changes
    UFUNCTION()
    void OnRep_ReplicatedBags();
};

void UInventoryComponent::OnRep_ReplicatedBags()
{
    // Update local cache (if needed)
    // ...
    
    // Notify UI of changes
    FullInventoryDispatcher.Broadcast();
    
    UE_LOG(LogTemp, Verbose, TEXT("Inventory replicated to client"));
}
```

### Struct Replication

Complex data uses custom structs:

```cpp
USTRUCT(BlueprintType)
struct FFullInventoryStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TMap<EBagSlot, TArray<FMinimalItemStorage>> AllBags;
    
    UPROPERTY(BlueprintReadWrite)
    TMap<EBagSlot, FBagProperties> BagProperties;
    
    // Custom replication logic (optional)
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

// Enable fast serialization
template<>
struct TStructOpsTypeTraits<FFullInventoryStruct> : 
    public TStructOpsTypeTraitsBase2<FFullInventoryStruct>
{
    enum
    {
        WithNetSerializer = true, // Use custom NetSerialize
        WithIdenticalViaEquality = true // Fast comparison
    };
};
```

### Delta Compression

Unreal automatically compresses replication:

```
┌────────────────────────────────────────────────────────┐
│              Delta Compression Example                  │
├────────────────────────────────────────────────────────┤
│                                                          │
│  Frame 1: Full inventory state (1000 bytes)            │
│    └─► Sent to new client on connection                │
│                                                          │
│  Frame 2: Player adds 1 item                           │
│    └─► Only changed item sent (50 bytes)               │
│                                                          │
│  Frame 3: No changes                                    │
│    └─► Nothing sent (0 bytes)                          │
│                                                          │
│  Frame 4: Player moves 1 item                          │
│    └─► Only position change sent (20 bytes)            │
│                                                          │
└────────────────────────────────────────────────────────┘
```

---

## RPC Patterns

### Server RPCs

**Definition**:
```cpp
// Declare RPC in header
UFUNCTION(Server, Reliable, WithValidation)
void Server_AddItemToInventory(int32 ItemID, EBagSlot Bag, int32 TopLeftIndex);

// Implementation
void AYourPlayerController::Server_AddItemToInventory_Implementation(
    int32 ItemID, EBagSlot Bag, int32 TopLeftIndex)
{
    // Server-only logic
    if (!Inventory)
        return;
    
    // Validate game state
    UInventoryItemBase* Item = FetchItemFromID(ItemID);
    if (!Item)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid ItemID: %d"), ItemID);
        return;
    }
    
    // Execute mutation
    bool bSuccess = Inventory->AddItemAt(Bag, ItemID, TopLeftIndex);
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to add item %d to bag"), ItemID);
    }
}

// Validation (anti-cheat)
bool AYourPlayerController::Server_AddItemToInventory_Validate(
    int32 ItemID, EBagSlot Bag, int32 TopLeftIndex)
{
    // Basic parameter validation
    if (ItemID <= 0)
        return false;
    
    if (TopLeftIndex < 0)
        return false;
    
    if (Bag == EBagSlot::Unknown)
        return false;
    
    return true;
}
```

**Usage**:
```cpp
void AYourPlayerController::ClientRequestAddItem(int32 ItemID)
{
    if (HasAuthority())
    {
        // Already on server, execute directly
        Server_AddItemToInventory_Implementation(ItemID, EBagSlot::Pocket1, 0);
    }
    else
    {
        // On client, send RPC to server
        Server_AddItemToInventory(ItemID, EBagSlot::Pocket1, 0);
    }
}
```

### Client RPCs

**Definition**:
```cpp
// Server notifies specific client
UFUNCTION(Client, Reliable)
void Client_NotifyInventoryFull();

void AYourPlayerController::Client_NotifyInventoryFull_Implementation()
{
    // Runs on client only
    if (IsLocalController())
    {
        // Show UI message
        UE_LOG(LogTemp, Warning, TEXT("Inventory is full!"));
        // Display toast notification
    }
}
```

**Usage**:
```cpp
void AYourPlayerController::Server_AddItemToInventory_Implementation(...)
{
    bool bSuccess = Inventory->AddItemAt(...);
    
    if (!bSuccess)
    {
        // Notify client of failure
        Client_NotifyInventoryFull();
    }
}
```

### Multicast RPCs

**Definition**:
```cpp
// Server broadcasts to all clients
UFUNCTION(NetMulticast, Reliable)
void Multicast_PlayDropItemEffect(FVector Location, int32 ItemID);

void AYourCharacter::Multicast_PlayDropItemEffect_Implementation(
    FVector Location, int32 ItemID)
{
    // Runs on server + all clients
    if (UWorld* World = GetWorld())
    {
        // Spawn visual effect
        UGameplayStatics::SpawnEmitterAtLocation(World, DropEffect, Location);
        // Play sound
        UGameplayStatics::PlaySoundAtLocation(World, DropSound, Location);
    }
}
```

**Usage**:
```cpp
void AYourCharacter::DropItem(int32 ItemID)
{
    if (!HasAuthority())
        return;
    
    FVector DropLoc = GetActorLocation() + GetActorForwardVector() * 100.0f;
    
    // Spawn item actor
    GameMode->SpawnItemFromActor(this, ItemID, DropLoc);
    
    // Play effect on all clients
    Multicast_PlayDropItemEffect(DropLoc, ItemID);
}
```

---

## Optimization Strategies

### 1. Replication Rate Limiting

```cpp
// In component constructor
UInventoryComponent::UInventoryComponent()
{
    // Limit replication frequency (default: 100Hz)
    SetNetUpdateFrequency(10.0f); // 10 updates/second
    
    // Enable relevancy checks
    SetIsReplicatedByDefault(true);
}
```

### 2. Conditional Replication

```cpp
// Only replicate to owner
DOREPLIFETIME_CONDITION(AYourPlayerController, Inventory, COND_OwnerOnly);

// Only replicate when changed
DOREPLIFETIME_CONDITION(AYourPlayerController, CoinPurse, COND_InitialOrOwner);

// Skip initial replication (load from save)
DOREPLIFETIME_CONDITION(AYourPlayerController, BankComponent, COND_SkipOwner);
```

### 3. Batching Updates

```cpp
// Bad: Multiple individual updates
void BadApproach()
{
    Inventory->AddItemAt(EBagSlot::Pocket1, 1001, 0);  // Replicate
    Inventory->AddItemAt(EBagSlot::Pocket1, 1002, 1);  // Replicate
    Inventory->AddItemAt(EBagSlot::Pocket1, 1003, 2);  // Replicate
    // 3 replication updates!
}

// Good: Batch updates
void GoodApproach()
{
    TArray<FMinimalItemStorage> ItemsToAdd = {
        {1001, 0, 100.0f, false},
        {1002, 1, 100.0f, false},
        {1003, 2, 100.0f, false}
    };
    
    Inventory->AddMultipleItems(EBagSlot::Pocket1, ItemsToAdd);
    // 1 replication update!
}
```

### 4. Relevancy Filtering

```cpp
// Override relevancy for dropped items
bool ADroppedItem::IsNetRelevantFor(
    const AActor* RealViewer, 
    const AActor* ViewTarget, 
    const FVector& SrcLocation) const
{
    // Only relevant if within pickup range
    float DistSq = (SrcLocation - GetActorLocation()).SizeSquared();
    float MaxDistSq = PickupRange * PickupRange;
    
    return DistSq <= MaxDistSq;
}
```

### 5. Struct Packing

```cpp
// Bad: Wasteful struct layout
struct FBadItemStorage
{
    int32 ItemID;        // 4 bytes
    bool bRotated;       // 1 byte + 3 padding
    int32 TopLeftIndex;  // 4 bytes
    float Durability;    // 4 bytes
    // Total: 16 bytes (25% wasted on padding)
};

// Good: Optimized layout
struct FGoodItemStorage
{
    int32 ItemID;        // 4 bytes
    int32 TopLeftIndex;  // 4 bytes
    float Durability;    // 4 bytes
    bool bRotated;       // 1 byte + 3 padding at end
    // Total: 16 bytes (18% padding, grouped at end)
};
```

---

## Common Issues

### Issue 1: Components Are Null on Client

**Symptom**: `Inventory` component is null when accessed on client.

**Cause**: Components not marked for replication.

**Solution**:
```cpp
// In constructor
Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
Inventory->SetIsReplicated(true);     // ← Add this
Inventory->SetNetAddressable();       // ← Add this

// In GetLifetimeReplicatedProps
DOREPLIFETIME(AYourPlayerController, Inventory); // ← Add this
```

### Issue 2: Changes Don't Replicate

**Symptom**: Server changes inventory, client UI doesn't update.

**Cause**: Forgot `ReplicatedUsing` callback or didn't fire delegates.

**Solution**:
```cpp
// Add callback
UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBags)
FFullInventoryStruct ReplicatedBags;

// Implement callback
void UInventoryComponent::OnRep_ReplicatedBags()
{
    FullInventoryDispatcher.Broadcast(); // ← Fire delegate
}

// Bind delegate in UI
Inventory->FullInventoryDispatcher.AddDynamic(this, &UMyWidget::OnInventoryChanged);
```

### Issue 3: RPC Fails Silently

**Symptom**: Client calls `Server_` function, nothing happens on server.

**Cause**: Validation function returns false.

**Solution**:
```cpp
bool AYourPlayerController::Server_AddItem_Validate(int32 ItemID)
{
    // Check validation logic
    if (ItemID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Validation failed: ItemID=%d"), ItemID);
        return false;
    }
    return true;
}
```

### Issue 4: Replication Lag

**Symptom**: UI updates feel delayed (> 100ms).

**Cause**: Low replication frequency or network congestion.

**Solution**:
```cpp
// Increase replication frequency for important components
Inventory->SetNetUpdateFrequency(30.0f); // 30 updates/second

// Use client-side prediction for UI
void UMyWidget::OnClientDragItem(...)
{
    // Optimistically update UI
    UpdateItemSlotVisuals_Immediate();
    
    // Send RPC to server
    PlayerController->Server_MoveItem(...);
    
    // Server will replicate back, correcting if prediction was wrong
}
```

### Issue 5: Component Created on Client

**Symptom**: Crashes or null pointers in multiplayer.

**Cause**: Components created on client (not replicated from server).

**Solution**:
```cpp
// WRONG: Creates component on client
AYourPlayerController::AYourPlayerController()
{
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
    // This runs on both server and client!
}

// CORRECT: Only create on server
AYourPlayerController::AYourPlayerController()
{
    if (HasAuthority()) // ← Add authority check
    {
        Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
        Inventory->SetIsReplicated(true);
        Inventory->SetNetAddressable();
    }
}
```

---

## Testing Multiplayer

### Local Testing (Editor)

**Method 1: Multiple PIE Clients**
1. Editor → Play → Net Mode: **Play As Listen Server**
2. Editor → Play → Number of Players: **2+**
3. Editor → Advanced Settings → Use Single Process: **Unchecked**

**Method 2: Dedicated Server + Client**
```powershell
# Terminal 1: Start dedicated server
YourProject.exe -server -log

# Terminal 2: Start client
YourProject.exe 127.0.0.1 -game -log
```

### Testing Checklist

- [ ] **Authority checks**: Verify `HasAuthority()` before mutations
- [ ] **RPC validation**: Test with invalid parameters
- [ ] **Replication delay**: Simulate network lag (Editor Settings → Emulate Lag)
- [ ] **Component nullity**: Check components exist on clients
- [ ] **Delegate firing**: Verify UI updates on all clients
- [ ] **Packet loss**: Test with simulated packet loss (5-10%)
- [ ] **High latency**: Test with 200ms+ ping

### Debug Commands

```cpp
// Show replication stats
ShowDebug Net

// Display inventory replication
ShowDebug InventoryComponent

// Log all RPCs
Log LogNet All

// Display client-side prediction errors
ShowDebug ClientSidePrediction
```

### Logging Best Practices

```cpp
// Good logging
UE_LOG(LogInventory, Verbose, TEXT("[%s] AddItem: ItemID=%d, Slot=%d, Index=%d"),
    HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
    ItemID, (int32)Slot, TopLeftIndex);

// Bad logging (no context)
UE_LOG(LogTemp, Warning, TEXT("Failed"));
```

---

## Advanced Topics

### Custom NetSerialize

For very large inventories, implement custom serialization:

```cpp
bool FFullInventoryStruct::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    // Custom serialization logic
    if (Ar.IsSaving())
    {
        // Write only changed bags (delta compression)
        int32 ChangedBagCount = ChangedBags.Num();
        Ar << ChangedBagCount;
        
        for (EBagSlot Slot : ChangedBags)
        {
            Ar << Slot;
            Ar << AllBags[Slot];
        }
    }
    else // Loading
    {
        int32 ChangedBagCount;
        Ar << ChangedBagCount;
        
        for (int32 i = 0; i < ChangedBagCount; ++i)
        {
            EBagSlot Slot;
            TArray<FMinimalItemStorage> BagContents;
            Ar << Slot;
            Ar << BagContents;
            AllBags.Add(Slot, BagContents);
        }
    }
    
    bOutSuccess = true;
    return true;
}
```

### Prediction & Rollback

For competitive games, implement client-side prediction:

```cpp
void UInventoryComponent::ClientPredictedMoveItem(...)
{
    // Store original state
    FScopedPredictionWindow PredictionScope(this);
    
    // Optimistically apply change
    MoveItem_Internal(...);
    
    // Send RPC
    Server_MoveItem(...);
    
    // If server rejects, state will rollback automatically
}
```

---

## Related Documentation

- [Component Architecture Guide](./Component_Architecture_Guide.md) - Component design patterns
- [Interface Implementation Guide](./Interface_Implementation_Guide.md) - Required interfaces
- [Integration Guide](./Integration_Guide.md) - Full setup walkthrough

---

*Last Updated: 2026-01-30*
