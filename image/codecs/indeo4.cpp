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

#include "common/scummsys.h"

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "image/codecs/indeo4.h"
#include "image/codecs/indeo/indeo_dsp.h"
#include "image/codecs/indeo/mem.h"

namespace Image {

#define IVI4_PIC_SIZE_ESC   7

Indeo4Decoder::Indeo4Decoder(uint16 width, uint16 height) : IndeoDecoderBase(width, height) {
	_ctx._isIndeo4 = true;
	_ctx._refBuf = 1;
	_ctx._bRefBuf = 3;
	_ctx._pFrame = new AVFrame();
}

bool Indeo4Decoder::isIndeo4(Common::SeekableReadStream &stream) {
	// Less than 16 bytes? This can't be right
	if (stream.size() < 16)
		return false;

	// Read in the start of the data
	byte buffer[16];
	stream.read(buffer, 16);
	stream.seek(-16, SEEK_CUR);

	// Validate the first 18-bit word has the correct identifier
	Indeo::GetBits gb(new Common::MemoryReadStream(buffer, 16 * 8), DisposeAfterUse::YES);
	bool isIndeo4 = gb.getBits(18) == 0x3FFF8;

	return isIndeo4;
}

const Graphics::Surface *Indeo4Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// Not Indeo 4? Fail
	if (!isIndeo4(stream))
		return nullptr;

	// Set up the frame data buffer
	byte *frameData = new byte[stream.size()];
	stream.read(frameData, stream.size());
	_ctx._frameData = frameData;
	_ctx._frameSize = stream.size();

	// Set up the GetBits instance for reading the data
	_ctx._gb = new GetBits(new Common::MemoryReadStream(_ctx._frameData, _ctx._frameSize * 8));

	// Decode the frame
	int err = decodeIndeoFrame();

	// Free the bit reader and frame buffer
	delete _ctx._gb;
	_ctx._gb = nullptr;
	delete[] frameData;
	_ctx._frameData = nullptr;
	_ctx._frameSize = 0;

	return (err < 0) ? nullptr : &_surface->rawSurface();
}

