#include "UI/ItemWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/InventoryGridWidget.h"

void UItemWidget::HandleAutoEquip()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;

	if (!IsBelongingToSelf())
	{
		PC->PlayerAutoLootItem(Item->ItemID, TopLeftID);
	}
	else
	{
		EEquipmentSlot TargetSlot;
		if (PC->PlayerTryAutoEquip(Item->ItemID, TargetSlot))
		{
			PC->PlayerAutoEquipItem(GetTopLeftID(), GetBagID(), Item->ItemID);
			ParentGrid->UnRegisterItem(this);
			RemoveFromParent();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::HandleAutoLoot()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;

	if (!IsBelongingToSelf())
	{
		PC->PlayerAutoLootItem(Item->ItemID, TopLeftID);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::HandleSellClick()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;

	if (!PC->IsTrading())
		return;

	PC->TryPresentSellItem(ParentGrid->GetBagID(), Item->ItemID, TopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::HandleActivation()
{
	if (Item->ItemID <= 0)
		return;

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		PC->HandleActivation(Item->ItemID, TopLeftID, BagID);
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UItemWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::RefreshInternal()
{
	float WidthP, HeightP;
	GetSizeInPixels(WidthP, HeightP);

	BackgroundSizeBox->SetWidthOverride(WidthP);
	BackgroundSizeBox->SetHeightOverride(HeightP);

	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage))
	{
		CanvasSlot->SetSize(FVector2D(WidthP, HeightP));
	}

	UpdateItemImage();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseEnter(MyGeometry, MouseEvent);
	if (BackgroundBorder)
	{
		const FColor HexColor = FColor::FromHex(TEXT("BCBCBC33"));
		BackgroundBorder->SetBrushColor(FLinearColor::FromSRGBColor(HexColor));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent);
	if (BackgroundBorder)
	{
		const FColor HexColor = FColor::FromHex(TEXT("00000080"));
		BackgroundBorder->SetBrushColor(FLinearColor::FromSRGBColor(HexColor));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::InitData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize,
                           int32 InputTopLeftID,
                           EBagSlot InputBagID, EEquipmentSlot InputOriginalSlotID, float InputDurability)
{
	InitBareData(InputItem, InputOwner, InputTileSize, InputDurability);
	TopLeftID = InputTopLeftID;
	BagID = InputBagID;
	OriginalSlotID = InputOriginalSlotID;

	SetToolTipText(FText::FromString(InputItem->Name));
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
	WidthP = TileSize * Item->Width;
	HeightP = TileSize * Item->Height;
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

//----------------------------------------------------------------------------------------------------------------------

void UItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	// When drag is cancelled, restore the item state
	StopDrag();
}
