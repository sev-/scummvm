/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMET_TALKTEXT_H
#define COMET_TALKTEXT_H

#include "common/serializer.h"
#include "common/util.h"

namespace Comet {

class CometEngine;

class TalkText {
public:
	TalkText(CometEngine *vm);
	~TalkText();
	void deactivate();
	void update();
	void stopText();
	void updateTextDialog();
	void updateText();
	void updateTalkAnims();
	int getPortraitTalkAnimNumber();
	void setText(byte *text);
	void resetTextValues();
	void showTextBubble(int index, byte *text, int textDuration);
	void setTextTableIndex(uint tableIndex);
	void setVoiceFileIndex(int narFileIndex);
	void playVoice(int voiceIndex);
	void stopVoice();
	void actorTalk(int actorIndex, int talkTextIndex, int color);
	void actorTalkWithAnim(int actorIndex, int talkTextIndex, int animNumber);
	void actorTalkPortrait(int actorIndex, int talkTextIndex, int animNumber, int fileIndex);
	void leaveJournal();
	void handleTalkFinished();
	void sync(Common::Serializer &s);
	int getTalkieMode() const { return _talkieMode; }
	void setTalkieMode(int value) { _talkieMode = value; }
	int getTextSpeed() const { return _textSpeed; }
	void setTextSpeed(int value) { _textSpeed = value; }
	int getTextMaxTextWidth() const { return _textMaxTextWidth; }
	int getTextMaxTextHeight() const { return _textMaxTextHeight; }
	bool isActive() const { return _textActive; }
	bool isBubbleActive() const { return _textBubbleActive; }
	bool isSpeechPlaying() const { return _talkieSpeechPlaying; }
	uint getTextTableIndex() const { return _textTableIndex; }
protected:
	CometEngine *_vm;
	int _talkieMode;
	int _textSpeed;
	byte *_currentText, *_textNextPos;
	int _textMaxTextHeight, _textMaxTextWidth, _textDuration, _textOriginalDuration;
	bool _textBubbleActive;
	int _portraitTalkCounter, _portraitTalkAnimNumber;
	bool _moreText, _textActive;
	byte _talkTextColor;
	byte _actorTalkText[1000]; // Buffer size is taken from the original
	int _talkActorIndex, _talkAnimIndex, _talkAnimPlayFrameIndex, _talkAnimFrameIndex, _talkTextIndex;
	int _currNarFileIndex;
	Common::String _narFilename;
	uint _textTableIndex;
	bool _talkieSpeechPlaying;
};

} // End of namespace Comet

#endif
