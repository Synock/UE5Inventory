// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryPOCGameMode.h"
#include "InventoryPOCCharacter.h"
#include "MainGameState.h"
#include <UObject/ConstructorHelpers.h>

AInventoryPOCGameMode::AInventoryPOCGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCGameMode::RegisterItem(const FBareItem& NewItem)
{
	ItemMap.Add(NewItem.Key, NewItem);

	AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();

	if (GS)
	{
		GS->RegisterItem(NewItem);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot register item as MGS is invalid"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

FBareItem AInventoryPOCGameMode::FetchItemFromID(int64 ID) const
{
	if (!ItemMap.Contains(ID))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find any item at ID %d"), ID);
		return FBareItem();
	}
	FBareItem LocalItem = ItemMap.FindChecked(ID);
	return LocalItem;
}
