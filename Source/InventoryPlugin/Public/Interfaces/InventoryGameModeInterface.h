// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "InventoryGameModeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API IInventoryGameModeInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void SpawnItemFromActor(AActor* Actor, uint32 ItemID, bool ClampOnGround = true);

	UFUNCTION(BlueprintCallable)
	virtual FInventoryItem FetchItemFromID(int32 ID) = 0;

	UFUNCTION(BlueprintCallable)
	virtual void RegisterItem(const FInventoryItem& NewItem) = 0;
	
};
