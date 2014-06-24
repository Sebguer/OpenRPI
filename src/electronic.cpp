/*------------------------------------------------------------------------\
|  electronics.cpp : Electronics Module                  atonementrpi.com |
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
#include "constants.h"
#include "object_damage.h"

int is_electric(OBJ_DATA *obj)
{
	for (int i = 0; i < MAX_ELECTRIC_TYPES; i++)
	{
		if (GET_ITEM_TYPE(obj) == electric_list[i])
			return 1;
	}

	return 0;
}


// Pretty simple - water makes non-waterproof items unhappy.
int electrical_water_damage (CHAR_DATA *ch, OBJ_DATA *obj , int mode)
{
	if (!obj)
		return 0;

	if (obj->enviro_conds[COND_WATER] < 10)
		return 0;

	if (!IS_ELECTRIC(obj))
		return 0;

	if (!obj->o.elecs.status)
		return 0;

	if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_WATERPROOF))
		return 0;

	if (mode)
	{
		object__add_damage(obj, 6, number(5,20));
		object__add_damage(obj, 6, number(5,20));

		if (ch)
		{
			act ("$p fizzles and sparks violently, wisps of smoke emerging before sharply shocking your hand.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$p fizzles and sparks violently.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

			wound_to_char(ch, (obj == ch->right_hand ? add_hash("rhand") : add_hash("lhand")), number(3,6), 6, 0, 0, 0);
		}
	}
	else
	{
		object__add_damage(obj, 6, number(5,20));
		object__add_damage(obj, 6, number(5,20));

		if (ch)
		{
			act ("$p fizzles and sparks violently, wisps of smoke emerging before sharply shocking your hand.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$p fizzles and sparks violently.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}

	obj->o.elecs.status = 0;
	return 1;
}

void do_shine(CHAR_DATA *ch, char *argument, int mode)
{
	;
}

OBJ_DATA *robot_deconstructor (CHAR_DATA *robot, bool death)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj = NULL;

	// If we're not dealing with a robot, then get out of here.
	if (!robot || !IS_NPC(robot) || str_cmp(lookup_race_variable(robot->race, RACE_NAME), "robot"))
	{
		return NULL;
	}

	// If we don't have an object for our carcass or our skin, don't bother.
	if (!vtoo(robot->mob->carcass_vnum) || !vtoo(robot->mob->skinned_vnum))
	{
		return NULL;
	}

	char *color = '\0';
	char *botcover = '\0';
	char *botmobile = '\0';
	char *botshape = '\0';
	char *botfunction = '\0';
	char *botmodule1 = '\0';
	char *botmodule2 = '\0';
	char *botmodule3 = '\0';
	char *botmodule4 = '\0';

	int battery_left = 0;

	if (robot->d_hairlength)
		color = robot->d_hairlength;

	if (robot->d_age)
		botcover = robot->d_age;

	if (robot->d_height)
		botmobile = robot->d_height;

	if (robot->d_eyes)
		botshape = robot->d_eyes;

	if (robot->d_frame)
		botfunction = robot->d_frame;

	if (robot->d_feat1 && str_cmp(robot->d_feat1, "(null"))
		botmodule1 = robot->d_feat1;

	if (robot->d_feat2 && str_cmp(robot->d_feat2, "(null"))
		botmodule2 = robot->d_feat2;

	if (robot->d_feat3 && str_cmp(robot->d_feat3, "(null"))
		botmodule3 = robot->d_feat3;

	if (robot->d_feat4 && str_cmp(robot->d_feat4, "(null"))
		botmodule4 = robot->d_feat4;

	OBJ_DATA *remote_obj = NULL;
    for (remote_obj = object_list; remote_obj; remote_obj = remote_obj->next)
    {
        if (remote_obj->deleted)
		{
            continue;
		}
        if (remote_obj->coldload_id == robot->mob->controller)
		{
			remote_obj->o.od.value[3] = 0;
			remote_obj->o.od.value[4] = 0;
			break;
		}
    }

	// If we've been killed, then we get smashed in to little pieces - can't be put back together so easily.
	if (death)
	{
		int death_num = 0;

		switch (robot->mob->carcass_vnum)
		{
		case 6520:
			death_num = 6128;
			break;
		case 6521:
			death_num = 6051;
			break;
		case 6523:
			death_num = 6126;
			break;
		case 6524:
			death_num = 6127;
			break;
		default:
			death_num = 6125;
			break;
		}

		if (!(obj = load_colored_object(death_num, color, botcover, botmobile, botshape, botfunction, 0, 0, 0, 0, 0)))
		{
			return NULL;
		}

		return obj;
	}
	else
	{
		if (!(obj = load_colored_object(robot->mob->carcass_vnum, color, botcover, botmobile, botshape, botfunction, botmodule1, botmodule2, botmodule3, botmodule4, 0)))
		{
			return NULL;
		}
	}

	if (robot->mob->carcass_vnum == 6520)
	{
		battery_left = (robot->morph_time - time (0)) / 15 / 60;
	}
	else if (robot->mob->carcass_vnum == 6521)
	{
		battery_left = (robot->morph_time - time (0)) / 10 / 60;
	}
	else if (robot->mob->carcass_vnum == 6523)
	{
		battery_left = (robot->morph_time - time (0)) / 5 * 2 / 60;
	}
	else if (robot->mob->carcass_vnum == 6524)
	{
		battery_left = (robot->morph_time - time (0)) / 60;
	}
	else
	{
		battery_left = (robot->morph_time - time (0)) / 5 / 60;
	}

	if (!(tobj = load_object(robot->mob->skinned_vnum)))
		return NULL;

	tobj->o.battery.power = MAX(battery_left, 0);
	obj_to_obj(tobj, obj);

	WOUND_DATA *wound = NULL;
	int wound_value = 0;

    for (wound = robot->wounds; wound; wound = wound->next)
    {
		if (!str_cmp (wound->severity, "small"))
			wound_value = 1;
		else if (!str_cmp (wound->severity, "minor"))
			wound_value = 6;
		else if (!str_cmp (wound->severity, "moderate"))
			wound_value = 10;
		else if (!str_cmp (wound->severity, "severe"))
			wound_value = 19;
		else if (!str_cmp (wound->severity, "grievous"))
			wound_value = 29;
		else if (!str_cmp (wound->severity, "terrible"))
			wound_value = 39;
		else
			wound_value = 50;

		object__add_damage(obj, 3, wound_value);
    }

	do_drop (robot, "all", 1);

	return obj;
}

CHAR_DATA * robot_constructor(OBJ_DATA *robot)
{
	CHAR_DATA *tch = NULL;
	OBJ_DATA *tobj = NULL;
	bool miner = false;

    if (!(tobj = robot->contains))
		return NULL;

	for (int i = 0; i < 10; i++)
	{
		if (!str_cmp(robot->var_cat[i], "$botfunction"))
		{
			if (!str_cmp(robot->var_color[i], "scavenger"))
				tch = load_mobile(VNUM_BOT_SCAVENGER);
			else if (!str_cmp(robot->var_color[i], "peeper"))
				tch = load_mobile(VNUM_BOT_PEEPER);
			else if (!str_cmp(robot->var_color[i], "sentry"))
				tch = load_mobile(VNUM_BOT_SENTRY);
			else if (!str_cmp(robot->var_color[i], "mining"))
			{
				tch = load_mobile(VNUM_BOT_MINER);
			}
			else if (!str_cmp(robot->var_color[i], "hauling"))
			{
				tch = load_mobile(VNUM_BOT_HAULER);
				miner = true;
			}
			else if (!str_cmp(robot->var_color[i], "combatant"))
				tch = load_mobile(VNUM_BOT_COMBATANT);
			else
				tch = load_mobile(VNUM_BOT_DROID);

			tch->d_frame = str_dup(robot->var_color[i]);

			break;
		}
	}

	if (!tch)
	{
		return NULL;
	}

	for (int i = 0, j = 0; i < 10; i++)
	{
		if (!str_cmp(robot->var_cat[i], "$botmodule"))
		{
			switch (j)
			{
			case 0:
				tch->d_feat1 = str_dup(robot->var_color[i]);
				break;
			case 1:
				tch->d_feat2 = str_dup(robot->var_color[i]);
				break;
			case 2:
				tch->d_feat3 = str_dup(robot->var_color[i]);
				break;
			case 3:
				tch->d_feat4 = str_dup(robot->var_color[i]);
				break;
			}
			j++;
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$botcover"))
		{
			tch->d_age = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$color"))
		{
			tch->d_hairlength = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$finecolor"))
		{
			tch->d_hairlength = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$drabcolor"))
		{
			tch->d_hairlength = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$plasticcolor"))
		{
			tch->d_hairlength = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$botmobile"))
		{
			tch->d_height = str_dup(robot->var_color[i]);
			continue;
		}

		if (!str_cmp(robot->var_cat[i], "$botshape"))
		{
			tch->d_eyes = str_dup(robot->var_color[i]);
			continue;
		}
	}

	tch->description = add_hash(robot->full_description);
	tch->short_descr = add_hash(robot->short_description);
	tch->long_descr = add_hash(robot->description);
	tch->name = add_hash (robot->name);

	tch->mob->carcass_vnum = robot->nVirtual;
	tch->mob->skinned_vnum = tobj->nVirtual;

	// We get our armour value from oval2.
	tch->armor = robot->o.od.value[2];

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "recorder")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "recorder")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "recorder")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "recorder")))
	{
		tch->skills[SKILL_COMMON] = 50;
	}
	else
	{
		tch->skills[SKILL_COMMON] = 1;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "foraging")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "foraging")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "foraging")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "foraging")))
	{
		tch->skills[SKILL_FORAGE] = 50;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "stealth")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "stealth")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "stealth")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "stealth")))
	{
		tch->skills[SKILL_SNEAK] = 40;
		tch->skills[SKILL_HIDE] = 40;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "traps")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "traps")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "traps")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "traps")))
	{
		tch->skills[SKILL_HUNTING] = 50;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "combat")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "combat")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "combat")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "combat")))
	{
		tch->skills[SKILL_DEFLECT] += 40;
		tch->skills[SKILL_DODGE] += 40;
		tch->skills[SKILL_AIM] += 40;
		 tch->skills[SKILL_BRAWLING] += 40;
		tch->skills[SKILL_SMALL_BLADE] += 40;
		tch->skills[SKILL_LONG_BLADE] += 40;
		tch->skills[SKILL_POLEARM] += 40;
		tch->skills[SKILL_BLUDGEON] += 40;
		tch->skills[SKILL_SOLE_WIELD] += 40;
		tch->skills[SKILL_DUAL_WIELD] += 40;
		
	}
	
	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "combatant")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "combatant")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "combatant")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "combatant")))
	{
		tch->skills[SKILL_DEFLECT] += 40;
		tch->skills[SKILL_DODGE] += 40;
		tch->skills[SKILL_AIM] += 40;
		 tch->skills[SKILL_BRAWLING] += 40;
		tch->skills[SKILL_SMALL_BLADE] += 40;
		tch->skills[SKILL_LONG_BLADE] += 40;
		tch->skills[SKILL_POLEARM] += 40;
		tch->skills[SKILL_BLUDGEON] += 40;
		tch->skills[SKILL_SOLE_WIELD] += 40;
		tch->skills[SKILL_DUAL_WIELD] += 40;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "melee")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "melee")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "melee")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "melee")))
	{
	    tch->skills[SKILL_BRAWLING] += 40;
		tch->skills[SKILL_SMALL_BLADE] += 40;
		tch->skills[SKILL_LONG_BLADE] += 40;
		tch->skills[SKILL_POLEARM] += 40;
		tch->skills[SKILL_BLUDGEON] += 40;
		tch->skills[SKILL_SOLE_WIELD] += 40;
		tch->skills[SKILL_DUAL_WIELD] += 40;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "firearms")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "firearms")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "firearms")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "firearms")))
	{
	    tch->skills[SKILL_AIM] += 20;
		tch->skills[SKILL_HANDGUN] += 20;
		tch->skills[SKILL_SMG] += 20;
		tch->skills[SKILL_RIFLE] += 20;
		tch->skills[SKILL_GUNNERY] += 20;
	}

	if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "repair")) ||
		(tch->d_feat2 && !str_cmp(tch->d_feat2, "repair")) ||
		(tch->d_feat3 && !str_cmp(tch->d_feat3, "repair")) ||
		(tch->d_feat4 && !str_cmp(tch->d_feat4, "repair")))
	{
		tch->skills[ SKILL_MECHANICS ] = 70;
	}

	// We get our size, and our hitpoints, from oval3.
	switch (robot->o.od.value[3])
	{
	case 0:
		tch->height = 3;
		tch->size = -3;
		tch->frame = 3;
		tch->max_hit = 10;
		tch->str = 3;
		tch->tmp_str = 3;
		tch->con = 3;
		tch->tmp_con = 3;

        tch->mob->damnodice = 1;
        tch->mob->damsizedice = 1;
		tch->mob->damroll = 0;

		if (tch->skills[SKILL_SNEAK])
			tch->skills[SKILL_SNEAK] += 40;
		if (tch->skills[SKILL_HIDE])
			tch->skills[SKILL_HIDE] += 40;

		// 15 minutes per hour in battery for tiny robots.
		tch->morph_time = time (0) + tobj->o.battery.power * 15 * 60;
		tch->morphto = 86;
		soma_add_affect(tch, SOMA_NO_LARM, -1, 0, 0, 250, 250, 250, 666, 666, 666, 666);
		soma_add_affect(tch, SOMA_NO_RARM, -1, 0, 0, 250, 250, 250, 666, 666, 666, 666);

		break;
	case 1:
		tch->size = -1;
		tch->height = 36;
		tch->frame = 3;
		tch->max_hit = 36;
		tch->str = 7;
		tch->tmp_str = 7;
		tch->con = 7;
		tch->tmp_con = 7;

        tch->mob->damnodice = 2;
        tch->mob->damsizedice = 2;
		tch->mob->damroll = 0;

		if (tch->skills[SKILL_SNEAK])
			tch->skills[SKILL_SNEAK] += 20;
		if (tch->skills[SKILL_HIDE])
			tch->skills[SKILL_HIDE] += 20;

		// 10 minutes per hour in battery for small robots.
		tch->morph_time = time (0) + tobj->o.battery.power * 10 * 60;
		tch->morphto = 86;

		soma_add_affect(tch, SOMA_NO_LARM, -1, 0, 0, 250, 250, 250, 666, 666, 666, 666);
		soma_add_affect(tch, SOMA_NO_RARM, -1, 0, 0, 250, 250, 250, 666, 666, 666, 666);
		break;
	case 3:
		tch->size = 1;
		tch->height = 90;
		tch->frame = 3;
		tch->max_hit = 155;

		if (miner)
		{
			tch->str = 25;
			tch->tmp_str = 25;
		}
		else
		{
			tch->str = 25;
			tch->tmp_str = 25;
		}

		tch->con = 20;
		tch->tmp_con = 20;

		if (tch->skills[SKILL_SNEAK])
			tch->skills[SKILL_SNEAK] -= 20;
		if (tch->skills[SKILL_HIDE])
			tch->skills[SKILL_HIDE] -= 20;
	//encumbered = (GET_STR (ch) * enc_tab[0].str_mult_wt < IS_CARRYING_W (ch) - 250);
        tch->mob->damnodice = 1;
        tch->mob->damsizedice = 4;
		tch->mob->damroll = 2;

		// 2.5 minutes per hour in battery for big robots.
		tch->morph_time = time (0) + tobj->o.battery.power * 5 / 2 * 60;
		tch->morphto = 86;
		break;
	case 4:
		tch->size = 3;
		tch->height = 300;
		tch->frame = 3;
		tch->max_hit = 200;

		if (miner)
		{
			tch->str = 25;
			tch->tmp_str = 25;
		}
		else
		{
			tch->str = 19;
			tch->tmp_str = 19;
		}

		tch->con = 19;
		tch->tmp_con = 19;

		if (tch->skills[SKILL_SNEAK])
			tch->skills[SKILL_SNEAK] -= 40;
		if (tch->skills[SKILL_HIDE])
			tch->skills[SKILL_HIDE] -= 40;

        tch->mob->damnodice = 2;
        tch->mob->damsizedice = 6;
		tch->mob->damroll = 3;

		// 1 minutes per hour in battery for huge robots.
		tch->morph_time = time (0) + tobj->o.battery.power * 60;
		tch->morphto = 86;
		break;
	default:
		tch->size = 0;
		tch->height = 64;
		tch->frame = 3;
		tch->max_hit = 64;

		if (miner)
		{
			tch->str = 25;
			tch->tmp_str = 25;
		}
		else
		{
			tch->str = 11;
			tch->tmp_str = 11;
		}

		tch->con = 11;
		tch->tmp_con = 11;

        tch->mob->damnodice = 2;
        tch->mob->damsizedice = 5;
		tch->mob->damroll = 0;

		// 5 minutes per hour in battery for ordinary robots.
		tch->morph_time = time (0) + tobj->o.battery.power * 5 * 60;
		tch->morphto = 86;
		break;
	}


	OBJECT_DAMAGE *damage = NULL;
	double injury = 0.0;
	for (damage = robot->damage; damage; damage = damage->next)
	{

		switch(damage->severity)
		{
			case DAMAGE_MINOR:
				injury = 0.100;
				break;
			case DAMAGE_MODERATE:
				injury = 0.200;
				break;
			case DAMAGE_LARGE:
				injury = 0.300;
				break;
			case DAMAGE_HUGE:
				injury = 0.400;
				break;
			case DAMAGE_MASSIVE:
				injury = 0.500;
				break;
			default:
				injury = 0.019;
				break;
		}

		injury = injury * tch->max_hit;
		injury = MAX(injury, 1.0);

		if (wound_to_char(tch, "structure", injury, 13, 0, 0, 0))
		{
			extract_char(tch);
			return NULL;
		}
	}

    tch->act |= ACT_STAYPUT;
    save_stayput_mobiles ();

	return tch;
}

void robot_switch_on (CHAR_DATA *ch, OBJ_DATA *obj, int mode)
{
	CHAR_DATA *tch = NULL;
	ROOM_DATA *room = NULL;
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *remoteObj = NULL;

	// Make sure Mr Robot is in the room.
	if (!obj->in_room || !(room = vnum_to_room(obj->in_room)) || !(tobj = obj->contains))
	{
		send_to_char ("You need to have the robot, machine, droid or automation placed on the ground in the room to turn it on.\n", ch);
		return;
	}

	if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_E_REMOTE)
	{
		remoteObj = ch->right_hand;
	}
	else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_E_REMOTE)
	{
		remoteObj = ch->left_hand;
	}

	if (!remoteObj || !remoteObj->o.elecs.status || remoteObj->o.od.value[3] || remoteObj->o.od.value[4])
	{
		send_to_char ("You need to be holding a remote, with power, and not already synchronised to a robot.\n", ch);
		return;
	}

	if (!(tch = robot_constructor(obj)))
	{
		send_to_char ("Your robot is far too damage to be activated - be sure to repair it first!\n", ch);
		return;
	}

	char_to_room(tch, ch->in_room);
	remoteObj->o.od.value[3] = tch->coldload_id;
	tch->mob->controller = remoteObj->coldload_id;
	extract_obj(tobj);
	extract_obj(obj);

	act ("You point at $N and click a button on $p, the robot activating.", false, ch, remoteObj, tch, TO_CHAR | _ACT_FORMAT);
	act ("$n points $p at $N, the robot activating.", false, ch, remoteObj, tch, TO_ROOM | _ACT_FORMAT);
}

void electronic_hourly(OBJ_DATA *obj)
{
	OBJ_DATA *tobj = NULL;

	// If the status is zero or less than zero, no need to deduct anything.
	if (obj->o.elecs.status <= 0)
		return;

	// If we don't have a battery, or it isn't really a battery, return.
	if (!(tobj = obj->contains))
	{
		// If we don't have a battery, and we're not set to be powered internally, then someone has probably
		// removed the battery at some point and we didn't turn off automatically at that juncture. So, turn it off now!
		if (obj->o.elecs.status >= 1 && !IS_SET(obj->o.elecs.elec_bits, ELEC_PWR_INTERNAL))
			obj->o.elecs.status = 0;
		return;
	}

	if (GET_ITEM_TYPE(tobj) != ITEM_E_BATTERY)
		return;

	// Now, zap the battery power by the usage amount, or, by 1 if we have no other value set.
	// Exception are watches, which don't zap any power: they just need some power to work.
	if (tobj->o.battery.power > 0 && GET_ITEM_TYPE(obj) != ITEM_E_CLOCK)
		tobj->o.battery.power -= (obj->o.elecs.usage > 0 ? obj->o.elecs.usage : 1);

	// If our battery is now at zero, then we set the status to zero.
	if (tobj->o.battery.power <= 0)
	{
		obj->o.elecs.status = 0;
		tobj->o.battery.power = 0;
		// put in flicker out message

		if (GET_ITEM_TYPE(obj) == ITEM_E_REMOTE)
		{
			CHAR_DATA *tch;
			for (tch = character_list; tch; tch = tch->next)
			{
				if (tch->deleted)
					continue;
				if ((tch->coldload_id == obj->o.od.value[4]) && tch->controlling)
				{
					tch->controlling = 0;
				}
				else if ((tch->coldload_id == obj->o.od.value[3]) && tch->controlled_by)
				{
					tch->controlled_by = 0;
					if (tch->mob)
						tch->mob->controller = 0;
				}
			}

			obj->o.od.value[3] = 0;
			obj->o.od.value[4] = 0;
		}
	}
}

// modify <object> <options>
void modify_electric (CHAR_DATA *ch, char *argument, int cmd, OBJ_DATA *obj)
{
	OBJ_DATA *tobj = NULL;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char arg3[MAX_STRING_LENGTH] = { '\0' };
	int i = 0;
	int freq = 0;

	// If the object contains something, and that something is a battery, we have our tobj.

	if (obj->contains && (GET_ITEM_TYPE(obj->contains) == ITEM_E_BATTERY))
		tobj = obj->contains;

	argument = one_argument (argument, arg2);

	if (!*arg2)
	{
		act("How do you wish to modify $p? Enter ""modify <object> help"" for object-specific commands.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!str_cmp(arg2, "on"))
	{
		if (obj->o.elecs.status != 0)
		{
			act("$p is already switched on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else if (tobj)
		{
			if (tobj->o.battery.power > 0)
			{
				if (GET_ITEM_TYPE(obj) == ITEM_E_ROBOT)
				{
					robot_switch_on(ch, obj, 0);
					return;
				}
				else
				{
					obj->o.elecs.status = 1;
					act("You switch $p on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act("$n switches $p on.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

                    if ( IS_BOOK( obj ) )
                    {
                        if ( !obj->writing_loaded )
                            load_writing( obj );

                        obj->open = 1;
                    }

					electrical_water_damage(ch, obj, 1);
					return;
				}
			}

			else
			{
				act("You attempt to switch $p on, but nothing happens: its source of power is dead.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n attempts to switche $p on, but nothing happens.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
		}
		else
		{
			act("You attempt to switch $p on, but nothing happens: it is missing a source of power.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n attempts to switche $p on, but nothing happens.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			return;
		}
	}

	else if (!str_cmp(arg2, "off"))
	{
		if (obj->o.elecs.status < 0)
		{
			act("$p cannot be switched off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else if (obj->o.elecs.status > 0)
		{
			obj->o.elecs.status = 0;
			act("You switch $p off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n switches $p off.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

			if (GET_ITEM_TYPE(obj) == ITEM_E_REMOTE)
			{
				CHAR_DATA *tch;
				for (tch = character_list; tch; tch = tch->next)
				{
					if (tch->deleted)
						continue;
					if ((tch->coldload_id == obj->o.od.value[4]) && tch->controlling)
					{
						tch->controlling = 0;
					}
					else if ((tch->coldload_id == obj->o.od.value[3]) && tch->controlled_by)
					{
						tch->controlled_by = 0;
						if (tch->mob)
							tch->mob->controller = 0;
					}
				}

				obj->o.od.value[3] = 0;
				obj->o.od.value[4] = 0;
			} else if ( IS_BOOK( obj ) )
			{
                if ( obj->writing )
                    save_writing( obj );

                obj->open = 0;
			}

			return;
		}
		else
		{
			act("$p is already switched off.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}

	argument = one_argument (argument, arg3);

	if (obj->o.elecs.status == 0 && str_cmp(arg2, "help"))
	{
		act("You will need to switch $p on in order to modify its configuration.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	switch (GET_ITEM_TYPE(obj))
	{
	case ITEM_E_RADIO:
	case ITEM_E_BUG:
		if (!str_cmp(arg2, "help"))
		{
			sprintf(buf, "\n#6You can use the follow commands on this type of item:#0\n\n");
			sprintf(buf + strlen(buf), "#6On#0:           Switches the radio on.\n");
			sprintf(buf + strlen(buf), "#6Off#0:          Switches the radio off.\n");
			sprintf(buf + strlen(buf), "#6Channel X#0:    Changes the radio to a new channel, between 1 and 1000.\n");

			if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_ENCRYPT))
				sprintf(buf + strlen(buf), "#6Encrypt X#0:    Encrypts the radio's transmissions, for a value between 1000000 and 9999999. Enter 0 to switch off.\n");

			if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_SCAN))
				sprintf(buf + strlen(buf), "#6Scan up|down#0: Scans up or down for any recent radio activity within a hundred channels.\n");

			send_to_char(buf, ch);
			return;
		}

		if (!*arg3)
		{
			act("How do you wish to modify $p? Enter 'modify <object> help' for object-specific commands.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (!str_cmp(arg2, "channel"))
		{
			if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_SET))
			{
				act("You cannot change $p's transmission channel.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			if ((i = atoi(arg3)))
			{
				if (GET_ITEM_TYPE(obj) == ITEM_E_BUG)
				{
					if (i > 1000 || i < 100)
					{
						send_to_char("You must choose a channel between 100 and 1000.\n", ch);
						return;
					}
					else
					{
						obj->o.radio.channel = i;
						sprintf(buf, "You switch $p to now receive and transmit over channel %d.", i);
						sprintf(buf2, "$n modifies some settings on $p.");
						act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
						act(buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
						return;
					}
				}
				else
				{
					if (i > 1000 || i < 1)
					{
						send_to_char("You must choose a channel between 1 and 1000.\n", ch);
						return;
					}
					else
					{
						obj->o.radio.channel = i;
						sprintf(buf, "You switch $p to now receive and transmit over channel %d.", i);
						sprintf(buf2, "$n modifies some settings on $p.");
						act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
						act(buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
						return;
					}
				}
			}
			else
			{
				send_to_char("You must choose a channel between 1 and 1000.\n", ch);
				return;
			}
		}
		else if (!str_cmp(arg2, "encrypt"))
		{
			if (!IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_ENCRYPT))
			{
				act("$p isn't able to send encrypted transmissions.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			i = atoi(arg3);

			if (i == 0)
			{
				obj->o.radio.encrypt = 0;
				act("You switch $p to no longer encrypt its transmissions.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n modifies some settings on $p", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				act(buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
			else if (i > 9999999 || i < 1000000)
			{
				send_to_char("You must choose an encryption vaue between 1000000 and 9999999.\n", ch);
				return;
			}
			else
			{
				obj->o.radio.encrypt = i;
				sprintf(buf, "You switch $p to use the encryption code %d.", i);
				act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act("$n modifies some settings on $p", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				return;
			}
		}
		else if (!str_cmp(arg2, "scan"))
		{
			if (!IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_SCAN))
			{
				act("$p isn't able to scan for transmissions.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			if (!*arg3)
			{
				act("You need to specify whether you want to scan '#6up#0' '#6down#0' from your current channel.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			if (!str_cmp(arg3, "up"))
				i = 1;
			else if (!str_cmp(arg3, "down"))
				i = -1;
			else
			{
				act("You need to specify whether you want to scan '#6up#0' '#6down#0' from your current channel.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			for (int j = 1; j <= 100; j++)
			{
				freq = (j * i);
				freq = obj->o.radio.channel + freq;

				if (freq < 1 || freq > 999)
					break;

				if (frequency[freq])
				{
					sprintf(buf, "$p detects some recent radio activity on channel %d.", freq);
					act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
			}

			sprintf(buf, "$p does not detect any radio activity within a hundred channels %s from channel %d.", (i == 1 ? "up" : "down"), obj->o.radio.channel);
			act(buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			act("How do you wish to modify $p? Enter 'modify <object> help' for object-specific commands.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		break;
	case ITEM_E_BOOST:
		if (!str_cmp(arg2, "help"))
		{
			sprintf(buf, "\n#6You can use the follow commands on this type of item:#0\n\n");
			sprintf(buf + strlen(buf), "#6On#0:           Switches the booster on, delivering the associated skill gain/s.\n");
			sprintf(buf + strlen(buf), "#6Off#0:          Switches the booster off.\n");
			send_to_char(buf, ch);
			return;
		}
		break;
    case ITEM_E_BOOK:
        if (!str_cmp(arg2, "help"))
		{
			sprintf(buf, "\n#6You can use the follow commands on this type of item:#0\n\n");
			sprintf(buf + strlen(buf), "#6On#0:           Switches the book on.\n");
			sprintf(buf + strlen(buf), "#6Off#0:          Switches the book off.\n");
			send_to_char(buf, ch);
			return;
		}
    case ITEM_E_BREATHER:
        if (!str_cmp(arg2, "help"))
		{
			sprintf(buf, "\n#6You can use the follow commands on this type of item:#0\n\n");
			sprintf(buf + strlen(buf), "#6On#0:           Switches the breather on.\n");
			sprintf(buf + strlen(buf), "#6Off#0:          Switches the breather off.\n");
			send_to_char(buf, ch);
			return;
		}
	case ITEM_E_REMOTE:
		if (!str_cmp(arg2, "help"))
		{
			sprintf(buf, "\n#6You can use the follow commands on this type of item:#0\n\n");
			sprintf(buf + strlen(buf), "#6On#0:           Switches the remote on.\n");
			sprintf(buf + strlen(buf), "#6Off#0:          Switches the remote off.\n");
			sprintf(buf + strlen(buf), "#6Synch <targ>#0: Synchronises the remote to the target.\n");
			sprintf(buf + strlen(buf), "#6Synch clear#0:  Clears the remote's synchronisation.\n\n");
			sprintf(buf + strlen(buf), "#6Monitor#0:      Toggles monitoring the remote's camera.\n\n");
			sprintf(buf + strlen(buf), "#6command <remote> <order> will make the droid the remote is synchronised\n");
			sprintf(buf + strlen(buf), "to carry out the command, provided the droid is capable of that action.\n");
			sprintf(buf + strlen(buf), "To shut-off the robot, try #6command <remote> deactivate#0");
			send_to_char(buf, ch);
			return;
		}
		else if (!str_cmp(arg2, "synch"))
		{
			if (!str_cmp(arg3, "clear"))
			{
				act ("You clear $p's synchronisation.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n clears $p's synchronisation.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				for (tch = character_list; tch; tch = tch->next)
				{
					if (tch->deleted)
						continue;
					if ((tch->coldload_id == obj->o.od.value[4]) && tch->controlling)
					{
						tch->controlling = 0;
					}
					else if ((tch->coldload_id == obj->o.od.value[3]) && tch->controlled_by)
					{
						tch->controlled_by = 0;
						if (tch->mob)
							tch->mob->controller = 0;
					}
				}
				obj->o.od.value[3] = 0;
				obj->o.od.value[4] = 0;
				return;
			}
			else
			{
				if (!(tch = get_char_vis (ch, arg3)))
				{
					send_to_char ("You don't see that droid or robot here.\n", ch);
					return;
				}
				else if (!IS_NPC(tch))
				{
					send_to_char ("You can only synch non-player characters.\n", ch);
					return;
				}
				else if (IS_NPC(ch))
				{
					send_to_char ("It is too dangerous to attempt a synchronisation as a robot.\n", ch);
					return;
				}
				else if (tch->race != lookup_race_id("robot"))
				{
					send_to_char ("You can only synchronise remotes to droids or robots.\n", ch);
					return;
				}
				else if (!tch->mob || tch->descr())
				{
					send_to_char ("Something else is already synchronised or controlling that droid or robot.\n", ch);
					return;
				}
				else if (tch->mob->controller && tch->mob->controller != obj->coldload_id)
				{
					send_to_char ("Something else is already synchronised or controlling that droid or robot2.\n", ch);
					return;
				}
				// else, if it's not on
				else
				{
					act ("You synchronise $p to $N.", false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
					act ("$n synchronises $p to $N.", false, ch, obj, tch, TO_ROOM | _ACT_FORMAT);
					tch->mob->controller = obj->coldload_id;
					obj->o.od.value[3] = tch->coldload_id;
				}
			}
		}
		else if (!str_cmp(arg2, "monitor"))
		{
			// We don't have an oval3, or we can't find our mobile we're meant to be controlling.
			if (!obj->o.od.value[3] || !get_char_id(obj->o.od.value[3]))
			{
				send_to_char ("You need to synchronise that remote to an active droid or robot first.\n", ch);
				for (tch = character_list; tch; tch = tch->next)
				{
					if (tch->deleted)
						continue;
					if ((tch->coldload_id == obj->o.od.value[4]) && tch->controlling)
					{
						tch->controlling = 0;
					}
					else if ((tch->coldload_id == obj->o.od.value[3]) && tch->controlled_by)
					{
						tch->controlled_by = 0;
						if (tch->mob)
							tch->mob->controller = 0;
					}
				}
				obj->o.od.value[3] = 0;
				obj->o.od.value[4] = 0;
				return;
			}
			// We do have a mobile, but someone not us is already controlling it.
			else if (obj->o.od.value[4] && get_char_id(obj->o.od.value[4]) && obj->o.od.value[4] != ch->coldload_id)
			{
				send_to_char ("Someone else is already monitoring that remote.\n", ch);
				return;
			}
			// We're toggling it - need to strip out all controllers.
			else if (obj->o.od.value[4] && obj->o.od.value[4] == ch->coldload_id)
			{
				act ("You stop monitoring $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n stops monitoring $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				for (tch = character_list; tch; tch = tch->next)
				{
					if (tch->deleted)
						continue;
					if ((tch->coldload_id == obj->o.od.value[4]) && tch->controlling)
					{
						tch->controlling = 0;
					}
					else if ((tch->coldload_id == obj->o.od.value[3]) && tch->controlled_by)
					{
						tch->controlled_by = 0;
						if (tch->mob)
							tch->mob->controller = 0;
					}
				}
				obj->o.od.value[4] = 0;
			}
			// Now, we monitor it.
			else if (obj->o.od.value[3] && (tch = get_char_id(obj->o.od.value[3])))
			{

				if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "recorder")) ||
					(tch->d_feat2 && !str_cmp(tch->d_feat2, "recorder")) ||
					(tch->d_feat3 && !str_cmp(tch->d_feat3, "recorder")) ||
					(tch->d_feat4 && !str_cmp(tch->d_feat4, "recorder")))
				{
					act ("You start monitoring $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					act ("$n starts monitoring $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
					ch->controlling = tch;
					tch->controlled_by = ch;
					tch->mob->controller = obj->coldload_id;
					obj->o.od.value[4] = ch->coldload_id;
				}
				else
				{
					act ("The droid synchronised to $p does not have that capability.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				}
				return;
			}
			else
			{
				send_to_char ("An error occured with monitor - please let an admin know.\n", ch);
				return;
			}
		}
		break;
	}
}

void do_deactivate (CHAR_DATA *ch, char *argument, int cmd)
{
	if (str_cmp(lookup_race_variable(ch->race, RACE_NAME), "robot"))
	{
		send_to_char("Only robots can deactivate.\n", ch);
		return;
	}

	OBJ_DATA *robot = NULL;

	if ((robot = robot_deconstructor(ch, false)))
	{
		obj_to_room(robot, ch->in_room);
	}
	else
	{
		act("$n could not power down - please email a log of this to holmes@parallelrpi.com", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	act ("Powering down...", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	act ("$n powers down.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	extract_char(ch);
	return;
}
