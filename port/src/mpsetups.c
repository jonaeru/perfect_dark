#include "types.h"
#include "constants.h"
#include "bss.h"
#include "game/pak.h"
#include "game/filelist.h"
#include "game/menu.h"
#include "game/savebuffer.h"
#include "game/mplayer/mplayer.h"
#include <string.h>
#include "fs.h"
#include "system.h"
#include "mpsetups.h"

/*
MP Setup File Format
	# header
	[version{1}]
	[defaultsetup{1}]
	[numsetups{1}]
	# setups
	[setup_1{80}]
	...
	[setup_n{80}]
 */

#define MPSETUP_VERSION 3

#define MPSETUP_EXPORTDIR "$S/exported/"
#define MPSETUP_FILENAME "mpsetups"
#define MPSETUP_FILENAME_EXP "mpsetups-ext"

#define MPSETUP_OP_DEFAULT 0
#define MPSETUP_OP_IMPORT 1
#define MPSETUP_OP_EXPORT 2

#define MPSETUP_IMPORT_OVERWRITE 0
#define MPSETUP_IMPORT_ADD 1

#define MPSETUP_IMPORT_CONFLICT 0x1313

extern struct menudialogdef g_MpSaveSetupNameMenuDialog;

s16 g_MpCurrentSetup = -1;
struct mpsetupfile g_MpSetupFile;

static struct mpsetupfile g_ImportMpSetupFile;
static u64 g_MpImportExportFilter[2];

static char g_StatusText[128];
static char g_TitleImportExportDialog[20];
static char g_LabelSetDefault[20] = "Set Default\n";

/* extended mpsetup menu definitions */

static MenuItemHandlerResult menuhandlerRenameSetup(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerDeleteSetup(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerImportOrExportSettings(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerImportAction(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerOpenImportExportDialog(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerSelectSetupHandler(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerSetupRename(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerSetupDelete(s32 operation, struct menuitem *item, union handlerdata *data);
static MenuItemHandlerResult menuhandlerSetupSetDefault(s32 operation, struct menuitem *item, union handlerdata *data);

static struct menuitem g_StatusOkMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t)g_StatusText,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_347, // "OK"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_StatusOkDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_OPTIONS_345, // "Cool!"
	g_StatusOkMenuItems,
	NULL,
	MENUDIALOGFLAG_DISABLEBANNER,
	NULL,
};

static struct menuitem g_StatusErrorMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t)g_StatusText,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_347, // "OK"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

/* public */
struct menudialogdef g_StatusErrorDialog = {
		MENUDIALOGTYPE_DANGER,
		L_OPTIONS_277, // "Failed"
		g_StatusErrorMenuItems,
		NULL,
		MENUDIALOGFLAG_DISABLEBANNER,
		NULL,
};

static struct menuitem g_RenameSetupItems[] = {
#if VERSION != VERSION_JPN_FINAL
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t)"Enter the setup name:\n",
		0,
		NULL,
	},
#endif
	{
		MENUITEMTYPE_KEYBOARD,
		18,
		0,
		0,
		1,
		menuhandlerRenameSetup,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_RenameSetupDialog = {
		MENUDIALOGTYPE_DEFAULT,
		(uintptr_t)"Setup Name:\n",
		g_RenameSetupItems,
		NULL,
		MENUDIALOGFLAG_LITERAL_TEXT,
		NULL,
};

static struct menuitem g_DeleteSetupItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t)"Delete Setup?\n",
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_385, // "No"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CENTRE,
		L_OPTIONS_386, // "Yes"
		0,
		menuhandlerDeleteSetup,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_DeleteSetupDialog = {
		MENUDIALOGTYPE_DANGER,
		(uintptr_t)"Delete Setup\n",
		g_DeleteSetupItems,
		NULL,
		MENUDIALOGFLAG_LITERAL_TEXT,
		NULL,
};

static struct menuitem g_ImportExportItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LOCKABLEMINOR | MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		140,
		0x0000004d,
		menuhandlerImportOrExportSettings,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_ImportExportDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t) g_TitleImportExportDialog,
	g_ImportExportItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	NULL,
};

static struct menuitem g_ManageImportExportItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		MPSETUP_OP_IMPORT,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Import Settings\n",
		0,
		menuhandlerOpenImportExportDialog,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		MPSETUP_OP_EXPORT,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Export Settings\n",
		0,
		menuhandlerOpenImportExportDialog,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_ManageImportExportDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t) "Import/Export\n",
	g_ManageImportExportItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	NULL,
};

