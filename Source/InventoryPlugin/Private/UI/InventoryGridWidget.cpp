#include "UI/InventoryGridWidget.h"

#include "UI/InventoryGridGeometry.h"

#include "InventoryPlugin.h"
#include "BagStorage.h"
#include "InventoryUtilities.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BankComponent.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/InventoryComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/LootPoolComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/LootableInterface.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/Interfaces/InventoryItemAmmoBagInterface.h"
#include "Items/Interfaces/InventoryItemAmmoInterface.h"
#include "UI/PendingDeliveryDragDropOperation.h"

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
	if (!CanAcceptDrop)
		return false;

	return !IsItself(IncomingItem, DraggedItemTopLeftID) && IsRoomAvailable(
		IncomingItem->GetReferencedItem(), DraggedItemTopLeftID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::HandleItemDrop(UItemWidget* IncomingItem)
{
	if (!IncomingItem || !IncomingItem->GetReferencedItem())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("HandleItemDrop: Invalid incoming item"));
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
		UE_LOG(LogInventoryPlugin, Error, TEXT("HandleItemDrop: Failed to get InventoryPlayerInterface"));
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

void UInventoryGridWidget::AddItemWidgetToGrid(UWidget* Content, int32 TopLeft)
{
	if (!GridCanvasPanel || !Content)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("AddItemWidgetToGrid: Null parameter - GridCanvasPanel=%s, Content=%s"),
			GridCanvasPanel ? TEXT("Valid") : TEXT("Null"),
			Content ? TEXT("Valid") : TEXT("Null"));
		return;
	}

	// Validate TopLeft index
	if (!IsValidGridIndex(TopLeft))
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("AddItemWidgetToGrid: Invalid TopLeft index %d"), TopLeft);
		return;
	}

	UCanvasPanelSlot* PanelSlot = GridCanvasPanel->AddChildToCanvas(Content);
	if (!PanelSlot)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("AddItemWidgetToGrid: Failed to create canvas panel slot"));
		return;
	}

	const float XPos = (TopLeft % Width) * TileSize;
	const float YPos = (TopLeft / Width) * TileSize;
	PanelSlot->SetAutoSize(true);
	PanelSlot->SetPosition(FVector2D(XPos, YPos));
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::CreateNewItem(const FMinimalItemStorage& ItemStorage)
{
	if (!GridCanvasPanel)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("CreateNewItem: GridCanvasPanel is null"));
		return;
	}

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetWorld());
	if (!Item)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("CreateNewItem: Failed to resolve item with ID %d — skipping widget creation"), ItemStorage.ItemID);
		return;
	}

	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(GetOwningPlayer(), ItemWidgetClass);
	if (!ItemWidget)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("CreateNewItem: Failed to create ItemWidget"));
		return;
	}

	ItemWidget->SetParentGrid(this);
	ItemWidget->InitData(Item, GetOwningPlayerPawn(), TileSize, ItemStorage.TopLeftID, BagID, EEquipmentSlot::Unknown, ItemStorage.Durability);

	if (ItemStorage.bIsLocked)
	{
		ItemWidget->SetLocked(true);
	}

	AddItemWidgetToGrid(ItemWidget, ItemStorage.TopLeftID);
	RegisterNewItem(ItemStorage.TopLeftID, ItemWidget);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::FullRefresh()
{
	if (!GridCanvasPanel)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("FullRefresh: GridCanvasPanel is null"));
		return;
	}

	// O(n) reset: clear all three data structures directly without per-item teardown.
	ItemList.Reset();
	ItemGrid.Init(nullptr, Width * Height);
	ClearItemLookupMap();
	GridCanvasPanel->ClearChildren();

	// Repopulate from authoritative data source.
	const TArray<FMinimalItemStorage>& ItemData = GetItemData();
	for (const FMinimalItemStorage& NewItem : ItemData)
	{
		CreateNewItem(NewItem);
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

void UInventoryGridWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Build grid preview in the UMG Designer so the canvas reflects
	// the configured Width / Height / TileSize before any runtime data arrives.
	// InitData() will overwrite these with actual bag dimensions at runtime.
	ItemGrid.Init(nullptr, Width * Height);
	Lines.Empty();
	CreateLineSegments();
	SetUISize(Width * TileSize, Height * TileSize);
}

void UInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Auto-init when BagID is pre-configured in Blueprint (EditDefaultsOnly) for all player-owned
	// bag slots. LootPool is excluded because it requires an explicit world-actor owner — call
	// InitData(LootableActor, EBagSlot::LootPool) manually in that case.
	if (BagID != EBagSlot::Unknown && BagID != EBagSlot::LootPool && ActorOwner == nullptr)
	{
		AActor* OwningPawn = GetOwningPlayerPawn();
		if (OwningPawn)
		{
			InitData(OwningPawn, BagID);
		}
		else
		{
			UE_LOG(LogInventoryPlugin, Warning,
				TEXT("UInventoryGridWidget::NativeConstruct: BagID=%d is preset but GetOwningPlayerPawn() is null. "
				     "Call InitData() explicitly once the player pawn is available."),
				static_cast<int32>(BagID));
		}
	}
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

	if (!PC)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("InitData: Failed to get InventoryPlayerInterface from player controller — bag %d will not respond to inventory changes"), static_cast<int32>(InputBagSlot));
		return;
	}

	UInventoryComponent* InventoryComponent = PC->GetInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogInventoryPlugin, Warning,
		       TEXT("InitData: InventoryComponent is not ready for bag %d — initialization deferred"),
		       static_cast<int32>(InputBagSlot));
		return;
	}

	InventoryComponent->FullInventoryDispatcher.AddUniqueDynamic(
		this, &UInventoryGridWidget::ResetTransaction);

	//these are internal bags of the inventory
	if (BagID == EBagSlot::Pocket1 || BagID == EBagSlot::Pocket2)
	{
		// Prefer explicit overrides, then read actual dimensions from the component so
		// any BagSet() customisation is honored. Fall back to 3×2 only if the component
		// hasn't been initialized yet (shouldn't happen in normal flow).
		int32 ActualWidth = InputWidth > 0 ? InputWidth : 3;
		int32 ActualHeight = InputHeight > 0 ? InputHeight : 2;

		if (InputWidth <= 0 || InputHeight <= 0)
		{
			if (const UBagStorage* BagData = InventoryComponent->GetRelatedBagConst(BagID))
			{
				ActualWidth = BagData->GetWidth();
				ActualHeight = BagData->GetHeight();
			}
		}

		ResizeBagArea(ActualWidth, ActualHeight);
		InventoryComponent->FullInventoryDispatcher.AddUniqueDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else if (BagID == EBagSlot::LootPool)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 8, InputHeight > 0 ? InputHeight : 8);

		if (ILootableInterface* LootableActor = Cast<ILootableInterface>(ActorOwner))
		{
			LootableActor->GetLootPoolDelegate().AddUniqueDynamic(this, &UInventoryGridWidget::Refresh);
			LootableActor->GetLootPoolDelegate().AddUniqueDynamic(this, &UInventoryGridWidget::ResetTransaction);
		}
		else
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("InitData: ActorOwner does not implement ILootableInterface for LootPool"));
		}
	}
	else if (BagID == EBagSlot::BankPool)
	{
		ResizeBagArea(InputWidth > 0 ? InputWidth : 8, InputHeight > 0 ? InputHeight : 16);
		PC->GetBankComponent()->BankPoolDispatcher.AddUniqueDynamic(this, &UInventoryGridWidget::Refresh);
		PC->GetBankComponent()->BankPoolDispatcher.AddUniqueDynamic(this, &UInventoryGridWidget::ResetTransaction);
	}
	else
	{
		const EEquipmentSlot RelatedSlot = UInventoryComponent::GetInventorySlotFromBagSlot(BagID);

		const IInventoryItemBagInterface* BagItem = Cast<IInventoryItemBagInterface>(
			PC->GetEquipmentForInventory()->GetEquippedItem(RelatedSlot));

		if (!BagItem)
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("InitData: No bag item found for equipment slot %d — grid defaults to 4x4"), static_cast<int32>(RelatedSlot));
			ResizeBagArea(4, 4);
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

		InventoryComponent->FullInventoryDispatcher.AddUniqueDynamic(this, &UInventoryGridWidget::Refresh);
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
	// ItemLookupMap is the single source of truth (incrementally maintained by
	// RegisterNewItem / UnRegisterItem). Find returning nullptr IS the not-found answer.
	const int64 Key = MakeItemKey(ItemData.TopLeftID, ItemData.ItemID);
	const TObjectPtr<UItemWidget>* FoundItem = ItemLookupMap.Find(Key);
	return FoundItem != nullptr && *FoundItem != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetLocalItem(const FMinimalItemStorage& ItemData, bool& Found) const
{
	Found = false;

	// ItemLookupMap is the single source of truth (incrementally maintained by
	// RegisterNewItem / UnRegisterItem). Find returning nullptr IS the not-found answer.
	const int64 Key = MakeItemKey(ItemData.TopLeftID, ItemData.ItemID);
	if (const TObjectPtr<UItemWidget>* FoundItem = ItemLookupMap.Find(Key))
	{
		if (*FoundItem != nullptr)
		{
			Found = true;
			return *FoundItem;
		}
	}


	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetItemWidgetAtPosition(int32 TopLeft) const
{
	// O(1): ItemGrid is indexed by cell position and populated for every cell an item occupies,
	// so this correctly returns the widget for any occupied cell — not just anchor (top-left) cells.
	if (!IsValidGridIndex(TopLeft))
		return nullptr;

	return ItemGrid[TopLeft];
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
		// ResetTransaction was bound to FullInventoryDispatcher for all bag types in InitData;
		// clean it up here too to avoid a dangling delegate after the widget is destroyed.
		if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		{
			if (UInventoryComponent* IC = PC->GetInventoryComponent())
				IC->FullInventoryDispatcher.RemoveAll(this);
		}
	}
	else
	{
		if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		{
			if (UInventoryComponent* IC = PC->GetInventoryComponent())
				IC->FullInventoryDispatcher.RemoveAll(this);
		}
	}

	ClearItemLookupMap();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsRoomAvailable(const UInventoryItemBase* ItemObject, int32 TopLeftIndex) const
{
	if (!ItemObject)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("IsRoomAvailable: ItemObject is null"));
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
		UE_LOG(LogInventoryPlugin, Warning, TEXT("IsRoomAvailable: Invalid item dimensions %dx%d"), ItemWidth, ItemHeight);
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
				UE_LOG(LogInventoryPlugin, Error, TEXT("IsRoomAvailable: Invalid grid index %d (Grid size: %d)"), ID, ItemGrid.Num());
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

bool UInventoryGridWidget::IsItself(UItemWidget* IncomingItem, int32 TopLeftIndex) const
{
	return IncomingItem->GetTopLeftID() == TopLeftIndex && IncomingItem->GetBagID() == BagID;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::RegisterNewItem(int32 TopLeft, UItemWidget* NewItem)
{
	if (!NewItem || !NewItem->GetReferencedItem())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("RegisterNewItem: Invalid item widget"));
		return;
	}

	const UInventoryItemBase* ItemBase = NewItem->GetReferencedItem();
	const int32 ItemWidth = ItemBase->Width;
	const int32 ItemHeight = ItemBase->Height;

	// Validate bounds before registration
	if (!IsWithinGridBounds(TopLeft, ItemWidth, ItemHeight))
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("RegisterNewItem: Item at %d with size %dx%d exceeds grid bounds"),
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
				UE_LOG(LogInventoryPlugin, Error, TEXT("RegisterNewItem: Invalid grid index %d during registration"), ID);
			}
		}
	}

	NewItem->SetParentGrid(this);

	// Collision-safe int64 key.
	const int64 Key = MakeItemKey(TopLeft, ItemBase->ItemID);
	ItemLookupMap.Add(Key, NewItem);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::UnRegisterItem(UItemWidget* NewItem)
{
	if (!NewItem)
		return;

	ItemList.Remove(NewItem);

	// Clear only the cells this item occupies using its stored position and dimensions.
	// This is O(ItemWidth * ItemHeight) instead of the previous O(Width * Height) full scan.
	if (const UInventoryItemBase* ItemBase = NewItem->GetReferencedItem())
	{
		const int32 TopLeft = NewItem->GetTopLeftID();
		if (IsWithinGridBounds(TopLeft, ItemBase->Width, ItemBase->Height))
		{
			const int32 Sx = TopLeft % Width;
			const int32 Sy = TopLeft / Width;
			for (int32 y = Sy; y < Sy + ItemBase->Height; ++y)
			{
				for (int32 x = Sx; x < Sx + ItemBase->Width; ++x)
				{
					const int32 ID = x + y * Width;
					if (ItemGrid.IsValidIndex(ID) && ItemGrid[ID] == NewItem)
						ItemGrid[ID] = nullptr;
				}
			}
		}

		const int64 Key = MakeItemKey(TopLeft, ItemBase->ItemID);
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
		UE_LOG(LogInventoryPlugin, Warning, TEXT("GetXYCellFromFloatingPoint: Invalid TileSize %.2f"), TileSize);
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

//----------------------------------------------------------------------------------------------------------------------
// BlueprintNativeEvent implementations
//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::Refresh_Implementation()
{
	FullRefresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::SetUISize_Implementation(float InputWidth, float InputHeight)
{
	if (!GridBorder)
		return;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GridBorder->Slot))
	{
		CanvasSlot->SetSize(FVector2D(InputWidth, InputHeight));
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Native drag-drop overrides
//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (InOperation && (Cast<UItemWidget>(InOperation->Payload) ||
		Cast<UPendingDeliveryDragDropOperation>(InOperation)))
	{
		DrawDropLocation = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	DrawDropLocation = false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!CanAcceptDrop)
	{
		DrawDropLocation = false;
		return false;
	}

	if (InOperation)
	{
		if (UPendingDeliveryDragDropOperation* DeliveryOperation =
			Cast<UPendingDeliveryDragDropOperation>(InOperation))
		{
			const UInventoryItemBase* DeliveryItem = DeliveryOperation->Item;
			const bool bCanPlace = DeliveryItem && IsValidGridIndex(DraggedItemTopLeftID) &&
				IsRoomAvailable(DeliveryItem, DraggedItemTopLeftID);
			if (bCanPlace)
			{
				if (IInventoryPlayerInterface* Player = GetInventoryPlayerInterface())
					if (UInventoryNetComponent* Net = Player->GetInventoryNetComponent())
						Net->Server_ClaimPendingDeliveryAt(DeliveryOperation->DeliveryId,
							FInventoryDeliveryDestination::MakeBag(BagID, DraggedItemTopLeftID));
			}
			DraggedItemTopLeftID = INDEX_NONE;
			DrawDropLocation = false;
			return bCanPlace;
		}
		if (UItemWidget* Item = Cast<UItemWidget>(InOperation->Payload))
		{
			return HandleItemDrop(Item);
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (InOperation)
	{
		const FGeometry* CanvasGeometry = GridCanvasPanel ? &GridCanvasPanel->GetCachedGeometry() : nullptr;
		const FGeometry* BorderGeometry = GridBorder ? &GridBorder->GetCachedGeometry() : nullptr;
		const FGeometry* GridGeometry = InventoryGridGeometry::ResolveGridGeometry(
			CanvasGeometry, BorderGeometry);
		const FVector2D LocalPos = InventoryGridGeometry::AbsoluteToGridLocal(
			InGeometry, GridGeometry, InDragDropEvent.GetScreenSpacePosition(), GridVisualInset);

		if (const UPendingDeliveryDragDropOperation* DeliveryOperation =
			Cast<UPendingDeliveryDragDropOperation>(InOperation))
		{
			if (!DeliveryOperation->Item)
				return false;
			DraggedItemTopLeftID = GetActualTopLeftCorner(static_cast<float>(LocalPos.X),
				static_cast<float>(LocalPos.Y), DeliveryOperation->Item->Width, DeliveryOperation->Item->Height);
			return IsValidGridIndex(DraggedItemTopLeftID);
		}
		if (UItemWidget* Item = Cast<UItemWidget>(InOperation->Payload))
		{
			return UpdateDraggedItemTopLeft(Item, static_cast<float>(LocalPos.X), static_cast<float>(LocalPos.Y));
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
// NativePaint — draws grid lines and drop-highlight overlay
//----------------------------------------------------------------------------------------------------------------------

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 MaxLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
	                                            InWidgetStyle, bParentEnabled);

	if (!GridBorder)
		return MaxLayerId;

	FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, MaxLayerId, InWidgetStyle, bParentEnabled);

	const FGeometry* CanvasGeometry = GridCanvasPanel ? &GridCanvasPanel->GetCachedGeometry() : nullptr;
	const FGeometry* BorderGeometry = &GridBorder->GetCachedGeometry();
	const FGeometry* GridGeometry = InventoryGridGeometry::ResolveGridGeometry(
		CanvasGeometry, BorderGeometry);
	const FVector2D LocalTopLeft = InventoryGridGeometry::GridOriginInWidgetLocal(
		AllottedGeometry, GridGeometry, GridVisualInset);

	for (const FInventoryLine& Line : Lines)
	{
		UWidgetBlueprintLibrary::DrawLine(Context,
			LocalTopLeft + Line.Begin,
			LocalTopLeft + Line.End,
			GridLineColor,
			/*bAntiAlias=*/false,
			/*Thickness=*/1.0f);
	}

	DrawBackground(Context, LocalTopLeft);

	return FMath::Max(MaxLayerId, Context.MaxLayer);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::DrawBackground(FPaintContext& Context, const FVector2D& LocalTopLeft) const
{
	if (!DropHighlightBrush || !DrawDropLocation || DraggedItemTopLeftID < 0)
		return;

	if (!UWidgetBlueprintLibrary::IsDragDropping())
		return;

	UDragDropOperation* DragOp = UWidgetBlueprintLibrary::GetDragDroppingContent();
	if (!DragOp)
		return;

	const UItemWidget* ItemWidget = Cast<UItemWidget>(DragOp->Payload);
	const UPendingDeliveryDragDropOperation* DeliveryOperation = Cast<UPendingDeliveryDragDropOperation>(DragOp);
	const UInventoryItemBase* Item = ItemWidget ? ItemWidget->GetReferencedItem() :
		(DeliveryOperation ? DeliveryOperation->Item.Get() : nullptr);
	if (!Item)
		return;

	const float ColX = static_cast<float>(DraggedItemTopLeftID % Width) * TileSize;
	const float RowY = static_cast<float>(DraggedItemTopLeftID / Width) * TileSize;
	const FVector2D DrawPosition = LocalTopLeft + FVector2D(ColX, RowY);
	const FVector2D DrawSize(Item->Width * TileSize, Item->Height * TileSize);

	const bool bCanPlace = CanAcceptDrop && IsRoomAvailable(Item, DraggedItemTopLeftID);
	const FLinearColor& Tint = bCanPlace ? DropColorValid : DropColorInvalid;

	UWidgetBlueprintLibrary::DrawBox(Context, DrawPosition, DrawSize, DropHighlightBrush, Tint);
}

