// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>
#include "ItemWidget.h"
#include "InventoryGridWidget.generated.h"

struct FBareItem;
class AInventoryPOCCharacter;

USTRUCT(BlueprintType)
struct FInventoryLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D Begin;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D End;
	FInventoryLine() = default;

	FInventoryLine(const FVector2D& InputBegin, const FVector2D& InputEnd)
	{
		Begin = InputBegin;
		End = InputEnd;
	}
};

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	float TileSize = 40;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	TArray<FInventoryLine> Lines;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	bool DrawDropLocation = false;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	int32 DraggedItemTopLeftID;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	BagSlot BagID = BagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 Width = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 Height = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize MaximumBagSize = EItemSize::Giant;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	bool CanAcceptDrop = true;

	//the Character that will display this
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TObjectPtr<AInventoryPOCCharacter> Character;

	//Actor owning the bag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Data")
	TObjectPtr<AActor> ActorOwner;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<UItemWidget*> ItemGrid;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<UItemWidget*> ItemList;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<UItemWidget*> ValidItemList;


public:
	UInventoryGridWidget(const FObjectInitializer& ObjectInitializer);
	void ResizeBagArea(int32 InputWidth, int32 InputHeight);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	void InitData(AActor* Owner, BagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	int GetWidth() const { return Width; }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	int GetHeight() const { return Height; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void SetUISize(float InputWidth, float InputHeight);

	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void GetPositionFromTopLeft(int32 TopLeft, float& PositionX, float& PositionY);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void CreateLineSegments();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void MousePositionInTile(float XPosition, float YPosition, bool& IsOnRight, bool& IsOnBottom) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	int32 GetActualTopLeftCorner(float XPosition, float YPosition, int32 ItemWidth, int32 ItemHeight) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	bool HasItemLocally(const FMinimalItemStorage& ItemData) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	UItemWidget* GetLocalItem(const FMinimalItemStorage& ItemData, bool& Found) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	void RegisterExistingItem(UItemWidget* ItemData);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	void SetCanAcceptDrop(bool Value) { CanAcceptDrop = Value; }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	void DeInitData();

protected:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	bool IsRoomAvailable(const FBareItem& ItemObject, int TopLeftIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	bool IsItself(UItemWidget* IncomingItem, int TopLeftIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void RegisterNewItem(int32 TopLeft, UItemWidget* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void UnRegisterItem(UItemWidget* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void PurgeItems();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	void GetXYCellFromFloatingPoint(float XPosition, float YPosition, int32& CellX, int32& CellY) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	int32 GetTopLeftFromCellXY(int32 CellX, int32 CellY) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	const TArray<FMinimalItemStorage>& GetItemData() const;
};
