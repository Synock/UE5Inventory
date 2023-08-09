// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "ItemBaseWidget.generated.h"

class UInventoryItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UItemBaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected :
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	AActor* Owner = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item")
	const UInventoryItemBase* Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI|Click")
	float RightClickMaxDuration = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI|Click")
	bool IsRightClicking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI|Click")
	FPointerEvent ClickEvent;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI|Click")
	FTimerHandle RightClickTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	float TileSize = 40.f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	UImage* ItemImagePointer = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	UBorder* BackgroundBorderPointer = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	USizeBox* BackgroundSizeBoxPointer = nullptr;

	UFUNCTION()
	void RightClickTimerFunction();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic)
	bool RightClickLongEffect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic)
	bool RightClickShortEffect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic)
	bool LeftClickEffect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintCosmetic)
	void SetupUI();

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable)
	void DisplayDescription(const FPointerEvent& InMouseEvent);

	UFUNCTION(BlueprintCallable)
	void DisplayBookText(const FPointerEvent& InMouseEvent);

	UFUNCTION(BlueprintCallable)
	void ResetSell();

	UFUNCTION(BlueprintCallable)
	void UpdateItemImage();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void InitBareData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	virtual void StopDrag();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	const UInventoryItemBase* GetReferencedItem() const { return Item; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();
};
