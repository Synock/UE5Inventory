#include "Components/InventoryDeliveryComponent.h"

#include "InventoryPlugin.h"
#include "InventoryUtilities.h"
#include "Components/InventoryComponent.h"
#include "Components/EquipmentComponent.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "Items/InventoryItemEquipable.h"
#include "Net/UnrealNetwork.h"

UInventoryDeliveryComponent::UInventoryDeliveryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
	PendingDeliveries.Owner = this;
}

void UInventoryDeliveryComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwner());
	PendingDeliveries.Owner = this;
}

void UInventoryDeliveryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInventoryDeliveryComponent, PendingDeliveries, COND_OwnerOnly);
}

void FPendingInventoryDeliveryArray::PostReplicatedReceive(
	const FFastArraySerializer::FPostReplicatedReceiveParameters&)
{
	if (Owner) Owner->BroadcastChanged();
}

void UInventoryDeliveryComponent::BroadcastChanged()
{
	DeliveriesChanged.Broadcast();
}

EInventoryDeliveryOutcome UInventoryDeliveryComponent::TryDeliverOrQueue(FInventoryDeliveryRequest Request)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !PlayerInterface || Request.ItemID <= 0)
		return EInventoryDeliveryOutcome::Rejected;

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Request.ItemID, GetWorld());
	if (!Item)
		return EInventoryDeliveryOutcome::Rejected;

	const UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponentConst();
	if (!Inventory)
		return EInventoryDeliveryOutcome::Rejected;

	if (Request.PreferredBag != EBagSlot::Unknown && Request.PreferredTopLeft >= 0 &&
		Inventory->CanPlaceItemAt(Request.PreferredBag, Item, Request.PreferredTopLeft))
	{
		PlayerInterface->PlayerAddItemWithDurability(Request.PreferredTopLeft, Request.PreferredBag,
			Request.ItemID, Request.Durability);
		return EInventoryDeliveryOutcome::Placed;
	}

	if (Request.bAllowAutoEquip)
	{
		EEquipmentSlot EquipmentSlot = EEquipmentSlot::Unknown;
		if (PlayerInterface->PlayerTryAutoEquip(Request.ItemID, EquipmentSlot) && EquipmentSlot != EEquipmentSlot::Unknown)
		{
			if (IEquipmentInterface* Equipment = Cast<IEquipmentInterface>(PlayerInterface->GetInventoryOwningActor()))
			{
				Equipment->EquipItemWithDurability(EquipmentSlot, Request.ItemID, Request.Durability);
				return EInventoryDeliveryOutcome::Placed;
			}
		}
	}

	int32 TopLeft = -1;
	const EBagSlot Bag = PlayerInterface->GetInventoryComponentConst()->FindSuitableSlot(Item, TopLeft);
	if (Bag != EBagSlot::Unknown && TopLeft >= 0)
	{
		PlayerInterface->PlayerAddItemWithDurability(TopLeft, Bag, Request.ItemID, Request.Durability);
		return EInventoryDeliveryOutcome::Placed;
	}

	if (!Request.DeliveryId.IsValid())
		Request.DeliveryId = FGuid::NewGuid();
	else if (FindDelivery(Request.DeliveryId))
		return EInventoryDeliveryOutcome::Queued;

	FPendingInventoryDelivery Pending;
	Pending.DeliveryId = Request.DeliveryId;
	Pending.ItemID = Request.ItemID;
	Pending.Durability = Request.Durability;
	Pending.Reason = Request.Reason;
	const FDateTime Now = FDateTime::UtcNow();
	Pending.CreatedAtUnixMs = (Now.GetTicks() - FDateTime(1970, 1, 1).GetTicks()) / ETimespan::TicksPerMillisecond;
	PendingDeliveries.Items.Add(Pending);
	PendingDeliveries.MarkItemDirty(PendingDeliveries.Items.Last());

	if (PendingDeliveries.Items.Num() > OperationalWarningThreshold)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Pending delivery queue for %s contains %d items"),
			*GetOwner()->GetName(), PendingDeliveries.Items.Num());
	}

	DeliveryQueuedServer.Broadcast(Pending);
	BroadcastChanged();
	return EInventoryDeliveryOutcome::Queued;
}

float UInventoryDeliveryComponent::GetPendingDeliveryWeight() const
{
	float Weight = 0.0f;
	for (const FPendingInventoryDelivery& Delivery : PendingDeliveries.Items)
		if (const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld()))
			Weight += FMath::Max(0.0f, Item->GetWeight());
	return Weight;
}

