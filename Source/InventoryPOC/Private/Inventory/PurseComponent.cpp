// Copyright 2021 Maximilien (Synock) Guislain


#include "Inventory/PurseComponent.h"
#include <Net/UnrealNetwork.h>

UPurseComponent::UPurseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPurseComponent, PurseContent);
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::OnRep_PurseData()
{
	PurseDispatcher.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("CALL FROM ONREPURSE %s"), *GetName());
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::EditCoinContent(int32 InputCP, int32 InputSP, int32 InputGP, int32 InputPP)
{
	PurseContent += {InputCP, InputSP, InputGP, InputPP};
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::PayAndAdjust(const FCoinValue& Cost)
{
	FCoinValue CurrentCoinValue = PurseContent;
	FCoinValue PaidCost = Cost;
	FCoinValue::RetrieveValue(CurrentCoinValue, PaidCost);

	PurseContent = {
		CurrentCoinValue.CopperPieces - PaidCost.CopperPieces,
		CurrentCoinValue.SilverPieces - PaidCost.SilverPieces,
		CurrentCoinValue.GoldPieces - PaidCost.GoldPieces,
		CurrentCoinValue.PlatinumPieces - PaidCost.PlatinumPieces
	};

	UE_LOG(LogTemp, Log, TEXT("Paying and adjusting %s"), *GetName());
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::RemoveCoins(const FCoinValue& CoinValue)
{
	EditCoinContent(-CoinValue.CopperPieces, -CoinValue.SilverPieces,
	                -CoinValue.GoldPieces, -CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::AddCoins(const FCoinValue& CoinValue)
{
	EditCoinContent(CoinValue.CopperPieces, CoinValue.SilverPieces,
	                CoinValue.GoldPieces, CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::LootPurse(UPurseComponent* OtherPurse)
{
	EditCoinContent(OtherPurse->GetCP(), OtherPurse->GetSP(), OtherPurse->GetGP(), OtherPurse->GetPP());
	OtherPurse->ClearPurse();
}

//----------------------------------------------------------------------------------------------------------------------

const FCoinValue& UPurseComponent::GetPurseContent() const
{
	return PurseContent;
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseComponent::ClearPurse()
{
	PurseContent = {0, 0, 0, 0};
}

//----------------------------------------------------------------------------------------------------------------------

bool UPurseComponent::HasContent()
{
	return PurseContent.CopperPieces != 0 || PurseContent.SilverPieces != 0 || PurseContent.GoldPieces != 0 ||
		PurseContent.PlatinumPieces != 0;
}

//----------------------------------------------------------------------------------------------------------------------

float UPurseComponent::GetTotalWeight()
{
	//for reference: 
	// 50ct volume 3.6191 cm3
	// 10cts volume 2.5
	// 5cts volume 2.4
	// plat 21.45 g/cm3
	// gold 19.3
	// silver 10.49
	// Copper 8.94
	constexpr float cpw = 8.94f * 3.62f;
	constexpr float spw = 10.49f * 3.62f;
	constexpr float gpw = 19.3 * 2.5;
	constexpr float ppw = 21.45 * 2.4;

	return (PurseContent.CopperPieces * cpw + PurseContent.SilverPieces * spw + PurseContent.GoldPieces * gpw +
		PurseContent.PlatinumPieces * ppw) / 1000.;
}
