// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/InventoryGameModeInterface.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

FVector IInventoryGameModeInterface::GetItemSpawnLocation(AActor* SpawningActor, bool ClampOnGround)
{
	if (!SpawningActor)
		return {};

	const FVector Location = SpawningActor->GetActorLocation();

	const FVector Forward = SpawningActor->GetActorForwardVector() * 0.2;

	FVector SpawnLocation = Location + Forward;

	if (ClampOnGround)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bDebugQuery = true;
		QueryParams.bTraceComplex = true;
		if (SpawningActor->GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 50),
		                                                        Location - FVector(0, 0, 1000),
		                                                        ECollisionChannel::ECC_Visibility, QueryParams))
		{
			SpawnLocation = HitResult.Location;
		}
	}

	return SpawnLocation;
}
