// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/StagingAreaWidget.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/StagingAreaSlotWidget.h"

void UStagingAreaWidget::InitData()
{


	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();

	StagingComponent = PC->GetStagingAreaItems();
	PC->GetStagingAreaItems()->StagingAreaDispatcher.AddUniqueDynamic(this, &UStagingAreaWidget::Refresh);
	//PC->GetStagingAreaItems()->StagingAreaDispatcher.AddUniqueDynamic(this, &UStagingAreaSlotWidget::ResetTransaction)
	Refresh();
}

void UStagingAreaWidget::Refresh()
{
	if(!IsVisible())
		return;

	int32 SID = 0;
	for(auto & StagingItem : StagingComponent->GetStagingAreaItems())
	{
		int32 LocalID = SID++;
		GetItemSlotFromID(LocalID)->InitBareData(UInventoryUtilities::GetItemFromID(StagingItem, GetWorld()),GetOwningPlayer(), 40.f);
		GetItemSlotFromID(LocalID)->SetToolTipText(FText::FromString(GetItemSlotFromID(LocalID)->GetReferencedItem()->Name));
		GetItemSlotFromID(LocalID)->Refresh();
	}

	for(int32 NID = SID; NID < 8; ++NID)
	{
		GetItemSlotFromID(NID)->InitBareData({},GetOwningPlayer(), 40.f);
		GetItemSlotFromID(NID)->SetToolTipText({});
		GetItemSlotFromID(NID)->Refresh();
	}
}

IInventoryPlayerInterface* UStagingAreaWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

UStagingAreaSlotWidget* UStagingAreaWidget::GetItemSlotFromID(int32 ID)
{
	if(ID == 0)
		return Slot0;
	if(ID == 1)
		return Slot1;
	if(ID == 2)
		return Slot2;
	if(ID == 3)
		return Slot3;
	if(ID == 4)
		return Slot4;
	if(ID == 5)
		return Slot5;
	if(ID == 6)
		return Slot6;
	if(ID == 7)
		return Slot7;

	return nullptr;
}
