#pragma once

#include "CoreMinimal.h"
#include "MerchantItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "MerchantItemListWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChangedDelegate, int32, ItemID);

/**
 * Merchant item list widget. Bind a UListView named "ItemListView" in the Blueprint
 * to get automatic C++ list management. Blueprint subclasses may override any method
 * for custom visual behavior while keeping the C++ logic as the authoritative fallback.
 */
UCLASS()
class INVENTORYPLUGIN_API UMerchantItemListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Bind a UListView named "ItemListView" in the Blueprint for automatic C++ management. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Merchant|List|UI")
	UListView* ItemListView = nullptr;

	/** Index in the list at which dynamic (sell) entries begin. */
	UPROPERTY(BlueprintReadOnly, Category = "Merchant|List")
	int32 DynamicStartIndex = 0;

#if WITH_AUTOMATION_WORKER
	int32 LastSelectionItemIDForTests = 0;
	int32 LastProgrammaticSelectionItemIDForTests = 0;
	int32 ProgrammaticSelectionCountForTests = 0;
#endif

	void BindListViewSelection();
	void UnbindListViewSelection();
	void HandleListItemSelectionChanged(UObject* SelectedItem);

public:
	/**
	 * Append an item entry to the list.
	 * C++ default creates a UMerchantItemData wrapper and adds it to ItemListView.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddDataToList(const FMerchantItemDataStruct& ItemData);
	virtual void AddDataToList_Implementation(const FMerchantItemDataStruct& ItemData);

	/**
	 * Remove all entries from the list.
	 * C++ default calls ItemListView->ClearListItems().
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClearList();
	virtual void ClearList_Implementation();

	/**
	 * Deselect the current selection.
	 * C++ default calls ItemListView->ClearSelection().
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClearSelection();
	virtual void ClearSelection_Implementation();

	/**
	 * Select the list entry matching ItemID after the list has been rebuilt.
	 * C++ default searches the bound ItemListView and selects the matching row.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool SelectItemByID(int32 ItemID);
	virtual bool SelectItemByID_Implementation(int32 ItemID);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSelectionChangedDelegate SelectionChangedDelegate;

	/**
	 * Update quantity/data for an existing dynamic entry.
	 * C++ default searches by ItemID and calls RequestRefresh on the list.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateDynamicElement(int64 ItemID, int32 Quantity);
	virtual void UpdateDynamicElement_Implementation(int64 ItemID, int32 Quantity);

	/**
	 * Remove all dynamic entries starting from DynamicStartID.
	 * C++ default removes every list item at index >= DynamicStartID.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RemoveElementsFrom(int32 DynamicStartID);
	virtual void RemoveElementsFrom_Implementation(int32 DynamicStartID);

#if WITH_AUTOMATION_WORKER
	void SetItemListViewForTests(UListView* InItemListView)
	{
		ItemListView = InItemListView;
		BindListViewSelection();
	}
	UListView* GetItemListViewForTests() const { return ItemListView; }
	int32 GetListItemCountForTests() const { return ItemListView ? ItemListView->GetNumItems() : 0; }
	int32 GetLastSelectionItemIDForTests() const { return LastSelectionItemIDForTests; }
	int32 GetLastProgrammaticSelectionItemIDForTests() const { return LastProgrammaticSelectionItemIDForTests; }
	int32 GetProgrammaticSelectionCountForTests() const { return ProgrammaticSelectionCountForTests; }
	void HandleListItemSelectionChangedForTests(UObject* SelectedItem) { HandleListItemSelectionChanged(SelectedItem); }
#endif
};
