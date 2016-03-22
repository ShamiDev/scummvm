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

#include "titanic/screen_manager.h"
#include "titanic/video_surface.h"

namespace Titanic {

CScreenManager *CScreenManager::_screenManagerPtr;
CScreenManager *CScreenManager::_currentScreenManagerPtr;

CScreenManager::CScreenManager(TitanicEngine *vm): _vm(vm) {
	_screenManagerPtr = nullptr;
	_currentScreenManagerPtr = nullptr;

	_frontRenderSurface = nullptr;
	_mouseCursor = nullptr;
	_textCursor = nullptr;
	_inputHandler = nullptr;
	_fontNumber = 0;
	// TODO: Further initialization

	_screenManagerPtr = this;
}

CScreenManager::~CScreenManager() {
	_screenManagerPtr = nullptr;
}

void CScreenManager::setWindowHandle(int v) {
	// Not needed
}

bool CScreenManager::resetWindowHandle(int v) {
	hideCursor();
	return true;
}

CScreenManager *CScreenManager::setCurrent() {
	if (!_currentScreenManagerPtr)
		_currentScreenManagerPtr = _screenManagerPtr;

	return _currentScreenManagerPtr;
}

void CScreenManager::setSurfaceBounds(int surfaceNum, const Rect &r) {
	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		_backSurfaces[surfaceNum]._bounds = r;
}

/*------------------------------------------------------------------------*/

OSScreenManager::OSScreenManager(TitanicEngine *vm): CScreenManager(vm),
		_directDrawManager(vm, false) {
	_field48 = 0;
	_field4C = 0;
	_field50 = 0;
	_field54 = 0;
}

OSScreenManager::~OSScreenManager() {
	destroyFrontAndBackBuffers();
	delete _mouseCursor;
	delete _textCursor;
}

void OSScreenManager::setMode(int width, int height, int bpp, uint numBackSurfaces, bool flag2) {
	destroyFrontAndBackBuffers();
	_directDrawManager.initVideo(width, height, bpp, numBackSurfaces);

	_frontRenderSurface = new OSVideoSurface(this, nullptr);
	_frontRenderSurface->setSurface(this, _directDrawManager._mainSurface);

	_backSurfaces.resize(numBackSurfaces);
	for (uint idx = 0; idx < numBackSurfaces; ++idx) {
		_backSurfaces[idx]._surface = new OSVideoSurface(this, nullptr);
		_backSurfaces[idx]._surface->setSurface(this, _directDrawManager._backSurfaces[idx]);
	}

	// Load fonts
	_fonts[0].load(149);
	_fonts[1].load(151);
	_fonts[2].load(152);
	_fonts[3].load(153);

	// Load the cursors
	loadCursors();
}

void OSScreenManager::drawCursors() {
	warning("OSScreenManager::drawCursors");
}

void OSScreenManager::proc6() {}
void OSScreenManager::proc7() {}

CVideoSurface *OSScreenManager::getSurface(int surfaceNum) const {
	if (surfaceNum == -1)
		return _frontRenderSurface;
	else if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		return _backSurfaces[surfaceNum]._surface;
	else
		return nullptr;
}

void OSScreenManager::proc9() {}
void OSScreenManager::proc10() {}

void OSScreenManager::blitFrom(int surfaceNum, CVideoSurface *src,
	const Point *destPos, const Rect *srcRect) {
	// Get the dest surface
	CVideoSurface *destSurface = _frontRenderSurface;
	if (surfaceNum < -1)
		return;
	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		destSurface = _backSurfaces[surfaceNum]._surface;
	if (!destSurface->hasSurface())
		return;
	
	Point destPoint = destPos ? *destPos : Point(0, 0);
	Rect srcBounds = srcRect ? *srcRect : Rect(0, 0, src->getWidth(), src->getHeight());
	Rect *bounds = &srcBounds;
	Rect rect2;

	if (surfaceNum >= 0 && !_backSurfaces[surfaceNum]._bounds.isEmpty()) {
		// Perform clipping to the bounds of the back surface
		rect2 = srcBounds;
		rect2.translate(-srcBounds.left, -srcBounds.top);
		rect2.translate(destPoint.x, destPoint.y);
		rect2.combine2(_backSurfaces[surfaceNum]._bounds);

		rect2.translate(-destPoint.x, -destPoint.y);
		rect2.translate(srcBounds.left, srcBounds.top);

		if (rect2.isEmpty())
			return;

		destPoint.x += rect2.left - srcBounds.left;
		destPoint.y += rect2.top - srcBounds.top;
		bounds = &rect2;
	}

	if (!bounds->isEmpty())
		destSurface->blitFrom(destPoint, src, bounds);
}

void OSScreenManager::proc12() {}
void OSScreenManager::proc13() {}
void OSScreenManager::proc14() {}
void OSScreenManager::proc15() {}
void OSScreenManager::proc16() {}
void OSScreenManager::getFont() {}
void OSScreenManager::proc18() {}
void OSScreenManager::proc19() {}

void OSScreenManager::clearSurface(int surfaceNum, Rect *bounds) {
	if (surfaceNum == -1)
		_directDrawManager._mainSurface->fill(bounds, 0);
	else if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		_directDrawManager._backSurfaces[surfaceNum]->fill(bounds, 0);
}

void OSScreenManager::resizeSurface(CVideoSurface *surface, int width, int height) {
	DirectDrawSurface *ddSurface = _directDrawManager.createSurface(width, height, 0);
	surface->setSurface(this, ddSurface);
}

CVideoSurface *OSScreenManager::createSurface(int w, int h) {
	DirectDrawSurface *ddSurface = _directDrawManager.createSurface(w, h, 0);
	return new OSVideoSurface(this, ddSurface);
}

CVideoSurface *OSScreenManager::createSurface(const CResourceKey &key) {
	return new OSVideoSurface(this, key);
}

void OSScreenManager::proc23() {}
void OSScreenManager::proc24() {}
void OSScreenManager::proc25() {}

void OSScreenManager::showCursor() {

}

void OSScreenManager::hideCursor() {

}

void OSScreenManager::destroyFrontAndBackBuffers() {
	delete _frontRenderSurface;
	_frontRenderSurface = nullptr;

	for (uint idx = 0; idx < _backSurfaces.size(); ++idx)
		delete _backSurfaces[idx]._surface;
	_backSurfaces.clear();
}

void OSScreenManager::loadCursors() {
	if (_mouseCursor) {
		hideCursor();
		delete _mouseCursor;
	}
	_mouseCursor = new CMouseCursor();
	showCursor();

	if (!_textCursor) {
		_textCursor = new CTextCursor();
	}
}

} // End of namespace Titanic
