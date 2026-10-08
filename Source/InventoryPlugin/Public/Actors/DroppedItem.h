#pragma once

#include "CoreMinimal.h"
#include "AbstractDroppedItem.h"
#include "GameFramework/Actor.h"
#include "DroppedItem.generated.h"

class UInventoryItemBase;

UCLASS()
class INVENTORYPLUGIN_API ADroppedItem : public AAbstractDroppedItem
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 ItemID = 0;

	// Durability is only used on server during pickup, no need to replicate
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Durability")
	float Durability = 100.0f;

public:

	ADroppedItem();

	virtual void InitializeFromItem(UInventoryItemBase* Item, bool AllowToRotate = true, const FRotator& SpawningActorRotation = FRotator::ZeroRotator);

	virtual void InitializeFromItemWithDurability(UInventoryItemBase* Item, float InDurability, bool AllowToRotate = true, const FRotator& SpawningActorRotation = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	float GetDurability() const { return Durability; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	void SetDurability(float InDurability) { Durability = InDurability; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemID() const { return ItemID; }



};
