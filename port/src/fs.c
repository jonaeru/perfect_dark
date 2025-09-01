#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <PR/ultratypes.h>
#include "constants.h"
#include "config.h"
#include "mod.h"
#include "system.h"
#include "platform.h"
#include "utils.h"
#include "fs.h"
#ifdef PLATFORM_WIN32
#include <direct.h>
#endif

#define DEFAULT_BASEDIR_NAME "data"

static char baseDir[FS_MAXPATH + 1]; // replaces $B
static char modDir[FS_MAXPATH + 1];  // replaces $M
static char saveDir[FS_MAXPATH + 1]; // replaces $S
static char homeDir[FS_MAXPATH + 1]; // replaces $H
static char exeDir[FS_MAXPATH + 1];  // replaces $E

static char gexModDir[FS_MAXPATH + 1];          // GoldenEye X Mod
static char kakarikoModDir[FS_MAXPATH + 1];     // Kakariko Village Mod
static char darknoonModDir[FS_MAXPATH + 1];     // Dark Moon Mod
static char goldfinger64ModDir[FS_MAXPATH + 1]; // Goldfinger 64 Mod
static char fojoModDir[FS_MAXPATH + 1];         // Friends of Joanna Mod
static char aioModDir[FS_MAXPATH + 1];          // All in One Mod

u32 g_ModNum = MOD_FOJO;

static s32 fsPathIsWritable(const char *path)
{
#ifdef PLATFORM_WIN32
	// on windows access() on directories will only check if the directory exists, so
	char tmp[FS_MAXPATH + 1] = { 0 };
	snprintf(tmp, sizeof(tmp), "%s/.tmp", path);
	FILE *f = fopen(tmp, "wb");
	if (f) {
		fclose(f);
		remove(tmp);
		return 1;
	}
	return 0;
#else
	return (access(path, W_OK) == 0);
#endif
}

s32 fsPathIsAbsolute(const char *path)
{
 return (path[0] == '/' || (isalpha(path[0]) && path[1] == ':'));
}

s32 fsPathIsCwdRelative(const char *path)
{
	// ., .., ./, ../
	return (path[0] == '.' && (path[1] == '.' || path[1] == '/' || path[1] == '\\' || path[1] == '\0'));
}
static inline const bool fsModFullPathCheck(const char *relPath, const char *modDir, char *pathBuf)
{
	if (modDir[0]) {
		if (!fsPathIsAbsolute(relPath)) {
			snprintf(pathBuf, FS_MAXPATH, "%s/%s", modDir, relPath);
		}
		if (fsFileSize(pathBuf) >= 0) {
			return true;
		}
	}

	return false;
}

static inline const bool fsModFullPath(char *pathBuf, const char *relPath)
{
	// if relPath doesn't contain textures/ or files/ subdir, don't even try to look in mod dirs
	// and return false
	if (!strstr(relPath, "textures") && !strstr(relPath, "files") && !strstr(relPath, "modconfig.txt") && !strstr(relPath, "sequences")) {
		printf("fsModFullPath ret false: relPath=%s\n", relPath);
		return false;
	}
	switch (g_ModNum) {
		case MOD_GEX:
			return fsModFullPathCheck(relPath, gexModDir, pathBuf);
			break;
		case MOD_KAKARIKO:
			return fsModFullPathCheck(relPath, kakarikoModDir, pathBuf);
			break;
		case MOD_DARKNOON:
			return fsModFullPathCheck(relPath, darknoonModDir, pathBuf);
			break;
		case MOD_GOLDFINGER_64:
			return fsModFullPathCheck(relPath, goldfinger64ModDir, pathBuf);
			break;
		case MOD_FOJO:
			return fsModFullPathCheck(relPath, fojoModDir, pathBuf);
			break;
		case MOD_NORMAL:
			return fsModFullPathCheck(relPath, aioModDir, pathBuf);
			break;
		default:
			return false;
			break;
	}
}

