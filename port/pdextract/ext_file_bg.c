#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"
#include "platform.h"

int m_SizeGDLs;

/**
 * 4 bytes decompressed size of primary data
 * 4 bytes compressed size of section 1 in its entirety
 * 4 bytes compressed size of primary data
 * Section 1:
 *     Primary: (zipped)
 *         4 bytes null
 *         4 bytes pointer to room table
 *         4 bytes pointer to portal table
 *         4 bytes pointer to bgcmds
 *         4 bytes pointer to lights table
 *         4 bytes null
 *         (room table)
 *         (portal table)
 *         (bgcmds)
 *         (lights table)
 *     room 1 (zipped)
 *     room 2 (zipped)
 *     ...
 * Section 2:
 *     2 bytes decompressed size of section (mask with 0x7fff)
 *     2 bytes compressed size of section
 *     Texture ID list (zipped)
 * Section 3:
 *     2 bytes decompressed size of section (mask with 0x7fff)
 *     2 bytes compressed size of section
 *     Zipped:
 *         (room bbox table)
 *         (list of roomgfxdata sizes)
 *         (list of light counts per room)
 */

struct fileheader {
	u32 primary_infsize;
	u32 section1_cmpsize;
	u32 primary_cmpsize;
};

struct sectionheader {
	u16 infsize;
	u16 cmpsize;
};

struct n64_primaryheader {
	u32 unused1;
	u32 ptr_rooms;
	u32 ptr_portals;
	u32 ptr_bgcmds;
	u32 ptr_lights;
	u32 unused2;
};

struct host_primaryheader {
	uintptr_t unused1;
	uintptr_t ptr_rooms;
	uintptr_t ptr_portals;
	uintptr_t ptr_bgcmds;
	uintptr_t ptr_lights;
	uintptr_t unused2;
};

struct n64_room {
	u32 ptr_gfxdata;
	u32 pos[3];
	u8 br_light_min;
	u8 br_light_max;
};

struct host_room {
	uintptr_t ptr_gfxdata;
	u32 pos[3];
	u8 br_light_min;
	u8 br_light_max;
};

struct portal {
	u16 verticesoffset;
	s16 roomnum1;
	s16 roomnum2;
	u8 flags;
};

struct portalvertices {
	u8 count;
	u32 vertices[1][3];
};

struct bgcmd {
	u8 type;
	u8 len;
	s32 param;
};

struct light {
	u16 roomnum;
	u16 colour;
	u8 brightness;
	u8 sparkable : 1;
	u8 healthy : 1;
	u8 on : 1;
	u8 sparking : 1;
	u8 vulnerable : 1;
	u8 brightnessmult;
	s8 dirx;
	s8 diry;
	s8 dirz;
	s16 bbox[4][3];
};

struct n64_roomblock {
	u8 type;
	u32 ptr_next;
	u32 ptr_gdl;
	u32 ptr_vertices;
	u32 ptr_colours;
};

struct host_roomblock {
	u8 type;
	uintptr_t ptr_next;
	uintptr_t ptr_gdl;
	uintptr_t ptr_vertices;
	uintptr_t ptr_colours;
};

struct n64_roomgfxdata {
	u32 ptr_vertices;
	u32 ptr_colours;
	u32 ptr_opablocks;
	u32 ptr_xlublocks;
	s16 lightsindex;
	s16 numlights;
	s16 numvertices;
	s16 numcolours;
	// roomblocks[]
};

struct host_roomgfxdata {
	uintptr_t ptr_vertices;
	uintptr_t ptr_colours;
	uintptr_t ptr_opablocks;
	uintptr_t ptr_xlublocks;
	s16 lightsindex;
	s16 numlights;
	s16 numvertices;
	s16 numcolours;
	// roomblocks[]
};

struct vtx {
	s16 coord[3];
	u8 flag;
	u8 color;
	u16 s;
	u16 t;
};

int m_NumRooms;

u32 m_AllocSizes[512];

