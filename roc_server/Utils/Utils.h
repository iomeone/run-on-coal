#pragma once

namespace ROC
{
namespace Utils
{

namespace Path
{

void EscapePath(std::string &f_path);

}

namespace Enum
{

int ReadEnumVector(const std::vector<std::string> &f_vec, const std::string &f_val);

}

namespace Lua
{

void lua_registerClass(lua_State *f_vm, const char *f_name, lua_CFunction f_func);
void lua_registerClassMethod(lua_State *f_vm, const char *f_name, lua_CFunction f_func);
inline void lua_registerClassFinish(lua_State *f_vm) { lua_pop(f_vm, 1); }

}

namespace Network
{

unsigned char GetPacketIdentifier(RakNet::Packet *f_packet);

}

}
}
