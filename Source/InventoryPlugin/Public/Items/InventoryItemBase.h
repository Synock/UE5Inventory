// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interfaces/InventoryItemInterface.h"
#include "InventoryItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemSize : uint8
{
	Tiny UMETA(DisplayName = "Tiny"),
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large"),
	Giant UMETA(DisplayName = "Giant")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Unknown = 0 UMETA(DisplayName = "Unknown"),
};

USTRUCT(BlueprintType)
struct FMaterialOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	UMaterialInstance* OverrideMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	int32 MaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	float TintIntensity = 1.0f;
};

class UTexture2D;
class UStaticMesh;
/**
 * Base class for all inventory items in the plugin.
 * Implements IInventoryItemInterface to provide standard access patterns.
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemBase : public UPrimaryDataAsset, public IInventoryItemInterface
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	uint8 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	uint8 Height = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Visual")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Visual")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Visual")
	FMaterialOverride OverrideMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	EItemSize ItemSize = EItemSize::Tiny;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	bool LoreItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	bool MagicItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	bool Temporary = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|ItemData")
	float Weight = 0.f;

	//------------------------------------------------------------------------------------------------------------------
	// IInventoryItemInterface Implementation
	//------------------------------------------------------------------------------------------------------------------

	virtual int32 GetItemID() const override { return ItemID; }
	virtual FString GetItemName() const override { return Name; }
	virtual FString GetItemDescription() const override { return Description; }

	virtual UTexture2D* GetIcon() const override { return Icon; }
	virtual UStaticMesh* GetMesh() const override { return Mesh; }
	virtual const FMaterialOverride& GetMaterialOverride() const override { return OverrideMaterial; }

	virtual float GetWeight() const override { return Weight; }
	virtual float GetBaseValue() const override { return BaseValue; }
	virtual uint8 GetWidth() const override { return Width; }
	virtual uint8 GetHeight() const override { return Height; }
	virtual EItemSize GetItemSize() const override { return ItemSize; }

	virtual bool IsLoreItem() const override { return LoreItem; }
	virtual bool IsMagicItem() const override { return MagicItem; }
	virtual bool IsTemporary() const override { return Temporary; }
};
