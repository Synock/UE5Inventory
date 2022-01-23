// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Inventory/PurseWidget.h"
#include "InventoryPOC/InventoryPOCCharacter.h"


void UPurseWidget::InitWidget(UPurseComponent* Owner)
{
	if(PursePointer)
		PursePointer->PurseDispatcher.RemoveAll(this);

	PursePointer = Owner;
	if (PursePointer)
		PursePointer->PurseDispatcher.AddDynamic(this, &UPurseWidget::Refresh);


	Refresh();
}
