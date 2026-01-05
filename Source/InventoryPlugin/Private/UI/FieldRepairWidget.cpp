#include "UI/FieldRepairWidget.h"
#include "Items/Interfaces/InventoryItemFieldRepairInterface.h"
#include "Items/InventoryItemEquipable.h"
#include "UI/FieldRepairSlotWidget.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UFieldRepairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callback
	if (RepairButton)
	{
		RepairButton->OnClicked.AddDynamic(this, &UFieldRepairWidget::OnRepairButtonClicked);
	}

	// Initialize UI
	UpdateUI();
}

void UFieldRepairWidget::NativeDestruct()
{
	// Clean up timer if widget is destroyed during repair
	if (IsRepairing)
	{
		CancelRepair();
	}

	Super::NativeDestruct();
}

void UFieldRepairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update repair progress display if repairing
	if (IsRepairing)
	{
		UpdateRepairProgressBar();
	}
}

void UFieldRepairWidget::InitializeWithRepairItem(TScriptInterface<IInventoryItemFieldRepairInterface> RepairItem, float RepairKitDurability)
{
	CurrentRepairItem = RepairItem;
	CurrentRepairKitDurability = RepairKitDurability;

	if (RepairItem.GetObject())
	{
		// Use current durability as max for now (could be refined)
		MaxRepairKitDurability = FMath::Max(1.0f, RepairKitDurability);
	}

	UpdateRepairKitBackground();
	UpdateRepairKitChargesBar();
	UpdateStatusText(FText::FromString("Select an item to repair"));

	ValidateState();
}

void UFieldRepairWidget::SetTargetItem(const UInventoryItemEquipable* TargetItem, float TargetDurability)
{
	// Cancel any ongoing repair
	if (IsRepairing)
	{
		CancelRepair();
	}

	CurrentTargetItem = TargetItem;
	CurrentTargetDurability = TargetDurability;

	if (TargetItem)
	{
		MaxTargetDurability = FMath::Max(1.0f, TargetItem->TotalDurability);
	}
	else
	{
		MaxTargetDurability = 100.0f;
	}

	UpdateUI();
	ValidateState();

	OnTargetItemChanged();
}

void UFieldRepairWidget::ClearTargetItem()
{
	SetTargetItem(nullptr, 0.0f);
}

void UFieldRepairWidget::UpdateUI()
{
	UpdateItemDurabilityBar();
	UpdateRepairKitChargesBar();
	UpdateRepairProgressBar();
	UpdateRepairKitBackground();
}

