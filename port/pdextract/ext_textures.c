#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

struct texture {
	uint32_t soundsurfacetype : 4;
	uint32_t surfacetype : 4;
	uint32_t dataoffset : 24;
	uint32_t unk04;
};

static void extract_textures_data(uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/textures_data.bin", g_OutPath);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}

static void extract_textures_table(uint32_t romoffset, size_t len)
{
	int num_textures = len / sizeof(struct texture);
	uint8_t *dst = calloc(1, len);

	struct texture *n64texes = (struct texture *) &g_Rom[romoffset];
	struct texture *hosttexes = (struct texture *) dst;

	for (int i = 0; i < num_textures; i++) {
		hosttexes[i].soundsurfacetype = n64texes[i].soundsurfacetype;
		hosttexes[i].surfacetype = n64texes[i].surfacetype;
		hosttexes[i].dataoffset = srctodst32(n64texes[i].dataoffset);
		hosttexes[i].unk04 = 0;
	}

	// Write it
	char outfilename[1024];
	sprintf(outfilename, "%s/textures_table.bin", g_OutPath);

	FILE *fp = openfile(outfilename);
	fwrite(dst, len, 1, fp);
	fclose(fp);

	free(dst);
}

void extract_textures(void)
{
	uint32_t data_offset = 0;
	uint32_t table_offset = 0;
	uint32_t end = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		data_offset = 0x1d65f40;
		table_offset = 0x1ff7ca0;
		end = 0x1ffea20;
		break;
	case ROMVERSION_PALFINAL:
		data_offset = 0x1d5ca20;
		table_offset = 0x1fee780;
		end = 0x1ff5500;
		break;
	case ROMVERSION_JPNFINAL:
		data_offset = 0x1d61f90;
		table_offset = 0x1ff68f0;
		end = 0x1ffd6b0;
		break;
	}

	extract_textures_data(data_offset, table_offset - data_offset);
	extract_textures_table(table_offset, end - table_offset);
}
