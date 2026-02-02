#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "InventoryItemBag.h"
#include "Interfaces/InventoryItemAmmoBagInterface.h"
#include "InventoryItemAmmoBag.generated.h"

UENUM(BlueprintType)
enum class EAmmoBagModel : uint8
{
	Empty,
	Single,
	Mid,
	Full
};

UCLASS()
class INVENTORYPLUGIN_API UInventoryItemAmmoBag : public UInventoryItemBag, public IInventoryItemAmmoBagInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|AmmoBag")
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|AmmoBag")
	UStaticMesh* SingleAmmoMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|AmmoBag")
	UStaticMesh* MidAmmoMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|AmmoBag")
	UStaticMesh* FullAmmoMesh = nullptr;

	virtual EAmmoType GetAmmoType() const override {return AmmoType;}

	virtual UStaticMesh* GetEmptyBagMesh() const override {return Mesh;}
	virtual UStaticMesh* GetLowMesh() const override {return SingleAmmoMesh;}
	virtual UStaticMesh* GetMidMesh() const override {return MidAmmoMesh;}
	virtual UStaticMesh* GetFullMesh() const override {return FullAmmoMesh;}
};
