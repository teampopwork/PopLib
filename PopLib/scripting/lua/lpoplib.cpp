#include "lpoplib.hpp"

void pop_openlibs(sol::state &lua)
{
	open_rect(lua);
	open_color(lua);
	open_keycodes(lua);
	open_point(lua);
	open_matrix(lua);
}