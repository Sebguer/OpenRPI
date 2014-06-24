/*------------------------------------------------------------------------\
|  traps.cpp : Traps Module                              atonementrpi.com |
|  Copyright (C) 2009, Kithrater                                          |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"
#include "object_damage.h"

const char* trap_attrs2[8] =
{
	"None",
	"strength",
	"dexterity",
	"constitution",
	"intelligence",
	"willpower",
	"mutations",
	"agility"
};

// So, if we remove an object from the room, then we need to go through
// and make sure any traps that were set to go off on this object are
// now set to be duds. We also clear up the description.

void trap_obj_release (ROOM_DATA *room, OBJ_DATA *obj)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *tobj;

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (tobj == obj)
			continue;

		if ((GET_ITEM_TYPE(tobj) == ITEM_TRAP))
		{
			if (tobj->o.od.value[1] == obj->coldload_id)
			{
				tobj->o.od.value[0] = 0;
				tobj->o.od.value[1] = -1;
				sprintf(buf, "%s has been set here in a trap.", get_obj_id(tobj->o.od.value[2])->short_description);
				*buf = toupper(*buf);
				tobj->description = str_dup(buf);
			}
		}
	}
}


// A bit of a cheat for when people move in to a room that has a trap on it:
// we want the command to run a second after the first person enters the room
// so we can capture the entire group that entered, and not just hit the
// leader.

void
	delayed_trapped (CHAR_DATA *ch)
{
	OBJ_DATA *obj;

	if (!ch || ch->deleted || !(obj = ch->delay_obj))
	{
		ch->delay = 0;
		ch->delay_obj = NULL;
		ch->delay_type = 0;
		return;
	}

	if (trap_enact(ch, obj, 2, NULL) != 1)
	{
		clear_moves (ch);
		clear_current_move(ch);
	}

	ch->delay = 0;
	ch->delay_obj = NULL;
	ch->delay_type = 0;
}


// Now we check the components: if they were set to break on the first
// use, now they break. If they were set to break on the trap reset,
// we break them now, otheriswe, we load them back in the room for
// people to re-use.

int
	trap_component_result (OBJ_DATA *obj, OBJ_DATA *tobj, ROOM_DATA *room)
{

	OBJ_DATA *junk1 = NULL;
	OBJ_DATA *junk2 = NULL;

	if (!obj)
		return 0;
	if (!tobj)
		return 0;
	if (!tobj->trap)
		return 0;
	if (!room)
		return 0;


	if (IS_SET(tobj->trap->trap_bit, TRAP_BREAK_USE))
	{
		if ((junk1 = load_object(tobj->trap->com_junk1)))
			obj_to_room(junk1, room->vnum);
		if ((junk2 = load_object(tobj->trap->com_junk2)))
			obj_to_room(junk2, room->vnum);

		if (obj->o.od.value[2] == tobj->coldload_id)
			obj->o.od.value[2] = 0;

		if (obj->o.od.value[3] == tobj->coldload_id)
			obj->o.od.value[3] = 0;

		if (obj->o.od.value[4] == tobj->coldload_id)
			obj->o.od.value[4] = 0;

		if (obj->o.od.value[5] == tobj->coldload_id)
			obj->o.od.value[5] = 0;

		obj_from_obj(&tobj, 0);
		extract_obj(tobj);

		return 1;
	}

	if (obj->o.od.value[0] == 0)
	{
		if (!IS_SET(tobj->trap->trap_bit, TRAP_BREAK_RESET))
		{
			obj_from_obj(&tobj, 0);
			obj_to_room(tobj, room->vnum);
		}
		else
		{
			if ((junk1 = load_object(tobj->trap->com_junk1)))
				obj_to_room(junk1, room->vnum);
			if ((junk2 = load_object(tobj->trap->com_junk2)))
				obj_to_room(junk2, room->vnum);
			obj_from_obj(&tobj, 0);
			extract_obj(tobj);
		}
		return 1;
	}

	return 0;
}

int
	trap_component_affect (CHAR_DATA *ch, OBJ_DATA *tobj)
{
	TRAP_DATA *trap = NULL;
	int j = 0;
	int dam = 0;
	OBJ_DATA *armor = NULL;
	int wear_loc = 0;
	int loc = 0;
	int times1 = 1;
	int times2 = 1;
	//POISON_DATA *poison = NULL;

	if (!tobj)
		return 0;

	if (!(trap = tobj->trap))
		return 0;

	if (trap->trap_bit == 0)
		return 0;

	if (trap != tobj->trap)
		return 0;

	// If this component has the "temporary" bit set on it, then it's a sole-hit component that's
	// already shot it's load, and so we ignore.
	// Otherwise, if we're set to only hit once, we'll set the temp bit - removed at the end of
	// component round.

	if (IS_SET(trap->trap_bit, TRAP_TAR_TEMP))
		return 0;
	else if (IS_SET(trap->trap_bit, TRAP_TAR_SOLE))
		trap->trap_bit |= (TRAP_TAR_TEMP);

	// Basically, if we're not hitting a sole person, only do one bit of damage to each person
	// we're getting called, to save it for the rest of the group.

	if (trap->dam1_times)
		times1 = trap->dam1_times;

	if (trap->dam2_times)
		times2 = trap->dam2_times;

	if (IS_SET(trap->trap_bit, TRAP_AFF_DAMAGE))
	{
		if (trap->dam1_dice && trap->dam1_sides)
		{
			for (j = 0; j < times1; j++)
			{
				if (trap->dam1_loc > 7)
					loc = number(0,3);
				else
					loc = trap->dam1_loc;

				dam = (trap->dam1_dice * number(1,trap->dam1_sides)) + trap->dam1_bonus;

				wear_loc = body_tab[0][loc].wear_loc1;

				dam = real_damage(ch, dam, &wear_loc, trap->dam1_type, MELEE_TABLE);

				damage_objects(ch, dam, "", wear_loc, trap->dam1_type, MELEE_TABLE, false);

				if (wound_to_char (ch, figure_location (ch, loc), dam, trap->dam1_type, 0, 0, 0))
					return 0;
			}

			// We only do the second lot of damage for an object if we did the first lot, otherwise, no dice.
			if (trap->dam2_dice && trap->dam2_sides)
			{
				for (j = 0; j < times2; j++)
				{
					if (trap->dam2_loc > 7)
						loc = number(0,3);
					else
						loc = trap->dam2_loc;

					dam = (trap->dam2_dice * number(1,trap->dam2_sides)) + trap->dam2_bonus;

					wear_loc = body_tab[0][loc].wear_loc1;

					dam = real_damage(ch, dam, &wear_loc, trap->dam2_type, MELEE_TABLE);

					damage_objects(ch, dam, "", wear_loc, trap->dam2_type, MELEE_TABLE, false);

					if (wound_to_char (ch, figure_location (ch, loc), dam, trap->dam2_type, 0, 0, 0))
						return 0;
				}
			}
		}
	}

	// Pretty simple - if the trap has a poison bit, then add those poisons and remove a shot.

	/*
	if (IS_SET(trap->trap_bit, TRAP_AFF_SOMATIC))
	{
	if (tobj->poison)
	{
	for (poison = tobj->poison; poison; poison = poison->next)
	{
	if (poison->uses != 0)
	scent_add_affect(ch, poison->poison_type, poison->duration, poison->latency,
	poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
	poison->attack, poison->decay, poison->sustain, poison->release);

	if(poison->uses == 0 || poison->uses == 1)
	remove_object_poison(tobj, poison);
	else if(poison->uses > -1)
	poison->uses --;
	}
	}
	}
	*/

	if (IS_SET(trap->trap_bit, TRAP_AFF_BIND))
	{
		;
	}

	if (IS_SET(trap->trap_bit, TRAP_AFF_TRIP))
	{
		GET_POS (ch) = REST;
		add_second_affect (SA_STAND, ((20-GET_AGI(ch))+number(3,7)), ch, NULL, NULL, 0);
		if (is_outdoors(ch->room))
		{
			object__enviro(ch, NULL, COND_DIRT, 5, HITLOC_NONE);
			object__enviro(ch, NULL, COND_DUST, 10, HITLOC_NONE);
		}
	}


	return 1;

}

