// Copyright 2023 Maximilien (Synock) Guislain


#include "Actors/DroppedItem.h"

#include "Components/CapsuleComponent.h"
#include "Items/InventoryItemBase.h"

ADroppedItem::ADroppedItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADroppedItem::InitializeFromItem(UInventoryItemBase* Item)
{
	if (Item)
	{
		if (Item->Mesh)
		{
			StaticItem->SetStaticMesh(Item->Mesh);

			if (Item->OverrideMaterial.OverrideMaterial)
				StaticItem->SetMaterial(Item->OverrideMaterial.MaterialID, Item->OverrideMaterial.OverrideMaterial);
		}
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
