// Copyright 2023 Maximilien (Synock) Guislain


#include "UI/Keyring/KeyringWidget.h"

#include "InventoryUtilities.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/KeyringComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemKey.h"
#include "UI/ItemWidget.h"
#include "UI/Keyring/KeyLineWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UKeyringWidget::InternalSetup()
{
	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

	if (!PlayerInterface)
		return;

	if(PlayerInterface->GetKeyring())
		PlayerInterface->GetKeyring()->KeyringChangedDelegate.AddUniqueDynamic(this, &UKeyringWidget::RefreshList);
}

//----------------------------------------------------------------------------------------------------------------------

void UKeyringWidget::RefreshList()
{
	ClearList();

	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

	if (!PlayerInterface)
		return;

	UKeyringComponent* KeyringComponent = PlayerInterface->GetKeyring();

	if (!KeyringComponent)
		return;

	for (auto& PossessedKey : KeyringComponent->GetAllPossessedKeys())
	{
		FkeyLineDataStruct KeyData;
		KeyData.ItemId = PossessedKey.ItemId;
		KeyData.KeyId = PossessedKey.KeyId;

		UInventoryItemBase* InventoryItemBase = UInventoryUtilities::GetItemFromID(KeyData.ItemId, GetWorld());

		if (!InventoryItemBase)
			continue;

		const UInventoryItemKey* KeyItem = Cast<UInventoryItemKey>(InventoryItemBase);

		if (!KeyItem)
			continue;

		KeyData.Name = KeyItem->Name;
		KeyData.KeyIcon = KeyItem->Icon;

		AddKey(KeyData);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UKeyringWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                  UDragDropOperation* InOperation)
{
	if (const UItemWidget* DroppedItemWidget = Cast<UItemWidget>(InOperation->Payload))
	{
		if (!DroppedItemWidget->IsBelongingToSelf())
			return false;

		const UInventoryItemKey* KeyItem = Cast<UInventoryItemKey>(DroppedItemWidget->GetReferencedItem());
		if (KeyItem && KeyItem->KeyID > 0)
		{
			IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

			if (!PlayerInterface)
				return false;

			PlayerInterface->PlayerAddKeyFromInventory(DroppedItemWidget->GetTopLeftID(), DroppedItemWidget->GetBagID(),
			                                           DroppedItemWidget->GetReferencedItem()->ItemID);
			return true;
		}
		return false;
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
