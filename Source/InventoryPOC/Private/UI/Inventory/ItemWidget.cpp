// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Inventory/ItemWidget.h"
#include "UI/Inventory/InventoryGridWidget.h"

void UItemWidget::InitData(const FBareItem& InputItem, AActor* InputOwner, float InputTileSize, int32 InputTopLeftID,
                           BagSlot InputBagID, InventorySlot InputOriginalSlotID)
{
	InitBareData(InputItem, InputOwner, InputTileSize);
	TopLeftID = InputTopLeftID;
	BagID = InputBagID;
	OriginalSlotID = InputOriginalSlotID;

	SetToolTipText(FText::FromString(InputItem.Name));
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::StopDrag()
{
	Super::StopDrag();

	UE_LOG(LogTemp, Log, TEXT("Drag and dropping was interrupted"));

	//item was originally equipped
	if (OriginalSlotID != InventorySlot::Unknown)
	{
	}
	else if (BagID != BagSlot::Unknown)
	{
		UE_LOG(LogTemp, Log, TEXT("Doing nothing"));
		ParentGrid->Refresh();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Impossible to stop dragging an item"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::GetSizeInPixels(float& WidthP, float& HeightP)
{
	WidthP = TileSize * Item.Width;
	HeightP = TileSize * Item.Height;
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::SetParentGrid(UInventoryGridWidget* InputParentGrid)
{
	ParentGrid = InputParentGrid;
}

//----------------------------------------------------------------------------------------------------------------------

bool UItemWidget::IsFromEquipment() const
{
	return OriginalSlotID != InventorySlot::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

bool UItemWidget::IsBelongingToSelf() const
{
	const bool OwnBagBool = BagID != BagSlot::LootPool;

	return IsFromEquipment() || OwnBagBool;
}
