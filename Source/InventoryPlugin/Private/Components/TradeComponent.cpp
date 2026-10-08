#include "Components/TradeComponent.h"
#include "Components/TradeReturnRouting.h"
#include "InventoryPlugin.h"

#include "InventoryUtilities.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/TradeInterface.h"
#include "InventoryPlugin.h"
#include "Components/CoinComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/InventoryDeliveryComponent.h"
#include "InventoryPlugin.h"
#include "Components/SphereComponent.h"
#include "InventoryPlugin.h"
#include "GameFramework/Character.h"
#include "InventoryPlugin.h"
#include "Net/UnrealNetwork.h"
#include "InventoryPlugin.h"
#include "GameFramework/PlayerController.h"
#include "InventoryPlugin.h"
#include "GameFramework/PlayerState.h"
#include "InventoryPlugin.h"

UTradeComponent::UTradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Create coin component for our offer (allows direct UI manipulation)
	OurCoinOffer = CreateDefaultSubobject<UCoinComponent>(TEXT("OurCoinOffer"));
	if (OurCoinOffer)
	{
		OurCoinOffer->SetIsReplicated(true);
	}

	// Create sphere component for distance detection (not created as subobject, created dynamically)
	TradeRangeSphere = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to our coin offer changes so we can update partner when coins change
	if (OurCoinOffer)
	{
		OurCoinOffer->PurseDispatcher.AddDynamic(this, &UTradeComponent::OnOurCoinOfferChanged);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// If player disconnects while trading, safely cancel the trade and return items
	if (GetOwner()->HasAuthority() && bIsTrading)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("TradeComponent: Player disconnected during trade, cancelling and returning items"));
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
}

