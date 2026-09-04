#pragma once

#include "CoreMinimal.h"

namespace InventoryHUDPositioning
{
	FVector2D ClampWindowPositionToViewport(const FVector2D& DesiredPosition, const FVector2D& DesiredSize,
	                                         int32 ViewportX, int32 ViewportY);
}
