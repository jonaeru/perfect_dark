#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

enum {
	AL_ADPCM_WAVE = 0,
	AL_RAW16_WAVE
};

struct generic_adpcm_book {
	int32_t order;
	int32_t npredictors;
	int16_t book[128];
};

struct generic_adpcm_loop {
	uint32_t start;
	uint32_t end;
	uint32_t count;
	int16_t state[16];
};

struct generic_raw_loop {
	uint32_t start;
	uint32_t end;
	uint32_t count;
};

struct generic_envelope {
	int32_t attackTime;
	int32_t decayTime;
	int32_t releaseTime;
	uint8_t attackVolume;
	uint8_t decayVolume;
};

struct generic_keymap {
	uint8_t velocityMin;
	uint8_t velocityMax;
	uint8_t keyMin;
	uint8_t keyMax;
	uint8_t keyBase;
	int8_t detune;
};

struct n64_adpcm_waveinfo {
	uint32_t loop; // ptr to struct generic_adpcm_loop
	uint32_t book; // ptr to struct generic_adpcm_book
};

struct host_adpcm_waveinfo {
	 uintptr_t loop; // ptr to struct generic_adpcm_loop
	 uintptr_t book; // ptr to struct generic_adpcm_book
};

struct n64_raw_waveinfo {
	uint32_t loop; // ptr to struct generic_raw_loop
};

struct host_raw_waveinfo {
	uintptr_t loop; // ptr to struct generic_raw_loop
};

struct n64_wavetable {
	uint32_t base;
	int32_t len;
	uint8_t type;
	uint8_t flags;
	union {
		struct n64_adpcm_waveinfo adpcmWave;
		struct n64_raw_waveinfo rawWave;
	} waveInfo;
};

struct host_wavetable {
	uintptr_t base;
	int32_t len;
	uint8_t type;
	uint8_t flags;
	union {
		struct host_adpcm_waveinfo adpcmWave;
		struct host_raw_waveinfo rawWave;
	} waveInfo;
};

struct n64_sound {
	uint32_t envelope; // ptr to struct generic_envelope
	uint32_t keyMap; // ptr to struct generic_keymap
	uint32_t wavetable; // ptr to struct n64_wavetable
	uint8_t samplePan;
	uint8_t sampleVolume;
	uint8_t flags;
};

struct host_sound {
	uintptr_t envelope; // ptr to struct generic_envelope
	uintptr_t keyMap; // ptr to struct generic_keymap
	uintptr_t wavetable; // ptr to struct host_wavetable
	uint8_t samplePan;
	uint8_t sampleVolume;
	uint8_t flags;
};

struct n64_instrument {
	uint8_t volume;
	uint8_t pan;
	uint8_t priority;
	uint8_t flags;
	uint8_t tremType;
	uint8_t tremRate;
	uint8_t tremDepth;
	uint8_t tremDelay;
	uint8_t vibType;
	uint8_t vibRate;
	uint8_t vibDepth;
	uint8_t vibDelay;
	int16_t bendRange;
	int16_t soundCount;
	uint32_t soundArray[1]; // ptr to struct n64_sound
};

struct host_instrument {
	uint8_t volume;
	uint8_t pan;
	uint8_t priority;
	uint8_t flags;
	uint8_t tremType;
	uint8_t tremRate;
	uint8_t tremDepth;
	uint8_t tremDelay;
	uint8_t vibType;
	uint8_t vibRate;
	uint8_t vibDepth;
	uint8_t vibDelay;
	int16_t bendRange;
	int16_t soundCount;
	uintptr_t soundArray[1]; // ptr to struct host_sound
};

struct n64_bank {
	int16_t instCount;
	uint8_t flags;
	uint8_t pad;
	int32_t sampleRate;
	uint32_t percussion; // ptr to struct n64_instrument
	uint32_t instArray[1]; // ptr to struct n64_instrument
};

