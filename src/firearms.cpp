/*------------------------------------------------------------------------\
|  firearms.cpp : Firearms Module                        atonementrpi.com |
|  Copyright (C) 2010, Kithrater                                          |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"
#include "constants.h"
#include "object_damage.h"

const char *direct[] =
{ "north", "east", "south", "west", "up", "down", "area", "\n" };

// 0 = firearm is loaded, wielded, and ready to fire
// 1 = firearm is wielded
// 2 = firearm is held.

OBJ_DATA *
	has_firearm (CHAR_DATA *ch, int cmd)
{
	OBJ_DATA *obj = NULL;
	if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_FIREARM)
	{
		obj = ch->right_hand;
		if ((cmd == 0 &&
			count_bullets(obj) &&
			(obj->location == WEAR_PRIM || obj->location == WEAR_SEC || obj->location == WEAR_BOTH) &&
			obj->o.firearm.setting != 1) ||
			(cmd == 1 &&
			(obj->location == WEAR_PRIM || obj->location == WEAR_SEC || obj->location == WEAR_BOTH)) ||
			cmd == 2)
		{
			return obj;
		}
	}

	if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_FIREARM)
	{
		obj = ch->left_hand;
		if ((cmd == 0 &&
			count_bullets(obj) &&
			(obj->location == WEAR_PRIM || obj->location == WEAR_SEC || obj->location == WEAR_BOTH) &&
			obj->o.firearm.setting != 1) ||
			(cmd == 1 &&
			(obj->location == WEAR_PRIM || obj->location == WEAR_SEC || obj->location == WEAR_BOTH)) ||
			cmd == 2)
		{
			return obj;
		}
	}

	return NULL;
}

void
	do_pointblank (CHAR_DATA *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int bodypart = 0;

	OBJ_DATA *obj = NULL;
	CHAR_DATA *tch = NULL;

	argument = one_argument(argument, arg);

	if (!*arg)
	{
		send_to_char("Usage: blank <target> <location>\n", ch);
		return;
	}

	if (!(tch = get_char_room_vis(ch, arg)))
	{
		sprintf(buf, "You don't see #5%s#0 here.\n", arg);
		send_to_char(buf, ch);
		return;
	}

	if (tch == ch)
		sprintf(arg, "head");
	else
		argument = one_argument (argument, arg);

	if (!*arg)
	{
		send_to_char ("Which bodypart do you want to put your firearm against?\n", ch);
		return;
	}
	if (!strcmp(arg, "body"))
	{
		bodypart = HITLOC_BODY;
	}
	else if (!strcmp(arg, "leg") || !strcmp(arg, "legs"))
	{
		if (number(0,1))
		{
			bodypart = HITLOC_HILEGS;
		}
		else
		{
			bodypart = HITLOC_LOLEGS;
		}
	}
	else if (!strcmp(arg, "arm") || !strcmp(arg, "arms"))
	{
		if (number(0,1))
		{
			bodypart = HITLOC_HIARMS;
		}
		else
		{
			bodypart = HITLOC_LOARMS;
		}
	}
	else if (!strcmp(arg, "head"))
	{
		bodypart = HITLOC_HEAD;
	}
	else if (!strcmp(arg, "foot") || !strcmp(arg, "feet"))
	{
		bodypart = HITLOC_FEET;
	}
	else if (!strcmp(arg, "hand") || !strcmp(arg, "hands"))
	{
		bodypart = HITLOC_HANDS;
	}
	else
	{
		send_to_char ("That's not a bodypart! Choose head, arms, hands, body, legs or feet.\n", ch);
		return;
	}

	if (!((obj = get_equip (ch, WEAR_BOTH)) || (obj = get_equip (ch, WEAR_PRIM))  || (obj = get_equip (ch, WEAR_SEC))) || (GET_ITEM_TYPE(obj) != ITEM_FIREARM))
	{
		send_to_char("You need to be wielding a firearm to do that.\n", ch);
		return;
	}

	broke_aim(ch, 0);

	if (tch == ch)
	{
		if (ch->fighting)
		{
			act("Fending off the blows, you somehow manage to prepare to place $p against your head.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n prepares to place $p against $s head.",false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_COMBAT);
			ch->delay = 5;
			ch->delay_type = DEL_POINTBLANK;
			ch->delay_ch = ch;
		}
		else
		{
			act("You place $p against your head.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n places $p against $s head.",false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			ch->delay = 1;
			ch->delay_type = DEL_POINTBLANK;
			ch->delay_ch = ch;
		}
		bodypart = 3;
	}
	else if (get_affect (ch, MAGIC_HIDDEN))
	{
		sprintf(buf, "You position yourself to stealthily place $p against $N's %s.", arg);
		act(buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
		ch->delay = 10;
		ch->delay_type = DEL_POINTBLANK;
		ch->delay_ch = tch;
		ch->delay_info1 = 2;
	}
	else if (GET_POS(tch) <= POSITION_SITTING)
	{
		sprintf(buf, "You position yourself to place $p against $N's %s.", arg);
		act(buf,false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
		sprintf(buf, "$n prepares to place $p against $N's %s.", arg);
		act(buf,false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
		sprintf(buf, "$n prepares to place $p against your %s.", arg);
		act(buf,false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
		ch->delay = 10;
		ch->delay_type = DEL_POINTBLANK;
		ch->delay_ch = tch;
		ch->delay_info1 = 1;
	}
	else if (GET_POS(tch) >= POSITION_FIGHTING)
	{
		if (ch->fighting != tch)
		{
			act("You'll need to engage $N in combat for a chance to place $p against $M.", false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			sprintf(buf, "You attempt place $p against $N's %s.", arg);
			act(buf, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
			sprintf(buf, "$n attempts to place $p against your %s.", arg);
			act(buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n attempts to place $p against $N's %s.", arg);
			act(buf, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
			ch->delay = 10;
			ch->delay_type = DEL_POINTBLANK;
			ch->delay_ch = tch;
		}
	}

	ch->delay_who = str_dup(arg);
	ch->delay_info1 = bodypart;
	ch->aiming_at = tch;
	ch->aim = 0;
	add_targeted(tch, ch);
	ch->combat_block = 3;
	tch->combat_block = 3;
}

void
	delayed_pointblank (CHAR_DATA * ch)
{
	OBJ_DATA *obj = NULL;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg[MAX_STRING_LENGTH] = { '\0' };

	if (!((obj = get_equip (ch, WEAR_BOTH)) || (obj = get_equip (ch, WEAR_PRIM)) || (obj = get_equip (ch, WEAR_SEC))) || (GET_ITEM_TYPE(obj) != ITEM_FIREARM))
	{
		send_to_char("You need to be wielding some type of firearm to do that.\n", ch);
		return;
	}

	if (!ch->delay_ch || !(tch = ch->delay_ch) || !(is_in_room(ch, tch)))
	{
		send_to_char("Your target is no longer here.\n", ch);
		return;
	}

	if (*ch->delay_who)
	{
		sprintf(arg, "%s", ch->delay_who);
	}
	else
	{
		sprintf(arg, "body");
	}

	if (tch == ch)
	{
		act("$p is now placed against your head.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		ch->aim = 20;
	}
	else if (get_affect (ch, MAGIC_HIDDEN) && ch->delay_info1 == 2)
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		sprintf(buf, "You emerge from hiding to place $p against $N's %s.", arg);
		act(buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
		sprintf(buf, "$n emerges from hiding to place $p against $N's %s.", arg);
		act(buf, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
		sprintf(buf, "$n emerge from hiding to place $p against your %s.", arg);
		act(buf, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
		ch->aim = 20;
	}
	else if (GET_POS(tch) <= POSITION_SITTING)
	{
		sprintf(buf, "You place $p against $N's %s.", arg);
		act(buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
		sprintf(buf, "$n places $p against $N's %s.", arg);
		act(buf, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
		sprintf(buf, "$n places $p against your %s.", arg);
		act(buf, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
		ch->aim = 20;
	}
	else if (GET_POS(tch) >= POSITION_FIGHTING)
	{
		if (ch->delay_info1 == 1)
		{
			act("$N is no longer in a vulnerable position: you will now need to engage $M in combat for a chance to place $p against $M.",
				false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else if (ch->fighting != tch)
		{
			act("You'll need to engage $N in combat for a chance to place $p against $M.", false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			sprintf(buf, "You place $p against $N's %s.", arg);
			act(buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n places $p against $N's %s.", arg);
			act(buf, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
			sprintf(buf, "$n places $p against your %s.", arg);
			act(buf, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
			ch->aim = 20;
		}
	}


	add_second_affect (SA_POINTSTRIKE, 90, ch, NULL, NULL, (ch->delay_info1 ? ch->delay_info1 : 0));

}

int broke_aim (CHAR_DATA *ch, int cmd)
{

	if (!ch || !ch->aiming_at)
		return 0;

	if (ch->aiming_at)
	{
		remove_targeted(ch->aiming_at, ch);
	}

	ch->aiming_at = NULL;
	ch->aim = 0;
	ch->delay_info1 = 0;

	if (get_second_affect(ch, SA_POINTSTRIKE, 0))
		remove_second_affect(get_second_affect(ch, SA_POINTSTRIKE, 0));
	if (ch->delay_who)
		mem_free(ch->delay_who);

	return 1;
}


void
	do_cover (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char arg[AVG_STRING_LENGTH] = { '\0' };
	char arg2[AVG_STRING_LENGTH] = { '\0' };
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *xaf = NULL;
	int dir = -1;
	int index = 0;
	int i = 0;
	int old_dir = -1;
	bool changed = false;
	bool hunker = false;
	bool existing_obj = false;
	CHAR_DATA *tch = NULL;
	int count = 0;

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if (!*argument)
	{
		if (((af = get_affect (ch, MAGIC_SIT_TABLE)) || (af = get_affect (ch, AFFECT_COVER))) && af->a.table.obj && GET_ITEM_TYPE(af->a.table.obj) == ITEM_COVER)
		{
			sprintf(arg, "%s area", OBJN(af->a.table.obj, ch));
		}
		else
		{
			sprintf(arg, "area");
		}
	}
	else
	{
		argument = one_argument (argument, arg);
	}

	for (index = 0; index < 7; index ++)
	{
		if (!strn_cmp (direct[index], arg, strlen (arg)))
			dir = index;
	}

	if (dir == -1)
	{
		if (*arg)
		{
			if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
			{
				sprintf (buf, "You don't see #2%s#0 here.\n", arg);
				send_to_char (buf, ch);
				return;
			}
			else if (!str_cmp(arg, "hunker"))
			{
				hunker = true;
			}
			else
			{
				dir = -1;
			}
		}
		else
		{
			send_to_char ("Usage: cover <direction>,\n       cover <object>, or\n\n       cover hunch, if already in medium or heavy cover\n", ch);
			return;
		}
	}

	if (obj)
	{

		argument = one_argument(argument, arg2);

		if (GET_ITEM_TYPE(obj) != ITEM_COVER)
		{
			act ("You cannot use $p as cover.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;

			if (((af = get_affect (tch, MAGIC_SIT_TABLE)) || (af = get_affect (tch, AFFECT_COVER))) && af->a.table.obj == obj)
				count++;
		}

		if (obj->o.container.table_max_sitting != 0 &&
			count >= obj->o.container.table_max_sitting)
		{
			act ("There is no available space at $p.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		// If we've not got static directions, we need to pick one, and add them one by one.
		if (obj->o.od.value[2] != 0)
		{
			for (index = 0; index < 7; index ++)
			{
				if (!strn_cmp (direct[index], arg2, strlen (arg2)))
					dir = index;
			}

			if (!IS_SET (obj->o.od.value[3], 1 << dir))
			{
				sprintf (buf, "$p doesn't provide cover from the %s.", direct[dir]);
				act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			for (xaf = ch->hour_affects; xaf; xaf = xaf->next)
			{
				if (xaf->type != AFFECT_COVER)
					continue;

				if (!xaf->a.cover.obj || xaf->a.cover.obj != obj)
					affect_remove(ch, xaf);

				if (xaf->a.cover.direction == dir)
				{
					sprintf (buf, "You are already taking cover the %s behind $p.", direct[dir]);
					act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}

				existing_obj = true;
			}
		}
		else
		{
			xaf = get_affect(ch, AFFECT_COVER);
			if (xaf && xaf->a.cover.obj == obj)
			{
				act ("But you are already taking cover from behind $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}
		}
	}
	else if (IS_SET (ch->room->room_flags, NOHIDE))
	{
		send_to_char ("The area itself offers no spots behind which you can take cover.\n", ch);
		return;
	}

	if (hunker)
	{
		if (!(af = get_affect(ch, AFFECT_COVER)))
		{
			send_to_char ("You need to be behind medium or heavy cover to properly hunch against it.\n", ch);
			return;
		}
		else if (af->a.cover.value == 0)
		{
			send_to_char ("You need to be behind medium or heavy cover to properly hunch against it.\n", ch);
			return;
		}
		else if (af->a.cover.value == 3)
		{
			send_to_char ("You're already hunching against in your cover.\n", ch);
			return;
		}

		for (xaf = ch->hour_affects; xaf; xaf = xaf->next)
		{
			if (xaf->type != AFFECT_COVER)
				continue;

			if (xaf->a.cover.value > 0)
				xaf->a.cover.value = 3;
		}

		if (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, ch->room->contents))
		{
			act ("You hunch down against $p.", false, ch, af->a.cover.obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n hunches down against $p.", false, ch, af->a.cover.obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("You hunch down your cover.", false, ch, af->a.cover.obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n hunches down against $s cover.", false, ch, af->a.cover.obj, 0, TO_ROOM | _ACT_FORMAT);
		}

		return;
	}

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type != AFFECT_COVER)
			continue;

		if (af->a.cover.direction == dir)
			continue;

		old_dir = af->a.cover.direction;
		i++;
	}

	if (i >= 1)
	{
		if (obj && obj->o.od.value[2])
		{
			if (i >= obj->o.od.value[2])
			{
				changed = true;
				remove_cover(ch, old_dir);
			}
		}
		else if (IS_SET(ch->room->room_flags, HIGH_COVER))
		{
			if (i >= 5)
			{
				changed = true;
				remove_cover(ch, old_dir);
			}
		}
		else if (IS_SET(ch->room->room_flags, MED_COVER))
		{
			if (i >= 3)
			{
				changed = true;
				remove_cover(ch, old_dir);
			}
		}
		else
		{
			changed = true;
			remove_cover(ch, old_dir);
		}
	}

	if (obj)
	{
		if (obj->o.od.value[2] == 0)
		{
			sprintf (buf, "You start taking cover from the %s behind $p", direct[dir]);
			sprintf (buf2, "$n starts taking cover from the %s behind $p", direct[dir]);
			if (changed)
			{
				sprintf (buf + strlen(buf), ", abandoning your cover from the %s", direct[old_dir]);
				sprintf (buf2 + strlen(buf2), ", abandoning #5%s#0 cover from the %s", HSHR(ch), direct[old_dir]);
			}
		}
		else
		{
			remove_cover(ch, -2);
			if (xaf && is_obj_in_list (xaf->a.cover.obj, ch->room->contents))
			{
				sprintf (buf + strlen(buf), ", abandoning your cover from behind #2%s#0", obj_short_desc(xaf->a.cover.obj));
				sprintf (buf2 + strlen(buf2), ", abandoning #5%s#0 cover behind #2%s#0", HSHR(ch), obj_short_desc(xaf->a.cover.obj));
			}

			sprintf (buf, "You start taking cover behind $p");
			sprintf (buf2, "$n starts taking cover behind $p");
		}

		ch->delay_info1 = dir;
		ch->delay_info2 = obj->coldload_id;
	}
	else
	{
		sprintf (buf, "You start taking cover from the %s", direct[dir]);
		sprintf (buf2, "$n starts taking cover from the %s", direct[dir]);
		ch->delay_info1 = dir;
		ch->delay_info2 = 0;

		xaf = get_affect(ch, AFFECT_COVER);
		if (xaf && xaf->a.cover.obj)
		{
			remove_cover(ch, -2);
			if (is_obj_in_list (xaf->a.cover.obj, ch->room->contents))
			{
				sprintf (buf + strlen(buf), ", abandoning #2%s#0", obj_short_desc(xaf->a.cover.obj));
				sprintf (buf2 + strlen(buf2), ", abandoning #2%s#0", obj_short_desc(xaf->a.cover.obj));
			}
		}
		else if (changed)
		{
			sprintf (buf + strlen(buf), ", abandoning your cover from the %s", direct[old_dir]);
			sprintf (buf2 + strlen(buf2), ", abandoning #5%s#0 cover from the %s", HSHR(ch), direct[old_dir]);
		}
	}

	strcat(buf, ".");
	strcat(buf2, ".");

	if (obj)
	{
		act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		act(buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
	}
	else
	{
		act(buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act(buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}

	if (GET_POS (ch) != SIT && GET_POS (ch) != SLEEP &&
		(af = get_affect (ch, MAGIC_SIT_TABLE)))
		affect_remove (ch, af);

	ch->delay_type = DEL_COVER;
	if (dir == 6)
		ch->delay = 2;
	else
		ch->delay = 4;
}

void
	delayed_cover (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af = NULL;
	CHAR_DATA *tch = NULL;

	int count = 0;
	int value = 0;
	int use = 0;
	int dir = 0;

	dir = ch->delay_info1;

	if (IS_SET (ch->room->room_flags, NOHIDE))
	{
		send_to_char ("This room offers no hiding spots.\n", ch);
		return;
	}
	else if (IS_SET(ch->room->room_flags, HIGH_COVER))
		value = 2;
	else if (IS_SET(ch->room->room_flags, MED_COVER))
		value = 1;
	else
		value = 0;

	if ((obj = get_obj_in_list_id (ch->delay_info2, ch->room->contents)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;

			if (((af = get_affect (tch, MAGIC_SIT_TABLE)) || (af = get_affect (tch, AFFECT_COVER))) && af->a.table.obj == obj)
			{
				count++;
			}
		}


		if (obj->o.container.table_max_sitting != 0 &&
			count >= obj->o.container.table_max_sitting)
		{
			act ("There is no available space at $p.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		send_to_char (buf, ch);
		GET_POS (ch) = SIT;
		value = obj->o.od.value[1];

		if (obj->o.od.value[2] == 0)
		{
			sprintf (buf, "You take cover from the %s behind #2%s#0.\n", direct[dir], obj_short_desc (obj));
			CREATE (af, AFFECTED_TYPE, 1);
			af->type = AFFECT_COVER;
			af->a.affect.uu1 = -1;
			af->a.affect.uu2 = value;
			af->a.affect.uu3 = use;
			af->a.affect.uu4 = dir;
			af->a.affect.uu5 = 0;
			af->a.affect.uu6 = 0;
			af->a.cover.obj = obj;

			affect_to_char (ch, af);
		}
		else
		{

			remove_cover(ch, -2);
			sprintf (buf, "You take cover from behind #2%s#0.\n", obj_short_desc (obj));

			for (int i = 0; i < 7; i++)
			{
				if (IS_SET (obj->o.od.value[3], 1 << i))
				{
					add_affect (ch, AFFECT_COVER, -1, value, use, i, 0, 0);
					af = get_affect (ch, AFFECT_COVER);
					af->a.cover.obj = obj;
				}

			}
		}
	}
	else
	{
		remove_cover(ch, dir);
		sprintf (buf, "You take cover from the %s.\n", direct[dir]);
		send_to_char (buf, ch);
		GET_POS (ch) = SIT;
		add_affect (ch, AFFECT_COVER, -1, value, use, dir, 0, 0);
	}
	return;
}

/* Type -2 is remove all covers
*  type 0~6 removes cover from that direction
*/
void
	remove_cover (CHAR_DATA *ch, int type)
{
	AFFECTED_TYPE *af = NULL;

	if (type == -2)
	{
		while ((af = get_affect(ch, AFFECT_COVER)))
			remove_affect_type (ch, AFFECT_COVER);
	}
	else
	{
		for (af = ch->hour_affects; af; af = af->next)
		{
			if (af->type != AFFECT_COVER)
				continue;

			if (af->a.cover.direction == type)
			{
				affect_remove (ch, af);
				break;
			}
		}
	}
	return;
}

int
	under_cover (CHAR_DATA *ch)
{
	AFFECTED_TYPE *af = NULL;
	int i = 0;

	for (af = ch->hour_affects; af; af = af->next)
		if (af->type == AFFECT_COVER)
			i++;

	return i;
}

// A simple function that counts the number of bullets is either a clip or a gun.

