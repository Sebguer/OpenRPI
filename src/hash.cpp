/*------------------------------------------------------------------------\
|  hash.c : Central Hash Module                       www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

// 3.8.09 - Added in functionality for ocues. - K

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"

#include "room.h"

extern rpie::server engine;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern int top_of_zone_table;
extern struct zone_data *zone_table;

void
	mobile_load_cues (MOB_DATA * mob)
{
	std::multimap<mob_cue,std::string> * cues = new std::multimap<mob_cue,std::string>;
	std::string world_db = engine.get_config ("world_db");
	mysql_safe_query
		( "SELECT cue+0, reflex"
		" FROM %s.cues"
		" WHERE mid = %d "
		" ORDER BY cue, id ASC",
		world_db.c_str (), mob->vnum);

	MYSQL_RES *result = mysql_store_result (database);
	if (result)
	{
		MYSQL_ROW row;
		while ((row = mysql_fetch_row (result)))
		{
			mob_cue c = mob_cue (strtol (row[0], 0, 10));
			cues->insert (std::make_pair (c, std::string (row[1])));
		}

		mysql_free_result (result);
	}

	mob->cues = cues;
}

void
	object_load_cues (OBJ_DATA * obj)
{
	std::multimap<obj_cue,std::string> * ocues = new std::multimap<obj_cue,std::string>;
	std::string world_db = engine.get_config ("world_db");
	mysql_safe_query
		( "SELECT cue+0, reflex"
		" FROM %s.ocues"
		" WHERE mid = %d "
		" ORDER BY cue, id ASC",
		world_db.c_str (), obj->nVirtual);

	MYSQL_RES *result = mysql_store_result (database);
	if (result)
	{
		MYSQL_ROW row;
		while ((row = mysql_fetch_row (result)))
		{
			obj_cue c = obj_cue (strtol (row[0], 0, 10));
			ocues->insert (std::make_pair (c, std::string (row[1])));
		}

		mysql_free_result (result);
	}

	obj->ocues = ocues;
}

CHAR_DATA *
	fread_mobile (int vnum, const int *nZone, FILE * fp)
{
	int i;
	long tmp;
	long tmp2;
	long tmp3;
	long clan1 = 0;
	long clan2 = 0, num = 0;
	char *p;
	char *p2;
	ROOM_DATA *room;
	CHAR_DATA *mob;
	SCENT_DATA *scent;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char chk;
	long materials = 0, buy_flags = 0;
	int skills[MAX_SKILLS];

	mob = new_char (0);		/* NPC */

	//clear_char (mob);
	mob->mob->currency_type = 0;

	mob->mob->vnum = vnum;
	mob->mob->zone = *nZone;

	mob->mob->carcass_vnum = 0;
	mobile_load_cues (mob->mob);

#define CHECK_DOUBLE_DEFS 1
#ifdef CHECK_DOUBLE_DEFS
	if (vnum_to_mob (vnum))
	{
		sprintf (buf, "Mob %d multiply defined!!", vnum);
		system_log (buf, true);
	}
	else
#endif

		add_mob_to_hash (mob);

	mob->name = fread_string (fp);

	one_argument (mob->name, buf);

	mob->tname = add_hash (CAP (buf));

	while ((i = strlen (mob->name)) > 0 &&
		(mob->name[i - 1] == '\r' || mob->name[i - 1] == '\n'))
		mob->name[i - 1] = '\0';

	mob->short_descr = fread_string (fp);

	while ((i = strlen (mob->short_descr)) > 0 &&
		(mob->short_descr[i - 1] == '\r' || mob->short_descr[i - 1] == '\n'))
		mob->short_descr[i - 1] = '\0';

	mob->long_descr = fread_string (fp);

	while ((i = strlen (mob->long_descr)) > 0 &&
		(mob->long_descr[i - 1] == '\r' || mob->long_descr[i - 1] == '\n'))
		mob->long_descr[i - 1] = '\0';

	mob->description = fread_string (fp);

	fscanf (fp, "%lu ", &tmp);
	mob->act = tmp;
	mob->act |= ACT_ISNPC;

	fscanf (fp, " %ld ", &tmp);
	mob->affected_by = tmp;

	fscanf (fp, " %ld ", &tmp);
	mob->offense = (int) tmp;
	fscanf (fp, " %ld ", &tmp);	/* Was defense */
	mob->race = (int) tmp;
	fscanf (fp, " %ld ", &tmp);
	mob->armor = (int) tmp;

	/* Need to reformat the following -- only need one var in the mob file for hp */

	fscanf (fp, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->max_hit = (int) tmp3;
	mob->hit = mob->max_hit;

	fscanf (fp, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->mob->damroll = (int) tmp3;
	mob->mob->damnodice = (int) tmp;
	mob->mob->damsizedice = (int) tmp2;

	mob->move = 50;
	mob->max_move = 50;

	fscanf (fp, " %ld ", &mob->time.birth);

	fscanf (fp, " %ld ", &tmp);
	mob->position = (int) tmp;

	fscanf (fp, " %ld ", &tmp);
	mob->default_pos = (int) tmp;

	fscanf (fp, " %ld ", &tmp);
	mob->sex = (int) tmp;

	fscanf (fp, " %ld ", &tmp);	/* Used for Regi's 7 econs for now */
	mob->mob->merch_seven = (int) tmp;

	// Previously deity; cannibalised for 'materials' for shopkeeps below.
	fscanf (fp, " %ld ", &materials);

	fscanf (fp, " %ld ", &tmp);	/* phys?  what's that? */
	mob->mob->vehicle_type = tmp;

	fscanf (fp, " %ld \n", &buy_flags);

	fscanf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", &mob->mob->skinned_vnum, &mob->circle, &mob->cell_1, &mob->mob->carcass_vnum, &mob->cell_2,	/*Formerly defense_bonus - free */
		&mob->ppoints,
		&mob->natural_delay,
		&mob->mob->helm_room,
		&mob->body_type,
		&mob->scent_type,
		&mob->nat_attack_type,
		&mob->mob->access_flags,
		&mob->height, &mob->frame, &mob->mob->noaccess_flags, &mob->cell_3, &mob->room_pos, &mob->mob->fallback, &mob->mob->armortype);

	if (world_version_in == 5)
	{
		fscanf (fp, "%d %d %d %d %d %d %d\n",
			&mob->str,
			&mob->intel,
			&mob->wil, &mob->aur, &mob->dex, &mob->con, &mob->speaks);
		mob->agi = 16;
	}
	else
		fscanf (fp, "%d %d %d %d %d %d %d %d\n",
		&mob->str,
		&mob->intel,
		&mob->wil,
		&mob->aur, &mob->dex, &mob->con, &mob->speaks, &mob->agi);

	if (lookup_race_int(mob->race, RACE_PC))
	{
		/* Humanoid NPCs. */
		mob->max_hit = 40 + mob->con * CONSTITUTION_MULTIPLIER;
		mob->hit = mob->max_hit;
	}

	mob->max_shock = 40 + mob->wil * CONSTITUTION_MULTIPLIER;
	mob->shock = mob->max_shock;

	fscanf (fp, "%d %d\n", &mob->flags, &mob->mob->currency_type);

	if (IS_SET (mob->flags, FLAG_KEEPER))
	{

		mob->shop = (SHOP_DATA *) get_perm (sizeof (SHOP_DATA));

		mob->shop->materials = materials;
		mob->shop->buy_flags = buy_flags;

		fgets (buf, 256, fp);

		sscanf (buf, "%d %d %f %f %f %f %d\n",
			&mob->shop->shop_vnum,
			&mob->shop->store_vnum,
			&mob->shop->markup,
			&mob->shop->discount,
			&mob->shop->econ_markup1,
			&mob->shop->econ_discount1, &mob->shop->econ_flags1);

		if (mob->mob->merch_seven > 0)
		{
			fscanf (fp,
				"%f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %d\n",
				&mob->shop->econ_markup2, &mob->shop->econ_discount2,
				&mob->shop->econ_flags2, &mob->shop->econ_markup3,
				&mob->shop->econ_discount3, &mob->shop->econ_flags3,
				&mob->shop->econ_markup4, &mob->shop->econ_discount4,
				&mob->shop->econ_flags4, &mob->shop->econ_markup5,
				&mob->shop->econ_discount5, &mob->shop->econ_flags5,
				&mob->shop->econ_markup6, &mob->shop->econ_discount6,
				&mob->shop->econ_flags6, &mob->shop->econ_markup7,
				&mob->shop->econ_discount7, &mob->shop->econ_flags7,
				&mob->shop->nobuy_flags);
		}
		else if (world_version_in >= 8)
		{
			fscanf (fp, "%f %f %d %f %f %d %d\n",
				&mob->shop->econ_markup2,
				&mob->shop->econ_discount2,
				&mob->shop->econ_flags2,
				&mob->shop->econ_markup3,
				&mob->shop->econ_discount3,
				&mob->shop->econ_flags3, &mob->shop->nobuy_flags);
		}

		for (i = 0; i <= MAX_DELIVERIES; i++)
		{
			num = fread_number (fp);
			if (num == -1)
				break;
			mob->shop->delivery[i] = num;
		}

		fscanf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
			&mob->shop->trades_in[0],
			&mob->shop->trades_in[1],
			&mob->shop->trades_in[2],
			&mob->shop->trades_in[3],
			&mob->shop->trades_in[4],
			&mob->shop->trades_in[5],
			&mob->shop->trades_in[6],
			&mob->shop->trades_in[7],
			&mob->shop->trades_in[8], &mob->shop->trades_in[9]);

		if (mob->shop->store_vnum && (room = vnum_to_room (mob->shop->store_vnum)))
		{
			room->room_flags |= STORAGE;
		}
	}

	for (i = 0; i < MAX_SKILLS; i++)
		skills[i] = 0;

	for (i = 0; i < (world_version_in >= 12 ? MAX_SKILLS : 60) / 10; i++)
		fscanf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
		&skills[i * 10],
		&skills[i * 10 + 1],
		&skills[i * 10 + 2],
		&skills[i * 10 + 3],
		&skills[i * 10 + 4],
		&skills[i * 10 + 5],
		&skills[i * 10 + 6],
		&skills[i * 10 + 7], &skills[i * 10 + 8], &skills[i * 10 + 9]);

	for (i = 0; i < MAX_SKILLS; i++)
		mob->skills[i] = skills[i];

	if (world_version_in >= 10)
		mob->clans = fread_string (fp);

	// For mobiles with scent

	if (IS_SET (mob->act, ACT_SCENT))
	{
		for (i = 0; i < mob->scent_type; i++)
		{
			//CREATE (scent, POISON_DATA, 1);
			scent = (SCENT_DATA *) get_perm (sizeof (SCENT_DATA));

			scent->scent_ref = 0;
			scent->permanent = 0;

			scent->atm_power = 0;
			scent->pot_power = 0;
			scent->rat_power = 0;

			scent->next = NULL;

			fscanf(fp, "%d %d %d %d %d\n", &scent->scent_ref, &scent->permanent, &scent->atm_power, &scent->pot_power, &scent->rat_power);

			scent_to_mob(mob, scent);
		}
	}

	/*** for morphing mobs ***/
	chk = getc (fp);
	if (chk != '#')
	{
		fscanf (fp, "%d %d %d\n", &mob->clock, &mob->morphto, &mob->morph_type);
	}
	else
		ungetc (chk, fp);

	/*************************/



	mob->time.played = 0;
	mob->time.logon = time (0);

	mob->intoxication = 0;
	mob->hunger = -1;
	mob->thirst = -1;

	mob->tmp_str = mob->str;
	mob->tmp_dex = mob->dex;
	mob->tmp_intel = mob->intel;
	mob->tmp_aur = mob->aur;
	mob->tmp_wil = mob->wil;
	mob->tmp_con = mob->con;
	mob->tmp_agi = mob->agi;

	mob->equip = NULL;

	mob->mob->vnum = vnum;

	mob->descriptor = 0;

	if (mob->speaks == 0)
	{
		mob->skills[SKILL_COMMON] = 100;
		mob->speaks = SKILL_COMMON;
	}

	if (!mob->skills[mob->speaks])
		mob->skills[mob->speaks] = 100;

	p = mob->clans;
	p2 = p;
	mob->clans = str_dup ("");

	while (*p2)
	{

		p = one_argument (p, buf);	/* flags     */
		p = one_argument (p, buf2);	/* clan name */

		if (!*buf2)
			break;

		add_clan_id (mob, buf2, buf);
	}

	if (p2 && *p2)
		mem_free (p2);

	if (clan1 == 1)
	{
		clan1 = 0;
		mob->act |= ACT_WILDLIFE;
	}

	if (clan2 == 1)
	{
		clan2 = 0;
		mob->act |= ACT_WILDLIFE;
	}

	if (clan1)
	{
		sprintf (buf, "%ld", clan1);
		add_clan_id (mob, buf,
			GET_FLAG (mob, FLAG_LEADER_1) ? "leader" : "member");
		mob->flags &= ~FLAG_LEADER_1;
	}

	if (clan2)
	{
		sprintf (buf, "%ld", clan2);
		add_clan_id (mob, buf,
			GET_FLAG (mob, FLAG_LEADER_2) ? "leader" : "member");
		mob->flags &= ~FLAG_LEADER_2;
	}

	fix_offense (mob);

	// Same calc as for health, but with willpower instead
	mob->max_shock = 40 + mob->wil * 4;
	mob->shock = mob->max_shock;

	if (lookup_race_variable (mob->race, RACE_BODY_PROTO) != NULL)
		mob->body_proto =
		atoi (lookup_race_variable (mob->race, RACE_BODY_PROTO));

	if (lookup_race_variable (mob->race, RACE_MIN_HEIGHT) != NULL)
		mob->mob->min_height =
		atoi (lookup_race_variable (mob->race, RACE_MIN_HEIGHT));

	if (lookup_race_variable (mob->race, RACE_MAX_HEIGHT) != NULL)
		mob->mob->max_height =
		atoi (lookup_race_variable (mob->race, RACE_MAX_HEIGHT));

	apply_race_affects (mob);

	return (mob);
}

