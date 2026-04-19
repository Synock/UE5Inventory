#include "UI/Repair/RepairWidget.h"
#include "UI/Repair/RepairLineWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/EquipmentInterface.h"
#include "Components/EquipmentComponent.h"
#include "Items/InventoryItemEquipable.h"
#include "Components/ListView.h"
#include "Components/Button.h"

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	if (RepairAllButton)
	{
		RepairAllButton->OnClicked.AddDynamic(this, &URepairWidget::OnRepairAllButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &URepairWidget::OnCloseButtonClicked);
	}

	// Bind list view entry initialization to set parent widget reference
	if (RepairItemList)
	{
		RepairItemList->OnEntryWidgetGenerated().AddUObject(this, &URepairWidget::OnRepairLineWidgetGenerated);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnRepairLineWidgetGenerated(UUserWidget& GeneratedWidget)
{
	// Set the parent repair widget reference on newly generated line widgets
	if (URepairLineWidget* LineWidget = Cast<URepairLineWidget>(&GeneratedWidget))
	{
		LineWidget->SetParentRepairWidget(this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::BuildRepairableItemList()
{
	RepairableItems.Empty();
	TotalRepairCost = FCoinValue{0, 0, 0, 0};

	IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!Player || !RepairerActor)
	{
		return;
	}

	// Get player's equipment component through the interface
	const UEquipmentComponent* EquipmentComp = Cast<UEquipmentComponent>(
		Player->GetInventoryOwningActor()->GetComponentByClass(UEquipmentComponent::StaticClass()));

	if (!EquipmentComp)
	{
		return;
	}

	// Get all equipped items
	const TArray<const UInventoryItemEquipable*>& AllEquipment = EquipmentComp->GetAllEquipment();

	// Iterate through all equipment slots
	for (int32 i = 0; i < AllEquipment.Num(); ++i)
	{
		if (const UInventoryItemEquipable* Item = AllEquipment[i])
		{
			const EEquipmentSlot EquipmentSlot = static_cast<EEquipmentSlot>(i);
			float CurrentDurability = 0.0f;

			// Get current durability for this slot
			if (EquipmentComp->GetEquipmentDurability(EquipmentSlot, CurrentDurability))
			{
				const float MaxDurability = Item->GetTotalDurability();
				const bool bNeedsRepair = CurrentDurability < MaxDurability;

				// Calculate repair cost
				FCoinValue RepairCost = RepairerActor->CalculateRepairCost(
					Item->ItemID, CurrentDurability, MaxDurability);

				// Add to total if needs repair
				if (bNeedsRepair)
				{
					TotalRepairCost += RepairCost;
				}

				// Create repair item data
				FRepairItemData ItemData;
				ItemData.ItemID = Item->ItemID;
				ItemData.Icon = Item->Icon;
				ItemData.ItemName = Item->Name;
				ItemData.Slot = EquipmentSlot;
				ItemData.CurrentDurability = CurrentDurability;
				ItemData.MaxDurability = MaxDurability;
				ItemData.RepairCost = RepairCost;
				ItemData.bNeedsRepair = bNeedsRepair;

				RepairableItems.Add(ItemData);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::PopulateRepairItemList()
{
	if (!RepairItemList)
	{
		return;
	}

	// Clear existing items in the list view
	RepairItemList->ClearListItems();

	// Convert FRepairItemData to URepairLineData objects and add to ListView
	for (const FRepairItemData& ItemData : RepairableItems)
	{
		URepairLineData* LineData = NewObject<URepairLineData>(this);
		if (LineData)
		{
			// Convert FRepairItemData to FRepairLineDataStruct
			LineData->Data.ItemID = ItemData.ItemID;
			LineData->Data.Icon = ItemData.Icon;
			LineData->Data.ItemName = ItemData.ItemName;
			LineData->Data.Slot = ItemData.Slot;
			LineData->Data.RepairCost = ItemData.RepairCost;
			LineData->Data.CurrentDurability = ItemData.CurrentDurability;
			LineData->Data.MaxDurability = ItemData.MaxDurability;

			// Add to list view
			RepairItemList->AddItem(LineData);
		}
	}

	// Request regeneration of all entries
	RepairItemList->RequestRefresh();
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::UpdateRepairAllCostDisplay()
{
	if (RepairAllCost)
	{
		RepairAllCost->SetCoinValue(TotalRepairCost);
	}

	// Check if transaction is in progress
	const bool bTransactionInProgress = IsRepairInProgress();

	// Update Repair All button
	if (RepairAllButton)
	{
		// Check if there are items to repair
		const bool bHasItemsToRepair = RepairableItems.ContainsByPredicate([](const FRepairItemData& Item)
		{
			return Item.bNeedsRepair;
		});

		// Check if player can afford repair all
		const bool bCanAffordRepairAll = CanAffordRepairAll();

		// Enable button only if: has items to repair, can afford, and no repair in progress
		const bool bShouldEnable = bHasItemsToRepair && bCanAffordRepairAll && !bTransactionInProgress;
		RepairAllButton->SetIsEnabled(bShouldEnable);

	}

	// Disable close button during transactions
	if (CloseButton)
	{
		CloseButton->SetIsEnabled(!bTransactionInProgress);
	}

	// Disable repair item list during transactions
	if (RepairItemList)
	{
		RepairItemList->SetIsEnabled(!bTransactionInProgress);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnRepairAllButtonClicked()
{
	IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!Player)
	{
		return;
	}

	// Check if already processing a transaction
	if (Player->GetTransactionBoolean())
	{
		return;
	}

	// Check if player can afford repair all
	if (!Player->PlayerCanPayAmount(TotalRepairCost))
	{
		OnNotEnoughMoneyDelegate.Broadcast();
		return;
	}

	// Set transaction boolean to disable buttons
	Player->SetTransactionBoolean(true);

	// Update UI to reflect disabled state
	UpdateRepairAllCostDisplay();

	// Call player interface to repair all equipment
	Player->PlayerRepairAllEquipment(TotalRepairCost);

	OnRepairSuccessfulDelegate.Broadcast();

	// Note: Refresh will be called from OnRepairTransactionComplete when server responds
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnCloseButtonClicked()
{
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		Player->StopRepairTrade();
	}

	DeInitRepairData();
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::RepairItem(int32 ItemID, EEquipmentSlot EquipmentSlot)
{
	IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!Player || ItemID <= 0)
	{
		return;
	}

	// Check if already processing a transaction
	if (Player->GetTransactionBoolean())
	{
		return;
	}

	// Find the item in the repairable items list
	const FRepairItemData* ItemToRepair = RepairableItems.FindByPredicate([ItemID, EquipmentSlot](const FRepairItemData& Item)
	{
		return Item.ItemID == ItemID && Item.Slot == EquipmentSlot;
	});

	if (!ItemToRepair || !ItemToRepair->bNeedsRepair)
	{
		return;
	}

	// Check if player can afford repair
	if (!Player->PlayerCanPayAmount(ItemToRepair->RepairCost))
	{
		OnNotEnoughMoneyDelegate.Broadcast();
		return;
	}

	// Set transaction boolean to disable buttons
	Player->SetTransactionBoolean(true);

	// Update UI to reflect disabled state
	UpdateRepairAllCostDisplay();

	// Call player interface to repair specific item
	Player->PlayerRepairEquipment(EquipmentSlot, ItemToRepair->RepairCost);

	OnRepairSuccessfulDelegate.Broadcast();

	// Note: Refresh will be called from OnRepairTransactionComplete when server responds
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::InitRepairData(AActor* InputRepairerActor)
{
	if (!RepairerActor && InputRepairerActor && InputRepairerActor->Implements<URepairInterface>())
	{
		RepairerActor.SetObject(InputRepairerActor);
		RepairerActor.SetInterface(Cast<IRepairInterface>(InputRepairerActor));

		if (RepairerName)
		{
			RepairerName->SetText(FText::FromString(RepairerActor->GetRepairerName()));
		}

		Refresh();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::DeInitRepairData()
{
	RepairerActor = nullptr;
	RepairableItems.Empty();
	TotalRepairCost = FCoinValue{0, 0, 0, 0};

	if (RepairItemList)
	{
		RepairItemList->ClearListItems();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::Refresh()
{
	BuildRepairableItemList();
	PopulateRepairItemList();
	UpdateRepairAllCostDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

bool URepairWidget::CanAffordRepairAll() const
{
	IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	return Player && Player->PlayerCanPayAmount(TotalRepairCost);
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnRepairTransactionComplete()
{
	// Reset transaction state to re-enable buttons
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		Player->ResetTransaction();
	}

	// Bind to equipment dispatcher to refresh when durability replication completes
	// This is more robust than a timer, especially with network latency
	if (AActor* OwningActor = GetOwningPlayerPawn())
	{
		if (IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(OwningActor))
		{
			if (UEquipmentComponent* EquipmentComp = EquipmentInterface->GetEquipmentComponent())
			{
				// Bind one-shot delegate to refresh when durability changes are replicated
				FScriptDelegate RefreshDelegate;
				RefreshDelegate.BindUFunction(this, FName("OnEquipmentDurabilityReplicated"));
				EquipmentComp->EquipmentDispatcher.Add(RefreshDelegate);
				return;
			}
		}
	}

	// Fallback if we couldn't bind to dispatcher
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnEquipmentDurabilityReplicated()
{
	// Unbind from dispatcher (one-shot behavior)
	if (AActor* OwningActor = GetOwningPlayerPawn())
	{
		if (IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(OwningActor))
		{
			if (UEquipmentComponent* EquipmentComp = EquipmentInterface->GetEquipmentComponent())
			{
				EquipmentComp->EquipmentDispatcher.RemoveAll(this);
			}
		}
	}

	// Now refresh with updated durability values
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

bool URepairWidget::IsRepairInProgress() const
{
	IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	return Player && Player->GetTransactionBoolean();
}

//----------------------------------------------------------------------------------------------------------------------
// IInventoryRepairWindowInterface
//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::InitRepairWindow_Implementation(AActor* NewRepairerActor)
{
	InitRepairData(NewRepairerActor);
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::DeInitRepairWindow_Implementation()
{
	DeInitRepairData();
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::ShowRepairWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::HideRepairWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::RefreshRepairWindow_Implementation()
{
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void URepairWidget::OnRepairWindowTransactionComplete_Implementation()
{
	OnRepairTransactionComplete();
}

//----------------------------------------------------------------------------------------------------------------------
