# Perfect Dark — jonaeru's All-in-One Mod PC Port

This directory contains the source code for the **Perfect Dark PC Port (All-in-One Mod)** fork developed by **jonaeru**.

* **Repository Fork:** [jonaeru/perfect_dark](https://github.com/jonaeru/perfect_dark)
* **Active Branch:** `port-mods/all-in-one`

---

## What is this?

This is a specialized fork of the Perfect Dark PC port project that implements the **All-in-One Mod (AIO)**. The AIO mod bundles multiple community expansions, including:
* **GoldenEye X (GEX):** Over 70 multiplayer arenas from GoldenEye 007 / GoldenEye X.
* **Perfect Dark Plus:** Custom maps, enhancements, and features.
* **All Solos in Multi:** Allows playing solo campaign maps inside the multiplayer Combat Simulator.
* **Additional Custom Arenas:** Kakariko Village, Dark Noon, and others.

---

## How to Set Up and Play (macOS)

### 1. Place the Base ROM
Place your legal US NTSC v1.1 Perfect Dark ROM in the `data/` folder and name it:
```
data/pd.ntsc-final.z64
```

### 2. Download the Mod Assets
The repository contains the engine changes but **does not check in the large mod assets** (textures, audio, maps).
1. Download the `all_in_one_mods` ZIP file from the official Google Drive:
   * **[Google Drive Folder Link](https://drive.google.com/drive/folders/1eo_SBUDaGa4LNByWj8fbiWrr8ny471Hf)** (maintained by jonaeru)
   * **[Atari-Dude's Updated Pack](https://drive.google.com/file/d/1wNugFKl-ITXsz7wyjVzbVJgQwDub1Nb_/view?usp=sharing)** (highly recommended, frequently updated)
2. Extract the ZIP file and copy the **`mods`** directory directly into the root of this folder:
   ```
   perfect_dark_jonaeru_aio/
   ├── data/
   ├── mods/
   │   ├── mod_allinone/
   │   ├── mod_dark_noon/
   │   ├── mod_gex/
   │   └── mod_kakariko/
   ...
   ```

### 3. Build the Port
From the root of this directory:
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

### 4. Run with Mod Arguments
On macOS, launch the compiled binary from the root directory by specifying the mod folders using the engine's custom parameters:
```bash
./build/pd --moddir mods/mod_allinone --gexmoddir mods/mod_gex --kakarikomoddir mods/mod_kakariko --darknoonmoddir mods/mod_dark_noon
```

---

## Key Differences from the Upstream Port
* Supports loading multiple mod subfolders simultaneously via command-line arguments (e.g. `--gexmoddir`, `--kakarikomoddir`, `--darknoonmoddir`).
* Expanded asset limits and tables to prevent crashes when loading massive custom arenas.
