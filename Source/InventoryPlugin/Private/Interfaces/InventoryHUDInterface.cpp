// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/EquipmentInterface.h"
#include "Components/EquipmentComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// Add default functionality here for any IInventoryHUDInterface functions that are not pure virtual.

void IInventoryHUDInterface::LockEquipmentSlot_Implementation(EEquipmentSlot EquipmentSlot, bool bLocked)
{
	// Default implementation that works through the equipment interface
	// Game code can override this if needed for custom HUD behavior

	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
	{
		return;
	}

	// Try to get the owning player/character with equipment
	AActor* Owner = Cast<AActor>(SelfObject->GetOuter());
	if (!Owner)
	{
		// If HUD is a widget, try to get the owning player pawn
		if (UUserWidget* Widget = Cast<UUserWidget>(SelfObject))
		{
			if (APlayerController* PC = Widget->GetOwningPlayer())
			{
				Owner = PC->GetPawn();
			}
		}
	}

	if (!Owner)
	{
		return;
	}

	// Get equipment interface from owner
	IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(Owner);
	if (!EquipmentInterface)
	{
		return;
	}

	UEquipmentComponent* EquipmentComp = EquipmentInterface->GetEquipmentComponent();
	if (!EquipmentComp)
	{
		return;
	}

	// Update lock state
	EquipmentComp->SetEquipmentLockState(EquipmentSlot, bLocked);

	// Broadcast equipment change to refresh UI
	EquipmentComp->EquipmentDispatcher.Broadcast();
}

