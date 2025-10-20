#include <vector>
#include "datatypes.h"

class Recipe {
    private:
        // If recipe can be shapeless
        bool shapeless = false;
        // Required minimum crafting grid size
        uint8_t size = 3;
        // Crafting recipe (size*size)
        std::vector<Item> recipe;
        // Item(s) that crafting will return
        Item resultItem;
    public:
        Recipe(bool shapeless, uint8_t size, std::vector<Item> recipe, Item resultItem)
            : shapeless(shapeless), size(size), recipe(recipe), resultItem(resultItem) {}
        bool Check(std::vector<Item> input);
};