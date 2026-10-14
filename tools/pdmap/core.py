import json
import struct
import os
import importlib.util
import subprocess
import sys


ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ROMID = "ntsc-final"

BUILD_DIR = os.path.join(ROOT, "build", ROMID, "assets", "files", "bgdata")
SETUP_DIR = os.path.join(ROOT, "build", ROMID)
SEG_SRC = os.path.join(ROOT, "build", ROMID, "assets", "files", "seg")

MOD_DIRS = [
    os.path.join(ROOT, "mods", "mod_allinone", "files", "bgdata"),
]

ASSETMGR_DIR = os.path.join(ROOT, "tools", "assetmgr")
MKPADS_TOOL = os.path.join(ASSETMGR_DIR, "mkpads")
MKTILES_TOOL = os.path.join(ASSETMGR_DIR, "mktiles")


def to_scaled_hex(value: float) -> int:
    return int(round(value * 6))


def from_scaled_hex(value: int) -> float:
    return value / 6.0


def mkword(a: int, b: int) -> int:
    return ((a << 16) | (b & 0xffff))


class Pad:
    def __init__(self, index: int, x: float, y: float, z: float,
                 dir_x: float = 0.0, dir_y: float = 1.0, dir_z: float = 0.0,
                 up_x: float = 0.0, up_y: float = 0.0, up_z: float = -1.0,
                 room: int = 0, liftnum: int = 0,
                 xmin: float = -100.0, xmax: float = 100.0,
                 ymin: float = -100.0, ymax: float = 100.0,
                 zmin: float = -100.0, zmax: float = 100.0,
                 aiwaitlift: bool = False, aionlift: bool = False,
                 aiwalkdirect: bool = False, aidrop: bool = False,
                 aicrouch: bool = False, aiignorey: bool = False,
                 aiduck: bool = False):
        self.index = index
        self.x = x
        self.y = y
        self.z = z
        self.dir_x = dir_x
        self.dir_y = dir_y
        self.dir_z = dir_z
        self.up_x = up_x
        self.up_y = up_y
        self.up_z = up_z
        self.room = room
        self.liftnum = liftnum
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        self.zmin = zmin
        self.zmax = zmax
        self.aiwaitlift = aiwaitlift
        self.aionlift = aionlift
        self.aiwalkdirect = aiwalkdirect
        self.aidrop = aidrop
        self.aicrouch = aicrouch
        self.aiignorey = aiignorey
        self.aiduck = aiduck

    def __repr__(self):
        return f"Pad({self.index}: ({self.x}, {self.y}, {self.z}) room={self.room})"


class Cover:
    def __init__(self, index: int, x: float, z: float, y: float = 10.0,
                 dir_x: float = 0, dir_y: float = 0, dir_z: float = -1,
                 special: int = 0, unk1a: int = 14269):
        self.index = index
        self.x = x
        self.y = y
        self.z = z
        self.dir_x = dir_x
        self.dir_y = dir_y
        self.dir_z = dir_z
        self.special = special
        self.unk1a = unk1a

    def __repr__(self):
        return f"Cover({self.index}: ({self.x}, {self.y}, {self.z}))"


