#pragma once

#include "Components/TradeComponent.h"
#include "InventoryDelivery.h"

namespace InventoryPlugin::TradeReturn
{
template <typename DeliveryFunction>
bool Route(const FTradeItemSlot& ItemSlot, DeliveryFunction&& Deliver)
{
	FInventoryDeliveryRequest Request;
	Request.ItemID = ItemSlot.ItemID;
	Request.Durability = ItemSlot.Durability;
	Request.Reason = EInventoryDeliveryReason::TradeReturn;
	// The source may have disappeared with an unequipped/dropped bag. It is a preference,
	// never proof that the destination is still valid.
	Request.PreferredBag = ItemSlot.SourceBagSlot;
	Request.PreferredTopLeft = ItemSlot.SourceTopLeft;
	return Deliver(Request) != EInventoryDeliveryOutcome::Rejected;
}
}
