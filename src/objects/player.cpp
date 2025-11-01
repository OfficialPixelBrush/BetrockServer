#include "player.h"
#include "client.h"
#include <cstdint>

// Get the players velocity
Vec3 Player::GetVelocity() {
    return previousPosition - position;
}

void Player::SetHealth(std::vector<uint8_t> &response, int8_t health) {
    if (health > HEALTH_MAX) { health = HEALTH_MAX; }
    if (health < 0) { health = 0; }
    this->health = health;
    Respond::UpdateHealth(response, this->health);
}

void Player::Hurt(std::vector<uint8_t> &response, int8_t damage) {
    this->health = this->health - damage;
    Respond::UpdateHealth(response, this->health);
}

void Player::Kill(std::vector<uint8_t> &response) {
    SetHealth(response,0);
}

void Player::PrintStats() {
    std::cout << username << ": " << position << ";" << stance <<  "; " << yaw << ", " << pitch << std::endl;
}

// Store the player data as an NBT-format file
void Player::Save() {
    // Note: Right now this implements all tags that a Beta 1.7.3 player.nbt has
    // This does not mean we actually, properly implement all of these just yet
	auto root = std::make_shared<CompoundTag>("");

    // TODO: Probably calulated based on where the player was in the previous tick?
    // Could use lastTickPosition
	auto motionList = std::make_shared<ListTag>("Motion");
	root->Put(motionList);
	motionList->Put(std::make_shared<DoubleTag>("x",0.0));
	motionList->Put(std::make_shared<DoubleTag>("y",-0.0784000015258789));
	motionList->Put(std::make_shared<DoubleTag>("z",0.0));

	root->Put(std::make_shared<ShortTag>("SleepTimer", 0));
	root->Put(std::make_shared<ShortTag>("Health",health));
	root->Put(std::make_shared<ShortTag>("Air",300));
	root->Put(std::make_shared<ByteTag>("OnGround",onGround));
	root->Put(std::make_shared<IntTag>("Dimension",dimension));

	auto rotationList = std::make_shared<ListTag>("Rotation");
	rotationList->Put(std::make_shared<FloatTag>("Yaw"  ,yaw));
	rotationList->Put(std::make_shared<FloatTag>("Pitch",pitch));
	root->Put(rotationList);
	
	root->Put(std::make_shared<FloatTag>("FallDistance", 0.0));
	root->Put(std::make_shared<ByteTag>("Sleeping", 0));

	auto posList = std::make_shared<ListTag>("Pos");
	posList->Put(std::make_shared<DoubleTag>("x",position.x));
	posList->Put(std::make_shared<DoubleTag>("y",position.y));
	posList->Put(std::make_shared<DoubleTag>("z",position.z));
	root->Put(posList);

	root->Put(std::make_shared<ShortTag>("DeathTime",0));
	root->Put(std::make_shared<ShortTag>("Fire",-20));
	root->Put(std::make_shared<ShortTag>("HurtTime",0));
	root->Put(std::make_shared<ShortTag>("AttackTime",0));

	auto nbtInventory = std::make_shared<ListTag>("Inventory");
	root->Put(nbtInventory);

    for (int i = 0; i < INVENTORY_MAIN_SIZE; i++) {
        Item item = inventory[i];
        if (item.id == SLOT_EMPTY) {
            continue;
        }
        nbtInventory->Put(
            NbtItem(
                InventoryMappingLocalToNbt(INVENTORY_SECTION_MAIN, i),
                item.id,
                item.amount,
                item.damage
            )
        );
    }

    for (int i = 0; i < INVENTORY_ARMOR_SIZE; i++) {
        Item item = armor[i];
        if (item.id == SLOT_EMPTY) {
            continue;
        }
        nbtInventory->Put(
            NbtItem(
                InventoryMappingLocalToNbt(INVENTORY_SECTION_ARMOR, i),
                item.id,
                item.amount,
                item.damage
            )
        );
    }

	// Yeet to file
    std::filesystem::path dirPath = Betrock::GlobalConfig::Instance().Get("level-name");
    if (dirPath == "") {
        Betrock::Logger::Instance().Warning(("Dirpath is empty!"));
        return;
    }
    dirPath += "/players/";
    if (std::filesystem::create_directories(dirPath)) {
        std::cout << "Directory created: " << dirPath << '\n';
    }
    std::filesystem::path entryPath = dirPath / (username + ".dat");
    std::ofstream writeFile(entryPath, std::ios::binary);
	NbtWrite(writeFile,root);
    writeFile.close();
}

