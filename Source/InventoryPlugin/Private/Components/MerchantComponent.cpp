// Copyright 2022 Maximilien (Synock) Guislain

#include "Components/MerchantComponent.h"
#include <Net/UnrealNetwork.h>

// Sets default values for this component's properties
UMerchantComponent::UMerchantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UMerchantComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::OnRep_DynamicPool()
{
	MerchantPoolDispatcher.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("OnRep_DynamicPool %s"), *GetName());
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::RemoveItemID_Implementation(int32 ItemID)
{
	//This method is retarded, please find something else T___T

	UE_LOG(LogTemp, Log, TEXT("Removing Item %d "), ItemID);
	int32 IDToRemove = -1;
	int32 LocalID = 0;
	for (auto& DynamicItem : DynamicMerchantPool)
	{
		if (DynamicItem.ItemID == ItemID)
		{
			UE_LOG(LogTemp, Log, TEXT("Found Item to remove %d quantity %d"), DynamicItem.ItemID, DynamicItem.Quantity);
			DynamicItem.Quantity--;

			if (DynamicItem.Quantity <= 0)
				IDToRemove = LocalID;

			break;
		}
		LocalID++;
	}

	if (IDToRemove >= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Removing Item %d (%d) because available quantity hit 0"), ItemID, IDToRemove);
		DynamicMerchantPool.RemoveAt(IDToRemove);
	}

	//Static items are not affected
}

//----------------------------------------------------------------------------------------------------------------------

int32 UMerchantComponent::GetItemAtIndex(int32 ListIndex) const
{
	if (ListIndex < StaticMerchantPool.Num())
		return StaticMerchantPool[ListIndex];

	if (ListIndex < StaticMerchantPool.Num() + DynamicMerchantPool.Num())
		return DynamicMerchantPool[ListIndex - StaticMerchantPool.Num()].ItemID;

	return -1;
}

//----------------------------------------------------------------------------------------------------------------------

int32 UMerchantComponent::GetItemFromStaticPool(int32 ListIndex) const
{
	if (ListIndex < StaticMerchantPool.Num())
		return StaticMerchantPool[ListIndex];

	return -1;
}

//----------------------------------------------------------------------------------------------------------------------

FMerchantDynamicItemStorage UMerchantComponent::GetItemFromDynamicPool(int32 ListIndex) const
{
	if (ListIndex < DynamicMerchantPool.Num())
		return DynamicMerchantPool[ListIndex];

	return {-1, -1};
}

//----------------------------------------------------------------------------------------------------------------------

bool UMerchantComponent::HasItem(int32 ItemID) const
{
	if (StaticMerchantPool.Contains(ItemID))
		return true;

	for (auto& ItemData : DynamicMerchantPool)
	{
		if (ItemData.ItemID == ItemID)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::InitDynamic_Implementation(const TArray<FMerchantDynamicItemStorage>& MerchantDynamicItems)
{
	DynamicMerchantPool = MerchantDynamicItems;
	//call the refresh broadcast just in case
	MerchantPoolDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::InitStatic_Implementation(const TArray<int32>& MerchantStaticItems)
{
	StaticMerchantPool = MerchantStaticItems;

	//call the refresh broadcast just in case
	MerchantPoolDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::AddItem_Implementation(int32 ItemID)
{
	// Somehow i will find a less retarded way to do that

	for (const auto& StaticItem : StaticMerchantPool)
		if (StaticItem == ItemID)
			return;

	for (auto& DynamicItem : DynamicMerchantPool)
		if (DynamicItem.ItemID == ItemID)
		{
			DynamicItem.Quantity++;
			return;
		}
	if (DynamicMerchantPool.Num() < DynamicPoolLimit)
		DynamicMerchantPool.Add({ItemID, 1});

	//otherwise drop the request
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::RemoveItem_Implementation(int32 ListIndex)
{
	if (ListIndex < DynamicMerchantPool.Num())
	{
		if (DynamicMerchantPool[ListIndex].Quantity > 1)
			DynamicMerchantPool[ListIndex].Quantity--;
		else
			DynamicMerchantPool.RemoveAt(ListIndex);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMerchantComponent, StaticMerchantPool);
	DOREPLIFETIME(UMerchantComponent, DynamicMerchantPool);
}
