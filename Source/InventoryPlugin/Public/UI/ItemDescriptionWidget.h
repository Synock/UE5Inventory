#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/InventoryItemBase.h"
#include "UI/InventoryItemDescriptionWidgetInterface.h"
#include "ItemDescriptionWidget.generated.h"

/**
 * Plugin-default item description tooltip widget.
 * Implements IInventoryItemDescriptionWidgetInterface so it works out-of-the-box
 * as the default class returned by IInventoryHUDInterface::GetItemDescriptionWidgetClass().
 * Override in a game-side subclass (e.g. UFinalItemDescriptionWidget) for richer display.
 */
UCLASS()
class INVENTORYPLUGIN_API UItemDescriptionWidget : public UUserWidget, public IInventoryItemDescriptionWidgetInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* Parent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UInventoryItemBase* ObservedItem = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Durability")
	float ItemDurability = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Durability")
	float ItemMaxDurability = 100.0f;

public:

	UFUNCTION(BlueprintCallable)
	bool IsLore() const;

	UFUNCTION(BlueprintCallable)
	bool IsMagic() const;

	UFUNCTION(BlueprintCallable)
	FString GetItemName() const;
	
	virtual FString GetItemDescription()const;

	virtual FString GetGeneralString() const;

	virtual FString GetSlotString() const;
	virtual FString GetRaceString() const;
	virtual FString GetClassString() const;
	virtual FString GetACString() const;
	virtual FString GetAttributesString() const;
	
	virtual FString GetEquipableString() const;

	virtual FString GetDamageString() const;
	virtual FString GetWeaponString() const;

	virtual FString GetContainerString() const;

	virtual FString GetFoodString() const;
	virtual FString GetDrinkString() const;
	virtual FString GetUsableString() const;
	
	virtual FString GetSpellString() const;
	
	UFUNCTION(BlueprintCallable)
	virtual FString GetGlobalString() const;

	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetTextureIcon() const;

	UFUNCTION(BlueprintCallable)
	void SetItemDurability(float InDurability) { ItemDurability = FMath::Clamp(InDurability, 0.0f, ItemMaxDurability); }

	UFUNCTION(BlueprintCallable)
	void SetItemMaxDurability(float InMaxDurability) { ItemMaxDurability = FMath::Max(1.0f, InMaxDurability); }

	UFUNCTION(BlueprintCallable)
	void SetItemDurabilityWithMax(float InDurability, float InMaxDurability)
	{
		ItemMaxDurability = FMath::Max(1.0f, InMaxDurability);
		ItemDurability = FMath::Clamp(InDurability, 0.0f, ItemMaxDurability);
	}

	UFUNCTION(BlueprintCallable)
	float GetItemDurability() const { return ItemDurability; }

	UFUNCTION(BlueprintCallable)
	float GetItemMaxDurability() const { return ItemMaxDurability; }

	UFUNCTION(BlueprintCallable)
	float GetDurabilityPercentage() const
	{
		return ItemMaxDurability > 0.0f ? (ItemDurability / ItemMaxDurability) * 100.0f : 100.0f;
	}

	UFUNCTION(BlueprintCallable)
	FString GetDurabilityConditionString() const;

	UFUNCTION(BlueprintCallable)
	virtual FString GetDurabilityString() const;

	// IInventoryItemDescriptionWidgetInterface
	virtual void InitDescription_Implementation(const UInventoryItemBase* Item) override;
	virtual void InitDescriptionWithDurability_Implementation(const UInventoryItemBase* Item, float Durability, float MaxDurability) override;

	/**
	 * Called after ObservedItem (and durability) have been set on this widget.
	 * Override in Blueprint to refresh all bound text blocks, icons, and stat displays.
	 * The C++ default is a no-op; Blueprint widgets should implement this instead of reading
	 * ObservedItem in Event Construct.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|ItemDescription")
	void OnDescriptionPopulated();
	virtual void OnDescriptionPopulated_Implementation() {}
};
