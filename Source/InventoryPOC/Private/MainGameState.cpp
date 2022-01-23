// Copyright 2022 Maximilien (Synock) Guislain


#include "MainGameState.h"
#include "Net/UnrealNetwork.h"

void AMainGameState::OnRep_ItemMap()
{
	ItemMap.Empty();

	for (auto& Item : CompleteItemList)
	{
		ItemMap.Emplace(Item.Key, Item);
	}
	UE_LOG(LogTemp, Log, TEXT("Repping item map for %d items"), ItemMap.Num());
}

//----------------------------------------------------------------------------------------------------------------------

FBareItem AMainGameState::FetchItemFromID(int64 ID)
{
	if (!ItemMap.Contains(ID))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find any item at ID %d"), ID);
		return FBareItem();
	}
	FBareItem localItem = ItemMap.FindChecked(ID);
	return localItem;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AMainGameState, CompleteItemList);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainGameState::RegisterItem(const FBareItem& NewItem)
{
	if (HasAuthority())
	{
		CompleteItemList.Add(NewItem);
		UE_LOG(LogTemp, Log, TEXT("Adding item %d %s to MGS"), NewItem.Key, *NewItem.Name);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to access MGS Item registration"));
	}
}
