#include "blocks.h"
#include "world.h"

// Roughly based on how they're defined in Infdev 20100327
bool IsOpaque(int16_t id) {
    return !(
        id == BLOCK_AIR ||
        id == BLOCK_LEAVES ||
        id == BLOCK_TALLGRASS ||
        id == BLOCK_TORCH ||
        id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_DANDELION ||
        id == BLOCK_ROSE ||
        id == BLOCK_FIRE ||
        id == BLOCK_FARMLAND ||
        id == BLOCK_SNOW_LAYER
    );
}

// Returns true for all translucent blocks
// So blocks that aren't 100% transparent
bool IsTranslucent(int16_t id) {
    return
        id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_LEAVES ||
        id == BLOCK_ICE ||
        id == BLOCK_MOB_SPAWNER
    ;
}

// Returns how much the skylight is filtered by the specified block
uint8_t GetTranslucency(int16_t id) {    
    // Water seems to drop the skylight brightness by 3 levels with each block
    switch(id) {
        case BLOCK_WATER_FLOWING:
        case BLOCK_WATER_STILL:
        case BLOCK_ICE:
            return 3;
        case BLOCK_LEAVES:
        case BLOCK_COBWEB:
            return 1;
        case BLOCK_LAVA_FLOWING:
        case BLOCK_LAVA_STILL:
            return 255;
        default:
            if (IsOpaque(id)) return 255;
    }
    return 0;
}

// Returns true for all blocks that are completely or partially transparent
bool IsTransparent(int16_t id) {
    return
        id == BLOCK_AIR ||
        id == BLOCK_SAPLING ||
        id == BLOCK_RAIL_POWERED ||
        id == BLOCK_RAIL_DETECTOR ||
        id == BLOCK_COBWEB ||
        id == BLOCK_GLASS ||
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
        id == BLOCK_REDSTONE_REPEATER_ON ||
        id == BLOCK_DOOR_IRON ||
        id == BLOCK_DOOR_WOOD ||
        id == BLOCK_TRAPDOOR
    ;
}

// TODO: Turn into int8_t for different light levels
// Returns true for all blocks that are emissive
bool IsEmissive(int16_t id) {
    return
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_FURNACE_LIT ||
        id == BLOCK_ORE_REDSTONE_ON || 
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_GLOWSTONE ||
        id == BLOCK_NETHER_PORTAL ||
        id == BLOCK_PUMPKIN_LIT
    ;
}

// Returns the level of emissiveness for each block
uint8_t GetEmissiveness(int16_t id) {
    switch(id) {
        case BLOCK_LAVA_FLOWING:
        case BLOCK_LAVA_STILL:
        case BLOCK_FIRE:
        case BLOCK_GLOWSTONE:
        case BLOCK_PUMPKIN_LIT:
            return 15;
        case BLOCK_TORCH:
            return 14;
        case BLOCK_FURNACE_LIT:
            return 13;
        case BLOCK_NETHER_PORTAL:
            return 11;
        case BLOCK_ORE_REDSTONE_ON:
            return 9;
        case BLOCK_REDSTONE_TORCH_ON:
            return 7;
        case BLOCK_MUSHROOM_BROWN:
            return 1;
        default:
            return 0;
    }
}

// Returns true for all blocks that are instantly breakable
bool IsInstantlyBreakable(int16_t id) {
    return
        id == BLOCK_SAPLING ||
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
        id == BLOCK_REDSTONE_REPEATER_ON
    ;
}

// Returns true if the passed block is interactable, and should thus cancel any block-placements
bool IsInteractable(int16_t id) {
    return
        id == BLOCK_BED ||
        id == BLOCK_DOOR_IRON ||
        id == BLOCK_DOOR_WOOD ||
        id == BLOCK_TRAPDOOR ||
        id == BLOCK_BUTTON_STONE ||
        id == BLOCK_LEVER ||
        id == BLOCK_CRAFTING_TABLE ||
        id == BLOCK_FURNACE ||
        id == BLOCK_FURNACE_LIT ||
        id == BLOCK_CHEST ||
        id == BLOCK_DISPENSER
    ;
}

