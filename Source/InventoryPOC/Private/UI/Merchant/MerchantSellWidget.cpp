// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Merchant/MerchantSellWidget.h"

#include "CommonUtilities.h"
#include "Actors/MerchantActor.h"


UTexture2D* UMerchantSellWidget::GetTextureFomName(const FString& TextureRef)
{
	return LoadObject<UTexture2D>(nullptr, *TextureRef);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitListFromStatic(UMerchantItemListWidget* InputListWidget)
{
	InputListWidget->ClearList();
	for (auto& Data : GetStaticDataDisplayable())
	{
		InputListWidget->AddDataToList(Data);
	}
	DynamicStartID = GetStaticDataDisplayable().Num();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitListFromDynamic(UMerchantItemListWidget* InputListWidget)
{
	for (auto& Data : GetDynamicDataDisplayable())
	{
		InputListWidget->AddDataToList(Data);
		UE_LOG(LogTemp, Log, TEXT("Adding dynamic data %d %s %d"), Data.Id, *Data.Name, Data.Quantity);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UMerchantSellWidget::MerchantCanSell(int64 ItemID) const
{
	return MerchantActor->HasItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FMerchantItemDataStruct> UMerchantSellWidget::GetStaticDataDisplayable()
{
	TArray<FMerchantItemDataStruct> Out;
	if (MerchantActor)
	{
		for (const auto& Item : MerchantActor->GetStaticItemsConst())
		{
			FBareItem LocalBareItem = UCommonUtilities::GetItemFromID(Item,GetWorld());
			Out.Add({
				LocalBareItem.Key, GetTextureFomName(LocalBareItem.IconName), LocalBareItem.Name, -1,
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
			FBareItem LocalBareItem = UCommonUtilities::GetItemFromID(Item.ItemID,GetWorld());
			Out.Add({
				LocalBareItem.Key, GetTextureFomName(LocalBareItem.IconName), LocalBareItem.Name, Item.Quantity,
				MerchantActor->AdjustPriceSell({LocalBareItem.BaseValue})
			});
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::InitMerchantData(AMerchantActor* InputMerchantActor)
{
	if (!MerchantActor)
	{
		MerchantActor = InputMerchantActor;
		//Link to the merchant dispatcher here
		MerchantActor->GetMerchantDispatcher().AddDynamic(this, &UMerchantSellWidget::Refresh);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantSellWidget::DeInitMerchantData()
{
	if(MerchantActor)
	{
		MerchantActor->GetMerchantDispatcher().Clear();
		MerchantActor = nullptr;
	}
}
