#ifndef _IN_GAME_PLAYER_H
#define _IN_GAME_PLAYER_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

f32 playerChooseSpawnLocation(f32 chrradius, struct coord *dstpos, RoomNum *dstrooms, struct prop *prop, s16 *spawnpads, s32 numspawnpads);
f32 playerChooseGeneralSpawnLocation(f32 chrradius, struct coord *pos, RoomNum *rooms, struct prop *prop);
void playerStartNewLife(void);
void playerLoadDefaults(void);
bool playerSpawnAnti(struct chrdata *chr, s32 param_2);
void playerSpawn(void);
void playerResetBond(struct playerbond *pb, struct coord *pos);
void playersTickAllChrBodies(void);
void playerChooseBodyAndHead(s32 *bodynum, s32 *headnum, s32 *arg2);
void playerTickChrBody(void);
void playerRemoveChrBody(void);
void playerTickMpSwirl(void);
void playerExecutePreparedWarp(void);
void playerStartCutscene(s16 anim_id);
void playerReorientForCutsceneStop(s32 tweenduration60);
void playerTickCutscene(bool arg0);
f32 playerGetCutsceneBlurFrac(void);
void playerSetZoomFovY(f32 fovy, f32 timemax);
f32 playerGetZoomFovY(void);
void playerTweenFovY(f32 targetfovy);
f32 playerGetTeleportFovY(void);
void playerUpdateZoom(void);
void playerStopAudioForPause(void);
void playerTickPauseMenu(void);
void playerPause(s32 root);
void playerUnpause(void);
Gfx *player0f0baf84(Gfx *gdl);
Gfx *playerDrawFade(Gfx *gdl, u32 r, u32 g, u32 b, f32 frac);
Gfx *playerDrawStoredFade(Gfx *gdl);
void playerUpdateColourScreenProperties(void);
void playerTickChrFade(void);
void playerDisplayHealth(void);
void playerTickDamageAndHealth(void);
void playerDisplayDamage(void);
Gfx *playerRenderHealthBar(Gfx *gdl);
void playerSurroundWithExplosions(s32 arg0);
void playerTickExplode(void);
void playerResetLoResIf4Mb(void);
s16 playerGetFbWidth(void);
s16 playerGetFbHeight(void);
bool playerHasSharedViewport(void);
s16 playerGetViewportWidth(void);
s16 playerGetViewportLeft(void);
s16 playerGetViewportHeight(void);
s16 playerGetViewportTop(void);
f32 player0f0bd358(void);
void playerUpdateShake(void);
void playerTickTeleport(f32 *arg0);
void playerConfigureVi(void);
void playerTick(bool arg0);
void playerAllocateMatrices(struct coord *cam_pos, struct coord *cam_look, struct coord *cam_up);
Gfx *playerUpdateShootRot(Gfx *gdl);
void playerDisplayShield(void);
Gfx *playerRenderShield(Gfx *gdl);
Gfx *playerRenderHud(Gfx *gdl);
void playerDie(bool force);
void playerDieByShooter(u32 shooter, bool force);
void playerCheckIfShotInBack(s32 attackerplayernum, f32 x, f32 z);
f32 playerGetHealthBarHeightFrac(void);
void player0f0c1840(struct coord *pos, struct coord *up, struct coord *look, struct coord *pos2, RoomNum *rooms);
void player0f0c1ba4(struct coord *pos, struct coord *up, struct coord *look, struct coord *memcampos, s32 memcamroom);
void player0f0c1bd8(struct coord *pos, struct coord *up, struct coord *look);
void playersClearMemCamRoom(void);
void playerSetPerimEnabled(struct prop *prop, bool enable);
bool playerUpdateGeometry(struct prop *prop, u8 **start, u8 **end);
void playerUpdatePerimInfo(void);
void playerGetBbox(struct prop *prop, f32 *radius, f32 *ymax, f32 *ymin);
f32 playerGetHealthFrac(void);
f32 playerGetShieldFrac(void);
void playerSetShieldFrac(f32 frac);
s32 playerGetMissionTime(void);
s32 playerTickBeams(struct prop *prop);
s32 playerTickThirdPerson(struct prop *prop);
void playerChooseThirdPersonAnimation(struct chrdata *chr, s32 crouchpos, f32 speedsideways, f32 speedforwards, f32 speedtheta, f32 *angleoffset, struct attackanimconfig **animcfg);
Gfx *playerRender(struct prop *prop, Gfx *gdl, bool xlupass);
Gfx *playerLoadMatrix(Gfx *gdl);
void player0f0c3320(Mtxf *matrices, s32 count);
void playerSetTickMode(s32 tickmode);
void playerBeginGeFadeIn(void);
void playersBeginMpSwirl(void);
void player0f0b9a20(void);
void playerEndCutscene(void);
void playerPrepareWarpType1(s16 pad_id);
void playerPrepareWarpType2(struct warpparams *cmd, bool hasdir, s32 arg2);
void playerPrepareWarpType3(f32 posangle, f32 rotangle, f32 range, f32 height1, f32 height2, s32 padnum);
void playerStartCutscene2(void);
void playerSetFadeColour(s32 r, s32 g, s32 b, f32 a);
void playerAdjustFade(f32 maxfadetime, s32 r, s32 g, s32 b, f32 frac);
void playerSetFadeFrac(f32 maxfadetime, f32 frac);
bool playerIsFadeComplete(void);
void playerStartChrFade(f32 duration60, f32 targetfrac);
void playerSetHiResEnabled(bool enable);
void playerAutoWalk(s16 aimpad, u8 walkspeed, u8 turnspeed, u8 lookup, u8 dist);
void playerLaunchSlayerRocket(struct weaponobj *rocket);
void playerSetGlobalDrawWorldOffset(s32 room);
void playerSetGlobalDrawCameraOffset(void);
bool playerIsHealthVisible(void);
void playerSetCameraMode(s32 mode);
void playerSetCamPropertiesWithRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void playerSetCamPropertiesWithoutRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void playerSetCamProperties(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void playerClearMemCamRoom(void);
struct sndstate *playerSndStart(s32 arg0, s16 sound, struct sndstate **handle, s32 playernum, f32 pitch, s32 fxbus, s32 fxmix);

#if MAX_PLAYERS > 4
s32 playerGetCount(void);
s32 playerGetLocalCount(void);
#endif

#ifndef PLATFORM_N64
f32 playerGetDefaultFovY(s32 playernum);
f32 playerGetZoomFovMult(s32 playernum);
#endif

#endif
