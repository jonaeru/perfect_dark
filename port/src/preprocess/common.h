#ifndef _PDE_COMMON_H
#define _PDE_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <winsock2.h>
// this BS comes from windows.h
#undef near
#undef far

#include <sys/param.h>

#include <PR/ultratypes.h>

#include "types.h"
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
#define uintptr_t u32
#endif

#define ALIGN(val, size)	((val + (size - 1)) & ~(size - 1))

#define PD_PTR_BASE(x, b) (void *)((u8 *)b + (uintptr_t)x)
#define PD_PTR_BASEOFS(x, b, d) (void *)((u8 *)b - d + (uintptr_t)x)


static inline f32 swapF32(f32 x) { *(u32*)&x = PD_BE32(*(u32*)&x); return x; }
static inline u32 swapU32(u32 x) { return PD_BE32(x); }
static inline s32 swapS32(s32 x) { return PD_BE32(x); }
static inline u16 swapU16(u16 x) { return PD_BE16(x); }
static inline s16 swapS16(s16 x) { return PD_BE16(x); }
static inline void* swapPtr(void** x) { return (void*)PD_BEPTR((uintptr_t)x); }
static inline struct coord swapCrd(struct coord crd) { crd.x = swapF32(crd.x); crd.y = swapF32(crd.y); crd.z = swapF32(crd.z); return crd; }
static inline u32 swapUnk(u32 x) { assert(0 && "unknown type"); return x; }

#define PD_SWAP_VAL(x) x = _Generic((x), \
	f32: swapF32, \
	u32: swapU32, \
	s32: swapS32, \
	u16: swapU16, \
	s16: swapS16, \
	struct coord: swapCrd, \
	default: swapUnk	\
)(x)

#define PD_SWAP_PTR(x) x = swapPtr((void *)(x))

// gbi related functions
void gbiReset(void);
void gbiSetSegment(int segment, u32 offset);
void gbiSetVtx(u32 src_offset, u32 dst_offset);
u32 gbiConvertGdl(u8 *dst, u32 dstpos, u8 *src, u32 srcpos, int segment_cmds);
void gbiConvertVtx(u8* dst, u32 dstpos, int count);
void gbiGdlRewriteAddrs(u8* dst, u32 offset);
void gbiAddTexAddr(u32 src_offset, u32 dst_offset);

// ptr marker functions
struct ptrmarker {
	u32 ptr_src;
	uintptr_t ptr_host;
};

void addMarker(u32 ptr_src, uintptr_t ptr_host);
struct ptrmarker* findPtrMarker(uintptr_t ptr_src);
void resetMarkers();

#endif
