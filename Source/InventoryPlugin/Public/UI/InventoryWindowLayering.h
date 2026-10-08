#pragma once

#include "CoreMinimal.h"

class UUserWidget;
class UWidget;

namespace InventoryWindowLayering
{
	/** Adds ChildWindow one layer above the draggable window that owns SourceWidget. */
	INVENTORYPLUGIN_API void AddToViewportAboveSource(UUserWidget* ChildWindow, const UWidget* SourceWidget,
	                                                 int32 FallbackZOrder);
}
