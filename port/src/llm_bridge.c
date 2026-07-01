#include <ultra64.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "platform.h"
#include "config.h"
#include "llm_bridge.h"
#include "llm_internal.h"
#include "system.h"
#include "utils.h"

#if PLATFORM_POSIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

static s32 g_LlmEnabled = 0;
static s32 g_LlmPort = LLM_DEFAULT_PORT;
static s32 g_LlmPlayerMask = 0;
static s32 g_LlmBotMask = 0;
static s32 g_LlmDecisionIntervalMs = 1000;
static s32 g_LlmActionTtlMs = 1500;

static struct LlmPlayerPad g_LlmPlayerPads[LLM_MAX_PLAYERS];
static struct LlmBotInput g_LlmBotInputs[LLM_MAX_BOT_SLOTS];

char g_LlmObserveJson[LLM_OBSERVE_CAP];
s32 g_LlmObserveLen = 0;
static u64 g_LlmLastObserveBuildUs = 0;
static u64 g_LlmLastActUs = 0;
static s32 g_LlmLastActTick = 0;
static s32 g_LlmAdapterConnected = 0;

pthread_mutex_t g_LlmMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_LlmHttpThread;
static volatile s32 g_LlmHttpRunning = 0;
static volatile s32 g_LlmHttpStop = 0;
static s32 g_LlmListenFd = -1;

s32 llmBridgeIsEnabled(void)
{
	return g_LlmEnabled;
}

s32 llmBridgeGetPlayerMask(void)
{
	return g_LlmPlayerMask;
}

s32 llmBridgeGetBotMask(void)
{
	return g_LlmBotMask;
}

static u64 llmNowUs(void)
{
	return sysGetMicroseconds();
}

static void llmExpireActions(void)
{
	const u64 now = llmNowUs();
	s32 i;

	for (i = 0; i < LLM_MAX_PLAYERS; i++) {
		if (g_LlmPlayerPads[i].active && now > g_LlmPlayerPads[i].expiry_us) {
			g_LlmPlayerPads[i].active = 0;
		}
	}

	for (i = 0; i < LLM_MAX_BOT_SLOTS; i++) {
		if (g_LlmBotInputs[i].active && now > g_LlmBotInputs[i].expiry_us) {
			g_LlmBotInputs[i].active = 0;
			g_LlmBotInputs[i].wants_fire = 0;
		}
	}
}

const struct LlmPlayerPad *llmPlayerGetPad(s32 playernum)
{
	if (playernum < 0 || playernum >= LLM_MAX_PLAYERS) {
		return NULL;
	}

	if (!g_LlmPlayerPads[playernum].active) {
		return NULL;
	}

	return &g_LlmPlayerPads[playernum];
}

struct LlmBotInput *llmBotGetInputForSlot(s32 slot)
{
	if (slot < 0 || slot >= LLM_MAX_BOT_SLOTS) {
		return NULL;
	}

	if (!g_LlmBotInputs[slot].active) {
		return NULL;
	}

	return &g_LlmBotInputs[slot];
}

static s32 llmJsonFindKey(const char *json, const char *key, const char **outStart, const char **outEnd)
{
	char pattern[128];
	const char *p;
	const char *colon;
	const char *value;

	snprintf(pattern, sizeof(pattern), "\"%s\"", key);
	p = strstr(json, pattern);
	if (!p) {
		return 0;
	}

	colon = strchr(p + strlen(pattern), ':');
	if (!colon) {
		return 0;
	}

	value = colon + 1;
	while (*value == ' ' || *value == '\t' || *value == '\n' || *value == '\r') {
		value++;
	}

	*outStart = value;
	*outEnd = value;

	if (*value == '"') {
		*outEnd = value + 1;
		while (**outEnd && **outEnd != '"') {
			if (**outEnd == '\\' && (*outEnd)[1]) {
				(*outEnd)++;
			}
			(*outEnd)++;
		}
		if (**outEnd == '"') {
			(*outEnd)++;
		}
		return 1;
	}

	if (*value == '{' || *value == '[') {
		s32 depth = 0;
		char open = *value;
		char close = (open == '{') ? '}' : ']';
		*outEnd = value;
		do {
			if (**outEnd == open) {
				depth++;
			} else if (**outEnd == close) {
				depth--;
			}
			(*outEnd)++;
		} while (**outEnd && depth > 0);
		return 1;
	}

	while (**outEnd && **outEnd != ',' && **outEnd != '}' && **outEnd != ']'
			&& **outEnd != '\n' && **outEnd != '\r') {
		(*outEnd)++;
	}
	return 1;
}

