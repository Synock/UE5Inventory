// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemWidget.h"
#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Items/InventoryItemBag.h"
#include "Items/InventoryItemBase.h"
#include "UI/InventoryEquipmentWidget.h"

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

bool UEquipmentSlotWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	if (!InputItem->GetReferencedItem())
		return false;

	//Can't swap itself
	if (InputItem->GetOriginalSlot() == SlotID)
		return false;

	if (!CanEquipItem(InputItem->GetReferencedItem()))
		return false;

	const int32 ItemID = InputItem->GetReferencedItem()->ItemID;

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
			if (!Item)
			{
				PC->PlayerSwapEquipment(ItemID, SlotID, 0, InputItem->GetOriginalSlot());
			}
			else if (CanEquipItemAtSlot(Item, InputItem->GetOriginalSlot()))
			{
				PC->PlayerSwapEquipment(ItemID, SlotID, Item->ItemID, InputItem->GetOriginalSlot());
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
	return Item && Cast<UInventoryItemBag>(Item);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::InnerRefresh()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
	{
		const IEquipmentInterface* EquipmentInterface = PC->GetEquipmentForInventory();
		check(EquipmentInterface);
		const UInventoryItemEquipable* Equipment = EquipmentInterface->GetEquippedItem(SlotID);
		Item = Equipment;
		UGenericSlotWidget::InnerRefresh();

		if (Equipment && Equipment->MultiSlotItem && ParentComponent)
		{
			for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown); i < static_cast<int32>(EEquipmentSlot::Last); ++
			     i)
			{
				if (const int32 LocalValue = 1 << i; LocalValue & Equipment->EquipableSlotBitMask)
				{
					EEquipmentSlot localSlot = static_cast<EEquipmentSlot>(i);
					if (localSlot != SlotID)
					{
						UEquipmentSlotWidget* OtherSlot = ParentComponent->GetSlotWidget(localSlot);

						if (!OtherSlot)
							continue;

						OtherSlot->DisableAndRefresh(Equipment);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::HideItem()
{
	if (const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item); Equipable && Equipable->
		MultiSlotItem && ParentComponent)
	{
		for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown); i < static_cast<int32>(EEquipmentSlot::Last); ++i)
		{
			const int32 localValue = 1 << i;
			if (localValue & Equipable->EquipableSlotBitMask)
			{
				EEquipmentSlot localSlot = static_cast<EEquipmentSlot>(i);
				if (localSlot != SlotID)
				{
					UEquipmentSlotWidget* OtherSlot = ParentComponent->GetSlotWidget(localSlot);

					if (!OtherSlot)
						continue;

					OtherSlot->EnabledSlot = true;
					OtherSlot->HideItem();
					OtherSlot->SetIsEnabled(true);
				}
			}
		}
	}

	Item = nullptr;

	UGenericSlotWidget::InnerRefresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::DisableAndRefresh(const UInventoryItemEquipable* InputItem)
{
	SetIsEnabled(false);
	EnabledSlot = false;
	if (InputItem->Icon)
	{
		UTexture2D* Tex = InputItem->Icon;

		if (!ItemImagePointer)
			return;

		ItemImagePointer->SetDesiredSizeOverride({TileSize, TileSize});
		ItemImagePointer->SetBrushFromTexture(Tex);
		ItemImagePointer->SetVisibility(ESlateVisibility::Visible);
	}
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

bool UEquipmentSlotWidget::CanEquipItem(const UInventoryItemBase* InputItem) const
{
	if (!CanDropItem(InputItem))
		return false;

	if (!CanEquipItemAtSlot(InputItem, SlotID))
		return false;

	//Other check are performed here
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItemAtSlot(const UInventoryItemBase* InputItem, EEquipmentSlot InputSlot)
{
	if (!InputItem)
	{
		UE_LOG(LogTemp, Log, TEXT("item is not equippable as it is not a valid item"));
		return false;
	}

	const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(InputItem);
	if (!Equipable)
	{
		UE_LOG(LogTemp, Log, TEXT("%s cannot be equipped"), *(InputItem->Name));
		return false;
	}

	const int32 LocalAcceptableBitMask = 1 << static_cast<uint32>(InputSlot);

	if (Equipable->MultiSlotItem)
	{
		if (InputSlot == EEquipmentSlot::WaistBag2 || InputSlot == EEquipmentSlot::BackPack2)
		{
			return false;
		}
	}

	if (Equipable->EquipableSlotBitMask & LocalAcceptableBitMask)
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
	if (!TextSlot1 || !TextSlot2)
		return;

	FString SlotName = UInventoryUtilities::GetSlotName(SlotID);

	if (SlotName.IsEmpty())
	{
		TextSlot1->SetVisibility(ESlateVisibility::Hidden);
		TextSlot2->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FString LeftPart;
	FString RightPart;
	SlotName.Split(" ", &LeftPart, &RightPart, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	if (!LeftPart.IsEmpty() && !RightPart.IsEmpty())
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
