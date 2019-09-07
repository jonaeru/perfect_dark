#include <ultra64.h>

char *lang[] = {
	/*0x00*/ "Complete\n",
	/*0x01*/ "Incomplete\n",
	/*0x02*/ "Failed\n",
	/*0x03*/ "\n",
	/*0x04*/ "",
	/*0x05*/ "earth:",
	/*0x06*/ "   pre-history",
	/*0x07*/ "rare presents",
	/*0x08*/ "Red\n",
	/*0x09*/ "Yellow\n",
	/*0x0a*/ "Blue\n",
	/*0x0b*/ "Magenta\n",
	/*0x0c*/ "Cyan\n",
	/*0x0d*/ "Orange\n",
	/*0x0e*/ "Purple\n",
	/*0x0f*/ "Brown\n",
	/*0x10*/ "Joanna Combat\n",
	/*0x11*/ "Joanna Trenchcoat\n",
	/*0x12*/ "Joanna Party Frock\n",
	/*0x13*/ "Joanna Frock (Ripped)\n",
	/*0x14*/ "Joanna Stewardess\n",
	/*0x15*/ "Joanna Wetsuit\n",
	/*0x16*/ "Joanna Aqualung\n",
	/*0x17*/ "Joanna Arctic\n",
	/*0x18*/ "Joanna Lab Tech.\n",
	/*0x19*/ "Elvis\n",
	/*0x1a*/ "Maian\n",
	/*0x1b*/ "Daniel Carrington\n",
	/*0x1c*/ "Carrington Evening Wear\n",
	/*0x1d*/ "Mr. Blonde\n",
	/*0x1e*/ "Cassandra De Vries\n",
	/*0x1f*/ "Trent Easton\n",
	/*0x20*/ "Jonathan\n",
	/*0x21*/ "CI Male Lab Technician\n",
	/*0x22*/ "CI Female Lab Technician\n",
	/*0x23*/ "CI Soldier\n",
	/*0x24*/ "DataDyne Shock Trooper\n",
	/*0x25*/ "DataDyne Female Guard\n",
	/*0x26*/ "DataDyne Security\n",
	/*0x27*/ "DataDyne Guard\n",
	/*0x28*/ "DataDyne Guard 2\n",
	/*0x29*/ "Secretary\n",
	/*0x2a*/ "Office Suit\n",
	/*0x2b*/ "Office Casual\n",
	/*0x2c*/ "Negotiator\n",
	/*0x2d*/ "DataDyne Sniper\n",
	/*0x2e*/ "G5 Guard\n",
	/*0x2f*/ "G5 SWAT Guard\n",
	/*0x30*/ "CIA Agent\n",
	/*0x31*/ "FBI Agent\n",
	/*0x32*/ "Area 51 Guard\n",
	/*0x33*/ "Area 51 Trooper\n",
	/*0x34*/ "Pilot\n",
	/*0x35*/ "Overalls\n",
	/*0x36*/ "Stripes\n",
	/*0x37*/ "Male Lab Technician\n",
	/*0x38*/ "Female Lab Technician\n",
	/*0x39*/ "DataDyne Lab Technician\n",
	/*0x3a*/ "Biotechnician\n",
	/*0x3b*/ "Alaskan Guard\n",
	/*0x3c*/ "Air Force One Pilot\n",
	/*0x3d*/ "Steward\n",
	/*0x3e*/ "Stewardess\n",
	/*0x3f*/ "Head Stewardess\n",
	/*0x40*/ "The President\n",
	/*0x41*/ "NSA Lackey\n",
	/*0x42*/ "Presidential Security\n",
	/*0x43*/ "President\"s Clone\n",
	/*0x44*/ "Pelagic Guard\n",
	/*0x45*/ "Maian Soldier\n",
	/*0x46*/ "Dinner Jacket\n",
	/*0x47*/ "No controller in controller socket 1",
	/*0x48*/ "Please power off and attach a controller",
	/*0x49*/ "DETECTED\n",
	/*0x4a*/ "NOT DETECTED\n",
	/*0x4b*/ "tm\n",
	/*0x4c*/ "N64 EXPANSION PAK\nNintendo 64 Product Identification\n",
	/*0x4d*/ "Product ID:\n",
	/*0x4e*/ "Product Code:\n",
	/*0x4f*/ "Variant:\n",
	/*0x50*/ "Developer:\n",
	/*0x51*/ "Perfect Dark\n",
	/*0x52*/ "NUS-NPDE-USA\n",
	/*0x53*/ "NTSC version 4.4 beta\n",
	/*0x54*/ "Rare Ltd. (twycross)\n",
	/*0x55*/ "Copyright Rare Ltd. 2000\n",
	/*0x56*/ "Perfect Dark and PerfectHead are trademarks of Rare Ltd.\n",
	/*0x57*/ "Presented in\nDolby Surround\n",
	/*0x58*/ "Uses Miles Sound System Copyright 1991-1999 RAD Game Tools Inc.\n",
	/*0x59*/ "MPEG Layer-3 playback supplied with the Miles Sound System from\n",
	/*0x5a*/ "RAD Game Tools, Inc. MPEG Layer-3 audio compression technology\n",
	/*0x5b*/ "licensed by Fraunhofer IIS and THOMSON multimedia\n",
	/*0x5c*/ "rare designs on the future <<<\n",
	/*0x5d*/ "Perfect Dark\n",
	/*0x5e*/ "Choose Your Reality\n",
	/*0x5f*/ "New...\n",
	/*0x60*/ "New Agent...\n",
	/*0x61*/ "Game Files\n",
	/*0x62*/ "Copy:\n",
	/*0x63*/ "Delete:\n",
	/*0x64*/ "Delete Game Notes...\n",
	/*0x65*/ "Single Player Files\n",
	/*0x66*/ "MultiPlayer Game Files\n",
	/*0x67*/ "MultiPlayer Player Files\n",
	/*0x68*/ "PerfectHead Files\n",
	/*0x69*/ "Controller Pak Game Notes Editor\n",
	/*0x6a*/ "Use this editor to delete game notes\nfrom your controller paks.\n",
	/*0x6b*/ "Choose Controller Pak to Edit:\n",
	/*0x6c*/ "Exit\n",
	/*0x6d*/ "Game Pak\n",
	/*0x6e*/ "Controller Pak 1\n",
	/*0x6f*/ "Controller Pak 2\n",
	/*0x70*/ "Controller Pak 3\n",
	/*0x71*/ "Controller Pak 4\n",
	/*0x72*/ "Perfect Menu\n",
	/*0x73*/ "Solo Missions\n",
	/*0x74*/ "Combat Simulator\n",
	/*0x75*/ "Co-Operative\n",
	/*0x76*/ "Counter-Operative\n",
	/*0x77*/ "Cinema\n",
	/*0x78*/ "Mission Select\n",
	/*0x79*/ "Mission 1\n",
	/*0x7a*/ "Mission 2\n",
	/*0x7b*/ "Mission 3\n",
	/*0x7c*/ "Mission 4\n",
	/*0x7d*/ "Mission 5\n",
	/*0x7e*/ "Mission 6\n",
	/*0x7f*/ "Mission 7\n",
	/*0x80*/ "Mission 8\n",
	/*0x81*/ "Mission 9\n",
	/*0x82*/ "Special Assignments\n",
	/*0x83*/ "dataDyne Central",
	/*0x84*/ "- Defection",
	/*0x85*/ "dataDyne Research",
	/*0x86*/ "- Investigation",
	/*0x87*/ "dataDyne Central",
	/*0x88*/ "- Extraction",
	/*0x89*/ "Carrington Villa",
	/*0x8a*/ "- Hostage One",
	/*0x8b*/ "Chicago",
	/*0x8c*/ "- Stealth",
	/*0x8d*/ "G5 Building",
	/*0x8e*/ "- Reconnaissance",
	/*0x8f*/ "Area 51",
	/*0x90*/ "- Infiltration",
	/*0x91*/ "Area 51",
	/*0x92*/ "- Rescue",
	/*0x93*/ "Area 51",
	/*0x94*/ "- Escape",
	/*0x95*/ "Air Base",
	/*0x96*/ "- Espionage",
	/*0x97*/ "Air Force One",
	/*0x98*/ "- Anti-Terrorism",
	/*0x99*/ "Crash Site",
	/*0x9a*/ "- Confrontation",
	/*0x9b*/ "Pelagic II",
	/*0x9c*/ "- Exploration",
	/*0x9d*/ "Deep Sea",
	/*0x9e*/ "- Nullify Threat",
	/*0x9f*/ "Carrington Institute",
	/*0xa0*/ "- Defense",
	/*0xa1*/ "Attack Ship",
	/*0xa2*/ "- Covert Assault",
	/*0xa3*/ "Skedar Ruins",
	/*0xa4*/ "- Battle Shrine",
	/*0xa5*/ "Mr. Blonde's Revenge",
	/*0xa6*/ "Maian SOS",
	/*0xa7*/ "Retaking the Institute",
	/*0xa8*/ "WAR!",
	/*0xa9*/ "The Duel",
	/*0xaa*/ "Mission Status\n",
	/*0xab*/ "Abort!\n",
	/*0xac*/ "Warning\n",
	/*0xad*/ "Do you want to abort the mission?\n",
	/*0xae*/ "Cancel\n",
	/*0xaf*/ "Abort\n",
	/*0xb0*/ "Inventory\n",
	/*0xb1*/ "Weapons Available\n",
	/*0xb2*/ "Options\n",
	/*0xb3*/ "Audio\n",
	/*0xb4*/ "Video\n",
	/*0xb5*/ "Control\n",
	/*0xb6*/ "Display\n",
	/*0xb7*/ "Cheats\n",
	/*0xb8*/ "Save Game\n",
	/*0xb9*/ "Change Agent...\n",
	/*0xba*/ "Warning\n",
	/*0xbb*/ "Do you want to load another agent?\n",
	/*0xbc*/ "Yes\n",
	/*0xbd*/ "No\n",
	/*0xbe*/ "Control Options\n",
	/*0xbf*/ "Control Player 2\n",
	/*0xc0*/ "Control Style\n",
	/*0xc1*/ "Reverse Pitch\n",
	/*0xc2*/ "Look Ahead\n",
	/*0xc3*/ "Head Roll\n",
	/*0xc4*/ "Auto-Aim\n",
	/*0xc5*/ "Aim Control\n",
	/*0xc6*/ "Back\n",
	/*0xc7*/ "Hold\n",
	/*0xc8*/ "Toggle\n",
	/*0xc9*/ "Display Options\n",
	/*0xca*/ "Display Player 2\n",
	/*0xcb*/ "Sight On Screen\n",
	/*0xcc*/ "Always Show Target\n",
	/*0xcd*/ "Show Zoom Range\n",
	/*0xce*/ "Ammo On Screen\n",
	/*0xcf*/ "Show Gun Function\n",
	/*0xd0*/ "Paintball\n",
	/*0xd1*/ "Subtitles\n",
	/*0xd2*/ "Show Mission Time\n",
	/*0xd3*/ "Back\n",
	/*0xd4*/ "Video Options\n",
	/*0xd5*/ "Screen Size\n",
	/*0xd6*/ "Ratio\n",
	/*0xd7*/ "Hi Res\n",
	/*0xd8*/ "2-Player Screen Split\n",
	/*0xd9*/ "Back\n",
	/*0xda*/ "Full\n",
	/*0xdb*/ "Wide\n",
	/*0xdc*/ "Cinema\n",
	/*0xdd*/ "Normal\n",
	/*0xde*/ "16:9\n",
	/*0xdf*/ "Horizontal\n",
	/*0xe0*/ "Vertical\n",
	/*0xe1*/ "Audio Options\n",
	/*0xe2*/ "Sound\n",
	/*0xe3*/ "Music\n",
	/*0xe4*/ "Sound Mode\n",
	/*0xe5*/ "Back\n",
	/*0xe6*/ "Mono",
	/*0xe7*/ "Stereo",
	/*0xe8*/ "Headphone",
	/*0xe9*/ "Surround",
	/*0xea*/ "Control\n",
	/*0xeb*/ "Single\n",
	/*0xec*/ "Double\n",
	/*0xed*/ "1.1\n",
	/*0xee*/ "1.2\n",
	/*0xef*/ "1.3\n",
	/*0xf0*/ "1.4\n",
	/*0xf1*/ "2.1\n",
	/*0xf2*/ "2.2\n",
	/*0xf3*/ "2.3\n",
	/*0xf4*/ "2.4\n",
	/*0xf5*/ "Briefing\n",
	/*0xf6*/ "Select Difficulty\n",
	/*0xf7*/ "Difficulty\n",
	/*0xf8*/ "Best Time\n",
	/*0xf9*/ "Agent\n",
	/*0xfa*/ "Special Agent\n",
	/*0xfb*/ "Perfect Agent\n",
	/*0xfc*/ "Cancel\n",
	/*0xfd*/ "Co-Operative Options\n",
	/*0xfe*/ "Radar On\n",
	/*0xff*/ "Friendly Fire\n",
	/*0x100*/ "Perfect Buddy\n",
	/*0x101*/ "Continue\n",
	/*0x102*/ "Cancel\n",
	/*0x103*/ "Human",
	/*0x104*/ "1 Sim",
	/*0x105*/ "2 Sims",
	/*0x106*/ "3 Sims",
	/*0x107*/ "4 Sims",
	/*0x108*/ "Counter-Operative Options\n",
	/*0x109*/ "Radar On\n",
	/*0x10a*/ "Counter-Operative\n",
	/*0x10b*/ "Continue\n",
	/*0x10c*/ "Cancel\n",
	/*0x10d*/ "Player 1",
	/*0x10e*/ "Player 2",
	/*0x10f*/ "Mission Overview\n",
	/*0x110*/ "Accept\n",
	/*0x111*/ "Decline\n",
	/*0x112*/ "Mission Completed\n",
	/*0x113*/ "Mission Failed\n",
	/*0x114*/ "Mission Status:\n",
	/*0x115*/ "Agent Status:\n",
	/*0x116*/ "Mission Time:\n",
	/*0x117*/ "Weapon of Choice:\n",
	/*0x118*/ "Kills:\n",
	/*0x119*/ "Accuracy:\n",
	/*0x11a*/ "Shot Total:\n",
	/*0x11b*/ "    Head Shots:\n",
	/*0x11c*/ "    Body Shots:\n",
	/*0x11d*/ "    Limb Shots:\n",
	/*0x11e*/ "    Others:\n",
	/*0x11f*/ "Press Start\n",
	/*0x120*/ "Deceased\n",
	/*0x121*/ "Active\n",
	/*0x122*/ "Disavowed\n",
	/*0x123*/ "Failed\n",
	/*0x124*/ "Completed\n",
	/*0x125*/ "Aborted\n",
	/*0x126*/ "Retry Mission\n",
	/*0x127*/ "Next Mission\n",
	/*0x128*/ "Accept\n",
	/*0x129*/ "Decline\n",
	/*0x12a*/ "Objectives\n",
	/*0x12b*/ "Press Start\n",
	/*0x12c*/ "Small but Perfect Menu\n",
	/*0x12d*/ "Combat Simulator\n",
	/*0x12e*/ "Training\n",
	/*0x12f*/ "Audio/Video\n",
	/*0x130*/ "Change Agent\n",
	/*0x131*/ "Audio/Visual\n",
	/*0x132*/ "Sound\n",
	/*0x133*/ "Music\n",
	/*0x134*/ "Sound Mode\n",
	/*0x135*/ "Ratio\n",
	/*0x136*/ "Back\n",
	/*0x137*/ "< Empty >\n",
	/*0x138*/ "DEL",
	/*0x139*/ "CAPS",
	/*0x13a*/ "CANCEL",
	/*0x13b*/ "OK",
	/*0x13c*/ "Combat Simulator Heads\n",
	/*0x13d*/ "Mission Heads\n",
	/*0x13e*/ "Error\n",
	/*0x13f*/ "Cancel\n",
	/*0x140*/ "The pak was not found.\n",
	/*0x141*/ "File was not saved.\n",
	/*0x142*/ "File would not load.\n",
	/*0x143*/ "Could not delete the file.\n",
	/*0x144*/ "Out of Memory.\n",
	/*0x145*/ "This player is already\nloaded for this game.\n",
	/*0x146*/ "Pak has been removed\n",
	/*0x147*/ "Pak is damaged or incorrectly inserted\n",
	/*0x148*/ "Game Note Delete Failed\n",
	/*0x149*/ "Error Loading Game\n",
	/*0x14a*/ "Error Saving Game\n",
	/*0x14b*/ "Error Loading Player\n",
	/*0x14c*/ "Error Saving Player\n",
	/*0x14d*/ "Error Loading PerfectHead\n",
	/*0x14e*/ "Error Saving PerfectHead\n",
	/*0x14f*/ "Error Reading File\n",
	/*0x150*/ "Error Writing File\n",
	/*0x151*/ "Error\n",
	/*0x152*/ "Delete Failed\n",
	/*0x153*/ "Could not delete the file.\n",
	/*0x154*/ "- Check that the Controller Pak is\ninserted correctly.\n",
	/*0x155*/ "Cancel\n",
	/*0x156*/ "There is no space.\n",
	/*0x157*/ "Cool!\n",
	/*0x158*/ "File Saved.\n",
	/*0x159*/ "OK\n",
	/*0x15a*/ "An error occured while\ntrying to save.\n",
	/*0x15b*/ "Try Again\n",
	/*0x15c*/ "Save Elsewhere\n",
	/*0x15d*/ "Cancel\n",
	/*0x15e*/ "An error occured while\ntrying to load.\n",
	/*0x15f*/ "Try Again\n",
	/*0x160*/ "Cancel\n",
	/*0x161*/ "Need Space For Head\n",
	/*0x162*/ "Choose a head to load over:\n",
	/*0x163*/ "OK\n",
	/*0x164*/ "Cancel\n",
	/*0x165*/ "Save\n",
	/*0x166*/ "Would you like to save\nyour file elsewhere?\n",
	/*0x167*/ "Yes\n",
	/*0x168*/ "No\n",
	/*0x169*/ "Please insert the pak\ncontaining your file\n",
	/*0x16a*/ "Please insert the pak\nyou are saving to\n",
	/*0x16b*/ "OK\n",
	/*0x16c*/ "Cancel\n",
	/*0x16d*/ "Select Location\n",
	/*0x16e*/ "Where\n",
	/*0x16f*/ "Spaces\n",
	/*0x170*/ "Delete Files...\n",
	/*0x171*/ "Cancel\n",
	/*0x172*/ "Full",
	/*0x173*/ "Copy File\n",
	/*0x174*/ "Select a file to copy:\n",
	/*0x175*/ "Push <B> to Exit\n",
	/*0x176*/ "Delete File\n",
	/*0x177*/ "Select a file to delete:\n",
	/*0x178*/ "Push <B> to Exit\n",
	/*0x179*/ "Warning\n",
	/*0x17a*/ "Are you sure you want\nto delete this file?\n",
	/*0x17b*/ "Cancel\n",
	/*0x17c*/ "OK\n",
	/*0x17d*/ "Delete Game Note\n",
	/*0x17e*/ "Are you sure you\nwant to delete\nthis game note?\n",
	/*0x17f*/ "No\n",
	/*0x180*/ "Yes\n",
	/*0x181*/ "Game Notes\n",
	/*0x182*/ "Delete Game Notes:\n",
	/*0x183*/ "Note\n",
	/*0x184*/ "Pages\n",
	/*0x185*/ "Push <B> to Exit\n",
	/*0x186*/ "Empty\n",
	/*0x187*/ "--\n",
	/*0x188*/ "Pages Free: \n",
	/*0x189*/ "Pages Free: %d\n",
	/*0x18a*/ "Pages Used: \n",
	/*0x18b*/ "Pages Used: %d\n",
	/*0x18c*/ "Perfect Dark Save already exists on this pak\n",
	/*0x18d*/ "There is enough space for perfect dark saves\n",
	/*0x18e*/ "Perfect Dark Requires %d More Notes\n",
	/*0x18f*/ "Enter Agent Name\n",
	/*0x190*/ "New...\n",
	/*0x191*/ "New Agent...\n",
	/*0x192*/ "New Recruit",
	/*0x193*/ "Mission Time:",
	/*0x194*/ "Challenge 1",
	/*0x195*/ "Challenge 2",
	/*0x196*/ "Challenge 3",
	/*0x197*/ "Challenge 4",
	/*0x198*/ "Challenge 5",
	/*0x199*/ "Challenge 6",
	/*0x19a*/ "Challenge 7",
	/*0x19b*/ "Challenge 8",
	/*0x19c*/ "Challenge 9",
	/*0x19d*/ "Challenge 10",
	/*0x19e*/ "Challenge 11",
	/*0x19f*/ "Challenge 12",
	/*0x1a0*/ "Challenge 13",
	/*0x1a1*/ "Challenge 14",
	/*0x1a2*/ "Challenge 15",
	/*0x1a3*/ "Challenge 16",
	/*0x1a4*/ "Challenge 17",
	/*0x1a5*/ "Challenge 18",
	/*0x1a6*/ "Challenge 19",
	/*0x1a7*/ "Challenge 20",
	/*0x1a8*/ "Challenge 21",
	/*0x1a9*/ "Challenge 22",
	/*0x1aa*/ "Challenge 23",
	/*0x1ab*/ "Challenge 24",
	/*0x1ac*/ "Challenge 25",
	/*0x1ad*/ "Challenge 26",
	/*0x1ae*/ "Challenge 27",
	/*0x1af*/ "Challenge 28",
	/*0x1b0*/ "Challenge 29",
	/*0x1b1*/ "Challenge 30",
	/*0x1b2*/ "Special\n",
	/*0x1b3*/ "Intro\n",
	/*0x1b4*/ "Mission 1\n",
	/*0x1b5*/ "Mission 2\n",
	/*0x1b6*/ "Mission 3\n",
	/*0x1b7*/ "Mission 4\n",
	/*0x1b8*/ "Mission 5\n",
	/*0x1b9*/ "Mission 6\n",
	/*0x1ba*/ "Mission 7\n",
	/*0x1bb*/ "Mission 8\n",
	/*0x1bc*/ "Mission 9\n",
	/*0x1bd*/ "Finale\n",
	/*0x1be*/ "Play All\n",
	/*0x1bf*/ "Intro\n",
	/*0x1c0*/ "1:1 Intro\n",
	/*0x1c1*/ "1:1 Outro\n",
	/*0x1c2*/ "1:2 Intro\n",
	/*0x1c3*/ "1:2 Outro\n",
	/*0x1c4*/ "1:3 Intro\n",
	/*0x1c5*/ "1:3 Outro\n",
	/*0x1c6*/ "2:1 Intro\n",
	/*0x1c7*/ "2:1 Intro 2\n",
	/*0x1c8*/ "2:1 Outro\n",
	/*0x1c9*/ "3:1 Intro\n",
	/*0x1ca*/ "3:1 Outro\n",
	/*0x1cb*/ "3:2 Intro\n",
	/*0x1cc*/ "3:2 Special\n",
	/*0x1cd*/ "3:2 Outro\n",
	/*0x1ce*/ "4:1 Intro\n",
	/*0x1cf*/ "4:1 Outro\n",
	/*0x1d0*/ "4:2 Intro\n",
	/*0x1d1*/ "4:2 Outro\n",
	/*0x1d2*/ "4:3 Intro\n",
	/*0x1d3*/ "4:3 Special\n",
	/*0x1d4*/ "4:3 Outro\n",
	/*0x1d5*/ "5:1 Intro\n",
	/*0x1d6*/ "5:1 Outro\n",
	/*0x1d7*/ "5:2 Intro\n",
	/*0x1d8*/ "5:2 Special\n",
	/*0x1d9*/ "5:2 Outro\n",
	/*0x1da*/ "5:3 Intro\n",
	/*0x1db*/ "5:3 Outro\n",
	/*0x1dc*/ "6:1 Intro\n",
	/*0x1dd*/ "6:1 Outro\n",
	/*0x1de*/ "6:2 Intro\n",
	/*0x1df*/ "6:2 Special\n",
	/*0x1e0*/ "6:2 Outro\n",
	/*0x1e1*/ "7:1 Intro\n",
	/*0x1e2*/ "7:1 Outro\n",
	/*0x1e3*/ "8:1 Intro\n",
	/*0x1e4*/ "8:1 Outro\n",
	/*0x1e5*/ "9:1 Intro\n",
	/*0x1e6*/ "9:1 Outro\n",
	/*0x1e7*/ "End Sequence\n",
	/*0x1e8*/ "Cinema\n",
	/*0x1e9*/ "cloak %d",
	/*0x1ea*/ "Pick Target\n",
	/*0x1eb*/ "Kills Score\n",
	/*0x1ec*/ NULL,
	/*0x1ed*/ NULL,
	/*0x1ee*/ NULL,
	/*0x1ef*/ NULL,
};
