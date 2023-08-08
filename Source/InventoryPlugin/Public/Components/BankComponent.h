// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "Components/ActorComponent.h"
#include "BankComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankPoolChangedDelegate);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBankItemAdd, int32, ItemID, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBankItemRemove, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankReorganize);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UBankComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_BankPool, BlueprintReadWrite, Category = "Inventory|BankPool")
	TArray<FMinimalItemStorage> Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|BankPool")
	EBagSlot LocalBagSlot = EBagSlot::BankPool;

	uint32 Width = 8;
	uint32 Height = 16;


public:
	UBankComponent();

	UPROPERTY(BlueprintAssignable)
	FOnBankPoolChangedDelegate BankPoolDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FBankItemAdd BankItemAddDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FBankItemRemove BankItemRemoveDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FOnBankReorganize BankReorganizeDispatcher;

	UFUNCTION()
	void OnRep_BankPool();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void AddItem(int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void RemoveItem(int32 TopLeftIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void Reorganize();

	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	int32 GetItemAtIndex(int32 ID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	const TArray<FMinimalItemStorage>& GetBagConst() const { return Items; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	bool IsEmpty() const { return Items.IsEmpty(); }
};
