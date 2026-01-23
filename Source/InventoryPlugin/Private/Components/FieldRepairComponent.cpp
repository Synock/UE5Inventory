#include "Components/FieldRepairComponent.h"

#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Interfaces/FieldRepairInterface.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemFieldRepair.h"
#include "Items/Interfaces/InventoryItemFieldRepairInterface.h"
#include "Net/UnrealNetwork.h"

UFieldRepairComponent::UFieldRepairComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UFieldRepairComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UFieldRepairComponent, ActiveFieldRepair, COND_OwnerOnly);
}

void UFieldRepairComponent::OnRep_ActiveFieldRepair()
{
	// Notify clients when repair state changes
	if (ActiveFieldRepair.bIsActive)
	{
		// Repair started - broadcast to UI
		OnFieldRepairStarted.Broadcast(
			ActiveFieldRepair.RepairKitItemID,
			ActiveFieldRepair.RepairKitBagSlot,
			ActiveFieldRepair.RepairKitTopLeft,
			ActiveFieldRepair.TargetEquipmentSlot,
			ActiveFieldRepair.TargetBagSlot
		);
	}
	else
	{
		// Repair ended - check if it completed successfully or failed/cancelled
		if (ActiveFieldRepair.bCompletedSuccessfully)
		{
			// Success - broadcast completion with replicated values
			OnFieldRepairCompleted.Broadcast(
				ActiveFieldRepair.ResultActualRepairAmount,
				ActiveFieldRepair.ResultNewTargetDurability,
				ActiveFieldRepair.ResultNewKitDurability
			);
		}
		else
		{
			// Failed or cancelled - use the replicated failure reason to provide specific feedback
			FText FailureMessage;
			switch (ActiveFieldRepair.FailureReason)
			{
			case EFieldRepairFailureReason::SkillCheckFailed:
				FailureMessage = FText::FromString(TEXT("Repair skill check failed"));
				OnFieldRepairFailed.Broadcast(FailureMessage);
				break;
			case EFieldRepairFailureReason::Interrupted:
				FailureMessage = FText::FromString(TEXT("Repair was interrupted by combat or forbidden action"));
				OnFieldRepairCancelled.Broadcast(FailureMessage);
				break;
			case EFieldRepairFailureReason::ManuallyCancelled:
				FailureMessage = FText::FromString(TEXT("Repair manually cancelled"));
				OnFieldRepairCancelled.Broadcast(FailureMessage);
				break;
			case EFieldRepairFailureReason::ValidationFailed:
				FailureMessage = FText::FromString(TEXT("Repair validation failed"));
				OnFieldRepairFailed.Broadcast(FailureMessage);
				break;
			default:
				FailureMessage = FText::FromString(TEXT("Repair was interrupted or failed"));
				OnFieldRepairCancelled.Broadcast(FailureMessage);
				break;
			}
		}
	}
}


bool UFieldRepairComponent::ValidateRepairKit(int32 RepairKitItemID, EBagSlot RepairKitBagSlot, int32 RepairKitTopLeft,
                                              UInventoryComponent* InventoryComp,
                                              const UInventoryItemBase*& OutRepairKitItem,
                                              const IInventoryItemFieldRepairInterface*& OutRepairInterface,
                                              float& OutRepairKitDurability) const
{
	if (!InventoryComp)
		return false;

	// Validate repair kit bag is valid
	if (!InventoryComp->IsBagValid(RepairKitBagSlot))
		return false;

	// Validate repair kit exists in inventory
	int32 FoundItemID = InventoryComp->GetItemAtIndex(RepairKitBagSlot, RepairKitTopLeft);
	if (FoundItemID != RepairKitItemID)
		return false;

	// Get repair kit item instance
	OutRepairKitItem = UInventoryUtilities::GetItemFromID(RepairKitItemID, GetWorld());
	if (!OutRepairKitItem)
		return false;

	// Cast to field repair interface
	OutRepairInterface = Cast<IInventoryItemFieldRepairInterface>(OutRepairKitItem);
	if (!OutRepairInterface)
		return false;

	// Get repair kit durability (charges)
	OutRepairKitDurability = 0.0f;
	const TArray<FMinimalItemStorage>& BagContents = InventoryComp->GetBagConst(RepairKitBagSlot);
	for (const FMinimalItemStorage& ItemStorage : BagContents)
	{
		if (ItemStorage.TopLeftID == RepairKitTopLeft && ItemStorage.ItemID == RepairKitItemID)
		{
			OutRepairKitDurability = ItemStorage.Durability;
			break;
		}
	}

	if (OutRepairKitDurability <= 0.0f)
		return false;

	return true;
}