const char *fsFullPath(const char *relPath)
{
	static char pathBuf[FS_MAXPATH + 1];

	bool istexture = (strstr(relPath, "textures") != NULL);

	if (relPath[0] == '$') {
		// expandable placeholder $X; will be replaced with the corresponding path, if any
		const char *expStr = NULL;
		switch (relPath[1]) {
			case 'E': expStr = exeDir; break;
			case 'H': expStr = homeDir; break;
			case 'M': expStr = modDir; break;
			case 'B': expStr = baseDir; break;
			case 'S': expStr = saveDir; break;
			default: break;
		}
		if (expStr) {
			const u32 len = strlen(expStr);
			if (len > 0) {
				memcpy(pathBuf, expStr, len);
				strncpy(pathBuf + len, relPath + 2, FS_MAXPATH - len);
				if (istexture) {
					sysLogPrintf(LOG_NOTE, "fsFullPath: texture expanded path: %s", pathBuf);
				}
				return pathBuf;
			}
		}
		// couldn't expand anything, return as is
		if (istexture) {
			sysLogPrintf(LOG_WARNING, "fsFullPath: texture invalid path (not expanded): %s", relPath);
		}
		return relPath;
	} else if (!baseDir[0] || fsPathIsAbsolute(relPath) || fsPathIsCwdRelative(relPath)) {
		// user explicitly wants working directory or this is an absolute path or we have no baseDir set up yet
		if (istexture) {
			sysLogPrintf(LOG_NOTE, "fsFullPath: texture explicit path: %s", relPath);
		}
		return relPath;
	}

	// path relative to mod or base dir; this will be a read request, so check where the file actually is
	if (fsModFullPath(pathBuf, relPath)) {
		// found in mod dir
		if (istexture) {
			sysLogPrintf(LOG_NOTE, "fsFullPath: texture mod path: %s", pathBuf);
		}
		return pathBuf;
	}

	// fall back to basedir
	snprintf(pathBuf, FS_MAXPATH, "%s/%s", baseDir, relPath);
	return pathBuf;
}



static inline void modInit(char* path, char* outModDir, s32 portable){
	if (path) {
		if (fsPathIsAbsolute(path) || fsPathIsCwdRelative(path) || path[0] == '$') {
			// path is explicit; check as-is
			if (fsFileSize(path) >= 0) {
				strncpy(outModDir, fsFullPath(path), FS_MAXPATH);
			}
		} else {
			// path is relative to workdir; try to find it
			const char *priority[] = { "$M",".", "$E", "$H" };
			for (s32 i = 0; i < 2 + (portable != 0); ++i) {
				sysLogPrintf(LOG_NOTE,"looking for moddir `%s` in: %s\n", path, priority[i]);
				char *tmp = strFmt("%s/%s", priority[i], path);
				if (fsFileSize(tmp) >= 0) {
					strncpy(outModDir, fsFullPath(tmp), FS_MAXPATH);
					break;
				}
			}
		}
		if (!outModDir[0]) {
			sysLogPrintf(LOG_WARNING, "could not find specified moddir `%s`", path);
			sysLogPrintf(LOG_WARNING, "outModDir: %s\n", outModDir);
		}
	}
}

s32 fsInit(void)
{
	sysGetExecutablePath(exeDir, FS_MAXPATH);

	// if this is set, default to exe path for everything
	const s32 portable = sysArgCheck("--portable");
	if (portable) {
		strcpy(homeDir, exeDir);
	} else {
		sysGetHomePath(homeDir, FS_MAXPATH);
	}

	// get path to base dir and expand it if needed
	const char *path = sysArgGetString("--basedir");
	if (!path) {
		// check if there's a `data` directory in working directory or homeDir, otherwise default to exe directory
		path = "$E/" DEFAULT_BASEDIR_NAME;
		if (!portable) {
			if (fsFileSize("./" DEFAULT_BASEDIR_NAME) >= 0) {
				path = "./" DEFAULT_BASEDIR_NAME;
			} else if (fsFileSize("$H/" DEFAULT_BASEDIR_NAME)) {
				path = "$H/" DEFAULT_BASEDIR_NAME;
			}
		}
	}
	strncpy(baseDir, fsFullPath(path), FS_MAXPATH);

	// get path to mod dir and expand it if needed
	// mod directory is overlaid on top of base directory
	path = sysArgGetString("--moddir");
	modInit(path, modDir, portable);

	// All in One Mod Dir
	path = sysArgGetString("--aiomoddir");
	modInit(path, aioModDir, portable);

	// Friends of Joanna Mod Dir
	path = sysArgGetString("--fojomoddir");
	modInit(path, fojoModDir, portable);

	// GoldenEye X Mod Dir
	path = sysArgGetString("--gexmoddir");
	modInit(path, gexModDir, portable);

	// Kakariko Village Mod Dir
	path = sysArgGetString("--kakarikomoddir");
	modInit(path, kakarikoModDir, portable);

	// Dark Moon Mod Dir
	path = sysArgGetString("--darknoonmoddir");
	modInit(path, darknoonModDir, portable);

	// Goldfinger 64 Mod Dir
	path = sysArgGetString("--goldfinger64moddir");
	modInit(path, goldfinger64ModDir, portable);

	// get path to save dir and expand it if needed
	path = sysArgGetString("--savedir");
	if (!path) {
		if (portable) {
			path = "$E";
		} else {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_OSX)
			// check if there's a config in the working directory, otherwise default to homeDir
			if (fsFileSize("./" CONFIG_FNAME) >= 0) {
				path = ".";
			} else {
				path = "$H";
			}
#else
			// check if working directory is writable, otherwise default to homeDir
			if (fsPathIsWritable("./")) {
				path = ".";
			} else {
				sysLogPrintf(LOG_WARNING, "cannot write to working directory, will use %s for saves instead", homeDir);
				path = "$H";
			}
#endif
		}
	}

	strncpy(saveDir, fsFullPath(path), FS_MAXPATH);
	sysLogPrintf(LOG_NOTE, " mod dir: %s", modDir);
	sysLogPrintf(LOG_NOTE, " gex mod dir: %s", gexModDir);
	sysLogPrintf(LOG_NOTE, " kakariko mod dir: %s", kakarikoModDir);
	sysLogPrintf(LOG_NOTE, " darknoon mod dir: %s", darknoonModDir);
	sysLogPrintf(LOG_NOTE, " goldfinger64 mod dir: %s", goldfinger64ModDir);

	sysLogPrintf(LOG_NOTE, "base dir: %s", baseDir);
	sysLogPrintf(LOG_NOTE, "save dir: %s", saveDir);

	return 0;
}

