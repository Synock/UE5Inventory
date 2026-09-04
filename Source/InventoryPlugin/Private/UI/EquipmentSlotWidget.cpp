#include "UI/EquipmentSlotWidget.h"
#include "InventoryPlugin.h"
#include "UI/ItemWidget.h"
#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/InventoryEquipmentWidget.h"
#include "UI/PendingDeliveryDragDropOperation.h"

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetupUI();
	InitData();
	UpdateTextSlots();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (BackgroundImage)
		BackgroundImage->SetColorAndOpacity(HoverColor);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (BackgroundImage)
		BackgroundImage->SetColorAndOpacity(DefaultColor);
}

//----------------------------------------------------------------------------------------------------------------------

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const bool bLeftClick = InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	IInventoryPlayerInterface* Player = GetInventoryPlayerInterface();
	const bool bCanPresentToMerchant = CanHandleMerchantSaleClick(bLeftClick,
		Player && Player->IsTrading(), EnabledSlot, bIsLocked, Item && Item->ItemID > 0,
		SlotID > EEquipmentSlot::Unknown && SlotID < EEquipmentSlot::Last);

	if (!bCanPresentToMerchant)
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (Cast<IInventoryItemBagInterface>(Item))
	{
		UInventoryComponent* Inventory = Player->GetInventoryComponent();
		if (!Inventory || !Inventory->IsLinkedEquipmentStorageEmptyAndUnreserved(SlotID))
		{
			NotifyInteractionBlocked(FText::FromString(TEXT("Empty this container before selling it.")));
			return FReply::Handled();
		}
	}

	Player->TryPresentEquippedSellItem(SlotID, Item->ItemID);
	return FReply::Handled();
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanHandleMerchantSaleClick(bool bLeftClick, bool bTrading,
	bool bSlotEnabled, bool bSlotLocked, bool bHasValidItem, bool bHasCanonicalSlot)
{
	return bLeftClick && bTrading && bSlotEnabled && !bSlotLocked && bHasValidItem &&
		bHasCanonicalSlot;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	StopDrag();
}

