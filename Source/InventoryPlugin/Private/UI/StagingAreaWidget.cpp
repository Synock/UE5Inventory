// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/StagingAreaWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"

void UStagingAreaWidget::InitData()
{

	Refresh();
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();

	PC->GetStagingAreaItems()->StagingAreaDispatcher.AddUniqueDynamic(this, &UStagingAreaWidget::Refresh);
	//PC->GetStagingAreaItems()->StagingAreaDispatcher.AddUniqueDynamic(this, &UStagingAreaSlotWidget::ResetTransaction);
}

void UStagingAreaWidget::Refresh()
{

}

IInventoryPlayerInterface* UStagingAreaWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}
