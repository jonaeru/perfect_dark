#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

enum contenttype {
	/* 0*/ CT_MODELDEF,
	/* 1*/ CT_NODE,
	/* 2*/ CT_TEXCONFIG,
	/* 3*/ CT_TEXDATA,
	/* 4*/ CT_PARTS,
	/* 5*/ CT_VTXCOL,
	/* 6*/ CT_GDL,
	/* 7*/ CT_RODATA_CHRINFO,
	/* 8*/ CT_RODATA_POSITION,
	/* 9*/ CT_RODATA_GUNDL,
	/*10*/ CT_RODATA_DISTANCE,
	/*11*/ CT_RODATA_REORDER,
	/*12*/ CT_RODATA_BBOX,
	/*13*/ CT_RODATA_CHRGUNFIRE,
	/*14*/ CT_RODATA_11,
	/*15*/ CT_RODATA_TOGGLE,
	/*16*/ CT_RODATA_POSITIONHELD,
	/*17*/ CT_RODATA_STARGUNFIRE,
	/*18*/ CT_RODATA_HEADSPOT,
	/*19*/ CT_RODATA_DL,
	/*20*/ CT_RODATA_19,
};

struct marker {
	uint32_t src_offset;
	uint32_t dst_offset;
	uint32_t parent_src_offset;
	enum contenttype type;
};

struct alignconfig {
	int before;
	int after;
};

struct alignconfig m_AlignConfigs[] = {
	/* 0*/ { 16, 4 },
	/* 1*/ { 4,  4 },
	/* 2*/ { 4,  4 },
	/* 3*/ { 8,  8 },
	/* 4*/ { 4,  4 },
	/* 5*/ { 4,  8 },
	/* 6*/ { 8,  8 },
	/* 7*/ { 4,  4 },
	/* 8*/ { 4,  4 },
	/* 9*/ { 8,  4 },
	/*10*/ { 4,  8 },
	/*11*/ { 4,  4 },
	/*12*/ { 4,  4 },
	/*13*/ { 4,  4 },
	/*14*/ { 4,  4 },
	/*15*/ { 4,  4 },
	/*16*/ { 4,  4 },
	/*17*/ { 8,  8 },
	/*18*/ { 4,  4 },
	/*19*/ { 8,  8 },
	/*20*/ { 4,  4 },
};

struct marker m_Markers[1024];
int m_NumMarkers;

#define MODELNODETYPE_CHRINFO      0x01
#define MODELNODETYPE_POSITION     0x02
#define MODELNODETYPE_GUNDL        0x04
#define MODELNODETYPE_05           0x05
#define MODELNODETYPE_DISTANCE     0x08
#define MODELNODETYPE_REORDER      0x09
#define MODELNODETYPE_BBOX         0x0a
#define MODELNODETYPE_0B           0x0b
#define MODELNODETYPE_CHRGUNFIRE   0x0c
#define MODELNODETYPE_0D           0x0d
#define MODELNODETYPE_0E           0x0e
#define MODELNODETYPE_0F           0x0f
#define MODELNODETYPE_11           0x11
#define MODELNODETYPE_TOGGLE       0x12
#define MODELNODETYPE_POSITIONHELD 0x15
#define MODELNODETYPE_STARGUNFIRE  0x16
#define MODELNODETYPE_HEADSPOT     0x17
#define MODELNODETYPE_DL           0x18
#define MODELNODETYPE_19           0x19
#define MODELNODETYPE_0100         0x0100
#define MODELNODETYPE_0200         0x0200

struct src_modeldef {
	uint32_t ptr_rootnode;
	uint32_t ptr_skel;
	uint32_t ptr_parts;
	int16_t numparts;
	int16_t nummatrices;
	uint32_t scale;
	uint16_t rwdatalen;
	uint16_t numtexconfigs;
	uint32_t ptr_texconfigs;
};

struct dst_modeldef {
	uintptr_t ptr_rootnode;
	uintptr_t ptr_skel;
	uintptr_t ptr_parts;
	int16_t numparts;
	int16_t nummatrices;
	uint32_t scale;
	uint16_t rwdatalen;
	uint16_t numtexconfigs;
	uintptr_t ptr_texconfigs;
};

struct src_modelnode {
	uint16_t type;
	uint32_t ptr_rodata;
	uint32_t ptr_parent;
	uint32_t ptr_next;
	uint32_t ptr_prev;
	uint32_t ptr_child;
};

struct dst_modelnode {
	uint16_t type;
	uintptr_t ptr_rodata;
	uintptr_t ptr_parent;
	uintptr_t ptr_next;
	uintptr_t ptr_prev;
	uintptr_t ptr_child;
};

struct generic_rodata_chrinfo {
	uint16_t animpart;
	int16_t mtxindex;
	uint32_t unk04;
	uint16_t rwdataindex;
};

struct generic_rodata_position {
	uint32_t pos[3];
	uint16_t part;
	int16_t mtxindexes[3];
	uint32_t drawdist;
};

