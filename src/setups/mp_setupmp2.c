//
// Stage ID 0x3a: Stack / Custom Box Level (MP)
//

#include "stagesetup.h"

extern s32 intro[];
extern u32 props[];
extern struct path paths[];
extern struct ailist ailists[];

struct stagesetup setup = {
	NULL,
	NULL,
	NULL,
	intro,
	props,
	paths,
	ailists,
	NULL,
};

u32 props[] = {
	weapon(0x0100, 0x0000, PAD_MP2_0010, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION00)
	weapon(0x0100, 0x0000, PAD_MP2_0011, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION01)
	weapon(0x0100, 0x0000, PAD_MP2_0012, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION02)
	weapon(0x0100, 0x0000, PAD_MP2_0013, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION03)
	weapon(0x0100, 0x0000, PAD_MP2_0014, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION04)
	weapon(0x0100, 0x0000, PAD_MP2_0015, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION05)
	weapon(0x0100, 0x0000, PAD_MP2_0016, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION06)
	weapon(0x0100, 0x0000, PAD_MP2_0017, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION07)
	weapon(0x0100, 0x0000, PAD_MP2_0018, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION08)
	weapon(0x0100, 0x0000, PAD_MP2_0019, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION09)
	endprops
};

s32 intro[] = {
	spawn(PAD_MP2_0000)
	spawn(PAD_MP2_0001)
	spawn(PAD_MP2_0002)
	spawn(PAD_MP2_0003)
	spawn(PAD_MP2_0004)
	spawn(PAD_MP2_0005)
	spawn(PAD_MP2_0006)
	spawn(PAD_MP2_0007)
	spawn(PAD_MP2_0008)
	spawn(PAD_MP2_0009)
	spawn(PAD_MP2_000A)
	spawn(PAD_MP2_000B)
	spawn(PAD_MP2_000C)
	spawn(PAD_MP2_000D)
	spawn(PAD_MP2_000E)
	spawn(PAD_MP2_000F)
	case(0, PAD_MP2_001A)
	case_respawn(0, PAD_MP2_001B)
	case(1, PAD_MP2_001C)
	case_respawn(1, PAD_MP2_001D)
	case(2, PAD_MP2_001E)
	case_respawn(2, PAD_MP2_001F)
	case(3, PAD_MP2_0020)
	case_respawn(3, PAD_MP2_0021)
	hill(PAD_MP2_0022)
	hill(PAD_MP2_0023)
	hill(PAD_MP2_0024)
	hill(PAD_MP2_0025)
	endintro
};

s32 path00[] = {
	-1,
};

struct path paths[] = {
	{ NULL, 0, 0 },
};

u8 unregistered_func1[] = {
	endlist
};

u8 unregistered_func2[] = {
	mp_init_simulants
	rebuild_teams
	rebuild_squadrons
	set_ailist(CHR_SELF, GAILIST_IDLE)
	endlist
};

struct ailist ailists[] = {
	{ NULL, 0 },
};
