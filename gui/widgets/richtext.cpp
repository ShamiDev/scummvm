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

#include "common/system.h"
#include "common/unicode-bidi.h"

#include "graphics/macgui/mactext.h"

#include "gui/gui-manager.h"

#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"

#include "gui/widgets/richtext.h"
#include "gui/widgets/scrollbar.h"

namespace GUI {

Graphics::MacWindowManager *_wm = nullptr;

void ensureWM() {
	if (_wm)
		return;

	if (ConfMan.hasKey("extrapath")) {
		Common::FSNode dir(ConfMan.get("extrapath"));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	uint32 wmMode = Graphics::kWMModeNoDesktop | Graphics::kWMMode32bpp | Graphics::kWMModeNoCursorOverride;

	_wm = new Graphics::MacWindowManager(wmMode);
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, x, y, w, h, scale, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip)
	: RichTextWidget(boss, x, y, w, h, false, text, tooltip) {
}

RichTextWidget::RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, name, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

void RichTextWidget::init() {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_TRACK_MOUSE | WIDGET_DYN_TOOLTIP);

	_type = kRichTextWidget;

	_verticalScroll = new ScrollBarWidget(this, _w, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;

	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	_textWidth = _w - _scrollbarWidth - _x;

	ensureWM();

	_limitH = 140;
}

void RichTextWidget::handleMouseWheel(int x, int y, int direction) {
	_verticalScroll->handleMouseWheel(x, y, direction);
}

void RichTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	warning("M: %s", _txtWnd->getMouseLink(x + _x + _scrolledX, y + _y + _scrolledY).encode().c_str());
}

void RichTextWidget::handleTooltipUpdate(int x, int y) {
	_tooltip = _txtWnd->getMouseLink(x + _x + _scrolledX, y + _y + _scrolledY);
	//warning("t: %s", _tooltip.encode().c_str());
}

void RichTextWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void RichTextWidget::recalc() {
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_limitH = _h;
	_textWidth = _w - _scrollbarWidth - _x;

	int h = _txtWnd->getTextHeight();

	if (h <= _limitH) _scrolledY = 0;
	if (_scrolledY > h - _limitH) _scrolledY = 0;

	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = 30 * 3;
	_verticalScroll->setPos(_textWidth, 0);
	_verticalScroll->setSize(_scrollbarWidth, _limitH-1);

	_txtWnd->setMaxWidth(_textWidth);
}

void RichTextWidget::createWidget() {
	uint32 bg = _wm->_pixelformat.ARGBToColor(0, 0xff, 0xff, 0xff); // transparent
	TextColorData *normal = g_gui.theme()->getTextColorData(kTextColorNormal);
	uint32 fg = _wm->_pixelformat.RGBToColor(normal->r, normal->g, normal->b);

	Graphics::MacFont macFont(Graphics::kMacFontNewYork, 30, Graphics::kMacFontRegular);

	_txtWnd = new Graphics::MacText(Common::U32String(), _wm, &macFont, fg, bg, _textWidth, Graphics::kTextAlignLeft);

	if (!_imageArchive.empty())
		_txtWnd->setImageArchive(_imageArchive);

	_txtWnd->setMarkdownText(_text);

	_surface = new Graphics::ManagedSurface(_w, _h, _wm->_pixelformat);

	recalc();
}

void RichTextWidget::reflowLayout() {
	Widget::reflowLayout();

	if (!_txtWnd)
		createWidget();

	recalc();

	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
	_verticalScroll->recalc();
}

void RichTextWidget::drawWidget() {
	if (!_txtWnd)
		createWidget();

	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), ThemeEngine::kWidgetBackgroundPlain);

	_surface->clear(_wm->_pixelformat.ARGBToColor(0, 0xff, 0xff, 0xff)); // transparent

	_txtWnd->draw(_surface, 0, _scrolledY, _w - _scrollbarWidth, _h, 0, 0);

	g_gui.theme()->drawManagedSurface(Common::Point(_x, _y), *_surface);
}

void RichTextWidget::draw() {
	Widget::draw();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->draw();
	}
}

void RichTextWidget::markAsDirty() {
	Widget::markAsDirty();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->markAsDirty();
	}
}

bool RichTextWidget::containsWidget(Widget *w) const {
	if (w == _verticalScroll || _verticalScroll->containsWidget(w))
		return true;
	return false;
}

Widget *RichTextWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w - _scrollbarWidth - _x)
		return _verticalScroll;

	return this;
}

} // End of namespace GUI
