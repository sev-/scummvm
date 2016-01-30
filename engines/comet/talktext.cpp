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

#include "comet/talktext.h"
#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/comet.h"
#include "comet/dialog.h"
#include "comet/input.h"
#include "comet/screen.h"

namespace Comet {

TalkText::TalkText(CometEngine *vm)
	: _vm(vm), _textActive(false),	_textBubbleActive(false), _talkieSpeechPlaying(false), _talkAnimIndex(-1) {
	
	_talkieMode = _vm->isFloppy() ? 0 : 1;
}

TalkText::~TalkText() {
}
	

void TalkText::deactivate() {
	_textActive = false;
	_talkieSpeechPlaying = false;
}

void TalkText::update() {
	if (_vm->isFloppy()) {
		updateTextDialog();
	} else {
		if (_talkieMode == 0)
			updateTextDialog();
		if ((_talkieMode == 1 && (_textActive || _textBubbleActive)) || (_talkieMode == 2 && _textBubbleActive))
			updateText();
		if (_vm->_dialog->isRunning())
			_vm->_dialog->update();
		updateTalkAnims();
	}
}

void TalkText::updateTextDialog() {
	if ((_vm->isFloppy() && _textActive) || (!_vm->isFloppy() && (_textActive || _textBubbleActive)))
		updateText();
	if (_vm->_dialog->isRunning())
		_vm->_dialog->update();
}

void TalkText::updateText() {

	Actor *talkingActor = _vm->_actors->getActor(_talkActorIndex);
	int textX, textY;

	if (talkingActor->_textX != -1) {
		textX = talkingActor->_textX;
		textY = talkingActor->_textY;
	} else {
		textX = talkingActor->_x;
		textY = talkingActor->_y - _textMaxTextHeight - 50;
	}

	_vm->drawBubble(textX - _textMaxTextWidth - 4, textY - 4, textX + _textMaxTextWidth + 4, textY + _textMaxTextHeight);
	_vm->_screen->drawText3(textX + 1, textY, _currentText, _talkTextColor, 0);
	
	if (--_textDuration <= 0) {
		_textActive = _moreText;
		if (_moreText) {
			// There's more text to display
			setText(_textNextPos);
		} else {
			if (_vm->isFloppy() || _talkieMode == 0 || !_talkieSpeechPlaying) {
				// Stop text display if text only mode or speech mode
				resetTextValues();
			} else if (_talkieMode == 1 && _talkieSpeechPlaying) {
				// Keep text display alive if text+speech mode
				_textDuration = 2;
				_textActive = true;
			}
		}
	}

}

void TalkText::updateTalkAnims() {
	if (!_vm->_mixer->isSoundHandleActive(_vm->_sampleHandle))
		stopVoice();

	// TODO: Update talk anim
}

int TalkText::getPortraitTalkAnimNumber() {
	if (_portraitTalkCounter == 0) {
		if (_talkieMode == 0) {
			_portraitTalkAnimNumber = _vm->randomValue(4);
			if (_portraitTalkAnimNumber == 0)
				_portraitTalkCounter = 1;
		} else {
			_portraitTalkAnimNumber = _vm->randomValue(3);
			if (!_talkieSpeechPlaying)
		  		_portraitTalkAnimNumber = 0;
		}
	} else {
		_portraitTalkCounter++;
		if (((_talkieMode == 1 || _talkieMode == 2) && _portraitTalkCounter == 1) || _portraitTalkCounter == 10)
			_portraitTalkCounter = 0;
	}
	return _portraitTalkAnimNumber;
}

void TalkText::setText(byte *text) {
	int lineCount = 0;

	_currentText = text;

	_textMaxTextHeight = 0;
	_textMaxTextWidth = 0;
	_moreText = false;
	_textDuration = 0;

	while (*text != '*') {
		int textWidth = _vm->_screen->getTextWidth(text);
		_textDuration += textWidth / 4;
		if (_textDuration < 100)
			_textDuration = 100;
		if (textWidth > _textMaxTextWidth)
			_textMaxTextWidth = textWidth;
		text += strlen((char*)text) + 1;
		if (textWidth != 0)
			_textMaxTextHeight++;
		if (++lineCount == 3 && *text != '*') {
			_moreText = true;
			break;
		}
	}

	_textNextPos = text;
	_textMaxTextWidth /= 2;
	_textMaxTextHeight *= 8;

	if (_textSpeed == 0) {
		_textDuration /= 2;
	} else if (_textSpeed == 2) {
		_textDuration = _textDuration / 2 + _textDuration;
	}
	
	_textOriginalDuration = _textDuration;
	
}

void TalkText::resetTextValues() {
	_vm->_dialog->stop();
	_textBubbleActive = false;
	deactivate();
}

void TalkText::stopText() {
	if (_vm->isFloppy()) {
		if (_textDuration > 1 && _textDuration <= _textOriginalDuration - 3)
			_textDuration = 1;
		if (_vm->_dialog->isRunning())
			_vm->_dialog->stop();			
	} else {
		_textDuration = 1;
		_textActive = false;
		stopVoice();
	}
	_vm->_input->waitForKeys();
}

void TalkText::showTextBubble(int index, byte *text, int textDuration) {
	_talkActorIndex = 0;
	_talkTextColor = 21;
	_talkTextIndex = index;
	setText(text);
	_textActive = true;
	_textBubbleActive = true;
	if (_vm->isFloppy()) {
		_textDuration = textDuration;
		_textOriginalDuration = textDuration;
	}
}

void TalkText::setTextTableIndex(uint tableIndex) {
	_textTableIndex = tableIndex;
}

void TalkText::setVoiceFileIndex(int narFileIndex) {
	if (!_vm->isFloppy()) {
		_currNarFileIndex = narFileIndex;
		if (_vm->getGameID() == GID_MUSEUM)
			narFileIndex = 6; // Lovecraft museum uses only this NAR file
		_narFilename = Common::String::format("D%02d.NAR", narFileIndex);
	}
}

void TalkText::playVoice(int voiceIndex) {
	if (!_vm->isFloppy()) {
		stopVoice();
		_textActive = true;
		_talkieSpeechPlaying = true;
		// TODO FIXME This is ugly
		_vm->_currSoundResourceIndex = -1;
		_vm->_res->loadFromNar(_vm->_soundResource, _narFilename.c_str(), voiceIndex);
		_vm->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_vm->_sampleHandle, _vm->_soundResource->makeAudioStream());
	}
}