static void convert_primary_rooms(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos)
{
	struct n64_room *n64_rooms = (struct n64_room *) &src[*srcpos];
	struct host_room *host_rooms = (struct host_room *) &dst[*dstpos];

	m_NumRooms = 0;

	for (int i = 1; n64_rooms[i].ptr_gfxdata != 0; i++) {
		m_NumRooms++;
	}

	for (int i = 0; i < m_NumRooms + 1; i++) {
		host_rooms[i].ptr_gfxdata = srctodst32(n64_rooms[i].ptr_gfxdata);
		host_rooms[i].pos[0] = srctodst32(n64_rooms[i].pos[0]);
		host_rooms[i].pos[1] = srctodst32(n64_rooms[i].pos[1]);
		host_rooms[i].pos[2] = srctodst32(n64_rooms[i].pos[2]);
		host_rooms[i].br_light_min = n64_rooms[i].br_light_min;
		host_rooms[i].br_light_max = n64_rooms[i].br_light_max;
	}

	*srcpos += sizeof(*n64_rooms) * (m_NumRooms + 2);
	*dstpos += sizeof(*host_rooms) * (m_NumRooms +2);
}

static void convert_primary_portals(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos, u32 *src_portalvtxs, u32 *dst_portalvtxs)
{
	struct portal *n64_portals = (struct portal *) &src[*srcpos];
	struct portal *host_portals = (struct portal *) &dst[*dstpos];

	// Count portals
	int numportals = 0;

	for (int i = 0; n64_portals[i].verticesoffset != 0; i++) {
		numportals++;
	}

	// Convert portals
	for (int i = 0; i < numportals; i++) {
		host_portals[i].verticesoffset = srctodst16(n64_portals[i].verticesoffset);
		host_portals[i].roomnum1 = srctodst16(n64_portals[i].roomnum1);
		host_portals[i].roomnum2 = srctodst16(n64_portals[i].roomnum2);
		host_portals[i].flags = n64_portals[i].flags;
	}

	*srcpos += sizeof(*n64_portals) * (numportals + 1);
	*dstpos += sizeof(*host_portals) * (numportals + 1);

	*src_portalvtxs = *srcpos;
	*dst_portalvtxs = *dstpos;

	while (1) {
		struct portalvertices *n64_pvertices = (struct portalvertices*)&src[*srcpos];
		struct portalvertices *host_pvertices = (struct portalvertices*)&dst[*dstpos];

		host_pvertices->count = n64_pvertices->count;

		for (int j = 0; j < host_pvertices->count; j++) {
			host_pvertices->vertices[j][0] = srctodst32(n64_pvertices->vertices[j][0]);
			host_pvertices->vertices[j][1] = srctodst32(n64_pvertices->vertices[j][1]);
			host_pvertices->vertices[j][2] = srctodst32(n64_pvertices->vertices[j][2]);
		}

		*srcpos += 4 + host_pvertices->count * 12;
		*dstpos += 4 + host_pvertices->count * 12;

		if (host_pvertices->count == 0)  break;
	}
}

static void convert_primary_bgcmds(u8 *dst, u32 *dstpos, u8 *src, u32 dst_portalvtxs, u32 src_portalvtxs)
{
	struct bgcmd *n64_cmds = (struct bgcmd *) src;
	struct bgcmd *host_cmds = (struct bgcmd *) &dst[*dstpos];
	int num_cmds = 0;

	for (int i = 0; n64_cmds[i].type != 0; i++) {
		num_cmds++;
	}

	num_cmds++;

	for (int i = 0; i < num_cmds; i++) {
		host_cmds[i].type = n64_cmds[i].type;
		host_cmds[i].len = n64_cmds[i].len;
		host_cmds[i].param = srctodst32(n64_cmds[i].param);

		if (host_cmds[i].type == 0x64) {
			u32 offset = host_cmds[i].param - src_portalvtxs;
			host_cmds[i].param = dst_portalvtxs + offset;
		}
	}

	*dstpos += sizeof(*host_cmds) * num_cmds;
}

static void convert_primary_lights(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos, u32 end)
{
	struct light *n64_lights = (struct light *) &src[*srcpos];
	struct light *host_lights = (struct light *) &dst[*dstpos];
	int num_lights = (end - *srcpos) / sizeof(struct light);

	for (int i = 0; i < num_lights; i++) {
		host_lights[i].roomnum = srctodst16(n64_lights[i].roomnum);
		host_lights[i].colour = srctodst16(n64_lights[i].colour);
		host_lights[i].brightness = n64_lights[i].brightness;
		host_lights[i].sparkable = n64_lights[i].sparkable;
		host_lights[i].healthy = n64_lights[i].healthy;
		host_lights[i].on = n64_lights[i].on;
		host_lights[i].sparking = n64_lights[i].sparking;
		host_lights[i].vulnerable = n64_lights[i].vulnerable;
		host_lights[i].brightnessmult = n64_lights[i].brightnessmult;
		host_lights[i].dirx = n64_lights[i].dirx;
		host_lights[i].diry = n64_lights[i].diry;
		host_lights[i].dirz = n64_lights[i].dirz;

		for (int j = 0; j < 4; j++) {
			host_lights[i].bbox[j][0] = srctodst16(n64_lights[i].bbox[j][0]);
			host_lights[i].bbox[j][1] = srctodst16(n64_lights[i].bbox[j][1]);
			host_lights[i].bbox[j][2] = srctodst16(n64_lights[i].bbox[j][2]);
		}
	}

	*srcpos += sizeof(*n64_lights) * num_lights;
	*dstpos += sizeof(*host_lights) * num_lights;
}

