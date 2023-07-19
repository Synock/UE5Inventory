// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "DroppedItem.generated.h"

class UInventoryItemBase;

UCLASS()
class INVENTORYPLUGIN_API ADroppedItem : public AActor
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticItem = nullptr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

public:

	ADroppedItem();

	virtual void InitializeFromItem(UInventoryItemBase* Item);

};
