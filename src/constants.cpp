/*------------------------------------------------------------------------\
 |  constants.c : Program Constants                    www.middle-earth.us |
 |  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
 |  Derived under license from DIKU GAMMA (0.0).                           |
 \------------------------------------------------------------------------*/

#include "structs.h"

const char *verbal_number[] = {
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen",
  "twenty",
  "twenty one",
  "twenty two",
  "twenty three",
  "twenty four",
  "twenty five",
  "twenty six",
  "twenty seven",
  "twenty eight",
  "twenty nine",
  "thirty",
  "thirty one",
  "thirty two",
  "thirty three",
  "thirty four",
  "thirty five",
  "thirty six",
  "thirty seven",
  "thirty eight",
  "thirty nine",
  "forty",
  "forty one",
  "forty two",
  "forty three",
  "forty four",
  "forty five",
  "forty six",
  "forty seven",
  "forty eight",
  "forty nine",
  "fifty",
  "fifty one",
  "fifty two",
  "fifty three",
  "fifty four",
  "fifty five",
  "fifty six",
  "fifty seven",
  "fifty eight",
  "fifty nine",
  "sixty",
  "sixty one",
  "sixty two",
  "sixty three",
  "sixty four",
  "sixty five",
  "sixty six",
  "sixty seven",
  "sixty eight",
  "sixty nine",
  "seventy",
  "seventy one",
  "seventy two",
  "seventy three",
  "seventy four",
  "seventy five",
  "seventy six",
  "seventy seven",
  "seventy eight",
  "seventy nine",
  "eighty",
  "eighty one",
  "eighty two",
  "eighty three",
  "eighty four",
  "eighty five",
  "eighty six",
  "eighty seven",
  "eighty eight",
  "eighty nine",
  "ninety",
  "ninety one",
  "ninety two",
  "ninety three",
  "ninety four",
  "ninety five",
  "ninety six",
  "ninety seven",
  "ninety eight",
  "ninety nine",
  "one hundred",
  "\n"
};


const char *verbose_dirs[] = {
  "the north",
  "the east",
  "the south",
  "the west",
  "above",
  "below",
  "outside",
  "inside",
  "the northeast",
  "the northwest",
  "the southeast",
  "the southwest",
  "\n"
};

const char *season_string[4] = {
  "spring",
  "summer",
  "autumn",
  "winter"
};

const char *month_short_name[12] = {
  "First",
  "Second",
  "Third",
  "Fourth",
  "Fifth",
  "Sixth",
  "Seventh",
  "Eighth",
  "Ninth",
  "Tenth",
  "Eleventh",
  "Twelfth"
};

const char *month_lkup[] = {
  "(null)",
  "Narvinye",
  "Nenime",
  "Sulime",
  "Viresse",
  "Lotesse",
  "Narie",
  "Cermie",
  "Urime",
  "Yavannie",
  "Narquelie",
  "Hisime",
  "Ringare",
  "\n"
};

const char *somatics[] = {
  "an unknown somatic effect",
  "a headache",
  "a minor concussion",
  "a major concussion",
  "a broken left arm",
  "a broken right arm",
  "a broken right leg",
  "a broken rib",
  "a mysterious illness",
  "an admin punishment",
  "a severed right arm",
  "a severed left arm",
  "a severed right leg",
  "a severed leg leg",
  "a severed head",
  "a severed torso",
  "a severed right hand",
  "a severed right leg",
  "a severed head",
  "\n"
};


const char *skills[] = {
  "Unused",
  "Brawling",
  "Small-Blade",
  "Long-Blade",
  "Polearm",
  "Bludgeon",
  "Dodge",
  "Deflect",
  "Sole-Wield",
  "Dual-Wield",
  "Aim",
  "Handgun",
  "Rifle",
  "Machinegun",
  "Gunnery",
  "Explosives",

  "Sneak",
  "Hide",
  "Steal",
  "Picklock",
  "Haggle",
  "Handle",
  "Hunting",
  "First-Aid",
  "Medicine",
  "Scavenge",
  "Eavesdrop",
  "Butchery",

  "Chemistry",
  "Mechanics",
  "Gunsmith",
  "Computerology",
  "Electronics",
  "Biology",
  "Weaponcraft",
  "Armorcraft",
  "Handicraft",
  "Artistry",

  "Education",
  "Voodoo",
  "Common",

  "\n"
};

