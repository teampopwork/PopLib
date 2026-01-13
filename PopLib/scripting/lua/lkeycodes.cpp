#include "lpoplib.hpp"
#include "misc/keycodes.hpp"

using namespace PopLib;

void open_keycodes(sol::state_view lua)
{
	sol::table poplib;
	if (lua["PopLib"].valid() && lua["PopLib"].get_type() == sol::type::table)
		poplib = lua["PopLib"];
	else
	{
		poplib = lua.create_table();
		lua["PopLib"] = poplib;
	}

	sol::table keycode = lua.create_table();
	keycode["UNKNOWN"] = KEYCODE_UNKNOWN;
	keycode["RETURN"] = KEYCODE_RETURN;
	keycode["BACK"] = KEYCODE_BACK;
	keycode["TAB"] = KEYCODE_TAB;
	keycode["SPACE"] = KEYCODE_SPACE;
	keycode["ESCAPE"] = KEYCODE_ESCAPE;
	keycode["LEFT"] = KEYCODE_LEFT;
	keycode["RIGHT"] = KEYCODE_RIGHT;
	keycode["UP"] = KEYCODE_UP;
	keycode["DOWN"] = KEYCODE_DOWN;
	keycode["SHIFT"] = KEYCODE_SHIFT;
	keycode["CONTROL"] = KEYCODE_CONTROL;
	keycode["ALT"] = KEYCODE_MENU;
	keycode["DELETE"] = KEYCODE_DELETE;
	keycode["INSERT"] = KEYCODE_INSERT;
	keycode["HOME"] = KEYCODE_HOME;
	keycode["END"] = KEYCODE_END;

	// F1–F12
	for (int i = 1; i <= 12; ++i)
	{
		keycode["F" + std::to_string(i)] = KEYCODE_F1 + (i - 1);
	}

	poplib["KeyCode"] = keycode;

	poplib.set_function("GetKeyCodeFromName", &GetKeyCodeFromName);
	poplib.set_function("GetKeyNameFromCode", &GetKeyNameFromCode);
}