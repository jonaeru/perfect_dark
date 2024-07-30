#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "system.h"

struct ptrmarker {
	uint32_t ptr_src;
	uintptr_t ptr_host;
};


struct ptrmarker m_PtrMarkers[2048];
int m_NumPtrMarkers;
int m_SizeGDLs;

static void add_marker(uint32_t ptr_src, uintptr_t ptr_host)
{
	if (m_NumPtrMarkers >= ARRAYCOUNT(m_PtrMarkers)) {
		fprintf(stderr, "[BG] Marker limit exceeded\n");
		exit(EXIT_FAILURE);
	}

	m_PtrMarkers[m_NumPtrMarkers].ptr_src = ptr_src;
	m_PtrMarkers[m_NumPtrMarkers].ptr_host = ptr_host;
	m_NumPtrMarkers++;
}

static struct ptrmarker* find_marker(uintptr_t ptr_src)
{
	for (int i = 0; i < m_NumPtrMarkers; i++) {
		if (m_PtrMarkers[i].ptr_src == ptr_src) {
			return &m_PtrMarkers[i];
		}
	}

	return NULL;
}

static void reset_markers()
{
	m_NumPtrMarkers = 0;
}

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
	uint32_t primary_infsize;
	uint32_t section1_cmpsize;
	uint32_t primary_cmpsize;
};

struct sectionheader {
	uint16_t infsize;
	uint16_t cmpsize;
};

struct n64_primaryheader {
	uint32_t unused1;
	uint32_t ptr_rooms;
	uint32_t ptr_portals;
	uint32_t ptr_bgcmds;
	uint32_t ptr_lights;
	uint32_t unused2;
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
	uint32_t ptr_gfxdata;
	uint32_t pos[3];
	uint8_t br_light_min;
	uint8_t br_light_max;
};

struct host_room {
	uintptr_t ptr_gfxdata;
	uint32_t pos[3];
	uint8_t br_light_min;
	uint8_t br_light_max;
};

struct portal {
	uint16_t verticesoffset;
	int16_t roomnum1;
	int16_t roomnum2;
	uint8_t flags;
};

struct portalvertices {
	uint8_t count;
	uint32_t vertices[1][3];
};

struct bgcmd {
	uint8_t type;
	uint8_t len;
	int32_t param;
};

struct light {
	uint16_t roomnum;
	uint16_t colour;
	uint8_t brightness;
	uint8_t sparkable : 1;
	uint8_t healthy : 1;
	uint8_t on : 1;
	uint8_t sparking : 1;
	uint8_t vulnerable : 1;
	uint8_t brightnessmult;
	int8_t dirx;
	int8_t diry;
	int8_t dirz;
	int16_t bbox[4][3];
};

struct n64_roomblock {
	uint8_t type;
	uint32_t ptr_next;
	uint32_t ptr_gdl;
	uint32_t ptr_vertices;
	uint32_t ptr_colours;
};

struct host_roomblock {
	uint8_t type;
	uintptr_t ptr_next;
	uintptr_t ptr_gdl;
	uintptr_t ptr_vertices;
	uintptr_t ptr_colours;
};

struct n64_roomgfxdata {
	uint32_t ptr_vertices;
	uint32_t ptr_colours;
	uint32_t ptr_opablocks;
	uint32_t ptr_xlublocks;
	int16_t lightsindex;
	int16_t numlights;
	int16_t numvertices;
	int16_t numcolours;
	// roomblocks[]
};

struct host_roomgfxdata {
	uintptr_t ptr_vertices;
	uintptr_t ptr_colours;
	uintptr_t ptr_opablocks;
	uintptr_t ptr_xlublocks;
	int16_t lightsindex;
	int16_t numlights;
	int16_t numvertices;
	int16_t numcolours;
	// roomblocks[]
};

struct vtx {
	int16_t coord[3];
	uint8_t flag;
	uint8_t color;
	uint16_t s;
	uint16_t t;
};

int m_NumRooms;

uint32_t m_AllocSizes[512];

