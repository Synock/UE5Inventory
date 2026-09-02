#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/CoinComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/ListView.h"
#include "Components/MerchantComponent.h"
#include "UI/Merchant/MerchantItemListWidget.h"
#include "UI/Merchant/MerchantSellWidget.h"
#include "UI/EquipmentSlotWidget.h"
#include "Items/InventoryItemBag.h"
#include "Items/InventoryItemEquipable.h"

//----------------------------------------------------------------------------------------------------------------------
// Merchant purse replication
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantPublicPurseSnapshotTest,
	"InventoryPlugin.Merchant.Regression.PublicPurseSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantPublicPurseSnapshotTest::RunTest(const FString& Parameters)
{
	UCoinComponent* CoinComponent = NewObject<UCoinComponent>(GetTransientPackage());
	if (!CoinComponent)
	{
		AddError(TEXT("Failed to create coin component"));
		return false;
	}

	const FCoinValue PrivateValue(1, 2, 3, 4);
	CoinComponent->SetPurseContentForTests(PrivateValue);
	TestFalse(TEXT("Coin components should default to owner-only purse replication"),
		CoinComponent->GetReplicatePurseToNonOwnersForTests());
	TestTrue(TEXT("Owner-only coin components should read the private purse"),
		CoinComponent->GetPurseContent().HasSameValue(PrivateValue));

	const FCoinValue MerchantValue(5, 6, 7, 8);
	CoinComponent->SetReplicatePurseToNonOwners(true);
	CoinComponent->SetPurseContentForTests(MerchantValue);

	TestTrue(TEXT("Merchant coin components can opt into public purse replication"),
		CoinComponent->GetReplicatePurseToNonOwnersForTests());
	TestTrue(TEXT("Public purse snapshot should track merchant purse content"),
		CoinComponent->GetPublicPurseContentForTests().HasSameValue(MerchantValue));
	TestTrue(TEXT("Public merchant purse should be used by display and CanPayAmount callers"),
		CoinComponent->GetPurseContent().HasSameValue(MerchantValue));

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Economy RPC validation
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantEconomyValidationAllowsStalePriceEchoTest,
	"InventoryPlugin.Merchant.Regression.ValidationAllowsStalePriceEcho",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantEconomyValidationAllowsStalePriceEchoTest::RunTest(const FString& Parameters)
{
	UInventoryNetComponent* NetComponent = NewObject<UInventoryNetComponent>(GetTransientPackage());
	if (!NetComponent)
	{
		AddError(TEXT("Failed to create inventory net component"));
		return false;
	}

	const FCoinValue ZeroEcho(0, 0, 0, 0);
	const FCoinValue StalePositiveEcho(9, 9, 9, 9);
	const FCoinValue NegativeEcho(-1, 0, 0, 0);

	TestTrue(TEXT("Buy validation should accept a non-negative stale price echo"),
		NetComponent->ValidatePlayerBuyFromMerchantForTests(100, StalePositiveEcho));
	TestTrue(TEXT("Buy validation should accept zero price echoes; server handler recomputes price"),
		NetComponent->ValidatePlayerBuyFromMerchantForTests(100, ZeroEcho));
	TestFalse(TEXT("Buy validation should still reject negative price payloads"),
		NetComponent->ValidatePlayerBuyFromMerchantForTests(100, NegativeEcho));
	TestFalse(TEXT("Buy validation should reject invalid item ids"),
		NetComponent->ValidatePlayerBuyFromMerchantForTests(-1, ZeroEcho));

	TestTrue(TEXT("Sell validation should accept stale non-negative price echoes"),
		NetComponent->ValidatePlayerSellToMerchantForTests(EBagSlot::Pocket1, 100, 0, StalePositiveEcho));
	TestFalse(TEXT("Sell validation should reject invalid source slots"),
		NetComponent->ValidatePlayerSellToMerchantForTests(EBagSlot::Pocket1, 100, -1, ZeroEcho));
	TestFalse(TEXT("Sell validation should reject negative price payloads"),
		NetComponent->ValidatePlayerSellToMerchantForTests(EBagSlot::Pocket1, 100, 0, NegativeEcho));

	TestTrue(TEXT("Equipped sell validation accepts a canonical slot and positive expected item id"),
		NetComponent->ValidatePlayerSellEquippedItemToMerchantForTests(EEquipmentSlot::Head, 100));
	TestFalse(TEXT("Equipped sell validation rejects the unknown slot"),
		NetComponent->ValidatePlayerSellEquippedItemToMerchantForTests(EEquipmentSlot::Unknown, 100));
	TestFalse(TEXT("Equipped sell validation rejects the sentinel slot"),
		NetComponent->ValidatePlayerSellEquippedItemToMerchantForTests(EEquipmentSlot::Last, 100));
	TestFalse(TEXT("Equipped sell validation rejects invalid expected item ids"),
		NetComponent->ValidatePlayerSellEquippedItemToMerchantForTests(EEquipmentSlot::Head, 0));

	TestTrue(TEXT("Repair validation should accept stale non-negative price echoes"),
		NetComponent->ValidatePlayerRepairEquipmentForTests(EEquipmentSlot::Head, StalePositiveEcho));
	TestFalse(TEXT("Repair validation should reject unknown equipment slots"),
		NetComponent->ValidatePlayerRepairEquipmentForTests(EEquipmentSlot::Unknown, ZeroEcho));
	TestFalse(TEXT("Repair-all validation should reject negative totals"),
		NetComponent->ValidatePlayerRepairAllEquipmentForTests(NegativeEcho));
	TestTrue(TEXT("Repair-all validation should accept non-negative stale totals"),
		NetComponent->ValidatePlayerRepairAllEquipmentForTests(StalePositiveEcho));

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Equipped source validation
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantEquippedSourceValidationTest,
	"InventoryPlugin.Merchant.Regression.EquippedSourceValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantEquippedSourceValidationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Owner);
	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* HeadItem = NewObject<UInventoryItemEquipable>();
	HeadItem->ItemID = 71001;
	HeadItem->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Head);
	Equipment->EquipItem(HeadItem, EEquipmentSlot::Head);

	TestTrue(TEXT("An exact equipped item with no reservations is removable for sale"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Head, HeadItem->ItemID));
	TestFalse(TEXT("A stale expected item id cannot remove a replacement item"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Head, HeadItem->ItemID + 1));

	UEquipmentComponent* ReservedEquipment = NewObject<UEquipmentComponent>(Owner);
	const FGuid SourceReservation = FGuid::NewGuid();
	TestTrue(TEXT("Test setup reserves the source equipment slot"),
		ReservedEquipment->ReservePendingDelivery(SourceReservation, HeadItem, EEquipmentSlot::Head));
	ReservedEquipment->EquipItem(HeadItem, EEquipmentSlot::Head);
	TestFalse(TEXT("A reserved source slot cannot be sold"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			ReservedEquipment, Inventory, EEquipmentSlot::Head, HeadItem->ItemID));

	UInventoryItemEquipable* TwoHandedItem = NewObject<UInventoryItemEquipable>();
	TwoHandedItem->ItemID = 71002;
	TwoHandedItem->MultiSlotItem = true;
	TwoHandedItem->EquipableSlotBitMask = (1 << static_cast<uint8>(EEquipmentSlot::Primary)) |
		(1 << static_cast<uint8>(EEquipmentSlot::Secondary));
	UInventoryItemEquipable* SecondaryItem = NewObject<UInventoryItemEquipable>();
	SecondaryItem->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Secondary);
	UEquipmentComponent* MultiSlotEquipment = NewObject<UEquipmentComponent>(Owner);
	const FGuid SecondaryReservation = FGuid::NewGuid();
	TestTrue(TEXT("Test setup reserves another slot in the multi-slot footprint"),
		MultiSlotEquipment->ReservePendingDelivery(SecondaryReservation, SecondaryItem,
			EEquipmentSlot::Secondary));
	MultiSlotEquipment->EquipItem(TwoHandedItem, EEquipmentSlot::Primary);
	TestFalse(TEXT("A reservation anywhere in a multi-slot footprint blocks the sale"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			MultiSlotEquipment, Inventory, EEquipmentSlot::Primary, TwoHandedItem->ItemID));

	UInventoryItemBag* EquippedBag = NewObject<UInventoryItemBag>();
	EquippedBag->ItemID = 71003;
	EquippedBag->Bag = true;
	EquippedBag->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::WaistBag1);
	UEquipmentComponent* BagEquipment = NewObject<UEquipmentComponent>(Owner);
	BagEquipment->EquipItem(EquippedBag, EEquipmentSlot::WaistBag1);
	Inventory->BagSet(EBagSlot::WaistBag1, true, 2, 2);
	TestTrue(TEXT("An empty equipped container is removable for sale"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			BagEquipment, Inventory, EEquipmentSlot::WaistBag1, EquippedBag->ItemID));

	UInventoryItemBase* ReservedFootprintItem = NewObject<UInventoryItemBase>();
	const FGuid FootprintReservation = FGuid::NewGuid();
	TestTrue(TEXT("Test setup reserves space in the linked equipped bag"),
		Inventory->ReserveItemFootprint(FootprintReservation, EBagSlot::WaistBag1,
			ReservedFootprintItem, 0));
	TestFalse(TEXT("A linked-bag footprint reservation blocks container sale"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			BagEquipment, Inventory, EEquipmentSlot::WaistBag1, EquippedBag->ItemID));

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Equipment-slot merchant click routing
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantEquipmentSlotClickRoutingTest,
	"InventoryPlugin.Merchant.Regression.EquipmentSlotClickRouting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantEquipmentSlotClickRoutingTest::RunTest(const FString& Parameters)
{
	auto CanRoute = [](bool bLeftClick, bool bTrading, bool bEnabled, bool bLocked,
		bool bHasItem, bool bCanonicalSlot)
	{
		return UEquipmentSlotWidget::CanHandleMerchantSaleClickForTests(bLeftClick, bTrading, bEnabled,
			bLocked, bHasItem, bCanonicalSlot);
	};

	TestTrue(TEXT("A left-click on an active equipped item routes to the merchant"),
		CanRoute(true, true, true, false, true, true));
	TestFalse(TEXT("An empty equipment slot preserves its existing click behavior"),
		CanRoute(true, true, true, false, false, true));
	TestFalse(TEXT("A locked equipment slot is not presented to the merchant"),
		CanRoute(true, true, true, true, true, true));
	TestFalse(TEXT("A click outside merchant trading preserves normal equipment behavior"),
		CanRoute(true, false, true, false, true, true));
	TestFalse(TEXT("A right-click is not treated as a merchant sale click"),
		CanRoute(false, true, true, false, true, true));
	TestFalse(TEXT("A disabled secondary multi-slot visual cannot originate a sale"),
		CanRoute(true, true, false, false, true, true));
	TestFalse(TEXT("An unknown equipment slot cannot originate a sale"),
		CanRoute(true, true, true, false, true, false));

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant widget refresh
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantWidgetRefreshWithoutMerchantTest,
	"InventoryPlugin.Merchant.Regression.RefreshWithoutMerchant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantWidgetRefreshWithoutMerchantTest::RunTest(const FString& Parameters)
{
	UMerchantSellWidget* Widget = NewObject<UMerchantSellWidget>(GetTransientPackage());
	if (!Widget)
	{
		AddError(TEXT("Failed to create merchant sell widget"));
		return false;
	}

	TestFalse(TEXT("No merchant bound means no item can be sold by merchant"),
		Widget->MerchantCanSellForTests(100));

	Widget->Refresh();
	TestTrue(TEXT("Refreshing without a merchant should be safe for delayed transaction refreshes"), true);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant close cleanup
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantWidgetCloseCleanupTest,
	"InventoryPlugin.Merchant.Regression.CloseClearsTransactionState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantWidgetCloseCleanupTest::RunTest(const FString& Parameters)
{
	UMerchantSellWidget* Widget = NewObject<UMerchantSellWidget>(GetTransientPackage());
	if (!Widget)
	{
		AddError(TEXT("Failed to create merchant sell widget"));
		return false;
	}

	Widget->SetMerchantSessionStateForTests(100, 7, EBagSlot::Pocket1, EMerchantWindowMode::Buy);
	TestTrue(TEXT("Test setup should contain active merchant transaction state"),
		Widget->HasMerchantSessionStateForTests());

	Widget->DeInitMerchantData();
	TestFalse(TEXT("Closing a merchant must clear selected item and sell-origin state"),
		Widget->HasMerchantSessionStateForTests());

	Widget->AssignEquippedSellData(200, EEquipmentSlot::Head);
	TestEqual(TEXT("Equipment selection records its equipment origin"),
		Widget->GetMerchantBuyOriginEquipmentSlotForTests(), EEquipmentSlot::Head);
	TestEqual(TEXT("Equipment selection clears the bag origin"),
		Widget->GetMerchantBuyOriginSlotForTests(), EBagSlot::Unknown);
	TestEqual(TEXT("Equipment selection clears bag coordinates"),
		Widget->GetMerchantBuyOriginTopLeftForTests(), -1);

	Widget->AssignSellData(300, 4, EBagSlot::Pocket2);
	TestEqual(TEXT("Bag selection clears the equipment origin"),
		Widget->GetMerchantBuyOriginEquipmentSlotForTests(), EEquipmentSlot::Unknown);
	Widget->ResetSellData();
	TestFalse(TEXT("Resetting a selection clears both origin forms"),
		Widget->HasMerchantSessionStateForTests());

	Widget->DeInitMerchantData();
	TestFalse(TEXT("Closing an equipped selection clears both origin forms"),
		Widget->HasMerchantSessionStateForTests());

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant static stock replication
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantStaticPoolRepNotifyTest,
	"InventoryPlugin.Merchant.Regression.StaticPoolRepNotify",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantStaticPoolRepNotifyTest::RunTest(const FString& Parameters)
{
	UMerchantComponent* MerchantComponent = NewObject<UMerchantComponent>(GetTransientPackage());
	if (!MerchantComponent)
	{
		AddError(TEXT("Failed to create merchant component"));
		return false;
	}

	UMerchantSellWidget* Widget = NewObject<UMerchantSellWidget>(GetTransientPackage());
	if (!Widget)
	{
		AddError(TEXT("Failed to create merchant sell widget"));
		return false;
	}

	MerchantComponent->MerchantPoolDispatcher.AddDynamic(Widget, &UMerchantSellWidget::Refresh);
	MerchantComponent->SetStaticMerchantPoolForTests({100, 200});
	MerchantComponent->OnRep_StaticPool();

	TestEqual(TEXT("Static pool replication must notify listeners so an already-open merchant UI refreshes"),
		MerchantComponent->GetStaticPoolRepNotifyCountForTests(), 1);
	TestEqual(TEXT("A bound merchant sell widget must receive the static-pool refresh notification"),
		Widget->GetRefreshCountForTests(), 1);
	TestEqual(TEXT("Static pool contents remain available after the replication notification"),
		MerchantComponent->GetStaticItemsConst().Num(), 2);
	TestEqual(TEXT("Static pool preserves the replicated item ids"),
		MerchantComponent->GetStaticItemsConst()[1], 200);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant item list selection
//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMerchantItemListSelectionBroadcastsTest,
	"InventoryPlugin.Merchant.Regression.ItemListSelectionBroadcasts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMerchantItemListSelectionBroadcastsTest::RunTest(const FString& Parameters)
{
	UMerchantItemListWidget* ListWidget = NewObject<UMerchantItemListWidget>(GetTransientPackage());
	UListView* ListView = NewObject<UListView>(ListWidget);
	if (!ListWidget || !ListView)
	{
		AddError(TEXT("Failed to create merchant item list test widgets"));
		return false;
	}

	ListWidget->SetItemListViewForTests(ListView);

	FMerchantItemDataStruct Row;
	Row.Id = 7000;
	Row.Name = TEXT("Water Flask");
	Row.Quantity = -1;
	ListWidget->AddDataToList(Row);

	TestEqual(TEXT("Native item list should add one row to the bound ListView"),
		ListWidget->GetListItemCountForTests(), 1);

	const TArray<UObject*> Items = ListView->GetListItems();
	TestEqual(TEXT("ListView should expose the generated row object"),
		Items.Num(), 1);
	if (Items.Num() != 1)
	{
		return false;
	}

	ListWidget->HandleListItemSelectionChangedForTests(Items[0]);

	TestEqual(TEXT("Forwarding a selected merchant list row should emit the merchant item ID"),
		ListWidget->GetLastSelectionItemIDForTests(), 7000);
	TestTrue(TEXT("A rebuilt merchant row should be found by item ID for selection restoration"),
		ListWidget->SelectItemByID(7000));
	TestFalse(TEXT("Selection restoration should fail when the item is no longer in the list"),
		ListWidget->SelectItemByID(7001));

	return true;
}

#endif // WITH_AUTOMATION_WORKER
