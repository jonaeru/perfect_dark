#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <PR/ultratypes.h>
#include "lib/rzip.h"
#include "romdata.h"
#include "fs.h"
#include "system.h"
#include "preprocess.h"
#include "platform.h"

/**
 * asset files and ROM segments can be replaced by optional external files,
 * but asset filenames still have to be either pulled from the ROM or from an
 * external file, so stuff can't be completely custom
 * 
 * all data is assumed to be big endian, so it has to be byteswapped
 * at load time, which is fucking terrible
 */

#define ROMDATA_FILEDIR "files"
#define ROMDATA_SEGDIR "segs"

#define ROMDATA_ROM_NAME "pd." VERSION_ROMID ".z64"
#define ROMDATA_ROM_SIZE 33554432

#if VERSION == VERSION_NTSC_FINAL
#define ROMDATA_ROM_TITLE "Perfect Dark"
#define ROMDATA_ROM_ID "NPDE"
#define ROMDATA_ROM_DESC "NTSC v1.1"
#define ROMDATA_FILES_OFS 0x28080
#define ROMDATA_DATA_OFS 0x39850
#elif VERSION == VERSION_PAL_FINAL
#define ROMDATA_ROM_TITLE "Perfect Dark"
#define ROMDATA_ROM_ID "NPDP"
#define ROMDATA_ROM_DESC "PAL"
#define ROMDATA_FILES_OFS 0x28910
#define ROMDATA_DATA_OFS 0x39850
#elif VERSION == VERSION_JPN_FINAL
#define ROMDATA_ROM_TITLE "PERFECT DARK"
#define ROMDATA_ROM_ID "NPDJ"
#define ROMDATA_ROM_DESC "JPN"
#define ROMDATA_FILES_OFS 0x28800
#define ROMDATA_DATA_OFS 0x39850
#else
#error "This ROM version is unsupported."
#endif

#define ROMDATA_MAX_FILES NUM_FILES

#define GBC_ROM_NAME "pd.gbc"
#define GBC_ROM_SIZE 4194304

u8 *g_RomFile;
u32 g_RomFileSize;

static u8 *romDataSeg;
static u32 romDataSegSize;
static const char *romName = ROMDATA_ROM_NAME;

enum loadsource {
	SRC_UNLOADED = 0,
	SRC_ROM,
	SRC_EXTERNAL
};

struct romfilepatch {
	u32 ofs;
	u32 len;
	const char *src;
	const char *dst;
};

struct romfile {
	u8 **segstart;
	u8 **segend;
	const char *name;
	u8 *data;
	u32 size;
	preprocessfunc preprocess;
	s32 source; // enum loadsource
	s32 preprocessed;
	const struct romfilepatch *patches;
	u32 numpatches;
};

/* patches for individual files; applied on file load, before preprocFuncs, but */
/* after unzip; only applied when loading from a ROM file                       */
static const struct romfilepatch filePatches[] = {
	/* FILE_USETUPLUE: fixes Jon's double "if what" in Infiltration outro */
	{ 0x92a2, 1, "\x6c", "\x99" },
	{ 0x92b0, 1, "\x6c", "\x99" },
};

static struct romfile fileSlots[5][ROMDATA_MAX_FILES] = {
	{ [FILE_USETUPLUE] = { .patches = &filePatches[0], .numpatches = 2 } },
	{ [FILE_USETUPLUE] = { .patches = &filePatches[0], .numpatches = 2 } }, // GoldenEye X Mod
	{ [FILE_USETUPLUE] = { .patches = &filePatches[0], .numpatches = 2 } }, // Kakariko Village Mod
	{ [FILE_USETUPLUE] = { .patches = &filePatches[0], .numpatches = 2 } }, // Dark Moon Mod
	{ [FILE_USETUPLUE] = { .patches = &filePatches[0], .numpatches = 2 } }, // Goldfinger 64 Mod
};

#define ROMSEG_START(n) _ ## n ## SegmentRomStart
#define ROMSEG_END(n) _ ## n ## SegmentRomEnd

