#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#ifdef FORCE_BE32
#define HOST_DWORDS_PER_CMD 1
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define HOST_DWORDS_PER_CMD 2
#else
#define HOST_DWORDS_PER_CMD 1
#endif

struct vtx {
	int16_t x;
	int16_t y;
	int16_t z;
	uint8_t flags;
	uint8_t colour;
	int16_t s;
	int16_t t;
};

uint32_t m_Segments[16];
uint8_t m_Seen[1024];
uint32_t m_SrcVtxOffset;
uint32_t m_DstVtxOffset;

void gbi_reset(void)
{
	for (int i = 0; i < ARRAYCOUNT(m_Segments); i++) {
		m_Segments[i] = 0;
	}

	for (int i = 0; i < ARRAYCOUNT(m_Seen); i++) {
		m_Seen[i] = 0;
	}

	m_SrcVtxOffset = 0;
	m_DstVtxOffset = 0;
}

void gbi_set_segment(int segment, uint32_t offset)
{
	m_Segments[segment] = offset;
}

void gbi_set_vtx(uint32_t src_offset, uint32_t dst_offset)
{
	m_SrcVtxOffset = src_offset;
	m_DstVtxOffset = dst_offset;
}

static void gbi_convert_col(uint8_t *dst, uint32_t dstpos, int count)
{
	for (int i = 0; i < count; i++) {
		int index = dstpos / 4;

		if (!m_Seen[index]) {
			uint32_t *col = (uint32_t *) &dst[dstpos];

			*col = srctodst32(*col);

			m_Seen[index] = 1;
		}

		dstpos += sizeof(uint32_t);
	}
}

static void gbi_convert_vtx(uint8_t *dst, uint32_t dstpos, int count)
{
	for (int i = 0; i < count; i++) {
		int index = dstpos / 4;

		if (!m_Seen[index]) {
			struct vtx *vtx = (struct vtx *) &dst[dstpos];

			vtx->x = srctodst16(vtx->x);
			vtx->y = srctodst16(vtx->y);
			vtx->z = srctodst16(vtx->z);
			vtx->s = srctodst16(vtx->s);
			vtx->t = srctodst16(vtx->t);

			m_Seen[index] = 1;
		}

		dstpos += 12;
	}
}

#define CMD_IS_VTX(cmd)      ((cmd & 0xff00000000000000) == 0x0400000000000000)
#define CMD_IS_DL(cmd)       ((cmd & 0xff00000000000000) == 0x0600000000000000)
#define CMD_IS_COL(cmd)      ((cmd & 0xff00000000000000) == 0x0700000000000000)
#define CMD_IS_ENDDL(cmd)    ((cmd & 0xff00000000000000) == 0xb800000000000000)
#define CMD_IS_MOVEWORD(cmd) ((cmd & 0xff00000000000000) == 0xbc00000000000000)

#define CMD_HAS_ADDR(cmd) CMD_IS_VTX(cmd) \
	|| CMD_IS_COL(cmd) \
	|| CMD_IS_MOVEWORD(cmd)

static void gbi_convert_vtx_by_cmd(uint64_t cmd, uint8_t *dst)
{
	uint8_t segment = cmd & 0xff000000;
	uint8_t offset = cmd & 0x00ffffff;
	uint32_t num_bytes = (cmd >> 32) & 0xffff;
	int num_vertices = num_bytes / 12;

	if (segment == 5) {
		uint32_t dst_offset = offset - m_SrcVtxOffset + m_DstVtxOffset;
		gbi_convert_vtx(dst, dst_offset, num_vertices);
	} else {
		gbi_convert_vtx(dst, m_Segments[segment] + offset, num_vertices);
	}
}

static void gbi_convert_col_by_cmd(uint64_t cmd, uint8_t *dst)
{
	uint8_t segment = cmd & 0xff000000;
	uint8_t offset = cmd & 0x00ffffff;
	uint32_t num_bytes = (cmd >> 32) & 0xffff;
	int num_colours = num_bytes / 4;

	if (segment == 5) {
		uint32_t dst_offset = offset - m_SrcVtxOffset + m_DstVtxOffset;
		gbi_convert_col(dst, dst_offset, num_colours);
	} else {
		gbi_convert_col(dst, m_Segments[segment] + offset, num_colours);
	}
}

/**
 * Segment 5 is the start of the model file. The data in these files shifts
 * depending on the pointer size, so this function adjusts the offset accordingly.
 */
static uint64_t gbi_rewrite_addr(uint64_t cmd)
{
	uint8_t segment = cmd & 0xff000000;
	uint8_t offset = cmd & 0x00ffffff;

	if (segment == 5) {
		if (offset < m_SrcVtxOffset) {
			printf("Tried to load data from offset 0x%x but segment starts at 0x%x\n", offset, m_SrcVtxOffset);
			exit(EXIT_FAILURE);
		}

		offset = offset - m_SrcVtxOffset + m_DstVtxOffset;
	}

	return (cmd & 0xffffffffff000000) | offset;
}

uint32_t gbi_convert_gdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	dstpos = ALIGN8(dstpos);

	uint64_t *n64_cmd = (uint64_t *) &src[srcpos];
	uint64_t *host_cmd = (uint64_t *) &dst[dstpos];
	uint64_t cmd;

	do {
		cmd = srctoh64(*n64_cmd);

#if HOST_DWORDS_PER_CMD == 2
		if (CMD_HAS_ADDR(cmd)) {
			cmd = gbi_rewrite_addr(cmd);
			host_cmd[0] = htodst64(cmd & 0xffffffff00000000);
			host_cmd[1] = htodst64(cmd & 0x00000000ffffffff);
		} else {
			host_cmd[0] = htodst64(cmd);
			host_cmd[1] = 0;
		}
#else
		host_cmd[0] = htodst64(cmd);
#endif
		dstpos += sizeof(*host_cmd) * HOST_DWORDS_PER_CMD;

		if (CMD_IS_VTX(cmd)) {
			gbi_convert_vtx_by_cmd(cmd, dst);
		} else if (CMD_IS_COL(cmd)) {
			gbi_convert_col_by_cmd(cmd, dst);
		}

		host_cmd += HOST_DWORDS_PER_CMD;
		n64_cmd++;
	} while (!CMD_IS_ENDDL(cmd));

	return dstpos;
}
