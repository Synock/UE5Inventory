// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/InventoryGameModeInterface.h"

#include "Components/LoreItemManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

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
		QueryParams.bDebugQuery = true;
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

bool IInventoryGameModeInterface::CanSpawnItem(UInventoryItemBase* NewItem)
{
	const auto* LoreComponent = GetLoreManagementComponent();
	if (!LoreComponent)
		return true;

	return true;
}

bool IInventoryGameModeInterface::DelayedLoreItemValidation(const UInventoryItemBase* LocalItem, ULootPoolComponent* Origin)
{
	auto* LoreComponent = GetLoreManagementComponent();
	if (!LoreComponent)
		return false;

	return LoreComponent->DelayedSpawnItem(LocalItem, Origin);
}

ULoreItemManagerComponent* IInventoryGameModeInterface::GetLoreManagementComponent()
{
	return nullptr;
}

float IInventoryGameModeInterface::GetCurrentInflationValue()
{
	return 0.f;
}
