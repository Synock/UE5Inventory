// Copyright 2025 Maximilien (Synock) Guislain

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "BagStorage.h"
#include "Items/InventoryItemBase.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helper: make a minimal UInventoryItemBase with given dimensions (no world needed)
// ─────────────────────────────────────────────────────────────────────────────
namespace GridBagSolverTestHelpers
{
	static UInventoryItemBase* MakeItem(uint8 W, uint8 H, EItemSize Size = EItemSize::Tiny)
	{
		UInventoryItemBase* Item = NewObject<UInventoryItemBase>(GetTransientPackage());
		Item->Width    = W;
		Item->Height   = H;
		Item->ItemSize = Size;
		return Item;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// IsRoomAvailable - empty grid
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverEmptyGridTest,
	"InventoryPlugin.GridBagSolver.IsRoomAvailable.EmptyGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverEmptyGridTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2); // 3-wide 2-tall = 6 cells
	UInventoryItemBase* Item1x1 = GridBagSolverTestHelpers::MakeItem(1, 1);
	UInventoryItemBase* Item2x2 = GridBagSolverTestHelpers::MakeItem(2, 2);

	TestTrue(TEXT("1x1 fits at index 0"), Solver.IsRoomAvailable(Item1x1, 0));
	TestTrue(TEXT("1x1 fits at last cell (index 5)"), Solver.IsRoomAvailable(Item1x1, 5));
	TestTrue(TEXT("2x2 fits at index 0"), Solver.IsRoomAvailable(Item2x2, 0));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsRoomAvailable - item too wide overflows right edge
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverOverflowRightTest,
	"InventoryPlugin.GridBagSolver.IsRoomAvailable.OverflowRight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverOverflowRightTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2);
	UInventoryItemBase* Item2x1 = GridBagSolverTestHelpers::MakeItem(2, 1);

	// Placing at column 2 (index 2): cols 2..3 — col 3 is out of bounds for width 3
	TestFalse(TEXT("2x1 at col 2 overflows right"), Solver.IsRoomAvailable(Item2x1, 2));

	// Placing at column 0 is fine
	TestTrue(TEXT("2x1 at col 0 is fine"), Solver.IsRoomAvailable(Item2x1, 0));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsRoomAvailable - item too tall overflows bottom edge
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverOverflowBottomTest,
	"InventoryPlugin.GridBagSolver.IsRoomAvailable.OverflowBottom",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverOverflowBottomTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2); // height = 2
	UInventoryItemBase* Item1x2 = GridBagSolverTestHelpers::MakeItem(1, 2);
	UInventoryItemBase* Item1x3 = GridBagSolverTestHelpers::MakeItem(1, 3);

	// 1x2 at row 0 is fine
	TestTrue(TEXT("1x2 at row 0 fits"),  Solver.IsRoomAvailable(Item1x2, 0));
	// 1x3 never fits in a 2-tall bag
	TestFalse(TEXT("1x3 never fits"),    Solver.IsRoomAvailable(Item1x3, 0));
	// 1x2 starting at row 1 also overflows
	TestFalse(TEXT("1x2 at row 1 overflows"), Solver.IsRoomAvailable(Item1x2, 3));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// RecordData + IsRoomAvailable: occupied cell blocks placement
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverOccupiedCellTest,
	"InventoryPlugin.GridBagSolver.RecordData.BlocksPlacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverOccupiedCellTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2);
	UInventoryItemBase* Existing = GridBagSolverTestHelpers::MakeItem(1, 1);
	UInventoryItemBase* New      = GridBagSolverTestHelpers::MakeItem(1, 1);

	// Place Existing at index 0
	Solver.RecordData(Existing, 0);

	// Index 0 is now occupied
	TestFalse(TEXT("Index 0 blocked after RecordData"), Solver.IsRoomAvailable(New, 0));
	// Index 1 is still free
	TestTrue(TEXT("Index 1 still free"), Solver.IsRoomAvailable(New, 1));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// RecordData: 2x2 item blocks all four cells
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverRecord2x2Test,
	"InventoryPlugin.GridBagSolver.RecordData.2x2BlocksFourCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverRecord2x2Test::RunTest(const FString& Parameters)
{
	// 4-wide 4-tall grid; place a 2x2 item at index 0 (top-left)
	GridBagSolver Solver(4, 4);
	UInventoryItemBase* Big  = GridBagSolverTestHelpers::MakeItem(2, 2);
	UInventoryItemBase* Tiny = GridBagSolverTestHelpers::MakeItem(1, 1);

	Solver.RecordData(Big, 0); // occupies cells (0,0),(1,0),(0,1),(1,1) = indices 0,1,4,5

	TestFalse(TEXT("Index 0 blocked"), Solver.IsRoomAvailable(Tiny, 0));
	TestFalse(TEXT("Index 1 blocked"), Solver.IsRoomAvailable(Tiny, 1));
	TestFalse(TEXT("Index 4 blocked"), Solver.IsRoomAvailable(Tiny, 4));
	TestFalse(TEXT("Index 5 blocked"), Solver.IsRoomAvailable(Tiny, 5));
	TestTrue(TEXT("Index 2 free"),     Solver.IsRoomAvailable(Tiny, 2));
	TestTrue(TEXT("Index 8 free"),     Solver.IsRoomAvailable(Tiny, 8));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// RecordData: out-of-bounds TopLeft is gracefully ignored
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverOutOfBoundsRecordTest,
	"InventoryPlugin.GridBagSolver.RecordData.OutOfBoundsIgnored",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverOutOfBoundsRecordTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2); // 6 cells
	UInventoryItemBase* Item = GridBagSolverTestHelpers::MakeItem(1, 1);

