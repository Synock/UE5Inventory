// Copyright 2025 Maximilien (Synock) Guislain

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "InventoryUtilities.h"
#include "Definitions.h"
#include "Components/InventoryComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// GetItemFootPrint
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesFootPrintTest,
	"InventoryPlugin.Utilities.GetItemFootPrint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesFootPrintTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Tiny  = 1x1"), UInventoryUtilities::GetItemFootPrint(EItemSize::Tiny),  FVector2D(1,1));
	TestEqual(TEXT("Small = 2x2"), UInventoryUtilities::GetItemFootPrint(EItemSize::Small), FVector2D(2,2));
	TestEqual(TEXT("Medium= 3x3"), UInventoryUtilities::GetItemFootPrint(EItemSize::Medium),FVector2D(3,3));
	TestEqual(TEXT("Large = 4x4"), UInventoryUtilities::GetItemFootPrint(EItemSize::Large), FVector2D(4,4));
	TestEqual(TEXT("Giant = 5x5"), UInventoryUtilities::GetItemFootPrint(EItemSize::Giant), FVector2D(5,5));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetItemSizeString
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesSizeStringTest,
	"InventoryPlugin.Utilities.GetItemSizeString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesSizeStringTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Tiny"),   UInventoryUtilities::GetItemSizeString(EItemSize::Tiny),   FString("Tiny"));
	TestEqual(TEXT("Small"),  UInventoryUtilities::GetItemSizeString(EItemSize::Small),  FString("Small"));
	TestEqual(TEXT("Medium"), UInventoryUtilities::GetItemSizeString(EItemSize::Medium), FString("Medium"));
	TestEqual(TEXT("Large"),  UInventoryUtilities::GetItemSizeString(EItemSize::Large),  FString("Large"));
	// NOTE: EItemSize::Giant maps to "Gigantic" in the implementation (not "Giant").
	TestEqual(TEXT("Giant → Gigantic"), UInventoryUtilities::GetItemSizeString(EItemSize::Giant), FString("Gigantic"));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetSlotName — spot-check a representative subset
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesSlotNameTest,
	"InventoryPlugin.Utilities.GetSlotName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesSlotNameTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Primary"),  UInventoryUtilities::GetSlotName(EEquipmentSlot::Primary),  FString("Primary"));
	TestEqual(TEXT("Head"),     UInventoryUtilities::GetSlotName(EEquipmentSlot::Head),     FString("Head"));
	TestEqual(TEXT("Feet"),     UInventoryUtilities::GetSlotName(EEquipmentSlot::Feet),     FString("Feet"));
	TestEqual(TEXT("WristL"),   UInventoryUtilities::GetSlotName(EEquipmentSlot::WristL),   FString("Wrist (L)"));
	TestEqual(TEXT("WristR"),   UInventoryUtilities::GetSlotName(EEquipmentSlot::WristR),   FString("Wrist (R)"));
	// Unknown slot → empty string
	TestTrue(TEXT("Unknown slot returns empty"),
		UInventoryUtilities::GetSlotName(EEquipmentSlot::Unknown).IsEmpty());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// CoinValueFromFloat / FloatFromCoinValue round-trip
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesCoinRoundTripTest,
	"InventoryPlugin.Utilities.CoinFloatRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesCoinRoundTripTest::RunTest(const FString& Parameters)
{
	const float Input = 2345.f;
	const FCoinValue Coins = UInventoryUtilities::CoinValueFromFloat(Input);
	const float Output = UInventoryUtilities::FloatFromCoinValue(Coins);
	TestTrue(TEXT("Round-trip 2345"), FMath::IsNearlyEqual(Output, Input, 0.01f));

	// Zero
	TestTrue(TEXT("Round-trip 0"), FMath::IsNearlyEqual(
		UInventoryUtilities::FloatFromCoinValue(UInventoryUtilities::CoinValueFromFloat(0.f)), 0.f, 0.01f));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ReduceCoinAmount (utility wrapper)
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesReduceTest,
	"InventoryPlugin.Utilities.ReduceCoinAmount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesReduceTest::RunTest(const FString& Parameters)
{
	const FCoinValue Input(100, 0, 0, 0); // 100 copper = 1 gold
	const FCoinValue Reduced = UInventoryUtilities::ReduceCoinAmount(Input);
	TestEqual(TEXT("100 CP reduces to 1 GP"), Reduced.GoldPieces, 1);
	TestEqual(TEXT("CP remainder 0"),         Reduced.CopperPieces, 0);
	TestEqual(TEXT("SP remainder 0"),         Reduced.SilverPieces, 0);
	TestEqual(TEXT("Value preserved"),        Reduced.ToFloat(), Input.ToFloat());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetCoinValueAsString
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesCoinStringTest,
	"InventoryPlugin.Utilities.GetCoinValueAsString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesCoinStringTest::RunTest(const FString& Parameters)
{
	// Empty purse → empty string
	TestTrue(TEXT("Empty returns empty string"),
		UInventoryUtilities::GetCoinValueAsString(FCoinValue(0,0,0,0)).IsEmpty());

	// Single denomination
	const FString CopperOnly = UInventoryUtilities::GetCoinValueAsString(FCoinValue(3,0,0,0));
	TestTrue(TEXT("Contains 'Copper'"), CopperOnly.Contains(TEXT("Copper")));
	TestFalse(TEXT("No Silver"),        CopperOnly.Contains(TEXT("Silver")));

	// All denominations
	const FString All = UInventoryUtilities::GetCoinValueAsString(FCoinValue(1,2,3,4));
	TestTrue(TEXT("Contains Platinum"), All.Contains(TEXT("Platinum")));
	TestTrue(TEXT("Contains Gold"),     All.Contains(TEXT("Gold")));
	TestTrue(TEXT("Contains Silver"),   All.Contains(TEXT("Silver")));
	TestTrue(TEXT("Contains Copper"),   All.Contains(TEXT("Copper")));
	TestTrue(TEXT("Ends with pieces"),  All.EndsWith(TEXT("pieces")));

	// Only platinum
	const FString PlatOnly = UInventoryUtilities::GetCoinValueAsString(FCoinValue(0,0,0,5));
	TestFalse(TEXT("No Silver in plat-only"), PlatOnly.Contains(TEXT("Silver")));
	TestTrue(TEXT("Platinum in plat-only"),   PlatOnly.Contains(TEXT("Platinum")));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// CanPay / CanPayWithChange wrappers
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesCanPayTest,
	"InventoryPlugin.Utilities.CanPay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesCanPayTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Exact can pay"),
		UInventoryUtilities::CanPay(FCoinValue(5,3,2,1), FCoinValue(5,3,2,1)));
	TestTrue(TEXT("Surplus can pay"),
		UInventoryUtilities::CanPay(FCoinValue(9,9,9,9), FCoinValue(1,1,1,1)));
	TestFalse(TEXT("Insufficient CP cannot pay"),
		UInventoryUtilities::CanPay(FCoinValue(4,3,2,1), FCoinValue(5,3,2,1)));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesCanPayWithChangeTest,
	"InventoryPlugin.Utilities.CanPayWithChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesCanPayWithChangeTest::RunTest(const FString& Parameters)
{
	// 1 plat (1000cp) > cost of 500cp → can pay with change
	TestTrue(TEXT("1 plat covers 500cp cost"),
		UInventoryUtilities::CanPayWithChange(FCoinValue(0,0,0,1), FCoinValue(0,5,0,0)));
	TestFalse(TEXT("Empty purse cannot pay anything"),
		UInventoryUtilities::CanPayWithChange(FCoinValue(0,0,0,0), FCoinValue(1,0,0,0)));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeChange wrapper
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesComputeChangeTest,
	"InventoryPlugin.Utilities.ComputeChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesComputeChangeTest::RunTest(const FString& Parameters)
{
	FCoinValue Available(0, 1, 0, 0); // 10cp worth
	FCoinValue Needed(7, 0, 0, 0);   // 7cp needed
	const bool bOK = UInventoryUtilities::ComputeChange(Available, Needed);
	TestTrue(TEXT("ComputeChange succeeds"), bOK);
	// Net remainder = 3cp
	const float Net = FCoinValue(
		Available.CopperPieces - Needed.CopperPieces,
		Available.SilverPieces - Needed.SilverPieces,
		Available.GoldPieces   - Needed.GoldPieces,
		Available.PlatinumPieces - Needed.PlatinumPieces).ToFloat();
	TestTrue(TEXT("Remainder is 3cp"), FMath::IsNearlyEqual(Net, 3.f, 0.01f));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BagSlot <-> EquipmentSlot round-trip conversions
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUtilitiesSlotConversionTest,
	"InventoryPlugin.Utilities.SlotConversion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUtilitiesSlotConversionTest::RunTest(const FString& Parameters)
{
	// BagSlot → EquipmentSlot
	TestEqual(TEXT("WaistBag1 → EqWaistBag1"),
		UInventoryComponent::GetInventorySlotFromBagSlot(EBagSlot::WaistBag1), EEquipmentSlot::WaistBag1);
	TestEqual(TEXT("Quiver → EqAmmo"),
		UInventoryComponent::GetInventorySlotFromBagSlot(EBagSlot::Quiver), EEquipmentSlot::Ammo);
	TestEqual(TEXT("Pocket1 → Unknown (no equipment slot)"),
		UInventoryComponent::GetInventorySlotFromBagSlot(EBagSlot::Pocket1), EEquipmentSlot::Unknown);

	// EquipmentSlot → BagSlot
	TestEqual(TEXT("EqBackPack1 → BackPack1"),
		UInventoryComponent::GetBagSlotFromInventory(EEquipmentSlot::BackPack1), EBagSlot::BackPack1);
	TestEqual(TEXT("EqAmmo → Quiver"),
		UInventoryComponent::GetBagSlotFromInventory(EEquipmentSlot::Ammo), EBagSlot::Quiver);
	TestEqual(TEXT("EqHead → Unknown"),
		UInventoryComponent::GetBagSlotFromInventory(EEquipmentSlot::Head), EBagSlot::Unknown);
	return true;
}

#endif // WITH_AUTOMATION_WORKER




