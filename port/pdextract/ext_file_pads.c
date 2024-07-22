#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define PADFLAG_INTPOS          0x0001
#define PADFLAG_UPALIGNTOX      0x0002
#define PADFLAG_UPALIGNTOY      0x0004
#define PADFLAG_UPALIGNTOZ      0x0008
#define PADFLAG_UPALIGNINVERT   0x0010
#define PADFLAG_LOOKALIGNTOX    0x0020
#define PADFLAG_LOOKALIGNTOY    0x0040
#define PADFLAG_LOOKALIGNTOZ    0x0080
#define PADFLAG_LOOKALIGNINVERT 0x0100
#define PADFLAG_HASBBOXDATA     0x0200

struct n64_header {
	int32_t num_pads;
	int32_t num_covers;
	uint32_t ptr_waypoints;
	uint32_t ptr_waygroups;
	uint32_t ptr_cover;
};

struct host_header {
	int32_t num_pads;
	int32_t num_covers;
	uintptr_t ptr_waypoints;
	uintptr_t ptr_waygroups;
	uintptr_t ptr_cover;
};

struct padheader {
	uint32_t flags : 18;
	uint32_t room : 10;
	uint32_t liftnum : 4;
};

struct n64_waypoint {
	int32_t padnum;
	uint32_t ptr_neighbours;
	int32_t groupnum;
	int32_t step;
};

struct host_waypoint {
	int32_t padnum;
	uintptr_t ptr_neighbours;
	int32_t groupnum;
	int32_t step;
};

struct n64_waygroup {
	uint32_t ptr_neighbours;
	uint32_t ptr_waypoints;
	int32_t step;
};

struct host_waygroup {
	uintptr_t ptr_neighbours;
	uintptr_t ptr_waypoints;
	int32_t step;
};

struct coverdefinition {
	uint32_t pos[3];
	uint32_t dir[3];
	uint16_t flags;
	uint16_t unk1a;
};

static int convert_pads(uint8_t *dst, int dstpos, uint8_t *src, int srcpos, int num_pads)
{
	uint16_t *src_offsets = (uint16_t *) &src[srcpos];
	uint16_t *dst_offsets = (uint16_t *) &dst[dstpos];

	dstpos += num_pads * sizeof(uint16_t);

	for (int i = 0; i < num_pads; i++) {
		dstpos = ALIGN4(dstpos);
		srcpos = srctoh16(src_offsets[i]);

		dst_offsets[i] = htodst16(dstpos);

		// Header
		uint32_t n64_padheader = srctoh32(*(uint32_t *) &src[srcpos]);
		struct padheader *host_padheader = (struct padheader *) &dst[dstpos];
		uint32_t flags = (n64_padheader >> 14) & 0x3ffff;

		*(uint32_t*)host_padheader = htodst32(n64_padheader);

		srcpos += sizeof(struct padheader);
		dstpos += sizeof(struct padheader);

		// Position
		if (flags & PADFLAG_INTPOS) {
			int16_t *srcptr = (int16_t *) &src[srcpos];
			int16_t *dstptr = (int16_t *) &dst[dstpos];

			dstptr[0] = srctodst16(srcptr[0]);
			dstptr[1] = srctodst16(srcptr[1]);
			dstptr[2] = srctodst16(srcptr[2]);

			srcpos += 8;
			dstpos += 8;
		} else {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Up
		if ((flags & (PADFLAG_UPALIGNTOX | PADFLAG_UPALIGNTOY | PADFLAG_UPALIGNTOZ)) == 0) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Look
		if ((flags & (PADFLAG_LOOKALIGNTOX | PADFLAG_LOOKALIGNTOY | PADFLAG_LOOKALIGNTOZ)) == 0) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Bbox
		if (flags & PADFLAG_HASBBOXDATA) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);
			dstptr[3] = srctodst32(srcptr[3]);
			dstptr[4] = srctodst32(srcptr[4]);
			dstptr[5] = srctodst32(srcptr[5]);

			srcpos += 4 * 6;
			dstpos += 4 * 6;
		}
	}

	return dstpos;
}

