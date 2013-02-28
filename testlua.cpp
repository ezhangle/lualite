#include <cstdlib>

#include <iostream>

extern "C" {

#include "lua/lualib.h"

}

#include "lualite/lualite.hpp"

struct point
{
  int x;
  int y;
};

namespace lualite
{

namespace detail
{

inline void set_result(lua_State* const L,
  point && point)
{
  lua_createtable(L, 2, 0);

  lua_pushliteral(L, "x");
  set_result(L, point.x);

  lua_pushliteral(L, "y");
  set_result(L, point.y);

  lua_rawset(L, -3);
}

template <std::size_t I>
inline point get_arg(lua_State* const L,
  point const)
{
  assert(lua_istable(L, I));

  struct point p;

  lua_pushliteral(L, "x");
  lua_rawget(L, -2);
  p.x = lua_tointeger(L, -1);

  lua_pushliteral(L, "y");
  lua_rawget(L, -2);
  p.y = lua_tointeger(L, -1);

  return p;
}

}

}

std::map<std::string, int> testfunc(int i)
{
  std::cout << "testfunc(): " << i << std::endl;

  return std::map<std::string, int>{{"bla", 4}};
}

struct testclass
{
  testclass()
  {
    std::cout << "testclass::testclass()" << std::endl;
  }

  testclass(int i)
  {
    std::cout << "testclass::testclass(int):" << i << std::endl;
  }

  std::vector<std::string> print() const
  {
    std::cout << "hello world!" << std::endl;

    return std::vector<std::string>(10, "bla!!!");
  }

  void print(int i)
  {
    std::cout << i << std::endl;
  }
};

int main(int argc, char* argv[])
{
  lua_State* L(luaL_newstate());

  luaL_openlibs(L);

  lualite::module(L,
    lualite::class_<testclass>("testclass")
      .constructor<int>()
      .enum_("smell", 9)
      .def("print", (void (testclass::*)(int))&testclass::print)
      .def("print_", (std::vector<std::string> (testclass::*)() const)&testclass::print),
    lualite::scope("subscope",
      lualite::class_<testclass>("testclass")
        .constructor<int>()
        .enum_("smell", 10)
        .def("testfunc", &testfunc)
        .def("print", (void (testclass::*)(int))&testclass::print)
        .def("print_", (std::vector<std::string> (testclass::*)() const)&testclass::print)
    )
  )
  .enum_("apple", 1)
  .def("testfunc", &testfunc);

  luaL_dostring(
    L,
    "local a = testfunc(3)\n"
    "print(a[\"bla\"])\n"
    "print(apple)\n"
    "print(testclass.__classname)\n"
    "print(testclass.smell)\n"
    "local b = testclass.new(1000)\n"
    "b:print(100)\n"
    "b:print_()\n"
    "local a = subscope.testclass.new(1111)\n"
    "print(subscope.testclass.smell)\n"
    "subscope.testclass.testfunc(200)\n"
    "local c = a:print_()\n"
    "print(c[10])\n"
  );

  lua_close(L);

  return EXIT_SUCCESS;
}
