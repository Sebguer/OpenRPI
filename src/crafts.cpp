/*------------------------------------------------------------------------\
|  crafts.c : Crafting Module                         www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>


#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"


#define s(a) send_to_char (a "\n", ch);

extern rpie::server engine;
extern std::multimap<std::string, room_prog> craft_prog_list;
extern const char *weather_states[];
const char *phase_flags[] =
{
	"cannot-leave-room",
	"open_on_self",		/* "open:" the skill on self */
	"require_on_self",		/* "req:" skill on self */
	"require_greater",		/* "req:" comparison is greater than */
	"\n"
};

const char *item_default_flags[] =
{
	"in-room",
	"give",
	"held",
	"wielded",
	"used",
	"produced",
	"worn",
	"sampled",
	"progressed",
	"optional",
	"sampled-held",
	"sampled-room",
	"used-held",
	"used-room",
	"progressed-held",
	"progressed-room",
	"optional-held",
	"optional-room",
	"\n"
};

const char *craft_mobile_flags[] =
{
	"set-owner",
	"\n"
};

const char *subcraft_flags[] =
{
	"object",
	"character",
	"defensive",
	"offensive",
	"area",
	"room",
	"self",
	"hidden",
	"\n"
};

const char *attrs[] =
{
	"str",
	"dex",
	"con",
	"wil",
	"int",
	"aur",
	"agi",
	"\n"
};


int crafts_in_craftset (char *category)
{
	SUBCRAFT_HEAD_DATA *craft;
	int count = 0;

	for (craft = crafts; craft; craft = craft->next)
	{
		if (!str_cmp (craft->craft_name, category))
		{
			count ++;
		}
	}

	return count;
}

int crafts_of_pc (CHAR_DATA * ch, char *category)
{
	int i = 0, j = 0;
	AFFECTED_TYPE *af;

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if ((af = get_affect (ch, i)))
		{
			if (!str_cmp (af->a.craft->subcraft->craft_name, category))
			{
				j++;
			}
		}
	}

	return j;
}

// Determines the average roll needed to beat this craft.

int craft_roll_average(int x, int y)
{
	return (x + (x * y))/2;

}

// Determines the maximum roll needed to beat this craft.

int craft_roll_max(int x, int y)
{
	return x * y;
}

int
	missing_craft_items (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	char *p;
	bool missing_said = false;
	int i = 0;
	int item_required[MAX_ITEMS_PER_SUBCRAFT];
	PHASE_DATA *phase = NULL;
	DEFAULT_ITEM_DATA *item;
	OBJ_DATA *obj_list[MAX_ITEMS_PER_SUBCRAFT];

	if (!af->a.craft || !af->a.craft->subcraft)
		return 0;

	memset (item_required, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (int));
	memset (obj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));

	for (phase = af->a.craft->subcraft->phases; phase; phase = phase->next)
	{

		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{

			item = af->a.craft->subcraft->obj_items[i];

			if (!item)
			{
				continue;
			}

			if (!item->items[0])  /* No items in list.  Nothing required */
				continue;

			obj_list[i] = get_item_obj (ch, item, phase);
		}

		if (phase->first)
		{
			for (p = phase->first; *p; p++)
			{
				if (*p == '$')
				{
					p++;
					if (isdigit (*p) && atoi (p) < MAX_ITEMS_PER_SUBCRAFT)
						item_required[atoi (p)] = 1;
				}
			}
		}
	}

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{

		if (!item_required[i])
			continue;

		if (obj_list[i])
			continue;

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
		{
			continue;
		}

		if (item
			&& (IS_SET (item->flags, SUBCRAFT_PRODUCED)
			|| IS_SET (item->flags, SUBCRAFT_GIVE)))
		{
			continue;
		}
		if (!missing_said)
		{
			missing_said = true;
			send_to_char ("\n#1You are missing one or more items:#0\n\n", ch);
			act ("$n stops doing $s craft.", false, ch, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN))
			missing_item_msg (ch, item, "You must wear ");
		else if (IS_SET (item->flags, SUBCRAFT_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_USED_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_USED_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED))
			missing_item_msg (ch, item, "In the room or held must be ");
		else if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_WIELDED))
			missing_item_msg (ch, item, "You must wield ");
		else
			missing_item_msg (ch, item, "You need ");
	}

	if (missing_said)
		return 1;

	return 0;
}

AFFECTED_TYPE *
	is_craft_command (CHAR_DATA * ch, char *argument)
{
	int i;
	SUBCRAFT_HEAD_DATA *craft;
	AFFECTED_TYPE *af;
	char command[MAX_STRING_LENGTH];
	char subcraft_name[MAX_STRING_LENGTH];
	char craft_argument[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
		return NULL;

	if (IS_MORTAL (ch) && ch->room && IS_SET (ch->room->room_flags, OOC))
		return NULL;

	argument = one_argument (argument, command);
	argument = one_argument (argument, subcraft_name);
	//sprintf (subcraft_name, "%s", argument);
	argument = one_argument (argument, craft_argument);

	for (craft = crafts; craft; craft = craft->next)
		if (!str_cmp (command, craft->command) &&
			!str_cmp (subcraft_name, craft->subcraft_name))
			break;

	if (!craft)
		return NULL;

	if (*craft_argument)
		craft->argument = add_hash(craft_argument);
	else
		craft->argument = NULL;

	/*
	craft->color_round = 0;

	for (int jind = 0; jind < MAX_DEFAULT_ITEMS; jind++)
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			craft->load_color[jind][ind] = '\0';
		}
	}

	for (int jind = 0; jind < MAX_DEFAULT_ITEMS; jind++)
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			craft->load_cat[jind][ind] = '\0';
		}
	}
	*/

	if (!IS_MORTAL (ch))
	{
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				break;
			else
			{
				if (af->a.craft->subcraft == craft)
				{
					return af;
				}
			}
		}
		magic_add_affect (ch, i, -1, 0, 0, 0, 0);
		af = get_affect (ch, i);
		af->a.craft =
			(struct affect_craft_type *) alloc (sizeof (struct affect_craft_type),
			23);
		af->a.craft->subcraft = craft;
	}
	else
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				continue;

			if (af->a.craft->subcraft == craft)
				break;
		}

		if (i > CRAFT_LAST)
			return NULL;

		return af;
}