struct src_rodata_gundl {
	uint32_t ptr_opagdl;
	uint32_t ptr_xlugdl;
	uint32_t ptr_baseaddr;
	uint32_t ptr_vertices;
	int16_t numvertices;
	int16_t unk12;
};

struct dst_rodata_gundl {
	uintptr_t ptr_opagdl;
	uintptr_t ptr_xlugdl;
	uintptr_t ptr_baseaddr;
	uintptr_t ptr_vertices;
	int16_t numvertices;
	int16_t unk12;
};

struct src_rodata_distance {
	uint32_t near;
	uint32_t far;
	uint32_t ptr_target;
	uint16_t rwdataindex;
};

struct dst_rodata_distance {
	uint32_t near;
	uint32_t far;
	uintptr_t ptr_target;
	uint16_t rwdataindex;
};

struct src_rodata_reorder {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c[3];
	uint32_t ptr_node_unk18;
	uint32_t ptr_node_unk1c;
	int16_t side;
	uint16_t rwdataindex;
};

struct dst_rodata_reorder {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c[3];
	uintptr_t ptr_node_unk18;
	uintptr_t ptr_node_unk1c;
	int16_t side;
	uint16_t rwdataindex;
};

struct generic_rodata_bbox {
	int32_t hitpart;
	uint32_t bbox[6];
};

struct src_rodata_chrgunfire {
	uint32_t pos[3];
	uint32_t dim[3];
	uint32_t ptr_texture;
	uint32_t unk1c;
	uint16_t rwdataindex;
	uint32_t ptr_baseaddr;
};

struct dst_rodata_chrgunfire {
	uint32_t pos[3];
	uint32_t dim[3];
	uintptr_t ptr_texture;
	uint32_t unk1c;
	uint16_t rwdataindex;
	uintptr_t ptr_baseaddr;
};

struct src_rodata_type11 {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	uint32_t unk10;
	uint32_t ptr_unk14;
};

struct dst_rodata_type11 {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	uint32_t unk10;
	uintptr_t ptr_unk14;
};

struct src_rodata_toggle {
	uint32_t ptr_target;
	uint16_t rwdataindex;
};

struct dst_rodata_toggle {
	uintptr_t ptr_target;
	uint16_t rwdataindex;
};

struct generic_rodata_positionheld {
	uint32_t pos[3];
	int16_t mtxindex;
	uint32_t unk10;
};

struct src_rodata_stargunfire {
	uint32_t unk00;
	uint32_t ptr_vertices;
	uint32_t ptr_gdl;
	uint32_t ptr_baseaddr;
};

struct dst_rodata_stargunfire {
	uint32_t unk00;
	uintptr_t ptr_vertices;
	uintptr_t ptr_gdl;
	uintptr_t ptr_baseaddr;
};

struct generic_rodata_headspot {
	uint16_t rwdataindex;
};

struct src_rodata_dl {
	uint32_t ptr_opagdl;
	uint32_t ptr_xlugdl;
	uint32_t ptr_colours;
	uint32_t ptr_vertices; // colours follow this array
	int16_t numvertices;
	int16_t mcount;
	uint16_t rwdataindex;
	uint16_t numcolours;
};

struct dst_rodata_dl {
	uintptr_t ptr_opagdl;
	uintptr_t ptr_xlugdl;
	uintptr_t ptr_colours;
	uintptr_t ptr_vertices; // colours follow this array
	int16_t numvertices;
	int16_t mcount;
	uint16_t rwdataindex;
	uint16_t numcolours;
};

struct generic_rodata_type19 {
	uint32_t numvertices;
	uint32_t vertices[3];
};

struct src_texconfig {
	uint32_t ptr;
	uint8_t width;
	uint8_t height;
	uint8_t level;
	uint8_t format;
	uint8_t depth;
	uint8_t s;
	uint8_t t;
	uint8_t unk0b;
};

struct dst_texconfig {
	uintptr_t ptr;
	uint8_t width;
	uint8_t height;
	uint8_t level;
	uint8_t format;
	uint8_t depth;
	uint8_t s;
	uint8_t t;
	uint8_t unk0b;
};

static struct marker *find_marker(uint32_t src_offset)
{
	for (int i = 0; i < m_NumMarkers; i++) {
		if (m_Markers[i].src_offset == src_offset) {
			return &m_Markers[i];
		}
	}

	return NULL;
}

static void set_marker(uint32_t src_offset, enum contenttype type, uint32_t parent_src_offset)
{
	if (src_offset == 0) {
		return;
	}

	src_offset &= 0x00ffffff;

	if (find_marker(src_offset)) {
		return;
	}

	if (m_NumMarkers >= ARRAYCOUNT(m_Markers)) {
		fprintf(stderr, "Marker limit exceeded\n");
		exit(EXIT_FAILURE);
	}

	m_Markers[m_NumMarkers].src_offset = src_offset;
	m_Markers[m_NumMarkers].type = type;
	m_Markers[m_NumMarkers].parent_src_offset = parent_src_offset;
	m_NumMarkers++;
}

