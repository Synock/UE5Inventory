// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>
#include "CoinValue.h"
#include "InventoryItem.h"

#include "MerchantInterface.generated.h"

class FOnMerchantDynamicPoolChangedDelegate;
class UCoinComponent;
class UMerchantComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMerchantInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IMerchantInterface
 *
 * This is the interface class for merchants in the inventory system.
 * Any class that implements this interface will have to provide the necessary functionality
 * to interact with the merchant's inventory and perform transactions.
 */
class INVENTORYPLUGIN_API IMerchantInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Retrieves the world context associated with the merchant.
	 *
	 * This method returns the world context associated with the merchant. It is used to access the
	 * world object in which the merchant is present.
	 *
	 * @return A pointer to the UWorld object representing the world context, if available, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UWorld* GetMerchantWorldContext() const = 0;

	/**
	 * @brief Retrieves the name of the merchant.
	 *
	 * @return FString The name of the merchant.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FString GetMerchantName() const = 0;

	/**
	 * @brief Retrieves the merchant ratio.
	 *
	 * This function returns the ratio value of the merchant. The ratio is a floating point number that represents
	 * the exchange rate or price factor used in merchant trading.
	 *
	 * @return The merchant ratio as a floating point value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual float GetMerchantRatio() const = 0;

	/**
	 * Retrieves the Coin Component associated with the inventory merchant.
	 *
	 * @return The Coin Component instance.
	 *
	 * @see UCoinComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UCoinComponent* GetCoinComponent() = 0;

	/**
	 * Get the merchant component associated with this object.
	 *
	 * @return The merchant component, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UMerchantComponent* GetMerchantComponent() = 0;

	/**
	 * @brief Retrieves the const pointer to the Coin Component associated with this object.
	 *
	 * This function returns the const pointer to the Coin Component associated with this object.
	 *
	 * @return The const pointer to the Coin Component.
	 *
	 * @see UCoinComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UCoinComponent* GetCoinComponentConst() const = 0;

	/**
	 * \brief Retrieves the constant reference to the merchant component.
	 *
	 * \return A pointer to the constant instance of the merchant component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UMerchantComponent* GetMerchantComponentConst() const = 0;

	/**
	 * \brief Get the merchant dispatcher.
	 *
	 * \return A reference to the merchant dispatcher.
	 *
	 * \note The merchant dispatcher is responsible for handling dynamic pool changes.
	 *
	 * \see FOnMerchantDynamicPoolChangedDelegate
	 */
	virtual FOnMerchantDynamicPoolChangedDelegate& GetMerchantDispatcher() const = 0;


	/**
	 * @brief Initialize merchant data.
	 *
	 * This method initializes the merchant's static items, dynamic items, and coin value.
	 *
	 * @param StaticItems The array of static items for the merchant.
	 * @param DynamicItems The array of dynamic items for the merchant.
	 * @param Coins The coin value for the merchant.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void InitMerchantData(const TArray<int32>& StaticItems,
	                              const TArray<FMerchantDynamicItemStorage>& DynamicItems,
	                              const FCoinValue& Coins);

	/**
	 * Retrieves the collection of static items available in the merchant's inventory.
	 *
	 * @return A constant reference to an array of integers representing the static item IDs.
	 *         The array is owned by the merchant component and should not be modified.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<int32>& GetStaticItemsConst() const;

	/**
	 * @brief Gets the constant reference to the array of dynamic merchant items.
	 *
	 * This method is used to retrieve the constant reference to the array of dynamic merchant items.
	 *
	 * @return The constant reference to the array of dynamic merchant items.
	 *
	 * @see FMerchantDynamicItemStorage
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<FMerchantDynamicItemStorage>& GetDynamicItemsConst() const;

	/**
	 * @brief Removes the specified amount of the item, if needed.
	 *
	 * This method removes the specified amount of the item identified by the given ItemID,
	 * if the item is present in the inventory. If the specified amount is equal to or greater
	 * than the current quantity of the item, the entire item will be removed from the inventory.
	 *
	 * @param ItemID The unique identifier of the item to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void RemoveItemAmountIfNeeded(int32 ItemID);

	/**
	 * \brief PayCoin method for merchant interface
	 *
	 * \param CoinValue The value of the coin to be paid
	 *
	 * This method is used to pay a specific coin value for a merchant. It calls the PayAndAdjust method of the coin component
	 * to deduct the coin value from the inventory and update the balance accordingly.
	 *
	 * \see FCoinValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PayCoin(const FCoinValue& CoinValue);

	/**
	 * \brief Function to receive a coin value.
	 *
	 * This function is a BlueprintCallable function that allows the merchant to receive a coin value.
	 *
	 * \param CoinValue The coin value to be received.
	 *
	 * \return None.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void ReceiveCoin(const FCoinValue& CoinValue);

	/**
	 * Check if the merchant has the specified item.
	 *
	 * @param ItemID The ID of the item to check.
	 * @return true if the merchant has the item, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool HasItem(int32 ItemID) const;

	/**
	 * Determines whether the merchant can pay the given amount with the available coins.
	 *
	 * @param CoinValue The amount to be paid with coins.
	 *
	 * @return True if the merchant can pay the amount with the available coins, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool CanPayAmount(const FCoinValue& CoinValue) const;

	/**
	 * @brief Adjusts the price of a product for buying.
	 *
	 * This method calculates the adjusted price of a product based on the given coin value.
	 *
	 * @param CoinValue The original coin value of the product.
	 *
	 * @return The adjusted coin value after applying the merchant ratio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceBuy(const FCoinValue& CoinValue) const;

	/**
	 * @brief Adjusts the sell price of an item based on the merchant ratio.
	 *
	 * This method takes a CoinValue object representing the sell price of an item and calculates the adjusted sell price
	 * by multiplying it with the merchant ratio obtained from the GetMerchantRatio method.
	 *
	 * @param CoinValue The original sell price of the item.
	 * @return The adjusted sell price based on the merchant ratio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceSell(const FCoinValue& CoinValue) const;

	/**
	 * @brief Adjusts the price of a product for regarding inflation.
	 *
	 * This method calculates the adjusted price of a product based on the given coin value.
	 *
	 * @param CoinValue The original coin value of the product.
	 *
	 * @return The adjusted coin value after applying the general inflation value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceForInflation(const FCoinValue& CoinValue) const;

	/**
	 * Retrieves the buy price for an item.
	 *
	 * @param ItemID The ID of the item to get the buy price for.
	 * @return The buy price of the item as a FCoinValue.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceBuy(int32 ItemID) const;

	/**
	 * @brief Retrieves the selling price of an item.
	 *
	 * This method returns the selling price of an item based on its ID. The selling price is calculated by getting the base value of the item and adjusting it using the AdjustPriceSell
	 * function.
	 *
	 * @param ItemID The ID of the item for which the selling price is to be retrieved.
	 * @return The selling price of the item as an FCoinValue struct.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceSell(int32 ItemID) const;

	/**
	 * @brief Adds a dynamic item to the merchant's inventory.
	 *
	 * This method allows for the addition of a dynamic item to the merchant's inventory.
	 * The item is specified by its unique identifier, ItemID.
	 *
	 * @param ItemID The unique identifier of the item to be added.
	 *
	 * @see GetMerchantComponent()
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void AddDynamicItem(int32 ItemID);
};