// Load the players data from an NBT-format file
bool Player::Load() {
    std::filesystem::path dirPath = Betrock::GlobalConfig::Instance().Get("level-name");
    dirPath += "/players/";
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Player Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }
    std::filesystem::path entryPath = dirPath / (username + ".dat");
    if (!std::filesystem::exists(entryPath)) {
        return false;
    }

    // Load the NBT Data into the root node
    std::ifstream readFile(entryPath, std::ios::binary);
    auto root = std::dynamic_pointer_cast<CompoundTag>(NbtRead(readFile));
    readFile.close();

    // Get the players saved rotation
    std::shared_ptr<ListTag> rotationList = std::dynamic_pointer_cast<ListTag>(root->Get("Rotation"));
    yaw   = std::dynamic_pointer_cast<FloatTag>(rotationList->Get(0))->GetData();
    pitch = std::dynamic_pointer_cast<FloatTag>(rotationList->Get(1))->GetData();

    // Get the players saved position
    std::shared_ptr<ListTag> posList = std::dynamic_pointer_cast<ListTag>(root->Get("Pos"));
    position.x = std::dynamic_pointer_cast<DoubleTag>(posList->Get(0))->GetData();
    position.y = std::dynamic_pointer_cast<DoubleTag>(posList->Get(1))->GetData();
    position.z = std::dynamic_pointer_cast<DoubleTag>(posList->Get(2))->GetData();

    health = std::dynamic_pointer_cast<ShortTag>(root->Get("Health"))->GetData();


    // Get the players saved inventory
    std::shared_ptr<ListTag> inventoryList = std::dynamic_pointer_cast<ListTag>(root->Get("Inventory"));
    for (size_t i = 0; i < inventoryList->GetNumberOfTags(); i++) {
        auto slot = std::dynamic_pointer_cast<CompoundTag>(inventoryList->Get(i));
        [[maybe_unused]] int8_t  slotNumber = std::dynamic_pointer_cast<ByteTag>(slot->Get("Slot"))->GetData();
        [[maybe_unused]] int16_t itemId = std::dynamic_pointer_cast<ShortTag>(slot->Get("id"))->GetData();
        [[maybe_unused]] int8_t  itemCount = std::dynamic_pointer_cast<ByteTag>(slot->Get("Count"))->GetData();
        [[maybe_unused]] int16_t itemDamage = std::dynamic_pointer_cast<ShortTag>(slot->Get("Damage"))->GetData();
        if (slotNumber >= 100) {
            armor[InventoryMappingNbtToLocal(INVENTORY_SECTION_ARMOR, slotNumber)] = {
                itemId,
                itemCount,
                itemDamage
            };
        } else {
            inventory[InventoryMappingNbtToLocal(INVENTORY_SECTION_MAIN, slotNumber)] = {
                itemId,
                itemCount,
                itemDamage
            };
        }
    }
    return true;
}

int8_t Player::InventoryMappingLocalToNbt(INVENTORY_SECTION section, int8_t slotId) {
    switch(section) {
        case INVENTORY_SECTION_MAIN:
            return slotId;
        case INVENTORY_SECTION_ARMOR:
            return slotId+100;
        default:
        case INVENTORY_SECTION_CRAFTING:
            return 0;
    }
}

int8_t Player::InventoryMappingNbtToLocal(INVENTORY_SECTION section, int8_t slotId) {
    switch(section) {
        case INVENTORY_SECTION_MAIN:
            return slotId;
        case INVENTORY_SECTION_ARMOR:
            return slotId-100;
        default:
        case INVENTORY_SECTION_CRAFTING:
            return 0;
    }
}