int
	count_bullets (OBJ_DATA *obj)
{
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *robj = NULL;
	int i = 0;

	if (GET_ITEM_TYPE(obj) == ITEM_CLIP)
	{
		for (robj = obj->contains, i = 0; robj; robj = robj->next_content)
			i += robj->count;

		return i;
	}
	else if (GET_ITEM_TYPE(obj) == ITEM_FIREARM)
	{
		if (IS_DIRECT(obj))
		{
			if (obj->contains)
			{
				for (tobj = obj->contains, i = 0; tobj; tobj = tobj->next_content)
				{
					if (GET_ITEM_TYPE(tobj) == ITEM_ROUND)
						i += tobj->count;
				}

				return i;
			}
			else
			{
				return 0;
			}
		}
		else if (IS_SLING(obj))
		{
			if (obj->contains)
				return 1;
			else
				return 0;
		}
		else
		{
			if ((tobj = obj->contains) && GET_ITEM_TYPE(tobj) == ITEM_CLIP)
			{
				for (robj = tobj->contains, i = 0; robj; robj = robj->next_content)
					i += robj->count;

				return i;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}

// A simple function that counts the number of bullets is either a clip or a gun, taking in to account
// spent casings that might be in a revolver, say.

int
	count_all_bullets (OBJ_DATA *obj)
{
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *robj = NULL;
	int i = 0;

	if (GET_ITEM_TYPE(obj) == ITEM_CLIP)
	{
		for (robj = obj->contains, i = 0; robj; robj = robj->next_content)
			i += robj->count;

		return i;
	}
	else if (GET_ITEM_TYPE(obj) == ITEM_FIREARM)
	{
		if (IS_DIRECT(obj))
		{
			if (obj->contains)
			{
				for (tobj = obj->contains, i = 0; tobj; tobj = tobj->next_content)
				{
					if (GET_ITEM_TYPE(tobj) == ITEM_ROUND || GET_ITEM_TYPE(tobj) == ITEM_CASE)
						i += tobj->count;
				}

				return i;
			}
			else
			{
				return 0;
			}
		}
		else if (IS_SLING(obj))
		{
			if (obj->contains)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if ((tobj = obj->contains) && GET_ITEM_TYPE(tobj) == ITEM_CLIP)
			{
				for (robj = tobj->contains, i = 0; robj; robj = robj->next_content)
					i += robj->count;

				return i;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}

OBJ_DATA *
	draw_bullet_from_belt (CHAR_DATA * ch, char *bullet, OBJ_DATA * ptrBelt, int *count, int caliber, int size, int *error_msg)
{
	OBJ_DATA *ptrBullet = NULL;

	if (ptrBelt && GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER)
	{
		for (ptrBullet = ptrBelt->contains; ptrBullet; ptrBullet = ptrBullet->next_content)
		{

			if (GET_ITEM_TYPE (ptrBullet) != ITEM_ROUND)
				continue;

			if (bullet && *bullet && !isname (bullet, ptrBullet->name))
				continue;

			if (ptrBullet->o.bullet.caliber != caliber)
			{
				*error_msg = 2;
				continue;
			}

			if (size && ptrBullet->o.bullet.size != size)
			{
				*error_msg = 1;
				continue;
			}

			ch->delay_info2 = ptrBullet->nVirtual;
			ch->delay_info1 = ptrBelt->nVirtual;

			*count = ptrBullet->count;

			error_msg = 0;
			return ptrBullet;
		}
	}
	return NULL;
}

OBJ_DATA *
	draw_clip_from_belt (CHAR_DATA * ch, char *clip, OBJ_DATA * ptrBelt, int amount, int caliber, int size, int *error_msg)
{
	OBJ_DATA *ptrClip = NULL;

	if (ptrBelt && GET_ITEM_TYPE (ptrBelt) == ITEM_AMMO_BELT)
	{
		for (ptrClip = ptrBelt->contains; ptrClip; ptrClip = ptrClip->next_content)
		{

			if (GET_ITEM_TYPE (ptrClip) != ITEM_CLIP)
				continue;

			if (clip && *clip && !isname (clip, ptrClip->name))
				continue;

			if (ptrClip->o.clip.caliber != caliber)
			{
				*error_msg = 2;
				continue;
			}

			if (size && ptrClip->o.clip.size != size)
			{
				*error_msg = 1;
				continue;
			}

			if (!ptrClip->contains || !ptrClip->o.clip.amount || ptrClip->o.clip.amount == 0 || count_bullets(ptrClip) == 0)
			{
				continue;
			}

			if (amount == -1)
			{
				if (ptrClip->o.clip.amount != ptrClip->o.clip.max)
					continue;
			}
			else if (count_bullets(ptrClip) < amount)
				continue;

			ch->delay_info2 = ptrClip->nVirtual;
			ch->delay_info1 = ptrBelt->nVirtual;

			*error_msg = 0;
			return ptrClip;
		}
	}
	return NULL;
}

/* load [ <missile> ] [ [ from ] [ <belt> ] ] */
void
	do_load_clip (CHAR_DATA * ch, char *argument, int cmd)
{
	// 1. are we wielding an empty clip?
	// 2. do we have appropriate ammo?
	// 3. is the caliber of the clip equal to the caliber of the ammo?

	short int nArgC = 0;
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrOffHand = NULL;
	OBJ_DATA *ptrBullet = NULL;
	OBJ_DATA *ptrBelt = NULL;
	char strBullet[AVG_STRING_LENGTH] = "";
	char strFrom[AVG_STRING_LENGTH] = "";
	char strBelt[AVG_STRING_LENGTH] = "";
	char strBuf[AVG_STRING_LENGTH * 4] = "";
	char strBuf2[AVG_STRING_LENGTH * 4] = "";
	char arg[AVG_STRING_LENGTH] = "";
	int count = 0;
	int amount = 0;
	int error_msg = 0;

	if (!(((ptrClip = ch->right_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)
		|| ((ptrClip = ch->left_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)))
	{
		send_to_char ("You must first be holding a firearm magazine.\n", ch);
		return;
	}

	// bBolts = (ptrClip->o.weapon.use_skill == SKILL_CROSSBOW);
	ptrOffHand = (ptrClip == ch->right_hand) ? ch->left_hand : ch->right_hand;

	if (isdigit(*argument))
	{
		argument = one_argument(argument, arg);
		if ((amount = atoi(arg)) < 1)
		{
			send_to_char("You must specify a positive number of how many rounds you wish to load.\n", ch);
			return;
		}
	}

	if (count_bullets(ptrClip) == ptrClip->o.clip.max)
	{
		act("But $p is already fully loaded.", false, ch, ptrClip, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	nArgC = (argument && *argument) ? sscanf (argument, "%s %s %s", strBullet, strFrom, strBelt) : 0;

	send_to_gods(argument);

	if (nArgC == 0)
	{
		/* no arguments specified check off-hand first ptrBow*/
		if (ptrOffHand)
		{
			if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND)
			{
				ptrBullet = ptrOffHand;
			}
			else
			{
				send_to_char("Your off-hand may contain rounds, otherwise it needs to be empty.\n", ch);
				return;
			}
		}
		// nothing in our off-hand, cycle through our inventory till we find a bandolier with
		// the right sort of bullets.
		else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, NULL)))
		{
			for (ptrBelt = ch->equip; ptrBelt; ptrBelt = ptrBelt->next_content)
			{
				if (GET_ITEM_TYPE(ptrBelt) == ITEM_BANDOLIER)
				{
					if ((ptrBullet = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg)))
					{
						break;
					}
				}
			}
		}
		/* no Belt either */
		else
		{
			send_to_char ("You don't seem to be holding rounds or wearing a bandolier.\n", ch);
			return;
		}
	}
	else if (nArgC == 1)
	{
		/* may be "load <missile>" or "load <Belt>" or "load <clip>" */
		/* check first for arrow in hand */
		if (ptrOffHand)
		{
			if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND
				&& isname (strBullet, ptrOffHand->name))
			{
				ptrBullet = ptrOffHand;
			}
			else
			{
				send_to_char("Your off-hand may contain rounds, otherwise it needs to be empty.\n", ch);
				return;
			}
		}
		/* nothing there, check for a Belt first, then an arrow */
		else
		{
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strBullet)))
			{
				ptrBullet = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg);
			}
			else if (isname(strBullet, ptrClip->name))
			{
				if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, NULL)))
				{
					ptrBullet = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg);
				}
				else
				{
					send_to_char ("Load what? Or from what?\n", ch);
					return;
				}
			}
			else
			{
				if ((ptrBelt = get_eq_by_type_and_name(ch, ITEM_BANDOLIER, NULL)))
				{
					ptrBullet = draw_bullet_from_belt (ch, strBullet, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg);
				}
				else
				{
					send_to_char ("Load what? Or from what?\n", ch);
					return;
				}
			}
		}
	}
	else if (nArgC == 2 && !strcmp (strBullet, "from"))
	{
		/* must be "load from <quiver>" */
		if (ptrOffHand)
		{
			if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND
				&& isname (strBullet, ptrOffHand->name))
			{
				ptrBullet = ptrOffHand;
			}
			else
			{
				send_to_char("Your off-hand may contain rounds, otherwise it needs to be empty.\n", ch);
				return;
			}
		}
		else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strFrom)))
		{
			ptrBullet = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg);
		}
		else
		{
			send_to_char ("Load from what3?\n", ch);
			return;
		}
	}
	else if (nArgC == 3 && !strcmp (strFrom, "from"))
	{
		if (ptrOffHand)
		{
			if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND
				&& isname (strBullet, ptrOffHand->name))
			{
				ptrBullet = ptrOffHand;
			}
			else
			{
				send_to_char("Your off-hand may contain rounds, otherwise it needs to be empty.\n", ch);
				return;
			}
		}
		/* must be "load <missile> from <quiv>" */
		else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strBelt)))
		{
			ptrBullet = draw_bullet_from_belt (ch, strBullet, ptrBelt, &count, ptrClip->o.clip.caliber, 0, &error_msg);
		}
		else
		{
			send_to_char ("Load from what?\n", ch);
			return;
		}
	}
	else
	{
		do_help (ch, "combat load", 0);
		return;
	}

	/* Was there a missle? */
	if (!ptrBullet)
	{
		send_to_char ("You don't seem to be holding rounds or wearing a bandolier containing rounds.\n", ch);
		return;
	}

	if (ptrBullet->o.bullet.caliber != ptrClip->o.clip.caliber)
	{
		send_to_char ("Those kind of rounds won't fit in to that kind of magazine.\n", ch);
		return;
	}

	if (ptrBullet->o.bullet.size != ptrClip->o.clip.size)
	{
		send_to_char ("Those kind of rounds won't fit in to that kind of magazine.\n", ch);
		return;
	}

	count = ptrBullet->count;

	// If we've specified an amount that is less than the amount of bullets in that stack, reduce the count to that amount
	if (amount > 0 && count >= amount)
		count = amount;
	// Otherwise, if we've set no amount, load us back up to full.
	else if (amount == 0 && (ptrClip->o.clip.amount + count) > ptrClip->o.clip.max)
	{
		count = ptrClip->o.clip.max - ptrClip->o.clip.amount;
		amount = count;
	}
	else if (amount > count)
	{
		amount = count;
	}

	if ((ptrClip->o.clip.amount + count) > ptrClip->o.clip.max)
	{
		send_to_char ("The magazine can't handle that many rounds.\n", ch);
		return;
	}

	if (ptrClip->count > 1)
	{
		send_to_char ("You can only load one magazine at a time.\n", ch);
		return;
	}

	if (ptrBelt)
	{
		if (amount)
		{
			sprintf (strBuf, "You begin loading %d of #2%s#0 from #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrBullet), obj_short_desc (ptrBelt), obj_short_desc (ptrClip));
			sprintf (strBuf2, "$n begins loading %d of #2%s#0 from #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrBullet), obj_short_desc (ptrBelt), obj_short_desc (ptrClip));
		}
		else
		{
			sprintf (strBuf, "You begin loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrBullet), obj_short_desc (ptrBelt), obj_short_desc (ptrClip));
			sprintf (strBuf2, "$n begins loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrBullet), obj_short_desc (ptrBelt), obj_short_desc (ptrClip));
		}
	}
	else
	{
		if (amount)
		{
			sprintf (strBuf, "You begin loading %d of #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrBullet), obj_short_desc (ptrClip));
			sprintf (strBuf2, "$n begins loading %d of #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrBullet), obj_short_desc (ptrClip));
		}
		else
		{
			sprintf (strBuf, "You begin loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrBullet), obj_short_desc (ptrClip));
			sprintf (strBuf2, "$n begins loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrBullet), obj_short_desc (ptrClip));
		}
	}


	act (strBuf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	act (strBuf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);


	ch->delay_info1 = (ptrBelt) ? ptrBelt->coldload_id : -1;
	ch->delay_info2 = ptrBullet->coldload_id;
	ch->delay_info3 = count;
	ch->delay_info3 = MIN ((ptrClip->o.clip.amount + count), count);
	//ch->delay_info3 = MAX (1, count);
	ch->delay = 1 + count;
	ch->delay = MIN ((ptrClip->o.clip.amount + count), count);
	ch->delay = (ch->delay / 2);
	ch->delay = MAX (ch->delay, 1);

	ch->delay_type = DEL_LOAD_CLIP;
	return;
}


void
	delayed_load_clip (CHAR_DATA * ch)
{
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrBullet = NULL;
	OBJ_DATA *ptrBelt = NULL;
	char buf[MAX_STRING_LENGTH];
	int i = 0;
	int count = 0;

	if (!(((ptrClip = ch->right_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)
		|| ((ptrClip = ch->left_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)))
	{
		send_to_char ("You must be holding a firearm magazine.\n", ch);
		return;
	}

	if (!(count = ch->delay_info3))
	{
		send_to_char ("You must have a round somewhere on you.\n", ch);
		return;
	}

	if (ch->delay_info1 < 0)
	{
		if (ch->right_hand && ch->right_hand->coldload_id == ch->delay_info2 && GET_ITEM_TYPE(ch->right_hand) == ITEM_ROUND)
		{
			ptrBullet = ch->right_hand;
		}
		else if (ch->left_hand && ch->left_hand->coldload_id == ch->delay_info2 && GET_ITEM_TYPE(ch->left_hand) == ITEM_ROUND)
		{
			ptrBullet = ch->left_hand;
		}
		else
		{
			send_to_char ("You must be holding a firearm round.\n", ch);
			return;
		}
	}
	else
	{
		if (ch->left_hand && ch->right_hand)
		{
			send_to_char ("Having taken another object in hand, you cease loading your magazine.\n",  ch);
			return;
		}
		for (i = 0; i < MAX_WEAR; i++)
		{
			if (!(ptrBelt = get_equip (ch, i)))
			{
				continue;
			}
			if (ptrBelt->coldload_id != ch->delay_info1 || !get_obj_in_list_id (ch->delay_info2, ptrBelt->contains))
			{
				continue;
			}

			if (GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER)
			{
				for (ptrBullet = ptrBelt->contains; ptrBullet; ptrBullet = ptrBullet->next_content)
				{
					if (GET_ITEM_TYPE (ptrBullet) == ITEM_ROUND && ptrBullet->coldload_id == ch->delay_info2)
					{
						break;
					}
				}
			}

			if (ptrBullet)
			{
				break;
			}
		}
	}
	if (!ptrBullet)
	{
		send_to_char("Having lost your ammunition, you cease loading your magazine.\n", ch);
		return;
	}


	if (count < ptrBullet->count)
	{
		if ((ptrClip->o.clip.amount + count) == ptrClip->o.clip.max)
			sprintf (buf, "You finish loading #2%s#0 with %d of #2%s#0, completely filling the magazine.\n", obj_short_desc(ptrClip), count, obj_short_desc (ptrBullet));
		else
			sprintf (buf, "You finish loading #2%s#0 with %d of #2%s#0.\n", obj_short_desc(ptrClip), count, obj_short_desc (ptrBullet));
	}
	else
	{
		if ((ptrClip->o.clip.amount + count) == ptrClip->o.clip.max)
			sprintf (buf, "You finish loading #2%s#0 with #2%s#0, completely filling the magazine.\n", obj_short_desc(ptrClip), obj_short_desc (ptrBullet));
		else
			sprintf (buf, "You finish loading #2%s#0 with #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc (ptrBullet));
	}

	send_to_char (buf, ch);

	if (count < ptrBullet->count)
	{
		sprintf (buf, "$n finishes loading #2%s#0 with %d of #2%s#0.\n", obj_short_desc(ptrClip), count, obj_short_desc (ptrBullet));
	}
	else
	{
		sprintf (buf, "$n finishes loading #2%s#0 with #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc (ptrBullet));
	}

	act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

	if (ch->delay_info1 >= 0)
	{
		obj_from_obj (&ptrBullet, count);
	}
	else
	{
		obj_from_char (&ptrBullet, count);
	}

	ptrClip->o.clip.amount += ptrBullet->count;
	obj_to_obj(ptrBullet, ptrClip);

	ch->delay = 0;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;
	ch->delay_type = 0;
}


void
	do_unload_clip (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buffer[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA *ptrClip = NULL;

	if (!(((ptrClip = ch->right_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)
		|| ((ptrClip = ch->left_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)))
	{
		send_to_char ("You must first be holding a firearm magazine.\n", ch);
		return;
	}

	if (!ptrClip->contains)
	{
		sprintf(buf, "%s#0 isn't loaded.\n", obj_short_desc(ptrClip));
		*buf = toupper(*buf);
		sprintf(buffer, "#2%s", buf);
		send_to_char(buffer, ch);
		return;
	}

	act("You start unloading rounds from $p.", false, ch, ptrClip, 0, TO_CHAR | _ACT_FORMAT);
	act("$n starts unloading rounds from $p.", false, ch, ptrClip, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

	ch->delay_info1 = ptrClip->coldload_id;
	ch->delay_type = DEL_UNLOAD_CLIP;
	ch->delay = count_bullets(ptrClip);
	ch->delay = ch->delay / 2;
	ch->delay = MAX(1, ch->delay);
}

void
	delayed_unload_clip (CHAR_DATA * ch)
{
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrBullet = NULL;
	OBJ_DATA *ptrNewBullet = NULL;
	OBJ_DATA *ptrBelt = NULL;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	bool first = true;
	bool room = true;
	bool redone = false;
	bool on_ground = false;
	int i = 0;
	char *error;

	if (!(((ptrClip = ch->right_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)
		|| ((ptrClip = ch->left_hand) && GET_ITEM_TYPE (ptrClip) == ITEM_CLIP)))
	{
		send_to_char ("You're not longer holding a firearm magazine.\n", ch);
		return;
	}

	if (ptrClip->coldload_id != ch->delay_info1)
	{
		send_to_char ("You're no longer holding the same firearm magazine.\n", ch);
		return;
	}

	if (!(ptrBullet = ptrClip->contains))
	{
		send_to_char ("The magazine is not loaded.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(ptrBullet) != ITEM_ROUND)
	{
		send_to_char ("The magazine is not loaded.\n", ch);
		return;
	}

	for (i = 0; i < MAX_WEAR; i++)
	{
		if (!(ptrBelt = get_equip (ch, i)))
			continue;

		if (GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER
			&& can_obj_to_container (ptrBullet, ptrBelt, &error, 1))
			break;
	}

	if (!ptrBelt)
	{
		sprintf (buf, "#2%s#0, placing", obj_short_desc (ptrClip));

		while (ptrClip->contains)
		{
			ptrNewBullet = ptrClip->contains;

			if (ptrNewBullet == ptrClip->contains || redone)
			{
				sprintf (buf + strlen(buf), " #2%s#0", obj_short_desc(ptrNewBullet));
				redone = false;
			}
			else if (!ptrClip->contains)
				sprintf (buf + strlen(buf), " and #2%s#0", obj_short_desc(ptrNewBullet));
			else
				sprintf (buf + strlen(buf), ", #2%s#0", obj_short_desc(ptrNewBullet));

			obj_from_obj (&ptrNewBullet, ptrNewBullet->count);

			if (!ch->right_hand || !ch->left_hand)
			{
				obj_to_char(ptrNewBullet, ch);
				sprintf (buf + strlen(buf), " in your hand");

				if (ptrClip->contains)
					sprintf (buf + strlen(buf), " and");
				else
					sprintf (buf + strlen(buf), ".");

				if (!on_ground)
					redone = true;
			}
			else
			{
				on_ground = true;
				obj_to_room(ptrNewBullet, ch->in_room);
			}
		}
		if (on_ground)
			sprintf (buf + strlen(buf), " on the ground.");

	}
	else
	{
		sprintf (buf, "#2%s#0, placing", obj_short_desc (ptrClip));

		while (ptrClip->contains)
		{
			ptrNewBullet = ptrClip->contains;

			obj_from_obj (&ptrNewBullet, ptrNewBullet->count);

			if (can_obj_to_container (ptrNewBullet, ptrBelt, &error, ptrNewBullet->count))
			{
				if (first)
				{
					first = false;
					sprintf (buf + strlen(buf), " #2%s#0", obj_short_desc(ptrNewBullet));
				}
				else if (!ptrClip->contains || !can_obj_to_container (ptrClip->contains, ptrBelt, &error, ptrClip->contains->count))
					sprintf (buf + strlen(buf), " and #2%s#0", obj_short_desc(ptrNewBullet));
				else
					sprintf (buf + strlen(buf), ", #2%s#0", obj_short_desc(ptrNewBullet));

				obj_to_obj (ptrNewBullet, ptrBelt);
			}
			else
			{
				if (room)
				{
					sprintf (buf + strlen(buf), " in to #2%s#0 until there is no more room, so the following go to the ground: ", obj_short_desc(ptrBelt));
					sprintf (buf + strlen(buf), " #2%s#0", obj_short_desc(ptrNewBullet));
					room = false;
				}
				else if (!ptrClip->contains)
					sprintf (buf + strlen(buf), " and #2%s#0", obj_short_desc(ptrNewBullet));
				else
					sprintf (buf + strlen(buf), ", #2%s#0", obj_short_desc(ptrNewBullet));

				obj_to_room(ptrNewBullet, ch->in_room);
			}
		}
		if (room)
			sprintf(buf + strlen(buf), " in to #2%s#0.", obj_short_desc(ptrBelt));
		else
			sprintf(buf + strlen(buf), ".");
	}

	ptrClip->o.clip.amount = 0;


	sprintf(buf2, "You unload %s", buf);
	sprintf(buf3, "$n unloads %s", buf);

	act(buf2, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	act(buf3, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
}


void
	do_load_firearm (CHAR_DATA * ch, char *argument, int cmd)
{
	// 1. are we wielding an empty clip?
	// 2. do we have appropriate ammo?
	// 3. is the caliber of the clip equal to the caliber of the ammo?

	short int nArgC = 0;
	OBJ_DATA *ptrRound = NULL;
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrOffHand = NULL;
	OBJ_DATA *ptrBelt = NULL;
	OBJ_DATA *ptrFirearm = NULL;
	char strClip[AVG_STRING_LENGTH] = "";
	char strFrom[AVG_STRING_LENGTH] = "";
	char strBelt[AVG_STRING_LENGTH] = "";
	char strBuf[AVG_STRING_LENGTH * 4] = "";
	char strBuf2[AVG_STRING_LENGTH * 4] = "";
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buffer[MAX_STRING_LENGTH] = {'\0'};
	char arg[AVG_STRING_LENGTH] = "";
	int amount = 0;
	int count = 0;
	int size = 0;
	int cap = 0;
	int error_msg = 0;

	if (!(((ptrFirearm = ch->right_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)
		|| ((ptrFirearm = ch->left_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)))
	{
		send_to_char ("You must first be holding a firearm.\n", ch);
		return;
	}

	if (IS_DIRECT(ptrFirearm))
	{
		if (IS_SET(ptrFirearm->o.firearm.bits, GUN_DIRECT_ONE))
			cap = 1;
		else if (IS_SET(ptrFirearm->o.firearm.bits, GUN_DIRECT_FIVE))
			cap = 5;
		else
			cap = 6;

		if (count_all_bullets(ptrFirearm) >= cap)
		{
			sprintf(buf, "%s#0 already has all of its chambers loaded.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}
	}
	else if (ptrFirearm->contains)
	{
		if (IS_SLING(ptrFirearm))
		{
			sprintf(buf, "%s#0 already has a BB in the pocket.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}
		else
		{
			sprintf(buf, "%s#0 is already loaded with a magazine.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}
	}

	// bBolts = (ptrClip->o.weapon.use_skill == SKILL_CROSSBOW);
	ptrOffHand = (ptrFirearm == ch->right_hand) ? ch->left_hand : ch->right_hand;

	//send_to_gods(argument);

	if (isdigit(*argument))
	{
		argument = one_argument(argument, arg);
		if ((amount = atoi(arg)) < 1)
		{
			send_to_char("You must specify a positive number for the minimum number of rounds you need in the magazine to load.\n", ch);
			return;
		}
	}
	else
	{
		one_argument(argument, arg);
		if (!str_cmp(arg, "full"))
			amount = -1;
	}

	if (IS_SLING(ptrFirearm))
		size = 3;
	else if (ptrFirearm->o.firearm.use_skill == SKILL_RIFLE)
		size = 2;
	else if (ptrFirearm->o.firearm.use_skill == SKILL_SMG || ptrFirearm->o.firearm.use_skill == SKILL_GUNNERY)
		size = 1;
	else
		size = 0;

	if (IS_DIRECT(ptrFirearm))
	{
		//send_to_gods(argument);
		cap = cap - count_all_bullets(ptrFirearm);

		if (amount > cap || !amount)
			amount = cap;

		if (cap <= 0)
		{
			act ("But $p is already fully loaded with rounds or spent casings.", false, ch, ptrFirearm, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		nArgC = (argument && *argument) ? sscanf (argument, "%s %s %s", strClip, strFrom, strBelt) : 0;

		// load or load "Full" or load "x"

		if (nArgC == 0)
		{
			/* no arguments specified check off-hand first ptrBow*/
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND)
				{
					ptrRound = ptrOffHand;
				}
				else
				{
					send_to_char("Your off-hand may contain some rounds, otherwise it needs to be empty.\n", ch);
					return;
				}
			}
			// nothing in our off-hand, cycle through our inventory till we find a bandolier with
			// the right sort of bullets.
			else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, NULL)))
			{
				for (ptrBelt = ch->equip; ptrBelt; ptrBelt = ptrBelt->next_content)
				{
					if (GET_ITEM_TYPE(ptrBelt) == ITEM_BANDOLIER)
					{
						if ((ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg)))
						{
							break;
						}
					}
				}
			}

			/* no Belt either */
			else
			{
				send_to_char ("You don't seem to be either holding rounds or wearing a bandolier with rounds.\n", ch);
				return;
			}
		}
		else if (nArgC == 1)
		{
			/* may be "load <missile>" or "load <Belt>" */
			/* check first for arrow in hand */
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND
					&& isname (strClip, ptrOffHand->name))
				{
					ptrRound = ptrOffHand;
				}
				else
				{
					send_to_char ("Load what1?\n", ch);
					return;
				}
			}
			/* nothing there, check for a Belt first, then a clip */
			else
			{
				if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strClip)))
				{
					ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
				}
				else
				{
					if ((ptrBelt = get_eq_by_type_and_name(ch, ITEM_BANDOLIER, NULL)))
					{
						ptrRound = draw_bullet_from_belt (ch, strClip, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
					}
					else
					{
						send_to_char ("Load what? Or from what?\n", ch);
						return;
					}
				}
			}
		}
		else if (nArgC == 2 && !strcmp (strClip, "from"))
		{
			/* must be "load from <quiver>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strFrom)))
			{
				ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else if (nArgC == 3 && !strcmp (strFrom, "from"))
		{
			/* must be "load <missile> from <quiv>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strBelt)))
			{
				ptrRound = draw_bullet_from_belt (ch, strClip, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else
		{
			do_help (ch, "combat load", 0);
			return;
		}

		/* Was there a missle? */
		if (!ptrRound)
		{
			if (error_msg == 2)
			{
				sprintf(buffer, "The rounds you have on you are the wrong caliber for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else if (error_msg == 1)
			{
				sprintf(buffer, "The rounds you have on you are the wrong size for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else
			{
				sprintf(buffer, "You need either rounds in your hand, or be wearing bandolier containing rounds.\n");
			}
			send_to_char(buffer, ch);
			return;
		}

		if (ptrFirearm->o.firearm.caliber != ptrRound->o.bullet.caliber)
		{
			sprintf(buf, "%s#0 is not the right caliber for #2%s#0.\n", obj_short_desc(ptrRound), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		int i = 0;

		for (i = 12; i <= 19; i++)
		{
			if (IS_SET(ptrFirearm->o.firearm.bits, 1 << i))
				break;
		}

		if (i < 12 || i > 18)
		{
			sprintf(buf, "%s#0 is not sized to fit #2%s#0.\n", obj_short_desc(ptrRound), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		if (amount > ptrRound->count)
			amount = ptrRound->count;

		if (ptrBelt)
		{
			if (amount)
			{
				sprintf (strBuf, "You begin loading %d of #2%s#0 from #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
			}
			else
			{
				sprintf (strBuf, "You begin loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
			}
			sprintf (strBuf2, "$n begins loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
		}
		else
		{
			if (amount)
				sprintf (strBuf, "You begin loading %d of #2%s#0 in to #2%s#0.", amount, obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));
			else
				sprintf (strBuf, "You begin loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));

			sprintf (strBuf2, "$n begins loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));
		}

		act (strBuf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act (strBuf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

		ch->delay_info1 = (ptrBelt) ? ptrBelt->coldload_id : -1;
		ch->delay_info2 = ptrRound->coldload_id;
		ch->delay_info3 = amount;
		ch->delay = amount;
		if (ptrBelt)
			ch->delay += amount;
		ch->delay_type = DEL_LOAD_FIREARM;
	}
	else if (IS_SLING(ptrFirearm))
	{
		//send_to_gods(argument);
		cap = 1 - count_all_bullets(ptrFirearm);
		amount = 1;

		if (cap <= 0)
		{
			act ("But $p's pocket already contains a BB.", false, ch, ptrFirearm, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		nArgC = (argument && *argument) ? sscanf (argument, "%s %s %s", strClip, strFrom, strBelt) : 0;

		// load or load "Full" or load "x"

		if (nArgC == 0)
		{
			/* no arguments specified check off-hand first ptrBow*/
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND)
				{
					ptrRound = ptrOffHand;
				}
				else
				{
					send_to_char("Your off-hand may contain some BBs, otherwise it needs to be empty.\n", ch);
					return;
				}
			}
			// nothing in our off-hand, cycle through our inventory till we find a bandolier with
			// the right sort of bullets.
			else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, NULL)))
			{
				for (ptrBelt = ch->equip; ptrBelt; ptrBelt = ptrBelt->next_content)
				{
					if (GET_ITEM_TYPE(ptrBelt) == ITEM_BANDOLIER)
					{
						if ((ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg)))
						{
							break;
						}
					}
				}
			}
			/* no Belt either */
			else
			{
				send_to_char ("You don't seem to be either holding BBs or wearing a bandolier with BBs.\n", ch);
				return;
			}
		}
		else if (nArgC == 1)
		{
			/* may be "load <missile>" or "load <Belt>" */
			/* check first for arrow in hand */
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_ROUND
					&& isname (strClip, ptrOffHand->name))
				{
					ptrRound = ptrOffHand;
				}
				else
				{
					send_to_char ("Load what1?\n", ch);
					return;
				}
			}
			/* nothing there, check for a Belt first, then a clip */
			else
			{
				if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strClip)))
				{
					ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
				}
				else
				{
					if ((ptrBelt = get_eq_by_type_and_name(ch, ITEM_BANDOLIER, NULL)))
					{
						ptrRound = draw_bullet_from_belt (ch, strClip, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
					}
					else
					{
						send_to_char ("Load what? Or from what?\n", ch);
						return;
					}
				}
			}
		}
		else if (nArgC == 2 && !strcmp (strClip, "from"))
		{
			/* must be "load from <quiver>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strFrom)))
			{
				ptrRound = draw_bullet_from_belt (ch, NULL, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else if (nArgC == 3 && !strcmp (strFrom, "from"))
		{
			/* must be "load <missile> from <quiv>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_BANDOLIER, strBelt)))
			{
				ptrRound = draw_bullet_from_belt (ch, strClip, ptrBelt, &count, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else
		{
			do_help (ch, "combat load", 0);
			return;
		}

		/* Was there a missle? */
		if (!ptrRound)
		{
			if (error_msg == 2)
			{
				sprintf(buffer, "The BBs you have on you are the wrong caliber for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else if (error_msg == 1)
			{
				sprintf(buffer, "The BBs you have on you are the wrong size for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else
			{
				sprintf(buffer, "You need either BBs in your hand, or be wearing bandolier containing BBs.\n");
			}
			send_to_char(buffer, ch);
			return;
		}

		if (ptrFirearm->o.firearm.caliber != ptrRound->o.bullet.caliber)
		{
			sprintf(buf, "%s#0 is not the right caliber for #2%s#0.\n", obj_short_desc(ptrRound), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		if (amount > ptrRound->count)
			amount = ptrRound->count;

		if (ptrBelt)
		{
			if (amount)
			{
				sprintf (strBuf, "You slip %d of #2%s#0 from #2%s#0 in to the pocket of #2%s#0.", amount, obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
			}
			else
			{
				sprintf (strBuf, "You slip #2%s#0 from #2%s#0 in to the pocket of #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
			}
			sprintf (strBuf2, "$n slips #2%s#0 from #2%s#0 in to the pocket of #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));

			obj_from_obj (&ptrRound, 1);
		}
		else
		{
			if (amount)
				sprintf (strBuf, "You slip %d of #2%s#0 in to the pocket of #2%s#0.", amount, obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));
			else
				sprintf (strBuf, "You slip #2%s#0 in to the pocket of #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));

			sprintf (strBuf2, "$n slips #2%s#0 in to the pocket of #2%s#0.", obj_short_desc (ptrRound), obj_short_desc (ptrFirearm));

			obj_from_char (&ptrRound, 1);
		}

		act (strBuf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act (strBuf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

		obj_to_obj(ptrRound, ptrFirearm);
		return;
	}
	else
	{
		//send_to_gods(argument);

		nArgC = (argument && *argument) ? sscanf (argument, "%s %s %s", strClip, strFrom, strBelt) : 0;

		// load or load "Full" or load "x"

		if (nArgC == 0)
		{
			/* no arguments specified check off-hand first ptrBow*/
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_CLIP)
				{
					ptrClip = ptrOffHand;
				}
				else
				{
					send_to_char("Your off-hand may contain a magazine, otherwise it needs to be empty.\n", ch);
					return;
				}
			}
			/* nothing in our off-hand, load from the first Belt we find */
			else if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_AMMO_BELT, NULL)))
			{
				for (ptrBelt = ch->equip; ptrBelt; ptrBelt = ptrBelt->next_content)
				{
					if (GET_ITEM_TYPE(ptrBelt) == ITEM_AMMO_BELT)
					{
						if ((ptrClip = draw_clip_from_belt (ch, NULL, ptrBelt, amount, ptrFirearm->o.firearm.caliber, size, &error_msg)));
						{
							break;
						}
					}
				}
			}

			/* no Belt either */
			else
			{
				send_to_char ("You don't seem to be holding a magazine or wearing an ammo-belt with a loaded clip ready.\n", ch);
				return;
			}
		}
		else if (nArgC == 1)
		{
			/* may be "load <missile>" or "load <Belt>" */
			/* check first for arrow in hand */
			if (ptrOffHand)
			{
				if (GET_ITEM_TYPE (ptrOffHand) == ITEM_CLIP
					&& isname (strClip, ptrOffHand->name))
				{
					ptrClip = ptrOffHand;
				}
				else
				{
					send_to_char ("Load what1?\n", ch);
					return;
				}
			}
			/* nothing there, check for a Belt first, then a clip */
			else
			{
				if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_AMMO_BELT, strClip)))
				{
					ptrClip = draw_clip_from_belt (ch, NULL, ptrBelt, amount, ptrFirearm->o.firearm.caliber, size, &error_msg);
				}
				else
				{
					if ((ptrBelt = get_eq_by_type_and_name(ch, ITEM_AMMO_BELT, NULL)))
					{
						ptrClip = draw_clip_from_belt (ch, strClip, ptrBelt, amount, ptrFirearm->o.firearm.caliber, size, &error_msg);
					}
					else
					{
						send_to_char ("Load what? Or from what?\n", ch);
						return;
					}
				}
			}
		}
		else if (nArgC == 2 && !strcmp (strClip, "from"))
		{
			/* must be "load from <quiver>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_AMMO_BELT, strFrom)))
			{
				ptrClip = draw_clip_from_belt (ch, NULL, ptrBelt, amount, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else if (nArgC == 3 && !strcmp (strFrom, "from"))
		{
			/* must be "load <missile> from <quiv>" */
			if ((ptrBelt = get_eq_by_type_and_name (ch, ITEM_AMMO_BELT, strBelt)))
			{
				ptrClip = draw_clip_from_belt (ch, strClip, ptrBelt, amount, ptrFirearm->o.firearm.caliber, size, &error_msg);
			}
			else
			{
				send_to_char ("Load from what?\n", ch);
				return;
			}
		}
		else
		{
			do_help (ch, "combat load", 0);
			return;
		}

		/* Was there a missle? */
		if (!ptrClip)
		{
			if (error_msg == 2)
			{
				sprintf(buffer, "The magazine you have on you are the wrong caliber for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else if (error_msg == 1)
			{
				sprintf(buffer, "The magazine you have on you are the wrong size for #2%s#0.\n", obj_short_desc(ptrFirearm));
			}
			else
			{
				sprintf(buffer, "You need either a loaded magazine in your hand, or be wearing an ammo-belt containing a loaded magazine.\n");
			}
			send_to_char(buffer, ch);
			return;
		}

		if (ptrFirearm->o.firearm.caliber != ptrClip->o.clip.caliber)
		{
			sprintf(buf, "%s#0 is not the right caliber for #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		int i = 0;

		for (i = 12; i <= 19; i++)
		{
			if (IS_SET(ptrFirearm->o.firearm.bits, 1 << i))
				break;
		}

		if (i < 12 || i > 18)
		{
			sprintf(buf, "%s#0 is not sized to fit #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		if (i != ptrClip->o.clip.type)
		{
			sprintf(buf, "%s#0 is not sized to fit #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		if (!ptrClip->contains)
		{
			if (ptrClip->carried_by)
			{
				if (ptrClip == ch->right_hand || ptrClip == ch->left_hand)
				{
					sprintf(buf, "%s#0 in your %s hand does not contain any rounds.\n", obj_short_desc(ptrClip), (ptrClip == ch->right_hand ? "right" : "left"));
					*buf = toupper(*buf);
					sprintf(buffer, "#2%s", buf);
					send_to_char(buffer, ch);
				}
				else
				{
					send_to_char ("The magazine you are holding does not rounds.\n", ch);
				}
			}
			else if (ptrBelt)
			{
				sprintf(buf, "%s#0 in #2%s#0 does not contain any rounds.\n", obj_short_desc(ptrClip), obj_short_desc(ptrBelt));
				*buf = toupper(*buf);
				sprintf(buffer, "#2%s", buf);
				send_to_char(buffer, ch);
			}
			else
			{
				send_to_char ("That magazine does not contain any rounds.\n", ch);
			}
			return;
		}

		if (ptrBelt)
		{
			sprintf (strBuf, "You begin loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrClip), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
			sprintf (strBuf2, "$n begins loading #2%s#0 from #2%s#0 in to #2%s#0.", obj_short_desc (ptrClip), obj_short_desc (ptrBelt), obj_short_desc (ptrFirearm));
		}
		else
		{
			sprintf (strBuf, "You begin loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrClip), obj_short_desc (ptrFirearm));
			sprintf (strBuf2, "$n begins loading #2%s#0 in to #2%s#0.", obj_short_desc (ptrClip), obj_short_desc (ptrFirearm));
		}

		act (strBuf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act (strBuf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

		ch->delay_info1 = (ptrBelt) ? ptrBelt->coldload_id : -1;
		ch->delay_info2 = ptrClip->coldload_id;
		ch->delay = 2;
		if (ptrBelt)
			ch->delay += 2;
		ch->delay_type = DEL_LOAD_FIREARM;
	}


	return;
}

void
	delayed_load_firearm (CHAR_DATA * ch)
{
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrRound = NULL;
	OBJ_DATA *ptrFirearm = NULL;
	OBJ_DATA *ptrBelt = NULL;
	char buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	int i = 0;
	int count = 0;
	int cap = 0;
	int size = 0;

	if (!(((ptrFirearm = ch->right_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)
		|| ((ptrFirearm = ch->left_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)))
	{
		send_to_char ("You must be holding a firearm.\n", ch);
		return;
	}

	if (ptrFirearm->o.firearm.use_skill == SKILL_RIFLE)
		size = 2;
	else if (ptrFirearm->o.firearm.use_skill == SKILL_SMG || ptrFirearm->o.firearm.use_skill == SKILL_GUNNERY)
		size = 1;
	else
		size = 0;

	if (IS_DIRECT(ptrFirearm))
	{
		if (ch->delay_info1 < 0)
		{
			if (ch->right_hand && ch->right_hand->coldload_id == ch->delay_info2)
			{
				ptrRound = ch->right_hand;
			}
			else if (ch->left_hand && ch->left_hand->coldload_id == ch->delay_info2)
			{
				ptrRound = ch->left_hand;
			}
		}
		else
		{
			if (ch->left_hand && ch->right_hand)
			{
				send_to_char ("Having taken another object in hand, you cease loading your firearm.\n",  ch);
				return;
			}
			for (i = 0; i < MAX_WEAR; i++)
			{
				if (!(ptrBelt = get_equip (ch, i)))
				{
					continue;
				}
				if (ptrBelt->coldload_id != ch->delay_info1 || !get_obj_in_list_id (ch->delay_info2, ptrBelt->contains))
				{
					continue;
				}

				if (GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER)
				{
					for (ptrRound = ptrBelt->contains; ptrRound; ptrRound = ptrRound->next_content)
					{
						if (GET_ITEM_TYPE (ptrRound) == ITEM_ROUND && ptrRound->coldload_id == ch->delay_info2)
						{
							break;
						}
					}
				}

				if (ptrRound)
				{
					break;
				}
			}
		}
		if (!ptrRound)
		{
			send_to_char("Having lost your rounds, you cease loading your firearm.\n", ch);
			return;
		}

		if (IS_SET(ptrFirearm->o.firearm.bits, GUN_DIRECT_ONE))
			cap = 1;
		else if (IS_SET(ptrFirearm->o.firearm.bits, GUN_DIRECT_FIVE))
			cap = 5;
		else
			cap = 6;

		cap = cap - count_all_bullets(ptrFirearm);

		if (ch->delay_info3 >= 1)
			count = ch->delay_info3;

		if (count > cap || !count)
			count = cap;

		if (count > ptrRound->count)
			count = ptrRound->count;

		sprintf (buf, "You finish loading %d of #2%s#0 in to #2%s#0.\n", count, obj_short_desc(ptrRound), obj_short_desc (ptrFirearm));
		act(buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buf = '\0';
		sprintf (buf, "$n finishes loading %d of #2%s#0 with #2%s#0.", count, obj_short_desc(ptrRound), obj_short_desc (ptrFirearm));
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);


		if (ch->delay_info1 >= 0)
		{
			obj_from_obj (&ptrRound, count);
		}
		else
		{
			obj_from_char (&ptrRound, count);
		}

		obj_to_obj(ptrRound, ptrFirearm);

		ch->delay = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay_type = 0;
	}
	else
	{
		if (ptrFirearm->contains)
		{
			sprintf(buf, "%s#0 is already loaded with #2%s#0.\n", obj_short_desc(ptrFirearm), obj_short_desc(ptrFirearm->contains));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		if (ch->delay_info1 < 0)
		{
			if (ch->right_hand && ch->right_hand->coldload_id == ch->delay_info2)
			{
				ptrClip = ch->right_hand;
			}
			else if (ch->left_hand && ch->left_hand->coldload_id == ch->delay_info2)
			{
				ptrClip = ch->left_hand;
			}
		}
		else
		{
			if (ch->left_hand && ch->right_hand)
			{
				send_to_char ("Having taken another object in hand, you cease loading your firearm.\n",  ch);
				return;
			}
			for (i = 0; i < MAX_WEAR; i++)
			{
				if (!(ptrBelt = get_equip (ch, i)))
				{
					continue;
				}
				if (ptrBelt->coldload_id != ch->delay_info1 || !get_obj_in_list_id (ch->delay_info2, ptrBelt->contains))
				{
					continue;
				}

				if (GET_ITEM_TYPE (ptrBelt) == ITEM_AMMO_BELT)
				{
					for (ptrClip = ptrBelt->contains; ptrClip; ptrClip = ptrClip->next_content)
					{
						if (GET_ITEM_TYPE (ptrClip) == ITEM_CLIP && ptrClip->coldload_id == ch->delay_info2)
						{
							break;
						}
					}
				}

				if (ptrClip)
				{
					break;
				}
			}
		}
		if (!ptrClip)
		{
			send_to_char("Having lost your magazine, you cease loading your firearm.\n", ch);
			return;
		}

		if (!ptrClip->contains)
		{
			if (ptrClip->carried_by)
			{
				if (ptrClip == ch->right_hand || ptrClip == ch->left_hand)
				{
					sprintf(buf, "%s#0 in your %s hand does not contain any rounds.\n", obj_short_desc(ptrClip), (ptrClip == ch->right_hand ? "right" : "left"));
					*buf = toupper(*buf);
					sprintf(buffer, "#2%s", buf);
					send_to_char(buffer, ch);
				}
				else
				{
					send_to_char ("The magazine you are holding does not rounds.\n", ch);
				}
			}
			else if (ptrBelt)
			{
				sprintf(buf, "%s#0 in #2%s#0 does not contain any rounds.\n", obj_short_desc(ptrClip), obj_short_desc(ptrBelt));
				*buf = toupper(*buf);
				sprintf(buffer, "#2%s", buf);
				send_to_char(buffer, ch);
			}
			else
			{
				send_to_char ("That magazine does not contain any rounds.\n", ch);
			}
			return;
		}

		sprintf (buf, "You finish loading #2%s#0 in to #2%s#0.\n", obj_short_desc(ptrClip), obj_short_desc (ptrFirearm));
		act(buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buf = '\0';
		sprintf (buf, "$n finishes loading #2%s#0 with #2%s#0.", obj_short_desc(ptrClip), obj_short_desc (ptrFirearm));
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);

		if (ch->delay_info1 >= 0)
		{
			obj_from_obj (&ptrClip, 0);
		}
		else
		{
			obj_from_char (&ptrClip, 0);
		}

		obj_to_obj(ptrClip, ptrFirearm);

		ch->delay = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay_type = 0;
	}
}

void
	do_unload_firearm (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *ptrFirearm = NULL;
	OBJ_DATA *ptrClip = NULL;
	OBJ_DATA *ptrRound = NULL;
	OBJ_DATA *ptrBelt = NULL;
	int rounds = 0;
	int casings = 0;
	int i = 0;

	char buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char *error;

	if (!(((ptrFirearm = ch->right_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)
		|| ((ptrFirearm = ch->left_hand) && GET_ITEM_TYPE (ptrFirearm) == ITEM_FIREARM)))
	{
		send_to_char ("You must first be holding a loaded firearm.\n", ch);
		return;
	}

	if (IS_DIRECT(ptrFirearm))
	{
		if (!count_all_bullets(ptrFirearm))
		{
			sprintf(buf, "%s#0 isn't loaded.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		for (ptrRound = ptrFirearm->contains; ptrRound; ptrRound = ptrRound->next_content)
		{
			if (GET_ITEM_TYPE(ptrRound) == ITEM_ROUND)
			{
				rounds = count_bullets(ptrFirearm);
				for (i = 0; i < MAX_WEAR; i++)
				{
					if (!(ptrBelt = get_equip (ch, i)))
						continue;

					if (GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER && can_obj_to_container (ptrRound, ptrBelt, &error, ptrRound->count))
					{
						break;
					}
				}
			}
			else
			{
				casings += ptrRound->count;
			}
		}

		if (!ptrBelt)
		{
			// We shift them to one-handing it.
			if (ptrFirearm->location == WEAR_BOTH)
			{
				ptrFirearm->location = WEAR_PRIM;
			}

			while (ptrFirearm->contains)
			{
				ptrRound = ptrFirearm->contains;
				obj_from_obj (&ptrRound, 0);
				obj_to_char(ptrRound, ch);
			}

			sprintf (buf, "You unload %s%s%s from $P.",
				(rounds > 1 ? "some rounds" : rounds == 1 ? "a round" : ""),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "some casings" : casings == 1 ? "a casing" : ""));
			sprintf (buf2, "$n unloads %s%s%s from $P.",
				(rounds > 1 ? "some rounds" : rounds == 1 ? "a round" : ""),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "some casings" : casings == 1 ? "a casing" : ""));
		}
		else
		{
			while (ptrFirearm->contains)
			{
				ptrRound = ptrFirearm->contains;
				obj_from_obj (&ptrRound, 1);

				if (GET_ITEM_TYPE(ptrRound) == ITEM_ROUND && can_obj_to_container (ptrRound, ptrBelt, &error, 1))
				{
					obj_to_obj(ptrRound, ptrBelt);
				}
				else
				{
					obj_to_char(ptrRound, ch);
				}
			}

			sprintf (buf, "You unload $P, %s#2%s#0%s%s.",
				(rounds > 1 ? "placing some rounds in " : rounds == 1 ? "placing a round in " : ""),
				obj_short_desc(ptrBelt),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "unloading some casings" : casings == 1 ? "unloading a casing" : ""));
			sprintf (buf2, "$n unloads $P, %s#2%s#0%s%s.",
				(rounds > 1 ? "placing some rounds in " : rounds == 1 ? "placing a round in " : ""),
				obj_short_desc(ptrBelt),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "unloading some casings" : casings == 1 ? "unloading a casing" : ""));
		}
	}
	else if (IS_SLING(ptrFirearm))
	{
		if (!count_all_bullets(ptrFirearm))
		{
			sprintf(buf, "%s#0 isn't loaded.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		for (ptrRound = ptrFirearm->contains; ptrRound; ptrRound = ptrRound->next_content)
		{
			if (GET_ITEM_TYPE(ptrRound) == ITEM_ROUND)
			{
				rounds = count_bullets(ptrFirearm);
				for (i = 0; i < MAX_WEAR; i++)
				{
					if (!(ptrBelt = get_equip (ch, i)))
						continue;

					if (GET_ITEM_TYPE (ptrBelt) == ITEM_BANDOLIER && can_obj_to_container (ptrRound, ptrBelt, &error, ptrRound->count))
					{
						break;
					}
				}
			}
			else
			{
				casings += ptrRound->count;
			}
		}

		if (!ptrBelt)
		{
			// We shift them to one-handing it.
			if (ptrFirearm->location == WEAR_BOTH)
			{
				ptrFirearm->location = WEAR_PRIM;
			}

			while (ptrFirearm->contains)
			{
				ptrRound = ptrFirearm->contains;
				obj_from_obj (&ptrRound, 0);
				obj_to_char(ptrRound, ch);
			}

			sprintf (buf, "You unload %s%s%s from $P.",
				(rounds > 1 ? "some BBs" : rounds == 1 ? "a BB" : ""),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "some casings" : casings == 1 ? "a casing" : ""));
			sprintf (buf2, "$n unloads %s%s%s from $P.",
				(rounds > 1 ? "some BBs" : rounds == 1 ? "a BB" : ""),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "some casings" : casings == 1 ? "a casing" : ""));
		}
		else
		{
			while (ptrFirearm->contains)
			{
				ptrRound = ptrFirearm->contains;
				obj_from_obj (&ptrRound, 1);

				if (GET_ITEM_TYPE(ptrRound) == ITEM_ROUND && can_obj_to_container (ptrRound, ptrBelt, &error, 1))
				{
					obj_to_obj(ptrRound, ptrBelt);
				}
				else
				{
					obj_to_char(ptrRound, ch);
				}
			}

			sprintf (buf, "You unload $P, %s#2%s#0%s%s.",
				(rounds > 1 ? "placing some BBs in " : rounds == 1 ? "placing a BB in " : ""),
				obj_short_desc(ptrBelt),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "unloading some casings" : casings == 1 ? "unloading a casing" : ""));
			sprintf (buf2, "$n unloads $P, %s#2%s#0%s%s.",
				(rounds > 1 ? "placing some BBs in " : rounds == 1 ? "placing a BB in " : ""),
				obj_short_desc(ptrBelt),
				(rounds && casings ? " and " : ""),
				(casings > 1 ? "unloading some casings" : casings == 1 ? "unloading a casing" : ""));
		}
	}
	else
	{

		if (!(ptrClip = ptrFirearm->contains))
		{
			sprintf(buf, "%s#0 isn't loaded.\n", obj_short_desc(ptrFirearm));
			*buf = toupper(*buf);
			sprintf(buffer, "#2%s", buf);
			send_to_char(buffer, ch);
			return;
		}

		for (i = 0; i < MAX_WEAR; i++)
		{
			if (!(ptrBelt = get_equip (ch, i)))
				continue;

			if (GET_ITEM_TYPE (ptrBelt) == ITEM_AMMO_BELT
				&& can_obj_to_container (ptrClip, ptrBelt, &error, 1))
				break;
		}

		if (!ptrBelt)
		{
			// We shift them to one-handing it.
			if (ptrFirearm->location == WEAR_BOTH)
			{
				ptrFirearm->location = WEAR_PRIM;
			}

			obj_from_obj (&ptrClip, 0);
			obj_to_char(ptrClip, ch);
			sprintf (buf, "You unload $p from $P.");
			sprintf (buf2, "$n unloads $p from $P.");
		}
		else
		{
			obj_from_obj (&ptrClip, 0);
			obj_to_obj(ptrClip, ptrBelt);
			sprintf (buf, "You unload $P, placing $p in #2%s#0", obj_short_desc(ptrBelt));
			sprintf (buf2, "$n unloads $P, placing $p in #2%s#0", obj_short_desc(ptrBelt));
		}
	}

	// If our gun was jammed, we clear the jam.
	if (ptrFirearm->o.firearm.setting < 0)
	{
		ptrFirearm->o.firearm.setting = -ptrFirearm->o.firearm.setting;
	}

	act(buf, false, ch, ptrClip, ptrFirearm, TO_CHAR | _ACT_FORMAT);
	act(buf2, false, ch, ptrClip, ptrFirearm, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
}


void
	delayed_unload_firearm (CHAR_DATA * ch)
{
	;
}

void modify_firearm (CHAR_DATA *ch, char *argument, int cmd, OBJ_DATA *obj)
{
	OBJ_DATA *tobj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char arg3[MAX_STRING_LENGTH] = { '\0' };

	// If the object contains something, and that something is a battery, we have our tobj.

	if (obj->contains)
		tobj = obj->contains;

	argument = one_argument (argument, arg2);

	if (!*arg2)
	{
		act("How do you wish to modify $p? Enter ""modify <object> help"" for object-specific commands.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!str_cmp(arg2, "safety"))
	{
		if (IS_SET(obj->o.firearm.bits, GUN_SAFETY))
		{
			argument = one_argument (argument, arg3);

			if (obj->o.firearm.setting < 0)
			{
				act ("$p is jammed: you'll need to unload it to clear the jam.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			if (!*arg3)
			{
				if (obj->o.firearm.setting == 1)
				{
					act("You switch $p's safety to off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act("$n switches $p's safety to off.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
					obj->o.firearm.setting = 0;
					return;
				}
				else
				{
					act("You switch $p's safety to on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act("$n switches $p's safety to on.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
					obj->o.firearm.setting = 1;
					return;
				}
			}
			else if (!str_cmp(arg3, "on"))
			{
				if (obj->o.firearm.setting == 1)
				{
					act("But $p's safety is already on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				else
				{
					act("You switch $p's safety to on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act("$n switches $p's safety to on.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
					obj->o.firearm.setting = 1;
					return;
				}
			}
			else if (!str_cmp(arg3, "off"))
			{
				if (obj->o.firearm.setting != 1)
				{
					act("But $p's safety is already off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				else
				{
					act("You switch $p's safety to off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act("$n switches $p's safety to off.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
					obj->o.firearm.setting = 0;
					return;
				}
			}
			else
			{
				act("You want to do what to $p? Enter ""modify <object> help"" for object-specific commands", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}
		}
		else
		{
			act("But $p doesn't have a safety.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			obj->o.firearm.setting = 0;
		}
	}
	else if (!str_cmp(arg2, "burst"))
	{

		if (obj->o.firearm.setting < 0)
		{
			act ("$p is jammed: you'll need to unload it to clear the jam.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (IS_SET(obj->o.firearm.bits, GUN_BURST))
		{

			if (obj->o.firearm.setting == 1)
			{
				act("You switch $p to semi-automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n switches $p to semi-automatic fire.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
				obj->o.firearm.setting = 0;
				return;
			}
			else
			{
				act("You switch $p to burst fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n switches $p to burst fire..", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
				obj->o.firearm.setting = 2;
				return;
			}
		}
		else
		{
			act("$p cannot be set to burst fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			obj->o.firearm.setting = 0;
		}
	}
	else if (!str_cmp(arg2, "auto") || !str_cmp(arg2, "automatic"))
	{

		if (obj->o.firearm.setting < 0)
		{
			act ("$p is jammed: you'll need to unload it to clear the jam.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}


		if (IS_SET(obj->o.firearm.bits, GUN_AUTOMATIC))
		{
			if (obj->o.firearm.setting == 1)
			{
				act("You switch $p to semi-automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n switches $p to semi-automatic fire.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
				obj->o.firearm.setting = 0;
				return;
			}
			else
			{
				act("You switch $p to automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n switches $p to automatic fire.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
				obj->o.firearm.setting = 3;
				return;
			}
		}
		else
		{
			act("$p cannot be set to automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			obj->o.firearm.setting = 0;
		}
	}
	else if (!str_cmp(arg2, "semi") || !str_cmp(arg2, "semi-automatic"))
	{

		if (obj->o.firearm.setting < 0)
		{
			act ("$p is jammed: you'll need to unload it to clear the jam.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (obj->o.firearm.setting != 0)
		{
			act("You switch $p to semi-automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n switches $p to semi-automatic fire.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
			obj->o.firearm.setting = 0;
			return;
		}
		else
		{
			act("$p is already set to semi-automatic fire.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			obj->o.firearm.setting = 0;
			return;
		}
	}
	else if (!str_cmp(arg2, "help"))
	{
		sprintf(buf, "\n#6You can use the follow commands on this firearm:#0\n\n");

		if (IS_SET(obj->o.firearm.bits, GUN_SAFETY))
			sprintf(buf + strlen(buf), "#6Safety#0:       Toggles the weapon's safety on and off.\n");
		sprintf(buf + strlen(buf), "#6Semi #0:        Switches the weapon to semi-automatic fire mode.\n");
		if (IS_SET(obj->o.firearm.bits, GUN_BURST))
			sprintf(buf + strlen(buf), "#6Burst#0:        Toggles the weapon to burst fire mode.\n");
		if (IS_SET(obj->o.firearm.bits, GUN_AUTOMATIC))
			sprintf(buf + strlen(buf), "#6Auto #0:        Toggles the weapon to automatic fire mode.\n");
		send_to_char(buf, ch);
		return;
	}
	else
	{
		act("How do you wish to modify $p? Enter ""modify <object> help"" for object-specific commands.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}
}


// We get a number out of 150, and divide it by ten, and reduce the aim time by
// a maximum of 10, for 1 second -- highly skilled dudes need no time to make a shot
// Range is how many rooms away: each point of range is an extra 2 seconds of time.

int
	aim_penalty (CHAR_DATA *ch, OBJ_DATA *obj, int range)
{
	int aim = 0;
	int skill = 0;
	int total = 0;

	if (!ch || !obj)
		return 0;

	skill = ch->skills[obj->o.firearm.use_skill];
	aim = (ch->skills[SKILL_AIM]) / 2;

	total = skill + aim;
	total -= ((range ? range : 0) * 20);
	total = (int) (total / 10);

	// Handguns aim a bit quicker
	if (obj->o.firearm.use_skill == SKILL_HANDGUN)
	{
		total -= 1;
	}

	total = MAX(0, total);
	total = MIN(10, total);

	return total;
}

void
	firearm_aim (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *firearm = NULL;
	CHAR_DATA *target = NULL;
	CHAR_DATA *tch = NULL;
	ROOM_DATA *room = NULL;
	ROOM_DIRECTION_DATA *exit = NULL;
	char arg1[MAX_STRING_LENGTH] = {'\0'};
	char arg2[MAX_STRING_LENGTH] = {'\0'};
	char arg3[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buffer[MAX_STRING_LENGTH] = {'\0'};
	int ranged = 0, dir = 0, delay = 0, bodypart = -1;
	bool old_target = false;
	char original[MAX_STRING_LENGTH];
	sprintf (original, "%s", argument);


	if (IS_SWIMMING (ch))
	{
		send_to_char ("You can't do that while swimming!\n", ch);
		return;
	}
	if (IS_FLOATING (ch))
	{
		send_to_char ("You can't do that while floating!\n", ch);
		return;
	}
	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}
	if (IS_SET(ch->room->room_flags, PEACE))
	{
		act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	// No arguments, no dice.
	if (!*arg1)
	{
		send_to_char ("Usage: aim (<direction>) <target> (<bodypart>)\n", ch);
		return;
	}

	// Do we have a firearm in either wear_both, wear_prim, or wear_sec?
	if (!((firearm = get_equip (ch, WEAR_BOTH)) || (firearm = get_equip (ch, WEAR_PRIM))  || (firearm = get_equip (ch, WEAR_SEC))) || (GET_ITEM_TYPE(firearm) != ITEM_FIREARM))
	{
		send_to_char ("You aren't wielding a firearm.\n", ch);
		return;
	}

	// Is it loaded?
	/*
	if (!firearm->contains)
	{
	send_to_char ("Your firearm isn't loaded.\n", ch);
	return;
	}
	*/

	// Is it loaded?
	/*
	if (firearm && !IS_SLING(firearm) && !IS_DIRECT(firearm) && firearm->contains && !firearm->contains->contains)
	{
	send_to_char ("Your firearm isn't loaded.\n", ch);
	return;
	}
	*/

	// If our arg1 is n, e, s, w, d or u, we're ranged baby.
	if ((!strn_cmp ("east", arg1, strlen (arg1)) ||
		!strn_cmp ("west", arg1, strlen (arg1)) ||
		!strn_cmp ("north", arg1, strlen (arg1)) ||
		!strn_cmp ("south", arg1, strlen (arg1)) ||
		!strn_cmp ("up", arg1, strlen (arg1)) ||
		!strn_cmp ("down", arg1, strlen (arg1))) && *arg2)
	{
		ranged = 1;
		ch->delay_info1 = 1;
	}

	if (!ranged)
	{
		if (!(target = get_char_room_vis (ch, arg1)))
		{
			send_to_char ("Who did you want to target?\n", ch);
			return;
		}
		if (target == ch)
		{
			send_to_char ("Now, now, now... things can't be THAT bad, can they?\n", ch);
			return;
		}
		if (IS_SET (target->act, ACT_VEHICLE))
		{
			sprintf (buf, "And what good will that do to hurt $N?");
			act (buf, false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
			return;
		}
		if (get_affect (target, MAGIC_HIDDEN) && IS_MORTAL (ch))
		{
			send_to_char ("Due to your target's cover, you cannot get a clear shot.\n", ch);
			return;
		}

		if (ch->aiming_at && are_grouped(ch->aiming_at, target))
		{
			old_target = true;
		}

		if (*arg2 && str_cmp(arg2, "!"))
		{
			if (!strcmp(arg2, "body"))
			{
				bodypart = HITLOC_BODY;
				delay = 1;
			}
			else if (!strcmp(arg2, "leg") || !strcmp(arg2, "legs"))
			{
				if (number(0,1))
				{
					bodypart = HITLOC_HILEGS;
				}
				else
				{
					bodypart = HITLOC_LOLEGS;
				}

				delay = 1;
			}
			else if (!strcmp(arg2, "arm") || !strcmp(arg2, "arms"))
			{
				if (number(0,1))
				{
					bodypart = HITLOC_HIARMS;
				}
				else
				{
					bodypart = HITLOC_LOARMS;
				}
				delay = 1;
			}
			else if (!strcmp(arg2, "head"))
			{
				bodypart = HITLOC_HEAD;
				delay = 4;
			}
			else if (!strcmp(arg2, "foot") || !strcmp(arg2, "feet"))
			{
				bodypart = HITLOC_FEET;
				delay = 2;
			}
			else if (!strcmp(arg2, "hand") || !strcmp(arg2, "hands"))
			{
				bodypart = HITLOC_HANDS;
				delay = 2;
			}
			else
			{
				send_to_char ("That's not a bodypart! Choose head, arms, hands, body, legs or feet.\n", ch);
				return;
			}
		}

		if (bodypart >= 0 && get_second_affect (ch, SA_POINTSTRIKE, NULL))
		{
			remove_second_affect(get_second_affect (ch, SA_POINTSTRIKE, NULL));
		}

		// Make sure people don't hit the wrong person.
		if (are_grouped (ch, target) && is_brother (ch, target) && *argument != '!')
		{
			sprintf (buf, "#1You are about to aim at $N #1who is a fellow group member!#0 To confirm, type \'#6aim %s !#0\', without the quotes.", original);
			act (buf, false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
			return;
		}


		// Now we write our aiming at message.
		if (ch->aiming_at && ch->aiming_at != target)
			sprintf (buf, "Switching targets, you now take aim at #5%s#0%s%s.", char_short (target), (bodypart >= 0 ? "'s " : ""), (bodypart >= 0 ? arg2 : ""));
		else
			sprintf (buf, "You take aim at #5%s#0%s%s.", char_short (target), (bodypart >= 0 ? "'s " : ""), (bodypart >= 0 ? arg2 : ""));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "%s#0 takes aim at #5%s#0%s%s with #2%s#0.", char_short (ch), char_short (target), (bodypart >= 0 ? "'s " : ""), (bodypart >= 0 ? arg2 : ""), obj_short_desc(firearm));
		*buf = toupper (*buf);
		sprintf (buffer, "#5%s", buf);

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (tch == target)
				continue;
			if (CAN_SEE (tch, ch))
				act (buffer, false, ch, 0, tch, TO_VICT | _ACT_FORMAT | _ACT_COMBAT | _ACT_FIREFIGHT);
		}
		sprintf (buf, "%s#0 takes aim at you%s%s with #2%s#0!",  char_short (ch), (bodypart >= 0 ? "r " : ""), (bodypart >= 0 ? arg2 : ""), obj_short_desc(firearm));
		*buf = toupper (*buf);
		sprintf (buffer, "#5%s", buf);
		if (CAN_SEE (target, ch))
			act (buffer, false, ch, 0, target, TO_VICT | _ACT_FORMAT);

		ch->aiming_at = target;
		ch->aim = aim_penalty(ch, firearm, ch->delay_info1);
		ch->aim -= delay;
		ch->aim = MAX(ch->aim, 0);
		if (bodypart >= 0)
			add_second_affect (SA_POINTSTRIKE, 90, ch, NULL, NULL, bodypart);
		add_targeted(target, ch);
		notify_guardians (ch, target, 3);
		ch->delay_info1 = 0;

		ch->combat_block = 3;
		target->combat_block = 3;

		criminalize (ch, target, target->room->zone, CRIME_SHOOT);

		return;
	}
	else if (ranged)
	{
		if (!strn_cmp ("north", arg1, strlen (arg1)))
			dir = 0;
		else if (!strn_cmp ("east", arg1, strlen (arg1)))
			dir = 1;
		else if (!strn_cmp ("south", arg1, strlen (arg1)))
			dir = 2;
		else if (!strn_cmp ("west", arg1, strlen (arg1)))
			dir = 3;
		else if (!strn_cmp ("up", arg1, strlen (arg1)))
			dir = 4;
		else if (!strn_cmp ("down", arg1, strlen (arg1)))
			dir = 5;

		if (!EXIT (ch, dir))
		{
			send_to_char ("There isn't an exit in that direction.\n", ch);
			return;
		}

		ch->delay_who = str_dup (dirs[dir]);

		room = vnum_to_room (EXIT (ch, dir)->to_room);

		exit = EXIT (ch, dir);
		if (exit
			&& IS_SET (exit->exit_info, EX_CLOSED)
			&& !IS_SET (exit->exit_info, EX_ISGATE))
		{
			send_to_char ("Your view is blocked.\n", ch);
			return;
		}

		if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
		{
			exit = room->dir_option[dir];
			if (!exit)
			{
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}

			if (exit
				&& IS_SET (exit->exit_info, EX_CLOSED)
				&& !IS_SET (exit->exit_info, EX_ISGATE))
			{
				send_to_char ("Your view is blocked.\n", ch);
				return;
			}

			if (room->dir_option[dir])
				room = vnum_to_room (room->dir_option[dir]->to_room);
			else
				room = NULL;

			if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
			{
				exit = room->dir_option[dir];
				if (!exit)
				{
					send_to_char ("You don't see them within range.\n", ch);
					return;
				}

				if (exit
					&& IS_SET (exit->exit_info, EX_CLOSED)
					&& !IS_SET (exit->exit_info, EX_ISGATE))
				{
					send_to_char ("Your view is blocked.\n", ch);
					return;
				}

				if (room->dir_option[dir])
					room = vnum_to_room (room->dir_option[dir]->to_room);
				else
					room = NULL;

				if (is_sunlight_restricted (ch, room))
					return;

				if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
				{
					exit = room->dir_option[dir];
					if (!exit)
					{
						send_to_char ("You don't see them within range.\n", ch);
						return;
					}

					if (exit
						&& IS_SET (exit->exit_info, EX_CLOSED)
						&& !IS_SET (exit->exit_info, EX_ISGATE))
					{
						send_to_char ("Your view is blocked.\n", ch);
						return;
					}

					send_to_char ("You don't see them within range.\n", ch);
					return;
				}

				else
					ch->delay_info1 = 3;
			}
			else
				ch->delay_info1 = 2;
		}

		if (!target || !CAN_SEE (ch, target) || !has_been_sighted (ch, target))
		{
			send_to_char ("You don't see them within range.\n", ch);
			return;
		}

		if (target == ch)
		{
			send_to_char ("Now, now, now... things can't be THAT bad, can they?\n", ch);
			return;
		}

		if (get_affect (target, MAGIC_HIDDEN) && IS_MORTAL (ch))
		{
			send_to_char ("Due to your target's cover, you cannot get a clear shot.\n", ch);
			return;
		}

		argument = one_argument(argument, arg3);

		if (*arg3)
		{
			if (!strcmp(arg3, "body"))
			{
				bodypart = 0;
				delay = 2;
			}
			else if (!strcmp(arg3, "leg") || !strcmp(arg3, "legs"))
			{
				bodypart = 1;
				delay = 1;
			}
			else if (!strcmp(arg3, "arm") || !strcmp(arg3, "arms"))
			{
				bodypart = 2;
				delay = 1;
			}
			else if (!strcmp(arg3, "head"))
			{
				bodypart = 3;
				delay = 4;
			}
			else if (!strcmp(arg3, "foot") || !strcmp(arg3, "feet"))
			{
				bodypart = 5;
				delay = 2;
			}
			else if (!strcmp(arg3, "hand") || !strcmp(arg3, "hands"))
			{
				bodypart = 6;
				delay = 2;
			}
			else
			{
				send_to_char ("That's not a bodypart! Choose head, arms, hands, body, legs or feet.\n", ch);
				return;
			}
		}

		if (bodypart >= 0 && get_second_affect (ch, SA_POINTSTRIKE, NULL))
		{
			remove_second_affect(get_second_affect (ch, SA_POINTSTRIKE, NULL));
		}

		sprintf (buf, "You turn %sward and take aim at #5%s#0%s%s.", dirs[dir], char_short (target), (bodypart >= 0 ? "'s " : ""), (bodypart >= 0 ? arg3 : ""));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "%s#0 turns %sward, taking aim at a distant target with #2%s#0.", char_short (ch), dirs[dir], obj_short_desc(firearm));

		*buf = toupper (*buf);
		ch->ranged_enemy = target;
		sprintf (buffer, "#5%s", buf);
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (tch == target)
				continue;
			if (CAN_SEE (tch, ch))
				act (buffer, false, ch, 0, tch, TO_VICT | _ACT_FORMAT | _ACT_FIREFIGHT | _ACT_COMBAT);
		}

		ch->combat_block = 3;
		target->combat_block = 3;

		// If you change an already existing aim to someone within the same group,
		// then we don't need to start all over again, just take a small delay.
		if (old_target)
		{
			ch->aiming_at = target;
			ch->aim -= 2;
			ch->aim = MAX(aim_penalty(ch, firearm, ch->delay_info1), ch->aim);
			add_targeted(target, ch);
			ch->aim -= delay;
			ch->aim = MAX(ch->aim, 0);
			if (bodypart >= 0)
				add_second_affect (SA_POINTSTRIKE, 90, ch, NULL, NULL, bodypart);
		}
		else
		{
			ch->aiming_at = target;
			ch->aim = MAX(aim_penalty(ch, firearm, ch->delay_info1), 0);
			ch->aim -= delay;
			ch->aim = MAX(ch->aim, 0);
			if (bodypart >= 0)
				add_second_affect (SA_POINTSTRIKE, 90, ch, NULL, NULL, bodypart);
			add_targeted(target, ch);
			criminalize (ch, target, target->room->zone, CRIME_ASSAULT);
		}

		// We also go through and add an automatic watch in that direction.
		remove_affect_type (ch, MAGIC_WATCH);
		remove_affect_type (ch, AFFECT_WATCH_DIR);
		magic_add_affect (ch, AFFECT_WATCH_DIR, -1, 0, 0, 0, 0);

		AFFECTED_TYPE *af = NULL;

		af = get_affect (ch, AFFECT_WATCH_DIR);

		af->a.shadow.shadow = NULL;
		af->a.shadow.edge = dir;

		notify_guardians (ch, target, 3);
		return;
	}
}

// Adds someone to our list of people aiming at us.
// Returns true if we added the guy, false if we didn't.

void
	add_targeted(CHAR_DATA * target, CHAR_DATA *aimer)
{
	// No adder or addee, don't bother.
	if (!target || !aimer)
		return;

	// Get some basics out of the way - if they're not in a room anywhere,
	// then they are probably dead/invalid target.
	if (!target->room || !aimer->room || !target->in_room || !aimer->in_room)
		return;

	TARGETED_BY *temp_one = NULL;

	for (temp_one = target->targeted_by; temp_one; temp_one = temp_one->next)
	{
		if (temp_one->ch == aimer)
			return;
	}

	if (!target->targeted_by)
	{
		CREATE (target->targeted_by, TARGETED_BY, 1);
		target->targeted_by->ch = aimer;
		target->targeted_by->next = NULL;
	}
	else
	{
		CREATE (temp_one, TARGETED_BY, 1);
		temp_one->ch = aimer;
		temp_one->next = target->targeted_by;
		target->targeted_by = temp_one;
	}

	// First we go through and make sure this guy isn't already on our list:
	// if he is, just return at this point.
	//vector<targeted_bys*>::iterator it;
	//for (it = target->targeted_by->begin(); it != target->targeted_by->end(); it++)
	//{
	//    if ((*it)->ch == aimer)
	//    {
	//        return;
	//    }
	//}

	// Otherwise, it's adding time.
	//TARGETED_BY *targeter = new TARGETED_BY;
	//targeter->ch = aimer;
	//target->targeted_by->push_back(targeter);

	return;
}

// Removes the aimer from our target's targeted_by list.
void
	remove_targeted (CHAR_DATA *target, CHAR_DATA *aimer)
{
	// No targer or aimer, quit out.
	if (!target || !aimer || !target->targeted_by)
		return;

	TARGETED_BY *temp_one = NULL;
	TARGETED_BY *temp_two = NULL;

	// If we're the first targeted_by, we strip out
	// our targeted_by to be freed, and set the rest
	// to be our next.
	if (target->targeted_by->ch == aimer)
	{
		target->targeted_by->ch->aiming_at = NULL;
		target->targeted_by->ch->aim = 0;
		temp_two = target->targeted_by;
		target->targeted_by = target->targeted_by->next;
	}
	else
	{
		// Otherwise, we look through
		for (temp_one = target->targeted_by; temp_one; temp_one = temp_one->next)
		{
			// If we're the next targeted by, set it to temp_two, and advance us
			// one place.
			if (temp_one->next && temp_one->next->ch == aimer)
			{

				temp_one->next->ch->aiming_at = NULL;
				temp_one->next->ch->aim = 0;
				temp_two = temp_one->next;
				temp_one->next = temp_one->next->next;
				break;
			}
		}
	}

	// If we found it, free it.
	if (temp_two)
		mem_free (temp_two);

	// Go through, and remove where we find a match -- if they do exist as a ch, clear
	// out their aiming and aiming_at too.
	//for (vector<targeted_bys*>::iterator it = target->targeted_by->begin(); it != target->targeted_by->end();it++)
	//{
	//    if ((*it)->ch == aimer)
	//    {
	//		(*it)->ch->aim = 0;
	//		(*it)->ch->aiming_at = 0;

	//		delete *it;
	//		target->targeted_by->erase(it);
	//		return;
	//	}
	//}
}

// Removes the aimer from our target's targeted_by list.
void
	remove_all_targeted (CHAR_DATA *target)
{

	if (!target || !target->targeted_by)
		return;

	while (target->targeted_by)
		remove_targeted(target, target->targeted_by->ch);

	target->targeted_by = NULL;

	// No targer or aimer, quit out.
	//if (!target)
	//    return;

	// Go through, and remove the aim and aiming_at data of everyone involved.
	//for (vector<targeted_bys*>::iterator it = target->targeted_by->begin(); it != target->targeted_by->end();it++)
	//{
	//    if ((*it)->ch)
	//    {
	//		(*it)->ch->aim = 0;
	//		(*it)->ch->aiming_at = 0;
	//	}
	//
	//	delete *it;
	//}

	//target->targeted_by->clear();
}


// Adds someone to our list via automatic
// Returns true if we added the guy, false if we didn't.

bool
	add_overwatch(CHAR_DATA * ch, CHAR_DATA *tch, int source, bool verbose)
{
	char buf[MAX_STRING_LENGTH] = {'\0'};

	// No adder or addee, don't bother.
	if (!ch || !tch)
		return false;

	// Don't bother adding ourselves, either.
	if (ch == tch)
		return false;

	// Get some basics out of the way - if they're not in a room anywhere,
	// then they are probably dead/invalid target.
	if (!ch->room || !tch->room || !ch->in_room || !tch->in_room)
		return false;

	// Source is the level at which this overwatch is being added:
	// 0 is manual,
	// 1 is from the leader,
	// 2 is via traded gunshots,
	// 3 is via group gunshots.
	// If our over state is less than that, we won't take the add.
	if (ch->over_target < source)
		return false;

	// If we're adding this from our leader, but we're not following anyone,
	// then we need to bug out.
	if (source == 1 && !ch->following)
		return false;

	// If we can't see them, then no overwatch is added.
	if (!CAN_SEE(ch, tch))
		return false;

	// Don't want to be accidentally targeting group mates.
	if (are_grouped(ch, tch))
		return false;

	// First we go through and make sure this guy isn't already on our list:
	// if he is, we kick him back off.
	vector<overwatch_enemy*>::iterator it;
	for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
	{
		if ((*it)->sdesc == char_short(tch))
		{
			return false;
		}
	}

	// Otherwise, it's adding time.

	OVERWATCH_ENEMY *enemy = new OVERWATCH_ENEMY;
	//sprintf(buf, "%s", char_short(tch));
	enemy->sdesc = char_short(tch);
	enemy->source = source;
	enemy->time = 4;
	ch->over_enemies->push_back(enemy);

	// If verbose, we make a note of it.
	if (verbose)
	{
		if (source == 1)
		{
			sprintf(buf, "You add $N to your overwatch list at #5%s#0's command.", char_short(ch->following));
		}
		else
		{
			sprintf(buf, "You add $N to your overwatch list.");
		}
		act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
	}

	return true;
}

// Comapres tch against ch's overwatches, and if there's a match,
// they fit within ch's threshold, and ch is ready to aim but isn't
// already aiming at something, then we're good to go.
//
// If check_only is true, then we just acknowledge the true value of check_overwatch, but not
// it actually taking effect, i.e. if we're scanning a whole bunch of people at once but only
// want to change our aim at the end.

bool
	check_overwatch (CHAR_DATA *ch, CHAR_DATA *tch, bool check_only)
{
	// No adder or addee, don't bother.
	if (!ch || !tch)
		return false;

	// Don't bother adding ourselves, either.
	if (ch == tch)
		return false;

	// Get some basics out of the way - if they're not in a room anywhere,
	// then they are probably dead/invalid target.
	if (!ch->room || !tch->room || !ch->in_room || !tch->in_room)
		return false;

	// If we're already aiming or fighting, don't bother.
	if (ch->aiming_at || ch->fighting)
		return false;

	// Obviously, if we've got no overwatches, then there's no point to this.
	if (ch->over_enemies->empty())
		return false;

	// If we can't see them, then no autoaim.
	if (!CAN_SEE(ch, tch))
		return false;

	// Don't want to be accidentally targeting group mates.
	if (are_grouped(ch, tch))
		return false;

	OBJ_DATA *firearm = NULL;

	// We obviously need to be wielding a firearm as well.
	if (!(firearm = has_firearm(ch, 0)))
		return false;

	bool is_found = false;

	// If all those conditions are met, then we go through our overwatch list and see if we can get a match.
	vector<overwatch_enemy*>::iterator it;
	for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
	{
		if ((*it)->sdesc == char_short(tch))
		{
			is_found = true;
			if (check_only)
				return true;
			else
				break;
		}
	}

	char buf[MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA *rch = NULL;

	// If we didn't get a match, then we return.
	if (!is_found)
		return false;

	if (tch)
	{
		if (get_affect(tch, AFFECT_COVER) && ch->over_thresh <= 0)
			return false;

		if (tch->speed == 8  && ch->over_thresh <= 1)
			return false;

		if (ch->in_room == tch->in_room)
		{
			sprintf(buf, "As $N falls within your sights, you take aim at #5%s#0.", HMHR(tch));
			act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n takes aim at #5%s#0 with $p.", char_short(tch));

			for (rch = ch->room->people; rch; rch = rch->next_in_room)
			{
				if (rch == ch)
					continue;
				if (rch == tch)
					continue;
				if (CAN_SEE (rch, ch))
					act (buf, false, ch, firearm, rch, TO_VICT | _ACT_FORMAT | _ACT_FIREFIGHT);
			}

			if (CAN_SEE (tch, ch))
				act ("$n takes aim at you with $p!", false, ch, firearm, tch, TO_VICT | _ACT_FORMAT);
		}
		else
		{
			int dir = 0;
			dir = rev_dir[track(ch, tch->in_room)];

			sprintf(buf, "As $N falls within your sights to the %s, you take aim at #5%s#0.\n", direction[dir], HMHR(tch));
			act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n turns %sward, taking aim at a distant target with $p.\n", direction[dir]);

			for (rch = ch->room->people; rch; rch = rch->next_in_room)
			{
				if (rch == ch)
					continue;
				if (rch == tch)
					continue;
				if (CAN_SEE (rch, ch))
					act (buf, false, ch, firearm, rch, TO_VICT | _ACT_FORMAT | _ACT_FIREFIGHT);
			}
			ch->delay_who = str_dup (dirs[rev_dir[dir]]);
		}

		add_targeted(tch, ch);
		ch->aiming_at = tch;
		ch->aim = aim_penalty(ch, firearm, 0);
		ch->aim = MAX(ch->aim, 0);

		notify_guardians (ch, tch, 3);
		return true;
	}
	return false;
}

// Goes through and removes any overwatches more than 4 in-game hours old.
void
	remove_overwatch (CHAR_DATA *ch)
{

	// No ch, quit out.
	if (!ch)
		return;

	for (vector<overwatch_enemy*>::iterator it = ch->over_enemies->begin(); it != ch->over_enemies->end();)
	{
		(*it)->time --;
		if ((*it)->time <= 0)
		{
			delete *it;
			ch->over_enemies->erase(it);
			it = ch->over_enemies->begin();
		}
		else
		{
			it++;
		}
	}
}

void
	remove_all_overwatch (CHAR_DATA *ch)
{
	int run = 0;

	// No ch, quit out.
	if (!ch)
		return;

	for (vector<overwatch_enemy*>::iterator it = ch->over_enemies->begin(); it != ch->over_enemies->end();it++)
	{
		run++;
		delete *it;
	}

	ch->over_enemies->clear();
}


void
	do_overwatch (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *target = NULL;
	ROOM_DATA *room = NULL;
	ROOM_DIRECTION_DATA *exit = NULL;
	int ind = 1;
	char original[MAX_STRING_LENGTH] = {'\0'};
	char arg1[MAX_STRING_LENGTH] = {'\0'};
	char arg2[MAX_STRING_LENGTH] = {'\0'};
	char arg3[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	int ranged = 0, dir = 0;

	sprintf (original, "%s", argument);
	argument = one_argument (argument, arg1);

	// No arguments, no dice.
	if (!*arg1)
	{
		send_to_char ("The overwatch command, o for short, allows you to keep track of those who are shooting you.\n", ch);
		send_to_char ("Entries on your overwatch list last for 1 Real Life hour.\n", ch);
		send_to_char ("Usage: \n", ch);
		send_to_char ("      #6o target#0: 1 for manual only, 2 for directed and manual, 3 for direct shots, 4 for group shots.\n", ch);
		send_to_char ("      #6o thresh#0: 1 for easy, 2 for medium, 3 for all.\n", ch);
		send_to_char ("      #6o list#0: shows everyone on your overwatch list and your current overwatch status\n", ch);
		send_to_char ("      #6o clear#0: clears your entire overwatch list\n", ch);
		send_to_char ("      #6o clear ###0: clears a specific numeric entry from your overwatch list\n", ch);
		//send_to_char ("      #6o share ## <target>#0: shares an entry from your overwatch list with your target\n", ch);
		send_to_char ("      #6o (<direction>) <target>#0: adds the target to your overwatch list\n", ch);
		send_to_char ("      #6o (follower) (<direction>) <target>#0: adds the target to your follower's overwatch list\n", ch);

		return;
	}

	if (!str_cmp(arg1, "target"))
	{

		argument = one_argument (argument, arg2);

		if (!*arg2 || !atoi(arg2) || atoi(arg2) > 4 || atoi(arg2) < 1)
		{
			act("You need to enter a target for your overwatch additions: 1 for manual only, 2 for leader, 3 for targets you trade shots with, 4 for groups your group trades shots with.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		else if (atoi(arg2) == 1)
		{
			act("Entries will won't be automatically added your overwatch list, only manually.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_target = 0;
		}
		else if (atoi(arg2) == 2)
		{
			act("Entries will won't be automatically added your overwatch list, only manually and as directed.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_target = 1;
		}
		else if (atoi(arg2) == 3)
		{
			act("Entries will be automatically added your overwatch list for targets you shoot at and those who shoot at you, or engages you in melee combat.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_target = 2;
		}
		else if (atoi(arg2) == 4)
		{
			act("Entries will be automatically added your overwatch list for targets who shoot at your group, or your group shoots at.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_target = 3;
		}
		else
		{
			ch->over_target = 0;
		}
		return;
	}
	else if (!str_cmp(arg1, "thresh"))
	{
		argument = one_argument (argument, arg2);

		if (!*arg2 || !atoi(arg2) || atoi(arg2) > 4 || atoi(arg2) < 1)
		{
			act("You need to enter a threshold for your overwatch targeting: 1 for easy targets, 2 for medium targets, 3 for hard targets, 4 for any targets", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		else if (atoi(arg2) == 1)
		{
			act("You'll now only aim at overwatch targets that are easy to hit -- typically, those not in cover and upright.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_thresh = 0;
		}
		else if (atoi(arg2) == 2)
		{
			act("You'll now aim at overwatch targets that are easy or moderately hard to hit -- typically, those not in cover or not crawling.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_thresh = 1;
		}
		else if (atoi(arg2) == 3)
		{
			act("You'll now aim at any and all overwatch targets.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->over_thresh = 2;
		}
		else
		{
			ch->over_thresh = 0;
		}
		return;
	}
	else if (!str_cmp(arg1, "list") || !str_cmp(arg1, "l"))
	{
		sprintf(buf, "You add overwatch for targets #6%s#0, and take aim at targets that are #6%s#0 to hit.\n\n",
			(ch->over_target == 0 ? "manually" : ch->over_target == 1 ? "as directed" : ch->over_target == 2 ? "that shoot at you" : "that shoot at your group"),
			(ch->over_thresh == 0 ? "easy" : ch->over_thresh == 1 ? "moderately hard" : "any chance"));

		if (ch->over_enemies->empty())
		{
			sprintf(buf + strlen(buf), "You currently have no entries on your overwatch list.\n");
			send_to_char(buf, ch);
		}
		else
		{
			sprintf(buf + strlen(buf), "You currently have the following entries on your overwatch list:\n");
			vector<overwatch_enemy*>::iterator it;
			for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
			{
				sprintf(buf + strlen(buf), "   %d: #5%s#0, via #6%s#0.\n",
					ind,
					(*it)->sdesc.c_str(),
					((*it)->source == 0 ? "manual addition" : (*it)->source == 1 ? "leader direction" : (*it)->source == 2 ? "shots traded" : "shots traded between groups"));
				ind++;
			}
			send_to_char(buf, ch);
		}
		return;
	}
	else if (!str_cmp(arg1, "clear"))
	{
		if (ch->over_enemies->empty())
		{
			send_to_char("You currently have no entries on your overwatch list to clear.\n", ch);
		}
		else if (!*argument)
		{
			remove_all_overwatch(ch);

			send_to_char("Your overwatch list has been cleared.\n", ch);

		}
		else
		{
			vector<overwatch_enemy*>::iterator it;
			for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
			{
				if (atoi(argument) == ind)
				{
					sprintf(buf, "You clear entry #6%d#0, #5%s#0, from your overwatch list.\n", ind, (*it)->sdesc.c_str());
					delete *it;
					ch->over_enemies->erase(it);
					send_to_char(buf, ch);
					return;
				}
				ind++;
			}

			sprintf(buf, "There isn't an entry numbered #5%s#0 on your overwatch list.\n", argument);
			send_to_char(buf, ch);
		}
		return;
	}
	else if (!str_cmp(arg1, "share"))
	{
		// todo
	}
	else
	{
		argument = one_argument(argument, arg2);
		// If our arg1 is n, e, s, w, d or u, we're ranged baby.
		if ((!strn_cmp ("east", arg1, strlen (arg1)) ||
			!strn_cmp ("west", arg1, strlen (arg1)) ||
			!strn_cmp ("north", arg1, strlen (arg1)) ||
			!strn_cmp ("south", arg1, strlen (arg1)) ||
			!strn_cmp ("up", arg1, strlen (arg1)) ||
			!strn_cmp ("down", arg1, strlen (arg1))) && *arg2)
		{
			ranged = 1;
		}

		if (!ranged)
		{
			if (!(target = get_char_room_vis (ch, arg1)))
			{
				send_to_char ("Who did you want to target?\n", ch);
				return;
			}
			if (target == ch)
			{
				send_to_char ("Well, I suppose we are all our own worst enemies...\n", ch);
				return;
			}
			if (!CAN_SEE(ch, target) && IS_MORTAL (ch))
			{
				send_to_char ("Who did you want to target?\n", ch);
				return;
			}

			if (target->following == ch)
			{
				argument = one_argument(argument, arg3);
				act ("You order $N to add an overwatch.", false, ch, 0, target, TO_CHAR);
				act ("$n orders you to add an overwatch.", false, ch, 0, target, TO_VICT);

				if (target->over_target > 0)
				{
					sprintf(buf, "overwatch %s", argument);
					command_interpreter(target, buf);
				}
				return;
			}

			// Don't want to be accidentally targeting group mates.
			if (are_grouped(ch, target) && *argument != '!')
			{
				sprintf (buf, "Are you sure you want to add $N, a group member, to your overwatch list?"
					"To confirm, type \'#6o %s !#0\', without the quotes.", original);
				act (buf, false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
				return;
			}

			vector<overwatch_enemy*>::iterator it;
			for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
			{
				if ((*it)->sdesc == char_short(target))
				{
					delete *it;
					ch->over_enemies->erase(it);
					sprintf(buf, "You remove #5%s#0 from your overwatch list.", char_short(target));
					act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
			}

			OVERWATCH_ENEMY *enemy = new OVERWATCH_ENEMY;
			enemy->sdesc = char_short(target);
			enemy->source = 0;
			enemy->time = 4;
			ch->over_enemies->push_back(enemy);
			sprintf(buf, "You add #5%s#0 to your overwatch list.", char_short(target));
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else if (ranged)
		{
			if (!strn_cmp ("north", arg1, strlen (arg1)))
				dir = 0;
			else if (!strn_cmp ("east", arg1, strlen (arg1)))
				dir = 1;
			else if (!strn_cmp ("south", arg1, strlen (arg1)))
				dir = 2;
			else if (!strn_cmp ("west", arg1, strlen (arg1)))
				dir = 3;
			else if (!strn_cmp ("up", arg1, strlen (arg1)))
				dir = 4;
			else if (!strn_cmp ("down", arg1, strlen (arg1)))
				dir = 5;

			if (!EXIT (ch, dir))
			{
				send_to_char ("There isn't an exit in that direction.\n", ch);
				return;
			}

			ch->delay_who = str_dup (dirs[dir]);

			room = vnum_to_room (EXIT (ch, dir)->to_room);

			exit = EXIT (ch, dir);
			if (exit
				&& IS_SET (exit->exit_info, EX_CLOSED)
				&& !IS_SET (exit->exit_info, EX_ISGATE))
			{
				send_to_char ("Your view is blocked.\n", ch);
				return;
			}

			if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
			{
				exit = room->dir_option[dir];
				if (!exit)
				{
					send_to_char ("You don't see them.\n", ch);
					return;
				}

				if (exit
					&& IS_SET (exit->exit_info, EX_CLOSED)
					&& !IS_SET (exit->exit_info, EX_ISGATE))
				{
					send_to_char ("Your view is blocked.\n", ch);
					return;
				}

				if (room->dir_option[dir])
					room = vnum_to_room (room->dir_option[dir]->to_room);
				else
					room = NULL;

				if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
				{
					exit = room->dir_option[dir];
					if (!exit)
					{
						send_to_char ("You don't see them within range.\n", ch);
						return;
					}

					if (exit
						&& IS_SET (exit->exit_info, EX_CLOSED)
						&& !IS_SET (exit->exit_info, EX_ISGATE))
					{
						send_to_char ("Your view is blocked.\n", ch);
						return;
					}

					if (room->dir_option[dir])
						room = vnum_to_room (room->dir_option[dir]->to_room);
					else
						room = NULL;

					if (is_sunlight_restricted (ch, room))
						return;

					if (!(target = get_char_room_vis2 (ch, room->vnum, arg2)) || !has_been_sighted (ch, target))
					{
						exit = room->dir_option[dir];
						if (!exit)
						{
							send_to_char ("You don't see them within range.\n", ch);
							return;
						}

						if (exit
							&& IS_SET (exit->exit_info, EX_CLOSED)
							&& !IS_SET (exit->exit_info, EX_ISGATE))
						{
							send_to_char ("Your view is blocked.\n", ch);
							return;
						}

						send_to_char ("You don't see them within range.\n", ch);
						return;
					}
				}
			}

			if (!target || !CAN_SEE (ch, target) || !has_been_sighted (ch, target))
			{
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}

			if (target == ch)
			{
				send_to_char ("Well, I suppose we are all our own worst enemies...\n", ch);
				return;
			}

			if (!CAN_SEE(ch, target) && IS_MORTAL (ch))
			{
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}

			vector<overwatch_enemy*>::iterator it;
			for (it = ch->over_enemies->begin(); it != ch->over_enemies->end(); it++)
			{
				if ((*it)->sdesc == char_short(target))
				{
					delete *it;
					ch->over_enemies->erase(it);
					sprintf(buf, "You remove #5%s#0 from your overwatch list.", char_short(target));
					act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
			}

			OVERWATCH_ENEMY *enemy = new OVERWATCH_ENEMY;
			enemy->sdesc = char_short(target);
			enemy->source = 0;
			enemy->time = 4;
			ch->over_enemies->push_back(enemy);
			sprintf(buf, "You add #5%s#0 to your overwatch list.", char_short(target));
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}
}

int
	calculate_firearm_result (CHAR_DATA * ch, int ch_skill, int att_modifier,
	CHAR_DATA * target, int def_modifier,
	OBJ_DATA * firearm, OBJ_DATA * bullet,
	AFFECTED_TYPE * spell, int *location, float *damage, int *poison, float *bleed, OBJ_DATA **armor, int direction, bool pointblank, bool fighting, int dam_mod, int cmd)
{
	//*armor1 = NULL, *armor2 = NULL;
	int roll = 0, defense = 0, assault = 0, result = 0;
	int hit_type = 0;
	int cover = 0;
	int att_roll = 0;
	int type = 0;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *taf = NULL;
	OBJ_DATA *obj = NULL;

	/* Determine result of hit attempt. */

	//if (!CAN_SEE (target, ch))
	//  def_modifier += number (15, 30);

	roll = number (1, SKILL_CEILING);

	// standing out in the open is 5~15 depending on terrain type%.
	// sitting is flat 20,
	// resting is flat 30,
	// anything else not fighting is 85.

	if (GET_POS(target) == POSITION_SITTING)
	{
		cover = 20;
	}
	else if (GET_POS(target) == POSITION_RESTING)
	{
		cover = 30;
	}
	else if (GET_POS(target) == POSITION_FIGHTING || GET_POS(target) == POSITION_STANDING)
	{
		cover = 5;
	}
	else
	{
		cover = 85;
	}

	if (direction == 6 && cover > 20)
		cover = 20;


	// light-cover is 85%
	// medium-cover is 95%
	// hard-cover or hunkered is 100%.
	//
	// In general, you need to roll critical successes or fire a lot of bullets to have a shot at hitting someone from cover,
	// or flank and hit them at a position where they're not in cover. OMG TACTICS!

	for (af = target->hour_affects; af; af = af->next)
	{
		if (af->type == AFFECT_COVER)
		{
			if (af->a.cover.direction == direction || direction == 6)
			{
				break;
			}
		}
	}

	if (af)
	{
		if (af->a.cover.value >= 3)
			cover = 125;
		else if (af->a.cover.value == 2)
			cover = 100;
		else if (af->a.cover.value == 1)
			cover = 95;
		else
			cover = 90;

		// If you've just fired or acted from cover, it gives you a litle less than you might otherwise get.
		if (af->a.cover.temp >= 1)
		{
			if (af->a.cover.value == 3)
				cover -= 30;
			else
				cover -= 10;
		}

		// Shooting someone from the same bit of cover negates all advantages.
		if ((taf = get_affect(ch, AFFECT_COVER)))
		{
			if (taf->a.cover.obj && af->a.cover.obj == taf->a.cover.obj)
				cover = 0;
		}
	}

	// If we're fighting them, no chance for cover.
	if (ch->fighting == target || target->fighting == ch)
	{
		cover = 0;
	}
	// Else, if they're moving, we modify the attacking chance
	// depending how fast they're going. Sprinters are hard to hit.
	else if (target->moves)
	{
		if (target->moves->flags && IS_SET(target->moves->flags, MF_CRAWL))
		{
			cover = 30;
		}
		else
		{
			switch (target->speed)
			{
			case 3:         // jogging
				att_modifier += 10;
				break;
			case 4:         // running
				att_modifier += 15;
				break;
			case 5:         // sprinting
				att_modifier += 20;
				break;
			case 0:         // crawling
				att_modifier += 5;
			default:         // walking
				break;
			}
			cover = 5;
		}
	}

	if (roll > cover)
	{
		if (roll % 5 == 0 || roll == 1)
			defense = RESULT_CF;
		else
			defense = RESULT_MF;
	}
	else
	{
		if (roll % 5 == 0)
			defense = RESULT_CS;
		else
			defense = RESULT_MS;
	}

	//std::string gods = "Cover: " + MAKE_STRING(cover) + " Roll: " + MAKE_STRING(roll) + " Result: " + MAKE_STRING(defense);
	//send_to_gods(gods.c_str());

	// Weather effects, only affect bows, but it does affect it A LOT
	// if (weather_info[ch->room->zone].fog == THIN_FOG)
	//  att_modifier += 5;

	// If you're shooting at someone in a forest or a wood, and you're not in the same room
	// as they are, take a big hit to your chances.

	/*
	if (target->room->sector_type ==SECT_WOODS && target->room->nVirtual != ch->room->nVirtual)
	att_modifier += 15;
	if (target->room->sector_type ==SECT_FOREST && target->room->nVirtual != ch->room->nVirtual)
	att_modifier += 25;
	*/

	// If they're in a small room, and not in cover, it's a bit easier to hit them.
	if (cover <= 5 && IS_SET(target->room->room_flags, SMALL_ROOM))
	{
		att_modifier -= 10;
	}
	// Likewise, if they're in a big room, and not in cover, it's a bit harder to hit them.
	else if (cover <= 5 && IS_SET(target->room->room_flags, BIG_ROOM))
	{
		att_modifier += 5;
	}

	// Random modifier
	roll = number (1, SKILL_CEILING);
	roll += att_modifier;
	roll = MIN (roll, SKILL_CEILING);

	if (!pointblank && get_second_affect (ch, SA_POINTSTRIKE, NULL))
	{
		switch (get_second_affect(ch, SA_POINTSTRIKE, NULL)->info2)
		{
		case HITLOC_HILEGS:
		case HITLOC_HIARMS:
		case HITLOC_LOLEGS:
		case HITLOC_LOARMS:
			roll = roll * 1.2;
			break;
		case HITLOC_HANDS:
		case HITLOC_FEET:
			roll = roll * 2.5;
		case HITLOC_HEAD:
		case HITLOC_EYES:
		case HITLOC_NECK:
			roll += 5;
			roll = roll * 3.0;
		default:
			roll = roll * 1;
			break;
		}
	}

	att_roll = (MAX(MIN(50, skill_level(ch, SKILL_AIM, 0)), skill_level(ch, ch_skill, 0)) + skill_level(ch, SKILL_AIM, 0)) / 2;

	if (roll > att_roll)
	{
		if (roll % 5 == 0 || roll == 1)
			assault = RESULT_CF;
		else
			assault = RESULT_MF;
	}
	else
	{
		if (roll % 5 == 0)
			assault = RESULT_CS;
		else
			assault = RESULT_MS;
	}

	//std::string gods2 = "Att_roll: " + MAKE_STRING(att_roll) + " Roll: " + MAKE_STRING(roll - att_modifier) + " Modifier: " + MAKE_STRING(att_modifier) + " Result: " + MAKE_STRING(assault);
	//send_to_gods(gods2.c_str());

	if (assault == RESULT_CS)
	{
		if (defense == RESULT_CS)
		{
			if (number(0,9))
				result = COVER_HIT;
			else
				result = GLANCING_HIT;
		}
		else if (defense == RESULT_MS)
			result = GLANCING_HIT;
		else if (defense == RESULT_MF)
			result = HIT;
		else if (defense == RESULT_CF)
			result = CRITICAL_HIT;
	}
	else if (assault == RESULT_MS)
	{
		if (defense == RESULT_CS)
			result = COVER_HIT;
		else if (defense == RESULT_MS)
		{
			if (number(0,9))
				result = COVER_HIT;
			else
				result = GLANCING_HIT;
		}
		else if (defense == RESULT_MF)
			result = HIT;
		else if (defense == RESULT_CF)
			result = CRITICAL_HIT;
	}
	else if (assault == RESULT_MF)
	{
		if (defense == RESULT_CS)
			result = MISS;
		else if (defense == RESULT_MS)
			result = MISS;
		else if (defense == RESULT_MF)
			result = MISS;
		else if (defense == RESULT_CF)
			result = GLANCING_HIT;
	}
	else
	{
		if (defense == RESULT_CS)
			result = MISS;
		else if (defense == RESULT_MS)
			result = MISS;
		else if (defense == RESULT_MF)
			result = MISS;
		else
			result = MISS;
	}

	// We also critical hit at point blank.
	if (pointblank)
		result = CRITICAL_HIT;

	// Fighting, it's a toss-up as to what we do:
	// If you've got a pistol, it's 10% crit, 20% hit, 30% glance, the rest miss.
	// If you don't have a pistol, it's 10% crit, 10% hit, 30% glance, the rest miss.
	if (fighting)
	{
		int melee_roll = number(0,9);

		switch (melee_roll)
		{
		case 0:
		case 1:
			result = CRITICAL_HIT;
			break;
		case 2:
			result = HIT;
			break;
		case 3:
			if (firearm->o.firearm.handedness == 2)
				result = HIT;
			else
				result = GLANCING_HIT;
			break;
		case 4:
		case 5:
			result = GLANCING_HIT;
			break;
		case 6:
			if (firearm->o.firearm.handedness == 2)
				result = GLANCING_HIT;
			else
				result = MISS;
			break;
		default :
			result = MISS;
			break;
		}
	}
	// Alternatively, if we're firing at someone who's approaching to attack us...
	else if (cmd == 2)
	{
		int approach_roll = number(0,9);

		switch (approach_roll)
		{
		case 0:
		case 1:
			result = CRITICAL_HIT;
			break;
		case 2:
		case 3:
		case 4:
			if (firearm->o.firearm.handedness == 2)
				result = CRITICAL_HIT;
			else
				result = HIT;
			break;
		case 5:
		case 6:
		case 7:
			if (firearm->o.firearm.handedness == 2)
				result = HIT;
			else
				result = GLANCING_HIT;
			break;
		case 8:
		case 9:
			if (firearm->o.firearm.handedness == 2)
				result = GLANCING_HIT;
			else
				result = MISS;
			break;
		default :
			result = MISS;
			break;
		}
	}

	if (result == MISS || result == CRITICAL_MISS || result == COVER_HIT)
		return result;

	/* Determine damage of hit, if applicable. */

	*damage = 0;

	roll = number (1, 100);
	*location = -1;

	while (roll > 0)
		roll = roll - body_tab[1][++(*location)].percent;

	if (get_second_affect(ch, SA_POINTSTRIKE, NULL))
	{
		*location = get_second_affect(ch, SA_POINTSTRIKE, NULL)->info2;
	}

	for (obj = target->equip; obj; obj = obj->next_content)
	{
		if (GET_ITEM_TYPE(obj) == ITEM_ARMOR && IS_SET(obj->o.od.value[2], 1 << *location))
			break;
	}

	if (!obj)
	{
		for (obj = target->equip; obj; obj = obj->next_content)
		{
			if (GET_ITEM_TYPE(obj) == ITEM_ARMOR && IS_SET(obj->o.od.value[3], 1 << *location))
				break;
		}
	}

	if (obj)
		*armor = obj;


	// Slings get handled differently.
	if (IS_SLING(firearm))
	{
		hit_type = 3;
		*bleed = 0;

		if (result == HIT)
		{
			*damage = number(2,3) + 1;
		}
		else if (result == CRITICAL_HIT)
		{
			*damage = number(3,3) + 1;
			result = HIT;
		}
		else
		{
			*damage = number(1,3) + 1;
			result = GLANCING_HIT;
		}

		*damage = real_damage(target, *damage, location, hit_type, 0);
		*damage *= (body_tab[1][*location].damage_mult * 1.0) / (body_tab[1][*location].damage_div * 1.0);
	}
	else
	{

		hit_type = 7;

		type = bullet->o.bullet.pierce;

		if (result == HIT)
		{
			*bleed = dice (3, bullet->o.bullet.sides);
			if (number(0,fabs(bullet->o.bullet.damage)))
			{
				*bleed += bullet->o.bullet.damage;
			}
			*damage = number(4,6);

		}
		else if (result == CRITICAL_HIT)
		{
			if ((type == 0 && number(0,1)) || type == 1)
			{
				*bleed = dice (4, bullet->o.bullet.sides);
				*bleed += bullet->o.bullet.damage;
				result = SHATTER_HIT;
			}
			else
			{
				*bleed = dice (3, bullet->o.bullet.sides);
				if (number(0,fabs(bullet->o.bullet.damage)))
				{
					*bleed += bullet->o.bullet.damage;
				}
				result = PUNCTURE_HIT;
			}
			*damage = number(7,9);
		}
		else
		{
			*bleed = dice (1, bullet->o.bullet.sides);
			if (number(0,fabs(bullet->o.bullet.damage)))
			{
				*bleed += bullet->o.bullet.damage;
			}
			if (*bleed > 5)
				*bleed = 5;

			*damage = number(1,3);
		}

		*bleed += dam_mod;
		*damage += dam_mod;

		*bleed = real_damage(target, *bleed, location, 7 + type, 0);
		*bleed *= (body_tab[1][*location].damage_mult * 1.0) / (body_tab[1][*location].damage_div * 1.0);

		// If you've been shot in the head or eye, then damage equals bleeding.
		if (*location == 3 || *location == 7)
		{
			*damage = *bleed;
		}

		// Armour has caught the bullet - take some concussive damage.
		if (*bleed <= 0)
		{
			hit_type = 3;
			*bleed = 0;
			*damage = number(4,6);
			if (*armor)
			{
				result = CAUGHT;
			}
			else
			{
				result = HIT;
			}
		}
		// If we've reduced our damage enough, then we've back down to a glancing hit.
		else if (*bleed <= 5 && (result == HIT || result == SHATTER_HIT || result == PUNCTURE_HIT || result == CRITICAL_HIT))
		{
			*bleed = dice (1, bullet->o.bullet.sides);
			if (!number(0,fabs(bullet->o.bullet.damage)))
			{
				*bleed += bullet->o.bullet.damage;
			}
			*bleed = 1;

			*damage = number(1,3);
			result = GLANCING_HIT;
		}
		// If we've still got a real hit, and we used soft-points,
		// we do four more bleed damage.
		else if (type == 1)
		{
			*bleed += 4;
		}
		// Likewise, if we used armour piercing, we've got 2 less bleed damage.
		else if (type == 2)
		{
			*bleed -= 4;
		}
	}

	if (*bleed < 0)
		*bleed = 0;

	if (*damage < 0)
		*damage = 0;

	// If we did damage, then it needs to affect our armour or clothing.
	damage_objects(target, number(1,10), "", *location, IS_SLING(firearm) ? 10 : 9, 1, false);

	return result;
}


void firearm_bang (ROOM_DATA *room, int fired, int target_room)
{
	int num_rooms = 0;
	int dists[1300];
	int i = 0;
	int dir = -1;
	ROOM_DATA *rooms[1300];
	CHAR_DATA *tch = NULL;
	char buf[AVG_STRING_LENGTH * 2] = { '\0' };
	char buf2[AVG_STRING_LENGTH] = { '\0' };

	for (i = 0; i <= 1300; i++)
		dists[i] = 0;
	for (i = 0; i <= 1300; i++)
		rooms[i] = NULL;

	get_room_list (4, room, rooms, dists, &num_rooms);

	if (fired == 1)
	{
		sprintf(buf2, "gunshot");
	}
	else if (fired < 7)
	{
		if (!number(0,4))
			sprintf(buf2, "burst of gunfire");
		else if (!number(0,3))
			sprintf(buf2, "rattle of gunfire");
		else if (!number(0,2))
			sprintf(buf2, "blast of gunfire");
		else if (!number(0,1))
			sprintf(buf2, "stacatto of gunfire");
		else
			sprintf(buf2, "uproar of gunfire");
	}
	else if (fired >= 7)
	{
		if (!number(0,4))
			sprintf(buf2, "cacophony of gunfire");
		else if (!number(0,3))
			sprintf(buf2, "barrage of gunfire");
		else if (!number(0,2))
			sprintf(buf2, "terrible din of gunfire");
		else if (!number(0,1))
			sprintf(buf2, "explosion of gunfire");
		else
			sprintf(buf2, "uproar of gunfire");
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, room->vnum);

			if (dir == -1)
				continue;

			if (rooms[i]->vnum == target_room)
				continue;

			sprintf (buf, "You hear a %s from the %s.\n", buf2, dirs[dir]);

			for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
			{
				if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
				{
					if (get_second_affect(tch, SA_DISTSHOTS, 0))
					{
						get_second_affect(tch, SA_DISTSHOTS, 0)->info2+= fired;
					}
					else
					{
						add_second_affect(SA_DISTSHOTS, 3, tch, 0, 0, fired);
					}
				}
				else
				{
					send_to_char(buf, tch);
				}
			}
		}
	}
}

void
	do_firearm_fire (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *target = NULL;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *xch = NULL;
	CHAR_DATA *fired_ch[25], *retal_ch = NULL;
	OBJ_DATA *ammo[13];
	OBJ_DATA *tobj = NULL;
	for (int ind = 0; ind < 13; ind++)
	{
		ammo[ind] = NULL;
	}
	OBJ_DATA *firearm = NULL;
	OBJ_DATA *clip = NULL;
	OBJ_DATA *shell = NULL;
	OBJ_DATA *bullet = NULL;
	OBJ_DATA *shield_obj = NULL;
	OBJ_DATA *cover = NULL;
	ROOM_DATA *room;
	ROOM_DIRECTION_DATA *exit = NULL;
	int old_result = 0;
	ROOM_DATA *next_room = NULL;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *xaf = NULL;
	AFFECTED_TYPE *target_af = NULL;
	AFFECTED_TYPE *group_af = NULL;
	char arg[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	char buf4[MAX_STRING_LENGTH] = {'\0'};
	char buf5[MAX_STRING_LENGTH] = {'\0'};
	char buf6[MAX_STRING_LENGTH] = {'\0'};
	char buf7[MAX_STRING_LENGTH] = {'\0'};
	char buffer[MAX_STRING_LENGTH] = {'\0'};
	char *from_direction = NULL;
	char *p;
	int ranged = 0, num_fired_ch = 0, i = 0;
	int attack_mod = 0;
	int fired = 1;
	// "auto" fire lets loose between 7 and 13 bullets, so we need to calculate the results.
	int res_result[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int res_location[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float res_damage[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float res_bleed[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char *location_table[13] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
	OBJ_DATA *res_armor1[13] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	int result_table[7] = {0, 0, 0, 0, 0, 0, 0};
	int dir = 0;
	bool switched_target = false;
	bool loop_skip = false;
	bool any_left = false;
	int poison = 0;
	int was_hidden = 0;
	int direction = -1;
	bool pointblank = false;
	int wild = 0;
	bool jam = false;
	int dam_mod = 0;
	LODGED_OBJECT_INFO *clodged = NULL;

	char original[MAX_STRING_LENGTH];
	sprintf (original, "%s", argument);

	argument = one_argument(argument, arg);

	//if (!str_cmp (arg, "volley"))
	//{
	//	argument = one_argument(argument, arg);
	//	if (!is_group_leader (ch))
	//	{
	//		send_to_char ("You are not currently leading a group.\n", ch);
	//		return;
	//	}
	//	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	//	{
	//		if ((tch == ch || are_grouped (tch, ch)) && tch->aiming_at && is_archer (tch))
	//			block = false;
	//	}
	//	if (block)
	//	{
	//		send_to_char ("No one in your group is currently preparing to fire.\n", ch);
	//		return;
	//	}

	//	act ("You give your group the signal to fire.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	//	act ("$n gives the signal to fire!", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	//	i = 0;

	//	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	//	{
	//		if (tch == ch)
	//			continue;
	//		if (!are_grouped (tch, ch))
	//			continue;
	//		if (!tch->aiming_at || !is_archer (tch))
	//			continue;
	//		send_to_room ("\n", ch->in_room);
	//		if (num_fired_ch < 25)
	//			fired_ch[num_fired_ch] = tch;
	//		num_fired_ch++;
	//		do_firearm_fire (tch, "", 1);
	//	}

	//	num_fired_ch = MIN (num_fired_ch, 24);

	//	fired_ch[num_fired_ch] = ch;

	//	// Leader doesn't need to fire if victim dies.
	//	if (!ch->aiming_at)
	//		return;

	//	send_to_room ("\n", ch->in_room);
	//}
	//else
		fired_ch[0] = ch;

	if (IS_SWIMMING (ch))
	{
		send_to_char ("You can't do that while swimming!\n", ch);
		return;
	}

	if (IS_FLOATING (ch))
	{
		send_to_char ("You can't do that while floating!\n", ch);
		return;
	}
	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if (IS_SET(ch->room->room_flags, PEACE))
	{
		act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
		return;
	}

	/*
	if (((bow = get_equip (ch, WEAR_PRIM)) && GET_ITEM_TYPE (bow) == ITEM_WEAPON
	&& bow->o.weapon.use_skill == SKILL_SLING)
	|| ((bow = get_equip (ch, WEAR_SEC))
	&& GET_ITEM_TYPE (bow) == ITEM_WEAPON
	&& bow->o.weapon.use_skill == SKILL_SLING))
	{
	skill_learn(ch, SKILL_SLING);
	fire_sling (ch, bow, argument);
	return;
	}
	*/

	// If we've got an object wielded in our right hand that is of type firearm, that's what we're looking for.
	if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_FIREARM &&
		(ch->right_hand->location == WEAR_BOTH || ch->right_hand->location == WEAR_PRIM || ch->right_hand->location == WEAR_SEC))
	{
		firearm = ch->right_hand;
	}

	// If we've got a firearm in our left hand...
	if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_FIREARM &&
		(ch->left_hand->location == WEAR_BOTH || ch->left_hand->location == WEAR_PRIM || ch->left_hand->location == WEAR_SEC))
	{
		// ...and we've already got a firearm i.e. one in our right hand...
		if (firearm)
		{
			// If our firearm in our right hand is wielded primary,
			if (firearm->location == WEAR_PRIM)
			{
				// And our secondary delay is less than our primary delay
				if (ch->secondary_delay < ch->primary_delay)
				{
					// Swap our firearm to our left hand.
					firearm = ch->left_hand;
				}
			}
			// Or if our firearm in our left hand is wielded secondary, vice versa.
			else
			{
				if (ch->primary_delay < ch->secondary_delay)
				{
					firearm = ch->left_hand;
				}
			}
		}
		// And we didn't have one in our right hand, go on.
		else
		{
			firearm = ch->left_hand;
		}
	}

	if (!firearm)
	{
		send_to_char ("You aren't wielding a ranged weapon!\n", ch);
		broke_aim(ch, 0);
		return;
	}

	if (IS_SLING(firearm) && !ch->aiming_at)
	{
		send_to_char ("You need to aim and pull back on your sling before you can fire.\n", ch);
		return;
	}

	if (firearm->o.firearm.setting == 2)
	{
		fired = 3;
	}
	else if (firearm->o.firearm.setting == 3)
	{
		fired = number(7,13);
	}
	else
	{
		fired = 1;
	}

	// Obviously, if we have less bullets in the gun we want to fire, we fail.

	if (count_bullets(firearm) < fired)
	{
		fired = count_bullets(firearm);
	}

	// You can do "shoot person" to shoot at someone in the room, but it's a snap-shot so no aim bonus.
	if (*arg && str_cmp(arg, "!"))
	{
		// If our arg1 is n, e, s, w, d or u, we're ranged baby.
		if (!strn_cmp ("east", arg, strlen (arg)) ||
			!strn_cmp ("west", arg, strlen (arg)) ||
			!strn_cmp ("north", arg, strlen (arg)) ||
			!strn_cmp ("south", arg, strlen (arg)) ||
			!strn_cmp ("up", arg, strlen (arg)) ||
			!strn_cmp ("down", arg, strlen (arg)) ||
			!str_cmp("ground", arg) ||
			!str_cmp("floor", arg) ||
			!str_cmp("sky", arg) ||
			!str_cmp("air", arg))
		{
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
			else
				dir = 6;

			if (!EXIT (ch, dir) && dir != 6)
			{
				send_to_char ("There isn't an exit in that direction.\n", ch);
				return;
			}

			if (dir <= 5 && dir >= 0)
			{
				room = vnum_to_room (EXIT (ch, dir)->to_room);

				if (room)
				{
					wild = 1;
					target = ch;
				}
				else
				{
					send_to_char ("An error was found in trying to shoot in to the room - please let an admin know.\n", ch);
					return;
				}
			}
			else
			{
				wild = 2;
				target = ch;
			}
			ranged = 0;
			broke_aim(ch, 0);
		}
		else
		{
			xch = ch->aiming_at;
			if (!(ch->aiming_at = get_char_room_vis (ch, arg)))
			{
				send_to_char ("You don't see that target here.\n", ch);
				broke_aim(ch, 0);
				return;
			}

			if (ch->aiming_at == ch)
			{
				send_to_char
					("Now, now, now... things can't be THAT bad, can they?\n", ch);
				return;
			}

			add_targeted(ch->aiming_at, ch);

			// If we didn't change our aim, we don't need to recalculate our penalty.
			if (!(xch == ch->aiming_at))
			{
				ch->aim = aim_penalty(ch, firearm, ch->delay_info1);
				// If we've changed aim, we need to remove our pointstrike.
				if (get_second_affect (ch, SA_POINTSTRIKE, NULL))
				{
					remove_second_affect(get_second_affect (ch, SA_POINTSTRIKE, NULL));
				}
			}
			if (!ch->aiming_at)
			{
				send_to_char ("You aren't aiming at anything.\n", ch);
				return;
			}
		}
	}


	if ((af = get_affect(ch, AFFECT_COVER)))
	{
		if (af->a.cover.value != 0 && af->a.cover.value != 3)
		{
			af = NULL;
		}
		else
		{
			for (xaf = ch->hour_affects; xaf; xaf = xaf->next)
			{
				if (xaf->type == AFFECT_COVER && (xaf->a.cover.value == 0 || xaf->a.cover.value == 3))
				{
					xaf->a.cover.temp = 3;
				}
			}
		}
	}

	if (IS_DIRECT(firearm))
	{

		for (tobj = firearm->contains; tobj; tobj = tobj->next_content)
		{
			if (GET_ITEM_TYPE(tobj) == ITEM_ROUND)
			{
				ammo[0] = tobj;
				break;
			}
		}

		if ((ammo[0]) == NULL)
		{
			sprintf(buf, "You %ssqueeze the trigger of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %ssqueezes the trigger of $p, but nothing happens.\n", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
			broke_aim(ch, 0);
			return;
		}
	}
	else if (IS_SLING(firearm))
	{
		for (tobj = firearm->contains; tobj; tobj = tobj->next_content)
		{
			if (GET_ITEM_TYPE(tobj) == ITEM_ROUND)
			{
				ammo[0] = tobj;
				break;
			}
		}

		if ((ammo[0]) == NULL)
		{
			sprintf(buf, "You %srelease the pocket of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %sreleases the pocket of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT);
			broke_aim(ch, 0);
			return;
		}
	}
	else
	{
		if (!(clip = firearm->contains))
		{
			sprintf(buf, "You %ssqueeze the trigger of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %ssqueezes the trigger of $p, but nothing happens.\n", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
			broke_aim(ch, 0);
			return;
		}

		if (!(ammo[0] = clip->contains))
		{
			sprintf(buf, "You %ssqueeze the trigger of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %ssqueezes the trigger of $p, but nothing happens.\n", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
			broke_aim(ch, 0);
			return;
		}
	}

	int counts = 1;
	int last = 0;
	for (int ind = 1; ind < fired; ind++)
	{
		if (ammo[last]->count > counts)
		{
			ammo[ind] = ammo[last];
			counts++;
		}
		else if (ammo[last]->next_content && GET_ITEM_TYPE(ammo[last]->next_content) == ITEM_ROUND)
		{
			counts = 1;
			ammo[ind] = ammo[last]->next_content;
			last = ind;
		}
	}

	if (!wild && !ch->aiming_at)
	{
		send_to_char ("You aren't aiming at anything.\n", ch);
		return;
	}
	else if (ch->aiming_at && (!ch->aiming_at->in_room || !ch->aiming_at->room))
	{
		send_to_char ("Your target is no longer present.\n", ch);
		broke_aim(ch, 1);
		return;
	}

    if (ch->balance <= -15)
    {
        send_to_char ("You're far too off-balance to attempt to hit anything but the air.\n", ch);
        return;
    }

	if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
		&& IS_SET (ch->room->room_flags, LAWFUL) && *arg != '!')
	{
		sprintf (buf, "You are in a lawful area; you would likely be flagged wanted for assault. "
			          "To confirm, type \'#6fire !#0\', without the quotes.");
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (firearm->o.firearm.setting == 1 || firearm->o.firearm.setting < 0)
	{
		sprintf(buf, "You %ssqueeze the trigger of $p, but nothing happens.", (af ? "rises from cover and " : ""));
		act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
		sprintf(buf2, "$n %ssqueezes the trigger of $p, but nothing happens.\n", (af ? "rises from cover and " : ""));
		act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
		broke_aim(ch, 0);
		return;
	}

	// Shooting without aiming thoroughly fucks you up - for every second you didn't wait, you get a 6% penalty,
	// for a maximum of 10 seconds, but also determined by your initial aim result.
	// In short, someone with 40 aim and 60 skill will have a base of 4.

	attack_mod += (99 - (9 * ch->aim));

	// If you are fully aimed, however, you get a bit of a bonus.
	if (ch->aim >= 20)
		pointblank = true;
	else if (ch->aim == 11)
		attack_mod -= 10;

	// Modify our difficuly by weapon 10 for each level we've dropped -
	// need to take care of yo weapon, fool.
	attack_mod += object__determine_condition(firearm) * 10;

	if (!wild)
	{
		add_targeted(target, ch);
		target = ch->aiming_at;
	}

	// Make sure people don't hit the wrong person.
	if (target && target != ch && are_grouped (ch, target) && is_brother (ch, target) && *argument != '!')
	{
		sprintf (buf, "#1You are about to attack $N #1who is a fellow group member!#0 To confirm, type \'#6fire %s !#0\', without the quotes.", original);
		act (buf, false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
		remove_targeted(target, ch);
		return;
	}

	//if (!cmd && !wild)
	//{
	//remove_targeted(target, ch);
	//}

	// Now we need to figure out how much our aim got kicked off...
	// we lose a second for every bullet we're firing, for a minimum of 1,
	// then we lose a second for the recoil raitng of the gun, for a minimum of 1.

	int recoil = fired;
	recoil = MAX(recoil, 1);

	// Handguns just have a bit less recoil in general.
	if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
	{
		recoil -= 1;
	}

	// Firing a pistol with two hands means less recoil
	if (firearm->o.firearm.handedness == 2 && firearm->location == WEAR_BOTH)
	{
		recoil -= 1;
		attack_mod -= 10;
	}
	// Likewise, firing a rifle or something one-handed means more recoil
	else if (firearm->o.firearm.handedness == 1 && firearm->location != WEAR_BOTH)
	{
		recoil += 2;
		attack_mod += 10;
	}
	// For two-handed wepaons, you really want to be holding them with both hands.
	else if (firearm->o.firearm.handedness == 3 && firearm->location != WEAR_BOTH)
	{
		recoil += 3;
		attack_mod += 20;
	}

	// Now we add on our recoil from the gun.
	recoil += firearm->o.firearm.recoil;

	// If we're not standing, we halve our recoil.
	if (GET_POS(ch) < STAND && firearm->location == WEAR_BOTH)
	{
		recoil = recoil / 2;
	}

	recoil = MAX(recoil, 1);
	ch->aim -= recoil;

	// Finally, our recoil is the highest of either our recoil-modded aim, or our hip-shooting - 1.
	// The -1 is to show the bitch that recoil is, and because no one who doesnt know about this will
	// stop and re-aim, and certainly won't do so in less than a second.
	ch->aim = MAX(ch->aim, aim_penalty(ch, firearm, ch->delay_info1) - 2);
	ch->aim = MIN(ch->aim, 10);

	// We also add to our combat delay - firing our weapon slows us down a fair bit.
	if (firearm->location == WEAR_SEC)
		ch->secondary_delay += 10;
	else
		ch->primary_delay += 10;

	ch->primary_delay = MIN(ch->primary_delay, 60);
	ch->secondary_delay = MIN(ch->secondary_delay, 60);

	if (!wild && IS_IMPLEMENTOR (target) && !IS_NPC (target) && !IS_NPC (ch))
		target = ch;

	if (!wild && ch->in_room != target->in_room)
	{
		ranged = 1;

		if (!ch->delay_info1)
			ch->delay_info1 = 1;
	}

	if (ranged)
	{
		if (!ch->delay_who)
		{
			send_to_char ("You seem to have lost sight of your quarry.\n", ch);
			broke_aim(ch, 0);
			return;
		}
		if (!strn_cmp ("north", ch->delay_who, strlen (ch->delay_who)))
			dir = 0;
		else if (!strn_cmp ("east", ch->delay_who, strlen (ch->delay_who)))
			dir = 1;
		else if (!strn_cmp ("south", ch->delay_who, strlen (ch->delay_who)))
			dir = 2;
		else if (!strn_cmp ("west", ch->delay_who, strlen (ch->delay_who)))
			dir = 3;
		else if (!strn_cmp ("up", ch->delay_who, strlen (ch->delay_who)))
			dir = 4;
		else if (!strn_cmp ("down", ch->delay_who, strlen (ch->delay_who)))
			dir = 5;
	}
	else
		dir = 6;

	if (get_affect (ch, MAGIC_HIDDEN))
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		send_to_char ("You emerge from concealment and prepare to fire.\n\n", ch);
		was_hidden = 1;
	}

	skill_learn(ch, firearm->o.firearm.use_skill);
	skill_learn(ch, SKILL_AIM);


	if (IS_SLING(firearm))
	{
		int skill_cap = ch->skills[SKILL_AIM];

		skill_use(ch, SKILL_AIM, 0);

		if (skill_cap == 50 && ch->skills[SKILL_AIM] > 50)
		{
			ch->skills[SKILL_AIM] = skill_cap;
		}

	}
	else
	{
		if (!wild && !pointblank)
			skill_use(ch, firearm->o.firearm.use_skill, 0);
		// Aiming happens less often as it's much more useful.
		if (!wild && !number(0,4) && !pointblank)
			skill_use(ch, SKILL_AIM, 0);
	}

	/* ranged_projectile_echoes (ch, target, bow, ammo); */

	// We add the amount of bullets we fire.
	object__add_damage (firearm, 1, fired);

	// We also add scent.
	if (!IS_SLING(firearm))
	{
		add_scent(firearm, scent_lookup("the acrid sting of cordite"), 1, (fired * 20), 1000, 0, 0);
		add_scent(ch->room, scent_lookup("the acrid sting of cordite"), 1, fired * 10, 1000, 0, 0);
		add_scent(get_equip(ch, WEAR_HANDS), scent_lookup("the acrid sting of cordite"), 1, fired * 10, 1000, 0, 0);
		add_scent(ch, scent_lookup("the acrid sting of cordite"), 1, fired * 10, 1000, 0, 0);
	}

	// Bonus modifier comes in skill rolls, not ovals.
	if (!ranged)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			attack_mod -= 20;
		else
			attack_mod -= 10;
	}
	// At one room, it's noitcable harder to hit with a handgun or shotgun
	else if (ch->delay_info1 == 1)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			attack_mod += 15;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			attack_mod += 10;
	}
	// At two rooms, bit harder, but noticably harder for handguns and shotguns.
	else if (ch->delay_info1 == 2)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			attack_mod += 30;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			attack_mod += 20;
		else
			attack_mod += 10;
	}
	// At three rooms, very hard for handguns and shotguns.
	else if (ch->delay_info1 >= 3)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			attack_mod += 50;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			attack_mod += 40;
		else
			attack_mod += 15;
	}
	else if (ranged)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			attack_mod += 50;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			attack_mod += 40;
		else
			attack_mod += 15;
	}


	// dam_mod depends on range: rifle bullets hit hard at
	// most ranges, while SMGs and Handguns get weak, encouraging
	// you to use the appropriate gun for the appropriate range.
	if (ranged >= 3)
	{
		if (firearm->o.firearm.use_skill == SKILL_RIFLE)
			dam_mod = -1;
		else if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			dam_mod = -3;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			dam_mod = -2;
	}
	else if (ranged == 2)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			dam_mod = -2;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			dam_mod = -1;
	}
	else if (ranged)
	{
		if (firearm->o.firearm.use_skill == SKILL_HANDGUN)
			dam_mod = -1;
		else if (firearm->o.firearm.use_skill == SKILL_SMG)
			dam_mod = -1;
	}

	if (is_mounted (ch))
		attack_mod +=15;

	// adjust target if it is grouped: much easier to hit big groups of people,
	// as archery is designed for.
	int group_size = 1;
	int group_mod = 0;
	int group_count = 0;

	bool cramp_involved = false;
	bool combat_involved = false; // Is there some combat going on here?
	bool table_involved = false;
	CHAR_DATA *original_target = target;

	if (!wild)
	{
		if (!pointblank && ch != target)
		{
			// If our target is sitting or taking cover at a table...
			if ((target_af = get_covtable_affect(original_target)))
			{
				// We add to our group for everyone else in the room also sitting
				// or taking cover at that table
				for (CHAR_DATA *grp = original_target->room->people; grp; grp = grp->next_in_room)
				{
					if (grp == original_target || grp == ch)
						continue;

					if ((group_af = get_covtable_affect(grp)) && group_af->a.table.obj == target_af->a.table.obj)
					{
						table_involved = true;

						group_size ++;
						group_count ++;

						if (number(1,group_count) == 1)
						{
							target = grp;
							if (target != original_target)
							{
								switched_target = true;
							}
						}
					}
				}
			}
			// Otherwise, we check other things that might exist...
			else
			{
				for (CHAR_DATA *grp = original_target->room->people; grp; grp = grp->next_in_room)
				{
					if (grp == original_target)
						continue;

					// If someone in the room is combat-involved with our target,
					// then we break out of this and do a new routine to determine
					// our group sizes.
					if (is_involved(grp, original_target, 0))
					{
						combat_involved = true;
						grp = NULL;
						break;
					}

					if (grp == ch)
						continue;

					// So, we haven't focused on their group at the table, or their combat group,
					// so let's now just tally people following them who aren't directly involved
					// in a combat or sitting around, but are in the same group.
					if (!get_covtable_affect(grp) && !grp->fighting && are_grouped(grp, original_target))
					{
						group_size ++;
						group_count ++;

						if (number(1,group_count) == 1)
						{
							target = grp;
							if (target != original_target)
							{
								switched_target = true;
							}
						}
					}
				}
			}

			// We count up everyone who's involved to get out group size.
			if (combat_involved)
			{
				group_count = 0;
				group_size = 0;
				for (CHAR_DATA *grp = original_target->room->people; grp; grp = grp->next_in_room)
				{
					if (grp == ch || grp == original_target)
						continue;

					if (is_involved(grp, original_target, 0))
					{
						group_size ++;
						group_count ++;

						if (number(1,group_count) == 1)
						{
							target = grp;
							if (target != original_target)
							{
								switched_target = true;
							}
						}
					}
				}
			}

			// If the room is cramped and we -still- haven't managed to hit anyone...
			if (IS_SET(target->room->room_flags, SMALL_ROOM) && !switched_target)
			{
				group_count = 0;
				group_size = 0;

				// We scan through everyone in the room, and if they're not otherwise grouped,
				// we mark them as a potential victim -- provided they aren't also grouped
				// with us (so we can't hit folks sitting at our table.
				for (CHAR_DATA *grp = original_target->room->people; grp; grp = grp->next_in_room)
				{
					if (grp == ch || grp == original_target || grp->in_room != original_target->in_room)
						continue;

					// Not theory grouped with either the target or us - no chance of hitting friends.
					if (!are_theory_grouped(grp, original_target) && !are_theory_grouped(grp, ch))
					{
						group_size ++;
						group_count ++;

						if (number(1,group_count) == 1)
						{
							target = grp;
							if (target != original_target)
							{
								cramp_involved = true;
								switched_target = true;
							}
						}
					}
				}

				// We've still got a group_size/10 chance of just ignoring cramped spaces all-together.
				// If howevere there are more than 10 people in the room, no such chance.
				if (number(0,(MIN(10-group_size,0))))
				{
					cramp_involved = false;
					switched_target = false;
					target = original_target;
				}
			}

			// If we've got more than one person involved in this group, we need to see who we hit.
			if (switched_target)
			{
				group_mod = MIN((group_size - 1),1) * 2 * (ranged + 1);

				if (combat_involved)
					group_mod = group_mod * 2;

				if (cramp_involved)
					group_mod = group_mod * 4;

				// If fail to roll a 1d100 greater than our skill minus the group_mod plus the att_mod, then we're back to
				// our original target.

				int x = number(1,100);
				int y = ((skill_level(ch, SKILL_AIM, 0) + skill_level(ch, firearm->o.firearm.use_skill, 0)) / 2) + attack_mod - group_mod;

				//std::string godsString = "Roll: " + MAKE_STRING(x) + " Target: " + MAKE_STRING(y);
				//send_to_gods(godsString.c_str());

				if (!(x >= y))
				{
					switched_target = false;
					target = original_target;
				}
				// If we've swapped targets, our to-hit is increased by the group_mod.
				else
				{
					if (combat_involved)
						group_mod = group_mod / 2;

					if (cramp_involved)
						group_mod = group_mod / 4;

					group_mod = group_mod / (ranged + 1);

					attack_mod -= group_mod;
				}
			}

			// If you're hidden, it's a lot harder for people to dodge your shot in calculate_missile_result... however, we removed our hidden flag at the start of this procedure so we could echo properly. Hence, the was_hidden flag.
			if (was_hidden)
			{
				if (!ranged)
				{
					attack_mod -= 20;
				}
				else
				{
					attack_mod -= 10;
				}
			}
		}
	}

	// If our quality - our damage is less than 800, we've got a chance of jamming.
	int jam_threshold = firearm->quality - object__count_damage(firearm);

	// If we're burst firing, we reduce our threshold some.
	if (fired > 1)
		jam_threshold -= 50;
	// Same if we're autofiring, only even more so.
	else if (fired >= 7)
		jam_threshold -= 100;

	// jam_threshold is also increased by half of certain conditions, up to a max of 100.
	jam_threshold -= MAX((firearm->enviro_conds[COND_DIRT] + firearm->enviro_conds[COND_DUST] + firearm->enviro_conds[COND_SLIME] + firearm->enviro_conds[COND_FILTH])/2, 100);

	// Now we spray for each bullet, yay.
	for (int ind = 0; ind < fired; ind++)
	{
		if (!IS_SLING(firearm) && !IS_DIRECT(firearm) && (jam_threshold < 800) && (number(1,100) <= ((9 - (jam_threshold / 100)) * 2)))
		{
			// So, we have a 2% chance of jamming for every hundred points or less
			// we're under that 800, e.g. 700~799 is 2%, 600~699 is 4%, and so on.
			firearm->o.firearm.setting = -firearm->o.firearm.setting;
			jam = true;
			fired = ind; // We also only have fired this many bullets now.

			for (int xind = ind; xind < fired; xind++)
			{
				res_result[xind] = JAM; // We set the rest of the results to being jammed.
			}

			break;
		}
		else if (wild)
		{
			res_result[ind] = MISS;
		}
		// Any shot beyond our first if we're fighting is going to be a miss.
		else if ((ch->fighting || GET_POS(ch) == POSITION_FIGHTING) && ind > 0)
		{
			res_result[ind] = MISS;
		}
		else
		{
			res_result[ind] = calculate_firearm_result (ch, firearm->o.firearm.use_skill, attack_mod + (ind * 6), target, was_hidden, firearm, ammo[ind], NULL, &res_location[ind], &res_damage[ind], &poison, &res_bleed[ind], &res_armor1[ind], dir, pointblank, (ch->fighting == target || target->fighting == ch ? true : false), dam_mod, cmd);
			res_damage[ind] = (int) res_damage[ind];
			res_bleed[ind] = (int) res_bleed[ind];
			location_table[ind] = str_dup(figure_location (target, res_location[ind]));
		}
	}

	// If we're fighting, we become a bit unbalanced as a result of all this commotion.
	if (ch->fighting)
	{

		if (ch->agi <= 9)
			ch->balance += -14;
		else if (ch->agi > 9 && ch->agi <= 13)
			ch->balance += -12;
		else if (ch->agi > 13 && ch->agi <= 15)
			ch->balance += -10;
		else if (ch->agi > 15 && ch->agi <= 18)
			ch->balance += -8;
		else
			ch->balance += -6;

		ch->balance = MAX (ch->balance, -25);
	}

	// We now set combat block.
	ch->combat_block = 3;
	target->combat_block = 3;

	if (get_second_affect(ch, SA_POINTSTRIKE, NULL))
		remove_second_affect(get_second_affect (ch, SA_POINTSTRIKE, NULL));



	if (fired == 1)
		sprintf(buf6, "a");
	else if (fired < 7)
		sprintf(buf6, "a burst of");
	else if (fired >= 7)
		sprintf(buf6, "a hail of");


	if (IS_SLING(firearm))
	{
		if (jam == true && fired == 0)
		{
			sprintf(buf, "You %srelease the pocket of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %sreleases the pocket of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT);
			broke_aim(ch, 0);
			return;
		}
		else if (ranged)
		{
			sprintf (buf, "You %sfire #2%s#0, #2%s %s%s#0 shooting %sward towards #5%s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  ch->delay_who, char_short (original_target));
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting %sward.",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  ch->delay_who);
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);
		}
		else
		{
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting towards #5%s#0.",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short (original_target));
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);

			sprintf (buf3, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting towards #5you#0!",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"));
			*buf3 = toupper (*buf3);
			sprintf (buffer, "#5%s", buf3);
			sprintf (buf3, "%s", buffer);


			sprintf (buf, "You %srelease the pocket of #2%s#0, #2%s %s%s#0 shooting towards #5%s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short (original_target));
		}
	}
	else if (firearm->o.firearm.use_skill >= SKILL_HANDGUN && firearm->o.firearm.use_skill <= SKILL_GUNNERY)
	{
		if (jam == true && fired == 0)
		{
			sprintf(buf, "You %ssqueeze the trigger of $p, but nothing happens.", (af ? "rises from cover and " : ""));
			act (buf, false, ch, firearm, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf2, "$n %ssqueezes the trigger of $p, but nothing happens.\n", (af ? "rises from cover and " : ""));
			act (buf2, false, ch, firearm, 0, TO_ROOM | _ACT_FORMAT | _ACT_FIREFIGHT);
			broke_aim(ch, 0);
			return;
		}
		else if (wild == 2)
		{
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting into #2the %s#0.",
				char_short(ch), (af ? "rises from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), arg);
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);

			sprintf (buf, "You %ssqueeze the trigger of #2%s#0, #2%s %s%s#0 shooting into #2the %s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), arg);

		}
		else if (wild == 1)
		{
			sprintf (buf, "You %sfire #2%s#0, #2%s %s%s#0 shooting %sward.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  arg);
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting %sward.",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  arg);
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);
		}
		else if (pointblank)
		{
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 ripping into #5%s#0.",
				char_short(ch), (af ? "rises from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), (target == ch ? (GET_SEX(ch) == SEX_MALE ? "himself" : "herself") : char_short (target)));
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);

			sprintf (buf3, "%s#0 %sfires #2%s#0, #2%s %s%s#0 ripping into #5you#0!",
				char_short(ch), (af ? "rises from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"));
			*buf3 = toupper (*buf3);
			sprintf (buffer, "#5%s", buf3);
			sprintf (buf3, "%s", buffer);

			sprintf (buf, "You %sfire #2%s#0, #2%s %s%s#0 ripping into #5%s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), (target == ch ? "yourself" : char_short (target)));
		}
		else if (ranged)
		{
			sprintf (buf, "You %sfire #2%s#0, #2%s %s%s#0 shooting %sward towards #5%s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  ch->delay_who, char_short (original_target));
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting %sward.",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"),  ch->delay_who);
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);
		}
		else
		{
			sprintf (buf2, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting towards #5%s#0.",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short (original_target));
			*buf2 = toupper (*buf2);
			sprintf (buffer, "#5%s", buf2);
			sprintf (buf2, "%s", buffer);
			watched_action(ch, buf2, 0, 1);

			sprintf (buf3, "%s#0 %sfires #2%s#0, #2%s %s%s#0 shooting towards #5you#0!",
				char_short(ch), (af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"));
			*buf3 = toupper (*buf3);
			sprintf (buffer, "#5%s", buf3);
			sprintf (buf3, "%s", buffer);


			sprintf (buf, "You %ssqueeze the trigger of #2%s#0, #2%s %s%s#0 shooting towards #5%s#0.",
				(af ? "rise from cover and " : ""),
				obj_short_desc(firearm), buf6, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short (original_target));
		}
	}
	else
	{
		sprintf (buffer, "FIREARM BUG? %s fires (VNUM: %d) from (VNUM: %d)", ch->tname, ammo[0]->nVirtual, firearm->nVirtual);
		system_log (buffer, true);
	}

	if (ranged)
	{
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;

			if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
			{
				if (get_second_affect(tch, SA_ROOMFIRED, 0))
				{
					get_second_affect(tch, SA_ROOMFIRED, 0)->info2+= fired;
				}
				else
				{
					add_second_affect(SA_ROOMFIRED, 3, tch, 0, 0, fired);
				}
			}
			else
			{
				act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
			}
		}

		sprintf (buf3, "%s %s#0 rips through the area, streaking %sward.", buf6, shell_name[firearm->o.firearm.caliber], ch->delay_who);
		*buf3 = toupper (*buf3);
		sprintf (buffer, "#2%s", buf3);
		sprintf (buf3, "%s", buffer);
		reformat_string (buf3, &p);
		if (EXIT (ch, dir))
			room = vnum_to_room (EXIT (ch, dir)->to_room);
		else
			room = NULL;
		if (room && target->room != room)
		{
			for (tch = room->people; tch; tch = tch->next_in_room)
			{
				//if (skill_use (tch, SKILL_SCAN, 0))
				//send_to_char (p, tch);

				if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
				{
					if (get_second_affect(tch, SA_ROOMSHOTS, 0))
					{
						get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
					}
					else
					{
						add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
					}
				}
				else
				{
					send_to_char(p, tch);
				}
			}

			if (room->dir_option[dir])
				room = vnum_to_room (room->dir_option[dir]->to_room);
			else
				room = NULL;

			if (room && target->room != room)
			{
				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					//if (skill_use (tch, SKILL_SCAN, 0))
					//send_to_char (p, tch);
					if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
					{
						if (get_second_affect(tch, SA_ROOMSHOTS, 0))
						{
							get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
						}
						else
						{
							add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
						}
					}
					else
					{
						send_to_char(p, tch);
					}
				}
				if (room->dir_option[dir])
					room = vnum_to_room (room->dir_option[dir]->to_room);
				else
					room = NULL;
				if (room && target->room != room)
				{
					for (tch = room->people; tch; tch = tch->next_in_room)
					{
						//if (skill_use (tch, SKILL_SCAN, 0))
						send_to_char (p, tch);

						if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
						{
							if (get_second_affect(tch, SA_ROOMSHOTS, 0))
							{
								get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
							}
							else
							{
								add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
							}
						}
						else
						{
							send_to_char(p, tch);
						}


					}
				}
			}
		}
		mem_free (p);
	}
	else if (wild == 1)
	{
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		sprintf (buf3, "%s %s#0 rips through the area, streaking %sward.", buf6, shell_name[firearm->o.firearm.caliber], arg);
		*buf3 = toupper (*buf3);
		sprintf (buffer, "#2%s", buf3);
		sprintf (buf3, "%s", buffer);
		reformat_string (buf3, &p);

		if (room)
		{
			for (tch = room->people; tch; tch = tch->next_in_room)
			{
				//send_to_char (p, tch);

				if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
				{
					if (get_second_affect(tch, SA_ROOMSHOTS, 0))
					{
						get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
					}
					else
					{
						add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
					}
				}
				else
				{
					send_to_char(p, tch);
				}
			}
			if (room->dir_option[dir])
			{
				room = vnum_to_room (room->dir_option[dir]->to_room);
			}
			else
			{
				room = NULL;
			}

			if (room)
			{
				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					//send_to_char (p, tch);
					if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
					{
						if (get_second_affect(tch, SA_ROOMSHOTS, 0))
						{
							get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
						}
						else
						{
							add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
						}
					}
					else
					{
						send_to_char(p, tch);
					}
				}
				if (room->dir_option[dir])
				{
					room = vnum_to_room (room->dir_option[dir]->to_room);
				}
				else
				{
					room = NULL;
				}

				if (room)
				{
					for (tch = room->people; tch; tch = tch->next_in_room)
					{
						//send_to_char (p, tch);
						if (IS_SET(tch->plr_flags, FIREFIGHT_FILTER))
						{
							if (get_second_affect(tch, SA_ROOMSHOTS, 0))
							{
								get_second_affect(tch, SA_ROOMSHOTS, 0)->info2+= fired;
							}
							else
							{
								add_second_affect(SA_ROOMSHOTS, 3, tch, 0, 0, fired);
							}
						}
						else
						{
							send_to_char(p, tch);
						}
					}
				}
			}
		}
		mem_free (p);
	}
	else if (!ranged)
	{
		act (buf, false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
		act (buf3, false, ch, 0, target, TO_VICT | _ACT_FORMAT);
		act (buf2, false, ch, 0, target, TO_NOTVICT | _ACT_FORMAT);
	}

	// Now we make some noise, people.
	if (!IS_SET(firearm->o.firearm.bits, GUN_SILENT) && !IS_SLING(firearm))
		firearm_bang (ch->room, fired, target->in_room);

	//obj_from_obj(&ammo, 0);

	if (ranged && ch->delay_who)
	{
		if (!str_cmp (ch->delay_who, "north"))
		{
			from_direction = str_dup ("the south");
			direction = 3;
		}
		else if (!str_cmp (ch->delay_who, "east"))
		{
			from_direction = str_dup ("the west");
			direction = 4;
		}
		else if (!str_cmp (ch->delay_who, "south"))
		{
			from_direction = str_dup ("the north");
			direction = 1;
		}
		else if (!str_cmp (ch->delay_who, "west"))
		{
			from_direction = str_dup ("the east");
			direction = 2;
		}
		else if (!str_cmp (ch->delay_who, "up"))
		{
			from_direction = str_dup ("below");
			direction = 6;
		}
		else if (!str_cmp (ch->delay_who, "down"))
		{
			from_direction = str_dup ("above");
			direction = 5;
		}
	}
	else
		from_direction = add_hash ("an indeterminate direction");


	// Now we count up how much of each result we.
	// 4 is a flat out miss
	// 6 is the cover being hit.
	// 7 is shield-block
	// 1 is glance
	// 10 is caught,
	// 2, 9 are lodgers
	// 8 is a puncture.
	if (!wild)
	{
		for (int ind = 0; ind < fired; ind++)
		{
			if (res_result[ind] == 4)
			{
				//std::string gods = "Miss ";
				//send_to_gods(gods.c_str());
				result_table[0]++;
			}
			else if (res_result[ind] == 6)
			{
				//std::string gods = "Cover ";
				//send_to_gods(gods.c_str());
				result_table[1]++;
			}
			else if (res_result[ind] == 7)
			{
				//std::string gods = "Shield ";
				//send_to_gods(gods.c_str());
				result_table[2]++;
			}
			else if (res_result[ind] == 1)
			{
				//std::string gods = "Glance " + MAKE_STRING(location_table[ind]);
				//send_to_gods(gods.c_str());
				result_table[3]++;
			}
			else if (res_result[ind] == 10)
			{
				//std::string gods = "Caught ";
				//send_to_gods(gods.c_str());
				result_table[4]++;
			}
			else if (res_result[ind] == 2 || res_result[ind] == 9)
			{
				//std::string gods = "Lodged " + MAKE_STRING(location_table[ind]);
				//send_to_gods(gods.c_str());
				result_table[5]++;
			}
			else if (res_result[ind] == 8)
			{
				//std::string gods = "Puncture " + MAKE_STRING(location_table[ind]);
				//send_to_gods(gods.c_str());
				result_table[6]++;
			}
			else if (!jam)
			{
				send_to_gods("result_table error found.");
			}
		}


		*buf = '\0';
		*buf2 = '\0';
		*buf3 = '\0';
		*buffer = '\0';

		int remainings = 0;
		int rounds = 0;
		int marker = -1;

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}

			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[0])
		{
			sprintf(buf + strlen(buf), " miss%s you%s", (fired == 1 ? "es" : ""),
				(!number(0,4) ? " completely" :
				!number(0,3) ? " broadly" :
				!number(0,2) ? "" :
				!number(0,1) ? " barely" : " by a whisper"));

			sprintf(buf2 + strlen(buf2), " miss%s %s%s", (fired == 1 ? "es" : ""), HMHR(target),
				(!number(0,4) ? " completely" :
				!number(0,3) ? " broadly" :
				!number(0,2) ? "" :
				!number(0,1) ? " barely" : " by a whisper"));

			sprintf(buf3 + strlen(buf3), " miss%s %s%s", (fired == 1 ? "es" : ""), HMHR(target),
				(!number(0,4) ? " completely" :
				!number(0,3) ? " broadly" :
				!number(0,2) ? "" :
				!number(0,1) ? " barely" : " by a whisper"));
		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[1])
		{
			af = get_affect(target, AFFECT_COVER);
			sprintf(buf + strlen(buf), " %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				(af ? (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, target->room->contents) ? obj_short_desc(af->a.cover.obj) : "your cover") : "the scenery"));

			sprintf(buf2 + strlen(buf2), " %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				(af ? (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, target->room->contents) ? obj_short_desc(af->a.cover.obj) : "the cover") : "the scenery"));

			sprintf(buf3 + strlen(buf3), " %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				(af ? (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, target->room->contents) ? obj_short_desc(af->a.cover.obj) : "the cover") : "the scenery"));

			if (af && af->a.cover.obj && is_obj_in_list(af->a.cover.obj, target->room->contents))
			{
				cover = af->a.cover.obj;
			}
		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[2])
		{
			sprintf(buf + strlen(buf), " %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				obj_short_desc(shield_obj));

			sprintf(buf2 + strlen(buf2), " %s %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				HSHR(target),
				obj_short_desc(shield_obj));

			sprintf(buf3 + strlen(buf3), " %s %s #2%s#0",
				(!number(0,3) ? "ricochets off" :
				!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				HSHR(target),
				obj_short_desc(shield_obj));
			*buffer = '\0';
		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[3])
		{
			*buf7 = '\0';
			rounds = 1;
			counts = 1;

			for (int ind = 0; ind < fired; ind++)
			{
				loop_skip = false;
				any_left = false;
				if (res_result[ind] == 1)
				{
					counts++;
					for (int xind = 0; xind < fired; xind++)
					{
						if (res_result[xind] == 1 && xind > ind && (!str_cmp(location_table[xind], location_table[ind])))
						{
							loop_skip = true;
						}
					}
					if (loop_skip)
					{
						if (rounds > 1)
							rounds ++;
						continue;
					}
					else if (rounds == 1)
					{
						sprintf(buf7, "%s", expand_wound_loc(location_table[ind]));
					}
					else
					{
						for (int xind = ind; xind < fired; xind++)
						{
							if (xind != ind && res_result[xind] == 3)
							{
								any_left = true;
								break;
							}
						}

						if (!any_left)
						{
							sprintf(buf7 + strlen(buf7), " and %s", expand_wound_loc(location_table[ind]));
							break;
						}
						else
						{
							sprintf(buf7 + strlen(buf7), ", %s", expand_wound_loc(location_table[ind]));
						}
					}
					rounds++;
				}
			}

			sprintf(buf + strlen(buf), " %s across your %s",
				(!number(0,5) ? "glances" :
				!number(0,4) ? "grazes" :
				!number(0,3) ? "brushes" :
				!number(0,2) ? "scratches " :
				!number(0,1) ? "scrapes" : "shaves"),
				buf7);

			sprintf(buf2 + strlen(buf2), " %s across %s %s",
				(!number(0,5) ? "glances" :
				!number(0,4) ? "grazes" :
				!number(0,3) ? "brushes" :
				!number(0,2) ? "scratches " :
				!number(0,1) ? "scrapes" : "shaves"),
				HSHR(target),
				buf7);

			sprintf(buf3 + strlen(buf3), " %s across %s %s",
				(!number(0,5) ? "glances" :
				!number(0,4) ? "grazes" :
				!number(0,3) ? "brushes" :
				!number(0,2) ? "scratches " :
				!number(0,1) ? "scrapes" : "shaves"),
				HSHR(target),
				buf7);
		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[4])
		{
			*buf7 = '\0';
			rounds = 1;
			counts = 1;

			for (int ind = 0; ind < fired; ind++)
			{
				loop_skip = false;
				any_left = false;
				if (res_result[ind] == 10)
				{
					counts++;
					for (int xind = 0; xind < fired; xind++)
					{
						if (res_result[xind] == 10 && xind > ind && res_armor1[ind] == res_armor1[xind])
						{
							loop_skip = true;
						}
					}
					if (loop_skip)
					{
						if (rounds > 1)
							rounds ++;
						continue;
					}
					else if (rounds == 1)
					{
						sprintf(buf7, "#2%s#0", obj_short_desc(res_armor1[ind]));
					}
					else
					{
						for (int xind = ind; xind < fired; xind++)
						{
							if (xind != ind && res_result[xind] == 3)
							{
								any_left = true;
								break;
							}
						}

						if (!any_left)
						{
							sprintf(buf7 + strlen(buf7), " and #2%s#0", obj_short_desc(res_armor1[ind]));
							break;
						}
						else
						{
							sprintf(buf7 + strlen(buf7), ", #2%s#0", obj_short_desc(res_armor1[ind]));
						}
					}
					rounds++;
				}
			}

			sprintf(buf + strlen(buf), " %s #2%s#0",
				(!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				buf7);
			sprintf(buf2 + strlen(buf2), " %s %s #2%s#0",
				(!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				HSHR(target), buf7);
			sprintf(buf3 + strlen(buf3), " %s %s #2%s#0",
				(!number(0,2) ? "strikes" :
				!number(0,1) ? "slams in to" : "thuds against"),
				HSHR(target), buf7);

		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[5])
		{
			*buf7 = '\0';
			rounds = 1;
			counts = 1;

			for (int ind = 0; ind < fired; ind++)
			{
				loop_skip = false;
				any_left = false;
				counts++;
				if (res_result[ind] == 2 || res_result[ind] == 9)
				{
					for (int xind = 0; xind < fired; xind++)
					{
						if ((res_result[xind] == 2 || res_result[xind] == 9) && xind > ind && (!str_cmp(location_table[xind], location_table[ind])))
						{
							loop_skip = true;
						}
					}
					if (loop_skip)
					{
						if (rounds > 1)
							rounds ++;
						continue;
					}
					else if (rounds == 1)
					{
						sprintf(buf7, "%s", expand_wound_loc(location_table[ind]));
					}
					else
					{
						for (int xind = ind; xind < fired; xind++)
						{
							if (xind != ind && (res_result[xind] == 2 || res_result[xind] == 9))
							{
								any_left = true;
								break;
							}
						}
						if (!any_left)
						{
							sprintf(buf7 + strlen(buf7), " and %s", expand_wound_loc(location_table[ind]));
							break;
						}
						else
						{
							sprintf(buf7 + strlen(buf7), ", %s", expand_wound_loc(location_table[ind]));
						}
					}
					rounds++;
				}
			}

			if (IS_SLING(firearm))
			{
				sprintf(buf + strlen(buf), " strikes your %s", buf7);
				sprintf(buf2 + strlen(buf2), " strikes %s %s", HSHR(target), buf7);
				sprintf(buf3 + strlen(buf3), " strikes %s %s", HSHR(target), buf7);
			}
			else
			{
				sprintf(buf + strlen(buf), " %s your %s",
					(!number(0,2) ? "lodges in" :
					!number(0,1) ? "ruptures" : "punctures"),
					buf7);

				sprintf(buf2 + strlen(buf2), " %s %s %s",
					(!number(0,2) ? "lodges in" :
					!number(0,1) ? "ruptures" : "punctures"),
					HSHR(target),
					buf7);

				sprintf(buf3 + strlen(buf3), " %s %s %s",
					(!number(0,2) ? "lodges in" :
					!number(0,1) ? "ruptures" : "punctures"),
					HSHR(target),
					buf7);
			}
		}

		/*  REAMININGS TEST  */
		marker++;
		remainings = 0;
		*buf6 = '\0';
		for (int ind = marker; ind < 7; ind++)
		{
			if (result_table[ind] != JAM)
				remainings++;
		}
		if (result_table[marker])
		{
			if (IS_SLING(firearm))
			{
				sprintf(buf6, "the ball bearing");
			}
			else
			{
				if (fired == 1)
					sprintf(buf6, "the bullet");
				else
				{
					if (result_table[marker] == fired)
						sprintf(buf6, "all the bullets");
					else if ((result_table[marker] * 100) > (fired * 100 / 2))
						sprintf(buf6, "most of the bullets");
					else if (result_table[marker] > 1)
						sprintf(buf6, "several bullets");
					else
						sprintf(buf6, "one of the bullets");
				}
			}
			if (*buf)
			{
				if (remainings > 1)
				{
					sprintf (buf + strlen(buf), ", %s", buf6);
					sprintf (buf2 + strlen(buf2), ", %s", buf6);
					sprintf (buf3 + strlen(buf3), ", %s", buf6);
				}
				else if (remainings == 1)
				{
					sprintf (buf + strlen(buf), " and %s", buf6);
					sprintf (buf2 + strlen(buf2), " and %s", buf6);
					sprintf (buf3 + strlen(buf3), " and %s", buf6);
				}
			}
			else
			{
				sprintf(buf, "%s", buf6);
				*buf = toupper(*buf);
				sprintf(buf2, "%s", buf6);
				*buf2 = toupper(*buf2);
				sprintf(buf3, "%s", buf6);
				*buf3 = toupper(*buf3);
			}
		}

		if (result_table[6])
		{
			*buf7 = '\0';
			rounds = 1;
			counts = 1;

			for (int ind = 0; ind < fired; ind++)
			{
				counts++;
				loop_skip = false;
				any_left = false;
				if (res_result[ind] == 8)
				{
					for (int xind = 0; xind < fired; xind++)
					{
						if (res_result[xind] == 8 && xind > ind && (!str_cmp(location_table[xind], location_table[ind])))
						{
							loop_skip = true;
						}
					}

					if (loop_skip)
					{
						if (rounds > 1)
							rounds ++;
						continue;
					}
					else if (rounds == 1)
					{
						sprintf(buf7, "%s", expand_wound_loc(location_table[ind]));
					}
					else
					{
						for (int xind = ind; xind < fired; xind++)
						{
							if (xind != ind && res_result[xind] == 8)
							{
								any_left = true;
								break;
							}
						}
						if (!any_left)
						{
							sprintf(buf7 + strlen(buf7), " and %s", expand_wound_loc(location_table[ind]));
							break;
						}
						else
						{
							sprintf(buf7 + strlen(buf7), ", %s", expand_wound_loc(location_table[ind]));
						}
					}
					rounds++;
				}
			}

			sprintf(buf + strlen(buf), " %s your %s",
				(!number(0,2) ? "perforates" :
				!number(0,1) ? "rips clean through" : "penetrates through"),
				buf7);

			sprintf(buf2 + strlen(buf2), " %s %s %s",
				(!number(0,2) ? "perforates" :
				!number(0,1) ? "rips clean through" : "penetrates through"),
				HSHR(target),
				buf7);

			sprintf(buf3 + strlen(buf3), " %s %s %s",
				(!number(0,2) ? "perforates" :
				!number(0,1) ? "rips clean through" : "penetrates through"),
				HSHR(target),
				buf7);

		}
	}


	// If we jammed, we'll need to append the message.
	if (jam)
	{
		if (fired >= 1)
		{
			if (!ranged)
			{
				sprintf (buf + strlen(buf), ". But, #2%s#0 fires no more shots as it jams mid-burst", obj_short_desc(firearm));
				sprintf (buf2 + strlen(buf2), ". But, #2%s#0 fires no more shots as it jams mid-burst", obj_short_desc(firearm));
			}
			else
			{
				sprintf (buf3 + strlen(buf3), ". But, #2%s#0 fires no more shots as it jams mid-burst", obj_short_desc(firearm));
			}
		}
		else
		{
			if (!ranged)
			{
				sprintf(buf, "But instead of firing, #2%s#0 jams", obj_short_desc(firearm));
				sprintf(buf2, "But instead of firing, #2%s#0 jams", obj_short_desc(firearm));
			}
			else
			{
				sprintf(buf3, "But instead of firing, #2%s#0 jams", obj_short_desc(firearm));
			}
		}
	}

	if (fired == 1)
		sprintf(buf6, "a");
	else if (fired < 7)
		sprintf(buf6, "a burst of");
	else if (fired >= 7)
		sprintf(buf6, "a hail of");

	*buf6 = toupper(*buf6);

	bool only_miss = false;

	if (ranged)
	{
		sprintf (buffer, "#2%s %s %s%s#0 comes whirring through the air from %s, heading straight towards you!\n\n%s", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), from_direction, buf);
		sprintf(buf, "%s", buffer);
		sprintf (buffer, "#2%s %s %s%s#0 comes whirring through the air from %s, heading straight towards #5%s#0!\n\n%s", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), from_direction, char_short(target), buf2);
		sprintf(buf2, "%s", buffer);

		if (result_table[1] > 0 || result_table[10] > 0 || result_table[2] > 0 || result_table[8] > 0 || result_table[9] > 0)
		{
			sprintf (buffer, "#2%s %s %s%s#0 comes whirring through the air from %s, hitting #5%s#0.", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), from_direction, char_short(target));
			sprintf (buf4, "%s", buffer);
			sprintf (buffer, "#2%s %s %s%s#0 hits #5%s#0.", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short(target));
			sprintf (buf7, "%s", buffer);
		}
		else
		{
			sprintf (buffer, "#2%s %s %s%s#0 comes whirring through the air from %s, missing #5%s#0.", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), from_direction, char_short(target));
			sprintf (buf4, "%s", buffer);
			sprintf (buffer, "#2%s %s %s%s#0 misses #5%s#0.", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short(target));
			sprintf (buf7, "%s", buffer);
			only_miss = true;
		}

		if (switched_target)
		{
			*buf3 = tolower(*buf3);
			sprintf (buffer, "#2%s %s %s%s#0 strays off course, heading instead towards #5%s#0: %s", buf6, buf5, shell_name[firearm->o.firearm.caliber], (fired == 1 ? "" : "s"), char_short(target), buf3);
			sprintf(buf3, "%s", buffer);
		}
	}
	else if (switched_target && !jam)
	{
		if (cramp_involved)
		{
			sprintf (buffer, "%s wildly towards #5you#0! %s", (fired == 1 ? "It ricochets" : "They ricochet"), buf);
			sprintf(buf, "%s", buffer);

			sprintf (buffer, "%s wildly towards #5%s#0! %s", (fired == 1 ? "It ricochets" : "They ricochet"), char_short(target), buf2);
			sprintf(buf2, "%s", buffer);
		}
		else
		{
			sprintf (buffer, "%s off course, heading instead towards #5you#0! %s", (fired == 1 ? "It strays" : "They stray"), buf);
			sprintf(buf, "%s", buffer);

			sprintf (buffer, "%s off course, heading instead towards #5%s#0! %s", (fired == 1 ? "It strays" : "They stray"), char_short(target), buf2);
			sprintf(buf2, "%s", buffer);
		}

		if (result_table[1] > 0 || result_table[10] > 0 || result_table[2] > 0 || result_table[8] > 0 || result_table[9] > 0)
		{
			sprintf (buffer, "%s #5%s#0.", (fired == 1 ? "It hits" : "They hit"), char_short(target));
			sprintf (buf4, "%s", buffer);
		}
		else
		{
			sprintf (buffer, "%s #5%s#0.", (fired == 1 ? "It misses" : "They miss"), char_short(target));
			sprintf (buf4, "%s", buffer);
			only_miss = true;
		}
	}

	strcat(buf, ".\n");
	strcat(buf2, ".\n");
	strcat(buf3, ".\n");

	if (IS_SLING(firearm))
	{
		obj_from_obj(&ammo[0], 1);
		obj_to_room (ammo[0], target->in_room);
	}
	else
	{
		for (int ind = 0; ind < fired; ind++)
		{
			old_result = res_result[ind];
			shell = NULL;
			bullet = NULL;

			if (res_result[ind] == GLANCING_HIT || res_result[ind] == MISS || res_result[ind] == SHIELD_BLOCK || res_result[ind] == PUNCTURE_HIT || res_result[ind] == COVER_HIT)
			{
				// If it's a glancing hit, 50% chance of striking scenery, 50% chance of flying somewhere.

				if (res_result[ind] == GLANCING_HIT)
				{
					if (number(0,1))
						res_result[ind] = COVER_HIT;
					else
						res_result[ind] = MISS;
				}

				// If it's a cover hit, 50% chance it shattered (and thus is gone), 50% chance it's flattened against terrain,
				// unless of course we have a cover object, in which case it's lodged in there.
				if (res_result[ind] == COVER_HIT)
				{
					if (number(0,1) && (bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "flattened", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						if (cover)
						{
							if (cover->lodged)
							{
								CREATE (cover->lodged, LODGED_OBJECT_INFO, 1);
								cover->lodged->vnum = bullet->nVirtual;
								cover->lodged->location = add_hash ("internal");
								cover->lodged->next = NULL;
								cover->lodged->colored = 1;
								cover->lodged->var_color = bullet->var_color[0];
								cover->lodged->var_color2 = bullet->var_color[1];
								cover->lodged->var_color3 = bullet->var_color[2];
								cover->lodged->short_description = bullet->short_description;
							}
							else
							{
								for (clodged = cover->lodged; clodged; clodged = clodged->next)
								{
									if (!clodged->next)
									{
										CREATE (clodged->next, LODGED_OBJECT_INFO, 1);
										clodged->next->vnum = bullet->nVirtual;
										clodged->next->location = add_hash ("internal");
										clodged->next->colored = 1;
										clodged->next->var_color = bullet->var_color[0];
										clodged->next->var_color2 = bullet->var_color[1];
										clodged->next->var_color3 = bullet->var_color[2];
										clodged->next->short_description = bullet->short_description;
										clodged->next->next = NULL;
										break;
									}
								}
							}
						}
						else
						{
							magic_add_obj_affect (bullet, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
							bullet->omote_str = add_hash ("is lodged in the scenery.");
							obj_to_room (bullet, target->in_room);
						}
					}
				}
				// Again, 50% chance it then shattered, otherwise it's chipped.
				else if (res_result[ind] == MISS || res_result[ind] == PUNCTURE_HIT)
				{
					if (number (0,1) && (bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "chipped", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						if (direction >= 0)
						{
							exit = EXIT (target, direction);

							// If there's not an exit to the next room, or 33% of the time, we strike here.
							if (!exit || !(next_room = vnum_to_room (exit->to_room)) || (IS_SET (exit->exit_info, EX_ISDOOR) && IS_SET (exit->exit_info, EX_CLOSED)) || !number(0,2))
							{
								magic_add_obj_affect (bullet, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
								bullet->omote_str = add_hash ("decorates the area.");
								if (wild && room)
									obj_to_room (bullet, room->vnum);
								else if (!wild)
									obj_to_room (bullet, target->in_room);
							}
							else
							{
								magic_add_obj_affect (bullet, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
								bullet->omote_str = add_hash ("decorates the area.");
								if (wild && room)
									obj_to_room (bullet, room->vnum);
								else if (!wild)
									obj_to_room (bullet, target->in_room);
							}
						}
						else
						{
							magic_add_obj_affect (bullet, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
							bullet->omote_str = add_hash ("decorates the area.");
							if (wild && room)
								obj_to_room (bullet, room->vnum);
							else if (!wild)
								obj_to_room (bullet, target->in_room);
						}
					}
				}
				res_result[ind] = old_result;

				//if (result == PUNCTURE_HIT)
				//  magic_add_affect (target, AFFECT_INTERNAL, -1, 2, 0, 0, 0);
			}
			else if (res_result[ind] == CRITICAL_HIT || res_result[ind] == HIT || res_result[ind] == SHATTER_HIT)
			{
				if (res_result[ind] == SHATTER_HIT)
				{
					if ((bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "shattered", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						lodge_missile (target, bullet, location_table[ind], (res_result[ind] == SHATTER_HIT ? 3 : 1));
					}

				}
				else if (!number(0,2))
				{
					if ((bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "flattened", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						lodge_missile (target, bullet, location_table[ind], 1);
					}
				}
				else if (!number(0,1))
				{
					if ((bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "chipped", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						lodge_missile (target, bullet, location_table[ind], 1);
					}
				}
				else
				{
					if ((bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "oblong", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
					{
						bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
						bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
						lodge_missile (target, bullet, location_table[ind], 1);
					}
				}
			}
			else if (res_result[ind] == CAUGHT)
			{
				if ((bullet = load_colored_object(fBULLET, ammo[ind]->var_color[0], "flattened", ammo[ind]->var_color[1], 0, 0, 0, 0, 0, 0, 0)))
				{
					bullet->o.od.value[2] = ammo[ind]->o.od.value[2];
					bullet->o.od.value[4] = ammo[ind]->o.od.value[4];
					if (res_armor1[ind])
					{
						if (!res_armor1[ind]->lodged)
						{
							CREATE (res_armor1[ind]->lodged, LODGED_OBJECT_INFO, 1);
							res_armor1[ind]->lodged->vnum = bullet->nVirtual;
							res_armor1[ind]->lodged->location = add_hash ("internal");
							res_armor1[ind]->lodged->next = NULL;
							res_armor1[ind]->lodged->colored = 1;
							res_armor1[ind]->lodged->var_color = bullet->var_color[0];
							res_armor1[ind]->lodged->var_color2 = bullet->var_color[1];
							res_armor1[ind]->lodged->var_color3 = bullet->var_color[2];
							res_armor1[ind]->lodged->short_description = bullet->short_description;
						}
						else
						{
							for (clodged = res_armor1[ind]->lodged; clodged; clodged = clodged->next)
							{
								if (!clodged->next)
								{
									CREATE (clodged->next, LODGED_OBJECT_INFO, 1);
									clodged->next->vnum = bullet->nVirtual;
									clodged->next->location = add_hash ("internal");
									clodged->next->colored = 1;
									clodged->next->var_color = bullet->var_color[0];
									clodged->next->var_color2 = bullet->var_color[1];
									clodged->next->var_color3 = bullet->var_color[2];
									clodged->next->short_description = bullet->short_description;
									clodged->next->next = NULL;
									break;
								}
							}
						}
					}
				}
			}


			if (!IS_DIRECT(firearm))
				clip->o.clip.amount -= 1;

			obj_from_obj(&ammo[ind], 1);
			extract_obj(ammo[ind]);

			if ((shell = LOAD_COLOR(ammo[ind], fSHELL + ammo[ind]->o.bullet.size)))
			{
				shell->o.od.value[2] = ammo[ind]->o.od.value[2];
				shell->o.od.value[4] = ammo[ind]->o.od.value[4];
				if (IS_DIRECT(firearm))
				{
					obj_to_obj(shell, firearm);
				}
				else
				{
					if (number(0,1))
					{
						magic_add_obj_affect (shell, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
						shell->omote_str = add_hash ("litters the area, out of sight.");
						obj_to_room (shell, ch->in_room);
					}
					else
					{
						shell->omote_str = add_hash ("lies scattered about the area.");
						obj_to_room (shell, ch->in_room);
					}
				}
			}


		}
	}

	char *out1, *out2, *out3, *out4, *out5;
	CHAR_DATA* rch = 0;
	reformat_string (buf, &out1);
	reformat_string (buf2, &out2);
	reformat_string (buf3, &out3);
	reformat_string (buf4, &out4);
	reformat_string (buf7, &out5);

	if (ranged)
	{
		if (!wild)
		{
			for (rch = ch->room->people; rch; rch = rch->next_in_room)
			{
				if (rch == ch)
				{
					send_to_char ("\n", rch);
					send_to_char (out3, rch);
					send_to_char ("\n", rch);
				}
				else if (IS_SET (rch->plr_flags, FIREFIGHT_FILTER))
				{
					;
				}
				else
				{
					send_to_char ("\n", rch);
					send_to_char (out3, rch);
					send_to_char ("\n", rch);
				}
			}
		}

		for (rch = target->room->people; rch; rch = rch->next_in_room)
		{
			if (rch == target)
			{
				send_to_char ("\n", rch);
				send_to_char (out1, rch);
				send_to_char ("\n", rch);
			}
			// If we've got filters on, we don't want to see misses:
			// we'll just lump them in with the rest of the nonsense.
			else if (IS_SET (rch->plr_flags, FIREFIGHT_FILTER))
			{
				if (only_miss)
				{
					if (get_second_affect(rch, SA_ROOMSHOTS, 0))
					{
						get_second_affect(rch, SA_ROOMSHOTS, 0)->info2+= fired;

					}
					else
					{
						add_second_affect(SA_ROOMSHOTS, 3, rch, 0, 0, fired);
					}
				}
				else
				{
					send_to_char ("\n", rch);
					send_to_char (out4, rch);
					send_to_char ("\n", rch);
				}
			}
			else
			{
				send_to_char ("\n", rch);
				send_to_char (out2, rch);
				send_to_char ("\n", rch);
			}
		}
	}
	else if (!wild)
	{
		for (rch = target->room->people; rch; rch = rch->next_in_room)
		{
			send_to_char ("\n", rch);
			if (rch == target)
			{
				send_to_char (out1, rch);
				send_to_char ("\n", rch);
			}
			else
			{
				if (rch == ch)
				{
					send_to_char (out2, rch);
					send_to_char ("\n", rch);
				}
				else if (IS_SET (rch->plr_flags, FIREFIGHT_FILTER))
				{
					send_to_char (out4, rch);
					send_to_char ("\n", rch);
				}
				else
				{
					send_to_char (out2, rch);
					send_to_char ("\n", rch);
				}
			}
		}
	}
	mem_free (out1);
	mem_free (out2);
	mem_free (out3);
	mem_free (out4);
	mem_free (out5);

	criminalize (ch, target, target->room->zone, CRIME_SHOOT);

	ch->enemy_direction = NULL;

	if (from_direction)
		mem_free (from_direction);

	bool shocked = false;
	bool group_shocked = false;
	bool miss_shocked = false;

	if (!IS_SLING(firearm))
	{
		for (int ind = 0; ind < fired; ind++)
		{
			if (res_damage[ind])
			{
				if (!IS_NPC (target))
				{
					target->death_ch = ch;
					target->death_obj = ammo[0];
				}

				if (wound_to_char (target, location_table[ind], (int) res_damage[ind], (IS_SLING(firearm) ? 3 : (res_result[ind] == CAUGHT ? 3 : (res_result[ind] == GLANCING_HIT ? 12 : 10))), (int) res_bleed[ind], 0, 0))
				{
					if (ranged)
						send_to_char ("\nYour target collapses, dead.\n", ch);
					ch->ranged_enemy = NULL;
					return;
				}
				// If we didn't kill them, we apply -another- wound for the puncture hit: the wound ripping out of them.
				else
				{
					if (!shocked)
					{
						shock_to_char(target, NULL, res_location[ind], 10, (int) res_damage[ind], 1);
						shocked = true;
					}
					// For all of our group members in the room, they suffer some shock from seeing us shot.
					if (!group_shocked)
					{
						for (tch = ch->room->people; tch; tch = tch->next_in_room)
						{
							if (are_grouped(target, tch) && ch != tch)
							{
								shock_to_char(tch, NULL, 0, 10, 10, 2);
								add_overwatch(tch, ch, 3, false);
							}
						}
						group_shocked = true;
					}

					if (res_result[ind] == PUNCTURE_HIT)
					{
						if (wound_to_char (target, location_table[ind], (int) res_damage[ind], 11, (int) res_bleed[ind], 0, 0))
						{
							if (ranged)
								send_to_char ("\nYour target collapses, dead.\n", ch);
							ch->ranged_enemy = NULL;
							return;
						}
					}

				}


				if (!IS_NPC (target))
				{
					target->death_ch = NULL;
					target->death_obj = 0;
				}
			}
			else
			{
				// If we got shot at but it hit our cover or missed us, add some
				// shock damage.
				if (!miss_shocked)
				{
					if (res_result[ind] == MISS || res_result[ind] == COVER_HIT)
					{
						shock_to_char(target, NULL, res_location[ind], 10, 10, 2);
						miss_shocked = true;
					}
				}

				// Otherwise, if we got hit but didn't get wounded, we still suffer
				// some shock damage for it.
				if (!shocked)
				{
					if (res_result[ind] == MISS || res_result[ind] == COVER_HIT)
					{
						shock_to_char(target, NULL, res_location[ind], 10, 10, 2);
						shocked = true;
					}
				}

				// For all of our group members in the room, they suffer some shock from seeing us shot.
				if (!group_shocked)
				{
					for (tch = ch->room->people; tch; tch = tch->next_in_room)
					{
						if (are_grouped(target, tch) && tch != ch)
						{
							shock_to_char(tch, NULL, 0, 10, 10, 2);
							add_overwatch(tch, ch, 3, false);
						}
					}
					group_shocked = true;
				}
			}
		}

		// If we're not in the room, then everyone moving through the room needs to make a pass, or hit the deck
		// Also, for people who are in cover, they get a bonus, but can still be a little shaken.
		if (target && ch->room != target->room)
		{
			for (tch = target->room->people; tch; tch = tch->next_in_room)
			{
				if (IS_SET(tch->act, ACT_WILDLIFE) || GET_TRUST(tch) || !AWAKE(tch))
					continue;

				if ((GET_FLAG(tch, FLAG_LEAVING) || GET_FLAG(tch, FLAG_ENTERING)) || (tch->moves && !IS_SET(tch->moves->flags, MF_CRAWL)) || (get_affect(tch, AFFECT_COVER)))
				{
					// Base is our willpower.
					int base = GET_WIL(tch);

					// -12% chance if it's a small room.
					if (IS_SET(tch->room->room_flags, SMALL_ROOM))
					{
						base -= 3;
					}
					// Or +12 % if it's a big room.
					//else if (IS_SET(tch->room->room_flags, BIG_ROOM))
					//{
					//	base += 3;
					//}

					// + 4% chance for each shot fired after the first.
					base -= (fired -1);

					// -/+ 4% for each caliber off of a .35 -- bigger guns make bigger sounds and make you jump down faster
					base -= (firearm->o.firearm.caliber - 3);

					// If we're already in cover, we get a +12% bonus.
					if (get_affect(ch, AFFECT_COVER))
					{
						base += 3;
					}

					// If we roll greater on a 1d25 than our base...
					if (number(1,25) > base)
					{
						if (get_affect(ch, AFFECT_COVER))
						{
							add_second_affect(SA_INVOLCOVER, number(8,12), tch, NULL, NULL, 0);
						}
						else
						{
							add_second_affect(SA_INVOLCOVER, number(8,12), tch, NULL, NULL, 1);
							clear_moves(tch);
							GET_POS (tch) = SIT;
							add_affect (ch, AFFECT_COVER, -1, 0, 0, dir, 0, 0);

							if (fired > 1)
							{
								act("The bullets whip past too close for your nerve to hold, and you throw yourself to the ground towards some cover from the direction they came!", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
							}
							else
							{
								act("The bullet whips past too close for your nerve to hold, and you throw yourself to the ground towards some cover from the direction they came!", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
							}

							act ("$n throws $mself to the ground.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
						}
					}
				}
			}
		}

		// If we got fired at, and we're not suffering from involuntary
		// cover, and we're not already taken cover from that direction,
		//
		if (IS_SET(target->plr_flags, AUTO_COVER))
		{
			if (!target->fighting && AWAKE(target) && GET_POS(target) != POSITION_FIGHTING && target->delay_type != DEL_COVER)
			{

				clear_moves(target);
				if (target->delay)
					command_interpreter(target, "stop");

				bool already_covered = false;
				bool can_cover = true;
				int covers = 0;

				for (af = target->hour_affects; af; af = af->next)
				{
					if (af->type != AFFECT_COVER)
						continue;

					if (af->a.cover.direction == dir)
					{
						already_covered = true;
						break;
					}

					covers ++;
				}

				OBJ_DATA *obj = NULL;

				if (get_covtable_affect(target))
				{
					obj = get_covtable_affect(target)->a.table.obj;
				}

				if (!already_covered)
				{
					if (covers >= 1)
					{
						if (obj && obj->o.od.value[2])
						{
							if (i >= obj->o.od.value[2])
							{
								can_cover = false;
							}
						}
						else if (IS_SET(target->room->room_flags, HIGH_COVER))
						{
							if (i >= 5)
							{
								can_cover = false;
							}
						}
						else if (IS_SET(target->room->room_flags, MED_COVER))
						{
							if (i >= 3)
							{
								can_cover = false;
							}
						}
						else
						{
							can_cover = false;
						}
					}

					if (can_cover)
					{
						if (dir == 6)
						{
							do_cover(target, "area", 0);
						}
						else
						{
							do_cover(target, (char *) dirs[rev_dir[dir]], 0);
						}
					}
				}
			}
		}
	}

	add_overwatch(ch, target, 2, false);
	add_overwatch(target, ch, 2, false);


	/*
	if(IS_NPC(target) && IS_RIDEE(target))
	{
	dump_rider(target->mount, 2);
	}
	*/

	if (!cmd)
	{				// Cmd > 0; volley-fire, delay NPC reaction to it.
		*buf = '\0';
		if (num_fired_ch > 1)
			retal_ch = fired_ch[number (0, num_fired_ch)];
		else
			retal_ch = ch;
		sprintf (buf, "Random retal_ch: %s\n", retal_ch->name);
		if (!retal_ch)
			retal_ch = ch;
		sprintf (buf + strlen (buf), "Failsafe retal_ch: %s\n", retal_ch->name);
		npc_ranged_response (target, retal_ch);
	}


	return;
}

// Remove bullets from people.
void
	do_extract (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char location[MAX_STRING_LENGTH];
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	LODGED_OBJECT_INFO *lodged;
	int removed = 0, target_found = 0;
	int modif = 0;
	int target_char = 0;

	if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC)
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	argument = one_argument (argument, arg1);

	if (!*arg1)
	{
		send_to_char ("Extract what?\n", ch);
		return;
	}

	if (*arg1)
	{
		if ((tch = get_char_room_vis (ch, arg1)))
		{
			target_found++;
			target_char++;
		}

		if (!tch)
		{
			send_to_char ("Remove what?\n", ch);
			return;
		}

		/**
		remove target object
		**/
		if (!target_found)
		{
			tch = ch;
			sprintf (arg2, "%s", arg1);
			target_found++;
		}
		else
		{
			argument = one_argument (argument, arg2);
		}

		if (!*arg2)
		{
			send_to_char ("Extract what?\n", ch);
			return;
		}

		if (target_char)
		{
			if (GET_POS (tch) > POSITION_RESTING && IS_MORTAL (ch) && !IS_SET(tch->act, ACT_PASSIVE))
			{
				send_to_char
					("The target must be resting before you can extract a lodged item.\n",
					ch);
				return;
			}

			for (lodged = tch->lodged; lodged; lodged = lodged->next)
			{
				if (isname (arg2, vtoo (lodged->vnum)->name))
				{
					sprintf (location, "%s", lodged->location);
					if (lodged->colored == 1)
						obj = load_colored_object (lodged->vnum, lodged->var_color, lodged->var_color2, lodged->var_color3, 0, 0, 0, 0, 0, 0, 0);
					else
						obj = load_object (lodged->vnum);
					obj->count = 1;

					if (GET_ITEM_TYPE(obj) != ITEM_BULLET)
					{
						send_to_char("You should just remove that item, instead of extracting it.\n", ch);
						extract_obj(obj);
						return;
					}

					if (obj)
					{
						removed++;
					}
					break;
				}
			}

			if (removed)
			{
				if (tch == ch)
				{
					*buf = '\0';
					*buf2 = '\0';
					sprintf (buf, "You begin the excruciating work of extracting $p from your %s.", expand_wound_loc (location));

					sprintf (buf2, "$n begins extracting $p from $s %s.", expand_wound_loc (location));
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

					if ((strcmp(location, "skull")) && (strcmp(location, "reye"))
						&& (strcmp(location, "leye")) && (strcmp(location, "abdomen"))
						&& (strcmp(location, "face")) && (strcmp(location, "neck")))
						modif = 0; //not in those loctions, so normal chance
					else
						modif = 20; //in a bad spot, so a penalty to healing check
				}
				else
				{
					*buf = '\0';
					*buf2 = '\0';
					*buf3 = '\0';
					sprintf (buf, "You begin the painstaking work of extracting $p from $N's %s.", expand_wound_loc (location));

					sprintf (buf2, "$n begins extracting $p from $N's %s.", expand_wound_loc (location));
					sprintf (buf3, "$n begins extracting $p from your %s.", expand_wound_loc (location));
					act (buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
					act (buf3, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);

					if ((strcmp(location, "skull")) && (strcmp(location, "reye"))
						&& (strcmp(location, "leye")) && (strcmp(location, "abdomen"))
						&& (strcmp(location, "face")) && (strcmp(location, "neck")))
						modif = 0; //not in those loctions, so normal chance
					else
						modif = 20; //in a bad spot, so a penalty to healing check
				}

				ch->delay_ch = tch;
				ch->delay = 30;
				ch->delay_type = DEL_EXTRACT_1;
				ch->delay_info1 = 60;
				ch->delay_info2 = modif;
				ch->delay_who = str_dup(arg2);
			}
			else if (!removed)
			{
				send_to_char("You don't see that -- how could you remove it?\n", ch);
				return;
			}
		} // if (target_char)
	} //if (!obj && *arg1)
}


void
	delayed_extract_pause(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char location[AVG_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	int removed = 0;
	CHAR_DATA *tch = NULL;
	LODGED_OBJECT_INFO *lodged;

	if (!(tch = ch->delay_ch) || tch->room != ch->room)
	{
		send_to_char("Your patient is no longer here.\n", ch);
		return;
	}
	else if (!ch->delay_who)
	{
		send_to_char("You can no longer see the object you were trying to extract.\n", ch);
		return;
	}


	for (lodged = tch->lodged; lodged; lodged = lodged->next)
	{
		if (isname (ch->delay_who, vtoo (lodged->vnum)->name))
		{
			sprintf (location, "%s", lodged->location);
			if (lodged->colored == 1)
				obj = load_colored_object (lodged->vnum, lodged->var_color, lodged->var_color2, lodged->var_color3, 0, 0, 0, 0, 0, 0, 0);
			else
				obj = load_object (lodged->vnum);
			obj->count = 1;

			if (GET_ITEM_TYPE(obj) != ITEM_BULLET)
			{
				send_to_char("You should just remove that item, instead of extracting it.\n", ch);
				extract_obj(obj);
				return;
			}

			removed++;
			break;
		}
	}

	if (removed)
	{
		if (tch == ch)
		{
			*buf = '\0';
			*buf2 = '\0';
			sprintf (buf, "You continue the excruciating work of extracting $p from your %s.", expand_wound_loc (location));

			sprintf (buf2, "$n continues extracting $p from $s %s.", expand_wound_loc (location));
			act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			*buf = '\0';
			*buf2 = '\0';
			*buf3 = '\0';
			sprintf (buf, "You continue delicate task of extracting $p from $N's %s.", expand_wound_loc (location));
			sprintf (buf2, "$n continues extracting $p from $N's %s.", expand_wound_loc (location));
			sprintf (buf3, "$n continues the excruciating taks of extracting $p from your %s.", expand_wound_loc (location));
			act (buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			act (buf2, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
			act (buf3, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
		}

		ch->delay_info1 = ch->delay_info1 - 30;
		extract_obj(obj);

		if (ch->delay_info1 > 0)
		{
			ch->delay = 30;
			ch->delay_type = DEL_EXTRACT_1;
		}
		else
		{
			ch->delay = 30;
			ch->delay_type = DEL_EXTRACT_2;
		}
	}
	else if (!removed)
	{
		send_to_char("You no longer see that bullet in your patient.\n", ch);
		return;
	}

}


void
	delayed_extract(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char location[AVG_STRING_LENGTH];
	OBJ_DATA *obj = NULL;
	int removed = 0;
	CHAR_DATA *tch = NULL;
	LODGED_OBJECT_INFO *lodged;
	int roll = 0;

	if (!(tch = ch->delay_ch) || tch->room != ch->room)
	{
		send_to_char("Your patient is no longer here.\n", ch);
		return;
	}
	else if (!ch->delay_who)
	{
		send_to_char("You can no longer see the object you were trying to extract.\n", ch);
		return;
	}

	for (lodged = tch->lodged; lodged; lodged = lodged->next)
	{
		if (isname (ch->delay_who, vtoo (lodged->vnum)->name))
		{
			sprintf (location, "%s", lodged->location);
			if (lodged->colored == 1)
				obj = load_colored_object (lodged->vnum, lodged->var_color, lodged->var_color2, lodged->var_color3, 0, 0, 0, 0, 0, 0, 0);
			else
				obj = load_object (lodged->vnum);
			obj->count = 1;

			if (GET_ITEM_TYPE(obj) != ITEM_BULLET)
			{
				send_to_char("You should just remove that item, instead of extracting it.\n", ch);
				extract_obj(obj);
				return;
			}

			removed++;
			break;
		}
	}

	if (removed)
	{
		roll = number (1, SKILL_CEILING) - skill_level(ch, SKILL_MEDICINE, ch->delay_info2);

		skill_use(ch, SKILL_MEDICINE, 0);
		skill_use(ch, SKILL_MEDICINE, 0);

		*buf = '\0';
		*buf2 = '\0';
		*buf3 = '\0';

		// Serious fall - botch by more than twenty, you open up a new wound
		if (roll > 20)
		{
			if (tch == ch)
			{
				sprintf (buf,  "You messily extract $p from your %s, fresh blood pouring forth from the wound", expand_wound_loc (location));
				sprintf (buf2, "$n messily extracts $p from $s %s, fresh blood pouring forth from the wound.", expand_wound_loc (location));

			}
			else
			{
				sprintf (buf,  "You messily extract $p from $N's %s, fresh blood pouring forth from the wound.", expand_wound_loc (location));
				sprintf (buf2, "$n messily extracts $p from $N's %s, fresh blood pouring forth from the wound.", expand_wound_loc (location));
				sprintf (buf3, "$n messily extracts $p from your %s, fresh blood pouring forth from the wound.", expand_wound_loc (location));
			}

			wound_to_char (tch, location, 5, 10, 5, 0, 0);

			/*
			if (!(af = get_affect(tch, AFFECT_INTERNAL)))
			{
			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
			af->type = AFFECT_INTERNAL;
			af->a.bleeding.duration = -1;
			af->a.bleeding.rate = 1;
			af->next = NULL;
			affect_to_char(tch, af);
			}
			else
			{
			af->a.bleeding.rate += 1;
			}
			*/
		}
		else if (roll > 0)
		{
			if (tch == ch)
			{
				sprintf (buf,  "You crudely extract $p from your %s, removing the bullet but further damaging your innards.", expand_wound_loc (location));
				sprintf (buf2, "$n crudely extracts $p from $s %s.", expand_wound_loc (location));

			}
			else
			{
				sprintf (buf, "You crudely extract $p from $N's %s, removing the bullet but further damaging $S innards.", expand_wound_loc (location));
				sprintf (buf2, "$n crudely extracts $p from $N's %s.", expand_wound_loc (location));
				sprintf (buf3, "$n crudely extracts $p from your %s.", expand_wound_loc (location));
			}


			/*
			if (!(af = get_affect(tch, AFFECT_INTERNAL)))
			{
			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
			af->type = AFFECT_INTERNAL;
			af->a.bleeding.duration = -1;
			af->a.bleeding.rate = 1;
			af->next = NULL;
			affect_to_char(tch, af);
			}
			else
			{
			af->a.bleeding.rate += 1;
			}
			*/
		}
		else if (roll <= -20)
		{
			if (tch == ch)
			{
				sprintf (buf,  "You masterfully extract $p from your %s, stemming some of your internal bloodloss.", expand_wound_loc (location));
				sprintf (buf2, "$n masterfully extracts $p from $s %s.", expand_wound_loc (location));

			}
			else
			{
				sprintf (buf,  "You masterfully extract $p from $N's %s, stemming some of $S internal bloodloss.", expand_wound_loc (location));
				sprintf (buf2, "$n masterfully extracts $p from $N's %s.", expand_wound_loc (location));
				sprintf (buf3, "$n masterfully extracts $p from your %s.", expand_wound_loc (location));
			}

			/*
			if ((af = get_affect(tch, AFFECT_INTERNAL)))
			{
			af->a.bleeding.rate -= 1;

			if (af->a.bleeding.rate <= 0)
			{
			affect_remove(tch, af);
			}
			}
			*/
		}
		else
		{
			if (tch == ch)
			{
				sprintf (buf,  "You neatly extract $p from your %s.", expand_wound_loc (location));
				sprintf (buf2, "$n neatly extracts $p from $s %s.", expand_wound_loc (location));

			}
			else
			{
				sprintf (buf,  "You neatly extract $p from $N's %s.", expand_wound_loc (location));
				sprintf (buf2, "$n neatly extracts $p from $N's %s.", expand_wound_loc (location));
				sprintf (buf3, "$n neatly extracts $p from your %s.", expand_wound_loc (location));
			}
		}

		if (tch == ch)
		{
			act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		else
		{

			act (buf, false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			act (buf2, false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
			act (buf3, false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
		}

		int error = 0;

		if (can_obj_to_inv (obj, ch, &error, 1))
			obj_to_char (obj, ch);
		else
			obj_to_room (obj, ch->in_room);

		lodge_from_char (tch, lodged);
	}
	else if (!removed)
	{
		send_to_char("You no longer see that bullet in your patient.\n", ch);
		return;
	}
}