bool UEquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (const UPendingDeliveryDragDropOperation* DeliveryOperation =
		Cast<UPendingDeliveryDragDropOperation>(InOperation))
		return DeliveryOperation->Item && CanEquipItem(DeliveryOperation->Item);
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (const UPendingDeliveryDragDropOperation* DeliveryOperation =
		Cast<UPendingDeliveryDragDropOperation>(InOperation))
	{
		if (!DeliveryOperation->Item || !CanEquipItem(DeliveryOperation->Item))
			return false;
		if (IInventoryPlayerInterface* Player = GetInventoryPlayerInterface())
		{
			if (UInventoryNetComponent* Net = Player->GetInventoryNetComponent())
			{
				Net->Server_ClaimPendingDeliveryAt(DeliveryOperation->DeliveryId,
					FInventoryDeliveryDestination::MakeEquipment(SlotID));
				return true;
			}
		}
		return false;
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::Refresh_Implementation()
{
	// Apply enabled/disabled tint to the item image before the base refresh runs.
	if (ItemImage)
	{
		const FLinearColor Tint = EnabledSlot
			? FLinearColor::White
			: ItemDisabledTint;
		ItemImage->SetBrushTintColor(FSlateColor(Tint));
	}

	InnerRefresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::InitData()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
	{
		UE_LOG(LogInventoryPlugin, Error,
		       TEXT("UEquipmentSlotWidget::InitData — no player interface on '%s'. Slot will not update."),
		       *GetName());
		return;
	}

	IEquipmentInterface* EquipmentInterface = PC->GetEquipmentForInventory();
	if (!EquipmentInterface)
	{
		UE_LOG(LogInventoryPlugin, Error,
		       TEXT("UEquipmentSlotWidget::InitData — no equipment interface on '%s'. Slot will not update."),
		       *GetName());
		return;
	}

	UEquipmentComponent* EquipmentComponent = EquipmentInterface->GetEquipmentComponent();
	if (!EquipmentComponent)
	{
		UE_LOG(LogInventoryPlugin, Error,
		       TEXT("UEquipmentSlotWidget::InitData — no equipment component on '%s'. Slot will not update."),
		       *GetName());
		return;
	}

	// Perform an initial sync before binding so the slot shows the correct state immediately.
	Refresh();

	// Both bindings are unique so hot-reload / re-entrance cannot register duplicates.
	EquipmentComponent->EquipmentDispatcher.AddUniqueDynamic(this, &UEquipmentSlotWidget::Refresh);
	EquipmentComponent->EquipmentDispatcher.AddUniqueDynamic(this, &UEquipmentSlotWidget::ResetTransaction);
}

void UEquipmentSlotWidget::ForEachSecondarySlot(const UInventoryItemEquipable* InItem,
                                               TFunctionRef<void(UEquipmentSlotWidget&)> Func) const
{
	if (!InItem || !InItem->MultiSlotItem || !ParentComponent)
		return;

	for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown);
	     i < static_cast<int32>(EEquipmentSlot::Last); ++i)
	{
		const int32 LocalBit = static_cast<int32>(1u << static_cast<uint32>(i));
		if (!(LocalBit & InItem->EquipableSlotBitMask))
			continue;

		const EEquipmentSlot LocalSlot = static_cast<EEquipmentSlot>(i);
		if (LocalSlot == SlotID)
			continue;

		UEquipmentSlotWidget* OtherSlot = ParentComponent->GetSlotWidget(LocalSlot);
		if (OtherSlot)
			Func(*OtherSlot);
	}
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
	return Item && Cast<IInventoryItemBagInterface>(Item);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::InnerRefresh()
{
	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return;

	IEquipmentInterface* EquipmentInterface = PC->GetEquipmentForInventory();
	if (!EquipmentInterface)
		return;

	UEquipmentComponent* EquipmentComponent = EquipmentInterface->GetEquipmentComponent();

	const UInventoryItemEquipable* Equipment = EquipmentInterface->GetEquippedItem(SlotID);
	Item = Equipment;

	if (Equipment)
	{
		MaxDurability = FMath::Max(1.0f, Equipment->GetTotalDurability());

		float EquipmentDurability = MaxDurability;
		if (EquipmentComponent &&
		    EquipmentComponent->GetEquipmentDurability(SlotID, EquipmentDurability))
		{
			Durability = EquipmentDurability;
		}
		else
		{
			Durability = MaxDurability;
		}

		if (EquipmentComponent)
		{
			const bool bNewLockState = EquipmentComponent->GetEquipmentLockState(SlotID);
			if (bIsLocked != bNewLockState)
			{
				bIsLocked = bNewLockState;
				OnLockStateChanged(bIsLocked);
			}
		}
	}

	UGenericSlotWidget::InnerRefresh();
	UpdateTooltip();

	// Disable all secondary slots occupied by this multi-slot item.
	ForEachSecondarySlot(Equipment, [Equipment](UEquipmentSlotWidget& LocalSlot)
	{
		LocalSlot.DisableAndRefresh(Equipment);
	});
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::HideItem()
{
	// Cache before Super clears Item so ForEachSecondarySlot still has the item reference.
	const UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(Item);

	// Re-enable every secondary slot that was mirroring this multi-slot item.
	ForEachSecondarySlot(EquipableItem, [](UEquipmentSlotWidget& LocalSlot)
	{
		LocalSlot.EnabledSlot = true;
		LocalSlot.SetIsEnabled(true);
		LocalSlot.HideItem();
	});

	// P2: call Super::HideItem() (sets Item = nullptr and refreshes base visuals via virtual chain)
	// instead of hard-coding UGenericSlotWidget::InnerRefresh() directly.
	Super::HideItem();

	UpdateTooltip();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::DisableAndRefresh(const UInventoryItemEquipable* InputItem)
{
	if (!InputItem)
		return;

	SetIsEnabled(false);
	EnabledSlot = false;
	if (InputItem->Icon)
	{
		UTexture2D* Tex = InputItem->Icon;

		if (!ItemImage)
			return;

		ItemImage->SetDesiredSizeOverride({TileSize, TileSize});
		ItemImage->SetBrushFromTexture(Tex);
		ItemImage->SetVisibility(ESlateVisibility::Visible);
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
		UE_LOG(LogInventoryPlugin, Verbose, TEXT("CanEquipItemAtSlot: null item"));
		return false;
	}

	const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(InputItem);
	if (!Equipable)
	{
		UE_LOG(LogInventoryPlugin, Verbose, TEXT("CanEquipItemAtSlot: '%s' is not equippable"), *InputItem->Name);
		return false;
	}


	const int32 LocalAcceptableBitMask = static_cast<int32>(1u << static_cast<uint32>(InputSlot));

	// Multi-slot items occupy their primary slot and all bits in EquipableSlotBitMask simultaneously.
	// The *2 slots (WaistBag2, BackPack2) are secondary positions reserved for the item's
	// visual overflow — they must never be the drop target for a multi-slot item.
	if (Equipable->MultiSlotItem)
	{
		if (InputSlot == EEquipmentSlot::WaistBag2 || InputSlot == EEquipmentSlot::BackPack2)
			return false;
	}

	return (Equipable->EquipableSlotBitMask & LocalAcceptableBitMask) != 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItemWidget(UItemWidget* InputItem) const
{
	if (!InputItem)
		return false;

	return CanEquipItem(InputItem->GetReferencedItem());
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::UpdateTextSlots()
{
	// LowerTextBox = first word, UpperTextBox = second word (preserves original BP assignment order)
	if (!LowerTextBox || !UpperTextBox)
		return;

	const FString SlotName = UInventoryUtilities::GetSlotName(SlotID);

	if (SlotName.IsEmpty())
	{
		LowerTextBox->SetVisibility(ESlateVisibility::Hidden);
		UpperTextBox->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FString LeftPart;
	FString RightPart;
	SlotName.Split(TEXT(" "), &LeftPart, &RightPart, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	if (!LeftPart.IsEmpty() && !RightPart.IsEmpty())
	{
		LowerTextBox->SetText(FText::FromString(LeftPart));
		UpperTextBox->SetText(FText::FromString(RightPart));
	}
	else
	{
		LowerTextBox->SetText(FText::FromString(SlotName));
		UpperTextBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::UpdateTooltip()
{
	if (Item == CachedTooltipItem)
		return;

	CachedTooltipItem = Item;

	if (Item && !Item->Name.IsEmpty())
		SetToolTipText(FText::FromString(Item->Name));
	else
		SetToolTipText(FText::GetEmpty());
}
