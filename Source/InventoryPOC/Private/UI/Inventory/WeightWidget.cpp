// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Inventory/WeightWidget.h"
#include "InventoryPOC/InventoryPOCCharacter.h"

void UWeightWidget::QueryTotalWeight()
{
	if(PCOwner)
	{
		TotalWeight = PCOwner->GetTotalWeight();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UWeightWidget::InitWidget(AInventoryPOCCharacter* Owner)
{
	PCOwner = Owner;
	PCOwner->WeightDispatcher.AddDynamic(this, &UWeightWidget::Refresh);
	Refresh();
}