int Indeo4Decoder::decodePictureHeader() {
	int             pic_size_indx, i, p;
	IVIPicConfig    _picConf;

	if (_ctx._gb->getBits(18) != 0x3FFF8) {
		warning("Invalid picture start code!");
		return -1;
	}

	_ctx._prevFrameType = _ctx._frameType;
	_ctx._frameType = _ctx._gb->getBits(3);
	if (_ctx._frameType == 7) {
		warning("Invalid frame type: %d", _ctx._frameType);
		return -1;
	}

	if (_ctx._frameType == IVI4_FRAMETYPE_BIDIR)
		_ctx._hasBFrames = true;

	_ctx._hasTransp = _ctx._gb->getBit();

	// unknown bit: Mac decoder ignores this bit, XANIM returns error
	if (_ctx._gb->getBit()) {
		warning("Sync bit is set!");
		return -1;
	}

	_ctx._dataSize = _ctx._gb->getBit() ? _ctx._gb->getBits(24) : 0;

	// null frames don't contain anything else so we just return
	if (_ctx._frameType >= IVI4_FRAMETYPE_NULL_FIRST) {
		warning("Null frame encountered!");
		return 0;
	}

	// Check key lock status. If enabled - ignore lock word.
	// Usually we have to prompt the user for the password, but
	// we don't do that because Indeo 4 videos can be decoded anyway
	if (_ctx._gb->getBit()) {
		_ctx._gb->skip(32);
		warning("Password-protected clip!");
	}

	pic_size_indx = _ctx._gb->getBits(3);
	if (pic_size_indx == IVI4_PIC_SIZE_ESC) {
		_picConf._picHeight = _ctx._gb->getBits(16);
		_picConf._picWidth = _ctx._gb->getBits(16);
	} else {
		_picConf._picHeight = _ivi4_common_pic_sizes[pic_size_indx * 2 + 1];
		_picConf._picWidth = _ivi4_common_pic_sizes[pic_size_indx * 2];
	}

	// Decode tile dimensions.
	_ctx._usesTiling = _ctx._gb->getBit();
	if (_ctx._usesTiling) {
		_picConf._tileHeight = scaleTileSize(_picConf._picHeight, _ctx._gb->getBits(4));
		_picConf._tileWidth = scaleTileSize(_picConf._picWidth, _ctx._gb->getBits(4));
	} else {
		_picConf._tileHeight = _picConf._picHeight;
		_picConf._tileWidth = _picConf._picWidth;
	}

	// Decode chroma subsampling. We support only 4:4 aka YVU9.
	if (_ctx._gb->getBits(2)) {
		warning("Only YVU9 picture format is supported!");
		return -1;
	}
	_picConf._chromaHeight = (_picConf._picHeight + 3) >> 2;
	_picConf._chromaWidth = (_picConf._picWidth + 3) >> 2;

	// decode subdivision of the planes
	_picConf._lumaBands = decodePlaneSubdivision();
	_picConf._chromaBands = 0;
	if (_picConf._lumaBands)
		_picConf._chromaBands = decodePlaneSubdivision();
	_ctx._isScalable = _picConf._lumaBands != 1 || _picConf._chromaBands != 1;
	if (_ctx._isScalable && (_picConf._lumaBands != 4 || _picConf._chromaBands != 1)) {
		warning("Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d",
			_picConf._lumaBands, _picConf._chromaBands);
		return -1;
	}

	// check if picture layout was changed and reallocate buffers
	if (_picConf.ivi_pic_config_cmp(_ctx._picConf)) {
		if (IVIPlaneDesc::initPlanes(_ctx._planes, &_picConf, 1)) {
			warning("Couldn't reallocate color planes!");
			_ctx._picConf._lumaBands = 0;
			return -2;
		}

		_ctx._picConf = _picConf;

		// set default macroblock/block dimensions
		for (p = 0; p <= 2; p++) {
			for (i = 0; i < (!p ? _picConf._lumaBands : _picConf._chromaBands); i++) {
				_ctx._planes[p]._bands[i]._mbSize = !p ? (!_ctx._isScalable ? 16 : 8) : 4;
				_ctx._planes[p]._bands[i]._blkSize = !p ? 8 : 4;
			}
		}

		if (IVIPlaneDesc::initTiles(_ctx._planes, _ctx._picConf._tileWidth,
			_ctx._picConf._tileHeight)) {
			warning("Couldn't reallocate internal structures!");
			return -2;
		}
	}

	_ctx._frameNum = _ctx._gb->getBit() ? _ctx._gb->getBits(20) : 0;

	// skip decTimeEst field if present
	if (_ctx._gb->getBit())
		_ctx._gb->skip(8);

	// decode macroblock and block huffman codebooks
	if (_ctx._mbVlc.decodeHuffDesc(&_ctx, _ctx._gb->getBit(), IVI_MB_HUFF) ||
		_ctx._blkVlc.decodeHuffDesc(&_ctx, _ctx._gb->getBit(), IVI_BLK_HUFF))
		return -1;

	_ctx._rvmapSel = _ctx._gb->getBit() ? _ctx._gb->getBits(3) : 8;

	_ctx._inImf = _ctx._gb->getBit();
	_ctx._inQ = _ctx._gb->getBit();

	_ctx._picGlobQuant = _ctx._gb->getBits(5);

	// TODO: ignore this parameter if unused
	_ctx._unknown1 = _ctx._gb->getBit() ? _ctx._gb->getBits(3) : 0;

	_ctx._checksum = _ctx._gb->getBit() ? _ctx._gb->getBits(16) : 0;

	// skip picture header extension if any
	while (_ctx._gb->getBit()) {
		_ctx._gb->skip(8);
	}

	if (_ctx._gb->getBit()) {
		warning("Bad blocks bits encountered!");
	}

	_ctx._gb->align();

	return 0;
}

void Indeo4Decoder::switchBuffers() {
	int is_prev_ref = 0, is_ref = 0;

	switch (_ctx._prevFrameType) {
	case IVI4_FRAMETYPE_INTRA:
	case IVI4_FRAMETYPE_INTRA1:
	case IVI4_FRAMETYPE_INTER:
		is_prev_ref = 1;
		break;
	}

	switch (_ctx._frameType) {
	case IVI4_FRAMETYPE_INTRA:
	case IVI4_FRAMETYPE_INTRA1:
	case IVI4_FRAMETYPE_INTER:
		is_ref = 1;
		break;

	default:
		break;
	}

	if (is_prev_ref && is_ref) {
		SWAP(_ctx._dstBuf, _ctx._refBuf);
	} else if (is_prev_ref) {
		SWAP(_ctx._refBuf, _ctx._bRefBuf);
		SWAP(_ctx._dstBuf, _ctx._refBuf);
	}
}

