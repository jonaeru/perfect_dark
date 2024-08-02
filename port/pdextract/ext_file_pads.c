#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "romdata.h"
#include "system.h"

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
	s32 num_pads;
	s32 num_covers;
	u32 ptr_waypoints;
	u32 ptr_waygroups;
	u32 ptr_cover;
};

struct host_header {
	s32 num_pads;
	s32 num_covers;
	uintptr_t ptr_waypoints;
	uintptr_t ptr_waygroups;
	uintptr_t ptr_cover;
};

struct padheader {
	u32 flags : 18;
	u32 room : 10;
	u32 liftnum : 4;
};

struct n64_waypoint {
	s32 padnum;
	u32 ptr_neighbours;
	s32 groupnum;
	s32 step;
};

struct host_waypoint {
	s32 padnum;
	uintptr_t ptr_neighbours;
	s32 groupnum;
	s32 step;
};

struct n64_waygroup {
	u32 ptr_neighbours;
	u32 ptr_waypoints;
	s32 step;
};

struct host_waygroup {
	uintptr_t ptr_neighbours;
	uintptr_t ptr_waypoints;
	s32 step;
};

struct coverdefinition {
	u32 pos[3];
	u32 dir[3];
	u16 flags;
	u16 unk1a;
};

static int convert_pads(u8 *dst, int dstpos, u8 *src, int srcpos, int num_pads)
{
	u16 *src_offsets = (u16 *) &src[srcpos];
	u16 *dst_offsets = (u16 *) &dst[dstpos];

	dstpos += num_pads * sizeof(u16);

	for (int i = 0; i < num_pads; i++) {
		dstpos = ALIGN4(dstpos);
		srcpos = srctoh16(src_offsets[i]);

		dst_offsets[i] = htodst16(dstpos);

		// Header
		u32 n64_padheader = srctoh32(*(u32 *) &src[srcpos]);
		struct padheader *host_padheader = (struct padheader *) &dst[dstpos];
		u32 flags = (n64_padheader >> 14) & 0x3ffff;

		*(u32*)host_padheader = htodst32(n64_padheader);

		srcpos += sizeof(struct padheader);
		dstpos += sizeof(struct padheader);

		// Position
		if (flags & PADFLAG_INTPOS) {
			s16 *srcptr = (s16 *) &src[srcpos];
			s16 *dstptr = (s16 *) &dst[dstpos];

			dstptr[0] = srctodst16(srcptr[0]);
			dstptr[1] = srctodst16(srcptr[1]);
			dstptr[2] = srctodst16(srcptr[2]);

			srcpos += 8;
			dstpos += 8;
		} else {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Up
		if ((flags & (PADFLAG_UPALIGNTOX | PADFLAG_UPALIGNTOY | PADFLAG_UPALIGNTOZ)) == 0) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Look
		if ((flags & (PADFLAG_LOOKALIGNTOX | PADFLAG_LOOKALIGNTOY | PADFLAG_LOOKALIGNTOZ)) == 0) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

			dstptr[0] = srctodst32(srcptr[0]);
			dstptr[1] = srctodst32(srcptr[1]);
			dstptr[2] = srctodst32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Bbox
		if (flags & PADFLAG_HASBBOXDATA) {
			u32 *srcptr = (u32 *) &src[srcpos];
			u32 *dstptr = (u32 *) &dst[dstpos];

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

static int convert_waypoints(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_waypoint *n64_waypoints = (struct n64_waypoint *) &src[srcpos];
	struct host_waypoint *host_waypoints = (struct host_waypoint *) &dst[dstpos];
	int num_waypoints;

	for (num_waypoints = 0; n64_waypoints[num_waypoints].padnum != -1; num_waypoints++);

	dstpos += (num_waypoints + 1) * sizeof(struct host_waypoint);

	u32 *host_neighbours = (u32 *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waypoints; i++) {
		host_waypoints[i].padnum = srctodst32(n64_waypoints[i].padnum);
		host_waypoints[i].ptr_neighbours = htodst32(dstpos);
		host_waypoints[i].groupnum = srctodst32(n64_waypoints[i].groupnum);
		host_waypoints[i].step = 0;

		u32 *n64_neighbours = (u32 *) &src[srctoh32(n64_waypoints[i].ptr_neighbours)];

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

static int convert_waygroups(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_waygroup *n64_waygroups = (struct n64_waygroup *) &src[srcpos];
	struct host_waygroup *host_waygroups = (struct host_waygroup *) &dst[dstpos];
	int num_waygroups;

	for (num_waygroups = 0; n64_waygroups[num_waygroups].ptr_neighbours != 0; num_waygroups++);

	dstpos += (num_waygroups + 1) * sizeof(struct host_waygroup);

	// Waygroups and child waypoints
	u32 *host_waypoints = (u32 *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].ptr_waypoints = htodst32(dstpos);
		host_waygroups[i].step = 0;

		u32 *n64_waypoints = (u32 *) &src[srctoh32(n64_waygroups[i].ptr_waypoints)];

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
	u32 *host_neighbours = (u32 *) &dst[dstpos];
	n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].ptr_neighbours = htodst32(dstpos);

		u32 *n64_neighbours = (u32 *) &src[srctoh32(n64_waygroups[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = srctodst32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	return dstpos;
}

static int convert_cover(u8 *dst, int dstpos, u8 *src, int srcpos, int num_covers)
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

static int convert_pads_file(u8 *dst, u8 *src)
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

u8* preprocessPadsFile_x64(u8 *data, u32 size, u32 *outSize) {
	u32 newSizeEstimated = romdataGetEstimatedFileSize(size, FT_PADS);
	u8* dst = sysMemZeroAlloc(newSizeEstimated);

	u32 newSize = convert_pads_file(dst, data);

	if (newSize > newSizeEstimated) {
		sysLogPrintf(LOG_ERROR, "overflow when trying to preprocess a pads file, size %d newsize %d", size, newSize);
		exit(EXIT_FAILURE);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
