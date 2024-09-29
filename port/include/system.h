#ifndef _IN_SYSTEM_H
#define _IN_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <PR/ultratypes.h>

#define LOGFLAG_NOCON (1 << 6)
#define LOGFLAG_SHOWMSG (1 << 7)

enum loglevel {
  LOG_NOTE = 0,
  LOG_CHAT = 0 | LOGFLAG_SHOWMSG,
  LOG_WARNING = 1 | LOGFLAG_SHOWMSG,
  LOG_ERROR = 2 | LOGFLAG_SHOWMSG,
};

void sysInitArgs(s32 argc, const char **argv);
void sysInit(void);

s32 sysArgCheck(const char *arg);
const char *sysArgGetString(const char *arg);
s32 sysArgGetInt(const char *arg, s32 defval);

u64 sysGetMicroseconds(void);
f32 sysGetSeconds(void);

void sysFatalError(const char *fmt, ...) __attribute__((noreturn));

s32 sysLogIsOpen(void);
void sysLogPrintf(s32 level, const char *fmt, ...);

void sysGetExecutablePath(char *outPath, const u32 outLen);
void sysGetHomePath(char *outPath, const u32 outLen);

void *sysMemAlloc(const u32 size);
void *sysMemZeroAlloc(const u32 size);
void sysMemFree(void *ptr);

// hns is specified in 100ns units
void sysSleep(const s64 hns);

// yield CPU if supported (e.g. during a busy loop)
void sysCpuRelax(void);

void crashInit(void);
void crashShutdown(void);

#ifdef __cplusplus
}
#endif

#endif