static int convert_waypoints(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
{
	struct n64_waypoint *n64_waypoints = (struct n64_waypoint *) &src[srcpos];
	struct host_waypoint *host_waypoints = (struct host_waypoint *) &dst[dstpos];
	int num_waypoints;

	for (num_waypoints = 0; n64_waypoints[num_waypoints].padnum != -1; num_waypoints++);

	dstpos += (num_waypoints + 1) * sizeof(struct host_waypoint);

	uint32_t *host_neighbours = (uint32_t *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waypoints; i++) {
		host_waypoints[i].padnum = srctodst32(n64_waypoints[i].padnum);
		host_waypoints[i].ptr_neighbours = htodst32(dstpos);
		host_waypoints[i].groupnum = srctodst32(n64_waypoints[i].groupnum);
		host_waypoints[i].step = 0;

		uint32_t *n64_neighbours = (uint32_t *) &src[srctoh32(n64_waypoints[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = srctodst32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waypoints[num_waypoints].padnum = 0xffffffff;
	host_waypoints[num_waypoints].ptr_neighbours = 0;
	host_waypoints[num_waypoints].groupnum = 0;
	host_waypoints[num_waypoints].step = 0;

	return dstpos;
}

static int convert_waygroups(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
{
	struct n64_waygroup *n64_waygroups = (struct n64_waygroup *) &src[srcpos];
	struct host_waygroup *host_waygroups = (struct host_waygroup *) &dst[dstpos];
	int num_waygroups;

	for (num_waygroups = 0; n64_waygroups[num_waygroups].ptr_neighbours != 0; num_waygroups++);

	dstpos += (num_waygroups + 1) * sizeof(struct host_waygroup);

	// Waygroups and child waypoints
	uint32_t *host_waypoints = (uint32_t *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].ptr_waypoints = htodst32(dstpos);
		host_waygroups[i].step = 0;

		uint32_t *n64_waypoints = (uint32_t *) &src[srctoh32(n64_waygroups[i].ptr_waypoints)];

		for (int j = 0; n64_waypoints[j] != 0xffffffff; j++) {
			host_waypoints[n++] = srctodst32(n64_waypoints[j]);
			dstpos += 4;
		}

		host_waypoints[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waygroups[num_waygroups].ptr_neighbours = 0;
	host_waygroups[num_waygroups].ptr_waypoints = 0;
	host_waygroups[num_waygroups].step = 0;

	// Waygroup neighbours
	uint32_t *host_neighbours = (uint32_t *) &dst[dstpos];
	n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].ptr_neighbours = htodst32(dstpos);

		uint32_t *n64_neighbours = (uint32_t *) &src[srctoh32(n64_waygroups[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = srctodst32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	return dstpos;
}

static int convert_cover(uint8_t *dst, int dstpos, uint8_t *src, int srcpos, int num_covers)
{
	struct coverdefinition *n64_covers = (struct coverdefinition *) &src[srcpos];
	struct coverdefinition *host_covers = (struct coverdefinition *) &dst[dstpos];

	for (int i = 0; i < num_covers; i++) {
		host_covers[i].pos[0] = srctodst32(n64_covers[i].pos[0]);
		host_covers[i].pos[1] = srctodst32(n64_covers[i].pos[1]);
		host_covers[i].pos[2] = srctodst32(n64_covers[i].pos[2]);
		host_covers[i].dir[0] = srctodst32(n64_covers[i].dir[0]);
		host_covers[i].dir[1] = srctodst32(n64_covers[i].dir[1]);
		host_covers[i].dir[2] = srctodst32(n64_covers[i].dir[2]);
		host_covers[i].flags = srctodst16(n64_covers[i].flags);
		host_covers[i].unk1a = srctodst16(n64_covers[i].unk1a);
	}

	dstpos += num_covers * sizeof(struct coverdefinition);

	return dstpos;
}

static int convert_binary(uint8_t *dst, uint8_t *src)
{
	int dstpos = 0;
	struct n64_header *n64_header = (struct n64_header *) src;
	struct host_header *host_header = (struct host_header *) dst;

	int num_pads = srctoh32(n64_header->num_pads);
	int num_covers = srctoh32(n64_header->num_covers);
	host_header->num_pads = htodst32(num_pads);
	host_header->num_covers = htodst32(num_covers);

	dstpos += sizeof(struct host_header);

	// Pads
	dstpos = convert_pads(dst, dstpos, src, sizeof(struct n64_header), num_pads);

	// Waypoints
	host_header->ptr_waypoints = htodst32(dstpos);
	dstpos = convert_waypoints(dst, dstpos, src, srctoh32(n64_header->ptr_waypoints));

	// Waygroups
	host_header->ptr_waygroups = htodst32(dstpos);
	dstpos = convert_waygroups(dst, dstpos, src, srctoh32(n64_header->ptr_waygroups));

	// Cover
	host_header->ptr_cover = htodst32(dstpos);
	dstpos = convert_cover(dst, dstpos, src, srctoh32(n64_header->ptr_cover), num_covers);

	return dstpos;
}

void extract_file_pads(char *filename, uint32_t romoffset, size_t len)
{
	// Unzip it
	size_t infsize = rzip_get_infsize(&g_Rom[romoffset]);
	uint8_t *src = malloc(infsize);

	int ret = rzip_inflate(src, infsize, &g_Rom[romoffset], len);

	if (ret < 0) {
		fprintf(stderr, "%s: Unable to inflate file: %d\n", filename, ret);
		exit(EXIT_FAILURE);
	}

	// Convert it
	uint8_t *dst = calloc(1, infsize * 2);
	int dstpos = convert_binary(dst, src);

	dstpos = ALIGN16(dstpos);

	// Zip it
	uint8_t *zipped = calloc(1, dstpos);
	size_t ziplen = dstpos;

	ret = rzip_deflate(zipped, &ziplen, dst, dstpos);

	if (ret < 0) {
		fprintf(stderr, "%s: Unable to compress file: %d\n", filename, ret);
		exit(EXIT_FAILURE);
	}

	// Write it
	char outfilename[1024];
	sprintf(outfilename, "%s/files/%s", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(zipped, ALIGN16(ziplen), 1, fp);
	fclose(fp);

	free(src);
	free(dst);
	free(zipped);
}
