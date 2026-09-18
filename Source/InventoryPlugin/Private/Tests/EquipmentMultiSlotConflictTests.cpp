#include "Misc/AutomationTest.h"

#include "Components/EquipmentComponent.h"
#include "Definitions.h"
#include "Items/InventoryItemEquipable.h"
#include "UI/EquipmentSlotWidget.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#if WITH_AUTOMATION_WORKER

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
UWorld* CreateMultiSlotTestWorld(FAutomationTestBase& Test)
{
	UWorld* W = UWorld::CreateWorld(EWorldType::Game, false);
	if (!W)
		Test.AddError(TEXT("Failed to create MultiSlotConflict test world"));
	return W;
}

void DestroyMultiSlotTestWorld(UWorld* W)
{
	if (W)
		W->DestroyWorld(false);
}

/**
 * Create a throwaway UInventoryItemEquipable with a given bitmask.
 * @param Outer      Outer object (e.g. a spawned Character)
 * @param BitMask    Which EEquipmentSlot bits the item accepts / occupies
 * @param bMultiSlot True → item occupies ALL bitmask slots simultaneously
 */
UInventoryItemEquipable* MakeEquipable(UObject* Outer, int32 BitMask, bool bMultiSlot)
{
	UInventoryItemEquipable* Item = NewObject<UInventoryItemEquipable>(Outer);
	Item->EquipableSlotBitMask = BitMask;
	Item->MultiSlotItem         = bMultiSlot;
	Item->ItemID                = FMath::RandRange(1, 99999);
	return Item;
}

/** Bitmask helper — converts an EEquipmentSlot value to its bit. */
constexpr int32 SlotBit(EEquipmentSlot S) { return static_cast<int32>(1u << static_cast<uint32>(S)); }
} // namespace

// ---------------------------------------------------------------------------
// Test 1 — Baseline: multi-slot item fits when all required slots are empty
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentMultiSlotAllSlotsEmptyTest,
	"InventoryPlugin.Equipment.MultiSlot.CanEquipItemAt.MultiSlotAllSlotsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentMultiSlotAllSlotsEmptyTest::RunTest(const FString& Parameters)
{
	UWorld* World = CreateMultiSlotTestWorld(*this);
	ACharacter* Character = World ? World->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn character for MultiSlot baseline test"));
		DestroyMultiSlotTestWorld(World);
		return false;
	}

	UEquipmentComponent* Equip = NewObject<UEquipmentComponent>(Character, TEXT("Equip"));
	TestNotNull(TEXT("EquipmentComponent created"), Equip);

	// Multi-slot item occupying BackPack1 + BackPack2
	const int32 Mask = SlotBit(EEquipmentSlot::BackPack1) | SlotBit(EEquipmentSlot::BackPack2);
	UInventoryItemEquipable* Backpack = MakeEquipable(Character, Mask, /*bMultiSlot=*/true);

	// No items equipped yet — both slots empty → should succeed
	const bool bResult = Equip->CanEquipItemAt(Backpack, EEquipmentSlot::BackPack1);
	TestTrue(TEXT("Multi-slot item can equip when all required slots are empty"), bResult);

	DestroyMultiSlotTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 2 — Regression: multi-slot item blocked when a sibling slot is occupied
// This is the exact scenario that used to reach the server RPC with no client
// feedback. CanEquipItemAt must return false.
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentMultiSlotSiblingOccupiedTest,
	"InventoryPlugin.Equipment.MultiSlot.CanEquipItemAt.MultiSlotSiblingOccupied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentMultiSlotSiblingOccupiedTest::RunTest(const FString& Parameters)
{
	UWorld* World = CreateMultiSlotTestWorld(*this);
	ACharacter* Character = World ? World->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn character for MultiSlot regression test"));
		DestroyMultiSlotTestWorld(World);
		return false;
	}

	UEquipmentComponent* Equip = NewObject<UEquipmentComponent>(Character, TEXT("Equip"));
	TestNotNull(TEXT("EquipmentComponent created"), Equip);

	// Occupy the Shoulders slot with a plain single-slot item
	const int32 ShouldersMask = SlotBit(EEquipmentSlot::Shoulders);
	UInventoryItemEquipable* SingleSlotItem = MakeEquipable(Character, ShouldersMask, /*bMultiSlot=*/false);
	Equip->EquipItem(SingleSlotItem, EEquipmentSlot::Shoulders);
	TestNotNull(TEXT("Single-slot item is now equipped in Shoulders"),
		Equip->GetItemAtSlot(EEquipmentSlot::Shoulders));

	// Multi-slot item that requires both Shoulders and BackPack1 simultaneously
	const int32 MultiMask = SlotBit(EEquipmentSlot::Shoulders) | SlotBit(EEquipmentSlot::BackPack1);
	UInventoryItemEquipable* MultiItem = MakeEquipable(Character, MultiMask, /*bMultiSlot=*/true);

	// BackPack1 is empty, but Shoulders is occupied → must be rejected
	const bool bResult = Equip->CanEquipItemAt(MultiItem, EEquipmentSlot::BackPack1);
	TestFalse(TEXT("Multi-slot item is blocked when a sibling slot (Shoulders) is already occupied"), bResult);

	DestroyMultiSlotTestWorld(World);
	return true;
}

// ---------------------------------------------------------------------------
// Test 3 — Codify the existing static-helper guard: multi-slot items must not
// be dropped onto WaistBag2 or BackPack2 (secondary/overflow slots).
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentMultiSlotSecondarySlotRejectedTest,
	"InventoryPlugin.Equipment.MultiSlot.CanEquipItemAtSlot.SecondarySlotRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentMultiSlotSecondarySlotRejectedTest::RunTest(const FString& Parameters)
{
	// WaistBag multi-slot: occupies WaistBag1 + WaistBag2
	const int32 WaistMask = SlotBit(EEquipmentSlot::WaistBag1) | SlotBit(EEquipmentSlot::WaistBag2);
	UInventoryItemEquipable* WaistBag = NewObject<UInventoryItemEquipable>(GetTransientPackage());
	WaistBag->EquipableSlotBitMask = WaistMask;
	WaistBag->MultiSlotItem        = true;

	// Dropping on the primary slot is fine (bitmask passes)
	TestTrue(TEXT("Multi-slot WaistBag accepted on WaistBag1 (primary)"),
		UEquipmentSlotWidget::CanEquipItemAtSlot(WaistBag, EEquipmentSlot::WaistBag1));

	// Dropping directly on the secondary slot must be rejected
	TestFalse(TEXT("Multi-slot WaistBag rejected on WaistBag2 (secondary overflow slot)"),
		UEquipmentSlotWidget::CanEquipItemAtSlot(WaistBag, EEquipmentSlot::WaistBag2));

	// Same for BackPack
	const int32 BackMask = SlotBit(EEquipmentSlot::BackPack1) | SlotBit(EEquipmentSlot::BackPack2);
	UInventoryItemEquipable* BackPack = NewObject<UInventoryItemEquipable>(GetTransientPackage());
	BackPack->EquipableSlotBitMask = BackMask;
	BackPack->MultiSlotItem        = true;

	TestTrue(TEXT("Multi-slot BackPack accepted on BackPack1 (primary)"),
		UEquipmentSlotWidget::CanEquipItemAtSlot(BackPack, EEquipmentSlot::BackPack1));
	TestFalse(TEXT("Multi-slot BackPack rejected on BackPack2 (secondary overflow slot)"),
		UEquipmentSlotWidget::CanEquipItemAtSlot(BackPack, EEquipmentSlot::BackPack2));

	return true;
}

#endif // WITH_AUTOMATION_WORKER