int
	trap_component_blank (CHAR_DATA *ch, OBJ_DATA *tobj, int size)
{
	int j = 0;
	int returned = 0;

	// Figure out whether it's the whole room, the whole group, part of the group, or if not, the sole person tripping it.
	if (tobj->trap->com_target_dice == -2)
		returned = 2;
	else if (tobj->trap->com_target_dice == -1)
	{
		j = size;
		returned = 1;
	}
	else if (tobj->trap->com_target_dice && tobj->trap->com_target_side)
	{
		j = tobj->trap->com_target_dice * (number(1,tobj->trap->com_target_side));

		if (j >= size)
			j = size;

		returned = 1;
	}

	if (size <= 1)
		returned = 0;

	return returned;

}

int
	trap_component_rounds (CHAR_DATA *ch, OBJ_DATA *tobj, int size, ROOM_DATA *room)
{
	bool sole = true;
	bool group = false;
	bool roomed = false;
	int i = 0;
	int hits = 0;
	int j = 0;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *temp_ch = NULL;

	int returned = 0;

	if (!tobj)
		return 0;

	if (!(tobj->trap))
		return 0;

	if (tobj->trap->trap_bit == 0)
		return 0;

	// Figure out whether it's the whole room, the whole group, part of the group, or if not, the sole person tripping it.
	if (tobj->trap->com_target_dice == -2)
	{
		roomed = true;
		sole = false;
		returned = 2;
	}
	else if (tobj->trap->com_target_dice == -1)
	{
		j = size;
		group = true;
		sole = false;
		returned = 1;
	}
	else if (tobj->trap->com_target_dice && tobj->trap->com_target_side)
	{
		j = tobj->trap->com_target_dice * (number(1,tobj->trap->com_target_side));

		if (j >= size)
		{
			j = size;
			group = true;
		}
		sole = false;
		returned = 1;
	}

	if (!room && (sole || size <= 1))
	{
		sole = true;
		group = false;
		trap_component_affect(ch, tobj);
		returned = 0;
	}

	// If we're not just a sole person, need to figure out who gets hit in the room.
	// This is -really- messy for the part-groups, but unfortunately, there isn't a
	// good way to randomly select guys from a linked-list.

	if (!sole)
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
		{
			if (tch->next_in_room)
				temp_ch = tch->next_in_room;

			if (roomed)
			{

				// Returns false is wound_char true (i.e. ch is dead).
				// As tch is dead, we need to link to the next guy in
				// the room to keep the prog running.

				if (!trap_component_affect(tch, tobj))
					tch = temp_ch;
				i++;
			}
			else if (group && (tch == ch || are_grouped(tch, ch)))
			{
				if (!trap_component_affect(tch, tobj))
					tch = temp_ch;
				i++;
			}
			else if ((i <= size) && (tch == ch || are_grouped(tch, ch)))
			{
				if (number(1, (size - i)) <= (j - hits))
				{
					if (!trap_component_affect(tch, tobj))
						tch = temp_ch;
					hits ++;
				}
				i++;
			}
			else if (i >= j)
				break;
		}
	}

	// Removing our temporary block so we this component can start afflicting people again.

	if (IS_SET(tobj->trap->trap_bit, TRAP_TAR_TEMP))
		tobj->trap->trap_bit &= ~(TRAP_TAR_TEMP);

	return returned;

}

