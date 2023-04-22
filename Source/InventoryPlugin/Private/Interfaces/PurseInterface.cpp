// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/PurseInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void IPurseInterface::PayCoinAmount(const FCoinValue& Amount)
{
	GetPurseComponent()->PayAndAdjust(Amount);
}

//----------------------------------------------------------------------------------------------------------------------

void IPurseInterface::ReceiveCoinAmount(const FCoinValue& Amount)
{
	GetPurseComponent()->AddCoins(Amount);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue IPurseInterface::GetCoinAmount() const
{
	return GetPurseComponentConst()->GetPurseContent();
}

//----------------------------------------------------------------------------------------------------------------------

void IPurseInterface::TransferCoinsTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	const FCoinValue & RemovedValue, const FCoinValue & AddedValue)
{
	GivingComponent->RemoveCoins(RemovedValue);
	ReceivingComponent->AddCoins(AddedValue);
}