static void convert_primary_rooms(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
{
	struct n64_room *n64_rooms = (struct n64_room *) &src[*srcpos];
	struct host_room *host_rooms = (struct host_room *) &dst[*dstpos];

	m_NumRooms = 0;

	for (int i = 1; n64_rooms[i].ptr_gfxdata != 0; i++) {
		m_NumRooms++;
	}

	for (int i = 0; i < m_NumRooms + 1; i++) {
		host_rooms[i].ptr_gfxdata = 0; // Not known at this point
		host_rooms[i].pos[0] = srctodst32(n64_rooms[i].pos[0]);
		host_rooms[i].pos[1] = srctodst32(n64_rooms[i].pos[1]);
		host_rooms[i].pos[2] = srctodst32(n64_rooms[i].pos[2]);
		host_rooms[i].br_light_min = n64_rooms[i].br_light_min;
		host_rooms[i].br_light_max = n64_rooms[i].br_light_max;
	}

	*srcpos += sizeof(*n64_rooms) * (m_NumRooms + 2);
	*dstpos += sizeof(*host_rooms) * (m_NumRooms +2);
}

static void convert_primary_portals(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos, uint32_t *src_portalvtxs, uint32_t *dst_portalvtxs)
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

static void convert_primary_bgcmds(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos, uint32_t dst_portalvtxs, uint32_t src_portalvtxs)
{
	struct bgcmd *n64_cmds = (struct bgcmd *) &src[*srcpos];
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
			uint32_t offset = host_cmds[i].param - src_portalvtxs;
			host_cmds[i].param = dst_portalvtxs + offset;
		}
	}

	*srcpos += sizeof(*n64_cmds) * num_cmds;
	*dstpos += sizeof(*host_cmds) * num_cmds;
}

static void convert_primary_lights(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos, uint32_t end)
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

	struct ptrmarker* marker = find_marker(*ptr);
	if (!marker) {
		fprintf(stderr, "[BG] WARNING: Unable to relink pointer: %p\n", ptr);
		exit(EXIT_FAILURE);
	}
	
	*ptr = marker->ptr_host;
}

