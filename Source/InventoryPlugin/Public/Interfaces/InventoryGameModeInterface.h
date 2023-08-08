// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "InventoryGameModeInterface.generated.h"

struct FCoinValue;
class ADroppedCoins;
class ADroppedItem;
class UInventoryItemBase;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IInventoryGameModeInterface
{
	GENERATED_BODY()

public:

	virtual ADroppedItem* SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID, const FVector& DesiredDropLocation, bool ClampOnGround = true) = 0;

	virtual ADroppedCoins* SpawnCoinsFromActor(AActor* SpawningActor, const FCoinValue& CoinValue, const FVector& DesiredDropLocation, bool ClampOnGround = true) = 0;

	virtual FVector GetItemSpawnLocation(AActor* SpawningActor,const FVector& DesiredDropLocation, bool ClampOnGround = true);

	UFUNCTION(BlueprintCallable)
	virtual UInventoryItemBase* FetchItemFromID(int32 ID) = 0;

	UFUNCTION(BlueprintCallable)
	virtual void RegisterItem(UInventoryItemBase* NewItem) = 0;
	
};