bool Indeo4Decoder::isNonNullFrame() const {
	return _ctx._frameType < IVI4_FRAMETYPE_NULL_FIRST;
}

int Indeo4Decoder::decodeBandHeader(IVIBandDesc *band) {
	int plane, bandNum, indx, transformId, scanIndx;
	int i;
	int quantMat;

	plane = _ctx._gb->getBits(2);
	bandNum = _ctx._gb->getBits(4);
	if (band->_plane != plane || band->_bandNum != bandNum) {
		warning("Invalid band header sequence!");
		return -1;
	}

	band->_isEmpty = _ctx._gb->getBit();
	if (!band->_isEmpty) {
		int old_blk_size = band->_blkSize;
		// skip header size
		// If header size is not given, header size is 4 bytes.
		if (_ctx._gb->getBit())
			_ctx._gb->skip(16);

		band->_isHalfpel = _ctx._gb->getBits(2);
		if (band->_isHalfpel >= 2) {
			warning("Invalid/unsupported mv resolution: %d!",
				band->_isHalfpel);
			return -1;
		}
		if (!band->_isHalfpel)
			_ctx._usesFullpel = true;

		band->_checksumPresent = _ctx._gb->getBit();
		if (band->_checksumPresent)
			band->_checksum = _ctx._gb->getBits(16);

		indx = _ctx._gb->getBits(2);
		if (indx == 3) {
			warning("Invalid block size!");
			return -1;
		}
		band->_mbSize = 16 >> indx;
		band->_blkSize = 8 >> (indx >> 1);

		band->_inheritMv = _ctx._gb->getBit();
		band->_inheritQDelta = _ctx._gb->getBit();

		band->_globQuant = _ctx._gb->getBits(5);

		if (!_ctx._gb->getBit() || _ctx._frameType == IVI4_FRAMETYPE_INTRA) {
			transformId = _ctx._gb->getBits(5);
			if ((uint)transformId >= FF_ARRAY_ELEMS(_transforms) ||
				!_transforms[transformId]._invTrans) {
				warning("Transform %d", transformId);
				return -3;
			}
			if ((transformId >= 7 && transformId <= 9) ||
				transformId == 17) {
				warning("DCT transform");
				return -3;
			}

			if (transformId < 10 && band->_blkSize < 8) {
				warning("wrong transform size!");
				return -1;
			}
			if ((transformId >= 0 && transformId <= 2) || transformId == 10)
				_ctx._usesHaar = true;

			band->_invTransform = _transforms[transformId]._invTrans;
			band->_dcTransform = _transforms[transformId]._dcTrans;
			band->_is2dTrans = _transforms[transformId]._is2dTrans;

			if (transformId < 10)
				band->_transformSize = 8;
			else
				band->_transformSize = 4;

			if (band->_blkSize != band->_transformSize) {
				warning("transform and block size mismatch (%d != %d)", band->_transformSize, band->_blkSize);
				return -1;
			}

			scanIndx = _ctx._gb->getBits(4);
			if (scanIndx == 15) {
				warning("Custom scan pattern encountered!");
				return -1;
			}
			if (scanIndx > 4 && scanIndx < 10) {
				if (band->_blkSize != 4) {
					warning("mismatching scan table!");
					return -1;
				}
			} else if (band->_blkSize != 8) {
				warning("mismatching scan table!");
				return -1;
			}

			band->_scan = _scan_index_to_tab[scanIndx];
			band->_scanSize = band->_blkSize;

			quantMat = _ctx._gb->getBits(5);
			if (quantMat == 31) {
				warning("Custom quant matrix encountered!");
				return -1;
			}
			if ((uint)quantMat >= FF_ARRAY_ELEMS(_quant_index_to_tab)) {
				warning("Quantization matrix %d", quantMat);
				return -1;
			}
			band->_quantMat = quantMat;
		} else {
			if (old_blk_size != band->_blkSize) {
				warning("The band block size does not match the configuration inherited");
				return -1;
			}
		}
		if (_quant_index_to_tab[band->_quantMat] > 4 && band->_blkSize == 4) {
			warning("Invalid quant matrix for 4x4 block encountered!");
			band->_quantMat = 0;
			return -1;
		}
		if (band->_scanSize != band->_blkSize) {
			warning("mismatching scan table!");
			return -1;
		}
		if (band->_transformSize == 8 && band->_blkSize < 8) {
			warning("mismatching _transformSize!");
			return -1;
		}

		// decode block huffman codebook
		if (!_ctx._gb->getBit())
			band->_blkVlc._tab = _ctx._blkVlc._tab;
		else
			if (band->_blkVlc.decodeHuffDesc(&_ctx, 1, IVI_BLK_HUFF))
				return -1;

		// select appropriate rvmap table for this band
		band->_rvmapSel = _ctx._gb->getBit() ? _ctx._gb->getBits(3) : 8;

		// decode rvmap probability corrections if any
		band->_numCorr = 0; // there is no corrections
		if (_ctx._gb->getBit()) {
			band->_numCorr = _ctx._gb->getBits(8); // get number of correction pairs
			if (band->_numCorr > 61) {
				warning("Too many corrections: %d",
					band->_numCorr);
				return -1;
			}

			// read correction pairs
			for (i = 0; i < band->_numCorr * 2; i++)
				band->_corr[i] = _ctx._gb->getBits(8);
		}
	}

	if (band->_blkSize == 8) {
		band->_intraBase = &_ivi4_quant_8x8_intra[_quant_index_to_tab[band->_quantMat]][0];
		band->_interBase = &_ivi4_quant_8x8_inter[_quant_index_to_tab[band->_quantMat]][0];
	} else {
		band->_intraBase = &_ivi4_quant_4x4_intra[_quant_index_to_tab[band->_quantMat]][0];
		band->_interBase = &_ivi4_quant_4x4_inter[_quant_index_to_tab[band->_quantMat]][0];
	}

	// Indeo 4 doesn't use scale tables
	band->_intraScale = NULL;
	band->_interScale = NULL;

	_ctx._gb->align();

	if (!band->_scan) {
		warning("band->_scan not set");
		return -1;
	}

	return 0;
}

