//
// Skedar Ruins
//

#include "../include/stagesetup.h"

u8 intro[];
u8 props[];
struct aipaths paths[];
struct ailists functions[];

void *setup[] = {
	NULL,
	NULL,
	NULL,
	intro,
	props,
	paths,
	functions,
	NULL,
};

u8 props[] = {
	endprops
};

u8 intro[] = {
	endintro
};

struct ailists functions[] = {
	{ NULL, 0 },
};

struct aipaths paths[] = {
	{ NULL, 0, 0 },
};

