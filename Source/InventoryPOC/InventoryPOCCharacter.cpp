// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryPOCCharacter.h"

#include "CommonUtilities.h"
#include "MainPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/GameplayStatics.h>
//////////////////////////////////////////////////////////////////////////
// AInventoryPOCCharacter

AInventoryPOCCharacter::AInventoryPOCCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	if (HasAuthority())
	{
		Purse = CreateDefaultSubobject<UPurseComponent>("Purse");
		Purse->SetNetAddressable(); // Make DSO components net addressable
		Purse->SetIsReplicated(true); // Enable replication by default
		Purse->PurseDispatcher_Server.AddDynamic(this, &AInventoryPOCCharacter::RecomputeTotalWeight);

		Equipment = CreateDefaultSubobject<UEquipmentComponent>("Equipment");
		Equipment->SetNetAddressable(); // Make DSO components net addressable
		Equipment->SetIsReplicated(true); // Enable replication by default
		Equipment->EquipmentDispatcher_Server.AddDynamic(this, &AInventoryPOCCharacter::RecomputeTotalWeight);

		Inventory = CreateDefaultSubobject<UFullInventoryComponent>("Inventory");
		Inventory->SetNetAddressable(); // Make DSO components net addressable
		Inventory->SetIsReplicated(true); // Enable replication by default
		Inventory->FullInventoryDispatcher_Server.AddDynamic(this, &AInventoryPOCCharacter::RecomputeTotalWeight);
	}

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//----------------------------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// Input

void AInventoryPOCCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AInventoryPOCCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AInventoryPOCCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AInventoryPOCCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AInventoryPOCCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AInventoryPOCCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AInventoryPOCCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AInventoryPOCCharacter::OnResetVR);
}


void AInventoryPOCCharacter::OnResetVR()
{
	// If InventoryPOC is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in InventoryPOC.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AInventoryPOCCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AInventoryPOCCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AInventoryPOCCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AInventoryPOCCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AInventoryPOCCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AInventoryPOCCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AInventoryPOCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AInventoryPOCCharacter, Equipment);
	DOREPLIFETIME_CONDITION(AInventoryPOCCharacter, Inventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AInventoryPOCCharacter, Purse, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AInventoryPOCCharacter, TotalWeight, COND_OwnerOnly);
}

// Server only
void AInventoryPOCCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AMainPlayerController* PC = Cast<AMainPlayerController>(GetController());
	if (PC)
	{
		PC->CreateHUD();
	}
}


//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	AMainPlayerController* PC = Cast<AMainPlayerController>(GetController());
	if (PC)
	{
		PC->CreateHUD();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Something bag happened in OnRep_PlayerState"));
	}
}

//----------------------------------------------------------------------------------------------------------------------

FOnFullInventoryComponentChanged& AInventoryPOCCharacter::GetInventoryDispatcher()
{
	check(Inventory);
	return Inventory->FullInventoryDispatcher;
}

//----------------------------------------------------------------------------------------------------------------------

FOnEquipmentChanged& AInventoryPOCCharacter::GetEquipmentDispatcher()
{
	check(Equipment);
	return Equipment->EquipmentDispatcher;
}

//----------------------------------------------------------------------------------------------------------------------

