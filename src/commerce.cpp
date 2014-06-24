/*------------------------------------------------------------------------\
|  commerce.c : shopkeeper commerce routines          www.middle-earth.us |
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include <sys/stat.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "group.h"
#include "utility.h"
#include "account.h"

#define VNPC_COPPER_PURSE		200
#define MAX_INV_COUNT			60

extern rpie::server engine;

const char *standard_object_colors[] =
{
	"black",
	"white",
	"grey",
	"red",
	"orange",
	"yellow",
	"green",
	"blue",
	"indigo",
	"purple",
	"brown",
	"\n"
};

const char *chemical_names[] =
{
	"hypochlorite",
	"bitartrate",
	"ferricyanide",
	"biosilicate",
	"thiosulfate",
	"salicylate",
	"ferrocyanide",
	"magneside",
	"oxyacetate",
	"quadraphosphate",
	"nitrohydrochloric",
	"trimethylxanthine",
	"celezolxana",
	"theoxanthine",
	"paraphylline",
	"wynstinnite",
	"paraoxynodrene",
	"noxoterraphate",
	"biocyaniside",
	"\n"
};



const char *sizes[] =
{
	"Sizeless",
	"XXS",
	"XS",
	"S",
	"M",
	"L",
	"XL",
	"XXL",
	"\n"
};

const char *sizes_named[] =
{
	"\01",			/* Binary 1 (^A) should be hard to enter for players */
	"XX-Small",
	"X-Small",
	"Small",
	"Medium",
	"Large",
	"X-Large",
	"XX-Large",
	"\n"
};


const char *econ_flags[] =
{
	"earth",			// 1 << 0
	"spaceship",			// 1 << 1
	"futuristic",			// 1 << 2
	"ruins",			// 1 << 3
	"generic",			// 1 << 4
	"junk",			// 1 << 5
	"fine",			// 1 << 6
	"poor",			// 1 << 7
	"raw",			// 1 << 8
	"cooked",			// 1 << 9
	"admin",			// 1 << 10
	"BUG",			// 1 << 11
	"practice",			// 1 << 12
	"used",			// 1 << 13
	"nobarter",                   // 1 << 14
	"alien",                      // 1 << 15
	"mutant",                     // 1 << 16
	"ordinary",                   // 1 << 17
	"good",                       // 1 << 18
	"superb",                     // 1 << 19
	"epic",                       // 1 << 20
	"trash",                      // 1 << 21
	"antique",                    // 1 << 22
	"cheap",                      // 1 << 23
	"PCMake",
	"Grungetown",
	"Carthage",
	"TramCity",
	"Mandira",
	"NoMansTown",
	"\n"
};

// Text of resource types.
// IMPORTANT - must have the same amount of entries in it as does econ flags,
// because the resource system piggy-backs off of it. Got to love these shortcuts!

const char *resources[] =
{
	"coal",
	"iron",
	"copper",
	"gold",
	"silver",
	"salt",
	"wheat",
	"barley",
	"wool",
	"flax",
	"linen",
	"silk",
	"FILLER",
	"FILLER",
	"FILLER",
	"FILLER",
	"FILLER",
	"FILLER",
	"nobarter",
	"\n",
};

const char *trade_bundles[] =
{
	"junk",
	"hides",
	"clothes",
	"food",
	"knickknacks",
	"\n"
};

/*
Adding a new econ zone?  Remember to update zone_to_econ_zone ()
*/

const struct econ_data default_econ_info[] =
{
	/* sold from     sold to -->
	V            minas tirith   osgiliath     gondor     minas morgul   foreign */
	{"minas tirith",
	{{1.00, 0.50}, {1.25, 0.75}, {1.40, 0.90}, {2.25, 1.75}, {1.60, 1.25}}
	},
	{"osgiliath",
	{{1.00, 0.75}, {1.00, 0.50}, {1.20, 0.80}, {2.00, 1.50}, {1.40, 0.95}}
	},
	{"gondor",
	{{1.10, 0.85}, {1.20, 0.95}, {1.00, 0.50}, {2.10, 1.65}, {1.50, 1.05}}
	},
	{"minas morgul",
	{{0.50, 0.25}, {0.75, 0.50}, {0.60, 0.40}, {1.00, 0.50}, {0.70, 0.45}}
	},
	{"foreign",
	{{1.10, 0.85}, {1.20, 0.95}, {1.25, 1.00}, {1.60, 1.25}, {1.00, 0.50}}
	},
	{"\n", {{1.00, 0.50}, {1.00, 0.50}, {1.00, 0.50}, {1.00, 0.50}, {1.00, 0.50}}}
};


// Returns true if namestring contains new material name-tag.

int
	is_tagged (char *name_str)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	sprintf (buf, "%s", name_str);

	for (int i = 0; *materials[i] != '\n'; i++)
	{
		sprintf (buf2, "%s", materials[i]);
		for (size_t j = 0; j <= strlen (buf2); j++)
			buf2[j] = toupper (buf2[j]);
		if (isnamec (buf2, buf))
			return 1;
	}

	return 0;
}

// Function to help ease the pain of revamping the obj database with materials flags.

