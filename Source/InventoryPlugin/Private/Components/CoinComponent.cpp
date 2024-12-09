// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/CoinComponent.h"
#include <Net/UnrealNetwork.h>

UCoinComponent::UCoinComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCoinComponent, PurseContent);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::OnRep_PurseData()
{
	PurseDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::EditCoinContent(int32 InputCP, int32 InputSP, int32 InputGP, int32 InputPP)
{
	PurseContent += {InputCP, InputSP, InputGP, InputPP};
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::PayAndAdjust(const FCoinValue& Cost)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

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

void UCoinComponent::PayAndAdjustSimple(const FCoinValue& Cost)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	float NewValue = FMath::Max(PurseContent.ToFloat() - Cost.ToFloat(), 0.f);
	PurseContent = FCoinValue(NewValue);

	UE_LOG(LogTemp, Log, TEXT("Paying and adjusting %s"), *GetName());
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::RemoveCoins(const FCoinValue& CoinValue)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	EditCoinContent(-CoinValue.CopperPieces, -CoinValue.SilverPieces,
	                -CoinValue.GoldPieces, -CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::AddCoins(const FCoinValue& CoinValue)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	EditCoinContent(CoinValue.CopperPieces, CoinValue.SilverPieces,
	                CoinValue.GoldPieces, CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::LootPurse(UCoinComponent* OtherPurse)
{
	EditCoinContent(OtherPurse->GetCP(), OtherPurse->GetSP(), OtherPurse->GetGP(), OtherPurse->GetPP());
	OtherPurse->ClearPurse();
}

//----------------------------------------------------------------------------------------------------------------------

const FCoinValue& UCoinComponent::GetPurseContent() const
{
	return PurseContent;
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::ClearPurse()
{
	PurseContent = {0, 0, 0, 0};
}

//----------------------------------------------------------------------------------------------------------------------

bool UCoinComponent::HasContent()
{
	return PurseContent.CopperPieces != 0 || PurseContent.SilverPieces != 0 || PurseContent.GoldPieces != 0 ||
		PurseContent.PlatinumPieces != 0;
}

//----------------------------------------------------------------------------------------------------------------------

float UCoinComponent::GetTotalWeight()
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
