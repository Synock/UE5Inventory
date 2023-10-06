// Copyright 2023 Maximilien (Synock) Guislain


#include "Components/KeyringComponent.h"

#include "Items/InventoryItemKey.h"
#include "Net/UnrealNetwork.h"

UKeyringComponent::UKeyringComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

bool UKeyringComponent::HasKey(int32 KeyId) const
{
	return Keyring.Contains(KeyId);
}

//----------------------------------------------------------------------------------------------------------------------

bool UKeyringComponent::TryAddKeyFromItem(const UInventoryItemKey* Item)
{
	if(!Item)
		return false;

	if(HasKey(Item->KeyID))
		return false;

	AddKey(Item->KeyID, Item->ItemID);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringComponent::AddKey(int32 KeyId, int32 ItemId)
{
	KeyringData.Add({KeyId, ItemId});
	Keyring.Add(KeyId);
	KeyRingToItemLUT.Add(KeyId, ItemId);
	KeyAdded.Broadcast(KeyId, ItemId);
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringComponent::RemoveKey(int32 KeyId)
{

	KeyringData.Remove({KeyId,KeyRingToItemLUT.FindChecked(KeyId)});
	Keyring.Remove(KeyId);
	KeyRingToItemLUT.Remove(KeyId);
	KeyRemoved.Broadcast(KeyId);
}

//----------------------------------------------------------------------------------------------------------------------

int32 UKeyringComponent::GetItemFromKey(int32 KeyId) const
{
	if(!KeyRingToItemLUT.Contains(KeyId))
		return -1;

	return KeyRingToItemLUT.FindChecked(KeyId);
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FKeyItemPair>& UKeyringComponent::GetAllPossessedKeys() const
{
	return KeyringData;
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringComponent::OnRep_KeyringChanged()
{
	Keyring.Empty();
	KeyRingToItemLUT.Empty();

	for(auto& KeyData : KeyringData)
	{
		Keyring.Add(KeyData.KeyId);
		KeyRingToItemLUT.Add(KeyData.KeyId, KeyData.ItemId);
	}

	KeyringChangedDelegate.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKeyringComponent, KeyringData);
}
