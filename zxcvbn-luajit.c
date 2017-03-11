#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zxcvbn.h"

static int zxcvbn(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "The first argument must be a string");
    }

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "The second argument must be a table with consecutive integer indexes starting from 1");
    }

    // Get the first argument value (the password string)
    size_t passwordLength;
    const char* pwdOnStack = lua_tolstring(L, 1, &passwordLength);

    // Ensure null-terminated by allocating extra byte and writing null to the last byte
    char *password = malloc((passwordLength + 1) * sizeof(char));
    password[passwordLength] = '\0';

    // Convert null bytes in the password to ASCII 1 as null-termination is required by zxcvbn.c
    for (size_t pwCharNo = 0; pwCharNo < passwordLength; pwCharNo++) {
        if (pwdOnStack[pwCharNo] == '\0') {
            password[pwCharNo] = '\1';
        } else {
            password[pwCharNo] = pwdOnStack[pwCharNo];
        }
    }

    // Get the size and value of the second argument (the user dictionary table)
    const int userDictLen = lua_objlen(L, 2);
    const char **userDict = malloc((userDictLen + 1) * sizeof(char*));

    for (int i = 0; i < userDictLen; i++) {
        // Put the dictionary entry at index i + 1 onto the stack
        lua_pushinteger(L, i + 1);
        lua_gettable(L, 2);

        // Get length and value from the stack
        size_t userDictEntryLen;
        const char *stackEntry = lua_tolstring(L, -1, &userDictEntryLen);

        // Ensure null-terminated by allocating extra byte and writing null to the last byte
        char *userDictEntry = malloc((userDictEntryLen + 1) * sizeof(char));
        userDictEntry[userDictEntryLen] = '\0';

        // Convert null bytes in the entry to ASCII 1 as null-termination is required by zxcvbn.c
        for (size_t entryCharNo = 0; entryCharNo < userDictEntryLen; entryCharNo++) {
            if (stackEntry[entryCharNo] == '\0') {
                userDictEntry[entryCharNo] = '\1';
            } else {
                userDictEntry[entryCharNo] = stackEntry[entryCharNo];
            }
        }

        // Pop after copying, as Lua-provided char pointer is only valid while stack doesn't change
        lua_pop(L, 1);
        userDict[i] = userDictEntry;
    }

    // End user dictionary with NULL as required by zxcvbn.c
    userDict[userDictLen] = NULL;

    // Calculate the entropy and return to Lua
    lua_pushnumber(L, ZxcvbnMatch(password, userDict, NULL));

    // Free memory
    for (int i = 0; i < userDictLen; i++) {
        free((void *) userDict[i]);
    }
    free((void *) userDict);

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
