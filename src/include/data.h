#ifndef _IN_DATA_H
#define _IN_DATA_H
#include <ultra64.h>
#include "constants.h"
#include "types.h"

#ifdef PLATFORM_N64
#define EXT_SEG
#define REF_SEG &
#else
#define EXT_SEG *
#define REF_SEG
#endif

#define bool s32
#define ubool u32

extern u8 *g_StackLeftAddrs[NUM_THREADS];
extern u8 *g_StackRightAddrs[NUM_THREADS];
extern u8 *g_StackAllocatedPos;
extern s32 var8005ce74;
extern f32 g_ViXScalesBySlot[NUM_GFXTASKS];
extern f32 g_ViYScalesBySlot[NUM_GFXTASKS];
extern bool g_SchedViModesPending[NUM_GFXTASKS];
extern s32 g_ViUnblackTimer;
extern s32 g_ViShakeDirection;
extern s32 g_ViShakeIntensity;
extern s32 g_ViShakeTimer;
extern s8 g_Resetting;
extern OSDevMgr __osViDevMgr;
extern u32 var8005cefc;
extern OSDevMgr __osPiDevMgr;
extern OSPiHandle *__osCurrentHandle[2];
extern u32 var8005cf30;
extern OSTime osClockRate;
extern s32 osViClock;
extern u32 __osGlobalIntMask;
extern s32 g_VmNumPages;
extern u8 g_LoadType;
extern s32 g_MainIsEndscreen;
extern s32 g_MainIsBooting;
extern bool g_MainIsDebugMenuOpen;
extern bool g_SndDisabled;
extern u16 g_SfxVolume;
extern s32 g_SoundMode;
extern s32 g_SndNumPlaying;
extern struct audiorussmapping g_AudioRussMappings[];
extern struct audioconfig g_AudioConfigs[];
extern bool g_JoyPfsPollMasterEnabled;
extern f32 var8005ef10[2];
extern s32 g_NumGlobalAilists;
extern s32 g_NumLvAilists;
extern bool g_ModelDistanceDisabled;
extern f32 g_ModelDistanceScale;
extern bool var8005efbc;
extern f32 var8005efc0;
extern u8 var8005efb0_2;
extern u32 var8005efd0;
extern bool var8005efd8_2;
extern bool g_ModelAnimMergingEnabled;
extern u32 var8005efe0;
extern u32 var8005efec;
extern u32 var8005eff8;
extern u32 g_NextAnimFrameIndex;
extern s16 g_NumAnimations;
extern struct animtableentry *g_Anims;
extern u8 *g_AnimToHeaderSlot;
extern s16 *var8005f014;
extern s32 g_AnimMaxBytesPerFrame;
extern s32 g_AnimMaxHeaderLength;
extern bool g_AnimHostEnabled;
extern s32 var8005f030;
extern s32 g_CdHasSavedBlock;
extern s32 var8005f038;
extern u16 *g_RdpOutBufferEnd;
extern u16 *g_RdpOutBufferStart;
extern struct rdptask *g_RdpCurTask;
extern N_ALSndPlayer *g_SndPlayer;
extern u32 var8005f4dc;
extern u32 var8005f548;
extern s16 n_eqpower[];
extern u32 var8005f66c;
extern u8 *var8005f6f8;
extern u8 *var8005f6fc;
extern u32 var8005f704;
extern OSViMode osViModeTable[];
extern u32 var80060890;
extern OSTimer *__osTimerList;
extern u32 __osPiAccessQueueEnabled;
extern OSThread *__osRunQueue;
extern OSThread *__osActiveQueue;
extern OSThread *__osRunningThread;
extern OSThread *__osFaultedThread;
extern u32 __osSiAccessQueueEnabled;
extern u32 __osContInitialized;
extern s32 __osPfsLastChannel;
extern s32 g_PfsPrevChannel;
extern u8 g_PfsPrevBank;
extern u32 var80060990;
extern u32 var800609a0;
extern u32 var800609c4;
extern u32 var800609f0;
extern u32 var800611ec;
extern OSViMode osViModePalLan1;
extern OSViMode osViModeMpalLan1;
extern OSViMode osViModeNtscLan1;
extern char ldigs[];
extern char udigs[];
extern Gfx var80061360[];
extern Gfx var80061380[];
extern Gfx var800613a0[];
extern f32 g_20SecIntervalFrac;
extern f32 g_Lv80SecIntervalFrac;
extern bool g_NbombsActive;
extern s32 g_WeatherActive;
extern u32 var80062410;
extern bool g_IsTitleDemo;
extern u32 g_TitleIdleTime60;
extern s32 g_TitleMode;
extern s32 g_TitleNextMode;
extern u32 g_TitleDelayedTimer;
extern s32 g_TitleDelayedMode;
extern s32 g_TitleTimer;
extern u32 var800624d4;
extern u32 var800624d8;
extern u32 var800624dc;
extern u32 var800624e0;
extern s32 g_TitleNextStage;
extern s32 var800624e8;
extern u32 var800624ec;
extern u32 var800624f0;
extern s32 var800624f4;
extern u8 g_FileState;
extern u8 var80062944;
extern u8 var80062948;
extern u8 var8006294c;
extern u32 var80062950;
extern bool g_WeatherTickEnabled;
extern struct var80062960 *var80062960;
extern f32 var80062968;
extern bool var8006296c;
extern s32 g_SelectedAnimNum;
extern u32 var80062974;
extern u32 var80062978;
extern u32 var8006297c;
extern s32 g_NextChrnum;
extern struct chrdata *g_ChrSlots;
extern s32 g_NumChrSlots;
extern struct shieldhit *g_ShieldHits;
extern bool g_ShieldHitActive;
extern s32 g_NumBondBodies;
extern s32 g_NumMaleGuardHeads;
extern s32 g_NumFemaleGuardHeads;
extern s32 g_NumMaleGuardTeamHeads;
extern s32 g_NumFemaleGuardTeamHeads;
extern s32 var80062b14;
extern s32 var80062b18;
extern s32 g_BondBodies[];
extern s32 g_MaleGuardHeads[];
extern s32 g_MaleGuardTeamHeads[];
extern s32 g_FemaleGuardHeads[];
extern s32 g_FemaleGuardTeamHeads[];
extern s32 var80062c80;
extern s32 g_ActiveMaleHeadsIndex;
extern s32 g_ActiveFemaleHeadsIndex;
extern f32 g_EnemyAccuracyScale;
extern f32 g_PlayerDamageRxScale;
extern f32 g_PlayerDamageTxScale;
extern f32 g_AttackWalkDurationScale;
extern s32 g_NumChrsSeenPlayerRecently2;
extern struct animtablerow g_DeathAnimsHumanGun[];
extern struct animtablerow g_AnimTableHumanSlumped[];
extern struct animtable *g_AnimTablesByRace[5];
extern struct attackanimconfig var800656c0[];
extern struct attackanimgroup *g_StandHeavyAttackAnims[][32];
extern struct attackanimconfig var80065be0[];
extern struct attackanimgroup *g_StandLightAttackAnims[][32];
extern struct attackanimconfig var800663d8[];
extern struct attackanimgroup *g_StandDualAttackAnims[][32];
extern struct attackanimgroup *g_KneelHeavyAttackAnims[][32];
extern struct attackanimgroup *g_KneelLightAttackAnims[][32];
extern struct attackanimgroup *g_KneelDualAttackAnims[][32];
extern struct attackanimconfig g_RollAttackAnims[];
extern struct attackanimconfig g_WalkAttackAnims[];
extern u32 g_StageFlags;
extern struct chrdata *g_BgChrs;
extern s16 *g_BgChrnums;
extern s32 g_NumBgChrs;
extern s16 *g_TeamList;
extern s16 *g_SquadronList;
extern struct var80067e6c *var80067fdc[];
extern s16 var80067ff0[];
extern f32 g_AutoAimScale;
extern u32 g_TintedGlassEnabled;
extern s32 g_AlarmTimer;
extern struct sndstate *g_AlarmAudioHandle;
extern f32 g_AlarmSpeakerWeight;
extern f32 g_GasReleaseTimer240;
extern bool g_GasReleasing;
extern struct coord g_GasPos;
extern s32 g_GasLastCough60;
extern f32 g_GasSoundTimer240;
extern struct sndstate *g_GasAudioHandle;
extern u32 g_CountdownTimerOff;
extern bool g_CountdownTimerRunning;
extern f32 g_CountdownTimerValue60;
extern u32 g_PlayersDetonatingMines;
extern s32 g_NextWeaponSlot;
extern s32 g_NextHatSlot;
extern struct linkliftdoorobj *g_LiftDoors;
extern struct padlockeddoorobj *g_PadlockedDoors;
extern struct safeitemobj *g_SafeItems;
extern struct linksceneryobj *g_LinkedScenery;
extern struct blockedpathobj *g_BlockedPaths;
extern struct prop *g_EmbedProp;
extern s32 g_EmbedHitPart;
extern f32 g_CctvWaitScale;
extern f32 g_CctvDamageRxScale;
extern f32 g_AutogunAccuracyScale;
extern f32 g_AutogunDamageTxScale;
extern f32 g_AutogunDamageRxScale;
extern f32 g_AmmoQuantityScale;
extern struct padeffectobj *g_PadEffects;
extern s32 g_LastPadEffectIndex;
extern struct autogunobj *g_ThrownLaptops;
extern struct beam *g_ThrownLaptopBeams;
extern s32 g_MaxThrownLaptops;
extern struct prop *g_Lifts[10];
extern u32 g_TvCmdlist00[];
extern u32 var8006aaa0[];
extern u32 var8006aae4[];
extern struct pschannel *g_PsChannels;
extern s8 var8006ae18;
extern s8 var8006ae1c;
extern s8 var8006ae20;
extern s8 var8006ae24;
extern s8 var8006ae28;
extern s32 g_ObjectiveLastIndex;
extern bool g_ObjectiveChecksDisabled;
extern u16 var8006ae90[];
extern u16 var8006af0c[];
extern u16 var8006af8c[];
extern struct weapon *g_Weapons[];
extern u32 g_BgunGunMemBaseSize4Mb2P;
extern bool g_CasingsActive;
extern s32 g_MaxModels;
extern s32 g_MaxAnims;
extern bool g_ModelIsLvResetting;
extern s32 g_ModelMostType1;
extern s32 g_ModelMostType2;
extern s32 g_ModelMostType3;
extern s32 g_ModelMostModels;
extern s32 g_ModelMostAnims;
extern s32 g_ViRes;
extern bool g_HiResEnabled;
extern s32 var8007072c;
extern u32 var80070738;
extern u32 var8007073c;
extern struct gecreditsdata *g_CurrentGeCreditsData;
extern bool g_PlayerTriggerGeFadeIn;
extern u32 var80070748;
extern u32 var8007074c;
extern bool g_PlayersWithControl[];
extern bool g_PlayerInvincible;
extern s32 g_InCutscene;
extern s16 g_DeathAnimations[];
extern s32 g_NumDeathAnimations;
extern s32 g_ScissorX1;
extern s32 g_ScissorX2;
extern s32 g_ScissorY1;
extern s32 g_ScissorY2;
extern u32 var800711f0;
#if VERSION >= VERSION_NTSC_1_0
extern char *g_StringPointer;
extern char *g_StringPointer2;
#else
extern char g_StringPointer[];
extern char g_StringPointer2[];
#endif
extern s32 g_MpPlayerNum;
extern u32 g_MpNumJoined;
extern u16 g_ControlStyleOptions[];
extern struct menudialogdef g_PreAndPostMissionBriefingMenuDialog;
extern struct solostage g_SoloStages[];
extern struct menudialogdef g_SoloMissionControlStyleMenuDialog;
extern struct menudialogdef g_CiControlStyleMenuDialog;
extern struct menudialogdef g_CiControlStylePlayer2MenuDialog;
extern struct menudialogdef g_ChangeAgentMenuDialog;
extern struct menudialogdef g_2PMissionOptionsHMenuDialog;
extern struct menudialogdef g_2PMissionOptionsVMenuDialog;
extern struct menudialogdef g_FrWeaponsAvailableMenuDialog;
extern struct menudialogdef g_SoloMissionPauseMenuDialog;
extern struct menudialogdef g_2PMissionPauseHMenuDialog;
extern struct menudialogdef g_2PMissionPauseVMenuDialog;
extern struct cutscene g_Cutscenes[];
extern struct menudialogdef g_CiMenuViaPcMenuDialog;
extern struct menudialogdef g_CiMenuViaPauseMenuDialog;
extern struct menudialogdef g_CheatsMenuDialog;
extern struct menudialogdef g_PakChoosePakMenuDialog;
extern struct menudialogdef g_FilemgrFileSelect4MbMenuDialog;
extern struct menudialogdef g_MpQuickGo4MbMenuDialog;
extern struct menudialogdef g_MpConfirmChallenge4MbMenuDialog;
extern struct menudialogdef g_MainMenu4MbMenuDialog;
extern struct menudialogdef g_MpEditSimulant4MbMenuDialog;
extern struct menudialogdef g_AdvancedSetup4MbMenuDialog;
extern struct filelist *g_FileLists[MAX_PLAYERS];
extern bool var80075bd0[];
extern struct headanim g_HeadAnims[2];
extern s32 var80075d60;
extern s32 var80075d64;
extern s32 var80075d68;
extern s32 g_DebugScreenshotRgb;
extern u8 unregistered_function1[];
extern u8 unregistered_function2[];
extern u8 unregistered_function3[];
extern struct ailist g_GlobalAilists[];
extern struct skeleton g_SkelDropship;
extern struct skeleton g_SkelSkShuttle;
extern struct skeleton g_SkelLift;
extern struct skeleton g_SkelCctv;
extern struct skeleton g_SkelTerminal;
extern struct skeleton g_SkelCiHub;
extern struct skeleton g_Skel19;
extern struct skeleton g_SkelAutogun;
extern struct skeleton g_Skel11;
extern struct skeleton g_Skel13;
extern struct skeleton g_Skel18;
extern struct skeleton g_Skel12;
extern struct skeleton g_SkelWindowedDoor;
extern struct skeleton g_Skel17;
extern struct skeleton g_Skel0C;
extern struct skeleton g_SkelLogo;
extern struct skeleton g_SkelPdLogo;
extern struct skeleton g_SkelHoverbike;
extern struct skeleton g_SkelJumpship;
extern struct skeleton g_SkelChopper;
extern struct skeleton g_SkelRope;
extern struct skeleton g_SkelBanner;
extern struct skeleton g_SkelMaianUfo;
extern struct skeleton g_SkelCableCar;
extern struct skeleton g_SkelSubmarine;
extern struct skeleton g_SkelTarget;
extern struct skeleton g_SkelRareLogo;
extern struct skeleton g_SkelWireFence;
extern struct skeleton g_SkelBB;
extern struct modelstate g_ModelStates[NUM_MODELS];
extern s8 g_PropExplosionTypes[];
extern struct skeleton g_SkelBasic;
extern struct skeleton g_SkelChrGun;
extern struct skeleton g_Skel0B;
extern struct modeldef g_PlayerModeldef;
extern struct skeleton g_Skel0A;
extern struct skeleton g_Skel20;
extern struct skeleton g_SkelClassicGun;
extern struct skeleton g_SkelCasing;
extern struct skeleton g_Skel06;
extern struct skeleton g_SkelUzi;
extern struct skeleton g_SkelJoypad;
extern struct skeleton g_Skel21;
extern struct skeleton g_SkelLaptopGun;
extern struct skeleton g_SkelK7Avenger;
extern struct skeleton g_SkelFalcon2;
extern struct skeleton g_SkelKnife;
extern struct skeleton g_SkelCmp150;
extern struct skeleton g_SkelDragon;
extern struct skeleton g_SkelSuperDragon;
extern struct skeleton g_SkelRocket;
extern struct skeleton g_Skel4A;
extern struct skeleton g_SkelShotgun;
extern struct skeleton g_SkelFarsight;
extern struct skeleton g_Skel4D;
extern struct skeleton g_SkelReaper;
extern struct skeleton g_SkelMauler;
extern struct skeleton g_SkelDevastator;
extern struct skeleton g_SkelPistol;
extern struct skeleton g_SkelAr34;
extern struct skeleton g_SkelMagnum;
extern struct skeleton g_SkelSlayerRocket;
extern struct skeleton g_SkelCyclone;
extern struct skeleton g_SkelSniperRifle;
extern struct skeleton g_SkelTranquilizer;
extern struct skeleton g_SkelCrossbow;
extern struct skeleton g_SkelTimedProxyMine;
extern struct skeleton g_SkelPhoenix;
extern struct skeleton g_SkelCallisto;
extern struct skeleton g_SkelRcp120;
extern struct skeleton g_SkelHudPiece;
extern struct skeleton g_SkelHand;
extern struct skeleton g_SkelLaser;
extern struct skeleton g_SkelGrenade;
extern struct skeleton g_SkelEcmMine;
extern struct skeleton g_SkelUplink;
extern struct skeleton g_SkelRemoteMine;
extern struct skeleton g_SkelChr;
extern struct skeleton g_SkelSkedar;
extern struct skeleton g_SkelDrCaroll;
extern struct skeleton g_Skel22;
extern struct skeleton g_SkelRobot;
extern struct headorbody g_HeadsAndBodies[152];
extern struct hatposition var8007dae4[1][6];
extern f32 g_SkyCloudOffset;
extern f32 g_SkyWindSpeed;
extern f32 g_SunAlphaFracs[];
extern s32 g_SunFlareTimers240[];
extern u32 var8007dba0;
extern u32 var8007dbb8;
extern u32 var8007dbd0;
extern struct coord g_TeleportToPos;
extern struct coord g_TeleportToUp;
extern struct coord g_TeleportToLook;
extern struct vtxstoretype g_VtxstoreTypes[];
extern s32 g_ExplosionShakeTotalTimer;
extern s32 g_ExplosionShakeIntensityTimer;
extern f32 g_ExplosionDamageTxScale;
extern struct sparktype g_SparkTypes[];
extern bool g_SparksAreActive;
extern struct weatherdata *g_WeatherData;
extern s32 g_NextShardNum;
extern bool g_ShardsActive;
extern s32 var800800f0jf;
extern s32 g_ScaleX;
extern s32 var80080108jf;
extern s32 var8007fad0;
extern struct font *g_FontNumeric;
extern struct fontchar *g_CharsNumeric;
extern struct font *g_FontHandelGothicXs;
extern struct fontchar *g_CharsHandelGothicXs;
extern struct font *g_FontHandelGothicSm;
extern struct fontchar *g_CharsHandelGothicSm;
extern struct font *g_FontHandelGothicMd;
extern struct fontchar *g_CharsHandelGothicMd;
extern struct font *g_FontHandelGothicLg;
extern struct fontchar *g_CharsHandelGothicLg;
extern bool var8007fb9c;
extern s32 g_StageIndex;
extern s16 var8007fc0c;
extern struct drawslot *g_BgSpecialDrawSlot;
extern u16 g_BgFrameCount;
extern s32 g_BgNumPortalCameraCacheItems;
extern f32 var8007fcb4;
extern struct stagetableentry g_Stages[61];
extern s32 g_RoomMtxNumSlots;
extern u32 g_GfxNumSwaps;
extern s32 g_NumReasonsToEndMpMatch;
extern s32 g_MusicEventQueueLength;
extern bool g_MusicNrgIsActive;
extern s32 g_MusicInterval240;
extern s32 g_MusicSleepRemaining240;
extern s32 g_MusicDeathTimer240;
extern s32 g_MusicAge60;
extern s32 g_MusicLife60;
extern s32 g_MusicSilenceTimer60;
extern bool g_Jpn;
#if VERSION >= VERSION_PAL_BETA
extern s32 g_LanguageId;
#endif
extern struct surfacetype *g_SurfaceTypes[15];
extern u16 *g_ZbufPtr1;
extern f32 var800845d4;
extern struct coord var800845dc;
extern struct menudialogdef g_2PMissionInventoryHMenuDialog;
extern struct menudialogdef g_2PMissionInventoryVMenuDialog;
extern struct menudialogdef g_MpEndscreenChallengeCheatedMenuDialog;
extern struct menudialogdef g_MpEndscreenChallengeFailedMenuDialog;
extern struct menudialogdef g_MpDropOutMenuDialog;
extern struct mparena g_MpArenas[];
extern struct menudialogdef g_MpWeaponsMenuDialog;
extern struct menudialogdef g_MpPlayerOptionsMenuDialog;
extern struct menudialogdef g_MpControlMenuDialog;
extern struct menudialogdef g_MpPlayerStatsMenuDialog;
extern struct menudialogdef g_MpPlayerNameMenuDialog;
extern struct menudialogdef g_MpLoadSettingsMenuDialog;
extern struct menudialogdef g_MpLoadPresetMenuDialog;
extern struct menudialogdef g_MpLoadPlayerMenuDialog;
extern struct menudialogdef g_MpArenaMenuDialog;
extern struct menudialogdef g_MpLimitsMenuDialog;
extern struct menudialogdef g_MpHandicapsMenuDialog;
extern struct menudialogdef g_MpReadyMenuDialog;
extern struct menudialogdef g_MpSimulantsMenuDialog;
extern struct menudialogdef g_MpTeamsMenuDialog;
extern struct menudialogdef g_MpChallengeListOrDetailsMenuDialog;
extern struct menudialogdef g_MpChallengeListOrDetailsViaAdvChallengeMenuDialog;
extern struct menudialogdef g_MpAdvancedSetupMenuDialog;
extern struct menudialogdef g_MpQuickGoMenuDialog;
extern struct menudialogdef g_MpQuickTeamGameSetupMenuDialog;
extern struct menudialogdef g_MpQuickTeamMenuDialog;
extern struct menudialogdef g_CombatSimulatorMenuDialog;
extern struct menudialogdef g_MpCombatOptionsMenuDialog;
extern struct menudialogdef g_HtbOptionsMenuDialog;
extern struct menudialogdef g_CtcOptionsMenuDialog;
extern struct menudialogdef g_KohOptionsMenuDialog;
extern struct menudialogdef g_HtmOptionsMenuDialog;
extern struct menudialogdef g_PacOptionsMenuDialog;
extern struct menudialogdef g_MpScenarioMenuDialog;
extern struct menudialogdef g_MpQuickTeamScenarioMenuDialog;
extern s32 var80087260;
extern bool g_MpEnableMusicSwitching;
extern struct mpweapon g_MpWeapons[NUM_MPWEAPONS];
#ifdef PLATFORM_N64
extern struct mphead g_MpHeads[VERSION == VERSION_JPN_FINAL ? 74 : 75];
#else // PD Plus Mod
extern struct mphead g_MpHeads[VERSION == VERSION_JPN_FINAL ? 75 : 76];
#endif
extern struct botprofile g_BotProfiles[18];
#ifdef PLATFORM_N64
extern struct mpbody g_MpBodies[61];
#else // PD Plus Mod
extern struct mpbody g_MpBodies[63];
#endif
extern struct mppreset g_MpPresets[];
extern u32 g_TeamColours[];
extern u32 var80087ce4[];
extern u8 g_BotCount;
extern struct aibotweaponpreference g_AibotWeaponPreferences[];
extern u32 var80087eb4;
extern struct challenge g_MpChallenges[30];
extern u8 g_FrIsValidWeapon;
extern s32 g_FrWeaponNum;
extern u8 g_ChrBioSlot;
extern u8 g_HangarBioSlot;
extern u8 g_DtSlot;
extern u8 var80088bb4;
extern struct vtxstoretype g_VtxstoreTypes[4];
extern struct menudialogdef g_FrWeaponListMenuDialog;
extern struct menudialogdef g_FrTrainingInfoInGameMenuDialog;
extern struct menudialogdef g_FrTrainingInfoPreGameMenuDialog;
extern struct menudialogdef g_FrCompletedMenuDialog;
extern struct menudialogdef g_FrFailedMenuDialog;
extern struct menudialogdef g_BioListMenuDialog;
extern struct menudialogdef g_DtListMenuDialog;
extern struct menudialogdef g_DtDetailsMenuDialog;
extern struct menudialogdef g_DtFailedMenuDialog;
extern struct menudialogdef g_DtCompletedMenuDialog;
extern struct menudialogdef g_HtListMenuDialog;
extern struct menudialogdef g_HtDetailsMenuDialog;
extern struct menudialogdef g_HtFailedMenuDialog;
extern struct menudialogdef g_HtCompletedMenuDialog;
extern struct menudialogdef g_HangarListMenuDialog;