bool UInventoryDeliveryComponent::HasPendingDeliveryReason(EInventoryDeliveryReason Reason) const
{
	return PendingDeliveries.Items.ContainsByPredicate([Reason](const FPendingInventoryDelivery& Delivery)
	{
		return Delivery.Reason == Reason;
	});
}

bool UInventoryDeliveryComponent::ReserveBagDestination(const FGuid& DeliveryId, EBagSlot& OutBag, int32& OutTopLeft)
{
	const FPendingInventoryDelivery* Delivery = FindDelivery(DeliveryId);
	if (!Delivery || !FindBagDestination(*Delivery, OutBag, OutTopLeft) || !PlayerInterface)
		return false;
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery->ItemID, GetWorld());
	UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent();
	return Inventory && Inventory->ReserveItemFootprint(DeliveryId, OutBag, Item, OutTopLeft);
}

bool UInventoryDeliveryComponent::ReserveDestination(const FGuid& DeliveryId,
	FInventoryDeliveryDestination& InOutDestination)
{
	const FPendingInventoryDelivery* Delivery = FindDelivery(DeliveryId);
	if (!Delivery || !PlayerInterface)
		return false;

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery->ItemID, GetWorld());
	if (!Item)
		return false;

	if (InOutDestination.Kind == EInventoryDeliveryDestinationKind::Automatic)
	{
		EEquipmentSlot EquipmentSlot = EEquipmentSlot::Unknown;
		if (PlayerInterface->PlayerTryAutoEquip(Delivery->ItemID, EquipmentSlot) &&
			EquipmentSlot != EEquipmentSlot::Unknown)
		{
			const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item);
			IEquipmentInterface* EquipmentInterface = PlayerInterface->GetEquipmentForInventory();
			UEquipmentComponent* Equipment = EquipmentInterface ? EquipmentInterface->GetEquipmentComponent() : nullptr;
			if (Equipment && Equipable && Equipment->ReservePendingDelivery(DeliveryId, Equipable, EquipmentSlot))
			{
				InOutDestination = FInventoryDeliveryDestination::MakeEquipment(EquipmentSlot);
				return true;
			}
		}

		if (!FindBagDestination(*Delivery, InOutDestination.Bag, InOutDestination.TopLeft))
			return false;
		InOutDestination.Kind = EInventoryDeliveryDestinationKind::Bag;
		UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent();
		return Inventory && Inventory->ReserveItemFootprint(DeliveryId, InOutDestination.Bag, Item,
			InOutDestination.TopLeft);
	}

	if (InOutDestination.Kind == EInventoryDeliveryDestinationKind::Bag)
	{
		UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent();
		return Inventory && Inventory->ReserveItemFootprint(DeliveryId, InOutDestination.Bag, Item,
			InOutDestination.TopLeft);
	}

	if (InOutDestination.Kind == EInventoryDeliveryDestinationKind::Equipment)
	{
		const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item);
		IEquipmentInterface* EquipmentInterface = PlayerInterface->GetEquipmentForInventory();
		UEquipmentComponent* Equipment = EquipmentInterface ? EquipmentInterface->GetEquipmentComponent() : nullptr;
		return Equipment && Equipable && Equipment->ReservePendingDelivery(DeliveryId, Equipable,
			InOutDestination.EquipmentSlot);
	}
	return false;
}

void UInventoryDeliveryComponent::ReleaseBagDestination(const FGuid& DeliveryId)
{
	if (PlayerInterface && PlayerInterface->GetInventoryComponent())
		PlayerInterface->GetInventoryComponent()->ReleaseItemFootprint(DeliveryId);
}

void UInventoryDeliveryComponent::ReleaseDestination(const FGuid& DeliveryId)
{
	ReleaseBagDestination(DeliveryId);
	if (PlayerInterface)
	{
		if (IEquipmentInterface* EquipmentInterface = PlayerInterface->GetEquipmentForInventory())
			if (UEquipmentComponent* Equipment = EquipmentInterface->GetEquipmentComponent())
				Equipment->ReleasePendingDeliveryReservation(DeliveryId);
	}
}

bool UInventoryDeliveryComponent::FindBagDestination(const FPendingInventoryDelivery& Delivery,
	EBagSlot& OutBag, int32& OutTopLeft) const
{
	OutBag = EBagSlot::Unknown;
	OutTopLeft = -1;
	if (!PlayerInterface || !Delivery.IsValid())
		return false;

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld());
	if (!Item)
		return false;

	OutBag = PlayerInterface->GetInventoryComponentConst()->FindSuitableSlot(Item, OutTopLeft);
	return OutBag != EBagSlot::Unknown && OutTopLeft >= 0;
}

