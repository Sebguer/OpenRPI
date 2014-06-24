/*------------------------------------------------------------------------\
 |  object_damage.c : Obejct Damage Class             www.middle-earth.us  |
 |  Copyright (C) 2005, Shadows of Isildur: Sighentist                     |
 |  Derived under license from DIKU GAMMA (0.0).                           |
 \------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "object_damage.h"
#include "utility.h"


extern rpie::server engine;


/*------------------------------------------------------------------------\
 |                                                                         |
 |  STRING STORAGE                                                         |
 |                                                                         |
 \------------------------------------------------------------------------*/

const char *enviro_desc[COND_TYPE_MAX][6] = {
    {"stains", "faintly stained", "stained", "stain-covered", "stain-ridden", "cleaning"},
    {"dirt",  "slightly dirty", "dirty", "dirt-covered", "dirt-encrusted", "cleaning"},
    {"dust",  "a little dusty", "dusty", "dust-covered", "dust-stricken", "wiping"},
    {"blood",  "spotted with blood", "bloody", "blood-covered", "blood-drenched", "cleaning"},
    {"water",  "damp", "wet", "soaking", "satured", "squeezing"},
    {"filth",  "a little filthy", "filthy", "filth-covered", "filth-ridden", "cleaning"},
    {"slime",  "somewhat slimy", "slimy", "slime-covered", "slime-covered", "wiping"},
    {"rust",  "starting to rust", "rusting", "rusted", "rust-covered", "scouring"}
};



const char *damage_type[DAMAGE_TYPE_MAX + 1] = {
	"pierce",
	"blunt",
	"slash",
	"freeze",			/*  5 */
	"burn",
	"fist",
	"blood",			/* 10 */
	"water",
	"permanent",			/* 13 */
	"repair",
	"\n"
};

const char *material_type[MATERIAL_TYPE_MAX + 1] = {
	"undefined",
	"organic",
	"textile",
	"metal",
	"ceramic",
	"glass",
	"electronic",
	"plastics",
	"\n"
};


const char *damage_severity[DAMAGE_SEVERITY_MAX + 1] = {
	"small",
	"minor",
	"moderate",
	"large",
	"huge",
	"massive",
	"\n"
};

const char *conditions[8][6] =
{
	//Generic
	{
		"It is in good condition",              // fine
		"It is rather worn",                    // worn
		"It is in poor condition",              // damaged
		"It is quite damaged",                  // damaged
		"It is very damaged",                   // ruined
        "It is completely destroyed"            // ruined
	},
	//organic
	{
		"It is in good condition, with no flaws present",
		"It has seen some damage, battered slightly",
		"It has been noticably damaged, tears running up and down its length",
		"This item is battered and half-broken",
		"Beaten and battered, this object has seen better days",
		"Pummeled, battered, and otherwise destroyed, this item is barely held together"
	},
	//textile
	{
		"It is in good condition, holding together perfectly",
		"It has a few minor tears, but nothing serious",
		"Rips and tears are noticable, but the item holds together",
		"This item bears serious damage, ripped and shredded throughout",
		"The item has start to come apart at the seams with gashes and holes",
		"Shredded and torn, this item is nearly destroyed"
	},
	//metal
	{
		"It is in good condition, completely free of rust and with few blemishes",
		"It has seen some usage, showing a little rust and scuffing",
		"It bears an amount of rust and sections beaten out of place",
		"It is beaten, misshapen and rusted with time and abuse",
		"Rust, wear, tears and punctures dominate this item",
		"That this item is even recognisable is a miracle in of itself"
	},
	//ceramic
	{
		"It is in good condition, whole and complete",
		"Minor scuffs and small cracks can be seen",
		"No small amount of scuffs are present, along with cracks and chip",
		"Cracks and chips cover this items surface",
		"Cracks are abundant, and a chunk or two is missing",
		"Chunks are broken away from this object, nearly compromising its integrity"
	},
	//glas
	{
		"It is in good condition, nearly luminescent in its wholeness",
		"Small scratches dot its surface occasionally",
		"A crack or two can be spotted on the surface of this item",
		"Cracks spider-web across the surface of this object, abundant",
		"Bits of this object are broken away, and nowhere can a crack not be seen",
		"Where this item isn't broken it is shattered, a spiderweb of cracks running over its nearly-broken surface"
	},
	//electronic
	{
		"It is in good condition, unblemished and unbroken",
		"Small scuffs and abrasions dot this item's surface intermittently",
		"Small dents are noticeable, alongside scuffs and scrapes",
		"The item is battered, dented, and cracked",
		"Dents and cracks and breaches adorn this item's suface",
		"Cracked and dented and broken, it's a miracle that this item survives"
	},
	//plastics
	{
		"It is in good condition, intact and full",
		"This item is barely scraped, but blemishes are apparent",
		"Scrapes and scuffs mar the surace of this object",
		"Tears run over the surface of this item, warping it",
		"The item is worn and torn, mishapen and warped",
		"Warped and torn, this item is hardly recognizable as what it once was"
	}
};

const char *scuffs_desc[6] =
{
	"\n",
	" and has a few small damages",
	" and has a number of small damages",
	" and has lots of small damages",
	" and bears scores small damages",
	" and badly suffers from hundreds of small damages"
};

const char *worn_desc[6] =
{
	"\n",
	". It is not as new as it once was",
	". It is beginning to show its age",
	". It has clearly seen a lot of use",
	". It has seen more than its fair share of use",
	". It is beyond any repair"
};

short skill_to_damage_name_index (ushort n_skill);

/*------------------------------------------------------------------------\
 |  new()                                                                  |
 |                                                                         |
 |  Returns a pointer to the newly allocated instance of object_damage.    |
 \------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new ()
{
	OBJECT_DAMAGE *thisPtr = NULL;

	/* TODO: Remove this when we're ready to go live with damage */
	if (engine.in_build_mode ())
		return NULL;

	CREATE (thisPtr, OBJECT_DAMAGE, 1);

	thisPtr->source = (DAMAGE_TYPE) 0;
	thisPtr->material = (MATERIAL_TYPE) 0;
	thisPtr->severity = (DAMAGE_SEVERITY) 0;
	thisPtr->impact = 0;
	thisPtr->name = 0;
	thisPtr->permanent = 0;
	thisPtr->when = 0;


	return thisPtr;
}


