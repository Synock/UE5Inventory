
#include "Actors/DroppedItem.h"

#include "Components/CapsuleComponent.h"
#include "Items/InventoryItemBase.h"
#include "Containers/List.h"

ADroppedItem::ADroppedItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADroppedItem::InitializeFromItem(UInventoryItemBase* Item, bool AllowToRotate, const FRotator& SpawningActorRotation)
{
	if (Item)
	{
		if (Item->Mesh)
		{
			StaticItem->SetStaticMesh(Item->Mesh);

			if (Item->OverrideMaterial.OverrideMaterial)
			{
				StaticItem->SetMaterial(Item->OverrideMaterial.MaterialID, Item->OverrideMaterial.OverrideMaterial);
				Item->OverrideMaterial.ApplyTint(StaticItem->CreateAndSetMaterialInstanceDynamic(Item->OverrideMaterial.MaterialID));
			}
		}
		//else initialize at some default mesh

		CapsuleComponent->SetRelativeLocation(Item->Mesh->GetBoundingBox().GetCenter());

		CapsuleComponent->InitCapsuleSize(
			FMath::Max(Item->Mesh->GetBoundingBox().GetSize().X, Item->Mesh->GetBoundingBox().GetSize().Y),
			Item->Mesh->GetBoundingBox().GetSize().Z * 0.5);

		if (AllowToRotate && Item->Mesh)
		{
			// Calculate optimal rotation based on bounding box dimensions
			const FVector BBoxSize = Item->Mesh->GetBoundingBox().GetSize();

			// Create sorted array of dimensions with axis info
			struct FAxisDimension {
				float Size;
				int32 AxisIndex; // 0=X, 1=Y, 2=Z
			};

			TArray<FAxisDimension> Dimensions;
			Dimensions.Add({(float)BBoxSize.X, 0});
			Dimensions.Add({(float)BBoxSize.Y, 1});
			Dimensions.Add({(float)BBoxSize.Z, 2});

			// Sort dimensions in ascending order (smallest to largest)
			Dimensions.Sort([](const FAxisDimension& A, const FAxisDimension& B) {
				return A.Size < B.Size;
			});

			// Dimensions[0] = shortest, Dimensions[1] = second-longest, Dimensions[2] = longest
			const int32 ShortestAxis = Dimensions[0].AxisIndex;
			const int32 LongestAxis = Dimensions[2].AxisIndex;

			// Get character's forward direction (yaw)
			float CharacterYaw = SpawningActorRotation.Yaw;

			FRotator ItemRotation = FRotator::ZeroRotator;

			// Orientation strategy:
			// - Shortest axis should point vertically (Z axis)
			// - Longest axis should point in the character's forward direction
			// - This makes items lie flat with their major axis aligned to character heading

			if (ShortestAxis == 0) // X is shortest (item is thin along X)
			{
				// Rotate 90 around Y axis: X->Z, Y->Y, Z->X
				// After this, if longest was Z: Z becomes X (forward axis)
				//                if longest was Y: Y stays Y (side axis)
				if (LongestAxis == 2) // Z was longest, now points along X (forward/backward)
				{
					ItemRotation = FRotator::MakeFromEuler({0.f, 90.f, 0.f}); // Pitch=90
					ItemRotation.Yaw = CharacterYaw + 180.f;
				}
				else // Y was longest, stays on Y (left/right)
				{
					ItemRotation = FRotator::MakeFromEuler({0.f, 90.f, 0.f}); // Pitch=90
					ItemRotation.Yaw = CharacterYaw + 90.f + 180.f; // Rotate 90 to align Y with forward
				}
			}
			else if (ShortestAxis == 1) // Y is shortest (item is thin along Y)
			{
				// Rotate 90 around X axis: Y->Z, X->X, Z->Y
				// After this, if longest was Z: Z becomes Y (side axis)
				//                if longest was X: X stays X (forward axis)
				if (LongestAxis == 2) // Z was longest, now points along Y (left/right)
				{
					ItemRotation = FRotator::MakeFromEuler({90.f, 0.f, 0.f}); // Roll=90
					ItemRotation.Yaw = CharacterYaw + 90.f + 180.f; // Rotate 90 to align Y with forward
				}
				else // X was longest, stays on X (forward/backward)
				{
					ItemRotation = FRotator::MakeFromEuler({90.f, 0.f, 0.f}); // Roll=90
					ItemRotation.Yaw = CharacterYaw + 180.f;
				}
			}
			else // Z is shortest (already vertical) - longest is X or Y
			{
				if (LongestAxis == 0) // X is longest (forward/backward axis)
				{
					ItemRotation.Yaw = CharacterYaw + 180.f;
				}
				else // Y is longest (left/right axis)
				{
					ItemRotation.Yaw = CharacterYaw + 90.f + 180.f;
				}
			}

			StaticItem->SetRelativeRotation(ItemRotation);
		}

		ItemID = Item->ItemID;
		Durability = 100.0f;  // Default durability
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ADroppedItem::InitializeFromItemWithDurability(UInventoryItemBase* Item, float InDurability, bool AllowToRotate, const FRotator& SpawningActorRotation)
{
	InitializeFromItem(Item, AllowToRotate, SpawningActorRotation);
	Durability = InDurability;
}

//----------------------------------------------------------------------------------------------------------------------

void ADroppedItem::BeginPlay()
{
	Super::BeginPlay();
}


