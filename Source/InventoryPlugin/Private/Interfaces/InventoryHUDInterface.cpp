#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/EquipmentInterface.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/Interfaces/InventoryItemBookInterface.h"
#include "Items/InventoryItemEquipable.h"
#include "UI/InventoryBagWindowInterface.h"
#include "UI/InventoryBookWidgetInterface.h"
#include "UI/InventoryLootWindowInterface.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

//----------------------------------------------------------------------------------------------------------------------
// Bag window registry — default no-ops
//----------------------------------------------------------------------------------------------------------------------

TScriptInterface<IInventoryBagWindowInterface> IInventoryHUDInterface::GetBagWindowForSlot(EBagSlot /*Slot*/) const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::RegisterBagWindowForSlot(EBagSlot /*Slot*/,
                                                       TScriptInterface<IInventoryBagWindowInterface> /*BagWindow*/)
{
}

//----------------------------------------------------------------------------------------------------------------------

TArray<EBagSlot> IInventoryHUDInterface::GetRegisteredBagSlots() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------
// Bag lifecycle
//----------------------------------------------------------------------------------------------------------------------

TSubclassOf<UUserWidget> IInventoryHUDInterface::GetBagWindowClass_Implementation() const
{
	// No plugin-default bag window asset; game code overrides this to return its draggable window class.
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::HandleBag_Implementation(EBagSlot InputBagSlot,
                                                       const TScriptInterface<IInventoryBagWindowInterface>& BagWindow)
{
	UObject* WindowObj = BagWindow.GetObject();
	if (!WindowObj)
		return;

	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
		return;

	APlayerController* PC = nullptr;
	if (const UUserWidget* AsWidget = Cast<UUserWidget>(SelfObject))
		PC = AsWidget->GetOwningPlayer();

	if (!PC)
		return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
		return;

	IEquipmentInterface* EquipInterface = Cast<IEquipmentInterface>(Pawn);
	if (!EquipInterface)
		return;

	const EEquipmentSlot EquipSlot = UInventoryComponent::GetInventorySlotFromBagSlot(InputBagSlot);
	const UInventoryItemEquipable* BagItem = EquipInterface->GetEquippedItem(EquipSlot);
	if (!BagItem)
		return;

	const IInventoryItemBagInterface* BagData = Cast<IInventoryItemBagInterface>(BagItem);
	if (!BagData)
		return;

	IInventoryBagWindowInterface::Execute_InitBagData(
		WindowObj,
		BagItem->Name,
		static_cast<int32>(BagData->GetBagWidth()),
		static_cast<int32>(BagData->GetBagHeight()),
		BagData->GetBagSize(),
		InputBagSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::DisplayBag_Implementation(EBagSlot InputBagSlot)
{
	TScriptInterface<IInventoryBagWindowInterface> BagWindow = GetBagWindowForSlot(InputBagSlot);

	if (!BagWindow.GetObject())
	{
		// Lazy-create: ask for the window class and spawn it.
		UObject* SelfObject = Cast<UObject>(this);
		if (!SelfObject)
			return;

		const TSubclassOf<UUserWidget> WindowClass = Execute_GetBagWindowClass(SelfObject);
		if (!WindowClass)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("IInventoryHUDInterface::DisplayBag — slot %d has no registered window and GetBagWindowClass returned nullptr."),
			       static_cast<int32>(InputBagSlot));
			return;
		}

		APlayerController* PC = nullptr;
		if (const UUserWidget* AsWidget = Cast<UUserWidget>(SelfObject))
			PC = AsWidget->GetOwningPlayer();

		UUserWidget* NewWindow = CreateWidget<UUserWidget>(PC, WindowClass);
		if (!NewWindow)
			return;

		if (!NewWindow->GetClass()->ImplementsInterface(UInventoryBagWindowInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("IInventoryHUDInterface::DisplayBag — created widget '%s' does not implement IInventoryBagWindowInterface."),
			       *WindowClass->GetName());
			return;
		}

		BagWindow = TScriptInterface<IInventoryBagWindowInterface>(NewWindow);
		RegisterBagWindowForSlot(InputBagSlot, BagWindow);
		NewWindow->AddToViewport();

		// SetPositionInViewport calls CanvasSlot->SetAutoSize(true), which prevents the
		// widget from stretching to fill the viewport on first creation.
		float MouseX = 0.f, MouseY = 0.f;
		if (PC)
			PC->GetMousePosition(MouseX, MouseY);
		// Align top-left corner to the cursor — window opens to the bottom-right of the mouse.
		NewWindow->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
		NewWindow->SetPositionInViewport(FVector2D(MouseX, MouseY), true);
	}

	UObject* WindowObj = BagWindow.GetObject();
	if (!WindowObj)
		return;

	// Populate with equipped item data.
	UObject* SelfObject = Cast<UObject>(this);
	Execute_HandleBag(SelfObject, InputBagSlot, BagWindow);

	IInventoryBagWindowInterface::Execute_ShowBagWindow(WindowObj);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::HideBag_Implementation(EBagSlot InputBagSlot)
{
	const TScriptInterface<IInventoryBagWindowInterface> BagWindow = GetBagWindowForSlot(InputBagSlot);
	if (UObject* WindowObj = BagWindow.GetObject())
		IInventoryBagWindowInterface::Execute_HideBagWindow(WindowObj);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::HideAllBags_Implementation()
{
	UObject* SelfObject = Cast<UObject>(this);
	for (const EBagSlot Slot : GetRegisteredBagSlots())
	{
		// Route through the full NativeEvent dispatch so Blueprint overrides of HideBag fire correctly.
		Execute_HideBag(SelfObject, Slot);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::UnequipBag_Implementation(EBagSlot InputBagSlot)
{
	const TScriptInterface<IInventoryBagWindowInterface> BagWindow = GetBagWindowForSlot(InputBagSlot);
	UObject* WindowObj = BagWindow.GetObject();
	if (!WindowObj)
		return;

	IInventoryBagWindowInterface::Execute_DeInitBagWindow(WindowObj);
	IInventoryBagWindowInterface::Execute_HideBagWindow(WindowObj);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::ToggleBag_Implementation(EBagSlot InputBagSlot)
{
	const TScriptInterface<IInventoryBagWindowInterface> BagWindow = GetBagWindowForSlot(InputBagSlot);
	UObject* WindowObj = BagWindow.GetObject();

	// No window registered yet — DisplayBag lazy-creates and shows it.
	if (!WindowObj)
	{
		UObject* SelfObject = Cast<UObject>(this);
		Execute_DisplayBag(SelfObject, InputBagSlot);
		return;
	}

	// If the window is currently visible, hide it directly.
	// Otherwise route through DisplayBag so HandleBag initializes the grid before showing.
	const UWidget* AsWidget = Cast<UWidget>(WindowObj);
	const bool bCurrentlyVisible = AsWidget &&
	                                (AsWidget->GetVisibility() == ESlateVisibility::Visible ||
	                                 AsWidget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible ||
	                                 AsWidget->GetVisibility() == ESlateVisibility::HitTestInvisible);

	if (bCurrentlyVisible)
	{
		IInventoryBagWindowInterface::Execute_HideBagWindow(WindowObj);
	}
	else
	{
		UObject* SelfObject = Cast<UObject>(this);
		Execute_DisplayBag(SelfObject, InputBagSlot);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Loot window registry — default no-ops
//----------------------------------------------------------------------------------------------------------------------

TScriptInterface<IInventoryLootWindowInterface> IInventoryHUDInterface::GetLootWindow() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::RegisterLootWindow(TScriptInterface<IInventoryLootWindowInterface> /*LootWindow*/)
{
}

//----------------------------------------------------------------------------------------------------------------------
// Loot lifecycle
//----------------------------------------------------------------------------------------------------------------------

TSubclassOf<UUserWidget> IInventoryHUDInterface::GetLootWindowClass_Implementation() const
{
	static TSoftClassPtr<UUserWidget> DefaultClass(FSoftObjectPath(TEXT("/InventoryPlugin/UI/UI_LootWidget.UI_LootWidget_C")));
	if (UClass* Loaded = DefaultClass.LoadSynchronous())
		return Loaded;
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::DisplayLootScreen_Implementation(AActor* LootedActor)
{
	if (!LootedActor)
		return;

	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
		return;

	APlayerController* PC = nullptr;
	if (const UUserWidget* AsWidget = Cast<UUserWidget>(SelfObject))
		PC = AsWidget->GetOwningPlayer();

	TScriptInterface<IInventoryLootWindowInterface> LootWindow = GetLootWindow();

	if (!LootWindow.GetObject())
	{
		const TSubclassOf<UUserWidget> WindowClass = Execute_GetLootWindowClass(SelfObject);
		if (!WindowClass)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("IInventoryHUDInterface::DisplayLootScreen — no registered window and GetLootWindowClass returned nullptr."));
			return;
		}

		UUserWidget* NewWindow = CreateWidget<UUserWidget>(PC, WindowClass);
		if (!NewWindow)
			return;

		if (!NewWindow->GetClass()->ImplementsInterface(UInventoryLootWindowInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("IInventoryHUDInterface::DisplayLootScreen — created widget '%s' does not implement IInventoryLootWindowInterface."),
			       *WindowClass->GetName());
			return;
		}

		LootWindow = TScriptInterface<IInventoryLootWindowInterface>(NewWindow);
		RegisterLootWindow(LootWindow);
		NewWindow->AddToViewport();

		// Position bottom-right of the cursor on first creation.
		NewWindow->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
		float MouseX = 0.f, MouseY = 0.f;
		if (PC)
			PC->GetMousePosition(MouseX, MouseY);
		NewWindow->SetPositionInViewport(FVector2D(MouseX, MouseY), true);
	}

	UObject* WindowObj = LootWindow.GetObject();
	if (!WindowObj)
		return;

	IInventoryLootWindowInterface::Execute_InitLootWindow(WindowObj, LootedActor);
	IInventoryLootWindowInterface::Execute_ShowLootWindow(WindowObj);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::HideLootScreen_Implementation()
{
	const TScriptInterface<IInventoryLootWindowInterface> LootWindow = GetLootWindow();
	UObject* WindowObj = LootWindow.GetObject();
	if (!WindowObj)
		return;

	IInventoryLootWindowInterface::Execute_DeInitLootWindow(WindowObj);
	IInventoryLootWindowInterface::Execute_HideLootWindow(WindowObj);
}

//----------------------------------------------------------------------------------------------------------------------
// Book
//----------------------------------------------------------------------------------------------------------------------

TSubclassOf<UUserWidget> IInventoryHUDInterface::GetBookWidgetClass_Implementation() const
{
	static TSoftClassPtr<UUserWidget> DefaultClass(FSoftObjectPath(TEXT("/InventoryPlugin/UI/UI_BookWidget.UI_BookWidget_C")));
	if (UClass* Loaded = DefaultClass.LoadSynchronous())
		return Loaded;
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::DisplayBookText_Implementation(const UInventoryItemBase* Item, float X, float Y)
{
	if (!Item)
		return;

	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
		return;

	APlayerController* PC = nullptr;
	if (const UUserWidget* AsWidget = Cast<UUserWidget>(SelfObject))
		PC = AsWidget->GetOwningPlayer();

	const TSubclassOf<UUserWidget> WidgetClass = Execute_GetBookWidgetClass(SelfObject);
	if (!WidgetClass)
		return;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!Widget)
		return;

	IInventoryBookWidgetInterface* BookWidget = Cast<IInventoryBookWidgetInterface>(Widget);
	if (!BookWidget)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("IInventoryHUDInterface::DisplayBookText — widget class '%s' does not implement IInventoryBookWidgetInterface."),
		       *WidgetClass->GetName());
		return;
	}

	IInventoryBookWidgetInterface::Execute_SetupUI(Widget);

	if (Item->GetClass()->ImplementsInterface(UInventoryItemBookInterface::StaticClass()))
	{
		IInventoryBookWidgetInterface::Execute_SetTitle(Widget, IInventoryItemBookInterface::Execute_GetBookTitle(Item));
		IInventoryBookWidgetInterface::Execute_SetPages(Widget, IInventoryItemBookInterface::Execute_GetBookPages(Item));
	}

	float MouseX = X, MouseY = Y;
	if (PC)
		PC->GetMousePosition(MouseX, MouseY);

	Widget->AddToViewport(5);
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(FVector2D(MouseX, MouseY), true);
}

//----------------------------------------------------------------------------------------------------------------------
// Field repair
//----------------------------------------------------------------------------------------------------------------------

void IInventoryHUDInterface::LockEquipmentSlot_Implementation(EEquipmentSlot EquipmentSlot, bool bLocked)
{
	UObject* SelfObject = Cast<UObject>(this);
	if (!SelfObject)
		return;

	AActor* Owner = Cast<AActor>(SelfObject->GetOuter());
	if (!Owner)
	{
		if (const UUserWidget* Widget = Cast<UUserWidget>(SelfObject))
		{
			if (APlayerController* PC = Widget->GetOwningPlayer())
				Owner = PC->GetPawn();
		}
	}

	if (!Owner)
		return;

	IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(Owner);
	if (!EquipmentInterface)
		return;

	UEquipmentComponent* EquipmentComp = EquipmentInterface->GetEquipmentComponent();
	if (!EquipmentComp)
		return;

	EquipmentComp->SetEquipmentLockState(EquipmentSlot, bLocked);
	EquipmentComp->EquipmentDispatcher.Broadcast();
}
