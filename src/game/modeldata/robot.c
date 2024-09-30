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
	{ /*0x0004*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_COMBAT, 1,    1,                0, 0                          },
	{ /*0x0005*/ 1, 1, 0, HEADBODYTYPE_MAIAN,       27,  FILE_CHEADELVIS,       1,    1,                0, 0                          },
	{ /*0x0006*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADROSS,        1,    1,                0, 0                          },
	{ /*0x0007*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADCARRINGTON,  1,    1,                0, 0                          },
	{ /*0x0008*/ 1, 1, 0, HEADBODYTYPE_MRBLONDE,    13,  FILE_CHEADMRBLONDE,    1,    1,                0, 0                          },
	{ /*0x0009*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADTRENT,       1,    1,                0, 0                          },
	{ /*0x000a*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADDDSHOCK,     1,    1,                0, 0                          },
	{ /*0x000b*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     13,  FILE_CHEADGRAHAM,      1,    1,                0, 0                          },
	{ /*0x000c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_FROCK,  1,    1,                0, 0                          },
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
#else // PD Plus Mod
	{ /*0x0015*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADGREY,        1,    1,                0, 0                          }, // Joannna (JP version)
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
	{ /*0x002f*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARKAQUA,    1,    1,                0, 0                          },
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
	{ /*0x003c*/ 0, 1, 0, HEADBODYTYPE_FEMALE,      13,  FILE_CHEADDARK_SNOW,   1,    1,                0, 0                          },
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
	{ /*0x0056*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_COMBAT,     1,    0.95305162668228, 0, FILE_GCOMBATHANDSLOD       },
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
	{ /*0x005f*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CLABTECH,         1,    0.93896716833115, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x0060*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CSTRIPES,         1,    0.92769956588745, 0, FILE_GHAND_BLACKGUARD      },
	{ /*0x0061*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_FROCK,      1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0062*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_TRENCH,     1,    0.95305162668228, 0, FILE_GHAND_JOTRENCH        },
	{ /*0x0063*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_COFFICEWORKER,    1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0064*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_COFFICEWORKER2,   1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0065*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      140, FILE_CSECRETARY,       1,    0.87323945760727, 0, FILE_GHAND_JOFROCK         },
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
	{ /*0x006d*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_RIPPED,     1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
	{ /*0x006e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     160, FILE_CDD_GUARD,        1,    0.93896716833115, 0, FILE_GHAND_DDSECURITY      },
	{ /*0x006f*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     157, FILE_CDD_SHOCK_INF,    1,    0.93896716833115, 0, FILE_GHAND_DDSHOCK         },
#ifdef PLATFORM_N64
	{ /*0x0070*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CTESTCHR,         1,    1,                0, 0                          },
#else // PD Plus Mod
	{ /*0x0070*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     132, FILE_CTESTCHR,         1,    1,                0, FILE_PCHRFLASHBANG         }, // Dr. Caroll
#endif
	{ /*0x0071*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     155, FILE_CBIOTECH,         1,    0.93896716833115, 0, FILE_GHAND_DDBIO           },
	{ /*0x0072*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CFBIGUY,          1,    0.93896716833115, 0, FILE_GHAND_FBIARM          },
	{ /*0x0073*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCIAGUY,          1,    0.93896716833115, 0, FILE_GHAND_CIA             },
	{ /*0x0074*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CA51TROOPER,      1,    0.93896716833115, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0075*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     157, FILE_CA51AIRMAN,       1,    0.93896716833115, 0, FILE_GHAND_A51AIRMAN       },
	{ /*0x0076*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CCHICROB,         1,    1,                0, 0                          },
	{ /*0x0077*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     153, FILE_CSTEWARD,         1,    0.89201879501343, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0078*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      143, FILE_CSTEWARDESS,      1,    0.85446006059647, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0079*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT,       1,    0.93896716833115, 0, FILE_GHAND_PRESIDENT       },
	{ /*0x007a*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      143, FILE_CSTEWARDESS_COAT, 1,    0.85446006059647, 0, FILE_GHAND_STEWARDESS_COAT },
	{ /*0x007b*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CMINISKEDAR,      0.75, 0.5,              0, 0                          },
	{ /*0x007c*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CNSA_LACKEY,      1,    0.93896716833115, 0, FILE_GHAND_CARRINGTON      },
	{ /*0x007d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRES_SECURITY,   1,    0.93896716833115, 0, FILE_GHAND_CARRINGTON      },
	{ /*0x007e*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      142, FILE_CNEGOTIATOR,      1,    0.85446006059647, 0, FILE_GHAND_JOFROCK         },
	{ /*0x007f*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CG5_GUARD,        1,    0.93896716833115, 0, FILE_GHAND_G5GUARD         },
	{ /*0x0080*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CPELAGIC_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_TRAGIC_PELAGIC  },
	{ /*0x0081*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CG5_SWAT_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_G5GUARD         },
	{ /*0x0082*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     158, FILE_CALASKAN_GUARD,   1,    0.93896716833115, 0, FILE_GHAND_JOSNOW          },
	{ /*0x0083*/ 1, 0, 1, HEADBODYTYPE_MAIAN,       106, FILE_CMAIAN_SOLDIER,   1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
#ifdef PLATFORM_N64
	{ /*0x0084*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT_CLONE, 1,    0.93896716833115, 0, FILE_GCOMBATHANDSLOD       },
#else // PD Plus Mod
	{ /*0x0084*/ 1, 1, 0, HEADBODYTYPE_DEFAULT,     169, FILE_PTTB_BOX        , 1,    1               , 0, FILE_PDOORCONSOLE          }, // Skedar
#endif
	{ /*0x0085*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPRESIDENT_CLONE, 1,    0.93896716833115, 0, FILE_GHAND_PRESIDENT       },
	{ /*0x0086*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_AF1,        1,    0.95305162668228, 0, FILE_GHAND_JOPILOT         },
	{ /*0x0087*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKWET,         1,    0.95305162668228, 0, FILE_GHAND_JOWETSUIT       },
	{ /*0x0088*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKAQUALUNG,    1,    0.95305162668228, 0, FILE_GHAND_JOWETSUIT       },
	{ /*0x0089*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKSNOW,        1,    0.95305162668228, 0, FILE_GHAND_JOSNOW          },
	{ /*0x008a*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARKLAB,         1,    0.95305162668228, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x008b*/ 0, 0, 1, HEADBODYTYPE_FEMALE,      159, FILE_CFEMLABTECH,      1,    0.87323945760727, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x008c*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CDDSNIPER,        1,    0.93896716833115, 0, FILE_GHAND_DDSNIPER        },
	{ /*0x008d*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CPILOTAF1,        1,    0.88262909650803, 0, FILE_GHAND_JOPILOT         },
	{ /*0x008e*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCILABTECH,       1,    0.93896716833115, 0, FILE_GHAND_CIFEMTECH       },
	{ /*0x008f*/ 0, 0, 1, HEADBODYTYPE_FEMALE,      159, FILE_CCIFEMTECH,       1,    0.86854463815689, 0, FILE_GHAND_CIFEMTECH       },
	{ /*0x0090*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CCARREVENINGSUIT, 1,    0.85915493965149, 0, FILE_GHAND_MRBLONDE        },
	{ /*0x0091*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CJONATHON,        1,    0.93896716833115, 0, FILE_GHAND_A51GUARD        },
	{ /*0x0092*/ 1, 0, 1, HEADBODYTYPE_DEFAULT,     159, FILE_CCISOLDIER,       1,    0.93896716833115, 0, FILE_GHAND_CISOLDIER       },
	{ /*0x0093*/ 1, 0, 0, HEADBODYTYPE_DEFAULT,     159, FILE_CSKEDARKING,      1,    1.25,             0, 0                          },
	{ /*0x0094*/ 1, 0, 0, HEADBODYTYPE_MAIAN,       106, FILE_CELVISWAISTCOAT,  1,    0.57276993989944, 0, FILE_GHAND_ELVIS           },
	{ /*0x0095*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_LEATHER,    1,    0.95305162668228, 0, FILE_GHAND_JOFROCK         },
	{ /*0x0096*/ 0, 0, 0, HEADBODYTYPE_FEMALE,      159, FILE_CDARK_NEGOTIATOR, 1,    0.95305162668228, 0, FILE_GHAND_JOAF1           },
	{ /*0x0097*/ 0, 0, 0, HEADBODYTYPE_DEFAULT,     0,   0,                     1,    0,                0, FILE_GCOMBATHANDSLOD       },
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
