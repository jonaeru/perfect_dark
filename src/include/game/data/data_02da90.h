#ifndef IN_GAME_DATA_02DA90_H
#define IN_GAME_DATA_02DA90_H
#include <ultra64.h>
#include "game/data/data_020df0.h"

extern struct mptrack g_MpTracks[NUM_MPTRACKS];
extern struct mppreset g_MpPresets[NUM_MPPRESETS];
extern u32 g_TeamColours[MAX_TEAMS];
extern u8 g_NumMpSimulantChrs;
extern struct var80087d14 var80087d14[];
extern u32 g_MpBotCommands[NUM_MPBOTCOMMANDS];
extern u32 var80087e2c;
extern u32 var80087e50;
extern u32 var80087e98;
extern u32 var80087eb0;
extern u32 var80088490;
extern u32 var80088494;
extern u32 var800884a4;
extern u32 g_MpChallengeIndex;
extern u32 var800884b4;
extern struct challenge g_MpChallenges[NUM_CHALLENGES];
extern u32 var800887c4;
extern u32 var80088800;
extern u8 var80088804;
extern u32 var80088808;
extern u32 var8008880c;
extern u32 var80088810;
extern u32 var80088854;
extern u8 var800888a0;
extern struct chrbio g_ChrBios[10];
extern u32 misc_bios[];
extern u32 var80088964;
extern u32 place_bios[];
extern u8 var80088ad8;
extern u8 var80088adc;
extern u32 var80088ae0;
extern u32 var80088aec;
extern u32 g_CiDeviceStageFlags[10];
extern u32 device_descriptions[];
extern u32 device_tips1[];
extern u32 device_tips2[];
extern u8 var80088bb8;
extern u32 var80088bbc;
extern u32 var80088bc8;
extern u32 var80088be4;
extern u32 var80088c04;
extern u32 var80088c20;
extern u32 var80088c3c;
extern u32 var80088c58;
extern u32 var80088c70;
extern u16 g_FiringRangeFailReasons[5];
extern u16 g_FiringRangeDifficultyNames[3];
extern u32 var80088c8c;
extern u32 var80088c90;
extern u32 var80088c94;
extern u32 var80088c98;
extern u32 var80088c9c;
extern u32 var80088ca0;
extern u32 var80088ca4;
extern u32 var80088ca8;
extern struct menu_dialog menudialog_frdifficulty;
extern struct menu_dialog g_FrWeaponListMenuDialog;
extern struct menu_dialog g_FrTrainingInfoMenuDialog;
extern struct menu_dialog menudialog_frtraininginfo2;
extern struct menu_dialog menudialog_frtrainingstats;
extern struct menu_dialog menudialog_frtrainingstats2;
extern u32 var800891b0;
extern struct menu_dialog menudialog_information;
extern u32 var80089268;
extern u32 var80089280;
extern struct menu_dialog menudialog_characterprofile;
extern struct menu_dialog menudialog_2f3c0;
extern struct menu_dialog g_DeviceTrainingListMenuDialog;
extern u32 var800893f8;
extern struct menu_dialog g_DeviceTrainingDetailsMenuDialog;
extern struct menu_dialog g_DeviceTrainingStatsFailedMenuDialog;
extern struct menu_dialog g_DeviceTrainingStatsCompletedMenuDialog;
extern struct menu_dialog g_HoloTrainingListMenuDialog;
extern struct menu_dialog g_HoloTrainingDetailsMenuDialog;
extern struct menu_dialog g_HoloTrainingStatsFailedMenuDialog;
extern struct menu_dialog g_HoloTrainingStatsCompletedMenuDialog;
extern u32 var800897a4;
extern u32 var800897b4;
extern u32 var800897c4;
extern struct hoverprop hoverprops[];
extern struct menu_dialog menudialog_2f938;
extern struct menu_dialog menudialog_2f950;
extern struct menu_dialog menudialog_hangarinformation;
extern u32 var80089ad0;
extern u32 var8008a2d0;

#endif