void
	craft_command (CHAR_DATA * ch, char *command_args,
	AFFECTED_TYPE * craft_affect)
{
	char *argument;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *target_ch = NULL;
	OBJ_DATA *target_obj = NULL;
	SUBCRAFT_HEAD_DATA *subcraft;
	AFFECTED_TYPE *af;
	bool sectors = false;
	bool pass = false;
	bool seasonchk = false;
	bool weatherchk = false;
	int i = 0;

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
			&& af->a.craft->timer)
		{
			send_to_char ("You are already crafting something.\n", ch);
			return;
		}
	}

	subcraft = craft_affect->a.craft->subcraft;

	argument = one_argument (command_args, buf);	/* Toss subcraft name */

	argument = one_argument (argument, buf);	/* Target */

	if (*buf)
	{
		if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_SELF))
		{
			sprintf (buf, "'%s %s' can only be done on you.\n",
				subcraft->command, subcraft->subcraft_name);
			send_to_char (buf, ch);
			return;
		}

		else if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_OBJ))
		{
			if (!(target_obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
				!(target_obj = get_obj_in_dark (ch, buf, ch->left_hand)))
			{
				send_to_char ("You don't have that in your inventory.\n", ch);
				return;
			}
		}

		else if (IS_SET (subcraft->subcraft_flags, SCF_AREA))
		{
			send_to_char ("This is an area spell; it takes no target.\n", ch);
			return;
		}

		else if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR))
		{
			if (!(target_ch = get_char_room_vis (ch, buf)))
			{
				send_to_char ("Your target isn't here.\n", ch);
				return;
			}
		}
	}
	else
		target_ch = ch;

	craft_affect->a.craft->target_ch = target_ch;
	craft_affect->a.craft->target_obj = target_obj;

	if (get_affect (ch, MAGIC_CRAFT_DELAY)
		&& (craft_affect->a.craft->subcraft->delay || craft_affect->a.craft->subcraft->faildelay) && IS_MORTAL (ch) && !engine.in_test_mode ())
	{
		act
			("Sorry, but your OOC craft delay timer is still in place. You'll receive a notification when it expires and you're free to craft delayed items again.",
			false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	/** Sectors **/
	pass = false;
	for (i = 0; i <= 24; i++)
		if (craft_affect->a.craft->subcraft->sectors[i])
			sectors = true;

	if (sectors)
	{
		for (i = 0; i <= 24; i++)
			if (craft_affect->a.craft->subcraft->sectors[i] - 1 ==
				ch->room->sector_type)
				pass = true;
		if (!pass)
		{
			send_to_char
				("That craft cannot be performed in this sort of terrain.\n", ch);
			return;
		}
	}

	/** seasons **/
	pass = false;
	for (i = 0; i <= 5; i++)
		if (craft_affect->a.craft->subcraft->seasons[i])
			seasonchk = true;

	if (seasonchk)
	{
		for (i = 0; i <= 5; i++)
			if (craft_affect->a.craft->subcraft->seasons[i] - 1 ==
				time_info.season)
				pass = true;
		if (!pass)
		{
			send_to_char
				("That craft cannot be performed during this season.\n", ch);
			return;
		}
	}

	/** weather **/
	pass = false;
	for (i = 0; i <= 8; i++)
		if (craft_affect->a.craft->subcraft->weather[i])
			weatherchk = true;

	if (weatherchk)
	{
		for (i = 0; i <= 12; i++)
			if (craft_affect->a.craft->subcraft->weather[i] - 1 ==
				weather_info[ch->room->zone].state)
				pass = true;
		if (!pass)
		{
			send_to_char
				("That craft cannot be performed in this weather.\n", ch);
			return;
		}
	}

	/** other requirements **/
	if (missing_craft_items (ch, craft_affect))
		return;

	if (num_followers(ch) < craft_affect->a.craft->subcraft->followers)
	{
		send_to_char
			("You do not have enough followers.\n", ch);
		return;
	}

	craft_affect->a.craft->phase = NULL;

	player_log (ch, craft_affect->a.craft->subcraft->command,
		craft_affect->a.craft->subcraft->subcraft_name);

	activate_phase (ch, craft_affect);
}

void
	do_materials (CHAR_DATA * ch, char *argument, int cmd)
{
	int i, clan_flags = 0;
	int phase_num = 0;
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *craft;
	PHASE_DATA *phase;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];
	char *p;
	CLAN_DATA *clan_def;
	bool racial = false;
	bool first = true; // this is the first one in the list
	bool sectors = false;
	bool seasonchk = false;
	bool weatherchk = false;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char
			("For which craft did you wish to obtain a materials list?\n", ch);
		return;
	}

	for (craft = crafts;
		craft && str_cmp (craft->subcraft_name, buf); craft = craft->next)
		;

	if (!craft)
	{
		send_to_char ("No such craft.  Type 'crafts' for a listing.\n", ch);
		return;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if (!(af = get_affect (ch, i)))
			continue;
		if (af->a.craft->subcraft == craft)
			break;
	}

	if (i > CRAFT_LAST && IS_MORTAL (ch))
	{
		send_to_char
			("That craft is not one your character has knowledge of.\n", ch);
		return;
	}

	send_to_char ("\n", ch);

	sprintf (buf, "#6%s:#0 %s %s\n", craft->craft_name, craft->command,
		craft->subcraft_name);
	buf[2] = toupper (buf[2]);

	send_to_char (buf, ch);

	if (!IS_MORTAL (ch))
	{
		*buf = '\0';
		for (i = 0; i <= 24; i++)
		{
			if (craft->opening[i] > 0)
				sprintf (buf + strlen (buf), " %s", skills[craft->opening[i]]);
		}
		if (*buf)
		{
			sprintf (buf2, "#6Opening Craft For:#0%s\n", buf);
			send_to_char (buf2, ch);
		}
	}

	/** races **/
	for (i = 0; i <= 24; i++)
	{
		if (craft->race[i] > 0)
			racial = true;
	}
	if (racial)
	{
		sprintf (buf, "#6Available To:#0 ");
		for (i = 0; i <= 24; i++)
			if (craft->race[i]  > 0)
				sprintf (buf + strlen (buf), "%s ",
				lookup_race_variable (craft->race[i] - 1, RACE_NAME));
		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}

	/** clans **/
	if (craft->clans && *craft->clans)
	{
		sprintf (buf, "#6Required Clans:#0 ");
		p = craft->clans;
		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			clan_def = get_clandef (clan_name);
			if (!clan_def)
				continue;
			if (!first)
				sprintf (buf + strlen (buf), "                ");
			sprintf (buf + strlen (buf), "%-30s [%s]\n", clan_def->literal,
				get_clan_rank_name (clan_flags));
			first = false;
		}
		send_to_char (buf, ch);
	}

	/** sectors **/
	for (i = 0; i <= 24; i++)
	{
		if (craft->sectors[i] > 0)
			sectors = true;
	}
	if (sectors)
	{
		sprintf (buf, "#6Only Usable In:#0 ");
		for (i = 0; i <= 24; i++)
			if (craft->sectors[i])
				sprintf (buf + strlen (buf), "%s ",
				sector_types[craft->sectors[i] - 1]);
		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}

	/** seasons **/
	for (i = 0; i <= 5; i++)
	{
		if (craft->seasons[i] > 0)
			seasonchk = true;
	}
	if (seasonchk)
	{
		sprintf (buf, "#6Usable During:#0 ");
		for (i = 0; i <= 5; i++)
			if (craft->seasons[i])
				sprintf (buf + strlen (buf), "%s ", seasons[craft->seasons[i] - 1]);
		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}

	/** group **/
	if (craft->followers > 0)
	{
		snprintf (buf, MAX_STRING_LENGTH,
			"#6Requires#0 %d followers.\n",
			craft->followers);
		send_to_char (buf, ch);
	}

	/**weather*/
	for (i = 0; i <= 8; i++)
	{
		if (craft->weather[i] > 0)
			weatherchk = true;
	}
	if (weatherchk)
	{
		sprintf (buf, "#6Usable During:#0 ");
		for (i = 0; i <= 8; i++)
			if (craft->weather[i])
				sprintf (buf + strlen (buf),
				"%s ",
				weather_states[craft->weather[i] - 1]);

		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}

	/** Phases information **/
	for (phase = craft->phases; phase; phase = phase->next)
	{

		phase_num++;

		sprintf (buf, "#5Phase %d:#0  %d seconds",
			phase_num, phase->phase_seconds);



		if ( phase->skill  > 0)
		{
			int x = craft_roll_max(phase->dice, phase->sides);
			snprintf (buf + strlen (buf), MAX_STRING_LENGTH,
				"%s %s %s skill required.#0\n",
				(ch->skills[phase->skill] >= x ? "#6" : ""),
				(x >= 110 ? "Master" : x >= 90 ? "Adroit" : x >= 60 ? "Familiar" : x >= 40 ? "Amateur" : "Beginner"),
				skills[phase->skill]);
		}
		else if ( phase->attribute > -1 )
		{
			snprintf (buf + strlen (buf), MAX_STRING_LENGTH,
				", %s attribute tested.\n",
				attrs[phase->attribute]);
		}
		else
		{
			snprintf (buf + strlen (buf), MAX_STRING_LENGTH, "\n");
		}

		send_to_char (buf, ch);

		if (phase->text)
		{
			snprintf (buf, MAX_STRING_LENGTH,  "%s\n",
				phase->text);
			send_to_char (buf, ch);
		}

		if (GET_TRUST(ch) && phase->phase_start_prog)
		{
			snprintf (buf, MAX_STRING_LENGTH,  "Phase Start Prog Command: %s\n",
				phase->phase_start_prog);
			send_to_char (buf, ch);
		}

		if (phase->tool != nullptr)
        missing_item_msg (ch, phase->tool, "Tool required:  ");

		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{
			if (craft->obj_items[i] &&
				craft->obj_items[i]->phase == phase &&
				!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED) &&
				!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE))
			{
				if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_HELD))
					missing_item_msg (ch, craft->obj_items[i],
					"Held (Reusable):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_IN_ROOM))
					missing_item_msg (ch, craft->obj_items[i],
					"In Room (Reusable):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED))
					missing_item_msg (ch, craft->obj_items[i],
					"Held or in Room (Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED_HELD))
					missing_item_msg (ch, craft->obj_items[i],
					"Held only (Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED_ROOM))
					missing_item_msg (ch, craft->obj_items[i],
					"Room only (Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED))
					missing_item_msg (ch, craft->obj_items[i],
					"Held or in Room (Progressed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_HELD))
					missing_item_msg (ch, craft->obj_items[i],
					"Held only (Progressed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_ROOM))
					missing_item_msg (ch, craft->obj_items[i],
					"Room only (Progressed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED))
					missing_item_msg (ch, craft->obj_items[i],
					"Held or in Room (Partially Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED_HELD))
					missing_item_msg (ch, craft->obj_items[i],
					"Held only (Partially Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED_ROOM))
					missing_item_msg (ch, craft->obj_items[i],
					"Room only (Partially Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_OPTIONAL))
					missing_item_msg (ch, craft->obj_items[i],
					"Held or in Room (Optionally Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_OPTIONAL_HELD))
					missing_item_msg (ch, craft->obj_items[i],
					"Held only (Optionally Consumed):  ");
				else if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_OPTIONAL_ROOM))
					missing_item_msg (ch, craft->obj_items[i],
					"Room only (Optionally Consumed):  ");
			}
			else if (craft->obj_items[i] &&
				craft->obj_items[i]->phase == phase &&
				IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED))
				missing_item_msg (ch, craft->obj_items[i],
				"#6Produced:#0  ");
			else if (craft->obj_items[i] &&
				craft->obj_items[i]->phase == phase &&
				IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE))
				missing_item_msg (ch, craft->obj_items[i],
				"#6Produced-to-hand:#0  ");
		}

		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{
			if (craft->fails[i] &&
				craft->fails[i]->phase == phase)
				missing_item_msg (ch, craft->fails[i], "#6Failure:#0  ");
		}

		if (phase->load_mob && vnum_to_mob (phase->load_mob))
		{
			sprintf (buf, "#6Produced:#0 #5%s#0\n",
				char_short (vnum_to_mob (phase->load_mob)));
			send_to_char (buf, ch);
		}

		if ( GET_TRUST (ch) )
		{
			if (phase->attribute > -1)
			{
				snprintf (buf, MAX_STRING_LENGTH,  "Attribute: %s vs %dd%d\n",
					attrs [phase->attribute],
					phase->dice,
					phase->sides);
				send_to_char (buf, ch);
			}

			if (phase->skill > 0)
			{
				snprintf (buf, MAX_STRING_LENGTH,  "Skill: %s vs %dd%d\n",
					skills[phase->skill],
					phase->dice,
					phase->sides);
				send_to_char (buf, ch);
			}

			if (GET_TRUST(ch) && phase->phase_end_prog)
			{
				snprintf (buf, MAX_STRING_LENGTH,  "Phase End Prog Command: %s\n",
					phase->phase_end_prog);
				send_to_char (buf, ch);
			}

			if (GET_TRUST(ch) && phase->phase_fail_prog)
			{
				snprintf (buf, MAX_STRING_LENGTH,  "Phase Fail Prog Command: %s\n",
					phase->phase_fail_prog);
				send_to_char (buf, ch);
			}



		}
	}

	if (craft->delay > 0)
	{
		sprintf (buf, "#6OOC Delay Timer:#0 %d RL Hours\n", craft->delay);
		send_to_char (buf, ch);
	}

	if (craft->faildelay > 0)
	{
		sprintf (buf, "#6OOC Failure Timer:#0 %d RL Hours\n", craft->faildelay);
		send_to_char (buf, ch);
	}

	if (craft->key_first > 0)
	{
		sprintf (buf, "#6Primary Key:#0 %d\n", craft->key_first);
		send_to_char (buf, ch);
	}

	if (craft->key_end > 0)
	{
		sprintf (buf, "#6Product Key:#0 %d\n", craft->key_end);
		send_to_char (buf, ch);
	}


}

void
	do_crafts (CHAR_DATA * ch, char *argument, int cmd)
{
	int has_a_craft = 0;
	int i = 0, j = 0;
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *craft, *tcraft;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	//char output[MAX_STRING_LENGTH];
	std::string output;
	char craft_name[MAX_STRING_LENGTH];
	char subcraft[MAX_STRING_LENGTH];
	char command[MAX_STRING_LENGTH];
	bool close = false;
	bool category = false;

	if (IS_NPC (ch))
	{
		send_to_char ("You shouldn't be using this command...\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	// listing craft catagories
	if (IS_MORTAL (ch))
	{
		if (!*buf)
		{
			output.assign("You currently have crafts in the following areas:\n\n");
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			{
				if ((af = get_affect (ch, i)))
				{
					sprintf(buf2, "  #6%-20s#0",
						af->a.craft->subcraft->craft_name);

					if (output.find(buf2, 0, strlen(buf2)) == std::string::npos)
					{
						j++;
						output.append(buf2);

						if (!(j % 3))
							output.append("\n");
					}

					has_a_craft = 1;
				}
			}//for (i = CRAFT_FIRST

			if ((j % 3))
				output.append("\n");

			if (!has_a_craft)
				send_to_char ("You have no knowledge of any such crafts.\n", ch);
			else
				page_string (ch->descr(), output.c_str());
		}//if (!*buf)

		//Listing individual crafts

		else
		{
			j = 0;
			if (!str_cmp (buf, "all"))
				output.assign("You know the following crafts:\n\n");
			else
			{
				output.assign ("You know the following ");
				sprintf(buf2, "#6%s#0", buf);
				output.append (buf2);
				output.append (" crafts:\n\n");
			}

			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			{
				if ((af = get_affect (ch, i)))
				{
					if (str_cmp (buf, "all")
						&& str_cmp (buf, af->a.craft->subcraft->craft_name))
						continue;

					j++;
					sprintf (name, "%s %s",
						af->a.craft->subcraft->command,
						af->a.craft->subcraft->subcraft_name);
					sprintf (buf2, "   #6%-30s#0", name);
					output.append(buf2);

					has_a_craft = 1;

					if (!(j % 2))
						output.append("\n");
				}
			}//for (i = CRAFT_FIRST
			if ((j % 2))

				output.append("\n");

			if (!has_a_craft)
				send_to_char ("You have no knowledge of any crafts.\n", ch);
			else
				page_string (ch->descr(), output.c_str());
		} //else (buf)
		return;
	} //if (IS_MORTAL (ch))

	// Immortal options - List
	if ((!*buf && !ch->pc->edit_craft) || !str_cmp (buf, "list"))
	{
		list_all_crafts (ch);
		return;
	}

	// Immortal options - new
	else if (!str_cmp (buf, "new"))
	{
		argument = one_argument (argument, buf);
		sprintf (craft_name, "%s", buf);
		argument = one_argument (argument, buf);
		sprintf (subcraft, "%s", buf);
		argument = one_argument (argument, buf);
		sprintf (command, "%s", buf);

		if (!*buf || !*craft_name || !*subcraft || !*command)
		{
			send_to_char
				("The syntax is as follows: craft new <craft> <subcraft> <command>\n", ch);
			return;
		}

		for (craft = crafts; craft; craft = craft->next)
		{
			if (!craft->next)
			{
				CREATE (craft->next, SUBCRAFT_HEAD_DATA, 1);
				craft = craft->next;
				break;
			}
		}

		craft->craft_name = add_hash (craft_name);
		craft->subcraft_name = add_hash (subcraft);
		craft->command = add_hash (command);
		craft->key_first = -1;
		craft->key_end = -1;
		ch->pc->edit_craft = craft;
		send_to_char ("New craft initialized and opened for editing.\n", ch);

		mysql_safe_query
			("INSERT INTO new_crafts VALUES ('%s', '%s', '%s', '%s')",
			craft->command,
			craft->subcraft_name,
			craft->craft_name,
			ch->tname);
		return;
	} //else if (!str_cmp (buf, "new"))

	// Immortal options - clone
	else if (!str_cmp (buf, "clone"))
	{
		if (!ch->pc->edit_craft)
		{
			send_to_char
				("You'll need to open the craft you want to clone, first.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
		if (!*buf)
		{
			send_to_char
				("You'll need to specify a craft name for the cloned craft.\n", ch);
			return;
		}
		else
			sprintf (craft_name, "%s", buf);

		argument = one_argument (argument, buf);
		if (!*buf)
		{
			send_to_char
				("You'll need to specify a subcraft name for the cloned craft.\n", ch);
			return;
		}
		else if (!str_cmp (buf, ch->pc->edit_craft->subcraft_name))
		{
			send_to_char
				("You'll need to specify a different subcraft name for the cloned craft.\n", ch);
			return;
		}
		else
			sprintf (subcraft, "%s", buf);

		argument = one_argument (argument, buf);
		if (!*buf)
		{
			send_to_char
				("You'll need to specify a command for the cloned craft.\n", ch);
			return;
		}
		else
			sprintf (command, "%s", buf);

		CREATE (tcraft, SUBCRAFT_HEAD_DATA, 1);
		memset (tcraft, 0, sizeof (SUBCRAFT_HEAD_DATA));
		memcpy (tcraft, ch->pc->edit_craft, sizeof (SUBCRAFT_HEAD_DATA));

		CREATE (tcraft->phases, PHASE_DATA, 1);
		memset (tcraft->phases, 0, sizeof (PHASE_DATA));
		memcpy (tcraft->phases, ch->pc->edit_craft->phases, sizeof (PHASE_DATA));

		if (ch->pc->edit_craft->obj_items)
		{
			for (i = 0; ch->pc->edit_craft->obj_items[i]; i++)
			{
				CREATE (tcraft->obj_items[i], DEFAULT_ITEM_DATA, 1);
				memset (tcraft->obj_items[i], 0,
					sizeof (DEFAULT_ITEM_DATA));
			}

			memcpy (tcraft->obj_items, ch->pc->edit_craft->obj_items,
				sizeof (DEFAULT_ITEM_DATA));
		}

		ch->pc->edit_craft = NULL;
		tcraft->craft_name = add_hash (craft_name);
		tcraft->subcraft_name = add_hash (subcraft);
		tcraft->command = add_hash (command);
		tcraft->next = NULL;

		for (craft = crafts; craft; craft = craft->next)
		{
			if (!craft->next)
			{
				CREATE (craft->next, SUBCRAFT_HEAD_DATA, 1);
				craft->next = tcraft;
				ch->pc->edit_craft = tcraft;
				send_to_char ("Craft cloned; new craft opened for editing.\n", ch);

				if (!IS_SET (tcraft->subcraft_flags, SCF_OBSCURE))
					mysql_safe_query ("INSERT INTO new_crafts VALUES 									('%s', '%s', '%s', '%s')",
					tcraft->command,
					tcraft->subcraft_name,
					tcraft->craft_name,
					ch->tname);

				return;
			}
		}
	} //else if (!str_cmp (buf, "clone"))

	//Immortal options - list by catagory
	for (craft = crafts; craft; craft = craft->next)
	{
		if (!str_cmp (craft->subcraft_name, buf))
			break;

		if (!str_cmp (craft->craft_name, buf))
		{
			category = true;
			break;
		}
	}

	if (category)
	{

		sprintf (buf2, "\nWe have the following #6%s#0 crafts:\n\n", buf);
		output.assign(buf2);
		for (craft = crafts; craft; craft = craft->next)
		{
			if (!str_cmp (craft->craft_name, buf))
			{
				sprintf (buf2,
					"#6Subcraft:#0 %-24s #6Command:#0 %-20s\n",
					craft->subcraft_name,
					craft->command);
				output.append(buf2);
			}
		}

		page_string (ch->descr(), output.c_str());
		return;
	}


	//Immortal options - edit craft
	if (!craft && *buf && !category)
		close = true;

	if (ch->pc->edit_craft)
		craft = ch->pc->edit_craft;

	if (!craft && !category)
	{
		send_to_char ("No such subcraft.\n", ch);
		return;
	}

	if (!ch->pc->edit_craft || ch->pc->edit_craft != craft)
	{
		ch->pc->edit_craft = craft;
		send_to_char ("Craft has been opened for editing.\n", ch);
		return;
	}
	else if (ch->pc->edit_craft && close)
	{
		send_to_char ("Craft closed and written to memory.\n", ch);
		ch->pc->edit_craft = NULL;
		return;
	}

	*b_buf = '\0';

	//Immortal options - display craft with in edit
	craftstat (ch, craft->subcraft_name);




	return;
}

void
	do_remcraft (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_affect;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH];
	bool craft_category = false;

	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		s ("Start by using the mob command on a PC.");
		s ("");
		s ("remcraft <subcraft-name>            Use crafts to get names");
		s ("");
		s ("   remcraft swordblank ");
		s ("");
		return;
	}

	if (IS_NPC (ch))
	{
		send_to_char ("This command cannot be used while switched.\n", ch);
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		send_to_char ("Start by using the MOB command on a PC.\n", ch);
		return;
	}

	for (craft = crafts; craft; craft = craft->next)
	{
		if (!str_cmp (craft->subcraft_name, buf))
			break;
		if (!str_cmp (craft->craft_name, buf))
		{
			craft_category = true;
			break;
		}
	}

	if (!str_cmp (buf, "all"))
		craft_category = true;

	if (!craft && !craft_category)
	{
		send_to_char ("No such craft.  Type 'crafts' for a listing.\n", ch);
		return;
	}

	for (af = edit_mob->hour_affects; af; af = next_affect)
	{

		next_affect = af->next;

		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		{
			if (!str_cmp (buf, "all")
				|| !str_cmp (af->a.craft->subcraft->craft_name, buf))
			{
				remove_affect_type (edit_mob, af->type);
				continue;
			}
			if (!str_cmp (af->a.craft->subcraft->subcraft_name, buf))
			{
				remove_affect_type (edit_mob, af->type);
				send_to_char ("Ok.\n", ch);
				break;
			}
		}
	}

	if (craft_category)
		send_to_char ("Craft category removed.\n", ch);
}

void
	do_addcraft (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af, *next_affect;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH];
	bool craft_category = false;

	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		s ("Start by using the mob command on a PC.");
		s ("");
		s ("addcraft <subcraft-name>            Use crafts to get names");
		s ("         <item list 1>,");
		s ("         <item list 2>, ...");
		s ("");
		s ("<item list> is item/flag or (item/flag item/flag ...)");
		s ("");
		s ("Example:");
		s ("                       v-no item list 1 specified");
		s ("   addcraft swordblank , (- 10001 10002), 10003");
		s ("             item list 2-^^^^^^^^^^^^^^^  ^^^^^-item list 3");
		s ("");
		return;
	}

	if (IS_NPC (ch))
	{
		send_to_char ("This command cannot be used while switched.\n", ch);
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		send_to_char ("Start by using the MOB command on a PC.\n", ch);
		return;
	}

	for (craft = crafts; craft; craft = craft->next)
	{
		if (!str_cmp (craft->subcraft_name, buf))
			break;
		if (!str_cmp (craft->craft_name, buf))
			craft_category = true;
	}

	if (!str_cmp (buf, "all"))
		craft_category = true;

	if (!craft && !craft_category)
	{
		send_to_char
			("No such craft or subcraft.  Type 'crafts' for a listing.\n", ch);
		return;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		if (!get_affect (edit_mob, i))
			break;

	if (i > CRAFT_LAST)
	{
		send_to_char ("Sorry, this PC already has the maximum allowed crafts"
			".\n", ch);
		return;
	}

	if (craft_category)
	{
		for (af = edit_mob->hour_affects; af; af = next_affect)
		{
			next_affect = af->next;
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{
				if (!str_cmp (af->a.craft->subcraft->craft_name, buf))
					remove_affect_type (edit_mob, af->type);
			}
		}
		for (craft = crafts; craft; craft = craft->next)
		{
			if (!strcmp ("all", buf) || !strcmp (craft->craft_name, buf))
			{
				for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
					if (!get_affect (edit_mob, i))
						break;
				magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
				af = get_affect (edit_mob, i);
				af->a.craft =
					(struct affect_craft_type *)
					alloc (sizeof (struct affect_craft_type), 23);
				af->a.craft->subcraft = craft;
			}
		}
	}
	else
	{
		magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
		af = get_affect (edit_mob, i);
		af->a.craft =
			(struct affect_craft_type *) alloc (sizeof (struct affect_craft_type),
			23);
		af->a.craft->subcraft = craft;
	}

	send_to_char ("Craft(s) added.\n", ch);
}

void
	update_crafts (CHAR_DATA * ch)
{
	SUBCRAFT_HEAD_DATA *craft;
	AFFECTED_TYPE *af;
	int i = 0;

	for (craft = crafts; craft; craft = craft->next)
	{
		if (((!strcmp (craft->craft_name, "general-fire")) ||
			(!strcmp (craft->craft_name, "general-labor")) ||
			(!strcmp (craft->craft_name, "general-food")) ||
			(!strcmp (craft->craft_name, "general-armor")) ||
			(!strcmp (craft->craft_name, "general-weapons")) ||
			(!strcmp (craft->craft_name, "general-recreation")) ||
			(!strcmp (craft->craft_name, "general-serving")) ||
			(!strcmp (craft->craft_name, "general")))
			&& !has_craft (ch, craft)
			&& has_required_crafting_skills (ch, craft))
		{
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
				if (!get_affect (ch, i))
					break;
			magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			af = get_affect (ch, i);
			af->a.craft =
				(struct affect_craft_type *)
				alloc (sizeof (struct affect_craft_type), 23);
			af->a.craft->subcraft = craft;
		}
		if (is_opening_craft (ch, craft)
			&& has_required_crafting_skills (ch, craft)
			&& !has_craft (ch, craft))
		{
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
				if (!get_affect (ch, i))
					break;
			magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			af = get_affect (ch, i);
			af->a.craft =
				(struct affect_craft_type *)
				alloc (sizeof (struct affect_craft_type), 23);
			af->a.craft->subcraft = craft;
		}
	}
}

AFFECTED_TYPE *
	is_crafting (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af, *next_af;

	for (af = ch->hour_affects; af; af = next_af)
	{
		next_af = af->next;
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
			&& af->a.craft->timer)
		{
			return af;
		}
	}

	return NULL;
}

/* Checks clan requirements for craft branch */

int
	meets_clan_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	char clan_name[MAX_STRING_LENGTH];
	char *c1;
	int flags = 0, flags2 = 0;
	bool required_clan = false;

	if (craft->clans && strlen (craft->clans) > 3)
	{
		required_clan = true;
		for (c1 = craft->clans; get_next_clan (&c1, clan_name, &flags);)
		{
			if (get_clan (ch, clan_name, &flags2) && flags2 >= flags)
				required_clan = false;
		}
	}

	if (required_clan)
		return 0;

	return 1;
}

/* Checks to see if race requirements for craft branch are met */

int
	meets_race_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	int i;
	bool required_race = false;

	for (i = 0; i <= 24; i++)
	{
		if (craft->race[i])
			required_race = true;
	}

	if (required_race)
	{
		for (i = 0; i <= 24; i++)
		{
			if (craft->race[i] && ch->race == craft->race[i] - 1)
				required_race = false;
		}
	}

	if (required_race)
		return 0;

	return 1;
}


/* Checks to see if they meet the skill req's for branching craft */

int
	meets_skill_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;

	if (!meets_race_requirements (ch, craft))
		return 0;

	if (!meets_clan_requirements (ch, craft))
		return 0;

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->req_skill)
		{
			if (!real_skill (ch, phase->req_skill))
				return 0;
		}
		if (phase->skill)
		{
			if (!real_skill (ch, phase->skill)
				|| ch->skills[phase->skill] <
				((int) ((phase->dice * phase->sides) * 0.660)))
				return 0;
		}
	}

	return 1;
}

