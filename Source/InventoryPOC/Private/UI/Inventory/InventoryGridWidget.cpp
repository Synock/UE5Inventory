// Copyright 2021 Maximilien (Synock) Guislain


#include "UI/Inventory/InventoryGridWidget.h"

#include "Actors/LootableActor.h"
#include "InventoryPOC/InventoryPOCCharacter.h"


UInventoryGridWidget::UInventoryGridWidget(const FObjectInitializer& ObjectInitializer): UUserWidget(ObjectInitializer)
{
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::ResizeBagArea(int32 InputWidth, int32 InputHeight)
{
	UE_LOG(LogTemp, Log, TEXT("Resizing bag to  %d %d"), InputWidth, InputHeight);
	Width = InputWidth;
	Height = InputHeight;

	ItemGrid.Empty();
	ItemGrid.SetNumZeroed(Width * Height);

	ItemList.Empty();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::InitData(AActor* Owner, BagSlot InputBagSlot)
{
	ActorOwner = Owner;
	BagID = InputBagSlot;
	APawn* OwningPawn = GetOwningPlayerPawn();
	Character = Cast<AInventoryPOCCharacter>(OwningPawn);
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to init a bag Widget to a non player"));
	}
	check(Character);
	//these are internal bags of the inventory
	if (BagID == BagSlot::Pocket1 || BagID == BagSlot::Pocket2)
	{
		ResizeBagArea(3, 2);
		Character->GetInventoryDispatcher().AddDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else if (BagID == BagSlot::LootPool)
	{
		ResizeBagArea(8, 8);
		static_cast<ALootableActor*>(ActorOwner)->GetLootPoolDeletate().
												  AddDynamic(this, &UInventoryGridWidget::Refresh);
	}
	else
	{

		InventorySlot RelatedSlot = UFullInventoryComponent::GetInventorySlotFromBagSlot(BagID);
		FBareItem BagItem = Character->GetEquippedItem(RelatedSlot);

		ensure(BagItem.Bag);
		ResizeBagArea(BagItem.BagWidth, BagItem.BagHeight);
		MaximumBagSize = BagItem.BagSize;
		Character->GetInventoryDispatcher().AddDynamic(this, &UInventoryGridWidget::Refresh);
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
	const int32 modVal = TopLeft % GetWidth();
	const int32 divVal = TopLeft / GetWidth();
	PositionX = modVal * TileSize;
	PositionY = divVal * TileSize;
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
	const float Hsize = TileSize / 2.f;

	IsOnRight = ModX > Hsize;
	IsOnBottom = ModY > Hsize;
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
	const uint32 HalfWidth = static_cast<uint32>(std::floor(ItemWidth / 2.0f));
	const uint32 HalfHeight = static_cast<uint32>(std::floor(ItemHeight / 2.0f));

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
	for (auto& elm : ItemList)
	{
		if (elm->GetTopLeftID() == ItemData.TopLeftID && elm->GetReferencedItem().Key == ItemData.ItemID)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

UItemWidget* UInventoryGridWidget::GetLocalItem(const FMinimalItemStorage& ItemData, bool& Found) const
{
	Found = false;
	for (auto& elm : ItemList)
	{
		if (elm->GetTopLeftID() == ItemData.TopLeftID && elm->GetReferencedItem().Key == ItemData.ItemID)
		{
			Found = true;
			return elm;
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
	if(BagID == BagSlot::LootPool)
	{
		static_cast<ALootableActor*>(ActorOwner)->GetLootPoolDeletate().
												  RemoveAll(this);
	}
	else
	{
		Character->GetInventoryDispatcher().RemoveAll(this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryGridWidget::IsRoomAvailable(const FBareItem& ItemObject, int TopLeftIndex) const
{
	const int32 MaxIndex = Width * Height;

	if (TopLeftIndex < 0 || TopLeftIndex > MaxIndex)
		return false;

	if (ItemObject.Size > MaximumBagSize)
		return false;

	const int32 ItemWidth = ItemObject.Width;
	const int32 ItemHeight = ItemObject.Height;
	const int32 sx = TopLeftIndex % Width;
	const int32 sy = TopLeftIndex / Width;

	for (int y = sy; y < sy + ItemHeight; ++y)
	{
		for (int x = sx; x < sx + ItemWidth; ++x)
		{
			if (x > Width || x < 0)
			{
				return false;
			}
			if (y > Height || y < 0)
			{
				return false;
			}

			const int32 id = x + y * Width;
			if (id < 0 || id >= Width * Height)
				return false;

			if (ItemGrid[id] != nullptr) //only look for empty stuff
			{
				UE_LOG(LogTemp, Log, TEXT("Cannot copy because %d %d %d (%d) is not empty"), x, y, id);
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
	const int ItemWidth = NewItem->GetReferencedItem().Width;
	const int ItemHeight = NewItem->GetReferencedItem().Height;
	int32 sx = TopLeft % Width;
	int32 sy = TopLeft / Width;

	for (int y = sy; y < sy + ItemHeight; ++y)
	{
		for (int x = sx; x < sx + ItemWidth; ++x)
		{
			const int32 id = x + y * Width;
			ItemGrid[id] = NewItem;
			UE_LOG(LogTemp, Log, TEXT("Setting grid %d %d (%d) to something"), x, y, id);
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

void UInventoryGridWidget::PurgeItems()
{
	for (int i = ItemList.Num() - 1; i >= 0; --i)
	{
		auto Item = ItemList[i];
		if (!ValidItemList.Contains(Item))
		{
			//remove from parent
			Item->RemoveFromParent();

			//remove from grid
			for (auto& Elm : ItemGrid)
			{
				if (Elm == Item)
					Elm = nullptr;
			}

			//remove from list;
			ItemList.RemoveAt(i);
		}
	}
	ValidItemList.Empty();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryGridWidget::GetXYCellFromFloatingPoint(float XPosition, float YPosition, int32& CellX,
                                                      int32& CellY) const
{
	CellX = FMath::Clamp(static_cast<int32>(std::floor(XPosition / TileSize)), 0, Width);
	CellY = FMath::Clamp(static_cast<int32>(std::floor(YPosition / TileSize)), 0, Height);
}

//----------------------------------------------------------------------------------------------------------------------

int32 UInventoryGridWidget::GetTopLeftFromCellXY(int32 CellX, int32 CellY) const
{
	return CellY * Width + CellX;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& UInventoryGridWidget::GetItemData() const
{
	check(Character);
	check(ActorOwner);

	//self inventory
	if (Character == ActorOwner)
		return Character->GetAllItemsInBag(BagID);

	if (BagID == BagSlot::LootPool)
	{
		ALootableActor* LootableActor = Cast<ALootableActor>(ActorOwner);
		check(LootableActor);
		if (LootableActor)
		{
			return LootableActor->GetLootPool()->GetBagConst();
		}
	}

	//we are basically fucked if we end up here
	return Character->GetAllItemsInBag(BagSlot::Unknown);
}
