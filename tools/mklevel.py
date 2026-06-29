#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import struct
import shutil
import importlib.util
import json
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
ROMID = os.environ.get("ROMID", "ntsc-final")

# ---------------------------------------------------------
# BINARY SETUP MACROS
# ---------------------------------------------------------

def mkword(a, b):
    return ((a << 16) | (b & 0xffff))

class Prop:
    def pack(self) -> bytes:
        raise NotImplementedError

class GenericObject(Prop):
    def __init__(self, type_, scale, model, pad, flags=0, flags2=0, flags3=0, maxdamage=1000):
        self.type_ = type_
        self.scale = scale
        self.model = model
        self.pad = pad
        self.flags = flags
        self.flags2 = flags2
        self.flags3 = flags3
        self.maxdamage = maxdamage

    def pack(self) -> bytes:
        w = struct.pack(
            ">5I",
            mkword(self.scale, self.type_),
            mkword(self.model, self.pad),
            self.flags,
            self.flags2,
            self.flags3
        )
        w += b"\x00" * (14 * 4)
        w += struct.pack(">I", self.maxdamage)
        w += b"\x00" * (2 * 4)
        w += struct.pack(">I", 0x0fff0000)
        return w

class Door(GenericObject):
    def __init__(self, maxfrac, perimfrac, accel, decel, maxspeed, doorflags, doortype, keyflags, autoclosetime, unk88, sibling, unkc4, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x01, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.maxfrac = maxfrac
        self.perimfrac = perimfrac
        self.accel = accel
        self.decel = decel
        self.maxspeed = maxspeed
        self.doorflags = doorflags
        self.doortype = doortype
        self.keyflags = keyflags
        self.autoclosetime = autoclosetime
        self.unk88 = unk88
        self.sibling = sibling
        self.unkc4 = unkc4
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", self.maxfrac)
        b += struct.pack(">I", self.perimfrac)
        b += struct.pack(">I", self.accel)
        b += struct.pack(">I", self.decel)
        b += struct.pack(">I", self.maxspeed)
        b += struct.pack(">I", mkword(self.doorflags, self.doortype))
        b += struct.pack(">I", self.keyflags)
        b += struct.pack(">I", self.autoclosetime)
        b += struct.pack(">13I", 0, 0, 0, self.unk88, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        b += struct.pack(">4I", 0, 0, 0, self.sibling)
        b += struct.pack(">I", 0)
        b += struct.pack(">I", self.unkc4)
        b += struct.pack(">I", 0)
        b += struct.pack(">I", 0xff000000)
        b += struct.pack(">3I", 0, 0, 0)
        return b

class DoorScale(Prop):
    def __init__(self, scale=0x0100):
        self.scale = scale
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x02, self.scale)

class StdObject(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x03, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        return super().pack()

class Key(GenericObject):
    def __init__(self, lockbits, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0):
        super().__init__(type_=0x04, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=1000)
        self.lockbits = lockbits
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">I", self.lockbits)

class Camera(GenericObject):
    def __init__(self, targetpad, yaw, ymaxspeed, maxdist, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x06, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.targetpad = targetpad
        self.yaw = yaw
        self.ymaxspeed = ymaxspeed
        self.maxdist = maxdist
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", mkword(self.targetpad, 0))
        b += struct.pack(">17I", *[0]*17)
        b += struct.pack(">I", self.yaw)
        b += struct.pack(">2I", 0, 0)
        b += struct.pack(">I", self.ymaxspeed)
        b += struct.pack(">I", 0)
        b += struct.pack(">I", self.maxdist)
        b += struct.pack(">I", 0)
        return b

class AmmoCrate(GenericObject):
    def __init__(self, ammotype, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x07, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.ammotype = ammotype
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">I", self.ammotype)

class Chr(Prop):
    def __init__(self, spawnflags, chrnum, pad, body, head, function, padpreset, chrpreset, hear_scale, view_dist, flags, flags2, team, squadron, chair, convtalk, tude, naturalanim, yvisang, teamscandist):
        self.spawnflags = spawnflags
        self.chrnum = chrnum
        self.pad = pad
        self.body = body
        self.head = head
        self.function = function
        self.padpreset = padpreset
        self.chrpreset = chrpreset
        self.hear_scale = hear_scale
        self.view_dist = view_dist
        self.flags = flags
        self.flags2 = flags2
        self.team = team
        self.squadron = squadron
        self.chair = chair
        self.convtalk = convtalk
        self.tude = tude
        self.naturalanim = naturalanim
        self.yvisang = yvisang
        self.teamscandist = teamscandist
    def pack(self) -> bytes:
        b = struct.pack(">2I", 0x09, self.spawnflags)
        b += struct.pack(">I", mkword(self.chrnum, self.pad))
        b += struct.pack(">I", mkword(mkshort(self.body, self.head), self.function))
        b += struct.pack(">I", mkword(self.padpreset, self.chrpreset))
        b += struct.pack(">I", mkword(self.hear_scale, self.view_dist))
        b += struct.pack(">2I", self.flags, self.flags2)
        b += struct.pack(">I", mkword(mkshort(self.team, self.squadron), self.chair))
        b += struct.pack(">I", self.convtalk)
        b += struct.pack(">I", mkword(mkshort(self.tude, self.naturalanim), mkshort(self.yvisang, self.teamscandist)))
        return b

class SingleMonitor(GenericObject):
    def __init__(self, image, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x0a, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.image = image
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">29I", *[0]*29)
        b += struct.pack(">I", mkword(0xffff, mkshort(0xff, self.image)))
        return b

class MultiMonitor(GenericObject):
    def __init__(self, image1, image2, image3, image4, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x0b, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.image1 = image1
        self.image2 = image2
        self.image3 = image3
        self.image4 = image4
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">116I", *[0]*116)
        b += struct.pack(">I", mkword(mkshort(self.image1, self.image2), mkshort(self.image3, self.image4)))
        return b

class Autogun(GenericObject):
    def __init__(self, targetpad, unk64, unk68, unk80, unk84, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x0d, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.targetpad = targetpad
        self.unk64 = unk64
        self.unk68 = unk68
        self.unk80 = unk80
        self.unk84 = unk84
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", mkword(self.targetpad, 0))
        b += struct.pack(">I", 0)
        b += struct.pack(">I", self.unk64)
        b += struct.pack(">I", self.unk68)
        b += struct.pack(">5I", 0,0,0,0,0)
        b += struct.pack(">I", self.unk80)
        b += struct.pack(">I", self.unk84)
        b += struct.pack(">9I", 0,0,0,0,0,0,0,0,0)
        return b

class LinkGuns(Prop):
    def __init__(self, gun1offset, gun2offset):
        self.gun1offset = gun1offset
        self.gun2offset = gun2offset
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x0e, mkword(self.gun1offset, self.gun2offset))

class Debris(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x0f, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)

class Hat(GenericObject):
    def __init__(self, scale=0x0100, model=0, chr_=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x11, scale=scale, model=model, pad=chr_, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)

class LiftDoor(Prop):
    def __init__(self, dooroffset, liftoffset, stopnum):
        self.dooroffset = dooroffset
        self.liftoffset = liftoffset
        self.stopnum = stopnum
    def pack(self) -> bytes:
        return struct.pack(">5I", 0x13, self.dooroffset, self.liftoffset, 0, self.stopnum)

class Shield(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x15, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">3I", 0x00010000, 0, 0)

class Tag(Prop):
    def __init__(self, id, value):
        self.id = id
        self.value = value
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x16, mkword(self.id, self.value), 0, 0)

class BeginObjective(Prop):
    def __init__(self, value, text, diffbit):
        self.value = value
        self.text = text
        self.diffbit = diffbit
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x17, self.value, self.text, self.diffbit)

