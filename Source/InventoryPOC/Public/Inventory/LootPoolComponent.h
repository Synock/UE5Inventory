// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "BareItem.h"
#include "LootPoolComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootPoolChangedDelegate);

///@brief
/// This class contains a loot pool with items for the player.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPOC_API ULootPoolComponent : public UActorComponent
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|LootPool")
	BagSlot LocalBagSlot = BagSlot::LootPool;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|LootPool")
	FOnLootPoolChangedDelegate LootPoolDispatcher;

	UFUNCTION()
	void OnRep_LootPool();

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|LootPool")
	void AddItem(int64 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|LootPool")
	void RemoveItem(int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|LootPool")
	void Init(const TArray<int64>& LootableItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	int64 GetItemAtIndex(int32 ID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootPool")
	const TArray<FMinimalItemStorage>& GetBagConst() const { return Items; }
};
