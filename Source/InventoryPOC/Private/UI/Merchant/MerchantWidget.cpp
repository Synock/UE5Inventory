// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Merchant/MerchantWidget.h"

void UMerchantWidget::InitMerchantData(AMerchantActor* InputMerchantActor)
{
	if(InputMerchantActor)
	{
		MerchantActor = InputMerchantActor;
	}
}
