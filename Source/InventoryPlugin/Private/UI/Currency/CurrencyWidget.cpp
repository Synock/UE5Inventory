#include "UI/Currency/CurrencyWidget.h"
#include "Interfaces/InventoryGameInstanceInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Update icon based on current currency type (shows in editor preview)
	UpdateCurrencyIcon();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyWidget::UpdateCurrencyIcon()
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

void UCurrencyWidget::Refresh()
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(CoinAmount));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyWidget::UpdateCoinValue(int32 NewCoinAmount)
{
	CoinAmount = NewCoinAmount;
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyWidget::SetCurrencyType(ECurrencyType NewCurrencyType)
{
	CurrencyType = NewCurrencyType;
	UpdateCurrencyIcon();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencyWidget::SetupCoinComponent(UCoinComponent* OriginCoinComponent, bool AllowForCurrencyChangeState)
{
	CoinComponent = OriginCoinComponent;
	AllowForCurrencyChange = AllowForCurrencyChangeState;
}

//----------------------------------------------------------------------------------------------------------------------

