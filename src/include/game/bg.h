#ifndef IN_GAME_BG_H
#define IN_GAME_BG_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void bgUnpausePropsInRoom(u32 roomnum, bool tintedglassonly);
void bgSetRoomOnscreen(s32 room, s32 draworder, struct screenbox *arg2);
void bgGetRoomBrightnessRange(s32 roomnum, u8 *min, u8 *max);
struct drawslot *bgGetRoomDrawSlot(s32 roomnum);
Gfx *bgRenderXrayData(Gfx *gdl, struct xraydata *xraydata);
Gfx *bgAddXrayTri(Gfx *gdl, struct xraydata *xraydata, s16 vertices1[3], s16 vertices2[3], s16 vertices3[3], u32 colour1, u32 colour2, u32 colour3);
void bgChooseXrayVtxColour(bool *inrange, s16 vertex[3], u32 *colour, struct xraydata *xraydata);
Gfx *bgProcessXrayTri(Gfx *gdl, struct xraydata *xraydata, s16 arg2[3], s16 arg3[3], s16 arg4[3], s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10);
Gfx *bgRenderGdlInXray(Gfx *gdl, s8 *readgdl, Vtx *vertices, s16 arg3[3]);
Gfx *bgRenderRoomXrayPass(Gfx *gdl, s32 roomnum, struct roomblock *blocks, bool recurse, s16 arg4[3]);
Gfx *bgRenderRoomInXray(Gfx *gdl, s32 roomnum);
Gfx *bgRenderSceneInXray(Gfx *gdl);
Gfx *bgRenderScene(Gfx *gdl);
Gfx *bgRenderArtifacts(Gfx *gdl);
void bgLoadFile(void *memaddr, u32 offset, u32 len);
s32 bgGetStageIndex(s32 stagenum);
f32 bgCalculatePortalSurfaceArea(s32 portal);
u8 bgCalculatePortalAlpha(s32 portal);
u32 not(u32 arg);
u32 bgXorBabebabe(u32 value);
void bgReset(s32 stagenum);
void bgBuildTables(s32 stagenum);
void bgStop(void);
void bgSetStageTranslationThing(f32 arg0);
f32 bgGetStageTranslationThing(void);
f32 bgGetScaleBg2Gfx(void);
void bgSetScaleBg2Gfx(f32 arg0);
void bgTickCounter(void);
void bgTick(void);
Gfx *bgRender(Gfx *gdl);
Gfx *bgScissorToViewport(Gfx *gdl);
Gfx *bgScissorWithinViewportF(Gfx *gdl, f32 viewleft, f32 viewtop, f32 viewright, f32 viewbottom);
Gfx *bgScissorWithinViewport(Gfx *gdl, s32 viewleft, s32 viewtop, s32 viewright, s32 viewbottom);
void bgClearPortalCameraCache(void);
bool bgRoomIntersectsScreenBox(s32 room, struct screenbox *arg1);
bool bg3dPosTo2dPos(struct coord *cornerpos, struct coord *screenpos);
bool bgGetPortalScreenBbox(s32 portal, struct screenbox *arg1);
Gfx *bgDrawBoxEdge(Gfx *gdl, s32 x1, s32 y1, s32 x2, s32 y2);
bool bgGetBoxIntersection(struct screenbox *a, struct screenbox *b);
void bgExpandBox(struct screenbox *a, struct screenbox *b);
void bgCopyBox(struct screenbox *dst, struct screenbox *src);
bool bgRoomIsOnscreen(s32 room);
bool bgRoomIsStandby(s32 room);
bool bgRoomIsOnPlayerScreen(s32 room, u32 playernum);
bool bgRoomIsOnPlayerStandby(s32 room, u32 aibotindex);
s32 bgFindPortalByVertices(struct portalvertices *pvertices);
u32 bgInflate(u8 *src, u8 *dst, u32 len);
Gfx *bgGetNextGdlInBlock(struct roomblock *block, Gfx *start, Gfx *end);
Gfx *bgGetNextGdlInLayer(s32 roomnum, Gfx *start, u32 types);
Vtx *bgFindVerticesForGdl(s32 roomnum, Gfx *gdl);
void bgLoadRoom(s32 roomnum);
void bgUnloadRoom(s32 room);
void bgUnloadAllRooms(void);
void bgGarbageCollectRooms(s32 bytesneeded, bool desparate);
void bgTickRooms(void);
Gfx *bgRenderRoomPass(Gfx *gdl, s32 roomnum, struct roomblock *blocks, bool arg3);
Gfx *bgRenderRoomOpaque(Gfx *gdl, s32 roomnum);
Gfx *bgRenderRoomXlu(Gfx *gdl, s32 roomnum);
s32 bgPopulateVtxBatchType(s32 roomnum, struct vtxbatch *batches, Gfx *gdl, s32 batchindex, Vtx *vertices, s32 arg5);
void bgFindRoomVtxBatches(s32 roomnum);
bool bgTestLineIntersectsIntBbox(struct coord *arg0, struct coord *arg1, s32 *arg2, s32 *arg3);
bool bgTestLineIntersectsBbox(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3);
bool bgTestHitOnObj(struct coord *arg0, struct coord *arg1, struct coord *arg2, Gfx *gdl, Gfx *gdl2, Vtx *vertices, struct hitthing *hitthing);
bool bgTestHitOnChr(struct model *model, struct coord *arg1, struct coord *arg2, struct coord *arg3, Gfx *arg4, Gfx *arg5, Vtx *vertices, f32 *arg7, struct hitthing *hitthing);
bool bgTestHitInVtxBatch(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct vtxbatch *batches, s32 roomnum, struct hitthing *hitthing);
s32 bg0f1612e4(struct coord *bbmin, struct coord *bbmax, struct coord *frompos, struct coord *dist, struct coord *arg4, struct coord *arg5);
bool bgTestHitInRoom(struct coord *frompos, struct coord *topos, s32 roomnum, struct hitthing *hitthing);
bool bgRoomIsLoaded(s32 room);
bool bgRoomContainsCoord(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoomCheap(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoomExpensive(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoom(struct coord *pos, RoomNum roomnum);
void bgFindRoomsByPos(struct coord *pos, RoomNum *inrooms, RoomNum *aboverooms, s32 max, RoomNum *bestroom);
bool bgCmdPushValue(bool value);
bool bgCmdPopValue(void);
bool bgCmdGetNthValueFromEnd(s32 offset);
struct bgcmd *bgCmdExecuteBranch(struct bgcmd *cmd, bool s2);
struct bgcmd *bgCmdExecute(struct bgcmd *cmd);
void bgTickPortalsXray(void);
void bgAddToSnake(RoomNum fromroomnum, RoomNum roomnum, s16 draworder, struct screenbox *box);
void bgConsumeSnakeItem(struct bgsnakeitem *arg0);
bool bgTryConsumeSnake(void);
void bgChooseRoomsToLoad(void);
void bgTickPortals(void);
Gfx *bgRenderSceneAndLoadCandidate(Gfx *gdl);
s32 bgGetForceOnscreenRooms(RoomNum *rooms, s32 len);
s32 bgRoomGetNeighbours(s32 room, RoomNum *rooms, s32 len);
bool bgRoomsAreNeighbours(s32 roomnum1, s32 roomnum2);
void bgCalculateScreenProperties(void);
void bgExpandRoomToPortals(s32 roomnum);
void bgPortalSwapRooms(s32 portal);
void bgInitPortal(s32 portalnum);
void bgInitRoom(s32 roomnum);
void bgSetPortalOpenState(s32 portal, bool open);
s32 bgFindPortalBetweenPositions(struct coord *pos1, struct coord *pos2);
bool bgIsBboxOverlapping(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3);
void bgCalculatePortalBbox(s32 portalnum, struct coord *bbmin, struct coord *bbmax);
void bgFindEnteredRooms(struct coord *bbmin, struct coord *upper, RoomNum *rooms, s32 maxlen, bool arg4);
#ifndef PLATFORM_N64
void bgCalculateGlaresForVisibleRooms(void);
#endif

#endif
