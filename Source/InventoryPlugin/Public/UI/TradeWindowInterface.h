#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TradeWindowInterface.generated.h"

class UTradeComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UTradeWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for the player-to-player trade window widget.
 *
 * Implemented by UTradeWindow (game side).
 * IInventoryHUDInterface's C++ defaults dispatch all trade window lifecycle
 * calls through this interface, keeping HUD logic game-agnostic.
 *
 * The implementing widget must bind a UTradeWidget (or equivalent) as a BindWidget
 * member and delegate InitTradeWindow / DeInitTradeWindow to it.
 */
class INVENTORYPLUGIN_API ITradeWindowInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate the trade window with data from the given trade component and make it visible.
	 * Called by IInventoryHUDInterface::OpenTradeWindow_Implementation after IsTrading() check.
	 * @param InTradeComponent The local player's trade component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Trade")
	void InitTradeWindow(UTradeComponent* InTradeComponent);
	virtual void InitTradeWindow_Implementation(UTradeComponent* InTradeComponent) {}

	/**
	 * Release trade data and unbind delegates.
	 * Called by IInventoryHUDInterface::CloseTradeWindow_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Trade")
	void DeInitTradeWindow();
	virtual void DeInitTradeWindow_Implementation() {}

	/** Make the trade window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Trade")
	void ShowTradeWindow();
	virtual void ShowTradeWindow_Implementation() {}

	/** Hide the trade window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Trade")
	void HideTradeWindow();
	virtual void HideTradeWindow_Implementation() {}

	/** Returns true if the trade window is currently visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Trade")
	bool IsTradeWindowVisible() const;
	virtual bool IsTradeWindowVisible_Implementation() const { return false; }
};

