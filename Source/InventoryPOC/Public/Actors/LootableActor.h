// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include "Inventory/PurseComponent.h"
#include "Inventory/LootPoolComponent.h"
#include "LootableActor.generated.h"

class AInventoryPOCCharacter;

///@brief Base class for Lootable actors
UCLASS()
class INVENTORYPOC_API ALootableActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALootableActor();

protected:
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	//Contains the lootable items.
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Lootable")
	TObjectPtr<ULootPoolComponent> LootPool;

	//Contains the lootable coins.
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Lootable")
	TObjectPtr<UPurseComponent> CashContent_loot;

	//Boolean to know if the actor is being looted, as only one person can loot items at a time.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Lootable")
	bool IsBeingLooted = false;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lootable")
	void InitLootPool(const TArray<int64>& LootableItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lootable")
	void InitCashPool(const FCoinValue& CoinValue);

public:
	void StartLooting(AInventoryPOCCharacter* Looter);
	void StopLooting(AInventoryPOCCharacter* Looter);
	ULootPoolComponent* GetLootPool();
	UPurseComponent* GetPurse();

	bool GetIsBeingLooted() const { return IsBeingLooted; }

	int64 GetItemData(int32 TopLeftID) const;

	void RemoveItem(int32 TopLeftID) const;

	FOnLootPoolChangedDelegate& GetLootPoolDeletate() { return LootPool->LootPoolDispatcher; }
};
