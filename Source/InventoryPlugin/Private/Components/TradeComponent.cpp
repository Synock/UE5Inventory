// Copyright 2025 Maximilien (Synock) Guislain

#include "Components/TradeComponent.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/TradeInterface.h"
#include "Components/CoinComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

UTradeComponent::UTradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Create coin components for holding trade offers
	// CreateDefaultSubobject can only be called in constructor
	OurCoinOffer = CreateDefaultSubobject<UCoinComponent>(TEXT("OurCoinOffer"));
	if (OurCoinOffer)
	{
		OurCoinOffer->SetIsReplicated(true);
	}

	TheirCoinOffer = CreateDefaultSubobject<UCoinComponent>(TEXT("TheirCoinOffer"));
	if (TheirCoinOffer)
	{
		TheirCoinOffer->SetIsReplicated(true);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// If player disconnects while trading, safely cancel the trade and return items
	if (GetOwner()->HasAuthority() && bIsTrading)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeComponent: Player disconnected during trade, cancelling and returning items"));
		CancelTrade();
	}

	Super::EndPlay(EndPlayReason);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Only replicate to owner - trade state is private to each participant
	DOREPLIFETIME_CONDITION(UTradeComponent, TradePartner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTradeComponent, OurOffer, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTradeComponent, TheirOffer, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTradeComponent, bIsTrading, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTradeComponent, OurCoinOffer, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UTradeComponent, TheirCoinOffer, COND_OwnerOnly);
}

