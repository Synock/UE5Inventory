// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryGameInstanceInterface.generated.h"

class UInventoryItemBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryGameInstanceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryGameInstanceInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual UInventoryItemBase* FetchItemFromID(int32 ID) = 0;

	UFUNCTION(BlueprintCallable)
	virtual void RegisterItem(UInventoryItemBase* NewItem) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Coin Display Icon Textures (Optional - for project-specific customization)
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Get the texture to use for copper coin icons
	 * @return Copper coin texture, or nullptr to use widget's configured texture
	 */
	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetCopperCoinIconTexture() const { return nullptr; }

	/**
	 * @brief Get the texture to use for silver coin icons
	 * @return Silver coin texture, or nullptr to use widget's configured texture
	 */
	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetSilverCoinIconTexture() const { return nullptr; }

	/**
	 * @brief Get the texture to use for gold coin icons
	 * @return Gold coin texture, or nullptr to use widget's configured texture
	 */
	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetGoldCoinIconTexture() const { return nullptr; }

	/**
	 * @brief Get the texture to use for platinum coin icons
	 * @return Platinum coin texture, or nullptr to use widget's configured texture
	 */
	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetPlatinumCoinIconTexture() const { return nullptr; }
};