static struct menuitem g_MpManageSettingsListItems[] = {
	{
		MENUITEMTYPE_LIST,
		0,
		MENUITEMFLAG_LABEL_CUSTOMCOLOUR,
		160,
		0x00000042,
		menuhandlerSelectSetupHandler,
	},
	{ MENUITEMTYPE_END },

};

/* public */
struct menudialogdef g_ManageSettingsDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t) "Manage Settings\n",
	g_MpManageSettingsListItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	&g_ManageImportExportDialog,
};

static struct menuitem g_ManageSetupItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Rename\n",
		0,
		menuhandlerSetupRename,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)"Delete\n",
		0,
		menuhandlerSetupDelete,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_LITERAL_TEXT,
		(uintptr_t)g_LabelSetDefault,
		0,
		menuhandlerSetupSetDefault,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_OPTIONS_213, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_ManageSetupDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)"Manage Setup",
	g_ManageSetupItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	NULL,
};

static struct menuitem g_ImportOverrideItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_LESSLEFTPADDING,
		(uintptr_t) "How to resolve setups\nwith the same name?\n",
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		MPSETUP_IMPORT_ADD,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t) "Add\n",
		0,
		menuhandlerImportAction,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		MPSETUP_IMPORT_OVERWRITE,
		MENUITEMFLAG_LITERAL_TEXT | MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		(uintptr_t) "Overwrite\n",
		0,
		menuhandlerImportAction,
	},
	{ MENUITEMTYPE_END },
};

static struct menudialogdef g_ImportOverrideDialog = {
	MENUDIALOGTYPE_DEFAULT,
	(uintptr_t)"Name Conflicts\n",
	g_ImportOverrideItems,
	NULL,
	MENUDIALOGFLAG_LITERAL_TEXT,
	NULL,
};

/* common utils */

static s32 mpsetupDeserialize(FILE *f, struct mpsetupfile *setupfile)
{
	s32 rx = 0;

	rx += fread(&setupfile->version, sizeof(setupfile->version), 1, f);
	rx += fread(&setupfile->defaultsetup, sizeof(setupfile->defaultsetup), 1, f);
	rx += fread(&setupfile->numsetups, sizeof(setupfile->numsetups), 1, f);

	u32 blocksize = (setupfile->version < 3) ? 80 : MPSETUP_BLOCKSIZE;

	for (int i = 0; i < setupfile->numsetups; ++i) {
		memset(setupfile->setups[i].bytes, 0, MPSETUP_BLOCKSIZE);
		rx += fread(setupfile->setups[i].bytes, blocksize, 1, f);
	}

	return rx;
}

static s32 mpsetupSerialize(FILE *f, struct mpsetupfile *setupfile)
{
	s32 wx = 0;

	wx += fwrite(&setupfile->version, sizeof(setupfile->version), 1, f);
	wx += fwrite(&setupfile->defaultsetup, sizeof(setupfile->defaultsetup), 1, f);
	wx += fwrite(&setupfile->numsetups, sizeof(setupfile->numsetups), 1, f);

	for (int i = 0; i < setupfile->numsetups; ++i) {
		wx += fwrite(setupfile->setups[i].bytes, sizeof(setupfile->setups[i].bytes), 1, f);
	}

	return wx;
}

static FILE *mpsetupOpenFile(bool write, u8 op) {
	const char *filename = fsFullPath("$S/" MPSETUP_FILENAME ".bin");

	if (op == MPSETUP_OP_EXPORT) {
		// create export directory if it doesn't exist
		if (fsFileSize(MPSETUP_EXPORTDIR) < 0) {
			if (fsCreateDir(MPSETUP_EXPORTDIR) != 0) {
				return NULL;
			}
		}
		filename = fsFullPath(MPSETUP_EXPORTDIR MPSETUP_FILENAME_EXP ".bin");
	} else if (op == MPSETUP_OP_IMPORT) {
		// same name as export but different folder
		filename = fsFullPath("$S/" MPSETUP_FILENAME_EXP ".bin");
	}

	FILE *f;

	if (fsFileSize(filename) < 0) {
		// setup file doesn't exist: create one
		f = fsFileOpenWrite(filename);
		fsFileFree(f);
	}

	if (write) {
		f = fsFileOpenWrite(filename);
	} else {
		f = fsFileOpenRead(filename);
	}

	if (f == NULL) {
		sysLogPrintf(LOG_ERROR, "Unable to open mp setup file");
		return NULL;
	}

	return f;
}

