#include <ultra64.h>
#include "constants.h"
#include "game/game_006900.h"
#include "game/title.h"
#include "game/pdmode.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/filelist.h"
#include "game/mainmenu.h"
#include "game/endscreen.h"
#include "game/playermgr.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/setup.h"
#include "game/challenge.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/mplayer/mplayer.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "input.h"
#endif

void menuStop(void)
{
#ifndef PLATFORM_N64
	if (inputMouseIsEnabled()) {
		inputLockMouse(true);
	}
#endif

	if (var80062944) {
		var80062944 = 0;
		var80062948 = 0;
		joySetDefaultPfsPollInterval();
		func0f110bf8();
	}
}
