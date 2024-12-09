// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "CoinValue.h"
#include "CoinComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurseChangedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurseChangedDelegate_Server);

///@brief
///Class responsible for holding coins
///There is no inner protection against negative coin amount
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API  UCoinComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCoinComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_PurseData, BlueprintReadOnly, Category="Inventory|Purse")
	FCoinValue PurseContent;

public:
	/**
	 * @brief Notifies when the PurseData has been replicated.
	 *
	 * This function is automatically called when the PurseData property has been modified
	 * and successfully replicated to clients. It broadcasts the PurseDispatcher event
	 * which can be captured by other components to handle the replicated data.
	 *
	 * @param None
	 */
	UFUNCTION()
	void OnRep_PurseData();

	/**
	 * @brief PurseDispatcher Variable
	 *
	 * @details This variable is a BlueprintAssignable property that belongs to the Inventory|Purse category.
	 * It is used to dispatch events when the purse is changed.
	 *
	 * @see FOnPurseChangedDelegate
	 */
	UPROPERTY(BlueprintAssignable, Category="Inventory|Purse")
	FOnPurseChangedDelegate PurseDispatcher;

	/**
	 * PurseDispatcher_Server is an event delegate that is used to dispatch notifications
	 * when the Purse inventory changes on the server side.
	 *
	 * It is BlueprintAssignable, which means it can be bound to in Blueprint classes, and
	 * BlueprintAuthorityOnly, which means it can only be executed by the server.
	 *
	 * This delegate belongs to the "Inventory|Purse" category.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category="Inventory|Purse")
	FOnPurseChangedDelegate_Server PurseDispatcher_Server;

	/**
	 * \brief Edit the content of the coin purse.
	 *
	 * This function updates the content of the coin purse by adding the specified amounts of copper piece (CP), silver piece (SP),
	 * gold piece (GP), and platinum piece (PP) to the existing content.
	 *
	 * \param InputCP The amount of copper pieces to add.
	 * \param InputSP The amount of silver pieces to add.
	 * \param InputGP The amount of gold pieces to add.
	 * \param InputPP The amount of platinum pieces to add.
	 *
	 * \return None.
	 *
	 * \note This function will broadcast the updated content of the coin purse after the update.
	 *
	 * \see UCoinComponent::PurseContent
	 * \see UCoinComponent::PurseDispatcher_Server
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void EditCoinContent(int32 InputCP, int32 InputSP, int32 InputGP, int32 InputPP);

	/**
	 * PayAndAdjust method adjusts the purse content by subtracting the cost from the current coin value.
	 *
	 * @param Cost - The cost to subtract from the purse content.
	 *
	 * Usage:
	 *     PayAndAdjust(Cost);
	 *
	 * Example:
	 *     FCoinValue Cost;
	 *     Cost.CopperPieces = 10;
	 *     Cost.SilverPieces = 3;
	 *     Cost.GoldPieces = 1;
	 *     Cost.PlatinumPieces = 0;
	 *     PayAndAdjust(Cost);
	 *
	 * Remarks:
	 *     - This method should only be called on the server.
	 *     - The cost is subtracted from the current coin value in the purse.
	 *     - The adjusted purse content is broadcasted to all relevant listeners.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void PayAndAdjust(const FCoinValue& Cost);

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void PayAndAdjustSimple(const FCoinValue& Cost);

	/**
	 * Removes coins from the purse.
	 *
	 * This method is used to remove a certain amount of coins from the purse. The coins will be subtracted from the current
	 * coin content. This method only executes on the server (ROLE_Authority).
	 *
	 * @param CoinValue - The value of the coins to be removed. It contains the number of copper, silver, gold, and platinum
	 *                    pieces to be subtracted.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void RemoveCoins(const FCoinValue& CoinValue);

	/**
	 * AddCoins method adds the given coin value to the purse inventory.
	 *
	 * @param CoinValue The coin value to be added to the purse inventory.
	 *
	 * @see FCoinValue
	 *
	 * @note This method can only be called by the server (ROLE_Authority).
	 *       Clients are not allowed to directly add coins to the inventory.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void AddCoins(const FCoinValue& CoinValue);

	/**
	 * \brief Loots the contents of another purse and adds it to this purse.
	 *
	 * This method allows the player to loot the contents of another purse and add it to their own purse.
	 *
	 * \param OtherPurse The purse from which the contents will be looted.
	 *
	 * \return None.
	 *
	 * \see UCoinComponent::EditCoinContent(), UCoinComponent::GetCP(), UCoinComponent::GetSP(),
	 * UCoinComponent::GetGP(), UCoinComponent::GetPP(), UCoinComponent::ClearPurse()
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void LootPurse(UCoinComponent* OtherPurse);

	/**
	 * @brief Retrieves the content of the purse.
	 *
	 * This method returns a constant reference to the FCoinValue object that represents the current content of the purse.
	 *
	 * @return The constant reference to the FCoinValue object representing the content of the purse.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	const FCoinValue& GetPurseContent() const;

	/**
	 * @brief Clears the purse content.
	 *
	 * This method sets the purse content to zero for all coin types.
	 *
	 * @param None.
	 *
	 * @return None.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void ClearPurse();

	/**
	 * @brief Checks if the purse has any content.
	 *
	 * This method checks if the purse has any content by examining the amount of copper pieces, silver pieces, gold pieces,
	 * and platinum pieces stored in the purse. If any of these amounts are non-zero, it means that the purse has content.
	 *
	 * @return true if the purse has content, false otherwise.
	 *
	 * @see UCoinComponent::PurseContent
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	bool HasContent();

	/**
	 * @brief Calculates the total weight of the coins in the purse.
	 *
	 * This method calculates the weight of each type of coin in the purse by multiplying the number of coins of each type
	 * with the weight per coin. The weight per coin is predefined for each type based on their material density and volume.
	 * The total weight is then calculated by summing the individual weights of each type of coin.
	 *
	 * @return The total weight of the coins in the purse, measured in kilograms.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	float GetTotalWeight();

	/**
	 * @brief Returns the amount of Copper Pieces (CP) in the purse.
	 *
	 * @return The amount of Copper Pieces (CP) in the purse.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetCP() const { return PurseContent.CopperPieces; }

	/**
	 * @brief Retrieve the value of Silver Pieces (SP) from the PurseContent.
	 *
	 * @return The number of Silver Pieces (SP) in the PurseContent.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetSP() const { return PurseContent.SilverPieces; }

	/**
	 * \brief Retrieves the current amount of Gold Pieces (GP) in the purse.
	 *
	 * \return The current amount of GP in the purse as an integer.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetGP() const { return PurseContent.GoldPieces; }

	/**
	 * GetPP method returns the number of Platinum Pieces in the purse.
	 *
	 * @return The number of Platinum Pieces in the purse.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetPP() const { return PurseContent.PlatinumPieces; }
};
