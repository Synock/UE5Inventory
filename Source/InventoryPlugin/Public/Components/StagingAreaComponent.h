// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	TArray<int32> StagingAreaItems;

public:
	UPROPERTY(BlueprintAssignable, Category="Inventory|Staging")
	FOnStagingAreaChangedDelegate StagingAreaDispatcher;

	UFUNCTION()
	void OnRep_StagingAreaItems();

	UFUNCTION(BlueprintCallable)
	const TArray<int32>& GetStagingAreaItems() const { return StagingAreaItems; }

	UFUNCTION(BlueprintCallable)
	void ClearStagingArea();

	UFUNCTION(BlueprintCallable)
	void AddItemToStagingArea(int32 ItemID);
};
