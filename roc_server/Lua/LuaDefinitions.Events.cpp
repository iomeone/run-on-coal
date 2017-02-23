#include "stdafx.h"
#include "Core/Core.h"
#include "Managers/EventManager.h"
#include "Managers/LuaManager.h"
#include "Lua/ArgReader.h"
#include "Lua/LuaDefinitions.Events.h"
#include "Utils/Utils.h"

namespace ROC
{
namespace Lua
{

const std::vector<std::string> g_eventNamesTable
{
    "onServerStart", "onServerStop", "onServerPulse",
    "onNetworkClientConnect", "onNetworkClientDisconnect","onNetworkDataRecieve"
};

int addEvent(lua_State *f_vm)
{
    std::string l_event;
    int l_func;
    void *l_point;
    ArgReader argStream(f_vm);
    argStream.ReadText(l_event);
    argStream.ReadFunction(l_func, &l_point);
    if(!argStream.HasErrors() && !l_event.empty())
    {
        int l_enumValue = Utils::ReadEnumVector(g_eventNamesTable, l_event);
        if(l_enumValue != -1)
        {
            EventType l_eventEnum = static_cast<EventType>(l_enumValue);
            bool l_result = LuaManager::GetCore()->GetLuaManager()->GetEventManager()->AddEvent(l_eventEnum, l_func, l_point);
            argStream.PushBoolean(l_result);
        }
        else argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int setEventMute(lua_State *f_vm)
{
    std::string l_event;
    void *l_point;
    bool l_mute;
    ArgReader argStream(f_vm);
    argStream.ReadText(l_event);
    argStream.ReadFunctionPointer(&l_point);
    argStream.ReadBoolean(l_mute);
    if(!argStream.HasErrors() && !l_event.empty())
    {
        int l_enumValue = Utils::ReadEnumVector(g_eventNamesTable, l_event);
        if(l_enumValue != -1)
        {
            EventType l_eventEnum = static_cast<EventType>(l_enumValue);
            bool l_result = LuaManager::GetCore()->GetLuaManager()->GetEventManager()->SetEventMute(l_eventEnum, l_point, l_mute);
            argStream.PushBoolean(l_result);
        }
        else argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int removeEvent(lua_State *f_vm)
{
    std::string l_event;
    void *l_point;
    ArgReader argStream(f_vm);
    argStream.ReadText(l_event);
    argStream.ReadFunctionPointer(&l_point);
    if(!argStream.HasErrors() && !l_event.empty())
    {
        int l_enumValue = Utils::ReadEnumVector(g_eventNamesTable, l_event);
        if(l_enumValue != -1)
        {
            EventType l_eventEnum = static_cast<EventType>(l_enumValue);
            bool l_result = LuaManager::GetCore()->GetLuaManager()->GetEventManager()->RemoveEvent(l_eventEnum, l_point);
            argStream.PushBoolean(l_result);
        }
        else argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

}
}
