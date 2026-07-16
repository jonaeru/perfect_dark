#include "types.h"
#include <string.h>
#include "fs.h"
#include "system.h"
#include "mpplayers.h"

/*
MP Player File Format ($S/mpplayers.bin)
	# header
	[version{1}]
	[numentries{2}]
	# entries
	[entry_1]
	...
	[entry_n]

	# entry layout
	[deviceserial{2}]
	[fileid{4}]
	[mpheadnum{2}]
	[mpbodynum{2}]
	[hasblob{1}]
	[blob{MPPLAYER_BLOCKSIZE}]

This is the authoritative store for MP player profiles. The eeprom pak is kept
only as the file-manager list/identity handle; the full profile (the wad blob)
plus the full-width head/body numbers live here, keyed by the eeprom fileguid.
The eeprom save format itself is unchanged for compatibility.
*/

#define MPPLAYERS_VERSION 1
#define MPPLAYERS_FILENAME "$S/mpplayers.bin"
#define MPPLAYERS_MAXENTRIES 1024

struct mpplayerentry {
	u16 deviceserial;
	s32 fileid;
	u16 mpheadnum;
	u16 mpbodynum;
	u8 hasblob;
	u8 blob[MPPLAYER_BLOCKSIZE];
};

struct mpplayersfile {
	u8 version;
	u16 numentries;
	struct mpplayerentry entries[MPPLAYERS_MAXENTRIES];
};

static struct mpplayersfile g_MpPlayersFile;
static bool g_MpPlayersLoaded = false;

static bool mpplayersGuidIsValid(u16 deviceserial, s32 fileid)
{
	return deviceserial != 0 && fileid != 0;
}

static s32 mpplayersDeserialize(FILE *f)
{
	g_MpPlayersFile.version = 0;
	g_MpPlayersFile.numentries = 0;

	if (fread(&g_MpPlayersFile.version, sizeof(g_MpPlayersFile.version), 1, f) != 1) {
		return -1;
	}

	if (fread(&g_MpPlayersFile.numentries, sizeof(g_MpPlayersFile.numentries), 1, f) != 1) {
		g_MpPlayersFile.numentries = 0;
		return -1;
	}

	if (g_MpPlayersFile.numentries > MPPLAYERS_MAXENTRIES) {
		g_MpPlayersFile.numentries = MPPLAYERS_MAXENTRIES;
	}

	for (u16 i = 0; i < g_MpPlayersFile.numentries; i++) {
		struct mpplayerentry *e = &g_MpPlayersFile.entries[i];
		memset(e, 0, sizeof(*e));
		fread(&e->deviceserial, sizeof(e->deviceserial), 1, f);
		fread(&e->fileid, sizeof(e->fileid), 1, f);
		fread(&e->mpheadnum, sizeof(e->mpheadnum), 1, f);
		fread(&e->mpbodynum, sizeof(e->mpbodynum), 1, f);
		fread(&e->hasblob, sizeof(e->hasblob), 1, f);
		fread(e->blob, 1, MPPLAYER_BLOCKSIZE, f);
	}

	return 0;
}

static s32 mpplayersSerialize(FILE *f)
{
	g_MpPlayersFile.version = MPPLAYERS_VERSION;

	fwrite(&g_MpPlayersFile.version, sizeof(g_MpPlayersFile.version), 1, f);
	fwrite(&g_MpPlayersFile.numentries, sizeof(g_MpPlayersFile.numentries), 1, f);

	for (u16 i = 0; i < g_MpPlayersFile.numentries; i++) {
		struct mpplayerentry *e = &g_MpPlayersFile.entries[i];
		fwrite(&e->deviceserial, sizeof(e->deviceserial), 1, f);
		fwrite(&e->fileid, sizeof(e->fileid), 1, f);
		fwrite(&e->mpheadnum, sizeof(e->mpheadnum), 1, f);
		fwrite(&e->mpbodynum, sizeof(e->mpbodynum), 1, f);
		fwrite(&e->hasblob, sizeof(e->hasblob), 1, f);
		fwrite(e->blob, 1, MPPLAYER_BLOCKSIZE, f);
	}

	return 0;
}

static void mpplayersLoadFile(void)
{
	if (g_MpPlayersLoaded) {
		return;
	}

	g_MpPlayersLoaded = true;
	g_MpPlayersFile.version = MPPLAYERS_VERSION;
	g_MpPlayersFile.numentries = 0;

	const char *filename = fsFullPath(MPPLAYERS_FILENAME);

	if (fsFileSize(filename) < 0) {
		// No file yet; start with an empty set.
		return;
	}

	FILE *f = fsFileOpenRead(filename);
	if (f == NULL) {
		sysLogPrintf(LOG_ERROR, "Unable to open mp player file for reading");
		return;
	}

	mpplayersDeserialize(f);
	fsFileFree(f);
}