void
	do_classify (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int updated = 0;
	bool update = false;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Usage: classify <object ##> <material flag>\n", ch);
		return;
	}

	if (!str_cmp (buf, "all"))
	{
		if (str_cmp (ch->tname, "Kithrater"))
		{
			send_to_char ("Get Kithrater to do this.\n", ch);
			return;
		}

		for (int i = 0; *materials[i] != '\n'; i++)
		{

			sprintf (buf2, "%s", materials[i]);
			for (size_t j = 0; j <= strlen (buf2); j++)
				buf2[j] = toupper (buf2[j]);

			for (obj = full_object_list; obj; obj = obj->lnext)
			{

				if (is_tagged (obj->name))
					continue;

				if (!str_cmp (buf2, "TEXTILE") && !isnamec (obj->name, buf2))
				{
					if (isname ("silk", obj->name) || isname ("wool", obj->name)
						|| isname ("woolen", obj->name)
						|| isname ("linen", obj->name)
						|| isname ("cloth", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "LEATHER")
					&& !isnamec (obj->name, buf2))
				{
					if (isname ("leather", obj->name)
						|| isname ("hide", obj->name)
						|| isname ("deerskin", obj->name)
						|| isname ("deerhide", obj->name)
						|| isname ("pelt", obj->name)
						|| isname ("rawhide", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "METAL") && !isnamec (obj->name, buf2))
				{
					if (isname ("sword", obj->name)
						|| isname ("scimitar", obj->name)
						|| isname ("pike", obj->name)
						|| isname ("dagger", obj->name)
						|| isname ("knife", obj->name)
						|| isname ("mace", obj->name)
						|| isname ("halberd", obj->name)
						|| isname ("iron", obj->name)
						|| isname ("steel", obj->name)
						|| isname ("mithril", obj->name)
						|| isname ("ringmail", obj->name)
						|| isname ("metal", obj->name)
						|| isname ("pewter", obj->name)
						|| isname ("scalemail", obj->name)
						|| isname ("bronze", obj->name)
						|| isname ("copper", obj->name)
						|| isname ("brass", obj->name)
						|| isname ("chainmail", obj->name)
						|| isname ("gold", obj->name)
						|| isname ("estoc", obj->name)
						|| isname ("platemail", obj->name)
						|| isname ("polearm", obj->name)
						|| isname ("silver", obj->name)
						|| isname ("lead", obj->name)
						|| isname ("brigandine", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "WOOD") && !isnamec (obj->name, buf2))
				{
					if (isname ("shortbow", obj->name)
						|| isname ("longbow", obj->name)
						|| isname ("crossbow", obj->name)
						|| isname ("oak", obj->name)
						|| isname ("lebethron", obj->name)
						|| isname ("wooden", obj->name)
						|| isname ("wood", obj->name)
						|| isname ("cedar", obj->name)
						|| isname ("ash", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "STONE") && !isnamec (obj->name, buf2))
				{
					if (isname ("rock", obj->name)
						|| isname ("stone", obj->name)
						|| isname ("granite", obj->name)
						|| isname ("onyx", obj->name)
						|| isname ("obsidian", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "GLASS") && !isnamec (obj->name, buf2))
				{
					if (isname ("glass", obj->name)
						|| isname ("bottle", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "PARCHMENT")
					&& !isnamec (obj->name, buf2))
				{
					if (isname ("parchment", obj->name)
						|| isname ("paper", obj->name)
						|| isname ("vellum", obj->name)
						|| isname ("scroll", obj->name)
						|| isname ("book", obj->name)
						|| isname ("scroll", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "LIQUID")
					&& !isnamec (obj->name, buf2))
				{
					if (isname ("fluid", obj->name)
						|| isname ("liquid", obj->name)
						|| isname ("water", obj->name)
						|| isname ("milk", obj->name)
						|| isname ("ale", obj->name)
						|| isname ("beer", obj->name)
						|| isname ("juice", obj->name)
						|| isname ("oil", obj->name)
						|| isname ("tea", obj->name)
						|| isname ("river", obj->name)
						|| isname ("pond", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "VEGETATION")
					&& !isnamec (obj->name, buf2))
				{
					if (isname ("herb", obj->name)
						|| isname ("flower", obj->name)
						|| isname ("plant", obj->name)
						|| isname ("root", obj->name))
						update = true;
				}
				else if (!str_cmp (buf2, "CERAMIC")
					&& !isnamec (obj->name, buf2))
				{
					if (isname ("ceramic", obj->name)
						|| isname ("clay", obj->name))
						update = true;
				}


				if (update)
				{
					updated++;
					sprintf (buf3, "%s %s", obj->name, buf2);
					if (obj->name && strlen (obj->name) > 1)
						mem_free (obj->name);
					obj->name = str_dup (buf3);
					update = false;
				}
			}
		}

		sprintf (buf, "%d items updated.\n", updated);
		send_to_char (buf, ch);
		return;
	}

	if (!str_cmp (buf, "count"))
	{
		for (obj = full_object_list; obj; obj = obj->lnext)
			if (!is_tagged (obj->name))
				updated++;

		sprintf (buf, "%d items remain untagged.\n", updated);
		send_to_char (buf, ch);
		return;
	}

	if (!(obj = vtoo (atoi (buf))))
	{
		send_to_char ("That object VNUM couldn't be found in the database.\n",
			ch);
		return;
	}

	if (is_tagged (obj->name))
	{
		send_to_char ("That object already has a material tag!\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	for (size_t j = 0; j <= strlen (buf); j++)
		buf[j] = toupper (buf[j]);

	*buf2 = '\0';

	if (!is_tagged (buf))
	{
		send_to_char
			("Unrecognized material type. See HELP MATERIALS for details.\n", ch);
		return;
	}
	else
		sprintf (buf2, "%s %s", obj->name, buf);

	if (obj->name && strlen (obj->name) > 1)
		mem_free (obj->name);

	obj->name = str_dup (buf2);

	send_to_char ("Done.\n", ch);
}

// The function called by give() when a player tries to redeem an order ticket.

void
	redeem_order (CHAR_DATA * ch, OBJ_DATA * ticket, CHAR_DATA * keeper)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	char *date_str;
	time_t current_time;

	if (!ch || !ticket || !keeper)
		return;

	mysql_safe_query ("SELECT * FROM special_orders WHERE id = %d",
		ticket->o.od.value[0]);

	result = mysql_store_result (database);
	if (!result)
	{
		send_to_char ("That special order couldn't be found in the database.\n",
			ch);
		return;
	}

	row = mysql_fetch_row (result);

	if (!row || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		send_to_char ("That special order couldn't be found in the database.\n",
			ch);
		return;
	}

	if (keeper->mob->vnum != atoi (row[2]))
	{
		act ("$N doesn't recognize $p - you seem to be in the wrong shop!",
			false, ch, ticket, keeper, TO_CHAR | _ACT_FORMAT);
		mysql_free_result (result);
		return;
	}

	if (time (0) < atoi (row[8]))
	{
		act
			("$N informs you that your order hasn't yet arrived. Please try back at a later time.",
			false, ch, 0, keeper, TO_CHAR | _ACT_FORMAT);
		mysql_free_result (result);
		return;
	}

	if (*row[4] && strlen (row[4]) > 2 && str_cmp (row[4], "(null)"))
		obj = load_colored_object (atoi (row[3]), row[4], 0, 0, 0, 0, 0, 0, 0, 0, 0);
	else
		obj = load_object (atoi (row[3]));

	obj->count = atoi (row[5]);

	if (!obj)
	{
		send_to_char ("That object could not be loaded.\n", ch);
		if (result)
			mysql_free_result (result);
		return;
	}

	mysql_safe_query ("UPDATE special_orders SET redeemed = 1 WHERE id = %d",
		ticket->o.od.value[0]);

	int port = engine.get_port ();
	mysql_safe_query ("INSERT INTO %s.receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'sold','%s','%s',%d,'%s',%d,%d,%d,'%d-%d-%d %d:00',%d)",
		(engine.get_config ("player_log_db")).c_str (),
		keeper->mob->vnum, GET_NAME (ch), char_short (ch),
		obj->nVirtual, obj->short_description, obj->count,
		atoi (row[6]), keeper->in_room, time_info.year,
		time_info.month + 1, time_info.day + 1, time_info.hour,
		port);

	mysql_free_result (result);

	mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = 'Orders' AND post_number = %d",
		ticket->o.od.value[0]);
	result = mysql_store_result (database);

	if (result && mysql_num_rows (result) > 0)
	{
		row = mysql_fetch_row (result);
		current_time = time (0);
		date_str = asctime (localtime (&current_time));
		if (strlen (date_str) > 1)
			date_str[strlen (date_str) - 1] = '\0';

		sprintf (buf, "%s\n#2Note:#0 Order redeemed by %s on %s.\n", row[5],
			ch->tname, date_str);

		mysql_safe_query
			("UPDATE virtual_boards SET message = '%s' WHERE board_name = 'Orders' AND post_number = %d",
			buf, ticket->o.od.value[0]);
	}

	if (result)
		mysql_free_result (result);

	act ("Nodding, $N accepts your ticket, producing $p a few moments later.",
		false, ch, obj, keeper, TO_CHAR | _ACT_FORMAT);
	act
		("Nodding, $N accepts $n's ticket before producing $p from the back room.",
		false, ch, obj, keeper, TO_NOTVICT | _ACT_FORMAT);

	obj_from_char (&ticket, 0);
	extract_obj (ticket);

	obj_to_char (obj, ch);
}

// Unified function to calculate the sale price of a given object based on
// the shopkeeper and any negotiations by a CHAR_DATA, if specified.

// Sell argument is set to T when PC is selling (NPC is buying), and F when
// NPC is selling (PC is buying).

// Results are not rounded for individual list price, but they are rounded
// when an item is purchased or sold.

float
	calculate_sale_price (OBJ_DATA * obj, CHAR_DATA * keeper, CHAR_DATA * ch,
	int quantity, bool round_result, bool sell)
{
	OBJ_DATA *robj;
	ROOM_DATA *store;
	NEGOTIATION_DATA *neg;
	float val_in_farthings = 0;
	float markup = 1.0;
	float markup2 = 1.0;

	if (!keeper || !obj)
		return -1;

	// Calculate costs of item; use econ_markup() for sale to PC, econ_discount()
	// for purchase from NPC or for sale to PC of previously used item.

	// Calculate added cost of any drink in container.

	if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON &&
		obj->contains &&
		GET_ITEM_TYPE (obj->contains) == ITEM_FLUID)
	{
		if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) && !sell)
			markup2 = econ_markup (keeper, obj->contains);
		else
			markup2 = econ_discount (keeper, obj->contains);
		val_in_farthings += obj->contains->silver * obj->contains->count * 4 * markup2;
		val_in_farthings += obj->contains->farthings * obj->contains->count * markup2;
	}

	// Calculate cost of main item.

	if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) && !sell)
		markup = econ_markup (keeper, obj);
	else //value for selling an item
		markup = econ_discount (keeper, obj);

	if (obj->obj_flags.set_cost > 0)
	{
		val_in_farthings = ((float)(obj->obj_flags.set_cost)/100.0) * quantity;
	}
	else if (obj->obj_flags.set_cost < 0)
	{
		val_in_farthings = 0.0f;
	}
	else
	{
		val_in_farthings +=
			(obj->silver * 4.0 + obj->farthings) * markup * quantity;
	}


	// If an object is damaged, then folks are going to be much less likely to want to purchase it.

	int damage_modifier[6] = {100, 75, 50, 25, 5, 0};

	if (!engine.in_test_mode ())
		if (obj->damage)
			val_in_farthings = val_in_farthings * damage_modifier[object__determine_condition(obj)];

	if (ch != NULL)
	{
		if (!sell)
		{
			// PC buying item
			for (neg = keeper->shop->negotiations; neg; neg = neg->next)
			{
				if (neg->ch_coldload_id == ch->coldload_id &&
					neg->obj_vnum == obj->nVirtual)
					break;
			}
		}
		else
		{
			// PC selling item
			for (neg = keeper->shop->negotiations; neg; neg = neg->next)
			{
				if (neg->ch_coldload_id == ch->coldload_id &&
					neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
					break;
			}
		}

		if (neg && neg->price_delta)
			val_in_farthings =
			val_in_farthings * (100.0 + neg->price_delta) / 100.0 ; //fixed formula

	}
	//ch != NULL

	// Resource modification: price is based on current amount/100 of that resource,
	// so the more you buy the more it costs, and the more you sell the less it is worth.
	// Calculated off the shopkeep's stack. Easy if the PC is buying, but a little tricky
	// if the PC is selling.

	if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE)
	{
		double multiplier = 1.00;
		if (!sell)
		{
			multiplier = 100 - obj->count;
			multiplier = multiplier * 2;
			multiplier = multiplier / 100;
			multiplier = 1 + multiplier;
			
			multiplier = MAX(0.25, multiplier);
			multiplier = MIN(2.00, multiplier);

			val_in_farthings = val_in_farthings * multiplier;
		}
		else
		{
			store = vnum_to_room (keeper->shop->store_vnum);

			if (!store->contents)
				return 0;

			for (robj = store->contents; robj; robj = robj->next_content)
			{
				if (robj->nVirtual == obj->nVirtual && robj->count >= 1)
				{
					multiplier = 100 - obj->count;
					multiplier = multiplier * 2;
					multiplier = multiplier / 100;
					multiplier = 1 + multiplier;
			
					multiplier = MAX(0.25, multiplier);
					multiplier = MIN(2.00, multiplier);

					val_in_farthings = val_in_farthings * multiplier;
					break;
				}
			}

			if (!robj)
				return 0;

		}

		if (val_in_farthings < 25)
			val_in_farthings = 25;
	}

	if (round_result)
	{
		if (!sell)
			val_in_farthings = (int) (ceilf (val_in_farthings));
		else
			val_in_farthings = (int) (floorf (val_in_farthings));
		return val_in_farthings;
	}
	else
		return val_in_farthings;

}

// Control function to check whether or not keeper can order a given object.

bool
	can_order (OBJ_DATA * obj)
{
	if (!obj)
		return false;

	// Items in z0 cannot be ordered, since a lot of OOC/staff items are there.
	if (obj->nVirtual < 1000)
		return false;

	// Items in z20 or above cannot be ordered, since a lot of unique goodies are there.
	if (obj->nVirtual > 20000)
		return false;

	// Can't order these things.
	if (GET_ITEM_TYPE (obj) == ITEM_NPC_OBJECT ||
		GET_ITEM_TYPE (obj) == ITEM_ROOM_RENTAL ||
		GET_ITEM_TYPE (obj) == ITEM_RENT_SHOP ||
		GET_ITEM_TYPE (obj) == ITEM_RENT_SHOP ||
		GET_ITEM_TYPE (obj) == ITEM_FLUID ||
		GET_ITEM_TYPE (obj) == ITEM_RENT_TICKET ||
		GET_ITEM_TYPE (obj) == ITEM_KEY)
		return false;

	// Nothing magic.
	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
		return false;

	// Can't order a bunch of things depending on their flags.
	if (IS_SET(obj->econ_flags, 1 << 0) ||  // earth
		IS_SET(obj->econ_flags, 1 << 1) ||  // spaceship
		IS_SET(obj->econ_flags, 1 << 2) ||  // futuristic
		IS_SET(obj->econ_flags, 1 << 5) ||  // junk
		IS_SET(obj->econ_flags, 1 << 10) || // admin
		IS_SET(obj->econ_flags, 1 << 11) || // BUG
		IS_SET(obj->econ_flags, 1 << 14) || // no barter
		IS_SET(obj->econ_flags, 1 << 15) || // alien
		IS_SET(obj->econ_flags, 1 << 16) || // mutant
		IS_SET(obj->econ_flags, 1 << 20))   // alien
		return false;

	// Nothing that is material type other and has no cost.
	if (determine_material(obj) == MAT_OTHER && !obj->silver && !obj->farthings)
		return false;

	// Can't order anything that is extremely heavy.
	if (!IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE) || OBJ_MASS(obj) > 40000)
		return false;

	// Can't order things that have been marked unique.
	if (strstr (obj->name, "unique") || strstr (obj->name, "IMM") || strstr (obj->name, "PC_"))
		return false;

	return true;
}

// Special order routine for NPC shopkeepers.
// Syntax:
// order place <#quantity> <condition> #vnum #money <variables 1 to 10>
// order search <keyword> <#keyword2>
// order fill
// order list
// order preview

void
	do_order (CHAR_DATA * ch, char *argument, int cmd)
{	
	if ( IS_NPC (ch))
	{
		send_to_char ("Only PCs can use the auction system, sorry.\n", ch);
		return;
	}

	CHAR_DATA *keeper = NULL;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	std::string thisArgument = argument, comArg, curArg, oneArg, twoArg, threeArg, fourArg, output = "";

	// We need to find someone who's both a Bulktrader and an Auctioneer
	for (keeper = ch->room->people; keeper; keeper = keeper->next_in_room)
		if (keeper != ch && IS_SET (keeper->act, ACT_BULKTRADER) && IS_SET (keeper->act, ACT_AUCTIONEER) && IS_NPC(keeper))
			break;

	if ( !keeper ) 
	{
		send_to_char ("I don't see any auctioneer here.\n", ch);
		return;
	}

	thisArgument = one_argument(thisArgument, comArg);

	if (comArg == "search")
	{
		thisArgument = one_argument(thisArgument, oneArg);
		thisArgument = one_argument(thisArgument, twoArg);

		if (oneArg.empty())
		{
			send_to_char("You need to specify a keyword to help narrow down the search.\n", ch);
			return;
		}

		// We need to cycle through our full list of objects and
		// display each one that matches the criteria.
		for (obj = full_object_list; obj; obj = obj->lnext)
		{
			if (!isname(oneArg.c_str(), obj->name))
				continue;

			if (!twoArg.empty() && !isname(twoArg.c_str(), obj->name))
				continue;

			if (can_order(obj))
			{
				if (output.empty())
				{
					output.assign("The following items are available to order:\n\n");
					output.append("Number      Item\n");
					output.append("======      ====\n");
				}

				output.append("#6");

				std::ostringstream conversion;

				conversion << obj->nVirtual;
				for (int i = 0, j = (6 - conversion.str().length()); i < j; i++)
				{
					output.append(" ");
				}

				output.append(conversion.str());

				output.append("      ");

				output.append("#2");

				output.append(obj->short_description, MIN((int) strlen(obj->short_description), 70));
				for (int i = 0, j = (71 - MIN((int) strlen(obj->short_description), 70)); i < j; i++)
				{
					output.append(" ");		
				}

				output.append("#0\n");
			}
		}

		if (output.empty())
		{
			send_to_char("No objects were found for those keyword/s! Try some more!\n", ch);
			return;
		}
		else
		{
			page_string (ch->descr(), output.c_str());
		}
	}
	else if (comArg == "retrieve")
	{
		char buf3[MAX_STRING_LENGTH] = {'\0'};
		MYSQL_RES	*result = NULL;
		MYSQL_ROW	row = NULL;
		OBJ_DATA	*tobj;
		FILE		*fp;

		*buf = '\0';
		*buf2 = '\0';

		/* First check for any filled orders for the player */
		std::string world_log_db = engine.get_config ("world_log_db");
		int house_id = keeper->mob->carcass_vnum;

		sprintf (buf,	"SELECT * FROM %s.ah_orders WHERE house_id = %d AND placed_by = '%s' AND status = 1 ORDER BY expires_at ASC", world_log_db.c_str(), house_id, GET_NAME(ch));
		mysql_safe_query (buf);

		result = mysql_store_result (database);

		if (result && (row = mysql_fetch_row(result)) )
		{			
			sprintf (buf, "%s/%s", ORDER_DIR, row[0]);
			fp = fopen (buf, "r");

			if ( !fp )
			{
				send_to_char ("There was an error loading your item from storage. Please contact staff.\n", ch);
				return;	
			}

			tobj = fread_obj(fp);
			fclose(fp);

			name_to_ident (ch, buf2);
			sprintf (buf,	"%s Yes, yes, I have something for you - go on and take #2%s#0.", buf2, obj_short_desc(tobj)); 		
			do_whisper (keeper, buf, 83);			
			obj_to_char (tobj, ch);

			sprintf (buf,
				"UPDATE %s.ah_orders SET status = 2 WHERE auction_id = %d",
				world_log_db.c_str (), atoi(row[0]));
			mysql_safe_query (buf);

			if ( result )
				mysql_free_result (result);
			return;
		}

		if ( result )
			mysql_free_result (result);

		sprintf (buf,	"SELECT * FROM %s.ah_orders WHERE house_id = %d AND placed_by = '%s' AND status = 0 AND expires_at <= UNIX_TIMESTAMP() ORDER BY expires_at ASC", world_log_db.c_str(), house_id, GET_NAME(ch));
		mysql_safe_query (buf);

		result = mysql_store_result (database);

		if (result && (row = mysql_fetch_row(result)) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf,	"%s You've got an order for #2%s#0 that no one filled - I'm releasing the #6%s#0 of chips you gave me for the price of the good.", buf2, row[10], row[5]); 		
			do_whisper (keeper, buf, 83);

			keeper_money_to_char(keeper, ch, atoi(row[5]));

			OBJ_DATA	*board = NULL;
			// Write up this sale so others can learn how to make a good order.
			if ((board = vtoo(atoi(row[3]))))
			{
				char *p;

				sprintf(buf, "#3Fail#0: #2%s#0 for #6%s#0 credits.", row[10], row[5]);

				sprintf(buf3, "At this time, #2%s#0 #3failed to be bought#0 for #6%d#0 credits.\n\n"
					"The item was ordered for sale by #5%s#0.",
					row[10], atoi(row[5]), row[7]);                                      

				reformat_string (buf3, &p);

				post_straight_to_mysql_board(fname((board)->name), buf, (char*) char_short(ch), p);

				mem_free(p);
			}

			sprintf (buf,
				"UPDATE %s.ah_orders SET status = 3 WHERE auction_id = %d",
				world_log_db.c_str (), atoi(row[0]));
			mysql_safe_query (buf);

			if ( result )
				mysql_free_result (result);
			return;
		}

		if ( result )
			mysql_free_result (result);

		name_to_ident (ch, buf2);
		sprintf (buf,	"%s I have neither any filled orders, nor lapsed orders, for you here.", buf2); 		
		do_whisper (keeper, buf, 83);
		
	}
	else if (comArg == "place")
	{
		thisArgument = one_argument(thisArgument, curArg);

		int vnum = 0;
		int price = 0;
		int quantity = 1;
		int condition = 1;
		bool variable = false;
		bool specific_variable = false;

		// If our first argument is less than 999 but still a number, we'll assume
		// we've gone for a quantity.
		if (atoi(curArg.c_str()) < 999 && atoi(curArg.c_str()) != 0)
		{
			quantity = atoi(curArg.c_str());

			if (quantity < 0 || quantity > 50)
			{
				name_to_ident (ch, buf2);			
				sprintf(buf, "%s You can only order between one and fifty items at a time.", buf2);
				do_whisper (keeper, buf, 83);
				return;
			}

			thisArgument = one_argument(thisArgument, curArg);
		}

		// Now we get our quality.
		if (curArg == "pristine")
		{
			condition = 0;
			thisArgument = one_argument(thisArgument, curArg);
		}
		else if (curArg == "good")
		{
			condition = 1;
			thisArgument = one_argument(thisArgument, curArg);
		}
		else if (curArg == "fair")
		{
			condition = 2;
			thisArgument = one_argument(thisArgument, curArg);
		}
		else if (curArg == "poor")
		{
			condition = 3;
			thisArgument = one_argument(thisArgument, curArg);
		}
		else if (curArg == "bad")
		{
			condition = 4;
			thisArgument = one_argument(thisArgument, curArg);
		}
		else if (curArg == "any")
		{
			condition = 5;
			thisArgument = one_argument(thisArgument, curArg);
		}

		// Now we find out what vnum we're trying to target, and get an obj from that.
		vnum = atoi(curArg.c_str());

		if (curArg.empty() || !vnum || vnum > 20000 || vnum < 1000)
		{
			send_to_char("You must specify an item-number: use #6order search#0 to find a number to order.\n", ch);
			return;
		}

		if (!vtoo(vnum) || !can_order(vtoo(vnum)))
		{
			send_to_char("There is no such thing to order: use #6order search#0 to find a number to order.\n", ch);
			return;
		}

		thisArgument = one_argument(thisArgument, curArg);

		price = atoi(curArg.c_str());

		// How mcuh do we want to pay for this?
		if (price <= 0)
		{
			name_to_ident (ch, buf2);			
			sprintf(buf, "%s You need to nominate a price in chips you are willing to pay for this order.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		char *xcolor[10];
		for (int ind = 0; ind < 10; ind++)
		{
			xcolor[ind] = '\0';
		}

		char *zcolor[10];
		for (int ind = 0; ind < 10; ind++)
		{
			zcolor[ind] = '\0';
		}

		if (IS_SET(vtoo(vnum)->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			variable = true;

			for (int xind = 0; xind <= 9; xind ++)
			{
				thisArgument = one_argument(thisArgument, curArg);

				if (vd_variable((char*) curArg.c_str()) != 0)
				{
					xcolor[xind] = str_dup(curArg.c_str());
					specific_variable = true;
				}
				else
				{	
					xcolor[xind] = str_dup("any variable");
				}				
			}

			int k = 0;
			int x = 0;

			char original[MAX_STRING_LENGTH];
			char temp[MAX_STRING_LENGTH];
			char *point;
			sprintf (original, "%s", vtoo(vnum)->full_description);
			point = strpbrk (original, "$");

			// If we found point...
			if (point)
			{
				// Then for every character in the string...
				// We run through the original, adding each bit of y to buf2.
				// However, if we find a $, we see if that's a category of variables.
				// If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

				for (size_t y = 0; y <= strlen (original); y++)
				{
					if (original[y] == *point)
					{
						k = y + 1;
						sprintf (temp, "$");
						// ... and jump ahead a point (to get to the letter after the $
						k = y + 1;

						// Now, until we hit something that's not a alpha-numeric character.
						while (isalpha (original[k]))
						{
							// add the word after the $ to our temporary marker.
							sprintf (temp + strlen (temp), "%c", original[k]);
							k++;
						}
						// Now, we set our end point as where our category ends plus 1.
						k = y + 1;

						// We advance until we get to the new non-alpha-numeric character.
						while (isalpha (original[k]))
							k++;

						//while (!isalpha (original[k]))
						//k++;

						// And then set the point of our main loop to that point
						y = k;
						zcolor[x] = add_hash(temp);
						x++;
					}
				}
			}
			else
			{
				variable = false;
			}

			for (int ind = 0; ind < 10; ind++)
			{
				if (str_cmp(xcolor[ind], "any variable") && !vc_exists(xcolor[ind], zcolor[ind]))
				{
					if (!((str_cmp(zcolor[ind], "$color") || str_cmp(zcolor[ind], "$drabcolor") || str_cmp(zcolor[ind], "$finecolor") ||
						 str_cmp(zcolor[ind], "$plasticcolor") || str_cmp(zcolor[ind], "$lizcolor") || str_cmp(zcolor[ind], "$leathercolor") ||
						 str_cmp(zcolor[ind], "$camocolor")) && 
						(vc_exists(xcolor[ind], "$color") || vc_exists(xcolor[ind], "$finecolor") || vc_exists(xcolor[ind], "$drabcolor") ||
						 vc_exists(xcolor[ind], "$camocolor") || vc_exists(xcolor[ind], "$plasticcolor")  || vc_exists(xcolor[ind], "$lizcolor") || 
						 vc_exists(xcolor[ind], "$leathercolor"))))
					{
						std::string temp;
						temp = zcolor[ind];
						temp.erase(0,1);
						sprintf(buf, "There isn't a #2%s#0 in the #2%s#0 variable category. Use the #6tags %s#0 command to see acceptable variables.\n", xcolor[ind], temp.c_str(), temp.c_str());
						send_to_char(buf, ch);

						for (int ind = 0; ind <= 9; ind++)
						{
							mem_free(xcolor[ind]);
							mem_free(zcolor[ind]);
						}

						return;
					}
				}
			}
		}

		obj = vtoo(vnum);

		// Fee is 1 for the first ten coins you want, and then an additional
		// 1 for every hundred coins of price you're asking for.
		int fee = 0;

		if (price < 10)
		{
			fee = 0;
		}
		else
		{
			fee = 1;
			fee += price / 50;
		}

		if (cmd == 2)
		{
			if (!can_subtract_money (ch, fee + price, keeper->mob->currency_type))
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s You don't seem to have enough credits to place for that order.", buf2);
				do_whisper (keeper, buf, 83);
				return;
			}

			name_to_ident (ch, buf2);
			sprintf(buf, "%s I will now pay #6%d#0 chips to the first person who can fill your order. In addition, I have taken #6%d#0 chips for my services. I will let you know when your order has been fulfilled - you will then be able to #6ORDER RETRIEVE#0 to receive your item. If no one fills your order in One Lunar Day, I will remove your order, allow you to retrieve your money, but keep my fee.\n", buf2, price, fee);
			do_whisper (keeper, buf, 83);
			subtract_money(ch, fee + price, keeper->mob->currency_type);

			// Put in our order stuff now.
			std::string world_log_db = engine.get_config ("world_log_db");
			int house_id = keeper->mob->carcass_vnum;
			int expires_at = time(0) + (60 * 60 * 6 * 28);	// Each IC day is 6 RL hours.

			sprintf (buf,
				"INSERT INTO %s.ah_orders "
				"(house_id, auctioneer_sdesc, auctioneer_board, expires_at, offered_price,"
				" placed_by, placed_by_sdesc, placed_by_account,"
				" wanted_vnum, wanted_sdesc, wanted_condition, wanted_quantity, wanted_vars, status,"
				" var1, var2, var3, var4, var5, var6, var7, var8, var9, var10)"
				" VALUES "
				"(%d, '%s', %d, %d, %d,"
				" '%s', '%s', '%s',"
				" %d, '%s', %d, %d, %d, %d,"
				" '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
				world_log_db.c_str (), 
				house_id, char_short(keeper), keeper->circle, expires_at, price,
				GET_NAME(ch), char_short(ch), ch->pc->account_name, 
				vnum, obj_short_desc(obj), condition, quantity, specific_variable, 0,
				xcolor[0], xcolor[1], xcolor[2], xcolor[3], xcolor[4], xcolor[5], xcolor[6], xcolor[7], xcolor[8], xcolor[9]);

			mysql_safe_query(buf);
			obj = NULL;
		}
		else
		{
			sprintf(buf,               "For #6%d#0 chips, you will place an order for #6%d#0 of the following:\n", price, quantity);
			sprintf(buf + strlen(buf), "   #2%s#0 in #3%s#0 condition.\n", obj_short_desc(obj), (condition == 5 ? "any" : condition == 4 ? "at least bad" : condition == 3 ? "at least poor" : condition == 2 ? "at least fair" : condition == 1 ? "at least good" : "pristine"));
			sprintf(buf + strlen(buf), "\nThe long description of this object will be: #2%s#0\n", obj->description);
			sprintf(buf + strlen(buf), "\nThe full description of this object will be:");
			sprintf(buf + strlen(buf), "\n#2%s#0\n", obj->full_description);

			send_to_char(buf, ch);

			if (variable)
			{
				sprintf(buf, "To fulfill your order, the item will need the following variables:\n");
				for (int ind = 0; ind <= 9; ind++)
				{
					if (zcolor[ind])
					{
						sprintf(buf + strlen(buf), "#3%12s#0 will need to be: #6%s#0\n", zcolor[ind], xcolor[ind]);
					}
				}

				strcat(buf, "\n");
				send_to_char(buf, ch);
			}

			name_to_ident (ch, buf2);
			sprintf(buf, "%s In addition to the #6%d#0 chips you will need to place here as surety for the order, it will cost for #6%d#0 chips to make it worth my time. To pay the total of #6%d#0 chips, and place your order, type #6ACCEPT#0.\n", buf2, price, fee, price + fee);
			do_whisper (keeper, buf, 83);

			for (int ind = 0; ind <= 9; ind++)
			{
				mem_free(xcolor[ind]);
				mem_free(zcolor[ind]);
			}

			obj = NULL;

			ch->delay_type = DEL_ORDER_PLACE;
			ch->delay_ch = keeper;
			ch->delay_who = add_hash(argument);
		}
		return;
	}
	else if (comArg == "list")
	{
		MYSQL_RES	*result = NULL;
		MYSQL_ROW	row = NULL;
		char buf2 [MAX_STRING_LENGTH];
		char buf3 [MAX_STRING_LENGTH];
		char buf4 [MAX_STRING_LENGTH];
		char statbuf[20];
		int  time_remaining = 0;
		int days = 0;
		int hours = 0;
		int minutes = 0;
		int house_id = 0;

		// Woot! Let's hear it for sloppy, lazy-ass hacks!
		house_id = keeper->mob->carcass_vnum;

		thisArgument = one_argument(thisArgument, oneArg);

		// Auction status.
		std::string world_log_db = engine.get_config ("world_log_db");
		// List auctions by keyword.
		if (!oneArg.empty())
		{
			sprintf (buf,
				"SELECT * FROM %s.ah_orders"
				" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
				" AND wanted_short LIKE \"%%%%%s%%%%\""
				" AND house_id = %d "
				" AND status < 2 "
				" ORDER BY expires_at ASC", 
				world_log_db.c_str (), oneArg.c_str(), house_id);
		}
		// List all auctions.
		else
		{
			sprintf (buf,	
				"SELECT * FROM %s.ah_orders"
				" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
				" AND house_id = %d"
				" AND status < 2 "
				" ORDER BY expires_at ASC", 
				world_log_db.c_str (), house_id);
		}

		mysql_safe_query (buf);

		result = mysql_store_result (database);

		if ( !result )
			return;

		*buf = '\0';
		*buf2 = '\0';
		*buf3 = '\0';
		*buf4 = '\0';

		sprintf (buf + strlen(buf), "\n");

		sprintf (buf + strlen(buf),
			" %6s##   %-40.40s   %-8.8s   %-6.6s   %-8.8s   %-9.9s   %-12.12s\n", 
			"ID", "Order Available to Fill", "Price", "Num.", "Cond.", "Details", "Time Left");

		sprintf (buf + strlen(buf), 
			"---------------------------------------------------------------------------------------------------\n");

		if ( !mysql_num_rows(result) && !oneArg.empty())
			sprintf (buf + strlen(buf), "\nThere are no orders for that currently listed in this auction house.\n");
		else if ( !mysql_num_rows(result) )
			sprintf (buf + strlen(buf), "\nThere are no orders currently listed matching that criteria.\n");	

		while ( (row = mysql_fetch_row(result)) )
		{
			sprintf (buf2, "%s cp", row[5]);

			time_remaining = (atoi(row[4]) - time(0)) * 4;

			days = time_remaining / (60*60*24);
			time_remaining %= (60*60*24);
			hours = time_remaining / (60*60);
			time_remaining %= (60*60);
			minutes = time_remaining / 60;
			time_remaining %= 60;

			if ( !days && !hours && !minutes )
				sprintf (buf4, ">1m");
			else 
			{
				*buf4 = '\0';
				if ( days )
					sprintf (buf4 + strlen(buf4), "%dd ", days);
				if ( hours )
					sprintf (buf4 + strlen(buf4), "%dh ", hours);
				if ( minutes )
					sprintf (buf4 + strlen(buf4), "%dm ", minutes);
			}

			switch(atoi(row[11]))
			{
			case 1:
				sprintf(buf3, "good");
				break;
			case 2:
				sprintf(buf3, "fair");
				break;
			case 3:
				sprintf(buf3, "poor");
				break;
			case 4:
				sprintf(buf3, "bad");
				break;
			case 5:
				sprintf(buf3, "any");
				break;
			default:
				sprintf(buf3, "pristine");
				break;
			}

			*statbuf = '\0';
			
			if ( !str_cmp (row[6], GET_NAME(ch)) )
				strcat (statbuf, "*");
			if ( atoi(row[14]) == 1)
				strcat (statbuf, "#9#1F#0");

			sprintf (buf + strlen(buf),
				"%2s%4s.   #2%-40.40s#0   %-8.8s   %-6.6s   %-8.8s   %-9.9s   %-12.12s\n", 
				statbuf, row[0], row[10], buf2, row[12], buf3, (atoi(row[13]) ? "#6Yes#0" : " No "),  buf4);
		}

		sprintf (buf + strlen(buf), "\n* = Your Order, #9#1F#0 = Order Filled\n");

		page_string (ch->descr(), buf);

		if ( result )
			mysql_free_result (result);
	}
	else if (comArg == "preview")
	{
		MYSQL_RES	*result = NULL;
		MYSQL_ROW	row = NULL;
		char buf2 [MAX_STRING_LENGTH];
		char buf3 [MAX_STRING_LENGTH];
		int	house_id = 0;
		int id = 0;
		OBJ_DATA *obj = NULL;

		*buf2 = '\0';
		*buf3 = '\0';

		thisArgument = one_argument(thisArgument, oneArg);
		if (oneArg.empty() || !atoi(oneArg.c_str()))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I don't see any order by that number: type #6ORDER LIST#0 to see what orders are outstanding.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
		id = atoi(oneArg.c_str());

		house_id = keeper->mob->carcass_vnum;
		std::string world_log_db = engine.get_config ("world_log_db");

		sprintf (buf, 
			"SELECT * FROM %s.ah_orders WHERE (expires_at > UNIX_TIMESTAMP()) AND auction_id = %d AND house_id = %d AND status != 2", 
			world_log_db.c_str (), id, house_id);
		mysql_safe_query (buf);

		result = mysql_store_result (database);

		if ( !result || !mysql_num_rows(result) ) 
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I don't see any order by that number: type #6ORDER LIST#0 to see what orders are outstanding.", buf2);
			do_whisper (keeper, buf, 83);
			if ( result )
				mysql_free_result (result);
			return;
		}

		row = mysql_fetch_row(result);

		obj = vtoo(atoi(row[9]));
		int condition = atoi(row[11]);
		bool variable = false;

		char *zcolor[10];
		for (int ind = 0; ind < 10; ind++)
		{
			zcolor[ind] = '\0';
		}

		if (IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			variable = true;

			int k = 0;
			int x = 0;

			char original[MAX_STRING_LENGTH];
			char temp[MAX_STRING_LENGTH];
			char *point;
			sprintf (original, "%s", obj->full_description);
			point = strpbrk (original, "$");

			// If we found point...
			if (point)
			{
				// Then for every character in the string...
				// We run through the original, adding each bit of y to buf2.
				// However, if we find a $, we see if that's a category of variables.
				// If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

				for (size_t y = 0; y <= strlen (original); y++)
				{
					if (original[y] == *point)
					{
						k = y + 1;
						sprintf (temp, "$");
						// ... and jump ahead a point (to get to the letter after the $
						k = y + 1;

						// Now, until we hit something that's not a alpha-numeric character.
						while (isalpha (original[k]))
						{
							// add the word after the $ to our temporary marker.
							sprintf (temp + strlen (temp), "%c", original[k]);
							k++;
						}
						// Now, we set our end point as where our category ends plus 1.
						k = y + 1;

						// We advance until we get to the new non-alpha-numeric character.
						while (isalpha (original[k]))
							k++;

						//while (!isalpha (original[k]))
						//k++;

						// And then set the point of our main loop to that point
						y = k;
						zcolor[x] = add_hash(temp);
						x++;
					}
				}
			}
			else
			{
				variable = false;
			}
		}

		sprintf(buf,			   "Order #6%d#0 is explained by #5%s#0:\n\n", id, char_short(keeper));
		sprintf(buf + strlen(buf), "This order, placed by #5%s#0, will provide #6%s#0 chips for #6%s#0 of:\n", row[7], row[5], row[12]);
		sprintf(buf + strlen(buf), "   #2%s#0.\n", obj_short_desc(obj));
		sprintf(buf + strlen(buf), "The item must be in #3%s#0 condition.\n\n", (condition == 5 ? "any" : condition == 4 ? "at least bad" : condition == 3 ? "at least poor" : condition == 2 ? "at least fair" : condition == 1 ? "at least good" : "pristine"));
		sprintf(buf + strlen(buf), "The full description of this item is:\n#2%s#0\n", obj->full_description);
		send_to_char(buf, ch);

		if (variable)
		{
			sprintf(buf, "To fulfill the order, the item must have the following variables:\n");
			for (int ind = 0; ind <= 9; ind++)
			{
				if (zcolor[ind])
				{
					sprintf(buf + strlen(buf), "#3%12s#0 will need to be: #6%s#0\n", zcolor[ind], row[15 + ind]);
				}
			}

			send_to_char(buf, ch);
		}

		if ( result )
			mysql_free_result (result);
		obj = NULL;
		for (int ind = 0; ind <= 9; ind++)
		{
			mem_free(zcolor[ind]);
		}

	}
	else if (comArg == "fill")
	{
		MYSQL_RES	*result = NULL;
		MYSQL_ROW	row = NULL;
		char buf2 [MAX_STRING_LENGTH];
		char buf3 [MAX_STRING_LENGTH];
		int	house_id = 0;
		int id = 0;
		OBJ_DATA *obj = NULL;

		*buf2 = '\0';
		*buf3 = '\0';

		thisArgument = one_argument(thisArgument, oneArg);
		if (oneArg.empty() || !atoi(oneArg.c_str()))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I don't see any order by that number: type #6ORDER LIST#0 to see what orders are outstanding.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
		id = atoi(oneArg.c_str());

		thisArgument = one_argument(thisArgument, twoArg);

		if (twoArg.empty())
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s You tell me what you are using to fill this order, and have it in your hands!", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		if (!(obj = get_obj_in_dark (ch, (char*) twoArg.c_str(), ch->right_hand)) &&
			!(obj = get_obj_in_dark (ch, (char*) twoArg.c_str(), ch->left_hand)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s You tell me what you are using to fill this order, and have it in your hands!", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		if (obj->contains)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I only want the object, and nothing inside of it - I'm not getting paid to hold that stuff.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		house_id = keeper->mob->carcass_vnum;
		std::string world_log_db = engine.get_config ("world_log_db");

		sprintf (buf, 
			"SELECT * FROM %s.ah_orders WHERE (expires_at > UNIX_TIMESTAMP()) AND auction_id = %d AND house_id = %d", 
			world_log_db.c_str (), id, house_id);
		mysql_safe_query (buf);

		result = mysql_store_result (database);

		if ( !result || !mysql_num_rows(result) ) 
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I don't see any order by that number: type #6ORDER LIST#0 to see what orders are outstanding.", buf2);
			do_whisper (keeper, buf, 83);
			if ( result )
				mysql_free_result (result);
			return;
		}

		row = mysql_fetch_row(result);

		if (atoi(row[14]))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s That order has already been filled.", buf2);
			do_whisper (keeper, buf, 83);
			if ( result )
				mysql_free_result (result);
			return;
		}

		// Right vnum?
		if (atoi(row[9]) != obj->nVirtual)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s #2%s#0 doesn't match #2%s#0!", buf2, obj_short_desc(obj), row[10]);
			do_whisper (keeper, buf, 83);
			if ( result )
				mysql_free_result (result);
			return;
		}

		// Right amount?
		int count = atoi(row[12]);	
		if (count > obj->count)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s You need #6%d#0 of those to fill this order: you only have #6%d#0.", buf2, count, obj->count);
			do_whisper (keeper, buf, 83);
			if ( result )
				mysql_free_result (result);
			return;
		}

		// Right condition?
		int want_condition = atoi(row[11]);
		double want_multiplier = 1.00;
		int dam_condition = 0;
		bool condition_ok = true;
		// don't care for any
		if (want_condition != 5)
		{
			// If we've got zero damage, then our have_condition is pristine.
			if (!obj->damage)
			{
				dam_condition = 0;
			}
			else
			{
				// Otherwise: good = 1, fair = 2, poor = 3, bad = 4 or 5.
				dam_condition = object__determine_condition(obj) + 1;
				dam_condition = MAX(dam_condition, 4);
			}

			if (dam_condition > want_condition)
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s Sorry, but #2%s#0 is not in good enough condition to satisfy the demands of the order.", buf2, obj_short_desc(obj));
				do_whisper (keeper, buf, 83);
				if ( result )
					mysql_free_result (result);
				return;
			}

			if (want_condition == 1)
			{
				want_multiplier = 0.90;
			}
			else if (want_condition == 2)
			{
				want_multiplier = 0.75;
			}
			else if (want_condition == 3)
			{
				want_multiplier = 0.50;
			}
			else if (want_condition == 4)
			{
				want_multiplier = 0.25;
			}

			// We figure out how many of our remaining ovals are close to
			// our prototype - if we're too far off, we reject on the grounds of quality.
			if (GET_ITEM_TYPE(obj) == ITEM_COMPONENT ||
				GET_ITEM_TYPE(obj) == ITEM_TOOL ||
				GET_ITEM_TYPE(obj) == ITEM_REPAIR_KIT ||
				GET_ITEM_TYPE(obj) == ITEM_HEALER_KIT)
			{
				if ((int) ((double) obj->o.od.value[0] * want_multiplier) < vtoo(obj->nVirtual)->o.od.value[0])
				{
					condition_ok = false;
				}
			}
			else if (GET_ITEM_TYPE(obj) == ITEM_PROGRESS)
			{
				if ((int) ((double) obj->o.od.value[0] * (1.00 - want_multiplier)) < vtoo(obj->nVirtual)->o.od.value[0])
				{
					condition_ok = false;
				}
			}

			if (condition_ok == false)
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s Sorry, but #2%s#0 does not have enough uses to satisfy the demands of the order.", buf2, obj_short_desc(obj));
				do_whisper (keeper, buf, 83);
				if ( result )
					mysql_free_result (result);
				return;
			}
		}

		// Right variables?
		int want_variable = atoi(row[13]);
		bool variable_ok = true;

		if (want_variable)
		{
			for (int ind = 0; ind < 10; ind ++)
			{
				if (str_cmp(row[15+ind], "any variable"))
				{
					if (str_cmp(row[15+ind], obj->var_color[ind]))
					{
						variable_ok = false;
					}
				}
			}

			if (variable_ok == false)
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s Sorry, but #2%s#0 does not have the right variables to satisfy the demands of the order.", buf2, obj_short_desc(obj));
				do_whisper (keeper, buf, 83);
				if ( result )
					mysql_free_result (result);
				return;
			}
		}

		int price = atoi(row[5]);
		int auc_id = atoi(row[0]);

		if (cmd == 1)
		{
			FILE		*fp;

			name_to_ident (ch, buf2);
			sprintf (buf, "%s Good, good. Here is #6%d#0 chips for your #2%s#0. I'll be holding on to it until the person who made the order comes to retrieve it.", buf2, price, obj_short_desc(obj));
			do_whisper (keeper, buf, 83);

			// HM the orderer if they've got an account.
			if ((row[8]))
			{
				MUDMAIL_DATA *message;
				account *acct = NULL;
				char date[32];
				char *p;

				sprintf(buf, "The Auctioneer: #5%s#0", row[2]);
				sprintf(buf2, "Purchase of #2%s#0 for #6%s#0 credits", obj_short_desc(obj), row[5]);
				sprintf(buf3, "The item you had ordered for purchase with #5%s#0 has been bought.\n\n"
					"After the auctioneers commission, you have spent #6%s#0 credits on the sale of #2%s#0.\n\n"
					"Visit #5%s#0 to '#6retrieve#0' your item.\n", row[2], row[5], obj_short_desc(obj), row[2]);
				reformat_string (buf3, &p);

				CREATE (message, MUDMAIL_DATA, 1);
				message->from = str_dup (buf);
				message->subject = str_dup (buf2);
				message->message = str_dup (p);
				message->from_account = str_dup ("AuctionHouse");
				message->date = str_dup (date);
				message->flags = 0;
				message->target = str_dup (row[6]);

				acct = new account (row[8]);
				save_hobbitmail_message (acct, message);
				delete acct;
				mem_free(p);
				//sprintf (buf, "[ OOC: Your auction of #2%s#0 just sold. ]", row[15]);
				//act (buf, false, tch, NULL, NULL, TO_CHAR | _ACT_FORMAT);
			}


			OBJ_DATA	*board = NULL;
			// Write up this sale so others can learn how to make a good order.
			if ((board = vtoo(atoi(row[3]))))
			{
				char *p;

				sprintf(buf, "#3Bought#0: #2%s#0 for #6%s#0 credits.", obj_short_desc(obj), row[5]);

				sprintf(buf3, "At this time, #2%s#0 was #3sold#0 for #6%d#0 credits by #5%s#0.\n\n"
					"The item was ordered for sale by #5%s#0.",
					obj_short_desc(obj), atoi(row[5]), char_short(ch), row[7]);                                      

				reformat_string (buf3, &p);

				post_straight_to_mysql_board(fname((board)->name), buf, (char*) char_short(ch), p);

				mem_free(p);
			}			

			if ( result )
				mysql_free_result (result);

			sprintf (buf,
				"UPDATE %s.ah_orders SET status = 1 WHERE auction_id = %d",
				world_log_db.c_str (), auc_id);
			mysql_safe_query (buf);

			sprintf (buf, "%s/%d", ORDER_DIR, id);
			fp = fopen (buf, "w");
	
			fwrite_a_obj (obj, fp);
			fclose (fp);
			obj_from_char(&obj, count);
			extract_obj(obj);

			keeper_money_to_char (keeper, ch, price);
		}
		else
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s All right, #2%s#0 looks like it will fill the order. I'll take it off your hands for #6%d#0 coins if you type #6ACCEPT#0.", buf2, obj_short_desc(obj), price);
			do_whisper (keeper, buf, 83);

			ch->delay_type = DEL_ORDER_FULFILL;
			ch->delay_ch = keeper;
			ch->delay_who = add_hash(argument);
		}
	}
	else
	{
		send_to_char ("Syntax - <required> fields and (optional) fields: \n"
			"Place an order     : order place (quantity) (condition) <item-number> <offered-price> (variable1) (variable2) ... (variable 10): \n"
			"Valid conditions are #6pristine#0, #2good#0, #3fair#0, #1poor#0, #5bad#0 or #9any#0.\n\n"
			"Find items to order: order search <keyword> (second keyword)\n"
			"Retrieve an item   : order retrieve\n"
			"\n"
			"Fill an order      : order fill <order number> <keyword>\n"
			"See orders to fill : order list\n"
			"Preview an order   : order preview <order number>\n", ch);
		return;
	}
}

void
	refresh_colors (CHAR_DATA * keeper)
{
	ROOM_DATA *room;
	OBJ_DATA *tobj, *next_obj;
	int i = 0, j = 0, reload_objs[500];

	if (!IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER)
		|| !keeper->shop)
		return;

	if (!(room = vnum_to_room (keeper->shop->store_vnum)))
		return;

	for (tobj = room->contents; tobj; tobj = next_obj)
	{
		next_obj = tobj->next_content;
		if (keeper_makes (keeper, tobj->nVirtual)
			&& IS_SET (tobj->obj_flags.extra_flags, ITEM_VARIABLE)
			&& !number (0, 1))
		{
			reload_objs[i] = tobj->nVirtual;
			i++;
			extract_obj (tobj);
		}
	}

	if (i)
	{
		for (j = 0; j < i; j++)
		{
			tobj = load_object (reload_objs[j]);
			if (tobj)
			{
				obj_to_room (tobj, room->vnum);
				fluid_object(tobj);
			}
		}
	}
}

void
	manage_resources (void)
{

	int true_hour = time_info.hour + (84 * time_info.day);	


	if (true_hour != 0)
		return;

	send_to_gods("Managed Resources");

	CHAR_DATA * keeper = NULL;
	ROOM_DATA *room = NULL;
	OBJ_DATA *tobj = NULL;
	int target = 0;

	for (keeper = character_list; keeper; keeper = keeper->next)
	{
		if (!IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER)
			|| !keeper->shop || !IS_SET(keeper->act, ACT_BULKTRADER))
			continue;

		if (!(room = vnum_to_room (keeper->shop->store_vnum)))
			continue;

		if (IS_SET (keeper->act, ACT_NOVNPC))
			continue;

		if (!room->psave_loaded)
			load_save_room (room);

		target = number(90,110);

		for (tobj = room->contents; tobj; tobj = tobj->next_content)
		{		
			if (GET_ITEM_TYPE (tobj) != ITEM_RESOURCE)
				continue;

			if (keeper_makes(keeper, tobj->nVirtual) && tobj->count < target)
			{
				tobj->count += 3;
			}
			else if (tobj->count > target)
			{
				tobj->count -= 3;
			}
		}
	}

	return;
}


