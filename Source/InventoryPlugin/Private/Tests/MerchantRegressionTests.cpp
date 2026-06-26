#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/CoinComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/MerchantComponent.h"
#include "UI/Merchant/MerchantSellWidget.h"

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

	MerchantComponent->SetStaticMerchantPoolForTests({100, 200});
	MerchantComponent->OnRep_StaticPool();

	TestEqual(TEXT("Static pool replication must notify listeners so an already-open merchant UI refreshes"),
		MerchantComponent->GetStaticPoolRepNotifyCountForTests(), 1);
	TestEqual(TEXT("Static pool contents remain available after the replication notification"),
		MerchantComponent->GetStaticItemsConst().Num(), 2);
	TestEqual(TEXT("Static pool preserves the replicated item ids"),
		MerchantComponent->GetStaticItemsConst()[1], 200);

	return true;
}

#endif // WITH_AUTOMATION_WORKER
