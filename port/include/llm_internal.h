#pragma once

#include <pthread.h>
#include <PR/ultratypes.h>

#include "llm_bridge.h"

#define LLM_OBSERVE_CAP 65536

extern char g_LlmObserveJson[LLM_OBSERVE_CAP];
extern s32 g_LlmObserveLen;
extern pthread_mutex_t g_LlmMutex;

struct LlmBotInput *llmBotGetInputForSlot(s32 slot);
