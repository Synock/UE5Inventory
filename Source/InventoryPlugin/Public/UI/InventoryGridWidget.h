#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>
#include "ItemWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Slate/SlateBrushAsset.h"
#include "Items/InventoryItemBase.h"
#include "InventoryGridWidget.generated.h"

class IInventoryPlayerInterface;
class UDragDropOperation;

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
	/** Size of each inventory tile in pixels. Set this in Blueprint class defaults to control the grid cell size. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|UI")
	float TileSize = 40.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	TArray<FInventoryLine> Lines;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	bool DrawDropLocation = false;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	int32 DraggedItemTopLeftID = INDEX_NONE;

	// ============================================================================
	// BindWidget — must match widget names in the Blueprint hierarchy exactly.
	// ============================================================================

	/** Border widget framing the grid — automatically bound from Blueprint. */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI", Meta = (BindWidget))
	TObjectPtr<UBorder> GridBorder = nullptr;

	/** Canvas panel that holds item widgets — automatically bound from Blueprint. */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI", Meta = (BindWidget))
	TObjectPtr<UCanvasPanel> GridCanvasPanel = nullptr;

	// ============================================================================
	// Designer-tunable paint properties
	// ============================================================================

	/** Color of the grid lines drawn over the bag area. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|UI")
	FLinearColor GridLineColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);

	/** Tint used for the drop-highlight box when the drop position is valid. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|UI")
	FLinearColor DropColorValid = FLinearColor(0.f, 1.f, 0.f, 0.02f);

	/** Tint used for the drop-highlight box when the drop position is invalid. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|UI")
	FLinearColor DropColorInvalid = FLinearColor(1.f, 0.f, 0.f, 0.02f);

	/** Solid-color brush used to draw the drop-highlight box. Assign SB_Color in Blueprint class defaults. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|UI")
	TObjectPtr<USlateBrushAsset> DropHighlightBrush = nullptr;

	/**
	 * The bag slot this grid represents. Set in Blueprint class defaults so the widget
	 * auto-initializes when added to the viewport (no manual InitData call needed for player bags).
	 * LootPool bags must still call InitData() explicitly with the lootable actor as owner.
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Bag")
	EBagSlot BagID = EBagSlot::Unknown;

	/**
	 * Default number of columns shown in the designer preview and used as a fallback
	 * when the runtime bag has not been initialized yet. InitData() will override this
	 * with the actual bag dimensions at runtime.
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Bag")
	int32 Width = 4;

	/**
	 * Default number of rows shown in the designer preview and used as a fallback
	 * when the runtime bag has not been initialized yet. InitData() will override this
	 * with the actual bag dimensions at runtime.
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Bag")
	int32 Height = 4;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize MaximumBagSize = EItemSize::Giant;

	/// In case of a quiver, this will limit the ammo type that can be stored in the bag
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EAmmoType AmmoTypeLimiter = EAmmoType::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	bool CanAcceptDrop = true;

	//Actor owning the bag
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TObjectPtr<AActor> ActorOwner = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<TObjectPtr<UItemWidget>> ItemGrid;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Data")
	TArray<TObjectPtr<UItemWidget>> ItemList;

	// Fast lookup map for item retrieval (optimization)
	UPROPERTY(Transient)
	TMap<int64, TObjectPtr<UItemWidget>> ItemLookupMap;

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Inventory|Data")
	TSubclassOf<UItemWidget> ItemWidgetClass = UItemWidget::StaticClass();

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	// ============================================================================
	// Native drag-drop and paint overrides (replace Blueprint event graph)
	// ============================================================================

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** Draws the drag-drop highlight box. Called from NativePaint. */
	void DrawBackground(FPaintContext& Context, const FVector2D& LocalTopLeft) const;

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
	void InitData(AActor* Owner, EBagSlot InputBagSlot, int32 InputWidth = -1, int32 InputHeight = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Data")
	EBagSlot GetBagID() const { return BagID; }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	int32 GetWidth() const { return Width; }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Data")
	int32 GetHeight() const { return Height; }

	/**
	 * Rebuilds the widget from the authoritative item data. C++ default calls FullRefresh();
	 * Blueprint subclasses may override for custom behavior.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
	virtual void Refresh_Implementation();

	/**
	 * Resizes the GridBorder canvas slot. C++ default resizes via UCanvasPanelSlot;
	 * Blueprint subclasses may override for custom layout.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void SetUISize(float InputWidth, float InputHeight);
	virtual void SetUISize_Implementation(float InputWidth, float InputHeight);

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
	UItemWidget* GetItemWidgetAtPosition(int32 TopLeft) const;

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
	bool IsRoomAvailable(const UInventoryItemBase* ItemObject, int32 TopLeftIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	bool IsItself(UItemWidget* IncomingItem, int32 TopLeftIndex) const;

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

	/** Adds Content to the bound GridCanvasPanel at the grid position for TopLeft. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void AddItemWidgetToGrid(UWidget* Content, int32 TopLeft);

	/** Creates and registers a new item widget inside the bound GridCanvasPanel. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void CreateNewItem(const FMinimalItemStorage& ItemStorage);

	/** Clears and repopulates the bound GridCanvasPanel from authoritative item data. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void FullRefresh();

private:
	// Helper methods for validation and safety
	bool IsValidGridIndex(int32 Index) const;
	bool IsValidCoordinate(int32 X, int32 Y) const;
	bool IsWithinGridBounds(int32 TopLeftIndex, int32 ItemWidth, int32 ItemHeight) const;
	void ClearItemLookupMap();

	/**
	 * Collision-safe 64-bit composite key: upper 32 bits = TopLeft, lower 32 bits = ItemID.
	 */
	static int64 MakeItemKey(int32 TopLeft, int32 ItemID)
	{
		return (static_cast<int64>(TopLeft) << 32) | static_cast<uint32>(ItemID);
	}
};
