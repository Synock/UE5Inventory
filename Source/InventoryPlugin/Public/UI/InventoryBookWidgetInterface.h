#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/Interfaces/InventoryItemBookInterface.h"
#include "InventoryBookWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryBookWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any UUserWidget subclass that can display a readable book item.
 *
 * Implement this interface on custom book widgets to make them compatible with
 * IInventoryHUDInterface::DisplayBookText's default C++ flow without inheriting
 * from UInventoryBookWidget.
 *
 * Required Blueprint widget bindings:
 *   - "TextBlock"  (URichTextBlock, mandatory) — populated by SetText
 *   - "TitleBlock" (UTextBlock, optional)      — populated by SetTitle
 */
class INVENTORYPLUGIN_API IInventoryBookWidgetInterface
{
	GENERATED_BODY()

public:
	/** Called after widget creation for visual/animation setup. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book|UI")
	void SetupUI();
	virtual void SetupUI_Implementation() {}

	/** Set the main body text of the book. Supports rich text markup. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book|UI")
	void SetText(const FText& Content);
	virtual void SetText_Implementation(const FText& Content) {}

	/** Set the title text of the book. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book|UI")
	void SetTitle(const FText& Title);
	virtual void SetTitle_Implementation(const FText& Title) {}

	/**
	 * Load all pages and display from the beginning.
	 * Single-page books call this with a one-element array.
	 * Implementations show one page at a time (UInventoryBookWidget)
	 * or a two-page spread (UBookWindow) and wire navigation buttons accordingly.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book|UI")
	void SetPages(const TArray<FBookPage>& Pages);
	virtual void SetPages_Implementation(const TArray<FBookPage>& Pages) {}
};
