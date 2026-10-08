#include "UI/Repair/RepairLineWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"
#include "UI/Repair/RepairWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

//----------------------------------------------------------------------------------------------------------------------

void URepairLineWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind repair button click event
	if (RepairButton)
	{
		RepairButton->OnClicked.AddUniqueDynamic(this, &URepairLineWidget::OnRepairButtonClicked);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairLineWidget::SetParentRepairWidget(URepairWidget* InParentWidget)
{
	ParentRepairWidget = InParentWidget;
}

//----------------------------------------------------------------------------------------------------------------------

void URepairLineWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	URepairLineData* Data = Cast<URepairLineData>(ListItemObject);

	if (Data)
	{
		UpdateDisplay(Data->Data);
		ItemID = Data->Data.ItemID;
		ItemSlot = Data->Data.Slot;
		CurrentDurability = Data->Data.CurrentDurability;
		MaxDurability = Data->Data.MaxDurability;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairLineWidget::UpdateDisplay(const FRepairLineDataStruct& ItemData)
{
	// Update item icon
	if (ItemIcon && ItemData.Icon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.Icon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update item name
	if (ItemName)
	{
		ItemName->SetText(FText::FromString(ItemData.ItemName));
	}

	// Update equipment slot
	if (EquipmentSlot)
	{
		FString SlotDisplayName = GetEquipmentSlotDisplayName(ItemData.Slot);
		EquipmentSlot->SetText(FText::FromString(SlotDisplayName));
	}

	// Update repair cost using CoinDisplayWidget
	if (RepairCost)
	{
		RepairCost->SetCoinValue(ItemData.RepairCost);
	}

	// Update repair button state based on durability
	if (RepairButton)
	{
		// Disable button if item is at full durability
		bool bNeedsRepair = ItemData.CurrentDurability < ItemData.MaxDurability;

		// Check if parent repair widget has repair in progress
		bool bRepairInProgress = false;

		if (ParentRepairWidget)
		{
			bRepairInProgress = ParentRepairWidget->IsRepairInProgress();
		}

		// Enable button only if needs repair AND no repair is in progress
		RepairButton->SetIsEnabled(bNeedsRepair && !bRepairInProgress);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairLineWidget::OnRepairButtonClicked()
{
	// Use the stored parent repair widget reference
	if (ParentRepairWidget)
	{
		RepairButton->SetIsEnabled(false);
		RepairCost->SetCoinValue({});
		ParentRepairWidget->RepairItem(ItemID, ItemSlot);
	}
}

//----------------------------------------------------------------------------------------------------------------------

FString URepairLineWidget::GetEquipmentSlotDisplayName(EEquipmentSlot EquipmentSlotToRetrieve) const
{
	// Convert enum to human-readable string
	switch (EquipmentSlotToRetrieve)
	{
		case EEquipmentSlot::Primary:
			return TEXT("Primary");
		case EEquipmentSlot::Secondary:
			return TEXT("Secondary");
		case EEquipmentSlot::Range:
			return TEXT("Range");
		case EEquipmentSlot::Ammo:
			return TEXT("Ammo");
		case EEquipmentSlot::Head:
			return TEXT("Head");
		case EEquipmentSlot::Face:
			return TEXT("Face");
		case EEquipmentSlot::EarL:
			return TEXT("Left Ear");
		case EEquipmentSlot::EarR:
			return TEXT("Right Ear");
		case EEquipmentSlot::Neck:
			return TEXT("Neck");
		case EEquipmentSlot::Shoulders:
			return TEXT("Shoulders");
		case EEquipmentSlot::Back:
			return TEXT("Back");
		case EEquipmentSlot::Torso:
			return TEXT("Torso");
		case EEquipmentSlot::WristL:
			return TEXT("Left Wrist");
		case EEquipmentSlot::WristR:
			return TEXT("Right Wrist");
		case EEquipmentSlot::Hands:
			return TEXT("Hands");
		case EEquipmentSlot::FingerL:
			return TEXT("Left Finger");
		case EEquipmentSlot::FingerR:
			return TEXT("Right Finger");
		case EEquipmentSlot::Waist:
			return TEXT("Waist");
		case EEquipmentSlot::Legs:
			return TEXT("Legs");
		case EEquipmentSlot::Feet:
			return TEXT("Feet");
		case EEquipmentSlot::Arms:
			return TEXT("Arms");
		case EEquipmentSlot::Unknown:
		default:
			return TEXT("Unknown");
	}
}

//----------------------------------------------------------------------------------------------------------------------

FReply URepairLineWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) == FKey("RightMouseButton"))
	{
		if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		{
			PC->GetInventoryHUDInterface()->DisplayItemDescriptionFromSource(
				UInventoryUtilities::GetItemFromID(ItemID, GetWorld()),
				InMouseEvent.GetScreenSpacePosition().X,
				InMouseEvent.GetScreenSpacePosition().Y, this);
		}
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

//----------------------------------------------------------------------------------------------------------------------

