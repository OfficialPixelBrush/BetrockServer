#pragma once

#include "plugin.h"
#include <vector>

/**
 * @brief Manages Plugins
 * 
 */
class PluginManager {
    private:
        std::vector<std::unique_ptr<Plugin>> plugins;
        std::vector<Plugin*> blockPlaceHook;
        std::vector<Plugin*> blockBreakHook;
    public:
        bool AddPlugin(const std::string& path);
        bool RemovePlugin(const std::string& name);
        
        // Subscribe to hook
        bool SubscribeToBlockBreakHook();
        bool SubscribeToPlaceBreakHook();

        // Inform of hooked event
        bool InformOfBlockBreak();
        bool InformOfBlockPlace();
};