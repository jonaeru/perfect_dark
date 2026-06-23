#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

#include <string.h>

void texInit(void)
{
	extern u8 EXT_SEG _textureslistSegmentRomStart;
	extern u8 EXT_SEG _textureslistSegmentRomEnd;

	u32 len = ((REF_SEG _textureslistSegmentRomEnd - REF_SEG _textureslistSegmentRomStart) + 15) & -16;
	u32 alloc_len = MAX_TEXTURES * sizeof(struct texture);

	g_Textures = mempAlloc(alloc_len, MEMPOOL_PERMANENT);
	memset(g_Textures, 0, alloc_len);

	dmaExec(g_Textures, (romptr_t) REF_SEG _textureslistSegmentRomStart, len);
}
