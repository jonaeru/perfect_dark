#include <string.h>
#include <ultra64.h>
#include "constants.h"
#include "game/bot.h"
#include "game/botact.h"
#include "game/botinv.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/mplayer/mplayer.h"
#include "game/playermgr.h"
#include "bss.h"
#include "data.h"
#include "types.h"

#include "llm_bridge.h"
#include "llm_internal.h"

static s32 llmBotSlotFromChr(struct chrdata *chr)
{
	s32 mpidx;

	if (!chr || !chr->aibot) {
		return -1;
	}

	mpidx = mpPlayerGetIndex(chr);
	if (mpidx < MAX_PLAYERS) {
		return -1;
	}

	return mpidx - MAX_PLAYERS;
}

s32 llmBotIsControlled(struct chrdata *chr)
{
	s32 slot;

	if (!llmBridgeIsEnabled()) {
		return 0;
	}

	slot = llmBotSlotFromChr(chr);
	if (slot < 0 || slot >= LLM_MAX_BOT_SLOTS) {
		return 0;
	}

	return (llmBridgeGetBotMask() & (1 << slot)) != 0;
}

s32 llmBotShouldSkipNativeAi(struct chrdata *chr)
{
	return llmBotIsControlled(chr);
}

s32 llmBotShouldSkipInvTick(struct chrdata *chr)
{
	struct LlmBotInput *input;
	s32 slot;

	if (!llmBotIsControlled(chr)) {
		return 0;
	}

	slot = llmBotSlotFromChr(chr);
	input = llmBotGetInputForSlot(slot);
	if (input && input->weapon_num >= 0) {
		return 0;
	}

	return 1;
}

s32 llmBotWantsFire(struct chrdata *chr)
{
	struct LlmBotInput *input;
	s32 slot;

	if (!llmBotIsControlled(chr)) {
		return 0;
	}

	slot = llmBotSlotFromChr(chr);
	input = llmBotGetInputForSlot(slot);
	if (!input || !input->active) {
		return 0;
	}

	return input->wants_fire ? 1 : 0;
}

void llmBotApplyInput(struct chrdata *chr)
{
	struct aibot *aibot;
	struct LlmBotInput *input;
	s32 slot;
	struct chrdata *targetchr;

	if (!llmBotIsControlled(chr) || !chr->aibot) {
		return;
	}

	slot = llmBotSlotFromChr(chr);
	input = llmBotGetInputForSlot(slot);
	if (!input || !input->active) {
		chr->aibot->speedmultforwards = 0.f;
		chr->aibot->speedmultsideways = 0.f;
		return;
	}

	aibot = chr->aibot;

	if (input->move_fwd < -1.f) input->move_fwd = -1.f;
	if (input->move_fwd > 1.f) input->move_fwd = 1.f;
	if (input->move_strafe < -1.f) input->move_strafe = -1.f;
	if (input->move_strafe > 1.f) input->move_strafe = 1.f;

	aibot->speedmultforwards = input->move_fwd;
	aibot->speedmultsideways = input->move_strafe;

	if (input->aim_yaw_delta != 0.f) {
		aibot->lookangle += input->aim_yaw_delta * 0.017453292f;
	}

	if (input->aim_pitch_delta != 0.f) {
		aibot->extraangle += input->aim_pitch_delta * 0.017453292f;
	}

	if (input->target_id >= 0 && input->target_id < g_MpNumChrs) {
		targetchr = g_MpAllChrPtrs[input->target_id];
		if (targetchr && targetchr->prop) {
			chr->target = targetchr->prop - g_Vars.props;
			aibot->attackingplayernum = mpPlayerGetIndex(targetchr);
			aibot->targetinsight = true;
			chr->myaction = MA_AIBOTATTACK;
			aibot->abortattacktimer60 = -1;
		}
	}

	if (input->reload) {
		botactReload(chr, HAND_RIGHT, true);
		if (chr->weapons_held[HAND_LEFT]) {
			botactReload(chr, HAND_LEFT, true);
		}
	}

	if (input->weapon_num >= WEAPON_UNARMED && input->weapon_num < WEAPON_SUICIDEPILL) {
		s32 func = (input->gun_func >= 0) ? input->gun_func : aibot->gunfunc;
		if (botinvGetItem(chr, input->weapon_num)) {
			botinvSwitchToWeapon(chr, input->weapon_num, func);
		}
	}

	if (input->fire) {
		aibot->shootdelaytimer60 = TICKS(120);
		aibot->targetinsight = true;
		chr->myaction = MA_AIBOTATTACK;
	}
}
