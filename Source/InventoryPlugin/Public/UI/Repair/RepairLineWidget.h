#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "CoinValue.h"
#include "Definitions.h"
#include "RepairLineWidget.generated.h"

class UCoinDisplayWidget;

/**
 * @struct FRepairLineDataStruct
 *
 * Data structure for a single repairable item in the list
 */
USTRUCT(BlueprintType)
struct FRepairLineDataStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	FString ItemName;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	EEquipmentSlot Slot = EEquipmentSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	FCoinValue RepairCost;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	float CurrentDurability = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	float MaxDurability = 100.0f;
};

/**
 * @class URepairLineData
 *
 * UObject wrapper for repair line data (required for list view entries)
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API URepairLineData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Repair")
	FRepairLineDataStruct Data;
};

/**
 * @class URepairLineWidget
 *
 * Displays a single repairable item in the repair list.
 * Shows item icon, name, equipment slot, repair cost, and repair button.
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API URepairLineWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Item icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|Line|UI")
	UImage* ItemIcon = nullptr;

	/** Item name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|Line|UI")
	UTextBlock* ItemName = nullptr;

	/** Equipment slot text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|Line|UI")
	UTextBlock* EquipmentSlot = nullptr;

	/** Repair cost display widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|Line|UI")
	UCoinDisplayWidget* RepairCost = nullptr;

	/** Repair button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Repair|Line|UI")
	UButton* RepairButton = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Repair|Line")
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Repair|Line")
	EEquipmentSlot ItemSlot = EEquipmentSlot::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Repair|Line")
	float CurrentDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Repair|Line")
	float MaxDurability = 100.0f;

	/** Reference to the parent repair widget */
	UPROPERTY(BlueprintReadOnly, Category = "Repair|Line")
	TObjectPtr<class URepairWidget> ParentRepairWidget = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update all UI elements with the provided data
	 * @param ItemData The repair line data to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair|Line")
	void UpdateDisplay(const FRepairLineDataStruct& ItemData);

	/**
	 * @brief Handle repair button click
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair|Line")
	void OnRepairButtonClicked();

	/**
	 * @brief Convert equipment slot enum to display string
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Repair|Line")
	FString GetEquipmentSlotDisplayName(EEquipmentSlot EquipmentSlotToRetrieve) const;

	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * @brief Set the parent repair widget reference
	 * @param InParentWidget The parent URepairWidget that owns this list
	 */
	UFUNCTION(BlueprintCallable, Category = "Repair|Line")
	void SetParentRepairWidget(URepairWidget* InParentWidget);

	/**
	 * @brief Get the current item ID
	 * @return The item ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Repair|Line")
	int32 GetItemID() const { return ItemID; }

	/**
	 * @brief Get the current item slot
	 * @return The equipment slot
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Repair|Line")
	EEquipmentSlot GetItemSlot() const { return ItemSlot; }
};