bool HasInventory(int16_t id) {
    return (
            id == BLOCK_CHEST ||
            id == BLOCK_CRAFTING_TABLE ||
            id == BLOCK_FURNACE ||
            id == BLOCK_FURNACE_LIT ||
            id == BLOCK_DISPENSER
    );
}

bool IsLiquid(int16_t id) {
    return (
        id == BLOCK_WATER_STILL ||
        id == BLOCK_WATER_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_LAVA_FLOWING
    );
}

// TODO: Do this right
bool IsSolid(int16_t id) {
    return
        IsOpaque(id) ||
        id == BLOCK_LEAVES
    ;
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

uint8_t GetSignOrientation(float playerYaw) {
    playerYaw = fmod(fmod(playerYaw, 360.0) + 360.0, 360.0);
    // Divide into 360/16th angles
    const float angleSlice = 360.0f/16.0f;

    // This is a lot easier than a huge-ass if-else chain
    int index = static_cast<int>(std::floor((playerYaw + angleSlice * 0.5f) / angleSlice)) % 16;
    if (index < 0) index += 16;

    // Face the player
    index = (index + 8) % 16;

    return static_cast<uint8_t>(index);
}

// Figure out which block should be placed based on the passed parameters
Block GetPlacedBlock(World* world, Int3 pos, int8_t face, float playerYaw, int8_t playerDirection, int16_t id, int16_t damage) {
	Block b = Block{(int8_t)id,(int8_t)damage,0};

	// Handle items that place as blocks
    if (id == ITEM_HOE_DIAMOMD ||
        id == ITEM_HOE_GOLD ||
        id == ITEM_HOE_IRON ||
        id == ITEM_HOE_STONE ||
        id == ITEM_HOE_WOOD
    ) {
        if (world->GetBlockType(pos-Int3{0,1,0}) == BLOCK_GRASS) {
            world->PlaceBlockUpdate(pos-Int3{0,1,0},BLOCK_FARMLAND);
        }
        id = SLOT_EMPTY;
        return b;
    }
    if (id == ITEM_SEEDS_WHEAT) {
        b.type = BLOCK_CROP_WHEAT;
        return b;
    }
	if (id == ITEM_REDSTONE) {
		b.type = BLOCK_REDSTONE;
		return b;
	}
	if (id == ITEM_SIGN) {
        b.type = BLOCK_SIGN_WALL;
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
                b.meta = 5;
                return b;
            case yPlus:
                b.type = BLOCK_SIGN;
                b.meta = GetSignOrientation(playerYaw);
                return b;
        }
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
        // TODO: Any non-solid block should work
        if (world->GetBlockType(pos+Int3{0,1,0}) != BLOCK_AIR ||
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
        world->PlaceBlockUpdate(pos+Int3{0,1,0},b.type,b.meta | 0b1000);
        return b;
    }
    if (id == ITEM_BED) {
        Int3 headboardOffset = Int3{0,0,0};
        // Determine the direction
        switch(playerDirection) {
            case zPlus:
                headboardOffset = Int3{0,0,1};
                b.meta = 0;
                break;
            case xMinus:
                headboardOffset = Int3{-1,0,0};
                b.meta = 1;
                break;
            case zMinus:
                headboardOffset = Int3{0,0,-1};
                b.meta = 2;
                break;
            case xPlus:
                headboardOffset = Int3{1,0,0};
                b.meta = 3;
                break;
        }
        // TODO: Any non-solid block should work
        if (world->GetBlockType(pos+headboardOffset) != BLOCK_AIR ||
            face != yPlus
        ) {
            b.type = SLOT_EMPTY;
            return b;
        }
        // Determine the door type
        b.type = BLOCK_BED;
        world->PlaceBlockUpdate(pos+headboardOffset,b.type,b.meta | 0b1000);
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

    // --- ALL ITEMS WAS A RIGHT-CLICK USE MUST BE HANDLED BEFORE HERE ---
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

bool CanGrow(int8_t type, int8_t otherType) {
    switch(type) {
        case BLOCK_MUSHROOM_BROWN:
        case BLOCK_MUSHROOM_RED:
            return IsOpaque(otherType);
        case BLOCK_DANDELION:
        case BLOCK_ROSE:
        case BLOCK_TALLGRASS:
            return
                otherType == BLOCK_GRASS ||
                otherType == BLOCK_DIRT ||
                otherType == BLOCK_FARMLAND
            ;
        case BLOCK_DEADBUSH:
            return otherType == BLOCK_SAND;
        case BLOCK_CROP_WHEAT:
            return otherType == BLOCK_FARMLAND;
        default:
            break;
    }
    return false;
}

// Check if a block can exist in the position its in
bool CanStay(int8_t type, World* world, Int3 pos) {
    switch(type) {
        case BLOCK_MUSHROOM_BROWN:
        case BLOCK_MUSHROOM_RED:
            if ((pos.y >= 0) && (pos.y < CHUNK_HEIGHT)) {
                return (world->GetTotalLight(pos) < 13 && CanGrow(type, world->GetBlockType(pos - Int3{0,-1,0})));
            }
            break;
        case BLOCK_DANDELION:
        case BLOCK_ROSE:
        case BLOCK_DEADBUSH:
        case BLOCK_TALLGRASS:
            return (
                (
                    world->GetTotalLight(pos) >= 8 ||
                    world->CanBlockSeeTheSky(pos)
                ) && 
                CanGrow(type, world->GetBlockType(Int3{pos.x,pos.y-1,pos.z}))
            );
        case BLOCK_CACTUS:
            {
                if (
                    IsSolid(world->GetBlockType(pos+Int3{-1,0,0})) ||
                    IsSolid(world->GetBlockType(pos+Int3{ 1,0,0})) ||
                    IsSolid(world->GetBlockType(pos+Int3{0,0,-1})) ||
                    IsSolid(world->GetBlockType(pos+Int3{0,0, 1}))
                ) {
                    return false;
                }
                int8_t blockType = world->GetBlockType(pos+Int3{0,-1,0});
                return (blockType == BLOCK_CACTUS || blockType == BLOCK_SAND);
            }
        case BLOCK_SUGARCANE:
            {
                int8_t blockType = world->GetBlockType(pos+Int3{0,-1,0});
                if (blockType == type) return true;
                if (blockType != BLOCK_GRASS && blockType != BLOCK_DIRT) return false;
                return (
                    world->GetBlockType(pos+Int3{-1,-1, 0}) == BLOCK_WATER_STILL ||
                    world->GetBlockType(pos+Int3{-1,-1, 0}) == BLOCK_WATER_FLOWING ||
                    world->GetBlockType(pos+Int3{ 1,-1, 0}) == BLOCK_WATER_STILL ||
                    world->GetBlockType(pos+Int3{ 1,-1, 0}) == BLOCK_WATER_FLOWING ||
                    world->GetBlockType(pos+Int3{ 0,-1,-1}) == BLOCK_WATER_STILL ||
                    world->GetBlockType(pos+Int3{ 0,-1,-1}) == BLOCK_WATER_FLOWING ||
                    world->GetBlockType(pos+Int3{ 0,-1, 1}) == BLOCK_WATER_STILL ||
                    world->GetBlockType(pos+Int3{ 0,-1, 1}) == BLOCK_WATER_FLOWING
                );
            }
        default:
            break;
    }
    // If nothing is set here, assume it can stay
    return true;
}

bool CanBePlaced(int8_t type, World* world, Int3 pos) {
    switch(type) {
        case BLOCK_SUGARCANE:
            return CanStay(type,world,pos);
        default:
            return true;
    }
}