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

#ifdef WIN32
#ifdef USE_PRINTING

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winspool.h>
#include <Objbase.h>
#include <commdlg.h>

#include "common/system.h"
#include "backends/platform/sdl/win32/win32.h"

#include "backends/printing/printman.h"
#include "win32-printman.h"
#include "common/ustr.h"

class Win32PrintJob;
class Win32PrintSettings;

class Win32PrintingManager : public PrintingManager {
public:
	virtual ~Win32PrintingManager();
	
	PrintJob *createJob(PrintCallback cb, const Common::String &jobName, PrintSettings *settings) override;

	PrintSettings *getDefaultPrintSettings() const;
	HGLOBAL getDefaultDevmodeGlobal() const;
	HGLOBAL getDefaultDevmodeGlobal(LPTSTR devName) const;
};

class Win32PrintJob : public PrintJob {
public:
	friend class Win32PrintingManager;

	Win32PrintJob(PrintCallback cb, const Common::String &jobName, Win32PrintSettings *settings);
	~Win32PrintJob();

	void drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos);
	void drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize);
	void drawText(const Common::String &text, Common::Point pos);

	void setTextColor(int r, int g, int b);
	Common::Rect getTextBounds(const Common::String &text) const;
	TextMetrics getTextMetrics();

	Common::Rational getPixelAspectRatio() const;
	Common::Rect getPrintableArea() const;
	Common::Point getPrintableAreaOffset() const;
	Common::Rect getPaperDimensions() const;

	const PrintSettings *getPrintSettings() const {
		return (const PrintSettings *)(settings);
	}

	void beginPage();
	void endPage();
	void endDoc();
	void abortJob();

	Common::String jobName;

private:
	void showPrinterDialog();
	void beginDocument();

	HDC createDefaultPrinterContext();
	HDC createPrinterContext(LPTSTR devName);
	HBITMAP buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf);

	HDC hdcPrint;
	bool jobActive;
	Win32PrintSettings *settings;

	friend class Win32PrintSettings;

protected:
	void print() override;
};

class Win32PrintSettings : public PrintSettings {
public:
	Win32PrintSettings(HGLOBAL devmodeGlobal) : devmodeGlobal(devmodeGlobal) {}
	~Win32PrintSettings() {
		GlobalFree(devmodeGlobal);
	}

	
	DuplexMode getDuplexMode() const;
	void setDuplexMode(DuplexMode mode);
	bool getLandscapeOrientation() const;
	void setLandscapeOrientation(bool);
	bool getColorPrinting() const;
	void setColorPrinting(bool);

	HGLOBAL getDevModeGlobal() { return devmodeGlobal; }
	void setDevModeGlobal(HGLOBAL);

	DEVMODE *lockDevmode() const;
	void unlockDevmode() const;

private:
	HGLOBAL devmodeGlobal;
};

Win32PrintingManager::~Win32PrintingManager() {}

PrintJob *Win32PrintingManager::createJob(PrintCallback cb, const Common::String &jobName, PrintSettings *settings) {
	return new Win32PrintJob(cb, jobName, (Win32PrintSettings*)settings);
}


Win32PrintJob::Win32PrintJob(PrintCallback cb, const Common::String &jobName, Win32PrintSettings *settings) : PrintJob(cb), jobName(jobName), jobActive(false), hdcPrint(NULL), settings(settings) {
	// hdcPrint = createDefaultPrinterContext();

	showPrinterDialog();
}

void Win32PrintJob::beginDocument() {
	DOCINFOA info;
	info.cbSize = sizeof(info);
	info.fwType = 0;
	info.lpszDatatype = nullptr;
	info.lpszOutput = nullptr;
	info.lpszDocName = const_cast<const char *>(jobName.c_str());
	StartDocA(hdcPrint, &info);
	jobActive = true;
}

