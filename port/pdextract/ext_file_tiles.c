#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"


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

uint32_t convert_tiles(uint8_t *dst, uint8_t *src, size_t srclen)
{
	int num_rooms = srctoh32(*(uint32_t *) src);

	size_t src_ptr_table_len = (num_rooms + 1) * 4;
	size_t dst_ptr_table_len = (num_rooms + 1) * sizeof(uint32_t);

	uint32_t *src_offsets = (uint32_t *) &src[4];

	size_t data_len = srclen - src_ptr_table_len - 4;

	*(uint32_t *) dst = htodst32(num_rooms);

	uint32_t *dst_offsets = (uint32_t *) (dst + sizeof(uint32_t));
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

	return ALIGN16(cur_dst_offset);
}

uint8_t *preprocessTilesFile_x64(uint8_t *data, uint32_t size, uint32_t *outSize)
{
	uint8_t *dst = sysMemZeroAlloc(size);

	uint32_t newSize = convert_tiles(dst, data, size);

	if (newSize > size) {
		sysLogPrintf(LOG_ERROR, "overflow when trying to preprocess a tiles file, size %d newsize %d", size, newSize);
		exit(EXIT_FAILURE);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
