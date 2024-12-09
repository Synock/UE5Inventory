// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/MerchantInterface.h"

#include "InventoryUtilities.h"
#include "Components/CoinComponent.h"
#include "Components/MerchantComponent.h"
#include "Items/InventoryItemBase.h"

void IMerchantInterface::InitMerchantData(const TArray<int32>& StaticItems,
                                          const TArray<FMerchantDynamicItemStorage>& DynamicItems,
                                          const FCoinValue& Coins)
{
	check(GetMerchantComponent());
	GetMerchantComponent()->InitStatic(StaticItems);
	GetMerchantComponent()->InitDynamic(DynamicItems);
	GetCoinComponent()->AddCoins(Coins);
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<int32>& IMerchantInterface::GetStaticItemsConst() const
{
	return GetMerchantComponentConst()->GetStaticItemsConst();
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMerchantDynamicItemStorage>& IMerchantInterface::GetDynamicItemsConst() const
{
	return GetMerchantComponentConst()->GetDynamicItemsConst();
}

//----------------------------------------------------------------------------------------------------------------------

void IMerchantInterface::RemoveItemAmountIfNeeded(int32 ItemID)
{
	GetMerchantComponent()->RemoveItemID(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

void IMerchantInterface::PayCoin(const FCoinValue& CoinValue)
{
	GetCoinComponent()->PayAndAdjustSimple(CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void IMerchantInterface::ReceiveCoin(const FCoinValue& CoinValue)
{
	GetCoinComponent()->AddCoins(CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

bool IMerchantInterface::HasItem(int32 ItemID) const
{
	return GetMerchantComponentConst()->HasItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

bool IMerchantInterface::CanPayAmount(const FCoinValue& CoinValue) const
{
	return FCoinValue::CanPayWithChange(GetCoinComponentConst()->GetPurseContent(), CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue IMerchantInterface::AdjustPriceBuy(const FCoinValue& CoinValue) const
{
	FCoinValue Value = CoinValue;
	Value *= GetMerchantRatio();
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue IMerchantInterface::AdjustPriceSell(const FCoinValue& CoinValue) const
{
	FCoinValue Value = CoinValue;
	Value *= (2.0 - GetMerchantRatio());
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue IMerchantInterface::GetItemPriceBuy(int32 ItemID) const
{
	const FCoinValue Price(UInventoryUtilities::GetItemFromID(ItemID, GetMerchantWorldContext())->BaseValue);
	return AdjustPriceBuy(Price);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue IMerchantInterface::GetItemPriceSell(int32 ItemID) const
{
	const FCoinValue Price(UInventoryUtilities::GetItemFromID(ItemID, GetMerchantWorldContext())->BaseValue);
	return AdjustPriceSell(Price);
}

//----------------------------------------------------------------------------------------------------------------------

void IMerchantInterface::AddDynamicItem(int32 ItemID)
{
	GetMerchantComponent()->AddItem(ItemID);
}
