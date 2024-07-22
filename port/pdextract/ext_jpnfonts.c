#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static void extract_jpnfont(const char *filename, uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/segs/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}

void extract_jpnfonts(void)
{
	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		extract_jpnfont("fontjpnsingle", 0x194440, 0xb020);
		extract_jpnfont("fontjpnmulti", 0x19fb40, 0x1a80);
		break;
	case ROMVERSION_PALFINAL:
		extract_jpnfont("fontjpnsingle", 0x17fc40, 0xb010);
		extract_jpnfont("fontjpnmulti", 0x18b340, 0x1a80);
		break;
	case ROMVERSION_JPNFINAL:
		extract_jpnfont("fontjpn", 0x178c40, 0x17920);
		break;
	}
}
