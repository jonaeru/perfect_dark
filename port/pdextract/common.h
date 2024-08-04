#ifndef _PDE_COMMON_H
#define _PDE_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>
// this BS comes from windows.h
#undef near
#undef far

#include <sys/param.h>

#include <PR/ultratypes.h>

#include "constants.h"


#if __BIG_ENDIAN__
	#define htonll(x)   (x)
	#define ntohll(x)   (x)
#else
	#define htonll(x)   ((((uint64_t)htonl(x&0xFFFFFFFF)) << 32) + htonl(x >> 32))
	#define ntohll(x)   ((((uint64_t)ntohl(x&0xFFFFFFFF)) << 32) + ntohl(x >> 32))
#endif

#define htobe16(x) htons(x)
#define htole16(x) (x)
#define be16toh(x) ntohs(x)
#define le16toh(x) (x)

#define htobe32(x) htonl(x)
#define htole32(x) (x)
#define be32toh(x) ntohl(x)
#define le32toh(x) (x)

#define htobe64(x) htonll(x)
#define htole64(x) (x)
#define be64toh(x) ntohll(x)
#define le64toh(x) (x)


#define srctoh16(value) ntohs(value)
#define srctoh32(value) ntohl(value)
#define srctoh64(value) ntohll(value)

#ifdef FORCE_BE32
#define htodst16(value) htons(value)
#define htodst32(value) htonl(value)
#define htodst64(value) htonll(value)
#define dsttoh16(value) ntohs(value)
#define dsttoh32(value) ntohl(value)
#define dsttoh64(value) ntohll(value)
#else
#define htodst16(value) (value)
#define htodst32(value) (value)
#define htodst64(value) (value)
#define dsttoh16(value) (value)
#define dsttoh32(value) (value)
#define dsttoh64(value) (value)
#endif

#define srctodst16(value) htodst16(srctoh16(value))
#define srctodst32(value) htodst32(srctoh32(value))
#define srctodst64(value) htodst64(srctoh64(value))

#ifdef FORCE_BE32
#define uintptr_t uint32_t
#endif

void gbi_reset(void);
void gbi_set_segment(int segment, uint32_t offset);
void gbi_set_vtx(uint32_t src_offset, uint32_t dst_offset);
uint32_t gbi_convert_gdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int segment_cmds);
void gbi_convert_vtx(uint8_t* dst, uint32_t dstpos, int count);
void gbi_gdl_rewrite_addrs(uint8_t* dst, uint32_t offset);
void gbi_add_tex_addr(uint32_t src_offset, uint32_t dst_offset);

struct ptrmarker {
	u32 ptr_src;
	uintptr_t ptr_host;
};

void add_marker(u32 ptr_src, uintptr_t ptr_host);
struct ptrmarker* find_ptr_marker(uintptr_t ptr_src);
void reset_markers();

#endif
