/*------------------------------------------------------------------------\
|  act.movement.c : Movement Module                   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"


extern double round (double x);	// from math.h


int nPartyRoom = 0;
char nPartyDir = -1;

CLAN_DATA *ptrPartyClan = NULL;
QE_DATA *quarter_event_list = NULL;

bool
	is_outdoors(ROOM_DATA *room)
{
	if (!room || room == NULL || room->vnum == NOWHERE)
		return false;

	if (room->sector_type == SECT_MOONCAVE ||
		room->sector_type == SECT_MOONDARK ||
		room->sector_type == SECT_MOONHILL ||
		room->sector_type == SECT_MOONLIGHT ||
		room->sector_type == SECT_MOONMOUNTAIN ||
		room->sector_type == SECT_MOONPLAIN ||
		room->sector_type == SECT_MOONSAND ||
		room->sector_type == SECT_RUINAPARTMENT ||
		room->sector_type == SECT_RUINGYM ||
		room->sector_type == SECT_RUINHIGHWAY ||
		room->sector_type == SECT_RUININSIDE ||
		room->sector_type == SECT_RUINLAB ||
		room->sector_type == SECT_RUINOFFICE ||
		room->sector_type == SECT_RUINOUTSIDE ||
		room->sector_type == SECT_RUINRELIG ||
		room->sector_type == SECT_RUINSTREET ||
		room->sector_type == SECT_RUINSHOP ||
		room->sector_type == SECT_RUINGENERATOR ||
		room->sector_type == SECT_RUINCHEMLAB ||
		room->sector_type == SECT_RUINWORKSHOP ||
		room->sector_type == SECT_RUINKITCHEN ||
		room->sector_type == SECT_RUINDOMICILE ||
		room->sector_type == SECT_RUINUTIL)
		return true;
	else
		return false;

}


const int movement_loss[] =
{
	1,				/* Inside */
	2,				/* MoonPlain */
	4,				/* MoonHill */
	6,				/* MoonMountain */
	6,				/* MoonSand */
	3,				/* MoonCave */
	2,				/* RuinStreet */
	4,				/* RuinFactory */
	4,				/* RuinOffice */
	2,				/* RuinApartment */
	2,				/* RuinHighway */
	2,				/* RuinInside */
	4,				/* RuinOutside */
	0,				/* Space */
	3,				/* Pit */
	1,				/* Leanto */
	10,				/* Lake */
	15,				/* River */
	9,				/* Ocean */
	7,				/* Reef */
	10,				/* Underwater */
	0,				/* Zero-G */
	2,				/* MoonDark */
	2,				/* MoonLight */
	2,				/* RuinLab */
	2,				/* Outside */
	1,				/* Spaceship */
	2,              // RuinGym
	2,              // Ruin Utility
	2,               // Ruin Religion
	2,				//Ruin Shop
	4,				//Ruin Generator
	2,				//Ruin Chemlab
	4,				//Ruin Workshop
	2,				//Ruin Kitchen
	2,				//Ruin Domicile
	0              // FREEFALL

};


const int terrain_walk_time[] =
{
	1,				/* Inside */
	1,				/* MoonPlain */
	3,				/* MoonHill */
	5,				/* MoonMountain */
	5,				/* MoonSand */
	2,				/* MoonCave */
	2,				/* RuinStreet */
	4,				/* RuinFactory */
	4,				/* RuinOffice */
	2,				/* RuinApartment */
	2,				/* RuinHighway */
	1,				/* RuinInside */
	3,				/* RuinOutside */
	10,				/* Space */
	2,				/* Pit */
	0,				/* Leanto */
	5,				/* Lake */
	5,				/* River */
	5,				/* Ocean */
	5,				/* Reef */
	5,				/* Underwater */
	0,				/* Zero-G */
	3,				/* MoonDark */
	3,				/* MoonLight */
	2,				/* RuinLab */
	2,				/* Outside */
	1,				/* Spaceship */
	2,              // RuinGym
	2,              // Ruin Utility
	2,               // Ruin Religion
	2,				//Ruin Shop
	4,				//Ruin Generator
	2,				//Ruin Chemlab
	4,				//Ruin Workshop
	2,				//Ruin Kitchen
	2,				//Ruin Domicile
	0              // Freefall
};


const int terrain_penalty[23][2] =
{
	{20, 20}, // inside
	{20, 20}, // city
	{20, 20}, // road
	{0, 10}, // trail
	{0, 20}, // field
	{10, 20}, // woods
	{30, 30}, // forest
	{0, 20}, // hills
	{10, 20}, // mountain
	{10, 10}, // swamp
	{10, 10}, // dock
	{20, 20}, // crowsnest
	{0, 20}, // pasture
	{10, 20}, // heath
	{20, 20}, // pit
	{20, 20}, // leanto
	{0, 10}, // lake
	{0, 10}, // river
	{0, 10}, // ocean
	{20, 20}, // reef
	{20, 20}, // underwater
	{20, 20}, // Zero-G
	{20, 20} // freefall
};

const char *leg_woundlocs[] =
{
	"groin",
	"rthigh",
	"lthigh",
	"rknee",
	"lknee",
	"rcalf",
	"lcalf",
	"rfoot",
	"lfoot",
	"\n"
};


const char* const relative_dirs[] =
{
	"northern",
	"eastern",
	"southern",
	"western",
	"above",
	"below",
	"outside",
	"inside",
	"northeastern",
	"northwestern",
	"southeastern",
	"southwestern",
};

const char* const dirs[] =
{
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"outside",
	"inside",
	"northeast",
	"northwest",
	"southeast",
	"southwest",
	"\n"
};

const int rev_dir[] =
{
	2,
	3,
	0,
	1,
	5,
	4,
	7,
	6,
	11,
	10,
	9,
	8
};

const char* const speeds[] =
{
	"walk",			/* 1.00 */
	"trudge",			/* 2.50 */
	"pace",			/* 1.60 */
	"jog",			/* 0.66 */
	"run",			/* 0.50 */
	"sprint",			/* 0.33 */
	"immwalk",			/* 0    */
	"swim",			/* 1.60 */
	"crawl",          /* 5.00 */
	"float",		/* 1.60 */
	"\n"
};

const char* const mount_speeds[] =
{
	"trot",
	"walk",
	"slow trot",
	"canter",
	"race",
	"gallop",
	"immwalk",
	"swim",
	"float",
	"\n"
};

const char* const mount_speeds_ing[] =
{
	"trotting",
	"walking",
	"slowly trotting",
	"cantering",
	"racing",
	"galloping",
	"immwalking",
	"swimming",
	"floating",
	"\n"
};

const char *direction[] =
{
	"south",
	"west",
	"north",
	"east",
	"down",
	"up",
	"inside",
	"outside",
	"southwest",
	"southeast",
	"northwest",
	"northeast"
};

const char *rev_d2[] =
{
	"the south",
	"the west",
	"the north",
	"the east",
	"below",
	"above",
	"inside",
	"outside",
	"the southwest",
	"the southeast",
	"the northwest",
	"the northeast"
};

const float move_speeds[] = { 1.00, 2.50, 1.60, 0.50, 0.33, 0.25, 1.60, 3.00 };

/* NOTICE:  Set the define ENCUMBERANCE_ENTRIES in structs.h if
more entries are added to encumberance_info.  */

const struct encumberance_info enc_tab[] =
{
	{300, 0, 0, 0, 0, 1.00, "unencumbered"},
	{600, 5, 2, 0, 0, 1.00, "lightly encumbered"},
	{900, 10, 4, 5, 1, 0.95, "encumbered"},
	{1300, 15, 6, 10, 2, 0.90, "heavily encumbered"},
	{2000, 25, 8, 15, 2, 0.80, "critically encumbered"},
	{9999900, 25, 4, 15, 2, 0.70, "immobile"},
};

const struct fatigue_data fatigue[] =
{
	{
		5, 0.65, "Completely Exhausted"
	},
	{15, 0.70, "Exhausted"},
		{25, 0.75, "Extremely Tired"},
		{35, 0.80, "Tired"},
		{45, 0.85, "Somewhat Tired"},
		{55, 0.90, "Winded"},
		{75, 0.95, "Somewhat Winded"},
		{90, 1.00, "Relatively Fresh"},
		{999, 1.00, "Completely Rested"}
};

// Function that returns true/false if the creature is a small animal,
// for now used only in sneaking so you aren't observed by small critters
// (thus causing you to go insane when you're trying to sneak about)

bool
	skip_sneak_check(CHAR_DATA *ch)
{
	// If we're a non-mobile or OOC, we can ignore this.
	if (lookup_race_int(ch->race, RACE_TYPE))
		return true;
	else
		return false;
}

// Function to determine how a mount handles over terrain.
// Increased chance of falling or mount damage the faster
// you go over harder terrain.

int
	ride_mount (CHAR_DATA *mount, CHAR_DATA *rider, int current, int target, int dir)
{
	int mod = 0;
	int speed;
	int rskill;
	int mskill;
	int check;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *saddle = NULL;

	speed = rider->pc->mount_speed;

	// A small chance you might get a chance of boosting your skill whilst walking, a higher
	// chance later on for more adventerous riding styles.

	if (!number(0,20))
		skill_use(rider, SKILL_HANDLE, 0);

	if (speed == 1)
	{
		return 1; // so long as you walk your mount, you'll not be in any danger.
	}
	else if (speed == 3)
	{
		mod = terrain_penalty[current][0] + terrain_penalty[target][0];
		mod = mod / 2;
		mod += 10;
	}
	else if (speed == 5)
	{
		mod = terrain_penalty[current][1] + terrain_penalty[target][1];
		mod = mod / 2;
		mod += 20;
	}

	// It's difficult to ride without a saddle.

	if (!(saddle = get_equip(mount, WEAR_BACK)))
	{
		mod += 10;
	}

	if (!number(0,5))
		skill_use(rider, SKILL_HANDLE, 0);

	rskill = MIN(rider->skills[SKILL_HANDLE] + 55 - mod, 99);
	mskill = MIN(mount->skills[SKILL_HANDLE] + 55 - mod, 99);

	if (number(1, SKILL_CEILING) > rskill + 40)
	{
		sprintf(buf, "As $n %s %s, you slide and fall from $s back in a particularly graceless display.", mount_speeds[speed], dirs[dir]);
		sprintf(buf2, "As you %s %s, you feel strangely bemused as $N falls from your back.", mount_speeds[speed], dirs[dir]);
		sprintf(buf3, "As $n %s %s, $N slowly slides and falls from $s back.", mount_speeds[speed], dirs[dir]);

		GET_POS (rider) = SIT;
		add_second_affect (SA_STAND, ((25-GET_AGI(rider))+number(1,3)), rider, NULL, NULL, 0);
		rider->mount = NULL;
		mount->mount = NULL;
		act(buf2, false, mount, 0, rider, TO_CHAR | _ACT_FORMAT);
		act(buf, false, mount, 0, rider, TO_VICT | _ACT_FORMAT);
		act(buf3, false, mount, 0, rider, TO_NOTVICT | _ACT_FORMAT);
		return 0;
	}

	check = number(1, SKILL_CEILING);

	mod = check - mskill;

	if (check > mskill)
	{
		if (mod < 15)
		{
			sprintf(buf, "As $n %ss %s, $e trips and stumbles, legs scrabbling for a moment before regaining $s standing.", mount_speeds[speed], dirs[dir]);
			sprintf(buf2, "As you %ss %s, you suddenly trip and stumble, legs scrabbling for a moment before you regain your standing.", mount_speeds[speed], dirs[dir]);
			sprintf(buf3, "As $n %ss %s, $e trips and stumbles, legs scrabbling for a moment before regaining $s standing.", mount_speeds[speed], dirs[dir]);
		}
		else if (mod < 30)
		{
			sprintf(buf, "As $n %ss %s, $e stumbles before falling to the ground.", mount_speeds[speed], dirs[dir]);
			sprintf(buf2, "As you %ss %s, you stumble before falling to the ground.", mount_speeds[speed], dirs[dir]);
			sprintf(buf3, "As $n %ss %s, $e stumbles before falling to the ground.", mount_speeds[speed], dirs[dir]);
			GET_POS (mount) = SIT;
			add_second_affect (SA_STAND, ((25-GET_AGI(mount))+number(1,3)), mount, NULL, NULL, 0);
		}
		else if (mod < 45)
		{
			sprintf(buf, "$n attempts to %ss %s but loses $s footing and hits the ground hard.", mount_speeds[speed], dirs[dir]);
			sprintf(buf2, "As you attempt tos %s %s, you lose your footing and hit the ground hard.", mount_speeds[speed], dirs[dir]);
			sprintf(buf3, "$n attempts to %ss %s but loses $s footing and hits the ground hard.", mount_speeds[speed], dirs[dir]);
			GET_POS (mount) = SIT;
			add_second_affect (SA_STAND, ((25-GET_AGI(mount))+number(1,3)), mount, NULL, NULL, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_BODY), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_BODY), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_LOLEGS), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_LOLEGS), dice (1, 8), 3, 0, 0, 0);
		}
		else
		{
			sprintf(buf, "$n attempts to %ss %s but trips and smashes in to the ground.", mount_speeds[speed], dirs[dir]);
			sprintf(buf2, "You attempt to %ss %s but trip and smash in to the ground.", mount_speeds[speed], dirs[dir]);
			sprintf(buf3, "$n attempts to %ss %s but trips and smashes in to the ground.", mount_speeds[speed], dirs[dir]);
			GET_POS (mount) = SIT;
			add_second_affect (SA_STAND, ((30-GET_AGI(mount))+number(1,3)), mount, NULL, NULL, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_BODY), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_BODY), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_HEAD), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (mount, figure_location (mount, HITLOC_HEAD), dice (1, 12), 3, 0, 0, 0);
		}

		check = number(1, SKILL_CEILING) + mod;

		mod = check - rskill;

		// The harder the horse falls, the harder it is to stay on.

		if (mod < 0)
		{
			sprintf(buf + strlen(buf), " Despite being jolted and shaken in the commotion, you manage to stay atop $n.");
			sprintf(buf2 + strlen(buf2), " Despite the commotion you undergo, $N remains mounted upon you through it.");
			sprintf(buf3 + strlen(buf3), " Shaken and jolted, $N remains atop of $n.");
		}
		else if (mod < 15)
		{
			sprintf(buf + strlen(buf), " You fall slowly from $n's back, landing safely but abruptly on the ground.");
			sprintf(buf2 + strlen(buf2), " You feel $N slide slowly from your back.");
			sprintf(buf3 + strlen(buf3), " $N slides slowly from $n's back, landing with a thump on the ground.");
			GET_POS (rider) = SIT;
			add_second_affect (SA_STAND, ((25-GET_AGI(rider))+number(1,3)), rider, NULL, NULL, 0);
			rider->mount = NULL;
			mount->mount = NULL;
		}
		else if (mod < 30)
		{
			sprintf(buf + strlen(buf), " You are jolted forcefully from $n's back, landing with a painful thud.");
			sprintf(buf2 + strlen(buf2), " You feel $N slip from from your back.");
			sprintf(buf3 + strlen(buf3), " $N is jolted from $n's back, landing hard on the ground.");
			GET_POS (rider) = SIT;
			add_second_affect (SA_STAND, ((25-GET_AGI(rider)) + number(1,3)), rider, NULL, NULL, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_BODY), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_BODY), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_LOLEGS), dice (1, 8), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_HEAD), dice (1, 8), 3, 0, 0, 0);
			rider->mount = NULL;
			mount->mount = NULL;
		}
		else
		{
			sprintf(buf + strlen(buf), " You are flung forcefully from $s back, landing with a sickening crunch.");
			sprintf(buf2 + strlen(buf2), " You feel $N flung from from your back.");
			sprintf(buf3 + strlen(buf3), " $N is forcefully flung from $n's back, landing hard on the ground with a thud and a crunch.");
			GET_POS (rider) = SIT;
			add_second_affect (SA_STAND, ((25-GET_AGI(rider)) + number(1,3)), rider, NULL, NULL, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_BODY), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_BODY), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_HEAD), dice (1, 12), 3, 0, 0, 0);
			wound_to_char (rider, figure_location (rider, HITLOC_NECK), dice (1, 12), 3, 0, 0, 0);
			rider->mount = NULL;
			mount->mount = NULL;
		}

		act(buf2, false, mount, 0, rider, TO_CHAR | _ACT_FORMAT);
		act(buf, false, mount, 0, rider, TO_VICT | _ACT_FORMAT);
		act(buf3, false, mount, 0, rider, TO_NOTVICT | _ACT_FORMAT);


		return 0;
	}

	return 1;
}

