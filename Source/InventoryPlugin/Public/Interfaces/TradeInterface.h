#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TradeInterface.generated.h"

class UTradeComponent;
class UCoinComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UTradeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class ITradeInterface
 *
 * Interface for actors that can participate in player-to-player trading.
 * Implemented by player controllers to enable direct item and coin exchanges.
 */
class INVENTORYPLUGIN_API ITradeInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets the trade component for this actor.
	 * @return Pointer to the trade component, or nullptr if not available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual UTradeComponent* GetTradeComponent() = 0;

	/**
	 * @brief Gets the const trade component for this actor.
	 * @return Const pointer to the trade component, or nullptr if not available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual const UTradeComponent* GetTradeComponentConst() const = 0;

	/**
	 * @brief Gets the coin component for trading.
	 * @return Pointer to the coin component for currency exchange.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual UCoinComponent* GetCoinComponent() = 0;

	/**
	 * @brief Gets the name of the trading partner.
	 * @return Display name of the trader.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual FString GetTraderName() const = 0;

	/**
	 * @brief Checks if this actor can currently trade.
	 * @return True if able to trade, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual bool CanTrade() const = 0;

	/**
	 * @brief Gets the world context for this trader.
	 * @return Pointer to the world object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual UWorld* GetTradeWorldContext() const = 0;
};

