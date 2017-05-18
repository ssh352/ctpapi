#include "caf/all.hpp"
#include <iostream>
#include <boost/lexical_cast.hpp>

extern "C" {  
#include "lua.h"  
#include "lualib.h"  
#include "lauxlib.h"  
}  


static int caf_send(lua_State* L) {
  
  caf::actor_id id = boost::lexical_cast<caf::actor_id>(lua_tostring(L, 1));
  // caf::actor self = caf::actor_cast<caf::actor>();  
  return 0;
}

caf::behavior foo(caf::event_based_actor* self) {
  return {
    [](int i) {
    std::cout << i << std::endl;
}

  };
}



int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  auto actor = system.spawn(foo);
  /*
  system.registry().put(caf::atom("foo"), caf::actor_cast<caf::strong_actor_ptr>(actor));

  auto self = system.registry().get(caf::atom("foo"));

  caf::anon_send(caf::actor_cast<caf::actor>(self), 100);
  
  */

    lua_State *L = luaL_newstate();  
    if (L == NULL)  
    {  
        return 0;  
    }  
    luaL_openlibs(L);  
    lua_register(L, "caf_send", caf_send);  
  
    int bRet = luaL_loadfile(L,"hello.lua");  
    if(bRet)  
    {  
        std::cout<<"load file error"<<std::endl;  
        return 0;  
    }  

    bRet = lua_pcall(L,0,0,0);  
    if(bRet)  
    {  
        std::cout<<"pcall error"<<std::endl;  
        return 0;  
    }  
  
    lua_getglobal(L, "send");
    lua_pushinteger(L, actor->address());
    int iRet= lua_pcall(L, 1, 0, 0);
    if (iRet)                      
    {  
        const char *pErrorMsg = lua_tostring(L, -1);  
        std::cout << pErrorMsg << std::endl;  
        lua_close(L);  
        return 0;  
    }  

  std::string input;
  while (std::cin >> input) {
    break;
  }

  system.registry().erase(caf::atom("foo"));

  return 1;
}

CAF_MAIN()