static s32 llmJsonParseBool(const char *start, const char *end, s32 *out)
{
	char tmp[16];
	s32 len = end - start;
	if (len <= 0 || len >= (s32)sizeof(tmp)) {
		return 0;
	}
	memcpy(tmp, start, len);
	tmp[len] = '\0';
	strTrim(tmp);
	if (!strcmp(tmp, "true") || !strcmp(tmp, "1")) {
		*out = 1;
		return 1;
	}
	if (!strcmp(tmp, "false") || !strcmp(tmp, "0")) {
		*out = 0;
		return 1;
	}
	return 0;
}

static s32 llmJsonParseInt(const char *start, const char *end, s32 *out)
{
	char tmp[32];
	s32 len = end - start;
	if (len <= 0 || len >= (s32)sizeof(tmp)) {
		return 0;
	}
	memcpy(tmp, start, len);
	tmp[len] = '\0';
	strTrim(tmp);
	*out = atoi(tmp);
	return 1;
}

static f32 llmJsonParseFloat(const char *start, const char *end)
{
	char tmp[32];
	s32 len = end - start;
	if (len <= 0 || len >= (s32)sizeof(tmp)) {
		return 0.f;
	}
	memcpy(tmp, start, len);
	tmp[len] = '\0';
	strTrim(tmp);
	return (f32)atof(tmp);
}

static u16 llmParseButtonName(const char *name, s32 len)
{
	char tmp[16];
	if (len <= 0 || len >= (s32)sizeof(tmp)) {
		return 0;
	}
	memcpy(tmp, name, len);
	tmp[len] = '\0';
	strTrim(tmp);

	if (!strcasecmp(tmp, "A")) return A_BUTTON;
	if (!strcasecmp(tmp, "B")) return B_BUTTON;
	if (!strcasecmp(tmp, "Z")) return Z_TRIG;
	if (!strcasecmp(tmp, "R")) return R_TRIG;
	if (!strcasecmp(tmp, "L")) return L_TRIG;
	if (!strcasecmp(tmp, "START")) return START_BUTTON;
	if (!strcasecmp(tmp, "CU") || !strcasecmp(tmp, "C_UP")) return U_CBUTTONS;
	if (!strcasecmp(tmp, "CD") || !strcasecmp(tmp, "C_DOWN")) return D_CBUTTONS;
	if (!strcasecmp(tmp, "CL") || !strcasecmp(tmp, "C_LEFT")) return L_CBUTTONS;
	if (!strcasecmp(tmp, "CR") || !strcasecmp(tmp, "C_RIGHT")) return R_CBUTTONS;
	if (!strcasecmp(tmp, "DU") || !strcasecmp(tmp, "UP")) return U_JPAD;
	if (!strcasecmp(tmp, "DD") || !strcasecmp(tmp, "DOWN")) return D_JPAD;
	if (!strcasecmp(tmp, "DL") || !strcasecmp(tmp, "LEFT")) return L_JPAD;
	if (!strcasecmp(tmp, "DR") || !strcasecmp(tmp, "RIGHT")) return R_JPAD;
	return 0;
}

