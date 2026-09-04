#pragma once

#include "CoreMinimal.h"
#include "InventoryDelivery.h"
#include "InventoryEscrow.generated.h"

/**
 * Server-authored record for an item temporarily removed from usable inventory.
 * The reservation keeps its original capacity unavailable until the transaction resolves.
 */
USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FInventoryEscrowItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Escrow")
	int32 ItemID = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Escrow")
	float Durability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Escrow")
	FInventoryDeliveryDestination Source;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Escrow")
	FGuid ReservationId;

	/** Effective weight at extraction time, including the source bag's reduction. */
	UPROPERTY(BlueprintReadOnly, Category="Inventory|Escrow")
	float EffectiveWeight = 0.0f;

	bool IsValid() const
	{
		return ItemID > 0 && ReservationId.IsValid() &&
			(Source.Kind == EInventoryDeliveryDestinationKind::Bag ||
			 Source.Kind == EInventoryDeliveryDestinationKind::Equipment);
	}
};
