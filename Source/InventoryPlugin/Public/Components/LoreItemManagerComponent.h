// Copyright 2024 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LoreItemManagerComponent.generated.h"


class UInventoryItemBase;
class ULootPoolComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API ULoreItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	TSet<int32> KnownLoreItems;

	TMap<int32, TObjectPtr<ULootPoolComponent>> DelayedLootPools;

public:
	ULoreItemManagerComponent();

	virtual bool CanSpawnItem(const UInventoryItemBase* InventoryItem) const;

	virtual bool DelayedSpawnItem(const UInventoryItemBase* InventoryItem, ULootPoolComponent* Requester);

};