const char *where[] = {
  "<used as light>          ",	// 0
  "<worn on finger>         ",	// 1
  "<worn on finger>         ",
  "<worn at neck>           ",
  "<worn at neck>           ",
  "<worn on body>           ",
  "<worn on head>           ",
  "<worn on legs>           ",
  "<worn on feet>           ",
  "<worn on hands>          ",
  "<worn on arms>           ",	// 10
  "<worn as shield>         ",
  "<worn about body>        ",
  "<worn about waist>       ",
  "<worn on right wrist>    ",
  "<worn on left wrist>     ",
  "<wielded primary>        ",
  "<wielded secondary>      ",
  "<wielded both hands>     ",
  "<worn around the chest>  ",
  "<worn on belt>           ",	// 20
  "<worn on belt>           ",
  "<across the back>        ",
  "<over the eyes>          ",
  "<worn at throat>         ",
  "<worn on the ears>       ",
  "<worn over shoulder>     ",
  "<worn over shoulder>     ",
  "<worn on right ankle>    ",
  "<worn on left ankle>     ",
  "<worn in hair>           ",	// 30
  "<worn on face>           ",
  "",
  "",
  "<about upper right arm>  ",	// 34
  "<about upper left arm>   ",	// 35
  "<worn over the body>     ",	// 36
  "<worn over the eyes>     ",	// 37
  "<worn around the hips>   ",  // 38
  "<worn as an error>		",  // 39
  "\n"
};

const char *locations[] = {
  "hand",
  "finger",
  "finger",
  "neck",
  "neck",
  "body",
  "head",
  "legs",
  "feet",
  "hands",
  "arms",
  "hands",
  "body",
  "waist",
  "wrist",
  "wrist",
  "hand",
  "hand",
  "hand",
  "hand",
  "belt",
  "belt",
  "back",
  "eyes",
  "throat",
  "ears",
  "shoulder",
  "shoulder",
  "ankle",
  "ankle",
  "hair",
  "face",
  "something",
  "something",
  "arm",
  "arm",
  "body",
  "eyes",
  "\n"
};


const char *color_liquid[] = {
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "black"
};