const char *fsGetModDir(void)
{
	if (g_ModNum == MOD_GEX) {
		return gexModDir[0] ? gexModDir : NULL;
	} else if (g_ModNum == MOD_KAKARIKO) {
		return kakarikoModDir[0] ? kakarikoModDir : NULL;
	} else if (g_ModNum == MOD_DARKNOON) {
		return darknoonModDir[0] ? darknoonModDir : NULL;
	} else if (g_ModNum == MOD_GOLDFINGER_64) {
		return goldfinger64ModDir[0] ? goldfinger64ModDir : NULL;
	} else {
		return modDir[0] ? modDir : NULL;
	}
}

s32 fsFileLoadTo(const char *name, void *dst, u32 dstSize)
{
	if (strstr(name, "textures")) {
		sysLogPrintf(LOG_NOTE, "fsFileLoadTo: loading texture %s to %p (size %u)", name, dst, dstSize);
	}
	const char *fullName = fsFullPath(name);

	FILE *f = fopen(fullName, "rb");
	if (!f) {
		return -1;
	}

	fseek(f, 0, SEEK_END);
	const s32 size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size < 0) {
		sysLogPrintf(LOG_ERROR, "fsFileLoadTo: empty file or invalid size (%d): %s", size, fullName);
		fclose(f);
		return -1;
	}

	if ((u32)size > dstSize) {
		sysLogPrintf(LOG_ERROR, "fsFileLoadTo: file too big for buffer (%u > %u): %s", size, dstSize, fullName);
		fclose(f);
		return -1;
	}

	fread(dst, 1, size, f);
	fclose(f);

	return size;
}

void *fsFileLoad(const char *name, u32 *outSize)
{
	const char *fullName = fsFullPath(name);

	FILE *f = fopen(fullName, "rb");
	if (!f) {
		sysLogPrintf(LOG_ERROR, "fsFileLoad: could not find file: %s", fullName);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	const s32 size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size < 0) {
		sysLogPrintf(LOG_ERROR, "fsFileLoad: empty file or invalid size (%d): %s", size, fullName);
		fclose(f);
		return NULL;
	}

	void *buf = NULL;
	if (size) {
		buf = sysMemZeroAlloc(size + 1); // sick hack for a free null terminator
		if (!buf) {
			sysLogPrintf(LOG_ERROR, "fsFileLoad: could not alloc %d bytes for file: %s", size, fullName);
			fclose(f);
			return NULL;
		}
		fread(buf, 1, size, f);
	}

	fclose(f);

	if (outSize) {
		*outSize = size;
	}

	return buf;
}

s32 fsFileSize(const char *name)
{
	const char *fullName = fsFullPath(name);
	struct stat st;
	if (stat(fullName, &st) < 0) {
		return -1;
	} else {
		return st.st_size;
	}
}

FILE *fsFileOpenWrite(const char *name)
{
	return fopen(fsFullPath(name), "wb");
}

FILE *fsFileOpenRead(const char *name)
{
	return fopen(fsFullPath(name), "rb");
}

void fsFileFree(FILE *f)
{
	fclose(f);
}

s32 fsCreateDir(const char *path)
{
#ifdef PLATFORM_WIN32
	return _mkdir(fsFullPath(path));
#else
	return mkdir(fsFullPath(path), 0777);
#endif
}
