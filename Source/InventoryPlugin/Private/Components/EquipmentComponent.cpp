// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/EquipmentComponent.h"
#include <Net/UnrealNetwork.h>
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Items/InventoryItemEquipable.h"

namespace
{
	UStaticMesh* LoadStaticMeshFromPath(const FName& Path)
	{
		if (Path == NAME_None)
			return nullptr;

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
	default: return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::Equip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot)
{
	const EEquipmentSocket PossibleSocket = FindBestSocketForItem(Item, EquipSlot);

	if (PossibleSocket == EEquipmentSocket::Unknown)
		return false;

	UStaticMeshComponent* WantedSocket = GetMeshComponentFromSocket(PossibleSocket);

	if (!WantedSocket)
		return false;

	UStaticMesh* StaticMesh = Item->Mesh;
	if (!StaticMesh)
		return false;

	return WantedSocket->SetStaticMesh(StaticMesh);
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::UnEquip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot)
{
	const EEquipmentSocket PossibleSocket = FindBestSocketForItem(Item, EquipSlot);

	if (PossibleSocket == EEquipmentSocket::Unknown)
		return false;

	UStaticMesh* StaticMesh = Item->Mesh;

	if (!StaticMesh)
		return false;

	UStaticMeshComponent* WantedSocket = GetMeshComponentFromSocket(PossibleSocket);

	if (!WantedSocket)
		return false;

	return WantedSocket->SetStaticMesh(nullptr);
}

//----------------------------------------------------------------------------------------------------------------------

EEquipmentSocket UEquipmentComponent::FindBestSocketForItem(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot)
{
	switch (EquipSlot)
	{
	case EEquipmentSlot::Primary:
		{
			if (!Item->Weapon)
				return EEquipmentSocket::Primary;

			return EEquipmentSocket::PrimarySheath;
		}
	case EEquipmentSlot::Secondary:
		{
			if (!Item->Weapon)
				return EEquipmentSocket::Secondary;

			return EEquipmentSocket::SecondarySheath;
		}
	case EEquipmentSlot::Range: return EEquipmentSocket::BackSheath;
	case EEquipmentSlot::Ammo: return EEquipmentSocket::AmmoBag;
	case EEquipmentSlot::Head: break;
	case EEquipmentSlot::Face: break;
	case EEquipmentSlot::Shoulders: break;
	case EEquipmentSlot::Back: break;
	case EEquipmentSlot::Waist: break;
	case EEquipmentSlot::WaistBag1: return EEquipmentSocket::WaistBag1;
	case EEquipmentSlot::WaistBag2: return EEquipmentSocket::WaistBag2;
	case EEquipmentSlot::BackPack1:
		{
			if (Item->TwoSlotsItem)
				return EEquipmentSocket::Backpack;
			return EEquipmentSocket::ShoulderBag1;
		}
	case EEquipmentSlot::BackPack2:
		{
			if (Item->TwoSlotsItem)
				return EEquipmentSocket::Backpack;
			return EEquipmentSocket::ShoulderBag2;
		}
	default: ;
	}

	return EEquipmentSocket::Unknown;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::Unsheath(EEquipmentSlot SlotToUnsheath)
{
	const UInventoryItemEquipable* Item = Equipment[static_cast<int>(SlotToUnsheath)];
	const EEquipmentSocket SheathSocket = FindBestSocketForItem(Item, SlotToUnsheath);

	if (SheathSocket != EEquipmentSocket::PrimarySheath && SheathSocket != EEquipmentSocket::SecondarySheath &&
		SheathSocket != EEquipmentSocket::BackSheath)
		return;

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
				LiveSocket = EEquipmentSocket::Primary;
			else if (SlotToUnsheath == EEquipmentSlot::Secondary)
				LiveSocket = EEquipmentSocket::Secondary;
			else if (SlotToUnsheath == EEquipmentSlot::Range)
				LiveSocket = EEquipmentSocket::Primary;
		}
		break;
	default:
		return;
	}

