#ifndef IN_GAME_MPLAYER_MPLAYER_H
#define IN_GAME_MPLAYER_MPLAYER_H
#include <ultra64.h>
#include "types.h"

f32 func0f187770(u8 value);
u32 func0f187838(void);
void mpStartMatch(void);
void func0f187944(void);
void func0f187e70(void);
void func0f187fbc(s32 playernum);
void func0f187fec(void);
void func0f18800c(s32 playernum, bool arg1);
void func0f1881d4(s32 index);
void mpSetDefaultSetup(void);
void mpSetDefaultNamesIfEmpty(void);
u32 func0f18844c(void);
void mpApplyLimits(void);
s32 mpGetPlayerRankings(struct mpteaminfo *info);
u32 func0f188930(void);
s32 mpGetTeamRankings(struct mpteaminfo *info);
s32 func0f188bcc(void);
u32 mpGetNumWeaponOptions(void);
char *mpGetWeaponLabel(s32 weaponnum);
void mpSetWeaponSlot(s32 slot, s32 mpweaponnum);
s32 mpGetWeaponSlot(s32 slot);
struct mpweapon *func0f188e24(s32 arg0);
s32 mpCountWeaponSetThing(s32 weaponsetindex);
s32 func0f188f9c(s32 arg0);
s32 func0f189058(s32 arg0);
u32 func0f189088(void);
char *mpGetWeaponSetName(s32 arg0);
u32 func0f18913c(void);
u32 func0f1892dc(void);
void mpSetWeaponSet(s32 weaponsetnum);
void func0f1895e8(void);
s32 mpGetWeaponSet(void);
bool mpIsPaused(void);
void mpSetPaused(u8 mode);
Gfx *func0f18973c(Gfx *gdl);
u32 func0f189cc0(void);
u32 func0f189dc8(void);
u32 func0f189ed0(void);
u32 func0f18a030(void);
void mpCalculatePlayerTitle(struct mpplayer *mpplayer);
void func0f18a56c(void);
void mpEndMatch(void);
s32 func0f18bb1c(void);
s32 mpGetNumHeads(void);
s32 mpGetHeadId(u8 headnum);
s32 mpGetHeadRequiredFeature(u8 headnum);
s32 mpGetBeauHeadId(u8 headnum);
u32 mpGetNumBodies(void);
s32 mpGetBodyId(u8 bodynum);
s32 mpGetMpbodynumByBodynum(u16 bodynum);
char *mpGetBodyName(u8 bodynum);
u8 mpGetBodyRequiredFeature(u8 bodynum);
s32 mpGetMpheadnumByMpbodynum(s32 bodynum);
u32 mpChooseRandomLockPlayer(void);
bool mpSetLock(s32 locktype, s32 playernum);
s32 mpGetLockType(void);
u32 mpGetLockPlayerNum(void);
bool mpIsPlayerLockedOut(u32 playernum);
void mpCalculateLockIfLastWinnerOrLoser(void);
bool mpIsTrackUnlocked(s32 tracknum);
s32 mpGetTrackSlotIndex(s32 tracknum);
s32 mpGetTrackNumAtSlotIndex(s32 slotindex);
s32 func0f18c200(void);
s32 mpGetTrackMusicNum(s32 slotindex);
char *mpGetTrackName(s32 slotindex);
void mpSetUsingMultipleTunes(bool enable);
bool mpGetUsingMultipleTunes(void);
bool mpIsMultiTrackSlotEnabled(s32 slot);
void mpSetMultiTrackSlotEnabled(s32 slot, bool enable);
void mpSetTrackSlotEnabled(s32 slot);
void mpEnableAllMultiTracks(void);
void mpDisableAllMultiTracks(void);
void mpRandomiseMultiTracks(void);
void mpSetTrackToRandom(void);
s32 mpGetCurrentTrackSlotNum(void);
u32 func0f18c4c0(void);
struct mpchr *func0f18c794(s32 index);
u32 func0f18c828(void);
s32 mpGetNumChrs(void);
u32 func0f18c8b8(void);
u32 func0f18c984(void);
u32 func0f18cb60(void);
s32 mpGetNumSimulants(void);
void mpRemoveSimulant(s32 index);
bool mpHasSimulants(void);
u32 func0f18cc8c(void);
bool mpIsSimSlotEnabled(s32 slot);
s32 mpGetSimTypeIndex(s32 type, s32 difficulty);
u32 func0f18cddc(void);
s32 mpPlayerGetIndex(struct chrdata *chr);
struct chrdata *mpGetChrFromPlayerIndex(s32 index);
s32 func0f18d074(s32 index);
s32 func0f18d0e8(s32 arg0);
u32 func0f18d1b8(void);
u32 func0f18d238(void);
u32 func0f18d2b8(void);
u32 func0f18d5c4(void);
void func0f18d9a4(char *arg0, char *buffer, u32 *arg2);
s32 func0f18d9fc(void *arg0, s32 arg1, s32 arg2, u16 arg3);
s32 func0f18dac0(void *arg0, s32 arg1, s32 arg2, u16 arg3);
s32 mpGetNumPresets(void);
bool mpIsPresetUnlocked(s32 presetnum);
s32 mpGetNumUnlockedPresets(void);
char *mpGetPresetNameBySlot(s32 slot);
void func0f18dcec(struct mpconfigfull *mpconfig);
u32 func0f18dec4(void);
u32 func0f18df5c(void);
u32 func0f18e16c(void);
u32 func0f18e39c(void);
s32 func0f18e420(s32 arg0, s32 arg1, u16 arg2);
s32 func0f18e4c8(s32 arg0, s32 arg1, u16 arg2);
void func0f18e558(void);
u32 func0f18e57c(u32 index, u32 arg1);

#endif
