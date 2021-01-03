#ifndef _IN_GAME_MPLAYER_SETUP_H
#define _IN_GAME_MPLAYER_SETUP_H
#include <ultra64.h>
#include "types.h"

char *mpGetCurrentPlayerName(struct menuitem *item);
s16 mpChooseRandomStage(void);
s32 menuhandler001791c8(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextWeaponNameForSlot(struct menuitem *item);
char *mpMenuTextSetupName(struct menuitem *item);
s32 func0f179da4(u32 operation, struct menuitem *item, union handlerdata *data, s32 arg3, s32 arg4, s32 arg5);
s32 menudialog0017a174(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandler0017a20c(u32 operation, struct menuitem *item, union handlerdata *data);
u32 mpMenuTextKills(void);
u32 mpMenuTextDeaths(void);
u32 mpMenuTextGamesPlayed(void);
u32 mpMenuTextGamesWon(void);
u32 mpMenuTextGamesLost(void);
u32 mpMenuTextHeadShots(void);
u32 mpMenuTextMedalAccuracy(void);
u32 mpMenuTextMedalHeadShot(void);
u32 mpMenuTextMedalKillMaster(void);
u32 mpMenuTextMedalSurvivor(void);
char *mpMenuTextAmmoUsed(struct menuitem *item);
char *mpMenuTextDistance(struct menuitem *item);
u32 mpMenuTextTime(void);
char *mpMenuTextAccuracy(struct menuitem *item);
void func0f17ad80(char *dst, f32 arg0);
char *mpMenuTextPainReceived(struct menuitem *item);
char *mpMenuTextDamageDealt(struct menuitem *item);
s32 menuhandlerMpMedal(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTitleStatsForPlayerName(struct menudialog *dialog);
u32 mpMenuTextUsernamePassword(void);
s32 func0f17b4f8(u32 operation, struct menuitem *item, union handlerdata *data, s32 arg3, s32 arg4);
char *mpMenuTextBodyName(struct menuitem *item);
void func0f17b8f0(void);
s32 menuhandler0017b91c(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017bab4(u32 operation, struct menuitem *item, union handlerdata *data);
u32 func0f17be2c(void);
s32 menuhandler0017bfc0(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpHandicapPlayer(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextHandicapPlayerName(struct menuitem *item);
s32 menuhandler0017c6a4(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextSimulantDescription(struct menuitem *item);
s32 menudialog0017ccfc(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpSimulantDifficulty(u32 operation, struct menuitem *item, union handlerdata *data);
u32 func0f17cfc0(void);
u32 func0f17d378(void);
s32 func0f17d4b0(u32 operation, struct menuitem *item, union handlerdata *data, s32 numteams);
char *mpMenuTextChrNameForTeamSetup(struct menuitem *item);
s32 func0f17dac4(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextSelectTuneOrTunes(struct menuitem *item);
s32 menuhandler0017dc84(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextCurrentTrack(struct menuitem *item);
s32 menuhandler0017e06c(u32 operation, struct menuitem *item, union handlerdata *data);
u32 func0f17e288(void);
u32 func0f17e318(void);
s32 menudialog0017e3fc(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandler0017e4d4(u32 operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextChallengeName(struct menuitem *item);
s32 mpCombatChallengesMenuDialog(u32 operation, struct menudialog *dialog, union handlerdata *data);
char *mpMenuTextSavePlayerOrCopy(struct menuitem *item);
char *mpMenuTextArenaName(struct menuitem *item);
char *mpMenuTextWeaponSetName(struct menuitem *item);
u32 func0f17f100(void);
void func0f17f260(void);
u32 func0f17f428(void);
s32 menuhandlerPlayerTeam(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpNumberOfSimulants(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSimulantsPerTeam(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017f74c(u32 operation, struct menuitem *item, union handlerdata *data);
u32 func0f17fa28(void);
void func0f17fcb0(s32 silent);
s32 menuhandlerMpSlowMotion(u32 operation, struct menuitem *item, union handlerdata *data);
void mpHtbAddPad(s16 padnum);
u32 func0f17ffe4(void);
void func0f180078(void);
void func0f1800a8(void);
u32 scenarioHtbCallback18(void);
void scenarioCtcReset(void);
bool scenarioCtcHighlight(struct prop *prop, u32 *colour);
void func0f181800(s32 *cmd);
void scenarioCtcCallback38(s16 arg0, s32 *arg1, s32 *arg2, s32 *arg3);
void scenarioKohCallback10(void);
u32 scenarioKohCallback18(void);
void func0f182aac(s32 *cmd);
void scenarioKohCallback38(s16 arg0, s32 *arg1, s32 *arg2, s32 *arg3);
void func0f182ba4(s16 padnum);
void func0f182bf4(void);
void func0f182c98(void);
void scenarioHtmReset(void);
void scenarioHtmCallback14(struct chrdata *chr);
u32 scenarioHtmCallback18(void);
Gfx *scenarioHtmRadar(Gfx *gdl);
void scenarioPacChooseVictims(void);
bool scenarioPacHighlight(struct prop *prop, u32 *colour);
void func0f1845bc(void);
u32 func0f1847b8(void);
void scenarioPacCallback10(void);
u32 scenarioPacCallback18(void);
bool scenarioPacRadar2(Gfx **gdl, struct prop *prop);
s32 mpOptionsMenuDialog(u32 operation, struct menudialog *dialog, union handlerdata *data);
char *mpMenuTextScenarioShortName(struct menuitem *item);
char *mpMenuTextScenarioName(struct menuitem *item);
s32 menuhandler00185068(u32 operation, struct menuitem *item, union handlerdata *data);
void func0f185568(void);
Gfx *func0f185774(Gfx *gdl);
u32 func0f185c14(void);
u32 func0f185e20(void);
void mpPrepareScenario(void);
u32 func0f186508(void);
void func0f1866b8(s32 playernum, char *message);
u32 func0f186780(void);
u32 chrGiveBriefcase(struct chrdata *chr, struct prop *prop);
void func0f187288(struct chrdata *chr, struct prop *prop);
bool chrGiveUplink(struct chrdata *chr, struct prop *prop);
void scenarioHtmActivateUplink(struct chrdata *chr, struct prop *prop);
s32 menuhandlerMpDropOut(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTeamsLabel(u32 operation, struct menuitem *item, union handlerdata *data);
s32 mpGetNumStages(void);
s32 menuhandlerMpControlStyle(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpWeaponSlot(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpWeaponSetDropdown(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpControlCheckbox(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpAimControl(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpCheckboxOption(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTeamsEnabled(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpDisplayOptionCheckbox(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpConfirmSaveChr(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpPlayerName(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSaveSetupOverwrite(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSaveSetupCopy(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpCharacterBody(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpUsernamePassword(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpCharacterHead(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTimeLimitSlider(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpScoreLimitSlider(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTeamScoreLimitSlider(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpRestoreScoreDefaults(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpRestoreHandicapDefaults(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menudialogMpReady(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menudialogMpSimulant(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpSimulantHead(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSimulantBody(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpDeleteSimulant(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpChangeSimulantType(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpClearAllSimulants(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpAddSimulant(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSimulantSlot(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menudialogMpSimulants(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpTwoTeams(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpThreeTeams(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpFourTeams(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpMaximumTeams(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpHumansVsSimulants(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpHumanSimulantPairs(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTeamSlot(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menudialogMpSelectTune(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpMultipleTunes(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpTeamNameSlot(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017e38c(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017e9d8(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpAbortChallenge(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpStartChallenge(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017ec64(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpLock(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSavePlayer(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandler0017ef30(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpSaveSettings(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menudialogMpGameSetup(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menudialogMpQuickGo(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpFinishedSetup(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerQuickTeamSeparator(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpQuickTeamOption(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menudialogCombatSimulator(u32 operation, struct menudialog *dialog, union handlerdata *data);
s32 menuhandlerMpAdvancedSetup(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpDisplayTeam(u32 operation, struct menuitem *item, union handlerdata *data);
s32 menuhandlerMpOneHitKills(u32 operation, struct menuitem *item, union handlerdata *data);

#endif
