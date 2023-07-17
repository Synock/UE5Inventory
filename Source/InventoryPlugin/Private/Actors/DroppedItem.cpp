// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/DroppedItem.h"

#include "Items/InventoryItemBase.h"

ADroppedItem::ADroppedItem()
{
	PrimaryActorTick.bCanEverTick = false;
	StaticItem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticItemMesh"));
	StaticItem->SetIsReplicated(true);
}

void ADroppedItem::InitializeFromItem(UInventoryItemBase* Item)
{
	if(Item)
	{
		if(Item->Mesh)
			StaticItem->SetStaticMesh(Item->Mesh);

		if(Item->Mesh->GetBoundingBox().GetSize().Z > Item->Mesh->GetBoundingBox().GetSize().Y)
		{
			StaticItem->SetRelativeRotation(FRotator::MakeFromEuler({0,90,0}));
		}
		//else initialize at some default mesh

		ItemID = Item->ItemID;
	}

}

void ADroppedItem::BeginPlay()
{
	Super::BeginPlay();
}
