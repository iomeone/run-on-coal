#include "stdafx.h"
#include "Core/Core.h"
#include "Managers/ElementManager.h"
#include "Managers/LuaManager.h"
#include "Managers/MemoryManager.h"
#include "Managers/SoundManager.h"
#include "Elements/Sound.h"
#include "Lua/ArgReader.h"
#include "Lua/LuaDefinitions.Sound.h"
#include "Utils/Utils.h"

namespace ROC
{
namespace Lua
{

const std::string g_soundStatesTable[] = {
    "stopped", "paused", "playing"
};

int soundCreate(lua_State *f_vm)
{
    std::string l_path;
    bool l_loop = false;
    ArgReader argStream(f_vm);
    argStream.ReadText(l_path);
    argStream.ReadNextBoolean(l_loop);
    if(!argStream.HasErrors() && !l_path.empty())
    {
        Sound *l_sound = LuaManager::GetCore()->GetElementManager()->CreateSound(l_path, l_loop);
        l_sound ? argStream.PushPointer(l_sound) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundPlay(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        l_sound->Play();
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundPause(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        l_sound->Pause();
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundStop(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        l_sound->Stop();
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundIsLooped(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.PushBoolean(!argStream.HasErrors() ? l_sound->IsLooped() : false);
    return argStream.GetReturnValue();
}

int soundGetState(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        int l_state = l_sound->GetState();
        (l_state != -1) ? argStream.PushText(g_soundStatesTable[l_state]) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSetSpeed(lua_State *f_vm)
{
    Sound *l_sound;
    float l_val;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.ReadNumber(l_val);
    if(!argStream.HasErrors())
    {
        l_sound->SetSpeed(l_val);
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGetSpeed(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        float l_speed = l_sound->GetSpeed();
        (l_speed != -1.f) ? argStream.PushNumber(l_speed) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSetVolume(lua_State *f_vm)
{
    Sound *l_sound;
    float l_val;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.ReadNumber(l_val);
    if(!argStream.HasErrors())
    {
        l_sound->SetVolume(l_val);
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGetVolume(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        float l_volume = l_sound->GetVolume();
        (l_volume != -1.f) ? argStream.PushNumber(l_volume) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSetTime(lua_State *f_vm)
{
    Sound *l_sound;
    float l_val;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.ReadNumber(l_val);
    if(!argStream.HasErrors())
    {
        l_sound->SetTime(l_val);
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGetTime(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        float l_time = l_sound->GetTime();
        (l_time != -1.f) ? argStream.PushNumber(l_time) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGetDuration(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(argStream.HasErrors())
    {
        float l_duration = l_sound->GetDuration();
        (l_duration != -1.f) ? argStream.PushNumber(l_duration) : argStream.PushBoolean(false);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSet3DEnabled(lua_State *f_vm)
{
    Sound *l_sound;
    bool l_state;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.ReadBoolean(l_state);
    if(!argStream.HasErrors())
    {
        bool l_result = l_sound->Set3DPositionEnabled(l_state);
        argStream.PushBoolean(l_result);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGet3DEnabled(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    argStream.PushBoolean(!argStream.HasErrors() ? l_sound->Get3DPositionEnabled() : false);
    return argStream.GetReturnValue();
}

int soundSet3DPosition(lua_State *f_vm)
{
    Sound *l_sound;
    glm::vec3 l_pos;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    for(int i = 0; i < 3; i++) argStream.ReadNumber(l_pos[i]);
    if(!argStream.HasErrors())
    {
        bool l_result = l_sound->Set3DPosition(l_pos);
        argStream.PushBoolean(l_result);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGet3DPosition(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        glm::vec3 l_pos(0.f);
        l_sound->Get3DPosition(l_pos);
        argStream.PushNumber(l_pos.x);
        argStream.PushNumber(l_pos.y);
        argStream.PushNumber(l_pos.z);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSet3DDistance(lua_State *f_vm)
{
    Sound *l_sound;
    glm::vec2 l_dist;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    for(int i = 0; i < 2; i++) argStream.ReadNumber(l_dist[i]);
    if(!argStream.HasErrors())
    {
        bool l_result = l_sound->Set3DDistance(l_dist);
        argStream.PushBoolean(l_result);
    }
    else argStream.PushBoolean(false);
    return 1;
}
int soundGet3DDistance(lua_State *f_vm)
{
    Sound *l_sound;
    ArgReader argStream(f_vm);
    argStream.ReadElement(l_sound);
    if(!argStream.HasErrors())
    {
        glm::vec2 l_dist(0.f);
        l_sound->Get3DDistance(l_dist);
        argStream.PushNumber(l_dist.x);
        argStream.PushNumber(l_dist.y);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}

int soundSetListenerOrientation(lua_State *f_vm)
{
    glm::vec3 l_pos, l_dir, l_up;
    ArgReader argStream(f_vm);
    for(int i = 0; i < 3; i++) argStream.ReadNumber(l_pos[i]);
    for(int i = 0; i < 3; i++) argStream.ReadNumber(l_dir[i]);
    for(int i = 0; i < 3; i++) argStream.ReadNumber(l_up[i]);
    if(!argStream.HasErrors())
    {
        LuaManager::GetCore()->GetSoundManager()->SetListenerPosition(l_pos);
        LuaManager::GetCore()->GetSoundManager()->SetListenerDirection(l_dir);
        LuaManager::GetCore()->GetSoundManager()->SetListenerUp(l_up);
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return 1;
}

int soundGetListenerOrientation(lua_State *f_vm)
{
    ArgReader argStream(f_vm);
    glm::vec3 l_pos, l_dir, l_up;
    LuaManager::GetCore()->GetSoundManager()->GetListenerPosition(l_pos);
    argStream.PushNumber(l_pos.x);
    argStream.PushNumber(l_pos.y);
    argStream.PushNumber(l_pos.z);
    LuaManager::GetCore()->GetSoundManager()->GetListenerDirection(l_dir);
    argStream.PushNumber(l_dir.x);
    argStream.PushNumber(l_dir.y);
    argStream.PushNumber(l_dir.z);
    LuaManager::GetCore()->GetSoundManager()->GetListenerUp(l_up);
    argStream.PushNumber(l_up.x);
    argStream.PushNumber(l_up.y);
    argStream.PushNumber(l_up.z);
    return argStream.GetReturnValue();
}

int soundSetGlobalVolume(lua_State *f_vm)
{
    float l_volume;
    ArgReader argStream(f_vm);
    argStream.ReadNumber(l_volume);
    if(!argStream.HasErrors())
    {
        LuaManager::GetCore()->GetSoundManager()->SetGlobalVolume(l_volume);
        argStream.PushBoolean(true);
    }
    else argStream.PushBoolean(false);
    return argStream.GetReturnValue();
}
int soundGetGlobalVolume(lua_State *f_vm)
{
    ArgReader argStream(f_vm);
    argStream.PushNumber(LuaManager::GetCore()->GetSoundManager()->GetGlobalVolume());
    return argStream.GetReturnValue();
}

}
}
