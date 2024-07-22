#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

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

static void extract_font(const char *filename, uint32_t romoffset, size_t srclen, int num_chars)
{
	size_t dstlen = srclen - (num_chars * sizeof(struct n64fontchar)) + (num_chars * sizeof(struct hostfontchar)) + 0x20;
	uint8_t *src = &g_Rom[romoffset];
	uint8_t *dst = calloc(1, dstlen);

	// Kerning table
	int32_t *src_kerning_table = (int32_t *) src;
	int32_t *dst_kerning_table = (int32_t *) dst;

	for (int i = 0; i < 13 * 13; i++) {
		dst_kerning_table[i] = srctodst32(src_kerning_table[i]);
	}

	uint32_t src_offset = ALIGN(13 * 13 * 4, sizeof(uint32_t));
	uint32_t dst_offset = ALIGN(13 * 13 * 4, sizeof(uintptr_t));

	// Character table
	if (g_RomVersion == ROMVERSION_JPNFINAL) {
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
	} else {
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
	}

	// Pixel data
	size_t len = srclen - src_offset;

	memcpy(&dst[dst_offset], &src[src_offset], len);

	dst_offset = ALIGN16(dst_offset + len);

	// Write it
	char outfilename[1024];
	sprintf(outfilename, "%s/segs/font%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(dst, dst_offset, 1, fp);
	fclose(fp);

	free(dst);
}

struct fontdef {
	const char *name;
	uint32_t offsets[3];
	int pal_extra_chars;
};

void extract_fonts(void)
{
	struct fontdef defs[] = {
        //                      ntsc       pal       jpn
		{"bankgothic",     {0x7f2390, 0x7ddb90, 0x7e1a20}, 0},
		{"zurich",         {0x7f4930, 0x7e0130, 0x7e3fc0}, 0},
        {"tahoma",         {0x7f7860, 0x7e3060, 0x7e6ef0}, 0},
        {"numeric",        {0x7f8b20, 0x7e4320, 0x7e81b0}, 0},
        {"handelgothicsm", {0x7f9d30, 0x7e5530, 0x7e93c0}, 1},
        {"handelgothicxs", {0x7fbfb0, 0x7e87b0, 0x7ec640}, 1},
        {"handelgothicmd", {0x7fdd80, 0x7eae20, 0x7eecb0}, 1},
        {"handelgothiclg", {0x8008e0, 0x7eee70, 0x7f2d00}, 0},
        {"ocramd",         {0x803da0, 0x7f2330, 0x7f61c0}, 0},
        {"ocralg",         {0x806ac0, 0x7f5050, 0x7f8ee0}, 0},
        {"end",            {0x80a250, 0x7f87e0, 0x7fc670}, 0},
    };

	int index = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		index = 0;
		break;
	case ROMVERSION_PALFINAL:
		index = 1;
		break;
	case ROMVERSION_JPNFINAL:
		index = 2;
		break;
	}

	for (int i = 0; i < ARRAYCOUNT(defs) - 1; i++) {
		int num_chars = g_RomVersion == ROMVERSION_PALFINAL && defs[i].pal_extra_chars ? 135 : 94;
		size_t len = defs[i + 1].offsets[index] - defs[i].offsets[index];
		extract_font(defs[i].name, defs[i].offsets[index], len, num_chars);
	}
}
