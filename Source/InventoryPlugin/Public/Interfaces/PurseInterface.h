// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "CoinValue.h"
#include "Components/CoinComponent.h"
#include "UObject/Interface.h"
#include "PurseInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPurseInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API IPurseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UCoinComponent* GetPurseComponent() = 0;

	virtual const UCoinComponent* GetPurseComponentConst() const = 0;

	virtual void PayCoinAmount(const FCoinValue& Amount);

	virtual void ReceiveCoinAmount(const FCoinValue& Amount);

	virtual FCoinValue GetCoinAmount() const;

	virtual void TransferCoinsTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
		const FCoinValue & RemovedValue, const FCoinValue & AddedValue);
};
