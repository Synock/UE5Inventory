// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/ItemBaseWidget.h"

#include "InventoryUtilities.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"
#include "TimerManager.h"
#include "Items/InventoryItemEquipable.h"

FReply UItemBaseWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsRightClicking)
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	IsRightClicking = false;
	GetWorld()->GetTimerManager().ClearTimer(RightClickTimerHandle);

	if (RightClickShortEffect())
		return FReply::Handled();

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

//----------------------------------------------------------------------------------------------------------------------

FReply UItemBaseWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ClickEvent = InMouseEvent;

	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) != FKey("RightMouseButton"))
	{
		LeftClickEffect();

		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	IsRightClicking = true;

	GetWorld()->GetTimerManager().SetTimer(RightClickTimerHandle, this, &UItemBaseWidget::RightClickTimerFunction,
	                                       RightClickMaxDuration,
	                                       false, RightClickMaxDuration);

	return FReply::Handled();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::DisplayDescription(const FPointerEvent& InMouseEvent)
{
	if (!Item)
		return;

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		PC->GetInventoryHUDInterface()->Execute_DisplayItemDescription(PC->GetInventoryHUDObject(), Item,
		                                                               InMouseEvent.GetScreenSpacePosition().X,
		                                                               InMouseEvent.GetScreenSpacePosition().Y);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::DisplayBookText(const FPointerEvent& InMouseEvent)
{
	if (!Item)
		return;

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		PC->GetInventoryHUDInterface()->Execute_DisplayBookText(PC->GetInventoryHUDObject(), Item,
		                                                        InMouseEvent.GetScreenSpacePosition().X,
		                                                        InMouseEvent.GetScreenSpacePosition().Y);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::ResetSell()
{
	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		if (PC->IsTrading())
			PC->ResetSellItem();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::UpdateItemImage()
{
	if (Item && Item->Icon)
	{
		UTexture2D* Tex = Item->Icon;

		if(!ItemImagePointer)
			return;

		ItemImagePointer->SetDesiredSizeOverride({TileSize, TileSize});
		ItemImagePointer->SetBrushFromTexture(Tex);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::RightClickTimerFunction()
{
	IsRightClicking = false;
	//RightClickLongEffect();
	DisplayDescription(ClickEvent);
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::InitBareData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize)
{
	Item = InputItem;
	Owner = InputOwner;
	TileSize = InputTileSize;
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::StopDrag()
{
}
