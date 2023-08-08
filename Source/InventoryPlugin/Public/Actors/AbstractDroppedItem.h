// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbstractDroppedItem.generated.h"

class UCapsuleComponent;

UCLASS()
class INVENTORYPLUGIN_API AAbstractDroppedItem : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticItem = nullptr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

public:
	AAbstractDroppedItem();

};