static uint32_t convert_section1_roomgfxdata(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t compsize, uint32_t src_roomoffset, uint32_t dst_roomoffset)
{
	reset_markers();
	dst_roomoffset += 0x0f000000;
	size_t infsize = rzip_get_infsize(src);
	uint8_t* src_roomgfxdata = calloc(1, infsize);

	int ret = rzip_inflate(src_roomgfxdata, infsize, src, compsize);

	if (ret < 0) {
		fprintf(stderr, "Unable to inflate roomgfxdata %d\n", ret);
		exit(EXIT_FAILURE);
	}

	uintptr_t gdls_addr[64];
	int numgdls = 0;

	struct n64_roomgfxdata *src_header = (struct n64_roomgfxdata*)src_roomgfxdata;
	struct host_roomgfxdata *dst_header = (struct host_roomgfxdata*)&dst[dstpos];

	dst_header->ptr_vertices = srctoh32(src_header->ptr_vertices);
	dst_header->ptr_colours = srctoh32(src_header->ptr_colours);
	dst_header->ptr_opablocks = srctoh32(src_header->ptr_opablocks);
	dst_header->ptr_xlublocks = srctoh32(src_header->ptr_xlublocks);
	dst_header->lightsindex = srctoh16(src_header->lightsindex);
	dst_header->numlights = srctoh16(src_header->numlights);
	dst_header->numvertices = srctoh16(src_header->numvertices);
	dst_header->numcolours = srctoh16(src_header->numcolours);

	intptr_t endpos = dst_header->ptr_vertices - src_roomoffset;
	uintptr_t curpos_src = sizeof(struct n64_roomgfxdata);
	uintptr_t curpos_dst = sizeof(struct host_roomgfxdata);

	// roomblocks
	int ncoords = 0;
	
	// save the ptr to roomblocks here to iterate later to fix the pointers
	struct host_roomblock *ptr_dst_roomblocks = (struct host_roomblock*)&dst[curpos_dst];

	int numblocks = 0;
	while ((intptr_t)(curpos_src + sizeof(struct n64_roomblock)) <= endpos) {
		struct n64_roomblock *src_roomblock = (struct n64_roomblock*)&src_roomgfxdata[curpos_src];
		struct host_roomblock *dest_roomblock = (struct host_roomblock*)&dst[curpos_dst];

		dest_roomblock->type = src_roomblock->type;
		dest_roomblock->ptr_next = srctoh32(src_roomblock->ptr_next);
		dest_roomblock->ptr_gdl = srctoh32(src_roomblock->ptr_gdl);
		dest_roomblock->ptr_vertices = srctoh32(src_roomblock->ptr_vertices);
		dest_roomblock->ptr_colours = srctoh32(src_roomblock->ptr_colours);

		add_marker(curpos_src + src_roomoffset, curpos_dst + dst_roomoffset);

		if (src_roomblock->type == BLOCK_PARENT) {
			ncoords++;
			uintptr_t vtx_ptr = dest_roomblock->ptr_vertices - src_roomoffset;
			if (vtx_ptr < endpos) endpos = vtx_ptr;
		}
		else if (src_roomblock->ptr_gdl) {
			gdls_addr[numgdls++] = srctoh32(src_roomblock->ptr_gdl) - src_roomoffset;
		}

		curpos_src += sizeof(struct n64_roomblock);
		curpos_dst += sizeof(struct host_roomblock);
		numblocks++;
	}

	// block coords
	for (size_t i = 0; i < ncoords; i++) {
		uint32_t *src_coord = (uint32_t*)(src_roomgfxdata + curpos_src);
		uint32_t *dst_coord = (uint32_t*)(dst + curpos_dst);

		dst_coord[0] = srctoh32(src_coord[0]);
		dst_coord[1] = srctoh32(src_coord[1]);
		dst_coord[2] = srctoh32(src_coord[2]);
		
		dst_coord[3] = srctoh32(src_coord[3]);
		dst_coord[4] = srctoh32(src_coord[4]);
		dst_coord[5] = srctoh32(src_coord[5]);

		add_marker(curpos_src + src_roomoffset, curpos_dst + dst_roomoffset);

		curpos_src += 6 * sizeof(uint32_t);
		curpos_dst += 6 * sizeof(uint32_t);
	}

	curpos_dst = ALIGN8(curpos_dst);

	// vertices
	uintptr_t ptr_src_vertices = curpos_src = ALIGN8(dst_header->ptr_vertices - src_roomoffset);
	uintptr_t ptr_dst_vertices = curpos_dst + dst_roomoffset;

	add_marker(curpos_src + src_roomoffset, curpos_dst + dst_roomoffset);

	uintptr_t vtx_end = dst_header->ptr_colours - src_roomoffset;
	vtx_end = dst_header->ptr_colours == 0 ? infsize : vtx_end;
	
	while (curpos_src < vtx_end) {
		struct vtx *src_vtx = (struct vtx*)(src_roomgfxdata + curpos_src);
		struct vtx *dst_vtx = (struct vtx*)(dst + curpos_dst);

		dst_vtx->coord[0] = srctoh16(src_vtx->coord[0]);
		dst_vtx->coord[1] = srctoh16(src_vtx->coord[1]);
		dst_vtx->coord[2] = srctoh16(src_vtx->coord[2]);
		
		dst_vtx->flag = src_vtx->flag;
		dst_vtx->color= src_vtx->color;

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
		ptr_src_colors = curpos_src = ALIGN8(dst_header->ptr_colours - src_roomoffset);
		ptr_dst_colors = curpos_dst + dst_roomoffset;

		add_marker(curpos_src + src_roomoffset, curpos_dst + dst_roomoffset);
		
		uintptr_t col_end = numgdls > 0 != 0 ? gdls_addr[0] : infsize;
		size_t col_len = col_end - curpos_src;

		memcpy(dst + curpos_dst, src_roomgfxdata + curpos_src, col_len);
		curpos_src += col_len;
		curpos_dst += col_len;
	}

	// gdls
	gbi_set_vtx(src_header->ptr_vertices, ptr_src_vertices);
	gbi_set_segment(0x0e, dst_header->ptr_vertices);
	
	m_SizeGDLs = 0;
	uintptr_t gdlstart = curpos_dst;
	for (size_t i = 0; i < numgdls; i++) {
		add_marker(gdls_addr[i] + src_roomoffset, curpos_dst + dst_roomoffset);
		curpos_dst = gbi_convert_gdl(dst, curpos_dst, src_roomgfxdata, gdls_addr[i], 1);
	}
	m_SizeGDLs = curpos_dst - gdlstart;

	// relink ptrs: roomblocks
	for (size_t i = 0; i < numblocks; i++) {
		struct host_roomblock *block = (struct host_roomblock*)&ptr_dst_roomblocks[i];
		
		relink_ptr(&block->ptr_next);

		if (block->type == BLOCK_LEAF) {
			relink_ptr(&block->ptr_gdl);
			uint32_t offset_vtx = block->ptr_vertices - ptr_src_vertices - src_roomoffset;
			uint32_t offset_col = block->ptr_colours - ptr_src_colors - src_roomoffset;

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

	free(src_roomgfxdata);
	return curpos_dst;
}

static uint32_t convert_section1(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, struct fileheader *n64_header)
{
	struct fileheader *host_header = (struct fileheader *) &dst[dstpos];
	dstpos += sizeof(*host_header);

	// Unzip primary
	size_t infsize = srctoh32(n64_header->primary_infsize);
	uint8_t *srcbin = calloc(1, infsize);
	uint8_t *dstbin = calloc(1, infsize * 2);
	uint32_t srcbinpos = 0;
	uint32_t dstbinpos = 0;

	int ret = rzip_inflate(srcbin, infsize, &src[srcpos], srctoh32(n64_header->primary_cmpsize));

	if (ret < 0) {
		fprintf(stderr, "Unable to inflate BG primary data: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	// Convert primary
	struct host_primaryheader *host_primary_header = (struct host_primaryheader *) &dstbin[dstbinpos];
	struct n64_primaryheader *n64_primary_header = (struct n64_primaryheader *) &srcbin[srcbinpos];
	srcbinpos += sizeof(struct n64_primaryheader);
	dstbinpos += sizeof(*host_primary_header);

	uint32_t src_bgcmds = srctodst32(n64_primary_header->ptr_bgcmds) - 0x0f000000;

	host_primary_header->unused1 = 0;
	host_primary_header->unused2 = 0;

	host_primary_header->ptr_rooms = dstbinpos + 0x0f000000;
	convert_primary_rooms(dstbin, &dstbinpos, srcbin, &srcbinpos);
	
	if (n64_primary_header->ptr_lights) {
		host_primary_header->ptr_lights = dstbinpos + 0x0f000000;
		srcbinpos = srctodst32(n64_primary_header->ptr_lights) - 0x0f000000;
		convert_primary_lights(dstbin, &dstbinpos, srcbin, &srcbinpos, src_bgcmds);
	}

	host_primary_header->ptr_portals = dstbinpos + 0x0f000000;
	srcbinpos = srctodst32(n64_primary_header->ptr_portals) - 0x0f000000;
	uint32_t src_portalvtxs, dst_portalvtxs;
	convert_primary_portals(dstbin, &dstbinpos, srcbin, &srcbinpos, &src_portalvtxs, &dst_portalvtxs);

	host_primary_header->ptr_bgcmds = dstbinpos + 0x0f000000;
	srcbinpos = src_bgcmds;
	convert_primary_bgcmds(dstbin, &dstbinpos, srcbin, &srcbinpos, dst_portalvtxs, src_portalvtxs);

	// Convert room gfxdata
	uint32_t n64_primary_diff = srctoh32(n64_header->primary_infsize) - srctoh32(n64_header->primary_cmpsize) - 0xc;
	struct n64_room *n64_rooms = (struct n64_room *) &srcbin[sizeof(struct n64_primaryheader)];
	struct host_room *host_rooms = (struct host_room *) &dstbin[sizeof(struct host_primaryheader)];
	
	uint8_t *roomgfxbuf = calloc(1, (size_t)1024 * 1024);
	uint8_t *allcmp = calloc(1, (size_t)1024 * 1024);
	int cmppos = 0;

	host_rooms[0].ptr_gfxdata = 0;
	for (int i = 1; i <= m_NumRooms; i++) {
		host_rooms[i].ptr_gfxdata = dstbinpos + cmppos + 0x0f000000;

		if (i == m_NumRooms) break;

		uint32_t gfxdatacompsize = srctoh32(n64_rooms[i + 1].ptr_gfxdata) - srctoh32(n64_rooms[i].ptr_gfxdata);
		srcpos = srctoh32(n64_rooms[i].ptr_gfxdata) - n64_primary_diff - 0x0f000000;
		uint32_t src_roomoffset = srctoh32(n64_rooms[i].ptr_gfxdata);
		uint32_t roomgfxbufpos = convert_section1_roomgfxdata(roomgfxbuf, 0, src+srcpos, gfxdatacompsize, src_roomoffset, dstbinpos + cmppos);

		size_t ziplen = 0;
		ret = rzip_deflate(&allcmp[cmppos], &ziplen, roomgfxbuf, roomgfxbufpos);
		if (ret < 0) {
			fprintf(stderr, "Unable to compress BG roomgfxdata for room 0x%x: %d\n", i, ret);
			exit(EXIT_FAILURE);
		}

		ziplen = ALIGN16(ziplen);
		cmppos += ziplen;

		m_AllocSizes[i - 1] = roomgfxbufpos + (ziplen > m_SizeGDLs ? ziplen*4 : m_SizeGDLs*4);
	}
	
	host_rooms[m_NumRooms+1].ptr_gfxdata = 0;

	// Zip primary
	size_t ziplen = 0;

	printf("dstbin:%d = %p\n", __LINE__, dstbin);
	printf("dstpos:%d = %d\n", __LINE__, dstpos);
	printf("dstbinpos:%d = %d\n", __LINE__, dstbinpos);
	ret = rzip_deflate(&dst[dstpos], &ziplen, dstbin, dstbinpos);

	if (ret < 0) {
		fprintf(stderr, "Unable to compress BG primary data: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	printf("ziplen:%d = %d\n", __LINE__, ziplen);
	ziplen = ALIGN16(ziplen);
	dstpos += ziplen;

	// Paste compressed roomgfxdata after it
	memcpy(&dst[dstpos], allcmp, cmppos);
	dstpos += cmppos;

	free(roomgfxbuf);
	free(allcmp);
	free(srcbin);
	free(dstbin);

	host_header->primary_infsize = dstbinpos;
	host_header->section1_cmpsize = ziplen + cmppos;
	host_header->primary_cmpsize = ziplen;

	return dstpos;
}

static void convert_section2_texturenums(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos, uint32_t srclen)
{
	uint16_t *src_ids = (uint16_t *) &src[*srcpos];
	uint16_t *dst_ids = (uint16_t *) &dst[*dstpos];
	int count = srclen / sizeof(uint16_t);

	for (int i = 0; i < count; i++) {
		dst_ids[i] = srctodst16(src_ids[i]);
	}

	*srcpos += srclen;
	*dstpos += srclen;
}

static void convert_section3_bboxes(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
{
	int16_t *src_bboxes = (int16_t *) &src[*srcpos];
	int16_t *dst_bboxes = (int16_t *) &dst[*dstpos];

	for (int i = 0; i < m_NumRooms; i++) {
		dst_bboxes[i * 6 + 0] = srctodst16(src_bboxes[i * 6 + 0]);
		dst_bboxes[i * 6 + 1] = srctodst16(src_bboxes[i * 6 + 1]);
		dst_bboxes[i * 6 + 2] = srctodst16(src_bboxes[i * 6 + 2]);
		dst_bboxes[i * 6 + 3] = srctodst16(src_bboxes[i * 6 + 3]);
		dst_bboxes[i * 6 + 4] = srctodst16(src_bboxes[i * 6 + 4]);
		dst_bboxes[i * 6 + 5] = srctodst16(src_bboxes[i * 6 + 5]);
	}

	*srcpos += sizeof(int16_t) * 6 * (m_NumRooms - 1);
	*dstpos += sizeof(int16_t) * 6 * (m_NumRooms - 1);
}

static void convert_section3_allocsizes(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
{
	uint16_t *dst_sizes = (uint16_t *) &dst[*dstpos];

	for (int i = 0; i < m_NumRooms; i++) {
		dst_sizes[i] = (m_AllocSizes[i] - 0x100) / 0x10;
	}

	*srcpos += sizeof(uint16_t) * (m_NumRooms - 1);
	*dstpos += sizeof(uint16_t) * (m_NumRooms - 1);
}

static void convert_section3_lightcounts(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
{
	memcpy(&dst[*dstpos], &src[*srcpos], m_NumRooms);

	*srcpos += (m_NumRooms - 1);
	*dstpos += (m_NumRooms - 1);
}

static uint32_t convert_zipped_section(int section_num, uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, struct sectionheader *n64_header)
{
	struct sectionheader *host_header = (struct sectionheader *) &dst[dstpos];
	dstpos += sizeof(*host_header);

	// Unzip
	size_t infsize = rzip_get_infsize(&src[srcpos]);
	uint8_t *srcbin = calloc(1, infsize);
	uint8_t *dstbin = calloc(1, infsize);
	uint32_t srcbinpos = 0;
	uint32_t dstbinpos = 0;

	int ret = rzip_inflate(srcbin, infsize, &src[srcpos], srctoh16(n64_header->cmpsize));

	if (ret < 0) {
		fprintf(stderr, "Unable to inflate BG section %d: %d\n", section_num, ret);
		exit(EXIT_FAILURE);
	}

	// Convert
	if (section_num == 2) {
		convert_section2_texturenums(dstbin, &dstbinpos, srcbin, &srcbinpos, infsize);
	} else if (section_num == 3) {
		convert_section3_bboxes(dstbin, &dstbinpos, srcbin, &srcbinpos);
		convert_section3_allocsizes(dstbin, &dstbinpos, srcbin, &srcbinpos);
		convert_section3_lightcounts(dstbin, &dstbinpos, srcbin, &srcbinpos);
	}

	// Zip
	size_t ziplen = 0;

	ret = rzip_deflate(&dst[dstpos], &ziplen, dstbin, dstbinpos);

	if (ret < 0) {
		fprintf(stderr, "Unable to compress BG section %d: %d\n", section_num, ret);
		exit(EXIT_FAILURE);
	}

	ziplen = ALIGN16(ziplen);

	host_header->infsize = dstbinpos;
	host_header->cmpsize = ziplen;

	dstpos += ziplen;

	free(srcbin);
	free(dstbin);

	return dstpos;
}

void extract_file_bg(char *filename, uint32_t romoffset, size_t len)
{
	if (0) {
		if (strcmp(filename, "bgdata/bg_mp5.seg") != 0) {
			return;
		}
	}

	if (strcmp(filename, "ob/ob_mid.seg") == 0) {
		return;
	}

	size_t dstlen = len * 2;
	uint8_t *dst = calloc(1, dstlen);
	uint32_t dstpos = 0;
	m_NumPtrMarkers = 0;
	m_SizeGDLs = 0;
	gbi_reset();

	printf("--------------------------------------------------------------------------------\n");
	printf("-- %s\n", filename);

	uint8_t *src = &g_Rom[romoffset];
	struct fileheader *n64_fileheader = (struct fileheader *) src;
	uint32_t n64_section2_start = sizeof(*n64_fileheader) + srctoh32(n64_fileheader->section1_cmpsize);
	struct sectionheader *n64_section2header = (struct sectionheader *) &src[n64_section2_start];
	uint32_t n64_section3_start = n64_section2_start + sizeof(*n64_section2header) + (srctoh16(n64_section2header->cmpsize) & 0x7fff);
	struct sectionheader *n64_section3header = (struct sectionheader *) &src[n64_section3_start];

	printf("section2_start 0x%x\n", n64_section2_start);
	printf("section3_start 0x%x\n", n64_section3_start);

	dstpos = convert_section1(dst, dstpos, src, sizeof(*n64_fileheader), n64_fileheader);
	dstpos = convert_zipped_section(2, dst, dstpos, src, n64_section2_start + sizeof(*n64_section2header), n64_section2header);
	dstpos = convert_zipped_section(3, dst, dstpos, src, n64_section3_start + sizeof(*n64_section3header), n64_section3header);
	dstpos = ALIGN16(dstpos);

	sysLogPrintf(LOG_NOTE, "%s	%d	%d	%.3f", filename, len, dstpos, (f32)dstpos / len);

	char outfilename[1024];
	sprintf(outfilename, "%s/files/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(dst, dstpos, 1, fp);
	fclose(fp);

	free(dst);
}
