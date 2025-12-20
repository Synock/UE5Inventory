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
	OnOurItemsChanged.Broadcast();
	OnOurCoinChanged.Broadcast();
	OnAcceptanceChanged.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_TheirOffer()
{
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
// Server-Only Trade Management
//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::StageItemForTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	// Store the item to be added when trade actually starts
	StagedItem = FTradeItemSlot(ItemID, BagSlot, TopLeft);

	UE_LOG(LogTemp, Log, TEXT("TradeComponent: Staged item for trade - ItemID=%d, BagSlot=%d, TopLeft=%d"),
		ItemID, static_cast<int32>(BagSlot), TopLeft);
}

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

	// Initialize trade state
	ResetTradeState();
	TradePartner = OtherTrader;
	bIsTrading = true;

	APlayerController* CurrentPC = Cast<APlayerController>(GetOwner());
	AActor* SelfActor = CurrentPC->GetPawn();

	// Initialize partner's trade state
	OtherTradeComponent->ResetTradeState();
	OtherTradeComponent->TradePartner =SelfActor;
	OtherTradeComponent->bIsTrading = true;

	// If we have a staged item, add it to the trade automatically
	if (StagedItem.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("TradeComponent: Auto-adding staged item to trade - ItemID=%d"),
			StagedItem.ItemID);

		AddItemToOffer(StagedItem.ItemID, StagedItem.SourceBagSlot, StagedItem.SourceTopLeft);

		// Clear staged item
		StagedItem = FTradeItemSlot();
	}

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

	// Notify partner
	if (ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner))
	{
		if (UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent())
		{
			PartnerComponent->ResetTradeState();
		}
	}

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

	// Add to offer
	OurOffer.Items.Add(FTradeItemSlot(ItemID, BagSlot, TopLeft));

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner))
	{
		if (UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent())
		{
			PartnerComponent->UpdatePartnerOffer(OurOffer);
		}
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

	OurOffer.Items.RemoveAt(SlotIndex);

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner))
	{
		if (UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent())
		{
			PartnerComponent->UpdatePartnerOffer(OurOffer);
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::SetCoinOffer(const FCoinValue& CoinAmount)
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

	OurOffer.Coin = CoinAmount;

	// Update the coin component so UDynamicPurseWidget can display it
	if (OurCoinOffer)
	{
		OurCoinOffer->ClearPurse();
		OurCoinOffer->AddCoins(CoinAmount);
	}

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view
	if (ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner))
	{
		if (UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent())
		{
			PartnerComponent->UpdatePartnerOffer(OurOffer);
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::SetAcceptance(bool bAccept)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (!bIsTrading)
		return;

	OurOffer.bAccepted = bAccept;

	// Update partner's view
	if (ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner))
	{
		if (UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent())
		{
			PartnerComponent->UpdatePartnerOffer(OurOffer);
		}
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
	if (TheirCoinOffer)
	{
		TheirCoinOffer->ClearPurse();
		TheirCoinOffer->AddCoins(PartnerOffer.Coin);
	}

	// When partner changes their offer, reset our acceptance
	if (!PartnerOffer.bAccepted && OurOffer.bAccepted)
	{
		OurOffer.bAccepted = false;
	}
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
	ITradeInterface* PartnerInterface = Cast<ITradeInterface>(TradePartner);
	if (!PartnerInterface)
		return false;

	UTradeComponent* PartnerComponent = PartnerInterface->GetTradeComponent();
	if (!PartnerComponent)
		return false;

	// Validate their side
	if (!PartnerComponent->ValidateOurItems() || !PartnerComponent->ValidateOurCoin())
		return false;

	// Validate space - check if both players can receive all items
	IInventoryPlayerInterface* OurInventory = GetInventoryInterface();
	IInventoryPlayerInterface* TheirInventory = Cast<IInventoryPlayerInterface>(TradePartner);

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

	// 1. Remove our items and give to partner
	for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
	{
		// Remove from our inventory
		OurInventory->PlayerRemoveItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot);

		// Add to their inventory (need to find a free slot)
		int32 FreeSlot = -1;

		// Try to find free space in their bags
		EBagSlot FreeBag = TheirInventory->GetInventoryComponent()->FindSuitableSlot(
			UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld()), FreeSlot);

		TheirInventory->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
	}

	// 2. Remove their items and give to us
	for (const FTradeItemSlot& ItemSlot : TheirOffer.Items)
	{
		// Remove from their inventory
		TheirInventory->PlayerRemoveItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot);

		int32 FreeSlot = -1;
		// Try to find free space in their bags
		EBagSlot FreeBag = OurInventory->GetInventoryComponent()->FindSuitableSlot(
			UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetOwner()->GetWorld()), FreeSlot);
		// Add to our inventory
		OurInventory->PlayerAddItem(FreeSlot, FreeBag, ItemSlot.ItemID);
	}

	// 3. Transfer coins
	UCoinComponent* OurCoin = OurInventory->GetCoinComponent();
	UCoinComponent* TheirCoin = TheirInventory->GetCoinComponent();

	if (OurCoin && TheirCoin)
	{
		if (OurOffer.Coin.ToFloat() > 0.f)
		{
			OurCoin->PayAndAdjust(OurOffer.Coin);
			TheirCoin->AddCoins(OurOffer.Coin);
		}

		if (TheirOffer.Coin.ToFloat() > 0.f)
		{
			TheirCoin->PayAndAdjust(TheirOffer.Coin);
			OurCoin->AddCoins(TheirOffer.Coin);
		}
	}

	// 4. Clean up trade session
	ResetTradeState();
	PartnerComponent->ResetTradeState();

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Validation
//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidateOurItems() const
{
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
	{
		int32 ActualItemID = InventoryInterface->PlayerGetItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot);
		if (ActualItemID != ItemSlot.ItemID)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidateOurCoin() const
{
	if (OurOffer.Coin.IsEmpty())
		return true;

	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	return InventoryInterface->PlayerCanPayAmount(OurOffer.Coin);
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

void UTradeComponent::ResetTradeState()
{
	TradePartner = nullptr;
	OurOffer.Reset();
	TheirOffer.Reset();
	bIsTrading = false;

	// Clear staged item (in case trade was cancelled before starting)
	StagedItem = FTradeItemSlot();

	// Clear coin components
	if (OurCoinOffer)
	{
		//add back the data was in the purse to the player
		Cast<IInventoryPlayerInterface>(GetOwner())->GetCoinComponent()->AddCoins(OurCoinOffer->GetCoinValue());
		OurCoinOffer->ClearPurse();
	}
	if (TheirCoinOffer)
	{
		TheirCoinOffer->ClearPurse();
	}
}