int Indeo4Decoder::decodeMbInfo(IVIBandDesc *band, IVITile *tile) {
	int         x, y, mvX, mvY, mvDelta, offs, mbOffset, blksPerMb,
		mvScale, mbTypeBits, s;
	IVIMbInfo	*mb, *ref_mb;
	int			row_offset = band->_mbSize * band->_pitch;

	mb = tile->_mbs;
	ref_mb = tile->_refMbs;
	offs = tile->_yPos * band->_pitch + tile->_xPos;

	blksPerMb = band->_mbSize != band->_blkSize ? 4 : 1;
	mbTypeBits = _ctx._frameType == IVI4_FRAMETYPE_BIDIR ? 2 : 1;

	// scale factor for motion vectors
	mvScale = (_ctx._planes[0]._bands[0]._mbSize >> 3) - (band->_mbSize >> 3);
	mvX = mvY = 0;

	if (((tile->_width + band->_mbSize - 1) / band->_mbSize) * ((tile->_height + band->_mbSize - 1) / band->_mbSize) != tile->_numMBs) {
		warning("numMBs mismatch %d %d %d %d", tile->_width, tile->_height, band->_mbSize, tile->_numMBs);
		return -1;
	}

	for (y = tile->_yPos; y < tile->_yPos + tile->_height; y += band->_mbSize) {
		mbOffset = offs;

		for (x = tile->_xPos; x < tile->_xPos + tile->_width; x += band->_mbSize) {
			mb->_xPos = x;
			mb->_yPos = y;
			mb->_bufOffs = mbOffset;
			mb->_bMvX = mb->_bMvY = 0;

			if (_ctx._gb->getBit()) {
				if (_ctx._frameType == IVI4_FRAMETYPE_INTRA) {
					warning("Empty macroblock in an INTRA picture!");
					return -1;
				}
				mb->_type = 1; // empty macroblocks are always INTER
				mb->_cbp = 0;  // all blocks are empty

				mb->_qDelta = 0;
				if (!band->_plane && !band->_bandNum && _ctx._inQ) {
					mb->_qDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table,
						IVI_VLC_BITS, 1);
					mb->_qDelta = IVI_TOSIGNED(mb->_qDelta);
				}

				mb->_mvX = mb->_mvY = 0; // no motion vector coded
				if (band->_inheritMv && ref_mb) {
					// motion vector inheritance
					if (mvScale) {
						mb->_mvX = scaleMV(ref_mb->_mvX, mvScale);
						mb->_mvY = scaleMV(ref_mb->_mvY, mvScale);
					} else {
						mb->_mvX = ref_mb->_mvX;
						mb->_mvY = ref_mb->_mvY;
					}
				}
			} else {
				if (band->_inheritMv) {
					// copy mb_type from corresponding reference mb
					if (!ref_mb) {
						warning("ref_mb unavailable");
						return -1;
					}
					mb->_type = ref_mb->_type;
				} else if (_ctx._frameType == IVI4_FRAMETYPE_INTRA ||
					_ctx._frameType == IVI4_FRAMETYPE_INTRA1) {
					mb->_type = 0; // mb_type is always INTRA for intra-frames
				} else {
					mb->_type = _ctx._gb->getBits(mbTypeBits);
				}

				mb->_cbp = _ctx._gb->getBits(blksPerMb);

				mb->_qDelta = 0;
				if (band->_inheritQDelta) {
					if (ref_mb) mb->_qDelta = ref_mb->_qDelta;
				} else if (mb->_cbp || (!band->_plane && !band->_bandNum &&
					_ctx._inQ)) {
					mb->_qDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table,
						IVI_VLC_BITS, 1);
					mb->_qDelta = IVI_TOSIGNED(mb->_qDelta);
				}

				if (!mb->_type) {
					mb->_mvX = mb->_mvY = 0; // there is no motion vector in intra-macroblocks
				} else {
					if (band->_inheritMv) {
						if (ref_mb) {
							// motion vector inheritance
							if (mvScale) {
								mb->_mvX = scaleMV(ref_mb->_mvX, mvScale);
								mb->_mvY = scaleMV(ref_mb->_mvY, mvScale);
							} else {
								mb->_mvX = ref_mb->_mvX;
								mb->_mvY = ref_mb->_mvY;
							}
						}
					} else {
						// decode motion vector deltas
						mvDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table,
							IVI_VLC_BITS, 1);
						mvY += IVI_TOSIGNED(mvDelta);
						mvDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table,
							IVI_VLC_BITS, 1);
						mvX += IVI_TOSIGNED(mvDelta);
						mb->_mvX = mvX;
						mb->_mvY = mvY;
						if (mb->_type == 3) {
							mvDelta = _ctx._gb->getVLC2(
								_ctx._mbVlc._tab->_table,
								IVI_VLC_BITS, 1);
							mvY += IVI_TOSIGNED(mvDelta);
							mvDelta = _ctx._gb->getVLC2(
								_ctx._mbVlc._tab->_table,
								IVI_VLC_BITS, 1);
							mvX += IVI_TOSIGNED(mvDelta);
							mb->_bMvX = -mvX;
							mb->_bMvY = -mvY;
						}
					}
					if (mb->_type == 2) {
						mb->_bMvX = -mb->_mvX;
						mb->_bMvY = -mb->_mvY;
						mb->_mvX = 0;
						mb->_mvY = 0;
					}
				}
			}

			s = band->_isHalfpel;
			if (mb->_type)
				if (x + (mb->_mvX >> s) + (y + (mb->_mvY >> s))*band->_pitch < 0 ||
					x + ((mb->_mvX + s) >> s) + band->_mbSize - 1
					+ (y + band->_mbSize - 1 + ((mb->_mvY + s) >> s))*band->_pitch > band->_bufSize - 1) {
					warning("motion vector %d %d outside reference", x*s + mb->_mvX, y*s + mb->_mvY);
					return -1;
				}

			mb++;
			if (ref_mb)
				ref_mb++;
			mbOffset += band->_mbSize;
		}

		offs += row_offset;
	}

	_ctx._gb->align();
	return 0;
}