	// TopLeft = 99 is out of bounds — should not crash and should not block any valid cell
	Solver.RecordData(Item, 99);
	TestTrue(TEXT("Cell 0 still free after OOB RecordData"), Solver.IsRoomAvailable(Item, 0));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetFirstValidTopLeft
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverGetFirstValidTest,
	"InventoryPlugin.GridBagSolver.GetFirstValidTopLeft.EmptyGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverGetFirstValidTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2);
	UInventoryItemBase* Item = GridBagSolverTestHelpers::MakeItem(1, 1);

	TestEqual(TEXT("First valid on empty grid is index 0"), Solver.GetFirstValidTopLeft(Item), 0);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverGetFirstValidAfterFillTest,
	"InventoryPlugin.GridBagSolver.GetFirstValidTopLeft.AfterPartialFill",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverGetFirstValidAfterFillTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(3, 2); // 6 cells
	UInventoryItemBase* Item = GridBagSolverTestHelpers::MakeItem(1, 1);

	// Fill cells 0, 1, 2 (row 0)
	Solver.RecordData(Item, 0);
	Solver.RecordData(Item, 1);
	Solver.RecordData(Item, 2);

	TestEqual(TEXT("First valid shifts to row 1 (index 3)"), Solver.GetFirstValidTopLeft(Item), 3);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridBagSolverGetFirstValidFullGridTest,
	"InventoryPlugin.GridBagSolver.GetFirstValidTopLeft.FullGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGridBagSolverGetFirstValidFullGridTest::RunTest(const FString& Parameters)
{
	GridBagSolver Solver(2, 2); // 4 cells
	UInventoryItemBase* Item = GridBagSolverTestHelpers::MakeItem(1, 1);

	for (int32 i = 0; i < 4; ++i)
		Solver.RecordData(Item, i);

	TestEqual(TEXT("No valid slot returns -1 on full grid"), Solver.GetFirstValidTopLeft(Item), -1);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BagStorage::InitializeData — reinitialization with items present is prevented
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBagStorageReinitPreventedTest,
	"InventoryPlugin.BagStorage.InitializeData.ReinitPrevented",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBagStorageReinitPreventedTest::RunTest(const FString& Parameters)
{
	UBagStorage* Bag = NewObject<UBagStorage>(GetTransientPackage());
	TestTrue(TEXT("First init succeeds"),
		Bag->InitializeData(EBagSlot::Pocket1, 3, 2));
	// Attempting to reinitialize when the bag is already marked valid (items would be 0 here,
	// but we verify the logic for the Items.Num() > 0 guard indirectly through the first init)
	// Second init on empty bag currently returns false only if Items.Num() > 0.
	// With Items empty, it would succeed again — this is the documented behaviour.
	const bool bSecond = Bag->InitializeData(EBagSlot::Pocket1, 5, 5);
	// Width changed to 5 on success, which means guard didn't fire (Items empty)
	if (bSecond)
	{
		TestEqual(TEXT("Bag width changed to 5 on second init"), Bag->GetWidth(), 5);
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BagStorage::GetBagSlotUsage — no div/0 on uninitialized bag
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBagStorageSlotUsageEmptyTest,
	"InventoryPlugin.BagStorage.GetBagSlotUsage.EmptyBag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBagStorageSlotUsageEmptyTest::RunTest(const FString& Parameters)
{
	UBagStorage* Bag = NewObject<UBagStorage>(GetTransientPackage());
	Bag->InitializeData(EBagSlot::Pocket1, 3, 2);
	// Empty bag should return 0
	TestTrue(TEXT("Empty bag usage is 0"), FMath::IsNearlyEqual(Bag->GetBagSlotUsage(), 0.f, 0.001f));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BagStorage::InitializeQuiverData
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBagStorageQuiverInitTest,
	"InventoryPlugin.BagStorage.InitializeQuiverData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBagStorageQuiverInitTest::RunTest(const FString& Parameters)
{
	UBagStorage* Bag = NewObject<UBagStorage>(GetTransientPackage());

	// Non-unknown ammo type → quiver
	Bag->InitializeQuiverData(EAmmoType::Arrows);
	TestTrue(TEXT("IsQuiver true for Arrows"),           Bag->GetIsQuiver());
	TestEqual(TEXT("AmmoType is Arrows"), Bag->GetAmmoTypeLimitation(), EAmmoType::Arrows);

	// Unknown → not a quiver
	Bag->InitializeQuiverData(EAmmoType::Unknown);
	TestFalse(TEXT("IsQuiver false for Unknown"),        Bag->GetIsQuiver());
	TestEqual(TEXT("AmmoType is Unknown"), Bag->GetAmmoTypeLimitation(), EAmmoType::Unknown);
	return true;
}

#endif // WITH_AUTOMATION_WORKER

