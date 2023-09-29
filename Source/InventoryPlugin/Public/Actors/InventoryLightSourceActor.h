// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
#include "InventoryLightSourceActor.generated.h"

UCLASS()
class INVENTORYPLUGIN_API AInventoryLightSourceActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInventoryLightSourceActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UStaticMeshComponent* StaticItem = nullptr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UPointLightComponent* WearableLightComponent = nullptr;
};
