#include "common/system.h"
#include "common/util.h"
#include "comet/text.h"

namespace Comet {

TextStrings::TextStrings(Common::File *fd, uint32 size) {
	uint32 firstOffs = fd->readUint32LE();
	_stringCount = firstOffs / 4;
	_stringOffsets = new uint32[_stringCount + 1];
	_stringOffsets[0] = 0;
	for (uint i = 1; i < _stringCount; i++)
		_stringOffsets[i] = fd->readUint32LE() - firstOffs;
	_stringOffsets[_stringCount] = size - firstOffs;
	size -= firstOffs; // size of text data
	_data = new byte[size];
	fd->read(_data, size);
	// decrypt the text data
	for (uint i = 0; i < size; i++)
		_data[i] = _data[i] - 0x54 * (i + 1);
}

TextStrings::~TextStrings() {
	delete _stringOffsets;
	delete _data;
}

byte *TextStrings::getString(uint stringIndex) {
	/*	We add 1 to the offset since that's where the actual text starts.
		The leftover byte at the beginning is the '*'-character, which
		serves as string terminator for the preceeding string. */
	return _data + _stringOffsets[stringIndex] + 1;
}

void TextStrings::loadString(uint stringIndex, byte *buffer) {

	int stringLen = _stringOffsets[stringIndex + 1] - _stringOffsets[stringIndex];
	memcpy(buffer, getString(stringIndex), stringLen);

}

TextReader::TextReader() {
}

TextReader::~TextReader() {
}

void TextReader::open(const char *filename) {
	_fd = new Common::File();
	if (!_fd->open(filename))
		error("TextReader::open() Could not open %s", filename);
	uint32 firstOffs = _fd->readUint32LE();
	_tableCount = firstOffs / 4;
	_tableOffsets = new uint32[_tableCount + 1];
	_tableOffsets[0] = firstOffs;
	for (uint i = 1; i < _tableCount; i++)
		_tableOffsets[i] = _fd->readUint32LE();
	_tableOffsets[_tableCount] = _fd->size();
}

void TextReader::close() {
	if (_fd) {
		_fd->close();
		delete _fd;
		_fd = NULL;
		delete _tableOffsets;
		_tableCount = 0;
	}
}

TextStrings *TextReader::loadTextStrings(uint tableIndex) {
	_fd->seek(_tableOffsets[tableIndex]);
	return new TextStrings(_fd, _tableOffsets[tableIndex + 1] - _tableOffsets[tableIndex]);
}

byte *TextReader::getString(uint tableIndex, uint stringIndex) {
	return getCachedTextStrings(tableIndex)->getString(stringIndex);
}

void TextReader::loadString(uint tableIndex, uint stringIndex, byte *buffer) {
	getCachedTextStrings(tableIndex)->loadString(stringIndex, buffer);
}

TextStrings *TextReader::getCachedTextStrings(uint tableIndex) {
	if (_cachedTextStringsTableIndex != tableIndex || !_cachedTextStrings) {
		debug("TextReader::getCachedTextStrings() loading table %d", tableIndex);
		delete _cachedTextStrings;
		_cachedTextStrings = loadTextStrings(tableIndex);
		_cachedTextStringsTableIndex = tableIndex;
	}
	return _cachedTextStrings;
}

} // End of namespace Comet
