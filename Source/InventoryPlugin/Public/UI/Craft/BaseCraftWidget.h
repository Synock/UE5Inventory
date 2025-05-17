// Copyright 2025 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseCraftWidget.generated.h"

class UInventoryGridWidget;
class UProgressBar;
class UButton;
/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UBaseCraftWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite);
	UButton* CratButtonPointer = nullptr;

	UPROPERTY(BlueprintReadWrite);
	UButton* CancelButtonPointer = nullptr;

	UPROPERTY(BlueprintReadWrite);
	UProgressBar* ProgressBarPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UInventoryGridWidget* InputGridPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UInventoryGridWidget* OutputGridPointer = nullptr;


	UFUNCTION(BLueprintCallable)
	void SetupUI(AActor* Owner, UButton* CratButton, UButton* CancelButton, UProgressBar* ProgressBar,UInventoryGridWidget* InputGrid, UInventoryGridWidget* OutputGrid);

public:
	UFUNCTION(BlueprintCallable)
	void EnableCraftButton();

	UFUNCTION(BlueprintCallable)
	void DisableCraftButton();
};