CHAR_DATA *
	load_mobile (int vnum)
{
	CHAR_DATA *proto;
	CHAR_DATA *new_mobile;
	MOB_DATA *mob_info;

	if (!(proto = vnum_to_mob (vnum)))
		return NULL;

	new_mobile = new_char (0);	/* NPC */

	mob_info = new_mobile->mob;

	/*

	memcpy (new_mobile, proto, sizeof (CHAR_DATA));

	new_mobile->mob = mob_info;

	memcpy (new_mobile->mob, proto->mob, sizeof (MOB_DATA));

	*/

	new_mobile->deep_copy(proto);

	/* A mostly unique number.  Can be used to ensure
	the same mobile is being used between game plays.  */

	new_mobile->coldload_id = get_next_coldload_id (0);
	new_mobile->deleted = 0;

	new_mobile->next = character_list;
	character_list = new_mobile;

	new_mobile->time.birth = time (0);

	new_mobile->max_move = calc_lookup (new_mobile, REG_MISC, MISC_MAX_MOVE);

	if (!new_mobile->height)
		make_height (new_mobile);

	if (!new_mobile->frame)
		make_frame (new_mobile);

	if (IS_SET (new_mobile->affected_by, AFF_HIDE) && !get_affect (new_mobile, MAGIC_HIDDEN))
		magic_add_affect (new_mobile, MAGIC_HIDDEN, -1, 0, 0, 0, 0);

	new_mobile->fight_mode = 2;

	if (IS_SET (new_mobile->flags, FLAG_VARIABLE))
	{
		randomize_mobile (new_mobile);
		new_mobile->flags &= ~FLAG_VARIABLE;
	}

	new_mobile->clans = str_dup (proto->clans);

	new_mobile->move = new_mobile->max_move;

	new_mobile->mount = NULL;
	new_mobile->wounds = NULL;
	new_mobile->lodged = NULL;
	new_mobile->subdue = NULL;

	new_mobile->scent = NULL;
	if (proto->scent)
	{
		SCENT_DATA *scent = NULL;

		for (scent = proto->scent; scent; scent = scent->next)
		{
			add_scent(new_mobile, scent->scent_ref, scent->permanent, scent->atm_power, scent->pot_power, scent->rat_power, 1);
		}

	}

	new_mobile->mob->owner = NULL;

	if ((new_mobile->clock > 0) && (new_mobile->morphto > 0))
	{
		new_mobile->morph_time = time (0) + new_mobile->clock * 15 * 60;
		new_mobile->act |= ACT_STAYPUT;
	}

	if (new_mobile->speaks != SKILL_COMMON)
		new_mobile->speaks = SKILL_COMMON;

	return new_mobile;
}

// Determines whether the variable we have is a valid color.

int
	is_variable_color(char *string)
{
	if (vd_variable(string))
		return 1;
	else
		return 0;
}

