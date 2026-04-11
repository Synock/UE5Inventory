#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "KeyringWidget.generated.h"

struct FkeyLineDataStruct;
class UKeyLineData;

UCLASS()
class INVENTORYPLUGIN_API UKeyringWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Bind a UListView named "KeyDataList" in the Blueprint for automatic C++ list management. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Inventory|Keyring|UI")
	UListView* KeyDataList = nullptr;

	/**
	 * Clear all keys from the list. C++ default calls KeyDataList->ClearListItems().
	 * Blueprint may override to add visual feedback.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClearList();
	virtual void ClearList_Implementation();

	/**
	 * Add a single key entry. C++ default creates a UKeyLineData and appends to KeyDataList.
	 * Blueprint may override to use a custom list layout.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddKey(const FkeyLineDataStruct& KeyLineData);
	virtual void AddKey_Implementation(const FkeyLineDataStruct& KeyLineData);

	UFUNCTION(BlueprintCallable)
	void InternalSetup();

public:
	UFUNCTION(BlueprintCallable)
	void RefreshList();

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                  UDragDropOperation* InOperation) override;
};