#ifndef PLATFORM_N64

extern struct extplayerconfig g_PlayerExtCfg[MAX_PLAYERS];

extern struct weathercfg g_WeatherConfig[WEATHERCFG_MAX_STAGES];
extern const struct weathercfg g_DefaultWeatherConfig;
extern const struct weathercfg *g_CurWeatherConfig;

extern s32 g_MenuUsingMouse;
extern s32 g_MenuKeyboardPlayer;

extern f32 g_ViShakeIntensityMult;
extern u32 g_TexFilter2D;
extern s32 g_HudCenter;
extern s32 g_MenuMouseControl;
extern u32 g_HudAlignModeL;
extern u32 g_HudAlignModeR;
extern s32 g_PrevFrameFb;
extern s32 g_BlurFb;
extern s32 g_BlurFbCapTimer;
extern bool g_BlurFbDirty;
extern s32 g_TickRateDiv;
extern s32 g_TickExtraSleep;
extern s32 g_MusicDisableMpDeath;
extern s32 g_BgunGeMuzzleFlashes;
extern s32 g_FileAutoSelect;

#define PLAYER_EXTCFG() g_PlayerExtCfg[g_Vars.currentplayerstats->mpindex & 3]
#define PLAYER_DEFAULT_FOV (PLAYER_EXTCFG().fovy)

#define TEX_FILTER_2D g_TexFilter2D
#define ADJUST_ZOOM_FOV(x) ((x) * PLAYER_EXTCFG().fovzoommult)

// Mod Switch
// 0: Normal
// 1: GoldenEye X Mod
// 2: Kakariko Village Mod
extern s32 g_ModNum;
// GoldenEye X Mod
extern struct modelstate g_GexModelStates[NUM_MODELS];
extern s8 g_GexPropExplosionTypes[];
// All Solos in Multi Mod
extern bool g_NotLoadMod;

// MP Weapon Random Choice
extern bool g_UseMpWeaponRandomChoice;
extern u8 g_MpWeaponRandomChoice[NUM_MPWEAPONS];
extern s32 g_MpWeaponRandomChoiceNum;

#else // PLATFORM_N64

#define TEX_FILTER_2D G_TF_BILERP
#define ADJUST_ZOOM_FOV(x) (x)
#define PLAYER_DEFAULT_FOV 60.f

#endif // PLATFORM_N64

#endif