void
	do_party (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[AVG_STRING_LENGTH];
	char msg[AVG_STRING_LENGTH];
	char strPartyDir[AVG_STRING_LENGTH];
	char clan[AVG_STRING_LENGTH];

	if (IS_MORTAL (ch))
	{
		send_to_char ("You're still too hungover from the last party!\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	if (*buf)
	{

		if (strcmp ("start", buf) == 0)
		{

			if (nPartyRoom != 0)
			{
				sprintf (msg,
					"Party-hopping is unsupported. Please continue to enjoy the party at %d.\n",
					nPartyRoom);
				send_to_char (msg, ch);
				return;
			}

			sscanf (argument, "%s %d %s", clan, &nPartyRoom, strPartyDir);
			if (!(ptrPartyClan = get_clandef (clan)))
			{
				sprintf (msg, "Clan '%s' is not defined. Aborting.\n", clan);
				send_to_char (msg, ch);
				nPartyRoom = 0;
				return;
			}

			switch (tolower (strPartyDir[0]))
			{
			case 'n':
				nPartyDir = 0;
				break;
			case 'e':
				nPartyDir = 1;
				break;
			case 's':
				nPartyDir = 2;
				break;
			case 'w':
				nPartyDir = 3;
				break;
			case 'u':
				nPartyDir = 4;
				break;
			case 'd':
				nPartyDir = 5;
				break;
			default:
				{
					send_to_char
						("You need to specify the direction of the exit leading into the party.\n",
						ch);
					return;
				}
			}

			sprintf (msg, "Starting party monitor at room %d for %s\n",
				nPartyRoom, ptrPartyClan->literal);
			send_to_gods (msg);
			return;

		}
		else if (strcmp ("end", buf) == 0)
		{

			if (nPartyRoom == 0)
			{
				send_to_char
					("There is no party in progress. Get back to work!\n", ch);
				return;
			}

			sprintf (msg, "Ending party monitor at room %d.\n", nPartyRoom);
			nPartyRoom = 0;
			send_to_gods (msg);
			return;

		}
	}
	send_to_char
		("Usage: party start clanname room dir\n\n\t> party start herenyand 2846 n\n\t> party end\n",
		ch);
	return;

}

int
	suffocated (CHAR_DATA *ch)
{
	char buf[AVG_STRING_LENGTH];
	float damage = 0.0;
	int roll = 0;

	if (!IS_MORTAL (ch)
		|| number (0, 9)
		|| (IS_SET (ch->act, ACT_FLYING) && !(IS_SET(ch->room->room_flags, NOAIR)))
		)
	{
		return 0;
	}

	roll = number (1, 4);

	if (roll == 1)
		act
		("Searing needles of pain lance through your rapidly-dimming field of vision as your lungs burn from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 2)
		act
		("Your vision blurs as your lungs scream for air, your heart pounding painfully in your chest.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 3)
		act
		("A numbing, chilly dullness settles in upon your mind as it slowly dies from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 4)
		act
		("Your limbs and chest burn in agony, and you instinctively open your mouth to gasp for air, only to choke as you realize there is none here to speak of.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	act ("Thrashing sluggishly, $n struggles as they begin to choke.", false,
		ch, 0, 0, TO_ROOM | _ACT_FORMAT);

// Same as drowning damage, suffocating from lack of air in a no atmosphere room is a bad thing.

	damage = number (15, 35);
	damage /= 100;
	damage = (int) (ch->max_move * damage);

	if (ch->move < (int) damage)
		ch->move = 0;
	else
		ch->move -= (int) damage;

	damage = number (2, 15);
	damage /= 100;
	damage = (int) (ch->max_hit * damage);

	if (damage > 0.0 && general_damage (ch, (int) damage))
	{
		sprintf (buf, "Suffocated: %s Room: %d", ch->tname, ch->in_room);
		system_log (buf, true);
		return 1;
	}

	return 0;
}


int
	drowned (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH];
	float damage = 0.0;
	int roll = 0;

	if (!IS_MORTAL (ch)
		|| IS_SET (ch->affected_by, AFF_BREATHE_WATER)
		|| number (0, 9)
		|| (IS_SET (ch->act, ACT_FLYING) && !(ch->room->sector_type == SECT_UNDERWATER))
		)
	{
		return 0;
	}

	roll = number (1, 4);

	if (roll == 1)
		act
		("Searing needles of pain lance through your rapidly-dimming field of vision as your lungs burn from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 2)
		act
		("Your vision blurs as your lungs scream for air, your heart pounding painfully in your chest.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 3)
		act
		("A numbing, chilly dullness settles in upon your mind as it slowly dies from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 4)
		act
		("Your limbs and chest burn in agony, and you instinctively open your mouth to gasp for air, only to choke on icy water as it fills your lungs.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	act ("Thrashing sluggishly, $n struggles in vain against the water.", false,
		ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	/* Saps both fatigue AND bloodloss points - drowning is bad news; saps fatigue */
	/* faster, so it immobilizes the player, while sapping general damage more slowly */
	/* to repesent how agonizing a death drowning can be. */

	damage = number (15, 35);
	damage /= 100;
	damage = (int) (ch->max_move * damage);

	if (ch->move < (int) damage)
		ch->move = 0;
	else
		ch->move -= (int) damage;

	damage = number (2, 15);
	damage /= 100;
	damage = (int) (ch->max_hit * damage);

	if (damage > 0.0 && general_damage (ch, (int) damage))
	{
		sprintf (buf, "Drowned: %s Room: %d", ch->tname, ch->in_room);
		system_log (buf, true);
		return 1;
	}

	return 0;
}

// Free fall check.
int
	floating_check (CHAR_DATA * ch)
{
	if (ch->room->sector_type != SECT_FREEFALL)
		return 0;

	if (!IS_MORTAL (ch))
		return 0;

	/* Want to make sure that as long as they're in a free fall room, they don't suffer encumberance issues. */
}

int
	swimming_check (CHAR_DATA * ch)
{
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *troom = NULL;
	float encumb_percent = 0.0, damage = 0.0;
	bool encumbered = false, exhausted = false;
	int check = 0;

	if (ch->room->sector_type != SECT_RIVER
		&& ch->room->sector_type != SECT_LAKE
		&& ch->room->sector_type != SECT_OCEAN
		&& ch->room->sector_type != SECT_UNDERWATER)
		return 0;

	if (!IS_MORTAL (ch))
		return 0;

	/* IS_ENCUMBERED() won't work, since we want to check if the PC is 'lightly encumbered' */
	/* or above, not fully encumbered or worse. */

	encumbered = (GET_STR (ch) * enc_tab[0].str_mult_wt < IS_CARRYING_W (ch));

	exhausted = (ch->move <= 10);

	encumb_percent =
		1.0 - (float) (CAN_CARRY_W (ch) -
		IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);
	encumb_percent *= 100;

	if (!encumb_percent)
		encumb_percent = 1;

	encumb_percent = (int) (encumb_percent * 4.5);

	if (!encumbered && !exhausted)
		return 0;

	check = ch->con*2 + ch->str + ch->agi + 10 + number(0, 40) - (armor_penalty(ch) * 10);

	if (check > (int) encumb_percent)
	{
		if (ch->room->sector_type != SECT_UNDERWATER && encumbered)
			send_to_char
			("You manage to tread water, staying afloat despite your encumberance. . .\n",
			ch);
		return 0;
	}

	exit = EXIT (ch, DOWN);
	if (exit)
	{
		troom = vnum_to_room (exit->to_room);
		if (troom && troom->sector_type != SECT_UNDERWATER)
			troom = NULL;
	}

	if (!encumbered && !exhausted)
	{
		/* Gradual fatigue sap for swimmers failing their initial skill check. */
		damage = number (3, 15);
		damage /= 100;
		damage = (int) (ch->max_move * damage);
		if (ch->move < (int) damage)
		{
			damage -= ch->move;
			ch->move = 0;
			if (damage > 0.0 && general_damage (ch, (int) damage))
				return 1;
		}
		else
			ch->move -= (int) damage;
	}
	else if (ch->room->sector_type != SECT_UNDERWATER)
	{
		if (number (1, 100) > encumb_percent && (!troom || !exhausted))
		{
			send_to_char
				("You splutter and choke, sinking briefly beneath the surface. . .\n",
				ch);
			act
				("Spluttering and choking, $n sinks briefly beneath the water's surface.",
				false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

			/* Saved check for encumbered, non-exhausted swimmer (or an exhausted swimmer who cannot */
			/* sink any further) inflicts more fatigue damage, eventually inflicting bloodloss damage */
			/* when they're totally exhausted and have no more fatigue to sap. */

			damage = number (3, 25);
			damage /= 100;
			damage = (int) (ch->max_move * damage);
			if (ch->move < (int) damage)
			{
				damage -= ch->move;
				ch->move = 0;
				if (damage > 0.0 && general_damage (ch, (int) damage))
					return 1;
			}
			else
				ch->move -= (int) damage;
		}
		else if (troom)
		{

			/* Failed check for encumbered swimmer, or exhaustion, sends them further down. */

			if (encumbered)
			{
				send_to_char
					("Struggling against the weight of your equipment, you sink below the surface. . .\n",
					ch);
				act
					("Struggling, $n sinks from sight beneath the water's surface.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else if (exhausted)
			{
				send_to_char
					("Utterly exhausted, you sink below the surface. . .\n", ch);
				act
					("Looking utterly exhausted, $n sinks from sight beneath the water's surface.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->vnum);
			do_look (ch, "", 77); // 77: no room display for Brief.
		}
	}
	else
	{
		if (number (1, 100) > encumb_percent && (!troom || !exhausted))
		{
			;
			/* Handle underwater asphyxiation in a different function; */
			/* only check for a failed swim skillcheck to see if they */
			/* sink any further due to encumberance. */
		}
		else if (troom)
		{
			if (encumbered)
			{
				send_to_char
					("You continue to sink helplessly beneath the weight of your gear. . .\n",
					ch);
				act
					("Struggling helplessly, $n continues to sink further downward.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else if (exhausted)
			{
				send_to_char
					("Too exhausted to fight it, you sink even further into the water. . .\n",
					ch);
				act
					("Struggling weakly, looking exhausted, $n sinks further downward.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->vnum);
			do_look (ch, "", 77); // 77: no display of room-desc for brief
		}
	}

	return 0;
}

void
	do_outside (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, OUTSIDE);
}

void
	do_inside (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, INSIDE);
}

void
	track_from_room (ROOM_DATA * room, TRACK_DATA * track)
{
	TRACK_DATA *temptrack = NULL;

	if (!room || !track)
		return;

	if (room->tracks == track)             // if our track is the first in the room,
	{
		if (track->next)                     // and we've got other tracks
		{
			room->tracks = track->next; // skip to the next track.
		}

		if (track->race < 1000000 && track->name)                      // die, track->name.
		{
			mem_free(track->name);
		}

		mem_free (track);
		return;
	}

	// Search through the tracks in the room.
	for (temptrack = room->tracks; temptrack; temptrack = temptrack->next)
	{
		if (temptrack->next == track) 	// If we find the track before our temp_track...
		{
			if (track->next)     		// and our track has another in the list after it
			{
				temptrack->next = track->next;  // then set the next track to be the track after the one we're looking to remove
			}
			else                               // otherwise, we're at the end of the list
			{
				temptrack->next = NULL;         // so we should set our next track to be null.
			}
		}
	}

	if (track->race < 1000000 && track->name)                      // die, track->name.
		mem_free(track->name);

	mem_free (track);                     // either way, we need to free track.
}

void
	track_expiration (ROOM_DATA * room, TRACK_DATA * track)
{
	int limit;

	limit = 24;

	if (track->hours_passed > limit)
	{
		track_from_room (room, track);
	}
}

void
	update_room_tracks (void)
{
	ROOM_DATA *room = NULL;
	TRACK_DATA *track = NULL;
	int limit;

	for (room = full_room_list; room; room = room->lnext)
	{
		for (track = room->tracks; track;)
		{
			limit = 48;

			//next_track = track->next;
			track->hours_passed++;
			/*
			if (weather_info[room->zone].state == LIGHT_RAIN)
			track->hours_passed++;
			else if (weather_info[room->zone].state == STEADY_RAIN)
			track->hours_passed += 2;
			else if (weather_info[room->zone].state == HEAVY_RAIN)
			track->hours_passed += 3;
			else if (weather_info[room->zone].state == LIGHT_SNOW)
			track->hours_passed += 2;
			else if (weather_info[room->zone].state == STEADY_SNOW)
			track->hours_passed += 4;
			else if (weather_info[room->zone].state == HEAVY_SNOW)
			track->hours_passed += 6;
			*/
			if (track->hours_passed > limit)
			{
				TRACK_DATA *temp = track;
				track = track->next;
				track_from_room (room, temp);
			}
			else
			{
				track = track->next;
			}
		}
	}
}

#define TRACK_LIMIT_PER_ROOM		10

void
	leave_tracks (CHAR_DATA * ch, int to_dir, int from_dir)
{
	TRACK_DATA *track;
	TRACK_DATA *rem_track = NULL;
	WOUND_DATA *wound = NULL;
	int bleeding = 0, i = 0;
	char name[MAX_STRING_LENGTH];

	if (!ch || !ch->room)
		return;

	if (!IS_MORTAL (ch))
		return;

	if (IS_SET (ch->act, ACT_FLYING))
		return;

	if ((ch->room->sector_type == SECT_INSIDE || ch->room->sector_type == SECT_OUTSIDE))
		return;

	/* Flying Creatures, or creatures who don't leave tracks at all */
	if (lookup_race_int(ch->race, RACE_TRACKS) == RACE_TRACKS_NEVER)
	{
		return;
	}

	for (wound = ch->wounds; wound; wound = wound->next)
	{
		bleeding += wound->bleeding;
	}

	/* Critters too small to track, unless bleeding. */
	if (bleeding < 1 && lookup_race_int(ch->race, RACE_TRACKS) == RACE_TRACKS_SMALL)
	{
		return;
	}

	CREATE (track, TRACK_DATA, 1);

	track->race = ch->race;
	track->to_dir = to_dir;
	track->from_dir = from_dir;
	track->speed = ch->speed;
	track->flags = 0;
	track->wildlife = 0;
	track->name = NULL;


	track->next = NULL;



	if (bleeding >= 1)
		track->flags |= BLOODY_TRACK;
	if (!IS_NPC (ch))
	{
		strcpy (name, GET_NAME (ch));
		track->name = add_hash (name);
		track->flags |= PC_TRACK;
	}

	if (IS_SET(ch->act, ACT_WILDLIFE))
		track->wildlife = 1;


	if (ch->room->tracks)
		track->next = ch->room->tracks;

	ch->room->tracks = track;

	for (rem_track = ch->room->tracks; rem_track;)
	{
		i++;
		if (i > TRACK_LIMIT_PER_ROOM)
		{
			TRACK_DATA *temp = rem_track;
			rem_track = rem_track->next;
			track_from_room (ch->room, temp);
		}
		else
		{
			rem_track = rem_track->next;
		}
	}
}

void
	clear_pmote (CHAR_DATA * ch)
{

	if (ch->pmote_str)
	{
		mem_free (ch->pmote_str); // char*
		ch->pmote_str = (char *) NULL;
	}
}

int
	check_climb (CHAR_DATA * ch)
{
	int check = 0;

	if (get_affect (ch, MAGIC_AFFECT_LEVITATE))
		return 1;

	if (IS_SET (ch->act, ACT_FLYING))
		return 1;

	if (!IS_MORTAL (ch))
		return 1;

	/* don't fall through solid ground or through doorways */
	if (!CAN_GO (ch, DOWN) ||
		IS_SET (EXIT (ch, DOWN)->exit_info, EX_ISDOOR) ||
		IS_SET (EXIT (ch, DOWN)->exit_info, EX_ISGATE))
		return 1;

	if (IS_SET (ch->room->room_flags, FALL))
	{
		send_to_char ("You plummet down!\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	if (!IS_SET (ch->room->room_flags, CLIMB))
		return 1;

	// If we're wildlife, we always pass our climb checks.
	if (IS_SET(ch->act, ACT_WILDLIFE))
		return 1;

	if (IS_ENCUMBERED (ch))
	{
		send_to_char ("\nYou are too encumbered to maintain your "
			"balance!  YOU FALL!\n\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	// Will return on average a score between 60 and 100, if you're unarmoured or partial leathers
	// Full leathers, 50 - 90,
	// Partial leather+metal, or metal+fur, 40 - 80
	// Full metal, 30 - 70
	// Full metal + fur: 20, 60.

	check = ((ch->str * 2 + ch->agi * 2) - (armor_penalty(ch) * 10) + number(-10, 30));

	if (number(1,SKILL_CEILING) >= check)
	{
		send_to_char ("\nYou lose your footing and FALL!\n\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	return 1;
}

void
	do_crawl (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (GET_POS(ch) != POSITION_SITTING)
	{
		send_to_char ("You must be sitting to crawl.\n", ch);
		return;
	}

	if (get_affect (ch, MAGIC_SIT_TABLE))
	{
		send_to_char ("You cannot be sitting at a table if you want to crawl.\n", ch);
		return;
	}

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Crawl in which direction?\n", ch);
		return;
	}

	if (IS_RIDER (ch))
	{
		send_to_char ("Dismount first.\n", ch);
		return;
	}
	else if (IS_RIDEE (ch))
	{
		send_to_char ("Your rider must dismount first.\n", ch);
		return;
	}

	do_move (ch, "crawl", dir);
}

void
	do_swim (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Swim in which direction?\n", ch);
		return;
	}

	if (IS_RIDER (ch))
	{
		send_to_char ("Dismount first.\n", ch);
		return;
	}
	else if (IS_RIDEE (ch))
	{
		send_to_char ("Your rider must dismount first.\n", ch);
		return;
	}

	do_move (ch, "swim", dir);
}

void 
	do_float(CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Float in which direction?\n", ch);
		return;
	}

	if (IS_RIDER (ch))
	{
		send_to_char ("Dismount first.\n", ch);
		return;
	}
	else if (IS_RIDEE (ch))
	{
		send_to_char ("Your rider must dismount first.\n", ch);
		return;
	}

	do_move (ch, "float", dir);
}

int
	clear_current_move (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	QE_DATA *qe;
	QE_DATA *tqe;

	/*
	printf ("Clear current move called for %s\n", ch->name);
	*/

	if (GET_FLAG (ch, FLAG_ENTERING))
		return 0;

	for (qe = quarter_event_list; qe; qe = qe->next)
		if (qe->ch == ch)
			break;

	if (!qe)
		return 0;

	if (qe == quarter_event_list)
		quarter_event_list = qe->next;
	else
	{

		for (tqe = quarter_event_list; tqe->next != qe; tqe = tqe->next)
			;

		tqe->next = qe->next;
	}

	ch->flags &= ~FLAG_LEAVING;
	if (qe->travel_str)
		mem_free (qe->travel_str);
	if (qe->group_str)
		mem_free (qe->group_str);
	mem_free (qe); // QE_DATA*

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch || tch->deleted)
			continue;

		if (tch->following == ch && CAN_SEE (tch, ch))
		{
			/*
			printf ("Clearing all moves for %s\n", tch->name);
			*/
			clear_moves (tch);
			clear_current_move (tch);
		}
	}

	return 1;
}

void
	stop_mount_train (CHAR_DATA * stopper)
{
	CHAR_DATA *ch;
	CHAR_DATA *leader;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;

	ch = stopper;
	leader = stopper;

	while (IS_HITCHEE (ch))
	{
		ch = ch->hitcher;
		leader = ch;
	}

	if (!ch)
	{
		printf ("Hey, ch is NULL!\n");
		return;
	}

	while (ch)
	{

		while (ch->moves)
		{
			move = ch->moves;
			ch->moves = move->next;
			if (move->travel_str)
				mem_free (move->travel_str);
			if (move->group_str)
				mem_free (move->group_str);
			mem_free (move); //MOVE_DATA*
		}

		if ((af = get_affect (ch, MAGIC_DRAGGER)))
			affect_remove (ch, af);

		if (IS_HITCHER (ch))
			ch = ch->hitchee;
		else
			ch = NULL;
	}

	if (stopper == leader)
		send_to_char ("You prevent your hitches from moving.\n", leader);
	else
		act ("You prevent the hitches led by $N from moving.",
		true, stopper, 0, leader, TO_CHAR);

	act ("$N prevents your hitches from moving.",
		true, leader, 0, stopper, TO_CHAR);
	act ("The hitches led by $n are stopped by $N.",
		false, leader, 0, stopper, TO_NOTVICT);
}

void
	clear_moves (CHAR_DATA * ch)
{
	MOVE_DATA *move = NULL;
	AFFECTED_TYPE *af;

	while (ch->moves)
	{
		move = ch->moves;
		ch->moves = move->next;
		if (move->travel_str)
			mem_free (move->travel_str);
		if (move->group_str)
			mem_free (move->group_str);
		mem_free (move); // MOVE_DATA*
	}

	if (move && (IS_HITCHER (ch) || IS_HITCHEE (ch)))
	{
		stop_mount_train (ch);
		return;
	}

	ch->flags &= ~(FLAG_ENTERING | FLAG_LEAVING);

	if ((af = get_affect (ch, MAGIC_DRAGGER)))
		affect_remove (ch, af);
}

void
	add_to_qe_list (QE_DATA * qe)
{
	QE_DATA *tqe;

	if (!quarter_event_list || quarter_event_list->event_time > qe->event_time)
	{
		qe->next = quarter_event_list;
		quarter_event_list = qe;
		return;
	}

	for (tqe = quarter_event_list; tqe->next; tqe = tqe->next)
		if (tqe->next->event_time > qe->event_time)
			break;

	qe->next = tqe->next;
	tqe->next = qe;
}

int
	is_moving_toward (CHAR_DATA * target, int dir, CHAR_DATA * archer)
{
	int line;

	if (!target || !archer)
		return 0;

	line = track (target, archer->in_room);

	if (line == dir)
		return 1;

	return 0;
}

int
	is_moving_away (CHAR_DATA * target, int dir, CHAR_DATA * archer)
{
	ROOM_DATA *troom;
	int line = 0, i = 0, range = 0;

	if (!target || !archer)
		return 0;

	troom = vnum_to_room (target->room->dir_option[dir]->to_room);

	line = track (archer, troom->vnum);

	if (line == dir)
	{
		range = 3;
		if (!range)
			return 0;
		troom = archer->room;
		if (!troom->dir_option[dir])
			return 0;
		troom = vnum_to_room (troom->dir_option[dir]->to_room);
		if (troom->vnum == target->room->dir_option[dir]->to_room)
			return 1;
		for (i = 1; i <= range; i++)
		{
			if (troom->vnum == target->room->dir_option[dir]->to_room)
				return 1;
			if (!troom->dir_option[dir])
				break;
			troom = vnum_to_room (troom->dir_option[dir]->to_room);
		}
	}

	return 0;
}

void
	outside_inside (CHAR_DATA *ch)
{
	char w_phrase[50] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	int wind_case = 0;
	int temp_case = 0;
	char wind[20] = { '\0' };


	// Don't get the message again for another five minutes.
	add_second_affect (SA_OUTDOOR, 300, ch, NULL, NULL, 0);

	if (IS_OUTSIDE (ch))
	{

		if (weather_info[ch->room->zone].temperature < 120)
		{
			wind_case = 0;
			temp_case = 1;
		}
		if (weather_info[ch->room->zone].temperature < 110)
		{
			temp_case = 2;
			wind_case = 1;
		}
		if (weather_info[ch->room->zone].temperature < 100)
		{
			wind_case = 2;
			temp_case = 3;
		}
		if (weather_info[ch->room->zone].temperature < 94)
		{
			wind_case = 3;
			temp_case = 3;
		}
		if (weather_info[ch->room->zone].temperature < 90)
			temp_case = 4;
		if (weather_info[ch->room->zone].temperature < 80)
		{
			wind_case = 4;
			temp_case = 5;
		}
		if (weather_info[ch->room->zone].temperature < 75)
			temp_case = 5;
		if (weather_info[ch->room->zone].temperature < 65)
		{
			wind_case = 5;
			temp_case = 6;
		}
		if (weather_info[ch->room->zone].temperature < 55)
		{
			temp_case = 7;
			wind_case = 6;
		}
		if (weather_info[ch->room->zone].temperature < 47)
		{
			wind_case = 7;
			temp_case = 8;
		}
		if (weather_info[ch->room->zone].temperature < 38)
			temp_case = 9;
		if (weather_info[ch->room->zone].temperature < 33)
		{
			wind_case = 8;
			temp_case = 10;
		}
		if (weather_info[ch->room->zone].temperature < 21)
		{
			wind_case = 9;
			temp_case = 11;
		}
		if (weather_info[ch->room->zone].temperature < 11)
			temp_case = 12;
		if (weather_info[ch->room->zone].temperature < 1)
		{
			wind_case = 10;
			temp_case = 13;
		}
		if (weather_info[ch->room->zone].temperature < -10)
			temp_case = 14;

		sprintf (w_phrase, "%s", sun_phase[time_info.phaseSun]);

		sprintf (buf, "it is a %s %s, and ", temp_phrase[temp_case], w_phrase);

		sprintf(buf3, "\n#6Outside, %s", buf);

		*buf = '\0';
		*buf2 = '\0';

		if (weather_info[ch->room->zone].wind_speed)
		{
			if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
				wind_case++;

			sprintf(buf, "%s", wind_temp_phrase[wind_case]);

			if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
				sprintf (wind, "%s northerly", buf);
			else
				sprintf (wind, "%s westerly", buf);
		}

		switch (weather_info[ch->room->zone].wind_speed)
		{
		case CALM:
			sprintf (buf, "the air is calm and quiet.");
			break;
		case BREEZE:
			sprintf (buf, "there is a %s breeze.", wind);
			break;
		case WINDY:
			sprintf (buf, "there is a %s wind.", wind);
			break;
		}
		sprintf(buf3 + strlen(buf3), "%s", buf);
	}

	*buf = '\0';
	*buf2 = '\0';

	switch (time_info.phaseEarth)
	{
	case PHASE_FULL_EARTH:
		sprintf(buf3 + strlen(buf3), " Against the backdrop of a star-scattered eternal space, the massive form of the Earth is now in full view, its marred surface reflecting an eerie bright white glow upon the moonscape.");
		break;
	case PHASE_GIBBOUS_WANING:
		sprintf(buf3 + strlen(buf3), " In the velvety, star-scattered night sky, the majestic Earth reflects an eerie white glow upon the surface of the Moon, though a thin sliver is overcome by darkness.");
		break;
	case PHASE_THREE_QUARTER:
		sprintf(buf3 + strlen(buf3), " The golden rays of the Sun begin to kiss the Moon's surface as its giant fiery form ascends over the horizon. A large portion of the majestic Earth is plunged into darkness with only a moderate crescent of silvery white remaining.");
		break;
	case PHASE_CRESCENT_WANING:
		sprintf(buf3 + strlen(buf3), " The Earth, now only a very thin sliver of silvery white, is plunged nearly entirely into darkness against an eternal backdrop of twinkling stars. The fiery giant that is the Sun now begins to mercilessly cast its intense heat and light upon the surface of the Moon.");
		break;
	case PHASE_NEW_EARTH:
		sprintf(buf3 + strlen(buf3), " The Sun has now reached its highest point in the sky, its omnipresent form casting blistering heat and intense light mercilessly onto the surface of the Moon. The majestic earth is now cast entirely in darkness, barely visible.");
		break;
	case PHASE_CRESCENT_WAXING:
		sprintf(buf3 + strlen(buf3), " The first signs of light have begun to creep over the surface of the Earth in the sky, giving it a silvery white glow in the form of a large sickle in the sky. The Sun's heat continues to beat mercilessly down upon the Moon, though its position in the sky has lowered.");
		break;
	case PHASE_FIRST_QUARTER:
		sprintf(buf3 + strlen(buf3), " The fiery giant that is the sun has now begun its descent across the Moon sky, casting the surroundings in a golden dusk. A quarter of the majestic Earth is now cast in a silvery reflective light against a backdrop of eternal space and stars.");
		break;
	default:
		sprintf(buf3 + strlen(buf3), " Against a backdrop of eternal space and glittering stars, the majestic Earth is now cast mostly in a silvery white, casting an eerie glow upon the surface of the cold Moon.");
		break;

	}

	char *p = '\0';
	reformat_string (buf3, &p);
	sprintf (buf3, "%s", p);
	mem_free (p); // char*
	sprintf(buf3 + strlen(buf3), "\n");
	send_to_char(buf3, ch);

}

void enter_room (QE_DATA * qe)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	float encumb_percent = 0.0;
	int from_dir = 0;
	int rooms_fallen = 0;
	int temp;
	int roomnum;
	int echo = 0;
	int died = 0, i = 0, sensed = 0, watched = 0;
	int dir;
	int observed = 0;
	ROOM_DATA *next_room;
	ROOM_DATA *prevroom;
	LODGED_OBJECT_INFO *lodged;
	ROOM_DIRECTION_DATA *room_exit;
	ROOM_DIRECTION_DATA *exit;
	CHAR_DATA *ch;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *tmp_ch;
	OBJ_DATA *prog_obj;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *drag_af;
	AFFECTED_TYPE *waf;
	char buf[MAX_STRING_LENGTH];
	char travel_str[MAX_STRING_LENGTH];
	char group_str[MAX_STRING_LENGTH];
	const char *direction2[] =
	{ "north", "east", "south", "west", "up", "down", "outside", "inside", "northeast", "northwest", "southeast", "southwest" };
	const char *dir_names[] =
	{
		"to the north", "to the east", "to the south",
		"to the west", "above", "below", "outside", "inside", "to the northeast", "to the northwest", "to the southeast", "to the southwest"
	};
	bool toward = false, away = false;

	ch = qe->ch;

	int swimability = ch->con*2 + ch->str + ch->agi - 20 + number(0, 40);
    // constitution * 2 + strength + agility - 20 + 0-39

    // If there is no character.
	if ( !ch )
	{
		if ( qe->travel_str )
			mem_free ( qe->travel_str );
		if ( qe->group_str )
			mem_free ( qe->group_str );

		mem_free ( qe ); // QE_DATA*
		return;
	}

    // If for some reason, the character is not in the source room.
	if ( !is_he_there (ch, qe->from_room ) )
	{
		if ( is_he_somewhere( ch ) )
			clear_moves( ch );
		if ( qe->travel_str )
			mem_free ( qe->travel_str );
		if ( qe->group_str)
			mem_free( qe->group_str );

		mem_free( qe ); // QE_DATA*
		return;
	}

    // Some toll code.
	if ( !is_toll_paid( ch, qe->dir ) )
	{
		sprintf (buf, "You must pay a toll before crossing %s.", dirs[qe->dir]);
		act (buf, false, ch, 0, 0, TO_CHAR);
		act ("$n is stopped by $N.",
			true, ch, 0, toll_collector (ch->room, qe->dir), TO_NOTVICT);
		act ("You stop $N for toll.",
			false, toll_collector (ch->room, qe->dir), 0, ch, TO_CHAR);
		clear_moves (ch);
		if (qe->travel_str)
			mem_free (qe->travel_str);
		if (qe->group_str)
			mem_free (qe->group_str);
		mem_free (qe); // QE_DATA*
		return;
	}

    if  ( ch->room->sector_type == SECT_FREEFALL && IS_SET (ch->flags, FLAG_LEAVING)
    	&& IS_ENCUMBERED (ch) && IS_MORTAL (ch)
    	)
    {
				send_to_char( "You float without difficulty despite your encumberance.\n", ch);
    }

	if  (   IS_SWIMMING( ch ) &&
            IS_SET (ch->flags, FLAG_LEAVING) &&
            ch->room->sector_type == SECT_UNDERWATER &&
            IS_ENCUMBERED (ch)&&
            IS_MORTAL (ch)
        )
	{
		encumb_percent =
			1.0 - ( float ) ( CAN_CARRY_W( ch ) -
			IS_CARRYING_W( ch ) ) / ( float ) CAN_CARRY_W( ch );
		encumb_percent *= 100;
		encumb_percent = ( int ) ( encumb_percent * 4.5 );

        // If we are drowning, we'll increase the encumberment.
		if ( IS_DROWNING( ch ) )
			encumb_percent = (int) (encumb_percent * 2.5);

		if ( swimability < encumb_percent )
		{
			if ( IS_ENCUMBERED( ch ) )
				send_to_char( "The sheer weight of your equipment prevents you from making progress.\n", ch);
			else
				send_to_char( "Your leaden limbs refuse to propel your exhausted body in that direction!\n", ch );

			clear_moves( ch );

			if ( qe->travel_str )
				mem_free (qe->travel_str);
			if ( qe->group_str )
				mem_free( qe->group_str );

			mem_free( qe ); // QE_DATA*
			return;
		}
	}

	remove_affect_type (ch, MAGIC_TOLL_PAID);

	if ((af = get_affect (ch, AFFECT_SHADOW)))
		af->a.shadow.edge = -1;

	travel_str[0] = '\0';
	if (qe && qe->travel_str)
	{
		strcpy (travel_str, qe->travel_str);
	}
	else if (ch->travel_str != NULL)
	{
		sprintf (travel_str, ", %s", ch->travel_str);
	}

	group_str[0] = '\0';
	if (qe && qe->group_str)
	{
		strcpy (group_str, qe->group_str);
	}

	if (IS_SET (qe->flags, MF_ARRIVAL))
	{

		ch->flags &= ~FLAG_ENTERING;

		if (IS_SET (ch->flags, FLAG_LEAVING))
			printf ("Leaving still set!!\n");

		if ((af = get_affect (ch, MAGIC_DRAGGER)) &&
			is_he_here (ch, (CHAR_DATA *) af->a.spell.t, 0) && number (0, 1))
			do_wake (ch, GET_NAME ((CHAR_DATA *) af->a.spell.t), 1);

		if (af)
			affect_remove (ch, af);

		remove_affect_type (ch, MAGIC_SNEAK);

		if (ch->moves)
			initiate_move (ch);

		if (qe->travel_str)
			mem_free (qe->travel_str);
		if (qe->group_str)
			mem_free (qe->group_str);
		mem_free (qe); // QE_DATA*
		qe = NULL;

		return;
	}

	if (IS_SET (qe->flags, MF_TOEDGE))
	{

		if (!(af = get_affect (ch, AFFECT_SHADOW)))
		{
			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
		}

		af->a.shadow.edge = qe->dir;

		ch->flags &= ~FLAG_LEAVING;

		if (!af->a.shadow.shadow)
			;

		else if (!is_he_somewhere (af->a.shadow.shadow))
			send_to_char ("You can no longer see who you are shadowing.\n", ch);

		else if (!could_see (ch, af->a.shadow.shadow))
		{
			sprintf (buf, "You lose sight of #5%s#0.",
				char_short (af->a.shadow.shadow));
			send_to_char (buf, ch);
		}

		if (ch->moves)
		{
			if (qe->travel_str)
				mem_free (qe->travel_str);
			if (qe->group_str)
				mem_free (qe->group_str);
			mem_free (qe); // QE_DATA*
			initiate_move (ch);
		}
		else
		{
			sprintf( buf, "$n stops just %s.", dir_names[ qe->dir ] );
			act( buf, false, ch, 0, 0, TO_ROOM );

			sprintf( buf, "You stop just %s.", dir_names[ qe->dir ] );
			act( buf, false, ch, 0, 0, TO_CHAR );

			if( qe->travel_str )
				mem_free( qe->travel_str );
			if ( qe->group_str )
				mem_free( qe->group_str );

			mem_free( qe ); // QE_DATA*
		}

		return;
	}

	room_exit = EXIT( ch, qe->dir );

	if (!room_exit)
	{

		sprintf ( buf, "There doesn't appear to be an exit %s.\n", dirs[ qe->dir ] );
		send_to_char( buf, ch );

		clear_moves( ch );
		ch->flags &= ~FLAG_LEAVING;

		if ( qe->travel_str )
			mem_free( qe->travel_str );
		if ( qe->group_str )
			mem_free( qe->group_str );

		mem_free( qe ); // QE_DATA*
		return;
	}

	if (!IS_SET (qe->flags, MF_PASSDOOR) &&
		IS_SET (room_exit->exit_info, EX_CLOSED))
	{

		if ( room_exit->keyword && strlen( room_exit->keyword ) )
		{
			sprintf( buf, "The %s seems to be closed.\n", fname( room_exit->keyword ) );
			send_to_char (buf, ch);
		} else
			send_to_char( "It seems to be closed.\n", ch );

		sprintf (buf, "$n stops at the closed %s.", fname (room_exit->keyword));
		act (buf, true, ch, 0, 0, TO_ROOM);

		clear_moves (ch);
		ch->flags &= ~FLAG_LEAVING;
		if (qe->travel_str)
			mem_free (qe->travel_str);
		if (qe->group_str)
			mem_free (qe->group_str);
		mem_free (qe); // QE_DATA*

		return;
	}

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					// If they're leaving in a direction you're already watching, then we need to add a tab so we don't keep looking that way.
					if (qe->dir == waf->a.shadow.edge || rev_dir[qe->dir] == waf->a.shadow.edge)
						magic_add_affect (tch, MAGIC_WATCH_SENT, -1, 0, 0, 0, 0);

					if (IS_SET (qe->flags, MF_SNEAK)
						&& odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) < number (1, 100))
					{
						qe->flags &= ~MF_SNEAK;
						sprintf (buf, "To the %s, you see $n stealthily leave %sward.\n",
							direction[rev_dir[waf->a.shadow.edge]], direction2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else if (!IS_SET (qe->flags, MF_SNEAK))
					{
						if (get_second_affect(ch, SA_GFOLLOW, NULL))
						{
							;
						}
						else if (get_second_affect(ch, SA_LEAD, NULL))
						{
							sprintf (buf, "To the %s, you see #5a %s#0 led by $n leave %sward.\n",
								direction[rev_dir[waf->a.shadow.edge]], qe->group_str, direction2[qe->dir]);
							act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						}
						else if (!(are_grouped(ch, tch) && rev_dir[waf->a.shadow.edge] == qe->dir
							&& GET_FLAG(tch, FLAG_ENTERING)))
						{
							sprintf (buf, "To the %s, you see $n leave %sward.\n",
								direction[rev_dir[waf->a.shadow.edge]], direction2[qe->dir]);
							act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						}
					}
				}
			}
		}

		exit = next_room->dir_option[dir];

		if (!(exit) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || IS_SET (qe->flags, MF_SNEAK) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					// If they're leaving in a direction you're already watching, then we need to add a tab so we don't keep looking that way.
					if (qe->dir == waf->a.shadow.edge || rev_dir[qe->dir] == waf->a.shadow.edge)
						magic_add_affect (tch, MAGIC_WATCH_SENT, -1, 0, 0, 0, 0);


					if (get_second_affect(ch, SA_GFOLLOW, NULL))
					{
						;
					}
					else if (get_second_affect(ch, SA_LEAD, NULL))
					{
						sprintf (buf, "Far to the %s, you see #5a %s#0 led by $n leave %sward.\n",
							direction[rev_dir[waf->a.shadow.edge]], qe->group_str, direction2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else
					{
						sprintf (buf, "Far to the %s, you see $n leave %sward.\n",
							direction[rev_dir[waf->a.shadow.edge]], direction2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
				}
			}
		}

		exit = next_room->dir_option[dir];

		if (!(exit) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || IS_SET (qe->flags, MF_SNEAK) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					// This time, we only add the not-tag if they're leaving towards you, because you won't be able to see them any longer.
					if (rev_dir[qe->dir] == waf->a.shadow.edge)
						magic_add_affect (tch, MAGIC_WATCH_SENT, -1, 0, 0, 0, 0);

					if (get_second_affect(ch, SA_GFOLLOW, NULL))
					{
						;
					}
					else if (get_second_affect(ch, SA_LEAD, NULL))
					{
						sprintf (buf, "Very far to the %s, you see #5a %s#0 led by $n leave %sward.\n",
							direction[rev_dir[waf->a.shadow.edge]], qe->group_str, direction2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else
					{
						sprintf (buf, "Very far to the %s, you see $n leave %sward.\n",
							direction[rev_dir[waf->a.shadow.edge]], direction2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
				}
			}
		}

	}

	if (watched)
	{
		ch = tmp_ch;
		watched = 0;
	}


	// Scan all nearby rooms to find out if anybody is aiming at our target.

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->aiming_at == ch)
		{
			if (tch->delay_who)
				mem_free (tch->delay_who);

			tch->delay_who = str_dup (dirs[qe->dir]);
			tch->delay_info1 = 1;

			sprintf (buf, "You carefully hold your aim as your quarry moves away...\n");
			send_to_char (buf, tch);
		}
	}

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)))
			continue;

		for (int z = 1; z <= 3; z++)
		{

			if (!exit || !(next_room = vnum_to_room (exit->to_room))
				|| IS_SET (exit->exit_info, EX_CLOSED))
				continue;

			for (tch = next_room->people; tch; tch = tch->next_in_room)
			{
				if (tch->aiming_at == ch)
				{
					if (!(toward = is_moving_toward (ch, qe->dir, tch))
						&& !(away = is_moving_away (ch, qe->dir, tch)))
					{
						if ((IS_SET (tch->plr_flags, AUTO_FIRE) || (IS_NPC(tch)))
							&& tch->aiming_at == ch
							&& has_firearm(tch, 0))
						{
							sprintf (buf, "Your quarry moves %sward, but before you lose sight. . .\n", direction2[qe->dir]);
							send_to_char(buf, tch);
							do_firearm_fire(tch, "", 1);
							broke_aim(tch, 0);
						}
						else
						{
							sprintf (buf, "You lose sight of your quarry as they move %sward.\n",
								direction2[qe->dir]);
							send_to_char (buf, tch);
							broke_aim(tch, 0);
						}
					}
					else if (toward)
					{
						sprintf (buf, "You carefully hold your aim as your quarry moves closer...\n");
						send_to_char (buf, tch);
						tch->delay_info1 -= 1;
						tch->delay_info1 = MAX(tch->delay_info1, 0);
						tch->delay_info1 = MIN(tch->delay_info1, 3);
					}
					else if (away)
					{
						if (tch->delay_who)
							mem_free (tch->delay_who);

						tch->delay_who = str_dup (dirs[qe->dir]);

						sprintf (buf, "You carefully hold your aim as your quarry moves away...\n");
						tch->delay_info1 += 1;
						tch->delay_info1 = MAX(tch->delay_info1, 0);
						tch->delay_info1 = MIN(tch->delay_info1, 3);
						send_to_char (buf, tch);
					}
					else
					{
						sprintf (buf, "You lose sight of your quarry.\n");
						send_to_char (buf, tch);
						broke_aim(tch, 0);
					}
				}
			}
			exit = next_room->dir_option[dir];
		}
	}

	ch->flags &= ~FLAG_LEAVING;

	if (IS_RIDEE(ch) && ch->mount)
	{
		tmp_ch = ch;
		ch = ch->mount;
		watched = 1;
	}

	if (ch->lodged && GET_ITEM_TYPE (vtoo (ch->lodged->vnum)) == ITEM_WEAPON
		&& !number (0, 2))
	{
		lodged = ch->lodged;
		obj = load_object (lodged->vnum);
		if (obj)
		{
			sprintf (buf,
				"#2%s#0 becomes dislodged with your movement, and falls to the ground.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf,
				"#2%s#0 becomes dislodged as #5%s#0 moves, and falls to the ground.",
				obj_short_desc (obj), char_short (ch));
			buf[2] = toupper (buf[2]);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj_to_room (obj, ch->room->vnum);
			lodge_from_char (ch, lodged);
		}
	}
	for (lodged = ch->lodged; lodged; lodged = lodged->next)
	{
		if (!lodged->next)
			break;
		if (GET_ITEM_TYPE (vtoo (lodged->next->vnum)) != ITEM_WEAPON)
			continue;
		if (!number (0, 2))
			continue;
		obj = load_object (lodged->next->vnum);
		if (!obj)
			continue;
		sprintf (buf,
			"#2%s#0 becomes dislodged with your movement, and falls to the ground.",
			obj_short_desc (obj));
		buf[2] = toupper (buf[2]);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf,
			"#2%s#0 becomes dislodged as #5%s#0 moves, and falls to the ground.",
			obj_short_desc (obj), char_short (ch));
		buf[2] = toupper (buf[2]);
		act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		obj_to_room (obj, ch->room->vnum);
		lodge_from_char (ch, lodged->next);
	}

	if ((drag_af = get_affect (ch, MAGIC_DRAGGER)) &&
		is_he_here (ch, (CHAR_DATA *) drag_af->a.spell.t, 1) &&
		GET_POS ((CHAR_DATA *) drag_af->a.spell.t) <= SLEEP)
	{
		char_from_room ((CHAR_DATA *) drag_af->a.spell.t);
		char_to_room ((CHAR_DATA *) drag_af->a.spell.t, room_exit->to_room);
	}
	else
		drag_af = NULL;

	if (IS_SUBDUER (ch))
	{
		char_from_room (ch->subdue);
		char_to_room (ch->subdue, room_exit->to_room);
	}

	if (IS_MOUNT (ch) && ch->mount && ch->mount->subdue)
	{
		char_from_room (ch->mount->subdue);
		char_to_room (ch->mount->subdue, room_exit->to_room);
		do_look (ch->mount->subdue, "", 77);
	}

	temp = GET_POS (ch);
	GET_POS (ch) = SLEEP;		/* Hack to make him not hear messages */

	shadowers_shadow (ch, room_exit->to_room, qe->dir);

	GET_POS (ch) = temp;

	if (is_mounted (ch))
	{
		char_from_room (ch->mount);
		char_to_room (ch->mount, room_exit->to_room);
	}

	roomnum = ch->in_room;
	prevroom = vnum_to_room (roomnum);

	// Sneakers who make separate successful SNEAK checks leave no tracks.
	// -- which doesn't really make sense.

	//if (!IS_SET (qe->flags, MF_SNEAK) ||
	//    odds_sqrt (ch->skills[SKILL_SNEAK]) < number (1, 100))

	leave_tracks (ch, qe->dir, ch->from_dir);

	char_from_room (ch);

	if ((prevroom->sector_type == SECT_LEANTO
		|| prevroom->sector_type == SECT_PIT) && prevroom->deity <= 15)
	{
		while (prevroom->contents)
		{
			obj = prevroom->contents;
			obj_from_room (&obj, 0);
			obj_to_room (obj, room_exit->to_room);
			if (echo == 0)
				send_to_char ("You take your belongings with you as you leave.\n",
				ch);
			echo = 1;
		}
	}

	char_to_room (ch, room_exit->to_room);

	if (ch->in_room && !ch->room)
	{
		send_to_gods("Error: char moved room but didn't keep ch->room. Let Kith know.");
		ch->room = vnum_to_room(ch->in_room);
	}

	if (qe->dir == 0)
		from_dir = 2;
	else if (qe->dir == 1)
		from_dir = 3;
	else if (qe->dir == 2)
		from_dir = 0;
	else if (qe->dir == 3)
		from_dir = 1;
	else if (qe->dir == 4)
		from_dir = 5;
	else
		from_dir = 4;

	ch->from_dir = from_dir;

	if (!IS_NPC (ch) && !IS_SET (ch->act, ACT_VEHICLE))
		weaken (ch, 0, qe->move_cost, NULL);
	else
		weaken (ch, 0, qe->move_cost / 5, NULL);

	// DISABLED-WEATHER-CODE
	/*if (!get_second_affect(ch, SA_OUTDOOR, NULL) && IS_OUTSIDE (ch)
		&& IS_SET(prevroom->room_flags,INDOORS))
		outside_inside(ch);*/

	for (prog_obj = ch->room->contents; prog_obj; prog_obj = prog_obj->next_content)
	{
		if (prog_obj->ocues)
		{
			char your_buf[MAX_STRING_LENGTH];
			typedef std::multimap<obj_cue,std::string>::const_iterator N;
			std::pair<N,N> range = prog_obj->ocues->equal_range (ocue_on_enter);
			for (N n = range.first; n != range.second; n++)
			{
				std::string cue = n->second;
				if (!cue.empty ())
				{
					strcpy (your_buf, cue.c_str ());
					command_interpreter (ch, your_buf);
				}
			}
		}
	}

	do_look (ch, "", 77); // 77: means do not display room description

	// If we were fleeing, we no longer need to be still fleeing.
    if (IS_SET (ch->flags, FLAG_FLEE))
    {
        ch->flags &= ~FLAG_FLEE;
    }
	/*
	Temporarily disabled.

	quick scan when they enter a room
	if (IS_SET (ch->plr_flags, QUIET_SCAN))
	do_scan (ch, "", 1);
	*/

	if (IS_MORTAL (ch) && ch->speed != SPEED_IMMORTAL)
	{
		if ((prevroom->sector_type != SECT_LAKE
			&& prevroom->sector_type != SECT_RIVER
			&& prevroom->sector_type != SECT_OCEAN
			&& prevroom->sector_type != SECT_UNDERWATER)
			&& SWIM_ONLY (ch->room))
		{
			send_to_char ("\n", ch);
			act ("You enter the water, and begin attempting to swim. . .",
				false, ch, 0, 0, TO_CHAR);
			object__drench (ch, ch->equip, true);
		}
		else if (prevroom->sector_type == SECT_UNDERWATER
			&& ch->room->sector_type == SECT_UNDERWATER)
		{
			send_to_char ("\n", ch);
			act ("You continue your swim beneath the water's surface. . .",
				false, ch, 0, 0, TO_CHAR);
		}
		else
			if ((prevroom->sector_type == SECT_LAKE
				|| prevroom->sector_type == SECT_RIVER
				|| prevroom->sector_type == SECT_OCEAN
				|| prevroom->sector_type == SECT_UNDERWATER)
				&& (ch->room->sector_type != SECT_LAKE
				&& ch->room->sector_type != SECT_RIVER
				&& ch->room->sector_type != SECT_OCEAN
				&& ch->room->sector_type != SECT_UNDERWATER))
			{
				send_to_char ("\n", ch);
				act ("You climb from the water, dripping.", false, ch, 0, 0,
					TO_CHAR);
			}
	}
	else if (IS_SET (ch->plr_flags, AUTOSCAN))
	{
		if ((af = get_affect (ch, AFFECT_WATCH_DIR)))
		{
			if (EXIT (ch, af->a.shadow.edge))
			{
				sprintf(buf, "\n#6Your constant watch %sward reveals the following:#0\n", direction[rev_dir[af->a.shadow.edge]]);
				send_to_char(buf, ch);
				directional_scan(ch, af->a.shadow.edge, 0, 0);
			}
		}
	}

	/*
	if ( !IS_SET (ch->act, ACT_PREY) && !IS_NPC (ch) ) {
	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
	if ( !IS_NPC (tch) || tch->desc )
	continue;
	if ( !IS_SET (tch->act, ACT_PREY) )
	continue;
	if ( get_affect (tch, MAGIC_HIDDEN) )
	continue;
	if ( !CAN_SEE (tch, ch) || (!CAN_SEE (ch, tch) && CAN_SEE (tch, ch)) )
	continue;
	evade_attacker (tch, -1);
	add_threat (tch, ch, 7);
	}
	}
	*/
	while (ch->sighted)
		ch->sighted = ch->sighted->next;	/* Remove list of target sightings. */

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room)))
			continue;
		sensed = 0;
		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if (real_skill (tch, SKILL_VOODOO) && !is_brother (ch, tch)
				&& IS_MORTAL (tch) && IS_SET (ch->act, ACT_AGGRESSIVE))
			{
				if (skill_use (tch, SKILL_VOODOO, 20))
				{
					sprintf (buf,
						"The hairs on the back of your neck prickle as you glance %sward.\n",
						direction[dir]);
					send_to_char (buf, tch);
				}
			}
			if (skill_use (ch, SKILL_VOODOO, 15)
				&& IS_SET (tch->act, ACT_AGGRESSIVE) && !is_brother (tch, ch)
				&& !sensed)
			{
				sprintf (buf,
					"A sense of foreboding fills you as you glance %sward.\n",
					direction2[dir]);
				send_to_char (buf, ch);
				sensed++;
			}
		}
	}

	if (IS_RIDEE(ch) && ch->mount)
	{
		tmp_ch = ch;
		ch = ch->mount;
		watched = 1;
	}

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// If you were flagged, they don't watch no more.
					if (get_affect (tch, MAGIC_WATCH_SENT))
					{
						affect_remove(tch, get_affect (tch, MAGIC_WATCH_SENT));
						continue;
					}

					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					if (IS_SET (qe->flags, MF_SNEAK)
						&& odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) < number (1, 100))
					{
						target_sighted (tch, ch);
						qe->flags &= ~MF_SNEAK;
						sprintf (buf, "To the %s, you see $n arrive stealthily from %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], rev_d2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else if (!IS_SET (qe->flags, MF_SNEAK))
					{
						if (get_second_affect(ch, SA_GFOLLOW, NULL))
						{
							target_sighted (tch, ch);
						}
						else if (get_second_affect(ch, SA_LEAD, NULL))
						{
							sprintf (buf, "To the %s, you see #5a %s#0 led by $n arrives from %s.\n",
								direction[rev_dir[waf->a.shadow.edge]], qe->group_str, rev_d2[qe->dir]);
							act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
							target_sighted (tch, ch);
						}


						// If the guy is in the same group as you are,
						// and the direction you see them arrive in is the
						// same direction you're travelling, and they arrived
						// from that direction in the opposite direction to which
						// you're traveling, don't show it: chances are you're
						// all heading in the same direction.

						else if (!(are_grouped(ch, tch) && waf->a.shadow.edge == qe->dir
							&& GET_FLAG(tch, FLAG_LEAVING)))
						{
							sprintf (buf, "To the %s, you see $n arrive from %s.\n",
								direction[rev_dir[waf->a.shadow.edge]], rev_d2[qe->dir]);
							act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
							target_sighted (tch, ch);
						}
					}

					// We also now see if we've got an overwatch ready and waiting.
					check_overwatch(tch, ch, false);
				}
			}
		}

		exit = next_room->dir_option[dir];

		if (!(exit) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || IS_SET (qe->flags, MF_SNEAK))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// If you were flagged, they don't watch no more.
					if (get_affect (tch, MAGIC_WATCH_SENT))
					{
						affect_remove(tch, get_affect (tch, MAGIC_WATCH_SENT));
						continue;
					}

					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					if (get_second_affect(ch, SA_GFOLLOW, NULL))
					{
						target_sighted (tch, ch);
					}
					else if (get_second_affect(ch, SA_LEAD, NULL))
					{
						sprintf (buf, "Far to the %s, you see #5a %s#0 led by $n arrives from %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], qe->group_str, rev_d2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						target_sighted (tch, ch);
					}
					else
					{
						sprintf (buf, "Far to the %s, you see $n arrive from %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], rev_d2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						target_sighted (tch, ch);
					}

					// We also now see if we've got an overwatch ready and waiting.
					check_overwatch(tch, ch, false);
				}
			}
		}

		exit = next_room->dir_option[dir];

		if (!(exit) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || IS_SET (qe->flags, MF_SNEAK))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge)
				{
					// If you were flagged, they don't watch no more.
					if (get_affect (tch, MAGIC_WATCH_SENT))
					{
						affect_remove(tch, get_affect (tch, MAGIC_WATCH_SENT));
						continue;
					}

					// Obviously, if they can't see you, we shouldn't be reporting on anything.
					//if (!could_see(tch, ch))
					//  continue;

					if (get_second_affect(ch, SA_GFOLLOW, NULL))
					{
						target_sighted (tch, ch);
					}
					else if (get_second_affect(ch, SA_LEAD, NULL))
					{
						sprintf (buf, "Very far to the %s, you see #5a %s#0 led by $n arrives from %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], qe->group_str, rev_d2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						target_sighted (tch, ch);
					}
					else
					{
						sprintf (buf, "Very far to the %s, you see $n arrive from %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], rev_d2[qe->dir]);
						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						target_sighted (tch, ch);
					}

					// We also now see if we've got an overwatch ready and waiting.
					check_overwatch(tch, ch, false);
				}
			}
		}

	}

	if (watched)
	{
		ch = tmp_ch;
		watched = 0;
	}

	if (is_mounted (ch))
	{
		do_look (ch->mount, "", 77);
		/*if (IS_SET (ch->plr_flags, QUIET_SCAN))
		do_scan (ch, "", 1);*/
	}

	if (IS_SUBDUER (ch))
	{
		do_look (ch->subdue, "", 77);
		/*if (IS_SET (ch->plr_flags, QUIET_SCAN))
		do_scan (ch, "", 1);*/
	}

	qe->flags |= MF_ARRIVAL;
	ch->flags |= FLAG_ENTERING;
	qe->event_time = qe->arrive_time;
	qe->from_room = ch->room;

	if (qe->event_time <= 0)
	{
		if (get_second_affect(ch, SA_GFOLLOW, NULL))
		{
			;
		}
		else if (get_second_affect(ch, SA_LEAD, NULL) && !ch->following)
		{
			sprintf (buf, "$n arives from %s, #5a %s#0 following behind.", rev_d2[qe->dir], group_str);
			act (buf, true, ch, 0, drag_af ? (CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT | _ACT_FORMAT);
		}
		else
		{
			sprintf (buf, "$n arrives from %s.", rev_d2[qe->dir]);
			act (buf, true, ch, 0, drag_af ? (CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT | _ACT_FORMAT);
		}
		//sprintf (buf, "$n arrives from %s.", rev_d2[qe->dir]);
		//act (buf, true, ch, 0, 0, TO_ROOM);
		enter_room (qe);
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (is_area_enforcer (tch) && enforcer (tch, ch, 1, 0) > 0)
			{
				continue;
			}
		}
	}
	else
	{
		if (is_mounted (ch) && IS_RIDEE (ch))
		{

			/* ch->mount is the rider
			sprintf (buf, "$n %ss $N in from %s.",
			mount_speeds[ch->mount->pc ? ch->mount->pc->mount_speed :
			ch->speed], rev_d2[qe->dir]);
			act (buf, false, ch->mount, 0, ch, TO_NOTVICT);
			*/

			if (get_second_affect(ch, SA_GFOLLOW, NULL)
				|| get_second_affect(ch->mount, SA_GFOLLOW, NULL))
			{
				;
			}
			else if (get_second_affect(ch, SA_LEAD, NULL) || get_second_affect(ch->mount, SA_LEAD, NULL))
			{
				sprintf (buf, "$n %ss $N in from %s, #5a %s#0 following behind.",
					mount_speeds[ch->mount->pc ? ch->mount->pc->mount_speed :
					ch->speed], rev_d2[qe->dir], group_str);
				act (buf, true, ch->mount, 0, ch, TO_NOTVICT | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "$n %ss $N in from %s.",
					mount_speeds[ch->mount->pc ? ch->mount->pc->mount_speed :
					ch->speed], rev_d2[qe->dir]);
				act (buf, false, ch->mount, 0, ch, TO_NOTVICT);
			}


		}

		else
		{
			// if sneaking and the room is nohide, or they fail the skillcheck remove the sneak affect
			if (IS_SET (qe->flags, MF_SNEAK)
				&& (IS_SET (ch->room->room_flags, NOHIDE)
				|| (would_reveal (ch)
				&& odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) <
				number (1, 100))))
			{

				observed = 0;
				for (tmp_ch = ch->room->people; tmp_ch;
					tmp_ch = tmp_ch->next_in_room)
				{
					if (tmp_ch != ch && CAN_SEE (ch, tmp_ch)
						&& !are_grouped (tmp_ch, ch) && !skip_sneak_check(tmp_ch))
					{
						observed = 1;
						break;
					}
				}
				if (observed)
					send_to_char ("You are observed.\n", ch);
				remove_affect_type (ch, MAGIC_HIDDEN);
				remove_affect_type (ch, MAGIC_SNEAK);
				qe->flags &= ~MF_SNEAK;
			}

			if (!IS_SET (qe->flags, MF_SNEAK))
			{
				char *move_names[] =
				{
					"walks", "trudges", "paces",
					"jogs", "runs", "sprints", "blasts",
					"swims", "crawls", "floats",
				};

				if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_FLY)
				{
					move_names[qe->speed_type] = "flies";
				}
				else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_SLITHER)
				{
					move_names[qe->speed_type] = "slithers";
				}
				else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_SCUTTLE)
				{
					move_names[qe->speed_type] = "scuttles";
				}
				else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_DRIFT)
				{
					move_names[qe->speed_type] = "drifts";
				}
				else if (lookup_race_int(ch->race, RACE_BOT_BITS))
				{
					move_names[qe->speed_type] = "traveling";
				}

				if (!IS_SET (ch->act, ACT_FLYING))
				{
					if (get_second_affect(ch, SA_GFOLLOW, NULL))
					{
						;
					}
					else if (get_second_affect(ch, SA_LEAD, NULL) && !ch->following)
					{
						sprintf (buf, "$n %s in from %s%s, #5a %s#0 following behind.", move_names[qe->speed_type], rev_d2[qe->dir], (drag_af ? ", dragging $N" : travel_str), group_str);
						act (buf, true, ch, 0, drag_af ?
							(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT | _ACT_FORMAT);
					}
					else
					{
						sprintf (buf, "$n %s in from %s%s.", move_names[qe->speed_type], rev_d2[qe->dir], (drag_af ? ", dragging $N" : travel_str));
						act (buf, true, ch, 0, drag_af ?
							(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT | _ACT_FORMAT);
					}
				}
				else
				{
					sprintf (buf, "$n flies in from %s%s.", rev_d2[qe->dir],
						drag_af ? ", dragging $N" : "");
					act (buf, true, ch, 0, drag_af ?
						(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT | _ACT_FORMAT);
				}

				// We also now see if we've got an overwatch ready and waiting.
				for (tmp_ch = ch->room->people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
				{
					if (tmp_ch != ch && CAN_SEE(tmp_ch, ch))
						check_overwatch(tmp_ch, ch, false);
				}

			}
			else
			{
				sprintf (buf, "[%s sneaking in from %s.]", ch->tname,
					rev_d2[qe->dir]);
				act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
				sprintf (buf, "#5%s#0 sneaks into the area.", char_short (ch));
				buf[2] = toupper (buf[2]);
				act (buf, true, ch, 0, 0, TO_NOTVICT | TO_GROUP);
			}
		}

		add_to_qe_list (qe);
	}

	// If they've got equipment, and are in an outdoor room, then we apply dust to their
	// lower body.
	if (is_outdoors(ch->room) && ch->equip)
	{
		object__enviro(ch, NULL, COND_DUST, 2, HITLOC_FEET);
		object__enviro(ch, NULL, COND_DIRT, 1, HITLOC_FEET);
		object__enviro(ch, NULL, COND_DUST, 1, HITLOC_LOLEGS);
	}

	if (IS_SUBDUER (ch))
		act ("$n has $N in tow.", true, ch, 0, ch->subdue, TO_NOTVICT);

	if (!check_climb (ch))
	{

		clear_moves (ch);
		clear_current_move (ch);

		while (IS_SET (ch->room->room_flags, CLIMB) ||
			IS_SET (ch->room->room_flags, FALL))
		{

			room_exit = EXIT (ch, DOWN);

			/* Don't automatically fall through doors */
			if (!room_exit
				|| IS_SET (room_exit->exit_info, EX_ISDOOR)
				|| IS_SET (room_exit->exit_info, EX_ISGATE))
				break;

			rooms_fallen++;

			act ("$n plummets down!", true, ch, 0, 0, TO_ROOM);
			send_to_char ("\n\nYou plummet down!\n\n", ch);

			char_from_room (ch);
			char_to_room (ch, room_exit->to_room);

			do_look (ch, "", 77);
			//no qscan while falling
		}

		if (SWIM_ONLY (ch->room))
		{
			send_to_char ("You land with a splash!\n", ch);
			act ("$n lands with a splash!", true, ch, 0, 0, TO_ROOM);
			object__drench (ch, ch->equip, true);
		}
		else
		{
			send_to_char ("You land with a thud.\n", ch);
			act ("$n lands with a thud!", false, ch, 0, 0, TO_ROOM);
		}

		for (i = 1; i <= number (1, 5) * rooms_fallen; i++)
		{
			died +=
				wound_to_char (ch, figure_location (ch, HITLOC_LOLEGS),
				rooms_fallen * number (1, 5), 3, 0, 0, 0);
			if (died)
				break;
		}

		if (!died && !SWIM_ONLY (ch->room))
			knock_out (ch, 15);

		if (drag_af)
		{
			victim = (CHAR_DATA *) drag_af->a.spell.t;

			if (victim && victim->in_room)
			{
				char_from_room (victim);
				char_to_room (victim, room_exit->to_room);
			}

			victim = NULL;
		}
	}

	else if (ch->following)
		follower_catchup (ch);

	if (ch->aiming_at)
	{
		send_to_char ("You lose your aim as you move.\n", ch);
		remove_targeted(ch->aiming_at, ch);
		ch->aiming_at = NULL;
		ch->aim = 0;
	}

	if (ch->in_room && !ch->room)
	{
		ch->room = vnum_to_room(ch->in_room);
	}

	// We'll slap on an enviro 33% of the time, to make it a little bit
	// uneven as to how it's applied.
	if (ch->room && ch->room->enviro_type && ch->room->enviro_power)
	{
		if (!number(0,2))
			object__enviro(ch, NULL, ch->room->enviro_type, ch->room->enviro_power / 2, HITLOC_NONE);
	}


	if (get_affect (ch, AFFECT_LISTEN_DIR))
	{
		send_to_char ("You cease eavesdropping as you move.\n", ch);
		remove_affect_type (ch, AFFECT_LISTEN_DIR);
	}

	// Traps:
	for (prog_obj = ch->room->contents; prog_obj; prog_obj = prog_obj->next_content)
	{
		// If an item is trapped...
		if (!IS_SET(qe->flags, MF_TOEDGE) && (GET_ITEM_TYPE(prog_obj) == ITEM_TRAP) && IS_MORTAL(ch))
		{
			// ... and the direction set equals the direction the person is entering from...
			if ((prog_obj->o.od.value[1] == rev_dir[qe->dir]) && ((prog_obj->o.od.value[0] >= 1) || (prog_obj->o.od.value[0] <= -1)))
			{

				if (is_with_group(ch) && !ch->following)
				{
					ch->delay_type = DEL_TRAPPED;
					ch->delay_obj = prog_obj;
					ch->delay = 0;
				}
				else if (!is_with_group(ch))
				{
					ch->delay_type = DEL_TRAPPED;
					ch->delay_obj = prog_obj;
					ch->delay = 0;
				}
			}
		}
	}

}

