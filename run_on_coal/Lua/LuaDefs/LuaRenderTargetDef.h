#pragma once

namespace ROC
{

class LuaRenderTargetDef final
{
    static int RenderTargetCreate(lua_State *f_vm);
protected:
    static void Init(lua_State *f_vm);

    friend class LuaManager;
};

}
