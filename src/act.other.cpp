/*------------------------------------------------------------------------\
|  act.other.c : Miscellaneous Module                 www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
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

extern rpie::server engine;

//std::vector<second_affect*> second_affect_vector;
SECOND_AFFECT *second_affect_list = NULL;
extern int keeper_has_money (CHAR_DATA * keeper, int cost);
extern void keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money);
extern void subtract_keeper_money (CHAR_DATA * keeper, int cost);

extern std::multimap<int, variable_data> obj_variable_list;
extern std::multimap<int, std::string> variable_categories;
extern std::vector<monitor_data*> monitor_list;

void
	do_test (CHAR_DATA * ch, char *argument, int cmd)
{
	//int dir = -1;
	//int target_room = 53001;
	//int num = 0;
	//int from_room = ch->in_room;
	//
	//ROOM_DATA *temp_room = NULL;

	//while (from_room != target_room)
	//{
	//	num++;
	//	temp_room = vtor(from_room);

	//	dir = pathfind (from_room, target_room);

	//	if (dir == -1)
	//	{
	//		send_to_char("No path could be found.\n", ch);
	//		break;
	//	}

 //       if (!temp_room->dir_option[dir])
	//	{
	//		send_to_char("No path could be found.\n", ch);
	//		break;
	//	}

	//	if (!vtor(from_room = temp_room->dir_option[dir]->to_room))
	//	{
	//		send_to_char("No path could be found.\n", ch);
	//		break;
	//	}
	//}

	//sprintf(buf, "There are %d rooms between you and room 66001.\n", num);
	//send_to_char(buf, ch);


	//int true_hour = time_info.hour;
	//int true_day = time_info.day;

	//time_info.hour = 0;
	//time_info.day = 0;


	//time_info.hour = true_hour;
	//time_info.day = true_day;

	/*
	ROOM_DATA *room = NULL;
	extern rpie::server engine;
	MYSQL_RES *result = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *next_o = NULL;
	std::string player_db = engine.get_config ("player_db");

	for (int ind = 101500; ind < 102000; ind++)
	{
		if ((room = vtor(ind)))
		{
			if (!str_cmp(room->name, "The Pre-Commencement Room"))
			{
				mysql_safe_query ("SELECT * FROM %s.pfiles WHERE room = %d", player_db.c_str (), room->nVirtual);

				if (!(result = mysql_store_result (database)) || !mysql_num_rows (result))
				{
					for (obj = vtor (ch->in_room)->contents; obj; obj = next_o)
					{
						next_o = obj->next_content;
						extract_obj (obj);
					}

					if (rdelete (room))
					{
						send_to_gods("Room Wiped!");
					}
					else
					{
						send_to_gods("Suitable room found, not wiped.");
					}
				}
				else
				{
					send_to_gods("Player found, not wiping.");
				}

				if (result)
				{
					mysql_free_result (result);
					result = NULL;
				}
			}
		}
	}
	*/
}

int
is_in_cell (CHAR_DATA * ch, int zone)
{
    CHAR_DATA *jailer;

    if (!zone_table[zone].jailer)
        return 0;

    jailer = vnum_to_mob (zone_table[zone].jailer);

    if (!jailer)
        return 0;

    if (ch->in_room == jailer->cell_1)
        return 1;
    else if (ch->in_room == jailer->cell_2)
        return 1;
    else if (ch->in_room == jailer->cell_3)
        return 1;

    return 0;
}

void
do_doitanyway (CHAR_DATA *ch, char * argument, int command)
{
    if (!GET_TRUST(ch) && !IS_NPC(ch) && !command)
    {
        send_to_char("You are not permitted to use this command.\n", ch);
        return;
    }

    add_second_affect (SA_DOANYWAY, 1, ch, NULL, NULL, 0);
    command_interpreter(ch, argument);
    return;
}



// MSP and MXP compatibility to go here.

#include <arpa/telnet.h>

#define MSP 90

char msp_sound_on[] = { (char) IAC, (char) WILL, (char) MSP, '\0' };
char msp_sound_off[] = { (char) IAC, (char) WONT, (char) MSP, '\0' };
char msp_sound_ok[] = { (char) IAC, (char) DO, (char) MSP, '\0' };
char msp_sound_no[] = { (char) IAC, (char) DONT, (char) MSP, '\0' };

#define MSP_ON		SEND_TO_Q (msp_sound_on, d);
#define MSP_OFF		SEND_TO_Q (msp_sound_off, d);
#define MSP_OK		SEND_TO_Q (msp_sound_ok, d);
#define MSP_NO		SEND_TO_Q (msp_sound_no, d);

int
sound_to_descriptor (DESCRIPTOR_DATA * d, const char *txt)
{
    int thisround;
    int total;

    total = strlen (txt);

    MSP_ON;
    MSP_OK;

    thisround = write (d->hSocketFD, txt, total);
    return 0;
}

void
do_music (CHAR_DATA *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *tch;
    DESCRIPTOR_DATA *d;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->sound)
        {
            d = tch->descr();
            sprintf (buf, "\n!!MUSIC(%s)\0", argument);
            sound_to_descriptor(d, buf);
        }
    }
}

void
do_sound (CHAR_DATA *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *tch;
    DESCRIPTOR_DATA *d;
    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->sound)
        {
            d = tch->descr();
            sprintf (buf, "\n!!SOUND(%s)\0", argument);
            sound_to_descriptor(d, buf);
        }
    }
}


// combine request - gives you a crate
// combine test - shows how much of any one material you could get
// combine test <number> <option> - shows how much of all other materials you could get after making that much
// combine return - gives back the crate
// combine option - gives you the options this firebreather can make
// combine produce <number> <option> - makes the goods, puts them in your crate.

