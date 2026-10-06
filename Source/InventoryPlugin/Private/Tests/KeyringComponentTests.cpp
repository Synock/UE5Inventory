#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "Components/KeyringComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeyringDuplicateLoadTest,
	"InventoryPlugin.Keyring.DuplicateLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeyringDuplicateLoadTest::RunTest(const FString& Parameters)
{
	UKeyringComponent* Keyring = NewObject<UKeyringComponent>(GetTransientPackage());
	if (!TestNotNull(TEXT("Keyring component"), Keyring))
		return false;

	Keyring->AddKey(60004, 15126);
	Keyring->AddKey(60004, 15126);

	TestEqual(TEXT("Reloading a key keeps one entry"), Keyring->GetAllPossessedKeys().Num(), 1);
	TestEqual(TEXT("First item mapping remains authoritative"), Keyring->GetItemFromKey(60004), 15126);

	Keyring->RemoveKey(60004);
	TestFalse(TEXT("One removal clears the key"), Keyring->HasKey(60004));
	TestTrue(TEXT("One removal clears the list"), Keyring->GetAllPossessedKeys().IsEmpty());
	return true;
}

#endif