void
	trap_assemble (CHAR_DATA *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	char arg4[MAX_STRING_LENGTH];
	char arg5[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *com_trig = NULL;
	OBJ_DATA *com_one = NULL;
	OBJ_DATA *com_two = NULL;
	OBJ_DATA *com_three = NULL;
	TRAP_DATA *trigger = NULL;
	TRAP_DATA *one = NULL;
	TRAP_DATA *two = NULL;
	TRAP_DATA *three = NULL;
	int skill = 0;
	int uses;
	int dir = -1;

	argument = one_argument(argument, arg);

	if (!*arg)
	{
		act ("Usage: trap assemble <object, 'room' or exit> <trigger> <components>", false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!strn_cmp ("north", arg, strlen (arg)))
		dir = 0;
	else if (!strn_cmp ("east", arg, strlen (arg)))
		dir = 1;
	else if (!strn_cmp ("south", arg, strlen (arg)))
		dir = 2;
	else if (!strn_cmp ("west", arg, strlen (arg)))
		dir = 3;
	else if (!strn_cmp ("up", arg, strlen (arg)))
		dir = 4;
	else if (!strn_cmp ("down", arg, strlen (arg)))
		dir = 5;

	if (dir >= 0)
	{
		if (!EXIT (ch, dir))
		{
			send_to_char ("There isn't an exit in that direction.\n", ch);
			return;
		}
	}
	else if (str_cmp("room", arg) && !(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 here.", arg);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	// Only one trap per exit and object to prevent insta-DEATH

	for (tobj = ch->room->contents; tobj; tobj = tobj->next_content)
	{
		if ((GET_ITEM_TYPE(tobj) == ITEM_TRAP))
		{
			if (dir >= 0 && dir == tobj->o.od.value[1])
			{
				send_to_char("A trap is already set over that exit. Disassemble the existing trap before setting a new one.\n", ch);
				return;
			}
			else if (obj && tobj->o.od.value[1] == obj->coldload_id)
			{
				act("$p has already been trapped. Disassemble the existing trap before setting a new one.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}
		}
	}

	argument = one_argument(argument, arg2);

	if (!(com_trig = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 in the room here.", arg2);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!(trigger = com_trig->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	if (!IS_SET (trigger->trap_bit, TRAP_ON_EXIT) &&
		!IS_SET (trigger->trap_bit, TRAP_ON_RELEASE) &&
		!IS_SET (trigger->trap_bit, TRAP_ON_OBJ))
	{
		act ("You can't use $p as the trigger for a trap.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	if (dir >= 0 && !IS_SET (trigger->trap_bit, TRAP_ON_EXIT))
	{
		act ("You can't use $p to trap an exit.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	if (obj && !IS_SET (trigger->trap_bit, TRAP_ON_OBJ))
	{
		act ("You can't use $p to trap an object.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	if (dir == -1 && !obj && !IS_SET (trigger->trap_bit, TRAP_ON_RELEASE))
	{
		act ("You can't use $p to set an on-release trap.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	argument = one_argument(argument, arg3);

	if (!*arg3)
		;
	else if (!(com_one = get_obj_in_list_vis (ch, arg3, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 in the room here.", arg3);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (com_one && !(one = com_one->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_one, 0, TO_CHAR);
		return;
	}

	if (com_one == com_trig)
		com_one = NULL;

	if (com_one && !IS_SET (one->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (one->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (one->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (one->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_one, 0, TO_CHAR);
		return;
	}

	argument = one_argument(argument, arg4);

	if (!*arg4)
		;
	else if (!(com_two = get_obj_in_list_vis (ch, arg4, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 in the room here.", arg4);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (com_two && !(two = com_two->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_two, 0, TO_CHAR);
		return;
	}

	if (com_two == com_trig)
		com_two = NULL;

	if (com_two && !IS_SET (two->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (two->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (two->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (two->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_two, 0, TO_CHAR);
		return;
	}

	argument = one_argument(argument, arg5);

	if (!*arg5)
		;
	else if (!(com_three = get_obj_in_list_vis (ch, arg5, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 in the room here.", arg5);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (com_three && !(three = com_three->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_three, 0, TO_CHAR);
		return;
	}

	if (com_three == com_trig)
		com_three = NULL;

	if (com_three && !IS_SET (three->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (three->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (three->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (three->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_three, 0, TO_CHAR);
		return;
	}

	if (!com_one && !com_two && !com_three &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't use $p alone as a trap: you need to add other components.", false, ch, com_trig, 0, TO_CHAR);
		return;
	}

	if (!(vtoo(VNUM_TRAP_OBJECT)))
	{
		send_to_char("An error has occured: please contact an admin.\n", ch);
		return;
	}

	if (com_trig)
	{
		if (com_trig == obj)
		{
			send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
			return;
		}
	}

	if (com_one)
	{
		if (com_one == obj)
		{
			send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
			return;
		}
	}

	if (com_two)
	{
		if (com_two == obj)
		{
			send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
			return;
		}
	}

	if (com_three)
	{
		if (com_three == obj)
		{
			send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
			return;
		}
	}


	// If they don't have the mechanics or engineering skill, give it to them.

	if (!(skill = ch->skills[SKILL_HUNTING]))
		skill_learn(ch, SKILL_HUNTING);

	if (com_three)
	{
		if (!com_two)
		{
			send_to_char("You don't see one of those components here.\n", ch);
			return;
		}
		if (!com_one)
		{
			send_to_char("You don't see one of those components here.\n", ch);
			return;
		}
		sprintf(buf, "Using $p as the trigger, and adding #2%s#0, #2%s#0 and #2%s#0, you work on assembling a trap ",
			com_one->short_description, com_two->short_description, com_three->short_description);
		sprintf(buf2, "Using $p as the trigger, and adding #2%s#0, #2%s#0 and #2%s#0, $n begins to assemble a trap ",
			com_one->short_description, com_two->short_description, com_three->short_description);
	}
	else if (com_two)
	{
		if (!com_one)
		{
			send_to_char("You don't see one of those components here.\n", ch);
			return;
		}
		sprintf(buf, "Using $p as the trigger, and adding #2%s#0 and #2%s#0, you work on assembling a trap ", com_one->short_description, com_two->short_description);
		sprintf(buf2, "Using $p as the trigger, and adding #2%s#0 and #2%s#0, $n begins to assemble a trap ", com_one->short_description, com_two->short_description);
	}
	else if (com_one)
	{
		sprintf(buf, "Using $p as the trigger, and adding #2%s#0, you work on assembling a trap ", com_one->short_description);
		sprintf(buf2, "Using $p as the trigger, and adding #2%s#0, $n begins to assemble a trap ", com_one->short_description);
	}
	else if (com_trig)
	{
		sprintf(buf, "Using $p, you work on assembling a trap ");
		sprintf(buf2, "Using $p, $n begins to assemble a trap ");
	}
	else
	{
		send_to_char("An error has occured: please contact an admin.", ch);
		return;
	}

	uses = trigger->com_trig_shots;

	if (uses <= 0)
		uses = -1;

	if (dir >= 0)
	{
		sprintf(buf + strlen(buf), "across the %sern exit ", dirs[dir]);
		sprintf(buf2 + strlen(buf2), "across the %sern exit ", dirs[dir]);
	}
	else if (obj)
	{
		sprintf(buf + strlen(buf), "about %s ", obj_short_desc(obj));
		sprintf(buf2 + strlen(buf2), "about %s ", obj_short_desc(obj));
	}

	sprintf(buf + strlen(buf), " . . .");
	sprintf(buf2 + strlen(buf2), ".");

	act (buf, false, ch, com_trig, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, com_trig, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay_type = DEL_TRAP_ASS_1;
	ch->delay = 12;

	if (obj)
		ch->delay_obj = obj;

	if (com_trig)
		ch->delay_info1 = (long int) com_trig;
	if (com_one)
		ch->delay_info2 = (long int) com_one;
	if (com_two)
		ch->delay_info3 = (long int) com_two;
	if (com_three)
		ch->delay_info4 = (long int) com_three;

	ch->delay_info5 = dir;

	criminalize(ch, NULL, ch->room->zone, CRIME_TRAPS);
}

void delayed_trap_assemble (CHAR_DATA * ch)
{

	act (". . . you continue assembling the trap . . .", false, ch, 0, 0, TO_CHAR);
	act ("$n continues to assemble a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay_type = DEL_TRAP_ASS_2;
	ch->delay = 12;
}


void delayed_trap_assemble2 (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char buf4[MAX_STRING_LENGTH];
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *trap_obj = NULL;
	OBJ_DATA *com_trig = NULL;
	OBJ_DATA *com_one = NULL;
	OBJ_DATA *com_two = NULL;
	OBJ_DATA *com_three = NULL;
	AFFECTED_TYPE *af = NULL;
	TRAP_DATA *trigger;
	TRAP_DATA *one;
	TRAP_DATA *two;
	TRAP_DATA *three;
	int skill = 0;
	int uses;
	int dir = -1;
	int diff = 0;
	int result = 0;
	int roll = 0;
	char *p;

	dir = ch->delay_info5;

	if (dir >= 0)
	{
		if (!EXIT (ch, dir))
		{
			send_to_char ("There isn't an exit in that direction.\n", ch);
			act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			return;
		}
	}
	else if (dir == -1)
		;
	else if (!(obj = ch->delay_obj) || !CAN_SEE_OBJ (ch, obj))
	{
		act ("The object you were trapping is no longer present!", false, ch, 0, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	// Only one trap per exit and object to prevent insta-DEATH

	for (tobj = ch->room->contents; tobj; tobj = tobj->next_content)
	{
		if ((GET_ITEM_TYPE(tobj) == ITEM_TRAP))
		{
			if (dir >= 0 && dir == tobj->o.od.value[1])
			{
				send_to_char("A trap is already set over that exit. Disassemble the existing trap before setting a new one.\n", ch);
				act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
			else if (obj && tobj->o.od.value[1] == obj->coldload_id)
			{
				act("$p has already been trapped. Disassemble the existing trap before setting a new one.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
		}
	}

	if (!(com_trig = (OBJ_DATA *) ch->delay_info1))
	{
		act ("Realising that some of your components are no longer present, you stop assembling the trap.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!(trigger = com_trig->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!IS_SET (trigger->trap_bit, TRAP_ON_EXIT) &&
		!IS_SET (trigger->trap_bit, TRAP_ON_RELEASE) &&
		!IS_SET (trigger->trap_bit, TRAP_ON_OBJ))
	{
		act ("You can't use $p as the trigger for a trap.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (dir >= 0 && !IS_SET (trigger->trap_bit, TRAP_ON_EXIT))
	{
		act ("You can't use $p to trap an exit.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (obj && !IS_SET (trigger->trap_bit, TRAP_ON_OBJ))
	{
		act ("You can't use $p to trap an object.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (dir == -1 && !obj && !IS_SET (trigger->trap_bit, TRAP_ON_RELEASE))
	{
		act ("You can't use $p to set an on-release trap.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!ch->delay_info2)
		;
	else if (!(com_one = (OBJ_DATA *) ch->delay_info2))
	{
		act ("Realising that some of your components are no longer present, you stop assembling the trap.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_one && !(one = com_one->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_one, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_one == com_trig)
		com_one = NULL;

	if (com_one && !IS_SET (one->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (one->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (one->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (one->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_one, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!ch->delay_info3)
		;
	else if (!(com_two = (OBJ_DATA *) ch->delay_info3))
	{
		act ("Realising that some of your components are no longer present, you stop assembling the trap.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_two && !(two = com_two->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_two, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_two == com_trig)
		com_two = NULL;

	if (com_two && !IS_SET (two->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (two->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (two->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (two->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_two, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!ch->delay_info4)
		;
	else if (!(com_three = (OBJ_DATA *) ch->delay_info4))
	{
		act ("Realising that some of your components are no longer present, you stop assembling the trap.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_three && !(three = com_three->trap))
	{
		act ("You can't use $p as part of a trap.", false, ch, com_three, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_three == com_trig)
		com_three = NULL;

	if (com_three && !IS_SET (three->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (three->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (three->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (three->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't add $p as a component: it doesn't look like it would do anything.", false, ch, com_three, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!com_one && !com_two && !com_three &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_DAMAGE) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_TRIP) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_BIND) &&
		!IS_SET (trigger->trap_bit, TRAP_AFF_SOMATIC))
	{
		act ("You can't use $p alone as a trap: you need to add other components.", false, ch, com_trig, 0, TO_CHAR);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!(trap_obj = load_object (VNUM_TRAP_OBJECT)))
	{
		send_to_char("An error has occured: please contact an admin.\n", ch);
		act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (com_trig)
		if (com_trig == obj)
		{
			send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
			act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			return;
		}

		if (com_one)
			if (com_one == obj)
			{
				send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
				act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}

			if (com_two)
				if (com_two == obj)
				{
					send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
					act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
					return;
				}

				if (com_three)
					if (com_three == obj)
					{
						send_to_char("You can't use an object as a lure if you're also going to use it as a component in the trap!\n", ch);
						act ("$n stops assembling a trap.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
						return;
					}

					// If they don't have the mechanics or engineering skill, give it to them.

					if (!(skill = ch->skills[SKILL_HUNTING]))
						skill_learn(ch, SKILL_HUNTING);

					// Now we go and find out what skill is required: more components mean more difficulty,
					// and then we roll vs MECHANICS. Critical fail is the trap hits us as we're making it,
					// major fail means the trap is a dud but we don't know that, and minor fail means we
					// know the trap is a dad (so we need to waste time fixing it).


					if (com_three && com_three->trap)
					{
						switch (com_three->trap->com_diff)
						{
						case 3:
							diff += 15;
							break;
						case 2:
							diff += 10;
							break;
						case 1:
							diff += 5;
							break;
						default:
							diff ++;
							break;
						}
					}

					if (com_two && com_two->trap)
					{
						switch (com_two->trap->com_diff)
						{
						case 3:
							diff += 15;
							break;
						case 2:
							diff += 10;
							break;
						case 1:
							diff += 5;
							break;
						default:
							diff ++;
							break;
						}
					}

					if (com_one && com_one->trap)
					{
						switch (com_one->trap->com_diff)
						{
						case 3:
							diff += 15;
							break;
						case 2:
							diff += 10;
							break;
						case 1:
							diff += 5;
							break;
						default:
							diff ++;
							break;
						}
					}

					if (com_trig && com_trig->trap && !com_one && !com_two && !com_three)
					{
						switch (com_trig->trap->com_diff)
						{
						case 3:
							diff += 25;
							break;
						case 2:
							diff += 20;
							break;
						case 1:
							diff += 10;
							break;
						default:
							diff ++;
							break;
						}
					}

					skill = skill_level(ch, SKILL_HUNTING, 0);
					roll = number(1, skill);

					// 3 means it hits the owner,
					// 2 means it's a dud -- not used anymore
					// 1 mean it's a dud but the owner knows it.
					// 0 means we're all OK.

					if (roll < diff)
					{
						// If we fail, then 1/3rd chance we hit ourselves.
						if (!number(0,2))
							result = 3;
						else
							result = 1;
					}
					else
						result = 0;

					//sprintf(buf, "Roll: %d Skill: %d Diff: %d Result: %d", roll, skill, diff, result);
					//send_to_gods(buf);


					sprintf(buf3, "This trap has been set %s%s with #2%s#0 as its trigger.", dir >= 0  ? "to the " : "", dir >= 0 ? dirs[dir] : "in the room", com_trig->short_description);

					if (com_three)
					{
						trap_obj->o.od.value[2] = com_trig->coldload_id;
						trap_obj->o.od.value[3] = com_one->coldload_id;
						trap_obj->o.od.value[4] = com_two->coldload_id;
						trap_obj->o.od.value[5] = com_three->coldload_id;

						sprintf(buf, "You finish using $p, #2%s#0, #2%s#0 and #2%s#0 to assemble $P",
							com_one->short_description, com_two->short_description, com_three->short_description);
						sprintf(buf3 + strlen(buf3), " When triggered, this trap will release #2%s#0, #2%s#0 and #2%s#0.",
							com_one->short_description, com_two->short_description, com_three->short_description);
					}
					else if (com_two)
					{
						trap_obj->o.od.value[2] = com_trig->coldload_id;
						trap_obj->o.od.value[3] = com_one->coldload_id;
						trap_obj->o.od.value[4] = com_two->coldload_id;

						sprintf(buf, "You finish using $p, #2%s#0 and #2%s#0 to assemble $P", com_one->short_description, com_two->short_description);
						sprintf(buf3 + strlen(buf3), " When triggered, this trap will release #2%s#0 and #2%s#0.",
							com_one->short_description, com_two->short_description);
					}
					else if (com_one)
					{
						trap_obj->o.od.value[2] = com_trig->coldload_id;
						trap_obj->o.od.value[3] = com_one->coldload_id;

						sprintf(buf, "You finish using $p and #2%s#0 to assemble $P", com_one->short_description);
						sprintf(buf3 + strlen(buf3), " When triggered, this trap will release #2%s#0.", com_one->short_description);
					}
					else if (com_trig)
					{
						trap_obj->o.od.value[2] = com_trig->coldload_id;
						sprintf(buf, "You finish using $p to make $P");
					}
					else
					{
						send_to_char("An error has occured: please contact an admin.", ch);
						return;
					}

					uses = trigger->com_trig_shots;

					if (uses <= 0)
						uses = -1;

					if (result == 1)
						sprintf(buf2, "$n assemble $p and hides it away.");
					else if (dir >= 0)
					{
						sprintf(buf + strlen(buf), " across the %sern exit.", dirs[dir]);
						sprintf(buf2, "$n assembles $p over the %sern exit.", dirs[dir]);
					}
					else if (obj)
					{
						sprintf(buf2, "$n assembles $p about %s.", obj_short_desc(obj));
						sprintf(buf + strlen(buf), " about %s.", obj_short_desc(obj));
					}
					else
					{
						sprintf(buf2, "$n assemble $p and hides it away.");
						sprintf(buf + strlen(buf), ".");
					}

					if (result == 1)
					{
						uses = 0;
						dir = -1;
						obj = NULL;
					}
					else
						trap_obj->o.od.value[0] = uses;

					if (result == 1)
						sprintf(buf + strlen(buf), " However, after completing the trap you realise that it is a dud, and will not release.");
					if (result == 3)
						sprintf(buf + strlen(buf), " However, with horrible certainty you realise your careless movements have triggered the trap.");

					/*
					if (obj)
					{
					sprintf(buf + strlen(buf), " using #2%s#0 as the lure.", obj_short_desc(obj));
					sprintf(buf3 + strlen(buf3), " It is using #2%s#0 as a lure.", obj_short_desc(obj));
					}
					*/

					if (!obj)
						trap_obj->o.od.value[1] = dir;
					else
						trap_obj->o.od.value[1] = obj->coldload_id;

					sprintf(buf4, "%s #1(trap)#2", com_trig->short_description);
					trap_obj->short_description = str_dup(buf4);

					sprintf(buf4, "%s trap set", com_trig->name);
					trap_obj->name = str_dup(buf4);

					if (dir >= 0)
						sprintf(buf4, "%s has been set across the %sern exit.", com_trig->short_description, dirs[dir]);
					else if (obj)
						sprintf(buf4, "%s has been set about %s", com_trig->short_description, obj_short_desc(obj));
					else
						sprintf(buf4, "%s has been set here in a trap.", com_trig->short_description);

					*buf4 = toupper(*buf4);

					sprintf(buf3 + strlen(buf3), " This trap has been put together %s, and appears to be %s complicated.",
						(skill > 75 ? "in a masterful fashion" : skill > 50 ? "in a professional manner" : skill > 35 ? "competently" : skill > 20 ? "simply" : "crudely"),
						(diff > 30 ? "fiendishly" : diff > 23 ? "extremely" : diff > 16 ? "somewhat" : diff > 10 ? "slightly" : "not very"));

					if (uses == 0)
						sprintf(buf3 + strlen(buf3), " This trap is a dud, and will not release.");
					else if (uses > 0)
						sprintf(buf3 + strlen(buf3), " This trap will trigger #2%d#0 time%s before needing to be reset.", uses, (uses > 1 ? "s" : ""));

					reformat_desc(buf3, &p);

					trap_obj->description = str_dup(buf4);

					trap_obj->full_description = str_dup(p);

					obj_to_room(trap_obj, ch->in_room);

					if (com_trig)
					{
						obj_from_room(&com_trig, 0);
						obj_to_obj(com_trig, trap_obj);
					}
					if (com_one)
					{
						obj_from_room(&com_one, 0);
						obj_to_obj(com_one, trap_obj);
					}
					if (com_two)
					{
						obj_from_room(&com_two, 0);
						obj_to_obj(com_two, trap_obj);
					}
					if (com_three)
					{
						obj_from_room(&com_three, 0);
						obj_to_obj(com_three, trap_obj);
					}

					act (buf, false, ch, com_trig, trap_obj, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, trap_obj, 0, TO_ROOM | _ACT_FORMAT);

					// Result 3 means we got hit by our own ju-ju.

					if (result == 3)
					{
						trap_enact (ch, trap_obj, 4, NULL);
					}
					{
						af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

						af->type = MAGIC_HIDDEN;
						af->a.hidden.duration = -1;
						af->a.hidden.hidden_value = skill;
						af->a.hidden.uu5 = diff;

						if (result == 1)
							af->a.hidden.uu6 = 0;
						else
							af->a.hidden.uu6 = ch->coldload_id;

						af->a.hidden.coldload_id = ch->coldload_id;
						af->next = trap_obj->xaffected;
						trap_obj->xaffected = af;
					}

					skill_use(ch, SKILL_HUNTING, diff);

}

void
	trap_detect (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	int speed = 1;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	AFFECTED_TYPE *af;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;

	argument = one_argument (argument, buf);

	if (!(str_cmp(buf, "slow")) || !(str_cmp(buf, "fast")))
	{
		if (!(str_cmp(buf, "slow")))
			speed = 2;
		else if (!(str_cmp(buf, "fast")))
			speed = 0;

		argument = one_argument (argument, buf);
	}

	if (!*buf)
	{
		act ("Usage: trap detect [slow|fast] <exit, object or room>", false, ch, 0, 0, TO_CHAR);
		return;
	}
	else
	{
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
			dir = -1;
			break;
		}
	}

	af = get_affect (ch, AFFECT_SHADOW);

	if (dir >= 0)
	{
		exit = EXIT (ch, dir);

		if (!exit || !(next_room = vnum_to_room (exit->to_room)))
		{
			send_to_char ("There's no such edge to search for traps abut.\n", ch);
			return;
		}

		if (af && (af->a.shadow.edge == dir))	/* On the edge */
		{
			sprintf(buf, "$n carefully searches the %s edge of the area.", dirs[dir]);
			sprintf(buf2, "You search carefully about the %s edge of the area...", dirs[dir]);
			act (buf, true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
			act (buf2, false, ch, 0, 0, TO_CHAR);
		}
		else
		{
			send_to_char ("You need to be standing by an edge to search for traps there.\n", ch);
			return;
		}
	}
	else if (dir == -1)
	{
		act ("$n carefully searches the area.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
		act ("You search carefully...", false, ch, 0, 0, TO_CHAR);
	}

	skill_learn(ch, SKILL_HUNTING);

	ch->delay_type = DEL_TRAP_SEARCH;
	ch->delay = 6 + ((6 + speed) * speed);
	ch->delay_info1 = dir;
	ch->delay_info2 = speed;
}

void
	delayed_trap_search (CHAR_DATA * ch)
{
	float search = 0;
	int search_base = 0;
	bool found = false;
	bool known = false;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *helm = NULL;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *taf = NULL;
	AFFECTED_TYPE *saf = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int dir = ch->delay_info1;
	int speed = ch->delay_info2;
	int skill = 0;
	int roll = 0;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;

	dir = ch->delay_info1;

	// So, assume 12 int and 12 wil, you'll get a search score between 37 and 77

	search = ((ch->intel * 3.0) + ch->wil) - 10 + number(0, 40);
	search_base = int(search);

	// If a character is wearing an enclosed helm, or something covering their face, then their search ability is significantly reduced.

	if (((helm = get_equip (ch, WEAR_HEAD)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK))
		|| ((helm = get_equip (ch, WEAR_FACE)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK)))
		search_base = search_base / 2;

	// We get the highest of either mechanics skill, their engineering skill -10, or their search_base divided by 2.

	skill = (skill_level(ch, SKILL_HUNTING, 0)) * 2;
	if (skill <= search_base)
		skill = search_base;

	// If you searched quickly, then you only have half the chance.

	if (speed == 2)
		skill = search_base * 2;
	else if (speed == 0)
		skill = search_base / 2;

	roll = number(1, skill);

	for (obj = ch->room->contents; obj; obj = obj->next_content)
	{
		known = false;

		if (!(af = get_obj_affect (obj, MAGIC_HIDDEN)))
			continue;

		if (GET_ITEM_TYPE(obj) != ITEM_TRAP)
			continue;

		// If we search north, then we search north.

		if (dir != obj->o.od.value[1])
			continue;

		if (!IS_LIGHT (ch->room)
			&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !is_goggled(ch)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
			continue;

		// If we already know it's there, continue.

		if (af->a.hidden.coldload_id == ch->coldload_id)
			continue;

		// If a trap's marked to be invisible, continue.

		if (get_obj_id(obj->o.od.value[2]) && get_obj_id(obj->o.od.value[2])->trap && IS_SET(get_obj_id(obj->o.od.value[2])->trap->trap_bit, TRAP_MISC_INVIS))
			continue;

		for (taf = obj->xaffected; taf; taf = taf->next)
		{
			if ((taf->type == MAGIC_HIDDEN) && (taf->a.hidden.coldload_id == ch->coldload_id))
				known = true;
		}

		if (known)
			continue;

		// Your roll at finding traps is made up by:
		// 1. 50% of your skill, plus
		// 2. between 0~100% of your skill

		if (search_base <= number (1, 100))
			continue;

		if (af->a.hidden.hidden_value
			&& roll < af->a.hidden.hidden_value)
			continue;

		if (roll < (af->a.hidden.hidden_value - 15))
		{
			skill_use(ch, SKILL_HUNTING, 0);

			if ((obj->o.od.value[0] == 0) || af->a.hidden.uu5 == 1)
				sprintf(buf, "You reveal $p, but too late you you accidentally triggered the trap in your search. Thankfully, it appears the trap was either not armed, or a dud.");
			else
			{
				act ("You reveal $p, but too late you realise you have accidentally triggered the trap in your search...", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				trap_enact(ch, obj, 4, NULL);
				break;
			}
		}
		else if (roll < af->a.hidden.hidden_value)
			continue;
		else
			sprintf(buf,"You espy $p, lying in wait for a victim.");

		found = true;

		saf = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
		saf->type = MAGIC_HIDDEN;
		saf->a.hidden.duration = -1;
		saf->a.hidden.hidden_value = af->a.hidden.hidden_value;
		saf->a.hidden.coldload_id = ch->coldload_id;
		saf->a.hidden.uu5 = af->a.hidden.uu5;
		saf->a.hidden.uu6 = af->a.hidden.uu6; // This is the coldload ID of the -original- person to hide the trap.
		saf->next = obj->xaffected;
		obj->xaffected = saf;
		act (buf, false, ch, obj, 0, TO_CHAR);

		break;
	}

	exit = EXIT (ch, dir);
	if (!found && exit && (next_room = vnum_to_room (exit->to_room)))
	{
		for (obj = next_room->contents; obj; obj = obj->next_content)
		{
			known = false;

			if (!(af = get_obj_affect (obj, MAGIC_HIDDEN)))
				continue;

			if (GET_ITEM_TYPE(obj) != ITEM_TRAP)
				continue;

			// If we searched north from the previous room, than we see if anything is set to the south

			if (rev_dir[dir] != obj->o.od.value[1])
				continue;

			if (!IS_LIGHT (next_room)
				&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !is_goggled(ch)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
				continue;

			if (af->a.hidden.coldload_id == ch->coldload_id)
				continue;

			// If a trap's marked to be invisible, continue.

			if (get_obj_id(obj->o.od.value[2]) && get_obj_id(obj->o.od.value[2])->trap && IS_SET(get_obj_id(obj->o.od.value[2])->trap->trap_bit, TRAP_MISC_INVIS))
				continue;

			for (taf = obj->xaffected; taf; taf = taf->next)
			{
				if ((taf->type == MAGIC_HIDDEN) && (taf->a.hidden.coldload_id == ch->coldload_id))
					known = true;
			}

			if (known)
				continue;

			if (search_base <= number (1, 100))
				continue;

			if (af->a.hidden.hidden_value
				&& roll < af->a.hidden.hidden_value)
				continue;

			if (roll < (af->a.hidden.hidden_value - 15))
			{
				skill_use(ch, SKILL_HUNTING, 0);

				if ((obj->o.od.value[0] == 0) || af->a.hidden.uu5 == 1)
					sprintf(buf, "To the %s, you reveal $p, but too late you you accidentally triggered the trap in your search. Thankfully, it appears the trap was either not armed, or a dud.", dirs[dir]);
				else
				{
					sprintf(buf, "To the %s, you reveal $p, but too late you realise you have accidentally triggered the trap in your search...", dirs[dir]);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					trap_enact(ch, obj, 4, NULL);
					break;
				}
			}
			else if (roll < af->a.hidden.hidden_value)
				continue;
			else
				sprintf(buf,"To the %s, you espy $p, lying in wait for a victim.", dirs[dir]);

			found = true;

			saf = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
			saf->type = MAGIC_HIDDEN;
			saf->a.hidden.duration = -1;
			saf->a.hidden.hidden_value = af->a.hidden.hidden_value;
			saf->a.hidden.coldload_id = ch->coldload_id;
			saf->a.hidden.uu5 = af->a.hidden.uu5;
			saf->a.hidden.uu6 = af->a.hidden.uu6; // This is the coldload ID of the -original- person to hide the trap.
			saf->next = obj->xaffected;
			obj->xaffected = saf;
			act (buf, false, ch, obj, 0, TO_CHAR);

			break;
		}
	}

	if (!found)
		send_to_char ("You didn't find anything.\n", ch);

	act ("$n finishes searching.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
	return;

}


void
	trap_disassemble (CHAR_DATA *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	int dir = -1;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;


	argument = one_argument(argument, arg);

	if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{
		sprintf (buf, "You don't see #2%s#0 in the room here.", arg);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (GET_ITEM_TYPE(obj) != ITEM_TRAP)
	{
		act ("There's no need to disassemble $p: it isn't a set trap.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	skill_learn(ch, SKILL_HUNTING);

	if (!(af = get_obj_affect(obj, MAGIC_HIDDEN)))
	{
		act ("Something is wrong with $p. Please contact an admin.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	af = get_affect (ch, AFFECT_SHADOW);

	dir = obj->o.od.value[1];

	if (dir >= 0 && dir <= 5)
	{
		exit = EXIT (ch, dir);

		if (!exit || !(next_room = vnum_to_room (exit->to_room)))
		{
			send_to_char ("There's no exit in that direction.\n", ch);
			return;
		}

		if (af && (af->a.shadow.edge == dir))	/* On the edge */
			;
		else
		{
			send_to_char ("You need to be standing by an exit to disassemble traps set across it.\n", ch);
			return;
		}
	}

	act ("You begin carefully disassembling $p.", false, ch, obj, 0, TO_CHAR);
	act ("$n begins to carefully disassemble $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay_type = DEL_TRAP_DIS;
	ch->delay = 9;
	ch->delay_obj = obj;

}


void delayed_trap_dissamble (CHAR_DATA * ch)
{
	int diff = 0;
	int roll = 0;
	int result = 0;
	OBJ_DATA *obj;
	int skill = 0;
	OBJ_DATA *com_trig;
	OBJ_DATA *com_one;
	OBJ_DATA *com_two;
	OBJ_DATA *com_three;
	AFFECTED_TYPE *af;

	if (!(obj = ch->delay_obj))
	{
		act ("The trap you were disassembling is no longer here.", false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!(af = get_obj_affect(obj, MAGIC_HIDDEN)))
	{
		act ("Something is wrong with $p. Please contact an admin.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	// The difficulty in defusing a trap is the overall difficulty of the trap plus the skill of the person who hid it.

	diff = (af->a.hidden.uu5 + af->a.hidden.hidden_value) / 2;

	// If it's our trap, cut the difficult by 75%.

	if (ch->coldload_id == af->a.hidden.uu6)
		diff = diff * 3 / 4;

	// If the trap is disarmed, cut the difficulty by 50%

	if (obj->o.od.value[0] == 0)
		diff = diff / 2;

	// Our roll is pretty simple: 2dskill/2, + 25% of skill. So, if we have 60 skill, that'll be on average 45.
	// I do 2dskill/2 to even out the dice rolls, otherwise it's a -bit- too random.

	skill = skill_level(ch, SKILL_HUNTING, 0);

	roll = number(2, skill/2) + (skill/4) - 1;

	// If we fail by more than 15, we get hit. If we fail, then we just do nothing. If we pass, it's disassembled.

	if (roll < (diff - 15))
		result = 2;
	else if (roll < diff)
		result = 1;
	else
		result = 0;

	if (result == 2)
	{
		act ("In attempting to disassemble $p you realise too late you have triggered the trap.", false, ch, obj, 0, TO_CHAR);
		skill_use(ch, SKILL_HUNTING, diff);
		trap_enact(ch, obj, 4, NULL);
		return;
	}
	else if (result == 1)
	{
		act ("You're were unable to determine a safe way to disable $p.", false, ch, obj, 0, TO_CHAR);
		act ("$n stops attempting to disassemble $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		skill_use(ch, SKILL_HUNTING, diff);
		return;
	}
	else
	{
		if ((com_trig = get_obj_id(obj->o.od.value[2])))
		{
			if (!com_trig->in_obj)
			{
				send_to_char ("Object not found - bug has been logged.\n", ch);
				send_to_gods ("Trap bug - object being sought was not in trap!");
			}
			else
			{
				obj_from_obj(&com_trig, 0);
				obj_to_room(com_trig, ch->in_room);
			}
		}
		if ((com_one = get_obj_id(obj->o.od.value[3])))
		{
			if (!com_one->in_obj)
			{
				send_to_char ("Object not found - bug has been logged.\n", ch);
				send_to_gods ("Trap bug - object being sought was not in trap!");
			}
			else
			{
				obj_from_obj(&com_one, 0);
				obj_to_room(com_one, ch->in_room);
			}
		}
		if ((com_two = get_obj_id(obj->o.od.value[4])))
		{
			if (!com_two->in_obj)
			{
				send_to_char ("Object not found - bug has been logged.\n", ch);
				send_to_gods ("Trap bug - object being sought was not in trap!");
			}
			else
			{
				obj_from_obj(&com_two, 0);
				obj_to_room(com_two, ch->in_room);
			}
		}
		if ((com_three = get_obj_id(obj->o.od.value[5])))
		{
			if (!com_three->in_obj)
			{
				send_to_char ("Object not found - bug has been logged.\n", ch);
				send_to_gods ("Trap bug - object being sought was not in trap!");
			}
			else
			{
				obj_from_obj(&com_three, 0);
				obj_to_room(com_three, ch->in_room);
			}
		}

		remove_obj_mult_affect(obj, MAGIC_HIDDEN);

		act ("You successfully disassemble $p, laying out the components before you.", false, ch, obj, 0, TO_CHAR);
		act ("$n disassembles $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		obj_from_room(&obj, 0);
		return;
	}

}


void
	trap_examine (CHAR_DATA *ch, char *argument, int cmd)
{
	;
}


void
	trap_defuse (CHAR_DATA *ch, char *argument, int cmd)
{
	;
}

int trap_enact (CHAR_DATA *tch, OBJ_DATA *obj, int cmd, OBJ_DATA *tobj)
{
	char tbuf[6][AVG_STRING_LENGTH];
	char tbuf2[6][AVG_STRING_LENGTH];
	OBJ_DATA *trig = NULL;
	OBJ_DATA *one = NULL;
	OBJ_DATA *two = NULL;
	OBJ_DATA *three = NULL;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *saf;
	int people = 0;
	int i = 0;
	int door = 0;
	int attr;
	int die;
	int sides;
	int roll = 0;
	int score = 0;
	int skill = 0;
	ROOM_DATA *room;

	int num_rooms = 0;
	int dists[37];
	int dir = 0;
	ROOM_DATA *rooms[37];

	bool first = false;
	bool known = false;

	for (int i = 0; i < 5; i++)
	{
		tbuf[i][0] = '\0';
		tbuf2[i][0] = '\0';
	}

	// Prelim stuff to make sure we've got a trap, a component, and a trigger.

	if (!obj)
		return 2;

	if (!(trig = get_obj_id(obj->o.od.value[2])))
		return 2;

	if (!trig)
		return 2;

	if (!trig->trap)
		return 2;

	if (!tch)
		return 2;

	if (!(room = tch->room))
		return 2;

	// If it's not self-inflicted, and the trap is a dud, then return.

	if (cmd != 4 && obj->o.od.value[0] == 0)
		return 2;

	// Obviously, if the trigger's not set for it, then it doesn't happen.

	if (cmd == 1 && !IS_SET(trig->trap->trap_bit, TRAP_ON_RELEASE))
		return 2;
	if (cmd == 2 && !IS_SET(trig->trap->trap_bit, TRAP_ON_EXIT))
		return 2;
	if (cmd == 3 && !IS_SET(trig->trap->trap_bit, TRAP_ON_OBJ))
		return 2;

	// If you set the trap, then it doesn't target you.

	if (cmd >= 2 && cmd <= 3)
		for (af = obj->xaffected; af; af = af->next)
			if (af->type == MAGIC_HIDDEN)
				if (af->a.hidden.uu6 == tch->coldload_id)
					return 2;

	// For directional movement, if they know the trap is there, they don't get sprung by it, unless it's set to always strike.
	// Special for directional, because you may, conceivable, be able to just not walk down -that- particular path to go north.
	// For objects, you're always likely to be struck by it.

	if (cmd == 2 && !IS_SET(trig->trap->trap_bit, TRAP_MISC_ALWAYS))
		for (af = obj->xaffected; af; af = af->next)
			if (af->type == MAGIC_HIDDEN)
				if (af->a.hidden.coldload_id == tch->coldload_id)
					return 2;


	// If we can avoid this trap, and we've got all the data, then make the roll.
	// Basically, if you roll equal to or greater than the attribute in question,
	// you make a lucky safe, unless it's a self-trigger or a release trigger,
	// where you then get no mercy.

	if (IS_SET(trig->trap->trap_bit, TRAP_TRIG_AVOID) && (attr = trig->trap->avoid_atr) && (die = trig->trap->avoid_dice) && (sides = trig->trap->avoid_sides))
	{
		switch (attr)
		{
		case 1:
			roll = GET_STR(tch);
			break;
		case 2:
			roll = GET_DEX(tch);
			break;
		case 3:
			roll = GET_CON(tch);
			break;
		case 4:
			roll = GET_INT(tch);
			break;
		case 5:
			roll = GET_WIL(tch);
			break;
		case 6:
			roll = GET_AUR(tch);
			break;
		case 7:
			roll = GET_AGI(tch);
			break;
		default:
			roll = -1;
			break;
		}

		// You never really have a chance to avoid a released trap or one you accidentally trigger yourself.

		if (cmd == 1 || cmd == 4)
			roll = -1;

		// If it's a movement-based trap, the speed at which you're moving will determine how likely it is you'll get hit.
		// If you're moving at anything above a walk, your ability to detect a trap is -seriously- hindered.

		if (cmd == 2)
		{
			switch (tch->speed)
			{
			case 1:         // trudging
				roll += 3;
				break;
			case 2:         // pacing neither hinders nor helps.
				roll += 0;
				break;
			case 3:         // jogging
				roll -= 3;
				break;
			case 4:         // running
				roll -= 6;
				break;
			case 5:         // sprinting
				roll -= 9;
				break;
			case 8:         // crawling
				roll += 9;
			default:         // walking
				roll -= 1;
				break;
			}
		}

		// If you already know the trap is there, it's much easier to aovid.

		for (af = obj->xaffected; af; af = af->next)
			if (af->type == MAGIC_HIDDEN)
				if (af->a.hidden.coldload_id == tch->coldload_id)
					known = true;


		// We then add the person's skill to the ability to detect, based off 40. People who suck at traps can't hide them very well.

		af = get_obj_affect(obj, MAGIC_HIDDEN);
		skill = af->a.hidden.hidden_value / 10;
		skill = skill - 4;
		roll -= skill;

		if (known)
			roll += 3;

		// If your roll is higher than the dice, you notice the trap, otherwise, you'll be hit by it.

		if (roll > 0)
		{
			if (roll >= (score = dice (die, sides)))
			{

				// If you didn't know the trap was already there, you do now.

				if ((af = get_obj_affect(obj, MAGIC_HIDDEN)) && !known)
				{
					saf = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
					saf->type = MAGIC_HIDDEN;
					saf->a.hidden.duration = -1;
					saf->a.hidden.hidden_value = af->a.hidden.hidden_value;
					saf->a.hidden.coldload_id = tch->coldload_id;
					saf->a.hidden.uu5 = af->a.hidden.uu5;
					saf->a.hidden.uu6 = af->a.hidden.uu6; // This is the coldload ID of the -original- person to hide the trap.
					saf->next = obj->xaffected;
					obj->xaffected = saf;
				}

				if (cmd == 2)
				{
					sprintf(tbuf[0], "By virtue of your %s, you %sspot $p before it is too late.", trap_attrs2[attr],
						(roll >= score + 3 ? "easily " : roll > score ? "" : "barely "));
					act (tbuf[0], false, tch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act ("$n comes to a stop.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
				}
				else if (cmd == 3)
				{
					sprintf(tbuf[0], "By virtue of your %s, you %sspot $p as you reach for $P.", trap_attrs2[attr],
						(roll >= score + 3 ? "easily " : roll > score ? "" : "barely "));
					act (tbuf[0], false, tch, obj, tobj, TO_CHAR | _ACT_FORMAT);
					act ("$n pauses as $e reaches for $P.", false, tch, obj, tobj, TO_ROOM | _ACT_FORMAT);
				}

				return 1;
			}
		}
	}

	// Load the rest of the stuff

	one = get_obj_id(obj->o.od.value[3]);
	two = get_obj_id(obj->o.od.value[4]);
	three = get_obj_id(obj->o.od.value[5]);

	// Now, we determine how many people there are, then we whack everyone after we spit out the message.

	int size = do_group_size(tch) + 1;

	people = trap_component_blank(tch, trig, size);

	// Now, create the damage string, and print it out to the room.

	if (trig && trig->trap && trig->trap->trigger_1st && trig->trap->trigger_3rd)
	{
		if (tobj)
		{
			sprintf (tbuf[0], "Reaching for $P, you");
			sprintf (tbuf2[0], "Reaching for $P, $n");
		}
		else
		{
			sprintf (tbuf[0], "You");
			sprintf (tbuf2[0], "$n");
		}

		if (people == 2)
		{
			sprintf (tbuf[0] + strlen(tbuf[0]), ", and all others in the room,");
			sprintf (tbuf2[0] + strlen(tbuf2[0]), ", and all others in the room,");
		}
		else if (people == 1)
		{
			sprintf (tbuf[0] + strlen(tbuf[0]), ", and those following you,");
			sprintf (tbuf2[0] + strlen(tbuf2[0]), ", and those following $m,");
		}

		sprintf (tbuf[0] + strlen(tbuf[0]), " %s", trig->trap->trigger_1st);
		sprintf (tbuf2[0] + strlen(tbuf2[0]), " %s", trig->trap->trigger_3rd);

		i++;

		if (trig->trap->strike_1st && trig->trap->strike_3rd)
		{
			sprintf (tbuf[1], "%s", trig->trap->strike_1st);
			sprintf (tbuf2[1], "%s", trig->trap->strike_3rd);
			i++;
		}
		if (one && one->trap && one->trap->strike_1st && one->trap->strike_3rd)
		{
			sprintf (tbuf[2], "%s", one->trap->strike_1st);
			sprintf (tbuf2[2], "%s", one->trap->strike_3rd);
			i++;
		}
		if (two && two->trap && two->trap->strike_1st && two->trap->strike_3rd)
		{
			sprintf (tbuf[3], "%s", two->trap->strike_1st);
			sprintf (tbuf2[3], " %s", two->trap->strike_3rd);
			i++;
		}
		if (three && three->trap && three->trap->strike_1st && three->trap->strike_3rd)
		{
			sprintf (tbuf[4], "%s", three->trap->strike_1st);
			sprintf (tbuf2[4], "%s", three->trap->strike_3rd);
			i++;
		}

		sprintf(tbuf[5], "%s", tbuf[0]);
		sprintf(tbuf2[5], "%s", tbuf2[0]);

		if (i > 1)
		{
			first = true;
			for (int j = 0; j < 5; j++)
			{
				if (!first)
				{
					if (*tbuf[j] && j == i)
					{
						sprintf(tbuf[5] + strlen(tbuf[5]), " and %s", tbuf[j]);
						sprintf(tbuf2[5] + strlen(tbuf2[5]), " and %s", tbuf2[j]);
						//break;
					}
					else if (*tbuf[j])
					{
						sprintf(tbuf[5] + strlen(tbuf[5]), ", %s", tbuf[j]);
						sprintf(tbuf2[5] + strlen(tbuf2[5]), ", %s", tbuf2[j]);
					}
				}
				else
					first = false;
			}
		}

		sprintf(tbuf[5] + strlen(tbuf[5]), ".");
		sprintf(tbuf2[5] + strlen(tbuf2[5]), ".");

	}
	else
	{
		if (tobj)
		{
			sprintf (tbuf[5], "Reaching for $P, you trigger $p.");
			sprintf (tbuf2[5], "Reaching for $P, $n triggers $p.");
		}
		else
		{
			sprintf (tbuf[5], "You trigger $p.");
			sprintf (tbuf2[5], "$n triggers $p.");
		}

	}

	act(tbuf[5], false, tch, trig, tobj, TO_CHAR | _ACT_FORMAT);
	act(tbuf2[5], false, tch, trig, tobj, TO_ROOM | _ACT_FORMAT);

	// Now, let's make some noise in the next room!

	i = 0;

	if (trig->trap->shout_1)
	{
		sprintf (tbuf[1], "%s", trig->trap->shout_1);
		i++;
	}
	if (one && one->trap && one->trap->shout_1)
	{
		sprintf (tbuf[2], "%s", one->trap->shout_1);
		i++;
	}
	if (two && two->trap && two->trap->shout_1)
	{
		sprintf (tbuf[3], "%s", two->trap->shout_1);
		i++;
	}
	if (three && three->trap && three->trap->shout_1)
	{
		sprintf (tbuf[4], "%s", three->trap->shout_1);
		i++;
	}

	sprintf(tbuf[5], "%s", tbuf[1]);

	if (i > 2)
	{
		first = true;
		for (int j = 1; j < 5; j++)
		{
			if (!first)
			{
				if (*tbuf[j] && j == i)
				{
					sprintf(tbuf[5] + strlen(tbuf[5]), " and %s", tbuf[j]);
				}
				else if (*tbuf[j])
				{
					sprintf(tbuf[5] + strlen(tbuf[5]), ", %s", tbuf[j]);
				}
			}
			else
				first = false;
		}
	}

	sprintf(tbuf[5] + strlen(tbuf[5]), ".");

	if ( i >= 1)
		for (door = 0; door <= 5; door++)
			if (EXIT (tch, door) && (EXIT (tch, door)->to_room != -1))
			{
				sprintf(tbuf[0], "From %s %s", rev_d[door], tbuf[5]);
				send_to_room (tbuf[0], EXIT (tch, door)->to_room);
			}

			// Now, let's go through and make a noise in the rooms beyond that!

			// Now, let's make some noise in the next room!

			i = 0;

			if (trig->trap->shout_1)
			{
				sprintf (tbuf[1], "%s", trig->trap->shout_1);
				i++;
			}
			if (one && one->trap && one->trap->shout_1)
			{
				sprintf (tbuf[2], "%s", one->trap->shout_1);
				i++;
			}
			if (two && two->trap && two->trap->shout_1)
			{
				sprintf (tbuf[3], "%s", two->trap->shout_1);
				i++;
			}
			if (three && three->trap && three->trap->shout_1)
			{
				sprintf (tbuf[4], "%s", three->trap->shout_1);
				i++;
			}

			sprintf(tbuf[5], "%s", tbuf[1]);

			if (i > 2)
			{
				first = true;
				for (int j = 1; j < 5; j++)
				{
					if (!first)
					{
						if (*tbuf[j] && j == i)
						{
							sprintf(tbuf[5] + strlen(tbuf[5]), " and %s", tbuf[j]);
						}
						else if (*tbuf[j])
						{
							sprintf(tbuf[5] + strlen(tbuf[5]), ", %s", tbuf[j]);
						}
					}
					else
						first = false;
				}
			}

			sprintf(tbuf[5] + strlen(tbuf[5]), ".");

			for (door = 0; door <= 36; door++)
				dists[door] = 0;
			for (door = 0; door <= 36; door++)
				rooms[door] = NULL;

			get_room_list (2, tch->room, rooms, dists, &num_rooms);


			if ( i >= 1)
			{
				for (door = 2; door < num_rooms; door++)
				{
					if (rooms[door]->people)
					{
						dir = track (rooms[door]->people, tch->room->vnum);

						if (dir == -1)
							continue;

						sprintf(tbuf[0], "From the %s, %s", dirs[dir], tbuf[5]);
						send_to_room (tbuf[0], rooms[door]->vnum);
					}
				}
			}

			if (trig && trig->trap)
				trap_component_rounds(tch, trig, size, room);
			if (one && one->trap)
				trap_component_rounds(tch, one, size, room);
			if (two && two->trap)
				trap_component_rounds(tch, two, size, room);
			if (three && three->trap)
				trap_component_rounds(tch, three, size, room);

			// First, we check how many uses the trap has: this was first set when we assembled the trap, based on the trigger's trap values.

			if (obj->o.od.value[0] > 0)
				obj->o.od.value[0] --;

			// Next, we go through and see if any of our components were break_on_use, and if so, produce their junk object.

			trap_component_result(obj, trig, room);
			trap_component_result(obj, one, room);
			trap_component_result(obj, two, room);
			trap_component_result(obj, three, room);

			// If the trap is worn out, grab it from the room.

			if (obj->o.od.value[0] == 0)
				obj_from_room(&obj, 0);

			return 0;
}

void
	trap_release (CHAR_DATA *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	OBJ_DATA *trig = NULL;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *tmp_tch = NULL;
	AFFECTED_TYPE *af = NULL;
	int i = 0;


	argument = one_argument(argument, arg);

	if (!*arg)
	{
		act ("Usage: trap release <trap> <target>", false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{
		sprintf (buf, "You don't see %s here.", arg);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (GET_ITEM_TYPE(obj) != ITEM_TRAP)
	{
		act ("You can't do that with $p.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	if (!(trig = get_obj_id(obj->o.od.value[2])))
	{
		send_to_char("An error has occured: please contact an admin.", ch);
		return;
	}

	if (!trig->trap)
	{
		send_to_char("An error has occured: please contact an admin.", ch);
		return;
	}

	if (!IS_SET(trig->trap->trap_bit, TRAP_ON_RELEASE))
	{
		act ("$p isn't the type of trap you can manually release.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	if (obj->o.od.value[1] != -1)
	{
		act ("$p hasn't been set to be released manually.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	if (!(af = obj->xaffected) && !(af = get_obj_affect (obj, MAGIC_HIDDEN)))
	{
		act ("But $p isn't hidden from sight: you can hardly expect it to surprise anyone.", false, ch, obj, 0, TO_CHAR);
		return;
	};

	argument = one_argument(argument, arg);

	if (!(tch = get_char_room_vis (ch, arg)))
	{
		sprintf (buf, "You don't see %s here.", arg);
		act (buf, false, ch, 0, 0, TO_CHAR);
		return;
	}

	// If they're already fighting, then the trap's liklely to hit someone
	// engaged in combat with them.
	if (tch->fighting)
	{
		i = num_attackers(tch);
		if (number(0,i))
			if ((tmp_tch = who_attackers(tch)))
				tch = tmp_tch;
	}

	if (trap_enact(tch, obj, 1, NULL))
	{
		act ("You attempt to release $p, but nothing happens.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	}
}


void do_trap (CHAR_DATA *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);

	if (!*arg)
	{
		act ("Usage: trap <assemble|detect|disassemble|release> <arguments>", false, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!str_cmp(arg, "assemble"))
		trap_assemble(ch, argument, cmd);
	else if (!str_cmp(arg, "disassemble"))
		trap_disassemble(ch, argument, cmd);
	else if (!str_cmp(arg, "release"))
		trap_release(ch, argument, cmd);
	else if (!str_cmp(arg, "detect"))
		trap_detect(ch, argument, cmd);
	else
	{
		act ("That's not a valid trap function.", false, ch, 0, 0, TO_CHAR);
		return;
	}

}
