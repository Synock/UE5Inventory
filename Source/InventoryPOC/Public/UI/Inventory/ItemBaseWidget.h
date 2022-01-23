// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Inventory/BareItem.h"
#include "Blueprint/UserWidget.h"
#include "ItemBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API UItemBaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected :
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	AActor* Owner = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	FBareItem Item;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Click")
	float RightClickMaxDuration = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Click")
	float ClickDuration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Click")
	bool IsRightClicking = false;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	float TileSize = 40.f;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void InitBareData(const FBareItem& InputItem, AActor* InputOwner, float InputTileSize);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	virtual void StopDrag();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	const FBareItem& GetReferencedItem() const { return Item; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();
};
