// Copyright 2022 Maximilien (Synock) Guislain


#include "Actors/MerchantActor.h"

#include "CommonUtilities.h"
#include "Inventory/MerchantComponent.h"
#include "Inventory/PurseComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AMerchantActor::AMerchantActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	if (HasAuthority())
	{
		MerchantComponent = CreateDefaultSubobject<UMerchantComponent>("MerchantComponent");
		MerchantComponent->SetNetAddressable(); // Make DSO components net addressable
		MerchantComponent->SetIsReplicated(true); // Enable replication by default

		CashContent_Merchant = CreateDefaultSubobject<UPurseComponent>("CashContent_Merchant");
		CashContent_Merchant->SetNetAddressable(); // Make DSO components net addressable
		CashContent_Merchant->SetIsReplicated(true); // Enable replication by default
	}
}

//----------------------------------------------------------------------------------------------------------------------


// Called when the game starts or when spawned
void AMerchantActor::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void AMerchantActor::InitMerchantData(const TArray<int64>& StaticItems,
                                      const TArray<FMerchantDynamicItemStorage>& DynamicItems, const FCoinValue& Coins)
{
	if (HasAuthority())
	{
		MerchantComponent->InitStatic(StaticItems);
		MerchantComponent->InitDynamic(DynamicItems);
		CashContent_Merchant->AddCoins(Coins);
	}
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<int64>& AMerchantActor::GetStaticItemsConst() const
{
	return MerchantComponent->GetStaticItemsConst();
}

//----------------------------------------------------------------------------------------------------------------------

void AMerchantActor::RemoveItemAmountIfNeeded(int64 ItemID)
{
	MerchantComponent->RemoveItemID(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

void AMerchantActor::PayCoin(const FCoinValue& CoinValue)
{
	if (HasAuthority())
		CashContent_Merchant->PayAndAdjust(CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void AMerchantActor::ReceiveCoin(const FCoinValue& CoinValue)
{
	if (HasAuthority())
		CashContent_Merchant->AddCoins(CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMerchantDynamicItemStorage>& AMerchantActor::GetDynamicItemsConst() const
{
	return MerchantComponent->GetDynamicItemsConst();
}

//----------------------------------------------------------------------------------------------------------------------

bool AMerchantActor::HasItem(int64 ItemID) const
{
	return MerchantComponent->HasItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

bool AMerchantActor::CanPayAmount(const FCoinValue& CoinValue) const
{
	return FCoinValue::CanPayWithChange(CashContent_Merchant->GetPurseContent(), CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue AMerchantActor::AdjustPriceBuy(const FCoinValue& CoinValue) const
{
	FCoinValue Value = CoinValue;
	Value *= MerchantRatio;
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue AMerchantActor::AdjustPriceSell(const FCoinValue& CoinValue) const
{
	FCoinValue Value = CoinValue;
	Value *= (2.0 - MerchantRatio);
	return Value;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue AMerchantActor::GetItemPriceBuy(int64 ItemID) const
{
	const FCoinValue Price(UCommonUtilities::GetItemFromID(ItemID, GetWorld()).BaseValue);
	return AdjustPriceBuy(Price);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue AMerchantActor::GetItemPriceSell(int64 ItemID) const
{
	const FCoinValue Price(UCommonUtilities::GetItemFromID(ItemID, GetWorld()).BaseValue);
	return AdjustPriceSell(Price);
}

//----------------------------------------------------------------------------------------------------------------------

void AMerchantActor::AddDynamicItem(int64 ItemID)
{
	MerchantComponent->AddItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

FOnMerchantDynamicPoolChangedDelegate& AMerchantActor::GetMerchantDispatcher() const
{
	return MerchantComponent->MerchantPoolDispatcher;
}

void AMerchantActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AMerchantActor, CashContent_Merchant);
	DOREPLIFETIME(AMerchantActor, MerchantComponent);
	DOREPLIFETIME(AMerchantActor, MerchantName);
	DOREPLIFETIME(AMerchantActor, MerchantRatio);
}
