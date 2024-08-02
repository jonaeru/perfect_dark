#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"

struct n64fontchar {
	u8 index;
	s8 baseline;
	u8 height;
	u8 width;
	s32 kerningindex;
	u32 pixeldata;
};

struct hostfontchar {
	u8 index;
	s8 baseline;
	u8 height;
	u8 width;
	s32 kerningindex;
	uintptr_t pixeldata;
};

struct n64fontcharjpn {
	u16 index;
	s8 baseline;
	u8 height;
	u8 width;
	s16 kerningindex;
	u32 pixeldata;
};

struct hostfontcharjpn {
	u16 index;
	s8 baseline;
	u8 height;
	u8 width;
	s16 kerningindex;
	uintptr_t pixeldata;
};

struct fontdef {
	const char *name;
	u32 offsets[3];
	int pal_extra_chars;
};

u8* preprocessFont_x64(u8 *src, u32 srclen, u32 *outSize)
{
	int num_chars = 94;
#if VERSION == VERSION_PAL_FINAL
	// PAL has more characters in these fonts
	extern u8* _fonthandelgothicsmSegmentRomStart;
	extern u8* _fonthandelgothicxsSegmentRomStart;
	extern u8* _fonthandelgothicmdSegmentRomStart;
	if (src == _fonthandelgothicsmSegmentRomStart
		|| src == _fonthandelgothicxsSegmentRomStart
		|| src == _fonthandelgothicmdSegmentRomStart) {
		numchars = 135;
	}
#endif

	size_t dstlen = srclen - (num_chars * sizeof(struct n64fontchar)) + (num_chars * sizeof(struct hostfontchar)) + 0x20;
	u8* dst = sysMemZeroAlloc(dstlen);

	// Kerning table
	s32 *src_kerning_table = (s32 *)src;
	s32 *dst_kerning_table = (s32 *)dst;

	for (int i = 0; i < 13 * 13; i++) {
		dst_kerning_table[i] = srctodst32(src_kerning_table[i]);
	}

	u32 src_offset = ALIGN(13 * 13 * 4, sizeof(u32));
	u32 dst_offset = ALIGN(13 * 13 * 4, sizeof(uintptr_t));

	// Character table
#if VERSION == VERSION_JPN_FINAL
		struct n64fontcharjpn *src_char_table = (struct n64fontcharjpn *) &src[src_offset];
		struct hostfontcharjpn *dst_char_table = (struct hostfontcharjpn *) &dst[dst_offset];
		u32 diff = (dst_offset + num_chars * sizeof(struct hostfontcharjpn)) - (src_offset + num_chars * sizeof(struct n64fontcharjpn));

		for (int i = 0; i < num_chars; i++) {
			dst_char_table[i].index = srctodst16(src_char_table[i].index);
			dst_char_table[i].baseline = src_char_table[i].baseline;
			dst_char_table[i].height = src_char_table[i].height;
			dst_char_table[i].width = src_char_table[i].width;
			dst_char_table[i].kerningindex = srctodst16(src_char_table[i].kerningindex);
			dst_char_table[i].pixeldata = src_char_table[i].pixeldata ? srctodst32(src_char_table[i].pixeldata) + diff : 0;
		}

		src_offset += num_chars * sizeof(struct n64fontcharjpn);
		dst_offset += num_chars * sizeof(struct hostfontcharjpn);
#else
		struct n64fontchar *src_char_table = (struct n64fontchar *) &src[src_offset];
		struct hostfontchar *dst_char_table = (struct hostfontchar *) &dst[dst_offset];
		u32 diff = (dst_offset + num_chars * sizeof(struct hostfontchar)) - (src_offset + num_chars * sizeof(struct n64fontchar));

		for (int i = 0; i < num_chars; i++) {
			dst_char_table[i].index = src_char_table[i].index;
			dst_char_table[i].baseline = src_char_table[i].baseline;
			dst_char_table[i].height = src_char_table[i].height;
			dst_char_table[i].width = src_char_table[i].width;
			dst_char_table[i].kerningindex = srctodst32(src_char_table[i].kerningindex);
			dst_char_table[i].pixeldata = src_char_table[i].pixeldata ? srctodst32(src_char_table[i].pixeldata) + diff : 0;
		}

		src_offset += num_chars * sizeof(struct n64fontchar);
		dst_offset += num_chars * sizeof(struct hostfontchar);
#endif

	// Pixel data
	u32 len = srclen - src_offset;
	memcpy(&dst[dst_offset], &src[src_offset], len);

	if (outSize) *outSize = ALIGN16(dst_offset + len);

	return dst;
}
