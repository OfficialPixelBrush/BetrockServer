#include "blocks.h"
#include "world.h"

// Returns true for all translucent blocks
// So blocks that aren't 100% transparent
bool IsTranslucent(int16_t id) {
    if (id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_LEAVES ||
        id == BLOCK_GLASS ||
        id == BLOCK_ICE ||
        id == BLOCK_MOB_SPAWNER)
    {
        return true;
    }
    return false;
}

// Returns how much the skylight is filtered by the specified block
uint8_t GetTranslucency(int16_t id, uint8_t &skylight) {
    uint8_t subtractor = 0;
    // Water seems to drop the skylight brightness by 3 levels with each block
    if (id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL) {
        subtractor = 3;
    }

    // Meanwhile, leaves seem to drop the skylight by 1 with each block, until reaching 12
    if (id == BLOCK_LEAVES && skylight > 12) {
        subtractor = 1;
    }

    if (
        id == BLOCK_GLASS ||
        id == BLOCK_ICE ||
        id == BLOCK_MOB_SPAWNER
    ) {
        subtractor = 1;
    }

    // Do this check to prevent a potential overflow
    if (skylight < subtractor) {
        skylight = 0;
    } else {
        skylight -= subtractor;
    }
    return skylight;
}

// Returns true for all blocks that are completely or partially transparent
bool IsTransparent(int16_t id) {
    if (id == BLOCK_AIR ||
        id == BLOCK_SAPLING ||
        id == BLOCK_RAIL_POWERED ||
        id == BLOCK_RAIL_DETECTOR ||
        id == BLOCK_COBWEB ||
        id == BLOCK_TALLGRASS ||
        id == BLOCK_DEADBUSH ||
        id == BLOCK_DANDELION ||
        id == BLOCK_ROSE ||
        id == BLOCK_MUSHROOM_BROWN ||
        id == BLOCK_MUSHROOM_RED ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_MOB_SPAWNER ||
        id == BLOCK_REDSTONE ||
        id == BLOCK_CROP_WHEAT ||
        id == BLOCK_SIGN ||
        id == BLOCK_LADDER ||
        id == BLOCK_RAIL ||
        id == BLOCK_SIGN_WALL ||
        id == BLOCK_LEVER ||
        id == BLOCK_PRESSURE_PLATE_STONE ||
        id == BLOCK_PRESSURE_PLATE_WOOD ||
        id == BLOCK_REDSTONE_TORCH_OFF ||
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_BUTTON_STONE ||
        id == BLOCK_CACTUS ||
        id == BLOCK_SUGARCANE ||
        id == BLOCK_FENCE ||
        id == BLOCK_REDSTONE_REPEATER_OFF ||
        id == BLOCK_REDSTONE_REPEATER_ON
        )
    {
        return true;
    }
    return false;
}

// TODO: Turn into int8_t for different light levels
// Returns true for all blocks that are emissive
bool IsEmissive(int16_t id) {
    if (id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_FURNACE_LIT ||
        id == BLOCK_ORE_REDSTONE_ON || 
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_GLOWSTONE ||
        id == BLOCK_NETHER_PORTAL ||
        id == BLOCK_PUMPKIN_LIT)
    {
        return true;
    }
    return false;
}

// Returns the level of emissiveness for each block
uint8_t GetEmissiveness(int16_t id) {
    if (id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_FIRE ||
        id == BLOCK_GLOWSTONE ||
        id == BLOCK_PUMPKIN_LIT)
    {
        return 15;
    } else if (id == BLOCK_TORCH) {
        return 14;
    } else if (id == BLOCK_FURNACE_LIT) {
        return 13;
    } else if (id == BLOCK_NETHER_PORTAL) {
        return 11;
    } else if (id == BLOCK_ORE_REDSTONE_ON) {
        return 9;
    } else if (id == BLOCK_REDSTONE_TORCH_ON) {
        return 7;
    }
    // TODO: Apparently brown mushrooms glow, but I don't know if that's the case for Beta 1.7.3
    // Test this!
    return 0;
}

// Returns true for all blocks that are instantly breakable
bool IsInstantlyBreakable(int16_t id) {
    if (id == BLOCK_SAPLING ||
        id == BLOCK_DEADBUSH ||
        id == BLOCK_TALLGRASS ||
        id == BLOCK_DANDELION || 
        id == BLOCK_ROSE ||
        id == BLOCK_MUSHROOM_BROWN ||
        id == BLOCK_MUSHROOM_RED ||
        id == BLOCK_TNT ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_REDSTONE ||
        id == BLOCK_CROP_WHEAT ||
        id == BLOCK_REDSTONE_TORCH_OFF ||
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_SUGARCANE ||
        id == BLOCK_REDSTONE_REPEATER_OFF ||
        id == BLOCK_REDSTONE_REPEATER_ON)
    {
        return true;
    }
    return false;
}

// Returns true if the passed block is interactable, and should thus cancel any block-placements
bool IsInteractable(int16_t id) {
    if (id == BLOCK_BED ||
        id == BLOCK_DOOR_IRON ||
        id == BLOCK_DOOR_WOOD ||
        id == BLOCK_TRAPDOOR ||
        id == BLOCK_BUTTON_STONE ||
        id == BLOCK_LEVER
    ) {
        return true;
    }
    return false;
}