enum contenttype m_NodeTypeToContentType[] = {
	/*0x00*/ -1,
	/*0x01*/ CT_RODATA_CHRINFO,
	/*0x02*/ CT_RODATA_POSITION,
	/*0x03*/ -1,
	/*0x04*/ CT_RODATA_GUNDL,
	/*0x05*/ -1,
	/*0x06*/ -1,
	/*0x07*/ -1,
	/*0x08*/ CT_RODATA_DISTANCE,
	/*0x09*/ CT_RODATA_REORDER,
	/*0x0a*/ CT_RODATA_BBOX,
	/*0x0b*/ -1,
	/*0x0c*/ CT_RODATA_CHRGUNFIRE,
	/*0x0d*/ -1,
	/*0x0e*/ -1,
	/*0x0f*/ -1,
	/*0x10*/ -1,
	/*0x11*/ CT_RODATA_11,
	/*0x12*/ CT_RODATA_TOGGLE,
	/*0x13*/ -1,
	/*0x14*/ -1,
	/*0x15*/ CT_RODATA_POSITIONHELD,
	/*0x16*/ CT_RODATA_STARGUNFIRE,
	/*0x17*/ CT_RODATA_HEADSPOT,
	/*0x18*/ CT_RODATA_DL,
	/*0x19*/ CT_RODATA_19,
};

static void populate_markers(uint8_t *src)
{
	m_NumMarkers = 0;

	set_marker(0x05000000, CT_MODELDEF, 0);

	for (int i = 0; i < m_NumMarkers; i++) {
		struct marker *marker = &m_Markers[i];
		void *src_thing = &src[marker->src_offset];

		switch (marker->type) {
		case CT_MODELDEF:
			struct src_modeldef *src_modeldef = src_thing;
			int num_texconfigs = srctoh16(src_modeldef->numtexconfigs);
			uint32_t texconfigpos = srctoh32(src_modeldef->ptr_texconfigs);
			set_marker(srctoh32(src_modeldef->ptr_rootnode), CT_NODE, marker->src_offset);
			set_marker(srctoh32(src_modeldef->ptr_parts), CT_PARTS, marker->src_offset);

			for (int i = 0; i < num_texconfigs; i++) {
				set_marker(texconfigpos + sizeof(struct src_texconfig) * i, CT_TEXCONFIG, marker->src_offset);
			}
			break;
		case CT_NODE:
			struct src_modelnode *src_node = src_thing;
			uint32_t node_type = srctoh16(src_node->type) & 0xff;
			set_marker(srctoh32(src_node->ptr_rodata), m_NodeTypeToContentType[node_type], marker->src_offset);
			set_marker(srctoh32(src_node->ptr_parent), CT_NODE, marker->src_offset);
			set_marker(srctoh32(src_node->ptr_next), CT_NODE, marker->src_offset);
			set_marker(srctoh32(src_node->ptr_prev), CT_NODE, marker->src_offset);
			set_marker(srctoh32(src_node->ptr_child), CT_NODE, marker->src_offset);
			break;
		case CT_TEXCONFIG:
			struct src_texconfig *src_texconfig = src_thing;
			if ((srctoh32(src_texconfig->ptr) & 0xff000000) == 0x05000000) {
				set_marker(srctoh32(src_texconfig->ptr), CT_TEXDATA, marker->src_offset);
			}
			break;
		case CT_PARTS:
			struct src_modeldef *src_modeldef2 = (struct src_modeldef *) src;
			uint32_t *src_parts = (uint32_t *) src_thing;
			int num_parts = srctoh16(src_modeldef2->numparts);

			for (int i = 0; i < num_parts; i++) {
				set_marker(srctoh32(src_parts[i]), CT_NODE, marker->src_offset);
			}
			break;
		case CT_RODATA_CHRINFO:
			break;
		case CT_RODATA_POSITION:
			break;
		case CT_RODATA_GUNDL:
			struct src_rodata_gundl *src_gundl = src_thing;
			set_marker(srctoh32(src_gundl->ptr_opagdl), CT_GDL, marker->src_offset);
			set_marker(srctoh32(src_gundl->ptr_xlugdl), CT_GDL, marker->src_offset);
			set_marker(srctoh32(src_gundl->ptr_vertices), CT_VTXCOL, marker->src_offset);
			break;
		case CT_RODATA_DISTANCE:
			struct src_rodata_distance *src_dist = src_thing;
			set_marker(srctoh32(src_dist->ptr_target), CT_NODE, marker->src_offset);
			break;
		case CT_RODATA_REORDER:
			struct src_rodata_reorder *src_reorder = src_thing;
			set_marker(srctoh32(src_reorder->ptr_node_unk18), CT_NODE, marker->src_offset);
			set_marker(srctoh32(src_reorder->ptr_node_unk1c), CT_NODE, marker->src_offset);
			break;
		case CT_RODATA_CHRGUNFIRE:
			struct src_rodata_chrgunfire *src_chrgunfire = src_thing;
			set_marker(srctoh32(src_chrgunfire->ptr_texture), CT_TEXCONFIG, marker->src_offset);
			break;
		case CT_RODATA_TOGGLE:
			struct src_rodata_toggle *src_toggle = src_thing;
			set_marker(srctoh32(src_toggle->ptr_target), CT_NODE, marker->src_offset);
			break;
		case CT_RODATA_STARGUNFIRE:
			struct src_rodata_stargunfire *src_stargunfire = src_thing;
			set_marker(srctoh32(src_stargunfire->ptr_vertices), CT_VTXCOL, marker->src_offset);
			set_marker(srctoh32(src_stargunfire->ptr_gdl), CT_GDL, marker->src_offset);
			break;
		case CT_RODATA_DL:
			struct src_rodata_dl *src_dl = src_thing;
			set_marker(srctoh32(src_dl->ptr_opagdl), CT_GDL, marker->src_offset);
			set_marker(srctoh32(src_dl->ptr_xlugdl), CT_GDL, marker->src_offset);
			set_marker(srctoh32(src_dl->ptr_colours), CT_VTXCOL, marker->src_offset);
			set_marker(srctoh32(src_dl->ptr_vertices), CT_VTXCOL, marker->src_offset);
			break;
		case CT_TEXDATA:
		case CT_VTXCOL:
		case CT_GDL:
		case CT_RODATA_BBOX:
		case CT_RODATA_11:
		case CT_RODATA_POSITIONHELD:
		case CT_RODATA_HEADSPOT:
		case CT_RODATA_19:
			break;
		}
	}
}

