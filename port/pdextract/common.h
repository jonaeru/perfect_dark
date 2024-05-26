#ifndef _COMMON_H
#define _COMMON_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define srctoh16(value) be16toh(value)
#define srctoh32(value) be32toh(value)
#define srctoh64(value) be64toh(value)

#ifdef FORCE_BE32
#define htodst16(value) htobe16(value)
#define htodst32(value) htobe32(value)
#define htodst64(value) htobe64(value)
#define dsttoh16(value) be16toh(value)
#define dsttoh32(value) be32toh(value)
#define dsttoh64(value) be64toh(value)
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

enum romversion {
	ROMVERSION_NTSC10,
	ROMVERSION_NTSCFINAL,
	ROMVERSION_PALFINAL,
	ROMVERSION_JPNFINAL,
	ROMVERSION_NTSCBETA,
	ROMVERSION_PALBETA,
	ROMVERSION_UNKNOWN,
};

extern uint8_t *g_Rom;
extern enum romversion g_RomVersion;
extern char g_OutPath[1024];

typedef uint32_t n64ptr_t;

#define ALIGN(val, size) ((val + (size - 1)) & ~(size - 1))
#define ALIGN16(val) ((val + 0xf) & ~0xf)
#define ALIGN8(val) ((val + 0x7) & ~0x7)
#define ALIGN4(val) ((val + 0x3) & ~0x3)

#define ARRAYCOUNT(arr) (int) (sizeof(arr) / sizeof(arr[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

FILE *openfile(char *filename);
size_t rzip_get_infsize(uint8_t *src);
int rzip_inflate(uint8_t *dst, size_t dstlen, uint8_t *src, size_t srclen);
int rzip_deflate(uint8_t *dst, size_t *dstlen, uint8_t *src, size_t srclen);

void gbi_reset(void);
void gbi_set_segment(int segment, uint32_t offset);
void gbi_set_vtx(uint32_t src_offset, uint32_t dst_offset);
uint32_t gbi_convert_gdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos);

#endif
