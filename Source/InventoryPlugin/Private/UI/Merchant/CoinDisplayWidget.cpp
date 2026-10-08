#include "UI/Merchant/CoinDisplayWidget.h"
#include "Interfaces/InventoryGameInstanceInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Try to get textures from GameInstance first (project-specific defaults)
	// This runs in both editor and runtime, allowing preview in the widget designer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (IInventoryGameInstanceInterface* GameInstanceInterface = Cast<IInventoryGameInstanceInterface>(GameInstance))
		{
			// Override with GameInstance textures if available
			if (UTexture2D* CopperTex = GameInstanceInterface->GetCopperCoinIconTexture())
			{
				CopperIconTexture = CopperTex;
			}
			if (UTexture2D* SilverTex = GameInstanceInterface->GetSilverCoinIconTexture())
			{
				SilverIconTexture = SilverTex;
			}
			if (UTexture2D* GoldTex = GameInstanceInterface->GetGoldCoinIconTexture())
			{
				GoldIconTexture = GoldTex;
			}
			if (UTexture2D* PlatinumTex = GameInstanceInterface->GetPlatinumCoinIconTexture())
			{
				PlatinumIconTexture = PlatinumTex;
			}
		}
	}

	// Apply icon textures (either from GameInstance or widget's configured textures)
	ApplyIconTextures();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetCoinValue(const FCoinValue& CoinValue)
{
	CopperPieces = CoinValue.CopperPieces;
	SilverPieces = CoinValue.SilverPieces;
	GoldPieces = CoinValue.GoldPieces;
	PlatinumPieces = CoinValue.PlatinumPieces;

	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetCopperPieces(int32 Value)
{
	CopperPieces = Value;
	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetSilverPieces(int32 Value)
{
	SilverPieces = Value;
	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetGoldPieces(int32 Value)
{
	GoldPieces = Value;
	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetPlatinumPieces(int32 Value)
{
	PlatinumPieces = Value;
	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetAllCurrencyValues(int32 Copper, int32 Silver, int32 Gold, int32 Platinum)
{
	CopperPieces = Copper;
	SilverPieces = Silver;
	GoldPieces = Gold;
	PlatinumPieces = Platinum;

	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue UCoinDisplayWidget::GetCoinValue() const
{
	return FCoinValue(CopperPieces, SilverPieces, GoldPieces, PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::Clear()
{
	CopperPieces = 0;
	SilverPieces = 0;
	GoldPieces = 0;
	PlatinumPieces = 0;

	UpdateDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetShowZeroValues(bool bShow)
{
	bShowZeroValues = bShow;
	UpdateVisibility();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::SetShowIcons(bool bShow)
{
	bShowIcons = bShow;
	UpdateVisibility();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::UpdateDisplay()
{
	// Update text blocks if they exist
	if (CopperText)
	{
		CopperText->SetText(FText::AsNumber(CopperPieces));
	}

	if (SilverText)
	{
		SilverText->SetText(FText::AsNumber(SilverPieces));
	}

	if (GoldText)
	{
		GoldText->SetText(FText::AsNumber(GoldPieces));
	}

	if (PlatinumText)
	{
		PlatinumText->SetText(FText::AsNumber(PlatinumPieces));
	}

	UpdateVisibility();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::UpdateVisibility()
{
	// Handle Copper visibility
	if (CopperIcon)
	{
		const bool bShowCopper = bShowZeroValues || CopperPieces > 0;
		CopperIcon->SetVisibility(bShowCopper && bShowIcons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (CopperText)
	{
		const bool bShowCopper = bShowZeroValues || CopperPieces > 0;
		CopperText->SetVisibility(bShowCopper ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Handle Silver visibility
	if (SilverIcon)
	{
		const bool bShowSilver = bShowZeroValues || SilverPieces > 0;
		SilverIcon->SetVisibility(bShowSilver && bShowIcons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (SilverText)
	{
		const bool bShowSilver = bShowZeroValues || SilverPieces > 0;
		SilverText->SetVisibility(bShowSilver ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Handle Gold visibility
	if (GoldIcon)
	{
		const bool bShowGold = bShowZeroValues || GoldPieces > 0;
		GoldIcon->SetVisibility(bShowGold && bShowIcons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (GoldText)
	{
		const bool bShowGold = bShowZeroValues || GoldPieces > 0;
		GoldText->SetVisibility(bShowGold ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Handle Platinum visibility
	if (PlatinumIcon)
	{
		const bool bShowPlatinum = bShowZeroValues || PlatinumPieces > 0;
		PlatinumIcon->SetVisibility(bShowPlatinum && bShowIcons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (PlatinumText)
	{
		const bool bShowPlatinum = bShowZeroValues || PlatinumPieces > 0;
		PlatinumText->SetVisibility(bShowPlatinum ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinDisplayWidget::ApplyIconTextures()
{
	// Apply copper icon texture
	if (CopperIcon && CopperIconTexture)
	{
		CopperIcon->SetBrushFromTexture(CopperIconTexture);
	}

	// Apply silver icon texture
	if (SilverIcon && SilverIconTexture)
	{
		SilverIcon->SetBrushFromTexture(SilverIconTexture);
	}

	// Apply gold icon texture
	if (GoldIcon && GoldIconTexture)
	{
		GoldIcon->SetBrushFromTexture(GoldIconTexture);
	}

	// Apply platinum icon texture
	if (PlatinumIcon && PlatinumIconTexture)
	{
		PlatinumIcon->SetBrushFromTexture(PlatinumIconTexture);
	}
}

//----------------------------------------------------------------------------------------------------------------------

