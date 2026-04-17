
#include "UI/LootScreenWidget.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "UI/InventoryGridWidget.h"
#include "Components/LootPoolComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/LootableInterface.h"

void ULootScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ULootScreenWidget::OnCloseButtonClicked);
	}

	if (LootAllButton)
	{
		LootAllButton->OnClicked.AddDynamic(this, &ULootScreenWidget::OnLootAllButtonClicked);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::NativeDestruct()
{
	DeInitLootData();
	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::OnCloseButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (IInventoryPlayerInterface* Inv = Cast<IInventoryPlayerInterface>(PC))
		{
			Inv->StopLooting(nullptr);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::OnLootAllButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (IInventoryPlayerInterface* Inv = Cast<IInventoryPlayerInterface>(PC))
		{
			Inv->PlayerAutoLootAll();
			Inv->StopLooting(nullptr);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

int32 ULootScreenWidget::ComputeRequiredGridHeight() const
{
	const ILootableInterface* Lootable = LootedActor.GetInterface();
	const ULootPoolComponent* Pool = Lootable ? Lootable->GetLootPoolComponentConst() : nullptr;

	if (!Pool || Pool->GetBagConst().IsEmpty())
		return MaxVisibleRows;

	const int32 SafeWidth = FMath::Max(1, BagWidth);
	int32 MaxRow = 0;
	for (const FMinimalItemStorage& Item : Pool->GetBagConst())
	{
		MaxRow = FMath::Max(MaxRow, Item.TopLeftID / SafeWidth);
	}

	// +1 converts 0-based row index to height; +1 safety buffer for items taller than 1 slot
	// Floor is MaxVisibleRows so the base grid is always the full normal size even when mostly empty
	return FMath::Clamp(MaxRow + 2, MaxVisibleRows, BagHeight);
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::InitLootData(AActor* InputLootedActor)
{
	if (ILootableInterface* Interface = Cast<ILootableInterface>(InputLootedActor); InputLootedActor && Interface)
	{
		LootedActor.SetObject(InputLootedActor);
		LootedActor.SetInterface(Interface);
		LootName = Interface->GetLootActorName();
		LootedActor = InputLootedActor;
	}
	InitUI();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::InitUI_Implementation()
{
	if (!LootGrid || !LootedActor.GetObject())
		return;

	// Register before InitData so our resize fires before the grid's own Refresh in the delegate chain
	if (ILootableInterface* Lootable = Cast<ILootableInterface>(LootedActor.GetObject()))
	{
		Lootable->GetLootPoolDelegate().AddUniqueDynamic(this, &ULootScreenWidget::OnLootPoolChanged);
	}

	const int32 RequiredHeight = ComputeRequiredGridHeight();
	LootGrid->InitData(Cast<AActor>(LootedActor.GetObject()), EBagSlot::LootPool, BagWidth, RequiredHeight);
	Refresh();
	// You can't drop stuff on a loot panel
	LootGrid->SetCanAcceptDrop(false);

	if (LootScrollSizeBox)
	{
		LootScrollSizeBox->SetMaxDesiredHeight(static_cast<float>(MaxVisibleRows) * LootGrid->GetTileSize());
	}

	if (LootScrollBox)
	{
		LootScrollBox->ScrollToStart();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::Refresh_Implementation()
{
	if (LootGrid)
		LootGrid->Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::DeInitUI_Implementation()
{
	if (LootedActor.GetObject())
	{
		if (ILootableInterface* Lootable = Cast<ILootableInterface>(LootedActor.GetObject()))
		{
			Lootable->GetLootPoolDelegate().RemoveDynamic(this, &ULootScreenWidget::OnLootPoolChanged);
		}
	}

	if (LootGrid)
		LootGrid->DeInitData();
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::OnLootPoolChanged()
{
	if (!LootGrid)
		return;

	const int32 NewHeight = ComputeRequiredGridHeight();
	if (NewHeight != LootGrid->GetHeight())
	{
		// ResizeBagArea clears ItemGrid/ItemList; Refresh repopulates them for the new dimensions
		LootGrid->ResizeBagArea(BagWidth, NewHeight);
		LootGrid->CreateLineSegments();
		LootGrid->Refresh();
	}

	if (LootScrollSizeBox)
	{
		LootScrollSizeBox->SetMaxDesiredHeight(static_cast<float>(MaxVisibleRows) * LootGrid->GetTileSize());
	}
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::DeInitLootData()
{
	DeInitUI();
	LootedActor = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
// IInventoryLootWindowInterface
//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::InitLootWindow_Implementation(AActor* InLootedActor)
{
	InitLootData(InLootedActor);
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::DeInitLootWindow_Implementation()
{
	DeInitLootData();
	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::ShowLootWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::HideLootWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void ULootScreenWidget::RefreshLootWindow_Implementation()
{
	Refresh();
}

