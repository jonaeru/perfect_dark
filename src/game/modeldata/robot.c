#include <ultra64.h>
#include "bss.h"
#include "data.h"
#include "types.h"

u8 g_SkelDrCarollJoints[][2] = {
	{ 0, 0 },
	{ 1, 1 },
	{ 2, 2 },
	{ 3, 3 },
};

struct skeleton g_SkelDrCaroll = {
	SKEL_DRCAROLL, ARRAYCOUNT(g_SkelDrCarollJoints), g_SkelDrCarollJoints,
};

u8 g_Skel22Joints[][2] = {
	{ 0,  0  },
	{ 1,  1  },
	{ 2,  2  },
	{ 3,  3  },
	{ 4,  4  },
	{ 5,  5  },
	{ 6,  6  },
	{ 7,  7  },
	{ 8,  8  },
	{ 9,  9  },
	{ 10, 10 },
	{ 11, 11 },
	{ 12, 12 },
	{ 13, 13 },
	{ 14, 14 },
	{ 15, 15 },
	{ 16, 16 },
	{ 17, 17 },
	{ 18, 19 },
	{ 19, 18 },
	{ 20, 21 },
	{ 21, 20 },
	{ 22, 23 },
	{ 23, 22 },
	{ 24, 25 },
	{ 25, 24 },
	{ 26, 27 },
	{ 27, 26 },
	{ 28, 29 },
	{ 29, 28 },
};

struct skeleton g_Skel22 = {
	SKEL_22, ARRAYCOUNT(g_Skel22Joints), g_Skel22Joints,
};

u8 g_SkelRobotJoints[][2] = {
	{ 0, 0 },
	{ 1, 2 },
	{ 2, 1 },
};

struct skeleton g_SkelRobot = {
	SKEL_ROBOT, ARRAYCOUNT(g_SkelRobotJoints), g_SkelRobotJoints,
};

