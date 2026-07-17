
#include "UI/StagingAreaWidget.h"
#include "InventoryPlugin.h"

#include "InventoryUtilities.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "InventoryPlugin.h"
#include "Items/InventoryItemBase.h"
#include "InventoryPlugin.h"
#include "UI/StagingAreaSlotWidget.h"
#include "InventoryPlugin.h"

void UStagingAreaWidget::InitData()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("StagingAreaWidget::InitData - No inventory player interface found"));
		return;
	}

	StagingComponent = PC->GetStagingAreaItems();
	if (!StagingComponent)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("StagingAreaWidget::InitData - No staging component found"));
		return;
	}

	// Bind to staging area updates
	StagingComponent->StagingAreaDispatcher.AddUniqueDynamic(this, &UStagingAreaWidget::Refresh);

	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaWidget::Refresh()
{
	if (!IsVisible())
		return;

	if (!StagingComponent)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("StagingAreaWidget::Refresh - Staging component is null"));
		return;
	}

	const TArray<FInventoryEscrowItem>& StagingItems = StagingComponent->GetStagingAreaItems();

	// Update slots with staged items
	int32 SlotIndex = 0;
	for (const FInventoryEscrowItem& StagingItemStorage : StagingItems)
	{
		if (SlotIndex >= MaxStagingSlots)
			break;

		UStagingAreaSlotWidget* SlotWidget = GetItemSlotFromID(SlotIndex);
		if (!SlotWidget)
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("StagingAreaWidget::Refresh - Slot %d is null"), SlotIndex);
			SlotIndex++;
			continue;
		}

		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(StagingItemStorage.ItemID, GetWorld());
		SlotWidget->InitBareData(Item, GetOwningPlayer(), 40.f, StagingItemStorage.Durability);


		if (Item)
		{
			SlotWidget->SetToolTipText(FText::FromString(Item->Name));
		}

		SlotWidget->Refresh();
		SlotIndex++;
	}

	// Clear remaining empty slots
	for (int32 EmptySlotIndex = SlotIndex; EmptySlotIndex < MaxStagingSlots; ++EmptySlotIndex)
	{
		UStagingAreaSlotWidget* SlotWidget = GetItemSlotFromID(EmptySlotIndex);
		if (!SlotWidget)
			continue;

		SlotWidget->InitBareData(nullptr, GetOwningPlayer(), 40.f);
		SlotWidget->SetToolTipText(FText::GetEmpty());
		SlotWidget->Refresh();
	}
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UStagingAreaWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

UStagingAreaSlotWidget* UStagingAreaWidget::GetItemSlotFromID(int32 ID) const
{
	switch (ID)
	{
	case 0: return Slot0;
	case 1: return Slot1;
	case 2: return Slot2;
	case 3: return Slot3;
	case 4: return Slot4;
	case 5: return Slot5;
	case 6: return Slot6;
	case 7: return Slot7;
	default:
		UE_LOG(LogInventoryPlugin, Warning, TEXT("StagingAreaWidget::GetItemSlotFromID - Invalid slot ID: %d"), ID);
		return nullptr;
	}
}
