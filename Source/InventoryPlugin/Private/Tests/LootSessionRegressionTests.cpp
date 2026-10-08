#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/InventoryNetComponent.h"
#include "Components/LootPoolComponent.h"
#include "Engine/World.h"
#include "Tests/LootSessionTestTypes.h"

namespace
{
	UInventoryNetComponent* AddTestNetComponent(AActor* Owner)
	{
		UInventoryNetComponent* Component = NewObject<UInventoryNetComponent>(Owner);
		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolOwnerOnlyReplicationTest,
	"InventoryPlugin.Loot.Regression.ItemsRemainOwnerOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolOwnerOnlyReplicationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Items remains owner-only"), ULootPoolComponent::GetItemsReplicationConditionForTests(),
		COND_OwnerOnly);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootSessionOwnershipLifecycleTest,
	"InventoryPlugin.Loot.Regression.OwnershipLifecycleAndContention",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootSessionOwnershipLifecycleTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* OriginalOwner = World->SpawnActor<AActor>();
	AActor* FirstPlayer = World->SpawnActor<AActor>();
	AActor* SecondPlayer = World->SpawnActor<AActor>();
	ALootSessionTestActor* LootActor = World->SpawnActor<ALootSessionTestActor>();
	UInventoryNetComponent* FirstNet = AddTestNetComponent(FirstPlayer);
	UInventoryNetComponent* SecondNet = AddTestNetComponent(SecondPlayer);
	LootActor->SetOwner(OriginalOwner);

	FirstNet->HandleLootActorForTests(LootActor);
	TestTrue(TEXT("First request acquires the loot lock"), LootActor->GetIsBeingLooted());
	TestEqual(TEXT("First request owns the loot actor"), LootActor->GetOwner(), FirstPlayer);
	TestEqual(TEXT("First component publishes its loot target"), FirstNet->LootedActor.Get(),
		static_cast<AActor*>(LootActor));
	TestEqual(TEXT("Previous owner is retained for restoration"), FirstNet->GetPreviousLootOwnerForTests(), OriginalOwner);

	TestTrue(TEXT("Busy targets remain structurally valid so contention cannot kick the requester"),
		SecondNet->ValidateLootActorForTests(LootActor));
	SecondNet->HandleLootActorForTests(LootActor);
	TestNull(TEXT("Contending component does not receive a loot target"), SecondNet->LootedActor.Get());
	TestEqual(TEXT("Contention cannot retarget actor ownership"), LootActor->GetOwner(), FirstPlayer);
	TestEqual(TEXT("Rejected contention does not call StartLooting again"), LootActor->StartCalls, 1);

	FirstNet->HandleStopLootingForTests();
	TestFalse(TEXT("Stopping releases the loot lock"), LootActor->GetIsBeingLooted());
	TestEqual(TEXT("Stopping restores the prior owner"), LootActor->GetOwner(), OriginalOwner);
	TestNull(TEXT("Stopping clears the session target"), FirstNet->LootedActor.Get());

	FirstNet->HandleLootActorForTests(LootActor);
	FirstNet->HandleEndPlayCleanupForTests();
	TestFalse(TEXT("EndPlay releases the loot lock"), LootActor->GetIsBeingLooted());
	TestEqual(TEXT("EndPlay restores the prior owner"), LootActor->GetOwner(), OriginalOwner);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootStaleRequestValidationTest,
	"InventoryPlugin.Loot.Regression.StaleRequestsSoftFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootStaleRequestValidationTest::RunTest(const FString& Parameters)
{
	UInventoryNetComponent* Net = NewObject<UInventoryNetComponent>(GetTransientPackage());
	TestTrue(TEXT("Well-formed loot request is valid even if mutable session state changed"),
		Net->ValidatePlayerLootItemForTests(0, EBagSlot::Pocket1, 100, 4));
	TestTrue(TEXT("Well-formed equip-from-loot request is valid even if source state changed"),
		Net->ValidatePlayerEquipItemFromLootForTests(100, EEquipmentSlot::Head, 4));
	TestFalse(TEXT("Negative destination index is malformed"),
		Net->ValidatePlayerLootItemForTests(-1, EBagSlot::Pocket1, 100, 4));
	TestFalse(TEXT("Unknown destination bag is malformed"),
		Net->ValidatePlayerLootItemForTests(0, EBagSlot::Unknown, 100, 4));
	TestFalse(TEXT("Non-positive item id is malformed"),
		Net->ValidatePlayerLootItemForTests(0, EBagSlot::Pocket1, 0, 4));
	TestFalse(TEXT("Unknown equipment slot is malformed"),
		Net->ValidatePlayerEquipItemFromLootForTests(100, EEquipmentSlot::Unknown, 4));

	Net->HandlePlayerLootItemForTests(0, EBagSlot::Pocket1, 100, 4);
	TestEqual(TEXT("Missing mutable session is softly rejected by the handler"),
		Net->GetRejectedLootRequestCountForTests(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPoolRepNotifyTest,
	"InventoryPlugin.Loot.Regression.RepNotifyBroadcasts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLootPoolRepNotifyTest::RunTest(const FString& Parameters)
{
	ULootPoolComponent* Pool = NewObject<ULootPoolComponent>(GetTransientPackage());
	ULootPoolTestListener* Listener = NewObject<ULootPoolTestListener>(GetTransientPackage());
	Pool->LootPoolDispatcher.AddDynamic(Listener, &ULootPoolTestListener::HandleLootPoolChanged);

	Pool->OnRep_LootPool();
	TestEqual(TEXT("Replicated loot changes notify the open grid"), Listener->NotificationCount, 1);
	return true;
}

#endif // WITH_AUTOMATION_WORKER