static s32 mpsetupSaveFile(u8 op, struct mpsetupfile *setupfile)
{
	FILE *f = mpsetupOpenFile(true, op);
	if (f == NULL) {
		return -1;
	}

	s32 nwritten = mpsetupSerialize(f, setupfile);
	if (nwritten < 1) {
		fsFileFree(f);
		sysLogPrintf(LOG_ERROR, "Unable to write the MP setup file");
		snprintf(g_StatusText, sizeof(g_StatusText), "Unable to write the setup file\n");
		return -1;
	}

	fsFileFree(f);
	return 0;
}

static void mpsetupDeleteByName(struct mpsetupfile *setupfile, const char *name)
{
	for (int i = 0; i < setupfile->numsetups; i++) {
		if (strcmp((char *)setupfile->setups[i].bytes, name) == 0) {
			// Found it! Shift remaining ones up
			for (int j = i; j < setupfile->numsetups - 1; ++j) {
				memcpy(setupfile->setups[j].bytes, setupfile->setups[j+1].bytes, MPSETUP_BLOCKSIZE);
			}
			setupfile->numsetups--;
			i--; // Adjust index because we shifted
		}
	}
}

static void mpsetupInjectPreset(struct mpsetupfile *setupfile, const char *name, u32 options, u8 scenario, u8 stagenum, u8 timelimit, u8 scorelimit, const u8 weapons[6], const u8 bot_bodies[MAX_BOTS], const u8 bot_heads[MAX_BOTS], const u8 bot_teams[MAX_BOTS], const char *bot_names[MAX_BOTS])
{
	extern s32 g_MpWeaponSetNum;

	// 1. Delete if already exists to ensure it is overwritten and moved to the top slot
	mpsetupDeleteByName(setupfile, name);

	// 2. Save current multiplayer state
	struct mpsetup saved_setup;
	struct mpbotconfig saved_bots[MAX_BOTS];
	s32 saved_weaponsetnum = g_MpWeaponSetNum;
	memcpy(&saved_setup, &g_MpSetup, sizeof(g_MpSetup));
	memcpy(saved_bots, g_BotConfigsArray, sizeof(g_BotConfigsArray));

	// 3. Configure the preset
	memset(&g_MpSetup, 0, sizeof(g_MpSetup));
	g_MpWeaponSetNum = WEAPONSET_CUSTOM;
	strncpy(g_MpSetup.name, name, sizeof(g_MpSetup.name) - 1);
	g_MpSetup.options = options;
	g_MpSetup.scenario = scenario;
	g_MpSetup.stagenum = stagenum;
	g_MpSetup.timelimit = timelimit;
	g_MpSetup.scorelimit = scorelimit;

	for (int j = 0; j < 6; j++) {
		g_MpSetup.weapons[j] = weapons[j];
	}

	// Enable Player 1
	g_MpSetup.chrslots = (1 << 0);

	// Configure simulants
	for (int i = 0; i < MAX_BOTS; i++) {
		if (i + 4 < 32) {
			g_MpSetup.chrslots |= (1 << (i + 4));
		}
		
		g_BotConfigsArray[i].type = BOTTYPE_GENERAL;
		g_BotConfigsArray[i].difficulty = BOTDIFF_NORMAL;
		g_BotConfigsArray[i].base.team = bot_teams[i];
		g_BotConfigsArray[i].base.mpbodynum = bot_bodies[i];
		g_BotConfigsArray[i].base.mpheadnum = bot_heads[i];
		if (bot_names && bot_names[i]) {
			strncpy(g_BotConfigsArray[i].base.name, bot_names[i], sizeof(g_BotConfigsArray[i].base.name) - 1);
		} else {
			sprintf(g_BotConfigsArray[i].base.name, "Sim %d", i + 1);
		}
	}

	// 4. Serialize the preset
	struct savebuffer setup;
	savebufferClear(&setup);
	mpsetupfileSaveWad(&setup);

	// 5. Restore the previous state
	memcpy(&g_MpSetup, &saved_setup, sizeof(g_MpSetup));
	memcpy(g_BotConfigsArray, saved_bots, sizeof(g_BotConfigsArray));
	g_MpWeaponSetNum = saved_weaponsetnum;

	// 6. Insert the preset into Slot 0, shifting other presets if necessary
	if (setupfile->numsetups >= MPSETUP_MAXSETUPS) {
		setupfile->numsetups = MPSETUP_MAXSETUPS;
	} else {
		setupfile->numsetups++;
	}

	// Shift existing setups to make room at Slot 0
	for (int i = setupfile->numsetups - 1; i > 0; i--) {
		memcpy(setupfile->setups[i].bytes, setupfile->setups[i - 1].bytes, MPSETUP_BLOCKSIZE);
	}
	
	// Put the new setup in Slot 0
	memcpy(setupfile->setups[0].bytes, setup.bytes, MPSETUP_BLOCKSIZE);

	// 7. Save file to disk
	mpsetupSaveFile(MPSETUP_OP_DEFAULT, setupfile);
}