static void llmParsePlayerObject(const char *objStart, const char *objEnd)
{
	const char *vstart;
	const char *vend;
	struct LlmPlayerPad pad;
	const u64 ttl = (u64)g_LlmActionTtlMs * 1000ULL;

	memset(&pad, 0, sizeof(pad));
	pad.expiry_us = llmNowUs() + ttl;

	if (llmJsonFindKey(objStart, "stick_x", &vstart, &vend)) {
		s32 val;
		if (llmJsonParseInt(vstart, vend, &val)) {
			if (val < -128) val = -128;
			if (val > 127) val = 127;
			pad.stick_x = (s8)val;
		}
	}
	if (llmJsonFindKey(objStart, "stick_y", &vstart, &vend)) {
		s32 val;
		if (llmJsonParseInt(vstart, vend, &val)) {
			if (val < -128) val = -128;
			if (val > 127) val = 127;
			pad.stick_y = (s8)val;
		}
	}
	if (llmJsonFindKey(objStart, "look_x", &vstart, &vend)) {
		s32 val;
		if (llmJsonParseInt(vstart, vend, &val)) {
			pad.look_x = (s8)val;
		}
	}
	if (llmJsonFindKey(objStart, "look_y", &vstart, &vend)) {
		s32 val;
		if (llmJsonParseInt(vstart, vend, &val)) {
			pad.look_y = (s8)val;
		}
	}
	if (llmJsonFindKey(objStart, "buttons", &vstart, &vend) && *vstart == '[') {
		const char *p = vstart + 1;
		while (p < vend && *p != ']') {
			while (p < vend && (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r')) {
				p++;
			}
			if (*p == '"') {
				const char *q = p + 1;
				while (q < vend && *q && *q != '"') {
					q++;
				}
				pad.buttons |= llmParseButtonName(p + 1, (s32)(q - p - 1));
				p = q + 1;
			} else {
				break;
			}
		}
	}

	pad.active = 1;
	g_LlmPlayerPads[0] = pad;
}

static void llmParseBotObject(const char *objStart, const char *objEnd)
{
	const char *vstart;
	const char *vend;
	struct LlmBotInput input;
	s32 slot = -1;
	const u64 ttl = (u64)g_LlmActionTtlMs * 1000ULL;

	memset(&input, 0, sizeof(input));
	input.target_id = -1;
	input.weapon_num = -1;
	input.gun_func = -1;
	input.expiry_us = llmNowUs() + ttl;

	if (llmJsonFindKey(objStart, "slot", &vstart, &vend)) {
		llmJsonParseInt(vstart, vend, &slot);
	}
	if (slot < 0 || slot >= LLM_MAX_BOT_SLOTS) {
		return;
	}

	if (llmJsonFindKey(objStart, "move_fwd", &vstart, &vend)) {
		input.move_fwd = llmJsonParseFloat(vstart, vend);
	}
	if (llmJsonFindKey(objStart, "move_strafe", &vstart, &vend)) {
		input.move_strafe = llmJsonParseFloat(vstart, vend);
	}
	if (llmJsonFindKey(objStart, "aim_yaw_delta", &vstart, &vend)) {
		input.aim_yaw_delta = llmJsonParseFloat(vstart, vend);
	}
	if (llmJsonFindKey(objStart, "aim_pitch_delta", &vstart, &vend)) {
		input.aim_pitch_delta = llmJsonParseFloat(vstart, vend);
	}
	if (llmJsonFindKey(objStart, "fire", &vstart, &vend)) {
		s32 fireVal = 0;
		llmJsonParseBool(vstart, vend, &fireVal);
		input.fire = fireVal ? 1 : 0;
		input.wants_fire = input.fire;
	}
	if (llmJsonFindKey(objStart, "reload", &vstart, &vend)) {
		s32 reloadVal = 0;
		llmJsonParseBool(vstart, vend, &reloadVal);
		input.reload = reloadVal ? 1 : 0;
	}
	if (llmJsonFindKey(objStart, "target_id", &vstart, &vend)) {
		s32 targetVal = -1;
		llmJsonParseInt(vstart, vend, &targetVal);
		input.target_id = (s16)targetVal;
	}
	if (llmJsonFindKey(objStart, "weapon_num", &vstart, &vend)) {
		s32 weaponVal = -1;
		llmJsonParseInt(vstart, vend, &weaponVal);
		input.weapon_num = (s16)weaponVal;
	}
	if (llmJsonFindKey(objStart, "gun_func", &vstart, &vend)) {
		s32 func;
		if (llmJsonParseInt(vstart, vend, &func)) {
			input.gun_func = (s8)func;
		}
	}

	input.active = 1;
	g_LlmBotInputs[slot] = input;
}

static void llmParseActJson(const char *json)
{
	const char *vstart;
	const char *vend;

	if (llmJsonFindKey(json, "player", &vstart, &vend) && *vstart == '{') {
		llmParsePlayerObject(vstart, vend);
	}

	if (llmJsonFindKey(json, "bots", &vstart, &vend) && *vstart == '[') {
		const char *p = vstart + 1;
		while (p < vend && *p != ']') {
			while (p < vend && (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r')) {
				p++;
			}
			if (*p == '{') {
				const char *objStart = p;
				s32 depth = 0;
				do {
					if (*p == '{') depth++;
					else if (*p == '}') depth--;
					p++;
				} while (p < vend && depth > 0);
				llmParseBotObject(objStart, p);
			} else {
				break;
			}
		}
	}

	g_LlmAdapterConnected = 1;
	g_LlmLastActUs = llmNowUs();
}

void llmBridgeBuildObservation(void);

static void llmSendHttpResponse(int fd, s32 status, const char *contentType, const char *body, s32 bodyLen)
{
	char header[512];
	s32 headerLen;

	headerLen = snprintf(header, sizeof(header),
			"HTTP/1.1 %d OK\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n"
			"Access-Control-Allow-Origin: http://127.0.0.1\r\n"
			"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Content-Type\r\n"
			"Connection: close\r\n\r\n",
			status, contentType, bodyLen);

	send(fd, header, headerLen, 0);
	if (body && bodyLen > 0) {
		send(fd, body, bodyLen, 0);
	}
}

static s32 llmOriginIsLocalhost(const char *origin)
{
	if (!origin || !origin[0]) {
		return 1;
	}
	return !strncmp(origin, "http://127.0.0.1", 16)
		|| !strncmp(origin, "http://localhost", 16)
		|| !strcmp(origin, "null");
}

static void llmHandleClient(int clientFd)
{
	char req[16384];
	char body[12288];
	s32 total = 0;
	s32 contentLen = 0;
	char *lineEnd;
	char *bodyStart;
	const char *method = "GET";
	const char *path = "/";
	const char *origin = NULL;
	s32 i;

	while (total < (s32)sizeof(req) - 1) {
		s32 n = (s32)recv(clientFd, req + total, sizeof(req) - 1 - total, 0);
		if (n <= 0) {
			break;
		}
		total += n;
		req[total] = '\0';
		if (strstr(req, "\r\n\r\n")) {
			break;
		}
	}

	lineEnd = strstr(req, "\r\n");
	if (lineEnd) {
		static char methodBuf[16];
		static char pathBuf[256];
		char *query;

		if (sscanf(req, "%15s %255s", methodBuf, pathBuf) == 2) {
			method = methodBuf;
			query = strchr(pathBuf, '?');
			if (query) {
				*query = '\0';
			}
			path = pathBuf;
		}
	}

	for (i = 0; i < total - 6; i++) {
		if (!strncasecmp(req + i, "Origin:", 7)) {
			origin = req + i + 7;
			while (*origin == ' ') origin++;
			break;
		}
		if (!strncasecmp(req + i, "Content-Length:", 15)) {
			contentLen = atoi(req + i + 15);
		}
	}

	if (!llmOriginIsLocalhost(origin) && origin) {
		llmSendHttpResponse(clientFd, 403, "application/json", "{\"error\":\"forbidden origin\"}", 30);
		close(clientFd);
		return;
	}

	if (!strcmp(method, "OPTIONS")) {
		llmSendHttpResponse(clientFd, 204, "text/plain", "", 0);
		close(clientFd);
		return;
	}

	bodyStart = strstr(req, "\r\n\r\n");
	if (bodyStart) {
		bodyStart += 4;
		if (contentLen > 0 && contentLen < (s32)sizeof(body)) {
			memcpy(body, bodyStart, contentLen);
			body[contentLen] = '\0';
		} else if (bodyStart < req + total) {
			strncpy(body, bodyStart, sizeof(body) - 1);
			body[sizeof(body) - 1] = '\0';
		}
	}

	if (!strcmp(path, "/api/llm/health")) {
		char resp[512];
		s32 len = snprintf(resp, sizeof(resp),
				"{\"ok\":true,\"enabled\":%d,\"port\":%d,\"adapter_connected\":%d,"
				"\"player_mask\":%d,\"bot_mask\":%d,\"observe_len\":%d}",
				g_LlmEnabled, g_LlmPort, g_LlmAdapterConnected,
				g_LlmPlayerMask, g_LlmBotMask, g_LlmObserveLen);
		llmSendHttpResponse(clientFd, 200, "application/json", resp, len);
	} else if (!strcmp(path, "/api/llm/observe")) {
		char observeCopy[LLM_OBSERVE_CAP];
		s32 observeLen = 0;

		pthread_mutex_lock(&g_LlmMutex);
		observeLen = g_LlmObserveLen;
		if (observeLen > 0 && observeLen < LLM_OBSERVE_CAP) {
			memcpy(observeCopy, g_LlmObserveJson, (size_t)observeLen + 1U);
		}
		pthread_mutex_unlock(&g_LlmMutex);
		llmSendHttpResponse(clientFd, 200, "application/json", observeCopy, observeLen);
	} else if (!strcmp(path, "/api/llm/act") && !strcmp(method, "POST")) {
		pthread_mutex_lock(&g_LlmMutex);
		llmParseActJson(body);
		g_LlmLastActTick++;
		{
			char resp[256];
			s32 len = snprintf(resp, sizeof(resp),
					"{\"ok\":true,\"applied_tick\":%d}", g_LlmLastActTick);
			llmSendHttpResponse(clientFd, 200, "application/json", resp, len);
		}
		pthread_mutex_unlock(&g_LlmMutex);
	} else if (!strcmp(path, "/api/llm/config") && !strcmp(method, "POST")) {
		const char *vstart;
		const char *vend;
		char resp[128];
		s32 len;

		pthread_mutex_lock(&g_LlmMutex);
		if (llmJsonFindKey(body, "player_mask", &vstart, &vend)) {
			llmJsonParseInt(vstart, vend, &g_LlmPlayerMask);
		}
		if (llmJsonFindKey(body, "bot_mask", &vstart, &vend)) {
			llmJsonParseInt(vstart, vend, &g_LlmBotMask);
		}
		len = snprintf(resp, sizeof(resp),
				"{\"ok\":true,\"player_mask\":%d,\"bot_mask\":%d}",
				g_LlmPlayerMask, g_LlmBotMask);
		pthread_mutex_unlock(&g_LlmMutex);
		llmSendHttpResponse(clientFd, 200, "application/json", resp, len);
	} else {
		llmSendHttpResponse(clientFd, 404, "application/json", "{\"error\":\"not found\"}", 21);
	}

	close(clientFd);
}

static void *llmClientThreadMain(void *arg)
{
	const int clientFd = (int)(intptr_t)arg;

	llmHandleClient(clientFd);
	return NULL;
}

static void *llmHttpThreadMain(void *arg)
{
	(void)arg;

	while (!g_LlmHttpStop) {
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		int clientFd = accept(g_LlmListenFd, (struct sockaddr *)&addr, &addrlen);
		pthread_t clientThread;

		if (clientFd < 0) {
			if (g_LlmHttpStop) {
				break;
			}
			sysSleep(100000);
			continue;
		}

		/* Serve each client on its own thread so /observe cannot block /act. */
		if (pthread_create(&clientThread, NULL, llmClientThreadMain, (void *)(intptr_t)clientFd) != 0) {
			llmHandleClient(clientFd);
		} else {
			pthread_detach(clientThread);
		}
	}

	return NULL;
}

static s32 llmStartHttpServer(void)
{
	struct sockaddr_in addr;
	int opt = 1;

	g_LlmListenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_LlmListenFd < 0) {
		sysLogPrintf(LOG_ERROR, "LLM bridge: socket() failed\n");
		return 0;
	}

	setsockopt(g_LlmListenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((u16)g_LlmPort);

	if (bind(g_LlmListenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		sysLogPrintf(LOG_ERROR, "LLM bridge: bind(%d) failed\n", g_LlmPort);
		close(g_LlmListenFd);
		g_LlmListenFd = -1;
		return 0;
	}

	if (listen(g_LlmListenFd, 32) < 0) {
		sysLogPrintf(LOG_ERROR, "LLM bridge: listen() failed\n");
		close(g_LlmListenFd);
		g_LlmListenFd = -1;
		return 0;
	}

	g_LlmHttpStop = 0;
	if (pthread_create(&g_LlmHttpThread, NULL, llmHttpThreadMain, NULL) != 0) {
		sysLogPrintf(LOG_ERROR, "LLM bridge: pthread_create() failed\n");
		close(g_LlmListenFd);
		g_LlmListenFd = -1;
		return 0;
	}

	g_LlmHttpRunning = 1;
	sysLogPrintf(LOG_NOTE, "LLM bridge listening on 127.0.0.1:%d\n", g_LlmPort);
	return 1;
}

void llmBridgeInit(void)
{
	if (!g_LlmEnabled) {
		return;
	}
	llmStartHttpServer();
}

void llmBridgeShutdown(void)
{
	if (!g_LlmHttpRunning) {
		return;
	}

	g_LlmHttpStop = 1;
	if (g_LlmListenFd >= 0) {
		shutdown(g_LlmListenFd, SHUT_RDWR);
		close(g_LlmListenFd);
		g_LlmListenFd = -1;
	}
	pthread_join(g_LlmHttpThread, NULL);
	g_LlmHttpRunning = 0;
}

void llmBridgeTick(void)
{
	if (!g_LlmEnabled) {
		return;
	}

	llmExpireActions();
	llmBridgeBuildObservation();
}

PD_CONSTRUCTOR static void llmConfigInit(void)
{
	configRegisterInt("LLM.Enabled", &g_LlmEnabled, 0, 1);
	configRegisterInt("LLM.Port", &g_LlmPort, 1024, 65535);
	configRegisterInt("LLM.PlayerMask", &g_LlmPlayerMask, 0, 15);
	configRegisterInt("LLM.BotMask", &g_LlmBotMask, 0, 255);
	configRegisterInt("LLM.DecisionIntervalMs", &g_LlmDecisionIntervalMs, 100, 10000);
	configRegisterInt("LLM.ActionTtlMs", &g_LlmActionTtlMs, 100, 30000);
}
