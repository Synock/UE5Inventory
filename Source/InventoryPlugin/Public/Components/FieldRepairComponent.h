#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "FieldRepairComponent.generated.h"


class UInventoryItemEquipable;
class UEquipmentComponent;
class IInventoryItemFieldRepairInterface;
class UInventoryComponent;

// Delegates for field repair events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnFieldRepairStarted, int32, RepairKitItemID, EBagSlot, RepairKitBagSlot,
                                              int32, RepairKitTopLeft, EEquipmentSlot, TargetEquipmentSlot, EBagSlot, TargetBagSlot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFieldRepairCompleted, float, ActualRepairAmount, float, NewTargetDurability,
                                               float, NewKitDurability);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldRepairCancelled, FText, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldRepairFailed, FText, Reason);

// Field repair active state tracking
USTRUCT(BlueprintType)
struct FActiveFieldRepair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RepairKitItemID = -1;

	UPROPERTY(BlueprintReadOnly)
	EBagSlot RepairKitBagSlot = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadOnly)
	int32 RepairKitTopLeft = -1;

	// Target item location (one or the other will be valid)
	UPROPERTY(BlueprintReadOnly)
	EEquipmentSlot TargetEquipmentSlot = EEquipmentSlot::Unknown; // If repairing equipped item

	UPROPERTY(BlueprintReadOnly)
	EBagSlot TargetBagSlot = EBagSlot::Unknown; // If repairing inventory item

	UPROPERTY(BlueprintReadOnly)
	int32 TargetTopLeft = -1; // If repairing inventory item

	UPROPERTY(BlueprintReadOnly)
	int32 TargetItemID = -1; // Item ID for validation

	UPROPERTY(BlueprintReadOnly)
	double StartTime = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float RepairDuration = 0.0f; // Expected duration for validation

	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;

	// Result data for when repair completes (replicated to client)
	UPROPERTY(BlueprintReadOnly)
	float ResultActualRepairAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ResultNewTargetDurability = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ResultNewKitDurability = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bCompletedSuccessfully = false;

	// Server-side timer for auto-completion and interrupt monitoring (not replicated)
	FTimerHandle ServerTimerHandle;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UFieldRepairComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFieldRepairComponent();

	UFUNCTION(BlueprintCallable)
	bool IsRepairActive() const { return ActiveFieldRepair.bIsActive; }

	UFUNCTION(BlueprintCallable)
	bool CheckIncomingRepairValidity(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
	                                 int32 RepairKitTopLeft,
	                                 EEquipmentSlot TargetEquipmentSlot,
	                                 EBagSlot TargetBagSlot, int32 TargetTopLeft,
	                                 int32 TargetItemID);

	UFUNCTION(BlueprintCallable)
	bool BeginFieldRepair(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
	                      int32 RepairKitTopLeft,
	                      EEquipmentSlot TargetEquipmentSlot,
	                      EBagSlot TargetBagSlot, int32 TargetTopLeft,
	                      int32 TargetItemID);

	UFUNCTION(BlueprintCallable)
	void CancelFieldRepair();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveFieldRepair, BlueprintReadOnly, Category = "FieldRepair")
	FActiveFieldRepair ActiveFieldRepair;

	UFUNCTION()
	void OnRep_ActiveFieldRepair();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FieldRepair")
	float InterruptCheckInterval = 0.5f; // How often to check for interrupts

public:
	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Delegates
	//------------------------------------------------------------------------------------------------------------------

	/** Broadcast when field repair starts on server */
	UPROPERTY(BlueprintAssignable, Category = "FieldRepair")
	FOnFieldRepairStarted OnFieldRepairStarted;

	/** Broadcast when field repair completes successfully */
	UPROPERTY(BlueprintAssignable, Category = "FieldRepair")
	FOnFieldRepairCompleted OnFieldRepairCompleted;

	/** Broadcast when field repair is cancelled or interrupted */
	UPROPERTY(BlueprintAssignable, Category = "FieldRepair")
	FOnFieldRepairCancelled OnFieldRepairCancelled;

	/** Broadcast when field repair fails validation */
	UPROPERTY(BlueprintAssignable, Category = "FieldRepair")
	FOnFieldRepairFailed OnFieldRepairFailed;

protected:

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Server-Side Timer Management
	//------------------------------------------------------------------------------------------------------------------

	/** Server-side timer callback that completes repair automatically */
	void ServerAutoCompleteFieldRepair();

	/** Server-side interrupt monitoring - checks if repair should be canceled */
	void ServerCheckFieldRepairInterrupts();

	/** Internal helper to complete repair with all validations */
	void ServerInternalCompleteFieldRepair();

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Client-Side Event Handlers
	//------------------------------------------------------------------------------------------------------------------

	/** Client-side handler that notifies HUD when repair completes successfully */
	UFUNCTION()
	void HandleRepairCompletedForHUD(float ActualRepairAmount, float NewTargetDurability, float NewKitDurability);

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Shared Validation Helpers
	//------------------------------------------------------------------------------------------------------------------

	/** Retrieves and validates repair kit data. Returns true if valid, populates out parameters */
	bool ValidateRepairKit(int32 RepairKitItemID, EBagSlot RepairKitBagSlot, int32 RepairKitTopLeft,
	                       UInventoryComponent* InventoryComp,
	                       const UInventoryItemBase*& OutRepairKitItem,
	                       const IInventoryItemFieldRepairInterface*& OutRepairInterface,
	                       float& OutRepairKitDurability) const;

	/** Retrieves and validates target item data. Returns true if valid, populates out parameters */
	bool ValidateTargetItem(EEquipmentSlot TargetEquipmentSlot, EBagSlot TargetBagSlot, int32 TargetTopLeft,
	                        int32 TargetItemID, UInventoryComponent* InventoryComp,
	                        UEquipmentComponent* EquipmentComp,
	                        const UInventoryItemEquipable*& OutTargetItem, float& OutCurrentDurability) const;
};
