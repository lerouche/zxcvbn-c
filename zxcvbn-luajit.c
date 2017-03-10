#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zxcvbn.h"

static int zxcvbn(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        return lua_error(L, "The first argument must be a string");
    }

    if (!lua_istable(L, 2)) {
        return lua_error(L, "The second argument must be a table with consecutive integer indexes starting from 1");
    }

	const char* pwd = lua_tostring(L, 1);
	const size_t userDictLen = lua_objlen(L, 2);
	const char **userDict;
	userDict = malloc((userDictLen + 1) * sizeof(char*));
	for (size_t i = 0; i < userDictLen; i++) {
		lua_pushinteger(L, i + 1);
		lua_gettable(L, 2);
		size_t userDictEntryLen;
		char* userDictEntry;
		const char* stackEntry = lua_tolstring(L, -1, &userDictEntryLen);
		userDictEntry = calloc(userDictEntryLen + 1, sizeof(char)); // Ensure null-terminated by allocating extra byte and writing all bytes over with zero
		strncpy(userDictEntry, stackEntry, userDictEntryLen);
		lua_pop(L, 1); // Pop after copying, as Lua-provided char pointer is only valid while stack doesn't change
		userDict[i] = userDictEntry;
	}
	userDict[userDictLen] = NULL;
	lua_pushnumber(L, ZxcvbnMatch(pwd, userDict, 0));
	for (size_t i = 0; i < userDictLen; i++) {
		free((void *)userDict[i]);
	}
	free((void *)userDict);
	return 1;
}

static const struct luaL_Reg zxcvbnlib [] = {
	{"getEntropy", zxcvbn},
	{NULL, NULL}
};

int luaopen_zxcvbn(lua_State *L) {
	luaL_register(L, "zxcvbn", zxcvbnlib);
	return 1;
}
