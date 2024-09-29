#include <string.h>
#include "types.h"
#include "platform.h"
#include "system.h"
#include "net/netbuf.h"

/* read */

void netbufStartRead(struct netbuf *buf)
{
	buf->error = 0;
	buf->wp = buf->size;
	buf->rp = 0;
}

void netbufStartReadData(struct netbuf *buf, const void *data, u32 size)
{
	buf->size = size;
	buf->data = (void *)data;
	netbufStartRead(buf);
}

s32 netbufReadLeft(const struct netbuf *buf)
{
	return buf->wp - buf->rp;
}

static inline u32 netbufCanRead(struct netbuf *buf, const u32 num)
{
	if (buf->error || buf->rp + num > buf->wp) {
		sysLogPrintf(LOG_ERROR, "NET: could not read %u bytes", num);
		__builtin_trap();
		buf->error = 1;
		return false;
	}
	return true;
}

u32 netbufReadData(struct netbuf *buf, void *out, u32 num)
{
	if (netbufCanRead(buf, num)) {
		memcpy(out, buf->data + buf->rp, num);
		buf->rp += num;
		return num;
	}
	return 0;
}

u8 netbufReadU8(struct netbuf *buf)
{
	if (netbufCanRead(buf, 1)) {
		return buf->data[buf->rp++];
	}
	return 0;
}

u16 netbufReadU16(struct netbuf *buf)
{
	if (netbufCanRead(buf, 2)) {
		const u16 ret = *(u16 *)&buf->data[buf->rp];
		buf->rp += sizeof(ret);
		return PD_LE16(ret);
	}
	return 0;
}

u32 netbufReadU32(struct netbuf *buf)
{
	if (netbufCanRead(buf, 4)) {
		const u32 ret = *(u32 *)&buf->data[buf->rp];
		buf->rp += sizeof(ret);
		return PD_LE32(ret);
	}
	return 0;
}

u64 netbufReadU64(struct netbuf *buf)
{
	if (netbufCanRead(buf, 8)) {
		const u64 ret = *(u64 *)&buf->data[buf->rp];
		buf->rp += sizeof(ret);
		return PD_LE64(ret);
	}
	return 0;
}

s8 netbufReadS8(struct netbuf *buf) {
	return (s8)netbufReadU8(buf);
}

s16 netbufReadS16(struct netbuf *buf) {
	return (s16)netbufReadU16(buf);
}

s32 netbufReadS32(struct netbuf *buf) {
	return (s32)netbufReadU32(buf);
}

s64 netbufReadS64(struct netbuf *buf) {
	return (s64)netbufReadU64(buf);
}

f32 netbufReadF32(struct netbuf *buf)
{
	union {
		f32 f;
		u32 i;
	} hack;
	hack.i = netbufReadU32(buf);
	return hack.f;
}

char *netbufReadStr(struct netbuf *buf)
{
	const u16 len = netbufReadU16(buf);
	if (netbufCanRead(buf, len)) {
		char *ret = (char *)&buf->data[buf->rp];
		buf->rp += len;
		return ret;
	}
	return NULL;
}

u32 netbufReadCoord(struct netbuf *buf, struct coord *out)
{
	if (netbufCanRead(buf, sizeof(*out))) {
		out->x = netbufReadF32(buf);
		out->y = netbufReadF32(buf);
		out->z = netbufReadF32(buf);
		return sizeof(*out);
	}
	return 0;
}

u32 netbufReadMtxf(struct netbuf *buf, Mtxf *out)
{
	if (netbufCanRead(buf, sizeof(*out))) {
		for (s32 i = 0; i < 4; ++i) {
			for (s32 j = 0; j < 4; ++j) {
				out->m[i][j] = netbufReadF32(buf);
			}
		}
		return sizeof(*out);
	}
	return 0;
}

u32 netbufReadSkip(struct netbuf *buf, u32 count)
{
	if (netbufCanRead(buf, count)) {
		buf->rp += count;
		return count;
	}
	return 0;
}

