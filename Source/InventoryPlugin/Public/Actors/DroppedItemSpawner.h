// Copyright 2025 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroppedItemSpawner.generated.h"

class UBoxComponent;
class ADroppedItem;
class UInventoryItemBase;

UCLASS()
class INVENTORYPLUGIN_API ADroppedItemSpawner : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InventoryPlugin|DroppedItem")
	TObjectPtr<UBoxComponent> SpawnArea;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InventoryPlugin|DroppedItem")
	UInventoryItemBase* DroppedItem;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InventoryPlugin|DroppedItem")
	float RespawnTime = 300.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InventoryPlugin|DroppedItem")
	bool RandomAreaSpawn = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticItemPreview = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryPlugin|DroppedItem")
	FTimerHandle RespawnTimer;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryPlugin|DroppedItem")
	ADroppedItem* SpawnedItem = nullptr;

	UFUNCTION(BlueprintCallable)
	FTransform GetSpawnPosition();

	UFUNCTION(BlueprintCallable)
	void StartRespawnTimer();

	UFUNCTION(BlueprintCallable)
	void SpawnDroppedItem();

	UFUNCTION()
	void RegisterItemPickup(AActor* DestroyedActor);

	UFUNCTION(BlueprintCallable)
	void UpdateStaticMesh();

public:
	ADroppedItemSpawner();
};
