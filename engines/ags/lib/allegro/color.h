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

#ifndef AGS_LIB_ALLEGRO_COLOR_H
#define AGS_LIB_ALLEGRO_COLOR_H

#include "common/scummsys.h"
#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

#define PALETTE_COUNT 256
#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

#include "common/pack-start.h"  // START STRUCT PACKING

struct color {
	byte r, g, b;
} PACKED_STRUCT;

typedef color RGB;
typedef color PALETTE[PALETTE_COUNT];

AL_VAR(PALETTE, _current_palette);

#include "common/pack-end.h"    // END STRUCT PACKING

//define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

extern int bestfit_color(const PALETTE pal, int r, int g, int b);

extern int _rgb_r_shift_15;
extern int _rgb_g_shift_15;
extern int _rgb_b_shift_15;
extern int _rgb_r_shift_16;
extern int _rgb_g_shift_16;
extern int _rgb_b_shift_16;
extern int _rgb_r_shift_24;
extern int _rgb_g_shift_24;
extern int _rgb_b_shift_24;
extern int _rgb_r_shift_32;
extern int _rgb_g_shift_32;
extern int _rgb_b_shift_32;
extern int _rgb_a_shift_32;

AL_FUNC(void, set_color, (int idx, AL_CONST RGB *p));
AL_FUNC(void, set_palette, (AL_CONST PALETTE p));
AL_FUNC(void, set_palette_range, (AL_CONST PALETTE p, int from, int to, int retracesync));

extern int makeacol(int r, int g, int b, int a);
extern int makeacol_depth(int color_depth, int r, int g, int b, int a);

extern int makecol15(int r, int g, int b);
extern int makecol16(int r, int g, int b);
extern int makecol24(int r, int g, int b);
extern int makecol32(int r, int g, int b);
extern int makeacol32(int r, int g, int b, int a);
extern int getr8(int c);
extern int getg8(int c);
extern int getb8(int c);
extern int makecol(byte r, byte g, byte b);
extern int makecol8(byte r, byte g, byte b);

extern int getr_depth(int color_depth, int c);
extern int getg_depth(int color_depth, int c);
extern int getb_depth(int color_depth, int c);
extern int geta_depth(int color_depth, int c);

} // namespace AGS3

#endif
