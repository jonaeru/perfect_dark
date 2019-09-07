#include <ultra64.h>

char *lang[] = {
	/*0x00*/ "|Background - \n\nKnocked unconscious during the evacuation of the Carrington Institute, Joanna wakes to find herself a prisoner aboard a Skedar assault ship.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nJoanna! I'm following the assault ship that is holding you! With me are two Maian protectors from the delegation at the White House. If you can get us aboard, we can help you take over the ship.\n\n|Objective One: - Disable Shield System\n\nThe shield system prevents any docking maneuvers from taking place. Drop the shields so Elvis and his companions can start an approach.\n\n|Objective Two: - Open Hangar Doors\n\nThe docking system is normally controlled automatically - it must have been overridden from the bridge. Open the doors using the local controls overlooking the hangar, and Elvis can finish the landing cycle.\n\n|Objective Three: - Access Navigational Systems\n\nGet Elvis to the star map so that he can find out where the assault ship is heading. There might also be the locations of Skedar strongholds within the star map - this information can be used for decisive strikes by the Maian Fleet.\n\n|Objective Four: - Sabotage Engine Systems\n\nThe plan to take over the assault ship might go awry if the Commander alters from the set course. Tamper with the engines so that the ship comes to a halt.\n\n|Objective Five: - Gain Control of Bridge\n\nOnce the bridge is taken, the ship is no longer under Skedar control. Only the Commander remains inside.\n\nEND\n",
	/*0x01*/ "SKEDAR ASSAULT SHIP\n",
	/*0x02*/ "|Background - \n\nKnocked unconscious during the evacuation of the Carrington Institute, Joanna wakes to find herself a prisoner aboard a Skedar assault ship.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nJoanna! I'm following the assault ship that is holding you! With me are two Maian protectors from the delegation at the White House. If you can get us aboard, we can help you take over the ship.\n\n|Objective One: - Disable Shield System\n\nThe shield system prevents any docking maneuvers from taking place. Drop the shields so Elvis and his companions can start an approach.\n\n|Objective Two: - Access Navigational Systems\n\nGet Elvis to the star map so that he can find out where the assault ship is heading. There might also be the locations of Skedar strongholds within the star map - this information can be used for decisive strikes by the Maian Fleet.\n\n|Objective Three: - Sabotage Engine Systems\n\nThe plan to take over the assault ship might go awry if the Commander alters from the set course. Tamper with the engines so that the ship comes to a halt.\n\n|Objective Four: - Gain Control of Bridge\n\nOnce the bridge is taken, the ship is no longer under Skedar control. Only the Commander remains inside.\n\nEND\n",
	/*0x03*/ "|Background - \n\nKnocked unconscious during the evacuation of the Carrington Institute, Joanna wakes to find herself a prisoner aboard a Skedar assault ship.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nJoanna! I'm following the assault ship that is holding you! With me are two Maian protectors from the delegation at the White House. If you can get us aboard, we can help you take over the ship.\n\n|Objective One: - Disable Shield System\n\nThe shield system prevents any docking maneuvers from taking place. Drop the shields so Elvis and his companions can start an approach.\n\n|Objective Two: - Access Navigational Systems\n\nGet Elvis to the star map so that he can find out where the assault ship is heading. There might also be the locations of Skedar strongholds within the star map - this information can be used for decisive strikes by the Maian Fleet.\n\n|Objective Three: - Gain Control of Bridge\n\nOnce the bridge is taken, the ship is no longer under Skedar control. Only the Commander remains inside.\n\nEND\n",
	/*0x04*/ "\n",
	/*0x05*/ "Disable shield system\n",
	/*0x06*/ "Open hangar doors\n",
	/*0x07*/ "Access navigational systems\n",
	/*0x08*/ "Sabotage engine systems\n",
	/*0x09*/ "Gain control of bridge\n",
	/*0x0a*/ "Ship's shields have been lowered.\n",
	/*0x0b*/ "Hangar doors have been opened.\n",
	/*0x0c*/ "Elvis has been killed.\n",
	/*0x0d*/ "Navigational information has been retrieved.\n",
	/*0x0e*/ "Lift has been activated.\n",
	/*0x0f*/ "All hangar Skedar have been killed.\n",
	/*0x10*/ "Outside, Joanna. Get the shields down and \nwe can help you.\n",
	/*0x11*/ "You've got to open the hangar doors so we can dock.\n",
	/*0x12*/ "Okay, we're in. I'll meet you in the hangar.\n",
	/*0x13*/ "Good to see you, Joanna.\n",
	/*0x14*/ "Take this - you should find it useful...\n",
	/*0x15*/ "Received AR34 assault rifle.\n",
	/*0x16*/ "Time to head upwards. I'll take this lift; \nyou take the other one.\n",
	/*0x17*/ "Engines have been disabled.\n",
	/*0x18*/ "Look out, Joanna! I think we've made them angry...\n",
	/*0x19*/ "Bridge has been captured.\n",
	/*0x1a*/ "You go on ahead, Jo. I'll secure the perimeter. \nWe'll meet up later.\n",
	/*0x1b*/ "I've got to get those shields down to let Elvis \nin. I need all the help I can get.\n",
	/*0x1c*/ "Now only the hangar bay doors are in the way.\n",
	/*0x1d*/ "This machine contains the Skedar star maps.\n",
	/*0x1e*/ "If we control the bridge, then the ship is ours.\n",
	/*0x1f*/ "Time to head upwards...\n",
	/*0x20*/ "Oohhh... I'd better not be where I think I am.\n",
	/*0x21*/ "And it's worse than that, my dear. Just look who you've got for company.\n",
	/*0x22*/ "You! I thought you'd managed to escape. We found no trace of you at all.\n",
	/*0x23*/ "You couldn't find me. But there was no hiding from the Skedar, as you and Mr. Carrington just discovered.\n",
	/*0x24*/ "This is it. Wait there! I'll make a distraction; it will give you a chance to get out. Use it or we'll both die.\n",
	/*0x25*/ "Why are you doing this, Cassandra?\n",
	/*0x26*/ "The Skedar used me, Joanna. You are my best chance for revenge.\n",
	/*0x27*/ "Eeeeaaarrrrggghhh!!!\n",
	/*0x28*/ "Wow. That's the first time I've seen another planet from space. It's beautiful.\n",
	/*0x29*/ "Hmmmm?\n",
	/*0x2a*/ "I don't... I don't believe it!\n",
	/*0x2b*/ "Elvis? What is it?\n",
	/*0x2c*/ "Where are you going?\n",
	/*0x2d*/ "We have to get down to the surface! Follow me!\n",
	/*0x2e*/ "We have to get out of here!\n",
	/*0x2f*/ "Hangar doors locked - shields still active.\n",
	/*0x30*/ "Critical mission object destroyed.\n",
	/*0x31*/ "Ammo depleted - consoles can't be destroyed.\n",
	/*0x32*/ "Obtain De Vries' necklace.\n",
	/*0x33*/ "Cassandra De Vries'\n",
	/*0x34*/ "Necklace\n",
	/*0x35*/ "De Vries' necklace\n",
	/*0x36*/ "Picked up De Vries' necklace.\n",
	/*0x37*/ NULL,
};