static void mpplayersSaveFile(void)
{
	const char *filename = fsFullPath(MPPLAYERS_FILENAME);

	FILE *f = fsFileOpenWrite(filename);
	if (f == NULL) {
		sysLogPrintf(LOG_ERROR, "Unable to open mp player file for writing");
		return;
	}

	mpplayersSerialize(f);
	fsFileFree(f);
}

static struct mpplayerentry *mpplayersFind(u16 deviceserial, s32 fileid)
{
	for (u16 i = 0; i < g_MpPlayersFile.numentries; i++) {
		struct mpplayerentry *e = &g_MpPlayersFile.entries[i];
		if (e->deviceserial == deviceserial && e->fileid == fileid) {
			return e;
		}
	}

	return NULL;
}

bool mpplayersGetEntry(u16 deviceserial, s32 fileid, u16 *mpheadnum, u16 *mpbodynum, u8 *blob, bool *hasblob)
{
	if (hasblob) {
		*hasblob = false;
	}

	if (!mpplayersGuidIsValid(deviceserial, fileid)) {
		return false;
	}

	mpplayersLoadFile();

	struct mpplayerentry *e = mpplayersFind(deviceserial, fileid);
	if (e == NULL) {
		return false;
	}

	if (mpheadnum) {
		*mpheadnum = e->mpheadnum;
	}

	if (mpbodynum) {
		*mpbodynum = e->mpbodynum;
	}

	if (e->hasblob) {
		if (blob) {
			memcpy(blob, e->blob, MPPLAYER_BLOCKSIZE);
		}

		if (hasblob) {
			*hasblob = true;
		}
	}

	return true;
}

void mpplayersSetEntry(u16 deviceserial, s32 fileid, u16 mpheadnum, u16 mpbodynum, const u8 *blob)
{
	if (!mpplayersGuidIsValid(deviceserial, fileid)) {
		return;
	}

	mpplayersLoadFile();

	struct mpplayerentry *e = mpplayersFind(deviceserial, fileid);

	if (e == NULL) {
		if (g_MpPlayersFile.numentries >= MPPLAYERS_MAXENTRIES) {
			sysLogPrintf(LOG_ERROR, "mp player file is full (%d entries)", MPPLAYERS_MAXENTRIES);
			return;
		}

		e = &g_MpPlayersFile.entries[g_MpPlayersFile.numentries++];
		memset(e, 0, sizeof(*e));
		e->deviceserial = deviceserial;
		e->fileid = fileid;
	} else if (e->mpheadnum == mpheadnum && e->mpbodynum == mpbodynum
			&& (blob == NULL
				? !e->hasblob
				: (e->hasblob && memcmp(e->blob, blob, MPPLAYER_BLOCKSIZE) == 0))) {
		// No change; avoid an unnecessary write.
		return;
	}

	e->mpheadnum = mpheadnum;
	e->mpbodynum = mpbodynum;

	if (blob) {
		e->hasblob = true;
		memcpy(e->blob, blob, MPPLAYER_BLOCKSIZE);
	}

	mpplayersSaveFile();
}

void mpplayersCopyEntry(u16 srcdeviceserial, s32 srcfileid, u16 dstdeviceserial, s32 dstfileid)
{
	u16 head;
	u16 body;
	bool hasblob = false;
	u8 blob[MPPLAYER_BLOCKSIZE];

	if (!mpplayersGetEntry(srcdeviceserial, srcfileid, &head, &body, blob, &hasblob)) {
		// Nothing to copy; make sure no stale entry lingers on the dest.
		mpplayersDeleteEntry(dstdeviceserial, dstfileid);
		return;
	}

	mpplayersSetEntry(dstdeviceserial, dstfileid, head, body, hasblob ? blob : NULL);
}

void mpplayersDeleteEntry(u16 deviceserial, s32 fileid)
{
	if (!mpplayersGuidIsValid(deviceserial, fileid)) {
		return;
	}

	mpplayersLoadFile();

	struct mpplayerentry *e = mpplayersFind(deviceserial, fileid);
	if (e == NULL) {
		return;
	}

	u16 index = (u16)(e - g_MpPlayersFile.entries);

	for (u16 i = index; i + 1 < g_MpPlayersFile.numentries; i++) {
		g_MpPlayersFile.entries[i] = g_MpPlayersFile.entries[i + 1];
	}

	g_MpPlayersFile.numentries--;

	mpplayersSaveFile();
}
