#include "UI/Currency/CurrencyFloatingItem.h"
#include "Interfaces/InventoryGameInstanceInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyFloatingItem::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Update UI for editor preview
	UpdateCurrencyIcon();
	UpdateCurrencyText();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyFloatingItem::UpdateCurrencyIcon()
{
	if (!CurrencyIcon)
		return;

	// Try to get texture from GameInstance based on currency type
	UTexture2D* IconTexture = nullptr;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (IInventoryGameInstanceInterface* GameInstanceInterface = Cast<IInventoryGameInstanceInterface>(GameInstance))
		{
			// Get the appropriate texture based on currency type
			switch (CurrencyType)
			{
			case ECurrencyType::Copper:
				IconTexture = GameInstanceInterface->GetCopperCoinIconTexture();
				break;
			case ECurrencyType::Silver:
				IconTexture = GameInstanceInterface->GetSilverCoinIconTexture();
				break;
			case ECurrencyType::Gold:
				IconTexture = GameInstanceInterface->GetGoldCoinIconTexture();
				break;
			case ECurrencyType::Platinum:
				IconTexture = GameInstanceInterface->GetPlatinumCoinIconTexture();
				break;
			}
		}
	}

	// Apply the texture if found
	if (IconTexture)
	{
		CurrencyIcon->SetBrushFromTexture(IconTexture);
		CurrencyIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyFloatingItem::UpdateCurrencyText()
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(CoinAmount));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyFloatingItem::SetupUI()
{
	UpdateCurrencyIcon();
	UpdateCurrencyText();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyFloatingItem::InitWidget(UCoinComponent* CoinOriginPointer, int32 InputCoinAmount,
                                       ECurrencyType InputCurrencyType)
{
	CoinOrigin = CoinOriginPointer;
	CoinAmount = InputCoinAmount;
	CurrencyType = InputCurrencyType;

	SetupUI();
}

//----------------------------------------------------------------------------------------------------------------------

