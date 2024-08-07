#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "system.h"

enum {
	AL_ADPCM_WAVE = 0,
	AL_RAW16_WAVE
};

struct generic_adpcm_book {
	s32 order;
	s32 npredictors;
	s16 book[128];
};

struct generic_adpcm_loop {
	u32 start;
	u32 end;
	u32 count;
	s16 state[16];
};

struct generic_raw_loop {
	u32 start;
	u32 end;
	u32 count;
};

struct generic_envelope {
	s32 attackTime;
	s32 decayTime;
	s32 releaseTime;
	u8 attackVolume;
	u8 decayVolume;
};

struct generic_keymap {
	u8 velocityMin;
	u8 velocityMax;
	u8 keyMin;
	u8 keyMax;
	u8 keyBase;
	s8 detune;
};

struct n64_adpcm_waveinfo {
	u32 loop; // ptr to struct generic_adpcm_loop
	u32 book; // ptr to struct generic_adpcm_book
};

struct host_adpcm_waveinfo {
	 uintptr_t loop; // ptr to struct generic_adpcm_loop
	 uintptr_t book; // ptr to struct generic_adpcm_book
};

struct n64_raw_waveinfo {
	u32 loop; // ptr to struct generic_raw_loop
};

struct host_raw_waveinfo {
	uintptr_t loop; // ptr to struct generic_raw_loop
};

struct n64_wavetable {
	u32 base;
	s32 len;
	u8 type;
	u8 flags;
	union {
		struct n64_adpcm_waveinfo adpcmWave;
		struct n64_raw_waveinfo rawWave;
	} waveInfo;
};

struct host_wavetable {
	uintptr_t base;
	s32 len;
	u8 type;
	u8 flags;
	union {
		struct host_adpcm_waveinfo adpcmWave;
		struct host_raw_waveinfo rawWave;
	} waveInfo;
};

struct n64_sound {
	u32 envelope; // ptr to struct generic_envelope
	u32 keyMap; // ptr to struct generic_keymap
	u32 wavetable; // ptr to struct n64_wavetable
	u8 samplePan;
	u8 sampleVolume;
	u8 flags;
};

struct host_sound {
	uintptr_t envelope; // ptr to struct generic_envelope
	uintptr_t keyMap; // ptr to struct generic_keymap
	uintptr_t wavetable; // ptr to struct host_wavetable
	u8 samplePan;
	u8 sampleVolume;
	u8 flags;
};

struct n64_instrument {
	u8 volume;
	u8 pan;
	u8 priority;
	u8 flags;
	u8 tremType;
	u8 tremRate;
	u8 tremDepth;
	u8 tremDelay;
	u8 vibType;
	u8 vibRate;
	u8 vibDepth;
	u8 vibDelay;
	s16 bendRange;
	s16 soundCount;
	u32 soundArray[1]; // ptr to struct n64_sound
};

struct host_instrument {
	u8 volume;
	u8 pan;
	u8 priority;
	u8 flags;
	u8 tremType;
	u8 tremRate;
	u8 tremDepth;
	u8 tremDelay;
	u8 vibType;
	u8 vibRate;
	u8 vibDepth;
	u8 vibDelay;
	s16 bendRange;
	s16 soundCount;
	uintptr_t soundArray[1]; // ptr to struct host_sound
};

struct n64_bank {
	s16 instCount;
	u8 flags;
	u8 pad;
	s32 sampleRate;
	u32 percussion; // ptr to struct n64_instrument
	u32 instArray[1]; // ptr to struct n64_instrument
};

struct host_bank {
	s16 instCount;
	u8 flags;
	u8 pad;
	s32 sampleRate;
	uintptr_t percussion; // ptr to struct host_instrument
	uintptr_t instArray[1]; // ptr to struct host_instrument
};

struct n64_bankfile {
	s16 revision;
	s16 bankCount;
	u32 bankArray[1]; // ptr to struct n64_bank
};

struct host_bankfile {
	s16 revision;
	s16 bankCount;
	uintptr_t bankArray[1]; // ptr to struct host_bank
};

// only proceeds to convert the next item if it's not already converted
#define AL_NEXT_ITEM(field, func) { \
	struct ptrmarker *marker = find_ptr_marker(srcpos); \
	if (marker == NULL) { \
		field = htodst32(dstpos); \
		add_marker(srcpos, field); \
		dstpos = func(dst, dstpos, src, srcpos); \
	} else { \
		field = marker->ptr_host; } \
}

