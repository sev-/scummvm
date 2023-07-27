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
 
#include "testbed/printing.h"

#ifdef USE_PRINTING

#include "base/version.h"

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "common/file.h"

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "base/plugins.h"

#include "graphics/pm5544.h"

#include "gui/gui-manager.h"

#include "backends/printing/printman.h"

namespace GUI {
	Graphics::ManagedSurface *loadSurfaceFromFile(const Common::String &name, int renderWidth = 0, int renderHeight = 0);
}

namespace Testbed {


PrintingTestSuite::PrintingTestSuite() {
	addTest("Abort Job", &PrintingTests::abortJob);
	addTest("Print Test Page", &PrintingTests::printTestPage);
	addTest("Print Engine List", &PrintingTests::printEngineList);
	addTest("Print the GPL", &PrintingTests::printGPL);
}

TestExitStatus PrintingTests::abortJob() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	auto lambda = [](PrintJob *job) -> void {
		job->abortJob();
	};

	PrintCallback cb = new Common::Functor1Lamb<PrintJob *, void, decltype(lambda)>(lambda);

	pm->printCustom(cb, "ScummVM to be aborted");

	return kTestPassed;
}


TestExitStatus PrintingTests::printTestPage() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	if (!g_gui.theme()->supportsImages()) {
		Testsuite::logPrintf("No logo to load in this theme, skipping test : printTestPage\n");
		return kTestSkipped;
	}

	if (Testsuite::handleInteractiveInput("Print a test page?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : printTestPage\n");
		return kTestSkipped;
	}

	const Graphics::ManagedSurface *logo = g_gui.theme()->getImageSurface("logo.bmp");
	if (!logo) {
		warning("Failed to load the scummvm logo.");
		return kTestFailed;
	}
	
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	auto lambda = [&logo](PrintJob *job) -> void {
		job->beginPage();

		const PrintSettings *settings = job->getPrintSettings();

		const Common::Rect printArea = job->getPrintableArea();

		Common::Point pos(20, 0);
		
		auto printLine = [&job, &pos](Common::String line) -> void {
			job->drawText(line, pos);
			pos += Common::Point(0, job->getTextBounds(line).height());
		};

		{
			Common::Rect logoArea(pos.x, pos.y, pos.x + logo->w * 4, pos.y + logo->h * 4);
			// Logo is 32 bpp
			job->drawBitmap(*logo, logoArea);
			pos += Common::Point(0, logoArea.height());
		}

		printLine(gScummVMVersionDate);
		printLine(gScummVMCompiler);
		printLine(gScummVMFeatures);

		if (settings->getColorPrinting()) {
			job->setTextColor(255, 0, 0);
			printLine("Red text");

			job->setTextColor(0, 255, 0);
			printLine("Green text");

			job->setTextColor(0, 0, 255);
			printLine("Blue text");

			job->setTextColor(0, 0, 0);
			printLine("Black text");
		} else {
			printLine("Grayscale printing only, no text color test");
		}

		// The test pattern is CLUT-8
		Graphics::ManagedSurface *testPattern = Graphics::renderPM5544(800, 800);
		job->drawBitmap(*testPattern, Common::Rect(pos.x, pos.y, pos.x + testPattern->w, pos.y + testPattern->h));
		delete testPattern;

		job->endPage();
		job->endDoc();
	};

	PrintCallback cb = new Common::Functor1Lamb<PrintJob *, void, decltype(lambda)>(lambda);

	pm->printCustom(cb, "ScummVM Testpage");
	
	return kTestPassed;
}

TestExitStatus PrintingTests::printEngineList() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}

	if (Testsuite::handleInteractiveInput("Print an engine list?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : printEngineList\n");
		return kTestSkipped;
	}

	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	auto lambda = [](PrintJob *job) -> void {
		job->beginPage();

		const Common::Rect printArea = job->getPrintableArea();

		Common::Point pos(20, 0);

		auto printLine = [&job, &pos](Common::String line) -> void {
			job->drawText(line, pos);
			pos += Common::Point(0, job->getTextBounds(line).height());
		};
		printLine("Engines:");

		const int logoSize = 500;
		const int logoPadding = 20;

		const PluginList &plugins = EngineMan.getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);
		PluginList::const_iterator iter = plugins.begin();
		for (; iter != plugins.end(); ++iter) {
			auto &plug = (*iter);
			auto &meta = plug->get<MetaEngineDetection>();

			// Try normal engine icon first
			Common::String path = Common::String::format("icons/%s.png", plug->getName());

			Graphics::ManagedSurface *logoSurf = GUI::loadSurfaceFromFile(path);

			if (!logoSurf) {
				// Check for a single game engine too
				path = Common::String::format("icons/%s-%s.png", plug->getName(), plug->getName());
				logoSurf = GUI::loadSurfaceFromFile(path);
			}

			if (logoSurf) {
				Common::Rational xRatio(logoSize, logoSurf->w);
				Common::Rational yRatio(logoSize, logoSurf->h);

				Common::Rational scaleFactor = ((xRatio < yRatio) ? xRatio : yRatio);

				Common::Rect logoArea(logoSurf->w * scaleFactor.toDouble(), logoSurf->h * scaleFactor.toDouble());

				logoArea.moveTo(pos);

				job->drawBitmap(*logoSurf, logoArea);

				delete logoSurf;
			} else {
				Common::String displayName(meta.getEngineName());
				displayName.wordWrap(12);
				auto lines = splitString(displayName, '\n');
				Common::Point linePos=pos;
				for (Common::String line : lines) {
					job->drawText(line, linePos);
					linePos += Common::Point(0, job->getTextBounds(line).height());
				}
			}

			pos.x += logoSize + logoPadding;

			if (!printArea.contains(pos.x + logoSize, pos.y + logoSize)) {
				pos.y += logoSize + logoPadding;
				pos.x = 0;
			}
		}

		job->endPage();
		job->endDoc();
	};

	PrintCallback cb = new Common::Functor1Lamb<PrintJob *, void, decltype(lambda)>(lambda);

	pm->printCustom(cb, "ScummVM Engines");

	return kTestPassed;
}

TestExitStatus PrintingTests::printGPL() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}

	Common::File f;
	if (!f.open("COPYING")) {
		warning("Failed to load COPYING");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Print the gpl to test long jobs?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : printGPL\n");
		f.close();
		return kTestSkipped;
	}

	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	pm->printPlainTextFile("The GPL", f);

	f.close();

	return kTestPassed;
}

} // End of namespace Testbed
#endif