void UFieldRepairWidget::UpdateItemDurabilityBar()
{
	if (!ItemDurabilityBar)
		return;

	if (CurrentTargetItem && MaxTargetDurability > 0.0f)
	{
		float Percent = CurrentTargetDurability / MaxTargetDurability;
		ItemDurabilityBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		ItemDurabilityBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemDurabilityBar->SetPercent(0.0f);
		ItemDurabilityBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFieldRepairWidget::UpdateRepairKitChargesBar()
{
	if (!RepairKitChargesBar)
		return;

	if (CurrentRepairItem.GetObject() && MaxRepairKitDurability > 0.0f)
	{
		float Percent = CurrentRepairKitDurability / MaxRepairKitDurability;
		RepairKitChargesBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		RepairKitChargesBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RepairKitChargesBar->SetPercent(0.0f);
		RepairKitChargesBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFieldRepairWidget::UpdateRepairProgressBar()
{
	if (!RepairProgressBar)
		return;

	if (IsRepairing)
	{
		RepairProgressBar->SetPercent(FMath::Clamp(RepairProgress, 0.0f, 1.0f));
		RepairProgressBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RepairProgressBar->SetPercent(0.0f);
		RepairProgressBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFieldRepairWidget::UpdateStatusText(const FText& Message)
{
	if (StatusText)
	{
		StatusText->SetText(Message);
	}
}

void UFieldRepairWidget::UpdateRepairKitBackground()
{
	if (!RepairKitBackground)
		return;

	if (UObject* RepairObject = CurrentRepairItem.GetObject())
	{
		// Cast to InventoryItemBase to get the icon
		if (const UInventoryItemBase* ItemBase = Cast<UInventoryItemBase>(RepairObject))
		{
			if (ItemBase->Icon)
			{
				RepairKitBackground->SetBrushFromTexture(ItemBase->Icon);
				RepairKitBackground->SetVisibility(ESlateVisibility::Visible);
				return;
			}
		}
	}

	RepairKitBackground->SetVisibility(ESlateVisibility::Hidden);
}


void UFieldRepairWidget::OnRepairButtonClicked()
{
	if (IsRepairing)
	{
		// Cancel repair if already in progress
		CancelRepair();
	}
	else
	{
		// Start repair
		StartRepair();
	}
}

bool UFieldRepairWidget::CanStartRepair(FText& OutReason) const
{
	// Check repair item
	IInventoryItemFieldRepairInterface* RepairInterface = Cast<IInventoryItemFieldRepairInterface>(CurrentRepairItem.GetObject());
	if (!RepairInterface)
	{
		OutReason = FText::FromString("No repair kit selected");
		return false;
	}

	// Check target item
	if (!CurrentTargetItem)
	{
		OutReason = FText::FromString("No item selected for repair");
		return false;
	}

	// Use repair item's validation via interface
	return RepairInterface->CanRepairItem(CurrentTargetItem, CurrentTargetDurability,
		MaxTargetDurability, CurrentRepairKitDurability, OutReason);
}

void UFieldRepairWidget::StartRepair()
{
	FText Reason;
	if (!CanStartRepair(Reason))
	{
		UpdateStatusText(Reason);
		return;
	}

	IInventoryItemFieldRepairInterface* RepairInterface = Cast<IInventoryItemFieldRepairInterface>(CurrentRepairItem.GetObject());
	if (!RepairInterface)
		return;

	// Calculate repair duration and amount via interface
	TotalRepairDuration = RepairInterface->GetRepairDuration();
	PlannedRepairAmount = RepairInterface->CalculateRepairAmount(CurrentTargetDurability, MaxTargetDurability);

	// Initialize repair state
	IsRepairing = true;
	ElapsedRepairTime = 0.0f;
	RepairProgress = 0.0f;

	// Disable repair button (will show "Cancel" via Blueprint)
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(RepairInterface->IsInterruptible());
	}

	// Start timer for repair ticks
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RepairTimerHandle, this, &UFieldRepairWidget::TickRepair,
			0.1f, true);
	}

	// Play start sound via interface
	if (USoundBase* StartSound = RepairInterface->GetRepairStartSound())
	{
		UGameplayStatics::PlaySound2D(this, StartSound);
	}

	UpdateStatusText(FText::FromString(FString::Printf(
		TEXT("Repairing... (%.1f seconds)"), TotalRepairDuration)));
	UpdateUI();

	OnRepairStarted();
}

void UFieldRepairWidget::TickRepair()
{
	if (!IsRepairing)
		return;

	// Update elapsed time
	ElapsedRepairTime += 0.1f;
	RepairProgress = TotalRepairDuration > 0.0f ? (ElapsedRepairTime / TotalRepairDuration) : 1.0f;

	// Check if repair is complete
	if (ElapsedRepairTime >= TotalRepairDuration)
	{
		CompleteRepair();
	}
}

void UFieldRepairWidget::CompleteRepair()
{
	if (!IsRepairing)
		return;

	// Stop timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepairTimerHandle);
	}

	IsRepairing = false;
	RepairProgress = 1.0f;

	IInventoryItemFieldRepairInterface* RepairInterface = Cast<IInventoryItemFieldRepairInterface>(CurrentRepairItem.GetObject());
	if (!RepairInterface)
		return;

	// Apply repair
	float MaxThreshold = RepairInterface->GetMaxDurabilityThreshold();
	float OldDurability = CurrentTargetDurability;
	CurrentTargetDurability = FMath::Min(CurrentTargetDurability + PlannedRepairAmount,
		MaxTargetDurability * MaxThreshold);
	float ActualRepair = CurrentTargetDurability - OldDurability;

	// Consume a charge via interface
	float ChargeAmount = RepairInterface->GetChargeConsumptionAmount();
	CurrentRepairKitDurability = FMath::Max(0.0f, CurrentRepairKitDurability - ChargeAmount);

	// Play complete sound via interface
	if (USoundBase* CompleteSound = RepairInterface->GetRepairCompleteSound())
	{
		UGameplayStatics::PlaySound2D(this, CompleteSound);
	}

	// Update UI
	UpdateStatusText(FText::FromString(FString::Printf(
		TEXT("Repair complete! Restored %.1f durability"), ActualRepair)));
	UpdateUI();

	// Re-enable button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(true);
	}

	OnRepairCompleted(ActualRepair);

	// Notify server/game mode to persist repair (best-effort). Try to use the Inventory Player Interface if available.
	if (IInventoryPlayerInterface* PlayerIF = GetInventoryPlayerInterface())
	{
		// Try to get repair kit item ID
		int32 RepairKitItemID = -1;
		if (UObject* RepairObj = CurrentRepairItem.GetObject())
		{
			if (const UInventoryItemBase* KitBase = Cast<UInventoryItemBase>(RepairObj))
			{
				RepairKitItemID = KitBase->ItemID;
			}
		}

		// Try to locate the equipment slot that contains the target item by scanning equipped items
		IEquipmentInterface* EquipIf = PlayerIF->GetEquipmentForInventory();
		if (EquipIf && CurrentTargetItem)
		{
			EEquipmentSlot FoundSlot = EEquipmentSlot::Unknown;
			for (uint8 SlotIndex = 0; SlotIndex < 32; ++SlotIndex)
			{
				EEquipmentSlot EquipmentSlot = static_cast<EEquipmentSlot>(SlotIndex);
				const UInventoryItemEquipable* Equipped = EquipIf->GetEquippedItem(EquipmentSlot);
				if (Equipped && Equipped->ItemID == CurrentTargetItem->ItemID)
				{
					FoundSlot = EquipmentSlot;
					break;
				}
			}

			if (FoundSlot != EEquipmentSlot::Unknown && RepairKitItemID > 0)
			{
				// Forward to server via the interface generated Execute_ function (works on UObject implementers)
				if (APlayerController* PC = GetOwningPlayer())
				{
					// Try to invoke the server RPC directly on the player controller: Server_CompleteFieldRepair(int32, EEquipmentSlot)
					UFunction* ServerFunc = PC->FindFunction(TEXT("Server_CompleteFieldRepair"));
					if (ServerFunc)
					{
						struct FServerCompleteParams
						{
							int32 RepairKitID;
							uint8 Slot;
						};
						FServerCompleteParams Params;
						Params.RepairKitID = RepairKitItemID;
						Params.Slot = static_cast<uint8>(FoundSlot);
						PC->ProcessEvent(ServerFunc, &Params);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("FieldRepairWidget: PlayerController does not expose Server_CompleteFieldRepair UFunction"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("FieldRepairWidget: No owning player controller to execute server RPC"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FieldRepairWidget: Could not auto-notify server of repair - missing slot or kit ID"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FieldRepairWidget: No equipment interface or no target item to resolve slot"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FieldRepairWidget: No IInventoryPlayerInterface available to notify server of repair"));
	}

}

void UFieldRepairWidget::CancelRepair()
{
	if (!IsRepairing)
		return;

	// Stop timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepairTimerHandle);
	}

	IsRepairing = false;
	RepairProgress = 0.0f;
	ElapsedRepairTime = 0.0f;

	// Play fail sound via interface
	if (IInventoryItemFieldRepairInterface* RepairInterface = Cast<IInventoryItemFieldRepairInterface>(CurrentRepairItem.GetObject()))
	{
		if (USoundBase* FailSound = RepairInterface->GetRepairFailSound())
		{
			UGameplayStatics::PlaySound2D(this, FailSound);
		}
	}

	UpdateStatusText(FText::FromString("Repair cancelled"));
	UpdateUI();

	// Re-enable button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(true);
	}

	OnRepairCancelled();
}

void UFieldRepairWidget::ValidateState()
{
	FText Reason;
	bool bCanRepair = CanStartRepair(Reason);

	// Enable/disable repair button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(bCanRepair && !IsRepairing);
	}

	// Update status text if not currently repairing
	if (!IsRepairing)
	{
		if (bCanRepair)
		{
			UpdateStatusText(FText::FromString("Ready to repair"));
		}
		else
		{
			UpdateStatusText(Reason);
		}
	}
}

IInventoryPlayerInterface* UFieldRepairWidget::GetInventoryPlayerInterface() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		return Cast<IInventoryPlayerInterface>(PC);
	}
	return nullptr;
}