static int convert_audio_envelope(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct generic_envelope *n64_envelope = (struct generic_envelope *) &src[srcpos];
	struct generic_envelope *host_envelope = (struct generic_envelope *) &dst[dstpos];

	dstpos += sizeof(struct generic_envelope);

	host_envelope->attackTime = srctodst32(n64_envelope->attackTime);
	host_envelope->decayTime = srctodst32(n64_envelope->decayTime);
	host_envelope->releaseTime = srctodst32(n64_envelope->releaseTime);
	host_envelope->attackVolume = n64_envelope->attackVolume;
	host_envelope->decayVolume = n64_envelope->decayVolume;

	return dstpos;
}

static int convert_audio_keymap(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct generic_keymap *n64_keymap = (struct generic_keymap *) &src[srcpos];
	struct generic_keymap *host_keymap = (struct generic_keymap *) &dst[dstpos];

	dstpos += sizeof(struct generic_keymap);

	host_keymap->velocityMin = n64_keymap->velocityMin;
	host_keymap->velocityMax = n64_keymap->velocityMax;
	host_keymap->keyMin = n64_keymap->keyMin;
	host_keymap->keyMax = n64_keymap->keyMax;
	host_keymap->keyBase = n64_keymap->keyBase;
	host_keymap->detune = n64_keymap->detune;

	return dstpos;
}

static int convert_audio_adpcm_loop(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct generic_adpcm_loop *n64_loop = (struct generic_adpcm_loop *) &src[srcpos];
	struct generic_adpcm_loop *host_loop = (struct generic_adpcm_loop *) &dst[dstpos];

	dstpos += sizeof(struct generic_adpcm_loop);

	host_loop->start = srctodst32(n64_loop->start);
	host_loop->end = srctodst32(n64_loop->end);
	host_loop->count = srctodst32(n64_loop->count);

	for (int i = 0; i < ARRAYCOUNT(host_loop->state); i++) {
		host_loop->state[i] = srctodst16(n64_loop->state[i]);
	}

	return dstpos;
}

static int convert_audio_adpcm_book(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct generic_adpcm_book *n64_book = (struct generic_adpcm_book *) &src[srcpos];
	struct generic_adpcm_book *host_book = (struct generic_adpcm_book *) &dst[dstpos];

	dstpos += sizeof(struct generic_adpcm_book);

	host_book->order = srctodst32(n64_book->order);
	host_book->npredictors = srctodst32(n64_book->npredictors);

	for (int i = 0; i < ARRAYCOUNT(host_book->book); i++) {
		host_book->book[i] = srctodst16(n64_book->book[i]);
	}

	return dstpos;
}

static int convert_audio_raw_loop(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct generic_raw_loop *n64_loop = (struct generic_raw_loop *) &src[srcpos];
	struct generic_raw_loop *host_loop = (struct generic_raw_loop *) &dst[dstpos];

	dstpos += sizeof(struct generic_raw_loop);

	host_loop->start = srctodst32(n64_loop->start);
	host_loop->end = srctodst32(n64_loop->end);
	host_loop->count = srctodst32(n64_loop->count);

	return dstpos;
}

static int convert_audio_wavetable(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_wavetable *n64_wavetable = (struct n64_wavetable *) &src[srcpos];
	struct host_wavetable *host_wavetable = (struct host_wavetable *) &dst[dstpos];

	dstpos += sizeof(struct host_wavetable);

	host_wavetable->base = srctodst32(n64_wavetable->base);
	host_wavetable->len = srctodst32(n64_wavetable->len);
	host_wavetable->type = n64_wavetable->type;
	host_wavetable->flags = n64_wavetable->flags;

	if (host_wavetable->type == AL_ADPCM_WAVE) {
		if (n64_wavetable->waveInfo.adpcmWave.loop) {
			srcpos = srctoh32(n64_wavetable->waveInfo.adpcmWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.loop, convert_audio_adpcm_loop);
		} else {
			host_wavetable->waveInfo.adpcmWave.loop = 0;
		}

		if (n64_wavetable->waveInfo.adpcmWave.book) {
			srcpos = srctoh32(n64_wavetable->waveInfo.adpcmWave.book);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.book, convert_audio_adpcm_book);
		} else {
			host_wavetable->waveInfo.adpcmWave.book = 0;
		}
	} else if (host_wavetable->type == AL_RAW16_WAVE) {
		if (n64_wavetable->waveInfo.rawWave.loop) {
			srcpos = srctoh32(n64_wavetable->waveInfo.rawWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.rawWave.loop, convert_audio_raw_loop);
		} else {
			host_wavetable->waveInfo.rawWave.loop = 0;
		}
	}

	return dstpos;
}

