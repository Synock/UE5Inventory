
#include "Items/InventoryItemBase.h"
#include "Materials/MaterialInstanceDynamic.h"

void FMaterialOverride::ApplyTint(UMaterialInstanceDynamic* Material) const
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Tint"), TintColor);
		Material->SetScalarParameterValue(TEXT("TintIntensity"), TintIntensity);
	}
}