void
	insert_exact_string_variables (OBJ_DATA * new_obj, OBJ_DATA * proto, char *string0, char *string1, char *string2, char *string3, char *string4, char *string5, char *string6, char *string7, char *string8, char *string9,
	char *st_cat0, char *st_cat1, char *st_cat2, char *st_cat3, char *st_cat4, char *st_cat5, char *st_cat6, char *st_cat7, char *st_cat8, char *st_cat9)
{
	char buf2[MAX_STRING_LENGTH];
	char temp[MAX_STRING_LENGTH];
	char original[MAX_STRING_LENGTH];

	char *xcolor[10];
	char *xcat[10];


	// We start our optionals by assuming we've got something.
	// If we don't have a variable, this'll be set to zero, so
	// we don't include it in short descs.
	int xoptional[10] =  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int xorder[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

	for (int ind = 0; ind <= 9; ind++)
	{
		xcolor[ind] = '\0';
		xcat[ind] = '\0';
	}

	char tempcolor[AVG_STRING_LENGTH] = { '\0' };
	char *point;
	int j = 0, h = 0;
	bool modified = false;

	char buf[AVG_STRING_LENGTH] = { '\0' };
	*buf2 = '\0';
	*temp = '\0';
	int temp_round = 0;
	bool bumped = false;
	bool skipped = false;

	if (string0 == 0)
		xcolor[0] = '\0';
	else
		xcolor[0] = str_dup (string0);

	if (string1 == 0)
		xcolor[1] = '\0';
	else
		xcolor[1] = str_dup (string1);

	if (string2 == 0)
		xcolor[2] = '\0';
	else
		xcolor[2] = str_dup (string2);

	if (string3 == 0)
		xcolor[3] = '\0';
	else
		xcolor[3] = str_dup (string3);

	if (string4 == 0)
		xcolor[4] = '\0';
	else
		xcolor[4] = str_dup (string4);

	if (string5 == 0)
		xcolor[5] = '\0';
	else
		xcolor[5] = str_dup (string5);

	if (string6 == 0)
		xcolor[6] = '\0';
	else
		xcolor[6] = str_dup (string6);

	if (string7 == 0)
		xcolor[7] = '\0';
	else
		xcolor[7] = str_dup (string7);

	if (string8 == 0)
		xcolor[8] = '\0';
	else
		xcolor[8] = str_dup (string8);

	if (string9 == 0)
		xcolor[9] = '\0';
	else
		xcolor[9] = str_dup (string9);

	if (st_cat0 == 0)
		xcat[0] = '\0';
	else
		xcat[0] = str_dup (st_cat0);

	if (st_cat1 == 0)
		xcat[1] = '\0';
	else
		xcat[1] = str_dup (st_cat1);

	if (st_cat2 == 0)
		xcat[2] = '\0';
	else
		xcat[2] = str_dup (st_cat2);

	if (st_cat3 == 0)
		xcat[3] = '\0';
	else
		xcat[3] = str_dup (st_cat3);

	if (st_cat4 == 0)
		xcat[4] = '\0';
	else
		xcat[4] = str_dup (st_cat4);

	if (st_cat5 == 0)
		xcat[5] = '\0';
	else
		xcat[5] = str_dup (st_cat5);

	if (st_cat6 == 0)
		xcat[6] = '\0';
	else
		xcat[6] = str_dup (st_cat6);

	if (st_cat7 == 0)
		xcat[7] = '\0';
	else
		xcat[7] = str_dup (st_cat7);

	if (st_cat8 == 0)
		xcat[8] = '\0';
	else
		xcat[8] = str_dup (st_cat8);

	if (st_cat9 == 0)
		xcat[9] = '\0';
	else
		xcat[9] = str_dup (st_cat9);


	// If color doesn't match any known variable colors, we set them to null.
	// Prevents you loading up silly colours.
	for (int ind = 0; ind <= 9; ind++)
	{
		if (!is_variable_color(xcolor[ind]))
			xcolor[ind] = '\0';
	}

	// If we've got a color, but we didn't have a category set, then just
	// try and pick one at random.

	// Hacky-hack time:
	// first we try our $colors, and then go to a random variable.
	for (int ind = 0; ind <= 9; ind++)
	{
		if (xcolor[ind] && !xcat[ind])
		{
			if (vc_exists(xcolor[ind], "$color"))
				xcat[ind] = add_hash("$color");
			else if (vc_exists(xcolor[ind], "$finecolor"))
				xcat[ind] = add_hash("$finecolor");
			else if (vc_exists(xcolor[ind], "$drabcolor"))
				xcat[ind] = add_hash("$drabcolor");
			else if (vc_exists(xcolor[ind], "$leathercolor"))
				xcat[ind] = add_hash("$leathercolor");
			else if (vc_exists(xcolor[ind], "$plasticcolor"))
				xcat[ind] = add_hash("$plasticcolor");
			else if (vc_exists(xcolor[ind], "$lizcolor"))
				xcat[ind] = add_hash("$lizcolor");
			else if (vc_exists(xcolor[ind], "$metalcolor"))
				xcat[ind] = add_hash("$metalcolor");
			else
				xcat[ind] = add_hash(vc_category(vd_variable(xcolor[ind])));
		}
	}

	// We remove the varible flag because we don't really need it anymore.
	new_obj->obj_flags.extra_flags |= ITEM_VARIABLE;

	// If we don't have a full desc, quit out.
	if (!*proto->full_description)
		return;

	// Save our original description of the full description.
	sprintf (original, "%s", proto->full_description);

	// Find at what point we have our first "$".
	point = strpbrk (original, "$");
	int round = 0;

	// If we found point...
	if (point)
	{
		// Then for every character in the string...
		// We run through the original, adding each bit of y to buf2.
		// However, if we find a $, we see if that's a category of variables.
		// If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

		for (size_t y = 0; y <= strlen (original); y++)
		{
			skipped = false;
			// If we're at the $...
			if (original[y] == *point)
			{
				// Then we're going to modify it...
				modified = true;
				// ... so let's keep a temporary marker ...
				sprintf (temp, "$");
				// ... and jump ahead a point (to get to the letter after the $
				j = y + 1;

				// Now, until we hit something that's not a alpha-numeric character.
				while (isalpha (original[j]))
				{
					// add the word after the $ to our temporary marker.
					sprintf (temp + strlen (temp), "%c", original[j]);
					j++;
				}

				// If there's a number after our category, then we're going to round it all up - let's add it to our xorder list.
				*buf = '\0';

				sprintf(buf, "%c", original[j]);

				if (isdigit(*buf))
					xorder[round] = atoi(buf);
				else
					xorder[round] = -1;

				// Now, we figure out which colour we'e setting by seeing if we don't have the color of the present round...
				if (!xcolor[round])
				{
					// If our temporary marker doesn't match any known category, then we've encountered an error and will return nothing but blanks.
					if (!vc_category(temp))
					{
						for (int ind = 0; ind <= 9; ind++)
						{
							new_obj->var_color[ind] = "(null)";
							new_obj->var_cat[ind] = "(null)";
						}
						return;
					}

					// If this is a optional variable, we don't add to xcat and xcolor if we don't need.
					if (!str_cmp(buf, "!"))
					{
						// There was an optional, and we had nothing..
						xoptional[round] = 0;
						skipped = true;
					}
					else
					{
						// Now that we know temp is from a proper category, we pull a random variable from that category and call it tempcolor.
						sprintf (tempcolor, "%s", vc_rand(temp));

						// Now, we check what round we are and assign tempcolor to that round, and do the same for the categories.
						xcolor[round] = add_hash (tempcolor);
						xcat[round] = add_hash (temp);
					}


				}
				// If we did have a colour specified, let's see if first of all if that color exists on our
				// own categories list. If it is, we need to revise the category. If not, then we'll just
				// stick with the category that we had set up above.
				else
				{
					if (vc_category(temp) && vc_exists(xcolor[round], temp))
					{
						xcat[round] = add_hash (temp);
					}
				}

				// Now, depending on the round, we add on to buf2 the color we just pulled, and then advance to the next round.
				if (!skipped)
				{
					if (xcat[round])
					{
						sprintf (buf2 + strlen (buf2), "%s", vd_full(xcat[round], xcolor[round]));
					}
					else
					{
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);
					}
				}

				// Now, we set our end point as where our category ends plus 1.
				j = y + 1;

				// We advance until we get to the new non-alpha-numeric character.
				while (isalpha (original[j]))
					j++;

				if (xorder[round] >= 0)
					j++;

				if (!str_cmp(buf, "!"))
				{
					j++;
				}

				// If we're at round ten, time to quit.
				if (round < 9)
					round ++;

				// And then set the point of our main loop to that point
				y = j;
			}
			sprintf (buf2 + strlen (buf2), "%c", original[y]);
		}
		mem_free (new_obj->full_description);
		new_obj->full_description = add_hash (buf2);
		reformat_desc(new_obj->full_description, &new_obj->full_description);
	}

	if (modified)
	{

		*buf2 = '\0';
		sprintf (original, "%s", proto->short_description);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->short_description);
			new_obj->short_description = add_hash (buf2);
		}

		*buf2 = '\0';
		sprintf (original, "%s", proto->description);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->description);
			new_obj->description = add_hash (buf2);
		}

		*buf2 = '\0';
		sprintf (original, "%s", proto->name);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->name);
			new_obj->name = add_hash (buf2);
		}

		if ((IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_ARMOR)
			|| (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_WORN)
			|| (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_CONTAINER))
		{

			*buf2 = '\0';
			sprintf (original, "%s", proto->desc_keys);
			point = strpbrk (original, "$");
			round = 0;

			if (point)
			{
				for (size_t y = 0; y <= strlen (original); y++)
				{
					temp_round = round;
					bumped = false;

					if (original[y] == *point)
					{
						modified = true;
						sprintf (temp, "$");
						j = y + 1;

						h = j;

						while (isalpha (original[h]))
							h++;

						*buf = '\0';

						sprintf(buf, "%c", original[h]);

						if (isdigit(*buf))
						{
							round = atoi(buf);
							bumped = true;
						}

						if (xcolor[round] && xoptional[round])
							sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

						round = temp_round;

						j = y + 1;

						while (isalpha (original[j]))
							j++;

						if (bumped)
							j++;

						y = j;
						round++;
					}
					sprintf (buf2 + strlen (buf2), "%c", original[y]);
				}
				mem_free (new_obj->desc_keys);
				new_obj->desc_keys = add_hash (buf2);
			}
		}

		if (GET_ITEM_TYPE(new_obj) == ITEM_FOOD || GET_ITEM_TYPE(new_obj) == ITEM_FLUID)
		{
			bool taste_change;
			bool has_taste;
			std::string total_taste;

			if (new_obj->ink_color && str_cmp(new_obj->ink_color, "(null)"))
			{
				has_taste = true;
				total_taste.assign(new_obj->ink_color);
			}

			for (int ind = 0; ind <= 9; ind++)
			{
				char *taste_buf;
				taste_buf = vd_flavour(xcat[ind], xcolor[ind]);

				if (taste_buf)
				{
					taste_change = true;
					if (has_taste)
					{
						total_taste += ", and " + MAKE_STRING(taste_buf);
					}
					else
					{
						total_taste = MAKE_STRING(taste_buf);
						has_taste = true;
					}
					mem_free(taste_buf);
				}
			}

			if (taste_change)
			{
				if (new_obj->ink_color)
				{
					mem_free(new_obj->ink_color);
				}

				new_obj->ink_color = add_hash(total_taste.c_str());
			}
		}
	}

	// End point.

	for (int ind = 0; ind <= 9; ind++)
	{
		if (new_obj->var_color[ind])
			mem_free (new_obj->var_color[ind]);

		if (new_obj->var_cat[ind])
			mem_free (new_obj->var_cat[ind]);
	}

	if (!modified)
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			new_obj->var_color[ind] = add_hash ("(null)");
			new_obj->var_cat[ind] = add_hash ("(null)");
		}
	}
	else
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			new_obj->var_color[ind] = add_hash (xcolor[ind]);
			new_obj->var_cat[ind] = add_hash (xcat[ind]);
		}
	}

	for (int ind = 0; ind <= 9; ind++)
	{
		mem_free(xcolor[ind]);
		mem_free(xcat[ind]);
	}

	correct_grammar(new_obj->short_description, &new_obj->short_description);
	correct_grammar(new_obj->description, &new_obj->description);
	correct_grammar(new_obj->full_description, &new_obj->full_description);
}

