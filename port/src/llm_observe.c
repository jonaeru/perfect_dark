#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <ultra64.h>
#include "constants.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/mplayer/mplayer.h"
#include "game/playermgr.h"
#include "bss.h"
#include "data.h"
#include "types.h"

#include "llm_bridge.h"
#include "llm_internal.h"

static void llmObserveAppend(char *buf, s32 cap, s32 *pos, const char *fmt, ...)
{
	char tmp[512];
	va_list args;
	s32 n;

	va_start(args, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, args);
	va_end(args);

	if (*pos + n >= cap - 1) {
		return;
	}
	memcpy(buf + *pos, tmp, n);
	*pos += n;
	buf[*pos] = '\0';
}

void llmBridgeBuildObservation(void)
{
	char localJson[LLM_OBSERVE_CAP];
	s32 pos = 0;
	s32 i;
	s32 playercount = PLAYERCOUNT();
	s32 prevplayernum = g_Vars.currentplayernum;
	s32 firstActor = 1;

	if (!llmBridgeIsEnabled()) {
		return;
	}

	localJson[0] = '\0';

	llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos,
			"{\"tick\":%d,\"stage\":%d,\"paused\":%s,\"scenario\":%d,\"player_mask\":%d,\"bot_mask\":%d,"
			"\"players\":[",
			g_Vars.lvframe60,
			g_Vars.stagenum,
			mpIsPaused() ? "true" : "false",
			g_MpSetup.scenario,
			llmBridgeGetPlayerMask(),
			llmBridgeGetBotMask());

	for (i = 0; i < playercount; i++) {
		struct player *pl;
		struct chrdata *chr;

		setCurrentPlayerNum(i);
		pl = g_Vars.currentplayer;
		chr = pl && pl->prop ? pl->prop->chr : NULL;

		if (i > 0) {
			llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos, ",");
		}

		if (chr && chr->prop) {
			llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos,
					"{\"id\":%d,\"team\":%d,\"pos\":{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f},"
					"\"facing_deg\":%.1f,\"health\":%.3f,\"dead\":%s}",
					i,
					chr->team,
					chr->prop->pos.x, chr->prop->pos.y, chr->prop->pos.z,
					chrGetRotY(chr),
					pl->bondhealth,
					pl->isdead ? "true" : "false");
		} else {
			llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos,
					"{\"id\":%d,\"team\":0,\"pos\":{\"x\":0,\"y\":0,\"z\":0},\"facing_deg\":0,\"health\":0,\"dead\":true}",
					i);
		}
	}

	llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos, "],\"actors\":[");

	for (i = 0; i < g_MpNumChrs; i++) {
		struct chrdata *chr = g_MpAllChrPtrs[i];
		const char *kind = (i < playercount) ? "player" : "bot";
		s32 botcmd = -1;

		if (!chr || !chr->prop) {
			continue;
		}

		if (!firstActor) {
			llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos, ",");
		}
		firstActor = 0;

		if (chr->aibot) {
			botcmd = chr->aibot->command;
		}

		llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos,
				"{\"id\":%d,\"kind\":\"%s\",\"team\":%d,\"pos\":{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f},"
				"\"health\":%.3f,\"weapon\":%d,\"command\":%d,\"dead\":%s}",
				i,
				kind,
				chr->team,
				chr->prop->pos.x, chr->prop->pos.y, chr->prop->pos.z,
				chr->maxdamage > 0.f ? (chr->maxdamage - chr->damage) / chr->maxdamage : 1.f,
				chr->aibot ? chr->aibot->weaponnum : -1,
				botcmd,
				chrIsDead(chr) ? "true" : "false");
	}

	llmObserveAppend(localJson, LLM_OBSERVE_CAP, &pos, "],\"events\":[]}");
	setCurrentPlayerNum(prevplayernum);

	/* Publish snapshot under lock; build work above must not hold the HTTP mutex. */
	pthread_mutex_lock(&g_LlmMutex);
	memcpy(g_LlmObserveJson, localJson, (size_t)pos + 1U);
	g_LlmObserveLen = pos;
	pthread_mutex_unlock(&g_LlmMutex);
}
