#include <ultra64.h>

#include "llm_bridge.h"

void llmPlayerMergePad(s32 playernum, OSContPad *npad)
{
	const struct LlmPlayerPad *vp;

	if (!npad || playernum < 0 || playernum >= LLM_MAX_PLAYERS) {
		return;
	}

	if (!(llmBridgeGetPlayerMask() & (1 << playernum))) {
		return;
	}

	vp = llmPlayerGetPad(playernum);
	if (!vp) {
		return;
	}

	/* Full AI takeover for this player — do not OR with keyboard/mouse. */
	npad->button = vp->buttons;
	npad->stick_x = vp->stick_x;
	npad->stick_y = vp->stick_y;
	npad->rstick_x = vp->look_x;
	npad->rstick_y = vp->look_y;
}
