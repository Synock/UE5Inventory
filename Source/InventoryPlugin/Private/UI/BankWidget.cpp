
#include "UI/BankWidget.h"
#include "InventoryPlugin.h"

#include "Components/BankComponent.h"
#include "InventoryPlugin.h"
#include "Components/Button.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "InventoryPlugin.h"
#include "UI/InventoryGridWidget.h"
#include "InventoryPlugin.h"
#include "UI/Currency/DynamicPurseWidget.h"
#include "InventoryPlugin.h"

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get the player controller interface
	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!PlayerInterface)
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("BankWidget::NativeConstruct - Failed to get InventoryPlayerInterface"));
		return;
	}

	// Initialize InventoryGrid with BankPool
	if (InventoryGrid)
	{
		// Use the owning player pawn as the actor owner for the bank grid
		AActor* OwnerActor = GetOwningPlayerPawn();
		if (!OwnerActor)
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("BankWidget::NativeConstruct - No owning player pawn, using controller"));
			OwnerActor = GetOwningPlayer();
		}

		// Initialize with BankPool slot
		InventoryGrid->InitData(OwnerActor, EBagSlot::BankPool);
		UE_LOG(LogInventoryPlugin, Verbose, TEXT("BankWidget::NativeConstruct - Initialized InventoryGrid with BankPool"));
	}
	else
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("BankWidget::NativeConstruct - InventoryGrid is null (check BindWidget)"));
	}

	// Initialize DynamicPurse with bank coin component
	if (DynamicPurse)
	{
		UCoinComponent* BankCoin = PlayerInterface->GetBankCoin();
		if (BankCoin)
		{
			DynamicPurse->InitWidget(BankCoin);
			UE_LOG(LogInventoryPlugin, Verbose, TEXT("BankWidget::NativeConstruct - Initialized DynamicPurse with BankCoin"));
		}
		else
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("BankWidget::NativeConstruct - GetBankCoin returned null"));
		}
	}
	else
	{
		UE_LOG(LogInventoryPlugin, Error, TEXT("BankWidget::NativeConstruct - DynamicPurse is null (check BindWidget)"));
	}

	// Bind button click events (optional buttons)
	if (ReorganiseButton)
	{
		ReorganiseButton->OnClicked.AddDynamic(this, &UBankWidget::OnReorganiseButtonClicked);
	}

	if (DoneButton)
	{
		DoneButton->OnClicked.AddDynamic(this, &UBankWidget::OnDoneButtonClicked);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::NativeDestruct()
{
	// Clear any active timer
	if (ReorganiseTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReorganiseTimerHandle);
	}

	// Clean up inventory grid
	if (InventoryGrid)
	{
		InventoryGrid->DeInitData();
	}

	// Unbind button click events
	if (ReorganiseButton)
	{
		ReorganiseButton->OnClicked.RemoveAll(this);
	}

	if (DoneButton)
	{
		DoneButton->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::OnDoneButtonClicked()
{
	OnDoneClicked.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::ReorganizeContent()
{
	if (const IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		if (UBankComponent* BankComp = Player->GetBankComponent())
		{
			BankComp->Reorganize();
		}
		else
		{
			UE_LOG(LogInventoryPlugin, Warning, TEXT("BankWidget::ReorganizeContent - Failed to get BankComponent"));
		}
	}
	else
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("BankWidget::ReorganizeContent - Failed to get InventoryPlayerInterface"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::OnReorganiseButtonClicked()
{
	// Disable the button immediately to prevent spam clicking
	if (ReorganiseButton)
	{
		ReorganiseButton->SetIsEnabled(false);
	}

	// Perform the reorganization
	ReorganizeContent();

	// Start timer to re-enable button after delay
	if (ReorganiseDelay > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ReorganiseTimerHandle,
			this,
			&UBankWidget::EnableReorganiseButton,
			ReorganiseDelay,
			false
		);
	}
	else
	{
		// If delay is 0 or negative, re-enable immediately
		EnableReorganiseButton();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::EnableReorganiseButton()
{
	if (ReorganiseButton)
	{
		ReorganiseButton->SetIsEnabled(true);
	}
}