//----------------------------------------------------------------------------------------------------------------------
// Replication Callbacks
//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnRep_TradePartner()
{
	OnTradeStateChanged.Broadcast();
	// UI is driven by OnRep_IsTrading — no UI calls here to avoid replication race conditions.
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

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
		return;

	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC);
	if (!PlayerInterface)
		return;

	IInventoryHUDInterface* HUDInterface = PlayerInterface->GetInventoryHUDInterface();
	if (!HUDInterface)
		return;

	UObject* HUDObject = PlayerInterface->GetInventoryHUDObject();
	if (bIsTrading)
		HUDInterface->Execute_OpenTradeWindow(HUDObject);
	else
		HUDInterface->Execute_CloseTradeWindow(HUDObject);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnOurCoinOfferChanged()
{
	// Only process if we're actively trading
	if (!bIsTrading)
		return;

	// On client: notify server via RPC
	if (!GetOwner()->HasAuthority())
	{
		Server_NotifyCoinOfferChanged();
		return;
	}
	OnEscrowChanged.Broadcast();

	// On server: process the change
	// Get current coin value
	FCoinValue CurrentCoinValue = OurCoinOffer ? OurCoinOffer->GetCoinValue() : FCoinValue();

	// Calculate the difference to determine if coins were added or removed
	float CurrentTotal = CurrentCoinValue.ToFloat();
	float PreviousTotal = PreviousCoinValue.ToFloat();

	if (CurrentTotal != PreviousTotal)
	{
		// Determine if coins were added or removed
		bool bCoinsAdded = CurrentTotal > PreviousTotal;
		FCoinValue DifferenceCoin;

		if (bCoinsAdded)
		{
			// Coins were added - calculate the difference
			DifferenceCoin.PlatinumPieces = CurrentCoinValue.PlatinumPieces - PreviousCoinValue.PlatinumPieces;
			DifferenceCoin.GoldPieces = CurrentCoinValue.GoldPieces - PreviousCoinValue.GoldPieces;
			DifferenceCoin.SilverPieces = CurrentCoinValue.SilverPieces - PreviousCoinValue.SilverPieces;
			DifferenceCoin.CopperPieces = CurrentCoinValue.CopperPieces - PreviousCoinValue.CopperPieces;
		}
		else
		{
			// Coins were removed - calculate the difference
			DifferenceCoin.PlatinumPieces = PreviousCoinValue.PlatinumPieces - CurrentCoinValue.PlatinumPieces;
			DifferenceCoin.GoldPieces = PreviousCoinValue.GoldPieces - CurrentCoinValue.GoldPieces;
			DifferenceCoin.SilverPieces = PreviousCoinValue.SilverPieces - CurrentCoinValue.SilverPieces;
			DifferenceCoin.CopperPieces = PreviousCoinValue.CopperPieces - CurrentCoinValue.CopperPieces;
		}

		// Broadcast notification to both players
		FString PlayerName = GetOwnerPlayerName();
		BroadcastTradeNotification(PlayerName, nullptr, DifferenceCoin, bCoinsAdded, false, true);

		// Update tracking
		PreviousCoinValue = CurrentCoinValue;
	}

	// Reset acceptance when coin offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Update partner's view with current coin value
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::Server_NotifyCoinOfferChanged_Implementation()
{
	// Server received notification from client that coins changed
	// Trigger the same logic as if it happened on server
	OnOurCoinOfferChanged();
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
	if (OurOffer.Items.Num() > 0 || (OurCoinOffer && OurCoinOffer->HasContent()))
	{
		ResetTradeState(true);
	}

	if (OtherTradeComponent->OurOffer.Items.Num() > 0 || (OtherTradeComponent->OurCoinOffer && OtherTradeComponent->OurCoinOffer->HasContent()))
	{
		OtherTradeComponent->ResetTradeState(true);
	}

	// Initialize trade state
	TradePartner = OtherTrader;
	bIsTrading = true;
	PreviousCoinValue = FCoinValue(); // Reset coin tracking

	APlayerController* CurrentPC = Cast<APlayerController>(GetOwner());
	AActor* SelfActor = CurrentPC->GetPawn();

	// Initialize partner's trade state
	OtherTradeComponent->TradePartner = SelfActor;
	OtherTradeComponent->bIsTrading = true;
	OtherTradeComponent->PreviousCoinValue = FCoinValue(); // Reset partner's coin tracking

	// Create and setup interaction sphere for distance detection (server only)
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(SelfActor))
	{
		// Create sphere component dynamically
		TradeRangeSphere = NewObject<USphereComponent>(OwnerCharacter, USphereComponent::StaticClass(), TEXT("TradeRangeSphere"));
		if (TradeRangeSphere)
		{
			TradeRangeSphere->RegisterComponent();
			TradeRangeSphere->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			TradeRangeSphere->SetSphereRadius(MaxTradeDistance);

			// Set collision settings - only detect pawns
			TradeRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			TradeRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			TradeRangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			TradeRangeSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

			// Bind to end overlap event
			TradeRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UTradeComponent::OnTradePartnerExitSphere);

			// Enable overlap events
			TradeRangeSphere->SetGenerateOverlapEvents(true);
		}
	}

	// Also create sphere for partner
	if (ACharacter* PartnerCharacter = Cast<ACharacter>(OtherTrader))
	{
		OtherTradeComponent->TradeRangeSphere = NewObject<USphereComponent>(PartnerCharacter, USphereComponent::StaticClass(), TEXT("TradeRangeSphere"));
		if (OtherTradeComponent->TradeRangeSphere)
		{
			OtherTradeComponent->TradeRangeSphere->RegisterComponent();
			OtherTradeComponent->TradeRangeSphere->AttachToComponent(PartnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			OtherTradeComponent->TradeRangeSphere->SetSphereRadius(MaxTradeDistance);

			OtherTradeComponent->TradeRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			OtherTradeComponent->TradeRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			OtherTradeComponent->TradeRangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			OtherTradeComponent->TradeRangeSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

			OtherTradeComponent->TradeRangeSphere->OnComponentEndOverlap.AddDynamic(OtherTradeComponent, &UTradeComponent::OnTradePartnerExitSphere);
			OtherTradeComponent->TradeRangeSphere->SetGenerateOverlapEvents(true);
		}
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

	// Cannot trade items from equipment slots (bags attached to equipment are OK)
	if (BagSlot == EBagSlot::Unknown)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("TradeComponent: Cannot add item from equipment slot to trade"));
		return false;
	}

	// Validate we own this item
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	int32 ActualItemID = InventoryInterface->PlayerGetItem(TopLeft, BagSlot);
	if (ActualItemID != ItemID)
		return false;
	UInventoryComponent* Inventory = InventoryInterface->GetInventoryComponent();
	const UInventoryItemBase* ItemDefinition = UInventoryUtilities::GetItemFromID(ItemID, GetOwner()->GetWorld());
	if (!Inventory || !ItemDefinition)
		return false;

	// Check if already in offer
	for (const FTradeItemSlot& ExistingSlot : OurOffer.Items)
	{
		if (ExistingSlot.SourceBagSlot == BagSlot && ExistingSlot.SourceTopLeft == TopLeft)
			return false; // Already offered
	}

	const float EffectiveWeight = Inventory->GetEffectiveItemWeight(BagSlot, ItemDefinition);
	const float ItemDurability = InventoryInterface->PlayerRemoveItem(TopLeft, BagSlot);
	const FGuid ReservationId = FGuid::NewGuid();
	if (!Inventory->ReserveItemFootprint(ReservationId, BagSlot, ItemDefinition, TopLeft))
	{
		InventoryInterface->PlayerAddItemWithDurability(TopLeft, BagSlot, ItemID, ItemDurability);
		return false;
	}
	// Add to offer
	OurOffer.Items.Add(FTradeItemSlot(ItemID, BagSlot, TopLeft, ItemDurability, ReservationId, EffectiveWeight));
	OnEscrowChanged.Broadcast();

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Broadcast notification to both players
	UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetOwner()->GetWorld());
	FString PlayerName = GetOwnerPlayerName();
	BroadcastTradeNotification(PlayerName, Item, FCoinValue(), true, true, true);

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
	int32 RemovedItemID = ItemSlot.ItemID;

	if (!ReturnEscrowedItem(ItemSlot))
		return false;

	OurOffer.Items.RemoveAt(SlotIndex);
	OnEscrowChanged.Broadcast();

	// Reset acceptance when offer changes
	OurOffer.bAccepted = false;
	TheirOffer.bAccepted = false;

	// Broadcast notification to both players
	UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(RemovedItemID, GetOwner()->GetWorld());
	FString PlayerName = GetOwnerPlayerName();
	BroadcastTradeNotification(PlayerName, Item, FCoinValue(), false, true, true);

	// Update partner's view
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		PartnerComponent->UpdatePartnerOffer(OurOffer);
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

	// Sync the coin value from the partner's OurCoinOffer component to our TheirOffer.CoinOffer
	UTradeComponent* PartnerComponent = GetPartnerTradeComponent();
	if (PartnerComponent && PartnerComponent->OurCoinOffer)
	{
		TheirOffer.CoinOffer = PartnerComponent->OurCoinOffer->GetCoinValue();
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

	// Validate distance first - if too far, cancel trade and notify
	if (!ValidateTradeDistance())
	{
		// Get partner component to notify them as well
		UTradeComponent* PartnerComponent = GetPartnerTradeComponent();

		FString CancellationReason = TEXT("Trade cancelled: You are too far away from your trade partner.");

		// Broadcast to local player (owner of this component)
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			OnTradeCancelled.Broadcast(PC, CancellationReason);
		}

		// Broadcast to partner
		if (PartnerComponent)
		{
			ACharacter* PartnerCharacter = Cast<ACharacter>(TradePartner);
			APlayerController* PartnerPC = PartnerCharacter ? Cast<APlayerController>(PartnerCharacter->GetController()) : nullptr;
			if (PartnerPC)
			{
				PartnerComponent->OnTradeCancelled.Broadcast(PartnerPC, CancellationReason);
			}
		}

		// Cancel the trade and return items
		CancelTrade();
		return false;
	}

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

	// Outgoing items already occupy their source capacity through reservations. During a successful exchange those
	// cells become available, so release both sides only for the synchronous plan/commit window.
	ReleaseOfferReservations();
	PartnerComponent->ReleaseOfferReservations();

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

	TArray<FInventoryDeliveryDestination> OurIncomingPlacements;
	TArray<FInventoryDeliveryDestination> TheirIncomingPlacements;
	const auto RejectForCapacity = [this, PartnerComponent]()
	{
		RestoreOfferReservations();
		PartnerComponent->RestoreOfferReservations();
		OurOffer.bAccepted = false;
		PartnerComponent->OurOffer.bAccepted = false;
		PartnerComponent->UpdatePartnerOffer(OurOffer);
		UpdatePartnerOffer(PartnerComponent->OurOffer);
		const FString Message = TEXT("Trade could not complete: one player does not have enough inventory space.");
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
			OnTradeCancelled.Broadcast(PC, Message);
		if (APlayerController* PartnerPC = Cast<APlayerController>(PartnerComponent->GetOwner()))
			PartnerComponent->OnTradeCancelled.Broadcast(PartnerPC, Message);
	};

	if (!OurInventory->GetInventoryComponent()->FindPlacementsForItems(TheirItems, OurIncomingPlacements) ||
		!TheirInventory->GetInventoryComponent()->FindPlacementsForItems(OurItems, TheirIncomingPlacements))
	{
		RejectForCapacity();
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------
	// Execute the trade atomically
	//------------------------------------------------------------------------------------------------------------------

	// 1. Give our items to partner (items are already removed from our inventory)
	bool bPlacementCommitted = true;
	for (int32 Index = 0; Index < OurOffer.Items.Num(); ++Index)
	{
		const FTradeItemSlot& ItemSlot = OurOffer.Items[Index];
		const FInventoryDeliveryDestination& Destination = TheirIncomingPlacements[Index];
		TheirInventory->PlayerAddItemWithDurability(Destination.TopLeft, Destination.Bag,
			ItemSlot.ItemID, ItemSlot.Durability);
		if (TheirInventory->PlayerGetItem(Destination.TopLeft, Destination.Bag) != ItemSlot.ItemID)
		{
			bPlacementCommitted = false;
			break;
		}
	}

	// 2. Give their items to us (items are already removed from their inventory)
	for (int32 Index = 0; bPlacementCommitted && Index < TheirOffer.Items.Num(); ++Index)
	{
		const FTradeItemSlot& ItemSlot = TheirOffer.Items[Index];
		const FInventoryDeliveryDestination& Destination = OurIncomingPlacements[Index];
		OurInventory->PlayerAddItemWithDurability(Destination.TopLeft, Destination.Bag,
			ItemSlot.ItemID, ItemSlot.Durability);
		if (OurInventory->PlayerGetItem(Destination.TopLeft, Destination.Bag) != ItemSlot.ItemID)
		{
			bPlacementCommitted = false;
			break;
		}
	}

	if (!bPlacementCommitted)
	{
		// Remove only entries that match this transaction's precomputed empty destinations, then restore the two
		// escrow reservation sets. Coins have not moved yet, so rollback is complete and idempotent.
		for (int32 Index = 0; Index < OurOffer.Items.Num(); ++Index)
		{
			const FInventoryDeliveryDestination& Destination = TheirIncomingPlacements[Index];
			if (TheirInventory->PlayerGetItem(Destination.TopLeft, Destination.Bag) == OurOffer.Items[Index].ItemID)
				TheirInventory->PlayerRemoveItem(Destination.TopLeft, Destination.Bag);
		}
		for (int32 Index = 0; Index < TheirOffer.Items.Num(); ++Index)
		{
			const FInventoryDeliveryDestination& Destination = OurIncomingPlacements[Index];
			if (OurInventory->PlayerGetItem(Destination.TopLeft, Destination.Bag) == TheirOffer.Items[Index].ItemID)
				OurInventory->PlayerRemoveItem(Destination.TopLeft, Destination.Bag);
		}
		RejectForCapacity();
		return false;
	}

	// 3. Transfer coins
	UCoinComponent* OurCoin = OurInventory->GetCoinComponent();
	UCoinComponent* TheirCoin = TheirInventory->GetCoinComponent();

	if (OurCoin && TheirCoin)
	{
		// Transfer our offered coins from OurCoinOffer component to their main purse
		if (OurCoinOffer && OurCoinOffer->HasContent())
		{
			FCoinValue OurOfferedCoins = OurCoinOffer->GetCoinValue();
			TheirCoin->AddCoins(OurOfferedCoins);
			// OurCoinOffer will be cleared in ResetTradeState
		}

		// Transfer their offered coins from TheirOffer.CoinOffer to our main purse
		if (!TheirOffer.CoinOffer.IsEmpty())
		{
			OurCoin->AddCoins(TheirOffer.CoinOffer);
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
		if (ItemSlot.ItemID <= 0 || !UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetWorld()))
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UTradeComponent::ValidateOurCoin() const
{
	if (!OurCoinOffer || !OurCoinOffer->HasContent())
		return true; // No coins offered, valid

	// Coins are in OurCoinOffer component which was already validated when added
	return true;
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

bool UTradeComponent::ValidateTradeDistance() const
{
	if (!TradePartner || !GetOwner())
		return false;

	// Get the owner's location (the player controller's pawn/character)
	AActor* OwnerActor = GetOwner();
	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerPawn)
	{
		// Owner might be a controller, try to get its pawn
		if (AController* OwnerController = Cast<AController>(OwnerActor))
		{
			OwnerPawn = OwnerController->GetPawn();
		}
	}

	if (!OwnerPawn)
		return false;

	// Get trade partner's location (should be a Character/Pawn)
	FVector OwnerLocation = OwnerPawn->GetActorLocation();
	FVector PartnerLocation = TradePartner->GetActorLocation();

	// Calculate distance between traders
	float Distance = FVector::Dist(OwnerLocation, PartnerLocation);

	// Check if within acceptable range
	return Distance <= MaxTradeDistance;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::OnTradePartnerExitSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Only run on server
	if (!GetOwner()->HasAuthority())
		return;

	// Verify we're still trading
	if (!bIsTrading || !TradePartner)
		return;

	// Check if the actor that exited is our trade partner
	if (OtherActor != TradePartner)
		return;

	// Trade partner has moved outside the sphere radius
	FString CancellationReason = TEXT("Trade cancelled: You moved too far away from your trade partner.");

	// Get partner component for notification
	UTradeComponent* PartnerComponent = GetPartnerTradeComponent();

	// Broadcast to local player (owner of this component)
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		OnTradeCancelled.Broadcast(PC, CancellationReason);
	}

	// Broadcast to partner
	if (PartnerComponent)
	{
		ACharacter* PartnerCharacter = Cast<ACharacter>(TradePartner);
		APlayerController* PartnerPC = PartnerCharacter ? Cast<APlayerController>(PartnerCharacter->GetController()) : nullptr;
		if (PartnerPC)
		{
			PartnerComponent->OnTradeCancelled.Broadcast(PartnerPC, CancellationReason);
		}
	}

	// Cancel the trade and return items
	CancelTrade();
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
	// Destroy and cleanup the trade range sphere if it exists
	if (TradeRangeSphere)
	{
		TradeRangeSphere->OnComponentEndOverlap.RemoveAll(this);
		TradeRangeSphere->DestroyComponent();
		TradeRangeSphere = nullptr;
	}

	// Return every escrowed item or retain unresolved entries. Never discard the only copy.
	if (bReturnItems)
	{
		TArray<FTradeItemSlot> UnresolvedItems;
		for (const FTradeItemSlot& ItemSlot : OurOffer.Items)
		{
			if (!ReturnEscrowedItem(ItemSlot))
				UnresolvedItems.Add(ItemSlot);
		}
		OurOffer.Items = MoveTemp(UnresolvedItems);
	}

	TradePartner = nullptr;
	if (!bReturnItems || OurOffer.Items.IsEmpty())
		OurOffer.Reset();
	OnEscrowChanged.Broadcast();
	TheirOffer.Reset();
	bIsTrading = false;

	// Clear our coin offer component
	if (OurCoinOffer)
	{
		// If returning items (trade canceled), coins in OurCoinOffer need to be returned to player
		// But OurCoinOffer already contains the coins (they were moved there when offering)
		// So we just need to move them back to the main purse
		if (bReturnItems && OurCoinOffer->HasContent())
		{
			IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
			if (InventoryInterface)
			{
				InventoryInterface->GetCoinComponent()->AddCoins(OurCoinOffer->GetCoinValue());
			}
		}
		OurCoinOffer->ClearPurse();
	}

	// Reset coin tracking
	PreviousCoinValue = FCoinValue();
}

bool UTradeComponent::ReturnEscrowedItem(const FTradeItemSlot& ItemSlot)
{
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	if (!InventoryInterface)
		return false;

	UInventoryComponent* Inventory = InventoryInterface->GetInventoryComponent();
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemSlot.ItemID, GetWorld());
	if (!Inventory || !Item)
		return false;

	Inventory->ReleaseItemFootprint(ItemSlot.ReservationId);
	if (Inventory->CanPlaceItemAt(ItemSlot.SourceBagSlot, Item, ItemSlot.SourceTopLeft))
	{
		InventoryInterface->PlayerAddItemWithDurability(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot,
			ItemSlot.ItemID, ItemSlot.Durability);
		if (InventoryInterface->PlayerGetItem(ItemSlot.SourceTopLeft, ItemSlot.SourceBagSlot) == ItemSlot.ItemID)
			return true;
	}
	UInventoryDeliveryComponent* DeliveryComponent = InventoryInterface->GetInventoryDeliveryComponent();
	if (!DeliveryComponent)
	{
		Inventory->ReserveItemFootprint(ItemSlot.ReservationId, ItemSlot.SourceBagSlot, Item,
			ItemSlot.SourceTopLeft);
		return false;
	}

	const bool bQueued = InventoryPlugin::TradeReturn::Route(ItemSlot,
		[DeliveryComponent](FInventoryDeliveryRequest Request)
		{
			return DeliveryComponent->TryDeliverOrQueue(MoveTemp(Request));
		});
	if (!bQueued)
		Inventory->ReserveItemFootprint(ItemSlot.ReservationId, ItemSlot.SourceBagSlot, Item,
			ItemSlot.SourceTopLeft);
	return bQueued;
}

