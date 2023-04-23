/*------------------------------------------------------------------------\
|  objects.c : Object Module                          www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

// 3/8/09: tastes added to eating.
// 3/8/09: first ocue is added: on_grab, when you pick something up off the ground or container.
// 2/8/09: changed rend command to take vnums for objects, and have silent error messages to tie in with room-prog. - K


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <fstream>  
#include <vector>
#include <iterator>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"

extern rpie::server engine;

const char *weapon_theme[] =
{ "stab", "pierce", "chop", "bludgeon", "slash", "lash", "burn", "shot" };

const char *fight_damage[] =
{ "stab", "pierce", "chop", "crush", "slash", "chill", "burn", "bite", "claw", "bullet", "fist", "\n" };

// Determines the material type of a given object for keepers; called by GET_KEEPER_MATERIAL_TYPE().

int
	determine_keeper_material (OBJ_DATA * obj)
{
	char buf[MAX_STRING_LENGTH];
	unsigned int i = 0, j = 0;

	for (i = 0; *materials[i] != '\n'; i++)
	{
		sprintf (buf, "%s", materials[i]);
		for (j = 0; j <= strlen (buf); j++)
		{
			buf[j] = toupper (buf[j]);
		}
		if (isnamec (buf, obj->name))
			return (1 << i);
	}

	return (1 << 0);
}

// Determines the material type of a given object; called by GET_KEEPER_TYPE().

int
	determine_material (OBJ_DATA * obj)
{
	char buf[MAX_STRING_LENGTH];
	unsigned int i = 0, j = 0;

	for (i = 0; *materials[i] != '\n'; i++)
	{
		sprintf (buf, "%s", materials[i]);
		for (j = 0; j <= strlen (buf); j++)
		{
			buf[j] = toupper (buf[j]);
		}
		if (isnamec (buf, obj->name))
			return (i);
	}

	return (MAT_OTHER);
}

/*------------------------------------------------------------------------\
|  do_rend()                                                              |
|                                                                         |
|  User command to tear into an object or player                          |
\------------------------------------------------------------------------*/
void
	do_rend (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	int num_args = 0;
	unsigned int impact = 0;
	unsigned int infection = 0;
	unsigned int bleed = 0;
	int type = -1;
	int wound_loc = -1;
	int onum = 0;
	//float dam = 0;
	char buf[13][MAX_STRING_LENGTH / 13];
	char *str_damage_sdesc = NULL;
	OBJ_DATA *insert_obj = NULL;
	OBJ_DATA *target_obj = NULL;
	OBJECT_DAMAGE *damage = NULL;
	CHAR_DATA *target_obj_ch = ch;	/* in possession of target_obj */
	CHAR_DATA *victim = NULL;
	CHAR_DATA *tch = NULL;
	bool room_target = false;
	char *error[] =
	{
		"\n"
		"Usage: rend [OPTIONS] object  or  rend [OPTIONS] victim\n"
		"\n"
		"  -d IMPACT         - Number of damage points.\n"
		"  -t TYPE           - See 'tag damage-types' for values.\n"
		"  -b BODYLOC        - See 'tag woundlocs' for values.\n"
		"  -o OBJECT         - Will insert OBJECT into the victim's wound.\n"
		"  -c CHARACTER      - Will damage the specified object on CHARACTER.\n"
		"  -i RATE           - Determines whether the wound is infected or not.\n"
		"  -z BLEED          - How much will this wound bleed?\n"
		"\n"
		"Examples:\n"
		"  > rend tunic                             - Apply random damage to an obj\n"
		"  > rend -d 12 -t stab vest                - Specific damage to an obj.\n"
		"  > rend -t bloodstain -c traithe gloves   - Damage someone elses obj.\n"
		"\n"
		"  > rend dravage                           - Apply random wound to char.\n"
		"  > rend -d 12 -t stab -b leye mirudus     - Specific damage to an char.\n"
		"  > rend -b head -o arrow tusken           - Wound char with inserted item.\n"
		"  > rend -b rthigh -t stab all             - Damages everyone in the room.\n" ,
		"#1Please specify an object or character to rend.#0\n",
		"#1Damage is raw hit points and doesn't account for armour. Without a value, it does a random small amount.#0\n",
		"#1Unknown attack type, refer to 'tags object-damage' for a list of values.#0\n",
		"#1Unknown wound location, refer to 'tags woundlocs' for a list of values.#0\n",
		"#1You don't see the subject of the -o option.#0\n",
		"#1You don't see the subject of the -c option.#0\n",
		"#1You don't see that target object or victim.#0\n",
		"#1You can't use the -c option with a victim.#0\n",
		"#1You can't use the -b option with a target object.#0\n",
		"#1You can't use the -i option with a target object.#0\n"
		"#1You can't use the -z option with a target object.#0\n"
	};

	if (!GET_TRUST (ch) && cmd != 2)
	{
		return;
	}

	if (!(num_args = sscanf (argument, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
		buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15], buf[16])))
	{

		if (cmd != 2)
			send_to_char (error[0], ch);
		return;
	}

	for (i = 0; i < num_args; i++)
	{
		if (strcmp (buf[i], "-d") == 0)
		{
			if ((i == num_args - 1) || !sscanf (buf[i + 1], "%d", &impact)
				|| impact < 0)
			{
				if (cmd != 2)
				{
					send_to_char (error[2], ch);
					send_to_char (error[0], ch);
				}
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-i") == 0)
		{
			if ((i == num_args - 1) || !sscanf (buf[i + 1], "%d", &infection)
				|| impact < 0 || impact > 100)
			{
				if (cmd != 2)
					send_to_char (error[0], ch);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-z") == 0)
		{
			if ((i == num_args - 1) || !sscanf (buf[i + 1], "%d", &bleed)
				|| impact < 0 || impact > 100)
			{
				if (cmd != 2)
					send_to_char (error[0], ch);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-t") == 0)
		{
			if ((i == num_args - 1)
				|| (type = index_lookup (fight_damage, buf[i + 1])) < 0)
			{
				if (cmd != 2)
				{
					send_to_char (error[3], ch);
					send_to_char (error[0], ch);
				}
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-b") == 0)
		{
			if ((i == num_args - 1)
				|| (wound_loc = index_lookup (wound_locations, buf[i + 1])) < 0)
			{
				if (cmd != 2)
				{
					send_to_char (error[4], ch);
					send_to_char (error[0], ch);
				}
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-o") == 0)
		{
			if (isdigit (*buf[i+1]))
				onum = atoi (buf[i+1]);

			if ((i == num_args - 1) || !*buf[i + 1]
			|| !((insert_obj = get_obj_in_dark (ch, buf[i + 1], ch->right_hand))
				||   (insert_obj = get_obj_in_dark (ch, buf[i + 1], ch->left_hand))
				||   (insert_obj = get_obj_in_dark (ch, buf[i + 1], ch->equip))
				||   (insert_obj = get_obj_in_list_vis (ch, buf[i + 1], ch->room->contents))
				||   (onum && (insert_obj = load_object (onum))) ))
			{

				if (cmd != 2)
					send_to_char (error[5], ch);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-c") == 0)
		{
			if ((i == num_args - 1) || !*buf[i + 1]
			|| !(target_obj_ch = get_char_room_vis (ch, buf[i + 1])))
			{
				if (cmd != 2)
					send_to_char (error[6], ch);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else
		{
			if (*buf[i] && !strcmp (buf[i], "all"))
				room_target = true;
			else if (!*buf[i]
			|| !((victim = get_char_room_vis (ch, buf[i]))

				|| (!strcmp (buf[i], "all"))

				|| (target_obj =
				get_obj_in_dark (target_obj_ch, buf[i],
				target_obj_ch->right_hand))
				|| (target_obj =
				get_obj_in_dark (target_obj_ch, buf[i],
				target_obj_ch->left_hand))
				|| (target_obj =
				get_obj_in_dark (target_obj_ch, buf[i],
				target_obj_ch->equip))
				|| (target_obj =
				get_obj_in_list_vis (target_obj_ch, buf[i],
				target_obj_ch->room->contents))))
			{
				if (cmd != 2)
					send_to_char (error[7], ch);
				return;
			}
		}
	}

	if (!target_obj && !victim && !room_target)
	{
		if (cmd != 2)
		{
			send_to_char (error[7], ch);
			send_to_char (error[0], ch);
		}
		return;
	}
	else if (victim && target_obj_ch != ch)
	{
		if (cmd != 2)
		{
			send_to_char (error[8], ch);
			send_to_char (error[0], ch);
		}
		return;
	}
	else if (target_obj && wound_loc >= 0)
	{
		if (cmd != 2)
		{
			send_to_char (error[9], ch);
			send_to_char (error[0], ch);
		}
		return;
	}
	else if (target_obj && infection > 0)
	{
		if (cmd != 2)
		{
			send_to_char (error[10], ch);
			send_to_char (error[0], ch);
		}
		return;
	}


	if (victim)
	{
		impact = (impact <= 0) ? number (1, 10) : impact;
		wound_loc = wound_loc >= 0 ? wound_loc : number(0,26);
		type = (type < 0) ? number (0, 10) : type;
		sprintf (buf[0],
			"Dam: %d of %d hits\nType: %s\nBodyLoc: %s\nInsert: %s\nInto Victim: %s\n",
			impact, victim->max_hit, fight_damage[type],
			(wound_loc >= 0) ? wound_locations[wound_loc] : "Random",
			(insert_obj) ? insert_obj->short_description : "None",
			victim->short_descr);
		if (cmd != 2)
			act (buf[0], false, ch, target_obj, 0, TO_CHAR | _ACT_FORMAT);

		wound_to_char(victim, add_hash(wound_locations[wound_loc]), impact, type, bleed, 0, infection);

		if (insert_obj && insert_obj->short_description)
		{
			lodge_missile(victim, insert_obj, add_hash(wound_locations[wound_loc]), 0);
		}


	}
	else if (room_target)
	{
		impact = (impact <= 0) ? number (1, 10) : impact;
		wound_loc = wound_loc >= 0 ? wound_loc : number(0,26);
		type = (type < 0) ? number (0, 10) : type;
		sprintf (buf[0],
			"Dam: %d \nType: %s\nBodyLoc: %s\nInsert: %s\nInto All N/PCs in Room\n",
			impact, damage_type[type],
			(wound_loc >= 0) ? wound_locations[wound_loc] : "Random",
			(insert_obj) ? insert_obj->short_description : "None");
		if (cmd != 2)
			act (buf[0], false, ch, target_obj, 0, TO_CHAR | _ACT_FORMAT);

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			impact = (impact <= 0) ? number (1, 10) : impact;
			wound_to_char(tch, add_hash(wound_locations[wound_loc]), impact, type, bleed, 0, infection);

			if (insert_obj && insert_obj->short_description)
			{
				lodge_missile(tch, insert_obj, add_hash(wound_locations[wound_loc]), 0);
			}
		}
	}
	else
	{
		impact = (impact <= 0) ? number (1, 6) : impact;
		type = (type < 0) ? number (0, 7) : type;

		if ((damage =
			object__add_damage (target_obj, type,
			(unsigned int) impact))
			&& (str_damage_sdesc = object_damage__get_sdesc (damage)))
		{

			sprintf (buf[0],
				"You concentrate on %s until #1%s#0 appears on %s.",
				(target_obj_ch != ch) ? "$N" : "$p", str_damage_sdesc,
				(target_obj_ch != ch) ? "$p" : "#3it#0");
			sprintf (buf[1], "You notice #1%s#0 on $p.", str_damage_sdesc);
			mem_free (str_damage_sdesc);

		}
		else
		{
			sprintf (buf[0],
				"You concentrate on %s but nothing seems to happen to %s.",
				(target_obj_ch != ch) ? "$N" : "$p",
				(target_obj_ch != ch) ? "$p" : "#2it#0");
			buf[1][0] = '\0';
		}
		act (buf[0], false, ch, target_obj, target_obj_ch,
			TO_CHAR | _ACT_FORMAT);
		if ((target_obj_ch != ch) && buf[1][0])
			act (buf[1], false, target_obj_ch, target_obj, 0,
			TO_CHAR | _ACT_FORMAT);

		if (object__determine_condition(target_obj) == 5)
		{
			act("$p crumbles away before your very eyes.", false, ch, target_obj, 0, TO_CHAR | _ACT_FORMAT);
			extract_obj(target_obj);
		}

	}

}

void
	object__add_enviro(OBJ_DATA *obj, int type, int amount)
{
	if (!obj)
		return;

	// Immortals don't worry.
	if (obj->equiped_by && !IS_MORTAL(obj->equiped_by))
		return;

	// Only these things suffer water damage...
	if (type == COND_WATER)
	{
		// 1 pt of water removes 3 pts of dust, adds 1/2 a point of dirt. Yay water!
		obj->enviro_conds[COND_DUST] -= amount * 3;
		obj->enviro_conds[COND_DUST] = MIN(obj->enviro_conds[COND_DUST], 99);
		obj->enviro_conds[COND_DUST] = MAX(obj->enviro_conds[COND_DUST], 0);
		obj->enviro_conds[COND_DIRT] += amount / 2;
		obj->enviro_conds[COND_DIRT] = MIN(obj->enviro_conds[COND_DIRT], 99);
		obj->enviro_conds[COND_DIRT] = MAX(obj->enviro_conds[COND_DIRT], 0);

		if (GET_MATERIAL_TYPE(obj) != MATERIAL_CERAMIC &&
			GET_MATERIAL_TYPE(obj) != MATERIAL_TEXTILE &&
			GET_MATERIAL_TYPE(obj) != MATERIAL_METAL &&
			GET_MATERIAL_TYPE(obj) != MATERIAL_ORGANIC &&
			GET_MATERIAL_TYPE(obj) != MATERIAL_ELECTRONIC)
		{
			obj->enviro_conds[type] = 0;
			return;
		}
	}
	// Only metal suffers rust.
	else if (type == COND_RUST)
	{
		if (
			GET_MATERIAL_TYPE(obj) != MATERIAL_METAL)
		{
			obj->enviro_conds[type] = 0;
			return;
		}
	}
	else if (type == COND_BLOOD)
	{
		obj->enviro_conds[COND_DUST] -= amount;
		obj->enviro_conds[COND_DUST] = MIN(obj->enviro_conds[COND_DUST], 99);
		obj->enviro_conds[COND_DUST] = MAX(obj->enviro_conds[COND_DUST], 0);
		add_scent(obj, scent_lookup("the metallic tang of blood"), 1, amount, 200, amount, 0);
	}

	obj->enviro_conds[type] += amount;
	obj->enviro_conds[type] = MIN(obj->enviro_conds[type], 99);
	obj->enviro_conds[type] = MAX(obj->enviro_conds[type], 0);

}

/*--------------------------------------------------------------------------\
|  object__enviro()                                                         |
|                                                                           |
|  Iterate through the object list or character and applies the enviroType. |
|  If a ch but no obj, then hits all or the wearloc.                        |
|  If an obj, hits that one.                                                |
\--------------------------------------------------------------------------*/
void
	object__enviro (CHAR_DATA * ch, OBJ_DATA * obj, int type, int amount, int wearloc)
{
	OBJ_DATA *tobj = NULL;

	if (!ch && !obj)
		return;

	if (ch)
	{
		// Immortals don't worry.
		if (!IS_MORTAL(ch))
			return;

		int count = 0;
		// If we haven't specified a hitloc, then we just slather it on randomly to everything we're wearing.
		if (wearloc < HITLOC_BODY || wearloc > MAX_HITLOC)
		{
			for (tobj = ch->equip; tobj != NULL; tobj = tobj->next_content)
			{
				object__add_enviro(tobj, type, amount);
			}

			if (ch->left_hand)
				object__add_enviro(ch->left_hand, type, amount);

			if (ch->right_hand)
				object__add_enviro(ch->right_hand, type, amount);

			return;
		}
		// If it is for a wearloc, then we count how many of those wearlocs we are wearing, divide our
		// amount up by that, and then slather it on selectively.
		else
		{
			for (tobj = ch->equip; tobj != NULL; tobj = tobj->next_content)
			{
				if (IS_SET(tobj->o.od.value[2], 1 << wearloc) || IS_SET(tobj->o.od.value[3], 1 << wearloc))
				{
					count++;
				}
			}

			if (count)
			{
				amount = amount / count;
				amount = MAX(amount, 1);

				for (tobj = ch->equip; tobj != NULL; tobj = tobj->next_content)
				{
					if (IS_SET(tobj->o.od.value[2], 1 << wearloc) || IS_SET(tobj->o.od.value[3], 1 << wearloc))
					{
						object__add_enviro(tobj, type, amount);
					}
				}
			}
			return;
		}
	}
	else if (obj)
	{
		object__add_enviro(obj, type, amount);
	}
}


/*------------------------------------------------------------------------\
|  object__drench()                                                       |
|                                                                         |
|  Iterate through the object list and apply water damage where necessary |
\------------------------------------------------------------------------*/
void
	object__drench (CHAR_DATA * ch, OBJ_DATA * _obj, bool isChEquip)
{
	OBJ_DATA *obj;

	if (_obj != NULL)
	{

		for (obj = _obj; obj != NULL; obj = obj->next_content)
		{

			/* Lights get extinguished in water */

			if ((GET_ITEM_TYPE (obj) == ITEM_LIGHT || GET_ITEM_TYPE (obj) == ITEM_CHEM_SMOKE) && !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
			{

				/* Lights out */
				if (obj->o.light.on)
				{
					act ("$p is extinguished.", false, 0, obj, ch,
						TO_ROOM | TO_CHAR | _ACT_FORMAT);
					obj->o.light.on = false;
				}
			}

			// Things get wet and fast.
			object__add_enviro(obj, COND_WATER, 26);

			// Non-water proof items, if not carried, get damaged when entering water.
			if (!ch)
			{
				electrical_water_damage(NULL, obj, 0);
			}
			else if (ch)
			{
				if (obj != ch->right_hand && obj != ch->left_hand)
				{
					electrical_water_damage(ch, obj, 0);
				}

			}
		}

	}
	if (isChEquip)
	{
		object__drench (ch, ch->left_hand, false);
		object__drench (ch, ch->right_hand, false);
	}
}

// How much damage points have we sustained?

int
	object__count_damage(OBJ_DATA *thisPtr)
{
	OBJECT_DAMAGE *damage = NULL;
	int calamity = 0;

	if (!thisPtr)
		return 0;

	for (damage = thisPtr->damage; damage; damage = damage->next)
		calamity += damage->impact;

	return calamity;
}

/*------------------------------------------------------------------------\
|  determine_condition()                                                  |
|                                                                         |
|  Determines which tier an object is in by its condition                 |
\------------------------------------------------------------------------*/

int
	object__determine_condition (OBJ_DATA * thisPtr)
{
	OBJECT_DAMAGE *damage;
	int calamity = 0;
	int quality = 0;
	int condition = 0;
	int tier = 0;
	//char buf[MAX_STRING_LENGTH];

	/* TODO: Remove this when we're ready to go live with damage */
	if (engine.in_build_mode ())
		return 0;

	/* Iterate through the damage instances attached to this object */
	for (damage = thisPtr->damage; damage; damage = damage->next)
		calamity += damage->impact;

	quality = thisPtr->quality;
	condition = quality - calamity;

	// The reported tier is the difference between the damage tier of the quality and the damage tier of the condition.
	// In general:
	// > 80     : Good
	// 79  - 50 : Fair
	// 49  - 20 : Poor
	// 19  - 0  : Bad
	// < 0      : Attrocious.
	// This is used so an item that is naturally "fair" doesn't report as being in "fair" condition,
	// or that you look at a "broken, shoddy dagger" and get the message that it is in perfect condition.
	// For armour, all values are doubled.
	//
	// For weapons and firearms, we have the following values:
	// > 900    : Good
	// 800 - 899: Fair
	// 700 - 799: Poor
	// 0   - 699  : Bad
	// < 0      : Attrocious.

	if (GET_ITEM_TYPE(thisPtr) == ITEM_WEAPON || GET_ITEM_TYPE(thisPtr) == ITEM_FIREARM)
	{
		if (condition > 900)
			tier = 0;
		else if (condition > 800)
			tier = 1;
		else if (condition > 700)
			tier = 2;
		else if (condition > 0)
			tier = 3;
		else
			tier = 4;

		if (quality <= 0)
			tier -= 4;
		else if (quality <= 700)
			tier -= 3;
		else if (quality <= 800)
			tier -= 2;
		else if (quality <= 900)
			tier -= 1;

		if (tier < 0)
			tier = 0;
	}
	else if (GET_ITEM_TYPE(thisPtr) == ITEM_ARMOR || GET_ITEM_TYPE(thisPtr) == ITEM_SHIELD)
	{
		if (condition > 160)
			tier = 0;
		else if (condition > 100)
			tier = 1;
		else if (condition > 40)
			tier = 2;
		else if (condition > 0)
			tier = 3;
		else
			tier = 4;

		if (quality <= 0)
			tier -= 4;
		else if (quality <= 40)
			tier -= 3;
		else if (quality <= 100)
			tier -= 2;
		else if (quality <= 160)
			tier -= 1;

		if (tier < 0)
			tier = 0;
	}
	else
	{
		if (condition > 80)
			tier = 0;
		else if (condition > 50)
			tier = 1;
		else if (condition > 20)
			tier = 2;
		else if (condition > 0)
			tier = 3;
		else
			tier = 4;

		if (quality <= 0)
			tier -= 4;
		else if (quality <= 20)
			tier -= 3;
		else if (quality <= 50)
			tier -= 2;
		else if (quality <= 80)
			tier -= 1;

		if (tier < 0)
			tier = 0;
	}

	// If condition is less than or equal to 100, then it should be broken.
	if (condition <= -100)
		tier = 5;

	// Testing tool to determine the quality of an object.
	//sprintf(buf, "%d %d %d", condition, quality, tier);
	//send_to_gods(buf);

	return tier;

}



/*------------------------------------------------------------------------\
|  get_material()                                                         |
|                                                                         |
|  Try to get a material used to compose this object instance.            |
\------------------------------------------------------------------------*/
MATERIAL_TYPE
	object__get_material (OBJ_DATA * thisPtr)
{
	switch (GET_MATERIAL_TYPE(thisPtr))
	{
	case MAT_ORGANIC:
		return MATERIAL_ORGANIC;
	case MAT_PLASTIC:
		return MATERIAL_PLASTICS;
	case MAT_METAL:
		return MATERIAL_METAL;
	case MAT_TEXTILE:
		return MATERIAL_TEXTILE;
	case MAT_CERAMIC:
		return MATERIAL_CERAMIC;
	case MAT_GLASS:
		return MATERIAL_GLASS;
	case MAT_ELECTRONIC:
		return MATERIAL_ELECTRONIC;

	default:
		return MATERIAL_TYPE_UNDEFINED;
	}
}




/*------------------------------------------------------------------------\
|  examine_damage()                                                       |
|                                                                         |
|  Iterate through the damage list and show the sdesc of each instance.   |
\------------------------------------------------------------------------*/
char *
	object__examine_damage (OBJ_DATA * thisPtr, int cmd)
{
	OBJECT_DAMAGE *damage;
	char *p = '\0', *str_damage_sdesc = '\0';
	static char buf[MAX_STRING_LENGTH] = {'\0'};
	static char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	char buf4[MAX_STRING_LENGTH] = {'\0'};
	int tier = 0;
	int scuffs = 0;
	int scuff_index = 0;
	int worn;
	int worn_index = 0;
	int i = 0;

	*buf = '\0';

	/* Not point cluttering up the BP. */
	if (engine.in_build_mode ())
		return buf;

	// Looking, or firearms and weapons, just return the overall condition.
	if (cmd == 1)
	{
		bool enviro_cond = false;
		bool enviro_remains = false;

		int enviro_tier = 0;

		// Find the highest enviro_cond provided it's greater than 25.
		for (int ind = 0; ind < COND_TYPE_MAX; ind++)
		{
			enviro_remains = false;
			if (thisPtr->enviro_conds[ind] > 10)
			{
				for (int xind = ind + 1; xind < COND_TYPE_MAX; xind++)
				{
					if (thisPtr->enviro_conds[xind] > 10)
					{
						enviro_remains = true;
						break;
					}
				}

				enviro_tier = thisPtr->enviro_conds[ind] > 75 ? 4 : thisPtr->enviro_conds[ind] > 50 ? 3 : thisPtr->enviro_conds[ind] > 25 ? 2 : 1;

				if (!enviro_cond)
					sprintf (buf3, "It is %s", enviro_desc[ind][enviro_tier]);
				else if (!enviro_remains)
					sprintf (buf3 + strlen(buf3), " and %s", enviro_desc[ind][enviro_tier]);
				else
					sprintf(buf3 + strlen(buf3), ", %s", enviro_desc[ind][enviro_tier]);

				enviro_cond = true;
			}
		}
		if (enviro_cond)
		{
			strcat(buf3, ". ");
		}

		if (thisPtr->scent)
		{
			int scent_count = 0;
			SCENT_DATA *scent = NULL;
			SCENT_DATA *scent_one = NULL;
			SCENT_DATA *scent_two = NULL;
			SCENT_DATA *scent_three = NULL;

			for (scent = thisPtr->scent; scent; scent = scent->next)
			{
				if (scent_strength(scent) >= 2)
				{
					if (!scent_one)
					{
						scent_one = scent;
						scent_count++;
					}
					else if (!scent_two)
					{
						scent_two = scent;
						scent_count++;
					}
					else if (!scent_three)
					{
						scent_three = scent;
						scent_count++;
					}
					else if (scent_strength(scent) > scent_strength(scent_one))
					{
						scent_one = scent;
					}
					else if (scent_strength(scent) > scent_strength(scent_two))
					{
						scent_two = scent;
					}
					else if (scent_strength(scent) > scent_strength(scent_two))
					{
						scent_three = scent;
					}
					else
						continue;
				}
			}

			if (scent_count == 3)
			{
				sprintf(buf4, "It has %s of %s, %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
			}
			else if (scent_count == 2)
			{
				sprintf(buf4, "It has %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
			}
			else if (scent_count == 1)
			{
				sprintf(buf4, "It has %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
			}
		}

		tier = object__determine_condition(thisPtr);
		if (!thisPtr->damage)
		{
			sprintf (buf2, "\n   #6%s%sIt is in good condition.#0\n", buf4, buf3);
		}
		else
		{
			sprintf (buf2, "\n   #6%s%s%s.#0\n", buf4, buf3, conditions[object__get_material(thisPtr)][tier]);
		}
		reformat_string (buf2, &p);
		sprintf (buf2, "\n%s", p);
		return buf2;
	}
	else
	{
		bool enviro_cond = false;
		bool enviro_remains = false;

		int enviro_tier = 0;

		// Find the highest enviro_cond provided it's greater than 25.
		for (int ind = 0; ind < COND_TYPE_MAX; ind++)
		{
			enviro_remains = false;
			if (thisPtr->enviro_conds[ind] > 10)
			{
				for (int xind = ind + 1; xind < COND_TYPE_MAX; xind++)
				{
					if (thisPtr->enviro_conds[xind] > 10)
					{
						enviro_remains = true;
						break;
					}
				}

				enviro_tier = thisPtr->enviro_conds[ind] > 75 ? 4 : thisPtr->enviro_conds[ind] > 50 ? 3 : thisPtr->enviro_conds[ind] > 25 ? 2 : 1;

				if (!enviro_cond)
					sprintf (buf3, "It is %s", enviro_desc[ind][enviro_tier]);
				else if (!enviro_remains)
					sprintf (buf3 + strlen(buf3), " and %s", enviro_desc[ind][enviro_tier]);
				else
					sprintf(buf3 + strlen(buf3), ", %s", enviro_desc[ind][enviro_tier]);

				enviro_cond = true;
			}
		}
		if (enviro_cond)
		{
			strcat(buf3, ". ");
		}

		if (thisPtr->scent)
		{
			int scent_count = 0;
			SCENT_DATA *scent = NULL;
			SCENT_DATA *scent_one = NULL;
			SCENT_DATA *scent_two = NULL;
			SCENT_DATA *scent_three = NULL;

			for (scent = thisPtr->scent; scent; scent = scent->next)
			{
				if (scent_strength(scent) >= 2)
				{
					if (!scent_one)
					{
						scent_one = scent;
						scent_count++;
					}
					else if (!scent_two)
					{
						scent_two = scent;
						scent_count++;
					}
					else if (!scent_three)
					{
						scent_three = scent;
						scent_count++;
					}
					else if (scent_strength(scent) > scent_strength(scent_one))
					{
						scent_one = scent;
					}
					else if (scent_strength(scent) > scent_strength(scent_two))
					{
						scent_two = scent;
					}
					else if (scent_strength(scent) > scent_strength(scent_two))
					{
						scent_three = scent;
					}
					else
						continue;
				}
			}

			if (scent_count == 3)
			{
				sprintf(buf4, "It has %s of %s, %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
			}
			else if (scent_count == 2)
			{
				sprintf(buf4, "It has %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
			}
			else if (scent_count == 1)
			{
				sprintf(buf4, "It has %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
			}
		}

		/* Iterate through the damage instances attached to this object */
		for (damage = thisPtr->damage; damage; damage = damage->next)
		{
			if (damage->severity <= 0 || damage-> impact <= 0)
			{
				if (damage->permanent)
				{
					worn += damage->impact;
				}
				else
				{
					scuffs += damage->impact;
				}
			}
			else if ((str_damage_sdesc = object_damage__get_sdesc (damage)) != NULL)
			{
				sprintf (buf + strlen(buf), "%s%s%s", (!damage->next && damage != thisPtr->damage && i ? " and " : " "), str_damage_sdesc, (!damage->next && damage != thisPtr->damage && i ? "." : ","));
				mem_free (str_damage_sdesc);

				i++;
			}
		}

		tier = object__determine_condition(thisPtr);

		if (scuffs > 41)
			scuff_index = 5;
		else if (scuffs > 31)
			scuff_index = 4;
		else if (scuffs > 21)
			scuff_index = 3;
		else if (scuffs > 11)
			scuff_index = 2;
		else if (scuffs >= 1)
			scuff_index = 1;
		else scuff_index = 0;

		if (worn > 500)
			worn_index = 5;
		else if (worn > 250)
			worn_index = 4;
		else if (worn > 150)
			worn_index = 3;
		else if (worn > 100)
			worn_index = 2;
		else if (worn >= 50)
			worn_index = 1;
		else worn_index = 0;

		if (!*buf)
			sprintf(buf2, "   #6%s%s%s%s%s.#0", buf4, buf3, conditions[0][tier], (scuff_index ? scuffs_desc[scuff_index] : ""), (worn_index ? worn_desc[worn_index] : ""));
		else
			sprintf(buf2, "   #6%s%s%s%s%s. It has%s#0", buf4, buf3, conditions[0][tier], (scuff_index ? scuffs_desc[scuff_index] : ""), (worn_index ? worn_desc[worn_index] : ""), buf);

		reformat_string (buf2, &p);
		sprintf (buf2, "\n%s", p);
		return buf2;

	}
}

/*------------------------------------------------------------------------\
|  add_damage()                                                           |
|                                                                         |
|  Try to get a material used to compose this object instance.            |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
	object__add_damage (OBJ_DATA * thisPtr, int source,
	unsigned int impact)
{
	DAMAGE_TYPE type;
	OBJECT_DAMAGE *damage = NULL;
	int armour_damage = 0;
	int old_tier = 0;

	/* TODO: Remove this when we're ready to go live with damage */
	if (engine.in_build_mode ())
		return NULL;

	// Go through and find the correct translation from damage type to affect. Default to fist.
	if (source == 2 || source == 4 || source == 8)
		type = DAMAGE_SLASH;
	else if (source == 0 || source == 1 || source == 7)
		type = DAMAGE_PIERCE;
	else if (source == 3)
		type = DAMAGE_BLUNT;
	else if (source == 5)
		type = DAMAGE_FREEZE;
	else if (source == 6)
		type = DAMAGE_BURN;
	else if (source == 9)
		type = DAMAGE_BULLET;
	else
		type = DAMAGE_FIST;

	// Weapons and firearms always get type permanent.
	if (GET_ITEM_TYPE(thisPtr) == ITEM_WEAPON || GET_ITEM_TYPE(thisPtr) == ITEM_FIREARM)
	{
		type = DAMAGE_PERMANENT;
	}

	if (GET_ITEM_TYPE(thisPtr) == ITEM_ARMOR && type == DAMAGE_BULLET)
		armour_damage = 1;

	if (thisPtr->dec_desc && thisPtr->dec_style)
	{
		old_tier = object__determine_condition(thisPtr);
	}

	if ((damage = object_damage__new_init (type, impact,object__get_material (thisPtr), 0, armour_damage)))
	{
		damage->next = thisPtr->damage;
		thisPtr->damage = damage;
		//thisPtr->item_wear -= damage->impact;
	}

	if (thisPtr->dec_desc && thisPtr->dec_style)
	{
		// We can lose our decoration if we get too badly beaten up.

		if (object__determine_condition(thisPtr) - old_tier > 0)
			thisPtr->dec_condition += 1;

		if (thisPtr->dec_condition > 4)
		{
			mem_free(thisPtr->dec_desc);
			thisPtr->dec_desc = NULL;
			mem_free(thisPtr->dec_style);
			thisPtr->dec_style = NULL;
			thisPtr->dec_short = 0;
			thisPtr->dec_quality = 0;
			thisPtr->dec_size = 0;
			thisPtr->dec_condition = 0;
		}
	}

	return damage;
}


void
	do_grip (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj = NULL;

	if (!*argument && !(obj = get_equip (ch, WEAR_PRIM))
		&& !(obj = get_equip (ch, WEAR_BOTH))
		&& !(obj = get_equip (ch, WEAR_SEC)))
	{
		send_to_char ("What item did you wish to shift your grip on?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf && !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		send_to_char ("You don't have that in your inventory.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) != ITEM_WEAPON && GET_ITEM_TYPE (obj) != ITEM_FIREARM)
	{
		send_to_char ("The grip command is only for use with weapons.\n", ch);
		return;
	}

	if (obj->location != WEAR_PRIM && obj->location != WEAR_BOTH && obj->location != WEAR_SEC)
	{
		send_to_char("You must first 'wield' a weapon before you can change your grip on it.\n", ch);
		return;
	}

	if ((obj->o.od.value[3] == SKILL_BRAWLING) || (GET_ITEM_TYPE (obj) == ITEM_WEAPON && obj->o.od.value[0] == 2))
	{
		send_to_char
			("The grip command cannot be used with this weapon type.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_WEAPON)
	{

		if (obj->o.od.value[0] != 1)
		{
			if (ch->str > 18 && (obj->o.od.value[0] == 3))
				;
			else
			{
				send_to_char ("You cannot shift your grip upon that weapon.\n", ch);
				return;
			}
		}

		argument = one_argument (argument, buf);

		if ((!*buf && obj->location == WEAR_PRIM) || !str_cmp (buf, "both"))
		{
			if (ch->right_hand && ch->left_hand)
			{
				send_to_char
					("You'll need to free your other hand to switch to a two-handed grip.\n",
					ch);
				return;
			}

			if (ch->right_hand && (get_soma_affect(ch, SOMA_BLUNT_L_SEVARM) || get_soma_affect(ch, SOMA_NO_LARM)))
			{
				send_to_char
					("Your left arm is too damaged to grip a weapon.\n", ch);
				return;
			}

			if (ch->left_hand && (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM) || get_soma_affect(ch, SOMA_NO_RARM)))
			{
				send_to_char
					("Your right arm is too damaged to grip a weapon.\n", ch);
				return;
			}

			if (obj->location == WEAR_BOTH)
			{
				send_to_char
					("You are already gripping your weapon in both hands.\n", ch);
				return;
			}
			sprintf (buf, "You shift to a two-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf, "$n shifts to a two-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj->location = WEAR_BOTH;
		}
		else if (!(*buf && obj->location == WEAR_BOTH) || !str_cmp (buf, "single"))
		{
			if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC)
			{
				send_to_char
					("You are already gripping your weapon in your primary hand.\n",
					ch);
				return;
			}
			sprintf (buf, "You shift to a single-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf, "$n shifts to a single-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj->location = WEAR_PRIM;
		}
	}
	else
	{
		argument = one_argument (argument, buf);

		if ((!*buf && obj->location == WEAR_PRIM) || !str_cmp (buf, "both"))
		{
			if (ch->right_hand && ch->left_hand)
			{
				send_to_char
					("You'll need to free your other hand to switch to a two-handed grip.\n",
					ch);
				return;
			}

			if (ch->right_hand && (get_soma_affect(ch, SOMA_BLUNT_L_SEVARM) || get_soma_affect(ch, SOMA_NO_LARM)))
			{
				send_to_char
					("Your left arm is too damaged to grip a weapon.\n", ch);
				return;
			}

			if (ch->left_hand && (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM) || get_soma_affect(ch, SOMA_NO_RARM)))
			{
				send_to_char
					("Your right arm is too damaged to grip a weapon.\n", ch);
				return;
			}

			if (obj->location == WEAR_BOTH)
			{
				send_to_char
					("You are already gripping your weapon in both hands.\n", ch);
				return;
			}
			sprintf (buf, "You shift to a two-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf, "$n shifts to a two-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj->location = WEAR_BOTH;
		}
		else if (!(*buf && obj->location == WEAR_BOTH) || !str_cmp (buf, "single"))
		{
			if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC)
			{
				send_to_char
					("You are already gripping your weapon in your primary hand.\n",
					ch);
				return;
			}
			sprintf (buf, "You shift to a single-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf, "$n shifts to a single-handed grip on #2%s#0.",
				obj->short_description);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj->location = WEAR_PRIM;
		}
	}
}


void
	do_switch_item (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *right, *left;

	right = ch->right_hand;
	left = ch->left_hand;

	if (!right && !left)
	{
		act ("You have nothing to switch!", false, ch, 0, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if ((right && right->location == WEAR_BOTH) ||
		(left && left->location == WEAR_BOTH))
	{
		act ("You must grip that in both hands!", false, ch, 0, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	ch->right_hand = NULL;
	ch->left_hand = NULL;

	if (right && right->location != WEAR_BOTH)
	{

		if (get_soma_affect(ch, SOMA_BLUNT_L_SEVARM) || get_soma_affect(ch, SOMA_NO_LARM))
		{
			send_to_char("Your left arm is too damaged to hold that.\n", ch);
			ch->right_hand = right;
			return;
		}

		act ("You shift $p to your left hand.", false, ch, right, 0,
			TO_CHAR | _ACT_FORMAT);
		ch->left_hand = right;
	}

	if (left && left->location != WEAR_BOTH)
	{


		if (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM) || get_soma_affect(ch, SOMA_NO_RARM))
		{
			send_to_char("Your left arm is too damaged to hold that.\n", ch);
			ch->left_hand = left;
			return;
		}

		act ("You shift $p to your right hand.", false, ch, left, 0,
			TO_CHAR | _ACT_FORMAT);
		ch->right_hand = left;
	}

}


void
	clear_omote (OBJ_DATA * obj)
{
	if (obj->omote_str)
	{
		mem_free (obj->omote_str);
		obj->omote_str = (char *) NULL;
	}

	remove_obj_mult_affect (obj, MAGIC_OMOTED);	/* Probably doesn't exist */
}

int
	can_obj_to_container (OBJ_DATA * obj, OBJ_DATA * container, char **msg,
	int count)
{
	OBJ_DATA *tobj;
	int i = 0;
	static char message[160];

	if (count > obj->count || count <= 0)
		count = obj->count;

	*msg = message;

	if (GET_ITEM_TYPE (container) == ITEM_SHEATH)
	{
		if (GET_ITEM_TYPE (obj) != ITEM_WEAPON)
		{
			sprintf (message, "Sheaths are only for storing weaponry.\n");
			return 0;
		}
		if (container->contains)
		{
			sprintf (message, "This sheath already contains a weapon.\n");
			return 0;
		}
		if (container->contained_wt + OBJ_MASS (obj) > container->o.od.value[0])
		{
			sprintf (message, "That weapon is too large for the sheath.\n");
			return 0;
		}
		return 1;
	}

	if (GET_ITEM_TYPE (container) == ITEM_HOLSTER)
	{
		if (GET_ITEM_TYPE (obj) != ITEM_FIREARM)
		{
			sprintf (message, "Holsters are only for storing firearms.\n");
			return 0;
		}
		if (container->contains)
		{
			sprintf (message, "This holster already contains a firearms.\n");
			return 0;
		}

		if (obj->obj_flags.weight > container->o.od.value[0])
		{
			sprintf (message, "That firearm is too large for the holster.\n");
			return 0;
		}
		return 1;
	}

	if (IS_ELECTRIC (container))
	{
		if (GET_ITEM_TYPE (obj) != ITEM_E_BATTERY)
		{
			sprintf (message, "You can only put batteries or a power pack in to this type of object.\n");
			return 0;
		}
		if (IS_SET(container->o.elecs.elec_bits, ELEC_PWR_INTERNAL))
		{
			sprintf (message, "This item is powered from its own internal source - it has no need for batteries or a power pack.\n");
			return 0;
		}
		if (container->contains)
		{
			sprintf (message, "This item already contains a battery or power pack.\n");
			return 0;
		}
		if (!IS_SET(container->o.elecs.elec_bits, (1 << obj->o.battery.type)))
		{
			sprintf (message, "This item is not compatible with that sort of battery.\n");
			return 0;
		}
		return 1;
	}

	if (GET_ITEM_TYPE (container) == ITEM_KEYRING)
	{
		if (GET_ITEM_TYPE (obj) != ITEM_KEY)
		{
			sprintf (message, "Keyrings are only able to hold keys!\n");
			return 0;
		}
		for (tobj = container->contains, i = 1; tobj; tobj = tobj->next_content)
			i++;
		if (i + 1 > container->o.od.value[0])
		{
			sprintf (message,
				"There are too many keys on this keyring to add another.\n");
			return 0;
		}
		return 1;
	}

	/*
	if (GET_ITEM_TYPE (container) == ITEM_WEAPON
	&& container->o.weapon.use_skill == SKILL_SLING)
	{
	if (GET_ITEM_TYPE (obj) != ITEM_BULLET)
	{
	sprintf (message,
	"You'll need to find proper ammunition use with the sling.\n");
	return 0;
	}
	if (container->contains)
	{
	sprintf (message, "The sling is already loaded!\n");
	return 0;
	}
	if (count > 1)
	{
	sprintf (message,
	"The sling is only capable of throwing one projectile at a time.\n");
	return 0;
	}
	return 1;
	}
	*/

	if (GET_ITEM_TYPE (container) == ITEM_QUIVER)
	{
		if (GET_ITEM_TYPE (obj) != ITEM_MISSILE)
		{
			sprintf (message,
				"Quivers are only for storing arrows and bolts.\n");
			return 0;
		}
		for (tobj = container->contains, i = 0; tobj; tobj = tobj->next_content)
			i += tobj->count;
		if (i + count > container->o.od.value[0])
		{
			sprintf (message, "The quiver can't hold that many missiles.\n");
			return 0;
		}
		return 1;
	}

	if (GET_ITEM_TYPE (container) == ITEM_AMMO_BELT)
	{
		if (GET_ITEM_TYPE(obj) != ITEM_CLIP)
		{
			sprintf (message,
				"Belts are only for storing magazines.\n");
			return 0;
		}

		for (tobj = container->contains, i = 0; tobj; tobj = tobj->next_content)
			i += tobj->count;
		if (i + count > container->o.od.value[0])
		{
			sprintf (message, "The belt can't hold that many magazine.\n");
			return 0;
		}
		return 1;
	}

	if (GET_ITEM_TYPE (container) == ITEM_BANDOLIER)
	{
		if (GET_ITEM_TYPE(obj) != ITEM_ROUND)
		{
			sprintf (message,
				"Bandoliers are only for storing rounds.\n");
			return 0;
		}

		for (tobj = container->contains, i = 0; tobj; tobj = tobj->next_content)
			i += tobj->count;
		if (i + count > container->o.od.value[0])
		{
			sprintf (message, "The bandolier can't hold that many rounds.\n");
			return 0;
		}
		return 1;
	}

	if (GET_ITEM_TYPE (container) != ITEM_CONTAINER && GET_ITEM_TYPE (container) != ITEM_COVER &&
		GET_ITEM_TYPE (container) != ITEM_CRATE && GET_ITEM_TYPE (container) != ITEM_WORN)
	{
		sprintf (message, "%s is not a container.\n",
			container->short_description);
		*message = toupper (*message);
		return 0;
	}

	if (container == obj)
	{
		sprintf (message, "You can't do that.\n");
		return 0;
	}

	if (IS_SET (container->o.od.value[1], CONT_CLOSED))
	{
		sprintf (message, "%s is closed.\n", container->short_description);
		*message = toupper (*message);
		return 0;
	}

	if (count > 1)
	{
		if (container->contained_wt + obj->obj_flags.weight * count >
			container->o.od.value[0])
		{
			sprintf (message, "That much won't fit.\n");
			return 0;
		}
	}
	else if (container->contained_wt + obj->obj_flags.weight >
		container->o.od.value[0])
	{

		sprintf (message, "%s won't fit.\n", obj->short_description);
		*message = toupper (*message);
		return 0;
	}

	return 1;
}

#define NO_TOO_MANY		1
#define NO_TOO_HEAVY	2
#define NO_CANT_TAKE	3
#define NO_CANT_SEE		4
#define NO_HANDS_FULL	5
#define NO_RESOURCE 	6
#define LEFT_BROKEN 	7
#define RIGHT_BROKEN	8

int
	can_obj_to_inv (OBJ_DATA * obj, CHAR_DATA * ch, int *error, int count)
{

	if (count > obj->count || count <= 0)
		count = obj->count;

	*error = 0;

	if (!obj->in_obj && !CAN_SEE_OBJ (ch, obj))
	{
		*error = NO_CANT_SEE;
		return 0;
	}

	if (!CAN_WEAR (obj, ITEM_TAKE)
		|| IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED))
	{
		*error = NO_CANT_TAKE;
		return 0;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE &&
		str_cmp("robot", lookup_race_variable(ch->race, RACE_NAME)))
	{
		*error = NO_RESOURCE;
		return 0;
	}

	if (ch->right_hand && (ch->right_hand->nVirtual == obj->nVirtual)
		&& (GET_ITEM_TYPE (ch->right_hand) == ITEM_MONEY))
		return 1;

	if (ch->left_hand && (ch->left_hand->nVirtual == obj->nVirtual)
		&& (GET_ITEM_TYPE (ch->left_hand) == ITEM_MONEY))
		return 1;

	if ((ch->right_hand && ch->left_hand) || get_equip (ch, WEAR_BOTH))
	{
		*error = NO_HANDS_FULL;
		return 0;
	}

	/* Check out the weight */

	if (!(obj->in_obj && obj->in_obj->carried_by == ch))
	{

		if ((IS_CARRYING_W (ch) + (count * obj->obj_flags.weight)) >
			CAN_CARRY_W (ch) && IS_MORTAL (ch))
		{
			*error = NO_TOO_HEAVY;
			return 0;
		}

	}


	if (ch->right_hand && ch->left_hand)
		;
	else if (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM) || get_soma_affect(ch, SOMA_NO_RARM))
	{
		* error = RIGHT_BROKEN;
		return 0;
	}


	if (ch->right_hand && (get_soma_affect(ch, SOMA_BLUNT_L_SEVARM) || get_soma_affect(ch, SOMA_NO_LARM)))
	{
		* error = LEFT_BROKEN;
		return 0;
	}

	if (ch->left_hand && (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM) || get_soma_affect(ch, SOMA_NO_RARM)))
	{
		* error = RIGHT_BROKEN;
		return 0;
	}

	return 1;
}

int
	obj_activate (CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (!obj->activation)
		return 0;

	magic_affect (ch, obj->activation);

	/* Deal with one time activation on object */

	if (!IS_SET (obj->obj_flags.extra_flags, ITEM_MULTI_AFFECT))
		obj->activation = 0;

	/* Oops, I guess that killed him. */

	if (GET_POS (ch) == POSITION_DEAD)
		return 1;

	return 0;
}

void
	get (CHAR_DATA * ch, OBJ_DATA * obj, int count)
{
	OBJ_DATA *container;
	CHAR_DATA *i;
	OBJ_DATA *trap_obj = NULL;
    
	if (IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER)
		&& obj->nVirtual != VNUM_CORPSE)
	{
		obj->obj_flags.extra_flags &= ~ITEM_TIMER;
		obj->obj_timer = 0;
	}

	if (!obj->in_obj)
	{

		if (IS_SET (obj->tmp_flags, SA_DROPPED))
		{
			send_to_char ("That can't be picked up at the moment.\n", ch);
			return;
		}

		// Mod for taking a corpse - Methuselah
		// If someone is skinning the corpse, don't allow it to be taken

		if (obj->nVirtual == VNUM_CORPSE)
		{
			for (i = ch->room->people; i; i = i->next_in_room)
			{
				if (i->delay_info1 == (long int) obj)
				{
					send_to_char ("You can't take that while it's being skinned.\n", ch);
					return;
				}
			}
		}
		// End mod for not taking an object when it's being skinned.  -Methuselah

		// TRAPS!
		// If you're going to pick something up, make check if any of the objects in the rooms
		// are traps, and set, and if so, inflict the trap upon you. If not, obj_from_room
		// calls a subfunction that clears any outstanding trap data.

		for (trap_obj = ch->room->contents; trap_obj; trap_obj = trap_obj->next_content)
		{
			if (trap_obj == obj)
				continue;

			if ((GET_ITEM_TYPE(trap_obj) == ITEM_TRAP) && IS_MORTAL(ch))
				if ((trap_obj->o.od.value[1] == obj->coldload_id) && ((trap_obj->o.od.value[0] >= 1) || (trap_obj->o.od.value[0] <= -1)))
					if (trap_enact(ch, trap_obj, 3, obj) != 2)
						return;
		}

		obj_from_room (&obj, count);

		clear_omote (obj);

		act ("You get $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n gets $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

		obj_to_char (obj, ch);

		if (obj->ocues)
		{
			char your_buf[MAX_STRING_LENGTH];
			typedef std::multimap<obj_cue,std::string>::const_iterator N;
			std::pair<N,N> range = obj->ocues->equal_range (ocue_on_produced);
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

		if (obj->activation &&
			IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
			obj_activate (ch, obj);

		return;
	}

	/* Don't activate object if it is in an object we're carrying */

	container = obj->in_obj;

	// TRAPS!
	// If you're going to pick something up, make check if any of the objects in the rooms
	// are traps, and set, and if so, inflict the trap upon you. If not, obj_from_room
	// calls a subfunction that clears any outstanding trap data.
	for (trap_obj = ch->room->contents; trap_obj; trap_obj = trap_obj->next_content)
	{
		if (trap_obj == container)
			continue;

		if ((GET_ITEM_TYPE(trap_obj) == ITEM_TRAP) && IS_MORTAL(ch))
			if ((trap_obj->o.od.value[1] == container->coldload_id) && ((trap_obj->o.od.value[0] >= 1) || (trap_obj->o.od.value[0] <= -1)))
				if (trap_enact(ch, trap_obj, 3, container) != 2)
					return;
	}

	obj_from_obj (&obj, count);

	act ("You get $p from $P.", false, ch, obj, container,
		TO_CHAR | _ACT_FORMAT);
	act ("$n gets $p from $P.", true, ch, obj, container,
		TO_ROOM | _ACT_FORMAT);

	// If it was an electronic thing, then we've just ripped the battery out.
	if (IS_ELECTRIC(container))
		container->o.elecs.status = 0;

	obj_to_char (obj, ch);

	if (obj->ocues)
	{
		char your_buf[MAX_STRING_LENGTH];
		typedef std::multimap<obj_cue,std::string>::const_iterator N;
		std::pair<N,N> range = obj->ocues->equal_range (ocue_on_grab);
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

	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate (ch, obj);
}

int
	just_a_number (char *buf)
{
	unsigned int i;

	if (!*buf)
		return 0;

	for (i = 0; i < strlen (buf); i++)
		if (!isdigit (buf[i]))
			return 0;

	return 1;
}

void
	do_junk (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What did you wish to junk?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		send_to_char ("You don't seem to be holding that.\n", ch);
		return;
	}

	obj_from_char (&obj, 0);
	if ( engine.in_play_mode ())
	{
		obj_to_room (obj, JUNKYARD);
		obj->obj_timer = 2;		// Junked items saved for 30 minutes - enough to grab it if you accidentally mess up.
		obj->obj_flags.extra_flags |= ITEM_TIMER;
	}
	else
	{
		extract_obj (obj);
	}

	sprintf (buf, "You junk #2%s#0.", obj_short_desc(obj));
	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
}

#define CONTAINER_LOC_NOT_FOUND	0
#define CONTAINER_LOC_ROOM		1
#define CONTAINER_LOC_INVENTORY	2
#define CONTAINER_LOC_WORN		3
#define CONTAINER_LOC_UNKNOWN	4

void
	do_get (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *hitch;
	ROOM_DATA *hitch_room;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *container = NULL;
	int container_loc = CONTAINER_LOC_UNKNOWN;
	int count = 0;
	int error;
	SECOND_AFFECT *sa;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char original_argument[MAX_STRING_LENGTH] = {'\0'};
	bool coldload_id = false;
	bool omoted = false;

	*arg1 = '\0';
	*arg2 = '\0';

	/*
	if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC)
	&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
	send_to_char ("This command has been disabled in OOC zones.\n", ch);
	return;
	}
	*/

	if (argument[strlen(argument) - 1] == '!')
	{
		omoted = true;
		for (size_t y = 0; y < strlen (argument) - 2; y++)
		{
			sprintf (original_argument + strlen (original_argument), "%c", argument[y]);
		}
		strcpy(argument, original_argument);
	}
	else
	{
		strcpy(original_argument, argument);
	}

	argument = one_argument (argument, arg1);

	if (just_a_number (arg1))
	{
		count = atoi (arg1);
		argument = one_argument (argument, arg1);
	}
	else if (!str_cmp (arg1, ".c"))
	{
		coldload_id = true;
		argument = one_argument (argument, arg1);
	}

	argument = one_argument (argument, arg2);

	if (!str_cmp (arg2, "from") || !str_cmp (arg2, "in"))
		argument = one_argument (argument, arg2);

	if (!str_cmp (arg2, "ground") || !str_cmp (arg2, "room"))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_ROOM;
	}

	else if (!str_cmp (arg2, "worn") || !str_cmp (arg2, "my"))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_WORN;
	}

	else if (!strn_cmp (arg2, "inventory", 3))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_INVENTORY;
	}

	if (*arg2 &&
		container_loc == CONTAINER_LOC_UNKNOWN &&
		(hitch = get_char_room_vis (ch, arg2)) &&
		hitch->mob &&
		hitch->mob->vehicle_type == VEHICLE_HITCH &&
		(hitch_room = vnum_to_room (hitch->mob->vnum)))
	{

		if (!(obj = get_obj_in_list_vis (ch, arg1, hitch_room->contents)))
		{
			act ("You don't see that in $N.", false, ch, 0, hitch,
				TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (!can_obj_to_inv (obj, ch, &error, count))
		{

			if (error == NO_CANT_TAKE)
				act ("You can't take $o.", true, ch, obj, 0, TO_CHAR);
			else if (error == NO_TOO_MANY)
				act ("You can't handle so much.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_TOO_HEAVY)
				act ("You can't carry so much weight.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_CANT_SEE)
				act ("You don't see it.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_HANDS_FULL)
				act ("Your hands are full!", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_RESOURCE)
				act ("You'll need a hauling robot to move $p.", true, ch, obj, 0, TO_CHAR);
			else if (error == LEFT_BROKEN)
				act ("Your left arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
			else if (error == RIGHT_BROKEN)
				act ("Your right arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
			return;
		}

		obj_from_room (&obj, count);

		act ("You take $p from $N.", false, ch, obj, hitch,
			TO_CHAR | _ACT_FORMAT);
		act ("$n takes $p from $N.", false, ch, obj, hitch,
			TO_NOTVICT | _ACT_FORMAT);

		char_from_room (ch);
		char_to_room (ch, hitch->mob->vnum);

		act ("$n reaches in and takes $p.", false, ch, obj, hitch,
			TO_NOTVICT | _ACT_FORMAT);

		char_from_room (ch);
		char_to_room (ch, hitch->in_room);

		obj_to_char (obj, ch);

		return;
	}

	else if (*arg2)
	{

		if (container_loc == CONTAINER_LOC_UNKNOWN &&
			!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->equip)) &&
			!(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if (container_loc == CONTAINER_LOC_ROOM &&
			!(container =
			get_obj_in_list_vis (ch, arg2, ch->room->contents)))
		{
			container_loc = CONTAINER_LOC_NOT_FOUND;
		}

		else if (container_loc == CONTAINER_LOC_INVENTORY &&
			!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->left_hand)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if (container_loc == CONTAINER_LOC_WORN &&
			!(container = get_obj_in_dark (ch, arg2, ch->equip)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		if (container_loc == CONTAINER_LOC_NOT_FOUND)
		{
			send_to_char ("You neither have nor see such a container.\n", ch);
			return;
		}

		if (GET_ITEM_TYPE (container) != ITEM_CONTAINER &&
			GET_ITEM_TYPE (container) != ITEM_QUIVER &&
			GET_ITEM_TYPE (container) != ITEM_CRATE &&
			GET_ITEM_TYPE (container) != ITEM_COVER &&
			GET_ITEM_TYPE (container) != ITEM_WORN &&
			//(GET_ITEM_TYPE (container) != ITEM_WEAPON &&
			// container->o.weapon.use_skill != SKILL_SLING) &&
			GET_ITEM_TYPE (container) != ITEM_SHEATH &&
			GET_ITEM_TYPE (container) != ITEM_HOLSTER &&
			GET_ITEM_TYPE (container) != ITEM_AMMO_BELT &&
			GET_ITEM_TYPE (container) != ITEM_BANDOLIER &&
			!IS_ELECTRIC (container) &&
			GET_ITEM_TYPE (container) != ITEM_KEYRING)
		{
			act ("$o isn't a container.", true, ch, container, 0, TO_CHAR);
			return;
		}
		/*
		if ( GET_ITEM_TYPE (container) != ITEM_QUIVER
		&& ( bow = get_equip (ch, WEAR_PRIM) )
		&& ( bow->o.weapon.use_skill == SKILL_SHORTBOW
		|| bow->o.weapon.use_skill == SKILL_LONGBOW) ) {
		send_to_char ("You'll have to stop using the bow first.\n", ch);
		return;
		}
		*/
		if (IS_SET (container->o.container.flags, CONT_CLOSED))
		{
			send_to_char ("That's closed!\n", ch);
			return;
		}

		if (GET_ITEM_TYPE(container) == ITEM_TRAP)
		{
			send_to_char ("You'll need to deconstruct the trap to retrieve its components.\n", ch);
			return;
		}

		if (container_loc == CONTAINER_LOC_UNKNOWN)
		{
			if (container->carried_by)
				container_loc = CONTAINER_LOC_INVENTORY;
			else if (container->equiped_by)
				container_loc = CONTAINER_LOC_WORN;
			else
				container_loc = CONTAINER_LOC_ROOM;
		}
	}

	if (!*arg1)
	{
		send_to_char ("Get what?\n", ch);
		return;
	}

	if (!str_cmp (arg1, "all"))
	{

		for (obj = (container ? container->contains : ch->room->contents);
			obj && !can_obj_to_inv (obj, ch, &error, 1);
			obj = obj->next_content)
			;

		if (!obj)
		{
			send_to_char ("There is nothing left you can take.\n", ch);
			return;
		}

		ch->delay_type = DEL_GET_ALL;
		ch->delay_obj = container;
		ch->delay_info1 = container_loc;
		ch->delay = 4;
	}

	else if (*arg2 && !str_cmp (arg2, "all"))
	{
		send_to_char ("You'll have to get things one at a time.\n", ch);
		return;
	}

	if (!container)
	{

		if (!obj && isdigit (*arg1) && coldload_id)
		{
			if (!(obj = get_obj_in_list_id (atoi (arg1), ch->room->contents))
				|| obj->in_room != ch->in_room)
			{
				send_to_char ("You don't see that here.\n", ch);
				return;
			}
		}

		if (!obj && !(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
		{
			send_to_char ("You don't see that here.\n", ch);
			return;
		}

		if (!can_obj_to_inv (obj, ch, &error, count))
		{

			if (error == NO_CANT_TAKE)
				act ("You can't take $o.", true, ch, obj, 0, TO_CHAR);
			else if (error == NO_TOO_MANY)
				act ("You can't handle so much.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_TOO_HEAVY)
				act ("You can't carry so much weight.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_CANT_SEE)
				act ("You don't see it.", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_HANDS_FULL)
				act ("Your hands are full!", true, ch, 0, 0, TO_CHAR);
			else if (error == NO_RESOURCE)
				act ("You'll need a hauling robot to move $p.", true, ch, obj, 0, TO_CHAR);
			else if (error == LEFT_BROKEN)
				act ("Your left arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
			else if (error == RIGHT_BROKEN)
				act ("Your right arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
			return;
		}

		if ((sa = get_second_affect (ch, SA_GET_OBJ, obj)))
			return;

		if (get_obj_affect(obj, MAGIC_OMOTED) && (!omoted))
		{
			sprintf(arg1, "That item has been omote-locked. To pick it up, you fill need to add a !\nto the end of the command, i.e. '#6get %s !#0'\n", original_argument);
			send_to_char(arg1, ch);
			return;
		}

		get (ch, obj, count);
		return;
	}

	/* get obj from container */

	if (!obj && !(obj = get_obj_in_dark (ch, arg1, container->contains)))
	{
		act ("You don't see that in $o.", true, ch, container, 0, TO_CHAR);
		return;
	}

	if (!can_obj_to_inv (obj, ch, &error, count))
	{

		if (error == NO_CANT_TAKE)
			act ("You cannot take $o.", true, ch, obj, 0, TO_CHAR);
		else if (error == NO_TOO_HEAVY)
			send_to_char ("You can't carry so much weight.\n", ch);
		else if (error == NO_TOO_MANY)
			send_to_char ("Your can't handle so much.\n", ch);
		else if (error == NO_HANDS_FULL)
			act ("Your hands are full!", true, ch, 0, 0, TO_CHAR);
		else if (error == NO_RESOURCE)
			act ("You'll need a hauling robot to move $p.", true, ch, obj, 0, TO_CHAR);
		else if (error == LEFT_BROKEN)
			act ("Your left arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
		else if (error == RIGHT_BROKEN)
			act ("Your right arm is too damaged to hold $p.",  true, ch, obj, 0, TO_CHAR);
		return;
	}

	if (container && container != obj)
		obj->in_obj = container;
	if (!container->contains)
		container->contains = obj;

	if (get_obj_affect(obj, MAGIC_OMOTED) && (!omoted))
	{
		sprintf(arg1, "That item has been omote-locked. To pick it up, you fill need to add a !\nto the end of the command, i.e. '#6get %s !#0'\n", original_argument);
		send_to_char(arg1, ch);
		return;
	}

	get (ch, obj, count);
}

void
	do_take (CHAR_DATA * ch, char *argument, int cmd)
{
	int worn_object = 0;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char obj_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch)
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	argument = one_argument (argument, obj_name);

	if (!*obj_name)
	{
		send_to_char ("Take what?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "from"))
		argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Take from whom?\n", ch);
		return;
	}

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see them.\n", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("Why take from yourself?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, obj_name, victim->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, obj_name, victim->left_hand)))
	{

		if (!(obj = get_obj_in_list_vis (ch, obj_name, victim->equip)))
		{
			act ("You don't see that on $N.", true, ch, 0, victim, TO_CHAR);
			return;
		}

		if (GET_TRUST (ch))
		{
			unequip_char (victim, obj->location);
			obj_to_char (obj, victim);
		}
		else
			worn_object = 1;
	}

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_NOREMOVE) && !GET_TRUST(ch))
	{
		act ("You are unable to remove $p from $N.", false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		act ("$n tries unsuccessfully to remove $p from you.", false, ch, obj, victim, TO_VICT | _ACT_FORMAT);
		return;
	}

	if (GET_POS (victim) == SLEEP && !GET_TRUST (ch))
	{

		wakeup (victim);

		if (GET_POS (victim) != SLEEP)
		{

			act ("$N awakens as you touch $M.", true, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e touches $M.",
				false, ch, 0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(GET_TRUST (ch) ||
		GET_POS (victim) <= SLEEP ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		IS_SUBDUEE (victim) || IS_MOUNT (victim)))
	{
		act ("$N prevents you from taking $p.", true, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
			true, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
			false, victim, obj, ch, TO_NOTVICT);
		return;
	}

	if (worn_object)
	{

		strcpy (buf2, locations[obj->location]);
		*buf2 = tolower (*buf2);

		sprintf (buf, "You begin to remove $p from $N's %s.", buf2);
		act (buf, true, ch, obj, victim, TO_CHAR | _ACT_FORMAT);

		sprintf (buf, "$n begins removing $p from $N's %s.", buf2);
		act (buf, false, ch, obj, victim, TO_NOTVICT | _ACT_FORMAT);

		sprintf (buf, "$N begins removing $p from your %s.", buf2);
		act (buf, true, victim, obj, ch, TO_CHAR | _ACT_FORMAT);

		ch->delay_info1 = (long int) obj;
		ch->delay_info2 = obj->location;
		ch->delay_ch = victim;
		ch->delay_type = DEL_TAKE;
		ch->delay = 15;

		return;
	}

	obj_from_char (&obj, 0);
	obj_to_char (obj, ch);

	if (!GET_TRUST (ch))
	{
		act ("$n takes $p from you.", true, victim, obj, ch,
			TO_CHAR | _ACT_FORMAT);
		act ("$n takes $p from $N.", false, ch, obj, victim,
			TO_NOTVICT | _ACT_FORMAT);
	}

	clear_omote (obj);

	act ("You take $p from $N.", true, ch, obj, victim, TO_CHAR);

	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate (ch, obj);
}

void
	delayed_take (CHAR_DATA * ch)
{
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	ch->delay = 0;
	victim = ch->delay_ch;
	obj = (OBJ_DATA *) ch->delay_info1;

	if (!is_he_here (ch, victim, true))
	{
		send_to_char ("Your victim left before you could finish taking the "
			"object.\n", ch);
		return;
	}

	if (get_equip (victim, ch->delay_info2) != obj)
	{
		send_to_char ("The thing you were after is gone now.", ch);
		return;
	}

	if (GET_POS (victim) == SLEEP && !GET_TRUST (ch))
	{

		wakeup (victim);

		if (GET_POS (victim) != SLEEP)
		{

			act ("$N awakens as struggle with $M.",
				true, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e struggles with $M.",
				false, ch, 0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(GET_TRUST (ch) ||
		GET_POS (victim) <= SLEEP ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		IS_SUBDUEE (victim) || IS_MOUNT (victim)))
	{
		act ("$N prevents you from taking $p.", true, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
			true, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
			false, victim, obj, ch, TO_NOTVICT);
		return;
	}

	sprintf (buf, "$n removes and takes $p from $N's %s.",
		locations[obj->location]);
	act (buf, false, ch, obj, victim, TO_NOTVICT | _ACT_FORMAT);

	sprintf (buf, "$N removes and takes $p from your %s.",
		locations[obj->location]);
	act (buf, true, victim, obj, ch, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "You remove and take $p from $N's %s.",
		locations[obj->location]);
	act (buf, true, ch, obj, victim, TO_CHAR | _ACT_FORMAT);

	unequip_char (victim, obj->location);

	obj_to_char (obj, ch);

	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate (ch, obj);
}

void
	get_break_delay (CHAR_DATA * ch)
{
	if (ch->delay_info1 == CONTAINER_LOC_ROOM)
		send_to_char ("You stop picking things up.\n", ch);
	else
		send_to_char ("You stop removing things.\n", ch);

	ch->delay = 0;
}

void
	delayed_get (CHAR_DATA * ch)
{
	OBJ_DATA *container = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *first_obj = NULL;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	int item_num = 0;
	int container_num = 0;
	int error = 0;
	char *locs[] = { "", "room", "inventory", "worn", "" };

	if (ch->delay_obj)
	{
		/* Makes sure that this container is in the room */

		container = ch->delay_obj;

		if (ch->delay_info1 == CONTAINER_LOC_ROOM)
			obj = ch->room->contents;
		else if (ch->delay_info1 == CONTAINER_LOC_INVENTORY)
			obj = ch->right_hand;
		else if (ch->delay_info1 == CONTAINER_LOC_WORN)
			obj = ch->equip;
		else
			obj = NULL;

		for (; obj; obj = obj->next_content)
		{
			container_num++;

			if (obj == container)
				break;
		}

		if (!obj)
		{
			send_to_char ("You can't get anything else.\n", ch);
			return;
		}

		first_obj = container->contains;
	}
	else
		first_obj = ch->room->contents;

	for (obj = first_obj; obj; obj = obj->next_content)
	{

		if (!IS_OBJ_VIS (ch, obj))
			continue;

		item_num++;

		if (can_obj_to_inv (obj, ch, &error, 0))
		{
			if (container_num)
				sprintf (buf, "#%d %s #%d",
				item_num, locs[ch->delay_info1], container_num);
			else
				sprintf (buf, "#%d", item_num);

			do_get (ch, buf, 0);

			if (obj->carried_by != ch)
				printf ("Oh boy...couldn't pick up %d\n", obj->nVirtual);
			else
				ch->delay = 4;
			return;
		}
	}

	send_to_char ("...and that's about all you can get.\n", ch);

	ch->delay = 0;
}

void
	delayed_remove (CHAR_DATA * ch)
{
	OBJ_DATA *obj, *eq;

	obj = (OBJ_DATA *) ch->delay_who;

	if (!obj)
	{
		ch->delay_type = 0;
		ch->delay_who = 0;
		ch->delay = 0;
		return;
	}

	if (obj->location == WEAR_WAIST)
	{
		if ((eq = get_equip (ch, WEAR_BELT_1)))
		{
			act ("$p falls free.", true, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", true, ch, eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_1), ch->in_room);
		}

		if ((eq = get_equip (ch, WEAR_BELT_2)))
		{
			act ("$p falls free.", true, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", true, ch, eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_2), ch->in_room);
		}
	}

	if (obj->attached /*&& obj->o.weapon.use_skill != SKILL_CROSSBOW*/)
		do_unload (ch, "", 0);

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
		do_hood (ch, "", 0);

	if (obj->location == WEAR_LIGHT && GET_ITEM_TYPE (obj) == ITEM_LIGHT)
		light (ch, obj, false, true);

	if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH || obj->location == WEAR_SHIELD)
		unequip_char (ch, obj->location);
	else
		obj_to_char (unequip_char (ch, obj->location), ch);

	act ("You stop using $p.", false, ch, obj, 0, TO_CHAR);
	act ("$n stops using $p.", true, ch, obj, 0, TO_ROOM);

	ch->delay = 0;
	ch->delay_type = 0;
	ch->delay_who = 0;
}

void
	drop_all (CHAR_DATA * ch, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	OBJ_DATA *tobj, *obj;

	if (!cmd && !spare_capacity(ch, obj, true))
	{
		send_to_char("This area is too small and already too full to contain the items you are holding.\n", ch);
		return;
	}

	if (ch->right_hand && GET_POS (ch) != POSITION_UNCONSCIOUS)
	{
		act ("You drop:", false, ch, 0, 0, TO_CHAR);
		act ("$n drops:", false, ch, 0, 0, TO_ROOM);
	}

	while (ch->right_hand || ch->left_hand)
	{

		if (ch->right_hand)
			obj = ch->right_hand;
		else
			obj = ch->left_hand;

		if (GET_POS (ch) != POSITION_UNCONSCIOUS)
		{
			act ("   $p", false, ch, obj, 0, TO_CHAR);
			act ("   $p", false, ch, obj, 0, TO_ROOM);
		}

		obj_from_char (&obj, 0);
		obj_to_room (obj, ch->in_room);

		if (obj->ocues)
		{
			char your_buf[MAX_STRING_LENGTH];
			typedef std::multimap<obj_cue,std::string>::const_iterator N;
			std::pair<N,N> range = obj->ocues->equal_range (ocue_on_drop);
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

		/*
		if (ch->fighting && GET_ITEM_TYPE (obj) == ITEM_WEAPON && (obj->o.od.value[3] == SKILL_AIM) && obj->o.od.value[5] == 1)
		{
		if (number(0,1))
		{
		send_to_char("The bowstring curls and snaps as the melee rages on!\n", ch);
		obj->o.od.value[5] = 2;
		}
		}
		*/

		if (obj->attached)
		{
			sprintf (buffer, "%s#0 clatters to the ground!",
				obj_short_desc (obj->attached));
			*buffer = toupper (*buffer);
			sprintf (buf, "#2%s", buffer);
			act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			tobj = load_object (obj->attached->nVirtual);
			extract_obj (obj->attached);
			obj->attached = NULL;
			obj_to_room (tobj, ch->in_room);
			obj->attached = NULL;
		}

		//if (GET_ITEM_TYPE (obj) == ITEM_WEAPON
		//	   && obj->o.weapon.use_skill == SKILL_SLING && ch->whirling)
		// 	 {
		//	  send_to_char ("You cease whirling your sling.\n", ch);
		//	  ch->whirling = 0;
		//	}
	}
}

void
	do_drop (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = "";
	char buffer[MAX_STRING_LENGTH] = "";
	OBJ_DATA *obj = NULL, *tobj = NULL;
	ROOM_DATA *room = NULL;
	int count = 0, old_count = 1;
	std::string first_person, third_person;

	argument = one_argument (argument, buf);

	if (!ch->room && vnum_to_room(ch->in_room))
	{
		ch->room = vnum_to_room(ch->in_room);
		send_to_gods("Do_drop: no room found");
	}

	/*
	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch)
	&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
	send_to_char ("This command has been disabled in OOC zones.\n", ch);
	return;
	}
	*/

	if (just_a_number (buf))
	{
		count = atoi (buf);
		argument = one_argument (argument, buf);
	}

	if (!*buf)
	{
		send_to_char ("Drop what?\n", ch);
		return;
	}

	if (!str_cmp (buf, "all"))
	{

		argument = one_argument (argument, buf);

		if (*buf)
		{
			send_to_char ("You can only 'drop all'.\n", ch);
			return;
		}

		drop_all (ch, cmd);

		return;
	}

	if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		send_to_char ("You do not have that item.\n", ch);
		return;
	}

    if ( !cmd && !spare_capacity( ch, obj, false ) )
	{
		send_to_char("This area is too small and already too full to contain that item.\n", ch);
		return;
	}

	if (count > obj->count)
		count = obj->count;

	sprintf (buffer, "%s", char_short(ch));
	*buffer = toupper(*buffer);
	old_count = obj->count;
	if (count)
		obj->count = count;

	first_person.assign("You drop #2");
	first_person.append(obj_short_desc(obj));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buffer);
	third_person.append("#0 drops #2");
	third_person.append(obj_short_desc(obj));
	third_person.append("#0");

	obj->count = old_count;

	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		obj_from_char (&obj, count);
		obj_to_room (obj, ch->in_room);

		if (obj->ocues)
		{
			char your_buf[MAX_STRING_LENGTH];
			typedef std::multimap<obj_cue,std::string>::const_iterator N;
			std::pair<N,N> range = obj->ocues->equal_range (ocue_on_drop);
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

		if (obj->activation &&
			IS_SET (obj->obj_flags.extra_flags, ITEM_DROP_AFFECT))
			obj_activate (ch, obj);

		/*
		if (ch->fighting && GET_ITEM_TYPE (obj) == ITEM_WEAPON && (obj->o.od.value[3] == SKILL_ARCHERY) && obj->o.od.value[5] == 1)
		{
		if (number(0,1))
		{
		send_to_char("The bowstring curls and snaps as the melee rages on!\n", ch);
		obj->o.od.value[5] = 2;
		}
		}
		*/

		if (obj->attached/*  && obj->o.weapon.use_skill != SKILL_CROSSBOW*/)
		{
			sprintf (buffer, "%s#0 clatters to the ground!",
				obj_short_desc (obj->attached));
			*buffer = toupper (*buffer);
			sprintf (buf, "#2%s", buffer);
			act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			tobj = load_object (obj->attached->nVirtual);
			extract_obj (obj->attached);
			obj->attached = NULL;
			obj_to_room (tobj, ch->in_room);
			obj->attached = NULL;
		}

		/*
		if (GET_ITEM_TYPE (obj) == ITEM_WEAPON
		&& obj->o.weapon.use_skill == SKILL_SLING && ch->whirling)
		{
		send_to_char ("You cease whirling your sling.\n", ch);
		ch->whirling = 0;
		}*/

		if ( !(first_person.empty()) )
		{
			sprintf (buffer, "%s %s", buf, first_person.c_str());
			do_omote (ch, buffer, 0);
		}
	}
}

void
	put_on_char (CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj,
	int count, char *argument)
{
	int location;
	ROOM_DATA *hitch_room;

	if (victim->mob && victim->mob->vehicle_type == VEHICLE_HITCH)
	{

		if ((hitch_room = vnum_to_room (victim->mob->vnum)))
		{

			obj_from_char (&obj, count);

			act ("You put $p in $N.", false, ch, obj, victim,
				TO_CHAR | _ACT_FORMAT);
			act ("$n puts $p in $N.", false, ch, obj, victim,
				TO_NOTVICT | _ACT_FORMAT);

			char_from_room (ch);
			char_to_room (ch, victim->mob->vnum);

			act ("$n reaches in and drops $p.",
				false, ch, obj, victim, TO_NOTVICT | _ACT_FORMAT);

			char_from_room (ch);
			char_to_room (ch, victim->in_room);

			obj_to_room (obj, victim->mob->vnum);

			return;
		}

		act ("You can't put $p in $N.", false, ch, obj, victim,
			TO_CHAR | _ACT_FORMAT);

		return;
	}

	if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_NECK))
		location = WEAR_NECK_1;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BODY))
		location = WEAR_BODY;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
		location = WEAR_HEAD;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BACK))
		location = WEAR_BACK;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_LEGS))
		location = WEAR_LEGS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FEET))
		location = WEAR_FEET;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HANDS))
		location = WEAR_HANDS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMS))
		location = WEAR_ARMS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT))
		location = WEAR_ABOUT;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WAIST))
		location = WEAR_WAIST;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST))
	{
		if (get_equip (victim, WEAR_WRIST_L))
			location = WEAR_WRIST_R;
		else
			location = WEAR_WRIST_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HAIR))
		location = WEAR_HAIR;

	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_OVER))
		location = WEAR_OVER;

	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_EYES))
		location = WEAR_EYES;

	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FACE))
		location = WEAR_FACE;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ANKLE))
	{
		if (get_equip (victim, WEAR_ANKLE_L))
			location = WEAR_ANKLE_R;
		else
			location = WEAR_ANKLE_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHOULDER))
	{
		if (get_equip (victim, WEAR_SHOULDER_L))
			location = WEAR_SHOULDER_R;
		else
			location = WEAR_SHOULDER_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMBAND))
	{
		if (get_equip (victim, WEAR_ARMBAND_R))
			location = WEAR_ARMBAND_L;
		else
			location = WEAR_ARMBAND_R;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD))
		location = WEAR_BLINDFOLD;
	else
	{
		act ("You can't put $p on $M.", true, ch, obj, victim, TO_CHAR);
		return;
	}

	if (IS_MOUNT (victim) && !IS_SET (obj->obj_flags.extra_flags, ITEM_MOUNT) && GET_ITEM_TYPE(obj) != ITEM_RESOURCE)
	{
		act ("$N is a mount.  You can't put $p on it.",
			true, ch, obj, victim, TO_CHAR);
		return;
	}

	if (get_equip (victim, location))
	{
		act ("$N is already wearing $p.",
			false, ch, get_equip (victim, location), victim, TO_CHAR);
		return;
	}

	if (GET_POS (victim) == SLEEP && !GET_TRUST (ch))
	{

		wakeup (victim);

		if (GET_POS (victim) != SLEEP)
		{

			act ("$N awakens as you touch $M.", true, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e touches $M.",
				false, ch, 0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(GET_TRUST (ch) ||
		GET_POS (victim) <= SLEEP ||
		!str_cmp("robot", lookup_race_variable(victim->race, RACE_NAME)) ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) || IS_MOUNT (victim)))
	{
		act ("$N stops you from putting $p on $M.",
			true, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to put $p from you.",
			true, victim, obj, ch, TO_CHAR);
		act ("$n stops $N from putting $p on $M.",
			false, victim, obj, ch, TO_NOTVICT);
		return;
	}

	ch->delay_type = DEL_PUTCHAR;
	ch->delay = 7;
	ch->delay_ch = victim;
	ch->delay_info1 = (long int) obj;
	ch->delay_info2 = location;

	act ("$n begins putting $p on $N.", false, ch, obj, victim,
		TO_NOTVICT | _ACT_FORMAT);
	act ("$n begins putting $p on you.", true, ch, obj, victim,
		TO_VICT | _ACT_FORMAT);
	act ("You begin putting $p on $N.", true, ch, obj, victim,
		TO_CHAR | _ACT_FORMAT);
}

void
	delayed_putchar (CHAR_DATA * ch)
{
	int location;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	ch->delay = 0;
	victim = ch->delay_ch;
	obj = (OBJ_DATA *) ch->delay_info1;
	location = ch->delay_info2;

	if (!is_he_here (ch, victim, true))
	{
		send_to_char ("Your victim left before you could finish dressing "
			"them.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE)
	{
		if (!is_obj_in_list (obj, ch->room->contents))
		{
			send_to_char ("You don't see that here.\n", ch);
			return;
		}
	}
	else
	{
		if (!is_obj_in_list (obj, ch->right_hand) &&
			!is_obj_in_list (obj, ch->left_hand))
		{
			act ("You no longer have the thing you were putting on $N.",
				false, ch, 0, victim, TO_CHAR);
			act ("$n stops putting something on $N.", true, ch, 0, victim, TO_ROOM);
			act ("$n stops putting something on you.",
				true, ch, 0, victim, TO_VICT);
			return;
		}
	}


	tobj = get_equip (victim, ch->delay_info2);

	if (tobj && tobj != obj)
	{
		act ("You discover that $N is already wearing $p.",
			true, ch, tobj, victim, TO_CHAR);
		act ("$n stops dressing $N.", false, ch, 0, victim, TO_NOTVICT);
		return;
	}

	if (GET_POS (victim) == SLEEP && !GET_TRUST (ch))
	{

		wakeup (victim);

		if (GET_POS (victim) != SLEEP)
		{

			act ("$N awakens as struggle with $M.",
				true, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e struggles with $M.",
				false, ch, 0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(GET_TRUST (ch) ||
		GET_POS (victim) <= SLEEP ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) || IS_MOUNT (victim)))
	{
		act ("$N prevents you from taking $p.", true, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
			true, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
			false, victim, obj, ch, TO_NOTVICT);
		return;
	}

	obj_from_char (&obj, 0);
	equip_char (victim, obj, location);

	strcpy (buf2, locations[location]);
	*buf2 = tolower (*buf2);

	sprintf (buf, "$n puts $p on $N's %s.", buf2);
	act (buf, false, ch, obj, victim, TO_NOTVICT | _ACT_FORMAT);

	sprintf (buf, "$N puts $p on your %s.", buf2);
	act (buf, true, victim, obj, ch, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "You put $p on $N's %s.", buf2);
	act (buf, true, ch, obj, victim, TO_CHAR | _ACT_FORMAT);

	if (obj->activation &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_WEAR_AFFECT))
		obj_activate (ch, obj);
}

void do_put (CHAR_DATA * ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH] = "";
	char arg[MAX_STRING_LENGTH] = "";
	char *error;
	OBJ_DATA *obj;
	OBJ_DATA *tar;
	OBJ_DATA *trap_obj;
	int count = 0, old_count = 1, put_light_on_table = 0;
	CHAR_DATA *victim;
	std::string first_person, third_person;

	argument = one_argument (argument, arg);

	if (just_a_number (arg))
	{
		count = atoi (arg);
		argument = one_argument (argument, arg);
	}

	if (!*arg)
	{
		send_to_char ("Put what?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->left_hand)))
	{
		sprintf (buffer, "You don't have a %s.\n", arg);
		send_to_char (buffer, ch);
		return;
	}

	argument = one_argument (argument, arg);

	if (!str_cmp (arg, "in") || !str_cmp (arg, "into"))
		argument = one_argument (argument, arg);

	else if (!str_cmp (arg, "on"))
	{

		argument = one_argument (argument, arg);

		if (!(victim = get_char_room_vis (ch, arg)))
		{
			act ("Put $p on whom?", true, ch, obj, 0, TO_CHAR);
			return;
		}

		put_on_char (ch, victim, obj, count, argument);

		return;
	}

	if (!*arg)
	{
		act ("Put $o into what?", false, ch, obj, 0, TO_CHAR);
		return;
	}

	if (!(tar = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(tar = get_obj_in_dark (ch, arg, ch->left_hand)) &&
		!(tar = get_obj_in_dark (ch, arg, ch->equip)) &&
		!(tar = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{

		if ((victim = get_char_room_vis (ch, arg)))
		{
			put_on_char (ch, victim, obj, count, argument);
			return;
		}

		sprintf (buffer, "You don't see a %s.\n", arg);
		send_to_char (buffer, ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) == ITEM_SLING && obj->attached)
	{
		send_to_char ("You'll need to unload that, first.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_GRENADE && obj->o.grenade.status == 1)
	{
		send_to_char("You can't put a live grenade in to there!\n", ch);
		return;
	}

	// mod by Methuselah for table_lamp

	if ((GET_ITEM_TYPE (obj) == ITEM_LIGHT && obj->o.light.on == true && !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC)) ||
		(GET_ITEM_TYPE (obj) == ITEM_E_LIGHT && obj->o.elecs.status)	)
	{
		put_light_on_table++;
	}

	if (!can_obj_to_container (obj, tar, &error, count))
	{
		send_to_char (error, ch);
		return;
	}

	sprintf (buffer, "%s", char_short(ch));
	*buffer = toupper(*buffer);
	old_count = obj->count;
	if (count)
		obj->count = count;

	first_person.assign("You put #2");
	first_person.append(obj_short_desc(obj));
	third_person.assign("#5");
	third_person.append(buffer);
	third_person.append("#0 puts #2");
	third_person.append(obj_short_desc(obj));

	if (IS_TABLE(tar))
	{
		first_person.append("#0 on #2");
		third_person.append("#0 on #2");
	}
	else
	{
		first_person.append("#0 into #2");
		third_person.append("#0 into #2");
	}

	first_person.append(obj_short_desc(tar));
	first_person.append("#0");
	third_person.append(obj_short_desc(tar));
	third_person.append("#0");

	obj->count = old_count;

	// TRAPS!
	// If you're going to pick something up, make check if any of the objects in the rooms
	// are traps, and set, and if so, inflict the trap upon you. If not, obj_from_room
	// calls a subfunction that clears any outstanding trap data.
	for (trap_obj = ch->room->contents; trap_obj; trap_obj = trap_obj->next_content)
	{
		if (trap_obj == tar)
			continue;

		if ((GET_ITEM_TYPE(trap_obj) == ITEM_TRAP) && IS_MORTAL(ch))
			if ((trap_obj->o.od.value[1] == tar->coldload_id) && ((trap_obj->o.od.value[0] >= 1) || (trap_obj->o.od.value[0] <= -1)))
				if (trap_enact(ch, trap_obj, 3, tar) != 2)
					return;
	}

	if (obj->in_room != -1 && !spare_capacity(ch, obj, false))
	{
		send_to_char("This area is too small and already too full to contain that item.\n", ch);
		return;
	}


	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		obj_from_char (&obj, count);
		obj_to_obj (obj, tar);
	}
	if (put_light_on_table)
		room_light(ch->room);

	return;
}


void
	do_give (CHAR_DATA * ch, char *argument, int cmd)
{
	char obj_name[MAX_INPUT_LENGTH];
	char vict_name[MAX_INPUT_LENGTH];
	CHAR_DATA *vict;
	OBJ_DATA *obj;
	int count = 0, error;

	argument = one_argument (argument, obj_name);

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch)
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	if (just_a_number (obj_name))
	{
		count = atoi (obj_name);
		argument = one_argument (argument, obj_name);
	}

	if (IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("Mounts can't use this command.n", ch);
		return;
	}

	if (!*obj_name)
	{
		send_to_char ("Give what?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_dark (ch, obj_name, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, obj_name, ch->left_hand)))
	{
		send_to_char ("You do not seem to have anything like that.\n", ch);
		return;
	}

	argument = one_argument (argument, vict_name);

	if (!str_cmp (vict_name, "to"))
		argument = one_argument (argument, vict_name);

	if (!*vict_name)
	{

		if (obj->obj_flags.type_flag == ITEM_TICKET)
		{
			unstable (ch, obj, NULL);
			return;
		}

		send_to_char ("Give to whom?\n", ch);
		return;
	}

	if (!(vict = get_char_room_vis (ch, vict_name)))
	{
		send_to_char ("No one by that name around here.\n", ch);
		return;
	}

	if (vict == ch)
	{
		send_to_char ("Give it to yourself? How generous...\n", ch);
		return;
	}


	if (GET_ITEM_TYPE(obj) == ITEM_GRENADE && obj->o.grenade.status == 1)
	{
		send_to_char("No one wants your present of a live grenade!\n", ch);
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_TICKET)
	{
		unstable (ch, obj, vict);
		return;
	}

	if (IS_NPC (vict) && IS_SET (vict->flags, FLAG_KEEPER) &&
		obj->obj_flags.type_flag == ITEM_MERCH_TICKET)
	{
		redeem_order (ch, obj, vict);
		return;
	}

	if (!can_obj_to_inv (obj, vict, &error, count))
	{
		if (error == NO_HANDS_FULL)
		{
			act ("$N's hands are currently occupied, I'm afraid.", true, ch,
				obj, vict, TO_CHAR | _ACT_FORMAT);
			act ("$n just tried to give you $o, but your hands are full.", true,
				ch, obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		if (error == LEFT_BROKEN)
		{
			act ("$N's left arm is too damaged to hold $o.", true, ch,
				obj, vict, TO_CHAR | _ACT_FORMAT);
			act ("$n just tried to give you $o, but your left arm is too damaged to hold it.", true,
				ch, obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		if (error == RIGHT_BROKEN)
		{
			act ("$N's right arm is too damaged to hold $o.", true, ch,
				obj, vict, TO_CHAR | _ACT_FORMAT);
			act ("$n just tried to give you $o, but your right arm is too damaged to hold it.", true,
				ch, obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		else if (error == NO_TOO_HEAVY || error == NO_RESOURCE)
		{
			act
				("$N struggles beneath the weight of the object, and so you take it back.",
				true, ch, obj, vict, TO_CHAR | _ACT_FORMAT);
			act
				("$n just tried to give you $o, but it is too heavy for you to carry.",
				true, ch, obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		else if (error == NO_CANT_TAKE)
		{
			act ("This item cannot be given.", false, ch, 0, 0, TO_CHAR);
			return;
		}
	}

	if (IS_CARRYING_N (vict) + 1 > CAN_CARRY_N (vict))
	{
		if (CAN_CARRY_N (vict) == 0)
			act ("$N isn't capable of carrying anyting.",
			false, ch, 0, vict, TO_CHAR);
		else
			act ("$N hands are full.", 0, ch, 0, vict, TO_CHAR);
		return;
	}

	if (OBJ_MASS (obj) + IS_CARRYING_W (vict) > CAN_CARRY_W (vict))
	{
		act ("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
		return;
	}

	obj_from_char (&obj, count);

	act ("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT | _ACT_FORMAT);
	act ("$n gives you $p.", 0, ch, obj, vict, TO_VICT | _ACT_FORMAT);
	act ("You give $p to $N.", 1, ch, obj, vict, TO_CHAR | _ACT_FORMAT);

	obj_to_char (obj, vict);

	if (obj->ocues)
	{
		typedef std::multimap<obj_cue,std::string>::const_iterator N;
		std::pair<N,N> range = obj->ocues->equal_range (ocue_on_give);
		for (N n = range.first; n != range.second; n++)
		{
			std::string cue = n->second;
			if (!cue.empty ())
			{
				const char *r = cue.c_str();
				char *p;
				char reflex[AVG_STRING_LENGTH] = "";
				strtol (r+2, &p, 0);
				strcpy (reflex, p);
				if (strncmp(r,"g ",2) == 0)
					command_interpreter (ch, reflex);
				else if (vict && strncmp(r,"r ",2) == 0)
					command_interpreter (vict, reflex);
			}
		}
	}


}

OBJ_DATA *
	get_equip (CHAR_DATA * ch, int location)
{
	OBJ_DATA *obj = NULL;

	if (location == WEAR_SHIELD)
	{
		if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_SHIELD)
			return ch->right_hand;
		if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_SHIELD)
			return ch->left_hand;
		return NULL;
	}

	if (ch->right_hand && ch->right_hand->location == location)
		return ch->right_hand;

	if (ch->left_hand && ch->left_hand->location == location)
		return ch->left_hand;

	if (location != WEAR_SHIELD)
	{
		for (obj = ch->equip; obj; obj = obj->next_content)
			if (obj->location == location)
				return obj;
	}

	return NULL;
}

OBJ_DATA *
	get_equip_arg (CHAR_DATA * ch, int location, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (location == WEAR_SHIELD)
	{
		if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_SHIELD && isname (argument, ch->right_hand->name))
			return ch->right_hand;
		if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_SHIELD && isname (argument, ch->left_hand->name))
			return ch->left_hand;
		return NULL;
	}

	if (ch->right_hand && ch->right_hand->location == location && isname (argument, ch->right_hand->name))
		return ch->right_hand;

	if (ch->left_hand && ch->left_hand->location == location && isname (argument, ch->left_hand->name))
		return ch->left_hand;

	if (location != WEAR_SHIELD)
	{
		for (obj = ch->equip; obj; obj = obj->next_content)
			if (obj->location == location && isname (argument, obj->name))
				return obj;
	}

	return NULL;
}

void
	do_sip (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *container;
	OBJ_DATA *drink;
	char buf[MAX_STRING_LENGTH] = "";
	std::string first_person, third_person;

	argument = one_argument (argument, buf);

	if (!(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		act ("You can't find it.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (GET_ITEM_TYPE (container) != ITEM_DRINKCON &&
		GET_ITEM_TYPE (container) != ITEM_FOUNTAIN)
	{
		act ("You cannot drink from $p.", false, ch, container, 0, TO_CHAR);
		return;
	}

	if (!(drink = container->contains))
	{
		act ("$p is empty.", false, ch, container, 0, TO_CHAR);
		return;
	}


	sprintf (buf, "%s", char_short(ch));
	*buf = toupper(*buf);
	first_person.assign("You sip some of ");
	first_person.append("#2");
	first_person.append(obj_short_desc(drink));
	first_person.append("#0 from #2");
	first_person.append(obj_short_desc(container));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buf);
	third_person.append("#0 sips some of ");
	third_person.append("#2");
	third_person.append(obj_short_desc(drink));
	third_person.append("#0 from #2");
	third_person.append(obj_short_desc(container));
	third_person.append("#0");

	bool tasted;
	std:string taste;

	if (drink->ink_color && str_cmp(drink->ink_color, "(null)"))
	{
		taste.assign("It tastes ");
		taste.append(drink->ink_color);
		taste.append(".\n");
	}

	evaluate_emote_string(ch, &first_person, third_person, argument);
}

void
	do_drink (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *container;
	OBJ_DATA *drink;
	char buf[MAX_STRING_LENGTH] = "";
	//char buf2[MAX_STRING_LENGTH];
	std::string first_person, third_person;
	int sips = 1, range = 0;
	const char *verbose_liquid_amount [] = {"", "some of the ", "a lot of the ", "most of the ", "all of the "};
	//POISON_DATA *poison;
	int poisoned = 0;

	argument = one_argument (argument, buf);

	if (!(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		act ("You can't find it.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (GET_ITEM_TYPE (container) != ITEM_DRINKCON &&
		GET_ITEM_TYPE (container) != ITEM_FOUNTAIN)
	{
		act ("You cannot drink from $p.", false, ch, container, 0, TO_CHAR);
		return;
	}

	/*
	if (get_soma_affect(ch, SOMA_DIGESTIVE_ULCER))
	{
	act ("The nausea emanating from your stomach drives any thought of drinking far from your mind.", false, ch, 0, 0, TO_CHAR);
	return;
	}
	*/

	if (!(drink = container->contains))
	{
		act ("$p is empty.", false, ch, container, 0, TO_CHAR);
		return;
	}

	if (*argument != '(' && *argument)
	{
		argument = one_argument (argument, buf);
		if (just_a_number(buf))
		{
			if (drink->count < atoi(buf) && drink->count != -1)
			{
				send_to_char ("There simply isn't that much to drink!\n", ch);
				return;
			}

			if (atoi(buf) < 1)
			{
				send_to_char ("As amusing as regurgitation can be, you may not drink negative amounts.\n", ch);
				return;
			}

			sips = atoi(buf);
		}
		else
		{
			send_to_char ("The correct syntax is #3drink <container> [<amount>] [(emote)]#0.\n", ch);
			return;
		}
	}

	if (sips > drink->count)
		sips = drink->count;

	sips = MIN(100, sips);

	if (sips > 1)
		range = 1;
	if (sips > (container->o.od.value[0] / 3))
		range = 2;
	if (sips > (container->o.od.value[0] * 2 / 3))
		range = 3;
	if (sips == container->o.od.value[0] || drink->count == -1 )
		range = 4;

	sprintf (buf, "%s", char_short(ch));
	*buf = toupper(*buf);
	first_person.assign("You drink ");
	first_person.append(verbose_liquid_amount[range]);
	first_person.append("#2");
	first_person.append(obj_short_desc(drink));
	first_person.append("#0 from #2");
	first_person.append(obj_short_desc(container));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buf);
	third_person.append("#0 drinks ");
	third_person.append(verbose_liquid_amount[range]);
	third_person.append("#2");
	third_person.append(obj_short_desc(drink));
	third_person.append("#0 from #2");
	third_person.append(obj_short_desc(container));
	third_person.append("#0");

	bool tasted;
	std:string taste;

	if (drink->ink_color && str_cmp(drink->ink_color, "(null)"))
	{
		taste.assign("It tastes ");
		taste.append(drink->ink_color);
		taste.append(".\n");
	}

	if (evaluate_emote_string(ch, &first_person, third_person, argument))
	{
	    sips *= 10;
		if (!tasted)
		{
			if (drink->ink_color && str_cmp(drink->ink_color, "(null)"))
				send_to_char(taste.c_str(), ch);

			tasted = true;
		}

		if (ch->intoxication != -1)
		{
			if (number (6, 20) > ((ch->con + ch->wil) / 2))
				ch->intoxication += (drink->o.fluid.alcohol * sips);
			else
				ch->intoxication += ((drink->o.fluid.alcohol * sips) / 2);
		}

		ch->thirst += (drink->o.fluid.water * sips);

		// Takes account of new starvation system - if you're absolutely near death,
		// then you don't need to eat a full week's worth of food just to get to starving
		// again. It should ony take one day's worth of food, or two 2-course meals, to
		// get you back to merely malnourished.

		if (ch->hunger >= 0)
		{
			ch->hunger += (drink->o.fluid.food * sips);
		}
		else if (ch->hunger <= -25 && ch->hunger >= -48)
		{
			ch->hunger += 2 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 2;
		}
		else if (ch->hunger <= -49 && ch->hunger >= -72)
		{
			ch->hunger += 3 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 2;
		}
		else if (ch->hunger <= -73 && ch->hunger >= -96)
		{
			ch->hunger += 4 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 2;
		}
		else if (ch->hunger <= -97 && ch->hunger >= -120)
		{
			ch->hunger += 5 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 2;
		}
		else if (ch->hunger <= -121 && ch->hunger >= -144)
		{
			ch->hunger += 6 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 6;
		}
		else if (ch->hunger <= -145)
		{
			ch->hunger += 7 * (drink->o.fluid.food * sips);

			if (ch->hunger > 2)
				ch->hunger = 2;
		}
		else
		{
			ch->hunger += (drink->o.fluid.food * sips);
		}


		if (ch->hunger > 48)
			ch->hunger = 48;

		if (ch->thirst > 300)
			ch->thirst = 300;

		if (GET_ITEM_TYPE(container) != ITEM_FOUNTAIN)
		{
			obj_from_obj(&drink, sips);
			extract_obj(drink);
		}
	}
}


void
	do_eat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	//char buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	std::string first_person, third_person, taste;
	int bites = 1, range = 0, old_count = 1;
	const char *verbose_bites_amount [] = {"", "a bite of ", "a few bites of ", "a lot of ", "most of ", "all of "};
	//POISON_DATA *poison;
	int poisoned = 0;
	OBJ_DATA *tobj = NULL;
	bool tasted = false;

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->equip)))
	{
		send_to_char ("You can't find it.\n", ch);
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_FOOD)
	{
		send_to_char ("That isn't food.  You can't eat it.\n", ch);
		return;
	}

	if (obj->equiped_by && obj->obj_flags.type_flag != ITEM_FOOD)
	{
		send_to_char ("You must remove that item before destroying it.\n", ch);
		return;
	}

	/*
	if (get_soma_affect(ch, SOMA_DIGESTIVE_ULCER))
	{
	act ("The nausea emanating from your stomach drives any thought of eating far from your mind.", false, ch, 0, 0, TO_CHAR);
	return;
	}
	*/

	if (*argument != '(' && *argument)
	{
		argument = one_argument (argument, buf);
		if (just_a_number(buf))
		{
			if (obj->o.food.bites < atoi(buf) && obj->o.food.bites != -1)
			{
				send_to_char ("There simply isn't that much to eat!\n", ch);
				return;
			}

			if (atoi(buf) < 1)
			{
				send_to_char ("As amusing as regurgitation can be, you may not eat negative amounts.\n", ch);
				return;
			}

			bites = atoi(buf);
		}
		else if (!strcmp (buf, "all"))
		{
			bites = MAX (1, obj->o.food.bites);
			if ((obj->o.food.food_value + ch->hunger) > 48)
			{
				send_to_char ("You are much too full to eat that much!\n", ch);
				return;
			}

		}
		else
		{
			send_to_char ("The correct syntax is #3eat <food> [<amount>] [(emote)]#0.\n", ch);
			return;
		}

	}

	if (obj->o.food.bites > 1)
		range = 1;
	if (bites > 1)
		range = 2;
	if (bites > (obj->o.food.bites / 3) && bites != 1 && obj->o.food.bites > 4)
		range = 3;
	if (bites > (obj->o.food.bites * 2 / 3) && bites != 1)
		range = 4;
	if (bites == obj->o.food.bites && bites != 1)
		range = 5;
	if (obj->o.food.bites == -1)
		range = 1;

	sprintf(buf, "%s", char_short(ch));
	*buf = toupper(*buf);

	old_count = obj->count;
	obj->count = 1;

	first_person.assign("You eat ");
	first_person.append(verbose_bites_amount[range]);
	first_person.append("#2");
	first_person.append(obj_short_desc(obj));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buf);
	third_person.append("#0 eats ");
	third_person.append(verbose_bites_amount[range]);
	third_person.append("#2");
	third_person.append(obj_short_desc(obj));
	third_person.append("#0");

	if (obj->ink_color && str_cmp(obj->ink_color, "(null)"))
	{
		taste.assign("It tastes ");
		taste.append(obj->ink_color);
		taste.append(".\n");
	}

	obj->count = old_count;

	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		if (obj->equiped_by)
			unequip_char (ch, obj->location);

		for (int i = 0; i < bites; i++)
		{

			/*
			if (obj->poison && IS_MORTAL (ch) && obj->obj_flags.type_flag == ITEM_FOOD)
			{
			for (poison = obj->poison; poison; poison = poison->next)
			{

			if (number(1,25) > GET_CON(ch))
			{
			soma_add_affect(ch, poison->poison_type, poison->duration, poison->latency,
			poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
			poison->attack, poison->decay, poison->sustain, poison->release);

			poisoned = poison->poison_type;
			}

			if (obj->poison->uses == 0 || obj->poison->uses == 1)
			remove_object_poison(obj, obj->poison);
			else if (obj->poison->uses != -1)
			obj->poison->uses --;
			}
			}
			*/

			if (!tasted)
			{
				if (obj->ink_color && str_cmp(obj->ink_color, "(null)"))
					send_to_char(taste.c_str(), ch);

				tasted = true;
			}



			// Takes account of new starvation system - if you're absolutely near death,
			// then you don't need to eat a full week's worth of food just to get to starving
			// again. It should ony take one day's worth of food, or two 2-course meals, to
			// get you back to merely malnourished.

			if (ch->hunger >= 0)
			{
				ch->hunger += get_bite_value (obj);
			}
			else if (ch->hunger <= -25 && ch->hunger >= -48)
			{
				ch->hunger += 2 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 2;
			}
			else if (ch->hunger <= -49 && ch->hunger >= -72)
			{
				ch->hunger += 3 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 2;
			}
			else if (ch->hunger <= -73 && ch->hunger >= -96)
			{
				ch->hunger += 4 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 2;
			}
			else if (ch->hunger <= -97 && ch->hunger >= -120)
			{
				ch->hunger += 5 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 2;
			}
			else if (ch->hunger <= -121 && ch->hunger >= -144)
			{
				ch->hunger += 6 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 6;
			}
			else if (ch->hunger <= -145)
			{
				ch->hunger += 7 * get_bite_value (obj);

				if (ch->hunger > 2)
					ch->hunger = 2;
			}
			else
			{
				ch->hunger += get_bite_value (obj);
			}

			if (ch->hunger > 48)
				ch->hunger = 48;

			if (ch->thirst > 300)
				ch->thirst = 300;

			obj->o.food.bites--;

			if (obj->count > 1 && obj->o.food.bites < 1)
			{
				obj->count--;
				obj->o.food.bites = vtoo (obj->nVirtual)->o.food.bites;
			}

			else if (obj->o.food.bites < 1 && obj->count <= 1)
			{
				if (obj->o.food.junk)
				{
					if (obj->var_color[0])
						tobj = load_colored_object(obj->o.food.junk, obj->var_color[0], 0, 0, 0, 0, 0, 0, 0, 0, 0);
					else
						tobj = load_object(obj->o.food.junk);

					if (tobj)
						obj_to_char(tobj, ch);
				}

				extract_obj (obj);

			}
		}

		if (ch->hunger > 48)
			ch->hunger = 48;
		if (ch->hunger > 36)
			act ("You are full.", false, ch, 0, 0, TO_CHAR);

		if (poisoned > 0)
			send_to_char(lookup_poison_variable(poisoned, 1), ch);

	}
}

void
	do_fill (CHAR_DATA * ch, char *argument, int cmd)
{
	int volume_to_transfer;
	OBJ_DATA *from;
	OBJ_DATA *to;
	OBJ_DATA *liquid;
	char buf[MAX_STRING_LENGTH];
	//POISON_DATA *poison;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("FILL <object> from/with <object>\n", ch);
		send_to_char ("Example:  fill bucket from well\n", ch);
		send_to_char ("          fill bucket well      (same thing)\n", ch);
		return;
	}

	if (!(to = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{

		if (get_obj_in_dark (ch, buf, ch->room->contents))
		{
			send_to_char ("It is too dark for that.\n", ch);
			return;
		}

		send_to_char ("Fill what?\n", ch);

		return;
	}

	if (GET_ITEM_TYPE (to) != ITEM_DRINKCON)
	{
		act ("You can't fill $p.", false, ch, to, 0, TO_CHAR);
		return;
	}

	if (to->contains && to->contains->count >= to->o.drinks.capacity)
	{
		act ("$p is full already.", false, ch, to, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "with") || !str_cmp (buf, "from"))
		argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (from = ch->room->contents; from; from = from->next_content)
		{
			if ((GET_ITEM_TYPE (from) == ITEM_FOUNTAIN || GET_ITEM_TYPE (from) == ITEM_DRINKCON)
				&& CAN_SEE_OBJ (ch, from))
			{
				break;
			}
		}

		if (!from)
		{
			act ("Fill $p from what?", false, ch, to, 0, TO_CHAR);
			return;
		}
	}
	else if (!(from = get_obj_in_dark (ch, buf, ch->room->contents)))
	{
		act ("Fill $p from what?", false, ch, to, 0, TO_CHAR);
		return;
	}

	if (GET_ITEM_TYPE (from) != ITEM_FOUNTAIN &&
		GET_ITEM_TYPE (from) != ITEM_DRINKCON)
	{
		act ("There is no way to fill $p from $P.",
			false, ch, to, from, TO_CHAR);
		return;
	}

	if (!from->contains)
	{
		act ("$p is empty.", false, ch, from, 0, TO_CHAR);
		return;
	}

	if (to->contains && to->contains->nVirtual != from->contains->nVirtual)
	{
		send_to_char ("You shouldn't mix fluids.\n", ch);
		return;
	}
	else if (to->contains && !vo_match_color(to->contains, from->contains))
	{
		send_to_char ("You shouldn't mix fluids.\n", ch);
		return;
	}

	// Can't transfer more than our to's capacity.
	int max_transfer = to->o.drinks.capacity;
	// Our maximum is reduced by the amount we already have in our to.
	max_transfer -= (to->contains ? to->contains->count : 0);
	// And we can't transfer more fluid than we have in our from..
	max_transfer = MIN(max_transfer, (from->contains->count));

	if (max_transfer <= 0)
	{
		act ("$p is already full, or there isn't enough in $P to pour.", false, ch, to, from, TO_CHAR);
		return;
	}

	sprintf (buf, "You fill $p from $P with %s.", obj_short_desc(from->contains));
	act (buf, false, ch, to, from, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "$n fills $p from $P with %s.", obj_short_desc(from->contains));
	act (buf, true, ch, to, from, TO_ROOM | _ACT_FORMAT);

	liquid = from->contains;
	obj_from_obj(&liquid, max_transfer);
	obj_to_obj(liquid, to);

	if (GET_ITEM_TYPE(from) == ITEM_FOUNTAIN && from->contains)
	{
		from->contains->count = from->o.drinks.volume;
	}
}

void
	do_pour (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *from;
	OBJ_DATA *to;
	OBJ_DATA *liquid;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	//POISON_DATA *poison;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What do you want to pour?\n", ch);
		return;
	}

	if (!(from = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(from = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		send_to_char ("You can't find it.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (from) != ITEM_DRINKCON)
	{
		act ("You can't pour from $p.\n", false, ch, from, 0, TO_CHAR);
		return;
	}

	if (!from->contains)
	{
		act ("$p is empty.", false, ch, from, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf2);

	if (!str_cmp (buf2, "out"))
	{
		do_empty (ch, buf, 0);
		return;
	}

	if (!*buf2)
	{
		send_to_char ("What do you want to pour it into?", ch);
		return;
	}

	if (!(to = get_obj_in_dark (ch, buf2, ch->right_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf2, ch->left_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf2, ch->room->contents)))
	{
		act ("You can't find it to pour $p into.", false, ch, from, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (GET_ITEM_TYPE (to) != ITEM_DRINKCON)
	{
		act ("You can't pour into $p.", false, ch, to, 0, TO_CHAR);
		return;
	}

	if (to->contains &&	from->contains->nVirtual != to->contains->nVirtual)
	{
		act ("If you want to fill $p with another liquid, then empty it first.",
			false, ch, to, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	// Can't transfer more than our to's capacity.
	int max_transfer = to->o.drinks.capacity;
	// Our maximum is reduced by the amount we already have in our to.
	max_transfer -= (to->contains ? to->contains->count : 0);
	// And we can't transfer more fluid than we have in our from..
	max_transfer = MIN(max_transfer, (from->contains->count));

	if (max_transfer <= 0)
	{
		act ("$p is already full, or there isn't enough in $P to pour.", false, ch, to, from, TO_CHAR);
		return;
	}

	sprintf (buf, "You pour %s from $p into $P.", obj_short_desc(from->contains));
	act (buf, false, ch, from, to, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "$n pours %s from $p into $P.", obj_short_desc(from->contains));
	act (buf, true, ch, from, to, TO_ROOM | _ACT_FORMAT);

	if (GET_ITEM_TYPE (to) == ITEM_DRINKCON)
	{
		liquid = from->contains;
		obj_from_obj(&liquid, max_transfer);
		obj_to_obj(liquid, to);
	}
}

/* functions related to wear */

void
	perform_wear (CHAR_DATA * ch, OBJ_DATA * obj, int keyword)
{
	switch (keyword)
	{

	case 0:

		if (obj->o.light.hours < 1)
		{
			act ("You hold $p and realize it is spent.",
				true, ch, obj, 0, TO_CHAR);
		}

		else
		{
			if (!obj->o.light.on)
			{
				act ("You light $p and hold it.", false, ch, obj, 0, TO_CHAR);
				act ("$n lights $p and holds it.", false, ch, obj, 0, TO_ROOM);
			}
			else
			{
				act ("You hold $p.", false, ch, obj, 0, TO_CHAR);
				act ("$n holds $p.", false, ch, obj, 0, TO_ROOM);
			}
		}
		break;

	case 1:
		act ("$n wears $p on $s finger.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case 2:
		act ("$n wears $p around $s neck.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p around your neck.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 3:
		act ("$n wears $p on $s body.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your body.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		if (!obj->size)
			obj->size = get_size (ch);
		break;
	case 4:
		act ("$n wears $p on $s head.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your head.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 5:
		act ("$n wears $p on $s legs.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your legs.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		if (!obj->size)
			obj->size = get_size (ch);
		break;
	case 6:
		act ("$n wears $p on $s feet.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your feet.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 7:
		act ("$n wears $p on $s hands.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your hands.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 8:
		act ("$n wears $p on $s arms.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your arms.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		if (!obj->size)
			obj->size = get_size (ch);
		break;
	case 9:
		act ("$n wears $p about $s body.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p about your body.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 10:
		act ("$n wears $p about $s waist.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p about your waist.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 11:
		act ("$n wears $p around $s wrist.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case 12:
		act ("$n wields $p.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		act ("You wield $p.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		break;
	case 14:
		act ("$n starts using $p.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		break;
	case 15:
		act ("$n affixes $p to $s belt.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You affix $p to your belt.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 16:
		act ("$n stores $p across $s back.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You store $p across your back.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 17:
		break;
	case 18:
		act ("$n wears $p around $s neck.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p around your neck.", false, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 19:
		act ("$n wears $p on $s ears.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your ears.", false, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 20:
		act ("$n slings $p over $s shoulder.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case 21:
		act ("$n wears $p around $s ankle.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case 22:
		act ("$n wears $p in $s hair.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p in your hair.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 23:
		act ("$n wears $p on $s face.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You wear $p on your face.", true, ch, obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case 24:
	case 25:
	case 26:
		act ("$n wears $p around $s upper arm.", true, ch, obj, 0,
			TO_ROOM | _ACT_FORMAT);
		/* TO_CHAR act occurs in wear() */
		break;
	case 27:
		act ("$n wears $p over $s body.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		act ("You wear $p over your body.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		break;
	case 28:
		act ("$n wears $p over $s eyes.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		act ("You wear $p over your eyes.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		break;
	case 29:
		act ("$n slips $p up around $s hips!", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		act ("You slip $p up around your hips.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		break;
	case 13:
		act ("$n wears $p around $s chest!", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		act ("You wear $p around your chest.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		break;
	}
}

OBJ_DATA *
	covered_loc (CHAR_DATA *ch, OBJ_DATA *obj_object)
{
	OBJ_DATA *tobj = NULL;

	if (GET_ITEM_TYPE(obj_object) == ITEM_ARMOR)
	{
		for (int i = 0; i < MAX_HITLOC; i++)
		{
			for (tobj = ch->equip; tobj; tobj = tobj->next_content)
			{
				if (GET_ITEM_TYPE(tobj) == ITEM_ARMOR && IS_SET(tobj->o.od.value[2], 1 << i) && IS_SET(obj_object->o.od.value[2], 1 << i))
				{
					return tobj;
				}

				/*
				if (GET_ITEM_TYPE(tobj) == ITEM_ARMOR && IS_SET(tobj->o.od.value[3], 1 << i) && IS_SET(obj_object->o.od.value[3], 1 << i))
				{
				return 1;
				}
				*/
			}
		}
	}
	/*
	else if (GET_ITEM_TYPE(obj_object) == ITEM_WORN)
	{
	for (int i = 0; i < MAX_HITLOC; i++)
	{
	for (tobj = ch->equip; tobj; tobj = tobj->next_content)
	{
	if (GET_ITEM_TYPE(tobj) == ITEM_WORN && IS_SET(tobj->o.od.value[2], 1 << i) && IS_SET(obj_object->o.od.value[2], 1 << i))
	{
	return tobj;
	}


	if (GET_ITEM_TYPE(tobj) == ITEM_WORN && IS_SET(tobj->o.od.value[3], 1 << i) && IS_SET(obj_object->o.od.value[3], 1 << i))
	{
	return 1;
	}

	}
	}
	}*/

	return 0;
}

void
	wear (CHAR_DATA * ch, OBJ_DATA * obj_object, int keyword)
{
	char buffer[MAX_STRING_LENGTH];

	char *hour_names[] =
	{
		"",
		"the remainder of the",
		"two", "three", "four", "five", "six", "seven", "eight", "nine",
		"ten", "eleven", "twelve", "quite a few"
	};
	int hours;

	if (IS_SET (obj_object->obj_flags.extra_flags, ITEM_MOUNT) &&
		!IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("That item is for animals.\n", ch);
		return;
	}

	if (!IS_SET (obj_object->obj_flags.extra_flags, ITEM_MOUNT) &&
		IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("You are an animal.  That item isn't for you.\n", ch);
		return;
	}

	if (obj_object->attached && GET_ITEM_TYPE (obj_object) == ITEM_SLING)
	{
		send_to_char ("You'll need to unload that, first.\n", ch);
		return;
	}

	// Can't wear a strung bow, to prevent the instant sniper-to-melee problem.
	/*

	if (keyword != 12 && GET_ITEM_TYPE (obj_object) == ITEM_WEAPON &&
	(obj_object->o.weapon.use_skill == SKILL_ARCHERY)
	&& obj_object->o.od.value[5] == 1)
	{
	send_to_char("You'll need to destring that first.\n", ch);
	return;
	}
	*/

	if (GET_ITEM_TYPE (obj_object) == ITEM_LIGHT
		&& obj_object->o.light.on == true
		&& !IS_SET (obj_object->obj_flags.extra_flags, ITEM_MAGIC))
	{
		send_to_char ("You'll need to snuff that, first.\n", ch);
		return;
	}


	if (keyword == 13 && obj_object->obj_flags.type_flag == ITEM_LIGHT)
		keyword = 0;

	switch (keyword)
	{

	case 0:			/* LIGHT SOURCE */
		if (get_equip (ch, WEAR_LIGHT))
			send_to_char ("You are already holding a light source.\n", ch);

		else if (!can_handle (obj_object, ch))
			send_to_char ("Your hands are occupied.\n", ch);
		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_LIGHT);

			light (ch, obj_object, true, false);

			hours = obj_object->o.light.hours;

			if (!hours)
			{
				send_to_char ("It doesn't produce light.\n", ch);
				break;
			}

			sprintf (buffer, "It will provide light for %s hour%s.\n",
				hours < 13 ? hour_names[hours] : hour_names[13],
				hours > 1 ? "s" : "");

			send_to_char (buffer, ch);
		}

		break;

	case 1:
		if (CAN_WEAR (obj_object, ITEM_WEAR_FINGER))
		{

			if (get_equip (ch, WEAR_FINGER_L) && get_equip (ch, WEAR_FINGER_R))
				send_to_char ("You are already wearing something on your "
				"ring fingers.\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);

			else
			{

				perform_wear (ch, obj_object, keyword);

				if (get_equip (ch, WEAR_FINGER_L))
				{
					sprintf (buffer,
						"You slip the %s on your right ring finger.\n",
						fname (obj_object->name));
					send_to_char (buffer, ch);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_FINGER_R);
				}

				else
				{
					sprintf (buffer,
						"You slip the %s on your left ring finger.\n",
						fname (obj_object->name));
					send_to_char (buffer, ch);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_FINGER_L);
				}
			}
		}
		else
			send_to_char ("You can't wear that on your ring finger.\n", ch);
		break;

	case 2:
		if (CAN_WEAR (obj_object, ITEM_WEAR_NECK))
		{
			if (get_equip (ch, WEAR_NECK_1) || get_equip (ch, WEAR_NECK_2))
				send_to_char ("You can only wear one thing around your "
				"neck.\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (get_equip (ch, WEAR_NECK_1))
				{
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_NECK_2);
				}
				else
				{
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_NECK_1);
				}
			}
		}
		else
			send_to_char ("You can't wear that around your neck.\n", ch);
		break;

	case 3:
		if (CAN_WEAR (obj_object, ITEM_WEAR_BODY))
		{
			if (get_equip (ch, WEAR_BODY))
				send_to_char ("You already wear something on your body."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			/*
			else if (obj_object->size && obj_object->size != get_size (ch))
			{
			if (obj_object->size > get_size (ch))
			act ("$p won't fit, it is too large.",
			true, ch, obj_object, 0, TO_CHAR);
			else
			act ("$p won't fit, it is too small.",
			true, ch, obj_object, 0, TO_CHAR);
			}
			*/

			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_BODY);
			}
		}
		else
			send_to_char ("You can't wear that on your body.\n", ch);
		break;

	case 4:
		if (CAN_WEAR (obj_object, ITEM_WEAR_HEAD))
		{
			if (get_equip (ch, WEAR_HEAD))
				send_to_char ("You already wear something on your head."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_HEAD);
			}
		}
		else
			send_to_char ("You can't wear that on your head.\n", ch);
		break;

	case 5:
		if (CAN_WEAR (obj_object, ITEM_WEAR_LEGS))
		{
			if (get_equip (ch, WEAR_LEGS))
				send_to_char ("You already wear something on your legs."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_LEGS);
			}
		}
		else
			send_to_char ("You can't wear that on your legs.\n", ch);
		break;

	case 6:
		if (CAN_WEAR (obj_object, ITEM_WEAR_FEET))
		{
			if (get_equip (ch, WEAR_FEET))
				send_to_char ("You already wear something on your feet."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				obj_from_char (&obj_object, 0);
				equip_char (ch, obj_object, WEAR_FEET);
			}
		}
		else
			send_to_char ("You can't wear that on your feet.\n", ch);
		break;

	case 7:
		if (CAN_WEAR (obj_object, ITEM_WEAR_HANDS))
		{
			if (get_equip (ch, WEAR_HANDS))
				send_to_char ("You already wear something on your hands."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_HANDS);
			}
		}
		else
			send_to_char ("You can't wear that on your hands.\n", ch);
		break;

	case 8:
		if (CAN_WEAR (obj_object, ITEM_WEAR_ARMS))
		{
			if (get_equip (ch, WEAR_ARMS))
				send_to_char ("You already wear something on your arms."
				"\n", ch);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_ARMS);
			}
		}
		else
			send_to_char ("You can't wear that on your arms.\n", ch);
		break;

	case 9:
		if (CAN_WEAR (obj_object, ITEM_WEAR_ABOUT))
		{
			if (get_equip (ch, WEAR_ABOUT))
			{
				send_to_char ("You already wear something about your body.\n",
					ch);
			}
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_ABOUT);
			}
		}
		else
		{
			send_to_char ("You can't wear that about your body.\n", ch);
		}
		break;
	case 10:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_WAIST))
			{
				if (get_equip (ch, WEAR_WAIST))
				{
					send_to_char
						("You already wear something about your waist.\n", ch);
				}
				else if (covered_loc(ch, obj_object))
					act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_WAIST);
				}
			}
			else
			{
				send_to_char ("You can't wear that about your waist.\n", ch);
			}
		}
		break;
	case 11:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_WRIST))
			{
				if (get_equip (ch, WEAR_WRIST_L) && get_equip (ch, WEAR_WRIST_R))
				{
					send_to_char
						("You already wear something around both your wrists.\n",
						ch);
				}
				else if (covered_loc(ch, obj_object))
					act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_WRIST_L))
					{
						sprintf (buffer,
							"You wear the %s around your right wrist.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_WRIST_R);
					}
					else
					{
						sprintf (buffer,
							"You wear the %s around your left wrist.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_WRIST_L);
					}
				}
			}
			else
			{
				send_to_char ("You can't wear that around your wrist.\n", ch);
			}
		}
		break;

	case 12:
		if (CAN_WEAR (obj_object, ITEM_WIELD) && !(obj_object->o.od.value[0] == 0))
		{
			/*
			if ( !can_handle (obj_object, ch) ) {
			send_to_char ("Your hands are occupied!\n", ch);
			return;
			}
			*/
			switch (obj_object->o.od.value[0])
			{
			case 1:	// primary weapons

				if (ch->str < 18)
				{
					if (get_equip (ch, WEAR_PRIM))
					{
						send_to_char
							("You are already wielding a primary weapon.\n", ch);
						return;
					}
					else if (get_equip (ch, WEAR_BOTH))
					{
						send_to_char
							("You are already wielding a two-handed weapon.\n",
							ch);
						return;
					}
					else
					{
						send_to_char ("OK.\n", ch);
						perform_wear (ch, obj_object, keyword);
						equip_char (ch, obj_object, WEAR_PRIM);
					}
					break;
				}		// > 17 str or troll wields ME in either hand.

			case 2:	// Light weapons, and should include brawling weapons.
				//case SKILL_SLING:
				if (get_equip (ch, WEAR_PRIM) && get_equip (ch, WEAR_SEC))
				{
					send_to_char
						("You are already wielding both a primary and a secondary weapon.\n",
						ch);
					return;
				}
				if (get_equip (ch, WEAR_BOTH))
				{
					send_to_char
						("You are already wielding a two-handed weapon.\n", ch);
					return;
				}
				send_to_char ("OK.\n", ch);
				perform_wear (ch, obj_object, keyword);
				if (!get_equip (ch, WEAR_PRIM))
					equip_char (ch, obj_object, WEAR_PRIM);
				else
					equip_char (ch, obj_object, WEAR_SEC);
				break;

			case 4:
				if (get_equip (ch, WEAR_BOTH) ||
					get_equip (ch, WEAR_PRIM) || get_equip (ch, WEAR_SEC))
				{
					send_to_char
						("You cannot wield this weapon while wielding another.\n",
						ch);
					return;
				}
				send_to_char ("OK.\n", ch);
				perform_wear (ch, obj_object, keyword);
				equip_char (ch, obj_object, WEAR_PRIM);
				break;

			case 3:	// Heavy weapons.
				if (ch->str >= 20)
				{
					// Extremely strong chars can wield two-handed weapons with one hand.
					if (get_equip (ch, WEAR_PRIM))
					{
						send_to_char
							("You are already wielding a primary weapon.\n", ch);
						return;
					}
					else if (get_equip (ch, WEAR_BOTH))
					{
						send_to_char
							("You are already wielding a two-handed weapon.\n",
							ch);
						return;
					}
					else
					{
						send_to_char ("OK.\n", ch);
						perform_wear (ch, obj_object, keyword);
						if (get_equip (ch, WEAR_SEC)
							|| (ch->right_hand && ch->left_hand))
							equip_char (ch, obj_object, WEAR_PRIM);
						else
							equip_char (ch, obj_object, WEAR_BOTH);
					}
					break;
				}
				if (get_equip (ch, WEAR_BOTH) ||
					get_equip (ch, WEAR_PRIM) ||
					get_equip (ch, WEAR_SEC) ||
					(ch->right_hand && ch->left_hand))
				{
					send_to_char ("You need both hands to wield this weapon.\n",
						ch);
					return;
				}
				send_to_char ("OK.\n", ch);
				perform_wear (ch, obj_object, keyword);
				equip_char (ch, obj_object, WEAR_BOTH);
				break;
			}

		}
		else
		{
			send_to_char ("You can't wield that.\n", ch);
		}
		break;

	case 14:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_SHIELD))
			{
				if (get_equip (ch, WEAR_SHIELD))
					send_to_char ("You are already using a shield.\n", ch);

				else if (!can_handle (obj_object, ch))
					send_to_char ("Your hands are occupied.\n", ch);

				else
				{
					perform_wear (ch, obj_object, keyword);
					sprintf (buffer, "You start using the %s.\n",
						fname (obj_object->name));
					send_to_char (buffer, ch);
					equip_char (ch, obj_object, WEAR_SHIELD);
				}
			}
			else
			{
				send_to_char ("You can't use that as a shield.\n", ch);
			}
		}
		break;
	case 15:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_BELT))
			send_to_char ("You cannot wear that on your belt.\n", ch);

		else if (!get_equip (ch, WEAR_WAIST))
			send_to_char ("You need a belt to wear that.\n", ch);

		else if (get_equip (ch, WEAR_BELT_1) && get_equip (ch, WEAR_BELT_2))
			send_to_char ("Your belt is full.\n", ch);

		else
		{
			int belt_loc;

			/* Mostly I expect pouches to be equiped here.
			put them in the second belt loc first */

			if (!get_equip (ch, WEAR_BELT_2))
				belt_loc = WEAR_BELT_2;
			else
				belt_loc = WEAR_BELT_1;

			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, belt_loc);
		}
		break;

	case 16:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_BACK))
			send_to_char ("You cannot wear that across your back.\n", ch);

		else if (get_equip (ch, WEAR_BACK))
			send_to_char ("You are already wearing something there.\n", ch);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_BACK);
		}
		break;

	case 17:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_BLINDFOLD))
			send_to_char ("You cannot wear that over your eyes.\n", ch);

		else if (get_equip (ch, WEAR_BLINDFOLD))
			send_to_char ("Something already covers your eyes.\n", ch);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_BLINDFOLD);
		}

		break;

	case 18:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_THROAT))
			send_to_char ("You cannot wear that around your throat.\n", ch);

		else if (get_equip (ch, WEAR_THROAT))
			act ("You are already wearing $p around your throat.",
			false, ch, get_equip (ch, WEAR_THROAT), 0, TO_CHAR);
		else if (covered_loc(ch, obj_object))
			act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_THROAT);
		}

		break;

	case 19:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_EAR))
			send_to_char ("You cannot wear that on your ears.\n", ch);

		else if (get_equip (ch, WEAR_EAR))
			act ("You are already wearing $p on your ears.",
			false, ch, get_equip (ch, WEAR_EAR), 0, TO_CHAR);
		else if (covered_loc(ch, obj_object))
			act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_EAR);
		}

		break;

	case 20:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_SHOULDER))
			{
				if (get_equip (ch, WEAR_SHOULDER_L) &&
					get_equip (ch, WEAR_SHOULDER_R))
				{
					send_to_char
						("You already wear something on both shoulders.\n", ch);
				}
				else if (covered_loc(ch, obj_object))
					act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_SHOULDER_L))
					{
						sprintf (buffer,
							"You sling the %s over your right shoulder.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_SHOULDER_R);
					}
					else
					{
						sprintf (buffer,
							"You sling the %s over your left shoulder.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_SHOULDER_L);
					}
				}
			}
			else
			{
				send_to_char ("You can't wear that on your shoulder.\n", ch);
			}
		}
		break;

	case 21:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_ANKLE))
			{
				if (get_equip (ch, WEAR_ANKLE_L) && get_equip (ch, WEAR_ANKLE_R))
				{
					send_to_char
						("You already wear something around both your ankles.\n",
						ch);
				}
				else if (covered_loc(ch, obj_object))
					act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_ANKLE_L))
					{
						sprintf (buffer,
							"You wear the %s around your right ankle.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_ANKLE_R);
					}
					else
					{
						sprintf (buffer,
							"You wear the %s around your left ankle.\n",
							fname (obj_object->name));
						send_to_char (buffer, ch);
						equip_char (ch, obj_object, WEAR_ANKLE_L);
					}
				}
			}
			else
			{
				send_to_char ("You can't wear that around your ankle.\n", ch);
			}
		}
		break;

	case 22:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_HAIR))
			send_to_char ("You cannot wear that in your hair.\n", ch);

		else if (get_equip (ch, WEAR_HAIR))
			act ("You are already wearing $p in your hair.",
			false, ch, get_equip (ch, WEAR_HAIR), 0, TO_CHAR);
		else if (covered_loc(ch, obj_object))
			act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_HAIR);
		}

		break;

	case 23:
		{
			if (!CAN_WEAR (obj_object, ITEM_WEAR_FACE))
				send_to_char ("You cannot wear that on your face.\n", ch);

			else if (get_equip (ch, WEAR_FACE))
				act ("You are already wearing $p on your face.",
				false, ch, get_equip (ch, WEAR_FACE), 0, TO_CHAR);
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);

			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_FACE);
			}

			break;
		}




		/* ARMBANDS, PATCHES, ARMLETS, ETC */
	case 24:
	case 25:
	case 26:
		{
			if (!CAN_WEAR (obj_object, ITEM_WEAR_ARMBAND))
			{
				send_to_char ("You can't wear that around your upper arm.\n", ch);
				return;;
			}

			if ((keyword == 24 && get_equip (ch, WEAR_ARMBAND_R)
				&& get_equip (ch, WEAR_ARMBAND_L))
				|| (keyword == 25 && get_equip (ch, WEAR_ARMBAND_R))
				|| (keyword == 26 && get_equip (ch, WEAR_ARMBAND_L)))
			{
				send_to_char
					("You already wearing something around your upper arm.\n", ch);
				return;
			}

			perform_wear (ch, obj_object, keyword);

			if (obj_object == ch->right_hand)
			{
				ch->right_hand = NULL;
			}
			else if (obj_object == ch->left_hand)
			{
				ch->left_hand = NULL;
			}

			if (keyword == 25
				|| (keyword == 24 && !get_equip (ch, WEAR_ARMBAND_R)))
			{
				sprintf (buffer, "You wear the %s around your upper right arm.\n",
					fname (obj_object->name));
				send_to_char (buffer, ch);
				equip_char (ch, obj_object, WEAR_ARMBAND_R);
			}
			else
			{
				sprintf (buffer, "You wear the %s around your upper left arm.\n",
					fname (obj_object->name));
				send_to_char (buffer, ch);
				equip_char (ch, obj_object, WEAR_ARMBAND_L);
			}
			break;
		}

	case 27:
		if (CAN_WEAR (obj_object, ITEM_WEAR_OVER))
		{
			if (get_equip (ch, WEAR_OVER))
			{
				send_to_char ("You already wear something over your body.\n",
					ch);
			}
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_OVER);
			}
		}
		else
		{
			send_to_char ("You can't wear that over your body.\n", ch);
		}
		break;

	case 28:
		if (CAN_WEAR (obj_object, ITEM_WEAR_EYES))
		{
			if (get_equip (ch, WEAR_EYES))
			{
				send_to_char ("You already wear something over your eyes.\n",
					ch);
			}
			else if (covered_loc(ch, obj_object))
				act ("You cannot wear $p, as $P already covers that spot.", false, ch, obj_object, covered_loc(ch, obj_object), TO_CHAR | _ACT_FORMAT);
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_EYES);
			}
		}
		else
		{
			send_to_char ("You can't wear that over your eyes.\n", ch);
		}
		break;

	case 29:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_UNDERWEAR))
			send_to_char ("You cannot slip into that.\n", ch);

		else if (get_equip (ch, WEAR_UNDERWEAR))
			send_to_char ("You are already wearing underwear.\n", ch);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_UNDERWEAR);
		}

		break;
	case 13:
		if (!CAN_WEAR (obj_object, ITEM_WEAR_OVERWEAR))
			send_to_char ("You cannot slip into that.\n", ch);

		else if (get_equip (ch, WEAR_OVERWEAR))
			send_to_char ("You are already wearing something there.\n", ch);

		else
		{
			perform_wear (ch, obj_object, keyword);
			if (obj_object == ch->right_hand)
				ch->right_hand = NULL;
			else if (obj_object == ch->left_hand)
				ch->left_hand = NULL;
			equip_char (ch, obj_object, WEAR_OVERWEAR);
		}

		break;

	case -1:
		{
			sprintf (buffer, "Wear %s where?.\n", fname (obj_object->name));
			send_to_char (buffer, ch);
		}
		break;
	case -2:
		{
			sprintf (buffer, "You can't wear the %s.\n",
				fname (obj_object->name));
			send_to_char (buffer, ch);
		}
		break;
	default:
		{
			sprintf (buffer, "Unknown type called in wear, obj VNUM %d.",
				obj_object->nVirtual);
			system_log (buffer, true);
		}
		break;
	}
}

