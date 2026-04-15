
#include "UI/ItemBaseWidget.h"

#include "InventoryPlugin.h"
#include "InventoryUtilities.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"
#include "TimerManager.h"
#include "Items/InventoryItemEquipable.h"

void UItemBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

//----------------------------------------------------------------------------------------------------------------------

FReply UItemBaseWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsLocked)
	{
		// Locked - suppress interactions; notify owner/UI via game-implemented event
		FText Msg = FText::FromString(TEXT("This item is locked while in use."));
		NotifyInteractionBlocked(Msg);
		return FReply::Handled();
	}

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
	if (bIsLocked)
	{
		// Block all interactions locally to improve UX; notify owner/UI
		// Return Handled() WITHOUT calling Super to prevent drag detection
		FText Msg = FText::FromString(TEXT("This item is locked while in use."));
		NotifyInteractionBlocked(Msg);
		return FReply::Handled();
	}

	ClickEvent = InMouseEvent;

	const FString ButtonName = UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent).ToString();
	UE_LOG(LogInventoryPlugin, Verbose, TEXT("NativeOnMouseButtonDown: Button=%s, Widget=%s, Item=%s"),
	       *ButtonName, *GetName(), Item ? *Item->Name : TEXT("None"));

	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) != FKey("RightMouseButton"))
	{
		LeftClickEffect();

		// Call Super to enable drag detection for unlocked items
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	IsRightClicking = true;
	UE_LOG(LogInventoryPlugin, Verbose, TEXT("Right-click started on widget %s, starting timer..."), *GetName());

	GetWorld()->GetTimerManager().SetTimer(RightClickTimerHandle, this, &UItemBaseWidget::RightClickTimerFunction,
	                                       RightClickMaxDuration,
	                                       false, RightClickMaxDuration);

	return FReply::Handled();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                           UDragDropOperation*& OutOperation)
{
	// Additional safety check: prevent drag for locked items
	if (bIsLocked)
	{
		OutOperation = nullptr;
		return;
	}

	// Call parent implementation to allow Blueprint OnDragDetected to work
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::DisplayDescription(const FPointerEvent& InMouseEvent)
{
	if (!Item)
	{
		return;
	}

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		PC->GetInventoryHUDInterface()->Execute_DisplayItemDescriptionWithDurability(PC->GetInventoryHUDObject(), Item,
			InMouseEvent.GetScreenSpacePosition().X,
			InMouseEvent.GetScreenSpacePosition().Y,
			Durability, MaxDurability);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::DisplayBookText(const FPointerEvent& InMouseEvent)
{
	if (!Item)
		return;

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		float MouseX = 0.f, MouseY = 0.f;
		if (APlayerController* PlayerController = GetOwningPlayer())
			PlayerController->GetMousePosition(MouseX, MouseY);

		PC->GetInventoryHUDInterface()->Execute_DisplayBookText(
			PC->GetInventoryHUDObject(), Item, MouseX, MouseY);
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

	UImage* ImageWidget = ItemImage;
	if (!ImageWidget)
	{
		UE_LOG(LogInventoryPlugin, Warning,
			   TEXT("ItemBaseWidget::UpdateItemImage - No image widget found (ItemImage or ItemImagePointer)"));
		return;
	}

	ImageWidget->SetDesiredSizeOverride(FVector2D(TileSize, TileSize));

	// Use new BindWidget property, fall back to deprecated pointer for backwards compatibility
	if (!Item || !Item->Icon)
	{
		//remove image
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	ImageWidget->SetBrushFromTexture(Item->Icon);
	ItemImage->SetVisibility(ESlateVisibility::Visible);
}


//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::SetLocked(bool bLocked)
{
	if (bIsLocked == bLocked)
		return;
	bIsLocked = bLocked;
	OnLockStateChanged(bIsLocked);
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::RightClickTimerFunction()
{
	if (bIsLocked)
	{
		FText Msg = FText::FromString(TEXT("This item is locked while in use."));
		NotifyInteractionBlocked(Msg);
		return;
	}

	IsRightClicking = false;
	UE_LOG(LogInventoryPlugin, Verbose, TEXT("Right-click timer completed on widget %s, displaying description..."), *GetName());
	//RightClickLongEffect();
	DisplayDescription(ClickEvent);
}

//----------------------------------------------------------------------------------------------------------------------

bool UItemBaseWidget::RightClickShortEffect_Implementation()
{
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::InitBareData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize,
                                   float InputDurability)
{
	Item = InputItem;
	Owner = InputOwner;
	TileSize = InputTileSize;

	SetToolTipText(Item ? FText::FromString(Item->Name) : FText::GetEmpty());
	// Get max durability from item if it's equipable
	if (const UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(InputItem))
	{
		MaxDurability = FMath::Max(1.0f, EquipableItem->GetTotalDurability());
	}
	else
	{
		MaxDurability = 100.0f; // Default for non-equipable items
	}

	Durability = FMath::Clamp(InputDurability, 0.0f, MaxDurability);

	UpdateItemImage();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::StopDrag()
{
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::Refresh_Implementation()
{
	RefreshInternal();
}

void UItemBaseWidget::RefreshInternal()
{
	UpdateItemImage();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::OnLockStateChanged_Implementation(bool bLocked)
{
	if (ItemImage)
	{
		// Apply a darker tint when locked. Use ColorAndOpacity if available.
		FLinearColor TargetColor = bLocked ? FLinearColor(0.25f, 0.25f, 0.25f, 1.0f) : FLinearColor::White;
		ItemImage->SetColorAndOpacity(TargetColor);
	}
}