int
	vnpc_customer (CHAR_DATA * keeper, int purse)
{
	ROOM_DATA *room;
	OBJ_DATA *tobj;
	int items_in_list = 0, target_item = 0, i = 0;
	int required_check = 0, item_cost = 0;
	float delivery_cost = 0;

	if (!IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER)
		|| !keeper->shop)
		return purse;

	if (IS_SET (keeper->act, ACT_NOVNPC))
		return purse;

	if (!(room = vnum_to_room (keeper->shop->store_vnum)))
		return purse;

	if (!room->psave_loaded)
		load_save_room (room);

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
			continue;
		items_in_list++;
	}

	if (!items_in_list)
		return purse;

	if (items_in_list == 1)
		target_item = 1;
	else
		target_item = number (1, items_in_list);

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
			continue;
		i++;
		if (i == target_item)
			break;
	}

	if (!tobj)
		return purse;

	// vNPC sales on things we produce just screw things up.

	if (keeper_makes (keeper, tobj->nVirtual))
		return purse;

	// Cost of item being sold to vNPC
	item_cost = (int) calculate_sale_price (tobj, keeper, NULL, 1, true, false);

	// Cost of ordering replacement item for merchant
	delivery_cost = calculate_sale_price (tobj, keeper, NULL, 1, true, true);

	if (item_cost > VNPC_COPPER_PURSE)
		return item_cost;

	required_check = 55 - (item_cost / 4);
	required_check = MAX (6, required_check);
	int port = engine.get_port ();

	if (number (1, 100) <= required_check)
	{
		target_item = tobj->nVirtual;
		obj_from_room (&tobj, 1);

		money_to_storeroom (keeper, item_cost);

		mysql_safe_query
			("INSERT INTO %s.receipts "
			"(time, shopkeep, transaction, who, customer, vnum, "
			"item, qty, cost, room, gametime, port) "
			"VALUES (NOW(),%d,'sold','%s','%s',%d,'%s',%d,%d,%d,'%d-%d-%d %d:00',%d)",
			(engine.get_config ("player_log_db")).c_str (),
			keeper->mob->vnum, "vNPC Customer",
			"an honest-looking person", tobj->nVirtual,
			tobj->short_description, 1, (int) item_cost, keeper->in_room,
			time_info.year, time_info.month + 1, time_info.day + 1,
			time_info.hour, port);

		if (keeper_makes (keeper, target_item)
			&& !get_obj_in_list_num (target_item, room->contents))
		{
			if (keeper_has_money (keeper, (int) delivery_cost))
			{
				subtract_keeper_money (keeper, (int) delivery_cost);
				obj_to_room (load_object (target_item),
					keeper->shop->store_vnum);
				mysql_safe_query
					("INSERT INTO %s.receipts "
					"(time, shopkeep, transaction, who, customer, vnum, "
					"item, qty, cost, room, gametime, port) "
					"VALUES (NOW()+1,%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
					(engine.get_config ("player_log_db")).c_str (),
					keeper->mob->vnum, "vNPC Merchant",
					"an honest-looking merchant", tobj->nVirtual,
					tobj->short_description, 1, delivery_cost,
					keeper->in_room, time_info.year, time_info.month + 1,
					time_info.day + 1, time_info.hour, port);
			}
		}
		extract_obj (tobj);
	}

	return item_cost;
}


#define NO_SUCH_ITEM1 "Don't seem to have that in my inventory. Would you like to buy something else?"
#define NO_SUCH_ITEM2 "I don't see that particular item. Perhaps you misplaced it?"
#define MISSING_CASH1 "A little too expensive for me now -- why don't you try back later?"
#define MISSING_CASH2 "You're a little short on credits, I see; come back when you can afford it."
#define DO_NOT_BUY    "I don't buy those sorts of things, I'm afraid."

