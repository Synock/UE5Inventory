#include "Misc/AutomationTest.h"

#include "Animation/AnimInstance.h"
#include "Components/EquipmentComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Items/InventoryItemEquipable.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentAnimatedOverlayTest,
	"InventoryPlugin.Equipment.AnimatedOverlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentAnimatedOverlayTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to create animated overlay test character"));
		if (TestWorld)
			TestWorld->DestroyWorld(false);
		return false;
	}

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	Equipment->RegisterComponent();
	UInventoryItemEquipable* Item = NewObject<UInventoryItemEquipable>(GetTransientPackage());
	USkeletalMesh* OverlayMesh = NewObject<USkeletalMesh>(GetTransientPackage());
	const int32 SlotIndex = static_cast<int32>(EEquipmentSlot::Torso);
	Equipment->Equipment[SlotIndex] = Item;

	Item->EquipmentAnimInstanceClass = UAnimInstance::StaticClass();
	USkeletalMeshComponent* Overlay = Equipment->CreateAndRegisterOverlayComponent(EEquipmentSlot::Torso, OverlayMesh);
	TestNotNull(TEXT("The animated overlay component is created"), Overlay);
	if (Overlay)
	{
		TestNull(TEXT("Independent animation does not use leader pose"), Overlay->LeaderPoseComponent.Get());
		TestEqual(TEXT("The configured equipment AnimBP is active"), Overlay->GetAnimClass(), UAnimInstance::StaticClass());

		Equipment->UpdateMasterMeshComponent(Character->GetMesh());
		TestNull(TEXT("Refreshing the master mesh preserves independent animation"), Overlay->LeaderPoseComponent.Get());

		Item->EquipmentAnimInstanceClass = nullptr;
		Equipment->TryUpdateDynamicMeshes({{EEquipmentSlot::Torso, OverlayMesh}}, {});
		TestEqual(TEXT("Clearing the opt-in restores leader pose"), Overlay->LeaderPoseComponent.Get(),
			static_cast<USkinnedMeshComponent*>(Character->GetMesh()));
		TestNull(TEXT("Clearing the opt-in removes the equipment AnimBP"), Overlay->GetAnimClass());

		Item->EquipmentAnimInstanceClass = UAnimInstance::StaticClass();
		Equipment->TryUpdateDynamicMeshes({{EEquipmentSlot::Torso, OverlayMesh}}, {});
		TestNull(TEXT("Re-enabling the opt-in releases leader pose"), Overlay->LeaderPoseComponent.Get());
		TestEqual(TEXT("Re-enabling the opt-in restores the equipment AnimBP"), Overlay->GetAnimClass(),
			UAnimInstance::StaticClass());
	}

	Equipment->TryUpdateDynamicMeshes({}, {});
	TestNull(TEXT("Removing the slot destroys its managed overlay"),
		Equipment->GetOverlayComponentForSlot(EEquipmentSlot::Torso));

	TestWorld->DestroyWorld(false);
	return true;
}

#endif
