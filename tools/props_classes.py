def mkshort(a, b): return ((a << 8) | (b & 0xff))

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
