// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Actors/LootableActor.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/BareItem.h"
#include "LootScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API ULootScreenWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	FString LootName;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagWidth = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagHeight = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	BagSlot CurrentBagSlot = BagSlot::LootPool;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	ALootableActor* LootedActor = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void DeInitUI();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitLootData(ALootableActor* InputLootedActor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitUI();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void DeInitLootData();

};