bool UInventoryDeliveryComponent::CommitClaim(const FGuid& DeliveryId, EBagSlot Bag, int32 TopLeft,
	bool bBroadcastPersistence)
{
	return CommitClaim(DeliveryId, FInventoryDeliveryDestination::MakeBag(Bag, TopLeft), bBroadcastPersistence);
}

bool UInventoryDeliveryComponent::CommitClaim(const FGuid& DeliveryId,
	const FInventoryDeliveryDestination& Destination, bool bBroadcastPersistence)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !PlayerInterface)
		return false;
	const auto FailCommit = [this, &DeliveryId]()
	{
		ReleaseDestination(DeliveryId);
		return false;
	};

	const int32 Index = PendingDeliveries.Items.IndexOfByPredicate(
		[&DeliveryId](const FPendingInventoryDelivery& Entry) { return Entry.DeliveryId == DeliveryId; });
	if (Index == INDEX_NONE)
		return FailCommit();

	const FPendingInventoryDelivery Delivery = PendingDeliveries.Items[Index];
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld());
	if (!Item)
		return FailCommit();

	bool bCommitted = false;
	if (Destination.Kind == EInventoryDeliveryDestinationKind::Bag &&
		Destination.Bag != EBagSlot::Unknown && Destination.TopLeft >= 0)
	{
		UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent();
		if (!Inventory)
			return FailCommit();
		const bool bHadReservation = Inventory->HasItemFootprintReservation(DeliveryId);
		if (bHadReservation)
			Inventory->ReleaseItemFootprint(DeliveryId);
		if (Inventory->CanPlaceItemAt(Destination.Bag, Item, Destination.TopLeft))
		{
			PlayerInterface->PlayerAddItemWithDurability(Destination.TopLeft, Destination.Bag,
				Delivery.ItemID, Delivery.Durability);
			bCommitted = PlayerInterface->PlayerGetItem(Destination.TopLeft, Destination.Bag) == Delivery.ItemID;
		}
		if (!bCommitted && bHadReservation)
			Inventory->ReserveItemFootprint(DeliveryId, Destination.Bag, Item, Destination.TopLeft);
	}
	else if (Destination.Kind == EInventoryDeliveryDestinationKind::Equipment)
	{
		const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item);
		IEquipmentInterface* EquipmentInterface = PlayerInterface->GetEquipmentForInventory();
		UEquipmentComponent* Equipment = EquipmentInterface ? EquipmentInterface->GetEquipmentComponent() : nullptr;
		if (!Equipment || !Equipable)
			return FailCommit();
		const bool bHadReservation = Equipment->HasPendingDeliveryReservation(DeliveryId);
		if (bHadReservation)
			Equipment->ReleasePendingDeliveryReservation(DeliveryId);
		if (Equipment->CanEquipItemAt(Equipable, Destination.EquipmentSlot))
		{
			Equipment->EquipItemWithDurability(Equipable, Destination.EquipmentSlot, Delivery.Durability);
			bCommitted = EquipmentInterface->GetEquippedItem(Destination.EquipmentSlot) == Equipable;
		}
		if (!bCommitted && bHadReservation)
			Equipment->ReservePendingDelivery(DeliveryId, Equipable, Destination.EquipmentSlot);
	}

	if (!bCommitted)
		return FailCommit();
	PendingDeliveries.Items.RemoveAt(Index);
	PendingDeliveries.MarkArrayDirty();
	if (bBroadcastPersistence)
		DeliveryClaimedServer.Broadcast(DeliveryId);
	BroadcastChanged();
	return true;
}

void UInventoryDeliveryComponent::AddLoadedDelivery(const FPendingInventoryDelivery& Delivery)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Delivery.IsValid() || FindDelivery(Delivery.DeliveryId))
		return;
	PendingDeliveries.Items.Add(Delivery);
	PendingDeliveries.MarkItemDirty(PendingDeliveries.Items.Last());
}

void UInventoryDeliveryComponent::SortLoadedDeliveries()
{
	PendingDeliveries.Items.StableSort([](const FPendingInventoryDelivery& A, const FPendingInventoryDelivery& B)
	{
		if (A.CreatedAtUnixMs != B.CreatedAtUnixMs)
			return A.CreatedAtUnixMs < B.CreatedAtUnixMs;
		return A.DeliveryId.ToString() < B.DeliveryId.ToString();
	});
	PendingDeliveries.MarkArrayDirty();
	BroadcastChanged();
}

const FPendingInventoryDelivery* UInventoryDeliveryComponent::FindDelivery(const FGuid& DeliveryId) const
{
	return PendingDeliveries.Items.FindByPredicate(
		[&DeliveryId](const FPendingInventoryDelivery& Entry) { return Entry.DeliveryId == DeliveryId; });
}
