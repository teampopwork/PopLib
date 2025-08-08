#ifndef __JSONPARSER_HPP__
#define __JSONPARSER_HPP__

#pragma once

#include "common.hpp"
#include <any>
#include <json.hpp>

struct PFILE;

namespace PopLib
{

/**
 * @brief a parser for json
 */
class JsonParser
{
  protected:
	/// @brief the json file name
	std::string mFileName;
	/// @brief the error text
	PopString mErrorText;
	/// @brief current line number
	int mLineNum;
	/// @brief true if failed
	bool mHasFailed;

	/// @brief fail
	/// @param theErrorText
	void Fail(const PopString &theErrorText);

  public:
	/// @brief the nlohmann::json object
	nlohmann::json mJson;

  public:
	/// @brief constructor
	JsonParser();
	/// @brief destructor
	virtual ~JsonParser();

	/// @brief opens a file
	/// @param theFilename
	/// @return true if success
	bool OpenFile(const std::string &theFilename);

	/// @brief gets an int from the json file
	/// @param theName
	/// @return the integer
	int GetValueInt(const std::string &theName);
	/// @brief gets a string from the json file
	/// @param theName
	/// @return the string
	PopString GetValueStr(const std::string &theName);
};

}; // namespace PopLib

#endif //__JSONPARSER_HPP__