int Indeo4Decoder::scaleTileSize(int defSize, int sizeFactor) {
	return sizeFactor == 15 ? defSize : (sizeFactor + 1) << 5;
}

int Indeo4Decoder::decodePlaneSubdivision() {
	int i;

	switch (_ctx._gb->getBits(2)) {
	case 3:
		return 1;

	case 2:
		for (i = 0; i < 4; i++)
			if (_ctx._gb->getBits(2) != 3)
				return 0;
		return 4;
	
	default:
		return 0;
	}
}

/*------------------------------------------------------------------------*/

/**
 *  Indeo 4 8x8 scan (zigzag) patterns
 */
static const uint8 ivi4AlternateScan8x8[64] = {
	0,  8,  1,  9, 16, 24,  2,  3, 17, 25, 10, 11, 32, 40, 48, 56,
	4,  5,  6,  7, 33, 41, 49, 57, 18, 19, 26, 27, 12, 13, 14, 15,
	34, 35, 43, 42, 50, 51, 59, 58, 20, 21, 22, 23, 31, 30, 29, 28,
	36, 37, 38, 39, 47, 46, 45, 44, 52, 53, 54, 55, 63, 62, 61, 60
};

static const uint8 ivi4AlternateScan4x4[16] = {
	0, 1, 4, 5, 8, 12, 2, 3, 9, 13, 6, 7, 10, 11, 14, 15
};

