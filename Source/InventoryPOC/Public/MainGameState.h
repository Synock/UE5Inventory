// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Inventory/BareItem.h"
#include "MainGameState.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API AMainGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int64, FBareItem> ItemMap;

	UPROPERTY(ReplicatedUsing=OnRep_ItemMap, BlueprintReadWrite)
	TArray<FBareItem> CompleteItemList;

	UFUNCTION()
	virtual void OnRep_ItemMap();

public:
	UFUNCTION(BlueprintCallable)
	FBareItem FetchItemFromID(int64 ID);

	UFUNCTION(BlueprintCallable)
	void RegisterItem(const FBareItem& NewItem);
};
