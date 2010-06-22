#include "comet/comet.h"
#include "sound/audiostream.h"
#include "sound/decoders/raw.h"
#include "sound/decoders/voc.h"

namespace Comet {

void CometEngine::openVoiceFile(int index) {

	if (_narFile) {
		_narFile->close();
		delete _narFile;
	}
	
	delete[] _narOffsets;
		
	char narFilename[16];
	snprintf(narFilename, 16, "D%02d.NAR", index);
	
	_narFile = new Common::File();

	if (!_narFile->open(narFilename))
		error("CometEngine::openVoiceFile()  Could not open %s", narFilename);

	// TODO: Better don't read the offsets at all, only in playVoice
	_narCount = _narFile->readUint32LE() / 4;
	while (!_narFile->eos() && _narCount == 0) {
		_narCount = _narFile->readUint32LE() / 4;
	}
	
	_narOffsets = new uint32[_narCount + 1];
 	_narFile->seek(0);

	for (int i = 0; i < _narCount; i++)
		_narOffsets[i] = _narFile->readUint32LE();
		
	_narOffsets[_narCount] = _narFile->size();

}
	
void CometEngine::playVoice(int number) {

	stopVoice();

	//debug("playVoice() number = %d; _narCount = %d", number, _narCount);

	if (!_narOffsets || number >= _narCount)
		return;

	//debug(4, "CometEngine::playVoice(): number = %d; count = %d", number, _narCount);
		
	if (number + 1 >= _narCount) {
		debug(4, "CometEngine::playVoice(%d)  Voice number error from debugging rooms", number);
		return;
	}

	if (_narOffsets[number] == 0)
		return;

	//debug(4, "CometEngine::playVoice(): offset = %08X", _narOffsets[number]);

	_narFile->seek(_narOffsets[number]);

	int size;
	
	if (_narOffsets[number + 1] <= _narOffsets[number]) {
		debug(4, "CometEngine::playVoice(%d)  Offset error", number);
		return;
	}
	
	size = _narOffsets[number + 1] - _narOffsets[number];

	//debug(4, "CometEngine::playVoice() size = %d", size);


	/* The VOC header's first byte is '\0' instead of a 'C' so we have to work around it */
	byte *readBuffer = (byte *)malloc(size);
	_narFile->read(readBuffer, size);
	readBuffer[0] = 'C';

	Common::MemoryReadStream vocStream(readBuffer, size, DisposeAfterUse::YES);

	Audio::AudioStream *stream = Audio::makeVOCStream(&vocStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_sampleHandle, stream);
}

void CometEngine::stopVoice() {
	if (_mixer->isSoundHandleActive(_sampleHandle))
		_mixer->stopHandle(_sampleHandle);
}

} // End of namespace Comet
