#ifndef __TEXTWIDGET_HPP__
#define __TEXTWIDGET_HPP__

#pragma once

#include "widget.hpp"
#include "scrolllistener.hpp"

namespace PopLib
{

class ScrollbarWidget;
class Font;

typedef std::vector<PopString> PopStringVector;
typedef std::vector<int> IntVector;

class TextWidget : public Widget, public ScrollListener
{
  public:
	Font *mFont;
	ScrollbarWidget *mScrollbar;

	PopStringVector mLogicalLines;
	PopStringVector mPhysicalLines;
	IntVector mLineMap;
	double mPosition;
	double mPageSize;
	bool mStickToBottom;
	int mHiliteArea[2][2];
	int mMaxLines;

  public:
	TextWidget();

	virtual PopStringVector GetLines();
	virtual void SetLines(PopStringVector theNewLines);
	virtual void Clear();
	virtual void DrawColorString(Graphics *g, const PopString &theString, int x, int y, bool useColors);
	virtual void DrawColorStringHilited(Graphics *g, const PopString &theString, int x, int y, int theStartPos,
										int theEndPos);
	virtual int GetStringIndex(const PopString &theString, int thePixel);

	virtual int GetColorStringWidth(const PopString &theString);
	virtual void Resize(int theX, int theY, int theWidth, int theHeight);
	virtual Color GetLastColor(const PopString &theString);
	virtual void AddToPhysicalLines(int theIdx, const PopString &theLine);

	virtual void AddLine(const PopString &theString);
	virtual bool SelectionReversed();
	virtual void GetSelectedIndices(int theLineIdx, int *theIndices);
	virtual void Draw(Graphics *g);
	virtual void ScrollPosition(int theId, double thePosition);
	virtual void GetTextIndexAt(int x, int y, int *thePosArray);
	virtual PopString GetSelection();

	virtual void MouseDown(int x, int y, int theClickCount);
	virtual void MouseDrag(int x, int y);

	virtual void KeyDown(KeyCode theKey);
};

} // namespace PopLib

#endif