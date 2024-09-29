#include <ultra64.h>
#include "constants.h"
#include "constants.h"
#include "game/camdraw.h"
#include "game/cheats.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/filelist.h"
#include "game/bg.h"
#include "game/challenge.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/options.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/fault.h"
#include "lib/joy.h"
#include "lib/snd.h"
#include "lib/mema.h"
#include "data.h"
#include "types.h"

s32 g_FilelistKnownPlugCounts[5];

struct filelist *g_FileLists[MAX_LOCAL_PLAYERS] = { NULL };
bool var80075bd0[] = { true, true, true, true };
bool var80075be0[] = { false, false, false, false };
u32 var80075bf0 = false;

void func0f110bf0(void)
{
	// empty
}

void func0f110bf8(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_FileLists); i++) {
		if (g_FileLists[i] != NULL) {
			memaFree(g_FileLists[i], align16(sizeof(struct filelist)));
			g_FileLists[i] = NULL;
		}
	}
}

/**
 * Allocate and build a file list.
 */
void filelistCreate(s32 listnum, u8 filetype)
{
	if (g_FileLists[listnum] == NULL) {
		bgGarbageCollectRooms(align16(sizeof(struct filelist)), 1);
		g_FileLists[listnum] = memaAlloc(align16(sizeof(struct filelist)));
	}

	g_FileLists[listnum]->timeuntilupdate = 1;
	g_FileLists[listnum]->filetype = filetype;

	if (var80062944 == 0) {
		joySetPfsPollInterval(3);
	}

	var80062944 = 1;
}

s32 filelistFindOrCreate(u8 filetype)
{
	s32 bestindex = -1;
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_FileLists); i++) {
		if (g_FileLists[i]) {
			if (g_FileLists[i]->filetype == filetype) {
				return i;
			}
		} else {
			if (bestindex == -1) {
				bestindex = i;
			}
		}
	}

	if (bestindex >= 0) {
		filelistCreate(bestindex, filetype);
		return bestindex;
	}

	return -1;
}

#if VERSION >= VERSION_NTSC_1_0
void filelistInvalidatePak(s32 device)
{
	g_FilelistKnownPlugCounts[device] = -1;
}
#endif

void filelistsTick(void)
{
	u32 updateall;
	u32 update;
	s32 i;
	static bool doneinit = false;

	if (!doneinit) {
		for (i = 0; i < ARRAYCOUNT(g_FilelistKnownPlugCounts); i++) {
			g_FilelistKnownPlugCounts[i] = -1;
		}

		doneinit = true;
	}

#if VERSION >= VERSION_NTSC_1_0
	for (i = 0, updateall = false; i < ARRAYCOUNT(g_FilelistKnownPlugCounts); i++) {
		if (pak0f1167d8(i) && pakGetPlugCount(i) != g_FilelistKnownPlugCounts[i]) {
			updateall = true;
			g_FilelistKnownPlugCounts[i] = pakGetPlugCount(i);
		}
	}
#else
	for (i = 0, updateall = false; i < ARRAYCOUNT(g_FilelistKnownPlugCounts); i++) {
		s32 plugcount = pakGetPlugCount(i);

		pak0f11698c(i);

		if (pak0f1167d8(i)) {
			plugcount = 0;
		}

		if (g_FilelistKnownPlugCounts[i] != plugcount) {
			updateall = true;
			g_FilelistKnownPlugCounts[i] = plugcount;
		}
	}
#endif

	for (i = 0; i < ARRAYCOUNT(g_FileLists); i++) {
		if (g_FileLists[i] != NULL) {
			g_FileLists[i]->updatedthisframe = false;

			update = updateall;

			if (g_FileLists[i]->timeuntilupdate > 0) {
				g_FileLists[i]->timeuntilupdate--;

				if (g_FileLists[i]->timeuntilupdate == 0) {
					update = true;
				}
			}

			if (var80075bd0[g_FileLists[i]->filetype]) {
				update = true;
			}

			if (update) {
				osSyncPrintf("Rebuilding pakWad %d:\n", i);
				filelistUpdate(g_FileLists[i]);
				g_FileLists[i]->updatedthisframe = true;
			}
		}
	}

	for (i = 0; i < ARRAYCOUNT(var80075bd0); i++) {
		var80075bd0[i] = false;
	}
}

