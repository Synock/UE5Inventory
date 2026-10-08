#pragma once

#include "InventoryEscrow.h"
#include "InventoryDelivery.h"

namespace InventoryPlugin::StagingReturn
{
template <typename DeliveryCallable>
bool Route(const FInventoryEscrowItem& StagedItem, DeliveryCallable&& Deliver)
{
	FInventoryDeliveryRequest Request;
	Request.ItemID = StagedItem.ItemID;
	Request.Durability = StagedItem.Durability;
	Request.Reason = EInventoryDeliveryReason::StagingReturn;
	Request.bAllowAutoEquip = true;
	return Deliver(Request) != EInventoryDeliveryOutcome::Rejected;
}
}
