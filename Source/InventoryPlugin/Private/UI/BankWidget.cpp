// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/BankWidget.h"

#include "Components/BankComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"

void UBankWidget::ReorganizeContent()
{
	if(const IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		Player->GetBankComponent()->Reorganize();
}