static void mpsetupInjectCustomPresets(struct mpsetupfile *setupfile)
{
	// Clean up any old presets first so they get replaced fresh
	mpsetupDeleteByName(setupfile, "WAR!");
	mpsetupDeleteByName(setupfile, "VILLA DEFENSE");
	mpsetupDeleteByName(setupfile, "AREA 51 RAID");
	mpsetupDeleteByName(setupfile, "LICENSE TO KILL");
	mpsetupDeleteByName(setupfile, "EXPLOSIVE CHAOS");
	mpsetupDeleteByName(setupfile, "VILLA CAMPAIGN");
	mpsetupDeleteByName(setupfile, "GEX BUNKER");
	mpsetupDeleteByName(setupfile, "GEX FACILITY");
	mpsetupDeleteByName(setupfile, "ZELDA KAKARIKO");
	mpsetupDeleteByName(setupfile, "DARK NOON");

	// Inject presets in reverse order so they appear in correct 1-to-6 order (from Slot 0 to Slot 5)

	// Preset 6: "DARK NOON" (Dark Noon Mod Valley)
	{
		u8 weapons[6] = { MPWEAPON_FALCON2, MPWEAPON_CMP150, MPWEAPON_SHOTGUN, MPWEAPON_K7AVENGER, MPWEAPON_ROCKETLAUNCHER, MPWEAPON_SHIELD };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x0c; // Elvis (g_MpBodies index 0x0c)
				bot_heads[i] = 0x04;  // HEAD_ELVIS (g_MpHeads index 0x04)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Elvis";
			} else {
				bot_bodies[i] = 0x38; // Maian Soldier (g_MpBodies index 0x38)
				bot_heads[i] = 0x14;  // HEAD_MAIAN_S (g_MpHeads index 0x14)
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "Maian";
			}
		}
		// STAGE_TEST_MP7 (0x3f) is Dark Noon Mod Valley
		mpsetupInjectPreset(setupfile, "DARK NOON", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_TEST_MP7, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}

	// Preset 5: "ZELDA KAKARIKO" (The custom Zelda Kakariko Village map)
	{
		u8 weapons[6] = { MPWEAPON_CROSSBOW, MPWEAPON_COMBATKNIFE, MPWEAPON_FALCON2, MPWEAPON_SNIPERRIFLE, MPWEAPON_GRENADE, MPWEAPON_NONE };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x0c; // Elvis (g_MpBodies index 0x0c)
				bot_heads[i] = 0x04;  // HEAD_ELVIS (g_MpHeads index 0x04)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Elvis";
			} else {
				bot_bodies[i] = 0x38; // Maian Soldier (g_MpBodies index 0x38)
				bot_heads[i] = 0x14;  // HEAD_MAIAN_S (g_MpHeads index 0x14)
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "Maian";
			}
		}
		// STAGE_24 (0x24) is Kakariko Village
		mpsetupInjectPreset(setupfile, "ZELDA KAKARIKO", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_24, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}

	// Preset 4: "GEX BUNKER" (Classic GoldenEye Bunker map from GEX Mod)
	{
		u8 weapons[6] = { MPWEAPON_FALCON2, MPWEAPON_CMP150, MPWEAPON_SHOTGUN, MPWEAPON_K7AVENGER, MPWEAPON_GRENADE, MPWEAPON_SHIELD };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x00; // Joanna Dark (g_MpBodies index 0x00)
				bot_heads[i] = 0x00;  // HEAD_DARK_COMBAT (g_MpHeads index 0x00)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Joanna";
			} else {
				bot_bodies[i] = 0x16; // Carrington Guard (g_MpBodies index 0x16)
				bot_heads[i] = 0x00;
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "Guard";
			}
		}
		// STAGE_EXTRA11 (0x10) is the Bunker level from GoldenEye X
		mpsetupInjectPreset(setupfile, "GEX BUNKER", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_EXTRA11, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}

	// Preset 3: "GEX FACILITY" (Classic GoldenEye Facility map from GEX Mod)
	{
		u8 weapons[6] = { MPWEAPON_FALCON2, MPWEAPON_CMP150, MPWEAPON_SHOTGUN, MPWEAPON_LAPTOPGUN, MPWEAPON_GRENADE, MPWEAPON_SHIELD };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x00; // Joanna Dark (g_MpBodies index 0x00)
				bot_heads[i] = 0x00;  // HEAD_DARK_COMBAT (g_MpHeads index 0x00)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Joanna";
			} else {
				bot_bodies[i] = 0x16; // Carrington Guard (g_MpBodies index 0x16)
				bot_heads[i] = 0x00;
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "Guard";
			}
		}
		// STAGE_EXTRA10 (0x0f) is the Facility level from GoldenEye X
		mpsetupInjectPreset(setupfile, "GEX FACILITY", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_EXTRA10, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}

	// Preset 2: "VILLA DEFENSE" (Carrington Villa Campaign map)
	{
		u8 weapons[6] = { MPWEAPON_FALCON2, MPWEAPON_MAGSEC4, MPWEAPON_DY357MAGNUM, MPWEAPON_CYCLONE, MPWEAPON_K7AVENGER, MPWEAPON_SHIELD };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x16; // Carrington Guard (g_MpBodies index 0x16)
				bot_heads[i] = 0x06;  // HEAD_CARRINGTON (g_MpHeads index 0x06)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Guard";
			} else {
				bot_bodies[i] = 0x22; // G5 Swat (g_MpBodies index 0x22)
				bot_heads[i] = 0;
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "G5 Swat";
			}
		}
		// STAGE_VILLA (0x2c) is the actual Carrington Villa Campaign stage
		mpsetupInjectPreset(setupfile, "VILLA DEFENSE", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_VILLA, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}

	// Preset 1: "WAR!" (Skedar Ruins Campaign map with Elvis vs Maian Soldiers)
	{
		u8 weapons[6] = { MPWEAPON_PHOENIX, MPWEAPON_CALLISTO, MPWEAPON_REAPER, MPWEAPON_MAULER, MPWEAPON_SLAYER, MPWEAPON_NONE };
		u8 bot_bodies[MAX_BOTS];
		u8 bot_heads[MAX_BOTS];
		u8 bot_teams[MAX_BOTS];
		const char *bot_names[MAX_BOTS];
		for (int i = 0; i < MAX_BOTS; i++) {
			if (i < MAX_BOTS / 2) {
				bot_bodies[i] = 0x0c; // Elvis (g_MpBodies index 0x0c)
				bot_heads[i] = 0x04;  // HEAD_ELVIS (g_MpHeads index 0x04)
				bot_teams[i] = 0;     // Red
				bot_names[i] = "Elvis";
			} else {
				bot_bodies[i] = 0x38; // Maian Soldier (g_MpBodies index 0x38)
				bot_heads[i] = 0x14;  // HEAD_MAIAN_S (g_MpHeads index 0x14)
				bot_teams[i] = 1;     // Blue
				bot_names[i] = "Maian";
			}
		}
		// STAGE_SKEDARRUINS (0x2a) is the actual Campaign Ruins stage
		mpsetupInjectPreset(setupfile, "WAR!", MPOPTION_TEAMSENABLED, MPSCENARIO_COMBAT, STAGE_SKEDARRUINS, 60, 100, weapons, bot_bodies, bot_heads, bot_teams, bot_names);
	}
}

