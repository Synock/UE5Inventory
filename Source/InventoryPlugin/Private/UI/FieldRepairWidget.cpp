#include "UI/FieldRepairWidget.h"
#include "InventoryPlugin.h"
#include "InventoryUtilities.h"
#include "UI/FieldRepairWidgetInterface.h"
#include "Items/Interfaces/InventoryItemFieldRepairInterface.h"
#include "Components/InventoryComponent.h"
#include "InventoryPlugin.h"
#include "Items/InventoryItemEquipable.h"
#include "InventoryPlugin.h"
#include "UI/FieldRepairSlotWidget.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "InventoryPlugin.h"
#include "Components/Button.h"
#include "InventoryPlugin.h"
#include "Components/ProgressBar.h"
#include "InventoryPlugin.h"
#include "Components/TextBlock.h"
#include "InventoryPlugin.h"
#include "Components/Image.h"
#include "InventoryPlugin.h"
#include "TimerManager.h"
#include "InventoryPlugin.h"
#include "Kismet/GameplayStatics.h"
#include "InventoryPlugin.h"

void UFieldRepairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callback
	if (RepairButton)
	{
		RepairButton->OnClicked.AddDynamic(this, &UFieldRepairWidget::OnRepairButtonClicked);
	}

	// Initialize UI via interface
	Execute_UpdateUI(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::NativeDestruct()
{
	// Clean up timer if widget is destroyed during repair
	if (IsRepairing)
	{
		CancelRepair();
	}

	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update repair progress display if repairing
	if (IsRepairing)
	{
		Execute_UpdateRepairProgressBar(this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::SetVisibility(ESlateVisibility InVisibility)
{
	ESlateVisibility OldVisibility = GetVisibility();

	// Call parent implementation first
	Super::SetVisibility(InVisibility);

	// If widget is being hidden, clean up state
	if (InVisibility == ESlateVisibility::Hidden || InVisibility == ESlateVisibility::Collapsed)
	{
		// Only cleanup if we were visible before (to avoid redundant operations)
		if (OldVisibility == ESlateVisibility::Visible || OldVisibility == ESlateVisibility::HitTestInvisible ||
			OldVisibility == ESlateVisibility::SelfHitTestInvisible)
		{
			// Cancel any ongoing repair
			if (IsRepairing)
			{
				CancelRepair();
			}

			// Clear the target item to ensure clean state for next window open
			if (CurrentTargetItem != nullptr)
			{
				ClearTargetItem();
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

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
	IFieldRepairWidgetInterface::Execute_UpdateStatusText(this, FText::FromString("Select an item to repair"));

	Execute_ValidateState(this);
}

//----------------------------------------------------------------------------------------------------------------------

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
		MaxTargetDurability = FMath::Max(1.0f, TargetItem->GetTotalDurability());
	}
	else
	{
		MaxTargetDurability = 100.0f;
	}

	IFieldRepairWidgetInterface::Execute_UpdateUI(this);
	Execute_ValidateState(this);

	OnTargetItemChanged();
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::ClearTargetItem()
{
	SetTargetItem(nullptr, 0.0f);
}

//----------------------------------------------------------------------------------------------------------------------

UTexture2D* UFieldRepairWidget::GetRepairKitIcon() const
{
	if (UObject* RepairObject = CurrentRepairItem.GetObject())
	{
		if (const UInventoryItemBase* ItemBase = Cast<UInventoryItemBase>(RepairObject))
		{
			return ItemBase->Icon;
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::TickRepair_Implementation()
{
	if (!IsRepairing)
		return;

	// Call base interface implementation
	IFieldRepairWidgetInterface::TickRepair_Implementation();

	// Check if repair is complete
	if (ElapsedRepairTime >= TotalRepairDuration)
	{
		CompleteRepair();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::OnRepairTimerTick()
{
	// Call TickRepair via Execute_ to properly invoke interface
	Execute_TickRepair(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::OnRepairButtonClicked_Implementation()
{
	if (IsRepairing)
		CancelRepair();
	else
		StartRepair();
}

//----------------------------------------------------------------------------------------------------------------------

bool UFieldRepairWidget::CanStartRepair_Implementation(FText& OutReason) const
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

//----------------------------------------------------------------------------------------------------------------------

bool UFieldRepairWidget::ShouldEnableRepairButton_Implementation() const
{
	FText Reason;
	return Execute_CanStartRepair(const_cast<UFieldRepairWidget*>(this), Reason) && !IsRepairing;
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::StartRepair()
{
	FText Reason;
	if (!Execute_CanStartRepair(this, Reason))
	{
		Execute_UpdateStatusText(this, Reason);
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
		World->GetTimerManager().SetTimer(RepairTimerHandle, this, &UFieldRepairWidget::OnRepairTimerTick,
			0.1f, true);
	}

	// Play start sound via interface
	if (USoundBase* StartSound = RepairInterface->GetRepairStartSound())
	{
		UGameplayStatics::PlaySound2D(this, StartSound);
	}

	Execute_UpdateStatusText(this, FText::FromString(FString::Printf(
		TEXT("Repairing... (%.1f seconds)"), TotalRepairDuration)));
	Execute_UpdateUI(this);

	OnRepairStarted();
}

//----------------------------------------------------------------------------------------------------------------------

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
	Execute_UpdateStatusText(this, FText::FromString(FString::Printf(
		TEXT("Repair complete! Restored %.1f durability"), ActualRepair)));
	Execute_UpdateUI(this);

	// Re-enable button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(true);
	}

	// Validate state for next repair (this will re-enable button if conditions met)
	Execute_ValidateState(this);

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
						UE_LOG(LogInventoryPlugin, Warning, TEXT("FieldRepairWidget: PlayerController does not expose Server_CompleteFieldRepair UFunction"));
					}
				}
				else
				{
					UE_LOG(LogInventoryPlugin, Warning, TEXT("FieldRepairWidget: No owning player controller to execute server RPC"));
				}
			}
			else
			{
				UE_LOG(LogInventoryPlugin, Warning, TEXT("FieldRepairWidget: Could not auto-notify server of repair - missing slot or kit ID"));
			}
		}
		else
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("FieldRepairWidget: No equipment interface or no target item to resolve slot"));
		}
	}
	else
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("FieldRepairWidget: No IInventoryPlayerInterface available to notify server of repair"));
	}

}

//----------------------------------------------------------------------------------------------------------------------

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

	Execute_UpdateStatusText(this, FText::FromString("Repair cancelled"));
	Execute_UpdateUI(this);

	// Re-enable button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(true);
	}

	// Validate state
	Execute_ValidateState(this);

	OnRepairCancelled();
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::ValidateState_Implementation()
{
	FText Reason;
	bool bCanRepair = Execute_CanStartRepair(this, Reason);
	bool bIsRepairing = IsRepairing;

	// Enable/disable repair button
	if (RepairButton)
	{
		RepairButton->SetIsEnabled(bCanRepair && !bIsRepairing);
	}

	// Update status text if not currently repairing
	if (!bIsRepairing)
	{
		if (bCanRepair)
		{
			Execute_UpdateStatusText(this, FText::FromString(TEXT("Ready to repair")));
		}
		else
		{
			Execute_UpdateStatusText(this, Reason);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UFieldRepairWidget::GetInventoryPlayerInterface() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		return Cast<IInventoryPlayerInterface>(PC);
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
// IFieldRepairWidgetInterface — Window Lifecycle
//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::InitFieldRepairWindow_Implementation(int32 RepairKitItemID, EBagSlot BagSlot, int32 TopLeft)
{
	IInventoryPlayerInterface* Player = GetInventoryPlayerInterface();
	if (!Player)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UFieldRepairWidget::InitFieldRepairWindow — no IInventoryPlayerInterface on owning controller."));
		return;
	}

	const UInventoryComponent* InvComp = Player->GetInventoryComponentConst();
	if (!InvComp)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UFieldRepairWidget::InitFieldRepairWindow — no InventoryComponent available."));
		return;
	}

	// Locate the item storage entry to read current durability.
	const TArray<FMinimalItemStorage>& BagItems = InvComp->GetBagConst(BagSlot);
	const FMinimalItemStorage* StorageEntry = BagItems.FindByPredicate([TopLeft](const FMinimalItemStorage& S)
	{
		return S.TopLeftID == TopLeft;
	});

	if (!StorageEntry)
	{
		UE_LOG(LogInventoryPlugin, Warning,
		       TEXT("UFieldRepairWidget::InitFieldRepairWindow — no item found at BagSlot %d / TopLeft %d."),
		       static_cast<int32>(BagSlot), TopLeft);
		return;
	}

	// Resolve the item object from the global registry.
	UInventoryItemBase* ItemBase = UInventoryUtilities::GetItemFromID(RepairKitItemID, GetWorld());
	if (!ItemBase)
	{
		UE_LOG(LogInventoryPlugin, Warning,
		       TEXT("UFieldRepairWidget::InitFieldRepairWindow — GetItemFromID returned null for ID %d."),
		       RepairKitItemID);
		return;
	}

	if (!ItemBase->GetClass()->ImplementsInterface(UInventoryItemFieldRepairInterface::StaticClass()))
	{
		UE_LOG(LogInventoryPlugin, Warning,
		       TEXT("UFieldRepairWidget::InitFieldRepairWindow — item ID %d does not implement IInventoryItemFieldRepairInterface."),
		       RepairKitItemID);
		return;
	}

	TScriptInterface<IInventoryItemFieldRepairInterface> RepairItem;
	RepairItem.SetObject(ItemBase);
	RepairItem.SetInterface(Cast<IInventoryItemFieldRepairInterface>(ItemBase));

	InitializeWithRepairItem(RepairItem, StorageEntry->Durability);
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::ShowFieldRepairWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::HideFieldRepairWindow_Implementation()
{
	if (IsRepairing)
		CancelRepair();

	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::DeInitFieldRepairWindow_Implementation()
{
	if (IsRepairing)
		CancelRepair();

	CurrentRepairItem = nullptr;
	ClearTargetItem();
}

//----------------------------------------------------------------------------------------------------------------------

void UFieldRepairWidget::OnFieldRepairFinished_Implementation(EBagSlot /*RepairBagSlot*/, int32 /*RepairTopLeft*/,
                                                              float /*ActualRepairAmount*/,
                                                              float NewTargetDurability, float NewKitDurability)
{
	// Apply server-confirmed durability values and refresh UI.
	CurrentTargetDurability = NewTargetDurability;
	CurrentRepairKitDurability = NewKitDurability;
	IsRepairing = false;
	RepairProgress = 0.0f;
	ElapsedRepairTime = 0.0f;

	Execute_UpdateUI(this);
	Execute_ValidateState(this);
}

