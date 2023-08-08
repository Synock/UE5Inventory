// Copyright 2023 Maximilien (Synock) Guislain

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

public:

	ADroppedItem();

	virtual void InitializeFromItem(UInventoryItemBase* Item);

};
