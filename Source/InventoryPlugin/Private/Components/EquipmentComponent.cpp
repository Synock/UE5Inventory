// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/EquipmentComponent.h"
#include <Net/UnrealNetwork.h>

#include "Actors/InventoryLightSourceActor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/InventoryModularCharacterInterface.h"
#include "Items/InventoryItemEquipable.h"

namespace
{
	UStaticMesh* LoadStaticMeshFromPath(const FName& Path)
	{
		if (Path == NAME_None)
		{
			return nullptr;
		}

		return Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *Path.ToString()));
	}
}

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
	case EEquipmentSocket::PrimarySheath: return PrimaryWeaponSheath;
	case EEquipmentSocket::SecondarySheath: return SecondaryWeaponSheath;
	case EEquipmentSocket::BackSheath: return BackWeaponSheath;
	case EEquipmentSocket::Unknown: return nullptr;
	case EEquipmentSocket::Primary: return PrimaryWeaponComponent;
	case EEquipmentSocket::Secondary: return SecondaryWeaponComponent;

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
	case EEquipmentSocket::Primary: return PrimaryWeaponComponentSkeletal;
	case EEquipmentSocket::Secondary: return SecondaryWeaponComponentSkeletal;
	case EEquipmentSocket::Head:
		{
			return Cast<IInventoryModularCharacterInterface>(GetOwner())->GetHelmetComponent();
		}
	case EEquipmentSocket::WristL: return LeftBracerComponent;
	case EEquipmentSocket::WristR: return RightBracerComponent;

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

	if (!WantedSocket->GetStaticMesh() || !WantedSocket->IsVisible())
	{
		if (PrimaryWeaponOriginalSlot == PossibleSocket)
		{
			return PrimaryWeaponComponent->SetStaticMesh(nullptr);
		}
		if (SecondaryWeaponOriginalSlot == PossibleSocket)
		{
			return SecondaryWeaponComponent->SetStaticMesh(nullptr);
		}
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
			if (!Item->Weapon)
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
	case EEquipmentSlot::Range: return EEquipmentSocket::BackSheath;
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
	{
		return;
	}

	const EEquipmentSocket SheathSocket = FindBestSocketForItem(Item, SlotToUnsheath);

	if (SheathSocket != EEquipmentSocket::PrimarySheath && SheathSocket != EEquipmentSocket::SecondarySheath &&
		SheathSocket != EEquipmentSocket::BackSheath)
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

	UStaticMeshComponent* LiveSocketComponent = GetMeshComponentFromSocket(LiveSocket);
	UStaticMeshComponent* SheathSocketComponent = GetMeshComponentFromSocket(SheathSocket);

	if (SheathSocketComponent->GetStaticMesh() == nullptr || LiveSocketComponent->GetStaticMesh() != nullptr)
	{
		return;
	}

	UStaticMesh* MeshPointer = SheathSocketComponent->GetStaticMesh();
	SheathSocketComponent->SetStaticMesh(nullptr);
	LiveSocketComponent->SetStaticMesh(MeshPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::Sheath()
{
	if (PrimaryWeaponComponent->GetStaticMesh() != nullptr)
	{
		UStaticMeshComponent* ReturnSocket = GetMeshComponentFromSocket(PrimaryWeaponOriginalSlot);

		if (!ReturnSocket || ReturnSocket->GetStaticMesh())
		{
			return;
		}

		UStaticMesh* MeshPointer = PrimaryWeaponComponent->GetStaticMesh();
		PrimaryWeaponComponent->SetStaticMesh(nullptr);
		ReturnSocket->SetStaticMesh(MeshPointer);
	}

	if (SecondaryWeaponComponent->GetStaticMesh() != nullptr)
	{
		UStaticMeshComponent* ReturnSocket = GetMeshComponentFromSocket(SecondaryWeaponOriginalSlot);

		if (!ReturnSocket || ReturnSocket->GetStaticMesh())
		{
			return;
		}

		UStaticMesh* MeshPointer = SecondaryWeaponComponent->GetStaticMesh();
		SecondaryWeaponComponent->SetStaticMesh(nullptr);
		ReturnSocket->SetStaticMesh(MeshPointer);
	}
	/*
		if (PrimaryWeaponComponentSkeletal->GetSkeletalMeshAsset() != nullptr)
		{
			USkeletalMeshComponent* ReturnSocket = GetSkeletalMeshComponentFromSocket(PrimaryWeaponOriginalSlot);

			if (!ReturnSocket || ReturnSocket->GetSkeletalMeshAsset())
				return;

			USkeletalMesh* MeshPointer = PrimaryWeaponComponentSkeletal->GetSkeletalMeshAsset();
			PrimaryWeaponComponentSkeletal->SetSkeletalMeshAsset(nullptr);
			ReturnSocket->SetSkeletalMeshAsset(MeshPointer);
		}

		if (SecondaryWeaponComponentSkeletal->GetSkeletalMeshAsset() != nullptr)
		{
			USkeletalMeshComponent* ReturnSocket = GetSkeletalMeshComponentFromSocket(SecondaryWeaponOriginalSlot);

			if (!ReturnSocket || ReturnSocket->GetSkeletalMeshAsset())
				return;

			USkeletalMesh* MeshPointer = SecondaryWeaponComponentSkeletal->GetSkeletalMeshAsset();
			SecondaryWeaponComponentSkeletal->SetSkeletalMeshAsset(nullptr);
			ReturnSocket->SetSkeletalMeshAsset(MeshPointer);
		}*/
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
			if (UMaterialInstanceDynamic* DynMat = SkeletalSocket->CreateAndSetMaterialInstanceDynamic(Material.MaterialID))
			{
				DynMat->SetVectorParameterValue(TEXT("Tint"), Material.TintColor);
				DynMat->SetScalarParameterValue(TEXT("TintIntensity"), Material.TintIntensity);
			}
		}

	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Equipment.Init(nullptr, 32);

	PrimaryWeaponComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryWeaponComponentMesh"));
	PrimaryWeaponComponent->SetIsReplicated(true);
	SecondaryWeaponComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryWeaponComponentMesh"));
	SecondaryWeaponComponent->SetIsReplicated(true);

	PrimaryWeaponComponentSkeletal = CreateDefaultSubobject<USkeletalMeshComponent>(
		TEXT("PrimaryWeaponComponentSkeletalMesh"));
	PrimaryWeaponComponentSkeletal->SetIsReplicated(true);
	SecondaryWeaponComponentSkeletal = CreateDefaultSubobject<USkeletalMeshComponent>(
		TEXT("SecondaryWeaponComponentSkeletalMesh"));
	SecondaryWeaponComponentSkeletal->SetIsReplicated(true);

	AmmoComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoComponentMesh"));
	AmmoComponent->SetIsReplicated(true);
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

	PrimaryWeaponSheath = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryWeaponSheathComponentMesh"));
	PrimaryWeaponSheath->SetIsReplicated(true);
	SecondaryWeaponSheath = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryWeaponSheathComponentMesh"));
	SecondaryWeaponSheath->SetIsReplicated(true);
	BackWeaponSheath = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackWeaponSheathComponentMesh"));
	BackWeaponSheath->SetIsReplicated(true);

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

	LeftBracerComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BracerLComponent"));
	LeftBracerComponent->SetIsReplicated(true);
	RightBracerComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BracerRComponent"));
	RightBracerComponent->SetIsReplicated(true);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UpdateMasterMeshComponent(USkeletalMeshComponent* Mesh)
{
	RightBracerComponent->SetLeaderPoseComponent(Mesh);
	LeftBracerComponent->SetLeaderPoseComponent(Mesh);
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

	PrimaryWeaponComponentSkeletal->AttachToComponent(PlayerMesh, AttachmentTransformRules,
	                                                  FName("SOCKET_RightHandWeapon"));
	SecondaryWeaponComponentSkeletal->AttachToComponent(PlayerMesh, AttachmentTransformRules,
	                                                    FName("SOCKET_LeftHandWeapon"));

	WaistBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_WaistBag1"));
	WaistBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_WaistBag2"));
	ShoulderBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_ShoulderBag1"));
	ShoulderBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_ShoulderBag2"));
	BackpackComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_Backpack"));

	PrimaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("PrimarySheath"));
	SecondaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SecondarySheath"));
	BackWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("BackSheath"));

	RingLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("RingL"));
	RingRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("RingR"));

	EarringLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("EarR"));
	EarringRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("EarL"));

	WristLComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("WristL"));
	WristRComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("WristR"));

	FAttachmentTransformRules TransformRules2(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
	                                          EAttachmentRule::SnapToTarget, true);
	LeftBracerComponent->AttachToComponent(PlayerMesh, TransformRules2, FName("root"));
	LeftBracerComponent->SetLeaderPoseComponent(Cast<ACharacter>(GetOwner())->GetMesh());

	RightBracerComponent->AttachToComponent(PlayerMesh, TransformRules2, FName("root"));
	RightBracerComponent->SetLeaderPoseComponent(Cast<ACharacter>(GetOwner())->GetMesh());
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(UEquipmentComponent, Equipment);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::OnRep_ItemList()
{
	EquipmentDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot)
{
	if (Equipment[static_cast<int>(InSlot)] == nullptr)
	{
		Equipment[static_cast<int>(InSlot)] = Item;
		Equip(Item, InSlot);
		EquipmentDispatcher_Server.Broadcast();
		ItemEquipedDispatcher_Server.Broadcast(InSlot, Item);
	}
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

FBoxSphereBounds UEquipmentComponent::GetEquipmentOverlapBox(EEquipmentSlot Slot) const
{
	UStaticMeshComponent* Component = nullptr;

	switch (Slot)
	{
	case EEquipmentSlot::Primary: Component = PrimaryWeaponComponent;
		break;
	case EEquipmentSlot::Secondary: Component = SecondaryWeaponComponent;
		break;
	default: return {};
	}

	if (!IsValid(Component->GetStaticMesh()))
	{
		return {};
	}

	FBoxSphereBounds MeshBounds = Component->GetStaticMesh()->GetBounds();

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

void UEquipmentComponent::UnEquipLightItem_Implementation() const
{
	SecondaryLightSource->DestroyChildActor();
}
