#include "lpoplib.hpp"
#include "math/point.hpp"

using namespace PopLib;

template <typename T> struct TPointWrapper : public TPoint<T>
{
	using TPoint<T>::TPoint; // inherit constructors

	TPointWrapper() : TPoint<T>()
	{
	}
	TPointWrapper(T x, T y) : TPoint<T>(x, y)
	{
	}
	TPointWrapper(const TPoint<T> &p) : TPoint<T>(p)
	{
	}

	TPointWrapper operator+(const TPointWrapper &other) const
	{
		return TPointWrapper(this->mX + other.mX, this->mY + other.mY);
	}

	TPointWrapper operator-(const TPointWrapper &other) const
	{
		return TPointWrapper(this->mX - other.mX, this->mY - other.mY);
	}

	TPointWrapper operator*(const TPointWrapper &other) const
	{
		return TPointWrapper(this->mX * other.mX, this->mY * other.mY);
	}

	TPointWrapper operator/(const TPointWrapper &other) const
	{
		return TPointWrapper(this->mX / other.mX, this->mY / other.mY);
	}

	TPointWrapper operator*(T s) const
	{
		return TPointWrapper(this->mX * s, this->mY * s);
	}

	TPointWrapper operator/(T s) const
	{
		return TPointWrapper(this->mX / s, this->mY / s);
	}

	bool operator==(const TPointWrapper &other) const
	{
		return this->mX == other.mX && this->mY == other.mY;
	}
};

void open_point(sol::state_view lua)
{
	sol::table poplib;
	if (lua["PopLib"].valid() && lua["PopLib"].get_type() == sol::type::table)
	{
		poplib = lua["PopLib"];
	}
	else
	{
		poplib = lua.create_table();
		lua["PopLib"] = poplib;
	}

	poplib.new_usertype<TPointWrapper<int>>(
		"Point",
		sol::constructors<TPointWrapper<int>(), TPointWrapper<int>(int, int),
						  TPointWrapper<int>(const TPoint<int> &)>(),
		"mX", &TPointWrapper<int>::mX, "mY", &TPointWrapper<int>::mY, sol::meta_function::equal_to,
		&TPointWrapper<int>::operator==, sol::meta_function::addition, &TPointWrapper<int>::operator+,
		sol::meta_function::subtraction, &TPointWrapper<int>::operator-, sol::meta_function::multiplication,
		sol::overload(
			static_cast<TPointWrapper<int> (TPointWrapper<int>::*)(const TPointWrapper<int> &) const>(
				&TPointWrapper<int>::operator*),
			static_cast<TPointWrapper<int> (TPointWrapper<int>::*)(int) const>(&TPointWrapper<int>::operator*)),
		sol::meta_function::division,
		sol::overload(
			static_cast<TPointWrapper<int> (TPointWrapper<int>::*)(const TPointWrapper<int> &) const>(
				&TPointWrapper<int>::operator/),
			static_cast<TPointWrapper<int> (TPointWrapper<int>::*)(int) const>(&TPointWrapper<int>::operator/)),
		sol::meta_function::to_string, [](const TPointWrapper<int> &p) {
			return "Point(" + std::to_string(p.mX) + ", " + std::to_string(p.mY) + ")";
		});

	poplib.new_usertype<TPointWrapper<double>>(
		"FPoint",
		sol::constructors<TPointWrapper<double>(), TPointWrapper<double>(double, double),
						  TPointWrapper<double>(const TPoint<double> &)>(),
		"mX", &TPointWrapper<double>::mX, "mY", &TPointWrapper<double>::mY, sol::meta_function::equal_to,
		&TPointWrapper<double>::operator==, sol::meta_function::addition, &TPointWrapper<double>::operator+,
		sol::meta_function::subtraction, &TPointWrapper<double>::operator-, sol::meta_function::multiplication,
		sol::overload(
			static_cast<TPointWrapper<double> (TPointWrapper<double>::*)(const TPointWrapper<double> &) const>(
				&TPointWrapper<double>::operator*),
			static_cast<TPointWrapper<double> (TPointWrapper<double>::*)(double) const>(
				&TPointWrapper<double>::operator*)),
		sol::meta_function::division,
		sol::overload(
			static_cast<TPointWrapper<double> (TPointWrapper<double>::*)(const TPointWrapper<double> &) const>(
				&TPointWrapper<double>::operator/),
			static_cast<TPointWrapper<double> (TPointWrapper<double>::*)(double) const>(
				&TPointWrapper<double>::operator/)),
		sol::meta_function::to_string, [](const TPointWrapper<double> &p) {
			return "FPoint(" + std::to_string(p.mX) + ", " + std::to_string(p.mY) + ")";
		});
}