#pragma once

#include "Layout/Geometry.h"

namespace InventoryGridGeometry
{
	inline const FGeometry* ResolveGridGeometry(const FGeometry* CanvasGeometry, const FGeometry* BorderGeometry)
	{
		return CanvasGeometry ? CanvasGeometry : BorderGeometry;
	}

	inline FVector2D AbsoluteToGridLocal(const FGeometry& WidgetGeometry, const FGeometry* GridGeometry,
		const FVector2D& AbsolutePosition, const FVector2D& ContentInset = FVector2D::ZeroVector)
	{
		const FVector2D LocalPosition = GridGeometry
			? GridGeometry->AbsoluteToLocal(AbsolutePosition)
			: WidgetGeometry.AbsoluteToLocal(AbsolutePosition);
		return LocalPosition - ContentInset;
	}

	inline FVector2D GridOriginInWidgetLocal(const FGeometry& WidgetGeometry, const FGeometry* GridGeometry,
		const FVector2D& ContentInset = FVector2D::ZeroVector)
	{
		const FVector2D ChildOrigin = GridGeometry
			? WidgetGeometry.AbsoluteToLocal(GridGeometry->LocalToAbsolute(FVector2D::ZeroVector))
			: FVector2D::ZeroVector;
		return ChildOrigin + ContentInset;
	}
}
