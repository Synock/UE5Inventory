#include "UI/InventoryGridWidget.h"

#include "InventoryUtilities.h"
#include "Components/BankComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/LootPoolComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/LootableInterface.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/Interfaces/InventoryItemAmmoBagInterface.h"
#include "Items/Interfaces/InventoryItemAmmoInterface.h"

//----------------------------------------------------------------------------------------------------------------------
// Validation Helper Methods
//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsValidGridIndex(int32 Index) const
{
	return Index >= 0 && Index < Width * Height;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsValidCoordinate(int32 X, int32 Y) const
{
	return X >= 0 && X < Width && Y >= 0 && Y < Height;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsWithinGridBounds(int32 TopLeftIndex, int32 ItemWidth, int32 ItemHeight) const
{
	if (!IsValidGridIndex(TopLeftIndex))
		return false;

	const int32 StartX = TopLeftIndex % Width;
	const int32 StartY = TopLeftIndex / Width;

	// Check for integer overflow
	if (StartX > INT32_MAX - ItemWidth || StartY > INT32_MAX - ItemHeight)
		return false;

	const int32 EndX = StartX + ItemWidth;
	const int32 EndY = StartY + ItemHeight;

	return EndX <= Width && EndY <= Height;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::ClearItemLookupMap()
{
	ItemLookupMap.Empty();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::RebuildItemLookupMap()
{
	ItemLookupMap.Empty(ItemList.Num());
	for (UItemWidget* Item : ItemList)
	{
		if (Item && Item->GetReferencedItem())
		{
			const int32 Key = Item->GetTopLeftID() * 100000 + Item->GetReferencedItem()->ItemID;
			ItemLookupMap.Add(Key, Item);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Main Implementation
//----------------------------------------------------------------------------------------------------------------------

FVector2D UInventoryGridWidget::GetItemScreenFootprint(UItemWidget* Item) const
{
	if (!Item || !Item->GetReferencedItem())
		return FVector2D::ZeroVector;

	const UInventoryItemBase* ItemBase = Item->GetReferencedItem();
	return FVector2D(ItemBase->Width * TileSize, ItemBase->Height * TileSize);
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
	if (!IncomingItem || !IncomingItem->GetReferencedItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleItemDrop: Invalid incoming item"));
		return false;
	}

	if (!CanProcessItemDrop(IncomingItem))
	{
		DraggedItemTopLeftID = INDEX_NONE;
		DrawDropLocation = false;
		return false;
	}

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleItemDrop: Failed to get InventoryPlayerInterface"));
		DraggedItemTopLeftID = INDEX_NONE;
		DrawDropLocation = false;
		return false;
	}

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

	DraggedItemTopLeftID = INDEX_NONE;
	DrawDropLocation = false;
	return true;
}

bool UInventoryGridWidget::UpdateDraggedItemTopLeft(UItemWidget* IncomingItem, float X, float Y)
{
	if (!IncomingItem || !IncomingItem->GetReferencedItem())
		return false;

	const UInventoryItemBase* Item = IncomingItem->GetReferencedItem();
	DraggedItemTopLeftID = GetActualTopLeftCorner(X, Y, Item->Width, Item->Height);

	return IsValidGridIndex(DraggedItemTopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::AddItemWidgetToGrid(UCanvasPanel* GridCanvasPanel, UWidget* Content, int32 TopLeft)
{
	if (!GridCanvasPanel || !Content)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemWidgetToGrid: Null parameter - GridCanvasPanel=%s, Content=%s"),
			GridCanvasPanel ? TEXT("Valid") : TEXT("Null"),
			Content ? TEXT("Valid") : TEXT("Null"));
		return;
	}

	// Validate TopLeft index
	if (!IsValidGridIndex(TopLeft))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemWidgetToGrid: Invalid TopLeft index %d"), TopLeft);
		return;
	}

	UCanvasPanelSlot* PanelSlot = GridCanvasPanel->AddChildToCanvas(Content);
	if (!PanelSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("AddItemWidgetToGrid: Failed to create canvas panel slot"));
		return;
	}

	const float XPos = (TopLeft % Width) * TileSize;
	const float YPos = (TopLeft / Width) * TileSize;
	PanelSlot->SetAutoSize(true);
	PanelSlot->SetPosition(FVector2D(XPos, YPos));
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::CreateNewItem(UCanvasPanel* GridCanvasPanel, const FMinimalItemStorage& ItemStorage)
{
	if (!GridCanvasPanel)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateNewItem: GridCanvasPanel is null"));
		return;
	}

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetWorld());
	if (!ensureMsgf(Item, TEXT("CreateNewItem: Failed to get item with ID %d"), ItemStorage.ItemID))
	{
		return;
	}

	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(GetOwningPlayer(), ItemWidgetClass);
	if (!ItemWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateNewItem: Failed to create ItemWidget"));
		return;
	}

	ItemWidget->SetParentGrid(this);
	ItemWidget->InitData(Item, GetOwningPlayerPawn(), TileSize, ItemStorage.TopLeftID, BagID, EEquipmentSlot::Unknown, ItemStorage.Durability);

	if (ItemStorage.bIsLocked)
	{
		ItemWidget->SetLocked(true);
	}

	AddItemWidgetToGrid(GridCanvasPanel, ItemWidget, ItemStorage.TopLeftID);
	RegisterNewItem(ItemStorage.TopLeftID, ItemWidget);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::FullRefresh(UCanvasPanel* GridCanvasPanel)
{
	if (!GridCanvasPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("FullRefresh: GridCanvasPanel is null"));
		return;
	}

	// Unregister all items (iterating backwards for safe removal)
	for (int32 Id = ItemList.Num() - 1; Id >= 0; --Id)
	{
		if (ItemList.IsValidIndex(Id))
		{
			UnRegisterItem(ItemList[Id]);
		}
	}

	GridCanvasPanel->ClearChildren();
	ClearItemLookupMap();

	// Create new items
	const TArray<FMinimalItemStorage>& ItemData = GetItemData();
	for (const FMinimalItemStorage& NewItem : ItemData)
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

	if (!ensureMsgf(PC, TEXT("InitData: Failed to get InventoryPlayerInterface from player controller")))
	{
		return;
	}

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

		if (ILootableInterface* LootableActor = Cast<ILootableInterface>(ActorOwner))
		{
			LootableActor->GetLootPoolDelegate().AddDynamic(this, &UInventoryGridWidget::Refresh);
			LootableActor->GetLootPoolDelegate().AddDynamic(this, &UInventoryGridWidget::ResetTransaction);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InitData: ActorOwner does not implement ILootableInterface for LootPool"));
		}
	}
	else if (BagID == EBagSlot::BankPool)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 8, InputHeight > 0 ? InputHeight : 16);
		PC->GetBankComponent()->BankPoolDispatcher.AddDynamic(this, &UInventoryGridWidget::Refresh);
		PC->GetBankComponent()->BankPoolDispatcher.AddDynamic(this, &UInventoryGridWidget::ResetTransaction);
	}
	else
	{
		const EEquipmentSlot RelatedSlot = UInventoryComponent::GetInventorySlotFromBagSlot(BagID);

		const IInventoryItemBagInterface* BagItem = Cast<IInventoryItemBagInterface>(
			PC->GetEquipmentForInventory()->GetEquippedItem(RelatedSlot));

		if (!ensureMsgf(BagItem, TEXT("InitData: Failed to get bag item for slot %d"), static_cast<int32>(RelatedSlot)))
		{
			ResizeBagArea(4, 4); // Fallback to reasonable default
		}
		else
		{
			ResizeBagArea(BagItem->GetBagWidth(), BagItem->GetBagHeight());
			MaximumBagSize = BagItem->GetBagSize();
			if (const IInventoryItemAmmoBagInterface* AmmoBag = Cast<IInventoryItemAmmoBagInterface>(BagItem))
			{
				AmmoTypeLimiter = AmmoBag->GetAmmoType();
			}
		}

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
	Lines.Empty();
	Lines.Reserve((Width + 1) + (Height + 1)); // Pre-allocate for efficiency

	{
		const float Y = TileSize * Height;
		for (int32 i = 0; i <= Width; ++i)
		{
			const float X = TileSize * i;
			Lines.Add(FInventoryLine(FVector2D(X, 0.f), FVector2D(X, Y)));
		}
	}

	{
		const float X = TileSize * Width;
		for (int32 i = 0; i <= Height; ++i)
		{
			const float Y = TileSize * i;
			Lines.Add(FInventoryLine(FVector2D(0.f, Y), FVector2D(X, Y)));
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
	// Clamp input positions to valid ranges to prevent overflow
	XPosition = FMath::Clamp(XPosition, 0.0f, Width * TileSize);
	YPosition = FMath::Clamp(YPosition, 0.0f, Height * TileSize);

	int32 CellX = 0;
	int32 CellY = 0;
	GetXYCellFromFloatingPoint(XPosition, YPosition, CellX, CellY);

	//easy case where the mouse cursor is on the only cell
	if (ItemWidth == 1 && ItemHeight == 1)
	{
		return GetTopLeftFromCellXY(CellX, CellY);
	}

	//annoying case where we have bigger objects
	const int32 HalfWidth = FMath::FloorToInt32(ItemWidth / 2.0f);
	const int32 HalfHeight = FMath::FloorToInt32(ItemHeight / 2.0f);

	//find out where the mouse is in the current cell
	bool IsOnRight = false;
	bool IsOnBottom = false;
	MousePositionInTile(XPosition, YPosition, IsOnRight, IsOnBottom);

	//by default item expand by half its size in each direction
	int32 ReachX = HalfWidth;
	int32 ReachY = HalfHeight;

	//however if we are on the right we will favor a right placement, changing the TopLeft
	if (IsOnRight)
		ReachX = FMath::Max(0, ReachX - 1);

	if (IsOnBottom)
		ReachY = FMath::Max(0, ReachY - 1);

	CellX = FMath::Clamp(CellX - ReachX, 0, Width - 1);
	CellY = FMath::Clamp(CellY - ReachY, 0, Height - 1);

	return GetTopLeftFromCellXY(CellX, CellY);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::HasItemLocally(const FMinimalItemStorage& ItemData) const
{
	// Use lookup map for O(1) search if available
	const int32 Key = ItemData.TopLeftID * 100000 + ItemData.ItemID;
	if (const TObjectPtr<UItemWidget>* FoundItem = ItemLookupMap.Find(Key))
	{
		return *FoundItem != nullptr;
	}

	// Fallback to linear search if map is not built
	for (const TObjectPtr<UItemWidget>& Elm : ItemList)
	{
		if (Elm && Elm->GetReferencedItem() &&
			Elm->GetTopLeftID() == ItemData.TopLeftID &&
			Elm->GetReferencedItem()->ItemID == ItemData.ItemID)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetLocalItem(const FMinimalItemStorage& ItemData, bool& Found) const
{
	Found = false;

	// Use lookup map for O(1) search if available
	const int32 Key = ItemData.TopLeftID * 100000 + ItemData.ItemID;
	if (const TObjectPtr<UItemWidget>* FoundItem = ItemLookupMap.Find(Key))
	{
		if (*FoundItem != nullptr)
		{
			Found = true;
			return *FoundItem;
		}
	}

	// Fallback to linear search if map is not built
	for (const TObjectPtr<UItemWidget>& Elm : ItemList)
	{
		if (Elm && Elm->GetReferencedItem() &&
			Elm->GetTopLeftID() == ItemData.TopLeftID &&
			Elm->GetReferencedItem()->ItemID == ItemData.ItemID)
		{
			Found = true;
			return Elm;
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetItemWidgetAtPosition(int32 TopLeft) const
{
	// Linear search through ItemList to find widget at TopLeft position
	for (const TObjectPtr<UItemWidget>& Item : ItemList)
	{
		if (Item && Item->GetTopLeftID() == TopLeft)
		{
			return Item;
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
		if (ILootableInterface* LootableActor = Cast<ILootableInterface>(ActorOwner))
		{
			LootableActor->GetLootPoolDelegate().RemoveAll(this);
		}
	}
	else
	{
		if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		{
			if (auto IC =  PC->GetInventoryComponent())
				IC->FullInventoryDispatcher.RemoveAll(this);
		}
	}

	ClearItemLookupMap();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsRoomAvailable(const UInventoryItemBase* ItemObject, int TopLeftIndex) const
{
	if (!ItemObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsRoomAvailable: ItemObject is null"));
		return false;
	}

	// Validate grid index
	if (!IsValidGridIndex(TopLeftIndex))
	{
		return false;
	}

	// Check item size constraint
	if (ItemObject->ItemSize > MaximumBagSize)
	{
		return false;
	}

	// Check ammo type limiter for specialized bags (quivers)
	if (AmmoTypeLimiter != EAmmoType::Unknown)
	{
		const IInventoryItemAmmoInterface* AmmoItem = Cast<IInventoryItemAmmoInterface>(ItemObject);
		if (!AmmoItem || AmmoItem->GetAmmoType() != AmmoTypeLimiter)
		{
			return false;
		}
	}

	const int32 ItemWidth = ItemObject->Width;
	const int32 ItemHeight = ItemObject->Height;

	// Validate item dimensions are positive
	if (ItemWidth <= 0 || ItemHeight <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsRoomAvailable: Invalid item dimensions %dx%d"), ItemWidth, ItemHeight);
		return false;
	}

	// Use helper to check bounds (includes overflow protection)
	if (!IsWithinGridBounds(TopLeftIndex, ItemWidth, ItemHeight))
	{
		return false;
	}

	const int32 sx = TopLeftIndex % Width;
	const int32 sy = TopLeftIndex / Width;

	// Check all cells that the item would occupy
	for (int32 y = sy; y < sy + ItemHeight; ++y)
	{
		for (int32 x = sx; x < sx + ItemWidth; ++x)
		{
			// Additional safety check (should already be validated by IsWithinGridBounds)
			if (!IsValidCoordinate(x, y))
			{
				return false;
			}

			const int32 ID = x + y * Width;

			// Bounds check for ItemGrid array access
			if (!ItemGrid.IsValidIndex(ID))
			{
				UE_LOG(LogTemp, Error, TEXT("IsRoomAvailable: Invalid grid index %d (Grid size: %d)"), ID, ItemGrid.Num());
				return false;
			}

			// Cell must be empty
			if (ItemGrid[ID] != nullptr)
			{
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
	if (!NewItem || !NewItem->GetReferencedItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterNewItem: Invalid item widget"));
		return;
	}

	const UInventoryItemBase* ItemBase = NewItem->GetReferencedItem();
	const int32 ItemWidth = ItemBase->Width;
	const int32 ItemHeight = ItemBase->Height;

	// Validate bounds before registration
	if (!IsWithinGridBounds(TopLeft, ItemWidth, ItemHeight))
	{
		UE_LOG(LogTemp, Error, TEXT("RegisterNewItem: Item at %d with size %dx%d exceeds grid bounds"),
			TopLeft, ItemWidth, ItemHeight);
		return;
	}

	// Register item in list
	ItemList.Add(NewItem);

	// Assign item to the grid map
	const int32 Sx = TopLeft % Width;
	const int32 Sy = TopLeft / Width;

	for (int32 y = Sy; y < Sy + ItemHeight; ++y)
	{
		for (int32 x = Sx; x < Sx + ItemWidth; ++x)
		{
			const int32 ID = x + y * Width;
			if (ItemGrid.IsValidIndex(ID))
			{
				ItemGrid[ID] = NewItem;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RegisterNewItem: Invalid grid index %d during registration"), ID);
			}
		}
	}

	NewItem->SetParentGrid(this);

	// Update lookup map for fast retrieval
	const int32 Key = TopLeft * 100000 + ItemBase->ItemID;
	ItemLookupMap.Add(Key, NewItem);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::UnRegisterItem(UItemWidget* NewItem)
{
	if (!NewItem)
	{
		return;
	}

	ItemList.Remove(NewItem);

	// Clear from grid
	for (TObjectPtr<UItemWidget>& Elm : ItemGrid)
	{
		if (Elm == NewItem)
		{
			Elm = nullptr;
		}
	}

	// Remove from lookup map
	if (NewItem->GetReferencedItem())
	{
		const int32 Key = NewItem->GetTopLeftID() * 100000 + NewItem->GetReferencedItem()->ItemID;
		ItemLookupMap.Remove(Key);
	}
}


//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::GetXYCellFromFloatingPoint(float XPosition, float YPosition, int32& CellX,
                                                      int32& CellY) const
{
	// Prevent divide-by-zero and ensure TileSize is valid
	if (TileSize <= 0.0f)
	{
		CellX = 0;
		CellY = 0;
		UE_LOG(LogTemp, Warning, TEXT("GetXYCellFromFloatingPoint: Invalid TileSize %.2f"), TileSize);
		return;
	}

	CellX = FMath::Clamp(FMath::FloorToInt32(XPosition / TileSize), 0, Width - 1);
	CellY = FMath::Clamp(FMath::FloorToInt32(YPosition / TileSize), 0, Height - 1);
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
	if (!PC)
	{
		static const TArray<FMinimalItemStorage> EmptyArray;
		return EmptyArray;
	}
	return PC->GetAllItemsInBag(BagID);
}

