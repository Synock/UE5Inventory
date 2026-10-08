#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PendingDeliveryWidget.generated.h"

class UButton;
class UTextBlock;
class UInventoryDeliveryComponent;
class UPendingDeliverySlotWidget;

/** Generic, collapsible presentation for the owner's protected delivery queue. */
UCLASS()
class INVENTORYPLUGIN_API UPendingDeliveryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category="Inventory|Delivery")
	void Refresh();

protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> QueueCountText;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> ClaimButton;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> ClaimAllButton;

	/** FIFO-head item presentation. Optional so legacy/native fallback layouts keep working. */
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UPendingDeliverySlotWidget> PendingItemSlot;

	UFUNCTION()
	void ClaimFirst();

	UFUNCTION()
	void ClaimAll();

private:
	FText GetNativeItemText() const;
	FText GetNativeCountText() const;
	FReply HandleNativeClaim();
	FReply HandleNativeClaimAll();

	UPROPERTY()
	TObjectPtr<UInventoryDeliveryComponent> DeliveryComponent;
};