Win32PrintJob::~Win32PrintJob() {
	if (jobActive) {
		abortJob();
		warning("Printjob still active during destruction!");
	}
	DeleteDC(hdcPrint);
	delete settings;
}

void Win32PrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos) {
	HDC hdcImg = CreateCompatibleDC(hdcPrint);

	HBITMAP bitmap = buildBitmap(hdcPrint, surf);
	if (!bitmap) {
		DeleteDC(hdcImg);
		return;
	}

	SelectObject(hdcImg, bitmap);
	if (surf.hasTransparentColor()) {
		byte pal[4];
		surf.grabPalette(pal, surf.getTransparentColor(), 1);
		UINT transpColor = RGB(pal[0], pal[1], pal[2]);
		TransparentBlt(hdcPrint, pos.x, pos.y, surf.w, surf.h, hdcImg, 0, 0, surf.w, surf.h, transpColor);
	} else if (surf.format.aBits()>0) {
		BLENDFUNCTION blend;
		blend.AlphaFormat = AC_SRC_ALPHA;
		blend.BlendFlags = 0;
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		AlphaBlend(hdcPrint, pos.x, pos.y, surf.w, surf.h, hdcImg, 0, 0, surf.w, surf.h, blend);
	} else {
		BitBlt(hdcPrint, pos.x, pos.y, surf.w, surf.h, hdcImg, 0, 0, SRCCOPY);
	}
	DeleteObject(bitmap);
	DeleteDC(hdcImg);
}

void Win32PrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize) {
	HDC hdcImg = CreateCompatibleDC(hdcPrint);

	HBITMAP bitmap = buildBitmap(hdcPrint, surf);
	if (!bitmap) {
		DeleteDC(hdcImg);
		return;
	}

	SelectObject(hdcImg, bitmap);
	if (surf.hasTransparentColor()) {
		byte pal[4];
		surf.grabPalette(pal, surf.getTransparentColor(), 1);
		UINT transpColor=RGB(pal[0],pal[1],pal[2]);
		TransparentBlt(hdcPrint, posAndSize.left, posAndSize.top, posAndSize.width(), posAndSize.height(), hdcImg, 0, 0, surf.w, surf.h, transpColor);
	} else if (surf.format.aBits() > 0) {
		BLENDFUNCTION blend;
		blend.AlphaFormat = AC_SRC_ALPHA;
		blend.BlendFlags = 0;
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		AlphaBlend(hdcPrint, posAndSize.left, posAndSize.top, posAndSize.width(), posAndSize.height(), hdcImg, 0, 0, surf.w, surf.h, blend);
	} else {
		StretchBlt(hdcPrint, posAndSize.left, posAndSize.top, posAndSize.width(), posAndSize.height(), hdcImg, 0, 0, surf.w, surf.h, SRCCOPY);
	}
	DeleteObject(bitmap);
	DeleteDC(hdcImg);
}

void Win32PrintJob::drawText(const Common::String &text, Common::Point pos) {
	TextOutA(hdcPrint, pos.x, pos.y, const_cast<char *>(text.c_str()), text.size());
}

void Win32PrintJob::setTextColor(int r, int g, int b) {
	SetTextColor(hdcPrint, RGB(r, g, b));
}

Common::Rect Win32PrintJob::getTextBounds(const Common::String &text) const {
	SIZE winSize;
	GetTextExtentPoint32A(hdcPrint, text.c_str(), text.size(), &winSize);
	return Common::Rect(winSize.cx, winSize.cy);
}

TextMetrics Win32PrintJob::getTextMetrics() {
	TEXTMETRICA winMetrics;
	TextMetrics metrics;
	GetTextMetricsA(hdcPrint, &winMetrics);

	metrics.accent = winMetrics.tmAscent;
	metrics.deccent = winMetrics.tmDescent;
	metrics.leading = winMetrics.tmExternalLeading;

	return metrics;
}

Common::Rational Win32PrintJob::getPixelAspectRatio() const {
	return Common::Rational(
		GetDeviceCaps(hdcPrint, ASPECTX),
		GetDeviceCaps(hdcPrint, ASPECTY)
	);
}

