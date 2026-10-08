// Copyright 2025 Maximilien (Synock) Guislain

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Loot/RandomizedEquipmentPool.h"
#include "Items/InventoryItemBase.h"

// ─────────────────────────────────────────────────────────────────────────────
// FItemAlternative::GetItem — empty list returns nullptr
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemAlternativeEmptyListTest,
	"InventoryPlugin.LootPool.ItemAlternative.EmptyListReturnsNullptr",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemAlternativeEmptyListTest::RunTest(const FString& Parameters)
{
	FItemAlternative Alt;
	// TotalProbability == 0 → should return nullptr without crashing
	TestNull(TEXT("Empty alternative list returns nullptr"), Alt.GetItem());
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// FItemAlternative::GetItem — single item always selected
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemAlternativeSingleItemTest,
	"InventoryPlugin.LootPool.ItemAlternative.SingleItemAlwaysSelected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemAlternativeSingleItemTest::RunTest(const FString& Parameters)
{
	UInventoryItemBase* Item = NewObject<UInventoryItemBase>(GetTransientPackage());
	Item->ItemID = 42;

	FItemAlternative Alt;
	FItemProbability Entry;
	Entry.Item = Item;
	Entry.Probability = 1.0f;
	Alt.AlternativeList.Add(Entry);

	// Run many times — single-item list must always return that item
	for (int32 i = 0; i < 20; ++i)
	{
		TestEqual(TEXT("Single item always returned"), Alt.GetItem(), Item);
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// FItemAlternative::GetItem — two equal-probability items
//   With the bug fix, each should appear roughly 50% of the time.
//   We run 200 trials and verify both items appear at least once.
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemAlternativeTwoItemsBothReachableTest,
	"InventoryPlugin.LootPool.ItemAlternative.TwoItems.BothReachable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemAlternativeTwoItemsBothReachableTest::RunTest(const FString& Parameters)
{
	UInventoryItemBase* ItemA = NewObject<UInventoryItemBase>(GetTransientPackage());
	UInventoryItemBase* ItemB = NewObject<UInventoryItemBase>(GetTransientPackage());
	ItemA->ItemID = 1;
	ItemB->ItemID = 2;

	FItemAlternative Alt;
	FItemProbability EntryA; EntryA.Item = ItemA; EntryA.Probability = 1.f;
	FItemProbability EntryB; EntryB.Item = ItemB; EntryB.Probability = 1.f;
	Alt.AlternativeList.Add(EntryA);
	Alt.AlternativeList.Add(EntryB);

	int32 CountA = 0, CountB = 0;
	for (int32 i = 0; i < 200; ++i)
	{
		UInventoryItemBase* Got = Alt.GetItem();
		if (Got == ItemA) ++CountA;
		else if (Got == ItemB) ++CountB;
	}

	TestTrue(TEXT("Item A appears at least once"), CountA > 0);
	TestTrue(TEXT("Item B appears at least once"), CountB > 0);
	TestEqual(TEXT("All 200 results accounted for"), CountA + CountB, 200);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// FItemAlternative::GetItem — four equal-probability items: all must be reachable
//   This test exposes / validates the probability accumulation bug fix.
//   BUG: Before fix, item 4 (index 3) was NEVER returned because SumProbability
//   accumulated via `+= AlternativeProbability` (which already includes old sum),
//   so by iteration 3 SumProbability was already >= 1.0, making AlternativeProbability >= 1.0
//   for item 4 and letting item 3 catch all remaining probability.
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemAlternativeFourItemsAllReachableTest,
	"InventoryPlugin.LootPool.ItemAlternative.FourItems.AllReachable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemAlternativeFourItemsAllReachableTest::RunTest(const FString& Parameters)
{
	TArray<UInventoryItemBase*> Items;
	FItemAlternative Alt;
	for (int32 i = 0; i < 4; ++i)
	{
		UInventoryItemBase* Item = NewObject<UInventoryItemBase>(GetTransientPackage());
		Item->ItemID = i + 1;
		Items.Add(Item);
		FItemProbability Entry;
		Entry.Item = Item;
		Entry.Probability = 1.0f; // all equal
		Alt.AlternativeList.Add(Entry);
	}

	TMap<int32, int32> Counts;
	for (int32 i = 0; i < 400; ++i)
	{
		UInventoryItemBase* Got = Alt.GetItem();
		if (Got) Counts.FindOrAdd(Got->ItemID)++;
	}

	TestTrue(TEXT("Item 1 reachable"), Counts.FindRef(1) > 0);
	TestTrue(TEXT("Item 2 reachable"), Counts.FindRef(2) > 0);
	TestTrue(TEXT("Item 3 reachable"), Counts.FindRef(3) > 0);
	// Item 4 was the one missing with the bug
	TestTrue(TEXT("Item 4 reachable (probability accumulation fix)"), Counts.FindRef(4) > 0);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// URandomizedLootPool::GetRandomisedItems — null item in pool does not crash
//   Validates the null guard added to GetRandomisedItems.
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolNullItemNoCrashTest,
	"InventoryPlugin.LootPool.GetRandomisedItems.NullItemNoCrash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolNullItemNoCrashTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());

	// Add a legitimate item
	UInventoryItemBase* ValidItem = NewObject<UInventoryItemBase>(GetTransientPackage());
	ValidItem->ItemID = 10;
	FItemProbability ValidEntry; ValidEntry.Item = ValidItem; ValidEntry.Probability = 1.0f;
	Pool->LootPool.Add(ValidEntry);

	// Add a null-item entry (simulates a misconfigured data asset row)
	FItemProbability NullEntry; NullEntry.Item = nullptr; NullEntry.Probability = 1.0f;
	Pool->LootPool.Add(NullEntry);

	// Should not crash; the null entry must be skipped
	TArray<int32> Results = Pool->GetRandomisedItems();
	TestTrue(TEXT("Valid item is included"), Results.Contains(10));
	TestFalse(TEXT("No invalid/default ItemID -1 in results"), Results.Contains(-1));
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// URandomizedLootPool::GetRandomisedCoin — result stays within range
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolGetRandomisedCoinRangeTest,
	"InventoryPlugin.LootPool.GetRandomisedCoin.StaysInRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolGetRandomisedCoinRangeTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());
	Pool->MinCoin = 10.f;
	Pool->MaxCoin = 50.f;
	Pool->ProbabilityCoin = 1.0f; // always drop coins

	for (int32 i = 0; i < 50; ++i)
	{
		const float Coin = Pool->GetRandomisedCoin();
		TestTrue(TEXT("Coin >= MinCoin"), Coin >= 10.f);
		TestTrue(TEXT("Coin <= MaxCoin"), Coin <= 50.f);
	}
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolGetRandomisedCoinZeroProbTest,
	"InventoryPlugin.LootPool.GetRandomisedCoin.ZeroProbabilityAlwaysZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolGetRandomisedCoinZeroProbTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());
	Pool->MinCoin = 100.f;
	Pool->MaxCoin = 500.f;
	Pool->ProbabilityCoin = 0.0f; // never drop coins

	for (int32 i = 0; i < 20; ++i)
	{
		TestEqual(TEXT("Zero probability produces no coin"), Pool->GetRandomisedCoin(), 0.f);
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// URandomizedLootPool::GetRandomisedItems — empty pool returns empty array
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolEmptyPoolTest,
	"InventoryPlugin.LootPool.GetRandomisedItems.EmptyPool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolEmptyPoolTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());
	const TArray<int32> Results = Pool->GetRandomisedItems();
	TestTrue(TEXT("Empty pool returns empty array"), Results.Num() == 0);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// URandomizedLootPool::GetRandomisedItems — probability 0 drops nothing
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolZeroProbItemTest,
	"InventoryPlugin.LootPool.GetRandomisedItems.ZeroProbabilityNeverDrops",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolZeroProbItemTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());
	UInventoryItemBase* Item = NewObject<UInventoryItemBase>(GetTransientPackage());
	Item->ItemID = 99;
	FItemProbability Entry; Entry.Item = Item; Entry.Probability = 0.0f;
	Pool->LootPool.Add(Entry);

	// Over 50 trials, a probability-0 item should never appear
	for (int32 i = 0; i < 50; ++i)
	{
		TestFalse(TEXT("Prob-0 item never drops"), Pool->GetRandomisedItems().Contains(99));
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// URandomizedLootPool::GetRandomisedItems — probability 1 always drops
// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolFullProbItemTest,
	"InventoryPlugin.LootPool.GetRandomisedItems.FullProbabilityAlwaysDrops",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolFullProbItemTest::RunTest(const FString& Parameters)
{
	URandomizedLootPool* Pool = NewObject<URandomizedLootPool>(GetTransientPackage());
	UInventoryItemBase* Item = NewObject<UInventoryItemBase>(GetTransientPackage());
	Item->ItemID = 77;
	FItemProbability Entry; Entry.Item = Item; Entry.Probability = 1.0f;
	Pool->LootPool.Add(Entry);

	for (int32 i = 0; i < 20; ++i)
	{
		TestTrue(TEXT("Prob-1 item always drops"), Pool->GetRandomisedItems().Contains(77));
	}
	return true;
}

#endif // WITH_AUTOMATION_WORKER



