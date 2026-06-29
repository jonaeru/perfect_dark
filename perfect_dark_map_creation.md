# Perfect Dark Map Creation & Design Expert Guide

This document provides a comprehensive, expert-level technical breakdown of how Perfect Dark processes multiplayer map geometry, rooms, and properties. It details the exact memory layouts and pipeline requirements to build custom maps for the Perfect Dark PC port and original N64 hardware.

## 1. The Map File Architecture (`.seg` format)

Perfect Dark background files (e.g., `bg_mp14.seg`) are packaged in a highly structured, multi-section binary format. The engine loads this file sequentially in distinct phases. The file is composed of three main compressed sections, followed by the compressed room payloads.

### Overall Structure
```
[ 0x00 ] Section 1 Header (Metadata)
[ 0x0C ] Section 1 Data (Compressed 'Primary Data')
[ ...  ] Section 2 Header (Metadata)
[ ...  ] Section 2 Data (Compressed Texture IDs)
[ ...  ] Section 3 Header (Metadata)
[ ...  ] Section 3 Data (Compressed Bounding Boxes)
[ ...  ] Compressed Room Data Blocks (Multiple blocks)
```

### Compression (Rarezip)
Most sections and all room geometries are compressed using **Rarezip** (a proprietary DEFLATE variant). These blocks are identified by a 16-bit magic header: `0x1173`. When the engine encounters this magic number, it routes the buffer to `bgInflate()` to expand it into memory.

## 2. Breakdown of the Sections

### Section 1: Primary Data
This section contains the fundamental data structures that define the connectivity and layout of the map.
The header (12 bytes) dictates the layout:
* `u32 inflatedsize`: Size of the decompressed payload.
* `u32 section1compsize`: Compressed size of this section.
* `u32 primcompsize`: Compressed size of the primary data within this section.

When decompressed, this section contains the 5 core pointers:
1. **Null Offset**
2. **`g_BgRooms` Array**: Array of `struct bgroom` which dictates the physical room definitions and the byte offsets to their geometry payloads.
3. **`g_BgPortals` Array**: Defines connections between rooms (doorways, windows). Used extensively for visibility culling.
4. **`g_BgCommands` Array**: Environment scripting (fog, lighting changes, music).
5. **`g_BgLightsFileData`**: Baked room light definitions.

#### The `bgroom` Struct
```c
struct bgroom {
    uint32_t payload_offset; // Absolute byte offset in the .seg file to the compressed room geometry
    struct coord pos;        // Float X, Y, Z vector for the room's absolute center point
    uint8_t min_light;       // Minimum ambient light brightness
    uint8_t max_light;       // Maximum ambient light brightness
    // ... padded to 20 bytes ...
};
```

### Section 2: Texture ID List
This section purely contains a list of 16-bit Texture IDs used by the map. During map load, the engine iterates over this array and calls `texLoadFromTextureNum()` to aggressively cache and bind the textures into RAM before the game starts.

### Section 3: Bounding Boxes
Contains a flat array of 16-bit integer boundary coordinates (X_min, Y_min, Z_min, X_max, Y_max, Z_max) relative to each room's center position. This is used by the collision and rendering engines for quick AABB (Axis-Aligned Bounding Box) testing.

---

## 3. Room Geometry (`roomgfxdata`)

When the engine determines a room is visible, it dynamically loads and inflates the room's geometry payload from the byte offset defined in `g_BgRooms`. The decompressed data maps perfectly to `struct roomgfxdata`.

```c
struct roomgfxdata {
    Vtx *vertices;               // Pointer to F3DEX2 vertex array
    Col *colours;                // Pointer to RGBA light/vertex colors
    struct roomblock *opablocks; // Root tree node for opaque geometry
    struct roomblock *xlublocks; // Root tree node for translucent (alpha) geometry
    // ...
    struct roomblock blocks[1];  // Starting address for the block tree
};
```

### The `roomblock` Tree
Perfect Dark structures its display lists in a BSP-like tree of `roomblock` nodes to optimize rendering.
* **`ROOMBLOCKTYPE_PARENT`**: Contains pointers to child nodes, allowing hierarchical traversal and culling.
* **`ROOMBLOCKTYPE_LEAF`**: Contains a direct `Gfx *gdl` pointer to an **F3DEX2 Display List**. This is the raw microcode that is sent to the RSP to draw the physical triangles on screen.

## 4. Multiplayer Setups and Object Spawning (The `mp_setup` files)

Multiplayer maps also rely on a companion `setup` file (e.g., `mp_setupmp14.c` or compiled to `Usetupmp14Z`). This file contains the placement arrays for weapons, props, paths, and spawn points.

### The Conversion Pipeline (`convertProps`)
When the engine loads a setup file, it processes the arrays via `convertProps()` in `filesetup.c`. 
A critical requirement for custom mods: **The `props[]` array MUST be terminated by a `0x00000000` (`OBJTYPE_END`) word.**

If custom setup generation scripts accidentally terminate the array with invalid padding (e.g., `0x34`), `convertProps` will iterate out of bounds and interpret garbage memory as `struct defaultobj`, causing an immediate **SIGSEGV crash** during level load.

### Simulant Spawning
Simulants rely on the availability of valid spawn pads and the successful parsing of the setup file. If the setup file crashes during `convertProps`, the map fails to initialize properly, and simulants will not spawn (and the match will not start). 

Simulant slots are tracked via `g_MpSetup.chrslots`. This is a 32-bit mask:
* Bits 0-3: Local human players.
* Bits 4-27: Simulant slots (expanded in the AIO mod from 8 to 24 via `MAX_BOTS`).
Function `mpIsChrParticipating(index)` leverages this mask to cleanly track which slots are active without overflowing.

---

## 5. Pipeline for Building Custom Maps

To build a custom map that the engine will accept natively, you must execute the following pipeline:

1. **Geometry Generation**: Generate 64-bit F3DEX2 Display Lists (drawing triangles, binding texture addresses).
2. **Room Packaging**: Wrap the display lists into the `roomblock` tree and prepend the `roomgfxdata` header.
3. **Compression**: Compress each room payload using the Rarezip algorithm.
4. **Section Assembly**: 
   * Generate Section 3 (AABBs).
   * Generate Section 2 (Texture IDs).
   * Assemble Section 1 (Header, 5 pointers, `g_BgRooms` offsets, Portals).
5. **Final `.seg` Packaging**: Concatenate all compressed sections into a single file and place it in the `data/` or `mods/` directory.
6. **Setup Integration**: Create a paired `mp_setup` binary with valid prop arrays (correctly `0x00` terminated) to spawn players and items.
