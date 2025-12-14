#include "UI/Merchant/MerchantSellWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Components/MerchantComponent.h"
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
	return MerchantActor->HasItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UMerchantSellWidget::GetCorrectPrice(float FloatValue) const
{
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

	if (!PC)
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
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HandleSellClick()
{
	IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer());

	if (!PC)
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

	// Update button
	if (BuySellButton)
	{
		BuySellButton->SetIsEnabled(!IsWorthless());
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
			if(!LocalBareItem)
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
			if (const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld()); LocalBareItem)
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
	const UInventoryItemBase* LocalBareItem = UInventoryUtilities::GetItemFromID(SelectedItemId, GetWorld());
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
	if(IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		Player->StopMerchantTrade();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitMerchantData(AActor* InputMerchantActor)
{
	if (!MerchantActor)
	{
		if (InputMerchantActor->Implements<UMerchantInterface>())
		{
			MerchantActor.SetObject(InputMerchantActor);
			MerchantActor.SetInterface(Cast<IMerchantInterface>(InputMerchantActor));

			if (MerchantPurse)
			{
				MerchantPurse->InitWidget(MerchantActor->GetCoinComponent());
			}

			MerchantActor->GetMerchantDispatcher().AddDynamic(this, &UMerchantSellWidget::Refresh);
			MerchantActor->GetCoinComponent()->PurseDispatcher.AddDynamic(this, &UMerchantSellWidget::Refresh);

			// Bind to item list selection changes
			if (ItemList)
			{
				ItemList->SelectionChangedDelegate.AddDynamic(this, &UMerchantSellWidget::OnItemListSelectionChanged);
			}

			// Bind BuySellButton click event
			if (BuySellButton)
			{
				BuySellButton->OnClicked.AddDynamic(this, &UMerchantSellWidget::OnBuySellButtonClicked);
			}

			// Bind DoneButton click event (optional)
			if (DoneButton)
			{
				DoneButton->OnClicked.AddDynamic(this, &UMerchantSellWidget::OnDoneButtonClicked);
			}

			if (MerchantNameText)
			{
				MerchantNameText->SetText(FText::FromString(MerchantActor->GetMerchantName()));
			}

			Refresh();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::DeInitMerchantData()
{
	if (MerchantActor)
	{
		MerchantActor->GetMerchantDispatcher().Clear();
		MerchantActor->GetCoinComponent()->PurseDispatcher.Clear();
		MerchantActor = nullptr;
	}

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
	AssignSellData(0,0,EBagSlot::Unknown);
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
