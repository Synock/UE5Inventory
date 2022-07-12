// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/ItemWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"
#include "UI/InventoryGridWidget.h"

void UItemWidget::HandleAutoEquip()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;
	
	if(!IsBelongingToSelf())
	{
		PC->PlayerAutoLootItem(Item.ItemID, TopLeftID);
	}
	else
	{
		EEquipmentSlot TargetSlot;
		if(PC->PlayerTryAutoEquip(Item.ItemID,TargetSlot))
		{
			PC->PlayerAutoEquipItem(GetTopLeftID(),GetBagID(),Item.ItemID);
			ParentGrid->UnRegisterItem(this);
			RemoveFromParent();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::HandleSellClick()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;

	if(!PC->IsTrading())
		return;
	
	PC->TryPresentSellItem(ParentGrid->GetBagID(),Item.ItemID, TopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UItemWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::InitData(const FInventoryItem& InputItem, AActor* InputOwner, float InputTileSize, int32 InputTopLeftID,
                           EBagSlot InputBagID, EEquipmentSlot InputOriginalSlotID)
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
	if (OriginalSlotID != EEquipmentSlot::Unknown)
	{
		IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
			PC->GetInventoryHUDInterface()->Execute_ForceRefreshInventory(PC->GetInventoryHUDObject());
	}
	else if (BagID != EBagSlot::Unknown)
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
	return OriginalSlotID != EEquipmentSlot::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

bool UItemWidget::IsBelongingToSelf() const
{
	const bool OwnBagBool = BagID != EBagSlot::LootPool;
	return IsFromEquipment() || OwnBagBool;
}
