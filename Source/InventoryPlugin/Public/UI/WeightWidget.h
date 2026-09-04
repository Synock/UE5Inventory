#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WeightWidget.generated.h"

class IInventoryPlayerInterface;
class APlayerCharacter;
/**
 * Displays the player's current carry weight.
 * Bind a UTextBlock named "WeightText" in the Blueprint to get automatic display updates.
 */
UCLASS()
class INVENTORYPLUGIN_API UWeightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Weight")
	float TotalWeight = 0.f;

	/** Optional: bind a UTextBlock named "WeightText" in Blueprint for automatic display updates. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Weight|UI")
	UTextBlock* WeightText = nullptr;

	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|Weight")
	void QueryTotalWeight();

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

public:
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void InitWidget();

	/**
	 * Refresh weight display. C++ default queries weight and updates WeightText if bound.
	 * Blueprint may override to add visual feedback (color changes, animations).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
	virtual void Refresh_Implementation();
};
