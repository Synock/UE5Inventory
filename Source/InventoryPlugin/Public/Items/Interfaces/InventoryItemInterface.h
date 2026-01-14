// Copyright 2025 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemInterface.generated.h"

enum class EItemSize : uint8;
struct FMaterialOverride;
class UTexture2D;
class UStaticMesh;

// This class does not need to be modified.
UINTERFACE()
class UInventoryItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Base interface for all inventory items.
 * Provides consistent access to item properties for plugin users.
 * Implement this interface to create custom item types with full plugin compatibility.
 */
class INVENTORYPLUGIN_API IInventoryItemInterface
{
	GENERATED_BODY()

public:
	// Core identification
	virtual int32 GetItemID() const = 0;
	virtual FString GetItemName() const = 0;
	virtual FString GetItemDescription() const = 0;

	// Visual properties
	virtual class UTexture2D* GetIcon() const = 0;
	virtual class UStaticMesh* GetMesh() const = 0;
	virtual const struct FMaterialOverride& GetMaterialOverride() const = 0;

	// Physical properties
	virtual float GetWeight() const = 0;
	virtual float GetBaseValue() const = 0;
	virtual uint8 GetWidth() const = 0;
	virtual uint8 GetHeight() const = 0;
	virtual EItemSize GetItemSize() const = 0;

	/// Item type bitmask for game-specific categorization (default: 0)
	/// This is intended to be overridden by game-specific item classes
	virtual int64 GetItemTypeBitMask() const { return 0;}


	// Item flags
	virtual bool IsLoreItem() const = 0;
	virtual bool IsMagicItem() const = 0;
	virtual bool IsTemporary() const = 0;

};
