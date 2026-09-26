#ifndef _IN_MPPLAYERS_H
#define _IN_MPPLAYERS_H

#include "types.h"

// The size of a PAKFILETYPE_MPPLAYER pak body (see pak.c). The full profile
// is serialised into this many bytes by mpplayerfileSaveWad.
#define MPPLAYER_BLOCKSIZE 0x4e

/**
 * PC-port authoritative storage for MP player profiles ($S/mpplayers.bin).
 *
 * The N64 eeprom MP player save only has room for 7-bit mpheadnum/mpbodynum
 * values, which is not enough now that extra characters have been added.
 * Rather than change the eeprom format (which must stay compatible), the full
 * profile is mirrored into mpplayers.bin keyed by the profile's eeprom fileguid
 * (deviceserial + fileid):
 *   - the full-width head/body numbers, and
 *   - the complete profile wad blob (MPPLAYER_BLOCKSIZE bytes).
 *
 * The eeprom pak is kept only as the file-manager list/identity handle; the
 * actual profile data is read back from mpplayers.bin when present. On the
 * first load of a profile that has no entry yet, the eeprom data is migrated
 * in, so subsequent loads come from mpplayers.bin.
 *
 * File format:
 *   [version{1}][numentries{2}]
 *   per entry:
 *     [deviceserial{2}][fileid{4}][mpheadnum{2}][mpbodynum{2}]
 *     [hasblob{1}][blob{MPPLAYER_BLOCKSIZE}]
 */

// Look up the entry for the profile with the given eeprom guid.
// Returns true if an entry exists. Any out pointer may be NULL.
// If blob is non-NULL and the entry has a stored blob, MPPLAYER_BLOCKSIZE bytes
// are copied into it and *hasblob is set to true.
bool mpplayersGetEntry(u16 deviceserial, s32 fileid, u16 *mpheadnum, u16 *mpbodynum, u8 *blob, bool *hasblob);

// Insert or update the entry for a profile and persist the file.
// If blob is non-NULL, MPPLAYER_BLOCKSIZE bytes are stored as the profile data.
void mpplayersSetEntry(u16 deviceserial, s32 fileid, u16 mpheadnum, u16 mpbodynum, const u8 *blob);

// Copy the extended entry from a source profile to a destination profile
// (used when copying a player profile to a new file) and persist the file.
void mpplayersCopyEntry(u16 srcdeviceserial, s32 srcfileid, u16 dstdeviceserial, s32 dstfileid);

// Remove the entry for a profile (used when a profile is deleted) and persist.
void mpplayersDeleteEntry(u16 deviceserial, s32 fileid);

#endif
