// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeyringComponent.generated.h"


class UInventoryItemKey;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeyRingChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKeyItemAdd, int32, KeyID, int32, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKeyItemRemove, int32, KeyID);

USTRUCT(BlueprintType)
struct FKeyItemPair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 KeyId = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ItemId = 0;

};


inline bool operator==(const FKeyItemPair& LHS, const FKeyItemPair& RHS) {return LHS.KeyId == RHS.KeyId && LHS.ItemId == RHS.ItemId; }


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UKeyringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeyringComponent();

protected:

	virtual void BeginPlay() override;

	// This is the main stuff to record which key are available
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_KeyringChanged)
	TArray<FKeyItemPair> KeyringData;

	// This is the main stuff to record which key are available
	TSet<int32> Keyring;

	/// This map link the KeyID to its origin item ID
	TMap<int32, int32> KeyRingToItemLUT;

public:

	bool HasKey(int32 KeyId) const;

	bool TryAddKeyFromItem(const UInventoryItemKey* Item);

	void AddKey(int32 KeyId, int32 ItemId);

	void RemoveKey(int32 KeyId);

	int32 GetItemFromKey(int32 KeyId) const;

	const TArray<FKeyItemPair>& GetAllPossessedKeys() const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Keyring")
	FOnKeyRingChanged KeyringChangedDelegate;

	FKeyItemAdd KeyAdded;

	FKeyItemRemove KeyRemoved;

	UFUNCTION()
	void OnRep_KeyringChanged();

};
