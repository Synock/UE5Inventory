#include "Components/CoinComponent.h"
#include "InventoryPlugin.h"
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
	DOREPLIFETIME_CONDITION(UCoinComponent, PurseContent, COND_OwnerOnly);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::OnRep_PurseData()
{
	PurseDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::EditCoinContent(int32 InputCP, int32 InputSP, int32 InputGP, int32 InputPP)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Client attempted to edit purse directly - blocked"));
		return;
	}

	const int64 NewCP = static_cast<int64>(PurseContent.CopperPieces) + InputCP;
	const int64 NewSP = static_cast<int64>(PurseContent.SilverPieces) + InputSP;
	const int64 NewGP = static_cast<int64>(PurseContent.GoldPieces) + InputGP;
	const int64 NewPP = static_cast<int64>(PurseContent.PlatinumPieces) + InputPP;

	// Clamp to valid range [0, INT32_MAX] to prevent underflow/overflow exploits
	PurseContent.CopperPieces = FMath::Clamp(NewCP, 0LL, static_cast<int64>(INT32_MAX));
	PurseContent.SilverPieces = FMath::Clamp(NewSP, 0LL, static_cast<int64>(INT32_MAX));
	PurseContent.GoldPieces = FMath::Clamp(NewGP, 0LL, static_cast<int64>(INT32_MAX));
	PurseContent.PlatinumPieces = FMath::Clamp(NewPP, 0LL, static_cast<int64>(INT32_MAX));

	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::PayAndAdjust(const FCoinValue& Cost)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!Cost.IsNonNegative())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Rejected negative PayAndAdjust cost on %s"), *GetName());
		return;
	}

	FCoinValue CurrentCoinValue = PurseContent;
	FCoinValue PaidCost = Cost;
	if (!FCoinValue::RetrieveValue(CurrentCoinValue, PaidCost))
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Rejected unaffordable PayAndAdjust cost on %s"), *GetName());
		return;
	}

	PurseContent = {
		CurrentCoinValue.CopperPieces - PaidCost.CopperPieces,
		CurrentCoinValue.SilverPieces - PaidCost.SilverPieces,
		CurrentCoinValue.GoldPieces - PaidCost.GoldPieces,
		CurrentCoinValue.PlatinumPieces - PaidCost.PlatinumPieces
	};

	UE_LOG(LogInventoryPlugin, Verbose, TEXT("PayAndAdjust: %s"), *GetName());
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::PayAndAdjustSimple(const FCoinValue& Cost)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!Cost.IsNonNegative())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Rejected negative PayAndAdjustSimple cost on %s"), *GetName());
		return;
	}

	const int64 NewValue = FMath::Max(PurseContent.ToCopperValue() - Cost.ToCopperValue(), 0LL);
	PurseContent = FCoinValue(static_cast<float>(FMath::Min(NewValue, static_cast<int64>(INT32_MAX))));

	UE_LOG(LogInventoryPlugin, Verbose, TEXT("PayAndAdjustSimple: %s"), *GetName());
	PurseDispatcher_Server.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::RemoveCoins(const FCoinValue& CoinValue)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!CoinValue.IsNonNegative())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Rejected negative RemoveCoins value on %s"), *GetName());
		return;
	}

	EditCoinContent(-CoinValue.CopperPieces, -CoinValue.SilverPieces,
	                -CoinValue.GoldPieces, -CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::AddCoins(const FCoinValue& CoinValue)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!CoinValue.IsNonNegative())
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Rejected negative AddCoins value on %s"), *GetName());
		return;
	}

	EditCoinContent(CoinValue.CopperPieces, CoinValue.SilverPieces,
	                CoinValue.GoldPieces, CoinValue.PlatinumPieces);
}

//----------------------------------------------------------------------------------------------------------------------

void UCoinComponent::LootPurse(UCoinComponent* OtherPurse)
{

	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Client attempted to loot purse - blocked"));
		return;
	}

	// Validate other purse exists and is not self
	if (!OtherPurse || OtherPurse == this)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Invalid purse loot attempt"));
		return;
	}

	AActor* OtherOwner = OtherPurse->GetOwner();
	if (!OtherOwner)
	{
		UE_LOG(LogInventoryPlugin, Warning, TEXT("Other purse has no owner"));
		return;
	}

	// TODO: Add lootability check via ILootableInterface when available
	// For now, we assume the GameMode has already validated loot rights

	AddCoins(OtherPurse->GetPurseContent());
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