static s32 mpsetupLoadFile(struct mpsetupfile *setupfile, u8 op)
{
	FILE *f = mpsetupOpenFile(false, op);
	if (f == NULL) {
		// File does not exist! Let's initialize setupfile to empty so we can inject into it
		memset(setupfile, 0, sizeof(*setupfile));
		setupfile->version = MPSETUP_VERSION;
	} else {
		mpsetupDeserialize(f, setupfile);
		fsFileFree(f);
	}

	if (op == MPSETUP_OP_DEFAULT) {
		mpsetupInjectCustomPresets(setupfile);
	}

	if (op == MPSETUP_OP_DEFAULT && setupfile->defaultsetup > 0) {
		mpsetupLoadSetup(setupfile->defaultsetup - 1);
	}

	return 0;
}

static s32 mpsetupImportFile(u8 op, u8 skipOverlap)
{
	if (!skipOverlap) {
		// check for names overlap
		u8 overlap = false;
		for (int i = 0; i < g_ImportMpSetupFile.numsetups; ++i) {
			for (int j = 0; j < g_MpSetupFile.numsetups; ++j) {
				if (strcmp(g_ImportMpSetupFile.setups[i].bytes, g_MpSetupFile.setups[j].bytes) == 0) {
					overlap = true;
					return MPSETUP_IMPORT_CONFLICT;
				}
			}
		}
	}

	for (int i = 0; i < g_ImportMpSetupFile.numsetups; ++i) {
		s16 overlapIdx = -1;
		for (int j = g_MpSetupFile.numsetups-1; j >= 0; --j) {
			if (strcmp(g_ImportMpSetupFile.setups[i].bytes, g_MpSetupFile.setups[j].bytes) == 0) {
				overlapIdx = j;
				break;
			}
		}

		s16 importIdx = overlapIdx;
		if (overlapIdx < 0 || op == MPSETUP_IMPORT_ADD) {
			if (g_MpSetupFile.numsetups == MPSETUP_MAXSETUPS) {
				snprintf(g_StatusText, sizeof(g_StatusText), "Number of setups exceeds %d\n", MPSETUP_MAXSETUPS);
				return -1;
			}
			importIdx = g_MpSetupFile.numsetups++;
		}

		memcpy(g_MpSetupFile.setups[importIdx].bytes, g_ImportMpSetupFile.setups[i].bytes, MPSETUP_BLOCKSIZE);
	}

	return mpsetupSaveCurrentFile();
}

