// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "GameFramework/Character.h"
#include "Inventory/EquipmentComponent.h"
#include "Inventory/FullInventoryComponent.h"
#include "Inventory/PurseComponent.h"
#include "InventoryPOCCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);

UCLASS(config=Game)
class AInventoryPOCCharacter : public ACharacterBase
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AInventoryPOCCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	virtual void PossessedBy(AController* NewController) override;

	/******************
	 * START OF INVENTORY RELATED STUFF
	 ****************/

	FOnFullInventoryComponentChanged& GetInventoryDispatcher();

	FOnEquipmentChanged& GetEquipmentDispatcher();

	FOnPurseChangedDelegate& GetPurseDispatcher();

	//------------------------------------------------------------------------------------------------------------------
	// Weight
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable)
	FOnWeightChanged WeightDispatcher;

	UFUNCTION(BlueprintCallable)
	float GetTotalWeight() const { return TotalWeight; }

	UFUNCTION()
	void OnRep_Weight();

	//------------------------------------------------------------------------------------------------------------------
	// Inventory
	//------------------------------------------------------------------------------------------------------------------

        UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
        TArray<FBareItem> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	const TArray<FMinimalItemStorage>& GetAllItemsInBag(BagSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	bool CanUnequipBag(InventorySlot Slot) const;

	UFUNCTION()
	bool PlayerTryAutoLootFunction(int64 InItemId, InventorySlot& PossibleEquipment, int32& InTopLeft,
	                               BagSlot& PossibleBag) const;

	UFUNCTION()
	void PlayerAddItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId);

	UFUNCTION()
	void PlayerRemoveItem(int32 TopLeft, BagSlot Slot);

	UFUNCTION()
	int64 PlayerGetItem(int32 TopLeft, BagSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	void PlayerMoveItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft, BagSlot OutSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Equipment
	//------------------------------------------------------------------------------------------------------------------

        UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
        TArray<FBareItem> GetAllEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	const FBareItem& GetEquippedItem(InventorySlot Slot) const;

	UFUNCTION()
	void PlayerEquip(InventorySlot InSlot, int64 InItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool PlayerTryAutoEquip(int64 InItemId, InventorySlot& PossibleEquipment) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void PlayerUnequipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, InventorySlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void PlayerEquipItemFromInventory(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft, BagSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void PlayerSwapEquipment(int64 InItemId, InventorySlot InSlot, int64 OutItemId, InventorySlot OutSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Coin
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin")
	void PayCoinAmount(const FCoinValue& Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin")
	void ReceiveCoinAmount(const FCoinValue& Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin")
	FCoinValue GetCoinAmount() const;

protected:

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Inventory")
	void Server_PlayerMoveItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft, BagSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerUnequipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, InventorySlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerEquipItemFromInventory(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft, BagSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerSwapEquipment(int64 DroppedItemId, InventorySlot DroppedInSlot, int64 SwappedItemId,
	                                InventorySlot DraggedOutSlot);

	void HandleTwoSlotItemEquip(const FBareItem& Item, InventorySlot& InSlot);

	void HandleTwoSlotItemUnequip(const FBareItem& Item, InventorySlot InSlot);

	void HandleEquipmentEffect(InventorySlot InSlot, const FBareItem& LocalItem);

	void HandleUnEquipmentEffect(InventorySlot InSlot, const FBareItem& LocalItem);

	UFUNCTION(BlueprintCallable)
	void RecomputeTotalWeight();

	/******************
	 *
	 *
	 *
	 *
	 ****************/

	UPROPERTY(ReplicatedUsing = OnRep_Weight, BlueprintReadOnly, Category = "Inventory|Inventory")
	float TotalWeight = 0.f;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Inventory")
	TObjectPtr<UPurseComponent> Purse;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Inventory")
	TObjectPtr<UFullInventoryComponent> Inventory;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Inventory")
	TObjectPtr<UEquipmentComponent> Equipment;

	/******************
	 * END OF INVENTORY RELATED STUFF
	 ****************/


	// Client only
	virtual void OnRep_PlayerState() override;

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
