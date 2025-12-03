#include "coms.h"
#include "server.h"
#include "plugin.h"

/**
 * @brief Sends a chat message to all clients
 * 
 * @param L 
 * @return int 
 */
int Plugin::lua_GlobalChat(lua_State *L) {
	std::vector<uint8_t> response;
	std::string message = lua_tostring(L, 1);
	Respond::ChatMessage(response, message);
	BroadcastToClients(response);
	return 1;
}

/**
 * @brief Gets the usernames of all connected players
 * 
 * @param L 
 * @return int 
 */
int Plugin::lua_GetPlayerList(lua_State *L) {
	auto clients = Betrock::Server::Instance().GetConnectedClients();
	lua_newtable(L);
	int index = 1;
	for (auto client : clients) {
		lua_pushstring(L, client->GetUsername().c_str() );
		lua_rawseti(L, -2,  index++);
	}
	return 1;
}

/**
 * @brief Gets more info on the passed player
 * 
 * @param L 
 * @return int 
 */
int Plugin::lua_GetPlayer([[maybe_unused]] lua_State *L) {
	//auto clients = Betrock::Server::Instance().FindClientByUsername();
	return 1;
}

/**
 * @brief Gets the type and meta values of the desired block
 * 
 * @param L 
 * @return int 
 */
int Plugin::lua_GetBlock(lua_State *L) {
	if (!CheckNum3(L)) {
		lua_pushnil(L);
		return 1;
	}
	// Get position
	int x = (int)lua_tointeger(L, 1);
	int y = (int)lua_tointeger(L, 2);
	int z = (int)lua_tointeger(L, 3);
	Int3 pos(x,y,z);

	// Get world
	auto world = Betrock::Server::Instance().GetWorld(0);

	// Push block info
	lua_newtable(L);
	lua_pushinteger(L, world->GetBlockType(pos));
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, world->GetBlockMeta(pos));
	lua_rawseti(L, -2, 2);
	return 1;
}

/**
 * @brief Sets the type and meta values of the desired block
 * 
 * @param L 
 * @return int 
 */
int Plugin::lua_SetBlock(lua_State *L) {
	if (!CheckNum3(L)) {
		lua_pushnil(L);
		return 1;
	}
	// Get position
	int x = (int)lua_tointeger(L, 1);
	int y = (int)lua_tointeger(L, 2);
	int z = (int)lua_tointeger(L, 3);
	Int3 pos(x,y,z);

	Block b = DecodeBlock(L);
	
	// Get world
	auto world = Betrock::Server::Instance().GetWorld(0);

	world->PlaceBlock(pos, b.type, b.meta);
	return 1;
}