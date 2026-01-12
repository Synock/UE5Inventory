// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "ItemBaseWidget.generated.h"

class UInventoryItemBase;

/**
 * Base widget for displaying inventory items with drag-drop support and durability tracking.
 * Uses modern BindWidget pattern for automatic Blueprint widget binding.
 * Supports right-click context menus and left-click interactions.
 */
UCLASS()
class INVENTORYPLUGIN_API UItemBaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ============================================================================
	// Item Data
	// ============================================================================

	/** The actor that owns this item widget */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	TObjectPtr<AActor> Owner = nullptr;

	/** Reference to the inventory item this widget represents */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	TObjectPtr<const UInventoryItemBase> Item = nullptr;

	/** Current durability of the item */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item|Durability")
	float Durability = 100.0f;

	/** Maximum durability of the item */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item|Durability")
	float MaxDurability = 100.0f;

	// ==========================================================================
	// Client-side lock to prevent local interactions for reserved/in-use items
	// ==========================================================================

	/** If true the widget is locked on client-side (disables drag/equip/sell/activation) */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item|Lock")
	bool bIsLocked = false;

	/** Blueprint event to update visuals when lock state changes (lock icon/tooltip) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Item|Lock")
	void OnLockStateChanged(bool bLocked);
	virtual void OnLockStateChanged_Implementation(bool bLocked);

	/**
	 * Notify owner/UI that an interaction was blocked (e.g., item locked).
	 * Plugin code raises this event; game code implements it (show chat, toast, etc.).
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory|Item|Events")
	void NotifyInteractionBlocked(const FText& Message);


	// ============================================================================
	// UI Components (BindWidget)
	// ============================================================================

	/** The image widget displaying the item icon - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI", Meta = (BindWidget))
	TObjectPtr<UImage> ItemImage = nullptr;

	/** The background border for styling - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI", Meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder = nullptr;

	/** The size box controlling widget dimensions - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI", Meta = (BindWidget))
	TObjectPtr<USizeBox> BackgroundSizeBox = nullptr;

	// ============================================================================
	// UI Configuration
	// ============================================================================

	/** Size of the tile in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI")
	float TileSize = 40.f;

	/** Maximum duration for right-click to trigger context menu instead of long press */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|UI|Click")
	float RightClickMaxDuration = 0.5f;

	// ============================================================================
	// Click State
	// ============================================================================

	/** Whether a right-click is currently in progress */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI|Click")
	bool IsRightClicking = false;

	/** Cached pointer event from the last click */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI|Click")
	FPointerEvent ClickEvent;

	/** Timer handle for right-click duration tracking */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI|Click")
	FTimerHandle RightClickTimerHandle;


	// ============================================================================
	// Timer & Click Handlers
	// ============================================================================

	/** Called during widget construction to set up bindings and backwards compatibility */
	virtual void NativePreConstruct() override;

	/** Called when right-click timer expires to show item description */
	UFUNCTION()
	void RightClickTimerFunction();

	/** Blueprint event for long right-click (context menu) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Events")
	bool RightClickLongEffect();

	/** Blueprint event for short right-click */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|UI|Events")
	bool RightClickShortEffect();
	virtual bool RightClickShortEffect_Implementation();

	/** Blueprint event for left-click */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Events")
	bool LeftClickEffect();

	/** Blueprint event to set up the UI on initialization */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Events")
	void SetupUI();

	// ============================================================================
	// Native Input Overrides
	// ============================================================================

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// ============================================================================
	// Display Functions
	// ============================================================================

	/** Display the item description tooltip at the specified screen position */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI|Display")
	void DisplayDescription(const FPointerEvent& InMouseEvent);

	/** Display book text for readable items */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI|Display")
	void DisplayBookText(const FPointerEvent& InMouseEvent);

	/** Reset the sell item state in trading interface */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI|Display")
	void ResetSell();

	/** Update the item image from the referenced item's icon */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI|Display")
	void UpdateItemImage();

	virtual void RefreshInternal();
public:
	// ============================================================================
	// Public Interface
	// ============================================================================

	/**
	 * Initialize the widget with item data
	 * @param InputItem - The inventory item to display
	 * @param InputOwner - The actor that owns this item
	 * @param InputTileSize - Size of the tile in pixels
	 * @param InputDurability - Current durability of the item
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Item")
	void InitBareData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize, float InputDurability = 100.0f);

	/** Stop any active drag operation */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|DragDrop")
	virtual void StopDrag();

	/** Get the inventory item this widget represents */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Inventory|Item")
	const UInventoryItemBase* GetReferencedItem() const { return Item; }

	/** Get the current durability of the item */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Inventory|Item|Durability")
	float GetDurability() const { return Durability; }

	/** Get the maximum durability of the item */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "Inventory|Item|Durability")
	float GetMaxDurability() const { return MaxDurability; }

	/** Set the client-side lock state */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Item|Lock")
	void SetLocked(bool bLocked);

	/** Query the client-side lock state */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Item|Lock")
	bool IsLocked() const { return bIsLocked; }

	/** Blueprint event to refresh the widget's visual state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI|Events")
	void Refresh();
};

