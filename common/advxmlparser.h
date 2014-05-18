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

#ifndef ADV_XML_PARSER_H
#define ADV_XML_PARSER_H

#include "common/scummsys.h"
#include "common/types.h"

#include "common/fs.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "common/memorypool.h"


namespace Common {

class SeekableReadStream;

struct XMLTree {
	typedef HashMap<String,String> AttributeMap;

	/**
	 * Enum showing type of tree.
	 *
	 * <iamkey> I am Text </iamkey> <![CDATA[ I am CData ]]>
	 */
	enum {
		kRoot,
		kKey,
		kText
	} type;

	/** 
	 * text is the key name if kKey,
	 *             text if kText,
	 */
	String text;
	Array<XMLTree *> children; ///< only if kKey or kRoot
	AttributeMap attrs; ///< only if kKey

	~XMLTree();
};

/**
 * This class is more flexible than the existing xml parser. It (should) parse
 * arbitrary xml structures and create an XMLTree representing that structure.
 *
 * The XMLTree should be much easier to parse programmatically than the pure XML
 *
 * For simple xml where the exact structure is known ahead of time, XMLParser
 * might be easier.
 */
class AdvXMLParser {
public:
	/**
	 * Parse the xml and return a tree structure representing the document
	 */
	XMLTree *parse();

	bool loadFile(const String &filename);
	bool loadStream(SeekableReadStream *stream);
private:

	SeekableReadStream *_stream;

	/**
	 * Main recursive parser
	 */
	XMLTree *internParse();

	/**
	 * returns the next token
	 */
	String parseToken();

	/** 
	 * returns the next byte without advancing the stream
	 */
	byte peekByte();

	/**
	 * returns the contents of a quoted string.
	 */
	String parseString();

	/**
	 * Advances the stream past the next appearence of end.
	 */
	void skipUntil(const String &end);

	/**
	 * returns the string up until the next appearence of end.
	 */
	String readUntil(const String &end);

	/**
	 * Outputs line and character information along with msg.
	 * TODO: gracefully handle errors instead of just crashing
	 */
	void parseError(const String &msg);

	/**
	 * Unescape xml escapes (&quot; etc.)
	 * TODO: not implemented
	 */
	String unescape(const String &msg);

	/**
	 * skips past '\n', '\r', '\t', and ' '
	 */
	void skipWhitespace();
};

} // namespace Common

#endif
