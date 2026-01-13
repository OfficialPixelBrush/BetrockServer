#include "player.h"
#include "client.h"
#include <cstdint>

// Get the players velocity
Vec3 Player::GetVelocity() {
    return previousPosition - position;
}

void Player::SetHealth(std::vector<uint8_t> &response, int8_t pHealth) {
    if (pHealth > HEALTH_MAX) pHealth = HEALTH_MAX;
    if (pHealth < 0) pHealth = 0;
    health = pHealth;
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
    std::cout << username << ": " << position << ";" << stance <<  "; " << yaw << ", " << pitch << "\n";
}

// Store the player data as an NBT-format file
void Player::Save() {
    // Note: Right now this implements all tags that a Beta 1.7.3 player.nbt has
    // This does not mean we actually, properly implement all of these just yet
	auto root = std::make_shared<CompoundNbtTag>("");

    // TODO: Probably calulated based on where the player was in the previous tick?
    // Could use lastTickPosition
	auto motionList = std::make_shared<ListNbtTag>("Motion");
	root->Put(motionList);
	motionList->Put(std::make_shared<DoubleNbtTag>("x",0.0));
	motionList->Put(std::make_shared<DoubleNbtTag>("y",-0.0784000015258789));
	motionList->Put(std::make_shared<DoubleNbtTag>("z",0.0));

	root->Put(std::make_shared<ShortNbtTag>("SleepTimer", 0));
	root->Put(std::make_shared<ShortNbtTag>("Health",health));
	root->Put(std::make_shared<ShortNbtTag>("Air",300));
	root->Put(std::make_shared<ByteNbtTag>("OnGround",onGround));
	root->Put(std::make_shared<IntNbtTag>("Dimension",dimension));

	auto rotationList = std::make_shared<ListNbtTag>("Rotation");
	rotationList->Put(std::make_shared<FloatNbtTag>("Yaw"  ,yaw));
	rotationList->Put(std::make_shared<FloatNbtTag>("Pitch",pitch));
	root->Put(rotationList);
	
	root->Put(std::make_shared<FloatNbtTag>("FallDistance", 0.0));
	root->Put(std::make_shared<ByteNbtTag>("Sleeping", 0));

	auto posList = std::make_shared<ListNbtTag>("Pos");
	posList->Put(std::make_shared<DoubleNbtTag>("x",position.x));
	posList->Put(std::make_shared<DoubleNbtTag>("y",position.y));
	posList->Put(std::make_shared<DoubleNbtTag>("z",position.z));
	root->Put(posList);

	root->Put(std::make_shared<ShortNbtTag>("DeathTime",0));
	root->Put(std::make_shared<ShortNbtTag>("Fire",-20));
	root->Put(std::make_shared<ShortNbtTag>("HurtTime",0));
	root->Put(std::make_shared<ShortNbtTag>("AttackTime",0));

	auto nbtInventory = std::make_shared<ListNbtTag>("Inventory");
	root->Put(nbtInventory);

    // Player Inventory slots in NBT order
    Inventory nbtInv = Inventory();
    nbtInv.Append(hotbarSlots);
    nbtInv.Append(inventory.GetRow(0));
    nbtInv.Append(inventory.GetRow(1));
    nbtInv.Append(inventory.GetRow(2));
    int slotId = 0;

    for (auto& ivs : nbtInv.GetLinearSlots()) {
        if (ivs.id == SLOT_EMPTY)
            continue;
        nbtInventory->Put(
            NbtItem(
                slotId++,
                ivs.id,
                ivs.amount,
                ivs.damage
            )
        );
    }

    // Armor slots in NBT order
    slotId = 100; // Starts at boots, goes up to helmet
    for (auto& i : armorSlots.GetLinearSlots()) {
        if (i.id == SLOT_EMPTY)
            continue;
        nbtInventory->Put(
            NbtItem(
                slotId++,
                i.id,
                i.amount,
                i.damage
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
        std::cerr << "Player Directory " << dirPath << " does not exist or is not a directory!" << "\n";
        return false;
    }
    std::filesystem::path entryPath = dirPath / (username + ".dat");
    if (!std::filesystem::exists(entryPath)) {
        return false;
    }

    // Load the NBT Data into the root node
    std::ifstream readFile(entryPath, std::ios::binary);
    auto root = std::dynamic_pointer_cast<CompoundNbtTag>(NbtRead(readFile));
    readFile.close();

    // Get the players saved rotation
    std::shared_ptr<ListNbtTag> rotationList = std::dynamic_pointer_cast<ListNbtTag>(root->Get("Rotation"));
    auto yawTag = std::dynamic_pointer_cast<FloatNbtTag>(rotationList->Get(0));
    if (yawTag)
        yaw   = yawTag->GetData();
    auto pitchTag = std::dynamic_pointer_cast<FloatNbtTag>(rotationList->Get(1));
    if (pitchTag)
        pitch   = pitchTag->GetData();

    // Get the players saved position
    std::shared_ptr<ListNbtTag> posList = std::dynamic_pointer_cast<ListNbtTag>(root->Get("Pos"));
    auto posTag = std::dynamic_pointer_cast<DoubleNbtTag>(posList->Get(0));
    if (posTag)
        position.x = posTag->GetData();
    posTag = std::dynamic_pointer_cast<DoubleNbtTag>(posList->Get(1));
    if (posTag)
        position.y = posTag->GetData();
    posTag = std::dynamic_pointer_cast<DoubleNbtTag>(posList->Get(2));
    if (posTag)
        position.z = posTag->GetData();

    auto healthTag = std::dynamic_pointer_cast<ShortNbtTag>(root->Get("Health"));
    if (healthTag)
        health = healthTag->GetData();

    // Get the players saved inventory
    std::shared_ptr<ListNbtTag> inventoryList = std::dynamic_pointer_cast<ListNbtTag>(root->Get("Inventory"));
    // This approach is a little hacky, but it works
    InventoryRow nbtInv = InventoryRow(INVENTORY_MAIN_ROWS * INVENTORY_MAIN_COLS);
    for (size_t i = 0; i < inventoryList->GetNumberOfTags(); i++) {
        auto slot = std::dynamic_pointer_cast<CompoundNbtTag>(inventoryList->Get(i));
        [[maybe_unused]] int8_t  slotNumber = 0;
        [[maybe_unused]] int16_t itemId = -1;
        [[maybe_unused]] int8_t  itemCount = 0;
        [[maybe_unused]] int16_t itemDamage = 0;
        
        auto slotTag = std::dynamic_pointer_cast<ByteNbtTag>(slot->Get("Slot"));
        if (slotTag)
            slotNumber = slotTag->GetData();
        auto itemIdTag = std::dynamic_pointer_cast<ShortNbtTag>(slot->Get("id"));
        if (itemIdTag)
            itemId = itemIdTag->GetData();
        auto itemCountTag = std::dynamic_pointer_cast<ByteNbtTag>(slot->Get("Count"));
        if (itemCountTag)
            itemCount = itemCountTag->GetData();
        auto itemDamageTag = std::dynamic_pointer_cast<ShortNbtTag>(slot->Get("Damage"));
        if (itemDamageTag)
            itemDamage = itemDamageTag->GetData();

        Item newItem = Item{
            itemId,
            itemCount,
            itemDamage
        };

        if (slotNumber >= 100) {
            armorSlots.SetSlot(slotNumber, newItem);
        } else if (slotNumber <= 8) {
            hotbarSlots.SetSlot(slotNumber, newItem);
        } else {
            nbtInv.SetSlot(slotNumber - INVENTORY_MAIN_HOTBAR_COLS, newItem);
        }
    }
    inventory.SetLinearSlots(INVENTORY_MAIN_COLS, nbtInv.GetLinearSlots());
    return true;
}