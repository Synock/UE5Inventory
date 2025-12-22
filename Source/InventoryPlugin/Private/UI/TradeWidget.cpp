#include "UI/TradeWidget.h"
#include "UI/TradeSlotWidget.h"
#include "UI/PurseWidget.h"
#include "UI/Currency/DynamicPurseWidget.h"
#include "Components/TradeComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/TradeInterface.h"
#include "InventoryUtilities.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/InventoryInterface.h"


void UTradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Unbind any existing callbacks before binding to prevent duplicates
	// This can happen if the widget is reused without being destroyed
	if (AcceptButton)
	{
		AcceptButton->OnClicked.RemoveAll(this);
		AcceptButton->OnClicked.AddDynamic(this, &UTradeWidget::OnAcceptButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveAll(this);
		CancelButton->OnClicked.AddDynamic(this, &UTradeWidget::OnCancelButtonClicked);
	}

	// Initialize slot indices (safe to call multiple times, just sets index values)
	if (OurSlot0) OurSlot0->InitializeSlot(0, true);
	if (OurSlot1) OurSlot1->InitializeSlot(1, true);
	if (OurSlot2) OurSlot2->InitializeSlot(2, true);
	if (OurSlot3) OurSlot3->InitializeSlot(3, true);
	if (OurSlot4) OurSlot4->InitializeSlot(4, true);
	if (OurSlot5) OurSlot5->InitializeSlot(5, true);
	if (OurSlot6) OurSlot6->InitializeSlot(6, true);
	if (OurSlot7) OurSlot7->InitializeSlot(7, true);

	if (TheirSlot0) TheirSlot0->InitializeSlot(0, false);
	if (TheirSlot1) TheirSlot1->InitializeSlot(1, false);
	if (TheirSlot2) TheirSlot2->InitializeSlot(2, false);
	if (TheirSlot3) TheirSlot3->InitializeSlot(3, false);
	if (TheirSlot4) TheirSlot4->InitializeSlot(4, false);
	if (TheirSlot5) TheirSlot5->InitializeSlot(5, false);
	if (TheirSlot6) TheirSlot6->InitializeSlot(6, false);
	if (TheirSlot7) TheirSlot7->InitializeSlot(7, false);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::NativeDestruct()
{
	// Unbind delegates
	if (TradeComponent)
	{
		TradeComponent->OnTradeStateChanged.RemoveAll(this);
		TradeComponent->OnOurItemsChanged.RemoveAll(this);
		TradeComponent->OnTheirItemsChanged.RemoveAll(this);
		TradeComponent->OnOurCoinChanged.RemoveAll(this);
		TradeComponent->OnTheirCoinChanged.RemoveAll(this);
		TradeComponent->OnAcceptanceChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::InitializeTrade(UTradeComponent* InTradeComponent)
{
	if (!InTradeComponent)
		return;

	// Unbind from old trade component if we're re-initializing
	if (TradeComponent && TradeComponent != InTradeComponent)
	{
		TradeComponent->OnTradeStateChanged.RemoveAll(this);
		TradeComponent->OnOurItemsChanged.RemoveAll(this);
		TradeComponent->OnTheirItemsChanged.RemoveAll(this);
		TradeComponent->OnOurCoinChanged.RemoveAll(this);
		TradeComponent->OnTheirCoinChanged.RemoveAll(this);
		TradeComponent->OnAcceptanceChanged.RemoveAll(this);
	}

	TradeComponent = InTradeComponent;

	// Unbind any existing delegates before binding to prevent duplicate bindings
	// This is critical because the widget might be reused without being destroyed
	TradeComponent->OnTradeStateChanged.RemoveAll(this);
	TradeComponent->OnOurItemsChanged.RemoveAll(this);
	TradeComponent->OnTheirItemsChanged.RemoveAll(this);
	TradeComponent->OnOurCoinChanged.RemoveAll(this);
	TradeComponent->OnTheirCoinChanged.RemoveAll(this);
	TradeComponent->OnAcceptanceChanged.RemoveAll(this);

	// Now bind to trade component delegates (clean slate)
	TradeComponent->OnTradeStateChanged.AddDynamic(this, &UTradeWidget::OnTradeStateChanged);
	TradeComponent->OnOurItemsChanged.AddDynamic(this, &UTradeWidget::OnOurItemsChanged);
	TradeComponent->OnTheirItemsChanged.AddDynamic(this, &UTradeWidget::OnTheirItemsChanged);
	TradeComponent->OnOurCoinChanged.AddDynamic(this, &UTradeWidget::OnOurCoinChanged);
	TradeComponent->OnTheirCoinChanged.AddDynamic(this, &UTradeWidget::OnTheirCoinChanged);
	TradeComponent->OnAcceptanceChanged.AddDynamic(this, &UTradeWidget::OnAcceptanceChanged);


	IInventoryPlayerInterface* OurInventory = GetInventoryInterface();
	if (OurInventory && OurNameText)
	{
		ITradeInterface* OurTradeInterface = Cast<ITradeInterface>(GetOwningPlayer());
		if (OurTradeInterface)
		{
			OurNameText->SetText(FText::FromString(OurTradeInterface->GetTraderName()));
		}
	}

	if (TradeComponent->GetTradePartner() && TheirNameText)
	{
		IInventoryInterface* TheirTradeInterface = Cast<IInventoryInterface>(TradeComponent->GetTradePartner());
		if (TheirTradeInterface)
		{
			TheirNameText->SetText(FText::FromString(TheirTradeInterface->GetInventoryOwnerName()));
		}
	}

	// Initialize our coin display widget with the coin component (allows automatic updates and UI manipulation)
	if (OurCoinOffer && TradeComponent->GetOurCoinComponent())
	{
		OurCoinOffer->InitWidget(TradeComponent->GetOurCoinComponent());
	}

	// Initial refresh
	RefreshOurOffer();
	RefreshTheirOffer();
	RefreshAcceptanceState();
	RefreshStatusText();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::CloseTrade()
{
	// This will be called by the cancel button or when trade completes
	//RemoveFromParent();
}

//----------------------------------------------------------------------------------------------------------------------
// Button Callbacks
//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnAcceptButtonClicked()
{
	if (!TradeComponent)
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	// Use inventoryinterface which has the RPC methods
	IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(PC);
	if (!Inventory)
		return;

	// Toggle acceptance using the wrapper function
	bool bCurrentAcceptance = TradeComponent->HaveWeAccepted();
	Inventory->PlayerToggleTradeAcceptance(!bCurrentAcceptance);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnCancelButtonClicked()
{
	if (!TradeComponent)
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	// Use inventory interface to cancel trade
	IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(PC);
	if (!Inventory)
		return;

	Inventory->PlayerCancelTrade();
	CloseTrade();
}

//----------------------------------------------------------------------------------------------------------------------
// Delegate Handlers
//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnTradeStateChanged()
{
	if (!TradeComponent || !TradeComponent->IsTrading())
	{
		CloseTrade();
		return;
	}

	RefreshStatusText();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnOurItemsChanged()
{
	RefreshOurOffer();
	RefreshStatusText();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnTheirItemsChanged()
{
	RefreshTheirOffer();
	RefreshStatusText();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnOurCoinChanged()
{
	RefreshOurOffer();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnTheirCoinChanged()
{
	RefreshTheirOffer();
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::OnAcceptanceChanged()
{
	RefreshAcceptanceState();
	RefreshStatusText();
}

//----------------------------------------------------------------------------------------------------------------------
// Refresh Functions
//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::RefreshOurOffer()
{
	if (!TradeComponent)
		return;

	const FTradeOffer& OurOffer = TradeComponent->GetOurOffer();

	// Update item slots
	for (int32 i = 0; i < 8; ++i)
	{
		UTradeSlotWidget* TradeSlot = GetOurSlot(i);
		if (!TradeSlot)
			continue;

		if (i < OurOffer.Items.Num())
		{
			// Get the actual item data from the ID
			const UInventoryItemBase* ItemData = UInventoryUtilities::GetItemFromID(
				OurOffer.Items[i].ItemID, GetWorld());
			TradeSlot->SetTradeItem(ItemData, GetOwningPlayer());
		}
		else
		{
			TradeSlot->ClearSlot();
		}
	}

	// Coin display automatically updates via OurCoinOffer component binding
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::RefreshTheirOffer()
{
	if (!TradeComponent)
		return;

	const FTradeOffer& TheirOffer = TradeComponent->GetTheirOffer();

	// Update item slots
	for (int32 i = 0; i < 8; ++i)
	{
		UTradeSlotWidget* TradeSlot = GetTheirSlot(i);
		if (!TradeSlot)
			continue;

		if (i < TheirOffer.Items.Num())
		{
			// Get the actual item data from the ID
			const UInventoryItemBase* ItemData = UInventoryUtilities::GetItemFromID(
				TheirOffer.Items[i].ItemID, GetWorld());

			// Get the partner's controller as the owner
			AActor* PartnerOwner = TradeComponent->GetTradePartner();
			TradeSlot->SetTradeItem(ItemData, PartnerOwner);
		}
		else
		{
			TradeSlot->ClearSlot();
		}
	}

	// Update coin display
	if (TheirCoinOffer)
	{
		TheirCoinOffer->SetCoinValue(TradeComponent->GetTheirCoinOffer());
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::RefreshAcceptanceState()
{
	if (!TradeComponent)
		return;

	bool bWeAccepted = TradeComponent->HaveWeAccepted();
	bool bTheyAccepted = TradeComponent->HaveTheyAccepted();

	// Update our acceptance indicator
	if (OurAcceptedText)
	{
		OurAcceptedText->SetVisibility(bWeAccepted ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (OurAcceptedIcon)
	{
		OurAcceptedIcon->SetVisibility(bWeAccepted ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Update their acceptance indicator
	if (TheirAcceptedText)
	{
		TheirAcceptedText->SetVisibility(bTheyAccepted ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (TheirAcceptedIcon)
	{
		TheirAcceptedIcon->SetVisibility(bTheyAccepted ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Update accept button text
	if (AcceptButtonText)
	{
		FString ButtonText = bWeAccepted ? TEXT("Unaccept") : TEXT("Accept");
		AcceptButtonText->SetText(FText::FromString(ButtonText));
	}

	// Disable accept button if both accepted (trade will execute)
	if (AcceptButton)
	{
		AcceptButton->SetIsEnabled(!TradeComponent->BothAccepted());
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeWidget::RefreshStatusText()
{
	if (!StatusText || !TradeComponent)
		return;

	if (TradeComponent->BothAccepted())
	{
		StatusText->SetText(FText::FromString(TEXT("Trade completed!")));
	}
	else if (TradeComponent->HaveWeAccepted())
	{
		StatusText->SetText(FText::FromString(TEXT("Waiting for other player to accept...")));
	}
	else if (TradeComponent->HaveTheyAccepted())
	{
		StatusText->SetText(FText::FromString(TEXT("Other player is ready. Accept to complete trade.")));
	}
	else
	{
		StatusText->SetText(FText::FromString(TEXT("Adjust your offer and click Accept when ready.")));
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------------------------------------------------------------

UTradeSlotWidget* UTradeWidget::GetOurSlot(int32 Index)
{
	switch (Index)
	{
	case 0: return OurSlot0;
	case 1: return OurSlot1;
	case 2: return OurSlot2;
	case 3: return OurSlot3;
	case 4: return OurSlot4;
	case 5: return OurSlot5;
	case 6: return OurSlot6;
	case 7: return OurSlot7;
	default: return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

UTradeSlotWidget* UTradeWidget::GetTheirSlot(int32 Index)
{
	switch (Index)
	{
	case 0: return TheirSlot0;
	case 1: return TheirSlot1;
	case 2: return TheirSlot2;
	case 3: return TheirSlot3;
	case 4: return TheirSlot4;
	case 5: return TheirSlot5;
	case 6: return TheirSlot6;
	case 7: return TheirSlot7;
	default: return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

IInventoryPlayerInterface* UTradeWidget::GetInventoryInterface() const
{
	APlayerController* PC = GetOwningPlayer();
	return Cast<IInventoryPlayerInterface>(PC);
}

