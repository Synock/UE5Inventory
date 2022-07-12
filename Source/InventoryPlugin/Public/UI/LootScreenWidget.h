// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.h"
#include "Interfaces/LootableInterface.h"
#include "LootScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API ULootScreenWidget : public UUserWidget
{
	GENERATED_BODY()
protected:

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* Parent = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	FString LootName;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagWidth = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagHeight = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	EBagSlot CurrentBagSlot = EBagSlot::LootPool;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Loot")
	TScriptInterface<ILootableInterface> LootedActor = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void DeInitUI();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitLootData(AActor* InputLootedActor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitUI();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void DeInitLootData();
};
