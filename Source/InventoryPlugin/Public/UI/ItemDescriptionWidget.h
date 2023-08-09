// Copyright 2022 Maximilien (Synock) Guislain

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
	
public:
	
};
