#include "UI/InventoryWindowLayering.h"

#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "UI/InventoryWindowLayerHost.h"

void InventoryWindowLayering::AddToViewportAboveSource(UUserWidget* ChildWindow, const UWidget* SourceWidget,
                                                        int32 FallbackZOrder)
{
	if (!ChildWindow)
	{
		return;
	}

	for (const UObject* Candidate = SourceWidget; Candidate; Candidate = Candidate->GetOuter())
	{
		if (Candidate->GetClass()->ImplementsInterface(UInventoryWindowLayerHost::StaticClass()))
		{
			IInventoryWindowLayerHost::Execute_AttachChildWindow(const_cast<UObject*>(Candidate), ChildWindow);
			return;
		}
	}

	ChildWindow->AddToViewport(FallbackZOrder);
}