void
do_combine (CHAR_DATA *ch, char *argument, int cmd)
{
    OBJ_DATA *frBre = NULL;
    char arg[MAX_STRING_LENGTH] = { '\0' };
    char arg2[MAX_STRING_LENGTH] = { '\0' };
    char arg3[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
	#define AMOUNT_MAX 17
    int amounts[AMOUNT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int conversion[AMOUNT_MAX] = {2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 1000, 2000, 2000, 1000};
	int counts[AMOUNT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tobj = NULL;
	OBJ_DATA *robj = NULL;
	OBJ_DATA *next_tobj = NULL;
    int weight_mod = 0;
    int quality_mod = 0;

    argument = one_argument(argument, arg);

    if (!ch->room || !ch->in_room)
    {
        send_to_char("Oddly enough, you don't appear to be in a room.\n", ch);
        return;
    }

    for (frBre = ch->room->contents; frBre; frBre = frBre->next_content)
    {
        if (GET_ITEM_TYPE(frBre) == ITEM_BREATHER)
        {
            break;
        }
    }

    if (!frBre)
    {
        send_to_char("There isn't anything here that can be used to combine goods.\n", ch);
        return;
    }

    if (!*arg)
    {
        sprintf(buf,               "combine usage: combine <request|test|return|produce|option> <option> <color>\n");
        sprintf(buf + strlen(buf), "combine request                  - produces a crate for you to place your goods\n");
        sprintf(buf + strlen(buf), "combine test                     - shows how many of each option your crate will produce\n");
        sprintf(buf + strlen(buf), "combine return                   - returns the crate (requires it to be empty)\n");
        sprintf(buf + strlen(buf), "combine option                   - shows what can be produced (for polymesh and polyprop)\n");
        sprintf(buf + strlen(buf), "combine produce <option> <color> - recycles goods as per what is in your crate, for a certan color.\n");
		sprintf(buf + strlen(buf), "                                   For available colors, type #6tag color#0.\n");
        send_to_char(buf, ch);
        return;
    }

	// Manual increase to make food a bit harder to produce.
	if (frBre->o.od.value[5] == 1)
	{
		conversion[0] = 2500;
	}


    for (obj = ch->room->contents; obj; obj = obj->next_content)
    {
        if (GET_ITEM_TYPE(obj) == ITEM_CRATE)
        {
            if (obj->o.od.value[4] == ch->coldload_id)
            {
                break;
            }
        }
    }

    if (!str_cmp(arg, "request"))
    {
        if (obj)
        {
            act ("But you already have requested $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if ((obj = load_object(frBre->o.od.value[4])))
        {
            obj_to_room(obj, ch->in_room);
            act ("You retrieve $p from the room.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n retrieves $p from the room..", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
            obj->o.od.value[4] = ch->coldload_id;
            return;
        }
        else
        {
            send_to_char("An error occured, and your crate could not be loaded. Please let an admin know.\n", ch);
            return;
        }
    }
    else if (!str_cmp(arg, "return"))
    {
        if (!obj)
        {
            act ("But you have not requested a crate.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (obj->contains)
        {
            act ("You must empty $p before returning it.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }
        else
        {
            act ("You return $p to its original place.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            extract_obj(obj);
            return;
        }
    }
    else if (!str_cmp(arg, "option"))
    {
        switch (frBre->o.od.value[5])
        {
            // cloth firebreather
        case 2:
            sprintf(buf,               "You can combine polymesh scraps to produce the following bolts of fabric:\n");
            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
            {
                if (!str_cmp(it->second.category, "$polymesh"))
                {
                    sprintf(buf + strlen(buf), "   #6%s#0\n", it->second.shorts);
                }
            }
            break;
            // plastic firebreather
        case 3:
            sprintf(buf,               "You can combine polyprop scraps to produce the following bars of polyprop:\n");
			sprintf(buf + strlen(buf), "   #6thick#0:   thick and dense polyprop giving of servicable quality\n");
			sprintf(buf + strlen(buf), "   #6uneven#0:  standard density polyprop of irregular color and solidity\n");
			sprintf(buf + strlen(buf), "   #6fragile#0: light polyprop but pocked and fragile\n");
            break;
		case 4:
            sprintf(buf,               "You can combine discarded bullets to produce the following blocks of gold:\n");
			sprintf(buf + strlen(buf), "   #6six#0:      an iron-and-gold alloy suited to making armor-piercing bullets\n");
			sprintf(buf + strlen(buf), "   #6twelve#0:   an iron-and-gold alloy suited to making jacketed bullets\n");
			sprintf(buf + strlen(buf), "   #6eighteen#0: an iron-and-gold alloy suited to making hollow-tipped bullets\n");
			sprintf(buf + strlen(buf), "   #6solid#0:    a solid gold brick to use for trade or making bullets\n");
            break;
        case 1:
            sprintf(buf,               "Most organic manner can be recycled at this firebreather,\nsuch as corpses or food mass.\n");
            break;
        default:
            sprintf(buf,               "Ironalloy scrap fed in to the firebreather will produce a\nfresh ingot of that same alloy.\n");

            break;
        }

        send_to_char(buf, ch);
    }


    else if (!str_cmp(arg, "test") || !str_cmp(arg, "produce"))
    {
        int option = -1;
		int color = 0;
		bool produce = false;

		if (!str_cmp(arg, "produce"))
		{
			produce = true;
		}

        if (!obj)
        {
            act ("You need to have requested a crate before you can do that.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (!obj->contains)
        {
            act ("There must be something in $p before you can do that.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }

		argument = one_argument(argument, arg2);
		argument = one_argument(argument, arg3);

		if (frBre->o.od.value[5] == 3)
		{
			if (produce)
			{
				if (!str_cmp(arg2, "thick"))
				{
					option = 4;
				}
				else if (!str_cmp(arg2, "uneven"))
				{
					option = 5;
				}
				else if (!str_cmp(arg2, "fragile"))
				{
					option = 6;
				}
				else
				{
					send_to_char("You need to specify an option. Type #6combine option#0 to see your choices.\n", ch);
					return;
				}
			}

			conversion[4] = 3200;
			conversion[5] = 2400;
			conversion[6] = 1800;

			if (*arg3)
			{
				if (!(color = vc_exists(arg3, "$color")))
				{
					send_to_char("That isn't a color that can be produced at the Firebreather. Type #6tags color#0 to see your options.", ch);
					return;
				}
			}
			else
			{
				sprintf(arg3, "off-white");
			}
		}
		else if (frBre->o.od.value[5] == 4)
		{
			if (produce)
			{
				if (!str_cmp(arg2, "solid"))
				{
					option = 1;
				}
				else if (!str_cmp(arg2, "six"))
				{
					option = 2;
				}
				else if (!str_cmp(arg2, "twelve"))
				{
					option = 3;
				}
				else if (!str_cmp(arg2, "eighteen"))
				{
					option = 4;
				}
				else
				{
					send_to_char("You need to specify an option. Type #6combine option#0 to see your choices.\n", ch);
					return;
				}
			}

			conversion[1] = 7000;
			conversion[2] = 600;
			conversion[3] = 800;
			conversion[4] = 1000;
		}
		else if (frBre->o.od.value[5] == 2)
		{
			if (produce)
			{
				if (!(option = vc_exists(arg2, "$polymesh")))
				{
					send_to_char("You need to specify an option. Type #6combine option#0 to see your choices.\n", ch);
					return;
				}
			}

			conversion[1] = 3000;
			conversion[2] = 2000;
			conversion[3] = 2000;
			conversion[4] = 3000;
			conversion[5] = 3000;
			conversion[6] = 2000;
			conversion[7] = 4000;
			conversion[8] = 2000;
			conversion[9] = 2000;
			conversion[10] = 2000;
			conversion[11] = 4500;
			conversion[12] = 4000;
			conversion[13] = 2000;
			conversion[14] = 2000;
			conversion[15] = 3000;
			conversion[16] = 4000;

			if (*arg3)
			{
				if (!(color = vc_exists(arg3, "$color")))
				{
					send_to_char("That isn't a color that can be produced at the Firebreather. Type #6tags color#0 to see your options.", ch);
					return;
				}
			}
			else
			{
				sprintf(arg3, "natural-hued");
			}

		}
		else if (frBre->o.od.value[5] == 1)
		{
			option = 0;
		}

        // Fairly simple process: we run through everything in our crate,
        // and add their totals to our amounts tally.
        for (tobj = obj->contains; tobj; tobj = tobj->next_content)
        {
            int sum_count = 0;

			if (tobj->nVirtual == frBre->o.od.value[0])
			{
				act ("You cannot produce anything from $p while $P remains within it - that is already a finished product!", false, ch, obj, tobj, TO_CHAR | _ACT_FORMAT);
				return;
			}

         // If it's scrap...
          //if (GET_ITEM_TYPE(tobj) == ITEM_SCRAP || tobj->nVirtual == VNUM_CORPSE || tobj->nVirtual == VNUM_HEAD)
          //  {
                // frBre 0 is a metal firebreather.
                if (frBre->o.od.value[5] == 0 && GET_MATERIAL_TYPE(tobj) == MATERIAL_METAL)
                {
                    int xind = 0;
                    // we need to figure out what our variable is...
                    for (int i = 0; i < 10; i++)
                    {
                        if (tobj->var_cat[i] && !str_cmp(tobj->var_cat[i], "$ironalloy"))
                        {
                            xind = vc_exists(tobj->var_color[i], "$ironalloy");
                            if (conversion[xind] == 2000)
                            {
                                weight_mod = vd_data(tobj->var_color[i], "$ironalloy", 1);
                                quality_mod = vd_data(tobj->var_color[i], "$ironalloy", 3);

                                if (weight_mod < 100)
                                {
                                    conversion[xind] += (100 - weight_mod) * 50;
                                }

                                if (quality_mod > 100)
                                {
                                    conversion[xind] += (quality_mod - 100) * 50;
                                }
                            }
                            amounts[xind] += tobj->obj_flags.weight * tobj->count;
                            sum_count += 1;
                            break;
                        }
                    }

					// If we get to here, we had no variables
					// for our metal object, so we'll default to alion.
					if (!sum_count)
					{
						amounts[1] += tobj->obj_flags.weight * tobj->count;
					}
                }
                // frBre 3 is the plaatic firebreather.
				else if (frBre->o.od.value[5] == 3 && (GET_MATERIAL_TYPE(tobj) == MATERIAL_PLASTICS))
				{
					amounts[0] += tobj->obj_flags.weight * tobj->count;
				}
                // frBre 2 is the polymesh firebreather.
                else if (frBre->o.od.value[5] == 2 && GET_MATERIAL_TYPE(tobj) == MATERIAL_TEXTILE)
                {
					amounts[0] += tobj->obj_flags.weight * tobj->count;
                }
				// frBre 4 is the bullet Firebreather
                else if (frBre->o.od.value[5] == 4 && (GET_ITEM_TYPE(tobj) == ITEM_BULLET || tobj->nVirtual == 13105))
                {
					amounts[0] += tobj->obj_flags.weight * tobj->count;
                }
				// frBre1 is the organic firebreather.
				else if (frBre->o.od.value[5] == 1 && (GET_MATERIAL_TYPE(tobj) == MATERIAL_ORGANIC || tobj->nVirtual == VNUM_CORPSE || tobj->nVirtual == VNUM_HEAD))
				{
					amounts[0] += tobj->obj_flags.weight * tobj->count;
				}
            //}
        }

        if (!str_cmp(arg, "test"))
        {
            sprintf(buf, "The contents of $p could produce the following:\n");
			bool found = false;
            // we need to figure out what our variable is...
			if (frBre->o.od.value[5] == 0)
			{
				for (int i = 0; i < AMOUNT_MAX; i++)
				{
					if (amounts[i] / conversion[i] >= 1)
					{
						sprintf(buf + strlen(buf), "   #6%d#0 bar%s of #6%s-iron#0\n", amounts[i] / conversion[i], (amounts[i] / conversion[i] > 1 ? "s" : ""), vd_short ("$ironalloy", i));
						found = true;
	                }
				}
			}
			else if (frBre->o.od.value[5] == 1)
			{
				sprintf(buf + strlen(buf), "   #6%d#0 small plastic tub%s of food paste\n", amounts[0] / conversion[0], (amounts[0] / conversion[0] > 1 ? "s" : ""));
				found = true;
			}
			else if (frBre->o.od.value[5] == 4)
			{
				if (amounts[0] / conversion[1] >= 1)
				{
					sprintf(buf + strlen(buf), "   #6%d#0#2 solid gold brick%s#0\n", amounts[0] / conversion[1], (amounts[0] / conversion[1] > 1 ? "s" : ""));
					found = true;
				}
				if (amounts[0] / conversion[2] >= 1)
				{
					sprintf(buf + strlen(buf), "   #6%d#0#2 small block%s of six-carat alloy#0\n", amounts[0] / conversion[2], (amounts[0] / conversion[2] > 1 ? "s" : ""));
					found = true;
				}
				if (amounts[0] / conversion[3] >= 1)
				{
					sprintf(buf + strlen(buf), "   #6%d#0#2 small block%s of twelve-carat alloy#0\n", amounts[0] / conversion[3], (amounts[0] / conversion[3] > 1 ? "s" : ""));
					found = true;
				}
				if (amounts[0] / conversion[4] >= 1)
				{
					sprintf(buf + strlen(buf), "   #6%d#0#2 small block%s of eighteen-carat alloy#0\n", amounts[0] / conversion[4], (amounts[0] / conversion[4] > 1 ? "s" : ""));
					found = true;
				}
			}
			else if (frBre->o.od.value[5] == 3)
			{
				for (int i = 4; i < 7; i++)
				{
					if (amounts[0] / conversion[i] >= 1)
					{
						sprintf(buf + strlen(buf), "   #6%d#0 bar%s of #6off-white %s-polyprop#0\n", amounts[0] / conversion[i], (amounts[0] / conversion[i] > 1 ? "s" : ""), vd_short ("$polyprop", i));
						found = true;
	                }
				}
			}
			else if (frBre->o.od.value[5] == 2)
			{
				for (int i = 1; i < AMOUNT_MAX; i++)
				{
					if (amounts[0] / conversion[i] >= 1)
					{
						sprintf(buf + strlen(buf), "   #6%d#0 bolt%s of #6natural-hued %s fabric#0\n", amounts[0] / conversion[i], (amounts[0] / conversion[i] > 1 ? "s" : ""), vd_short ("$polymesh", i));
						found = true;
	                }
				}
			}

			if (!found)
			{
				sprintf(buf + strlen(buf), "   Nothing! You haven't put in enough weight. Typically, you need at least twenty\n pounds of scraps to produce anything, but rarer materials will require more scrap.\n");
			}


            act(buf, false, ch, obj, 0, TO_CHAR);
        }
		else if (!str_cmp(arg, "produce"))
		{
			bool first = false;
			bool something = false;

			// If it's a metal object, we need to run through this for each of our indicies and spit out the new metal.
			if (frBre->o.od.value[5] == 0)
			{
				// For each of our amounts...
				for (int ind = 1; ind < AMOUNT_MAX; ind++)
				{
					// If we've got at least one bar's worth here...
					if (amounts[ind] / conversion[ind] >= 1)
					{
						tobj = obj->contains;
						next_tobj = NULL;
						// We need to consume objects until we've got a count that is equal to or greater than our amount
						while (counts[ind] / conversion[ind] < amounts[ind] / conversion[ind])
						{
							int sum_count = 0;
							// Of course, if we have no tobj for whatever reason, we'll need to break.
							if (tobj)
								next_tobj = tobj->next_content;
							else
								break;

							if (tobj->nVirtual == frBre->o.od.value[0])
							{
								tobj = next_tobj;
								continue;
							}

							// If our object happens to be scrap and is made out of metal...
							if (/*GET_ITEM_TYPE(tobj) == ITEM_SCRAP &&*/ GET_MATERIAL_TYPE(tobj) == MATERIAL_METAL)
							{
								// Let's cycle through its variables until we find one that belongs
								// to our current ind...
								for (int i = 0; i < 10; i++)
								{
									if (tobj->var_cat[i] && !str_cmp(tobj->var_cat[i], "$ironalloy"))
									{
										sum_count += 1;
										if (ind == vc_exists(tobj->var_color[i], "$ironalloy"))
										{
											// If so, then we need to add this to our count of ind, and destroy the object.
											counts[ind] += tobj->obj_flags.weight;

											// If we've got more than 1 in our stack, we need to loop through again.
											if (tobj->count > 1)
												next_tobj = tobj;

											// Now we extract our counted object.
											obj_from_obj(&tobj, 1);
											extract_obj (tobj);
											// We only want one variable of any one alloy.
											break;
										}
									}
								}

								// If we get to here, we had no variables
								// for our metal object, so we'll default to alion.
								if (!sum_count)
								{
									counts[1] += tobj->obj_flags.weight;
									if (tobj->count > 1)
										next_tobj = tobj;

									obj_from_obj(&tobj, 1);
									extract_obj (tobj);
								}
							}
							// And we move on to our next tobj.
							tobj = next_tobj;
						}

						// Now lets load up a new ironalloy of the right ind for each one we've got.
						for (int i = 0; i < amounts[ind] / conversion[ind]; i ++)
						{
							if ((robj = load_colored_object(frBre->o.od.value[0], vd_short("$ironalloy", ind), 0, 0, 0, 0, 0, 0, 0, 0, 0)))
							{
								obj_to_obj(robj, obj);
							}
						}

						// If we did load something up...
						if (robj)
						{
							// Spit out a nice little message if we haven't already...
							if (!first)
							{
								act ("You nod to the workers milling about $p, who slowly amble over and take $P from infront of you, sorting through the various scraps and junk you've placed within before starting up the Firebreather.\n", false, ch, frBre, obj, TO_CHAR | _ACT_FORMAT);
								act ("The workers milling about $p take $P from in front of $n.", false, ch, frBre, obj, TO_ROOM | _ACT_FORMAT);
								first = true;
							}

							// And then tell us what we've won.
							something = true;
							sprintf(buf, "$p (x%d) is produced and loaded in to $P.", amounts[ind] / conversion[ind]);
							act(buf, false, ch, robj, obj, TO_CHAR | _ACT_FORMAT);
						}
					}
				}
			}
			// Otherwise, for everything else, we're just testing amounts[0] against conversion[option]
			else
			{
				// If we've got at least one bar's worth here...
				if (amounts[0] / conversion[option] >= 1)
				{
					tobj = obj->contains;
					next_tobj = NULL;
					// We need to consume objects until we've got a count that is equal to or greater than our amount
					while (counts[0] / conversion[option] < amounts[0] / conversion[option])
					{
						// Of course, if we have no tobj for whatever reason, we'll need to break.
						if (tobj)
							next_tobj = tobj->next_content;
						else
							break;

						// If our object fits the criteria for our particular firebreathers...
						if ((frBre->o.od.value[5] == 1 && ((/*GET_ITEM_TYPE(tobj) == ITEM_SCRAP &&*/ GET_MATERIAL_TYPE(tobj) == MATERIAL_ORGANIC) || tobj->nVirtual == VNUM_CORPSE || tobj->nVirtual == VNUM_HEAD)) ||
							(frBre->o.od.value[5] == 2 && /*GET_ITEM_TYPE(tobj) == ITEM_SCRAP &&*/ GET_MATERIAL_TYPE(tobj) == MATERIAL_TEXTILE) ||
							(frBre->o.od.value[5] == 4 && (GET_ITEM_TYPE(tobj) == ITEM_BULLET || tobj->nVirtual == 13105)) ||
							(frBre->o.od.value[5] == 3 && /*GET_ITEM_TYPE(tobj) == ITEM_SCRAP &&*/ GET_MATERIAL_TYPE(tobj) == MATERIAL_PLASTICS))
						{

							// we add it to the count.
							counts[0] += tobj->obj_flags.weight;

							if (tobj->count > 1)
								next_tobj = tobj;

							obj_from_obj(&tobj, 1);
							extract_obj (tobj);
						}

						// And we move on to our next tobj.
						tobj = next_tobj;
					}
                    
					// Now lets load up a new ironalloy of the right ind for each one we've got.
					for (int i = 0; i < amounts[0] / conversion[option]; i ++)
					{
						if (frBre->o.od.value[5] == 1)
						{
							if ((robj = load_object(frBre->o.od.value[0])))
							{
								obj_to_obj(robj, obj);
							}
						}
						else if (frBre->o.od.value[5] == 2)
						{
							if ((robj = load_colored_object(frBre->o.od.value[0], arg3, vd_short("$polymesh", option), 0, 0, 0, 0, 0, 0, 0, 0)))
							{
								obj_to_obj(robj, obj);
							}
						}
						else if (frBre->o.od.value[5] == 3)
						{
							if ((robj = load_colored_object(frBre->o.od.value[0], arg3, vd_short("$polyprop", option), 0, 0, 0, 0, 0, 0, 0, 0)))
							{
								obj_to_obj(robj, obj);
							}
						}
						else if (frBre->o.od.value[5] == 4)
						{
							int load_block = 0;

							if (option == 1)
								load_block = 14000;
							else if (option == 2)
								load_block = 13044;
							else if (option == 3)
								load_block = 13103;
							else if (option == 4)
								load_block = 13104;

							if ((robj = load_colored_object(load_block, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)))
							{
								obj_to_obj(robj, obj);
							}
						}
					}

					// If we did load something up...
					if (robj)
					{
						// Spit out a nice little message if we haven't already...
						if (!first)
						{
							act ("You nod to the workers milling about $p, who slowly amble over and take $P from infront of you, sorting through the various scraps and junk you've placed within before starting up the Firebreather.\n", false, ch, frBre, obj, TO_CHAR | _ACT_FORMAT);
							act ("The workers milling about $p take $P from in front of $n.", false, ch, frBre, obj, TO_ROOM | _ACT_FORMAT);
							first = true;
						}
						// And then tell us what we've won.
						something = true;
						sprintf(buf, "$p (x%d) is produced and loaded in to $P.", amounts[0] / conversion[option]);
						act(buf, false, ch, robj, obj, TO_CHAR | _ACT_FORMAT);
					}
				}
			}
			if (!something)
			{
				act ("There isn't enough resources in $p for $P to produce anything with.", false, ch, obj, frBre, TO_CHAR | _ACT_FORMAT);
				return;
			}
		}
    }
	else
	{
        sprintf(buf,               "combine usage: combine <request|test|return|produce|option> <option>\n");
        sprintf(buf + strlen(buf), "combine request            - produces a crate for you to place your goods\n");
        sprintf(buf + strlen(buf), "combine test               - shows how many of each option your crate will produce\n");
        sprintf(buf + strlen(buf), "combine return             - returns the crate (requires it to be empty)\n");
        sprintf(buf + strlen(buf), "combine option             - shows what can be produced (for polymesh and polyprop)\n");
        sprintf(buf + strlen(buf), "combine produce <option>   - recycles goods as per what is in your crate.\n");
        send_to_char(buf, ch);
        return;
	}
}

void do_attire (CHAR_DATA *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    char * result = NULL;
       
    argument = one_argument(argument, arg);

    if (!*arg)
    {
        act ("Usage: wmote <worn clothing> <attire>\n", false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (!(obj = get_obj_in_dark (ch, arg, ch->equip))) 
    {
        sprintf (buf, "You are not wearing '%s.'\n", arg);
        act (buf, false, ch, 0, 0, TO_CHAR);
        return;
    }
    else
    {
        if (!*argument || strlen(argument) > 22)
        {
            act ("Your wmote string may only be 22 characters long.\n", false, ch, 0, 0, TO_CHAR);
            return;
        }
        if (!(strcmp(argument, "normal")))
        {
            act ("You clear the attire for $p.\n", false, ch, obj, 0, TO_CHAR);
		    obj->attire = (char *) NULL;
		    return;
        }
        result = argument;
        sprintf (buf, "You set the attire string of $p to: %s\n", argument);
        act (buf, false, ch, obj, 0, TO_CHAR);
        obj->attire = str_dup(result);
    }
}

void do_conceal (CHAR_DATA *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    char * result = NULL;
    argument = one_argument(argument, arg);

    if (!*arg)
    {
        act ("Usage: conceal <worn clothing>\n", false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (!(obj = get_obj_in_dark (ch, arg, ch->equip)) || 
       (IS_SET (obj->obj_flags.extra_flags2, ITEM_CONCEALED)))
    {
        sprintf (buf, "You are not wearing an unconcealed '%s.'\n", arg);
        act (buf, false, ch, 0, 0, TO_CHAR);
        return;
    }
    else
    {
        if (obj->location == WEAR_OVERWEAR ||
            obj->location == WEAR_UNDERWEAR ||
            obj->location == WEAR_FINGER_R ||
            obj->location == WEAR_FINGER_L ||
            obj->location == WEAR_BODY ||
            obj->location == WEAR_NECK_1 ||
            obj->location == WEAR_NECK_2 ||
            obj->location == WEAR_ARMS ||
            obj->location == WEAR_ABOUT ||       
            obj->location == WEAR_WAIST ||
            obj->location == WEAR_WRIST_R ||
            obj->location == WEAR_WRIST_L ||
            obj->location == WEAR_BELT_1 ||
            obj->location == WEAR_BELT_2 ||
            obj->location == WEAR_THROAT ||
            obj->location == WEAR_EAR ||
            obj->location == WEAR_SHOULDER_R ||
            obj->location == WEAR_SHOULDER_L ||
            obj->location == WEAR_ANKLE_R ||
            obj->location == WEAR_ANKLE_L ||
            obj->location == WEAR_HAIR ||
            obj->location == WEAR_ARMBAND_R ||
            obj->location == WEAR_ARMBAND_L)
        {
            act ("You shift clothing to conceal $p.", false, ch, obj, 0, TO_CHAR);
            act ("$n shifts their clothing to conceal $p.", false, ch, obj, 0, TO_ROOM);
            obj->obj_flags.extra_flags2 |= ITEM_CONCEALED;
            return;
        } 
        else 
        {
            act ("You cannot conceal $p.", false, ch, obj, 0, TO_CHAR);
            return;
        }
    }
}

void do_reveal (CHAR_DATA *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CLAN_DATA *clan_def = NULL;
    OBJ_DATA *obj = NULL;
    CHAR_DATA *tch = NULL;
    AFFECTED_TYPE *af = NULL;
    int skill = 0;
    int clan_id = 0;

    argument = one_argument(argument, arg);

    if (!*arg)
    {
        act ("Usage: reveal <object hidden in room> <character | 'Clan Name' | group | room>\n"
             "       reveal <hidden clothing>\n", false, ch, 0, 0, TO_CHAR);
        
        return;
    }

    if (!(obj = get_obj_in_list_vis (ch, arg, ch->room->contents)))
    {
        if (!(obj = get_obj_in_dark (ch, arg, ch->equip)) || 
           (!IS_SET (obj->obj_flags.extra_flags2, ITEM_CONCEALED)))
        {
            sprintf (buf, "You don't see %s hidden here.", arg);
            act (buf, false, ch, 0, 0, TO_CHAR);
            return;
        }
        else
        {
            act ("You shift clothing to reveal $p.", false, ch, obj, 0, TO_CHAR);
            act ("$n shifts their clothing to reveal $p.", false, ch, obj, 0, TO_ROOM);
            obj->obj_flags.extra_flags2 &= ~ITEM_CONCEALED;
            return;
        }
    }

    if (!(af = obj->xaffected) && !(af = get_obj_affect (obj, MAGIC_HIDDEN)))
    {
        act ("But $p isn't hidden from sight.", false, ch, obj, 0, TO_CHAR);
        return;
    }

    if (GET_ITEM_TYPE(obj) == ITEM_TRAP)
    {
        act ("$p is a trap, and so not allowed to be revealed just yet.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }


    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "all") || !str_cmp(arg, "room"))
    {
        remove_obj_mult_affect(obj, MAGIC_HIDDEN);
        act ("You reveal $p.", false, ch, obj, 0, TO_CHAR);
        act ("$n reveals $p.", false, ch, obj, 0, TO_ROOM);
        return;
    }
    else if (str_cmp(arg, "group") && !(tch = get_char_room_vis (ch, arg)) && !(clan_def = get_clandef_long(arg)))
    {
        sprintf (buf, "You don't see %s standing here, or you are not part of clan '%s' (enter the full clan name enclosed in quotes).", arg, arg);
        act (buf, false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (!IS_LIGHT (ch->room)
            && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !is_goggled(ch)
            && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
        sprintf (buf, "You don't see %s hidden here.", arg);
        act (buf, false, ch, obj, 0, TO_CHAR);
        return;
    }

    if ((clan_def = get_clandef_long(arg)))
    {
        clan_id = clan_def->id;

        if (clan_id && clan_def)
        {
            for (af = obj->xaffected; af; af = af->next)
            {
                if (af->a.hidden.clan == clan_id)
                {
                    sprintf (buf, "You rehide $p, remarking the signal for the members of %s to notice.", clan_def->literal);
                    act (buf, false, ch, obj, 0, TO_CHAR);
                    break;
                }
                else
                {
                    sprintf (buf, "You make a mark about $p for the members of %s to notice.", clan_def->literal);
                    act (buf, false, ch, obj, 0, TO_CHAR);
                    break;
                }
            }

            remove_obj_mult_affect(obj, MAGIC_HIDDEN);

            af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

            af->type = MAGIC_HIDDEN;
            af->a.hidden.duration = -1;
            af->a.hidden.hidden_value = skill;
            af->a.hidden.coldload_id = ch->coldload_id;
            af->a.hidden.clan = clan_id;
            af->next = obj->xaffected;
            obj->xaffected = af;
            return;
        }
        else
        {
            act ("There doesn't appear to be any such clan in the database. Please notify an administrator.", false, ch, 0, 0, TO_CHAR);
            return;
        }
    }
    else if (!str_cmp(arg, "group"))
    {
        if (!do_group_size(ch))
        {
            act ("But there's no one in your group to reveal it to!", false, ch, 0, 0, TO_CHAR);
            return;
        }

        for (tch = ch->room->people; tch; tch = tch->next_in_room)
        {
            if (!are_grouped(ch, tch))
                continue;

            if (!IS_LIGHT (tch->room)
                    && !get_affect (tch, MAGIC_AFFECT_INFRAVISION)
					&& !is_goggled(tch)
                    && !IS_SET (tch->affected_by, AFF_INFRAVIS))
            {
                continue;
            }

            // If someone already reveals something to you,

            for (af = obj->xaffected; af; af = af->next)
            {
                if (af->type == MAGIC_HIDDEN)
                {
                    skill = af->a.hidden.hidden_value;
                    if (af->a.hidden.coldload_id == tch->coldload_id)
                    {
                        act ("$N reveals $p to you, but then, you already knew it was there.", false, tch, obj, ch, TO_CHAR);
                        continue;
                    }
                }
            }

            af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

            af->type = MAGIC_HIDDEN;
            af->a.hidden.duration = -1;
            af->a.hidden.hidden_value = skill;
            af->a.hidden.coldload_id = tch->coldload_id;
            af->next = obj->xaffected;
            obj->xaffected = af;
            act ("$N reveals $p to you.", false, tch, obj, ch, TO_CHAR);
        }

        act ("You reveal $p to everyone in your group.", false, ch, obj, 0, TO_CHAR);
    }
    else
    {

        if (!IS_LIGHT (tch->room)
                && !get_affect (tch, MAGIC_AFFECT_INFRAVISION)
				&& !is_goggled(tch)
                && !IS_SET (tch->affected_by, AFF_INFRAVIS))
        {
            act ("$N wouldn't be able to see $p, even if it wasn't hidden.", false, ch, obj, tch, TO_CHAR);
            return;
        }

        // If someone already reveals something to you,

        for (af = obj->xaffected; af; af = af->next)
        {
            if (af->type == MAGIC_HIDDEN)
            {
                skill = af->a.hidden.hidden_value;
                if (af->a.hidden.coldload_id == tch->coldload_id)
                {
                    act ("You reveal $p to $N.", false, ch, obj, tch, TO_CHAR);
                    act ("$N reveals $p to you, but then, you already knew it was there.", false, tch, obj, ch, TO_CHAR);
                    return;
                }
            }
        }

        af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

        af->type = MAGIC_HIDDEN;
        af->a.hidden.duration = -1;
        af->a.hidden.hidden_value = skill;
        af->a.hidden.coldload_id = tch->coldload_id;
        af->next = obj->xaffected;
        obj->xaffected = af;


        act ("You reveal $p to $N.", false, ch, obj, tch, TO_CHAR);
        act ("$N reveals $p to you.", false, tch, obj, ch, TO_CHAR);

    }
}


// Allows a PC to upgrade their stats when they have reached certain criteria.
// Designed to allow the development of true "hero" characters, and reward those
// who are able to keep their characters alive for a long time.


void
do_upgrade (CHAR_DATA *ch, char *argument, int cmd)
{
	const char* attribs[7] =
	{
		"strength",
		"dexterity",
		"constitution",
		"intelligence",
		"willpower",
		"mutations",
		"agility"
	};


    char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char arg3[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

	int raise = -1;
	int lower = -1;

    argument = one_argument(argument, arg1);

	if (get_affect (ch, AFFECT_LOST_CON))
    {
        send_to_char("You are feeling too badly beaten right now to worry about improving your body or mind.\n", ch);
		return;
    }

	if (get_affect (ch, AFFECT_UPGRADE_DELAY))
	{
        send_to_char("Your body and mind are still recovering from your last upgrade.\n", ch);
		return;
	}

    if (!*arg1)
    {
        send_to_char("You need to enter an attribute to raise - #6str, int, agi, dex, con, or wil#0.\n", ch);
        return;
    }

	argument = one_argument(argument, arg2);

	if (!*arg2)
	{
        send_to_char("You need to enter an attribute to decrease - #6str, int, agi, dex, con, or wil#0.\n", ch);
        return;
	}

	if (!str_cmp(arg1, "str"))
	{
		raise = 0;

		if (ch->str + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme strength.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg1, "dex"))
	{
		raise = 1;

		if (ch->dex + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme dexterity.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg1, "con"))
	{
		raise = 2;

		if (ch->con + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme constitution.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg1, "int"))
	{
		raise = 3;

		if (ch->intel + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme intelligence.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg1, "wil"))
	{
		raise = 4;

		if (ch->wil + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme willpower.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg1, "agi"))
	{
		raise = 6;

		if (ch->agi + 1 > 18)
		{
			send_to_char ("There is no possibility of improving on your extreme agility.\n", ch);
			return;
		}
	}

	if (raise == -1)
    {
        send_to_char("You need to enter an attribute to raise - #6str, int, agi, dex, con, or wil#0.\n", ch);
        return;
    }

	if (!str_cmp(arg2, "str"))
	{
		lower = 0;

		if (ch->str - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal strength.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg2, "dex"))
	{
		lower = 1;

		if (ch->dex - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal dexterity.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg2, "con"))
	{
		lower = 2;

		if (ch->con - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal constitution.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg2, "int"))
	{
		lower = 3;

		if (ch->intel - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal intelligence.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg2, "wil"))
	{
		lower = 4;

		if (ch->wil - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal willpower.\n", ch);
			return;
		}
	}
	else if (!str_cmp(arg2, "agi"))
	{
		lower = 6;

		if (ch->agi - 1 < 10)
		{
			send_to_char ("There is no possibility of further debasing your dismal agility.\n", ch);
			return;
		}
	}

	if (lower == -1)
    {
        send_to_char("You need to enter an attribute to decrease - #6str, int, agi, dex, con, or wil#0.\n", ch);
        return;
	}

	argument = one_argument(argument, arg3);

	if (str_cmp(arg3, "!"))
	{
		sprintf(buf, "Are you sure you want to upgrade your #6%s#0 at the expense of your #6%s#0?\n"
					 "Type '#6upgrade %s %s !#0' to confirm.", attribs[raise], attribs[lower], arg1, arg2);
		send_to_char(buf, ch);
		return;
	}

	sprintf(buf, "After six more hours of playing, you will gain a point of #6%s#0, and lose a point of #6%s#0. Be sure to roleplay the change accordingly. You will next be able to upgrade an attribute in 90 Real-Life days.", attribs[raise], attribs[lower]);

	act(buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	magic_add_affect (ch, AFFECT_GAIN_POINT, 24, 0, 0, 0, raise);
	magic_add_affect (ch, AFFECT_LOSE_POINT, 24, 0, 0, 0, lower);
	magic_add_affect (ch, AFFECT_UPGRADE_DELAY, -1, (time (0) + 90 * 24 * 60 * 60), 0, 0, 0);

}

void
do_commence (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *tch, *tch_next;
    DESCRIPTOR_DATA *td;
    int from_room = 0, to_room = 0;
    bool morgul = false;
    char buf[MAX_STRING_LENGTH];

    if (!IS_SET (ch->plr_flags, NEWBIE))
    {
        send_to_char ("It appears that you've already begun play...\n", ch);
        return;
    }

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Sorry, but guests are not allowed into the gameworld.\n",
                      ch);
        return;
    }

    act
    ("$n travels towards his destiny in the town of Rust...",
     true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    sprintf (buf, "\n#6Welcome to %s!#0\n\n", MUD_NAME);
    send_to_char (buf, ch);

    if (get_queue_position (ch) != -1)
        update_assist_queue (ch, true);

    from_room = ch->in_room;

    /*
    Procedure for setting people up in different places.

    if (IS_SET (ch->plr_flags, START_MORDOR))
        {
          char_from_room (ch);
          char_to_room (ch, MINAS_MORGUL_START_LOC);
          // char_to_room (ch, EDEN_START_LOC);
          ch->was_in_room = 0;
          add_clan (ch, "mordor_char", CLAN_MEMBER);
          if (!ch->skills[SKILL_SPEAK_BLACK_SPEECH]
    	  || ch->skills[SKILL_SPEAK_BLACK_SPEECH] < 30)
    	{
    	  ch->skills[SKILL_SPEAK_BLACK_SPEECH] = 30;
    	  ch->pc->skills[SKILL_SPEAK_BLACK_SPEECH] = 30;
    	}
          morgul = true;
        } */

    char_from_room (ch);
    char_to_room (ch, MINAS_TIRITH_START_LOC);
    ch->was_in_room = 0;
    add_clan (ch, "rustclan", CLAN_MEMBER);
    if (!ch->skills[SKILL_COMMON]
            || ch->skills[SKILL_COMMON] < 30)
    {
        ch->skills[SKILL_COMMON] = 30;
        ch->pc->skills[SKILL_COMMON] = 30;
    }

    ch->plr_flags &= ~NEWBIE;
    ch->time.played = 0;
	ch->shock = ch->max_shock;
    do_save (ch, "", 0);

    act ("One of the many pods swings open, and out emerges $n!", true, ch, 0, 0,
         TO_ROOM | _ACT_FORMAT);
    sprintf (buf,
             "#3[OOC MESSAGE: %s has awoken.]#0",
             char_short (ch), (morgul) ? "the Tur Edendor settlement" : "Sleeping Pod Bay 3GZ");

    for (td = descriptor_list; td; td = td->next)
    {
        if (!td->character || td->connected != CON_PLYNG)
            continue;
        if (!is_brother (ch, td->character))
            continue;
        if (IS_SET (td->character->plr_flags, MENTOR))
        {
            act (buf, true, ch, 0, td->character, TO_VICT | _ACT_FORMAT);
        }
    }

    // Delete auto-genned individual debriefing area, if applicable.

    if (from_room >= 100000 && vnum_to_room (from_room) &&
            !str_cmp (vnum_to_room (from_room)->name, PREGAME_ROOM_NAME))
    {

        for (tch = (vnum_to_room (from_room))->people; tch; tch = tch_next)
        {
            tch_next = tch->next_in_room;
            if (!IS_NPC (tch))
            {
                char_from_room (tch);
                if ((to_room = tch->was_in_room) && vnum_to_room (to_room))
                    char_to_room (tch, to_room);
                else
                    char_to_room (tch, OOC_LOUNGE);
            }
        }

        delete_contiguous_rblock (vnum_to_room (from_room), -1, -1);
    }

    do_look (ch, NULL, 0);
}

void
do_ic (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];

    if (!IS_SET (ch->room->room_flags, OOC))
    {
        send_to_char ("This isn't an OOC area. Find your way out, IC.\n", ch);
        return;
    }

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Sorry, but guests are not allowed into the gameworld.\n",
                      ch);
        return;
    }

    if (!ch->was_in_room)
    {
        send_to_char ("Hmm... I'm not quite sure where to drop you. You'd"
                      " best petition for assistance. Sorry!\n", ch);
        return;
    }

    if (ch->was_in_room == -1)
    {
        send_to_char ("You'll need to begin play via the COMMENCE command.\n",
                      ch);
        return;
    }

    sprintf (buf, "Weary of the OOC chatter, #5%s#0 returns to the Ship.",
             char_short (ch));
    act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    send_to_char ("Weary of the OOC chatter, you return to the Ship.\n\n",
                  ch);

    char_from_room (ch);
    char_to_room (ch, ch->was_in_room);
    ch->was_in_room = 0;

    sprintf (buf, "%s#0 enters the area from the OOC lounge.)",
             char_short (ch));
    *buf = toupper (*buf);
    sprintf (buffer, "(#5%s", buf);
    sprintf (buf, "%s", buffer);

    act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    do_look (ch, "", 0);

}

void
delayed_ooc (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];

    send_to_char ("You are now exiting to the OOC lounge...\n", ch);
    send_to_char
    ("\n#1Remember, the use of this lounge is a privilege -- do not abuse it!\n"
     "To return to the in-character parts of the grid, use the IC command.#0\n\n",
     ch);

    sprintf (buf, "%s#0 has exited to the OOC lounge.)", char_short (ch));
    *buf = toupper (*buf);
    sprintf (buffer, "(#5%s", buf);
    sprintf (buf, "%s", buffer);

    act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    ch->was_in_room = ch->room->vnum;
    char_from_room (ch);
    char_to_room (ch, OOC_LOUNGE);

    sprintf (buf, "%s#0 has entered the OOC lounge.", char_short (ch));
    *buf = toupper (*buf);
    sprintf (buffer, "#5%s", buf);
    sprintf (buf, "%s", buffer);

    act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    do_look (ch, "", 0);

    ch->delay = 0;
}

void
do_quit (CHAR_DATA * ch, char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH];
    int dwelling = 0, outside = 0;
    bool block = false;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *tch;
    DESCRIPTOR_DATA *d;

    if (ch->descr() && ch->descr()->original)
    {
        send_to_char ("Not while you are switched.  RETURN first.\n\r", ch);
        return;
    }

    if (is_switched (ch))
        return;

    if (IS_NPC (ch))
    {
        send_to_char ("NPC's cannot quit.\n\r", ch);
        return;
    }

    if (ch->in_room == 60001 || ch->in_room == 60002 || ch->in_room == 60003)
    {
        if (IS_MORTAL (ch))
        {
            send_to_char ("You can't quit out in the Battlefield!\n", ch);
            return;
        }
    }

    argument = one_argument (argument, arg);

    if (!IS_SET (ch->flags, FLAG_GUEST) && IS_MORTAL (ch)
            && !IS_SET (ch->room->room_flags, SAFE_Q) && ch->descr() && cmd != 3)
    {
        send_to_char
        ("You may not quit in this area. Most inns, taverns, and so forth are\n"
         "designated as safe-quit areas; if you are in the wilderness, you may\n"
         "also use the CAMP command to create a safe-quit room. #1If you simply\n"
         "drop link without quitting, your character will remain here, and may\n"
         "be harmed or injured while you are away. We make no guarantees!#0\n",
         ch);
        return;
    }

    if (GET_POS (ch) == POSITION_FIGHTING && !IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("You can't quit during combat!\n", ch);
        return;
    }

    if (IS_SUBDUEE (ch))
    {
        act ("$N won't let you leave!", false, ch, 0, ch->subdue, TO_CHAR);
        return;
    }

    if (IS_MORTAL (ch) &&
            IS_SET (ch->room->room_flags, LAWFUL) &&
            get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone) && cmd != 3)
    {
        send_to_char ("You may not exit the game here.  While a criminal in "
                      "this area you may not quit\nin lawful places.\n", ch);
        return;
    }

    if (IS_HITCHER (ch) && !cmd)
    {
        send_to_char
        ("You cannot quit out with a hitched mount; try CAMPING or the STABLE.\n",
         ch);
        return;
    }
/*
	std::vector<monitor_data*>::iterator it;
                for (it = monitor_list.begin(); it != monitor_list.end();it++)
                {
                        if ( !(*it)->watcher )
                        continue;

                        if ( !(*it)->radio_freq )
                        continue;

                        if ( (*it)->watcher != ch )
                        continue;

                        monitor_list.erase(it);                }

*/

	if (IS_SUBDUER (ch))
        do_release (ch, "", 0);

    if (!IS_NPC (ch) && ch->pc->edit_player)
    {
        unload_pc (ch->pc->edit_player);
        ch->pc->edit_player = NULL;
    }

    if (ch->combat_block >=3 || ch->combat_block < -3)
        ch->combat_block = 0;

    if (!GET_TRUST(ch) && ch->combat_block && ch->combat_block <= 3 && ch->combat_block >= -3)
    {
        send_to_char("\nYou have been too recently engaged in combat to quit.\n"
                     "You may need to wait up to 3 minutes before you are\n"
                     "able to quit. You will receive a message informing\n"
                     "you when you are able to quit.\n\n", ch);

        if (ch->combat_block > 0)
            ch->combat_block = ch->combat_block * -1;
        return;
    }


    remove_affect_type (ch, MAGIC_SIT_TABLE);

    act ("Goodbye, friend.. Come back soon!", false, ch, 0, 0, TO_CHAR);
    act ("$n leaves the area.", true, ch, 0, 0, TO_ROOM);

    sprintf (s_buf, "%s has left the game.\n", GET_NAME (ch));

    if (!ch->pc->admin_loaded)
        send_to_gods (s_buf);

    d = ch->descr();

    sprintf (s_buf, "%s is quitting.  Saving character.", GET_NAME (ch));
    system_log (s_buf, false);

    if (ch->pc)			/* Not sure why we wouldn't have a pc, but being careful */
    {
        ch->pc->last_logoff = time (0);
        ch->lastregen = time (0);
    }


    if (ch->in_room >= 100000 && vnum_to_room (ch->in_room)
            && (vnum_to_room (ch->in_room))->dir_option[OUTSIDE] != NULL)
    {
        for (tch = ch->room->people; tch; tch = tch->next_in_room)
        {
            if (tch != ch && IS_MORTAL (tch))
                block = true;
        }
        dwelling = ch->in_room;
        outside = ch->was_in_room;
        if (ch->room->dir_option[OUTSIDE]
                && !IS_SET (ch->room->dir_option[OUTSIDE]->exit_info, EX_CLOSED))
            block = true;
        if (!block)
        {
            for (obj = (vnum_to_room (outside))->contents; obj; obj = obj->next_content)
            {
                if (obj->o.od.value[0] == dwelling
                        && GET_ITEM_TYPE (obj) == ITEM_DWELLING
                        && IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC_DWELLING))
                {
                    obj->obj_flags.extra_flags |= ITEM_VNPC;
                    sprintf (buf,
                             "#2%s#0 grows still as its occupants settle in.",
                             obj_short_desc (obj));
                    buf[2] = toupper (buf[2]);
                    send_to_room (buf, obj->in_room);
                    break;
                }
            }
        }
    }

    extract_char (ch);

    ch->descriptor = NULL;

    if (!d)
        return;

    d->character = NULL;

    if (!d->acct || str_cmp(d->acct->name.c_str (), "Guest") == 0)
    {
        close_socket (d);
        return;
    }

    d->connected = CON_ACCOUNT_MENU;
    nanny (d, "");
}

void
do_save (CHAR_DATA * ch, char *argument, int cmd)
{
    char *p;
    char buf[MAX_STRING_LENGTH];

    if (GET_TRUST (ch))
    {
        p = one_argument (argument, buf);

        if (*buf)
        {
            save_document (ch, argument);
            return;
        }
    }

    if (IS_NPC (ch))
        return;

    sprintf (buf, "Saving %s.\n\r", GET_NAME (ch));
    send_to_char (buf, ch);
    //save_char (ch, true); -- lol wutz?
}

void
do_focus (CHAR_DATA * ch, char * argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    argument = one_argument (argument, buf);
    int ind;
    AFFECTED_TYPE *af;

    if (!*buf)
    {
        send_to_char("You need to name a skill you want to focus on.\n", ch);
        return;
    }

    ind = index_lookup (skills, buf);

    if (ind == -1)
    {
        send_to_char ("There isn't any such skill.\n", ch);
        return;
    }

    if (!real_skill (ch, ind) || (ch->skills[ind] < 11))
    {
        send_to_char ("You don't know that skill.\n", ch);
        return;
    }

    af = get_affect (ch, MAGIC_FLAG_FOCUS + ind);


    if ((af = get_affect (ch, MAGIC_FLAG_FOCUS + ind)))
    {
        affect_remove (ch, af);
        sprintf(buf2, "You no longer focus upon your %s skill\n", skills[ind]);
        send_to_char(buf2, ch);
    }
    else if ((af = get_affect (ch, MAGIC_FLAG_IGNORE + ind)))
    {
        affect_remove (ch, af);
        magic_add_affect (ch, MAGIC_FLAG_FOCUS + ind, 0, -1, 0, 0, 0);
        sprintf(buf2, "You will no longer ignore improving your %s skill, but will now focus on improving it.\n", skills[ind]);
        send_to_char(buf2, ch);
    }
    else
    {
        magic_add_affect (ch, MAGIC_FLAG_FOCUS + ind, 0, -1, 0, 0, 0);
        sprintf(buf2, "You focus upon increasing your %s skill\n", skills[ind]);
        send_to_char(buf2, ch);
    }

}

void
do_ignore (CHAR_DATA * ch, char * argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    argument = one_argument (argument, buf);
    int ind;
    AFFECTED_TYPE *af;

    if (!*buf)
    {
        send_to_char("You need to name a skill you want to ignore.\n", ch);
        return;
    }

    ind = index_lookup (skills, buf);

    if (ind == -1)
    {
        send_to_char ("There isn't any such skill.\n", ch);
        return;
    }

    if (!real_skill (ch, ind) || (ch->skills[ind] < 11))
    {
        send_to_char ("You don't know that skill.\n", ch);
        return;
    }

    if ((af = get_affect (ch, MAGIC_FLAG_IGNORE + ind)))
    {
        affect_remove (ch, af);
        sprintf(buf2, "You no longer ignore improving your %s skill\n", skills[ind]);
        send_to_char(buf2, ch);
    }
    else if ((af = get_affect (ch, MAGIC_FLAG_FOCUS + ind)))
    {
        affect_remove (ch, af);
        magic_add_affect (ch, MAGIC_FLAG_IGNORE + ind, 0, -1, 0, 0, 0);
        sprintf(buf2, "You will no longer focus on improving your %s skill, but will now ignore improving it.\n", skills[ind]);
        send_to_char(buf2, ch);
    }
    else
    {
        magic_add_affect (ch, MAGIC_FLAG_IGNORE + ind, 0, -1, 0, 0, 0);
        sprintf(buf2, "You will now ignore improving your %s skill\n", skills[ind]);
        send_to_char(buf2, ch);
    }
}

void
do_sneak (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int dir;

    if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
        send_to_char ("You cannot do this in an OOC area.\n", ch);
        return;
    }

    if (real_skill(ch, SKILL_HIDE) && ch->skills[SKILL_HIDE] >= 10)
        skill_learn(ch, SKILL_SNEAK);

    if (!real_skill (ch, SKILL_SNEAK))
    {
        send_to_char ("You just aren't stealthy enough to try.\n", ch);
        return;
    }

    if (IS_SUBDUER (ch))
    {
        send_to_char ("You can't sneak while you have someone in tow.\n", ch);
        return;
    }

    if (IS_ENCUMBERED (ch))
    {
        send_to_char ("You are too encumbered to sneak.\n\r", ch);
        return;
    }

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

    if (ch->speed == SPEED_JOG ||
            ch->speed == SPEED_RUN || ch->speed == SPEED_SPRINT)
    {
        sprintf (buf, "You can't sneak and %s at the same time.\n",
                 speeds[ch->speed]);
        send_to_char (buf, ch);
        return;
    }

    // Can't be sneaking and leading a big group.

    if (do_group_size(ch) > 5 && !ch->following)
    {
        send_to_char("You can't stealthily lead a group of this size.\n", ch);
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
            send_to_char ("Sneak in what direction?\n\r", ch);
            return;
        }

    }

    if ((dir = index_lookup (dirs, buf)) == -1)
    {
        send_to_char ("Sneak where?\n\r", ch);
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

    // Heads up to the player sneaking into a no-hide room,
    // as long as they can see inside
    ROOM_DATA * dest = vnum_to_room (ch->room->dir_option[dir]->to_room);
    if (dest && IS_SET (dest->room_flags, NOHIDE) && *argument != '!'
            && (IS_LIGHT (dest)
                || get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				|| is_goggled(ch)
                || IS_SET (ch->affected_by, AFF_INFRAVIS)))
    {
        char message [AVG_STRING_LENGTH] = "";
        sprintf (message, "   As you quietly approach the area ahead, "
                 "you notice that there \nare no hiding places available.  "
                 "If you wish to sneak out of this \narea into the one "
                 "ahead, then: #6 sneak %s !#0\n", buf);
        send_to_char (message, ch);
        return;
    }

    if (!number(0,10))
        skill_use (ch, SKILL_SNEAK, 0);

    if (odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) >= number (1, 100)
            || !would_reveal (ch))
    {
        magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
    }
    else
    {
        remove_affect_type (ch, MAGIC_HIDDEN);
        act ("$n attempts to be stealthy.", true, ch, 0, 0, TO_ROOM);
    }

    do_move (ch, "", dir);
}

void
do_hood (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj;

    if (!(obj = get_equip (ch, WEAR_NECK_1))
            || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

        if (!(obj = get_equip (ch, WEAR_NECK_2))
                || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
        {
            if (!(obj = get_equip (ch, WEAR_OVER))
                    || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
            {

                if (!(obj = get_equip (ch, WEAR_ABOUT))
                        || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
                {
                    send_to_char ("You are not wearing a hooded item.\n\r", ch);
                    return;
                }
            }
        }
    }

    if (!IS_SET (ch->affected_by, AFF_HOODED))
    {

        act ("You raise $p's hood, obscuring your face.", false, ch, obj, 0,
             TO_CHAR | _ACT_FORMAT);

        act ("$n raises $p's hood, concealing $s face.",
             false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

        ch->affected_by |= AFF_HOODED;

        return;
    }

    ch->affected_by &= ~AFF_HOODED;

    act ("You lower $p's hood, revealing your features.", false, ch, obj, 0,
         TO_CHAR | _ACT_FORMAT);
    act ("$n lowers $p's hood, revealing $s features.", false, ch, obj, 0,
         TO_ROOM | _ACT_FORMAT);

    return;
}

void
do_hide (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *tch;
    OBJ_DATA *obj;
    AFFECTED_TYPE *af = NULL;
    char buf[MAX_STRING_LENGTH];
    int delay = 11;

    if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
        send_to_char ("You cannot do this in an OOC area.\n", ch);
        return;
    }

    skill_learn(ch, SKILL_HIDE);

    if (!real_skill (ch, SKILL_HIDE))
    {
        send_to_char ("You lack the skill to hide.\n", ch);
        return;
    }

    if (IS_SET (ch->room->room_flags, NOHIDE))
    {
        send_to_char ("This room offers no hiding spots.\n", ch);
        return;
    }

    if (ch->aiming_at)
    {
        send_to_char ("You loose your aim as you move to conceal yourself.\n", ch);
		remove_targeted(ch->aiming_at, ch);
        ch->aiming_at = NULL;
        ch->aim = 0;
        return;
    }

    argument = one_argument (argument, buf);

    if (*buf)
    {

        if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
                !(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
        {
            send_to_char ("You don't have that.\n", ch);
            return;
        }

        if (!get_obj_in_list_vis (ch, buf, ch->right_hand) &&
                !get_obj_in_list_vis (ch, buf, ch->left_hand))
        {
            act ("It is too dark to hide it.", false, ch, 0, 0, TO_CHAR);
            return;
		}

		if (!spare_capacity(ch, obj, false))
		{
			send_to_char("This area is too small and already too full to contain that item.\n", ch);
			return;
		}

		act ("You begin looking for a hiding spot for $p.",
             false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

        ch->delay_type = DEL_HIDE_OBJ;
        ch->delay_info1 = (long int) obj;
        ch->delay = 5;

        return;
    }

    if (IS_ENCUMBERED (ch))
    {
        send_to_char ("You are too encumbered to hide.\n", ch);
        return;
    }

    if (do_group_size(ch) > 9 && !ch->following)
    {
        send_to_char("You cannot hide with a group this size following you.\n", ch);
        return;
    }

    // If you're being watched, you can't even attempt to hide.

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if ((af = get_affect(tch, MAGIC_WATCH)))
        {
            if ((CHAR_DATA *) af->a.spell.t == ch)
            {
                if (tch->following != ch && ch->following != tch && (ch->following && tch->following ? ch->following != tch->following : 1))
                {
                    send_to_char ("You are being observed too closely by those in the room to attempt to hide.\n", ch);
                    return;
                }
            }
        }
    }

	if (ch->targeted_by)
	{
		send_to_char ("You are being observed too closely to attempt to hide.\n", ch);
        return;
	}

	sprintf(buf, "You start trying to conceal yourself.");

    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    act("$n begins to conceal $mself.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    delay = 14 - (int) (skill_level(ch, SKILL_HIDE, 0) / 10);
    delay = MAX(4,delay);

    ch->delay = delay;
    ch->delay_type = DEL_HIDE;
}

void
delayed_hide (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    CHAR_DATA *tch;
    AFFECTED_TYPE *af;

    ch->delay_type = 0;

	if (ch->targeted_by)
	{
		send_to_char ("You are being observed too closely to attempt to hide.\n", ch);
        return;
	}

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if ((af = get_affect(tch, MAGIC_WATCH)))
        {
            if ((CHAR_DATA *) af->a.spell.t == ch)
            {
                if (tch->following != ch && ch->following != tch && (ch->following && tch->following ? ch->following != tch->following : 1))
                {
                    send_to_char ("You are being observed too closely by those in the room to attempt to hide.\n", ch);
                    return;
                }
            }
        }
    }

    send_to_char ("You settle down in what looks like a good spot.\n", ch);

    if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_LIGHT)
        obj = ch->right_hand;

    if (obj)
    {
        if (obj->o.light.hours > 0 && obj->o.light.on)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.light.on = 0;
        }
        obj = NULL;
    }

    if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_LIGHT)
        obj = ch->left_hand;

    if (obj)
    {
        if (obj->o.light.hours > 0 && obj->o.light.on)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.light.on = 0;
        }
        obj = NULL;
    }

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_E_LIGHT)
        obj = ch->right_hand;

    if (obj)
    {
        if (obj->o.e_light.status > 0)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.e_light.status = 0;
        }
        obj = NULL;
    }

	if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_E_LIGHT)
        obj = ch->left_hand;

    if (obj)
    {
        if (obj->o.e_light.status > 0)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.e_light.status = 0;
        }
        obj = NULL;
    }

    for (obj = ch->equip; obj; obj = obj->next_content)
    {
        if (obj->obj_flags.type_flag != ITEM_LIGHT && obj->obj_flags.type_flag != ITEM_CHEM_SMOKE)
            continue;

        if (obj->o.light.hours > 0 && obj->o.light.on)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.light.on = 0;
        }
    }

    for (obj = ch->equip; obj; obj = obj->next_content)
    {
        if (obj->obj_flags.type_flag != ITEM_E_LIGHT)
            continue;

        if (obj->o.elecs.status)
        {

            act ("You put out $p so you won't be detected.",
                 false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

            obj->o.elecs.status = 0;
        }
    }

    if (get_affect (ch, MAGIC_HIDDEN))
    {
        room_light (ch->room);
        return;
    }


	// Hide criteria:
	// you can buff if up to 10 by just typing hide: doesn't need anyone in the room.
	// to get it up to 20, there needs to be someone in this room, or in an adjacent room.
	// to get it up to 40, there needs to be 2 such people
	// to get it up to 50, there needs to be 1 PC or something aggro in this room or an adjacent room
	// to get it up to 60, there needs to be either 2 PC or something aggressive in this room or an adjacent room.
	// to get it up to 70, there needs to be either a PC or something aggressive in the room with you.
	// to get it up above 75, there needs to be something aggressive in the room with you.

	if (ch->skills[SKILL_HIDE] < 10)
	{
		skill_use (ch, SKILL_HIDE, 0);
	}
	else
	{
	    int char_seen = 0;
		CHAR_DATA *tch = NULL;
		ROOM_DIRECTION_DATA *exit = NULL;
		ROOM_DATA *next_room = NULL;

		for (int dir = 0; dir <= LAST_DIR;dir++)
		{
			if ((exit = EXIT (ch, dir))
					&& !IS_SET (exit->exit_info, EX_CLOSED)
					&& (next_room = vnum_to_room (exit->to_room)))
			{
	            if (next_room->people)
				{
					for (tch = next_room->people; tch; tch = tch->next_in_room)
					{
						if (could_see(ch, tch))
						{
							if (ch->skills[SKILL_HIDE] < 20)
							{
								char_seen ++;
							}
							else
							{
								if (tch->pc || would_attack(ch, tch))
								{
									char_seen ++;
								}
							}
						}
					}

				}
			}
		}

		if (ch->skills[SKILL_HIDE] > 70)
		{
			char_seen = 0;
		}

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;

			if (could_see(ch, tch))
			{
				if (ch->skills[SKILL_HIDE] < 20)
				{
					char_seen ++;
				}
				else
				{
					if (tch->pc || would_attack(ch, tch))
					{
						char_seen ++;
					}
				}
			}
		}


		if (ch->skills[SKILL_HIDE] < 20)
		{
			if (char_seen >= 1)
				skill_use(ch, SKILL_HIDE, 0);
		}
		else if (ch->skills[SKILL_HIDE] < 40)
		{
			if (char_seen >= 2)
				skill_use(ch, SKILL_HIDE, 0);
		}
		else if (ch->skills[SKILL_HIDE] < 50)
		{
			if (char_seen >= 3)
				skill_use(ch, SKILL_HIDE, 0);
		}
		else if (ch->skills[SKILL_HIDE] < 60)
		{
			if (char_seen >= 4)
				skill_use(ch, SKILL_HIDE, 0);
		}
		else
		{
			if (char_seen >= 5)
				skill_use(ch, SKILL_HIDE, 0);
		}
    }

    magic_add_affect (ch, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
    sprintf (buf, "[%s hides]", ch->tname);
    act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);

    room_light (ch->room);
}

void
delayed_hide_obj (CHAR_DATA * ch)
{
    OBJ_DATA *obj;
    AFFECTED_TYPE *af;

    obj = (OBJ_DATA *) ch->delay_info1;

    if (obj != ch->right_hand && obj != ch->left_hand)
    {
        send_to_char ("You don't have whatever you were hiding anymore.\n", ch);
        return;
    }

    obj_from_char (&obj, 0);

    remove_obj_mult_affect (obj, MAGIC_HIDDEN);	/* Probably doesn't exist */

    af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

    af->type = MAGIC_HIDDEN;
    af->a.hidden.duration = -1;
    af->a.hidden.hidden_value = ch->skills[SKILL_HIDE];
    af->a.hidden.coldload_id = ch->coldload_id;

    act ("You hide $p.", false, ch, obj, 0, TO_CHAR);

    af->next = obj->xaffected;
    obj->xaffected = af;
    obj_to_room (obj, ch->in_room);

    act ("$n hides $p.", false, ch, obj, 0, TO_ROOM);
}

/**********************************************************************
 * CASE 1:
 * Usage: palm <item>
 *        will get item from room.
 *             Uses Steal
 *
 * CASE 2:
 * Usage: palm <item> into <container>
 *        will put item into container in the room, including tables
 *              Uses Steal
 *
 * CASE 3:
 * Usage: palm <item> into <targetPC> <contaienr>
 *        will put item into container worn by PC (target or self)
 *             Uses Steal if PC is target
 *             Uses Steal if PC is self
 *
 * CASE 4:
 * Usage: palm <item> from <container>
 *        takes item from container worn by PC
 *             Uses Steal
 *
 * CASE 5:
 * Usage: palm <item> from <targetPC> <container>
 *        takes item from container worn by PC
 *             Uses Steal if PC is self
 *
 * Note: Must use STEAL command if taking from a container worn
 * by targetPC.
 *
 **********************************************************************/

void
do_palm (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    char contbuf[MAX_STRING_LENGTH];
    char msgbuf[MAX_STRING_LENGTH];
    char objtarget[MAX_STRING_LENGTH];
    CHAR_DATA *tch;
    OBJ_DATA *cobj = NULL;  //container
    OBJ_DATA *tobj = NULL;  //item being palmed
    AFFECTED_TYPE *af;
    int modifier = 0;
    bool into = false;
    bool from = false;
    bool targchk = false;  //is there a target PC?
    bool contchk = false;  //is there a container?

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

    if (real_skill(ch, SKILL_HIDE) && ch->skills[SKILL_HIDE] >= 10)
        skill_learn(ch, SKILL_STEAL);

    if (!real_skill (ch, SKILL_STEAL) && !real_skill (ch, SKILL_STEAL))
    {
        send_to_char ("You lack the skill to palm objects.\n\r", ch);
        return;
    }

	int skill_cap = ch->skills[SKILL_STEAL];

    argument = one_argument (argument, objtarget);

    if (!*objtarget)
    {
        send_to_char ("What did you wish to palm?\n", ch);
        return;
    }

    if (ch->right_hand && ch->left_hand)
    {
        send_to_char
        ("One of your hands needs to be free before attempting to palm something.\n",
         ch);
        return;
    }

    argument = one_argument (argument, buf); //into or from

    if (!*buf)
    {
        /****** CASE 1: palm <item> (from room) *******/
        if (!(tobj = get_obj_in_list_vis (ch, objtarget, ch->room->contents)))
        {
            send_to_char ("You don't see that here.\n", ch);
            return;
        }

        if (tobj->obj_flags.weight / 100 > 3)
        {
            send_to_char ("That's too heavy for you to pick up very stealthily.\n", ch);
            return;
        }

        if (!IS_SET (tobj->obj_flags.wear_flags, ITEM_TAKE))
        {
            send_to_char ("That cannot be picked up.\n", ch);
            return;
        }

        obj_from_room (&tobj, 0);
        clear_omote (tobj);
        act ("You carefully attempt to palm $p.", false, ch, tobj, 0,
             TO_CHAR | _ACT_FORMAT);

        /* Alert the staff of the theft */
        sprintf (msgbuf, "#3[Guardian: %s%s]#0 Tries to palm %s in %d.",
                 GET_NAME (ch),
                 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                 tobj->short_description,
                 ch->in_room);
        send_to_guardians (msgbuf, 0xFF);

        if (!skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
        {
            act ("$n attempts to surreptitiously take $p.", false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
        }

        obj_to_char (tobj, ch);
        clear_moves (ch);
        clear_current_move (ch);
		if (skill_cap == 40 && ch->skills[SKILL_STEAL] > 40)
		{
			ch->skills[SKILL_STEAL] = skill_cap;
		}

        return;
    } //CASE 1:


    /****** CASE 2, 3, 4 & 5:
    	palm <item> [from | into] [target?] <container>
    *****/
    if (!str_cmp (buf, "into"))
    {
        //container/target name
        argument = one_argument (argument, contbuf);
        into = true;
    }
    else if (!str_cmp (buf, "from"))
    {
        //container/target name
        argument = one_argument (argument, contbuf);
        from = true;
    }
    else
    {
        if (!from & !into)
        {
            send_to_char ("Do you wish to palm INTO or FROM a container?\n", ch);
            return;
        }
    }

    if (!(tch = get_char_room_vis (ch, contbuf))) //contbuf is not PC, so it may be a container CASE 2 or 4
    {
        if (!(cobj = get_obj_in_list (contbuf, ch->equip)) && !(cobj = get_obj_in_list_vis (ch, contbuf, ch->room->contents)))
        {
            send_to_char ("You don't see that here.\n", ch);
            return;
        }

        if (cobj == get_obj_in_list (contbuf, ch->equip))
        {
            tch =  get_char_room_vis (ch, "self");
            targchk = true; //self is the target PC CASE 3 & 5
            contchk = true;
        }

        if (cobj == get_obj_in_list_vis (ch, contbuf, ch->room->contents))
        {
            targchk = false;
            contchk = true;
        }

        if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
        {
            send_to_char ("You can only palm items into or from containers.\n", ch);
            return;
        }

    }
    else
    {
        targchk = true; //there is a target PC CASE 3 & 5
    }

    /*** CASE 2: palm <item> into <container> ***/
// item is tobj
// cobj is container
    if (into && !targchk && contchk)
    {
        if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
                && !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
        {
            send_to_char ("What did you wish to palm into it?\n", ch);
            return;
        }

        if (tobj->obj_flags.weight / 100 > 3)
        {
            send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
            return;
        }

        //Treat tables as a special container
        if (!IS_TABLE(cobj))
        {
            act ("You carefully slide $p onto $P.", false, ch, tobj,
                 cobj, TO_CHAR | _ACT_FORMAT);

            /* Alert the staff of the theft */
            sprintf (msgbuf,
                     "#3[Guardian: %s%s]#0 Tries to slip %s onto %s in %d.",
                     GET_NAME (ch),
                     IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                     tobj->short_description,
                     cobj->short_description,
                     ch->in_room);
            send_to_guardians (msgbuf, 0xFF);

            if (!skill_use
                    (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
                act ("$n attempts to surreptitiously place $p atop $P.",
                     false, ch, tobj, cobj, TO_ROOM | _ACT_FORMAT);
        }
        else
        {
            act ("You carefully slide $p into $P.", false, ch, tobj,
                 cobj, TO_CHAR | _ACT_FORMAT);

            /* Alert the staff of the theft */
            sprintf (msgbuf,
                     "#3[Guardian: %s%s]#0 Tries to slip %s into %s in %d.",
                     GET_NAME (ch),
                     IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                     tobj->short_description,
                     cobj->short_description,
                     ch->in_room);
            send_to_guardians (msgbuf, 0xFF);

            if (!skill_use
                    (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
                act ("$n attempts to surreptitiously place $p into $P.",
                     false, ch, tobj, cobj, TO_ROOM | _ACT_FORMAT);
        }

        //transfer any special effects
        for (af = tobj->xaffected; af; af = af->next)
            affect_modify (ch, af->type, af->a.spell.location,
                           af->a.spell.modifier, tobj->obj_flags.bitvector,
                           false, 0);

        if (ch->right_hand == tobj)
            ch->right_hand = NULL;
        else if (ch->left_hand == tobj)
            ch->left_hand = NULL;
        obj_to_obj (tobj, cobj);

		if (skill_cap == 40 && ch->skills[SKILL_STEAL] > 40)
		{
			ch->skills[SKILL_STEAL] = skill_cap;
		}

        return;
    }//CASE 2

    /*** CASE 3: palm <item> into <targetPC> <container> ***/
// item is tobj
// tch is targetPC
// cobj is container
    if (into && targchk)
    {
        argument = one_argument (argument, contbuf);

        if (!contchk)
        {
            if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
            {
                send_to_char ("You don't see that container.\n", ch);
                return;
            }

            if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
            {
                send_to_char ("You can only palm items into containers.\n", ch);
                return;
            }
        }

        if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
                && !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
        {
            send_to_char ("What did you wish to palm into it?\n", ch);
            return;
        }

        if (tobj->obj_flags.weight / 100 > 3)
        {
            send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
            return;
        }

        if (tch == ch)
        {
            act ("You carefully slide $p into $P.", false, ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);

            /* Alert the staff of the theft */
            sprintf (msgbuf,
                     "#3[Guardian: %s%s]#0 Tries to secretly place %s into %s in %d.",
                     GET_NAME (ch),
                     IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                     tobj->short_description,
                     cobj->short_description,
                     ch->in_room);
            send_to_guardians (msgbuf, 0xFF);


            if (!skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
                act ("$n attempts to surreptitiously manipulate $p.",
                     false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);


        }
        else
        {
            modifier = tch->intel;
            modifier += tobj->obj_flags.weight / 100;
            modifier += 10;

            sprintf (msgbuf, "You carefully slide $p into #5%s#0's $P.",
                     char_short (tch));
            act (msgbuf, false, ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);

            /* Alert the staff of the theft */
            sprintf (msgbuf, "#3[Guardian: %s%s]#0 Plants %s on %s in %d.",
                     GET_NAME (ch),
                     IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                     tobj->short_description,
                     (!IS_NPC (tch)) ? GET_NAME (tch) : (tch->short_descr),
                     ch->in_room);
            send_to_guardians (msgbuf, 0xFF);

            if (!skill_use (ch, SKILL_STEAL, modifier))
            {
                sprintf (msgbuf,
                         "$n approaches you and surreptitiously slips $p into #2%s#0!",
                         obj_short_desc (cobj));
                act (msgbuf, false, ch, tobj, tch, TO_VICT | _ACT_FORMAT);

                sprintf (msgbuf,
                         "$n approaches $N and surreptitiously slips $p into #2%s#0.",
                         obj_short_desc (cobj));
                act (msgbuf, false, ch, tobj, tch, TO_NOTVICT | _ACT_FORMAT);
            }
        }

        for (af = tobj->xaffected; af; af = af->next)
            affect_modify (ch, af->type, af->a.spell.location,
                           af->a.spell.modifier, tobj->obj_flags.bitvector,
                           false, 0);

        if (ch->right_hand == tobj)
            ch->right_hand = NULL;
        else if (ch->left_hand == tobj)
            ch->left_hand = NULL;
        obj_to_obj (tobj, cobj);

		if (skill_cap == 40 && ch->skills[SKILL_STEAL] > 40)
		{
			ch->skills[SKILL_STEAL] = skill_cap;
		}

        return;
    } //CASE 3

    /*** CASE 4: palm <item> from <container> ***/
// item is tobj
// cobj is container
    if (from && !targchk && contchk)
    {
        if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
        {
            send_to_char ("You don't see such an item in that container.\n",
                          ch);
            return;
        }

        if (tobj->obj_flags.weight / 100 > 3)
        {
            send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
            return;
        }

        if (ch->right_hand && ch->left_hand)
        {
            send_to_char
            ("One of your hands needs to be free before attempting to palm something.\n",
             ch);
            return;
        }

        act ("You carefully flick $p from $P into your hand.", false,
             ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);

        /* Alert the staff of the theft */
        sprintf (msgbuf,
                 "#3[Guardian: %s%s]#0 Tries to secretly draw %s in %d.",
                 GET_NAME (ch),
                 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                 tobj->short_description,
                 ch->in_room);
        send_to_guardians (msgbuf, 0xFF);

        if (!skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
            act ("$n attempts to handle $p surreptitiously.", false, ch,
                 tobj, 0, TO_ROOM | _ACT_FORMAT);


        obj_from_obj (&tobj, 0); //rmoves tobj from wherever it is
        obj_to_char (tobj, ch);

		if (skill_cap == 40 && ch->skills[SKILL_STEAL] > 40)
		{
			ch->skills[SKILL_STEAL] = skill_cap;
		}

        return;

    }// CASE 4


    /*** CASE 5: palm <item> from <targetPC> <container> ***/
// item is tobj
// tch is targetPC
// cobj is container

    if (from && targchk)
    {
        argument = one_argument (argument, contbuf);

        if (!contchk)
        {
            if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
            {
                send_to_char
                ("What did you wish to palm from?\n", ch);
                return;
            }

            if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
            {
                send_to_char ("You can only palm items from containers.\n", ch);
                return;
            }
        }

        if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
        {
            send_to_char ("You don't see such an item in that container.\n",
                          ch);
            return;
        }

        if (tobj->obj_flags.weight / 100 > 3)
        {
            send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
            return;
        }

        if (ch->right_hand && ch->left_hand)
        {
            send_to_char
            ("One of your hands needs to be free before attempting to palm something.\n",
             ch);
            return;
        }

        if (tch == ch)
        {
            act ("You carefully attempt to palm $p from $P.", false, ch,
                 tobj, cobj, TO_CHAR | _ACT_FORMAT);

            /* Alert the staff of the theft */
            sprintf (msgbuf,
                     "#3[Guardian: %s%s]#0 Tries to palm %s from %s in %d.",
                     GET_NAME (ch),
                     IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                     tobj->short_description,
                     cobj->short_description,
                     ch->in_room);
            send_to_guardians (msgbuf, 0xFF);

            if (!skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight / 100))
                act ("$n gets $p from $P.", false, ch, tobj, cobj, 		     TO_ROOM | _ACT_FORMAT);

            obj_from_obj (&tobj, 0); //removes tobj from wherever it is
            obj_to_char (tobj, ch);

			if (skill_cap == 40 && ch->skills[SKILL_STEAL] > 40)
			{
				ch->skills[SKILL_STEAL] = skill_cap;
			}

        } //tch = ch
        else
        {
            //must use the steal command. Besides, you can't see the item in the other guys container
            send_to_char ("You can't see into that container.\n",
                          ch);
            return;
        }
        return;
    } // CASE 5

    return;
}//end function

void
do_steal (CHAR_DATA * ch, char *argument, int cmd)
{
    char target[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    char obj_name[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tobj = NULL;
    int i, j, obj_num, amount = 0, modifier = 0, count = 0;

    sprintf (command, "%s", argument);

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

    if (real_skill(ch, SKILL_HIDE) && ch->skills[SKILL_HIDE] >= 10)
        skill_learn(ch, SKILL_STEAL);

    if (!real_skill (ch, SKILL_STEAL))
    {
        send_to_char ("You lack the skill to steal.\n\r", ch);
        return;
    }

    if (ch->right_hand || ch->left_hand)
    {
        send_to_char
        ("Both your hands need to be free before attempting to steal something.\n",
         ch);
        return;
    }

    argument = one_argument (argument, target);

    if (!*target)
    {
        send_to_char ("From whom did you wish to steal?\n", ch);
        return;
    }

    if (*target)
    {
        if (!(victim = get_char_room_vis (ch, target)))
        {
            send_to_char ("Steal from whom?", ch);
            return;
        }

        if (victim == ch)
        {
            send_to_char ("You can't steal from yourself!\n\r", ch);
            return;
        }

        if (victim->fighting)
        {
            act ("$N's moving around too much for you to attempt a grab.",
                 false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            return;
        }

		if (IS_SET(ch->room->room_flags, PEACE))
		{
			act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
	        return;
		}

        if (!IS_NPC (victim) && !IS_MORTAL (victim) && !victim->pc->mortal_mode)
        {
            send_to_char
            ("The immortal cowers as you approach.  You just don't have the heart!\n\r",
             ch);
            return;
        }

        argument = one_argument (argument, obj_name);
        if (*obj_name && *obj_name != '!')
        {

            if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
                    && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!')
            {
                sprintf (buf,
                         "You are in a lawful area. If you're caught stealing, "
                         "you may be killed or imprisoned. To confirm, "
                         "type \'#6steal %s !#0\', without the quotes.",
                         command);
                act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                return;
            }

            if ((tobj = get_equip (victim, WEAR_BELT_1)) &&
                    CAN_SEE_OBJ (ch, tobj) && isname (obj_name, tobj->name))
                obj = tobj;

            else if ((tobj = get_equip (victim, WEAR_BELT_2)) &&
                     CAN_SEE_OBJ (ch, tobj) && isname (obj_name, tobj->name))
                obj = tobj;

            if (!obj)
            {
                send_to_char ("You don't see that on their belt.\n", ch);
                return;
            }

            modifier = victim->intel / 5;
            modifier += obj->obj_flags.weight / 100;
            modifier += 10;

            /* Alert the staff of the theft */
            notify_guardians (ch, victim, 6);

            if (skill_use (ch, SKILL_STEAL, modifier))
            {
                act
                ("You approach $N, deftly slipping $p from $S belt and moving off before you can be noticed.",
                 false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
                unequip_char (victim, obj->location);
                obj_to_char (obj, ch);
                return;
            }
            else
            {
                if ((skill_use (victim, SKILL_STEAL, -10)) || (victim->intel > number(1,30)))
                {
                    act
                    ("You approach $N cautiously, but at the last moment, before you can make the grab, $E glances down and notices your attempt!",
                     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
                    act
                    ("Suddenly, you glance down, and notice $m attempting to lift $p from your belt!",
                     false, ch, obj, victim, TO_VICT | _ACT_FORMAT);
                    send_to_char ("\n", ch);
                    criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
                    return;
                }
                else
                {
                    act
                    ("You approach $N cautiously, but at the last moment $E turns away, and your attempt is stymied. Thankfully, however, you have not been noticed.",
                     false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
                    return;
                }
            }
        }
        else
        {

            if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
                    && IS_SET (ch->room->room_flags, LAWFUL) && *obj_name != '!')
            {
                sprintf (buf,
                         "You are in a lawful area. If you're caught stealing, "
                         "you may be killed or imprisoned. To confirm, "
                         "type \'#6steal %s !#0\', without the quotes.",
                         command);
                act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                return;
            }

            for (obj = victim->equip; obj; obj = obj->next_content)
                if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER
                        && !CAN_WEAR (obj, ITEM_WEAR_FEET)
                        && !CAN_WEAR (obj, ITEM_WEAR_WRIST)
                        && !CAN_WEAR (obj, ITEM_WEAR_ANKLE))
                    break;

            if (!obj)
            {
                send_to_char
                ("They don't seem to be wearing any containers you can pilfer from.\n",
                 ch);
                return;
            }

            i = 0;

            for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                i++;

            obj_num = number (0, i);

            j = 0;

            for (tobj = obj->contains; tobj; tobj = tobj->next_content, j++)
                if (j == obj_num)
                    break;


            modifier = victim->intel/2;

            if (tobj)
                modifier += tobj->obj_flags.weight;

            if (IS_SET (obj->o.container.flags, CONT_CLOSED))
                modifier += 15;

            /* Alert the staff of the theft */
            notify_guardians (ch, victim, 6);

            if (skill_use (ch, SKILL_STEAL, modifier))
            {
                if (tobj)
                {
                    act
                    ("You approach $N cautiously, slipping a hand inside $p. A moment later you move stealthily away, having successfully lifted something!",
                     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
                    if (tobj->count >= 6)
                    {
                        amount = number (1, 6);
                    }
                    else
                    {
                        count = tobj->count;
                        count = MAX (1, tobj->count);
                        amount = number (1, count);
                    }
                    obj_from_obj (&tobj, amount);
                    obj_to_char (tobj, ch);
                    /*
                       if ( IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
                       obj_activate (ch, obj);
                     */
                    /*
                       tobj->count -= amount;
                       new_obj = load_object(tobj->nVirtual);
                       new_obj->count = amount;
                       obj_to_char (new_obj, ch);
                       if ( tobj->count <= 0 ) {
                       tobj->count = 1;
                       extract_obj(tobj);
                       }
                       }
                       else {
                       count = tobj->count;
                       count = MAX (1, tobj->count);
                       amount = number(1,count);
                       tobj->count -= amount;
                       new_obj = load_object(tobj->nVirtual);
                       new_obj->count = amount;
                       obj_to_char (new_obj, ch);
                       if ( tobj->count <= 0 ) {
                       tobj->count = 1;
                       extract_obj (tobj);
                       }
                       }
                     */
                    return;
                }
                else
                {
                    act
                    ("You approach $N cautiously, slipping your hand stealthily into $p. A moment later you withdraw, having been unable to find anything to lift, though your attempt has gone unnoticed.",
                     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
                    return;
                }
            }
            else
            {
                if ((skill_use (victim, SKILL_STEAL, -10)) || (victim->intel > number(1,30)))
                {
                    act
                    ("You approach $N cautiously, moving to slip your hand into $p. At the last moment, $E glances down, noticing your attempt!",
                     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
                    act
                    ("You glance down, your gaze having been attracted by a hint of movement; you notice $n's hand moving toward $p, in an attempt to pilfer from you!",
                     false, ch, obj, victim, TO_VICT | _ACT_FORMAT);
                    send_to_char ("\n", ch);
                    criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
                    return;
                }
                else
                {
                    act
                    ("You approach $N cautiously, but at the last moment $E turns away, fate having allowed $M to evade your attempt. Thankfully, it would seem that you were unnoticed.",
                     false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
                    return;
                }
            }
        }
    }
}

void
post_idea (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *ch;
    char msg[MAX_STRING_LENGTH];
    char *date;
    char *date2;
    time_t account_time;

    account_time = d->acct->created_on;
    date = (char *) asctime (localtime (&account_time));

    account_time = time (0);
    date2 = (char *) asctime (localtime (&account_time));

    date2[strlen (date2) - 1] = '\0';

    ch = d->character;
    if (!*d->pending_message->message)
    {
        send_to_char ("No suggestion report entered.\n", ch);
        mem_free (date);
        mem_free (date2);
        return;
    }

    sprintf (msg, "From: %s [%d]\n", ch->tname, ch->in_room);
    sprintf (msg + strlen (msg), "\n");
    sprintf (msg + strlen (msg), "%s", ch->descr()->pending_message->message);

    add_message (1, "Ideas",
                 -5,
                 ch->descr()->acct->name.c_str (),
                 date2, "Player Suggestion", "", msg, 0);

    send_to_char ("Thank you! Your suggestion has been recorded.\n\r", ch);

    unload_message (d->pending_message);

    mem_free (date);
    mem_free (date2);
}

void
do_idea (CHAR_DATA * ch, char *argument, int cmd)
{

    send_to_char
    ("Please submit your ideas for consideration on our web forum, located at:\n\n"
     "   #6forum.parallelrpi.com#0\n", ch);
    return;

}


const char *
get_room_desc_tag (CHAR_DATA * ch, ROOM_DATA * room)
{
    /*
    if (IS_MORTAL (ch)
        && weather_info[room->zone].state == HEAVY_SNOW
        && !IS_SET (room->room_flags, INDOORS))
      {

        return NULL;

      }

    else*/
    if (!room->extra
            || desc_weather[room->zone] == WR_NORMAL
            || !room->extra->weather_desc[desc_weather[room->zone]])
    {

        if (room->extra && room->extra->weather_desc[WR_NIGHT] && !sun_light)
        {

            return weather_room[WR_NIGHT];
        }
        else
        {

            return NULL;

        }

    }
    else
    {
        return weather_room[desc_weather[room->zone]];
    }
}

void
post_typo (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *ch;
    char buf2[AVG_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    char *date;
    char *date2;
    const char *wr;
    time_t account_time;

    account_time = d->acct->created_on;
    date = (char *) asctime (localtime (&account_time));

    account_time = time (0);
    date2 = (char *) asctime (localtime (&account_time));

    date2[strlen (date2) - 1] = '\0';

    ch = d->character;
    if (!*d->pending_message->message)
    {
        send_to_char ("No typo report posted.\n", ch);
        mem_free (date);
        mem_free (date2);
        return;
    }

    if (ch->room->zone <= 99)
        sprintf (buf2, "%s", zone_table[ch->room->zone].name);
    else
        sprintf (buf2, "Auto-Generated Zone");

    wr = get_room_desc_tag (ch, ch->room);
    sprintf (msg, "From: %s [%d%s%s]\n", ch->tname, ch->in_room,
             wr ? " - rset " : "", wr ? wr : "");
    sprintf (msg + strlen (msg), "\n");
    sprintf (msg + strlen (msg), "%s", ch->descr()->pending_message->message);

    add_message (1, "Typos",
                 -5, ch->descr()->acct->name.c_str (), date2, buf2, "", msg, 0);

    send_to_char
    ("Thank you! Your typo report has been entered into our tracking system.\n\r",
     ch);

    unload_message (d->pending_message);

    mem_free (date);
    mem_free (date2);
}

void
do_typo (CHAR_DATA * ch, char *argument, int cmd)
{

    if (IS_NPC (ch))
    {
        send_to_char ("Mobs can't submit bug reports.\n\r", ch);
        return;
    }

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);
    send_to_char
    ("Enter a typo report to be submitted to the admins. Terminate\n"
     "the editor with an '@' symbol. Please note that your post\n"
     "will be stamped with all pertinent contact information; no\n"
     "need to include that in the body of your message. Thanks for\n"
     "doing your part to help improve our world!\n", ch);

    make_quiet (ch);

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_typo;
}

void
post_nominate (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *ch;
    char buf2[AVG_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    char *date;
    char *date2;
    const char *wr;
    time_t account_time;

    account_time = d->acct->created_on;
    date = (char *) asctime (localtime (&account_time));

    account_time = time (0);
    date2 = (char *) asctime (localtime (&account_time));

    date2[strlen (date2) - 1] = '\0';

    ch = d->character;
    if (!*d->pending_message->message)
    {
        send_to_char ("You must leave a reason for your nomination.\n", ch);
        mem_free (date);
        mem_free (date2);
        ch->delay_ch = NULL;
        return;
    }

    d->acct->nominate_rpp ();

    sprintf (buf2, "#6%s#0: %s", ch->delay_ch->tname, ch->delay_ch->pc->account_name);

    wr = get_room_desc_tag (ch, ch->room);
    sprintf (msg, "From: %s [%d%s%s]\n", ch->tname, ch->in_room,
             wr ? " - rset " : "", wr ? wr : "");
    sprintf (msg + strlen (msg), "\n");
    sprintf (msg + strlen (msg), "%s", ch->descr()->pending_message->message);

    add_message (1, "Nominations",
                 -5, ch->descr()->acct->name.c_str (), date2, buf2, "", msg, 0);

    send_to_char
    ("Thank you! Your nomination for a reward point will be reviewed by the staff.\n\r",
     ch);

    unload_message (d->pending_message);

    mem_free (date);
    mem_free (date2);
    ch->delay_ch = NULL;
}

void
do_nominate (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *tch;
    char buf[MAX_STRING_LENGTH];
    char original[MAX_STRING_LENGTH];
    account *acct;
    char verify[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        send_to_char ("Mobiles can't submit nominations.\n\r", ch);
        return;
    }

    sprintf (original, "nominate %s", argument);
    argument = one_argument(argument, buf);

    if (!*buf)
    {
        send_to_char ("Which PC would you like to nominate for a Reward Point?\n"
                      "To nominate an account, see #6HELP NOMINATE#0.\n", ch);
        return;
    }

    if (!(tch = get_char_room_vis (ch, buf)))
    {
        send_to_char("You must be in the same room as the PC to nominate them.\n", ch);
        return;
    }

    if (IS_NPC (tch) || IS_SET(tch->flags, FLAG_GUEST))
    {
        send_to_char ("This command only works for player characters.\n", ch);
        return;
    }

    if (tch == ch)
    {
        send_to_char ("Really? You think -you- deserve an award? Who'da thunk it?\n", ch);
        return;
    }

    argument = one_argument(argument, verify);

    if (*verify != '!')
    {
        sprintf(buf, "Are you sure you wish to nominate #5%s#0? Type #6%s !#0 to confirm\n", char_short(tch), original);
        act(buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    acct = new account (ch->pc->account_name);

    if (!acct->is_registered ())
    {
        delete acct;
        act("Hmm... there seems to be a problem with your account. Notify the staff.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if ((time (0) - acct->get_last_nominate ()) <= 60 * 60 * 24 * 1)
    {
        send_to_char("You've already made a nomination within the last day.\n", ch);
        return;
    }

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);
    send_to_char
    ("Describe why you believe the player in question deserves a\n"
     "reward. Typically, the admins give rewards to players who\n"
     "demonstrate strong roleplaying abilities, clan management\n"
     "achievements, assisting new players, contributing objects\n"
     "or documentation to the game, and so on.\n\n"
     "Terminate the editor with an '@' symbol, and remember you\n"
     "can only nominate once a month.\n", ch);

    make_quiet (ch);

    ch->delay_ch = tch;

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;
    ch->descr()->proc = post_nominate;

}


void
post_bug (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *ch;
    char buf2[MAX_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    char *date;
    char *date2;
    time_t account_time;

    account_time = d->acct->created_on;
    date = (char *) asctime (localtime (&account_time));

    account_time = time (0);
    date2 = (char *) asctime (localtime (&account_time));

    date2[strlen (date2) - 1] = '\0';

    ch = d->character;
    if (!*d->pending_message->message)
    {
        send_to_char ("No bug report posted.\n", ch);
        mem_free (date);
        mem_free (date2);
        return;
    }

    sprintf (buf2, "Bug Report");

    sprintf (msg, "From: %s [%d]\n", ch->tname, ch->in_room);
    sprintf (msg + strlen (msg), "\n");
    sprintf (msg + strlen (msg), "%s", ch->descr()->pending_message->message);

    add_message (1, "Bugs",
                 -5, ch->descr()->acct->name.c_str (), date2, buf2, "", msg, 0);

    send_to_char
    ("Thank you! Your bug report has been entered into our system.\n\r", ch);

    unload_message (d->pending_message);

    mem_free (date);
    mem_free (date2);
}

void
do_bug (CHAR_DATA * ch, char *argument, int cmd)
{

    send_to_char
    ("This command has been disabled; we have found in past experience that bug\n"
     "reports are best posted on our web forum, in case they aren't actually bugs.\n"
     "However, if you feel this is an urgent issue, please email the staff.\n",
     ch);
    return;
}

void
do_brief (CHAR_DATA * ch, char *argument, int cmd)
{
    send_to_char ("Sorry, but this command has been disabled.\n", ch);
}

void
do_compact (CHAR_DATA * ch, char *argument, int cmd)
{
    const char *message[2] =
    {
        "You are now in the uncompacted mode.\n",
        "You are now in compact mode.\n"
    };

    // toggle FLAG_COMPACT and test if it is ON or OFF.
    // deliver the appropriate message to the player
    send_to_char (message[((ch->flags ^= FLAG_COMPACT) & FLAG_COMPACT)], ch);
}

void
sa_roomshots (SECOND_AFFECT * sa)
{
    char buf[AVG_STRING_LENGTH * 2] = {'\0'};

    if (!is_he_somewhere (sa->ch))
        return;

    std::string output;

	if (sa->info2 == 1)
	{
		sprintf(buf, "#6A bullet");
	}
	else if (sa->info2 < 7)
	{
		sprintf(buf, "#6Bullets");
	}
	else if (sa->info2 < 14)
	{
		sprintf(buf, "#6A volley of bullets");
	}
	else
	{
		sprintf(buf, "#6A storm of bullets");
	}

	if (!number(0,4))
		sprintf(buf + strlen(buf), " zip%s past.#0\n", sa->info2 > 1 ? "" : "s");
	else if (!number(0,3))
		sprintf(buf + strlen(buf), " zoom%s by.#0\n", sa->info2 > 1 ? "" : "s");
	else if (!number(0,2))
		sprintf(buf + strlen(buf), " hurtle%s through.#0\n", sa->info2 > 1 ? "" : "s");
	else if (!number(0,1))
		sprintf(buf + strlen(buf), " whistle%s past.#0\n", sa->info2 > 1 ? "" : "s");
	else
		sprintf(buf + strlen(buf), " whir%s by.#0\n", sa->info2 > 1 ? "" : "s");


	send_to_char(buf, sa->ch);
}


void
sa_roomfired (SECOND_AFFECT * sa)
{
    char buf[AVG_STRING_LENGTH * 2] = {'\0'};
	char buf2[AVG_STRING_LENGTH] = {'\0'};

    if (!is_he_somewhere (sa->ch))
        return;

    std::string output;

	if (sa->info2 == 1)
	{
		sprintf(buf2, " gunshot is");
	}
	else if (sa->info2 < 14)
	{
		if (!number(0,4))
			sprintf(buf2, " burst of gunshots are");
		else if (!number(0,3))
			sprintf(buf2, " rattle of gunshots are");
		else if (!number(0,2))
			sprintf(buf2, " multitude of gunshots are");
		else if (!number(0,1))
			sprintf(buf2, " stacatto of gunshots are");
		else
			sprintf(buf2, "n volley of gunshots are");
	}
	else
	{
		if (!number(0,4))
			sprintf(buf2, " barrage of gunshots are");
		else if (!number(0,3))
			sprintf(buf2, " storm of gunshots are");
		else if (!number(0,2))
			sprintf(buf2, " terrible rain of gunshots are");
		else if (!number(0,1))
			sprintf(buf2, "n explosion of gunshots are");
		else
			sprintf(buf2, "n uproar of gunshots are");
	}

	sprintf(buf, "#6A%s being fired here.#0\n", buf2);

	send_to_char(buf, sa->ch);
}

void
sa_distshots (SECOND_AFFECT * sa)
{
    char buf[AVG_STRING_LENGTH * 2] = {'\0'};
	char buf2[AVG_STRING_LENGTH] = {'\0'};

    if (!is_he_somewhere (sa->ch))
        return;

    std::string output;

	if (sa->info2 == 1)
	{
		sprintf(buf2, " gunshot");
	}
	else if (sa->info2 < 14)
	{
		if (!number(0,4))
			sprintf(buf2, " burst of gunfire");
		else if (!number(0,3))
			sprintf(buf2, " rattle of gunfire");
		else if (!number(0,2))
			sprintf(buf2, " blast of gunfire");
		else if (!number(0,1))
			sprintf(buf2, " stacatto of gunfire");
		else
			sprintf(buf2, "n uproar of gunfire");
	}
	else
	{
		if (!number(0,4))
			sprintf(buf2, " cacophony of gunfire");
		else if (!number(0,3))
			sprintf(buf2, " barrage of gunfire");
		else if (!number(0,2))
			sprintf(buf2, " terrible rain of gunfire");
		else if (!number(0,1))
			sprintf(buf2, "n explosion of gunfire");
		else
			sprintf(buf2, "n uproar of gunfire");
	}

	sprintf(buf, "#6A%s can be heard here.#0\n", buf2);

	send_to_char(buf, sa->ch);
}

void
sa_involcover (SECOND_AFFECT * sa)
{
    if (!is_he_somewhere (sa->ch))
        return;

    if (GET_POS (sa->ch) == FIGHT || GET_POS (sa->ch) == STAND)
        return;

	if (sa->info2)
	{
		send_to_char("You've recovered your will enough to stand again.\n", sa->ch);
	}
}

void
sa_stand (SECOND_AFFECT * sa)
{
    if (!is_he_somewhere (sa->ch))
        return;

    if (GET_POS (sa->ch) == FIGHT || GET_POS (sa->ch) == STAND)
        return;

    do_stand (sa->ch, "", 0);
}

void
sa_grenade (SECOND_AFFECT *sa)
{
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char buffer[MAX_STRING_LENGTH] = {'\0'};
    char *p = '\0';
    int mag_thresh = 10;
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj = NULL;
    ROOM_DATA *room = NULL;


    if (!(obj = sa->obj))
        return;
    if (GET_ITEM_TYPE(obj) != ITEM_GRENADE)
        return;
    if (obj->o.grenade.status != 1)
        return;

    mag_thresh = MAX(1, obj->o.grenade.magnitude);

    // Now let's figure out where the grenade is: either someone can be holding it,
    // or it's in the room (we'll code disallow putting a live grenade in an object)

    if (obj->in_room && (room = vnum_to_room(obj->in_room)))
    {
        int people_count = 0;
        int people_pass = 0;
        int hit_count = mag_thresh;

        send_to_gods("Grenade in room.");
        sprintf(buf, "%s#0 explodes with a deafining boom and a shower of red-hot shrapnel!", obj_short_desc(obj));
        *buf = toupper(*buf);
        sprintf(buffer, "#2%s", buf);
        reformat_string (buffer, &p);
        sprintf (buffer, "%s", p);
        mem_free (p); // char*
        send_to_room(buffer, room->vnum);

        for (tch = room->people; tch; tch = tch->next_in_room)
        {
            people_count++;
        }

        for (tch = room->people; tch; tch = tch->next_in_room, people_pass++)
        {
            bool broken_loop = false;
            if (number(1, people_count - people_pass) <= hit_count)
            {
                AFFECTED_TYPE *af = NULL;
                for (af = tch->hour_affects; af; af = af->next)
                {
                    if (af->type == AFFECT_COVER && af->a.cover.direction == 6)
                    {
                        if (number(1,100) <= (af->a.cover.value >= 2 ? 99 : af->a.cover.value == 1 ? 95 : 90))
                        {
                            if (af->a.cover.obj && is_obj_in_list (af->a.cover.obj, tch->room->contents))
                                act ("Flying chunks of shrapnel imbed themselves into $p!", false, tch, af->a.cover.obj, 0, TO_CHAR | _ACT_FORMAT);
                            else
                                act ("Flying chunks of shrapnel imbed themselves into the cover you hide behind!", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
                            broken_loop = true;
                            break;
                        }
                    }
                }

                hit_count --;
                if (broken_loop)
                    continue;

                act ("You are lacerated by some of the flying chunks of shrapnel!", false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);

                for (int j = 0; j < number(1,3); j++)
                {
                    int damage = 0;
                    int location = 0;
                    char loc[AVG_STRING_LENGTH] = {'\0'};
                    int i = number (1, 100);
                    while (i > 0)
                        i = i - body_tab[MELEE_TABLE][++location].percent;
                    damage = dice (2, 5) + 2;
                    damage = real_damage(tch, damage, &location, 0, 0);
                    sprintf (loc, "%s", figure_location (tch, location));
                    damage_objects(tch, damage, loc, location, 0, MELEE_TABLE, false);
                    wound_to_char(tch, loc, damage, 0, 0, 0, 0);
                }
            }

            if (hit_count <= 0)
                break;
        }

        extract_obj(obj);
    }
    else if (obj->carried_by && (tch = obj->carried_by) && tch->room)
    {
        send_to_gods("Grenade held by doofus.");
        act ("$p held by $n explodes with a deafining boom, driving red-hot shrapnel into $s body.", false, tch, obj, 0, TO_ROOM | _ACT_FORMAT);
        act ("$p held by you explodes with a deafining boom, driving red-hot shrapnel into your body.", false, tch, obj, 0, TO_CHAR | _ACT_FORMAT);

        // You take ten wounds if you're lucky enough to be holding the grenade.
        for (int j = 0; j < mag_thresh; j++)
        {
            int damage = 0;
            int location = 0;
            char loc[AVG_STRING_LENGTH] = {'\0'};
            int i = number (1, 100);
            while (i > 0)
                i = i - body_tab[MELEE_TABLE][++location].percent;
            damage = dice (2, 5) + 2;
            damage = real_damage(tch, damage, &location, 0, 0);
            sprintf (loc, "%s", figure_location (tch, location));
            damage_objects(tch, damage, loc, location, 0, MELEE_TABLE, false);
            wound_to_char(tch, loc, damage, 0, 0, 0, 0);
        }
        extract_obj(obj);
    }

}


void
sa_get (SECOND_AFFECT * sa)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_he_somewhere (sa->ch))
        return;

    sa->obj->tmp_flags &= ~SA_DROPPED;

    if (sa->obj == sa->ch->right_hand || sa->obj == sa->ch->left_hand)
        return;

    sprintf (buf, "get .c %d", sa->obj->coldload_id);

    command_interpreter (sa->ch, buf);
}

void
sa_wear (SECOND_AFFECT * sa)
{
    int num;
    char buf[MAX_STRING_LENGTH];

    if (!is_he_somewhere (sa->ch))
        return;

    if (!(num = is_obj_in_list (sa->obj, sa->ch->right_hand)) &&
            !(num = is_obj_in_list (sa->obj, sa->ch->left_hand)))
    {
        if (!IS_SET (sa->ch->flags, FLAG_COMPETE) ||
                !is_obj_in_list (sa->obj, sa->ch->room->contents))
            return;

        extract_obj (sa->obj);

        return;
    }

    if (CAN_WEAR (sa->obj, ITEM_WIELD))
        sprintf (buf, "wield .c %d", sa->obj->coldload_id);
    else
        sprintf (buf, "wear .c %d", sa->obj->coldload_id);

    if (!CAN_WEAR (sa->obj, ITEM_WEAR_SHIELD)
            && GET_ITEM_TYPE (sa->obj) != ITEM_SHIELD)
        command_interpreter (sa->ch, buf);
}

void
sa_close_door (SECOND_AFFECT * sa)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_he_somewhere (sa->ch))
        return;

    sprintf (buf, "close %s", sa->info);
    command_interpreter (sa->ch, buf);

    sprintf (buf, "lock %s", sa->info);
    command_interpreter (sa->ch, buf);
}

void
sa_combat_door (SECOND_AFFECT * sa)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_he_somewhere (sa->ch))
        return;

    // We've left the room - probably fled.
    if (sa->ch->in_room != sa->info2)
        return;

    // If we're not fighting, don't worry about it.
    if (!sa->ch->fighting)
        return;

    sprintf(buf, "%s", sa->info);
    command_interpreter (sa->ch, buf);
}

void
sa_knock_out (SECOND_AFFECT * sa)
{
    if (!is_he_somewhere (sa->ch))
        return;

    if (GET_POS (sa->ch) != SLEEP)
        return;

    if (get_affect (sa->ch, MAGIC_AFFECT_SLEEP))
        return;

    GET_POS (sa->ch) = REST;

    send_to_char ("You regain consciousness, flat on the ground.", sa->ch);
    act ("$n regains consciousness.", false, sa->ch, 0, 0, TO_ROOM);
}

void
sa_move (SECOND_AFFECT * sa)
{
    if (!is_he_somewhere (sa->ch))
        return;

    if (GET_POS (sa->ch) < FIGHT)
        return;

    do_move (sa->ch, "", sa->info2);
}

void
sa_command (SECOND_AFFECT * sa)
{

    if (!is_he_somewhere (sa->ch))
        return;

    command_interpreter(sa->ch, sa->info);

}

void
add_second_affect (int type, int seconds, CHAR_DATA * ch, OBJ_DATA * obj,
                   const char *info, int info2)
{
    SECOND_AFFECT *sa;
    CREATE (sa, SECOND_AFFECT, 1);

    if (type)
        sa->type = type;
    else
        sa->type = 0;

    if (seconds)
        sa->seconds = seconds;
    else
        sa->seconds = 0;

    if (ch)
        sa->ch = ch;
    else
        sa->ch = NULL;

    if (obj)
        sa->obj = obj;
    else
        sa->obj = NULL;

    if (info2)
        sa->info2 = info2;
    else
        sa->info2 = 0;

    if (info)
        sa->info = add_hash (info);
    else
        sa->info = NULL;

    sa->next = second_affect_list;
    second_affect_list = sa;
}

/*
SECOND_AFFECT *
get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj)
{
  if (!ch && !obj)
    return NULL;

  if (!type)
    return NULL;

  vector<second_affect*>::iterator it;
  for (it = second_affect_vector.begin(); it != second_affect_vector.end(); it++)
  {
    if ((!ch || (*it)->ch == ch) && (*it)->type == type)
      if (!obj || obj == (*it)->obj)
        if ((*it)->seconds)
  	  return (*it);
  }

  return NULL;
}


void
remove_second_affect (SECOND_AFFECT * sa)
{
  vector<second_affect*>::iterator it;
  for (it = second_affect_vector.begin(); it != second_affect_vector.end(); it++)
  {
    if ((*it) == sa)
    {
      second_affect_vector.erase(it);

      if (sa->info)
        mem_free (sa->info);

      mem_free (sa);

      break;
    }
  }
}


void
second_affect_update (void)
{
  SECOND_AFFECT *sa;
  extern int second_affect_active;

  vector<second_affect*>::iterator it;
  for (it = second_affect_vector.begin(); it != second_affect_vector.end();)
  {
    sa = (*it);

    if (--(sa->seconds) > 0)
    {
      it++;
      continue;
    }

    second_affect_active = 1;

    switch (sa->type)
    {
      case SA_STAND:
        sa_stand (sa);
	break;
      case SA_GET_OBJ:
        sa_get (sa);
        break;
      case SA_WEAR_OBJ:
        sa_wear (sa);
	break;
      case SA_CLOSE_DOOR:
        sa_close_door (sa);
        break;
      case SA_COMBAT_DOOR:
        sa_combat_door (sa);
        break;
      case SA_WORLD_SWAP:
        break;
      case SA_KNOCK_OUT:
	sa_knock_out (sa);
        break;
      case SA_MOVE:
        sa_move (sa);
        break;
      case SA_RESCUE:
        sa_rescue (sa);
        break;
      case SA_COMMAND:
        sa_command (sa);
        break;
    }

    second_affect_active = 0;

    second_affect_vector.erase(it);

    if (sa->info)
      mem_free (sa->info);

    mem_free (sa);
    it = second_affect_vector.begin();
  }
}
*/

SECOND_AFFECT *
get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj)
{
    SECOND_AFFECT *sa = NULL;

    for (sa = second_affect_list; sa; sa = sa->next)
        if ((!ch || sa->ch == ch) && sa->type == type)
            if (!obj || obj == sa->obj)
                return sa;

    return NULL;
}

void
remove_second_affect (SECOND_AFFECT * sa)
{
    SECOND_AFFECT *sa_list;

    if (sa == second_affect_list)
    {

        second_affect_list = sa->next;

        if (sa->info)
            mem_free (sa->info);

        mem_free (sa);
        return;
    }

    for (sa_list = second_affect_list; sa_list; sa_list = sa_list->next)
        if (sa_list->next == sa)
            sa_list->next = sa->next;

    if (sa->info)
        mem_free (sa->info);

    mem_free (sa);
}

void
second_affect_update (void)
{
    SECOND_AFFECT *sa;
    SECOND_AFFECT *sa_t;
    SECOND_AFFECT *next_sa;
    extern int second_affect_active;

    for (sa = second_affect_list; sa; sa = next_sa)
    {

        next_sa = sa->next;

        if (--(sa->seconds) > 0)
            continue;

        if (sa == second_affect_list)
            second_affect_list = sa->next;
        else
        {
            for (sa_t = second_affect_list; sa_t->next; sa_t = sa_t->next)
                if (sa_t->next == sa)
                {
                    sa_t->next = sa->next;
                    break;
                }
        }

        second_affect_active = 1;

        switch (sa->type)
        {
        case SA_STAND:
            sa_stand (sa);
            break;
        case SA_GET_OBJ:
            sa_get (sa);
            break;
        case SA_WEAR_OBJ:
            sa_wear (sa);
            break;
        case SA_CLOSE_DOOR:
            sa_close_door (sa);
            break;
        case SA_COMBAT_DOOR:
            sa_combat_door (sa);
            break;
        case SA_WORLD_SWAP:
            break;
        case SA_KNOCK_OUT:
            sa_knock_out (sa);
            break;
        case SA_MOVE:
            sa_move (sa);
            break;
        case SA_RESCUE:
            sa_rescue (sa);
            break;
        case SA_COMMAND:
            sa_command (sa);
            break;
        case SA_GRENADE:
            sa_grenade (sa);
            break;
		case SA_ROOMSHOTS:
			sa_roomshots (sa);
			break;
		case SA_DISTSHOTS:
			sa_distshots (sa);
			break;
		case SA_ROOMFIRED:
			sa_roomfired (sa);
			break;
		case SA_INVOLCOVER:
			sa_involcover (sa);
			break;
        }

        second_affect_active = 0;

        if (sa->info)
        {
            mem_free (sa->info);
            sa->info = NULL;
        }

        mem_free (sa);
    }
}


void
do_scommand (CHAR_DATA * ch, char * argument, int cmd)
{
    if (!IS_NPC(ch) && !GET_TRUST(ch) && !cmd)
    {
        send_to_char("You are not permitted to use this command.\n\r", ch);
        return;
    }
    std::string ArgumentList = argument, ThisArgument;
    ArgumentList = one_argument(ArgumentList, ThisArgument);
    if (ArgumentList.empty() || ThisArgument.empty())
    {
        send_to_char("Correct syntax: #6scommand delay command#0.\n\r", ch);
        return;
    }
    if (!(is_number(ThisArgument.c_str())))
    {
        send_to_char("Delay must be a number.\n\r", ch);
        return;
    }
    add_second_affect(SA_COMMAND, atoi(ThisArgument.c_str()), ch, NULL, ArgumentList.c_str(), 0);
    //output = "SA Log: " + MAKE_STRING(ch->tname) + " , Command: " + ArgumentList + ".";
    //system_log (output.c_str(), true);
    return;
}

void
prisoner_release (CHAR_DATA * ch, int zone)
{
    CHAR_DATA *tch;
    OBJ_DATA *obj, *bag;
    OBJ_DATA *tobj, *next_obj;
    OBJ_DATA *tobj2, *next_obj2;
    OBJ_DATA *tobj3, *next_obj3;
    ROOM_DATA *room;
    int dir = 0, jail_vnum = 0;
    bool jailed = false;
    char buf[MAX_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    char *date;
    time_t time_now;

    /* Make sure prisoners are in their cell */

    for (dir = 0; dir <= LAST_DIR; dir++)
    {
        if (!ch->room->dir_option[dir])
            continue;
        if (!(room = vnum_to_room (ch->room->dir_option[dir]->to_room)))
            continue;
        for (tch = room->people; tch; tch = tch->next_in_room)
        {
            if (IS_SET (tch->act, ACT_JAILER))
            {
                jailed = true;
                jail_vnum = tch->in_room;
                break;
            }
        }
    }

    if (!jailed)
        return;

    send_to_room ("The jailer flings open the cell door.\n\r", ch->in_room);
    act ("The jailer brutally knocks $n unconscious.", false, ch, 0, 0,
         TO_ROOM);
    act ("The jailer brutally knocks you unconscious.", false, ch, 0, 0,
         TO_CHAR);

    GET_POS (ch) = SLEEP;

    act ("The jailer removes $n and closes the door before you can react.",
         false, ch, 0, 0, TO_ROOM);

    char_from_room (ch);

    sprintf (buf, "A prison bag, labeled \'Belongings for %s\' sits here.",
             ch->short_descr);

    bag = NULL;

    for (obj = object_list; obj; obj = obj->next)
    {
        if (obj->deleted)
            continue;
        if (GET_ITEM_TYPE (obj) != ITEM_CONTAINER)
            continue;
        if (!str_cmp (obj->description, buf))
        {
            bag = load_object (1345);
            for (tobj = obj->contains; tobj; tobj = next_obj)
            {
                next_obj = tobj->next_content;
                if (IS_SET (tobj->obj_flags.extra_flags, ITEM_ILLEGAL))
                    continue;
                for (tobj2 = tobj->contains; tobj2; tobj2 = next_obj2)
                {
                    next_obj2 = tobj2->next_content;
                    if (IS_SET (tobj2->obj_flags.extra_flags, ITEM_ILLEGAL))
                        extract_obj (tobj2);
                    if (GET_ITEM_TYPE (tobj2) == ITEM_WEAPON)
                        extract_obj (tobj2);
                    for (tobj3 = tobj2->contains; tobj3; tobj3 = next_obj3)
                    {
                        next_obj3 = tobj3->next_content;
                        if (IS_SET (tobj3->obj_flags.extra_flags, ITEM_ILLEGAL))
                            extract_obj (tobj3);
                        if (GET_ITEM_TYPE (tobj3) == ITEM_WEAPON)
                            extract_obj (tobj3);
                    }
                }
                obj_from_obj (&tobj, 0);
                obj_to_obj (tobj, bag);
            }
            extract_obj (obj);
            obj_to_char (bag, ch);
        }
    }

    switch (zone)
    {
    case 12:
        char_to_room (ch, VNUM_HULE_RELEASE_ROOM);
        break;
    default:
        char_to_room (ch, jail_vnum);
        break;
    }

    time_now = time (0);
    date = (char *) asctime (localtime (&time_now));
    date[strlen (date) - 1] = '\0';

    if (bag)
        sprintf (msg, "Released from prison in %s with belongings intact.\n",
                 zone_table[ch->room->zone].name);
    else
        sprintf (msg, "Released from prison in %s. Belongings were not found!\n",
                 zone_table[ch->room->zone].name);

    sprintf (buf, "#2Released:#0 %s", ch->tname);

    add_message (1, ch->tname, -2, "Server", date, "Released.", "", msg, 0);
    add_message (1, "Prisoners", -5, "Server", date, buf, "", msg, 0);

}

// We hit this every 5 minutes, to see what needs to be done or doesn't.

void
five_minute_obj_update()
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tobj = NULL;
    CHAR_DATA *tch = NULL;
    SCENT_DATA *scent = NULL;
    SCENT_DATA *next_scent = NULL;
    ROOM_DATA *room = NULL;

    for (room = full_room_list; room; room = room->lnext )
    {
        for (scent = room->scent; scent; scent = next_scent)
        {
            next_scent = scent->next;

            // If our scent is not permanent, then we lost 5 points of atm_power, and 1 point of rat_power
            if (!scent->permanent)
            {
                scent->atm_power -= 5;
                scent->pot_power -= 5;
                scent->rat_power -= 10;

                scent->atm_power = MAX(scent->atm_power, 0);
                scent->pot_power = MAX(scent->pot_power, 0);
                scent->rat_power = MAX(scent->rat_power, 0);
            }

            // If our scent power is less or equal to five, then we lose the scent all together and
            // continue on to the next scent.
            if (scent->atm_power <= 0)
            {
                remove_room_scent(room, scent);
                continue;
            }

            // If our potential power is less than a hundred or we don't have any scent strength, then continue on --
            // the scent isn't powerful or potent enough to be transferred to anything.
            if (scent->pot_power < 100 || !scent_strength(scent))
            {
                continue;
            }

            for (tch = room->people; tch; tch = tch->next_in_room)
            {
                add_scent(tch, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                for (obj = tch->equip; obj; obj = obj->next_content)
                {
                    add_scent(obj, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                }

                if (tch->right_hand)
                    add_scent(tch->right_hand, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                if (tch->left_hand)
                    add_scent(tch->left_hand, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
            }

            for (obj = room->contents; obj; obj = obj->next_content)
            {
                add_scent(obj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                {
                    add_scent(tobj, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                }
            }
        }
    }

    for (obj = object_list; obj; obj = obj->next)
    {
        if (obj->deleted)
            continue;

        if (obj->ocues)
        {
            typedef std::multimap<obj_cue,std::string>::const_iterator N;
            std::pair<N,N> range = obj->ocues->equal_range (ocue_on_five);
            for (N n = range.first; n != range.second; n++)
            {
                std::string cue = n->second;
                if (!cue.empty ())
                {

                    CHAR_DATA *temp_mob;
                    OBJ_DATA *real_obj;
                    OBJ_DATA *test_obj;
                    OBJ_DATA *prog_obj;
                    //temp_obj = obj;

                    int nesting = 0;
                    const char *r = cue.c_str();
                    char *p;
                    char reflex[AVG_STRING_LENGTH] = "";
                    strtol (r+2, &p, 0);
                    strcpy (reflex, p);
                    if (strncmp(r,"h ",2) == 0)
                        nesting = 1;
                    else if (strncmp(r,"r ",2) == 0)
                        nesting = 2;
                    else if (strncmp(r,"p ",2) == 0)
                        nesting = 3;
                    else if (strncmp(r,"n ",2) == 0)
                        nesting = 4;
                    else if (strncmp(r,"a ",2) == 0)
                        nesting = 5;

                    if ((temp_mob = load_mobile (VNUM_DOER)))
                    {

                        std::string test_buf;
                        test_buf.assign("#2");
                        test_buf.append(obj->short_description);
                        test_buf.append("#0");
                        temp_mob->short_descr = str_dup(test_buf.c_str());

                        test_buf.assign("#2");
                        test_buf.append(obj->full_description);
                        test_buf.append("#0");
                        temp_mob->long_descr = str_dup(test_buf.c_str());

                        if (vnum_to_room(obj->in_room) && (nesting == 2 || nesting == 3 || nesting == 5))
                        {
                            char_to_room (temp_mob, obj->in_room);
                            prog_obj = load_object(obj->nVirtual);
                            obj_to_char(prog_obj, temp_mob);
                            command_interpreter (temp_mob, reflex);
                            extract_obj(prog_obj);
                            extract_char (temp_mob);
                        }
                        else if (obj->carried_by && !obj->in_obj && vnum_to_room(obj->carried_by->in_room) && (nesting == 1 || nesting == 3 || nesting == 5))
                        {
                            char_to_room (temp_mob, obj->carried_by->in_room);
                            prog_obj = load_object(obj->nVirtual);
                            obj_to_char(prog_obj, temp_mob);
                            command_interpreter (temp_mob, reflex);
                            extract_obj(prog_obj);
                            extract_char (temp_mob);
                        }
                        else if (obj->in_obj && !obj->carried_by && (nesting == 4 || nesting == 5))
                        {
                            test_obj = obj->in_obj;
                            // Given that you could put a virtually limitless amount of containers, this should find the last nest.
                            while (test_obj)
                            {
                                real_obj = test_obj;
                                test_obj = test_obj->in_obj;
                            }

                            if (vnum_to_room(real_obj->in_room))
                            {
                                char_to_room (temp_mob, real_obj->in_room);
                                prog_obj = load_object(real_obj->nVirtual);
                                obj_to_char(prog_obj, temp_mob);
                                command_interpreter (temp_mob, reflex);
                                extract_obj(prog_obj);
                                extract_char (temp_mob);
                            }
                            else if (real_obj->carried_by && vnum_to_room(real_obj->carried_by->in_room))
                            {
                                char_to_room (temp_mob, real_obj->carried_by->in_room);
                                prog_obj = load_object(real_obj->nVirtual);
                                obj_to_char(prog_obj, temp_mob);
                                command_interpreter (temp_mob, reflex);
                                extract_obj(prog_obj);
                                extract_char (temp_mob);
                            }
                        }
                        else
                        {
                            char_to_room (temp_mob, 666);
                            extract_char (temp_mob);
                        }
                    }
                }
            }
        }

        if (obj->enviro_conds[COND_WATER] > 0)
        {
            obj->enviro_conds[COND_WATER] -= 4;
            obj->enviro_conds[COND_WATER] = MAX(obj->enviro_conds[COND_WATER], 0);
        }

        for (scent = obj->scent; scent; scent = next_scent)
        {
            next_scent = scent->next;

            // If our scent is not permanent, then we lost 5 points of atm_power, and 1 point of rat_power
            if (!scent->permanent)
            {
                scent->atm_power -= 5;
                scent->pot_power -= 5;
                scent->rat_power -= 10;

                scent->atm_power = MAX(scent->atm_power, 0);
                scent->pot_power = MAX(scent->pot_power, 0);
                scent->rat_power = MAX(scent->rat_power, 0);
            }

            // If our scent power is less or equal to five, then we lose the scent all together and
            // continue on to the next scent.
            if (scent->atm_power <= 0)
            {
                remove_obj_scent(obj, scent);
                continue;
            }

            // If our potential power is less than a hundred or we don't have any scent strength, then continue on --
            // the scent isn't powerful or potent enough to be transferred to anything.
            if (scent->pot_power < 100 || !scent_strength(scent))
            {
                continue;
            }

            // If our scent is being worn, then we add our scent to the person wearing us.
            if (obj->equiped_by && obj->equiped_by->in_room)
            {
                add_scent(obj->equiped_by, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
            }

            // If our scent is being held, then we add our scent to the person holding us.
            if (obj->carried_by && obj->carried_by->in_room)
            {
                add_scent(obj->carried_by, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
            }

            // If our scent is in a container, then we add our scent to the container, and everything else
            // in that container.
            if (obj->in_obj)
            {
                add_scent(obj->in_obj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);

                for (tobj = obj->in_obj->contains; tobj; tobj = tobj->next_content)
                {
                    if (tobj != obj)
                        add_scent(tobj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                }
            }

            // If our object contains other objects, then stink them up too.
            if (obj->contains)
            {
                for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                {
                    add_scent(tobj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                }
            }

            // todo: give smells to people sitting at tables.

            if ((room = vnum_to_room(obj->in_room)))
            {
                // If our strength is less than 3 for a cramped room, or 5 for a wide room, then it can't spread to the room.
                if ((IS_SET(room->room_flags, SMALL_ROOM) && scent_strength(scent) < 3) || scent_strength(scent) < 5)
                {
                    continue;
                }

                // Cramped rooms get smellier more quickly.
                if (IS_SET(room->room_flags, SMALL_ROOM))
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                else
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
            }
            else if (obj->equiped_by && (room = obj->equiped_by->room))
            {
                // If our strength is less than 3 for a cramped room, or 5 for a wide room, then it can't spread to the room.
                if ((IS_SET(room->room_flags, SMALL_ROOM) && scent_strength(scent) < 3) || scent_strength(scent) < 5)
                {
                    continue;
                }

                // Cramped rooms get smellier more quickly.
                if (IS_SET(room->room_flags, SMALL_ROOM))
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                else
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
            }
            else if (obj->carried_by && (room = obj->carried_by->room))
            {
                // If our strength is less than 3 for a cramped room, or 5 for a wide room, then it can't spread to the room.
                if ((IS_SET(room->room_flags, SMALL_ROOM) && scent_strength(scent) < 3) || scent_strength(scent) < 5)
                {
                    continue;
                }

                // Cramped rooms get smellier more quickly.
                if (IS_SET(room->room_flags, SMALL_ROOM))
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                else
                    add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
            }
        }
    }
}


void
rl_minute_affect_update (void)
{
    CHAR_DATA *ch = NULL;
    SCENT_DATA *scent = NULL;
    SCENT_DATA *next_scent = NULL;
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tobj = NULL;
    ROOM_DATA *room = NULL;
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *next_af = NULL;
	char buf[MAX_STRING_LENGTH] = {'\0'};
    char buf2[MAX_STRING_LENGTH] = {'\0'};

    next_minute_update += 60;	/* This is a RL minute */

    // Every five minutes, we do our five minute update.
    if (minute_update_count % 5 == 0 || !minute_update_count)
    {
        if (!engine.in_build_mode())
            five_minute_obj_update();
    }

	for (obj = object_list; obj; obj = obj->next)
	{
		if (obj->deleted)
			continue;

		if (obj->ocues)
		{
			typedef std::multimap<obj_cue,std::string>::const_iterator N;
			std::pair<N,N> range = obj->ocues->equal_range (ocue_on_one);
			for (N n = range.first; n != range.second; n++)
			{
				std::string cue = n->second;
				if (!cue.empty ())
				{

					CHAR_DATA *temp_mob;
					OBJ_DATA *real_obj;
					OBJ_DATA *test_obj;
					OBJ_DATA *prog_obj;
					//temp_obj = obj;

					int nesting = 0;
					const char *r = cue.c_str();
					char *p;
					char reflex[AVG_STRING_LENGTH] = "";
					strtol (r+2, &p, 0);
					strcpy (reflex, p);
					if (strncmp(r,"h ",2) == 0)
						nesting = 1;
					else if (strncmp(r,"r ",2) == 0)
						nesting = 2;
					else if (strncmp(r,"p ",2) == 0)
						nesting = 3;
					else if (strncmp(r,"n ",2) == 0)
						nesting = 4;
					else if (strncmp(r,"a ",2) == 0)
						nesting = 5;

					if ((temp_mob = load_mobile (VNUM_DOER)))
					{

						std::string test_buf;
						test_buf.assign("#2");
						test_buf.append(obj->short_description);
						test_buf.append("#0");
						temp_mob->short_descr = str_dup(test_buf.c_str());

						test_buf.assign("#2");
						test_buf.append(obj->full_description);
						test_buf.append("#0");
						temp_mob->long_descr = str_dup(test_buf.c_str());

						if (vnum_to_room(obj->in_room) && (nesting == 2 || nesting == 3 || nesting == 5))
						{
							char_to_room (temp_mob, obj->in_room);
							prog_obj = load_object(obj->nVirtual);
							obj_to_char(prog_obj, temp_mob);
							command_interpreter (temp_mob, reflex);
							extract_obj(prog_obj);
							extract_char (temp_mob);
						}
						else if (obj->carried_by && !obj->in_obj && vnum_to_room(obj->carried_by->in_room) && (nesting == 1 || nesting == 3 || nesting == 5))
						{
							char_to_room (temp_mob, obj->carried_by->in_room);
							prog_obj = load_object(obj->nVirtual);
							obj_to_char(prog_obj, temp_mob);
							command_interpreter (temp_mob, reflex);
							extract_obj(prog_obj);
							extract_char (temp_mob);
						}
						else if (obj->in_obj && !obj->carried_by && (nesting == 4 || nesting == 5))
						{
							test_obj = obj->in_obj;
							// Given that you could put a virtually limitless amount of containers, this should find the last nest.
							while (test_obj)
							{
								real_obj = test_obj;
								test_obj = test_obj->in_obj;
							}

							if (vnum_to_room(real_obj->in_room))
							{
								char_to_room (temp_mob, real_obj->in_room);
								prog_obj = load_object(real_obj->nVirtual);
								obj_to_char(prog_obj, temp_mob);
								command_interpreter (temp_mob, reflex);
								extract_obj(prog_obj);
								extract_char (temp_mob);
							}
							else if (real_obj->carried_by && vnum_to_room(real_obj->carried_by->in_room))
							{
								char_to_room (temp_mob, real_obj->carried_by->in_room);
								prog_obj = load_object(real_obj->nVirtual);
								obj_to_char(prog_obj, temp_mob);
								command_interpreter (temp_mob, reflex);
								extract_obj(prog_obj);
								extract_char (temp_mob);
							}
						}
						else
						{
							char_to_room (temp_mob, 666);
							extract_char (temp_mob);
						}
					}
				}
			}
		}
	}

    for (ch = character_list; ch; ch = ch->next)
    {
        if (ch->deleted)
            continue;

        if (!ch->room)
            continue;

        if (IS_SET (ch->room->room_flags, OOC))
            continue;

        /*
          if (ch->mana < ch->max_mana && !IS_SET (ch->room->room_flags, OOC))
        {
          ch->mana += (ch->aur / 3) + number (2, 6);
        }
        */

        if (!IS_NPC(ch) && ch->combat_block)
        {
            if (ch->combat_block < 0)
            {
                ch->combat_block += 1;

                if (!ch->combat_block)
                    send_to_char("You may now quit the game.\n", ch);
            }
            else
                ch->combat_block -= 1;
        }


        for (af = ch->hour_affects; af; af = next_af)
        {

            next_af = af->next;

            if (af->type == MAGIC_SIT_TABLE)
                continue;

            /*** NOTE:  Make sure these are excluded in hour_affect_update ***/

            if (af->type >= MAGIC_SPELL_GAIN_STOP
                    && af->type <= MAGIC_CRAFT_BRANCH_STOP)
            {
                if (--af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
            {
                if (--af->a.spell.duration <= 0)
                {
                    *buf = '\0';
                    sprintf (buf, "%s",
                             /*(af->a.spell.sn >= 0) ? spell_fade_echo (af->a.spell.sn, af->type) :*/ "");
                    if (!*buf)
                    {
                        sprintf (buf, "'%s' %d",
                                 lookup_spell_variable (af->a.spell.sn,
                                                        VAR_NAME), af->type);
                        sprintf (buf2,
                                 "There seems to be a problem with one of the fade echoes on the %s spell. Please report this to the staff. Thank you.",
                                 buf);
                        // act (buf2, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                        send_to_gods (buf2);
                        system_log (buf2, true);
                    }
                    else
                        act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                    if (af->type == MAGIC_AFFECT_SLEEP)
                    {
                        affect_remove (ch, af);
                        do_wake (ch, "", 0);
                        if (IS_NPC (ch))
                            do_stand (ch, "", 0);
                    }
                    else
                        affect_remove (ch, af);
                }
            }

            if (af->type == MAGIC_PETITION_MESSAGE)
            {
                if (--af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            // If they've got internal bloodlosses, time to strike them with it.
            if (af->type == AFFECT_INTERNAL)
            {
                general_damage(ch, af->a.bleeding.rate);

                // If we're not getting our regular bleeding messages, we need to convey
                // the fact we're receiving internal damage.
                if (!ch->bleeding_prompt && get_affect(ch, AFFECT_INTERNAL) && !number(0, 3))
                {
                    send_to_char("Your insides wrench and heave, spreading agony throughout your body.", ch);
                }
            }

			// Reduce their "time until skill gain again"
            if (af->type >= MAGIC_SKILL_GAIN_STOP &&
                    af->type <= MAGIC_SKILL_GAIN_STOP + LAST_SKILL)
            {
				// But not if they're idle!
			    if (!ch->descr() || ch->descr()->idle)
					continue;

                if (--af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            if (af->type >= AFFECT_TRAUMA_PINNED &&
                    af->type <= AFFECT_TRAUMA_SUSTAINED)
            {
                if (--af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            if (af->type == AFFECT_FORAGED)
            {
                if (--af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            if (af->type >= SOMA_FIRST && af->type <= SOMA_LAST)
            {
                // If the effect is latent, then don't bother doing this.
                if (af->a.soma.latency <= 0)
                    soma_rl_minute_affect (ch, af);
            }
        }

        if ((minute_update_count % 5 == 0 || !minute_update_count) && !engine.in_build_mode())
        {
			// hook: cue_on_five reflex
            if (ch->mob && ch->mob->cues)
            {
				char your_buf[AVG_STRING_LENGTH * 2] = {'\0'};
                typedef std::multimap<mob_cue,std::string>::const_iterator N;
                std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_five);
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

            for (scent = ch->scent; scent; scent = next_scent)
            {
                next_scent = scent->next;

                // If our scent is not permanent, then we lost 5 points of atm_power, and 1 point of rat_power
                if (!scent->permanent)
                {
                    scent->atm_power -= 5;
                    scent->pot_power -= 5;
                    scent->rat_power -= 10;

                    scent->atm_power = MAX(scent->atm_power, 0);
                    scent->pot_power = MAX(scent->pot_power, 0);
                    scent->rat_power = MAX(scent->rat_power, 0);
                }

                // If our scent power is less or equal to five, then we lose the scent all together and continue on
                if (scent->atm_power <= 0)
                {
                    remove_mob_scent(ch, scent);
                    continue;
                }

                // If our potential power is less than a hundred or we don't have any scent strength, then continue on --
                // the scent isn't powerful or potent enough to be transferred to anything.
                if (scent->pot_power < 100 || !scent_strength(scent))
                {
                    continue;
                }

                if (ch->right_hand)
                {
                    add_scent(ch->right_hand, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                }

                if (ch->left_hand)
                {
                    add_scent(ch->left_hand, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                }

                for (obj = ch->equip; obj; obj = obj->next_content)
                {
                    add_scent(obj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);

                    for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                    {
                        add_scent(tobj, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                    }
                }

                if (get_covtable_affect(ch) && (obj = get_covtable_affect(ch)->a.table.obj))
                {
                    add_scent(obj, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);

                    for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                    {
                        add_scent(tobj, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                    }
                }

                if ((room = ch->room))
                {
                    // If our strength is less than 3 for a cramped room, or 5 for a wide room, then it can't spread to the room.
                    if ((IS_SET(room->room_flags, SMALL_ROOM) && scent_strength(scent) < 3) || scent_strength(scent) < 5)
                    {
                        continue;
                    }

                    // Cramped rooms get smellier more quickly.
                    if (IS_SET(room->room_flags, SMALL_ROOM))
                        add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power, scent->pot_power, scent->rat_power, 0);
                    else
                        add_scent(room, scent->scent_ref, scent->permanent, scent->rat_power / 2, scent->pot_power, scent->rat_power, 0);
                }
            }
		}

		// hook: cue_on_one reflex
		if (ch->mob && ch->mob->cues)
		{
			char your_buf[AVG_STRING_LENGTH * 2] = {'\0'};
			typedef std::multimap<mob_cue,std::string>::const_iterator N;
			std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_one);
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
	minute_update_count++;
}

void
ten_second_update (void)
{
    CHAR_DATA *ch = NULL, *ch_next = NULL;
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *next_af = NULL;

    for (ch = character_list; ch; ch = ch_next)
    {

        ch_next = ch->next;

        if (ch->deleted)
            continue;

        if (!ch->room)
            continue;

        if (IS_SET (ch->room->room_flags, OOC))
            continue;

        if ((ch->room->sector_type == SECT_LAKE
                || ch->room->sector_type == SECT_RIVER
                || ch->room->sector_type == SECT_OCEAN
                || ch->room->sector_type == SECT_UNDERWATER) && !number (0, 1))
        {
            if (swimming_check (ch))
                continue;		/* PC drowned. */
        }

        /* This is required here to prevent the ever-so-cheating
           "stop" "hit guy" "stop" "hit guy" bug. Appropriately,
            do_hit no longer clears your delay.
        */

        if (!ch->fighting)
        {
            ch->primary_delay -= 16;
            ch->secondary_delay -= 16;

            if (ch->primary_delay < 0)
                ch->primary_delay = 0;

            if (ch->secondary_delay < 0)
                ch->secondary_delay = 0;
        }

        for (af = ch->hour_affects; af; af = next_af)
        {
            next_af = af->next;


            if (af->type == MAGIC_SIT_TABLE)
                continue;

            /***** Exclusion must be made in hour_affect_update!!! ******/
            /*
            if (af->type >= MAGIC_SMELL_FIRST && af->type <= MAGIC_SMELL_LAST)
            {
                ;
            }
            */

            else if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
            {

                if (IS_NPC (ch))
                    continue;

                if (!af->a.craft->timer)
                    continue;

                af->a.craft->timer -= 10;

                if (af->a.craft->timer < 0)
                    af->a.craft->timer = 0;

                if (!af->a.craft->timer)
                    activate_phase (ch, af);
            }

            else if (af->type >= SOMA_FIRST &&
                     af->type <= SOMA_LAST)
            {
                if (af->a.soma.latency <= 0)
                    soma_ten_second_affect (ch, af);
            }

            else if (af->type >= MAGIC_CRIM_HOODED &&
                     af->type < MAGIC_CRIM_HOODED + 100)
            {

                af->a.spell.duration -= 10;

                /* If ch is fighting, keep the temp flag on him so
                   we can criminalize him after the fight with the
                   guard is over. */

                if (ch->fighting && af->a.spell.duration < 1)
                    af->a.spell.duration = 1;

                if (af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }

            else if (af->type == MAGIC_STARED || af->type == MAGIC_WARNED
                     || af->type == MAGIC_ALERTED )
            {
                af->a.spell.duration -= 10;

                if (af->a.spell.duration <= 0)
                    affect_remove (ch, af);
            }
        }

        /*if(snowstorm && is_human(ch) && !IS_NPC(ch) && !GET_TRUST(ch) && IS_OUTSIDE(ch))
          soma_add_affect(ch, SOMA_SNOWCOLD, 1, 0, 0, 4, 4, 4, 1, 2, 3, 4);
        else if((soma = get_affect(ch, SOMA_SNOWCOLD)) && !IS_OUTSIDE(ch))
        {
          soma->a.soma.duration -= 3;
          soma->a.soma.attack -= 3;
          soma->a.soma.decay -= 3;
          soma->a.soma.sustain -= 3;
          soma->a.soma.release -= 3;
          soma->a.soma.max_power -= 10;
          soma->a.soma.lvl_power -= 10;
          soma->a.soma.atm_power -= 10;
        }*/



        if (ch->deleted)
            continue;

        if (ch->mob && ch->mob->resets)
            activate_resets (ch);
    }
}

void
payday (CHAR_DATA * ch, CHAR_DATA * employer, AFFECTED_TYPE * af)
{
    int t;
    int i;
    OBJ_DATA *tobj;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if (time_info.holiday >= 1)
    {
        send_to_char ("Check again after the feastday has ended.\n", ch);
        return;
    }

    t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

    af->a.job.pay_date = t + af->a.job.days;

    if (af->a.job.cash)
    {
        if (employer)
        {
            if (keeper_has_money(employer, af->a.job.cash))
            {
                keeper_money_to_char(employer, ch, af->a.job.cash);
                subtract_keeper_money (employer, af->a.job.cash);
                sprintf (buf,
                         "INSERT INTO server_logs.payroll "
                         "(time, shopkeep, who, customer, amount, "
                         "room, gametime, port) "
                         "VALUES (NOW(), %d, '%s','%s', %d, %d,'%d-%d-%d %d:00',%d)",
                         employer->mob->vnum, GET_NAME (ch), char_short (ch),
                         af->a.job.cash, ch->in_room, time_info.year,
                         time_info.month + 1, time_info.day + 1, time_info.hour, engine.get_port ());
                mysql_safe_query (buf);

                sprintf (buf, "$N pays you for all your hard work.");
                act (buf, true, ch, 0, employer, TO_CHAR | _ACT_FORMAT);
                act ("$N pays $n some credits.", false, ch, 0, employer, 	       TO_NOTVICT | _ACT_FORMAT);
            } //employer/keeper has the credit
            else
            {
                sprintf (buf, "$N whispers to you that they do not have the funds available. Try again later.");
                act (buf, true, ch, 0, employer, TO_CHAR | _ACT_FORMAT);
                act ("$N whispers somemthing to $n.", false, ch, 0, employer,  TO_NOTVICT | _ACT_FORMAT);
            } //employer Does NOT have the credit
        }//there is an employer
        else
        {
            obj = load_object (1542);
            obj->count = af->a.job.cash;
            obj_to_char (obj, ch);
            sprintf (buf, "You are paid %d credits.\n", af->a.job.cash);
            send_to_char (buf, ch);
        } // there is not an employer
    } //paid in cash

    if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
    {

        for (i = af->a.job.count; i; i--)
            obj_to_char (load_object (af->a.job.object_vnum), ch);

        if (employer)
        {
            sprintf (buf, "$N pays you %d x $p.", af->a.job.count);
            act (buf, false, ch, tobj, employer, TO_CHAR | _ACT_FORMAT);
            act ("$N pays $n with $o.", true, ch, tobj, employer,
                 TO_NOTVICT | _ACT_FORMAT);
        }
        else
        {
            sprintf (buf, "You are paid %d x $p.", af->a.job.count);
            act (buf, false, ch, tobj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n is paid with $o.", true, ch, tobj, 0,
                 TO_ROOM | _ACT_FORMAT);
        }
    }

    sprintf (buf, "\nYour next payday is in %d days.\n", af->a.job.days);
    send_to_char (buf, ch);
}

void
do_payday (CHAR_DATA * ch, char *argument, int cmd)
{
    int i;
    int t;
    bool isEmployed = false;
    CHAR_DATA *employer = NULL;
    OBJ_DATA *tobj = NULL;
    AFFECTED_TYPE *af;
    CHAR_DATA *tch;
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];

    if (time_info.holiday >= 1)
    {
        send_to_char ("Check again after the feastday has ended.\n", ch);
        return;
    }

    t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

    for (i = JOB_1; i <= JOB_3; i++)
    {
        employer = 0;

        if (!(af = get_affect (ch, i)))
            continue;

        isEmployed = true;

        if (af->a.job.employer)
            employer = vnum_to_mob (af->a.job.employer);

        /* PC action to get paid if it is time */

        if (t >= af->a.job.pay_date)
        {
            if (!af->a.job.employer)
            {
                payday (ch, NULL, af);
                continue;
            }
            for (tch = ch->room->people; tch; tch = tch->next_in_room)
            {
                if (IS_NPC (tch) && tch->mob->vnum == af->a.job.employer)
                {
                    payday (ch, tch, af);
                    continue;
                }
            }
        }

        /* Either its not time to be paid, or employer is not around */

        if ((af->a.job.employer) && (employer))
        {
            sprintf (buf, "%s#0 will pay you ", employer->short_descr);
            *buf = toupper (*buf);
            sprintf (buffer, "#5%s", buf);
            sprintf (buf, "%s", buffer);
        }
        else
            strcpy (buf, "You get paid ");

        if (af->a.job.cash)
        {
            sprintf (buf + strlen (buf), "%d credits", af->a.job.cash);
            if (af->a.job.count && vtoo (af->a.job.object_vnum))
                strcat (buf, " and ");
        }

        if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
        {
            if (af->a.job.count == 1)
                strcat (buf, "$p");
            else
                sprintf (buf + strlen (buf), "%d x $p", af->a.job.count);
        }

        sprintf (buf + strlen (buf), ", every %d day%s.  Next #3payday#0 in %d "
                 "day%s.",
                 af->a.job.days, af->a.job.days == 1 ? "" : "s",
                 af->a.job.pay_date - t,
                 af->a.job.pay_date - t == 1 ? "" : "s");

        act (buf, true, ch, tobj, employer, TO_CHAR | _ACT_FORMAT);
    }

    if (!isEmployed)
    {
        send_to_char ("You do not appear to have been setup with a payday.\n",
                      ch);
    }
}

void
hour_affect_update (void)
{
    int t;
    CHAR_DATA *ch;
    AFFECTED_TYPE *af;
    AFFECTED_TYPE *next_af;

    t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

    for (ch = character_list; ch; ch = ch->next)
    {

        if (ch->deleted || !ch->room)
            continue;

        if (ch->room && IS_SET (ch->room->room_flags, OOC))
            continue;

        if (!ch->over_enemies->empty())
            remove_overwatch (ch);

        for (af = ch->hour_affects; af; af = next_af)
        {

            next_af = af->next;

            if (af->type == MAGIC_SIT_TABLE)
                continue;

            if (af->type >= JOB_1 && af->type <= JOB_3)
            {

                if (t < af->a.job.pay_date)
                    continue;

                if (af->a.job.employer) /// \todo Check as dead code
                    continue;

                /* payday (ch, NULL, af); */
                continue;
            }

            if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
                continue;

            //if (af->type >= MAGIC_SMELL_FIRST && af->type <= MAGIC_SMELL_LAST)
            //    continue;

            if (af->type == AFFECT_WATCH_DIR)
                continue;

            if (af->type == AFFECT_GUARD_DIR)
                continue;

            if (af->type >= MAGIC_SKILL_GAIN_STOP &&
                    af->type <= MAGIC_SKILL_GAIN_STOP + LAST_SKILL)
                continue;

            if (af->type >= AFFECT_TRAUMA_PINNED &&
                    af->type <= AFFECT_TRAUMA_SUSTAINED)
                continue;

            if (af->type >= MAGIC_FLAG_NOGAIN &&
                    af->type <= MAGIC_FLAG_NOGAIN + LAST_SKILL)
                continue;

            if (af->type >= MAGIC_FLAG_FOCUS &&
                    af->type <= MAGIC_FLAG_FOCUS + LAST_SKILL)
                continue;

            if (af->type >= MAGIC_FLAG_IGNORE &&
                    af->type <= MAGIC_FLAG_IGNORE + LAST_SKILL)
                continue;

            if (af->type >= MAGIC_CRIM_HOODED &&
                    af->type < MAGIC_CRIM_HOODED + 100)
                continue;

            if (af->type == MAGIC_STARED)
                continue;

            if (af->type == MAGIC_ALERTED)
                continue;

            if (af->type == MAGIC_RAISED_HOOD)
                continue;

            if (af->type == MAGIC_CRAFT_DELAY
					|| af->type == AFFECT_UPGRADE_DELAY
                    || af->type == MAGIC_CRAFT_BRANCH_STOP)
                continue;

            if (af->type == MAGIC_PETITION_MESSAGE)
                continue;

            if (af->type == AFFECT_CHOKING)
            	continue;

            if (af->type == AFFECT_HOLDING_BREATH)
                continue;

            if (af->type == AFFECT_FORAGED)
                continue;

            if (af->type == MAGIC_GUARD ||
                    af->type == AFFECT_SHADOW ||
                    (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST))
                continue;

            if (af->a.spell.duration > 0)
                af->a.spell.duration--;

            if (af->type >= SOMA_FIRST &&
                    af->type <= SOMA_LAST)
            {
                if (af->a.soma.latency > 0)
                    af->a.soma.latency--;
            }

            if (af->a.spell.duration)
                continue;

            else if (af->type >= MAGIC_CRIM_BASE + 1
                     && af->type <= MAGIC_CRIM_BASE + 99)
                prisoner_release (ch, af->type - MAGIC_CRIM_BASE);

            else if (af->type == AFFECT_LOST_CON)
            {
                send_to_char ("You finally feel at your best once more.\n", ch);
                ch->con += af->a.spell.sn;
                ch->tmp_con += af->a.spell.sn;
            }

            else if (af->type == AFFECT_GAIN_POINT)
            {
				switch (af->a.spell.sn)
				{
				case 1:
					ch->dex += 1;
					ch->tmp_dex += 1;
					send_to_char ("At last, you feel more dextrous.\n", ch);
					break;
				case 2:
					ch->con += 1;
					ch->tmp_con += 1;
					send_to_char ("At last, you feel more hardy and constitutional.\n", ch);
					break;
				case 3:
					ch->intel += 1;
					ch->tmp_intel += 1;
					send_to_char ("At last, you feel more alert and intelligence.\n", ch);
					break;
				case 4:
					ch->wil += 1;
					ch->tmp_wil += 1;
					send_to_char ("At last, you feel more willful and focused.\n", ch);
					break;
				case 5:
					ch->aur += 1;
					ch->tmp_aur += 1;
					send_to_char ("At last, you feel more bestial and mutation-e.\n", ch);
					break;
				case 6:
					ch->agi += 1;
					ch->tmp_agi += 1;
					send_to_char ("At last, you feel more agile and speedy.\n", ch);
					break;
				default:
					ch->str += 1;
					ch->tmp_str += 1;
					send_to_char ("At last, you feel more strong and muscled.\n", ch);
					break;
				}
            }

            else if (af->type == AFFECT_LOSE_POINT)
            {
				switch (af->a.spell.sn)
				{
				case 1:
					ch->dex -= 1;
					ch->tmp_dex -= 1;
					send_to_char ("Alas, you feel less dextrous.\n", ch);
					break;
				case 2:
					ch->con -= 1;
					ch->tmp_con -= 1;
					send_to_char ("Alas, you feel less hardy and constitutional.\n", ch);
					break;
				case 3:
					ch->intel -= 1;
					ch->tmp_intel -= 1;
					send_to_char ("Alas, you feel less alert and intelligence.\n", ch);
					break;
				case 4:
					ch->wil -= 1;
					ch->tmp_wil -= 1;
					send_to_char ("Alas, you feel less willful and focused.\n", ch);
					break;
				case 5:
					ch->aur -= 1;
					ch->tmp_aur -= 1;
					send_to_char ("Alas, you feel less bestial and mutation-e.\n", ch);
					break;
				case 6:
					ch->agi -= 1;
					ch->tmp_agi -= 1;
					send_to_char ("Alas, you feel less agile and speedy.\n", ch);
					break;
				default:
					ch->str -= 1;
					ch->tmp_str -= 1;
					send_to_char ("At last, you feel more strong and muscled.\n", ch);
					break;
				}
            }

            /******************************************/
            /*   SPELL WEAR-OFF MESSAGE               */
            /******************************************/

            else if (af->type == MAGIC_AFFECT_TONGUES)
                ch->speaks = af->a.spell.modifier;	/* Revert to former language */

            affect_remove (ch, af);
        }
    }
}

/**************************************
* mode:
* 0 - initial learning of skill
* 1 - know skill, learn additional points to the skill
*
******************************************/
void teach_skill (CHAR_DATA * student, int skill, CHAR_DATA * teacher)
{
    int mode;
    int modifier = 0;
    float multi = 0.0;
    float percentage;
    int multiplier = 1;
    int learn_chance = 0;
    int skill_diff = 100;
    int index;
    int min;
    int max;
    int char_skills = 0;
    char buf[MAX_STRING_LENGTH];
    int roll;

    if (real_skill(student, skill) < 0)
    {
        student->pc->skills[skill] = 0;
        student->skills[skill] = student->pc->skills[skill];
    }

    if (real_skill (student, skill)) // adding to existing skill
    {
        mode = 1;
        skill_diff = real_skill (teacher, skill) - real_skill (student, skill);
        if (skill_diff < 10)
        {
            send_to_char("You are not advanced enough beyond your student to educate them further.\n", teacher);
            return;
        }
    }
    else //learning new skill
    {
        mode = 0;
    }

    //timer for skill check
    if (get_affect (student, MAGIC_SKILL_GAIN_STOP + skill)
            || get_affect (student, MAGIC_FLAG_NOGAIN + skill))
    {
        send_to_char ("Your student seems distracted and unable to pay attention to your teaching.\n",
                      teacher);
        sprintf (buf, "$N tries to teach you something about '%s', but you are still confused.", skills[skill]);
        act (buf, false, student, 0, teacher, TO_CHAR | _ACT_FORMAT);
        return;
    }

    //teachers may learn a little bit from the teaching too.
    skill_use (teacher, skill, 0);

    // Chance to learn it
    //modified by the number of skills already known
    //and by the difficulty of the skill
    for (index = 1; index <= LAST_SKILL; index++)
    {
        if (real_skill (student, index))
            char_skills++;
    }

    if (LAST_SKILL < char_skills)
        char_skills = LAST_SKILL;

    percentage = ((double)char_skills / (double)LAST_SKILL);

    if (percentage >= 0.0 && percentage <= .10) // 12 skills
        multiplier = 4;
    else if (percentage > .10 && percentage <= .15) // 17 skills
        multiplier = 3;
    else if (percentage > .15 && percentage <= .20) // 23 skills
        multiplier = 2;
    else
        multiplier = 1;

    learn_chance = MIN ((int)(calc_lookup (student, REG_LV, skill) * multiplier), 65);

    learn_chance += (GET_INT (teacher) + GET_INT (student))/2;
    learn_chance += (GET_WIL (teacher) + GET_WIL (student))/2;

    roll = number (1, 80);
    if (roll > learn_chance)
    {
        send_to_char("The intricacies of this skill seem to be beyond your pupil at this time.\n",
                     teacher);
        sprintf (buf, "$N tries to teach you something about '%s', but you just don't understand.", skills[skill]);
        act (buf, false, student, 0, teacher, TO_CHAR | _ACT_FORMAT);

        //40 to 180 minutes until they can learn this skill again
        if (!get_affect (student, MAGIC_SKILL_GAIN_STOP + skill)
                && !get_affect (student, MAGIC_FLAG_NOGAIN + skill))
        {
            magic_add_affect (student, MAGIC_SKILL_GAIN_STOP + skill, 150, 0, 0, 0, 0);
        }
        return;
    }

// How much they learn
    //INT/WIL bonus/penalty for teacher
    modifier = GET_INT (teacher) - 14;
    modifier += GET_WIL (teacher) - 14;

    //INT/WIL bonus/penalty for student
    modifier += GET_INT (student) - 14;
    modifier += GET_WIL (student) - 14;

    modifier *= 1;	//modifer adjustment for worth of INT/WIL

    //skill level bonus/penalty for teacher
    modifier += (real_skill (teacher, skill) - 50);

    if (modifier > 80)
        modifier = 80;

    if (modifier < 0)
        modifier = 0;

    //convert to a multiplier (approx range of .25 to 5)
    multi = (double)(100/(100 - modifier));


    if (!skill_max(student, (int) (multi * calc_lookup (student, REG_LV, skill)), 0))
    {
        if (mode == 1) //add to existing skill by 1 point.
        {
            student->pc->skills[skill] ++;
            student->skills[skill] = student->pc->skills[skill];
        }

        else //new skill
        {
            student->pc->skills[skill] = 10;
            student->skills[skill] = student->pc->skills[skill];
        }

        //reduce to CAP value if needed
        if (student->pc->skills[skill] > calc_lookup (student, REG_CAP, skill))
        {
            student->pc->skills[skill] = calc_lookup (student, REG_CAP, skill);
            student->skills[skill] = student->pc->skills[skill];
        }

        //240 to 360 minutes (4 - 6 RL hours)(up to 1 day IG) (until they can learn this skill again
        if (!get_affect (student, MAGIC_SKILL_GAIN_STOP + skill)
                && !get_affect (student, MAGIC_FLAG_NOGAIN + skill))
        {
            min = 240;

            max = 240 + number (1, 60);
            max = MIN (360, max);

            magic_add_affect (student, MAGIC_SKILL_GAIN_STOP + skill,
                              number (min, max), 0, 0, 0, 0);

            send_to_char
            ("Your student seems to have learned something.\n",
             teacher);
            sprintf (buf, "$N teaches you something new about '%s'.", skills[skill]);
            act (buf, false, student, 0, teacher, TO_CHAR | _ACT_FORMAT);
        }
    }
    else
    {
        send_to_char("Your student seems unable to learn anything, as if they have already learnt all they will ever be able to.\n", teacher);
        act ("$N attempts to teach you something, but your mind is too full, as if you have no room to learn anything more.\n", false, student, 0, teacher, TO_CHAR | _ACT_FORMAT);
    }
    return;

}

void
open_skill (CHAR_DATA * ch, int skill)
{
    if (IS_NPC (ch))
        ch->skills[skill] += calc_lookup (ch, REG_OV, skill);
    else
    {
        ch->pc->skills[skill] = calc_lookup (ch, REG_OV, skill);
        ch->skills[skill] += ch->pc->skills[skill];
    }
}

#define MIN_PREREQ	20

int
prereq_skill (CHAR_DATA * ch, CHAR_DATA * victim, int skill,
              int prereq1, int prereq2)
{
    char buf[MAX_STRING_LENGTH];

    if (prereq1 && victim->skills[prereq1] < MIN_PREREQ)
    {

        sprintf (buf, "$N cannot learn '%s' until $S learns '%s' sufficiently.",
                 skills[skill], skills[prereq1]);
        act (buf, true, ch, 0, victim, TO_CHAR);

        sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
                 "'%s' sufficiently.", skills[skill], skills[prereq1]);
        act (buf, true, victim, 0, ch, TO_CHAR);

        return 0;
    }

    else if (prereq2 && victim->skills[prereq2] < MIN_PREREQ)
    {

        sprintf (buf, "$N cannot learn '%s' until $E learns '%s' sufficiently.",
                 skills[skill], skills[prereq2]);
        act (buf, true, ch, 0, victim, TO_CHAR);

        sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
                 "'%s' sufficiently.", skills[skill], skills[prereq2]);
        act (buf, true, victim, 0, ch, TO_CHAR);

        return 0;
    }

    return 1;
}

int
meets_craft_teaching_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
    PHASE_DATA *phase;

    for (phase = craft->phases; phase; phase = phase->next)
    {
        if (phase->skill)
            if (!real_skill (ch, phase->skill)
                    || ch->skills[phase->skill] <
                    (int) ((phase->dice * phase->sides) * .75))
                return 0;
    }

    return 1;
}

int
meets_craft_learning_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
    PHASE_DATA *phase;

    for (phase = craft->phases; phase; phase = phase->next)
    {
        if (phase->skill)
            if (!real_skill (ch, phase->skill)
                    || ch->skills[phase->skill] <
                    (int) ((phase->dice * phase->sides) * .33))
                return 0;
    }

    return 1;
}

void
do_teach (CHAR_DATA * ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i;
    CHAR_DATA *victim;
    SUBCRAFT_HEAD_DATA *craft;
    AFFECTED_TYPE *af;
    int sn = -1;


    if (IS_SET (ch->room->room_flags, OOC))
    {
        send_to_char ("This is not allowed in OOC areas.\n", ch);
        return;
    }

    half_chop (argument, arg1, arg2);

    if (!*arg1)
    {
        send_to_char ("Teach what?\n\r", ch);
        return;
    }

    if (!*arg2)
    {
        send_to_char ("Teach what to who?\n\r", ch);
        return;
    }

    if (!(victim = get_char_room_vis (ch, arg2)))
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        send_to_char ("They are too busy being important to learn anything.\n\r", ch);
        return;
    }

    if ((i = index_lookup (skills, arg1)) == -1)
    {
        for (craft = crafts; craft; craft = craft->next)
        {
            if (ch->descr()->original && !str_cmp (craft->subcraft_name, arg1))
                break;
            if (!str_cmp (craft->subcraft_name, arg1) && has_craft (ch, craft))
                break;
        }

        if (craft)
        {
            if (!ch->descr()->original
                    && !meets_craft_teaching_requirements (ch, craft))
            {
                send_to_char
                ("You aren't proficient enough to teach that craft yet.\n",
                 ch);
                return;
            }
            if (!meets_craft_learning_requirements (victim, craft))
            {
                send_to_char
                ("The intricacies of this craft seem to be beyond your pupil at this time.\n",
                 ch);
                return;
            }
            if (has_craft (victim, craft))
            {
                send_to_char ("They already know that craft.\n", ch);
                return;
            }
            for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
                if (!get_affect (victim, i))
                    break;
            magic_add_affect (victim, i, -1, 0, 0, 0, 0);
            af = get_affect (victim, i);
            af->a.craft =
                (struct affect_craft_type *)
                alloc (sizeof (struct affect_craft_type), 23);
            af->a.craft->subcraft = craft;
            sprintf (buf, "You teach $N '%s %s'.", craft->command,
                     craft->subcraft_name);
            act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            sprintf (buf, "$n has taught you '%s %s'.", craft->command,
                     craft->subcraft_name);
            act (buf, false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
            sprintf (buf, "$n teaches $N something.");
            act (buf, false, ch, 0, victim, TO_NOTVICT | _ACT_FORMAT);
            return;
        }
    }

    if ((i = index_lookup (skills, arg1)) == -1 &&
            (sn = spell_lookup (arg1)) == -1)
    {
        send_to_char ("No such skill, spell or craft.\n\r", ch);
        return;
    }

//Spells

    if (sn != -1)
    {
        send_to_char ("No teaching spells at the moment.\n\r", ch);
        return;
    }

//Regular Skills
    if (ch->skills[i] < 10)
    {
        send_to_char ("You don't know that skill!\n\r", ch);
        return;
    }

    if (ch->skills[i] < 40)
    {
        send_to_char ("You don't yet know that skill well enough.\n\r", ch);
        return;
    }


    if (!trigger (ch, argument, TRIG_TEACH))
        return;

    switch (i)
    {
//Psionics
    case SKILL_DEFLECT:
    case SKILL_DODGE:
        send_to_char ("Inherent combat skills cannot be taught.\n", ch);
        return;
	
	case SKILL_EDUCATION:
		send_to_char ("You cannot teach education.\n", ch);
		return;

    case SKILL_VOODOO:
        send_to_char ("Psionics cannot be taught.\n", ch);
        return;

//Dependant skills
    case SKILL_SNEAK:
        if (!prereq_skill (ch, victim, SKILL_SNEAK, SKILL_HIDE, 0))
        {
            send_to_char ("They don't know enough about hiding.\n\r", ch);
            return;
        }
        break;

    case SKILL_STEAL:
        if (!prereq_skill (ch, victim, SKILL_STEAL, SKILL_SNEAK, SKILL_HIDE))
        {
            send_to_char ("They don't know enough about several things.\n\r", ch);
            return;
        } /// \todo Check if the above condition is always true.
        break;
		
	case SKILL_MEDICINE:
		if (!prereq_skill (ch, victim, SKILL_MEDICINE, SKILL_FIRSTAID, 0))
		{
			send_to_char ("They don't know enough about first aid.\n\r", ch);
			return;
		}
		break;

//Skills that are not supported at this time

//The following skills will be taught without any checks or restrictions
//Normal skills
    }

    teach_skill (victim, i, ch);

    if (IS_MORTAL (victim) || !IS_NPC(victim))
        update_crafts (victim);

    send_to_char ("Done.\n\r", ch);
}

void
add_memory (CHAR_DATA * add, CHAR_DATA * mob)
{
    struct memory_data *memory;
    char name[MAX_STRING_LENGTH];

    if (IS_NPC (add))
        one_argument (GET_NAMES (add), name);
    else
        strcpy (name, GET_NAME (add));

    for (memory = mob->remembers; memory; memory = memory->next)
        if (!strcmp (memory->name, name))
            return;

    CREATE (memory, struct memory_data, 1);

    memory->name = add_hash (name);
    memory->next = mob->remembers;

    mob->remembers = memory;
}

void
forget (CHAR_DATA * ch, CHAR_DATA * foe)
{
    struct memory_data *mem;
    struct memory_data *tmem;

    if (!ch->remembers)
        return;

    if (!strcmp (GET_NAME (foe), ch->remembers->name))
    {
        mem = ch->remembers;
        ch->remembers = ch->remembers->next;
        mem_free (mem);
        return;
    }

    for (mem = ch->remembers; mem->next; mem = mem->next)
    {
        if (!strcmp (GET_NAME (foe), mem->next->name))
        {
            tmem = mem->next;
            mem->next = tmem->next;
            mem_free (tmem);
            return;
        }
    }
}


// Characters scout for a tree to cut down.
// Can specify a certain type of wood, and
// coders can input certain zones to have only
// certain types of trees.

const char *dorthonion_trees[] =
{
    "pine",
    "pine",
    "pine",
    "pine",
    "pine",
    "pine",
    "fir",
    "larch",
    "larch",
    "oak",
    "birch",
    "willow",
    "rowan",
    "\n"
};


void
do_scout (CHAR_DATA * ch, char *argument, int cmd)
{
    /*
    int ind = 0;
    int zone = 0;
    int sector_type;
    int delay = 0;
    int i;
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char temp[MAX_STRING_LENGTH];


    skill_learn(ch, SKILL_UNUSED);

    if (!real_skill (ch, SKILL_UNUSED))
    {
      send_to_char ("You don't have any idea about what sort of tree you should be looking for.\n\r", ch);
      return;
    }

    if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
      && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
      send_to_char ("It's too dark to scout for trees.\n\r", ch);
      return;
    }

    if (IS_SET(ch->room->room_flags, NO_LOG))
    {
    send_to_char("You can't see any trees at all.\n\r", ch);
    return;
    }

    if (is_sunlight_restricted (ch))
    return;

    zone = ch->in_room / 1000;

    argument = one_argument (argument, arg);

    if(!*arg)
    {
    act("You need to specify whether you're scouting for a #6young#0 tree, or #6survey#0ing what types of trees there are in the area.\n", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    return;
    }
    else if(!str_cmp(arg, "survey"))
    {
    send_to_char ("You can see the following trees in this area:\n", ch);
    strcpy (buf, "   ");

    if(zone >= 10 && zone <= 19)
    {

      for (i = 0; *(char *) dorthonion_trees[i] != '\n'; i++)
      {
        if(str_cmp(temp, dorthonion_trees[i]))
            sprintf (buf + strlen (buf), "%s\n   ", (char *) dorthonion_trees[i]);
        sprintf(temp, "%s", dorthonion_trees[i]);
      }
    }
    else
    {
      for (i = 0; *(char *) hardwood_colors[i] != '\n'; i++)
      {
        if(str_cmp(temp, hardwood_colors[i]))
          sprintf (buf + strlen (buf), "%s\n   ", (char *) hardwood_colors[i]);
        sprintf(temp, "%s", hardwood_colors[i]);
      }
    }

    send_to_char(buf, ch);
    return;
    }

    // Remove the ability to scout for tall trees until some other stuff is implemented.

    else if(!str_cmp(arg, "tall"))
    ch->delay_info3 = -2;

    argument = one_argument (argument, arg2);

    if(*arg2)
    {
    // Specific zones have their own types of trees.
    if(zone == 10)
    {
      if((ind = index_lookup (dorthonion_trees, arg2)) != -1)
      {
        ch->delay_info1 = ind + 1;
        delay = 10;
      }
      else
      {
        send_to_char ("There are no such trees in this area.\n", ch);
        return;
      }
    }
    else
    {
      if((ind = index_lookup (hardwood_colors, arg2)) != -1)
      {
        ch->delay_info1 = ind + 1;
        delay = 10;
      }
      else
      {
        send_to_char ("There are no such trees in this area.\n", ch);
        return;
      }
    }
    }

    sector_type = vtor (ch->in_room)->sector_type;

    if (sector_type != SECT_WOODS &&
      sector_type != SECT_FOREST &&
      sector_type != SECT_FIELD &&
      sector_type != SECT_HEATH && sector_type != SECT_HILLS)
    {
      send_to_char ("This place is devoid of trees suitable to logging.\n\r", ch);
      return;
    }
    else if(ch->delay_info3 == -2 &&
      (sector_type == SECT_FIELD ||
      sector_type == SECT_HEATH || sector_type == SECT_HILLS))
    {
    send_to_char ("There aren't tall, mature trees to find here.\n\r", ch);
    return;
    }
    send_to_char ("You begin looking for a tree to cut down.\n\r", ch);
    act ("$n begins examining the flora.", true, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_SCOUT;

    // Takes longer to find a suitable tree in fields, heath or hills.


    switch(sector_type)
    {
    case SECT_WOODS:
    case SECT_FOREST:
      ch->delay = 5;
       break;
    case SECT_FIELD:
    case SECT_HEATH:
      ch->delay = 15;
      break;
    case SECT_HILLS:
      ch->delay = 25;
      break;
    default:
      ch->delay = 1;
      break;
    }

    ch->delay += 20 + delay;
    */
}



void
delayed_scout (CHAR_DATA * ch)
{
    /*
    OBJ_DATA *obj = NULL;
    int zone = 0;

    if (skill_level (ch, SKILL_UNUSED, -35))
      {
        if((zone = ch->in_room / 1000) == 10)
        {
          if(ch->delay_info1 != 0)
          {
            obj = load_colored_object (72 + ch->delay_info3, add_hash (dorthonion_trees[ch->delay_info1 - 1]), 0, 0, 0, 0, 0, 0, 0, 0, 0);
          }
          else
          {
            obj = load_colored_object (72 + ch->delay_info3, add_hash (dorthonion_trees[number(0,12)]), 0, 0, 0, 0, 0, 0, 0, 0, 0);
          }
        }
        else
        {
          if(ch->delay_info1 != 0)
          {
            obj = load_colored_object (72 + ch->delay_info3, add_hash (hardwood_colors[ch->delay_info1 - 1]), 0, 0, 0, 0, 0, 0, 0, 0, 0);
          }
          else
          {
            obj = load_object (72 + ch->delay_info3);
          }
        }

        act ("You find $p, fit for logging.", false, ch, obj, 0, TO_CHAR);
        act ("$n scouts out $p.", true, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
        obj_to_room (obj, ch->in_room);
      }
    else
      {
        send_to_char ("Your efforts to find a tree to log are of no avail.\n\r", ch);
        act ("$n stops searching the flora.", true, ch, 0, 0, TO_ROOM);
      }

    ch->delay_type = 0;
    ch->delay = 0;
    ch->delay_info1 = 0;
    ch->delay_info2 = 0;
    ch->delay_info3 = 0;
    */
}

void
do_logging (CHAR_DATA * ch, char *argument, int cmd)
{
    /*
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tree = NULL;

    if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
        && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
      send_to_char ("It's too dark to cut down trees.\n\r", ch);
      return;
    }

    if (is_sunlight_restricted (ch))
      return;

    skill_learn(ch, SKILL_UNUSED);

    if (!real_skill (ch, SKILL_UNUSED))
    {
      send_to_char ("You don't have any idea how one goes about cutting down trees.\n\r", ch);
      return;
    }


    if (!((obj = get_equip (ch, WEAR_PRIM)) || (obj = get_equip (ch, WEAR_BOTH)))
        || (obj->o.weapon.use_skill != SKILL_LONG_BLADE)
        || get_equip (ch, WEAR_SEC))
    {
      send_to_char("You need to be wielding an axe of some sort for cutting down trees.\n", ch);
      return;
    }

    argument = one_argument (argument, arg);

    if(!*arg)
    {
      send_to_char("What tree do you wish to cut down?\n", ch);
      return;
    }
    else if (!tree && !(tree = get_obj_in_list_vis (ch, arg, ch->room->contents)))
    {
      send_to_char("You don't see that here.\n", ch);
      return;
    }
    else if(tree->obj_flags.type_flag != ITEM_TREE)
    {
      send_to_char("That's not a tree!\n", ch);
      return;
    }

    // Not letting you cut down big trees until there's some point to doing so.

    else if(tree->nVirtual != 70 && tree->nVirtual != 72)
    {
      send_to_char("That's not a tree you can cut down so easily.\n", ch);
    }

    sprintf(buf, "You take up position by #2%s#0 and make the first cut with #2%s#0.\n",
      tree->short_description, obj->short_description);
    send_to_char(buf, ch);

    sprintf(buf, "$n takes up position by %s, making the first cut with %s.",
      tree->short_description, obj->short_description);
    act (buf, true, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_LOG1;
    ch->delay = 20;
    ch->delay_info1 = (long int) obj;
    ch->delay_info2 = (long int) tree;
    */
}

void
delayed_log1 (CHAR_DATA *ch)
{
    /*
    OBJ_DATA *obj;
    OBJ_DATA *tree;

    obj = (OBJ_DATA *) ch->delay_info1;
    tree = (OBJ_DATA *) ch->delay_info2;

    if (!obj)
    	{
        // The axe is gone, abort.
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay = 0;
        ch->delay_type = 0;
        send_to_char ("Without your axe, you find it hard going to chop down the tree.\n", ch);
      return;
          }
    if (!tree)
    	{
        // The tree is gone, abort.
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay = 0;
        ch->delay_type = 0;
        send_to_char ("Without a tree to log, your efforts are in vain.\n", ch);
      return;
          }

    if (CAN_SEE_OBJ (ch, obj))
    {
      send_to_char ("You work on cutting a diagonal section near the base of the tree trunk.\n", ch);
      act ("$n continues to cut into the tree.", false, ch, 0, 0,
         TO_ROOM | _ACT_FORMAT);

      ch->delay_type = DEL_LOG2;
      ch->delay = 20;
    }
    else
    {
      // Can't see the tree anymore
      ch->delay_info1 = 0;
      ch->delay_info2 = 0;
      ch->delay = 0;
      ch->delay_type = 0;
      send_to_char ("You can't see any tree to log.\n", ch);
      return;
    }

    */
}

void
delayed_log2 (CHAR_DATA *ch)
{
    /*
    OBJ_DATA *obj;
    OBJ_DATA *tree;

    obj = (OBJ_DATA *) ch->delay_info1;
    tree = (OBJ_DATA *) ch->delay_info2;

    if (!obj)
    	{
        // The axe is gone, abort.
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay = 0;
        ch->delay_type = 0;
        send_to_char ("Without your axe, you'll find it hard going to chop down the tree.\n", ch);
      return;
          }
    if (!tree)
    	{
        // The axe is gone, abort.
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay = 0;
        ch->delay_type = 0;
        send_to_char ("Without a tree to log, your efforts are in vain.\n", ch);
      return;
          }

    if (CAN_SEE_OBJ (ch, obj))
    {
      send_to_char ("You continue to cut in to the trunk of the tree, widening the diagonal cut.\n", ch);
      act ("$n continues to cut into the tree, the treetop starting to shake.", false, ch, 0, 0,
         TO_ROOM | _ACT_FORMAT);

      ch->delay_type = DEL_LOG3;
      ch->delay = 20;
    }
    else
    {
      // Can't see the tree anymore
      ch->delay_info1 = 0;
      ch->delay_info2 = 0;
      ch->delay = 0;
      ch->delay_type = 0;
      send_to_char ("You can't see any tree to log.\n", ch);
      return;
    }
    */
}

void
delayed_log3 (CHAR_DATA *ch)
{
    /*
    CHAR_DATA *tch = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    int door = 0;
    char *rev_d[] = {
      "the south,",
      "the west,",
      "the north,",
      "the east,",
      "below,",
      "above,"
    };
    OBJ_DATA *obj;
    OBJ_DATA *tree;
    OBJ_DATA *felled;
    int i, j;

    obj = (OBJ_DATA *) ch->delay_info1;
    tree = (OBJ_DATA *) ch->delay_info2;

    if (!obj)
    {
      // The axe is gone, abort.
      ch->delay_info1 = 0;
      ch->delay_info2 = 0;
      ch->delay = 0;
      ch->delay_type = 0;
      send_to_char ("Without your axe, you'll find it hard going to chop down the tree.\n", ch);
      return;
    }
    if (!tree)
    {
      // The tree is gone, abort.
      ch->delay_info1 = 0;
      ch->delay_info2 = 0;
      ch->delay = 0;
      ch->delay_type = 0;
      send_to_char ("Without a tree to log, your efforts are in vain.\n", ch);
      return;
    }

    skill_use(ch, SKILL_UNUSED, 0);

    i = number(1, SKILL_CEILING);
    j = ch->skills[SKILL_UNUSED];

    // Cutting down small trees is a -lot- easier.

    if(tree->nVirtual == 72)
      i -= 40;

    ch->delay_type = 0;
    ch->delay = 0;
    ch->delay_info1 = 0;
    ch->delay_info2 = 0;

    if(i > j + 25)
    {
      act("Despite your best effort, $p remains rooted firmly in the ground. It dawns upon you are going to have to start from scratch if you want that tree cut down.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
      act("Despite $n's best effort, $p remains rooted firmly in the ground.", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);
      return;
    }

    if(tree->nVirtual == 70)
      felled = load_colored_object (71, tree->var_color[0], 0, 0, 0, 0, 0, 0, 0, 0, 0);
    else if(tree->nVirtual == 72)
      felled = load_colored_object (73, tree->var_color[0], 0, 0, 0, 0, 0, 0, 0, 0, 0);
    else
    {
      send_to_char("Despite your best effort, you simply cannot seem to chop this tree down.\n",
        ch);
      act("Despite $n's protracted chopping at $p the tree remains upright. ", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);
      return;
    }

    act("With a tearing and cracking, $p finally begins its slow descent.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
    act("With a tearing and cracking, $p finally begins its slow descent.\n", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);

    for (door = 0; door <= 5; door++)
    {
      if (EXIT (ch, door) && (EXIT (ch, door)->to_room != -1))
      {
        for (tch = vtor (EXIT (ch, door)->to_room)->people; tch; tch = tch->next_in_room)
        {
      if (tch == ch)
          continue;
      sprintf (buf, "From %s you hear a loud, wooden cracking noise followed by a long tearing sound. Soon enough, it ends in a resounding thump", rev_d[door]);
      act (buf, false, tch, 0, ch, TO_ROOM | _ACT_FORMAT);
          break;
        }
      }
    }

    if(i > j + 40)
    {
       act("With a sudden, chilling realization, you watch as $p twists midway through its fall.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
       act("With an urgent splitting sound, $p twists midway through its fall.", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);

      if(number(1,20) >= GET_AGI (ch))
      {
        act("You dive a moment too late as $p crashes to the ground, your legs caught in the whipping branches and your ears flooded by an awful din of smashing wood and rustling leaves.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
        act("As $p falls, $n makes $s dive to safety a moment to late, $s legs caught up in the whipping branches as the tree crashes with an awful thump.", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 3, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 3, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 3, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 3, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 2, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 2, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 2, 0, 0, 0);
        wound_to_char (ch, figure_location (ch, number(0,3)), number (1, 5), 2, 0, 0, 0);
      }
      else
      {
        act("You dive just in time as $p crashes to the ground, your body safe but your ears flooded by an awful din of smashing wood and rustling leaves.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
        act("As $p falls, $n makes $s dive to safety, narrowly escaping the whipping branches as the tree crashes with an awful thump.", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);
      }
    }
    else
    {
      act("Having stepped smartly out of the way, you watch as $p hits the ground with a resounding thump.", false, ch, tree, 0, TO_CHAR | _ACT_FORMAT);
      act("At last, $p hits the ground with a resounding thump.\n", false, ch, tree, 0, TO_ROOM | _ACT_FORMAT);
    }
    obj_to_room (felled, ch->in_room);

    extract_obj (tree);
    */
}


void
do_forage (CHAR_DATA * ch, char *argument, int cmd)
{

    /*
    int sector_type;

    if (!real_skill (ch, SKILL_FORAGE))
      {
        send_to_char ("You don't have any idea how to forage!\n\r", ch);
        return;
      }

    if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
        && !IS_SET (ch->affected_by, AFF_INFRAVIS))
      {
        send_to_char ("It's too dark to forage.\n\r", ch);
        return;
      }

    if (is_sunlight_restricted (ch))
      return;

    sector_type = vtor (ch->in_room)->sector_type;

    if (sector_type != SECT_WOODS &&
        sector_type != SECT_FOREST &&
        sector_type != SECT_FIELD &&
        sector_type != SECT_PASTURE &&
        sector_type != SECT_HEATH && sector_type != SECT_HILLS)
      {
        send_to_char ("This place is devoid of anything edible.\n\r", ch);
        return;
      }

    send_to_char ("You begin rummaging for something edible.\n\r", ch);
    act ("$n begins examining the flora.", true, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_FORAGE;
    ch->delay = 25;
    */
}

#define NUM_FORAGEABLES 17
const int forageables[NUM_FORAGEABLES] =
{
    161,
    97002,
    92072,
    97132,
    97202,
    97238,
    97372,
    97405,
    97554,
    97594,
    97739,
    97742,
    97759,
    97784,
    97853,
    98043,
    98098
};

void
delayed_forage (CHAR_DATA * ch)
{
    OBJ_DATA *obj;
    int seasonal_penalty = 0, temp = 0, base_temp = 75, range = 20;

    temp = weather_info[ch->room->zone].temperature;
    seasonal_penalty =
        (temp > base_temp) ? (temp - base_temp) : (base_temp - temp);
    seasonal_penalty =
        (seasonal_penalty > range) ? (seasonal_penalty - range) : 0;

    if (skill_use (ch, SKILL_FORAGE, seasonal_penalty))
    {

        obj = NULL;
        obj = load_object (forageables[number (0, NUM_FORAGEABLES - 1)]);
        if (!obj)
            obj = load_object (161);
        if (obj)
        {
            act ("You pick $p.", false, ch, obj, 0, TO_CHAR);
            act ("$n finds $p hidden in the flora.", true, ch, obj, 0,
                 TO_ROOM | _ACT_FORMAT);
            obj_to_char (obj, ch);
            if (obj->in_room != NOWHERE)
                act ("$n leaves $p on the the ground.", true, ch, obj, 0,
                     TO_ROOM | _ACT_FORMAT);
        }
        else
        {
            send_to_char ("You deserved something, but the MUD is bare.\n\r",
                          ch);
            return;
        }



    }
    else
    {
        send_to_char ("Your efforts to find food are of no avail.\n\r", ch);
        act ("$n stops searching the flora.", true, ch, 0, 0, TO_ROOM);
    }
}

int
has_a_key (CHAR_DATA * mob)
{
    if (mob->right_hand && GET_ITEM_TYPE (mob->right_hand) == ITEM_KEY)
        return 1;
    if (mob->left_hand && GET_ITEM_TYPE (mob->left_hand) == ITEM_KEY)
        return 1;

    return 0;
}

void
do_knock (CHAR_DATA * ch, char *argument, int cmd)
{
    int door;
    int target_room;
    int key;
    char dir[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char key_name[MAX_STRING_LENGTH];
    CHAR_DATA *tch;
    ROOM_DATA *room;

    argument = one_argument (argument, buf);
    argument = one_argument (argument, dir);

    if ((door = find_door (ch, buf, dir)) == -1)
        return;

    if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
            && !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
    {
        send_to_char ("That's not a door.\n\r", ch);
        return;
    }

    else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
    {
        send_to_char ("It's already open!\n\r", ch);
        return;
    }

    target_room = EXIT (ch, door)->to_room;

    if (!vnum_to_room (target_room))
    {
        send_to_char ("Actually, that door doesn't lead anywhere.\n\r", ch);
        return;
    }

    sprintf (buf, "You hear tapping from the %s.\n\r", dirs[rev_dir[door]]);
    send_to_room (buf, target_room);

    act ("You rap on the door.", false, ch, 0, 0, TO_CHAR);
    act ("$n raps on the door.", false, ch, 0, 0, TO_ROOM);

    /* The trigger command is activated for mobs in the room that ch
       occupies.  So, we have to switch to the target_room to get the
       trigger to happen in that room.
     */

    room = vnum_to_room (target_room);

    //ch->room = room;
    //
    //trigger_info = trigger (ch, argument, TRIG_KNOCK);
    //  ch->room = vtor (ch->in_room);

    /* An unfortunate drag:  If the trigger was activated, that doesn't
       necessarily mean some mob shouldn't automatically open the door.
       But, the way this is implemented, no mob will unlock the door
       if the trigger was activated.
     */
    //if (!trigger_info)
    //  return;

    key = room->dir_option[rev_dir[door]]->key;

    for (tch = room->people; tch; tch = tch->next_in_room)
    {

        if (!IS_NPC (tch) || !AWAKE (tch) || GET_POS (tch) == FIGHT || !CAN_SEE (tch, tch))	/* Too dark if he can't see self. */
            continue;

        if (!has_a_key (tch))
            continue;

        if (tch->descr())
            continue;

        if (!is_brother (ch, tch) &&
                !(IS_SET (ch->act, ACT_ENFORCER) &&
                  IS_SET (tch->act, ACT_ENFORCER)))
            continue;

        if (!IS_SET (room->dir_option[rev_dir[door]]->exit_info, EX_LOCKED)
                || has_key (tch, NULL, key))
        {
            one_argument (room->dir_option[rev_dir[door]]->keyword, key_name);
            sprintf (buf, "unlock %s %s", key_name, dirs[rev_dir[door]]);
            command_interpreter (tch, buf);
            sprintf (buf, "open %s %s", key_name, dirs[rev_dir[door]]);
            command_interpreter (tch, buf);
            sprintf (buf, "%s %s", key_name, dirs[rev_dir[door]]);
            add_second_affect (SA_CLOSE_DOOR, IS_NPC (ch) ? 5 : 10, tch, NULL, buf, 0);
            return;
        }
    }
}

void
do_accuse (CHAR_DATA * ch, char *argument, int cmd)
{
    int hours = -1;
    CHAR_DATA *victim;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH];
    char tmpbuf[MAX_STRING_LENGTH];
    CHAR_DATA *tmp = (CHAR_DATA *) NULL;

    /*   accuse { pc | mob } [hours]                                 */

    if (IS_MORTAL (ch) && !is_area_leader (ch))
    {
        send_to_char ("You cannot accuse anyone here.\n\r", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("The format is: accuse { pc | mob } [hours].\nHours should be -1 for permanently wanted\n", ch);
        return;
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {

        victim = get_char (buf);

        if (!victim)
        {

            /* Let's imms accuse no matter what */
            if (!IS_MORTAL (ch))
            {
                tmp = load_pc (buf);
                if (tmp == (CHAR_DATA *) NULL)
                {
                    send_to_char ("No PC with that name.\n", ch);
                    return;
                }
                victim = tmp;
            }
            else
            {
                send_to_char ("Nobody is here by that name.\n", ch);
                return;
            }
        }

        if (IS_NPC (victim))
        {
            send_to_char ("Who do you wish to accuse?\n", ch);
            return;
        }
    }

    if (tmp == (CHAR_DATA *) NULL)	/* Only f the char is logged in */
        /* unfortunately, is_area_leader() requires the player to
           be logged in.  That means an imm can set an area leader
           as wanted.  WHile not perfect, its not unreasonable all
           in all. */
        if (is_area_leader (victim))
        {
            act ("$N is a leader, you can't accuse $M.",
                 false, ch, 0, victim, TO_CHAR);
            if (tmp != (CHAR_DATA *) NULL)
                unload_pc (tmp);
            return;
        }

    argument = one_argument (argument, buf);

    if (*buf)
    {
        if (!atoi (buf))
        {
            send_to_char ("What number of hours do you intend to use?\n", ch);
            if (tmp != (CHAR_DATA *) NULL)
                unload_pc (tmp);
            return;
        }

        hours = atoi (buf);
    }

    if ((af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)))
    {

        if (hours == -1 && af->a.spell.duration == -1)
        {
            sprintf (tmpbuf, "%s is already permanently wanted.",
                     victim->short_descr);
            send_to_char (tmpbuf, ch);
            if (tmp != (CHAR_DATA *) NULL)
                unload_pc (tmp);
            return;
        }

        if (hours == -1)
        {
            sprintf (tmpbuf, "%s is already permanently wanted.",
                     victim->short_descr);
            send_to_char (tmpbuf, ch);
        }

        af->a.spell.duration = hours;
    }
    else
    {
		if (ch->room->zone == 56 ||
			ch->room->zone == 75 ||
			ch->room->zone == 67)
		{
			char *date;
			time_t time_now;
			time_now = time (0);
			date = (char *) asctime (localtime (&time_now));
			date[strlen (date) - 1] = '\0';
			char msg[MAX_STRING_LENGTH];

			sprintf (msg, "Word was passed around by #5%s#0 that #5%s#0 would be barred from the marketplace, and would remain barred until the Enforcers reviewed #5%s#0 case.\n", ch->short_descr, victim->short_descr, HSHR(victim));

			char *p;
			reformat_string (msg, &p);
			sprintf (msg, "%s", p);
			mem_free (p); // char*

			sprintf (buf, "#3Barred:#0 %s", victim->short_descr);
			add_message (1, "Marketplace-Trouble", -5, "Server", date, buf, "", msg, 0);
			post_straight_to_mysql_board("Marketplace-Trouble", buf, ch->short_descr, msg);
		}


        magic_add_affect (victim, MAGIC_CRIM_BASE + ch->room->zone,
                          hours, 0, 0, 0, 0);
        if (ch->room->zone == 56) //MT
        {
            magic_add_affect (victim, MAGIC_CRIM_BASE + 75,
                              hours, 0, 0, 0, 0);
            magic_add_affect (victim, MAGIC_CRIM_BASE + 67,
                              hours, 0, 0, 0, 0);
        }
        else if (ch->room->zone == 75) //MT
        {
            magic_add_affect (victim, MAGIC_CRIM_BASE + 56,
                              hours, 0, 0, 0, 0);
            magic_add_affect (victim, MAGIC_CRIM_BASE + 67,
                              hours, 0, 0, 0, 0);
        }
        else if (ch->room->zone == 67) //MT
        {
            magic_add_affect (victim, MAGIC_CRIM_BASE + 56,
                              hours, 0, 0, 0, 0);
            magic_add_affect (victim, MAGIC_CRIM_BASE + 75,
                              hours, 0, 0, 0, 0);
        }
    }
    if (tmp != (CHAR_DATA *) NULL)
        unload_pc (tmp);
    send_to_char ("Ok.\n", ch);
}

void
do_pardon (CHAR_DATA * ch, char *argument, int cmd)
{
    DESCRIPTOR_DATA *td;
    CHAR_DATA *victim;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH];
    char tmpbuf[MAX_STRING_LENGTH];
    CHAR_DATA *tmp = (CHAR_DATA *) NULL;

    if (IS_MORTAL (ch) && !is_area_leader (ch))
    {
        send_to_char ("You cannot pardon anyone here.\n\r", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!(victim = get_char_room_vis (ch, buf)))
    {

        victim = get_char_vis (ch, buf);

        if (!victim)
        {
            if (!IS_MORTAL (ch))
            {
                tmp = load_pc (buf);
                if (tmp == (CHAR_DATA *) NULL)
                {
                    send_to_char ("No PC with that name.\n", ch);
                    return;
                }
                victim = tmp;
            }
            else
            {

                send_to_char ("Nobody is here by that name.\n\r", ch);
                return;
            }
        }

        if (IS_NPC (victim))
        {
            send_to_char ("Who?\n\r", ch);
            return;
        }
    }

    if (!(af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)))
    {

        if (IS_SET (victim->act, ACT_PARIAH))
        {
            sprintf (tmpbuf, "%s is a pariah, and cannot be pardoned.",
                     victim->short_descr);
            send_to_char (tmpbuf, ch);

            if (tmp != (CHAR_DATA *) NULL)
                unload_pc (tmp);
            return;
        }

        sprintf (tmpbuf, "%s isn't wanted for anything.", victim->short_descr);

        send_to_char (tmpbuf, ch);

        if (tmp != (CHAR_DATA *) NULL)
            unload_pc (tmp);
        return;
    }

    if (IS_NPC (victim))
    {
        if (af->type >= MAGIC_CRIM_BASE + 1 && af->type <= MAGIC_CRIM_BASE + 99)
            prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
    }
    else
    {
        for (td = descriptor_list; td; td = td->next)
        {
            if (td->connected != CON_PLYNG)
                continue;
            if (!td->character)
                continue;
            if (td->character == victim)
            {
                if (af->type >= MAGIC_CRIM_BASE + 1
                        && af->type <= MAGIC_CRIM_BASE + 99)
                    prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
                break;
            }
        }
    }

	if (af->type == MAGIC_CRIM_BASE + 56 ||
		af->type == MAGIC_CRIM_BASE + 75 ||
		af->type == MAGIC_CRIM_BASE + 67)
	{
		AFFECTED_TYPE *saf = NULL;
		if ((saf = get_affect (victim, MAGIC_CRIM_BASE + 56)))
		{
			affect_remove(victim, saf);
		}
		if ((saf = get_affect (victim, MAGIC_CRIM_BASE + 75)))
		{
			affect_remove(victim, saf);
		}
		if ((saf = get_affect (victim, MAGIC_CRIM_BASE + 67)))
		{
			affect_remove(victim, saf);
		}

		char *date;
		time_t time_now;
		time_now = time (0);
		date = (char *) asctime (localtime (&time_now));
		date[strlen (date) - 1] = '\0';
		char msg[MAX_STRING_LENGTH];

		sprintf (msg, "Word was passed around by #5%s#0 that #5%s#0 has been pardoned of #5%s#0 crimes and could re-enter the Marketplace.\n", ch->short_descr, victim->short_descr, HSHR(victim));

		char *p;
		reformat_string (msg, &p);
		sprintf (msg, "%s", p);
		mem_free (p); // char*

		sprintf (buf, "#6Pardoned:#0 %s", victim->short_descr);
		add_message (1, "Marketplace-Trouble", -5, "Server", date, buf, "", msg, 0);
		post_straight_to_mysql_board("Marketplace-Trouble", buf, ch->short_descr, msg);
	}
	else
	{
		affect_remove (victim, af);
	}

    if (tmp != (CHAR_DATA *) NULL)
        unload_pc (tmp);
    send_to_char ("Ok.\n\r", ch);
}

/* nod_log

room = guard room
seeker_sdescs = (hooded) descs
seeker_name
timestamp
ig_time

store 1 ig month
*/


void
do_nod (CHAR_DATA * ch, char *argument, int cmd)
{
    int opened_a_door = 0;
    int dir;
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char key_name[MAX_STRING_LENGTH] = {'\0'};
    CHAR_DATA *victim;

    argument = one_argument (argument, buf);

    if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (ch))
    {
        if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
                !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
        {
            send_to_char
            ("You decide against making a commotion. PETITION to request to speak.\n",
             ch);
            return;
        }
    }

    if (!*buf)
    {
        act ("You nod.", false, ch, 0, 0, TO_CHAR);
        act ("$n nods.", false, ch, 0, 0, TO_ROOM);
        return;
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {
        send_to_char ("You don't see that person.\n\r", ch);
        return;
    }

    if (IS_NPC (victim) &&
            AWAKE (victim) &&
            !victim->fighting &&
            CAN_SEE (victim, ch) &&
            is_brother (ch, victim) && has_a_key (victim) && !victim->descr())
    {
        if ((get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone)
                || get_affect (ch, MAGIC_CRIM_HOODED + ch->room->zone))
                && is_area_enforcer(victim))
        {
            if (is_hooded(ch))
            {
                do_say (victim, "(sternly) Show yer face.", 0);
            }
            else
            {
                do_alert (victim, "", 0);
                do_say (victim, "(sourly) You ain't gettin' away so easy, you lout.", 0);
            }
            return;
        }
        argument = one_argument (argument, buf);

        if (!*buf)
        {
            for (dir = 0; dir <= LAST_DIR; dir++)
            {

                if (!EXIT (ch, dir))
                    continue;

                if (IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED)
                        && !has_key (victim, NULL, EXIT (ch, dir)->key))
                    continue;

                one_argument (EXIT (victim, dir)->keyword, key_name);
                sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
                command_interpreter (victim, buf);
                sprintf (buf, "open %s %s", key_name, dirs[dir]);
                command_interpreter (victim, buf);
                sprintf (buf, "%s %s", key_name, dirs[dir]);
                add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

                opened_a_door = 1;
            }
        }
        else
        {
            dir = is_direction (buf);
            if (dir == -1 || !EXIT (ch, dir))
            {
                send_to_char ("There is no exit in that direction.\n", ch);
                return;
            }
            if (IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED)
                    && !has_key (victim, NULL, EXIT (ch, dir)->key))
                ;
            else
            {
                one_argument (EXIT (victim, dir)->keyword, key_name);
                sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
                command_interpreter (victim, buf);
                sprintf (buf, "open %s %s", key_name, dirs[dir]);
                command_interpreter (victim, buf);
                sprintf (buf, "%s %s", key_name, dirs[dir]);
                add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

                opened_a_door = 1;
            }
        }

        if (opened_a_door)
            return;
    }

    sprintf (buf, "You nod to #5%s#0.", char_short (victim));
    act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);

    act ("$n nods to you.", true, ch, 0, victim, TO_VICT | _ACT_FORMAT);
    act ("$n nods to $N.", true, ch, 0, victim, TO_NOTVICT | _ACT_FORMAT);
}
void camp_home (CHAR_DATA * ch, char *argument, int cmd)
{
	 int from_room = 0, to_room = 0;
char buf[MAX_STRING_LENGTH] = { '\0' };
CHAR_DATA *tch;
char *date;
time_t current_time;
char arg2[MAX_STRING_LENGTH] = { '\0' };

argument = one_argument (argument, arg2);

	if (!*arg2)
	{
		send_to_char("Please enter a reason for camping. <Usage: camp home : I need to go to sleep> Please make sure to add the colon.\n", ch);
	return;
	}

sprintf (buf, "%s just used camp_home from VNUM:%d \n", GET_NAME (ch),
          ch->room->vnum);
    send_to_gods (buf);


		
			current_time = time (0);
			date = (char *) asctime (localtime (&current_time));
			date[strlen (date) - 1] = '\0';
			sprintf (buf, "From: %s [%d]\n\n", GET_NAME (ch), ch->room->vnum);
			sprintf (buf + strlen (buf), "%s\n", argument);
			add_message (1, "Camps", -5, ch->tname, date, "Logged Home-Camps",
				"", buf, 0);
			mem_free (date); // char*
		




 send_to_char ("You search for a safe location to rest. [Using camp home]\n\r",
                  ch);
    act ("$n begins looking around.", true, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_HOME_CAMP1;
    ch->delay = 30;
}

void
delayed_home_camp1 (CHAR_DATA * ch)
{
    send_to_char
    ("Finding a relatively quiet place, you hunker down. [Home camp]\n", ch);
    act ("$n hunkers down in a relatively quiet place.", false,
         ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_HOME_CAMP2;
    ch->delay = 30;
}

void
delayed_home_camp2 (CHAR_DATA * ch)
{

	if (ch->room->zone == 55 || ch->room->zone == 56 || ch->room->zone == 75 || ch->room->zone == 67)
	{
		send_to_char
		("The sounds of Rust surround you: somewhere in the distance there are\n"
		"shouts, soft and sporadic gunfire, the lazy sound of a man's laughter and\n"
		"the leering call of a tawdry whore, the nasally snivelling of a street punk\n"
		"kid and the metal creak of machinery. \n", ch);
	}
	else
	{
    send_to_char
    ("After double checking the area, you curl up, your eyes eventually closing despite the occasional sounds of the Wilderness. [Home camp]\n", ch);
	}

    act ("$n's eyes close.",
         false, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_HOME_CAMP3;
    ch->delay = 30;
}

void
delayed_home_camp3 (CHAR_DATA * ch)
{
  ROOM_DATA *ch_room = NULL;

	 int from_room = 0, to_room = 0;
    send_to_char
    ("Finally, eventually, you doze off, falling into a light and unsettled sleep. [Home camp]\n", ch);

    act ("Finished, $n slips from your sight.", true, ch, 0, 0, TO_ROOM);

	 from_room = ch->in_room;
    char_from_room (ch);
    char_to_room (ch, BOOZER);
    ch->was_in_room = 0;
    do_quit (ch, "", 1);


}

void
do_camp (CHAR_DATA * ch, char *argument, int cmd)
{
    int sector_type, block = 0;
    struct room_prog *p;
char arg[MAX_STRING_LENGTH] = { '\0' };

    if (is_switched (ch))
        return;
    if (ch->descr() && ch->descr()->original)
    {
        send_to_char ("Not while switched. Use RETURN first.\n", ch);
        return;
    }

    for (p = ch->room->prg; p; p = p->next)
        if (!str_cmp (p->keys, "lean-to leanto")
                && !str_cmp (p->command, "enter"))
            block++;

    if (block)
    {
        send_to_char ("Someone else appears to have already made camp here.\n",
                      ch);
        return;
    }

    if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, SAFE_Q))
    {
        delayed_camp4 (ch);
        return;
    }

    sector_type = ch->room->sector_type;

    ///\TODO Check the necessity of this code. I believe it is a relic
    /// from a former implementation  of camp which added room progs to
    /// the room.
    if (engine.in_build_mode ())
    {
        send_to_char ("Camping is not allowed on the build server.\n", ch);
        return;
    }

    /*
    if (sector_type != SECT_WOODS && sector_type != SECT_FOREST &&
        sector_type != SECT_FIELD && sector_type != SECT_HILLS)
      {
        send_to_char ("You can only camp in the woods, forest, a "
      	    "field or the hills.\n\r", ch);
        return;
      }
    */

    /*
    	send_to_char ("You'll need to pitch a tent or find suitable shelter.\n", ch);
    	return;
    */
argument = one_argument (argument, arg);

if (!str_cmp(arg, "home"))
{
camp_home(ch, argument, cmd);
return;
}

    send_to_char ("You search for a safe location to rest.\n\r",
                  ch);
    act ("$n begins looking around.", true, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_CAMP1;
    ch->delay = 30;
}

void
delayed_camp1 (CHAR_DATA * ch)
{
    send_to_char
    ("Finding a relatively quiet place, you hunker down.\n", ch);
    act ("$n hunkers down in a relatively quiet place.", false,
         ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_CAMP2;
    ch->delay = 30;
}

void
delayed_camp2 (CHAR_DATA * ch)
{

	if (ch->room->zone == 55 || ch->room->zone == 56 || ch->room->zone == 75 || ch->room->zone == 67)
	{
		send_to_char
		("The sounds of Rust surround you: somewhere in the distance there are\n"
		"shouts, soft and sporadic gunfire, the lazy sound of a man's laughter and\n"
		"the leering call of a tawdry whore, the nasally snivelling of a street punk\n"
		"kid and the metal creak of machinery. \n", ch);
	}
	else
	{
    send_to_char
    ("After double checking the area, you curl up, your eyes eventually closing despite the occasional sounds of the Wilderness.\n", ch);
	}

    act ("$n's eyes close.",
         false, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_CAMP3;
    ch->delay = 30;
}

void
delayed_camp3 (CHAR_DATA * ch)
{
    ROOM_DATA *room = ch->room;

    send_to_char
    ("Finally, eventually, you doze off, falling into a light and unsettled sleep.\n", ch);

    act ("Finished, $n slips from your sight.", true, ch, 0, 0, TO_ROOM);

    room->room_flags |= SAFE_Q;
    do_quit (ch, "", 1);
    room->room_flags &= ~SAFE_Q;
    /// \todo Can we camp without modifying the SAFE_Q room flag?
}

void
delayed_camp4 (CHAR_DATA * ch)
{
    do_quit (ch, "", 0);
}

void
knock_out (CHAR_DATA * ch, int seconds)
{
    SECOND_AFFECT *sa;

    if (GET_POS (ch) > SLEEP)
    {
        send_to_char ("You stagger to the ground, losing consciousness!\n\r",
                      ch);
        act ("$n staggers to the ground, losing consciousness.", false, ch, 0,
             0, TO_ROOM);
        GET_POS (ch) = SLEEP;
    }

    if ((sa = get_second_affect (ch, SA_KNOCK_OUT, NULL)))
    {
        if (sa->seconds < seconds)
            sa->seconds = seconds;
        return;
    }

    add_second_affect (SA_KNOCK_OUT, seconds, ch, NULL, NULL, 0);
}

void
do_tables (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj;
    CHAR_DATA *tmp;
    AFFECTED_TYPE *af_table = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int table_count = 0;

    send_to_char ("\n", ch);

    for (obj = ch->room->contents; obj; obj = obj->next_content)
    {
		if (!IS_FURNISH(obj))
            continue;

        table_count++;

        sprintf (buf, "#6%s#0\n", obj_desc (obj));

        if (IS_TABLE(obj))
        {
            for (tmp = ch->room->people; tmp; tmp = tmp->next_in_room)
            {
                af_table = get_covtable_affect (tmp);

                if (!af_table)
                    af_table = get_affect (tmp, AFFECT_COVER);

                if (af_table && is_at_table (tmp, obj) && tmp != ch)
                {
                    sprintf (buf2, "    #5%s#0 is seated here.\n",
                             CAP (char_short (tmp)));
                    strcat (buf, buf2);
                }
            }
        }
        send_to_char (buf, ch);
    }

    if (!table_count)
        send_to_char ("   None.\n", ch);

    return;
}

void
do_corpses (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH] = "";
    int corpse_count = 0;

    send_to_char ("\n", ch);

    for (obj = ch->room->contents; obj; obj = obj->next_content)
    {
        if ((obj->nVirtual != VNUM_CORPSE && obj->nVirtual != VNUM_WHOLEARM && obj->nVirtual != VNUM_WHOLELEG && obj->nVirtual != VNUM_LOWERTORSO &&
                obj->nVirtual != VNUM_UPPERTORSO && obj->nVirtual != VNUM_ZOMBIE_HEAD) || !CAN_SEE_OBJ (ch, obj))
            continue;

        corpse_count++;

        sprintf (buf + strlen(buf), "#2%s#0\n", obj_desc (obj));
    }

    if (!corpse_count)
        send_to_char ("   You don't see any corpses.\n", ch);
    else
        page_string (ch->descr(), buf);

    return;
}

void
do_bullets (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH] = "";
    int bullet_count = 0;

    send_to_char ("\n", ch);

    for (obj = ch->room->contents; obj; obj = obj->next_content)
    {
        if (!CAN_SEE_OBJ (ch, obj) || (GET_ITEM_TYPE(obj) != ITEM_BULLET && GET_ITEM_TYPE(obj) != ITEM_CASE))
            continue;

        bullet_count++;

        sprintf (buf + strlen(buf), "#2%s#0\n", obj_desc (obj));
    }

    if (!bullet_count)
        send_to_char ("   You don't see any bullets or casings.\n", ch);
    else
        page_string (ch->descr(), buf);

    return;
}


// Command Ownership, for transfering ownership of mobs
// Syntax: OWNERSHIP TRANSFER <mob> <character> or OWNERSHIP SET <mob> <character>

void do_ownership (CHAR_DATA *ch, char *argument, int command)
{
    CHAR_DATA *property, *target;
    std::string ArgumentList = argument;
    std::string ThisArgument;
    std::string Output;
    bool transfer = true;

    ArgumentList = one_argument(ArgumentList, ThisArgument);

    if ((ThisArgument.find("?", 0) != std::string::npos) || ThisArgument.empty())
    {
        send_to_char("Syntax is:\n", ch);
        send_to_char("ownership transfer <mob> <character>\n", ch);
        send_to_char("(Staff level)\nownership set <mob> <character> \n", ch);
        return;
    }

    if (ThisArgument.find("set", 0) != std::string::npos)
    {
        transfer = false;

        if (!GET_TRUST(ch))
        {
            send_to_char ("You can only transfer ownership.\n", ch);
            return;
        }
    }

    ArgumentList = one_argument(ArgumentList, ThisArgument);

    if (ThisArgument.empty())
    {
        send_to_char ("Which individual do you wish to transfer ownership of?\n", ch);
        return;
    }

    property = get_char_room_vis (ch, ThisArgument.c_str());

    if (!property)
    {
        if (GET_TRUST(ch))
        {
            property = get_char ((char*)ThisArgument.c_str());
        }

        if (!property)
        {
            send_to_char ("Cannot find mobile with keyword \"#2", ch);
            send_to_char (ThisArgument.c_str(), ch);
            send_to_char ("#0\".\n", ch);
            return;
        }
    }

    if (!IS_NPC(property))
    {
        send_to_char ("You have no authority to deliniate the ownership of a PC.\n", ch);
        return;
    }

    if (IS_NPC(property) && !property->mob->owner  && !GET_TRUST(ch))
    {
        send_to_char ("You have no authority to deliniate the ownership of this individual.\n", ch);
        return;
    }

    if (property->mob->owner && strcmp(property->mob->owner, ch->tname) && !GET_TRUST(ch))
    {
        send_to_char ("You have no authority to deliniate the ownership of this individual.\n", ch);
        return;
    }

    ArgumentList = one_argument(ArgumentList, ThisArgument);


    if (ThisArgument.empty())
    {
        send_to_char ("Transfer the ownership to whom?\n", ch);
        return;
    }

    target = get_char_room_vis(ch, ThisArgument.c_str());

    if (!target)
    {
        if (GET_TRUST(ch))
        {
            target = get_char ((char*)ThisArgument.c_str());
        }

        send_to_char ("You do not see a person with the keyword \"#2", ch);
        send_to_char (ThisArgument.c_str(), ch);
        send_to_char ("#0\" to transfer #5", ch);
        send_to_char (char_short(property), ch);
        send_to_char ("#0 to.\n", ch);
        return;
    }

    if (!transfer && GET_TRUST(ch))
    {

        ThisArgument[0] = toupper(ThisArgument[0]);
        property->mob->owner = str_dup (ThisArgument.c_str());
        send_to_char ("Setting ownership of #5", ch);
        send_to_char (char_short(property), ch);
        send_to_char ("#0 to \"#2", ch);
        send_to_char (ThisArgument.c_str(), ch);
        send_to_char ("#0\".", ch);
        return;

    }

    else
    {

        ArgumentList = one_argument(ArgumentList, ThisArgument);

        if (IS_NPC(target) && (ThisArgument.find('!') == std::string::npos))
        {
            send_to_char ("You are proposing to transfer ownership of #5", ch);
            send_to_char (char_short(property), ch);
            send_to_char ("#0 to #5", ch);
            send_to_char (char_short(target), ch);
            send_to_char ("#0, who is an NPC. Please confirm by typing #6OWNERSHIP TRANSFER <property> <target> !#0\n", ch);
            return;
        }


        Output.assign(target->tname);
        Output[0] = toupper(Output[0]);
        property->mob->owner = str_dup (Output.c_str());
        send_to_char ("You transfer ownership of #5", ch);
        send_to_char (char_short(property), ch);
        send_to_char ("#0 to #5", ch);
        send_to_char (char_short(target), ch);
        send_to_char ("#0.\n", ch);
        Output.assign("#5");
        Output.append(char_short(ch));
        Output.append("#0 transfers ownership of #5");
        Output.append(char_short(property));
        Output.append("#0 to you.\n");
        Output[2] = toupper(Output[2]);
        send_to_char (Output.c_str(), target);
        return;
    }
    return;
}



