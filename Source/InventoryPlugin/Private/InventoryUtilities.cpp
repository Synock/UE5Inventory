// Copyright 2022 Maximilien (Synock) Guislain

#include "InventoryUtilities.h"

//these includes are somehow needed 
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Interfaces/InventoryGameInstanceInterface.h"
#include "Interfaces/InventoryGameModeInterface.h"
#include "Interfaces/InventoryGameStateInterface.h"
#include "Kismet/GameplayStatics.h"

FVector2D UInventoryUtilities::GetItemFootPrint(EItemSize Size)
{
	int ItemWidth = 1;
	int ItemHeight = 1;

	switch (Size)
	{
	default:
	case EItemSize::Tiny:
		break;
	case EItemSize::Small:
		ItemWidth = 2;
		ItemHeight = 2;
		break;
	case EItemSize::Medium:
		ItemWidth = 3;
		ItemHeight = 3;
		break;
	case EItemSize::Large:
		ItemWidth = 4;
		ItemHeight = 4;
		break;
	case EItemSize::Giant:
		ItemWidth = 5;
		ItemHeight = 5;
		break;
	}
	return FVector2D(ItemWidth, ItemHeight);
}

//----------------------------------------------------------------------------------------------------------------------

FString UInventoryUtilities::GetItemSizeString(EItemSize Size)
{
	switch (Size)
	{
	default:
	case EItemSize::Tiny:
		return "Tiny";
	case EItemSize::Small:
		return "Small";
	case EItemSize::Medium:
		return "Medium";
	case EItemSize::Large:
		return "Large";
	case EItemSize::Giant:
		return "Gigantic";
	}
}

//----------------------------------------------------------------------------------------------------------------------

FString UInventoryUtilities::GetSlotName(EEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EEquipmentSlot::Primary: return "Primary";
	case EEquipmentSlot::Secondary: return "Secondary";
	case EEquipmentSlot::Range: return "Range";
	case EEquipmentSlot::Ammo: return "Ammo";
	case EEquipmentSlot::Head: return "Head";
	case EEquipmentSlot::Face: return "Face";
	case EEquipmentSlot::EarL: return "Ear (L)";
	case EEquipmentSlot::EarR: return "Ear (R)";
	case EEquipmentSlot::Neck: return "Neck";
	case EEquipmentSlot::Shoulders: return "Shoulders";
	case EEquipmentSlot::Back: return "Back";
	case EEquipmentSlot::Torso: return "Torso";
	case EEquipmentSlot::WristL: return "Wrist (L)";
	case EEquipmentSlot::WristR: return "Wrist (R)";
	case EEquipmentSlot::Hands: return "Hands";
	case EEquipmentSlot::FingerL: return "Finger (L)";
	case EEquipmentSlot::FingerR: return "Finger (R)";
	case EEquipmentSlot::Waist: return "Waist";
	case EEquipmentSlot::Legs: return "Legs";
	case EEquipmentSlot::Foot: return "Foot";
	case EEquipmentSlot::Arms: return "Arms";
	case EEquipmentSlot::WaistBag1: return "Waist Bag";
	case EEquipmentSlot::WaistBag2: return "Waist Bag";
	case EEquipmentSlot::BackPack1: return "Backpack";
	case EEquipmentSlot::BackPack2: return "Backpack";
	default: return {};
	}
}

//----------------------------------------------------------------------------------------------------------------------

