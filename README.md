# Unreal Engine 5 Inventory proof of concept

This repo is a proof of concept, grid based, fully replicated inventory and equipment system for unreal engine 5.

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
* Trading system

Despite some effort to make this plugin a bit more generic, you'd better be off forking this repo and and tailoring it to you needs.

## Integration tutorial:

You can find a this plugin integrated and used, including an in depth explanation of the necessary integration steps in this repository : https://github.com/Synock/UE5PluginIntegration

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