void filelistUpdate(struct filelist *list)
{
	const u32 sp3a88[] = {
		PAKFILETYPE_GAME,
		PAKFILETYPE_MPSETUP,
		PAKFILETYPE_MPPLAYER,
		PAKFILETYPE_CAMERA,
	};

	s32 sp1288[2560];
	u32 spa88[512];
	s8 filedevices[2560];
	s32 i;
	s32 j;
	s32 ret;
	s32 len;

	// Display order means game pak then controller paks
	// Device order means controller paks then game pak (ie. SAVEDEVICE constant order)

	// Mapping of display order to device order
	const s8 dis2dev[] = {
		SAVEDEVICE_GAMEPAK,
		SAVEDEVICE_CONTROLLERPAK1,
		SAVEDEVICE_CONTROLLERPAK2,
		SAVEDEVICE_CONTROLLERPAK3,
		SAVEDEVICE_CONTROLLERPAK4,
	};

	// Mapping of device order to display order
	const s8 dev2dis[] = { 1, 2, 3, 4, /* game pak */ 0 };

	list->numdevices = 0;

	// Iterating in display order (game pak then controller paks)
	for (i = 0, len = 0; i < ARRAYCOUNT(dis2dev); i++) {
		list->unk305[dis2dev[i]] = 0;
		list->devicestartindexes[i] = -1;

		ret = pakGetFileIdsByType(dis2dev[i], sp3a88[list->filetype], spa88);

		if (ret == 0) {
			// No error
			for (j = 0; spa88[j] != 0; j++) {
				sp1288[len] = spa88[j];
				filedevices[len] = dis2dev[i];
				len++;
			}

			list->spacesfree[dis2dev[i]] = 0;

			if (list->filetype == FILETYPE_CAMERA) {
				list->spacesfree[dis2dev[i]] = pakGetNumFreeCameraSpacesInPak(dis2dev[i]);
			}

			list->deviceguids[dis2dev[i]].fileid = 0;
			list->deviceguids[dis2dev[i]].deviceserial = pakGetSerial(dis2dev[i]);
		} else {
			// PFS error?
			list->spacesfree[dis2dev[i]] = -1;

			if (ret == 13) {
				list->timeuntilupdate = 5;
			}
		}
	}

	if (len);

	list->numfiles = 0;

	// Iterating files
	for (i = 0; i < len; i++) {
		struct filelistfile *file = &list->files[list->numfiles];
		s32 ret = pakReadBodyAtGuid(filedevices[i], sp1288[i], file->name, sizeof(file->name));

		if (ret);

		if (ret == 0) {
			// No error
			if (list->devicestartindexes[dev2dis[filedevices[i]]] == -1) {
				list->numdevices++;
				list->devicestartindexes[dev2dis[filedevices[i]]] = list->numfiles;
			}

			file->deviceserial = pakGetSerial(filedevices[i]);
			file->fileid = sp1288[i];

			list->numfiles++;
		} else if (ret == 10) {
			// PFS_ERR_ID_FATAL?
			list->unk305[filedevices[i]]++;

			if (list->unk305[filedevices[i]] >= 2) {
				list->spacesfree[filedevices[i]]++;

				if (list->deviceguids[filedevices[i]].fileid == 0) {
					list->deviceguids[filedevices[i]].fileid = sp1288[i];
					list->deviceguids[filedevices[i]].deviceserial = pakGetSerial(filedevices[i]);
				}
			}
		}
	}
}

