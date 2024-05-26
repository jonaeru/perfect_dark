#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void extract_file_audio(char *filename, uint32_t romoffset, size_t len);
void extract_file_bg(char *filename, uint32_t romoffset, size_t len);
void extract_file_lang(char *filename, uint32_t romoffset, size_t len);
void extract_file_model(char *filename, uint32_t romoffset, size_t len);
void extract_file_pads(char *filename, uint32_t romoffset, size_t len);
void extract_file_tiles(char *filename, uint32_t romoffset, size_t len);

static void extract_file(int filenum, uint32_t romoffset, size_t len)
{
	uint32_t name_uint32_table = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		name_uint32_table = 0x1d5ca00;
		break;
	case ROMVERSION_PALFINAL:
		name_uint32_table = 0x1d534e0;
		break;
	case ROMVERSION_JPNFINAL:
		name_uint32_table = 0x1d58a20;
		break;
	}

	uint32_t name_offset = name_uint32_table + srctoh32(*(uint32_t *) &g_Rom[name_uint32_table + filenum * 4]);

	char *filename = (char *) &g_Rom[name_offset];

	if (strstr(filename, ".seg")) {
		extract_file_bg(filename, romoffset, len);
	} else if (strstr(filename, "padsZ")) {
		extract_file_pads(filename, romoffset, len);
	} else if (strstr(filename, "tilesZ")) {
		extract_file_tiles(filename, romoffset, len);
	} else if (filename[0] == 'A') {
		extract_file_audio(filename, romoffset, len);
	} else if (filename[0] == 'C' || filename[0] == 'G' || filename[0] == 'P') {
		extract_file_model(filename, romoffset, len);
	} else if (filename[0] == 'L') {
		extract_file_lang(filename, romoffset, len);
	} else if (filename[0] == 'U') {
		// Setup file - don't extract
	} else {
		fprintf(stderr, "Unrecognised file type: '%s'\n", filename);
		exit(EXIT_FAILURE);
	}
}

void extract_files(void)
{
	size_t infsize = rzip_get_infsize(&g_Rom[0x39850]);
	uint8_t *data = malloc(infsize);

	int ret = rzip_inflate(data, infsize, &g_Rom[0x39850], infsize);

	if (ret < 0) {
		fprintf(stderr, "Unable to inflate data segment: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	uint32_t offset = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		offset = 0x28080;
		break;
	case ROMVERSION_PALFINAL:
		offset = 0x28910;
		break;
	case ROMVERSION_JPNFINAL:
		offset = 0x28800;
		break;
	}

	uint32_t *offsets = (uint32_t *) &data[offset];
	int i = 0;

	while (1) {
		uint32_t curr = srctoh32(offsets[i]);
		uint32_t next = srctoh32(offsets[i + 1]);

		if (next == 0) {
			break;
		}

		if (curr) {
			extract_file(i, curr, next - curr);
		}

		i++;
	}

	free(data);
}
