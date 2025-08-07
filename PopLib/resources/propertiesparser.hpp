#ifndef __PROPERTIES_PARSER_HPP__
#define __PROPERTIES_PARSER_HPP__

#pragma once

#include "appbase.hpp"

namespace PopLib
{

class XMLParser;

class PropertiesParser
{
  public:
	AppBase *mApp;
	XMLParser *mXMLParser;
	PopString mError;
	bool mHasFailed;

  protected:
	void Fail(const PopString &theErrorText);

	bool ParseSingleElement(PopString *theString);
	bool ParseStringArray(StringVector *theStringVector);
	bool ParseProperties();
	bool DoParseProperties();

  public:
	PropertiesParser(AppBase *theApp);
	virtual ~PropertiesParser();

	bool ParsePropertiesFile(const std::string &theFilename);
	bool ParsePropertiesBuffer(const Buffer &theBuffer);
	PopString GetErrorText();
};

} // namespace PopLib

#endif