class EndObjective(Prop):
    def pack(self) -> bytes:
        return struct.pack(">I", 0x18)

class RequireObjectDestroyed(Prop):
    def __init__(self, object_):
        self.object_ = object_
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x19, self.object_)

class CompleteFlags(Prop):
    def __init__(self, stageflag):
        self.stageflag = stageflag
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x1a, self.stageflag)

class FailFlags(Prop):
    def __init__(self, stageflag):
        self.stageflag = stageflag
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x1b, self.stageflag)

class RequireObjectCollected(Prop):
    def __init__(self, object_):
        self.object_ = object_
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x1c, self.object_)

class RequireObjectThrown(Prop):
    def __init__(self, object_):
        self.object_ = object_
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x1d, self.object_)

class RequireObjectHolographed(Prop):
    def __init__(self, object_):
        self.object_ = object_
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x1e, self.object_, 0, 0)

class RequireRoomEntered(Prop):
    def __init__(self, room):
        self.room = room
    def pack(self) -> bytes:
        return struct.pack(">2I", 0x20, self.room)

class RequireObjectThrownInRoom(Prop):
    def __init__(self, object_, pad):
        self.object_ = object_
        self.pad = pad
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x21, self.object_, self.pad, 0)

class Briefing(Prop):
    def __init__(self, value, text):
        self.value = value
        self.text = text
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x23, self.value, self.text, 0)