class MapDef:
    def __init__(self, name: str):
        self.name = name
        self.pads: list[Pad] = []
        self.covers: list[Cover] = []
        self.props: list = []
        self.intro: list = []

    def add_pad(self, index: int, x: float, y: float, z: float, **kwargs):
        kwargs.setdefault("room", 1)
        p = Pad(index, x, y, z, **kwargs)
        self.pads.append(p)
        return p

    def add_cover(self, x: float, z: float, y: float = 10.0, **kwargs):
        c = Cover(len(self.covers), x, z, y, **kwargs)
        self.covers.append(c)
        return c

    def add_prop(self, prop):
        self.props.append(prop)

    def add_intro(self, cmd):
        self.intro.append(cmd)

    def pack_pads_json(self) -> dict:
        name_upper = self.name.upper()
        json_pads = []
        for p in self.pads:
            json_pads.append({
                "id": f"PAD_{name_upper}_{p.index:04X}",
                "pos": [p.x, p.y, p.z],
                "dir": [p.dir_x, p.dir_y, p.dir_z],
                "up": [p.up_x, p.up_y, p.up_z],
                "xmin": p.xmin, "xmax": p.xmax,
                "ymin": p.ymin, "ymax": p.ymax,
                "zmin": p.zmin, "zmax": p.zmax,
                "aiwaitlift": p.aiwaitlift, "aionlift": p.aionlift,
                "aiwalkdirect": p.aiwalkdirect, "aidrop": p.aidrop,
                "aicrouch": p.aicrouch, "aiignorey": p.aiignorey,
                "aiduck": p.aiduck,
                "liftnum": p.liftnum, "room": p.room,
            })

        # --- Waypoint navigation graph ---------------------------------------
        # Build a SYMMETRIC (bidirectional) adjacency graph. Perfect Dark's
        # waypoint route-finder (waypointFindRoute / waypointDiscoverSteps in
        # padhalllv.c) discovers step numbers outward from the source via each
        # node's neighbour list, then reconstructs the path by walking neighbour
        # links back from the destination. That back-walk assumes edges are
        # navigable both ways: if A lists B as a neighbour but B does not list A,
        # the reconstruction can dereference a NULL neighbour and crash
        # (EXC_BAD_ACCESS in waypointFindRoute). A naive k-nearest graph is NOT
        # symmetric, so we explicitly add the reciprocal edge for every link.
        K_NEAREST = 6
        adjacency = {p.index: set() for p in self.pads}
        for p1 in self.pads:
            distances = []
            for p2 in self.pads:
                if p1 != p2:
                    dist = ((p2.x - p1.x) ** 2 + (p2.z - p1.z) ** 2) ** 0.5
                    distances.append((dist, p2.index))
            distances.sort()
            for _dist, j in distances[:K_NEAREST]:
                # Add the edge in both directions to keep the graph symmetric.
                adjacency[p1.index].add(j)
                adjacency[j].add(p1.index)

        waypoints = []
        room_waygroups = {}
        for p1 in self.pads:
            neighbours = []
            # Deterministic order (by index) so the binary is reproducible.
            for j in sorted(adjacency[p1.index]):
                neighbours.append({
                    "waypoint": f"WAYPOINT_{name_upper}_{j:04X}",
                    "flag4000": False, "flag8000": False,
                })
            waygroup_id = f"WAYGROUP_{name_upper}_{p1.room:04X}"
            room_waygroups[waygroup_id] = True
            waypoints.append({
                "id": f"WAYPOINT_{name_upper}_{p1.index:04X}",
                "pad": f"PAD_{name_upper}_{p1.index:04X}",
                "neighbours": neighbours,
                "waygroup": waygroup_id,
            })

        waygroups = []
        for wg_id in room_waygroups:
            neighbours = []
            for other_id in room_waygroups:
                if wg_id != other_id:
                    neighbours.append({
                        "waygroup": other_id,
                        "flag4000": False, "flag8000": False,
                    })
            waygroups.append({"id": wg_id, "neighbours": neighbours})

        cover_json = []
        for c in self.covers:
            cover_json.append({
                "id": f"COVER_{name_upper}_{c.index:04X}",
                "pos": [c.x, c.y, c.z],
                "dir": [c.dir_x, c.dir_y, c.dir_z],
                "special": c.special, "unk1a": c.unk1a,
            })

        return {"pads": json_pads, "waypoints": waypoints,
                "waygroups": waygroups, "cover": cover_json}

    def pack_setup(self) -> bytes:
        props_bin = b"".join(p.pack() for p in self.props) + struct.pack(">I", 0x34)
        intro_bin = b"".join(cmd.pack() for cmd in self.intro) + struct.pack(">I", 12)

        # --- Multiplayer simulant-init ailist (id 0x1000) ---------------------
        # In Perfect Dark, the engine auto-creates a hidden "background" chr
        # (chrnum = id - 0x60 = 0x0FA0 = 4000) for every setup ailist whose id is
        # >= 0x1000 (see game_00b820.c) and runs that ailist at match start.
        #
        # Combat Simulator stages rely on this to spawn their simulants:
        # botmgrAllocateBot() only *allocates* the bot chrs (at the origin, with
        # no room); they are not placed into the arena until mp_init_simulants
        # (AI command 0x0185 -> botSpawnAll) runs, which chooses a spawn pad and
        # grounds each bot. If the setup has no such ailist, the bots stay at
        # (0,0,0) with rooms=[-1] and immediately fall out of the world, so no
        # simulants are ever visible. We therefore always emit this ailist.
        #
        # AI bytecode (2-byte big-endian opcode + args, lengths per g_CommandLengths):
        #   01 85           mp_init_simulants   -> botSpawnAll()
        #   01 45           rebuild_teams
        #   01 46           rebuild_squadrons
        #   00 05 fd 00 00  set_ailist(CHR_SELF=0xfd, GAILIST_IDLE=0x0000)
        #   00 04           endlist
        ailist_code = bytes([
            0x01, 0x85,
            0x01, 0x45,
            0x01, 0x46,
            0x00, 0x05, 0xfd, 0x00, 0x00,
            0x00, 0x04,
        ])
        # Pad bytecode to 4-byte alignment so the following table stays aligned.
        while len(ailist_code) % 4:
            ailist_code += b"\x00"

        ptr_props = 0x20
        ptr_intro = ptr_props + len(props_bin)
        ptr_ailist_code = ptr_intro + len(intro_bin)
        ptr_ailists = ptr_ailist_code + len(ailist_code)

        # ailist table: struct n64_ailist { u32 ptr_list; s32 id; }, null-terminated.
        ailist_table = (
            struct.pack(">Ii", ptr_ailist_code, 0x1000)
            + struct.pack(">Ii", 0, 0)
        )

        ptr_paths = ptr_ailists + len(ailist_table)
        # Empty paths table: a single null struct n64_path (8 bytes) terminator.
        paths_term = b"\x00" * 8

        header = struct.pack(">8I",
            0,                # ptr_waypoints
            0,                # ptr_waygroups
            0,                # ptr_cover
            ptr_intro,
            ptr_props,
            ptr_paths,
            ptr_ailists,
            0,                # ptr_padfiledata
        )
        return header + props_bin + intro_bin + ailist_code + ailist_table + paths_term

    def __repr__(self):
        return f"MapDef({self.name}: {len(self.pads)} pads, {len(self.props)} props, {len(self.intro)} intro)"


def load_level_module(name: str):
    level_path = os.path.join(ROOT, "src", "levels", f"{name}.py")
    if not os.path.exists(level_path):
        raise FileNotFoundError(f"Level module not found: {level_path}")

    spec = importlib.util.spec_from_file_location(f"src.levels.{name}", level_path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    if not hasattr(mod, "build"):
        raise AttributeError(f"Level module {name} has no build() function")

    return mod


def run_tool(tool: str, json_path: str):
    env = dict(os.environ, ROMID=ROMID)
    subprocess.run([sys.executable, tool, json_path], cwd=ROOT, env=env, check=True)


def compile_tiles(name: str, tiles_json_path: str):
    run_tool(MKTILES_TOOL, tiles_json_path)
    return os.path.join(BUILD_DIR, f"bg_{name}_tilesZ")


def compile_pads(name: str, pads_json_path: str):
    run_tool(MKPADS_TOOL, pads_json_path)
    return os.path.join(BUILD_DIR, f"bg_{name}_padsZ")
