#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static int get_next_nonzero(uint8_t *src, uint32_t offset, size_t len)
{
	uint32_t *ptr = (uint32_t *) &src[offset];
	uint32_t *end = (uint32_t *) &src[len];

	while (ptr < end) {
		if (*ptr != 0) {
			return srctoh32(*ptr);
		}

		ptr++;
	}

	return 0;
}

static int get_num_strings(uint8_t *src, size_t len)
{
	uint32_t ptr_table_len = get_next_nonzero(src, 0, len);

	if (ptr_table_len) {
		return ptr_table_len / sizeof(uint32_t);
	}

	return 1;
}

void extract_file_lang(char *filename, uint32_t romoffset, size_t srclen)
{
	// Unzip it
	size_t infsize = rzip_get_infsize(&g_Rom[romoffset]);
	uint8_t *src = malloc(infsize);

	int ret = rzip_inflate(src, infsize, &g_Rom[romoffset], srclen);

	if (ret < 0) {
		fprintf(stderr, "%s: Unable to inflate file: %d\n", filename, ret);
		exit(EXIT_FAILURE);
	}

	// Convert it
	uint32_t *src_offsets = (uint32_t *) src;
	size_t src_ptr_table_len = srctoh32(src_offsets[0]);
	size_t src_str_table_len = infsize - src_ptr_table_len;
	int num_strings = get_num_strings(src, srclen);
	size_t dst_ptr_table_len = num_strings * sizeof(uintptr_t);
	size_t dst_str_table_len = src_str_table_len;

	uint8_t *dst = calloc(1, dst_ptr_table_len + dst_str_table_len + 0x20);
	int cur_dst_offset = dst_ptr_table_len;

	uintptr_t *dst_offsets = (uintptr_t *) dst;

	for (int i = 0; i < num_strings; i++) {
		uint32_t src_offset = srctoh32(src_offsets[i]);

		if (src_offset) {
			// Write pointer
			dst_offsets[i] = htodst32(cur_dst_offset);

			// Write string
			uint32_t end = get_next_nonzero(src, (i + 1) * 4, num_strings * 4);

			if (!end) {
				end = infsize;
			}

			size_t len = end - src_offset;
			len = ALIGN4(len);
			memcpy(&dst[cur_dst_offset], &src[src_offset], len);

			cur_dst_offset += len;
		} else {
			// Write pointer
			dst_offsets[i] = 0;
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
