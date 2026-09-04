#pragma once

#include "CoreMinimal.h"
#include "UI/Merchant/CoinDisplayWidget.h"
#include "PurseWidget.generated.h"

/**
 * @class UPurseWidget
 *
 * Widget for displaying a player's or merchant's purse (coin inventory).
 * Inherits from CoinDisplayWidget to get automatic coin display with icons.
 * Automatically syncs with a UCoinComponent and updates when coins change.
 */
UCLASS()
class INVENTORYPLUGIN_API UPurseWidget : public UCoinDisplayWidget
{
	GENERATED_BODY()

protected:
	/** Reference to the coin component this widget is displaying */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Purse")
	class UCoinComponent* PursePointer = nullptr;

public:
	/**
	 * @brief Refresh the display from the coin component
	 * Called automatically when coin component changes
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void Refresh();

	/**
	 * @brief Initialize the widget with a coin component
	 * Binds to the component's PurseDispatcher for automatic updates
	 * @param Owner The coin component to display
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void InitWidget(UCoinComponent* Owner);

	/**
	 * @brief Get the current purse pointer
	 * @return The coin component this widget is displaying
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|UI")
	UCoinComponent* GetPursePointer() const { return PursePointer; }
};
