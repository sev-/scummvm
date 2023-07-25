/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_PRINTING_PRINTMAN_H
#define BACKENDS_PRINTING_PRINTMAN_H

#include "common/scummsys.h"

#ifdef USE_PRINTING

#include "common/rect.h"
#include "common/noncopyable.h"
#include "common/rational.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/file.h"
#include "common/func.h"
#include "graphics/managed_surface.h"

class PrintJob;
class PrintSettings;

typedef Common::Functor1<PrintJob *, void> *PrintCallback;

class PrintingManager : Common::NonCopyable {
public:
	virtual ~PrintingManager();

	void printCustom(PrintCallback cb) { printCustom(cb, "ScummVM"); }
	void printCustom(PrintCallback cb, PrintSettings *settings) { printCustom(cb, "ScummVM", settings); }
	void printCustom(PrintCallback cb, const Common::String &jobName);
	void printCustom(PrintCallback cb, const Common::String &jobName, PrintSettings *settings);

	void printImage(const Common::String &jobName, const Graphics::ManagedSurface &surf, bool scale=false);

	void printImage(const Graphics::ManagedSurface &surf, bool scale = false) {
		printImage("ScummVM", surf, scale);
	}

	void printPlainTextFile(const Common::String &jobName, Common::SeekableReadStream &file);
	void printPlainTextFile(Common::File &file);

	//creates a new PrintSettings object. either hand ownership back using createJob or delete it yourself. Not both.
	virtual PrintSettings *getDefaultPrintSettings() const = 0;

protected:
	PrintJob *createJob(PrintCallback cb, const Common::String &jobName);

	// takes ownership of the PrintSettings object
	virtual PrintJob *createJob(PrintCallback cb, const Common::String &jobName, PrintSettings *settings) = 0;
};

class TextMetrics;

class PrintSettings : Common::NonCopyable {

public:
	enum DuplexMode {
		Unknown = 0,
		Simplex = 1,
		Vertical = 2,
		Horizontal = 3
	};
	
	virtual ~PrintSettings() {}

public:
	virtual DuplexMode getDuplexMode() const = 0;
	virtual void setDuplexMode(DuplexMode mode) = 0;
	virtual bool getLandscapeOrientation() const = 0;
	virtual void setLandscapeOrientation(bool landscapeOrientation) = 0;
	virtual bool getColorPrinting() const = 0;
	virtual void setColorPrinting(bool colorMode) = 0;
};

class PrintJob : Common::NonCopyable {
public:
	friend class PrintingManager;
	PrintJob(PrintCallback cb);
	virtual ~PrintJob();

	virtual void drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos);
	virtual void drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize) = 0;
	virtual void drawText(const Common::String &text, Common::Point pos) = 0;

	virtual void setTextColor(int r, int g, int b) = 0;
	virtual Common::Rect getTextBounds(const Common::String &text) const = 0;
	virtual TextMetrics getTextMetrics() = 0;

	virtual Common::Rational getPixelAspectRatio() const = 0;

	// Size of the printable area, in pixels
	virtual Common::Rect getPrintableArea() const = 0;
	// Offset from the top left corner of the paper to the printable area
	virtual Common::Point getPrintableAreaOffset() const = 0;
	// Size of the paper
	virtual Common::Rect getPaperDimensions() const = 0;

	//Does not give away ownership of the PrintSettings object
	virtual const PrintSettings *getPrintSettings() const = 0;

	virtual void beginPage() = 0;
	virtual void endPage() = 0;
	virtual void endDoc() = 0;
	virtual void abortJob() = 0;

protected:
	PrintCallback printCallback;

	virtual bool print()=0;
};

class TextMetrics {
public:
	// distance over the base line
	uint accent;
	// distance unde the base line
	uint deccent;
	//space to add between text lines, in addition to the height.
	uint leading;

	uint getHeight() const { return accent + deccent; }
	uint getLineHeight() const { return getHeight() + leading; }
};

#endif
#endif
