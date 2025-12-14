// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/BankWidget.h"

#include "Components/BankComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "UI/InventoryGridWidget.h"
#include "UI/Currency/DynamicPurseWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get the player controller interface
	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!PlayerInterface)
	{
		UE_LOG(LogTemp, Error, TEXT("BankWidget::NativeConstruct - Failed to get InventoryPlayerInterface"));
		return;
	}

	// Initialize InventoryGrid with BankPool
	if (InventoryGrid)
	{
		// Use the owning player pawn as the actor owner for the bank grid
		AActor* OwnerActor = GetOwningPlayerPawn();
		if (!OwnerActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("BankWidget::NativeConstruct - No owning player pawn, using controller"));
			OwnerActor = GetOwningPlayer();
		}

		// Initialize with BankPool slot
		InventoryGrid->InitData(OwnerActor, EBagSlot::BankPool);
		UE_LOG(LogTemp, Log, TEXT("BankWidget::NativeConstruct - Initialized InventoryGrid with BankPool"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BankWidget::NativeConstruct - InventoryGrid is null (check BindWidget)"));
	}

	// Initialize DynamicPurse with bank coin component
	if (DynamicPurse)
	{
		UCoinComponent* BankCoin = PlayerInterface->GetBankCoin();
		if (BankCoin)
		{
			DynamicPurse->InitWidget(BankCoin);
			UE_LOG(LogTemp, Log, TEXT("BankWidget::NativeConstruct - Initialized DynamicPurse with BankCoin"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BankWidget::NativeConstruct - GetBankCoin returned null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BankWidget::NativeConstruct - DynamicPurse is null (check BindWidget)"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UBankWidget::NativeDestruct()
{
	// Clean up inventory grid
	if (InventoryGrid)
	{
		InventoryGrid->DeInitData();
	}

	Super::NativeDestruct();
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
			UE_LOG(LogTemp, Warning, TEXT("BankWidget::ReorganizeContent - Failed to get BankComponent"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BankWidget::ReorganizeContent - Failed to get InventoryPlayerInterface"));
	}
}
