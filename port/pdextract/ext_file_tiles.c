#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

struct tile {
	uint8_t type;
	uint8_t numvertices;
	uint16_t flags;
	uint16_t floortype;
	uint8_t xmin;
	uint8_t ymin;
	uint8_t zmin;
	uint8_t xmax;
	uint8_t ymax;
	uint8_t zmax;
	uint16_t floorcol;
};

void extract_file_tiles(char *filename, uint32_t romoffset, size_t len)
{
	// Unzip it
	size_t infsize = rzip_get_infsize(&g_Rom[romoffset]);
	uint8_t *src = malloc(infsize);

	int ret = rzip_inflate(src, infsize, &g_Rom[romoffset], len);

	if (ret < 0) {
		fprintf(stderr, "%s: Unable to inflate file: %d\n", filename, ret);
		exit(EXIT_FAILURE);
	}

	int num_rooms = srctoh32(*(uint32_t *) src);

	size_t src_ptr_table_len = (num_rooms + 1) * 4;
	size_t dst_ptr_table_len = (num_rooms + 1) * sizeof(uint32_t);

	uint32_t *src_offsets = (uint32_t *) &src[4];

	size_t data_len = infsize - src_ptr_table_len - 4;

	uint8_t *dst = calloc(1, dst_ptr_table_len + data_len + 0x20);

	*(uint32_t *) dst = htodst32(num_rooms);

	uint32_t*dst_offsets = (uint32_t*) (dst + sizeof(uint32_t));
	uint32_t cur_dst_offset = dst_ptr_table_len + sizeof(uint32_t);

	uint32_t cur_src_offset = srctoh32(src_offsets[0]);
	uint32_t end_src_offset = srctoh32(src_offsets[num_rooms]);
	int room = 0;

	while (1) {
		// Check if this is a new room and write the pointer offset if so
		while (room < (num_rooms + 1) && srctoh32(src_offsets[room]) == cur_src_offset) {
			dst_offsets[room] = htodst32(cur_dst_offset);
			room++;
		}

		if (cur_src_offset >= end_src_offset) {
			break;
		}

		// Write tile data
		struct tile *n64tile = (struct tile *) &src[cur_src_offset];
		struct tile hosttile;

		hosttile.type = n64tile->type;
		hosttile.numvertices = n64tile->numvertices;
		hosttile.flags = srctodst16(n64tile->flags);
		hosttile.floortype = srctodst16(n64tile->floortype);
		hosttile.xmin = n64tile->xmin;
		hosttile.ymin = n64tile->ymin;
		hosttile.zmin = n64tile->zmin;
		hosttile.xmax = n64tile->xmax;
		hosttile.ymax = n64tile->ymax;
		hosttile.zmax = n64tile->zmax;
		hosttile.floorcol = srctodst16(n64tile->floorcol);

		memcpy(&dst[cur_dst_offset], &hosttile, sizeof(struct tile));

		cur_dst_offset += sizeof(struct tile);
		cur_src_offset += sizeof(struct tile);

		// Write tile vertices
		for (int i = 0; i < hosttile.numvertices; i++) {
			*(int16_t *) &dst[cur_dst_offset + 0] = srctodst16(*(int16_t *) &src[cur_src_offset + 0]);
			*(int16_t *) &dst[cur_dst_offset + 2] = srctodst16(*(int16_t *) &src[cur_src_offset + 2]);
			*(int16_t *) &dst[cur_dst_offset + 4] = srctodst16(*(int16_t *) &src[cur_src_offset + 4]);
			cur_dst_offset += 6;
			cur_src_offset += 6;
		}
	}

	cur_dst_offset = ALIGN16(cur_dst_offset);

	// Zip it
	uint8_t *zipped = calloc(1, cur_dst_offset);
	size_t ziplen = cur_dst_offset;

	ret = rzip_deflate(zipped, &ziplen, dst, cur_dst_offset);

	if (ret < 0) {
		fprintf(stderr, "%s: Unable to compress file: %d\n", filename, ret);
		exit(EXIT_FAILURE);
	}

	ziplen = ALIGN16(ziplen);

	// Write it
	char outfilename[1024];

	sprintf(outfilename, "%s/files/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(zipped, ziplen, 1, fp);
	fclose(fp);

	free(zipped);
	free(dst);
	free(src);
}
