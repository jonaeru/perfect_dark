#include <ultra64.h>

char *lang[] = {
	/*0x00*/ "|Background - \n\nThe hidden heart of the Skedar religion is exposed at last - a desert planet burning under three suns. The leader of the Skedar terrorist faction is based at the Battle Shrine on the surface of the planet. The destruction of the Shrine and the death of their leader would break the morale of the Skedar.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nThere's some groundwork to do, Joanna - I need parts of the temple marked as targets for bombardment. And we need to be absolutely certain that the Skedar leader is dead.\n\n|Objective One: - Identify Temple Targets\n\nAttach a targetting device to each of three obelisks on the surface. They form part of a damaged but still functional shield generator that might cause part of the Shrine to escape the bombardment.\n\n|Objective Two: - Activate Bridge\n\nApparently caused many years ago by one of the earthquakes which plague the planet, a deep ravine cuts the inner sanctum area off from the rest of the Shrine. The Skedar have made a retractable bridge to provide access, but you can guarantee it will be well guarded.\n\n|Objective Three: - Gain Access To Inner Sanctum\n\nSearch through the maze of underground corridors to find the Inner Sanctum. Expect resistance to be fierce, fanatical even. This is a shrine to war, and they are very devout in their worship.\n\n|Objective Four: - Destroy Secret Skedar Army\n\nThe Maian High Command suspects that more Skedar Warriors are kept in stasis in unknown locations. This could be one of those places. If it is, destroy them. The last thing we need to face now is a stream of psychotic reinforcements.\n\n|Objective Five: - Assasinate Skedar Leader\n\nThe leader is a warrior much revered by the Skedar. His death will be a hammer blow to their morale, and will mark the end of their warpath throughout the galaxy. Peace for all.\n\nEND\n",
	/*0x01*/ "SKEDAR BATTLE SHRINE\n",
	/*0x02*/ "|Background - \n\nThe hidden heart of the Skedar religion is exposed at last - a desert planet burning under three suns. The leader of the Skedar terrorist faction is based at the Battle Shrine on the surface of the planet. The destruction of the Shrine and the death of their leader would break the morale of the Skedar.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nThere's some groundwork to do, Joanna - I need parts of the temple marked as targets for bombardment. And we need to be absolutely certain that the Skedar leader is dead.\n\n|Objective One: - Identify Temple Targets\n\nAttach a targetting device to each of three obelisks on the surface. They form part of a damaged but still functional shield generator that might cause part of the Shrine to escape the bombardment\n\n|Objective Two: - Activate Bridge\n\nApparently caused many years ago by one of the earthquakes which plague the planet, a deep ravine cuts the inner sanctum area off from the rest of the Shrine. The Skedar have made a retractable bridge to provide access, but you can guarantee it will be well guarded.\n\n|Objective Three: - Gain Access To Inner Sanctum\n\nSearch through the maze of underground corridors to find the Inner Sanctum. Expect resistance to be fierce, fanatical even. This is a shrine to war, and they are very devout in their worship.\n\n|Objective Four: - Destroy Secret Skedar Army\n\nThe Maian High Command suspects that more Skedar Warriors are kept in stasis in unknown locations. This could be one of those places. If it is, destroy them. The last thing we need to face now is a stream of psychotic reinforcements.\n\n|Objective Five: - Assasinate Skedar Leader\n\nThe leader is a warrior much revered by the Skedar. His death will be a hammer blow to their morale, and will mark the end of their warpath throughout the galaxy. Peace for all.\n\nEND\n",
	/*0x03*/ "|Background - \n\nThe hidden heart of the Skedar religion is exposed at last - a desert planet burning under three suns. The leader of the Skedar terrorist faction is based at the Battle Shrine on the surface of the planet. The destruction of the Shrine and the death of their leader would break the morale of the Skedar.\n\n|Carrington - Not Available.\n\n|Elvis - \n\nThere's some groundwork to do, Joanna - I need parts of the temple marked as targets for bombardment. And we need to be absolutely certain that the Skedar leader is dead.\n\n|Objective One: - Identify Temple Targets\n\nAttach a targetting device to each of three obelisks on the surface. They form part of a damaged but still functional shield generator that might cause part of the Shrine to escape the bombardment\n\n|Objective Two: - Gain Access To Inner Sanctum\n\nSearch through the maze of underground corridors to find the Inner Sanctum. Expect resistance to be fierce, fanatical even. This is a shrine to war, and they are very devout in their worship.\n\n|Objective Three: - Destroy Secret Skedar Army\n\nThe Maian High Command suspects that more Skedar Warriors are kept in stasis in unknown locations. This could be one of those places. If it is, destroy them. The last thing we need to face now is a stream of psychotic reinforcements.\n\n|Objective Four: - Assasinate Skedar Leader\n\nThe leader is a warrior much revered by the Skedar. His death will be a hammer blow to their morale, and will mark the end of their warpath throughout the galaxy. Peace for all.\n\nEND\n",
	/*0x04*/ "\n",
	/*0x05*/ "Identify Temple Targets\n",
	/*0x06*/ "Activate Bridge\n",
	/*0x07*/ "Gain Access To Inner Sanctum\n",
	/*0x08*/ "Destroy Secret Skedar Army\n",
	/*0x09*/ "Assasinate Skedar Leader\n",
	/*0x0a*/ "Obtain Night Sight\n",
	/*0x0b*/ "\n",
	/*0x0c*/ "Night Sight\n",
	/*0x0d*/ "Night Sight\n",
	/*0x0e*/ "Picked up Night Sight\n",
	/*0x0f*/ "Obtain Scanner\n",
	/*0x10*/ "\n",
	/*0x11*/ "Scanner\n",
	/*0x12*/ "Scanner\n",
	/*0x13*/ "Picked up Scanner\n",
	/*0x14*/ "Obtain Target Amplifier\n",
	/*0x15*/ "\n",
	/*0x16*/ "Target Amplifier\n",
	/*0x17*/ "Target Amplifier\n",
	/*0x18*/ "Picked up Target Amplifier\n",
	/*0x19*/ "Target Amplifier placed correctly\n",
	/*0x1a*/ "Target Amplifier placed incorrectly\n",
	/*0x1b*/ "Generator De-activated\n",
	/*0x1c*/ "Skedar Inner Sanctum Has Been Reached\n",
	/*0x1d*/ "Skedar Secret Army Has Been Defeated\n",
	/*0x1e*/ "This seems to be one of the special pillars.\n",
	/*0x1f*/ "The power's more constant round here. This must\nbe the inner sanctum.\n",
	/*0x20*/ "OH NO!!! A Skedar army in suspended animation!\n",
	/*0x21*/ "Okay this is it... cut off the head and the \nbody will perish.\n",
	/*0x22*/ "This is the Skedar fanatics' most holy place. They believe this planet is sacred ground. The battle shrine is located at the most holy part of this holy planet.",
	/*0x23*/ "So let me get this straight, its holy. But there's more to it than that, isn't there?",
	/*0x24*/ "Yes. In all of the time that we Maians fought the Skedar, we never found this place. We knew that until it was destroyed, the war would never be over. We fought the Skedar to a standstill, we earned a ceasefire, but we always had to be on our guard. The destruction of this place would mean a chance at true peace.",
	/*0x25*/ "Then we have to be certain that no fanatics survive. I'll go in and take out the leader. You'll have to summon the Maian fleet to level the Shrine.",
	/*0x26*/ "If you're caught on the ground when the fleet gets here you won't stand a chance. ",
	/*0x27*/ "That's a mistake I don't intend to make.",
	/*0x28*/ "Yesss...",
	/*0x29*/ "Joanna! Joanna! Where are you? Are you hurt?",
	/*0x2a*/ "Elvis! Elvis! Over here.",
	/*0x2b*/ "No problem, grab my arms, well get you out in no time.",
	/*0x2c*/ "No, quick! Give me a gun!",
	/*0x2d*/ "Will! You! Just! Let! Go!",
	/*0x2e*/ "I managed to get the fleet to stop the bombardment while I looked for you. They'll start as soon as we're clear.",
	/*0x2f*/ "I'd have got out in time if that one hadn't grabbed my foot.",
	/*0x30*/ "Of course, Joanna, of course.",
	/*0x31*/ "No, I would have.",
	/*0x32*/ "I believe you.",
	/*0x33*/ NULL,
	/*0x34*/ NULL,
	/*0x35*/ NULL,
	/*0x36*/ NULL,
	/*0x37*/ NULL,
};
