/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_DIALOGINTERFACE_H
#define STARK_DIALOGINTERFACE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/array.h"
#include "common/rect.h"

namespace Stark {

class VisualImageXMG;
class ClickText;

namespace Gfx {
class Texture;
}

class DialogInterface {
	VisualImageXMG *_passiveBackGroundTexture;
	VisualImageXMG *_activeBackGroundTexture;
	Gfx::Texture *_texture;
	Common::Array<ClickText*> _options;
	bool _hasOptions;
	void clearOptions();
	void renderOptions();
public:
	DialogInterface();
	virtual ~DialogInterface();
	void render();
	void update();
	void notifySubtitle(const Common::String &subtitle);
	void notifyDialogOptions(const Common::StringArray &options);
	bool containsPoint(Common::Point point);
	void handleMouseOver(Common::Point point);
	void handleClick(Common::Point point);
};

} // End of namespace Stark

#endif // STARK_DIALOG_INTERFACE_H