Common::Rect Win32PrintJob::getPrintableArea() const {
	return Common::Rect(
		GetDeviceCaps(hdcPrint, HORZRES),
		GetDeviceCaps(hdcPrint, VERTRES)
	);
}

Common::Point Win32PrintJob::getPrintableAreaOffset() const {
	return Common::Point(
		GetDeviceCaps(hdcPrint, PHYSICALOFFSETX),
		GetDeviceCaps(hdcPrint, PHYSICALOFFSETY)
	);
}

Common::Rect Win32PrintJob::getPaperDimensions() const {
	return Common::Rect(
		GetDeviceCaps(hdcPrint, PHYSICALWIDTH),
		GetDeviceCaps(hdcPrint, PHYSICALHEIGHT)
	);
}

void Win32PrintJob::beginPage() {
	StartPage(hdcPrint);
}

void Win32PrintJob::endPage() {
	EndPage(hdcPrint);
}

void Win32PrintJob::endDoc() {
	EndDoc(hdcPrint);
	jobActive = false;
}

void Win32PrintJob::abortJob() {
	AbortDoc(hdcPrint);
	jobActive = false;
}

void Win32PrintJob::showPrinterDialog() {
	PRINTDLGEX dlg;

	HWND parent = (dynamic_cast<OSystem_Win32 *>(g_system))->getHwnd();

	memset(&dlg, 0, sizeof(dlg));
	dlg.lStructSize = sizeof(dlg);

	dlg.hwndOwner = parent;

	dlg.Flags = PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_NOCURRENTPAGE | PD_NOPAGENUMS;
	dlg.nStartPage = START_PAGE_GENERAL;
	dlg.hDevMode = settings->getDevModeGlobal();

	HRESULT res=PrintDlgEx(&dlg);

	if (res != S_OK) {
		error("PrintDlgEx failed");
		return;
	}

	GlobalFree(dlg.hDevNames);

	settings->setDevModeGlobal(dlg.hDevMode);

	if (dlg.dwResultAction != PD_RESULT_PRINT) {
		return;
	}

	this->hdcPrint = dlg.hDC;
}

HDC Win32PrintJob::createDefaultPrinterContext() {
	TCHAR szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success=GetDefaultPrinter(szPrinter, &cchPrinter);
	if (!success)
		return NULL;

	return createPrinterContext(szPrinter);
}
HDC Win32PrintJob::createPrinterContext(LPTSTR devName) {
	DEVMODE *devmode = settings->lockDevmode();
	HDC printerDC = CreateDC(TEXT("WINSPOOL"), devName, NULL, devmode);
	settings->unlockDevmode();
	return printerDC;
}

PrintSettings *Win32PrintingManager::getDefaultPrintSettings() const {
	HGLOBAL devmode = getDefaultDevmodeGlobal();

	if (!devmode)
		return nullptr;

	return new Win32PrintSettings(devmode);
}

HGLOBAL Win32PrintingManager::getDefaultDevmodeGlobal() const {
	TCHAR szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success = GetDefaultPrinter(szPrinter, &cchPrinter);
	if (!success)
		return nullptr;

	return getDefaultDevmodeGlobal(szPrinter);
}

HGLOBAL Win32PrintingManager::getDefaultDevmodeGlobal(LPTSTR devName) const {
	HANDLE handle;
	BOOL success;

	HWND parent = (dynamic_cast<OSystem_Win32 *>(g_system))->getHwnd();

	success = OpenPrinter(devName, &handle, NULL);
	if (!success)
		return nullptr;

	int size = DocumentProperties(parent, handle, devName, NULL, NULL, 0);
	HGLOBAL devmodeGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	DEVMODE *devmode = (DEVMODE *)GlobalLock(devmodeGlobal);
	DocumentProperties(parent, handle, devName, devmode, NULL, DM_OUT_BUFFER);

	// Apply setting presets here

	if (false) {
		DocumentProperties(parent, handle, devName, devmode, devmode, DM_IN_BUFFER | DM_IN_PROMPT | DM_OUT_BUFFER);
	}

	GlobalUnlock(devmodeGlobal);

	ClosePrinter(handle);

	return devmodeGlobal;
}


