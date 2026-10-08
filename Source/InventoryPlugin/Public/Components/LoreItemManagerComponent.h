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

	/// Items whose global ownership status is being queried from the backend.
	/// IsKnownLoreItem() treats these as blocked until the HTTP response resolves the state.
	TSet<int32> PendingStatusChecks;

	TMap<int32, TWeakObjectPtr<ULootPoolComponent>> DelayedLootPools;

public:
	ULoreItemManagerComponent();

	virtual bool CanSpawnItem(const UInventoryItemBase* InventoryItem) const;

	virtual bool DelayedSpawnItem(const UInventoryItemBase* InventoryItem, ULootPoolComponent* Requester);

};
