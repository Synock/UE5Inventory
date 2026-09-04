
#include "Interfaces/InventoryGameModeInterface.h"

#include "CoinValue.h"
#include "Actors/DroppedCoins.h"
#include "Actors/DroppedItem.h"
#include "Components/LoreItemManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

ADroppedItem* IInventoryGameModeInterface::SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID,
	const FVector& DesiredDropLocation, bool ClampOnGround, float Durability)
{

		if (!SpawningActor)
			return nullptr;

		if (ItemID <= 0)
			return nullptr;

		const FVector SpawnLocation = GetItemSpawnLocation(SpawningActor, DesiredDropLocation, ClampOnGround);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = SpawningActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		UInventoryItemBase* ItemToSpawn = FetchItemFromID(ItemID);

		ADroppedItem* Item = SpawningActor->GetWorld()->SpawnActor<ADroppedItem>(SpawnLocation, SpawningActor->GetActorRotation(),
																	SpawnParams);
		Item->SetReplicates(true);
		Item->InitializeFromItem(ItemToSpawn, true, SpawningActor->GetActorRotation());
		return Item;
}

//------------------------------------------------------------------------------------------------------------------

ADroppedItem* IInventoryGameModeInterface::SpawnItemFromActorRaw(AActor* SpawningActor, UInventoryItemBase* ItemToSpawn, float Durability)
{
	if (!SpawningActor)
		return nullptr;

	if (!ItemToSpawn)
		return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SpawningActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADroppedItem* Item = SpawningActor->GetWorld()->SpawnActor<ADroppedItem>(SpawningActor->GetActorLocation(), SpawningActor->GetActorRotation(),
																SpawnParams);
	Item->SetReplicates(true);
	Item->InitializeFromItem(ItemToSpawn, false);
	return Item;
}

//------------------------------------------------------------------------------------------------------------------

ADroppedCoins* IInventoryGameModeInterface::SpawnCoinsFromActor(AActor* SpawningActor, const FCoinValue& CoinValue,
	const FVector& DesiredDropLocation, bool ClampOnGround)
{
	if (!SpawningActor)
		return nullptr;

	if (CoinValue.IsEmpty())
		return nullptr;

	const FVector SpawnLocation = GetItemSpawnLocation(SpawningActor, DesiredDropLocation, ClampOnGround);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SpawningActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ADroppedCoins* Item = SpawningActor->GetWorld()->SpawnActor<ADroppedCoins>(SpawnLocation, SpawningActor->GetActorRotation(),
																  SpawnParams);
	Item->SetReplicates(true);
	Item->InitializeFromCoinValue(CoinValue);

	return Item;
}

//------------------------------------------------------------------------------------------------------------------

FVector IInventoryGameModeInterface::GetItemSpawnLocation(AActor* SpawningActor, const FVector& DesiredDropLocation, bool ClampOnGround)
{
	if (!SpawningActor)
		return DesiredDropLocation;

	FVector SpawnLocation = DesiredDropLocation;

	if(SpawnLocation.IsNearlyZero())
	{
		const FVector Location = SpawningActor->GetActorLocation();

		const FVector Forward = SpawningActor->GetActorForwardVector() * 0.2;

		SpawnLocation = Location + Forward;
	}

	if (ClampOnGround)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		if (SpawningActor->GetWorld()->LineTraceSingleByChannel(HitResult, SpawnLocation + FVector(0, 0, 50),
		                                                        SpawnLocation - FVector(0, 0, 1000),
		                                                        ECollisionChannel::ECC_Visibility, QueryParams))
		{
			SpawnLocation = HitResult.Location;
		}
	}

	return SpawnLocation;
}

//------------------------------------------------------------------------------------------------------------------

bool IInventoryGameModeInterface::CanSpawnItem(UInventoryItemBase* NewItem)
{
	const ULoreItemManagerComponent* LoreComponent = GetLoreManagementComponent();
	if (!LoreComponent)
		return true;

	return LoreComponent->CanSpawnItem(NewItem);
}

//------------------------------------------------------------------------------------------------------------------

bool IInventoryGameModeInterface::DelayedLoreItemValidation(const UInventoryItemBase* LocalItem, ULootPoolComponent* Origin)
{
	auto* LoreComponent = GetLoreManagementComponent();
	if (!LoreComponent)
		return false;

	return LoreComponent->DelayedSpawnItem(LocalItem, Origin);
}

//------------------------------------------------------------------------------------------------------------------

ULoreItemManagerComponent* IInventoryGameModeInterface::GetLoreManagementComponent()
{
	return nullptr;
}


float IInventoryGameModeInterface::GetCurrentInflationValue()
{
	return 0.f;
}
