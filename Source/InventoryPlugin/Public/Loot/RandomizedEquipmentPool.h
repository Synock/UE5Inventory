// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RandomizedEquipmentPool.generated.h"

class UInventoryItemEquipable;

USTRUCT(BlueprintType)
struct FItemProbability
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInventoryItemEquipable* Item = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Probability = 1.f;

};
UCLASS()
class INVENTORYPLUGIN_API URandomizedEquipmentPool : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FItemProbability> EquipmentSet;
};