void
	do_wear (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[256];
	char buffer[MAX_STRING_LENGTH];
	OBJ_DATA *obj_object;
	int keyword;
	/* These match a switch/case statement in wear() above */
	static char *keywords[] =
	{
		"finger",			/* 1 */
		"neck",
		"body",
		"head",
		"legs",			/* 5 */
		"feet",
		"hands",
		"arms",
		"about",
		"waist",			/* 10 */
		"wrist",
		"HOLDER",			/* Someone miss something here?  - Rassilon */
		"overwear",
		"shield",
		"belt",			/* 15 */
		"back",
		"blindfold",		/* 17 */
		"throat",			/* 18 */
		"ears",			/* 19 */
		"shoulder",			/* 20 */
		"ankle",
		"hair",
		"face",			/* 23 */
		"armband",			/* 24 */
		"armbandright",		/* 25 */
		"armbandleft",		/* 26 */
		"over",
		"eyes",
		"underwear",
		"\n"
	};

	*arg1 = '\0';
	*arg2 = '\0';

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (*arg1)
	{
		if (!strn_cmp (arg1, ".c", 2))
		{
			obj_object = get_obj_in_list_id (atoi (arg2), ch->right_hand);
			if (!obj_object)
				obj_object = get_obj_in_list_id (atoi (arg2), ch->left_hand);
		}
		else
		{
			obj_object = get_obj_in_dark (ch, arg1, ch->right_hand);
			if (!obj_object)
				obj_object = get_obj_in_dark (ch, arg1, ch->left_hand);
		}
		if (obj_object)
		{
			if (*arg2 && !isdigit (*arg2))
			{
				keyword = search_block (arg2, keywords, false);	/* Partial Match */
				if (keyword == -1)
				{
					sprintf (buf, "%s is an unknown body location.\n", arg2);
					send_to_char (buf, ch);
				}
				else
				{
					wear (ch, obj_object, keyword + 1);
				}
			}
			else
			{
				keyword = -2;

				if (CAN_WEAR (obj_object, ITEM_WEAR_SHIELD))
					keyword = 14;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FINGER))
					keyword = 1;
				if (CAN_WEAR (obj_object, ITEM_WEAR_NECK))
					keyword = 2;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WRIST))
					keyword = 11;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WAIST))
					keyword = 10;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ARMS))
					keyword = 8;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HANDS))
					keyword = 7;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FEET))
					keyword = 6;
				if (CAN_WEAR (obj_object, ITEM_WEAR_LEGS))
					keyword = 5;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ABOUT))
					keyword = 9;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HEAD))
					keyword = 4;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BODY))
					keyword = 3;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BELT))
					keyword = 15;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BACK))
					keyword = 16;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BLINDFOLD))
					keyword = 17;
				if (CAN_WEAR (obj_object, ITEM_WEAR_THROAT))
					keyword = 18;
				if (CAN_WEAR (obj_object, ITEM_WEAR_EAR))
					keyword = 19;
				if (CAN_WEAR (obj_object, ITEM_WEAR_SHOULDER))
					keyword = 20;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ANKLE))
					keyword = 21;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HAIR))
					keyword = 22;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FACE))
					keyword = 23;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ARMBAND))
					keyword = 24;
				if (CAN_WEAR (obj_object, ITEM_WEAR_OVER))
					keyword = 27;
				if (CAN_WEAR (obj_object, ITEM_WEAR_EYES))
					keyword = 28;
				if (CAN_WEAR (obj_object, ITEM_WEAR_UNDERWEAR))
					keyword = 29;
				if (CAN_WEAR (obj_object, ITEM_WEAR_OVERWEAR))
					keyword = 13;
					
				if (obj_object->activation &&
					IS_SET (obj_object->obj_flags.extra_flags,
					ITEM_WEAR_AFFECT))
					obj_activate (ch, obj_object);

				wear (ch, obj_object, keyword);
			}
		}
		else
		{
			sprintf (buffer, "You do not seem to have the '%s'.\n", arg1);
			send_to_char (buffer, ch);
		}
	}
	else
	{
		send_to_char ("Wear what?\n", ch);
	}
}

