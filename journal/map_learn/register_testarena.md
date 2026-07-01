# Stage registration plan: testarena

Already registered: **False**

## 1. `src/include/files.h`

```c
#define FILE_BG_TESTARENA_SEG       0x07e0
#define FILE_BG_TESTARENA_TILES     0x07e1
#define FILE_BG_TESTARENA_PADS      0x07e2
#define FILE_USETUPTESTARENA         0x07e3
#define FILE_UMP_SETUPTESTARENA      0x07e4
```

## 2. `src/assets/ntsc-final/files/list.c`

```c
/*0x07e0*/ "bgdata/bg_testarena.seg",
/*0x07e1*/ "bgdata/bg_testarena_tilesZ",
/*0x07e2*/ "bgdata/bg_testarena_padsZ",
/*0x07e3*/ "bgdata/UsetuptestarenaZ",
/*0x07e4*/ "bgdata/Ump_setuptestarenaZ",
```

## 3. `src/game/stagetable.c`

```c
/*0x07e0*/ STAGE_TESTARENA, 2, 255, 100, 100, 0, FILE_BG_TESTARENA_SEG, FILE_BG_TESTARENA_TILES, FILE_BG_TESTARENA_PADS, FILE_USETUPTESTARENA, FILE_UMP_SETUPTESTARENA, 1, 1, 100, 0, 0, -1, 255, 0x3e19999a, -1, 400, 0, 1, SFX_ALARM_DEFAULT, 0,
```

## 4. `src/game/mplayer/setup.c` (`g_MpArenas[]`)

```c
{ STAGE_TESTARENA, 0, 0x7FFF }, // testarena custom arena
```

> After editing, run `make -j8` and `pdmap build {name} --deploy`.
> For quick test without menu registration, use `--deploy-as uff` instead.
