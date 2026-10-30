#include "Misc/AutomationTest.h"

#include "Components/EquipmentComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Items/InventoryItemEquipable.h"

#if WITH_AUTOMATION_WORKER

namespace
{
UWorld* CreateEquipmentAttachmentTestWorld(FAutomationTestBase& Test)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		Test.AddError(TEXT("Failed to create equipment attachment test world"));
	}
	return TestWorld;
}

void DestroyEquipmentAttachmentTestWorld(UWorld* TestWorld)
{
	if (TestWorld)
	{
		TestWorld->DestroyWorld(false);
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentAttachmentSkipsOwnerMeshWithoutAssetTest,
	"InventoryPlugin.Equipment.Attachments.SkipOwnerMeshWithoutAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentAttachmentSkipsOwnerMeshWithoutAssetTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = CreateEquipmentAttachmentTestWorld(*this);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn equipment attachment test character"));
		DestroyEquipmentAttachmentTestWorld(TestWorld);
		return false;
	}

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	TestNotNull(TEXT("Equipment component is created"), Equipment);

	const bool bAttached = Equipment && Equipment->AttachEquipmentComponentsToOwnerMeshIfReady();
	TestFalse(TEXT("Equipment attachment waits for an owner mesh asset"), bAttached);

	DestroyEquipmentAttachmentTestWorld(TestWorld);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentAttachmentMissingSocketsIsIdempotentTest,
	"InventoryPlugin.Equipment.Attachments.MissingSocketsAreIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentAttachmentMissingSocketsIsIdempotentTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = CreateEquipmentAttachmentTestWorld(*this);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn equipment attachment test character"));
		DestroyEquipmentAttachmentTestWorld(TestWorld);
		return false;
	}

	USkeletalMesh* EmptyMesh = NewObject<USkeletalMesh>(Character, TEXT("EmptyEquipmentAttachmentMesh"));
	Character->GetMesh()->SetSkeletalMesh(EmptyMesh);

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	TestNotNull(TEXT("Equipment component is created"), Equipment);

	const bool bFirstAttach = Equipment && Equipment->AttachEquipmentComponentsToOwnerMeshIfReady();
	const bool bSecondAttach = Equipment && Equipment->AttachEquipmentComponentsToOwnerMeshIfReady();

	TestFalse(TEXT("Equipment attachment does not attach to missing sockets"), bFirstAttach);
	TestFalse(TEXT("Repeated missing-socket attachment remains a no-op"), bSecondAttach);

	DestroyEquipmentAttachmentTestWorld(TestWorld);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentReplicationPreservesHeldItemTest,
	"InventoryPlugin.Equipment.Replication.PreservesHeldItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

//----------------------------------------------------------------------------------------------------------------------

bool FEquipmentReplicationPreservesHeldItemTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = CreateEquipmentAttachmentTestWorld(*this);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn equipment replication test character"));
		DestroyEquipmentAttachmentTestWorld(TestWorld);
		return false;
	}

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	UInventoryItemEquipable* HeldItem = NewObject<UInventoryItemEquipable>(Character);
	HeldItem->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Secondary);
	HeldItem->EquipmentMesh = NewObject<USkeletalMesh>(Character);
	Equipment->EquipItem(HeldItem, EEquipmentSlot::Secondary);

	USkeletalMeshComponent* Hand = Equipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Secondary);
	TestEqual(TEXT("Non-weapon item equips directly to the hand"), Hand->GetSkeletalMeshAsset(), HeldItem->EquipmentMesh);
	Equipment->OnRep_ItemList();
	TestEqual(TEXT("Replication retains the held non-weapon mesh"), Hand->GetSkeletalMeshAsset(), HeldItem->EquipmentMesh);
	Equipment->OnRep_ItemList();
	TestEqual(TEXT("Repeated replication retains the held mesh"), Hand->GetSkeletalMeshAsset(), HeldItem->EquipmentMesh);

	Hand->SetSkeletalMeshAsset(nullptr);
	Equipment->OnRep_ItemList();
	TestEqual(TEXT("Replication restores a missing held mesh"), Hand->GetSkeletalMeshAsset(), HeldItem->EquipmentMesh);

	DestroyEquipmentAttachmentTestWorld(TestWorld);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentReplicationClearsDistinctSheathTest,
	"InventoryPlugin.Equipment.Replication.ClearsDistinctSheath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

//----------------------------------------------------------------------------------------------------------------------

bool FEquipmentReplicationClearsDistinctSheathTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = CreateEquipmentAttachmentTestWorld(*this);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to spawn sheath replication test character"));
		DestroyEquipmentAttachmentTestWorld(TestWorld);
		return false;
	}

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	USkeletalMeshComponent* Hand = Equipment->GetSkeletalMeshComponentFromSocket(EEquipmentSocket::Secondary);
	for (const bool bShield : {false, true})
	{
		UInventoryItemEquipable* Item = NewObject<UInventoryItemEquipable>(Character);
		Item->EquipableSlotBitMask = 1 << static_cast<uint8>(EEquipmentSlot::Secondary);
		Item->EquipmentMesh = NewObject<USkeletalMesh>(Character);
		Item->Weapon = !bShield;
		Item->Shield = bShield;
		Equipment->EquipItem(Item, EEquipmentSlot::Secondary);

		const EEquipmentSocket SheathSocket = bShield ? EEquipmentSocket::BackSheath : EEquipmentSocket::SecondarySheath;
		USkeletalMeshComponent* Sheath = Equipment->GetSkeletalMeshComponentFromSocket(SheathSocket);
		Hand->SetSkeletalMeshAsset(Item->EquipmentMesh);
		Sheath->SetSkeletalMeshAsset(Item->EquipmentMesh);
		Equipment->OnRep_ItemList();
		TestEqual(bShield ? TEXT("Drawn shield remains in the hand") : TEXT("Drawn off-hand weapon remains in the hand"),
			Hand->GetSkeletalMeshAsset(), Item->EquipmentMesh);
		TestNull(bShield ? TEXT("Duplicate shield sheath mesh is cleared") : TEXT("Duplicate weapon sheath mesh is cleared"),
			Sheath->GetSkeletalMeshAsset());
		Equipment->RemoveItem(EEquipmentSlot::Secondary);
	}

	DestroyEquipmentAttachmentTestWorld(TestWorld);
	return true;
}

#endif
