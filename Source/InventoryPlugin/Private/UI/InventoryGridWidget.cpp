// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/InventoryGridWidget.h"

#include "InventoryUtilities.h"
#include "Components/BankComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/LootPoolComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/LootableInterface.h"
#include "Items/InventoryItemBag.h"

FVector2D UInventoryGridWidget::GetItemScreenFootprint(UItemWidget* Item) const
{
	if (!Item)
		return {};

	return {Item->GetReferencedItem()->Width * TileSize, Item->GetReferencedItem()->Height * TileSize};
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::CanProcessItemDrop(UItemWidget* IncomingItem) const
{
	return !IsItself(IncomingItem, DraggedItemTopLeftID) && IsRoomAvailable(
		IncomingItem->GetReferencedItem(), DraggedItemTopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::HandleItemDrop(UItemWidget* IncomingItem)
{
	if (!IncomingItem)
		return false;

	if (!IncomingItem->GetReferencedItem())
		return false;

	if (!CanProcessItemDrop(IncomingItem))
	{
		DraggedItemTopLeftID = -1;
		DrawDropLocation = false;
		return false;
	}

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return false;

	const UInventoryItemBase* Item = IncomingItem->GetReferencedItem();
	if (IncomingItem->IsBelongingToSelf())
	{
		if (IncomingItem->IsFromEquipment())
		{
			PC->PlayerUnequipItem(DraggedItemTopLeftID, BagID, Item->ItemID, IncomingItem->GetOriginalSlot());
		}
		else
		{
			PC->PlayerMoveItem(DraggedItemTopLeftID, BagID, Item->ItemID, IncomingItem->GetTopLeftID(),
			                   IncomingItem->GetBagID());
		}
	}
	else
	{
		PC->PlayerLootItem(DraggedItemTopLeftID, BagID, Item->ItemID, IncomingItem->GetTopLeftID());
	}


	DraggedItemTopLeftID = -1;
	DrawDropLocation = false;
	return true;
}

bool UInventoryGridWidget::UpdateDraggedItemTopLeft(UItemWidget* IncomingItem, float X, float Y)
{
	if (!IncomingItem)
		return false;

	DraggedItemTopLeftID = GetActualTopLeftCorner(X, Y, IncomingItem->GetReferencedItem()->Width,
	                                              IncomingItem->GetReferencedItem()->Height);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::AddItemWidgetToGrid(UCanvasPanel* GridCanvasPanel, UWidget* Content, int32 TopLeft)
{
	if (!GridCanvasPanel || !Content)
		return;

	UCanvasPanelSlot* PanelSlot = GridCanvasPanel->AddChildToCanvas(Content);

	const float XPos = (TopLeft % Width) * TileSize;
	const float YPos = (TopLeft / Width) * TileSize;
	PanelSlot->SetAutoSize(true);
	PanelSlot->SetPosition({XPos, YPos});
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::CreateNewItem(UCanvasPanel* GridCanvasPanel, const FMinimalItemStorage& ItemStorage)
{
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetWorld());
	check(Item);
	UItemWidget* ItemWidget = Cast<UItemWidget>(CreateWidget(GetOwningPlayer(), ItemWidgetClass));

	ItemWidget->SetParentGrid(this);
	ItemWidget->InitData(Item, GetOwningPlayerPawn(), TileSize, ItemStorage.TopLeftID, BagID);

	AddItemWidgetToGrid(GridCanvasPanel, ItemWidget, ItemStorage.TopLeftID);
	RegisterNewItem(ItemStorage.TopLeftID, ItemWidget);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::FullRefresh(UCanvasPanel* GridCanvasPanel)
{
	for (int Id = ItemList.Num() - 1; Id >= 0; --Id)
	{
		UnRegisterItem(ItemList[Id]);
	}

	GridCanvasPanel->ClearChildren();

	for (auto& NewItem : GetItemData())
	{
		CreateNewItem(GridCanvasPanel, NewItem);
	}
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UInventoryGridWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::ResetTransaction()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		PC->ResetTransaction();
}

//----------------------------------------------------------------------------------------------------------------------

UInventoryGridWidget::UInventoryGridWidget(const FObjectInitializer& ObjectInitializer): UUserWidget(ObjectInitializer)
{
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::ResizeBagArea(int32 InputWidth, int32 InputHeight)
{
	Width = InputWidth;
	Height = InputHeight;

	ItemGrid.Empty();
	ItemGrid.SetNumZeroed(Width * Height);

	ItemList.Empty();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::InitData(AActor* Owner, EBagSlot InputBagSlot, int32 InputWidth, int32 InputHeight)
{
	ActorOwner = Owner;
	BagID = InputBagSlot;

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();

	check(PC);
	PC->GetInventoryComponent()->FullInventoryDispatcher.AddUniqueDynamic(
		this, &UInventoryGridWidget::ResetTransaction);

	//these are internal bags of the inventory
	if (BagID == EBagSlot::Pocket1 || BagID == EBagSlot::Pocket2)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 3, InputHeight > 0 ? InputHeight : 2);
		PC->GetInventoryComponent()->FullInventoryDispatcher.AddDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else if (BagID == EBagSlot::LootPool)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 8, InputHeight > 0 ? InputHeight : 8);

		Cast<ILootableInterface>(ActorOwner)->GetLootPoolDelegate().
		                                      AddDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else if (BagID == EBagSlot::BankPool)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 8, InputHeight > 0 ? InputHeight : 16);
		PC->GetBankComponent()->BankPoolDispatcher.AddDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else
	{
		const EEquipmentSlot RelatedSlot = UInventoryComponent::GetInventorySlotFromBagSlot(BagID);

		const UInventoryItemBag* BagItem = Cast<UInventoryItemBag>(
			PC->GetEquipmentForInventory()->GetEquippedItem(RelatedSlot));

		ensure(BagItem);
		ResizeBagArea(BagItem->BagWidth, BagItem->BagHeight);
		MaximumBagSize = BagItem->BagSize;
		PC->GetInventoryComponent()->FullInventoryDispatcher.AddDynamic(this, &UInventoryGridWidget::Refresh);
	}

	//next block is UI size
	SetUISize(GetWidth() * TileSize, GetHeight() * TileSize);
	Lines.Empty();
	CreateLineSegments();
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::GetPositionFromTopLeft(int32 TopLeft, float& PositionX, float& PositionY)
{
	const int32 ModVal = TopLeft % GetWidth();
	const int32 DivVal = TopLeft / GetWidth();
	PositionX = ModVal * TileSize;
	PositionY = DivVal * TileSize;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::CreateLineSegments()
{
	{
		float Y = TileSize * Height;
		for (size_t i = 0; i <= Width; ++i)
		{
			float X = TileSize * i;
			Lines.Add(FInventoryLine({X, 0.f}, {X, Y}));
		}
	}

	{
		float X = TileSize * Width;
		for (size_t i = 0; i <= Height; ++i)
		{
			float Y = TileSize * i;
			Lines.Add(FInventoryLine({0.f, Y}, {X, Y}));
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::MousePositionInTile(float XPosition, float YPosition, bool& IsOnRight,
                                               bool& IsOnBottom) const
{
	const float ModX = FMath::Fmod(XPosition, TileSize);
	const float ModY = FMath::Fmod(YPosition, TileSize);
	const float HSize = TileSize / 2.f;

	IsOnRight = ModX > HSize;
	IsOnBottom = ModY > HSize;
}

//----------------------------------------------------------------------------------------------------------------------

int32 UInventoryGridWidget::GetActualTopLeftCorner(float XPosition, float YPosition, int32 ItemWidth,
                                                   int32 ItemHeight) const
{
	int32 CellX = 0;
	int32 CellY = 0;
	GetXYCellFromFloatingPoint(XPosition, YPosition, CellX, CellY);

	//easy case where the mouse cursor is on the only cell
	if (ItemWidth == 1 && ItemHeight == 1)
	{
		return GetTopLeftFromCellXY(CellX, CellY);
	}

	//annoying case where we have bigger objects
	const uint32 HalfWidth = static_cast<uint32>(FMath::Floor(ItemWidth / 2.0f));
	const uint32 HalfHeight = static_cast<uint32>(FMath::Floor(ItemHeight / 2.0f));

	//find out where the mouse is in the current cell
	bool IsOnRight = false;
	bool IsOnBottom = false;
	MousePositionInTile(XPosition, YPosition, IsOnRight, IsOnBottom);

	//by default item expand by half its size in each direction
	int32 ReachX = HalfWidth;
	int32 ReachY = HalfHeight;

	//however if we are on the right we will favor a right placement, changing the TopLeft
	if (IsOnRight)
		ReachX -= 1;

	if (IsOnBottom)
		ReachY -= 1;

	CellX -= ReachX;
	CellY -= ReachY;

	return GetTopLeftFromCellXY(CellX, CellY);;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::HasItemLocally(const FMinimalItemStorage& ItemData) const
{
	for (auto& Elm : ItemList)
	{
		if (Elm->GetTopLeftID() == ItemData.TopLeftID && Elm->GetReferencedItem()->ItemID == ItemData.ItemID)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetLocalItem(const FMinimalItemStorage& ItemData, bool& Found) const
{
	Found = false;
	for (auto& Elm : ItemList)
	{
		if (Elm->GetTopLeftID() == ItemData.TopLeftID && Elm->GetReferencedItem()->ItemID == ItemData.ItemID)
		{
			Found = true;
			return Elm;
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::RegisterExistingItem(UItemWidget* ItemData)
{
	RegisterNewItem(ItemData->GetTopLeftID(), ItemData);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::DeInitData()
{
	if (BagID == EBagSlot::LootPool)
	{
		Cast<ILootableInterface>(ActorOwner)->GetLootPoolDelegate().
		                                      RemoveAll(this);
	}
	else
	{
		GetInventoryPlayerInterface()->GetInventoryComponent()->FullInventoryDispatcher.RemoveAll(this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsRoomAvailable(const UInventoryItemBase* ItemObject, int TopLeftIndex) const
{
	const int32 MaxIndex = Width * Height;

	if (TopLeftIndex < 0 || TopLeftIndex > MaxIndex)
		return false;

	if (ItemObject->ItemSize > MaximumBagSize)
		return false;

	const int32 ItemWidth = ItemObject->Width;
	const int32 ItemHeight = ItemObject->Height;
	const int32 sx = TopLeftIndex % Width;
	const int32 sy = TopLeftIndex / Width;

	for (int y = sy; y < sy + ItemHeight; ++y)
	{
		for (int x = sx; x < sx + ItemWidth; ++x)
		{
			if (x >= Width || x < 0)
			{
				return false;
			}
			if (y >= Height || y < 0)
			{
				return false;
			}

			const int32 ID = x + y * Width;
			if (ID < 0 || ID >= Width * Height)
				return false;

			if (ItemGrid[ID] != nullptr) //only look for empty stuff
			{
				UE_LOG(LogTemp, Log, TEXT("Cannot copy because %d %d %d is not empty"), x, y, ID);
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsItself(UItemWidget* IncomingItem, int TopLeftIndex) const
{
	return IncomingItem->GetTopLeftID() == TopLeftIndex && IncomingItem->GetBagID() == BagID;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::RegisterNewItem(int32 TopLeft, UItemWidget* NewItem)
{
	//register item
	ItemList.Add(NewItem);

	//assign item to the map
	const int ItemWidth = NewItem->GetReferencedItem()->Width;
	const int ItemHeight = NewItem->GetReferencedItem()->Height;
	const int32 Sx = TopLeft % Width;
	const int32 Sy = TopLeft / Width;

	for (int y = Sy; y < Sy + ItemHeight; ++y)
	{
		for (int x = Sx; x < Sx + ItemWidth; ++x)
		{
			const int32 ID = x + y * Width;
			ItemGrid[ID] = NewItem;
		}
	}

	NewItem->SetParentGrid(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::UnRegisterItem(UItemWidget* NewItem)
{
	ItemList.Remove(NewItem);

	for (auto& Elm : ItemGrid)
	{
		if (Elm == NewItem)
			Elm = nullptr;
	}
}


//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::GetXYCellFromFloatingPoint(float XPosition, float YPosition, int32& CellX,
                                                      int32& CellY) const
{
	CellX = FMath::Clamp(static_cast<int32>(FMath::Floor(XPosition / TileSize)), 0, Width);
	CellY = FMath::Clamp(static_cast<int32>(FMath::Floor(YPosition / TileSize)), 0, Height);
}

//----------------------------------------------------------------------------------------------------------------------

int32 UInventoryGridWidget::GetTopLeftFromCellXY(int32 CellX, int32 CellY) const
{
	return CellY * Width + CellX;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UInventoryGridWidget::GetItemData() const
{
	if (ILootableInterface* Lootable = Cast<ILootableInterface>(ActorOwner); BagID == EBagSlot::LootPool && Lootable)
	{
		return Lootable->GetLootPoolComponent()->GetBagConst();
	}

	const IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	return PC->GetAllItemsInBag(BagID);
}