const char *fullness[] = {
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


const char *exit_bits[] = {
  "IsDoor",
  "Closed",
  "Locked",
  "RSClosed",
  "RSLocked",
  "PickProof",
  "Secret",
  "Trapped",
  "Toll",
  "IsGate",
  "\n"
};

extern const int electric_list [] = {
	ITEM_E_RADIO,
	ITEM_E_LIGHT,
	ITEM_E_PHONE,
	ITEM_E_BOOST,
	ITEM_E_REMOTE,
	ITEM_E_ROBOT,
	ITEM_E_BUG,
	ITEM_E_CLOCK,
	ITEM_E_MEDICAL,
	ITEM_E_GOGGLE,
	ITEM_E_BOOK,
	ITEM_E_BREATHER,
};




const int earth_grid[] = {
  120,				/* Inside */
  320,				/* City */
  170,				/* Road */
  110,				/* Trail */
  100,				/* Field */
  90,				/* Woods */
  80,				/* Forest */
  80,				/* Hills */
  65,				/* Mountains */
  120,				/* Swamp */
  270,				/* Water_swim */
  340,				/* Water_noswim */
  585,				/* Ocean */
  510,				/* Dock */
  230,				/* Reef */
  580,				/* Crowsnest */
  135,				/* Pasture */
  95,				/* Heath */
  75,				/* Pit */
  100				/* Lean-to  */
};

const int wind_grid[] = {
  640,				/* Inside */
  175,				/* City */
  220,				/* Road */
  240,				/* Trail */
  80,				/* Field */
  240,				/* Woods */
  280,				/* Forest */
  120,				/* Hills */
  100,				/* Mountains */
  140,				/* Swamp */
  90,				/* Water_swim */
  60,				/* Water_noswim */
  50,				/* Ocean */
  75,				/* Dock */
  300,				/* Reef */
  55,				/* Crowsnest */
  120,				/* Pasture */
  65,				/* Heath */
  850,				/* Pit */
  100				/* Lean-to */
};

const int fire_grid[] = {
  120,				/* Inside */
  115,				/* City */
  175,				/* Road */
  190,				/* Trail */
  210,				/* Field */
  275,				/* Woods */
  350,				/* Forest */
  150,				/* Hills */
  135,				/* Mountains */
  475,				/* Swamp */
  525,				/* Water_swim */
  675,				/* Water_noswim */
  895,				/* Ocean */
  520,				/* Dock */
  475,				/* Reef */
  340,				/* Crowsnest */
  125,				/* Pasture */
  140,				/* Heath */
  125,				/* Pit */
  100				/* Lean-to  */
};

const int water_grid[] = {
  450,				/* Inside */
  275,				/* City */
  300,				/* Road */
  275,				/* Trail */
  200,				/* Field */
  140,				/* Woods */
  100,				/* Forest */
  175,				/* Hills */
  225,				/* Mountains */
  75,				/* Swamp */
  60,				/* Water_swim */
  50,				/* Water_noswim */
  30,				/* Ocean */
  65,				/* Dock */
  95,				/* Reef */
  100,				/* Crowsnest */
  275,				/* Pasture */
  175,				/* Heath */
  675,				/* Pit */
  100				/* Lean-to  */
};

const int shadow_grid[] = {
  70,				/* Inside */
  90,				/* City */
  200,				/* Road */
  320,				/* Trail */
  410,				/* Field */
  500,				/* Woods */
  540,				/* Forest */
  490,				/* Hills */
  570,				/* Mountains */
  70,				/* Swamp */
  500,				/* Water_swim */
  500,				/* Water_noswim */
  500,				/* Ocean */
  340,				/* Dock */
  300,				/* Reef */
  650,				/* Crowsnest */
  200,				/* Pasture */
  65,				/* Heath */
  520,				/* Pit */
  510,				/* Lean-to */
};



const char *seasons[] = {
  "Spring",
  "Summer",
  "Autumn",
  "Winter",
  "\n"
};

const char *affected_bits[] = {
  "Undefined",
  "Invisible",
  "Infravision",
  "Detect-Invisible",
  "Detect-Magic",
  "Sense-Life",			/* 5 */
  "Transporting",
  "Sanctuary",
  "Group",
  "Curse",
  "Magic-only",			/* 10 */
  "Poison",
  "AScan",
  "AFallback",
  "Undefined",
  "Undefined",			/* 15 */
  "Sleep",
  "Dodge",
  "ASneak",
  "AHide",
  "Fear",			/* 20 */
  "Follow",
  "Hooded",
  "Charm",			/* was affected_bits[21] */
  "\n"
};

const char *smallgood_types[] = {
  "smallgoods",
  "ore",
  "grain",
  "fur",
  "meat",
  "\n"
};

const char *action_bits[] = {
  "Memory",
  "Sentinel",
  "Rescuer",
  "IsNPC",
  "NoVNPC",
  "Aggressive",
  "Stayzone",
  "Fixer",
  "Sent-Aggro",
  "BulkTrader",
  "Shooter",
  "NoBuy",
  "Enforcer",
  "PackAnimal",
  "Vehicle",
  "Stop",
  "Squeezer",
  "Pariah",
  "Mount",
  "Scented",                   /* Mob has, or should have, some venom */
  "PCOwned",
  "Wildlife",			/* Mob won't attack other wildlife */
  "Stayput",			/* Mob saves and reloads after boot */
  "Passive",			/* Mob won't assist clan brother in combat */
  "Auctioneer",			/* Mob is an auctioneer - auctions.cpp */
  "Econzone",			/* NPC, if keeper, uses econ zone price dis/markups */
  "Jailer",
  "\n"
};


const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Unconscious",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};

const char *connected_types[] = {
  "Playing",
  "Entering Name",
  "Confirming Name",
  "Entering Password",
  "Entering New Password",
  "Confirming New password",
  "Choosing Gender",
  "Reading Message of the Day",
  "Main Menu",
  "Changing Password",
  "Confirming Changed Password",
  "Rolling Attributes",
  "Selecting Race",
  "Decoy Screen",
  "Creation Menu",
  "Selecting Attributes",
  "New Player Menu",
  "Documents Menu",
  "Selecting Documentation",
  "Reading Documentation",
  "Picking Skills",
  "New Player",
  "Age Select",
  "Height-Frame Select",
  "New Char Intro Msg",
  "New Char Intro Wait",
  "Creation Comment",
  "Read Reject Message",
  "Web Connection",
  "\n"
};

const char *sex_types[] = {
  "Sexless",
  "Male",
  "Female",
  "\n"
};

const char *sex_noun[] = {
  "it",
  "him",
  "her",
  "\n",
};

const char *weather_room[] = {
  "foggy",
  "cloudy",
  "rainy",
  "stormy",
  "snowy",
  "blizzard",
  "night",
  "nfoggy",
  "nrainy",
  "nstormy",
  "nsnowy",
  "nblizzard",
  "day",
  "\n"
};