//----------------------------------------------------------------------------------------------------------------------
// Replication Callbacks
//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_TradePartner()
{
	OnTradeStateChanged.Broadcast();
	// If trading just started, open the trade UI
	if (bIsTrading)
	{
		// Get the player controller
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			// Open trade window via the HUD interface
			if (IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC))
			{
				if (IInventoryHUDInterface* HUDInterface = PlayerInterface->GetInventoryHUDInterface())
				{
					HUDInterface->Execute_OpenTradeWindow(PlayerInterface->GetInventoryHUDObject());
				}
			}
		}
	}
	else
	{
		// Trading ended, close the trade UI
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			if (IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC))
			{
				if (IInventoryHUDInterface* HUDInterface = PlayerInterface->GetInventoryHUDInterface())
				{
					HUDInterface->Execute_CloseTradeWindow(PlayerInterface->GetInventoryHUDObject());
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_OurOffer()
{
	// Update coin component display
	/*if (OurCoinOffer)
	{
		OurCoinOffer->ClearPurse();
		OurCoinOffer->AddCoins(OurOffer.Coin);
	}*/

	OnOurItemsChanged.Broadcast();
	OnOurCoinChanged.Broadcast();
	OnAcceptanceChanged.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_TheirOffer()
{
	// Update coin component display - IMPORTANT: This fixes the coin display bug!
	OnTheirItemsChanged.Broadcast();
	OnTheirCoinChanged.Broadcast();
	OnAcceptanceChanged.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_IsTrading()
{
	OnTradeStateChanged.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_TheirCoinOffer()
{
	OnTheirCoinChanged.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------
// Server-Only Trade Management
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::StartTrade(ACharacter* OtherTrader)
{
	// Server authority check
	if (!GetOwner()->HasAuthority())
		return false;

	// Already trading
	if (bIsTrading)
		return false;

	APlayerController* OtherTraderController = Cast<APlayerController>(OtherTrader->GetController());

	// Invalid partner
	if (!OtherTrader || OtherTraderController == GetOwner())
		return false;

	// Check if other trader can trade
	ITradeInterface* OtherTradeInterface = Cast<ITradeInterface>(OtherTraderController);
	if (!OtherTradeInterface || !OtherTradeInterface->CanTrade())
		return false;

	UTradeComponent* OtherTradeComponent = OtherTradeInterface->GetTradeComponent();
	if (!OtherTradeComponent)
		return false;

	// Check if they're already trading with someone else
	if (OtherTradeComponent->IsTrading())
		return false;

	// Clean up any leftover state (only return items if we somehow have items in offer without being in a trade)
	if (OurOffer.Items.Num() > 0 || !OurCoinOffer->GetCoinValue().IsEmpty())
	{
		ResetTradeState(true);
	}

	if (OtherTradeComponent->OurOffer.Items.Num() > 0 || !OtherTradeComponent->OurCoinOffer->GetCoinValue().IsEmpty())
	{
		OtherTradeComponent->ResetTradeState(true);
	}

	// Initialize trade state
	TradePartner = OtherTrader;
	bIsTrading = true;

	APlayerController* CurrentPC = Cast<APlayerController>(GetOwner());
	AActor* SelfActor = CurrentPC->GetPawn();

	// Initialize partner's trade state
	OtherTradeComponent->TradePartner = SelfActor;
	OtherTradeComponent->bIsTrading = true;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::CancelTrade()
{
	if (!GetOwner()->HasAuthority())
		return;

	if (!bIsTrading || !TradePartner)
	{
		ResetTradeState();
		return;
	}

	// Notify partner to reset (which will return their items)
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->ResetTradeState();
	}

	// Return our items
	ResetTradeState();
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::AddItemToOffer(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	if (!GetOwner()->HasAuthority())
		return false;

	if (!bIsTrading)
		return false;

	// Max 8 items (like staging area)
	if (OurOffer.Items.Num() >= 8)
		return false;

	// Validate we own this item
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	int32 ActualItemID = InventoryInterface->PlayerGetItem(TopLeft, BagSlot);
	if (ActualItemID != ItemID)
		return false;

	// Check if already in offer
	for (const FTradeItemSlot& ExistingSlot : OurOffer.Items)
	{
		if (ExistingSlot.SourceBagSlot == BagSlot && ExistingSlot.SourceTopLeft == TopLeft)
			return false; // Already offered
	}

	float ItemDurability = InventoryInterface->PlayerRemoveItem(TopLeft, BagSlot);
	// Add to offer
	OurOffer.Items.Add(FTradeItemSlot(ItemID, BagSlot, TopLeft, ItemDurability));

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::RemoveItemFromOffer(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority())
		return false;

	if (!bIsTrading)
		return false;

	if (SlotIndex < 0 || SlotIndex >= OurOffer.Items.Num())
		return false;

	// Get the item info before removing it
	const FTradeItemSlot& ItemSlot = OurOffer.Items[SlotIndex];

	// Return item to inventory
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (InventoryInterface)
	{
		// Try to return to original location first
		int32 ItemAtOriginalLocation = InventoryInterface->PlayerGetItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot);

		if (ItemAtOriginalLocation == 0) // Original location is empty
		{
			// Return to original location
			InventoryInterface->PlayerAddItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot, ItemSlot.ItemID);
		}
		else
		{
			// Original location occupied, find a new spot
			int32 FreeSlot = -1;
			UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld());
			EBagSlot FreeBag = InventoryInterface->GetInventoryComponent()->FindSuitableSlot(Item, FreeSlot);

			if (FreeBag != EBagSlot::Unknown && FreeSlot >= 0)
			{
				InventoryInterface->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TradeComponent: Could not find space to return item %d when removing from trade"), ItemSlot.ItemID);
				// Item is stuck in limbo - consider dropping it or keeping it in trade
				// For now, we'll still remove it from offer but log the warning
			}
		}
	}

	OurOffer.Items.RemoveAt(SlotIndex);

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

/*bool UTradeComponent::SetCoinOffer(const FCoinValue& CoinAmount)
{
	if (!GetOwner()->HasAuthority())
		return false;

	if (!bIsTrading)
		return false;

	// Validate we have this much coin
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	if (!InventoryInterface->PlayerCanPayAmount(CoinAmount))
		return false;

	// First, return any previously offered coins back to main purse
	if (OurCoinOffer && OurCoinOffer->GetCoinValue().ToFloat() > 0.f)
	{
		FCoinValue PreviousOffer = OurCoinOffer->GetCoinValue();
		InventoryInterface->GetCoinComponent()->AddCoins(PreviousOffer);
		OurCoinOffer->ClearPurse();
	OurOffer.Coin = CoinAmount;
		if (OurCoinOffer)
	// Update the coin component so UDynamicPurseWidget can display it
	if (OurCoinOffer)
	{
		OurCoinOffer->ClearPurse();
		OurCoinOffer->AddCoins(CoinAmount);
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
	}

	return true;
}*/

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::SetAcceptance(bool bAccept)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (!bIsTrading)
		return;

	OurOffer.bAccepted = bAccept;

	// Update partner's view
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
	}

	// If both accepted, execute trade
	if (BothAccepted())
	{
		ExecuteTrade();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::UpdatePartnerOffer(const FTradeOffer& PartnerOffer)
{
	if (!GetOwner()->HasAuthority())
		return;

	TheirOffer = PartnerOffer;

	// Update the coin component so UDynamicPurseWidget can display it
	/*if (TheirCoinOffer)
	{
		TheirCoinOffer->ClearPurse();
		TheirCoinOffer->AddCoins(PartnerOffer.Coin);
	}*/

	// When partner changes their offer, reset our acceptance
	if (!PartnerOffer.bAccepted && OurOffer.bAccepted)
	{
		OurOffer.bAccepted = false;
	}

	// IMPORTANT: Check if both players have now accepted
	// This fixes the bug where trade doesn't execute when second player accepts
	/*if (BothAccepted())
	{
		ExecuteTrade();
	}*/
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ExecuteTrade()
{
	if (!GetOwner()->HasAuthority())
		return false;

	if (!bIsTrading || !TradePartner)
		return false;

	// Both must have accepted
	if (!BothAccepted())
		return false;

	// Validate our side
	if (!ValidateOurItems() || !ValidateOurCoin())
		return false;

	// Get partner component
	UTradeComponent* PartnerComponent = GetPartnerTradeComponent();
	if (!PartnerComponent)
		return false;

	// Validate their side
	if (!PartnerComponent->ValidateOurItems() || !PartnerComponent->ValidateOurCoin())
		return false;

	ACharacter* PartnerCharacter = Cast<ACharacter>(TradePartner);
	APlayerController* PartnerController = PartnerCharacter ? Cast<APlayerController>(PartnerCharacter->GetController()) : nullptr;
	// Validate space - check if both players can receive all items
	IInventoryPlayerInterface* OurInventory = GetInventoryInterface();
	IInventoryPlayerInterface* TheirInventory = Cast<IInventoryPlayerInterface>(PartnerController);

	if (!OurInventory || !TheirInventory)
		return false;

	// Build array of items we're receiving from them
	TArray<UInventoryItemBase*> TheirItems;
	for (const FTradeItemSlot& ItemSlot : TheirOffer.Items)
	{
		UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld());
		if (Item)
			TheirItems.Add(Item);
	}

	// Build array of items they're receiving from us
	TArray<UInventoryItemBase*> OurItems;
	for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
	{
		UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld());
		if (Item)
			OurItems.Add(Item);
	}

	// Validate we can receive their items
	if (!OurInventory->GetInventoryComponent()->CanReceiveAllItems(TheirItems))
		return false;

	// Validate they can receive our items
	if (!TheirInventory->GetInventoryComponent()->CanReceiveAllItems(OurItems))
		return false;

	//------------------------------------------------------------------------------------------------------------------
	// Execute the trade atomically
	//------------------------------------------------------------------------------------------------------------------

	// 1. Give our items to partner (items are already removed from our inventory)
	for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
	{
		// Add to their inventory (need to find a free slot)
		int32 FreeSlot = -1;

		// Try to find free space in their bags
		EBagSlot FreeBag = TheirInventory->GetInventoryComponent()->FindSuitableSlot(
			UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld()), FreeSlot);

		TheirInventory->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
	}

	// 2. Give their items to us (items are already removed from their inventory)
	for (const FTradeItemSlot& ItemSlot : TheirOffer.Items)
	{
		int32 FreeSlot = -1;
		// Try to find free space in our bags
		EBagSlot FreeBag = OurInventory->GetInventoryComponent()->FindSuitableSlot(
			UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld()), FreeSlot);
		// Add to our inventory
		OurInventory->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
	}

	// 3. Transfer coins
	// Note: Coins are already in OurCoinOffer and TheirCoinOffer components
	// We need to transfer from offer components to main purses
	UCoinComponent* OurCoin = OurInventory->GetCoinComponent();
	UCoinComponent* TheirCoin = TheirInventory->GetCoinComponent();

	if (OurCoin && TheirCoin)
	{
		// Transfer our offered coins from OurCoinOffer to their main purse
		if (OurCoinOffer && OurCoinOffer->GetCoinValue().ToFloat() > 0.f)
		{
			FCoinValue OurOfferedCoins = OurCoinOffer->GetCoinValue();
			TheirInventory->GetCoinComponent()->AddCoins(OurOfferedCoins);
			// OurCoinOffer will be cleared in ResetTradeState
		}

		// Transfer their offered coins from TheirCoinOffer to our main purse
		if (TheirCoinOffer && TheirCoinOffer->GetCoinValue().ToFloat() > 0.f)
		{
			FCoinValue TheirOfferedCoins = TheirCoinOffer->GetCoinValue();
			OurInventory->GetCoinComponent()->AddCoins(TheirOfferedCoins);
			// TheirCoinOffer will be cleared in ResetTradeState
		}
	}

	// 4. Clean up trade session (don't return items since trade was successful)
	ResetTradeState(false);
	PartnerComponent->ResetTradeState(false);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Validation
//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidateOurItems() const
{
	// Items are already removed from inventory when added to trade offer
	// Just validate that all items in our offer are valid (non-zero ItemID)
	for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
	{
		if (ItemSlot.ItemID <= 0)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidateOurCoin() const
{
	if (!OurCoinOffer->HasContent())
		return true;

	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	return InventoryInterface->PlayerCanPayAmount(OurCoinOffer->GetCoinValue());
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidatePartnerHasSpace() const
{
	if (!TradePartner)
		return false;

	IInventoryPlayerInterface* TheirInventory = Cast<IInventoryPlayerInterface>(TradePartner);
	if (!TheirInventory)
		return false;

	// Build array of items we're giving them
	TArray<UInventoryItemBase*> ItemsToGive;
	for (const FTradeItemSlot& Slot : OurOffer.Items)
	{
		// Get item data using InventoryUtilities
		UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Slot.ItemID, GetOwner()->GetWorld());
		if (Item)
		{
			ItemsToGive.Add(Item);
		}
	}

	// Check if they can receive all our items
	UInventoryComponent* TheirInventoryComp = TheirInventory->GetInventoryComponent();
	if (!TheirInventoryComp)
		return false;

	return TheirInventoryComp->CanReceiveAllItems(ItemsToGive);
}

//----------------------------------------------------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UTradeComponent::GetInventoryInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwner());
}

//----------------------------------------------------------------------------------------------------------------------

UTradeComponent* UTradeComponent::GetPartnerTradeComponent() const
{
	if (!TradePartner || !IsValid(TradePartner))
		return nullptr;

	// TradePartner is a Character/Pawn, need to get the controller
	ACharacter* PartnerCharacter = Cast<ACharacter>(TradePartner);
	if (!PartnerCharacter || !IsValid(PartnerCharacter))
		return nullptr;

	APlayerController* PartnerController = Cast<APlayerController>(PartnerCharacter->GetController());
	if (!PartnerController || !IsValid(PartnerController))
		return nullptr;

	ITradeInterface* PartnerInterface = Cast<ITradeInterface>(PartnerController);
	if (!PartnerInterface)
		return nullptr;

	UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent();
	if (!PartnerComponent || !IsValid(PartnerComponent))
		return nullptr;

	return PartnerComponent;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::ResetTradeState(bool bReturnItems)
{
	// Return all items in our offer back to inventory (only if trade was cancelled, not completed)
	if (bReturnItems)
	{
		IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
		if (InventoryInterface && OurOffer.Items.Num() > 0)
		{
			for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
			{
				// Try to return to original location first
				int32 ItemAtOriginalLocation = InventoryInterface->PlayerGetItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot);

				if (ItemAtOriginalLocation == 0) // Original location is empty
				{
					// Return to original location
					InventoryInterface->PlayerAddItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot, ItemSlot.ItemID);
				}
				else
				{
					// Original location occupied, find a new spot
					int32 FreeSlot = -1;
					UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld());
					EBagSlot FreeBag = InventoryInterface->GetInventoryComponent()->FindSuitableSlot(Item, FreeSlot);

					if (FreeBag != EBagSlot::Unknown && FreeSlot >= 0)
					{
						InventoryInterface->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("TradeComponent: Could not find space to return item %d when canceling trade"), ItemSlot.ItemID);
						// Item lost - this is a critical error but should be very rare
						// In production, might want to queue this for later or drop on ground
					}
				}
			}
		}
	}

	TradePartner = nullptr;
	OurOffer.Reset();
	TheirOffer.Reset();
	bIsTrading = false;

	// Clear coin components
	if (OurCoinOffer)
	{
		//add back the data was in the purse to the player (only if returning items)
		if (bReturnItems)
		{
			Cast<IInventoryPlayerInterface>(GetOwner())->GetCoinComponent()->AddCoins(OurCoinOffer->GetCoinValue());
		}
		OurCoinOffer->ClearPurse();
	}
	if (TheirCoinOffer)
	{
		TheirCoinOffer->ClearPurse();
	}
}
