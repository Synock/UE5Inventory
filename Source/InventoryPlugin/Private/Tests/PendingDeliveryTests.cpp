#if WITH_AUTOMATION_WORKER

#include "Misc/AutomationTest.h"
#include "Components/InventoryDeliveryComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/StagingAreaComponent.h"
#include "UI/StagingAreaSlotWidget.h"
#include "Components/TradeComponent.h"
#include "Components/TradeReturnRouting.h"
#include "Components/StagingReturnRouting.h"
#include "BagStorage.h"
#include "GameFramework/Actor.h"
#include "Items/InventoryItemBase.h"
#include "Items/InventoryItemEquipable.h"
#include "UI/PendingDeliveryWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryLoadOrderTest,
	"InventoryPlugin.Delivery.LoadOrderAndDeduplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryLoadOrderTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UInventoryDeliveryComponent* Component = NewObject<UInventoryDeliveryComponent>(Owner);

	FPendingInventoryDelivery Later;
	Later.DeliveryId = FGuid::NewGuid(); Later.ItemID = 2; Later.CreatedAtUnixMs = 200;
	FPendingInventoryDelivery Earlier;
	Earlier.DeliveryId = FGuid::NewGuid(); Earlier.ItemID = 1; Earlier.CreatedAtUnixMs = 100;

	Component->AddLoadedDelivery(Later);
	Component->AddLoadedDelivery(Earlier);
	Component->AddLoadedDelivery(Earlier);
	Component->SortLoadedDeliveries();

	TestEqual(TEXT("Duplicate delivery IDs are ignored"), Component->GetPendingDeliveryCount(), 2);
	TestEqual(TEXT("Loaded deliveries are FIFO"), Component->GetPendingDeliveries()[0].ItemID, 1);
	TestTrue(TEXT("Lookup uses the immutable delivery ID"), Component->FindDelivery(Later.DeliveryId) != nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryReplicatedRemovalNotificationTest,
	"InventoryPlugin.Delivery.ReplicatedFinalRemovalCollapsesTray",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryReplicatedRemovalNotificationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UInventoryDeliveryComponent* Component = NewObject<UInventoryDeliveryComponent>(Owner);
	UPendingDeliveryWidget* Tray = NewObject<UPendingDeliveryWidget>();
	Tray->SetVisibility(ESlateVisibility::Visible);
	Component->DeliveriesChanged.AddDynamic(Tray, &UPendingDeliveryWidget::Refresh);

	FPendingInventoryDeliveryArray ReplicatedArray;
	ReplicatedArray.Owner = Component;
	const FFastArraySerializer::FPostReplicatedReceiveParameters ReceiveParameters{1, false};
	ReplicatedArray.PostReplicatedReceive(ReceiveParameters);

	TestTrue(TEXT("The received queue is empty after the final replicated removal"),
		ReplicatedArray.Items.IsEmpty());
	TestEqual(TEXT("The post-receive notification collapses the stale tray"), Tray->GetVisibility(),
		ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryGridReservationTest,
	"InventoryPlugin.Delivery.GridReservationAndEmptySentinel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryGridReservationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Inventory empty cells use INDEX_NONE"), UInventoryComponent::IsEmptyItemId(INDEX_NONE));
	TestFalse(TEXT("Item zero is not the empty sentinel"), UInventoryComponent::IsEmptyItemId(0));

	AActor* Owner = NewObject<AActor>();
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Owner);
	UInventoryItemBase* WideItem = NewObject<UInventoryItemBase>();
	WideItem->Width = 2; WideItem->Height = 1; WideItem->ItemSize = EItemSize::Small;
	UInventoryItemBase* SmallItem = NewObject<UInventoryItemBase>();
	SmallItem->Width = 1; SmallItem->Height = 1; SmallItem->ItemSize = EItemSize::Tiny;
	const FGuid ReservationId = FGuid::NewGuid();

	TestTrue(TEXT("A valid complete footprint can be reserved"),
		Inventory->ReserveItemFootprint(ReservationId, EBagSlot::Pocket1, WideItem, 0));
	TestFalse(TEXT("Reservations block every covered cell"),
		Inventory->CanPlaceItemAt(EBagSlot::Pocket1, SmallItem, 1));
	TestTrue(TEXT("The containing bag reports active reserved capacity"),
		Inventory->HasReservationsInBag(EBagSlot::Pocket1));
	TArray<UInventoryItemBase*> IncomingItems{SmallItem};
	TArray<FInventoryDeliveryDestination> Placements;
	TestTrue(TEXT("Placement planning can use remaining capacity"),
		Inventory->FindPlacementsForItems(IncomingItems, Placements));
	TestTrue(TEXT("Placement planning never selects an escrow-reserved cell"),
		Placements.Num() == 1 && Placements[0].TopLeft != 0 && Placements[0].TopLeft != 1);
	TestTrue(TEXT("Reservations do not block unrelated cells"),
		Inventory->CanPlaceItemAt(EBagSlot::Pocket1, SmallItem, 2));
	Inventory->ReleaseItemFootprint(ReservationId);
	TestTrue(TEXT("Released cells become available again"),
		Inventory->CanPlaceItemAt(EBagSlot::Pocket1, SmallItem, 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStagingEscrowCapacityAndWeightTest,
	"InventoryPlugin.Staging.EscrowCapacityAndWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStagingEscrowCapacityAndWeightTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UStagingAreaComponent* Staging = NewObject<UStagingAreaComponent>(Owner);
	for (int32 Index = 0; Index < UStagingAreaComponent::MaxStagedItems; ++Index)
	{
		FInventoryEscrowItem Item;
		Item.ItemID = 1000 + Index;
		Item.Durability = 10.0f + Index;
		Item.Source = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket1, Index);
		Item.ReservationId = FGuid::NewGuid();
		Item.EffectiveWeight = 1.25f;
		TestTrue(FString::Printf(TEXT("Staged item %d is accepted within the server limit"), Index),
			Staging->AddItemToStagingArea(Item));
	}
	FInventoryEscrowItem Ninth;
	Ninth.ItemID = 2000;
	Ninth.Source = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket2, 0);
	Ninth.ReservationId = FGuid::NewGuid();
	Ninth.EffectiveWeight = 99.0f;
	TestFalse(TEXT("The ninth staged item is rejected authoritatively"),
		Staging->AddItemToStagingArea(Ninth));
	TestEqual(TEXT("Rejected staging does not change escrow contents"),
		Staging->GetStagingAreaItems().Num(), UStagingAreaComponent::MaxStagedItems);
	TestEqual(TEXT("Escrow retains the effective carried weight of all staged items"),
		Staging->GetEscrowWeight(), 10.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTradeCoinEscrowWeightTest,
	"InventoryPlugin.Trade.CoinEscrowRetainsWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTradeCoinEscrowWeightTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UTradeComponent* Trade = NewObject<UTradeComponent>(Owner);
	FCoinValue Coins;
	Coins.CopperPieces = 100;
	Trade->GetOurCoinComponent()->AddCoins(Coins);
	TestTrue(TEXT("Offered currency remains part of carried weight"), Trade->GetEscrowWeight() > 0.0f);
	TestEqual(TEXT("Trade escrow reports the coin component's exact weight"), Trade->GetEscrowWeight(),
		Trade->GetOurCoinComponent()->GetTotalWeight());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryReturnReasonGuardTest,
	"InventoryPlugin.Delivery.ReturnReasonGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryReturnReasonGuardTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UInventoryDeliveryComponent* Deliveries = NewObject<UInventoryDeliveryComponent>(Owner);
	FPendingInventoryDelivery Reward;
	Reward.DeliveryId = FGuid::NewGuid();
	Reward.ItemID = 1;
	Reward.Reason = EInventoryDeliveryReason::Reward;
	Deliveries->AddLoadedDelivery(Reward);
	TestFalse(TEXT("Rewards do not masquerade as player-return overflow"),
		Deliveries->HasPendingDeliveryReason(EInventoryDeliveryReason::TradeReturn));
	FPendingInventoryDelivery Return = Reward;
	Return.DeliveryId = FGuid::NewGuid();
	Return.ItemID = 2;
	Return.Reason = EInventoryDeliveryReason::StagingReturn;
	Deliveries->AddLoadedDelivery(Return);
	TestTrue(TEXT("A staged return is discoverable for transaction gating"),
		Deliveries->HasPendingDeliveryReason(EInventoryDeliveryReason::StagingReturn));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryDestinationTest,
	"InventoryPlugin.Delivery.ExplicitDestinationPayload",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryDestinationTest::RunTest(const FString& Parameters)
{
	const FInventoryDeliveryDestination Bag = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket2, 17);
	TestEqual(TEXT("Bag destination kind is explicit"), Bag.Kind, EInventoryDeliveryDestinationKind::Bag);
	TestEqual(TEXT("Bag destination retains the exact bag"), Bag.Bag, EBagSlot::Pocket2);
	TestEqual(TEXT("Bag destination retains the exact top-left"), Bag.TopLeft, 17);

	const FInventoryDeliveryDestination Equipment =
		FInventoryDeliveryDestination::MakeEquipment(EEquipmentSlot::Head);
	TestEqual(TEXT("Equipment destination kind is explicit"), Equipment.Kind,
		EInventoryDeliveryDestinationKind::Equipment);
	TestEqual(TEXT("Equipment destination retains the exact slot"), Equipment.EquipmentSlot,
		EEquipmentSlot::Head);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPendingDeliveryEquipmentReservationTest,
	"InventoryPlugin.Delivery.EquipmentReservationAndCompatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPendingDeliveryEquipmentReservationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* HeadItem = NewObject<UInventoryItemEquipable>();
	HeadItem->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Head);
	UInventoryItemEquipable* PrimaryItem = NewObject<UInventoryItemEquipable>();
	PrimaryItem->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Primary);

	TestTrue(TEXT("Compatible empty equipment slot accepts a pending delivery"),
		Equipment->CanEquipItemAt(HeadItem, EEquipmentSlot::Head));
	TestFalse(TEXT("Incompatible equipment slot is rejected"),
		Equipment->CanEquipItemAt(PrimaryItem, EEquipmentSlot::Head));

	const FGuid Reservation = FGuid::NewGuid();
	TestTrue(TEXT("Compatible equipment footprint can be reserved"),
		Equipment->ReservePendingDelivery(Reservation, HeadItem, EEquipmentSlot::Head));
	TestTrue(TEXT("The target equipment slot is reserved"),
		Equipment->IsEquipmentSlotReserved(EEquipmentSlot::Head));
	TestEqual(TEXT("Auto equipment selection skips a reserved single-slot item"),
		Equipment->FindSuitableSlot(HeadItem), EEquipmentSlot::Unknown);
	TestFalse(TEXT("A second claim cannot race the reservation"),
		Equipment->ReservePendingDelivery(FGuid::NewGuid(), HeadItem, EEquipmentSlot::Head));
	Equipment->ReleasePendingDeliveryReservation(Reservation);
	TestFalse(TEXT("Releasing a failed claim clears its equipment reservation"),
		Equipment->IsEquipmentSlotReserved(EEquipmentSlot::Head));

	Equipment->EquipItemWithDurability(HeadItem, EEquipmentSlot::Head, 42.5f);
	TestFalse(TEXT("An occupied equipment slot rejects a delivery"),
		Equipment->CanEquipItemAt(HeadItem, EEquipmentSlot::Head));
	float Durability = 0.0f;
	TestTrue(TEXT("Equipped durability is readable"),
		Equipment->GetEquipmentDurability(EEquipmentSlot::Head, Durability));
	TestEqual(TEXT("Equipment claim preserves exact durability"), Durability, 42.5f);

	UEquipmentComponent* RingEquipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* RingItem = NewObject<UInventoryItemEquipable>();
	RingItem->EquipableSlotBitMask = (1 << static_cast<uint8>(EEquipmentSlot::FingerL)) |
		(1 << static_cast<uint8>(EEquipmentSlot::FingerR));
	const FGuid RingReservation = FGuid::NewGuid();
	TestTrue(TEXT("A delivery can reserve the first legal duplicate equipment slot"),
		RingEquipment->ReservePendingDelivery(RingReservation, RingItem, EEquipmentSlot::FingerL));
	TestEqual(TEXT("Auto equipment selection falls through to the next legal duplicate slot"),
		RingEquipment->FindSuitableSlot(RingItem), EEquipmentSlot::FingerR);
	const FGuid SecondRingReservation = FGuid::NewGuid();
	TestTrue(TEXT("The second duplicate equipment slot can also be reserved"),
		RingEquipment->ReservePendingDelivery(SecondRingReservation, RingItem, EEquipmentSlot::FingerR));
	TestEqual(TEXT("Auto equipment selection reports no fit when all legal duplicate slots are reserved"),
		RingEquipment->FindSuitableSlot(RingItem), EEquipmentSlot::Unknown);

	UEquipmentComponent* MultiSlotEquipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* MultiSlotBag = NewObject<UInventoryItemEquipable>();
	MultiSlotBag->MultiSlotItem = true;
	MultiSlotBag->EquipableSlotBitMask = (1 << static_cast<uint8>(EEquipmentSlot::WaistBag1)) |
		(1 << static_cast<uint8>(EEquipmentSlot::WaistBag2));
	TestEqual(TEXT("Auto equipment selection chooses the primary slot for an open multi-slot item"),
		MultiSlotEquipment->FindSuitableSlot(MultiSlotBag), EEquipmentSlot::WaistBag1);
	const FGuid MultiReservation = FGuid::NewGuid();
	TestTrue(TEXT("A multi-slot delivery reserves its complete equipment footprint"),
		MultiSlotEquipment->ReservePendingDelivery(MultiReservation, MultiSlotBag, EEquipmentSlot::WaistBag1));
	TestTrue(TEXT("The secondary multi-slot position is also reserved"),
		MultiSlotEquipment->IsEquipmentSlotReserved(EEquipmentSlot::WaistBag2));
	TestFalse(TEXT("A multi-slot item cannot target its secondary visual slot"),
		MultiSlotEquipment->CanEquipItemAt(MultiSlotBag, EEquipmentSlot::WaistBag2, MultiReservation));
	TestEqual(TEXT("Auto equipment selection reports no fit for a reserved multi-slot item"),
		MultiSlotEquipment->FindSuitableSlot(MultiSlotBag), EEquipmentSlot::Unknown);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiSlotEquipmentOccupancyTest,
	"InventoryPlugin.Equipment.MultiSlotOccupancy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiSlotEquipmentOccupancyTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UInventoryItemEquipable* TwoHandedWeapon = NewObject<UInventoryItemEquipable>();
	TwoHandedWeapon->Weapon = true;
	TwoHandedWeapon->MultiSlotItem = true;
	TwoHandedWeapon->EquipableSlotBitMask = (1 << static_cast<uint8>(EEquipmentSlot::Primary)) |
		(1 << static_cast<uint8>(EEquipmentSlot::Secondary));

	UInventoryItemEquipable* Torch = NewObject<UInventoryItemEquipable>();
	Torch->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Secondary);

	UInventoryItemEquipable* OneHandedWeapon = NewObject<UInventoryItemEquipable>();
	OneHandedWeapon->Weapon = true;
	OneHandedWeapon->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Primary);

	UEquipmentComponent* WeaponFirstEquipment = NewObject<UEquipmentComponent>(Owner);
	WeaponFirstEquipment->EquipItem(TwoHandedWeapon, EEquipmentSlot::Primary);
	TestFalse(TEXT("An equipped two-handed weapon blocks the secondary slot"),
		WeaponFirstEquipment->CanEquipItemAt(Torch, EEquipmentSlot::Secondary));
	TestEqual(TEXT("Auto-equip cannot select the blocked secondary slot"),
		WeaponFirstEquipment->FindSuitableSlot(Torch), EEquipmentSlot::Unknown);
	TestFalse(TEXT("A pending delivery cannot reserve the blocked secondary slot"),
		WeaponFirstEquipment->ReservePendingDelivery(FGuid::NewGuid(), Torch, EEquipmentSlot::Secondary));
	TestTrue(TEXT("The two-handed weapon can be removed from its stored primary slot"),
		WeaponFirstEquipment->RemoveItem(EEquipmentSlot::Primary));
	TestEqual(TEXT("Removing the two-handed weapon makes the torch eligible again"),
		WeaponFirstEquipment->FindSuitableSlot(Torch), EEquipmentSlot::Secondary);

	UEquipmentComponent* TorchFirstEquipment = NewObject<UEquipmentComponent>(Owner);
	TorchFirstEquipment->EquipItem(Torch, EEquipmentSlot::Secondary);
	TestFalse(TEXT("An equipped torch blocks the complete two-handed weapon footprint"),
		TorchFirstEquipment->CanEquipItemAt(TwoHandedWeapon, EEquipmentSlot::Primary));

	UEquipmentComponent* OneHandedEquipment = NewObject<UEquipmentComponent>(Owner);
	OneHandedEquipment->EquipItem(OneHandedWeapon, EEquipmentSlot::Primary);
	TestEqual(TEXT("A one-handed primary weapon leaves the torch slot available"),
		OneHandedEquipment->FindSuitableSlot(Torch), EEquipmentSlot::Secondary);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStagingSingleItemRemovalTest,
	"InventoryPlugin.Staging.Regression.SingleItemRemovalPreservesOtherEscrow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStagingSingleItemRemovalTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	UStagingAreaComponent* Staging = NewObject<UStagingAreaComponent>(Owner);
	FInventoryEscrowItem First;
	First.ItemID = 70001;
	First.Source = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket1, 0);
	First.ReservationId = FGuid::NewGuid();
	FInventoryEscrowItem Second;
	Second.ItemID = 70002;
	Second.Source = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket2, 0);
	Second.ReservationId = FGuid::NewGuid();

	TestTrue(TEXT("First escrow item can be staged"), Staging->AddItemToStagingArea(First));
	TestTrue(TEXT("Second escrow item can be staged"), Staging->AddItemToStagingArea(Second));
	TestTrue(TEXT("Selected escrow item can be removed by reservation"),
		Staging->RemoveItemFromStagingArea(First.ReservationId));
	TestEqual(TEXT("Removing one staged item preserves the other slot"),
		Staging->GetStagingAreaItems().Num(), 1);
	TestEqual(TEXT("The unselected escrow item remains staged"),
		Staging->GetStagingAreaItems()[0].ReservationId, Second.ReservationId);
	TestFalse(TEXT("A stale reservation cannot remove another staged item"),
		Staging->RemoveItemFromStagingArea(First.ReservationId));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStagingSlotNativeDragAssetContractTest,
	"InventoryPlugin.Staging.UI.StagingSlotUsesNativeDragReturn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStagingSlotNativeDragAssetContractTest::RunTest(const FString& Parameters)
{
	UClass* SlotClass = LoadClass<UStagingAreaSlotWidget>(
		nullptr, TEXT("/InventoryPlugin/UI/UI_StagingAreaSlot.UI_StagingAreaSlot_C"));
	TestNotNull(TEXT("Authored staging slot asset loads"), SlotClass);
	if (SlotClass)
	{
		TestTrue(TEXT("Authored staging slot uses the native drag-and-return implementation"),
			SlotClass->IsChildOf(UStagingAreaSlotWidget::StaticClass()));
	}
	return SlotClass != nullptr;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTradeCancellationRemovedBackpackRegressionTest,
	"InventoryPlugin.Trade.Regression.RemovedBackpackUsesOverflow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTradeCancellationRemovedBackpackRegressionTest::RunTest(const FString& Parameters)
{
	FTradeItemSlot EscrowedItem;
	EscrowedItem.ItemID = 70000;
	EscrowedItem.Durability = 37.5f;
	EscrowedItem.SourceBagSlot = EBagSlot::BackPack1;
	EscrowedItem.SourceTopLeft = 0;

	bool bDeliveryWasInvoked = false;
	const bool bReturned = InventoryPlugin::TradeReturn::Route(EscrowedItem,
		[this, &bDeliveryWasInvoked](const FInventoryDeliveryRequest& Request)
		{
			bDeliveryWasInvoked = true;
			TestEqual(TEXT("The escrowed item ID is preserved"), Request.ItemID, 70000);
			TestEqual(TEXT("The escrowed item's exact durability is preserved"), Request.Durability, 37.5f);
			TestEqual(TEXT("A trade cancellation is identified as a trade return"), Request.Reason,
				EInventoryDeliveryReason::TradeReturn);
			TestEqual(TEXT("The removed backpack is only supplied as the preferred bag"), Request.PreferredBag,
				EBagSlot::BackPack1);
			TestEqual(TEXT("The original cell is only supplied as the preferred cell"), Request.PreferredTopLeft, 0);
			// A missing/invalid preferred bag makes TryDeliverOrQueue choose another bag or overflow.
			return EInventoryDeliveryOutcome::Queued;
		});

	TestTrue(TEXT("Trade cancellation always invokes protected delivery"), bDeliveryWasInvoked);
	TestTrue(TEXT("Queueing counts as a lossless escrow return"), bReturned);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStagingCancellationProtectedReturnTest,
	"InventoryPlugin.Staging.Regression.CancellationUsesProtectedReturn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStagingCancellationProtectedReturnTest::RunTest(const FString& Parameters)
{
	FInventoryEscrowItem StagedItem;
	StagedItem.ItemID = 70001;
	StagedItem.Durability = 28.75f;
	StagedItem.Source = FInventoryDeliveryDestination::MakeBag(EBagSlot::Pocket1, 0);
	StagedItem.ReservationId = FGuid::NewGuid();
	bool bDeliveryWasInvoked = false;
	const bool bReturned = InventoryPlugin::StagingReturn::Route(StagedItem,
		[this, &bDeliveryWasInvoked](const FInventoryDeliveryRequest& Request)
		{
			bDeliveryWasInvoked = true;
			TestEqual(TEXT("Staging cancellation preserves the item ID"), Request.ItemID, 70001);
			TestEqual(TEXT("Staging cancellation preserves exact durability"), Request.Durability, 28.75f);
			TestEqual(TEXT("Staging cancellation identifies a Give return"), Request.Reason,
				EInventoryDeliveryReason::StagingReturn);
			TestTrue(TEXT("Staging returns retain auto-equip fallback"), Request.bAllowAutoEquip);
			return EInventoryDeliveryOutcome::Queued;
		});

	TestTrue(TEXT("Cancellation invokes protected delivery"), bDeliveryWasInvoked);
	TestTrue(TEXT("Overflow queueing counts as a successful staging return"), bReturned);
	TestFalse(TEXT("Rejected delivery remains unresolved"), InventoryPlugin::StagingReturn::Route(StagedItem,
		[](const FInventoryDeliveryRequest&) { return EInventoryDeliveryOutcome::Rejected; }));
	return true;
}

#endif
