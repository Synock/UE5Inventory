// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/WeightWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"

void UWeightWidget::QueryTotalWeight()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
	{
		TotalWeight = PC->GetTotalWeight();
	}
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UWeightWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UWeightWidget::InitWidget()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
	{
		PC->GetWeightChangedDelegate().AddDynamic(this, &UWeightWidget::Refresh);
	}
	Refresh();
}
