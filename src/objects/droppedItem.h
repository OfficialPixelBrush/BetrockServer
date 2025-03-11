#pragma once
#include <cstdint>
#include "helper.h"
#include "entity.h"
#include "entityType.h"

class DroppedItem : public Entity {
    public:
        Item item;

        DroppedItem(int &entityId, EntityType entityType, Vec3 position, int8_t dimension, std::string world, Item item)
            : Entity(entityId++, entityType, position, dimension, world),
            item(item)
        {}
};