#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

struct animtableentry {
	uint16_t numframes;
	uint16_t bytesperframe;
	uint32_t data;
	uint16_t headerlen;
	uint8_t framelen;
	uint8_t flags;
};

static void extract_anims_data(uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/anims_data.bin", g_OutPath);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}

static void extract_anims_table(uint32_t data_offset, uint32_t table_offset)
{
	int num_anims = srctoh32(*(uint32_t *) &g_Rom[table_offset]);
	struct animtableentry *srcanims = (struct animtableentry *) &g_Rom[table_offset + 4];

	size_t dstlen = ALIGN16(4 + num_anims * sizeof(struct animtableentry));
	uint8_t *dst = calloc(1, dstlen);
	*(uint32_t *) dst = htodst32(num_anims);
	struct animtableentry *dstanims = (struct animtableentry *) (dst + 4);

	for (int i = 0; i < num_anims; i++) {
		dstanims[i].numframes = srctodst16(srcanims[i].numframes);
		dstanims[i].bytesperframe = srctodst16(srcanims[i].bytesperframe);
		dstanims[i].data = srctodst32(srcanims[i].data);
		dstanims[i].headerlen = srctodst16(srcanims[i].headerlen);
		dstanims[i].framelen = srcanims[i].framelen;
		dstanims[i].flags = srcanims[i].flags;
	}

	char outfilename[1024];
	sprintf(outfilename, "%s/segs/animations", g_OutPath);

	FILE *fp = openfile(outfilename);
	size_t datalen = table_offset - data_offset;
	fwrite(&g_Rom[data_offset], datalen, 1, fp);
	fwrite(dst, dstlen, 1, fp);
	fclose(fp);

	free(dst);
}

void extract_anims(void)
{
	uint32_t table_offset = 0;
	uint32_t data_offset = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		data_offset = 0x1a15c0;
		table_offset = 0x7cd1a0;
		break;
	case ROMVERSION_PALFINAL:
		data_offset = 0x18cdc0;
		table_offset = 0x7b89a0;
		break;
	case ROMVERSION_JPNFINAL:
		data_offset = 0x190c50;
		table_offset = 0x7bc830;
		break;
	}

	extract_anims_table(data_offset, table_offset);
}
