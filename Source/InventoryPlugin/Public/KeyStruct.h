// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include "KeyStruct.generated.h"

USTRUCT(BlueprintType)
struct FKeyStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|KeyData")
	int32 ItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|KeyData")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|KeyData")
	FString IconName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|KeyData")
	int32 KeyID = 0;
};
