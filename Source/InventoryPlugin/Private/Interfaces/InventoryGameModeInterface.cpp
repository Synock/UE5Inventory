// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/InventoryGameModeInterface.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
// Add default functionality here for any IInventoryGameModeInterface functions that are not pure virtual.
void IInventoryGameModeInterface::SpawnItemFromActor(AActor* Actor, uint32 ItemID, bool ClampOnGround)
{
	if (!Actor)
		return;

	if (ItemID == 0)
		return;

	const FVector Location = Actor->GetActorLocation();

	//const FVector Forward = Actor->GetActorForwardVector();//maybe 1m ahead?

	FVector SpawnLocation = Location;

	if (ClampOnGround)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bDebugQuery = true;
		QueryParams.bTraceComplex = true;
		if (Actor->GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location - FVector(0, 0, 1000),
		                                                ECollisionChannel::ECC_Visibility, QueryParams))
		{
			SpawnLocation = HitResult.Location;
		}
	}

	//spawn actor here
}