class GasBottle(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x24, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)

class RenameObject(Prop):
    def __init__(self, objoffset, weapon, text, text2, text3, text4, text5):
        self.objoffset = objoffset
        self.weapon = weapon
        self.text = text
        self.text2 = text2
        self.text3 = text3
        self.text4 = text4
        self.text5 = text5
    def pack(self) -> bytes:
        return struct.pack(">10I", 0x25, self.objoffset, self.weapon, self.text, self.text2, self.text3, self.text4, self.text5, 0, 0)

class PadlockedDoor(Prop):
    def __init__(self, dooroffset, lockoffset):
        self.dooroffset = dooroffset
        self.lockoffset = lockoffset
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x26, self.dooroffset, self.lockoffset, 0)

class Glass(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x2a, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">I", 0)

class Safe(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x2b, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)

class SafeItem(Prop):
    def __init__(self, item, safe, door):
        self.item = item
        self.safe = safe
        self.door = door
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x2c, self.item, self.safe, self.door)

class Camera2(Prop):
    def __init__(self, scale, u1, u2, u3, u4, u5, pad):
        self.scale = scale
        self.u1 = u1
        self.u2 = u2
        self.u3 = u3
        self.u4 = u4
        self.u5 = u5
        self.pad = pad
    def pack(self) -> bytes:
        return struct.pack(">7I", mkword(self.scale, 0x2e), self.u1, self.u2, self.u3, self.u4, self.u5, self.pad)

class TintedGlass(GenericObject):
    def __init__(self, unk5c, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x2f, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.unk5c = unk5c
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">3I", self.unk5c, 0xffff, 0)

