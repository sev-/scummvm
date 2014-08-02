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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/advxmlparser.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/stream.h"

namespace Common {

XMLTree::~XMLTree() {
	for (int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

bool AdvXMLParser::loadFile(const String &filename) {
	_stream = SearchMan.createReadStreamForMember(filename);
	loadStream(_stream);
	return _stream;
}

bool AdvXMLParser::loadStream(SeekableReadStream *stream) {
	return (_stream = stream);
}

XMLTree *AdvXMLParser::parse() {
	XMLTree *header = internParse();
	if (header->type != XMLTree::kKey || header->text != "?xml")
		parseError("No header");
	if (!header->attrs.contains("version") || header->attrs["version"] != "1.0")
		parseError("Missing or invalid version in header");

	XMLTree *result = new XMLTree();
	result->type = XMLTree::kRoot;
	while (!_stream->eos()) {
		result->children.push_back(internParse());
	}
	return result;
}

XMLTree *AdvXMLParser::internParse() {
	char c = _stream->readByte();
	String token;
	XMLTree *result;
	if (c == '<') {
		// Either key or CData or comment
		token = parseToken();
		if (token == "!--") {
			skipUntil("-->");
			return internParse();
		}
		if (token == "![CDATA[") {
			result = new XMLTree();
			result->type = XMLTree::kText;
			result->text = readUntil("]]>");
			return result;
		}
		result = new XMLTree();
		result->type = XMLTree::kKey;
		result->text = token;

		// Read attributes
		c = peekByte();
		while (c != '>' && c != '/') {
			if (c == '?' && result->text == "?xml") {
				// Special exception for header
				_stream->readByte();
				c = _stream->readByte();
				if (c == '>')
					return result;
				parseError("Unexpected ?");
			}
			token = parseToken();
			c = _stream->readByte();
			if (c != '=')
				parseError("Expected = in attribute list");
			skipWhitespace();
			String value;
			// attribute values aren't always quoted
			if (peekByte() == '"')
				value = parseString();
			else
				value = parseToken();
			result->attrs[token] = value;
			c = peekByte();
		}

		c = _stream->readByte();
		if (c == '/') {
			// might be self close tag i.e. <iamkey/>
			char next = _stream->readByte();
			if (next == '>')
				return result;
			parseError("Unexpected /");
		}

		// Read Children
		while (!_stream->eos()) {
			c = peekByte();
			if (c == '<') {
				_stream->readByte();
				c = _stream->readByte();
				if (c == '/') {
					// close tag
					token = parseToken();
					if (token != result->text)
						parseError("Unexpected close tag");
					skipUntil(">");
					return result;
				}
				_stream->seek(-2, SEEK_CUR);
				result->children.push_back(internParse());
			} else {
				internParse();
			}
		}
	}
	result = new XMLTree();
	result->type = XMLTree::kText;
	result->text = unescape(readUntil("<"));
	if (!_stream->eos())
		_stream->seek(-1, SEEK_CUR);
	return result;
}

String AdvXMLParser::parseToken() {
	skipWhitespace();
	String result;
	char c = _stream->readByte();
	while(
			c != ' ' &&
			c != '=' &&
			c != '>' &&
			c != '\n' &&
			c != '\r' &&
			c != '\t' ) {
		result += c;
		c = _stream->readByte();
	}
	_stream->seek(-1, SEEK_CUR);
	return result;
}

void AdvXMLParser::skipUntil(const String &end) {
	readUntil(end);
}

String AdvXMLParser::readUntil(const String &end) {
	int size = end.size();
	char * buffer = new char[size + 1];
	String result;

	buffer[size] = '\0';
	while (!_stream->eos()) {
		int actual = _stream->read(buffer, size);
		if (actual != size)
			break;
		if (end == buffer)
			break;
		_stream->seek(-size, SEEK_CUR);
		result += _stream->readByte();
	}

	delete[] buffer;
	return result;
}

String AdvXMLParser::unescape(const String &src) {
	// FIXME: Actually unescape &quot; and such
	return src;
}

void AdvXMLParser::parseError(const String &msg) {
	int pos = _stream->pos();
	int i = 0;
	int lines = 1;
	int chars;
	_stream->seek(0, SEEK_SET);
	while (i != pos) {
		if (_stream->readByte() == '\n') {
			lines++;
			chars = 0;
		}
		++i;
		++chars;
	}
	error("Error while parsing on line %d:%d: %s", lines, chars, msg.c_str());
}

byte AdvXMLParser::peekByte() {
	byte b = _stream->readByte();
	_stream->seek(-1, SEEK_CUR);
	return b;
}

String AdvXMLParser::parseString() {
	skipWhitespace();
	char start = _stream->readByte();
	if (start != '\'' && start != '"')
		parseError("Expected quoted string");
	return readUntil(String(start));
}

void AdvXMLParser::skipWhitespace() {
	char c;
	do {
		c = _stream->readByte();
	} while (c == ' ' || c == '\n' || c == '\t' || c == '\r');
	_stream->seek(-1, SEEK_CUR);
}

} // namespace Common
