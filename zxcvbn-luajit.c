#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "zxcvbn.h"

static int zxcvbn(lua_State *L, int calcScore) {
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
    double calculatedEntropy = ZxcvbnMatch(password, userDict, NULL);
    if (calcScore) {
        double lhs = pow(2, calculatedEntropy) * 0.00005;
        int calculatedScore;
        if (lhs < 100) {
            calculatedScore = 0;
        } else if (lhs < 10000) {
            calculatedScore = 1;
        } else if (lhs < 1000000) {
            calculatedScore = 2;
        } else if (lhs < 100000000) {
            calculatedScore = 3;
        } else {
            calculatedScore = 4;
        }
        lua_pushnumber(L, calculatedScore);
    } else {
        lua_pushnumber(L, calculatedEntropy);
    }

    // Free memory
    for (int i = 0; i < userDictLen; i++) {
        free((void *) userDict[i]);
    }
    free((void *) userDict);

    return 1;
}

static int zxcvbnlib_getEntropy(lua_State *L) {
    return zxcvbn(L, 0);
}

static int zxcvbnlib_getScore(lua_State *L) {
    return zxcvbn(L, 1);
}

static const struct luaL_Reg zxcvbnlib [] = {
    {"getEntropy", zxcvbnlib_getEntropy},
    {"getScore", zxcvbnlib_getScore},
    {NULL, NULL}
};

int luaopen_zxcvbn(lua_State *L) {
    luaL_register(L, "zxcvbn", zxcvbnlib);
    return 1;
}
