/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 */


#ifndef GRAPHICS_IFF_H
#define GRAPHICS_IFF_H

#include "common/iff_container.h"

namespace Graphics {

struct Surface;


struct BMHD {
	uint16 width, height;
	uint16 x, y;
	byte depth;
	byte masking;
	byte pack;
	byte flags;
	uint16 transparentColor;
	byte xAspect, yAspect;
	uint16 pageWidth, pageHeight;

	BMHD() {
		memset(this, 0, sizeof(*this));
	}
};


//	handles ILBM subtype of IFF FORM files
//
class ILBMDecoder : public Common::IFFParser {

protected:
	void readBMHD(Common::IFFChunk &chunk);
	void readCMAP(Common::IFFChunk &chunk);
	void readBODY(Common::IFFChunk &chunk);

	BMHD 	_bitmapHeader;
	uint32 	_colorCount;

	Surface *_surface;
	byte    **_colors;

	void fillPlane(byte *out, byte* buf, uint32 width, uint32 plane);

public:
	ILBMDecoder(Common::ReadStream &input, Surface &surface, byte *&colors);
	virtual ~ILBMDecoder() { }
	void decode();
};


//	handles PBM subtype of IFF FORM files
//
class PBMDecoder : public Common::IFFParser {

protected:
	void readBMHD(Common::IFFChunk &chunk);
	void readCMAP(Common::IFFChunk &chunk);
	void readBODY(Common::IFFChunk &chunk);

	BMHD 	_bitmapHeader;
	uint32 	_colorCount;

	Surface *_surface;
	byte    **_colors;

public:
	PBMDecoder(Common::ReadStream &input, Surface &surface, byte *&colors);
	virtual ~PBMDecoder() { }
	void decode();
};

/*
	TODO: rename this routine to decodePBM, and update the SAGA code that's
	using it. The routine has already been implemented using the above PBMDecoder.
*/
void decodeILBM(Common::ReadStream &input, Surface &surface, byte *&colors);


/*
	PackBits is a RLE compression algorithm introduced
	by Apple. It is also used to encode ILBM and PBM
	subtypes of IFF files, and some flavours of TIFF.

	The following implementation uses a static storage
	and is buffered, that means you can't destroy the
	input stream before you are done with it.
*/
class PackBitsReadStream : public Common::ReadStream {

protected:
	Common::ReadStream *_input;

	byte	_storage[257];
	byte	*_wStoragePos;
	byte	*_rStoragePos;

	byte*	_out;
	byte*	_outEnd;
	int32	_fed;
	int32	_unpacked;

	void store(byte b);
	void feed();
	void unpack();

public:
	PackBitsReadStream(Common::ReadStream &input);
	~PackBitsReadStream();

	virtual bool eos() const;
	uint32 read(void *dataPtr, uint32 dataSize);
};

}

#endif
