// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "InventoryItem.h"
#include "LootPoolComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootPoolChangedDelegate);

///@brief
/// This class contains a loot pool with items for the player.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API ULootPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULootPoolComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_LootPool, BlueprintReadWrite, Category = "Inventory|LootPool")
	TArray<FMinimalItemStorage> Items;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|LootPool")
	EBagSlot LocalBagSlot = EBagSlot::LootPool;

	uint32 Width = 8;
	uint32 Height = 16;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|LootPool")
	FOnLootPoolChangedDelegate LootPoolDispatcher;

	UFUNCTION()
	void OnRep_LootPool();

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|LootPool")
	void AddItem(int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|LootPool")
	void RemoveItem(int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	void Init(const TArray<int32>& LootableItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	int32 GetItemAtIndex(int32 ID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	const TArray<FMinimalItemStorage>& GetBagConst() const { return Items; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	bool IsEmpty() const { return Items.IsEmpty(); }

	/// Try to refrain using this functions, as it rebuilds the solver from scratch every time.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory|LootPool")
	bool AddItemSomewhere(int32 ItemID);
};