void
	process_quarter_events (void)
{
	QE_DATA *qe;

	for (qe = quarter_event_list; qe; qe = qe->next)
		qe->event_time--;

	while (quarter_event_list && quarter_event_list->event_time <= 0)
	{

		qe = quarter_event_list;

		quarter_event_list = qe->next;

		enter_room (qe);
	}
}

void
	exit_room (CHAR_DATA * ch, int dir, int flags, int leave_time,
	int arrive_time, int speed_name, int needed_movement,
	char *travel_str, char *group_str)
{
	CHAR_DATA *mount;
	CHAR_DATA *rider;
	QE_DATA *qe;
	OBJ_DATA *prog_obj;


	char buf[AVG_STRING_LENGTH];
	char *speed_type_names[] =
	{
		"walking", "wandering", "slowly walking", "jogging",
		"running", "sprinting", "swimming", "crawling", "floating",
	};
	char buf1[AVG_STRING_LENGTH];
	/*
	char		*mount_noun_speeds [] = {
	"trot", "trudge", "slowly trot", "quickly trot",
	"gallop", "quickly gallop" };
	*/

	ch->flags |= FLAG_LEAVING;

	CREATE (qe, QE_DATA, 1);

	qe->ch = ch;
	qe->dir = dir;
	qe->speed_type = speed_name;
	qe->flags = flags;
	qe->from_room = ch->room;
	qe->event_time = leave_time;
	qe->arrive_time = arrive_time;
	qe->move_cost = needed_movement;
	qe->next = NULL;
	qe->travel_str = travel_str;
	qe->group_str = group_str;

	if (ch->speed == SPEED_IMMORTAL)
	{
		sprintf (buf1, "$n leaves in a flash %s.", dirs[dir]);
		if (dir != OUTSIDE && dir != INSIDE)
			sprintf (buf, "You blast %sward.\n", dirs[dir]);
		else
			sprintf (buf, "You blast %s.\n", dirs[dir]);
		send_to_char (buf, ch);
		act (buf1, true, ch, 0, 0, TO_ROOM);
	}
	else
	{

		if (IS_SUBDUER (ch))
		{
			sprintf (buf, "$n starts %s %s, dragging $N along.",
				speed_type_names[ch->speed], dirs[dir]);
			act (buf, true, ch, 0, ch->subdue, TO_NOTVICT);
		}

		if (is_mounted (ch) && IS_RIDEE (ch))
		{

			mount = ch;
			rider = ch->mount;

			sprintf (buf, "You make $N %s %s.",
				mount_speeds[rider->pc ? rider->pc->mount_speed :
				mount->speed], dirs[dir]);
			act (buf, false, rider, 0, mount, TO_CHAR);

			if (do_group_size(rider) > 5)
			{
				if (!rider->following)
				{
					sprintf (buf, "$n starts %s $N %s, #5a %s#0 following behind $m.",
						mount_speeds_ing[rider->pc ? rider->pc->mount_speed :
						mount->speed], dirs[dir], group_str);
					act (buf, true, rider, 0, mount, TO_ROOM | _ACT_FORMAT);
					sprintf (buf, "#5A %s#0 follows in your wake.\n", group_str);
					send_to_char(buf, rider);

					add_second_affect (SA_LEAD, leave_time, rider, NULL, NULL, 0);
				}
				else
				{
					add_second_affect (SA_GFOLLOW, leave_time, rider, NULL, NULL, 0);
					add_second_affect (SA_GFOLLOW, leave_time, mount, NULL, NULL, 0);
				}
			}
			else
			{
				sprintf (buf, "$n starts %s $N %s.",
					mount_speeds_ing[rider->pc ? rider->pc->mount_speed :
					mount->speed], dirs[dir]);
				act (buf, false, rider, 0, mount, TO_NOTVICT);
			}
		}

		if (IS_SET (flags, MF_SNEAK))
		{
			sprintf (buf, "[%s starts sneaking %s.]", ch->tname, dirs[dir]);
			act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
			if (dir != OUTSIDE && dir != INSIDE)
				sprintf (buf, "#5%s#0 begins sneaking %sward.", char_short (ch),
				dirs[dir]);
			else
				sprintf (buf, "#5%s#0 begins sneaking %s.", char_short (ch),
				dirs[dir]);
			buf[2] = toupper (buf[2]);
			act (buf, true, ch, 0, 0, TO_NOTVICT | TO_GROUP);
		}
	}

	// Traps:

	for (prog_obj = ch->room->contents; prog_obj; prog_obj = prog_obj->next_content)
	{
		// If an item is trapped, they're not immortal and they're not walking to the edge of the room.
		if (!IS_SET(qe->flags, MF_TOEDGE) && (GET_ITEM_TYPE(prog_obj) == ITEM_TRAP) && IS_MORTAL(ch))
		{
			// ... and the direction set equals the direction the person is leaving to...
			if ((prog_obj->o.od.value[1] == qe->dir) && ((prog_obj->o.od.value[0] >= 1) || (prog_obj->o.od.value[0] <= -1)))
			{
				if (trap_enact(ch, prog_obj, 2, NULL) != 2)
				{
					clear_moves (ch);
					clear_current_move(ch);
					return;
				}
			}
		}
	}

	if (qe->event_time <= 0)
	{
		enter_room (qe);
		return;
	}

	add_to_qe_list (qe);

	hitches_follow (ch, dir, leave_time, arrive_time);

	if (IS_RIDEE(ch))
		followers_follow (ch->mount, dir, leave_time, arrive_time);
	else
		followers_follow (ch, dir, leave_time, arrive_time);
}