bool InteractWithBlock(Block* b) {
    if (b->type == BLOCK_TRAPDOOR ||
        b->type == BLOCK_DOOR_WOOD
    ) {
        b->meta = b->meta ^ 0b100;
    }
    return true;
}

// Returns true if the destroyed item maintains its NBT data upon being dropped
bool KeepDamageOnDrop(int8_t type) {
    if (type == BLOCK_WOOL) {
        return true;
    }
    return false;
}

// Returns true for all blocks that do not drop anything when they're destroyed
bool NoDrop(Item item) {
    if (item.id == BLOCK_BEDROCK ||
        item.id == BLOCK_WATER_FLOWING ||
        item.id == BLOCK_WATER_STILL || 
        item.id == BLOCK_LAVA_FLOWING ||
        item.id == BLOCK_LAVA_STILL ||
        item.id == BLOCK_GLASS ||
        item.id == BLOCK_COBWEB ||
        item.id == BLOCK_DEADBUSH ||
        item.id == BLOCK_TALLGRASS ||
        item.id == BLOCK_FIRE ||
        item.id == BLOCK_MOB_SPAWNER ||
        item.id == BLOCK_ICE ||
        item.id == BLOCK_NETHER_PORTAL ||
        item.id == BLOCK_CAKE
    ) {
        return true;
    }
    return false;
}

// Returns the items that're dropped when a block is destroyed
Item GetDrop(Item item) {
    if (NoDrop(item)) {
        return Item{ -1, 0, 0 };
    }
    // By default, give back one of the same block
    if (item.id == BLOCK_CROP_WHEAT) {
        if (item.damage < MAX_CROP_SIZE) {
            item.id = ITEM_SEEDS_WHEAT;
        } else {
            item.id = ITEM_WHEAT;
        }
    }
    if (item.id == BLOCK_STONE) {
        item.id = BLOCK_COBBLESTONE;
    }
    if (item.id == BLOCK_GRASS) {
        item.id = BLOCK_DIRT;
    }
    if (item.id == BLOCK_SUGARCANE) {
        item.id = ITEM_SUGARCANE;
    }
    if (item.id == BLOCK_ORE_COAL) {
        item.id = ITEM_COAL;
    }
    if (item.id == BLOCK_LEAVES) {
        item.id = BLOCK_SAPLING;
        // 1/20 chance
        item.amount = 1;
    }
    if (item.id == BLOCK_ORE_LAPIS_LAZULI) {
        item.id = ITEM_DYE;
        // 4-8
        item.amount = 4;
        item.damage = 4;
    }
    if (item.id == BLOCK_BED) {
        item.id = ITEM_BED;
    }
    if (item.id == BLOCK_REDSTONE) {
        item.id = ITEM_REDSTONE;
    }
    if (item.id == BLOCK_ORE_DIAMOND) {
        item.id = ITEM_DIAMOND;
    }
    if (item.id == BLOCK_CROP_WHEAT) {
        item.id = ITEM_WHEAT;
    }
    if (item.id == BLOCK_SIGN || item.id == BLOCK_SIGN_WALL) {
        item.id = ITEM_SIGN;
    }
    if (item.id == BLOCK_DOOR_WOOD) {
        item.id = ITEM_DOOR_WOOD;
    }
    if (item.id == BLOCK_DOOR_IRON) {
        item.id = ITEM_DOOR_IRON;
    }
    if (item.id == BLOCK_ORE_REDSTONE_OFF || item.id == BLOCK_ORE_REDSTONE_ON) {
        item.id = ITEM_REDSTONE;
        // 4-5
        item.amount = 4;
    }
    if (item.id == BLOCK_REDSTONE_TORCH_OFF || item.id == BLOCK_REDSTONE_TORCH_ON) {
        item.id = BLOCK_REDSTONE_TORCH_ON;
    }
    if (item.id == BLOCK_CLAY) {
        item.id = ITEM_CLAY;
        // ???
        item.amount = 4;
    }
    if (item.id == BLOCK_GLOWSTONE) {
        item.id = ITEM_GLOWSTONE_DUST;
        // 2-4
        item.amount = 2;
    }
    if (item.id == BLOCK_REDSTONE_REPEATER_ON || item.id == BLOCK_REDSTONE_REPEATER_OFF) {
        item.id = BLOCK_REDSTONE_REPEATER_OFF;
    }
    if (!KeepDamageOnDrop(item.id)) {
        item.damage = 0;
    }
    return item;
}

// Determine in which direction a block needs to be placed
void BlockToFace(Int3& pos, int8_t& direction) {
	switch(direction) {
		case yMinus:
			pos.y--;
			break;
		case yPlus:
            pos.y++;
			break;
		case zMinus:
            pos.z--;
			break;
		case zPlus:
            pos.z++;
			break;
		case xMinus:
            pos.x--;
			break;
		case xPlus:
            pos.x++;
			break;
		default:
			break;
	}
}