bool UFieldRepairComponent::ValidateTargetItem(EEquipmentSlot TargetEquipmentSlot, EBagSlot TargetBagSlot,
                                               int32 TargetTopLeft, int32 TargetItemID,
                                               UInventoryComponent* InventoryComp,
                                               UEquipmentComponent* EquipmentComp,
                                               const UInventoryItemEquipable*& OutTargetItem,
                                               float& OutCurrentDurability) const
{
	if (!InventoryComp || !EquipmentComp)
		return false;

	OutTargetItem = nullptr;
	OutCurrentDurability = 0.0f;

	if (TargetEquipmentSlot != EEquipmentSlot::Unknown)
	{
		// Repairing equipped item
		OutTargetItem = EquipmentComp->GetItemAtSlot(TargetEquipmentSlot);
		if (!OutTargetItem)
			return false;

		// Get equipment durability
		if (!EquipmentComp->GetEquipmentDurability(TargetEquipmentSlot, OutCurrentDurability))
			return false;
	}
	else if (TargetBagSlot != EBagSlot::Unknown && TargetTopLeft >= 0)
	{
		// Repairing inventory item
		if (!InventoryComp->IsBagValid(TargetBagSlot))
			return false;

		// Validate target item exists in inventory
		int32 FoundTargetItemID = InventoryComp->GetItemAtIndex(TargetBagSlot, TargetTopLeft);
		if (FoundTargetItemID != TargetItemID)
			return false;

		// Get target item instance
		const UInventoryItemBase* TargetItemBase = UInventoryUtilities::GetItemFromID(TargetItemID, GetWorld());
		if (!TargetItemBase)
			return false;

		OutTargetItem = Cast<UInventoryItemEquipable>(TargetItemBase);
		if (!OutTargetItem)
			return false;

		// Get durability from inventory
		const TArray<FMinimalItemStorage>& TargetBagContents = InventoryComp->GetBagConst(TargetBagSlot);
		bool bFoundDurability = false;
		for (const FMinimalItemStorage& ItemStorage : TargetBagContents)
		{
			if (ItemStorage.TopLeftID == TargetTopLeft && ItemStorage.ItemID == TargetItemID)
			{
				OutCurrentDurability = ItemStorage.Durability;
				bFoundDurability = true;
				break;
			}
		}

		if (!bFoundDurability)
			return false;
	}
	else
	{
		return false;
	}

	return true;
}


bool UFieldRepairComponent::CheckIncomingRepairValidity(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
                                                        int32 RepairKitTopLeft, EEquipmentSlot TargetEquipmentSlot,
                                                        EBagSlot TargetBagSlot, int32 TargetTopLeft,
                                                        int32 TargetItemID)
{
	IInventoryPlayerInterface* IIPI = Cast<IInventoryPlayerInterface>(GetOwner());
	if (!IIPI)
		return false;

	UInventoryComponent* InventoryComp = IIPI->GetInventoryComponent();
	UEquipmentComponent* EquipmentComp = IIPI->GetEquipmentForInventory()->GetEquipmentComponent();

	if (!InventoryComp || !EquipmentComp)
		return false;

	// Validate repair kit using shared helper
	const UInventoryItemBase* RepairKitItem = nullptr;
	const IInventoryItemFieldRepairInterface* RepairInterface = nullptr;
	float RepairKitDurability = 0.0f;

	if (!ValidateRepairKit(RepairKitItemID, RepairKitBagSlot, RepairKitTopLeft, InventoryComp,
	                       RepairKitItem, RepairInterface, RepairKitDurability))
		return false;

	// Validate target item using shared helper
	const UInventoryItemEquipable* TargetItem = nullptr;
	float CurrentDurability = 0.0f;

	if (!ValidateTargetItem(TargetEquipmentSlot, TargetBagSlot, TargetTopLeft, TargetItemID,
	                        InventoryComp, EquipmentComp, TargetItem, CurrentDurability))
		return false;

	// Additional validation: Check if repair kit can repair this specific item type
	FText ValidationReason;
	if (!RepairInterface->CanRepairItem(TargetItem, CurrentDurability, TargetItem->GetTotalDurability(),
	                                    RepairKitDurability, ValidationReason))
	{
		return false;
	}

	return true;
}

