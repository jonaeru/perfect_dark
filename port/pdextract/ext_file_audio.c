#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void extract_file_audio(char *filename, uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/files/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}
