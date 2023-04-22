// Copyright 2022 Maximilien (Synock) Guislain


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
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::AddItemToStagingArea(int32 ItemID)
{
	StagingAreaItems.Add(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

void UStagingAreaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStagingAreaComponent, StagingAreaItems);
}
