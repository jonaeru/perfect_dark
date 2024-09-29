#ifndef _IN_NETMSG_H
#define _IN_NETMSG_H

#include <PR/ultratypes.h>
#include "net/net.h"
#include "net/netbuf.h"

#define SVC_BAD           0x00 // trash
#define SVC_NOP           0x01 // does nothing
#define SVC_AUTH          0x02 // auth response, sent in response to CLC_AUTH
#define SVC_CHAT          0x03 // chat message
#define SVC_STAGE_START   0x10 // start level
#define SVC_STAGE_END     0x11 // end level
#define SVC_PLAYER_MOVE   0x20 // player movement and inputs
#define SVC_PLAYER_GUNS   0x21 // player gun state
#define SVC_PLAYER_STATS  0x22 // player stats (health etc)
#define SVC_PROP_MOVE     0x30 // prop movement
#define SVC_PROP_SPAWN    0x31 // new prop spawned
#define SVC_PROP_DAMAGE   0x32 // prop was damaged
#define SVC_PROP_PICKUP   0x33 // prop was picked up
#define SVC_PROP_USE      0x34 // door/lift/etc was used
#define SVC_PROP_DOOR     0x35 // door state changed
#define SVC_PROP_LIFT     0x36 // lift state changed
#define SVC_CHR_DAMAGE    0x42 // chr was damaged
#define SVC_CHR_DISARM    0x43 // chr's weapons were dropped

#define CLC_BAD      0x00 // trash
#define CLC_NOP      0x01 // does nothing
#define CLC_AUTH     0x02 // auth request, sent immediately after connecting
#define CLC_CHAT     0x03 // chat message
#define CLC_MOVE     0x04 // player input
#define CLC_SETTINGS 0x05 // player settings changed

u32 netmsgClcAuthWrite(struct netbuf *dst);
u32 netmsgClcAuthRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgClcChatWrite(struct netbuf *dst, const char *str);
u32 netmsgClcChatRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgClcMoveWrite(struct netbuf *dst);
u32 netmsgClcMoveRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgClcSettingsWrite(struct netbuf *dst);
u32 netmsgClcSettingsRead(struct netbuf *src, struct netclient *srccl);

u32 netmsgSvcAuthWrite(struct netbuf *dst, struct netclient *authcl);
u32 netmsgSvcAuthRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcChatWrite(struct netbuf *dst, const char *str);
u32 netmsgSvcChatRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcStageStartWrite(struct netbuf *dst);
u32 netmsgSvcStageStartRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcStageEndWrite(struct netbuf *dst);
u32 netmsgSvcStageEndRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPlayerMoveWrite(struct netbuf *dst, struct netclient *movecl);
u32 netmsgSvcPlayerMoveRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPlayerStatsWrite(struct netbuf *dst, struct netclient *actcl);
u32 netmsgSvcPlayerStatsRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropSpawnWrite(struct netbuf *dst, struct prop *prop);
u32 netmsgSvcPropSpawnRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropMoveWrite(struct netbuf *dst, struct prop *prop, struct coord *initrot);
u32 netmsgSvcPropMoveRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropDamageWrite(struct netbuf *dst, struct prop *prop, f32 damage, struct coord *pos, s32 weaponnum, s32 playernum);
u32 netmsgSvcPropDamageRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropPickupWrite(struct netbuf *dst, struct netclient *actcl, struct prop *prop, const s32 tickop);
u32 netmsgSvcPropPickupRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropUseWrite(struct netbuf *dst, struct prop *prop, struct netclient *usercl, const s32 tickop);
u32 netmsgSvcPropUseRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropDoorWrite(struct netbuf *dst, struct prop *prop, struct netclient *usercl);
u32 netmsgSvcPropDoorRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcPropLiftWrite(struct netbuf *dst, struct prop *prop);
u32 netmsgSvcPropLiftRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcChrDamageWrite(struct netbuf *dst, struct chrdata *chr, f32 damage, struct coord *vector, struct gset *gset, struct prop *aprop, s32 hitpart, bool damageshield, struct prop *prop2, s32 side, s16 *arg11, bool explosion, struct coord *explosionpos);
u32 netmsgSvcChrDamageRead(struct netbuf *src, struct netclient *srccl);
u32 netmsgSvcChrDisarmWrite(struct netbuf *dst, struct chrdata *chr, struct prop *attacker, u8 weaponnum, f32 wpndamage, struct coord *wpnpos);
u32 netmsgSvcChrDisarmRead(struct netbuf *src, struct netclient *srccl);

#endif