void
	do_list (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char stock_buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];
	int i;
	float val_in_farthings = 0;
	int header_said = 0;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	OBJ_DATA *obj;
	NEGOTIATION_DATA *neg;

	room = ch->room;

	argument = one_argument (argument, buf);

	if (*buf && (keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;
	}

	if (!keeper)
	{
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if (!GET_TRUST (ch) && !CAN_SEE (keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if (GET_POS (keeper) <= POSITION_SLEEPING)
	{
		act ("$N is not conscious.", true, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (GET_POS (keeper) == POSITION_FIGHTING)
	{
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if (!keeper->shop || !IS_SET (keeper->flags, FLAG_KEEPER))
	{

		if (keeper == ch)
			send_to_char ("You don't have a shop.\n", ch);
		else
			act ("$N isn't a shopkeeper.", false, ch, 0, keeper, TO_CHAR);

		return;
	}

	if (keeper->shop->shop_vnum && keeper->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	i = 0;

	*output = '\0';

	for (obj = store->contents; obj; obj = obj->next_content)
	{

		i++;

		if (GET_ITEM_TYPE (obj) == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
		{
			i--;
			if (obj->next_content)
			{
				continue;
			}
			else
				break;
		}

		if (*buf
			&& (!isname (buf, obj->name)
			&& !(GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title
			&& isname (buf, obj->book_title))
			&& !(GET_ITEM_TYPE (obj) == ITEM_DRINKCON
			&& obj->contains
			&& isname (buf, obj->contains->name))))
			continue;

		if (!CAN_WEAR (obj, ITEM_TAKE))
			continue;

		/* Prevent players from buying back items they've sold, and prevent
		all others from buying a sold item for 15 minutes to prevent abuse */

		// Removing this until I see players abusing it like an Austrian girl locked in
		// an underground cellar complex.
		// - special K

		/*
		if ((obj->sold_by != ch->coldload_id
		&& (time (0) - obj->sold_at <= 60 * 15))
		|| (obj->sold_by == ch->coldload_id
		&& (time (0) - obj->sold_at <= 60 * 60)))
		continue;
		*/

		if (!header_said)
		{
			act ("$N describes $S inventory:", false, ch, 0, keeper, TO_CHAR);
			sprintf (output + strlen (output),
				"\n   #      price        item\n");
			sprintf (output + strlen (output), "  ===     =====        ====\n");
			header_said = 1;
		}

		val_in_farthings =
			calculate_sale_price (obj, keeper, ch, 1, false, false);

		if (val_in_farthings == 0 && obj->obj_flags.set_cost == 0)
			val_in_farthings = 1;

		*stock_buf = '\0';

		if (!keeper_makes (keeper, obj->nVirtual))
			sprintf (stock_buf, "(%d in stock)", obj->count);

		// We don't want the number of resources to be displayed, so show the
		// actual short description, not the short desc modified via count et al.
		// - special K

		// Tossed it in so it displays both the buy/sell price, for ease of use.

		char amt[30] = "";
		char ramt[30] = "";

		if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE)
		{

			sprintf (amt,"%7.2f #9#1sell#0#1",val_in_farthings);
			sprintf (ramt,"%7.2f #9#2buy#0#1", calculate_sale_price (obj, keeper, ch, 1, false, true));
			sprintf (buf2, "  #1%3d   %s%s%s%s  #2%-80.80s#0",
				i, ((val_in_farthings) ? amt : "   free   "),
				"#9#6 /#0#1", ((val_in_farthings) ? ramt : "   free   "),
				"   ", obj->short_description);
		}
		else if (IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) && keeper_makes (keeper, obj->nVirtual))
		{
			sprintf (amt,"%7.2f cp",val_in_farthings);
			sprintf (buf2, "  #1%3d   %s%s  #2%-80.80s#0",
				i, ((val_in_farthings) ? amt : "   free   "),
				"   ", obj_short_desc (vtoo(obj->nVirtual)));
		}
		else
		{
			sprintf (amt,"%7.2f cp",val_in_farthings);
			sprintf (buf2, "  #1%3d   %s%s  #2%-80.80s#0",
				i, ((val_in_farthings) ? amt : "   free   "),
				"   ", obj_short_desc (obj));
		}

		for (neg = keeper->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual)
				break;
		}

		if (strlen (obj_short_desc (obj)) > 72)
		{
			buf2[94] = '.';
			buf2[95] = '.';
			buf2[96] = '.';
		}
		buf2[97] = '\0';
		strcat (buf2, "#0");

		/*
		if (IS_WEARABLE (obj))
		{
		if (obj->size)
		sprintf (buf2 + strlen (buf2), " (%s)", sizes_named[obj->size]);
		else if (keeper_makes (keeper, obj->nVirtual))
		sprintf (buf2 + strlen (buf2), " (all sizes)");
		else
		strcat (buf2, " (all sizes)");
		}
		*/

		if (IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) && keeper_makes (keeper, obj->nVirtual))
			sprintf (buf2 + strlen (buf2), " #2(variable)#0");

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
			strcat (buf2, " #6(used)#0");

		sprintf (buf2 + strlen (buf2), "%s", neg ? " (neg)" : "");

		strcat (buf2, "\n");

		if (strlen (output) + strlen (buf2) > MAX_STRING_LENGTH)
			break;

		sprintf (output + strlen (output), "%s", buf2);
	}

	if (!header_said)
	{
		if (*buf)
			act ("$N doesn't have any of those.", false, ch, 0, keeper, TO_CHAR);
		else
			act ("Sadly, $N has nothing to sell.", false, ch, 0, keeper, TO_CHAR);
	}
	else
		page_string (ch->descr(), output);
}

void
	do_preview (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	OBJ_DATA *obj;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	bool found = false;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];

	/* buy [keeper] [count] item [size | !] */

	/* cmd is 1 when this is a barter. */

	room = ch->room;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Preview what?\n", ch);
		return;
	}

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!keeper)
		{
			send_to_char ("There is no shopkeeper here.\n", ch);
			return;
		}

		if (!*buf)
		{
			act ("PREVIEW what from $N?", true, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if (GET_POS (keeper) <= POSITION_SLEEPING)
	{
		act ("$N is not conscious.", true, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (GET_POS (keeper) == POSITION_FIGHTING)
	{
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if (!keeper->shop)
	{

		if (keeper == ch)
			send_to_char ("You are not a shopkeeper.", ch);
		else
			act ("$N is not a keeper.", false, ch, 0, keeper, TO_CHAR);

		return;
	}

	if (keeper->shop->shop_vnum && keeper->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if (store->contents && is_number (buf))
	{

		obj = store->contents;

		while (obj && GET_ITEM_TYPE (obj) == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			obj = obj->next_content;

		for (i = 1;; i++)
		{

			if (!obj)
				break;

			if (GET_ITEM_TYPE (obj) == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			{
				i--;
				if (obj->next_content)
				{
					obj = obj->next_content;
					continue;
				}
				else
					break;
			}

			if (i == atoi (buf))
			{
				found = true;
				break;
			}

			obj = obj->next_content;
		}

		if (!obj || !found)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	else if (!(obj = get_obj_in_list_vis_not_money (ch, buf,
		vnum_to_room (keeper->shop->
		store_vnum)->
		contents)))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
		do_whisper (keeper, buf, 83);
		return;
	}


	if (IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) && keeper_makes (keeper, obj->nVirtual))
	{
		act ("$N shows you $p.", false, ch, vtoo(obj->nVirtual), keeper, TO_CHAR);
		send_to_char("\n", ch);
		sprintf(buf3, "To see a sample description of this object, type #6buy %s var#0.\n", buf);
		send_to_char(buf3, ch);
	}
	else
	{
		act ("$N shows you $p.", false, ch, obj, keeper, TO_CHAR);
		send_to_char ("\n", ch);

		show_obj_to_char (obj, ch, 15);
		show_obj_to_char (obj, ch, 5);
	}



	//append the output of the evaluate command
	show_evaluate_information(ch, obj);
}

int
	keeper_has_item (CHAR_DATA * keeper, int ovnum)
{
	OBJ_DATA *tobj;
	ROOM_DATA *room;

	if (!keeper || !(room = vnum_to_room (keeper->shop->store_vnum)) || !ovnum)
		return 0;

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (tobj->nVirtual == ovnum
			&& !IS_SET (vtoo (ovnum)->obj_flags.extra_flags, ITEM_VARIABLE))
			return 1;
	}

	return 0;
}

int
	keeper_makes (CHAR_DATA * keeper, int ovnum)
{
	int i;

	if (!keeper || !keeper->shop || !ovnum)
		return 0;

	for (i = 0; i < MAX_TRADES_IN; i++)
		if (keeper->shop->delivery[i] == ovnum)
			return 1;

	return 0;
}

void
	money_to_storeroom (CHAR_DATA * keeper, int amount)
{
	OBJ_DATA *obj, *next_obj;
	ROOM_DATA *store;
	int money = 0;

	if (!keeper->shop)
		return;
	if (!keeper->shop->store_vnum)
		return;
	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
		return;

	while (keeper_has_money (keeper, 1))
	{
		for (obj = store->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
			{
				money += ((int) obj->farthings) * obj->count;
				obj_from_room (&obj, 0);
				extract_obj (obj);
			}
		}
	}
	money += amount;

	if (keeper->mob->currency_type == CURRENCY_FOOD)
	{

		if (money / 240)
		{
			// Silver tree.
			obj = load_object (50093);
			obj->count = money / 240;
			obj_to_room (obj, store->vnum);
			money %= 240;
		}

		if (money / 25)
		{
			// Silver royal.
			obj = load_object (50092);
			obj->count = money / 25;
			obj_to_room (obj, store->vnum);
			money %= 25;
		}

		if (money / 5)
		{
			// Bronze copper.
			obj = load_object (50091);
			obj->count = money / 5;
			obj_to_room (obj, store->vnum);
			money %= 5;
		}
		if (money)
		{
			// Copper bit.
			obj = load_object (50090);
			obj->count = money;
			obj_to_room (obj, store->vnum);
		}
	}
	else if (keeper->mob->currency_type == CURRENCY_PHOENIX)
	{
		if (money / 1000)
		{
			obj = load_object (14017);
			obj->count = money / 1000;
			obj_to_room (obj, store->vnum);
			money %= 1000;
		}

		if (money / 100)
		{
			obj = load_object (14016);
			obj->count = money / 100;
			obj_to_room (obj, store->vnum);
			money %= 100;
		}

		if (money / 50)
		{
			obj = load_object (14015);
			obj->count = money / 50;
			obj_to_room (obj, store->vnum);
			money %= 50;
		}

		if (money / 20)
		{
			obj = load_object (14014);
			obj->count = money / 20;
			obj_to_room (obj, store->vnum);
			money %= 20;
		}

		if (money / 10)
		{
			obj = load_object (14013);
			obj->count = money / 10;
			obj_to_room (obj, store->vnum);
			money %= 10;
		}

		if (money / 5)
		{
			obj = load_object (14012);
			obj->count = money / 5;
			obj_to_room (obj, store->vnum);
			money %= 5;
		}

		if (money)
		{
			obj = load_object (14011);
			obj->count = money;
			obj_to_room (obj, store->vnum);
		}
	}
}

void
	subtract_keeper_money (CHAR_DATA * keeper, int cost)
{
	OBJ_DATA *obj, *next_obj;
	ROOM_DATA *store;
	int money = 0;

	if (!keeper->shop)
		return;
	if (!keeper->shop->store_vnum)
		return;
	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
		return;

	while (keeper_has_money (keeper, 1))
	{
		for (obj = store->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (GET_ITEM_TYPE (obj) == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			{
				money += ((int) obj->farthings) * obj->count;
				obj_from_room (&obj, 0);
				extract_obj (obj);
			}
		}
	}

	money -= cost;

	if (keeper->mob->currency_type == CURRENCY_FOOD)
	{
		if (money / 240)
		{
			// Silver tree.
			//if (keeper->mob->currency_type == CURRENCY_ORKISH) -- other currencies
			obj = load_object (50093);
			obj->count = money / 240;
			obj_to_room (obj, store->vnum);
			money %= 240;
		}

		if (money / 25)
		{
			// Silver royal.
			obj = load_object (50092);
			obj->count = money / 25;
			obj_to_room (obj, store->vnum);
			money %= 25;
		}

		if (money / 5)
		{
			// Bronze copper.
			obj = load_object (50091);
			obj->count = money / 5;
			obj_to_room (obj, store->vnum);
			money %= 5;
		}
		if (money)
		{
			// Copper bit.
			obj = load_object (50090);
			obj->count = money;
			obj_to_room (obj, store->vnum);
		}
	}
	else if (keeper->mob->currency_type == CURRENCY_PHOENIX)
	{
		if (money / 1000)
		{
			obj = load_object (14017);
			obj->count = money / 1000;
			obj_to_room (obj, store->vnum);
			money %= 1000;
		}

		if (money / 100)
		{
			obj = load_object (14016);
			obj->count = money / 100;
			obj_to_room (obj, store->vnum);
			money %= 100;
		}

		if (money / 50)
		{
			obj = load_object (14015);
			obj->count = money / 50;
			obj_to_room (obj, store->vnum);
			money %= 50;
		}

		if (money / 20)
		{
			obj = load_object (14014);
			obj->count = money / 20;
			obj_to_room (obj, store->vnum);
			money %= 20;
		}

		if (money / 10)
		{
			obj = load_object (14013);
			obj->count = money / 10;
			obj_to_room (obj, store->vnum);
			money %= 10;
		}

		if (money / 5)
		{
			obj = load_object (14012);
			obj->count = money / 5;
			obj_to_room (obj, store->vnum);
			money %= 5;
		}

		if (money)
		{
			obj = load_object (14011);
			obj->count = money;
			obj_to_room (obj, store->vnum);
		}
	}
}

void
	do_buy (CHAR_DATA * ch, char *argument, int cmd)
{
	int buy_count = 1;
	float keepers_cost = 0;
	float delivery_cost = 0;
	int i;
	int regardless = 0;
	int wants_off_size = 0;
	int size = -1;
	int nobarter_flag;
	int discount = 0;
	int keeper_success;
	int language_barrier = 0;
	int ch_success, flags = 0;
	int orig_count = 0;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *horse;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	NEGOTIATION_DATA *neg = NULL;
	char name[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	char buf4[MAX_STRING_LENGTH] = {'\0'};

	bool variable = false;
	bool variable_help = false;

	char *xcolor[10];
	for (int ind = 0; ind < 10; ind++)
	{
		xcolor[ind] = '\0';
	}

	/* buy [keeper] [count] item [size | ! | variable] */

	/* cmd is 1 when this is a barter. */

	/* cmd is 2 when confirming a purchase */

	room = ch->room;

	argument = one_argument (argument, buf);

	if (cmd != 2)
	{
		if (!*buf)
		{
			send_to_char ("Buy what?\n", ch);
			return;
		}

		if ((keeper = get_char_room_vis (ch, buf))
			&& IS_SET (keeper->flags, FLAG_KEEPER))
			argument = one_argument (argument, buf);

		else
		{
			for (tch = room->people; tch; tch = tch->next_in_room)
				if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
					break;

			keeper = tch;

			if (!*buf)
			{
				act ("Buy what from $N?", true, ch, 0, keeper, TO_CHAR);
				return;
			}
		}

		if (!keeper || !keeper->shop)
		{
			send_to_char("Buy from whom?\n", ch);
			return;
		}

		if (!(store = vnum_to_room (keeper->shop->store_vnum)))
		{
			do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",	0);
			return;
		}

		argument = one_argument (argument, buf2);

		/* buf is a count if buf2 is an object */

		if (isdigit (*buf) && *buf2 && (isdigit (*buf2) || (keeper && keeper->shop
			&& get_obj_in_list_vis_not_money (ch, buf2, store->contents))))
		{
			buy_count = atoi (buf);

			if (buy_count > 50)
			{
				send_to_char ("You can only buy up to 50 items at a time.\n",
					ch);
				return;
			}
			strcpy (buf, buf2);
			argument = one_argument (argument, buf2);
		}

		if (*buf2 == '!')
			regardless = 1;
		else if (!str_cmp(buf2, "variable") || !str_cmp(buf2, "var") || !str_cmp(buf2, "v"))
		{
			argument = one_argument (argument, buf3);
			variable = true;

			if (*buf3)
			{
				if (!str_cmp(buf3, "?"))
				{
					variable_help = true;
					variable = false;
				}
			}
		}
		else if (*buf2)
		{
			size = index_lookup (sizes_named, buf2);

			if (size == -1)
				size = index_lookup (sizes, buf2);

			wants_off_size = 1;
		}
	}
	else
		keeper = ch->delay_ch;

	if (!keeper || keeper->room != ch->room)
	{
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if (keeper == ch)
	{
		send_to_char ("You can't buy from yourself!\n", ch);
		return;
	}

	if (GET_POS (keeper) <= POSITION_SLEEPING)
	{
		act ("$N is not conscious.", true, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (GET_POS (keeper) == POSITION_FIGHTING)
	{
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if (!GET_TRUST (ch) && !CAN_SEE (keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if (!keeper->shop)
	{

		if (keeper == ch)
			send_to_char ("You are not a shopkeeper.", ch);
		else
			act ("$N is not a keeper.", false, ch, 0, keeper, TO_CHAR);

		return;
	}

	if (!keeper->shop || !IS_NPC (keeper))
	{
		send_to_char ("Are you sure they're a shopkeeper?\n", ch);
		return;
	}

	if (keeper->shop->shop_vnum && keeper->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if (cmd != 2)
	{
		if (is_number (buf))
		{

			obj = store->contents;

			while (obj && GET_ITEM_TYPE (obj) == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type,
				obj))
				obj = obj->next_content;

			for (i = 1;; i++)
			{

				if (!obj)
					break;

				if (GET_ITEM_TYPE (obj) == ITEM_MONEY
					&& keeper_uses_currency_type (keeper->mob->currency_type,
					obj))
				{
					i--;
					if (obj->next_content)
					{
						obj = obj->next_content;
						continue;
					}
					else
					{
						obj = NULL;
						break;
					}
				}

				/* Prevent players from buying back items they've sold, and prevent
				all others from buying a sold item for 15 minutes to prevent abuse

				if ((obj->sold_by != ch->coldload_id
				&& (time (0) - obj->sold_at <= 60 * 15))
				|| (obj->sold_by == ch->coldload_id
				&& (time (0) - obj->sold_at <= 60 * 60)))
				{
				i--;
				if (obj->next_content)
				{
				obj = obj->next_content;
				continue;
				}
				else
				{
				obj = NULL;
				break;
				}
				}*/

				if (i == atoi (buf))
					break;

				obj = obj->next_content;
			}

			if (!obj)
			{
				send_to_char ("There are not that many items in the keeper's "
					"inventory.\n", ch);
				return;
			}
		}

		else if (!(obj = get_obj_in_list_vis_not_money (ch, buf,
			vnum_to_room (keeper->shop->
			store_vnum)->
			contents)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}

		if ((variable == true || variable_help == true) && (!IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) || !keeper_makes (keeper, obj->nVirtual)))
		{
			send_to_char ("You cannot buy that item as a variable - just try buying it normally.\n", ch);
			return;
		}

		if (IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) && keeper_makes (keeper, obj->nVirtual))
		{
			if (variable == true || variable_help == true)
			{
				int k = 0;
				int x = 0;

				char original[MAX_STRING_LENGTH];
				char temp[MAX_STRING_LENGTH];
				char *point;
				sprintf (original, "%s", vtoo(obj->nVirtual)->full_description);
				point = strpbrk (original, "$");

				char *zcolor[10];
				for (int ind = 0; ind < 10; ind++)
				{
					zcolor[ind] = '\0';
				}

				// If we found point...
				if (point)
				{
					sprintf(buf4, "\n#2%s#0 has following variables you can specify when purchasing it:\n", vtoo(obj->nVirtual)->short_description);
					// Then for every character in the string...
					// We run through the original, adding each bit of y to buf2.
					// However, if we find a $, we see if that's a category of variables.
					// If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

					for (size_t y = 0; y <= strlen (original); y++)
					{
						if (original[y] == *point)
						{
							k = y + 1;
							sprintf (temp, "$");
							// ... and jump ahead a point (to get to the letter after the $
							k = y + 1;

							// Now, until we hit something that's not a alpha-numeric character.
							while (isalpha (original[k]))
							{
								// add the word after the $ to our temporary marker.
								sprintf (temp + strlen (temp), "%c", original[k]);
								k++;
							}
							// Now, we set our end point as where our category ends plus 1.
							k = y + 1;

							// We advance until we get to the new non-alpha-numeric character.
							while (isalpha (original[k]))
								k++;

							//while (!isalpha (original[k]))
							//k++;

							// And then set the point of our main loop to that point
							y = k;
							zcolor[x] = add_hash(temp);
							x++;
							sprintf(buf4 + strlen(buf4), "\n   #2%d:#0 %s", x,temp);
						}
					}

					if (variable_help == true)
					{
						std::string temp1, temp2;
						temp1 = zcolor[0];
						temp1.erase(0,1);

						if (zcolor[1])
						{
							temp2 = zcolor[1];
							temp2.erase(0,1);

							sprintf(buf4 + strlen(buf4), "\n\nFor example: #6buy %s var '%s' '%s'#0 etc...\n", buf, vc_rand(zcolor[0]), vc_rand(zcolor[1]));
							sprintf(buf4 + strlen(buf4), "You need to put the variables in the order listed above\n\n");
							sprintf(buf4 + strlen(buf4), "Type #6tags#0 to see the range of variables you can select, \nand #6tags <example>#0 to see the specific variables\n");
							sprintf(buf4 + strlen(buf4), "For example, #6tags %s#0 or #6tags %s#0.\n\n", temp1.c_str(), temp2.c_str());
							sprintf(buf4 + strlen(buf4), "To have a variable picked at random, you can use the ! character.\n");
							sprintf(buf4 + strlen(buf4), "For example: #6buy %s var ! '%s'\n\n", buf, vc_rand(zcolor[1]));
							sprintf(buf4 + strlen(buf4), "You can use #6tags finecolor#0 and #6tags drabcolor#0 variables in place of #6color#0, too.\n");
							sprintf(buf4 + strlen(buf4), "Finally you can just type #6buy %s var#0 to see a random possibility available to you.\n", buf);
						}
						else
						{
							sprintf(buf4 + strlen(buf4), "\n\nFor example: #6buy %s var '%s'\n", buf, vc_rand(zcolor[0]));
							sprintf(buf4 + strlen(buf4), "You need to put the variables in the order listed above\n\n");
							sprintf(buf4 + strlen(buf4), "Type #6tags#0 to see the range of variables you can select, \nand #6tags <example>#0 to see the specific variables\n");
							sprintf(buf4 + strlen(buf4), "For example, #6tags %s#0.\n\n", temp1.c_str());
							sprintf(buf4 + strlen(buf4), "To have a variable picked at random, you can use the ! character.\n");
							sprintf(buf4 + strlen(buf4), "For example: #6buy %s var !\n\n", buf);
							sprintf(buf4 + strlen(buf4), "You can use #6tags finecolor#0 and #6tags drabcolor#0 variables in place of #6color#0, too.\n");
							sprintf(buf4 + strlen(buf4), "Finally you can just type #6buy %s var#0 to see a random possibility available to you.\n", buf);
						}

						send_to_char (buf4, ch);

						for (int ind = 0; ind <= 9; ind++)
						{
							mem_free(xcolor[ind]);
							mem_free(zcolor[ind]);
						}
						return;
					}
					else if (variable == true)
					{
						if (*buf3)
						{
							if (vd_variable(buf3) != 0)
							{
								for (int xind = 0; xind <= 9; xind ++)
								{
									if (vd_variable(buf3) != 0 || !str_cmp(buf3, "!"))
									{
										xcolor[xind] = str_dup(buf3);
									}
									else
										break;

									argument = one_argument (argument, buf3);
								}
							}
						}

						if (ch->room->vnum == 221 && (!xcolor[0] || !xcolor[1] || !xcolor[2] || !xcolor[3] || !xcolor[4]))
						{
							send_to_char("You need to specify all the variables for your firearm, and can only pick chrion metal - see the room description for more help.\n", ch);

							for (int ind = 0; ind <= 9; ind++)
							{
								mem_free(xcolor[ind]);
								mem_free(zcolor[ind]);
							}
							return;

						}

						for (int ind = 0; ind < 10; ind++)
						{
							if (xcolor[ind] && zcolor[ind])
							{
								if (!str_cmp(xcolor[ind], "!"))
								{
									xcolor[ind] = str_dup(vc_rand(zcolor[ind]));
								}

								if (ch->room->vnum >= 202 && ch->room->vnum <= 299)
								{
									if (!str_cmp(zcolor[ind], "$ironalloy"))
									{
										xcolor[ind] = str_dup("chrion");
									}
									else if (!str_cmp(zcolor[ind], "$polymesh"))
									{
										xcolor[ind] = str_dup("coarse");
									}
									else if (!str_cmp(zcolor[ind], "$polyprop"))
									{
										xcolor[ind] = str_dup("uneven");
									}
									else if (!str_cmp(zcolor[ind], "$rifleframe"))
									{
										if (!str_cmp(xcolor[ind], "automatic rifle") || !str_cmp(xcolor[ind], "assault rifle") || !str_cmp(xcolor[ind], "rifle"))
										{
											xcolor[ind] = str_dup("hunting rifle");
										}
									}
									else if (!str_cmp(zcolor[ind], "$smgframe"))
									{
										if (!str_cmp(xcolor[ind], "heavy submachine gun"))
										{
											xcolor[ind] = str_dup("submachine gun");
										}
									}
									else if (!str_cmp(zcolor[ind], "$pistolframe"))
									{
										if (!str_cmp(xcolor[ind], "pistol"))
										{
											xcolor[ind] = str_dup("compact pistol");
										}
										else if (!str_cmp(xcolor[ind], "revolver"))
										{
											xcolor[ind] = str_dup("compact revolver");
										}
									}
									else if (!str_cmp(zcolor[ind], "$riflecaliber"))
									{
										if (str_cmp(xcolor[ind], ".30"))
										{
											xcolor[ind] = str_dup(".30");
										}
									}
									else if (!str_cmp(zcolor[ind], "$pistolcaliber"))
									{
										if (str_cmp(xcolor[ind], ".25"))
										{
											xcolor[ind] = str_dup(".25");
										}
									}
									else if (!str_cmp(zcolor[ind], "$smgcaliber"))
									{
										if (str_cmp(xcolor[ind], ".20"))
										{
											xcolor[ind] = str_dup(".20");
										}
									}
								}

								if (!vc_exists(xcolor[ind], zcolor[ind]))
								{
									if (str_cmp(zcolor[ind], "$color") || (!vc_exists(xcolor[ind], "$finecolor") && !vc_exists(xcolor[ind], "$drabcolor")))
									{
										std::string temp;
										temp = zcolor[ind];
										temp.erase(0,1);
										sprintf(buf4, "There isn't a #2%s#0 in the #2%s#0 variable category. Use the #6tags %s#0 command to see acceptable variables.\n", xcolor[ind], temp.c_str(), temp.c_str());
										send_to_char(buf4, ch);

										for (int ind = 0; ind <= 9; ind++)
										{
											mem_free(xcolor[ind]);
											mem_free(zcolor[ind]);
										}

										return;
									}
								}
								else if (vc_checkrand(xcolor[ind], zcolor[ind]))
								{
									std::string temp;
									temp = zcolor[ind];
									temp.erase(0,1);
									sprintf(buf4, "You aren't allowed to select variable #2%s#0 of the #2%s#0 category. See #6tags %s#0 to see which variable are allowed.\n", xcolor[ind], temp.c_str(), temp.c_str());
									send_to_char(buf4, ch);
									for (int ind = 0; ind <= 9; ind++)
									{
										mem_free(xcolor[ind]);
										mem_free(zcolor[ind]);
									}

									return;
								}
							}
						}
					}
				}

				for (int ind = 0; ind <= 9; ind++)
				{
					mem_free(zcolor[ind]);
				}
			}
			else
			{
				sprintf(buf4, "You can only buy that object as a variable. Type #6buy %s var ?#0 to get started.", buf);
				act (buf4, false, ch, obj, 0, TO_CHAR);
				return;
			}
		}
		else if (IS_WEARABLE (obj) && wants_off_size)
		{
			if (obj->size && obj->size != size)
			{
				act ("$p isn't that size.", false, ch, obj, 0, TO_CHAR);
				return;
			}
		}

		else if (IS_WEARABLE (obj) && obj->size &&
			obj->size != get_size (ch) && !regardless && obj->size != size)
		{
			act ("$p wouldn't fit you.", false, ch, obj, 0, TO_CHAR);
			act ("(End the buy command with ! if you really want it.)",
				false, ch, obj, 0, TO_CHAR);
			return;
		}
		/*
		if(GET_ITEM_TYPE(obj) == ITEM_RESOURCE && buy_count > 1)
		{
		send_to_char("You can only buy one load of resources at a time.\n", ch);
		do_whisper (keeper, buf, 83);
		return;
		}*/

		if (!keeper_makes (keeper, obj->nVirtual) && buy_count > obj->count)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I only have %d of that in stock at the moment.",
				buf2, obj->count);
			do_whisper (keeper, buf, 83);
			return;
		}
		else if (keeper_makes (keeper, obj->nVirtual) && buy_count > obj->count)
			obj->count = buy_count;

		if (buy_count < 1)
			buy_count = 1;
	}
	else
	{
		if (ch->delay_type != DEL_PURCHASE_ITEM || !ch->purchase_obj)
		{
			send_to_char ("There is no purchase in progress, I'm afraid.\n",
				ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->purchase_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		if ((obj = ch->purchase_obj) && (obj->in_room != keeper->shop->store_vnum) && (!keeper_makes (keeper, obj->nVirtual) && IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE)))
		{
			send_to_char ("That item is no longer available for purchase.\n",
				ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->purchase_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}
		else if (!obj)
		{
			send_to_char ("That item is no longer available for purchase.\n",
				ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->purchase_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		buy_count = ch->delay_info1;

		if (buy_count < 1)
			buy_count = 1;

		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->purchase_obj = NULL;
		ch->delay_ch = NULL;
	}

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, buy_count, true, false);

	if (IS_SET (ch->room->room_flags, LAWFUL) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s Those are illegal.  I can't sell them.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (GET_ITEM_TYPE (obj) == ITEM_NPC_OBJECT)
	{
		if (cmd != 2)
		{
			if (!*buf2)
			{
				send_to_char
					("You'll need to specify a name for your new NPC, e.g. \"buy horse Shadowfax\".\n",
					ch);
				return;
			}
		}
		else
		{
			sprintf (buf2, "%s", ch->delay_who);
			mem_free (ch->delay_who);
		}
		if (strlen (buf2) > 26)
		{
			send_to_char
				("The NPC's name must be 26 letters or fewer in length.\n", ch);
			return;
		}
		for (size_t i = 0; i < strlen (buf2); i++)
		{
			if (!isalpha (buf2[i]))
			{
				send_to_char ("Invalid characters in the proposed NPC name.\n",
					ch);
				return;
			}
		}
		sprintf (name, "%s", buf2);
	}

	if (cmd == 1)
	{
		/* passed by barter command to do_buy */

		name_to_ident (ch, buf);

		if (keepers_cost < 20)
		{
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
		{
			strcat (buf, " I won't haggle over a used piece of merchandse.");
			do_whisper (keeper, buf, 83);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if (nobarter_flag != -1 &&
			IS_SET (keeper->shop->nobuy_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
				"are fixed, take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (nobarter_flag != -1 && IS_SET (obj->econ_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle over the price "
				"of this item.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* Search for existing entry in keepers negotiations list */

		for (neg = keeper->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual && neg->true_if_buying)
				break;
		}

		if (neg)
		{
			if (neg->price_delta > 0)
			{
				strcat (buf, " No, no, I cannot afford to lower the price on "
					"that again.");
				do_whisper (keeper, buf, 83);
				return;
			}

			strcat (buf, " You're persistent, aren't you?  I said no, and I "
				"meant no.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* keeper will be reluctant to sell to foreigners */
		language_barrier =
			(keeper->skills[ch->speaks] <
			15) ? (15 - keeper->skills[ch->speaks]) : (0);
		keeper_success =
			combat_roll (MIN
			(95, keeper->skills[SKILL_HAGGLE] + language_barrier), 0, 0);
		ch_success =
			combat_roll (MAX (5, ch->skills[SKILL_HAGGLE] - language_barrier), 0, 0);

		if (ch_success == SUC_CS && keeper_success == SUC_MS)
			discount = 3;
		else if (ch_success == SUC_CS && keeper_success == SUC_MF)
			discount = 6;
		else if (ch_success == SUC_CS && keeper_success == SUC_CF)
			discount = 9;

		else if (ch_success == SUC_MS && keeper_success == SUC_MS)
			discount = 3;
		else if (ch_success == SUC_MS && keeper_success == SUC_MF)
			discount = 6;
		else if (ch_success == SUC_MS && keeper_success == SUC_CF)
			discount = 9;

		else if (ch_success == SUC_MF && keeper_success == SUC_MS)
			discount = 0;
		else if (ch_success == SUC_MF && keeper_success == SUC_MF)
			discount = 0;
		else if (ch_success == SUC_MF && keeper_success == SUC_CF)
			discount = 3;

		else
			discount = 0;		/* A CF by ch */

		discount = -1 * discount; //changing to a lower price
		neg = (NEGOTIATION_DATA *) alloc (sizeof (NEGOTIATION_DATA), 40);
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->nVirtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60;	/* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 1;
		neg->next = keeper->shop->negotiations;
		keeper->shop->negotiations = neg;

		if (discount == 0)
		{
			strcat (buf, " The price is as stated.  Take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		else if (discount == 5)
			strcat (buf, " I like your face, you seem an honest and "
			"trustworthy sort.  You can have it for ");
		else if (discount == 10)
			strcat (buf, " It's just not my day, is it?  All right, you win, "
			"I'll sell at your price.  It's yours for ");
		else
			strcat (buf, " My word!  I need to learn to count!  At this rate, "
			"I'll be out of business in a week.  Here, here, take "
			"your ill-gotten gain and begone.  Take it away for ");

		keepers_cost = keepers_cost * (100 + discount) / 100;

		keepers_cost = (int) keepers_cost;

		sprintf (buf + strlen (buf), "%d credit%s", (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");

		strcat (buf, ".");

		do_whisper (keeper, buf, 83);

		return;
	}

	if ((keepers_cost > 0 && keepers_cost < 1)
		|| (keepers_cost == 0 && obj->obj_flags.set_cost == 0))
		keepers_cost = 1;

	keepers_cost = (int) keepers_cost;

	if (cmd != 2)
	{
		orig_count = obj->count;
		obj->count = buy_count;
		if (obj->in_room != keeper->shop->store_vnum)
			obj->in_room = keeper->shop->store_vnum;

		if (variable && IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) && keeper_makes (keeper, obj->nVirtual))
		{

			break_delay(ch);
			tobj = load_colored_object(obj->nVirtual, xcolor[0], xcolor[1], xcolor[2], xcolor[3], xcolor[4], xcolor[5], xcolor[6], xcolor[7], xcolor[8], xcolor[9]);

			if (buy_count)
				tobj->count = buy_count;
			else
				buy_count = 1;

			if (is_clothing_item(tobj->nVirtual && (!tobj->o.od.value[4] || !tobj->o.od.value[5])))
				clothing_qualitifier(tobj, NULL);
			keepers_cost = calculate_sale_price (tobj, keeper, ch, buy_count, true, false);
			keepers_cost = keepers_cost * (100 + discount) / 100;
			keepers_cost = (int) keepers_cost;
			sprintf (buf, "You have opted to purchase #2%s#0, for a total of %d credit%s. To confirm, please use the ACCEPT command.\n", obj_short_desc (tobj), (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");
			sprintf(buf + strlen(buf), "\nThe long description of this object will be: #2%s#0\n", tobj->description);
			sprintf(buf + strlen(buf), "\nThe full description of this object will be:");
			sprintf(buf + strlen(buf), "\n#2%s#0\n", tobj->full_description);

			//char *p;
			//reformat_string (buf, &p);
			//sprintf (buf, "%s", p);
			//mem_free (p); // char*
			tobj->obj_flags.extra_flags |= ITEM_TIMER;
			tobj->obj_timer = 2; // We will kill this in two tics if no one claims it.

			ch->purchase_obj = tobj;
			for (int ind = 0; ind <= 9; ind++)
			{
				mem_free(xcolor[ind]);
			}
		}
		else
		{
			sprintf (buf, "You have opted to purchase #2%s#0, for a total of %d credit%s. To confirm, please use the ACCEPT command.", obj_short_desc (obj), (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");
			ch->purchase_obj = obj;
		}


		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		ch->delay_type = DEL_PURCHASE_ITEM;
		ch->delay_info1 = buy_count;
		if (ch->delay_info1 < 1)
			ch->delay_info1 = 1;
		ch->delay_ch = keeper;
		obj->count = orig_count;
		if (GET_ITEM_TYPE (obj) == ITEM_NPC_OBJECT)
			ch->delay_who = str_dup (name);
		return;
	}

	if (!can_subtract_money (ch, (int) keepers_cost, keeper->mob->currency_type))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, MISSING_CASH2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (obj->morphTime)
	{
		obj->clock = vtoo (obj->nVirtual)->clock;
		obj->morphTime = time (0) + obj->clock * 15 * 60;
	}

	tobj = obj;
	obj_from_room (&tobj, buy_count);

	int port = engine.get_port ();

	mysql_safe_query
		("INSERT INTO %s.receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'sold','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
		(engine.get_config ("player_log_db")).c_str (),
		keeper->mob->vnum, GET_NAME (ch), char_short (ch),
		tobj->nVirtual, tobj->short_description, tobj->count, keepers_cost,
		keeper->in_room, time_info.year, time_info.month + 1,
		time_info.day + 1, time_info.hour, port);

	if (keeper_makes (keeper, obj->nVirtual)
		&& !get_obj_in_list_num (obj->nVirtual,
		vnum_to_room (keeper->shop->store_vnum)->contents))
	{
		tobj->count = buy_count;
		delivery_cost = calculate_sale_price (obj, keeper, NULL, 1, true, true);
		if (keeper_has_money (keeper, (int) delivery_cost))
		{
			OBJ_DATA *new_obj = NULL;
			obj_to_room ((new_obj = load_object (obj->nVirtual)), keeper->shop->store_vnum);

			fluid_object(new_obj);

			subtract_keeper_money (keeper, (int) delivery_cost);
			mysql_safe_query
				("INSERT INTO %s.receipts "
				"(time, shopkeep, transaction, who, customer, vnum, "
				"item, qty, cost, room, gametime, port) "
				"VALUES (NOW()+1,%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
				(engine.get_config ("player_log_db")).c_str (),
				keeper->mob->vnum, "vNPC Merchant",
				"an honest-looking merchant", obj->nVirtual,
				obj->short_description, 1, delivery_cost, keeper->in_room,
				time_info.year, time_info.month + 1, time_info.day + 1,
				time_info.hour, port);
		}
	}

	if (keeper_makes (keeper, tobj->nVirtual) && IS_SET(tobj->obj_flags.extra_flags, ITEM_VARIABLE) && ch->purchase_obj && (ch->purchase_obj->nVirtual == tobj->nVirtual))
	{
		tobj = ch->purchase_obj;
		ch->purchase_obj = NULL;
	}

	if (IS_SET(tobj->obj_flags.extra_flags, ITEM_TIMER))
	{
		tobj->obj_flags.extra_flags &= ~ITEM_TIMER;
		tobj->obj_timer = 0; // Remove the timer.
	}

	act ("$n buys $p.", false, ch, tobj, 0, TO_ROOM);
	act ("$N sells you $p.", false, ch, tobj, keeper, TO_CHAR);
	act ("You sell $N $p.", false, keeper, tobj, ch, TO_CHAR);

	subtract_money (ch, (int) keepers_cost, keeper->mob->currency_type);

	name_to_ident (ch, buf2);

	if (ch->room->zone == 5 || ch->room->zone == 6)
	{
		sprintf (buf,
			"%s You're lucky I gave it to you for %d credit%s, maggot.",
			buf2, (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");
	}
	else
	{
		sprintf (buf, "%s A veritable steal at ", buf2);
		sprintf (buf + strlen (buf),
			"%d credit%s! Enjoy it, my friend.", (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");
	}

	do_whisper (keeper, buf, 83);

	money_to_storeroom (keeper, (int) keepers_cost);

	if (GET_ITEM_TYPE (tobj) == ITEM_NPC_OBJECT)
	{
		*name = toupper (*name);
		if (tobj->o.od.value[0] == 0
			|| !(horse = load_mobile (tobj->o.od.value[0])))
		{
			send_to_char
				("There seems to be a problem. Please inform the staff.\n", ch);
			return;
		}
		send_to_room ("\n", keeper->in_room);
		sprintf (buf, "%s#0 is released into your custody.",
			char_short (horse));
		*buf = toupper (*buf);
		sprintf (buf2, "#5%s", buf);
		act (buf2, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "%s#0 is released into #5%s#0's custody.",
			char_short (horse), char_short (ch));
		*buf = toupper (*buf);
		sprintf (buf2, "#5%s", buf);
		act (buf2, false, ch, 0, 0, TO_NOTVICT | _ACT_FORMAT);
		char_to_room (horse, ch->in_room);
		horse->act |= ACT_STAYPUT;
		sprintf (buf, "%s %s", horse->name, name);
		mem_free (horse->name);
		horse->name = str_dup (buf);
		if (get_clan (ch, "hules", &flags))
		{
			add_clan (horse, "hules", CLAN_MEMBER);
		}
		if (IS_SET (horse->act, ACT_MOUNT))
			hitch_char (ch, horse);
		if (!IS_NPC (ch))
		{
			horse->mob->owner = str_dup (ch->tname);
			save_char (ch, true);
		}
		return;
	}

	if (GET_ITEM_TYPE (obj) == ITEM_LIGHT)
	{
		tobj->o.od.value[1] = obj->o.od.value[1];
	}

	if (IS_WEARABLE (tobj))
	{
		if (size != -1)
			tobj->size = size;
		else if (regardless && tobj->size)
			;
		else
			tobj->size = get_size (ch);
	}

	if (GET_ITEM_TYPE (tobj) == ITEM_CONTAINER && tobj->o.od.value[2] > 0
		&& vtoo (tobj->o.od.value[2])
		&& GET_ITEM_TYPE (vtoo (tobj->o.od.value[2])) == ITEM_KEY)
	{
		obj = load_object (tobj->o.od.value[2]);
		obj->o.od.value[1] = tobj->coldload_id;
		obj_to_obj (obj, tobj);
		obj = load_object (tobj->o.od.value[2]);
		obj->o.od.value[1] = tobj->coldload_id;
		obj_to_obj (obj, tobj);
	}

	if (GET_ITEM_TYPE(tobj) == ITEM_RESOURCE)
	{
		one_argument (obj->name, buf2);
		sprintf (buf, "emote sets *%s down nearby, nodding to ~%s.", buf2, ch->tname);
		command_interpreter (keeper, buf);
		obj_to_room (tobj, keeper->in_room);
	}
	else if (ch->right_hand && ch->left_hand)
	{
		sprintf (buf,
			"%s Your hands seem to be full, so I'll just set this down for you to pick up when you've a chance.",
			ch->tname);
		send_to_char ("\n", ch);
		do_whisper (keeper, buf, 83);
		one_argument (obj->name, buf2);
		send_to_char ("\n", ch);
		sprintf (buf, "emote sets *%s down nearby, nodding to ~%s.", buf2,
			ch->tname);
		command_interpreter (keeper, buf);
		obj_to_room (tobj, keeper->in_room);
	}
	else
		obj_to_char (tobj, ch);


	if (neg)
		if (!neg->transactions++)
			;
	//skill_use (ch, SKILL_HAGGLE, 0);

}
int
	keeper_uses_currency_type (int currency_type, OBJ_DATA * obj)
{
	if (currency_type == CURRENCY_FOOD)
	{
		if (obj->nVirtual == 50093 || obj->nVirtual == 50092
			|| obj->nVirtual == 50091 || obj->nVirtual == 50090)
			return 1;
	}
	else if (currency_type == CURRENCY_PHOENIX)
	{
		if (obj->nVirtual == 14011 || obj->nVirtual == 14012
			|| obj->nVirtual == 14013 || obj->nVirtual == 14014
			|| obj->nVirtual == 14015 || obj->nVirtual == 14016 || obj->nVirtual == 14017)
			return 1;
	}
	return 0;
}

int
	trades_in (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	int i;
	bool block = true;

	if (!(obj->silver + obj->farthings))
		return 0;

	if (obj->obj_flags.type_flag == 0)
		return 0;

	if (GET_ITEM_TYPE (obj) == ITEM_MONEY
		&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
		return 0;

	if (IS_SET (keeper->room->room_flags, LAWFUL)
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
		return 0;

	//since fluids are no-take, this check has to be here
	//casuing some side effects so I am setting it back to no-buy
	if (GET_ITEM_TYPE (obj) == ITEM_FLUID)
		return 0;

	if (!IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
		return 0;

	for (i = 0; i < MAX_TRADES_IN; i++)
	{
		if (obj->obj_flags.type_flag == keeper->shop->trades_in[i])
			block = false;
	}

	if (block)
		return 0;

	if (keeper->shop->materials
		&& !IS_SET (keeper->shop->materials, GET_KEEPER_MATERIAL_TYPE (obj)))
		return 0;

	if (keeper->shop->buy_flags && !(obj->econ_flags & keeper->shop->buy_flags))
		return 0;

	if (keeper->shop->nobuy_flags
		&& (obj->econ_flags & keeper->shop->nobuy_flags))
		return 0;

	// Check any liquid inside the object for trades_in eligibility.

	if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON && obj->contains)
	{
		if (!trades_in (keeper, obj->contains))
			return 0;
	}

	return 1;
}


int
	keeper_has_money (CHAR_DATA * keeper, int cost)
{
	ROOM_DATA *store;
	OBJ_DATA *obj;
	int money = 0;

	if (!keeper->shop)
		return 0;
	if (!keeper->shop->store_vnum)
		return 0;
	if (!(store = vnum_to_room (keeper->shop->store_vnum)))
		return 0;

	if (!store->psave_loaded)
		load_save_room (store);

	if (store->contents && GET_ITEM_TYPE (store->contents) == ITEM_MONEY
		&& keeper_uses_currency_type (keeper->mob->currency_type,
		store->contents))
	{
		money = ((int) store->contents->farthings) * store->contents->count;
	}
	for (obj = store->contents; obj; obj = obj->next_content)
	{
		if (obj->next_content && GET_ITEM_TYPE (obj->next_content) == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type,
			obj->next_content))
		{
			money += ((int) obj->next_content->farthings) * obj->next_content->count;
		}
	}

	if (money < cost)
		return 0;
	else
		return money;
}

void
	keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money)
{
	OBJ_DATA *obj, *tobj;
	char buf[MAX_STRING_LENGTH];
	int denom[7] = {0,0,0,0,0,0,0};
	int location;
	bool money_found = false;

	for (location = 0; location < MAX_WEAR; location++)
	{
		if (!(tobj = get_equip (ch, location)))
			continue;
		if (GET_ITEM_TYPE (tobj) == ITEM_CONTAINER)
		{
			for (obj = tobj->contains; obj; obj = obj->next_content)
				if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
					money_found = true;
			if (money_found)
				break;
		}
	}

	if (!tobj)
	{
		for (location = 0; location < MAX_WEAR; location++)
		{
			if (!(tobj = get_equip (ch, location)))
				continue;
			if (GET_ITEM_TYPE (tobj) == ITEM_CONTAINER)
				break;
		}
	}

	if (keeper->mob->currency_type == CURRENCY_FOOD)
	{
		if (money / 240)
		{
			obj = load_object (50093);
			obj->count = money / 240;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[0] = money / 240;
			money %= 240;
		}

		if (money / 25)
		{
			obj = load_object (50092);
			obj->count = money / 25;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[1] = money / 25;
			money %= 25;
		}

		if (money / 5)
		{
			obj = load_object (50091);
			obj->count = money / 5;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[2] = money / 5;
			money %= 5;
		}

		if (money)
		{
			obj = load_object (50090);
			obj->count = money;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[3] = money;
		}

		send_to_char ("\n", ch);
		if (tobj)
			sprintf (buf,
			"$N gives you the following credits, which you tuck away in #2%s#0:",
			obj_short_desc (tobj));
		else
			sprintf (buf, "$N gives you the following credits:");

		act (buf, true, ch, 0, keeper, TO_CHAR | _ACT_FORMAT);

		*buf = '\0';

		if (denom[0])
			sprintf (buf + strlen (buf),
			"   #2%d condensed food ration%s#0\n", denom[0],
			denom[0] > 1 ? "s" : "");
		if (denom[1])
			sprintf (buf + strlen (buf), "   #2%d weighty food ration%s#0\n",
			denom[1], denom[1] > 1 ? "s" : "");
		if (denom[2])
			sprintf (buf + strlen (buf),
			"   #2%d packaged food ration%s#0\n", denom[2],
			denom[2] > 1 ? "s" : "");
		if (denom[3])
			sprintf (buf + strlen (buf), "   #2%d small, wrapped food ration%s#0\n",
			denom[3], denom[3] > 1 ? "s" : "");
	}
	else if (keeper->mob->currency_type == CURRENCY_PHOENIX)
	{
		if (money / 1000)
		{
			obj = load_object (14017);
			obj->count = money / 1000;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[0] = money / 1000;
			money %= 1000;
		}

		if (money / 100)
		{
			obj = load_object (14016);
			obj->count = money / 100;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[1] = money / 100;
			money %= 100;
		}

		if (money / 50)
		{
			obj = load_object (14015);
			obj->count = money / 50;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[2] = money / 50;
			money %= 50;
		}

		if (money / 20)
		{
			obj = load_object (14014);
			obj->count = money / 20;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[3] = money / 20;
			money %= 20;
		}

		if (money / 10)
		{
			obj = load_object (14013);
			obj->count = money / 10;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[4] = money / 10;
			money %= 10;
		}

		if (money / 5)
		{
			obj = load_object (14012);
			obj->count = money / 5;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[5] = money / 5;
			money %= 5;
		}

		if (money)
		{
			obj = load_object (14011);
			obj->count = money;
			if (tobj)
				obj_to_obj (obj, tobj);
			else
				obj_to_char (obj, ch);
			denom[6] = money;
		}

		send_to_char ("\n", ch);
		if (tobj)
			sprintf (buf,
			"$N gives you the following chips, which you tuck away in #2%s#0:",
			obj_short_desc (tobj));
		else
			sprintf (buf, "$N gives you the following chips:");

		act (buf, true, ch, 0, keeper, TO_CHAR | _ACT_FORMAT);

		*buf = '\0';

		if (denom[0])
			sprintf (buf + strlen (buf), "   #2%d gold-flecked circular chip%s#0\n",
			denom[0], denom[0] > 1 ? "s" : "");
		if (denom[1])
			sprintf (buf + strlen (buf), "   #2%d black circular chip%s#0\n",
			denom[1], denom[1] > 1 ? "s" : "");
		if (denom[2])
			sprintf (buf + strlen (buf), "   #2%d green circular chip%s#0\n",
			denom[2], denom[2] > 1 ? "s" : "");
		if (denom[3])
			sprintf (buf + strlen (buf), "   #2%d yellow circular chip%s#0\n",
			denom[3], denom[3] > 1 ? "s" : "");
		if (denom[4])
			sprintf (buf + strlen (buf), "   #2%d blue circular chip%s#0\n",
			denom[4], denom[4] > 1 ? "s" : "");
		if (denom[5])
			sprintf (buf + strlen (buf), "   #2%d red circular chip%s#0\n",
			denom[5], denom[5] > 1 ? "s" : "");
		if (denom[6])
			sprintf (buf + strlen (buf), "   #2%d white circular chip%s#0\n",
			denom[6], denom[6] > 1 ? "s" : "");
	}


	send_to_char (buf, ch);
}

void
	do_sell (CHAR_DATA * ch, char *argument, int cmd)
{
	int objs_in_storage;
	int sell_count = 1;
	int nobarter_flag;
	int language_barrier = 0;
	int keeper_success;
	int ch_success;
	int discount, same_obj = 0;
	float keepers_cost = 0;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	NEGOTIATION_DATA *neg;
	bool resource = false;

	argument = one_argument (argument, buf);

	room = ch->room;

	if (isdigit (*buf))
	{
		sell_count = atoi (buf);
		argument = one_argument (argument, buf);
		if (sell_count > MAX_INV_COUNT)
		{
			sprintf (buf2,
				"Sorry, but you can only sell up to %d items at a time.\n",
				MAX_INV_COUNT);
			send_to_char (buf2, ch);
			return;
		}
	}

	if (!*buf)
	{
		send_to_char ("Sell what?\n", ch);
		return;
	}

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);
	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!*buf)
		{
			act ("Sell what to $N?", true, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if (keeper == ch)
	{
		send_to_char ("You can't sell to yourself!\n", ch);
		return;
	}

	if (!keeper->shop || !IS_NPC (keeper))
	{
		act ("$N does not seem to be a shopkeeper.", true, ch, 0, keeper,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (GET_POS (keeper) <= POSITION_SLEEPING)
	{
		act ("$N is not conscious.", true, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (GET_POS (keeper) == POSITION_FIGHTING)
	{
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if (!keeper->shop ||
		(keeper->shop->shop_vnum && keeper->shop->shop_vnum != ch->in_room))
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(room = vnum_to_room (keeper->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (IS_SET (keeper->act, ACT_NOBUY))
	{
		do_say (keeper, "Sorry, but I don't deal in second-hand merchandise.",
			0);
		return;
	}

	if (!room->psave_loaded)
		load_save_room (room);

	if (!GET_TRUST (ch) && !CAN_SEE (keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	argument = one_argument (argument, buf2);

	if (!str_cmp (buf2, "from") || !str_cmp (buf2, "on"))
	{
		argument = one_argument (argument, buf2);
		if (!(tch = get_char_room_vis (ch, buf2)))
		{
			act ("Sell what from who?", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (!IS_SET(tch->affected_by, AFF_TRANSPORTING))
		{
			act ("$N isn't carrying anything that can be sold like that.", true, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (!(obj = get_obj_in_list_vis (ch, buf, tch->equip)) &&
			!(obj = get_obj_in_list_vis (ch, buf, tch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, tch->left_hand)))
		{
			act ("You don't see that on or being carried by $N.", true, ch, 0, tch, TO_CHAR);
			return;
		}


		if (tch->following != ch || GET_ITEM_TYPE(obj) != ITEM_RESOURCE)
		{
			act ("You can't take $p from $N.", true, ch, obj, tch, TO_CHAR);
			return;
		}

		resource = true;
	}

	if (!resource)
	{
		if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM2);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	if (obj->count < sell_count)
		sell_count = obj->count;

	if (sell_count < 1)
		sell_count = 1;

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, sell_count, true, true);

	if (IS_SET (keeper->act, ACT_BULKTRADER))
	{
		if (GET_ITEM_TYPE(obj) != ITEM_RESOURCE)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I only deal in bulk goods.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	if (!trades_in (keeper, obj))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ((GET_ITEM_TYPE (obj) == ITEM_LIGHT && !obj->o.od.value[1]))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (IS_SET (ch->room->room_flags, LAWFUL) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s I can't buy that.  It's illegal to possess.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (vnum_to_room (keeper->shop->store_vnum))
	{

		objs_in_storage = 0;

		for (tobj = vnum_to_room (keeper->shop->store_vnum)->contents;
			tobj; tobj = tobj->next_content)
		{
			if (GET_ITEM_TYPE (tobj) != ITEM_MONEY
				&& IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD))
				objs_in_storage++;
			if (tobj->nVirtual == obj->nVirtual)
				same_obj += tobj->count;
		}

		if (((same_obj + sell_count) > MAX_INV_COUNT)
			&& (GET_ITEM_TYPE (obj) != ITEM_TOSSABLE  && GET_ITEM_TYPE (obj) != ITEM_RESOURCE))
		{
			name_to_ident (ch, buf2);
			sprintf (buf,
				"%s I have quite enough of that for now, thank you; try back again later.",
				buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		if (objs_in_storage > 125)
		{
			name_to_ident (ch, buf2);
			sprintf (buf,
				"%s I have too much stuff as it is.  Perhaps you'd like to purchase something instead?",
				buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	if (cmd == 1)
	{
		/* passed by barter command to do_sell */

		if (sell_count > 1)
		{
			send_to_char ("You can only barter for one item at a time.\n", ch);
			return;
		}

		name_to_ident (ch, buf);

		if (keepers_cost < 20)
		{
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 83);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if (nobarter_flag != -1 &&
			IS_SET (keeper->shop->nobuy_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
				"are fixed, take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
		{
			strcat (buf, " I won't haggle over a used piece of merchandse.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (nobarter_flag != -1 && IS_SET (obj->econ_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle over the price "
				"of this item.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* Search for existing entry in keepers negotiations list */

		for (neg = keeper->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
				break;
		}

		if (neg)
		{
			if (neg->price_delta > 0)
			{
				strcat (buf, " No, no, I will not pay any higher a price.");
				do_whisper (keeper, buf, 83);
				return;
			}

			strcat (buf, " Listen, as much as I like you, I simply cannot "
				"offer you what you're asking.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* keeper will be reluctant to sell to foreigners */
		language_barrier =
			(keeper->skills[ch->speaks] <
			15) ? (15 - keeper->skills[ch->speaks]) : (0);
		keeper_success =
			combat_roll (MIN
			(95, keeper->skills[SKILL_HAGGLE] + language_barrier), 0, 0);
		ch_success =
			combat_roll (MAX (5, ch->skills[SKILL_HAGGLE] - language_barrier), 0, 0);

		if (ch_success == SUC_CS && keeper_success == SUC_MS)
			discount = 5;
		else if (ch_success == SUC_CS && keeper_success == SUC_MF)
			discount = 10;
		else if (ch_success == SUC_CS && keeper_success == SUC_CF)
			discount = 15;

		else if (ch_success == SUC_MS && keeper_success == SUC_MS)
			discount = 0;
		else if (ch_success == SUC_MS && keeper_success == SUC_MF)
			discount = 5;
		else if (ch_success == SUC_MS && keeper_success == SUC_CF)
			discount = 10;

		else if (ch_success == SUC_MF && keeper_success == SUC_MS)
			discount = 0;
		else if (ch_success == SUC_MF && keeper_success == SUC_MF)
			discount = 0;
		else if (ch_success == SUC_MF && keeper_success == SUC_CF)
			discount = 5;

		else
			discount = 0;		/* A CF by ch */

		neg = (NEGOTIATION_DATA *) alloc (sizeof (NEGOTIATION_DATA), 40);
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->nVirtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60;	/* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 0;
		neg->next = keeper->shop->negotiations;
		keeper->shop->negotiations = neg;

		if (discount == 0)
		{
			strcat (buf, " Sorry, but it's just not worth more than my "
				"initial offer.");
			do_whisper (keeper, buf, 83);
			return;
		}

		else if (discount == 5)
			strcat (buf, " I've been looking for these.  It's a pleasure doing "
			"business with you.  I'll pay you ");
		else if (discount == 10)
			strcat (buf, " Perhaps if I go back to bed now, I can salvage some "
			"small part of my self respect.  I'll pay you ");
		else
			strcat (buf, " It is a dark day.  I'll have to sell my home and "
			"business just to recoup what I've lost this day."
			"  I'll give you ");

		keepers_cost = keepers_cost * (100 + discount) / 100;

		sprintf (buf + strlen (buf), "%d credit%s", (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");

		strcat (buf, ".");

		do_whisper (keeper, buf, 83);

		return;
	} //end  if (cmd == 1) bartering

	keepers_cost = (int) keepers_cost;

	/* Look up negotiations for this ch/obj on keeper */

	for (neg = keeper->shop->negotiations; neg; neg = neg->next)
	{
		if (neg->ch_coldload_id == ch->coldload_id &&
			neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
			break;
	}

	keepers_cost = (int) keepers_cost;

	if (keepers_cost < 1)
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s Bah, that isn't even worth my time!", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!keeper_has_money (keeper, (int) keepers_cost))
	{

		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, MISSING_CASH1);

		do_whisper (keeper, buf, 83);

		if (!IS_SET (keeper->act, ACT_PCOWNED) ||
			!vnum_to_room (keeper->shop->store_vnum))
			return; /// \todo Flagged as unreachable

		return;
	}

	act ("$n sells $p.", false, ch, obj, 0, TO_ROOM);
	act ("You sell $p.", false, ch, obj, 0, TO_CHAR);
	send_to_char ("\n", ch);

	name_to_ident (ch, buf2);
	sprintf (buf, "%s Here's the amount we've agreed upon.", buf2);

	/* Pay customer */

	do_whisper (keeper, buf, 83);

	subtract_keeper_money (keeper, (int) keepers_cost);
	keeper_money_to_char (keeper, ch, (int) keepers_cost);

	obj_from_char (&obj, sell_count);
	int port = engine.get_port ();

	mysql_safe_query
		("INSERT INTO %s.receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
		(engine.get_config ("player_log_db")).c_str (),
		keeper->mob->vnum, GET_NAME (ch), char_short (ch),
		obj->nVirtual, obj->short_description, obj->count, keepers_cost,
		keeper->in_room, time_info.year, time_info.month + 1,
		time_info.day + 1, time_info.hour, port);

	money_from_char_to_room (keeper, keeper->shop->store_vnum);

	if (keeper_makes (keeper, obj->nVirtual) && GET_ITEM_TYPE(obj) != ITEM_RESOURCE)
	{
		extract_obj (obj);
	}
	else
	{
		if (GET_ITEM_TYPE (obj) != ITEM_RESOURCE)
			obj->obj_flags.extra_flags |= ITEM_PC_SOLD;
		obj->sold_at = (int) time (0);
		obj->sold_by = ch->coldload_id;
		obj_to_room (obj, keeper->shop->store_vnum);
	}

	if (neg)
		if (!neg->transactions++)
			;
	//skill_use (ch, SKILL_HAGGLE, 0);
}

void
	do_value (CHAR_DATA * ch, char *argument, int cmd)
{
	float keepers_cost = 0;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	ROOM_DATA *room;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Appraise what?\n", ch);
		return;
	}

	room = ch->room;

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!*buf)
		{
			act ("Have $N appraise what?", true, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if (!keeper->shop)
	{
		act ("$N does not seem to be a shopkeeper.", true, ch, 0, keeper,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (GET_POS (keeper) <= POSITION_SLEEPING)
	{
		act ("$N is not conscious.", true, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (GET_POS (keeper) == POSITION_FIGHTING)
	{
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!trades_in (keeper, obj))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ((keepers_cost == 0 && GET_ITEM_TYPE (obj) == ITEM_MONEY))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!GET_TRUST (ch) && !CAN_SEE (keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, obj->count, true, true);

	name_to_ident (ch, buf2);

	*buf3 = '\0';

	sprintf (buf3, "%d credit%s", (int) keepers_cost,
		(int) keepers_cost > 1 ? "s" : "");

	keepers_cost = (int) keepers_cost;

	if (!keepers_cost)
	{
		if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE)
		{
			sprintf (buf, "%s I do not deal in those sort of resources.", buf2);
		}
		else
		{
			sprintf (buf, "%s I'm afraid that isn't even worth my time...", buf2);
		}
	}
	else
		sprintf (buf, "%s I'd buy %s for... %s.", buf2,
		obj->count > 1 ? "those" : "that", buf3);

	do_whisper (keeper, buf, 83);
}

void
	do_exchange (CHAR_DATA * ch, char *argument, int cmd)
{
	int count;
	CHAR_DATA *keeper;
	OBJ_DATA *coins;
	OBJ_DATA *new_coins;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?' || !isdigit (*buf))
	{
		send_to_char ("> exchange 10 silver      Gives you 40 farthings\n", ch);
		send_to_char ("> exchange 24 farthings   Gives you 6 pennies\n", ch);
		return;
	}

	count = atoi (buf);

	if (!count)
		return;

	argument = one_argument (argument, buf);

	if (!(coins = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(coins = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		send_to_char ("You don't have those.\n", ch);
		return;
	}

	if (coins->nVirtual != VNUM_PENNY && coins->nVirtual != VNUM_FARTHING)
	{
		send_to_char ("You can only exchange pennies or farthings.\n", ch);
		return;
	}

	if (coins->count < count)
	{
		send_to_char ("You don't have that many credits to exchange.\n", ch);
		return;
	}

	for (keeper = ch->room->people; keeper; keeper = keeper->next_in_room)
		if (keeper != ch && IS_SET (keeper->flags, FLAG_KEEPER))
			break;

	if (!keeper)
	{
		send_to_char
			("Sorry, there is no shopkeeper here to give you change.\n", ch);
		return;
	}

	if (coins->nVirtual == VNUM_PENNY)
	{
		new_coins = load_object (VNUM_FARTHING);
		new_coins->count = count * 4;
		name_money (new_coins);
		obj_to_char (new_coins, ch);

		if (count == coins->count)
			extract_obj (coins);
		else
			coins->count -= count;

		sprintf (buf, "$N takes your %d penn%s and gives you %d farthings.",
			count, count > 1 ? "ies" : "y", count * 4);
		act (buf, false, ch, 0, keeper, TO_CHAR);
		act ("$N makes change for $n.", false, ch, 0, keeper, TO_NOTVICT);
		return;
	}

	/* Making change for farthings */

	count = count - (count % 4);	/* Round the farthings to pennies */

	if (!count)
	{
		act ("$N can't make change for less than 4 farthings.",
			false, ch, 0, keeper, TO_CHAR);
		return;
	}

	new_coins = load_object (VNUM_PENNY);
	new_coins->count = count / 4;
	name_money (new_coins);
	obj_to_char (new_coins, ch);

	if (count == coins->count)
		extract_obj (coins);
	else
		coins->count -= count;

	sprintf (buf, "$N takes your %d farthings and gives you %d penn%s.",
		count, count / 4, (count / 4) > 1 ? "ies" : "y");
	act (buf, false, ch, 0, keeper, TO_CHAR);
	act ("$N makes change for $n.", false, ch, 0, keeper, TO_NOTVICT);
}

void
	do_barter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	if (!real_skill (ch, SKILL_HAGGLE))
	{
		send_to_char
			("You're not convincing enough to barter, unfortunately.\n", ch);
		return;
	}
	argument = one_argument (argument, buf);

	if (is_abbrev (buf, "buy"))
		do_buy (ch, argument, 1);
	else if (is_abbrev (buf, "sell"))
		do_sell (ch, argument, 1);
	else
		send_to_char ("barter sell ITEM    or\n barter buy ITEM\n", ch);
}

/*
TODO: this command needs some work. some old code was never finished and the new code is different
> receipts for [<day>] [<month>] [<year>]
> receipts to <person>
> receipts from <object>
-------
> receipts  [summary] [r <rvnum>]

*/
void
	do_receipts (CHAR_DATA * ch, char *argument, int cmd)
{
	short last_day = -1, last_month = -1, last_year = -1, nArgs = 0, nSeekDay =
		0, nSeekMonth = 0, nSeekYear = 0;
    bool hassome = false;
	int nAmtSold = 0, nAmtBought = 0, nTAmtSold = 0, nTAmtBought = 0;
	long day = -1, month = -1, year = -1;
	char *ptrFor = NULL, *ptrFrom = NULL, *ptrTo = NULL;
	CHAR_DATA *keeper = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char args[3][AVG_STRING_LENGTH / 3] = { "", "", "" };
	char buf[MAX_STRING_LENGTH] = "";
	bool sumchk = false;
	//int shopnum = 0;

	/****** begin future options test statments ***/
	if (argument && argument[0])
	{
		if (!strncmp (argument, "for ", 4))
		{
			ptrFor = argument + 4;
		}
		else if ((ptrFor = strstr (argument, " for ")))
		{
			ptrFor = ptrFor + 5;
		}

		if (ptrFor)
		{
			nArgs = sscanf (ptrFor, "%s %s %s", args[0], args[1], args[2]);
			if (nArgs == 3	/* DD MONTH YYYY */
				&& isdigit (args[0][0]) && isdigit (args[2][0])
				&& (nSeekMonth = index_lookup (month_lkup, args[1])))
			{
				nSeekYear = strtol (args[2], NULL, 10);
				if (nSeekYear < time_info.year - 1
					|| nSeekYear > time_info.year)
				{
					nSeekYear = 0;
				}
				if (nSeekMonth < 1 || nSeekMonth > 12)
				{
					nSeekMonth = 0;
				}
				else if (!nSeekYear)
				{
					nSeekYear =
						(nSeekMonth <=
						time_info.month) ? time_info.year : time_info.year - 1;
				}
				nSeekDay = strtol (args[0], NULL, 10);
				if (nSeekDay < 1 || nSeekDay > 31)
				{
					nSeekDay = 0;
				}
				else if (!nSeekMonth)
				{
					nSeekMonth = (nSeekDay <= time_info.day)
						? time_info.month
						: (((time_info.month - 1) >= 0)
						? (time_info.month - 1) : 11);
				}
			}//if (nArgs == 3

			else if (nArgs == 2	/* MONTH YYYY */
				&& (nSeekMonth = index_lookup (month_lkup, args[0]))
				&& isdigit (args[1][0]))
			{
			}

			else if (nArgs == 2	/* DD MONTH */
				&& isdigit (args[0][0])
				&& (nSeekMonth = index_lookup (month_lkup, args[1])))
			{
			}
		}// if (ptrFor)

		if (!strncmp (argument, "from ", 5))
		{
			ptrFrom = argument;
		}

		else if ((ptrFrom = strstr (argument, " from ")))
		{
			ptrFrom = ptrFrom + 6;
		}

		if (!strncmp (argument, "to ", 3))
		{
			ptrTo = argument;
		}
		else if ((ptrTo = strstr (argument, " to ")))
		{
			ptrTo = ptrTo + 4;
		}

		if (!strncmp (argument, "summary", 7))
		{
			sumchk = true;
		}
	}// if (argument && argument[0])



	if (IS_NPC (ch) && ch->shop)
	{
		keeper = ch;
	}
	else
	{
		for (keeper = character_list; keeper; keeper = keeper->next)
		{
			if (IS_NPC (keeper) && keeper->shop
				&& keeper->shop->store_vnum == ch->in_room)
				break;
		}
	}
	if (keeper == NULL)
	{
		send_to_char ("You do not see a book of receipts here.\n", ch);
		return;
	}

	/* Detail */
	int port = engine.get_port ();

	mysql_safe_query
		("SELECT time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port, "
		"EXTRACT(YEAR FROM gametime) as year, "
		"EXTRACT(MONTH FROM gametime) as month, "
		"EXTRACT(DAY FROM gametime) as day "
		"FROM %s.receipts "
		"WHERE shopkeep = '%d' AND port = '%d' "
		"ORDER BY time DESC;",
		(engine.get_config ("player_log_db")).c_str (),
		keeper->mob->vnum, port);

	if ((result = mysql_store_result (database)) == NULL)
	{
		send_to_gods ((char *) mysql_error (database));
		send_to_char ("The book of receipts is unavailable at the moment.\n",
			ch);
		return;
	}

	send_to_char ("Examining a book of receipts:\n", ch);
	while ((row = mysql_fetch_row (result)) != NULL)
	{

		day = strtol (row[14], NULL, 10);
		month = strtol (row[13], NULL, 10) - 1;
		year = strtol (row[12], NULL, 10);
		if (day != last_day || month != last_month || year != last_year) 
		{

			if (last_day > 0 && month != last_month)
			{
				sprintf (buf + strlen (buf),
					"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n",
					month_short_name[(int) last_month], (int) last_year,
					nAmtSold, nAmtBought);
				/* send_to_char ( buf, ch ); */
				nTAmtBought += nAmtBought;
				nTAmtSold += nAmtSold;
				nAmtBought = 0;
				nAmtSold = 0;
			}

			if (!sumchk)
			{
				sprintf (buf + strlen (buf),
					"\nOn #6%s %d#0:\n\n",
					short_time_string(day, month), (int) year);
				/* send_to_char ( buf, ch ); */
			}

            hassome = true;
            
			last_day = day;
			last_month = month;
			last_year = year;
		}

		if (strcmp (row[2], "sold") == 0)
		{
			nAmtSold += strtol (row[8], NULL, 10);
		}
		else if (strcmp (row[2], "bought") == 0)
		{
			nAmtBought += strtol (row[8], NULL, 10);
		}

		row[2][0] = toupper (row[2][0]);
		if (!sumchk)
		{
			sprintf (buf + strlen (buf),
				"%s #2%s#0 of #2%s#0 %s #5%s#0 for #2%s cp#0.\n",
				row[2], row[7], row[6], (row[2][0] == 's') ? "to" : "from",
				(IS_NPC (ch) || IS_MORTAL (ch)) ? row[4] : row[3], row[8]);
			/* send_to_char ( buf, ch ); */
		}

		if (strlen (buf) > MAX_STRING_LENGTH - 512)
		{
			strcat (buf,
				"\n#1There were more sales than could be displayed.#0\n\n");
			break;
		}
	}

	mysql_free_result (result);

	if (hassome)
	{
		if ((nTAmtBought + nTAmtSold == 0) && (nAmtSold + nAmtBought > 0))
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n"
				"    Current credits on hand: #2%d cp#0.\n",
				month_short_name[(int) last_month], (int) last_year,
				nAmtSold, nAmtBought, keeper_has_money (keeper, 0));
		}
		else
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n"
				"    Total for period:    Sales #2%d cp#0, Purchases #2%d cp#0.\n"
				"    Current credits on hand:  #2%d cp#0.\n",
				month_short_name[(int) last_month], (int) last_year,
				nAmtSold, nAmtBought, nTAmtSold, nTAmtBought,
				keeper_has_money (keeper, 0));
		}
		page_string (ch->descr(), buf);
	}
}

int
	get_uniq_ticket (void)
{
	int tn = 1;
	int i;
	FILE *fp_ls;
	char buf[MAX_STRING_LENGTH];

	if (!(fp_ls = popen ("ls tickets", "r")))
	{
		system_log ("The ticket system is broken, get_uniq_ticket()", true);
		return -1;
	}

	/* The TICKET_DIR should be filled with files that have seven
	digit names (zero padded on the left).
	*/

	while (!feof (fp_ls))
	{

		if (!fgets (buf, 80, fp_ls))
			break;

		for (i = 0; i < 7; i++)
			if (!isdigit (buf[i]))
				continue;

		if (tn != atoi (buf))
			break;

		tn = atoi (buf) + 1;
	}

	pclose (fp_ls);

	return tn;
}

void unhitch_char (CHAR_DATA * ch, CHAR_DATA * hitch);
void
	do_stable (CHAR_DATA * ch, char *argument, int cmd)
{
	int ticket_num, i = 0;
	CHAR_DATA *animal = NULL;
	CHAR_DATA *new_hitch;
	CHAR_DATA *keeper;
	AFFECTED_TYPE *af;
	OBJ_DATA *ticket;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	bool paid_for = false;

	for (keeper = ch->room->people; keeper; keeper = keeper->next_in_room)
		if (keeper != ch &&
			keeper->mob &&
			IS_SET (keeper->flags, FLAG_KEEPER) &&
			IS_SET (keeper->flags, FLAG_OSTLER))
			break;

	if (!keeper)
	{
		send_to_char ("There is no ostler here.\n", ch);
		return;
	}

	if (!ch->hitchee || !is_he_here (ch, ch->hitchee, 0))
	{
		send_to_char ("You have no hitch.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if (!(animal = get_char_room_vis (ch, buf)))
		{
			send_to_char ("There isn't such an animal here.\n", ch);
			return;
		}

		if (animal != ch->hitchee)
		{
			act ("$N isn't hitched to you.", false, ch, 0, animal, TO_CHAR);
			return;
		}
	}

	/* Make sure mount isn't already mounted */

	if (ch->hitchee->mount && is_he_here (ch, ch->hitchee->mount, 0))
	{
		name_to_ident (ch->hitchee->mount, buf2);
		sprintf (buf, "tell %s Have #5%s#0 dismount your hitch first.",
			buf2, char_short (ch->hitchee->mount));
		command_interpreter (keeper, buf);
	}

	animal = ch->hitchee;

	i = MAGIC_STABLING_PAID;
	while ((af = get_affect (ch, i)))
	{
		if (af->a.spell.sn == animal->coldload_id)
		{
			paid_for = true;
			break;
		}
		i++;
		if (i > MAGIC_STABLING_LAST)
		{
			i = MAGIC_STABLING_PAID;
			break;
		}
	}

	if (!paid_for && !is_brother (ch, keeper)
		&& !can_subtract_money (ch, 20, keeper->mob->currency_type))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s You seem to be a bit short on credits right now.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!paid_for)
	{
		CREATE (af, AFFECTED_TYPE, 1);
		af->type = i;
		af->a.spell.sn = animal->coldload_id;
		af->a.spell.duration = 168;
		affect_to_char (ch, af);
		if (!is_brother (ch, keeper))
			subtract_money (ch, 20, keeper->mob->currency_type);
	}

	if ((ticket_num = get_uniq_ticket ()) == -1)
	{
		send_to_char ("OOC:  The ticket system is broken.  Sorry.\n", ch);
		return;
	}

	unhitch_char (ch, animal);

	/* Take reigns of hitches hitch, if chained */

	if (animal->hitchee && is_he_here (animal, animal->hitchee, 0))
	{

		new_hitch = animal->hitchee;
		unhitch_char (animal, new_hitch);

		if (hitch_char (ch, new_hitch))
			act ("You take the reigns of $N.", false, ch, 0, new_hitch, TO_CHAR);
	}

	ticket = load_object (VNUM_TICKET);

	if (!ticket)
	{
		send_to_char ("OOC:  Your hitch could not be saved.  Please report "
			"this as a problem.\n", ch);
		return;
	}

	ticket->o.ticket.ticket_num = ticket_num;
	ticket->o.ticket.keeper_vnum = keeper->mob->vnum;
	ticket->o.ticket.stable_date = (int) time (0);

	obj_to_char (ticket, ch);

	sprintf (buf, "The number %d is scrawled on this ostler's ticket.",
		ticket->o.ticket.ticket_num);
	sprintf (buf + strlen (buf),
		"\n\n#6OOC: To retrieve your mount, GIVE this ticket to the ostler\n"
		"     with whom you stabled it; be sure you don't lose this!#0");
	ticket->full_description = str_dup (buf);

	act ("$N gives you $p.", false, ch, ticket, keeper, TO_CHAR | _ACT_FORMAT);
	act ("You give $N $p.", false, keeper, ticket, ch, TO_CHAR | _ACT_FORMAT);
	act ("$N gives $n $p.", false, ch, ticket, keeper,
		TO_NOTVICT | _ACT_FORMAT);

	act ("$N leads $n to the stables.",
		false, animal, 0, keeper, TO_ROOM | _ACT_FORMAT);

	sprintf (buf, TICKET_DIR "/%07d", ticket_num);

	if (!(fp = fopen (buf, "w")))
	{
		perror (buf);
		system_log ("Unable to save ticketed mobile to file.", true);
		return;
	}

	save_mobile (animal, fp, "HITCH", 1);	/* Extracts the mobile */

	fclose (fp);
}

void
	unstable (CHAR_DATA * ch, OBJ_DATA * ticket, CHAR_DATA * keeper)
{
	CHAR_DATA *back_hitch = NULL;
	CHAR_DATA *animal;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char ticket_file[MAX_STRING_LENGTH];

	if (!keeper)
	{
		for (keeper = ch->room->people; keeper; keeper = keeper->next_in_room)
			if (keeper != ch &&
				keeper->mob &&
				IS_SET (keeper->flags, FLAG_KEEPER) &&
				IS_SET (keeper->flags, FLAG_OSTLER))
				break;

		if (!keeper)
		{
			send_to_char ("There is no ostler here.\n", ch);
			return;
		}
	}

	if (!keeper->mob)
	{
		/* Can happen if given specifically to a PC */
		send_to_char ("Only NPCs can be ostlers.\n", ch);
		return;
	}

	if (!CAN_SEE (keeper, ch))
	{
		act ("It appears that $N can't see you.",
			false, ch, 0, keeper, TO_CHAR);
		return;
	}

	if (ticket->o.ticket.keeper_vnum != keeper->mob->vnum)
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "tell %s Sorry, that's not one of my tickets.", buf2);
		command_interpreter (keeper, buf);
		return;
	}

	sprintf (ticket_file, TICKET_DIR "/%07d", ticket->o.ticket.ticket_num);

	if (stat (ticket_file, (struct stat *) buf2) == -1)
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "tell %s Yeah, that's my ticket, but I don't have "
			"anyting in that stall.", buf2);
		command_interpreter (keeper, buf);
		return;
	}

	if (is_he_here (ch, ch->hitchee, 0))
	{
		back_hitch = ch->hitchee;
		unhitch_char (ch, back_hitch);
	}

	animal = load_saved_mobiles (ch, ticket_file);
	if (ticket->o.od.value[0])
		offline_healing (animal, ticket->o.ticket.stable_date);

	if (!animal)
	{
		send_to_char ("OOC:  See an admin.  There is something wrong with that "
			"ticket.\n", ch);
		if (back_hitch)
			hitch_char (ch, back_hitch);

		return;
	}

	if (back_hitch && !ch->hitchee)
		hitch_char (ch, back_hitch);
	else if (back_hitch)
		hitch_char (ch->hitchee, back_hitch);

	extract_obj (ticket);

	save_char (ch, true);
	unlink (ticket_file);

	act ("$N trots $3 from the stables and hands you the reins.",
		false, ch, (OBJ_DATA *) animal, keeper, TO_CHAR | _ACT_FORMAT);
	act ("$N trots $3 from the stables and hands $n the reins.",
		false, ch, (OBJ_DATA *) animal, keeper, TO_NOTVICT | _ACT_FORMAT);
	act ("You trot $3 from the stables and hand $N the reins.",
		false, keeper, (OBJ_DATA *) animal, ch, TO_CHAR | _ACT_FORMAT);
}

void
	do_pay2 (CHAR_DATA * ch, char *argument, int cmd)
{
	int seen_tollkeeper = 0;
	int dir = 0;
	int max_toll;
	int num_crossers = 0;
	int tolls;
	int i;
	CHAR_DATA *crosser = NULL;
	CHAR_DATA *cross_list[50];
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *taf;
	char buf[MAX_STRING_LENGTH];

	/* pay
	pay [<char | direction>] # [all | char]
	*/

	/* Help:

	PAY

	Pay a toll to a tollkeeper.  If a tollkeeper prevents travel along a path,
	he can be paid a toll to allow passage.  You can pay a toll for yourself,
	for others, or for all those following you.

	> pay                 Shows the amount of toll being collected
	> pay #               Pay the toll as long as it is '#' or less.
	> pay <direction> #   Pay the toll to the tollkeeper who guards a
	specific direction, but not more than '#'.
	> pay <char> #        Pay the toll to the tollkeeper named <char>,
	but not more than '#'.

	> pay <char>        "all"    Pay <char> or character who guards <direction>
	<direction> # <char>   a max toll of '#' for <char> or everyone (all)
	following you.

	Examples:

	> pay
	> pay north 5 all
	> pay troll 10 guard

	For convenience, the SET command can be used to set AUTOTOLL.  If you follow
	another character, you will automatically pay your toll (if it hasn't been
	paid for you) up to a limit of your AUTOTOLL value.  Example of AUTOTOLL:

	> set autotoll 5     When following, automatically pay tolls up to 5.

	The TOLL command can be used to setup a toll crossing, but only in areas setup
	to support tolls.

	See also:  SET, TOLL

	*/

	if (!is_human (ch))
	{
		send_to_char ("Non-humans don't have to pay toll.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch->deleted)
				continue;

			if (tch == ch)
				continue;

			if (!(af = get_affect (tch, MAGIC_TOLL)))
				continue;

			if (af->a.toll.room_num != ch->in_room)
				continue;

			seen_tollkeeper = 1;

			sprintf (buf, "$n wants %d credit%s to cross %s.",
				af->a.toll.charge,
				af->a.toll.charge == 1 ? "" : "s", dirs[af->a.toll.dir]);
			act (buf, false, tch, 0, ch, TO_VICT);
		}

		if (!seen_tollkeeper)
			send_to_char ("There are no toll keepers here.\n", ch);

		return;
	}

	if (!just_a_number (buf))
	{
		/* pay ch | dir */

		if (!str_cmp (buf, "all"))
		{
		}

		else if ((tch = get_char_room_vis (ch, buf)))
		{
			/* pay <ch> */

			if (!(af = get_affect (tch, MAGIC_TOLL)))
			{
				act ("$N isn't collecting tolls.", false, ch, 0, tch, TO_CHAR);
				return;
			}

			dir = af->a.toll.dir;
		}

		else
		{

			if ((dir = index_lookup (dirs, buf)) == -1)
			{
				/* pay dir */
				send_to_char ("No such tollkeeper or direction.\n", ch);
				return;
			}

			/* Who is the char covering this direction? */

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{

				if (tch->deleted ||
					tch == ch ||
					!(af = get_affect (tch, MAGIC_TOLL)) ||
					af->a.toll.room_num != ch->in_room)
					continue;

				if (dir == af->a.toll.dir)
					break;
			}

			if (!tch)
			{
				send_to_char ("There is nobody there taking tolls.\n", ch);
				return;
			}
		}

		argument = one_argument (argument, buf);
	}

	if (!just_a_number (buf))
	{
		act ("How much are you willing to pay $N?", false, ch, 0, tch, TO_CHAR);
		return;
	}

	/* if pay #, then we still have to find someone to pay off */

	if (!tch)
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch->deleted ||
				tch == ch ||
				!(af = get_affect (tch, MAGIC_TOLL)) ||
				af->a.toll.room_num != ch->in_room)
				continue; /// \todo Flagges as unreachable

			break;
		}

		if (!tch)
		{
			send_to_char ("Nobody is collecting tolls here.\n", ch);
			return;
		}

		dir = af->a.toll.dir;
	}

	/* Collect the money */

	af = get_affect (tch, MAGIC_TOLL);

	if (!af)
	{
		send_to_char ("Something is wrong.  Please contact an admin.\n", ch);
		system_log ("No affect on tch. Expected a toll affect.", true);
		return;
	}

	if (!can_subtract_money (ch, af->a.toll.charge, tch->mob->currency_type))
	{
		send_to_char ("You don't have enough credits to pay the toll.\n", ch);
		return;
	}

	max_toll = atoi (buf);

	argument = one_argument (argument, buf);

	if (!*buf)
		cross_list[num_crossers++] = ch;

	else if (!str_cmp (buf, "all"))
	{
		/* Pay for followers */

		cross_list[num_crossers++] = ch;

		for (crosser = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (crosser == ch)
				continue;

			if (crosser->following != ch || !CAN_SEE (ch, crosser))
				continue;

			cross_list[num_crossers] = crosser;
			num_crossers++;
		}
	}

	else
	{
		while (*buf)
		{
			if (!(crosser = get_char_room_vis (ch, buf)))
			{
				send_to_char ("Pay toll for whom?\n", ch);
				return;
			}

			cross_list[num_crossers++] = crosser;

			argument = one_argument (argument, buf);
		}
	}

	/* How many tolls will ch pay? */

	tolls = 0;

	for (i = 0; i < num_crossers; i++)
	{

		if ((taf = get_affect (cross_list[i], MAGIC_TOLL_PAID)) &&
			taf->a.toll.dir == dir && taf->a.toll.room_num == ch->in_room)
			continue;

		tolls++;
	}

	if (tolls <= 0)
	{
		send_to_char ("Nobody needs a toll paid.\n", ch);
		return;
	}

	if (max_toll < af->a.toll.charge)
	{
		sprintf (buf, "$N wants a %d credit toll.", af->a.toll.charge);
		act (buf, false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (!can_subtract_money
		(ch, af->a.toll.charge * tolls, tch->mob->currency_type))
	{
		send_to_char ("You don't have enough credits to pay the tolls.\n", ch);
		return;
	}

	for (i = 0; i < num_crossers; i++)
	{

		if ((taf = get_affect (cross_list[i], MAGIC_TOLL_PAID)) &&
			taf->a.toll.dir == dir && taf->a.toll.room_num == ch->in_room)
		{

			if (num_crossers == 1)
			{
				if (cross_list[i] == ch)
					send_to_char ("You've already paid for your toll.\n", ch);
				else
					act ("$3 has already accepted a toll for $N.",
					false, ch, (OBJ_DATA *) tch, cross_list[i], TO_CHAR);
			}

			continue;

		}

		magic_add_affect (ch, MAGIC_TOLL_PAID, -1, 0, 0, 0, 0);

		taf = get_affect (ch, MAGIC_TOLL_PAID);

		taf->a.toll.dir = dir;
		taf->a.toll.room_num = ch->in_room;

		if (cross_list[i] != ch)
		{
			act ("$n pays $N a toll for $3.",
				true, ch, (OBJ_DATA *) cross_list[i], tch, TO_NOTVICT);
			act ("You pay a toll to $N for $3.",
				false, ch, (OBJ_DATA *) tch, cross_list[i], TO_CHAR);
		}

		else
		{
			sprintf (buf, "You pay $N a %d penny toll.", af->a.toll.charge);
			act (buf, false, ch, 0, tch, TO_CHAR);
			act ("$n pays $N a toll.", true, ch, 0, tch, TO_ROOM);
		}
	}

	subtract_money (ch, af->a.toll.charge * num_crossers,
		tch->mob->currency_type);

	act ("$n pays you a toll.", false, ch, 0, tch, TO_VICT);
}

int
	is_toll_paid (CHAR_DATA * ch, int dir)
{
	CHAR_DATA *collector;
	AFFECTED_TYPE *af;

	if (!is_human (ch))
		return 1;

	if (!(collector = toll_collector (ch->room, dir)))
		return 1;

	if (is_brother (collector, ch))
		return 1;

	if (!CAN_SEE (collector, ch))
		return 1;

	if (!(af = get_affect (ch, MAGIC_TOLL_PAID)))
		return 0;

	if (af->a.toll.dir == dir && af->a.toll.room_num == ch->in_room)
		return 1;

	return 0;
}

void
	do_pay (CHAR_DATA * ch, char *argument, int cmd)
{
	/* pay
	pay [<char | direction>] # [all | char]
	*/

	int max_pay = 0;
	int num_payees = 0;
	int dir = -1;
	int seen_tollkeeper = 0;
	int currency_type = 0;
	int i;
	CHAR_DATA *collector;
	CHAR_DATA *tch;
	CHAR_DATA *payees[50];
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *af_collector;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if (!(tch = toll_collector (ch->room, dir)) || tch == ch)
				continue;

			af = get_affect (tch, MAGIC_TOLL);

			seen_tollkeeper = 1;

			sprintf (buf, "$n wants %d penn%s to cross %s.",
				af->a.toll.charge,
				af->a.toll.charge == 1 ? "y" : "ies",
				dirs[af->a.toll.dir]);
			act (buf, false, tch, 0, ch, TO_VICT);
		}

		if (!seen_tollkeeper)
			send_to_char ("There are no toll keepers here.\n", ch);

		return;
	}

	if ((dir = index_lookup (dirs, buf)) != -1)
	{
		collector = toll_collector (ch->room, dir);
		if (!collector)
		{
			send_to_char ("There is nobody here collecting tolls that way.\n",
				ch);
			return;
		}
	}

	else if ((collector = get_char_room_vis (ch, buf)))
	{
		if (!(af = get_affect (collector, MAGIC_TOLL)))
		{
			act ("$N isn't collecting a toll.\n",
				false, ch, 0, collector, TO_CHAR);
			return;
		}

		dir = af->a.toll.dir;
	}

	else if (!just_a_number (buf))
	{
		send_to_char ("You don't see that person collecting tolls.\n", ch);
		return;
	}

	else
	{
		/* A number WITHOUT a char/dir */
		max_pay = atoi (buf);

		for (dir = 0; dir <= LAST_DIR; dir++)
			if ((collector = toll_collector (ch->room, dir)))
				break;

		if (!collector)
		{
			send_to_char ("Nobody is collecting tolls here.\n", ch);
			return;
		}
	}

	if (GET_POS (collector) <= SLEEP)
	{
		act ("$N isn't conscious.", false, ch, 0, collector, TO_CHAR);
		return;
	}

	if (GET_POS (collector) == FIGHT)
	{
		act ("$N is fighting!", false, ch, 0, collector, TO_CHAR);
		return;
	}

	/* We know dir and who to pay now (collector).  If we don't know
	how much to pay, figure that out. */

	if (!max_pay)
	{
		argument = one_argument (argument, buf);

		if (!just_a_number (buf))
		{
			send_to_char ("How much are you willing to pay?\n", ch);
			return;
		}

		max_pay = atoi (buf);
	}

	af_collector = get_affect (collector, MAGIC_TOLL);

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "for"))
		argument = one_argument (argument, buf);

	if (!*buf)
	{

		if ((af = get_affect (ch, MAGIC_TOLL_PAID)) && af->a.toll.dir == dir)
		{
			send_to_char ("You already have permission to cross.\n", ch);
			return;
		}

		payees[num_payees++] = ch;
	}

	else if (!str_cmp (buf, "all"))
	{

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (!is_human (tch))
				continue;

			if (tch != ch)
				if (tch->deleted || tch->following != ch || !CAN_SEE (ch, tch))
					continue;

			if ((af = get_affect (tch, MAGIC_TOLL_PAID)) &&
				af->a.toll.dir == dir)
				continue;

			payees[num_payees++] = tch;
		}

		if (!num_payees)
		{
			send_to_char ("There is nobody to pay for.\n", ch);
			return;
		}
	}

	else if ((tch = get_char_room_vis (ch, buf)))
	{

		if (!is_human (tch))
		{
			send_to_char ("Only humans need pay tolls.\n", ch);
			return;
		}

		if ((af = get_affect (tch, MAGIC_TOLL_PAID)) && af->a.toll.dir == dir)
		{
			act ("$N already has permission to cross.",
				false, ch, 0, tch, TO_CHAR);
			return;
		}

		payees[num_payees++] = tch;
	}

	else
	{
		send_to_char ("Pay for whom?\n", ch);
		return;
	}

	if (af_collector->a.toll.charge > max_pay)
	{
		sprintf (buf, "$N wants %d credit%s per toll.",
			af_collector->a.toll.charge,
			af_collector->a.toll.charge == 1 ? "" : "s");
		act (buf, false, ch, 0, collector, TO_CHAR);
		return;
	}

	if (!collector->mob)
		currency_type = 0;
	else if (collector->mob->currency_type > 1)
		currency_type = 0;
	else
		currency_type = 1;

	if (!can_subtract_money
		(ch, af_collector->a.toll.charge * num_payees, currency_type))
	{
		send_to_char ("You don't have enough credits to pay the toll.\n", ch);
		return;
	}

	act ("$n pays $N a toll for:", false, ch, 0, collector, TO_NOTVICT);

	sprintf (buf, "$n pays you a toll of %d credit%s for:",
		af_collector->a.toll.charge * num_payees,
		af_collector->a.toll.charge * num_payees == 1 ? "" : "s");
	act (buf, false, ch, 0, collector, TO_VICT);

	sprintf (buf, "You pay $N a toll of %d credit%s for:",
		af_collector->a.toll.charge * num_payees,
		af_collector->a.toll.charge * num_payees == 1 ? "" : "s");
	act (buf, false, ch, 0, collector, TO_CHAR);

	for (i = 0; i < num_payees; i++)
	{
		if (ch == payees[i])
			act ("   yourself", false, ch, 0, 0, TO_CHAR);
		else
			act ("   you", false, payees[i], 0, 0, TO_CHAR);

		act ("   $n", false, payees[i], 0, ch, TO_ROOM);

		magic_add_affect (payees[i], MAGIC_TOLL_PAID, -1, 0, 0, 0, 0);

		af = get_affect (payees[i], MAGIC_TOLL_PAID);
		af->a.toll.dir = dir;
		af->a.toll.room_num = ch->in_room;
	}

	subtract_money (ch, af_collector->a.toll.charge * num_payees,
		currency_type);
}

void
	stop_tolls (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;

	if (!(af = get_affect (ch, MAGIC_TOLL)))
		return;

	if (af->a.toll.room_num == ch->in_room)
	{
		send_to_char ("You stop collecting tolls.\n", ch);
		act ("$n stops collecting tolls.", true, ch, 0, 0, TO_ROOM);
	}

	remove_affect_type (ch, MAGIC_TOLL);
}

CHAR_DATA *
	toll_collector (ROOM_DATA * room, int dir)
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		if (tch->deleted)
			continue;

		if (!(af = get_affect (tch, MAGIC_TOLL)))
			continue;

		if (af->a.toll.dir != dir || af->a.toll.room_num != room->vnum)
			continue;

		return tch;
	}

	return NULL;
}

void
	do_toll (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	ROOM_DIRECTION_DATA *exit;
	char buf[MAX_STRING_LENGTH];

	/*

	Help:

	TOLL

	Setup a toll crossing.  Any character may setup a toll crossing in an area
	that supports tolls.  All humans are stopped and expected to pay a toll of
	your choosing.

	> toll <direction> <amount to collect>

	Example:

	> toll north 5         Setup a toll crossing to the North.  The toll is
	5 coins.

	Non-humans are not expected to pay tolls, since they usually don't have money.
	People who you cannot see can pass without paying toll, if they choose.

	See also:  PAY, SET

	*/

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("toll <direction> <amount to collect>\n", ch);
		return;
	}

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Expected a direction:  North, South, East, or West.\n"
			"toll <direction> <amount to collect>\n", ch);
		return;
	}

	if ((tch = toll_collector (ch->room, dir)) && tch != ch)
	{
		act ("$N is already collecting a toll here.\n",
			false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (!(exit = EXIT (ch, dir)))
	{
		send_to_char ("There is no exit there.\n", ch);
		return;
	}

	if (!IS_SET (exit->exit_info, EX_TOLL))
	{
		send_to_char ("It isn't possible to collect tolls in that direction.\n",
			ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!just_a_number (buf) || atoi (buf) < 1)
	{
		send_to_char ("How much will your charge for others for the toll?\n"
			"toll <direction> <amount to collect>\n", ch);
		return;
	}

	if (tch)
	{
		/* We just want to modify direction or toll charge */
		af = get_affect (ch, MAGIC_TOLL);

		af->a.toll.dir = dir;
		af->a.toll.charge = atoi (buf);
		af->a.toll.room_num = ch->in_room;

		sprintf (buf, "You will collect %d penn%s when people pass %s.\n",
			af->a.toll.charge, af->a.toll.charge > 1 ? "ies" : "y",
			dirs[af->a.toll.dir]);
		send_to_char (buf, ch);

		return;
	}

	magic_add_affect (ch, MAGIC_TOLL, -1, 0, 0, 0, 0);

	af = get_affect (ch, MAGIC_TOLL);

	af->a.toll.dir = dir;
	af->a.toll.charge = atoi (buf);
	af->a.toll.room_num = ch->in_room;

	sprintf (buf, "You will collect %d penn%s when people pass %s.\n",
		af->a.toll.charge, af->a.toll.charge > 1 ? "ies" : "y",
		dirs[af->a.toll.dir]);
	send_to_char (buf, ch);

	sprintf (buf, "$n stands %s, ready to collect tolls.", dirs[dir]);
	act (buf, true, ch, 0, 0, TO_ROOM);
}

int
	can_subtract_money (CHAR_DATA * ch, int farthings_to_subtract,
	int currency_type)
{
	OBJ_DATA *obj, *tobj;
	int money = 0, location;

	if ((obj = ch->right_hand))
	{
		if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
		{
			if (currency_type == CURRENCY_FOOD)
			{
				if (obj->nVirtual >= 50090 && obj->nVirtual <= 50093)
				{
					money += ((int) ch->right_hand->farthings) * ch->right_hand->count;
				}
			}
			else if (currency_type == CURRENCY_PHOENIX)
			{
				if (obj->nVirtual >= 14011 && obj->nVirtual <= 14017)
				{
					money += ((int) ch->right_hand->farthings) * ch->right_hand->count;
				}
			}
		}
		else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = tobj->next_content)
			{
				if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
				{
					if (currency_type == CURRENCY_FOOD)
					{
						if (tobj->nVirtual >= 50090 && tobj->nVirtual <= 50093)
						{
							money += ((int) tobj->farthings) * tobj->count;
						}
					}
					else if (currency_type == CURRENCY_PHOENIX)
					{
						if (tobj->nVirtual >= 14011 && tobj->nVirtual <= 14017)
						{
							money += ((int) tobj->farthings) * tobj->count;
						}
					}
				}
			}
		}
	}


	if ((obj = ch->left_hand))
	{
		if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
		{
			if (currency_type == CURRENCY_FOOD)
			{
				if (obj->nVirtual >= 50090 && obj->nVirtual <= 50093)
				{
					money += ((int) ch->left_hand->farthings) * ch->left_hand->count;
				}
			}
			else if (currency_type == CURRENCY_PHOENIX)
			{
				if (obj->nVirtual >= 14011 && obj->nVirtual <= 14017)
				{
					money += ((int) ch->left_hand->farthings) * ch->left_hand->count;
				}
			}
		}
		else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = tobj->next_content)
			{
				if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
				{
					if (currency_type == CURRENCY_FOOD)
					{
						if (tobj->nVirtual >= 50090 && tobj->nVirtual <= 50093)
						{
							money += ((int) tobj->farthings) * tobj->count;
						}
					}
					else if (currency_type == CURRENCY_PHOENIX)
					{
						if (tobj->nVirtual >= 14011 && tobj->nVirtual <= 14017)
						{
							money += ((int) tobj->farthings) * tobj->count;
						}
					}
				}
			}
		}
	}

	for (location = 0; location < MAX_WEAR; location++)
	{
		if (!(obj = get_equip (ch, location)))
			continue;
		if (GET_ITEM_TYPE (obj) != ITEM_CONTAINER)
			continue;
		if (IS_SET (obj->o.container.flags, CONT_CLOSED))
			continue;
		for (tobj = obj->contains; tobj; tobj = tobj->next_content)
		{
			if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
			{
				if (currency_type == CURRENCY_FOOD)
				{
					if (tobj->nVirtual >= 50090 && tobj->nVirtual <= 50093)
					{
						money += ((int) tobj->farthings) * tobj->count;
					}
				}
				else if (currency_type == CURRENCY_PHOENIX)
				{
					if (tobj->nVirtual >= 14011 && tobj->nVirtual <= 14017)
					{
						money += ((int) tobj->farthings) * tobj->count;
					}
				}
			}
		}
	}

	if (money < farthings_to_subtract)
		return 0;

	return 1;
}

// If you give the subtract_money function a negative farthings_to_subtract, it will supress all output to the player
void
	subtract_money (CHAR_DATA * ch, int farthings_to_subtract, int currency_type)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *tobj, *obj, *next_obj;
	bool change = false;
	int money = 0, location;
	bool container = false;
	bool SupressOutput = false; // Japheth's addition

	if (farthings_to_subtract < 0)
	{
		farthings_to_subtract *= -1;
		SupressOutput = true;
	}

	for (location = 0; location < MAX_WEAR; location++)
	{
		if (!(obj = get_equip (ch, location)))
			continue;
		if (GET_ITEM_TYPE (obj) != ITEM_CONTAINER)
			continue;
		if (IS_SET (obj->o.container.flags, CONT_CLOSED))
			continue;
		for (tobj = obj->contains; tobj; tobj = next_obj)
		{
			next_obj = tobj->next_content;
			if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
			{
				if ((currency_type == CURRENCY_FOOD
					&& (tobj->nVirtual >= 50090 && tobj->nVirtual <= 50093)))
				{
					money += ((int) tobj->farthings) * tobj->count;
					obj_from_obj (&tobj, 0);
					extract_obj (tobj);
					ch->delay_obj = obj;
					container = true;
				}
				else if ((currency_type == CURRENCY_PHOENIX
					&& (tobj->nVirtual >= 14011 && tobj->nVirtual <= 14017)))
				{
					money += ((int) tobj->farthings) * tobj->count;
					obj_from_obj (&tobj, 0);
					extract_obj (tobj);
					ch->delay_obj = obj;
					container = true;
				}
			}
		}
	}

	if ((obj = ch->right_hand))
	{
		if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
		{
			if ((currency_type == CURRENCY_FOOD
				&& (obj->nVirtual >= 50090 && obj->nVirtual <= 50093)))
			{
				money += ((int) ch->right_hand->farthings) * ch->right_hand->count;
				extract_obj (ch->right_hand);
				ch->right_hand = NULL;
			}
			else if ((currency_type == CURRENCY_PHOENIX
				&& (obj->nVirtual >= 14011 && obj->nVirtual <= 14017)))
			{
				money += ((int) ch->right_hand->farthings) * ch->right_hand->count;
				extract_obj (ch->right_hand);
				ch->right_hand = NULL;
			}
		}
		else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = next_obj)
			{
				next_obj = tobj->next_content;
				if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
				{
					if ((currency_type == CURRENCY_FOOD
						&& (tobj->nVirtual >= 50090
						&& tobj->nVirtual <= 50093)))
					{
						money += ((int)tobj->farthings * tobj->count);
						obj_from_obj (&tobj, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
					else if ((currency_type == CURRENCY_PHOENIX
						&& (tobj->nVirtual >= 14011
						&& tobj->nVirtual <= 14017)))
					{
						money += ((int)tobj->farthings * tobj->count);
						obj_from_obj (&tobj, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
				}
			}
		}
	}

	if ((obj = ch->left_hand))
	{
		if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
		{
			if ((currency_type == CURRENCY_FOOD
				&& (obj->nVirtual >= 50090 && obj->nVirtual <= 50093)))
			{
				money += ((int)ch->left_hand->farthings * ch->left_hand->count);
				extract_obj (ch->left_hand);
				ch->left_hand = NULL;
			}
			else if ((currency_type == CURRENCY_PHOENIX
				&& (obj->nVirtual >= 14011 && obj->nVirtual <= 14017)))
			{
				money += ((int)ch->left_hand->farthings * ch->left_hand->count);
				extract_obj (ch->left_hand);
				ch->left_hand = NULL;
			}
		}
		else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = next_obj)
			{
				next_obj = tobj->next_content;
				if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
				{
					if ((currency_type == CURRENCY_FOOD
						&& (tobj->nVirtual >= 50090
						&& tobj->nVirtual <= 50093)))
					{
						money += ((int) tobj->farthings * tobj->count);
						obj_from_obj (&tobj, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
					else if ((currency_type == CURRENCY_PHOENIX
						&& (tobj->nVirtual >= 14011
						&& tobj->nVirtual <= 14017)))
					{
						money += ((int) tobj->farthings * tobj->count);
						obj_from_obj (&tobj, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
				}
			}
		}
	}

	money -= farthings_to_subtract;

	if (money <= 0) // Serious bugfix - Japheth 10th May 2007
	{
		return;
	}

	obj = ch->delay_obj;
	ch->delay_obj = NULL;


	if (currency_type == CURRENCY_FOOD)
	{

		if (money / 240)
		{
			// Silver tree.
			tobj = load_object (50093);
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			tobj->count = money / 240;
			money %= 240;
			change = true;
		}

		if (money / 25)
		{
			// Silver royal.
			tobj = load_object (50092);
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			tobj->count = money / 25;
			money %= 25;
			change = true;
		}

		if (money / 5)
		{
			// Bronze ration.
			tobj = load_object (50091);
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			tobj->count = money / 5;
			money %= 5;
			change = true;
		}

		if (money)
		{
			// ration bit.
			tobj = load_object (50090);
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			tobj->count = money;
			change = true;
		}
	}
	else if (currency_type == CURRENCY_PHOENIX)
	{
		if (money / 1000)
		{
			tobj = load_object (14017);
			tobj->count = money / 1000;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			money %= 1000;
			change = true;
		}

		if (money / 100)
		{
			tobj = load_object (14016);
			tobj->count = money / 100;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			money %= 100;
			change = true;
		}

		if (money / 50)
		{
			tobj = load_object (14015);
			tobj->count = money / 50;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			change = true;
			money %= 50;
		}

		if (money / 20)
		{
			tobj = load_object (14014);
			tobj->count = money / 20;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			change = true;
			money %= 20;
		}

		if (money / 10)
		{
			tobj = load_object (14013);
			tobj->count = money / 10;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			change = true;
			money %= 10;
		}

		if (money / 5)
		{
			tobj = load_object (14012);
			tobj->count = money / 5;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			change = true;
			money %= 5;
		}

		if (money)
		{
			tobj = load_object (14011);
			tobj->count = money;
			if (obj)
				obj_to_obj (tobj, obj);
			else
				obj_to_char (tobj, ch);
			change = true;
		}
	}

	if (!SupressOutput)
	{
		if (container)
			send_to_char
			("\nRifling through your belongings, you retrieve your chips.\n", ch);
		else
			send_to_char ("\nYou offer up the specified amount.\n", ch);

		if (change)
		{
			if (obj)
				sprintf (buf, "Change is made, which you then deposit in #2%s#0.",
				obj_short_desc (obj));
			else
				sprintf (buf, "Change is made for the amount offered.");
			send_to_char ("\n", ch);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
	}

	ch->delay_obj = NULL;
}

void
	money_from_char_to_room (CHAR_DATA * ch, int vnum)
{
	OBJ_DATA *obj;

	if (!vnum || !vnum_to_room (vnum))
		return;

	if ((obj = get_obj_in_list_num (VNUM_FARTHING, ch->right_hand)) ||
		(obj = get_obj_in_list_num (VNUM_FARTHING, ch->left_hand)))
	{
		obj_from_char (&obj, 0);
		obj_to_room (obj, vnum);
	}

	if ((obj = get_obj_in_list_num (VNUM_PENNY, ch->right_hand)) ||
		(obj = get_obj_in_list_num (VNUM_PENNY, ch->left_hand)))
	{
		obj_from_char (&obj, 0);
		obj_to_room (obj, vnum);
	}
}

int
	name_to_econ_zone (char *econ_zone_name)
{
	int i;

	for (i = 0; *default_econ_info[i].flag_name != '\n'; i++)
		if (!strcmp (econ_zone_name, default_econ_info[i].flag_name))
			return i;

	return -1;
}

int
	zone_to_econ_zone (int zone)
{
	switch (zone)
	{
	case 15:
		return name_to_econ_zone ("azadmere");
	case 20:
		return name_to_econ_zone ("cherafir");
	case 5:
		return name_to_econ_zone ("coranan");
	case 50:
		return name_to_econ_zone ("evael");
	case 10:
	case 11:
	case 32:
	case 76:
		return name_to_econ_zone ("kaldor");
	case 51:
		return name_to_econ_zone ("kanday");
	case 23:
	case 25:
	case 27:
	case 31:
		return name_to_econ_zone ("orbaal");
	case 40:
		return name_to_econ_zone ("rethem");
	case 58:
		return name_to_econ_zone ("shiran");
	case 59:
	case 60:
	case 62:
		return name_to_econ_zone ("telen");
	case 14:
		return name_to_econ_zone ("thay");
	case 12:
		return name_to_econ_zone ("trobridge");

	default:
		return -1;
	}
}

int
	obj_to_econ_zone (OBJ_DATA * obj)
{
	int i;
	int j;

	for (i = 0; *econ_flags[i] != '\n'; i++)
		if (IS_SET (obj->econ_flags, (1 << i)))
			for (j = 0; *default_econ_info[j].flag_name != '\n'; j++)
				if (!strcmp (econ_flags[i], default_econ_info[j].flag_name))
					return j;

	return -1;
}

void
	econ_markup_discount (CHAR_DATA * keeper, OBJ_DATA * obj, float *markup,
	float *discount)
{
	/* Ok, this is a bit nutty.  How it works is...

	If the flag ACT_ECONZONE is set, then we have default markups
	and discounts (defined by default_econ_info in constants.c) that
	apply and override settings the keeper might have.

	Certain zones are econ zones, such as Kaldor which is zones
	10, 11, 31, and 76.  So, if an object has an econ_flag listed
	in default_econ_info, we use the markup/discount determined in
	the matrix using the object's econ flag against the zone's econ
	zone.

	If the keeper isn't flagged as ACT_ECONZONE, then match the
	different econ flags against the object to pick a discount/markup.
	*/

	int keeper_econ_zone = -1;
	int object_econ_zone = -1;
	char buf[MAX_STRING_LENGTH];

	if (IS_SET (keeper->act, ACT_ECONZONE))
	{
		keeper_econ_zone = zone_to_econ_zone (keeper->room->zone);
		object_econ_zone = obj_to_econ_zone (obj);

		if (object_econ_zone == -1)
			object_econ_zone = keeper_econ_zone;

		if (keeper_econ_zone == -1)
		{
			sprintf (buf, "Keeper %d in room %d can't be associated with "
				"an econ zone.",
				IS_NPC (keeper) ? keeper->mob->vnum : 0,
				keeper->in_room);
			system_log (buf, true);
		}

		else
		{
			*markup = default_econ_info[object_econ_zone].obj_econ_info
				[keeper_econ_zone].markup;
			*discount = default_econ_info[object_econ_zone].obj_econ_info
				[keeper_econ_zone].discount;
			return;
		}
	}

	if (obj->econ_flags & keeper->shop->econ_flags1 &&
		keeper->shop->econ_markup1 > 0)
	{
		*markup = keeper->shop->econ_markup1;
		*discount = keeper->shop->econ_discount1;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags2 &&
		keeper->shop->econ_markup2 > 0)
	{
		*markup = keeper->shop->econ_markup2;
		*discount = keeper->shop->econ_discount2;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags3 &&
		keeper->shop->econ_markup3 > 0)
	{
		*markup = keeper->shop->econ_markup3;
		*discount = keeper->shop->econ_discount3;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags4 &&
		keeper->shop->econ_markup4 > 0)
	{
		*markup = keeper->shop->econ_markup4;
		*discount = keeper->shop->econ_discount4;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags5 &&
		keeper->shop->econ_markup5 > 0)
	{
		*markup = keeper->shop->econ_markup5;
		*discount = keeper->shop->econ_discount5;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags6 &&
		keeper->shop->econ_markup6 > 0)
	{
		*markup = keeper->shop->econ_markup6;
		*discount = keeper->shop->econ_discount6;
	}

	else if (obj->econ_flags & keeper->shop->econ_flags7 &&
		keeper->shop->econ_markup7 > 0)
	{
		*markup = keeper->shop->econ_markup7;
		*discount = keeper->shop->econ_discount7;
	}

	else
	{
		*markup = keeper->shop->markup;
		*discount = keeper->shop->discount;
	}
}

float
	econ_discount (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	float markup;
	float discount;

	econ_markup_discount (keeper, obj, &markup, &discount);

	return discount;
}

float
	econ_markup (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	float markup;
	float discount;

	econ_markup_discount (keeper, obj, &markup, &discount);

	return markup;
}



float
	tally (OBJ_DATA * obj, char *buffer, int depth, int *weight)
{
	int count = 0;
	float cost = 0.0, subtotal = 0.0;
	char format[AVG_STRING_LENGTH] = "";
	char weights[AVG_STRING_LENGTH] = "";
	OBJ_DATA *tobj = NULL;

	if (!obj || (strlen (buffer) > MAX_STRING_LENGTH - 256))
		return 0.00;

	count = (obj->count) ? obj->count : 1;
	cost = (obj->farthings + obj->silver * 4) * count;
	sprintf(weights, ", %d.%02d lb", obj->obj_flags.weight / 100, obj->obj_flags.weight % 100);
	*weight += obj->obj_flags.weight * count;
	sprintf (format, "%% 10.02f cp%%s - %%%dc#2%%s#0", 2 * depth);
	sprintf (buffer + strlen (buffer), format, cost, weights, ' ',
		obj->short_description);
	if (count > 1)
	{
		sprintf (buffer + strlen (buffer), " (x%d)\n", count);
	}
	else
	{
		strcat (buffer, "\n");
	}
	subtotal += cost;

	for (tobj = obj->contains; tobj; tobj = tobj->next_content)
	{
		subtotal += tally (tobj, buffer, depth + 1, &*weight);
	}
	return subtotal;
}

void
	do_tally (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL, *tch2 = NULL;
	OBJ_DATA *obj = NULL;
	int location = 0;
	float subtotal = 0.0, total = 0.0;
	int weight = 0;
	bool bTallyAll = true;
	char arg1[AVG_STRING_LENGTH] = "";
	char buffer[MAX_STRING_LENGTH] = "";

	if (!GET_TRUST (ch))
	{
		send_to_char ("Huh?\n", ch);
		return;
	}

	if (argument && *argument)
	{
		argument = one_argument (argument, arg1);
		bTallyAll = false;
		if (!(tch2 = get_char_room_vis (ch, arg1)))
		{
			send_to_char ("You do not see them here.\n", ch);
			return;
		}
	}

	if (bTallyAll)
	{
		strcpy (buffer, "\n#6Tally in Room:#0\n");
		for (obj = ch->room->contents; obj; obj = obj->next_content)
		{
			subtotal += tally (obj, buffer, 1, &weight);
		}
		sprintf (buffer + strlen (buffer),
			"----------------------------------------------\n"
			"% 10.02f cp - Subtotal\n"
			" %10d.%02d lb - Subtotal\n", subtotal, weight / 100, weight % 100);
		total += subtotal;
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if ((!bTallyAll && tch != tch2) || (bTallyAll && tch == ch))
		{
			continue;
		}


		subtotal = 0.0;
		sprintf (buffer + strlen (buffer),
			"\n#6Tally of #5%s#0 (%s):#0\n", tch->short_descr, tch->tname);

		if ((obj = tch->right_hand))
		{
			subtotal += tally (obj, buffer, 1, &weight);
		}

		if ((obj = tch->left_hand))
		{
			subtotal += tally (obj, buffer, 1, &weight);
		}

		for (location = 0; location < MAX_WEAR; location++)
		{
			if (!(obj = get_equip (tch, location)))
				continue;
			subtotal += tally (obj, buffer, 1, &weight);
		}

		sprintf (buffer + strlen (buffer),
			"----------------------------------------------\n"
			"% 10.02f cp - %sotal\n"
			" %10d.%02d lb - %sotal\n",
			subtotal, (bTallyAll) ? "Subt" : "T",
			weight / 100, weight % 100, (bTallyAll) ? "Subt" : "T");

		total += subtotal;
	}

	if (bTallyAll)
	{

		sprintf (buffer + strlen (buffer),
			"==============================================\n"
			"% 10.02f cp - Total\n"
			" %10d.%02d lb - Total\n", total, weight / 100, weight % 100);

	}

	page_string (ch->descr(), buffer);
}

#include <memory>

void
	do_mark (CHAR_DATA* ch, char *argument, int cmd)
{

	// First - Assert we have a valid usage case
	if (!IS_NPC(ch) || !ch->shop)
	{
		const char* message =
			"#6OOC - Only the shopkeeper mob may use this command.#0\n"
			"#6      e.g: command <clanmember> mark <item> <value>#0\n";
		send_to_char (message, ch);
		return;
	}

	// Make certain we have a storage room
	ROOM_DATA* store = 0;
	if (!(store = vnum_to_room (ch->shop->store_vnum)))
	{
		do_ooc (ch, "I seem to have lost my store room.", 0);
		return;
	}

	// Load the storage if we need to
	if (!store->psave_loaded)
	{
		load_save_room (store);
	}

	// Check if we have an inventory to work with
	if (!store->contents)
	{
		do_say (ch, "I have nothing for sale at the moment.", 0);
		return;
	}


	////////////////////////////////////////////////////////////////////////////
	// Usages: mark <what to change> <new value>
	// what to change: number, keyword, nth.keyword, all.keyword, all
	// new value: number, free, reset
	////////////////////////////////////////////////////////////////////////////
	char* ptr = 0;

	enum
	{
		mark_none = 0,
		mark_by_list_number,
		mark_by_keyword,
		mark_nth_by_keyword,
		mark_all_by_keyword,
		mark_all
	}
	mark_mode = mark_none;

	// first argument either a number or keyword

	if (!argument || !*argument)
	{
		do_say (ch, "What did you want to change the price of?", 0);
		return;
	}

	int list_number = 0;
	std::string keyword;
	char *item_string = new char[strlen (argument)];
	argument = one_argument (argument, item_string);
	if (strncasecmp ("all", item_string, 3) == 0)
	{
		if (strlen (item_string) == 3)
		{
			mark_mode = mark_all;
		}
		else if (item_string[3] == '.')
		{
			mark_mode = mark_all_by_keyword;
			keyword = item_string + 4;
		}
		else
		{
			mark_mode = mark_by_keyword;
			keyword = item_string;
		}
	}
	else if ((list_number = strtol (item_string, &ptr, 10)))
	{
		if (ptr && *ptr)
		{
			if (*ptr == '.')
			{
				mark_mode = mark_nth_by_keyword;
				keyword = ptr + 1;
			}
			else
			{
				mark_mode = mark_by_keyword;
				keyword = item_string;
			}
		}
		else
		{
			mark_mode = mark_by_list_number;
		}
	}
	else
	{
		mark_mode = mark_by_keyword;
		keyword = item_string;
	}
	delete [] item_string;

	// second argument must be a decimal, "free", or "reset"
	if (!argument || !*argument)
	{
		do_say (ch, "What price did you want to set?", 0);
		return;
	}

	float new_value;
	char* value_string = new char[strlen (argument)];
	argument = one_argument (argument, value_string);
	if (strcasecmp ("free", value_string) == 0)
	{
		new_value = -1.0f;
	}
	else if (strcasecmp ("reset", value_string) == 0)
	{
		new_value = 0.0f;
	}
	else if (!(new_value = strtof (value_string, &ptr))
		&& (ptr >= value_string))
	{
		char* errmsg =
			"The 'new value' parameter must be either a decimal number, "
			"'free', or 'reset'.";
		delete [] value_string;
		do_ooc (ch, errmsg, 0);
		return;
	}
	else if (new_value <= 0.0f)
	{
		new_value = -1.0f;
	}
	delete [] value_string;
	// else: new_value is a valid decimal.


	if (mark_mode == mark_by_keyword)
	{
		mark_mode = mark_nth_by_keyword;
		list_number = 1;
	}

	bool found = false;
	int i = 1;
	int count = 0;
	int currency = ch->mob->currency_type;
	OBJ_DATA* obj = 0;
	for (obj = store->contents; (obj && !found); obj = obj->next_content)
	{
		// skip money
		if (GET_ITEM_TYPE (obj) == ITEM_MONEY
			&& keeper_uses_currency_type (currency, obj))
		{
			continue;
		}

		if (
			// mark all
			(mark_mode == mark_all)

			// mark_by_list_number
			|| (mark_mode == mark_by_list_number
			&& (list_number == i++)
			&& (found = true))

			// mark_(nth/all)_by_keyword
			|| ((isname (keyword.c_str (), obj->name)
			|| (GET_ITEM_TYPE (obj) == ITEM_BOOK
			&& obj->book_title
			&& isname (keyword.c_str (), obj->book_title))
			|| (GET_ITEM_TYPE (obj) == ITEM_DRINKCON
			&& obj->contains			
			&& isname (keyword.c_str (), obj->contains->name)))
			&& (mark_mode == mark_all_by_keyword
			|| (mark_mode == mark_nth_by_keyword
			&& (list_number == i++)
			&& (found = true)))))

		{
			obj->obj_flags.set_cost = (int)(100.0 * new_value);
			++count;
		}

	}
	if (count)
	{
		do_say (ch, "Our inventory is updated.", 0);
	}
	else
	{
		do_say (ch, "There doesn't seem to be anything like that "
			"in our inventory.", 0);
	}
}

void
	do_payroll (CHAR_DATA * ch, char *argument, int cmd)
{
	short last_day = -1, last_month = -1, last_year = -1;
	int payrollAmt = 0, payrollTAmt = 0;
	long day = -1, month = -1, year = -1;
	CHAR_DATA *keeper = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH];
	//int shopnum = 0;

	if (IS_NPC (ch) && ch->shop)
	{
		keeper = ch;
	}
	else
	{
		for (keeper = character_list; keeper; keeper = keeper->next)
		{
			if (IS_NPC (keeper) &&
				keeper->shop &&
				keeper->shop->store_vnum == ch->in_room)
				break;
		}
	}

	if (keeper == NULL)
	{
		send_to_char ("You do not see a payroll ledger here.\n", ch);
		return;
	}

	/* Detail */
	int port = engine.get_port ();
	mysql_safe_query
		("SELECT time, shopkeep, customer, "
		"amount, room, gametime, port, "
		"EXTRACT(YEAR FROM gametime) as year, "
		"EXTRACT(MONTH FROM gametime) as month, "
		"EXTRACT(DAY FROM gametime) as day "
		"FROM %s.payroll "
		"WHERE shopkeep = '%d' AND port = '%d' "
		"ORDER BY time DESC;",
		(engine.get_config ("player_log_db")).c_str (),
		keeper->mob->vnum, port);

	if ((result = mysql_store_result (database)) == NULL)
	{
		send_to_gods ((char *) mysql_error (database));
		send_to_char ("The payroll ledgers are unavailable at the moment.\n", ch);
		return;
	}

	send_to_char ("Examining a payroll ledger:\n", ch);

	sprintf(buf, "---------------------------\n");
	while ((row = mysql_fetch_row (result)) != NULL)
	{

		day = strtol (row[9], NULL, 10);
		month = strtol (row[8], NULL, 10) - 1;
		year = strtol (row[7], NULL, 10);
		if (day != last_day || month != last_month || year != last_year)
		{
			if (last_day > 0 && month != last_month)
			{
				sprintf (buf + strlen (buf),
					"\n    Total for #6%s %d#0: #2%d cp#0.\n\n",
					month_short_name[(int) last_month],
					(int) last_year,
					payrollAmt);

				payrollTAmt += payrollAmt;
				payrollAmt = 0;
			}

			sprintf (buf + strlen (buf),
				"\n On #6%s %d#0:\n\n",
				short_time_string(day, month),
				(int) year);

			last_day = day;
			last_month = month;
			last_year = year;
		}

		payrollAmt += strtol (row[3], NULL, 10);

		sprintf (buf + strlen (buf), " #5%s#0 was paid %ld credits.\n", row[2], strtol (row[3], NULL, 10));

		if (strlen (buf) > MAX_STRING_LENGTH - 512)
		{
			strcat (buf, "\n #1There were more paychecks than could be displayed.#0\n\n");
			break;
		}
	} //end while

	mysql_free_result (result);

	if (last_day > 0)
	{
		if (payrollTAmt == 0 &&
			payrollAmt > 0)
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Payroll #2%d cp#0.\n\n"
				"    Current credits on hand: #2%d cp#0.\n",
				month_short_name[(int) last_month],
				(int) last_year,
				payrollAmt,
				keeper_has_money (keeper, 0));
		}
		else
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Payroll #2%d cp#0.\n\n"
				"    Total for period:      Payroll #2%d cp#0.\n"
				"    Current credits on hand:  #2%d cp#0.\n",
				month_short_name[(int) last_month],
				(int) last_year,
				payrollAmt,
				payrollTAmt,
				keeper_has_money (keeper, 0));
		}
		page_string (ch->descr(), buf);
	}
}

// trade assess : uses haggle + eavesdrop skill to gauge saturation
// trade gossip : uses eavesdrop skill to find out last person who traded
// trade goods  : sells some goods for some cash.
// trade sneaky : uses the steal skill to make a roll for some extra cash.
// trade noisily: uses the haggle skill for some extra cash.

void
	do_trade (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *market_obj = NULL;
	OBJ_DATA *bundle_obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	std::string strArgument = argument, argOne;

	for (market_obj = ch->room->contents; market_obj; market_obj = market_obj->next_content)
	{
		if (GET_ITEM_TYPE(market_obj) == ITEM_TRADE_MARKET)
		{
			break;
		}
	}

	if (!market_obj)
	{
		send_to_char("There don't see any market here from which you can trade.\n", ch);
		return;
	}

	strArgument = one_argument(strArgument, argOne);

	if (argOne.empty())
	{
		sprintf(buf,               "trade usage: trade <goods|sneakily|noisily|assess>\n");
		sprintf(buf + strlen(buf), "trade goods    - sells a bundle of goods to the commonfolk in the marketplace\n");
		sprintf(buf + strlen(buf), "trade sneakily - sells goods, and use your steal skill to shortchange customers\n");
		sprintf(buf + strlen(buf), "trade noisily  - sells goods, and use your haggle skill to aggressively up-sell\n");
		sprintf(buf + strlen(buf), "trade assess   - use your eavesdrop skill to determine what products are in demand\n");
		sprintf(buf + strlen(buf), "trade assist   - use your skills to help someone who is already trading\n");
		//sprintf(buf + strlen(buf), "trade gossip   - use your eavesdrop skill to find out whats been sold\n");

		send_to_char(buf, ch);
		return;
	}
	else if (argOne == "goods")
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->left_hand;
		}
		else
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		CHAR_DATA *tch = NULL;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (ch == tch)
				continue;
			if (tch->delay_type == DEL_TRADE)
			{
				send_to_char("Someone is already trading here: you'll need to wait until they're finished.\n", ch);
				return;

			}
		}

		act ("You make your way to one of the vacant stalls in the market and unwrap $P, beginning your call for customers.", false, ch, market_obj, bundle_obj, TO_CHAR | _ACT_FORMAT);
		act ("$n unwraps $P at a vacant stall in $p and begins hawking $s wares.", false, ch, market_obj, bundle_obj, TO_ROOM | _ACT_FORMAT);

		send_to_char("\n#6OOC: Each trade takes take roughly 5 RL minutes to complete - roleplay it out!#0\n", ch);

		ch->delay_type = DEL_TRADE;
		ch->delay = 60;
		ch->delay_info1 = 0; // This is where your modifier comes in.
		ch->delay_info2 = 2; // This is how many loops we have left.
		ch->delay_obj = bundle_obj;
	}
	else if (argOne == "sneakily")
	{
		if (ch->skills[SKILL_STEAL] < 10)
		{
			send_to_char("You need some ability to steal, palm and cheat before you can try that.\n", ch);
			return;
		}

		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->left_hand;
		}
		else
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		CHAR_DATA *tch = NULL;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (ch == tch)
				continue;
			if (tch->delay_type == DEL_TRADE)
			{
				send_to_char("Someone is already trading here: you'll need to wait until they're finished.\n", ch);
				return;

			}
		}

		act ("You make your way to one of the vacant stalls in the market and unwrap $P, beginning your call for customers who you look forwarding to short-changing...", false, ch, market_obj, bundle_obj, TO_CHAR | _ACT_FORMAT);
		act ("$n unwraps $P at a vacant stall in $p and begins hawking $s wares.", false, ch, market_obj, bundle_obj, TO_ROOM | _ACT_FORMAT);

		send_to_char("\n#6OOC: Each trade takes take roughly 5 RL minutes to complete - roleplay it out!#0\n", ch);

		ch->delay_type = DEL_TRADE;
		ch->delay = 60;
		ch->delay_info1 = 1; // This is where your modifier comes in.
		ch->delay_info2 = 2; // This is how many loops we have left.
		ch->delay_obj = bundle_obj;
	}
	else if (argOne == "noisily")
	{
		if (ch->skills[SKILL_HAGGLE] < 10)
		{
			send_to_char("You need some ability to shout, bargain, and make a sale before you can try that.\n", ch);
			return;
		}

		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TRADE_BUNDLE)
		{
			bundle_obj = ch->left_hand;
		}
		else
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		CHAR_DATA *tch = NULL;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (ch == tch)
				continue;
			if (tch->delay_type == DEL_TRADE)
			{
				send_to_char("Someone is already trading here: you'll need to wait until they're finished.\n", ch);
				return;

			}
		}

		act ("You make your way to one of the vacant stalls in the market and unwrap $P, bellowing out the virtues of your goods to prospective customers.", false, ch, market_obj, bundle_obj, TO_CHAR | _ACT_FORMAT);
		act ("$n unwraps $P at a vacant stall in $p and begins hawking $s wares.", false, ch, market_obj, bundle_obj, TO_ROOM | _ACT_FORMAT);

		send_to_char("\n#6OOC: Each trade takes take roughly 5 RL minutes to complete - roleplay it out!#0\n", ch);

		ch->delay_type = DEL_TRADE;
		ch->delay = 60;
		ch->delay_info1 = 2; // This is where your modifier comes in.
		ch->delay_info2 = 2; // This is how many loops we have left.
		ch->delay_obj = bundle_obj;
	}
	else if (argOne == "assist")
	{
		std::string argTwo;
		CHAR_DATA *tch = NULL;
		CHAR_DATA *rch = NULL;
		int assistance_type = 0;

		strArgument = one_argument(strArgument, argTwo);

		if (argTwo.empty())
		{
			send_to_char("You don't see them here - you need to target a person currently trading.\n", ch);
			return;
		}

		if (!(tch = get_char_room_vis (ch, argTwo.c_str())))
		{
			send_to_char("You don't see them here - you need to target a person currently trading.\n", ch);
			return;
		}

		if (tch == ch)
		{
			send_to_char("You can't assist yourself.\n", ch);
			return;
		}

		if (tch->delay_type != DEL_TRADE)
		{
			act("You can't assist $n because they aren't trading.", false, tch, 0, ch, TO_VICT | _ACT_FORMAT);
			return;
		}

		if (strArgument.empty())
		{
			sprintf(buf,               "You have the following options to assist the trader:\n");
			sprintf(buf + strlen(buf), "protect    - use Brawling to protect the trader from pickpockets and gossip\n");
			sprintf(buf + strlen(buf), "spruik     - use Haggle to drive more business to the trader\n");
			sprintf(buf + strlen(buf), "entertain  - use Artistry to lure more customers to the trader\n");
			sprintf(buf + strlen(buf), "pickpocket - use Steal fleece customers about the trader's stall\n");
			sprintf(buf + strlen(buf), "gossip     - use Eavesdrop to spread malicious rumours about the trader's competitors\n");

			send_to_char(buf, ch);
			return;
		}

		if (strArgument == "protect")
		{
			if (ch->skills[SKILL_BRAWLING] < 10)
			{
				send_to_char("You don't know enough about Brawling to provide assistance that way.\n" ,ch);
				return;
			}

			sprintf(buf2, "providing a stern eye of the goings on in $p.");
			assistance_type = 1;
		}
		else if (strArgument == "spruik")
		{
			if (ch->skills[SKILL_HAGGLE] < 10)
			{
				send_to_char("You don't know enough about Haggling to provide assistance that way.\n" ,ch);
				return;
			}

			sprintf(buf2, "raising a voice to help further excite the crowd of $p.");
			assistance_type = 2;
		}
		else if (strArgument == "entertain")
		{
			if (ch->skills[SKILL_ARTISTRY] < 10)
			{
				send_to_char("You don't know enough about Artistry to provide assistance that way.\n" ,ch);
				return;
			}

			sprintf(buf2, "deploying artistic talents to intrigue the crowds of $p.");
			assistance_type = 3;
		}
		else if (strArgument == "pickpocket")
		{
			if (ch->skills[SKILL_STEAL] < 10)
			{
				send_to_char("You don't know enough about Stealing to provide assistance that way.\n" ,ch);
				return;
			}

			sprintf(buf2, "keeping a very sharp eye on the trade occuring in $p.");
			assistance_type = 4;
		}
		else if (strArgument == "gossip")
		{
			if (ch->skills[SKILL_EAVESDROP] < 10)
			{
				send_to_char("You don't know enough about Eavesdropping to provide assistance that way.\n" ,ch);
				return;
			}

			sprintf(buf2, "mingling with and gossiping to the crowds of $p.");
			assistance_type = 5;
		}
		else
		{
			sprintf(buf,               "You have the following options to assist the trader:\n");
			sprintf(buf + strlen(buf), "protect    - use Brawling to protect the trader from pickpockets and gossip\n");
			sprintf(buf + strlen(buf), "spruik     - use Haggle to drive more business to the trader\n");
			sprintf(buf + strlen(buf), "entertain  - use Artistry to lure more customers to the trader\n");
			sprintf(buf + strlen(buf), "pickpocket - use Steal fleece customers about the trader's stall\n");
			sprintf(buf + strlen(buf), "gossip     - use Eavesdrop to spread malicious rumours about the trader's competitors\n");

			send_to_char(buf, ch);
			return;
		}

		for (rch = ch->room->people; rch; rch = rch->next)
		{
			if (rch == tch || rch == ch)
				continue;
			if (rch->delay_type == DEL_TRADE_ASSIST && rch->delay_info3 == assistance_type)
			{
				send_to_char("Someone is already providing that kind of assistance to a trader here.\n", ch);
				return;

			}
		}

		sprintf(buf, "You move to assist $N in $S trading, %s", buf2);
		act (buf, false, ch, market_obj, tch, TO_CHAR | _ACT_FORMAT);
		sprintf(buf, "$n moves to assist you in your trading, %s", buf2);
		act (buf, false, ch, market_obj, tch, TO_VICT | _ACT_FORMAT);
		sprintf(buf, "$n moves to assist $N in $S trading, %s", buf2);
		act (buf, false, ch, market_obj, tch, TO_NOTVICT | _ACT_FORMAT);

		send_to_char("\n#6OOC: Each trade takes take roughly 5 RL minutes to complete - roleplay it out!#0\n", ch);

		ch->delay_type = DEL_TRADE_ASSIST;
		ch->delay = tch->delay + 1;
		ch->delay_info2 = tch->delay_info2;
		ch->delay_info3 = assistance_type;
		ch->delay_ch = tch;
		return;

	}
	else if (argOne == "assess")
	{
		if (ch->skills[SKILL_EAVESDROP] < 10)
		{
			send_to_char("You need some ability to eavesdrop before you can find out any useful information.\n", ch);
			return;
		}

		act ("You linger about $p, hoping to overhear some valuable gossip about what goods are in demand and what are oversupplied.", false, ch, market_obj, 0, TO_CHAR | _ACT_FORMAT);
		ch->delay_type = DEL_TRADE;
		ch->delay = 30;
		ch->delay_info1 = 3; // This is where your modifier comes in.
		ch->delay_info2 = 0; // This is how many loops we have left.
	}
	else
	{
		sprintf(buf,               "trade usage: trade <goods|sneakily|noisily|assess|assist>\n");
		sprintf(buf + strlen(buf), "trade goods    - sells a bundle of goods to the commonfolk in the marketplace\n");
		sprintf(buf + strlen(buf), "trade sneakily - sells goods, and use your steal skill to shortchange customers\n");
		sprintf(buf + strlen(buf), "trade noisily  - sells goods, and use your haggle skill to aggressively up-sell\n");
		sprintf(buf + strlen(buf), "trade assess   - use your eavesdrop skill to determine what products are in demand\n");
		sprintf(buf + strlen(buf), "trade assist   - use your skills to help someone who is already trading\n");
		//sprintf(buf + strlen(buf), "trade gossip   - use your eavesdrop skill to find out whats been sold\n");

		send_to_char(buf, ch);
		return;
	}
}


void
	delayed_trade_assist (CHAR_DATA *ch)
{
	OBJ_DATA *market_obj = NULL;
	CHAR_DATA *tch = NULL;

	for (market_obj = ch->room->contents; market_obj; market_obj = market_obj->next_content)
	{
		if (GET_ITEM_TYPE(market_obj) == ITEM_TRADE_MARKET)
		{
			break;
		}
	}

	if (!market_obj)
	{
		send_to_char("There don't see any market here from which you can assist a trader.\n", ch);
		return;
	}

	if (ch->delay_info2)
	{				
		if (!(tch = ch->delay_ch))
		{
			send_to_char("There needs to be a person presently trading for you to assist.\n", ch);
			return;
		}

		if (ch->room != tch->room || tch->delay_type != DEL_TRADE)
		{
			send_to_char("There needs to be a person presently trading for you to assist.\n", ch);
			return;
		}

		CHAR_DATA *rch = NULL;

		for (rch = ch->room->people; rch; rch = rch->next)
		{
			if (rch == tch || rch == ch)
			{
				continue;
			}
			if (rch->delay_type == DEL_TRADE_ASSIST && rch->delay_info3 == ch->delay_info3)
			{
				send_to_char("Someone is already providing that kind of assistance to a trader here.\n", ch);
				return;

			}
		}

		ch->delay_info2 --;
		ch->delay = tch->delay + 1;
		act ("You continue to assist $N in hawking $S goods from the stall in $p.", false, ch, market_obj, tch, TO_CHAR | _ACT_FORMAT);
		return;
	}
	else
	{
		CHAR_DATA *rch = NULL;

		for (rch = ch->room->people; rch; rch = rch->next)
		{
			if (rch == tch || rch == ch)
			{
				continue;
			}
			if (rch->delay_type == DEL_TRADE_ASSIST && rch->delay_info3 == ch->delay_info3)
			{
				send_to_char("Someone is already providing that kind of assistance to a trader here.\n", ch);
				return;

			}
		}

		send_to_char("You assistance comes to an end.\n", ch);

		if (ch->delay_info3 == 1)
			skill_use(ch, SKILL_BRAWLING, 0);
		else if (ch->delay_info3 == 2)
			skill_use(ch, SKILL_HAGGLE, 0);
		else if (ch->delay_info3 == 3)
			skill_use(ch, SKILL_ARTISTRY, 0);
		else if (ch->delay_info3 == 4)
			skill_use(ch, SKILL_STEAL, 0);
		else if (ch->delay_info3 == 5)
			skill_use(ch, SKILL_EAVESDROP, 0);

	}	
}

void
	delayed_trade (CHAR_DATA *ch)
{
	OBJ_DATA *market_obj = NULL;
	OBJ_DATA *bundle_obj = NULL;

	for (market_obj = ch->room->contents; market_obj; market_obj = market_obj->next_content)
	{
		if (GET_ITEM_TYPE(market_obj) == ITEM_TRADE_MARKET)
		{
			break;
		}
	}

	if (!market_obj)
	{
		send_to_char("There don't see any market here from which you can trade.\n", ch);
		return;
	}

	if (ch->delay_info1 == 3)
	{
		char buf[MAX_STRING_LENGTH] = { '\0' };
		int eaves_skill = skill_level(ch, SKILL_EAVESDROP, 0);
		int eaves_thresh = 0;
		float market_sat[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

		if (eaves_skill > 50)
			eaves_thresh = 2;
		else if (eaves_skill > 25)
			eaves_thresh = 1;
		else
			eaves_thresh = 0;

		sprintf(buf, "From your observations, you learn...\n");

		for (int ind = 0; ind < 5; ind++)
		{
			int current_sat = market_obj->o.od.value[ind+1];
			int maximum_sat = vtoo(market_obj->nVirtual)->o.od.value[ind+1];
			market_sat[ind] = (current_sat * 1.0 / maximum_sat * 1.0);

			if (eaves_thresh == 2)
			{
				sprintf(buf + strlen(buf), "   there is a #6%s#0 of #6%s#0\n", 
					(market_sat[ind] >= 0.76 ? "severe shortage" :
					market_sat[ind] >= 0.61 ? "shortage" :
					market_sat[ind] >= 0.40 ? "balanced supply" :
					market_sat[ind] >= 0.25 ? "oversupply" : "complete glut"), trade_bundles[ind]);
			}
			else if (eaves_thresh == 1)
			{
				sprintf(buf + strlen(buf), "   there is a #6%s#0 of #6%s#0\n", 
					(market_sat[ind] >= 0.61 ? "shortage" : market_sat[ind] >= 0.40 ? "balanced supply" : "oversupply"), trade_bundles[ind]);
			}
			else
			{
				sprintf(buf + strlen(buf), "   there is a #6%s#0 of #6%s#0\n", 
					(market_sat[ind] >= 0.61 ? "imbalanced supply" : market_sat[ind] >= 0.40 ? "balanced supply" : "imbalanced supply"), trade_bundles[ind]);
			}
		}

		send_to_char(buf, ch);
		return;
	}
	else if (ch->delay_info2)
	{		
		if (!ch->delay_obj)
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TRADE_BUNDLE && ch->delay_obj == ch->right_hand)
		{
			bundle_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TRADE_BUNDLE && ch->delay_obj == ch->left_hand)
		{
			bundle_obj = ch->left_hand;
		}
		else
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		ch->delay_info2 --;
		ch->delay = 90;
		act ("You continue to hawk $p from your stall in $P.", false, ch, bundle_obj, market_obj, TO_CHAR | _ACT_FORMAT);
		return;
	}
	else
	{
		if (!ch->delay_obj)
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TRADE_BUNDLE && ch->delay_obj == ch->right_hand)
		{
			bundle_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TRADE_BUNDLE && ch->delay_obj == ch->left_hand)
		{
			bundle_obj = ch->left_hand;
		}
		else
		{
			send_to_char("You need to be holding a bundle of sorted goods to trade.\n", ch);
			return;
		}

		CHAR_DATA *tch = NULL;
		OBJ_DATA *money_obj = NULL;
		bool no_skill_sale = false;
		int base_price = 2;
		int current_price = 0;
		int multiplier = bundle_obj->o.od.value[1];
		bool sneak_pass = true;
		bool shout_pass = true;
		bool cloth_pass = true;
		bool thief_pass = true;

		if (!multiplier)
			multiplier = 1;

		int haggle_skill = skill_level(ch, SKILL_HAGGLE, 0);
		int skill_mod = 0;
		char buf[MAX_STRING_LENGTH] = { '\0' };
		char buf2[MAX_STRING_LENGTH] = { '\0' };
		char buf3[MAX_STRING_LENGTH] = { '\0' };

		if (haggle_skill >= 80)
		{
			skill_mod = 3;
		}
		else if (haggle_skill >= 60)
		{
			skill_mod = 2;
		}
		else if (haggle_skill >= 40)
		{
			skill_mod = 1;
		}
		else if (haggle_skill < 10)
		{
			if (!haggle_skill)
				skill_learn(ch, SKILL_HAGGLE);

			no_skill_sale = true;
			skill_mod = -1;
		}

		current_price = base_price + skill_mod;

		// add in bonuses for EQ here
		OBJ_DATA *eq = NULL;

		int filth_count = 0;

		// First of all, if your clothing is worn,
		// or got a condition, it costs you sales.
		// No one's buying stuff from a dirty, slime-covered
		// dude in ripped up shirts.

		for (eq = ch->equip; eq; eq = eq->next_content)
		{
			bool found_break;

			if (object__determine_condition(eq) >= 2)
			{
				filth_count ++;
				continue;
			}


			for (int ind = 0; ind < COND_TYPE_MAX; ind++)
			{
				if (eq->enviro_conds[ind] > 25)
				{
					filth_count ++;
					found_break = true;
					break;
				}
			}

			if (found_break)
				continue;
		}

		if (filth_count >= 4)
		{
			current_price -= 2;
			cloth_pass = false;
		}

		// add in saturation impacts here
		int bundle_type = bundle_obj->o.od.value[0];
		bundle_type ++;
		int current_sat = market_obj->o.od.value[bundle_type];
		int maximum_sat = vtoo(market_obj->nVirtual)->o.od.value[bundle_type];
		float saturation = (current_sat * 1.0 / maximum_sat * 1.0);
		int sat_bonus = 0;
		bool assistance = false;

		if (market_obj->o.od.value[bundle_type] < 1)
		{
			send_to_gods("marketset 1");
			market_obj->o.od.value[bundle_type] = 1;
		}

		if (saturation >= 0.76)
		{
			sat_bonus = 2;
			market_obj->o.od.value[bundle_type] -= 3;
		}
		else if (saturation >= 0.61)
		{
			sat_bonus = 1;
			market_obj->o.od.value[bundle_type] -= 2;
		}
		else if (saturation >= 0.40)
		{
			sat_bonus = 0;
			market_obj->o.od.value[bundle_type] -= 1;
		}
		else if (saturation >= 0.25)
		{
			sat_bonus = -1;
			market_obj->o.od.value[bundle_type] -= 1;
		}
		else
		{
			sat_bonus = -2;			
			market_obj->o.od.value[bundle_type] -= 1;
		}

		if (market_obj->o.od.value[bundle_type] < 1)
		{
			send_to_gods("marketset 2");
			market_obj->o.od.value[bundle_type] = 1;
		}

		current_price += sat_bonus;

		// add in play variations here
		if (ch->delay_info1 == 1)
		{
			if (skill_use(ch, SKILL_STEAL, 0))
			{
				current_price += 1;
			}
			else
			{
				current_price -= 1;
				sneak_pass = false;
			}
		}

		if (ch->delay_info1 == 2)
		{
			if (skill_use(ch, SKILL_HAGGLE, 0))
			{
				current_price += 1;
			}
			else
			{
				current_price -= 1;
				shout_pass = false;
			}
		}

		if (multiplier > 1)
		{
			float danger_chance = (multiplier - 1) * 10;

			if (number(1, 100) <= danger_chance)
			{
				thief_pass = false;

				int result = number(1,3);

				if (result == 1)
				{
					sprintf(buf2, "Your sales were also slightly hampered by a competiting businessman in the area. ");
				}
				else if (result == 2)
				{
					sprintf(buf2, "You notice the count of your profits is not quite right - a pickpocket must've been operating in the area. ");
				}
				else
				{
					sprintf(buf2, "Your sales were damaged by a vicious whispering campaign against the quality of your products. ");
				}

				current_price -= 1;
			}
		}

		current_price = current_price * multiplier;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;

			if (tch->delay_type == DEL_TRADE_ASSIST && tch->delay_info3)
			{
				int assist_skill = 0;				
				assistance = true;

				switch (tch->delay_info3)
				{					
				case 1:
					assist_skill = SKILL_BRAWLING;
					if (thief_pass)
					{
						current_price += multiplier;
						thief_pass = false;
					}
					sprintf(buf3 + strlen(buf3), "Throughout the trading, #5%s#0 provides a stern eye, ensuring nothing untowards occurs. ", char_short(tch));
					break;
				case 2:
					assist_skill = SKILL_HAGGLE;
					sprintf (buf3 + strlen(buf3), "With great gusto, #5%s#0 adds %s voice to spruik and cajole people, ", char_short(tch), HSHR(tch));
					break;
				case 3:
					assist_skill = SKILL_ARTISTRY;
					sprintf (buf3 + strlen(buf3), "Demonstrating passion and energy, #5%s#0 puts on a performance designed to lure customers closer, ", char_short(tch));
					break;
				case 4:
					assist_skill = SKILL_STEAL;
					sprintf (buf3 + strlen(buf3), "Skulking around the corners of the stall, #5%s#0 makes a move on the chip-pouches of the customers, ", char_short(tch));
					break;
				default:
					assist_skill = SKILL_EAVESDROP;
					sprintf (buf3 + strlen(buf3), "Engaging in a vicious whispering campaign against other sellers, #5%s#0 slinks through the market, ", char_short(tch));
					break;
				}


				if (tch->delay_info3 > 1)
				{
					int roll = number(1, skill_level(ch, assist_skill, 0));

					if (roll >= 40)
					{
						current_price += 2;
						sprintf (buf3 + strlen(buf3), "leading to a noticable increase in the chips made from the trading. ");
					}
					else if (roll >= 30)
					{
						current_price += 1;
						sprintf (buf3 + strlen(buf3), "causing to a slight increase in the chip earned. ");
					}
					else if (roll >= 15)
					{						
						sprintf (buf3 + strlen(buf3), "but the effect of all this isn't immediately obvious. ");
					}
					else if (roll >= 5)
					{
						current_price -= 1;
						sprintf (buf3 + strlen(buf3), "driving away a few customers with their lack of skill. ");
					}
					else
					{
						current_price -= 2;
						sprintf (buf3 + strlen(buf3), "turning away more than a few customers with their dismal performance. ");
					}
				}
			}

			char *p;
			reformat_string (buf3, &p);
			sprintf (buf3, "%s", p);
			strcat (buf3, "\n");
			mem_free (p); // char*
		}

		current_price = MAX(base_price * multiplier, current_price);
		current_price = MIN(current_price, 10 * multiplier);

		if (no_skill_sale)
			current_price = 1;

		if (assistance)
		{
			act (buf3, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act (buf3, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}

		sprintf(buf, "After a %s bout of hawking from your stall at $P, all of $p is gone. %s%s%s%sFor your troubles, you have made #2%d white, circular chip%s#0.", 
			(skill_mod >= 3 ? "fantasticaly successful" : skill_mod >= 2 ? "very successful" : skill_mod >= 1 ? "modestly successful" : skill_mod >= 0 ? "barely worthwhile" : "embarrasingly poor"),
			(sneak_pass ? "" : "However, from the lessened business that passed by your stall, you suspect customers passed on word on your shortchanging. "),
			(shout_pass ? "" : "However, from the lessened business that passed by your stall, you suspect your boisterous shouting repelled and not attrachted customers. "),
			(cloth_pass ? "" : "Sadly, the foul condition of your clothing lost you a number of prospective sales. "),
			(thief_pass ? "" : buf2),
			current_price, current_price > 1 ? "s" : "");

		act (buf, false, ch, bundle_obj, market_obj, TO_CHAR | _ACT_FORMAT);

		act ("$n finishes hawking $s wares from $p at a stall in $P.", false, ch, bundle_obj, market_obj, TO_ROOM | _ACT_FORMAT);


		skill_use(ch, SKILL_HAGGLE, 0);

		obj_from_char (&bundle_obj, 1);
		extract_obj(bundle_obj);

		if ((money_obj = load_object(14011)))
		{
			money_obj->count = current_price;
			obj_to_char(money_obj, ch);
		}
	}	
}


// rent begin: gives you a token if you don't have one already.
// rent topup: tops up your token.
// rent access: get let in.
// rent balance: see how much time you've got left.
// rent replace: get a new key, and make the old one defunct.
// rent duplicate: get another key produced.

#define SHORT_ON_RENT		"You haven't got enough for your rent. Don't bother talking to me again until you do."
#define RENT_PROBLEM		"We're not open for business just now - come back later."
#define RENT_COMMENCED		"A pleasure doing business with you. To see how much rent you have left, type #6rent balance#0. Here's your key. To visit your room, type #6rent access#0 - you'll need your key handy to do so."
#define RENT_TOPPEDUP		"Ah, thank you very much. That's one more full Lunar day added to your rent balance."
#define ALREADY_RENTING		"But you're already renting from us - one lease per tennant."
#define NOT_RENTING			"But we don't have you on our records as renting here - just because you have the key doesn't prove anything."
#define MISSING_KEY			"You'll need to show me some kind of key if you want to do that."
#define MISSING_RENT		"Nobody is getting up in to that room until I see some rent - the owner will need to #6topup#0."
#define GO_ON_UP			"Welcome back, go right up to your room."
#define ROOM_PROBLEM		"We don't have any such room on our records here - are you sure that's the right pass?"
#define RENT_REPLACED		"Here's your new key. If you lose it again, it will cost you again."
#define KEY_PROBLEM		    "You aren't holding any key that I recognise."
#define RENT_REPLICATED	    "Here's another copy of the key. Be sure to keep track of both."

void
	do_rent(CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *office_obj = NULL;
	OBJ_DATA *key_obj = NULL;
	CHAR_DATA *landlord_mob = NULL;
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	ROOM_DATA *entry_room = NULL;
	std::string world_log_db = engine.get_config ("world_log_db");
	std::string strArgument = argument;
	int room_template;
	int room_destination;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};

	for (office_obj = ch->room->contents; office_obj; office_obj = office_obj->next_content)
	{
		if (GET_ITEM_TYPE(office_obj) == ITEM_RENT_SHOP)
		{
			break;
		}
	}
	if (!office_obj)
	{
		send_to_char("You can't do that here - look for a landlord and his office.\n", ch);
		return;
	}
	for (landlord_mob = ch->room->people; landlord_mob; landlord_mob = landlord_mob->next_in_room)
	{
		if (IS_NPC(landlord_mob) && landlord_mob->mob->vnum == office_obj->o.od.value[3])
		{
			break;
		}
	}

	if (!landlord_mob)
	{
		send_to_char ("There is no landlord in sight.\n", ch);
		return;
	}

	int rent = office_obj->o.od.value[0];	
	int balance = 0;

	if (cmd == 1)
	{
		if (!can_subtract_money (ch, rent, landlord_mob->mob->currency_type))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, SHORT_ON_RENT);
			do_whisper (landlord_mob, buf, 83);
			return;
		}

		if (!(key_obj = load_object(office_obj->o.od.value[2])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (!(vnum_to_room(office_obj->o.od.value[1])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		room_template = office_obj->o.od.value[1];

		if (!(room_destination = clone_contiguous_rblock (vnum_to_room (room_template), -1, true)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if ((entry_room = vnum_to_room(room_destination)))
		{
			CREATE (entry_room->dir_option[OUTSIDE], struct room_direction_data, 1);
			entry_room->dir_option[OUTSIDE]->general_description = str_dup ("");
			entry_room->dir_option[OUTSIDE]->keyword = str_dup ("entryway");
			entry_room->dir_option[OUTSIDE]->exit_info |= (1 << 0);
			entry_room->dir_option[OUTSIDE]->key = 0;
			entry_room->dir_option[OUTSIDE]->to_room = landlord_mob->in_room;
			entry_room->dir_option[OUTSIDE]->pick_penalty = 0;
		}

		subtract_money (ch, rent, landlord_mob->mob->currency_type);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, RENT_COMMENCED);
		do_whisper (landlord_mob, buf3, 83);		
		key_obj->o.od.value[0] = room_destination;
		obj_to_char(key_obj, ch);

		act ("$N hands $p to you.", false, ch, key_obj, landlord_mob, TO_CHAR | _ACT_FORMAT);
		act ("$N hands $p to $n.", false, ch, key_obj, landlord_mob, TO_ROOM | _ACT_FORMAT);

		key_obj->o.od.value[2] = key_obj->coldload_id;
		mysql_safe_query ("INSERT INTO %s.rent (key_coldload, agency_vnum, pc_coldload, pc_short, room_coldload, balance) VALUES (%d, %d, %d, '%s', %d, 29)", world_log_db.c_str(), key_obj->o.od.value[2], office_obj->nVirtual, ch->coldload_id, char_short(ch), room_destination);		
		ch->delay_type = 0;
		return;		
	}
	else if (cmd == 3)
	{
		if (!can_subtract_money (ch, rent/2, landlord_mob->mob->currency_type))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, SHORT_ON_RENT);
			do_whisper (landlord_mob, buf, 83);
			return;
		}

		if (!(key_obj = load_object(office_obj->o.od.value[2])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT room_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		key_obj->o.od.value[0] = atoi(row[0]);

		if ( result )
			mysql_free_result (result);

		subtract_money (ch, rent/2, landlord_mob->mob->currency_type);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, RENT_REPLACED);
		do_whisper (landlord_mob, buf3, 83);
		obj_to_char(key_obj, ch);

		act ("$N hands $p to you.", false, ch, key_obj, landlord_mob, TO_CHAR | _ACT_FORMAT);
		act ("$N hands $p to $n.", false, ch, key_obj, landlord_mob, TO_ROOM | _ACT_FORMAT);

		key_obj->o.od.value[2] = key_obj->coldload_id;
		mysql_safe_query ("UPDATE %s.rent SET key_coldload = %d WHERE agency_vnum = %d AND pc_coldload = %d", world_log_db.c_str (), key_obj->o.od.value[2], office_obj->nVirtual, ch->coldload_id);

		ch->delay_type = 0;
		return;
	}
	else if (cmd == 4)
	{
		if (!can_subtract_money (ch, (rent/2), landlord_mob->mob->currency_type))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, SHORT_ON_RENT);
			do_whisper (landlord_mob, buf, 83);
			return;
		}

		if (!(key_obj = load_object(office_obj->o.od.value[2])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT room_coldload, key_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		key_obj->o.od.value[0] = atoi(row[0]);
		key_obj->o.od.value[2] = atoi(row[1]);

		if ( result )
			mysql_free_result (result);

		subtract_money (ch, (rent/2), landlord_mob->mob->currency_type);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, RENT_REPLICATED);
		do_whisper (landlord_mob, buf3, 83);
		obj_to_char(key_obj, ch);

		act ("$N hands $p to you.", false, ch, key_obj, landlord_mob, TO_CHAR | _ACT_FORMAT);
		act ("$N hands $p to $n.", false, ch, key_obj, landlord_mob, TO_ROOM | _ACT_FORMAT);

		ch->delay_type = 0;
		return;
	}
	else if (cmd == 2)
	{
		if (!can_subtract_money (ch, rent, landlord_mob->mob->currency_type))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, SHORT_ON_RENT);
			do_whisper (landlord_mob, buf, 83);
			return;
		}

		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->left_hand;
		}

		if (!key_obj)
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_KEY);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		subtract_money (ch, rent, landlord_mob->mob->currency_type);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, RENT_TOPPEDUP);
		do_whisper (landlord_mob, buf3, 83);

		mysql_safe_query ("UPDATE %s.rent SET balance=balance+29 WHERE agency_vnum = %d AND pc_coldload = %d AND key_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id, key_obj->o.od.value[2]);
		ch->delay_type = 0;
		return;
	}

	if (strArgument == "commence")
	{			
		if (!(vtoo(office_obj->o.od.value[2])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (!(vnum_to_room(office_obj->o.od.value[1])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT pc_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id);

		result = mysql_store_result (database);

		if (result && (row = mysql_fetch_row (result)))
		{
			if (ch->coldload_id == atoi(row[0]))
			{
				if ( result )
					mysql_free_result (result);
				name_to_ident (ch, buf2);
				sprintf (buf3, "%s %s", buf2, ALREADY_RENTING);
				do_whisper (landlord_mob, buf3, 83);
				return;
			}
		}

		if ( result )
			mysql_free_result (result);

		if (office_obj->o.od.value[4] > 0)
		{
			sprintf (buf,	"You can have the room for a full Lunar day #6[OOC: 7 Real-Life Days]#0 for #2%d credits#0."
				"At any time, you can pay the same amount for another Lunar day. You fall behind, you'll need to pay your "
				"back rent before I let you back in to your place. I'll allow you to take multiple people in to the room, but you will only get one key.", rent);
		}
		else
		{
			sprintf (buf,	"You can have the room for a full Lunar day #6[OOC: 7 Real-Life Days]#0 for #2%d credits#0."
				"At any time, you can pay the same amount for another Lunar day. You fall behind, you'll need to pay your "
				"back rent before I let you back in to your place. This room has space enough for only one person at any time.", rent);
		}

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (landlord_mob, buf3, 83);

		send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to commence your renting.\n", ch);

		ch->delay_type = DEL_RENT_COMMENCE;
		return;
	}
	else if (strArgument == "topup")
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->left_hand;
		}

		if (!key_obj)
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_KEY);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT pc_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d AND key_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id, key_obj->o.od.value[2]);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (ch->coldload_id != atoi(row[0]))
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, NOT_RENTING);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if ( result )
			mysql_free_result (result);

		sprintf (buf,	"You can have the room for another full Lunar day #6[OOC: 7 Real-Life Days]#0 for #2%d credits#0. "
			"You fall behind, you'll need to pay your back rent before I let you back in to your place.", rent);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (landlord_mob, buf3, 83);

		send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to top up your rent balance.\n", ch);

		ch->delay_type = DEL_RENT_TOPUP;
		return;
	}
	else if (strArgument == "replicate")
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->left_hand;
		}

		if (!key_obj)
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_KEY);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT pc_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d AND key_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id, key_obj->o.od.value[2]);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (ch->coldload_id != atoi(row[0]))
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, NOT_RENTING);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if ( result )
			mysql_free_result (result);

		sprintf (buf,	"You can have a duplicate key produced for #2%d credits#0. Both keys will work, but only the original owner will be able to topup, replicate, and replace.", (rent / 2));

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (landlord_mob, buf3, 83);

		send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to duplicate your key.\n", ch);

		ch->delay_type = DEL_RENT_REPLICATE;
		return;
	}
	else if (strArgument == "replace")
	{
		mysql_safe_query ("SELECT pc_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (ch->coldload_id != atoi(row[0]))
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, NOT_RENTING);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if ( result )
			mysql_free_result (result);

		sprintf (buf,	"You can have a replacement key for a half Lunar day's worth of rent, or, #2%d credits#0. "
			"This doesn't increase your rent balance, and your old key will become invalid.", rent/2);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (landlord_mob, buf3, 83);

		send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to replace your key.\n", ch);

		ch->delay_type = DEL_RENT_REPLACE;
		return;
	}

	else if (strArgument == "balance")
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->left_hand;
		}

		if (!key_obj)
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_KEY);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT balance, pc_coldload FROM %s.rent WHERE agency_vnum = %d AND pc_coldload = %d AND key_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, ch->coldload_id, key_obj->o.od.value[2]);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, RENT_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if (ch->coldload_id != atoi(row[1]))
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, NOT_RENTING);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		balance = atoi(row[0]);

		if ( result )
			mysql_free_result (result);

		if (balance > 4)
		{
			sprintf (buf,	"You have some balance - I believe you would have at least #6%d Real Life Day's#0 worth of rent left.", balance / 4);
		}
		else if (balance <= 0)
		{
			sprintf (buf,	"You are behind on the rent by at least #6%d Real Life Day's#0. You have to #6topup#0 your rent before I allow you back in your room.", -balance / 4);
		}
		else
		{
			sprintf (buf,	"You have less than #6a Real Life Day's#0 worth of rent. You're going to need to #6topup#0 your rent soon, or be locked out.");
		}

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (landlord_mob, buf3, 83);

		return;
	}
	else if (strArgument == "access")
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->right_hand;
		}
		else if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_RENT_TICKET)
		{
			key_obj = ch->left_hand;
		}

		if (!key_obj || key_obj->o.od.value[1] != office_obj->nVirtual)
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_KEY);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		mysql_safe_query ("SELECT balance FROM %s.rent WHERE agency_vnum = %d AND key_coldload = %d", world_log_db.c_str (), office_obj->nVirtual, key_obj->o.od.value[2]);

		result = mysql_store_result (database);

		if (!result || !(row = mysql_fetch_row (result)) )
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, KEY_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		balance = atoi(row[0]);

		if (balance <= 0)
		{
			if ( result )
				mysql_free_result (result);
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, MISSING_RENT);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		if ( result )
			mysql_free_result (result);

		if (!key_obj->o.od.value[0] || !(vnum_to_room(key_obj->o.od.value[0])))
		{
			name_to_ident (ch, buf2);
			sprintf (buf3, "%s %s", buf2, ROOM_PROBLEM);
			do_whisper (landlord_mob, buf3, 83);
			return;
		}

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, GO_ON_UP);
		do_whisper (landlord_mob, buf3, 83);

		if (office_obj->o.od.value[4] > 0)
		{
			act ("Using $p, you and those following you enter your room.", false, ch, key_obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("Using $p, $n and those following $m enters $s room.", false, ch, key_obj, 0, TO_ROOM | _ACT_FORMAT); 

			int queue = 0;
			CHAR_DATA * transfer_queue [128]; // can trans up to 128 people
			CHAR_DATA * tmp_ch = ch->room->people;
			for (; tmp_ch && queue < 124; tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch->following == ch || tmp_ch == ch)
				{
					if (tmp_ch->mount)
					{
						transfer_queue[queue++] = tmp_ch->mount;
					}
					if (IS_SUBDUER(tmp_ch))
					{
						transfer_queue[queue++] = tmp_ch->subdue;
					}
					if (IS_HITCHER(tmp_ch))
					{
						transfer_queue[queue++] = tmp_ch->hitchee;
					}
					transfer_queue[queue++] = tmp_ch;
				}
			}

			// move them
			do
			{
				char_from_room (transfer_queue[--queue]);
				char_to_room (transfer_queue[queue], key_obj->o.od.value[0]);
				do_look (transfer_queue[queue],"",77);
			}
			while (queue);

			act ("$n's group enters the room.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("Using $p, you enter your room.", false, ch, key_obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("Using $p, $n enters $s room.", false, ch, key_obj, 0, TO_ROOM | _ACT_FORMAT); 
			char_from_room(ch);
			char_to_room(ch, key_obj->o.od.value[0]);
			act ("$n enters the room.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			do_look (ch, "", 77);
		}
		return;
	}
	else
	{
		sprintf(buf,               "rent usage: rent <commence|topup|balance|access>\n");
		sprintf(buf + strlen(buf), "rent commence  - take out a Lunar day-to-day lease on a room\n");
		sprintf(buf + strlen(buf), "rent topup     - add another Lunar day's worth of rent to your balance\n");
		sprintf(buf + strlen(buf), "rent balance   - check how much balance you have owing\n");
		sprintf(buf + strlen(buf), "rent access    - access your room\n");
		sprintf(buf + strlen(buf), "rent replace   - replaces your key (your old key will no longer function)\n");
		sprintf(buf + strlen(buf), "rent replicate - produces another key (your old key will still longer function)\n");
		//sprintf(buf + strlen(buf), "trade gossip   - use your eavesdrop skill to find out whats been sold\n");

		send_to_char(buf, ch);
		return;
	}
}

