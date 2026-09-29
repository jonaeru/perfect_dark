#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

void texInit(void)
{
	extern u8 EXT_SEG _textureslistSegmentRomStart;
	extern u8 EXT_SEG _textureslistSegmentRomEnd;

	u32 len = ((REF_SEG _textureslistSegmentRomEnd - REF_SEG _textureslistSegmentRomStart) + 15) & -16;

#ifndef PLATFORM_N64
	u32 extendedLen = MAX_TEXTURES * sizeof(struct texture);
	struct texture *extendedTextures = mempAlloc(extendedLen, MEMPOOL_PERMANENT);

	dmaExec(extendedTextures, (romptr_t) REF_SEG _textureslistSegmentRomStart, len);

	// Initialize with default values ​​for extended textures
	u32 romTextureCount = len / sizeof(struct texture);
	for (u32 i = romTextureCount; i < MAX_TEXTURES; i++) {
		extendedTextures[i].surfacetype = SURFACETYPE_DEFAULT;
		extendedTextures[i].soundsurfacetype = SURFACETYPE_DEFAULT;
		extendedTextures[i].dataoffset = 0;
		extendedTextures[i].unk04_00 = 0;
		extendedTextures[i].unk04_04 = 0;
		extendedTextures[i].unk04_08 = 0;
		extendedTextures[i].unk04_0c = 0;
	}

	g_Textures = extendedTextures;
#else
	g_Textures = mempAlloc(len, MEMPOOL_PERMANENT);

	dmaExec(g_Textures, (romptr_t) REF_SEG _textureslistSegmentRomStart, len);
#endif
}