HBITMAP Win32PrintJob::buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (colorCount - 1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = surf.w;
	bitmapInfo->bmiHeader.biHeight = -((LONG)surf.h); // Blame the OS2 team for bitmaps being upside down
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = (surf.format.isCLUT8()?8:surf.format.bpp());
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
	bitmapInfo->bmiHeader.biSizeImage = 0;
	bitmapInfo->bmiHeader.biClrUsed = (surf.format.isCLUT8()?colorCount:0);
	bitmapInfo->bmiHeader.biClrImportant = (surf.format.isCLUT8() ? colorCount : 0);

	if (surf.hasPalette()) {
		byte *colors = new byte[colorCount * 3];
		surf.grabPalette(colors, 0, colorCount);

		byte *palette = colors;
		for (uint colorIndex = 0; colorIndex < colorCount; ++colorIndex, palette += 3) {
			bitmapInfo->bmiColors[colorIndex].rgbRed = palette[0];
			bitmapInfo->bmiColors[colorIndex].rgbGreen = palette[1];
			bitmapInfo->bmiColors[colorIndex].rgbBlue = palette[2];
			bitmapInfo->bmiColors[colorIndex].rgbReserved = 0;
		}

		delete[] colors;
	}

	HBITMAP bitmap = CreateDIBitmap(hdc, &(bitmapInfo->bmiHeader), CBM_INIT, surf.getPixels(), bitmapInfo, DIB_RGB_COLORS);

	free(bitmapInfo);
	return bitmap;
}

void Win32PrintJob::print() {
	beginDocument();
	(*printCallback)(this);
}

PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

PrintSettings::DuplexMode Win32PrintSettings::getDuplexMode() const {
	PrintSettings::DuplexMode mode;
	DEVMODE *devmode = lockDevmode();
	mode = (PrintSettings::DuplexMode)devmode->dmDuplex;
	unlockDevmode();
	return mode;
}

void Win32PrintSettings::setDuplexMode(PrintSettings::DuplexMode mode) {
	DEVMODE *devmode = lockDevmode();
	devmode->dmDuplex = mode;
	unlockDevmode();
}

bool Win32PrintSettings::getLandscapeOrientation() const {
	bool landscape;
	DEVMODE *devmode = lockDevmode();
	landscape = devmode->dmOrientation == DMORIENT_LANDSCAPE;
	unlockDevmode();
	return landscape;
}

void Win32PrintSettings::setLandscapeOrientation(bool landscapeOrientation) {
	DEVMODE *devmode = lockDevmode();
	devmode->dmOrientation = landscapeOrientation ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
	unlockDevmode();
}

bool Win32PrintSettings::getColorPrinting() const {
	bool color;
	DEVMODE *devmode = lockDevmode();
	color = devmode->dmColor == DMCOLOR_COLOR;
	unlockDevmode();
	return color;
}

void Win32PrintSettings::setColorPrinting(bool colorPrinting) {
	DEVMODE *devmode = lockDevmode();
	devmode->dmColor = colorPrinting ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;
	unlockDevmode();
}

void Win32PrintSettings::setDevModeGlobal(HGLOBAL newGlobal) {
	GlobalFree(devmodeGlobal);
	devmodeGlobal = newGlobal;
}

DEVMODE *Win32PrintSettings::lockDevmode() const {
	return (DEVMODE *) GlobalLock(devmodeGlobal);
}

void Win32PrintSettings::unlockDevmode() const {
	GlobalUnlock(devmodeGlobal);
}


#endif // USE_PRINTING
#endif // WIN32
