#ifndef SCRIPTING_BASE_HPP
#define SCRIPTING_BASE_HPP

#pragma once

#include "../common.hpp"
#include "../appbase.hpp"
#include <sol/sol.hpp>
#include <functional>
#include <string>

class ScriptingBase
{
  public:
	ScriptingBase() = default;
	virtual ~ScriptingBase()
	{
		Shutdown();
	}

	bool Initialize();
	void Shutdown();

	bool ExecuteFile(const PopString &fileName);
	bool ExecuteString(const PopString &code);
	bool ExecuteFolder(const PopString &folderPath);

	template <typename Func> void RegisterFunction(const std::string &name, Func fn)
	{
		lua.set_function(name, fn);
	}

	sol::state &GetLuaState()
	{
		return lua;
	}

	void SetScriptFolder(const PopString &folder)
	{
		scriptFolder = folder;
	}

  private:
	sol::state lua;
	PopString scriptFolder = "scripts/";
};

#endif