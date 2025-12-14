#include "UI/Currency/CurrencySelectionWidget.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Interfaces/InventoryGameInstanceInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Update currency icon for editor preview
	UpdateCurrencyIcon();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::UpdateCurrencyIcon()
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

void UCurrencySelectionWidget::SetupUI()
{
	// Setup spinbox
	if (CurrencySpinbox)
	{
		CurrencySpinbox->SetMinValue(0.0f);
		CurrencySpinbox->SetMaxValue(static_cast<float>(MaximumCoinCapacity));
		CurrencySpinbox->SetValue(0.0f);
		CurrencySpinbox->OnValueChanged.AddDynamic(this, &UCurrencySelectionWidget::OnSpinboxValueChanged);
	}

	// Setup slider
	if (CurrencySlider)
	{
		CurrencySlider->SetMinValue(0.0f);
		CurrencySlider->SetMaxValue(static_cast<float>(MaximumCoinCapacity));
		CurrencySlider->SetValue(0.0f);
		CurrencySlider->OnValueChanged.AddDynamic(this, &UCurrencySelectionWidget::OnSliderValueChanged);
	}

	// Setup OK button
	if (OkButton)
	{
		OkButton->OnClicked.AddDynamic(this, &UCurrencySelectionWidget::OnOkButtonClicked);
	}

	// Update currency icon
	UpdateCurrencyIcon();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::OnSpinboxValueChanged(float Value)
{
	// Synchronize slider with spinbox
	if (CurrencySlider)
	{
		CurrencySlider->SetValue(Value);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::OnSliderValueChanged(float Value)
{
	// Synchronize spinbox with slider
	if (CurrencySpinbox)
	{
		CurrencySpinbox->SetValue(Value);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::OnOkButtonClicked()
{
	// Get the current value from spinbox
	int32 SelectedValue = 0;
	if (CurrencySpinbox)
	{
		SelectedValue = FMath::RoundToInt(CurrencySpinbox->GetValue());
	}

	// Execute the transfer
	DoTheCoinTransfer(SelectedValue);

	// Remove widget from parent
	RemoveFromParent();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::InitWidget(UCoinComponent* OriginCoinComponent, UCoinComponent* DestinationCoinComponent,
                                          ECurrencyType InputCurrencyType, ECurrencyType OutputCurrencyType, bool AllowForCurrencyChangeState)
{
	check(OriginCoinComponent);
	check(DestinationCoinComponent);

	CurrencyType = InputCurrencyType;
	Origin = OriginCoinComponent;
	Destination = DestinationCoinComponent;
	DesiredCurrencyType = OutputCurrencyType;

	// Determine maximum capacity based on currency type
	switch (CurrencyType)
	{
	default:
	case ECurrencyType::Copper:
		MaximumCoinCapacity = OriginCoinComponent->GetCP();
		break;
	case ECurrencyType::Silver:
		MaximumCoinCapacity = OriginCoinComponent->GetSP();
		break;
	case ECurrencyType::Gold:
		MaximumCoinCapacity = OriginCoinComponent->GetGP();
		break;
	case ECurrencyType::Platinum:
		MaximumCoinCapacity = OriginCoinComponent->GetPP();
		break;
	}

	AllowForCurrencyChange = AllowForCurrencyChangeState;

	// Setup all UI elements
	SetupUI();
}

//----------------------------------------------------------------------------------------------------------------------

void UCurrencySelectionWidget::DoTheCoinTransfer(int32 SelectedCoinValue)
{
	if (SelectedCoinValue == 0)
	{
		return;
	}

	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		FCoinValue RemovedValue = UInventoryUtilities::ConvertCoins(CurrencyType, SelectedCoinValue, CurrencyType);
		FCoinValue AddedValue = UInventoryUtilities::ConvertCoins(CurrencyType, SelectedCoinValue, DesiredCurrencyType);
		Player->TransferCoinTo(Origin, Destination, RemovedValue, AddedValue);
	}
}

//----------------------------------------------------------------------------------------------------------------------

