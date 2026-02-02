#include "Interfaces/FieldRepairInterface.h"

//----------------------------------------------------------------------------------------------------------------------
// Field Repair wrappers (default implementations)
//----------------------------------------------------------------------------------------------------------------------

void IFieldRepairInterface::BeginFieldRepair(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
												 int32 RepairKitTopLeft, EEquipmentSlot TargetEquipmentSlot,
												 EBagSlot TargetBagSlot,
												 int32 TargetTopLeft, int32 TargetItemID)
{
	Server_BeginFieldRepair(RepairKitItemID, RepairKitBagSlot, RepairKitTopLeft, TargetEquipmentSlot, TargetBagSlot,
	TargetTopLeft, TargetItemID);
}
//----------------------------------------------------------------------------------------------------------------------

void IFieldRepairInterface::CancelFieldRepair()
{
	Server_CancelFieldRepair();
}

