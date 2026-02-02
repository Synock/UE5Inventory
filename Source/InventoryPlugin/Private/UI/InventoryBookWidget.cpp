
#include "UI/InventoryBookWidget.h"

void UInventoryBookWidget::CloseButtonCalled()
{
	OnCloseEvent.Broadcast();
}

void UInventoryBookWidget::SetText(const FText& TextToDisplay)
{
	TextBlock->SetText(TextToDisplay);
}
