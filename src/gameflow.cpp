#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/gl_text.h"
#include "core/console.h"
#include "engine.h"
#include "gameflow.h"
#include "engine.h"
#include "script.h"
#include "anim_state_control.h"
#include "world.h"

struct gameflow_manager_s gameflow_manager;

void Gameflow_Init()
{
    for(int i = 0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
    }
}

void Gameflow_Do()
{
    if(!gameflow_manager.NextAction)
        return;

    bool completed = true;

    for(int i = 0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        switch(gameflow_manager.Actions[i].opcode)
        {
            case TR_GAMEFLOW_OP_LEVELCOMPLETE:
                {
                    int top = lua_gettop(engine_lua);
                    lua_getglobal(engine_lua, "getNextLevel");                       // Must be loaded from gameflow script!
                    lua_pushnumber(engine_lua, gameflow_manager.CurrentGameID);      // Push the 1st argument
                    lua_pushnumber(engine_lua, gameflow_manager.CurrentLevelID);     // Push the 2nd argument
                    lua_pushnumber(engine_lua, gameflow_manager.Actions[i].operand); // Push the 3rd argument

                    if (lua_CallAndLog(engine_lua, 3, 3, 0))
                    {
                        gameflow_manager.CurrentLevelID = lua_tonumber(engine_lua, -1);   // First value in stack is level id
                        strncpy(gameflow_manager.CurrentLevelName, lua_tostring(engine_lua, -2), LEVEL_NAME_MAX_LEN); // Second value in stack is level name
                        strncpy(gameflow_manager.CurrentLevelPath, lua_tostring(engine_lua, -3), MAX_ENGINE_PATH); // Third value in stack is level path
                        Engine_LoadMap(gameflow_manager.CurrentLevelPath);
                    }
                    else
                    {
                        Con_AddLine("Fatal Error: Failed to call GetNextLevel()", FONTSTYLE_CONSOLE_WARNING);
                    }
                    lua_settop(engine_lua, top);
                    gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
                }
                break;

            case TR_GAMEFLOW_NOENTRY:
                continue;

            default:
                gameflow_manager.Actions[i].opcode = TR_GAMEFLOW_NOENTRY;
                break;  ///@FIXME: Implement all other gameflow opcodes here!
        };   // end switch(gameflow_manager.Operand)
        completed = false;
    }

    if(completed) gameflow_manager.NextAction = false;    // Reset action marker!
}

bool Gameflow_Send(int opcode, int operand)
{
    for(int i = 0; i < TR_GAMEFLOW_MAX_ACTIONS; i++)
    {
        if(gameflow_manager.Actions[i].opcode == opcode) return false;

        if(gameflow_manager.Actions[i].opcode == TR_GAMEFLOW_NOENTRY)
        {
            gameflow_manager.Actions[i].opcode  = opcode;
            gameflow_manager.Actions[i].operand = operand;
            gameflow_manager.NextAction         = true;
            return true;
        }
    }
    return false;
}