UInventoryItemBase* UInventoryUtilities::GetItemFromID(int32 ItemID, UWorld* WorldContext)
{
	check(WorldContext);

	UInventoryItemBase* LocalItem = nullptr;

	if (IInventoryGameInstanceInterface* GI = Cast<IInventoryGameInstanceInterface>(
		UGameplayStatics::GetGameInstance(WorldContext)); GI)
	{
		LocalItem = GI->FetchItemFromID(ItemID);
	}
	else if (IInventoryGameModeInterface* GM = Cast<IInventoryGameModeInterface>(
			UGameplayStatics::GetGameMode(WorldContext))
		; GM)
	{
		LocalItem = GM->FetchItemFromID(ItemID);
	}
	else
	{
		IInventoryGameStateInterface* GS = Cast<IInventoryGameStateInterface>(
			UGameplayStatics::GetGameState(WorldContext));
		check(GS);
		if (GS)
			LocalItem = GS->FetchItemFromID(ItemID);
	}

	return LocalItem;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UInventoryUtilities::CoinValueFromFloat(float InputValue)
{
	return FCoinValue(InputValue);
}

//----------------------------------------------------------------------------------------------------------------------

float UInventoryUtilities::FloatFromCoinValue(const FCoinValue& CoinValue)
{
	return CoinValue.ToFloat();
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UInventoryUtilities::ReduceCoinAmount(const FCoinValue& CoinValue)
{
	FCoinValue Value = CoinValue;
	Value.ReduceCoinAmount();
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UInventoryUtilities::ConvertCoins(ECurrencyType InputCurrency, int32 InputValue,
                                             ECurrencyType OutputCurrency)
{
	float InputRawValue;
	float OutputRawValue;

	switch (InputCurrency)
	{
	default:
	case ECurrencyType::Copper: InputRawValue = 1.f;
		break;
	case ECurrencyType::Silver: InputRawValue = 10.f;
		break;
	case ECurrencyType::Gold: InputRawValue = 100.f;
		break;
	case ECurrencyType::Platinum: InputRawValue = 1000.f;
		break;
	}

	const float BaseValue = InputValue * InputRawValue;

	switch (OutputCurrency)
	{
	default:
	case ECurrencyType::Copper: OutputRawValue = 1.f;
		break;
	case ECurrencyType::Silver: OutputRawValue = 10.f;
		break;
	case ECurrencyType::Gold: OutputRawValue = 100.f;
		break;
	case ECurrencyType::Platinum: OutputRawValue = 1000.f;
		break;
	}

	const float ConversionRate = InputRawValue / OutputRawValue;
	const float NewValue = InputValue * ConversionRate;

	FCoinValue OutputValue;

	switch (OutputCurrency)
	{
	default:
	case ECurrencyType::Copper: OutputValue.CopperPieces = FMath::Floor(NewValue + 0.5);
		break;
	case ECurrencyType::Silver: OutputValue.SilverPieces = FMath::Floor(NewValue + 0.5);
		break;
	case ECurrencyType::Gold: OutputValue.GoldPieces = FMath::Floor(NewValue + 0.5);
		break;
	case ECurrencyType::Platinum: OutputValue.PlatinumPieces = FMath::Floor(NewValue + 0.5);
		break;
	}

	const float Rest = BaseValue - OutputValue.ToFloat();
	OutputValue += Rest;
	return OutputValue;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryUtilities::CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	return FCoinValue::CanPay(AvailableCoins, NeededCoins);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryUtilities::CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	return FCoinValue::CanPayWithChange(AvailableCoins, NeededCoins);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryUtilities::ComputeChange(FCoinValue& AvailableCoins, FCoinValue& NeededCoins)
{
	return FCoinValue::RetrieveValue(AvailableCoins, NeededCoins);
}

//----------------------------------------------------------------------------------------------------------------------

UTexture2D* UInventoryUtilities::LoadTexture2D(const FString& Path)
{
	if (Path.IsEmpty())
		return nullptr;

	return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path));
}

//----------------------------------------------------------------------------------------------------------------------

FString UInventoryUtilities::GetCoinValueAsString(const FCoinValue& CoinValue)
{
	FString ResultString;

	if (CoinValue.PlatinumPieces == 0 && CoinValue.GoldPieces == 0 && CoinValue.SilverPieces == 0 && CoinValue.
		CopperPieces == 0)
		return {};


	FString PlatinumString;
	FString GoldString;
	FString SilverString;
	FString CopperString;
	if (CoinValue.PlatinumPieces)
	{
		PlatinumString += FString::FormatAsNumber(CoinValue.PlatinumPieces) + " Platinum";
	}
	if (CoinValue.GoldPieces)
	{
		GoldString += FString::FormatAsNumber(CoinValue.GoldPieces) + " Gold";
	}
	if (CoinValue.SilverPieces)
	{
		SilverString += FString::FormatAsNumber(CoinValue.SilverPieces) + " Silver";
	}
	if (CoinValue.CopperPieces)
	{
		CopperString += FString::FormatAsNumber(CoinValue.CopperPieces) + " Copper";
	}

	if (!PlatinumString.IsEmpty())
		ResultString += PlatinumString;

	if (!PlatinumString.IsEmpty() && (!GoldString.IsEmpty() || !SilverString.IsEmpty() || !CopperString.IsEmpty()))
		ResultString += ", ";

	if (!GoldString.IsEmpty())
		ResultString += GoldString;

	if (!GoldString.IsEmpty() && (!SilverString.IsEmpty() || !CopperString.IsEmpty()))
		ResultString += ", ";

	if (!SilverString.IsEmpty())
		ResultString += SilverString;

	if (!SilverString.IsEmpty() && !CopperString.IsEmpty())
		ResultString += ", ";

	if (!CopperString.IsEmpty())
		ResultString += CopperString;

	ResultString += " pieces";

	return ResultString;
}
