
#include "Components/StagingAreaComponent.h"

#include "Net/UnrealNetwork.h"

UStagingAreaComponent::UStagingAreaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::OnRep_StagingAreaItems()
{
	StagingAreaDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::ClearStagingArea()
{
	StagingAreaItems.Empty();
	StagingAreaDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

bool UStagingAreaComponent::AddItemToStagingArea(const FInventoryEscrowItem& ItemStorage)
{
	if (!ItemStorage.IsValid() || !HasCapacity())
		return false;
	StagingAreaItems.Add(ItemStorage);
	StagingAreaDispatcher.Broadcast();
	return true;
}

void UStagingAreaComponent::SetStagingAreaItems(const TArray<FInventoryEscrowItem>& Items)
{
	StagingAreaItems = Items;
	StagingAreaDispatcher.Broadcast();
}

float UStagingAreaComponent::GetEscrowWeight() const
{
	float Weight = 0.0f;
	for (const FInventoryEscrowItem& Item : StagingAreaItems)
		Weight += FMath::Max(0.0f, Item.EffectiveWeight);
	return Weight;
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStagingAreaComponent, StagingAreaItems);
}
