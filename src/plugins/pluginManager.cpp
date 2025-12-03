#include "pluginManager.h"

/**
 * @brief Add and initialized the desired plugin
 * 
 * @param path The path of the plugin
 * @return Initialization succeeded
 */
bool PluginManager::AddPlugin(const std::string& path) {
    // Go through /scripts/plugins folder
	// Load plugin file and let it sit in it's own lua VM
	// Start Plugin script
    try {
        plugins.push_back(std::make_unique<Plugin>(path));
        return true;
    } catch (const std::exception &e) {
        Betrock::Logger::Instance().Error(e.what());
        return false;
    }
}

/**
 * @brief Remove a plugin
 * 
 * @param name The name of the plugin that should be removed
 * @return If removal succeeds 
 */
bool PluginManager::RemovePlugin(const std::string& name) {
    auto it = std::ranges::find_if(plugins,
        [&](const auto& p) { return p->GetName() == name; });

    // Plugin with that name doesn't exist
    if (it == plugins.end())
        return false;

    // Plugin gets erased
    plugins.erase(it);
    return true;
}

bool PluginManager::InformOfBlockBreak() {
    for (auto p : blockBreakHook) {
        p->BlockBreakHook();
    }
    return true;
}

bool PluginManager::SubscribeToBlockBreakHook() {
    return true;
}

bool PluginManager::InformOfBlockPlace() {
    for (auto p : blockPlaceHook) {
        p->BlockPlaceHook();
    }
    return true;
}

bool PluginManager::SubscribeToBlockPlaceHook() {
    return true;
}