// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/PurseWidget.h"
#include "Components/CoinComponent.h"


void UPurseWidget::InitWidget(UCoinComponent* Owner)
{
	if (PursePointer)
		PursePointer->PurseDispatcher.RemoveAll(this);

	PursePointer = Owner;
	if (PursePointer)
		PursePointer->PurseDispatcher.AddDynamic(this, &UPurseWidget::Refresh);

	Refresh();
}
