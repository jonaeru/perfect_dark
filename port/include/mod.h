#ifndef _IN_MOD_H
#define _IN_MOD_H

#include <PR/ultratypes.h>

#define MOD_CONFIG_FNAME "modconfig.txt"

// Sentinel meaning "leave this field unchanged" for modTexOverrideSet().
#define MOD_TEX_KEEP (-1)

struct animtableentry;

s32 modConfigLoad(const char *path);

// Applies a surfacetype/soundsurfacetype override to g_Textures[texnum],
// recording the original values the first time each field is touched so they
// can be restored later. Pass MOD_TEX_KEEP for a field to leave it unchanged.
void modTexOverrideSet(u16 texnum, s16 surfacetype, s16 soundsurfacetype);

// Restores every g_Textures field previously changed via modTexOverrideSet()
// back to its original value and clears the recorded overrides.
void modTexOverrideRestore(void);

s32 modTextureLoad(u16 num, void *dst, u32 dstSize);

s32 modAnimationLoadDescriptor(u16 num, struct animtableentry *anim);
void *modAnimationLoadData(u16 num);

void *modSequenceLoad(u16 num, u32 *outSize);

#endif
