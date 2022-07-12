// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Merchant/MerchantSellWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Components/MerchantComponent.h"

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

	BuySellButtonPointer->SetIsEnabled(false);
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

	BuySellButtonPointer->SetIsEnabled(false);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::UpdateItemPreview()
{
	if (!ImageIconPreviewPointer)
		return;

	if (SelectedItemId <= 0)
	{
		HideItemPreview();
		return;
	}

	FInventoryItem LocalBareItem = UInventoryUtilities::GetItemFromID(SelectedItemId, GetWorld());

	UTexture2D* Image = UInventoryUtilities::LoadTexture2D(LocalBareItem.IconName);

	if (Image)
	{
		ImageIconPreviewPointer->SetBrushFromTexture(Image);
		ImageIconPreviewPointer->SetVisibility(ESlateVisibility::Visible);
	}


	CurrentItemNamePointer->SetText(FText::FromString(LocalBareItem.Name));
	CurrentItemNamePointer->SetVisibility(ESlateVisibility::Visible);

	UpdatePricePreview();

	BuySellButtonPointer->SetIsEnabled(!IsWorthless());

	BuySellButtonTextPointer->SetText(MerchantMode != EMerchantWindowMode::Buy
		                                  ? FText::FromString("Buy")
		                                  : FText::FromString("Sell"));
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::HideItemPreview()
{
	ImageIconPreviewPointer->SetVisibility(ESlateVisibility::Hidden);
	CurrentItemNamePointer->SetVisibility(ESlateVisibility::Hidden);
	CurrentItemPricePointer->SetVisibility(ESlateVisibility::Hidden);
	BuySellButtonPointer->SetIsEnabled(false);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::UpdatePricePreview()
{
	const FCoinValue CurrentValue = GetSelectedItemPrice();
	const FString PriceString = FString::FormatAsNumber(CurrentValue.PlatinumPieces) + "p" +
		FString::FormatAsNumber(CurrentValue.GoldPieces) + "g" +
		FString::FormatAsNumber(CurrentValue.SilverPieces) + "s" +
		FString::FormatAsNumber(CurrentValue.CopperPieces) + "c";

	CurrentItemPricePointer->SetText(FText::FromString(PriceString));
	CurrentItemPricePointer->SetVisibility(ESlateVisibility::Visible);
}


//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitUIInternal(UButton* BuySellButton, UImage* ImageIconPreview, UTextBlock* CurrentItemName,
                                         UTextBlock* BuySellButtonText, UTextBlock* CurrentItemPrice,
                                         UMerchantItemListWidget* ItemListWidget,
                                         UPurseWidget* MerchantPurseWidget, UTextBlock* MerchantNameText)
{
	BuySellButtonPointer = BuySellButton;
	ImageIconPreviewPointer = ImageIconPreview;
	CurrentItemNamePointer = CurrentItemName;
	BuySellButtonTextPointer = BuySellButtonText;
	CurrentItemPricePointer = CurrentItemPrice;
	ListWidgetPointer = ItemListWidget;
	MerchantPursePointer = MerchantPurseWidget;
	MerchantNameTextPointer = MerchantNameText;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FMerchantItemDataStruct> UMerchantSellWidget::GetStaticDataDisplayable()
{
	TArray<FMerchantItemDataStruct> Out;

	if (MerchantActor)
	{
		for (const auto& Item : MerchantActor->GetStaticItemsConst())
		{
			FInventoryItem LocalBareItem = UInventoryUtilities::GetItemFromID(Item, GetWorld());
			Out.Add({
				LocalBareItem.ItemID, UInventoryUtilities::LoadTexture2D(LocalBareItem.IconName), LocalBareItem.Name,
				-1,
				MerchantActor->AdjustPriceSell({LocalBareItem.BaseValue})
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
			if (FInventoryItem LocalBareItem = UInventoryUtilities::GetItemFromID(Item.ItemID, GetWorld()); LocalBareItem.ItemID >= 0)
			{
				Out.Add({
				   LocalBareItem.ItemID, UInventoryUtilities::LoadTexture2D(LocalBareItem.IconName),
				   LocalBareItem.Name, Item.Quantity,
				   MerchantActor->AdjustPriceSell({LocalBareItem.BaseValue})
			   });
			}
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UMerchantSellWidget::GetSelectedItemPrice() const
{
	const FInventoryItem LocalBareItem = UInventoryUtilities::GetItemFromID(SelectedItemId, GetWorld());
	const FCoinValue TransactionValue = GetCorrectPrice(LocalBareItem.BaseValue);

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

			MerchantPursePointer->InitWidget(MerchantActor->GetCoinComponent());
			MerchantActor->GetMerchantDispatcher().AddDynamic(this, &UMerchantSellWidget::Refresh);
			MerchantActor->GetCoinComponent()->PurseDispatcher.AddDynamic(this, &UMerchantSellWidget::Refresh);

			MerchantNameTextPointer->SetText(FText::FromString(MerchantActor->GetMerchantName()));

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
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::Refresh()
{
	MerchantPursePointer->Refresh();
	ListWidgetPointer->ClearList();
	InitListFromStatic(ListWidgetPointer);
	InitListFromDynamic(ListWidgetPointer);
	if(MerchantCanSell(SelectedItemId))
	{
		BuySellButtonPointer->SetIsEnabled(true);
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
	ListWidgetPointer->ClearSelection();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughPlayerMoney()
{
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughPlayerSpace()
{
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::OnNotEnoughMerchantMoney()
{
}