static s32 mpsetupExportFile(void)
{
	struct mpsetupfile expMpSetupFile;
	expMpSetupFile.numsetups = 0;
	expMpSetupFile.defaultsetup = 0;
	expMpSetupFile.version = MPSETUP_VERSION;

	u8 maxsetups = MPSETUP_MAXSETUPS;
	maxsetups = MIN(maxsetups, g_MpSetupFile.numsetups);
	for (int i = 0; i < maxsetups; ++i) {
		u8 bank = i < 64 ? 0 : 1;
		u8 bit = i - bank * 64;
		if (g_MpImportExportFilter[bank] & (1 << bit)) {
			u8 n = expMpSetupFile.numsetups;
			expMpSetupFile.numsetups++;
			char *name = g_MpSetupFile.setups[i].bytes;
			memcpy(expMpSetupFile.setups[n].bytes, g_MpSetupFile.setups[i].bytes, MPSETUP_BLOCKSIZE);
			expMpSetupFile.numsetups = n + 1;
		}
	}

	s32 err = mpsetupSaveFile(MPSETUP_OP_EXPORT, &expMpSetupFile);
	if (err) {
		snprintf(g_StatusText, sizeof(g_StatusText), "Unable to write\nsetup file\n");
	}

	return err;
}

static s32 mpsetupDelete(void)
{
	s32 slotindex = g_Menus[g_MpPlayerNum].mpsetup.slotindex;
	for (int i = slotindex; i < g_MpSetupFile.numsetups - 1; ++i) {
		u8* dst = g_MpSetupFile.setups[i].bytes;
		u8* src = g_MpSetupFile.setups[i+1].bytes;
		memcpy(dst, src, MPSETUP_BLOCKSIZE);
	}

	if (g_MpCurrentSetup > slotindex) {
		g_MpCurrentSetup--;
	}
	else if (g_MpCurrentSetup == slotindex) {
		g_MpCurrentSetup = -1;
	}

	if (g_MpSetupFile.defaultsetup > slotindex + 1) {
		 g_MpSetupFile.defaultsetup--;
	}
	else if (g_MpSetupFile.defaultsetup == slotindex + 1) {
		g_MpSetupFile.defaultsetup = 0;
	}

	g_MpSetupFile.numsetups--;
	return mpsetupSaveCurrentFile();
}

/* menu handlers */

// Rename setup dialog
static MenuItemHandlerResult menuhandlerRenameSetup(s32 operation, struct menuitem *item, union handlerdata *data)
{
	char *name = data->keyboard.string;
	s32 slotindex = g_Menus[g_MpPlayerNum].mpsetup.slotindex;
	struct setupblock *setup = &g_MpSetupFile.setups[slotindex];
	s32 err;

	switch (operation) {
	case MENUOP_GETTEXT:
		strcpy(name, setup->bytes);
		break;
	case MENUOP_SETTEXT:
		strcpy(g_MpSetup.name, name);
		break;
	case MENUOP_SET:
		strcpy(setup->bytes, name);
		err = mpsetupSaveSetup(slotindex, true);
		if (!err) {
			menuPopDialog();
		} else {
			// TODO
			// menuPushDialog(&g_FilemgrFileSavedMenuDialog);
		}
		break;
	}

	return 0;
}

// Delete setup dialog
static MenuItemHandlerResult menuhandlerDeleteSetup(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 err = mpsetupDelete();
		if (!err) {
			menuPopDialog();
			menuPopDialog();
		} else {
			// TODO
		}
	}

	return 0;
}

