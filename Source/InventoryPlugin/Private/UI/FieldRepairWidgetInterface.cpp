#include "UI/FieldRepairWidgetInterface.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Items/InventoryItemEquipable.h"

void IFieldRepairWidgetInterface::UpdateUI_Implementation()
{
	auto SelfReference = Cast<UObject>(this);
	Execute_UpdateItemDurabilityBar(SelfReference);
	Execute_UpdateRepairKitChargesBar(SelfReference);
	Execute_UpdateRepairProgressBar(SelfReference);
	Execute_UpdateRepairKitBackground(SelfReference);
}

void IFieldRepairWidgetInterface::UpdateItemDurabilityBar_Implementation()
{
	UProgressBar* ItemDurabilityBar = GetItemDurabilityBar();
	if (!ItemDurabilityBar)
		return;

	const UInventoryItemEquipable* TargetItem = GetCurrentTargetItem();
	float CurrentDurability = GetCurrentTargetDurability();
	float MaxDurability = GetMaxTargetDurability();

	if (TargetItem && MaxDurability > 0.0f)
	{
		float Percent = CurrentDurability / MaxDurability;
		ItemDurabilityBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		ItemDurabilityBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemDurabilityBar->SetPercent(0.0f);
		ItemDurabilityBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void IFieldRepairWidgetInterface::UpdateRepairKitChargesBar_Implementation()
{
	UProgressBar* RepairKitChargesBar = GetRepairKitChargesBar();
	if (!RepairKitChargesBar)
		return;

	float CurrentDurability = GetCurrentRepairKitDurability();
	float MaxDurability = GetMaxRepairKitDurability();

	if (MaxDurability > 0.0f)
	{
		float Percent = CurrentDurability / MaxDurability;
		RepairKitChargesBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		RepairKitChargesBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RepairKitChargesBar->SetPercent(0.0f);
		RepairKitChargesBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void IFieldRepairWidgetInterface::UpdateRepairProgressBar_Implementation()
{
	UProgressBar* RepairProgressBar = GetRepairProgressBar();
	if (!RepairProgressBar)
		return;

	bool bIsRepairing = GetIsRepairing();
	float Progress = GetRepairProgress();

	if (bIsRepairing)
	{
		RepairProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
		RepairProgressBar->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RepairProgressBar->SetPercent(0.0f);
		RepairProgressBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void IFieldRepairWidgetInterface::UpdateStatusText_Implementation(const FText& Message)
{
	UTextBlock* StatusTextWidget = GetStatusText();
	if (StatusTextWidget)
	{
		StatusTextWidget->SetText(Message);
	}
}

void IFieldRepairWidgetInterface::UpdateRepairKitBackground_Implementation()
{
	UImage* RepairKitBackground = GetRepairKitBackground();
	if (!RepairKitBackground)
		return;

	UTexture2D* Icon = GetRepairKitIcon();
	if (Icon)
	{
		RepairKitBackground->SetBrushFromTexture(Icon);
		RepairKitBackground->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RepairKitBackground->SetVisibility(ESlateVisibility::Hidden);
	}
}

void IFieldRepairWidgetInterface::TickRepair_Implementation()
{
	if (!GetIsRepairing())
		return;

	// Update elapsed time
	float ElapsedTime = GetElapsedRepairTime();
	ElapsedTime += 0.1f;
	SetElapsedRepairTime(ElapsedTime);

	// Calculate progress
	float TotalDuration = GetTotalRepairDuration();
	float Progress = TotalDuration > 0.0f ? (ElapsedTime / TotalDuration) : 1.0f;
	SetRepairProgress(Progress);
}

void IFieldRepairWidgetInterface::ValidateState_Implementation()
{
	FText Reason;
	bool bCanRepair = Execute_CanStartRepair(Cast<UObject>(this), Reason);
	bool bIsRepairing = GetIsRepairing();

	// Update status text if not currently repairing
	if (!bIsRepairing)
	{
		if (bCanRepair)
		{
			Execute_UpdateStatusText(Cast<UObject>(this), FText::FromString(TEXT("Ready to repair")));
		}
		else
		{
			Execute_UpdateStatusText(Cast<UObject>(this), Reason);
		}
	}
}