/* write */

void netbufStartWrite(struct netbuf *buf)
{
	buf->wp = 0;
	buf->error = 0;
}

static inline u32 netbufCanWrite(struct netbuf *buf, const u32 num)
{
	if (buf->error || buf->wp + num > buf->size) {
		buf->error = 1;
		return false;
	}
	return true;
}

s32 netbufWriteLeft(const struct netbuf *buf)
{
	return buf->size - buf->wp;
}

u32 netbufWriteData(struct netbuf *buf, const void *data, u32 num)
{
	if (netbufCanWrite(buf, num)) {
		memcpy(buf->data + buf->wp, data, num);
		buf->wp += num;
		return num;
	}
	return 0;
}

u32 netbufWriteU8(struct netbuf *buf, const u8 v)
{
	if (netbufCanWrite(buf, sizeof(v))) {
		buf->data[buf->wp++] = v;
		return sizeof(v);
	}
	return 0;
}

u32 netbufWriteU16(struct netbuf *buf, const u16 v)
{
	if (netbufCanWrite(buf, sizeof(v))) {
		*(u16 *)&buf->data[buf->wp] = PD_LE16(v);
		buf->wp += sizeof(v);
		return sizeof(v);
	}
	return 0;
}

u32 netbufWriteU32(struct netbuf *buf, const u32 v)
{
	if (netbufCanWrite(buf, sizeof(v))) {
		*(u32 *)&buf->data[buf->wp] = PD_LE32(v);
		buf->wp += sizeof(v);
		return sizeof(v);
	}
	return 0;
}

u32 netbufWriteU64(struct netbuf *buf, const u64 v)
{
	if (netbufCanWrite(buf, sizeof(v))) {
		*(u64 *)&buf->data[buf->wp] = PD_LE64(v);
		buf->wp += sizeof(v);
		return sizeof(v);
	}
	return 0;
}

u32 netbufWriteS8(struct netbuf *buf, const s8 v)
{
	return netbufWriteU8(buf, (u8)v);
}

u32 netbufWriteS16(struct netbuf *buf, const s16 v)
{
	return netbufWriteU16(buf, (u16)v);
}

u32 netbufWriteS32(struct netbuf *buf, const s32 v)
{
	return netbufWriteU32(buf, (u32)v);
}

u32 netbufWriteS64(struct netbuf *buf, const s64 v)
{
	return netbufWriteU64(buf, (u64)v);
}

u32 netbufWriteF32(struct netbuf *buf, const f32 v)
{
	const union {
		f32 f;
		u32 i;
	} hack = { .f = v };
	return netbufWriteU32(buf, hack.i);
}

u32 netbufWriteStr(struct netbuf *buf, const char *v)
{
	if (!v) {
		return 0;
	}

	u32 len = strlen(v) + 1;
	if (len > 0xFFFF) {
		len = 0xFFFF;
	}

	if (netbufCanWrite(buf, len + sizeof(u16))) {
		netbufWriteU16(buf, len);
		netbufWriteData(buf, v, len);
		buf->data[buf->wp - 1] = 0;
		return sizeof(u16) + len;
	}

	return 0;
}

u32 netbufWriteCoord(struct netbuf *buf, const struct coord *v)
{
	if (netbufCanWrite(buf, sizeof(*v))) {
		netbufWriteF32(buf, v->x);
		netbufWriteF32(buf, v->y);
		netbufWriteF32(buf, v->z);
		return sizeof(*v);
	}
	return 0;
}

u32 netbufWriteMtxf(struct netbuf *buf, const Mtxf *in)
{
	if (netbufCanWrite(buf, sizeof(*in))) {
		for (s32 i = 0; i < 4; ++i) {
			for (s32 j = 0; j < 4; ++j) {
				netbufWriteF32(buf, in->m[i][j]);
			}
		}
		return sizeof(*in);
	}
	return 0;
}

void netbufReset(struct netbuf *buf)
{
	memset(buf, 0, sizeof(*buf));
}