/* Makes sure PC has all absolute baseline specs necessary for a given craft */

int
	has_required_crafting_skills (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int i = 0;

	if (!meets_race_requirements (ch, craft))
		return 0;

	if (!meets_clan_requirements (ch, craft))
		return 0;

	for (i = 0; i <= 24; i++)
	{
		if (craft->opening[i] > 0 && (real_skill (ch, craft->opening[i]) < 10))
			return 0;
	}

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->req_skill)
		{
			if (!real_skill (ch, phase->req_skill))
				return 0;
		}
		if (phase->skill)
		{
			if (!real_skill (ch, phase->skill))
				return 0;
		}
	}

	return 1;

}

int
	is_opening_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	int i = 0;

	for (i = 0; i <= 24; i++)
	{
		if (craft->opening[i] > 0 && (real_skill (ch, craft->opening[i]) >= 10))
			return 1;
	}

	return 0;

}

char *
	craft_argument (char *argument, char *arg_first)
{
	char cEnd;

	if (argument == NULL)
		return "";

	while (isspace (*argument))
		argument++;

	if (*argument == '(')
	{
		argument++;
		strcpy (arg_first, "(");
		return argument;
	}

	if (*argument == ')')
	{
		argument++;
		strcpy (arg_first, ")");
		return argument;
	}

	cEnd = ' ';

	while (*argument != '\0')
	{

		if (*argument == '(' || *argument == ')')
			break;

		if (*argument == cEnd)
		{
			argument++;
			break;
		}

		if (cEnd == ' ')
			*arg_first = tolower (*argument);
		else
			*arg_first = *argument;

		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace (*argument))
		argument++;

	return argument;
}



void
	add_to_default_ovals_list (CRAFT_OVAL_DATA * ovals, char *flag_vnum, int i)
{
	if (*flag_vnum == '(' || *flag_vnum == ')')
		return;

	if (i == 1)
	{
		ovals->from = atoi(flag_vnum);
		return;
	}

	if (i == 2)
	{
		ovals->to = atoi(flag_vnum);
		return;
	}

	if (i == 3)
	{
		ovals->oval = atoi(flag_vnum);
		return;
	}
}

void
	add_to_default_variable_list (CRAFT_VARIABLE_DATA * vars, char *flag_vnum, int i)
{
	if (*flag_vnum == '(' || *flag_vnum == ')')
		return;

	if (vc_category(flag_vnum))
	{
		vars->category = str_dup(flag_vnum);
		return;
	}

	if (i == 1)
	{
		vars->from = atoi(flag_vnum);
		return;
	}

	if (i == 2)
	{
		vars->pos = atoi(flag_vnum);
		return;
	}

	if (i == 3)
	{
		vars->to = atoi(flag_vnum);
		return;
	}
}


void
	add_to_default_list (DEFAULT_ITEM_DATA * items, char *flag_vnum)
{
	int ind;
	int ovnum;
	int i;
	char buf[MAX_STRING_LENGTH];

	if (!items->item_counts)
		items->item_counts = 1;

	if (*flag_vnum == '(' || *flag_vnum == ')')
		return;

	/* '-' means to erase all vnums */

	if (*flag_vnum == '-')
	{
		memset (items->items, 0, MAX_DEFAULT_ITEMS);
		return;
	}

	if ((*flag_vnum == 'x' || *flag_vnum == 'X') && isdigit (flag_vnum[1]))
	{
		sprintf (buf, "%c", flag_vnum[1]);
		if (flag_vnum[2] && isdigit (flag_vnum[2]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[2]);
		if (flag_vnum[3] && isdigit (flag_vnum[3]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[3]);
		if (flag_vnum[4] && isdigit (flag_vnum[4]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[4]);
		items->item_counts = atoi (buf);
		return;
	}

	if (!isdigit (*flag_vnum))
	{
		if ((ind = index_lookup (item_default_flags, flag_vnum)) == -1)
		{
			;
		}
		else
			items->flags |= (1 << ind);

		return;
	}

	ovnum = atoi (flag_vnum);

	if (!ovnum || !vtoo (ovnum))
	{
		sprintf (buf, "NOTE:  vnum %s does not exist for CRAFTS!", flag_vnum);
		system_log (buf, true);
	}

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{

		if (!items->items[i])
		{
			items->items[i] = ovnum;
			break;
		}

		/** removed to allow multiple items ****
		if (items->items[i] == ovnum)
		{
		break;
		}
		*****/
	}

	if (i >= MAX_DEFAULT_ITEMS)
	{
		system_log ("WARNING:  Too many items specified in default item list!",
			true);
		sprintf (buf, "Item %d not added.  Max allowed in list is %d items.",
			ovnum, MAX_DEFAULT_ITEMS);
		system_log (buf, true);
	}
}

char *
	read_item_list (DEFAULT_ITEM_DATA ** items, char *list, PHASE_DATA * phase)
{
	char buf[MAX_STRING_LENGTH];
	char *argument;
	DEFAULT_ITEM_DATA *deflt;

	*items = (DEFAULT_ITEM_DATA *) alloc (sizeof (DEFAULT_ITEM_DATA), 27);

	deflt = *items;

	deflt->phase = phase;

	argument = craft_argument (list, buf);

	if (!*buf)
		return argument;

	if (*buf != '(')
	{
		add_to_default_list (deflt, buf);
		return argument;
	}

	do
	{
		argument = craft_argument (argument, buf);
		add_to_default_list (deflt, buf);
	}
	while (*buf != ')');

	return argument;
}



char *
	read_ovals_list (CRAFT_OVAL_DATA ** ovals, char *list, PHASE_DATA * phase)
{
	char buf[MAX_STRING_LENGTH];
	char *argument;
	int i = 0;
	CRAFT_OVAL_DATA *deflt;

	*ovals = (CRAFT_OVAL_DATA *) alloc (sizeof (CRAFT_OVAL_DATA), 27);

	deflt = *ovals;

	deflt->phase = phase;

	argument = craft_argument (list, buf);

	if (!*buf)
		return argument;

	if (*buf != '(')
	{
		add_to_default_ovals_list (deflt, buf, i);
		return argument;
	}

	do
	{
		i++;
		argument = craft_argument (argument, buf);
		add_to_default_ovals_list (deflt, buf, i);		
	}
	while (*buf != ')');

	return argument;
}

char *
	read_variable_list (CRAFT_VARIABLE_DATA ** vars, char *list, PHASE_DATA * phase)
{
	char buf[MAX_STRING_LENGTH];
	char *argument;
	int i = 0;
	CRAFT_VARIABLE_DATA *deflt;

	*vars = (CRAFT_VARIABLE_DATA *) alloc (sizeof (CRAFT_VARIABLE_DATA), 27);

	deflt = *vars;

	deflt->phase = phase;

	argument = craft_argument (list, buf);

	if (!*buf)
		return argument;

	if (*buf != '(')
	{
		add_to_default_variable_list (deflt, buf, i);
		return argument;
	}

	do
	{
		argument = craft_argument (argument, buf);
		add_to_default_variable_list (deflt, buf, i);
		i++;
	}
	while (*buf != ')');

	return argument;
}

PHASE_DATA *
	new_phase ()
{
	PHASE_DATA *phase;

	phase = (PHASE_DATA *) alloc (sizeof (PHASE_DATA), 26);
	phase->attribute = -1;

	return phase;
}

char *
	read_extended_text (FILE * fp, char *first_line)
{
	int continues;
	char line[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char *buf_end = '\0';
	char *buf_start = '\0';

	*buf = '\0';
	*line = '\0';

	strcpy (buf, first_line);

	*line = '\0';

	while (1)
	{

		continues = 0;

		for (buf_start = buf; *buf_start == ' ';)
			buf_start++;

		buf_end = buf_start + strlen (buf_start) - 1;

		while (1)
		{

			if (buf_end == buf_start)
				break;

			if (*buf_end == ' ' || *buf_end == '\n' || *buf_end == '\r')
				buf_end--;

			else if (*buf_end == '\\')
			{
				continues = 1;
				*buf_end = '\0';
				break;
			}

			else
				break;

			buf_end[1] = '\0';
		}

		strcat (line, buf_start);

		if (!continues)
			break;

		fgets (buf, MAX_STRING_LENGTH - 1, fp);
	}

	return str_dup (line);
}

void
	subcraft_line (FILE * fp_reg, char *line)
{
	int ind;
	int items_num;
	int variable_num, oval_num;
	int ski_att, i;
	char c[10];
	char *argument = '\0';
	char buf[MAX_STRING_LENGTH];
	PHASE_DATA *phase = NULL;
	SUBCRAFT_HEAD_DATA *subcraft;


	argument = one_argument (line, buf);

	if (!str_cmp (buf, "craft"))
	{
		if (!crafts)
		{
			crafts = subcraft =
				(SUBCRAFT_HEAD_DATA *) alloc (sizeof (SUBCRAFT_HEAD_DATA), 25);
			memset (crafts, 0, sizeof (SUBCRAFT_HEAD_DATA));
		}
		else
		{
			for (subcraft = crafts; subcraft->next; subcraft = subcraft->next)
				;

			subcraft->next =
				(SUBCRAFT_HEAD_DATA *) alloc (sizeof (SUBCRAFT_HEAD_DATA), 25);
			subcraft = subcraft->next;
			memset (subcraft, 0, sizeof (SUBCRAFT_HEAD_DATA));
		}

		subcraft->crafts_start = ftell (fp_reg) - strlen (line) - 1;

		subcraft->clans = add_hash ("");

		argument = one_argument (argument, buf);
		subcraft->craft_name = str_dup (buf);

		argument = one_argument (argument, buf);

		if (str_cmp (buf, "subcraft"))
		{
			system_log ("Missing 'subcraft' on craft line:", true);
			system_log (line, true);
			system_log (buf, true);
			abort ();
		}

		argument = one_argument (argument, buf);
		subcraft->subcraft_name = str_dup (buf);

		argument = one_argument (argument, buf);

		if (str_cmp (buf, "command"))
		{
			system_log ("Missing 'command' on craft line:", true);
			system_log (line, true);
			system_log (buf, true);
			abort ();
		}

		argument = one_argument (argument, buf);
		subcraft->command = str_dup (buf);

		return;
	}

	if (!crafts)
	{
		system_log ("Missing first 'craft' line.", true);
		system_log (line, true);
		abort ();
	}


	/* Determine current subcraft */

	for (subcraft = crafts; subcraft->next; subcraft = subcraft->next)
		;

	if (!subcraft->phases &&
		(!str_cmp (buf, "fail:") || !str_cmp (buf, "failure:")))
	{
		subcraft->failure = read_extended_text (fp_reg, argument);
		return;
	}

	if (!subcraft->phases &&
		(!str_cmp (buf, "failobj:") || !str_cmp (buf, "failobjs:")))
	{
		subcraft->failobjs = read_extended_text (fp_reg, argument);
		return;
	}
	if (!subcraft->phases &&
		(!str_cmp (buf, "failmob:") || !str_cmp (buf, "failmobs:")))
	{
		subcraft->failmobs = read_extended_text (fp_reg, argument);
		return;
	}

	if (!subcraft->phases && !str_cmp (buf, "flags:"))
	{

		argument = one_argument (argument, buf);

		while (*buf)
		{

			if ((ind = index_lookup (subcraft_flags, buf)) != -1)
				subcraft->subcraft_flags |= (1 << ind);
			else
			{
				system_log ("Subcraft_flags error (line, subcraft):", true);
				system_log (line, true);
				system_log (subcraft->subcraft_name, true);
			}

			argument = one_argument (argument, buf);
		}

		if (IS_SET (subcraft->subcraft_flags, SCF_OFFENSIVE) &&
			IS_SET (subcraft->subcraft_flags, SCF_DEFENSIVE))
		{
			subcraft->subcraft_flags &= ~SCF_DEFENSIVE;
		}

		if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_OBJ) &&
			IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR))
		{
			subcraft->subcraft_flags &= ~SCF_DEFENSIVE;
		}

		return;
	} //flags

	else if (!str_cmp (buf, "sectors:"))
	{

		if (!*argument)
			return;

		argument = one_argument (argument, buf);

		while (*buf)
		{

			if ((ind = index_lookup (sector_types, buf)) != -1)
			{
				for (i = 0; i <= 24; i++)
				{
					if (!subcraft->sectors[i])
					{
						subcraft->sectors[i] = ind + 1;
						break;
					}
				}
			}
			else
			{
				system_log ("Illegal subcraft sector:", true);
				system_log (buf, true);
			}

			argument = one_argument (argument, buf);
		}
		return;
	}//sectors

	else if (!str_cmp (buf, "seasons:"))
	{

		if (!*argument)
			return;

		argument = one_argument (argument, buf);

		while (*buf)
		{
			if ((ind = index_lookup (seasons, buf)) != -1)
			{
				for (i = 0; i <= 5; i++)
				{
					if (!subcraft->seasons[i])
					{
						subcraft->seasons[i] = ind + 1;
						break;
					}
				}
			}
			argument = one_argument (argument, buf);
		}
		return;
	}//seasons

	else if (!str_cmp (buf, "weather:"))
	{
		if (!*argument)
			return;

		argument = one_argument (argument, buf);
		while (*buf)
		{
			if ((ind = index_lookup (weather_states, buf)) != -1)
			{
				for (i = 0; i <= 8; i++)
				{
					if (!subcraft->weather[i])
					{
						subcraft->weather[i] = ind + 1;
						break;
					}
				}
			}
			argument = one_argument (argument, buf);
		}
		return;
	}

	else if (!str_cmp (buf, "opening:"))
	{

		argument = one_argument (argument, buf);

		while (*buf)
		{

			if (atoi (buf) <= LAST_SKILL)
			{
				for (i = 0; i <= 24; i++)
				{
					if (!subcraft->opening[i])
					{
						subcraft->opening[i] = atoi (buf);
						break;
					}
				}
			}
			argument = one_argument (argument, buf);
		}
		return;
	}//opening skills

	else if (!str_cmp (buf, "race:"))
	{
		argument = one_argument (argument, buf);
		while (*buf)
		{
			for (i = 0; i <= 24; i++)
			{
				if (!subcraft->race[i])
				{
					subcraft->race[i] = atoi (buf);
					break;
				}
			}
			argument = one_argument (argument, buf);
		}
		return;
	}//race

	else if (!str_cmp (buf, "clans:"))
	{

		subcraft->clans = add_hash (argument);

		return;
	}

	else if (!str_cmp (buf, "ic_delay:"))
	{
		argument = one_argument (argument, buf);
		subcraft->delay = atoi (buf);
		return;
	}

	else if (!str_cmp (buf, "fail_delay:"))
	{
		argument = one_argument (argument, buf);
		subcraft->faildelay = atoi (buf);
		return;
	}

	else if (!str_cmp (buf, "start_key:"))
	{
		argument = one_argument (argument, buf);
		subcraft->key_first = atoi (buf);
		return;
	}

	else if (!str_cmp (buf, "end_key:"))
	{
		argument = one_argument (argument, buf);
		subcraft->key_end = atoi (buf);
		return;
	}

	else if (!str_cmp (buf, "followers:"))
	{
		argument = one_argument (argument, buf);
		subcraft->followers = atoi (buf);
		return;
	}

	if (!str_cmp (buf, "phase") || !str_cmp (buf, "phase:"))
	{
		if (!subcraft->phases)
			subcraft->phases = new_phase ();
		else
		{
			for (phase = subcraft->phases; phase->next; phase = phase->next)
				;

			phase->next = new_phase ();
		}

		return;
	}

	/* Determine current phase */

	if (subcraft->phases)
	{
		for (phase = subcraft->phases; phase->next; phase = phase->next)
			;
	}

	while (*argument == ' ')
		argument++;

	if (!str_cmp (buf, "end"))
		subcraft->crafts_end = ftell (fp_reg);

	else if (!str_cmp (buf, "1st:"))
		phase->first = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "2nd:"))
		phase->second = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "3rd:"))
		phase->third = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "self:"))
		phase->self = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "1stfail:") || !str_cmp (buf, "failure:"))
		phase->failure = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "2ndfail:"))
		phase->second_failure = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "3rdfail:"))
		phase->third_failure = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "group:"))
		phase->group_mess = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "groupfail:"))
		phase->fail_group_mess = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "f:"))
	{
		argument = one_argument (argument, buf);
		while (*buf)
		{
			if ((ind = index_lookup (phase_flags, buf)) != -1)
				phase->flags |= (1 << ind);
			else
			{
				system_log ("Illegal subcraft flags:", true);
				system_log (buf, true);
			}
			argument = one_argument (argument, buf);
		}
	}

	else if ( !str_cmp (buf, "s:") || !str_cmp (buf, "skill:") )
	{
		argument = one_argument (argument, buf);
		if ( (ski_att = index_lookup (skills, buf)) == -1 )
		{
			system_log ("Illegal skill name in craft:", true);
			system_log (buf, true);
		}

		else
			phase->skill = ski_att;

		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "vs") || !str_cmp (buf, "vs.") )
			argument = one_argument (argument, buf);

		if ( 3 != (sscanf (buf, "%d%[dD]%d", &phase->dice, c, &phase->sides)))
		{
			system_log ("Parm error in skill: (s:) of craft:", true);
			system_log (buf, true);
		}

		if ( phase->skill && (phase->dice < 1 || phase->sides < 1) )
		{
			phase->dice = 1;
			phase->sides = 100;
		}
	}//skill

	else if ( !str_cmp (buf, "a:") || !str_cmp (buf, "attr:") )
	{
		argument = one_argument (argument, buf);
		if ( (ski_att = index_lookup (attrs, buf)) == -1 )
		{
			system_log ("Illegal attribute name in craft:", true);
			system_log (buf, true);
		}

		else
			phase->attribute = ski_att;

		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "vs") || !str_cmp (buf, "vs.") )
			argument = one_argument (argument, buf);

		if ( 3 != (sscanf (buf, "%d%[dD]%d", &phase->dice, c, &phase->sides)))
		{
			system_log ("Parm error in attribute: (a:) of craft:", true);
			system_log (buf, true);
		}

		if ( phase->attribute > -1  && (phase->dice < 1 || phase->sides < 1) )
		{
			phase->dice = 1;
			phase->sides = 100;
		}
	}//end attri

	else if (!str_cmp (buf, "t:"))
	{
		argument = one_argument (argument, buf);

		if (isdigit (*buf))
			phase->phase_seconds = atoi (buf);
		else
		{
			system_log ("Illegal t: (time) value declared in subcraft:", true);
			system_log (buf, true);
		}
	}

	else if (!str_cmp (buf, "Fail"))
	{
		argument = one_argument (argument, buf);


		if ((isdigit (*buf) && buf[1] == ':') ||
			(isdigit (*buf) && isdigit (buf[1]) && buf[2] == ':'))
		{

			items_num = atoi (buf);
			if (subcraft->fails[items_num])
			{
				abort ();
			}

			read_item_list (&subcraft->fails[atoi (buf)], argument, phase);
		}
	}

	else if (!str_cmp (buf, "mob:"))
	{

		argument = one_argument (argument, buf);

		if (isdigit (*buf))
			phase->load_mob = atoi (buf);

		argument = one_argument (argument, buf);

		while (*buf)
		{

			if ((ind = index_lookup (craft_mobile_flags, buf)) != -1)
				phase->nMobFlags |= (1 << ind);
			else
			{
				system_log ("Illegal subcraft flags:", true);
				system_log (buf, true);
			}

			argument = one_argument (argument, buf);
		}
	}

	else if (!str_cmp (buf, "tool:"))
		read_item_list (&phase->tool, argument, phase);

	else if ((isdigit (*buf) && buf[1] == ':') ||
		(isdigit (*buf) && isdigit (buf[1]) && buf[2] == ':'))
	{

		items_num = atoi (buf);
		if (subcraft->obj_items[items_num])
		{
			abort ();
		}

		read_item_list (&subcraft->obj_items[atoi (buf)], argument, phase);
	}

	else if ((isdigit (*buf) && buf[1] == ';') ||
		(isdigit (*buf) && isdigit (buf[1]) && buf[2] == ';'))
	{

		variable_num = atoi (buf);
		if (subcraft->craft_variable[variable_num])
		{
			abort ();
		}

		read_variable_list (&subcraft->craft_variable[atoi (buf)], argument, phase);
	}

	else if ((isdigit (*buf) && buf[1] == '+') ||
		(isdigit (*buf) && isdigit (buf[1]) && buf[2] == '+'))
	{

		oval_num = atoi (buf);
		if (subcraft->craft_oval[oval_num])
		{
			abort ();
		}

		read_ovals_list (&subcraft->craft_oval[atoi (buf)], argument, phase);
	}

	else if (!str_cmp (buf, "cost:"))
	{
		argument = one_argument (argument, buf);

		while (*buf)
		{

			if (!str_cmp (buf, "hits"))
			{
				argument = one_argument (argument, buf);
				if (!just_a_number (buf))
				{
					break;
				}

				phase->hit_cost = atoi (buf);
			}

			else if (!str_cmp (buf, "moves"))
			{
				argument = one_argument (argument, buf);
				if (!just_a_number (buf))
				{
					break;
				}

				phase->move_cost = atoi (buf);
			}

			argument = one_argument (argument, buf);
		}
	}

	else if (!str_cmp (buf, "startpprog:"))
		phase->phase_start_prog = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "endpprog:"))
		phase->phase_end_prog = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "failpprog:"))
		phase->phase_fail_prog = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "text:"))
		phase->text = read_extended_text (fp_reg, argument);

	else if (!str_cmp (buf, "spell:"))
		read_spell (phase, argument);

	else if (!str_cmp (buf, "open:"))
	{

		if (phase->open_skill)
			;

		argument = one_argument (argument, buf);

		if (!str_cmp (buf, "target"))
		{
			phase->flags &= ~PHASE_OPEN_ON_SELF;
			argument = one_argument (argument, buf);
		}
		else
			phase->flags |= PHASE_OPEN_ON_SELF;

		if ((ski_att = index_lookup (skills, buf)) == -1)
		{
			;
		}
		else
			phase->open_skill = ski_att;
	}

	else if (!str_cmp (buf, "req:") || !str_cmp (buf, "require:"))
	{

		if (phase->req_skill)
			;

		argument = one_argument (argument, buf);

		if (!str_cmp (buf, "target"))
		{
			phase->flags &= ~PHASE_REQUIRE_ON_SELF;
			argument = one_argument (argument, buf);
		}
		else
			phase->flags |= PHASE_REQUIRE_ON_SELF;

		if ((ski_att = index_lookup (skills, buf)) == -1)
		{
			;
		}
		else
			phase->req_skill = ski_att;

		argument = one_argument (argument, buf);

		if (*buf == '>')
			phase->flags |= PHASE_REQUIRE_GREATER;
		else if (*buf == '<')
			phase->flags &= ~PHASE_REQUIRE_GREATER;
		else
		{
			;
		}

		argument = one_argument (argument, buf);

		if (!just_a_number (buf))
		{
			;
		}

		phase->req_skill_value = atoi (buf);
	}

	else
	{
		system_log ("Did not know what to do with craft line:", true);
		system_log (line, true);
	}
}

