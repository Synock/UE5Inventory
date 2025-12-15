#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Interfaces/RepairInterface.h"
#include "CoinValue.h"
#include "Definitions.h"
#include "RepairWidget.generated.h"

class URepairLineData;
class UCoinDisplayWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughMoneyForRepair);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepairSuccessful);

/**
 * @struct FRepairItemData
 *
 * Data structure for items that can be repaired
 */
USTRUCT(BlueprintType)
struct FRepairItemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	FString ItemName;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	EEquipmentSlot Slot = EEquipmentSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	float CurrentDurability = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	float MaxDurability = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	FCoinValue RepairCost;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Repair")
	bool bNeedsRepair = false;
};

/**
 * @class URepairWidget
 *
 * Main widget for the repair system UI.
 * Displays repairable items and handles repair transactions.
 * Uses modern BindWidget approach for automatic UI element binding.
 */
UCLASS()
class INVENTORYPLUGIN_API URepairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget - automatically bound by name in Blueprint)
	//------------------------------------------------------------------------------------------------------------------

	/** Button to repair all damaged items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|UI")
	UButton* RepairAllButton = nullptr;

	/** Button to close the repair widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|UI")
	UButton* CloseButton = nullptr;

	/** Text displaying the repairer NPC's name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|UI")
	UTextBlock* RepairerName = nullptr;

	/** Coin display widget for repair all cost */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|UI")
	UCoinDisplayWidget* RepairAllCost = nullptr;

	/** ListView displaying all repairable items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|UI")
	UListView* RepairItemList = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	TScriptInterface<IRepairInterface> RepairerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	TArray<FRepairItemData> RepairableItems;

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	FCoinValue TotalRepairCost;

	//------------------------------------------------------------------------------------------------------------------
	// Lifecycle
	//------------------------------------------------------------------------------------------------------------------

	virtual void NativeConstruct() override;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Build the list of repairable items from player equipment
	 */
	void BuildRepairableItemList();

	/**
	 * @brief Populate the ListView with RepairLineData objects
	 */
	void PopulateRepairItemList();

	/**
	 * @brief Update the repair all cost display
	 */
	void UpdateRepairAllCostDisplay();

	/**
	 * @brief Handle repair all button click
	 */
	UFUNCTION()
	void OnRepairAllButtonClicked();

	/**
	 * @brief Handle close button click
	 */
	UFUNCTION()
	void OnCloseButtonClicked();

public:
	//------------------------------------------------------------------------------------------------------------------
	// Public Interface
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Initialize the repair widget with a repairer NPC
	 * @param InputRepairerActor The NPC actor that implements IRepairInterface
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void InitRepairData(AActor* InputRepairerActor);

	/**
	 * @brief Clean up repair data when closing
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void DeInitRepairData();

	/**
	 * @brief Refresh the repair list (call when equipment changes)
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void Refresh();

	/**
	 * @brief Repair a specific item (called by RepairLineWidget)
	 * @param ItemID The item ID to repair
	 * @param EquipmentSlot The equipment slot of the item
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void RepairItem(int32 ItemID, EEquipmentSlot EquipmentSlot);

	/**
	 * @brief Get the total repair cost for all items
	 * @return Total cost to repair all damaged items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Repair")
	FCoinValue GetTotalRepairCost() const { return TotalRepairCost; }

	/**
	 * @brief Check if player has enough money for repair all
	 * @return True if player can afford to repair all items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Repair")
	bool CanAffordRepairAll() const;

	//------------------------------------------------------------------------------------------------------------------
	// Delegates
	//------------------------------------------------------------------------------------------------------------------

	/** Broadcast when player doesn't have enough money */
	UPROPERTY(BlueprintAssignable, Category = "Repair")
	FNotEnoughMoneyForRepair OnNotEnoughMoneyDelegate;

	/** Broadcast when repair is successful */
	UPROPERTY(BlueprintAssignable, Category = "Repair")
	FRepairSuccessful OnRepairSuccessfulDelegate;
};

