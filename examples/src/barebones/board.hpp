#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#pragma once

#include "PopLib/widget/widget.hpp"

namespace PopLib
{
class Graphics;

class GameApp;

class Board : public Widget
{
  public:
	Board(GameApp *theApp);

	virtual ~Board();

	virtual void Draw(Graphics *g);

	virtual void Update();

  private:
	GameApp *mApp;
};

} // namespace PopLib

#endif // __BOARD_H__