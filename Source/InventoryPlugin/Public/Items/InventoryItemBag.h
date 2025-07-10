// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEquipable.h"
#include "Interfaces/InventoryItemBagInterface.h"
#include "InventoryItemBag.generated.h"


UCLASS()
class INVENTORYPLUGIN_API UInventoryItemBag : public UInventoryItemEquipable, public IInventoryItemBagInterface
{
public:
	GENERATED_BODY()

	/// @brief If true, this item is a bag and can hold other items.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	bool Bag = false;

	/// @brief The size of the bag, which determines how big items it can hold.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize BagSize = EItemSize::Giant;

	/// @brief The width of the bag in grid units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagWidth = 1;

	/// @brief The height of the bag in grid units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagHeight = 1;

	/// @brief The weight reduction factor for the bag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	float WeightReduction = 0.f; // 0.0 is no reduction, 1.0 is full reduction (no weight)

	virtual bool IsBag() const override { return Bag; }
	virtual EItemSize GetBagSize() const override { return BagSize; }
	virtual uint8 GetBagWidth() const override { return BagWidth; }
	virtual uint8 GetBagHeight() const override { return BagHeight; }
	virtual float GetWeightReduction() const override { return WeightReduction; }
	
	virtual UStaticMesh* GetStandardMesh() const override {return Mesh;}

};