class Lift(GenericObject):
    def __init__(self, pad1, pad2, pad3, pad4, door1, door2, door3, door4, accel, maxspeed, unk84, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x30, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.pad1 = pad1
        self.pad2 = pad2
        self.pad3 = pad3
        self.pad4 = pad4
        self.door1 = door1
        self.door2 = door2
        self.door3 = door3
        self.door4 = door4
        self.accel = accel
        self.maxspeed = maxspeed
        self.unk84 = unk84
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", mkword(self.pad1, self.pad2))
        b += struct.pack(">I", mkword(self.pad3, self.pad4))
        b += struct.pack(">4I", self.door1, self.door2, self.door3, self.door4)
        b += struct.pack(">2I", 0, 0)
        b += struct.pack(">I", self.accel)
        b += struct.pack(">I", self.maxspeed)
        b += struct.pack(">I", self.unk84)
        b += struct.pack(">3I", 0, 0, 0)
        return b

class ConditionalScenery(Prop):
    def __init__(self, trigger, unexp, exp):
        self.trigger = trigger
        self.unexp = unexp
        self.exp = exp
    def pack(self) -> bytes:
        return struct.pack(">5I", 0x31, self.trigger, self.unexp, self.exp, 0)

class BlockedPath(Prop):
    def __init__(self, objoffset, waypoint1, waypoint2):
        self.objoffset = objoffset
        self.waypoint1 = waypoint1
        self.waypoint2 = waypoint2
    def pack(self) -> bytes:
        return struct.pack(">4I", 0x32, self.objoffset, mkword(self.waypoint1, self.waypoint2), 0)

class Hoverbike(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x33, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", mkword(mkshort(0x01, 0x1000), 0)) # HOVTYPE_BIKE, HOVFLAG_FIRSTTICK
        b += struct.pack(">33I", *[0]*33)
        return b

class EndProps(Prop):
    def pack(self) -> bytes:
        return struct.pack(">I", 0x34)

class HoverProp(GenericObject):
    def __init__(self, hovtype, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x35, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.hovtype = hovtype
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", mkword(mkshort(self.hovtype, 0x1000), 0))
        b += struct.pack(">16I", *[0]*16)
        return b

class Fan(GenericObject):
    def __init__(self, maxspeed, accel, on, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x36, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.maxspeed = maxspeed
        self.accel = accel
        self.on = on
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">6I", 0, 0, self.maxspeed, 0, self.accel, self.on)

class Hovercar(GenericObject):
    def __init__(self, ailist, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x37, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.ailist = ailist
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", self.ailist)
        b += struct.pack(">14I", *[0]*14)
        return b

class PadEffect(Prop):
    def __init__(self, effect, pad):
        self.effect = effect
        self.pad = pad
    def pack(self) -> bytes:
        return struct.pack(">3I", 0x38, self.effect, self.pad)

class Chopper(GenericObject):
    def __init__(self, ailist, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x39, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.ailist = ailist
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">I", self.ailist)
        b += struct.pack(">34I", *[0]*34)
        return b

class Mine(GenericObject):
    def __init__(self, weapon, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x3a, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
        self.weapon = weapon
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">3I", mkword(mkshort(self.weapon, 0), 0), 1, 0)

class EscaStep(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x3b, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">4I", 0, 0, 0, 0)


class Weapon(GenericObject):
    def __init__(self, weapon, scale=0x0100, model=0, chr_=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x08, scale=scale, model=model, pad=chr_, flags=flags, flags2=flags2, flags3=flags3, maxdamage=1000)
        self.weapon = weapon
    def pack(self) -> bytes:
        return super().pack() + struct.pack(">3I", mkword(mkshort(self.weapon, 0), 0), 0x00ffffff, 0)

class AmmoCrateMulti(GenericObject):
    def __init__(self, scale=0x0100, model=0, pad=0, flags=0, flags2=0, flags3=0, maxdamage=1000):
        super().__init__(type_=0x14, scale=scale, model=model, pad=pad, flags=flags, flags2=flags2, flags3=flags3, maxdamage=maxdamage)
    def pack(self) -> bytes:
        b = super().pack()
        b += struct.pack(">19I", *[0xffff0000]*19)
        return b

class IntroCmd:
    def pack(self) -> bytes:
        raise NotImplementedError

class Intro:
    class Spawn(IntroCmd):
        def __init__(self, pad):
            self.pad = pad
        def pack(self):
            return struct.pack(">3I", 0, self.pad, 0)

    class Weapon(IntroCmd):
        def __init__(self, weapon_id, pad):
            self.weapon_id = weapon_id
            self.pad = pad
        def pack(self):
            return struct.pack(">4I", 1, self.weapon_id, self.pad, 0)

    class Ammo(IntroCmd):
        def __init__(self, ammotype, pad):
            self.ammotype = ammotype
            self.pad = pad
        def pack(self):
            return struct.pack(">4I", 2, self.ammotype, self.pad, 0)

    class Outfit(IntroCmd):
        def __init__(self, outfit):
            self.outfit = outfit
        def pack(self):
            return struct.pack(">2I", 5, self.outfit)

    class WatchTime(IntroCmd):
        def __init__(self, minutes, seconds):
            self.minutes = minutes
            self.seconds = seconds
        def pack(self):
            return struct.pack(">3I", 7, self.minutes, self.seconds)

    class CreditOffset(IntroCmd):
        def __init__(self, offset):
            self.offset = offset
        def pack(self):
            return struct.pack(">2I", 8, self.offset)
    
    class Case(IntroCmd):
        def __init__(self, pad, team):
            self.pad = pad
            self.team = team
        def pack(self):
            return struct.pack(">3I", 9, self.team, self.pad)
            
    class CaseRespawn(IntroCmd):
        def __init__(self, pad, team):
            self.pad = pad
            self.team = team
        def pack(self):
            return struct.pack(">3I", 10, self.team, self.pad)
            
    class Hill(IntroCmd):
        def __init__(self, pad):
            self.pad = pad
        def pack(self):
            return struct.pack(">2I", 11, self.pad)


# ---------------------------------------------------------
# MAP DEFINITION
# ---------------------------------------------------------

class Pad:
    def __init__(self, id, x, z, y=10, room=0):
        self.id = id
        self.x = x
        self.y = y
        self.z = z
        self.room = room

class Cover:
    def __init__(self, id, x, z, y=10, dir_x=0, dir_y=0, dir_z=-1, special=0, unk1a=14269):
        self.id = id
        self.x = x
        self.y = y
        self.z = z
        self.dir_x = dir_x
        self.dir_y = dir_y
        self.dir_z = dir_z
        self.special = special
        self.unk1a = unk1a

class MapDef:
    def __init__(self, name):
        self.name = name
        self.seg_source = None
        self.pads = []
        self.covers = []
        self.props = []
        self.intro = []
        
        self.tiles_source_template = "mp14" # Default for floor grid

    def add_pad(self, x, z, y=10, room=0):
        p = Pad(len(self.pads), x, z, y, room=room)
        self.pads.append(p)
        return p

    def add_cover(self, x, z, y=10, dir_x=0, dir_y=0, dir_z=-1):
        c = Cover(len(self.covers), x, z, y, dir_x, dir_y, dir_z)
        self.covers.append(c)
        return c

    def add_prop(self, prop):
        self.props.append(prop)

    def add_intro(self, intro):
        self.intro.append(intro)

    def _build_pads_json(self):
        json_pads = []
        for p in self.pads:
            json_pads.append({
                "id": f"PAD_{self.name.upper()}_{p.id:04X}",
                "pos": [p.x, p.y, p.z],
                "dir": [0.0, 1.0, 0.0],
                "up": [0.0, 0.0, -1.0],
                "xmin": -100.0, "xmax": 100.0,
                "ymin": -100.0, "ymax": 100.0,
                "zmin": -100.0, "zmax": 100.0,
                "aiwaitlift": False, "aionlift": False, "aiwalkdirect": False,
                "aidrop": False, "aicrouch": False, "aiignorey": False, "aiduck": False,
                "liftnum": 0, "room": p.room,
            })

        # Auto-connect waypoints to nearest neighbors
        waypoints = []
        room_waygroups = {}
        
        for i, p1 in enumerate(self.pads):
            # Compute distances for graph
            distances = []
            for j, p2 in enumerate(self.pads):
                if i != j:
                    dist = ((p2.x - p1.x)**2 + (p2.z - p1.z)**2)**0.5
                    distances.append((dist, j))
            distances.sort()
            
            neighbours = []
            for dist, j in distances[:6]:
                neighbours.append({
                    "waypoint": f"WAYPOINT_{self.name.upper()}_{j:04X}",
                    "flag4000": False,
                    "flag8000": False
                })
                
            waygroup_id = f"WAYGROUP_{self.name.upper()}_{p1.room:04X}"
            room_waygroups[waygroup_id] = True
            
            waypoints.append({
                "id": f"WAYPOINT_{self.name.upper()}_{i:04X}",
                "pad": f"PAD_{self.name.upper()}_{i:04X}",
                "neighbours": neighbours,
                "waygroup": waygroup_id
            })

        waygroups = []
        for wg_id in room_waygroups.keys():
            # Simply connect every waygroup to every other waygroup for now
            wg_neighbours = []
            for other_id in room_waygroups.keys():
                if wg_id != other_id:
                    wg_neighbours.append({
                        "waygroup": other_id,
                        "flag4000": False,
                        "flag8000": False
                    })
            waygroups.append({
                "id": wg_id,
                "neighbours": wg_neighbours
            })

        cover_json = []
        for c in self.covers:
            cover_json.append({
                "id": f"COVER_{self.name.upper()}_{c.id:04X}",
                "pos": [c.x, c.y, c.z],
                "dir": [c.dir_x, c.dir_y, c.dir_z],
                "special": c.special,
                "unk1a": c.unk1a
            })

        data = {"pads": json_pads, "waypoints": waypoints, "waygroups": waygroups, "cover": cover_json}
        
        out_path = os.path.join(ROOT, "src", "assets", ROMID, "pads", f"{self.name}.json")
        with open(out_path, "w") as fd:
            json.dump(data, fd, indent="\t")
        return out_path

    def _build_tiles_json(self):
        in_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{self.tiles_source_template}.json")
        out_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{self.name}.json")
        
        with open(in_path) as fd:
            data = json.load(fd)

        rooms = {}
        for key, tiles in data["rooms"].items():
            rooms[key.replace(self.tiles_source_template.upper(), self.name.upper())] = tiles
        data["rooms"] = rooms

        with open(out_path, "w") as fd:
            json.dump(data, fd, indent="\t")
        return out_path

    def _pack_setup(self) -> bytes:
        props_bin = b"".join(p.pack() for p in self.props) + struct.pack(">i", 0x34)
        intro_bin = b"".join(i.pack() for i in self.intro) + struct.pack(">I", 12)
        
        # null terminator (8 bytes) for paths and ailists struct arrays so the parser stops
        null_terminator = b"\x00" * 8

        ptr_props = 0x20
        ptr_intro = ptr_props + len(props_bin)
        ptr_paths = ptr_intro + len(intro_bin)
        ptr_ailists = ptr_paths

        header = struct.pack(">8I", 
            0, # ptr_waypoints
            0, # ptr_waygroups
            0, # ptr_cover
            ptr_intro,
            ptr_props,
            ptr_paths,
            ptr_ailists,
            0 # ptr_padfiledata
        )
        return header + props_bin + intro_bin + null_terminator


# ---------------------------------------------------------
# COMPILER AND DEPLOYER
# ---------------------------------------------------------

def run_tool(tool, json_path):
    env = dict(os.environ, ROMID=ROMID)
    subprocess.run([sys.executable, tool, json_path], cwd=ROOT, env=env, check=True)

def deploy_file(src_path, filename):
    targets = [
        os.path.join(ROOT, "mods", "mod_allinone", "files", "bgdata"),
        os.path.join(ROOT, "mods", "mod_gex", "files", "bgdata"),
        os.path.join(ROOT, "..", "perfect_dark", "mods", "mod_allinone", "files", "bgdata"),
        os.path.join(ROOT, "..", "perfect_dark", "mods", "mod_gex", "files", "bgdata"),
        os.path.expanduser("~/Library/CloudStorage/GoogleDrive-moyoteg@gmail.com/My Drive/Games/N64 Decomp/Decompiled/pd-arm64-osx/mods/mod_allinone/files/bgdata"),
        os.path.expanduser("~/Library/CloudStorage/GoogleDrive-moyoteg@gmail.com/My Drive/Games/N64 Decomp/Decompiled/pd-arm64-osx/mods/mod_gex/files/bgdata")
    ]
    for t in targets:
        if os.path.exists(t):
            dst = os.path.join(t, filename)
            shutil.copy(src_path, dst)
            print(f"Deployed {filename} -> {dst}")

def compile_map(mapdef):
    print(f"== Building Unified Map: {mapdef.name} ==")
    
    # 1. Generate JSONs
    pads_json = mapdef._build_pads_json()
    tiles_json = mapdef._build_tiles_json()
    
    # 2. Compile Pads & Tiles
    mktiles = os.path.join(ROOT, "tools", "assetmgr", "mktiles")
    mkpads = os.path.join(ROOT, "tools", "assetmgr", "mkpads")
    run_tool(mktiles, tiles_json)
    run_tool(mkpads, pads_json)
    
    bgdata = os.path.join(ROOT, "build", ROMID, "assets", "files", "bgdata")
    deploy_file(os.path.join(bgdata, f"bg_{mapdef.name}_tilesZ"), f"bg_{mapdef.name}_tilesZ")
    deploy_file(os.path.join(bgdata, f"bg_{mapdef.name}_padsZ"), f"bg_{mapdef.name}_padsZ")
    
    # 3. Compile Setup
    sys.path.insert(0, os.path.join(ROOT, 'tools', 'assetmgr'))
    import assetmgr
    zipped_setup = assetmgr.zip(mapdef._pack_setup())
    setup_path = os.path.join(ROOT, "build", ROMID, f"Ump_setup{mapdef.name}Z")
    with open(setup_path, "wb") as f:
        f.write(zipped_setup)
    
    # Setup goes to `files/`, not `files/bgdata/`
    setup_targets = [
        os.path.join(ROOT, "mods", "mod_allinone", "files"),
        os.path.join(ROOT, "mods", "mod_gex", "files"),
        os.path.join(ROOT, "..", "perfect_dark", "mods", "mod_allinone", "files"),
        os.path.join(ROOT, "..", "perfect_dark", "mods", "mod_gex", "files"),
        os.path.expanduser("~/Library/CloudStorage/GoogleDrive-moyoteg@gmail.com/My Drive/Games/N64 Decomp/Decompiled/pd-arm64-osx/mods/mod_allinone/files"),
        os.path.expanduser("~/Library/CloudStorage/GoogleDrive-moyoteg@gmail.com/My Drive/Games/N64 Decomp/Decompiled/pd-arm64-osx/mods/mod_gex/files")
    ]
    for t in setup_targets:
        if os.path.exists(t):
            dst = os.path.join(t, f"Ump_setup{mapdef.name}Z")
            shutil.copy(setup_path, dst)
            print(f"Deployed setup -> {dst}")
            
    # 4. Copy Segment Geometry
    if mapdef.seg_source:
        src_seg = os.path.join(ROOT, "mods", "mod_gex", "files", "bgdata", mapdef.seg_source)
        seg_path = os.path.join(bgdata, f"bg_{mapdef.name}.seg")
        shutil.copy(src_seg, seg_path)
        deploy_file(seg_path, f"bg_{mapdef.name}.seg")

    print("== Done! ==")

def load_level_module(name):
    py_path = os.path.join(ROOT, "src", "levels", f"{name}.py")
    if not os.path.exists(py_path):
        print(f"Error: {py_path} not found.")
        sys.exit(1)
        
    spec = importlib.util.spec_from_file_location("level_module", py_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: mklevel.py <level_name>")
        sys.exit(1)
        
    level_name = sys.argv[1]
    module = load_level_module(level_name)
    
    if not hasattr(module, 'build'):
        print(f"Error: Module {level_name} has no build() function.")
        sys.exit(1)
        
    mapdef = module.build()
    compile_map(mapdef)
