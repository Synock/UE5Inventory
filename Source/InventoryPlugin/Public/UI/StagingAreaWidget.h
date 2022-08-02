// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StagingAreaWidget.generated.h"

class IInventoryPlayerInterface;
class UStagingAreaSlotWidget;
class UStagingAreaComponent;
/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UStagingAreaWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable)
	void InitData();

	UFUNCTION(BlueprintCallable)
	void Refresh();

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaComponent* StagingComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot0 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot1 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot2 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot3 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot4 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot5 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot6 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	UStagingAreaSlotWidget* Slot7 = nullptr;


	UStagingAreaSlotWidget* GetItemSlotFromID(int32 ID);
};
