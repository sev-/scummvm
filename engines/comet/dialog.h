#ifndef COMET_DIALOG_H
#define COMET_DIALOG_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/array.h"

#include "comet/comet.h"

namespace Comet {

struct DialogItem {
	int16 index;
	char *text;
	byte *scriptIp;
};

class Dialog {
public:

	Dialog(CometEngine *vm);
	~Dialog();

	void run(Script *script);
	void stop();
	void update();
	bool isRunning() const { return _dialogRunning; }
	byte *getChoiceScriptIp();
	
protected:

	CometEngine *_vm;

	int _dialogSelectedItemIndex, _dialogSelectedItemIndex2;
	int _dialogTextSubIndex, _dialogTextX, _dialogTextY;
	byte _dialogTextColor;
	int _dialogTextColorInc;
	bool _dialogRunning;
	Common::Array<DialogItem> _dialogItems;
	Common::Array<RectItem> _dialogRects;

	void drawTextBubbles();
	
};

}

#endif
