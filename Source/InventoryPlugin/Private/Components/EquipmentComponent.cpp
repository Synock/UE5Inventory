#include "Components/EquipmentComponent.h"
#include "InventoryPlugin.h"
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

namespace
{
	TSet<FString> LoggedMissingEquipmentSockets;

	bool AttachComponentToEquipmentSocketIfAvailable(USceneComponent* Component,
	                                                 USkeletalMeshComponent* PlayerMesh,
	                                                 FName SocketName,
	                                                 const AActor* Owner)
	{
		if (!Component || !PlayerMesh || !PlayerMesh->GetSkeletalMeshAsset())
		{
			return false;
		}

		if (!PlayerMesh->DoesSocketExist(SocketName))
		{
			const FString LogKey = FString::Printf(TEXT("%s:%s:%s"),
				Owner ? *Owner->GetClass()->GetPathName() : TEXT("UnknownOwner"),
				*PlayerMesh->GetSkeletalMeshAsset()->GetName(),
				*SocketName.ToString());
			if (!LoggedMissingEquipmentSockets.Contains(LogKey))
			{
				LoggedMissingEquipmentSockets.Add(LogKey);
				UE_LOG(LogInventoryPlugin, Verbose,
				       TEXT("%s cannot attach equipment component '%s' to mesh socket '%s' because mesh '%s' does not define it."),
				       Owner ? *Owner->GetName() : TEXT("UnknownOwner"),
				       *Component->GetName(),
				       *SocketName.ToString(),
				       *PlayerMesh->GetSkeletalMeshAsset()->GetName());
			}
			return false;
		}

		if (Component->GetAttachParent() == PlayerMesh && Component->GetAttachSocketName() == SocketName)
		{
			return true;
		}

		const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		return Component->AttachToComponent(PlayerMesh, AttachmentTransformRules, SocketName);
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
			EEquipmentSocket LiveSocket = EEquipmentSocket::Unknown;
			if (PossibleSocket == EEquipmentSocket::PrimarySheath)
			{
				LiveSocket = EEquipmentSocket::Primary;
			}
			else if (PossibleSocket == EEquipmentSocket::SecondarySheath)
			{
				LiveSocket = EEquipmentSocket::Secondary;
			}
			else if (PossibleSocket == EEquipmentSocket::BackSheath)
			{
				// BackSheath is used for shields (Secondary slot → SecondaryWeaponComponent)
				// and primary-routed weapons.  Use EquipSlot to match Unsheath() logic.
				LiveSocket = (EquipSlot == EEquipmentSlot::Secondary)
					? EEquipmentSocket::Secondary
					: EEquipmentSocket::Primary;
			}
			else if (PossibleSocket == EEquipmentSocket::RangedSheath)
			{
				LiveSocket = EEquipmentSocket::Primary;
			}

			if (LiveSocket != EEquipmentSocket::Unknown)
			{
				if (USkeletalMeshComponent* LiveSocketComponent = GetSkeletalMeshComponentFromSocket(LiveSocket))
				{
					// Always send the multicast so clients that have the weapon drawn in-hand
					// receive the clear command even when the server-local pointer is already null.
					UpdateEquipment(LiveSocketComponent, nullptr, {});
					LiveSocketComponent->SetSkeletalMeshAsset(nullptr);
					// Reset the tracked original slot so Sheath() doesn't try to return
					// a mesh that no longer exists.
					if (LiveSocket == EEquipmentSocket::Primary)
						PrimaryWeaponOriginalSlot = EEquipmentSocket::PrimarySheath;
					else if (LiveSocket == EEquipmentSocket::Secondary)
						SecondaryWeaponOriginalSlot = EEquipmentSocket::SecondarySheath;
				}
			}

			// Clear the sheath socket itself (may already be null on the server if the weapon
			// was unsheathed on the client, but the multicast still notifies all clients).
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
			UE_LOG(LogInventoryPlugin, Error, TEXT("Invalid return socket for primary weapon sheathing"));
			return;
		}