void pheadAllocateTextures(s32 playernum, struct perfectheadtexturelist *textures)
{
	s32 i;
	s32 j;
	s32 k;

	if (g_Menus[playernum].fm.headtextures == NULL) {
		if (textures == NULL) {
			g_Menus[playernum].fm.unke40_01 = true;
			bgGarbageCollectRooms(align16(sizeof(struct perfectheadtexturelist)), 1);
			g_Menus[playernum].fm.headtextures = memaAlloc(align16(sizeof(struct perfectheadtexturelist)));
		} else {
			g_Menus[playernum].fm.headtextures = textures;
			g_Menus[playernum].fm.unke40_01 = false;
		}
	}

	if (g_Menus[playernum].fm.headtextures == NULL) {
#if VERSION >= VERSION_NTSC_1_0
		faultAssert("tc != NULL", "gamefile.c", 458);
#else
		faultAssert("tc != NULL", "gamefile.c", 450);
#endif
	}

	for (i = 0; i != ARRAYCOUNT(g_Menus[playernum].fm.headtextures->fileguids); i++) {
		g_Menus[playernum].fm.headtextures->fileguids[i].fileid = 0;
		g_Menus[playernum].fm.headtextures->fileguids[i].deviceserial = 0;
	}

	g_Menus[playernum].fm.headtextures->lastupdated240 = 0;

	g_Menus[playernum].fm.headtextures->selectedtexture.width = 16;
	g_Menus[playernum].fm.headtextures->selectedtexture.height = 16;
	g_Menus[playernum].fm.headtextures->selectedtexture.level = 0;
	g_Menus[playernum].fm.headtextures->selectedtexture.format = G_IM_FMT_I;
	g_Menus[playernum].fm.headtextures->selectedtexture.depth = 0;
	g_Menus[playernum].fm.headtextures->selectedtexture.s = 0;
	g_Menus[playernum].fm.headtextures->selectedtexture.t = 1;
	g_Menus[playernum].fm.headtextures->selectedtexture.unk0b = 0;

	for (j = 0; j < 16; j++) {
		for (k = 0; k < 0x80; k++) {
			g_Menus[playernum].fm.headtextures->unk000[j][k] = k & 0xff;
		}
	}
}

void pheadFreeTextures(s32 playernum)
{
	if (g_Menus[playernum].fm.headtextures != NULL) {
		if (g_Menus[playernum].fm.unke40_01) {
			memaFree(g_Menus[playernum].fm.headtextures, align16(sizeof(struct perfectheadtexturelist)));
		}

		g_Menus[playernum].fm.headtextures = NULL;
	}
}

struct textureconfig *pheadGetTexture(s32 playernum, s32 fileid, u16 deviceserial)
{
	s32 i;
	s32 freeslot = -1;
	s32 indextouse = -1;

	for (i = 0; i < 16; i++) {
		if (g_Menus[playernum].fm.headtextures->fileguids[i].fileid == fileid
				&& g_Menus[playernum].fm.headtextures->fileguids[i].deviceserial == deviceserial) {
			indextouse = i;
			break;
		}

		if (g_Menus[playernum].fm.headtextures->fileguids[i].fileid == 0) {
			if (g_Menus[playernum].fm.headtextures->fileguids[i].deviceserial == 0) {
				freeslot = i;
			}
		}
	}

	if (indextouse == -1) {
		s8 device = pakFindBySerial(deviceserial);

		if (device < 0) {
			return NULL;
		}

		if (freeslot == -1) {
			return NULL;
		}

		if (g_Vars.thisframestart240 - g_Menus[playernum].fm.headtextures->lastupdated240 < 20) {
			return NULL;
		}

		g_Menus[playernum].fm.headtextures->lastupdated240 = g_Vars.thisframestart240;

		func0f15015c(device, fileid, g_Menus[playernum].fm.headtextures->unk000[freeslot]);

		g_Menus[playernum].fm.headtextures->fileguids[freeslot].fileid = fileid;
		g_Menus[playernum].fm.headtextures->fileguids[freeslot].deviceserial = deviceserial;

		indextouse = freeslot;
	}

	if (indextouse == -1) {
		return NULL;
	}

	g_Menus[playernum].fm.headtextures->selectedtexture.textureptr = g_Menus[playernum].fm.headtextures->unk000[indextouse];

	return &g_Menus[playernum].fm.headtextures->selectedtexture;
}