void
	insert_string_variables (OBJ_DATA * new_obj, OBJ_DATA * proto, char *string0, char *string1, char *string2, char *string3, char *string4, char *string5, char *string6, char *string7, char *string8, char *string9)
{
	char buf2[MAX_STRING_LENGTH];
	char temp[MAX_STRING_LENGTH];
	char original[MAX_STRING_LENGTH];

	char *xcolor[10];
	char *xcat[10];


	// We start our optionals by assuming we've got something.
	// If we don't have a variable, this'll be set to zero, so
	// we don't include it in short descs.
	int xoptional[10] =  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int xorder[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

	for (int ind = 0; ind <= 9; ind++)
	{
		xcolor[ind] = '\0';
		xcat[ind] = '\0';
	}

	char tempcolor[AVG_STRING_LENGTH] = { '\0' };
	char *point;
	int j = 0, h = 0;
	bool modified = false;

	char buf[AVG_STRING_LENGTH] = { '\0' };
	*buf2 = '\0';
	*temp = '\0';
	int temp_round = 0;
	bool bumped = false;
	bool skipped = false;

	if (string0 == 0)
		xcolor[0] = '\0';
	else
		xcolor[0] = str_dup (string0);

	if (string1 == 0)
		xcolor[1] = '\0';
	else
		xcolor[1] = str_dup (string1);

	if (string2 == 0)
		xcolor[2] = '\0';
	else
		xcolor[2] = str_dup (string2);

	if (string3 == 0)
		xcolor[3] = '\0';
	else
		xcolor[3] = str_dup (string3);

	if (string4 == 0)
		xcolor[4] = '\0';
	else
		xcolor[4] = str_dup (string4);

	if (string5 == 0)
		xcolor[5] = '\0';
	else
		xcolor[5] = str_dup (string5);

	if (string6 == 0)
		xcolor[6] = '\0';
	else
		xcolor[6] = str_dup (string6);

	if (string7 == 0)
		xcolor[7] = '\0';
	else
		xcolor[7] = str_dup (string7);

	if (string8 == 0)
		xcolor[8] = '\0';
	else
		xcolor[8] = str_dup (string8);

	if (string9 == 0)
		xcolor[9] = '\0';
	else
		xcolor[9] = str_dup (string9);


	// If color doesn't match any known variable colors, we set them to null.
	// Prevents you loading up silly colours.
	for (int ind = 0; ind <= 9; ind++)
	{
		if (!is_variable_color(xcolor[ind]))
			xcolor[ind] = '\0';
	}

	// If we have got colours, we need to take a guess as to which category they belong to.

	// Hacky-hack time:
	// first we try our $colors, and then go to a random variable.
	for (int ind = 0; ind <= 9; ind++)
	{
		if (xcolor[ind])
		{
			if (vc_exists(xcolor[ind], "$color"))
				xcat[ind] = add_hash("$color");
			else if (vc_exists(xcolor[ind], "$finecolor"))
				xcat[ind] = add_hash("$finecolor");
			else if (vc_exists(xcolor[ind], "$drabcolor"))
				xcat[ind] = add_hash("$drabcolor");
			else if (vc_exists(xcolor[ind], "$leathercolor"))
				xcat[ind] = add_hash("$leathercolor");
			else if (vc_exists(xcolor[ind], "$plasticcolor"))
				xcat[ind] = add_hash("$plasticcolor");
			else if (vc_exists(xcolor[ind], "$lizcolor"))
				xcat[ind] = add_hash("$lizcolor");
			else if (vc_exists(xcolor[ind], "$metalcolor"))
				xcat[ind] = add_hash("$metalcolor");
			else
				xcat[ind] = add_hash(vc_category(vd_variable(xcolor[ind])));
		}
	}

	// We remove the varible flag because we don't really need it anymore.
	new_obj->obj_flags.extra_flags |= ITEM_VARIABLE;

	// If we don't have a full desc, quit out.
	if (!*proto->full_description)
		return;

	// Save our original description of the full description.
	sprintf (original, "%s", proto->full_description);

	// Find at what point we have our first "$".
	point = strpbrk (original, "$");
	int round = 0;

	// If we found point...
	if (point)
	{
		// Then for every character in the string...
		// We run through the original, adding each bit of y to buf2.
		// However, if we find a $, we see if that's a category of variables.
		// If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

		for (size_t y = 0; y <= strlen (original); y++)
		{
			skipped = false;
			// If we're at the $...
			if (original[y] == *point)
			{
				// Then we're going to modify it...
				modified = true;
				// ... so let's keep a temporary marker ...
				sprintf (temp, "$");
				// ... and jump ahead a point (to get to the letter after the $
				j = y + 1;

				// Now, until we hit something that's not a alpha-numeric character.
				while (isalpha (original[j]))
				{
					// add the word after the $ to our temporary marker.
					sprintf (temp + strlen (temp), "%c", original[j]);
					j++;
				}

				// If there's a number after our category, then we're going to round it all up - let's add it to our xorder list.

				*buf = '\0';

				sprintf(buf, "%c", original[j]);

				if (isdigit(*buf))
					xorder[round] = atoi(buf);
				else
					xorder[round] = -1;

				// Now, we figure out which colour we'e setting by seeing if we don't have the color of the present round...
				if (!xcolor[round])
				{
					// If our temporary marker doesn't match any known category, then we've encountered an error and will return nothing but blanks.
					if (!vc_category(temp))
					{
						for (int ind = 0; ind <= 9; ind++)
						{
							new_obj->var_color[ind] = "(null)";
							new_obj->var_cat[ind] = "(null)";
						}
						return;
					}

					// If this is a optional variable, we don't add to xcat and xcolor if we don't need.
					if (!str_cmp(buf, "!"))
					{
						// There was an optional, and we had nothing..
						xoptional[round] = 0;
						skipped = true;
					}
					else
					{
						// Now that we know temp is from a proper category, we pull a random variable from that category and call it tempcolor.
						sprintf (tempcolor, "%s", vc_rand(temp));

						// Now, we check what round we are and assign tempcolor to that round, and do the same for the categories.
						xcolor[round] = add_hash (tempcolor);
						xcat[round] = add_hash (temp);
					}


				}
				// If we did have a colour specified, let's see if first of all if that color exists on our
				// own categories list. If it is, we need to revise the category. If not, then we'll just
				// stick with the category that we had set up above.
				else
				{
					if (vc_category(temp) && vc_exists(xcolor[round], temp))
					{
						xcat[round] = add_hash (temp);
					}
				}

				// Now, depending on the round, we add on to buf2 the color we just pulled, and then advance to the next round.
				if (!skipped)
				{
					if (xcat[round])
					{
						sprintf (buf2 + strlen (buf2), "%s", vd_full(xcat[round], xcolor[round]));
					}
					else
					{
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);
					}
				}

				// Now, we set our end point as where our category ends plus 1.
				j = y + 1;

				// We advance until we get to the new non-alpha-numeric character.
				while (isalpha (original[j]))
					j++;

				if (xorder[round] >= 0)
					j++;

				if (!str_cmp(buf, "!"))
				{
					j++;
				}

				// If we're at round ten, time to quit.
				if (round < 9)
					round ++;

				// And then set the point of our main loop to that point
				y = j;
			}
			sprintf (buf2 + strlen (buf2), "%c", original[y]);
		}
		mem_free (new_obj->full_description);
		new_obj->full_description = add_hash (buf2);
		reformat_desc(new_obj->full_description, &new_obj->full_description);
	}

	if (modified)
	{

		*buf2 = '\0';
		sprintf (original, "%s", proto->short_description);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->short_description);
			new_obj->short_description = add_hash (buf2);
		}

		*buf2 = '\0';
		sprintf (original, "%s", proto->description);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->description);
			new_obj->description = add_hash (buf2);
		}

		*buf2 = '\0';
		sprintf (original, "%s", proto->name);
		point = strpbrk (original, "$");
		round = 0;

		if (point)
		{
			for (size_t y = 0; y <= strlen (original); y++)
			{
				temp_round = round;
				bumped = false;

				if (original[y] == *point)
				{
					modified = true;
					sprintf (temp, "$");
					j = y + 1;

					h = j;

					while (isalpha (original[h]))
						h++;

					*buf = '\0';

					sprintf(buf, "%c", original[h]);

					if (isdigit(*buf))
					{
						round = atoi(buf);
						bumped = true;
					}

					if (xcolor[round] && xoptional[round])
						sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

					round = temp_round;

					j = y + 1;

					while (isalpha (original[j]))
						j++;

					if (bumped)
						j++;

					y = j;
					round++;
				}
				sprintf (buf2 + strlen (buf2), "%c", original[y]);
			}
			mem_free (new_obj->name);
			new_obj->name = add_hash (buf2);
		}

		if ((IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_ARMOR)
			|| (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_WORN)
			|| (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_CONTAINER))
		{

			*buf2 = '\0';
			sprintf (original, "%s", proto->desc_keys);
			point = strpbrk (original, "$");
			round = 0;

			if (point)
			{
				for (size_t y = 0; y <= strlen (original); y++)
				{
					temp_round = round;
					bumped = false;

					if (original[y] == *point)
					{
						modified = true;
						sprintf (temp, "$");
						j = y + 1;

						h = j;

						while (isalpha (original[h]))
							h++;

						*buf = '\0';

						sprintf(buf, "%c", original[h]);

						if (isdigit(*buf))
						{
							round = atoi(buf);
							bumped = true;
						}

						if (xcolor[round] && xoptional[round])
							sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

						round = temp_round;

						j = y + 1;

						while (isalpha (original[j]))
							j++;

						if (bumped)
							j++;

						y = j;
						round++;
					}
					sprintf (buf2 + strlen (buf2), "%c", original[y]);
				}
				mem_free (new_obj->desc_keys);
				new_obj->desc_keys = add_hash (buf2);
			}
		}

		if (GET_ITEM_TYPE(new_obj) == ITEM_FOOD || GET_ITEM_TYPE(new_obj) == ITEM_FLUID)
		{
			bool taste_change;
			bool has_taste;
			std::string total_taste;

			if (new_obj->ink_color && str_cmp(new_obj->ink_color, "(null)"))
			{
				has_taste = true;
				total_taste.assign(new_obj->ink_color);
			}

			for (int ind = 0; ind <= 9; ind++)
			{
				char *taste_buf;
				taste_buf = vd_flavour(xcat[ind], xcolor[ind]);

				if (taste_buf)
				{
					taste_change = true;
					if (has_taste)
					{
						total_taste += ", and " + MAKE_STRING(taste_buf);
					}
					else
					{
						total_taste = MAKE_STRING(taste_buf);
						has_taste = true;
					}
					mem_free(taste_buf);
				}
			}

			if (taste_change)
			{
				if (new_obj->ink_color)
				{
					mem_free(new_obj->ink_color);
				}

				new_obj->ink_color = add_hash(total_taste.c_str());
			}
		}
	}

	// End point.

	for (int ind = 0; ind <= 9; ind++)
	{
		if (new_obj->var_color[ind])
			mem_free (new_obj->var_color[ind]);

		if (new_obj->var_cat[ind])
			mem_free (new_obj->var_cat[ind]);
	}

	if (!modified)
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			new_obj->var_color[ind] = add_hash ("(null)");
			new_obj->var_cat[ind] = add_hash ("(null)");
		}
	}
	else
	{
		for (int ind = 0; ind <= 9; ind++)
		{
			new_obj->var_color[ind] = add_hash (xcolor[ind]);
			new_obj->var_cat[ind] = add_hash (xcat[ind]);
		}
	}

	for (int ind = 0; ind <= 9; ind++)
	{
		mem_free(xcolor[ind]);
		mem_free(xcat[ind]);
	}

	correct_grammar(new_obj->short_description, &new_obj->short_description);
	correct_grammar(new_obj->description, &new_obj->description);
	correct_grammar(new_obj->full_description, &new_obj->full_description);
}


