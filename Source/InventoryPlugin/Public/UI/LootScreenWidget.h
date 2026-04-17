#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.h"
#include "Interfaces/LootableInterface.h"
#include "UI/InventoryLootWindowInterface.h"
#include "LootScreenWidget.generated.h"

class UButton;
class UScrollBox;
class USizeBox;
class UInventoryGridWidget;

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API ULootScreenWidget : public UUserWidget, public IInventoryLootWindowInterface
{
	GENERATED_BODY()
protected:

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	UUserWidget* Parent = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	FString LootName;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagWidth = 8;

	/**
	 * Maximum number of rows the loot grid can grow to.
	 * Matches the LootPoolComponent's hard cap (16). Reduce to artificially limit grid growth.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Loot")
	int32 BagHeight = 16;

	/**
	 * Number of grid rows visible in the scroll box before scrolling is required.
	 * The scroll box (LootScrollSizeBox) is sized to MaxVisibleRows * TileSize pixels.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Loot|UI")
	int32 MaxVisibleRows = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Loot")
	EBagSlot CurrentBagSlot = EBagSlot::LootPool;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Loot")
	TScriptInterface<ILootableInterface> LootedActor = nullptr;

	/**
	 * Optional: bind a UInventoryGridWidget named "LootGrid" in the Blueprint
	 * to get automatic C++ initialization from InitUI_Implementation.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Loot|UI")
	UInventoryGridWidget* LootGrid = nullptr;

	/**
	 * Optional: bind a USizeBox named "LootScrollSizeBox" that wraps the scroll box.
	 * C++ sets its MaxDesiredHeight to MaxVisibleRows * TileSize so the panel never
	 * overextends the screen regardless of how many loot rows are present.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Loot|UI")
	USizeBox* LootScrollSizeBox = nullptr;

	/**
	 * Optional: bind a UScrollBox named "LootScrollBox" that contains the LootGrid.
	 * C++ resets the scroll position to the top whenever the loot panel is opened.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Loot|UI")
	UScrollBox* LootScrollBox = nullptr;

	/** Optional close button. When clicked, calls StopLooting on the owning player. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Loot|UI")
	UButton* CloseButton = nullptr;

	/** Optional loot-all button. When clicked, calls PlayerAutoLootAll then StopLooting on the owning player. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Loot|UI")
	UButton* LootAllButton = nullptr;

	/**
	 * Called after InitLootData to set up the visual layout.
	 * C++ default initializes LootGrid if bound; Blueprint may override for extra setup.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitUI();
	virtual void InitUI_Implementation();

	/**
	 * Refreshes loot display. C++ default calls LootGrid->Refresh() if bound.
	 * Blueprint may override to add visual feedback.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void Refresh();
	virtual void Refresh_Implementation();

	/**
	 * Tears down loot display. C++ default calls LootGrid->DeInitData() if bound.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void DeInitUI();
	virtual void DeInitUI_Implementation();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitLootData(AActor* InputLootedActor);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void DeInitLootData();

	// ---- IInventoryLootWindowInterface --------------------------------------
	virtual void InitLootWindow_Implementation(AActor* LootedActor) override;
	virtual void DeInitLootWindow_Implementation() override;
	virtual void ShowLootWindow_Implementation() override;
	virtual void HideLootWindow_Implementation() override;
	virtual void RefreshLootWindow_Implementation() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnLootAllButtonClicked();

	/** Computes the minimum grid height that fits all current loot pool items, clamped to [MaxVisibleRows, BagHeight]. */
	int32 ComputeRequiredGridHeight() const;

	/** Called whenever the loot pool changes while the widget is open. Resizes the grid and scroll box. */
	UFUNCTION()
	void OnLootPoolChanged();
};
