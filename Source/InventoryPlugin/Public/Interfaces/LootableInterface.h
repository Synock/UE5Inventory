// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "CoinValue.h"

#include "UObject/Interface.h"
#include "LootableInterface.generated.h"

class UCoinComponent;
class ULootPoolComponent;
class APlayerCharacter;
class FOnLootPoolChangedDelegate;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class ULootableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API ILootableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual UCoinComponent* GetCoinComponent() = 0;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual ULootPoolComponent* GetLootPoolComponent() = 0;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual UCoinComponent* GetCoinComponentConst() const = 0;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual ULootPoolComponent* GetLootPoolComponentConst() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool GetIsBeingLooted() const = 0;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void SetIsBeingLooted(bool LootStatus) = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool GetIsDestroyable() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void DestroyLootActor() = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual FString GetName() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void InitLootPool(const TArray<int32>& LootableItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void InitCashPool(const FCoinValue& CoinValue);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StartLooting(AActor* Looter);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StopLooting(AActor* Looter);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual int32 GetItemData(int32 TopLeftID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void RemoveItem(int32 TopLeftID);

	virtual FOnLootPoolChangedDelegate& GetLootPoolDelegate();
};
