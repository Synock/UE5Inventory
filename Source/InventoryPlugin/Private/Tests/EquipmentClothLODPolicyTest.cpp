#include "Misc/AutomationTest.h"

#include "Components/EquipmentComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEquipmentClothLODPolicyTest,
	"InventoryPlugin.Equipment.ClothLODPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentClothLODPolicyTest::RunTest(const FString& Parameters)
{
	USkeletalMesh* ClothMesh = LoadObject<USkeletalMesh>(
		nullptr, TEXT("/Game/Equipment/Human_Male/Hum_M_SmallCape.Hum_M_SmallCape"));
	if (!ClothMesh || ClothMesh->GetMeshClothingAssets().IsEmpty())
	{
		AddError(TEXT("Hum_M_SmallCape must remain a valid tracked cloth test fixture"));
		return false;
	}

	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	ACharacter* Character = TestWorld ? TestWorld->SpawnActor<ACharacter>() : nullptr;
	if (!Character)
	{
		AddError(TEXT("Failed to create cloth LOD policy test character"));
		if (TestWorld)
			TestWorld->DestroyWorld(false);
		return false;
	}

	UEquipmentComponent* Equipment = NewObject<UEquipmentComponent>(Character, TEXT("Equipment"));
	Equipment->RegisterComponent();
	TestFalse(TEXT("Equipment ticking starts disabled without cloth"), Equipment->IsComponentTickEnabled());

	USkeletalMeshComponent* ClothComponent = NewObject<USkeletalMeshComponent>(Equipment, TEXT("ClothOverlay"));
	ClothComponent->SetSkeletalMesh(ClothMesh);
	Equipment->VariableMeshesMap.Emplace(EEquipmentSlot::Back, ClothComponent);
	Equipment->RefreshClothPolicyTickState();
	TestTrue(TEXT("A cloth overlay enables equipment policy ticking"), Equipment->IsComponentTickEnabled());

	ClothComponent->SetPredictedLODLevel(0);
	ClothComponent->bDisableClothSimulation = false;
	TestFalse(TEXT("Already-enabled LOD 0 cloth does not request a reset"),
		Equipment->ApplyClothSimulationLODPolicy(ClothComponent));
	TestFalse(TEXT("LOD 0 cloth simulation remains enabled"), ClothComponent->bDisableClothSimulation != 0);

	for (const int32 LODIndex : {1, 2, 3})
	{
		ClothComponent->SetPredictedLODLevel(LODIndex);
		Equipment->ApplyClothSimulationLODPolicy(ClothComponent);
		TestTrue(FString::Printf(TEXT("LOD %d disables cloth simulation"), LODIndex),
			ClothComponent->bDisableClothSimulation != 0);
	}

	ClothComponent->SetPredictedLODLevel(0);
	TestTrue(TEXT("Returning to LOD 0 requests reset and re-enable"),
		Equipment->ApplyClothSimulationLODPolicy(ClothComponent));
	TestFalse(TEXT("Returning to LOD 0 enables cloth simulation"), ClothComponent->bDisableClothSimulation != 0);

	USkeletalMeshComponent* NonClothComponent = NewObject<USkeletalMeshComponent>(Equipment, TEXT("RigidOverlay"));
	NonClothComponent->SetSkeletalMesh(NewObject<USkeletalMesh>(GetTransientPackage()));
	NonClothComponent->SetPredictedLODLevel(2);
	TestFalse(TEXT("Non-cloth overlays are ignored"),
		Equipment->ApplyClothSimulationLODPolicy(NonClothComponent));
	TestFalse(TEXT("Non-cloth simulation flag is untouched"), NonClothComponent->bDisableClothSimulation != 0);

	Equipment->VariableMeshesMap.Empty();
	Equipment->RefreshClothPolicyTickState();
	TestFalse(TEXT("Removing the last cloth overlay disables policy ticking"), Equipment->IsComponentTickEnabled());

	TestWorld->DestroyWorld(false);
	return true;
}

#endif
