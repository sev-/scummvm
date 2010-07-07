#include "common/system.h"
#include "common/util.h"

#include "comet/text.h"

namespace Comet {

TextReader::TextReader(CometEngine *vm)
	: _vm(vm), _cachedTextResource(NULL), _cachedTextResourceTableIndex(-1) {
}

TextReader::~TextReader() {
	delete _cachedTextResource;
}

void TextReader::setTextFilename(const char *filename) {
	_textFilename = filename;	
	_cachedTextResourceTableIndex = -1;
	delete _cachedTextResource;
}

TextResource *TextReader::loadTextResource(uint tableIndex) {
	TextResource *textResource = new TextResource();
	_vm->_res->loadFromCC4(textResource, _textFilename.c_str(), tableIndex);
	return textResource;
}

byte *TextReader::getString(uint tableIndex, uint stringIndex) {
	return getCachedTextResource(tableIndex)->getString(stringIndex);
}

void TextReader::loadString(uint tableIndex, uint stringIndex, byte *buffer) {
	getCachedTextResource(tableIndex)->loadString(stringIndex, buffer);
}

TextResource *TextReader::getCachedTextResource(uint tableIndex) {
	if ((uint)_cachedTextResourceTableIndex != tableIndex || !_cachedTextResource) {
		debug("TextReader::getCachedTextResource() loading table %d", tableIndex);
		delete _cachedTextResource;
		_cachedTextResource = loadTextResource(tableIndex);
		_cachedTextResourceTableIndex = tableIndex;
	}
	return _cachedTextResource;
}

} // End of namespace Comet