int
	calc_movement_charge (CHAR_DATA * ch, int dir, int wanted_time, int flags,
	int *speed, int *speed_name, float *walk_time)
{
	char buf[MAX_STRING_LENGTH];
	int needed_movement = 0;
	float cost_modifier = 1.00;
	float t;
	float work = 1.00;
	int i;
	int strchk;
	AFFECTED_TYPE *dragger;
	ROOM_DATA *target_room;
	ROOM_DIRECTION_DATA *room_exit;
	int swimability = ((ch->str + ch->agi) * 2) - 20 + number(0, 40);

	room_exit = EXIT (ch, dir);

	if (!room_exit || !(target_room = vnum_to_room (room_exit->to_room)))
		return 0;

	if (dir == UP &&
		IS_SET (target_room->room_flags, FALL) &&
		!get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
		return 0;

	if (dir == UP &&
		IS_SET (target_room->room_flags, CLIMB) &&
		IS_ENCUMBERED (ch) &&
		!get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
		return 0;

	/* TOEDGE means were in the room and just want to get to the edge. */

	if (IS_SET (flags, MF_TOEDGE))
		needed_movement = movement_loss[ch->room->sector_type] / 2;

	/* TONEXT_EDGE means we're about to walk into the room and cross it
	completely to the other edge. */

	else if (IS_SET (flags, MF_TONEXT_EDGE))
		needed_movement = movement_loss[target_room->sector_type];

	/* Otherwise, we're just walking from midpoint to midpoint */

	else
		needed_movement = (movement_loss[ch->room->sector_type] +
		movement_loss[target_room->sector_type]) / 2;

	if ((dragger = get_affect (ch, MAGIC_DRAGGER)))
		cost_modifier *= 1.50;
	else if (IS_SUBDUER (ch))
		cost_modifier *= 1.50;

	for (i = 0; i < ENCUMBERANCE_ENTRIES; i++)
	{
	//	if (GET_STR (ch) >= 20)
	//		strchk = 20;
		
			strchk = GET_STR (ch);

		if (strchk * enc_tab[i].str_mult_wt >= IS_CARRYING_W (ch))
			break;
	}

	if (i >= 5 && !IS_FLOATING(ch))
	{
		printf ("Huh?  %f\n",
			(float) (CAN_CARRY_W (ch) - IS_CARRYING_W (ch)) /
			(float) CAN_CARRY_W (ch));
		printf ("Very encumbered pc %s in room %d\n", ch->name, ch->in_room);
		sprintf (buf, "You are carrying too much to move.\n");
		send_to_char (buf, ch);
		clear_moves (ch);
		return -1;
	}

	/* 4 pulses per second.  5 seconds/room for a 13 agi N/PC */

	*walk_time = 0;

	*walk_time = 2 * 13.0 * 5.0 / (GET_AGI (ch) ? GET_AGI (ch) : 13);

	*walk_time += terrain_walk_time[ch->room->sector_type];

	// People walking around with items unslung get more penalty to their move times - you don't want to rush with a 4 foot sword drawn.


	if ((get_equip(ch, WEAR_PRIM) && get_equip(ch, WEAR_SEC)) ||
		(get_equip(ch, WEAR_PRIM) && get_equip(ch, WEAR_SHIELD)) ||
		(get_equip(ch, WEAR_SEC) && get_equip(ch, WEAR_SHIELD)) ||
		get_equip(ch, WEAR_BOTH))
	{
		*walk_time += 3;
	}
	else if (get_equip(ch, WEAR_PRIM) || get_equip(ch, WEAR_SEC) || get_equip(ch, WEAR_SHIELD))
	{
		*walk_time += 2;
	}

	// Takes extra time and extra stamina to walk through big rooms,
	// less time and less stamina to walk through small rooms.
	if (IS_SET(ch->room->room_flags, BIG_ROOM))
	{
		*walk_time += 3;
		needed_movement += 2;
	}
	else if (IS_SET(ch->room->room_flags, SMALL_ROOM))
	{
		*walk_time -= 1;
		needed_movement -= 1;
	}

	*walk_time += enc_tab[i].delay;
	needed_movement += enc_tab[i].delay;

	if (wanted_time <= 0)
		wanted_time = (int) round (*walk_time * move_speeds[ch->speed]);

	if (*walk_time <= 0)
		*walk_time = 9.0;

	if (get_affect (ch, MAGIC_AFFECT_SLOW))
		*walk_time *= 1.5;

	if (get_affect (ch, MAGIC_AFFECT_SPEED))
		*walk_time *= 0.5;

	if ((SWIM_ONLY (ch->room) || SWIM_ONLY (target_room))
		&& !IS_SET (ch->act, ACT_FLYING))
	{
		*walk_time *= (1.45 - ((float) swimability / 100));
		cost_modifier *= (1.45 - ((float) swimability / 100));
	}

	t = wanted_time / *walk_time;

	if (t < 2.50 && i >= 5)
		t = 2.50;
	else if (t < 1.60 && i >= 4)
		t = 1.60;
	else if (t < 1.00 && i >= 3)
		t = 1.00;
	else if (t < 0.66 && i >= 2)
		t = 0.66;
	else if (t < 0.50 && i >= 1)
		t = 0.50;
	else if (t < 0.33)
		t = 0.33;

	if (GET_POS(ch) == POSITION_SITTING)
	{
		*speed_name = SPEED_CREEP;
		t = 5.0000;
	}

	else if (t > 1.60001)
		*speed_name = SPEED_CRAWL;
	else if (t > 1.00001)
		*speed_name = SPEED_PACED;
	else if (t > 0.66001)
		*speed_name = SPEED_WALK;
	else if (t > 0.50001)
		*speed_name = SPEED_JOG;
	else if (t > 0.33001)
		*speed_name = SPEED_RUN;
	else
		*speed_name = SPEED_SPRINT;

	if (IS_SWIMMING (ch) && !IS_SET (ch->act, ACT_FLYING))
	{
		t = 1.60;
		*speed_name = SPEED_SWIM;
	}

	if (IS_FLOATING (ch))
	{
		*speed_name = SPEED_FLOAT;
		t = 3.00;	
	}

	*speed = (int) round (*walk_time * t);

	float move_t = 1.0;
	if (t > 1)
	{
		move_t =  1 + ((t - 1)/2);
	}
	else if (t < 1)
	{
		move_t = 1 - ((1 - t)/2);
	}

	cost_modifier = cost_modifier / move_t;

	if (i <= 1)
		i = 1;
	else if (i <= 2)
		cost_modifier = cost_modifier * 1.15;
	else if (i <= 3)
		cost_modifier = cost_modifier * 1.25;
	else if (i <= 5)
		cost_modifier = cost_modifier * 1.50;

	if (GET_TRUST (ch) && ch->speed == SPEED_IMMORTAL)
	{
		*speed_name = SPEED_IMMORTAL;
		needed_movement = 0;
		*speed = 0;
	}

	if (IS_RIDER (ch))
		needed_movement = 0;

	if (IS_SET (ch->act, ACT_MOUNT))
		needed_movement = (int) round (((float) needed_movement) * 0.33);

	if (*speed < 0)
		*speed = 1;

	work = round(needed_movement * cost_modifier);
	needed_movement = int (work);

	//sprintf(buf, "Needed_Movement: %d, Speed: %d", needed_movement, *speed);
	//send_to_gods(buf);

	needed_movement = MAX(needed_movement, 1);

	return (needed_movement);
}

void
	initiate_move (CHAR_DATA * ch)
{
	int dir;
	int eq_penalty;
	int flags;
	int needed_movement;
	int exit_speed;		/* Actually, time */
	int enter_speed;		/* Actually, time */
	float walk_time;
	int speed;
	int wanted_time;
	int speed_name;
	OBJ_DATA *obj;
	unsigned char bIsMaskHolder = 0;
	CHAR_DATA *tch, *pch = NULL;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	//OBJ_DATA *bow;
	ROOM_DATA *target_room;
	ROOM_DIRECTION_DATA *room_exit;
	LODGED_OBJECT_INFO *lodged;
	SECOND_AFFECT *sa;
	char location[MAX_STRING_LENGTH];
	int stuck = 0;
	char buf[MAX_STRING_LENGTH], suffix[25];
	char *move_names[] =
	{
		"walking", "wandering", "slowly walking",
		"jogging", "running", "sprinting", "blasting",
		"swimming", "crawling", "floating"
	};
	char *move_names2[] =
	{
		"walk", "wander", "pace",
		"jog", "run", "sprint", "blast",
		"swim", "crawl", "float"
	};
	char buf1[MAX_STRING_LENGTH];
	char msg[AVG_STRING_LENGTH];
	char travel_str[MAX_STRING_LENGTH] = "";
	char group_str[MAX_STRING_LENGTH] = "";


	if (!ch->moves)
	{
		printf ("Nothing to initiate!\n");
		return;
	}

	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		move_names[ch->speed] = "traveling";
		move_names2[ch->speed] = "travel";
	}
	else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_FLY)
	{
		move_names[ch->speed] = "flying";
		move_names2[ch->speed] = "flie";
	}
	else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_SLITHER)
	{
		move_names[ch->speed] = "slithering";
		move_names2[ch->speed] = "slither";
	}
	else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_SCUTTLE)
	{
		move_names[ch->speed] = "scuttling";
		move_names2[ch->speed] = "scuttle";
	}
	else if (lookup_race_int(ch->race, RACE_MOVEMENT) == RACE_MOVEMENT_DRIFT)
	{
		move_names[ch->speed] = "drifting";
		move_names2[ch->speed] = "drift";
	}
	else if (lookup_race_int(ch->race, RACE_BOT_BITS))
	{
		move_names[ch->speed] = "traveling";
		move_names2[ch->speed] = "travel";
	}


	if (IS_HITCHEE (ch))
	{

		if (!GET_FLAG (ch->hitcher, FLAG_LEAVING))
		{

			if (IS_RIDEE (ch))
				act ("Your mount is hitched to $N.",
				true, ch->mount, 0, ch->hitcher, TO_CHAR);

			act ("You can't move while hitched to $N.",
				true, ch, 0, ch->hitcher, TO_CHAR);

			clear_moves (ch);
			return;
		}
	}

	if (ch->fighting)
	{

		dir = ch->moves->dir;

		/* A "PURSUING" mob will follow from a fight after one second */

		/* Non-sentinel aggro/enforcer mobs now follow by default, if morale check is made. */

		if (((IS_SET (ch->act, ACT_AGGRESSIVE)
			|| (IS_SET (ch->act, ACT_ENFORCER)
			&& !IS_SET (ch->act, ACT_SENTINEL)))) && ch->following
			&& is_he_here (ch, ch->following, true)
			&& GET_FLAG (ch->following, FLAG_FLEE))
		{

			stop_fighting (ch);

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
				if (tch->fighting == ch && tch != ch && GET_HIT (tch) > 0)
				{
					set_fighting (ch, tch);
					return;
				}

				add_second_affect (SA_MOVE, 1, ch, NULL, NULL, dir);
		}

		return;
	}

	move = ch->moves;
	ch->moves = move->next;

	dir = move->dir;
	flags = move->flags;
	wanted_time = move->desired_time;

	if (GET_POS (ch) != STAND && !IS_SET (flags, MF_CRAWL))
	{
		send_to_char ("You'll need to stand in order to move.\n", ch);
		clear_moves (ch);
		return;
	}

	if (move->travel_str)
	{
		sprintf (travel_str, ", %s", move->travel_str);
		mem_free (move->travel_str);
	}

	if (move->group_str)
	{
		sprintf (group_str, ", %s", move->group_str);
		mem_free (move->group_str);
	}

	mem_free (move); // MOVE_DATA*

	room_exit = EXIT (ch, dir);

	if (!room_exit || !(target_room = vnum_to_room (room_exit->to_room)) ||
		(IS_SET (room_exit->exit_info, EX_CLOSED) && ch->room->secrets[dir]
	&& IS_MORTAL (ch)))
	{

		if (ch->room->extra && ch->room->extra->alas[dir])
			send_to_char (ch->room->extra->alas[dir], ch);
		else
			send_to_char ("Alas, you cannot go that way...\n", ch);

		clear_moves (ch);
		return;
	}

	if (dir == UP
		&& !get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
	{

		if (IS_SET (target_room->room_flags, FALL))
		{

			send_to_char ("Too steep.  You can't climb up.\n", ch);
			clear_moves (ch);
			return;
		}

		if (IS_SET (target_room->room_flags, CLIMB))
		{

			if (IS_ENCUMBERED (ch))
			{
				send_to_char ("You're too encumbered to climb up.\n", ch);
				clear_moves (ch);
				return;
			}
			else if ((ch->left_hand && ch->right_hand)
				|| get_equip (ch, WEAR_BOTH))
			{
				send_to_char ("You'll need to free your hands to climb up.\n",
					ch);
				clear_moves (ch);
				return;
			}
		}
	}

	if (!(target_room->capacity == 0))
	{
		if ((sa = get_second_affect(ch, SA_PUSH, NULL))
			&& sa->info2 == 1)
		{
			remove_second_affect(sa);
		}
		else if (!room_avail(target_room, ch)
			&& !(GET_TRUST(ch))
			&& !(IS_SET(ch->act, ACT_SQUEEZER))
			&& !(IS_SET(target_room->room_flags, FORT)))
		{
			send_to_char("\nThere isn't enough room for you -- try pushing your way in instead.\n", ch);
			clear_moves (ch);
			return;
		}
	}

	/*if (IS_SET (ch->act, ACT_MOUNT) &&
		IS_SET (target_room->room_flags, NO_MOUNT))
	{
		act ("You can't go there.", true, ch, 0, 0, TO_CHAR);

		if (IS_RIDEE (ch))
			act ("$N can't go there.", true, ch->mount, 0, ch, TO_CHAR);

		if (IS_HITCHEE (ch))
			act ("$N can't go there.", true, ch->hitcher, 0, ch, TO_CHAR);

		clear_moves (ch);
		return;
	}*/

	/*
	if(ch->formation && (
	target_room->sector_type == SECT_WOODS ||
	target_room->sector_type == SECT_FOREST ||
	target_room->sector_type == SECT_SWAMP ||
	target_room->sector_type == SECT_MOUNTAIN ||
	target_room->sector_type == SECT_RIVER ||
	target_room->sector_type == SECT_LAKE ||
	target_room->sector_type == SECT_UNDERWATER))
	{
	send_to_char("You cannot march your formed followers in to such terrain; you will have to break from formation first.\n", ch);
	clear_moves(ch);
	return;
	}
	*/

	if (nPartyRoom
		&& ch->in_room == nPartyRoom
		&& !is_clan_member (ch, ptrPartyClan->name))
	{

		if (IS_MORTAL (ch) && get_affect (ch, MAGIC_HIDDEN))
		{
			remove_affect_type (ch, MAGIC_HIDDEN);
			act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM);
		}

		for (pch = ch->room->people; pch; pch = pch->next_in_room)
			if (IS_NPC (pch) && is_clan_member (pch, ptrPartyClan->name))
				break;

		name_to_ident (ch, buf1);

		if (dir == nPartyDir)
		{

			if ((obj = get_equip (ch, WEAR_FACE))
				&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
			{
				strcpy (msg, "Please enjoy your evening.");
				if (pch)
				{
					if (!ch->following)
					{
						sprintf (buf, "%s (with a polite wave) %s", buf1, msg);
						do_tell (pch, buf, 0);
					}
				}
				else
				{
					sprintf (buf,
						"#5A doorman#0 tells you in Westron, with a polite wave,\n   \"%s\"\n",
						msg);
					act (buf, false, ch, 0, 0, TO_CHAR);
				}
			}
			else
			{
				sprintf (msg,
					"Pardon me, but I'm afraid you must wear a mask to enter our estate this evening. We'd be happy to loan you a mask for the evening for a deposit of ten bits.");

				if (pch)
				{
					if (!ch->following)
					{
						sprintf (buf, "%s (stepping forward) %s", buf1, msg);
						do_tell (pch, buf, 0);
					}
				}
				else
				{
					sprintf (buf,
						"#5A doorman#0 tells you in Westron, stepping forward,\n   \"%s\"\n",
						msg);
					act (buf, false, ch, 0, 0, TO_CHAR);

				}

				clear_moves (ch);
				return;
			}

		}
		else
		{

			for (obj = object_list; obj; obj = obj->next)
			{
				if (obj->nVirtual != 3078)
				{
					continue;
				}
				if (obj->equiped_by)
				{
					if (obj->equiped_by == ch)
					{
						bIsMaskHolder = 1;
						break;
					}
					else
					{
						continue;
					}
				}
				if (obj->carried_by)
				{
					if (obj->carried_by == ch)
					{
						bIsMaskHolder = 1;
						break;
					}
					else
					{
						continue;
					}
				}
				if (obj->in_obj)
				{
					if (obj->in_obj->carried_by == ch
						|| obj->in_obj->equiped_by == ch)
					{
						bIsMaskHolder = 1;
					}
					else if (obj->in_obj->in_obj)
					{
						if (obj->in_obj->in_obj->carried_by == ch
							|| obj->in_obj->in_obj->equiped_by == ch)
						{
							bIsMaskHolder = 1;
						}
						else if (obj->in_obj->in_obj->in_obj
							&& (obj->in_obj->in_obj->in_obj->carried_by ==
							ch
							|| obj->in_obj->in_obj->in_obj->
							equiped_by == ch))
						{
							bIsMaskHolder = 1;
						}
					}
				}
			}

			if (bIsMaskHolder)
			{
				sprintf (msg,
					"Pardon me, if you'd be so kind as to return the mask, I shall return your deposit to you. I'm terribly sorry, but we can't let you leave with it.");
				if (pch)
				{
					if (!ch->following)
					{
						sprintf (buf, "%s (stepping forward) %s", buf1, msg);
						do_tell (pch, buf, 0);
					}
				}
				else
				{
					sprintf (buf,
						"#5A doorman#0 tells you in Westron, stepping forward,\n   \"%s\"",
						msg);
					act (buf, false, ch, 0, 0, TO_CHAR);
				}
				clear_moves (ch);
				return;
			}
			else
			{
				sprintf (msg, "Thank you for attending.");
				if (pch)
				{
					if (!ch->following)
					{
						sprintf (buf, "%s (waving politely) %s", buf1, msg);
						do_tell (pch, buf, 0);
					}
				}
				else
				{
					sprintf (buf,
						"#5A doorman#0 tells you in Westron,\n   \"%s\"",
						msg);
					act (buf, false, ch, 0, 0, TO_CHAR);
				}
			}
		}
	}

	if (isguarded (ch->room, dir) && (IS_MORTAL (ch) || IS_NPC (ch)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (!CAN_SEE (tch, ch))
				continue;
			if (IS_SET (ch->act, ACT_FLYING) && !IS_SET (tch->act, ACT_FLYING))
				continue;
			if (IS_SET (ch->act, ACT_PREY))
				continue;
			if ((af = get_affect (tch, AFFECT_GUARD_DIR)))
			{
				if (af->a.shadow.edge == dir)
				{
					if (get_affect (tch, MAGIC_HIDDEN))
					{
						remove_affect_type (tch, MAGIC_HIDDEN);
						act
							("$N emerges from hiding and moves to block your egress in that direction.",
							false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
						act
							("$n attempts to move past you, but you emerge from hiding and intercept $m.", false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else
					{
						act ("$N moves to block your egress in that direction.",
							false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
						act
							("$n attempts to move past you, but you intercept $m.",
							false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					clear_moves (ch);
					return;
				}
			}
		}
	}

	if (IS_MORTAL (ch)
		&& (!IS_SET (flags, MF_SWIM) && SWIM_ONLY (vnum_to_room (room_exit->to_room))
		&&  !IS_SWIMMING (ch)) && !IS_SET (ch->act, ACT_FLYING)
		&&  !IS_SET(ch->flags, FLAG_FLEE))
	{
		send_to_char ("You'll have to swim for it.\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_SET (flags, MF_SWIM) &&
		!IS_SWIMMING (ch) && !SWIM_ONLY (vnum_to_room (room_exit->to_room)))
	{
		send_to_char ("You can't swim there.\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_MORTAL (ch)
		&& (!IS_SET (flags, MF_FLOAT) && FLOAT_ONLY (vnum_to_room (room_exit->to_room))
		&&  !IS_FLOATING (ch)) && !IS_SET (ch->act, ACT_FLYING)
		&&  !IS_SET(ch->flags, FLAG_FLEE))
	{
		send_to_char ("You'll have to float.\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_SET (flags, MF_FLOAT) &&
		!IS_FLOATING (ch) && !FLOAT_ONLY (vnum_to_room (room_exit->to_room)))
	{
		send_to_char ("You can't float there..\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_SET (flags, MF_FLOAT) || FLOAT_ONLY (target_room))
	{
		if (IS_RIDER (ch))
		{
			send_to_char ("Dismount first.\n", ch);
			return;
		}
		else if (IS_RIDEE (ch))
		{
			send_to_char ("Your rider must dismount first.\n", ch);
			return;
		}
		move_names[ch->speed] = "floating";
		move_names2[ch->speed] = "float";
	}

	if (IS_SET (flags, MF_SWIM) || SWIM_ONLY (target_room))
	{
		if (IS_RIDER (ch))
		{
			send_to_char ("Dismount first.\n", ch);
			return;
		}
		else if (IS_RIDEE (ch))
		{
			send_to_char ("Your rider must dismount first.\n", ch);
			return;
		}
		move_names[ch->speed] = "swimming";
		move_names2[ch->speed] = "swim";
	}

	if (IS_SET (flags, MF_CRAWL))
	{
		if (IS_RIDER (ch))
		{
			send_to_char ("Dismount first.\n", ch);
			return;
		}
		else if (IS_RIDEE (ch))
		{
			send_to_char ("Your rider must dismount first.\n", ch);
			return;
		}
		move_names[ch->speed] = "crawling";
		move_names2[ch->speed] = "crawl";

		if (is_outdoors(ch->room))
		{
			object__enviro(ch, NULL, COND_DIRT, 5, HITLOC_NONE);
			object__enviro(ch, NULL, COND_DUST, 8, HITLOC_NONE);
		}

	}

	if (IS_SET (ch->act, ACT_MOUNT)
		&& IS_RIDEE(ch) && !ride_mount(ch, ch->mount, ch->room->sector_type, target_room->sector_type, dir))
	{
		clear_moves (ch);
		return;
	}

	needed_movement =
		calc_movement_charge (ch, dir, wanted_time, flags, &speed, &speed_name,
		&walk_time);

	// Move failed for some reason, i.e. too much encumbrance

	if (needed_movement == -1)
	{
		clear_moves (ch);
		return;
	}

	// People in armour find it harder to cross difficult terrain,
	// giving an advantage to lightly armoured skirmshers.

	eq_penalty = armor_penalty(ch);

	if (IS_MORTAL(ch) && eq_penalty
		&& ch->room->sector_type != SECT_INSIDE
		/*&& ch->room->sector_type != SECT_CITY
		&& ch->room->sector_type != SECT_ROAD*/)
	{
		//if(ch->room->sector_type == SECT_TRAIL)
		//  eq_penalty = eq_penalty / 2;

		speed += eq_penalty;

		if (eq_penalty > 1)
			needed_movement += eq_penalty - 1;
	}

	// If you've got an arrow in your leg, you move slower
	// and suffer more from it.

	for (lodged = ch->lodged; lodged; lodged = lodged->next)
	{
		sprintf(location, "%s", lodged->location);
		stuck = index_lookup(leg_woundlocs, location);
	}

	if (stuck)
	{
		speed += 2;
		needed_movement += 1;
	}

	// Transporting creatures move a -lot- slower than others.

	if (IS_SET(ch->affected_by, AFF_TRANSPORTING))
	{
		speed += 5;
		speed = speed * 3;
	}

	if (GET_MOVE (ch) < needed_movement)
	{
		send_to_char ("You are too exhausted.\n", ch);
		clear_moves (ch);
		return;
	}

	// Can't walk with a loaded longbow.

	/*
	if((bow = get_equip (ch, WEAR_BOTH)) && bow->o.weapon.use_skill == SKILL_LONGBOW && bow->loaded)
	{
	command_interpreter(ch, "unload");
	}
	*/

	if ((speed + 1) / 2)
	{
		if (IS_SUBDUER (ch))
		{
			sprintf (buf, "You start %s %s, dragging $N.",
				move_names[speed_name], dirs[dir]);
			act (buf, true, ch, 0, ch->subdue, TO_CHAR);
			sprintf (buf, "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, true, ch->subdue, 0, ch, TO_CHAR);
		}
		if (IS_MOUNT (ch) && ch->mount && ch->mount->subdue)
		{
			sprintf (buf, "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, true, ch->subdue, 0, ch, TO_CHAR);
			act (buf, true, ch->mount->subdue, 0, ch->mount, TO_CHAR);
		}
	}

	if (IS_SET (flags, MF_TOEDGE))
	{
		exit_speed = speed;
		enter_speed = 0;
	}

	else if (IS_SET (flags, MF_TONEXT_EDGE))
	{
		exit_speed = 1;
		enter_speed = speed;
	}

	else
	{
		exit_speed = (speed + 1) / 2;
		enter_speed = speed / 2;
	}

	if (get_affect (ch, AFFECT_GUARD_DIR))
		remove_affect_type (ch, AFFECT_GUARD_DIR);

	if (get_affect (ch, MAGIC_WATCH))
		remove_affect_type (ch, MAGIC_WATCH);

	if (IS_SET (flags, MF_SNEAK))
	{
		if (dir != OUTSIDE && dir != INSIDE)
			sprintf (buf, "You cautiously begin sneaking %sward.\n", dirs[dir]);
		else
			sprintf (buf, "You cautiously begin sneaking %s.\n", dirs[dir]);
		send_to_char (buf, ch);
		exit_speed += 4;
		if (ch->speed > 2)
			ch->speed = 2;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if ((af = get_affect (tch, MAGIC_WATCH)))
			{
				if ((CHAR_DATA *) af->a.spell.t == ch)
				{
					sprintf(buf, "You spy $N beginning to sneak %s.", dirs[dir]);
					act(buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				}
			}
		}

	}


	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		move_names[ch->speed] = add_hash ("travelling");
		travel_str[0] = '\0';	/* override travel strings in vehicles */
	}
	else if ((strlen (travel_str) == 0) && (ch->travel_str != NULL))
	{
		sprintf (travel_str, ", %s", ch->travel_str);
	}

	if (dir == OUTSIDE || dir == INSIDE)
		*suffix = '\0';
	else
		sprintf (suffix, "ward");

	if (do_group_size(ch) > 5)
	{
		if (IS_RIDEE(ch) && ch->mount && !ch->mount->following)
			sprintf (group_str, group_makeup(ch->mount));
		else if (!ch->following)
			sprintf (group_str, group_makeup(ch));

		if (ch->formation)
			exit_speed += (do_group_size(ch) - 5);
		else
			exit_speed += ((do_group_size(ch) - 5) / 2);
	}
	else if (IS_RIDEE(ch) && ch->mount && do_group_size(ch->mount) > 5)
	{
		sprintf (group_str, group_makeup(ch->mount));
	}

	// Changed to account for moving in groups, to cut down the god-awful spam.

	if (((ch->room->sector_type == SECT_INSIDE || IS_SET(ch->room->room_flags, INDOORS)) && !IS_SET (flags, MF_SNEAK)
		&& ch->speed != SPEED_IMMORTAL) || (!IS_MORTAL (ch)
		&& ch->speed != SPEED_IMMORTAL
		&& !IS_SET (flags, MF_SNEAK)))
	{
		if (do_group_size(ch) > 5)
		{
			if (!ch->following)
			{
				sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
				act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

				sprintf (buf, "You begin %s %s%s%s, #5a %s#0 following behind.\n", move_names[ch->speed],
					dirs[dir], suffix, travel_str, group_str);
				send_to_char (buf, ch);

				add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
				sprintf (buf, "You begin %s %s%s%s.\n", move_names[ch->speed],
					dirs[dir], suffix, travel_str);
				send_to_char (buf, ch);

				add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
		}
		else
		{
			sprintf (buf, "You begin %s %s%s%s.\n", move_names[ch->speed],
				dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);

			sprintf (buf1, "$n begins %s %s%s%s.", move_names[ch->speed], dirs[dir],
				suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
	}

	if (ch->room->sector_type != SECT_INSIDE && !IS_SET(ch->room->room_flags, INDOORS)
		&& IS_MORTAL (ch)  && !IS_SET (flags, MF_SNEAK) && !IS_SUBDUER (ch)
		&& !is_mounted (ch) && !IS_RIDEE (ch))
	{

		switch (weather_info[ch->room->zone].state)
		{
			/*
			case LIGHT_RAIN:
			exit_speed += 2;
			needed_movement = (int) round (((float) needed_movement) * 1.2);

			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the light rain.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf, "You %s %s%s through the light rain%s, #5a %s#0 following behind.\n", move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf, "You %s %s%s through the light rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf1, "$n %ss %s%s through the light rain%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			sprintf (buf, "You %s %s%s through the light rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			}
			break;
			case STEADY_RAIN:
			exit_speed += 4;
			needed_movement = (int) round (((float) needed_movement) * 1.5);

			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the rain.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf, "You %s %s%s through the rain%s, #5a %s#0 following behind.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf, "You %s %s%s through the rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf1, "$n %ss %s%s through the rain%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			sprintf (buf, "You %s %s%s through the rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			}
			break;
			case HEAVY_RAIN:
			exit_speed += 6;
			needed_movement *= 2;

			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the lashing rain.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf,
			"You %s %s%s, struggling through the lashing rain%s, #5a %s#0 following behind.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf,
			"You %s %s%s, struggling through the lashing rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf,
			"You %s %s%s, struggling through the lashing rain%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			sprintf (buf1,
			"$n %ss %s%s, struggling through the lashing rain%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			}
			break;
			case LIGHT_SNOW:
			needed_movement = (int) round (((float) needed_movement) * 1.5);


			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the light snowfall.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf, "You %s %s%s through the light snowfall%s, #5a %s#0 following behind.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf, "You %s %s%s through the light snowfall%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf, "You %s %s%s through the light snowfall%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			sprintf (buf1, "$n %ss %s%s through the light snowfall%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			}
			break;
			case STEADY_SNOW:
			exit_speed += 8;
			needed_movement *= 2;


			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the steadily falling snow.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf, "You %s %s%s through the steadily falling snow%s, #5a %s#0 following behind.\n", move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf, "You %s %s%s through the steadily falling snow%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf, "You %s %s%s through the steadily falling snow%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			sprintf (buf1, "$n %ss %s%s through the steadily falling snow%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			}
			break;
			case HEAVY_SNOW:
			exit_speed += 12;
			needed_movement *= 4;

			if (do_group_size(ch) > 5)
			{
			if (is_group_leader(ch))
			{
			sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m through the shrieking snow.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
			act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf,
			"You %s %s%s, struggling through the shrieking snow%s, #5a %s#0 following behind.\n", move_names2[ch->speed], dirs[dir], suffix, travel_str, group_str);
			send_to_char (buf, ch);
			add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
			}
			else
			{
			sprintf (buf,
			"You %s %s%s, struggling through the shrieking snow%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
			}
			}
			else
			{
			sprintf (buf,
			"You %s %s%s, struggling through the shrieking snow%s.\n",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			sprintf (buf1,
			"$n %ss %s%s, struggling through the shrieking snow%s.",
			move_names2[ch->speed], dirs[dir], suffix, travel_str);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			}
			break;
			*/
		default:


			if (do_group_size(ch) > 5)
			{
				if (is_group_leader(ch))
				{
					sprintf (buf1, "$n begins %s %s%s%s, #5a %s#0 following behind $m.", move_names[ch->speed], dirs[dir], suffix, travel_str, group_str);
					act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
					sprintf (buf, "You begin %s %s%s%s, #5a %s#0 following behind.\n", move_names[ch->speed],
						dirs[dir], suffix, travel_str, group_str);
					send_to_char (buf, ch);
					add_second_affect (SA_LEAD, exit_speed, ch, NULL, NULL, 0);
				}
				else
				{
					sprintf (buf, "You begin %s %s%s%s.\n", move_names[ch->speed],
						dirs[dir], suffix, travel_str);
					send_to_char (buf, ch);
					add_second_affect (SA_GFOLLOW, exit_speed, ch, NULL, NULL, 0);
				}
			}
			else
			{
				sprintf (buf1, "$n begins %s %s%s%s.", move_names[ch->speed],
					dirs[dir], suffix, travel_str);
				act (buf1, true, ch, 0, 0, TO_ROOM);
				sprintf (buf, "You begin %s %s%s%s.\n", move_names[ch->speed],
					dirs[dir], suffix, travel_str);
				send_to_char (buf, ch);
			}
			break;
		}
	}

	if (stuck && number(1,20) > GET_WIL(ch))
	{
		sprintf(buf, "Pain shoots through your %s as you move.\n", expand_wound_loc(location));
		send_to_char(buf, ch);
	}

	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		sprintf (buf, "%s", char_short (ch));
		*buf = toupper (*buf);
		sprintf (buf1, "#5%s#0 begins %s %s%s.\n", buf, move_names[ch->speed],
			dirs[dir], suffix);
		send_to_room (buf1, ch->mob->vnum);
		needed_movement = 0;
	}

	if (ch->room->enviro_type && ch->room->enviro_power)
	{
		object__enviro(ch, NULL, ch->room->enviro_type, ch->room->enviro_power / 2, HITLOC_NONE);
	}

	exit_room (ch, dir, flags, exit_speed, enter_speed, speed_name,
		needed_movement,
		strlen (travel_str) ? str_dup (travel_str) : NULL,
		strlen (group_str) ? str_dup (group_str) : NULL);
}

int
	isguarded (ROOM_DATA * room, int dir)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;

	if (!room->people)
		return 0;

	int thresh = 1;
	int count = 0;

	if (IS_SET(room->room_flags, BIG_ROOM) ||
		IS_SET(room->room_flags, LAWFUL))
	{
		thresh = 3;
	}

	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		if ((af = get_affect (tch, AFFECT_GUARD_DIR)))
		{
			if (af->a.shadow.edge == dir)
			{
				count ++;
			}
		}
	}

	if (count >= thresh)
		return 1;
	else
		return 0;
}

void move( CHAR_DATA * ch, char * argument, int dir, int speed ) {
	MOVE_DATA *move;
	MOVE_DATA *tmove;
	QE_DATA *qe;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = "";
	int i = 0, j = 0;
	char *tmp = NULL;

	if (get_affect (ch, MAGIC_TOLL))
		stop_tolls (ch);

	if (*argument == '(')
	{
		tmp = (char *) alloc (strlen (argument), 31);
		sprintf (buf, "%s", argument);
		i = 1;
		j = 0;
		tmp[j] = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				send_to_char
					("Exactly how is it that you are trying to move?\n", ch);
				clear_moves (ch);
				return;
			}
			if (buf[i] == '*')
			{
				send_to_char
					("Unfortunately, the use of *object references is not allowed while moving.\n",
					ch);
				clear_moves (ch);
				return;
			}
			if (buf[i] == '~')
			{
				send_to_char
					("Unfortunately, the use of ~character references is not allowed while moving.\n",
					ch);
				clear_moves (ch);
				return;
			}
			tmp[j++] = buf[i++];
			tmp[j] = '\0';
		}
		buf[i] = '\0';
	}
	else
	{
		argument = one_argument (argument, buf);
	}

	CREATE (move, MOVE_DATA, 1);

	move->dir = dir;
	move->desired_time = speed;
	move->next = NULL;
	move->travel_str = tmp;
	move->group_str = NULL;

	if (get_affect (ch, MAGIC_SNEAK))
		move->flags = MF_SNEAK;
	else
	{
		move->flags = 0;
		if (!ch->mob || !IS_SET (ch->affected_by, AFF_SNEAK))
			remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if (!str_cmp (buf, "float"))
		move->flags |= MF_FLOAT;

	if (!str_cmp (buf, "swim"))
		move->flags |= MF_SWIM;

	if (!str_cmp (buf, "crawl"))
		move->flags |= MF_CRAWL;

	if (!IS_MORTAL (ch) && !str_cmp (buf, "!"))
		move->flags |= MF_PASSDOOR;

	if (!str_cmp (buf, "stand"))	/* Stand */
		move->flags = MF_TOEDGE;

	// If you're moving, we'll remove your cover as a precaution.
	remove_cover(ch, -2);

	if (!ch->moves)
	{

		for (qe = quarter_event_list; qe && qe->ch != ch; qe = qe->next)
			;

		if (qe && qe->dir == rev_dir[dir] && GET_FLAG (ch, FLAG_LEAVING))
		{

			send_to_char ("You turn around.\n", ch);
			act ("$n changes directions and returns.", true, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);

			qe->dir = dir;
			qe->event_time = qe->arrive_time;

			qe->flags |= MF_ARRIVAL;
			ch->flags &= ~FLAG_LEAVING;
			ch->flags |= FLAG_ENTERING;

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (!IS_NPC (tch))
					continue;
				if (tch->following != ch)
					continue;
				for (qe = quarter_event_list; qe && qe->ch && qe->ch != tch;
					qe = qe->next)
					;

				send_to_char ("You turn around.\n", tch);
				act ("$n changes directions and returns.", true, tch, 0, 0,
					TO_ROOM | _ACT_FORMAT);

				if (qe)
				{
					qe->dir = dir;
					qe->event_time = qe->arrive_time;

					qe->flags |= MF_ARRIVAL;
					tch->flags &= ~FLAG_LEAVING;
					tch->flags |= FLAG_ENTERING;
				}
			}

			return;
		}

		ch->moves = move;

		if (!GET_FLAG (ch, FLAG_LEAVING) && !GET_FLAG (ch, FLAG_ENTERING))
			initiate_move (ch);

		return;
	}

	for (tmove = ch->moves; tmove->next;)
		tmove = tmove->next;

	tmove->next = move;
}

void do_move (CHAR_DATA * ch, char *argument, int dir) {
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[AVG_STRING_LENGTH];
	char command[12];

	if (!can_move (ch))
		return;

	if ( !IS_NPC(ch) )
	{
		for (af = ch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
				&& af->a.craft->timer)
			{
				send_to_char ("You'll need to stop crafting first.\n", ch);
				return;
			}
		}
	}

	/* If you are trying to move, bag the pmote */
	clear_pmote (ch);

	if (get_affect (ch, MAGIC_TOLL))
		stop_tolls (ch);

	if (dir == UP)
		sprintf (command, "up");
	else if (dir == DOWN)
		sprintf (command, "down");
	else if (dir == EAST)
		sprintf (command, "east");
	else if (dir == WEST)
		sprintf (command, "west");
	else if (dir == NORTH)
		sprintf (command, "north");
	else
		sprintf (command, "south");


	if (!is_mounted (ch) && GET_POS (ch) == POSITION_FIGHTING)
	{

		/* Make sure nobody is trying to fight us before
		declining move request */

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch == ch)
				continue;

			if (tch->fighting == ch && GET_POS (tch) >= POSITION_FIGHTING)
			{
				send_to_char ("You can't get away - try to FLEE instead!\n",
					ch);
				return;
			}
		}

	}

	// Prevent people from easily moving away from their group.

	if ((af = get_affect (ch, AFFECT_GROUP_RETREAT)) || IS_SET(ch->flags, FLAG_FLEE))
	{
		ch->following = 0;
		ch->formation = 0;
	}
	else if (ch->following)
	{
		if (*argument != '!' && ch->in_room == ch->following->in_room)
		{
			sprintf(buf, "If you wish to move away from and stop following $N, type #6%s !#0 to confirm it.", command);
			act(buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
			return;
		}
		if (ch->following->in_room == ch->in_room)
			act("$N is no longer following you.", false, ch->following, 0, ch, TO_CHAR | _ACT_FORMAT);
		ch->following = 0;
		ch->formation = 0;
	}

	if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG) && ch->room->dir_option[dir]
	&& vnum_to_room (ch->room->dir_option[dir]->to_room)
		&& IS_SET (vnum_to_room (ch->room->dir_option[dir]->to_room)->room_flags, FALL)
		&& !IS_SET (ch->act, ACT_FLYING) && *argument != '!')
	{
		sprintf (buf,
			"#6Moving in that direction will quite likely result in a rather nasty fall. If you're sure about this, type \'%s !\' to confirm.#0",
			command);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	

	// Old Somatics Code - put in by Tiamat again

	if ((af = get_soma_affect(ch, SOMA_SHAKES)))
	{
	if ((number(0,25) > GET_CON(ch) || number(0,25) > GET_WIL(ch)) && (number(0,1000) < af->a.soma.lvl_power))
	{
	if (get_affect (ch, MAGIC_HIDDEN) && would_reveal (ch))
	remove_affect_type (ch, MAGIC_HIDDEN);

	send_to_char("You try to move your body, but you are wracked by a sudden spasm of pain and fall to the ground.\n", ch);
	sprintf(buf, "$n twitches violently, limbs spasming as $e collapses to the ground.");
	act (buf, true, ch, 0, 0, TO_ROOM);

	GET_POS (ch) = SIT;
	add_second_affect (SA_STAND, ((25-GET_AGI(ch))+number(1,3)), ch, NULL, NULL, 0);
	if (ch->following);
	ch->formation = 0;

	ch->following = 0;
	return;
	}

	}

	if (IS_RIDER (ch))
		ch = ch->mount;

	move (ch, argument, dir, 0);
}

void
	hitches_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time)
{
	MOVE_DATA *m;

	if (!IS_HITCHER (ch) ||
		GET_FLAG (ch->hitchee, FLAG_ENTERING) ||
		GET_FLAG (ch->hitchee, FLAG_LEAVING))
		return;

	while (ch->moves)
	{
		m = ch->moves;
		ch->moves = m->next;
		if (m->travel_str)
			mem_free (m->travel_str);
		mem_free (m); // MOVE_DATA*
	}

	clear_current_move (ch->hitchee);

	move (ch->hitchee, "", dir, leave_time + arrive_time);
}


void
	do_east (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, EAST);
}

void do_west( CHAR_DATA * ch, char * argument, int cmd ) {
	do_move( ch, argument, WEST );
}

void do_north( CHAR_DATA * ch, char * argument, int cmd ) {
	do_move( ch, argument, NORTH );
}

void do_northeast( CHAR_DATA * ch, char * argument, int cmd ) {
	do_move( ch, argument, NORTHEAST );
}

void
	do_northwest (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, NORTHWEST);
}

void
	do_southeast (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTHEAST);
}

void
	do_southwest (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTHWEST);
}

void
	do_south (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTH);
}

void
	do_up (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, UP);
}

