/*------------------------------------------------------------------------\
|  arena.c : Arena Combat Processor                   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <mysql/mysql.h>
#include <dirent.h>
#include <signal.h>

#include "server.h"

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"



void
	equip_slave (CHAR_DATA * tmob, int cmd)
{
	OBJ_DATA *tobj;
	OBJ_DATA *obj;
	int weapon;

	if (cmd == 0)
		cmd = number(1,4);

	for (tobj = tmob->equip; tobj; tobj = tobj->next_content)
	{
		if (tobj == tmob->equip)
			tmob->equip = tmob->equip->next_content;
		else
			tmob->equip->next_content = tobj->next_content;
	}
	tmob->right_hand = NULL;
	tmob->left_hand = NULL;

	if ((obj = load_object (55001)))
		equip_char (tmob, obj, WEAR_ABOUT);

	switch (cmd)
	{
	case 1:
		if ((obj = load_object (1102)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_PRIM);
		}
		if ((obj = load_object (4013)))
		{
			obj_to_char (obj, tmob);
		}
		break;
	case 2:
		if ((obj = load_object (1031)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_BOTH);
		}
		break;
	case 3:
		if ((obj = load_object (1107)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_PRIM);
		}
		if ((obj = load_object (4013)))
		{
			obj_to_char (obj, tmob);
		}
		break;
	case 5:
		if ((obj = load_object (1102)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_BOTH);
		}
		break;
	case 6:
		if ((obj = load_object (1107)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_BOTH);
		}
		break;
	default:
		if ((obj = load_object (1103)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_PRIM);
		}
		if ((obj = load_object (1103)))
		{
			obj_to_char (obj, tmob);
			equip_char (tmob, obj, WEAR_SEC);
		}
		break;
	};
}


int in_arena_room(CHAR_DATA *ch)
{
	if (!ch->in_room)
		return 0;
	else if (ch->in_room == GRUNGE_SMALL1 ||
		ch->in_room == GRUNGE_SMALL2)
		return 2;
	else if (ch->in_room == GRUNGE_ARENA1 ||
		ch->in_room == GRUNGE_ARENA2 ||
		ch->in_room == GRUNGE_ARENA3 ||
		ch->in_room == GRUNGE_ARENA4)
		return 1;
	else
		return 0;
}

int is_arena_room(int vnum)
{
	if (!vnum)
		return 0;
	else if (vnum == GRUNGE_SMALL1 ||
		vnum == GRUNGE_SMALL2)
		return 2;
	else if (vnum == GRUNGE_ARENA1 ||
		vnum == GRUNGE_ARENA2 ||
		vnum == GRUNGE_ARENA3 ||
		vnum == GRUNGE_ARENA4)
		return 1;
	else
		return 0;
}

const int grunge_arena_rooms[4] =
{
	GRUNGE_ARENA1,
	GRUNGE_ARENA2,
	GRUNGE_ARENA3,
	GRUNGE_ARENA4
};

const int grunge_private_arena_rooms[4] =
{
	GRUNGE_SMALL1,
	GRUNGE_SMALL2
};

#define PITBAG_DESC_PREPEND "Belongings for"
#define VNUM_PITBAG 55000

bool grunge_arena_first_fight = false;
int grunge_arena_first_fighters = 0;
bool grunge_arena_second_fight = false;
int grunge_arena_second_fighters = 0;

bool grunge_arena_stopped = false;
bool grunge_arena_fight = false;
bool grunge_arena_barker_check = false;
int grunge_arena_second_echo = 0;
int grunge_arena_third_echo = 0;
int grunge_arena_cleanup_echo = 0;
int grunge_arena_last_echo = 0;
int red = 0, blue = 0, barkers = 0; // count the numbers actually in the pit.
int grunge_blue_max = 5;
int grune_red_max = 5;

bool blue_win = false;
bool red_win = false;


void
	arena_strip (CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *bag = NULL;
	char buf[MAX_STRING_LENGTH];

	if (!( vnum_to_room(VNUM_DROPROOM)))
	{
		system_log("ERROR: Room does not exist in r_strip", true);
	}
	else
	{
		bag = load_object (VNUM_JAILBAG);

		if (bag && (ch->right_hand || ch->left_hand || ch->equip))
		{
			sprintf (buf, "bag %s OTHER", ch->tname);
			mem_free(bag->name);
			bag->name = str_dup (buf);

			// Can only get a bag if you have the coldload_id to prevent rampant thievery.
			bag->o.od.value[4] = ch->coldload_id;

			if (ch->right_hand)
			{
				obj = ch->right_hand;
				obj_from_char (&obj, 0);
				if (bag)
					obj_to_obj (obj, bag);
			}

			if (ch->left_hand)
			{
				obj = ch->left_hand;
				obj_from_char (&obj, 0);
				if (bag)
					obj_to_obj (obj, bag);
			}

			while (ch->equip)
			{
				obj = ch->equip;
				if (bag)
					obj_to_obj (unequip_char (ch, obj->location), bag);
			}

			obj_to_room (bag, VNUM_DROPROOM);
		}
	}
}

void
	grunge_arena_barkers (void)
{
	/*
	char buf[MAX_STRING_LENGTH];
	int i, numwargs;
	CHAR_DATA *tch;

	if (morgul_arena_warg_check)
	return;

	if (!vtom (60001))
	return;

	numwargs = number (5, 8);
	sprintf (buf,
	"Frenzied barks and howls crowd out even the sounds of carnage from the the battefield, as a pack of rabid wolves cross the hill and descend on to the combatants, tearing and savaging men and orcs alike!");
	send_to_room (buf, 60001);
	send_to_room (buf, 60002);
	send_to_room (buf, 60003);
	send_to_room (buf, 60005);

	for (i = 1; i <= numwargs; i++)
	{
	tch = load_mobile (60002);
	char_to_room (tch, 60002);
	}

	morgul_arena_warg_check = true;
	wargs = numwargs;
	*/
}

extern rpie::server engine;

#define ARENA_NOT_REGISTERED    "You need to register before you can fight in the Arena. Ask me how by typing #6ARENA REGISTER#0."
#define ARENA_TOO_INJURED       "I'm not letting you fight like that - you're too injured to give people a decent show. You need to be in prime condition to make it worth my while."
#define ARENA_TOO_FULL          "I'd love to let you fight, but all the arenas are full right now."
#define ARENA_DEFINE_WEAPONS    "You need to tell me what weapons you want to fight with -- options are longblade, polearm, bludgeon, smallblade, longblade-only or bludgeon-only."
#define ARENA_READY_FIGHT		"Yes, yes, you look ready to fight. Remember to see #6HELP ARENA#0 if you have any problems."
#define ARENA_NO_CROWD			"You missed the deadline for entrants for the latest crowd fight. It should be an hour or so before we have another one -- come back here then."
#define ARENA_FIGHT_TYPE		"You need to tell me what kind of what you want - a #6crowd#0 fight, or a #6personal#0 fight."


void
	arena_equip_pc_fighter(CHAR_DATA *ch, int cmd)
{
	OBJ_DATA *bag = NULL;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH];
	bag = load_object (VNUM_JAILBAG);
	int drop_room = VNUM_DROPROOM;

	if (!( vnum_to_room(drop_room)))
	{
		send_to_gods("ERROR: Room does not exist in arena_equip_pc_fighter");
		return;
	}

	if (bag && (ch->right_hand || ch->left_hand || ch->equip))
	{
		sprintf (buf, "bag %s OTHER", ch->tname);
		mem_free(bag->name);
		bag->name = str_dup (buf);

		if (ch->right_hand)
		{
			obj = ch->right_hand;
			obj_from_char (&obj, 0);
			if (bag)
				obj_to_obj (obj, bag);
		}

		if (ch->left_hand)
		{
			obj = ch->left_hand;
			obj_from_char (&obj, 0);
			if (bag)
				obj_to_obj (obj, bag);
		}

		while (ch->equip)
		{
			obj = ch->equip;
			if (bag)
				obj_to_obj (unequip_char (ch, obj->location), bag);
		}

		obj_to_room (bag, drop_room);
	}


	if ((obj = load_object (55001)))
		equip_char (ch, obj, WEAR_ABOUT);

	switch (cmd)
	{
	case 1:
		if ((obj = load_object (1102)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_PRIM);
		}
		if ((obj = load_object (4013)))
		{
			obj_to_char (obj, ch);
		}
		break;
	case 2:
		if ((obj = load_object (1031)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_BOTH);
		}
		break;
	case 3:
		if ((obj = load_object (1107)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_PRIM);
		}
		if ((obj = load_object (4013)))
		{
			obj_to_char (obj, ch);
		}
		break;
	case 5:
		if ((obj = load_object (1102)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_BOTH);
		}
		break;
	case 6:
		if ((obj = load_object (1107)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_BOTH);
		}
		break;

	default:
		if ((obj = load_object (1103)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_PRIM);
		}
		if ((obj = load_object (1103)))
		{
			obj_to_char (obj, ch);
			equip_char (ch, obj, WEAR_SEC);
		}
		break;
	};
}

