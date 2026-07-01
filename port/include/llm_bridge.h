#pragma once

/* Implements Game LLM Bridge Protocol v1 — see tools/game_llm/protocols/BRIDGE_API.md */

#include <PR/ultratypes.h>
#include <PR/os_cont.h>

struct chrdata;

#define LLM_MAX_BOT_SLOTS 8
#define LLM_MAX_PLAYERS   4
#define LLM_DEFAULT_PORT  8780

struct LlmPlayerPad {
	s8 stick_x;
	s8 stick_y;
	s8 look_x;
	s8 look_y;
	u16 buttons;
	u64 expiry_us;
	u8 active;
};

struct LlmBotInput {
	f32 move_fwd;
	f32 move_strafe;
	f32 aim_yaw_delta;
	f32 aim_pitch_delta;
	u8 fire;
	u8 reload;
	s16 target_id;
	s16 weapon_num;
	s8 gun_func;
	u8 wants_fire;
	u64 expiry_us;
	u8 active;
};

void llmBridgeInit(void);
void llmBridgeShutdown(void);
void llmBridgeTick(void);

s32 llmBridgeIsEnabled(void);
s32 llmBridgeGetPlayerMask(void);
s32 llmBridgeGetBotMask(void);

const struct LlmPlayerPad *llmPlayerGetPad(s32 playernum);
void llmPlayerMergePad(s32 playernum, OSContPad *npad);

s32 llmBotIsControlled(struct chrdata *chr);
s32 llmBotWantsFire(struct chrdata *chr);
void llmBotApplyInput(struct chrdata *chr);
s32 llmBotShouldSkipNativeAi(struct chrdata *chr);
s32 llmBotShouldSkipInvTick(struct chrdata *chr);
