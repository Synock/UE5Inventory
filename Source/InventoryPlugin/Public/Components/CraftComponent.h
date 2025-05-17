// Copyright 2025 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "Components/ActorComponent.h"
#include "CraftComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftInputPoolChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftOutputPoolChangedDelegate);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInputCraftItemAdd, int32, ItemID, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputItemRemove, int32, TopLeftIndex);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UCraftComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_InputCraftPool, BlueprintReadWrite, Category = "Inventory|Craft")
	TArray<FMinimalItemStorage> InputCraftPool;

	UPROPERTY(ReplicatedUsing = OnRep_OutputCraftPool, BlueprintReadWrite, Category = "Inventory|Craft")
	TArray<FMinimalItemStorage> OutputCraftPool;

	
public:
	// Sets default values for this component's properties
	UCraftComponent();

	UPROPERTY(BlueprintAssignable)
	FOnCraftInputPoolChangedDelegate InputCraftPoolDispatcher;

	UPROPERTY(BlueprintAssignable)
	FOnCraftOutputPoolChangedDelegate OutputCraftPoolDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FInputCraftItemAdd ItemAddDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FInputItemRemove ItemRemoveDispatcher;

	UFUNCTION()
	void OnRep_InputCraftPool();

	UFUNCTION()
	void OnRep_OutputCraftPool();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|InputPool")
	void AddItem(int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void RemoveItem(int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	int32 GetItemAtIndex(int32 ID) const;
};
