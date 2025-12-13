#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/InventoryItemBase.h"
#include "ItemDescriptionWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UItemDescriptionWidget : public UUserWidget
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

public:

};
