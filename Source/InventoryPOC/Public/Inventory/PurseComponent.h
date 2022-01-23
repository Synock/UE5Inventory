// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "CoinValue.h"
#include "PurseComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurseChangedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurseChangedDelegate_Server);

///@brief
///Class responsible for holding coins
///There is no inner protection against negative coin amount
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPOC_API UPurseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPurseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_PurseData, BlueprintReadOnly, Category="Inventory|Purse")
	FCoinValue PurseContent;

public:
	UFUNCTION()
	void OnRep_PurseData();

	UPROPERTY(BlueprintAssignable, Category="Inventory|Purse")
	FOnPurseChangedDelegate PurseDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category="Inventory|Purse")
	FOnPurseChangedDelegate_Server PurseDispatcher_Server;

	//Modify the content of the purse according to input
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void EditCoinContent(int32 InputCP, int32 InputSP, int32 InputGP, int32 InputPP);

	//Pay the asked price, tweak the amount of coin to fit what is available
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void PayAndAdjust(const FCoinValue& Cost);

	//Remove coins
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void RemoveCoins(const FCoinValue& CoinValue);

	//Add coins
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void AddCoins(const FCoinValue& CoinValue);

	//Put the content of OtherPurse into this one, an empty OtherPurse
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void LootPurse(UPurseComponent* OtherPurse);

	//Return the purse content
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	const FCoinValue& GetPurseContent() const;

	//Empty the purse
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	void ClearPurse();

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	bool HasContent();

	//Compute the combined weight of all the coins
	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	float GetTotalWeight();

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetCP() const { return PurseContent.CopperPieces; }

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetSP() const { return PurseContent.SilverPieces; }

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetGP() const { return PurseContent.GoldPieces; }

	UFUNCTION(BlueprintCallable, Category="Inventory|Purse")
	int32 GetPP() const { return PurseContent.PlatinumPieces; }
};
