#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryDelivery.h"
#include "InventoryDeliveryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryDeliveriesChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryDeliveryQueuedServer, const FPendingInventoryDelivery&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryDeliveryClaimedServer, const FGuid&);

class IInventoryPlayerInterface;

UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UInventoryDeliveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryDeliveryComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Delivery")
	FOnInventoryDeliveriesChanged DeliveriesChanged;

	FOnInventoryDeliveryQueuedServer DeliveryQueuedServer;
	FOnInventoryDeliveryClaimedServer DeliveryClaimedServer;

	UFUNCTION(BlueprintPure, Category="Inventory|Delivery")
	const TArray<FPendingInventoryDelivery>& GetPendingDeliveries() const { return PendingDeliveries.Items; }

	UFUNCTION(BlueprintPure, Category="Inventory|Delivery")
	int32 GetPendingDeliveryCount() const { return PendingDeliveries.Items.Num(); }

	UFUNCTION(BlueprintPure, Category="Inventory|Delivery")
	float GetPendingDeliveryWeight() const;

	bool HasPendingDeliveryReason(EInventoryDeliveryReason Reason) const;

	EInventoryDeliveryOutcome TryDeliverOrQueue(FInventoryDeliveryRequest Request);
	bool FindBagDestination(const FPendingInventoryDelivery& Delivery, EBagSlot& OutBag, int32& OutTopLeft) const;
	bool ReserveBagDestination(const FGuid& DeliveryId, EBagSlot& OutBag, int32& OutTopLeft);
	bool ReserveDestination(const FGuid& DeliveryId, FInventoryDeliveryDestination& InOutDestination);
	void ReleaseBagDestination(const FGuid& DeliveryId);
	void ReleaseDestination(const FGuid& DeliveryId);
	bool CommitClaim(const FGuid& DeliveryId, EBagSlot Bag, int32 TopLeft, bool bBroadcastPersistence = true);
	bool CommitClaim(const FGuid& DeliveryId, const FInventoryDeliveryDestination& Destination,
		bool bBroadcastPersistence = true);
	void AddLoadedDelivery(const FPendingInventoryDelivery& Delivery);
	void SortLoadedDeliveries();

	const FPendingInventoryDelivery* FindDelivery(const FGuid& DeliveryId) const;

protected:
	UPROPERTY(Replicated)
	FPendingInventoryDeliveryArray PendingDeliveries;

	UPROPERTY(EditDefaultsOnly, Category="Inventory|Delivery")
	int32 OperationalWarningThreshold = 32;

private:
	IInventoryPlayerInterface* PlayerInterface = nullptr;
	void BroadcastChanged();
	friend struct FPendingInventoryDeliveryArray;
};
