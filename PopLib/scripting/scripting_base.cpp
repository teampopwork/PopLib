#include "scripting_base.hpp"
#include "debug/log.hpp"
#include "lua/lpoplib.hpp"
#include <filesystem>

using namespace PopLib;

bool ScriptingBase::Initialize()
{
	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string, sol::lib::math,
					   sol::lib::table);

	LOG_INFO("Lua scripting initialized");

	pop_openlibs(lua);

	// replace print
	lua["print"] = [](sol::variadic_args va) {
		std::string combined;
		bool first = true;
		for (auto v : va)
		{
			if (!first)
				combined += "\t";
			first = false;

			if (v.is<std::string>())
			{
				combined += v.as<std::string>();
			}
			else if (v.is<bool>())
			{
				combined += (v.as<bool>() ? "true" : "false");
			}
			else if (v.is<double>())
			{
				combined += std::to_string(v.as<double>());
			}
			else if (v.is<int>())
			{
				combined += std::to_string(v.as<int>());
			}
			else
			{
				combined += "<non-printable>";
			}
		}
		LOG_INFO(combined.c_str());
	};

	return true;
}

void ScriptingBase::Shutdown()
{
	lua.collect_garbage();
	LOG_INFO("Lua scripting shut down");
}

bool ScriptingBase::ExecuteFile(const PopString &fileName)
{
	std::filesystem::path fullPath = std::filesystem::path(scriptFolder) / fileName;

	if (!std::filesystem::exists(fullPath))
	{
		LOG_ERROR("Lua file not found: %s", fullPath.c_str());
		return false;
	}

	auto result = lua.safe_script_file(
		fullPath.string(), [](lua_State * /*L*/, sol::protected_function_result pfr) -> sol::protected_function_result {
			if (!pfr.valid())
			{
				sol::error err = pfr;
				LOG_ERROR("Lua execution error: %s", err.what());
			}
			return pfr;
		});

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Failed to execute file '%s': %s", fullPath.string().c_str(), err.what());
		return false;
	}

	LOG_INFO("Executed Lua file: %s", fullPath.string().c_str());
	return true;
}

bool ScriptingBase::ExecuteString(const PopString &code)
{
	auto result = lua.safe_script(
		std::string(code), [](lua_State * /*L*/, sol::protected_function_result pfr) -> sol::protected_function_result {
			if (!pfr.valid())
			{
				sol::error err = pfr;
				LOG_ERROR("Lua execution error: %s", err.what());
			}
			return pfr;
		});

	if (!result.valid())
	{
		sol::error err = result;
		LOG_ERROR("Failed to execute Lua string: %s", err.what());
		return false;
	}

	return true;
}

bool ScriptingBase::ExecuteFolder(const PopString &folderPath)
{
	std::filesystem::path fullPath = folderPath.empty() ? scriptFolder : folderPath;

	if (!std::filesystem::exists(fullPath) || !std::filesystem::is_directory(fullPath))
	{
		LOG_ERROR("Lua folder not found: %s", fullPath.string().c_str());
		return false;
	}

	for (auto &entry : std::filesystem::directory_iterator(fullPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".lua")
		{
			LOG_INFO("Executing Lua file: %s", entry.path().string().c_str());
			if (!ExecuteFile(entry.path().filename().string()))
			{
				LOG_ERROR("Failed to execute Lua file: %s", entry.path().string().c_str());
				return false;
			}
		}
	}

	return true;
}