OBJ_DATA *
	load_object (int vnum)
{
	OBJ_DATA *proto;
	OBJ_DATA *new_obj;
	WRITING_DATA *writing;
	int i;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *new_af;
	AFFECTED_TYPE *last_af = NULL;

	if (!(proto = vtoo (vnum)))
		return NULL;

	new_obj = new_object ();

	//memcpy (new_obj, proto, sizeof (OBJ_DATA));

	new_obj->deep_copy(proto);

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	new_obj->next_content = 0;

	for (af = proto->xaffected; af; af = af->next)
	{

		new_af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if (!new_obj->xaffected)
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->scent = NULL;
	if (proto->scent)
	{
		SCENT_DATA *scent = NULL;

		for (scent = proto->scent; scent; scent = scent->next)
		{
			add_scent(new_obj, scent->scent_ref, scent->permanent, scent->atm_power, scent->pot_power, scent->rat_power, 1);
		}
	}

	new_obj->next = object_list;
	object_list = new_obj;

	new_obj->count = 1;

	if (new_obj->clock && new_obj->morphto)
		new_obj->morphTime = time (0) + new_obj->clock * 15 * 60;



	if ( USES_BOOK_CODE( new_obj ) )
	{
	    int writing_oval = GET_PAGE_OVAL( new_obj );

		if ( !new_obj->writing && new_obj->o.od.value[ writing_oval ] > 0 )
		{
			CREATE ( new_obj->writing, WRITING_DATA, 1 );

			for ( i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[ writing_oval ]; i++ )
			{
				writing->message    = add_hash ("blank");
				writing->author     = add_hash ("blank");
				writing->date       = add_hash ("blank");
				writing->ink        = add_hash ("blank");
				writing->language   = 0;
				writing->script     = 0;
				writing->skill      = 0;
				writing->torn       = false;

				if (i != new_obj->o.od.value[ writing_oval ] )
				{
					CREATE (writing->next_page, WRITING_DATA, 1);
					writing = writing->next_page;
				}
			}
		}

		new_obj->o.od.value[ writing_oval + 1 ] = unused_writing_id ();
	} else if ( GET_ITEM_TYPE( new_obj ) == ITEM_PARCHMENT )
        new_obj->o.od.value[ 0 ] = unused_writing_id ();

	// If we're booting, then the only objects we're loading have already been loaded:
	// no need to give them a new set of variables.
	extern int booting;
	if (!booting && IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE) && *proto->full_description)
	{
		insert_string_variables (new_obj, proto, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		vo_specify(new_obj);

		if (get_scent(new_obj, scent_lookup("generic-variable")))
		{
			if (GET_ITEM_TYPE(new_obj) == ITEM_CHEM_SMOKE)
			{
				for (int ind = 0; ind < 10; ind++)
				{
					if (!str_cmp(new_obj->var_cat[ind], "$naturalscents") ||
						!str_cmp(new_obj->var_cat[ind], "$artificialscents"))
					{
						remove_obj_scent(new_obj, scent_lookup("generic-variable"));
						add_scent(new_obj, scent_lookup(new_obj->var_color[ind]), 1, new_obj->o.smoke.scent_power, 0, 0, 1);
					}
				}
			}
		}
	}

	if (GET_ITEM_TYPE (new_obj) == ITEM_WEAPON)
		new_obj->obj_flags.extra_flags |= ITEM_NEWSKILLS;


	if (GET_ITEM_TYPE (new_obj) == ITEM_CLUSTER)
	{
		if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MINOR))
		{
			new_obj->o.od.value[3] += number(-2,2);
			MAX(new_obj->o.od.value[3], 1);
		}

		if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MINOR))
		{
			new_obj->o.od.value[5] += number(-2,2);
			MAX(new_obj->o.od.value[5], 1);
		}
	}

	new_obj->contains = NULL;
	new_obj->lodged = NULL;
	new_obj->wounds = NULL;
	new_obj->equiped_by = NULL;
	new_obj->carried_by = NULL;
	new_obj->in_obj = NULL;

	if (!new_obj->item_wear)
		new_obj->item_wear = 100;

	new_obj->coldload_id = get_next_coldload_id (2);

	vtoo (new_obj->nVirtual)->instances++;

	return new_obj;
}

OBJ_DATA *
	load_exact_colored_object (int vnum, char *color0, char *color1, char *color2, char *color3, char *color4, char *color5, char *color6, char *color7, char *color8, char *color9,
	char *cat0, char *cat1, char *cat2, char *cat3, char *cat4, char *cat5, char *cat6, char *cat7, char *cat8, char *cat9)
{
	OBJ_DATA *proto;
	OBJ_DATA *new_obj;
	WRITING_DATA *writing;
	int i;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *new_af;
	AFFECTED_TYPE *last_af = NULL;

	if (!(proto = vtoo (vnum)))
		return NULL;

	new_obj = new_object ();

	//memcpy (new_obj, proto, sizeof (OBJ_DATA));

	new_obj->deep_copy(proto);

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	for (af = proto->xaffected; af; af = af->next)
	{

		new_af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if (!new_obj->xaffected)
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->next = object_list;
	object_list = new_obj;

	new_obj->count = 1;

	if (new_obj->clock && new_obj->morphto)
		new_obj->morphTime = time (0) + new_obj->clock * 15 * 60;

	if ( USES_BOOK_CODE( new_obj ) )
	{
	    int writing_oval = GET_PAGE_OVAL( new_obj );

		if ( !new_obj->writing && new_obj->o.od.value[ writing_oval ] > 0)
		{
			CREATE( new_obj->writing, WRITING_DATA, 1 );

			for ( i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[ writing_oval ]; i++ )
			{
				writing->message    = add_hash( "blank" );
				writing->author     = add_hash( "blank" );
				writing->date       = add_hash( "blank" );
				writing->ink        = add_hash( "blank" );
				writing->language   = 0;
				writing->script     = 0;
				writing->skill      = 0;
				writing->torn       = false;

				if ( i != new_obj->o.od.value[ writing_oval ] )
				{
					CREATE ( writing->next_page, WRITING_DATA, 1 );
					writing = writing->next_page;
				}
			}
		}
	}

	if (IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE) && *proto->full_description)
	{
		insert_exact_string_variables (new_obj, proto, color0, color1, color2, color3, color4, color5, color6, color7, color8, color9,
			cat0, cat1, cat2, cat3, cat4, cat5, cat6, cat7, cat8, cat9);
		vo_specify(new_obj);

		if (get_scent(new_obj, scent_lookup("generic-variable")))
		{
			if (GET_ITEM_TYPE(new_obj) == ITEM_CHEM_SMOKE)
			{
				for (int ind = 0; ind < 10; ind++)
				{
					if (!str_cmp(new_obj->var_cat[ind], "$naturalscents") ||
						!str_cmp(new_obj->var_cat[ind], "$artificialscents"))
					{
						remove_obj_scent(new_obj, scent_lookup("generic-variable"));
						add_scent(new_obj, scent_lookup(new_obj->var_color[ind]), 1, new_obj->o.smoke.scent_power, 0, 0, 1);
					}
				}
			}
		}
	}

	if (GET_ITEM_TYPE (new_obj) == ITEM_WEAPON)
		new_obj->obj_flags.extra_flags |= ITEM_NEWSKILLS;

	if (GET_ITEM_TYPE (new_obj) == ITEM_CLUSTER)
	{
		if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MINOR))
		{
			new_obj->o.od.value[3] += number(-2,2);
			MAX(new_obj->o.od.value[3], 1);
		}

		if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MINOR))
		{
			new_obj->o.od.value[5] += number(-2,2);
			MAX(new_obj->o.od.value[5], 1);
		}
	}

	new_obj->contains = NULL;
	new_obj->lodged = NULL;
	new_obj->wounds = NULL;
	new_obj->equiped_by = NULL;
	new_obj->carried_by = NULL;
	new_obj->in_obj = NULL;

	if (!new_obj->item_wear)
		new_obj->item_wear = 100;

	new_obj->coldload_id = get_next_coldload_id (2);

	vtoo (new_obj->nVirtual)->instances++;

	return new_obj;
}