void
	do_arena (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, argOne, argTwo, argThree, argFour, argGladiator, output;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	int season = 1, i = 0, j = 0;
	bool large_fight = false;
	int weapon_type = 0;
	std::string player_db = engine.get_config ("player_db");
	MYSQL_RES *result;
	MYSQL_ROW row;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *overseer = NULL;

	if (strArgument == "leave")
	{
		if (ch->in_room == GRUNGE_SMALL1 || ch->in_room == GRUNGE_SMALL2 ||
			ch->in_room == GRUNGE_ARENA1 || ch->in_room == GRUNGE_ARENA2 ||
			ch->in_room == GRUNGE_ARENA3 || ch->in_room == GRUNGE_ARENA4)
		{
			if ((ch->in_room == GRUNGE_SMALL1 && grunge_arena_first_fighters == 2) ||
				(ch->in_room == GRUNGE_SMALL2 && grunge_arena_second_fighters == 2) ||
				((ch->in_room == GRUNGE_ARENA1 || ch->in_room == GRUNGE_ARENA2 ||
				ch->in_room == GRUNGE_ARENA3 || ch->in_room == GRUNGE_ARENA4) && (blue || red)))
			{
				send_to_char("You can't leave the arena with a combatant ready to fight you!\n", ch);
				return;
			}
			else
			{
				grunge_arena__leave(ch);
				return;
			}
		}
		else
		{
			send_to_char("Hmm?\n", ch);
			return;
		}
	}


	strArgument = one_argument (strArgument, argOne);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
		if (tch != ch && IS_NPC(tch) && tch->mob->vnum == GRUNGE_ARENA_BOSS)
			break;

	overseer = tch;

	if ( !overseer )
	{
		send_to_char ("I don't see the Arena Boss here.\n", ch);
		return;
	}

	if (argOne == "enter")
	{
		bool is_registered = false;

		if (strArgument.empty())
		{
			send_to_char("Syntax: arena enter <crowd/personal> weapontype\n", ch);
			return;
		}

		strArgument = one_argument (strArgument, argTwo);

		if (argTwo == "crowd")
		{
			if (!grunge_arena_second_echo && !grunge_arena_third_echo)
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s %s", buf2, ARENA_NO_CROWD);
				do_whisper (overseer, buf, 83);
				return;
			}
			else
			{
				large_fight = true;
			}
		}
		else if (argTwo != "personal")
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_FIGHT_TYPE);
			do_whisper (overseer, buf, 83);
			return;
		}

		if (strArgument == "longblade")
			weapon_type = 1;
		else if (strArgument == "polearm")
			weapon_type = 2;
		else if (strArgument == "bludgeon")
			weapon_type = 3;
		else if (strArgument == "smallblade")
			weapon_type = 4;
		else if (strArgument == "longblade-only")
			weapon_type = 5;
		else if (strArgument == "bludgeon-only")
			weapon_type = 6;

		if (weapon_type == 0)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_DEFINE_WEAPONS);
			do_whisper (overseer, buf, 83);
			return;
		}

		mysql_safe_query ("SELECT name, alias FROM %s.arena_gladiator WHERE season = '%d'", player_db.c_str(), season);

		if (!(result = mysql_store_result (database)))
			return;

		while ((row = mysql_fetch_row(result)))
		{
			if (!str_cmp(row[0], ch->tname))
			{
				is_registered = true;
				break;
			}
		}

		if (!is_registered)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_NOT_REGISTERED);
			do_whisper (overseer, buf, 83);
			return;
		}

		if (get_damage_total(ch) > 5)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_TOO_INJURED);
			do_whisper (overseer, buf, 83);
			return;
		}

		if (large_fight)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_READY_FIGHT);
			do_whisper (overseer, buf, 83);

			// We want to make sure they're good to fight to avoid
			// shenanigangs.
			if (GET_FLAG (ch, FLAG_PACIFIST))
			{
				ch->flags &= ~FLAG_PACIFIST;
			}
			if (GET_FLAG (ch, FLAG_AUTOFLEE))
			{
				ch->flags &= ~FLAG_AUTOFLEE;
			}
			ch->position = STAND;
			ch->fight_mode = 2;
			ch->effort = 100;

			if (is_clan_member(tch, "arena_blue"))
			{
				remove_clan(tch, "arena_blue");
			}
			if (is_clan_member(tch, "arena_red"))
			{
				remove_clan(tch, "arena_red");
			}

			grunge_arena__do_enter(ch, "", weapon_type);
			return;
		}

		if ((grunge_arena_first_fighters >= 2 && grunge_arena_second_fighters >= 2) || (grunge_arena_first_fight && grunge_arena_second_fight))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_TOO_FULL);
			do_whisper (overseer, buf, 83);
			return;
		}

		if (!grunge_arena_first_fight && grunge_arena_first_fighters < 2)
		{
			grunge_arena_first_fighters ++;
			if (grunge_arena_first_fighters == 2)
				grunge_arena_first_fight = true;
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_READY_FIGHT);
			do_whisper (overseer, buf, 83);
			act ("At your request, $N guides you down to the preparatory room. After you suit up in armour and receive your weapon, you are lead on to the northern cage.", false, ch, 0, overseer, TO_CHAR | _ACT_FORMAT);
			act ("At $n's request, $N guides $m in to the northern cage.", false, ch, 0, overseer, TO_ROOM | _ACT_FORMAT);
			char_from_room(ch);
			arena_strip(ch);
			equip_slave(ch, weapon_type);
			char_to_room(ch, GRUNGE_SMALL1);
			act ("$n enters the cage.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			// We want to make sure they're good to fight to avoid
			// shenanigangs.
			if (GET_FLAG (ch, FLAG_PACIFIST))
			{
				ch->flags &= ~FLAG_PACIFIST;
			}
			if (GET_FLAG (ch, FLAG_AUTOFLEE))
			{
				ch->flags &= ~FLAG_AUTOFLEE;
			}
			ch->position = STAND;
			ch->fight_mode = 2;
			ch->effort = 100;
			command_interpreter(ch, "look");
			return;
		}
		else
		{
			grunge_arena_second_fighters ++;
			if (grunge_arena_second_fighters == 2)
				grunge_arena_second_fight = true;
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ARENA_READY_FIGHT);
			do_whisper (overseer, buf, 83);
			act ("At your request, $N guides you down to the preparatory room. After you suit up in armour and receive your weapon, you are lead on to the southern cage.", false, ch, 0, overseer, TO_CHAR | _ACT_FORMAT);
			act ("At $n's request, $N guides $m in to the southern cage.", false, ch, 0, overseer, TO_ROOM | _ACT_FORMAT);
			char_from_room(ch);
			arena_strip(ch);
			equip_slave(ch, weapon_type);
			char_to_room(ch, GRUNGE_SMALL2);
			act ("$n enters the cage.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			// We want to make sure they're good to fight to avoid
			// shenanigangs.
			if (GET_FLAG (ch, FLAG_PACIFIST))
			{
				ch->flags &= ~FLAG_PACIFIST;
			}
			if (GET_FLAG (ch, FLAG_AUTOFLEE))
			{
				ch->flags &= ~FLAG_AUTOFLEE;
			}
			ch->position = STAND;
			ch->fight_mode = 2;
			ch->effort = 100;
			command_interpreter(ch, "look");
			return;
		}
	}
	else if (argOne == "register")
	{
		mysql_safe_query ("SELECT name FROM %s.arena_gladiator WHERE season = '%d'", player_db.c_str(), season);

		if (!(result = mysql_store_result (database)))
			return;

		while ((row = mysql_fetch_row(result)))
		{
			if (!str_cmp(row[0], ch->tname))
			{
				send_to_char("\nBut you're already registered to fight for this season.\n", ch);
				return;
			}
		}

		mysql_safe_query ("INSERT INTO %s.arena_gladiator (name, short, win, lose, fought, season) VALUES('%s', '%s', '%d', '%d', '%d', '%d')", player_db.c_str(), ch->tname,  ch->short_descr, 0, 0, 0, season);

		output = "\nCongratulations! You have registered for Season " + MAKE_STRING(season) + ".\n";
		page_string (ch->descr(), output.c_str());

	}
	else if (argOne == "score")
	{
		strArgument = one_argument (strArgument, argTwo);
		strArgument = one_argument (strArgument, argThree);

		if (atoi(argTwo.c_str()) > 0)
		{
			season = MAX(1, atoi(argTwo.c_str()));

			if (!argThree.empty())
			{
				argGladiator = argThree;
			}
		}
		else if (!argTwo.empty())
		{
			argGladiator = argTwo;
		}

		if (argGladiator.empty())
		{
			mysql_safe_query ("SELECT id, short, win, lose, fought FROM %s.arena_gladiator WHERE season = '%d'", player_db.c_str(), season);
			output =           "\n     Arena Scores for Season " + MAKE_STRING(season);
		}
		else
		{
			mysql_safe_query ("SELECT id, short, win, lose, fought FROM %s.arena_gladiator WHERE season = '%d' and (short like '%%%s%%')", player_db.c_str(), season, argGladiator.c_str());
			output =           "\n     Arena Scores for Season " + MAKE_STRING(season) + " for " + argGladiator;
		}

		if (!(result = mysql_store_result (database)))
			return;


		output.append("\n#2+----+-----------------------------------------------------------------+-----+------+--------+#0\n");
		output.append(  "#2| ID |                           Description                           | Won | Lost | Fought |#0\n");
		output.append(  "#2+----+-----------------------------------------------------------------+-----+------+--------+#0\n");


		if (!result || !mysql_num_rows (result))
		{
			output.append("#2|                                                                                              |#0\n");
			output.append("#2|                    #0There have not been any fights for this season!                           #2|#0\n");
			output.append("#2|                                                                                              |#0\n");
			output.append( "#2+----+-----------------------------------------------------------------+-----+------+--------+#0\n");
		}
		else
		{
			while ((row = mysql_fetch_row(result)))
			{
				output.append("#2| #0");

				output.append(row[0], MIN((int) strlen(row[0]), 3));

				for (i = 0, j = (3 - MIN((int) strlen(row[0]), 3)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				output.append(row[1], MIN((int) strlen(row[1]), 64));

				for (i = 0, j = (64 - MIN((int) strlen(row[1]), 64)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				output.append(row[2], MIN((int) strlen(row[2]), 4));

				for (i = 0, j = (4 - MIN((int) strlen(row[2]), 4)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				output.append(row[3], MIN((int) strlen(row[3]), 5));

				for (i = 0, j = (5 - MIN((int) strlen(row[3]), 5)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				output.append(row[4], MIN((int) strlen(row[4]), 7));

				for (i = 0, j = (7 - MIN((int) strlen(row[4]), 7)); i < j; i++)
					output.append(" ");

				output.append("#2|#0");

				output.append("\n");

			}
			output.append(  "#2+----+-----------------------------------------------------------------+-----+------+--------+#0\n");
		}

		page_string (ch->descr(), output.c_str());

	}
	else if (argOne == "history")
	{
		strArgument = one_argument (strArgument, argTwo);
		strArgument = one_argument (strArgument, argThree);

		if (atoi(argTwo.c_str()) > 0)
		{
			season = MAX(1, atoi(argTwo.c_str()));

			if (!argThree.empty())
			{
				argGladiator = argThree;
			}
		}
		else if (!argTwo.empty())
		{
			argGladiator = argTwo;
		}

		if (argGladiator.empty())
		{
			mysql_safe_query ("SELECT id, red_short, blue_short, degree FROM %s.arena_fights WHERE season = '%d'", player_db.c_str(), season);
			output =           "\n     Arena Battles and Results for Season " + MAKE_STRING(season);
		}
		else
		{
			mysql_safe_query ("SELECT id, red_short, blue_short, degree FROM %s.arena_fights WHERE season = '%d' and (red_short like '%%%s%%' or blue_short like '%%%s%%')", player_db.c_str(), season, argGladiator.c_str(), argGladiator.c_str());
			output =           "\n     Arena Battles and Results for fighters named " + argGladiator +" in Season " + MAKE_STRING(season);
		}

		if (!(result = mysql_store_result (database)))
			return;


		output.append("\n#2+----+---------------------------------------------------------------------+----------+#0\n");
		output.append(  "#2| ID | Victor                                                              | Degree   |#0\n");
		output.append(  "#2|    | Loser                                                               |          |#0\n");
		output.append(  "#2+----+---------------------------------------------------------------------+----------+#0\n");


		if (!result || !mysql_num_rows (result))
		{
			output.append("#2|                                                                                              |#0\n");
			output.append("#2|                    #0There have not been any fights for this season!                           #2|#0\n");
			output.append("#2|                                                                                              |#0\n");
			output.append("#2+----------------------------------------------------------------------------------------------+#0\n");
		}
		else
		{
			while ((row = mysql_fetch_row(result)))
			{

				output.append("#2| #0");

				output.append(row[0], MIN((int) strlen(row[0]), 3));

				for (i = 0, j = (3 - MIN((int) strlen(row[0]), 3)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				output.append(row[1], MIN((int) strlen(row[1]), 68));

				for (i = 0, j = (68 - MIN((int) strlen(row[1]), 68)); i < j; i++)
					output.append(" ");

				output.append("#2| #0");

				if (atoi(row[3]) >= 2)
				{
					argFour = "easily";
				}
				else if (atoi(row[3]) == 1)
				{
					argFour = "solidly";
				}
				else
				{
					argFour = "barely";
				}

				output.append(argFour);

				for (i = 0, j = (9 - argFour.length()); i < j; i++)
					output.append(" ");

				output.append("#2|#0");

				output.append("\n");				

				output.append("#2|    | #0");

				output.append(row[2], MIN((int) strlen(row[2]), 68));

				for (i = 0, j = (68 - MIN((int) strlen(row[2]), 68)); i < j; i++)
					output.append(" ");

				output.append("#2|          |\n#0");

				output.append("#2+----+---------------------------------------------------------------------+----------+#0\n");


			}
		}

		page_string (ch->descr(), output.c_str());

	}
	else if (GET_TRUST(ch))
	{

		if (argOne == "start")
		{

			if (grunge_arena_fight)
			{
				send_to_char ("There is already a fight in progress.\n", ch);
				return;
			}
			else if (!grunge_arena_fight)
			{
				grunge_arena_first ();
				send_to_char ("Grungetown Arena Initiated.\n", ch);
				return;
			}
		}

		if (argOne == "halt")
		{
			if (grunge_arena_stopped)
			{
				send_to_char
					("The Arena is already halted. Type ARENA RESUME to cancel.\n", ch);
				return;
			}
			grunge_arena_stopped = true;
			send_to_char ("The Arena will not start again until an admin types ARENA RESUME.\n", ch);
			return;
		}

		if (argOne == "resume")
		{
			if (!grunge_arena_stopped)
			{
				send_to_char
					("The Battle has not been halted by an administrator.\n", ch);
				return;
			}
			grunge_arena_stopped = false;
			send_to_char ("The Battle will now start again periodically as usual.\n", ch);
			return;
		}

		if (argOne == "barkers")
		{
			if (!grunge_arena_fight)
			{
				send_to_char ("There is currently no fight in progress!\n", ch);
				return;
			}
			if (grunge_arena_barker_check)
			{
				send_to_char ("The Barkers have already been released!\n", ch);
				return;
			}
			grunge_arena_barkers ();
			send_to_char ("The Barkers have been released.\n", ch);
			return;
		}

		if (argOne == "stop")
		{
			if (!grunge_arena_fight)
			{
				send_to_char ("There is currently no fight in progress!\n", ch);
				return;
			}
			send_to_char ("Grungetown Arena cancelled.\n", ch);
			grunge_arena_cleanup ();
			return;
		}
	}
	else
	{
		send_to_char ("Eh? Check HELP ARENA for command syntax.\n", ch);
	}
}

void
	grunge_arena_last (void)
{
	char buf[MAX_STRING_LENGTH];

	grunge_arena_last_echo = 0;
	sprintf (buf, "Finally, the crowd shuffles out of the viewing rooms, leaving only the bloodstained floors of the arena.");	
	send_to_room (buf, GRUNGE_ARENA1);
	send_to_room (buf, GRUNGE_ARENA2);
	send_to_room (buf, GRUNGE_ARENA3);
	send_to_room (buf, GRUNGE_ARENA4);
	send_to_room (buf, GRUNGE_PUBLIC_VIEW);
	send_to_room (buf, GRUNGE_VIP_VIEW);
	grunge_arena_fight = false;
	grunge_arena_barker_check = false;

}

void
	grunge_arena_third (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!(tch = d->character) || d->connected != CON_PLYNG)
			continue;
		if (tch->deleted)
			continue;
		if (IS_NPC (tch))
			continue;
		if (!IS_SET (tch->flags, FLAG_GUEST))
			continue;
		if (tch->in_room == GRUNGE_ARENA1
			|| tch->in_room == GRUNGE_ARENA2
			|| tch->in_room == GRUNGE_ARENA3
			|| tch->in_room == GRUNGE_ARENA4)
			continue;
		act ("\n#2The Grungetown Arena is no longer accepting new Guest entrants.#0", false, tch, 0, 0, TO_CHAR);
	}

	grunge_arena_third_echo = 0;
	sprintf (buf, "Two teams of gladiators are now assembled, one decorated with blue paint splashed over their armour, the other with red -- or maybe blood. The chanting of the crowd now reaches fever pitch, dozens of feet stamping down on the thin mesh that serves to separate the populace of Grungetown from the fight below. At some unseen signal, the gladiators begin to fight!");
	send_to_room (buf, GRUNGE_ARENA1);
	send_to_room (buf, GRUNGE_ARENA2);
	send_to_room (buf, GRUNGE_ARENA3);
	send_to_room (buf, GRUNGE_ARENA4);
	send_to_room (buf, GRUNGE_PUBLIC_VIEW);
	send_to_room (buf, GRUNGE_VIP_VIEW);
	grunge_arena ();
}

void
	grunge_arena_second (void)
{
	char buf[MAX_STRING_LENGTH];

	grunge_arena_second_echo = 0;
	sprintf (buf, "The idle talking from the denizens, mutations and cybernetics of Grungetown shifts in to the starts of cheers, catcalls, taunts, and laughs as gladiators, dressed in archaic armanents from Earth long gone, begin to enter the killing fields beneath the mesh floor.");
	grunge_arena_third_echo = 61;
	send_to_room (buf, GRUNGE_ARENA1);
	send_to_room (buf, GRUNGE_ARENA2);
	send_to_room (buf, GRUNGE_ARENA3);
	send_to_room (buf, GRUNGE_ARENA4);
	send_to_room (buf, GRUNGE_PUBLIC_VIEW);
	send_to_room (buf, GRUNGE_PRIVATE_VIEW);	
	send_to_room (buf, GRUNGE_VIP_VIEW);
}

void
	grunge_arena_first (void)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	DESCRIPTOR_DATA *d;

	if (!vnum_to_room (GRUNGE_ARENA1)
		|| !vnum_to_room (GRUNGE_ARENA2)
		|| !vnum_to_room (GRUNGE_ARENA3)
		|| !vnum_to_room (GRUNGE_ARENA4))
		return;

	if (grunge_arena_stopped)
		return;

	// Do a quick clean-up of the arena.
	grunge_arena_cleanup ();

	/*
	for (d = descriptor_list; d; d = d->next)
	{
	if (!(tch = d->character) || d->connected != CON_PLYNG)
	continue;
	if (tch->deleted)
	continue;
	if (IS_NPC (tch))
	continue;
	if (!IS_SET (tch->flags, FLAG_GUEST))
	continue;
	act
	("\n#2A twitch in your bones tells you that the Grungetown Arena is preparing for a new fight. To join, simply type ENTER ARENA: you will be returned here when the battle is won or lost.", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}*/

	sprintf (buf,"A hubub of people from Grungetown begin filling up the arena, the excited chattering and stomping across the mesh floor flooding the air with a roar of sound.\n\n");
	sprintf (buf + strlen(buf), "#6If you are registered with the Arena Boss, you can enter this fight by taking #9#6ARENA ENTER#0#6 at the Arena Registration Room.#0\n");
	grunge_arena_second_echo = 31;
	send_to_room (buf, GRUNGE_OUTSIDE);
	send_to_room (buf, GRUNGE_REGO);
	send_to_room (buf, GRUNGE_ENTRY);
	send_to_room (buf, GRUNGE_ARENA1);
	send_to_room (buf, GRUNGE_ARENA2);
	send_to_room (buf, GRUNGE_ARENA3);
	send_to_room (buf, GRUNGE_ARENA4);
	send_to_room (buf, GRUNGE_PUBLIC_VIEW);
	send_to_room (buf, GRUNGE_PRIVATE_VIEW);	
	send_to_room (buf, GRUNGE_VIP_VIEW);
}

void
	grunge_arena (void)
{
	CHAR_DATA *tmob;
	CHAR_DATA *blue_mob = NULL;
	CHAR_DATA *red_mob = NULL;
	ROOM_DATA *room = NULL;
	//int room, num_slaves;
	int i = 0;

	grunge_arena_fight = true;

	//num_slaves = number (20, 20);

	// Unflag the rooms pacifist so the fighting can begin.
	for (i = 0; i <= 3; i++)
	{
		room = vnum_to_room (grunge_arena_rooms[i]);
		if (!room)
			continue;
		if (IS_SET(room->room_flags, PEACE))
		{
			room->room_flags &= ~PEACE;
		}
	}

	// Load up the blue gladiators.
	for (i = 0; i <= 4; i++)
	{
		tmob = NULL;
		tmob = load_mobile (BLUE_GLAD);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		equip_slave (tmob, 0);
		tmob->act |= ACT_AGGRESSIVE;
		char_to_room(tmob, GRUNGE_ARENA1 + number(0,3));
		blue++;
	}

	// Load up red gladiators.
	for (i = 0; i <= 4; i++)
	{
		tmob = NULL;
		tmob = load_mobile (RED_GLAD);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		equip_slave (tmob, 0);
		tmob->act |= ACT_AGGRESSIVE;
		char_to_room(tmob, GRUNGE_ARENA1 + number(0,3));
		red++;
	}
}

/*
void
grunge_engage_echo (CHAR_DATA * ch, CHAR_DATA * vict)
{
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
CHAR_DATA *tch, *ally = NULL;

for (tch = ch->room->people; tch; tch = tch->next_in_room)
{
if (tch->fighting == vict && vict->fighting == tch)
{
ally = tch;
break;
}
}

if (ally)
sprintf (buf, "%s#0 engages #5%s#0, coming to #5%s#0's aid!",
char_short (ch), char_short (vict), char_short (ally));
else
sprintf (buf, "%s#0 engages #5%s#0!", char_short (ch), char_short (vict));

*buf = toupper (*buf);
sprintf (buf2, "#5%s", buf);
sprintf (buf, "%s", buf2);

send_to_room (buf, GRUNGE_PUBLIC_VIEW);
send_to_room (buf, GRUNGE_VIP_VIEW);
}
*/

int
	is_grunge_arena_clear (void)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;
	int i = 0;

	for (i = 0; i <= 3; i++)
	{
		room = vnum_to_room (grunge_arena_rooms[i]);
		if (!room)
			continue;
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (!IS_NPC (tch))
				return 0;
	}

	return 1;
}

void
	grunge_arena_cleanup (void)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;
	OBJ_DATA *obj, *next_o;
	CHAR_DATA *tch, *next_c;
	WOUND_DATA *wound, *next_wound;
	int i;

	red = 0;
	blue = 0;

	grunge_arena_cleanup_echo = 0;

	if (grunge_arena_fight)
	{
		sprintf (buf,	"As the surviving gladiators trudge back in to the Arena complex, a crew of janitors and aides stream out, pulling away casulties and removing discarded weapons.");
		send_to_room (buf, GRUNGE_ARENA1);
		send_to_room (buf, GRUNGE_ARENA2);
		send_to_room (buf, GRUNGE_ARENA3);
		send_to_room (buf, GRUNGE_ARENA4);
		send_to_room (buf, GRUNGE_PUBLIC_VIEW);
		send_to_room (buf, GRUNGE_VIP_VIEW);

		if (grunge_arena_barker_check)
		{
			sprintf (buf,	"The Barker-handlers quickly run back in to the arena, chaining back up the surviving beasts.");
			send_to_room (buf, GRUNGE_ARENA1);
			send_to_room (buf, GRUNGE_ARENA2);
			send_to_room (buf, GRUNGE_ARENA3);
			send_to_room (buf, GRUNGE_ARENA4);
			send_to_room (buf, GRUNGE_PUBLIC_VIEW);
			send_to_room (buf, GRUNGE_VIP_VIEW);
			//send_to_room (buf, 5201);
			//send_to_room (buf, 5200);
		}
	}
	else
	{
		sprintf (buf,	"Before the fighty begins, a crew of janitors sweeps the sands and removes any nosey-bodies from the Arena floor.");
		send_to_room (buf, GRUNGE_ARENA1);
		send_to_room (buf, GRUNGE_ARENA2);
		send_to_room (buf, GRUNGE_ARENA3);
		send_to_room (buf, GRUNGE_ARENA4);
	}

	for (i = 0; i <= 3; i++)
	{
		room = vnum_to_room (grunge_arena_rooms[i]);
		for (obj = room->contents; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (!room->people)
			continue;
		for (tch = room->people; tch; tch = next_c)
		{
			next_c = tch->next_in_room;
			if (IS_NPC (tch))
				extract_char (tch);
			else if (!IS_NPC (tch))
			{
				if (IS_SET (tch->flags, FLAG_GUEST) && tch->descr())
				{
					tch->damage = 0;
					for (wound = tch->wounds; wound; wound = next_wound)
					{
						next_wound = wound->next;
						wound_from_char (tch, wound);
					}
					create_guest_avatar (tch->descr(), "recreate");
				}
				else if (!IS_SET(tch->flags, FLAG_GUEST))
				{

					for (obj = tch->equip; obj; obj = next_o)
					{
						next_o = obj->next_content;
						extract_obj (obj);
					}
					if (tch->left_hand)
					{
						extract_obj(tch->left_hand);
					}		
					if (tch->right_hand)
					{
						extract_obj(tch->right_hand);
					}

					char_to_room(tch, VNUM_DROPROOM);
					sprintf (buf, "get %s", tch->tname);
					command_interpreter(tch, buf);
					char_from_room(tch);
					char_to_room(tch, GRUNGE_CHANGE1 + number(0,1));

					act ("$n is dragged back in to the backrooms by a pair of Arena handlers.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);

					for (wound = tch->wounds; wound; wound = wound->next)
					{
						if (!str_cmp (wound->severity, "grievous") ||
							!str_cmp (wound->severity, "terrible") || 
							!str_cmp (wound->severity, "horrific"))
							wound->healerskill = 50;

						if (wound->infection)
							wound->infection = -1;


						wound->fracture = 0;
						wound->bleeding = 0;
						wound->bindskill = 0;
					}

					if (grunge_arena_fight)
					{

						obj = load_object(14011);
						obj->count = 2;
						obj_to_char(obj, tch);
						act ("You are quickly dragged to the backrooms by a pair of Arena handlers, and after the doctor treats your injuries, you are unceremoniously dumped in the Arena equipment room. At some point, $p is pressed in to your hand for your work today.", false, tch, obj, 0, TO_CHAR | _ACT_FORMAT);                    
					}
					else
					{
						act ("You are quickly dragged to the backroom by a pair of Arena handlers, , and after the doctor treats your injuries, you are unceremoniously dumped in the Arena equipment room.", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
					}

					if (is_clan_member(tch, "arena_blue"))
					{
						remove_clan(tch, "arena_blue");
					}
					if (is_clan_member(tch, "arena_red"))
					{
						remove_clan(tch, "arena_red");
					}

					command_interpreter(tch, "look");
				}
			}
		}
	}

	// Flag the rooms pacifist so we avoid fighting.
	for (i = 0; i <= 3; i++)
	{
		room = vnum_to_room (grunge_arena_rooms[i]);
		if (!room)
			continue;
		if (!IS_SET(room->room_flags, PEACE))
		{
			room->room_flags |= PEACE;
		}
		// Final check to make sure no-one is here.
		room->people = NULL;
	}

	if (grunge_arena_fight)
		grunge_arena_last_echo = 31;
}

void
	grunge_arena__leave (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	OBJ_DATA *next_o = NULL;

	sprintf (buf, "%s#0 leaves the Arena.", char_short (ch));

	if (!IS_NPC(ch) /*&& !IS_SET (tch->flags, FLAG_GUEST)*/)
	{
		for (obj = ch->equip; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (ch->left_hand)
		{
			extract_obj(ch->left_hand);
		}		
		if (ch->right_hand)
		{
			extract_obj(ch->right_hand);
		}

		if (ch->in_room == GRUNGE_SMALL1)
		{
			grunge_arena_first_fighters = 0;
			grunge_arena_first_fight = false;
		}
		if (ch->in_room == GRUNGE_SMALL2)
		{
			grunge_arena_second_fighters = 0;
			grunge_arena_second_fight = false;
		}

		act ("You leave the Arena.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n leaves the Arena.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		char_from_room(ch);
		char_to_room(ch, VNUM_DROPROOM);
		sprintf (buf, "get %s", ch->tname);
		command_interpreter(ch, buf);
		char_from_room(ch);
		char_to_room(ch, GRUNGE_CHANGE1 + number(0,1));
		act ("$n is dragged back in to the backrooms by a pair of Arena handlers.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		act ("Opting out, you are quickly dragged to the backrooms by a pair of Arena handlers.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);                    

		if (is_clan_member(ch, "arena_blue"))
		{
			remove_clan(ch, "arena_blue");
		}
		if (is_clan_member(ch, "arena_red"))
		{
			remove_clan(ch, "arena_red");
		}
	}
	else if (IS_NPC(ch))
	{
		act ("You leave the Arena.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n leaves the Arena.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	//send_to_room (buf, 5201);
	//send_to_room (buf, 5200);

	if (in_arena_room(ch) == 1)
	{
		send_to_room (buf, GRUNGE_PUBLIC_VIEW);
		send_to_room (buf, GRUNGE_VIP_VIEW);

		if (is_clan_member(ch, "arena_blue"))
		{
			blue--;
		}
		else if (is_clan_member(ch, "arena_red"))
		{
			red--;
		}
	}
}


void
	grunge_arena__defeat (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	OBJ_DATA *next_o = NULL;
	WOUND_DATA *wound = NULL;
	ROOM_DATA *room = NULL;

	if (ch->death_ch)
	{
		sprintf (buf, "%s#0 has been defeated by #5%s#0!", char_short (ch), char_short (ch->death_ch));
	}
	else if (!ch->death_ch)
	{
		sprintf (buf, "%s#0 has been defeated!", char_short (ch));
	}

	if (!IS_NPC(ch) /*&& !IS_SET (tch->flags, FLAG_GUEST)*/)
	{
		for (obj = ch->equip; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (ch->left_hand)
		{
			extract_obj(ch->left_hand);
		}		
		if (ch->right_hand)
		{
			extract_obj(ch->right_hand);
		}

		if (ch->death_ch)
		{
			act ("You are defeated by $N!", false, ch, 0, ch->death_ch, TO_CHAR | _ACT_FORMAT);
			act ("$n has been defeated by $N!", false, ch, 0, ch->death_ch, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("You are defeated!", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n has been defeated!", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}

		room = ch->room;

		if (ch->fighting)
		{
			stop_fighting (ch);
		}

		char_from_room(ch);
		char_to_room(ch, VNUM_DROPROOM);

		if (GET_POS (ch) == POSITION_UNCONSCIOUS)
		{
			GET_POS (ch) = REST;
			send_to_char ("Groaning groggily, you regain consciousness.\n", ch);
		}

		sprintf (buf3, "get %s", ch->tname);
		command_interpreter(ch, buf3);
		char_from_room(ch);
		char_to_room(ch, GRUNGE_CHANGE1 + number(0,1));
		act ("$n is dragged back in to the backrooms by a pair of Arena handlers.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		obj = load_object(14011);
		obj->count = 2;
		obj_to_char(obj, ch);
		act ("Defeated, you are quickly dragged to the backrooms by a pair of Arena handlers, and after the doctor treats your injuries, you are unceremoniously dumped in the Arena equipment room. At some point, $p is pressed in to your hand for your work today.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);                    

		command_interpreter(ch, "look");

		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (!str_cmp (wound->severity, "grievous") ||
				!str_cmp (wound->severity, "terrible") || 
				!str_cmp (wound->severity, "horrific"))
				wound->healerskill = 50;

			if (wound->infection)
				wound->infection = -1;

			wound->fracture = 0;
			wound->bleeding = 0;
			wound->bindskill = 0;
		}
	}

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	//send_to_room (buf, 5201);
	//send_to_room (buf, 5200);

	if (is_arena_room(room->vnum) == 1)
	{
		send_to_room (buf, GRUNGE_PUBLIC_VIEW);
		send_to_room (buf, GRUNGE_VIP_VIEW);

		if (is_clan_member(ch, "arena_blue"))
		{
			blue--;
			remove_clan(ch, "arena_blue");
		}
		else if (is_clan_member(ch, "arena_red"))
		{
			red--;
			remove_clan(ch, "arena_red");
		}

		if (blue <= 0 && !grunge_arena_cleanup_echo && grunge_arena_fight)
		{
			sprintf (buf, "\nThe final blue-team gladiator is struck down to the riotous cheers of the crowd.\n");
			send_to_room (buf, GRUNGE_ARENA1);
			send_to_room (buf, GRUNGE_ARENA2);
			send_to_room (buf, GRUNGE_ARENA3);
			send_to_room (buf, GRUNGE_ARENA4);
			send_to_room (buf, GRUNGE_PUBLIC_VIEW);
			send_to_room (buf, GRUNGE_VIP_VIEW);
			grunge_arena_cleanup_echo = 21;
		}

		if (red <= 0 && !grunge_arena_cleanup_echo && grunge_arena_fight)
		{

			sprintf (buf, "\nThe final red-team gladiator is struck down to the riotous cheers of the crowd.\n");
			send_to_room (buf, GRUNGE_ARENA1);
			send_to_room (buf, GRUNGE_ARENA2);
			send_to_room (buf, GRUNGE_ARENA3);
			send_to_room (buf, GRUNGE_ARENA4);
			send_to_room (buf, GRUNGE_PUBLIC_VIEW);
			send_to_room (buf, GRUNGE_VIP_VIEW);
			grunge_arena_cleanup_echo = 21;
		}
	}
	else if (is_arena_room(room->vnum) == 2)
	{
		send_to_room (buf, GRUNGE_PRIVATE_VIEW);
		send_to_room ("\nBoth fighters leave the cage, now ready for a new fight.\n", GRUNGE_PRIVATE_VIEW);

		if (room->vnum == GRUNGE_SMALL1)
		{
			grunge_arena_first_fight = false;
			grunge_arena_first_fighters = 0;
		}
		else if (room->vnum == GRUNGE_SMALL2)
		{
			grunge_arena_second_fight = false;
			grunge_arena_second_fighters = 0;
		}

		CHAR_DATA *tch = NULL;

		for (tch = room->people; tch; tch = tch->next_in_room)
		{
			if (tch != ch)
			{
				if (GET_TRUST(tch) && tch->next_in_room)
					continue;

				for (obj = tch->equip; obj; obj = next_o)
				{
					next_o = obj->next_content;
					extract_obj (obj);
				}

				if (tch->left_hand)
				{
					extract_obj(tch->left_hand);
				}		
				if (tch->right_hand)
				{
					extract_obj(tch->right_hand);
				}

				if (tch->fighting)
				{
					stop_fighting (tch);
				}
				char_from_room(tch);
				char_to_room(tch, VNUM_DROPROOM);

				if (GET_POS (tch) == POSITION_UNCONSCIOUS)
				{
					GET_POS (tch) = REST;
					send_to_char ("Groaning groggily, you regain consciousness.\n", tch);
				}

				sprintf (buf3, "get %s", tch->tname);
				command_interpreter(tch, buf3);
				char_from_room(tch);
				char_to_room(tch, GRUNGE_CHANGE1 + number(0,1));
				obj = load_object(14011);
				obj->count = 4;
				obj_to_char(obj, tch);
				act ("$n is led back in to the backrooms by a pair of Arena handlers.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
				act ("Victorous, you are led to the backrooms by a pair of Arena handlers, and after the doctor treats your injuries, stride in to the Arena equipment room. At some point, $p is pressed in to your hand for your service.", false, tch, obj, 0, TO_CHAR | _ACT_FORMAT);

				command_interpreter(tch, "look");

				for (wound = tch->wounds; wound; wound = wound->next)
				{
					if (!str_cmp (wound->severity, "grievous") ||
						!str_cmp (wound->severity, "terrible") || 
						!str_cmp (wound->severity, "horrific"))
						wound->healerskill = 50;

					if (wound->infection)
						wound->infection = -1;

					wound->fracture = 0;
					wound->bleeding = 0;
					wound->bindskill = 0;
				}

				// Degree is simple - if you dished out more than 25, it's an easy victory,
				//					- if you dished out 5 or less, it's a bare victory.

				int degree = 0;
				int diff = get_damage_total(ch) - get_damage_total(tch);
				if (diff > 30)
				{
					degree = 2;
				}
				else if (diff > 10)
				{
					degree = 1;
				}
				else
				{
					degree = 0;				
				}

				std::string player_db = engine.get_config ("player_db");

				mysql_safe_query ("INSERT INTO %s.arena_fights (red_short, blue_short, degree, odds_top, odds_under, season) VALUES ('%s', '%s', %d, %d, %d, %d)", player_db.c_str(), tch->short_descr, ch->short_descr, degree, 0, 0, 1);
				mysql_safe_query ("UPDATE %s.arena_gladiator SET win=win+1, fought=fought+1 WHERE name = '%s'", player_db.c_str(), tch->tname);
				mysql_safe_query ("UPDATE %s.arena_gladiator SET lose=lose+1, fought=fought+1 WHERE name = '%s'", player_db.c_str(), ch->tname);
				break;
			}
		}
	}
}

void
	grunge_arena__death (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	OBJ_DATA *next_o = NULL;
	WOUND_DATA *wound = NULL;
	ROOM_DATA *room = NULL;

	if (ch->death_ch)
	{
		sprintf (buf, "%s#0 has been defeated by #5%s#0!", char_short (ch), char_short (ch->death_ch));
	}
	else if (!ch->death_ch)
	{
		sprintf (buf, "%s#0 has been defeated!", char_short (ch));
	}

	if (!IS_NPC(ch) /*&& !IS_SET (tch->flags, FLAG_GUEST)*/)
	{
		for (obj = ch->equip; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (ch->left_hand)
		{
			extract_obj(ch->left_hand);
		}		
		if (ch->right_hand)
		{
			extract_obj(ch->right_hand);
		}

		if (ch->death_ch)
		{
			act ("You are defeated by $N!", false, ch, 0, ch->death_ch, TO_CHAR | _ACT_FORMAT);
			act ("$n has been defeated by $N!", false, ch, 0, ch->death_ch, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("You are slain!", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n has been slain!", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}

		room = ch->room;
		char_from_room(ch);
		char_to_room(ch, 56311);
		act ("$n's corpse is dragged back in to the backrooms by a pair of Arena handlers.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	//send_to_room (buf, 5201);
	//send_to_room (buf, 5200);

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	if (in_arena_room(ch) == 1)
	{
		send_to_room (buf, GRUNGE_PUBLIC_VIEW);
		send_to_room (buf, GRUNGE_VIP_VIEW);

		if (is_clan_member(ch, "arena_blue"))
		{
			blue--;
		}
		else if (is_clan_member(ch, "arena_red"))
		{
			red--;
		}

		if (blue <= 0 && !grunge_arena_cleanup_echo && grunge_arena_fight)
		{
			sprintf (buf, "\nThe final blue-team gladiator is struck down to the riotous cheers of the crowd.\n");
			send_to_room (buf, GRUNGE_ARENA1);
			send_to_room (buf, GRUNGE_ARENA2);
			send_to_room (buf, GRUNGE_ARENA3);
			send_to_room (buf, GRUNGE_ARENA4);
			send_to_room (buf, GRUNGE_PUBLIC_VIEW);
			send_to_room (buf, GRUNGE_VIP_VIEW);
			grunge_arena_cleanup_echo = 21;
		}

		if (red <= 0 && !grunge_arena_cleanup_echo && grunge_arena_fight)
		{

			sprintf (buf, "\nThe final red-team gladiator is struck down to the riotous cheers of the crowd.\n");
			send_to_room (buf, GRUNGE_ARENA1);
			send_to_room (buf, GRUNGE_ARENA2);
			send_to_room (buf, GRUNGE_ARENA3);
			send_to_room (buf, GRUNGE_ARENA4);
			send_to_room (buf, GRUNGE_PUBLIC_VIEW);
			send_to_room (buf, GRUNGE_VIP_VIEW);
			grunge_arena_cleanup_echo = 21;
		}
	}
	else if (is_arena_room(room->vnum) == 2)
	{
		send_to_room (buf, GRUNGE_PRIVATE_VIEW);
		send_to_room ("\nThe remaining fighter leave the cage, now ready for a new fight.\n", GRUNGE_PRIVATE_VIEW);

		if (room->vnum == GRUNGE_SMALL1)
		{
			grunge_arena_first_fight = false;
			grunge_arena_first_fighters = 0;
		}
		else if (room->vnum == GRUNGE_SMALL2)
		{
			grunge_arena_second_fight = false;
			grunge_arena_second_fighters = 0;
		}

		CHAR_DATA *tch = NULL;

		for (tch = room->people; tch; tch = tch->next_in_room)
		{
			if (tch != ch)
			{
				if (GET_TRUST(tch) && tch->next_in_room)
					continue;

				for (obj = tch->equip; obj; obj = next_o)
				{
					next_o = obj->next_content;
					extract_obj (obj);
				}

				if (tch->left_hand)
				{
					extract_obj(tch->left_hand);
				}		
				if (tch->right_hand)
				{
					extract_obj(tch->right_hand);
				}


				if (tch->fighting)
				{
					stop_fighting (tch);
				}
				char_from_room(tch);
				char_to_room(tch, VNUM_DROPROOM);

				if (GET_POS (tch) == POSITION_UNCONSCIOUS)
				{
					GET_POS (tch) = REST;
					send_to_char ("Groaning groggily, you regain consciousness.\n", tch);
				}

				sprintf (buf, "get %s", tch->tname);
				command_interpreter(tch, buf);
				char_from_room(tch);
				char_to_room(tch, GRUNGE_CHANGE1 + number(0,1));
				obj = load_object(14011);
				obj->count = 4;
				obj_to_char(obj, tch);
				act ("$n is led back in to the backrooms by a pair of Arena handlers.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
				act ("Victorous, you are led to the backrooms by a pair of Arena handlers, and after the doctor treats your injuries, stride in to the Arena equipment room. At some point, $p is pressed in to your hand for your service.", false, tch, obj, 0, TO_CHAR | _ACT_FORMAT);

				command_interpreter(tch, "look");

				for (wound = tch->wounds; wound; wound = wound->next)
				{
					if (!str_cmp (wound->severity, "grievous") ||
						!str_cmp (wound->severity, "terrible") || 
						!str_cmp (wound->severity, "horrific"))
						wound->healerskill = 50;

					if (wound->infection)
						wound->infection = -1;

					wound->fracture = 0;	
					wound->bleeding = 0;
					wound->bindskill = 0;
				}

				// Degree is simple - if you dished out more than 25, it's an easy victory,
				//					- if you dished out 5 or less, it's a bare victory.

				int degree = 0;
				int diff = get_damage_total(ch) - get_damage_total(tch);
				if (diff > 25)
				{
					degree = 2;
				}
				else if (diff > 5)
				{
					degree = 1;
				}
				else
				{
					degree = 0;				
				}

				std::string player_db = engine.get_config ("player_db");

				mysql_safe_query ("INSERT INTO %s.arena_fights (red_short, blue_short, degree, odds_top, odds_under, season) VALUES ('%s', '%s', %d, %d, %d, %d)", player_db.c_str(), tch->short_descr, ch->short_descr, degree, 0, 0, 1);
				mysql_safe_query ("UPDATE %s.arena_gladiator SET win=win+1, fought=fought+1 WHERE name = '%s'", player_db.c_str(), tch->tname);
				mysql_safe_query ("UPDATE %s.arena_gladiator SET lose=lose+1, fought=fought+1 WHERE name = '%s'", player_db.c_str(), ch->tname);
				break;
			}
		}
	}
}

void
	grunge_combat_message (CHAR_DATA * src, CHAR_DATA * tar, char *location, int damage, int room)
{
	char area[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char impact[MAX_STRING_LENGTH];
	bool small_arena = false;

	if (damage == 0)
		return;

	if (room == GRUNGE_ARENA4)
		sprintf (area, "north-western section");
	else if (room == GRUNGE_ARENA1)
		sprintf (area, "north-eastern section");
	else if (room == GRUNGE_ARENA2)
		sprintf (area, "south-eastern section");
	else if (room == GRUNGE_ARENA3)
		sprintf (area, "south-western section");
	else if (room == GRUNGE_SMALL1)
	{
		sprintf (area, "northern cage");
		small_arena = true;
	}
	else if (room == GRUNGE_SMALL2)
	{
		sprintf (area, "southern cage");
		small_arena = true;
	}

	if (small_arena)
	{
		if (damage > 3 && damage <= 6)
			sprintf (impact, "a light blow to");
		else if (damage > 6 && damage <= 10)
			sprintf (impact, "moderately hard on");
		else if (damage > 10 && damage <= 15)
			sprintf (impact, "a heavy blow to");
		else if (damage > 15 && damage <= 20)
			sprintf (impact, "a severe blow to");
		else if (damage > 20 && damage <= 25)
			sprintf (impact, "grievously hard on");
		else if (damage >= 25)
			sprintf (impact, "a devastating blow to");
		else
			return;
	}
	else
	{
		if (damage > 15 && damage <= 20)
			sprintf (impact, "a severe blow to");
		else if (damage > 20 && damage <= 25)
			sprintf (impact, "grievously hard on");
		else if (damage >= 25)
			sprintf (impact, "a devastating blow to");
		else
			return;
	}

	sprintf (buf, "In the %s of the arena, #5%s#0", area, char_short (src));
	sprintf (buf + strlen (buf), " strikes #5%s#0 %s the %s.", char_short (tar), impact, expand_wound_loc (location));

	if (small_arena)
	{
		send_to_room(buf, GRUNGE_PRIVATE_VIEW);
	}
	else
	{
		send_to_room (buf, GRUNGE_PUBLIC_VIEW);
		send_to_room (buf, GRUNGE_VIP_VIEW);
	}
	//send_to_room (buf, 5201);
	//send_to_room (buf, 5200);

}


void
	grunge_arena__update_delays (void)
{
	if (grunge_arena_last_echo > 1)
	{
		grunge_arena_last_echo--;
		if (grunge_arena_last_echo == 1)
			grunge_arena_last ();
	}

	if (grunge_arena_cleanup_echo > 1)
	{
		grunge_arena_cleanup_echo--;
		if (grunge_arena_cleanup_echo == 1)
			grunge_arena_cleanup ();
	}

	if (grunge_arena_second_echo > 1)
	{
		grunge_arena_second_echo--;
		if (grunge_arena_second_echo == 1)
			grunge_arena_second ();
	}

	if (grunge_arena_third_echo > 1)
	{
		grunge_arena_third_echo--;
		if (grunge_arena_third_echo == 1)
			grunge_arena_third ();
	}
}

void
	grunge_arena__do_shout (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	int i = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };


	if (cmd == 0)
	{
		for (i = 0; i <= 3; i++)
		{
			for (tch = vnum_to_room (grunge_arena_rooms[i])->people; tch; tch = tch->next_in_room)
			{
				if (tch == ch)
					continue;
				if (GET_SEX (ch) == SEX_MALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a male voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a male voice shout from the stands,");
					}
					else
					{
						sprintf (buf,
							"You hear a male voice shout from the stands,");
					}
				}
				else if (GET_SEX (ch) == SEX_FEMALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a female voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a female voice shout from the stands,");
					}
					else
						sprintf (buf,
						"You hear a female voice shout from the stands,");
				}
				else
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a voice shout from the stands,");
					}
					else
						sprintf (buf, "You hear a voice shout from the stands,");
				}
				if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));
				if (!decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
				{
					sprintf (buf + strlen (buf),
						" though you cannot decipher the words.");
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				}
				else if (tch->descr())
				{
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
					if (decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
					{
						if (!IS_SET (ch->room->room_flags, OOC))
							sprintf (buf, "   \"%s\"\n", argument);
						else
							sprintf (buf, "   \"%s\"\n", argument);
						send_to_char (buf, tch);
					}
				}

				if (GET_TRUST (tch) && !IS_NPC (tch)
					&& GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';
				sprintf (buf2, argument);	/* Reset. */
			}
		}
	}
	else if (cmd == 1)
	{
		for (i = 0; i <= 1; i++)
		{
			for (tch = vnum_to_room (grunge_private_arena_rooms[i])->people; tch; tch = tch->next_in_room)
			{
				if (tch == ch)
					continue;
				if (GET_SEX (ch) == SEX_MALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a male voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a male voice shout from the stands,");
					}
					else
					{
						sprintf (buf,
							"You hear a male voice shout from the stands,");
					}
				}
				else if (GET_SEX (ch) == SEX_FEMALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a female voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a female voice shout from the stands,");
					}
					else
						sprintf (buf,
						"You hear a female voice shout from the stands,");
				}
				else
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a voice shout in an unknown tongue from the stands,");
						else
							sprintf (buf,
							"You hear a voice shout from the stands,");
					}
					else
						sprintf (buf, "You hear a voice shout from the stands,");
				}
				if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));
				if (!decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
				{
					sprintf (buf + strlen (buf),
						" though you cannot decipher the words.");
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				}
				else if (tch->descr())
				{
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
					if (decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
					{
						if (!IS_SET (ch->room->room_flags, OOC))
							sprintf (buf, "   \"%s\"\n", argument);
						else
							sprintf (buf, "   \"%s\"\n", argument);
						send_to_char (buf, tch);
					}
				}

				if (GET_TRUST (tch) && !IS_NPC (tch)
					&& GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';
				sprintf (buf2, argument);	/* Reset. */
			}
		}
	}

}

void
	grunge_arena__do_look (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	ROOM_DATA *troom = NULL;
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, arg1);


	if (!cmd)
	{

		if (!*arg1)
		{
			*buf = '\0';
			send_to_char
				("You gaze down through the mesh floor, seeing...\n",
				ch);

			troom = vnum_to_room (GRUNGE_ARENA4);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the north-western part of the Arena:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			troom = vnum_to_room (GRUNGE_ARENA1);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the north-eastern part of the Arena:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			troom = vnum_to_room (GRUNGE_ARENA2);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the south-eastern part of the Arena:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			troom = vnum_to_room (GRUNGE_ARENA3);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the south-western part of the Arena:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			if (!*buf)
				send_to_char
				("\n...nothing of interest. There is no battle currently in progress.\n",
				ch);
			else
			{
				sprintf (buf, "#6Blue Fighters Remaining: %d#0\n", blue);
				send_to_char(buf, ch);
				sprintf (buf, "#6Red Fighters Remaining: %d#0\n", red);
				send_to_char(buf, ch);
			}
			return;
		}
		else
		{
			tch = get_char_room (arg1, GRUNGE_ARENA4);
			if (!tch)
				tch = get_char_room (arg1, GRUNGE_ARENA1);
			if (!tch)
				tch = get_char_room (arg1, GRUNGE_ARENA2);
			if (!tch)
				tch = get_char_room (arg1, GRUNGE_ARENA3);
			if (!tch)
			{
				send_to_char ("You do not see that in the Arena.\n", ch);
				return;
			}

			show_char_to_char (tch, ch, (cmd == 2) ? 15 : 1);
			return;
		}
	}
	else
	{
		if (!*arg1)
		{
			*buf = '\0';
			send_to_char ("You down through the mesh floor, seeing...\n", ch);

			troom = vnum_to_room (GRUNGE_SMALL1);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the northern cage:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			troom = vnum_to_room (GRUNGE_SMALL2);
			if (troom && troom->people)
			{
				sprintf (buf, "#6\nIn the southern cage:#0\n");
				send_to_char (buf, ch);
				list_char_to_char (troom->people, ch, 0);
			}
			if (!*buf)
				send_to_char ("\n...nothing of interest. There are no battles currently in progress.\n", ch);
			return;
		}
		else
		{
			tch = get_char_room (arg1, GRUNGE_SMALL1);
			if (!tch)
				tch = get_char_room (arg1, GRUNGE_SMALL2);
			{
				send_to_char ("You do not see that in the cages of the Arena.\n", ch);
				return;
			}

			show_char_to_char (tch, ch, (cmd == 2) ? 15 : 1);
			return;
		}

	}
}


void
	grunge_arena__do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char rand[AVG_STRING_LENGTH];
	int roll = number (1, 2);
	int i = 0, j = 0;


	if (!grunge_arena_second_echo && !grunge_arena_third_echo)
	{
		send_to_char
			("The Grungetown Arena is not accepting new entrants at the moment.\n",
			ch);
		return;
	}
	if (ch->in_room == GRUNGE_ARENA1 || ch->in_room == GRUNGE_ARENA2 || ch->in_room == GRUNGE_ARENA3 || ch->in_room == GRUNGE_ARENA4)
	{
		send_to_char ("You're already in the Grungetown Arena!\n", ch);
		return;
	}

	/*
	if ((!vtom (5999) || !vtom (5997) || !vtom (5998)) ||
	(!vtor (5142) || !vtor (5119) || !vtor (5196) ||
	!vtor (5197) || !vtor (5198) || !vtor (5199)))
	{
	send_to_char ("The necessary prototypes cannot be found. Aborting.\n",
	ch);
	return;
	}*/

	act ("$n descends down the stairs to take place in the upcoming crowd fight in the Grungetown Arena...", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	send_to_char ("#2\nYou descend down the stairs to take place in the upcoming crowd fight, shedding your equipment for that of the gladiator.\n",  ch);

	if (red > blue)
	{
		//ch->race = lookup_race_id("Human");
		send_to_char ("#6\nYou have been placed on the Blue team: kill the red-splashed gladiators#0\n", ch);
		blue++;
		/*
		sprintf(rand, mob_vc_rand("$gladiator", &j));
		sprintf(buf2, "man %s blue-painted", rand);
		ch->name = str_dup(buf2);
		sprintf(buf2, "a blue-painted %s man", rand);
		ch->short_descr = str_dup(buf2);
		strcat(buf2, "is here.");
		*buf2 = toupper(*buf2);
		ch->long_descr = str_dup(buf2);
		*/
		add_clan (ch, "arena_blue", CLAN_MEMBER);
		arena_equip_pc_fighter(ch, cmd);
		//equip_slave(ch, cmd);
	}
	else
	{
		//ch->race = lookup_race_id("Human");
		send_to_char ("#6\nYou have been placed on the Red team: kill the blue-painted gladiators#0\n", ch);
		red++;
		/*
		sprintf(rand, mob_vc_rand("$gladiator", &j));
		sprintf(buf2, "male %s red-splashed", rand);
		ch->name = str_dup(buf2);
		sprintf(buf2, "a red-splashed %s male", rand);
		ch->short_descr = str_dup(buf2);
		strcat(buf2, "is here.");
		*buf2 = toupper(*buf2);
		ch->long_descr = str_dup(buf2);
		*/
		add_clan (ch, "arena_red", CLAN_MEMBER);
		arena_equip_pc_fighter(ch, cmd);
		//equip_slave(ch, cmd);
	}
	if (IS_SET (ch->plr_flags, NEWBIE_HINTS))
	{
		ch->plr_flags &= ~NEWBIE_HINTS;
	}
	if (!IS_SET (ch->plr_flags, COMBAT_FILTER))
	{
		ch->plr_flags |= COMBAT_FILTER;
	}

	/*
	randomize_mobile (ch);

	for (i = 1; i <= 18; i++) //weapon skills
	ch->skills[i] = number (20, 50);

	send_to_char ("#2\nThe battle will begin in just a moment. Good luck!#0\n", ch);
	ch->move = 100;
	ch->max_move = 100;
	ch->hit = 120;
	ch->max_hit = 120;
	ch->descr()->original = NULL;    
	sprintf (buf, "%s %s", ch->name, ch->tname);
	mem_free (ch->name);
	ch->name = str_dup (buf);
	*/

	char_from_room (ch);
	char_to_room(ch, GRUNGE_ARENA1 + number(0,3));
	return;
}
