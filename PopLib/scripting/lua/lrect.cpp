#include "lpoplib.hpp"
#include "math/rect.hpp"

using namespace PopLib;

void open_rect(sol::state_view lua)
{
	sol::table poplib;
	if (lua["PopLib"].valid() && lua["PopLib"].get_type() == sol::type::table)
		poplib = lua["PopLib"];
	else
	{
		poplib = lua.create_table();
		lua["PopLib"] = poplib;
	}

	poplib.new_usertype<PopLib::Rect>(
		"Rect",
		sol::constructors<PopLib::Rect(), PopLib::Rect(int, int, int, int), PopLib::Rect(const PopLib::Rect &)>(), "mX",
		&PopLib::Rect::mX, "mY", &PopLib::Rect::mY, "mWidth", &PopLib::Rect::mWidth, "mHeight", &PopLib::Rect::mHeight,

		"Intersects", &PopLib::Rect::Intersects, "Intersection", &PopLib::Rect::Intersection, "Union",
		static_cast<PopLib::Rect (PopLib::Rect::*)(const PopLib::Rect &)>(&PopLib::Rect::Union), "Contains",
		sol::overload(static_cast<bool (PopLib::Rect::*)(int, int) const>(&PopLib::Rect::Contains),
					  static_cast<bool (PopLib::Rect::*)(const PopLib::TPoint<int> &) const>(&PopLib::Rect::Contains)),
		"Offset",
		sol::overload(static_cast<void (PopLib::Rect::*)(int, int)>(&PopLib::Rect::Offset),
					  static_cast<void (PopLib::Rect::*)(const PopLib::TPoint<int> &)>(&PopLib::Rect::Offset)),
		"Inflate", &PopLib::Rect::Inflate, sol::meta_function::to_string,
		[](const PopLib::Rect &r) {
			return "Rect(" + std::to_string(r.mX) + "," + std::to_string(r.mY) + "," + std::to_string(r.mWidth) + "," +
				   std::to_string(r.mHeight) + ")";
		},
		sol::meta_function::equal_to, &PopLib::Rect::operator==);

	poplib.new_usertype<PopLib::FRect>(
		"FRect",
		sol::constructors<PopLib::FRect(), PopLib::FRect(double, double, double, double),
						  PopLib::FRect(const PopLib::FRect &)>(),
		"mX", &PopLib::FRect::mX, "mY", &PopLib::FRect::mY, "mWidth", &PopLib::FRect::mWidth, "mHeight",
		&PopLib::FRect::mHeight,

		"Intersects", &PopLib::FRect::Intersects, "Intersection", &PopLib::FRect::Intersection, "Union",
		static_cast<PopLib::FRect (PopLib::FRect::*)(const PopLib::FRect &)>(&PopLib::FRect::Union), "Contains",
		sol::overload(
			static_cast<bool (PopLib::FRect::*)(double, double) const>(&PopLib::FRect::Contains),
			static_cast<bool (PopLib::FRect::*)(const PopLib::TPoint<double> &) const>(&PopLib::FRect::Contains)),
		"Offset",
		sol::overload(static_cast<void (PopLib::FRect::*)(double, double)>(&PopLib::FRect::Offset),
					  static_cast<void (PopLib::FRect::*)(const PopLib::TPoint<double> &)>(&PopLib::FRect::Offset)),
		"Inflate", &PopLib::FRect::Inflate, sol::meta_function::to_string,
		[](const PopLib::FRect &r) {
			return "FRect(" + std::to_string(r.mX) + "," + std::to_string(r.mY) + "," + std::to_string(r.mWidth) + "," +
				   std::to_string(r.mHeight) + ")";
		},
		sol::meta_function::equal_to, &PopLib::FRect::operator==);
}