#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Actor.h"
#include "Items/InventoryItemBag.h"
#include "Items/InventoryItemEquipable.h"
#include "UI/EquipmentSlotWidget.h"

namespace
{
constexpr int32 BeltSlotBit(EEquipmentSlot Slot)
{
	return static_cast<int32>(1u << static_cast<uint32>(Slot));
}

UInventoryItemEquipable* MakeItem(UObject* Outer, int32 ItemId, int32 Mask, bool bMultiSlot = false)
{
	UInventoryItemEquipable* Item = NewObject<UInventoryItemEquipable>(Outer);
	Item->ItemID = ItemId;
	Item->EquipableSlotBitMask = Mask;
	Item->MultiSlotItem = bMultiSlot;
	return Item;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBeltPouchDependencyTest, "InventoryPlugin.Equipment.Belt.PouchDependency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBeltPouchDependencyTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Owner);
	UInventoryItemEquipable* Belt = MakeItem(Owner, 81001, BeltSlotBit(EEquipmentSlot::Waist));
	UInventoryItemEquipable* FirstPouch = MakeItem(Owner, 81002, BeltSlotBit(EEquipmentSlot::WaistBag1));
	UInventoryItemEquipable* SecondPouch = MakeItem(Owner, 81003, BeltSlotBit(EEquipmentSlot::WaistBag2));

	TestFalse(TEXT("First pouch needs a belt"), Equipment->CanEquipItemAt(FirstPouch, EEquipmentSlot::WaistBag1));
	TestFalse(TEXT("Second pouch needs a belt"), Equipment->CanEquipItemAt(SecondPouch, EEquipmentSlot::WaistBag2));
	TestEqual(TEXT("Auto equip skips a pouch without a belt"), Equipment->FindSuitableSlot(FirstPouch),
		EEquipmentSlot::Unknown);
	Equipment->EquipItem(Belt, EEquipmentSlot::Waist);
	TestTrue(TEXT("First pouch fits with a belt"), Equipment->CanEquipItemAt(FirstPouch, EEquipmentSlot::WaistBag1));
	TestTrue(TEXT("Second pouch fits with a belt"), Equipment->CanEquipItemAt(SecondPouch, EEquipmentSlot::WaistBag2));
	Equipment->EquipItem(FirstPouch, EEquipmentSlot::WaistBag1);
	Equipment->EquipItem(SecondPouch, EEquipmentSlot::WaistBag2);
	Equipment->ReduceEquipmentDurability(EEquipmentSlot::Waist, Belt->GetTotalDurability());
	float BrokenDurability = -1.f;
	TestTrue(TEXT("Broken belt remains an equipped item"),
		Equipment->GetEquipmentDurability(EEquipmentSlot::Waist, BrokenDurability));
	TestEqual(TEXT("Belt reaches zero durability"), BrokenDurability, 0.f);
	TestFalse(TEXT("Direct removal cannot orphan worn pouches"), Equipment->RemoveItem(EEquipmentSlot::Waist));
	TestFalse(TEXT("Server sale and swap removal guard rejects the belt"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Waist, Belt->ItemID));
	UInventoryNetComponent* Net = NewObject<UInventoryNetComponent>(Owner);
	TestTrue(TEXT("A well-formed blocked unequip RPC passes network validation"),
		Net->Server_PlayerUnequipItem_Validate(0, EBagSlot::Pocket1, Belt->ItemID, EEquipmentSlot::Waist));
	TestTrue(TEXT("A well-formed blocked drop RPC passes network validation"),
		Net->Server_DropItemFromEquipment_Validate(EEquipmentSlot::Waist, FVector::ZeroVector));
	TestFalse(TEXT("Invalid unequip slot still fails network validation"),
		Net->Server_PlayerUnequipItem_Validate(0, EBagSlot::Pocket1, Belt->ItemID, EEquipmentSlot::Unknown));
	Equipment->RemoveItem(EEquipmentSlot::WaistBag1);
	TestFalse(TEXT("Second pouch still blocks belt removal"), Equipment->RemoveItem(EEquipmentSlot::Waist));
	Equipment->RemoveItem(EEquipmentSlot::WaistBag2);
	TestTrue(TEXT("Removal guard permits the belt after both pouches are removed"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Waist, Belt->ItemID));
	TestTrue(TEXT("Belt can be removed after both pouches"), Equipment->RemoveItem(EEquipmentSlot::Waist));
	Equipment->EquipItem(Belt, EEquipmentSlot::Waist);
	Equipment->EquipItem(FirstPouch, EEquipmentSlot::WaistBag1);
	Equipment->RemoveAll();
	TestNull(TEXT("Death teardown removes the pouch"), Equipment->GetItemAtSlot(EEquipmentSlot::WaistBag1));
	TestNull(TEXT("Death teardown removes the belt"), Equipment->GetItemAtSlot(EEquipmentSlot::Waist));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBeltMultiSlotPouchTest, "InventoryPlugin.Equipment.Belt.MultiSlotPouch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBeltMultiSlotPouchTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Owner);
	Owner->AddInstanceComponent(Inventory);
	UInventoryItemEquipable* ArmorBelt = MakeItem(Owner, 81004,
		BeltSlotBit(EEquipmentSlot::Torso) | BeltSlotBit(EEquipmentSlot::Waist), true);
	UInventoryItemEquipable* Pouch = MakeItem(Owner, 81005, BeltSlotBit(EEquipmentSlot::WaistBag1));
	Equipment->EquipItem(ArmorBelt, EEquipmentSlot::Torso);
	TestTrue(TEXT("Multi-slot armor covers Waist"), Equipment->GetItemCoveringSlot(EEquipmentSlot::Waist) == ArmorBelt);
	TestTrue(TEXT("Multi-slot Waist coverage supports a pouch"),
		Equipment->CanEquipItemAt(Pouch, EEquipmentSlot::WaistBag1));
	Equipment->EquipItem(Pouch, EEquipmentSlot::WaistBag1);
	TestFalse(TEXT("Removing covering armor cannot orphan its pouch"), Equipment->RemoveItem(EEquipmentSlot::Torso));
	TestFalse(TEXT("Removal guard blocks multi-slot Waist covering gear"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Torso, ArmorBelt->ItemID));
	Equipment->RemoveItem(EEquipmentSlot::WaistBag1);
	Equipment->RemoveItem(EEquipmentSlot::Torso);

	UInventoryItemBag* Combined = NewObject<UInventoryItemBag>(Owner);
	Combined->ItemID = 81006;
	Combined->Bag = true;
	Combined->MultiSlotItem = true;
	Combined->EquipableSlotBitMask = BeltSlotBit(EEquipmentSlot::Waist) | BeltSlotBit(EEquipmentSlot::WaistBag1);
	TestFalse(TEXT("Combined item must be equipped at Waist"),
		Equipment->CanEquipItemAt(Combined, EEquipmentSlot::WaistBag1));
	TestFalse(TEXT("Combined item cannot be dropped on its secondary bag widget"),
		UEquipmentSlotWidget::CanEquipItemAtSlot(Combined, EEquipmentSlot::WaistBag1));
	TestTrue(TEXT("Combined item fits Waist without a separate belt"),
		Equipment->CanEquipItemAt(Combined, EEquipmentSlot::Waist));
	Equipment->EquipItem(Combined, EEquipmentSlot::Waist);
	TestEqual(TEXT("Integrated pouch is found through the covered bag slot"),
		Equipment->GetItemCoveringSlot(EEquipmentSlot::WaistBag1), static_cast<const UInventoryItemEquipable*>(Combined));
	TestEqual(TEXT("Waist bag storage maps to the first pouch grid"),
		UInventoryComponent::GetBagSlotFromInventory(EEquipmentSlot::Waist), EBagSlot::WaistBag1);
	TestFalse(TEXT("A separate pouch conflicts with the integrated pouch"),
		Equipment->CanEquipItemAt(Pouch, EEquipmentSlot::WaistBag1));
	Inventory->BagSet(EBagSlot::WaistBag1, true, 2, 2);
	TestTrue(TEXT("Empty integrated storage permits removal"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Waist, Combined->ItemID));
	UInventoryItemBase* Stored = NewObject<UInventoryItemBase>(Owner);
	const FGuid Reservation = FGuid::NewGuid();
	TestTrue(TEXT("Reserve integrated pouch storage"),
		Inventory->ReserveItemFootprint(Reservation, EBagSlot::WaistBag1, Stored, 0));
	TestFalse(TEXT("Reserved integrated storage blocks removal"),
		UInventoryNetComponent::CanRemoveEquippedItemFromComponentsForTests(
			Equipment, Inventory, EEquipmentSlot::Waist, Combined->ItemID));
	TestFalse(TEXT("Direct removal also preserves reserved integrated storage"),
		Equipment->RemoveItem(EEquipmentSlot::Waist));
	Inventory->ReleaseItemFootprint(Reservation);
	TestTrue(TEXT("Combined item can be removed once storage is clear"),
		Equipment->RemoveItem(EEquipmentSlot::Waist));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBeltHipSheathTest, "InventoryPlugin.Equipment.Belt.HipSheath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBeltHipSheathTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* Weapon = MakeItem(Owner, 81007, BeltSlotBit(EEquipmentSlot::Primary));
	Weapon->Weapon = true;
	Weapon->EquipmentMesh = NewObject<USkeletalMesh>(Owner);
	UInventoryItemEquipable* Belt = MakeItem(Owner, 81008, BeltSlotBit(EEquipmentSlot::Waist));
	USkeletalMeshComponent* Hand = Equipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Primary);
	USkeletalMeshComponent* Sheath = Equipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::PrimarySheath);
	if (!TestNotNull(TEXT("Primary hand component"), Hand) ||
		!TestNotNull(TEXT("Primary hip sheath component"), Sheath))
		return false;
	Equipment->EquipItem(Weapon, EEquipmentSlot::Primary);
	TestEqual(TEXT("No-belt hip weapon starts in hand"),
		Hand->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	TestNull(TEXT("No-belt hip sheath stays empty"), Sheath->GetSkeletalMeshAsset());
	Equipment->SheathMelee();
	TestEqual(TEXT("Cannot sheath a hip weapon without belt"),
		Hand->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	Equipment->SheathRanged();
	TestNull(TEXT("Ranged sheathing cannot bypass the hip belt rule"), Sheath->GetSkeletalMeshAsset());
	Equipment->EquipItem(Belt, EEquipmentSlot::Waist);
	Equipment->SheathMelee();
	TestEqual(TEXT("Belt allows hip sheathing"),
		Sheath->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	TestNull(TEXT("Sheathed weapon leaves hand"), Hand->GetSkeletalMeshAsset());
	Equipment->RemoveItem(EEquipmentSlot::Waist);
	TestEqual(TEXT("Belt removal draws hip weapon"),
		Hand->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	TestNull(TEXT("Removed belt leaves hip sheath empty"), Sheath->GetSkeletalMeshAsset());

	UEquipmentComponent* LoadedEquipment = NewObject<UEquipmentComponent>(Owner);
	USkeletalMeshComponent* LoadedHand = LoadedEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Primary);
	USkeletalMeshComponent* LoadedSheath = LoadedEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::PrimarySheath);
	LoadedEquipment->EquipItem(Belt, EEquipmentSlot::Waist);
	LoadedEquipment->EquipItem(Weapon, EEquipmentSlot::Primary);
	LoadedEquipment->OnRep_ItemList();
	TestEqual(TEXT("Loading belt before weapon leaves the weapon sheathed"),
		LoadedSheath->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	TestNull(TEXT("Loaded weapon is absent from hand"), LoadedHand->GetSkeletalMeshAsset());
	LoadedEquipment->UnsheathMelee();
	LoadedEquipment->OnRep_ItemList();
	TestEqual(TEXT("Equipment refresh preserves a drawn hip weapon"),
		LoadedHand->GetSkeletalMeshAsset(), Weapon->EquipmentMesh);
	TestNull(TEXT("Equipment refresh does not duplicate a drawn hip weapon"),
		LoadedSheath->GetSkeletalMeshAsset());

	UEquipmentComponent* OtherEquipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* Offhand = MakeItem(Owner, 81009, BeltSlotBit(EEquipmentSlot::Secondary));
	Offhand->Weapon = true;
	Offhand->EquipmentMesh = NewObject<USkeletalMesh>(Owner);
	USkeletalMeshComponent* OffhandSocket = OtherEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Secondary);
	USkeletalMeshComponent* OffhandSheath = OtherEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::SecondarySheath);
	if (!TestNotNull(TEXT("Secondary hand component"), OffhandSocket) ||
		!TestNotNull(TEXT("Secondary hip sheath component"), OffhandSheath))
		return false;
	OtherEquipment->EquipItem(Offhand, EEquipmentSlot::Secondary);
	TestEqual(TEXT("No-belt offhand weapon starts in hand"), OffhandSocket->GetSkeletalMeshAsset(), Offhand->EquipmentMesh);
	OtherEquipment->SheathMelee();
	TestNull(TEXT("No-belt offhand sheath stays empty"), OffhandSheath->GetSkeletalMeshAsset());
	OtherEquipment->EquipItem(Belt, EEquipmentSlot::Waist);
	OtherEquipment->SheathMelee();
	TestEqual(TEXT("Belt allows offhand sheathing"), OffhandSheath->GetSkeletalMeshAsset(), Offhand->EquipmentMesh);
	OtherEquipment->RemoveItem(EEquipmentSlot::Waist);
	TestEqual(TEXT("Belt removal draws offhand weapon"), OffhandSocket->GetSkeletalMeshAsset(), Offhand->EquipmentMesh);
	OtherEquipment->OnRep_ItemList();
	TestNull(TEXT("Offhand refresh does not duplicate the drawn weapon"), OffhandSheath->GetSkeletalMeshAsset());

	UEquipmentComponent* BackEquipment = NewObject<UEquipmentComponent>(Owner);
	UInventoryItemEquipable* Shield = MakeItem(Owner, 81010, BeltSlotBit(EEquipmentSlot::Secondary));
	Shield->Shield = true;
	Shield->EquipmentMesh = NewObject<USkeletalMesh>(Owner);
	BackEquipment->EquipItem(Shield, EEquipmentSlot::Secondary);
	TestEqual(TEXT("Back-mounted shield remains stowed without a belt"),
		BackEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::BackSheath)->GetSkeletalMeshAsset(),
		Shield->EquipmentMesh);
	UInventoryItemEquipable* Ranged = MakeItem(Owner, 81011, BeltSlotBit(EEquipmentSlot::Range));
	Ranged->Weapon = true;
	Ranged->EquipmentMesh = NewObject<USkeletalMesh>(Owner);
	BackEquipment->EquipItem(Ranged, EEquipmentSlot::Range);
	TestEqual(TEXT("Ranged weapon remains stowed without a belt"),
		BackEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::RangedSheath)->GetSkeletalMeshAsset(),
		Ranged->EquipmentMesh);
	BackEquipment->UnsheathRanged();
	BackEquipment->OnRep_ItemList();
	TestEqual(TEXT("Equipment refresh preserves a drawn ranged weapon"),
		BackEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Primary)->GetSkeletalMeshAsset(),
		Ranged->EquipmentMesh);
	TestNull(TEXT("Equipment refresh does not duplicate a drawn ranged weapon"),
		BackEquipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::RangedSheath)->GetSkeletalMeshAsset());
	return true;
}

#endif
