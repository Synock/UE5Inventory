// Copyright 2022 Maximilien (Synock) Guislain


#include "Actors/LootableActor.h"
#include "InventoryPOC/InventoryPOCCharacter.h"

#include <Net/UnrealNetwork.h>

// Sets default values
ALootableActor::ALootableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	if (HasAuthority())
	{
		LootPool = CreateDefaultSubobject<ULootPoolComponent>("LootPool");
		LootPool->SetNetAddressable(); // Make DSO components net addressable
		LootPool->SetIsReplicated(true); // Enable replication by default

		CashContent_loot = CreateDefaultSubobject<UPurseComponent>("CashContent_loot");
		CashContent_loot->SetNetAddressable(); // Make DSO components net addressable
		CashContent_loot->SetIsReplicated(true); // Enable replication by default
	}
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts or when spawned
void ALootableActor::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::InitLootPool(const TArray<int64>& LootableItems)
{
	if (HasAuthority())
	{
		LootPool->Init(LootableItems);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::InitCashPool(const FCoinValue& CoinValue)
{
	if (HasAuthority())
	{
		CashContent_loot->AddCoins(CoinValue);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::StartLooting(AInventoryPOCCharacter* Looter)
{
	IsBeingLooted = true;
	Looter->ReceiveCoinAmount(CashContent_loot->GetPurseContent());
	CashContent_loot->ClearPurse();
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::StopLooting(AInventoryPOCCharacter* Looter)
{
	IsBeingLooted = false;
}

//----------------------------------------------------------------------------------------------------------------------

ULootPoolComponent* ALootableActor::GetLootPool()
{
	return LootPool;
}

//----------------------------------------------------------------------------------------------------------------------

UPurseComponent* ALootableActor::GetPurse()
{
	return CashContent_loot;
}

//----------------------------------------------------------------------------------------------------------------------

int64 ALootableActor::GetItemData(int32 TopLeftID) const
{
	return LootPool->GetItemAtIndex(TopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::RemoveItem(int32 TopLeftID) const
{
	if (HasAuthority())
	{
		LootPool->RemoveItem(TopLeftID);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ALootableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(ALootableActor, CashContent_loot);
	DOREPLIFETIME(ALootableActor, LootPool);
	DOREPLIFETIME(ALootableActor, IsBeingLooted);
}