// Import (or Export) settings dialog
static MenuItemHandlerResult menuhandlerImportOrExportSettings(s32 operation, struct menuitem *item, union handlerdata *data)
{
	static const char *labels[] = {
		"Select All",
		"Select None",
		"Import",
		"Export",
	};

	s32 op = g_Menus[g_MpPlayerNum].mpsetup.unke24;

	struct mpsetupfile *setupfile = op == MPSETUP_OP_IMPORT ? &g_ImportMpSetupFile : &g_MpSetupFile;

	u8 numsetups = setupfile->numsetups;
	u8 numitems = numsetups + 3;
	u8 bank = data->list.value < 64 ? 0 : 1;
	u8 bit = data->list.value - bank * 64;
	u8 hasselection = (g_MpImportExportFilter[0] | g_MpImportExportFilter[1]) != 0;
	u8 ofs;
	s32 err;
	u32 colour;
	s32 index;

	switch (operation) {
	case MENUOP_GETCOLOUR:
		colour = data->list.unk04;
		if (data->list.value == numitems - 1) {
			data->list.unk04 = hasselection ? colour : (colour & 0xffffff00) | 0x66;
		}
		break;

	case MENUOP_GETOPTIONCOUNT:
		data->list.value = numitems;
		break;

	case MENUOP_GETOPTIONTEXT:
		if (data->list.value < numsetups) {
			return (uintptr_t) setupfile->setups[data->list.value].bytes;
		} else {
			// creates an offset to select the "Export" or "Import" label for the last item
			ofs = (data->list.value == numitems - 1) ? op - 1 : 0;
			return (intptr_t)labels[data->list.value - numsetups + ofs];
		}

	case MENUOP_SET:
		if (data->list.value < setupfile->numsetups) {
			g_MpImportExportFilter[bank] ^= (1 << bit);
		} else {
			index = data->list.value - setupfile->numsetups;

			switch (index) {
			// Select All
			case 0:
				g_MpImportExportFilter[0] = g_MpImportExportFilter[1] = -1;
				break;
			// Select None
			case 1:
				g_MpImportExportFilter[0] = g_MpImportExportFilter[1] = 0;
				break;
			// Export (or Import)
			case 2:
				if (hasselection) {
					err = op == MPSETUP_OP_IMPORT ? mpsetupImportFile(0, false) : mpsetupExportFile();
					if (!err) {
						menuPopDialog();
						if (op == MPSETUP_OP_IMPORT) {
							// back to the 'Manage Settings' screen
							menuPopDialog();
							menuPushDialog(&g_ManageSettingsDialog);
						} else {
							snprintf(g_StatusText, sizeof(g_StatusText), "File %s.bin\nwritten to the folder 'exported'\n", MPSETUP_FILENAME_EXP);
							menuPushDialog(&g_StatusOkDialog);
						}
					} else {
						if (err == MPSETUP_IMPORT_CONFLICT) {
							menuPopDialog();
							menuPushDialog(&g_ImportOverrideDialog);
						} else {
							menuPushDialog(&g_StatusErrorDialog);
						}
					}
				}
				break;
			}
		}
		break;

	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0x000fffff;
		break;

	case MENUOP_GETLISTITEMCHECKBOX:
		if (data->list.value < numsetups) {
			data->list.unk04 = (g_MpImportExportFilter[bank] & (1 << bit));
		}
		break;
	}

	return 0;
}

// Import: overwrite dialog
static MenuItemHandlerResult menuhandlerImportAction(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 err = mpsetupImportFile(item->param, true);
		if (!err) {
			menuPopDialog();
			menuPushDialog(&g_ManageSettingsDialog);
		} else {
			menuPushDialog(&g_StatusErrorDialog);
		}
	}

	return 0;
}

// Manage Import/Export settings dialog
static MenuItemHandlerResult menuhandlerOpenImportExportDialog(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_SET:
		if (item->param == MPSETUP_OP_IMPORT) {
			strcpy(g_TitleImportExportDialog, "Import Settings\n");
			if (fsFileSize("$S/" MPSETUP_FILENAME_EXP ".bin") < 0) {
				snprintf(
					g_StatusText, sizeof(g_StatusText),
					"No import file found.\n"
					"Place the file %s.bin\n"
					"Next to your %s.bin file\n",
					MPSETUP_FILENAME_EXP, MPSETUP_FILENAME
				);
				menuPushDialog(&g_StatusErrorDialog);
				return 0;
			}

			mpsetupLoadFile(&g_ImportMpSetupFile, MPSETUP_OP_IMPORT);
		} else {
			strcpy(g_TitleImportExportDialog, "Export Settings\n");
		}
		g_Menus[g_MpPlayerNum].mpsetup.unke24 = item->param;
		g_MpImportExportFilter[0] = g_MpImportExportFilter[1] = -1;
		menuPushDialog(&g_ImportExportDialog);
		break;
	}

	return 0;
}