OBJ_DATA *
	load_colored_object (int vnum, char *color0, char *color1, char *color2, char *color3, char *color4, char *color5, char *color6, char *color7, char *color8, char *color9)
{
	OBJ_DATA *proto;
	OBJ_DATA *new_obj;
	WRITING_DATA *writing;
	int i;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *new_af;
	AFFECTED_TYPE *last_af = NULL;

	if (!(proto = vtoo (vnum)))
		return NULL;

	new_obj = new_object ();

	//memcpy (new_obj, proto, sizeof (OBJ_DATA));

	new_obj->deep_copy(proto);

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	for (af = proto->xaffected; af; af = af->next)
	{

		new_af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if (!new_obj->xaffected)
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->next = object_list;
	object_list = new_obj;

	new_obj->count = 1;

	if (new_obj->clock && new_obj->morphto)
		new_obj->morphTime = time (0) + new_obj->clock * 15 * 60;

	if ( USES_BOOK_CODE( new_obj ) )
	{
	    int writing_oval = GET_PAGE_OVAL( new_obj );

		if ( !new_obj->writing && new_obj->o.od.value[ writing_oval ] > 0 )
		{
			CREATE( new_obj->writing, WRITING_DATA, 1 );

			for ( i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[ writing_oval ]; i++ )
			{
				writing->message        = add_hash ("blank");
				writing->author         = add_hash ("blank");
				writing->date           = add_hash ("blank");
				writing->ink            = add_hash ("blank");
				writing->language       = 0;
				writing->script         = 0;
				writing->skill          = 0;
				writing->torn           = false;

				if ( i != new_obj->o.od.value[ writing_oval ] )
				{
					CREATE (writing->next_page, WRITING_DATA, 1);
					writing = writing->next_page;
				}
			}
		}
	}

	if (IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE) && *proto->full_description)
	{
		insert_string_variables (new_obj, proto, color0, color1, color2, color3, color4, color5, color6, color7, color8, color9);
		vo_specify(new_obj);

		if (get_scent(new_obj, scent_lookup("generic-variable")))
		{
			if (GET_ITEM_TYPE(new_obj) == ITEM_CHEM_SMOKE)
			{
				for (int ind = 0; ind < 10; ind++)
				{
					if (!str_cmp(new_obj->var_cat[ind], "$naturalscents") ||
						!str_cmp(new_obj->var_cat[ind], "$artificialscents"))
					{
						remove_obj_scent(new_obj, scent_lookup("generic-variable"));
						add_scent(new_obj, scent_lookup(new_obj->var_color[ind]), 1, new_obj->o.smoke.scent_power, 0, 0, 1);
					}
				}
			}
		}
	}

	if (GET_ITEM_TYPE (new_obj) == ITEM_WEAPON)
		new_obj->obj_flags.extra_flags |= ITEM_NEWSKILLS;

	if (GET_ITEM_TYPE (new_obj) == ITEM_CLUSTER)
	{
		if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_ONE_MINOR))
		{
			new_obj->o.od.value[3] += number(-2,2);
			MAX(new_obj->o.od.value[3], 1);
		}

		if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MAJOR))
		{
			new_obj->o.od.value[3] += number(-6,6);
			MAX(new_obj->o.od.value[3], 1);
		}
		else if (IS_SET(new_obj->o.od.value[1], CLUS_TWO_MINOR))
		{
			new_obj->o.od.value[5] += number(-2,2);
			MAX(new_obj->o.od.value[5], 1);
		}
	}

	new_obj->contains = NULL;
	new_obj->lodged = NULL;
	new_obj->wounds = NULL;
	new_obj->equiped_by = NULL;
	new_obj->carried_by = NULL;
	new_obj->in_obj = NULL;

	if (!new_obj->item_wear)
		new_obj->item_wear = 100;

	new_obj->coldload_id = get_next_coldload_id (2);

	vtoo (new_obj->nVirtual)->instances++;

	return new_obj;
}

OBJ_DATA *
	fread_object (int vnum, int nZone, FILE * fp)
{
	OBJ_DATA *obj;
	float tmpf = 0;
	int tmp;
	char chk[50];
	char buf[MAX_STRING_LENGTH];
	EXTRA_DESCR_DATA *new_descr;
	EXTRA_DESCR_DATA *tmp_descr;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *taf;
	OBJ_CLAN_DATA *newclan = NULL;
	SCENT_DATA *scent;
	SCENT_DATA *xscent;
	TRAP_DATA *trap;
	char peak_char;
	extern char *null_string;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = vnum;
	obj->zone = nZone;

#if CHECK_DOUBLE_DEFS
	if (vtoo (vnum))
	{
		sprintf (buf, "OBJ %d multiply defined!!", vnum);
		system_log (buf, true);
	}
	else
#endif
		add_obj_to_hash (obj);

	object_load_cues(obj);

	obj->name = fread_string (fp);
	obj->short_description = fread_string (fp);
	obj->description = fread_string (fp);
	obj->full_description = fread_string (fp);



	if (!strcmp (obj->full_description, "(null)"))
	{
		sprintf (buf, "NOTE:  Object %d with '(null)' full description fixed.",
			obj->nVirtual);
		system_log (buf, true);
		obj->full_description = null_string;
	}

	/* *** numeric data *** */

	fscanf (fp, " %d ", &tmp);
	obj->obj_flags.type_flag = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->obj_flags.extra_flags = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->obj_flags.wear_flags = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->o.od.value[0] = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->o.od.value[1] = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->o.od.value[2] = tmp;
	fscanf (fp, " %d ", &tmp);
	obj->o.od.value[3] = tmp;

	fscanf (fp, " %d ", &tmp);	/* Weight */

	if (world_version_in == 4)
		obj->obj_flags.weight = tmp * 100;
	else
		obj->obj_flags.weight = tmp;

	fscanf (fp, " %f\n", &tmpf);
	obj->silver = tmpf;		/* Changed to silver from cost */

	fscanf (fp, " %d ", &tmp);
	obj->o.od.value[4] = tmp;

	fscanf (fp, " %d ", &tmp);
	obj->room_pos = tmp;

	if (world_version_in == 6)
	{
		fscanf (fp, " %d", &obj->o.od.value[5]);
		if (obj->obj_flags.type_flag == ITEM_WORN ||
			obj->obj_flags.type_flag == ITEM_ARMOR)
			obj->o.od.value[5] = 0;
	}

	else
	{
		if (obj->obj_flags.type_flag == ITEM_CLUSTER  || obj->obj_flags.type_flag == ITEM_TOOL)
		{
			fscanf (fp, " %d", &obj->o.od.value[5]);
			obj->ink_color = fread_string (fp);
		}
		else if (obj->obj_flags.type_flag == ITEM_INK || obj->obj_flags.type_flag == ITEM_FOOD  || obj->obj_flags.type_flag == ITEM_FLUID)
			obj->ink_color = fread_string (fp);
		else if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
			obj->obj_flags.type_flag == ITEM_WORN)
			obj->desc_keys = fread_string (fp);

		else if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
			obj->obj_flags.type_flag == ITEM_ARMOR)
			obj->desc_keys = fread_string (fp);

		else if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
			obj->obj_flags.type_flag == ITEM_CONTAINER)
			obj->desc_keys = fread_string (fp);

		else if (obj->obj_flags.type_flag == ITEM_TOSSABLE)
			obj->desc_keys = fread_string (fp);

		else
		{
			fscanf (fp, " %d", &obj->o.od.value[5]);
		}
	}

	fscanf (fp, " %d %d %d %d %d\n",
		&obj->activation,
		&obj->quality, &obj->econ_flags, &obj->size, &obj->count);

	if (world_version_out >= 11)
	{
		fscanf (fp, "%f %d %d %d %d %d %d\n",
			&obj->farthings,
			&obj->clock,
			&obj->morphto, &obj->item_wear, &obj->material, &tmp, &tmp);
	}

	if (GET_ITEM_TYPE (obj) == ITEM_INK)
	{
		obj->clock = 0;
		obj->morphto = 0;
	}

	/* *** extra descriptions *** */

	obj->ex_description = 0;
	obj->wdesc = 0;

	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'E')
			break;

		fscanf (fp, " %s \n", chk);

		new_descr =
			(struct extra_descr_data *)
			get_perm (sizeof (struct extra_descr_data));

		new_descr->keyword = fread_string (fp);
		new_descr->description = fread_string (fp);

		/* Add descr's in same order as read so that they
		can get written back in same order */

		new_descr->next = NULL;

		if (!obj->ex_description)
			obj->ex_description = new_descr;
		else
		{
			tmp_descr = obj->ex_description;
			while (tmp_descr->next)
				tmp_descr = tmp_descr->next;
			tmp_descr->next = new_descr;
		}
	}
	while (1);

	tmp = 0;

	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'A')
			break;

		fscanf (fp, " %s \n", chk);

		af = (AFFECTED_TYPE *) get_perm (sizeof (AFFECTED_TYPE));

		af->type = 0;
		af->a.spell.duration = -1;
		af->a.spell.bitvector = 0;
		af->a.spell.sn = 0;
		af->next = NULL;

		fscanf (fp, " %d %d\n", &af->a.spell.location, &af->a.spell.modifier);

		if (af->a.spell.location || af->a.spell.modifier)
		{

			tmp++;

			if (!obj->xaffected)
				obj->xaffected = af;
			else
			{
				for (taf = obj->xaffected; taf->next; taf = taf->next)
					;
				taf->next = af;
			}
		}

	}
	while (1);

	// One clan per object
	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'C')
			break;

		fscanf (fp, "%s \n", chk);

		CREATE (newclan, OBJ_CLAN_DATA, 1);
		newclan->name = fread_string (fp);
		newclan->rank = fread_string (fp);
		newclan->next = NULL;

		tmp++;

		if (!obj->clan_data)
			obj->clan_data = newclan;
	}
	while (1);


	tmp = 0;

	// Up to ten variable per object
	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'V')
			break;

		fscanf (fp, "%s \n", chk);

		obj->var_color[tmp] = fread_string (fp);
		obj->var_cat[tmp] = fread_string (fp);

		tmp++;

	}
	while (1);




	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'S')
			break;

		fscanf (fp, " %s \n", chk);

		//CREATE (poison, POISON_DATA, 1);


		scent = (SCENT_DATA *) get_perm (sizeof (SCENT_DATA));

		scent->scent_ref = 0;

		scent->permanent = 0;

		scent->atm_power = 0;
		scent->pot_power = 0;
		scent->rat_power = 0;

		scent->next = NULL;

		fscanf(fp, "%d %d %d %d %d\n",
			&scent->scent_ref, &scent->permanent, &scent->atm_power, &scent->pot_power, &scent->rat_power);

		if (scent->scent_ref)
		{

			tmp++;

			if (!obj->scent)
				obj->scent = scent;
			else
			{
				for (xscent = obj->scent; xscent->next; xscent = xscent->next)
					;
				xscent->next = scent;
			}
		}

	}
	while (1);

	do
	{
		while ((peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
			peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if (peak_char != 'T')
			break;

		fscanf (fp, " %s \n", chk);

		trap = (TRAP_DATA *) get_perm (sizeof (TRAP_DATA));

		fscanf(fp, "%d %d %d %d %d "
				   "%d %d %d %d "
				   "%d %d %d %d %d %d "
				   "%d %d %d %d %d "
				   "%d %d %d %d %d "
				   "%d %d %d %d\n",
			&trap->trap_bit, &trap->com_diff, &trap->com_cost[0], &trap->com_target_dice, &trap->com_target_side,
			&trap->com_uses, &trap->com_junk1, &trap->com_junk2, &trap->com_shots,
			&trap->com_total_shots, &trap->com_trig_shots, &trap->com_skill, &trap->avoid_atr, &trap->avoid_dice, &trap->avoid_sides,
			&trap->dam1_dice, &trap->dam1_sides, &trap->dam1_type, &trap->dam1_loc, &trap->dam1_bonus,
			&trap->dam2_dice, &trap->dam2_sides, &trap->dam2_type, &trap->dam2_loc, &trap->dam2_bonus,
			&trap->bind_atr, &trap->bind_str, &trap->dam1_times, &trap->dam2_times);

		trap->trigger_1st = fread_string (fp);
		trap->trigger_3rd = fread_string (fp);
		trap->strike_1st = fread_string (fp);
		trap->strike_3rd = fread_string (fp);
		trap->shout_1 = fread_string (fp);
		trap->shout_2 = fread_string (fp);

		if (!str_cmp ("(null)", trap->trigger_1st))
			trap->trigger_1st = NULL;
		if (!str_cmp ("(null)", trap->trigger_3rd))
			trap->trigger_1st = NULL;
		if (!str_cmp ("(null)", trap->strike_1st))
			trap->strike_1st = NULL;
		if (!str_cmp ("(null)", trap->strike_3rd))
			trap->strike_3rd = NULL;
		if (!str_cmp ("(null)", trap->shout_1))
			trap->shout_1 = NULL;
		if (!str_cmp ("(null)", trap->shout_2))
			trap->shout_2 = NULL;

		if (trap->trap_bit)
		{
			tmp++;
			obj->trap = trap;
		}


	}
	while (1);


	if (tmp > 20)
		printf ("Object %d has %d affects\n", obj->nVirtual, tmp);

	obj->in_room = NOWHERE;
	obj->next_content = 0;
	obj->carried_by = 0;
	obj->equiped_by = 0;
	obj->in_obj = 0;
	obj->contains = 0;

	if (obj->count == 0)
		obj->count = 1;

	if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHIELD))
		obj->obj_flags.wear_flags &= ~ITEM_WEAR_SHIELD;

	obj->creater = fread_string (fp);
	obj->editer = fread_string (fp);


	if (!engine.in_play_mode ())
		do_object_standards(NULL, obj, 1);

	return obj;
}

