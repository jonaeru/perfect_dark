#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"

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

uint32_t convert_lang_file(uint8_t *dst, uint8_t *src, size_t srclen)
{
	// Convert it
	uint32_t *src_offsets = (uint32_t *) src;
	size_t src_ptr_table_len = srctoh32(src_offsets[0]);
	size_t src_str_table_len = srclen - src_ptr_table_len;
	int num_strings = get_num_strings(src, srclen);
	size_t dst_ptr_table_len = num_strings * sizeof(uintptr_t);
	size_t dst_str_table_len = src_str_table_len;

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
				end = srclen;
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

	return ALIGN16(cur_dst_offset);
}

uint8_t *preprocessLangFile_x64(uint8_t *data, uint32_t size, uint32_t *outSize) {
	uint32_t newSizeEstimated = (uint32_t)(size * 1.3);
	uint8_t *dst = sysMemZeroAlloc(newSizeEstimated);

	uint32_t newSize = convert_lang_file(dst, data, size);

	if (newSize > newSizeEstimated) {
		sysLogPrintf(LOG_ERROR, "overflow when trying to preprocess a lang file, size %d newsize %d", size, newSize);
		exit(EXIT_FAILURE);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