static void sort_markers(void)
{
	for (int i = 0; i < m_NumMarkers - 1; i++) {
		uint32_t min_offset = 0xffffffff;
		int min_index = -1;

		for (int j = i + 1; j < m_NumMarkers; j++) {
			if (m_Markers[j].src_offset < min_offset) {
				min_offset = m_Markers[j].src_offset;
				min_index = j;
			}
		}

		if (min_index >= 0 && m_Markers[min_index].src_offset < m_Markers[i].src_offset) {
			struct marker tmp = m_Markers[i];
			m_Markers[i] = m_Markers[min_index];
			m_Markers[min_index] = tmp;
		}
	}
}

static uint32_t convert_content(uint8_t *dst, uint8_t *src, uint32_t src_file_len)
{
	uint32_t dstpos = 0;

	for (int i = 0; i < m_NumMarkers; i++) {
		struct marker *marker = &m_Markers[i];
		void *src_thing = &src[marker->src_offset];
		uint32_t src_end = i < m_NumMarkers - 1 ? m_Markers[i + 1].src_offset : src_file_len;
		uint32_t src_len = src_end - marker->src_offset;

		dstpos = ALIGN(dstpos, m_AlignConfigs[marker->type].before);

		marker->dst_offset = dstpos;
		void *dst_thing = &dst[dstpos];

		switch (marker->type) {
		case CT_MODELDEF:
			struct src_modeldef *src_modeldef = src_thing;
			struct dst_modeldef *dst_modeldef = dst_thing;
			dst_modeldef->ptr_skel = srctodst32(src_modeldef->ptr_skel);
			dst_modeldef->numparts = srctodst16(src_modeldef->numparts);
			dst_modeldef->nummatrices = srctodst16(src_modeldef->nummatrices);
			dst_modeldef->scale = srctodst32(src_modeldef->scale);
			dst_modeldef->rwdatalen = srctodst16(src_modeldef->rwdatalen);
			dst_modeldef->numtexconfigs = srctodst16(src_modeldef->numtexconfigs);
			dstpos += sizeof(*dst_modeldef);
			break;
		case CT_NODE:
			struct src_modelnode *src_node = src_thing;
			struct dst_modelnode *dst_node = dst_thing;
			dst_node->type = srctodst16(src_node->type);
			dstpos += sizeof(*dst_node);
			break;
		case CT_TEXCONFIG:
			struct src_texconfig *src_texconfig = src_thing;
			struct dst_texconfig *dst_texconfig = dst_thing;

			while (src_len >= sizeof(*src_texconfig)) {
				if ((srctoh32(src_texconfig->ptr) & 0xff000000) != 0x05000000) {
					dst_texconfig->ptr = srctodst32(src_texconfig->ptr);
				}

				dst_texconfig->width = src_texconfig->width;
				dst_texconfig->height = src_texconfig->height;
				dst_texconfig->level = src_texconfig->level;
				dst_texconfig->format = src_texconfig->format;
				dst_texconfig->depth = src_texconfig->depth;
				dst_texconfig->s = src_texconfig->s;
				dst_texconfig->t = src_texconfig->t;
				dst_texconfig->unk0b = src_texconfig->unk0b;

				dstpos += sizeof(*dst_texconfig);
				src_len -= sizeof(*src_texconfig);
				src_texconfig++;
				dst_texconfig++;
			}
			break;
		case CT_TEXDATA:
			memcpy(dst_thing, src_thing, src_len);
			dstpos += src_len;
			break;
		case CT_PARTS:
			struct src_modeldef *src_modeldef2 = (struct src_modeldef *) src;
			uint32_t *src_parts = (uint32_t *) src_thing;
			uintptr_t *dst_parts = (uintptr_t *) dst_thing;
			int num_parts = srctoh16(src_modeldef2->numparts);
			uint16_t *src_nums = (uint16_t *) &src_parts[num_parts];
			uint16_t *dst_nums = (uint16_t *) &dst_parts[num_parts];

			for (int i = 0; i < num_parts; i++) {
				dst_nums[i] = srctodst16(src_nums[i]);
			}
			dstpos += num_parts * sizeof(uintptr_t) + num_parts * sizeof(uint16_t);
			if (num_parts) {
				dstpos = ALIGN8(dstpos);
			}
			break;
		case CT_VTXCOL:
			memcpy(dst_thing, src_thing, src_len);
			dstpos += src_len;
			break;
		case CT_GDL:
			struct marker *parent = find_marker(marker->parent_src_offset);
			uint32_t src_vtx;
			uint32_t dst_vtx;

			if (parent->type == CT_RODATA_GUNDL) {
				struct src_rodata_gundl *src_rodata = (struct src_rodata_gundl *) &src[parent->src_offset];
				struct dst_rodata_gundl *dst_rodata = (struct dst_rodata_gundl *) &dst[parent->dst_offset];
				src_vtx = srctoh32(src_rodata->ptr_vertices) & 0x00ffffff;
				dst_vtx = dsttoh32(dst_rodata->ptr_vertices) & 0x00ffffff;
			} else if (parent->type == CT_RODATA_STARGUNFIRE) {
				struct src_rodata_stargunfire *src_rodata = (struct src_rodata_stargunfire *) &src[parent->src_offset];
				struct dst_rodata_stargunfire *dst_rodata = (struct dst_rodata_stargunfire *) &dst[parent->dst_offset];
				src_vtx = srctoh32(src_rodata->ptr_vertices) & 0x00ffffff;
				dst_vtx = dsttoh32(dst_rodata->ptr_vertices) & 0x00ffffff;
			} else if (parent->type == CT_RODATA_DL) {
				struct src_rodata_dl *src_rodata = (struct src_rodata_dl *) &src[parent->src_offset];
				struct dst_rodata_dl *dst_rodata = (struct dst_rodata_dl *) &dst[parent->dst_offset];
				src_vtx = srctoh32(src_rodata->ptr_vertices) & 0x00ffffff;
				dst_vtx = dsttoh32(dst_rodata->ptr_vertices) & 0x00ffffff;
			}

			gbi_set_vtx(src_vtx, dst_vtx);
			gbi_set_segment(0x04, dst_vtx);
			gbi_set_segment(0x05, 0);
			dstpos = gbi_convert_gdl(dst, dstpos, src, marker->src_offset);
			break;
		case CT_RODATA_CHRINFO:
			struct generic_rodata_chrinfo *src_chrinfo = src_thing;
			struct generic_rodata_chrinfo *dst_chrinfo = dst_thing;
			dst_chrinfo->animpart = srctodst16(src_chrinfo->animpart);
			dst_chrinfo->mtxindex = srctodst16(src_chrinfo->mtxindex);
			dst_chrinfo->unk04 = srctodst32(src_chrinfo->unk04);
			dst_chrinfo->rwdataindex = srctodst16(src_chrinfo->rwdataindex);
			dstpos += sizeof(*dst_chrinfo);
			break;
		case CT_RODATA_POSITION:
			struct generic_rodata_position *src_position = src_thing;
			struct generic_rodata_position *dst_position = dst_thing;
			dst_position->pos[0] = srctodst32(src_position->pos[0]);
			dst_position->pos[1] = srctodst32(src_position->pos[1]);
			dst_position->pos[2] = srctodst32(src_position->pos[2]);
			dst_position->part = srctodst16(src_position->part);
			dst_position->mtxindexes[0] = srctodst16(src_position->mtxindexes[0]);
			dst_position->mtxindexes[1] = srctodst16(src_position->mtxindexes[1]);
			dst_position->mtxindexes[2] = srctodst16(src_position->mtxindexes[2]);
			dst_position->drawdist = srctodst32(src_position->drawdist);
			dstpos += sizeof(*dst_position);
			break;
		case CT_RODATA_GUNDL:
			struct src_rodata_gundl *src_gundl = src_thing;
			struct dst_rodata_gundl *dst_gundl = dst_thing;
			dst_gundl->ptr_vertices = 0;
			dst_gundl->numvertices = srctodst16(src_gundl->numvertices);
			dst_gundl->unk12 = srctodst16(src_gundl->unk12);
			dstpos += sizeof(*dst_gundl);
			break;
		case CT_RODATA_DISTANCE:
			struct src_rodata_distance *src_dist = src_thing;
			struct dst_rodata_distance *dst_dist = dst_thing;
			dst_dist->near = srctodst32(src_dist->near);
			dst_dist->far = srctodst32(src_dist->far);
			dst_dist->rwdataindex = srctodst16(src_dist->rwdataindex);
			dstpos += sizeof(*dst_dist);
			break;
		case CT_RODATA_REORDER:
			struct src_rodata_reorder *src_reorder = src_thing;
			struct dst_rodata_reorder *dst_reorder = dst_thing;
			dst_reorder->unk00 = srctodst32(src_reorder->unk00);
			dst_reorder->unk04 = srctodst32(src_reorder->unk04);
			dst_reorder->unk08 = srctodst32(src_reorder->unk08);
			dst_reorder->unk0c[0] = srctodst32(src_reorder->unk0c[0]);
			dst_reorder->unk0c[1] = srctodst32(src_reorder->unk0c[1]);
			dst_reorder->unk0c[2] = srctodst32(src_reorder->unk0c[2]);
			dst_reorder->side = srctodst16(src_reorder->side);
			dst_reorder->rwdataindex = srctodst16(src_reorder->rwdataindex);
			dstpos += sizeof(*dst_reorder);
			break;
		case CT_RODATA_BBOX:
			struct generic_rodata_bbox *src_bbox = src_thing;
			struct generic_rodata_bbox *dst_bbox = dst_thing;
			dst_bbox->hitpart = srctodst32(src_bbox->hitpart);
			dst_bbox->bbox[0] = srctodst32(src_bbox->bbox[0]);
			dst_bbox->bbox[1] = srctodst32(src_bbox->bbox[1]);
			dst_bbox->bbox[2] = srctodst32(src_bbox->bbox[2]);
			dst_bbox->bbox[3] = srctodst32(src_bbox->bbox[3]);
			dst_bbox->bbox[4] = srctodst32(src_bbox->bbox[4]);
			dst_bbox->bbox[5] = srctodst32(src_bbox->bbox[5]);
			dstpos += sizeof(*dst_bbox);
			break;
		case CT_RODATA_CHRGUNFIRE:
			struct src_rodata_chrgunfire *src_chrgunfire = src_thing;
			struct dst_rodata_chrgunfire *dst_chrgunfire = dst_thing;
			dst_chrgunfire->pos[0] = srctodst32(src_chrgunfire->pos[0]);
			dst_chrgunfire->pos[1] = srctodst32(src_chrgunfire->pos[1]);
			dst_chrgunfire->pos[2] = srctodst32(src_chrgunfire->pos[2]);
			dst_chrgunfire->dim[0] = srctodst32(src_chrgunfire->dim[0]);
			dst_chrgunfire->dim[1] = srctodst32(src_chrgunfire->dim[1]);
			dst_chrgunfire->dim[2] = srctodst32(src_chrgunfire->dim[2]);
			dst_chrgunfire->unk1c = srctodst32(src_chrgunfire->unk1c);
			dst_chrgunfire->rwdataindex = srctodst16(src_chrgunfire->rwdataindex);
			dst_chrgunfire->ptr_baseaddr = 0;
			dstpos += sizeof(*dst_chrgunfire);
			break;
		case CT_RODATA_11:
			struct src_rodata_type11 *src_type11 = src_thing;
			struct dst_rodata_type11 *dst_type11 = dst_thing;
			dst_type11->unk00 = srctodst32(src_type11->unk00);
			dst_type11->unk04 = srctodst32(src_type11->unk04);
			dst_type11->unk08 = srctodst32(src_type11->unk08);
			dst_type11->unk0c = srctodst32(src_type11->unk0c);
			dst_type11->unk10 = srctodst32(src_type11->unk10);
			dst_type11->ptr_unk14 = srctodst32(src_type11->ptr_unk14); // Not read by game engine
			dstpos += sizeof(*dst_type11);
			break;
		case CT_RODATA_TOGGLE:
			struct src_rodata_toggle *src_toggle = src_thing;
			struct dst_rodata_toggle *dst_toggle = dst_thing;
			dst_toggle->rwdataindex = srctodst16(src_toggle->rwdataindex);
			dstpos += sizeof(*dst_toggle);
			break;
		case CT_RODATA_POSITIONHELD:
			struct generic_rodata_positionheld *src_posheld = src_thing;
			struct generic_rodata_positionheld *dst_posheld = dst_thing;
			dst_posheld->pos[0] = srctodst32(src_posheld->pos[0]);
			dst_posheld->pos[1] = srctodst32(src_posheld->pos[1]);
			dst_posheld->pos[2] = srctodst32(src_posheld->pos[2]);
			dst_posheld->mtxindex = srctodst16(src_posheld->mtxindex);
			dst_posheld->unk10 = srctodst32(src_posheld->unk10);
			dstpos += sizeof(*dst_posheld);
			break;
		case CT_RODATA_STARGUNFIRE:
			struct src_rodata_stargunfire *src_stargunfire = src_thing;
			struct dst_rodata_stargunfire *dst_stargunfire = dst_thing;
			dst_stargunfire->unk00 = srctodst32(src_stargunfire->unk00);
			dstpos += sizeof(*dst_stargunfire);
			break;
		case CT_RODATA_HEADSPOT:
			struct generic_rodata_headspot *src_headspot = src_thing;
			struct generic_rodata_headspot *dst_headspot = dst_thing;
			dst_headspot->rwdataindex = srctodst16(src_headspot->rwdataindex);
			dstpos += sizeof(*dst_headspot);
			break;
		case CT_RODATA_DL:
			struct src_rodata_dl *src_dl = src_thing;
			struct dst_rodata_dl *dst_dl = dst_thing;
			dst_dl->numvertices = srctodst16(src_dl->numvertices);
			dst_dl->mcount = srctodst16(src_dl->mcount);
			dst_dl->rwdataindex = srctodst16(src_dl->rwdataindex);
			dst_dl->numcolours = srctodst16(src_dl->numcolours);
			dstpos += sizeof(*dst_dl);
			break;
		case CT_RODATA_19:
			struct generic_rodata_type19 *src_type19 = src_thing;
			struct generic_rodata_type19 *dst_type19 = dst_thing;
			uint32_t *src_vertices = src_thing + sizeof(uint32_t);
			uint32_t *dst_vertices = dst_thing + sizeof(uint32_t);
			int num_vertices = srctoh32(src_type19->numvertices);

			dst_type19->numvertices = htodst32(num_vertices);
			dstpos += sizeof(uint32_t);

			for (int i = 0; i < num_vertices; i++) {
				dst_vertices[i * 3 + 0] = srctodst32(src_vertices[i * 3 + 0]);
				dst_vertices[i * 3 + 1] = srctodst32(src_vertices[i * 3 + 1]);
				dst_vertices[i * 3 + 2] = srctodst32(src_vertices[i * 3 + 2]);
				dstpos += sizeof(uint32_t) * 3;
			}
			break;
		}

		dstpos = ALIGN(dstpos, m_AlignConfigs[marker->type].after);
	}

	return dstpos;
}