int
	calculate_race_height (CHAR_DATA * tch)
{
	int min = 0, max = 0;
	float percentile = 0.0;

	if (tch->mob && tch->mob->min_height && tch->mob->max_height)
	{
		min = tch->mob->min_height;
		max = tch->mob->max_height;
		if (max < min)
			max += number (1, 10);
	}
	else if (!lookup_race_variable (tch->race, RACE_MIN_HEIGHT)
		|| !lookup_race_variable (tch->race, RACE_MAX_HEIGHT))
		return -1;
	else
	{
		min = atoi (lookup_race_variable (tch->race, RACE_MIN_HEIGHT));
		max = atoi (lookup_race_variable (tch->race, RACE_MAX_HEIGHT));
	}

	if (!IS_NPC (tch))
	{
		if (tch->height == 1)	// V. short
			percentile = (float) (number (1, 10) / 100.0);
		else if (tch->height == 2)	// short
			percentile = (float) (number (11, 30) / 100.0);
		else if (tch->height == 3)	// average
			percentile = (float) (number (31, 69) / 100.0);
		else if (tch->height == 4)	// tall
			percentile = (float) (number (70, 89) / 100.0);
		else
			percentile = (float) (number (90, 100) / 100.0);	// v. tall
	}
	else
		percentile = (float) (number (25, 85) / 100.0);

	return ((int) (min + ((max - min) * percentile)));
}

int
	calculate_size_height (CHAR_DATA * tch)
{
	if (tch->size == -3)		// XXS  (insect)
		return (number (1, 3));
	else if (tch->size == -2)	// XS   (rodent, rabbit, etc.)
		return (number (3, 7));
	else if (tch->size == -1)
	{
		// S    (small humanoid, dog, cat, etc.)
		if (tch->body_proto == PROTO_HUMANOID)
			return (number (36, 60));
		else
			return (number (36, 54));
	}
	else if (tch->size == 0)
	{
		// M    (average humanoid, livestock, etc.)
		if (tch->body_proto == PROTO_HUMANOID)
			return (number (58, 70));
		else
			return (number (36, 54));
	}
	else if (tch->size == 1)	// L    (large humanoid, troll)
		return (number (100, 120));
	else if (tch->size == 2)	// XL   (ent, giant)
		return (number (180, 240));
	else				// XXL  (dragon)
		return (number (300, 480));
}

void
	make_height (CHAR_DATA * mob)
{
	if ((mob->height = calculate_race_height (mob)) == -1)
		mob->height = calculate_size_height (mob);

	return;
}

