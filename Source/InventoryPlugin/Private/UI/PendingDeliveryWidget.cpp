#include "UI/PendingDeliveryWidget.h"

#include "InventoryUtilities.h"
#include "Components/Button.h"
#include "Components/InventoryDeliveryComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/TextBlock.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/PendingDeliverySlotWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UPendingDeliveryWidget::RebuildWidget()
{
	if (GetClass() != StaticClass())
		return Super::RebuildWidget();

	return SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Pending Deliveries")))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text_Lambda([this]() { return GetNativeItemText(); })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				SNew(STextBlock).Text_Lambda([this]() { return GetNativeCountText(); })
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton).Text(FText::FromString(TEXT("Claim")))
					.OnClicked_UObject(this, &UPendingDeliveryWidget::HandleNativeClaim)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton).Text(FText::FromString(TEXT("Claim All That Fit")))
					.OnClicked_UObject(this, &UPendingDeliveryWidget::HandleNativeClaimAll)
				]
			]
		];
}

void UPendingDeliveryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		DeliveryComponent = Player->GetInventoryDeliveryComponent();
		if (DeliveryComponent)
			DeliveryComponent->DeliveriesChanged.AddUniqueDynamic(this, &UPendingDeliveryWidget::Refresh);
	}
	if (ClaimButton) ClaimButton->OnClicked.AddUniqueDynamic(this, &UPendingDeliveryWidget::ClaimFirst);
	if (ClaimAllButton) ClaimAllButton->OnClicked.AddUniqueDynamic(this, &UPendingDeliveryWidget::ClaimAll);
	Refresh();
}

void UPendingDeliveryWidget::NativeDestruct()
{
	if (DeliveryComponent)
		DeliveryComponent->DeliveriesChanged.RemoveDynamic(this, &UPendingDeliveryWidget::Refresh);
	DeliveryComponent = nullptr;
	Super::NativeDestruct();
}

void UPendingDeliveryWidget::Refresh()
{
	if (!DeliveryComponent)
	{
		if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		{
			DeliveryComponent = Player->GetInventoryDeliveryComponent();
			if (DeliveryComponent)
				DeliveryComponent->DeliveriesChanged.AddUniqueDynamic(this, &UPendingDeliveryWidget::Refresh);
		}
	}

	const int32 Count = DeliveryComponent ? DeliveryComponent->GetPendingDeliveryCount() : 0;
	if (Count <= 0)
	{
		if (PendingItemSlot) PendingItemSlot->ClearDelivery();
		if (ItemNameText) ItemNameText->SetText(FText::GetEmpty());
		if (QueueCountText) QueueCountText->SetText(FText::GetEmpty());
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (QueueCountText) QueueCountText->SetText(FText::AsNumber(Count));
	if (ItemNameText)
	{
		const FPendingInventoryDelivery& Delivery = DeliveryComponent->GetPendingDeliveries()[0];
		const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld());
		ItemNameText->SetText(Item ? FText::FromString(Item->Name) : FText::FromString(TEXT("Unknown item")));
	}
	if (PendingItemSlot)
		PendingItemSlot->InitializeDelivery(DeliveryComponent->GetPendingDeliveries()[0], GetOwningPlayer());
	SetVisibility(ESlateVisibility::Visible);
}

void UPendingDeliveryWidget::ClaimFirst()
{
	if (!DeliveryComponent || DeliveryComponent->GetPendingDeliveries().IsEmpty()) return;
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		if (UInventoryNetComponent* Net = Player->GetInventoryNetComponent())
			Net->Server_ClaimPendingDelivery(DeliveryComponent->GetPendingDeliveries()[0].DeliveryId);
}

void UPendingDeliveryWidget::ClaimAll()
{
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		if (UInventoryNetComponent* Net = Player->GetInventoryNetComponent())
			Net->Server_ClaimAllPendingDeliveries();
}

FText UPendingDeliveryWidget::GetNativeItemText() const
{
	if (!DeliveryComponent || DeliveryComponent->GetPendingDeliveries().IsEmpty())
		return FText::GetEmpty();
	const FPendingInventoryDelivery& Delivery = DeliveryComponent->GetPendingDeliveries()[0];
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld());
	return Item ? FText::FromString(Item->Name) : FText::FromString(TEXT("Unknown item"));
}

FText UPendingDeliveryWidget::GetNativeCountText() const
{
	return FText::Format(FText::FromString(TEXT("{0} waiting")),
		FText::AsNumber(DeliveryComponent ? DeliveryComponent->GetPendingDeliveryCount() : 0));
}

FReply UPendingDeliveryWidget::HandleNativeClaim()
{
	ClaimFirst();
	return FReply::Handled();
}

FReply UPendingDeliveryWidget::HandleNativeClaimAll()
{
	ClaimAll();
	return FReply::Handled();
}
