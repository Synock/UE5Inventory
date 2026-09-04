#pragma once

#include "Layout/Geometry.h"

namespace InventoryGridGeometry
{
	inline const FGeometry* ResolveGridGeometry(const FGeometry* CanvasGeometry, const FGeometry* BorderGeometry)
	{
		return CanvasGeometry ? CanvasGeometry : BorderGeometry;
	}

	inline FVector2D AbsoluteToGridLocal(const FGeometry& WidgetGeometry, const FGeometry* GridGeometry,
		const FVector2D& AbsolutePosition)
	{
		return GridGeometry
			? GridGeometry->AbsoluteToLocal(AbsolutePosition)
			: WidgetGeometry.AbsoluteToLocal(AbsolutePosition);
	}

	/** Both geometries must come from the same paint-space layout pass. */
	inline FVector2D GridOriginInWidgetLocal(const FGeometry& WidgetPaintGeometry,
		const FGeometry* GridPaintGeometry)
	{
		return GridPaintGeometry
			? WidgetPaintGeometry.AbsoluteToLocal(GridPaintGeometry->LocalToAbsolute(FVector2D::ZeroVector))
			: FVector2D::ZeroVector;
	}
}