static const uint8 ivi4VerticalScan4x4[16] = {
	0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15
};

static const uint8 ivi4HorizontalScan4x4[16] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const uint Indeo4Decoder::_ivi4_common_pic_sizes[14] = {
	640, 480, 320, 240, 160, 120, 704, 480, 352, 240, 352, 288, 176, 144
};

Indeo4Decoder::Transform Indeo4Decoder::_transforms[18] = {
	{ IndeoDSP::ffIviInverseHaar8x8,  IndeoDSP::ffIviDcHaar2d,       1 },
	{ IndeoDSP::ffIviRowHaar8,         IndeoDSP::ffIviDcHaar2d,       0 },
	{ IndeoDSP::ffIviColHaar8,         IndeoDSP::ffIviDcHaar2d,       0 },
	{ IndeoDSP::ffIviPutPixels8x8,    IndeoDSP::ffIviPutDcPixel8x8, 1 },
	{ IndeoDSP::ffIviInverseSlant8x8, IndeoDSP::ffIviDcSlant2d,      1 },
	{ IndeoDSP::ffIviRowSlant8,        IndeoDSP::ffIviDcRowSlant,     1 },
	{ IndeoDSP::ffIviColSlant8,        IndeoDSP::ffIviDcColSlant,     1 },
	{ NULL, NULL, 0 }, // inverse DCT 8x8
	{ NULL, NULL, 0 }, // inverse DCT 8x1
	{ NULL, NULL, 0 }, // inverse DCT 1x8
	{ IndeoDSP::ffIviInverseHaar4x4,  IndeoDSP::ffIviDcHaar2d,       1 },
	{ IndeoDSP::ffIviInverseSlant4x4, IndeoDSP::ffIviDcSlant2d,      1 },
	{ NULL, NULL, 0 }, // no transform 4x4
	{ IndeoDSP::ffIviRowHaar4,         IndeoDSP::ffIviDcHaar2d,       0 },
	{ IndeoDSP::ffIviColHaar4,         IndeoDSP::ffIviDcHaar2d,       0 },
	{ IndeoDSP::ffIviRowSlant4,        IndeoDSP::ffIviDcRowSlant,     0 },
	{ IndeoDSP::ffIviColSlant4,        IndeoDSP::ffIviDcColSlant,     0 },
	{ NULL, NULL, 0 }, // inverse DCT 4x4
};

const uint8 *const Indeo4Decoder::_scan_index_to_tab[15] = {
	// for 8x8 transforms
	ffZigZagDirect,
	ivi4AlternateScan8x8,
	_ffIviHorizontalScan8x8,
	_ffIviVerticalScan8x8,
	ffZigZagDirect,

	// for 4x4 transforms
	_ffIviDirectScan4x4,
	ivi4AlternateScan4x4,
	ivi4VerticalScan4x4,
	ivi4HorizontalScan4x4,
	_ffIviDirectScan4x4,

	// TODO: check if those are needed
	_ffIviHorizontalScan8x8,
	_ffIviHorizontalScan8x8,
	_ffIviHorizontalScan8x8,
	_ffIviHorizontalScan8x8,
	_ffIviHorizontalScan8x8
};

/**
 *  Indeo 4 dequant tables
 */