/*------------------------------------------------------------------------\
 |  new_init()                                                             |
 |                                                                         |
 |  Returns a pointer to the newly allocated and initialized instance of   |
 |  object damage.                                                         |
 \------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new_init (DAMAGE_TYPE source, int impact,
                         MATERIAL_TYPE material, int permanent, int cmd)
{
	OBJECT_DAMAGE *thisPtr = NULL;
	int mod = 0;

	// A rough table. Negatives do less damage, positives do more, i.e. slashing is very effective against textiles, but blunt damage is ineffective.

	int material_v_source[8][10] =
	{
		//pierce | blunt | slash | freeze | burn | fist | blood | water | permanent
        //              p   b   s   fr  bu  fi  bl  wa  pe  bul
		/*undefined*/ { 0,  0,  0,  0,  0, -4,  0,  0,  0,  0},
		/*organic*/   { 0,  0,  4,  0,  0, -4,  0,  0,  0,  0},
		/*textile*/   { 0, -4,  4, -2,  2, -4,  0,  0,  0,  0},
		/*metal*/     {-2,  2, -4,  0,  0, -4,  0,  0,  0,  4},
		/*ceramic*/   {-2,  2, -2,  0,  0, -4,  0,  0,  0,  2},
		/*glass*/     { 2,  4,  0,  0,  0, -4,  0,  0,  0,  6},
		/*electronic*/{ 2,  0,  0,  4,  4, -4,  0,  0,  0,  0},
		/*plastic*/   { 0,  0, -2, -4,  4, -4,  0,  0,  0,  0}
	};

	if (!(thisPtr = object_damage__new ()))
	{
	  return NULL;
	}

	mod = material_v_source[material][source];

	// We do some hokey-pokey to avoid a 1 point tear becoming an 11 point tear.

	if (mod <= 0)
		impact += mod;
	else if (impact > mod)
		impact += mod;
	else if (impact > (mod/2))
		impact += (mod/2);

	if (impact <= 0)
		return NULL;

    if (cmd)
        impact = impact * 2;

	if (impact <= 6)
		thisPtr->severity = (DAMAGE_SEVERITY) 0;
	else if (impact <= 10)
		thisPtr->severity = (DAMAGE_SEVERITY) 1;
	else if (impact <= 19)
		thisPtr->severity = (DAMAGE_SEVERITY) 2;
	else if (impact <= 29)
		thisPtr->severity = (DAMAGE_SEVERITY) 3;
	else if (impact <= 39)
		thisPtr->severity = (DAMAGE_SEVERITY) 4;
	else if (impact >= 40)
		thisPtr->severity = (DAMAGE_SEVERITY) 5;

	if (impact > 50)
		impact = 50;

	thisPtr->source = source;
	thisPtr->material = material;
	thisPtr->impact = (source == DAMAGE_BLOOD) ? 0 : impact;
	thisPtr->name = number (0, 3);
	thisPtr->permanent = permanent;
	thisPtr->when = time (0);

	//char buf[MAX_STRING_LENGTH];
	//sprintf(buf, "Source: %s, Material: %s, Damage: %d", damage_type[source], material_type[material], impact);
	//send_to_gods(buf);
	return thisPtr;
}


/*------------------------------------------------------------------------\
 |  delete()                                                               |
 |                                                                         |
 |  Frees the memory of this object damage instance and returns the next.  |
 \------------------------------------------------------------------------*/
void
free_damage (OBJECT_DAMAGE * damage)
{
	if (!damage)
		return;

	mem_free (damage);
}

int
has_enviro_conds (OBJ_DATA *obj)
{
    if (!obj)
        return false;

    for (int i = 0; i < COND_TYPE_MAX; i++)
    {
        if (obj->enviro_conds[i] > 10)
            return true;
    }

    return false;
}


void
object_damage__delete (OBJ_DATA *obj, OBJECT_DAMAGE *damage)
{
	OBJECT_DAMAGE *tempdamage = NULL;

	if (!obj || !damage)
		return;

	if (obj->damage == damage)
		obj->damage = obj->damage->next;
	else
	{
		for (tempdamage = obj->damage; tempdamage; tempdamage = tempdamage->next)
			if (tempdamage->next == damage)
			tempdamage->next = tempdamage->next->next;
	}

	free_damage(damage);

}

/*------------------------------------------------------------------------\
 |  get_sdescr()                                                            |
 |                                                                         |
 |  Returns a short description of the damage instance.                    |
 \------------------------------------------------------------------------*/
char *
object_damage__get_sdesc (OBJECT_DAMAGE * thisPtr)
{
	ushort n_sdesc_length = 0;
	char *str_sdesc = NULL;
	extern const char *damage_name[8][8][4];

	//if (thisPtr->source == DAMAGE_PERMANENT)
	//	return NULL;

	n_sdesc_length = strlen (thisPtr->permanent ? "permanent" : damage_severity[thisPtr->severity]) +
		strlen (damage_name[thisPtr->source][thisPtr->material][thisPtr->name]) + 5;
	str_sdesc = (char *) alloc (sizeof (char) * n_sdesc_length, 0);
	sprintf (str_sdesc, "%s %s %s",
	         (isvowel (damage_severity[thisPtr->severity][0])) ? "an" : "a",
	         thisPtr->permanent ? "permanent" : damage_severity[thisPtr->severity],
	         (damage_name[thisPtr->source][thisPtr->material][thisPtr->name]));
	return str_sdesc;
}


/*------------------------------------------------------------------------\
 |  write_to_file()                                                        |
 |                                                                         |
 |  Export a string that describes this damage instance.                   |
 \------------------------------------------------------------------------*/
int
object_damage__write_to_file (OBJECT_DAMAGE * thisPtr, FILE * fp)
{
	return fprintf (fp, "Damage     %d %d %d %d %d %d %d\n",
	                thisPtr->source, thisPtr->material,
	                thisPtr->severity, thisPtr->impact,
	                thisPtr->name, thisPtr->permanent, thisPtr->when);
}


/*------------------------------------------------------------------------\
 |  read_from_file()                                                       |
 |                                                                         |
 |  Import a string that describes a damage instance.                      |
 \------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__read_from_file (FILE * fp)
{
	OBJECT_DAMAGE *thisPtr = NULL;

	if (!(thisPtr = object_damage__new ()))
	{
		return NULL;
	}

	/* TODO: Make this an fscanf statement? */
	thisPtr->source = (DAMAGE_TYPE) fread_number (fp);
	thisPtr->material = (MATERIAL_TYPE) fread_number (fp);
	thisPtr->severity = (DAMAGE_SEVERITY) fread_number (fp);
	thisPtr->impact = fread_number (fp);
	thisPtr->name = fread_number (fp);
	thisPtr->permanent = fread_number (fp);
	thisPtr->when = fread_number (fp);

	return thisPtr;
}

