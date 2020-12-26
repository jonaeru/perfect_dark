#ifndef IN_GAME_PROPOBJ_H
#define IN_GAME_PROPOBJ_H
#include <ultra64.h>
#include "types.h"

extern f32 var80069948;
extern f32 var8006994c;
extern f32 var80069950;
extern f32 var80069954;
extern f32 var80069958;

void alarmActivate(void);
void alarmDeactivate(void);
bool alarmIsActive(void);
bool audioPlayFromProp(s32 channel_id, s16 audio_id, s32 volumemaybe, struct prop *prop, s32 arg4, s32 arg5);
f32 countdownTimerGetValue(void);
bool countdownTimerIsHidden(void);
bool countdownTimerIsRunning(void);
void countdownTimerSetRunning(bool running);
void countdownTimerSetValue(f32 frames);
void countdownTimerSetVisible(u32 flag, bool show);
void countdownTimerTick(void);
bool func0f066310(struct prop *doorprop, bool arg1);
bool doorIsPadlockFree(struct doorobj *door);
u32 func0f066640(void);
u32 func0f0666cc(void);
u32 func0f0667ac(void);
u32 func0f0667b4(void);
f32 func0f0667bc(u32 arg0);
u32 func0f0667c4(void);
u32 func0f0667cc(void);
u32 func0f0667d4(void);
u32 func0f0667dc(void);
u32 func0f06680c(void);
u32 func0f06683c(void);
u32 func0f06686c(void);
u32 func0f06689c(void);
u32 func0f0668cc(void);
u32 func0f0668fc(void);
u32 func0f06692c(void);
u32 func0f06695c(void);
u32 func0f06698c(void);
u32 func0f0669bc(void);
u32 func0f0669ec(void);
u32 func0f066a1c(void);
u32 func0f066abc(void);
u32 func0f066b5c(void);
u32 func0f067424(void);
u32 func0f0674bc(void);
u32 func0f0675c8(void);
u32 func0f0677ac(void);
u32 func0f0678f8(void);
u32 func0f06797c(void);
u32 func0f0679ac(void);
u32 func0f067bc4(void);
u32 func0f067d88(void);
bool func0f067dc4(struct model *model, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4);
bool modelSetRedBox(struct model *model, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4);
void func0f06803c(f32 *arg0, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4);
struct defaultobj *objFindByPadNum(s32 padnum);
u32 func0f068218(void);
void func0f0682dc(struct obj48 *obj48);
void func0f0682fc(struct prop *prop);
void func0f068368(struct obj48 *obj48);
struct obj48 *func0f06843c(void);
void func0f0685e4(struct prop *prop);
u32 func0f068694(void);
u32 func0f0686e0(void);
struct obj48 *func0f0686f0(void);
u32 func0f068760(void);
u32 func0f0687b8(struct defaultobj *obj);
u32 func0f0687e4(void);
u32 func0f0688f4(void);
f32 *func0f06896c(void *modelfiledata);
u32 func0f068998(void);
struct model08thing *func0f068aa8(struct model *model);
u32 func0f068ad4(void);
struct model08thing *func0f068af4(struct defaultobj *obj);
u32 func0f068b14(void);
u32 func0f068c04(void);
u32 func0f068fc8(void);
void func0f069144(struct prop *prop, u8 *nextcol, u16 floorcol);
void func0f069630(struct prop *prop, u8 *nextcol, u16 floorcol);
void func0f069710(u8 *shadecol, u8 *nextcol);
u32 func0f069750(void);
void func0f069850(struct defaultobj *obj, struct coord *pos, f32 *realrot, struct geo *geo);
void func0f069b4c(struct defaultobj *obj);
void func0f069c1c(struct defaultobj *obj);
void func0f069c70(struct defaultobj *obj, bool arg1, bool arg2);
u32 func0f069cd8(void);
u32 func0f069d38(void);
u32 func0f06a170(void);
struct prop *func0f06a1ec(struct singlemonitorobj *monitor, struct model08 *filedata, u32 arg2, u32 arg3);
struct prop *func0f06a52c(struct singlemonitorobj *monitor, struct model08 *model08);
struct prop *func0f06a550(struct singlemonitorobj *monitor);
u32 func0f06a580(void);
f32 func0f06a620(struct defaultobj *obj);
void func0f06a730(struct defaultobj *obj, struct pad *newpad, Mtxf *matrix, s16 *arg3, struct pad *arg4);
u32 func0f06ab60(void);
void func0f06ac40(struct defaultobj *obj);
void func0f06ac90(struct prop *prop);
void func0f06ad2c(void *ptr, bool arg1, bool arg2);
void func0f06b34c(void *ptr, s32 arg1);
u32 func0f06b36c(void);
bool func0f06b39c(struct coord *arg0, struct coord *arg1, struct coord *arg2, f32 arg3);
u32 func0f06b488(void);
u32 func0f06b610(void);
u32 func0f06be44(void);
u32 func0f06bea0(void);
u32 func0f06c28c(void);
u32 func0f06c8ac(void);
u32 func0f06cd00(void);
u32 func0f06d37c(void);
void func0f06d90c(f32 *frac, f32 maxfrac, f32 *fracspeed, f32 accel, f32 decel, f32 maxspeed);
void func0f06db00(f32 *arg0, f32 arg1, f32 *arg2, f32 arg3, f32 arg4, f32 arg5);
u32 func0f06dbd8(void);
u32 func0f06e87c(void);
u32 func0f06e9cc(void);
u32 func0f06eb4c(void);
u32 func0f06ec20(void);
u32 func0f06ed64(void);
u32 func0f06ef44(void);
u32 func0f06f0a0(void);
void func0f06f314(struct prop *prop, u32 explosiontype);
void func0f06f504(struct prop *ammocrate);
void func0f06f54c(struct prop *weapon);
void func0f07063c(struct prop *prop, bool arg1);
void func0f070698(struct prop *prop, bool arg1);
void func0f0706f8(struct prop *prop, bool arg1);
void func0f07079c(struct prop *prop, bool arg1);
s32 func0f07092c(struct coord *pos, f32 arg1, f32 arg2, f32 arg3);
u32 func0f070a1c(void);
u32 func0f070bd0(void);
u32 func0f070ca0(void);
void liftActivate(struct prop *prop, u8 liftnum);
struct prop *liftFindByPad(s16 padnum);
u32 func0f070eac(void);
void func0f070f08(struct liftobj *lift, bool arg0);
void liftGoToStop(struct liftobj *lift, s32 stopnum);
u32 func0f071360(void);
void func0f0713e4(struct defaultobj *obj, struct hov *hov, struct coord *coord, s16 *rooms, f32 *matrix);
void func0f0714b8(struct defaultobj *obj, struct hov *hov);
s32 objIsHoverpropOrBike(struct defaultobj *obj);
f32 hoverpropGetTurnAngle(struct defaultobj *obj);
void hoverpropSetTurnAngle(struct defaultobj *obj, f32 angle);
u32 func0f072144(void);
u32 func0f072650(void);
u32 func0f0726ec(void);
void func0f072774(struct defaultobj *obj);
u32 func0f0727d4(void);
void func0f072adc(struct hoverbikeobj *hoverbike, f32 arg1, f32 arg2, f32 arg3);
u32 func0f0732d4(void);
u32 func0f073478(void);
u32 func0f073ae8(void);
u32 func0f073c6c(struct defaultobj *obj, bool *arg1);
void doorTick(struct prop *doorprop);
void doorUpdatePortalIfWindowed(struct prop *door, s32 playercount);
u32 func0f077448(void);
void platformDisplaceProps(struct prop *platform, s16 *propnums, struct coord *prevpos, struct coord *newpos);
void liftTick(struct prop *lift);
void escalatorTick(struct prop *escalator);
void cameraTick(struct prop *camera);
u32 func0f078930(void);
void fanTick(struct prop *fan);
void func0f078be0(struct prop *fan);
void autogunTick(struct prop *autogun);
u32 func0f079ca4(void);
void func0f079f1c(struct prop *autogun);
u32 func0f07accc(void);
struct chopperobj *chopperFromHovercar(struct chopperobj *obj);
bool chopperCheckTargetInFov(struct chopperobj *hovercar, u8 param_2);
bool chopperCheckTargetInSight(struct chopperobj *obj);
void chopperSetTarget(struct chopperobj *obj, u32 chrnum);
bool chopperAttack(struct chopperobj *obj);
bool chopperStop(struct chopperobj *obj);
bool chopperSetArmed(struct chopperobj *obj, bool armed);
void chopperRestartTimer(struct chopperobj *obj);
f32 chopperGetTimer(struct chopperobj *heli);
void chopperSetMaxDamage(struct chopperobj *obj, u16 health);
u32 func0f07b164(void);
void chopperFireRocket(struct chopperobj *chopper, bool side);
u32 func0f07b3f0(void);
void func0f07ba38(struct prop *prop, f32 roty, f32 rotx, struct coord *coord, u32 arg4);
void chopperTickAttackMode2(struct prop *chopper);
void func0f07c7b0(struct prop *chopper);
void chopperTickAttackMode0(struct prop *chopper);
void chopperTickAttackMode1(struct prop *chopper);
void hovercarTick(struct prop *hovercar);
void hoverpropTick(struct prop *hoverprop, bool arg1);
void hoverbikeTick(struct prop *hoverbike, bool arg1);
void func0f07e058(struct prop *prop);
void func0f07e0b8(struct prop *glass, s32 playercount, bool *arg2);
u32 func0f07e184(void);
u32 func0f07e1fc(void);
void func0f07e2cc(struct prop *prop);
bool propCanRegen(struct prop *prop);
u32 func0f07e474(struct prop *prop);
s32 objTick(struct prop *prop);
Gfx *func0f07f6d0(Gfx *gdl);
u32 func0f07f918(void);
void imageSlotSetImage(struct monitorscreen *screen, u32 arg1);
u32 func0f07fbf0(void);
u32 func0f07fbf8(void);
u32 func0f0809c4(void);
u32 func0f080f8c(void);
u32 func0f081220(void);
u32 func0f081310(void);
u32 func0f081c18(void);
u32 func0f081ccc(void);
u32 func0f0826cc(void);
void propobjSetDropped(struct prop *prop, u32 reason);
void func0f082a1c(struct defaultobj *obj, struct coord *coord, f32 rotate, bool arg3, bool arg4);
void func0f082d74(void);
void func0f082e84(struct defaultobj *obj, struct coord *arg1, struct coord *arg2, struct coord *arg3, bool arg4);
void func0f082f88(struct prop *prop);
void func0f08307c(struct prop *prop, bool arg1);
u32 func0f083db0(void);
u32 func0f0840ac(void);
u32 func0f0841dc(void);
u32 func0f084594(void);
u32 func0f0849dc(void);
u32 add43214321(u32 value);
u32 func0f084cf0(void);
u32 func0f084e58(void);
u32 func0f084f64(void);
u32 func0f085050(void);
bool func0f085194(struct defaultobj *obj);
u32 func0f0851ec(void);
u32 func0f085270(void);
void func0f0852ac(struct defaultobj *obj, f32 damage, struct coord *pos, s32 arg3, s32 arg4);
u32 func0f0859a0(void);
u32 func0f085e00(void);
u32 func0f085eac(void);
u32 propobjGetCiTagId(struct prop *prop);
bool objIsHealthy(struct defaultobj *obj);
u32 func0f0869cc(void);
bool currentPlayerTryMountHoverbike(struct prop *prop);
bool propobjInteract(struct prop *prop);
void propObjSetOrUnsetHiddenFlag00400000(struct prop *prop, bool enable);
bool func0f087458(struct prop *prop, struct geo **geos, u32 *arg2);
void propObjGetBbox(struct prop *prop, f32 *width, f32 *ymax, f32 *ymin);
void ammotypeGetPickedUpText(char *dst);
void ammotypeGetDeterminer(char *dst, s32 ammotype, s32 qty);
void ammotypeGetPickupName(char *dst, s32 ammotype, s32 qty);
void ammotypePlayPickupSound(u32 ammotype);
u32 propPlayPickupSound(struct prop *prop, s32 weapon);
void func0f087d10(s32 weaponnum);
void ammotypeGetPickupMessage(char *dst, s32 ammotype, s32 qty);
void currentPlayerQueuePickupAmmoHudmsg(s32 ammotype, s32 pickupqty);
void func0f088028(s32 ammotype, s32 quantity, bool arg2, bool showhudmsg);
s32 ammocrateGetPickupAmmoQty(struct ammocrateobj *crate);
s32 weaponGetPickupAmmoQty(struct weaponobj *weapon);
void weaponGetPickupText(char *buffer, s32 weaponnum, bool dual);
void currentPlayerQueuePickupWeaponHudmsg(u32 weaponnum, bool dual);
u32 func0f088840(struct prop *prop, bool showhudmsg);
u32 func0f089014(void);
bool func0f0899dc(struct prop *prop, f32 *arg0, f32 *arg1, f32 *arg2);
void func0f089a94(bool arg0, struct model *model);
void func0f089c70(struct hatobj *hat, struct chrdata *chr, struct model08 *filedata, u32 arg3, u32 arg4);
void func0f089d64(struct hatobj *hat, struct chrdata *chr);
void hatAssignToChr(struct hatobj *hat, struct chrdata *chr);
u32 chrTryEquipHat(struct chrdata *chr, u32 model, u32 flags);
u32 func0f089f8c(void);
u32 func0f08a38c(void);
u32 func0f08a724(void);
u32 func0f08a88c(void);
void playerActivateRemoteMineDetonator(s32 playernum);
struct weaponobj *func0f08aa70(s32 weaponnum, struct prop *prop);
struct weaponobj *weaponFindThrown(s32 weaponnum);
u32 func0f08ab64(void);
u32 func0f08ab9c(void);
void func0f08abd4(struct coord *pos, u32 arg1);
void func0f08acb0(void);
void propweaponSetDual(struct weaponobj *weapon1, struct weaponobj *weapon2);
u32 func0f08adc8(void);
u32 func0f08ae0c(void);
u32 func0f08ae54(struct defaultobj *obj, struct chrdata *chr);
u32 func0f08b108(struct weaponobj *weapon, struct chrdata *chr, struct model08 *model08, u32 arg3, u32 arg4);
void func0f08b208(struct weaponobj *weapon, struct chrdata *chr);
void func0f08b25c(struct weaponobj *weapon, struct chrdata *chr);
u32 func0f08b27c(void);
u32 func0f08b658(void);
struct weaponobj *func0f08b880(u32 model, u32 weaponnum, struct chrdata *chr);
void chrSetWeaponFlag4(struct chrdata *chr, s32 slot);
struct prop *func0f08b8e8(struct chrdata *chr, s32 weaponnum, s32 arg2, u32 flags, s32 arg4, s32 arg5);
struct prop *chrGiveWeapon(struct chrdata *chr, s32 model, s32 weaponnum, u32 flags);
u32 func0f08bb3c(void);
void func0f08bb5c(struct prop *prop, bool firing, s32 room);
s32 func0f08bc5c(struct prop *prop);
u32 func0f08bcf4(void);
bool func0f08bd00(struct prop *playerprop, struct prop *doorprop);
u32 func0f08bdd4(void);
u32 func0f08be80(void);
u32 func0f08bf78(void);
s32 func0f08c040(struct doorobj *door);
void func0f08c190(void);
u32 func0f08c424(void);
u32 func0f08c484(void);
u32 func0f08c54c(struct doorobj *door);
u32 func0f08cb20(void);
u32 func0f08d3dc(void);
u32 func0f08d460(void);
void doorActivatePortal(struct doorobj *door);
void doorDeactivatePortal(struct doorobj *door);
struct prop *func0f08d540(struct doorobj *door, struct coord *pos, f32 *matrix, s16 *rooms, struct coord *coord, struct coord *centre);
void func0f08d784(s32 soundtype, struct prop *prop);
void func0f08daa8(s32 soundtype, struct prop *prop);
void func0f08dd44(s32 soundtype, struct prop *prop);
void func0f08df10(s32 soundtype, struct prop *prop);
void func0f08e0c4(struct doorobj *door);
void func0f08e1a0(struct doorobj *door);
u32 decodeXorAaaaaaaa(u32 value);
void func0f08e224(struct doorobj *door);
void func0f08e2ac(struct doorobj *door);
void doorSetMode(struct doorobj *door, s32 newmode);
void doorActivate(struct doorobj *door, s32 newmode);
s32 doorIsClosed(struct doorobj *door);
s32 doorIsOpen(struct doorobj *door);
u32 func0f08e5a8(void);
u32 func0f08e6bc(void);
u32 func0f08e794(void);
bool func0f08e8ac(struct prop *prop, struct coord *coord, f32 arg2, u32 arg3);
bool posIsInDrawDistance(struct coord *arg);
void func0f08ea50(struct doorobj *door);
bool func0f08ed74(struct doorobj *door);
void doorDoCalc(struct doorobj *door);
u32 func0f08f538(void);
u32 func0f08f604(void);
u32 func0f08f968(void);
u32 func0f08fcb8(void);
void doorActivateWrapper(struct prop *prop, bool arg1);
u32 func0f08fffc(void);
void func0f0900c0(struct prop *prop, struct doorobj *door);
bool propdoorInteract(struct prop *doorprop);
void alarmStopAudio(void);
u32 func0f09044c(void);
void func0f0904e0(void);
bool func0f090520(void);
void func0f09054c(void);
Gfx *func0f0908b8(Gfx *gdl);
u32 func0f090d34(void);
void alarmTick(void);
u32 func0f091030(void);
void func0f0910ac(void);
u32 func0f091250(void);
void projectileCreate(struct prop *fromprop, u32 arg1, struct coord *pos, struct coord *direction, s32 weaponnum, struct prop *targetprop);
void objSetModelPartVisible(struct defaultobj *obj, s32 partnum, bool visible);

#endif
