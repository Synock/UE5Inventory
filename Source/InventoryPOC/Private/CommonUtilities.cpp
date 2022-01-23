// Copyright 2022 Maximilien (Synock) Guislain


#include "CommonUtilities.h"

#include "InventoryPOC/InventoryPOCGameMode.h"
#include "MainGameState.h"
#include "Kismet/GameplayStatics.h"

FBareItem UCommonUtilities::GetItemFromID(int64 ItemID, UWorld* WorldContext)
{
	check(WorldContext);
	AInventoryPOCGameMode* GM = Cast<AInventoryPOCGameMode>(UGameplayStatics::GetGameMode(WorldContext));
	FBareItem LocalItem;
	if (GM)
	{
		LocalItem = GM->FetchItemFromID(ItemID);
	}
	else
	{
		AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(WorldContext));
		check(GS);
		if (GS)
			LocalItem = GS->FetchItemFromID(ItemID);
	}

	return LocalItem;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UCommonUtilities::CoinValueFromFloat(float InputValue)
{
	return FCoinValue(InputValue);
}

//----------------------------------------------------------------------------------------------------------------------

float UCommonUtilities::FloatFromCoinValue(const FCoinValue& CoinValue)
{
	return CoinValue.ToFloat();
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UCommonUtilities::ReduceCoinAmount(const FCoinValue& CoinValue)
{
	FCoinValue Value = CoinValue;
	Value.ReduceCoinAmount();
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

bool UCommonUtilities::CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	return FCoinValue::CanPay(AvailableCoins, NeededCoins);
}

//----------------------------------------------------------------------------------------------------------------------

bool UCommonUtilities::CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	return FCoinValue::CanPayWithChange(AvailableCoins, NeededCoins);
}

//----------------------------------------------------------------------------------------------------------------------

bool UCommonUtilities::ComputeChange(FCoinValue& AvailableCoins, FCoinValue& NeededCoins)
{
	return FCoinValue::RetrieveValue(AvailableCoins, NeededCoins);
}