struct headorbody g_HeadsAndBodies[] = {
	//           ismale
	//           |                                  height
	//           |                                  |
#ifndef PLATFORM_N64 // All in One Mod
	{ /*0x0000*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CDJBOND,          1,    1.0446009635925,  0, FILE_GHAND_BOND      },
	{ /*0x0001*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCONNERY,         1,    1.0300469398499,  0, FILE_GHAND_BONDWHITE },
	{ /*0x0002*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     165, FILE_CDALTON,          1,    1.0572769641876,  0, FILE_GHAND_BOND      },
	{ /*0x0003*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CMOORE,           1,    1.0399061441422,  0, FILE_GHAND_BOND      },
#else
#if VERSION >= VERSION_NTSC_1_0
	{ /*0x0000*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CDJBOND,          1,    1.0446009635925,  0, FILE_GHAND_DDSECURITY      },
	{ /*0x0001*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCONNERY,         1,    1.0300469398499,  0, FILE_GHAND_MRBLONDE        },
	{ /*0x0002*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     165, FILE_CDALTON,          1,    1.0572769641876,  0, FILE_GHAND_DDSECURITY      },
	{ /*0x0003*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CMOORE,           1,    1.0399061441422,  0, FILE_GHAND_DDSECURITY      },
#else
	{ /*0x0000*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     169, FILE_CDJBOND,          1,    1.0446009635925,  0, FILE_GHAND_DDSECURITY      },
	{ /*0x0001*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     169, FILE_CCONNERY,         1,    1.0300469398499,  0, FILE_GHAND_MRBLONDE        },
	{ /*0x0002*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CDALTON,          1,    1.0572769641876,  0, FILE_GHAND_DDSECURITY      },
	{ /*0x0003*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     169, FILE_CMOORE,           1,    1.0399061441422,  0, FILE_GHAND_DDSECURITY      },
#endif
#endif
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0004*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_COMBAT_JP, 1, 1,                0, 0                          },
#else
	{ /*0x0004*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_COMBAT, 1,    1,                0, 0                          },
#endif
	{ /*0x0005*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       27,  FILE_CHEADELVIS,       1,    1,                0, 0                          },
	{ /*0x0006*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADROSS,        1,    1,                0, 0                          },
	{ /*0x0007*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCARRINGTON,  1,    1,                0, 0                          },
	{ /*0x0008*/ 1, 1, 0, HEADBODYTYPE_MRBLONDE,    13,  FILE_CHEADMRBLONDE,    1,    1,                0, 0                          },
	{ /*0x0009*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADTRENT,       1,    1,                0, 0                          },
	{ /*0x000a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDDSHOCK,     1,    1,                0, 0                          },
	{ /*0x000b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRAHAM,      1,    1,                0, 0                          },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x000c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_FROCK_JP, 1,  1,                0, 0                          },
#else
	{ /*0x000c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_FROCK,  1,    1,                0, 0                          },
#endif
	{ /*0x000d*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADSECRETARY,   1,    1,                0, 0                          },
	{ /*0x000e*/ 0, 1, 0, HEADBODYTYPE_CASS,        13,  FILE_CHEADCASSANDRA,   1,    1,                0, 0                          },
	{ /*0x000f*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       27,  FILE_CHEADTHEKING,     1,    1,                0, 0                          },
	{ /*0x0010*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADFEM_GUARD,   1,    1,                0, 0                          },
	{ /*0x0011*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJON,         1,    1,                0, 0                          },
	{ /*0x0012*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMARK2,       1,    1,                0, 0                          },
	{ /*0x0013*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCHRIST,      1,    1,                0, 0                          },
	{ /*0x0014*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADRUSS,        1,    1,                0, 0                          },
#ifdef PLATFORM_N64
	{ /*0x0015*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       13,  FILE_CHEADGREY,        1,    1,                0, 0                          },
#else // All in One Mod
#if VERSION >= VERSION_JPN_FINAL
	{ /*0x0015*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_US,     1,    1,                0, 0                          }, // Joanna (US)
#else
	{ /*0x0015*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADGREY,        1,    1,                0, 0                          }, // Joanna (JP)
#endif
#endif
	{ /*0x0016*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDARLING,     1,    1,                0, 0                          },
	{ /*0x0017*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADROBERT,      1,    1,                0, 0                          },
	{ /*0x0018*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x0019*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADFEM_GUARD2,  1,    1,                0, 0                          },
	{ /*0x001a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBRIAN,       1,    1,                0, 0                          },
	{ /*0x001b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJAMIE,       1,    1,                0, 0                          },
	{ /*0x001c*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDUNCAN2,     1,    1,                0, 0                          },
	{ /*0x001d*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBIOTECH,     1,    1,                0, 0                          },
	{ /*0x001e*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADNEIL2,       1,    1,                0, 0                          },
	{ /*0x001f*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADEDMCG,       1,    1,                0, 0                          },
	{ /*0x0020*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADANKA,        1,    1,                0, 0                          },
	{ /*0x0021*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADLESLIE_S,    1,    1,                0, 0                          },
	{ /*0x0022*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMATT_C,      1,    1,                0, 0                          },
	{ /*0x0023*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPEER_S,      1,    1,                0, 0                          },
	{ /*0x0024*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADEILEEN_T,    1,    1,                0, 0                          },
	{ /*0x0025*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADANDY_R,      1,    1,                0, 0                          },
	{ /*0x0026*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEN_R,       1,    1,                0, 0                          },
	{ /*0x0027*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSTEVE_K,     1,    1,                0, 0                          },
	{ /*0x0028*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJONATHAN,    1,    1,                0, 0                          },
	{ /*0x0029*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       27,  FILE_CHEADMAIAN_S,     1,    1,                0, 0                          },
	{ /*0x002a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSHAUN,       1,    1,                0, 0                          },
	{ /*0x002b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x002c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADEILEEN_H,    1,    1,                0, 0                          },
	{ /*0x002d*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSCOTT_H,     1,    1,                0, 0                          },
	{ /*0x002e*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSANCHEZ,     1,    1,                0, 0                          },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x002f*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARKAQUA_JP, 1,    1,                0, 0                          },
#else
	{ /*0x002f*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARKAQUA,    1,    1,                0, 0                          },
#endif
	{ /*0x0030*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDDSNIPER,    1,    1,                0, 0                          },
	{ /*0x0031*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x0032*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x0033*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x0034*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBEAU,        1,    1,                0, 0                          },
	{ /*0x0035*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRIFFEY,     1,    1,                0, 0                          },
	{ /*0x0036*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMOTO,        1,    1,                0, 0                          },
	{ /*0x0037*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADKEITH,       1,    1,                0, 0                          },
	{ /*0x0038*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADWINNER,      1,    1,                0, 0                          },
	{ /*0x0039*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CA51FACEPLATE,    1,    1,                0, 0                          },
	{ /*0x003a*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       27,  FILE_CHEADELVIS_GOGS,  1,    1,                0, 0                          },
	{ /*0x003b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSTEVEM,      1,    1,                0, 0                          },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x003c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_SNOW_JP, 1,   1,                0, 0                          },
#else
	{ /*0x003c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_SNOW,   1,    1,                0, 0                          },
#endif
	{ /*0x003d*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPRESIDENT,   1,    1,                0, 0                          },
	{ /*0x003e*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEAD_VD,         1,    1,                0, 0                          },
	{ /*0x003f*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADKEN,         1,    1,                0, 0                          },
	{ /*0x0040*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJOEL,        1,    1,                0, 0                          },
	{ /*0x0041*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADTIM,         1,    1,                0, 0                          },
	{ /*0x0042*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRANT,       1,    1,                0, 0                          },
	{ /*0x0043*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPENNY,       1,    1,                0, 0                          },
	{ /*0x0044*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADROBIN,       1,    1,                0, 0                          },
	{ /*0x0045*/ 0, 1, 0, HEADBODYTYPE_FEMALEGUARD, 13,  FILE_CHEADALEX,        1,    1,                0, 0                          },
	{ /*0x0046*/ 0, 1, 0, HEADBODYTYPE_FEMALEGUARD, 13,  FILE_CHEADJULIANNE,    1,    1,                0, 0                          },
	{ /*0x0047*/ 0, 1, 0, HEADBODYTYPE_FEMALEGUARD, 13,  FILE_CHEADLAURA,       1,    1,                0, 0                          },
	{ /*0x0048*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDAVEC,       1,    1,                0, 0                          },
	{ /*0x0049*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCOOK,        1,    1,                0, 0                          },
	{ /*0x004a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPRYCE,       1,    1,                0, 0                          },
	{ /*0x004b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSILKE,       1,    1,                0, 0                          },
	{ /*0x004c*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSMITH,       1,    1,                0, 0                          },
	{ /*0x004d*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGARETH,      1,    1,                0, 0                          },
	{ /*0x004e*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMURCHIE,     1,    1,                0, 0                          },
	{ /*0x004f*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADWONG,        1,    1,                0, 0                          },
	{ /*0x0050*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCARTER,      1,    1,                0, 0                          },
	{ /*0x0051*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADTINTIN,      1,    1,                0, 0                          },
	{ /*0x0052*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMUNTON,      1,    1,                0, 0                          },
	{ /*0x0053*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSTAMPER,     1,    1,                0, 0                          },
	{ /*0x0054*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJONES,       1,    1,                0, 0                          },
	{ /*0x0055*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPHELPS,      1,    1,                0, 0                          },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0056*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_COMBAT_JP,  1,    0.95305162668228, 0, FILE_GCOMBATHANDSLOD       },
#else
	{ /*0x0056*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_COMBAT,     1,    0.95305162668228, 0, FILE_GCOMBATHANDSLOD       },
#endif
	{ /*0x0057*/ 1, 0, 0, HEADBODYTYPE_MAIAN,       106, FILE_CELVIS1,          1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
	{ /*0x0058*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CAREA51GUARD,     1,    0.92769956588745, 0, FILE_GHAND_A51GUARD        },
	{ /*0x0059*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_COVERALL,         1,    0.92769956588745, 0, FILE_GHAND_A51GUARD        },
	{ /*0x005a*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     154, FILE_CCARRINGTON,      1,    0.85915493965149, 0, FILE_GHAND_CARRINGTON      },
#if VERSION >= VERSION_NTSC_1_0
	{ /*0x005b*/ 1, 0, 0, HEADBODYTYPE_MRBLONDE,    169, FILE_CMRBLONDE,        1,    1.1032863855362,  0, FILE_GHAND_MRBLONDE        },
#else
	{ /*0x005b*/ 1, 0, 0, HEADBODYTYPE_MRBLONDE,    175, FILE_CMRBLONDE,        1,    1.1032863855362,  0, FILE_GHAND_MRBLONDE        },
#endif
	{ /*0x005c*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CSKEDAR,          1,    1,                0, 0                          },
	{ /*0x005d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     161, FILE_CTRENT,           1,    0.93896716833115, 0, FILE_GHAND_TRENT           },
	{ /*0x005e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CDDSHOCK,         1,    0.93896716833115, 0, FILE_GHAND_DDFODDER        },
#ifdef PLATFORM_N64
	{ /*0x005f*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CLABTECH,         1,    0.93896716833115, 0, FILE_GHAND_MRBLONDE        },
#else // All in One Mod
	{ /*0x005f*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CLABTECH,         1,    0.93896716833115, 0, FILE_GHAND_LABTECH         },
#endif
	{ /*0x0060*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CSTRIPES,         1,    0.92769956588745, 0, FILE_GHAND_BLACKGUARD      },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0061*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_FROCK_JP,   1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
#else
	{ /*0x0061*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_FROCK,      1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
#endif
	{ /*0x0062*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_TRENCH,     1,    0.95305162668228, 0, FILE_GHAND_JOTRENCH        },
	{ /*0x0063*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_COFFICEWORKER,    1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0064*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_COFFICEWORKER2,   1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
#ifdef PLATFORM_N64
	{ /*0x0065*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      140, FILE_CSECRETARY,       1,    0.87323945760727, 0, FILE_GHAND_JOFROCK         },
#else // All in One Mod
	{ /*0x0065*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      140, FILE_CSECRETARY,       1,    0.87323945760727, 0, FILE_GHAND_NATALYA         },
#endif
#if VERSION >= VERSION_NTSC_1_0
	{ /*0x0066*/ 0, 0, 0, HEADBODYTYPE_CASS,        167, FILE_CCASSANDRA,       1,    0.98591554164886, 0, FILE_GHAND_VRIES           },
#else
	{ /*0x0066*/ 0, 0, 0, HEADBODYTYPE_CASS,        169, FILE_CCASSANDRA,       1,    0.98591554164886, 0, FILE_GHAND_VRIES           },
#endif
	{ /*0x0067*/ 1, 0, 0, HEADBODYTYPE_MAIAN,       106, FILE_CTHEKING,         1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
	{ /*0x0068*/ 0, 0, 1, HEADBODYTYPE_FEMALEGUARD, 160, FILE_CFEM_GUARD,       1,    0.96713620424271, 0, FILE_GHAND_JOTRENCH        },
	{ /*0x0069*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CDD_LABTECH,      1,    0.93896716833115, 0, FILE_GHAND_DDLABTECH       },
	{ /*0x006a*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     160, FILE_CDD_SECGUARD,     1,    0.93427228927612, 0, FILE_GHAND_DDSECURITY      },
	{ /*0x006b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CDRCARROLL,       1,    1,                0, 0                          },
	{ /*0x006c*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CEYESPY,          1,    1,                0, 0                          },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x006d*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_RIPPED_JP,  1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
#else
	{ /*0x006d*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_RIPPED,     1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
#endif
#ifdef PLATFORM_N64
	{ /*0x006e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     160, FILE_CDD_GUARD,        1,    0.93896716833115, 0, FILE_GHAND_DDSECURITY      },
#else // All in One Mod
	{ /*0x006e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     160, FILE_CDD_GUARD,        1,    0.93896716833115, 0, FILE_GHAND_DD_GUARD        },
#endif
	{ /*0x006f*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CDD_SHOCK_INF,    1,    0.93896716833115, 0, FILE_GHAND_DDSHOCK         },
#ifdef PLATFORM_N64
	{ /*0x0070*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CTESTCHR,         1,    1,                0, 0                          },
#else // All in One Mod
	{ /*0x0070*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     144, FILE_CCARROLL2,        1,    1,                0, FILE_GHAND_CAROLL          }, // Dr. Caroll (Sinister)
#endif
	{ /*0x0071*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     155, FILE_CBIOTECH,         1,    0.93896716833115, 0, FILE_GHAND_DDBIO           },
	{ /*0x0072*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CFBIGUY,          1,    0.93896716833115, 0, FILE_GHAND_FBIARM          },
	{ /*0x0073*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCIAGUY,          1,    0.93896716833115, 0, FILE_GHAND_CIA             },
	{ /*0x0074*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CA51TROOPER,      1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0075*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_CA51AIRMAN,       1,    0.93896716833115, 0, FILE_GHAND_A51AIRMAN       },
	{ /*0x0076*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CCHICROB,         1,    1,                0, 0                          },
#ifdef PLATFORM_N64
	{ /*0x0077*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     153, FILE_CSTEWARD,         1,    0.89201879501343, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0078*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      143, FILE_CSTEWARDESS,      1,    0.85446006059647, 0, FILE_GHAND_JOFROCK         },
#else // All in One Mod
	{ /*0x0077*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     153, FILE_CSTEWARD,         1,    0.89201879501343, 0, FILE_GHAND_STEWARD         },
	{ /*0x0078*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      143, FILE_CSTEWARDESS,      1,    0.85446006059647, 0, FILE_GHAND_NATALYA         },
#endif
	{ /*0x0079*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT,       1,    0.93896716833115, 0, FILE_GHAND_PRESIDENT       },
	{ /*0x007a*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      143, FILE_CSTEWARDESS_COAT, 1,    0.85446006059647, 0, FILE_GHAND_STEWARDESS_COAT },
	{ /*0x007b*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CMINISKEDAR,      0.75, 0.5,              0, 0                          },
#ifdef PLATFORM_N64
	{ /*0x007c*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CNSA_LACKEY,      1,    0.93896716833115, 0, FILE_GHAND_CARRINGTON      },
	{ /*0x007d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRES_SECURITY,   1,    0.93896716833115, 0, FILE_GHAND_CARRINGTON      },
	{ /*0x007e*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      142, FILE_CNEGOTIATOR,      1,    0.85446006059647, 0, FILE_GHAND_JOFROCK         },
#else // All in One Mod
	{ /*0x007c*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CNSA_LACKEY,      1,    0.93896716833115, 0, FILE_GHAND_NSA_LACKEY      },
	{ /*0x007d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRES_SECURITY,   1,    0.93896716833115, 0, FILE_GHAND_PRES_SECURITY   },
	{ /*0x007e*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      142, FILE_CNEGOTIATOR,      1,    0.85446006059647, 0, FILE_GHAND_NATALYA         },
#endif
	{ /*0x007f*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CG5_GUARD,        1,    0.93896716833115, 0, FILE_GHAND_G5GUARD         },
	{ /*0x0080*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CPELAGIC_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_TRAGIC_PELAGIC  },
	{ /*0x0081*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CG5_SWAT_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_G5GUARD         },
	{ /*0x0082*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CALASKAN_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_JOSNOW          },
	{ /*0x0083*/ 1, 0, 1, HEADBODYTYPE_MAIAN,       106, FILE_CMAIAN_SOLDIER,   1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
#ifdef PLATFORM_N64
	{ /*0x0084*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT_CLONE, 1,    0.93896716833115, 0, FILE_GCOMBATHANDSLOD       },
	{ /*0x0085*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT_CLONE, 1,    0.93896716833115, 0, FILE_GHAND_PRESIDENT       },
#else // All in One Mod
	{ /*0x0084*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     181, FILE_CSKEDAR2        , 1,    1               , 0, FILE_GHAND_SKEDAR          }, // Skedar
	{ /*0x0085*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT_CLONE, 1,    0.93896716833115, 0, FILE_GHAND_PRESIDENT_CLONE },
#endif
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0086*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_AF1_JP,     1,    0.95305162668228, 0, FILE_GHAND_JOPILOT         },
#else
	{ /*0x0086*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_AF1,        1,    0.95305162668228, 0, FILE_GHAND_JOPILOT         },
#endif
	{ /*0x0087*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKWET,         1,    0.95305162668228, 0, FILE_GHAND_JOWETSUIT       },
	{ /*0x0088*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKAQUALUNG,    1,    0.95305162668228, 0, FILE_GHAND_JOWETSUIT       },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0089*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKSNOW_JP,     1,    0.95305162668228, 0, FILE_GHAND_JOSNOW          },
#else
	{ /*0x0089*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKSNOW,        1,    0.95305162668228, 0, FILE_GHAND_JOSNOW          },
#endif
#ifdef PLATFORM_N64
	{ /*0x008a*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKLAB,         1,    0.95305162668228, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x008b*/ 0, 0, 1, HEADBODYTYPE_FEMALE,      159, FILE_CFEMLABTECH,      1,    0.87323945760727, 0, FILE_GHAND_MRBLONDE        },
#else // All in One Mod
	{ /*0x008a*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKLAB,         1,    0.95305162668228, 0, FILE_GHAND_LABTECH         },
	{ /*0x008b*/ 0, 0, 1, HEADBODYTYPE_FEMALE,      159, FILE_CFEMLABTECH,      1,    0.87323945760727, 0, FILE_GHAND_TECHWOMAN       },
#endif
	{ /*0x008c*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CDDSNIPER,        1,    0.93896716833115, 0, FILE_GHAND_DDSNIPER        },
	{ /*0x008d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPILOTAF1,        1,    0.88262909650803, 0, FILE_GHAND_JOPILOT         },
#ifdef PLATFORM_N64
	{ /*0x008e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCILABTECH,       1,    0.93896716833115, 0, FILE_GHAND_CIFEMTECH       },
#else // All in One Mod
	{ /*0x008e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCILABTECH,       1,    0.93896716833115, 0, FILE_GHAND_CILABTECH       },
#endif
	{ /*0x008f*/ 0, 0, 1, HEADBODYTYPE_FEMALE,      159, FILE_CCIFEMTECH,       1,    0.86854463815689, 0, FILE_GHAND_CIFEMTECH       },
	{ /*0x0090*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CCARREVENINGSUIT, 1,    0.85915493965149, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x0091*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CJONATHON,        1,    0.93896716833115, 0, FILE_GHAND_A51GUARD        },
	{ /*0x0092*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCISOLDIER,       1,    0.93896716833115, 0, FILE_GHAND_CISOLDIER       },
	{ /*0x0093*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CSKEDARKING,      1,    1.25,             0, 0                          },
	{ /*0x0094*/ 1, 0, 0, HEADBODYTYPE_MAIAN,       106, FILE_CELVISWAISTCOAT,  1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
#if (VERSION == VERSION_JPN_FINAL) && !defined(PLATFORM_N64) // All in One Mod
	{ /*0x0095*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_LEATHER_JP,    1, 0.95305162668228, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0096*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_NEGOTIATOR_JP, 1, 0.95305162668228, 0, FILE_GHAND_JOAF1           },
#else
	{ /*0x0095*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_LEATHER,    1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0096*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_NEGOTIATOR, 1,    0.95305162668228, 0, FILE_GHAND_JOAF1           },
#endif
	{ /*0x0097*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     0,   0,                     1,    0,                0, FILE_GCOMBATHANDSLOD       },
#ifndef PLATFORM_N64 // All in One Mod
	{ /*0x0098*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADNATALYA,     1,    1,                0, 0                          }, // Natalya
	{ /*0x0099*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADXENIA,       1,    1,                0, 0                          }, // Xenia
	{ /*0x009a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADTREVELYAN,   1,    1,                0, 0                          }, // Trevelyan
	{ /*0x009b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADOURUMOV,     1,    1,                0, 0                          }, // Ourumov
	{ /*0x009c*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBROSNAN,     1,    1,                0, 0                          }, // Bond (Classic): Brosnan
	{ /*0x009d*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDALTON,      1,    1,                0, 0                          }, // Bond (Classic): Dalton
	{ /*0x009e*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCONNERY,     1,    1,                0, 0                          }, // Bond (Classic): Connery
	{ /*0x009f*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMOORE,       1,    1,                0, 0                          }, // Bond (Classic): Moore
	{ /*0x00a0*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADLAZENBY,     1,    1,                0, 0                          }, // Bond (Classic): Lazenby
	{ /*0x00a1*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJAWS,        1,    1,                0, 0                          }, // Jaws
	{ /*0x00a2*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSNOWGUARD,   1,    1,                0, 0                          }, // Siberian Special Forces
	{ /*0x00a3*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBROSNAN2,    1,    1,                0, 0                          }, // Bond (Arctic)
	{ /*0x00a4*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADODDJOB,      1,    1,                0, 0                          }, // Oddjob
	{ /*0x00a5*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDAVED,       1,    1,                0, 0                          }, // Dr. Doak #1
	{ /*0x00a6*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDAVED2,      1,    1,                0, 0                          }, // Dr. Doak #2
	{ /*0x00a7*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMISHKIN,     1,    1,                0, 0                          }, // Mishkin
	{ /*0x00a8*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBORIS,       1,    1,                0, 0                          }, // Boris
	{ /*0x00a9*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADVALENTIN,    1,    1,                0, 0                          }, // Valentin
	{ /*0x00aa*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMAYDAY,      1,    1,                0, 0                          }, // May Day
	{ /*0x00ab*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBARONSAMEDI, 1,    1,                0, 0                          }, // Baron Samedi
	{ /*0x00ac*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBARONSAMEDI2, 1,   1,                0, 0                          }, // Baron Samedi (Death)
	{ /*0x00ad*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBALACLAVA,   1,    1,                0, 0                          }, // Terrorist
	{ /*0x00ae*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDWAYNE,      1,    1,                0, 0                          }, // Dwayne
	{ /*0x00af*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSANTA,       1,    1,                0, 0                          }, // Santa Claus
	{ /*0x00b0*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADELF,         1,    1,                0, 0                          }, // Elf
	{ /*0x00b1*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPILOT,       1,    1,                0, 0                          }, // Helicopter Pilot
	{ /*0x00b2*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMARION,      1,    1,                0, 0                          }, // Rosika/Marion
	{ /*0x00b3*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADKARL,        1,    1,                0, 0                          }, // Karl Hilton #1
	{ /*0x00b4*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADKARL2,       1,    1,                0, 0                          }, // Karl Hilton #2
	{ /*0x00b5*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMARTIN,      1,    1,                0, 0                          }, // Martin Hollis
	{ /*0x00b6*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMARK0,       1,    1,                0, 0                          }, // Mark Edmonds
	{ /*0x00b7*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDUNCAN0,     1,    1,                0, 0                          }, // Duncan Botwood #1
	{ /*0x00b8*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDUNCAN01,    1,    1,                0, 0                          }, // Duncan Botwood #2
	{ /*0x00b9*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJONES0,      1,    1,                0, 0                          }, // B Jones
	{ /*0x00ba*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRANT0,      1,    1,                0, 0                          }, // Grant Kirkhope
	{ /*0x00bb*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADROBIN0,      1,    1,                0, 0                          }, // Robin Beanland
	{ /*0x00bc*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRAEME,      1,    1,                0, 0                          }, // Graeme Norgate #1
	{ /*0x00bd*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRAEME2,     1,    1,                0, 0                          }, // Graeme Norgate #2
	{ /*0x00be*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSTEVE_E,     1,    1,                0, 0                          }, // Steve Ellis
	{ /*0x00bf*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADKEN0,        1,    1,                0, 0                          }, // Ken Lobb
	{ /*0x00c0*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMANDY,       1,    1,                0, 0                          }, // Mandy '?'
	{ /*0x00c1*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADVIVIEN,      1,    1,                0, 0                          }, // Vivien '?'
	{ /*0x00c2*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSALLY,       1,    1,                0, 0                          }, // Sally '?'
	{ /*0x00c3*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJOEL0,       1,    1,                0, 0                          }, // Joel Hochberg
	{ /*0x00c4*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJOE2,        1,    1,                0, 0                          }, // Joe '?'
	{ /*0x00c5*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJOE,         1,    1,                0, 0                          }, // Joe '?' (Altered)
	{ /*0x00c6*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSCOTT_H0,    1,    1,                0, 0                          }, // Scott Hochberg
	{ /*0x00c7*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSTEVEH,      1,    1,                0, 0                          }, // Steve Hurst
	{ /*0x00c8*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJIM,         1,    1,                0, 0                          }, // Jim '?' #1
	{ /*0x00c9*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADJIM2,        1,    1,                0, 0                          }, // Jim '?' #2
	{ /*0x00ca*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADNEIL0,       1,    1,                0, 0                          }, // Neil Voss
	{ /*0x00cb*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADLEE,         1,    1,                0, 0                          }, // Lee Ray
	{ /*0x00cc*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCHRIS,       1,    1,                0, 0                          }, // Chris Dolan #1
	{ /*0x00cd*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCHRIS2,      1,    1,                0, 0                          }, // Chris Dolan #2
	{ /*0x00ce*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDES,         1,    1,                0, 0                          }, // Des Easen
	{ /*0x00cf*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSHAUN0,      1,    1,                0, 0                          }, // Shaun Read
	{ /*0x00d0*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPETE,        1,    1,                0, 0                          }, // Pete Cox
	{ /*0x00d1*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADALAN,        1,    1,                0, 0                          }, // Alan Tipper
	{ /*0x00d2*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADBANDOVERFLOW, 1,   1,                0, 0                          }, // Surface Glitch Head
	{ /*0x00d3*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADWRECK,       1,    1,                0, 0                          }, // Wreck
	{ /*0x00d4*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSUBDRAG,     1,    1,                0, 0                          }, // SubDrag
	{ /*0x00d5*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGALORE,      1,    1,                0, 0                          }, // Galore (Plane)
	{ /*0x00d6*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSOGUN,       1,    1,                0, 0                          }, // Sogun
	{ /*0x00d7*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMRKANE,      1,    1,                0, 0                          }, // Mark Kane
	{ /*0x00d8*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADPARIS,       1,    1,                0, 0                          }, // Paris Carver
	{ /*0x00d9*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDONKEY,      1,    1,                0, 0                          }, // Donkey Kong
	{ /*0x00da*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADCASEYDARK,   1,    1,                0, 0                          }, // Casey Dark
	{ /*0x00db*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADHAMM,        1,    1,                0, 0                          }, // Everett Hamm
	{ /*0x00dc*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDRNO,        1,    1,                0, 0                          }, // Dr. No
	{ /*0x00dd*/ 0, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADWAILIN,      1,    1,                0, 0                          }, // Wai Lin
	{ /*0x00de*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADAURIC,       1,    1,                0, 0                          }, // Auric Goldfinger
	{ /*0x00df*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADELEKTRA,     1,    1,                0, 0                          }, // Elektra King
	{ /*0x00e0*/ 1, 1, 0, HEADBODYTYPE_MRBLONDE,    13,  FILE_CHEADMRX,         1,    1,                0, 0                          }, // Tyrant Mr. X
	{ /*0x00e1*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADSKEDAR,      1,    1,                0, 0                          }, // Skedar
	{ /*0x00e2*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCARROLL,     1,    1,                0, 0                          }, // Dr. Caroll
	{ /*0x00e3*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCARROLL_SINISTER, 1, 1,              0, 0                          }, // Dr. Caroll (Sinister)
	{ /*0x00e4*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADCHRISTMASJONES, 1, 1,                0, 0                          }, // Christmas Jones
	{ /*0x00e5*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCJ,          1,    1,                0, 0                          }, // Carl Johnson "CJ"
	{ /*0x00e6*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADMACTONIGHT,  1,    1,                0, 0                          }, // Moon Head
	{ /*0x00e7*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CNATALYA,         1,    0.9660999775,     0, FILE_GHAND_NATALYA         }, // Natalya (Russia)
	{ /*0x00e8*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CXENIA,           1,    1,                0, FILE_GHAND_XENIA           }, // Xenia
	{ /*0x00e9*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CTREVELYAN,       1,    1,                0, FILE_GHAND_TREVELYAN       }, // Trevelyan (Janus)
	{ /*0x00ea*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     174, FILE_COURUMOV,         1,    1.0778000355,     0, FILE_GHAND_OURUMOV         }, // Ourumov
	{ /*0x00eb*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CSPICEBOND,       1,    0.9660999775,     0, FILE_GHAND_NATALYA         }, // Natalya (Cuba)
	{ /*0x00ec*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CDJBOND2,         1,    1.0446000099,     0, FILE_GHAND_BOND            }, // Bond (Tuxedo): Brosnan
	{ /*0x00ed*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CDALTON2,         1,    1.0446000099,     0, FILE_GHAND_BOND            }, // Bond (Classic): Dalton
	{ /*0x00ee*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCONNERY2,        1,    1.0579999685,     0, FILE_GHAND_BONDWHITE       }, // Bond (Classic): Connery
	{ /*0x00ef*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CMOORE2,          1,    1.0446000099,     0, FILE_GHAND_BOND            }, // Bond (Classic): Moore
	{ /*0x00f0*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CLAZENBY,         1,    1.0446000099,     0, FILE_GHAND_BOND            }, // Bond (Classic): Lazenby
	{ /*0x00f1*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     181, FILE_CJAWS,            1,    1.199000001,      0, FILE_GHAND_JAWS            }, // Jaws
	{ /*0x00f2*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CSNOWGUARD,       1,    1,                0, FILE_GHAND_SNOWGUARD       }, // Siberian Special Forces
	{ /*0x00f3*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CSNOWBOND,        1,    1.0437999964,     0, FILE_GHAND_SNOWGUARD       }, // Bond (Arctic)
	{ /*0x00f4*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     134, FILE_CODDJOB,          1,    0.787800014,      0, FILE_GHAND_ODDJOB          }, // Oddjob
	{ /*0x00f5*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CTECHMAN,         1,    1,                0, FILE_GHAND_MRBLONDE        }, // Male Scientist
	{ /*0x00f6*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CGREATGUARD,      1,    1,                0, FILE_GHAND_MISHKIN         }, // Mishkin
	{ /*0x00f7*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CSUITBOND,        1,    1.0579999685,     0, FILE_GHAND_BLUE_SUIT       }, // Bond (Formal)
	{ /*0x00f8*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBOILERBOND,      1,    1.0403000116,     0, FILE_GHAND_BLACKSTEALTH    }, // Bond (Stealth)
	{ /*0x00f9*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBOILERTREV,      1,    1,                0, FILE_GHAND_BLACKSTEALTH    }, // Trevelyan (006)
	{ /*0x00fa*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CTIMBERBOND,      1,    1.0437999964,     0, FILE_GHAND_TIMBERBOND      }, // Bond (Jungle)
	{ /*0x00fb*/ 1, 0, 0, HEADBODYTYPE_MAIAN,       106, FILE_CELTONWAISTCOAT,  1,    0.5727699399,     0, FILE_GHAND_ELVIS           }, // Elton
	{ /*0x00fc*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     163, FILE_CBORIS,           1,    0.9702000022,     0, FILE_GHAND_BORIS           }, // Boris
	{ /*0x00fd*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     158, FILE_CVALENTIN,        1,    0.9323999882,     0, FILE_GHAND_VALENTIN        }, // Valentin
	{ /*0x00fe*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CMAYDAY,          1,    1,                0, FILE_GHAND_MAYDAY          }, // May Day
	{ /*0x00ff*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBARONSAMEDI,     1,    1,                0, FILE_GHAND_SAMEDI          }, // Baron Samedi
	{ /*0x0000*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBARONSAMEDI2,    1,    1,                0, FILE_GHAND_SAMEDI2         }, // Baron Samedi (Death)
	{ /*0x0101*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     158, FILE_CSANTA,           1,    0.9323999882,     0, FILE_GHAND_SANTA           }, // Santa Claus
	{ /*0x0102*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     134, FILE_CELF,             1,    0.787800014,      0, FILE_GHAND_ELF             }, // Elf
	{ /*0x0103*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_COLIVEGUARD,      1,    1,                0, FILE_GHAND_OLIVEGUARD      }, // Russian Soldier
	{ /*0x0104*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CPILOT,           1,    1,                0, FILE_GHAND_PILOT           }, // Helicopter Pilot
	{ /*0x0105*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CTREVGUARD,       1,    1,                0, FILE_GHAND_MISHKIN         }, // Janus Special Forces
	{ /*0x0106*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CARMOURGUARD,     1,    1,                0, FILE_GHAND_ARMOURGUARD     }, // Janus Marine
	{ /*0x0107*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CGREATGUARD2,     1,    1,                0, FILE_GHAND_BROWN           }, // Siberian Guard (Brown)
	{ /*0x0108*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBLUECAMGUARD,    1,    1,                0, FILE_GHAND_BLUECAMGUARD    }, // Arctic Commando
	{ /*0x0109*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBLUEMAN,         1,    1,                0, FILE_GHAND_JOFROCK         }, // Male Civilian (Blue)
	{ /*0x010a*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CGREYMAN,         1,    1,                0, FILE_GHAND_GREYMAN         }, // Male Civilian (Grey)
	{ /*0x010b*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CMOONGUARD,       1,    1,                0, FILE_GHAND_MOONGUARD       }, // Male Moonraker Elite
	{ /*0x010c*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     149, FILE_CMOONFEMALE,      1,    0.8852999806,     0, FILE_GHAND_MOONFEMALE      }, // Female Moonraker Elite
	{ /*0x010d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCAMGUARD,        1,    1,                0, FILE_GHAND_CAMGUARD        }, // Jungle Commando
	{ /*0x010e*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CGREYGUARD,       1,    1,                0, FILE_GHAND_STPGUARD        }, // St. Petersburg Guard
	{ /*0x010f*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CRUSGUARD,        1,    1,                0, FILE_GHAND_RUSGUARD        }, // Russian Infantry
	{ /*0x0110*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCOMMGUARD,       1,    1,                0, FILE_GHAND_COMMGUARD       }, // Russian Commandant
	{ /*0x0111*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CNAVYGUARD,       1,    1,                0, FILE_GHAND_NAVYGUARD       }, // Naval Officer
	{ /*0x0112*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     149, FILE_CFATTECHWOMAN,    1,    0.8852999806,     0, FILE_GHAND_NATALYA         }, // Rosika/Marion
	{ /*0x0113*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     154, FILE_CTECHWOMAN,       1,    0.9160000086,     0, FILE_GHAND_TECHWOMAN       }, // Female Scientist
	{ /*0x0114*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     149, FILE_CJEANWOMAN,       1,    0.8852999806,     0, FILE_GHAND_TECHWOMAN       }, // Female Civilian (Jeans)
	{ /*0x0115*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     149, FILE_CBLUEWOMAN,       1,    0.8852999806,     0, FILE_GHAND_NATALYA         }, // Female Civilian (Skirt)
	{ /*0x0116*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCARDIMAN,        1,    1,                0, FILE_GHAND_JOFROCK         }, // Male Civilian (Vest)
	{ /*0x0117*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCHECKMAN,        1,    1,                0, FILE_GHAND_CHECKMAN        }, // Male Civilian (Plaid)
	{ /*0x0118*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CREDMAN,          1,    1,                0, FILE_GHAND_REDMAN          }, // Male Civilian (Red)
	{ /*0x0119*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBONDALPS,        1,    1.0579999685,     0, FILE_GHAND_BONDALPS        }, // Bond (Classic, Alps)
	{ /*0x011a*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CBONDRANCH,       1,    1.0446000099,     0, FILE_GHAND_BONDRANCH       }, // Bond (Classic, Ranch)
	{ /*0x011b*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CGALOREPLANE,     1,    0.96609998,       0, FILE_GHAND_GALOREPLANE     }, // Galore (Plane)
	{ /*0x011c*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CGALORERANCH,     1,    0.96609998,       0, FILE_GHAND_GALORERANCH     }, // Galore (Ranch)
	{ /*0x011d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CSOGUNTRON,       1,    0.9389671683,     0, FILE_GHAND_SOGUNTRON       }, // Sogun
	{ /*0x011e*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     144, FILE_CCARROLL3,        1,    1,                0, FILE_GHAND_CAROLL          }, // Dr. Caroll
	{ /*0x011f*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CNATALYA_XMAS,    1,    0.9660999775,     0, FILE_GHAND_NATALYA         }, // Natalya (Santa)
	{ /*0x0120*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     162, FILE_CNATALYA_ELF,     1,    0.9660999775,     0, FILE_GHAND_NATALYA         }, // Natalya (Elf)
	{ /*0x0121*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CCASEYDARK,       1,    0.9530516267,     0, FILE_GHAND_CASEYDARK       }, // Casey Dark
	{ /*0x0122*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CEVERETTHAMM,     1,    1.0446009636,     0, FILE_GHAND_BLUE_SUIT       }, // Everett Hamm
	{ /*0x0123*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CXENIA_XMAS,      1,    1,                0, FILE_GHAND_XENIA_XMAS      }, // Xenia (Santa)
	{ /*0x0124*/ 1, 0, 0, HEADBODYTYPE_MRBLONDE,    181, FILE_CMRX,             1.1499999762, 0.9750000238, 0, FILE_GHAND_MRX         }, // Tyrant Mr. X
	{ /*0x0125*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     149, FILE_CSILVERWOMAN,     1,    0.8852999806,     0, FILE_GHAND_TECHWOMAN       }, // Estate Patron (Silver)
	{ /*0x0126*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     181, FILE_CJAWS_BLUE,       1,    1.1990000010,     0, FILE_GHAND_JAWS_BLUE       }, // Jaws (Blazer)
	{ /*0x0127*/ 0, 0, 1, HEADBODYTYPE_FEMALEGUARD, 160, FILE_CFEMSPY_BLU,      1,    0.9389669895,     0, FILE_GHAND_FEMSPY_BLU      }, // Female Spy (Blue)
	{ /*0x0128*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     167, FILE_CCJ,              1,    1.0000000000,     0, FILE_GHAND_PRESIDENT_CLONE }, // Carl Johnson "CJ"
#endif
};

// [headnum][hattype]
struct hatposition var8007dae4[1][6] = {
	{
		// HEAD_SHAUN
		{ -0.070299997925758, 0.49189999699593, -0.83359998464584, 1.072811961174,   1.0883259773254,  0.92612099647522 },
		{ -0.10000000149012,  0.42750000953674, -0.48249998688698, 1.0333679914474,  0.96552097797394, 0.92990499734879 },
		{ 0.18000000715256,   0,                0,                 1.0722140073776,  1,                1                },
		{ 0.23700000345707,   0.97699999809265, -0.43999999761581, 1.1784629821777,  1.1406099796295,  1.1434650421143  },
		{ -0.090300001204014, 0.23190000653267, 0.12639999389648,  0.99080002307892, 1.0199999809265,  0.84659999608994 },
		{ 0,                  0.14849999547005, 0.37929999828339,  1.1548000574112,  0.99190002679825, 0.95139998197556 },
	}
};