bool UFieldRepairComponent::BeginFieldRepair(int32 RepairKitItemID, EBagSlot RepairKitBagSlot, int32 RepairKitTopLeft,
                                             EEquipmentSlot TargetEquipmentSlot, EBagSlot TargetBagSlot,
                                             int32 TargetTopLeft, int32 TargetItemID)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (!CheckIncomingRepairValidity(RepairKitItemID, RepairKitBagSlot, RepairKitTopLeft,
	                                 TargetEquipmentSlot, TargetBagSlot, TargetTopLeft, TargetItemID))
	{
		// Validation failed - just return false, no need to broadcast (repair never started)
		return false;
	}

	//this is already checked out by the above function
	const UInventoryItemBase* RepairKitItem = UInventoryUtilities::GetItemFromID(RepairKitItemID, GetWorld());
	const IInventoryItemFieldRepairInterface* RepairInterface = Cast<IInventoryItemFieldRepairInterface>(RepairKitItem);

	// All validations passed - store active repair state
	ActiveFieldRepair.RepairKitItemID = RepairKitItemID;
	ActiveFieldRepair.RepairKitBagSlot = RepairKitBagSlot;
	ActiveFieldRepair.RepairKitTopLeft = RepairKitTopLeft;
	ActiveFieldRepair.TargetEquipmentSlot = TargetEquipmentSlot;
	ActiveFieldRepair.TargetBagSlot = TargetBagSlot;
	ActiveFieldRepair.TargetTopLeft = TargetTopLeft;
	ActiveFieldRepair.TargetItemID = TargetItemID;
	ActiveFieldRepair.StartTime = GetWorld()->GetTimeSeconds();
	ActiveFieldRepair.RepairDuration = RepairInterface->GetRepairDuration();
	ActiveFieldRepair.bIsActive = true;
	ActiveFieldRepair.bCompletedSuccessfully = false; // Clear previous result

	// Broadcast repair started event (will also trigger OnRep for clients)
	OnFieldRepairStarted.Broadcast(RepairKitItemID, RepairKitBagSlot, RepairKitTopLeft,
	                               TargetEquipmentSlot, TargetBagSlot);

	// Start server-side timer to auto-complete repair after duration
	if (UWorld* World = GetWorld())
	{
		// Auto-complete timer
		World->GetTimerManager().SetTimer(
			ActiveFieldRepair.ServerTimerHandle,
			this,
			&UFieldRepairComponent::ServerAutoCompleteFieldRepair,
			ActiveFieldRepair.RepairDuration,
			false // Don't loop
		);

		// Interrupt monitoring timer (checks every 0.5s for combat/death/trading/etc)
		FTimerHandle InterruptCheckHandle;
		World->GetTimerManager().SetTimer(
			InterruptCheckHandle,
			this,
			&UFieldRepairComponent::ServerCheckFieldRepairInterrupts,
			InterruptCheckInterval, // Check every half second
			true // Loop until repair completes or is cancelled
		);

		return true;
	}

	return false;
}

void UFieldRepairComponent::CancelFieldRepair()
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!ActiveFieldRepair.bIsActive)
		return; // No active repair to cancel

	// Clear server timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveFieldRepair.ServerTimerHandle);
	}

	// Clear state with manual cancellation reason (this will trigger OnRep for clients)
	ActiveFieldRepair.bIsActive = false;
	ActiveFieldRepair.FailureReason = EFieldRepairFailureReason::ManuallyCancelled;

	// Broadcast cancellation event on server (for listen server)
	OnFieldRepairCancelled.Broadcast(FText::FromString(TEXT("Repair manually cancelled")));
}

void UFieldRepairComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind client-side handler to repair completion event
	// This will notify the HUD when repair finishes
	//OnFieldRepairCompleted.AddDynamic(this, &UFieldRepairComponent::HandleRepairCompletedForHUD);
}

void UFieldRepairComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup: unbind from the delegate
	//OnFieldRepairCompleted.RemoveDynamic(this, &UFieldRepairComponent::HandleRepairCompletedForHUD);

	Super::EndPlay(EndPlayReason);
}