void UTradeComponent::ReleaseOfferReservations()
{
	if (IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface())
		if (UInventoryComponent* Inventory = InventoryInterface->GetInventoryComponent())
			for (const FTradeItemSlot& Slot : OurOffer.Items)
				Inventory->ReleaseItemFootprint(Slot.ReservationId);
}

bool UTradeComponent::RestoreOfferReservations()
{
	IInventoryPlayerInterface* InventoryInterface = GetInventoryInterface();
	UInventoryComponent* Inventory = InventoryInterface ? InventoryInterface->GetInventoryComponent() : nullptr;
	if (!Inventory)
		return false;
	bool bAllRestored = true;
	for (const FTradeItemSlot& Slot : OurOffer.Items)
	{
		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Slot.ItemID, GetWorld());
		bAllRestored &= Item && Inventory->ReserveItemFootprint(Slot.ReservationId, Slot.SourceBagSlot, Item,
			Slot.SourceTopLeft);
	}
	return bAllRestored;
}

float UTradeComponent::GetEscrowWeight() const
{
	float Weight = 0.0f;
	for (const FTradeItemSlot& Slot : OurOffer.Items)
		Weight += FMath::Max(0.0f, Slot.EffectiveWeight);
	if (OurCoinOffer)
		Weight += OurCoinOffer->GetTotalWeight();
	return Weight;
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeComponent::BroadcastTradeNotification(const FString& PlayerName, UInventoryItemBase* Item, const FCoinValue& CoinValue, bool bIsAdd, bool bIsItem, bool bIsOurAction)
{
	// Broadcast to our delegates
	if (bIsItem)
	{
		if (bIsAdd)
		{
			OnTradeItemAdded.Broadcast(PlayerName, Item, bIsOurAction);
		}
		else
		{
			OnTradeItemRemoved.Broadcast(PlayerName, Item, bIsOurAction);
		}
	}
	else
	{
		if (bIsAdd)
		{
			OnTradeCoinAdded.Broadcast(PlayerName, CoinValue, bIsOurAction);
		}
		else
		{
			OnTradeCoinRemoved.Broadcast(PlayerName, CoinValue, bIsOurAction);
		}
	}

	// Also broadcast to partner's delegates
	if (UTradeComponent* PartnerComponent = GetPartnerTradeComponent())
	{
		if (bIsItem)
		{
			if (bIsAdd)
			{
				PartnerComponent->OnTradeItemAdded.Broadcast(PlayerName, Item, false); // false = partner's action from their perspective
			}
			else
			{
				PartnerComponent->OnTradeItemRemoved.Broadcast(PlayerName, Item, false);
			}
		}
		else
		{
			if (bIsAdd)
			{
				PartnerComponent->OnTradeCoinAdded.Broadcast(PlayerName, CoinValue, false);
			}
			else
			{
				PartnerComponent->OnTradeCoinRemoved.Broadcast(PlayerName, CoinValue, false);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

FString UTradeComponent::GetOwnerPlayerName() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{

		// Try IInventoryInterface first (has GetInventoryOwnerName method)
		if (IInventoryInterface* InventoryInterface = Cast<IInventoryInterface>(PC))
		{
			return InventoryInterface->GetInventoryOwnerName();
		}

		if (IInventoryInterface* InventoryInterface = Cast<IInventoryInterface>(PC->GetPawn()))
		{
			return InventoryInterface->GetInventoryOwnerName();
		}

		// Fallback to player state name
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			return PS->GetPlayerName();
		}
	}

	return TEXT("Unknown");
}

