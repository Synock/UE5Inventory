// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "AbstractDroppedItem.h"
#include "CoinValue.h"
#include "GameFramework/Actor.h"
#include "DroppedCoins.generated.h"

UCLASS()
class INVENTORYPLUGIN_API ADroppedCoins : public AAbstractDroppedItem
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FCoinValue InsideValue;

public:
	ADroppedCoins();

	virtual void InitializeFromCoinValue(const FCoinValue& Value);

};