void UFieldRepairComponent::HandleRepairCompletedForHUD(float ActualRepairAmount, float NewTargetDurability,
                                                        float NewKitDurability)
{
	// This is called on both server (listen server) and clients when repair completes
	// Notify the HUD interface about the completion

	if (IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwner()))
	{
		if (IInventoryHUDInterface* HUD = PlayerInterface->GetInventoryHUDInterface())
		{
			// Use the replicated values from ActiveFieldRepair struct
			HUD->Execute_NotifyFieldRepairFinished(
				PlayerInterface->GetInventoryHUDObject(),
				ActiveFieldRepair.RepairKitBagSlot,
				ActiveFieldRepair.RepairKitTopLeft,
				ActualRepairAmount,
				NewTargetDurability,
				NewKitDurability
			);
		}
	}
}

void UFieldRepairComponent::ServerAutoCompleteFieldRepair()
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	// This is called by the server timer when repair duration elapses
	if (!ActiveFieldRepair.bIsActive)
		return;

	// Perform the actual repair using the internal helper
	ServerInternalCompleteFieldRepair();
}

void UFieldRepairComponent::ServerCheckFieldRepairInterrupts()
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	// This is called periodically to check if repair should be interrupted
	if (!ActiveFieldRepair.bIsActive)
	{
		// Timer will complete naturally when repair finishes
		return;
	}

	bool bShouldInterrupt = false;

	IFieldRepairInterface* FieldRepairInterface = Cast<IFieldRepairInterface>(GetOwner());

	if (FieldRepairInterface->IsForbiddenActionActiveWhileRepairing())
	{
		bShouldInterrupt = true;
	}

	if (bShouldInterrupt)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server interrupting field repair"));

		// Clear server timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ActiveFieldRepair.ServerTimerHandle);
		}

		// Clear state with interrupt reason (this will trigger OnRep for clients)
		ActiveFieldRepair.bIsActive = false;
		ActiveFieldRepair.bCompletedSuccessfully = false;
		ActiveFieldRepair.FailureReason = EFieldRepairFailureReason::Interrupted;

		// Broadcast cancellation event on server (for listen server)
		OnFieldRepairCancelled.Broadcast(FText::FromString(TEXT("Repair interrupted by combat or forbidden action")));
	}
}

