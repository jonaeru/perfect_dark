import struct


class IntroCmd:
    code: int = 0

    def pack(self) -> bytes:
        raise NotImplementedError


class Spawn(IntroCmd):
    code = 0

    def __init__(self, pad: int):
        self.pad = pad

    def pack(self) -> bytes:
        return struct.pack(">3I", self.code, self.pad, 0)


class Weapon(IntroCmd):
    code = 1

    def __init__(self, weapon_id: int, dualweapon: int = -1):
        self.weapon_id = weapon_id
        self.dualweapon = dualweapon

    def pack(self) -> bytes:
        return struct.pack(">I I i I", self.code, self.weapon_id, self.dualweapon, 0)


class Ammo(IntroCmd):
    code = 2

    def __init__(self, ammotype: int, quantity: int = 100):
        self.ammotype = ammotype
        self.quantity = quantity

    def pack(self) -> bytes:
        return struct.pack(">4I", self.code, self.ammotype, self.quantity, 0)


class Outfit(IntroCmd):
    code = 5

    def __init__(self, outfit: int):
        self.outfit = outfit

    def pack(self) -> bytes:
        return struct.pack(">2I", self.code, self.outfit)


class WatchTime(IntroCmd):
    code = 7

    def __init__(self, minutes: int, seconds: int):
        self.minutes = minutes
        self.seconds = seconds

    def pack(self) -> bytes:
        return struct.pack(">3I", self.code, self.minutes, self.seconds)


class Case(IntroCmd):
    code = 9

    def __init__(self, team: int, pad: int):
        self.team = team
        self.pad = pad

    def pack(self) -> bytes:
        # INTROCMD_CASE is 3 words (code, team, pad) per src/include/intro.h and
        # the host parser's cmd_size table. A trailing word desyncs convertIntro.
        return struct.pack(">3I", self.code, self.team, self.pad)


class CaseRespawn(IntroCmd):
    code = 10

    def __init__(self, team: int, pad: int):
        self.team = team
        self.pad = pad

    def pack(self) -> bytes:
        # INTROCMD_CASERESPAWN is 3 words (code, team, pad); see Case above.
        return struct.pack(">3I", self.code, self.team, self.pad)


class Hill(IntroCmd):
    code = 11

    def __init__(self, pad: int):
        self.pad = pad

    def pack(self) -> bytes:
        return struct.pack(">2I", self.code, self.pad)


class End(IntroCmd):
    code = 12

    def pack(self) -> bytes:
        return struct.pack(">I", self.code)


INTRO_CODE_MAP = {
    0: Spawn,
    1: Weapon,
    2: Ammo,
    5: Outfit,
    7: WatchTime,
    9: Case,
    10: CaseRespawn,
    11: Hill,
    12: End,
}