	if (LiveSocket == EEquipmentSocket::Primary)
		PrimaryWeaponOriginalSlot = SheathSocket;

	else if (LiveSocket == EEquipmentSocket::Secondary)
		SecondaryWeaponOriginalSlot = SheathSocket;

	UStaticMeshComponent* LiveSocketComponent = GetMeshComponentFromSocket(LiveSocket);
	UStaticMeshComponent* SheathSocketComponent = GetMeshComponentFromSocket(SheathSocket);

	if (SheathSocketComponent->GetStaticMesh() == nullptr || LiveSocketComponent->GetStaticMesh() != nullptr)
		return;

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

		if (!ReturnSocket || !ReturnSocket->GetStaticMesh())
			return;

		UStaticMesh* MeshPointer = PrimaryWeaponComponent->GetStaticMesh();
		PrimaryWeaponComponent->SetStaticMesh(nullptr);
		ReturnSocket->SetStaticMesh(MeshPointer);
	}

	if (SecondaryWeaponComponent->GetStaticMesh() != nullptr)
	{
		UStaticMeshComponent* ReturnSocket = GetMeshComponentFromSocket(SecondaryWeaponOriginalSlot);

		if (!ReturnSocket || !ReturnSocket->GetStaticMesh())
			return;

		UStaticMesh* MeshPointer = SecondaryWeaponComponent->GetStaticMesh();
		SecondaryWeaponComponent->SetStaticMesh(nullptr);
		ReturnSocket->SetStaticMesh(MeshPointer);
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
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwnerRole() == ROLE_Authority)
		return;

	ACharacter* Parent = Cast<ACharacter>(GetOwner());
    	if (!Parent)
    		return;
	USkeletalMeshComponent* PlayerMesh = Parent->GetMesh();

	if(!PlayerMesh)
		return;

	if(!PrimaryWeaponComponent)
		return;

	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget,true);
	PrimaryWeaponComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules, FName("SOCKET_RightHandWeapon"));
	SecondaryWeaponComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_LeftHandWeapon"));
	AmmoComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_AmmoBag"));

	WaistBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_WaistBag1"));
	WaistBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_WaistBag2"));
	ShoulderBag1Component->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_ShoulderBag1"));
	ShoulderBag2Component->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_ShoulderBag2"));
	BackpackComponent->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_Backpack"));

	PrimaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_PrimaryScabbard"));
	SecondaryWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_SecondaryScabbard"));
	BackWeaponSheath->AttachToComponent(PlayerMesh, AttachmentTransformRules,  FName("SOCKET_BackWeaponScabbard"));
	// ...
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
		return true;

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

float UEquipmentComponent::GetTotalWeight() const
{
	float TotalWeight = 0.f;
	for (const auto& Item : Equipment)
	{
		if (Item)
			TotalWeight += Item->Weight;
	}
	return TotalWeight;
}

//----------------------------------------------------------------------------------------------------------------------

EEquipmentSlot UEquipmentComponent::FindSuitableSlot(const UInventoryItemEquipable* Item) const
{
	for (size_t i = 1; i < Equipment.Num(); ++i)
	{
		if (!Equipment[i])
		{
			const int32 LocalAcceptableBitMask = std::pow(2., static_cast<double>(i));
			const EEquipmentSlot CurrentSlot = static_cast<EEquipmentSlot>(i);
			if (Item->EquipableSlotBitMask & LocalAcceptableBitMask)
			{
				if (Item->TwoSlotsItem)
				{
					if (CurrentSlot == EEquipmentSlot::WaistBag1 || CurrentSlot == EEquipmentSlot::BackPack1)
						return CurrentSlot;
				}
				else
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
		return {};

	FBoxSphereBounds MeshBounds = Component->GetStaticMesh()->GetBounds();

	return MeshBounds;
}
