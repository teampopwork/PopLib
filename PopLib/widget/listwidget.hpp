#ifndef __LISTWIDGET_HPP__
#define __LISTWIDGET_HPP__

#pragma once

#include "scrolllistener.hpp"
#include "widget.hpp"

namespace PopLib
{

typedef std::vector<PopString> PopStringVector;
typedef std::vector<Color> ColorVector;

class ScrollbarWidget;
class ListListener;
class Font;

class ListWidget : public Widget, public ScrollListener
{
  public:
	enum
	{
		JUSTIFY_LEFT = 0,
		JUSTIFY_CENTER,
		JUSTIFY_RIGHT
	};

	enum
	{
		COLOR_BKG = 0,
		COLOR_OUTLINE,
		COLOR_TEXT,
		COLOR_HILITE,
		COLOR_SELECT,
		COLOR_SELECT_TEXT,
	};

  public:
	int mId;
	Font *mFont;
	ScrollbarWidget *mScrollbar;
	int mJustify;

	PopStringVector mLines;
	ColorVector mLineColors;
	double mPosition;
	double mPageSize;
	int mHiliteIdx;
	int mSelectIdx;
	ListListener *mListListener;
	ListWidget *mParent;
	ListWidget *mChild;
	bool mSortFromChild;
	bool mDrawOutline;
	int mMaxNumericPlaces;
	int mItemHeight;

	bool mDrawSelectWhenHilited;
	bool mDoFingerWhenHilited;

	void SetHilite(int theHiliteIdx, bool notifyListener = false);

  public:
	ListWidget(int theId, Font *theFont, ListListener *theListListener);
	virtual ~ListWidget();

	virtual void RemovedFromManager(WidgetManager *theManager);

	virtual PopString GetSortKey(int theIdx);
	virtual void Sort(bool ascending);
	virtual PopString GetStringAt(int theIdx);
	virtual void Resize(int theX, int theY, int theWidth, int theHeight);
	virtual int AddLine(const PopString &theLine, bool alphabetical);
	virtual void SetLine(int theIdx, const PopString &theString);
	virtual int GetLineCount();
	virtual int GetLineIdx(const PopString &theLine);
	virtual void SetColor(const PopString &theLine, const Color &theColor);
	virtual void SetColor(int theIdx, const Color &theColor);
	virtual void SetLineColor(int theIdx, const Color &theColor);
	virtual void RemoveLine(int theIdx);
	virtual void RemoveAll();
	virtual int GetOptimalWidth();
	virtual int GetOptimalHeight();
	virtual void OrderInManagerChanged();
	virtual void Draw(Graphics *g);
	virtual void ScrollPosition(int theId, double thePosition);
	virtual void MouseMove(int x, int y);
	virtual void MouseWheel(int theDelta);
	virtual void MouseDown(int x, int y, int theClickCount)
	{
		Widget::MouseDown(x, y, theClickCount);
	}
	virtual void MouseDown(int x, int y, int theBtnNum, int theClickCount);
	virtual void MouseLeave();
	virtual void SetSelect(int theSelectIdx);
};

} // namespace PopLib

#endif