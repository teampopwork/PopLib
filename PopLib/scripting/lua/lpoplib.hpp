#ifndef LPOPLIB_HPP
#define LPOPLIB_HPP

#pragma once

#include <sol/sol.hpp>
#include "../scripting_base.hpp"

void(open_rect)(sol::state_view lua);

void(pop_openlibs)(sol::state &lua);

#endif