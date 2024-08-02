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

struct texaddr {
	uint32_t src;
	uint32_t dst;
};

uint32_t m_Segments[16];
uint32_t m_SrcVtxOffset;
uint32_t m_DstVtxOffset;

struct texaddr m_TexAddrs[64];
int m_NumTexAddr;

void gbi_reset(void)
{
	for (int i = 0; i < ARRAYCOUNT(m_Segments); i++) {
		m_Segments[i] = 0;
	}
	
	m_SrcVtxOffset = 0;
	m_DstVtxOffset = 0;
	m_NumTexAddr = 0;
}

void gbi_set_segment(int segment, uint32_t offset)
{
	m_Segments[segment] = offset & 0x00ffffff;
}

void gbi_set_vtx(uint32_t src_offset, uint32_t dst_offset)
{
	m_SrcVtxOffset = src_offset & 0x00ffffff;
	m_DstVtxOffset = dst_offset & 0x00ffffff;
}

void gbi_add_tex_addr(uint32_t src_offset, uint32_t dst_offset)
{
	m_TexAddrs[m_NumTexAddr].src   = src_offset & 0x00ffffff;
	m_TexAddrs[m_NumTexAddr++].dst = dst_offset & 0x00ffffff;
}

uint32_t gbi_find_tex_addr(uint32_t src_offset)
{
	for (int i = 0; i < m_NumTexAddr; i++) {
		if (m_TexAddrs[i].src == src_offset)
			return m_TexAddrs[i].dst;
	}

	return 0;
}

void gbi_convert_vtx(uint8_t *dst, uint32_t offset, int count)
{
	if (!offset) return;
	struct vtx* vtxs = (struct vtx*)(dst + offset);

	for (int i = 0; i < count; i++) {
		struct vtx* vtx = &vtxs[i];

		vtx->x = srctodst16(vtx->x);
		vtx->y = srctodst16(vtx->y);
		vtx->z = srctodst16(vtx->z);
		vtx->s = srctodst16(vtx->s);
		vtx->t = srctodst16(vtx->t);
	}
}

#define CMD_IS_VTX(cmd)      ((cmd & 0xff00000000000000) == 0x0400000000000000)
#define CMD_IS_DL(cmd)       ((cmd & 0xff00000000000000) == 0x0600000000000000)
#define CMD_IS_COL(cmd)      ((cmd & 0xff00000000000000) == 0x0700000000000000)
#define CMD_IS_ENDDL(cmd)    ((cmd & 0xff00000000000000) == 0xb800000000000000)
#define CMD_IS_MOVEWORD(cmd) ((cmd & 0xff00000000000000) == 0xbc00000000000000)
#define CMD_IS_MOVEMEM(cmd)  ((cmd & 0xff00000000000000) == 0x0300000000000000)
#define CMD_IS_MTX(cmd)      ((cmd & 0xff00000000000000) == 0x0100000000000000)
#define CMD_IS_SETTIMG(cmd)  ((cmd & 0xff00000000000000) == 0xfd00000000000000)
#define CMD_IS_SETZIMG(cmd)  ((cmd & 0xff00000000000000) == 0xfe00000000000000)
#define CMD_IS_SETCIMG(cmd)  ((cmd & 0xff00000000000000) == 0xff00000000000000)

#define CMD_HAS_ADDR(cmd) CMD_IS_VTX(cmd) \
	|| CMD_IS_COL(cmd) \
	|| CMD_IS_MOVEWORD(cmd) \
	|| CMD_IS_SETTIMG(cmd)


#define CMD_IS_SEGMENTED(cmd) ( CMD_IS_MOVEMEM(cmd) \
	|| CMD_IS_MTX(cmd) \
	|| CMD_IS_VTX(cmd) \
	|| CMD_IS_COL(cmd) \
	|| CMD_IS_DL(cmd) \
	|| CMD_IS_SETTIMG(cmd) \
	|| CMD_IS_SETCIMG(cmd) \
	|| CMD_IS_SETZIMG(cmd))

/**
 * Segment 5 is the start of the model file. The data in these files shifts
 * depending on the pointer size, so this function adjusts the offset accordingly.
 */
static uint64_t gbi_rewrite_addr(uint64_t cmd, uint8_t opcode)
{
	uint8_t segment = (cmd & 0x0f000000) >> 24;
	uint32_t offset = cmd & 0x00ffffff;

	if (segment == 5) {
		if (opcode == 0xfd) {
			offset = gbi_find_tex_addr(offset);
		}
		else {
			if (offset < m_SrcVtxOffset) {
				printf("Tried to load data from offset 0x%x but segment starts at 0x%x\n", offset, m_SrcVtxOffset);
				exit(EXIT_FAILURE);
			}

			offset = offset - m_SrcVtxOffset + m_DstVtxOffset;
		}
	}

	return (cmd & 0xffffffffff000000) | offset;
}

void gbi_gdl_rewrite_addrs(uint8_t *dst, uint32_t offset)
{
	if (!offset) return;

	uint64_t *cmds = (uint64_t*)(dst + (offset & 0x00ffffff));
	if (!cmds) return;

	uint64_t cmd;

	do {
		cmd = *cmds;

		if (CMD_HAS_ADDR((cmd << 32))) {
			uint8_t opcode = (cmd >> 24) & 0xff;
			cmds[1] = gbi_rewrite_addr(cmds[1], opcode);
		}

		if (CMD_IS_SEGMENTED((cmd << 32))) {
			cmds[1] |= 1;
		}

		cmds += 2;
	} while (!CMD_IS_ENDDL((cmd << 32)));
}

uint32_t gbi_convert_gdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int segment_cmds)
{
	dstpos = ALIGN8(dstpos);

	uint64_t *n64_cmd = (uint64_t*)&src[srcpos];
	uint64_t *host_cmd = (uint64_t*)&dst[dstpos];
	uint64_t cmd;

	do {
		cmd = srctoh64(*n64_cmd);

#if HOST_DWORDS_PER_CMD == 2
		host_cmd[0] = htodst64((cmd & 0xffffffff00000000) >> 32);
		host_cmd[1] = htodst64(cmd & 0xffffffff);

		if (segment_cmds && CMD_IS_SEGMENTED(cmd)) {
			host_cmd[1] |= 1;
		}
#else
		host_cmd[0] = htodst64(cmd);

		if (segment_cmds && CMD_IS_SEGMENTED(cmd)) {
			host_cmd[0] |= 1;
		}
#endif

		dstpos += sizeof(*host_cmd) * HOST_DWORDS_PER_CMD;
		host_cmd += HOST_DWORDS_PER_CMD;
		n64_cmd++;
	} while (!CMD_IS_ENDDL(cmd));

	return dstpos;
}