#define BLOCK_LEAF 0
#define BLOCK_PARENT 1

void relink_ptr(uintptr_t** ptr)
{
	if (!*ptr) return;

	struct ptrmarker* marker = find_ptr_marker(*ptr);
	if (!marker) {
		fprintf(stderr, "[BG] WARNING: Unable to relink pointer: %p\n", ptr);
		exit(EXIT_FAILURE);
	}
	
	*ptr = marker->ptr_host;
}

static u32 convert_section1_roomgfxdata(u8 *dst, u8 *src, u32 infsize, u32 src_ofs)
{
	reset_markers();

	u32 dst_roomoffset = src_ofs;

	uintptr_t gdls_addr[64];
	int numgdls = 0;

	struct n64_roomgfxdata *src_header = (struct n64_roomgfxdata*)src;
	struct host_roomgfxdata *dst_header = (struct host_roomgfxdata*)dst;

	dst_header->ptr_vertices = srctoh32(src_header->ptr_vertices);
	dst_header->ptr_colours = srctoh32(src_header->ptr_colours);
	dst_header->ptr_opablocks = srctoh32(src_header->ptr_opablocks);
	dst_header->ptr_xlublocks = srctoh32(src_header->ptr_xlublocks);
	dst_header->lightsindex = srctoh16(src_header->lightsindex);
	dst_header->numlights = srctoh16(src_header->numlights);
	dst_header->numvertices = srctoh16(src_header->numvertices);
	dst_header->numcolours = srctoh16(src_header->numcolours);

	intptr_t endpos = dst_header->ptr_vertices - src_ofs;
	uintptr_t curpos_src = sizeof(struct n64_roomgfxdata);
	uintptr_t curpos_dst = sizeof(struct host_roomgfxdata);

	// roomblocks
	int ncoords = 0;
	
	// save the ptr to roomblocks here to iterate later to fix the pointers
	struct host_roomblock *ptr_dst_roomblocks = (struct host_roomblock*)&dst[curpos_dst];

	int numblocks = 0;
	while ((intptr_t)(curpos_src + sizeof(struct n64_roomblock)) <= endpos) {
		struct n64_roomblock *src_roomblock = (struct n64_roomblock*)&src[curpos_src];
		struct host_roomblock *dest_roomblock = (struct host_roomblock*)&dst[curpos_dst];

		dest_roomblock->type = src_roomblock->type;
		dest_roomblock->ptr_next = srctoh32(src_roomblock->ptr_next);
		dest_roomblock->ptr_gdl = srctoh32(src_roomblock->ptr_gdl);
		dest_roomblock->ptr_vertices = srctoh32(src_roomblock->ptr_vertices);
		dest_roomblock->ptr_colours = srctoh32(src_roomblock->ptr_colours);

		add_marker(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		if (src_roomblock->type == BLOCK_PARENT) {
			ncoords++;
			uintptr_t vtx_ptr = dest_roomblock->ptr_vertices - src_ofs;
			if (vtx_ptr < endpos) endpos = vtx_ptr;
		}
		else if (src_roomblock->ptr_gdl) {
			gdls_addr[numgdls++] = srctoh32(src_roomblock->ptr_gdl) - src_ofs;
		}

		curpos_src += sizeof(struct n64_roomblock);
		curpos_dst += sizeof(struct host_roomblock);
		numblocks++;
	}

	// block coords
	for (size_t i = 0; i < ncoords; i++) {
		u32 *src_coord = (u32*)(src + curpos_src);
		u32 *dst_coord = (u32*)(dst + curpos_dst);

		dst_coord[0] = srctoh32(src_coord[0]);
		dst_coord[1] = srctoh32(src_coord[1]);
		dst_coord[2] = srctoh32(src_coord[2]);
		
		dst_coord[3] = srctoh32(src_coord[3]);
		dst_coord[4] = srctoh32(src_coord[4]);
		dst_coord[5] = srctoh32(src_coord[5]);

		add_marker(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		curpos_src += 6 * sizeof(u32);
		curpos_dst += 6 * sizeof(u32);
	}

	curpos_dst = ALIGN8(curpos_dst);

	// vertices
	uintptr_t ptr_src_vertices = curpos_src = ALIGN8(dst_header->ptr_vertices - src_ofs);
	uintptr_t ptr_dst_vertices = curpos_dst + dst_roomoffset;

	add_marker(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

	uintptr_t vtx_end = dst_header->ptr_colours - src_ofs;

	while (dst_header->ptr_colours && curpos_src < vtx_end) {
		struct vtx *src_vtx = (struct vtx*)(src + curpos_src);
		struct vtx *dst_vtx = (struct vtx*)(dst + curpos_dst);

		dst_vtx->coord[0] = srctoh16(src_vtx->coord[0]);
		dst_vtx->coord[1] = srctoh16(src_vtx->coord[1]);
		dst_vtx->coord[2] = srctoh16(src_vtx->coord[2]);
		
		dst_vtx->flag = src_vtx->flag;
		dst_vtx->color = src_vtx->color;

		dst_vtx->s = srctoh16(src_vtx->s);
		dst_vtx->t = srctoh16(src_vtx->t);

		curpos_src += sizeof(struct vtx);
		curpos_dst += sizeof(struct vtx);
	}

	curpos_dst = ALIGN8(curpos_dst);

	// colors
	uintptr_t ptr_src_colors = 0;
	uintptr_t ptr_dst_colors = 0;

	if (dst_header->ptr_colours) {
		ptr_src_colors = curpos_src = ALIGN8(dst_header->ptr_colours - src_ofs);
		ptr_dst_colors = curpos_dst + dst_roomoffset;

		add_marker(curpos_src + src_ofs, curpos_dst + dst_roomoffset);

		uintptr_t col_end = numgdls > 0 != 0 ? gdls_addr[0] : infsize;
		size_t col_len = col_end - curpos_src;

		memcpy(dst + curpos_dst, src + curpos_src, col_len);
		curpos_src += col_len;
		curpos_dst += col_len;
	}

	// gdls
	gbi_set_vtx(src_header->ptr_vertices, ptr_src_vertices);
	gbi_set_segment(0x0e, dst_header->ptr_vertices);
	
	m_SizeGDLs = 0;
	uintptr_t gdlstart = curpos_dst;
	for (size_t i = 0; i < numgdls; i++) {
		add_marker(gdls_addr[i] + src_ofs, curpos_dst + dst_roomoffset);
		curpos_dst = gbi_convert_gdl(dst, curpos_dst, src, gdls_addr[i], 1);
	}
	m_SizeGDLs = curpos_dst - gdlstart;

	// relink ptrs: roomblocks
	for (size_t i = 0; i < numblocks; i++) {
		struct host_roomblock *block = (struct host_roomblock*)&ptr_dst_roomblocks[i];
		
		relink_ptr(&block->ptr_next);

		if (block->type == BLOCK_LEAF) {
			relink_ptr(&block->ptr_gdl);
			u32 offset_vtx = block->ptr_vertices - ptr_src_vertices - src_ofs;
			u32 offset_col = block->ptr_colours - ptr_src_colors - src_ofs;

			block->ptr_vertices = ptr_dst_vertices + offset_vtx;
			block->ptr_colours = ptr_dst_colors + offset_col;
		}
		else {
			relink_ptr(&block->ptr_gdl); // child
			relink_ptr(&block->ptr_vertices); // coord
		}
	}
	
	// relink ptrs: header
	relink_ptr(&dst_header->ptr_vertices);
	relink_ptr(&dst_header->ptr_colours);
	relink_ptr(&dst_header->ptr_opablocks);
	relink_ptr(&dst_header->ptr_xlublocks);

	return curpos_dst;
}

static u32 convert_section1(u8 *dst, u8 *src, u32 ofs)
{
	// Convert primary
	u32 srcpos = 0;
	u32 dstpos = 0;
	struct host_primaryheader *host_primary_header = (struct host_primaryheader *) dst;
	struct n64_primaryheader *n64_primary_header = (struct n64_primaryheader *) src;
	srcpos += sizeof(struct n64_primaryheader);
	dstpos += sizeof(*host_primary_header);

	u32 src_bgcmds = srctodst32(n64_primary_header->ptr_bgcmds) - ofs;

	host_primary_header->unused1 = 0;
	host_primary_header->unused2 = 0;

	host_primary_header->ptr_rooms = dstpos + ofs;
	convert_primary_rooms(dst, &dstpos, src, &srcpos);
	
	if (n64_primary_header->ptr_lights) {
		host_primary_header->ptr_lights = dstpos + ofs;
		srcpos = srctodst32(n64_primary_header->ptr_lights) - ofs;
		convert_primary_lights(dst, &dstpos, src, &srcpos, src_bgcmds);
	}

	host_primary_header->ptr_portals = dstpos + ofs;
	srcpos = srctodst32(n64_primary_header->ptr_portals) - ofs;
	u32 src_portalvtxs, dst_portalvtxs;
	convert_primary_portals(dst, &dstpos, src, &srcpos, &src_portalvtxs, &dst_portalvtxs);

	host_primary_header->ptr_bgcmds = dstpos + ofs;
	convert_primary_bgcmds(dst, &dstpos, &src[src_bgcmds], dst_portalvtxs, src_portalvtxs);

	return dstpos;
}

static void convert_section2_texturenums(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos, u32 srclen)
{
	u16 *src_ids = (u16 *) &src[*srcpos];
	u16 *dst_ids = (u16 *) &dst[*dstpos];
	int count = srclen / sizeof(u16);

	for (int i = 0; i < count; i++) {
		dst_ids[i] = srctodst16(src_ids[i]);
	}

	*srcpos += srclen;
	*dstpos += srclen;
}

static void convert_section3_bboxes(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos)
{
	s16 *src_bboxes = (s16 *) &src[*srcpos];
	s16 *dst_bboxes = (s16 *) &dst[*dstpos];

	for (int i = 0; i < m_NumRooms; i++) {
		dst_bboxes[i * 6 + 0] = srctodst16(src_bboxes[i * 6 + 0]);
		dst_bboxes[i * 6 + 1] = srctodst16(src_bboxes[i * 6 + 1]);
		dst_bboxes[i * 6 + 2] = srctodst16(src_bboxes[i * 6 + 2]);
		dst_bboxes[i * 6 + 3] = srctodst16(src_bboxes[i * 6 + 3]);
		dst_bboxes[i * 6 + 4] = srctodst16(src_bboxes[i * 6 + 4]);
		dst_bboxes[i * 6 + 5] = srctodst16(src_bboxes[i * 6 + 5]);
	}

	*srcpos += sizeof(s16) * 6 * (m_NumRooms - 1);
	*dstpos += sizeof(s16) * 6 * (m_NumRooms - 1);
}

static void convert_section3_allocsizes(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos)
{
	u16 *dst_sizes = (u16 *) &dst[*dstpos];

	for (int i = 0; i < m_NumRooms; i++) {
		dst_sizes[i] = (m_AllocSizes[i] - 0x100) / 0x10;
	}

	*srcpos += sizeof(u16) * (m_NumRooms - 1);
	*dstpos += sizeof(u16) * (m_NumRooms - 1);
}

static void convert_section3_lightcounts(u8 *dst, u32 *dstpos, u8 *src, u32 *srcpos)
{
	memcpy(&dst[*dstpos], &src[*srcpos], m_NumRooms);

	*srcpos += (m_NumRooms - 1);
	*dstpos += (m_NumRooms - 1);
}

void preprocessBgSection1(u8 *data, u32 size, u32 ofs)
{
	u8 *dst = sysMemZeroAlloc(size);

	u32 newSize = convert_section1(dst, data, ofs);

	if (newSize > size) {
		sysLogPrintf(LOG_ERROR, "overflow when trying to preprocess a bg file, size %d newsize %d", size, newSize);
		exit(EXIT_FAILURE);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);
}

u32 preprocessBgRoom(u8 *data, u32 size, u32 room_ofs)
{
	size *= 2;
	u8 *dst = sysMemZeroAlloc(size);
	u32 newSize = convert_section1_roomgfxdata(dst, data, size, room_ofs);

	if (newSize > size) {
		sysLogPrintf(LOG_ERROR, "overflow when trying to preprocess a bg room, size %d newsize %d", size, newSize);
		exit(EXIT_FAILURE);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	return newSize;
}