void
	rent_update (void)
{
	int true_hour = time_info.hour + (84 * time_info.day);	

	// Deduct some rent.
	if (!(true_hour%24))
	{
		std::string world_log_db = engine.get_config ("world_log_db");
		send_to_gods("Rent deducted.");
		mysql_safe_query ("UPDATE %s.rent SET balance=balance-1 WHERE balance > -60", world_log_db.c_str ());

		// We now rip out the objects of everything with a balance equal to or less than -60,
		// as your landlord has now taken this to repay him for his time and effort, and to
		// remove dead objects from the game.
		MYSQL_RES *result = NULL;
		MYSQL_ROW row;

		mysql_safe_query ("SELECT * FROM %s.rent WHERE balance <= -60", world_log_db.c_str ());

		if (!(result = mysql_store_result (database)))
			return;

		int room_vnum;
		int id;
		OBJ_DATA *obj = NULL;
		ROOM_DATA *room = NULL;

		while ((row = mysql_fetch_row(result)))
		{
			if (row[0] && *row[0])
				id = atoi(row[0]);

			if (row[5] && *row[5])
				room_vnum = atoi(row[5]);

			if ((room = vnum_to_room(room_vnum)))
			{
				if ((obj = room->contents))
				{
					for (obj = room->contents; obj; obj = obj->next_content)
					{
						extract_obj(obj);
					}
					mysql_safe_query ("UPDATE %s.rent SET balance = 0 WHERE id = %d", world_log_db.c_str (), id);
				}
				else
				{
					mysql_safe_query ("DELETE FROM %s.rent WHERE id = %d", world_log_db.c_str (), id);
					delete_contiguous_temporary_rblock (vnum_to_room (room_vnum), -1, -1);
				}
			}
		}		
	}
}
