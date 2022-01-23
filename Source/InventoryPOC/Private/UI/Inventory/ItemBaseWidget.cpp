// Copyright 2021 Maximilien (Synock) Guislain


#include "UI/Inventory/ItemBaseWidget.h"

void UItemBaseWidget::InitBareData(const FBareItem& InputItem, AActor* InputOwner, float InputTileSize)
{
	Item = InputItem;
	Owner = InputOwner;
	TileSize = InputTileSize;
}

//----------------------------------------------------------------------------------------------------------------------

void UItemBaseWidget::StopDrag()
{
}