static uint32_t resolve_pointer(uint32_t src_offset)
{
	if (src_offset == 0) {
		return 0;
	}

	struct marker *marker = find_marker(src_offset & 0x00ffffff);
	return marker ? (0x05000000 | marker->dst_offset) : 0;
}

static void relink_pointers(uint8_t *dst, uint8_t *src)
{
	for (int i = 0; i < m_NumMarkers; i++) {
		struct marker *marker = &m_Markers[i];
		void *src_thing = &src[marker->src_offset];
		void *dst_thing = &dst[marker->dst_offset];

		switch (marker->type) {
		case CT_MODELDEF:
			struct src_modeldef *src_modeldef = src_thing;
			struct dst_modeldef *dst_modeldef = dst_thing;
			dst_modeldef->ptr_rootnode = htodst32(resolve_pointer(srctoh32(src_modeldef->ptr_rootnode)));
			dst_modeldef->ptr_parts = htodst32(resolve_pointer(srctoh32(src_modeldef->ptr_parts)));
			dst_modeldef->ptr_texconfigs = htodst32(resolve_pointer(srctoh32(src_modeldef->ptr_texconfigs)));
			break;
		case CT_NODE:
			struct src_modelnode *src_node = src_thing;
			struct dst_modelnode *dst_node = dst_thing;
			dst_node->ptr_rodata = htodst32(resolve_pointer(srctoh32(src_node->ptr_rodata)));
			dst_node->ptr_parent = htodst32(resolve_pointer(srctoh32(src_node->ptr_parent)));
			dst_node->ptr_next = htodst32(resolve_pointer(srctoh32(src_node->ptr_next)));
			dst_node->ptr_prev = htodst32(resolve_pointer(srctoh32(src_node->ptr_prev)));
			dst_node->ptr_child = htodst32(resolve_pointer(srctoh32(src_node->ptr_child)));
			break;
		case CT_TEXCONFIG:
			struct src_texconfig *src_texconfig = src_thing;
			struct dst_texconfig *dst_texconfig = dst_thing;
			if ((srctoh32(src_texconfig->ptr) & 0xff000000) == 0x05000000) {
				dst_texconfig->ptr = htodst32(resolve_pointer(srctoh32(src_texconfig->ptr)));
			}
			break;
		case CT_PARTS:
			struct src_modeldef *src_modeldef2 = (struct src_modeldef *) src;
			uint32_t *src_parts = (uint32_t *) src_thing;
			uintptr_t *dst_parts = (uintptr_t *) dst_thing;
			int num_parts = srctoh16(src_modeldef2->numparts);

			for (int i = 0; i < num_parts; i++) {
				dst_parts[i] = htodst32(resolve_pointer(srctoh32(src_parts[i])));
			}
			break;
		case CT_RODATA_CHRINFO:
			break;
		case CT_RODATA_POSITION:
			break;
		case CT_RODATA_GUNDL:
			struct src_rodata_gundl *src_gundl = src_thing;
			struct dst_rodata_gundl *dst_gundl = dst_thing;
			dst_gundl->ptr_opagdl = htodst32(resolve_pointer(srctoh32(src_gundl->ptr_opagdl)));
			dst_gundl->ptr_xlugdl = htodst32(resolve_pointer(srctoh32(src_gundl->ptr_xlugdl)));
			dst_gundl->ptr_vertices = htodst32(resolve_pointer(srctoh32(src_gundl->ptr_vertices)));
			break;
		case CT_RODATA_DISTANCE:
			struct src_rodata_distance *src_dist = src_thing;
			struct dst_rodata_distance *dst_dist = dst_thing;
			dst_dist->ptr_target = htodst32(resolve_pointer(srctoh32(src_dist->ptr_target)));
			break;
		case CT_RODATA_REORDER:
			struct src_rodata_reorder *src_reorder = src_thing;
			struct dst_rodata_reorder *dst_reorder = dst_thing;
			dst_reorder->ptr_node_unk18 = htodst32(resolve_pointer(srctoh32(src_reorder->ptr_node_unk18)));
			dst_reorder->ptr_node_unk1c = htodst32(resolve_pointer(srctoh32(src_reorder->ptr_node_unk1c)));
			break;
		case CT_RODATA_CHRGUNFIRE:
			struct src_rodata_chrgunfire *src_chrgunfire = src_thing;
			struct dst_rodata_chrgunfire *dst_chrgunfire = dst_thing;
			dst_chrgunfire->ptr_texture = htodst32(resolve_pointer(srctoh32(src_chrgunfire->ptr_texture)));
			break;
		case CT_RODATA_TOGGLE:
			struct src_rodata_toggle *src_toggle = src_thing;
			struct dst_rodata_toggle *dst_toggle = dst_thing;
			dst_toggle->ptr_target = htodst32(resolve_pointer(srctoh32(src_toggle->ptr_target)));
			break;
		case CT_RODATA_STARGUNFIRE:
			struct src_rodata_stargunfire *src_stargunfire = src_thing;
			struct dst_rodata_stargunfire *dst_stargunfire = dst_thing;
			dst_stargunfire->ptr_vertices = htodst32(resolve_pointer(srctoh32(src_stargunfire->ptr_vertices)));
			dst_stargunfire->ptr_gdl = htodst32(resolve_pointer(srctoh32(src_stargunfire->ptr_gdl)));
			break;
		case CT_RODATA_DL:
			struct src_rodata_dl *src_dl = src_thing;
			struct dst_rodata_dl *dst_dl = dst_thing;
			dst_dl->ptr_opagdl = htodst32(resolve_pointer(srctoh32(src_dl->ptr_opagdl)));
			dst_dl->ptr_xlugdl = htodst32(resolve_pointer(srctoh32(src_dl->ptr_xlugdl)));
			dst_dl->ptr_colours = htodst32(resolve_pointer(srctoh32(src_dl->ptr_colours)));
			dst_dl->ptr_vertices = htodst32(resolve_pointer(srctoh32(src_dl->ptr_vertices)));
			break;
		case CT_TEXDATA:
		case CT_VTXCOL:
		case CT_GDL:
		case CT_RODATA_BBOX:
		case CT_RODATA_11:
		case CT_RODATA_POSITIONHELD:
		case CT_RODATA_HEADSPOT:
		case CT_RODATA_19:
			break;
		}
	}
}

int calc_align(uint32_t value)
{
	if ((value % 16) == 0) {
		return 16;
	} else if ((value % 8) == 0) {
		return 8;
	} else if ((value % 4) == 0) {
		return 4;
	} else if ((value % 2) == 0) {
		return 2;
	} else {
		return 1;
	}
}

static int convert_model(uint8_t *dst, uint8_t *src, uint32_t srclen)
{
	uint32_t dstpos;

	populate_markers(src);
	sort_markers();

	dstpos = convert_content(dst, src, srclen);
	relink_pointers(dst, src);

	return dstpos;
}

void extract_file_model(char *filename, uint32_t romoffset, size_t len)
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
	uint8_t *dst = calloc(1, infsize * 4);

	int dstpos = convert_model(dst, src, infsize);

	if (dstpos > infsize * 2) {
		fprintf(stderr, "%s: overflow\n", filename);
		exit(EXIT_FAILURE);
	}

	// Zip it
	uint8_t *zipped = calloc(1, dstpos);
	size_t ziplen = dstpos;

	ret = rzip_deflate(zipped, &ziplen, dst, dstpos);

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