void
	do_wield (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj_object;
	int keyword = 12;
	SECOND_AFFECT *sa;
	char buffer[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (!*arg1)
	{
		send_to_char ("Wield what?\n", ch);
		return;
	}

	if (!str_cmp (arg1, ".c"))
	{
		obj_object = get_obj_in_list_id (atoi (arg2), ch->right_hand);
		if (!obj_object)
			obj_object = get_obj_in_list_id (atoi (arg2), ch->left_hand);
	}
	else
	{
		obj_object = get_obj_in_dark (ch, arg1, ch->right_hand);
		if (!obj_object)
			obj_object = get_obj_in_dark (ch, arg1, ch->left_hand);
	}

	if (obj_object)
	{
		if (obj_object->location == WEAR_PRIM
			|| obj_object->location == WEAR_SEC
			|| obj_object->location == WEAR_BOTH)
		{
			send_to_char ("You're already wielding that!\n", ch);
			return;
		}

		if ((sa = get_second_affect (ch, SA_WEAR_OBJ, obj_object)))
			return;

		if (obj_object->activation &&
			IS_SET (obj_object->obj_flags.extra_flags, ITEM_WIELD_AFFECT))
			obj_activate (ch, obj_object);

		// If you're in a brawl room, fighting, didn't currently have a weapon,
		// and your opponent doesn't have a weapon, you crim-flag yourself.

		if (IS_SET(ch->room->room_flags, BRAWL) && IS_SET(ch->room->room_flags, LAWFUL)
			&& ch->fighting && !has_weapon(ch) && !has_weapon(ch->fighting) && !is_wanted_in_area(ch->fighting)
			&& (*argument == '!' || *arg2 == '!'))
		{
			criminalize (ch, ch->fighting, vnum_to_room (ch->fighting->in_room)->zone, CRIME_ASSAULT);
		}
		else if (IS_SET(ch->room->room_flags, BRAWL) && IS_SET(ch->room->room_flags, LAWFUL)
			&& ch->fighting && !has_weapon(ch) && !has_weapon(ch->fighting) && !is_wanted_in_area(ch->fighting))
		{
			sprintf (buffer, "You are in a lawful, but brawable area; if you wield that weapon, you'll be wanted for assault. Type \'#6wield %s !#0\', without the quotes, to continue.", arg1);
			act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		wear (ch, obj_object, keyword);

	}

	else
	{
		sprintf (buffer, "You do not seem to have the '%s'.\n", arg1);
		send_to_char (buffer, ch);
	}
}

void
	do_remove (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *arrow = NULL;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	char location[MAX_STRING_LENGTH];
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *eq;
	LODGED_OBJECT_INFO *lodged;
	int removed = 0, target_found = 0;
	int modif = 0;
	int target_obj = 0, target_char = 0;
	char *tmp;
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

	/*
	if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC)
	&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
	send_to_char ("This command has been disabled in OOC zones.\n", ch);
	return;
	}
	*/

	argument = one_argument (argument, arg1);

	if (!*arg1)
	{
		send_to_char ("Remove what?1\n", ch);
		return;
	}

	if (IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("Mounts can't use this command.\n", ch);
		return;
	}

	if ((point = strcspn(arg1, ",")) && point != strlen(arg1))
	{
		tmp = strtok(arg1, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}

	for (obj = ch->equip; obj; obj = obj->next_content)
	{
		if (IS_OBJ_VIS (ch, obj) &&
			((!twofer && isname (arg1, obj->name)) ||
			(twofer && isname(mod, obj->name) && isname(tmp, obj->name))))
			break;
	}

	if (!obj)
	{
		if (ch->right_hand)
		{
			if (isname (arg1, ch->right_hand->name))
				obj = ch->right_hand;
		}
		if (ch->left_hand)
		{
			if (isname (arg1, ch->left_hand->name))
				obj = ch->left_hand;
		}
	}

	if ((get_equip (ch, WEAR_BOTH) && obj != get_equip (ch, WEAR_BOTH))
		|| ((ch->right_hand && obj != ch->right_hand)
		&& (ch->left_hand && obj != ch->left_hand)))
	{
		send_to_char ("Your hands are otherwise occupied, at the moment.\n",
			ch);
		return;
	}


	if (!obj && *arg1)
	{
		if ((tch = get_char_room_vis (ch, arg1)))
		{
			target_found++;
			target_char++;
		}

		//for (obj = ch->room->contents; obj; obj = obj->next_content)
		//{
		// if (IS_OBJ_VIS (ch, obj) && isname (arg1, obj->name))
		// {
		// target_found++;
		// target_obj++;
		// break;
		// }
		//}

		if ((obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
		{
			target_found++;
			target_obj++;
		}
		if (!tch && !obj)
		{
			send_to_char ("Remove what?2\n", ch);
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
			send_to_char ("Remove what?3\n", ch);
			return;
		}

		if (target_char)
		{
			if (GET_POS (tch) > POSITION_RESTING && IS_MORTAL (ch) && !IS_SET(tch->act, ACT_PASSIVE))
			{
				send_to_char
					("The target must be resting before you can remove a lodged item.\n",
					ch);
				return;
			}

			/***
			remove target object area
			***/
			argument = one_argument (argument, arg3);

			for (lodged = tch->lodged; lodged; lodged = lodged->next)
			{
				if (isname (arg2, vtoo (lodged->vnum)->name))
				{
					//we have a specified location, and it doesn't match this wound
					//so we break out and go to the next wound. If there is no
					//specified location, then we continue with the regular code.
					if ((*arg3) && strcmp(arg3, lodged->location))
						continue;
					else
					{
						sprintf (location, "%s", lodged->location);
						if (lodged->colored == 1)
							obj = load_colored_object (lodged->vnum, lodged->var_color, lodged->var_color2, lodged->var_color3, 0, 0, 0, 0, 0, 0, 0);
						else
							obj = load_object (lodged->vnum);
						obj->count = 1;


						if (GET_ITEM_TYPE(obj) == ITEM_BULLET)
						{
							send_to_char("That item cannot be removed so simply: you will need to extract it instead.\n", ch);
							extract_obj(obj);
							return;
						}

						int error = 0;

						if (can_obj_to_inv (obj, ch, &error, 1))
							obj_to_char (obj, ch);
						else
							obj_to_room (obj, ch->in_room);

						lodge_from_char (tch, lodged);
						removed++;
						break;
					}
				}
			}

			if (removed && tch == ch)
			{
				*buf = '\0';
				*buf2 = '\0';
				sprintf (buf, "You carefully work #2%s#0 loose, wincing in pain as removing it opens the wound on your %s anew.",
					obj->short_description, expand_wound_loc (location));

				sprintf (buf2, "%s#0 carefully works #2%s#0 loose, wincing in pain as removing it opens the wound on %s %s anew.",
					char_short (ch), obj->short_description, HSHR (ch), expand_wound_loc (location));
				*buf2 = toupper (*buf2);
				sprintf (buffer, "#5%s", buf2);
				sprintf (buf2, "%s", buffer);
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

				if ((strcmp(location, "skull")) && (strcmp(location, "reye"))
					&& (strcmp(location, "leye")) && (strcmp(location, "abdomen"))
					&& (strcmp(location, "groin")) && (strcmp(location, "muzzle")))
					modif = 0; //not in those loctions, so normal chance
				else
					modif = 20; //in a bad spot, so a penalty to healing check

				if (skill_use (ch, SKILL_MEDICINE, modif))
					return; // no extra damage if you make your heal skill check
				else
				{
					wound_to_char (tch, location, dice (obj->o.od.value[0], obj->o.od.value[1]),
						0, number (1, 4), 0, 0);
					return;
				}
			}
			else if (removed && tch != ch)
			{
				sprintf (buf,
					"You carefully work #2%s#0 loose.",
					obj->short_description);
				sprintf (buf2,
					"%s#0 carefully works #2%s#0 loose.",
					char_short (ch), obj->short_description);
				*buf2 = toupper (*buf2);
				sprintf (buffer, "#5%s", buf2);
				sprintf (buf2, "%s", buffer);
				act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				act (buf2, false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
				sprintf (buf,
					"%s#0 carefully works #2%s#0 loose, wincing as removing it opens the wound on your %s anew.",
					char_short (ch), obj->short_description,
					expand_wound_loc (location));
				*buf = toupper (*buf);
				sprintf (buffer, "#5%s", buf);
				sprintf (buf, "%s", buffer);
				act (buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);

				if ((strcmp(location, "skull")) &&
					(strcmp(location, "reye")) &&
					(strcmp(location, "leye")) &&
					(strcmp(location, "abdomen")) &&
					(strcmp(location, "groin")) &&
					(strcmp(location, "muzzle")))
					modif = 0; //not in those loctions, so normal chance
				else
					modif = 20; //in a bad spot, so a penalty to healing check

				if (skill_use (ch, SKILL_MEDICINE, modif))
					return; // no extra damage if you make your heal skill check
				else
				{
					wound_to_char (tch, location, dice (obj->o.od.value[0], obj->o.od.value[1]),
						0, number (1, 4), 0, 0);
					return;
				}
			}
			else if (!removed)
			{
				send_to_char("You don't see that -- how could you remove it?\n", ch);
				return;
			}
		} // if (target_char)

		else if (target_obj)
		{

			for (lodged = obj->lodged; lodged; lodged = lodged->next)
			{

				if (isname (arg2, vtoo (lodged->vnum)->name))
				{
					sprintf (location, "%s", lodged->location);

					if (lodged->colored == 1)
						arrow = load_colored_object (lodged->vnum, lodged->var_color, lodged->var_color2, lodged->var_color3, 0, 0, 0, 0, 0, 0, 0);
					else
						arrow = load_object (lodged->vnum);
					arrow->count = 1;
					obj_to_char (arrow, ch);
					lodge_from_obj (obj, lodged);
					removed++;
					break;
				}
			}

			if (removed)
			{
				sprintf (buf, "You retrieve #2%s#0 from #2%s#0's %s.",
					arrow->short_description, obj->short_description,
					expand_wound_loc (location));
				sprintf (buf2, "%s#0 retrieves #2%s#0 from #2%s#0's %s.",
					char_short (ch), arrow->short_description,
					obj->short_description, expand_wound_loc (location));
				*buf2 = toupper (*buf2);
				sprintf (buffer, "#5%s", buf2);
				sprintf (buf2, "%s", buffer);
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
			else if (!removed)
			{
				send_to_char
					("You don't see that -- how could you remove it?\n", ch);
				return;
			}
		} //if (target_obj)
	} //if (!obj && *arg1)

	if (!obj)
	{
		send_to_char ("Remove what?4\n", ch);
		return;
	}

	if (obj->location == -1)
	{
		send_to_char ("You don't need to remove that!\n", ch);
		return;
	}

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_NOREMOVE))
	{
		act ("You are unable to remove $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (SWIM_ONLY (ch->room))
	{
		act ("You begin attempting to remove $p, hindered by the water. . .",
			false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		ch->delay_type = DEL_WATER_REMOVE;
		ch->delay_who = (char *) obj;
		ch->delay = 45 - number (1, 15);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE)
	{
		act ("You can't remove $p on your own - someone will need to unpack it from you.", true, ch, obj, 0, TO_CHAR);
		return;
	}

	if (obj->location == WEAR_WAIST)
	{
		if ((eq = get_equip (ch, WEAR_BELT_1)))
		{
			act ("$p falls to the floor.", true, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", true, ch, eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_1), ch->in_room);
		}

		if ((eq = get_equip (ch, WEAR_BELT_2)))
		{
			act ("$p falls to the floor.", true, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", true, ch, eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_2), ch->in_room);
		}
	}

	if (obj->attached)
		do_unload (ch, "", 0);

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
		do_hood (ch, "", 0);

	if (obj->location == WEAR_LIGHT && GET_ITEM_TYPE (obj) == ITEM_LIGHT)
		light (ch, obj, false, true);

	if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH || obj->location == WEAR_SHIELD)
		unequip_char (ch, obj->location);
	else
		obj_to_char (unequip_char (ch, obj->location), ch);

    if (IS_SET (obj->obj_flags.extra_flags2, ITEM_CONCEALED))
        obj->obj_flags.extra_flags2 &= ~ITEM_CONCEALED;
     
    if (obj->attire)
        obj->attire = (char *) NULL;
           
	act ("You stop using $p.", false, ch, obj, 0, TO_CHAR);
	act ("$n stops using $p.", true, ch, obj, 0, TO_ROOM | _ACT_BLEED);
}

int
	can_handle (OBJ_DATA * obj, CHAR_DATA * ch)
{
	int wear_count = 0;

	if (get_equip (ch, WEAR_BOTH))
		wear_count = wear_count + 2;
	if (get_equip (ch, WEAR_SHIELD))
		wear_count++;
	if (get_equip (ch, WEAR_LIGHT))
		wear_count++;
	if (get_equip (ch, WEAR_PRIM))
		wear_count++;
	if (get_equip (ch, WEAR_SEC))
		wear_count++;

	if (wear_count > 2)
	{
		return 0;
	}

	if (wear_count == 2)
		return 0;

	if (obj->o.od.value[0] == 0 && get_equip (ch, WEAR_PRIM))
		return 0;

	if ((obj->obj_flags.wear_flags & ITEM_WIELD) && obj->o.od.value[0] == 3)
	{
		/* Two-handed */
		if (wear_count == 0)
			return 1;
		else
			return 0;
	}

	return 1;
}



#define IS_SHEATHABLE(obj) ( (obj != NULL)  && ( ( GET_ITEM_TYPE(obj) == ITEM_FIREARM ) || ( GET_ITEM_TYPE(obj) == ITEM_ROUND ) || ( GET_ITEM_TYPE(obj) == ITEM_CLIP )  || ( GET_ITEM_TYPE(obj) == ITEM_MISSILE ) || ( GET_ITEM_TYPE(obj) == ITEM_WEAPON && obj->o.weapon.use_skill != SKILL_AIM) ) )

void
	do_sheathe (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH] = "";
	char arg2[MAX_STRING_LENGTH] = "";
	char buf[MAX_STRING_LENGTH] = "";
	OBJ_DATA *obj = NULL;
	OBJ_DATA *obj_prim;
	OBJ_DATA *obj_sec;
	OBJ_DATA *obj_both;
	OBJ_DATA *sheath = NULL;
	char *msg;
	int i = 0, sheathed = 0;
	std::string first_person, third_person;

	if (*argument != '(')
	{
		argument = one_argument (argument, arg1);
		if (*argument != '(')
			argument = one_argument (argument, arg2);
	}

	obj_prim = get_equip (ch, WEAR_PRIM);
	obj_sec = get_equip (ch, WEAR_SEC);
	obj_both = get_equip (ch, WEAR_BOTH);

	if (!*arg1)
	{
		if (obj_prim && IS_OBJ_VIS (ch, obj_prim))
		{
			obj = obj_prim;
		}
		else if (obj_sec && IS_OBJ_VIS (ch, obj_sec))
		{
			obj = obj_sec;
		}
		else if (obj_both && IS_OBJ_VIS (ch, obj_both))
		{
			obj = obj_both;
		}
		else if (IS_SHEATHABLE (ch->right_hand))
		{
			obj = ch->right_hand;
		}
		else if (IS_SHEATHABLE (ch->left_hand))
		{
			obj = ch->left_hand;
		}
	}
	else if (obj_prim &&
		IS_OBJ_VIS (ch, obj_prim) && isname (arg1, obj_prim->name))
	{
		obj = obj_prim;
	}
	else if (obj_sec &&
		IS_OBJ_VIS (ch, obj_sec) && isname (arg1, obj_sec->name))
	{
		obj = obj_sec;
	}
	else if (obj_both &&
		IS_OBJ_VIS (ch, obj_both) && isname (arg1, obj_both->name))
	{
		obj = obj_both;
	}
	else if (IS_SHEATHABLE (ch->right_hand)
		&& IS_OBJ_VIS (ch, ch->right_hand)
		&& isname (arg1, ch->right_hand->name))
	{
		obj = ch->right_hand;
	}
	else if (IS_SHEATHABLE (ch->left_hand)
		&& IS_OBJ_VIS (ch, ch->left_hand)
		&& isname (arg1, ch->left_hand->name))
	{
		obj = ch->left_hand;
	}

	if (!obj)
	{
		if (!*arg1)
			send_to_char ("You aren't wielding anything.\n", ch);
		else
			send_to_char ("You aren't wielding that.\n", ch);
		return;
	}

	if (!IS_SHEATHABLE (obj))
	{
		send_to_char ("You can only sheath a melee weapon, missile or firearm.\n", ch);
		return;
	}

	if (IS_NPC (ch) && IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BELT))
	{
		one_argument (obj->name, buf);
		do_wear (ch, buf, 0);
		return;
	}

	if (*arg2)
	{
		if (!(sheath = get_obj_in_list_vis (ch, arg2, ch->equip)))
		{
			send_to_char ("What did you want to sheathe it in?\n", ch);
			return;
		}
		if (can_obj_to_container (obj, sheath, &msg, 1))
			sheathed++;
	}
	else if (!*arg2)
	{
		for (i = 0; i < MAX_WEAR; i++)
		{
			if (!(sheath = get_equip (ch, i)))
				continue;

			if (GET_ITEM_TYPE (sheath) != ITEM_SHEATH
				&& GET_ITEM_TYPE (sheath) != ITEM_HOLSTER
				&& GET_ITEM_TYPE (sheath) != ITEM_AMMO_BELT
				&& GET_ITEM_TYPE (sheath) != ITEM_BANDOLIER
				&& GET_ITEM_TYPE (sheath) != ITEM_QUIVER)
			{
				continue;
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_WEAPON
				&& GET_ITEM_TYPE (sheath) != ITEM_SHEATH)
			{
				continue;
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_FIREARM
				&& GET_ITEM_TYPE (sheath) != ITEM_HOLSTER)
			{
				continue;
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_CLIP
				&& GET_ITEM_TYPE (sheath) != ITEM_AMMO_BELT)
			{
				continue;
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_ROUND
				&& GET_ITEM_TYPE (sheath) != ITEM_BANDOLIER)
			{
				continue;
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_MISSILE
				&& GET_ITEM_TYPE (sheath) != ITEM_QUIVER)
			{
				continue;
			}
			if (!can_obj_to_container (obj, sheath, &msg, 1))
				continue;
			sheathed++;
			break;
		}
	}

	if (!sheathed)
	{
		send_to_char
			("You aren't wearing anything capable of bearing that item.\n", ch);
		return;
	}

	sprintf (buf, "%s", char_short(ch));
	*buf = toupper(*buf);
	if (GET_ITEM_TYPE(sheath) == ITEM_HOLSTER)
		first_person.assign("You holster #2");
	else if (GET_ITEM_TYPE(sheath) == ITEM_AMMO_BELT || GET_ITEM_TYPE(sheath) == ITEM_BANDOLIER)
		first_person.assign("You pocket #2");
	else
		first_person.assign("You sheath #2");
	first_person.append(obj_short_desc(obj));
	first_person.append("#0 in #2");
	first_person.append(obj_short_desc(sheath));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buf);
	if (GET_ITEM_TYPE(sheath) == ITEM_HOLSTER)
		third_person.append("#0 holsters #2");
	else if (GET_ITEM_TYPE(sheath) == ITEM_AMMO_BELT || GET_ITEM_TYPE(sheath) == ITEM_BANDOLIER)
		third_person.append("#0 pockets #2");
	else
		third_person.append("#0 sheathes #2");
	third_person.append(obj_short_desc(obj));
	third_person.append("#0 in #2");
	third_person.append(obj_short_desc(sheath));
	third_person.append("#0");

	if (evaluate_emote_string (ch, &first_person, third_person, argument) )
	{
		if (ch->right_hand == obj)
			ch->right_hand = NULL;
		else if (ch->left_hand == obj)
			ch->left_hand = NULL;
		obj_to_obj (obj, sheath);
	}

	return;
}

void
	do_draw (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH] = "";
	int obj_destination = 0, i;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *sheath;
	std::string first_person, third_person;
	bool crim = false;
	char buf[MAX_STRING_LENGTH];

	if (*argument != '(')
		argument = one_argument (argument, arg1);

	for (i = 0; i < MAX_WEAR; i++)
	{
		if (!(sheath = get_equip (ch, i)))
			continue;
		if (GET_ITEM_TYPE (sheath) != ITEM_SHEATH && GET_ITEM_TYPE (sheath) != ITEM_HOLSTER)
			continue;
		if (*arg1)
		{
			if (!(obj = get_obj_in_list_vis (ch, arg1, sheath->contains)))
				continue;
			break;
		}
		else if ((!*arg1) && sheath->contains)
		{
			obj = sheath->contains;
			break;
		}
	}

	if (!obj && get_equip (ch, WEAR_BELT_1) &&
		GET_ITEM_TYPE (get_equip (ch, WEAR_BELT_1)) == ITEM_WEAPON)
		obj = get_equip (ch, WEAR_BELT_1);
	else if (!obj && get_equip (ch, WEAR_BELT_2) &&
		GET_ITEM_TYPE (get_equip (ch, WEAR_BELT_2)) == ITEM_WEAPON)
		obj = get_equip (ch, WEAR_BELT_2);

	if (!obj)
	{
		if (!*arg1)
			send_to_char ("You have nothing to draw!\n", ch);
		else
			send_to_char ("You don't have that in a sheath.\n", ch);
		return;
	}

	if (!IS_SET (obj->obj_flags.wear_flags, ITEM_WIELD))
	{
		act ("You can't wield $o.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	switch (obj->o.od.value[0])
	{
	case 1:	// Medium weapons.
		if (ch->str < 18)
		{
			if (get_equip (ch, WEAR_PRIM))
			{
				send_to_char ("You are already wielding a primary weapon.\n",
					ch);
				return;
			}
			else if (get_equip (ch, WEAR_BOTH))
			{
				send_to_char ("You are already wielding a two-handed weapon.\n",
					ch);
				return;
			}
			else
			{
				obj_destination = WEAR_PRIM;
			}
			break;
		}			// > 17 str wields ME in either hand.
	case 2:	// Light weapons.

		if (get_equip (ch, WEAR_PRIM) && get_equip (ch, WEAR_SEC))
		{
			send_to_char
				("You are already wielding both a primary and a secondary weapon.\n",
				ch);
			return;
		}
		if (get_equip (ch, WEAR_BOTH))
		{
			send_to_char ("You are already wielding a two-handed weapon.\n",
				ch);
			return;
		}
		if (!get_equip (ch, WEAR_PRIM))
			obj_destination = WEAR_PRIM;
		else
			obj_destination = WEAR_SEC;
		break;

	case 4:
		if (get_equip (ch, WEAR_BOTH) ||
			get_equip (ch, WEAR_PRIM) ||
			get_equip (ch, WEAR_SEC) || (ch->right_hand || ch->left_hand))
		{
			send_to_char ("You need both hands to wield this weapon.\n", ch);
			return;
		}
		obj_destination = WEAR_BOTH;
		break;
	case 3: // HEAVY WEAPONS
		if (ch->str >= 20)
		{
			// Extremely strong chars can wield two-handed weapons with one hand.
			if (get_equip (ch, WEAR_PRIM))
			{
				send_to_char ("You are already wielding a primary weapon.\n",
					ch);
				return;
			}
			else if (get_equip (ch, WEAR_BOTH))
			{
				send_to_char ("You are already wielding a two-handed weapon.\n",
					ch);
				return;
			}
			else
			{
				if (get_equip (ch, WEAR_SEC)
					|| (ch->right_hand || ch->left_hand))
					obj_destination = WEAR_PRIM;
				else
					obj_destination = WEAR_BOTH;
			}
			break;
		}
		if (get_equip (ch, WEAR_BOTH) ||
			get_equip (ch, WEAR_PRIM) ||
			get_equip (ch, WEAR_SEC) || (ch->right_hand || ch->left_hand))
		{
			send_to_char ("You need both hands to wield this weapon.\n", ch);
			return;
		}
		obj_destination = WEAR_BOTH;
		break;
	}

	if (obj_destination == WEAR_BOTH && (ch->right_hand || ch->left_hand))
	{
		send_to_char ("You'll need both hands free to draw this weapon.\n", ch);
		return;
	}
	else if ((obj_destination == WEAR_PRIM || obj_destination == WEAR_SEC)
		&& (ch->right_hand && ch->left_hand))
	{
		send_to_char ("You'll need a free hand to draw that weapon.\n", ch);
		return;
	}

	if (IS_SET(ch->room->room_flags, BRAWL) && IS_SET(ch->room->room_flags, LAWFUL)
		&& ch->fighting && !has_weapon(ch) && !is_wanted_in_area(ch->fighting)
		&& !has_weapon(ch->fighting) && (*argument == '!' || *arg1 == '!'))
	{
		crim = true;
	}
	else if (IS_SET(ch->room->room_flags, BRAWL) && IS_SET(ch->room->room_flags, LAWFUL)
		&& ch->fighting && !has_weapon(ch) && !has_weapon(ch->fighting) && !is_wanted_in_area(ch->fighting))
	{
		sprintf (buf, "You are in a lawful, but brawable area; if you wield that weapon, you'll be wanted for assault. Type \'#6draw %s !#0\', without the quotes, to continue.", arg1);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}


	unequip_char (ch, obj->location);

	if (obj->in_obj)
		obj_from_obj (&obj, 0);

	equip_char (ch, obj, obj_destination);

	if (obj_destination == WEAR_BOTH)
		ch->right_hand = obj;
	else if (obj_destination == WEAR_PRIM)
	{
		if (ch->right_hand)
			ch->left_hand = obj;
		else
			ch->right_hand = obj;
	}
	else if (obj_destination == WEAR_SEC)
	{
		if (ch->right_hand)
			ch->left_hand = obj;
		else
			ch->right_hand = obj;
	}

	sprintf (arg1, "%s", char_short(ch));
	arg1[0] = toupper(arg1[0]);

	first_person.assign("You draw #2");
	first_person.append(obj_short_desc(obj));
	third_person.assign("#5");
	third_person.append(arg1);
	third_person.append("#0 draws #2");
	third_person.append(obj_short_desc(obj));
	if (sheath)
	{
		first_person.append("#0 from #2");
		first_person.append(obj_short_desc(sheath));
		third_person.append("#0 from #2");
		third_person.append(obj_short_desc(sheath));
	}
	first_person.append("#0");
	third_person.append("#0");

	if (!(evaluate_emote_string (ch, &first_person, third_person, argument)))
	{
		if (ch->right_hand == obj)
			ch->right_hand = NULL;
		else if (ch->left_hand == obj)
			ch->left_hand = NULL;

		obj_to_obj(obj, sheath);
	}
	else
	{
		if (crim)
			criminalize (ch, ch->fighting, vnum_to_room (ch->fighting->in_room)->zone, CRIME_ASSAULT);
	}
}

void
	do_butcher (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *tch;

	if (!((ch->right_hand && GET_ITEM_TYPE (ch->right_hand)
		== ITEM_WEAPON && (ch->right_hand->o.weapon.use_skill == SKILL_SMALL_BLADE))
		|| (ch->left_hand && GET_ITEM_TYPE (ch->left_hand)
		== ITEM_WEAPON && (ch->left_hand->o.weapon.use_skill == SKILL_SMALL_BLADE))))
	{

		send_to_char
			("You need to be holding some sort of knife to properly butcher.\n",
			ch);
		return;
	}

	argument = one_argument (argument, arg);

	/* Allow the user to enter a carcass name in case he wants to
	say 2.carcass instead of just carcass */

	if (!*arg)
		strcpy (arg, "carcass");

	obj = get_obj_in_list_vis (ch, arg, ch->room->contents);

	if (!obj)
	{
		send_to_char ("You don't see a carcass here.\n", ch);
		return;
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->delay_info1 == (long int) obj)
		{
			send_to_char
				("Someone's already butchering that carcass at the moment.\n", ch);
			return;
		}
	}

	if (GET_ITEM_TYPE(obj) != ITEM_CARCASS)
	{
		send_to_char ("You can only butcher carcasses.\n", ch);
		return;
	}

	if (!obj->o.od.value[0])
	{
		send_to_char
			("After a moment of poking, you realize that it isn't worth butchering.\n",
			ch);
		return;
	}

	ch->delay_info1 = (long int) obj;

	skill_learn(ch, SKILL_BUTCHERY);

	if (!real_skill (ch, SKILL_BUTCHERY))
		send_to_char ("You begin hacking away at the carcass.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 30)
		send_to_char ("You begin flailing at the carcass.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 45)
		send_to_char ("You begin slashing into the carcass.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 60)
		send_to_char ("You begin carving the carcass.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 75)
		send_to_char ("You begin operating on the carcass.\n", ch);
	else
		send_to_char ("You begin delicately stripping meat from the carcass.\n", ch);

	sprintf (buf, "$n begins to butcher #2%s#0.", obj->short_description);

	act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	watched_action(ch, "butcher a carcass", 0, 1);

	ch->delay_type = DEL_BUTC_1;
	ch->delay = 12;

	// Add time to the object timer so it doesn't decay while we're skinning it.  -Methuselah
	if ((obj->obj_timer - time(0)) < 1000)
		obj->obj_timer = time(0) + 1000;

	// Add time to the object->morphTime timer in case it's close to morphing.  -Methuselah
	if ((obj->morphTime - time(0)) < 1000)
		obj->morphTime = time(0) + 1000;
}



void
	delayed_butcher1 (CHAR_DATA * ch)
{
	OBJ_DATA *obj;

	// make sure the carcass is still here, if not throw an error and abort.
	obj = (OBJ_DATA *) ch->delay_info1;

	// is it really a carcass?
	if (GET_ITEM_TYPE(obj) != ITEM_CARCASS)
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (!obj)
	{
		// The carcass being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (CAN_SEE_OBJ (ch, obj))
	{
		send_to_char ("You cut through the bones and gristle of the carcass as you strip the meat from it.\n", ch);
		act ("$n cuts through the bones and gristle of $p.", false, ch, obj, ch,
			TO_ROOM | _ACT_FORMAT);

		ch->delay_type = DEL_BUTC_2;
		ch->delay = 12;
	}
	else
	{
		// Can't see the carcass anymore
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see any carcass to butcher.\n", ch);
		return;
	}
}

void
	delayed_butcher2 (CHAR_DATA * ch)
{
	OBJ_DATA *obj;

	// make sure the carcass is still here, if not throw an error and abort.
	obj = (OBJ_DATA *) ch->delay_info1;

	// is it really a carcass?
	if (GET_ITEM_TYPE(obj) != ITEM_CARCASS)
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (!obj)
	{
		// The carcass being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (CAN_SEE_OBJ (ch, obj))
	{
		send_to_char ("You strip viscera from the carcass, and continue flaying meat.\n", ch);
		act ("$n flays meat and strips viscera from $p.", false,
			ch, obj, 0, TO_ROOM | _ACT_FORMAT);

		ch->delay_type = DEL_BUTC_3;
		ch->delay = 12;
	}
	else
	{
		// The carcass being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see any carcass to butcher.\n", ch);
		return;
	}
}
void
	delayed_butcher3 (CHAR_DATA * ch)
{
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	OBJ_DATA *obj3;
	OBJ_DATA *carcass;
	OBJ_DATA *remains;
	int i = 1;
	bool failed = false;
	char buf[MAX_STRING_LENGTH];


	carcass = (OBJ_DATA *) ch->delay_info1;

	// is it really a CARCASS
	if (GET_ITEM_TYPE(carcass) != ITEM_CARCASS)
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (!carcass)
	{
		// The carcass being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The carcass you were butchering is no longer here.\n", ch);
		return;
	}

	if (!CAN_SEE_OBJ (ch, carcass))
	{
		// The carcass being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see the carcass you were butchering.\n", ch);
		return;
	}

	if (!carcass || sizeof (*carcass) < sizeof (OBJ_DATA))
	{
		send_to_char
			("There has been an error with the carcass. Aborting to avoid crash.\n", ch);
		return;
	}

	// First two ranks, easy to butcher.

	if (skill_level (ch, SKILL_BUTCHERY, -40))
	{
		if (carcass->o.od.value[0])
		{
			if (!(obj1 = load_object(carcass->o.od.value[0])))
			{
				send_to_char ("Problem...please contact an immortal.\n", ch);
				return;
			}
			else
			{

				// Give them a piece of meat for free, otherwise,
				// they roll their skill for each piece of meat.

				obj_to_room (obj1, ch->in_room);

				for (i = 1; i < carcass->o.od.value[1]; i++)
				{
					if (skill_use(ch, SKILL_BUTCHERY, -40))
						obj_to_room (load_object(carcass->o.od.value[0]), ch->in_room);
				}

				sprintf(buf, "You succeed in cutting #2%s#0 from the carcass.\n", obj_short_desc (obj1));
				send_to_char(buf, ch);
			}
		}
		else
			failed = true;
	}
	else
	{
		failed = true;
		send_to_char ("You demolished the carcass, failing to recover any meat from the bones.\n", ch);
		act ("$n demolishes a carcass.", false, ch, 0, 0, TO_ROOM);
	}

	if (!failed && skill_use(ch, SKILL_BUTCHERY, -20))
	{
		if (carcass->o.od.value[2])
		{
			if (!(obj2 = load_object(carcass->o.od.value[2])))
			{
				send_to_char ("Problem...please contact an immortal.\n", ch);
				return;
			}
			else
			{

				// Give them a piece of meat for free, otherwise,
				// they roll their skill for each piece of meat.

				obj_to_room (obj2, ch->in_room);

				for (i = 1; i < carcass->o.od.value[3]; i++)
				{
					if (skill_use(ch, SKILL_BUTCHERY, -20))
						obj_to_room (load_object(carcass->o.od.value[2]), ch->in_room);
				}

				sprintf(buf, "You prevail in recovering #2%s#0 from the bones.\n", obj_short_desc (obj2));
				send_to_char(buf, ch);
			}
		}
		else
			failed = true;
	}
	else
	{
		failed = true;
		send_to_char ("You fail to render anything else of use from the carcass.\n", ch);
		act ("$n cuts some meat from a carcass.", false, ch, 0, 0, TO_ROOM);
	}

	if (!failed && skill_use(ch, SKILL_BUTCHERY, 0))
	{
		if (carcass->o.od.value[4])
		{
			if (!(obj3 = load_object(carcass->o.od.value[4])))
			{
				send_to_char ("Problem...please contact an immortal.\n", ch);
				return;
			}
			else
			{

				// Give them a piece of meat for free, otherwise,
				// they roll their skill for each piece of meat.

				obj_to_room (obj3, ch->in_room);

				for (i = 1; i < carcass->o.od.value[5]; i++)
				{
					if (skill_use(ch, SKILL_BUTCHERY, 0))
						obj_to_room (load_object(carcass->o.od.value[4]), ch->in_room);
				}

				sprintf(buf, "With #2%s#0, you have stripped the carcass for all it has.\n", obj_short_desc (obj3));
				send_to_char(buf, ch);

			}
		}
		act ("$n strips $p for all the meat is has.", false, ch, carcass, 0, TO_ROOM);

	}
	else if (!failed)
	{
		send_to_char ("You fail to render anything else of use from the carcass.\n", ch);
		act ("$n cuts a good deal of meat from a carcass.", false, ch, 0, 0, TO_ROOM);
	}

	if (carcass->quality)
	{
		if (!(remains = load_object(carcass->quality)))
			;
		else
			obj_to_room(remains, ch->in_room);
	}


	ch->delay_type = 0;
	ch->delay = 0;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;

	extract_obj (carcass);
}


void
	do_skin (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj_corpse;
	char obj_name[MAX_INPUT_LENGTH];
	CHAR_DATA *tch;

	if (!((ch->right_hand && GET_ITEM_TYPE (ch->right_hand)
		== ITEM_WEAPON && (ch->right_hand->o.weapon.use_skill == SKILL_SMALL_BLADE))
		|| (ch->left_hand && GET_ITEM_TYPE (ch->left_hand)
		== ITEM_WEAPON && (ch->left_hand->o.weapon.use_skill == SKILL_SMALL_BLADE))))
	{

		send_to_char
			("You need to be holding some sort of knife to properly skin.\n",
			ch);
		return;
	}

	argument = one_argument (argument, obj_name);

	/* Allow the user to enter a corpse name in case he wants to
	say 2.corpse instead of just corpse */

	if (!*obj_name)
		strcpy (obj_name, "corpse");

	obj_corpse = get_obj_in_list_vis (ch, obj_name, ch->room->contents);

	if (!obj_corpse)
	{
		send_to_char ("You don't see a corpse here.\n", ch);
		return;
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->delay_info1 == (long int) obj_corpse)
		{
			send_to_char
				("Someone's already skinning that corpse at the moment.\n", ch);
			return;
		}
	}

	if (obj_corpse->nVirtual != VNUM_CORPSE
		&& !strstr (obj_corpse->name, "corpse"))
	{
		send_to_char ("You can only skin corpses.\n", ch);
		return;
	}

	if (!obj_corpse->o.od.value[2] && !obj_corpse->o.od.value[3])
	{
		send_to_char ("After a moment of poking, you realize that it isn't worth skinning.\n", ch);
		return;
	}

	ch->delay_info1 = (long int) obj_corpse;

	skill_learn(ch, SKILL_BUTCHERY);

	if (!real_skill (ch, SKILL_BUTCHERY))
		send_to_char ("You begin hacking away at the corpse.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 30)
		send_to_char ("You begin flailing at the corpse.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 45)
		send_to_char ("You begin slashing into the corpse.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 60)
		send_to_char ("You begin carving the corpse.\n", ch);
	else if (ch->skills[SKILL_BUTCHERY] < 75)
		send_to_char ("You begin operating on the corpse.\n", ch);
	else
		send_to_char ("You begin delicately skinning the corpse.\n", ch);

	sprintf (buf, "$n begins skinning #2%s#0.", obj_corpse->short_description);

	act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay_type = DEL_SKIN_1;
	ch->delay = 3;

	// Add time to the object timer so it doesn't decay while we're skinning it.  -Methuselah
	if ((obj_corpse->obj_timer - time(0)) < 1000)
		obj_corpse->obj_timer = time(0) + 1000;

	// Add time to the object->morphTime timer in case it's close to morphing.  -Methuselah
	if ((obj_corpse->morphTime - time(0)) < 1000)
		obj_corpse->morphTime = time(0) + 1000;
}

void
	delayed_skin_new1 (CHAR_DATA * ch)
{
	OBJ_DATA *obj_corpse;

	// make sure the corpse is still here, if not throw an error and abort.
	obj_corpse = (OBJ_DATA *) ch->delay_info1;

	// is it really a corpse?
	if (obj_corpse->nVirtual != VNUM_CORPSE
		&& !strstr (obj_corpse->name, "corpse"))
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (!obj_corpse)
	{
		// The corpse being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (CAN_SEE_OBJ (ch, obj_corpse))
	{
		send_to_char ("You start to cut into the corpse.\n", ch);
		act ("$n starts to cut into the corpse.", false, ch, 0, 0,
			TO_ROOM | _ACT_FORMAT);

		watched_action(ch, "skin a corpse.", 0, 0);

		ch->delay_type = DEL_SKIN_2;
		ch->delay = 7;
	}
	else
	{
		// Can't see the corpse anymore
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see any corpse to skin.\n", ch);
		return;
	}
}

void
	delayed_skin_new2 (CHAR_DATA * ch)
{
	OBJ_DATA *obj_corpse;

	// make sure the corpse is still here, if not throw an error and abort.
	obj_corpse = (OBJ_DATA *) ch->delay_info1;

	// is it really a corpse?
	if (obj_corpse->nVirtual != VNUM_CORPSE
		&& !strstr (obj_corpse->name, "corpse"))
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (!obj_corpse)
	{
		// The corpse being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (CAN_SEE_OBJ (ch, obj_corpse))
	{
		send_to_char ("You seem to be making progress as you dig into the corpse.\n", ch);
		act ("$n seems to be making progress as $e digs into the corpse.", false,
			ch, 0, 0, TO_ROOM | _ACT_FORMAT);

		ch->delay_type = DEL_SKIN_3;
		ch->delay = 10;
	}
	else
	{
		// The corpse being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see any corpse to skin.\n", ch);
		return;
	}
}

void
	delayed_skin_new3 (CHAR_DATA * ch)
{
	OBJ_DATA *skin = NULL;
	OBJ_DATA *corpse = NULL;
	OBJ_DATA *carcass = NULL;
	char buf[MAX_INPUT_LENGTH];
	char *p;

	corpse = (OBJ_DATA *) ch->delay_info1;

	// is it really a corpse?
	if (corpse->nVirtual != VNUM_CORPSE
		&& !strstr (corpse->name, "corpse"))
	{
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (!corpse)
	{
		// The corpse being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("The corpse you were skinning is no longer here.\n", ch);
		return;
	}

	if (!CAN_SEE_OBJ (ch, corpse))
	{
		// The corpse being skinned is gone, abort.
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay = 0;
		ch->delay_type = 0;
		send_to_char ("You can't see the corpse you were skinning.\n", ch);
		return;
	}

	if (!corpse || sizeof (*corpse) < sizeof (OBJ_DATA))
	{
		send_to_char
			("There has been an error with the corpse. Aborting to avoid crash.\n",
			ch);
		return;
	}

	// It's not difficult to skin an animal, hence the +50 bonus.
	// Real skill is in the butchery.

	if (skill_use (ch, SKILL_BUTCHERY, -50))
	{
		//A corpse that is WILL_SKIN has a negative o.od.value[2], See make-corpse() for details . We must adjust to get a vnum we can load?
		if (!(skin = LOAD_COLOR(corpse, corpse->o.od.value[2])))
		{
			if (!(skin = LOAD_COLOR(corpse, -corpse->o.od.value[2])))
			{
				send_to_char ("Problem...please contact an immortal.\n", ch);
				return;
			}
		}

		obj_to_room (skin, ch->in_room);

		// If the skin is a use_component, we set the number of uses to reflect the proper
		// surface area to volume ratio of our weight.

		if (GET_ITEM_TYPE(skin) == ITEM_COMPONENT)
		{
			double corpse_weight = corpse->obj_flags.weight;
			// Add 10% of the weight for a lost the head.
			// Yes, I know that 1 *.9 * 1.1 != 1, but
			// we'll assume some weight is lost forever in the beheading process.
			if (IS_SET (corpse->o.container.flags, CONT_BEHEADED))
			{
				corpse_weight = corpse_weight * 1.10;
			}
			double cube_inch_ratio = 0.0361270;
			double cubic_inch = corpse_weight / cube_inch_ratio;
			double square_inch = 6 * pow(cubic_inch, 2.0 / 3.0);
			double square_foot = square_inch / 144;

			square_foot = MAX(1, (int)square_foot);

			skin->o.od.value[0] = (int) square_foot;
		}

		strcpy (buf, skin->name);

		for (p = buf; *p && *p != ' ';)
		{
			p++;
		}

		*p = '\0';

		if (CAN_SEE_OBJ (ch, skin))
		{
			send_to_char ("You have successfully skinned the corpse.\n", ch);
			act ("$n has successfully skinned the corpse.", false, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);
			do_get (ch, buf, 0);
		}
		else
		{
			send_to_char ("You demolished the corpse.\n", ch);
			act ("$n demolishes a corpse.", false, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);
		}
	}
	else
	{
		send_to_char ("You demolished the corpse.\n", ch);
		act ("$n demolishes a corpse.", false, ch, 0, 0, TO_ROOM);
	}

	ch->delay_type = 0;
	ch->delay = 0;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;

	if (!(carcass = load_object (corpse->o.od.value[3])))
	{
		if (!(carcass = load_object (-corpse->o.od.value[3])))
		{
			extract_obj (corpse);
			return;
		}
	}

	// Lose 10% of the weight for the head.
	if (!IS_SET (corpse->o.container.flags, CONT_BEHEADED))
	{
		corpse->obj_flags.weight = corpse->obj_flags.weight * 0.90;
	}

	// Carcass becomes 85% of the weight
	carcass->obj_flags.weight = corpse->obj_flags.weight * 0.85;
	// Skin becomes 15% of the weight
	if (skin)
		skin->obj_flags.weight = corpse->obj_flags.weight * 0.15;

	extract_obj (corpse);

	obj_to_room (carcass, ch->in_room);
}

/*
Return an int which is the objnum of a plant suitable for the sector type
with some probability depending on rarity, and what the character was after.
*/

const int herbArray[HERB_NUMSECTORS * HERB_RARITIES][5] =
	/* Arrays for herbs by rsector and rarity by obj number */
	/* { #herbs, obj#'s } by rarity, from very rare to very common */

	// 7000-7009, fungii
	// 7010-7019, flowers
	// 7020-7029, berries
	// 7030-7039, weeds
	// 7040-7049, other

	// Each and every one of these has five possible things, for the five types of objects.
	// If you've specificed what you've sought, then you might just find it, otherwise,
	// it loads it up for you at random.

	/* Lab herb plants */
{
	{4, 10001, 5020, 0, 0},
	{4, 10000, 5020, 5048, 8023},
	{4, 5002, 8033, 8034, 6014},
	{4, 8000, 4018, 10014, 2025},
	{4, 10004, 5024, 5019, 5001},

	/* Office herb plants */
	{4, 8029, 5016, 2000, 1005},
	{4, 1001, 1004, 5006, 8025},
	{4, 5002, 8033, 8034, 6014},
	{4, 8000, 4018, 10014, 2025},
	{4, 10004, 5024, 5019, 5001},

	/* Apartment herb plants */
	{4, 3009, 3015, 3010, 3016},
	{4, 3000, 3003, 3500, 3009},
	{4, 5002, 8033, 8034, 8025},
	{4, 8000, 4018, 10014, 2025},
	{4, 10004, 5024, 5019, 5001},

	/* Factory herb plants */
	{4, 8004, 8025, 6014, 5048},
	{4, 5017, 8028, 8035, 8036},
	{4, 5002, 8033, 8034, 8013},
	{4, 8000, 4018, 10014, 2025},
	{4, 10004, 5024, 5019, 5005}
};





int
	GetHerbPlant (int sector_type, int pos, int rarity)
{

#define RUMM_VERY_RARE	5    // 5%
#define RUMM_RARE	15   // 10%
#define RUMM_UNCOMMON	35   // 20%
#define RUMM_COMMON	65   // 30%
#define RUMM_VCOMMON	100  // 35%

#define RUMM_MAX 100

	int i, objnum, rarityIndex;

	// So, it rolls a number, and decides what you get.

	i = number (1, RUMM_MAX);

	if (i <= RUMM_VERY_RARE)
		rarityIndex = 4;
	else if (i <= RUMM_RARE)
		rarityIndex = 3;
	else if (i <= RUMM_UNCOMMON)
		rarityIndex = 2;
	else if (i <= RUMM_COMMON)
		rarityIndex = 1;
	else				/* very common */
		rarityIndex = 0;

	if (rarity && pos)
	{
		rarity -= 1;
		objnum = foraged_object(sector_type, rarity, pos);
	}
	else
	{
		// If we don't find our object, it's because we've foraged
		// and gotten a rarity with nothing in it. So here goes...
		if (!(objnum = foraged_object(sector_type, rarityIndex, 0)))
		{
			// If we still get noting even after trying everything,
			// let us now.
			if (!(objnum = foraged_object(sector_type, 0, 0)) &&
				!(objnum = foraged_object(sector_type, 1, 0)) &&
				!(objnum = foraged_object(sector_type, 2, 0)) &&
				!(objnum = foraged_object(sector_type, 3, 0)) &&
				!(objnum = foraged_object(sector_type, 4, 0)))
			{
				send_to_gods("No foragables in anything!.");
				return objnum;
			}
		}
	}

	return objnum;

}

void
	do_rummage (CHAR_DATA * ch, char *argument, int cmd)
{
	int sector_type;
	char arg[MAX_STRING_LENGTH];
	char use[AVG_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA *obj = NULL;
	int j = 1;
	int k = 0;
	int seek = 0;
	bool said = false;

	skill_learn(ch, SKILL_FORAGE);

	if (!real_skill (ch, SKILL_FORAGE))
	{
		send_to_char ("You don't have any idea how to forage!\n\r", ch);
		return;
	}

	//send_to_char("Forage is temporarily disabled - likely will be working early next week.\n", ch);
	//return;

	if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !is_goggled(ch)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("It's too dark to forage.\n\r", ch);
		return;
	}

	if (is_sunlight_restricted (ch))
		return;

	sector_type = vnum_to_room (ch->in_room)->sector_type;

	/*
	if (sector_type == SECT_CITY || sector_type == SECT_INSIDE)
	{
	send_to_char ("You will not find any healthy plants growing here.\n",
	ch);
	return;
	}
	*/

	if (IS_SET(ch->room->room_flags, NO_LOG))
	{
		send_to_char ("You cannot forage here.\n", ch);
		return;
	}

	if (!(foraged_all(ch->room->sector_type)))
	{
		send_to_char("\nYou do not know of anything you could forage from this area.\n", ch);
		return;
	}

	argument = one_argument (argument, arg);

	if (*arg)
	{
		if (!str_cmp(arg, "survey"))
		{
			sprintf(buf, "You believe you could find the following items here...\n");

			for (k = 0; k < 5; k++)
			{
				said = false;

				sprintf (use, "Very Common");

				if (k == 1)
				{
					if (ch->skills[SKILL_FORAGE] >= 20)
						sprintf (use, "Common");
					else
						continue;
				}
				else if (k == 2)
				{
					if (ch->skills[SKILL_FORAGE] >= 40)
						sprintf (use, "Uncommon");
					else
						continue;
				}
				else if (k == 3)
				{
					if (ch->skills[SKILL_FORAGE] >= 60)
						sprintf (use, "Rare");
					else
						continue;
				}
				else if (k == 4)
				{
					if (ch->skills[SKILL_FORAGE] >= 80)
						sprintf (use, "Very Rare");
					else
						continue;
				}

				j = 1;

				std::vector<foraged_good*>::iterator it;
				for (it = foraged_goods_list.begin(); it != foraged_goods_list.end(); it++)
				{
					if ((*it)->sector != sector_type)
						continue;

					if ((*it)->rarity != k)
						continue;

					if ((obj = vtoo ((*it)->vnum)))
					{

						if (!said)
						{
							sprintf (buf + strlen(buf), "#2\n%s Items:#0\n", use);
							said = true;
						}

						sprintf (buf + strlen (buf), "   #2%d#0: %s\n", j, obj->short_description);

						j++;
					}
				}
			}
			send_to_char(buf, ch);
			return;
		}
		else if (!str_cmp(arg, "vcommon") || !str_cmp(arg, "vc"))
		{
			argument = one_argument(argument, use);
			if (isdigit(*use))
			{
				seek = atoi(use);
				ch->delay_info2 = 1;
			}
		}
		else if ((!str_cmp(arg, "common") || !str_cmp(arg, "c")) && ch->skills[SKILL_FORAGE] >= 20)
		{
			argument = one_argument(argument, use);
			if (isdigit(*use))
			{
				seek = atoi(use);
				ch->delay_info2 = 2;
			}
		}
		else if ((!str_cmp(arg, "uncommon") || !str_cmp(arg, "uc"))  && ch->skills[SKILL_FORAGE] >= 40)
		{
			argument = one_argument(argument, use);
			if (isdigit(*use))
			{
				seek = atoi(use);
				ch->delay_info2 = 3;
			}
		}
		else if ((!str_cmp(arg, "rare") || !str_cmp(arg, "r"))  && ch->skills[SKILL_FORAGE] >= 60)
		{
			argument = one_argument(argument, use);
			if (isdigit(*use))
			{
				seek = atoi(use);
				ch->delay_info2 = 4;
			}
		}
		else if ((!str_cmp(arg, "vrare") || !str_cmp(arg, "vr"))  && ch->skills[SKILL_FORAGE] >= 80)
		{
			argument = one_argument(argument, use);
			if (isdigit(*use))
			{
				seek = atoi(use);
				ch->delay_info2 = 5;
			}
		}
	}

	if (get_affect(ch, MAGIC_HIDDEN))
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		act ("Your actions have compromised your concealment.", true, ch, 0, 0, TO_CHAR);
	}

	send_to_char ("You begin searching the area for anything of use.\n\r", ch);
	act ("$n begins rummaging through the area.", true, ch, 0, 0,
		TO_ROOM);

	watched_action(ch, "rummaging through the area.", 0, 0);

	ch->delay_type = DEL_RUMMAGE;

	if (seek)
		ch->delay = 30;
	else
		ch->delay = 20;

	ch->delay_info1 = seek;
}


/*int reject_list[27] =
{05014, 5003, 5015, 5018, 5022, 5023, 6015, 6014,
6025, 8003, 8041, 8015, 1061, 5011, 8037, 5012,
5008, 53, 8039, 10008, 10009, 10005, 10006, 8027,
10012, 10003, 10010
};
*/
#define VNUM_SHROOM 2036
void
    delayed_rummage (CHAR_DATA * ch)
{
    OBJ_DATA *obj = NULL;
    int objnum;
    AFFECTED_TYPE *herbed;
    AFFECTED_TYPE *af = NULL;
    char arg[MAX_STRING_LENGTH];
    int error;


    for (herbed = ch->hour_affects; herbed; herbed = herbed->next)
    {
        if (herbed->type == AFFECT_FORAGED && herbed->a.spell.sn == ch->in_room)
        {
            send_to_char ("You've searched here too recently to have any chance of uncovering anything more.\n\r", ch);
            return;
        }
    }

    /*
    herbed = is_room_affected (ch->room->affects, HERBED_COUNT);

    if (herbed && (herbed->a.herbed.timesHerbed >= MAX_HERBS_PER_ROOM))
    {
    send_to_char ("This area has been stripped of anything useful for the time being.\n\r",
    ch);
    return;
    }
    */

    if (skill_use (ch, SKILL_FORAGE, -40 + (ch->delay_info2 * 20)))
    {
        objnum = GetHerbPlant (ch->room->sector_type, ch->delay_info1, ch->delay_info2);
        obj = load_object (objnum);



        if (obj)
        {
              if (GET_ITEM_TYPE(obj) == ITEM_CLUSTER)
	    {		
		  act ("Your rummaging has revealed $p.", false, ch, obj, 0, TO_CHAR);
                   obj_to_room (obj, ch->in_room);
             } 
           else if (obj->nVirtual == VNUM_SHROOM)
{
act ("Your rummaging has revealed $p.", false, ch, obj, 0, TO_CHAR);
                   obj_to_room (obj, ch->in_room);
}
else if (IS_NPC (ch))
	
	    {	
		obj_to_room (obj, ch->in_room);	
		  act ("$n's rummaging unconvers $p.", false, ch, obj, 0, TO_ROOM);
                    add_affect (ch, AFFECT_FORAGED, 30, 0, 0, 0, 0, ch->in_room);
             } 
	
else 
{
             obj_to_char (obj, ch);
            act ("Your rummaging has revealed $p.", false, ch, obj, 0, TO_CHAR);
        //    act ("$n's rummaging unconvers $p.", false, ch, obj, 0, TO_ROOM);
}
            add_affect (ch, AFFECT_FORAGED, 30, 0, 0, 0, 0, ch->in_room);

            /*
            if (!herbed)
            {
            herbed = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
            herbed->type = HERBED_COUNT;
            herbed->a.herbed.timesHerbed = 1;
            herbed->a.herbed.duration = HERB_RESET_DURATION;
            herbed->next = ch->room->affects;
            ch->room->affects = herbed;
            }
            else
            {
            herbed->a.herbed.timesHerbed++;
            herbed->a.herbed.duration = HERB_RESET_DURATION;
            }
            */
        }
        else
        {
            send_to_char("You successfully rummaged but there is naught to be found\n\r", ch);
            sprintf(arg, "Herbalism Object %d missing.", objnum);
            send_to_gods(arg);
        }
    }
    else
    {
        // If you fail, you have a +20% diff chance of getting something random.
        if (skill_use (ch, SKILL_FORAGE, 20))
        {

            objnum = GetHerbPlant (ch->room->sector_type, 0, 0);
            obj = load_object (objnum);
            if (obj)
            {
                obj_to_char (obj, ch);
                act ("Your rummaging does not reveal what you want, but you do find $p.", false, ch, obj, 0, TO_CHAR);
            //    act ("$n's rummaging unconvers $p.", false, ch, obj, 0, TO_ROOM);
            }
	if (IS_NPC (ch))
	
	    {		
		  act ("$n stops rummaging about.", true, ch, 0, 0, TO_ROOM);
             } 
            else
            {
                send_to_char ("Your rummaging efforts are to no avail.\n\r", ch);
            //    act ("$n stops rummaging about.", true, ch, 0, 0, TO_ROOM);
            }
        }
        else
        {
            send_to_char ("Your rummaging efforts are to no avail.\n\r", ch);
        //    act ("$n stops rummaging about.", true, ch, 0, 0, TO_ROOM);
        }

        add_affect (ch, AFFECT_FORAGED, 30, 0, 0, 0, 0, ch->in_room);
        //}
    }
     

   if (error == NO_HANDS_FULL)
        {
            act ("Your hands are full!", true, ch, 0, 0, TO_CHAR);
            return;
        }
        if (error == NO_TOO_HEAVY)
            {            
            act ("You can't carry so much weight.", true, ch, 0, 0, TO_CHAR);
            return;
            }
   
	// If this is a cluster object with the right setting, then we'll make it appear invsibile.
	if (obj)
	{
		if (GET_ITEM_TYPE(obj) == ITEM_CLUSTER && IS_SET(obj->o.od.value[1], CLUS_APPEAR_HIDDEN))
		{
			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

			af->type = MAGIC_HIDDEN;
			af->a.hidden.duration = -1;
			af->a.hidden.hidden_value = 50;
			af->a.hidden.coldload_id = ch->coldload_id;
			af->a.hidden.clan = 0;
			af->next = obj->xaffected;
			obj->xaffected = af;
		}
	}
}

void
	do_gather (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char arg[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA *obj = NULL;

	skill_learn(ch, SKILL_FORAGE);

	if (!real_skill (ch, SKILL_FORAGE))
	{
		send_to_char ("You need training in forgaging before you can gather from a cluster.\n", ch);
		return;
	}

	argument = one_argument (argument, arg);

	if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_CLUSTER)
	{
		act ("You cannot gather anything from $p.", true, ch, obj, 0, TO_CHAR);
		return;
	}

	if (obj->o.od.value[0] && IS_SET(obj->o.od.value[1], CLUS_ONE_SKILLED) && IS_SET(obj->o.od.value[1], CLUS_TWO_SKILLED) && (ch->skills[obj->o.od.value[0]] < 10))
	{
		sprintf(buf, "You require some knowledge of %s before you can attempt to gather from $p.", skills[obj->o.od.value[0]]);
		act (buf, true, ch, obj, 0, TO_CHAR);
		return;
	}

	if (get_affect(ch, MAGIC_HIDDEN))
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		act ("Your actions have compromised your concealment.", true, ch, 0, 0, TO_CHAR);
	}

	if (obj->ink_color && str_cmp(obj->ink_color, "(null)"))
	{
		sprintf(buf, "%s, you attempt to gather from $p.", obj->ink_color);
		sprintf(buf2, "%s, $n attempts to gather from $p.", obj->ink_color);
		*buf = toupper(*buf);
		*buf2 = toupper(*buf2);
	}
	else
	{
		sprintf(buf, "You attempt to gather from $p.");
		sprintf(buf2, "$n attempts to gather from $p.");
	}

	act (buf, true, ch, obj, 0, TO_CHAR);
	act (buf2, true, ch, obj, 0, TO_ROOM);

	ch->delay_type = DEL_GATHER;
	ch->delay_obj = obj;

	if (IS_SET(obj->o.od.value[1], CLUS_DELAY_LARGE))
		ch->delay = 20;
	else if (IS_SET(obj->o.od.value[1], CLUS_DELAY_SMALL))
		ch->delay = 10;
	else
		ch->delay = 15;
}

void
	delayed_gather (CHAR_DATA * ch)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj1 = NULL;
	OBJ_DATA *tobj2 = NULL;
	OBJ_DATA *junk = NULL;
	int i = 0;
	int skill_one = SKILL_FORAGE;
	int skill_two = SKILL_FORAGE;
	int roll_one = 0;
	int roll_two = 0;
	int count_one = 0;
	int count_two = 0;
	int check_one = 0;
	int check_two = 0;
	int amount_one = 0;
	int amount_two = 0;
	//POISON_DATA *poison = NULL;

	char buf_two[MAX_STRING_LENGTH] = { '\0' };
	char buf_one[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char trap_buf[MAX_STRING_LENGTH] = { '\0' };
	char trap_buf2[MAX_STRING_LENGTH] = { '\0' };

	bool passed = false;
	bool fail_one = false;
	bool fail_two = false;
	bool already = false;
	int result = 0; // 2 for eaten, 1 for junk, 0 for remains

	if (!(obj = ch->delay_obj) || !CAN_SEE_OBJ (ch, obj))
	{
		send_to_char ("The item you were gathering from is no longer here!\n", ch);
		return;
	}

	// Figure out what skill value we're using for each object

	if (obj->o.od.value[0])
	{
		if (IS_SET(obj->o.od.value[1], CLUS_ONE_SKILLED))
		{
			skill_one = obj->o.od.value[0];
		}

		if (IS_SET(obj->o.od.value[1], CLUS_TWO_SKILLED))
		{
			skill_two = obj->o.od.value[0];
		}
	}

	// Figure out what our two skill rolls are going to be.

	if (IS_SET(obj->o.od.value[1], CLUS_ONE_HIGH))
	{
		roll_one = 80;
		check_one = 40;
	}
	else if (IS_SET(obj->o.od.value[1], CLUS_ONE_LOW))
	{
		roll_one = 30;
		check_one = 15;
	}
	else
	{
		roll_one = 50;
		check_one = 25;
	}

	if (IS_SET(obj->o.od.value[1], CLUS_TWO_HIGH))
	{
		roll_two = 80;
		check_two = 40;
	}
	else if (IS_SET(obj->o.od.value[1], CLUS_TWO_LOW))
	{
		roll_two = 30;
		check_two = 15;
	}
	else
	{
		roll_two = 50;
		check_two = 25;
	}

	amount_one = obj->o.od.value[3];
	amount_two = obj->o.od.value[5];

	if (amount_one == 0 && amount_two == 0)
	{
		if (IS_SET(obj->o.od.value[1], CLUS_JUNK_PRODUCE) && obj->room_pos && vtoo(obj->room_pos))
		{
			if (IS_SET(obj->o.od.value[1], CLUS_JUNK_COLOR))
			{
				junk = LOAD_COLOR(obj, obj->room_pos);
			}
			else
			{
				junk = load_object(obj->room_pos);
			}

			obj_to_room(junk, ch->in_room);
			act ("You finish gathering from $p, but do not manage to find anything. Only $P remains.", true, ch, obj, junk, TO_CHAR | _ACT_FORMAT);
			act ("$n finishes gathering from $p, but does not manage to find anything. Only $P remains.", true, ch, obj, junk, TO_ROOM | _ACT_FORMAT);
			extract_obj(obj);
		}
		else
		{
			act ("You finish gathering from $p, but do not manage to find anything.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n finishes gathering from $p, but does not manage to find anything.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		return;
	}

	if (amount_one && obj->o.od.value[2] && vtoo(obj->o.od.value[2]))
	{
		// So, if the skill is forage, or we need a skill and we have it, then start rolling for objects...
		if ((skill_one == SKILL_FORAGE) || (ch->skills[skill_one] >= 10))
		{
			// Give them their freebie if they're guaranteed one.
			if (!IS_SET(obj->o.od.value[1], CLUS_NO_GUARANTEE))
			{
				if (IS_SET(obj->o.od.value[1], CLUS_ONE_COLOR))
				{
					tobj1 = LOAD_COLOR(obj, obj->o.od.value[2]);
					obj_to_room (tobj1, ch->in_room);
				}
				else
				{
					tobj1 = load_object(obj->o.od.value[2]);
					obj_to_room (tobj1, ch->in_room);
				}
				count_one += 1;
				obj->o.od.value[3] -= 1;
				passed = true;
			}
			for (i = (IS_SET(obj->o.od.value[1], CLUS_NO_GUARANTEE) ? 0 : 1); i < amount_one; i++)
			{
				if (skill_level(ch, skill_one, 0) >= number(1,roll_one))
				{
					if (IS_SET(obj->o.od.value[1], CLUS_ONE_COLOR))
					{
						tobj1 = LOAD_COLOR(obj, obj->o.od.value[2]);
						obj_to_room (tobj1, ch->in_room);
					}
					else
					{
						tobj1 = load_object(obj->o.od.value[2]);
						obj_to_room (tobj1, ch->in_room);
					}

					count_one += 1;
					passed = true;
					obj->o.od.value[3] -= 1;
				}
				else
				{
					fail_one = true;

					if (!IS_SET(obj->o.od.value[1], CLUS_ONE_KEEP))
					{
						obj->o.od.value[3] -= 1;
					}
				}
			}
			if (tobj1)
			{
				sprintf(buf_one, "#2%s#0", obj_short_desc(tobj1));
			}
		}
		// However, if the skill isn't forage, and we don't have that skill, and the object isn't set to keep,
		// then nice going genius - you just ruined it.
		else if (skill_one != SKILL_FORAGE && ch->skills[skill_one] <= 10 && !IS_SET(obj->o.od.value[1], CLUS_ONE_KEEP))
		{
			obj->o.od.value[3] = 0;
		}
	}

	if (amount_two && obj->o.od.value[4] && vtoo(obj->o.od.value[4]))
	{
		// So, if the skill is forage, or we need a skill and we have it, then start rolling for objects...
		if ((skill_one == SKILL_FORAGE) || (ch->skills[skill_two] >= 10))
		{
			// Give them their freebie if they're guaranteed one.
			if (!IS_SET(obj->o.od.value[1], CLUS_NO_GUARANTEE))
			{
				if (IS_SET(obj->o.od.value[1], CLUS_TWO_COLOR))
				{
					tobj2 = LOAD_COLOR(obj, obj->o.od.value[4]);
					obj_to_room (tobj2, ch->in_room);
				}
				else
				{
					tobj2 = load_object(obj->o.od.value[4]);
					obj_to_room (tobj2, ch->in_room);
				}
				count_two += 1;
				obj->o.od.value[5] -= 1;
				passed = true;
			}
			for (i = (IS_SET(obj->o.od.value[1], CLUS_NO_GUARANTEE) ? 0 : 1); i < amount_two; i++)
			{
				if (skill_level(ch, skill_two, 0) >= number(1,roll_two))
				{
					if (IS_SET(obj->o.od.value[1], CLUS_TWO_COLOR))
					{
						tobj2 = LOAD_COLOR(obj, obj->o.od.value[4]);
						obj_to_room (tobj2, ch->in_room);
					}
					else
					{
						tobj2 = load_object(obj->o.od.value[4]);
						obj_to_room (tobj2, ch->in_room);
					}

					count_two += 1;
					passed = true;
					obj->o.od.value[5] -= 1;
				}
				else
				{
					fail_two = true;

					if (!IS_SET(obj->o.od.value[1], CLUS_TWO_KEEP))
					{
						obj->o.od.value[5] -= 1;
					}
				}
			}

			if (tobj2)
			{
				sprintf(buf_two, "#2%s#0", obj_short_desc(tobj2));
			}

		}
		// However, if the skill isn't forage, and we don't have that skill, and the object isn't set to keep,
		// then nice going genius - you just ruined it.
		else if (skill_one != SKILL_FORAGE && ch->skills[skill_two] <= 10 && !IS_SET(obj->o.od.value[1], CLUS_TWO_KEEP))
		{
			obj->o.od.value[5] = 0;
		}
	}

	skill_use(ch, SKILL_FORAGE, 0);

	if (IS_SET(obj->o.od.value[1], CLUS_FAIL_TRAP) && (fail_one || fail_two) && obj->trap)
	{
		trap_component_rounds (ch, obj, 1, ch->room);

		if (obj->trap->trigger_1st && obj->trap->trigger_3rd)
		{
			sprintf (trap_buf, " In the process, you %s.", obj->trap->trigger_1st);
			sprintf (trap_buf2, " In the process, $n %s.", obj->trap->trigger_3rd);
		}
	}

	/*
	if (IS_SET(obj->o.od.value[1], CLUS_FAIL_SOMATIC) && (fail_one || fail_two) && obj->poison)
	{

	for (poison = obj->poison; poison; poison = poison->next)
	{
	soma_add_affect(ch, poison->poison_type, poison->duration, poison->latency,
	poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
	poison->attack, poison->decay, poison->sustain, poison->release);
	if (poison->uses > 0)
	{
	poison->uses -= 1;
	}
	}
	for (poison = obj->poison; poison; poison = poison->next)
	{
	if (poison->uses == 0)
	{
	remove_object_poison(obj, poison);
	}
	}

	}
	*/

	if (obj->o.od.value[3] == 0 && obj->o.od.value[5] == 0)
	{
		if (IS_SET(obj->o.od.value[1], CLUS_JUNK_PRODUCE) && obj->room_pos && vtoo(obj->room_pos))
		{
			result = 1;
		}
		else if (IS_SET(obj->o.od.value[1], CLUS_EMPTY_SEARCH))
		{
			result = 0;
		}
		else
		{
			result = 2;
		}
	}

	sprintf(buf, "You finish gathering from $p,");
	sprintf(buf2, "$n finishes gathering from $p,");

	if (!tobj1 && !tobj2)
	{
		if (IS_SET(obj->o.od.value[1], CLUS_JUNK_PRODUCE) && vtoo(obj->room_pos))
		{
			if (IS_SET(obj->o.od.value[1], CLUS_JUNK_COLOR))
			{
				junk = LOAD_COLOR(obj, obj->room_pos);
			}
			else
			{
				junk = load_object(obj->room_pos);
			}

			obj_to_room(junk, ch->in_room);
			act ("You finish gathering from $p, but do not manage to find anything. Only $P remains.", true, ch, obj, junk, TO_CHAR | _ACT_FORMAT);
			act ("$n finishes gathering from $p, but does not manage to find anything. Only $P remains.", true, ch, obj, junk, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("You finish gathering from $p, but do not manage to find anything.", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n finishes gathering from $p, but does not manage to find anything.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		if (result == 2)
		{
			extract_obj(obj);
		}
		return;
	}

	if (tobj1)
	{
		sprintf(buf + strlen(buf), " extracting %s%s", (count_one > 1 ? "several of " : ""), buf_one);
		sprintf(buf2 + strlen(buf2), " extracting %s%s", (count_one > 1 ? "several of " : ""), buf_one);
		already = true;
	}

	if (tobj2)
	{
		sprintf(buf + strlen(buf), "%s %s%s", (already ? ", and" : "extracting"), (count_two > 1 ? "several of " : ""), buf_two);
		sprintf(buf2 + strlen(buf2), "%s %s%s", (already ? ", and" : "extracting"), (count_two > 1 ? "several of " : ""), buf_two);
	}

	if (result == 1)
	{
		if (IS_SET(obj->o.od.value[1], CLUS_JUNK_COLOR))
		{
			junk = LOAD_COLOR(obj, obj->room_pos);
		}
		else
		{
			junk = load_object(obj->room_pos);
		}

		obj_to_room(junk, ch->in_room);

		sprintf(buf + strlen(buf), ". Only $P remains.");
		sprintf(buf2 + strlen(buf2), ". Only $P remains.");

		if (*trap_buf && *trap_buf2)
		{
			strcat(buf, trap_buf);
			strcat(buf2, trap_buf2);
		}

		act (buf, true, ch, obj, junk, TO_CHAR | _ACT_FORMAT);
		act (buf2, true, ch, obj, junk, TO_ROOM | _ACT_FORMAT);
		extract_obj(obj);
		return;
	}
	else
	{
		sprintf(buf + strlen(buf), ".");
		sprintf(buf2 + strlen(buf2), ".");

		if (*trap_buf && *trap_buf2)
		{
			strcat(buf, trap_buf);
			strcat(buf2, trap_buf2);
		}

		act (buf, true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		act (buf2, true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		if (result == 2)
		{
			extract_obj(obj);
		}
		return;
	}

}

void
	do_identify (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *plant;
	ROOM_DATA *room;

	skill_learn(ch, SKILL_FORAGE);

	if (!real_skill (ch, SKILL_BIOLOGY) && !real_skill (ch, SKILL_FORAGE))
	{
		send_to_char ("You need training in herbalism or foraging before you can identify "
			"flora.\n", ch);
		return;
	}

	room = ch->room;

	argument = one_argument (argument, buf);

	if (!(plant = get_obj_in_list_vis (ch, buf, room->contents)) &&
		!(plant = get_obj_in_list (buf, ch->right_hand)) &&
		!(plant = get_obj_in_list (buf, ch->left_hand)))
	{
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if (!(plant->obj_flags.type_flag == ITEM_CLUSTER
		|| (plant->obj_flags.type_flag == ITEM_FOOD && GET_MATERIAL_TYPE(plant) == 1 << 9)))
	{
		act ("$p isn't something you can identify: try and #6forage#0 for something first, or identify something gained by #6gather#0ing.",
			true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	act ("You begin to examine $p.", true, ch, plant, 0, TO_CHAR);
	act ("$n begins examining $p.", true, ch, plant, 0, TO_ROOM);

	ch->delay_type = DEL_IDENTIFY;
	ch->delay_who = str_dup (buf);
	ch->delay = 15 - (ch->skills[SKILL_FORAGE] / 10);
}

void
	delayed_identify (CHAR_DATA * ch)
{
	OBJ_DATA *plant;
	ROOM_DATA *room;
	char buf[MAX_STRING_LENGTH];

	room = ch->room;

	strcpy (buf, ch->delay_who);

	mem_free (ch->delay_who);
	ch->delay_who = NULL;

	if (!(plant = get_obj_in_list_vis (ch, buf, room->contents)) &&
		!(plant = get_obj_in_list (buf, ch->right_hand)) &&
		!(plant = get_obj_in_list (buf, ch->left_hand)))
	{
		send_to_char ("You failed to identify the contents of the plant before "
			"it was destroyed.\n", ch);
		return;
	}

	act ("$n finishes looking at $p.", true, ch, plant, 0, TO_ROOM);

	if (!plant->o.od.value[5] || !plant->o.od.value[4]
	|| (ch->skills[SKILL_BIOLOGY] < plant->o.od.value[4] / 2
		&& ch->skills[SKILL_FORAGE] < plant->o.od.value[4] / 2))
	{
		act ("You have no clue as to what $p might yield.",
			true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (ch->skills[SKILL_BIOLOGY] >= plant->o.od.value[4]
	|| ch->skills[SKILL_FORAGE] >= plant->o.od.value[4])
	{
		if (plant->obj_flags.type_flag == ITEM_CLUSTER)
		{
			switch (plant->o.od.value[5])
			{
			case 1:
				act ("You determine that $p will yield safe, edible food.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 2:
				act ("You determine that $p will yield food that whilst not safe to eat raw, may be edible once cooked.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 3:
				act ("You determine that $p will yield food dangerous to eat, but may have other uses.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 4:
				act ("You determine that $p will not yield anything edible but may have other uses.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			default:
				act ("You're unable to determine what $p will yield.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			}
		}
		else
		{
			switch (plant->o.od.value[3])
			{
			case 1:
				act ("You determine that $p is safe, edible food.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 2:
				act ("You determine that $p will be safe to eat once properly cooked or prepared.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 3:
				act ("You determine that $p is dangerous to eat, but can be prepared in other ways.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 4:
				act ("You determine that $p is not edible but may have other uses.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			default:
				act ("You unable to determine what use $p is.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			}
		}
	}
	else if (ch->skills[SKILL_BIOLOGY] >= plant->o.od.value[4] / 2
		|| ch->skills[SKILL_FORAGE] >= plant->o.od.value[4] / 2)
	{
		if (plant->obj_flags.type_flag == ITEM_CLUSTER)
		{
			switch (plant->o.od.value[5])
			{
			case 1:
			case 2:
			case 3:
				act ("You determine that $p will yield edible food.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 4:
				act ("You determine that $p will not yield anything edible but may have other uses.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			default:
				act ("You're unable to determine what $p will yield.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			}
		}
		else
		{
			switch (plant->o.od.value[3])
			{
			case 1:
			case 2:
			case 3:
				act ("You determine that $p is edible food.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			case 4:
				act ("You determine that $p is not edible but may have other uses.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			default:
				act ("You determine that $p will not yield anything of use.",
					true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
				break;
			}
		}
	}
	else
	{
		act ("You unable to determine what use $p is.",
			true, ch, plant, 0, TO_CHAR | _ACT_FORMAT);
	}
}
#define VNUM_CRATE 8137
void do_empty (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	OBJ_DATA *container;
	OBJ_DATA *target_container;
    bool into_other_container = false;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	//POISON_DATA *poison;

	argument = one_argument (argument, buf);

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch)
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		send_to_char ("That is not allowed in OOC areas.\n", ch);
		return;
	}

	if (!(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		send_to_char ("You don't have that object.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE( container ) == ITEM_CONTAINER )
	{
	    char container_name[ MAX_STRING_LENGTH ];
	    argument = one_argument( argument, container_name );

        if ( * container_name )
        {
            if ( !( target_container = get_obj_in_list_vis( ch, container_name, ch->room->contents ) ) )
            {
                send_to_char( "You do not see that on the ground.\n", ch );
                return;
            }

            if ( GET_ITEM_TYPE( target_container ) != ITEM_CONTAINER && container->nVirtual==VNUM_CRATE)
            {
                send_to_char( "How do you expect to empty anything into that?\n", ch );
                return;
            }

            into_other_container = true;
        }

        if ( into_other_container )
        {
            if ( container->nVirtual == VNUM_CORPSE )
            {
                send_to_char("You'll need to manually strip that corpse.\n", ch);
                return;
            } else if ( target_container->nVirtual == VNUM_CORPSE )
            {
                send_to_char("You'll have to manually dress that corpse up.\n",ch);
                return;
            }

            if ( container->contains ) {
                sprintf( buf, "You turn #2%s#0 upside down and start to empty it into $p:", container->short_description );
                act( buf, false, ch, target_container, 0, TO_CHAR );
                sprintf( buf, "$n turns #2%s#0 upside down and starts emptying it into $p:", container->short_description );
                act( buf, true, ch, target_container, 0, TO_ROOM );
            }

            int count = 0;
            char * error = new char[ 64 ];

            while ( container->contains ) {
                obj = container->contains;

                if ( !can_obj_to_container( obj, target_container, &error, 0 ) )
                {
                    send_to_char (error, ch);

                    act("You stop emptying $p.", false, ch, container, 0, TO_CHAR);
                    act("$n stops emptying $p.", true, ch, container, 0, TO_ROOM);

                    return;
                }

                obj_from_obj( &obj, 0 );
                obj_to_obj( obj, target_container );
                act( "    $p", false, ch, obj, 0, TO_CHAR );
                act( "    $p", true, ch, obj, 0, TO_ROOM );
            }

            act("You finish emptying $p.", false, ch, container, 0, TO_CHAR);
            act("$n finishes emptying $p.", true, ch, container, 0, TO_ROOM);
        } else {
            if (!spare_capacity(ch, obj, false))
            {
                send_to_char("This area is too small and already too full to contain that item.\n", ch);
                return;
            }

            if (container->contains)
            {
                act ("You turn $p upside down, spilling its contents:",
                    false, ch, container, 0, TO_CHAR);
                act ("$n turns $p upside down, spilling its contents:",
                    true, ch, container, 0, TO_ROOM);
            }

            while (container->contains)
            {
                obj = container->contains;
                obj_from_obj (&obj, 0);
                obj_to_room (obj, ch->in_room);
                act ("    $p", false, ch, obj, 0, TO_CHAR);
                act ("    $p", false, ch, obj, 0, TO_ROOM);
            }
        }

		container->contained_wt = 0;

		return;
	}

	/*
	if (GET_ITEM_TYPE (container) == ITEM_POTION)
	{
	act ("You spill the contents of $p on the ground.",
	false, ch, container, 0, TO_CHAR);
	act ("$n spills the contents of $p on the ground.",
	false, ch, container, 0, TO_ROOM);
	extract_obj (container);
	return;
	}
	*/

	if (GET_ITEM_TYPE (container) == ITEM_DRINKCON)
	{
		if (!container->contains)
		{
			act ("$o is already empty.", false, ch, container, 0, TO_CHAR);
			return;
		}

		act ("You spill the contents of $p on the ground.",
			false, ch, container, 0, TO_CHAR);
		act ("$n spills the contents of $p on the ground.",
			false, ch, container, 0, TO_ROOM);

		container->o.drinks.spell_1 = 0;
		container->o.drinks.spell_2 = 0;
		container->o.drinks.spell_3 = 0;

		while (container->contains)
		{
			extract_obj(container->contains);
		}

		//for (poison = container->poison; poison; poison = poison->next)
		//    remove_object_poison(container, container->poison);

		return;
	}

	if (GET_ITEM_TYPE (container) == ITEM_LIGHT &&
		!is_name_in_list ("candle", container->name))
	{

		if (!container->o.light.hours)
		{
			act ("$o is already empty.", false, ch, container, 0, TO_CHAR);
			return;
		}

		if (container->o.light.on)
			light (ch, container, false, true);

		sprintf (buf, "You empty %s from $p on the ground.",
			vnum_to_liquid_name (container->o.light.liquid));
		act (buf, false, ch, container, 0, TO_CHAR);

		sprintf (buf, "$n empties %s from $p on the ground.",
			vnum_to_liquid_name (container->o.light.liquid));
		act (buf, false, ch, container, 0, TO_ROOM);

		container->o.light.hours = 0;

		return;
	}

	act ("You can't figure out how to empty $p.",
		false, ch, container, 0, TO_CHAR);
}

void
	do_blindfold (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		send_to_char ("There's no such person here.\n", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("Wear a blindfold if you want to blindfold yourself.\n",
			ch);
		return;
	}

	if (get_equip (victim, WEAR_BLINDFOLD))
	{
		act ("$N is already blindfolded.", false, ch, 0, victim, TO_CHAR);
		return;
	}

	if (ch->right_hand
		&& IS_SET (ch->right_hand->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD))
		obj = ch->right_hand;

	else if (ch->left_hand
		&& IS_SET (ch->left_hand->obj_flags.wear_flags,
		ITEM_WEAR_BLINDFOLD))
		obj = ch->left_hand;

	if (!obj)
	{
		send_to_char ("You don't have a blindfold available.\n", ch);
		return;
	}

	if (!AWAKE (victim) && number (0, 4))
	{
		if (wakeup (victim))
		{
			act ("You've awoken $N.", false, ch, 0, victim, TO_CHAR);
			act ("$N wakes you up while trying to blindfold you!",
				false, ch, 0, victim, TO_VICT);
			act ("$n wakes $N up while trying to bindfold $M.",
				false, ch, 0, victim, TO_NOTVICT);
		}
	}

	if (!(!AWAKE (victim) ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) || IS_SUBDUEE (victim)))
	{
		act ("$N won't let you blindfold $M.", false, ch, 0, victim, TO_CHAR);
		return;
	}

	if (obj->carried_by)
		obj_from_char (&obj, 0);

	act ("$N blindfolds you!", true, victim, 0, ch, TO_CHAR);
	act ("You place $p over $N's eyes.", false, ch, obj, victim, TO_CHAR);
	act ("$n places $p over $N's eyes.", false, ch, obj, victim, TO_NOTVICT);

	equip_char (victim, obj, WEAR_BLINDFOLD);
}

/*

Create a head object.

Modify corpse object to be 'headless'.

*/

void
	do_behead (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *tool;
	OBJ_DATA *head;
	SCENT_DATA *scent = NULL;

	argument = one_argument (argument, buf);

	if (!(corpse = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		if (get_obj_in_list_vis (ch, buf, ch->right_hand)
			|| get_obj_in_list_vis (ch, buf, ch->left_hand))
			send_to_char ("Drop the corpse to behead it.\n", ch);
		else
			send_to_char ("You don't see that corpse here.\n", ch);
		return;
	}

	if (corpse->nVirtual != VNUM_CORPSE)
	{
		act ("$o is not a corpse.", false, ch, corpse, 0, TO_CHAR);
		return;
	}

	if (IS_SET (corpse->o.container.flags, CONT_BEHEADED))
	{
		act ("$p looks headless already.", false, ch, corpse, 0, TO_CHAR);
		return;
	}

	if (!(tool = get_equip (ch, WEAR_PRIM)) &&
		!(tool = get_equip (ch, WEAR_SEC)) &&
		!(tool = get_equip (ch, WEAR_BOTH)))
	{
		act ("You need to wield a sword, axe or knife to behead $p.",
			false, ch, corpse, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (tool->o.weapon.hit_type < 0 || tool->o.weapon.hit_type > 5)
	{
		sprintf (buf, "Your weapon, $p, vnum %d, doesn't have a valid hit_type "
			"set.  This is a bug.  Please petition and report it.  "
			"Thanks.", tool->nVirtual);
		act (buf, false, ch, tool, 0, TO_CHAR);
		sprintf (buf, "Weapon vnum %d has illegal hit type %d",
			tool->nVirtual, tool->o.weapon.hit_type);
		system_log (buf, true);
		return;
	}

	if (tool->o.weapon.hit_type == 1 ||	/* 1 = pierce */
		tool->o.weapon.hit_type == 3)
	{
		/* 3 = bludgeon */
		sprintf (buf2, "You can't %s the head off with $p.  Try a weapon "
			"that will slice or chop.",
			weapon_theme[tool->o.weapon.hit_type]);
		act (buf2, false, ch, tool, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	head = load_object (VNUM_HEAD);

	head->name = str_dup ("head");
	if (!strncmp (corpse->short_description, "the corpse of ", 14))
		strcpy (buf2, corpse->short_description + 14);
	else
		strcpy (buf2, "some unfortunate creature");

	sprintf (buf, "The head of %s rests here.", buf2);
	head->description = str_dup (buf);

	sprintf (buf, "the head of %s", buf2);
	head->short_description = str_dup (buf);

	sprintf (buf, "You %s the head from $p.",
		weapon_theme[tool->o.weapon.hit_type]);
	act (buf, false, ch, corpse, 0, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "$n cuts the head from $p.");
	act (buf, false, ch, corpse, 0, TO_ROOM | _ACT_FORMAT);

	mem_free (corpse->description);
	mem_free (corpse->short_description);

	sprintf (buf, "The headless corpse of %s is lying here.", buf2);
	corpse->description = str_dup (buf);

	sprintf (buf, "the headless corpse of %s", buf2);
	corpse->short_description = str_dup (buf);

	strcpy (buf2, corpse->name);

	mem_free (corpse->name);

	sprintf (buf, "headless %s", buf2);
	corpse->name = str_dup (buf);

	head->obj_flags.weight = corpse->obj_flags.weight / 10;
	corpse->obj_flags.weight -= head->obj_flags.weight;

	corpse->o.container.flags |= CONT_BEHEADED;

	head->obj_timer = corpse->obj_timer;

	if (corpse->o.od.value[4] == 11)
	{
		TOGGLE (head->econ_flags, 1 << 2);
		head->silver = 20;
	}

	if ((scent = get_scent(corpse, scent_lookup(("the putrid rot of decaying flesh")))))
	{
		add_scent(head, scent->scent_ref, scent->permanent, scent->atm_power, scent->pot_power, scent->rat_power, 1);
	}

	obj_to_room (head, ch->in_room);
}

void
	light (CHAR_DATA * ch, OBJ_DATA * obj, int on, int on_off_msg)
{
	/* Automatically correct any problems with on/off status */

	if (obj->o.light.hours <= 0)
		obj->o.light.on = 0;

	if (!on && !obj->o.light.on)
		return;

	if (on && obj->o.light.hours <= 0)
		return;

	if (on && obj->o.light.on)
		return;

	obj->o.light.on = on;

	if (on && get_affect (ch, MAGIC_HIDDEN))
	{
		if (would_reveal (ch))
			act ("You reveal yourself.", false, ch, 0, 0, TO_CHAR);
		else
			act ("The light will reveal your hiding place.",
			false, ch, 0, 0, TO_CHAR);

		remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if (on)
	{
		if (GET_ITEM_TYPE(obj) == ITEM_CHEM_SMOKE)
		{
			SCENT_DATA *scent;
			for (scent = obj->scent; scent; scent = scent->next)
			{
				if (scent->permanent && scent->pot_power == 0)
				{
					scent->pot_power = 100;
					scent->rat_power = obj->o.smoke.scent_ratio;
				}
			}
		}
		else
		{
			room_light (ch->room);	/* lighten before messages */
		}

		if (on_off_msg)
		{
			act ("You light $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n lights $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}

	}
	else
	{
		if (on_off_msg)
		{
			act ("You put out $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n puts out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}

		if (GET_ITEM_TYPE(obj) == ITEM_CHEM_SMOKE)
		{
			SCENT_DATA *scent;
			for (scent = obj->scent; scent; scent = scent->next)
			{
				if (scent->permanent && scent->pot_power == 100)
				{
					scent->pot_power = 0;
					scent->rat_power = 0;
				}
			}
		}
		else
		{
			room_light (ch->room);	/* darken after messages */
		}
	}
}

void
	do_light (CHAR_DATA * ch, char *argument, int cmd)
{
	int on = 1;
	int room_only = 0;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "room"))
	{
		room_only = 1;
		argument = one_argument (argument, buf);
	}

	if (!*buf)
	{
		send_to_char ("Light what?\n", ch);
		return;
	}

	if (room_only)
	{
		obj = get_obj_in_dark (ch, buf, ch->room->contents);
	}
	else
	{
		obj = get_obj_in_dark (ch, buf, ch->right_hand);
		if (!obj)
			obj = get_obj_in_dark (ch, buf, ch->left_hand);
		if (!obj && (obj = get_obj_in_dark (ch, buf, ch->equip))
			&& !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
		{
			act ("You can't light $p while you're still wearing it.\n", false,
				ch, obj, 0, TO_CHAR);
			return;
		}
	}

	if (!obj)
	{
		send_to_char ("You don't see that light source.\n", ch);
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_LIGHT && obj->obj_flags.type_flag != ITEM_E_LIGHT && obj->obj_flags.type_flag != ITEM_CHEM_SMOKE)
	{
		act ("You cannot light $p.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "off"))
		on = 0;

	if (on && SWIM_ONLY (ch->room) && !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
	{
		act ("You can't light $p while you're swimming.\n", false, ch, obj, 0,
			TO_CHAR);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_E_LIGHT)
	{
		modify_electric(ch, (on ? str_dup("on") : str_dup("off")), 0, obj);
		return;
	}
	else
	{
		if (!on && !obj->o.light.on)
		{
			act ("$p isn't lit.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		if (on && obj->o.light.hours <= 0)
		{
			act ("$p will no longer light.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		if (on && obj->o.light.on)
		{
			act ("$p is already lit.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		light (ch, obj, on, true);
	}
}

// Program allows for the packing of resource-type objects, to get around
// the fact that they're big and heavy and not the sort of thing anything
// other than a mule or a troll can conceivably move.

void
	do_pack (CHAR_DATA * ch, char *argument, int cmd)
{

	char arg[MAX_STRING_LENGTH] = "";
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	int count;
	int error = 0;

	argument = one_argument (argument, arg);

	if (just_a_number (arg))
	{
		count = atoi (arg);
		argument = one_argument (argument, arg);
	}

	if (!*arg)
	{
		send_to_char ("Pack what?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if (!CAN_WEAR (obj, ITEM_TAKE)
		|| IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED))
	{
		act ("$p cannot be lifted at all.", true, ch, obj, 0, TO_CHAR);
		return;
	}

	if (GET_ITEM_TYPE(obj) != ITEM_RESOURCE)
	{
		act("There is no need to pack $p. Use the 'put' command instead.", true, ch, obj, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, arg);

	if (!str_cmp (arg, "on") || !str_cmp (arg, "onto"))
		argument = one_argument (argument, arg);


	if (!(victim = get_char_room_vis (ch, arg)))
	{
		act ("Pack $p on to whom?", true, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!IS_SET (victim->act, ACT_PACKANIMAL))
	{
		act ("$N isn't capable of carrying things such as $p.", true, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!(can_obj_to_inv(obj, victim, &error, 1)) && error != NO_RESOURCE)
	{
		act ("$N isn't capable of carrying things such as $p.", true, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		return;
	}

	put_on_char (ch, victim, obj, 0, argument);
	return;
}

// modify <object> <options>
void do_modify (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char arg[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, arg);

	if (!*arg)
	{
		send_to_char ("Modify what?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->equip)))
	{
		send_to_char ("You do not see that item either in your hands, on your self, or in the room.\n", ch);
		return;
	}

	if (IS_ELECTRIC(obj))
		modify_electric(ch, argument, cmd, obj);
	else if (GET_ITEM_TYPE(obj) == ITEM_FIREARM)
		modify_firearm(ch, argument, cmd, obj);
	else
		act("You can't modify $p, only items of an electronic or firearm nature.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

	return;
}

void do_discard (CHAR_DATA *ch, char *argument, int cmd)
{
	char newArg[MAX_STRING_LENGTH] = { '\0' };
	sprintf(newArg, "deal hand card burn%s", argument);
	do_card (ch, newArg, cmd);
}

void do_deal (CHAR_DATA *ch, char *argument, int cmd)
{
	char newArg[MAX_STRING_LENGTH] = { '\0' };
	sprintf(newArg, "deal %s", argument);
	do_card (ch, newArg, cmd);
}

void do_shuffle (CHAR_DATA *ch, char *argument, int cmd)
{
	char newArg[MAX_STRING_LENGTH] = { '\0' };
	sprintf(newArg, "shuffle %s", argument);
	do_card (ch, newArg, cmd);
}

//void do_turn (CHAR_DATA *ch, char *argument, int cmd)
//{
//  char newArg[MAX_STRING_LENGTH] = { '\0' };
//  sprintf(newArg, "turn %s", argument);
//  do_card (ch, newArg, cmd);
//}

void do_card (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj = NULL;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };
	char buf6[MAX_STRING_LENGTH] = { '\0' };
	char arg[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char arg3[MAX_STRING_LENGTH] = { '\0' };
	char arg4[MAX_STRING_LENGTH] = { '\0' };
	char arg5[MAX_STRING_LENGTH] = { '\0' };
	char arg6[MAX_STRING_LENGTH] = { '\0' };
	int j = 0, i = 0;
	int exist [4][13];
	int trans [4][13];
	int count = 0;
	bool found = false;
	bool newCard = false;
	bool roomFound = false;
	bool tchFound = false;
	int missed = 1;
	int amount = 1;
	int style = -1;
	int dealt = 0;
	char preDeal[MAX_STRING_LENGTH] = {'\0'};
	bool specific = false;
	bool room = true;
	bool hands = true;
	bool shownValue = false;

	const char *deals[] = {"face-down", "privately", "face-up", "to be a deck", "to be burned"};
	char *su[] = {"H","D","C","S"};
	char *cu[] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
	int s = -1;
	int c = -1;
	int i_max = 0;
	int j_max = 0;
	int i_count = 0;
	int j_count = 0;
	bool j_found = false;


	for (j = 0; j <= 3; j++)
		for (i = 0; i <=12; i++)
			exist[j][i] = 0;

	for (j = 0; j <= 3; j++)
		for (i = 0; i <=12; i++)
			trans[j][i] = 0;

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if (!*arg2 || !*arg)
	{
		send_to_char("Cards syntax: deal|turn|shuffle <deck>\n"
			"              deal [room | hand] [#] <deck> <room, burn or character> [up|down|private] [suit of value] \n"
			"                or, 'deal' from 'room/hand' 'three' of 'card' to 'man', 'up'.\n"
			"                e.g. deal 3 card man private, will deal three private cards to the man\n"
			"                e.g. deal room 2 card room up, will deal two cards from the deck in the room to face up\n"
			"                e.g. deal hand card woman H 7, will deal the Seven of Hearts from your hand to the woman\n"
			"              discard [# of card] \n"
			"              shuffle <deck> \n"
			"              turn <deck> up|down|private|deck|burn\n", ch);
		return;
	}

	if (!str_cmp(arg2, "room"))
	{
		argument = one_argument(argument, arg2);
		hands = false;
	}
	else if (!str_cmp(arg2, "hand"))
	{
		argument = one_argument(argument, arg2);
		room = false;
	}

	if (isdigit(*arg2))
	{
		amount = atoi(arg2);
		argument = one_argument(argument, arg2);

		if (amount < 1)
			amount = 1;
	}

	if (!(obj = (hands ? get_obj_in_dark (ch, arg2, ch->right_hand) : NULL)) &&
		!(obj = (hands ? get_obj_in_dark (ch, arg2, ch->left_hand) : NULL)) &&
		!(obj = (room ? get_obj_in_list_vis (ch, arg2, ch->room->contents) : NULL)))
	{
		send_to_char ("You do not see that pile of cards either in your hands or in the room.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(obj) != ITEM_CARD)
	{
		send_to_char ("That's not a pile of cards.\n", ch);
		return;
	}

	for (j = 0; j <= 3; j++)
	{
		for (i = 0; i <=12; i++)
		{
			if (IS_SET(obj->o.od.value[j], 1 << i))
			{
				exist[j][i] = 1;
				count++;
			}
		}
	}

	// So we don't mix cards of different decks.
	if (obj->room_pos <= 0)
		obj->room_pos = obj->coldload_id;

	obj->o.od.value[5] = count;
	sprintf(preDeal, obj_short_desc(obj));

	if (!str_cmp(arg, "turn"))
	{

		if (obj != ch->right_hand && obj != ch->left_hand)
		{
			send_to_char("You need to be holding the cards you wish to turn.\n", ch);
			return;
		}

		argument = one_argument(argument, arg3);

		if (*arg3)
		{
			if (!str_cmp(arg3, "up") || !str_cmp(arg3, "u"))
				style = 2;
			else if (!str_cmp(arg3, "private") || !str_cmp(arg3, "p"))
				style = 1;
			else if (!str_cmp(arg3, "down") || !str_cmp(arg3, "d"))
				style = 0;
			else if (!str_cmp(arg3, "deck"))
				style = 3;
			else if (!str_cmp(arg3, "burn"))
				style = 4;
			else
			{
				send_to_char("You must either choose to turn the cards 'up', 'down', 'private', 'deck' or 'burn'.\n", ch);
				return;
			}
		}
		else
		{
			send_to_char("You must either choose to turn the cards 'up', 'down', 'private', 'deck' or 'burn'.\n", ch);
			return;
		}


		if (style == obj->o.od.value[4] && style != 2)
		{
			send_to_char("But those cards are already shown that way!\n", ch);
			return;
		}

		sprintf(buf, "You turn $p from #6%s#0 to #6%s#0.", deals[obj->o.od.value[4]], deals[style]);
		sprintf(buf2, "$n turns $p from #6%s#0 to #6%s#0.", deals[obj->o.od.value[4]], deals[style]);
		obj->o.od.value[4] = style;

		if (obj->o.od.value[5] <= 5 && style == 2)
		{
			if (obj->o.od.value[5] == 1)
				sprintf(buf5, "\n#6The card is");
			else
				sprintf(buf5, "\n#6The cards are");

			for (j = 0; j <= 3; j++)
			{
				j_found = false;
				for (i = 0; i <= 12; i++)
				{
					if (IS_SET (obj->o.od.value[j], (1 << i)))
					{
						j_found = true;
						break;
					}
				}
				if (j_found)
					j_max++;
			}

			for (j = 0; j <= 3; j++)
			{
				*buf6 = '\0';
				i_max = 0;
				i_count = 0;

				for (i = 0; i <= 12; i++)
					if (IS_SET (obj->o.od.value[j], (1 << i)))
						i_max ++;

				for (i = 0; i <= 12; i++)
				{
					if (IS_SET (obj->o.od.value[j], (1 << i)))
					{
						i_count++;
						if (i_count == i_max)
						{
							if (i_count == 1)
							{
								if (j_count && ((j_count + 1) == j_max))
									sprintf (buf6 + strlen (buf6), " and the #0%s#6 of #0%s#6", cards[i], suits[j]);
								else if (j_count)
									sprintf (buf6 + strlen (buf6), ", the #0%s#6 of #0%s#6", cards[i], suits[j]);
								else
									sprintf (buf6 + strlen (buf6), " the #0%s#6 of #0%s#6", cards[i], suits[j]);
							}
							else
								sprintf (buf6 + strlen (buf6), " and #0%s#6 of #0%s#6", cards[i], suits[j]);
						}
						else if (i_count == 1)
						{
							if (j_count)
								sprintf (buf6 + strlen (buf6), ", the #0%s#6", cards[i]);
							else
								sprintf (buf6 + strlen (buf6), " the #0%s#6", cards[i]);
						}
						else
							sprintf (buf6 + strlen (buf6), ", #0%s#6", cards[i]);
					}
				}

				if (*buf6)
				{
					j_count++;
					sprintf(buf5 + strlen(buf5), "%s", buf6);
				}
			}

			strcat (buf5, ".#0");
			strcat (buf, buf5);
			strcat (buf2, buf5);
		}

		act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		act(buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
	}
	else if (!str_cmp(arg, "shuffle"))
	{
		if (obj != ch->right_hand)
		{
			send_to_char("You need to be holding the first pile of cards you wish to combine in your right hand.\n", ch);
			return;
		}

		if (!(tobj = ch->left_hand) || GET_ITEM_TYPE(tobj) != ITEM_CARD)
		{
			send_to_char("You must be holding the second pile of cards you wish to combine in your left hand.\n", ch);
			return;
		}

		if (tobj->room_pos != obj->room_pos)
		{
			send_to_char("But these cards aren't from matching decks! You don't want to mix up your cards now, do you?.\n", ch);
			return;
		}

		act("You shuffle $P in to $p.", false, ch, obj, tobj, TO_CHAR | _ACT_FORMAT);
		act("$n shuffles $P in to $p.", false, ch, obj, tobj, TO_ROOM | _ACT_FORMAT);

		for (j = 0; j <= 3; j++)
		{
			for (i = 0; i <=12; i++)
			{
				if (IS_SET(tobj->o.od.value[j], 1 << i))
				{
					obj->o.od.value[j] += 1 << i;
					tobj->o.od.value[j] -= 1 << i;
					obj->o.od.value[5]++;
					tobj->o.od.value[5]--;
				}
			}
		}

		if (tobj->o.od.value[4] == 3)
			obj->o.od.value[4] = 3;

		extract_obj(tobj);
	}
	else if (!str_cmp(arg, "deal"))
	{
		argument = one_argument(argument, arg3);

		if (str_cmp(arg3, "burn"))
		{
			argument = one_argument(argument, arg4);
			if (*arg4)
			{
				if (!str_cmp(arg4, "up") || !str_cmp(arg4, "u"))
					style = 2;
				else if (!str_cmp(arg4, "private") || !str_cmp(arg4, "p"))
					style = 1;
				else if (!str_cmp(arg4, "down") || !str_cmp(arg4, "d"))
					style = 0;
				else
				{
					send_to_char("You must either choose to deal the cards 'down', 'private' or 'up'.\n", ch);
					return;
				}
			}
		}
		else
		{
			one_argument(argument, arg4);
			if (!str_cmp(arg4, "all") || !str_cmp(arg4, "hand"))
				amount = obj->o.od.value[5];
			else if (isdigit(*arg4))
			{
				amount = atoi(arg4);
				if (amount < 1)
					amount = 1;
			}
		}

		argument = one_argument(argument, arg5);
		argument = one_argument(argument, arg6);

		if ((*arg5 && !*arg6) || (*arg6 && !*arg5))
		{
			send_to_char("You need to specify both a suit and a number, e.g. H 7, C A, S K, D 2.\n", ch);
			return;
		}

		if (*arg5 && *arg6)
		{
			if (obj->o.od.value[5] == 0 || (obj->o.od.value[5] == 1 && (ch != obj->equiped_by && ch != obj->carried_by)))
			{
				send_to_char("You're unable to see those cards in order to deal a specific one from them.\n", ch);
				return;
			}

			for (int h = 0; h <= 3; h++)
			{
				if (!str_cmp(arg5, su[h]))
					s = h;
			}
			if (s > 3 || s < 0)
			{
				send_to_char("You need to nominate a suit: H, D, C or S\n", ch);
				return;
			}
			*arg5 += 1;
			for (int h = 0; h <=12; h++)
			{
				if (!str_cmp(arg6, cu[h]))
					c = h;
			}
			if (s > 12 || s < 0)
			{
				send_to_char("You need to nominate a card value: A, 2, 3, 4, 5, 6, 7, 8, 9, 10, J, Q or K.\n", ch);
				return;
			}

			if (!exist[s][c] || !IS_SET(obj->o.od.value[s], 1 << c))
			{
				send_to_char("That card isn't currently in that pile.\n", ch);
				return;
			}

			amount = 1;
			specific = true;
		}

		if (!str_cmp(arg3, "room") || !str_cmp(arg3, "burn"))
		{
			if (!str_cmp(arg3, "burn"))
				style = 4;

			if (style == -1)
				style = 2;

			for (tobj = ch->room->contents; tobj; tobj = tobj->next_content)
			{
				// If this isn't a card, continue;
				if (GET_ITEM_TYPE(tobj) != ITEM_CARD)
				{
					continue;
				}
				// If we're not of the same type, continue;
				if (tobj->room_pos != obj->room_pos)
				{
					continue;
				}
				// If we're not turned up the same way, continue.
				if (tobj->o.od.value[4] != style)
				{
					continue;
				}
				// Can't deal cards back in to a deck.
				if (tobj->o.od.value[4] == 3)
				{
					continue;
				}

				roomFound = true;
				break;
			}

			if (!tobj)
			{
				newCard = true;
				tobj = load_colored_object(998, obj->var_color[0], obj->var_color[1], obj->var_color[2], 0, 0, 0, 0, 0, 0, 0);
				tobj->o.od.value[0] = 0;
				tobj->o.od.value[1] = 0;
				tobj->o.od.value[2] = 0;
				tobj->o.od.value[3] = 0;
				tobj->o.od.value[4] = 0;
				tobj->o.od.value[5] = 0;
				tobj->room_pos = obj->room_pos;
				tobj->o.od.value[4] = style;
			}
		}
		else if ((tch = get_char_room_vis (ch, arg3)))
		{
			if (style == -1)
				style = 1;

			if (((tobj = tch->right_hand) && GET_ITEM_TYPE (tobj) == ITEM_CARD) ||
				((tobj = tch->left_hand) && GET_ITEM_TYPE (tobj) == ITEM_CARD))
				tchFound = true;
			else
			{
				tobj = load_colored_object(998, obj->var_color[0], obj->var_color[1], obj->var_color[2], 0, 0, 0, 0, 0, 0, 0);
				tobj->o.od.value[0] = 0;
				tobj->o.od.value[1] = 0;
				tobj->o.od.value[2] = 0;
				tobj->o.od.value[3] = 0;
				tobj->o.od.value[5] = 0;
				tobj->room_pos = obj->room_pos;
				tobj->o.od.value[4] = style;
			}
		}
		else
		{
			send_to_char("You either need to deal the cards to the room, to another character in the room, or to your self.\n", ch);
			return;
		}
		if (!tobj)
		{
			send_to_char("Couldn't find a set of new cards to create! Report this to Kithrater please.\n", ch);
			return;
		}
		else if (tobj == obj)
		{
			send_to_char("There's no point dealing the same cards in to itself!\n", ch);
			return;
		}
		if (tobj->room_pos != obj->room_pos)
		{
			send_to_char("But these cards aren't from matching decks! You don't want to mix up your cards now, do you?\n", ch);
			return;
		}

		if (tobj->o.od.value[4] == 3)
		{
			send_to_char("You can't deal cards in to a deck: you'll have to shuffle them back instead.\n", ch);
			return;
		}

		if (specific)
		{
			trans[s][c] = 1;
			obj->o.od.value[s] -= 1 << c;
			tobj->o.od.value[s] += 1 << c;
			obj->o.od.value[5]--;
			tobj->o.od.value[5]++;
			dealt = 1;
			if (obj->o.od.value[5] == 0)
				sprintf(buf4, "And that's all of the cards there are.");
		}
		else
		{
			for (int h = 1; h <= amount; h++)
			{
				found = false;
				missed = 1;
				for (j = 0; j <= 3; j++)
				{
					for (i = 0; i <=12; i++)
					{
						if (IS_SET(obj->o.od.value[j], 1 << i))
						{
							if (!number(0, (obj->o.od.value[5] - missed)))
							{
								obj->o.od.value[j] -= 1 << i;
								tobj->o.od.value[j] += 1 << i;
								trans[j][i] = 1;
								found = true;
								break;
							}
							missed++;
						}
					}
					if (found)
						break;
				}
				obj->o.od.value[5]--;
				tobj->o.od.value[5]++;

				dealt++;

				if (obj->o.od.value[5] == 0)
				{
					sprintf(buf4, "And that's all of the cards there are.");
					break;
				}
			}
		}

		if (dealt <= 5 && style == 2)
		{
			if (dealt == 1)
				sprintf(buf5, "\n#6The card is");
			else
				sprintf(buf5, "\n#6The cards are");

			for (j = 0; j <= 3; j++)
			{
				j_found = false;
				for (i = 0; i <= 12; i++)
				{
					if (trans[j][i])
					{
						j_found = true;
						break;
					}
				}
				if (j_found)
					j_max++;
			}

			for (j = 0; j <= 3; j++)
			{
				*buf6 = '\0';
				i_max = 0;
				i_count = 0;

				for (i = 0; i <= 12; i++)
					if (trans[j][i])
						i_max ++;

				for (i = 0; i <= 12; i++)
				{
					if (trans[j][i])
					{
						i_count++;
						if (i_count == i_max)
						{
							if (i_count == 1)
							{
								if (j_count && ((j_count + 1) == j_max))
									sprintf (buf6 + strlen (buf6), " and the #0%s#6 of #0%s#6", cards[i], suits[j]);
								else if (j_count)
									sprintf (buf6 + strlen (buf6), ", the #0%s#6 of #0%s#6", cards[i], suits[j]);
								else
									sprintf (buf6 + strlen (buf6), " the #0%s#6 of #0%s#6", cards[i], suits[j]);
							}
							else
								sprintf (buf6 + strlen (buf6), " and #0%s#6 of #0%s#6", cards[i], suits[j]);
						}
						else if (i_count == 1)
						{
							if (j_count)
								sprintf (buf6 + strlen (buf6), ", the #0%s#6", cards[i]);
							else
								sprintf (buf6 + strlen (buf6), " the #0%s#6", cards[i]);
						}
						else
							sprintf (buf6 + strlen (buf6), ", #0%s#6", cards[i]);
					}
				}

				if (*buf6)
				{
					j_count++;
					sprintf(buf5 + strlen(buf5), "%s", buf6);
				}
			}

			strcat (buf5, ".#0");
			shownValue = true;
		}

		if (newCard)
		{
			sprintf(buf, "You deal out #6%s#0 cards #6%s#0 from #2%s#0.", verbal_number[dealt], deals[style], preDeal);
			sprintf(buf2, "$n deals out #6%s#0 cards #6%s#0 from #2%s#0.", verbal_number[dealt], deals[style], preDeal);
			sprintf(buf3, "$n deals out #6%s#0 cards #6%s#0 from #2%s#0.", verbal_number[dealt], deals[style], preDeal);
			obj_to_room(tobj, ch->in_room);
		}
		else if (roomFound)
		{
			sprintf(buf, "You deal out #6%s#0 cards #6%s#0 from #2%s#0 in to #2%s#0.", verbal_number[dealt], deals[style], preDeal, obj_short_desc(tobj));
			sprintf(buf2, "$n deals out #6%s#0 cards #6%s#0 from #2%s#0 in to #2%s#0.", verbal_number[dealt], deals[style], preDeal, obj_short_desc(tobj));
			sprintf(buf3, "$n deals out #6%s#0 cards #6%s#0 from #2%s#0 in to #2%s#0.", verbal_number[dealt], deals[style], preDeal, obj_short_desc(tobj));
		}
		else if (tch)
		{
			if (ch == tch)
			{
				sprintf(buf, "You deal #6%s#0 cards #6%s#0 from #2%s#0 to #5yourself#0.", verbal_number[dealt], deals[style], preDeal);
				sprintf(buf2, "$n deals #6%s#0 cards #6%s#0 from #2%s#0 to #5%sself#0.", verbal_number[dealt], deals[style], preDeal, HMHR(ch));
				sprintf(buf3, "$n deals #6%s#0 cards #6%s#0 from #2%s#0 to #5yourself#0.", verbal_number[dealt], deals[style], preDeal);
			}
			else
			{
				sprintf(buf, "You deal #6%s#0 cards #6%s#0 from #2%s#0 to #5%s#0.", verbal_number[dealt], deals[style], preDeal, char_short(tch));
				sprintf(buf2, "$n deals #6%s#0 cards #6%s#0 from #2%s#0 to #5%s#0.", verbal_number[dealt], deals[style], preDeal, char_short(tch));
				sprintf(buf3, "$n deals #6%s#0 cards #6%s#0 from #2%s#0 to you.", verbal_number[dealt], deals[style], preDeal);
			}
			if (!tchFound)
				obj_to_char(tobj, tch);
		}

		if (shownValue)
		{
			strcat (buf, buf5);
			strcat (buf2, buf5);
			strcat (buf3, buf5);
		}


		act(buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		if (tch)
		{
			act(buf2, false, ch, 0, 0, TO_NOTVICT | _ACT_FORMAT);
			act(buf3, false, ch, 0, 0, TO_VICT | _ACT_FORMAT);
		}
		else
			act(buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);


		if (obj->o.od.value[5] <= 0)
		{
			send_to_char("And that's all of the cards there are in that pile.\n", ch);
			extract_obj(obj);
		}
	}
	else
	{
		send_to_char("Cards syntax: deal|turn|shuffle <deck>\n"
			"              deal [room | hand] <deck> [#] <room, burn or character> [up|down|private] [suit of value] \n"
			"              shuffle <deck> \n"
			"              turn <deck> up|down|private|deck|burn\n", ch);
		return;
	}
}


void
	do_decorate (CHAR_DATA * ch, char *argument, int cmd)
{
	char orig_argument[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[AVG_STRING_LENGTH * 4] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tool = NULL;
	OBJ_DATA *tool_leftover = NULL;
	int skill = 0;
	int size = 1;
	int short_swap = 0;
	char *p;
	bool refresh = false;

	if (!*argument)
	{
		send_to_char ("Syntax: decorate <object> [short] [small|large|tiny|entire] text of your decoration here\n", ch);
		return;
	}

	sprintf(orig_argument, "%s", argument);

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		send_to_char ("You don't see that item here.\n", ch);
		return;
	}

	if (obj == ch->right_hand)
	{
		if (!ch->left_hand || GET_ITEM_TYPE(ch->left_hand) != ITEM_TOOL)
		{
			send_to_char ("You need to be holding some kind of artistry tool in your other hand.\n", ch);
			return;
		}
		else
		{
			tool = ch->left_hand;
		}
	}
	else
	{
		if (!ch->right_hand || GET_ITEM_TYPE(ch->right_hand) != ITEM_TOOL)
		{
			send_to_char ("You need to be holding some kind of artistry tool in your other hand.\n", ch);
			return;
		}
		else
		{
			tool = ch->right_hand;
		}
	}

	if (!IS_SET(obj->obj_flags.extra_flags, ITEM_DECORATABLE))
	{
		send_to_char ("This item cannot be decorated - usually, only the end products of crafts can be decorated.\n", ch);
		return;
	}

	if (tool->o.od.value[5] && !IS_SET(tool->o.od.value[5], 1 << GET_MATERIAL_TYPE(obj)))
	{
		sprintf(buf, "$p is not able to decorate made from materials such as $P.");
		act (buf, false, ch, tool, obj, TO_CHAR | _ACT_FORMAT);
		return;
	}

	skill = ch->skills[SKILL_ARTISTRY];

	one_argument (argument, buf);

	if (!str_cmp(buf, "erase"))
	{
		if (obj->nVirtual == 5065)
		{
			send_to_char("That artwork is far too large to erase - you'll need to start all over gain.", ch);
			return;
		}

		if (obj->dec_desc && obj->dec_style)
		{
			if (ch->skills[SKILL_ARTISTRY] + 10 <= obj->dec_quality)
			{
				act ("You can't erase $p's decoration, as your talents are insufficient to remove the work without permanently damaging the item.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			act ("You remove $p's decoration.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

			mem_free(obj->dec_desc);
			obj->dec_desc = NULL;
			mem_free(obj->dec_style);
			obj->dec_style = NULL;
			obj->dec_short = 0;
			obj->dec_quality = 0;
			obj->dec_size = 0;
			obj->dec_condition = 0;
		}
		else
		{
			act ("You can't erase $p's decoration, because it currently has none!", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}
	else if (!str_cmp(buf, "refresh"))
	{
		if (obj->nVirtual == 5065)
		{
			send_to_char("That artwork is far too large to refresh - you'll need to start all over gain.", ch);
			return;
		}

		if (obj->dec_desc && obj->dec_style)
		{
			// Need to be within 10 points of their skill to repaint it.
			if (ch->skills[SKILL_ARTISTRY] + 10 <= obj->dec_quality)
			{
				act ("You can't refresh $p's decoration, as your talents are insufficient to replicate the work.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}
			else if (str_cmp(tool->ink_color, obj->dec_style))
			{
				act ("You can't refresh $p's decoration, as $P cannot replicate the current decorations.", false, ch, obj, tool, TO_CHAR | _ACT_FORMAT);
				return;
			}
			else
			{
				refresh = true;
			}
		}
		else
		{
			act ("You can't refresh $p's decoration, because it currently has none!", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}
	else if (obj->dec_desc && obj->dec_style && IS_MORTAL (ch))
	{
		send_to_char ("This item has already been decorated - if you wish to refresh an existing decoration, use the #6decorate <item> refresh#0 command.\n", ch);
		return;
	}
	else
	{
		if (GET_ITEM_TYPE(obj) == ITEM_ARTWORK)
		{
			short_swap = 2;
			size = 4;
			argument = one_argument (argument, buf);
		}
		else
		{
			if (!str_cmp(buf, "short"))
			{
				if (tool->o.od.value[2] > 0)
				{
					short_swap = 1;
					argument = one_argument (argument, buf);
					one_argument (argument, buf);
				}
				else
				{
					act ("$p does not allow you the option of appending a short description while decorating.", false, ch, tool, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
			}

			if (!str_cmp(buf, "small"))
			{
				size = 1;
				argument = one_argument (argument, buf);
			}
			else if (!str_cmp(buf, "large"))
			{
				size = 2;
				argument = one_argument (argument, buf);
			}
			else if (!str_cmp(buf, "tiny"))
			{
				size = 3;
				argument = one_argument (argument, buf);
			}
			if (!str_cmp(buf, "entire"))
			{
				size = 4;
				argument = one_argument (argument, buf);
			}
		}
	}

	if (tool->o.od.value[0] < (size == 3 ? 1 : size) && tool->o.od.value[0] != -1)
	{
		act ("$p doesn't have enough materials left to decorate with.", false, ch, tool, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	// Some tools require a bare minimum of skill to use.
	if (tool->o.od.value[3] && skill < tool->o.od.value[3])
	{
		act ("Your Artistry skill is not sufficiently developed to properly use $p.", false, ch, tool, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	// Some tools aren't good enough to let you do beyond a certain level of work.
	if (tool->o.od.value[4] && skill > tool->o.od.value[4])
	{
		skill = tool->o.od.value[4];
	}

	if (!*argument)
	{
		send_to_char ("How did you want to decorate your item?\n", ch);
		return;
	}

	if (GET_ITEM_TYPE(obj) == ITEM_ARTWORK)
	{
		if (strlen (argument) > AVG_STRING_LENGTH * 4)
		{
			sprintf (buf2, "There is a 1024-character limit on object decorations - your description is currently %u characters long.\n", strlen(argument));
			send_to_char (buf2, ch);
			return;
		}
	}
	else
	{
		if (strlen (argument) > AVG_STRING_LENGTH)
		{
			sprintf (buf2, "There is a 256-character limit on object decorations - your description is currently %u characters long.\n", strlen(argument));
			send_to_char (buf2, ch);
			return;
		}
	}

	if (argument[strlen(argument) - 1] == '!' || refresh)
	{
		if (get_affect (ch, MAGIC_CRAFT_DELAY) && IS_MORTAL (ch))
		{
			act	("Sorry, but your OOC delay timer is still in place. You'll receive a notification when it expires and you're free to decorate once more.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		// Cull the last two degrees of our argument for the "!" part.
		if (refresh)
		{
			sprintf(buf2, "%s", obj->dec_desc);
			size = obj->dec_size;
			short_swap = obj->dec_short ? 1 : 0;
		}
		else
		{
			for (size_t y = 0; y < strlen (argument) - 2; y++)
			{
				sprintf (buf2 + strlen (buf2), "%c", argument[y]);
			}
		}

		if (buf2[strlen(buf2) - 1] != '.')
			strcat (buf2, ".");

		obj->dec_desc = add_hash (buf2);
		obj->dec_style = add_hash (tool->ink_color);
		obj->dec_quality = skill;
		obj->dec_condition = skill >= 50 ? 0 : 1;
		obj->dec_size = size - 1;

		if (short_swap)
		{
			obj->dec_short = tool->o.od.value[2];
		}
		else
		{
			obj->dec_short = 0;
		}

		for (size_t y = 0; y < strlen (obj->full_description) - 1; y++)
		{
			sprintf (buf4 + strlen (buf4), "%c", obj->full_description[y]);
		}


		if (GET_ITEM_TYPE(obj) == ITEM_ARTWORK)
		{
			sprintf (buf3, "%s With %s skill and %s, this item has a %s and %s depiction of: %s", buf4,
				(skill >= 75 ? dec_skills[8] : skill >= 70 ? dec_skills[7] : skill >= 60 ? dec_skills[6] : skill >= 50 ? dec_skills[5] : skill >= 40 ? dec_skills[4] : skill >= 30 ? dec_skills[3] : skill >= 20 ? dec_skills[2] : skill >= 10 ? dec_skills[1] : dec_skills[0]),
				tool->ink_color, dec_size[size - 1], skill >= 50 ? dec_conditions[0] : dec_conditions[1], buf2);
		}
		else
		{
			sprintf (buf3, "%s With %s skill and %s, this item has a %s and %s decoration of: %s", buf4,
				(skill >= 75 ? dec_skills[8] : skill >= 70 ? dec_skills[7] : skill >= 60 ? dec_skills[6] : skill >= 50 ? dec_skills[5] : skill >= 40 ? dec_skills[4] : skill >= 30 ? dec_skills[3] : skill >= 20 ? dec_skills[2] : skill >= 10 ? dec_skills[1] : dec_skills[0]),
				tool->ink_color, dec_size[size - 1], skill >= 50 ? dec_conditions[0] : dec_conditions[1], buf2);
		}

		if (short_swap == 1)
		{
			sprintf(buf3 + strlen(buf3), "\n\nThe short description will also be as follows: #2%s#0", obj_short_desc(obj));
		}
		else if (short_swap == 2)
		{
			sprintf(buf5, "a %s-%s %s of %s",
				(skill >= 75 ? dec_short_skills[8] : skill >= 70 ? dec_short_skills[7] : skill >= 60 ? dec_short_skills[6] : skill >= 50 ? dec_short_skills[5] : skill >= 40 ? dec_short_skills[4] : skill >= 30 ? dec_short_skills[3] : skill >= 20 ? dec_short_skills[2] : skill >= 10 ? dec_short_skills[1] : dec_short_skills[0]),
				dec_short[obj->dec_short], fname(obj->name), buf);

			sprintf(buf3 + strlen(buf3), "\n\nThe short description will also be as follows: #2%s#0", buf5);

			if (obj->short_description)
				mem_free(obj->short_description);
			obj->short_description = add_hash(buf5);

			sprintf(buf5 + strlen(buf5), " is here.");
			*buf5 = toupper(*buf5);

			if (obj->description)
				mem_free(obj->description);

			obj->description = add_hash(buf5);

			sprintf(buf5, "%s %s %s %s %s",
				(skill >= 75 ? dec_skills[8] : skill >= 70 ? dec_skills[7] : skill >= 60 ? dec_skills[6] : skill >= 50 ? dec_skills[5] : skill >= 40 ? dec_skills[4] : skill >= 30 ? dec_skills[3] : skill >= 20 ? dec_skills[2] : skill >= 10 ? dec_skills[1] : dec_skills[0]),
				dec_short[obj->dec_short], fname(obj->name), buf, materials[GET_MATERIAL_TYPE(obj)]);

			if (obj->name)
				mem_free(obj->name);

			obj->name = add_hash(buf5);

			reformat_desc(obj->full_description, &obj->full_description);
		}

		reformat_string (buf3, &p);
		mem_free (p); //char*

		sprintf(buf, "Given your skill, tools, and settings specified, the full description of your decorated object will appear as follows:\n\n%s\n\n"
			"#9#1Warning: The decoration you submit must be consistent with the tool used and your Artistry skill level. Entering masterpiece level decorations when you have a novice level Artistry skill, or similar shenanigans, will result in the admins removing the item from the game permanently.#0\n", buf3);

		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		// If we have a finite number of uses for this tool, and we've use them all up, then game over.
		if (tool->o.od.value[0] > 0)
		{
			// Exception for tiny - only one usage.
			if (size == 3)
				size = 1;
			// We use up size usages of this thing.
			tool->o.od.value[0] -= size;

			if (tool->o.od.value[0] == 0)
			{
				if (vtoo(tool->o.od.value[1]) && (tool_leftover = load_object(tool->o.od.value[1])))
				{
					act ("You use up the last of $p, leaving $P.", false, ch, tool, tool_leftover, TO_CHAR | _ACT_FORMAT);
					act ("$n uses up the last of $p, leaving $P.", false, ch, tool, tool_leftover, TO_ROOM | _ACT_FORMAT);
					extract_obj(tool);
					obj_to_char(tool_leftover, ch);
				}
				else
				{
					act ("You use up the last of $p.", false, ch, tool, 0, TO_CHAR | _ACT_FORMAT);
					act ("$n uses up the last of $p.", false, ch, tool, 0, TO_ROOM | _ACT_FORMAT);
				}
			}
		}

		skill_use(ch, SKILL_ARTISTRY, 0);
		// Add a RL hour delay
		magic_add_affect (ch, MAGIC_CRAFT_DELAY, -1, (time (0) + 60 * 60), 0, 0, 0);
	}
	else
	{
		for (size_t y = 0; y < strlen (obj->full_description) - 1; y++)
		{
			sprintf (buf4 + strlen (buf4), "%c", obj->full_description[y]);
		}


		if (GET_ITEM_TYPE(obj) != ITEM_ARTWORK)
		{
			sprintf (buf3, "%s With #6%s skill#0 and #6%s#0, this item has a #6%s#0 and #6%s#0 decoration of: %s", buf4,
				(skill >= 75 ? dec_skills[8] : skill >= 70 ? dec_skills[7] : skill >= 60 ? dec_skills[6] : skill >= 50 ? dec_skills[5] : skill >= 40 ? dec_skills[4] : skill >= 30 ? dec_skills[3] : skill >= 20 ? dec_skills[2] : skill >= 10 ? dec_skills[1] : dec_skills[0]),
				tool->ink_color, dec_size[size - 1], skill >= 50 ? dec_conditions[0] : dec_conditions[1], argument);
		}
		else
		{
			sprintf (buf3, "%s With #6%s skill#0 and #6%s#0, this item has a #6%s#0 and #6%s#0 depiction of: %s", buf4,
				(skill >= 75 ? dec_skills[8] : skill >= 70 ? dec_skills[7] : skill >= 60 ? dec_skills[6] : skill >= 50 ? dec_skills[5] : skill >= 40 ? dec_skills[4] : skill >= 30 ? dec_skills[3] : skill >= 20 ? dec_skills[2] : skill >= 10 ? dec_skills[1] : dec_skills[0]),
				tool->ink_color, dec_size[size - 1], skill >= 50 ? dec_conditions[0] : dec_conditions[1], argument);
		}

		if (short_swap == 1)
		{
			obj->dec_short = tool->o.od.value[2];
			sprintf(buf3 + strlen(buf3), "\n\nThe short description will also be as follows: #2%s#0", obj_short_desc(obj));
			obj->dec_short = 0;
		}
		else if (short_swap == 2)
		{
			obj->dec_short = tool->o.od.value[2];
			sprintf(buf3 + strlen(buf3), "\n\nThe short description will also be as follows: #2a %s-%s %s of %s#0",
				(skill >= 75 ? dec_short_skills[8] : skill >= 70 ? dec_short_skills[7] : skill >= 60 ? dec_short_skills[6] : skill >= 50 ? dec_short_skills[5] : skill >= 40 ? dec_short_skills[4] : skill >= 30 ? dec_short_skills[3] : skill >= 20 ? dec_short_skills[2] : skill >= 10 ? dec_short_skills[1] : dec_short_skills[0]),
				dec_short[obj->dec_short], fname(obj->name), buf);
			obj->dec_short = 0;
		}

		reformat_string (buf3, &p);
		mem_free (p); //char*



		sprintf(buf, "Given your skill, tools, and settings specified, the full description of your decorated object will appear as follows:\n\n%s\n\n"
			"#6If you are happy with the way this looks, repeat your command with a ! at the end, i.e:\n#0"
			"#6decorate %s !\n\n#0"
			"#9#1Warning: The decoration you submit must be consistent with the tool used and your Artistry skill level. Entering masterpiece level decorations when you have a novice level Artistry skill, or similar shenanigans, will result in the admins removing the item from the game permanently.#0\n",
			buf3, orig_argument);

		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}




#define MAX_CLOTHING_ITEMS 24

const int clothes_to_qualitify[MAX_CLOTHING_ITEMS] =
{
	3026,
	3027,
	3028,
	3029,
	3030,
	3031,
	3032,
	3033,
	3034,
	3036,
	3037,
	3038,
	3039,
	3040,
	3041,
	3042,
	3047,
	3050,
	3057,
	3058,
	3071,
	3072,
	65002,
	1
};

const char *quality_design[5] =
{
	"It is raggedly cut and unhemmed",
	"It is crude and simple in its manufacture",
	"It is of average quality",
	"It is neatly hemmed and fitted",
	"It is superbly cut and tailored to fit precisely"
};

const char *quality_stitching[5] =
{
	" and is crudely looped-stitched and falling apart.",
	" and is roughly hand-stitched.",
	" and is plainly stitched.",
	" and is finely stitched.",
	" and it appears virtually seamless."
};

bool is_clothing_item(int nVirtual)
{
	for (int i = 0; i < MAX_CLOTHING_ITEMS; i++)
	{
		if (nVirtual == clothes_to_qualitify[i])
		{
			return true;
		}
	}

	return false;
}


void
	do_origins (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char arg[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	bool found = false;

	argument = one_argument (argument, arg);

	if (!*arg)
	{
		send_to_char ("Find the origin of what?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->equip)) &&
		!((obj = vtoo(atoi(arg))) && can_order(obj)))
	{
		send_to_char ("No such object can be searched for, or you do not see that item either in your hands, on your self, or in the room.\n", ch);
		return;
	}

	AFFECTED_TYPE *af = NULL;
	PHASE_DATA *phase = NULL;
	bool craft_break = false;
	char craft_buf[MAX_STRING_LENGTH] = { '\0' };

	// What crafts does it come from...
	for (int i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if ((af = get_affect (ch, i)))
		{
			for (int xind = 0; xind < MAX_ITEMS_PER_SUBCRAFT; xind++)
			{
				if (af->a.craft->subcraft->obj_items[xind] &&
				    (IS_SET (af->a.craft->subcraft->obj_items[xind]->flags, SUBCRAFT_PRODUCED)
					|| IS_SET (af->a.craft->subcraft->obj_items[xind]->flags, SUBCRAFT_GIVE)))
				{
					for (int zind = 0; zind < MAX_DEFAULT_ITEMS; zind++)
					{
						if (obj->nVirtual == af->a.craft->subcraft->obj_items[xind]->items[zind])
						{
							if (!*craft_buf)
							{
								sprintf(craft_buf, "\n#6From the following crafts#0:\n");
							}

							sprintf(craft_buf + strlen(craft_buf), "   - %s %s, of the %s craftset\n", af->a.craft->subcraft->command, af->a.craft->subcraft->subcraft_name, af->a.craft->subcraft->craft_name);
							craft_break = true;
						}
					}
				}

				if (craft_break)
					break;
			}

			craft_break = false;
		}
	}

	if (!*craft_buf)
	{
		SUBCRAFT_HEAD_DATA *craft, *tcraft;
		for (craft = crafts; craft; craft = craft->next)
		{
			for (int xind = 0; xind < MAX_ITEMS_PER_SUBCRAFT; xind++)
			{
				if (craft->obj_items[xind] &&
				    (IS_SET (craft->obj_items[xind]->flags, SUBCRAFT_PRODUCED)
					|| IS_SET (craft->obj_items[xind]->flags, SUBCRAFT_GIVE)))
				{
					for (int zind = 0; zind < MAX_DEFAULT_ITEMS; zind++)
					{
						if (obj->nVirtual == craft->obj_items[xind]->items[zind])
						{
							if (!*craft_buf)
							{
								sprintf(craft_buf, "\n#6From the following craftsets#0:\n");
							}

							sprintf(craft_buf + strlen(craft_buf), "   - %s\n", craft->craft_name);
							craft_break = true;
						}
					}
				}

				if (craft_break)
					break;
			}

			craft_break = false;
		}
	}

	OBJ_DATA *test_obj = NULL;
	char sample_buf[MAX_STRING_LENGTH] = { '\0' };

	for (test_obj = full_object_list; test_obj; test_obj = test_obj->lnext)
	{
		if (GET_ITEM_TYPE(test_obj) == ITEM_COMPONENT && test_obj->o.od.value[1] == obj->nVirtual)
		{
			if (!*sample_buf)
			{
				sprintf(sample_buf, "\n#6From sampling or progressing the follow objects#0:\n");
			}

			sprintf(sample_buf + strlen(sample_buf), "   - sampling #2%s#0\n", obj_short_desc(test_obj));
		}

		if (GET_ITEM_TYPE(test_obj) == ITEM_PROGRESS && test_obj->o.od.value[2] == obj->nVirtual)
		{
			if (!*sample_buf)
			{
				sprintf(sample_buf, "\n#6From sampling or progressing the follow objects#0:\n");
			}

			sprintf(sample_buf + strlen(sample_buf), "   - progressing #2%s#0\n", obj_short_desc(test_obj));
		}
	}

	std::vector<foraged_good*>::iterator it;
	char forage_buf[MAX_STRING_LENGTH] = { '\0' };
	for (it = foraged_goods_list.begin(); it != foraged_goods_list.end(); it++)
	{
		if ((*it)->vnum == obj->nVirtual)
		{
			if (!*forage_buf)
			{
				sprintf(forage_buf, "\n#6You might be able to find this item by foraging in the following places#0:\n");
			}

			sprintf(forage_buf + strlen(forage_buf), "   - %s%s\n", ((*it)->rarity > 2 ? "rarely " : (*it)->rarity < 2 ? "easily " : ""), real_sector_names[(*it)->sector]);
		}
	}
	char *p;

	if (*forage_buf || *sample_buf || *craft_buf)
	{
		sprintf(buf, "\nYou know of the following possible origins for #2%s#0:", obj_short_desc(obj));
		reformat_string (buf, &p);
		sprintf (buf, "%s", p);
		sprintf(buf + strlen(buf), "%s%s%s", craft_buf, sample_buf, forage_buf);
	}
	else
	{
		sprintf(buf, "\nYou have no idea as to the possible origins of #2%s#0.\n", obj_short_desc(obj));
		reformat_string (buf, &p);
		sprintf (buf, "%s", p);
	}

	send_to_char(buf, ch);

};

// Applies the quality keywords to an object's description if
// it's ovals are currently blank.
void clothing_qualitifier(OBJ_DATA *obj, CHAR_DATA *ch)
{
	if (!obj)
		return;

	if (!is_clothing_item(obj->nVirtual))
		return;

	int cloth_skill = 1;

	if (ch)
	{
		cloth_skill = (ch->skills[SKILL_HANDICRAFT])/20;
		cloth_skill = MAX(cloth_skill, 0);
		cloth_skill = MIN(cloth_skill, 4);
	}

	std::string newDesc, descAppend;

	obj->o.od.value[4] = cloth_skill;
	obj->o.od.value[5] = cloth_skill;

	descAppend = MAKE_STRING(quality_design[obj->o.od.value[4]]) + MAKE_STRING(quality_stitching[obj->o.od.value[5]]);
	newDesc = obj->full_description + descAppend;
	obj->full_description = add_hash(newDesc.c_str());
	reformat_desc(obj->full_description, &obj->full_description);

	return;
}


// Japheth's mem-fixes


void obj_data::deep_copy (OBJ_DATA *copy_from)
{
	memcpy(this, copy_from, sizeof(obj_data));

	char buf[MAX_INPUT_LENGTH];
	sprintf(buf, "Object Error - vnum: %d, in_room: %d", this->nVirtual, this->in_room);

	if (copy_from->short_description)
	{
		this->short_description = str_dup(copy_from->short_description);
	}
	if (copy_from->name)
	{
		this->name = str_dup(copy_from->name);
	}
	if (copy_from->description)
	{
		this->description = str_dup(copy_from->description);
	}
	if (copy_from->full_description)
	{
		this->full_description = str_dup(copy_from->full_description);
	}
	if (copy_from->omote_str)
	{
		this->omote_str = str_dup(copy_from->omote_str);
	}
	if (copy_from->attire)
	{
		this->attire = str_dup(copy_from->attire);
	}
	if (copy_from->ink_color)
	{
		this->ink_color = str_dup(copy_from->ink_color);
	}
	if (copy_from->desc_keys)
	{
		this->desc_keys = str_dup(copy_from->desc_keys);
	}
	for (int ind = 0; ind <= 9; ind++)
	{
		if (copy_from->var_color[ind])
			this->var_color[ind] = str_dup(copy_from->var_color[ind]);
		else
			this->var_color[ind] = str_dup("(null)");

		if (copy_from->var_cat[ind])
			this->var_cat[ind] = str_dup(copy_from->var_cat[ind]);
		else
			this->var_cat[ind] = str_dup("(null)");
	}

	if (copy_from->book_title)
	{
		this->book_title = str_dup(copy_from->book_title);
	}
	if (copy_from->indoor_desc)
	{
		this->indoor_desc = str_dup(copy_from->indoor_desc);
	}
	if (copy_from->creater)
	{
		this->creater = str_dup(copy_from->creater);
	}
	if (copy_from->editer)
	{
		this->editer = str_dup(copy_from->editer);
	}
	if (copy_from->ocues)
	{
		this->ocues = new std::multimap<obj_cue,std::string> (*copy_from->ocues);
	}

	if (this->nVirtual > 100000 || this->nVirtual < 0)
	{
		send_to_gods("ERROR! LET KITHRATER KNOW ASAP!");
		system_log(buf, true);
	}


	if (copy_from->dec_desc)
	{
		this->dec_desc = str_dup(copy_from->dec_desc);
	}
	if (copy_from->dec_style)
	{
		this->dec_style = str_dup(copy_from->dec_style);
	}

	mem_free(buf);

}


void obj_data::partial_deep_copy (OBJ_DATA *proto)
{
	//char buf[MAX_INPUT_LENGTH];
	//sprintf(buf, "Object Error - vnum: %d, in_room: %d", this->nVirtual, this->in_room);

	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		if (this->name)
		{
			mem_free(this->name);
		}
		this->name = str_dup(proto->name);
	}
	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		if (this->short_description)
		{
			mem_free(this->short_description);
		}
		this->short_description = str_dup(proto->short_description);
	}
	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		if (this->description)
		{
			mem_free(this->description);
		}
		this->description = str_dup(proto->description);
	}
	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		if (this->full_description)
		{
			mem_free(this->full_description);
		}
		this->full_description = str_dup(proto->full_description);
	}

	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
		this->obj_flags.extra_flags = proto->obj_flags.extra_flags;

	if (IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
		insert_string_variables (this, proto, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	else
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			this->var_color[ind] = "(null)";
			this->var_cat[ind] = "(null)";
		}
	}

	this->obj_flags.type_flag = proto->obj_flags.type_flag;

	this->obj_flags.wear_flags = proto->obj_flags.wear_flags;

	this->o.od.value[0] = proto->o.od.value[0];

	if (GET_ITEM_TYPE (this) != ITEM_KEY)
		this->o.od.value[1] = proto->o.od.value[1];

	this->o.od.value[2] = proto->o.od.value[2];
	this->o.od.value[3] = proto->o.od.value[3];
	this->o.od.value[4] = proto->o.od.value[4];
	this->o.od.value[5] = proto->o.od.value[5];

	this->quality = proto->quality;
	this->econ_flags = proto->econ_flags;

	this->obj_flags.weight = proto->obj_flags.weight;

	this->silver = proto->silver;
	this->farthings = proto->farthings;

	this->activation = proto->activation;

	if (!this->equiped_by)
		this->size = proto->size;

	this->obj_flags.extra_flags |= ITEM_NEWSKILLS;

	/*
	if (this->nVirtual > 100000 || this->nVirtual < 0)
	{
	send_to_gods("ERROR! LET KITHRATER KNOW ASAP!");
	system_log(buf, true);
	}

	mem_free(buf);
	*/



}