void
	do_down (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, DOWN);
}

int
	find_door (CHAR_DATA * ch, char *type, char *dir)
{
	char buf[MAX_STRING_LENGTH];
	int door;
	char *dirs[] =
	{
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"\n"
	};

	if (*dir)			/* a direction was specified */
	{
		if ((door = search_block (dir, dirs, false)) == -1)	/* Partial Match */
		{
			send_to_char ("That's not a direction.\n", ch);
			return (-1);
		}

		if (EXIT (ch, door))
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
				if (isname (type, EXIT (ch, door)->keyword))
					return (door);
				else
				{
					sprintf (buf, "I see no %s there.\n", type);
					send_to_char (buf, ch);
					return (-1);
				}
			else
				return (door);
		else
		{
			send_to_char ("There is nothing to open or close there.\n", ch);
			return (-1);
		}
	}
	else				/* try to locate the keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
					if (isname (type, EXIT (ch, door)->keyword))
						return (door);

		sprintf (buf, "I see no %s here.\n", type);
		send_to_char (buf, ch);
		return (-1);
	}
}

void do_open (CHAR_DATA * ch, char *argument, int cmd)
{
	extern int second_affect_active;
	int door, other_room;
	char buffer[MAX_STRING_LENGTH];
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Open what?\n", ch);

	if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
	{

		/* this is an object */

		if ( IS_BOOK( obj ) && !IS_ELECTRIC( obj ) )
		{
			if (obj->open)
			{
                send_to_char ("It's already opened.\n", ch);
				return;
			}

			if ( !*dir || atoi (dir) <= 1 )
			{
				if ( !obj->writing || !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
				{
					sprintf (buf,"You open #2%s#0, and notice that it has no pages.\n",
						obj->short_description
                    );
					send_to_char (buf, ch);

					sprintf (buf, "%s#0 opens #2%s#0.",
                        char_short (ch),
						obj->short_description
                    );
					sprintf (buffer, "#5%s", CAP (buf));
					act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

					obj->open = 1;
					return;
				}
				sprintf (buf, "You open #2%s#0 to the first page.\n",
                    obj->short_description
                );
				send_to_char (buf, ch);

				sprintf (buf, "%s#0 opens #2%s#0 to the first page.",
					char_short (ch), obj->short_description
                );
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

				obj->open = 1;
				return;
			}
			else if (atoi (dir) > obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
			{
				send_to_char( "There aren't that many pages in the book.\n", ch );
				return;
			}
			else
			{
				sprintf(buf, "You open #2%s#0 to page %d.\n",
					obj->short_description,
					atoi( dir )
                );
				send_to_char (buf, ch);

				sprintf( buf, "%s#0 opens #2%s#0.",
                    char_short( ch ),
					obj->short_description
                );
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

				obj->open = atoi (dir);
				return;
			}
		}
		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
			{
				send_to_char ("That cannot be opened or closed.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("That's already open.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's locked.\n", ch);
				return;
			}
			obj->o.od.value[2] &= ~CONT_CLOSED;
			act ("You open $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n opens $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0 && vnum_to_room (obj->o.od.value[0])
				&& vnum_to_room (obj->o.od.value[0])->dir_option[OUTSIDE])
			{
				send_to_room ("The entryway is opened from the other side.",
					obj->o.od.value[0]);
				vnum_to_room (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info &=
					~EX_CLOSED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("But it's already open!\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
			send_to_char ("You can't do that.\n", ch);
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("It seems to be locked.\n", ch);
		else
		{
			obj->o.container.flags &= ~CONT_CLOSED;
			act ("You open $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n opens $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* perhaps it is a door */

		/* TODO: find_object_dwelling causes a crash if a guest is logged in ??? */

		if (get_second_affect (ch, SA_COMBAT_DOOR, NULL))
			return;

		if (door > 5 && (obj = find_dwelling_obj (ch->room->vnum)))
		{

			if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
			{
				send_to_char ("This entryway cannot be opened or closed.\n",
					ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("The entrance is already open.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's locked.\n", ch);
				return;
			}
			obj->o.od.value[2] &= ~CONT_CLOSED;
			sprintf (buf, "#2%s#0 is opened from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's impossible, I'm afraid.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("It's already open!\n", ch);
		else if (IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It seems to be locked.\n", ch);
		else
		{
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				if (!second_affect_active && ch->fighting)
				{
					sprintf (buf, "open %s %s", EXIT (ch, door)->keyword, dirs[door]);
					add_second_affect (SA_COMBAT_DOOR, number(5,8), ch, NULL, buf, ch->in_room);
					sprintf (buf, "You attempt to move towards the %s $T in order to to open it.", relative_dirs[door]);
					act (buf, false, ch, 0, EXIT (ch, door)->keyword, TO_CHAR | _ACT_FORMAT);
					return;
				}

				EXIT (ch, door)->exit_info &= ~EX_CLOSED;

				sprintf (buf, "You open the %s $T.", relative_dirs[door]);
				act (buf, false, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s $T.", relative_dirs[door]);
				act (buf, false, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				if (!second_affect_active && ch->fighting)
				{
					sprintf (buf, "open %s", dirs[door]);
					add_second_affect (SA_COMBAT_DOOR, number(5,8), ch, NULL, buf, ch->in_room);
					sprintf (buf, "You attempt to move towards the %s door in order to to open it.", relative_dirs[door]);
					act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}

				EXIT (ch, door)->exit_info &= ~EX_CLOSED;

				sprintf (buf, "You open the %s door.", relative_dirs[door]);
				act ("You open the door.", false, ch, 0, 0,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s door.", relative_dirs[door]);
				act ("$n opens the door.", false, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
			}
			/* now for opening the OTHER side of the door! */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vnum_to_room (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info &= ~EX_CLOSED;
						if (back->keyword && strlen (back->keyword))
						{
							sprintf (buf,
								"The %s %s %s opened from the other side.\n",
								relative_dirs[rev_dir[door]], back->keyword,
								(back->keyword[strlen (back->keyword) - 1] ==
								's') ? "are" : "is");
							/* was  "The %s is opened from the other side.\n",
							fname(back->keyword)); */
							send_to_room (buf, EXIT (ch, door)->to_room);
						}
						else
						{
							sprintf (buf,
								"The %s door is opened from the other side.",
								relative_dirs[rev_dir[door]]);
							send_to_room (buf, EXIT (ch, door)->to_room);
						}
					}
		}
	}
}


void
	do_close (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	if (GET_POS (ch) == POSITION_FIGHTING)
	{
		send_to_char ("No way! You are fighting for your life!\n\r", ch);
		return;
	}

	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Close what?\n", ch);
	else
		if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
		{

			/* this is an object */

			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
			{
				if (!obj->open)
				{
					send_to_char ("That isn't currently open.\n", ch);
					return;
				}
				sprintf (buf, "You close #2%s#0.\n", obj->short_description);
				send_to_char (buf, ch);
				sprintf (buf, "%s#0 closes #2%s#0.", char_short (ch),
					obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = 0;
				return;
			}
			if (obj->obj_flags.type_flag == ITEM_DWELLING)
			{
				if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
				{
					send_to_char ("That cannot be opened or closed.\n", ch);
					return;
				}
				else if (IS_SET (obj->o.od.value[2], CONT_CLOSED))
				{
					send_to_char ("That's already closed.\n", ch);
					return;
				}
				obj->o.od.value[2] |= CONT_CLOSED;
				act ("You close $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n closes $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				if (obj->o.od.value[0] > 0 && vnum_to_room (obj->o.od.value[0]))
				{
					send_to_room ("The entryway is closed from the other side.",
						obj->o.od.value[0]);
					vnum_to_room (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info |=
						EX_CLOSED;
				}
				return;
			}
			else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
				send_to_char ("That's not a container.\n", ch);
			else if (IS_SET (obj->o.container.flags, CONT_CLOSED))
				send_to_char ("But it's already closed!\n", ch);
			else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
				send_to_char ("That's impossible.\n", ch);
			else
			{
				obj->o.container.flags |= CONT_CLOSED;
				act ("You close $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n closes $p.", false, ch, obj, 0, TO_ROOM);
			}
		}
		else if ((door = find_door (ch, type, dir)) >= 0)
		{

			/* Or a door */

			if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
				&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
				send_to_char ("That's absurd.\n", ch);
			else if (IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
				send_to_char ("It's already closed!\n", ch);
			else
			{
				if (door > 5 && (obj = find_dwelling_obj (ch->room->vnum)))
				{
					if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
					{
						send_to_char ("This entryway cannot be opened or closed.\n",
							ch);
						return;
					}
					else if (IS_SET (obj->o.od.value[2], CONT_CLOSED))
					{
						send_to_char ("The entrance is already closed.\n", ch);
						return;
					}
					else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
					{
						send_to_char ("I'm afraid that's locked.\n", ch);
						return;
					}
					obj->o.od.value[2] |= CONT_CLOSED;
					sprintf (buf, "#2%s#0 is closed from the other side.",
						obj_short_desc (obj));
					buf[2] = toupper (buf[2]);
					send_to_room (buf, obj->in_room);
				}
				EXIT (ch, door)->exit_info |= EX_CLOSED;
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
				{
					sprintf (buf, "You close the %s $T.", relative_dirs[door]);
					act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s $T.", relative_dirs[door]);
					act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
						TO_ROOM | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "You close the %s door.", relative_dirs[door]);
					act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s door.", relative_dirs[door]);
					act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				}
				/* now for closing the other side, too */
				if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
					if ((back = vnum_to_room (other_room)->dir_option[rev_dir[door]]))
						if (back->to_room == ch->in_room)
						{
							back->exit_info |= EX_CLOSED;
							if (back->keyword && strlen (back->keyword))
							{
								sprintf (buf,
									"The %s %s closes quietly.\n",
									relative_dirs[rev_dir[door]], back->keyword);
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								sprintf (buf,
									"The %s door closes quietly.\n",
									relative_dirs[rev_dir[door]]);
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}
			}
		}
}


OBJ_DATA *
	has_key (CHAR_DATA * ch, OBJ_DATA * obj, int key)
{
	OBJ_DATA *tobj;

	if (ch->right_hand && ch->right_hand->nVirtual == key)
	{
		tobj = ch->right_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->left_hand && ch->left_hand->nVirtual == key)
	{
		tobj = ch->left_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_KEYRING)
	{
		for (tobj = ch->right_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_KEYRING)
	{
		for (tobj = ch->left_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	return 0;
}


void
	do_lock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Lock what?\n", ch);
	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (obj->o.od.value[3] <= 0)
			{
				send_to_char ("That cannot be locked.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("You'll need to close it first.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's already locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] |= CONT_LOCKED;
			act ("You lock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n locks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0)
			{
				send_to_room ("The entryway is unlocked from the other side.",
					obj->o.od.value[0]);
				vnum_to_room (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info |=
					EX_LOCKED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("Maybe you should close it first...\n", ch);
		else if (obj->o.container.key <= 0)
			send_to_char ("That thing can't be locked.\n", ch);
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			send_to_char ("You don't seem to have the proper key.\n", ch);
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("It is locked already.\n", ch);
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags |= CONT_LOCKED;
			act ("You lock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n locks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* a door, perhaps */

		if ((obj = find_dwelling_obj (ch->room->vnum)))
		{
			if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("You'll need to close the entrance first.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's already locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] |= CONT_LOCKED;
			sprintf (buf, "#2%s#0 is locked from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's absurd.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("You have to close it first, I'm afraid.\n", ch);
		else if (EXIT (ch, door)->key < 0)
			send_to_char ("There does not seem to be any keyholes.\n", ch);
		else if (!has_key (ch, NULL, EXIT (ch, door)->key))
			send_to_char ("You don't have the proper key.\n", ch);
		else if (IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It's already locked!\n", ch);
		else
		{
			EXIT (ch, door)->exit_info |= EX_LOCKED;
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				//dwellings don't have directions for doors
				if (ch->in_room < 100000)
					sprintf (buf, "You lock the %s $T.", relative_dirs[door]);
				else
					sprintf (buf, "You lock the $T.");

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);

				if (ch->in_room < 100000)
					sprintf (buf, "$n locks the %s $T.", relative_dirs[door]);
				else
					sprintf (buf, "$n locks the $T.");

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You lock the %s door.", relative_dirs[door]);
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

				if (ch->in_room < 100000)
					sprintf (buf, "$n locks the %s door.", relative_dirs[door]);
				else
					sprintf (buf, "$n locks the door.");


				act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			/* now for locking the other side, too */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vnum_to_room (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info |= EX_LOCKED;
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s %s is locked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								else
									sprintf (buf,
									"The %s is locked from the other side.\n",
									back->keyword);

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s door is locked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								else
									sprintf (buf,
									"The door is locked from the other side.\n");
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}

					}
		}
	}
}


void
	do_unlock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Unlock what?\n", ch);

	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (obj->o.od.value[3] <= 0)
			{
				send_to_char ("That cannot be unlocked.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that isn't locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] &= ~CONT_LOCKED;
			act ("You unlock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n unlocks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0)
			{
				send_to_room ("The entryway is unlocked from the other side.",
					obj->o.od.value[0]);
				vnum_to_room (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info &=
					~EX_LOCKED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("It isn't closed.\n", ch);
		else if (obj->o.container.key <= 0)
			send_to_char ("Odd - you can't seem to find a keyhole.\n", ch);
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			send_to_char ("You don't have the proper key.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("Oh.. it wasn't locked, after all.\n", ch);
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags &= ~CONT_LOCKED;
			act ("You unlock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n unlocks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}

	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* it is a door */

		if ((obj = find_dwelling_obj (ch->room->vnum)))
		{
			if (!IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that isn't locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] &= ~CONT_LOCKED;
			sprintf (buf, "#2%s#0 is unlocked from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's absurd.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("Heck.. it ain't even closed!\n", ch);
		else if (EXIT (ch, door)->key < 0)
			send_to_char ("You can't seem to spot any keyholes.\n", ch);
		else if (!has_key (ch, NULL, EXIT (ch, door)->key))
			send_to_char ("You do not have the proper key for that.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It's already unlocked, it seems.\n", ch);
		else
		{
			EXIT (ch, door)->exit_info &= ~EX_LOCKED;
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				//dwellings don't have directions for door
				if (ch->in_room < 100000)
					sprintf (buf, "You unlock the %s $T.", relative_dirs[door]);
				else
					sprintf (buf, "You unlock the $T.");

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s $T.", relative_dirs[door]);
				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				//dwellings don't have directions for doors
				if (ch->in_room < 100000)
					sprintf (buf, "You unlock the %s door.", relative_dirs[door]);
				else
					sprintf (buf, "You unlock the door.");

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s door.", relative_dirs[door]);
				act (buf, false, ch, 0, 0, TO_ROOM);
			}
			/* now for unlocking the other side, too */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vnum_to_room (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info &= ~EX_LOCKED;
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								//dwellings don't have directions for doors
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s %s is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								else
									sprintf (buf,
									"The %s is unlocked from the other side.\n",
									back->keyword);

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								//dwellings don't have directions for doors
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s door is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								else
									sprintf (buf,
									"The door is unlocked from the other side.\n");

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}

					}
		}
	}
}

void
	do_pick (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *locked_obj = NULL;

	if (real_skill(ch, SKILL_HIDE) && ch->skills[SKILL_HIDE] >= 10)
		skill_learn(ch, SKILL_PICK);

	if (!real_skill (ch, SKILL_PICK))
	{
		send_to_char ("You don't know how to pick locks.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What would you like to pick?\n", ch);
		return;
	}

	if (!(ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_LOCKPICK) &&
		!(ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_LOCKPICK))
	{
		send_to_char ("You must be holding a lockpick.\n", ch);
		return;
	}

	if (!(locked_obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
		locked_obj = get_obj_in_dark (ch, buf, ch->right_hand);

	if (locked_obj)
	{

		if (locked_obj->obj_flags.type_flag != ITEM_CONTAINER &&
			GET_ITEM_TYPE (locked_obj) != ITEM_DWELLING)
		{
			act ("You can't pick $p.", false, ch, locked_obj, 0, TO_CHAR);
			return;
		}

		if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER)
		{
			if (locked_obj->o.od.value[2] <= 0)
			{
				act ("$p looks unpickable.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.container.flags, CONT_CLOSED))
			{
				act ("$p is already open.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.container.flags, CONT_LOCKED))
			{
				act ("As you start, you discover $p is already unlocked.",
					false, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}
		else
		{
			if (locked_obj->o.od.value[3] <= 0)
			{
				act ("$p looks unpickable.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.od.value[1], CONT_CLOSED))
			{
				act ("$p is already open.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.od.value[1], CONT_LOCKED))
			{
				act ("As you start, you discover $p is already unlocked.",
					false, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}


		sprintf (buf,
			"#3[Guardian: %s%s]#0 Tries to pick the lock of %s in %d.",
			GET_NAME (ch), IS_SET (ch->plr_flags,
			NEW_PLAYER_TAG) ? " (new)" : "",
			locked_obj->short_description, ch->in_room);
		send_to_guardians (buf, 0xFF);

		act ("You begin picking the lock of $p.",
			false, ch, locked_obj, 0, TO_CHAR);
		act ("$n uses $s tools on $p.", true, ch, locked_obj, NULL, TO_ROOM);

		ch->delay_type = DEL_PICK_OBJ;
		ch->delay = 25 - ch->skills[SKILL_PICK] / 10;
		ch->delay_info1 = (long int) locked_obj;

		return;
	}

	switch (*buf)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	default:
		send_to_char ("You may pick north, south, east, west, up, or "
			"down.\n", ch);
		return;
	}

	if (!EXIT (ch, dir))
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	if (!IS_SET (EXIT (ch, dir)->exit_info, EX_ISDOOR)
		&& !IS_SET (EXIT (ch, dir)->exit_info, EX_ISGATE))
	{
		send_to_char ("No door in that direction.\n", ch);
		return;
	}

	if (!IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED))
	{
		send_to_char ("It's already open.\n", ch);
		return;
	}

	sprintf (buf, "#3[Guardian: %s%s]#0 Tries to pick the lock of %s in %d.",
		GET_NAME (ch),
		IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
		EXIT (ch, dir)->keyword, ch->in_room);
	send_to_guardians (buf, 0xFF);

	act ("You try to pick the $T.",
		false, ch, 0, EXIT (ch, dir)->keyword, TO_CHAR);

	if (number (1, 100) > ch->skills[SKILL_PICK])
	{

		act ("$n uses $s tools on the $T.",
			true, ch, 0, EXIT (ch, dir)->keyword, TO_ROOM);

		/* 100% crime in daylight for failure.  1/3 chance if night
		and not guarded. */

		if (sun_light || is_guarded (NULL, ch) || !number (0, 2))
			criminalize (ch, NULL, ch->room->zone, CRIME_PICKLOCK);
	}

	ch->delay_type = DEL_PICK;
	ch->delay = 25 - ch->skills[SKILL_PICK] / 10;
	ch->delay_info1 = dir;
	ch->delay_info2 = ch->in_room;
}

void
	delayed_pick_obj (CHAR_DATA * ch)
{
	int roll;
	OBJ_DATA *tobj;
	OBJ_DATA *locked_obj = NULL;
	OBJ_DATA *obj = NULL;

	locked_obj = (OBJ_DATA *) ch->delay_info1;

	if (!is_obj_here (ch, locked_obj, 1))
	{

		if (ch->right_hand && ch->right_hand == locked_obj)
			obj = ch->right_hand;

		if (ch->left_hand && ch->left_hand == locked_obj)
			obj = ch->left_hand;

		if (obj == locked_obj)
		{
			send_to_char ("You stop picking.\n", ch);
			return;
		}
	}

	if (!IS_SET (locked_obj->o.container.flags, CONT_CLOSED) ||
		!IS_SET (locked_obj->o.container.flags, CONT_LOCKED))
	{
		send_to_char ("You stop picking.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER)
	{
		if (ch->skills[SKILL_PICK] > locked_obj->o.container.pick_penalty)
			skill_use (ch, SKILL_PICK, locked_obj->o.container.pick_penalty);

		if ((roll =
			number (1,
			100)) >
			ch->skills[SKILL_PICK] - locked_obj->o.container.pick_penalty)
		{
			if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)))
			{
				act ("You fail miserably, snapping your pick in the process!",
					false, ch, 0, 0, TO_CHAR);
				act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", false, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", true, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}
	else if (GET_ITEM_TYPE (locked_obj) == ITEM_DWELLING)
	{
		if (ch->skills[SKILL_PICK] > locked_obj->o.od.value[4])
			skill_use (ch, SKILL_PICK, locked_obj->o.od.value[4]);

		if ((roll =
			number (1,
			100)) > ch->skills[SKILL_PICK] - locked_obj->o.od.value[4])
		{
			if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)))
			{
				act ("You fail miserably, snapping your pick in the process!",
					false, ch, 0, 0, TO_CHAR);
				act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", false, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", true, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}

	act ("You have successfully picked the lock.", true, ch, 0, 0, TO_CHAR);
	act ("$n has picked the lock of $p.", true, ch, locked_obj, 0,
		TO_ROOM | _ACT_FORMAT);

	if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER)
		locked_obj->o.container.flags &= ~CONT_LOCKED;
	else
		locked_obj->o.od.value[2] &= ~CONT_LOCKED;
}

void
	delayed_pick (CHAR_DATA * ch)
{
	ROOM_DATA *troom;
	OBJ_DATA *tobj;
	int dir;
	int roll;

	dir = ch->delay_info1;

	if (ch->delay_info2 != ch->in_room || !EXIT (ch, dir))
		return;

	if (IS_SET (EXIT (ch, dir)->exit_info, EX_PICKPROOF))
	{
		send_to_char ("You failed.\n", ch);
		return;
	}

	if (!IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED))
	{
		send_to_char ("Someone has unlocked the door for you.\n", ch);
		return;
	}

	/* Don't let PC get any practice out of this lock if it is beyond
	his/her ability. */

	if (ch->skills[SKILL_PICK] > EXIT (ch, dir)->pick_penalty)
		skill_use (ch, SKILL_PICK, EXIT (ch, dir)->pick_penalty);

	if ((roll =
		number (1, 100)) > ch->skills[SKILL_PICK] - EXIT (ch,
		dir)->pick_penalty)
	{
		if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)))
		{
			act ("You fail miserably, snapping your pick in the process!",
				false, ch, 0, 0, TO_CHAR);
			act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);
			extract_obj (tobj);
			return;
		}
		send_to_char ("You failed.\n", ch);
		return;
	}

	ch->room->dir_option[dir]->exit_info &= ~EX_LOCKED;

	if ((troom = vnum_to_room (ch->room->dir_option[dir]->to_room)))
		troom->dir_option[rev_dir[dir]]->exit_info &= ~EX_LOCKED;

	send_to_char ("You successfully picked the lock.\n", ch);
}

void
	enter_vehicle (CHAR_DATA * ch, CHAR_DATA * ent_mob)
{
	if (!ent_mob->mob)
	{
		send_to_char ("You can't.\n", ch);
		return;
	}

	if (!vnum_to_room (ent_mob->mob->vnum))
	{
		send_to_char ("A note on the entrance says, 'broken'\n", ch);
		system_log ("Attempt to use a broken boat or hitch, enter_vehicle()",
			true);
		return;
	}

	if (ch == ent_mob)
		return;  //you can't enter yourself

	/*		if (!room_avail(vtor (ent_mob->mob->nVirtual), NULL, ch))
	{
	send_to_char("There is not enough room.\n", ch);
	return;
	}*/

	if (ent_mob->mob->vehicle_type == VEHICLE_HITCH)
	{
		act ("You climb into $N.", false, ch, 0, ent_mob, TO_CHAR);
		act ("$n climbs into $N.", true, ch, 0, ent_mob, TO_NOTVICT);
	}

	/*
	else if (ch->room->sector_type != SECT_DOCK)
	send_to_char ("You swim close, grab some netting, and hoist "
	"yourself aboard.\n", ch);
	*/

	char_from_room (ch);

	remove_affect_type (ch, MAGIC_SNEAK);
	remove_affect_type (ch, MAGIC_HIDDEN);

	char_to_room (ch, ent_mob->mob->vnum);

	ch->vehicle = ent_mob;
	ch->coldload_id = ent_mob->coldload_id;

	if (!IS_NPC (ch))
		ch->pc->boat_virtual = ent_mob->mob->vnum;

	act ("$n has boarded.", true, ch, 0, 0, TO_ROOM);

	do_look (ch, "", 15);
}

void
	do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *room;
	OBJ_DATA *obj;
	char buf[MAX_INPUT_LENGTH];
	char tmp[MAX_STRING_LENGTH];
	int door;
	CHAR_DATA *ent_mob, *tch;
	int occupants = 0;

	one_argument (argument, buf);

	/*
	if (!IS_NPC (ch) && IS_SET (ch->flags, FLAG_GUEST) && *buf)
	{
	*buf = tolower (*buf);
	if (!str_cmp (buf, "arena"))
	{
	grunge_arena__do_enter (ch, argument, cmd);
	return;
	}
	}
	*/

	/** Entering a vehicle***/
	if (*buf && (ent_mob = get_char_room_vis (ch, buf)) &&
		IS_SET (ent_mob->act, ACT_VEHICLE))
	{
		enter_vehicle (ch, ent_mob);
		return;
	}

	/** Enter a dwelling **/
	if (*buf && (obj = get_obj_in_list_vis (ch, buf, ch->room->contents))
		&& (GET_ITEM_TYPE (obj) == ITEM_DWELLING))
	{
		argument = one_argument (argument, buf);
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHABLE)
			&& !IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED))
		{
			send_to_char ("You'll need to pitch it first.\n", ch);
			return;
		}
		if (IS_SET (obj->o.od.value[2], CONT_CLOSED)
			&& !(!IS_MORTAL (ch) && *argument == '!'))
		{
			send_to_char ("You'll need to open it before entering.\n", ch);
			return;
		}
		//if (obj->o.od.value[5] < 99000 || obj->o.od.value[5] > 99999)
		//{
		//	send_to_char
		//		("This dwelling object doesn't have a master floorplan set!\n",
		//		ch);
		//	return;
		//}
		room = generate_dwelling_room (obj);
		if (!room)
		{
			send_to_char
				("There is an error with that dwelling object. Please report.\n",
				ch);
			return;
		}
		for (tch = room->people; tch; tch = tch->next_in_room)
			occupants++;

		if (obj->o.od.value[1] > 0 && occupants >= obj->o.od.value[1])
		{
			send_to_char ("There are already too many people inside.\n", ch);
			return;
		}
		sprintf (buf, "You enter #2%s#0.", obj_short_desc (obj));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n enters #2%s#0.", obj_short_desc (obj));
		act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		obj->o.od.value[0] = room->vnum;
		ch->was_in_room = ch->in_room;
		char_from_room (ch);
		char_to_room (ch, room->vnum);
		send_to_char ("\n", ch);
		do_look (ch, "", 77);
		sprintf (buf, "$n enters from the outside.");
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		if (IS_MORTAL (ch))
			room->occupants++;
		return;
	}

	if (*buf)			/* an argument was supplied, search for door keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
					if (!str_cmp (EXIT (ch, door)->keyword, buf))
					{
						do_move (ch, "", door);
						return;
					}
					sprintf (tmp, "There is no %s here.\n", buf);
					send_to_char (tmp, ch);
	}
	else if (IS_SET (vnum_to_room (ch->in_room)->room_flags, INDOORS))
		send_to_char ("You are already indoors.\n", ch);
	else
	{
		/* try to locate an entrance */
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->to_room != NOWHERE)
					if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED) &&
						IS_SET (vnum_to_room (EXIT (ch, door)->to_room)->room_flags, INDOORS))
					{
						do_move (ch, "", door);
						return;
					}
					send_to_char ("You can't seem to find anything to enter.\n", ch);
	}
}


void
	leave_vehicle (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *vehicle;
	ROOM_DATA *troom;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (ch->vehicle)
	{

		vehicle = ch->vehicle;

		if (vehicle->mob->vnum != ch->in_room)
		{
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}

		act ("$n disembarks.", true, ch, 0, 0, TO_ROOM);

		troom = vnum_to_room (vehicle->in_room);

		if (troom->sector_type == SECT_OCEAN ||
			is_room_affected (troom->affects, MAGIC_ROOM_FLOOD))
		{
			if (*buf != '!')
			{
				act ("$N is at sea.", false, ch, 0, vehicle, TO_CHAR);
				send_to_char ("If you jump ship into the water, type:\n"
					"   #3leave !#0\n", ch);
				return;
			}
		}

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		if (!IS_NPC (ch))
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", true, ch, 0, vehicle, TO_ROOM);

		do_look (ch, "", 15);
		return;
	}
	else if (IS_SET (ch->room->room_flags, VEHICLE))
	{
		if (!(vehicle = get_mob_vnum (ch->room->vnum)))
		{
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}
		act ("$n disembarks.", true, ch, 0, 0, TO_ROOM);

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		if (!IS_NPC (ch))
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", true, ch, 0, vehicle, TO_ROOM);

		do_look (ch, "", 15);
		return;
	}

	send_to_char ("You cannot exit the craft from here.\n", ch);
}

void
	do_leave (CHAR_DATA * ch, char *argument, int cmd)
{

	if (ch->vehicle || IS_SET (ch->room->room_flags, VEHICLE))
	{
		leave_vehicle (ch, argument);
		return;
	}

	do_outside (ch, argument, 0);
}

void
	do_stand (CHAR_DATA * ch, char *argument, int cmd)
{
	int already_standing = 0;
	int dir;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH] = "";
	char buf2[MAX_STRING_LENGTH] = "";
	char buf3[MAX_STRING_LENGTH] = "";
	std::string first_person, third_person;

	if (!can_move (ch))
	{
		send_to_char ("You can't move.\n", ch);
		return;
	}

	if (*argument != '(')
	{
		argument = one_argument (argument, buf);
	}

	if (*buf)
	{
		if (!str_cmp(buf, "centre") || !str_cmp(buf, "clear") || !str_cmp(buf, "c"))
		{
			if ((af = get_affect (ch, AFFECT_SHADOW)))
			{
				dir = rev_dir[af->a.shadow.edge];
				sprintf(buf2, "You move back from the %s.", direction[dir]);
				sprintf(buf3, "$n moves back from the %s.", direction[dir]);

				act (buf2, false, ch, 0, 0, TO_CHAR);
				act (buf3, false, ch, 0, 0, TO_ROOM);
				affect_remove(ch, af);
			}
			else
			{
				send_to_char("But you are not standing on any edge.\n", ch);
			}
			return;
		}

		if ((dir = index_lookup (dirs, buf)) == -1)
		{
			send_to_char ("Stand where?\n", ch);
			return;
		}

		if (!(af = get_affect (ch, AFFECT_SHADOW)))
		{

			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
			af->a.shadow.edge = -1;	/* Not on an edge */
		}

		move (ch, "stand", dir, 0);
		return;
	}


	
	// Old Somatics Code no longer needed - put in again by Tiamat
	if (get_soma_affect(ch, SOMA_SHAKES) && get_second_affect(ch, SA_STAND, NULL))
	{
	send_to_char ("Your body twitches and flails, your legs refusing to obey you!\n", ch);
	}


	if (get_second_affect (ch, SA_STAND, NULL))
		return;

	if (get_second_affect (ch, SA_INVOLCOVER, NULL) && !ch->fighting)
	{
		send_to_char("You're can't will yourself to stand just yet.\n", ch);
		get_second_affect(ch, SA_INVOLCOVER, NULL)->info2 = 1;
		return;
	}

	if ((static_cast<float>(ch->shock) / ch->max_shock) < 0.2000)
	{
		send_to_char("You are too badly shaken and traumatised to stand right now.\n", ch);
		return;
	}

	if (GET_POS (ch) == STAND)
		already_standing = 1;

	switch (GET_POS (ch))
	{
	case STAND:
		act ("You are already standing.", false, ch, 0, 0, TO_CHAR);
		return;
		break;

	case SIT:
		if ((af = get_affect (ch, AFFECT_COVER)))
		{
			clear_pmote(ch);
			if (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, ch->room->contents))
			{
				std::string temp;
				temp = "You rise up from behind " + MAKE_STRING(obj_short_desc(af->a.cover.obj)) + ".";
				first_person.assign(temp);
				temp = "#0 rises up from behind " + MAKE_STRING(obj_short_desc(af->a.cover.obj)) + ".";

				third_person.assign("#5");
				third_person.append(char_short(ch));
				third_person[2] = toupper (third_person[2]);
				third_person.append(temp);
			}
			else
			{
				first_person.assign("You rise up from cover");
				third_person.assign("#5");
				third_person.append(char_short(ch));
				third_person[2] = toupper (third_person[2]);
				third_person.append("#0 rises out of cover");
			}


			remove_cover(ch, -2);

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS (ch) = STAND;
			else
				return;

			break;
		}

		else if ((af = get_affect (ch, MAGIC_SIT_TABLE)) &&
			is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents))
		{
			first_person.assign("You get up from #2");
			first_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			first_person.append("#0");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 gets up from #2");
			third_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			third_person.append("#0");

		}
		else
		{
			first_person.assign("You stand up");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 clambers on #5");
			third_person.append(HSHR(ch));
			third_person.append("#0 feet");
		}
		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			GET_POS (ch) = STAND;
		else
			return;

		break;

	case REST:

		first_person.assign("You stop resting, and stand up");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 stops resting, and clambers on #5");
		third_person.append(HSHR(ch));
		third_person.append("#0 feet");

		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			GET_POS (ch) = POSITION_STANDING;
		else
			return;

		break;

	case SLEEP:
		do_wake (ch, argument, 0);
		break;

	case FIGHT:
		act ("You are standing and fighting.", false, ch, 0, 0, TO_CHAR);
		break;

	default:
		act ("You stop floating around, and put your feet on the ground.",
			false, ch, 0, 0, TO_CHAR);
		act ("$n stops floating around, and puts $s feet on the ground.",
			true, ch, 0, 0, TO_ROOM);
		break;
	}

	if (!already_standing && GET_POS (ch) == STAND)
		follower_catchup (ch);

	if (GET_POS (ch) != SIT && GET_POS (ch) != SLEEP &&
		(af = get_affect (ch, MAGIC_SIT_TABLE)))
		affect_remove (ch, af);

	watched_action(ch, "stand up", 0, 0);
}


void
	do_sit (CHAR_DATA * ch, char *argument, int cmd)
{
	int count;
	CHAR_DATA *tch;
	SECOND_AFFECT *sa;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]="";
	std::string first_person, third_person;

	if (*argument != '(')
		argument = one_argument (argument, buf);

	// if ((sa = get_second_affect (ch, SA_STAND, NULL)) && !(GET_POS(ch) == FIGHTING)))
	//	remove_second_affect (sa);

	if (*buf && !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		send_to_char ("You don't see that to sit at.\n", ch);
		return;
	}

	if (obj)
	{

		if (!IS_TABLE(obj))
		{
			act ("You cannot sit at $p.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		if (GET_POS (ch) != STAND)
		{
			act ("You must be standing before you can sit at $p.",
				false, ch, obj, 0, TO_CHAR);
			return;
		}

		count = 0;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch == ch)
				continue;

			if (((af = get_affect (tch, MAGIC_SIT_TABLE)) || (af = get_affect (tch, AFFECT_COVER))) &&
				af->a.table.obj == obj)
				count++;
		}

		if (obj->o.container.table_max_sitting != 0 &&
			count >= obj->o.container.table_max_sitting)
		{
			act ("There is no available space at $p.", false, ch, obj, 0,
				TO_CHAR);
			return;
		}

		first_person.assign("You sit at #2");
		first_person.append(obj_short_desc(obj));
		first_person.append("#0");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 sits at #2");
		third_person.append(obj_short_desc(obj));
		third_person.append("#0");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
		{
			magic_add_affect (ch, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0);

			af = get_affect (ch, MAGIC_SIT_TABLE);

			af->a.table.obj = obj;

			GET_POS (ch) = SIT;
		}

		clear_pmote(ch);

		return;
	}

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
		{
			first_person.assign("You sit down");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 sits down");

			if (evaluate_emote_string (ch, &first_person, third_person, argument))
			{
				GET_POS (ch) = POSITION_SITTING;
			}
			else
				return;
		}
		break;
	case POSITION_SITTING:
		{
			send_to_char ("You're sitting already.\n", ch);
		}
		break;
	case POSITION_RESTING:
		{
			first_person.assign("You stop resting, and sit up");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 stops resting");
			if (evaluate_emote_string (ch, &first_person, third_person, argument))
				GET_POS (ch) = POSITION_SITTING;
			else
				return;
		}
		break;
	case POSITION_SLEEPING:
		{
			act ("You have to wake up first.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_FIGHTING:
		act ("YOU'RE FIGHTING!  YOU'LL GET SLAUGHTERED!!!",
			false, ch, 0, 0, TO_CHAR);
		act ("You sit down in combat.", false, ch, 0, 0, TO_CHAR);
		act ("$n sits down in combat.", true, ch, 0, 0, TO_ROOM);
		GET_POS (ch) = SIT;
		break;
	default:
		{
			act ("You stop floating around, and sit down.", false, ch, 0, 0,
				TO_CHAR);
			act ("$n stops floating around, and sits down.", true, ch, 0, 0,
				TO_ROOM);
			GET_POS (ch) = POSITION_SITTING;
		}
		break;
	}
	clear_pmote(ch);

	watched_action(ch, "sit down", 0, 0);
}


void
	do_rest (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;

	if (!can_move (ch))
	{
		send_to_char ("You can't move.\n", ch);
		return;
	}

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
		{
			first_person.assign("You sit down and rest your tired bones");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 sits down and rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS(ch) = POSITION_RESTING;
			else
				return;
		}
		break;
	case POSITION_SITTING:
		{
			first_person.assign("You rest your tired bones");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS(ch) = POSITION_RESTING;
			else
				return;
		}
		break;
	case POSITION_RESTING:
		{
			act ("You are already resting.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_SLEEPING:
		{
			act ("You have to wake up first.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_FIGHTING:
		{
			act ("Rest while fighting? are you MAD?", false, ch, 0, 0, TO_CHAR);
		}
		break;
	default:
		{
			act ("You stop floating around, and stop to rest your tired bones.",
				false, ch, 0, 0, TO_CHAR);
			act ("$n stops floating around, and rests.", false, ch, 0, 0,
				TO_ROOM);
			GET_POS (ch) = POSITION_SITTING;
		}
		break;
	}

	clear_pmote(ch);
	watched_action(ch, "sit down to rest", 0, 0);
}


void
	do_sleep (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;
	clear_pmote (ch);

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
	case POSITION_SITTING:
	case POSITION_RESTING:

		first_person.assign("You fall asleep");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);

		if (IS_SET (ch->act, ACT_MOUNT))
		{
			third_person.append("#0 lowers #5");
			third_person.append(HSHR(ch));
			third_person.append("#0 head and falls asleep");
		}
		else
			third_person.append("#0 falls asleep");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
			GET_POS (ch) = POSITION_SLEEPING;
		else
			return;

		break;

	case POSITION_SLEEPING:
		send_to_char ("You are already sound asleep.\n", ch);
		break;

	case POSITION_FIGHTING:
		send_to_char ("Sleep while fighting? are you MAD?\n", ch);
		break;

	default:
		act ("You stop floating around, and lie down to sleep.",
			false, ch, 0, 0, TO_CHAR);
		act ("$n stops floating around, and lie down to sleep.",
			true, ch, 0, 0, TO_ROOM);
		GET_POS (ch) = POSITION_SLEEPING;
		break;
	}

	watched_action(ch, "lay down to sleep", 0, 0);
}

int
	wakeup (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;

	if (GET_POS (ch) != SLEEP)
		return 0;

	if (get_affect (ch, MAGIC_AFFECT_SLEEP))
		return 0;

	send_to_char ("Your sleep is disturbed.\n", ch);

	if ((af = get_affect (ch, MAGIC_SIT_TABLE)) &&
		is_obj_in_list (af->a.table.obj, ch->room->contents))
		GET_POS (ch) = SIT;
	else
		GET_POS (ch) = REST;

	return 1;
}

void
	do_wake (CHAR_DATA * ch, char *argument, int not_noisy)
{
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if (!(tch = get_char_room_vis (ch, buf)))
		{
			if (not_noisy)
				send_to_char ("They aren't here.\n", ch);
			return;
		}
	}

	/* Awaken someone else */

	if (*buf && tch != ch)
	{

		if (GET_POS (ch) == POSITION_SLEEPING)
		{
			send_to_char ("You must be awake yourself to do that.\n", ch);
			return;
		}

		if (GET_POS (tch) != POSITION_SLEEPING)
		{
			act ("$E isn't asleep.", true, ch, 0, tch, TO_CHAR);
			return;
		}

		if (get_affect (tch, MAGIC_AFFECT_SLEEP))
		{
			if (not_noisy)
				act ("$E doesn't respond.", false, ch, 0, tch, TO_CHAR);
			return;
		}

		if (get_second_affect (tch, SA_KNOCK_OUT, NULL))
		{
			if (not_noisy)
				act ("$E can't be roused.", false, ch, 0, tch, TO_CHAR);
			return;
		}

		act ("You wake $M up.", false, ch, 0, tch, TO_CHAR);
		GET_POS (tch) = POSITION_RESTING;

		act ("You are awakened by $n.", false, ch, 0, tch, TO_VICT);

		if ((tch = being_dragged (ch)))
		{
			act ("You awaken to discover $N was dragging you!",
				false, ch, 0, tch, TO_CHAR);
			if ((af = get_affect (tch, MAGIC_DRAGGER)))
				affect_remove (tch, af);
		}

		return;
	}

	/* Awaken yourself */

	if (get_affect (ch, MAGIC_AFFECT_SLEEP))
	{
		send_to_char ("You can't wake up!\n", ch);
		return;
	}

	if (GET_POS (ch) > POSITION_SLEEPING)
		send_to_char ("You are already awake...\n", ch);

	else if (GET_POS (ch) == POSITION_UNCONSCIOUS)
		send_to_char ("You're out cold, I'm afraid.\n", ch);

	else if (get_second_affect (ch, SA_KNOCK_OUT, NULL))
		send_to_char ("Your body is still recovering from trauma.", ch);

	else
	{
		GET_POS (ch) = REST;

		if ((tch = being_dragged (ch)))
		{
			if (GET_POS (ch) == REST)
			{
				act ("You awake to discover $N is dragging you!",
					false, ch, 0, tch, TO_CHAR);
			}
			else
			{
				act ("You awaken to discover $N was dragging you!",
					false, ch, 0, tch, TO_CHAR);
				if ((af = get_affect (tch, MAGIC_DRAGGER)))
					affect_remove (tch, af);
			}
		}
		else
		{
			if (GET_POS (ch) == REST)
				act ("You open your eyes.", false, ch, 0, 0, TO_CHAR);
			else
				act ("You awaken and stand.", false, ch, 0, 0, TO_CHAR);
		}

		act ("$n awakens.", true, ch, 0, 0, TO_ROOM);
	}
}



void
	do_sail (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	int to_sector;
	int we_moved = 0;
	char direction[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;

	if (!ch->vehicle)
	{
		send_to_char ("You're not on a sailing vessel.\n", ch);
		return;
	}

	/*
	This should be unedited when there is finally some sailing, but for now,
	it's gone as SKILL_SAIL is undefined.

	if (!real_skill (ch, SKILL_SAIL))
	{
	send_to_char ("Seek knowledge of sailing first.\n", ch);
	return;
	}

	*/

	if (ch->in_room != ch->vehicle->mob->helm_room)
	{
		send_to_char ("Navigation is done from the helm only.\n", ch);
		return;
	}

	argument = one_argument (argument, direction);

	if ((dir = index_lookup (dirs, direction)) == -1)
	{
		send_to_char ("Sail where?\n", ch);
		return;
	}

	if (!CAN_GO (ch->vehicle, dir))
	{
		send_to_char ("You can't sail that way.\n", ch);
		return;
	}

	to_sector = vnum_to_room (EXIT (ch->vehicle, dir)->to_room)->sector_type;

	if (is_room_affected (vnum_to_room (EXIT (ch->vehicle, dir)->to_room)->affects,
		MAGIC_ROOM_FLOOD))
	{
		send_to_char ("The flooded land there is too shallow to sail.\n", ch);
		return;
	}

	if (to_sector != SECT_OCEAN)
	{
		send_to_char ("Land lies that way.\n", ch);
		return;
	}

	/*
	This should be unedited when there is finally some sailing, but for now,
	it's gone as SKILL_SAIL is undefined.

	if (skill_use (ch, SKILL_SAIL, 0))
	{
	do_move (ch->vehicle, "", dir);
	we_moved = 1;
	}*/

	else if (number (0, 4) == 4)
	{
		dir = (dir + 1) % 4;

		if (!CAN_GO (ch->vehicle, dir))
		{
			send_to_char ("The wind stalls.\n", ch);
			return;
		}

		to_sector = vnum_to_room (EXIT (ch->vehicle, dir)->to_room)->sector_type;

		if (to_sector != SECT_OCEAN)
		{
			send_to_char ("The wind isn't cooperating.\n", ch);
			return;
		}

		do_move (ch->vehicle, "", dir);

		sprintf (buf, "The wind and currents force you off "
			"course to the %s.\n", dirs[dir]);
		we_moved = 1;
	}

	else
		send_to_char ("The wind stalls.\n", ch);

	if (we_moved)
		for (tch = character_list; tch; tch = tch->next)
		{

			if (tch->deleted)
				continue;

			if (tch->vehicle == ch->vehicle &&
				(IS_OUTSIDE (tch)
				|| tch->vehicle->mob->helm_room == tch->in_room))
				act ("$N sails . . .", false, tch, 0, tch->vehicle, TO_CHAR);
		}
}

/*	track finds the shortest path between a character and a room somewhere
in the mud.
*/

int search_sequence = 0;
int high_water = 0;
int rooms_scanned = 0;
/*
int
track_room (ROOM_DATA * from_room, int to_room)
{
int room_set_1[5000];		// These hardly need to be more than
int room_set_2[5000];		// about 200 elements...5000 tb safe
int *rooms;
int *torooms;
int *tmp;
int targets;
int options;
int i;
int dir;
ROOM_DATA *room;
ROOM_DATA *opt_room;
ROOM_DATA *target_room;

if (!from_room)
return -1;

if (!(target_room = vtor (to_room)))
return -1;

search_sequence++;

rooms = room_set_1;
torooms = room_set_2;

options = 1;

rooms[0] = to_room;

high_water = 0;

while (options)
{

targets = 0;

for (i = 0; i < options; i++)
{

opt_room = vtor (rooms[i]);

for (dir = 0; dir <= LAST_DIR; dir++)
{

if (!opt_room->dir_option[dir] ||
opt_room->dir_option[dir]->to_room == NOWHERE)
continue;

room = vtor (opt_room->dir_option[dir]->to_room);

if (!room)
{
continue;
}

if (!room->dir_option[rev_dir[dir]] ||
room->dir_option[rev_dir[dir]]->to_room !=
opt_room->nVirtual)
{
// printf ("Room %d only connects '%s' with room %d.\n",
// opt_room->nVirtual, dirs [dir], room->nVirtual);
continue;
}

if (room->search_sequence == search_sequence)
continue;

rooms_scanned++;

room->search_sequence = search_sequence;

if (room->nVirtual == from_room->nVirtual)
return rev_dir[dir];

torooms[targets++] = room->nVirtual;

if (targets > high_water)
high_water = targets;
}
}

tmp = rooms;
rooms = torooms;
torooms = tmp;		// Important - must point at other structure.

options = targets;
}

return -1;
}
*/
int
	track (CHAR_DATA * ch, int to_room)
{
	int room_set_1[5000];		/* These hardly need to be more than */
	int room_set_2[5000];		/*  about 200 elements...5000 tb safe */
	int *rooms;
	int *torooms;
	int *tmp;
	int targets;
	int options;
	int i;
	int dir;
	int count = 0;
	char buf[MAX_STRING_LENGTH];	/* KILLER CDR */
	ROOM_DATA *room;
	ROOM_DATA *opt_room;
	ROOM_DATA *target_room;

	if (!(target_room = vnum_to_room (to_room)))
		return -1;

	if (to_room == ch->in_room)
		return -1;

	search_sequence++;

	rooms = room_set_1;
	torooms = room_set_2;

	options = 1;

	rooms[0] = to_room;

	high_water = 0;

	while (options)
	{

		targets = 0;

		for (i = 0; i < options; i++)
		{

			opt_room = vnum_to_room (rooms[i]);

			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!opt_room->dir_option[dir] ||
					opt_room->dir_option[dir]->to_room == NOWHERE)
					continue;

				room = vnum_to_room (opt_room->dir_option[dir]->to_room);

				if (!room)
				{
					sprintf (buf, "Huh?  Room %d -> %d (bad)\n",
						opt_room->vnum,
						opt_room->dir_option[dir]->to_room);
					send_to_char (buf, ch);
					continue;
				}

				if (!room->dir_option[rev_dir[dir]] ||
					room->dir_option[rev_dir[dir]]->to_room !=
					opt_room->vnum)
				{
					/*                      printf ("Room %d only connects '%s' with room %d.\n",
					opt_room->nVirtual, dirs [dir], room->nVirtual);
					*/
					continue;
				}

				// Ensures that mobiles can get around rivers -- without this, they'll
				// just sit at the edges of them, thumb up their ass.

				if (!IS_SET (ch->act, ACT_FLYING) &&
					(room->sector_type == SECT_RIVER || room->sector_type == SECT_LAKE
					|| room->sector_type == SECT_OCEAN || room->sector_type == SECT_UNDERWATER))
					continue;

				// Vehicles can only travel along their path as set via access flags

				if (IS_SET (ch->act, ACT_VEHICLE)
					&& !(ch->mob->access_flags & room->room_flags))
					continue;

				if (room->search_sequence == search_sequence)
					continue;

				rooms_scanned++;

				room->search_sequence = search_sequence;

				if (GET_TRUST (ch))
				{
					if (!strncmp (room->description, "No Description Set", 18))
					{
						if (count % 12 == 11)
							send_to_char ("\n", ch);
						count++;

						sprintf (buf, "%5d ", room->vnum);
						send_to_char (buf, ch);
					}
				}

				if (room->vnum == ch->in_room)
					return rev_dir[dir];

				torooms[targets++] = room->vnum;

				if (targets > high_water)
					high_water = targets;
			}
		}

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targets;
	}

	if (GET_TRUST (ch))
	{
		sprintf (buf, "\nTotal rooms: %d\n", count);
		send_to_char (buf, ch);
	}

	return -1;
}


int
	pathfind (int from_room, int to_room)
{
	int room_set_1[5000];		/* These hardly need to be more than */
	int room_set_2[5000];		/*  about 200 elements...5000 tb safe */
	int *rooms;
	int *torooms;
	int *tmp;
	int targets;
	int options;
	int i;
	int dir;
	ROOM_DATA *room;
	ROOM_DATA *opt_room;
	ROOM_DATA *target_room;

	if (!(target_room = vnum_to_room (to_room)))
		return -1;

	search_sequence++;

	rooms = room_set_1;
	torooms = room_set_2;

	options = 1;

	rooms[0] = to_room;

	high_water = 0;

	while (options)
	{
		targets = 0;

		for (i = 0; i < options; i++)
		{

			opt_room = vnum_to_room (rooms[i]);

			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!opt_room->dir_option[dir] ||
					opt_room->dir_option[dir]->to_room == NOWHERE)
					continue;

				room = vnum_to_room (opt_room->dir_option[dir]->to_room);

				if (!room)
				{
					continue;
				}

				if (!room->dir_option[rev_dir[dir]] ||
					room->dir_option[rev_dir[dir]]->to_room !=
					opt_room->vnum)
				{
					continue;
				}

				if (room->search_sequence == search_sequence)
					continue;

				rooms_scanned++;

				room->search_sequence = search_sequence;

				if (room->vnum == from_room)
					return rev_dir[dir];

				torooms[targets++] = room->vnum;

				if (targets > high_water)
					high_water = targets;
			}
		}

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targets;
	}

	return -1;
}


char *
	crude_name (int race)
{
	char *ptrBodyProto = NULL;
	if ((ptrBodyProto = lookup_race_variable (race, RACE_BODY_PROTO)) != NULL)
	{
		switch (strtol (ptrBodyProto, NULL, 10))
		{
		case PROTO_HUMANOID:
			return "bipedal creature";
		case PROTO_FOURLEGGED_PAWS:
		case PROTO_FOURLEGGED_FEET:
			return "four-legged creature";
		case PROTO_FOURLEGGED_HOOVES:
			return "four-legged, cloven-hoofed creature";
		case PROTO_SERPENTINE:
			return "no-legged creatre";
		case PROTO_SPIDER:
			return "pincer-legged creature";
		case PROTO_BOT:
			return "mechanical device";
		case PROTO_WINGED_TAIL:
		case PROTO_WINGED_NOTAIL:
		case PROTO_GHOST:
		default:
			;
		}
	}
	return "unknown creature";
}

char *
	specific_name (int race)
{
	static char buf[MAX_STRING_LENGTH];

	///\TODO: Make racially non-specific
	switch (race)
	{
	case 0:   // Unknown
	case 1:   // Survivor
	case 2:   // Hosted-Terror
	case 5:   // Denizen
	case 67:  // Mutation
	case 68:  // Cybernetic
	case 69:  // Phoenixer
		return "humanoid";
	default:
		strcpy (buf, lookup_race_variable (race, RACE_NAME));
		if (!*buf || strcmp (buf, "(null)") == 0)
		{
			return "unknown creature";
		}
		*buf = tolower (*buf);
		return buf;
	}

}

char *
	track_age (int hours_passed)
{
	if (hours_passed <= 1)
		return "within the hour";
	else if (hours_passed <= 4)
		return "recently";
	else if (hours_passed <= 6)
		return "within hours";
	else if (hours_passed <= 12)
		return "within a half-day";
	else if (hours_passed <= 24)
		return "within a day";
	else if (hours_passed <= 48)
		return "within a couple days";
	else if (hours_passed <= 72)
		return "within a few days";
	else
		return "within days";
}

char *
	speed_adj (int speed)
{
	if (speed == 0)
		return "a brisk walk";
	else if (speed == 1)
		return "a faltering stagger";
	else if (speed == 2)
		return "a deliberate pace";
	else if (speed == 3)
		return "a swift jog";
	else if (speed == 4)
		return "a loping run";
	else if (speed == 8)
		return "a inching crawl";
	else
		return "a haphazard sprint";
}

void
	delayed_track (CHAR_DATA * ch)
{

	TRACK_DATA *track;
	bool found = false;
	int needed;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];


	int skill_cap = ch->skills[SKILL_HUNTING];

	skill_use (ch, SKILL_HUNTING, 0);

	// Basically, if you went over 50 with this skill as
	// a result of skill_use, we taper you back down -
	// can't spam your way to master.
	if (skill_cap == 50 && ch->skills[SKILL_HUNTING] > 50)
	{
		ch->skills[SKILL_HUNTING] = skill_cap;
	}

	*output = '\0';

	for (track = ch->room->tracks; track; track = track->next)
	{
		*buf = '\0';
		needed = ch->skills[SKILL_HUNTING];
		needed -= track->hours_passed / 4;
		if (IS_SET (track->flags, BLOODY_TRACK))
			needed += number (40, 45);
		needed += number (10, 20);
		needed = MAX (needed, 5);
		if (number (1, 100) > needed)
			continue;
		if (!found)
			send_to_char ("\n", ch);
		found = true;
		if (needed < 30)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 are here, leading from %s to %s.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race), dirs[track->from_dir],
				dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 are here, coming from the %s and then doubling back.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race), dirs[track->from_dir]);
		}
		else
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.",
				specific_name (track->race), IS_SET (track->flags,
				BLOODY_TRACK) ?
				", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir],
				dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.",
				specific_name (track->race), IS_SET (track->flags,
				BLOODY_TRACK) ?
				", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir]);
		}
		*buf = toupper (*buf);
		sprintf (output + strlen (output), "%s ", buf);
	}

	if (!found)
	{
		send_to_char ("You were unable to locate any tracks in the area.\n",
			ch);
		return;
	}
	else
	{
		reformat_string (output, &p);
		page_string (ch->descr(), p);
		mem_free (p); //char*
	}
}


void
	skill_tracking (CHAR_DATA * ch, char *argument, int cmd)
{
	int delay;
	TRACK_DATA *track;
	char buf[MAX_STRING_LENGTH];


	if (ch->room->sector_type == SECT_INSIDE || ch->room->sector_type == SECT_OUTSIDE)
	{
		send_to_char ("Tracking in such an area is all but impossible.\n", ch);
		return;
	}

	if (!IS_MORTAL (ch))
	{
		if (!ch->room->tracks)
		{
			send_to_char ("There are no tracks here.\n", ch);
			return;
		}
		sprintf (buf, "The following tracks are here:\n\n");
		for (track = ch->room->tracks; track; track = track->next)
		{
			if (track->from_dir)
				sprintf (buf + strlen (buf),
				"%s tracks, from the %s, heading %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->from_dir], dirs[track->to_dir],
				speeds[track->speed], track->hours_passed);
			else
				sprintf (buf + strlen (buf),
				"%s tracks heading to the %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->to_dir], speeds[track->speed],
				track->hours_passed);
			if (IS_SET (track->flags, PC_TRACK))
				sprintf (buf + strlen (buf), " #2(PC %s)#0", track->name);
			if (IS_SET (track->flags, BLOODY_TRACK))
				strcat (buf, " #1(bloody)#0");
			if (IS_SET (track->flags, FLEE_TRACK))
				strcat (buf, " #6(fled)#0");
			strcat (buf, "\n");
		}
		send_to_char (buf, ch);
		return;
	}

	skill_learn(ch, SKILL_HUNTING);

	if (!real_skill (ch, SKILL_HUNTING))
	{
		send_to_char ("You don't know how to track!\n", ch);
		return;
	}

	if (is_dark (ch->room) && IS_MORTAL (ch)
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !is_goggled(ch)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("You can't see well enough to pick up any tracks.\n", ch);
		return;
	}

	// Base time is 11 seconds, but every 10 points and every part of skill in tracking
	// you have reduces it by 1, so 33 is 7 seconds, 78 is 3 seconds.

	delay = 11 - (int) (ch->skills[SKILL_HUNTING] / 10);

	ch->delay = delay;
	ch->delay_type = DEL_TRACK;
	send_to_char ("You survey the area carefully, searching for tracks...\n",
		ch);
	act ("$n surveys the area slowly, searching for something.", true, ch, 0, 0,
		TO_ROOM | _ACT_FORMAT);
}

void
	do_track (CHAR_DATA * ch, char *argument, int cmd)
{
	int to_room;
	int dir;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	bool nodded = false;

	// If an npc was commanded by a player, disallow the track to succeed.
	if (cmd > 0 && IS_NPC (ch) && ((GET_TRUST (ch)) >= (cmd - 1)))
		return;

	if (!IS_NPC (ch))
	{
		if (IS_MORTAL (ch) || !*argument)
		{
			skill_tracking (ch, argument, cmd);

			return;
		}
	}

	argument = one_argument (argument, buf);

	if (isdigit (*buf))
		to_room = atoi (buf);
	else if ((tch = get_char_vis (ch, buf)))
		to_room = tch->in_room;
	else
	{
		send_to_char ("You can't locate them.\n", ch);
		return;
	}

	if (ch->in_room == to_room)
		return;

	dir = track (ch, to_room);

	if (dir == -1)
	{
		send_to_char ("Unknown direction.\n", ch);
		return;
	}

	if (GET_TRUST (ch))
	{
		sprintf (buf, "[High: %d Scan: %d] Move %s\n",
			high_water, rooms_scanned, dirs[dir]);
		send_to_char (buf, ch);
	}

	high_water = 0;
	rooms_scanned = 0;

	one_argument (ch->room->dir_option[dir]->keyword, buf2);
	sprintf (buf, "%s %s", buf2, dirs[dir]);

	if (IS_SET (ch->room->dir_option[dir]->exit_info, EX_LOCKED))
	{
		if (ch->room->dir_option[dir]->key)
		{
			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (get_obj_in_list_num
					(ch->room->dir_option[dir]->key, tch->right_hand)
					|| get_obj_in_list_num (ch->room->dir_option[dir]->key,
					tch->left_hand))
				{
					name_to_ident (tch, buf2);
					sprintf (buf, "%s", buf2);
					do_nod (ch, buf, 0);
					nodded = true;
					break;
				}
			}
		}
		if (!nodded)
			do_knock (ch, buf, 0);
	}
	else if (IS_SET (ch->room->dir_option[dir]->exit_info, EX_CLOSED))
	{
		do_open (ch, buf, 0);
	}

	do_move (ch, "", dir);
}

void
	do_drag (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];

	argument = one_argument (argument, name);

	if (!(victim = get_char_room_vis (ch, name)))
	{
		if (!(obj = get_obj_in_list_vis (ch, name, ch->room->contents)))
		{
			send_to_char ("Drag who or what?\n", ch);
			return;
		}
	}

	if (victim)
	{
		if (ch == victim)
		{
			send_to_char ("Drag yourself?\n", ch);
			return;
		}

		if (IS_SWIMMING (ch))
		{
			send_to_char ("You can barely tred water yourself!\n", ch);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Drag where?\n", ch);
		return;
	}

	if (victim && AWAKE (victim))
	{
		act ("$N is conscious, you can't drag $M.",
			false, ch, 0, victim, TO_CHAR);
		return;
	}

	if (victim && number (1, 4) == 1)
	{
		do_wake (ch, name, 1);
		if (GET_POS (victim) > SLEEP)
			return;
	}

	if (victim && IS_SUBDUER (ch))
	{
		send_to_char
			("You can't drag anything while you have someone in tow.\n", ch);
		return;
	}

	if (obj && !IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
	{
		act ("$p is firmly attached.  You can't drag it.",
			false, ch, obj, 0, TO_CHAR);
		return;
	}

	magic_add_affect (ch, MAGIC_DRAGGER, -1, 0, 0, 0, 0);

	if ((af = get_affect (ch, MAGIC_DRAGGER)))
		af->a.spell.t = obj ? (long int) obj : (long int) victim;

	do_move (ch, "", dir);
}

int
	is_mounted (CHAR_DATA * ch)
{
	/* Is a mount mounted, or has a PC mounted something */

	if (!ch->mount || !is_he_here (ch, ch->mount, false))
		return 0;

	if (ch != ch->mount->mount)
	{
		ch->mount = NULL;
		return 0;
	}

	return 1;
}

int
	can_mount (CHAR_DATA * ch, CHAR_DATA * mount)
{
	if (CAN_SEE (ch, mount) &&
		IS_SET (mount->act, ACT_MOUNT) &&
		!IS_SET (ch->act, ACT_MOUNT) &&
		GET_POS (mount) == STAND && !is_mounted (mount))
		return 1;

	return 0;
}

int
	hitch_char (CHAR_DATA * ch, CHAR_DATA * hitch)
{
	if (IS_HITCHER (ch))
		return 0;

	if (!IS_SET (hitch->act, ACT_MOUNT))
		return 0;

	if (IS_HITCHEE (hitch))
		return 0;

	ch->hitchee = hitch;
	hitch->hitcher = ch;

	return 1;
}

void
	unhitch_char (CHAR_DATA * ch, CHAR_DATA * hitch)
{
	if (ch != hitch->hitcher || ch->hitchee != hitch)
		return;

	ch->hitchee = NULL;
	hitch->hitcher = NULL;
}

void
	do_hitch (CHAR_DATA * ch, char *argument, int cmd)
{
	int is_vehicle = 0;
	CHAR_DATA *hitch;
	CHAR_DATA *hitcher;
	OBJ_DATA *bridle;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!(hitch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf && IS_HITCHER (ch))
	{
		act ("You currently have $N hitched.",
			true, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if (hitch->mob && hitch->mob->vehicle_type == VEHICLE_HITCH)
		is_vehicle = 1;

	if (!IS_SET (hitch->act, ACT_MOUNT))
	{
		act ("$N can't be hitched.", true, ch, 0, hitch, TO_CHAR);
		return;
	}

	if (IS_HITCHEE (hitch))
	{
		act ("$N is already hitched to $3.",
			true, ch, (OBJ_DATA *) hitch->hitcher, hitch, TO_CHAR);
		return;
	}

	if (IS_RIDEE (hitch))
	{
		act ("$N is riding that animal.", true, ch, 0, hitch->mount, TO_CHAR);
		return;
	}

	if (!is_vehicle)
	{
		if (!(bridle = get_equip (hitch, WEAR_NECK_1)))
		{
			act ("$N doesn't have a bridle to grab.",
				true, ch, 0, hitch, TO_CHAR);
			return;
		}

		if (GET_ITEM_TYPE (bridle) != ITEM_BRIDLE)
		{
			act ("$N is wearing $o, which is not a bridle.",
				true, ch, bridle, hitch, TO_CHAR);
			return;
		}
	}

	if (!*buf && is_vehicle)
	{
		send_to_char ("You may only hitch a vehicle to an animal or another "
			"vehicle.\n", ch);
		return;
	}

	if (!*buf)
	{

		hitch_char (ch, hitch);

		act ("You grab the reins of $N.", true, ch, 0, hitch, TO_CHAR);
		act ("$n grabs the reins of $N.", false, ch, 0, hitch, TO_NOTVICT);
		act ("$N grabs your reins.", true, hitch, 0, ch, TO_CHAR);

		return;
	}

	if (!str_cmp (buf, "to"))
		argument = one_argument (argument, buf);

	if (!(hitcher = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_SET (hitcher->act, ACT_MOUNT))
	{
		act ("$N can't be hitched.", true, ch, 0, hitcher, TO_CHAR);
		return;
	}

	if (IS_HITCHER (hitcher))
	{
		act ("$N has an animal hitched already.",
			true, ch, 0, hitcher, TO_CHAR);
		return;
	}

	hitch_char (hitcher, hitch);

	act ("$n hitches $N to $3.",
		false, ch, (OBJ_DATA *) hitcher, hitch, TO_NOTVICT);
	act ("$N hitches $3 to you.",
		false, ch, (OBJ_DATA *) hitch, hitcher, TO_VICT);
	act ("You hitch $3 to $N.", true, ch, (OBJ_DATA *) hitch, hitcher, TO_CHAR);
	act ("$N hitches you to $3.",
		true, hitch, (OBJ_DATA *) hitcher, ch, TO_CHAR);
}

void
	do_unhitch (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *hitch;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		if (!is_he_here (ch, ch->hitchee, 0))
		{
			send_to_char ("Unhitch what?\n", ch);
			return;
		}

		hitch = ch->hitchee;
	}

	else if (!(hitch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_HITCHEE (hitch))
	{
		act ("$N isn't currently hitched.", true, ch, 0, hitch, TO_CHAR);
		return;
	}

	/* Mount is hitched, but not by us.  Check for non-mount hitch */

	if (hitch->hitcher != ch && !IS_SET (hitch->hitcher->act, ACT_MOUNT))
	{
		act ("$N is holding that animal's reigns.",
			true, ch, 0, hitch->hitcher, TO_CHAR);
		return;
	}

	unhitch_char (hitch->hitcher, hitch);

	act ("$n unhitches $N.", true, ch, 0, hitch, TO_NOTVICT);
	act ("You unhitch $N.", true, ch, 0, hitch, TO_CHAR);
	act ("$N unhitches you.", true, hitch, 0, ch, TO_CHAR);
}

void
	do_mount (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mount;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (mount = ch->room->people; mount; mount = mount->next_in_room)
			if (can_mount (ch, mount))
				break;

		if (!mount)
		{
			send_to_char ("There is nothing here you can mount.\n", ch);
			return;
		}
	}

	else if (!(mount = get_char_room_vis (ch, buf)))
	{
		send_to_char ("There is no such mount here.\n", ch);
		return;
	}

	if (!can_mount (ch, mount))
	{
		act ("You can't mount $N.", false, ch, 0, mount, TO_CHAR);
		return;
	}

	if (IS_SET(mount->affected_by, AFF_TRANSPORTING))
	{
		act ("There's no room on $N's back for you!.", false, ch, 0, mount, TO_CHAR);
		return;
	}

	skill_learn(ch, SKILL_HANDLE);

	if (!real_skill (ch, SKILL_HANDLE) && mount->skills[SKILL_HANDLE] < 33)
	{
		act ("$N is too wild for you to even try.",
			false, ch, 0, mount, TO_CHAR);
		return;
	}

	if (real_skill (mount, SKILL_HANDLE) < 33 && !skill_use (ch, SKILL_HANDLE, 0))
	{
		act ("$N backs away just as you attempt to mount it.",
			false, ch, 0, mount, TO_CHAR);
		act ("You back away as $N approaches you.",
			false, mount, 0, ch, TO_CHAR);
		act ("$N backs away when $n attempts to mount it.",
			false, ch, 0, mount, TO_NOTVICT);
		return;
	}

	if (mount->skills[SKILL_HANDLE] < 90 && !skill_use (mount, SKILL_HANDLE, -75))
	{
		act ("You manage to straddle $N, but it quickly throws you off.",
			false, ch, 0, mount, TO_CHAR);
		act ("$n is thrown to the ground after attempting to mount $N.",
			false, ch, 0, mount, TO_NOTVICT);
		act ("You throw $N to the ground after $E attempts to mount you.",
			false, mount, 0, ch, TO_CHAR);
		return;
	}

	act ("$n perches $mself on top of $N.", false, ch, 0, mount, TO_NOTVICT);
	act ("You mount $N.", false, ch, 0, mount, TO_CHAR);
	act ("$N mounts you.", false, mount, 0, ch, TO_CHAR);

	mount->mount = ch;
	ch->mount = mount;

	unhitch_char (ch, mount);
}

void
	do_tame (CHAR_DATA * ch, char *argument, int cmd)
{
	send_to_char ("This command is under construction.\n", ch);
}

void
	do_dismount (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!is_mounted (ch))
	{
		ch->mount = NULL;
		send_to_char ("You're not on a mount.\n", ch);
		return;
	}

	act ("$n dismounts from $N.", false, ch, 0, ch->mount, TO_NOTVICT);
	act ("You dismount from $N.", false, ch, 0, ch->mount, TO_CHAR);
	act ("$N gets off of you.", false, ch->mount, 0, ch, TO_CHAR);

	hitch_char (ch, ch->mount);

	ch->mount->mount = NULL;
	ch->mount = NULL;
}

/*
Character          Skill       Meaning
----------         --------    -------------------------------------
PC               Tame        No meaning
PC               Ride        If the mount bucks, the rider falls if his
RIDE skill check fails.
If RIDE skill is 0 for PC, he cannot attempt
to mount a horse with RIDE skill < 33
PC               Break       If PC tries to bridle a mount with no TAME:
If PC's BREAK < 50, mount shies away, else
If PC passes BREAK skill check:
TAME is opened on mount, and is bridled
else
PC gets kicked for 2d12 and fails bridle
NOTE:  I intended to create a break command, but
never did, so BREAK skill isn't complete.
mob              Tame        If mount has TAME >= 33, PC can always mount.
If mount has 0 < TAME < 33, mount will shy bridle
1d100 < mount's TAME * 3
mob              Ride        If
*/

void
	do_bridle (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mount;
	OBJ_DATA *obj;
	OBJ_DATA *bridle;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (IS_HITCHER (ch))
	{
		act ("Unhitch $N first.", true, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if (!*buf)
	{
		send_to_char ("What do you want to bridle?\n", ch);
		return;
	}

	if (!(mount = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_SET (mount->act, ACT_MOUNT))
	{
		act ("$N is not a mount; it can't be bridled.",
			false, ch, 0, mount, TO_CHAR);
		return;
	}

	if (!(bridle = ch->right_hand))
	{
		send_to_char ("You need to hold a bridle in your right hand.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (bridle) != ITEM_BRIDLE)
	{
		act ("$o is not a bridle.", false, ch, bridle, 0, TO_CHAR);
		return;
	}

	if ((obj = get_equip (mount, WEAR_NECK_1)))
	{
		act ("$N already has $o around its neck.",
			false, ch, obj, mount, TO_CHAR);
		return;
	}

	if (!real_skill (mount, SKILL_HANDLE))
	{
		if (ch->skills[SKILL_HANDLE] < 30)
		{
			act ("$N shies away as attempt to slip $o onto $M.",
				false, ch, bridle, mount, TO_CHAR);
			return;
		}
		if (!skill_use (ch, SKILL_HANDLE, -30))
		{
			act ("$N kicks you as you attempt to bridle $M!",
				false, ch, 0, mount, TO_CHAR);
			act ("You kick $N when $E tries to bridle you.",
				false, mount, 0, ch, TO_CHAR);
			act ("$N kicks $n as $E attempts to bridle $M.",
				false, ch, 0, mount, TO_NOTVICT);
			wound_to_char (ch, figure_location (ch, HITLOC_BODY), dice (2, 12), 3, 0, 0,
				0);
			return;
		}
	}

	else if (mount->skills[SKILL_HANDLE] < 33)
	{

		if (number (1, 100) < mount->skills[SKILL_HANDLE] * 3)
		{
			act ("$N shies away as attempt to slip $p onto $M.", false, ch,
				bridle, mount, TO_CHAR);
			return;
		}
	}

	act ("You slip $p onto $N.", false, ch, bridle, mount, TO_CHAR);
	act ("$N slips $p over your neck.", false, mount, bridle, ch, TO_CHAR);
	act ("$n slips $p over $N's neck.", false, ch, bridle, mount, TO_NOTVICT);

	obj_from_char (&bridle, 0);

	equip_char (mount, bridle, WEAR_NECK_1);

	hitch_char (ch, mount);
}

void
	dump_rider (CHAR_DATA * rider, int forced)
{
	if (!IS_RIDER (rider))
		return;

	if (!skill_use (rider, SKILL_HANDLE, 0))
	{
		if (forced == 2)
		{
			act ("You buck and thrash as the missile strikes, throwing $N off your back!",
				false, rider->mount, 0, rider, TO_CHAR);
			act ("$N bucks and thrashes as the missile strikes, throwing you off!",
				false, rider, 0, rider->mount, TO_CHAR);
			act ("$N bucks and thrashes as the missile strikes, throwing $n off its back.",
				true, rider, 0, rider->mount, TO_NOTVICT);
		}
		else
		{
			act ("You throw $N off your back!",
				false, rider->mount, 0, rider, TO_CHAR);
			act ("$N bucks and throws you off!",
				false, rider, 0, rider->mount, TO_CHAR);
			act ("$N bucks $n off its back.",
				true, rider, 0, rider->mount, TO_NOTVICT);
		}

		rider->mount->mount = NULL;
		rider->mount = NULL;

		if (wound_to_char (rider, figure_location (rider, HITLOC_LOLEGS), 1, 3, 0, 0, 0))
			return;

		GET_POS (rider) = SIT;
		add_second_affect (SA_STAND, 5, rider, NULL, NULL, 0);

		return;
	}

	act ("You buck as you pretend to be REAL scared.",
		false, rider->mount, 0, rider, TO_CHAR);
	act ("$N bucks and tries to throw you off!", false, rider, 0, rider->mount, TO_CHAR);
	act ("$N bucks and tries to throw $n off its back.", true, rider, 0, rider->mount, TO_NOTVICT);
}

void
	do_buck (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	if (!IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("This is a mount only command.\n", ch);
		return;
	}

	if (!IS_RIDEE (ch))
	{
		send_to_char ("You're not being ridden by anybody.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf == '!')
		dump_rider (ch->mount, true);
	else
		dump_rider (ch->mount, false);
}

void
	do_shadow (CHAR_DATA * ch, char *argument, int cmd)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (!(af = get_affect (ch, AFFECT_SHADOW)) || !af->a.shadow.shadow)
		{
			send_to_char ("You aren't shadowing anybody.\n", ch);
			return;
		}

		send_to_char ("Ok.\n", ch);

		af->a.shadow.shadow = NULL;

		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("They aren't here to shadow.\n", ch);
		return;
	}

	if (tch == ch)
	{
		if ((af = get_affect (ch, AFFECT_SHADOW)))
		{
			send_to_char ("Ok, you stop shadowing.\n", ch);
			af->a.shadow.shadow = NULL;
			return;
		}

		send_to_char ("Ok.\n", ch);
		return;
	}

	magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

	af = get_affect (ch, AFFECT_SHADOW);

	af->a.shadow.shadow = tch;
	af->a.shadow.edge = -1;	/* Not on an edge */

	act ("You will attempt to shadow $N.", false, ch, 0, tch, TO_CHAR);
}

void
	shadowers_shadow (CHAR_DATA * ch, int to_room, int move_dir)
{
	int dir;
	CHAR_DATA *tch;
	CHAR_DATA *next_in_room;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *room;

	/* ch is leaving in direction dir.  Pick up people in same room and
	have them move to the edge.  Pick up people in surrounding rooms on
	edges and have them move to edge of ch's current room too. */

	/* Handle current room shadowers first */

	if (ch->in_room && !ch->room)
	{
		ch->room = vnum_to_room(ch->in_room);
	}

	for (tch = ch->room->people; tch; tch = next_in_room)
	{

		next_in_room = tch->next_in_room;

		if (ch == tch)
			continue;

		if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
			continue;

		if (tch->moves)
			continue;

		if (!(af = get_affect (tch, AFFECT_SHADOW)))
			continue;

		if (af->a.shadow.shadow != ch)
			continue;

		if (!CAN_SEE (tch, ch))
			continue;

		move = (MOVE_DATA *) alloc (sizeof (MOVE_DATA), 24);

		move->dir = move_dir;
		move->flags = MF_TOEDGE;
		move->desired_time = 0;
		move->next = NULL;
		move->travel_str = NULL;
		move->group_str = NULL;

		tch->moves = move;

		initiate_move (tch);
	}

	/* Handle people who are on the edge of this room.  Those people
	will head to the edge of this room joining the room that ch is
	about to enter. */

	for (dir = 0; dir <= LAST_DIR; dir++)
	{

		if (!(exit = EXIT (ch, dir)))
			continue;

		if (exit->to_room == to_room)	/* The shadowee is returning */
			continue;

		if (!(room = vnum_to_room (exit->to_room)))
			continue;

		for (tch = room->people; tch; tch = next_in_room)
		{

			next_in_room = tch->next_in_room;

			if (ch == tch)
				continue;		/* Hopefully not possible */

			if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
				continue;

			if (tch->moves)
				continue;

			if (!(af = get_affect (tch, AFFECT_SHADOW)))
				continue;

			if (af->a.shadow.shadow != ch)
				continue;

			if (af->a.shadow.edge != rev_dir[dir])
				continue;

			if (!could_see (tch, ch))	/* Make sure shadowee is visable */
				continue;

			/* Make N/PC enter room of ch as ch leaves */

			move = (MOVE_DATA *) alloc (sizeof (MOVE_DATA), 24);

			move->dir = af->a.shadow.edge;
			move->flags = MF_TONEXT_EDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;
			move->group_str = NULL;

			tch->moves = move;

			/* Make N/PC move to edge joining room ch just entered */

			move = (MOVE_DATA *) alloc (sizeof (MOVE_DATA), 24);

			move->dir = move_dir;
			move->flags = MF_TOEDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;
			move->group_str = NULL;

			tch->moves->next = move;

			initiate_move (tch);
		}
	}
}

void
	watched_action(CHAR_DATA *ch, char *arg, int dist, int mode)
{
	int dir;
	CHAR_DATA *tch;
	AFFECTED_TYPE *waf;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *next_room;
	char buf[MAX_STRING_LENGTH];
	char end[1] = {'\0'};
	size_t j = 0;
	char heard[MAX_STRING_LENGTH] = { '\0' };
	char speech[MAX_STRING_LENGTH] = { '\0' };

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge/* && could_see(tch, ch)*/)
				{
					target_sighted (tch, ch);
					if (!mode)
					{
						sprintf (buf, "To the %s, you see $n %s", direction[rev_dir[waf->a.shadow.edge]], arg);
					}
					else
					{
						// Mode 2 is text from emotes with quotes:
						// we need to run the bit in quotation marks through the
						// whisper_it function to determine what people can
						// hear from a distance. Fun times.

						if (mode == 2)
						{
							*speech = '\0';
							*heard = '\0';
							j = 0;
							for (size_t y = 0; y <= strlen (arg); y++)
							{
								if (arg[y]=='\"') //if this is true, we have found the first quotation mark
								{
									sprintf(speech, "\"");
									j = y + 1;
									while (arg[j]!='\"') //move through the speech string until you find more quotation marks
									{
										sprintf(speech + strlen(speech), "%c", arg[j]);
										j++;
									}

									sprintf(speech + strlen(speech), "\"");
									whisper_it(tch, speech, speech, 1);
									sprintf(heard + strlen(heard), "%s", speech);
									y = j;
								}
								else
								{
									sprintf(heard + strlen(heard), "%c", arg[y]);
								}
							}
							sprintf (buf, "To the %s, %s", direction[rev_dir[waf->a.shadow.edge]], heard);
						}
						else
						{
							sprintf (buf, "To the %s, %s", direction[rev_dir[waf->a.shadow.edge]], arg);
						}
					}

					*end = buf[strlen(buf) - 1];

					if (str_cmp(end, "."))
						strcat(buf, ".");

					strcat(buf, "\n");

					act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					check_overwatch(tch, ch, false);
				}
			}
		}

		exit = next_room->dir_option[dir];

		if (!(exit) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
			continue;

		if (dist)
		{
			for (tch = next_room->people; tch; tch = tch->next_in_room)
			{
				if ((waf = get_affect (tch, AFFECT_WATCH_DIR)))
				{
					if (rev_dir[dir] == waf->a.shadow.edge/* && could_see(tch, ch)*/)
					{
						target_sighted (tch, ch);

						if (!mode)
							sprintf (buf, "Far to the %s, you see $n %s",
							direction[rev_dir[waf->a.shadow.edge]], arg);
						else
							sprintf (buf, "Far to the %s, %s",
							direction[rev_dir[waf->a.shadow.edge]], arg);

						*end = buf[strlen(buf) - 1];

						if (str_cmp(end, "."))
							strcat(buf, ".");

						strcat(buf, "\n");

						act(buf, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
						check_overwatch(tch, ch, false);
					}
				}
			}
		}
	}
}

int
	room_avail(ROOM_DATA *troom, CHAR_DATA *ch)
{
	int count = 0;
	CHAR_DATA *tch = NULL;

	if (troom->capacity == 0)
		return(1);

	if (IS_SET(troom->room_flags, FORT))
		return(1);

	if (ch && (IS_NPC (ch) || IS_MORTAL (ch)))
	{
		for (tch = troom->people; tch; tch = tch->next_in_room)
		{
			if ((IS_NPC (tch) || IS_MORTAL (tch)) && !IS_SET(tch->act, ACT_SQUEEZER))
				count++;
		}
	}

	if (count >= troom->capacity)
		return (0); //there is no room
	else
		return (1); //there is room
}


void
	do_push (CHAR_DATA *ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *tch;
	int dir;
	int capac;
	int i = 0;
	int mod;
	int check;

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if (IS_SUBDUER (ch))
	{
		send_to_char ("You can't push while you have someone in tow.\n", ch);
		return;
	}

	if (get_second_affect (ch, SA_PUSH, NULL))
	{
		send_to_char ("You need a few more moments breath before you can attempt to push again.\n", ch);
		return;
	}

	if (IS_SWIMMING (ch))
	{
		send_to_char ("You can't do that while swimming!\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		if (IS_NPC (ch) && IS_SET (ch->affected_by, AFF_SNEAK))
		{
			if (cmd == 0)
				sprintf (buf, "north");
			if (cmd == 1)
				sprintf (buf, "east");
			if (cmd == 2)
				sprintf (buf, "south");
			if (cmd == 3)
				sprintf (buf, "west");
			if (cmd == 4)
				sprintf (buf, "up");
			if (cmd == 5)
				sprintf (buf, "down");
		}
		else
		{
			send_to_char ("Push in what direction?\n\r", ch);
			return;
		}

	}

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Push where?\n\r", ch);
		return;
	}

	if (!CAN_GO (ch, dir))
	{
		if (ch->room->extra && ch->room->extra->alas[dir])
			send_to_char (ch->room->extra->alas[dir], ch);
		else
			send_to_char ("There is no exit that way.\n", ch);
		return;
	}

	ROOM_DATA * dest = vnum_to_room (ch->room->dir_option[dir]->to_room);

	if (dest && (capac = dest->capacity) <= 0 && room_avail(dest, ch))
	{
		send_to_char ("The room has plenty of room for you to enter without needing to push.\n", ch);
		return;
	}

	for (tch = dest->people; tch; tch = tch->next_in_room)
	{
		if ((IS_NPC (tch) || IS_MORTAL (tch)) && !IS_SET(tch->act, ACT_SQUEEZER))
		{
			i++;
		}
	}

	mod = i - capac;

	if (mod < 0)
		mod = 0;

	check = number ((1 + mod), (20 + mod));

	weaken (ch, 0, check, NULL);

	if (IS_SET(ch->act, ACT_SQUEEZER))
	{
		send_to_char ("You easily slip in.\n", ch);
		sprintf(buf, "$n slips %sward.", direction[dir]);
		act(buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	else if (check < GET_STR(ch))
	{
		send_to_char ("You force your way in.\n", ch);
		sprintf(buf, "$n forces $s way %sward.", direction[dir]);
		act(buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	else if (check < GET_AGI(ch))
	{
		send_to_char ("You manage to squeeze your way in.\n", ch);
		sprintf(buf, "$n squeezes $s way %sward.", direction[dir]);
		act(buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	else
	{
		send_to_char ("You try to force and squeeze your way in, but fail.\n", ch);
		sprintf(buf, "$n attempts to force and squeeze $s way %sward, but fails.", direction[dir]);
		act(buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		add_second_affect (SA_PUSH, 10, ch, NULL, NULL, 0);
		return;
	}

	add_second_affect (SA_PUSH, 10, ch, NULL, NULL, 1);

	do_move (ch, "", dir);
}

