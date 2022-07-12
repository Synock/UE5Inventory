// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemWidget.h"
#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "GenericPlatform/GenericPlatformMath.h"
void UEquipmentSlotWidget::InitData()
{
	Refresh();

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	IEquipmentInterface* EquipmentInterface = PC->GetEquipmentForInventory();
	UEquipmentComponent* EquipmentComponent = EquipmentInterface->GetEquipmentComponent();

	check(EquipmentInterface);
	check(EquipmentComponent);
	EquipmentComponent->EquipmentDispatcher.AddDynamic(this, &UEquipmentSlotWidget::Refresh);

	
	EquipmentComponent->EquipmentDispatcher.AddUniqueDynamic(this, &UEquipmentSlotWidget::ResetTransaction);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::UpdateItemImageVisibility()
{
	if (ItemImagePointer)
		ItemImagePointer->SetVisibility(Item.ItemID >= 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::UpdateSlotState()
{
	EnabledSlot = Item.ItemID != 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	if (!CanEquipItem(InputItem->GetReferencedItem()))
		return false;

	const int32 ItemID = InputItem->GetReferencedItem().ItemID;

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return false;
	
	if (!InputItem->IsBelongingToSelf()) //Equip from loot
	{
		PC->PlayerEquipItemFromLoot(ItemID, SlotID, InputItem->GetTopLeftID());
	}
	else
	{
		if (InputItem->IsFromEquipment())
		{
			if (CanEquipItemAtSlot(Item, InputItem->GetOriginalSlot()))
			{
				PC->PlayerSwapEquipment(ItemID, SlotID, Item.ItemID, InputItem->GetOriginalSlot());
			}
			else //We cannot swap equipment
			{
				return false;
			}
		}
		else
		{
			PC->PlayerEquipItemFromInventory(ItemID, SlotID, InputItem->GetTopLeftID(),
			                                                    InputItem->GetBagID());
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::UnEquipBagSpecific()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return false;
	
	if (PC->CanUnequipBag(SlotID))
	{
		const EBagSlot Bag = UInventoryComponent::GetBagSlotFromInventory(SlotID);
		PC->GetInventoryHUDInterface()->Execute_HideBag(PC->GetInventoryHUDObject(), Bag);
		PC->GetInventoryHUDInterface()->Execute_UnequipBag(PC->GetInventoryHUDObject(), Bag);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::OpenBag() const
{
	if (IsBag())
	{
		if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		{
			const EBagSlot Bag = UInventoryComponent::GetBagSlotFromInventory(SlotID);
			PC->GetInventoryHUDInterface()->Execute_ToggleBag(PC->GetInventoryHUDObject(), Bag);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::IsBag() const
{
	return Item.ItemID >= 0 && Item.Bag;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::InnerRefresh()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
	{
		check(PC->GetEquipmentForInventory());
		Item = PC->GetEquipmentForInventory()->GetEquippedItem(SlotID);
		UpdateItemImageVisibility();
		UpdateItemImage();
		UpdateSlotState();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::HideItem()
{
	Item.ItemID = -1;

	if (Item.TwoSlotsItem && SisterSlot)
	{
		SisterSlot->HideItem();
	}

	UpdateItemImageVisibility();
	UpdateItemImage();
	UpdateSlotState();
}

void UEquipmentSlotWidget::ResetTransaction()
{
	if(IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		PC->ResetTransaction();
	
}

IInventoryPlayerInterface* UEquipmentSlotWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::StopDrag()
{
	Super::StopDrag();
	if (SlotID != EEquipmentSlot::Unknown)
	{
		if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
			PC->GetInventoryHUDInterface()->Execute_ForceRefreshInventory(PC->GetInventoryHUDObject());
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::SetSisterSlot(UEquipmentSlotWidget* NewSisterSlot)
{
	SisterSlot = NewSisterSlot;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItem(const FInventoryItem& InputItem) const
{
	if (Item.ItemID >= 0 || !EnabledSlot)
	{
		UE_LOG(LogTemp, Log, TEXT("Slot is currently unvailable"));
		return false;
	}

	if (!CanEquipItemAtSlot(InputItem, SlotID))
		return false;

	//Other check are performed here
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItemAtSlot(const FInventoryItem& InputItem, EEquipmentSlot InputSlot)
{
	if (InputItem.ItemID < 0)
	{
		UE_LOG(LogTemp, Log, TEXT("item is not equippable as it is not a valid item"));
		return false;
	}

	if (!InputItem.Equipable)
	{
		UE_LOG(LogTemp, Log, TEXT("%s cannot be equipped"), *InputItem.Name);
		return false;
	}

	const int32 LocalAcceptableBitMask = FMath::Pow(2., static_cast<double>(InputSlot));

	if (InputItem.TwoSlotsItem)
	{
		if (InputSlot == EEquipmentSlot::WaistBag2 || InputSlot == EEquipmentSlot::BackPack2)
		{
			return false;
		}
	}

	if (InputItem.EquipableSlotBitMask & LocalAcceptableBitMask)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::TryEquipItem(UItemWidget* InputItem)
{
	if (CanEquipItem(InputItem->GetReferencedItem()))
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::UpdateTextSlots()
{
	if(!TextSlot1 || !TextSlot2)
		return;
	
	FString SlotName = UInventoryUtilities::GetSlotName(SlotID);

	if(SlotName.IsEmpty())
	{
		TextSlot1->SetVisibility(ESlateVisibility::Hidden);
		TextSlot2->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	
	FString LeftPart;
	FString RightPart;
	SlotName.Split(" ", &LeftPart, &RightPart, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	if(!LeftPart.IsEmpty() && !RightPart.IsEmpty())
	{
		TextSlot1->SetText(FText::FromString(LeftPart));
		TextSlot2->SetText(FText::FromString(RightPart));
	}
	else
	{
		TextSlot1->SetText(FText::FromString(SlotName));
		TextSlot2->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}