void
	reg_read_crafts (FILE * fp_reg, char *buf)
{
	char buf2[MAX_STRING_LENGTH];
	char *argument;

	while (!feof (fp_reg))
	{

		fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if (*buf && buf[strlen (buf) - 1] == '\n')
			buf[strlen (buf) - 1] = '\0';

		if (!*buf || *buf == '#')
			continue;

		argument = one_argument (buf, buf2);

		if (!str_cmp (buf2, "[end-crafts]"))
		{
			return;
		}

		subcraft_line (fp_reg, buf);
	}
}

void
	boot_crafts ()
{
	FILE *fp_crafts;
	char buf[MAX_STRING_LENGTH];

	if (!engine.in_play_mode ())
	{
		system ("./ordercrafts.pl ../regions/crafts");
	}

	if (!(fp_crafts = fopen (CRAFTS_FILE, "r")))
	{
		system_log ("The crafts file could not be read!", true);
		return;
	}

	while (!feof (fp_crafts))
	{

		fgets (buf, MAX_STRING_LENGTH, fp_crafts);

		if (*buf && buf[strlen (buf) - 1] == '\n')
			buf[strlen (buf) - 1] = '\0';

		if (!*buf || *buf == '#')
			continue;

		if (!str_cmp (buf, "[END]"))
			break;

		if (!str_cmp (buf, "[CRAFTS]"))
			reg_read_crafts (fp_crafts, buf);
	}

	fclose (fp_crafts);
}

void
	craft_prepare_message (CHAR_DATA * ch, char *message, CHAR_DATA * n,
	CHAR_DATA * N, CHAR_DATA * T, char *phase_msg,
	OBJ_DATA * tool, OBJ_DATA ** obj_list)
{
	int ovnum;
	char *p;
	char buf[MAX_STRING_LENGTH];

	*buf = '\0';

	for (p = buf; *phase_msg;)
	{
		if (*phase_msg == '$' && isdigit (phase_msg[1]))
		{
			phase_msg++;
			if (isdigit (*phase_msg) &&
				(ovnum = atoi (phase_msg)) < MAX_ITEMS_PER_SUBCRAFT)
			{
				sprintf (p, "#2%s#0", OBJS (obj_list[ovnum], ch));
				p = p + strlen (p);
			}

			while (isdigit (*phase_msg))
				phase_msg++;
		}

		else if (*phase_msg == '$')
		{

			switch (phase_msg[1])
			{
			case 'e':
				sprintf (p, "#5%s#0", n ? HSSH (n) : "HSSH-e");
				break;
			case 'm':
				sprintf (p, "#5%s#0", n ? HMHR (n) : "HMHR-m");
				break;
			case 's':
				sprintf (p, "#5%s#0", n ? HSHR (n) : "HSHR-s");
				break;
			case 'E':
				sprintf (p, "#5%s#0", N ? HSSH (N) : "HSSH-E");
				break;
			case 'M':
				sprintf (p, "#5%s#0", N ? HMHR (N) : "HMHR-M");
				break;
			case 'S':
				sprintf (p, "#5%s#0", N ? HSHR (n) : "HSHR-S");
				break;
			case 't':
				sprintf (p, "#2%s#0", tool ? OBJS (tool, ch) : "something");
				p = p + strlen (p);
				break;
			case 'T':
				sprintf (p, "#5%s#0", T ? PERS (T, ch) : "SOMEBODY-T");
				/* p [2] = toupper (p [2]); */
				break;
			case 'n':
				sprintf (p, "#5%s#0", n ? PERS (n, ch) : "SOMEBODY-n");
				/*p [2] = toupper (p [2]); */
				break;
			case 'N':
				sprintf (p, "#5%s#0", N ? PERS (N, ch) : "SOMEBODY-N");
				/*p [2] = toupper (p [2]); */
				break;
			}

			phase_msg += 2;
			p = p + strlen (p);
		}

		else
		{
			*(p++) = *(phase_msg++);
			*p = '\0';
		}
	}

	strcpy (message, buf);
}

OBJ_DATA *
	obj_list_vnum (CHAR_DATA * ch, OBJ_DATA * list, int vnum)
{
	for (; list; list = list->next_content)
		if (list->nVirtual == vnum && CAN_SEE_OBJ (ch, list))
			return list;

	return NULL;
}

OBJ_DATA *
	obj_list_vnum_dark (OBJ_DATA * list, int vnum)
{
	for (; list; list = list->next_content)
		if (list->nVirtual == vnum)
			return list;

	return NULL;
}


int
	craft__count_available_objs (CHAR_DATA * ch, int vnum)
{
	int nTally = 0;
	OBJ_DATA *ptrObj = NULL;

	nTally += (ch->right_hand
		&& ch->right_hand->nVirtual == vnum) ? ch->right_hand->count : 0;
	nTally += (ch->left_hand
		&& ch->left_hand->nVirtual == vnum) ? ch->left_hand->count : 0;
	for (ptrObj = ch->room->contents; ptrObj != NULL;
		ptrObj = ptrObj->next_content)
	{
		nTally += (ptrObj->nVirtual == vnum) ? ptrObj->count : 0;
	}
	return nTally;
}

int
	craft__count_available_uses (CHAR_DATA * ch, int vnum, int mode)
{
	int nTally = 0;
	OBJ_DATA *ptrObj = NULL;
	OBJ_DATA *orig_var = NULL;

	if (mode == 2)
	{
		for (ptrObj = ch->room->contents; ptrObj != NULL;
			ptrObj = ptrObj->next_content)
		{
			if (!orig_var)
			{
				if (ptrObj->nVirtual == vnum && IS_SET(ptrObj->obj_flags.extra_flags, ITEM_VARIABLE))
				{
					orig_var = ptrObj;
				}
			}
			else if (!vo_match_color(ptrObj, orig_var))
			{
				continue;
			}

			nTally += (ptrObj->nVirtual == vnum) ? ptrObj->o.od.value[0] : 0;
		}
	}
	else if (mode == 1)
	{
		nTally += (ch->right_hand
			&& ch->right_hand->nVirtual == vnum) ? ch->right_hand->o.od.value[0] : 0;

		if (ch->right_hand && ch->right_hand->nVirtual == vnum && IS_SET(ch->right_hand->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			orig_var = ch->right_hand;
		}

		if (orig_var && ch->left_hand && vo_match_color(ch->left_hand, orig_var))
		{
			nTally += (ch->left_hand
				&& ch->left_hand->nVirtual == vnum) ? ch->left_hand->o.od.value[0] : 0;
		}
		else if (!orig_var)
		{
			nTally += (ch->left_hand
				&& ch->left_hand->nVirtual == vnum) ? ch->left_hand->o.od.value[0] : 0;
		}
	}
	else
	{
		nTally += (ch->right_hand
			&& ch->right_hand->nVirtual == vnum) ? ch->right_hand->o.od.value[0] : 0;
		if (ch->right_hand && ch->right_hand->nVirtual == vnum && IS_SET(ch->right_hand->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			orig_var = ch->right_hand;
		}

		if (orig_var && ch->left_hand && vo_match_color(ch->left_hand, orig_var))
		{
			nTally += (ch->left_hand
				&& ch->left_hand->nVirtual == vnum) ? ch->left_hand->o.od.value[0] : 0;
		}
		else if (!orig_var)
		{
			nTally += (ch->left_hand
				&& ch->left_hand->nVirtual == vnum) ? ch->left_hand->o.od.value[0] : 0;
			if (ch->left_hand && ch->left_hand->nVirtual == vnum && IS_SET(ch->left_hand->obj_flags.extra_flags, ITEM_VARIABLE))
			{
				orig_var = ch->left_hand;
			}
		}

		for (ptrObj = ch->room->contents; ptrObj != NULL;
			ptrObj = ptrObj->next_content)
		{
			if (!orig_var)
			{
				if (ptrObj->nVirtual == vnum && IS_SET(ptrObj->obj_flags.extra_flags, ITEM_VARIABLE))
				{
					orig_var = ptrObj;
				}
			}
			else if (!vo_match_color(ptrObj, orig_var))
			{
				continue;
			}
			nTally += (ptrObj->nVirtual == vnum) ? ptrObj->o.od.value[0] : 0;
		}
	}
	orig_var = NULL;
	return nTally;
}

OBJ_DATA *
	get_item_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase)
{
	int j = 0;
	int vnum = 0;
	PHASE_DATA *tmp_phase = NULL;
	OBJ_DATA *tobj = NULL;

	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{

		if (!(vnum = item->items[j]))
			continue;

		if ((IS_SET (item->flags, SUBCRAFT_PRODUCED)
			|| IS_SET (item->flags, SUBCRAFT_GIVE)) && (item->phase != phase))
		{

			/* if produced later, add to list, so it can be used by another phase */
			for (tmp_phase = phase->next;
				tmp_phase;
				tmp_phase = tmp_phase->next)
				if (tmp_phase == item->phase)
				{
					//item->key_index = j;
					return vtoo (vnum);
				}
		}

		if (IS_SET (item->flags, SUBCRAFT_USED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_USED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_USED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				//if (item->item_counts > craft__count_available_objs (ch, vnum))
				//    continue;
				//else
				//{
				//item->key_index = j;
				return tobj;
				//}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				//if (item->item_counts > craft__count_available_objs (ch, vnum))
				//    continue;
				//else
				//{
				//item->key_index = j;
				return tobj;
				//}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				//if (item->item_counts > craft__count_available_objs (ch, vnum))
				//    continue;
				//else
				//{
				//item->key_index = j;
				return tobj;
				//}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_uses (ch, vnum, 0))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts > craft__count_available_uses (ch, vnum, 1))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_uses (ch, vnum, 2))
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}


		if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					//	item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
		{
			if ((tobj = obj_list_vnum_dark (ch->right_hand, vnum))
				|| (tobj = obj_list_vnum_dark (ch->left_hand, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WIELDED))
		{
			if (((tobj = get_equip (ch, WEAR_BOTH)) && tobj->nVirtual == vnum)
				|| ((tobj = get_equip (ch, WEAR_PRIM)) && tobj->nVirtual == vnum)
				|| ((tobj = get_equip (ch, WEAR_SEC)) && tobj->nVirtual == vnum))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN))
		{
			for (tobj = ch->equip; tobj; tobj = tobj->next_content)
			{
				if (tobj->nVirtual != vnum)
					continue;
				if (item->item_counts > tobj->count)
					continue;

				if (tobj->location != WEAR_BOTH &&
					tobj->location != WEAR_PRIM &&
					tobj->location != WEAR_SEC)
				{
					//item->key_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN)
			|| IS_SET (item->flags, SUBCRAFT_WIELDED)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD)
			|| IS_SET (item->flags, SUBCRAFT_HELD)
			|| IS_SET (item->flags, SUBCRAFT_IN_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD)
			|| IS_SET (item->flags, SUBCRAFT_USED)
			|| IS_SET (item->flags, SUBCRAFT_USED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_USED_HELD))
			continue;

		/* Grab this item wherever it is */

		if (((tobj = obj_list_vnum_dark (ch->right_hand, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = obj_list_vnum (ch, ch->left_hand, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = obj_list_vnum (ch, ch->room->contents, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_BOTH)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_PRIM)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_SEC)) && tobj->nVirtual == vnum))
		{
			if (item->item_counts > tobj->count)
				continue;
			else
			{
				//item->key_index = j;
				return tobj;
			}
		}
	}

	return NULL;
}

OBJ_DATA *
	get_key_start_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index)
{
	int j;
	int vnum = 0;
	OBJ_DATA *tobj;

	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{
		vnum = item->items[j];
		if (vnum < 1)
			continue;

		if (item->phase != phase)
		  continue;

		if (IS_SET (item->flags, SUBCRAFT_USED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_USED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_USED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_uses (ch, vnum, 0))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum)))
			{
				if (item->item_counts <= craft__count_available_uses (ch, vnum, 1))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_uses (ch, vnum, 2))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}


		if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
		{
			if ((tobj = obj_list_vnum_dark (ch->right_hand, vnum))
				|| (tobj = obj_list_vnum_dark (ch->left_hand, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WIELDED))
		{
			if (((tobj = get_equip (ch, WEAR_BOTH)) && tobj->nVirtual == vnum)
				|| ((tobj = get_equip (ch, WEAR_PRIM)) && tobj->nVirtual == vnum)
				|| ((tobj = get_equip (ch, WEAR_SEC)) && tobj->nVirtual == vnum))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN))
		{
			for (tobj = ch->equip; tobj; tobj = tobj->next_content)
			{
				if (tobj->nVirtual != vnum)
					continue;
				if (item->item_counts > tobj->count)
					continue;

				if (tobj->location != WEAR_BOTH &&
					tobj->location != WEAR_PRIM &&
					tobj->location != WEAR_SEC)
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN)
			|| IS_SET (item->flags, SUBCRAFT_WIELDED)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD)
			|| IS_SET (item->flags, SUBCRAFT_HELD)
			|| IS_SET (item->flags, SUBCRAFT_IN_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD)
			|| IS_SET (item->flags, SUBCRAFT_USED)
			|| IS_SET (item->flags, SUBCRAFT_USED_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_USED_HELD))
			continue;

		/* Grab this item wherever it is */

		if (((tobj = obj_list_vnum_dark (ch->right_hand, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = obj_list_vnum (ch, ch->left_hand, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = obj_list_vnum (ch, ch->room->contents, vnum)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_BOTH)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_PRIM)) &&
			tobj->nVirtual == vnum) ||
			((tobj = get_equip (ch, WEAR_SEC)) && tobj->nVirtual == vnum))
		{
			if (item->item_counts > tobj->count)
				continue;
			else
			{
				ch->craft_index = j;
				return tobj;
			}
		}
	}

	return NULL;
}

