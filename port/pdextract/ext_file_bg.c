#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

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

int m_NumRooms;

uint16_t m_AllocSizes[512];

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

	*srcpos += sizeof(*n64_rooms) * (m_NumRooms + 1);
	*dstpos += sizeof(*host_rooms) * (m_NumRooms + 1);
}

static void convert_primary_portals(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
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

	// Convert portal vertices
	uint8_t *n64_vtxdata = &src[*srcpos];
	uint8_t *host_vtxdata = &dst[*dstpos];

	for (int i = 0; i < numportals; i++) {
		struct portalvertices *n64_pvertices = (struct portalvertices*) &n64_vtxdata[*srcpos];
		struct portalvertices *host_pvertices = (struct portalvertices*) &host_vtxdata[*dstpos];

		host_pvertices->count = n64_pvertices->count;

		for (int j = 0; j < host_pvertices->count; j++) {
			host_pvertices->vertices[j][0] = srctodst32(n64_pvertices->vertices[j][0]);
			host_pvertices->vertices[j][1] = srctodst32(n64_pvertices->vertices[j][1]);
			host_pvertices->vertices[j][2] = srctodst32(n64_pvertices->vertices[j][2]);
		}

		*srcpos += 4 + host_pvertices->count * 12;
		*dstpos += 4 + host_pvertices->count * 12;
	}
}

static void convert_primary_bgcmds(uint8_t *dst, uint32_t *dstpos, uint8_t *src, uint32_t *srcpos)
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

static uint32_t convert_section1_roomgfxdata(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	return dstpos;
}

static size_t sum(uint8_t *ptr, uint8_t *end)
{
	size_t sum = 0;

	while (ptr < end) {
		sum = (sum + *ptr) & 0xffffffff;
		ptr++;
	}

	return sum;
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
	srcbinpos += sizeof(struct n64_primaryheader);
	dstbinpos += sizeof(*host_primary_header);

	host_primary_header->unused1 = 0;
	host_primary_header->unused2 = 0;

	host_primary_header->ptr_rooms = dstbinpos;
	convert_primary_rooms(dstbin, &dstbinpos, srcbin, &srcbinpos);

	host_primary_header->ptr_portals = dstbinpos;
	convert_primary_portals(dstbin, &dstbinpos, srcbin, &srcbinpos);

	host_primary_header->ptr_bgcmds = dstbinpos;
	convert_primary_bgcmds(dstbin, &dstbinpos, srcbin, &srcbinpos);

	host_primary_header->ptr_lights = dstbinpos;
	convert_primary_lights(dstbin, &dstbinpos, srcbin, &srcbinpos, infsize);

	// Convert room gfxdata
	uint32_t n64_primary_diff = n64_header->primary_infsize - n64_header->primary_cmpsize;
	struct n64_room *n64_rooms = (struct n64_room *) &src[sizeof(*n64_header)];
	struct host_room *host_rooms = (struct host_room *) &dst[dstbinpos];

	uint8_t *roomgfxbuf = calloc(1, 1024 * 1024);
	uint8_t *allcmp = calloc(1, 1024 * 1024);
	int roomgfxbufpos = 0;
	int cmppos = 0;

	//for (int i = 0; i < m_NumRooms; i++) {
	//	host_rooms[i].ptr_gfxdata = dstbinpos + cmppos;

	//	srcpos = srctoh32(n64_rooms[i].ptr_gfxdata) - n64_primary_diff;
	//	roomgfxbufpos = convert_section1_roomgfxdata(roomgfxbuf, 0, src, srcpos);

	//	size_t ziplen = 0;
	//	ret = rzip_deflate(&allcmp[cmppos], &ziplen, roomgfxbuf, roomgfxbufpos);

	//	if (ret < 0) {
	//		fprintf(stderr, "Unable to compress BG roomgfxdata for room 0x%x: %d\n", i, ret);
	//		exit(EXIT_FAILURE);
	//	}

	//	ziplen = ALIGN16(ziplen);
	//	cmppos += ziplen;
	//}

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
		dst_sizes[i] = m_AllocSizes[i];
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
	return;

	size_t dstlen = len * 2;
	uint8_t *dst = calloc(1, dstlen);
	uint32_t dstpos = 0;

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

	char outfilename[1024];
	sprintf(outfilename, "%s/files/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(dst, dstpos, 1, fp);
	fclose(fp);

	free(dst);
}
