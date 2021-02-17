#ifndef _IN_GAME_GAME_0B69D0_H
#define _IN_GAME_GAME_0B69D0_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

f32 playerChooseSpawnLocation(f32 chrwidth, struct coord *dstpos, s16 *dstrooms, struct prop *prop, s16 *spawnpads, s32 numspawnpads);
f32 playerChooseGeneralSpawnLocation(f32 chrwidth, struct coord *pos, s16 *rooms, struct prop *prop);
void currentPlayerStartNewLife(void);
void currentPlayerResetToDefault(void);
bool currentPlayerAssumeChrForAnti(struct chrdata *chr, s32 param_2);
void currentPlayerSpawn(void);
void func0f0b85a0(struct playerbond *pb, struct coord *pos);
void func0f0b85f8(void);
void currentPlayerChooseBodyAndHead(s32 *bodynum, s32 *headnum, s32 *arg2);
void func0f0b8ba0(void);
void func0f0b9538(void);
void func0f0b96e8(void);
void func0f0b9cbc(void);
void cameraDoAnimation(s16 anim_id);
void func0f0ba190(u32 arg0);
void func0f0ba29c(bool arg0);
f32 cutsceneGetBlurFrac(void);
void currentPlayerSetZoomFovY(f32 fovy, f32 timemax);
f32 currentPlayerGetZoomFovY(void);
void func0f0ba8b0(f32 value);
f32 currentPlayerGetTeleportFovY(void);
void currentPlayerUpdateZoom(void);
void func0f0bace0(void);
void currentPlayerTickPauseMenu(void);
void currentPlayerPause(s32 root);
void func0f0baf38(void);
Gfx *func0f0baf84(Gfx *gdl);
Gfx *fadeDraw(Gfx *gdl, u32 r, u32 g, u32 b, f32 frac);
Gfx *currentPlayerDrawFade(Gfx *gdl);
void currentPlayerUpdateColourScreenProperties(void);
void currentPlayerTickChrFade(void);
void func0f0bb69c(void);
void func0f0bb814(void);
void func0f0bbf14(void);
Gfx *hudRenderHealthBar(Gfx *gdl);
void currentPlayerSurroundWithExplosions(s32 arg0);
void currentPlayerTickExplode(void);
void viResetDefaultModeIf4Mb(void);
s16 viGetFbWidth(void);
s16 viGetFbHeight(void);
bool func0f0bc4c0(void);
s16 currentPlayerGetViewportWidth(void);
s16 currentPlayerGetViewportLeft(void);
s16 currentPlayerGetViewportHeight(void);
s16 currentPlayerGetViewportTop(void);
f32 func0f0bd358(void);
void func0f0bd3c4(void);
void currentPlayerTickTeleport(f32 *arg0);
void currentPlayerConfigureVi(void);
void func0f0bd904(bool arg0);
void func0f0bfc7c(struct coord *cam_pos, struct coord *cam_look, struct coord *cam_up);
Gfx *currentPlayerUpdateShootRot(Gfx *gdl);
void currentPlayerUpdateShieldShow(void);
Gfx *func0f0c0190(Gfx *gdl);
Gfx *func0f0c07c8(Gfx *gdl);
void currentPlayerDie(bool force);
void currentPlayerDieByShooter(u32 shooter, bool force);
void currentPlayerCheckIfShotInBack(s32 attackerplayernum, f32 x, f32 z);
f32 currentPlayerGetHealthBarHeightFrac(void);
void func0f0c1840(struct coord *pos, struct coord *up, struct coord *look, struct coord *pos2, s16 *rooms);
void func0f0c1ba4(struct coord *pos, struct coord *up, struct coord *look, struct coord *memcampos, s32 memcamroom);
void func0f0c1bd8(struct coord *pos, struct coord *up, struct coord *look);
void allPlayersClearMemCamRoom(void);
void func0f0c1e54(struct prop *prop, bool enable);
bool playerUpdateGeometry(struct prop *prop, struct tiletype3 **arg1, struct tiletype3 **arg2);
void currentPlayerUpdatePerimInfo(void);
void propPlayerGetBbox(struct prop *prop, f32 *width, f32 *ymax, f32 *ymin);
f32 currentPlayerGetHealthFrac(void);
f32 currentPlayerGetShieldFrac(void);
void currentPlayerSetShieldFrac(f32 shield);
s32 getMissionTime(void);
s32 func0f0c228c(struct prop *prop);
u32 func0f0c2364(void);
u32 func0f0c2a58(void);
Gfx *playerRender(struct prop *prop, Gfx *gdl, bool withalpha);
Gfx *currentPlayerLoadMatrix(Gfx *gdl);
u32 func0f0c3320(void);
void setTickMode(s32 tickmode);
void func0f0b9650(void);
void func0f0b9674(void);
void func0f0b9a20(void);
void func0f0b9afc(void);
void warpBondToPad(s16 pad_id);
void func0f0b9bac(u32 *cmd, s32 arg1, s32 arg2);
void func0f0b9c1c(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, s32 arg5);
void func0f0ba010(void);
void currentPlayerSetFadeColour(s32 r, s32 g, s32 b, f32 a);
void currentPlayerAdjustFade(f32 maxfadetime, s32 r, s32 g, s32 b, f32 frac);
void currentPlayerSetFadeFrac(f32 maxfadetime, f32 frac);
bool currentPlayerIsFadeComplete(void);
void currentPlayerStartChrFade(f32 duration60, f32 targetfrac);
void optionsSetHiRes(bool enable);
void currentPlayerAutoWalk(s16 aimpad, u8 walkspeed, u8 turnspeed, u8 lookup, u8 dist);
void currentPlayerLaunchSlayerRocket(struct weaponobj *rocket);
void currentPlayerSetGlobalDrawWorldOffset(s32 room);
void currentPlayerSetGlobalDrawCameraOffset(void);
bool currentPlayerIsHealthVisible(void);
void currentPlayerSetCameraMode(s32 mode);
void currentPlayerSetCamPropertiesWithRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void currentPlayerSetCamPropertiesWithoutRoom(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void currentPlayerSetCamProperties(struct coord *pos, struct coord *up, struct coord *look, s32 room);
void currentPlayerClearMemCamRoom(void);

#endif
