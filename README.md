# Unreal Engine 5 Replicated Inventory Plugin

A grid-based, fully replicated inventory and equipment plugin for Unreal Engine 5. It provides the core systems in C++ and exposes focused extension points for game-specific rules, UI, and presentation.

You will find find the following feature included:
* Grid based
* Replicated
* Extensible bag system
* Equipment system
* Weight management
* Monetary system
* Loot system
* Merchant system
* Banking system
* Trading system (player-to-player)
* Repair system (NPC and field repair)
* Durability system for equipment
* Skeletal mesh equipment support

The plugin is intended to be extended for your game's item definitions, visuals, and gameplay rules. Fork it or subclass the provided C++ extension points as appropriate for your project.

## Documentation and integration

Start with the maintained local documentation:

- [Integration Guide](./Docs/Integration_Guide.md) — installation, required game-side bridges, UI wiring, replication, and troubleshooting.
- [Interface Implementation Guide](./Docs/Interface_Implementation_Guide.md) — required versus optional interface contracts.
- [Component Architecture Guide](./Docs/Component_Architecture_Guide.md) — component responsibilities and extension points.
- [Item System Guide](./Docs/Item_System_Guide.md) — creating and configuring item definitions.
- [Replication System Guide](./Docs/Replication_System_Guide.md) — multiplayer synchronization details.

The plugin's `UInventoryNetComponent` owns the inventory Server RPCs, so a consuming project does not need to recreate the previous set of RPC declarations and implementation stubs. The Integration Guide walks through the remaining game-side setup: item registry, PlayerController/Character components, and HUD window registration.

For an older complete-project reference, see [UE5PluginIntegration](https://github.com/Synock/UE5PluginIntegration). Prefer the local guides above for the current API.

![Inventory Example](./Images/InventoryExample.png?raw=true "InventoryExample")

## Foreword and philosophy

This project was developed as a dual purpose: Learning unreal engine and having a functional inventory system for my project.
As such some specific features and behavior are coded in a way that may not suit every usage.
For instance, items definition is performed only through the server, but an alternative system should not be too difficult to be introduced.
Most classes introduced are designed to be easily subclassed and suited to most of the needs.

Because I (still) have no clue on how to code with unreal engine, some code/design and choice may be absurd, bug-prone or ineffective.

I will maintain this project and update it if I make any core change affecting inventory in my main project.


## Inventory & Equipment system

The grid based inventory system is based on the example tutorial from https://www.youtube.com/watch?v=4CjpBoKl6s8

It has been adapted and modified to handle replication and be written as much as possible in c++.
Some aspects of the original inventory were dropped as they were not needed due to design choices (e.g. objects rotation).

The proposed equipment system is simple and lightweight. It contains a great number of equipment slots and items can fit
different slots according to a bitmask. On this repository only a very small fraction of the equipment is usable with
waist located bag slots.

### Extensible bags system
In the proposed inventory system, inventory space can be extended by equipping and opening bags item. These bags can have different shape and size and allow for a more fine-tuning aspect of the inventory.
In the example project, 2 3x2 inventory space is given by default (representing trouser's pockets). This default space can be increase by equipping other bags on the given equipment slots.

### Items and bags size

As the inventory is based on a grid system, item size is inherently present.
However, to allow for more flexibility in the design of the bags, items have a supplementary size parameter.
This parameter is used with the bags property to be able to only contain items up to a maximum defined size.

### Item weight
Each item (and coin) have a specific weight that had up for encumbrance purposes.

### Double item slot
Some equipable items may use several slots of the inventory at once where applicable, such as a bag that would cover both shoulders.


## Monetary system
A basic monetary system is introduced. Each item have a basic value represented by a float.
Player have access to a variety of coins (namely Copper, Silver, Gold and Platinum) of different values. Any combination of coins that match the item value is valid.
Money are handled by Players and thus can be looted, but also by Merchants to handle transaction with players.

## Merchant system

Merchant system is kept to a bare minimum and uses ListWidget to display merchant owned items.
ListWidget is a code adaptation from https://www.youtube.com/watch?v=JyMEAx8-nbY

* Merchants can refuse to buy goods.
* Merchant have a limited pool of money to handle to the player.
* Merchants have a static pool of available items (think : bread, cookies and pie for a bakery) which are in unlimited amount for the player to buy.
* Merchants also have a dynamic pool that are incremented by player selling their items, and decremented when a player buy said item.
* Merchant have a ratio allowing to edit their greediness when buying or selling items.

## Loot system
Players can loot specific actors to gain access to new items and money.
This Looting system is replicated and allow a simple one player access to the items at once.
An automatic looting button is present.

## Trading system
Players can trade items and currency directly with each other through a secure two-phase commit system.
Both players must accept the trade before items and currency are exchanged.
The system includes trade cancellation, insufficient space detection, and full server-side validation to prevent cheating.

## Repair systems

### NPC Repair
NPCs can repair player equipment for a fee. Repair costs are configurable based on item value and durability loss.
NPCs can optionally refuse to repair certain item types. Full UI support with repair preview.

### Field Repair
Players can repair their own equipment using consumable repair kits. This system includes:
* Success/failure mechanics with optional skill integration
* Interruptible by combat, movement, or other actions
* Repair kits with limited charges
* Optional skill-up system on successful repairs

## Durability system
Equipment items can have durability that decreases with use (combat, time, etc.).
The durability system is optional and can be configured per item.
Items with low durability become less effective and require repair.
Durability state is fully replicated and persists to backend storage.

## Skeletal mesh equipment
Weapons and equipment can use animated skeletal meshes instead of just static meshes.
Includes support for left-hand IK for two-handed weapons and improved attachment system.

---

## Migration Notes

### FItemContainerLine Now Built Into Plugin

As of the latest version, `FItemContainerLine` (used for DataTable item registration) is now part of the InventoryPlugin instead of requiring manual implementation.

**If you get DataTable errors after updating:**

Add this to your `Config/DefaultEngine.ini`:

```ini
[CoreRedirects]
+StructRedirects=(OldName="/Script/YourModuleName.ItemContainerLine",NewName="/Script/InventoryPlugin.ItemContainerLine")
```

Replace `YourModuleName` with your game module name, then restart the editor. Your DataTables will automatically fix themselves.

See [Integration Guide - DataTable Troubleshooting](./Docs/Integration_Guide.md#troubleshooting-broken-datatables-after-plugin-update) for details.
