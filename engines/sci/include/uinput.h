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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* unified input header file */

#ifndef _SCI_UINPUT_H
#define _SCI_UINPUT_H

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

namespace Sci {

struct _state;

#define SCI_INPUT_DEFAULT_CLOCKTIME 100000
#define SCI_INPUT_DEFAULT_REDRAWTIME 30000


typedef struct {
	short type;
	short data;
	short buckybits;
	short character; /* for keyboard events: 'data' after applying
			 ** the effects of 'buckybits', e.g. if
			 **   type == SCI_EVT_KEYBOARD
			 **   data == 'a'
			 **   buckybits == SCI_EVM_LSHIFT
			 ** then
			 **   character == 'A'
			 ** For 'Alt', characters are interpreted by their
			 ** PC keyboard scancodes.
			 */
} sci_event_t;

/*Values for type*/
#define SCI_EVT_NONE            0
#define SCI_EVT_MOUSE_PRESS     (1<<0)
#define SCI_EVT_MOUSE_RELEASE   (1<<1)
#define SCI_EVT_KEYBOARD        (1<<2)
#define SCI_EVT_JOYSTICK        (1<<6)
#define SCI_EVT_SAID            (1<<7)
/*Fake values for other events*/
#define SCI_EVT_ERROR           (1<<10)
#define SCI_EVT_QUIT            (1<<11)
#define SCI_EVT_NONBLOCK	(1<<15)
/* The QUIT event may be used to signal an external 'quit' command being
** issued to the gfx driver.  */
#define SCI_EVT_ANY             0x7fff



/* Keycodes of special keys: */
#define SCI_K_ESC 27
#define SCI_K_BACKSPACE 8
#define SCI_K_ENTER 13
#define SCI_K_TAB '\t'
#define SCI_K_SHIFT_TAB (0xf << 8)

#define SCI_K_END (79 << 8)
#define SCI_K_DOWN (80 << 8)
#define SCI_K_PGDOWN (81 << 8)
#define SCI_K_LEFT (75 << 8)
#define SCI_K_CENTER (76 << 8)
#define SCI_K_RIGHT (77 << 8)
#define SCI_K_HOME (71 << 8)
#define SCI_K_UP (72 << 8)
#define SCI_K_PGUP (73 << 8)
#define SCI_K_INSERT (82 << 8)
#define SCI_K_DELETE (83 << 8)

#define SCI_K_F1 (59<<8)
#define SCI_K_F2 (60<<8)
#define SCI_K_F3 (61<<8)
#define SCI_K_F4 (62<<8)
#define SCI_K_F5 (63<<8)
#define SCI_K_F6 (64<<8)
#define SCI_K_F7 (65<<8)
#define SCI_K_F8 (66<<8)
#define SCI_K_F9 (67<<8)
#define SCI_K_F10 (68<<8)

#define SCI_K_SHIFT_F1 (84<<8)
#define SCI_K_SHIFT_F2 (85<<8)
#define SCI_K_SHIFT_F3 (86<<8)
#define SCI_K_SHIFT_F4 (87<<8)
#define SCI_K_SHIFT_F5 (88<<8)
#define SCI_K_SHIFT_F6 (89<<8)
#define SCI_K_SHIFT_F7 (90<<8)
#define SCI_K_SHIFT_F8 (91<<8)
#define SCI_K_SHIFT_F9 (92<<8)
#define SCI_K_SHIFT_F10 (93<<8)

/*Values for buckybits */
#define SCI_EVM_RSHIFT          (1<<0)
#define SCI_EVM_LSHIFT          (1<<1)
#define SCI_EVM_CTRL            (1<<2)
#define SCI_EVM_ALT             (1<<3)
#define SCI_EVM_SCRLOCK         (1<<4)
#define SCI_EVM_NUMLOCK         (1<<5)
#define SCI_EVM_CAPSLOCK        (1<<6)
#define SCI_EVM_INSERT          (1<<7)

#define SCI_EVM_NO_FOOLOCK      (~(SCI_EVM_SCRLOCK | SCI_EVM_NUMLOCK | SCI_EVM_CAPSLOCK | SCI_EVM_INSERT))
#define SCI_EVM_ALL             0xFF

} // End of namespace Sci

#endif /* _SCI_UINPUT_H */
