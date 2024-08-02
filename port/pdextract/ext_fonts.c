#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"

struct n64fontchar {
	uint8_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int32_t kerningindex;
	uint32_t pixeldata;
};

struct hostfontchar {
	uint8_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int32_t kerningindex;
	uintptr_t pixeldata;
};

struct n64fontcharjpn {
	uint16_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int16_t kerningindex;
	uint32_t pixeldata;
};

struct hostfontcharjpn {
	uint16_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int16_t kerningindex;
	uintptr_t pixeldata;
};

struct fontdef {
	const char *name;
	uint32_t offsets[3];
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
	uint8_t* dst = sysMemZeroAlloc(dstlen);

	// Kerning table
	int32_t *src_kerning_table = (int32_t *)src;
	int32_t *dst_kerning_table = (int32_t *)dst;

	for (int i = 0; i < 13 * 13; i++) {
		dst_kerning_table[i] = srctodst32(src_kerning_table[i]);
	}

	uint32_t src_offset = ALIGN(13 * 13 * 4, sizeof(uint32_t));
	uint32_t dst_offset = ALIGN(13 * 13 * 4, sizeof(uintptr_t));

	// Character table
#if VERSION == VERSION_JPN_FINAL
		struct n64fontcharjpn *src_char_table = (struct n64fontcharjpn *) &src[src_offset];
		struct hostfontcharjpn *dst_char_table = (struct hostfontcharjpn *) &dst[dst_offset];
		uint32_t diff = (dst_offset + num_chars * sizeof(struct hostfontcharjpn)) - (src_offset + num_chars * sizeof(struct n64fontcharjpn));

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
		uint32_t diff = (dst_offset + num_chars * sizeof(struct hostfontchar)) - (src_offset + num_chars * sizeof(struct n64fontchar));

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
