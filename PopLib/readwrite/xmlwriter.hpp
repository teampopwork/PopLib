// xmlwriter.h: interface for the XMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLWRITER_HPP__3DB6D22D_A120_4738_B622_06E90FAED4E4__INCLUDED_)
#define AFX_XMLWRITER_HPP__3DB6D22D_A120_4738_B622_06E90FAED4E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)
#pragma warning(disable : 4503)

#include <stack>
#include "xmlparser.hpp"
#include "common.hpp"

#include "debug/perftimer.hpp"

namespace PopLib
{
class XMLWriter
{
  protected:
	PopString mFileName;
	PopString mErrorText;
	int mLineNum;
	FILE *mFile;
	bool mHasFailed;
	bool mOpenAttributes;

	std::stack<PopString> mSectionStack;
	std::stack<PopString> mWarningStack;

  protected:
	bool CheckFileOpen();
	bool ValidateElementNodeName(const PopString &theNodeName);
	void Fail(const PopString &theErrorText);
	void Warn(const PopString &theWarning);
	void Init();

  public:
	XMLWriter();
	virtual ~XMLWriter();

	static bool AddAttribute(XMLElement *theElement, const PopString &aAttributeKey, const PopString &aAttributeValue);
	bool WriteAttribute(const PopString &aAttributeKey, const PopString &aAttributeValue);
	bool WriteAttribute(const PopString &aAttributeKey, const float &aAttributeValue);
	bool WriteAttribute(const PopString &aAttributeKey, const int &aAttributeValue);
	bool WriteAttribute(const PopString &aAttributeKey);
	void Comment(const PopString &theComment);
	bool StartElement(const PopString &theElementName);
	bool StartElement(XMLElement *theElement);
	bool StopElement();
	bool WriteElementText(PopString theText);
	bool OpenFile(const PopString &theFilename);
	bool CloseFile();
	PopString GetErrorText();
	int GetCurrentLineNum();
	PopString GetFileName();

	bool HasFailed();
};
}; // namespace PopLib

#endif // !defined(AFX_XMLWRITER_HPP__3DB6D22D_A120_4738_B622_06E90FAED4E4__INCLUDED_)
