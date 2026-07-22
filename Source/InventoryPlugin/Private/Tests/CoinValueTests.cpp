// Copyright 2025 Maximilien (Synock) Guislain

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "CoinValue.h"

// ─────────────────────────────────────────────────────────────────────────────
// FCoinValue - Float Constructor
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueFloatConstructorBasicTest,
	"InventoryPlugin.CoinValue.FloatConstructor.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueFloatConstructorBasicTest::RunTest(const FString& Parameters)
{
	// 1234 → PP=1, GP=2, SP=3, CP=4
	const FCoinValue V(1234.f);
	TestEqual(TEXT("CopperPieces"), V.CopperPieces, 4);
	TestEqual(TEXT("SilverPieces"), V.SilverPieces, 3);
	TestEqual(TEXT("GoldPieces"),   V.GoldPieces,   2);
	TestEqual(TEXT("PlatinumPieces"), V.PlatinumPieces, 1);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueFloatConstructorZeroTest,
	"InventoryPlugin.CoinValue.FloatConstructor.Zero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueFloatConstructorZeroTest::RunTest(const FString& Parameters)
{
	const FCoinValue V(0.f);
	TestEqual(TEXT("CP"), V.CopperPieces, 0);
	TestEqual(TEXT("SP"), V.SilverPieces, 0);
	TestEqual(TEXT("GP"), V.GoldPieces,   0);
	TestEqual(TEXT("PP"), V.PlatinumPieces, 0);
	TestTrue(TEXT("IsEmpty after zero construction"), V.IsEmpty());
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueFloatConstructorNegativeTest,
	"InventoryPlugin.CoinValue.FloatConstructor.Negative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueFloatConstructorNegativeTest::RunTest(const FString& Parameters)
{
	// Negative cast truncates to 0 via static_cast<int32>; result is all zeros
	const FCoinValue V(-50.f);
	// static_cast<int32>(-50) = -50, so CopperPieces = -50 % 10 = 0 (C++ truncation),
	// SilverPieces = (-50/10) % 10 = -5 — document this as a known edge-case.
	// The constructor does not guard against negative input; callers must ensure positive values.
	// We just verify it doesn't crash and ToFloat reflects reality.
	const float F = V.ToFloat();
	TestTrue(TEXT("Negative input yields non-positive ToFloat"), F <= 0.f);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ToFloat / round-trip
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueToFloatTest,
	"InventoryPlugin.CoinValue.ToFloat.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueToFloatTest::RunTest(const FString& Parameters)
{
	// ToFloat = CP + SP*10 + GP*100 + PP*1000
	TestEqual(TEXT("1 copper  = 1"),    FCoinValue(1,0,0,0).ToFloat(),    1.f);
	TestEqual(TEXT("1 silver  = 10"),   FCoinValue(0,1,0,0).ToFloat(),   10.f);
	TestEqual(TEXT("1 gold    = 100"),  FCoinValue(0,0,1,0).ToFloat(),  100.f);
	TestEqual(TEXT("1 plat    = 1000"), FCoinValue(0,0,0,1).ToFloat(), 1000.f);
	TestEqual(TEXT("mixed"),            FCoinValue(4,3,2,1).ToFloat(), 1234.f);

	// round-trip: float → FCoinValue → ToFloat
	const float Original = 9999.f;
	const FCoinValue V(Original);
	TestEqual(TEXT("Round-trip 9999"), V.ToFloat(), Original);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsEmpty
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueIsEmptyTest,
	"InventoryPlugin.CoinValue.IsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueIsEmptyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Default-constructed is empty"), FCoinValue().IsEmpty());
	TestFalse(TEXT("1 copper is not empty"),  FCoinValue(1,0,0,0).IsEmpty());
	TestFalse(TEXT("1 silver is not empty"),  FCoinValue(0,1,0,0).IsEmpty());
	TestFalse(TEXT("1 plat is not empty"),    FCoinValue(0,0,0,1).IsEmpty());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// operator+=
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueAdditionTest,
	"InventoryPlugin.CoinValue.OperatorPlus.Normal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueAdditionTest::RunTest(const FString& Parameters)
{
	FCoinValue A(3, 2, 1, 0);
	A += FCoinValue(7, 8, 9, 1);
	TestEqual(TEXT("CP"), A.CopperPieces, 10);
	TestEqual(TEXT("SP"), A.SilverPieces, 10);
	TestEqual(TEXT("GP"), A.GoldPieces,   10);
	TestEqual(TEXT("PP"), A.PlatinumPieces, 1);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueAdditionOverflowTest,
	"InventoryPlugin.CoinValue.OperatorPlus.OverflowClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueAdditionOverflowTest::RunTest(const FString& Parameters)
{
	// Adding INT32_MAX to INT32_MAX should clamp to INT32_MAX, not wrap or crash
	FCoinValue A(INT32_MAX, 0, 0, 0);
	A += FCoinValue(INT32_MAX, 0, 0, 0);
	TestEqual(TEXT("Overflow clamped to INT32_MAX"), A.CopperPieces, INT32_MAX);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// operator*=
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueMulRatioAboveOneTest,
	"InventoryPlugin.CoinValue.OperatorMul.RatioAboveOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueMulRatioAboveOneTest::RunTest(const FString& Parameters)
{
	// Doubling 100 copper should give exactly 200 copper.
	FCoinValue V(100, 0, 0, 0);
	V *= 2.0f;
	TestEqual(TEXT("ToFloat after *2"), V.ToFloat(), 200.f);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueMulRatioBelowOneTest,
	"InventoryPlugin.CoinValue.OperatorMul.RatioBelowOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueMulRatioBelowOneTest::RunTest(const FString& Parameters)
{
	// Halving 100 copper with merchant bias (round down): FVal=50 - 0.5 = 49.5 → ceil = 50
	FCoinValue V(100, 0, 0, 0);
	V *= 0.5f;
	TestEqual(TEXT("ToFloat after *0.5 (sell bias)"), V.ToFloat(), 50.f);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueMulZeroTest,
	"InventoryPlugin.CoinValue.OperatorMul.ZeroRatio",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueMulZeroTest::RunTest(const FString& Parameters)
{
	FCoinValue V(500, 10, 5, 2);
	V *= 0.0f;
	TestTrue(TEXT("After *0 should be empty"), V.IsEmpty());
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueMulEmptyValueStaysEmptyTest,
	"InventoryPlugin.CoinValue.OperatorMul.EmptyValueStaysEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueMulEmptyValueStaysEmptyTest::RunTest(const FString& Parameters)
{
	FCoinValue V(0, 0, 0, 0);
	V *= 1.1f;
	TestTrue(TEXT("Markup on an empty value should stay empty"), V.IsEmpty());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ReduceCoinAmount
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueReduceBasicTest,
	"InventoryPlugin.CoinValue.ReduceCoinAmount.BasicCarry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueReduceBasicTest::RunTest(const FString& Parameters)
{
	// 25 copper → 5 copper + 2 silver
	FCoinValue V(25, 0, 0, 0);
	V.ReduceCoinAmount();
	TestEqual(TEXT("CP after reduce"), V.CopperPieces, 5);
	TestEqual(TEXT("SP after reduce"), V.SilverPieces, 2);
	TestEqual(TEXT("GP after reduce"), V.GoldPieces, 0);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueReduceCascadeTest,
	"InventoryPlugin.CoinValue.ReduceCoinAmount.CascadeCarry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueReduceCascadeTest::RunTest(const FString& Parameters)
{
	// 1000 copper → fully reduced to 1 platinum
	FCoinValue V(1000, 0, 0, 0);
	V.ReduceCoinAmount();
	TestEqual(TEXT("CP"), V.CopperPieces, 0);
	TestEqual(TEXT("SP"), V.SilverPieces, 0);
	TestEqual(TEXT("GP"), V.GoldPieces,   0);
	TestEqual(TEXT("PP"), V.PlatinumPieces, 1);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueReduceValuePreservedTest,
	"InventoryPlugin.CoinValue.ReduceCoinAmount.PreservesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueReduceValuePreservedTest::RunTest(const FString& Parameters)
{
	FCoinValue V(123, 45, 67, 8);
	const float Before = V.ToFloat();
	V.ReduceCoinAmount();
	TestEqual(TEXT("ToFloat unchanged after reduce"), V.ToFloat(), Before);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// CanPay
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueCanPayExactTest,
	"InventoryPlugin.CoinValue.CanPay.ExactAmount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueCanPayExactTest::RunTest(const FString& Parameters)
{
	const FCoinValue Have(5, 3, 1, 0);
	const FCoinValue Need(5, 3, 1, 0);
	TestTrue(TEXT("Exact match can pay"), FCoinValue::CanPay(Have, Need));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueCanPayInsufficientTest,
	"InventoryPlugin.CoinValue.CanPay.Insufficient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueCanPayInsufficientTest::RunTest(const FString& Parameters)
{
	// Cannot pay 1 silver with only copper even if total value is sufficient—CanPay is denomination-exact
	TestFalse(TEXT("5cp cannot pay 1sp via CanPay"),
		FCoinValue::CanPay(FCoinValue(5,0,0,0), FCoinValue(0,1,0,0)));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueCanPayWithChangeTest,
	"InventoryPlugin.CoinValue.CanPayWithChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueCanPayWithChangeTest::RunTest(const FString& Parameters)
{
	// 10 copper CAN cover a 1-silver cost via change
	TestTrue(TEXT("10cp covers 1sp with change"),
		FCoinValue::CanPayWithChange(FCoinValue(10,0,0,0), FCoinValue(0,1,0,0)));

	// 9 copper cannot cover 1 silver (9 < 10)
	TestFalse(TEXT("9cp cannot cover 1sp"),
		FCoinValue::CanPayWithChange(FCoinValue(9,0,0,0), FCoinValue(0,1,0,0)));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueToCopperValueTest,
	"InventoryPlugin.CoinValue.ToCopperValue.MixedDenominations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueToCopperValueTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Mixed denominations convert to copper"), FCoinValue(4, 3, 2, 1).ToCopperValue(), 1234LL);
	TestEqual(TEXT("Uses int64 for large values"),
		FCoinValue(0, 0, 0, INT32_MAX).ToCopperValue(), static_cast<int64>(INT32_MAX) * 1000LL);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueIsNonNegativeTest,
	"InventoryPlugin.CoinValue.IsNonNegative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueIsNonNegativeTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Zero is non-negative"), FCoinValue(0, 0, 0, 0).IsNonNegative());
	TestTrue(TEXT("Positive mixed value is non-negative"), FCoinValue(1, 2, 3, 4).IsNonNegative());
	TestFalse(TEXT("Negative copper is rejected"), FCoinValue(-1, 0, 0, 0).IsNonNegative());
	TestFalse(TEXT("Negative platinum is rejected"), FCoinValue(0, 0, 0, -1).IsNonNegative());
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueHasSameValueTest,
	"InventoryPlugin.CoinValue.HasSameValue.DenominationChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueHasSameValueTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("10 copper equals 1 silver"), FCoinValue(10, 0, 0, 0).HasSameValue(FCoinValue(0, 1, 0, 0)));
	TestTrue(TEXT("100 copper equals 1 gold"), FCoinValue(100, 0, 0, 0).HasSameValue(FCoinValue(0, 0, 1, 0)));
	TestFalse(TEXT("Different values are not equal"), FCoinValue(9, 0, 0, 0).HasSameValue(FCoinValue(0, 1, 0, 0)));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueCanPayWithChangeRejectsNegativeTest,
	"InventoryPlugin.CoinValue.CanPayWithChange.RejectsNegative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueCanPayWithChangeRejectsNegativeTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Negative needed value is rejected"),
		FCoinValue::CanPayWithChange(FCoinValue(10, 0, 0, 0), FCoinValue(-1, 0, 0, 0)));
	TestFalse(TEXT("Negative available value is rejected"),
		FCoinValue::CanPayWithChange(FCoinValue(-10, 0, 0, 0), FCoinValue(1, 0, 0, 0)));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// RetrieveValue (make-change algorithm)
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueRetrieveValueExactTest,
	"InventoryPlugin.CoinValue.RetrieveValue.ExactPayment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueRetrieveValueExactTest::RunTest(const FString& Parameters)
{
	FCoinValue Available(5, 0, 0, 0);
	FCoinValue Needed(5, 0, 0, 0);
	const bool bResult = FCoinValue::RetrieveValue(Available, Needed);
	TestTrue(TEXT("Exact payment succeeds"), bResult);
	// Remainder: Available - Needed = 0
	TestEqual(TEXT("Remaining CP"), Available.CopperPieces - Needed.CopperPieces, 0);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueRetrieveValueBreakSilverTest,
	"InventoryPlugin.CoinValue.RetrieveValue.BreakSilver",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueRetrieveValueBreakSilverTest::RunTest(const FString& Parameters)
{
	// Have 1 silver (=10cp), need 5cp → should break the silver
	FCoinValue Available(0, 1, 0, 0);
	FCoinValue Needed(5, 0, 0, 0);
	const bool bResult = FCoinValue::RetrieveValue(Available, Needed);
	TestTrue(TEXT("Breaking silver to pay copper succeeds"), bResult);
	// After adjustment: Available.CP should be >= Needed.CP
	TestTrue(TEXT("Available CP >= Needed CP after adjustment"),
		Available.CopperPieces >= Needed.CopperPieces);
	// Net change: (Available - Needed) ToFloat = 1 silver - 5 copper = 5 copper
	const float NetValue = FCoinValue(
		Available.CopperPieces  - Needed.CopperPieces,
		Available.SilverPieces  - Needed.SilverPieces,
		Available.GoldPieces    - Needed.GoldPieces,
		Available.PlatinumPieces - Needed.PlatinumPieces).ToFloat();
	TestTrue(TEXT("Net change is 5 copper"), FMath::IsNearlyEqual(NetValue, 5.f, 0.01f));
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueRetrieveValueInsufficientTest,
	"InventoryPlugin.CoinValue.RetrieveValue.Insufficient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueRetrieveValueInsufficientTest::RunTest(const FString& Parameters)
{
	FCoinValue Available(3, 0, 0, 0);
	FCoinValue Needed(5, 0, 0, 0);
	const bool bResult = FCoinValue::RetrieveValue(Available, Needed);
	TestFalse(TEXT("Cannot pay more than available"), bResult);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoinValueRetrieveValueBreakGoldTest,
	"InventoryPlugin.CoinValue.RetrieveValue.BreakGold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCoinValueRetrieveValueBreakGoldTest::RunTest(const FString& Parameters)
{
	// Have 1 gold (=100cp), need 15 silver (=150cp)... wait that's more
	// Have 2 gold (=200cp), need 15 silver (=150cp). True total: 200 >= 150 → can pay
	FCoinValue Available(0, 0, 2, 0);
	FCoinValue Needed(0, 15, 0, 0);
	const bool bResult = FCoinValue::RetrieveValue(Available, Needed);
	TestTrue(TEXT("Breaking gold to pay silver succeeds"), bResult);
	const float NetValue = FCoinValue(
		Available.CopperPieces  - Needed.CopperPieces,
		Available.SilverPieces  - Needed.SilverPieces,
		Available.GoldPieces    - Needed.GoldPieces,
		Available.PlatinumPieces - Needed.PlatinumPieces).ToFloat();
	TestTrue(TEXT("Net change is 50cp (2gp - 15sp)"), FMath::IsNearlyEqual(NetValue, 50.f, 0.01f));
	return true;
}

#endif // WITH_AUTOMATION_WORKER