OBJ_DATA *
	get_key_end_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index)
{
	int j;
	int vnum;
	PHASE_DATA *tmp_phase;

	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{
		vnum = item->items[j];
		if (vnum < 1)
			continue;

		if (item->phase != phase)
			continue;

		for (tmp_phase = phase->next; tmp_phase; tmp_phase = tmp_phase->next)
		{
			if (tmp_phase == item->phase)
			{
				if (index == j)
				{
					return vtoo (vnum);
				}
			}
		}
	}

	return NULL;
}

void
	missing_item_msg (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, char *header)
{
	int i;
	int choice_count = 0;
	OBJ_DATA *proto_obj;
	char *p;
	char buf[MAX_STRING_LENGTH];

	if (item->item_counts > 1)
		sprintf (buf, "%s%d of ", header, item->item_counts);
	else
		sprintf (buf, "%s", header);

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
		if (item->items[i])
			choice_count++;

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{

		if (!item->items[i])
			continue;

		choice_count--;

		proto_obj = vtoo (item->items[i]);

		if (!proto_obj)
			continue;

		sprintf (buf + strlen (buf), "#2%s#0", proto_obj->short_description);
		if (choice_count)
			strcat (buf, ", ");

		if (choice_count == 1)
			strcat (buf, "or ");

		if (!choice_count)
			strcat (buf, ".");
	}

	reformat_string (buf, &p);
	send_to_char (p, ch);

	mem_free (p);
}

OBJ_DATA * item_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item)
{
	int i;
	int choice_count = 0;
	OBJ_DATA *proto_obj;

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
		if (item->items[i])
			choice_count++;

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{

		if (!item->items[i])
			continue;

		choice_count--;

		proto_obj = vtoo (item->items[i]);

		if (!proto_obj)
			continue;

		return proto_obj;
	}

}

int
	craft_uses (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!craft->obj_items[i])
		{
			continue;
		}
		if (!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED_HELD) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED_ROOM) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_HELD) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED_HELD) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_SAMPLED_ROOM) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_HELD) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_ROOM) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_IN_ROOM))
		{
			continue;
		}
		for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
		{
			if (craft->obj_items[i]->items[j] == vnum)
			{
				return 1;
			}
		}
	}

	return 0;
}

int
	craft_produces (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!craft->obj_items[i])
			continue;
		if (!
			(IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED)
			|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE)))
			continue;
		for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
		{
			if (craft->obj_items[i]->items[j] == vnum)
				return 1;
		}
	}

	return 0;
}

SUBCRAFT_HEAD_DATA *
	branch_craft_backward (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft, int level)
{
	SUBCRAFT_HEAD_DATA *tcraft, *dcraft, *next_craft;
	int i = 0;

	if (level >= 10)
		return NULL;

	for (tcraft = crafts; tcraft; tcraft = next_craft)
	{
		next_craft = tcraft->next;
		if (str_cmp (tcraft->craft_name, craft->craft_name))
			continue;
		if (!str_cmp (tcraft->subcraft_name, craft->subcraft_name))
			continue;
		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{
			if (!craft->obj_items[i])
				continue;
			if (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED)
				|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE)
				|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED)
				|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_HELD)
				|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PROGRESSED_ROOM)
				|| IS_SET (craft->obj_items[i]->flags, SUBCRAFT_IN_ROOM))
				continue;
			if (!tcraft)
				continue;
			if (craft_produces (tcraft, craft->obj_items[i]->items[0]))
			{
				if (!has_craft (ch, tcraft)
					&& meets_skill_requirements (ch, tcraft))
					return tcraft;
				if (!meets_skill_requirements (ch, tcraft))	/* Dead-end; need to improve skills first. */
					continue;
				if ((dcraft = branch_craft_backward (ch, tcraft, ++level)))	/* Branching craft beneath tcraft. */
					return dcraft;
			}
		}
	}

	return NULL;

}

SUBCRAFT_HEAD_DATA *
	branch_craft_forward (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft, int level)
{
	SUBCRAFT_HEAD_DATA *tcraft, *dcraft, *next_craft;
	int i = 0;

	if (level >= 10)
		return NULL;

	for (tcraft = crafts; tcraft; tcraft = next_craft)
	{
		next_craft = tcraft->next;
		if (str_cmp (tcraft->craft_name, craft->craft_name))
			continue;
		if (!str_cmp (tcraft->subcraft_name, craft->subcraft_name))
			continue;
		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{
			if (!craft->obj_items[i])
				continue;
			if (!(IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED) ||
				IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE)))
				continue;
			if (!craft_uses (tcraft, craft->obj_items[i]->items[0]))
				continue;
			if ((dcraft = branch_craft_backward (ch, tcraft, ++level)))
				return dcraft;
			if (has_craft (ch, tcraft)
				|| !meets_skill_requirements (ch, tcraft))
				continue;
			return tcraft;
		}
	}

	return NULL;

}

SUBCRAFT_HEAD_DATA *
	get_related_subcraft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	SUBCRAFT_HEAD_DATA *tcraft;
	AFFECTED_TYPE *af;
	int i = 0;

	tcraft = branch_craft_backward (ch, craft, 0);

	if (!tcraft)
		tcraft = branch_craft_forward (ch, craft, 0);

	if (!tcraft)
	{
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if ((af = get_affect (ch, i)))
			{
				if (str_cmp
					(af->a.craft->subcraft->craft_name, craft->craft_name))
					continue;
				tcraft = branch_craft_backward (ch, af->a.craft->subcraft, 0);
				if (!tcraft)
					tcraft = branch_craft_forward (ch, af->a.craft->subcraft, 0);
				if (tcraft && tcraft != craft)
					return tcraft;
			}
		}
	}

	if (tcraft && tcraft != craft)
		return tcraft;

	return NULL;
}

int
	has_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	AFFECTED_TYPE *af;

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		{
			if (!str_cmp
				(af->a.craft->subcraft->subcraft_name, craft->subcraft_name))
				return 1;
		}
	}

	return 0;
}

