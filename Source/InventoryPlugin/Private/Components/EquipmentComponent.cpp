#include "Components/EquipmentComponent.h"
#include <Net/UnrealNetwork.h>

#include "Actors/InventoryLightSourceActor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/InventoryModularCharacterInterface.h"
#include "Items/InventoryItemBag.h"
#include "Items/InventoryItemEquipable.h"
#include "Items/Interfaces/InventoryItemAmmoBagInterface.h"

//----------------------------------------------------------------------------------------------------------------------

UStaticMeshComponent* UEquipmentComponent::GetMeshComponentFromSocket(EEquipmentSocket Socket) const
{
	switch (Socket)
	{
	case EEquipmentSocket::AmmoBag: return AmmoComponent;
	case EEquipmentSocket::WaistBag1: return WaistBag1Component;
	case EEquipmentSocket::WaistBag2: return WaistBag2Component;
	case EEquipmentSocket::ShoulderBag1: return ShoulderBag1Component;
	case EEquipmentSocket::ShoulderBag2: return ShoulderBag2Component;
	case EEquipmentSocket::Backpack: return BackpackComponent;
	case EEquipmentSocket::Unknown: return nullptr;

	case EEquipmentSocket::EarL: return EarringLComponent;
	case EEquipmentSocket::EarR: return EarringRComponent;
	case EEquipmentSocket::RingR: return RingRComponent;
	case EEquipmentSocket::RingL: return RingLComponent;

	case EEquipmentSocket::WristL: return WristLComponent;
	case EEquipmentSocket::WristR: return WristRComponent;

	default: return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

USkeletalMeshComponent* UEquipmentComponent::GetSkeletalMeshComponentFromSocket(EEquipmentSocket Socket) const
{
	switch (Socket)
	{
	case EEquipmentSocket::Unknown: return nullptr;
	case EEquipmentSocket::Primary: return PrimaryWeaponComponent;
	case EEquipmentSocket::Secondary: return SecondaryWeaponComponent;
	case EEquipmentSocket::PrimarySheath: return PrimaryWeaponSheath;
	case EEquipmentSocket::SecondarySheath: return SecondaryWeaponSheath;
	case EEquipmentSocket::BackSheath: return BackWeaponSheath;
	case EEquipmentSocket::RangedSheath: return RangedWeaponSheath;
	case EEquipmentSocket::Head:
		{
			if (auto* ModularCharacter = Cast<IInventoryModularCharacterInterface>(GetOwner()))
			{
				return ModularCharacter->GetHelmetComponent();
			}
			return nullptr;
		}
	//case EEquipmentSocket::WristL: return LeftBracerComponent;
	//case EEquipmentSocket::WristR: return RightBracerComponent;

	default: return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::Equip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot)
{
	const EEquipmentSocket PossibleSocket = FindBestSocketForItem(Item, EquipSlot);

	if (PossibleSocket == EEquipmentSocket::Unknown)
	{
		return false;
	}

	// skeletal mesh have precedence over static mesh
	if (Item->EquipmentMesh)
	{
		USkeletalMeshComponent* SkeletalSocket = GetSkeletalMeshComponentFromSocket(PossibleSocket);

		if (SkeletalSocket)
		{
			UpdateEquipment(SkeletalSocket, Item->EquipmentMesh, Item->EquipmentMeshMaterialOverride);
			return true;
		}
	}

	UStaticMeshComponent* WantedSocket = GetMeshComponentFromSocket(PossibleSocket);

	if (!WantedSocket)
	{
		return false;
	}
	UStaticMesh* StaticMesh = Item->Mesh;
	if (const IEquipmentInterface* Interface = Cast<IEquipmentInterface>(GetOwner()))
		StaticMesh = Interface->GetPreferedMesh(StaticMesh);

	if (!StaticMesh)
	{
		return false;
	}

	if (Item->OverrideMaterial.OverrideMaterial)
		WantedSocket->SetMaterial(Item->OverrideMaterial.MaterialID, Item->OverrideMaterial.OverrideMaterial);

	return WantedSocket->SetStaticMesh(StaticMesh);
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::UnEquip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot)
{
	const EEquipmentSocket PossibleSocket = FindBestSocketForItem(Item, EquipSlot);

	if (PossibleSocket == EEquipmentSocket::Unknown)
	{
		return false;
	}

	if (Item->EquipmentMesh)
	{
		if (USkeletalMeshComponent* SkeletalSocket = GetSkeletalMeshComponentFromSocket(PossibleSocket))
		{
			UpdateEquipment(SkeletalSocket, nullptr, {});
			SkeletalSocket->SetSkeletalMeshAsset(nullptr);
			return true;
		}
	}

	const UStaticMesh* StaticMesh = Item->Mesh;
	if (!StaticMesh)
	{
		return false;
	}

	UStaticMeshComponent* WantedSocket = GetMeshComponentFromSocket(PossibleSocket);

	if (!WantedSocket)
	{
		return false;
	}

	return WantedSocket->SetStaticMesh(nullptr);
}

//----------------------------------------------------------------------------------------------------------------------

EEquipmentSocket UEquipmentComponent::FindBestSocketForItem(const UInventoryItemEquipable* Item,
                                                            EEquipmentSlot EquipSlot)
{
	switch (EquipSlot)
	{
	case EEquipmentSlot::Primary:
		{
			if (!Item->Weapon || Item->Unsheathable)
			{
				return EEquipmentSocket::Primary;
			}

			return EEquipmentSocket::PrimarySheath;
		}
	case EEquipmentSlot::Secondary:
		{
			if (Item->Shield)
			{
				return EEquipmentSocket::BackSheath;
			}

			if (!Item->Weapon)
			{
				return EEquipmentSocket::Secondary;
			}

			return EEquipmentSocket::SecondarySheath;
		}
	case EEquipmentSlot::Range:
		{
			return EEquipmentSocket::RangedSheath;
		}
	case EEquipmentSlot::Ammo: return EEquipmentSocket::AmmoBag;
	case EEquipmentSlot::Head:
		{
			return EEquipmentSocket::Head;
		}
	case EEquipmentSlot::Face:
		{
			return EEquipmentSocket::Face;
		}
	case EEquipmentSlot::WristL:
		{
			return EEquipmentSocket::WristL;
		}
	case EEquipmentSlot::WristR:
		{
			return EEquipmentSocket::WristR;
		}
	case EEquipmentSlot::Shoulders: break;
	case EEquipmentSlot::Back: break;
	case EEquipmentSlot::Waist: break;
	case EEquipmentSlot::WaistBag1: return EEquipmentSocket::WaistBag1;
	case EEquipmentSlot::WaistBag2: return EEquipmentSocket::WaistBag2;
	case EEquipmentSlot::BackPack1:
		{
			if (Item->MultiSlotItem)
			{
				return EEquipmentSocket::Backpack;
			}
			return EEquipmentSocket::ShoulderBag1;
		}
	case EEquipmentSlot::BackPack2:
		{
			if (Item->MultiSlotItem)
			{
				return EEquipmentSocket::Backpack;
			}
			return EEquipmentSocket::ShoulderBag2;
		}

	case EEquipmentSlot::EarL:
		return EEquipmentSocket::EarL;

	case EEquipmentSlot::EarR:
		return EEquipmentSocket::EarR;

	case EEquipmentSlot::FingerL:
		return EEquipmentSocket::RingL;

	case EEquipmentSlot::FingerR:
		return EEquipmentSocket::RingR;
	default: ;
	}

	return EEquipmentSocket::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::Unsheath(EEquipmentSlot SlotToUnsheath)
{
	const UInventoryItemEquipable* Item = Equipment[static_cast<int>(SlotToUnsheath)];

	if (!Item)
		return;

	if (Item->Unsheathable)
		return;

	const EEquipmentSocket SheathSocket = FindBestSocketForItem(Item, SlotToUnsheath);

	if (SheathSocket != EEquipmentSocket::PrimarySheath && SheathSocket != EEquipmentSocket::SecondarySheath &&
		SheathSocket != EEquipmentSocket::BackSheath &&
		SheathSocket != EEquipmentSocket::RangedSheath)
	{
		return;
	}

	EEquipmentSocket LiveSocket = EEquipmentSocket::Unknown;

	switch (SheathSocket)
	{
	case EEquipmentSocket::PrimarySheath: LiveSocket = EEquipmentSocket::Primary;
		break;
	case EEquipmentSocket::SecondarySheath: LiveSocket = EEquipmentSocket::Secondary;
		break;

	case EEquipmentSocket::BackSheath:
		{
			if (SlotToUnsheath == EEquipmentSlot::Primary)
			{
				LiveSocket = EEquipmentSocket::Primary;
			}
			else if (SlotToUnsheath == EEquipmentSlot::Secondary)
			{
				LiveSocket = EEquipmentSocket::Secondary;
			}
			else if (SlotToUnsheath == EEquipmentSlot::Range)
			{
				LiveSocket = EEquipmentSocket::Primary;
			}
			break;
		}
	case EEquipmentSocket::RangedSheath:
		{
			if (SlotToUnsheath == EEquipmentSlot::Range)
			{
				LiveSocket = EEquipmentSocket::Primary;
			}
		}
		break;
	default:
		return;
	}

	if (LiveSocket == EEquipmentSocket::Primary)
	{
		PrimaryWeaponOriginalSlot = SheathSocket;
	}

	else if (LiveSocket == EEquipmentSocket::Secondary)
	{
		SecondaryWeaponOriginalSlot = SheathSocket;
	}


	USkeletalMeshComponent* LiveSocketComponent = GetSkeletalMeshComponentFromSocket(LiveSocket);
	USkeletalMeshComponent* SheathSocketComponent = GetSkeletalMeshComponentFromSocket(SheathSocket);

	if (SheathSocketComponent->GetSkeletalMeshAsset() == nullptr || LiveSocketComponent->GetSkeletalMeshAsset() !=
		nullptr)
	{
		return;
	}

	USkeletalMesh* MeshPointer = SheathSocketComponent->GetSkeletalMeshAsset();
	SheathSocketComponent->SetSkeletalMeshAsset(nullptr);
	LiveSocketComponent->SetSkeletalMeshAsset(MeshPointer);

	IsHoldingATwoHandedWeapon = Item->MultiSlotItem;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::Sheath()
{
	if (PrimaryWeaponComponent->GetSkeletalMeshAsset() != nullptr)
	{
		USkeletalMeshComponent* ReturnSocket = GetSkeletalMeshComponentFromSocket(PrimaryWeaponOriginalSlot);


		if (!ReturnSocket)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid return socket for primary weapon sheathing"));
			return;
		}

		// If sheath already has mesh, log warning but continue (overwrite)
		if (ReturnSocket->GetSkeletalMeshAsset())
		{
			UE_LOG(LogTemp, Warning, TEXT("Primary sheath socket already occupied, overwriting"));
		}

		USkeletalMesh* MeshPointer = PrimaryWeaponComponent->GetSkeletalMeshAsset();
		PrimaryWeaponComponent->SetSkeletalMeshAsset(nullptr);
		ReturnSocket->SetSkeletalMeshAsset(MeshPointer);
	}

	if (SecondaryWeaponComponent->GetSkeletalMeshAsset() != nullptr)
	{
		USkeletalMeshComponent* ReturnSocket = GetSkeletalMeshComponentFromSocket(SecondaryWeaponOriginalSlot);


		if (!ReturnSocket)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid return socket for secondary weapon sheathing"));
			return;
		}

		// If sheath already has mesh, log warning but continue (overwrite)
		if (ReturnSocket->GetSkeletalMeshAsset())
		{
			UE_LOG(LogTemp, Warning, TEXT("Secondary sheath socket already occupied, overwriting"));
		}

		USkeletalMesh* MeshPointer = SecondaryWeaponComponent->GetSkeletalMeshAsset();
		SecondaryWeaponComponent->SetSkeletalMeshAsset(nullptr);
		ReturnSocket->SetSkeletalMeshAsset(MeshPointer);
	}

	IsHoldingATwoHandedWeapon = false;
}

void UEquipmentComponent::UpdateEquipment_Implementation(USkeletalMeshComponent* SkeletalSocket,
                                                         USkeletalMesh* LocalItem,
                                                         const TArray<FMaterialOverride>& MaterialOverride)
{
	if (SkeletalSocket)
	{
		SkeletalSocket->SetSkeletalMeshAsset(LocalItem);
		for (auto& Material : MaterialOverride)
		{
			SkeletalSocket->SetMaterial(Material.MaterialID, Material.OverrideMaterial);
			if (UMaterialInstanceDynamic* DynMat = SkeletalSocket->CreateAndSetMaterialInstanceDynamic(
				Material.MaterialID))
			{
				DynMat->SetVectorParameterValue(TEXT("Tint"), Material.TintColor);
				DynMat->SetScalarParameterValue(TEXT("TintIntensity"), Material.TintIntensity);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Equipment.Init(nullptr, 32);
	EquipmentDurability.Init(100.0f, 32);

	PrimaryWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrimaryWeaponComponentMesh"));
	PrimaryWeaponComponent->SetIsReplicated(true);
	SecondaryWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SecondaryWeaponComponentMesh"));
	SecondaryWeaponComponent->SetIsReplicated(true);

	PrimaryWeaponSheath = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrimaryWeaponSheathComponentMesh"));
	PrimaryWeaponSheath->SetIsReplicated(true);
	SecondaryWeaponSheath = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SecondaryWeaponSheathComponentMesh"));
	SecondaryWeaponSheath->SetIsReplicated(true);
	BackWeaponSheath = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackWeaponSheathComponentMesh"));
	BackWeaponSheath->SetIsReplicated(true);

	RangedWeaponSheath = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerBackWeaponSheathComponentMesh"));
	RangedWeaponSheath->SetIsReplicated(true);

	AmmoComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoComponentMesh"));
	AmmoVariableComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoVariableComponentMesh"));
	AmmoComponent->SetIsReplicated(true);
	AmmoVariableComponent->SetIsReplicated(true);
	WaistBag1Component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaistBag1ComponentMesh"));
	WaistBag1Component->SetIsReplicated(true);
	WaistBag2Component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaistBag2ComponentMesh"));
	WaistBag2Component->SetIsReplicated(true);
	ShoulderBag1Component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShoulderBag1ComponentMesh"));
	ShoulderBag1Component->SetIsReplicated(true);
	ShoulderBag2Component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShoulderBag2ComponentMesh"));
	ShoulderBag2Component->SetIsReplicated(true);
	BackpackComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackComponentMesh"));
	BackpackComponent->SetIsReplicated(true);

	EarringLComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EarringLComponent"));
	EarringLComponent->SetIsReplicated(true);
	EarringRComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EarringRComponent"));
	EarringRComponent->SetIsReplicated(true);
	RingLComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingLComponent"));
	RingLComponent->SetIsReplicated(true);
	RingRComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingRComponent"));
	RingRComponent->SetIsReplicated(true);

	SecondaryLightSource = CreateDefaultSubobject<UChildActorComponent>(TEXT("SecondaryLightSource"));
	SecondaryLightSource->SetIsReplicated(true);

	WristLComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WristLComponent"));
	WristLComponent->SetIsReplicated(true);
	WristRComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WristRComponent"));
	WristRComponent->SetIsReplicated(true);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UpdateMasterMeshComponent(USkeletalMeshComponent* Mesh)
{
	for (auto&& [MeshPointer, MeshComponent] : VariableMeshesMap)
	{
		MeshComponent->SetLeaderPoseComponent(Mesh);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::TryUpdateDynamicMeshes(const TMap<EEquipmentSlot, USkeletalMesh*>& MeshArray,
	const TMap<EEquipmentSlot, TArray<FMaterialOverride>>& OverrideArray)
{

	TMap<EEquipmentSlot, bool> RelevantSlots;

	for (auto&& [Slot, NotUsed] : VariableMeshesMap)
	{
		RelevantSlots.Emplace(Slot, false);
	}

	for (auto && [Slot, MeshPointer] : MeshArray)
	{
		RelevantSlots.FindOrAdd(Slot) = true;

		//If the slot and component already exist
		if (VariableMeshesMap.Contains(Slot))
		{
			auto& NewSkeletalMeshComponent = VariableMeshesMap.FindChecked(Slot);
			if (NewSkeletalMeshComponent->GetSkeletalMeshAsset() != MeshPointer)
			{
				NewSkeletalMeshComponent->SetSkeletalMesh(MeshPointer);
			}
		}
		else // We need to add this one
		{
			USkeletalMeshComponent* NewSkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this);
			NewSkeletalMeshComponent->RegisterComponent();
			NewSkeletalMeshComponent->SetIsReplicated(true);
			NewSkeletalMeshComponent->SetSkeletalMesh(MeshPointer);
			NewSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

			FAttachmentTransformRules TransformRules2(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
								  EAttachmentRule::SnapToTarget, true);


			NewSkeletalMeshComponent->AttachToComponent(Cast<ACharacter>(GetOwner())->GetMesh(), TransformRules2);
			NewSkeletalMeshComponent->SetLeaderPoseComponent(Cast<ACharacter>(GetOwner())->GetMesh());
			VariableMeshesMap.Emplace(Slot, NewSkeletalMeshComponent);
		}
	}

	//cleanup obsolete slots
	for (auto&& [Slot, SlotStatus] : RelevantSlots)
	{
		if (SlotStatus == false)
		{
			auto& NewSkeletalMeshComponent = VariableMeshesMap.FindChecked(Slot);
			NewSkeletalMeshComponent->UnregisterComponent();
			NewSkeletalMeshComponent->MarkAsGarbage();
		}
	}


}
//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SellMaterialForAllMeshes(int MaterialID, UMaterialInstance* MaterialInstance)
{
	for (auto& [Slot, MeshComponent] : VariableMeshesMap)
	{
		USkeletalMeshComponent* Mesh = MeshComponent;
		if (Mesh->GetNumMaterials() > MaterialID)
		{
			Mesh->SetMaterial(MaterialID, MaterialInstance);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		return;
	}

	ACharacter* Parent = Cast<ACharacter>(GetOwner());
	if (!Parent)
	{
		return;
	}
	USkeletalMeshComponent* PlayerMesh = Parent->GetMesh();

	if (!PlayerMesh)
	{
		return;
	}

	if (!PrimaryWeaponComponent)
	{
		return;
	}

	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);

	PrimaryWeaponComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_RightHandWeapon"));
	SecondaryWeaponComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_LeftHandWeapon"));
	AmmoComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_AmmoBag"));
	AmmoVariableComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_AmmoBag"));
	PrimaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("PrimarySheath"));
	SecondaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SecondarySheath"));
	BackWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("BackSheath"));
	RangedWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("LowerBackSheath"));


	WaistBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_WaistBag1"));
	WaistBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_WaistBag2"));
	ShoulderBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_ShoulderBag1"));
	ShoulderBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_ShoulderBag2"));
	BackpackComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_Backpack"));

	RingLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("RingL"));
	RingRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("RingR"));

	EarringLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("EarR"));
	EarringRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("EarL"));

	WristLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("WristL"));
	WristRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("WristR"));

	//Disable camera collision for EVERY piece of equipment
	for (auto&& [MeshPointer, MeshComponent] : VariableMeshesMap)
	{
		FAttachmentTransformRules TransformRules2(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
										  EAttachmentRule::SnapToTarget, true);
		MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComponent->AttachToComponent(PlayerMesh, TransformRules2);
		MeshComponent->SetLeaderPoseComponent(Cast<ACharacter>(GetOwner())->GetMesh());
	}

	PrimaryWeaponComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SecondaryWeaponComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	AmmoComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	AmmoVariableComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	PrimaryWeaponSheath->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SecondaryWeaponSheath->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BackWeaponSheath->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RangedWeaponSheath->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ShoulderBag1Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ShoulderBag2Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WaistBag1Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WaistBag2Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BackpackComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Enforce refresh of skeletal mesh equipment that may otherwise be preloaded and ignored
	if (auto PrimaryWeapon = GetItemAtSlot(EEquipmentSlot::Primary))
		Equip(PrimaryWeapon, EEquipmentSlot::Primary);

	if (auto SecondaryWeapon = GetItemAtSlot(EEquipmentSlot::Secondary))
		Equip(SecondaryWeapon, EEquipmentSlot::Secondary);

	if (auto RangedWeapon = GetItemAtSlot(EEquipmentSlot::Range))
		Equip(RangedWeapon, EEquipmentSlot::Range);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipmentComponent, Equipment);
	DOREPLIFETIME(UEquipmentComponent, EquipmentDurability);
	DOREPLIFETIME(UEquipmentComponent, IsHoldingATwoHandedWeapon);
	DOREPLIFETIME(UEquipmentComponent, PrimaryWeaponComponent);
	DOREPLIFETIME(UEquipmentComponent, SecondaryWeaponComponent);
	DOREPLIFETIME(UEquipmentComponent, PrimaryWeaponSheath);
	DOREPLIFETIME(UEquipmentComponent, SecondaryWeaponSheath);
	DOREPLIFETIME(UEquipmentComponent, BackWeaponSheath);
	DOREPLIFETIME(UEquipmentComponent, RangedWeaponSheath);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::OnRep_ItemList()
{
	EquipmentDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::OnRep_EquipmentDurability()
{
	// Broadcast to notify UI widgets that durability has changed
	EquipmentDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot)
{
	if (Equipment[static_cast<int>(InSlot)] == nullptr)
	{
		Equipment[static_cast<int>(InSlot)] = Item;

		// Initialize with default durability from item definition
		int32 SlotIndex = static_cast<int>(InSlot);
		if (EquipmentDurability.Num() <= SlotIndex)
		{
			EquipmentDurability.SetNum(Equipment.Num());
		}
		EquipmentDurability[SlotIndex] = Item->Durability;

		Equip(Item, InSlot);
		EquipmentDispatcher_Server.Broadcast();
		ItemEquipedDispatcher_Server.Broadcast(InSlot, Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipItemWithDurability(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot, float Durability)
{
	if (Equipment[static_cast<int>(InSlot)] == nullptr)
	{
		Equipment[static_cast<int>(InSlot)] = Item;

		// Ensure durability array is sized correctly
		int32 SlotIndex = static_cast<int>(InSlot);
		if (EquipmentDurability.Num() <= SlotIndex)
		{
			EquipmentDurability.SetNum(Equipment.Num());
		}

		EquipmentDurability[SlotIndex] = Durability;

		Equip(Item, InSlot);
		EquipmentDispatcher_Server.Broadcast();
		ItemEquipedDispatcher_Server.Broadcast(InSlot, Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::GetEquipmentDurability(EEquipmentSlot InSlot, float& OutDurability) const
{
	int32 SlotIndex = static_cast<int>(InSlot);
	if (EquipmentDurability.IsValidIndex(SlotIndex) && Equipment[SlotIndex] != nullptr)
	{
		OutDurability = EquipmentDurability[SlotIndex];
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SetEquipmentDurability(EEquipmentSlot InSlot, float Durability)
{
	int32 SlotIndex = static_cast<int>(InSlot);
	if (EquipmentDurability.IsValidIndex(SlotIndex))
	{
		EquipmentDurability[SlotIndex] = Durability;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::ReduceEquipmentDurability(EEquipmentSlot InSlot, float DurabilityReduction)
{
	int32 SlotIndex = static_cast<int>(InSlot);
	if (!EquipmentDurability.IsValidIndex(SlotIndex) || Equipment[SlotIndex] == nullptr)
	{
		return;
	}

	if (DurabilityReduction > 0.0f)
	{
		const UInventoryItemEquipable* Item = Equipment[SlotIndex];
		if (!Item || Item->TotalDurability <= 0.0f)
		{
			return;
		}

		float OldDurability = EquipmentDurability[SlotIndex];
		float NewDurability = FMath::Max(0.0f, OldDurability - DurabilityReduction);
		EquipmentDurability[SlotIndex] = NewDurability;

		// Calculate durability percentages
		float OldPercent = (OldDurability / Item->TotalDurability) * 100.0f;
		float NewPercent = (NewDurability / Item->TotalDurability) * 100.0f;

		// Check for threshold crossings and broadcast warnings
		// Thresholds: 50%, 25%, 10%, 0%
		if (OldPercent > 50.0f && NewPercent <= 50.0f)
		{
			DurabilityWarningDispatcher.Broadcast(InSlot, NewPercent, Item);
		}
		else if (OldPercent > 25.0f && NewPercent <= 25.0f)
		{
			DurabilityWarningDispatcher.Broadcast(InSlot, NewPercent, Item);
		}
		else if (OldPercent > 10.0f && NewPercent <= 10.0f)
		{
			DurabilityWarningDispatcher.Broadcast(InSlot, NewPercent, Item);
		}
		else if (OldPercent > 0.0f && NewPercent <= 0.0f)
		{
			DurabilityWarningDispatcher.Broadcast(InSlot, NewPercent, Item);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SetEquipmentLockState(EEquipmentSlot InSlot, bool bLocked)
{
	if (InSlot == EEquipmentSlot::Unknown || InSlot >= EEquipmentSlot::Last)
	{
		return;
	}

	EquipmentLockStates.Add(InSlot, bLocked);
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::GetEquipmentLockState(EEquipmentSlot InSlot) const
{
	if (InSlot == EEquipmentSlot::Unknown || InSlot >= EEquipmentSlot::Last)
	{
		return false;
	}

	const bool* LockState = EquipmentLockStates.Find(InSlot);
	return LockState ? *LockState : false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::IsSlotEmpty(EEquipmentSlot InSlot)
{
	if (!Equipment[static_cast<int>(InSlot)])
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<const UInventoryItemEquipable*>& UEquipmentComponent::GetAllEquipment() const
{
	return Equipment;
}

//----------------------------------------------------------------------------------------------------------------------

const UInventoryItemEquipable* UEquipmentComponent::GetItemAtSlot(EEquipmentSlot InSlot) const
{
	return Equipment[static_cast<int>(InSlot)];
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::RemoveItem(EEquipmentSlot InSlot)
{
	if (IsSlotEmpty(InSlot))
		return false;

	UnEquip(Equipment[static_cast<int>(InSlot)], InSlot);
	ItemUnEquipedDispatcher_Server.Broadcast(InSlot, Equipment[static_cast<int>(InSlot)]);

	Equipment[static_cast<int>(InSlot)] = nullptr;
	EquipmentDispatcher_Server.Broadcast();
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::RemoveAll()
{
	for (uint32 SlotId = 0; SlotId < static_cast<uint32>(EEquipmentSlot::Last); ++SlotId)
	{
		RemoveItem(static_cast<EEquipmentSlot>(SlotId));
	}
}

//----------------------------------------------------------------------------------------------------------------------

float UEquipmentComponent::GetTotalWeight() const
{
	float TotalWeight = 0.f;
	for (const auto& Item : Equipment)
	{
		if (Item)
		{
			TotalWeight += Item->Weight;
		}
	}
	return TotalWeight;
}

//----------------------------------------------------------------------------------------------------------------------

EEquipmentSlot UEquipmentComponent::FindSuitableSlot(const UInventoryItemEquipable* Item) const
{
	// In the specific case of multiple slots items, if one of the slot is used, we can't equip it.
	if (Item->MultiSlotItem)
	{
		EEquipmentSlot PrimarySlot = EEquipmentSlot::Unknown;

		for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown); i < static_cast<int32>(EEquipmentSlot::Last); ++i)
		{
			const int32 LocalAcceptableBitMask = 1 << i;

			if (Item->EquipableSlotBitMask & LocalAcceptableBitMask)
			{
				if (Equipment[i])
					return PrimarySlot;
			}
		}
	}

	for (size_t i = 1; i < Equipment.Num(); ++i)
	{
		if (!Equipment[i])
		{
			const int32 LocalAcceptableBitMask = 1 << i;
			const EEquipmentSlot CurrentSlot = static_cast<EEquipmentSlot>(i);
			if (Item->EquipableSlotBitMask & LocalAcceptableBitMask)
			{
				return CurrentSlot;
			}
		}
	}

	return EEquipmentSlot::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UnsheathMelee()
{
	Unsheath(EEquipmentSlot::Primary);
	Unsheath(EEquipmentSlot::Secondary);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SheathMelee()
{
	Sheath();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UnsheathRanged()
{
	Unsheath(EEquipmentSlot::Range);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SheathRanged()
{
	if (PrimaryWeaponComponent->GetSkeletalMeshAsset() != nullptr)
	{
		USkeletalMeshComponent* ReturnSocket = GetSkeletalMeshComponentFromSocket(PrimaryWeaponOriginalSlot);

		if (!ReturnSocket || ReturnSocket->GetSkeletalMeshAsset())
			return;

		USkeletalMesh* MeshPointer = PrimaryWeaponComponent->GetSkeletalMeshAsset();
		PrimaryWeaponComponent->SetSkeletalMeshAsset(nullptr);
		ReturnSocket->SetSkeletalMeshAsset(MeshPointer);
	}

	IsHoldingATwoHandedWeapon = false;
}

//----------------------------------------------------------------------------------------------------------------------

FBoxSphereBounds UEquipmentComponent::GetEquipmentOverlapBox(EEquipmentSlot Slot) const
{
	USkeletalMeshComponent* Component = nullptr;

	switch (Slot)
	{
	case EEquipmentSlot::Primary: Component = PrimaryWeaponComponent;
		break;
	case EEquipmentSlot::Secondary: Component = SecondaryWeaponComponent;
		break;
	default: return {};
	}

	if (!IsValid(Component->GetSkeletalMeshAsset()))
	{
		return {};
	}

	const FBoxSphereBounds MeshBounds = Component->GetSkeletalMeshAsset()->GetBounds();

	return MeshBounds;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SetEquipmentLight(TSubclassOf<AInventoryLightSourceActor> LightActor,
                                            EEquipmentSlot Slot) const
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		EquipLightItem(LightActor);
	}
	if (LightActor)
	{
		ACharacter* Parent = Cast<ACharacter>(GetOwner());
		if (!Parent)
		{
			return;
		}

		USkeletalMeshComponent* PlayerMesh = Parent->GetMesh();

		if (!PlayerMesh)
		{
			return;
		}

		FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		switch (Slot)
		{
		case EEquipmentSlot::Secondary:
			break;
		default: ;
		}
		SecondaryLightSource->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_LeftHandWeapon"));
		SecondaryLightSource->SetChildActorClass(LightActor);
		SecondaryLightSource->CreateChildActor();
	}
	else
	{
		SecondaryLightSource->DestroyChildActor();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::SetAllEquipmentCollisionDisabled()
{
	PrimaryWeaponComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SecondaryWeaponComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	AmmoComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	PrimaryWeaponSheath->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SecondaryWeaponSheath->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	BackWeaponSheath->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RangedWeaponSheath->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ShoulderBag1Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ShoulderBag2Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WaistBag1Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WaistBag2Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	BackpackComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	for (auto&& [MeshPointer, MeshComponent] : VariableMeshesMap)
	{
		MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::IsWeaponTwoHanded() const
{
	return IsHoldingATwoHandedWeapon;
}

//----------------------------------------------------------------------------------------------------------------------

FTransform UEquipmentComponent::GetOffHandTransform() const
{
	FTransform Out;
	if (PrimaryWeaponComponent && IsHoldingATwoHandedWeapon && PrimaryWeaponComponent->GetSkeletalMeshAsset())
	{
		Out = PrimaryWeaponComponent->GetSocketTransform(FName("SOCKET_LeftHandPosition"),
		                                                 ERelativeTransformSpace::RTS_World);
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UpdateBagUsage(EBagSlot BagSlot, float BagUsage)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("Bag update is server side"));
	}
	if (BagSlot == EBagSlot::Quiver)
	{
		//const UInventoryItemBag* Quiver = Cast<UInventoryItemBag>();
		//if (Quiver)
		{
			const UInventoryItemEquipable* Bag = GetItemAtSlot(EEquipmentSlot::Ammo);

			if (!Bag)
				return;

			const IInventoryItemAmmoBagInterface* QuiverInterface = Cast<IInventoryItemAmmoBagInterface>(Bag);
			if (!QuiverInterface)
			{
				UE_LOG(LogTemp, Warning, TEXT("No quiver within %d"), BagSlot);
				return;
			}
			if (QuiverInterface)
			{
				UE_LOG(LogTemp, Log, TEXT("Bag usage %f"), BagUsage);
				if (BagUsage == 0.f)
				{
					// hide the ammo of the quiver
					AmmoVariableComponent->SetStaticMesh(nullptr);
					AmmoVariableComponent->SetVisibility(false);
				}
				else if (BagUsage >= QuiverInterface->GetFullMeshThreshold())
				{
					AmmoVariableComponent->SetStaticMesh(QuiverInterface->GetFullMesh());
					AmmoVariableComponent->SetVisibility(true);
				}
				else if (BagUsage >= QuiverInterface->GetMidMeshThreshold())
				{
					AmmoVariableComponent->SetStaticMesh(QuiverInterface->GetMidMesh());
					AmmoVariableComponent->SetVisibility(true);
				}
				else
				{
					AmmoVariableComponent->SetStaticMesh(QuiverInterface->GetLowMesh());
					AmmoVariableComponent->SetVisibility(true);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipLightItem_Implementation(TSubclassOf<AInventoryLightSourceActor> LightActor) const
{
	if (LightActor)
	{
		ACharacter* Parent = Cast<ACharacter>(GetOwner());
		if (!Parent)
		{
			return;
		}

		USkeletalMeshComponent* PlayerMesh = Parent->GetMesh();

		if (!PlayerMesh)
		{
			return;
		}

		FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		SecondaryLightSource->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_LeftHandWeapon"));
		SecondaryLightSource->SetChildActorClass(LightActor);
		SecondaryLightSource->CreateChildActor();
	}
	else
	{
		SecondaryLightSource->DestroyChildActor();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UnEquipLightItem_Implementation() const
{
	SecondaryLightSource->DestroyChildActor();
}
