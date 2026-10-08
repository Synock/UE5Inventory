#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/LootableInterface.h"
#include "LootSessionTestTypes.generated.h"

class UCoinComponent;
class ULootPoolComponent;

/** Minimal module-private lootable used by deterministic session tests. */
UCLASS(NotBlueprintable, Transient)
class ALootSessionTestActor final : public AActor, public ILootableInterface
{
	GENERATED_BODY()

public:
	ALootSessionTestActor();

	virtual UCoinComponent* GetCoinComponent() override { return nullptr; }
	virtual ULootPoolComponent* GetLootPoolComponent() override { return LootPool; }
	virtual UCoinComponent* GetCoinComponentConst() const override { return nullptr; }
	virtual ULootPoolComponent* GetLootPoolComponentConst() const override { return LootPool; }
	virtual bool GetIsBeingLooted() const override { return bBeingLooted; }
	virtual void SetIsBeingLooted(bool LootStatus) override { bBeingLooted = LootStatus; }
	virtual bool GetIsDestroyable() const override { return false; }
	virtual void DestroyLootActor() override {}
	virtual FString GetLootActorName() const override { return TEXT("Test Loot"); }
	virtual void StartLooting(AActor* Looter) override;
	virtual void StopLooting(AActor* Looter) override;
	virtual int32 GetItemData(int32 TopLeftID) const override;
	virtual void RemoveItem(int32 TopLeftID) override;

	int32 StartCalls = 0;
	int32 StopCalls = 0;
	int32 TestItemID = 100;
	int32 TestItemTopLeft = 4;

private:
	UPROPERTY()
	TObjectPtr<ULootPoolComponent> LootPool;

	bool bBeingLooted = false;
};

UCLASS(NotBlueprintable, Transient)
class ULootPoolTestListener final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleLootPoolChanged() { ++NotificationCount; }

	int32 NotificationCount = 0;
};
