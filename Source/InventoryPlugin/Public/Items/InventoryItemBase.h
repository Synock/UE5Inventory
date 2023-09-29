// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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

class UTexture2D;
class UStaticMesh;
/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemBase : public UPrimaryDataAsset
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
	
};
