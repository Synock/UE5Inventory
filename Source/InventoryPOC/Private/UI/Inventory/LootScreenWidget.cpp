// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Inventory/LootScreenWidget.h"

void ULootScreenWidget::InitLootData(ALootableActor* InputLootedActor)
{
	if (InputLootedActor)
	{
		LootName = InputLootedActor->GetName();
		LootedActor = InputLootedActor;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::DeInitLootData()
{
	//LootedActor->GetLootPoolDeletate().RemoveAll(this);
	DeInitUI();
	LootedActor = nullptr;
}