void
npc_repair (CHAR_DATA * ch, CHAR_DATA * mob, OBJ_DATA *obj, char *argument)
{
	OBJECT_DAMAGE *damage;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	int small = false;
	float cost = 0;
	extern const char *damage_name[8][8][4];

	if (!mob || !IS_SET (mob->act, ACT_FIXER))
	{
		send_to_char ("I don't see a physician here.\n", ch);
		return;
	}

	if (mob->delay)
	{
		act ("$n appears to be busy.", true, ch, 0, mob, TO_CHAR | _ACT_FORMAT);
		return;
	}

	argument = one_argument (argument, buf2);

	if (!*buf2)
	{
		send_to_char ("How did you want your item repaired?\n", ch);
		return;
	}

	name_to_ident (ch, buf3);
	if (!obj->damage)
	{
		sprintf (buf, "whisper %s I don't see any damage on #2%s#0 to repair!", buf3, obj_short_desc(obj));
		command_interpreter (mob, buf);
		return;
	}

	if (!str_cmp (buf2, "value"))
	{
		if (!*argument)
		{
			send_to_char
				("For what damage did you wish to get an appraisal?\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		argument = one_argument (argument, buf2);
		if (!strn_cmp (buf, "all_damage", strlen (buf)))
		{
			for (damage = obj->damage; damage; damage = damage->next)
			{
				if (damage->permanent)
					continue;
				if (damage->next)
					cost += (damage->impact + damage->severity) * 0.45;
				else
					cost += (damage->impact + damage->severity) * 0.55;
			}
			if (mob->shop)
				cost *= mob->shop->markup;
		}
		else if (!strn_cmp (buf, "small_damage", strlen (buf)))
		{
			for (damage = obj->damage; damage; damage = damage->next)
			{
				if (damage->severity)
					continue;
				if (damage->next)
					cost += damage->impact * 0.45;
				else
					cost += damage->impact * 0.55;
			}
			if (mob->shop)
				cost *= mob->shop->markup;

			small = true;
		}
		else
		{
			for (damage = obj->damage; damage; damage = damage->next)
			{
				if (!str_cmp (damage_severity[damage->severity], buf) && !str_cmp (damage_name[damage->source][damage->material][damage->name], buf2))
				{
					cost += (damage->impact + damage->severity) * 0.60;
					if (mob->shop)
						cost *= mob->shop->markup;
					break;
				}
			}
			if (!damage)
			{
				sprintf (buf, "whisper %s  I don't see any damage there to treat.",
				         buf2);
				command_interpreter (mob, buf);
				return;
			}
			else if (damage->permanent)
			{
				sprintf (buf, "whisper %s That damage is beyond repair.",
				         buf2);
				command_interpreter (mob, buf);
				return;
			}
		}

		if (cost < 1 && cost > 0)
			cost = 1;

		if (cost < 1)
		{
			sprintf (buf,
			         "whisper %s All the damage have been taken care of - there's nothing I can do.",
			         buf3);
			command_interpreter (mob, buf);
			return;
		}

		sprintf (buf,
		         "whisper %s I'll get you taken care of for a total of %d credits.",
		         buf3, (int) cost);
		command_interpreter (mob, buf);
		return;
	}

	argument = one_argument (argument, buf);

	if (!strn_cmp (buf2, "all_damage", strlen (buf2)))
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (damage->permanent)
				continue;
			if (damage->next)
				cost += (damage->impact + damage->severity) * 0.45;
			else
				cost += (damage->impact + damage->severity) * 0.55;
		}
		if (mob->shop)
			cost *= mob->shop->markup;
	}
	else if (!strn_cmp (buf2, "small_damage", strlen (buf2)))
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (damage->severity)
				continue;
			if (damage->next)
				cost += damage->impact * 0.45;
			else
				cost += damage->impact * 0.55;
		}
		if (mob->shop)
			cost *= mob->shop->markup;

		small = true;

	}
	else
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (!str_cmp (damage_severity[damage->severity], buf2) && !str_cmp (damage_name[damage->source][damage->material][damage->name], buf))
			{
				cost += (damage->impact + damage->severity) * 0.60;
				if (mob->shop)
					cost *= mob->shop->markup;
				break;
			}
		}
		if (!damage)
		{
			sprintf (buf, "whisper %s I don't see any damage there to treat.", buf3);
			command_interpreter (mob, buf);
			return;
		}
		else if (damage->permanent)
		{
			sprintf (buf, "whisper %s That damage is beyond repair.", buf3);
			command_interpreter (mob, buf);
			return;
		}
	}

	if (cost < 1 && cost > 0)
		cost = 1;

	if (cost < 1)
	{
		sprintf (buf,
		         "whisper %s All the damage have been taken care of - there's nothing I can do.",
		         buf3);
		command_interpreter (mob, buf);
		return;
	}

	if (!is_brother (ch, mob))
	{

		if (!can_subtract_money (ch, (int) cost, mob->mob->currency_type))
		{
			sprintf (buf, "%s You seem to be a little short on credits to trade.", buf3);
			do_whisper (mob, buf, 83);
			return;
		}

		subtract_money (ch, (int) cost, mob->mob->currency_type);
		if (mob->shop && mob->shop->store_vnum)
			money_to_storeroom (mob, (int) cost);

		send_to_char ("\n", ch);
	}
	else
	{
		sprintf (buf, "whisper %s There is no cost to you for this treatment.",
		         buf3);
		command_interpreter (mob, buf);
	}

	act ("$N promptly repairs the damage on $p.", true, ch, obj, mob,
	     TO_CHAR | _ACT_FORMAT);
	act ("$N promptly repairs the damage of $n's $p.", true, ch, obj, mob,
	     TO_ROOM | _ACT_FORMAT);

	if (damage)
	{
		object_damage__delete(obj, damage);
	}
	else
		for (damage = obj->damage; damage; damage = damage->next)
	{
		if (damage->permanent)
			continue;

		if (small && (damage->severity > DAMAGE_SMALL))
			continue;

		if (damage->severity == DAMAGE_MASSIVE)
		{
			damage->permanent = 1;
			damage->impact = (int) damage->impact / 2;
		}
		else
			object_damage__delete(obj, damage);
	}
}


short
skill_to_damage_name_index (ushort n_skill)
{
	switch (n_skill)
	{

		/*    case SKILL_TEXTILECRAFT:
		 return 0;
		 case SKILL_HIDEWORKING:
		 return 1;
		 case SKILL_METALCRAFT:
		 return 2;
		 case SKILL_WOODCRAFT:
		 return 3;
		 case SKILL_STONECRAFT:
		 return 4;
		 case SKILL_DYECRAFT:
		 return -1;
		 case SKILL_GLASSWORK:
		 return -1;
		 case SKILL_GEMCRAFT:
		 return -1;
		 case SKILL_POTTERY:
		 return -1; */
		default:
			return -1;

	}
}


/*------------------------------------------------------------------------\
 |  do_mend()                                                              |
 |                                                                         |
 |  User command to repair a damaged object.                               |
 \------------------------------------------------------------------------*/
