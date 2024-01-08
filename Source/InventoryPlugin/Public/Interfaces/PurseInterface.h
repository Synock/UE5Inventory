// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "CoinValue.h"
#include "Components/CoinComponent.h"
#include "UObject/Interface.h"
#include "PurseInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPurseInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IPurseInterface
 *
 * @brief This class is an interface that provides functions for managing a purse in a game.
 *
 * The IPurseInterface class is used as a base class to implement the purse functionality in a game.
 * It provides functions to get, pay, receive, and transfer coin amounts in a purse.
 */
class INVENTORYPLUGIN_API IPurseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Get the purse component.
	 *
	 * This method returns a pointer to the purse component associated with the current object.
	 *
	 * @return A pointer to the purse component.
	 * @note The returned pointer must not be deleted by the caller, as it is managed by the current object.
	 *       If the purse component is not available, nullptr will be returned.
	 */
	virtual UCoinComponent* GetPurseComponent() = 0;

	/**
	 * @brief Retrieves the constant pointer to the purse component.
	 *
	 * This method returns a constant pointer to the purse component of an object.
	 * The purse component represents the collection of coins in the purse.
	 *
	 * @return A constant pointer to the purse component.
	 *
	 * @note The returned pointer is constant, meaning that the value cannot be modified.
	 *
	 * @see UCoinComponent
	 */
	virtual const UCoinComponent* GetPurseComponentConst() const = 0;

	/**
	 * @brief Pays a specified coin amount from the purse.
	 *
	 * This method allows paying a specified coin amount from the purse. The method
	 * internally calls the PayAndAdjust method of the purse component to perform the
	 * payment and adjust the purse balance accordingly.
	 *
	 * @param Amount The coin amount to be paid from the purse.
	 *
	 * @see GetPurseComponent()
	 */
	virtual void PayCoinAmount(const FCoinValue& Amount);

	/**
	 * Receives the given coin amount.
	 *
	 * This method is used to receive a specified coin amount and add it to the purse.
	 *
	 * @param Amount The amount of coins to receive.
	 */
	virtual void ReceiveCoinAmount(const FCoinValue& Amount);

	/**
	 * @brief Get the amount of coins in the purse.
	 *
	 * This method retrieves the current amount of coins stored in the purse.
	 *
	 * @return The amount of coins in the purse.
	 */
	virtual FCoinValue GetCoinAmount() const;

	/**
	 * @brief Transfers coins from one coin component to another coin component.
	 *
	 * This method transfers the specified number of coins from the giving coin component to the receiving coin component.
	 *
	 * @param GivingComponent The coin component that will give the coins.
	 * @param ReceivingComponent The coin component that will receive the coins.
	 * @param RemovedValue The value representing the number of coins to be removed from the giving component.
	 * @param AddedValue The value representing the number of coins to be added to the receiving component.
	 *
	 * @return void
	 */
	virtual void TransferCoinsTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                             const FCoinValue & RemovedValue, const FCoinValue & AddedValue);
};
