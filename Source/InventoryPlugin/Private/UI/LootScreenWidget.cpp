// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/LootScreenWidget.h"

void ULootScreenWidget::InitLootData(AActor* InputLootedActor)
{
	if (ILootableInterface* Interface = Cast<ILootableInterface>(InputLootedActor); InputLootedActor && Interface)
	{
		LootedActor.SetObject(InputLootedActor);
		LootedActor.SetInterface(Interface);
		LootName = Interface->GetLootActorName();
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
