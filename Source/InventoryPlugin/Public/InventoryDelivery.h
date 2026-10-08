#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryDelivery.generated.h"

UENUM(BlueprintType)
enum class EInventoryDeliveryReason : uint8
{
	Unspecified,
	Reward,
	ScriptedGrant,
	StagingReturn,
	TradeReturn,
	EquipmentReturn
};

UENUM(BlueprintType)
enum class EInventoryDeliveryOutcome : uint8
{
	Placed,
	Queued,
	Rejected
};

UENUM(BlueprintType)
enum class EInventoryDeliveryDestinationKind : uint8
{
	Automatic,
	Bag,
	Equipment
};

/** Server-authoritative destination requested by a pending-delivery claim. */
USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FInventoryDeliveryDestination
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	EInventoryDeliveryDestinationKind Kind = EInventoryDeliveryDestinationKind::Automatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	EBagSlot Bag = EBagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	int32 TopLeft = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	EEquipmentSlot EquipmentSlot = EEquipmentSlot::Unknown;

	static FInventoryDeliveryDestination MakeBag(EBagSlot InBag, int32 InTopLeft)
	{
		FInventoryDeliveryDestination Result;
		Result.Kind = EInventoryDeliveryDestinationKind::Bag;
		Result.Bag = InBag;
		Result.TopLeft = InTopLeft;
		return Result;
	}

	static FInventoryDeliveryDestination MakeEquipment(EEquipmentSlot InSlot)
	{
		FInventoryDeliveryDestination Result;
		Result.Kind = EInventoryDeliveryDestinationKind::Equipment;
		Result.EquipmentSlot = InSlot;
		return Result;
	}
};

USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FInventoryDeliveryRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	FGuid DeliveryId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	float Durability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	EInventoryDeliveryReason Reason = EInventoryDeliveryReason::Unspecified;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	EBagSlot PreferredBag = EBagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	int32 PreferredTopLeft = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Delivery")
	bool bAllowAutoEquip = false;
};

USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FPendingInventoryDelivery : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	FGuid DeliveryId;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	int32 ItemID = -1;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	float Durability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	EInventoryDeliveryReason Reason = EInventoryDeliveryReason::Unspecified;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	int64 CreatedAtUnixMs = 0;

	bool IsValid() const { return DeliveryId.IsValid() && ItemID > 0; }
};

USTRUCT()
struct INVENTORYPLUGIN_API FPendingInventoryDeliveryArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPendingInventoryDelivery> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<class UInventoryDeliveryComponent> Owner = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FPendingInventoryDelivery, FPendingInventoryDeliveryArray>(Items, DeltaParms, *this);
	}

	/** Notify listeners only after the complete replicated delta, including removals, has been applied. */
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);
};

template<>
struct TStructOpsTypeTraits<FPendingInventoryDeliveryArray> : TStructOpsTypeTraitsBase2<FPendingInventoryDeliveryArray>
{
	enum { WithNetDeltaSerializer = true };
};
