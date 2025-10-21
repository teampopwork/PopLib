#include "lpoplib.hpp"
#include "math/matrix.hpp"

using namespace PopLib;

void open_matrix(sol::state_view lua)
{
	sol::table poplib;
	if (lua["PopLib"].valid() && lua["PopLib"].get_type() == sol::type::table)
		poplib = lua["PopLib"];
	else
	{
		poplib = lua.create_table();
		lua["PopLib"] = poplib;
	}

	poplib.new_usertype<PopLib::Matrix3>(
		"Matrix3", sol::constructors<PopLib::Matrix3()>(), "ZeroMatrix", &PopLib::Matrix3::ZeroMatrix,
		"LoadIdentity", &PopLib::Matrix3::LoadIdentity, sol::meta_function::multiplication,
		sol::resolve<PopLib::Matrix3(const PopLib::Matrix3 &) const>(&PopLib::Matrix3::operator*),
		sol::meta_function::to_string, [](const PopLib::Matrix3 &m) { return "Matrix3(...)"; });

	poplib.new_usertype<PopLib::Transform2D>(
		"Transform2D",
		sol::constructors<PopLib::Transform2D(), PopLib::Transform2D(bool),
						  PopLib::Transform2D(const PopLib::Matrix3 &)>(),
		sol::base_classes, sol::bases<PopLib::Matrix3>(),

		"Translate", &PopLib::Transform2D::Translate, "RotateRad", &PopLib::Transform2D::RotateRad,
		"RotateDeg", &PopLib::Transform2D::RotateDeg, "Scale", &PopLib::Transform2D::Scale);

	poplib.new_usertype<PopLib::Transform>(
		"Transform", sol::constructors<PopLib::Transform()>(), "Reset", &PopLib::Transform::Reset, "Translate",
		&PopLib::Transform::Translate, "RotateRad", &PopLib::Transform::RotateRad, "RotateDeg",
		&PopLib::Transform::RotateDeg, "Scale", &PopLib::Transform::Scale, "GetMatrix", &PopLib::Transform::GetMatrix,

		"mTransX1", &PopLib::Transform::mTransX1, "mTransY1", &PopLib::Transform::mTransY1, "mTransX2",
		&PopLib::Transform::mTransX2, "mTransY2", &PopLib::Transform::mTransY2, "mScaleX", &PopLib::Transform::mScaleX,
		"mScaleY", &PopLib::Transform::mScaleY, "mRot", &PopLib::Transform::mRot, "mHaveRot",
		&PopLib::Transform::mHaveRot, "mHaveScale", &PopLib::Transform::mHaveScale, "mComplex",
		&PopLib::Transform::mComplex);
}