/* segment table for ntsc-final                                                     */
/* size will get calculated automatically if it is 0                                */
/* if there are replacement files in the data dir, they will be loaded instead      */
/* offsets are specified for ntsc-final, pal-final and jpn-final in that order      */
#define ROMSEG_LIST() \
	ROMSEG_DECL_SEG(fontjpnsingle,      0x194b20,  0x180330,  0x0,       0x0,      preprocessJpnFont       ) \
	ROMSEG_DECL_SEG(fontjpnmulti,       0x19fb40,  0x18b340,  0x0,       0x0,      preprocessJpnFont       ) \
	ROMSEG_DECL_SEG(animations,         0x1a15c0,  0x18cdc0,  0x190c50,  0x0,      preprocessAnimations    ) \
	ROMSEG_DECL_SEG(mpconfigs,          0x7d0a40,  0x7bc240,  0x7c00d0,  0x11e0,   preprocessMpConfigs     ) \
	ROMSEG_DECL_SEG(mpstringsE,         0x7d1c20,  0x7bd420,  0x7c12b0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsJ,         0x7d5320,  0x7c0b20,  0x7c49b0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsP,         0x7d8a20,  0x7c4220,  0x7c80b0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsG,         0x7dc120,  0x7c7920,  0x7cb7b0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsF,         0x7df820,  0x7cb020,  0x7ceeb0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsS,         0x7e2f20,  0x7ce720,  0x7d25b0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(mpstringsI,         0x7e6620,  0x7d1e20,  0x7d5cb0,  0x3700,   NULL                    ) \
	ROMSEG_DECL_SEG(firingrange,        0x7e9d20,  0x7d5520,  0x7d93b0,  0x1550,   NULL                    ) \
	ROMSEG_DECL_SEG(fonttahoma,         0x7f7860,  0x7e3060,  0x7e6ef0,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(fontnumeric,        0x7f8b20,  0x7e4320,  0x7e81b0,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(fonthandelgothicsm, 0x7f9d30,  0x7e5530,  0x7e93c0,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(fonthandelgothicxs, 0x7fbfb0,  0x7e87b0,  0x7ec640,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(fonthandelgothicmd, 0x7fdd80,  0x7eae20,  0x7eecb0,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(fonthandelgothiclg, 0x8008e0,  0x7eee70,  0x7f2d00,  0x0,      preprocessFont          ) \
	ROMSEG_DECL_SEG(sfxctl,             0x80a250,  0x7f87e0,  0x7fc670,  0x2fb80,  preprocessALBankFile    ) \
	ROMSEG_DECL_SEG(sfxtbl,             0x839dd0,  0x828360,  0x82c1f0,  0x4c2160, NULL                    ) \
	ROMSEG_DECL_SEG(seqctl,             0xcfbf30,  0xcea4c0,  0xcee350,  0xa060,   preprocessALBankFile    ) \
	ROMSEG_DECL_SEG(seqtbl,             0xd05f90,  0xcf4520,  0xcf83b0,  0x17c070, NULL                    ) \
	ROMSEG_DECL_SEG(sequences,          0xe82000,  0xe70590,  0xe74420,  0x563a0,  preprocessSequences     ) \
	ROMSEG_DECL_SEG(texturesdata,       0x1d65f40, 0x1d5ca20, 0x1d61f90, 0x0,      NULL                    ) \
	ROMSEG_DECL_SEG(textureslist,       0x1ff7ca0, 0x1fee780, 0x1ff68f0, 0x0,      preprocessTexturesList  ) \
	ROMSEG_DECL_SEG(copyright,          0x1ffea20, 0x1ff5500, 0x1ffd6b0, 0xb30,    NULL                    ) \
	ROMSEG_DECL_SEG(fontjpn,            0x0,       0x0,       0x178c40,  0x17920,  preprocessJpnFont       )

// declare the vars first

#undef ROMSEG_DECL_SEG
#define ROMSEG_DECL_SEG(name, ofs_ntsc, ofs_pal, ofs_jpn, size, preproc) u8 *ROMSEG_START(name), *ROMSEG_END(name);
ROMSEG_LIST()

// this is part of the animations seg and as such does not follow the naming convention
// these are set in preprocessAnimations
u8 *_animationsTableRomStart;
u8 *_animationsTableRomEnd;

// then build the table

#undef ROMSEG_DECL_SEG

#if VERSION == VERSION_NTSC_FINAL
#define ROMSEG_DECL_SEG(name, ofs_ntsc, ofs_pal, ofs_jpn, size, preproc) { &ROMSEG_START(name), &ROMSEG_END(name), #name, (u8 *)ofs_ntsc, size, preproc },
#elif VERSION == VERSION_PAL_FINAL
#define ROMSEG_DECL_SEG(name, ofs_ntsc, ofs_pal, ofs_jpn, size, preproc) { &ROMSEG_START(name), &ROMSEG_END(name), #name, (u8 *)ofs_pal, size, preproc },
#elif VERSION == VERSION_JPN_FINAL
#define ROMSEG_DECL_SEG(name, ofs_ntsc, ofs_pal, ofs_jpn, size, preproc) { &ROMSEG_START(name), &ROMSEG_END(name), #name, (u8 *)ofs_jpn, size, preproc },
#endif

static struct romfile romSegs[] = {
	ROMSEG_LIST()
	{ NULL, NULL, NULL, NULL, 0, NULL },
};

/* the game sets g_LoadType to the type of file it expects,              */
/* so we can hijack that in fileLoad and automatically byteswap the file */
static preprocessfunc filePreprocFuncs[] = {
	/* LOADTYPE_NONE  */ NULL,
	/* LOADTYPE_BG    */ NULL, // loaded in parts
	/* LOADTYPE_TILES */ preprocessTilesFile,
	/* LOADTYPE_LANG  */ preprocessLangFile,
	/* LOADTYPE_SETUP */ preprocessSetupFile,
	/* LOADTYPE_PADS  */ preprocessPadsFile,
	/* LOADTYPE_MODEL */ preprocessModelFile,
	/* LOADTYPE_GUN   */ preprocessGunFile,
};

static inline void romdataWrongRomError(const char *fmt, ...)
{
	char reason[1024];
	reason[0] = '\0';

	va_list args;
	va_start(args, fmt);
	vsnprintf(reason, sizeof(reason), fmt, args);
	va_end(args);

	sysFatalError("Wrong ROM file.\n%s\nEnsure that you have the correct " ROMDATA_ROM_DESC " ROM in z64 format.", reason);
}

static inline void romdataLoadRom(void)
{
	sysLogPrintf(LOG_NOTE, "ROM file: %s", romName);

	g_RomFile = fsFileLoad(romName, &g_RomFileSize);

	if (!g_RomFile) {
		sysFatalError("Could not open ROM file %s.\nEnsure that it is in the %s directory.", romName, fsFullPath(""));
	}

	// zips are not guaranteed to start with PK, but might as well at least try
	if (g_RomFileSize > 2 && (!memcmp(g_RomFile, "PK", 2) || !memcmp(g_RomFile, "Rar", 3) || !memcmp(g_RomFile, "7z", 2))) {
		romdataWrongRomError("Your ROM is in an archive file. Please extract it.");
	}

	if (g_RomFileSize != ROMDATA_ROM_SIZE) {
		romdataWrongRomError("ROM size does not match: expected: %u, got: %u.", ROMDATA_ROM_SIZE, g_RomFileSize);
	}

	if (memcmp(g_RomFile + 0x3b, ROMDATA_ROM_ID, 4) || memcmp(g_RomFile + 0x20, ROMDATA_ROM_TITLE, sizeof(ROMDATA_ROM_TITLE) - 1)) {
		romdataWrongRomError("ROM header does not match.");
	}

	// inflate the compressed data segment since that's where some useful stuff is

	u8 *zipped = g_RomFile + ROMDATA_DATA_OFS;
	if (!rzipIs1173(zipped)) {
		romdataWrongRomError("Data segment is not 1173-compressed.");
	}

	const u32 dataSegLen = ((u32)zipped[2] << 16) | ((u32)zipped[3] << 8) | (u32)zipped[4];
	if (dataSegLen < ROMDATA_FILES_OFS) {
		romdataWrongRomError("Data segment too small (%u), need at least %u.", dataSegLen, ROMDATA_FILES_OFS);
	}

	u8 *dataSeg = sysMemAlloc(dataSegLen);
	if (!dataSeg) {
		sysFatalError("Could not allocate %u bytes for data segment.", dataSegLen);
	}

	u8 scratch[5 * 1024];
	if (rzipInflate(zipped, dataSeg, scratch) < 0) {
		free(dataSeg);
		sysFatalError("Could not inflate data segment.");
	}

	romDataSeg = dataSeg;
	romDataSegSize = dataSegLen;
}

static inline void romdataUpdateSegStartEnd(struct romfile* seg)
{
	if (seg->segstart) {
		*seg->segstart = seg->data;
	}

	if (seg->segend) {
		*seg->segend = seg->data + seg->size;
	}
}

static inline void romdataInitSegment(struct romfile *seg)
{
	if (!seg->data) {
		// unused in this ROM, skip it
		sysLogPrintf(LOG_NOTE, "skipping segment %s", seg->name);
		return;
	}

	if (!seg->size) {
		// size unknown
		if (seg[1].name) {
			// use next segment's base to calculate
			seg->size = seg[1].data - seg->data;
		} else {
			// this is the last segment, calculate based on rom size
			seg->size = (uintptr_t)g_RomFileSize - (uintptr_t)seg->data;
		}
	}

	// check if we have an external replacement and load it if so
	char tmp[FS_MAXPATH];
	snprintf(tmp, sizeof(tmp), ROMDATA_SEGDIR "/%s", seg->name);
	u8 *newData = NULL;
	const s32 extFileSize = fsFileSize(tmp);
	if (extFileSize > 0) {
		newData = fsFileLoad(tmp, &seg->size);
	}

	if (!newData) {
		// no external data, just make it point to the rom
		if (g_RomFile) {
			newData = g_RomFile + (uintptr_t)seg->data;
			seg->source = SRC_ROM;
			sysLogPrintf(LOG_NOTE, "loading segment %s from ROM (offset %08x pointer %p)", seg->name, (uintptr_t)seg->data, newData);
		} else {
			sysFatalError("No ROM or external file for segment:\n%s", seg->name);
		}
	} else {
		// loaded external data
		seg->source = SRC_EXTERNAL;
		sysLogPrintf(LOG_NOTE, "loading segment %s from file (pointer %p)", seg->name, newData);
	}

	seg->data = newData;

	romdataUpdateSegStartEnd(seg);

	// call the post load function if any
	if (seg->preprocess && !seg->preprocessed) {
		newData = seg->preprocess(seg->data, seg->size, &seg->size);

		if (newData) {
			if (seg->source == SRC_EXTERNAL)
				sysMemFree(seg->data);
			seg->data = newData;
			romdataUpdateSegStartEnd(seg);
		}
		
		seg->preprocessed = 1;
	}
}

static inline s32 romdataLoadExternalFileList(void)
{
	romDataSeg = fsFileLoad("filenames.lst", &romDataSegSize); // this null terminates the file by itself
	if (!romDataSeg || !romDataSegSize) {
		return 0;
	}

	s32 n = 1;
	char *p = (char *)romDataSeg;
	while (*p && n < ROMDATA_MAX_FILES) {
		// skip whitespace
		while (*p && isspace(*p)) ++p;
		if (*p) {
			const char *start = p;
			// skip to next whitespace or end of file
			while (*p && !isspace(*p)) ++p;
			// null terminate the name if needed
			if (*p) {
				*p++ = '\0';
			}
			fileSlots[g_ModNum][n++].name = start;
		}
	}

	return n - 1;
}

static inline void romdataInitFiles(void)
{
	if (!g_RomFile) {
		// no ROM; try to load the file name list from disk
		if (!romdataLoadExternalFileList()) {
			sysFatalError("No ROM file or external filename table found.");
		}
		return;
	}

	// the file offset table is in the data seg
	const u32 *offsets = (u32 *)(romDataSeg + ROMDATA_FILES_OFS);
	u32 i;
	for (i = 1; offsets[i]; ++i) {
		if (offsets + i + 1 < (u32 *)(romDataSeg + romDataSegSize)) {
			const u32 nextofs = PD_BE32(offsets[i + 1]);
			const u32 ofs = PD_BE32(offsets[i]);
			fileSlots[MOD_NORMAL][i].data = g_RomFile + ofs;
			fileSlots[MOD_NORMAL][i].size = nextofs - ofs;
			fileSlots[MOD_NORMAL][i].source = SRC_UNLOADED;
			fileSlots[MOD_NORMAL][i].preprocessed = 0;

			for (int modnum = MOD_GEX; modnum < NUM_MODS; modnum++) {
				fileSlots[modnum][i] = fileSlots[MOD_NORMAL][i];
			}
		}
	}

	// last offset is to the name table
	const u32 *nameOffsets = (u32 *)(g_RomFile + PD_BE32(offsets[i - 1]));
	for (i = 1; nameOffsets[i]; ++i) {
		const u32 ofs = PD_BE32(nameOffsets[i]);
		fileSlots[MOD_NORMAL][i].name = (const char *)nameOffsets + ofs; // ofs is relative to the start of the name table

		for (int modnum = MOD_GEX; modnum < NUM_MODS; modnum++) {
			fileSlots[modnum][i].name = fileSlots[MOD_NORMAL][i].name;
		}
	}
}

struct FileSlotExpansionData {
	s32 fileNum;
	const char *name;
};

static const struct FileSlotExpansionData expansionFiles[] = {
	// All in One Mod
	{ FILE_CCARROLL2,             "Ccarroll2Z"             }, // Dr. Caroll Body
	{ FILE_CSKEDAR2,              "Cskedar2Z"              }, // Skedar Body
	{ FILE_GHAND_CAROLL,          "Ghand_carollZ"          }, // Dr. Caroll Hand
	{ FILE_GHAND_SKEDAR,          "Ghand_skedarZ"          }, // Skedar Hand
	{ FILE_CHEADNATALYA,          "CheadnatalyaZ"          }, // Natalya (Russia) Head
	{ FILE_CNATALYA,              "CnatalyaZ"              }, // Natalya (Russia) Body
	{ FILE_CHEADXENIA,            "CheadxeniaZ"            },
	{ FILE_CXENIA,                "CxeniaZ"                },
	{ FILE_GHAND_XENIA,           "Ghand_xeniaZ"           },
	{ FILE_CHEADTREVELYAN,        "CheadtrevelyanZ"        },
	{ FILE_CTREVELYAN,            "CtrevelyanZ"            },
	{ FILE_GHAND_TREVELYAN,       "Ghand_trevelyanZ"       },
	{ FILE_CHEADOURUMOV,          "CheadourumovZ"          },
	{ FILE_COURUMOV,              "CourumovZ"              },
	{ FILE_GHAND_OURUMOV,         "Ghand_ourumovZ"         },
	{ FILE_CSPICEBOND,            "CspicebondZ"            },
	{ FILE_CHEADBROSNAN,          "CheadbrosnanZ"          },
	{ FILE_CDJBOND2,              "Cdjbond2Z"              },
	{ FILE_GHAND_BOND,            "Ghand_bondZ"            },
	{ FILE_CHEADDALTON,           "CheaddaltonZ"           },
	{ FILE_CDALTON2,              "Cdalton2Z"              },
	{ FILE_CHEADCONNERY,          "CheadconneryZ"          },
	{ FILE_CCONNERY2,             "Cconnery2Z"             },
	{ FILE_GHAND_BONDWHITE,       "Ghand_bondwhiteZ"       },
	{ FILE_CHEADMOORE,            "CheadmooreZ"            },
	{ FILE_CMOORE2,               "Cmoore2Z"               },
	{ FILE_CHEADLAZENBY,          "CheadlazenbyZ"          },
	{ FILE_CLAZENBY,              "ClazenbyZ"              },
	{ FILE_CHEADJAWS,             "CheadjawsZ"             },
	{ FILE_CJAWS,                 "CjawsZ"                 },
	{ FILE_CHEADSNOWGUARD,        "CheadsnowguardZ"        },
	{ FILE_CSNOWGUARD,            "CsnowguardZ"            },
	{ FILE_GHAND_SNOWGUARD,       "Ghand_snowguardZ"       },
	{ FILE_CHEADBROSNAN2,         "Cheadbrosnan2Z"         },
	{ FILE_CSNOWBOND,             "CsnowbondZ"             },
	{ FILE_CHEADODDJOB,           "CheadoddjobZ"           },
	{ FILE_CODDJOB,               "CoddjobZ"               },
	{ FILE_GHAND_ODDJOB,          "Ghand_oddjobZ"          },
	{ FILE_CHEADDAVED,            "CheaddavedZ"            },
	{ FILE_CHEADDAVED2,           "Cheaddaved2Z"           },
	{ FILE_CTECHMAN,              "CtechmanZ"              },
	{ FILE_CHEADMISHKIN,          "CheadmishkinZ"          },
	{ FILE_CGREATGUARD,           "CgreatguardZ"           },
	{ FILE_GHAND_MISHKIN,         "Ghand_mishkinZ"         },
	{ FILE_CSUITBOND,             "CsuitbondZ"             },
	{ FILE_GHAND_BLUE_SUIT,       "Ghand_blue_suitZ"       },
	{ FILE_CBOILERBOND,           "CboilerbondZ"           },
	{ FILE_GHAND_BLACKSTEALTH,    "Ghand_blackstealthZ"    },
	{ FILE_CBOILERTREV,           "CboilertrevZ"           },
	{ FILE_CTIMBERBOND,           "CtimberbondZ"           },
	{ FILE_CELTONWAISTCOAT,       "CeltonwaistcoatZ"       },
	{ FILE_CHEADBORIS,            "CheadborisZ"            },
	{ FILE_CBORIS,                "CborisZ"                },
	{ FILE_GHAND_BORIS,           "Ghand_borisZ"           },
	{ FILE_CHEADVALENTIN,         "CheadvalentinZ"         },
	{ FILE_CVALENTIN,             "CvalentinZ"             },
	{ FILE_CHEADMAYDAY,           "CheadmaydayZ"           },
	{ FILE_CMAYDAY,               "CmaydayZ"               },
	{ FILE_GHAND_MAYDAY,          "Ghand_maydayZ"          },
	{ FILE_CHEADBARONSAMEDI,      "CheadbaronsamediZ"      },
	{ FILE_CBARONSAMEDI,          "CbaronsamediZ"          },
	{ FILE_GHAND_SAMEDI,          "Ghand_samediZ"          },
	{ FILE_CHEADBARONSAMEDI2,     "Cheadbaronsamedi2Z"     },
	{ FILE_CBARONSAMEDI2,         "Cbaronsamedi2Z"         },
	{ FILE_GHAND_SAMEDI2,         "Ghand_samedi2Z"         },
	{ FILE_CHEADBALACLAVA,        "CheadbalaclavaZ"        },
	{ FILE_CHEADDWAYNE,           "CheaddwayneZ"           },
	{ FILE_CHEADSANTA,            "CheadsantaZ"            },
	{ FILE_CSANTA,                "CsantaZ"                },
	{ FILE_GHAND_SANTA,           "Ghand_santaZ"           },
	{ FILE_CHEADELF,              "CheadelfZ"              },
	{ FILE_CELF,                  "CelfZ"                  },
	{ FILE_GHAND_ELF,             "Ghand_elfZ"             },
	{ FILE_COLIVEGUARD,           "ColiveguardZ"           },
	{ FILE_GHAND_OLIVEGUARD,      "Ghand_oliveguardZ"      },
	{ FILE_CHEADPILOT,            "CheadpilotZ"            },
	{ FILE_CPILOT,                "CpilotZ"                },
	{ FILE_GHAND_PILOT,           "Ghand_pilotZ"           },
	{ FILE_CTREVGUARD,            "CtrevguardZ"            },
	{ FILE_CARMOURGUARD,          "CarmourguardZ"          },
	{ FILE_GHAND_GREYMAN,         "Ghand_greymanZ"         },
	{ FILE_CGREATGUARD2,          "Cgreatguard2Z"          },
	{ FILE_GHAND_BROWN,           "Ghand_brownZ"           },
	{ FILE_CBLUECAMGUARD,         "CbluecamguardZ"         },
	{ FILE_GHAND_BLUECAMGUARD,    "Ghand_bluecamguardZ"    },
	{ FILE_CBLUEMAN,              "CbluemanZ"              },
	{ FILE_CGREYMAN,              "CgreymanZ"              },
	{ FILE_CMOONGUARD,            "CmoonguardZ"            },
	{ FILE_GHAND_MOONGUARD,       "Ghand_moonguardZ"       },
	{ FILE_CMOONFEMALE,           "CmoonfemaleZ"           },
	{ FILE_CCAMGUARD,             "CcamguardZ"             },
	{ FILE_GHAND_CAMGUARD,        "Ghand_camguardZ"        },
	{ FILE_CGREYGUARD,            "CgreyguardZ"            },
	{ FILE_GHAND_STPGUARD,        "Ghand_stpguardZ"        },
	{ FILE_CRUSGUARD,             "CrusguardZ"             },
	{ FILE_GHAND_RUSGUARD,        "Ghand_rusguardZ"        },
	{ FILE_CCOMMGUARD,            "CcommguardZ"            },
	{ FILE_GHAND_COMMGUARD,       "Ghand_commguardZ"       },
	{ FILE_CNAVYGUARD,            "CnavyguardZ"            },
	{ FILE_GHAND_NAVYGUARD,       "Ghand_navyguardZ"       },
	{ FILE_CHEADMARION,           "CheadmarionZ"           },
	{ FILE_CFATTECHWOMAN,         "CfattechwomanZ"         },
	{ FILE_CTECHWOMAN,            "CtechwomanZ"            },
	{ FILE_CJEANWOMAN,            "CjeanwomanZ"            },
	{ FILE_CBLUEWOMAN,            "CbluewomanZ"            },
	{ FILE_CCARDIMAN,             "CcardimanZ"             },
	{ FILE_CCHECKMAN,             "CcheckmanZ"             },
	{ FILE_GHAND_CHECKMAN,        "Ghand_checkmanZ"        },
	{ FILE_CREDMAN,               "CredmanZ"               },
	{ FILE_GHAND_REDMAN,          "Ghand_redmanZ"          },
	{ FILE_CHEADKARL,             "CheadkarlZ"             },
	{ FILE_CHEADKARL2,            "Cheadkarl2Z"            },
	{ FILE_CHEADMARTIN,           "CheadmartinZ"           },
	{ FILE_CHEADMARK0,            "Cheadmark0Z"            },
	{ FILE_CHEADDUNCAN0,          "Cheadduncan0Z"          },
	{ FILE_CHEADDUNCAN01,         "Cheadduncan01Z"         },
	{ FILE_CHEADJONES0,           "Cheadjones0Z"           },
	{ FILE_CHEADGRANT0,           "Cheadgrant0Z"           },
	{ FILE_CHEADROBIN0,           "Cheadrobin0Z"           },
	{ FILE_CHEADGRAEME,           "CheadgraemeZ"           },
	{ FILE_CHEADGRAEME2,          "Cheadgraeme2Z"          },
	{ FILE_CHEADSTEVE_E,          "Cheadsteve_eZ"          },
	{ FILE_CHEADKEN0,             "Cheadken0Z"             },
	{ FILE_CHEADMANDY,            "CheadmandyZ"            },
	{ FILE_CHEADVIVIEN,           "CheadvivienZ"           },
	{ FILE_CHEADSALLY,            "CheadsallyZ"            },
	{ FILE_CHEADJOEL0,            "Cheadjoel0Z"            },
	{ FILE_CHEADJOE2,             "Cheadjoe2Z"             },
	{ FILE_CHEADJOE,              "CheadjoeZ"              },
	{ FILE_CHEADSCOTT_H0,         "Cheadscott_h0Z"         },
	{ FILE_CHEADSTEVEH,           "CheadstevehZ"           },
	{ FILE_CHEADJIM,              "CheadjimZ"              },
	{ FILE_CHEADJIM2,             "Cheadjim2Z"             },
	{ FILE_CHEADNEIL0,            "Cheadneil0Z"            },
	{ FILE_CHEADLEE,              "CheadleeZ"              },
	{ FILE_CHEADCHRIS,            "CheadchrisZ"            },
	{ FILE_CHEADCHRIS2,           "Cheadchris2Z"           },
	{ FILE_CHEADDES,              "CheaddesZ"              },
	{ FILE_CHEADSHAUN0,           "Cheadshaun0Z"           },
	{ FILE_CHEADPETE,             "CheadpeteZ"             },
	{ FILE_CHEADALAN,             "CheadalanZ"             },
	{ FILE_CHEADBANDOVERFLOW,     "CheadbandoverflowZ"     },
	{ FILE_CHEADWRECK,            "CheadwreckZ"            },
	{ FILE_CHEADSUBDRAG,          "CheadsubdragZ"          },
	{ FILE_CBONDALPS,             "CbondalpsZ"             },
	{ FILE_GHAND_BONDALPS,        "Ghand_bondalpsZ"        },
	{ FILE_CBONDRANCH,            "CbondranchZ"            },
	{ FILE_GHAND_BONDRANCH,       "Ghand_bondranchZ"       },
	{ FILE_CHEADGALORE,           "CheadgaloreZ"           },
	{ FILE_CGALOREPLANE,          "CgaloreplaneZ"          },
	{ FILE_CGALORERANCH,          "CgaloreranchZ"          },
	{ FILE_GHAND_GALORERANCH,     "Ghand_galoreranchZ"     },
	{ FILE_CHEADSOGUN,            "CheadsogunZ"            },
	{ FILE_CSOGUNTRON,            "CsoguntronZ"            },
	{ FILE_GHAND_SOGUNTRON,       "Ghand_soguntronZ"       },
	{ FILE_CHEADMRKANE,           "CheadmrkaneZ"           },
	{ FILE_CHEADPARIS,            "CheadparisZ"            },
	{ FILE_CCARROLL3,             "Ccarroll3Z"             },
	{ FILE_CNATALYA_XMAS,         "Cnatalya_xmasZ"         },
	{ FILE_CNATALYA_ELF,          "Cnatalya_elfZ"          },
	{ FILE_CHEADDONKEY,           "CheaddonkeyZ"           },
	{ FILE_CHEADCASEYDARK,        "CheadcaseydarkZ"        },
	{ FILE_CCASEYDARK,            "CcaseydarkZ"            },
	{ FILE_CHEADHAMM,             "CheadhammZ"             },
	{ FILE_CEVERETTHAMM,          "CeveretthammZ"          },
	{ FILE_CHEADDRNO,             "CheaddrnoZ"             },
	{ FILE_CHEADWAILIN,           "CheadwailinZ"           },
	{ FILE_CHEADAURIC,            "CheadauricZ"            },
	{ FILE_CHEADELEKTRA,          "CheadelektraZ"          },
	{ FILE_CXENIA_XMAS,           "Cxenia_xmasZ"           },
	{ FILE_GHAND_XENIA_XMAS,      "Ghand_xenia_xmasZ"      },
	{ FILE_CHEADMRX,              "CheadmrxZ"              },
	{ FILE_CMRX,                  "CmrxZ"                  },
	{ FILE_GHAND_MRX,             "Ghand_mrxZ"             },
	{ FILE_CHEADDARK_US,          "Cheaddark_usZ"          }, // Joanna (US)
	{ FILE_CHEADDARK_COMBAT_JP,   "Cheaddark_combat_jpZ"   }, // Joanna (JP)
	{ FILE_CHEADDARK_FROCK_JP,    "Cheaddark_frock_jpZ"    }, // Joanna (JP)
	{ FILE_CHEADDARKAQUA_JP,      "Cheaddarkaqua_jpZ"      }, // Joanna (JP)
	{ FILE_CHEADDARK_SNOW_JP,     "Cheaddark_snow_jpZ"     }, // Joanna (JP)
	{ FILE_CDARK_COMBAT_JP,       "Cdark_combat_jpZ"       }, // Joanna (JP)
	{ FILE_CDARK_FROCK_JP,        "Cdark_frock_jpZ"        }, // Joanna (JP)
	{ FILE_CDARK_RIPPED_JP,       "Cdark_ripped_jpZ"       }, // Joanna (JP)
	{ FILE_CDARK_AF1_JP,          "Cdark_af1_jpZ"          }, // Joanna (JP)
	{ FILE_CDARKSNOW_JP,          "Cdarksnow_jpZ"          }, // Joanna (JP)
	{ FILE_CDARK_LEATHER_JP,      "Cdark_leather_jpZ"      }, // Joanna (JP)
	{ FILE_CDARK_NEGOTIATOR_JP,   "Cdark_negotiator_jpZ"   }, // Joanna (JP)
	{ FILE_GHAND_NATALYA,         "Ghand_natalyaZ"         },
	{ FILE_GHAND_GALOREPLANE,     "Ghand_galoreplaneZ"     },
	{ FILE_GHAND_TECHWOMAN,       "Ghand_techwomanZ"       },
	{ FILE_GHAND_MOONFEMALE,      "Ghand_moonfemaleZ"      },
	{ FILE_GHAND_CILABTECH,       "Ghand_cilabtechZ"       },
	{ FILE_GHAND_TIMBERBOND,      "Ghand_timberbondZ"      },
	{ FILE_GHAND_PRES_SECURITY,   "Ghand_pres_securityZ"   },
	{ FILE_GHAND_NSA_LACKEY,      "Ghand_nsa_lackeyZ"      },
	{ FILE_GHAND_VALENTIN,        "Ghand_valentinZ"        },
	{ FILE_GHAND_CASEYDARK,       "Ghand_caseydarkZ"       },
	{ FILE_GHAND_STEWARD,         "Ghand_stewardZ"         },
	{ FILE_GHAND_ARMOURGUARD,     "Ghand_armourguardZ"     },
	{ FILE_GHAND_LABTECH,         "Ghand_labtechZ"         },
	{ FILE_GHAND_JAWS,            "Ghand_jawsZ"            },
	{ FILE_GHAND_DD_GUARD,        "Ghand_dd_guardZ"        },
	{ FILE_GHAND_PRESIDENT_CLONE, "Ghand_president_cloneZ" },
	{ FILE_CHEADSKEDAR,           "CheadskedarZ"           },
	{ FILE_CHEADCARROLL,          "CheadcarrollZ"          },
	{ FILE_CHEADCARROLL_SINISTER, "Cheadcarroll_sinisterZ" },
	{ FILE_CSILVERWOMAN,          "CsilverwomanZ"          },
	{ FILE_CHEADCHRISTMASJONES,   "CheadchristmasjonesZ"   },
	{ FILE_CJAWS_BLUE,            "Cjaws_blueZ"            },
	{ FILE_GHAND_JAWS_BLUE,       "Ghand_jaws_blueZ"       },
	{ FILE_CFEMSPY_BLU,           "Cfemspy_bluZ"           },
	{ FILE_GHAND_FEMSPY_BLU,      "Ghand_femspy_bluZ"      },
	{ FILE_CHEADCJ,               "CheadcjZ"               },
	{ FILE_CCJ,                   "CcjZ"                   },
	{ FILE_CHEADMACTONIGHT,       "CheadmactonightZ"       },
};

static inline bool romdataIsExternalOnlyFile(s32 fileNum)
{
	for (int i = 0; i < ARRAYCOUNT(expansionFiles); i++) {
		if (expansionFiles[i].fileNum == fileNum) {
			return true;
		}
	}

	return false;
}

static inline void romdataInitFileSlotExpansion(void)
{
	// file slot expansion
	for (int i = 0; i < ARRAYCOUNT(expansionFiles); i++) {
		s32 fileNum = expansionFiles[i].fileNum;

		fileSlots[MOD_NORMAL][fileNum].data = 0;
		fileSlots[MOD_NORMAL][fileNum].size = 0;
		fileSlots[MOD_NORMAL][fileNum].source = SRC_UNLOADED;
		fileSlots[MOD_NORMAL][fileNum].preprocessed = 0;
		fileSlots[MOD_NORMAL][fileNum].name = expansionFiles[i].name;

		for (int modnum = MOD_GEX; modnum < NUM_MODS; modnum++) {
			fileSlots[modnum][fileNum] = fileSlots[MOD_NORMAL][fileNum];
		}
	}
}

static inline void romdataResetFile(s32 fileNum)
{
	// the file offset table is in the data seg
	const u32 *offsets = (u32 *)(romDataSeg + ROMDATA_FILES_OFS);
	if (offsets + fileNum + 1 < (u32 *)(romDataSeg + romDataSegSize)) {
		const u32 nextofs = PD_BE32(offsets[fileNum + 1]);
		const u32 ofs = PD_BE32(offsets[fileNum]);
		// free any external (mod) allocation before repointing data to the ROM
		if (fileSlots[g_ModNum][fileNum].source == SRC_EXTERNAL && fileSlots[g_ModNum][fileNum].data) {
			sysMemFree(fileSlots[g_ModNum][fileNum].data);
		}
		fileSlots[g_ModNum][fileNum].data = g_RomFile + ofs;
		fileSlots[g_ModNum][fileNum].size = nextofs - ofs;
		fileSlots[g_ModNum][fileNum].source = SRC_UNLOADED;
		fileSlots[g_ModNum][fileNum].preprocessed = 0;
	}
}

static inline struct romfile *romdataGetSeg(const char *name)
{
	struct romfile *seg = romSegs;
	while (seg->name && strcmp(name, seg->name)) {
		++seg;
	}
	return seg;
}

s32 romdataInit(void)
{
	const char *altRomName = sysArgGetString("--rom-file");
	if (altRomName) {
		romName = altRomName;
	}

	romdataLoadRom();

	// set segments to point to the rom or load them externally
	for (struct romfile *seg = romSegs; seg->name; ++seg) {
		romdataInitSegment(seg);
	}

	// load file table from the files segment
	romdataInitFiles();

	// file slot expansion
	romdataInitFileSlotExpansion();

	sysLogPrintf(LOG_NOTE, "romdataInit: loaded rom, size = %u", g_RomFileSize);

	return 0;
}

static inline bool romdataCheckGbcRomContents(const u8 *gbcRomFile, const u32 gbcRomSize)
{
	if (gbcRomSize != GBC_ROM_SIZE) {
		return false;
	}

	// ROM title
	if (memcmp(gbcRomFile + 0x134, "PerfDark   VPDE", 15) != 0) {
		return false;
	}

	// Licensee code
	if (memcmp(gbcRomFile + 0x144, "4Y", 2) != 0) {
		return false;
	}

	// Header and global checksums
	if (gbcRomFile[0x14D] != 0xA1 || gbcRomFile[0x14E] != 0xAD || gbcRomFile[0x14F] != 0x0F) {
		return false;
	}

	return true;
}

s32 romdataCheckGbcRom(void)
{
	if (fsFileSize(GBC_ROM_NAME) < 0) {
		// bail early if it doesn't exist to avoid generating error messages
		return false;
	}

	u32 gbcRomSize = 0;
	u8 *gbcRomFile = fsFileLoad(GBC_ROM_NAME, &gbcRomSize);
	if (!gbcRomFile) {
		return false;
	}

	const bool ret = romdataCheckGbcRomContents(gbcRomFile, gbcRomSize);
	sysMemFree(gbcRomFile);

	if (ret) {
		sysLogPrintf(LOG_NOTE, "romdataCheckGbcRom: valid GBC rom found");
	}

	return ret;
}

s32 romdataFileGetSize(s32 fileNum)
{
	if (fileNum < 1 || fileNum >= ROMDATA_MAX_FILES) {
		sysLogPrintf(LOG_ERROR, "romdataFileGetSize: invalid file num %d", fileNum);
		return -1;
	}

	// ensure any external files are loaded and we use their size
	if (romdataFileLoad(fileNum, NULL)) {
		return fileSlots[g_ModNum][fileNum].size;
	}

	sysLogPrintf(LOG_ERROR, "romdataFileGetSize: could not load file num %d", fileNum);
	return -1;
}

u8 *romdataFileGetData(s32 fileNum)
{
	return romdataFileLoad(fileNum, NULL);
}

u8 *romdataFileLoad(s32 fileNum, u32 *outSize)
{
	if (fileNum < 1 || fileNum >= ROMDATA_MAX_FILES) {
		sysLogPrintf(LOG_ERROR, "romdataFileLoad: invalid file num %d", fileNum);
		return NULL;
	}

	u8 *out = NULL;

	// try to load external file
	if (fileSlots[g_ModNum][fileNum].source == SRC_UNLOADED) {
		char tmp[FS_MAXPATH] = { 0 };
		snprintf(tmp, sizeof(tmp), ROMDATA_FILEDIR "/%s", fileSlots[g_ModNum][fileNum].name);

		// All Solos in Multi Mod: in solo/coop/counter-op do not load vanilla-slot mod
		// replacements (stage bg/pads/tiles), but still load external-only expansion files
		// (new custom head/body/hand slots referenced by robot.c), otherwise they resolve
		// to NULL and crash when a solo character uses a custom model (e.g. Ghand_labtechZ).
		if (fsFileSize(tmp) > 0 && (!g_NotLoadMod || romdataIsExternalOnlyFile(fileNum))) {
			u32 size = 0;

			out = fsFileLoad(tmp, &size);

			if (out && size) {
				sysLogPrintf(LOG_NOTE, "file %d (%s) loaded externally (g_ModNum: %d)", fileNum, fileSlots[g_ModNum][fileNum].name, g_ModNum);
				fileSlots[g_ModNum][fileNum].data = out;
				fileSlots[g_ModNum][fileNum].size = size;
				fileSlots[g_ModNum][fileNum].source = SRC_EXTERNAL;
				// external file; do not apply patches to this
				fileSlots[g_ModNum][fileNum].numpatches = 0;
			}
		}

		if (fileSlots[g_ModNum][fileNum].source == SRC_UNLOADED) {
			// if external-only file not found
			if (romdataIsExternalOnlyFile(fileNum)) {
				sysLogPrintf(LOG_ERROR, "romdataFileLoad: external-only file %d (%s) not found", fileNum, fileSlots[g_ModNum][fileNum].name ? fileSlots[g_ModNum][fileNum].name : "(null)");
				return NULL;
			} else {
				// tried and failed, fall back to ROM
				fileSlots[g_ModNum][fileNum].source = SRC_ROM;
			}
		}
	}

	if (!out) {
		out = fileSlots[g_ModNum][fileNum].data;
	}

	if (out && outSize) {
		*outSize = fileSlots[g_ModNum][fileNum].size;
	}

	return out;
}

void romdataFilePreprocess(s32 fileNum, s32 loadType, u8 *data, u32 size, u32 *outSize)
{
	if (fileNum < 1 || fileNum >= ROMDATA_MAX_FILES) {
		sysLogPrintf(LOG_ERROR, "romdataFilePreprocess: invalid file num %d", fileNum);
		return;
	}

	if (data && size /* && !fileSlots[g_ModNum][fileNum].preprocessed*/) {
		if (loadType && loadType < (u32)ARRAYCOUNT(filePreprocFuncs) && filePreprocFuncs[loadType]) {
			// apply patches
			for (u32 i = 0; i < fileSlots[g_ModNum][fileNum].numpatches; ++i) {
				const struct romfilepatch *p = &fileSlots[g_ModNum][fileNum].patches[i];
				if (!memcmp(data + p->ofs, p->src, p->len)) {
					memcpy(data + p->ofs, p->dst, p->len);
					sysLogPrintf(LOG_NOTE, "file %d (%s) patched at offset 0x%x", fileNum, fileSlots[g_ModNum][fileNum].name, p->ofs);
				}
			}
			// then preprocess
			filePreprocFuncs[loadType](data, size, outSize);
			// fileSlots[g_ModNum][fileNum].preprocessed = 1;
		}
	}
}

void romdataFileFree(s32 fileNum)
{
	if (fileNum < 1 || fileNum >= ROMDATA_MAX_FILES) {
		sysLogPrintf(LOG_ERROR, "fsFileFree: invalid file num %d", fileNum);
		return;
	}

	if (fileSlots[g_ModNum][fileNum].source == SRC_EXTERNAL) {
		sysMemFree(fileSlots[g_ModNum][fileNum].data);
		fileSlots[g_ModNum][fileNum].data = NULL;
	}

	fileSlots[g_ModNum][fileNum].source = SRC_UNLOADED;
}

void romdataFileFreeForSolo(void)
{
	// reset files so mods don't affect solo, co-op and counter-op missions
	for (s32 fileNum = 1; fileNum < ROMDATA_MAX_FILES; fileNum++) {
		// exclude additional files
		if (romdataIsExternalOnlyFile(fileNum)) {
			continue;
		}
		// exclude language text files
		if (fileNum >= FILE_LAMEE && fileNum <= FILE_LWAX_STR_I) {
			continue;
		}
		romdataResetFile(fileNum);
	}
}

const char *romdataFileGetName(s32 fileNum)
{
	if (fileNum < 1 || fileNum >= ROMDATA_MAX_FILES) {
		return NULL;
	}
	return fileSlots[g_ModNum][fileNum].name;
}

s32 romdataFileGetNumForName(const char *name)
{
	if (!name || !name[0]) {
		return -1;
	}

	for (s32 i = 0; i < ROMDATA_MAX_FILES; ++i) {
		if (fileSlots[g_ModNum][i].name && !strcmp(fileSlots[g_ModNum][i].name, name)) {
			return i;
		}
	}

	return -1;
}

u8 *romdataSegGetData(const char *segName)
{
	return romdataGetSeg(segName)->data;
}

u8 *romdataSegGetDataEnd(const char *segName)
{
	struct romfile *seg = romdataGetSeg(segName);
	return seg->data + seg->size;
}

u32 romdataSegGetSize(const char *segName)
{
	return romdataGetSeg(segName)->size;
}

u32 romdataFileGetEstimatedSize(const u32 size, const u32 loadtype)
{
#ifdef PLATFORM_64BIT
	switch (loadtype) {
	case LOADTYPE_BG:	   return (u32)(size * 1.1f);
	case LOADTYPE_TILES: return (u32)(size * 1.1f);
	case LOADTYPE_LANG:  return (u32)(size * 1.3f);
	case LOADTYPE_SETUP: return (u32)(size * 1.5f);
	case LOADTYPE_PADS:  return (u32)(size * 1.7f);
	case LOADTYPE_MODEL: return (u32)(size * 1.7f);
	case LOADTYPE_GUN: return (u32)(size * 1.7f);
	default:
		sysLogPrintf(LOG_WARNING, "romdataFileGetEstimatedSize: wrong loadtype %d", loadtype);
	}
#endif
	return size;
}
