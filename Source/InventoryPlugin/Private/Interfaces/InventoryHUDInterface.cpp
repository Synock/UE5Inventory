#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/EquipmentInterface.h"
#include "Components/EquipmentComponent.h"
#include "Items/Interfaces/InventoryItemBookInterface.h"
#include "UI/InventoryBookWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// Add default functionality here for any IInventoryHUDInterface functions that are not pure virtual.

TSubclassOf<UUserWidget> IInventoryHUDInterface::GetBookWidgetClass_Implementation() const
{
	static TSoftClassPtr<UUserWidget> DefaultClass(FSoftObjectPath(TEXT("/InventoryPlugin/UI/UI_BookWidget.UI_BookWidget_C")));
	if (UClass* Loaded = DefaultClass.LoadSynchronous())
	{
		return Loaded;
	}
	return nullptr;
}

void IInventoryHUDInterface::DisplayBookText_Implementation(const UInventoryItemBase* Item, float X, float Y)
{
	if (!Item)
	{
		return;
	}

	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (const UUserWidget* AsWidget = Cast<UUserWidget>(SelfObject))
	{
		PC = AsWidget->GetOwningPlayer();
	}

	const TSubclassOf<UUserWidget> WidgetClass = Execute_GetBookWidgetClass(SelfObject);
	if (!WidgetClass)
	{
		return;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!Widget)
	{
		return;
	}

	IInventoryBookWidgetInterface* BookWidget = Cast<IInventoryBookWidgetInterface>(Widget);
	if (!BookWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("IInventoryHUDInterface::DisplayBookText — widget class '%s' does not implement IInventoryBookWidgetInterface."), *WidgetClass->GetName());
		return;
	}

	IInventoryBookWidgetInterface::Execute_SetupUI(Widget);

	if (Item->GetClass()->ImplementsInterface(UInventoryItemBookInterface::StaticClass()))
	{
		IInventoryBookWidgetInterface::Execute_SetTitle(Widget, IInventoryItemBookInterface::Execute_GetBookTitle(Item));
		IInventoryBookWidgetInterface::Execute_SetPages(Widget, IInventoryItemBookInterface::Execute_GetBookPages(Item));
	}

	Widget->AddToViewport(5);
	Widget->SetPositionInViewport(FVector2D(X, Y), true);
}



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