void
	make_frame (CHAR_DATA * mob)
{
	if (!lookup_race_variable (mob->race, RACE_ID))
	{
		mob->frame = 3;
		return;
	}

	if (mob->sex == SEX_MALE)
		mob->frame = 3 + number (-1, 3);
	else
		mob->frame = 3 + number (-3, 1);
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void
	reset_zone (int zone)
{
	int cmd_no;
	int count_vnum_in_room;
	int i;
	int current_room = -1;
	CHAR_DATA *mob = NULL;
	CHAR_DATA *tmob = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj;
	SUBCRAFT_HEAD_DATA *craft;
	AFFECTED_TYPE *af;
	RESET_AFFECT *ra;
	RESET_DATA *reset;
	char buf[MAX_STRING_LENGTH];

	extern bool bIsCopyOver;	// Defined in comm.c

	if (!zone_table[zone].cmd)
		return;

	for (cmd_no = 0;; cmd_no++)
	{

		if (ZCMD.command == 'S')
			break;

		if (ZCMD.command == 'M')
		{
			/* Mob to room */

			mob = NULL;

			if (!ZCMD.enabled)
				continue;

			ZCMD.enabled = 0;

			if ( /*(!engine.in_play_mode ()) && */ (mob = load_mobile (ZCMD.arg1)))
			{
				mob->mob->reset_zone = zone;
				mob->mob->reset_cmd = cmd_no;

				if (!mob->height && !IS_SET (mob->flags, FLAG_VARIABLE))
					make_height (mob);

				if (!mob->frame)
					make_frame (mob);

				mob->mob->spawnpoint = ZCMD.arg3;
				char_to_room (mob, ZCMD.arg3);

				if (ZCMD.arg4 && (tmob = load_mobile (ZCMD.arg4)))
				{
					tmob->mount = mob;
					mob->mount = tmob;
					tmob->mob->spawnpoint = ZCMD.arg3;
					char_to_room (tmob, ZCMD.arg3);
				}
			}
			else if (!bIsCopyOver)
			{
				sprintf (buf, "Unable to load mob virtual %d!", ZCMD.arg1);
				system_log (buf, true);
			}
		}

		else if (ZCMD.command == 'R')
		{
			/* Defining room */
			current_room = ZCMD.arg1;
			continue;
		}

		else if (ZCMD.command == 'A' ||	/* Affect on char */
			ZCMD.command == 'r')
		{
			/* Affect on room */

			if (!ZCMD.arg1)
			{
				system_log ("ZCMD is zero.", true);
				continue;
			}

			if (!mob)
				continue;

			ra = (RESET_AFFECT *) ZCMD.arg1;

			if (get_affect (mob, ra->type))
				continue;

			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

			af->type = ra->type;
			af->a.spell.duration = ra->duration;
			af->a.spell.modifier = ra->modifier;
			af->a.spell.location = ra->location;
			af->a.spell.bitvector = ra->bitvector;
			af->a.spell.sn = ra->sn;
			af->a.spell.t = ra->t;
			af->next = NULL;

			if (ZCMD.command == 'r')
			{
				af->next = vnum_to_room (current_room)->affects;
				vnum_to_room (current_room)->affects = af;
			}
			else
				affect_to_char (mob, af);

			continue;
		}

		else if (ZCMD.command == 'm')
		{

			if (!mob)
				continue;

			if (ZCMD.arg1 == RESET_REPLY)
			{
				reset = (RESET_DATA *) alloc (sizeof (RESET_DATA), 33);

				reset->type = RESET_REPLY;

				reset->command = str_dup ((char *) ZCMD.arg2);

				reset->when.month = -1;
				reset->when.day = -1;
				reset->when.hour = -1;
				reset->when.minute = -1;
				reset->when.second = -1;

				reset_insert (mob, reset);
			}
		}

		else if (ZCMD.command == 'C')
		{

			if (!mob)
				continue;

			if (!ZCMD.arg1)
				continue;

			for (craft = crafts;
				craft && str_cmp (craft->subcraft_name, (char *) ZCMD.arg1);
				craft = craft->next)
				;

			if (!craft)
			{

				sprintf (buf, "RESET: No such craft %s on mob %d, room %d",
					craft->subcraft_name, mob->mob->vnum,
					mob->in_room);

				system_log (buf, true);
			}

			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
				if (!get_affect (mob, i))
					break;

			magic_add_affect (mob, i, -1, 0, 0, 0, 0);

			af = get_affect (mob, i);

			af->a.craft =
				(struct affect_craft_type *)
				alloc (sizeof (struct affect_craft_type), 23);

			af->a.craft->subcraft = craft;
		}

		else if (ZCMD.command == 'O')
		{

			obj = NULL;

			count_vnum_in_room = 0;

			for (tobj = vnum_to_room (ZCMD.arg3)->contents;
				tobj; tobj = tobj->next_content)
				if (tobj->nVirtual == ZCMD.arg1)
					count_vnum_in_room++;

			if (count_vnum_in_room < ZCMD.arg2 &&
				(obj = load_object (ZCMD.arg1)))
				obj_to_room (obj, ZCMD.arg3);
		}

		else if (ZCMD.command == 'P')
		{
			if (!obj)
				continue;

			if ((tobj = load_object (ZCMD.arg1)))
				obj_to_obj (tobj, obj);
		}

		else if (ZCMD.command == 'G')
		{

			obj = NULL;

			if (!mob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
			{

				if (obj->nVirtual == VNUM_PENNY)
				{
					obj->count = ZCMD.arg2;
					name_money (obj);
				}

				if (obj->nVirtual == VNUM_PENNY &&
					IS_SET (mob->flags, FLAG_KEEPER))
				{
					extract_obj (obj);
					obj = NULL;
				}
				else
					obj_to_char (obj, mob);
			}
		}

		else if (ZCMD.command == 'E')
		{

			obj = NULL;

			if (!mob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
			{

				if (IS_WEARABLE (obj))
					obj->size = get_size (mob);

				equip_char (mob, obj, ZCMD.arg3);

				cmd_no++;
				if (ZCMD.command == 's')
				{
					obj_to_obj (load_object (ZCMD.arg1), obj);
					tobj = load_object (ZCMD.arg2);
					if (ZCMD.arg2 > 1)
						for (i = 1; i < ZCMD.arg2; i++)
							obj_to_obj (load_object (ZCMD.arg1), obj);
				}
				else
					cmd_no--;

			}
		}

		else if (ZCMD.command == 'a')
		{

			obj = NULL;

			if (!tmob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
			{

				if (IS_WEARABLE (obj))
					obj->size = get_size (tmob);

				equip_char (tmob, obj, ZCMD.arg3);

				cmd_no++;
				if (ZCMD.command == 's')
				{
					obj_to_obj (load_object (ZCMD.arg1), obj);
					tobj = load_object (ZCMD.arg1);
					if (ZCMD.arg2 > 1)
						for (i = 1; i < ZCMD.arg2; i++)
							obj_to_obj (load_object (ZCMD.arg1), obj);
				}
				else
					cmd_no--;
			}
		}

		else if (ZCMD.command == 'D')
		{
			set_door_state (ZCMD.arg1, ZCMD.arg2, (exit_state) ZCMD.arg3);
		}
	}				/* for */

	zone_table[zone].age = 0;
}

#undef ZCMD

void
	list_validate (char *name)
{
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	int cycle_count;
	char buf[MAX_STRING_LENGTH];

	sprintf (buf, "List validate:  %s entered.\n", name);
	system_log (buf, false);

	for (ch = character_list, cycle_count = 0; ch; ch = ch->next)
	{
		if (cycle_count++ > 10000)
		{
			system_log ("Character list cycle failed.", true);
			((int *) 0)[-1] = 0;
		}
	}

	for (obj = object_list, cycle_count = 0; obj; obj = obj->next)
	{
		if (cycle_count++ > 10000)
		{
			system_log ("Object list cycle failed.", true);
			((int *) 0)[-1] = 0;
		}
	}

	sprintf (buf, "List validate:  %s completed.\n", name);
	system_log (buf, false);
}

void
	cleanup_the_dead (int mode)
{
	OBJ_DATA *obj;
	OBJ_DATA *next_obj = NULL;
	OBJ_DATA *prev_obj = NULL;
	CHAR_DATA *ch;
	CHAR_DATA *next_ch;
	CHAR_DATA *prev_ch = NULL;

	if (mode == 1 || mode == 0)
	{
		for (ch = character_list; ch; ch = next_ch)
		{

			next_ch = ch->next;

			if (!ch->deleted)
			{
				prev_ch = ch;
				continue;
			}

			if (ch == character_list)
				character_list = next_ch;
			else
				prev_ch->next = next_ch;

			if (!IS_NPC (ch))
			{
				unload_pc (ch);
				continue;
			}

			free_char (ch);

			ch = NULL;
		}
	}

	if (mode == 2 || mode == 0)
	{
		for (obj = object_list; obj; obj = next_obj)
		{

			next_obj = obj->next;

			if (!obj->deleted)
			{
				prev_obj = obj;
				continue;
			}

			if (obj == object_list)
				object_list = next_obj;
			else
				prev_obj->next = next_obj;

			if (obj)
				free_obj (obj);

			obj = NULL;
		}
	}
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void
	refresh_zone (void)
{
	int cmd_no, i;
	int count_vnum_in_room;
	static int zone = 0;
	CHAR_DATA *mob = NULL, *tmob = NULL;
	ROOM_DATA *room;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH];

	if (!zone_table[zone].cmd)
		return;

	if (IS_FROZEN (zone))
		return;

	sprintf (buf, "Refreshing zone %d and loading any unloaded psaves...",
		zone);
	system_log (buf, false);

	for (room = full_room_list; room; room = room->lnext)
	{
		if (room->zone != zone)
			continue;
		if (!room->psave_loaded)
			load_save_room (room);
	}

	for (cmd_no = 0;; cmd_no++)
	{

		if (ZCMD.command == 'S')
			break;

		if (ZCMD.command == 'D')
			continue;

		if (ZCMD.command == 'M')
		{

			mob = NULL;

			if (!ZCMD.enabled)
				continue;

			ZCMD.enabled = 0;

			if ((mob = load_mobile (ZCMD.arg1)))
			{
				mob->mob->reset_zone = zone;
				mob->mob->reset_cmd = cmd_no;
				char_to_room (mob, ZCMD.arg3);
			}

			/*			act ("$n has arrived.", true, mob, 0, 0, TO_ROOM | _ACT_FORMAT);*/

			if (ZCMD.arg4 && (tmob = load_mobile (ZCMD.arg4)))
			{
				tmob->mount = mob;
				mob->mount = tmob;
				tmob->mob->spawnpoint = ZCMD.arg3;
				char_to_room (tmob, ZCMD.arg3);
			}
		}

		else if (ZCMD.command == 'O')
		{

			obj = NULL;

			count_vnum_in_room = 0;

			for (tobj = vnum_to_room (ZCMD.arg3)->contents;
				tobj; tobj = tobj->next_content)
				if (tobj->nVirtual == ZCMD.arg1)
					count_vnum_in_room++;

			if (count_vnum_in_room < ZCMD.arg2 &&
				(obj = load_object (ZCMD.arg1)))
				obj_to_room (obj, ZCMD.arg3);
		}

		else if (ZCMD.command == 'P')
		{
			if (!obj)
				continue;

			if ((tobj = load_object (ZCMD.arg1)))
				obj_to_obj (tobj, obj);
		}

		else if (ZCMD.command == 'G')
		{

			obj = NULL;

			if (!mob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
			{
				if (obj->nVirtual == VNUM_MONEY)
				{
					obj->o.od.value[0] = ZCMD.arg2;
					obj->obj_flags.set_cost = ZCMD.arg2; // ??
					obj->obj_flags.weight = ZCMD.arg2;
					name_money (obj);
				}
				obj_to_char (obj, mob);
			}
		}

		else if (ZCMD.command == 'E')
		{

			obj = NULL;

			if (!mob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
				equip_char (mob, obj, ZCMD.arg3);
		}

		else if (ZCMD.command == 'a')
		{

			obj = NULL;

			if (!tmob)
				continue;

			if ((obj = load_object (ZCMD.arg1)))
			{

				if (IS_WEARABLE (obj))
					obj->size = get_size (tmob);

				equip_char (tmob, obj, ZCMD.arg3);

				cmd_no++;
				if (ZCMD.command == 's')
				{
					obj_to_obj (load_object (ZCMD.arg1), obj);
					tobj = load_object (ZCMD.arg1);
					if (ZCMD.arg2 > 1)
						for (i = 1; i < ZCMD.arg2; i++)
							obj_to_obj (load_object (ZCMD.arg1), obj);
				}
				else
					cmd_no--;
			}
		}

	}				/* for */

	if (zone + 1 >= MAX_ZONE)
		zone = 0;
	else
		zone++;
}

#undef ZCMD
