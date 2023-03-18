// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>
#include "ItemWidget.h"
#include "Components/CanvasPanel.h"
#include "Items/InventoryItemBase.h"
#include "InventoryGridWidget.generated.h"

class IInventoryPlayerInterface;

USTRUCT(BlueprintType)
struct FInventoryLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D Begin{0.f, 0.f};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D End{0.f, 0.f};
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
class INVENTORYPLUGIN_API UInventoryGridWidget : public UUserWidget
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
	EBagSlot BagID = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 Width = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 Height = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize MaximumBagSize = EItemSize::Giant;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	bool CanAcceptDrop = true;

	//Actor owning the bag
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TObjectPtr<AActor> ActorOwner = GetOwningPlayerPawn();

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<UItemWidget*> ItemGrid;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<UItemWidget*> ItemList;

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Data")
	TSubclassOf<UItemWidget> ItemWidgetClass = UItemWidget::StaticClass();

public:
	UInventoryGridWidget(const FObjectInitializer& ObjectInitializer);
	void ResizeBagArea(int32 InputWidth, int32 InputHeight);

	UFUNCTION()
	void ResetTransaction();


	/**
	 * @brief Init the grid data for the inventory.
	 * @param Owner Owning actor
	 * @param InputBagSlot Considered bag slot
	 * @param InputWidth if the bag is not an item, the grid width may be overriden by this value
	 * @param InputHeight if the bag is not an item, the grid height may be overriden by this value
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	void InitData(AActor* Owner, EBagSlot InputBagSlot, int32 InputWidth  = -1, int32 InputHeight = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Data")
	EBagSlot GetBagID() const { return BagID; }

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

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void UnRegisterItem(UItemWidget* NewItem);

protected:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	bool IsRoomAvailable(const UInventoryItemBase* ItemObject, int TopLeftIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	bool IsItself(UItemWidget* IncomingItem, int TopLeftIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void RegisterNewItem(int32 TopLeft, UItemWidget* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	void GetXYCellFromFloatingPoint(float XPosition, float YPosition, int32& CellX, int32& CellY) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	int32 GetTopLeftFromCellXY(int32 CellX, int32 CellY) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	const TArray<FMinimalItemStorage>& GetItemData() const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	FVector2D GetItemScreenFootprint(UItemWidget* Item) const;

	bool CanProcessItemDrop(UItemWidget* IncomingItem) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI|Helper")
	bool HandleItemDrop(UItemWidget* IncomingItem);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Helper")
	bool UpdateDraggedItemTopLeft(UItemWidget* IncomingItem, float X, float Y);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void AddItemWidgetToGrid(UCanvasPanel* GridCanvasPanel, UWidget* Content, int32 TopLeft);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void CreateNewItem(UCanvasPanel* GridCanvasPanel, const FMinimalItemStorage& ItemStorage);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void FullRefresh(UCanvasPanel* GridCanvasPanel);
};