struct host_bank {
	int16_t instCount;
	uint8_t flags;
	uint8_t pad;
	int32_t sampleRate;
	uintptr_t percussion; // ptr to struct host_instrument
	uintptr_t instArray[1]; // ptr to struct host_instrument
};

struct n64_bankfile {
	int16_t revision;
	int16_t bankCount;
	uint32_t bankArray[1]; // ptr to struct n64_bank
};

struct host_bankfile {
	int16_t revision;
	int16_t bankCount;
	uintptr_t bankArray[1]; // ptr to struct host_bank
};

static int convert_audio_envelope(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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

static int convert_audio_keymap(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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

static int convert_audio_adpcm_loop(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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

static int convert_audio_adpcm_book(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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

static int convert_audio_raw_loop(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
{
	struct generic_raw_loop *n64_loop = (struct generic_raw_loop *) &src[srcpos];
	struct generic_raw_loop *host_loop = (struct generic_raw_loop *) &dst[dstpos];

	dstpos += sizeof(struct generic_raw_loop);

	host_loop->start = srctodst32(n64_loop->start);
	host_loop->end = srctodst32(n64_loop->end);
	host_loop->count = srctodst32(n64_loop->count);

	return dstpos;
}

static int convert_audio_wavetable(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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
			host_wavetable->waveInfo.adpcmWave.loop = htodst32(dstpos);
			srcpos = srctoh32(n64_wavetable->waveInfo.adpcmWave.loop);
			dstpos = convert_audio_adpcm_loop(dst, dstpos, src, srcpos);
		} else {
			host_wavetable->waveInfo.adpcmWave.loop = 0;
		}

		if (n64_wavetable->waveInfo.adpcmWave.book) {
			host_wavetable->waveInfo.adpcmWave.book = htodst32(dstpos);
			srcpos = srctoh32(n64_wavetable->waveInfo.adpcmWave.book);
			dstpos = convert_audio_adpcm_book(dst, dstpos, src, srcpos);
		} else {
			host_wavetable->waveInfo.adpcmWave.book = 0;
		}
	} else if (host_wavetable->type == AL_RAW16_WAVE) {
		if (n64_wavetable->waveInfo.rawWave.loop) {
			host_wavetable->waveInfo.rawWave.loop = htodst32(dstpos);
			srcpos = srctoh32(n64_wavetable->waveInfo.rawWave.loop);
			dstpos = convert_audio_raw_loop(dst, dstpos, src, srcpos);
		} else {
			host_wavetable->waveInfo.rawWave.loop = 0;
		}
	}

	return dstpos;
}

static int convert_audio_sound(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
{
	struct n64_sound *n64_sound = (struct n64_sound *) &src[srcpos];
	struct host_sound *host_sound = (struct host_sound *) &dst[dstpos];

	dstpos += sizeof(struct host_sound);

	if (n64_sound->envelope) {
		host_sound->envelope = htodst32(dstpos);
		srcpos = srctoh32(n64_sound->envelope);
		dstpos = convert_audio_envelope(dst, dstpos, src, srcpos);
	} else {
		host_sound->envelope = 0;
	}

	if (n64_sound->keyMap) {
		host_sound->keyMap = htodst32(dstpos);
		srcpos = srctoh32(n64_sound->keyMap);
		dstpos = convert_audio_keymap(dst, dstpos, src, srcpos);
	} else {
		host_sound->keyMap = 0;
	}

	if (n64_sound->wavetable) {
		host_sound->wavetable = htodst32(dstpos);
		srcpos = srctoh32(n64_sound->wavetable);
		dstpos = convert_audio_wavetable(dst, dstpos, src, srcpos);
	} else {
		host_sound->wavetable = 0;
	}

	host_sound->samplePan = n64_sound->samplePan;
	host_sound->sampleVolume = n64_sound->sampleVolume;
	host_sound->flags = n64_sound->flags;

	return dstpos;
}

static int convert_audio_instrument(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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
		host_instrument->soundArray[i] = htodst32(dstpos);
		srcpos = srctoh32(n64_instrument->soundArray[i]);
		dstpos = convert_audio_sound(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

static int convert_audio_bank(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
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
		host_bank->percussion = htodst32(dstpos);
		srcpos = srctoh32(n64_bank->percussion);
		dstpos = convert_audio_instrument(dst, dstpos, src, srcpos);
	} else {
		host_bank->percussion = 0;
	}

	for (int i = 0; i < instCount; i++) {
		host_bank->instArray[i] = htodst32(dstpos);
		srcpos = srctoh32(n64_bank->instArray[i]);
		dstpos = convert_audio_instrument(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

static int convert_audio_bankfile(uint8_t *dst, int dstpos, uint8_t *src, int srcpos)
{
	struct n64_bankfile *n64_bankfile = (struct n64_bankfile *) &src[srcpos];
	struct host_bankfile *host_bankfile = (struct host_bankfile *) &dst[dstpos];
	int bankCount = srctoh16(n64_bankfile->bankCount);

	host_bankfile->revision = srctodst16(n64_bankfile->revision);
	host_bankfile->bankCount = htodst16(bankCount);

	dstpos = dstpos + sizeof(struct host_bankfile) + sizeof(uintptr_t) * (bankCount - 1);

	for (int i = 0; i < bankCount; i++) {
		host_bankfile->bankArray[i] = htodst32(dstpos);
		srcpos = srctoh32(n64_bankfile->bankArray[i]);
		dstpos = convert_audio_bank(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

static void extract_audio_ctl(const char *filename, uint32_t romoffset, size_t srclen)
{
	size_t dstlen = srclen * 4;
	uint8_t *src = &g_Rom[romoffset];
	uint8_t *dst = calloc(1, dstlen);
	int dstpos = 0;

	dstpos = convert_audio_bankfile(dst, dstpos, src, 0);

	char outfilename[1024];
	sprintf(outfilename, "%s/segs/%sctl", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(dst, ALIGN16(dstpos), 1, fp);
	fclose(fp);

	free(dst);
}

static void extract_audio_tbl(const char *filename, uint32_t romoffset, size_t len)
{
	char outfilename[1024];
	sprintf(outfilename, "%s/segs/%stbl", g_OutPath, filename);

	FILE *fp = openfile(outfilename);
	fwrite(&g_Rom[romoffset], len, 1, fp);
	fclose(fp);
}

void extract_audio(void)
{
	uint32_t sfx_ctl = 0;
	uint32_t sfx_tbl = 0;
	uint32_t sfx_end = 0;

	uint32_t seq_ctl = 0;
	uint32_t seq_tbl = 0;
	uint32_t seq_end = 0;

	switch (g_RomVersion) {
	case ROMVERSION_NTSC10:
	case ROMVERSION_NTSCFINAL:
		sfx_ctl = 0x80a250;
		sfx_tbl = 0x839dd0;
		sfx_end = 0xcfbf30;
		seq_ctl = 0xcfbf30;
		seq_tbl = 0xd05f90;
		seq_end = 0xe82000;
		break;
	case ROMVERSION_PALFINAL:
		sfx_ctl = 0x7f87e0;
		sfx_tbl = 0x828360;
		sfx_end = 0xcea4c0;
		seq_ctl = 0xcea4c0;
		seq_tbl = 0xcf4520;
		seq_end = 0xe70590;
		break;
	case ROMVERSION_JPNFINAL:
		sfx_ctl = 0x7fc670;
		sfx_tbl = 0x82c1f0;
		sfx_end = 0xcee350;
		seq_ctl = 0xcee350;
		seq_tbl = 0xcf83b0;
		seq_end = 0xe74420;
		break;
	}

	extract_audio_ctl("sfx", sfx_ctl, sfx_tbl - sfx_ctl);
	extract_audio_tbl("sfx", sfx_tbl, sfx_end - sfx_tbl);
	extract_audio_ctl("seq", seq_ctl, seq_tbl - seq_ctl);
	extract_audio_tbl("seq", seq_tbl, seq_end - seq_tbl);
}
