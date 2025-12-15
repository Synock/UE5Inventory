#include "Components/RepairComponent.h"
#include "InventoryUtilities.h"

URepairComponent::URepairComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URepairComponent::BeginPlay()
{
	Super::BeginPlay();
}

FCoinValue URepairComponent::CalculateSingleItemRepairCost(float ItemBaseValue, float CurrentDurability,
                                                            float MaxDurability) const
{
	if (MaxDurability <= 0.0f || CurrentDurability >= MaxDurability)
	{
		// Item is at full durability or invalid
		return FCoinValue{0, 0, 0, 0};
	}

	// Calculate durability lost as percentage
	const float DurabilityLost = FMath::Max(0.0f, MaxDurability - CurrentDurability);
	const float DurabilityPercentLost = DurabilityLost / MaxDurability;

	// Calculate base repair cost
	// RepairCost = (ItemValue * PercentLost) * BaseCostMultiplier * RepairCostMultiplier
	const float RawRepairCost = ItemBaseValue * DurabilityPercentLost * BaseCostMultiplier * RepairCostMultiplier;

	// Convert to coin value and ensure minimum cost of 1 copper
	const int32 TotalCopper = FMath::Max(1, FMath::RoundToInt(RawRepairCost));

	return UInventoryUtilities::CoinValueFromFloat(static_cast<float>(TotalCopper));
}

void URepairComponent::SetRepairCostMultiplier(float NewMultiplier)
{
	RepairCostMultiplier = FMath::Max(0.1f, NewMultiplier);  // Minimum 10% cost
}

