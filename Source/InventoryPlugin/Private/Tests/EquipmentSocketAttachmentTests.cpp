#include "Misc/AutomationTest.h"

#include "Components/EquipmentComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

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

#endif
