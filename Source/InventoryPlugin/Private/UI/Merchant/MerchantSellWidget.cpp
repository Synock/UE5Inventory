#include "UI/Merchant/MerchantSellWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Components/MerchantComponent.h"
#include "TimerManager.h"
#include "Items/InventoryItemBase.h"

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnBuySellButtonClicked()
{
	// Route to correct handler based on merchant mode
	if (MerchantMode == EMerchantWindowMode::Sell)
	{
		// Merchant is selling to player (player buying from merchant)
		HandleBuyClick();
	}
	else if (MerchantMode == EMerchantWindowMode::Buy)
	{
		// Merchant is buying from player (player selling to merchant)
		HandleSellClick();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnDoneButtonClicked()
{
	StopTrading();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitListFromStatic(UMerchantItemListWidget* InputListWidget)
{
	InputListWidget->ClearList();
	for (auto& Data : GetStaticDataDisplayable())
	{
		InputListWidget->AddDataToList(Data);
	}
	DynamicStartID = GetStaticDataDisplayable().Num() - 1;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitListFromDynamic(UMerchantItemListWidget* InputListWidget)
{
	for (auto& Data : GetDynamicDataDisplayable())
	{
		InputListWidget->AddDataToList(Data);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UMerchantSellWidget::MerchantCanSell(int32 ItemID) const
{
	return MerchantActor && ItemID > 0 && MerchantActor->HasItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UMerchantSellWidget::CanMerchantAcceptItem(const UInventoryItemBase* Item) const
{
	if (!MerchantActor)
		return false;

	FText Reason;
	return MerchantActor->CanAcceptItemType(Item, Reason);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UMerchantSellWidget::GetCorrectPrice(float FloatValue) const
{
	if (!MerchantActor)
		return {};

	FCoinValue BaseValue = UInventoryUtilities::CoinValueFromFloat(FloatValue);

	if (MerchantMode == EMerchantWindowMode::Sell)
	{
		BaseValue = MerchantActor->AdjustPriceSell(BaseValue);
	}
	else
	{
		BaseValue = MerchantActor->AdjustPriceBuy(BaseValue);
	}

	return BaseValue;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HandleBuyClick()
{
	IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

	if (!PC || !MerchantActor || SelectedItemId <= 0)
		return;

	if (!PC->PlayerCanPutItemSomewhere(SelectedItemId))
	{
		OnNotEnoughPlayerSpace();
		return;
	}

	const FCoinValue TransactionValue = GetSelectedItemPrice();

	if (!PC->PlayerCanPayAmount(TransactionValue))
	{
		OnNotEnoughPlayerMoney();
		return;
	}

	PC->PlayerBuyFromMerchant(SelectedItemId, TransactionValue);

	if (BuySellButton)
	{
		BuySellButton->SetIsEnabled(false);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TransactionRefreshTimer, this, &UMerchantSellWidget::Refresh, 0.25f, false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HandleSellClick()
{
	IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

	if (!PC || !MerchantActor || SelectedItemId <= 0 || MerchantBuyOriginSlot == EBagSlot::Unknown ||
		MerchantBuyOriginTopLeft < 0)
		return;

	const FCoinValue TransactionValue = GetSelectedItemPrice();

	if (!MerchantActor->CanPayAmount(TransactionValue))
	{
		OnNotEnoughMerchantMoney();
		return;
	}

	PC->PlayerSellToMerchant(MerchantBuyOriginSlot, SelectedItemId, MerchantBuyOriginTopLeft, TransactionValue);

	MerchantBuyOriginSlot = EBagSlot::Unknown;
	MerchantBuyOriginTopLeft = -1;
	SelectedItemId = 0;

	UpdateItemPreview();

	if (BuySellButton)
	{
		BuySellButton->SetIsEnabled(false);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TransactionRefreshTimer, this, &UMerchantSellWidget::Refresh, 0.25f, false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::UpdateItemPreview()
{
	if (!ItemIconPreview)
		return;

	if (SelectedItemId <= 0)
	{
		HideItemPreview();
		return;
	}

	const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(SelectedItemId, GetWorld());

	if (!LocalBareItem)
	{
		HideItemPreview();
		return;
	}

	// Update icon
	if (UTexture2D* Image = LocalBareItem->Icon)
	{
		ItemIconPreview->SetBrushFromTexture(Image);
		ItemIconPreview->SetVisibility(ESlateVisibility::Visible);
	}

	// Update name
	if (ItemName)
	{
		ItemName->SetText(FText::FromString(LocalBareItem->Name));
		ItemName->SetVisibility(ESlateVisibility::Visible);
	}

	// Update price
	UpdatePricePreview();

	// Update button - disable if worthless OR if merchant won't accept item type
	if (BuySellButton)
	{
		bool bCanSell = !IsWorthless();

		// In Buy mode (player selling to merchant), also check if merchant accepts this item type
		if (MerchantMode == EMerchantWindowMode::Buy && bCanSell)
		{
			bCanSell = CanMerchantAcceptItem(LocalBareItem);
		}

		BuySellButton->SetIsEnabled(bCanSell);
	}

	if (BuySellButtonText)
	{
		BuySellButtonText->SetText(MerchantMode != EMerchantWindowMode::Buy
			                           ? FText::FromString("Buy")
			                           : FText::FromString("Sell"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HideItemPreview()
{
	if (ItemIconPreview)
	{
		ItemIconPreview->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ItemName)
	{
		ItemName->SetVisibility(ESlateVisibility::Hidden);
	}

	// Hide text-based price if present
	if (ItemPriceText)
	{
		ItemPriceText->SetVisibility(ESlateVisibility::Hidden);
	}

	// Hide coin display widget if present
	if (ItemPrice)
	{
		ItemPrice->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BuySellButton)
	{
		BuySellButton->SetIsEnabled(false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::UpdatePricePreview()
{
	const FCoinValue CurrentValue = GetSelectedItemPrice();

	// Prefer CoinDisplayWidget if available
	if (ItemPrice)
	{
		ItemPrice->SetCoinValue(CurrentValue);
		ItemPrice->SetVisibility(ESlateVisibility::Visible);
	}
	// Fall back to TextBlock display
	else if (ItemPriceText)
	{
		const FString PriceString = FString::FormatAsNumber(CurrentValue.PlatinumPieces) + "p " +
			FString::FormatAsNumber(CurrentValue.GoldPieces) + "g " +
			FString::FormatAsNumber(CurrentValue.SilverPieces) + "s " +
			FString::FormatAsNumber(CurrentValue.CopperPieces) + "c";

		ItemPriceText->SetText(FText::FromString(PriceString));
		ItemPriceText->SetVisibility(ESlateVisibility::Visible);
	}
}


//----------------------------------------------------------------------------------------------------------------------

TArray<FMerchantItemDataStruct> UMerchantSellWidget::GetStaticDataDisplayable()
{
	TArray<FMerchantItemDataStruct> Out;

	if (MerchantActor)
	{
		for (const auto& Item : MerchantActor->GetStaticItemsConst())
		{
			const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(Item, GetWorld());
			if (!LocalBareItem)
				continue;

			Out.Add({
				LocalBareItem->ItemID, LocalBareItem->Icon, LocalBareItem->Name,
				-1,
				MerchantActor->AdjustPriceSell({LocalBareItem->BaseValue})
			});
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FMerchantItemDataStruct> UMerchantSellWidget::GetDynamicDataDisplayable()
{
	TArray<FMerchantItemDataStruct> Out;

	if (MerchantActor)
	{
		for (const auto& Item : MerchantActor->GetDynamicItemsConst())
		{
			if (const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld());
				LocalBareItem)
			{
				Out.Add({
					LocalBareItem->ItemID, LocalBareItem->Icon,
					LocalBareItem->Name, Item.Quantity,
					MerchantActor->AdjustPriceSell({LocalBareItem->BaseValue})
				});
			}
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UMerchantSellWidget::GetSelectedItemPrice() const
{
	if (!MerchantActor || SelectedItemId <= 0)
		return {};

	const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(SelectedItemId, GetWorld());
	if (!LocalBareItem)
		return {};

	const FCoinValue TransactionValue = GetCorrectPrice(LocalBareItem->BaseValue);

	return TransactionValue;
}

//----------------------------------------------------------------------------------------------------------------------

bool UMerchantSellWidget::IsWorthless()
{
	const FCoinValue CurrentValue = GetSelectedItemPrice();
	if (CurrentValue.CopperPieces != 0)
		return false;

	if (CurrentValue.SilverPieces != 0)
		return false;

	if (CurrentValue.GoldPieces != 0)
		return false;

	if (CurrentValue.PlatinumPieces != 0)
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::StopTrading()
{
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		Player->StopMerchantTrade();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::ResetMerchantSessionState()
{
	SelectedItemId = 0;
	DynamicStartID = 0;
	MerchantCanBuy = true;
	MerchantBuyOriginSlot = EBagSlot::Unknown;
	MerchantBuyOriginTopLeft = -1;
	MerchantMode = EMerchantWindowMode::Sell;

	if (ItemList)
	{
		ItemList->ClearSelection();
		ItemList->ClearList();
	}

	if (MerchantPurse)
		MerchantPurse->InitWidget(nullptr);

	if (MerchantNameText)
		MerchantNameText->SetText(FText::GetEmpty());

	HideItemPreview();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitMerchantData(AActor* InputMerchantActor)
{
	if (!InputMerchantActor || !InputMerchantActor->Implements<UMerchantInterface>())
		return;

	if (MerchantActor.GetObject() == InputMerchantActor)
	{
		Refresh();
		return;
	}

	// Replication can replace one merchant actor with another without delivering an
	// intermediate nullptr. Always release the old delegates before binding the new session.
	DeInitMerchantData();

	MerchantActor.SetObject(InputMerchantActor);
	MerchantActor.SetInterface(Cast<IMerchantInterface>(InputMerchantActor));

	if (MerchantPurse)
	{
		MerchantPurse->InitWidget(MerchantActor->GetCoinComponent());
	}

	MerchantActor->GetMerchantDispatcher().AddUniqueDynamic(this, &UMerchantSellWidget::Refresh);
	MerchantActor->GetCoinComponent()->PurseDispatcher.AddUniqueDynamic(this, &UMerchantSellWidget::Refresh);

	// Bind to item list selection changes
	if (ItemList)
	{
		ItemList->SelectionChangedDelegate.AddUniqueDynamic(this, &UMerchantSellWidget::OnItemListSelectionChanged);
	}

	// Bind BuySellButton click event
	if (BuySellButton)
	{
		BuySellButton->OnClicked.AddUniqueDynamic(this, &UMerchantSellWidget::OnBuySellButtonClicked);
	}

	// Bind DoneButton click event (optional)
	if (DoneButton)
	{
		DoneButton->OnClicked.AddUniqueDynamic(this, &UMerchantSellWidget::OnDoneButtonClicked);
	}

	if (MerchantNameText)
	{
		MerchantNameText->SetText(FText::FromString(MerchantActor->GetMerchantName()));
	}

	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::DeInitMerchantData()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TransactionRefreshTimer);

	if (MerchantActor)
	{
		MerchantActor->GetMerchantDispatcher().RemoveDynamic(this, &UMerchantSellWidget::Refresh);
		if (UCoinComponent* CoinComponent = MerchantActor->GetCoinComponent())
			CoinComponent->PurseDispatcher.RemoveDynamic(this, &UMerchantSellWidget::Refresh);

		MerchantActor = nullptr;
	}

	ResetMerchantSessionState();

	// Unbind from item list selection changes
	if (ItemList)
	{
		ItemList->SelectionChangedDelegate.RemoveAll(this);
	}

	// Unbind button click events
	if (BuySellButton)
	{
		BuySellButton->OnClicked.RemoveAll(this);
	}

	if (DoneButton)
	{
		DoneButton->OnClicked.RemoveAll(this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::Refresh()
{
	if (!MerchantActor)
	{
		HideItemPreview();
		return;
	}

	if (MerchantPurse)
		MerchantPurse->Refresh();


	if (ItemList)
	{
		ItemList->ClearList();
		InitListFromStatic(ItemList);
		InitListFromDynamic(ItemList);
	}

	if (MerchantCanSell(SelectedItemId))
	{
		if (BuySellButton)
		{
			BuySellButton->SetIsEnabled(true);
		}
	}
	else
	{
		SelectedItemId = 0;
		UpdateItemPreview();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::ResetSellData()
{
	AssignSellData(0, 0, EBagSlot::Unknown);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::AssignSellData(int32 ItemID, int32 TopLeft, EBagSlot OriginBag)
{
	MerchantBuyOriginSlot = OriginBag;
	MerchantBuyOriginTopLeft = TopLeft;
	SelectedItemId = ItemID;
	MerchantMode = EMerchantWindowMode::Buy;

	UpdateItemPreview();

	if (ItemList)
	{
		ItemList->ClearSelection();
	}


	// Broadcast price quote for the item
	if (MerchantActor && ItemID > 0)
	{
		// Check if merchant accepts this item type

		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
		if (Item)
		{
			FText Reason;
			if (!MerchantActor->CanAcceptItemType(Item, Reason))
			{
				// Merchant doesn't accept this item type - notify and don't allow the sale
				OnMerchantRejectsItemType(MerchantActor->GetMerchantName(), Item->GetItemName());
				return;
			}

			const FCoinValue OfferPrice = GetSelectedItemPrice();
			OnMerchantOffersPriceQuote(MerchantActor->GetMerchantName(), Item->Name, OfferPrice);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnItemListSelectionChanged(int32 ItemID)
{
	// Reset buy-specific data
	MerchantBuyOriginSlot = EBagSlot::Unknown;
	MerchantBuyOriginTopLeft = -1;

	// Update selected item
	SelectedItemId = ItemID;

	// Set mode to Sell (player buying from merchant)
	MerchantMode = EMerchantWindowMode::Sell;

	// Update the item preview
	UpdateItemPreview();

	// Broadcast that merchant is offering this item for sale
	if (MerchantActor && ItemID > 0)
	{
		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetWorld());
		if (Item)
		{
			const FCoinValue SalePrice = GetSelectedItemPrice();
			OnMerchantOffersItemForSale(MerchantActor->GetMerchantName(), Item->Name, SalePrice);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughPlayerMoney()
{
	OnNotEnoughPlayerMoneyDelegate.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughPlayerSpace()
{
	OnNotEnoughPlayerSpaceDelegate.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughMerchantMoney()
{
	OnNotEnoughMerchantMoneyDelegate.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnMerchantRejectsItemType(const FString& MerchantName, const FString& RefusedItemName)
{
	OnMerchantRejectsItemTypeDelegate.Broadcast(MerchantName, RefusedItemName);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnMerchantOffersPriceQuote(const FString& MerchantName, const FString& ItemOfferName,
                                                     const FCoinValue& OfferPrice)
{
	OnMerchantOffersPriceQuoteDelegate.Broadcast(MerchantName, ItemOfferName, OfferPrice);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnMerchantOffersItemForSale(const FString& MerchantName, const FString& ItemOfferName,
                                                       const FCoinValue& SalePrice)
{
	OnMerchantOffersItemForSaleDelegate.Broadcast(MerchantName, ItemOfferName, SalePrice);
}

//----------------------------------------------------------------------------------------------------------------------
// IInventoryMerchantWindowInterface
//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitMerchantWindow_Implementation(AActor* NewMerchantActor)
{
	InitMerchantData(NewMerchantActor);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::DeInitMerchantWindow_Implementation()
{
	DeInitMerchantData();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::ShowMerchantWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HideMerchantWindow_Implementation()
{
	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::RefreshMerchantWindow_Implementation()
{
	Refresh();
}

