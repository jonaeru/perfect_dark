#include <ultra64.h>
#include "data.h"
#include "types.h"

Gfx gdl000[] = {
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2),
	gsDPSetTextureLOD(G_TL_TILE),
	gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
	gsSPTexture(0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_16b, 1, 0xabcd002a),
	gsDPLoadSync(),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1567, 0),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 7, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD),
	gsDPSetTileSize(G_TX_RENDERTILE, 0, 0, 0x00dc, 0x00dc),
	gsDPPipeSync(),
	gsSPEndDisplayList(),
};

Gfx gdl070[] = {
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_2CYCLE),
	gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_CLD_SURF2),
	gsDPSetTextureLOD(G_TL_TILE),
	gsDPSetCombineMode(G_CC_INTERFERENCE, G_CC_MODULATEIA2),
	gsSPTexture(0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0188, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 7, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_CLAMP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, 6, G_TX_NOLOD),
	gsDPSetTileSize(G_TX_RENDERTILE, 0, 0, 0x00dc, 0x00dc),
	gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 4, 0x0188, 1, 0, G_TX_NOMIRROR | G_TX_CLAMP, 4, 2, G_TX_NOMIRROR | G_TX_CLAMP, 4, 2),
	gsDPSetTileSize(1, 0, 0, 0x00dc, 0x00dc),
	gsSPEndDisplayList(),
};

Gfx gdl0d8[] = {
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
	gsDPSetTextureLOD(G_TL_TILE),
	gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
	gsSPTexture(0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_16b, 1, 0xabcd0002),
	gsDPLoadSync(),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 511, 0),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 4, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD),
	gsDPSetTileSize(G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c),
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPEndDisplayList(),
};

u32 g_ExplosionTextureNums[] = {
	0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
};

struct textureconfig g_TextureConfigs[] = {
	{ 0x0003, 48,  48,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c27, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0da5, 64,  48,  5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0003, 48,  48,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0003, 48,  48,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0003, 48,  48,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0004, 32,  32,  5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0005, 54,  54,  3, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c28, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0854, 48,  48,  5, G_IM_FMT_IA,   G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0855, 48,  48,  5, G_IM_FMT_IA,   G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0856, 48,  48,  5, G_IM_FMT_IA,   G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x08f0, 24,  24,  5, G_IM_FMT_IA,   G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b53, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b53, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b53, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0d74, 32,  24,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0d72, 32,  24,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0006, 16,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0007, 16,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0008, 16,  32,  1, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0859, 16,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x085a, 16,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0009, 16,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x000a, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x000b, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c31, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x000c, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x000d, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c32, 32,  1,   5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x000e, 54,  54,  3, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x000f, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c33, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c34, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c35, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c36, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c37, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c38, 128, 16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c39, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3b, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0010, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3c, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3d, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3e, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c3f, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c40, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c41, 16,  16,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c42, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c43, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c45, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c46, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c47, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c48, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c49, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4b, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4c, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4d, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4e, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0011, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0012, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c4f, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c50, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c51, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c52, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c53, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c54, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c55, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c56, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c57, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c58, 32,  32,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c59, 32,  32,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c5a, 32,  32,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c5b, 32,  32,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c5c, 128, 48,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_CLAMP  },
	{ 0x0c5d, 16,  16,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c5e, 16,  16,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c5f, 16,  16,  5, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c60, 54,  54,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c61, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c62, 32,  32,  5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c63, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c64, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c65, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c66, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c67, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c68, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c69, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6c, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6d, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6b, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6e, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c6f, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c70, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c71, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c72, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c73, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c74, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c75, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c76, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c77, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c78, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c79, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7b, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7c, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7d, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7e, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c7f, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c80, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c81, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c82, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c83, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c84, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c85, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c86, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c87, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c88, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c89, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8b, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8c, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8d, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8e, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c8f, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0013, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c90, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0014, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0001, 32,  28,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_CLAMP  },
	{ 0x0001, 32,  28,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_CLAMP  },
	{ 0x0001, 122, 18,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0001, 15,  15,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0001, 20,  20,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c91, 16,  16,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c92, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0015, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0016, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0c93, 16,  16,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0c94, 32,  128, 0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_WRAP   },
	{ 0x0017, 16,  16,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0018, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0c95, 32,  32,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0019, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0c96, 64,  64,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x001a, 8,   16,  5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x001b, 16,  16,  3, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0001, 1,   1,   0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c97, 16,  16,  3, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x001c, 2,   8,   1, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x001d, 8,   8,   1, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x001c, 2,   8,   1, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x01e5, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c98, 8,   8,   0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c99, 2,   16,  1, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0385, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x063b, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c9a, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x063c, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0385, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0617, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0618, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0619, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061a, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061b, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061c, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061d, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061e, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x061f, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0620, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0621, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0622, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0623, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0624, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0625, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0626, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b4f, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b52, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b50, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0b51, 56,  36,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0858, 14,  14,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x084e, 11,  11,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x08f4, 14,  14,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x060a, 32,  32,  5, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c9b, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0112, 64,  64,  5, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c9c, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c9d, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0c9e, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0c9f, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0ca0, 64,  64,  0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0ca1, 64,  64,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0ca2, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0a9a, 64,  24,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0ca3, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_MIRROR, G_TX_MIRROR },
	{ 0x0b99, 32,  48,  0, G_IM_FMT_I,    G_IM_SIZ_8b,  G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0caf, 32,  32,  0, G_IM_FMT_IA,   G_IM_SIZ_8b,  G_TX_WRAP,   G_TX_WRAP   },
	{ 0x0da0, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0da1, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0da2, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0da3, 32,  32,  0, G_IM_FMT_RGBA, G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
	{ 0x0da4, 4,   4,   0, G_IM_FMT_RGBA, G_IM_SIZ_16b, G_TX_WRAP,   G_TX_WRAP   },
	{ 0x003c, 64,  64,  1, G_IM_FMT_IA,   G_IM_SIZ_32b, G_TX_CLAMP,  G_TX_CLAMP  },
};
