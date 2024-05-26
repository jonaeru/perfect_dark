#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define ROM_SIZE (1024 * 1024 * 32)

uint8_t *g_Rom = NULL;
enum romversion g_RomVersion = 0;
char g_OutPath[1024] = {'\0'};

void extract_anims(void);
void extract_audio(void);
void extract_banners(void);
void extract_files(void);
void extract_fonts(void);
void extract_jpnfonts(void);
void extract_textures(void);

/**
 * We allow modded ROMs to be used, so we can't simply compare the ROM checksum.
 */
static enum romversion identify_rom_version(uint8_t *rom)
{
	char game_code[5];
	game_code[0] = rom[0x3b];
	game_code[1] = rom[0x3c];
	game_code[2] = rom[0x3d];
	game_code[3] = rom[0x3e];
	game_code[4] = '\0';

	char version_byte = rom[0x3f];

	if (strncmp((char *) &rom[0x20], "Perfect Dark DBGNTSC", 20) == 0) {
		return ROMVERSION_NTSCBETA;
	}

	if (strcmp(game_code, "NPDJ") == 0) {
		return ROMVERSION_JPNFINAL;
	}

	if (strcmp(game_code, "NPDE") == 0) {
		if (version_byte == 1) {
			return ROMVERSION_NTSCFINAL;
		}

		if (version_byte == 0) {
			return ROMVERSION_NTSC10;
		}
	}

	if (strcmp(game_code, "NPDP") == 0) {
		if (rom[0x100f] == 0xe0) {
			return ROMVERSION_PALBETA;
		}

		if (rom[0x100f] == 0xf0) {
			return ROMVERSION_PALFINAL;
		}
	}

	return ROMVERSION_UNKNOWN;
}

static void flip_pairs(uint8_t *rom)
{
	uint16_t *ptr = (uint16_t *) rom;

	for (int i = 0; i < ROM_SIZE / 2; i++) {
		ptr[i] = __bswap_16(ptr[i]);
	}
}

static void flip_words(uint8_t *rom)
{
	uint32_t *ptr = (uint32_t *) rom;

	for (int i = 0; i < ROM_SIZE / 4; i++) {
		ptr[i] = __bswap_32(ptr[i]);
	}
}

static void extract_rom(char *romfile)
{
	FILE *fp = fopen(romfile, "rb");

	if (fp == NULL) {
		fprintf(stderr, "Unable to open '%s' for reading.\n", romfile);
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_END);

	if (ftell(fp) != ROM_SIZE) {
		fprintf(stderr, "The ROM size is not 32MB. Is this a Perfect Dark ROM?\n");
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_SET);

	g_Rom = malloc(ROM_SIZE);
	fread(g_Rom, 1, ROM_SIZE, fp);
	fclose(fp);

	uint32_t magic = (g_Rom[0] << 24) | (g_Rom[1] << 16) | (g_Rom[2] << 8) | g_Rom[3];

	switch (magic) {
	case 0x80371240:
		break;
	case 0x37804012:
		flip_pairs(g_Rom);
		break;
	case 0x40123780:
		flip_words(g_Rom);
		break;
	default:
		fprintf(stderr, "Unable to detect ROM header. Is this an N64 ROM?\n");
		exit(EXIT_FAILURE);
		break;
	}

	g_RomVersion = identify_rom_version(g_Rom);

	if (g_RomVersion == ROMVERSION_NTSCBETA || g_RomVersion == ROMVERSION_PALBETA) {
		fprintf(stderr, "The beta versions are not supported.\n");
		exit(EXIT_FAILURE);
	}

	if (g_RomVersion == ROMVERSION_UNKNOWN) {
		fprintf(stderr, "Unable to determine your Perfect Dark version. Is this a Perfect Dark ROM?\n");
		exit(EXIT_FAILURE);
	}

	extract_anims();
	extract_audio();
	extract_files();
	extract_fonts();
	extract_jpnfonts();
	extract_textures();
	extract_banners();

	free(g_Rom);
	g_Rom = NULL;
}

int main(int argc, char **argv)
{
	char *romfile = NULL;

	if (argc < 2) {
		fprintf(stderr, "Usage: pd-extract <ROMFILE> [PATH]\n");
		fprintf(stderr, "Read ROMFILE, writing assets to PATH.\n");
		fprintf(stderr, "With no PATH, assume \"extracted/\".\n");
		return EXIT_FAILURE;
	}

	romfile = argv[1];

	if (argc > 2) {
		strcpy(g_OutPath, argv[2]);
	} else {
		strcpy(g_OutPath, "extracted");
	}

	if (g_OutPath[strlen(g_OutPath) - 1] == '/') {
		g_OutPath[strlen(g_OutPath) - 1] = '\0';
	}

	extract_rom(romfile);

	return EXIT_SUCCESS;
}