static int convert_audio_sound(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_sound *n64_sound = (struct n64_sound *) &src[srcpos];
	struct host_sound *host_sound = (struct host_sound *) &dst[dstpos];

	dstpos += sizeof(struct host_sound);

	if (n64_sound->envelope) {
		srcpos = srctoh32(n64_sound->envelope);
		AL_NEXT_ITEM(host_sound->envelope, convert_audio_envelope);
	} else {
		host_sound->envelope = 0;
	}

	if (n64_sound->keyMap) {
		srcpos = srctoh32(n64_sound->keyMap);
		AL_NEXT_ITEM(host_sound->keyMap, convert_audio_keymap);
	} else {
		host_sound->keyMap = 0;
	}

	if (n64_sound->wavetable) {
		srcpos = srctoh32(n64_sound->wavetable);
		AL_NEXT_ITEM(host_sound->wavetable, convert_audio_wavetable);
	} else {
		host_sound->wavetable = 0;
	}

	host_sound->samplePan = n64_sound->samplePan;
	host_sound->sampleVolume = n64_sound->sampleVolume;
	host_sound->flags = n64_sound->flags;

	return dstpos;
}

static int convert_audio_instrument(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_instrument *n64_instrument = (struct n64_instrument *) &src[srcpos];
	struct host_instrument *host_instrument = (struct host_instrument *) &dst[dstpos];
	int soundCount = srctoh16(n64_instrument->soundCount);

	host_instrument->volume = n64_instrument->volume;
	host_instrument->pan = n64_instrument->pan;
	host_instrument->priority = n64_instrument->priority;
	host_instrument->flags = n64_instrument->flags;
	host_instrument->tremType = n64_instrument->tremType;
	host_instrument->tremRate = n64_instrument->tremRate;
	host_instrument->tremDepth = n64_instrument->tremDepth;
	host_instrument->tremDelay = n64_instrument->tremDelay;
	host_instrument->vibType = n64_instrument->vibType;
	host_instrument->vibRate = n64_instrument->vibRate;
	host_instrument->vibDepth = n64_instrument->vibDepth;
	host_instrument->vibDelay = n64_instrument->vibDelay;
	host_instrument->bendRange = srctodst16(n64_instrument->bendRange);
	host_instrument->soundCount = htodst16(soundCount);

	dstpos = dstpos + sizeof(struct host_instrument) + sizeof(uintptr_t) * (soundCount - 1);

	for (int i = 0; i < soundCount; i++) {
		srcpos = srctoh32(n64_instrument->soundArray[i]);
		AL_NEXT_ITEM(host_instrument->soundArray[i], convert_audio_sound);
	}

	return dstpos;
}

static int convert_audio_bank(u8 *dst, int dstpos, u8 *src, int srcpos)
{
	struct n64_bank *n64_bank = (struct n64_bank *) &src[srcpos];
	struct host_bank *host_bank = (struct host_bank *) &dst[dstpos];
	int instCount = srctoh16(n64_bank->instCount);

	host_bank->instCount = htodst16(instCount);
	host_bank->flags = n64_bank->flags;
	host_bank->pad = n64_bank->pad;
	host_bank->sampleRate = srctodst32(n64_bank->sampleRate);

	dstpos = dstpos + sizeof(struct host_bank) + sizeof(uintptr_t) * (instCount - 1);

	if (n64_bank->percussion) {
		srcpos = srctoh32(n64_bank->percussion);
		AL_NEXT_ITEM(host_bank->percussion, convert_audio_instrument);
	} else {
		host_bank->percussion = 0;
	}

	for (int i = 0; i < instCount; i++) {
		srcpos = srctoh32(n64_bank->instArray[i]);
		AL_NEXT_ITEM(host_bank->instArray[i], convert_audio_instrument);
	}

	return dstpos;
}

static int convert_audio_bankfile(u8 *dst, u8 *src)
{
	struct n64_bankfile *n64_bankfile = (struct n64_bankfile *)src;
	struct host_bankfile *host_bankfile = (struct host_bankfile *)dst;
	int bankCount = srctoh16(n64_bankfile->bankCount);

	host_bankfile->revision = srctodst16(n64_bankfile->revision);
	host_bankfile->bankCount = htodst16(bankCount);


	u32 dstpos = sizeof(struct host_bankfile) + sizeof(uintptr_t) * (bankCount - 1);

	for (int i = 0; i < bankCount; i++) {
		host_bankfile->bankArray[i] = htodst32(dstpos);
		u32 srcpos = srctoh32(n64_bankfile->bankArray[i]);
		dstpos = convert_audio_bank(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

u8 *preprocessALBankFile_x64(u8 *src, u32 size, u32 *outSize)
{
	reset_markers();
	u32 dstlen = size * 3;
	u8 *dst = sysMemZeroAlloc(dstlen);

	*outSize = convert_audio_bankfile(dst, src);
	*outSize = ALIGN16(*outSize);

	return dst;
}
