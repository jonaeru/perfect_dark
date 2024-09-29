#ifndef _IN_NETBUF_H
#define _IN_NETBUF_H

#include "types.h"

struct netbuf {
	u8 *data;
	u32 size;
	u32 rp;
	u32 wp;
	u32 error;
};

void netbufStartRead(struct netbuf *buf);
void netbufStartReadData(struct netbuf *buf, const void *data, u32 size);
s32 netbufReadLeft(const struct netbuf *buf);
u32 netbufReadData(struct netbuf *buf, void *out, u32 num);
u8 netbufReadU8(struct netbuf *buf);
u16 netbufReadU16(struct netbuf *buf);
u32 netbufReadU32(struct netbuf *buf);
u64 netbufReadU64(struct netbuf *buf);
s8 netbufReadS8(struct netbuf *buf);
s16 netbufReadS16(struct netbuf *buf);
s32 netbufReadS32(struct netbuf *buf);
s64 netbufReadS64(struct netbuf *buf);
f32 netbufReadF32(struct netbuf *buf);
char *netbufReadStr(struct netbuf *buf);
u32 netbufReadCoord(struct netbuf *buf, struct coord *out);
u32 netbufReadMtxf(struct netbuf *buf, Mtxf *out);
u32 netbufReadSkip(struct netbuf *buf, u32 count);

void netbufStartWrite(struct netbuf *buf);
s32 netbufWriteLeft(const struct netbuf *buf);
u32 netbufWriteData(struct netbuf *buf, const void *data, u32 num);
u32 netbufWriteU8(struct netbuf *buf, const u8 v);
u32 netbufWriteU16(struct netbuf *buf, const u16 v);
u32 netbufWriteU32(struct netbuf *buf, const u32 v);
u32 netbufWriteU64(struct netbuf *buf, const u64 v);
u32 netbufWriteS8(struct netbuf *buf, const s8 v);
u32 netbufWriteS16(struct netbuf *buf, const s16 v);
u32 netbufWriteS32(struct netbuf *buf, const s32 v);
u32 netbufWriteS64(struct netbuf *buf, const s64 v);
u32 netbufWriteF32(struct netbuf *buf, const f32 v);
u32 netbufWriteStr(struct netbuf *buf, const char *v);
u32 netbufWriteCoord(struct netbuf *buf, const struct coord *v);
u32 netbufWriteMtxf(struct netbuf *buf, const Mtxf *in);

void netbufReset(struct netbuf *buf);

#endif