		// CRITICAL FIX: Don't overwrite if sheath already occupied - return early to prevent mesh loss
		if (ReturnSocket->GetSkeletalMeshAsset())
		{
			UE_LOG(LogInventoryPlugin, Error, TEXT("Primary sheath socket already occupied - cannot sheath (would lose mesh reference)"));
			return;
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
			UE_LOG(LogInventoryPlugin, Error, TEXT("Invalid return socket for secondary weapon sheathing"));
			return;
		}

		// CRITICAL FIX: Don't overwrite if sheath already occupied - return early to prevent mesh loss
		if (ReturnSocket->GetSkeletalMeshAsset())
		{
			UE_LOG(LogInventoryPlugin, Error, TEXT("Secondary sheath socket already occupied - cannot sheath (would lose mesh reference)"));
			return;
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

		// Only apply material overrides if we have a valid mesh
		if (LocalItem)
		{
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
		else if (MaterialOverride.Num() > 0)
		{
			UE_LOG(LogInventoryPlugin, Verbose, TEXT("UpdateEquipment: Skipping %d material overrides because mesh is null"), MaterialOverride.Num());
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
	// Seed the stale list from all currently managed slots.
	// Each slot present in the incoming set is removed; whatever remains is destroyed.
	TArray<EEquipmentSlot> SlotsToRemove;
	SlotsToRemove.Reserve(VariableMeshesMap.Num());
	for (auto&& [Slot, _] : VariableMeshesMap)
		SlotsToRemove.Add(Slot);

	for (auto&& [Slot, MeshPointer] : MeshArray)
	{
		SlotsToRemove.RemoveSingleSwap(Slot, EAllowShrinking::No);

		bool bMeshChanged = false;
		USkeletalMeshComponent* MeshComp;

		if (USkeletalMeshComponent** Existing = VariableMeshesMap.Find(Slot))
		{
			MeshComp = *Existing;
			if (MeshComp->GetSkeletalMeshAsset() != MeshPointer)
			{
				MeshComp->SetSkeletalMesh(MeshPointer);
				bMeshChanged = true;
			}
		}
		else
		{
			MeshComp = CreateAndRegisterOverlayComponent(Slot, MeshPointer);
			if (!MeshComp)
				continue;
			bMeshChanged = true;
		}

		// Only reapply overrides when the mesh actually changed.
		// Avoids allocating new UMaterialInstanceDynamic objects on every rebuild
		// when only an unrelated slot triggered UpdateMeshFromInternal.
		if (bMeshChanged)
		{
			if (const TArray<FMaterialOverride>* Overrides = OverrideArray.Find(Slot))
				ApplyMaterialOverrides(MeshComp, *Overrides);
		}
	}

	for (EEquipmentSlot SlotToRemove : SlotsToRemove)
	{
		if (USkeletalMeshComponent* Comp = VariableMeshesMap.FindRef(SlotToRemove))
		{
			Comp->UnregisterComponent();
			Comp->MarkAsGarbage();
		}
		VariableMeshesMap.Remove(SlotToRemove);
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

USkeletalMeshComponent* UEquipmentComponent::CreateAndRegisterOverlayComponent(EEquipmentSlot Slot,
	USkeletalMesh* Mesh)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!ensure(Owner))
		return nullptr;

	USkeletalMeshComponent* MasterMesh = Owner->GetMesh();
	if (!ensure(MasterMesh))
		return nullptr;

	USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(this);
	MeshComp->RegisterComponent();
	// Dynamic overlay components are not replicated directly.
	// The Equipment array replication + OnRep_ItemList drives their recreation on clients.
	MeshComp->SetSkeletalMesh(Mesh);
	MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn,   ECR_Ignore);

	const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget,
	                                               EAttachmentRule::SnapToTarget,
	                                               EAttachmentRule::SnapToTarget, true);
	MeshComp->AttachToComponent(MasterMesh, TransformRules);
	MeshComp->SetLeaderPoseComponent(MasterMesh);
	VariableMeshesMap.Emplace(Slot, MeshComp);
	return MeshComp;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::ApplyMaterialOverrides(USkeletalMeshComponent* MeshComp,
	const TArray<FMaterialOverride>& Overrides)
{
	for (const FMaterialOverride& Override : Overrides)
	{
		MeshComp->SetMaterial(Override.MaterialID, Override.OverrideMaterial);
		if (UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(Override.MaterialID))
		{
			DynMat->SetVectorParameterValue(TEXT("Tint"), Override.TintColor);
			DynMat->SetScalarParameterValue(TEXT("TintIntensity"), Override.TintIntensity);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UpdateSingleOverlayMesh(EEquipmentSlot Slot, USkeletalMesh* Mesh,
	const TArray<FMaterialOverride>& Overrides)
{
	if (!Mesh)
	{
		if (USkeletalMeshComponent* Comp = VariableMeshesMap.FindRef(Slot))
		{
			Comp->UnregisterComponent();
			Comp->MarkAsGarbage();
			VariableMeshesMap.Remove(Slot);
		}
		return;
	}

	bool bMeshChanged = false;
	USkeletalMeshComponent* MeshComp;

	if (USkeletalMeshComponent** Existing = VariableMeshesMap.Find(Slot))
	{
		MeshComp = *Existing;
		if (MeshComp->GetSkeletalMeshAsset() != Mesh)
		{
			MeshComp->SetSkeletalMesh(Mesh);
			bMeshChanged = true;
		}
	}
	else
	{
		MeshComp = CreateAndRegisterOverlayComponent(Slot, Mesh);
		if (!MeshComp)
			return;
		bMeshChanged = true;
	}

	if (bMeshChanged)
		ApplyMaterialOverrides(MeshComp, Overrides);
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentComponent::AttachEquipmentComponentsToOwnerMeshIfReady()
{
	ACharacter* Parent = Cast<ACharacter>(GetOwner());
	if (!Parent)
	{
		return false;
	}

	USkeletalMeshComponent* PlayerMesh = Parent->GetMesh();
	if (!PlayerMesh || !PlayerMesh->GetSkeletalMeshAsset())
	{
		return false;
	}

	if (!PrimaryWeaponComponent)
	{
		return false;
	}

	bool bAttachedAnyComponent = false;
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(PrimaryWeaponComponent, PlayerMesh, FName("SOCKET_RightHandWeapon"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(SecondaryWeaponComponent, PlayerMesh, FName("SOCKET_LeftHandWeapon"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(AmmoComponent, PlayerMesh, FName("SOCKET_AmmoBag"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(AmmoVariableComponent, PlayerMesh, FName("SOCKET_AmmoBag"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(PrimaryWeaponSheath, PlayerMesh, FName("PrimarySheath"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(SecondaryWeaponSheath, PlayerMesh, FName("SecondarySheath"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(BackWeaponSheath, PlayerMesh, FName("BackSheath"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(RangedWeaponSheath, PlayerMesh, FName("LowerBackSheath"), Parent);

	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(WaistBag1Component, PlayerMesh, FName("SOCKET_WaistBag1"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(WaistBag2Component, PlayerMesh, FName("SOCKET_WaistBag2"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(ShoulderBag1Component, PlayerMesh, FName("SOCKET_ShoulderBag1"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(ShoulderBag2Component, PlayerMesh, FName("SOCKET_ShoulderBag2"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(BackpackComponent, PlayerMesh, FName("SOCKET_Backpack"), Parent);

	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(RingLComponent, PlayerMesh, FName("RingL"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(RingRComponent, PlayerMesh, FName("RingR"), Parent);

	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(EarringLComponent, PlayerMesh, FName("EarR"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(EarringRComponent, PlayerMesh, FName("EarL"), Parent);

	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(WristLComponent, PlayerMesh, FName("WristL"), Parent);
	bAttachedAnyComponent |= AttachComponentToEquipmentSocketIfAvailable(WristRComponent, PlayerMesh, FName("WristR"), Parent);

	for (auto&& [MeshPointer, MeshComponent] : VariableMeshesMap)
	{
		if (!MeshComponent)
		{
			continue;
		}

		const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget,
		                                               EAttachmentRule::SnapToTarget,
		                                               EAttachmentRule::SnapToTarget, true);
		MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		if (MeshComponent->GetAttachParent() != PlayerMesh)
		{
			bAttachedAnyComponent |= MeshComponent->AttachToComponent(PlayerMesh, TransformRules);
		}
		MeshComponent->SetLeaderPoseComponent(PlayerMesh);
	}

	return bAttachedAnyComponent;
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

	if (!PrimaryWeaponComponent)
	{
		return;
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

	AttachEquipmentComponentsToOwnerMeshIfReady();

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
	// UpdateMeshFromInternal (via EquipmentDispatcher) handles the full rebuild including
	// TryUpdateDynamicMeshes, which creates cloth overlays for body-part slots and removes
	// stale overlays via its SlotsToRemove path.  It must run first so the overlay map is
	// in its final state before the per-slot refresh below.
	EquipmentDispatcher.Broadcast();

	// Weapon socket SKMs (Primary, Secondary, Ranged) are NOT managed by UpdateMeshFromInternal
	// (which only handles the merged body mesh).  If UEquipmentComponent::BeginPlay ran before
	// the initial Equipment replication arrived (the common NPC-spawn case), the socket
	// components will be empty even though the server sent the per-equip UpdateEquipment
	// multicast.  Re-sync them whenever the replicated Equipment array changes so weapon
	// meshes are always correct after any replication update.
	if (PrimaryWeaponComponent)
	{
		if (const UInventoryItemEquipable* PrimaryWeapon = GetItemAtSlot(EEquipmentSlot::Primary))
			Equip(PrimaryWeapon, EEquipmentSlot::Primary);
		else
		{
			PrimaryWeaponComponent->SetSkeletalMeshAsset(nullptr);
			if (PrimaryWeaponSheath)
				PrimaryWeaponSheath->SetSkeletalMeshAsset(nullptr);
		}
	}
	if (SecondaryWeaponComponent)
	{
		if (const UInventoryItemEquipable* SecWeapon = GetItemAtSlot(EEquipmentSlot::Secondary))
			Equip(SecWeapon, EEquipmentSlot::Secondary);
		else
		{
			SecondaryWeaponComponent->SetSkeletalMeshAsset(nullptr);
			if (SecondaryWeaponSheath)
				SecondaryWeaponSheath->SetSkeletalMeshAsset(nullptr);
			if (BackWeaponSheath)
				BackWeaponSheath->SetSkeletalMeshAsset(nullptr);
		}
	}
	if (RangedWeaponSheath)
	{
		if (const UInventoryItemEquipable* RangedWeapon = GetItemAtSlot(EEquipmentSlot::Range))
			Equip(RangedWeapon, EEquipmentSlot::Range);
		else
			RangedWeaponSheath->SetSkeletalMeshAsset(nullptr);
	}

	// Refresh only overlay-eligible slots (those for which GetEquipmentOverlayMesh returns a
	// non-null mesh, e.g. Shoulders, Neck, Back, Face, Wrists).
	// Body-part slots (Torso, Legs, etc.) return nullptr here and are managed exclusively by
	// TryUpdateDynamicMeshes inside UpdateMeshFromInternal above.
	// Calling UpdateSingleOverlayMesh(nullptr) for those slots would immediately destroy any
	// cloth overlay just created by that path.
	if (IInventoryModularCharacterInterface* ModularChar = Cast<IInventoryModularCharacterInterface>(GetOwner()))
	{
		for (int32 SlotIdx = 0; SlotIdx < static_cast<int32>(EEquipmentSlot::Last); ++SlotIdx)
		{
			const EEquipmentSlot Slot = static_cast<EEquipmentSlot>(SlotIdx);
			const UInventoryItemEquipable* Item = Equipment[SlotIdx];
			USkeletalMesh* OverlayMesh = ModularChar->GetEquipmentOverlayMesh(Slot, Item);
			if (OverlayMesh)
			{
				UpdateSingleOverlayMesh(Slot, OverlayMesh,
					Item ? Item->EquipmentMeshMaterialOverride : TArray<FMaterialOverride>{});
			}
		}
	}
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
	if (CanEquipItemAt(Item, InSlot))
	{
		Equipment[static_cast<int>(InSlot)] = Item;

		// Initialize with default durability from item definition
		int32 SlotIndex = static_cast<int>(InSlot);
		if (EquipmentDurability.Num() <= SlotIndex)
		{
			EquipmentDurability.SetNum(Equipment.Num());
		}

		EquipmentDurability[SlotIndex] = Item->GetTotalDurability();

		Equip(Item, InSlot);
		EquipmentDispatcher_Server.Broadcast();
		ItemEquipedDispatcher_Server.Broadcast(InSlot, Item);

		// Plugin default: immediately update overlay for this slot
		if (IInventoryModularCharacterInterface* ModularChar = Cast<IInventoryModularCharacterInterface>(GetOwner()))
		{
			if (USkeletalMesh* OverlayMesh = ModularChar->GetEquipmentOverlayMesh(InSlot, Item))
				UpdateSingleOverlayMesh(InSlot, OverlayMesh, Item->EquipmentMeshMaterialOverride);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::EquipItemWithDurability(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot, float Durability)
{
	if (CanEquipItemAt(Item, InSlot))
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

		// Plugin default: immediately update overlay for this slot
		if (IInventoryModularCharacterInterface* ModularChar = Cast<IInventoryModularCharacterInterface>(GetOwner()))
		{
			if (USkeletalMesh* OverlayMesh = ModularChar->GetEquipmentOverlayMesh(InSlot, Item))
				UpdateSingleOverlayMesh(InSlot, OverlayMesh, Item->EquipmentMeshMaterialOverride);
		}
	}
}

bool UEquipmentComponent::IsEquipmentSlotReserved(EEquipmentSlot InSlot, const FGuid& IgnoredReservation) const
{
	for (const TPair<FGuid, TArray<EEquipmentSlot>>& Reservation : PendingDeliveryReservations)
	{
		if (Reservation.Key != IgnoredReservation && Reservation.Value.Contains(InSlot))
			return true;
	}
	return false;
}

bool UEquipmentComponent::CanEquipItemAt(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot,
	const FGuid& IgnoredReservation) const
{
	const int32 SlotIndex = static_cast<int32>(InSlot);
	if (!Item || InSlot == EEquipmentSlot::Unknown || InSlot >= EEquipmentSlot::Last ||
		!Equipment.IsValidIndex(SlotIndex))
		return false;

	const uint32 TargetBit = 1u << static_cast<uint32>(SlotIndex);
	if ((static_cast<uint32>(Item->EquipableSlotBitMask) & TargetBit) == 0)
		return false;
	if (Item->MultiSlotItem && (InSlot == EEquipmentSlot::WaistBag2 || InSlot == EEquipmentSlot::BackPack2))
		return false;

	TArray<EEquipmentSlot> RequiredSlots{InSlot};
	if (Item->MultiSlotItem)
	{
		for (int32 Index = static_cast<int32>(EEquipmentSlot::Unknown) + 1;
			Index < static_cast<int32>(EEquipmentSlot::Last); ++Index)
		{
			if ((static_cast<uint32>(Item->EquipableSlotBitMask) & (1u << static_cast<uint32>(Index))) != 0)
				RequiredSlots.AddUnique(static_cast<EEquipmentSlot>(Index));
		}
	}

	for (const EEquipmentSlot RequiredSlot : RequiredSlots)
	{
		const int32 RequiredIndex = static_cast<int32>(RequiredSlot);
		if (!Equipment.IsValidIndex(RequiredIndex) || Equipment[RequiredIndex] != nullptr ||
			IsEquipmentSlotReserved(RequiredSlot, IgnoredReservation))
			return false;
	}
	return true;
}

bool UEquipmentComponent::ReservePendingDelivery(const FGuid& DeliveryId, const UInventoryItemEquipable* Item,
	EEquipmentSlot InSlot)
{
	if (!DeliveryId.IsValid() || PendingDeliveryReservations.Contains(DeliveryId) ||
		!CanEquipItemAt(Item, InSlot, DeliveryId))
		return false;

	TArray<EEquipmentSlot>& Slots = PendingDeliveryReservations.Add(DeliveryId);
	Slots.Add(InSlot);
	if (Item->MultiSlotItem)
	{
		for (int32 Index = static_cast<int32>(EEquipmentSlot::Unknown) + 1;
			Index < static_cast<int32>(EEquipmentSlot::Last); ++Index)
		{
			if ((static_cast<uint32>(Item->EquipableSlotBitMask) & (1u << static_cast<uint32>(Index))) != 0)
				Slots.AddUnique(static_cast<EEquipmentSlot>(Index));
		}
	}
	return true;
}

void UEquipmentComponent::ReleasePendingDeliveryReservation(const FGuid& DeliveryId)
{
	PendingDeliveryReservations.Remove(DeliveryId);
}

bool UEquipmentComponent::HasPendingDeliveryReservation(const FGuid& DeliveryId) const
{
	return PendingDeliveryReservations.Contains(DeliveryId);
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
		if (!Item || Item->GetTotalDurability() <= 0.0f)
		{
			return;
		}

		float OldDurability = EquipmentDurability[SlotIndex];
		float NewDurability = FMath::Max(0.0f, OldDurability - DurabilityReduction);
		EquipmentDurability[SlotIndex] = NewDurability;

		// Calculate durability percentages
		float OldPercent = (OldDurability / Item->GetTotalDurability()) * 100.0f;
		float NewPercent = (NewDurability / Item->GetTotalDurability()) * 100.0f;

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

	const UInventoryItemEquipable* RemovedItem = Equipment[static_cast<int>(InSlot)];

	UnEquip(RemovedItem, InSlot);

	// Null the slot BEFORE broadcasting so any subscriber (e.g. ALootableCorpse)
	// that calls UpdateMeshFromInternal() sees the slot as already empty and
	// rebuilds the merged mesh without the removed item.
	Equipment[static_cast<int>(InSlot)] = nullptr;

	ItemUnEquipedDispatcher_Server.Broadcast(InSlot, RemovedItem);
	EquipmentDispatcher_Server.Broadcast();

	// Remove any overlay component for this slot (no-op if none existed)
	UpdateSingleOverlayMesh(InSlot, nullptr, {});
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
	for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown) + 1;
		i < static_cast<int32>(EEquipmentSlot::Last); ++i)
	{
		const EEquipmentSlot CurrentSlot = static_cast<EEquipmentSlot>(i);
		if (CanEquipItemAt(Item, CurrentSlot))
			return CurrentSlot;
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
	if (PrimaryWeaponComponent
		&& IsHoldingATwoHandedWeapon
		&& PrimaryWeaponComponent->GetSkeletalMeshAsset()
		&& PrimaryWeaponComponent->DoesSocketExist(FName("SOCKET_LeftHandPosition")))
	{
		return PrimaryWeaponComponent->GetSocketTransform(FName("SOCKET_LeftHandPosition"),
		                                                  ERelativeTransformSpace::RTS_World);
	}

	// No valid socket — caller should treat Identity as "IK disabled".
	return FTransform::Identity;
}

//----------------------------------------------------------------------------------------------------------------------

FVector UEquipmentComponent::GetWeaponTipLocation() const
{
	// Primary weapon mesh owns the WeaponTip socket — the weapon component tracks hand_r
	// automatically (SnapToTarget attachment to SOCKET_RightHandWeapon on the character mesh).
	// Fall back to the character's right-hand socket if the weapon has no WeaponTip defined.
	if (PrimaryWeaponComponent && PrimaryWeaponComponent->GetSkeletalMeshAsset())
	{
		if (PrimaryWeaponComponent->DoesSocketExist(FName("WeaponTip")))
			return PrimaryWeaponComponent->GetSocketLocation(FName("WeaponTip"));

		// No WeaponTip socket on this weapon mesh — use the attachment root (grip) as fallback
		return PrimaryWeaponComponent->GetComponentLocation();
	}

	// No weapon drawn — fall back to the character's right-hand socket
	if (const ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		if (const USkeletalMeshComponent* Mesh = Owner->GetMesh();
			Mesh && Mesh->GetSkeletalMeshAsset() && Mesh->DoesSocketExist(FName("SOCKET_RightHandWeapon")))
		{
			return Mesh->GetSocketLocation(FName("SOCKET_RightHandWeapon"));
		}
	}

	return FVector::ZeroVector;
}

//----------------------------------------------------------------------------------------------------------------------

FVector UEquipmentComponent::GetDefenseContactLocation() const
{
	// Defender contact point:
	//   - Shield equipped  → ShieldCenter socket on SecondaryWeaponComponent (shield mesh)
	//   - Off-hand weapon  → WeaponTip socket on SecondaryWeaponComponent (parrying weapon)
	//   - Nothing equipped → SOCKET_LeftHandWeapon bone on the character mesh
	if (SecondaryWeaponComponent && SecondaryWeaponComponent->GetSkeletalMeshAsset())
	{
		// Prefer ShieldCenter (defined on shields), then WeaponTip (defined on weapons)
		if (SecondaryWeaponComponent->DoesSocketExist(FName("ShieldCenter")))
			return SecondaryWeaponComponent->GetSocketLocation(FName("ShieldCenter"));

		if (SecondaryWeaponComponent->DoesSocketExist(FName("WeaponTip")))
			return SecondaryWeaponComponent->GetSocketLocation(FName("WeaponTip"));

		return SecondaryWeaponComponent->GetComponentLocation();
	}

	// Nothing in the off-hand — use the left-hand socket as best approximation
	if (const ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		if (const USkeletalMeshComponent* Mesh = Owner->GetMesh();
			Mesh && Mesh->GetSkeletalMeshAsset() && Mesh->DoesSocketExist(FName("SOCKET_LeftHandWeapon")))
		{
			return Mesh->GetSocketLocation(FName("SOCKET_LeftHandWeapon"));
		}
	}

	return FVector::ZeroVector;
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::UpdateBagUsage(EBagSlot BagSlot, float BagUsage)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UE_LOG(LogInventoryPlugin, Verbose, TEXT("UpdateBagUsage: server-side, slot %d"), static_cast<int32>(BagSlot));
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
				UE_LOG(LogInventoryPlugin, Warning, TEXT("UpdateBagUsage: no quiver interface on item at Ammo slot (BagSlot=%d)"), static_cast<int32>(BagSlot));
				return;
			}
			if (QuiverInterface)
			{
				UE_LOG(LogInventoryPlugin, Verbose, TEXT("UpdateBagUsage: usage=%.3f for slot %d"), BagUsage, static_cast<int32>(BagSlot));
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

UStaticMeshComponent* UEquipmentComponent::GetStaticMeshComponentForSlot(EEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EEquipmentSlot::WaistBag1:  return WaistBag1Component;
	case EEquipmentSlot::WaistBag2:  return WaistBag2Component;
	case EEquipmentSlot::BackPack1:  return ShoulderBag1Component;
	case EEquipmentSlot::BackPack2:  return ShoulderBag2Component;
	default:                         return nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentComponent::ApplyRigidItemFitting(EEquipmentSlot Slot, FName BoneName, FVector LocationOffset, FVector Scale)
{
	UStaticMeshComponent* Comp = GetStaticMeshComponentForSlot(Slot);
	if (!Comp)
		return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
		return;

	USkeletalMeshComponent* OwnerMesh = Owner->GetMesh();
	if (!OwnerMesh)
		return;

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
	                                EAttachmentRule::SnapToTarget, true);
	Comp->AttachToComponent(OwnerMesh, Rules, BoneName);
	Comp->SetRelativeLocation(LocationOffset);
	Comp->SetRelativeScale3D(Scale);
}

USkeletalMeshComponent* UEquipmentComponent::GetOverlayComponentForSlot(EEquipmentSlot Slot) const
{
	if (const auto* Found = VariableMeshesMap.Find(Slot))
		return *Found;
	return nullptr;
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
