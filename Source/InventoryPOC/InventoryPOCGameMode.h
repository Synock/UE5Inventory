// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/GameModeBase.h>
#include "Inventory/BareItem.h"
#include "InventoryPOCGameMode.generated.h"

UCLASS(minimalapi)
class AInventoryPOCGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected :
	UPROPERTY(BlueprintReadWrite)
	TMap<int64, FBareItem> ItemMap;

public:
	AInventoryPOCGameMode();

	UFUNCTION(BlueprintCallable)
	void RegisterItem(const FBareItem& NewItem);

	UFUNCTION(BlueprintCallable)
	FBareItem FetchItemFromID(int64 ID) const;
};
