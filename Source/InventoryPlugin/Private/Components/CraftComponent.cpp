// Copyright 2025 Maximilien (Synock) Guislain


#include "Components/CraftComponent.h"

#include "Net/UnrealNetwork.h"

UCraftComponent::UCraftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftComponent::OnRep_InputCraftPool()
{
	InputCraftPoolDispatcher.Broadcast();
}

void UCraftComponent::OnRep_OutputCraftPool()
{
	OutputCraftPoolDispatcher.Broadcast();
}

int32 UCraftComponent::GetItemAtIndex(int32 ID) const
{
	for (auto& Item : InputCraftPool)
	{
		if (Item.TopLeftID == ID)
			return Item.ItemID;
	}
	return -1;
}

void UCraftComponent::RemoveItem_Implementation(int32 TopLeftIndex)
{
	int32 ID = 0;
	for (const auto& Item : InputCraftPool)
	{
		if (Item.TopLeftID == TopLeftIndex)
		{
			InputCraftPool.RemoveAt(ID);
			break;
		}
		++ID;
	}

	ItemRemoveDispatcher.Broadcast(TopLeftIndex);
}

void UCraftComponent::AddItem_Implementation(int32 ItemID, int32 TopLeftIndex)
{
	InputCraftPool.Add({ItemID, TopLeftIndex});
	ItemAddDispatcher.Broadcast(ItemID, TopLeftIndex);
}

void UCraftComponent::BeginPlay()
{
	Super::BeginPlay();

}

//----------------------------------------------------------------------------------------------------------------------

void UCraftComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UCraftComponent, InputCraftPool, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCraftComponent, OutputCraftPool, COND_OwnerOnly);
}

