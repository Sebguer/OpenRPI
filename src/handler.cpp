/*------------------------------------------------------------------------\
|  handler.c : Handler Module                         www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"


extern int knockout;
extern rpie::server engine;

COMBAT_MSG_DATA *cm_list = NULL;

const char *targets[] =
{
    "Self",
    "\n"
};

/* use_table_data is filled in at boot time from registry */

struct use_table_data use_table[] =
{
    {
        8
    },				/* BRAWLING */
    {10},
    {10},
    {10},
    {16},
    {16},
    {16},
    {22},
    {22},
    {22},
    {25},
    {25},
    {0},
    {0},
    {0},
    {0}
};

int
is_hooded (CHAR_DATA * ch)
{
    OBJ_DATA *obj;

    if ((obj = get_equip (ch, WEAR_NECK_1))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
        return 1;

    if ((obj = get_equip (ch, WEAR_NECK_2))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
        return 1;

    if ((obj = get_equip (ch, WEAR_OVER))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
        return 1;

    if ((obj = get_equip (ch, WEAR_ABOUT))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
        return 1;

    if (((obj = get_equip (ch, WEAR_HEAD))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
            || ((obj = get_equip (ch, WEAR_FACE))
                && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

        if (obj->obj_flags.type_flag == ITEM_WORN
                || obj->obj_flags.type_flag == ITEM_ARMOR)
            return 1;
    }

    return 0;
}

char *
char_names (CHAR_DATA * ch)
{
    OBJ_DATA *obj;

    if ((obj = get_equip (ch, WEAR_NECK_1)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
        return obj->desc_keys;

    if ((obj = get_equip (ch, WEAR_NECK_2)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
        return obj->desc_keys;

    if ((obj = get_equip (ch, WEAR_OVER)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
        return obj->desc_keys;

    if ((obj = get_equip (ch, WEAR_ABOUT)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
        return obj->desc_keys;

    if (((obj = get_equip (ch, WEAR_HEAD))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
            || ((obj = get_equip (ch, WEAR_FACE))
                && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

        if (obj->obj_flags.type_flag == ITEM_WORN)
        {
            if (!obj->desc_keys)
                return "obscured";

            return obj->desc_keys;
        }

        if (obj->obj_flags.type_flag == ITEM_ARMOR)
        {
            if (!obj->desc_keys)
                return "obscured";

            return obj->desc_keys;
        }
    }

    return ch->name;
}


char *
fname (char *namelist)
{
    static char holder[30];
    char *point;

    if (!namelist)
        return "";

    for (point = holder; isalpha (*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}

char *
fname_hyphen (char *namelist)
{
    static char holder[30];
    char *point;

    if (!namelist)
        return "obscured";

    for (point = holder;
            isalpha (*namelist) || *namelist == '-' || *namelist == ' ';
            namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return holder;
}

int
isname (const char *str, char *namelist)
{
    char *curname = '\0';
    const char *curstr = '\0';

    if (!str)
        return 0;

    if (!namelist)
        return 0;

    curname = namelist;
    for (;;)
    {
        for (curstr = str;; curstr++, curname++)
        {
            if ((!*curstr && !isalpha (*curname))
                    || is_abbrev (curstr, curname))
                return (1);

            if (!*curname)
                return (0);

            if (!*curstr || *curname == ' ')
                break;

            if (tolower (*curstr) != tolower (*curname))
                break;
        }

        /* skip to next name */

        for (; isalpha (*curname); curname++);
        if (!*curname)
            return (0);
        curname++;		/* first char of new name */
    }
}

// Case-sensitive version of isname().

int
isnamec (char *str, char *namelist)
{
    char *curname = '\0';
    char *curstr = '\0';

    if (!str)
        return 0;

    if (!namelist)
        return 0;

    curname = namelist;
    for (;;)
    {
        for (curstr = str;; curstr++, curname++)
        {
            if ((!*curstr && !isalpha (*curname))
                    || is_abbrevc (curstr, curname))
                return (1);

            if (!*curname)
                return (0);

            if (!*curstr || *curname == ' ')
                break;

            if (*curstr != *curname)
                break;
        }

        /* skip to next name */

        for (; isalpha (*curname); curname++);
        if (!*curname)
            return (0);
        curname++;		/* first char of new name */
    }
}

int
real_skill (CHAR_DATA * ch, int skill)
{
    if (!IS_NPC (ch) || IS_SET (ch->act, ACT_STAYPUT))
        return ch->skills[skill];
    else
        return vnum_to_mob (ch->mob->vnum)->skills[skill];
}

void
affect_modify (CHAR_DATA * ch, int type, int loc, int mod, int bitv,
               int add, int sn)
{
    if (type >= JOB_1 && type <= JOB_3)
        return;

    if (type >= CRAFT_FIRST && type <= CRAFT_LAST)
        return;

    //if (type >= MAGIC_SMELL_FIRST && type <= MAGIC_SMELL_LAST)
    //    return;

    if (type >= SOMA_FIRST && type <= SOMA_LAST)
        return;

    if (type == MAGIC_GUARD)
        return;

    if (type == MAGIC_WATCH)
        return;

    if (type == AFFECT_SHADOW)
        return;

    if (type == MUTE_EAVESDROP)
        return;

    if (add && bitv)
        ch->affected_by |= bitv;

    else if (bitv)
    {
        ch->affected_by &= ~bitv;
        mod = -mod;
    }

    /*
    	switch (type) {
    		case SPELL_STRENGTH:		GET_STR (ch) += mod;				return;
    		case SPELL_DEXTERITY:		GET_DEX (ch) += mod;				return;
    		case SPELL_INTELLIGENCE:	GET_INT (ch) += mod;				return;
    		case SPELL_AURA:			GET_AUR (ch) += mod;				return;
    		case SPELL_WILL:			GET_WIL (ch) += mod;				return;
    		case SPELL_CONSTITUTION:	GET_CON (ch) += mod;				return;
    		case SPELL_AGILITY:			GET_AGI (ch) += mod;				return;
    		default:														break;
    	}
    */

    if (loc >= (SKILL_BRAWLING + 10000) && loc <= (LAST_SKILL + 10000))
    {
        if (add)
            ch->skills[loc - 10000] += mod;
        else
        {
            if (ch->pc && ch->pc->skills[loc - 10000] < ch->skills[loc - 10000])
                ch->skills[loc - 10000] -= mod;
            else if (!ch->pc)
                ch->skills[loc - 10000] -= mod;
        }
        return;
    }

    else
        switch (loc)
        {

        case APPLY_NONE:
        case APPLY_CASH:
        case APPLY_SAVING_BREATH:
        case APPLY_SAVING_SPELL:
        case APPLY_AC:
            break;

        case APPLY_STR:
            GET_STR (ch) += mod;
            break;
        case APPLY_DEX:
            GET_DEX (ch) += mod;
            break;
        case APPLY_INT:
            GET_INT (ch) += mod;
            break;
        case APPLY_AUR:
            GET_AUR (ch) += mod;
            break;
        case APPLY_WIL:
            GET_WIL (ch) += mod;
            ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
            break;
        case APPLY_CON:
            GET_CON (ch) += mod;
            if (!IS_NPC (ch))
                ch->max_hit = 10 + 6 * GET_CON (ch);
            else
            {
                ch->max_hit += mod * 6;
                if (GET_HIT (ch) > ch->max_hit)
                    GET_HIT (ch) = ch->max_hit;
            }
            ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
            break;
        case APPLY_AGI:
            GET_AGI (ch) += mod;
            break;
        case APPLY_AGE:
            ch->time.birth += mod;
            break;
        case APPLY_HIT:
            ch->max_hit += mod;
            break;
        case APPLY_MOVE:
            ch->max_move += mod;
            break;
        case APPLY_DAMROLL:
            if (IS_NPC (ch))
                GET_DAMROLL (ch) += mod;
            break;
        default:
            break;

        }				/* switch */
}

void
nullify_affects (CHAR_DATA * ch)
{
    AFFECTED_TYPE *af;
    OBJ_DATA *eq;
    int i;

    /* Remove affects caused by equipment */

    for (i = 0; i < MAX_WEAR; i++)
    {

        if (!(eq = get_equip (ch, i)) ||
                i == WEAR_BELT_1 || i == WEAR_BELT_2 || i == WEAR_BACK)
            continue;

        for (af = eq->xaffected; af; af = af->next)
        {
            if ((af->a.spell.location != APPLY_OFFENSE &&
                    af->a.spell.location != APPLY_DEFENSE))
                affect_modify (ch, af->type, af->a.spell.location,
                               af->a.spell.modifier,
                               eq->obj_flags.bitvector, false, 0);
        }
    }


    /* Remove affects caused by spells */

    for (af = ch->hour_affects; af; af = af->next)
        affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
                       af->a.spell.bitvector, false, af->a.spell.sn);


    /* Restore base capabilities */

    ch->tmp_str = ch->str;
    ch->tmp_dex = ch->dex;
    ch->tmp_intel = ch->intel;
    ch->tmp_aur = ch->aur;
    ch->tmp_con = ch->con;
    ch->tmp_wil = ch->wil;
}

void
reapply_affects (CHAR_DATA * ch)
{
    AFFECTED_TYPE *af;
    OBJ_DATA *eq;
    int i;

    /* Add equipment affects back */

    for (i = 0; i < MAX_WEAR; i++)
    {

        if (!(eq = get_equip (ch, i)) ||
                i == WEAR_BELT_1 || i == WEAR_BELT_2 || i == WEAR_BACK)
            continue;

        for (af = eq->xaffected; af; af = af->next)
            if (af->a.spell.location != APPLY_OFFENSE &&
                    af->a.spell.location != APPLY_DEFENSE)
                affect_modify (ch, af->type, af->a.spell.location,
                               af->a.spell.modifier,
                               eq->obj_flags.bitvector, true, 0);
    }

    /* Add spell affects back */

    for (af = ch->hour_affects; af; af = af->next)
        affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
                       af->a.spell.bitvector, true, af->a.spell.sn);

    /* Make certain values are between 0..25, not < 0 and not > 25! */

    GET_DEX (ch) = MAX (0, MIN (GET_DEX (ch), 25));
    GET_INT (ch) = MAX (0, MIN (GET_INT (ch), 25));
    GET_WIL (ch) = MAX (0, MIN (GET_WIL (ch), 25));
    GET_AUR (ch) = MAX (0, MIN (GET_AUR (ch), 25));
    GET_CON (ch) = MAX (0, MIN (GET_CON (ch), 25));
    GET_STR (ch) = MAX (0, MIN (GET_STR (ch), 25));
}

AFFECTED_TYPE *
get_obj_affect_location (OBJ_DATA * obj, int location)
{
    AFFECTED_TYPE *af;

    if (!obj->xaffected)
        return NULL;

    for (af = obj->xaffected; af; af = af->next)
        if (af->a.spell.location == location)
            return af;

    return NULL;
}

void
remove_obj_affect_location (OBJ_DATA * obj, int location)
{
    AFFECTED_TYPE *af;
    AFFECTED_TYPE *taf;

    if (!obj->xaffected)
        return;

    if (obj->xaffected->a.spell.location == location)
    {
        af = obj->xaffected;
        obj->xaffected = obj->xaffected->next;
        mem_free (af);
        return;
    }

    for (af = obj->xaffected; af->next; af = af->next)
        if (af->next->a.spell.location == location)
        {
            taf = af->next;
            af->next = taf->next;
            mem_free (taf);
            return;
        }
}

void
affect_to_obj (OBJ_DATA * obj, AFFECTED_TYPE * af)
{
    AFFECTED_TYPE *taf;

    if (!obj->xaffected)
    {
        obj->xaffected = af;
        return;
    }

    for (taf = obj->xaffected; taf->next; taf = taf->next)
        ;

    taf->next = af;
}

/*
void
venom_to_mob (CHAR_DATA * mob, POISON_DATA * poison)
{
    POISON_DATA *taf;

    if (!mob->venom)
    {
        mob->venom = poison;
        return;
    }

    for (taf = mob->venom; taf->next; taf = taf->next)
        ;

    taf->next = poison;
}

void
poison_to_obj (OBJ_DATA * obj, POISON_DATA * poison)
{
    POISON_DATA *taf;

    if (!obj->poison)
    {
        obj->poison = poison;
        return;
    }

    for (taf = obj->poison; taf->next; taf = taf->next)
        ;

    taf->next = poison;
}

void remove_mob_venom (CHAR_DATA * mob, POISON_DATA *af)
{
    POISON_DATA *taf;

    if (af == mob->venom)
    {
        mob->venom = af->next;
        mem_free (af);
        return;
    }

    for (taf = mob->venom; taf->next; taf = taf->next)
    {
        if (af == taf->next)
        {
            taf->next = af->next;
            mem_free (af);
            return;
        }
    }
}


void remove_object_poison (OBJ_DATA * obj, POISON_DATA *af)
{
    POISON_DATA *taf;

    if (af == obj->poison)
    {
        obj->poison = af->next;
        mem_free (af);
        return;
    }

    for (taf = obj->poison; taf->next; taf = taf->next)
    {
        if (af == taf->next)
        {
            taf->next = af->next;
            mem_free (af);
            return;
        }
    }
}
*/


void
scent_to_room (ROOM_DATA * room, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec = NULL;

    if (!room->scent)
    {
        room->scent = scent;
        room->scent->next = NULL;
        return;
    }

    for (scent_sec =room->scent; scent_sec->next; scent_sec =scent_sec->next)
        ;

    scent_sec->next = scent;
    scent->next = NULL;
}

void
scent_to_mob (CHAR_DATA * mob, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec = NULL;

    if (!mob->scent)
    {
        mob->scent = scent;
        mob->scent->next = NULL;
        return;
    }

    for (scent_sec = mob->scent; scent_sec->next; scent_sec = scent_sec->next)
        ;

    scent_sec->next = scent;
    scent->next = NULL;
}

void
scent_to_obj (OBJ_DATA * obj, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec = NULL;

    if (!obj->scent)
    {
        obj->scent = scent;
        obj->scent->next = NULL;
        return;
    }

    for (scent_sec = obj->scent; scent_sec->next; scent_sec = scent_sec->next)
        ;

    scent_sec->next = scent;
    scent->next = NULL;
}

void
remove_mob_scent (CHAR_DATA * mob, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec;

    if (scent == mob->scent)
    {
        mob->scent = scent->next;
        mem_free (scent);
        return;
    }

    for (scent_sec = mob->scent; scent_sec->next; scent_sec = scent_sec->next)
    {
        if (scent == scent_sec->next)
        {
            scent_sec->next = scent->next;
            mem_free (scent);
            return;
        }
    }
}

void
remove_mob_scent (CHAR_DATA * mob, int scent_ref)
{
    SCENT_DATA *scent;
    SCENT_DATA *scent_sec;

    if (mob->scent && mob->scent->scent_ref == scent_ref)
    {
        scent = mob->scent;
        mob->scent = mob->scent->next;
        mem_free (scent);
        return;
    }

    for (scent = mob->scent; scent->next; scent = scent->next)
    {
        if (scent->next && scent->next->scent_ref == scent_ref)
        {
            scent_sec = scent->next;
            scent->next = scent_sec->next;
            mem_free (scent);
            return;
        }
    }
}

void
remove_room_scent (ROOM_DATA *room, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec;

    if (scent == room->scent)
    {
        room->scent = scent->next;
        mem_free (scent);
        return;
    }

    for (scent_sec =room->scent; scent_sec->next; scent_sec = scent_sec->next)
    {
        if (scent == scent_sec->next)
        {
            scent_sec->next = scent->next;
            mem_free (scent);
            return;
        }
    }
}

void
remove_room_scent (ROOM_DATA *room, int scent_ref)
{
    SCENT_DATA *scent;
    SCENT_DATA *scent_sec;

    if (room->scent && room->scent->scent_ref == scent_ref)
    {
        scent = room->scent;
        room->scent = room->scent->next;
        mem_free (scent);
        return;
    }

    for (scent = room->scent; scent->next; scent = scent->next)
    {
        if (scent->next && scent->next->scent_ref == scent_ref)
        {
            scent_sec = scent->next;
            scent->next = scent_sec->next;
            mem_free (scent);
            return;
        }
    }
}

void
remove_obj_scent (OBJ_DATA *obj, SCENT_DATA *scent)
{
    SCENT_DATA *scent_sec;

    if (scent == obj->scent)
    {
        obj->scent = scent->next;
        mem_free (scent);
        return;
    }

    for (scent_sec = obj->scent; scent_sec->next; scent_sec = scent_sec->next)
    {
        if (scent == scent_sec->next)
        {
            scent_sec->next = scent->next;
            mem_free (scent);
            return;
        }
    }
}

void
remove_obj_scent (OBJ_DATA *obj, int scent_ref)
{
    SCENT_DATA *scent;
    SCENT_DATA *scent_sec;

    if (obj->scent && obj->scent->scent_ref == scent_ref)
    {
        scent = obj->scent;
        obj->scent = obj->scent->next;
        mem_free (scent);
        return;
    }

    for (scent = obj->scent; scent->next; scent = scent->next)
    {
        if (scent->next && scent->next->scent_ref == scent_ref)
        {
            scent_sec = scent->next;
            scent->next = scent_sec->next;
            mem_free (scent);
            return;
        }
    }
}

// removes all instances of a type on an object

void
remove_obj_mult_affect (OBJ_DATA * obj, int type)
{
    AFFECTED_TYPE *af;

    int i = 0;
    int j = 0;

    if (!obj->xaffected)
        return;

    for (af = obj->xaffected; af; af = af->next)
    {
        if (af->type == type)
            j ++;
    }

    for (i = 0; i < j; i++)
        remove_obj_affect(obj, type);

}

void
remove_obj_affect (OBJ_DATA * obj, int type)
{
    AFFECTED_TYPE *af;
    AFFECTED_TYPE *free_af;

    if (!obj->xaffected)
        return;

    if (obj->xaffected->type == type)
    {
        af = obj->xaffected;
        obj->xaffected = af->next;
        mem_free (af);
        return;
    }

    for (af = obj->xaffected; af->next; af = af->next)
    {
        if (af->next->type == type)
        {
            free_af = af->next;
            af->next = free_af->next;
            mem_free (free_af);
            return;
        }
    }
}

AFFECTED_TYPE *
get_obj_affect (OBJ_DATA * obj, int type)
{
    AFFECTED_TYPE *af;

    if (!obj->xaffected)
        return NULL;

    for (af = obj->xaffected; af; af = af->next)
        if (af->type == type)
            return af;

    return NULL;
}

// returns the affect of a table we're sitting at or an
// object we're hiding behind.

AFFECTED_TYPE *
get_covtable_affect (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af = NULL;

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type == MAGIC_SIT_TABLE && is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents))
		{
			return af;
		}
		else if (af->type == AFFECT_COVER && is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents))
		{
			return af;
		}
	}

    return NULL;
}

AFFECTED_TYPE *
get_affect (const CHAR_DATA * ch, int affect_type)
{
    AFFECTED_TYPE *af;

    for (af = ch->hour_affects; af && af->type != affect_type; af = af->next);
    return af;
}

AFFECTED_TYPE *
get_soma_affect (const CHAR_DATA * ch, int affect_type)
{
    AFFECTED_TYPE *af;

    for (af = ch->hour_affects; af && af->type != affect_type; af = af->next);
    {
        if (af && af->type >= SOMA_FIRST && af->type <= SOMA_LAST)
        {
            return af;
        }
    }

    return NULL;
}

SCENT_DATA *
get_scent (CHAR_DATA * ch, int scent_ref)
{
    SCENT_DATA *scent;

    for (scent = ch->scent; scent && scent->scent_ref != scent_ref; scent = scent->next);
    {
        if (scent && scent->scent_ref == scent_ref)
        {
            return scent;
        }
    }
    return NULL;
}

SCENT_DATA *
get_scent (ROOM_DATA * room, int scent_ref)
{
    SCENT_DATA *scent;

    for (scent = room->scent; scent && scent->scent_ref != scent_ref; scent = scent->next);
    {
        if (scent && scent->scent_ref == scent_ref)
        {
            return scent;
        }
    }
    return NULL;
}


SCENT_DATA *
get_scent (OBJ_DATA * obj, int scent_ref)
{
    SCENT_DATA *scent;

    for (scent = obj->scent; scent && scent->scent_ref != scent_ref; scent = scent->next);
    {
        if (scent && scent->scent_ref == scent_ref)
        {
            return scent;
        }
    }
    return NULL;
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's.
*/

void
affect_to_char (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
    af->next = ch->hour_affects;
    ch->hour_affects = af;

    //affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier, af->a.spell.bitvector, true, af->a.spell.sn);
}

void
remove_affect_type (CHAR_DATA * ch, int type)
{
    AFFECTED_TYPE *af;

    if ((af = get_affect (ch, type)))
        affect_remove (ch, af);
}

/* Remove an affected_type structure from a char (called when duration
   reaches zero).  Pointer *af must never be NIL!  Frees mem and calls
   affect_location_apply.
*/

void
affect_remove (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
    AFFECTED_TYPE *taf;

    //affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier, af->a.spell.bitvector, false, af->a.spell.sn);

    /* remove structure *af from linked list */

    if (ch->hour_affects == af)
        ch->hour_affects = af->next;

    else
    {
        for (taf = ch->hour_affects; taf && taf->next != af; taf = taf->next)
            ;

        if (!taf)
        {
            return;
        }

        taf->next = af->next;
    }

    if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
        mem_free (af->a.craft);

    mem_free (af);
}

void
char_from_room (CHAR_DATA * ch)
{
    CHAR_DATA *i;
    ROOM_DATA *room;
    AFFECTED_TYPE *af;

    if (ch->in_room == NOWHERE)
    {
        system_log
        ("NOWHERE extracting char from room (handler.c, char_from_room)",
         true);
        abort ();
    }

    if (ch->room == NULL)
        return;

    room = ch->room;

    if (room && room->sector_type == SECT_LEANTO && IS_MORTAL (ch))
        room->deity--;

    if (ch == room->people)	/* head of list */
        room->people = ch->next_in_room;

    else
    {
        /* locate the previous element */

        for (i = room->people; i; i = i->next_in_room)
        {
            if (i->next_in_room == ch)
            {
                i->next_in_room = ch->next_in_room;
                break;
            }
        }

    }

	// If we're moving, we definitely want to capture our broken
	// aim here to we don't run in to problems elsewhere: so many
	// ways people can be moved from room to room.
	broke_aim(ch, 0);

    ch->in_room = NOWHERE;
    ch->room = NULL;
    ch->next_in_room = NULL;

    if ((af = get_affect (ch, MAGIC_SIT_TABLE)))
        remove_affect_type (ch, MAGIC_SIT_TABLE);

    if (room)
    {
        room_light (room);
        if (IS_MORTAL (ch) && room->vnum >= 100000
                && IS_SET (room->room_flags, TEMPORARY))
        {
            if (!room->people)
                rdelete (room);
        }
    }
}

/* place a character in a room */
void
char_to_room (CHAR_DATA * ch, int room_num)
{
    OBJ_DATA *obj;
    ROOM_DATA *room = NULL, *troom;
    char buf[MAX_STRING_LENGTH];
    int curr = 0;
  
    // Handle echoes/vNPC decloaking for portable dwellings, i.e. tents.
    if (ch->in_room >= 100000)
    {
        room = vnum_to_room (ch->in_room);
        if (room && room->dir_option[OUTSIDE]
                && (troom = vnum_to_room (room->dir_option[OUTSIDE]->to_room)))
        {
            load_save_room (troom);
            for (obj = troom->contents; obj; obj = obj->next_content)
            {
                if (obj->deleted)
                    continue;
                if (GET_ITEM_TYPE (obj) == ITEM_DWELLING &&
                        IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC_DWELLING) &&
                        obj->o.od.value[0] == ch->in_room)
                {
                    sprintf (buf, "#2%s#0 rustles as its occupants stir.",
                             obj_short_desc (obj));
                    buf[2] = toupper (buf[2]);
                    send_to_room (buf, obj->in_room);
                    obj->obj_flags.extra_flags &= ~ITEM_VNPC;
                    room->occupants++;
                    break;
                }
            } //for (obj = troom->contents
        } //if (room &&

        else if (!room)
            room = vnum_to_room (0);
    } //if (ch->in_room >= 100000)

    else if (!(room = vnum_to_room (room_num)))
    {
        sprintf (buf, "Room %d doesn't exist in char_to_room()! (%s)", room_num,
                 ch->tname);
        system_log (buf, true);
        room = vnum_to_room (0);
    }

    if (room->people && room->people != ch)
        ch->next_in_room = room->people;
    else
        ch->next_in_room = NULL;

    room->people = ch;
    
    ch->in_room = room->vnum;
    ch->room = room;

  if (!IS_NPC (ch) && !GET_TRUST (ch))
        room->room_flags |= PC_ENTERED; 


 if (IS_SET(room->room_flags, LOWAIR)
   && !get_affect (ch, AFFECT_CHOKING))
{ 
	if (has_breather(ch))
     { 
		act
        ("You suck in a breath of metallic, generated oxygen.\n",
        false, ch, 0, 0, TO_CHAR); 
	}
}

    if (IS_SET(room->room_flags, LOWAIR)
        && !get_affect (ch, AFFECT_CHOKING))
    {
        if (IS_MORTAL (ch) && !(has_breather(ch)))
        {
            act
            ("Your breathing becomes labored as the air grows thin...",
             false, ch, 0, 0, TO_CHAR);
            send_to_char ("\n", ch);
            curr = MAX ((ch->con + ch->str + ch->agi) * number (1, 3), 25);
            magic_add_affect (ch, AFFECT_CHOKING, curr, 0, 0, 0, curr);
        }    
    } 
	
    else if (IS_SET(room->room_flags, NOAIR)
        && !get_affect (ch, AFFECT_CHOKING))
	{ 
		if (has_breather(ch))
		{ 
			act
            ("You suck in a breath of metallic, generated oxygen.\n",
             false, ch, 0, 0, TO_CHAR); 
		}
        if (IS_MORTAL (ch) && !(has_breather(ch)))
        {
            act
            ("You take a deep breath just before entering the oxygen-deprived zone...",
             false, ch, 0, 0, TO_CHAR);
            send_to_char ("\n", ch);
            curr = MAX ((ch->con + ch->str/2 + ch->agi/2) * number (1, 3), 25);
            magic_add_affect (ch, AFFECT_CHOKING, curr, 0, 0, 0, curr);
        }
    }
    else if (!IS_SET(room->room_flags, NOAIR) && !IS_SET(room->room_flags, LOWAIR)
        && get_affect (ch, AFFECT_CHOKING))
    {
        if (IS_MORTAL (ch) && room->sector_type != SECT_UNDERWATER)
        {
            send_to_char ("\n", ch);
            act ("You inhale deeply, filling your lungs with much-needed air.",
                 false, ch, 0, 0, TO_CHAR);
            send_to_char ("\n", ch);
        }
        remove_affect_type (ch, AFFECT_CHOKING);
    }

    
    if (room->sector_type == SECT_UNDERWATER
            && !get_affect (ch, AFFECT_HOLDING_BREATH))
    {
        if (IS_MORTAL (ch))
        {
            act
            ("You take a deep breath just before plunging into the water...",
             false, ch, 0, 0, TO_CHAR);
            send_to_char ("\n", ch);
            curr = MAX ((ch->con*2 + ch->str + ch->agi) * number (1, 3), 25);
            magic_add_affect (ch, AFFECT_HOLDING_BREATH, curr, 0, 0, 0, curr);
        }
    }
    else if (room->sector_type != SECT_UNDERWATER && get_affect (ch, AFFECT_HOLDING_BREATH))
    {
        if (IS_MORTAL (ch) && !IS_SET(room->room_flags, NOAIR)
            && !IS_SET(room->room_flags, LOWAIR))
        {
            send_to_char ("\n", ch);
            act ("You inhale deeply, filling your lungs with much-needed air.",
                 false, ch, 0, 0, TO_CHAR);
            send_to_char ("\n", ch);
        }
        remove_affect_type (ch, AFFECT_HOLDING_BREATH);
    }

    room_light (ch->room);

    if (!room->psave_loaded)
        load_save_room (room);
}

void
obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch)	/* STACKing */
{
    int dir = 0;
    OBJ_DATA *tobj;
    bool stacked_obj = false;

    // Make sure we have a count of at least 1
    obj->count = (obj->count > 0) ? obj->count : 1;

    // reset any value override
    obj->obj_flags.set_cost = 0;

    /* Do object stacking */

    if (obj->nVirtual == VNUM_PENNY || obj->nVirtual == VNUM_FARTHING)
        return;

    if (GET_ITEM_TYPE (obj) == ITEM_CARD)
    {
        ;
    }
    else if (IS_SET (obj->obj_flags.extra_flags, ITEM_STACK) &&
             !(GET_ITEM_TYPE (obj) == ITEM_FOOD &&
               obj->o.food.food_value != obj->o.food.bites))
    {

        if (ch->right_hand && ch->right_hand->nVirtual == obj->nVirtual
                && ch->right_hand->morphTime == obj->morphTime
                && vo_match_color(ch->right_hand, obj))
        {
            tobj = ch->right_hand;
            obj->count += tobj->count;
            if (obj->nVirtual == VNUM_PENNY || obj->nVirtual == VNUM_FARTHING)
                name_money (obj);
            extract_obj (tobj);
            stacked_obj = true;
            ch->right_hand = obj;
            obj->carried_by = ch;
            obj->location = -1;
            obj->in_room = NOWHERE;
            obj->in_obj = NULL;
            return;
        }

        else if (ch->left_hand && ch->left_hand->nVirtual == obj->nVirtual
                 && ch->left_hand->morphTime == obj->morphTime
                 && vo_match_color(ch->left_hand, obj))
        {
            tobj = ch->left_hand;
            obj->count += tobj->count;
            if (obj->nVirtual == VNUM_PENNY || obj->nVirtual == VNUM_FARTHING)
                name_money (obj);
            extract_obj (tobj);
            stacked_obj = true;
            ch->left_hand = obj;
            obj->carried_by = ch;
            obj->location = -1;
            obj->in_room = NOWHERE;
            obj->in_obj = NULL;
            return;
        }
    }

    if (ch->left_hand && get_soma_affect(ch, SOMA_BLUNT_R_SEVARM))
    {
        send_to_char
        ("Since your hands are full, you set the object on the ground.\n",
         ch);
        obj_to_room (obj, ch->in_room);
        obj->in_obj = NULL;
        return;
    }

    if (((ch->right_hand && ch->left_hand) && !stacked_obj)
            || get_equip (ch, WEAR_BOTH))
    {
        send_to_char
        ("Since your hands are full, you set the object on the ground.\n",
         ch);
        obj_to_room (obj, ch->in_room);
        obj->in_obj = NULL;
        return;
    }

    if (get_soma_affect(ch, SOMA_BLUNT_R_SEVARM))
        ch->left_hand = obj;
    else if (!ch->right_hand)
        ch->right_hand = obj;
    else
        ch->left_hand = obj;

    if (GET_ITEM_TYPE (obj) == ITEM_KEY && IS_NPC (ch))
    {
        for (dir = 0; dir <= LAST_DIR; dir++)
        {
            if (EXIT (ch, dir)
                    && IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED)
                    && has_key (ch, NULL, EXIT (ch, dir)->key) && !ch->descr())
            {
                do_pmote (ch, "keeps a watchful eye on the entryways here.", 0);
                break;
            }
        }
    }

	remove_obj_mult_affect(obj, MAGIC_OMOTED);

	if (GET_ITEM_TYPE(obj) == ITEM_RESOURCE && !IS_SET (ch->affected_by, AFF_TRANSPORTING))
	{
		ch->affected_by |= AFF_TRANSPORTING;
	}

    obj->carried_by = ch;

    obj->next_content = NULL;

    obj->location = -1;

    obj->in_room = NOWHERE;

    obj->in_obj = NULL;

    room_light (ch->room);
}

OBJ_DATA *
split_obj (OBJ_DATA * obj, int count)
{
    OBJ_DATA *new_obj;

    if (obj->count < 1 || count >= obj->count)
        return obj;

    obj->count -= count;

    new_obj = load_object (obj->nVirtual);
    new_obj->count = count;

    if (obj->carried_by)
    {

        new_obj->location = -1;

        new_obj->carried_by = obj->carried_by;

        new_obj->in_room = NOWHERE;

        IS_CARRYING_N (obj->carried_by)++;
    }

    else if (obj->in_obj)
    {
        new_obj->next_content = obj->in_obj->contains;
        obj->in_obj->contains = new_obj;
    }

    return new_obj;
}

void
obj_from_char (OBJ_DATA ** obj, int count)	/* STACKing */
{
    int make_money = 0, fluid = 0, volume = 0;
    int nMorphTime = 0;
    CHAR_DATA *ch;


    ch = (*obj)->carried_by;

	if (ch == NULL)
	{
		return;
	}

	if (GET_ITEM_TYPE(*obj) == ITEM_E_REMOTE)
	{
		CHAR_DATA *tch;
		act ("You stop monitoring $p.", false, ch, (*obj), 0, TO_CHAR | _ACT_FORMAT);
		act ("$n stops monitoring $p.", false, ch, (*obj), 0, TO_ROOM | _ACT_FORMAT);
		for (tch = character_list; tch; tch = tch->next)
		{
			if (tch->deleted)
				continue;
			if ((tch->coldload_id == (*obj)->o.od.value[4]) && tch->controlling)
			{
				tch->controlling = 0;
			}
			else if ((tch->coldload_id == (*obj)->o.od.value[3]) && tch->controlled_by)
			{
				tch->controlled_by = 0;
				if (tch->mob)
					tch->mob->controller = 0;
			}
		}
		(*obj)->o.od.value[4] = 0;
	}

    /* Take a partial number of objs? */

    if (count != 0 && count < (*obj)->count)
    {
        (*obj)->count -= count;

        nMorphTime = (*obj)->morphTime;

        if ((*obj)->nVirtual == VNUM_PENNY || (*obj)->nVirtual == VNUM_FARTHING)
            name_money (*obj);

        if (IS_SET ((*obj)->obj_flags.extra_flags, ITEM_VARIABLE))
            *obj = load_colored_object ((*obj)->nVirtual, (*obj)->var_color[0], (*obj)->var_color[1], (*obj)->var_color[2], (*obj)->var_color[3],
                                        (*obj)->var_color[4], (*obj)->var_color[5], (*obj)->var_color[6], (*obj)->var_color[7], (*obj)->var_color[8], (*obj)->var_color[9]);
        else
            *obj = load_object ((*obj)->nVirtual);

        (*obj)->count = count;
        (*obj)->morphTime = nMorphTime;

        if ((*obj)->nVirtual == VNUM_PENNY || (*obj)->nVirtual == VNUM_FARTHING)
            name_money (*obj);

        (*obj)->carried_by = NULL;
        return;
    }

    /* Remove object from inventory */

    if (ch->right_hand == *obj)
        ch->right_hand = NULL;
    else if (ch->left_hand == *obj)
        ch->left_hand = NULL;

	if (GET_ITEM_TYPE(*obj) == ITEM_RESOURCE && IS_SET (ch->affected_by, AFF_TRANSPORTING))
	{
		ch->affected_by &= ~AFF_TRANSPORTING;
	}

    if (make_money)
        *obj = create_money (count);
    else
        IS_CARRYING_N (ch)--;

    (*obj)->carried_by = NULL;
    (*obj)->next_content = NULL;
    (*obj)->equiped_by = NULL;
    (*obj)->in_room = NOWHERE;
    (*obj)->in_obj = NULL;

    room_light (ch->room);
}

void
equip_char (CHAR_DATA * ch, OBJ_DATA * obj, int pos)
{
    char buf[MAX_STRING_LENGTH];

    if (ch == 0)
    {
        sprintf (buf,
                 "#1OBJECT MORPHING BUG! NULL ch pointer. Crash averted. Object vnum %d in room %d.#0\n",
                 obj->nVirtual,
                 obj->in_room);
        send_to_gods (buf);
        system_log (buf, true);
        return;
    }

    if (pos < 0)
    {
        sprintf (buf,
                 "#1OBJECT MORPHING BUG! Crash averted. Position %d, vnum %d, character %s in room %d.#0\n",
                 pos, obj->nVirtual, ch->tname, obj->in_room);
        send_to_gods (buf);
        system_log (buf, true);
        return;
    }

    if (get_equip (ch, pos))
        return;


    if (obj->in_room != NOWHERE)
    {
        system_log ("EQUIP: Obj is in_room when equip.", true);
        return;
    }

    obj->location = pos;

    if (pos != WEAR_PRIM && pos != WEAR_SEC && pos != WEAR_BOTH
            && pos != WEAR_SHIELD)
    {
        obj->next_content = ch->equip;
        ch->equip = obj;
    }

    if (pos == WEAR_PRIM || pos == WEAR_SEC || pos == WEAR_BOTH
            || pos == WEAR_SHIELD)
        obj->carried_by = ch;

    obj->equiped_by = ch;
}

OBJ_DATA *
unequip_char (CHAR_DATA * ch, int pos)
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *tobj = NULL;

    obj = get_equip (ch, pos);

    if (!obj)
        return NULL;

    if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) && is_hooded (ch))
        ch->affected_by &= ~AFF_HOODED;

    if (ch->equip == obj)
        ch->equip = ch->equip->next_content;
    else
    {
        for (tobj = ch->equip; tobj; tobj = tobj->next_content)
            if (tobj->next_content && obj == tobj->next_content)
            {
                tobj->next_content = obj->next_content;
                break;
            }
    }

    obj->location = -1;
    obj->equiped_by = NULL;
    obj->next_content = NULL;

    if (ch->aiming_at)
    {
		remove_targeted(ch->aiming_at, ch);
        ch->aiming_at = NULL;
        ch->aim = 0;
        ch->delay_info1 = 0;
        ch->delay_who = NULL;
        send_to_char ("You stop aiming your weapon.\n", ch);
    }

    return (obj);
}


int
get_number (char **name)
{

    int i;
    char *ppos = '\0';
    char number[MAX_INPUT_LENGTH] = { '\0' };

    if ((ppos = (char *) strchr (*name, '.')))
    {
        *ppos++ = '\0';
        strcpy (number, *name);
        strcpy (*name, ppos);

        for (i = 0; *(number + i); i++)
            if (!isdigit (*(number + i)))
                return (0);

        return (atoi (number));
    }

    return (1);
}

OBJ_DATA *
get_carried_item (CHAR_DATA * ch, int item_type)
{
    if (!ch)
        return NULL;

    if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == item_type)
        return ch->right_hand;

    if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == item_type)
        return ch->left_hand;

    return NULL;
}

/* Search a given list for an object, and return a pointer to that object */
OBJ_DATA *
get_obj_in_list (char *name, OBJ_DATA * list)
{
    OBJ_DATA *i, *drink;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy (tmpname, name);
    tmp = tmpname;
    if (!(number = get_number (&tmp)))
        return (0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
    {
        if (isname (tmp, i->name)
                || (GET_ITEM_TYPE (i) == ITEM_BOOK && i->book_title
                    && isname (tmp, i->book_title))
                || (GET_ITEM_TYPE (i) == ITEM_DRINKCON
                    && i->contains
                    && isname (tmp, i->contains->name)))
        {
            if (j == number)
                return (i);
            j++;
        }
    }

    return (0);
}

OBJ_DATA *
get_obj_in_list_num (int num, OBJ_DATA * list)
{
    OBJ_DATA *obj;

    for (obj = list; obj; obj = obj->next_content)
        if (obj->nVirtual == num)
            return obj;

    return NULL;
}

// Get the last object in a list.

OBJ_DATA *get_obj_in_list_last (OBJ_DATA * list)
{
    OBJ_DATA *obj;

    for (obj = list; obj; obj = obj->next_content)
        if (!obj->next_content)
            return obj;

    return NULL;
}

/*search the entire world for an object, and return a pointer  */
OBJ_DATA *
get_obj (char *name)
{
    OBJ_DATA *i,*drink;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy (tmpname, name);
    tmp = tmpname;
    if (!(number = get_number (&tmp)))
        return (0);

    for (i = object_list, j = 1; i && (j <= number); i = i->next)
    {

        if (i->deleted)
            continue;

        if (isname (tmp, i->name)
                || (GET_ITEM_TYPE (i) == ITEM_BOOK && i->book_title
                    && isname (tmp, i->book_title))
                || (GET_ITEM_TYPE (i) == ITEM_DRINKCON
                    && i->contains
                    && isname (tmp, i->contains->name)))
        {
            if (j == number)
                return (i);
            j++;
        }
    }

    return (0);
}

CHAR_DATA *
get_char_id (int coldload_id)
{
    CHAR_DATA *ch;

    for (ch = character_list; ch; ch = ch->next)
    {
        if (ch->deleted)
            continue;
        if (ch->coldload_id == coldload_id)
            return ch;
    }

    return NULL;
}

OBJ_DATA *
get_obj_in_list_id (int coldload_id, OBJ_DATA * list)
{
    OBJ_DATA *obj;

    for (obj = list; obj; obj = obj->next_content)
    {
        if (obj->deleted)
            continue;
        if (obj->coldload_id == coldload_id)
            return obj;
    }

    return NULL;
}

OBJ_DATA *
get_obj_id (int coldload_id)
{
    OBJ_DATA *obj;

    if (!coldload_id)
        return NULL;

    for (obj = object_list; obj; obj = obj->next)
    {
        if (obj->deleted)
            continue;
        if (obj->coldload_id == coldload_id)
            return obj;
    }

    return NULL;
}

/* search a room for a char, and return a pointer if found..  */
CHAR_DATA *
get_char_room (char *name, int room)
{
    CHAR_DATA *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy (tmpname, name);
    tmp = tmpname;
    if (!(number = get_number (&tmp)))
        return (0);

    for (i = vnum_to_room (room)->people, j = 1; i && (j <= number);
            i = i->next_in_room)
        if (isname (tmp, char_names (i)))
        {
            if (j == number)
                return (i);
            j++;
        }

    return (0);
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_char (char *name)
{
    CHAR_DATA *i;
    int j;
    int number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy (tmpname, name);
    tmp = tmpname;

    if (!(number = get_number (&tmp)))
        return 0;

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
    {

        if (i->deleted)
            continue;

        if (isname (tmp, char_names (i)))
        {
            if (j == number)
                return (i);
            j++;
        }
    }

    return 0;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_mob_vnum (int nVirtual)
{
    CHAR_DATA *i;

    for (i = character_list; i; i = i->next)
    {

        if (i->deleted)
            continue;

        if (!IS_NPC (i))
            continue;

        if (i->mob->vnum == nVirtual)
            return i;
    }

    return 0;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_char_nomask (char *name)
{
    CHAR_DATA *i;
    int j;
    int number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy (tmpname, name);
    tmp = tmpname;

    if (!(number = get_number (&tmp)))
        return 0;

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
    {

        if (i->deleted)
            continue;

        if (isname (tmp, GET_NAMES (i)))
        {
            if (j == number)
                return (i);
            j++;
        }
    }

    return 0;
}

void
obj_to_room (OBJ_DATA * object, int room)	/* STACKing */
{
    int add_at_top = 0;
    ROOM_DIRECTION_DATA *exit;
    ROOM_DATA *r, *troom = NULL;
    OBJ_DATA *tobj;
    char buf[MAX_STRING_LENGTH];

    if (!object)
        return;

    if (object->nVirtual == VNUM_PENNY || object->nVirtual == VNUM_FARTHING)
        return;

    r = vnum_to_room (room);

    if (!r)
        return;

    if (SWIM_ONLY (r))
    {
        tobj = object->next_content;
        object->next_content = NULL;
        object__drench (NULL, object, false);
        object->next_content = tobj;
    }

    if (r->contents
            && IS_SET (object->obj_flags.extra_flags, ITEM_STACK)
            && !get_obj_affect (object, MAGIC_HIDDEN))
    {

        for (tobj = r->contents; tobj; tobj = tobj->next_content)
        {

            if (tobj->nVirtual != object->nVirtual
                    || get_obj_affect (tobj, MAGIC_HIDDEN)
                    || tobj->morphTime != object->morphTime
                    || !vo_match_color(tobj, object))
                continue;

            tobj->count += object->count;
            extract_obj (object);
            room_light (r);
            return;
        }
    }
    // special case here: if we're loading up a hidden bullet, we want it to stack with other
    // hidden bullets or casings in the room, to help cut down on almight spam.
    else if (r->contents && (GET_ITEM_TYPE(object) == ITEM_CASE || GET_ITEM_TYPE(object) == ITEM_BULLET)
             && IS_SET (object->obj_flags.extra_flags, ITEM_STACK) && get_obj_affect (object, MAGIC_HIDDEN))
    {
        for (tobj = r->contents; tobj; tobj = tobj->next_content)
        {

            if (tobj->nVirtual != object->nVirtual
                    || !get_obj_affect (tobj, MAGIC_HIDDEN)
                    || tobj->morphTime != object->morphTime
                    || !vo_match_color(tobj, object))
                continue;

            tobj->count += object->count;
            extract_obj (object);
            room_light (r);
            return;
        }
    }

    if (add_at_top)
    {
        object->next_content = r->contents;
        r->contents = object;
    }
    else
    {
        if (!r->contents)
        {
            r->contents = object;
            object->next_content = NULL;
        }
        else
            for (tobj = r->contents; tobj; tobj = tobj->next_content)
            {
                if (!tobj->next_content)
                {
                    tobj->next_content = object;
                    object->next_content = NULL;
                    break;
                }
            }
    }

    object->in_room = room;
    object->carried_by = 0;
    object->equiped_by = NULL;
    object->in_obj = NULL;
    object->location = -1;

    room_light (r);

    exit = r->dir_option[DOWN];

    if (exit
            && !IS_SET (exit->exit_info, EX_ISDOOR)
            && !IS_SET (exit->exit_info, EX_ISGATE))
    {
        troom = vnum_to_room (exit->to_room);

        if (troom && troom->sector_type == SECT_UNDERWATER && SWIM_ONLY(r))
        {

            if (object->obj_flags.weight / 100 >= 1
                    || GET_ITEM_TYPE (object) == ITEM_MONEY)
            {
                sprintf (buf,
                         "#2%s#0 slowly sinks from sight down into the water.",
                         obj_short_desc (object));
                buf[2] = toupper (buf[2]);
                send_to_room (buf, r->vnum);
                obj_from_room (&object, 0);
                obj_to_room (object, troom->vnum);
                sprintf (buf, "#2%s#0 slowly sinks into view from above.",
                         obj_short_desc (object));
                buf[2] = toupper (buf[2]);
                send_to_room (buf, object->in_room);
            }

        }
        else if (troom
                 && (IS_SET (r->room_flags, CLIMB)
                     || IS_SET (r->room_flags, FALL)))
        {
            sprintf (buf, "#2%s#0 plummets down!", obj_short_desc (object));
            buf[2] = toupper (buf[2]);
            send_to_room (buf, r->vnum);
            obj_from_room (&object, 0);
            obj_to_room (object, troom->vnum);
            sprintf (buf, "#2%s#0 falls from above.", obj_short_desc (object));
            buf[2] = toupper (buf[2]);
            send_to_room (buf, object->in_room);
        }
    }

}

/* Take an object from a room */
void
obj_from_room (OBJ_DATA ** obj, int count)	/* STACKing */
{
    int make_money = 0, fluid = 0, volume = 0;
    int nMorphTime = 0;
    OBJ_DATA *tobj;
    ROOM_DATA *room;

    room = vnum_to_room ((*obj)->in_room);

    if (!room)
        return;

    // Ensure that any dwelling rooms are destroyed now that dwelling has been moved/removed.

    if (GET_ITEM_TYPE (*obj) == ITEM_DWELLING && (*obj)->o.od.value[0]
            && vnum_to_room ((*obj)->o.od.value[0]))
    {
        delete_contiguous_rblock (vnum_to_room ((*obj)->o.od.value[0]), -1,
                                  room->vnum);
        (*obj)->o.od.value[0] = 0;
    }

    /* NPC jailbags disappear, unless they are taken */

    if ((*obj)->nVirtual == VNUM_JAILBAG)
        (*obj)->obj_timer = 0;

    /* Take a partial number of objs? */

    if (count != 0 && count < (*obj)->count)
    {

        (*obj)->count -= count;
        nMorphTime = (*obj)->morphTime;

        if ((*obj)->nVirtual == VNUM_PENNY || (*obj)->nVirtual == VNUM_FARTHING)
            name_money (*obj);

        if (IS_SET ((*obj)->obj_flags.extra_flags, ITEM_VARIABLE))
            *obj = load_colored_object ((*obj)->nVirtual, (*obj)->var_color[0], (*obj)->var_color[1], (*obj)->var_color[2], (*obj)->var_color[3],
                                        (*obj)->var_color[4], (*obj)->var_color[5], (*obj)->var_color[6], (*obj)->var_color[7], (*obj)->var_color[8], (*obj)->var_color[9]);
        else
            *obj = load_object ((*obj)->nVirtual);

        (*obj)->count = count;
        (*obj)->morphTime = nMorphTime;

        if ((*obj)->nVirtual == VNUM_PENNY || (*obj)->nVirtual == VNUM_FARTHING)
            name_money (*obj);

        room_light (room);

        (*obj)->in_room = NOWHERE;
        (*obj)->next_content = NULL;

        return;
    }

    /* Remove object from the room */

    if (room->contents == *obj)
        room->contents = (*obj)->next_content;
    else if (room->contents)
    {
        for (tobj = room->contents;
                tobj->next_content; tobj = tobj->next_content)
            if (tobj->next_content == *obj)
            {
                tobj->next_content = (*obj)->next_content;
                break;
            }
    }

    if (make_money)
        *obj = create_money (count);

    (*obj)->in_room = NOWHERE;
    (*obj)->next_content = NULL;

    room_light (room);

    remove_obj_mult_affect (*obj, MAGIC_HIDDEN);
    trap_obj_release(room, *obj);


}

void
obj_to_obj (OBJ_DATA * obj, OBJ_DATA * container)
{
    OBJ_DATA *tobj;

    if (!obj || !container)
        return;

    if (obj == container)
    {
        send_to_gods("Error found in obj_to_obj: container and obj were equal.");
        return;
    }

    if (container->contains && IS_SET (obj->obj_flags.extra_flags, ITEM_STACK))
    {

        for (tobj = container->contains; tobj; tobj = tobj->next_content)
        {
            if (tobj->nVirtual != obj->nVirtual
                    || tobj->morphTime != obj->morphTime
                    || !vo_match_color(tobj, obj))
                continue;

            tobj->count += obj->count;
            //container->contained_wt += OBJ_MASS (tobj); -- double-counts existing objects: just work off what we're adding.
            container->contained_wt += OBJ_MASS (obj);
            extract_obj (obj);
            return;
        }
    }

    obj->in_obj = container;
    obj->equiped_by = NULL;
    obj->carried_by = NULL;

    if (container->contains && !is_obj_in_list (obj, container->contains))
        obj->next_content = container->contains;
    else if (!container->contains)
        container->contains = obj;
    else
    {
        extract_obj (obj);
        return;
    }

    container->contains = obj;
    obj->location = -1;

    for (tobj = container; tobj; tobj = tobj->in_obj)
        tobj->contained_wt += OBJ_MASS (obj);
}

void
obj_from_obj (OBJ_DATA ** obj, int count)
{
    int adjust_wt;
    int fluid = 0, volume = 0;
    OBJ_DATA *tobj;

    if (!(*obj))
        return;

    if (count < 0)
        count = 0;

    if (!count)
        count = (*obj)->count;

    /* Removing only part of obj from container */

    if (count < (*obj)->count)
    {

        tobj = *obj;

        if (IS_SET (tobj->obj_flags.extra_flags, ITEM_VARIABLE))
            *obj = load_colored_object ((*obj)->nVirtual, (*obj)->var_color[0], (*obj)->var_color[1], (*obj)->var_color[2], (*obj)->var_color[3],
                                        (*obj)->var_color[4], (*obj)->var_color[5], (*obj)->var_color[6], (*obj)->var_color[7], (*obj)->var_color[8], (*obj)->var_color[9]);
        else
            *obj = load_object (tobj->nVirtual);

        tobj->count -= count;

        (*obj)->count = count;
        (*obj)->morphTime = tobj->morphTime;
        (*obj)->in_obj = tobj->in_obj;

        adjust_wt = (*obj)->count * (*obj)->obj_flags.weight;
    }

    else
    {
        /* Removing obj completely from container */

        adjust_wt = OBJ_MASS (*obj);

        if ((*obj)->in_obj->contains == *obj)
            (*obj)->in_obj->contains = (*obj)->next_content;
        else
        {
            for (tobj = (*obj)->in_obj->contains;
                    tobj->next_content; tobj = tobj->next_content)
                if (tobj->next_content == *obj)
                {
                    tobj->next_content = (*obj)->next_content;
                    break;
                }
        }
    }

    for (tobj = (*obj)->in_obj; tobj; tobj = tobj->in_obj)
        tobj->contained_wt -= adjust_wt;

    (*obj)->next_content = NULL;
    (*obj)->in_obj = NULL;
}

void
object_list_new_owner (OBJ_DATA * list, CHAR_DATA * ch)
{
    if (list)
    {
        object_list_new_owner (list->contains, ch);
        object_list_new_owner (list->next_content, ch);
        list->carried_by = ch;
    }
}

void
remove_object_affect (OBJ_DATA * obj, AFFECTED_TYPE * af)
{
    AFFECTED_TYPE *taf;

    if (af == obj->xaffected)
    {
        obj->xaffected = af->next;
        mem_free (af);
        return;
    }

    for (taf = obj->xaffected; taf->next; taf = taf->next)
    {
        if (af == taf->next)
        {
            taf->next = af->next;
            mem_free (af);
            return;
        }
    }
}

void
destroy_dwelling (OBJ_DATA * obj)
{
    OBJ_DATA *tobj, *tobj_next;
    CHAR_DATA *tch, *tch_next;
    ROOM_DATA *troom;
    char buf[MAX_STRING_LENGTH];

    if (!(troom = vnum_to_room (obj->o.od.value[0])))
        return;

    sprintf (buf, "#2%s#0 is suddenly torn down around your ears!",
             obj_short_desc (obj));
    buf[2] = toupper (buf[2]);
    send_to_room (buf, troom->vnum);

    for (tobj = troom->contents; tobj; tobj = tobj_next)
    {
        tobj_next = tobj->next_content;
        obj_from_room (&tobj, 0);
        obj_to_room (tobj, obj->in_room);
    }

    for (tch = troom->people; tch; tch = tch_next)
    {
        tch_next = tch->next_in_room;
        char_from_room (tch);
        char_to_room (tch, obj->in_room);
    }
}

void
extract_obj (OBJ_DATA * obj)
{
    OBJ_DATA *tobj;

    if (GET_ITEM_TYPE (obj) == ITEM_DWELLING)
        destroy_dwelling (obj);

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
	}

    while (obj->contains)
        extract_obj (obj->contains);

    if (obj->equiped_by != NULL)
    {
        tobj = unequip_char (obj->equiped_by, obj->location);
        extract_obj (tobj);
    }

    if (obj->carried_by != NULL)
        obj_from_char (&obj, 0);
    else if (obj->in_obj != NULL)
        obj_from_obj (&obj, 0);
    else if (obj->in_room != NOWHERE)
        obj_from_room (&obj, 0);

    while (obj->xaffected)
        remove_object_affect (obj, obj->xaffected);

    while (obj->damage)
        object_damage__delete(obj, obj->damage);

    obj->carried_by = NULL;
    obj->equiped_by = NULL;
    obj->in_obj = NULL;

    obj->deleted = 1;

    knockout = 1;			/* Get obj out of object_list ASAP */

    vtoo (obj->nVirtual)->instances--;
}

void
morph_obj (OBJ_DATA * obj)
{

    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *newObj;

    if (obj->deleted)
        return;

    if (obj->in_room && vnum_to_room (obj->in_room)
            && IS_SET (vnum_to_room (obj->in_room)->room_flags, STORAGE))
        return;

    if (obj->morphto)
    {
        if (obj->morphto == 86)
        {
            extract_obj (obj);
            return;
        }
        else
        {
			newObj = LOAD_COLOR(obj, obj->morphto);
			fluid_object(newObj);
        }
    }
    else
    {
        sprintf (buf, "Object %d has a morph clock, but no morph Objnum\n",
                 obj->nVirtual);
        system_log (buf, true);
        return;
    }

    if (!newObj)
    {
        sprintf (buf, "Attempt to load morph obj %d for object %d failed\n",
                 obj->morphto, obj->nVirtual);
        system_log (buf, true);
        return;
    }

    newObj->count = obj->count;
    if (obj->equiped_by)
    {
        unequip_char (obj->equiped_by, obj->location);
        equip_char (obj->equiped_by, newObj, obj->location);
    }

    if (obj->carried_by)
    {
        CHAR_DATA *ch;

        ch = obj->carried_by;

        obj_from_char (&obj, 0);
        obj_to_char (newObj, ch);
    }


    if (obj->in_obj)
    {
        OBJ_DATA *container;

        container = obj->in_obj;
        obj_from_obj (&obj, 0);

        obj_to_obj (newObj, container);
    }

    if (obj->in_room != NOWHERE)
    {
        int room;

        room = obj->in_room;
        obj_from_room (&obj, 0);

        obj_to_room (newObj, room);
    }

    while (obj->xaffected)
        remove_object_affect (obj, obj->xaffected);

    obj->deleted = 1;
}

void
remove_sighting (CHAR_DATA * ch, CHAR_DATA * target)
{
    SIGHTED_DATA *sighted;

    if (!ch || !target)
        return;

    if (ch->sighted && ch->sighted->target == target)
        ch->sighted = ch->sighted->next;

    for (sighted = ch->sighted; sighted; sighted = sighted->next)
    {
        if (sighted->next && sighted->next->target == target)
            sighted->next = sighted->next->next;
    }
}

void
extract_char (CHAR_DATA * ch)
{
    CHAR_DATA *k;
    CHAR_DATA *next_char;
    CHAR_DATA *tch;
    ROOM_DATA *troom;
    OBJ_DATA *tobj;
    DESCRIPTOR_DATA *td;
    AFFECTED_TYPE *af;
    SECOND_AFFECT *sa;
    int was_in;
    char buf[MAX_STRING_LENGTH];

    extern struct timeval time_now;

    if (engine.in_play_mode () && IS_NPC (ch))
    {
        if (IS_SET (ch->act, ACT_STAYPUT))
        {
            mysql_safe_query
            ("DELETE FROM stayput_mobiles"
             " WHERE coldload_id = %d",
             ch->coldload_id);
            sprintf (buf, "save/mobiles/%d", ch->coldload_id);
            unlink (buf);
        }

        mysql_safe_query ("DELETE FROM reboot_mobiles WHERE coldload_id = %d",
                          ch->coldload_id);
        sprintf (buf, "save/reboot/%d", ch->coldload_id);
        unlink (buf);
    }

    if (ch->mob &&
            IS_SET (ch->act, ACT_VEHICLE) && (troom = vnum_to_room (ch->mob->vnum)))
    {

        /* By mistake, the vehicle might be in the same room as its
           entrance room.  In that case, n/pcs and mobs don't need to
           be moved.
         */

        if (troom == ch->room)
        {
            act ("$n is destroyed and some people fall out.",
                 false, ch, 0, 0, TO_ROOM);
            for (tch = troom->people; tch; tch = tch->next_in_room)
                tch->vehicle = NULL;
        }

        else
        {

            while (troom->people)
            {
                tch = troom->people;
                char_from_room (tch);
                char_to_room (tch, ch->in_room);
                tch->vehicle = NULL;
                act ("$N falls out of $n.", false, ch, 0, tch, TO_NOTVICT);
                act ("You fall out of $N as it is destroyed!",
                     false, tch, 0, ch, TO_CHAR);
            }

            while (troom->contents)
            {
                tobj = troom->contents;
                obj_from_room (&tobj, 0);
                obj_to_room (tobj, ch->in_room);
            }
        }
    }

    if (ch->descr() && ch->descr()->original)
        do_return (ch, "", 0);

    if (!IS_NPC (ch) && ch->room)
        save_attached_mobiles (ch, 1);

    if (!IS_NPC (ch))
        clear_watch (ch);

	if (ch->aiming_at)
	{
		remove_targeted(ch->aiming_at, ch);
	}

    /* Clear out guarders and watchers*/

    for (k = character_list; k; k = k->next)
    {
        if (k->deleted)
            continue;
        if ((af = get_affect (k, MAGIC_GUARD)) &&
                (CHAR_DATA *) af->a.spell.t == ch)
            affect_remove (k, af);
        if ((af = get_affect (k, MAGIC_WATCH)) &&
                (CHAR_DATA *) af->a.spell.t == ch)
            affect_remove (k, af);
        if (k->ranged_enemy == ch)
        {
            k->ranged_enemy = NULL;
            k->enemy_direction = NULL;
        }
        if (k->aiming_at == ch)
        {
            remove_targeted (ch, k);
            k->aiming_at = NULL;
            k->aim = 0;
        }
        if (k->subdue == ch)
            k->subdue = NULL;

        remove_threat (k, ch);
        remove_attacker (k, ch);
        remove_sighting (k, ch);

		if (k->pc && k->pc->edit_player && k->pc->edit_player == ch)
		{
			k->pc->edit_player = NULL;
			send_to_char("Player quitting - please re-mobile to continue working on them.\n", k);
		}
    }

    // Clear out second affects.

    for (sa = second_affect_list; sa;)
    {
        SECOND_AFFECT *next_sa = sa->next;

        if (sa->ch == ch)
            remove_second_affect(sa);

        sa = next_sa;
    }

    /*
    vector<second_affect*>::iterator it;
    for (it = second_affect_vector.begin(); it != second_affect_vector.end();)
    {
      if ((*it)->ch == ch)
      {
        second_affect_vector.erase(it);

        if ((*it)->info)
          mem_free ((*it)->info);
        mem_free ((*it));
        it = second_affect_vector.begin();
      }
      else
        it++;
    }
    */

    if (!IS_NPC (ch) && ch->pc->edit_player)
        unload_pc (ch->pc->edit_player);

    if (!IS_NPC (ch))
        save_char (ch, true);

    if (!IS_NPC (ch) && !ch->descr())
    {
        for (td = descriptor_list; td; td = td->next)
            if (td->original == ch)
                do_return (td->character, "", 0);
    }

    if (ch->in_room == NOWHERE)
    {
        system_log ("NOWHERE extracting char. (handler.c, extract_char)", true);
        //abort ();
    }

    stop_followers (ch);

    /* Enable reset for killed mob */

    if (IS_NPC (ch))
    {
        if (ch->mob->reset_zone != 0 || ch->mob->reset_cmd != 0)
            zone_table[ch->mob->reset_zone].cmd[ch->mob->reset_cmd].enabled = 1;
    }

    if (ch->descr())
    {
        if (ch->descr()->snoop.snooping && ch->descr()->snoop.snooping->descr())
            ch->descr()->snoop.snooping->descr()->snoop.snoop_by = 0;

        if (ch->descr()->snoop.snoop_by && ch->descr()->snoop.snoop_by->descr())
        {
            send_to_char ("Your victim is no longer among us.\n\r",
                          ch->descr()->snoop.snoop_by);
            ch->descr()->snoop.snoop_by->descr()->snoop.snooping = 0;
        }

        ch->descr()->snoop.snooping = ch->descr()->snoop.snoop_by = 0;
    }

    if (ch->controlling)
    {
        ch->controlling->controlled_by = 0;
        ch->controlling = 0;
    }

    if (ch->controlled_by)
    {
		OBJ_DATA *remote_obj = NULL;

		if (IS_NPC(ch) && (remote_obj = get_obj_id(ch->mob->controller)))
		{
			send_to_char ("Your viewscreen goes blank.\n\r",  ch->controlled_by);
			remote_obj->o.od.value[3] = 0;
			remote_obj->o.od.value[4] = 0;
		}

        ch->controlled_by->controlling = 0;
        ch->controlled_by = 0;
    }

    if (ch->fighting)
        stop_fighting (ch);

    for (k = combat_list; k; k = next_char)
    {
        /* FUNDAMENTALLY FLAWED LOGIC */
        next_char = k->next_fighting;
        if (k->fighting == ch)
            stop_fighting (k);
    }

    if (get_queue_position (ch) != -1)
        update_assist_queue (ch, true);

    /* Must remove from room before removing the equipment! */

    was_in = ch->in_room;
    char_from_room (ch);
    ch->in_room = was_in;

    if (ch->right_hand)
        extract_obj (ch->right_hand);

    if (ch->left_hand)
        extract_obj (ch->left_hand);

    while (ch->equip)
        extract_obj (ch->equip);

    ch->deleted = 1;

	// Don't clear their effects here: will prevent crafts from saving
	// if they are ever called again.

	if (IS_NPC(ch))
	{
		while (ch->hour_affects)
		{
			affect_remove (ch, ch->hour_affects);
		}
	}

    if (ch->descr() != NULL)
        ch->descr()->character = NULL;

    knockout = 1;			/* Get N/PC out of character_list ASAP */

    if (ch->descr() != NULL && !ch->descr()->acct
            && !IS_SET (ch->flags, FLAG_GUEST))
    {
        td = ch->descr();
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), td);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       td);
        SEND_TO_Q ("Your Choice: ", td);
        td->connected = CON_LOGIN;
        ch->descriptor = NULL;
        td->character = NULL;
        td->original = NULL;
        td->prompt_mode = 0;
        td->login_time = time_now;
        td->time_last_activity = mud_time;
    }
}

CHAR_DATA *
get_char_room_vis (CHAR_DATA * ch, const char *name)
{
    CHAR_DATA *tch = NULL;
    int j = 1;
    int number;
    char tmpname[MAX_STRING_LENGTH] = {'\0'};
    char immkeys[MAX_STRING_LENGTH] = {'\0'};
    char *tmp = '\0';
    char *tchtemp = '\0';
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    /* The player may use '.' to indicate last targeted n/pc. */

    if (!strcmp (name, "."))
    {
        if (!ch->pc)
            return NULL;

        if (is_he_here (ch, ch->pc->dot_shorthand, true))
            return ch->pc->dot_shorthand;
    }

    if (!strcmp (name, "self") || !strcmp (name, "me"))
        return ch;

    strcpy (tmpname, name);

	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}

    if (!(number = get_number (&tmp)))
        return NULL;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {

        sprintf (immkeys, "%s %s person", char_names (tch), tch->name);

        if (are_grouped (ch, tch) && IS_MORTAL (ch))
            tchtemp = tch->name;
        else if (IS_MORTAL (ch))
            tchtemp = char_names (tch);
        else
            tchtemp = tch->name;


        // if (isname (tmp, IS_MORTAL (ch) ? char_names (tch) : immkeys))
        if ((!twofer && (isname (tmp, tchtemp) || isname (tmp, char_names (tch)))) ||
			(twofer && (isname (tmp, tchtemp) || isname (tmp, char_names (tch))) && (isname (mod, tchtemp) || isname (mod, char_names (tch)))))
        {

            if (CAN_SEE (ch, tch) || ch == tch)
            {
                if (j == number)
                {
                    if (ch->pc)
                        ch->pc->dot_shorthand = tch;
                    return tch;
                }

                j++;
            }
        }
    }

    return NULL;
}

CHAR_DATA *
get_char_room_vis2 (CHAR_DATA * ch, int vnum, char *name)
{
    ROOM_DATA *room;
    CHAR_DATA *tch;
    int j = 1;
    int number;
    char tmpname[MAX_STRING_LENGTH];
    char immkeys[MAX_STRING_LENGTH];
    char *tmp;
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    if (!(room = vnum_to_room (vnum)))
        return NULL;

    /* The player may use '.' to indicate last targeted n/pc. */

    if (!strcmp (name, "."))
    {
        if (!ch->pc)
            return NULL;

        if (is_he_here (ch, ch->pc->dot_shorthand, true))
            return ch->pc->dot_shorthand;
    }

    if (!strcmp (name, "self") || !strcmp (name, "me"))
        return ch;

    strcpy (tmpname, name);

	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}

    if (!(number = get_number (&tmp)))
        return NULL;

    for (tch = room->people; tch; tch = tch->next_in_room)
    {

        sprintf (immkeys, "%s %s person", char_names (tch), tch->name);




        if ((!twofer && isname (tmp, IS_MORTAL (ch) ? char_names (tch) : immkeys)) ||
			(twofer && isname (tmp, IS_MORTAL (ch) ? char_names (tch) : immkeys) && isname (mod, IS_MORTAL (ch) ? char_names (tch) : immkeys)))

        {
            if (CAN_SEE (ch, tch) || ch == tch)
            {

                if (j == number)
                {
                    if (ch->pc)
                        ch->pc->dot_shorthand = tch;
                    return tch;
                }

                j++;
            }
        }
    }

    return NULL;
}

CHAR_DATA *
get_char_vis (CHAR_DATA * ch, char *name)
{
    CHAR_DATA *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    if (!strcmp (name, "self") || !strcmp (name, "me"))
        return ch;

    /* check location */
    if ((i = get_char_room_vis (ch, name)))
        return (i);

    strcpy (tmpname, name);
	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}
    if (!(number = get_number (&tmp)))
        return (0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
    {

        if (i->deleted)
            continue;

        if ((!twofer && isname (tmp, GET_NAMES (i))) ||
			(twofer && isname (tmp, GET_NAMES (i)) && isname (mod, GET_NAMES (i))))
            if (CAN_SEE (ch, i))
            {
                if (j == number)
                    return i;
                j++;
            }
    }

    return 0;
}

OBJ_DATA *
get_obj_in_list_vis_not_money (CHAR_DATA * ch, char *name, OBJ_DATA * list)
{
    OBJ_DATA *obj,*drink;
    int j = 1;
    int number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    strcpy (tmpname, name);
	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}

    if (!(number = get_number (&tmp)))
        return NULL;

    if (isdigit (*name))
    {

        if (!(number = atoi (name)))
            return NULL;

        for (obj = list; number && obj; obj = obj->next_content)
        {

            if (obj->nVirtual == VNUM_FARTHING || obj->nVirtual == VNUM_PENNY)
                continue;

            if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
                continue;

            if (CAN_SEE_OBJ (ch, obj) && !(--number))
                return obj;
        }

        return NULL;
    }

    for (obj = list; obj && (j <= number); obj = obj->next_content)
    {

        if (obj->nVirtual == VNUM_FARTHING || obj->nVirtual == VNUM_PENNY)
            continue;

        if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
            continue;

        if ((!twofer && isname (tmp, obj->name)) ||
			(twofer && isname(mod, obj->name) && isname(tmp, obj->name)) ||
			(GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title && isname (tmp, obj->book_title))
                || (GET_ITEM_TYPE (obj) == ITEM_DRINKCON && obj->contains && isname (tmp, obj->contains->name)))
            if (CAN_SEE_OBJ (ch, obj))
            {
                if (j == number)
                    return (obj);
                j++;
            }
    }

    return 0;
}

OBJ_DATA *
get_obj_in_list_vis (CHAR_DATA * ch, const char *name, OBJ_DATA * list)
{
    OBJ_DATA *obj;
    int j = 1;
    int number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    if (!name || !*name)
        return NULL;

    if (!list)
        return NULL;

    strcpy (tmpname, name);

	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}

    if (!(number = get_number (&tmp)))
        return NULL;

    if (*name == '#')
    {

        if (!(number = atoi (&name[1])))
            return NULL;

        for (obj = list; number && obj; obj = obj->next_content)
            if (CAN_SEE_OBJ (ch, obj) && !(--number))
                return obj;

        return NULL;
    }

    for (obj = list; obj && (j <= number); obj = obj->next_content)
    {
        if ((!twofer && isname (tmp, obj->name)) ||
			 (twofer && isname(mod, obj->name) && isname(tmp, obj->name)) ||
			 (GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title && isname (tmp, obj->book_title)))
		{
            if (CAN_SEE_OBJ (ch, obj))
            {
                if (j == number)
                    return (obj);
                j++;
            }
		}
    }

    return 0;
}

OBJ_DATA *
get_obj_in_dark (CHAR_DATA * ch, char *name, OBJ_DATA * list)
{
    OBJ_DATA *obj;
    int j = 1;
    int number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = '\0';
	char *mod;
	bool twofer = false;
	unsigned int point = 0;

    *tmpname = '\0';

    strcpy (tmpname, name);
	if ((point = strcspn(tmpname, ",")) && point != strlen(tmpname))
	{
		tmp = strtok(tmpname, ",");
		mod = strtok(NULL, ",");
		twofer = true;
	}
	else
	{
		tmp = tmpname;
		twofer = false;
	}

    if (!(number = get_number (&tmp)))
        return 0;

    if (*name == '#')
    {

        if (!(number = atoi (&name[1])))
            return NULL;

        for (obj = list; number && obj; obj = obj->next_content)
            if (IS_OBJ_VIS (ch, obj) && !(--number))
                return obj;

        return NULL;
    }

    for (obj = list; obj && (j <= number); obj = obj->next_content)
    {
        if ((!twofer && isname (tmp, obj->name)) ||
			(twofer && isname(mod, obj->name) && isname(tmp, obj->name)) ||
			(GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title && isname (tmp, obj->book_title)))
            if (IS_OBJ_VIS (ch, obj))
            {
                if (j == number)
                    return obj;
                j++;
            }
    }

    return 0;
}

/* search the entire world for an object, and return a pointer  */
OBJ_DATA *
get_obj_vis (CHAR_DATA * ch, char *name)
{
    OBJ_DATA *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* scan items carried */
    if ((i = get_obj_in_list_vis (ch, name, ch->right_hand)))
        return (i);

    if ((i = get_obj_in_list_vis (ch, name, ch->left_hand)))
        return (i);

    /* scan room */
    if ((i = get_obj_in_list_vis (ch, name, vnum_to_room (ch->in_room)->contents)))
        return (i);

    strcpy (tmpname, name);
    tmp = tmpname;
    if (!(number = get_number (&tmp)))
        return (0);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list, j = 1; i && (j <= number); i = i->next)
    {

        if (i->deleted)
            continue;

        if ( isname (tmp, i->name) || ( GET_ITEM_TYPE (i) == ITEM_BOOK && i->book_title && isname (tmp, i->book_title)))
            if (CAN_SEE_OBJ (ch, i))
            {
                if (j == number)
                    return (i);
                j++;
            }
    }
    return (0);
}

void
name_money (OBJ_DATA * obj)
{
    static char *farthing_one = "coin copper money cash brass farthing";
    static char *farthing_some = "coins copper money cash brass farthings";

    static char *farthing_short_1 = "one farthing";
    static char *farthing_short_2 = "two farthings";
    static char *farthing_short_3 = "three farthings";
    static char *farthing_short_4 = "four farthings";
    static char *farthing_short_handful = "a handful of farthings";
    static char *farthing_short_small = "a small pile of farthings";
    static char *farthing_short_pile = "a pile of farthings";
    static char *farthing_short_big = "a big pile of farthings";
    static char *farthing_short_huge = "a huge pile of farthings";
    static char *farthing_short_enormous = "an enormous pile of farthings";

    static char *name_one_penny = "coin silver penny money cash";
    static char *name_some_pennies = "coins silver pennies money cash";

    static char *short_mountain = "a mountain of silver coins";
    static char *short_huge = "a huge pile of silver coins";
    static char *short_big = "a big pile of silver coins";
    static char *short_pile = "a pile of silver coins";
    static char *short_several = "some silver coins";
    static char *short_one_penny = "a silver penny";

    static char *long_mountain = "A mountain of silver coins!";
    static char *long_huge = "A huge pile of silver coins!";
    static char *long_big = "A big pile of silver coins.";
    static char *long_pile = "A pile of silver coins.";
    static char *long_several = "Several silver coins.";
    static char *long_one_penny = "One silver penny.";

    obj->full_description = '\0';

    if (obj->nVirtual == VNUM_FARTHING)
    {

        obj->description = str_dup ("");

        if (obj->count == 1)
            obj->name = str_dup (farthing_one);
        else
            obj->name = str_dup (farthing_some);

        if (obj->count == 1)
            obj->short_description = str_dup (farthing_short_1);

        else if (obj->count == 2)
            obj->short_description = str_dup (farthing_short_2);

        else if (obj->count == 3)
            obj->short_description = str_dup (farthing_short_3);

        else if (obj->count == 4)
            obj->short_description = str_dup (farthing_short_4);

        else if (obj->count > 2501)	/* more than 2500 coins */
            obj->short_description = str_dup (farthing_short_enormous);

        else if (obj->count > 1001)	/* 1001 - 2500 coins */
            obj->short_description = str_dup (farthing_short_huge);

        else if (obj->count > 101)	/* 101 - 1000 coins */
            obj->short_description = str_dup (farthing_short_big);

        else if (obj->count > 51)	/* 51 - 100 coins */
            obj->short_description = str_dup (farthing_short_pile);

        else if (obj->count > 21)	/* 21 - 50 coins */
            obj->short_description = str_dup (farthing_short_small);

        else			/* 5 - 20 coins */
            obj->short_description = str_dup (farthing_short_handful);

        return;
    }

    if (obj->o.od.value[0] == 1)
    {
        obj->name = str_dup (name_one_penny);
        obj->short_description = str_dup (short_one_penny);
        obj->description = str_dup (long_one_penny);
    }

    else if (obj->o.od.value[0] >= 100000)
    {
        obj->name = str_dup (name_some_pennies);
        obj->short_description = str_dup (short_mountain);
        obj->description = str_dup (long_mountain);
    }

    else if (obj->o.od.value[0] >= 10000)
    {
        obj->name = str_dup (name_some_pennies);
        obj->short_description = str_dup (short_huge);
        obj->description = str_dup (long_huge);
    }

    else if (obj->o.od.value[0] >= 1000)
    {
        obj->name = str_dup (name_some_pennies);
        obj->short_description = str_dup (short_big);
        obj->description = str_dup (long_big);
    }

    else if (obj->o.od.value[0] >= 100)
    {
        obj->name = str_dup (name_some_pennies);
        obj->short_description = str_dup (short_pile);
        obj->description = str_dup (long_pile);
    }

    else
    {
        obj->name = str_dup (name_some_pennies);
        obj->short_description = str_dup (short_several);
        obj->description = str_dup (long_several);
    }
}


OBJ_DATA *
create_money (int amount)
{
    OBJ_DATA *obj;

    if (amount <= 0)
    {
        system_log ("ERROR: Try to create 0 or negative money.", true);
        abort ();
    }

    obj = load_object (VNUM_PENNY);

    obj->count = amount;

    name_money (obj);

    return (obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int
generic_find (char *arg, int bitvector, CHAR_DATA * ch,
              CHAR_DATA ** tar_ch, OBJ_DATA ** tar_obj)
{
    static char *ignore[] =
    {
        "the",
        "in",
        "on",
        "at",
        "\n"
    };

    int i;
    char name[256];
    bool found;

    found = false;


    /* Eliminate spaces and "ignore" words */
    while (*arg && !found)
    {

        for (; *arg == ' '; arg++);

        for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++);
        name[i] = 0;
        arg += i;
        if (search_block (name, ignore, true) > -1)
            found = true;

    }

    if (!name[0])
        return (0);

    *tar_ch = 0;
    *tar_obj = 0;

    if (IS_SET (bitvector, FIND_CHAR_ROOM))
    {
        /* Find person in room */
        if ((*tar_ch = get_char_room_vis (ch, name)))
        {
            return (FIND_CHAR_ROOM);
        }
    }

    if (IS_SET (bitvector, FIND_CHAR_WORLD))
    {
        if ((*tar_ch = get_char_vis (ch, name)))
        {
            return (FIND_CHAR_WORLD);
        }
    }

    if (IS_SET (bitvector, FIND_OBJ_EQUIP))
    {
        for (found = false, i = 0; i < MAX_WEAR && !found; i++)
            if (get_equip (ch, i) && !str_cmp (name, get_equip (ch, i)->name))
            {
                *tar_obj = get_equip (ch, i);
                found = true;
            }
        if (found)
        {
            return (FIND_OBJ_EQUIP);
        }
    }

    if (IS_SET (bitvector, FIND_OBJ_INV))
    {
        if ((*tar_obj = get_obj_in_list (name, ch->right_hand)))
        {
            return (FIND_OBJ_INV);
        }

        if ((*tar_obj = get_obj_in_list (name, ch->left_hand)))
        {
            return (FIND_OBJ_INV);
        }

        if ((*tar_obj = get_obj_in_list (name, ch->equip)))
        {
            return (FIND_OBJ_INV);
        }
    }

    if (IS_SET (bitvector, FIND_OBJ_ROOM))
    {
        if ((*tar_obj =
                    get_obj_in_list_vis (ch, name, vnum_to_room (ch->in_room)->contents)))
        {
            return (FIND_OBJ_ROOM);
        }
    }

    if (IS_SET (bitvector, FIND_OBJ_WORLD))
    {
        if ((*tar_obj = get_obj_vis (ch, name)))
        {
            return (FIND_OBJ_WORLD);
        }
    }

    return (0);
}

/* Return true if obj with vnum is equipt */

bool
get_obj_in_equip_num (CHAR_DATA * ch, long vnum)
{
    OBJ_DATA *eq;

    for (eq = ch->equip; eq; eq = eq->next_content)
        if (eq->nVirtual == vnum)
            return 1;

    return 0;
}

void
clear_delays (CHAR_DATA * ch)
{

    ch->delay = 0;
    ch->delay_info1 = 0;
    ch->delay_info2 = 0;
    ch->delay_info3 = 0;
    ch->delay_info4 = 0;
    ch->delay_info5 = 0;
    ch->delay_obj = NULL;
    ch->delay_ch = NULL;

    if (ch->delay_who)
    {
        mem_free (ch->delay_who);
        ch->delay_who = NULL;
    }

    ch->delay_type = 0;
}

int frequency[1000];

void
update_delays (void)
{
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *xaf = NULL;
    OBJ_DATA *obj = NULL;
    CHAR_DATA *ch, *ch_next;

    grunge_arena__update_delays ();

    // Reduce the time since the frequency was hit.

    for (int i = 0; i < 1000; i++)
    {
        if (frequency[i])
            frequency[i]--;

        if (frequency[i] < 0)
            frequency[i] = 0;
    }


    for (ch = character_list; ch; ch = ch_next)
    {

        ch_next = ch->next;

        if (!ch || ch->deleted || !ch->room || ch->in_room == NOWHERE)
            continue;

/*  if (IS_MORTAL (ch) && (af = get_affect (ch, AFFECT_HOLDING_BREATH)))
        {
            if (has_breather(ch))
            {
                send_to_char( "You suck in a breath of metallic, generated oxygen.\n", ch);
                continue;
            }
            
            if (--af->a.spell.duration <= 0)
            {
                if (drowned (ch))
                    continue;
            }
        }
*/
    if (IS_MORTAL (ch) && (af = get_affect (ch, AFFECT_CHOKING)))
        {
            if (has_breather(ch))
            {
                if (number(0, 4))
                    //send_to_char( "You suck in a breath of metallic, generated oxygen.\n", ch);
                continue;
            }
            
            if (IS_SET(ch->room->room_flags, LOWAIR) && number(0, 4))
                continue;
                
            if (--af->a.spell.duration <= 0)
            {
                if (suffocated (ch))
                    continue;
            }
        }

        if ((af = get_affect (ch, AFFECT_GROUP_RETREAT)))
        {
            if (--af->a.spell.duration <= 0)
            {

                if (ch->fighting)
                {
                    stop_fighting (ch);
                }

                if (GET_POS(ch) > SIT)
                {
                    char buf[AVG_STRING_LENGTH] = "";
                    sprintf (buf, "#3You retreat %sward!#0\n", dirs[af->a.spell.sn]);
                    send_to_char (buf, ch);

                    sprintf (buf, "$n retreats %sward.", dirs[af->a.spell.sn]);
                    act (buf, false, ch, 0, 0, TO_ROOM);
                    do_move (ch, "", af->a.spell.sn);
                }

                remove_affect_type (ch, AFFECT_GROUP_RETREAT);
            }
        }

        // If we fired and so broke our cover a bit, we need to reduce that down and quick.
        if ((af = get_affect(ch, AFFECT_COVER)) && af->a.cover.temp)
        {
            for (xaf = ch->hour_affects; xaf; xaf = xaf->next)
            {
                if (xaf->type == AFFECT_COVER && xaf->a.cover.value == 0)
                {
                    xaf->a.cover.temp--;
                    xaf->a.cover.temp = MAX(xaf->a.cover.temp, 0);
                }
            }
        }

        if (ch->stun > 0)
        {
            ch->stun--;
            if (ch->stun <= 0)
            {
                act ("You shake your head, recovering from the stun.", false,
                     ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                act ("$n shakes $s head, seeming to recover.", false, ch, 0, 0,
                     TO_ROOM | _ACT_FORMAT);
            }
        }
        if (ch->roundtime)
        {
            ch->roundtime--;
            if (ch->roundtime <= 0)
            {
                act ("You can perform another action, now.", false, ch, 0, 0,
                     TO_CHAR | _ACT_FORMAT);
            }
        }
        if (ch->balance < 0)
        {
            ch->balance++;
            if (ch->balance == 0)
            {
                send_to_char
                ("You feel as if you have fully regained your balance.\n",
                 ch);
            }
        }
        if (ch->aiming_at)
        {

            obj = get_equip (ch, WEAR_BOTH);
            if (!obj || GET_ITEM_TYPE(obj) != ITEM_FIREARM)
                obj = get_equip (ch, WEAR_PRIM);
            else if (!obj || GET_ITEM_TYPE(obj) != ITEM_FIREARM)
                obj = get_equip (ch, WEAR_SEC);
            else if (!obj || GET_ITEM_TYPE(obj) != ITEM_FIREARM)
                obj = NULL;

            // If we are not wielding a firearm, then we need to break our aim.
            if (!obj || !ch->aiming_at)
            {
                broke_aim(ch, 0);
            }
            else if (ch->delay_type == DEL_POINTBLANK)
            {
                ch->aim++;
                if (ch->in_room != ch->aiming_at->in_room)
                {
					remove_targeted(ch->aiming_at, ch);
                    break_delay(ch);
                }
            }
            else if (ch->fighting)
            {
                ch->aim = aim_penalty(ch, obj, 0);
            }
			else if (ch->aim == 11)
			{
				if (IS_NPC(ch) && !ch->descr() && IS_SET(ch->act, ACT_SHOOTER))
				{
					// 50% chance of taking the shot each tick.
					if (!number(0,1))
						do_firearm_fire(ch, "", 0);
				}
			}
            else if (ch->aim < 11)
            {
                ch->aim++;
                if ((ch->in_room == ch->aiming_at->in_room && ch->aim >= 5 && IS_NPC (ch)) || ch->aim >= 11)
                {
                    send_to_char ("", ch);
                    if (IS_NPC (ch) && !ch->descr() && !IS_FROZEN (ch->room->zone))
                    {
                        if (CAN_SEE (ch, ch->aiming_at) &&
                                (IS_SET (ch->act, ACT_AGGRESSIVE)
                                 || !IS_SUBDUEE (ch->aiming_at)))
                        {
                            //do_fire (ch, "", 0);
                        }
                        else
                        {
							remove_targeted(ch->aiming_at, ch);
                            ch->aiming_at = NULL;
                            ch->aim = 0;
                        }
                    }
                }
            }
        }
        if (ch->whirling && ch->whirling < 4)
        {
            ch->whirling++;
            if (ch->whirling == 4)
            {
                send_to_char
                ("You've got the sling whirling about as fast as you can manage.\n",
                 ch);
            }
        }

        // Special case: we want traps to go off as soon as the group is in the room, so we need a little bit of cheating.

        if (ch->delay_type == DEL_TRAPPED)
            delayed_trapped(ch);

        if (ch->deleted || !ch->delay || --ch->delay)
            continue;

        switch (ch->delay_type)
        {
        case DEL_TRAPPED:
            delayed_trapped (ch);
            break;
        case DEL_PITCH:
            delayed_pitch (ch);
            break;
        case DEL_OOC:
            delayed_ooc (ch);
            break;
        case DEL_FORAGE:
            delayed_forage (ch);
            break;
        case DEL_SCOUT:
            delayed_scout (ch);
            break;
        case DEL_FORM:
            delayed_form (ch);
            break;
        case DEL_LOG1:
            delayed_log1 (ch);
            break;
        case DEL_LOG2:
            delayed_log2 (ch);
            break;
        case DEL_LOG3:
            delayed_log3 (ch);
            break;
        case DEL_RUMMAGE:
            delayed_rummage (ch);
            break;
        case DEL_QUAFF:
            delayed_quaff (ch);
            break;
        case DEL_APP_APPROVE:
            ch->delay += 10;
            break;
        case DEL_CAST:
            delayed_cast (ch);
            break;
            /*Fyren start */
            /*??			case DEL_SKIN:			delayed_skin (ch);				break; */
        case DEL_SKIN_1:
            delayed_skin_new1 (ch);
            break;
        case DEL_SKIN_2:
            delayed_skin_new2 (ch);
            break;
        case DEL_SKIN_3:
            delayed_skin_new3 (ch);
            break;
            /*Fyren end */
        case DEL_BUTC_1:
            delayed_butcher1 (ch);
            break;
        case DEL_BUTC_2:
            delayed_butcher2 (ch);
            break;
        case DEL_BUTC_3:
            delayed_butcher3 (ch);
            break;
        case DEL_COUNT_COIN:
            delayed_count_coin (ch);
            break;
        case DEL_IDENTIFY:
            delayed_identify (ch);
            break;
        case DEL_GATHER:
            delayed_gather (ch);
            break;
        case DEL_COMBINE:
            break;
        case DEL_WHAP:
            delayed_whap (ch);
            break;
        case DEL_WATER_REMOVE:
            delayed_remove (ch);
            break;
        case DEL_GET_ALL:
            delayed_get (ch);
            break;
        case DEL_EMPATHIC_HEAL:
            delayed_heal (ch);
            break;
        case DEL_MENTAL_BOLT:
            delayed_bolt (ch);
            break;
        case DEL_SEARCH:
            delayed_search (ch);
            break;
        case DEL_PICK:
            delayed_pick (ch);
            break;
        case DEL_ALERT:
            delayed_alert (ch);
            break;
        case DEL_INVITE:
            break_delay (ch);
            break;
        case DEL_CAMP1:
            delayed_camp1 (ch);
            break;
        case DEL_CAMP2:
            delayed_camp2 (ch);
            break;
        case DEL_CAMP3:
            delayed_camp3 (ch);
            break;
        case DEL_CAMP4:
            delayed_camp4 (ch);
            break;
  case DEL_HOME_CAMP1:
            delayed_home_camp1 (ch);
            break;
        case DEL_HOME_CAMP2:
            delayed_home_camp2 (ch);
            break;
        case DEL_HOME_CAMP3:
            delayed_home_camp3 (ch);
            break;
        case DEL_COVER:
            delayed_cover (ch);
            break;
        case DEL_TAKE:
            delayed_take (ch);
            break;
        case DEL_PUTCHAR:
            delayed_putchar (ch);
            break;
        case DEL_STARE:
            delayed_study (ch);
            break;
        case DEL_HIDE:
            delayed_hide (ch);
            break;
        case DEL_HIDE_OBJ:
            delayed_hide_obj (ch);
            break;
        case DEL_PICK_OBJ:
            delayed_pick_obj (ch);
            break;
        case DEL_BIND_WOUNDS:
            delayed_bind (ch);
            break;
        case DEL_LONG_BIND:
            delayed_long_bind (ch);
            break;
        case DEL_CLEAN:
            delayed_long_clean (ch);
            break;
        case DEL_TREAT_WOUND:
            delayed_treatment (ch);
            break;
        case DEL_LOAD_WEAPON:
            delayed_load (ch);
            break;
        case DEL_TRACK:
            delayed_track (ch);
            break;
        case DEL_POISON_ITEM:
            delayed_poison (ch);
            break;
        case DEL_MEND1:
            delayed_mend1 (ch);
            break;
        case DEL_MEND2:
            delayed_mend2 (ch);
            break;
        case DEL_TRAP_ASS_1:
            delayed_trap_assemble (ch);
            break;
        case DEL_TRAP_ASS_2:
            delayed_trap_assemble2 (ch);
            break;
        case DEL_TRAP_DIS:
            delayed_trap_dissamble (ch);
            break;
        case DEL_TRAP_SEARCH:
            delayed_trap_search (ch);
            break;
        case DEL_CHEM_COMBINE_MIX:
            delayed_chem_combine_mix (ch);
            break;
        case DEL_CHEM_CONCENTRATE_MIX:
            delayed_chem_concentrate_mix (ch);
            break;
        case DEL_CHEM_INJECT:
            delayed_inject (ch);
            break;
        case DEL_LOAD_CLIP:
            delayed_load_clip (ch);
            break;
        case DEL_UNLOAD_CLIP:
            delayed_unload_clip (ch);
            break;
        case DEL_LOAD_FIREARM:
            delayed_load_firearm (ch);
            break;
        case DEL_UNLOAD_FIREARM:
            delayed_unload_firearm (ch);
            break;
        case DEL_POINTBLANK:
            delayed_pointblank (ch);
            break;
        case DEL_EXTRACT_1:
            delayed_extract_pause (ch);
            break;
        case DEL_EXTRACT_2:
            delayed_extract(ch);
            break;
		case DEL_TRADE:
			delayed_trade(ch);
			break;
		case DEL_TRADE_ASSIST:
			delayed_trade_assist(ch);
			break;
        }

        // If we don't have a delay, i.e. we've moved on to the second phase of a chain,
        // then clear out all of the delay info we might still be carrying on us.
        if (!ch->delay)
            clear_delays(ch);

    }
}

void
break_delay (CHAR_DATA * ch)
{

    OBJ_DATA *obj;

    if (ch->aim)
    {
        send_to_char ("You cease aiming your weapon.\n", ch);
        ch->aim = 0;
        if (ch->aiming_at)
        {
            remove_targeted(ch->aiming_at, ch);
            ch->aiming_at = NULL;
        }
        return;
    }

    if (ch->whirling)
    {
        send_to_char ("You cease whirling your sling.\n", ch);
        ch->whirling = 0;
        return;
    }

    switch (ch->delay_type)
    {

    case DEL_LOAD_WEAPON:
        send_to_char ("You cease loading your weapon.\n", ch);
        ch->delay = 0;
        break;

    case DEL_WATER_REMOVE:
        send_to_char ("You cease attempting to remove that item.\n", ch);
        ch->delay = 0;
        break;

    case DEL_EXTRACT_1:
    case DEL_EXTRACT_2:
        ch->delay_ch = NULL;
        if (ch->delay_who)
            mem_free(ch->delay_who);
        send_to_char ("You cease extracting bullets.\n", ch);
        break;

	case DEL_TRADE:
		send_to_char ("You cease trading at the marketplace.\n", ch);
		break;

    case DEL_BIND_WOUNDS:
    case DEL_LONG_BIND:
        ch->flags &= ~FLAG_BINDING;
        ch->delay = 0;
        send_to_char ("You cease your ministrations.\n", ch);
        break;

    case DEL_TRACK:
        ch->delay = 0;
        send_to_char ("You discontinue your search for tracks.\n", ch);
        break;

    case DEL_TREAT_WOUND:
        ch->delay = 0;
        send_to_char ("You cease your ministrations.\n", ch);
        break;

    case DEL_BACKSTAB:
        ch->delay = 0;
        send_to_char ("You abort your backstab attempt.\n\r", ch);
        break;

    case DEL_PURCHASE_ITEM:
    case DEL_ORDER_ITEM:
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_ch = NULL;

		if (ch->delay_obj && !ch->delay_obj->deleted)
        {
          extract_obj(ch->delay_obj);
        }
		ch->delay_obj = NULL;

        send_to_char ("You decide against making the previous purchase.\n\r", ch);
        break;

    case DEL_CANCEL_AUCTION:
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_ch = NULL;
        send_to_char ("You decide against cancelling the auction.\n\r", ch);
        break;

    case DEL_PLACE_AUCTION:
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_ch = NULL;
        send_to_char ("You decide against placing the auction.\n\r", ch);
        break;

    case DEL_PLACE_BID:
    case DEL_PLACE_BUYOUT:
	case DEL_ORDER_PLACE:
	case DEL_ORDER_FULFILL:
        ch->delay_type = 0;
        ch->delay_ch = NULL;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char ("You decide against placing the bid.\n\r", ch);
        break;

	case DEL_RENT_COMMENCE:
	case DEL_RENT_TOPUP:
	case DEL_RENT_REPLACE:
	case DEL_RENT_REPLICATE:
        ch->delay = 0;
        ch->delay_type = 0;
		send_to_char ("You decide against engaging in rental transactions.\n", ch);
		break;

    case DEL_PITCH:
        ch->delay = 0;
        ch->delay_obj = NULL;
        send_to_char ("You cease pitching the tent.\n\r", ch);
        break;

    case DEL_OOC:
        ch->delay = 0;
        send_to_char ("You decide against going to the OOC lounge.\n\r", ch);
        break;

    case DEL_STRING_BOW:
        ch->delay = 0;
        ch->delay_info1 = 0;
        send_to_char("You stop stringing your bow.\n\r", ch);
        break;

    case DEL_DESTRING_BOW:
        ch->delay = 0;
        ch->delay_info1 = 0;
        send_to_char("You stop destringing your bow.\n\r", ch);
        break;

    case DEL_FORAGE:
        ch->delay = 0;
        send_to_char ("You stop foraging.\n\r", ch);
        break;

    case DEL_RESTRING_BOW:
        ch->delay = 0;
        ch->delay_info1 = 0;
        send_to_char("You stop restringing your bow.\n\r", ch);
        break;

    case DEL_FORM:
        ch->delay = 0;
        send_to_char ("You stop ordering your followers in to formation.\n", ch);
        act ("$n halts ordering $s followers in to formation.\n", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        break;

    case DEL_SCOUT:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char ("You stop scouting a tree.\n\r", ch);
        break;

    case DEL_LOG1:
    case DEL_LOG2:
    case DEL_LOG3:
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char ("You stop chopping down the tree.\n\r", ch);
        act ("$n abruptly stops chopping down the tree.",
             false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        break;

    case DEL_WEAVESIGHT:
        ch->delay = 0;
        send_to_char
        ("You blink and shake your heard, ceasing your concentration.\n\r",
         ch);
        break;

    case DEL_RUMMAGE:
        ch->delay = 0;
        send_to_char ("You stop rummaging.\n\r", ch);
        break;

    case DEL_APP_APPROVE:
        ch->delay = 0;
        send_to_char ("You pass on this application.\n\r", ch);
        if (ch->pc->msg)
        {
            send_to_char ("Your prepared message was deleted.\n\r", ch);
            mem_free (ch->pc->msg);
            ch->pc->msg = NULL;
        }
        break;

    case DEL_SKIN_1:
    case DEL_SKIN_2:
    case DEL_SKIN_3:
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        act ("You abruptly end your skinning.", true, ch, 0, 0, TO_CHAR);
        act ("$n abruptly stops skinning.",
             false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        break;

    case DEL_BUTC_1:
    case DEL_BUTC_2:
    case DEL_BUTC_3:
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        act ("You abruptly stop butchering.", true, ch, 0, 0, TO_CHAR);
        act ("$n abruptly stops butchering.",
             false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        break;

    case DEL_COUNT_COIN:
        stop_counting (ch);
        break;

    case DEL_IDENTIFY:
        ch->delay = 0;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        act ("You stop trying to identify the flora.", true, ch, 0, 0, TO_CHAR);
        break;

    case DEL_GATHER:
        ch->delay = 0;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        act ("You stop gathering.", true, ch, 0, 0, TO_CHAR);
        break;

    case DEL_COMBINE:
        ch->delay_who = 0;
        ch->delay = 0;
        break;

    case DEL_GET_ALL:
        get_break_delay (ch);
        break;

    case DEL_EMPATHIC_HEAL:
        ch->delay = 0;
        ch->delay_ch = NULL;
        act ("You stop your healing concentration.", false, ch, 0, 0, TO_CHAR);
        break;

    case DEL_MENTAL_BOLT:
        ch->delay = 0;
        ch->delay_ch = NULL;
        act ("You stop your mental attack.", false, ch, 0, 0, TO_CHAR);
        break;

    case DEL_SEARCH:
        ch->delay = 0;
        act ("You stop searching.", false, ch, 0, 0, TO_CHAR);
        act ("$n stops searching.", true, ch, 0, 0, TO_ROOM);
        break;

    case DEL_PICK:
        ch->delay = 0;
        act ("You stop trying to pick the lock.", false, ch, 0, 0, TO_CHAR);
        break;

    case DEL_ALERT:
        ch->delay = 0;
        act ("You forget about responding to the alert.", false, ch, 0, 0,
             TO_CHAR);
        break;

    case DEL_INVITE:
        ch->delay = 0;
        send_to_char ("You decline to join.\n", ch);
        if (is_he_here (ch, ch->delay_ch, 1))
            act ("$N declines your offer.", false, ch->delay_ch, 0, ch, TO_CHAR);
        break;

    case DEL_CAMP1:
    case DEL_CAMP2:
    case DEL_CAMP3:
 case DEL_HOME_CAMP1:
        case DEL_HOME_CAMP2: 
        case DEL_HOME_CAMP3:   
    case DEL_CAMP4:
        ch->delay = 0;
        send_to_char ("You stop building your camp.\n", ch);
        act ("$n stops building $s camp.", true, ch, 0, 0, TO_ROOM);
        break;

    case DEL_TAKE:
        ch->delay = 0;
        send_to_char ("You stop trying to remove the object.\n", ch);
        act ("$n stops trying to get the object from the body.",
             false, ch, 0, 0, TO_ROOM);
        break;

    case DEL_PUTCHAR:
        ch->delay = 0;
        send_to_char ("You stop doing what you're doing.\n", ch);
        act ("$n 'stops doing what $e's doing.", false, ch, 0, 0, TO_ROOM);
        break;

    case DEL_STARE:
        ch->delay = 0;
        break;

    case DEL_HIDE:
        ch->delay = 0;
        send_to_char ("You stop trying to hide.\n", ch);
        break;

    case DEL_SCAN:
        ch->delay = 0;
        break;

    case DEL_QUICK_SCAN:
        ch->delay = 0;
        break;

    case DEL_HIDE_OBJ:
        ch->delay = 0;
        break;

    case DEL_PICK_OBJ:
        ch->delay = 0;
        break;

    case DEL_POISON_ITEM:
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_info3 = 0;
        send_to_char("You stop your poisoning.\n", ch);
        act ("$n stops $s nefarious poisoning.", false, ch, 0, 0, TO_ROOM);
        break;

    case DEL_QUAFF:
        ch->delay = 0;
        ch->delay_type = 0;
        obj = (OBJ_DATA *) ch->delay_info1;
        act ("You stop ingesting $p.", true, ch, obj, 0, TO_CHAR);
        act ("$n stops ingesting $p.", false, ch, obj, 0, TO_ROOM);
        ch->delay_info1 = 0;
        break;

    case DEL_COVER:
        ch->delay = 0;
        send_to_char ("You stop trying to take cover.\n", ch);
        act ("$n stops taking cover.", false, ch, 0, 0, TO_ROOM);
        break;

    case DEL_MEND1:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop mending the item.\n\r", ch);
        break;

    case DEL_MEND2:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop mending the item.\n\r", ch);
        break;

    case DEL_TRAP_ASS_1:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop assembling the trap.\n\r", ch);
        break;

    case DEL_TRAP_ASS_2:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop assembling the trap.\n\r", ch);
        break;

    case DEL_TRAP_DIS:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop disassembling the trap.\n\r", ch);
        break;

    case DEL_CHEM_CONCENTRATE_MIX:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_info3 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop mixing chemicals.\n\r", ch);
        break;

    case DEL_CHEM_COMBINE_MIX:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_info3 = 0;
        ch->delay_obj = NULL;
        if (ch->delay_who)
        {
            mem_free (ch->delay_who);
            ch->delay_who = NULL;
        }
        send_to_char("You stop mixing chemicals.\n\r", ch);
        break;

    case DEL_CHEM_INJECT:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_ch = NULL;

        if (ch->fighting)
            send_to_char("The blow halts your attempt to inject your target.\n\r", ch);
        else
            send_to_char("You stop attempting to inject your target.\n\r", ch);
        break;

    case DEL_POINTBLANK:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_ch = NULL;

        if (ch->fighting)
            send_to_char("The blow halts your attempt to place your firearm against your target.\n\r", ch);
        else
            send_to_char("You stop attempting to place your firearm against your target.\n\r", ch);
        break;


    case DEL_TRAP_SEARCH:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char("You stop detcting traps.\n\r", ch);
        break;

    case DEL_TRAP_DEFUSE:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop defusing the trap.\n\r", ch);
        break;

    case DEL_TRAP_EXAMINE:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop examining the trap.\n\r", ch);
        break;

    case DEL_LOAD_CLIP:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop loading the magazine.\n\r", ch);
        break;

    case DEL_UNLOAD_CLIP:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop unloading the magazine.\n\r", ch);
        break;

    case DEL_LOAD_FIREARM:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop loading the firearm.\n\r", ch);
        break;

    case DEL_UNLOAD_FIREARM:
        ch->delay = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ch->delay_obj = NULL;
        send_to_char("You stop unloading the firearm.\n\r", ch);
        break;

    case DEL_CLEAN:
        send_to_char("You cease cleaning.\n", ch);
        break;
    }

    clear_delays(ch);

    ch->delay_type = 0;
}

AFFECTED_TYPE *
is_room_affected (AFFECTED_TYPE * af, int type)
{
    while (af)
    {
        if (af->type == type)
            return af;

        af = af->next;
    }

    return NULL;
}

void
add_room_affect (AFFECTED_TYPE ** af, int type, int duration)
{
    AFFECTED_TYPE *raffect;

    if ((raffect = is_room_affected (*af, type)))
    {
        raffect->a.room.duration += duration;
        return;
    }

    raffect = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

    raffect->type = type;
    raffect->a.room.duration = duration;
    raffect->next = *af;

    *af = raffect;
}

int
is_in_room (CHAR_DATA * ch, CHAR_DATA * target)
{
    CHAR_DATA *tch;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch == target)
            return 1;
    }

    return 0;
}

int
add_registry (int reg_index, int value, const char *string)
{
    REGISTRY_DATA *new_reg;
    static int init = 0;

    for (; init < MAX_REGISTRY; init++)	/* Only executes once in */
        registry[init] = NULL;	/* the life of the game  */

    if (value == -1)
    {
        abort ();
    }

    CREATE (new_reg, REGISTRY_DATA, 1);

    new_reg->string = str_dup (string);
    new_reg->value = value;
    new_reg->next = registry[reg_index];
    registry[reg_index] = new_reg;

    return 0;
}

void
free_registry (void)
{
    int i = 0;

    while (registry[i])
        i++;
    i--;
    while (i)
    {
        mem_free (registry[i]->string);
        mem_free (registry[i]);
        i--;
    }

    mem_free (registry[0]->string);
    mem_free (registry[0]);
}

int
lookup_value (char *string, int reg_index)
{
    REGISTRY_DATA *reg;

    if (!string)
        return -1;

    for (reg = registry[reg_index]; reg; reg = reg->next)
        if (!str_cmp (string, reg->string))
            return reg->value;

    return -1;
}

char *
lookup_string (int value, int reg_index)
{
    REGISTRY_DATA *reg;

    for (reg = registry[reg_index]; reg; reg = reg->next)
        if (value == reg->value)
            return reg->string;

    return NULL;
}

void
reg_additional_spell_info (int sn, char *buf)
{
    char buf2[MAX_STRING_LENGTH];
    char *argument;

    argument = one_argument (buf, buf2);

    while (*buf2)
    {

        if (!str_cmp (buf2, "duration"))
        {
            argument = one_argument (argument, buf2);
            add_registry (REG_DURATION, atoi (buf2), spell_table[sn].name);
        }

        argument = one_argument (argument, buf2);
    }
}

void
reg_read_magic (FILE * fp_reg, char *buf)
{
    char buf2[MAX_STRING_LENGTH];
    char spell_name[MAX_STRING_LENGTH];
    char *argument;
    int sn = 0;

    const char *target_types[] =
    {
        "area", "offense", "defense", "self", "obj", "objinv",
        "secret", "ignore", "\n"
    };

    while (!feof (fp_reg))
    {

        fgets (buf, MAX_STRING_LENGTH, fp_reg);

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        if (!*buf || *buf == '#')
            continue;

        if (*buf == '+')
        {
            reg_additional_spell_info (sn, buf);
            continue;
        }

        argument = one_argument (buf, spell_name);

        if (!str_cmp (spell_name, "[end]"))
            return;

        if ((sn = spell_lookup (spell_name)) == -1)
        {
            printf ("Spell '%s' does not exist.\n", spell_name);
            abort ();
        }

        argument = one_argument (argument, buf2);

        if ((spell_table[sn].target = index_lookup (target_types, buf2)) == -1)
        {
            printf ("Spell '%s' has illegal target: %s\n", spell_name, buf2);
            abort ();
        }

        argument = one_argument (argument, buf2);

        if (!*buf2)
            continue;

        spell_table[sn].piety_cost = atoi (buf2);

        argument = one_argument (argument, buf2);

        if (!*buf2)
            continue;

        spell_table[sn].delay = atoi (buf2);

        argument = one_argument (argument, buf2);

        if (!*buf2)
            continue;

        spell_table[sn].msg_off = str_dup (buf2);
    }
}

void
reg_read_ov_lv_cap (FILE * fp_reg, char *buf)
{
    int sn;
    char *argument;
    char skill_name[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    while (!feof (fp_reg))
    {

        fgets (buf, MAX_STRING_LENGTH, fp_reg);

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        if (!*buf || *buf == '#')
            continue;

        argument = one_argument (buf, skill_name);

        if (!str_cmp (skill_name, "[end]"))
            return;

        if ((sn = index_lookup (skills, skill_name)) == -1)
        {
            printf ("Unknown skill name in registry file: %s\n", skill_name);
            abort ();
        }

        argument = one_argument (argument, buf2);

        add_registry (REG_OV, sn, buf2);

        argument = one_argument (argument, buf2);

        add_registry (REG_LV, sn, buf2);

        argument = one_argument (argument, buf2);

        add_registry (REG_CAP, sn, buf2);
    }
}

void
reg_read_skill_max_rates (FILE * fp_reg, char *buf)
{
    int sn;
    char *argument;
    char buf2[MAX_STRING_LENGTH];
    char skill_name[MAX_STRING_LENGTH];

    while (!feof (fp_reg))
    {

        fgets (buf, MAX_STRING_LENGTH, fp_reg);

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        if (!*buf || *buf == '#')
            continue;

        argument = one_argument (buf, skill_name);

        if (!str_cmp (buf, "[end]"))
            return;

        if ((sn = index_lookup (skills, skill_name)) == -1)
        {
            printf ("Unknown skill name in registry file: %s\n", skill_name);
            abort ();
        }

        argument = one_argument (argument, buf2);

        if (!isdigit (*buf2))
        {
            printf ("Invalid hourly rate for skill %s\n", skill_name);
            abort ();
        }

        add_registry (REG_MAX_RATES, sn, buf2);
    }
}

void
reg_read_variables (FILE * fp_reg, char *buf)
{
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char *argument;
    int value;

    while (!feof (fp_reg))
    {

        fgets (buf, MAX_STRING_LENGTH, fp_reg);

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        if (!*buf || *buf == '#')
            continue;

        argument = one_argument (buf, buf2);

        if (!str_cmp (buf2, "[end]"))
            return;

        argument = one_argument (argument, buf3);

        value = atoi (buf3);

        if (!value)
        {
            system_log ("ILLEGAL VARIABLE VALUE FOR VARIABLE:", true);
            system_log (buf2, true);
            system_log (buf3, true);
            abort ();
        }

        if (!str_cmp (buf2, "pulse_violence"))
            pulse_violence = value;
        else
        {
            system_log ("UNKNOWN VARIABLE:", true);
            system_log (buf2, true);
            abort ();
        }
    }
}

void
insert_combat_msg (COMBAT_MSG_DATA * cm)
{
    COMBAT_MSG_DATA *tcm;
    COMBAT_MSG_DATA *prev = NULL;

    if (!cm_list)
    {
        cm_list = cm;
        return;
    }

    if (cm->off_result <= cm_list->off_result)
    {
        cm->next = cm_list;
        cm_list = cm;
        return;
    }

    for (tcm = cm_list; tcm; tcm = tcm->next)
    {

        if (cm->off_result <= tcm->off_result)
            break;

        prev = tcm;
    }

    if (!tcm)
        prev->next = cm;
    else
    {
        cm->next = prev->next;
        prev->next = cm;
    }
}

void
add_combat_message (char *line)
{
    int r1;
    int r2;
    char *argument;
    char buf[MAX_STRING_LENGTH];

    static COMBAT_MSG_DATA *cm;
    static int party = 0;

    if (!party)
    {
        argument = one_argument (line, buf);

        if ((r1 = index_lookup (rs_name, buf)) == -1)
        {
            system_log ("Illegal 1st result in registry, line:", true);
            system_log (line, true);
            abort ();
        }

        argument = one_argument (argument, buf);

        if ((r2 = index_lookup (rs_name, buf)) == -1)
        {
            system_log ("Illegal 2nd result in registry, line:", true);
            system_log (line, true);
            abort ();
        }

        argument = one_argument (argument, buf);

        CREATE (cm, COMBAT_MSG_DATA, 1);

        *buf = toupper (*buf);

        if (*buf == 'I' || *buf == 'P' || *buf == 'B' ||
                *buf == 'D' || *buf == 'F' || *buf == '*')
            cm->table = *buf;
        else
        {
            system_log ("Illegal table name:  table, off-result:", true);
            system_log (buf, true);
            system_log (rs_name[r1], true);
            abort ();
        }

        cm->off_result = r1;
        cm->def_result = r2;
        cm->next = NULL;

    }
    else
        argument = line;

    while (isspace (*argument))
        argument++;

    if (party == 0)
        cm->def_msg = str_dup (argument);
    else if (party == 1)
        cm->off_msg = str_dup (argument);
    else
        cm->other_msg = str_dup (argument);

    if (party == 0)
        insert_combat_msg (cm);

    party++;

    if (party > 2)
        party = 0;
}


#define TABLE_USE_TABLE 		1
#define TABLE_COMBAT_MESSAGE	2

void
reg_read_tables (FILE * fp_reg, char *buf)
{
    int table_id = 0;
    int sn;
    char skill_name[MAX_STRING_LENGTH];
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

        if (!str_cmp (buf2, "[end]"))
            return;

        if (!strn_cmp (buf2, "end-", 4) && table_id)
        {
            table_id = 0;
            continue;
        }

        if (!table_id)
        {
            if (!str_cmp (buf2, "begin-use-table"))
                table_id = TABLE_USE_TABLE;
            else if (!str_cmp (buf2, "begin-combat-message-table"))
                table_id = TABLE_COMBAT_MESSAGE;
            else
            {
                system_log ("UNKNOWN TABLE NAME:", true);
                system_log (buf2, true);
                abort ();
            }

            continue;
        }

        if (table_id == TABLE_USE_TABLE)
        {
            if ((sn = index_lookup (skills, buf2)) == -1)
            {
                printf ("Unknown skill in USE-TABLE: %s\n", skill_name);
                abort ();
            }

            argument = one_argument (argument, buf2);

            if (!atoi (buf2))
            {
                system_log ("ILLEGAL/NO VALUE IN REGISTRY, USE-TABLE, SKILL:",
                            true);
                system_log (skills[sn], true);
                abort ();
            }

            use_table[sn].delay = atoi (buf2);
        }

        else if (table_id == TABLE_COMBAT_MESSAGE)
        {
            add_combat_message (buf);
        }
    }
}

void
read_spell (PHASE_DATA * phase, char *argument)
{
    int spell_type;
    int ind;
    int duration = 0;
    int power = 0;
    int target_flags = 0;
    char buf[MAX_STRING_LENGTH];

    if (phase->spell_type)
    {
        system_log ("TWO SPELL DEFINED IN ONE PHASE (Second one ignored):",
                    true);
        system_log (argument, true);
        return;
    }

    argument = one_argument (argument, buf);

    if ((spell_type = lookup_value (buf, REG_CRAFT_MAGIC)) == -1)
    {
        system_log ("ERROR ON SPELL LINE:", true);
        system_log (buf, true);
        system_log (argument, true);
        return;
    }

    while (*(argument = one_argument (argument, buf)))
    {

        if (!str_cmp (buf, "duration"))
        {
            argument = one_argument (argument, buf);
            duration = atoi (buf);
            if (!duration)
            {
                system_log ("DURATION PROBLEM IN CRAFT USING SPELL:", true);
                system_log (lookup_string (spell_type, REG_CRAFT_MAGIC), true);
                return;
            }
        }

        else if (!str_cmp (buf, "power"))
        {
            argument = one_argument (argument, buf);
            if (!isdigit (*buf))
            {
                system_log
                ("POWER HAS A PROBLEM IN CRAFT DEFINITION FOR SPELL:", true);
                system_log (lookup_string (spell_type, REG_CRAFT_MAGIC), true);
                return;
            }

            power = atoi (buf);
        }

        else if ((ind = index_lookup (targets, buf)) != -1)
            target_flags |= 1 << ind;

        else
        {
            system_log ("UNKNOWN KEYWORD IN SPELL DEFINITION FOR SPELL:", true);
            system_log (lookup_string (spell_type, REG_CRAFT_MAGIC), true);
            system_log (buf, true);
        }
    }

    phase->spell_type = spell_type;
    phase->duration = duration;
    phase->targets = target_flags;
    phase->power = power;
}

void
read_registry ()
{
    FILE *fp_reg;
    char buf[MAX_STRING_LENGTH];

    if (!(fp_reg = fopen (REGISTRY_FILE, "r")))
    {
        perror ("Unable to open registry!");
        abort ();
    }

    while (!feof (fp_reg))
    {

        fgets (buf, MAX_STRING_LENGTH, fp_reg);

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        if (!*buf || *buf == '#')
            continue;

        if (!str_cmp (buf, "[END]"))
            break;

        else if (!str_cmp (buf, "[MAGIC]"))
            reg_read_magic (fp_reg, buf);

        else if (!str_cmp (buf, "[OV-LV-CAP]"))
            reg_read_ov_lv_cap (fp_reg, buf);

        else if (!str_cmp (buf, "[VARIABLES]"))
            reg_read_variables (fp_reg, buf);

        else if (!str_cmp (buf, "[TABLES]"))
            reg_read_tables (fp_reg, buf);

        else if (!str_cmp (buf, "[SKILL-MAX-RATES]"))
            reg_read_skill_max_rates (fp_reg, buf);

        else if (!str_cmp (buf, "[CRAFTS]"))
            reg_read_crafts (fp_reg, buf);
    }

    fclose (fp_reg);
}

void
setup_registry (void)
{
    int i;

    add_registry (REG_REGISTRY, REG_REGISTRY, "Registry");
    add_registry (REG_REGISTRY, REG_SPELLS, "Spells");
    add_registry (REG_REGISTRY, REG_DURATION, "Duration");
    add_registry (REG_REGISTRY, REG_OV, "OV");
    add_registry (REG_REGISTRY, REG_LV, "LV");
    add_registry (REG_REGISTRY, REG_CAP, "Cap");
    add_registry (REG_REGISTRY, REG_MISC, "Misc");
    add_registry (REG_REGISTRY, REG_MAGIC_SPELLS, "Magic");
    add_registry (REG_REGISTRY, REG_MAX_RATES, "Rates");
    add_registry (REG_REGISTRY, REG_CRAFT_MAGIC, "Craftmagic");

    /*

    add_registry (REG_CRAFT_MAGIC, MAGIC_AKLASH_ODOR, "Aklash Odor");
    add_registry (REG_CRAFT_MAGIC, MAGIC_ROSE_SCENT, "Rose Scent");
    add_registry (REG_CRAFT_MAGIC, MAGIC_JASMINE_SCENT, "Jasmine Scent");
    add_registry (REG_CRAFT_MAGIC, MAGIC_SEWER_STENCH, "Sewer Stench");
    add_registry (REG_CRAFT_MAGIC, MAGIC_SOAP_AROMA, "Soap Aroma");

    add_registry (REG_CRAFT_MAGIC, MAGIC_CINNAMON_SCENT, "Cinnamon Scent");
    add_registry (REG_CRAFT_MAGIC, MAGIC_LEORTEVALD_STENCH,
                  "Leortevald Stench");
    add_registry (REG_CRAFT_MAGIC, MAGIC_YULPRIS_ODOR, "Yulpris Odor");
    add_registry (REG_CRAFT_MAGIC, MAGIC_FRESH_BREAD, "Fresh Bread");
    add_registry (REG_CRAFT_MAGIC, MAGIC_MOWN_HAY, "Mown Hay");

    add_registry (REG_CRAFT_MAGIC, MAGIC_FRESH_LINEN, "Fresh Linen");
    add_registry (REG_CRAFT_MAGIC, MAGIC_INCENSE_SMOKE, "Incense Smoke");
    add_registry (REG_CRAFT_MAGIC, MAGIC_WOOD_SMOKE, "Wood Smoke");
    */

    /* Add in all skills to craft magic */

    for (i = 1; i < LAST_SKILL; i++)
        add_registry (REG_CRAFT_MAGIC, MAGIC_SKILL_MOD_FIRST + i, skills[i]);

    add_registry (REG_SKILLS, SKILL_BRAWLING, "Brawling");
    add_registry (REG_SKILLS, SKILL_SMALL_BLADE, "Small-Blade");
    add_registry (REG_SKILLS, SKILL_LONG_BLADE, "Long-Blade");
    add_registry (REG_SKILLS, SKILL_POLEARM, "Polearm");
    add_registry (REG_SKILLS, SKILL_BLUDGEON, "Bludgeon");
    add_registry (REG_SKILLS, SKILL_DEFLECT, "Deflect");
    add_registry (REG_SKILLS, SKILL_DODGE, "Dodge");
    add_registry (REG_SKILLS, SKILL_SOLE_WIELD, "Sole-Wield");
    add_registry (REG_SKILLS, SKILL_DUAL_WIELD, "Dual-Wield");
    add_registry (REG_SKILLS, SKILL_AIM, "Aim");
    add_registry (REG_SKILLS, SKILL_HANDGUN, "Handgun");
    add_registry (REG_SKILLS, SKILL_RIFLE, "Rifle");
    add_registry (REG_SKILLS, SKILL_SMG, "Machinegun");
    add_registry (REG_SKILLS, SKILL_GUNNERY, "Gunnery");
    add_registry (REG_SKILLS, SKILL_ARCHERY, "Archery");

    add_registry (REG_SKILLS, SKILL_SNEAK, "Sneak");
    add_registry (REG_SKILLS, SKILL_HIDE, "Hide");
    add_registry (REG_SKILLS, SKILL_STEAL, "Steal");
    add_registry (REG_SKILLS, SKILL_PICK, "Picklock");
    add_registry (REG_SKILLS, SKILL_HAGGLE, "Haggle");
    add_registry (REG_SKILLS, SKILL_HANDLE, "Handle");
    add_registry (REG_SKILLS, SKILL_HUNTING, "Hunting");
    add_registry (REG_SKILLS, SKILL_FIRSTAID, "First-Aid");
    add_registry (REG_SKILLS, SKILL_MEDICINE, "Medicine");
    add_registry (REG_SKILLS, SKILL_FORAGE, "Scavenge");
    add_registry (REG_SKILLS, SKILL_EAVESDROP, "Eavesdrop");
    add_registry (REG_SKILLS, SKILL_BUTCHERY, "Butchery");

    add_registry (REG_SKILLS, SKILL_CHEMISTRY, "Chemistry");
    add_registry (REG_SKILLS, SKILL_MECHANICS, "Mechanics");
    add_registry (REG_SKILLS, SKILL_GUNSMITH, "Gunsmith");
    add_registry (REG_SKILLS, SKILL_COMPUTEROLOGY, "Computerology");
    add_registry (REG_SKILLS, SKILL_ELECTRONICS, "Electronics");
    add_registry (REG_SKILLS, SKILL_BIOLOGY, "Biology");
    add_registry (REG_SKILLS, SKILL_WEAPONCRAFT, "Weaponcraft");
    add_registry (REG_SKILLS, SKILL_ARMORCRAFT, "Armorcraft");
    add_registry (REG_SKILLS, SKILL_HANDICRAFT, "Handicraft");
    add_registry (REG_SKILLS, SKILL_ARTISTRY, "Artistry");

    add_registry (REG_SKILLS, SKILL_VOODOO, "Voodoo");
    add_registry (REG_SKILLS, SKILL_EDUCATION, "Education");
    add_registry (REG_SKILLS, SKILL_COMMON, "Common");


    add_registry (REG_MISC_NAMES, MISC_DELAY_OFFSET, "Delayoffset");
    add_registry (REG_MISC_NAMES, MISC_MAX_CARRY_N, "Maxcarry_n");
    add_registry (REG_MISC_NAMES, MISC_MAX_CARRY_W, "Maxcarry_w");
    add_registry (REG_MISC_NAMES, MISC_MAX_MOVE, "Maxmove");
    add_registry (REG_MISC_NAMES, MISC_STEAL_DEFENSE, "Stealdefense");

    add_registry (REG_MISC, MISC_DELAY_OFFSET, "(str + dex) / 6");
    add_registry (REG_MISC, MISC_MAX_CARRY_N, "dex / 3 + 4");
    add_registry (REG_MISC, MISC_MAX_CARRY_W, "str * 2500");
    add_registry (REG_MISC, MISC_MAX_MOVE, "(con + wil) / 2 * 5 + 25");
    add_registry (REG_MISC, MISC_STEAL_DEFENSE, "(int + dex) / 2");

    add_registry (REG_MAGIC_SPELLS, POISON_LETHARGY, "Lethargy");

    /*

    add_registry (REG_MAGIC_SPELLS, MAGIC_AKLASH_ODOR, "Aklash Odor");
    add_registry (REG_MAGIC_SPELLS, MAGIC_ROSE_SCENT, "Rose Scent");
    add_registry (REG_MAGIC_SPELLS, MAGIC_JASMINE_SCENT, "Jasmine Scent");
    add_registry (REG_MAGIC_SPELLS, MAGIC_SEWER_STENCH, "Sewer Stench");
    add_registry (REG_MAGIC_SPELLS, MAGIC_SOAP_AROMA, "Soap Aroma");
    add_registry (REG_MAGIC_SPELLS, MAGIC_CINNAMON_SCENT, "Cinnamon Scent");
    add_registry (REG_MAGIC_SPELLS, MAGIC_LEORTEVALD_STENCH,
                  "Leortevald Stench");
    add_registry (REG_MAGIC_SPELLS, MAGIC_YULPRIS_ODOR, "Yulpris Odor");
    add_registry (REG_MAGIC_SPELLS, MAGIC_FRESH_BREAD, "Fresh Bread");
    add_registry (REG_MAGIC_SPELLS, MAGIC_MOWN_HAY, "Mown Hay");
    add_registry (REG_MAGIC_SPELLS, MAGIC_FRESH_LINEN, "Fresh Linen");
    add_registry (REG_MAGIC_SPELLS, MAGIC_INCENSE_SMOKE, "Incense Smoke");
    add_registry (REG_MAGIC_SPELLS, MAGIC_WOOD_SMOKE, "Wood Smoke");
    */

    read_registry ();
}

void
load_dynamic_registry (void)
{
    FILE *fp_dr;
    NAME_SWITCH_DATA *last_name = NULL;
    NAME_SWITCH_DATA *name_switch;
    char *argument;
    char token[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (!(fp_dr = fopen (DYNAMIC_REGISTRY, "r")))
    {
        perror (DYNAMIC_REGISTRY);
        system_log ("Couldn't open DYNAMIC REGISTRY.", true);
        return;
    }

    while (fgets (buf, MAX_STRING_LENGTH - 1, fp_dr))
    {

        if (buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        argument = one_argument (buf, token);

        if (!str_cmp (token, "newclanname"))
        {

            name_switch =
                (struct name_switch_data *)
                alloc (sizeof (struct name_switch_data), 38);

            argument = one_argument (argument, token);
            name_switch->old_name = str_dup (token);

            argument = one_argument (argument, token);
            name_switch->new_name = str_dup (token);

            if (!last_name)
                clan_name_switch_list = name_switch;
            else
                last_name->next = name_switch;

            last_name = name_switch;

            continue;
        }
        /*
           if ( !str_cmp (token, "clandef") ) {
           add_clandef (argument);
           continue;
           }
         */
    }


    fclose (fp_dr);
    clan__do_load ();
}

void
morph_mob (CHAR_DATA * ch)
{
    char nbuf[MAX_STRING_LENGTH] = {'\0'};
    CHAR_DATA *newMob = NULL;
    OBJ_DATA *nobj;
    int temp_vnum = 0;
    int troom = 0;
    int jdex = 0;
    int flag = 0;
    AFFECTED_TYPE *af = NULL;

    if (ch->deleted)
        return;

    if (GET_POS (ch) == POSITION_FIGHTING
            || IS_SUBDUER (ch)
            || IS_SUBDUEE (ch)
            || IS_HITCHER (ch)
            || IS_HITCHEE (ch)
            || IS_RIDER (ch)
            || IS_RIDEE (ch)
            || (IS_DROWNING(ch))
            || (ch->following != nullptr)
            || !IS_NPC (ch)
            || IS_SUFFOCATING (ch))
        return;

    temp_vnum = ch->morphto;
    troom = ch->in_room;
    flag = ch->morph_type;

    if (temp_vnum <= 0)
    {
        sprintf (nbuf, "Mob %d has a morph clock, but no morph Mobvnum\n",
                 ch->mob->vnum);
        system_log (nbuf, true);
        return;
    }

    if (troom <= 0)
    {
        sprintf (nbuf, "Mob %d has a morph clock, and will morph to %d but there is an error with the room number\n",
                 ch->mob->vnum, ch->morphto);
        system_log (nbuf, true);
        return;
    }

    if (temp_vnum == 86)
    {
		OBJ_DATA *robot = NULL;
		if (ch->in_room && (robot = robot_deconstructor(ch, false)))
		{
			obj_to_room(robot, ch->in_room);
			act ("Powering down...", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n powers down.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
        extract_char (ch);
        return;
    }
    else
    {
        newMob = load_mobile (temp_vnum);
    }

    if (!newMob)
    {
        sprintf (nbuf, "Attempt to load target morph mob %d from mob %d failed\n",
                 ch->morphto, ch->mob->vnum);
        system_log (nbuf, true);
        send_to_gods(nbuf);
        return;
    }

    /********************
    morphtype = 1 will simply transfer gear to the new mob.
    morphtype = 2 will keep the same description, just change the skills

    *********************/
    if (flag == 1)
// physical morph includes new skills from new prototype
    {
        newMob->was_in_room = ch->was_in_room;
        newMob->last_room = ch->last_room;
        //newMob->hour_affects = ch->hour_affects;

        // Because of crash-bugs with writing soft data,
        // we'll manually recreate all the affects on the
        // new dude.
        for (af = ch->hour_affects; af; af = af->next)
        {
            affect_to_char(newMob, af);
        }
    }

// DELETED: flag two which copies over all the stats from the old mobile.
// VERY bad with new mobile code. This should work better.


    /******** objects and equip for all ***/
    for (jdex = 1; jdex < MAX_WEAR; jdex++)
    {
        if (get_equip (ch, jdex))
        {
            nobj = unequip_char (ch, jdex);
            obj_to_char (nobj, newMob);
            equip_char (newMob, nobj, jdex);
        }
    }

    if (ch->right_hand)
    {
        nobj = ch->right_hand;
        ch->right_hand = NULL;
        nobj->equiped_by = newMob;
        nobj->carried_by = newMob;
        newMob->right_hand = nobj;
    }

    if (ch->left_hand)
    {
        nobj = ch->left_hand;
        ch->left_hand = NULL;
        nobj->equiped_by = newMob;
        nobj->carried_by = newMob;
        newMob->left_hand = nobj;
    }


    newMob->act |= ACT_STAYPUT;
    extract_char (ch);
    char_to_room (newMob, troom);


}

int
can_learn (CHAR_DATA * ch)
{

    CHAR_DATA * src = NULL;
    int flag = 0;

    for (src = character_list; src; src = src->next)
    {
        if (src->morphto == ch->mob->vnum)
        {
            flag = 1;
            return (flag);
        }
    }
    return 0;
}


// Japheth's mem-fixes

// This function is only intended to be called by redefine_mobiles()
void char_data::partial_deep_copy (CHAR_DATA *proto)
{
    if (this->name)
    {
        mem_free(this->name);
    }
    this->name = str_dup(proto->name);

    if (this->short_descr)
    {
        mem_free(this->short_descr);
    }
    this->short_descr = str_dup(proto->short_descr);

    if (this->long_descr)
    {
        mem_free(this->long_descr);
    }
    this->long_descr = str_dup(proto->long_descr);

    if (this->description)
    {
        mem_free(this->description);
    }
    this->description = str_dup(proto->description);

    if (this->clans)
    {
        mem_free(this->clans);
    }
    this->clans = str_dup(proto->clans);

    this->act = proto->act;
    this->mob->damnodice = proto->mob->damnodice;
    this->mob->damsizedice = proto->mob->damsizedice;
    this->position = proto->position;
    this->default_pos = proto->default_pos;
    this->hmflags = proto->hmflags;

    this->str = proto->str;
    this->dex = proto->dex;
    this->intel = proto->intel;
    this->aur = proto->aur;
    this->con = proto->con;
    this->wil = proto->wil;
    this->agi = proto->agi;

    this->flags = proto->flags;
    this->shop = proto->shop;

    this->hit = proto->hit;
    this->max_hit = proto->max_hit;
    this->move = proto->move;
    this->max_move = proto->max_move;
    this->armor = proto->armor;
    this->offense = proto->offense;
    this->mob->damroll = proto->mob->damroll;
    this->ppoints = proto->ppoints;
    this->nat_attack_type = proto->nat_attack_type;

    this->sex = proto->sex;
    this->deity = proto->deity;

    this->circle = proto->circle;
    this->mob->skinned_vnum = proto->mob->skinned_vnum;
    this->mob->carcass_vnum = proto->mob->carcass_vnum;
    this->mob->merch_seven = proto->mob->merch_seven;
    this->mob->vehicle_type = proto->mob->vehicle_type;
    this->mob->helm_room = proto->mob->helm_room;
    this->natural_delay = proto->natural_delay;
    this->fight_mode = proto->fight_mode;
    this->race = proto->race;
    this->mob->access_flags = proto->mob->access_flags;
    this->speaks = proto->speaks;

    /*
    this->height				  = proto->height;
    this->frame				  = proto->frame;
    */
    this->age = proto->age;

    for (int i = 0; i < MAX_SKILLS; i++)
        this->skills[i] = proto->skills[i];

    this->str = proto->str;
    this->dex = proto->dex;
    this->con = proto->con;
    this->wil = proto->wil;
    this->aur = proto->aur;
    this->intel = proto->intel;

    this->mob->currency_type = proto->mob->currency_type;
}

void
char_data::clear_char ()
{
    this->in_room = 0;
    this->room = NULL;
    this->deleted = 0;
    this->circle = 0;
    this->fight_mode = 0;
    this->debug_mode = 0;
    this->primary_delay = 0;
    this->secondary_delay = 0;
    this->coldload_id = 0;
    this->natural_delay = 0;
    this->body_type = 0;
    this->scent_type = 0;
    this->nat_attack_type = 0;
    this->flags = 0;
    this->move_points = 0;
    this->hit_points = 0;
    this->speaks = 0;
    this->alarm = 0;
    this->trigger_delay = 0;
    this->trigger_line = 0;
    this->trigger_id = 0;
    this->psionic_talents = 0;
    this->subdue = NULL;
    this->prog = NULL;
    this->vartab = NULL;
    this->shop = NULL;
    this->vehicle = NULL;
    this->str = 0;
    this->intel = 0;
    this->wil = 0;
    this->dex = 0;
    this->con = 0;
    this->aur = 0;
    this->agi = 0;
    this->tmp_str = 0;
    this->tmp_intel = 0;
    this->tmp_wil = 0;
    this->tmp_dex = 0;
    this->tmp_con = 0;
    this->tmp_aur = 0;
    this->tmp_agi = 0;
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        this->skills[i] = 0;
    }
    this->hour_affects = NULL;
    this->equip = NULL;
    this->descriptor = NULL;
    this->next_in_room = NULL;
    this->next = NULL;
    this->next_fighting = NULL;
    this->next_assist = NULL;
    this->assist_pos = 0;
    this->following = NULL;
    this->pc = NULL;
    this->mob = NULL;
    this->moves = NULL;
    this->casting_arg = NULL;
    this->hit = 0;
    this->max_hit = 0;
    this->move = 0;
    this->max_move = 0;
    this->armor = 0;
    this->offense = 0;
    this->ppoints = 0;
    this->fighting = NULL;
    this->distance_to_target = 0;
    this->remembers = NULL;
    this->affected_by = 0;
    this->position = 8;
    this->default_pos = 8;
    this->act = 0;
    this->hmflags = 0;
    this->carry_weight = 0;
    this->carry_items = 0;
    this->delay_type = 0;
    this->delay = 0;
    this->delay_who = NULL;
    this->delay_who2 = NULL;
    this->delay_who3 = NULL;
    this->delay_who4 = NULL;
    this->delay_ch = NULL;
    this->delay_obj = NULL;
    this->delay_info1 = 0;
    this->delay_info2 = 0;
    this->delay_info3 = 0;
    this->delay_info4 = 0;
    this->delay_info5 = 0;
    this->was_in_room = 0;
    this->intoxication = 0;
    this->hunger = 0;
    this->thirst = 0;
    this->last_room = 0;
    this->attack_type = 0;
    this->name = NULL;
    this->tname = NULL;
    this->short_descr = NULL;
    this->long_descr = NULL;
    this->pmote_str = NULL;
    this->voice_str = NULL;
    this->description = NULL;
    this->sex = 0;
    this->deity = 0;
    this->race = 0;
    this->color = 0;
    this->speed = 0;
    this->age = 0;
    this->height = 0;
    this->frame = 0;
    this->totem = 0;
    this->time.birth = 0;
    this->time.logon = 0;
    this->time.played = 0;
    this->clans = NULL;
    this->mount = NULL;
    this->hitcher = NULL;
    this->hitchee = NULL;
    this->combat_log = NULL;
    this->wounds = NULL;
    this->damage = 0;
    this->lastregen = 0;
    this->defensive = 0;
    this->cell_1 = 0;
    this->cell_2 = 0;
    this->cell_3 = 0;
    this->laststuncheck = 0;
    this->knockedout = 0;
    this->writes = 0;
    this->stun = 0;
    this->curse = 0;
    this->aiming_at = NULL;
    this->aim = 0;
    this->lodged = NULL;
    this->shock = 0;
    this->max_shock = 0;
    this->harness = 0;
    this->max_harness = 0;
    this->preparing_id = 0;
    this->preparing_time = 0;
    this->enchantments = NULL;
    this->roundtime = 0;
    this->right_hand = NULL;
    this->left_hand = NULL;
    this->plr_flags = 0;
    this->ranged_enemy = NULL;
    this->enemy_direction = NULL;
    this->threats = NULL;
    this->attackers = NULL;
    this->whirling = 0;
    this->from_dir = 0;
    this->sighted = NULL;
    this->scent = NULL;
    this->balance = 0;
    this->travel_str = NULL;
    for (int i = 0; i < MAX_LEARNED_SPELLS; i++)
    {
        this->spells[i][0] = 0;
        this->spells[i][1] = 0;
    }
    this->body_proto = 0;
    this->bmi = 0;
    this->size = 0;
    this->guardian_mode = 0;
    this->hire_storeroom = 0;
    this->hire_storeobj = 0;
    this->dmote_str = NULL;
    this->cover_from_dir = 0;
    this->morph_type = 0;
    this->clock = 0;
    this->morph_time = 0;
    this->morphto = 0;
    this->craft_index = 0;
    for (int i = 0; i < 100; i++)
    {
        this->enforcement[i] = 0;
    }
    this->plan = NULL;
    this->goal = NULL;
    this->bleeding_prompt = false;
    this->talents = 0;
    this->room_pos = 0;
    this->formation = 0;
    this->group = NULL;
    this->talents = 0;
    this->effort = 0;
    this->combat_block = 0;

    // Variable descs.

    this->d_age = NULL;
    this->d_eyes = NULL;
    this->d_hairlength = NULL;
    this->d_haircolor = NULL;
    this->d_hairstyle = NULL;
    this->d_height = NULL;
    this->d_frame = NULL;
    this->d_feat1 = NULL;
    this->d_feat2 = NULL;
    this->d_feat3 = NULL;
    this->d_feat4 = NULL;

    this->controlled_by = NULL;
    this->controlling = NULL;

	//this->targeted_by = new std::vector<targeted_bys*>;
	this->targeted_by = NULL;

	this->over_enemies = new std::vector<overwatch_enemy*>;

    this->over_target = 0;
    this->over_thresh = 0;

    this->defense_mode = 0;

    this->death_ch = NULL;
    this->death_obj = NULL;
	this->purchase_obj = NULL;
	this->descriptor = NULL;
}

char_data::char_data ()
{
    this->clear_char();
}

char_data::~char_data ()
{
    VAR_DATA *var;
    struct memory_data *mem;
    if (this->pc)
    {
        delete this->pc;
        this->pc = NULL;
    }
    if (this->mob)
    {
        delete this->mob;
        this->mob = NULL;
    }

    if (this->clans && *this->clans)
    {
        mem_free (this->clans);
        this->clans = NULL;
    }

    if (this->combat_log && *this->combat_log)
    {
        mem_free (this->combat_log);
        this->combat_log = NULL;
    }

    if (this->enemy_direction && *this->enemy_direction)
    {
        mem_free (this->enemy_direction);
        this->enemy_direction = NULL;
    }

    if (this->delay_who && !isdigit (*this->delay_who) && *this->delay_who)
    {
        mem_free (this->delay_who);
        this->delay_who = NULL;
    }

    if (this->delay_who2 && !isdigit (*this->delay_who2) && *this->delay_who2)
    {
        mem_free (this->delay_who2);
        this->delay_who2 = NULL;
    }

    if (this->delay_who3 && !isdigit (*this->delay_who3) && *this->delay_who3)
    {
        mem_free (this->delay_who3);
        this->delay_who3 = NULL;
    }

    if (this->delay_who4 && !isdigit (*this->delay_who4) && *this->delay_who4)
    {
        mem_free (this->delay_who4);
        this->delay_who4 = NULL;
    }

    if (this->voice_str && *this->voice_str)
    {
        mem_free (this->voice_str);
        this->voice_str = NULL;
    }

    if (this->travel_str && *this->travel_str)
    {
        mem_free (this->travel_str);
        this->travel_str = NULL;
    }

    if (this->pmote_str && *this->pmote_str)
    {
        mem_free (this->pmote_str);
        this->pmote_str = NULL;
    }

    if (this->plan)
    {
        delete this->plan;
        this->plan = 0;
    }
    if (this->goal)
    {
        delete this->goal;
        this->goal = 0;
    }

    while (this->vartab)
    {

        var = this->vartab;
        this->vartab = var->next;

        if (var->name && *var->name)
        {
            mem_free (var->name);
            var->name = NULL;
        }

        mem_free (var);
        var = NULL;
    }

    while (this->remembers)
    {
        mem = this->remembers;
        this->remembers = mem->next;
        mem_free (mem);
        mem = NULL;
    }

    while (this->wounds)
        wound_from_char (this, this->wounds);

    while (this->lodged)
        lodge_from_char (this, this->lodged);

    while (this->attackers)
        attacker_from_char (this, this->attackers);

    while (this->threats)
        threat_from_char (this, this->threats);
    //this->threats.clear();

    while (this->hour_affects)
        affect_remove (this, this->hour_affects);

    while (this->scent)
        remove_mob_scent(this, this->scent);

	remove_all_targeted(this);
	remove_all_overwatch(this);

    clear_pmote(this);
    clear_dmote(this);


    if (this->controlled_by)
    {
        this->controlled_by->controlling = NULL;
    }
    if (this->controlling)
    {
        this->controlling->controlled_by = NULL;
    }

    if (this->mob)
    {
        CHAR_DATA* proto = vnum_to_mob(this->mob->vnum);
        if (proto)
        {
            if (this->tname && *this->tname
                    && this->tname != proto->tname)
            {
                mem_free (this->tname);
                this->tname = NULL;
            }

            if (this->name && *this->name
                    && this->name != proto->name)
            {
                mem_free (this->name);
                this->name = NULL;
            }

            if (this->short_descr && *this->short_descr
                    && this->short_descr != proto->short_descr )
            {
                mem_free (this->short_descr);
                this->short_descr = NULL;
            }

            if (this->long_descr && *this->long_descr
                    && this->long_descr != proto->long_descr)
            {
                mem_free (this->long_descr);
                this->long_descr = NULL;
            }

            if (this->description && *this->description
                    && this->description != proto->description)
            {
                mem_free (this->description);
                this->description = NULL;
            }

            if (this->d_age && *this->d_age
                    && this->d_age != proto->d_age)
            {
                mem_free (this->d_age);
                this->d_age = NULL;
            }

            if (this->d_eyes && *this->d_eyes
                    && this->d_eyes != proto->d_eyes)
            {
                mem_free (this->d_eyes);
                this->d_eyes = NULL;
            }

            if (this->d_hairlength && *this->d_hairlength
                    && this->d_hairlength != proto->d_hairlength)
            {
                mem_free (this->d_hairlength);
                this->d_hairlength = NULL;
            }

            if (this->d_haircolor && *this->d_haircolor
                    && this->d_haircolor != proto->d_haircolor)
            {
                mem_free (this->d_haircolor);
                this->d_haircolor = NULL;
            }

            if (this->d_hairstyle && *this->d_hairstyle
                    && this->d_hairstyle != proto->d_hairstyle)
            {
                mem_free (this->d_hairstyle);
                this->d_hairstyle = NULL;
            }

            if (this->d_height && *this->d_height
                    && this->d_height != proto->d_height)
            {
                mem_free (this->d_height);
                this->d_height = NULL;
            }

            if (this->d_frame && *this->d_frame
                    && this->d_frame != proto->d_frame)
            {
                mem_free (this->d_frame);
                this->d_frame = NULL;
            }

            if (this->d_feat1 && *this->d_feat1
                    && this->d_feat1 != proto->d_feat1)
            {
                mem_free (this->d_feat1);
                this->d_feat1 = NULL;
            }

            if (this->d_feat2 && *this->d_feat2
                    && this->d_feat2 != proto->d_feat2)
            {
                mem_free (this->d_feat1);
                this->d_feat2 = NULL;
            }

            if (this->d_feat3 && *this->d_feat3
                    && this->d_feat3 != proto->d_feat3)
            {
                mem_free (this->d_feat3);
                this->d_feat3 = NULL;
            }

            if (this->d_feat4 && *this->d_feat4
                    && this->d_feat4 != proto->d_feat4)
            {
                mem_free (this->d_feat4);
                this->d_feat4 = NULL;
            }

        }
        else
        {
            fprintf (stderr, "Proto not defined for NPC %d?\n", this->mob->vnum);
        }
    }
    else
    {
        if (this->tname && *this->tname )
        {
            mem_free (this->tname);
            this->tname = NULL;
        }

        if (this->name && *this->name )
        {
            mem_free (this->name);
            this->name = NULL;
        }

        if (this->short_descr && *this->short_descr)
        {
            mem_free (this->short_descr);
            this->short_descr = NULL;
        }

        if (this->long_descr && *this->long_descr)
        {
            mem_free (this->long_descr);
            this->long_descr = NULL;
        }

        if (this->description && *this->description)
        {
            mem_free (this->description);
            this->description = NULL;
        }

        if (this->d_age && *this->d_age)
        {
            mem_free (this->d_age);
            this->d_age = NULL;
        }

        if (this->d_eyes && *this->d_eyes)
        {
            mem_free (this->d_eyes);
            this->d_eyes = NULL;
        }

        if (this->d_hairlength && *this->d_hairlength)
        {
            mem_free (this->d_hairlength);
            this->d_hairlength = NULL;
        }

        if (this->d_haircolor && *this->d_haircolor)
        {
            mem_free (this->d_haircolor);
            this->d_haircolor = NULL;
        }

        if (this->d_hairstyle && *this->d_hairstyle)
        {
            mem_free (this->d_hairstyle);
            this->d_hairstyle = NULL;
        }

        if (this->d_height && *this->d_height)
        {
            mem_free (this->d_height);
            this->d_height = NULL;
        }

        if (this->d_frame && *this->d_frame)
        {
            mem_free (this->d_frame);
            this->d_frame = NULL;
        }

        if (this->d_feat1 && *this->d_feat1)
        {
            mem_free (this->d_feat1);
            this->d_feat1 = NULL;
        }

        if (this->d_feat2 && *this->d_feat2)
        {
            mem_free (this->d_feat1);
            this->d_feat2 = NULL;
        }

        if (this->d_feat3 && *this->d_feat3)
        {
            mem_free (this->d_feat3);
            this->d_feat3 = NULL;
        }

        if (this->d_feat4 && *this->d_feat4)
        {
            mem_free (this->d_feat4);
            this->d_feat4 = NULL;
        }
    }
}

pc_data::pc_data()
{
    this->dreams = NULL;
    this->dreamed = NULL;
    this->aliases = NULL;
    this->execute_alias = NULL;
    this->nanny_state = 0;
    this->role = 0;
    this->admin_loaded = false;
    this->creation_comment = NULL;
    this->imm_enter = NULL;
    this->imm_leave = NULL;
    this->site_lie = NULL;
    this->account_name = NULL;
    this->msg = NULL;
    this->edit_player = NULL;
    this->target_mob = NULL;
    this->dot_shorthand = NULL;
    this->last_global_staff_msg = 0;
    this->staff_notes = 0;
    this->mortal_mode = 0;
    this->create_state = 0;
    this->edit_obj = 0;
    this->edit_mob = 0;
    this->load_count = 0;
    this->start_str = 0;
    this->start_dex = 0;
    this->start_con = 0;
    this->start_wil = 0;
    this->start_aur = 0;
    this->start_intel = 0;
    this->start_agi = 0;
    this->level = 0;
    this->boat_virtual = 0;
    this->mount_speed = 0;
    this->time_last_activity = 0;
    this->is_guide = 0;
    this->profession = 0;
    this->app_cost = 0;
    this->chargen_flags = 0;
    this->last_global_pc_msg = 0;
    this->sleep_needed = 0;
    this->auto_toll = 0;
    this->doc_type = 0;
    this->doc_index = -1;
    this->writing_on = NULL;
    this->special_role = NULL;
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        this->skills[i] = 0;
    }
    this->owner = NULL;
    this->last_logon = 0;
    this->last_logoff = 0;
    this->last_disconnect = 0;
    this->last_connect = 0;
    this->last_died = 0;
    this->edit_craft = NULL;
    this->power_level = 0;
}


pc_data::~pc_data()
{
    DREAM_DATA *dream;
    ALIAS_DATA *tmp_alias;
    ROLE_DATA *trole;

    while (this->aliases)
    {
        tmp_alias = this->aliases;
        this->aliases = this->aliases->next_alias;
        alias_free (tmp_alias);
    }
    while (this->dreams)
    {
        dream = this->dreams;

        this->dreams = this->dreams->next;

        if (dream->dream && *dream->dream)
        {
            mem_free (dream->dream);
            dream->dream = NULL;
        }

        mem_free (dream);
        dream = NULL;
    }
    while (this->dreamed)
    {
        dream = this->dreamed;
        this->dreamed = this->dreamed->next;
        if (dream->dream && *dream->dream)
        {
            mem_free (dream->dream);
            dream->dream = NULL;
        }
        mem_free (dream);
        dream = NULL;
    }

    if ((trole = this->special_role) != NULL)
    {
        if (trole->summary && *trole->summary)
            mem_free (trole->summary);
        if (trole->body && *trole->body)
            mem_free (trole->body);
        if (trole->date && *trole->date)
            mem_free (trole->date);
        if (trole->poster && *trole->poster)
            mem_free (trole->poster);
        mem_free (trole);
        this->special_role = NULL;
    }

    if (this->account_name && *this->account_name)
    {
        mem_free (this->account_name);
        this->account_name = NULL;
    }

    if (this->site_lie && *this->site_lie)
    {
        mem_free (this->site_lie);
        this->site_lie = NULL;
    }

    if (this->imm_leave && *this->imm_leave)
    {
        mem_free (this->imm_leave);
        this->imm_leave = NULL;
    }

    if (this->imm_enter && *this->imm_enter)
    {
        mem_free (this->imm_enter);
        this->imm_enter = NULL;
    }

    if (this->creation_comment && *this->creation_comment)
    {
        mem_free (this->creation_comment);
        this->creation_comment = NULL;
    }

    if (this->msg && *this->msg)
    {
        mem_free (this->msg);
        this->msg = NULL;
    }

    this->owner = NULL;
}

mob_data::mob_data()
{
    this->owner = NULL;
    this->hnext = NULL;
    this->lnext = NULL;
    this->resets = NULL;
    this->cues = NULL;
    this->skinned_vnum = 0;
    this->carcass_vnum = 0;
    this->damnodice = 0;
    this->damroll = 0;
    this->min_height = 0;
    this->max_height = 0;
    this->size = 0;
    this->vnum = 0;
    this->zone = 0;
    this->spawnpoint = 0;
    this->merch_seven = 0;
    this->vehicle_type = 0;
    this->helm_room = 0;
    this->access_flags = 0;
    this->noaccess_flags = 0;
    this->reset_zone = 0;
    this->reset_cmd = 0;
    this->currency_type = 0;
    this->jail = 0;
    this->resets = NULL;
    this->fallback = 0;
    this->controller = 0;
    this->armortype = 0;
	this->ai_delay = 0;
}

mob_data::~mob_data()
{
    if (this->owner && *this->owner)
    {
        mem_free(this->owner);
    }
}

void char_data::deep_copy (CHAR_DATA *copy_from)
{
// Lazy way of getting everything non-dynamic across. One advantage of this approach is that people who add members to char_data don't have to add them here unless they use dynamic memory.
    mob_data *tmob = this->mob;
    pc_data *tpc = this->pc;
    memcpy (this, copy_from, sizeof(CHAR_DATA));
    this->mob = tmob;
    this->pc = tpc;

    if (copy_from->delay_who)
    {
        this->delay_who = str_dup(copy_from->delay_who);
    }

    if (copy_from->delay_who2)
    {
        this->delay_who2 = str_dup(copy_from->delay_who2);
    }

    if (copy_from->delay_who3)
    {
        this->delay_who3 = str_dup(copy_from->delay_who3);
    }

    if (copy_from->delay_who4)
    {
        this->delay_who4 = str_dup(copy_from->delay_who4);
    }

    if (copy_from->casting_arg)
    {
        this->casting_arg = str_dup(copy_from->casting_arg);
    }

    if (copy_from->name)
    {
        this->name = str_dup(copy_from->name);
    }

    if (copy_from->tname)
    {
        this->tname = str_dup(copy_from->tname);
    }

    if (copy_from->short_descr)
    {
        this->short_descr = str_dup(copy_from->short_descr);
    }

    if (copy_from->long_descr)
    {
        this->long_descr = str_dup(copy_from->long_descr);
    }

    if (copy_from->pmote_str)
    {
        this->pmote_str = str_dup(copy_from->pmote_str);
    }

    if (copy_from->voice_str)
    {
        this->voice_str = str_dup(copy_from->voice_str);
    }

    if (copy_from->description)
    {
        this->description = str_dup(copy_from->description);
    }

    if (copy_from->d_age)
    {
        this->d_age = str_dup(copy_from->d_age);
    }

    if (copy_from->d_eyes)
    {
        this->d_eyes = str_dup(copy_from->d_eyes);
    }

    if (copy_from->d_haircolor)
    {
        this->d_haircolor = str_dup(copy_from->d_haircolor);
    }

    if (copy_from->d_hairlength)
    {
        this->d_hairlength = str_dup(copy_from->d_hairlength);
    }

    if (copy_from->d_hairstyle)
    {
        this->d_hairstyle = str_dup(copy_from->d_hairstyle);
    }

    if (copy_from->d_frame)
    {
        this->d_frame = str_dup(copy_from->d_frame);
    }

    if (copy_from->d_height)
    {
        this->d_height = str_dup(copy_from->d_height);
    }

    if (copy_from->d_feat1)
    {
        this->d_feat1 = str_dup(copy_from->d_feat1);
    }

    if (copy_from->d_feat2)
    {
        this->d_feat2 = str_dup(copy_from->d_feat2);
    }

    if (copy_from->d_feat3)
    {
        this->d_feat3 = str_dup(copy_from->d_feat3);
    }

    if (copy_from->d_feat4)
    {
        this->d_feat4 = str_dup(copy_from->d_feat4);
    }

    if (copy_from->clans)
    {
        this->clans = str_dup(copy_from->clans);
    }

    if (copy_from->enemy_direction)
    {
        this->enemy_direction = str_dup(copy_from->enemy_direction);
    }

    if (copy_from->combat_log)
    {
        this->combat_log = str_dup(copy_from->combat_log);
    }

    if (copy_from->travel_str)
    {
        this->travel_str = str_dup(copy_from->travel_str);
    }

    if (copy_from->dmote_str)
    {
        this->dmote_str = str_dup(copy_from->dmote_str);
    }

    if (copy_from->plan)
    {
        this->plan = new std::string (*copy_from->plan);
    }

    if (copy_from->goal)
    {
        this->goal = new std::string (*copy_from->goal);
    }

	this->over_enemies = new std::vector<overwatch_enemy*>;

    if (copy_from->pc && this->pc)
    {
        this->pc->deep_copy(copy_from->pc);
    }

    if (copy_from->mob && this->mob)
    {
        this->mob->deep_copy(copy_from->mob);
    }
}

void pc_data::deep_copy (pc_data *copy_from)
{
    memcpy(this, copy_from, sizeof(pc_data));

    if (copy_from->creation_comment)
    {
        this->creation_comment = str_dup(copy_from->creation_comment);
    }

    if (copy_from->imm_enter)
    {
        this->imm_enter = str_dup(copy_from->imm_enter);
    }

    if (copy_from->imm_leave)
    {
        this->imm_leave = str_dup(copy_from->imm_leave);
    }

    if (copy_from->site_lie)
    {
        this->site_lie = str_dup(copy_from->site_lie);
    }

    if (copy_from->account_name)
    {
        this->account_name = str_dup(copy_from->account_name);
    }

    if (copy_from->msg)
    {
        this->msg = str_dup(copy_from->msg);
    }
}

void mob_data::deep_copy (mob_data *copy_from)
{
    memcpy(this, copy_from, sizeof(mob_data));

    if (copy_from->owner)
    {
        this->owner = str_dup(copy_from->owner);
    }
}

DESCRIPTOR_DATA * char_data::descr ()
{
    if (controlled_by)
    {
        return controlled_by->descr();
    }

    return this->descriptor;
};