void TalkText::stopVoice() {
	if (!_vm->isFloppy()) {
		if (_vm->_mixer->isSoundHandleActive(_vm->_sampleHandle))
			_vm->_mixer->stopHandle(_vm->_sampleHandle);
		if (_talkieMode == 2 && !_textBubbleActive) {
			_textActive = false;
			_textDuration = 0;
		}
		_talkieSpeechPlaying = false;
	}
}

void TalkText::actorTalk(int actorIndex, int talkTextIndex, int color) {
	_talkActorIndex = actorIndex;
	_talkTextIndex = talkTextIndex;
	if (_vm->isFloppy() || _talkieMode == 0 || _talkieMode == 1) {
		_vm->_textReader->loadString(_textTableIndex + 3, _talkTextIndex, _actorTalkText);
		setText(_actorTalkText);
	}
	if (!_vm->isFloppy() && (_talkieMode == 2 || _talkieMode == 1))
		playVoice(_talkTextIndex);
	_textActive = true;
	_talkTextColor = color;
}

void TalkText::actorTalkWithAnim(int actorIndex, int talkTextIndex, int animNumber) {
	Actor *actor = _vm->_actors->getActor(actorIndex);
	actorTalk(actorIndex, talkTextIndex, actor->_textColor);
	if (animNumber != 255) {
		// Save current actor animation
		_talkAnimIndex = actor->_animIndex;
		_talkAnimPlayFrameIndex = actor->_animPlayFrameIndex;
		_talkAnimFrameIndex = actor->_animFrameIndex;
		// Set actor talk animation
		actor->setAnimationIndex(animNumber);
		actor->_status = 2;
	} else {
		_talkAnimIndex = -1;
	}
}

void TalkText::actorTalkPortrait(int actorIndex, int talkTextIndex, int animNumber, int fileIndex) {
	Actor *portraitActor = _vm->_actors->getActor(kActorPortrait);
	int16 animationSlot = _vm->_animationMan->getAnimationResource(_vm->_animationType, fileIndex);
	portraitActor->init(animationSlot);
	if (actorIndex != -1) {
		portraitActor->_textX = 0;
		portraitActor->_textY = 160;
		portraitActor->_textColor = _vm->_actors->getActor(actorIndex)->_textColor;
	}
	_vm->_animationType = 0;
	portraitActor->setPosition(0, 199);
	actorTalkWithAnim(kActorPortrait, talkTextIndex, animNumber);
	_talkAnimIndex = actorIndex;
	_vm->_screen->enableTransitionEffect();
}

void TalkText::leaveJournal() {
	stopVoice();
	_textActive = false;
	setVoiceFileIndex(_textTableIndex);
}

void TalkText::handleTalkFinished() {
	if (_talkActorIndex == 10) {
		if (_talkAnimIndex != -1)
			_vm->_actors->getActor(_talkAnimIndex)->_visible = true;
		_vm->_actors->getActor(10)->_life = 0;
		_vm->_screen->enableTransitionEffect();
	} else if (_talkAnimIndex != -1) {
		// Restore previous actor animation
		Actor *actor = _vm->_actors->getActor(_talkActorIndex);
		actor->setAnimationIndex(_talkAnimIndex);
		actor->_animPlayFrameIndex = _talkAnimPlayFrameIndex;
		actor->_animFrameIndex = _talkAnimFrameIndex;
		_talkAnimIndex = -1;
	}
}

void TalkText::sync(Common::Serializer &s) {
	s.syncAsUint16LE(_textTableIndex);
	if (s.isLoading())
		setVoiceFileIndex(_textTableIndex);
}

} // End of namespace Comet