void do_mend ( CHAR_DATA * ch, char *argument, int cmd)
{
	OBJECT_DAMAGE *damage = NULL;

	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char *dam = '\0';
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *kit;
	bool found = false;
	int cost = 0;
	double cost_quality_table[6] = {0.50, 0.75, 1.00, 1.50, 2.00, 2.50};
	int repair_skill = -1;
	int kit_range = 0;
	CHAR_DATA *tch;
	bool is_robot = !str_cmp( lookup_race_variable( ch->race, RACE_NAME ), "robot" );

	extern const char *damage_severity[DAMAGE_SEVERITY_MAX + 1];
	extern const char *damage_name[8][8][4];

	bool general = false;

	argument = one_argument (argument, arg);

    if ( is_robot ) {
        if ( !(obj = get_obj_in_list_vis( ch, arg, ch->room->contents ) ) ) {
            send_to_char( "#6KRZZ#3V12#6:#0 TARGET NOT FOUND!\n\r", ch );
            return;
        }

        if ( GET_ITEM_TYPE( obj ) != ITEM_E_ROBOT ) {
            send_to_char( "#6KRZZ#3V12#6:#0 MODEL NOT IN DATABASE!\n\r", ch );
            return;
        }
    } else {
        if (!(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
            !(obj = get_obj_in_dark (ch, arg, ch->left_hand)))
        {
            send_to_char ("You must be holding an item to repair it.\n", ch);
            return;
        }
    }

	argument = one_argument (argument, arg);

	if ((tch = get_char_room_vis (ch, arg)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (!IS_NPC (tch))
				continue;
			if (!IS_SET (tch->act, ACT_FIXER))
				continue;
			npc_repair (ch, tch, obj, argument);
			return;
		}
		send_to_char ("You don't see them here, or you need to specify what damage on your object you want to mend.\n", ch);
		return;
	}


	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_REPAIR_KIT)
		kit = ch->right_hand;
	else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_REPAIR_KIT)
		kit = ch->left_hand;
	else
	{
		send_to_char ("You need to have some sort of repair kit handy to repair damages.\n", ch);
		return;
	}

	if (kit->o.od.value[0] <= 0)
	{
		send_to_char("\nYour repair kit is empty.", ch);
	}

	if (!obj->damage)
	{
		sprintf(buf, "$p does not appear to be damaged.");
		act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	argument = one_argument (argument, arg2);

	if (kit->o.od.value[4] && !IS_SET(kit->o.od.value[4], 1 << GET_MATERIAL_TYPE(obj)))
	{
		sprintf(buf, "$p does not contain the necessary materials to mend $P.");
		act (buf, false, ch, kit, obj, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (kit->o.od.value[3] && (ch->skills[kit->o.od.value[3]] < 10))
	{
		sprintf(buf, "You need some knowledge of %s to use $p.", skills[kit->o.od.value[3]]);
		act (buf, false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	repair_skill = kit->o.od.value[3];

	if (kit->o.od.value[2] > ch->skills[repair_skill])
	{
		sprintf(buf, "You do not have the necessary knowledge of %s to use $p.", skills[repair_skill]);
		act (buf, false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!str_cmp("all", arg) || !str_cmp("superficial", arg) || !str_cmp("small", arg))
		general = true;
	else if (!*arg || !*arg2)
	{
		send_to_char ("Which spot of damage are you repairing? Select both the size and type of damage, e.g. 'mend vest minor rip'.\n", ch);
		return;
	}


	// So you're not killed by thousands of lines of spam of "small nicks", we're going to clump them together. You repair them all at once.

	if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
	{
		general = true;
	}

	if (general)
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (!str_cmp (damage_severity[damage->severity], "small"))
			{
				if (object_damage__get_sdesc (damage) != NULL)
				{
					cost += 1;
					found = true;
				}
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
			cost = cost / 2;
	}
	// If we find the two words that match, go for the first one. One terrible nick is just as bad as another terrible nick.
	else
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (!str_cmp (damage_severity[damage->severity], arg) && !str_cmp (damage_name[damage->source][damage->material][damage->name], arg2))
			{
				if ((dam = object_damage__get_sdesc (damage)) != NULL)
				{

					if (GET_ITEM_TYPE(obj) == ITEM_WORN ||
						GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
						cost = damage->impact * 0.50;
					else
						cost = damage->impact;
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			act ("You don't see that sort of damage on $p", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (damage->permanent)
		{
			sprintf(buf, "#2%s#0 is far too severe for $p to ever be fully repaired.", dam);
			buf[2] = toupper(buf[2]);
			act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		kit_range = kit->o.od.value[1];

		if (kit_range < 0)
			kit_range = 0;
		else if (kit_range > 5)
			kit_range = 5;

		if (damage->severity > kit_range)
		{
			sprintf(buf, "$p does not contain the necessary components to mend damage the size of #2%s#0 on $P.", dam);
			act (buf, false, ch, kit, obj, TO_CHAR | _ACT_FORMAT);
			return;
		}

		// If the repair kit needs a skill, then we'll require a certain threshold to be able to repair the damage properly.
		// In short:
		// Massive damage: adroit or better
		// Huge damage: talented or better
		// Large damage: familiar or better
		// Moderate damage: novice or better
		// minor and small can be repaired by anyone, provided they've go the skill (if the kit is calling for one)

		// However, if the skill needed is computerology, it can repair any size.

		if (repair_skill && repair_skill != SKILL_COMPUTEROLOGY)
		{
			switch (damage->severity)
			{
			case DAMAGE_MODERATE:
				if (ch->skills[repair_skill] < 20)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				break;
			case DAMAGE_LARGE:
				if (ch->skills[repair_skill] < 30)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				break;
			case DAMAGE_HUGE:
				if (ch->skills[repair_skill] < 40)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				break;
			case DAMAGE_MASSIVE:
				if (ch->skills[repair_skill] < 50)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
				break;
			default:
				break;
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_WORN ||
			GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
			cost = cost * 0.50;
		else if (IS_SET(obj->econ_flags, QUALITY_EPIC))
			cost = cost_quality_table[5] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_SUPERB))
			cost = cost_quality_table[4] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_GOOD))
			cost = cost_quality_table[3] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_ORDINARY))
			cost = cost_quality_table[2] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_POOR))
			cost = cost_quality_table[1] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_TRASH))
			cost = cost_quality_table[0] * cost;

		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
			cost = cost / 2;
	}

	cost = MAX(cost, 1);

	if (!found)
	{
		act ("You don't see that sort of damage on $p", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (cost > kit->o.od.value[0])
	{
		sprintf(buf, "There aren't enough materials left in your repair kit to fix #6%s#0 on $p.", dam);
		act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	// Need a test to see if you've got the right sort of materials to fix it.


	if (general)
	{
		sprintf(buf, "You begin to mend the minor damage on $p.");
		sprintf(buf2, "$n begins to mend the minor damage on $p.");
	}
	else
	{
		sprintf(buf, "You begin to mend #2%s#0 on $p.", dam);
		sprintf(buf2, "$n begins to mend #2%s#0 on $p.", dam);
	}

	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay = 15;
	ch->delay_type = DEL_MEND1;
	ch->delay_info1 = general;
	ch->delay_obj = obj;
	ch->delay_who = dam;

}

void
delayed_mend1 (CHAR_DATA * ch)
{
	char *dam;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	bool general = ch->delay_info1;

	if (ch->delay_who)
		dam = ch->delay_who;

	if (!(obj = ch->delay_obj) && !(ch->right_hand && obj == ch->right_hand) &&
	    !(ch->left_hand && ch->delay_obj == ch->left_hand))
	{
		send_to_char ("You must be holding an item to repair it.\n", ch);
		return;
	}

	if (general)
	{
		sprintf(buf, "You continue to mend the minor damage on $p.");
		sprintf(buf2, "$n continues to mend the minor damage on $p.");
	}
	else
	{
		sprintf(buf, "You continue to mend #2%s#0 on $p.", dam);
		sprintf(buf2, "$n continues to mend #2%s#0 on $p.", dam);
	}

	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	ch->delay = 15;
	ch->delay_type = DEL_MEND2;
}

void
delayed_mend2 (CHAR_DATA * ch)
{
	OBJECT_DAMAGE *damage = NULL;

	char *dam;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	//char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *kit;
	int cost = 0;
	double cost_quality_table[6] = {0.50, 0.75, 1.00, 1.50, 2.00, 2.50};
	int repair_skill = -1;

	extern const char *damage_severity[DAMAGE_SEVERITY_MAX + 1];

	bool general = ch->delay_info1;
	bool found = false;

	if (ch->delay_who)
		dam = ch->delay_who;

	if (!(obj = ch->delay_obj) && !(ch->right_hand && obj == ch->right_hand) &&
	    !(ch->left_hand && ch->delay_obj == ch->left_hand))
	{
		send_to_char ("You must be holding an item to repair it.\n", ch);
		return;
	}

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_REPAIR_KIT)
		kit = ch->right_hand;
	else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_REPAIR_KIT)
		kit = ch->left_hand;
	else
	{
		send_to_char ("You need to have some sort of repair kit handy to repair damages.\n", ch);
		return;
	}

	if (kit->o.od.value[4] && !IS_SET(kit->o.od.value[4], 1 << GET_MATERIAL_TYPE(obj)))
	{
		sprintf(buf, "$p does not contain the necessary materials to mend $P.");
		act (buf, false, ch, kit, obj, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (kit->o.od.value[3] && (ch->skills[kit->o.od.value[3]] < 10))
	{
		sprintf(buf, "You need some knowledge of %s to use $p.", skills[kit->o.od.value[3]]);
		act (buf, false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	repair_skill = kit->o.od.value[3];

	if (kit->o.od.value[2] > ch->skills[repair_skill])
	{
		sprintf(buf, "You do not have the necessary knowledge of %s to use $p.", skills[repair_skill]);
		act (buf, false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	// Need a skill test to see what sort of job you do, and for various other matters.
	// No skill use for general repairs.

	if (!general && repair_skill)
		skill_use(ch, repair_skill, 0);

	int running_cost = 0;

	if (general)
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (!str_cmp (damage_severity[damage->severity], "small") && !damage->permanent)
			{
				if (object_damage__get_sdesc (damage) != NULL)
				{
					cost = damage->impact;
					running_cost += cost;
					if (running_cost > kit->o.od.value[0])
					{
						//sprintf(buf3, "There wasn't enough materials left in your repair kit to fix all the minor damage on $p.");
						break;
					}
					object_damage__delete(obj, damage);
				}
			}
		}

		// If it's a weapon or firearm, we need to add on our permanent damage.
		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
		{
			// If our damage is more than 8, we subtract 8, then divide by 4 to
			// get how many permanent points of damage
			if (running_cost >= 8)
			{
				int perm_dam = running_cost;
				perm_dam -= 8;
				perm_dam /= 4;
				perm_dam = MAX(1, perm_dam);

				// First we try and find a bit of damage that is already small and permanent...
				for (damage = obj->damage; damage; damage = damage->next)
				{
					if (!str_cmp (damage_severity[damage->severity], "small") && !damage->permanent)
					{
						break;
					}
				}

				// If we don't find it, we create it and make it permanent.
				if (!damage)
				{
					damage = object__add_damage(obj, 1, 1);
					damage->permanent = 1;
				}

				damage->impact += perm_dam;
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
			running_cost = MAX(running_cost / 2, 1);
		else
			running_cost = MAX(running_cost, 1);

		kit->o.od.value[0] -= running_cost;
	}
	else
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			if (!str_cmp(dam, object_damage__get_sdesc (damage)))
			{
				if (GET_ITEM_TYPE(obj) == ITEM_WORN ||
					GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
					cost = cost * 0.50;
				else
					cost = damage->impact;
				dam = object_damage__get_sdesc (damage);
				found = true;
				break;
			}
		}

		if(!found)
		{
			act ("You don't see that sort of damage on $p", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (repair_skill && repair_skill != SKILL_COMPUTEROLOGY)
		{
			switch (damage->severity)
			{
				case DAMAGE_MINOR:
					running_cost = cost;
					break;

				case DAMAGE_MODERATE:
					if (ch->skills[repair_skill] < 20)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
					break;
				case DAMAGE_LARGE:
					if (ch->skills[repair_skill] < 35)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
					break;
				case DAMAGE_HUGE:
					if (ch->skills[repair_skill] < 50)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
					break;
				case DAMAGE_MASSIVE:
					if (ch->skills[repair_skill] < 60)
				{
					sprintf(buf, "It is beyond your knowledge of %s to repair #2%s#0 on $p.", skills[repair_skill], dam);
					act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
					return;
				}
					break;
				default:
					break;
			}
		}

		running_cost = cost;

		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM || GET_ITEM_TYPE(obj) == ITEM_WEAPON)
		{
			if (cost >= 8)
			{
				int perm_dam = cost;
				perm_dam -= 8;
				perm_dam /= 4;
				perm_dam = MAX(1, perm_dam);

				for (damage = obj->damage; damage; damage = damage->next)
				{
					if (!str_cmp (damage_severity[damage->severity], "small") && !damage->permanent)
					{
						break;
					}
				}

				if (!damage)
				{
					damage = object__add_damage(obj, 1, 1);
					damage->permanent = 1;
				}

				damage->impact += perm_dam;
			}

			cost = cost / 2;
		}

		if (GET_ITEM_TYPE(obj) == ITEM_WORN ||
			GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
			cost = cost * 0.50;
		else if (IS_SET(obj->econ_flags, QUALITY_EPIC))
			cost = cost_quality_table[5] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_SUPERB))
			cost = cost_quality_table[4] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_GOOD))
			cost = cost_quality_table[3] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_ORDINARY))
			cost = cost_quality_table[2] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_POOR))
			cost = cost_quality_table[1] * cost;
		else if (IS_SET(obj->econ_flags, QUALITY_TRASH))
			cost = cost_quality_table[0] * cost;

		cost = MAX(cost, 1);

		if(!str_cmp (damage_severity[damage->severity], "massive"))
		{
			damage->permanent = 1;
			damage->impact = (int) damage->impact / 2;
		}
		else
		{
			object_damage__delete(obj, damage);
		}

		kit->o.od.value[0] -= cost;
	}

	// If you used up more than 10 points, we get a chance to make a newbie skill roll.
	if (running_cost >= 10)
	{
		if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
		{
			if (!ch->skills[SKILL_ARMORCRAFT])
			{
				skill_learn(ch, SKILL_ARMORCRAFT);
			}
			else if (ch->skills[SKILL_ARMORCRAFT] < 10)
			{
				skill_use(ch, SKILL_ARMORCRAFT, 0);
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_WEAPON)
		{
			if (!ch->skills[SKILL_WEAPONCRAFT])
			{
				skill_learn(ch, SKILL_WEAPONCRAFT);
			}
			else if (ch->skills[SKILL_WEAPONCRAFT] < 10)
			{
				skill_use(ch, SKILL_WEAPONCRAFT, 0);
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_FIREARM)
		{
			if (!ch->skills[SKILL_GUNSMITH])
			{
				skill_learn(ch, SKILL_GUNSMITH);
			}
			else if (ch->skills[SKILL_GUNSMITH] < 10)
			{
				skill_use(ch, SKILL_GUNSMITH, 0);
			}
		}

		if (GET_ITEM_TYPE(obj) == ITEM_WORN || GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
		{
			if (!ch->skills[SKILL_HANDICRAFT])
			{
				skill_learn(ch, SKILL_HANDICRAFT);
			}
			else if (ch->skills[SKILL_HANDICRAFT] < 10)
			{
				skill_use(ch, SKILL_HANDICRAFT, 0);
			}
		}

		if (IS_ELECTRIC(obj))
		{
			if (!ch->skills[SKILL_ELECTRONICS])
			{
				skill_learn(ch, SKILL_ELECTRONICS);
			}
			else if (ch->skills[SKILL_ELECTRONICS] < 10)
			{
				skill_use(ch, SKILL_ELECTRONICS, 0);
			}
		}
	}

	if (general)
	{
		sprintf(buf, "You finish mending the minor damage on $p.");
		sprintf(buf2, "$n finishes mending the minor damage on $p.");
	}
	else
	{
		if (damage->permanent)
			sprintf(buf, "You partially mend #2%s#0, but the damage is too terrible for $p to ever be fully restorred.", dam);
		else
			sprintf(buf, "You finish mending #2%s#0 on $p.", dam);

		sprintf(buf2, "$n finishes mending #2%s#0 on $p.", dam);
	}

	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	if (kit->o.od.value[0] <= 0)
		send_to_char("\nYour repair kit is now empty.", ch);

}


void do_clean (CHAR_DATA *ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char arg[MAX_STRING_LENGTH] = { '\0' };
    int count = 0;
    bool clean_all = false;

    argument = one_argument(argument, arg);

    if (!*arg)
    {
        send_to_char("Syntax: clean <object held|all> [stains|dirt|dust|blood|water|filth|slime|rust]", ch);
        return;
    }

    if (!str_cmp(arg, "all"))
    {
        clean_all = true;
    }
    else if (!(obj = (get_obj_in_dark (ch, arg, ch->right_hand))) &&
        !(obj = (get_obj_in_dark (ch, arg, ch->left_hand))))
    {
        send_to_char ("You are not holding that object.\n", ch);
        return;
    }

    bool specific_enviro = false;
    int highest = 0;

    // Find the highest enviro_cond provided it's greater than 25.
    if (!clean_all)
    {
        for (int ind = 0; ind < COND_TYPE_MAX; ind++)
        {
            count += obj->enviro_conds[ind];
            if (ind != COND_STAIN && (obj->enviro_conds[ind] > 10 || (ind == COND_WATER && obj->enviro_conds[ind] > 20)))
            {
                specific_enviro = true;
                if (obj->enviro_conds[ind] > obj->enviro_conds[highest])
                {
                    highest = ind;
                }
            }
        }
    }

    if (clean_all)
    {
        highest = 0;
        for (obj = ch->equip; obj; obj = obj->next_content)
        {
            // Find the highest enviro_cond provided it's greater than 25.
            for (int ind = 0; ind < COND_TYPE_MAX; ind++)
            {
                if (ind != COND_STAIN && ((ind != COND_WATER && obj->enviro_conds[ind] > 10) || (ind == COND_WATER && obj->enviro_conds[ind] > 20)))
                {
                    highest = 1;
                    break;
                }
            }
            if (highest)
                break;
        }

        if (!obj)
        {
            send_to_char("Nothing you are wearing is dirty enough to warrant cleaning.\n", ch);
            return;
        }

        act ("You start cleaning your equipment, beginning with $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
        act ("$n starts cleaning $s equipment, beginning with $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
        ch->delay_obj = obj;
        ch->delay_type = DEL_CLEAN;
        ch->delay_info1 = 1; // whether it's a general clean or not
        ch->delay_info2 = -1; // how many times we repeat the cleaning dance - -1 for special.
        ch->delay_info3 = -1; // which ind we're cleaning
        ch->delay = 10;
    }
    else if (!specific_enviro)
    {
        act ("You start cleaning $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
        act ("$n starts cleaning $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
        ch->delay_obj = obj;
        ch->delay_type = DEL_CLEAN;
        ch->delay_info1 = 1; // whether it's a general clean or not
        ch->delay_info2 = count > 15 ? 1 : 0; // how many times we repeat the cleaning dance
        ch->delay_info3 = -1; // which ind we're cleaning
        ch->delay = 15;
    }
    else
    {
        int tier = obj->enviro_conds[highest] > 75 ? 3 : obj->enviro_conds[highest] > 50 ? 2 : obj->enviro_conds[highest] > 25 ? 1 : 0;

        if (*argument)
        {
            for (int ind = 0; ind < COND_TYPE_MAX; ind++)
            {
                if (!str_cmp(enviro_desc[ind][0], argument))
                {
                    highest = ind;
                    break;
                }
            }
        }

        OBJ_DATA *tobj = NULL;

        if (highest == COND_STAIN)
        {
            if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_REPAIR_KIT)
                tobj = ch->right_hand;
            else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_REPAIR_KIT)
                tobj = ch->left_hand;

            if (!tobj)
            {
                act("You will need a repair or cleaning kit to clean these stubborn stains from $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
                return;
            }
            else if (tobj && tobj->o.od.value[4] && !IS_SET(tobj->o.od.value[4], 1 << GET_MATERIAL_TYPE(obj)))
            {
                sprintf(buf, "$p does not contain the necessary materials to clean $P.");
                act (buf, false, ch, tobj, obj, TO_CHAR | _ACT_FORMAT);
                return;
            }
            else if (tobj && !tobj->o.od.value[1])
            {
                sprintf(buf, "$p does not contain enough components to clean $P.");
                act (buf, false, ch, tobj, obj, TO_CHAR | _ACT_FORMAT);
                return;
            }

            sprintf(buf, "You start %s the %s from $p with #2%s#0.", enviro_desc[highest][5], enviro_desc[highest][0], obj_short_desc(tobj));
            sprintf(buf2, "$n starts %s the %s from $p with #2%s#0.", enviro_desc[highest][5], enviro_desc[highest][0], obj_short_desc(tobj));
        }
        else
        {
            sprintf(buf, "You start %s the %s from $p.", enviro_desc[highest][5], enviro_desc[highest][0]);
            sprintf(buf2, "$n starts %s the %s from $p.", enviro_desc[highest][5], enviro_desc[highest][0]);
        }
        ch->delay_obj = obj;
        ch->delay_type = DEL_CLEAN;
        ch->delay_info1 = 0; // whether it's a general clean or not
        ch->delay_info2 = tier; // how many times we repeat the cleaning dance
        ch->delay_info3 = highest; // which ind we're cleaning
        ch->delay = 15;

    }

	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
};

void delayed_long_clean(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	int general = ch->delay_info1;

	if (!ch->delay_obj)
	{
		send_to_char ("You must be holding or wearing an item to clean it.\n", ch);
		return;
	}

	if (!(obj = ch->delay_obj) && !(ch->right_hand && obj == ch->right_hand) &&
	    !(ch->left_hand && ch->delay_obj == ch->left_hand) &&
	    !(ch->equip && obj->equiped_by != ch))
	{
		send_to_char ("You must be holding or wearing an item to clean it.\n", ch);
		return;
	}

	if (ch->delay_info2 > 0)
	{
        if (general)
        {
            sprintf(buf, "You continue to clean $p.");
            sprintf(buf2, "$n continues to clean $p.");
        }
        else
        {
            sprintf(buf, "You continue %s the %s from $p.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0]);
            sprintf(buf2, "$n continues %s the %s from $p.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0]);
        }

        ch->delay_obj = obj;
        ch->delay_type = DEL_CLEAN;
        ch->delay_info1 = ch->delay_info1;      // whether it's a general clean or not
        ch->delay_info2 = ch->delay_info2 - 1; // how many times we repeat the cleaning dance
        ch->delay_info3 = ch->delay_info3;      // which ind we're cleaning
        ch->delay = 15;
	}
	else
	{
        if (general)
        {
            if (ch->delay_info3 == -1)
            {
                for (int ind = 0; ind < COND_TYPE_MAX; ind++)
                {
                    if (ind == COND_WATER)
                    {
                        if (obj->enviro_conds[ind] > 20)
                            obj->enviro_conds[ind] = 20;
                    }
                    // Stains only clean via kits.
                    else if (ind == COND_STAIN)
                    {
                        continue;
                    }
                    // Blood adds a 10% rate of stain, which needs a toolkit to clean off.
                    else if(ch->delay_info3 == COND_BLOOD)
                    {
                        object__add_enviro(obj, COND_STAIN, obj->enviro_conds[ind]/10);
                        obj->enviro_conds[ind] = 0;
                    }
                    else
                        obj->enviro_conds[ind] = 0;
                }

                int highest = 0;
                obj = NULL;
                for (obj = ch->equip; obj; obj = obj->next_content)
                {
                    // Find the highest enviro_cond provided it's greater than 25.
                    for (int ind = 0; ind < COND_TYPE_MAX; ind++)
                    {
                        if (ind != COND_STAIN && ((ind != COND_WATER && obj->enviro_conds[ind] > 10) || (ind == COND_WATER && obj->enviro_conds[ind] > 20)))
                        {
                            highest = 1;
                            break;
                        }
                    }
                    if (highest)
                        break;
                }

                if (obj)
                {
                    sprintf(buf, "You finish cleaning $p, and start cleaning $P.");
                    sprintf(buf2, "$n finish cleaning $p, and start cleaning $P.");
                    act (buf, false, ch, ch->delay_obj, obj, TO_CHAR | _ACT_FORMAT);
                    //act (buf2, false, ch, ch->delay_obj, obj, TO_ROOM | _ACT_FORMAT);

                    ch->delay_obj = obj;
                    ch->delay_type = DEL_CLEAN;
                    ch->delay_info1 = 1; // whether it's a general clean or not
                    ch->delay_info2 = -1; // how many times we repeat the cleaning dance - -1 for special.
                    ch->delay_info3 = -1; // which ind we're cleaning
                    ch->delay = 10;
                    return;
                }
                else
                {
                    sprintf(buf, "You finish cleaning $p, and have nothing more to clean.");
                    sprintf(buf2, "$n finish cleaning $p, and have nothing more to clean.");
                    act (buf, false, ch, ch->delay_obj, 0, TO_CHAR | _ACT_FORMAT);
                    act (buf2, false, ch, ch->delay_obj, 0, TO_ROOM | _ACT_FORMAT);
                    return;

                }

            }
            else
            {
                sprintf(buf, "You finish cleaning $p.");
                sprintf(buf2, "$n finish cleaning $p.");

                for (int ind = 0; ind < COND_TYPE_MAX; ind++)
                {
                    if (ind == COND_WATER)
                    {
                        if (obj->enviro_conds[ind] > 20)
                            obj->enviro_conds[ind] = 20;
                    }
                    // Stains only clean via kits.
                    else if (ind == COND_STAIN)
                    {
                        continue;
                    }
                    // Blood adds a 10% rate of stain, which needs a toolkit to clean off.
                    else if(ch->delay_info3 == COND_BLOOD)
                    {
                        object__add_enviro(obj, COND_STAIN, obj->enviro_conds[ind]/10);
                        obj->enviro_conds[ind] = 0;
                    }
                    else
                        obj->enviro_conds[ind] = 0;
                }
            }
        }
        else
        {
            OBJ_DATA *tobj = NULL;
            if (ch->delay_info3 == COND_STAIN)
            {
                if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_REPAIR_KIT)
                    tobj = ch->right_hand;
                else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_REPAIR_KIT)
                    tobj = ch->left_hand;

                if (!tobj)
                {
                    act("You will need a repair or cleaning kit to keep cleaning these stubborn stains from $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
                    return;
                }
                else if (tobj && tobj->o.od.value[4] && !IS_SET(tobj->o.od.value[4], 1 << GET_MATERIAL_TYPE(obj)))
                {
                    sprintf(buf, "$p does not contain the necessary materials to clean $P.");
                    act (buf, false, ch, tobj, obj, TO_CHAR | _ACT_FORMAT);
                    return;
                }
                else if (tobj && !tobj->o.od.value[1])
                {
                    sprintf(buf, "$p does not contain enough components to clean $P.");
                    act (buf, false, ch, tobj, obj, TO_CHAR | _ACT_FORMAT);
                    return;
                }

                sprintf(buf, "You finish %s the %s from $p with #2%s#0.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0], obj_short_desc(tobj));
                sprintf(buf2, "$n finishes %s the %s from $p with #2%s#0.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0], obj_short_desc(tobj));

                tobj->o.od.value[0] -= 1;
                tobj->o.od.value[0] = MAX(tobj->o.od.value[0], 0);

                if (tobj->o.od.value[0] <= 0)
                    send_to_char("\nYour repair kit is now empty.", ch);
            }
            else
            {
                sprintf(buf, "You finish %s the %s from $p.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0]);
                sprintf(buf2, "$n finishes %s the %s from $p.", enviro_desc[ch->delay_info3][5], enviro_desc[ch->delay_info3][0]);
            }

            if (ch->delay_info3 == COND_WATER)
            {
                obj->enviro_conds[ch->delay_info3] = 20;
            }
            // Blood adds a 10% rate of stain, which needs a toolkit to clean off.
            else if(ch->delay_info3 == COND_BLOOD)
            {
                obj->enviro_conds[ch->delay_info3] = 0;
                object__add_enviro(obj, COND_STAIN, obj->enviro_conds[ch->delay_info3]/10);
            }
            else
            {
                obj->enviro_conds[ch->delay_info3] = 0;
            }
        }
	}

	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	act (buf2, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
};


const char *damage_name[11][8][4] = {
	/*  DAMAGE_PIERCE  */
	{
		{"puncture", "gouge", "perforation", "rupture"},    /* vs undefined */
		{"puncture", "gouge", "perforation", "rupture"},    /* vs organic */
		{"hole", "prick", "stab", "rupture"},    /* vs textile */
		{"puncture", "gouge", "perforation", "rupture"},    /* vs metal */
		{"puncture", "gouge", "perforation", "rupture"},    /* vs ceramic */
		{"scratch", "chink", "chip", "shatter"},    /* vs glass */
		{"puncture", "gouge", "perforation", "rupture"},    /* vs electronic */
		{"puncture", "gouge", "perforation", "rupture"}    /* vs plastics */
	},
	// BLUNT
	{
		{"dent", "pit", "crater", "pockmark"},    /* vs undefined */
		{"dent", "notch", "pit", "pockmark"},    /* vs organic */
		{"wear", "pit", "wrinkle", "crease"},    /* vs textile */
		{"dent", "pit", "crater", "pockmark"},    /* vs metal */
		{"crack", "notch", "pit", "pockmark"},    /* vs ceramic */
		{"break", "crack", "shatter", "chip"},    /* vs glass */
		{"dent", "notch", "pit", "pockmark"},    /* vs electronic */
		{"dent", "notch", "pit", "pockmark"}    /* vs plastics */
	},
	// SLASH AND CHOP
	{
		{"cut", "gash", "breach", "slice"},    /* vs undefined */
		{"cut", "gash", "slash", "slice"},    /* vs organic */
		{"tear", "rip", "shred", "cut"},    /* vs textile */
		{"cut", "gash", "breach", "slice"},    /* vs metal */
		{"cut", "gash", "slash", "slice"},    /* vs ceramic */
		{"scrape", "shatter", "crack", "notch"},    /* vs glass */
		{"cut", "gash", "slash", "slice"},    /* vs electronic */
		{"cut", "gash", "slash", "slice"}    /* vs plastics */
	},

	/*  DAMAGE_FREEZE  */
	{
		{"frost-burn", "crack", "chink", "break"},    /* vs undefined */
		{"frost-burn", "crack", "frostbite", "freeze"},    /* vs organic */
		{"frost-burn", "crack", "coldspot", "break"},    /* vs textile */
		{"frost-burn", "crack", "chink", "break"},    /* vs metal */
		{"frost-burn", "crack", "chink", "break"},    /* vs ceramic */
		{"shatter", "crack", "splinter", "break"},    /* vs glass */
		{"frost-burn", "crack", "chink", "break"},    /* vs electronic */
		{"frost-burn", "crack", "chink", "break"}    /* vs plastics */
	},
	/*  DAMAGE_BURN  */
	{
		{"burn", "scorch", "score", "sear"},    /* vs undefined */
		{"burn", "scorch", "char", "sear"},    /* vs organic */
		{"burn", "scorch", "char", "sear"},    /* vs textile */
		{"burn", "scorch", "score", "sear"},    /* vs metal */
		{"burn", "scorch", "char", "sear"},    /* vs ceramic */
		{"black-spot", "shatter", "crack", "break"},    /* vs glass */
		{"burn", "scorch", "score", "sear"},    /* vs electronic */
		{"burn", "scorch", "score", "sear"}    /* vs plastics */
	},
	/*  DAMAGE_FIST  */
	{
		{"scuff", "ding", "scratch", "scrape"},    /* vs undefined */
		{"scuff", "ding", "scratch", "bruise"},    /* vs organic */
		{"scuff", "abrasion", "scratch", "scrape"},    /* vs textile */
		{"scuff", "ding", "scratch", "scrape"},    /* vs metal */
		{"scuff", "chip", "scratch", "scrape"},    /* vs ceramic */
		{"scuff", "break", "scratch", "scrape"},    /* vs glass */
		{"scuff", "ding", "scratch", "scrape"},    /* vs electronic */
		{"scuff", "ding", "scratch", "scrape"}    /* vs plastics */
	},
	/*  DAMAGE_BLOOD  */
	{
		{"stain", "splatter", "blotch", "spot"},    /* vs undefined */
		{"stain", "splatter", "blotch", "spot"},    /* vs organic */
		{"stain", "splatter", "blotch", "spot"},    /* vs textile */
		{"stain", "splatter", "blotch", "spot"},    /* vs metal */
		{"stain", "splatter", "blotch", "spot"},    /* vs ceramic */
		{"stain", "splatter", "blotch", "spot"},    /* vs glass */
		{"stain", "splatter", "blotch", "spot"},    /* vs electronic */
		{"stain", "splatter", "blotch", "spot"}    /* vs plastics */
	},

	/*  DAMAGE_WATER  */
	{
		{"rust", "corrosion", "mark", "wear"},    /* vs undefined */
		{"rust", "corrosion", "mark", "wear"},    /* vs organic */
		{"water-damage", "corrosion", "mark", "wear"},    /* vs textile */
		{"rust", "corrosion", "mark", "wear"},    /* vs metal */
		{"chip", "crack", "mark", "wear"},    /* vs ceramic */
		{"crack", "break", "scratch", "scrape"},    /* vs glass */
		{"rust", "chink", "mark", "wear"},    /* vs electronic */
		{"crack", "erosion", "mark", "wear"}    /* vs plastics */
	},

	/*  DAMAGE_PERMANENT  */
	{
		{"damage", "damage", "damage", "damage"},    /* vs undefined */
		{"damage", "damage", "damage", "damage"},    /* vs organic */
		{"damage", "damage", "damage", "damage"},    /* vs textile */
		{"damage", "damage", "damage", "damage"},    /* vs metal */
		{"damage", "damage", "damage", "damage"},    /* vs ceramic */
		{"damage", "damage", "damage", "damage"},    /* vs glass */
		{"damage", "damage", "damage", "damage"},    /* vs electronic */
		{"damage", "damage", "damage", "damage"}    /* vs plastics */
	},


    /*  DAMAGE_REPAIR  */
	{
		{"damage", "damage", "damage", "damage"},    /* vs undefined */
		{"damage", "damage", "damage", "damage"},    /* vs organic */
		{"damage", "damage", "damage", "damage"},    /* vs textile */
		{"damage", "damage", "damage", "damage"},    /* vs metal */
		{"damage", "damage", "damage", "damage"},    /* vs ceramic */
		{"damage", "damage", "damage", "damage"},    /* vs glass */
		{"damage", "damage", "damage", "damage"},    /* vs electronic */
		{"damage", "damage", "damage", "damage"}    /* vs plastics */
	},

    /*  DAMAGE_BULLET  */
	{
		{"hole", "bullethole", "hole", "bullethole"},    /* vs undefined */
		{"hole", "bullethole", "puncture", "rupture"},    /* vs organic */
		{"hole", "bullethole", "tear", "rip"},    /* vs textile */
		{"puncture", "bullethole", "perforation", "rupture"},    /* vs metal */
		{"puncture", "bullethole", "perforation", "rupture"},    /* vs ceramic */
		{"hole", "bullethole", "chip", "shatter"},          /* vs glass */
		{"puncture", "bullethole", "perforation", "rupture"},    /* vs electronic */
		{"puncture", "bullethole", "perforation", "rupture"}    /* vs plastics */
	}
};
