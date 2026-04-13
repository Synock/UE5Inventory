
#include "UI/InventoryBookWidget.h"
#include "InventoryPlugin.h"

void UInventoryBookWidget::CloseButtonCalled()
{
	OnCloseEvent.Broadcast();
}

void UInventoryBookWidget::SetText(const FText& TextToDisplay)
{
	if (TextBlock)
		TextBlock->SetText(TextToDisplay);
	else
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UInventoryBookWidget::SetText — TextBlock is null. Ensure a URichTextBlock named 'TextBlock' exists in the Blueprint."));
}
