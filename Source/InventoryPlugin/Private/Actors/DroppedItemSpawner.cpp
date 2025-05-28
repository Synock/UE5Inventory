// Copyright 2025 Maximilien (Synock) Guislain


#include "Actors/DroppedItemSpawner.h"

#include "Actors/DroppedItem.h"
#include "Components/BoxComponent.h"
#include "Interfaces/InventoryGameModeInterface.h"
#include "Items/InventoryItemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameModeBase.h"


// Sets default values
ADroppedItemSpawner::ADroppedItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));

	StaticItemPreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticItemMeshPreview"));
	StaticItemPreview->SetIsReplicated(false);
	StaticItemPreview->SetHiddenInGame(true);
	StaticItemPreview->AttachToComponent(SpawnArea, FAttachmentTransformRules::KeepRelativeTransform);
	RootComponent = SpawnArea;
}

void ADroppedItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnDroppedItem();
}

FTransform ADroppedItemSpawner::GetSpawnPosition()
{
	if (!SpawnArea)
		return FTransform();

	FVector BoxExtent = SpawnArea->GetScaledBoxExtent();
	FVector Origin = SpawnArea->GetComponentLocation();

	FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);
	return FTransform(RandomPoint);
}

void ADroppedItemSpawner::StartRespawnTimer()
{
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ADroppedItemSpawner::SpawnDroppedItem, RespawnTime,
	                                       false);
}

void ADroppedItemSpawner::SpawnDroppedItem()
{
	if (!DroppedItem)
		return;


	IInventoryGameModeInterface* GMI = Cast<IInventoryGameModeInterface>(GetWorld()->GetAuthGameMode());

	if (!GMI)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DroppedItemSpawner: No GameMode implementing IInventoryGameModeInterface found!"));
		return;
	}

	if (!GMI->CanSpawnItem(DroppedItem))
	{
		StartRespawnTimer();
		return;
	}

	if (RandomAreaSpawn)
	{
		FTransform SpawnTransform = GetSpawnPosition();
		FVector NewLocation = GMI->GetItemSpawnLocation(this, SpawnTransform.GetLocation(), true);
		SpawnTransform = FTransform(NewLocation);
		SpawnedItem = GMI->SpawnItemFromActor(this, DroppedItem->ItemID,
		                                      SpawnTransform.GetLocation(), true);
	}
	else
	{
		SpawnedItem = GMI->SpawnItemFromActorRaw(this, DroppedItem);
	}

	if (SpawnedItem)
	{
		SpawnedItem->OnDestroyed.AddDynamic(this, &ADroppedItemSpawner::RegisterItemPickup);
	}
}


void ADroppedItemSpawner::RegisterItemPickup(AActor* DestroyedActor)
{
	if (SpawnedItem != DestroyedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("DroppedItemSpawner: RegisterItemPickup: SpawnedItem != DestroyedActor!"));
	}
	SpawnedItem = nullptr;
	StartRespawnTimer();
}

void ADroppedItemSpawner::UpdateStaticMesh()
{
	if (DroppedItem && StaticItemPreview)
		StaticItemPreview->SetStaticMesh(DroppedItem->Mesh);
}
