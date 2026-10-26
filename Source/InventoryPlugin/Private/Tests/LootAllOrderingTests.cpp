#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/LootAllOrdering.h"
#include "Items/InventoryItemBag.h"

namespace
{
struct FLootOrderFixture
{
	TMap<int32, UInventoryItemBase*> Items;

	UInventoryItemEquipable* AddEquipment(int32 Id, EEquipmentSlot Slot)
	{
		UInventoryItemEquipable* Item = NewObject<UInventoryItemEquipable>();
		Item->ItemID = Id;
		Item->EquipableSlotBitMask = static_cast<int32>(1u << static_cast<uint32>(Slot));
		Items.Add(Id, Item);
		return Item;
	}

	//------------------------------------------------------------------------------------------------------------------

	TArray<FMinimalItemStorage> Order(TArray<FMinimalItemStorage> Loot) const
	{
		return LootAllOrdering::Build(MoveTemp(Loot), [this](int32 Id)
		{
			return Items.FindRef(Id);
		});
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootAllArmorOrderTest, "InventoryPlugin.Loot.All.PreserveArmorOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootAllArmorOrderTest::RunTest(const FString& Parameters)
{
	FLootOrderFixture Fixture;
	Fixture.Items.Add(1, NewObject<UInventoryItemBase>());
	Fixture.AddEquipment(2, EEquipmentSlot::Head);
	UInventoryItemBag* Bag = NewObject<UInventoryItemBag>();
	Bag->Bag = true;
	Fixture.Items.Add(3, Bag);
	Fixture.AddEquipment(4, EEquipmentSlot::Torso);
	Fixture.AddEquipment(5, EEquipmentSlot::Waist);
	Fixture.AddEquipment(6, EEquipmentSlot::Primary)->Weapon = true;

	const TArray<FMinimalItemStorage> Ordered = Fixture.Order({{6, 50}, {5, 40}, {4, 30}, {3, 20}, {2, 10}, {1, 0}});
	const int32 Expected[] = {2, 5, 3, 4, 6, 1};
	TestEqual(TEXT("Every corpse entry is retained"), Ordered.Num(), 6);
	for (int32 Index = 0; Index < Ordered.Num(); ++Index)
		TestEqual(TEXT("Visible order with minimal belt promotion"), Ordered[Index].ItemID, Expected[Index]);

	const TArray<FMinimalItemStorage> WithoutBelt = Fixture.Order({{6, 0}, {4, 1}, {2, 2}, {3, 3}, {1, 4}});
	const int32 Unchanged[] = {6, 4, 2, 3, 1};
	for (int32 Index = 0; Index < WithoutBelt.Num(); ++Index)
		TestEqual(TEXT("Equipment order without a belt stays unchanged"), WithoutBelt[Index].ItemID, Unchanged[Index]);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootAllCombinedWaistTest, "InventoryPlugin.Loot.All.CombinedWaistAndMultipleBelts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootAllCombinedWaistTest::RunTest(const FString& Parameters)
{
	FLootOrderFixture Fixture;
	Fixture.AddEquipment(1, EEquipmentSlot::Head);
	Fixture.AddEquipment(2, EEquipmentSlot::Primary)->Weapon = true;
	UInventoryItemBag* Combined = NewObject<UInventoryItemBag>();
	Combined->Bag = true;
	Combined->MultiSlotItem = true;
	Combined->EquipableSlotBitMask = (1 << static_cast<uint8>(EEquipmentSlot::Waist)) |
		(1 << static_cast<uint8>(EEquipmentSlot::WaistBag1));
	Fixture.Items.Add(3, Combined);
	Fixture.AddEquipment(4, EEquipmentSlot::Torso);
	UInventoryItemEquipable* ArmorBelt = Fixture.AddEquipment(5, EEquipmentSlot::Waist);
	ArmorBelt->MultiSlotItem = true;
	ArmorBelt->EquipableSlotBitMask |= 1 << static_cast<uint8>(EEquipmentSlot::Torso);
	Fixture.AddEquipment(6, EEquipmentSlot::Waist);

	const TArray<FMinimalItemStorage> Ordered = Fixture.Order({{1, 0}, {2, 1}, {3, 2}, {4, 3}, {5, 4}, {6, 5}});
	const int32 Expected[] = {1, 3, 5, 6, 2, 4};
	for (int32 Index = 0; Index < Ordered.Num(); ++Index)
		TestEqual(TEXT("Waist providers keep order and precede weapon"), Ordered[Index].ItemID, Expected[Index]);
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootAllStorageIdentityTest, "InventoryPlugin.Loot.All.StorageIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootAllStorageIdentityTest::RunTest(const FString& Parameters)
{
	FLootOrderFixture Fixture;
	Fixture.AddEquipment(1, EEquipmentSlot::Waist);
	Fixture.AddEquipment(2, EEquipmentSlot::Primary)->Weapon = true;
	const TArray<FMinimalItemStorage> Original = {{1, 8, 17.f}, {2, 0, 83.f}, {1, 4, 39.f}, {999, 2, 61.f}};
	const TArray<FMinimalItemStorage> Ordered = Fixture.Order(Original);
	const int32 ExpectedPositions[] = {4, 8, 0, 2};
	TestEqual(TEXT("No entries lost or duplicated"), Ordered.Num(), Original.Num());
	for (int32 Index = 0; Index < Ordered.Num(); ++Index)
	{
		const FMinimalItemStorage& Entry = Ordered[Index];
		TestEqual(TEXT("Duplicate IDs retain distinct source positions"), Entry.TopLeftID, ExpectedPositions[Index]);
		const FMinimalItemStorage* Source = Original.FindByPredicate([&Entry](const FMinimalItemStorage& Item)
		{
			return Item.TopLeftID == Entry.TopLeftID;
		});
		TestEqual(TEXT("Durability follows the source entry"), Entry.Durability, Source->Durability);
	}
	TestEqual(TEXT("Source snapshot is unchanged"), Original[0].TopLeftID, 8);
	TestTrue(TEXT("Empty loot stays empty"), Fixture.Order({}).IsEmpty());
	return true;
}

#endif
