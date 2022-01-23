// Copyright 2022 Maximilien (Synock) Guislain


#include "MainPlayerController.h"
#include "InventoryPOC/InventoryPOCCharacter.h"
#include "Actors/LootableActor.h"
#include "Actors/MerchantActor.h"

#include <Blueprint/UserWidget.h>
#include <Net/UnrealNetwork.h>

//----------------------------------------------------------------------------------------------------------------------
// HUD Functions
//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::CreateHUD()
{
	// Only create once
	if (UIHUDWidget)
		return;

	if (!UIHUDWidgetClass)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("%s() Missing UIHUDWidgetClass. Please fill in on the Blueprint of the PlayerController."),
		       *FString(__FUNCTION__));
		return;
	}

	// Only create a HUD for local player
	if (!IsLocalPlayerController())
		return;

	UIHUDWidget = CreateWidget<UHUDWidget>(this, UIHUDWidgetClass);
	UIHUDWidget->AddToViewport();
}

//----------------------------------------------------------------------------------------------------------------------

UHUDWidget* AMainPlayerController::GetMainHUD()
{
	return UIHUDWidget;
}

//----------------------------------------------------------------------------------------------------------------------
// General helper functions
//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::PlayerCanPutItemSomewhere(int64 ItemID) const
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	InventorySlot TriedSlot = InventorySlot::Unknown;
	BagSlot TriedBag = BagSlot::Unknown;
	int32 InTopLeft = -1;
	return Char->PlayerTryAutoLootFunction(ItemID, TriedSlot, InTopLeft, TriedBag);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::PlayerCanPayAmount(const FCoinValue& CoinValue) const
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	return FCoinValue::CanPayWithChange(Char->GetCoinAmount(), CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------
// Replication functions
//----------------------------------------------------------------------------------------------------------------------
void AMainPlayerController::OnRep_LootedActor()
{
	if (LootedActor)
	{
		UE_LOG(LogTemp, Log,
			   TEXT("OnRep_LootedActor %s"), *LootedActor->GetName());
		GetMainHUD()->DisplayLootScreen(LootedActor);
	}
	else
	{
		UE_LOG(LogTemp, Log,
			   TEXT("Clearing loot action"));
		GetMainHUD()->HideLootScreen();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::OnRep_MerchantActor()
{
	if (MerchantActor)
	{
		UE_LOG(LogTemp, Log,
			   TEXT("OnRep_MerchantActor %s"), *MerchantActor->GetName());
		GetMainHUD()->DisplayMerchantScreen(MerchantActor);
	}
	else
	{
		UE_LOG(LogTemp, Log,
			   TEXT("Clearing Merchant Action"));
		GetMainHUD()->HideMerchantScreen();
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::PlayerTryAutoEquip(int64 InItemId, InventorySlot& PossibleEquipment)
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	return Char->PlayerTryAutoEquip(InItemId, PossibleEquipment);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerAutoEquipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId)
{
	Server_PlayerAutoEquipItem(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------
// Loot related functions
//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::IsLooting() const
{
	return LootedActor != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::StopLooting()
{
	if (LootedActor)
		Server_StopLooting();

}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerAutoLootAll_Implementation()
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());

	TArray<FMinimalItemStorage> CopyArray = LootedActor->GetLootPool()->GetBagConst();

	InventorySlot TriedSlot = InventorySlot::Unknown;
	BagSlot TriedBag = BagSlot::Unknown;
	int32 InTopLeft = -1;

	TArray<InventorySlot> ForbiddenSlot;
	TArray<FMinimalItemStorage> ItemToAdd;

	for (const auto& Item : CopyArray)
	{
		if (!Char->PlayerTryAutoLootFunction(Item.ItemID, TriedSlot, InTopLeft, TriedBag))
			break;

		if (TriedSlot != InventorySlot::Unknown)
		{
			Server_PlayerEquipItemFromLoot(Item.ItemID, TriedSlot, Item.TopLeftID);
		}
		else if (TriedBag != BagSlot::Unknown)
		{
			Server_PlayerLootItem(InTopLeft, TriedBag, Item.ItemID, Item.TopLeftID);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerAutoLootItem(int64 InItemId, int32 OutTopLeft)
{
	InventorySlot TriedSlot = InventorySlot::Unknown;
	BagSlot TriedBag = BagSlot::Unknown;
	int32 InTopLeft = -1;

	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	if (Char->PlayerTryAutoLootFunction(InItemId, TriedSlot, InTopLeft, TriedBag))
	{
		if (TriedSlot != InventorySlot::Unknown)
		{
			Server_PlayerEquipItemFromLoot(InItemId, TriedSlot, OutTopLeft);
		}
		else if (TriedBag != BagSlot::Unknown)
		{
			Server_PlayerLootItem(InTopLeft, TriedBag, InItemId, OutTopLeft);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerLootItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft)
{
	Server_PlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerEquipItemFromLoot(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft)
{
	Server_PlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_PlayerEquipItemFromLoot_Implementation(int64 InItemId, InventorySlot InSlot,
                                                                          int32 OutTopLeft)
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	LootedActor->RemoveItem(OutTopLeft);
	Char->PlayerEquip(InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_PlayerEquipItemFromLoot_Validate(int64 InItemId, InventorySlot InSlot,
                                                                    int32 OutTopLeft)
{
	//more check needed
	const int64 ActualItemID = LootedActor->GetItemData(OutTopLeft);
	return ActualItemID == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_PlayerLootItem_Implementation(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
                                                                 int32 OutTopLeft)
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	LootedActor->RemoveItem(OutTopLeft);
	Char->PlayerAddItem(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_PlayerLootItem_Validate(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
                                                           int32 OutTopLeft)
{
	const int64 ActualItemID = LootedActor->GetItemData(OutTopLeft);
	return ActualItemID == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_StopLooting_Implementation()
{
	if (LootedActor)
	{
		LootedActor->StopLooting(static_cast<AInventoryPOCCharacter*>(GetPawn()));
		LootedActor = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_LootActor_Implementation(ALootableActor* InputLootedActor)
{
	if(!LootedActor)
	{
		LootedActor = InputLootedActor;
		LootedActor->StartLooting(static_cast<AInventoryPOCCharacter*>(GetPawn()));
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_LootActor_Validate(ALootableActor* InputLootedActor)
{
	if(LootedActor)
		return false;

	if (InputLootedActor == nullptr)
		return false;

	if (InputLootedActor->GetIsBeingLooted())
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant related functions
//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::IsTrading() const
{
	return MerchantActor != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::TryPresentSellItem(BagSlot OutSlot, int64 ItemId, int32 TopLeft)
{
	GetMainHUD()->TryPresentSellItem(OutSlot, ItemId, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::ResetSellItem()
{
	GetMainHUD()->ResetSellItem();
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerBuyFromMerchant(int64 ItemId, const FCoinValue& Price)
{
	Server_PlayerBuyFromMerchant(ItemId, Price);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_PlayerBuyFromMerchant_Implementation(int64 ItemId,
                                                                        const FCoinValue& Price)
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetCharacter());


	//this can happen during race condition if two players buy out the same item at (almost) the same time
	if (!MerchantActor->HasItem(ItemId))
		return;

	//Remove Coin from the player
	Char->PayCoinAmount(Price);

	//Remove item from the merchant if needed
	MerchantActor->RemoveItemAmountIfNeeded(ItemId);

	//Add item to the player
	InventorySlot TriedSlot = InventorySlot::Unknown;
	BagSlot TriedBag = BagSlot::Unknown;
	int32 InTopLeft = -1;
	if (Char->PlayerTryAutoLootFunction(ItemId, TriedSlot, InTopLeft, TriedBag))
	{
		if (TriedSlot != InventorySlot::Unknown)
		{
			Char->PlayerEquip(TriedSlot, ItemId);
		}
		else if (TriedBag != BagSlot::Unknown)
		{
			Char->PlayerAddItem(InTopLeft, TriedBag, ItemId);
		}
		else
			check(nullptr); //hopefully we never go here
	}

	//Add Coin to the merchant
	MerchantActor->ReceiveCoin(Price);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_PlayerBuyFromMerchant_Validate(int64 ItemId,
                                                                  const FCoinValue& Price)
{
	if (!MerchantActor)
		return false;

	if (!PlayerCanPayAmount(Price)) //Player must have the asked price
		return false;
	//Merchant needs to have the item somewhere (or not, see race condition)

	//Player must have enough room

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::PlayerSellToMerchant(BagSlot OutSlot, int64 ItemId, int32 TopLeft, const FCoinValue& Price)
{
	Server_PlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_PlayerSellToMerchant_Implementation(BagSlot OutSlot, int64 ItemId, int32 TopLeft,
                                                                       const FCoinValue& Price)
{
	if (!MerchantActor->CanPayAmount(Price))
		return;

	//Player item is removed
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	Char->PlayerRemoveItem(TopLeft, OutSlot);

	//Merchant cash is reduced
	MerchantActor->PayCoin(Price);

	//Merchant item is added
	MerchantActor->AddDynamicItem(ItemId);

	//Player cash is increased
	Char->ReceiveCoinAmount(Price);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_PlayerSellToMerchant_Validate(BagSlot OutSlot, int64 ItemId, int32 TopLeft,
                                                                 const FCoinValue& Price)
{
	if (!MerchantActor)
		return false;

	//race condition
	/*if (!MerchantActor->CanPayAmount(Price)) //Merchant must have the asked price
		return false;*/

	//Player needs to have the item
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	const int64 ActualItemID = Char->PlayerGetItem(TopLeft, OutSlot);

	if (ActualItemID != ItemId)
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_StopMerchantTrade_Implementation()
{
	if (MerchantActor)
		MerchantActor = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::Server_MerchantTrade_Implementation(AMerchantActor* InputMerchantActor)
{
	MerchantActor = InputMerchantActor;
}

//----------------------------------------------------------------------------------------------------------------------


void AMainPlayerController::Server_PlayerAutoEquipItem_Implementation(int32 InTopLeft, BagSlot InSlot, int64 InItemId)
{
	InventorySlot SlotToEquip;
	if (!PlayerTryAutoEquip(InItemId, SlotToEquip))
		return;

	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());

	Char->PlayerEquipItemFromInventory(InItemId, SlotToEquip, InTopLeft, InSlot);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMainPlayerController::Server_PlayerAutoEquipItem_Validate(int32 InTopLeft, BagSlot InSlot, int64 InItemId)
{
	AInventoryPOCCharacter* Char = static_cast<AInventoryPOCCharacter*>(GetPawn());
	const int64 ActualItemID = Char->PlayerGetItem(InTopLeft, InSlot);

	if (ActualItemID != InItemId)
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void AMainPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMainPlayerController, LootedActor, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMainPlayerController, MerchantActor, COND_OwnerOnly);
}
