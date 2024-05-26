#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

FILE *openfile(char *filename)
{
	char path[1024];
	struct stat status;

	memcpy(path, filename, strlen(filename));
	dirname(path);

	while (stat(path, &status) == -1) {
//		while (mkdir(path, 0755) == -1) {
		while (mkdir(path) == -1) {
			dirname(path);
		}

		memcpy(path, filename, strlen(filename));
		dirname(path);
	}

	FILE *fp = fopen(filename, "wb");

	if (fp == NULL) {
		fprintf(stderr, "Unable to open '%s' for writing\n", filename);
		exit(EXIT_FAILURE);
	}

	return fp;
}

size_t rzip_get_infsize(uint8_t *src)
{
	if (src[0] != 0x11 || src[1] != 0x73) {
		fprintf(stderr, "rzip_get_infsize: 1173 not found\n");
		exit(EXIT_FAILURE);
	}

	return src[2] << 16 | (src[3] << 8) | src[4];
}

int rzip_inflate(uint8_t *dst, size_t dstlen, uint8_t *src, size_t srclen)
{
	if (src[0] != 0x11 || src[1] != 0x73) {
		fprintf(stderr, "rzip_inflate: 1173 not found\n");

		for (int i = 0; i < 16; i++) {
			fprintf(stderr, "%02x ", src[i]);
		}

		fprintf(stderr, "\n");

		exit(EXIT_FAILURE);
	}

	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = srclen - 5;
	stream.next_in = &src[5];
	stream.avail_out = dstlen;
	stream.next_out = dst;

	inflateInit2(&stream, -15);
	int ret = inflate(&stream, Z_FINISH);
	inflateEnd(&stream);

	return ret;
}

int rzip_deflate(uint8_t *dst, size_t *dstlen, uint8_t *src, size_t srclen)
{
	dst[0] = 0x11;
	dst[1] = 0x73;
	dst[2] = (srclen >> 16) & 0xff;
	dst[3] = (srclen >> 8) & 0xff;
	dst[4] = srclen & 0xff;

	*dstlen = *dstlen - 5;

	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = srclen;
	stream.next_in = src;
	stream.avail_out = *dstlen;
	stream.next_out = &dst[5];

	deflateInit2(&stream, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
	int ret = deflate(&stream, Z_FINISH);
	deflateEnd(&stream);

	*dstlen = stream.next_out - dst;

	return ret;
}
