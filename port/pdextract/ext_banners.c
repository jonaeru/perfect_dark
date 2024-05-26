#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void extract_banner(const char *filename, uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/%s.bin", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}

void extract_banners(void)
{
	uint32_t accessingpak = 0;
	uint32_t copyright = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		copyright = 0x1ffea20;
		accessingpak = 0x1fff550;
		break;
	case ROMVERSION_PALFINAL:
		copyright = 0x1ff5500;
		accessingpak = 0x1ff6030;
		break;
	case ROMVERSION_JPNFINAL:
		copyright = 0x1ffd6b0;
		accessingpak = 0x1ffe1e0;
		break;
	}

	extract_banner("copyright", copyright, 0xb30);
	extract_banner("accessingpak", accessingpak, 0x8b0);
}
