// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RandomizedEquipmentPool.generated.h"

class UInventoryItemBase;

/// Item and probability
USTRUCT(BlueprintType)
struct FItemProbability: public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInventoryItemBase* Item = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Probability = 1.f;

};

/// A list of possible equipment items, with only one that can be obtained at a time
USTRUCT(BlueprintType)
struct FItemAlternative: public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FItemProbability> AlternativeList;

	// Global probability of this alternative to exist.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ExistenceProbability = 1.f;

	UInventoryItemBase* GetItem() const
	{
		float TotalProbability = 0.f;

		for (auto& EquipmentData : AlternativeList)
		{
			TotalProbability += EquipmentData.Probability;
		}

		if (TotalProbability > 0.f)
		{
			float ProbaStatus = FMath::RandRange(0.f, 1.f);
			float SumProbability = 0.f;
			for (const auto& [Item, Probability] : AlternativeList)
			{
				const float AlternativeProbability = SumProbability + (Probability / TotalProbability);
				if (ProbaStatus <= AlternativeProbability)
				{
					return Item;
				}

				SumProbability += AlternativeProbability;
			}
		}

		return nullptr;
	}

};

UCLASS()
class INVENTORYPLUGIN_API URandomizedLootPool : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinCoin = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxCoin = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ProbabilityCoin = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FItemProbability> LootPool;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FItemAlternative> AlternativeSet;

	TArray<int32> GetRandomisedItems() const;

	float GetRandomisedCoin() const;

};

