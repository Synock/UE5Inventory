#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryEscrow.h"
#include "StagingAreaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStagingAreaChangedDelegate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UStagingAreaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStagingAreaComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_StagingAreaItems, BlueprintReadWrite, Category = "Inventory|Staging")
	TArray<FInventoryEscrowItem> StagingAreaItems;

public:
	UPROPERTY(BlueprintAssignable, Category="Inventory|Staging")
	FOnStagingAreaChangedDelegate StagingAreaDispatcher;

	UFUNCTION()
	void OnRep_StagingAreaItems();

	UFUNCTION(BlueprintCallable)
	const TArray<FInventoryEscrowItem>& GetStagingAreaItems() const { return StagingAreaItems; }

	static constexpr int32 MaxStagedItems = 8;
	bool HasCapacity() const { return StagingAreaItems.Num() < MaxStagedItems; }
	float GetEscrowWeight() const;

	UFUNCTION(BlueprintCallable)
	void ClearStagingArea();

	UFUNCTION(BlueprintCallable)
	bool AddItemToStagingArea(const FInventoryEscrowItem& ItemStorage);

	/** Replaces the staging contents after a partial, lossless rollback. */
	void SetStagingAreaItems(const TArray<FInventoryEscrowItem>& Items);
};
