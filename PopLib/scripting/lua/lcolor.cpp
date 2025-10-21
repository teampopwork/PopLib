#include "lpoplib.hpp"
#include "graphics/color.hpp"

using namespace PopLib;

void open_color(sol::state_view lua)
{
	sol::table poplib;
	if (lua["PopLib"].valid() && lua["PopLib"].get_type() == sol::type::table)
		poplib = lua["PopLib"];
	else
	{
		poplib = lua.create_table();
		lua["PopLib"] = poplib;
	}

	poplib.new_usertype<PopLib::Color>(
		"Color",
		sol::constructors<PopLib::Color(), PopLib::Color(int), PopLib::Color(int, int), PopLib::Color(int, int, int),
						  PopLib::Color(int, int, int, int)>(),

		"GetRed", &PopLib::Color::GetRed, "GetGreen", &PopLib::Color::GetGreen, "GetBlue", &PopLib::Color::GetBlue,
		"GetAlpha", &PopLib::Color::GetAlpha, "ToInt", &PopLib::Color::ToInt,

		"mRed", &PopLib::Color::mRed, "mGreen", &PopLib::Color::mGreen, "mBlue", &PopLib::Color::mBlue, "mAlpha",
		&PopLib::Color::mAlpha);

	poplib["Color"]["Black"] = PopLib::Color::Black;
	poplib["Color"]["White"] = PopLib::Color::White;
}