const uint16 Indeo4Decoder::_ivi4_quant_8x8_intra[9][64] = {
	{
      43,  342,  385,  470,  555,  555,  598,  726,
     342,  342,  470,  513,  555,  598,  726,  769,
     385,  470,  555,  555,  598,  726,  726,  811,
     470,  470,  555,  555,  598,  726,  769,  854,
     470,  555,  555,  598,  683,  726,  854, 1025,
     555,  555,  598,  683,  726,  854, 1025, 1153,
     555,  555,  598,  726,  811,  982, 1195, 1451,
     555,  598,  726,  811,  982, 1195, 1451, 1793
	},
	{
      86, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
    1195, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
    2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
    2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827
	},
	{
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835
	},
	{
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
     897,  897,  897,  897,  897,  897,  897,  897,
    1067, 1067, 1067, 1067, 1067, 1067, 1067, 1067,
    1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
    1409, 1409, 1409, 1409, 1409, 1409, 1409, 1409,
    1579, 1579, 1579, 1579, 1579, 1579, 1579, 1579,
    1750, 1750, 1750, 1750, 1750, 1750, 1750, 1750,
    1921, 1921, 1921, 1921, 1921, 1921, 1921, 1921,
    2091, 2091, 2091, 2091, 2091, 2091, 2091, 2091
	},
	{
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390
	},
	{
      22,  171,  214,  257,  257,  299,  299,  342,
     171,  171,  257,  257,  299,  299,  342,  385,
     214,  257,  257,  299,  299,  342,  342,  385,
     257,  257,  257,  299,  299,  342,  385,  427,
     257,  257,  299,  299,  342,  385,  427,  513,
     257,  299,  299,  342,  385,  427,  513,  598,
     299,  299,  299,  385,  385,  470,  598,  726,
     299,  299,  385,  385,  470,  598,  726,  897
	},
	{
      86,  598, 1195, 1195, 2390, 2390, 2390, 2390,
     598,  598, 1195, 1195, 2390, 2390, 2390, 2390,
    1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
    1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_8x8_inter[9][64] = {
	{
     427,  427,  470,  427,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470,
     427,  427,  470,  470,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470
	},
	{
    1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
    1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
    2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
    2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414
	},
	{
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281
	},
	{
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433
	},
	{
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
    1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281
	},
	{
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433
	},
	{
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707
	},
	{
      86,  171,  171,  214,  214,  214,  214,  257,
     171,  171,  214,  214,  214,  214,  257,  257,
     171,  214,  214,  214,  214,  257,  257,  257,
     214,  214,  214,  214,  257,  257,  257,  299,
     214,  214,  214,  257,  257,  257,  299,  299,
     214,  214,  257,  257,  257,  299,  299,  299,
     214,  257,  257,  257,  299,  299,  299,  342,
     257,  257,  257,  299,  299,  299,  342,  342
	},
	{
     854,  854, 1195, 1195, 1707, 1707, 1707, 1707,
     854,  854, 1195, 1195, 1707, 1707, 1707, 1707,
    1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
    1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_4x4_intra[5][16] = {
	{
      22,  214,  257,  299,
     214,  257,  299,  342,
     257,  299,  342,  427,
     299,  342,  427,  513
	},
	{
     129, 1025, 1451, 1451,
    1025, 1025, 1451, 1451,
    1451, 1451, 2049, 2049,
    1451, 1451, 2049, 2049
	},
	{
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171
	},
	{
      43,   43,   43,   43,
     171,  171,  171,  171,
     171,  171,  171,  171,
     171,  171,  171,  171
	},
	{
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_4x4_inter[5][16] = {
	{
     107,  214,  257,  299,
     214,  257,  299,  299,
     257,  299,  299,  342,
     299,  299,  342,  342
	},
	{
     513, 1025, 1238, 1238,
    1025, 1025, 1238, 1238,
    1238, 1238, 1451, 1451,
    1238, 1238, 1451, 1451
	},
	{
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171
	},
	{
      43,   43,   43,   43,
     171,  171,  171,  171,
     171,  171,  171,  171,
     171,  171,  171,  171
	},
	{
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43
	}
};

const uint8 Indeo4Decoder::_quant_index_to_tab[22] = {
	0, 1, 0, 2, 1, 3, 0, 4, 1, 5, 0, 1, 6, 7, 8, // for 8x8 quant matrixes
	0, 1, 2, 2, 3, 3, 4                          // for 4x4 quant matrixes
};

} // End of namespace Image
