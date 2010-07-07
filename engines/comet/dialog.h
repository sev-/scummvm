#ifndef COMET_DIALOG_H
#define COMET_DIALOG_H

#include "common/util.h"
#include "common/array.h"

#include "comet/comet.h"
#include "comet/script.h"

namespace Comet {

struct DialogItem {
	int16 index;
	byte *text;
	uint16 scriptIp;
};

class Dialog {
public:

	Dialog(CometEngine *vm);
	~Dialog();

	void start(Script *script);
	void stop();
	void update();
	bool isRunning() const { return _isRunning; }
	uint16 getChoiceScriptIp();
	
protected:

	CometEngine *_vm;

	int _selectedItemIndex, _selectedItemIndex2;
	int _introTextIndex, _textX, _textY;
	byte _textColor;
	int _textColorInc;
	bool _isRunning;
	Common::Array<DialogItem> _items;
	Common::Array<GuiRectangle> _itemRectangles;

	void drawTextBubbles();
	
};

}

#endif