// Figure out which block should be placed based on the passed parameters
Block GetPlacedBlock(World* world, Int3 pos, int8_t face, int8_t playerDirection, int16_t id, int16_t damage) {
	Block b = Block{(int8_t)id,(int8_t)damage,0,0};

	// Handle items that place as blocks
	if (id == ITEM_REDSTONE) {
		b.type = BLOCK_REDSTONE;
		return b;
	}
	if (id == ITEM_SUGARCANE) {
		b.type = BLOCK_SUGARCANE;
		return b;
	}
    if (id == ITEM_DOOR_WOOD ||
        id == ITEM_DOOR_IRON
    ) {
        // Check the block above this one
        Block* aboveBlock = world->GetBlock(pos+Int3{0,1,0});
        // TODO: Any non-solid block should work
        if (aboveBlock->type != BLOCK_AIR ||
            face != yPlus
        ) {
            b.type = SLOT_EMPTY;
            return b;
        }
        // Determine the door type
        switch(id) {
            case ITEM_DOOR_WOOD:
                b.type = 64;
                break;
            case ITEM_DOOR_IRON:
                b.type = 71;
                break;
        }
        // Determine the direction
        switch(playerDirection) {
            case xPlus:
                b.meta = 0;
                break;
            case zPlus:
                b.meta = 1;
                break;
            case xMinus:
                b.meta = 2;
                break;
            case zMinus:
                b.meta = 3;
                break;
        }
        world->PlaceBlock(pos+Int3{0,1,0},b.type,b.meta | 0b1000);
        return b;
    }
	if (id == ITEM_REDSTONE_REPEATER ||
		id == BLOCK_REDSTONE_REPEATER_OFF ||
		id == BLOCK_REDSTONE_REPEATER_ON) {
		b.type = BLOCK_REDSTONE_REPEATER_OFF;
		switch(playerDirection) {
			case zMinus:
				b.meta = 0;
				return b;
			case xPlus:
				b.meta = 1;
				return b;
			case zPlus:
				b.meta = 2;
				return b;
			case xMinus:
				b.meta = 3;
				return b;
		}
		return b;
	}

	// If it hasn't been caught yet by any of the items
	// its an invalid block, so we don't care.
	if (id > BLOCK_MAX) {
		b.type = 0;
		return b;
	}

	// Handle placement of blocks
    if (id == BLOCK_TRAPDOOR) {
        switch(face) {
            case zMinus:
                b.meta = 0;
                return b;
            case zPlus:
                b.meta = 1;
                return b;
            case xMinus:
                b.meta = 2;
                return b;
            case xPlus:
                b.meta = 3;
                return b;
        }
    }
	if (id == BLOCK_STAIRS_WOOD ||
		id == BLOCK_STAIRS_COBBLESTONE
	) {
		switch(playerDirection) {
			case xPlus:
				b.meta = 0;
				return b;
			case xMinus:
				b.meta = 1;
				return b;
			case zPlus:
				b.meta = 2;
				return b;
			case zMinus:
				b.meta = 3;
				return b;
		}
	}
	if (id == BLOCK_DISPENSER ||
		id == BLOCK_FURNACE ||
		id == BLOCK_FURNACE_LIT
	) {
		switch(playerDirection) {
			case zPlus:
				b.meta = 2;
				return b;
			case zMinus:
				b.meta = 3;
				return b;
			case xPlus:
				b.meta = 4;
				return b;
			case xMinus:
				b.meta = 5;
				return b;
		}
	}
	if (id == BLOCK_PUMPKIN ||
		id == BLOCK_PUMPKIN_LIT
	) {
		switch(playerDirection) {
			case zMinus:
				b.meta = 0;
				return b;
			case xPlus:
				b.meta = 1;
				return b;
			case zPlus:
				b.meta = 2;
				return b;
			case xMinus:
				b.meta = 3;
				return b;
		}
	}
	if (id == BLOCK_TORCH ||
		id == BLOCK_REDSTONE_TORCH_OFF||
		id == BLOCK_REDSTONE_TORCH_ON
	) {
		switch(face) {
			case yMinus:
				b.type = SLOT_EMPTY;
				return b;
			case zPlus:
				b.meta = 3;
				return b;
			case zMinus:
				b.meta = 4;
				return b;
			case xPlus:
				b.meta = 1;
				return b;
			case xMinus:
				b.meta = 2;
				return b;
			default:
				b.meta = 0;
				return b;
		}
	}
	if (id == BLOCK_LADDER) {
		switch(face) {
			case zMinus:
				b.meta = 2;
				return b;
			case zPlus:
				b.meta = 3;
				return b;
			case xMinus:
				b.meta = 4;
				return b;
			case xPlus:
				b.meta =  5;
				return b;
		}
	}
	return b;
}

// Tick the passed block
void RandomTick(Block* b, Int3 pos) {
    switch(b->type) {
        /*
        case BLOCK_DIRT:
            b->type = BLOCK_GRASS;
            return;
        */
        case BLOCK_CROP_WHEAT:
            if (b->meta < MAX_CROP_SIZE) {
                b->meta++;
            }
            return;
    }
}