void
	branch_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *tcraft;
	int num = 0, num2 = 0, i = 0, stop = 0;
	char buf[MAX_STRING_LENGTH];
	bool related = false;
	bool skipped_categories = false;
	int group_count = 0;
	int pc_count = 0;

	if (get_affect (ch, MAGIC_CRAFT_BRANCH_STOP) && !engine.in_test_mode ())
	{
		return;
	}

	if ((!strcmp (craft->craft_name, "general-fire")) ||
		(!strcmp (craft->craft_name, "general-labor")) ||
		(!strcmp (craft->craft_name, "general-food")) ||
		(!strcmp (craft->craft_name, "general-armor")) ||
		(!strcmp (craft->craft_name, "general-weapons")) ||
		(!strcmp (craft->craft_name, "general-recreation")) ||
		(!strcmp (craft->craft_name, "general-serving")) ||
		(!strcmp (craft->craft_name, "general")))
		return;

	// Check if there are any crafts in our current group we might need...
	if (!(tcraft = get_related_subcraft (ch, craft)))
	{
		// If not, we just cycle through our list,
		// and look for a craft within the same group
		// that we've got the skill requirements for.
		for (tcraft = crafts; tcraft; tcraft = tcraft->next)
		{
			group_count ++;

			if (tcraft == craft)
				continue;
			if (!str_cmp (craft->craft_name, tcraft->craft_name)
				&& meets_skill_requirements (ch, tcraft)
				&& !has_craft (ch, tcraft))
				num++;
		}

		if (num <= 0)
		{
			if (!str_cmp(craft->craft_name, "poor-weaponcrafting"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("poor-weaponcrafting") == crafts_of_pc(ch, "poor-weaponcrafting"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("ordinary-weaponcrafting", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("ordinary-weaponcrafting", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else if (!str_cmp(craft->craft_name, "poor-gunsmithing"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("poor-gunsmithing") == crafts_of_pc(ch, "poor-gunsmithing"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("ordinary-gunsmithing", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("ordinary-gunsmithing", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else if (!str_cmp(craft->craft_name, "poor-armorcrafting"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("poor-armorcrafting") == crafts_of_pc(ch, "poor-armorcrafting"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("ordinary-armorcrafting", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("ordinary-armorcrafting", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else if (!str_cmp(craft->craft_name, "ordinary-weaponcrafting"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("ordinary-weaponcrafting") == crafts_of_pc(ch, "ordinary-weaponcrafting"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("good-weaponcrafting", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("good-weaponcrafting", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else if (!str_cmp(craft->craft_name, "ordinary-gunsmithing"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("ordinary-gunsmithing") == crafts_of_pc(ch, "ordinary-gunsmithing"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("good-gunsmithing", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("good-gunsmithing", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else if (!str_cmp(craft->craft_name, "ordinary-armorcrafting"))
			{
				// And we've got all the poor-weaponcrafting crafts...
				if (crafts_in_craftset("ordinary-armorcrafting") == crafts_of_pc(ch, "ordinary-armorcrafting"))
				{
					for (tcraft = crafts; tcraft; tcraft = tcraft->next)
					{
						group_count ++;

						if (tcraft == craft)
							continue;
						if (!str_cmp ("good-armorcrafting", tcraft->craft_name)
							&& meets_skill_requirements (ch, tcraft)
							&& !has_craft (ch, tcraft))
						{
							num2++;
						}
					}	
				}

				if (num2 <= 0 || number(0,1))
					return;

				stop = number (1, num2);

				for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
				{
					if (tcraft == craft)
						continue;
					if (!str_cmp ("good-armorcrafting", tcraft->craft_name)
						&& meets_skill_requirements (ch, tcraft)
						&& !has_craft (ch, tcraft))
						i++;
					if (i == stop)
					{
						skipped_categories = true;
						break;
					}
				}
				if (!skipped_categories)
					return;

			}
			else
			{
				return;
			}
		}
		if (!skipped_categories)
		{
			stop = number (1, num);

			for (tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next)
			{
				if (tcraft == craft)
					continue;
				if (!str_cmp (craft->craft_name, tcraft->craft_name)
					&& meets_skill_requirements (ch, tcraft)
					&& !has_craft (ch, tcraft))
					i++;
				if (i == stop)
					break;
			}
		}
	}
	else
		related = true;

	if (!tcraft || tcraft == craft || has_craft (ch, tcraft))
		return;

	if (related)
		sprintf (buf, "You have branched a related subcraft, #6%s %s#0!\n",
		tcraft->command, tcraft->subcraft_name);
	else
		sprintf (buf, "You have branched a new subcraft, #6%s %s#0!\n",
		tcraft->command, tcraft->subcraft_name);

	send_to_char (buf, ch);

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if (!get_affect (ch, i))
			break;
	}

	magic_add_affect (ch, i, -1, 0, 0, 0, 0);
	af = get_affect (ch, i);
	af->a.craft =
		(struct affect_craft_type *) alloc (sizeof (struct affect_craft_type),
		23);
	af->a.craft->subcraft = tcraft;

	magic_add_affect (ch, MAGIC_CRAFT_BRANCH_STOP, number (30, 90), 0, 0, 0, 0);
}

int
	requires_skill_check (SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->req_skill || phase->skill)
			return 1;
	}

	return 0;
}

int
	branching_check (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	SUBCRAFT_HEAD_DATA *tcraft;
	AFFECTED_TYPE *af;
	int max = 0, max_amt = 0, multiplier = 0;
	int total_crafts = 0, char_crafts = 0, i = 0;
	float percentage = 0;

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->skill)
		{
			if (phase->dice * phase->sides > max_amt)
			{
				max = phase->skill;
				max_amt = phase->dice * phase->sides;
			}
		}

		if (phase->req_skill)
		{
			if (phase->req_skill_value > max_amt)
			{
				max = phase->req_skill;
				max_amt = phase->req_skill_value;
			}
		}
	}

	for (tcraft = crafts; tcraft; tcraft = tcraft->next)
	{
		if (!str_cmp (tcraft->craft_name, craft->craft_name)
			&& meets_skill_requirements (ch, tcraft))
			total_crafts++;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if (!(af = get_affect (ch, i)))
			continue;
		if (!str_cmp (af->a.craft->subcraft->craft_name, craft->craft_name))
			char_crafts++;
	}

	if (total_crafts <= char_crafts)
		total_crafts = char_crafts + 1;

	percentage = (float) char_crafts / (float) total_crafts;

	if (percentage >= 0 && percentage <= .33)
		multiplier = 4;
	else if (percentage > .33 && percentage <= .60)
		multiplier = 3;
	else if (percentage > .60 && percentage <= .75)
		multiplier = 2;
	else
		multiplier = 1;

	return MIN ((calc_lookup (ch, REG_LV, max) * multiplier), 65);
}

int
	figure_craft_delay (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int seconds = 0, skill = 0, numskills = 0;

	seconds = craft->delay * 60 * 60;

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->req_skill)
		{
			numskills++;
			skill += ch->skills[phase->req_skill];
		}
		if (phase->skill)
		{
			numskills++;
			skill += ch->skills[phase->skill];
		}
	}

	if (!numskills)
		numskills = 1;

	skill /= numskills;

	if (skill >= 30 && skill < 50)
		seconds = (int) ((float) seconds * 0.85);
	else if (skill >= 50 && skill < 70)
		seconds = (int) ((float) seconds * 0.72);
	else if (skill >= 70)
		seconds = (int) ((float) seconds * 0.61);

	return seconds;
}

int
	figure_craft_faildelay (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int seconds = 0, skill = 0, numskills = 0;

	seconds = craft->faildelay * 60 * 60;

	for (phase = craft->phases; phase; phase = phase->next)
	{
		if (phase->req_skill)
		{
			numskills++;
			skill += ch->skills[phase->req_skill];
		}
		if (phase->skill)
		{
			numskills++;
			skill += ch->skills[phase->skill];
		}
	}

	if (!numskills)
		numskills = 1;

	skill /= numskills;

	if (skill >= 30 && skill < 50)
		seconds = (int) ((float) seconds * 0.85);
	else if (skill >= 50 && skill < 70)
		seconds = (int) ((float) seconds * 0.72);
	else if (skill >= 70)
		seconds = (int) ((float) seconds * 0.61);

	return seconds;
}

void
	activate_phase (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	char numbuf[20];
	int count;
	OBJ_DATA *failobj;
	CHAR_DATA *failmob;
	int nObjectTally = 0;
	int nObjectVnum = 0;
	int i;
	int attr_value = 0;
	int dice_value = 0;
	int delay_time = 0;
	bool missing_said = false;
	int phase_failed = 0;
	int skill_value;
	int index = 0;
	int dice_val;
	int ch_level;
	char *p;
	char *first;
	PHASE_DATA *phase = NULL;
	DEFAULT_ITEM_DATA *item = NULL;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *mob = NULL;
	CHAR_DATA *target_ch = NULL;
	OBJ_DATA *tool = NULL;
	OBJ_DATA *target_obj = NULL;
	OBJ_DATA *obj_list[MAX_ITEMS_PER_SUBCRAFT];
	OBJ_DATA *fobj_list[MAX_ITEMS_PER_SUBCRAFT];
	OBJ_DATA *prog_take_obj_list[MAX_ITEMS_PER_SUBCRAFT];
	OBJ_DATA *prog_load_obj_list[MAX_ITEMS_PER_SUBCRAFT];
	OBJ_DATA *ptrObj = NULL, *ptrObjNext = NULL;
	SUBCRAFT_HEAD_DATA *subcraft = NULL;
	CRAFT_VARIABLE_DATA *vars = NULL;
	int item_required[MAX_ITEMS_PER_SUBCRAFT];
	char buf[MAX_STRING_LENGTH];

	for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		obj_list[i] = NULL;
		fobj_list[i] = NULL;
		prog_take_obj_list[i] = NULL;
		prog_load_obj_list[i] = NULL;
	}

	/* A craft is active as long as af->a.craft->timer is non-zero. */

	if (af->a.craft->phase)
		phase = af->a.craft->phase;

	/* Shorthand variables */

	subcraft = af->a.craft->subcraft;
	target_ch = af->a.craft->target_ch;
	target_obj = af->a.craft->target_obj;

	/* Point affect at next phase, and setup timer for that phase */

	if (!phase)
	{
		phase = af->a.craft->subcraft->phases;
		af->a.craft->phase = phase->next;
		af->a.craft->timer = phase->phase_seconds;
	}
	else
	{
		af->a.craft->phase = phase->next;

		if (phase->next)
			af->a.craft->timer = phase->phase_seconds;
		else
			af->a.craft->timer = 0;
	}

	if (!IS_MORTAL (ch))
		af->a.craft->timer = 0;

	memset (item_required, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (int));
	memset (obj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));
	memset (fobj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));

	/* load object lists */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (!item->items[0])	/* No items in list.  Nothing required */
			continue;

		// one item from obj_items
		if (i == subcraft->key_first)
		{
			obj_list[i] = get_key_start_obj (ch, item, phase, ch->craft_index);
			continue;
		}
		else if (i == subcraft->key_end)
		{
			obj_list[i] = get_key_end_obj (ch, item, phase, ch->craft_index);
			continue;
		}
		else
		{
			obj_list[i] = get_item_obj (ch, item, phase);
		}
	}

	/* load fail obj lists */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->fails[i];

		if (!item)
			continue;

		if (!item->items[0])	/* No items in list.  Nothing required */
			continue;

		fobj_list[i] = get_item_obj (ch, item, phase);
	}

	/* Determine what is required for this phase based on first person */
	/* p refers to the 3 in object3, or $3 in messages */
	if (phase->first)
	{
		for (p = phase->first; *p; p++)
		{
			if (*p == '$')
			{
				p++;
				if (isdigit (*p) && atoi (p) < MAX_ITEMS_PER_SUBCRAFT)
					item_required[atoi (p)] = 1;
			}
		}
	}

	/** tool check **/
	if (phase->tool && phase->tool->items[0])
	{
		if (!(tool = get_item_obj (ch, phase->tool, phase)))
		{
			send_to_char ("You are missing a tool.\n", ch);
			af->a.craft->timer = 0;
			return;
		}
	}

	/** group check **/
	if ((subcraft->followers > 0) && !(num_followers (ch) >= subcraft->followers))
	{

		send_to_char ("You need more followers.\n", ch);
		af->a.craft->timer = 0;
		return;

	}

	/* Make sure all required objects are accounted for */

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!item_required[i])
			continue;

		if (obj_list[i])
			continue;

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (item && (IS_SET (item->flags, SUBCRAFT_PRODUCED) ||
			IS_SET (item->flags, SUBCRAFT_SAMPLED) || IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD) || IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM) ||
			IS_SET (item->flags, SUBCRAFT_GIVE))
			&& item->phase == phase)
			continue;

		if (!missing_said)
		{
			missing_said = true;
			send_to_char ("\n#1You are missing one or more items:#0\n\n", ch);
			act ("$n stops doing $s craft.", false, ch, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if (IS_SET (item->flags, SUBCRAFT_WORN))
			missing_item_msg (ch, item, "You must wear ");
		else if (IS_SET (item->flags, SUBCRAFT_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED))
			missing_item_msg (ch, item, "In the room or held must be ");
		else if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_WIELDED))
			missing_item_msg (ch, item, "You must wield ");
		else if (IS_SET (item->flags, SUBCRAFT_USED_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_USED_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else if (IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else
			missing_item_msg (ch, item, "You need ");
	}//for (i = 0;

	if (missing_said)
		return;


	// CPROGS START OF PHASE

	if (IS_MORTAL(ch) && phase->phase_start_prog)
	{
		std::multimap<std::string, room_prog>::iterator it;
		it = craft_prog_list.find(subcraft->subcraft_name);
		if (it != craft_prog_list.end())
		{
			std::pair<std::multimap<std::string, room_prog>::iterator, std::multimap<std::string, room_prog>::iterator> range = craft_prog_list.equal_range(subcraft->subcraft_name);
			for (std::multimap<std::string, room_prog>::iterator bit = range.first; bit != range.second; ++bit)
			{
				if (!str_cmp(bit->second.command, phase->phase_start_prog))
				{
					c_prog(ch, phase->phase_start_prog, bit->second);
					break;
				}
			}
		}
	}

	/* Check for skill requirements */

	if (phase->req_skill)
	{
		if (IS_SET (phase->flags, PHASE_REQUIRE_ON_SELF))
			skill_value = ch->skills[phase->req_skill];
		else
			skill_value = target_ch->skills[phase->req_skill];

		if (IS_SET (phase->flags, PHASE_REQUIRE_GREATER) &&
			skill_value <= phase->req_skill_value)
			phase_failed = 1;
		else if (!IS_SET (phase->flags, PHASE_REQUIRE_GREATER) &&
			skill_value >= phase->req_skill_value)
			phase_failed = 1;

		if (ch->skills[phase->skill] < craft_roll_max(phase->dice, phase->sides))
		{
			skill_use(ch, phase->req_skill, 0);
		}
	}

	if (phase->skill)
	{

		// If there's no chance we can fail this craft, then we get
		// no chance of increasing our skill. Nothing gained out of
		// spamming a skill endlessly.

		if (ch->skills[phase->skill] < craft_roll_max(phase->dice, phase->sides))
		{
			skill_use(ch, phase->skill, 0);
		}

		dice_val = dice (phase->dice, phase->sides);
		ch_level = skill_level (ch, phase->skill,0); //includes object effects

		if (dice_val > ch_level)
		{
			phase_failed = 1;
		}
	}

	if ( phase->attribute > -1 )
	{
		switch ( phase->attribute )
		{
		case 0:
			attr_value = GET_STR (ch);
			break;
		case 1:
			attr_value = GET_DEX (ch);
			break;
		case 2:
			attr_value = GET_CON (ch);
			break;
		case 3:
			attr_value = GET_WIL (ch);
			break;
		case 4:
			attr_value = GET_INT (ch);
			break;
		case 5:
			attr_value = GET_AUR (ch);
			break;
		case 6:
			attr_value = GET_AGI (ch);
			break;
		}

		dice_value = dice (phase->dice, phase->sides);
		if ( attr_value < dice_value )
		{
			phase_failed = 1;
		}
	}

	/* Deducts uses from a component-type item */
	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (item->phase != phase)
			continue;

		for (int j = 0; j < MAX_DEFAULT_ITEMS; j++)
		{
			vars = subcraft->craft_variable[j];

			if (!vars)
				continue;

			if (vars->phase != phase)
				continue;

			if (vars->from != i)
				continue;

			if (!obj_list[i])
				continue;

			if (vars && *vars->category)
			{
				for (int h = 0; h < 10; h++)
				{
					if (obj_list[i]->var_color[h] && str_cmp (obj_list[i]->var_color[h], "(null)") &&
						obj_list[i]->var_cat[h] && str_cmp (obj_list[i]->var_cat[h], "(null)"))
					{
						if (!str_cmp(vars->category, obj_list[i]->var_cat[h]))
						{
							//sprintf(buf, "Vars->to: %d, Vars->pos: %d, Obj_list[i]->var_color[h]: %s, i: %d, j: %d, h: %d.", vars->to, vars->pos, obj_list[i]->var_color[h], i, j, h);
							//send_to_gods(buf);
							subcraft->load_color[vars->to][vars->pos] = str_dup(obj_list[i]->var_color[h]);
							subcraft->load_cat[vars->to][vars->pos] = str_dup(vars->category);
							//send_to_gods("subcraft->load_color[vars->to][vars->pos]");
							//send_to_gods(subcraft->load_color[vars->to][vars->pos]);
							//send_to_gods("obj_list[i]->var_color[h]");
							//send_to_gods(obj_list[i]->var_color[h]);
							//send_to_gods("vars->category");
							//send_to_gods(vars->category);

						}
					}
				}
			}
		}
	}

	/* Deducts uses from a component-type item */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		bool skipped = false;
		OBJ_DATA *orig_var = NULL;
		OBJ_DATA *xobj = NULL;
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		/*!item_required [i] || */
		if (item->phase != phase || (!IS_SET (item->flags, SUBCRAFT_SAMPLED) &&
			!IS_SET (item->flags, SUBCRAFT_SAMPLED_HELD) &&
			!IS_SET (item->flags, SUBCRAFT_SAMPLED_ROOM)))
			continue;

		if (!obj_list[i])
		{
			act
				("You are not using enough of a particular item to finish this craft.",
				false, ch, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;
			return;
		}

		/* Purge Consumed Craft Items */
		nObjectTally = item->item_counts;
		nObjectVnum = obj_list[i]->nVirtual;
		if (nObjectTally && ch->right_hand
			&& ch->right_hand->nVirtual == nObjectVnum)
		{

			skipped = false;
			// If we don't have an original variable object,
			// but this object is a variable: we now have our variable object.
			if (!orig_var)
			{
				if (IS_SET(ch->right_hand->obj_flags.extra_flags, ITEM_VARIABLE))
				{
					orig_var = ch->right_hand;
				}
			}
			// if we did have a variable object, but it doesn't match the colors of this one, we need to skip.
			else if (!vo_match_color(ch->right_hand, orig_var))
			{
				skipped = true;
			}

			if (!skipped)
			{

				if (ch->right_hand->o.od.value[0] <= nObjectTally)
				{
					nObjectTally -= ch->right_hand->o.od.value[0];

					if (GET_ITEM_TYPE(ch->right_hand) == ITEM_COMPONENT)
					{	
						if (vtoo(ch->right_hand->o.od.value[1]) && (xobj = LOAD_EXACT_COLOR(ch->right_hand, ch->right_hand->o.od.value[1])))
						{
							fluid_object(xobj);
							obj_to_char(xobj, ch);
						}						
						extract_obj (ch->right_hand);
					}
				}
				else
				{
					ch->right_hand->o.od.value[0] -= nObjectTally;
					nObjectTally = 0;
				}
			}
		}

		if (nObjectTally && ch->left_hand
			&& ch->left_hand->nVirtual == nObjectVnum)
		{

			skipped = false;
			// If we don't have an original variable object,
			// but this object is a variable: we now have our variable object.
			if (!orig_var)
			{
				if (IS_SET(ch->left_hand->obj_flags.extra_flags, ITEM_VARIABLE))
				{
					orig_var = ch->left_hand;
				}
			}
			// if we did have a variable object, but it doesn't match the colors of this one, we need to skip.
			else if (!vo_match_color(ch->left_hand, orig_var))
			{
				skipped = true;
			}

			if (!skipped)
			{

				if (ch->left_hand->o.od.value[0] <= nObjectTally)
				{
					nObjectTally -= ch->left_hand->o.od.value[0];
					if (GET_ITEM_TYPE(ch->left_hand) == ITEM_COMPONENT)
					{
						if (vtoo(ch->left_hand->o.od.value[1]) && (xobj = LOAD_EXACT_COLOR(ch->left_hand, ch->left_hand->o.od.value[1])))
						{
							fluid_object(xobj);
							obj_to_char(xobj, ch);
						}
						extract_obj (ch->left_hand);
					}
				}
				else
				{
					ch->left_hand->o.od.value[0] -= nObjectTally;
					nObjectTally = 0;
				}
			}
		}

		if (nObjectTally && ch->room->contents)
		{
			for (ptrObj = ch->room->contents; nObjectTally && ptrObj != NULL;)
			{
				if (ptrObj->nVirtual == nObjectVnum)
				{

					skipped = false;
					// If we don't have an original variable object,
					// but this object is a variable: we now have our variable object.
					if (!orig_var)
					{
						if (IS_SET(ptrObj->obj_flags.extra_flags, ITEM_VARIABLE))
						{
							orig_var = ptrObj;
						}
					}
					// if we did have a variable object, but it doesn't match the colors of this one, we need to skip.
					else if (!vo_match_color(ptrObj, orig_var))
					{
						skipped = true;
					}

					if (!skipped)
					{
						if (ptrObj->o.od.value[0] <= nObjectTally)
						{
							if (GET_ITEM_TYPE(ptrObj) == ITEM_COMPONENT)
							{
								nObjectTally -= ptrObj->o.od.value[0];
								ptrObjNext = ptrObj->next_content;
								if (vtoo(ptrObj->o.od.value[1]) && (xobj = LOAD_EXACT_COLOR(ptrObj, ptrObj->o.od.value[1])))
								{
									fluid_object(xobj);
									obj_to_room(xobj, ch->in_room);
								}
								extract_obj (ptrObj);
							}
							ptrObj = ptrObjNext;
						}
						else
						{
							ptrObj->o.od.value[0] -= nObjectTally;
							nObjectTally = 0;
							ptrObj = ptrObj->next_content;
						}
					}
					else
					{
						ptrObj = ptrObj->next_content;
					}
				}
				else
				{
					ptrObj = ptrObj->next_content;
				}
			}//for (ptrObj = ch->room->contents
		}//if (nObjectTally

		orig_var = NULL;
	}//if (!obj_list[i])



	/* Add to our progressed items from wherever they are */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];
		OBJ_DATA *tobj = NULL;

		if (!item)
			continue;
		/*!item_required [i] || */
		if (item->phase != phase || (!IS_SET (item->flags, SUBCRAFT_PROGRESSED) &&
			!IS_SET (item->flags, SUBCRAFT_PROGRESSED_HELD) &&
			!IS_SET (item->flags, SUBCRAFT_PROGRESSED_ROOM)))
			continue;

		if (!obj_list[i])
			continue;

		if (item->item_counts)
			obj_list[i]->o.od.value[0] += item->item_counts;
		else
			obj_list[i]->o.od.value[0] ++;


		if (ch->in_room && GET_ITEM_TYPE(obj_list[i]) == ITEM_PROGRESS && obj_list[i]->o.od.value[0] >= obj_list[i]->o.od.value[1] && obj_list[i]->o.od.value[2] && vtoo(obj_list[i]->o.od.value[2]))
		{
			if ((tobj = LOAD_EXACT_COLOR(obj_list[i], obj_list[i]->o.od.value[2])))
			{
				fluid_object(tobj);
				prog_load_obj_list[i] = tobj;
				prog_take_obj_list[i] = obj_list[i];
			}
		}
	}


	/* Removed USED items from wherever they are */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		/*!item_required [i] || */
		if (item->phase != phase || (!IS_SET (item->flags, SUBCRAFT_USED) &&
			!IS_SET (item->flags, SUBCRAFT_USED_HELD) &&
			!IS_SET (item->flags, SUBCRAFT_USED_ROOM)))
			continue;

		if (!obj_list[i])
		{
			act
				("You are not using enough of a particular item to finish this craft.",
				false, ch, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;
			return;
		}

		/* Purge Consumed Craft Items */
		nObjectTally = item->item_counts;
		nObjectVnum = obj_list[i]->nVirtual;
		if (nObjectTally && ch->right_hand
			&& ch->right_hand->nVirtual == nObjectVnum)
		{
			if (ch->right_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->right_hand->count;
				extract_obj (ch->right_hand);
			}
			else
			{
				ch->right_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}

		if (nObjectTally && ch->left_hand
			&& ch->left_hand->nVirtual == nObjectVnum)
		{
			if (ch->left_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->left_hand->count;
				extract_obj (ch->left_hand);
			}
			else
			{
				ch->left_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}

		if (nObjectTally && ch->room->contents)
		{
			for (ptrObj = ch->room->contents; nObjectTally && ptrObj != NULL;)
			{
				if (ptrObj->nVirtual == nObjectVnum)
				{
					if (ptrObj->count <= nObjectTally)
					{
						nObjectTally -= ptrObj->count;
						ptrObjNext = ptrObj->next_content;
						extract_obj (ptrObj);
						ptrObj = ptrObjNext;
					}
					else
					{
						ptrObj->count -= nObjectTally;
						nObjectTally = 0;
						ptrObj = ptrObj->next_content;
					}
				}
				else
				{
					ptrObj = ptrObj->next_content;
				}
			}//for (ptrObj = ch->room->contents
		}//if (nObjectTally
	}//if (!obj_list[i])

	/* Removed OPTIONAL items from wherever they are */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		/*!item_required [i] || */
		if (item->phase != phase || (!IS_SET (item->flags, SUBCRAFT_OPTIONAL) &&
			!IS_SET (item->flags, SUBCRAFT_OPTIONAL_HELD) &&
			!IS_SET (item->flags, SUBCRAFT_OPTIONAL_ROOM)))
			continue;

		if (!obj_list[i])
			continue;

		/* Purge Consumed Craft Items */
		nObjectTally = item->item_counts;
		nObjectVnum = obj_list[i]->nVirtual;
		if (nObjectTally && ch->right_hand
			&& ch->right_hand->nVirtual == nObjectVnum)
		{
			if (ch->right_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->right_hand->count;
				extract_obj (ch->right_hand);
			}
			else
			{
				ch->right_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}

		if (nObjectTally && ch->left_hand
			&& ch->left_hand->nVirtual == nObjectVnum)
		{
			if (ch->left_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->left_hand->count;
				extract_obj (ch->left_hand);
			}
			else
			{
				ch->left_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}

		if (nObjectTally && ch->room->contents)
		{
			for (ptrObj = ch->room->contents; nObjectTally && ptrObj != NULL;)
			{
				if (ptrObj->nVirtual == nObjectVnum)
				{
					if (ptrObj->count <= nObjectTally)
					{
						nObjectTally -= ptrObj->count;
						ptrObjNext = ptrObj->next_content;
						extract_obj (ptrObj);
						ptrObj = ptrObjNext;
					}
					else
					{
						ptrObj->count -= nObjectTally;
						nObjectTally = 0;
						ptrObj = ptrObj->next_content;
					}
				}
				else
				{
					ptrObj = ptrObj->next_content;
				}
			}//for (ptrObj = ch->room->contents
		}//if (nObjectTally
	}//if (!obj_list[i])

	/* Create objects made by this phase */

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (phase_failed)
			continue;

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (!item->items[0])
			continue;

		if (!(IS_SET (item->flags, SUBCRAFT_PRODUCED)
			|| IS_SET (item->flags, SUBCRAFT_GIVE))
			|| item->phase != phase)
			continue;

		/* We need to account for the quantity of an item */

		/*
		*color = '\0';
		*color2 = '\0';
		*color3 = '\0';

		for (j = 0; j < MAX_ITEMS_PER_SUBCRAFT; j++)
		{
		item = af->a.craft->subcraft->obj_items[j];

		if (item && item->color && *item->color)
		{
		if(!*color)
		{
		sprintf (color, "%s", item->color);
		mem_free (item->color);
		}
		else if(!*color2)
		{
		sprintf (color2, "%s", item->color);
		mem_free (item->color);
		}
		else
		{
		sprintf (color3, "%s", item->color);
		mem_free (item->color);
		break;
		}
		}
		}
		*/

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		index = ch->craft_index;
		if ((i == subcraft->key_end) && !(i == 0))
			index = ch->craft_index;
		else
			index = 0;

		if (item->items[index] && vtoo (item->items[index]))
		{
			if (IS_SET (vtoo (item->items[index])->obj_flags.extra_flags, ITEM_VARIABLE))
			{
				obj_list[i] = load_exact_colored_object (item->items[index], subcraft->load_color[i][0], subcraft->load_color[i][1], subcraft->load_color[i][2],
					subcraft->load_color[i][3], subcraft->load_color[i][4], subcraft->load_color[i][5], subcraft->load_color[i][6],
					subcraft->load_color[i][7], subcraft->load_color[i][8], subcraft->load_color[i][9],
					subcraft->load_cat[i][0], subcraft->load_cat[i][1], subcraft->load_cat[i][2],
					subcraft->load_cat[i][3], subcraft->load_cat[i][4], subcraft->load_cat[i][5], subcraft->load_cat[i][6],
					subcraft->load_cat[i][7], subcraft->load_cat[i][8], subcraft->load_cat[i][9]);
				fluid_object(obj_list[i]);
			}
			else
			{
				obj_list[i] = load_object (item->items[index]);
				fluid_object(obj_list[i]);
			}
		}

		if (!obj_list[i])
		{
			act
				("An object produced by this craft was not found. Please inform the staff.",
				false, ch, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;
			return;
		}

		obj_list[i]->count = item->item_counts;

		if (is_clothing_item(obj_list[i]->nVirtual))
		{
			clothing_qualitifier(obj_list[i], ch);
		}

		if (IS_SET (item->flags, SUBCRAFT_GIVE))
		{
			obj_to_char (obj_list[i], ch);
		}
		else
		{
			obj_to_room (obj_list[i], ch->in_room);
		}
	}//for (i = 0;

	/** load mob on failure **/
	if (!phase_failed && phase->load_mob && vnum_to_mob (phase->load_mob))
	{

		if ((mob = load_mobile (phase->load_mob)))
		{
			char_to_room (mob, ch->in_room);

			if (get_affect (mob, MAGIC_HIDDEN))
				remove_affect_type (mob, MAGIC_HIDDEN);

			if (engine.in_play_mode ())
			{
				mob->act |= ACT_STAYPUT;
				save_stayput_mobiles ();
			}

			if (IS_SET (phase->nMobFlags, CRAFT_MOB_SETOWNER))
			{
				mob->mob->owner = str_dup (ch->tname);
			}
		}
	}

	/** Load objects on Failure ***/
	if (phase_failed)
	{
		if (!(subcraft->fails[1] == 0))
		{

			for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
			{
				item = af->a.craft->subcraft->fails[i];
				if (!item || !item->items[0] || item->phase != phase)
					continue;

				/* Fails - We need to account for the quantity of an item */

				/* *color = '\0';

				for (j = 1; j <= MAX_ITEMS_PER_SUBCRAFT; j++)
				{
				item = af->a.craft->subcraft->fails[j];
				if (item && item->color && *item->color)
				{
				sprintf (color, "%s", item->color);
				mem_free (item->color);
				break;
				}
				}
				*/

				item = af->a.craft->subcraft->fails[i];

				if (!item)
					continue;

				if (item->items[0] && vtoo (item->items[0]))
				{
					if (IS_SET (vtoo(item->items[0])->obj_flags.extra_flags, ITEM_VARIABLE))
					{
						fobj_list[i] = load_exact_colored_object (item->items[0], subcraft->load_color[i][0], subcraft->load_color[i][1], subcraft->load_color[i][2],
						subcraft->load_color[i][3], subcraft->load_color[i][4], subcraft->load_color[i][5], subcraft->load_color[i][6],
						subcraft->load_color[i][7], subcraft->load_color[i][8], subcraft->load_color[i][9],
						subcraft->load_cat[i][0], subcraft->load_cat[i][1], subcraft->load_cat[i][2],
						subcraft->load_cat[i][3], subcraft->load_cat[i][4], subcraft->load_cat[i][5], subcraft->load_cat[i][6],
						subcraft->load_cat[i][7], subcraft->load_cat[i][8], subcraft->load_cat[i][9]);
						fluid_object(fobj_list[i]);
					}
					else
					{
						fobj_list[i] = load_object (item->items[0]);
						fluid_object(fobj_list[i]);
					}
				}

				if (!fobj_list[i])
				{
					act
						("An object produced by this craft was not found. Please inform the staff.", false, ch, 0, 0, TO_CHAR);

					af->a.craft->timer = 0;
					return;
				}

				fobj_list[i]->count = item->item_counts;
				obj_to_room (fobj_list[i], ch->in_room);

			}

			/* 1st person failure message */
			if (phase->failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->failure, tool, fobj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else if (af->a.craft->subcraft->failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, af->a.craft->subcraft->failure, tool, fobj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else
				act ("You fail at your work.", false, ch, 0, 0, TO_CHAR);

			/* 2nd person failure message */
			if (target_ch == ch)  /* Don't give second to self. */
				;

			else if (phase->second_failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->second_failure, tool, fobj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, target_ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else
				act ("$n abruptly stops $s craft.", false, target_ch, 0, 0, TO_CHAR | _ACT_FORMAT);

			/* 3rd person failure message */
			if (phase->third_failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->third_failure, tool, fobj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, target_ch, TO_NOTVICT | _ACT_FORMAT);
			}

			else
				act ("$n abruptly stops $s craft.", false, ch, 0, target_ch, TO_NOTVICT | _ACT_FORMAT);

			// CPROGS FAIL PROG:
			if (IS_MORTAL(ch) && phase->phase_fail_prog)
			{
				std::multimap<std::string, room_prog>::iterator it;
				it = craft_prog_list.find(subcraft->subcraft_name);
				if (it != craft_prog_list.end())
				{
					std::pair<std::multimap<std::string, room_prog>::iterator, std::multimap<std::string, room_prog>::iterator> range = craft_prog_list.equal_range(subcraft->subcraft_name);
					for (std::multimap<std::string, room_prog>::iterator bit = range.first; bit != range.second; ++bit)
					{
						if (!str_cmp(bit->second.command, phase->phase_fail_prog))
						{
							c_prog(ch, phase->phase_fail_prog, bit->second);
							break;
						}
					}
				}
			}
		}

		/** old style failure system **/
		else
		{
			/* 1st person failure message - old style */
			if (phase->failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->failure, tool, obj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else if (af->a.craft->subcraft->failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, af->a.craft->subcraft->failure, tool, obj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else
				act ("You fail at your work.", false, ch, 0, 0, TO_CHAR);

			/* 2nd person failure message - old style */
			if (target_ch == ch)  /* Don't give second to self. */
				;

			else if (phase->second_failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->second_failure, tool, obj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, target_ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else
				act ("$n abruptly stops $s craft.", false, target_ch, 0, 0, TO_CHAR | _ACT_FORMAT);

			/* 3rd person failure message - old style */
			if (phase->third_failure)
			{
				craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->third_failure, tool, obj_list);

				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);

				act (buf, false, ch, 0, target_ch, TO_NOTVICT | _ACT_FORMAT);
			}

			else
				act ("$n abruptly stops $s craft.", false, ch, 0, target_ch, TO_NOTVICT | _ACT_FORMAT);

			/* Group fail message */

			if ((subcraft->followers > 0)
				&& (num_followers (ch) >= subcraft->followers)
				&& phase->fail_group_mess)
			{
				craft_prepare_message (ch, buf, ch, target_ch, tch,
					phase->fail_group_mess, tool, obj_list);
				if (*buf == '#')
					buf[2] = toupper (buf[2]);
				else
					*buf = toupper (*buf);
				act (buf, false, ch, 0, 0, TO_GROUP | _ACT_FORMAT);
			}

			/* Load failure objects */
			count = 0;
			p = subcraft->failobjs;

			while (p != NULL && *p != '\0' && *p != ')')
			{
				/* punt leading whitespace */
				while (isspace (*p) && *p != '\0')
					p++;

				/* Make sure there is no whitespace inside */
				while (isspace (*p) && *p != '\0')
					p++;

				/* Now if we have an 'x', we have a count */
				if (*p == 'x')
				{
					p++;

					while (!isdigit (*p) && *p != '\0')
						p++;

					for (i = 0; isdigit (*p); i++, p++)
						numbuf[i] = *p;

					numbuf[i] = '\0';
					count = atoi (numbuf);
				}

				while (!isdigit (*p) && *p != '\0')
					p++;

				for (i = 0; isdigit (*p); i++, p++)
					numbuf[i] = *p;

				numbuf[i] = '\0';

				if (i > 0)
				{
					/* Then we have some kind of # */
					failobj = load_object (atoi (numbuf));
					fluid_object(failobj);

					if (failobj != NULL)
					{
						if (count > 0)
							failobj->count = count;

						obj_to_room (failobj, ch->in_room);
					}
				}
			}

			p = subcraft->failmobs;

			while (p != NULL && *p != '\0' && *p != ')')
			{
				/* punt leading whitespace */
				while (isspace (*p) && *p != '\0')
					p++;

				/* Make sure there is no whitespace inside */
				while (isspace (*p) && *p != '\0')
					p++;

				/* Now if we have an 'x', we have a count */
				if (*p == 'x')
				{
					p++;

					while (!isdigit (*p) && *p != '\0')
						p++;

					for (i = 0; isdigit (*p); i++, p++)
						numbuf[i] = *p;

					numbuf[i] = '\0';
					count = atoi (numbuf);
				}

				while (!isdigit (*p) && *p != '\0')
					p++;

				for (i = 0; isdigit (*p); i++, p++)
					numbuf[i] = *p;

				numbuf[i] = '\0';

				if (i > 0)
				{
					/* Then we have some kind of # */
					if ((failmob = load_mobile (atoi (numbuf))))
					{
						char_to_room (failmob, ch->in_room);

						if (get_affect (failmob, MAGIC_HIDDEN))
							remove_affect_type (failmob, MAGIC_HIDDEN);

						failmob->act |= ACT_STAYPUT;
						save_stayput_mobiles ();
					}
				}
			}

			// CPROGS FAIL PROG:
			if (phase->phase_fail_prog)
			{
				std::multimap<std::string, room_prog>::iterator it;
				it = craft_prog_list.find(subcraft->subcraft_name);
				if (it != craft_prog_list.end())
				{
					std::pair<std::multimap<std::string, room_prog>::iterator, std::multimap<std::string, room_prog>::iterator> range = craft_prog_list.equal_range(subcraft->subcraft_name);
					for (std::multimap<std::string, room_prog>::iterator bit = range.first; bit != range.second; ++bit)
					{
						if (!str_cmp(bit->second.command, phase->phase_fail_prog))
						{
							c_prog(ch, phase->phase_fail_prog, bit->second);
							break;
						}
					}
				}
			}
		} //end of old fail system

		if (af->a.craft->subcraft->faildelay)
		{
			delay_time = time (0) + figure_craft_faildelay (ch, af->a.craft->subcraft);
			magic_add_affect (ch, MAGIC_CRAFT_DELAY, -1, delay_time, 0, 0, 0);
		}

		af->a.craft->timer = 0;
		return;
	}

	/* First person message */
	if (phase->first || phase->self)
	{

		first = phase->first;

		/* Use self text, if it exists,  if PC targets himself */

		if (phase->self &&
			target_ch == ch &&
			IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR))
			first = phase->self;

		craft_prepare_message (ch, buf, ch, target_ch, NULL,
			first, tool, obj_list);

		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	/* Second person message */
	if (phase->second && target_ch && target_ch != ch)
	{

		craft_prepare_message (target_ch, buf, ch,
			target_ch, NULL, phase->second, tool, obj_list);

		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		act (buf, false, target_ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	/* Third person message */
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if (!phase->third)
			break;

		if (tch == ch || tch == target_ch)
			continue;

		craft_prepare_message (tch, buf, ch, target_ch, tch,
			phase->third, tool, obj_list);
		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		act (buf, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	/* Group members message */

	if ((subcraft->followers > 0)
		&& (num_followers (ch) >= subcraft->followers)
		&& phase->group_mess)
	{
		craft_prepare_message (ch, buf, ch, target_ch, tch,
			phase->group_mess, tool, obj_list);
		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		act (buf, false, ch, 0, 0, TO_GROUP | _ACT_FORMAT);
	}

	/* Cast any spells from this phase */

	if (phase->spell_type)
	{

		/* Handle casting and first and second person messages */

		if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_SELF) ||
			IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR))
		{

			apply_affect (target_ch, phase->spell_type, phase->duration,
				phase->power);
		}
	}

	if (phase->open_skill)
	{
		if (IS_SET (phase->flags, PHASE_OPEN_ON_SELF) &&
			!real_skill (ch, phase->open_skill))
			open_skill (ch, phase->open_skill);

		else if (!IS_SET (phase->flags, PHASE_OPEN_ON_SELF) &&
			!real_skill (target_ch, phase->open_skill))
			open_skill (target_ch, phase->open_skill);
	}

	if (IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) &&
		!is_he_here (ch, target_ch, 1))
	{
		send_to_char ("Your target is no longer here.\n", ch);
		af->a.craft->timer = 0;
		return;
	}

	if (subcraft->followers > 0) //followers suffer
		craft_group_move(ch, phase->move_cost, "cost of craft");

	if (weaken (ch, 0, phase->move_cost, "cost of craft"))
		return;			/* person running the craft died of exhaustion*/

	if (subcraft->followers > 0) //followers suffer
		craft_group_pain (ch, phase->hit_cost);

	if (general_damage (ch, phase->hit_cost))
		return; /* person running the craft died from the pain */

	/* Immediately activate next phase after a phase with 0 timer */
	if (!af->a.craft->timer && af->a.craft->phase)
		activate_phase (ch, af);
	else if (!af->a.craft->phase
		&& requires_skill_check (af->a.craft->subcraft))
	{
		if (number (1, 100) <= branching_check (ch, af->a.craft->subcraft))
			branch_craft (ch, af->a.craft->subcraft);
	}

	// CPROGS END OF PHASE

	if (IS_MORTAL(ch) && phase->phase_end_prog)
	{
		std::multimap<std::string, room_prog>::iterator it;
		it = craft_prog_list.find(subcraft->subcraft_name);
		if (it != craft_prog_list.end())
		{
			std::pair<std::multimap<std::string, room_prog>::iterator, std::multimap<std::string, room_prog>::iterator> range = craft_prog_list.equal_range(subcraft->subcraft_name);
			for (std::multimap<std::string, room_prog>::iterator bit = range.first; bit != range.second; ++bit)
			{
				if (!str_cmp(bit->second.command, phase->phase_end_prog))
				{
					c_prog(ch, phase->phase_end_prog, bit->second);
					break;
				}
			}
		}
	}

	// Load progressed objects.
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (prog_load_obj_list[i] && prog_take_obj_list[i])
		{
			obj_to_room(prog_load_obj_list[i], ch->in_room);
			act ("You finish $p, producing $P.", false, ch, prog_take_obj_list[i], prog_load_obj_list[i], TO_CHAR | _ACT_FORMAT);
			act ("$n finishes $p, producing $P.", false, ch, prog_take_obj_list[i], prog_load_obj_list[i], TO_ROOM | _ACT_FORMAT);

			if (is_clothing_item(prog_load_obj_list[i]->nVirtual))
			{
				clothing_qualitifier(prog_load_obj_list[i], ch);
			}

			extract_obj(prog_take_obj_list[i]);
		}
	}

	if (af->a.craft->subcraft->delay && !af->a.craft->phase)
	{
		delay_time = time (0) + figure_craft_delay (ch, af->a.craft->subcraft);
		magic_add_affect (ch, MAGIC_CRAFT_DELAY, -1, delay_time, 0, 0, 0);
	}


}

void
	craft_setup (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char output[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;


	if (!str_cmp (subcmd, "craft"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			send_to_char ("Craft name contains illegal characters.\n", ch);
			return;
		}
		craft->craft_name = add_hash (buf);
		sprintf (output, "Craft name changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if (!str_cmp (subcmd, "subcraft"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			send_to_char ("Subcraft name contains illegal characters.\n", ch);
			return;
		}
		craft->subcraft_name = add_hash (buf);
		sprintf (output, "Subcraft name changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if (!str_cmp (subcmd, "command"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			send_to_char ("Command name contains illegal characters.\n", ch);
			return;
		}
		craft->command = add_hash (buf);
		sprintf (output, "Craft command changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if (!str_cmp (subcmd, "hidden") || !str_cmp (subcmd, "obscure"))
	{
		TOGGLE_BIT (craft->subcraft_flags, SCF_OBSCURE);
		if (IS_SET (craft->subcraft_flags, SCF_OBSCURE))
		{
			sprintf (output,
				"DELETE FROM new_crafts WHERE subcraft = '%s'",
				craft->subcraft_name);
			mysql_safe_query (output);
			sprintf (output, "DELETE FROM crafts WHERE subcraft = '%s'",
				craft->subcraft_name);
			mysql_safe_query (output);
			send_to_char
				("This craft will not be displayed on the website or in the Palantir Weekly.\n",
				ch);
		}
		else
		{
			send_to_char
				("This craft will be displayed on the website after the next reboot.\n",
				ch);
		}
		return;
	}

}

void
	craft_sectors (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH];
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (sector_types, buf);
	else
		index = -1;

	if (index == -1)
	{
		send_to_char ("That isn't a recognized sector type.\n", ch);
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->sectors[i])
			j = i;
		else if (craft->sectors[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->sectors[i] = 0;
		send_to_char
			("Craft no longer requires the specified sector.\n",
			ch);
		return;
	}

	else if (i == -1 && j == -1)
	{
		send_to_char
			("That craft's list of required sectors is currently full.\n",
			ch);
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->sectors[j] = index + 1;
		send_to_char
			("The specified sector has been added to the list of required   sector types.\n",
			ch);
		return;
	}
}

void
	craft_seasons (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH];
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (seasons, buf);
	else
		index = -1;

	if (index == -1)
	{
		send_to_char ("That isn't a recognized seasons.\n", ch);
		return;
	}

	for (j = -1, i = 5; i >= 0; i--)
	{
		if (!craft->seasons[i])
			j = i;
		else if (craft->seasons[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->seasons[i] = 0;
		send_to_char
			("Craft no longer requires the specified season.\n",
			ch);
		return;
	}

	else if (i == -1 && j == -1)
	{
		send_to_char
			("That craft's list of required seasons is currently full.\n",
			ch);
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->seasons[j] = index + 1;
		send_to_char
			("The specified season has been added to the list of required seasons.\n",
			ch);
		return;
	}

	return;
}

void
	craft_opening (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH];
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (skills, buf);
	else
		index = -1;

	if (index == -1)
	{
		send_to_char ("That isn't a recognized skill name.\n", ch);
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->opening[i])
			j = i;
		else if (craft->opening[i] == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->opening[i] = 0;
		send_to_char
			("Craft no longer an opening craft for the specified skill.\n",
			ch);
		return;
	}

	else if (i == -1 && j == -1)
	{
		send_to_char
			("That craft's list of skills it opens for is currently full.\n",
			ch);
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->opening[j] = index;
		send_to_char
			("This craft will now open for the specified skill.\n",
			ch);
		return;
	}

	return;
}

void
	craft_race (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH];
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = lookup_race_id(buf);
	else
		index = -1;

	if (index == -1)
	{
		send_to_char ("That isn't a recognized race name.\n", ch);
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->race[i])
			j = i;
		else if (craft->race[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->race[i] = 0;
		send_to_char
			("That race is no longer required for this craft.\n",
			ch);
		return;
	}

	else if (i == -1 && j == -1)
	{
		send_to_char
			("That craft's list of required races is currently full.\n",
			ch);
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->race[j] = index + 1;
		send_to_char
			("This craft will now require the specified race.\n",
			ch);
		return;
	}
	return;
}

void
	craft_clan (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int flags = 0;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);
	argument = one_argument (argument, buf2);

	if (!get_clandef (buf))
	{
		send_to_char ("That is not a recognized clan.\n", ch);
		return;
	}

	if (get_clan_in_string (craft->clans, buf, &flags))
	{
		if (!*buf2)
		{
			craft->clans =
				str_dup (remove_clan_from_string (craft->clans, buf));
			send_to_char
				("The specified clan requirement has been removed.\n",
				ch);
			return;
		}

		else if (!(flags = clan_flags_to_value (buf2)))
		{
			send_to_char
				("That is not a recognized clan rank.\n",
				ch);
			return;
		}

		else
		{
			craft->clans =
				str_dup (remove_clan_from_string (craft->clans, buf));
			craft->clans =
				str_dup (add_clan_to_string (craft->clans, buf, flags));
			send_to_char
				("The rank requirement for the specified clan has been updated.\n",
				ch);
			return;
		}
	}

	else
	{
		if (!*buf2)
			sprintf (buf2, "member");

		if (*buf2 && !(flags = clan_flags_to_value (buf2)))
		{
			send_to_char
				("That is not a recognized clan rank.\n",
				ch);
			return;
		}

		if (flags)
		{
			craft->clans =
				str_dup (add_clan_to_string (craft->clans, buf, flags));
		}
		else
		{
			craft->clans =
				str_dup (add_clan_to_string (craft->clans, buf, CLAN_MEMBER));
		}

		send_to_char
			("The specified clan and rank have been added as requirements.\n",
			ch);
		return;
	}
	return;
}


void
	craft_delete (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;
	SUBCRAFT_HEAD_DATA *tmpcraft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf != '!')
	{
		send_to_char
			("If you are SURE you'd like to delete this craft, please use 'cset delete !'.\n",
			ch);
		return;
	}

	mysql_safe_query
		("DELETE FROM new_crafts WHERE subcraft = '%s' AND craft = '%s'",
		craft->subcraft_name,
		craft->craft_name);

	sprintf (output,
		"The %s craft has been deleted.\n",
		craft->subcraft_name);

	if (crafts == craft)
		crafts = crafts->next;
	else
	{
		for (tmpcraft = crafts; tmpcraft; tmpcraft = tmpcraft->next)
			if (tmpcraft->next == craft)
				tmpcraft->next = tmpcraft->next->next;
	}

	send_to_char (output, ch);
	ch->pc->edit_craft = NULL;
	return;
}

void
	craft_failure (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;
	if (!*argument && craft->failure)
	{
		craft->failure = NULL;
		send_to_char
			("Failure string deleted\n",
			ch);
		return;
	}

	else if (!*argument && !craft->failure)
	{
		send_to_char
			("What do you want the header failure message to be?\n",
			ch);
		return;
	}

	else
	{
		craft->failure = add_hash (argument);
		sprintf (output, "Header Failure string modified.\n");
		send_to_char (output, ch);
	}
}

/** failure objects stored in a string, NOT array **/
void
	craft_failobjs (CHAR_DATA * ch, char *argument, char *subcmd)
{
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		send_to_char ("Which failure object(s) did you wish to set?\n", ch);
		return;
	}

	if (!isdigit (*argument))
	{
		send_to_char
			("You must specify a list of VNUMs, ie. cset failobjs VNUM1 VNUM2 VNUM3.\n",
			ch);
		return;
	}

	if (atoi (argument) == 0)
	{
		craft->failobjs = NULL;
		send_to_char ("Failure object list deleted.\n", ch);
		return;
	}
	else
	{
		craft->failobjs = add_hash (argument);
		send_to_char ("Failure object inserted.\n", ch);
		return;
	}

	return;
}


/** failure mobs stored in a string, NOT array **/
void
	craft_failmobs (CHAR_DATA * ch, char *argument, char *subcmd)
{

	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		send_to_char ("What mobile did you wish to set?\n", ch);
		return;
	}

	if (!isdigit (*argument))
	{
		send_to_char
			("You must specify a VNUMs, e.g. 'cset failmobs VNUM1 VNUM2 VNUM3'.\n",
			ch);
		return;
	}

	if (atoi (argument) == 0)
	{
		craft->failmobs = NULL;
		send_to_char ("Failure mobile list deleted.\n", ch);
		return;
	}

	else
	{
		craft->failmobs = add_hash (argument);
		send_to_char ("Failure mobile list inserted.\n", ch);
		return;
	}
	return;
}

void
	craft_delay (CHAR_DATA * ch, char *argument, char *subcmd)
{
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	craft = ch->pc->edit_craft;


	if (!*buf)
	{
		send_to_char
			("How many OOC hours did you want to set the delay timer to?\nUse <cset delay X fail> for failure delay.\n",
			ch);
		return;
	}

	if (!isdigit (*buf))
	{
		send_to_char
			("You must specify a number of RL hours to set the timer to.\n",
			ch);
		return;
	}

	if (!str_cmp (argument, "fail"))
	{
		craft->faildelay = atoi (buf);
		send_to_char ("The craft's OOC Failure delay timer has been set.\n", ch);
		return;
	}

	else
	{
		craft->delay = atoi (buf);
		send_to_char ("The craft's OOC delay timer has been set.\n", ch);
		return;
	}
}

void
	craft_key (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		send_to_char
			("Which object list did you want to use as the key?\n",
			ch);
		return;
	}

	if (!isdigit (*argument))
	{
		send_to_char
			("You must specify the number of the object list that you wish to use as the key.\n",
			ch);
		return;
	}

	craft->key_first = atoi (argument);
	if (craft->key_first == 0)
	{
		send_to_char
			("The craft's key objects lists have been deleted.\n",
			ch);
		craft->key_end = -1;
		craft->key_first = -1;
		return;
	}
	else
	{
		sprintf
			(output,
			"The craft's key objects list has been set to %d.\n",
			craft->key_first );
		send_to_char (output, ch);
		return;
	}
	return;
}

void
	craft_key_product (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		send_to_char
			("Which object list did you want to use for key-production?\n",
			ch);
		return;
	}

	if (!isdigit (*argument))
	{
		send_to_char
			("You must specify the number (1-9) of the object list that you wish to use for key-production. \n",
			ch);
		return;
	}

	craft->key_end = atoi (argument);

	if (craft->key_end == 0)
	{
		send_to_char
			("The craft's key objects lists have been deleted.\n",
			ch);
		craft->key_end = -1;
		craft->key_first = -1;
		return;
	}
	else
	{
		sprintf
			(output,
			"The craft's key-production list has been set to %d.\n",
			craft->key_end );
		send_to_char (output, ch);
		return;
	}
	return;
}

void
	craft_weather (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH];
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (weather_states, buf);
	else
		index = -1;

	if (index == -1)
	{
		send_to_char ("That isn't a recognized weather state.\n", ch);
		return;
	}

	for (j = -1, i = 8; i >= 0; i--)
	{
		if (!craft->weather[i])
			j = i;
		else if (craft->weather[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->weather[i] = 0;
		send_to_char
			("Craft no longer requires the specified weather.\n",
			ch);
		return;
	}

	else if (i == -1 && j == -1)
	{
		send_to_char
			("That craft's list of required weather is currently full.\n",
			ch);
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->weather[j] = index + 1;
		send_to_char
			("The specified weather has been added to the list of required weather.\n",
			ch);
		return;
	}

	return;
}

void
	craft_group (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH];
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		send_to_char
			("How many followers do you need in your group?\n",
			ch);
		return;
	}

	if (!isdigit (*argument))
	{
		send_to_char
			("You must specify the number of followers that you wish to require.\n",
			ch);
		return;
	}

	craft->followers = atoi (argument);
	if (craft->followers == 0)
	{
		send_to_char
			("The craft's group requiremnt have been deleted.\n",
			ch);
		return;
	}
	else
	{
		sprintf
			(output,
			"This craft now requires %d followers in the leaders group.\n",
			craft->followers );
		send_to_char (output, ch);
		return;
	}
	return;
}

// called when giving pain to groups while crafting
void
	craft_group_move (CHAR_DATA * ch, int move_cost, char *message)
{
	CHAR_DATA *top_leader = NULL;
	CHAR_DATA *tch = NULL;

	if (!(top_leader = ch->following))
		top_leader = ch;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{

		if (tch->deleted)
			continue;

		if (tch->following == ch)
			weaken (tch, 0, move_cost, "cost of craft");
	}
	return;
}

// called when giving pain to groups while crafting
void
	craft_group_pain (CHAR_DATA * ch, int pain_cost)
{
	CHAR_DATA *top_leader;
	CHAR_DATA *tch = NULL;

	if (!(top_leader = ch->following))
		top_leader = ch;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{

		if (tch->deleted)
			continue;

		if (tch->following == ch)
			general_damage (tch, pain_cost);
	}
	return;
}

void
	craftstat (CHAR_DATA * ch, char *argument)
{
	int i;
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH];
	bool racial = false;
	bool opening = false;
	bool sectors = false;
	bool seasonchk = false;
	bool weatherchk = false;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char
			("Which craft did you wish to view?", ch);
		return;
	}

	for (craft = crafts;
		craft && str_cmp (craft->subcraft_name, buf);
		craft = craft->next)
		;

	if (!craft)
	{
		send_to_char ("No such craft.  Type 'crafts' for a listing.\n", ch);
		return;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if (!(af = get_affect (ch, i)))
			continue;
		if (af->a.craft->subcraft == craft)
			break;
	}

	/** displays information **/
	*buf = '\0';

	sprintf (b_buf, "#6Craft:#0 %s #6Subcraft:#0 %s #6Command:#0 %s\n",
		craft->craft_name, craft->subcraft_name, craft->command);

	for (i = 0; i <= 24; i++)
	{
		if (craft->race[i] > 0)
		{
			racial = true;
			sprintf (buf + strlen (buf), " %s",
				lookup_race_variable (craft->race[i] - 1, RACE_NAME));
		}
	}

	if (racial)
		sprintf (b_buf + strlen (b_buf), "  #6Race:#0 %s\n", buf);

	if (craft->clans && strlen (craft->clans) > 3)
		sprintf (b_buf + strlen (b_buf), "  #6Clans:#0 %s\n", craft->clans);

	opening = false;

	*buf = '\0';

	for (i = 0; i <= 24; i++)
	{
		if (craft->opening[i] > 0)
		{
			opening = true;
			sprintf (buf + strlen (buf), " %s", skills[craft->opening[i]]);
		}
	}

	if (opening)
		sprintf (b_buf + strlen (b_buf), "  #6Base:#0  %s\n", buf);

	/** sectors **/
	for (i = 0; i <= 24; i++)
		if (craft->sectors[i] > 0)
			sectors = true;

	if (sectors)
	{
		sprintf (b_buf + strlen (b_buf), "  #6Sectors:#0 ");
		for (i = 0; i <= 24; i++)
			if (craft->sectors[i])
				sprintf (b_buf + strlen (b_buf), "%s ",
				sector_types[craft->sectors[i] - 1]);
		sprintf (b_buf + strlen (b_buf), "\n");
	}

	/** seasons **/
	for (i = 0; i <= 5; i++)
		if (craft->seasons[i] > 0)
			seasonchk = true;

	if (seasonchk)
	{
		sprintf (b_buf + strlen (b_buf), "  #6Seasons:#0 ");
		for (i = 0; i <= 5; i++)
			if (craft->seasons[i])
				sprintf (b_buf + strlen (b_buf), "%s ",
				seasons[craft->seasons[i] - 1]);
		sprintf (b_buf + strlen (b_buf), "\n");
	}

	/** weather **/
	for (i = 0; i <= 8; i++)
	{
		if (craft->weather[i] > 0)
			weatherchk = true;
	}

	if (weatherchk)
	{
		sprintf (b_buf + strlen (b_buf), "  #6Weather:#0 ");
		for (i = 0; i <= 8; i++)
			if (craft->weather[i])
				sprintf (b_buf + strlen (b_buf), "%s ",
				weather_states[craft->weather[i] - 1]);
		sprintf (b_buf + strlen (b_buf), "\n");
	}

	/** group **/
	if (craft->followers > 0)
		sprintf (b_buf + strlen (b_buf), "  #6Followers:#0 %d \n",
		craft->followers);

	/** delay **/
	if (craft->delay)
		sprintf (b_buf + strlen (b_buf), "  #6OOC Delay Timer:#0 %d RL Hours\n",
		craft->delay);

	if (craft->faildelay)
		sprintf (b_buf + strlen (b_buf), "  #6OOC Delay Timer:#0 %d RL Hours\n",
		craft->faildelay);

	if (craft->failure)
		sprintf (b_buf + strlen (b_buf), "  #6Fail:#0 %s\n", craft->failure);

	if (craft->failobjs)
		sprintf (b_buf + strlen (b_buf), "  #6Failobjs:#0 %s\n", craft->failobjs);

	if (craft->failmobs)
		sprintf (b_buf + strlen (b_buf), "  #6Failmobs:#0 %s\n", craft->failmobs);

	if (craft->key_first > 0)
		sprintf (b_buf + strlen (b_buf), "  #6Primary Key:#0 %d\n", craft->key_first);

	if (craft->key_end > 0)
		sprintf (b_buf + strlen (b_buf), "  #6Product Key:#0 %d\n", craft->key_end);

	display_craft (ch, craft);

	return;

}

//list the crafts of a PC
void
	do_craftspc (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	std::string output;
	AFFECTED_TYPE *af;

	argument = one_argument(argument, buf);

	tch = load_pc(buf);
	if (tch)
	{
		sprintf (buf2,
			"\n#6%s#0 has the following crafts on %s pfile:\n\n",
			tch->tname,
			HSHR (tch));

		output.assign(buf2);

		for (af = tch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{
				sprintf (buf2,
					"  #6Craft:#0 %-20s #6Subcraft:#0 %-12s #6Command:#0 %-10s\n",
					af->a.craft->subcraft->craft_name,
					af->a.craft->subcraft->subcraft_name,
					af->a.craft->subcraft->command);

				output.append(buf2);
			}
		}

		unload_pc (tch);
		page_string (ch->descr(), output.c_str());
		return;
	}
	return;
}