void UFieldRepairComponent::ServerInternalCompleteFieldRepair()
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!ActiveFieldRepair.bIsActive)
		return; // Already completed or cancelled

	IFieldRepairInterface* FieldRepairInterface = Cast<IFieldRepairInterface>(GetOwner());
	if (FieldRepairInterface->IsForbiddenActionActiveWhileRepairing())
	{
		ActiveFieldRepair.bIsActive = false;
		return;
	}
	IInventoryPlayerInterface* IIPI = Cast<IInventoryPlayerInterface>(GetOwner());
	UInventoryComponent* InventoryComp = IIPI->GetInventoryComponent();
	UEquipmentComponent* EquipmentComp = IIPI->GetEquipmentForInventory()->GetEquipmentComponent();

	if (!InventoryComp || !EquipmentComp)
	{
		ActiveFieldRepair.bIsActive = false;
		return;
	}

	// Re-validate repair kit using shared helper
	const UInventoryItemBase* RepairKitItem = nullptr;
	const IInventoryItemFieldRepairInterface* RepairInterface = nullptr;
	float RepairKitDurability = 0.0f;

	if (!ValidateRepairKit(ActiveFieldRepair.RepairKitItemID, ActiveFieldRepair.RepairKitBagSlot,
	                       ActiveFieldRepair.RepairKitTopLeft, InventoryComp,
	                       RepairKitItem, RepairInterface, RepairKitDurability))
	{
		ActiveFieldRepair.bIsActive = false;
		ActiveFieldRepair.bCompletedSuccessfully = false;
		ActiveFieldRepair.FailureReason = EFieldRepairFailureReason::ValidationFailed;

		// Broadcast on server (for listen server), client will infer from OnRep
		OnFieldRepairFailed.Broadcast(FText::FromString(TEXT("Repair kit no longer available or has no charges")));
		return;
	}

	// Re-validate target item using shared helper
	const UInventoryItemEquipable* TargetItem = nullptr;
	float CurrentDurability = 0.0f;

	if (!ValidateTargetItem(ActiveFieldRepair.TargetEquipmentSlot, ActiveFieldRepair.TargetBagSlot,
	                        ActiveFieldRepair.TargetTopLeft, ActiveFieldRepair.TargetItemID,
	                        InventoryComp, EquipmentComp, TargetItem, CurrentDurability))
	{
		ActiveFieldRepair.bIsActive = false;
		ActiveFieldRepair.bCompletedSuccessfully = false;
		ActiveFieldRepair.FailureReason = EFieldRepairFailureReason::ValidationFailed;

		// Broadcast on server (for listen server), client will infer from OnRep
		OnFieldRepairFailed.Broadcast(FText::FromString(TEXT("Target item no longer available or equipped")));
		return;
	}

	bool RepairSuccess = FieldRepairInterface->RollRepairSuccess(RepairInterface);
	if (!RepairSuccess)
	{
		ActiveFieldRepair.bIsActive = false;
		ActiveFieldRepair.bCompletedSuccessfully = false;
		ActiveFieldRepair.FailureReason = EFieldRepairFailureReason::SkillCheckFailed;

		OnFieldRepairFailed.Broadcast(FText::FromString(TEXT("Repair attempt failed")));
		return;
	}

	// Determine repair type
	bool bIsEquipmentRepair = (ActiveFieldRepair.TargetEquipmentSlot != EEquipmentSlot::Unknown);
	bool bIsInventoryRepair = (ActiveFieldRepair.TargetBagSlot != EBagSlot::Unknown && ActiveFieldRepair.TargetTopLeft
		>= 0);

	// SERVER-AUTHORITATIVE CALCULATIONS (same for both equipment and inventory)
	float PlannedRepairAmount = RepairInterface->CalculateRepairAmount(CurrentDurability, TargetItem->GetTotalDurability()) *
		FieldRepairInterface->GetFieldRepairSkillModifier(RepairInterface);

	float MaxThreshold = RepairInterface->GetMaxDurabilityThreshold();

	float NewTargetDurability = FMath::Min(CurrentDurability + PlannedRepairAmount,
	                                       TargetItem->GetTotalDurability() * MaxThreshold);

	float ActualRepairAmount = NewTargetDurability - CurrentDurability;

	// Apply repair based on item location
	if (bIsEquipmentRepair)
	{
		// Apply repair to equipment
		EquipmentComp->SetEquipmentDurability(ActiveFieldRepair.TargetEquipmentSlot, NewTargetDurability);

		// Persistence handled by equipment component's replication
	}
	else if (bIsInventoryRepair)
	{
		// Apply repair to inventory item by directly updating durability
		InventoryComp->UpdateItemDurability(ActiveFieldRepair.TargetBagSlot, ActiveFieldRepair.TargetTopLeft,
		                                    ActiveFieldRepair.TargetItemID, NewTargetDurability);
	}

	const float DurabilityToRemove = RepairInterface->GetChargeConsumptionAmount();
	const float ChargeAmount = FMath::Min(DurabilityToRemove, RepairKitDurability);
	float NewKitDurability = FMath::Max(0.0f, RepairKitDurability - ChargeAmount);

	if (NewKitDurability <= 0.0f)
	{
		// Repair kit is depleted - remove from inventory
		InventoryComp->RemoveItem(ActiveFieldRepair.RepairKitBagSlot, ActiveFieldRepair.RepairKitTopLeft);
	}
	else
	{
		// Update repair kit durability by directly updating it
		InventoryComp->UpdateItemDurability(ActiveFieldRepair.RepairKitBagSlot, ActiveFieldRepair.RepairKitTopLeft,
		                                    ActiveFieldRepair.RepairKitItemID, NewKitDurability);
	}

	// Broadcast success event with server-authoritative values (on server for listen server)
	OnFieldRepairCompleted.Broadcast(ActualRepairAmount, NewTargetDurability, NewKitDurability);

	// Store results in replicated struct so clients can read them in OnRep
	ActiveFieldRepair.ResultActualRepairAmount = ActualRepairAmount;
	ActiveFieldRepair.ResultNewTargetDurability = NewTargetDurability;
	ActiveFieldRepair.ResultNewKitDurability = NewKitDurability;
	ActiveFieldRepair.bCompletedSuccessfully = true;

	// Send chat notification (only on server, will be replicated via chat system)
	const FString RepairMessage = FString::Printf(TEXT("You repaired %s, restoring %.1f durability."),
	                                              *TargetItem->Name, ActualRepairAmount);

	// Clear server timer if it hasn't fired yet
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveFieldRepair.ServerTimerHandle);
	}

	// Clear active repair state (this will trigger OnRep for clients)
	ActiveFieldRepair.bIsActive = false;
}