// Manage settings dialog
static MenuItemHandlerResult menuhandlerSelectSetupHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	u32 colour;
	switch (operation) {
	case MENUOP_GETCOLOUR:
		colour = data->list.unk04;
		if (data->list.value + 1 == g_MpSetupFile.defaultsetup) {
			data->list.unk04 = (colour & 0xffff0000) | 0xff;
		}
		break;
	case MENUOP_GETOPTIONCOUNT:
		data->list.value = g_MpSetupFile.numsetups;
		break;
	case MENUOP_GETOPTIONTEXT:
		return (uintptr_t) g_MpSetupFile.setups[data->list.value].bytes;
	case MENUOP_SET:
		g_Menus[g_MpPlayerNum].mpsetup.slotindex = data->list.value;
		if (data->list.value == g_MpSetupFile.defaultsetup - 1) {
			strcpy(g_LabelSetDefault, "Clear Default\n");
		} else {
			strcpy(g_LabelSetDefault, "Set Default\n");
		}
		menuPushDialog(&g_ManageSetupDialog);
		break;
	case MENUOP_GETSELECTEDINDEX:
		data->list.value = 0xfffff;
		break;
	}

	return 0;
}

static MenuItemHandlerResult menuhandlerSetupRename(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPushDialog(&g_RenameSetupDialog);
	}

	return 0;
}

static MenuItemHandlerResult menuhandlerSetupDelete(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		menuPushDialog(&g_DeleteSetupDialog);
	}

	return 0;
}

static MenuItemHandlerResult menuhandlerSetupSetDefault(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 selected = g_Menus[g_MpPlayerNum].mpsetup.slotindex;
		// clicked on "clear default"
		if (selected == g_MpSetupFile.defaultsetup - 1) {
			g_MpSetupFile.defaultsetup = 0;
			strcpy(g_LabelSetDefault, "Set Default\n");
		} else {
			g_MpSetupFile.defaultsetup = selected + 1;
			strcpy(g_LabelSetDefault, "Clear Default\n");
		}

		mpsetupSaveCurrentFile();
	}

	return 0;
}

/* public functions */

s32 mpsetupLoadCurrentFile(void)
{
	return mpsetupLoadFile(&g_MpSetupFile, MPSETUP_OP_DEFAULT);
}

s32 mpsetupSaveCurrentFile(void)
{
	g_MpSetupFile.version = MPSETUP_VERSION;
	return mpsetupSaveFile(MPSETUP_OP_DEFAULT, &g_MpSetupFile);
}

s32 mpsetupSaveSetup(s32 slotindex, u8 savefile)
{
	struct savebuffer setup;

	// request to add a new setup
	if (slotindex == g_MpSetupFile.numsetups) {
		g_MpCurrentSetup = g_MpSetupFile.numsetups++;
	}

	savebufferClear(&setup);
	mpsetupfileSaveWad(&setup);

	memcpy(g_MpSetupFile.setups[slotindex].bytes, setup.bytes, MPSETUP_BLOCKSIZE);

	return savefile ? mpsetupSaveCurrentFile() : 0;
}

void mpsetupLoadSetup(s32 index)
{
	struct savebuffer buffer;
	savebufferClear(&buffer);
	struct setupblock *block = &g_MpSetupFile.setups[index];
	memcpy(&buffer.bytes, block->bytes, MPSETUP_BLOCKSIZE);
	mpsetupfileLoadWad(&buffer, g_MpSetupFile.version);
	g_MpCurrentSetup = index;
}

void mpsetupCopyAllFromPak(void)
{
	if (fsFileSize("$S/" MPSETUP_FILENAME ".bin") > 0) {
		return;
	}

	filelistCreate(1, FILETYPE_MPSETUP);
	filelistsTick();

	for (int i = 0; i < g_FileLists[1]->numfiles; ++i) {
		struct savebuffer buffer;
		savebufferClear(&buffer);
		struct filelistfile *file = &g_FileLists[1]->files[i];
		s32 device = pakFindBySerial(file->deviceserial);
		s32 err = pakReadBodyAtGuid(device, file->fileid, buffer.bytes, 0);

		if (err != 0) {
			sysLogPrintf(LOG_ERROR, "Unable to read pak. device %d fileid %d deviceserial %d err %d",
				device, file->fileid, file->deviceserial, err);
			continue;
		}

		mpsetupfileLoadWad(&buffer, 0);

		// save the file when writing the last setup
		u8 savefile = i == g_FileLists[1]->numfiles - 1;
		mpsetupSaveSetup(g_MpSetupFile.numsetups, savefile);
	}

	// to reset the mp setup
	mpInit(false);
}
