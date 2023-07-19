// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/DroppedItem.h"

#include "Items/InventoryItemBase.h"

ADroppedItem::ADroppedItem()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticItem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticItemMesh"));
	StaticItem->SetIsReplicated(true);
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetIsReplicated(true);
	SetRootComponent(StaticItem);

	CapsuleComponent->SetHiddenInGame(false);
	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetupAttachment(StaticItem);

	StaticItem->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

	// Put that how you actually want it
	//CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	//StaticItem->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);


}

void ADroppedItem::InitializeFromItem(UInventoryItemBase* Item)
{
	if (Item)
	{
		if (Item->Mesh)
			StaticItem->SetStaticMesh(Item->Mesh);
		//else initialize at some default mesh

		CapsuleComponent->SetRelativeLocation(Item->Mesh->GetBoundingBox().GetCenter());

		CapsuleComponent->InitCapsuleSize(
			FMath::Max(Item->Mesh->GetBoundingBox().GetSize().X, Item->Mesh->GetBoundingBox().GetSize().Y),
			Item->Mesh->GetBoundingBox().GetSize().Z * 0.5);

		if (Item->Mesh->GetBoundingBox().GetSize().Z > Item->Mesh->GetBoundingBox().GetSize().Y)
		{
			StaticItem->SetRelativeRotation(FRotator::MakeFromEuler({90, 0, 0}));
			//CapsuleComponent->SetRelativeRotation(FRotator::MakeFromEuler({90, 0, 0}));
		}

		ItemID = Item->ItemID;
	}
}

void ADroppedItem::BeginPlay()
{
	Super::BeginPlay();
}
