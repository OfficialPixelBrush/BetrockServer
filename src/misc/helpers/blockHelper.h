#pragma once
#include <cstdint>

#include "datatypes.h"
#include "directions.h"
#include "items.h"

class World;

bool IsOpaque(int16_t id);
bool IsTranslucent(int16_t id);
uint8_t GetOpacity(int16_t id);
bool IsTransparent(int16_t id);
bool IsEmissive(int16_t id);
bool IsLiquid(int16_t id);
bool IsSolid(int16_t id);
uint8_t GetEmissiveness(int16_t id);
bool IsInstantlyBreakable(int16_t id);
bool IsInteractable(int16_t id);
bool HasInventory(int16_t id);
bool CanStay(int8_t type, World *world, Int3 pos);
bool CanBePlaced(int8_t type, World *world, Int3 pos);
uint8_t GetSignOrientation(float playerYaw);
Block GetPlacedBlock(World *world, Int3 pos, Int3 targetPos, int8_t face, float playerYaw, int8_t playerDirection, int16_t id,
					 int16_t damage);
void BlockToFace(Int3 &pos, int8_t &direction);