FOnPurseChangedDelegate& AInventoryPOCCharacter::GetPurseDispatcher()
{
	check(Purse);
	return Purse->PurseDispatcher;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FBareItem> AInventoryPOCCharacter::GetAllItems() const
{
  TArray<FBareItem> ItemsList;
  for(unsigned int BagID = 1; BagID < static_cast<unsigned int>(BagSlot::LastValidBag); ++BagID)
  {
    const TArray<FMinimalItemStorage>& Items = Inventory->GetBagConst(static_cast<BagSlot>(BagID));
        for(auto & Item : Items)
    {
          FBareItem LocalItem = UCommonUtilities::GetItemFromID(Item.ItemID, GetWorld());
          ItemsList.Add(LocalItem);
          }
  }
  return ItemsList;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& AInventoryPOCCharacter::GetAllItemsInBag(BagSlot Slot) const
{
	return Inventory->GetBagConst(Slot);
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::CanUnequipBag(InventorySlot Slot) const
{
	if (Slot != InventorySlot::WaistBag1 && Slot != InventorySlot::WaistBag2 &&
		Slot != InventorySlot::BackPack1 && Slot != InventorySlot::BackPack2)
		return true;

	return Inventory->GetBagConst(UFullInventoryComponent::GetBagSlotFromInventory(Slot)).Num() == 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::PlayerTryAutoLootFunction(int64 InItemId, InventorySlot& PossibleEquipment,
                                                       int32& InTopLeft,
                                                       BagSlot& PossibleBag) const
{
	PossibleEquipment = InventorySlot::Unknown;
	PossibleBag = BagSlot::Unknown;
	InTopLeft = -1;

	const FBareItem LocalItem = UCommonUtilities::GetItemFromID(InItemId, GetWorld());

	if (LocalItem.Equipable)
		PossibleEquipment = Equipment->FindSuitableSlot(LocalItem);

	if (PossibleEquipment != InventorySlot::Unknown)
		return true;

	PossibleBag = Inventory->FindSuitableSlot(LocalItem, InTopLeft);

	if (PossibleBag != BagSlot::Unknown)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerAddItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId)
{
	if (HasAuthority())
		Inventory->AddItemAt(InSlot, InItemId, InTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerRemoveItem(int32 TopLeft, BagSlot Slot)
{
	if (HasAuthority())
		Inventory->RemoveItem(Slot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

int64 AInventoryPOCCharacter::PlayerGetItem(int32 TopLeft, BagSlot Slot) const
{
	return Inventory->GetItemAtIndex(Slot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerMoveItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft,
                                            BagSlot OutSlot)
{
	Server_PlayerMoveItem(InTopLeft, InSlot, InItemId, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FBareItem> AInventoryPOCCharacter::GetAllEquipment() const
{
  return Equipment->GetAllEquipment();
}

//----------------------------------------------------------------------------------------------------------------------

const FBareItem& AInventoryPOCCharacter::GetEquippedItem(InventorySlot Slot) const
{
	return Equipment->GetItemAtSlot(Slot);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerEquip(InventorySlot InSlot, int64 InItemId)
{
	if (HasAuthority())
	{
		const FBareItem LocalItem = UCommonUtilities::GetItemFromID(InItemId, GetWorld());

		Equipment->EquipItem(LocalItem, InSlot);
		HandleEquipmentEffect(InSlot, LocalItem);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::PlayerTryAutoEquip(int64 InItemId, InventorySlot& PossibleEquipment) const
{
	const FBareItem LocalItem = UCommonUtilities::GetItemFromID(InItemId, GetWorld());
	PossibleEquipment = InventorySlot::Unknown;

	if (LocalItem.Equipable)
		PossibleEquipment = Equipment->FindSuitableSlot(LocalItem);

	if (PossibleEquipment != InventorySlot::Unknown)
		return true;

	return false;
}
//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerUnequipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, InventorySlot OutSlot)
{
	Server_PlayerUnequipItem(InTopLeft, InSlot, InItemId, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerEquipItemFromInventory(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft,
                                                          BagSlot OutSlot)
{
	Server_PlayerEquipItemFromInventory(InItemId, InSlot, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PlayerSwapEquipment(int64 InItemId, InventorySlot InSlot, int64 OutItemId,
                                                 InventorySlot OutSlot)
{
	Server_PlayerSwapEquipment(InItemId, InSlot, OutItemId, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::RecomputeTotalWeight()
{
	if (HasAuthority())
	{
		const float Weight = Inventory->GetTotalWeight() + Equipment->GetTotalWeight() + Purse->GetTotalWeight();
		TotalWeight = Weight;
		UE_LOG(LogTemp, Log, TEXT("New total weight %f (Inv : %f, Eqp : %f, Prs : %f"), Weight,
		       Inventory->GetTotalWeight(),
		       Equipment->GetTotalWeight(),
		       Purse->GetTotalWeight());
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::PayCoinAmount(const FCoinValue& Amount)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Paying and adjusting %s"), *GetName());
		Purse->PayAndAdjust(Amount);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::ReceiveCoinAmount(const FCoinValue& Amount)
{
	if (HasAuthority())
		Purse->AddCoins(Amount);
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue AInventoryPOCCharacter::GetCoinAmount() const
{
	return {Purse->GetCP(), Purse->GetSP(), Purse->GetGP(), Purse->GetPP()};
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::Server_PlayerMoveItem_Implementation(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
																  int32 OutTopLeft, BagSlot OutSlot)
{
	Inventory->RemoveItem(OutSlot, OutTopLeft);
	PlayerAddItem(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::Server_PlayerMoveItem_Validate(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
															int32 OutTopLeft, BagSlot OutSlot)
{
	return PlayerGetItem(OutTopLeft, OutSlot) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::Server_PlayerUnequipItem_Implementation(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
																	 InventorySlot OutSlot)
{
	HandleUnEquipmentEffect(OutSlot,Equipment->GetItemAtSlot(OutSlot));
	Equipment->RemoveItem(OutSlot);
	Inventory->AddItemAt(InSlot, InItemId, InTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::Server_PlayerUnequipItem_Validate(int32 InTopLeft, BagSlot InSlot, int64 InItemId,
															   InventorySlot OutSlot)
{
	const FBareItem ConsideredItem = Equipment->GetItemAtSlot(OutSlot);
	if (ConsideredItem.Key <= 0)
		return false;

	if (ConsideredItem.Bag)
	{
		if (Inventory->GetBagConst(UFullInventoryComponent::GetBagSlotFromInventory(OutSlot)).Num() > 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Unequip item was unempty bag slot %d, item %d"), OutSlot, InItemId);
			return false;
		}
	}

	return ConsideredItem.Key == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::Server_PlayerEquipItemFromInventory_Implementation(
	int64 InItemId, InventorySlot InSlot, int32 OutTopLeft, BagSlot OutSlot)
{
	PlayerEquip(InSlot, InItemId);
	Inventory->RemoveItem(OutSlot, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::Server_PlayerEquipItemFromInventory_Validate(
	int64 InItemId, InventorySlot InSlot, int32 OutTopLeft, BagSlot OutSlot)
{
	UE_LOG(LogTemp, Log, TEXT("Validating item equipment %d"), InItemId);

	if (InItemId <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Input item is invalid %d"), InItemId);
		return false;
	}

	if (PlayerGetItem(OutTopLeft, OutSlot) != InItemId)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find item in player inventory %d"), InItemId);
		return false;
	}


	if (GetEquippedItem(InSlot).Key > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Target slot is not empty (%d"), InSlot);
		return false;
	}

	const FBareItem LocalItem = UCommonUtilities::GetItemFromID(InItemId,GetWorld());

	if (LocalItem.Key <= 0)
		return false;

	if (!LocalItem.Equipable) //other check to be performed here
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------


void AInventoryPOCCharacter::Server_PlayerSwapEquipment_Implementation(int64 DroppedItemId, InventorySlot DroppedInSlot,
																	   int64 SwappedItemId,
																	   InventorySlot DraggedOutSlot)
{
	FBareItem ItemToMove = GetEquippedItem(DroppedInSlot);
	FBareItem DroppedItem = GetEquippedItem(DraggedOutSlot);

	Equipment->RemoveItem(DroppedInSlot);
	HandleUnEquipmentEffect(DroppedInSlot, ItemToMove);
	Equipment->EquipItem(DroppedItem, DroppedInSlot);
	HandleEquipmentEffect(DroppedInSlot, DroppedItem);

	Equipment->RemoveItem(DraggedOutSlot);
	HandleUnEquipmentEffect(DraggedOutSlot, DroppedItem);
	Equipment->EquipItem(ItemToMove, DraggedOutSlot);
	HandleEquipmentEffect(DraggedOutSlot, ItemToMove);
}

//----------------------------------------------------------------------------------------------------------------------

bool AInventoryPOCCharacter::Server_PlayerSwapEquipment_Validate(int64 DroppedItemId, InventorySlot DroppedInSlot,
																 int64 SwappedItemId,
																 InventorySlot DraggedOutSlot)
{
	UE_LOG(LogTemp, Log, TEXT("Validating equipment swap from %d item %d to %d item %d"), DraggedOutSlot, SwappedItemId,
		   DroppedInSlot, DroppedItemId);

	if (DroppedInSlot == InventorySlot::Unknown)
		return false;

	if (DraggedOutSlot == InventorySlot::Unknown)
		return false;

	if (GetEquippedItem(DraggedOutSlot).Key != DroppedItemId)
		return false;

	if (GetEquippedItem(DroppedInSlot).Key != SwappedItemId)
		return false;

	if (GetEquippedItem(DraggedOutSlot).TwoSlotsItem)
		return GetEquippedItem(DroppedInSlot).TwoSlotsItem;

	return true;
}
//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::HandleTwoSlotItemEquip(const FBareItem& Item, InventorySlot& InSlot)
{
	if (Item.TwoSlotsItem)
	{
		InventorySlot PrimarySlot = InSlot;
		InventorySlot SecondarySlot = InSlot;

		if (PrimarySlot == InventorySlot::BackPack1 || PrimarySlot == InventorySlot::BackPack2)
		{
			PrimarySlot = InventorySlot::BackPack1;
			SecondarySlot = InventorySlot::BackPack2;
		}
		else if (PrimarySlot == InventorySlot::WaistBag1 || PrimarySlot == InventorySlot::WaistBag2)
		{
			PrimarySlot = InventorySlot::WaistBag1;
			SecondarySlot = InventorySlot::WaistBag2;
		}

		FBareItem GhostItem;
		GhostItem.Key = 0;
		GhostItem.IconName = Item.IconName;

		Equipment->EquipItem(GhostItem, SecondarySlot);
		InSlot = PrimarySlot;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::HandleTwoSlotItemUnequip(const FBareItem& Item, InventorySlot InSlot)
{
	if (Item.TwoSlotsItem)
	{
		InventorySlot PrimarySlot = InSlot;
		InventorySlot SecondarySlot = InSlot;

		if (PrimarySlot == InventorySlot::BackPack1)
			SecondarySlot = InventorySlot::BackPack2;

		if (PrimarySlot == InventorySlot::WaistBag1)
			SecondarySlot = InventorySlot::WaistBag2;

		Equipment->RemoveItem(SecondarySlot);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::HandleEquipmentEffect(InventorySlot InSlot, const FBareItem& LocalItem)
{
	HandleTwoSlotItemEquip(LocalItem, InSlot);
	//do equipment specific stuff here
	if (LocalItem.Bag)
	{
		const BagSlot AffectedSlot = UFullInventoryComponent::GetBagSlotFromInventory(InSlot);
		Inventory->BagSet(AffectedSlot, true, LocalItem.BagWidth, LocalItem.BagHeight, LocalItem.BagSize);
	}
}
//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::HandleUnEquipmentEffect(InventorySlot InSlot, const FBareItem& LocalItem)
{
	HandleTwoSlotItemUnequip(LocalItem, InSlot);
	//do equipment specific stuff here
	if (LocalItem.Bag)
	{
		const BagSlot AffectedSlot = UFullInventoryComponent::GetBagSlotFromInventory(InSlot);
		Inventory->BagSet(AffectedSlot, false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void AInventoryPOCCharacter::OnRep_Weight()
{
	WeightDispatcher.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Total weight modified"));
}
