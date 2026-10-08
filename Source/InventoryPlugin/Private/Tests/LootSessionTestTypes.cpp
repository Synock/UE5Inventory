#include "Tests/LootSessionTestTypes.h"

#include "Components/LootPoolComponent.h"

ALootSessionTestActor::ALootSessionTestActor()
{
	bReplicates = true;
	LootPool = CreateDefaultSubobject<ULootPoolComponent>(TEXT("LootPool"));
	LootPool->SetIsReplicated(true);
}

void ALootSessionTestActor::StartLooting(AActor* Looter)
{
	if (bBeingLooted || !Looter)
		return;

	++StartCalls;
	bBeingLooted = true;
}

void ALootSessionTestActor::StopLooting(AActor* Looter)
{
	if (!bBeingLooted)
		return;

	++StopCalls;
	bBeingLooted = false;
}

int32 ALootSessionTestActor::GetItemData(int32 TopLeftID) const
{
	return TopLeftID == TestItemTopLeft ? TestItemID : -1;
}

void ALootSessionTestActor::RemoveItem(int32 TopLeftID)
{
	if (TopLeftID == TestItemTopLeft)
		TestItemID = -1;
}
