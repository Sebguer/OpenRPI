/*------------------------------------------------------------------------\
|  act.offensive.c : Violence Module                  www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <string>
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

// 04/08/09: Added in who_attacker, which randomly returns one person attacking ch

// Wasn't any function to quickly determine how many people are attacking somone,
// so here it is. Run through a list of people in the room, and if they're attacking
// ch, increase the counter.

int
num_attackers (CHAR_DATA * ch)
{

    CHAR_DATA * tch = NULL;
    int i = 0;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->fighting == ch)
        {
            i += 1;
        }
    }

    return i;
}

CHAR_DATA *
who_attackers (CHAR_DATA * ch)
{
    CHAR_DATA * rch = NULL;
    int j = 0;

    j = num_attackers(ch);

    if (!j)
        return NULL;

    for (rch = ch->room->people; rch; rch = rch->next_in_room)
    {
        if (rch->fighting == ch)
        {
            if (j == 1)
                return rch;
            else if (number (0,j))
                return rch;
            j--;
        }
    }

    return NULL;

}



/* ch here is the victim mob or PC */
void
notify_guardians (CHAR_DATA * ch, CHAR_DATA * tch, int cmd)
{
    unsigned short int flag = 0x00;
    char buf[AVG_STRING_LENGTH];
    char *strViolation[] =
    {
        "Hits to injure", "Hits to kill", "Hits to injure", "Took aim at",
        "Slings at", "Throws at",
        "Attempts to steal from", "Attempts to pick the lock of"
    };


    if (!ch || !tch || IS_NPC (ch)	/* ignore attacks by npcs */
            || IS_SET (ch->flags, FLAG_GUEST)
            || (IS_SET (tch->act, ACT_PREY) && cmd >= 3)
            || GET_TRUST (ch)	/* ignore attacks by imms */
            || GET_TRUST (tch)	/* ignore attacks on imms */
       )
    {

        return;

    }

    flag |= (!IS_NPC (tch)) ? (GUARDIAN_PC) : 0;
    flag |= (lookup_race_int(tch->race, RACE_PC)) ? (GUARDIAN_NPC_HUMANOIDS) : (GUARDIAN_NPC_WILDLIFE);
    flag |= (tch->shop) ? (GUARDIAN_NPC_SHOPKEEPS) : 0;
    flag |= (IS_SET (tch->act, ACT_SENTINEL)) ? (GUARDIAN_NPC_SENTINELS) : 0;
    flag |= (IS_SET (tch->act, ACT_ENFORCER)) ? (GUARDIAN_NPC_ENFORCERS) : 0;
    flag |= ((tch->right_hand && GET_ITEM_TYPE (tch->right_hand) == ITEM_KEY)
             || (tch->left_hand
                 && GET_ITEM_TYPE (tch->left_hand) ==
                 ITEM_KEY)) ? (GUARDIAN_NPC_KEYHOLDER) : 0;

    if (ch->in_room == tch->in_room)
    {

        sprintf (buf, "#3[Guardian: %s%s]#0 %s %s%s in %d.",
                 GET_NAME (ch),
                 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                 strViolation[cmd],
                 (!IS_NPC (tch)) ? GET_NAME (tch) : (tch->short_descr),
                 (!IS_NPC (tch)
                  && IS_SET (tch->plr_flags,
                             NEW_PLAYER_TAG)) ? " (new)" : (IS_SET (flag,
                                                                    GUARDIAN_NPC_KEYHOLDER)
                                                            ? " (keyholder)"
                                                            : (IS_SET
                                                               (flag,
                                                                GUARDIAN_NPC_SHOPKEEPS)
                                                               ? " (shopkeeper)"
                                                               : (IS_SET
                                                                  (flag,
                                                                   GUARDIAN_NPC_ENFORCERS)
                                                                  ?
                                                                  " (enforcer)"
                                                                  : (IS_SET
                                                                     (flag,
                                                                      GUARDIAN_NPC_SENTINELS)
                                                                     ?
                                                                     " (sentinel)"
                                                                     : "")))),
                 tch->in_room);

    }
    else
    {

        sprintf (buf, "#3[Guardian: %s%s]#0 %s %s%s in %d, from %d.#0",
                 GET_NAME (ch),
                 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
                 strViolation[cmd],
                 (!IS_NPC (tch)) ? GET_NAME (tch) : (tch->short_descr),
                 (!IS_NPC (tch)
                  && IS_SET (tch->plr_flags,
                             NEW_PLAYER_TAG)) ? " (new)" : (IS_SET (flag,
                                                                    GUARDIAN_NPC_KEYHOLDER)
                                                            ? " (keyholder)"
                                                            : (IS_SET
                                                               (flag,
                                                                GUARDIAN_NPC_SHOPKEEPS)
                                                               ? " (shopkeeper)"
                                                               : (IS_SET
                                                                  (flag,
                                                                   GUARDIAN_NPC_ENFORCERS)
                                                                  ?
                                                                  " (enforcer)"
                                                                  : (IS_SET
                                                                     (flag,
                                                                      GUARDIAN_NPC_SENTINELS)
                                                                     ?
                                                                     " (sentinel)"
                                                                     : "")))),
                 tch->in_room, ch->in_room);
    }
    buf[11] = toupper (buf[11]);
    send_to_guardians (buf, flag);
}

void
do_throw (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *tobj, *armor1 = NULL, *armor2 = NULL;
    ROOM_DATA *troom = NULL;
    ROOM_DIRECTION_DATA *exit = NULL;
    CHAR_DATA *tch = NULL;
    AFFECTED_TYPE *af;
    bool can_lodge = false, ranged = false;
    int dir = 0, result = 0, location = 0;
    int wear_loc1 = 0, wear_loc2 = 0, wound_type = 0;
    float damage = 0;
    int range_mod = 0;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    char strike_location[MAX_STRING_LENGTH];
    int poison = 0;
    bool aimed_room = false;

    const char *verbose_dirs[] =
    {
        "the north",
        "the east",
        "the south",
        "the west",
        "above",
        "below",
        "outside",
        "inside",
        "the northeast",
        "the northwest",
        "the southeast",
        "the southwest",
        "\n"
    };

    if (IS_SWIMMING (ch))
    {
        send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

    if (IS_FLOATING (ch))
    {
        send_to_char("You can't do that while floating!\n", ch);
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

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("What did you wish to throw?\n", ch);
        return;
    }

    if (!(tobj = get_obj_in_list (buf, ch->right_hand)) &&
            !(tobj = get_obj_in_list (buf, ch->left_hand)))
    {
        send_to_char ("You aren't holding that in either hand.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("At what did you wish to throw?\n", ch);
        return;
    }

    if ((dir = is_direction (buf)) == -1)
    {
        if (!str_cmp(buf, "room"))
        {
            aimed_room = true;

        }
        else if (!(tch = get_char_room_vis (ch, buf)))
        {
            send_to_char ("At what did you wish to throw?\n", ch);
            return;
        }

		/*
        if (obj && GET_ITEM_TYPE(obj) != ITEM_COVER && GET_ITEM_TYPE(obj) != ITEM_TABLE)
        {
            send_to_char ("You can only target a grenade at a piece of cover or furniture.\n", ch);
            return;
        }
		*/
    }

    if (ch->fighting)
    {
        send_to_char
        ("You are currently engaged in melee combat and cannot throw.\n", ch);
        return;
    }

    if (ch->balance <= -15)
    {
        send_to_char ("You're far too off-balance to attempt a throw.\n", ch);
        return;
    }

    if (tobj->obj_flags.weight / 100 > ch->str * 2)
    {
        send_to_char ("That object is too heavy to throw effectively.\n", ch);
        return;
    }

    if (tobj->count > 1)
    {
        send_to_char ("You may only throw one object at a time.\n", ch);
        return;
    }

    if (!tch && dir != -1)
    {
        if ((exit = EXIT (ch, dir)))
            troom = vnum_to_room (EXIT (ch, dir)->to_room);

        /* for throwing 'out' of a dwelling */
        //if ((dir == 6) && (ch->in_room > 100000))
        //	troom = vtor(ch->was_in_room);

        /*** harsh way to deal with throwing things out without crashing the game	**/
        if ((dir == 6) && (ch->in_room > 100000))
        {
            obj_from_char (&tobj, 0);
            extract_obj (tobj);
            sprintf (buf, "You dispose of  #2%s#0.", tobj->short_description);
            act (buf, false, ch, tobj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }
        /****/
        if (!troom)
        {
            send_to_char ("There is no exit in that direction.\n", ch);
            return;
        }

        if (exit
                && IS_SET (exit->exit_info, EX_ISDOOR)
                && IS_SET (exit->exit_info, EX_CLOSED)
                && !IS_SET (exit->exit_info, EX_ISGATE))
        {
            send_to_char ("Your view is blocked.\n", ch);
            return;
        }

        argument = one_argument (argument, buf);

        if (*buf)
        {
            tch = get_char_room_vis2 (ch, troom->vnum, buf);
            if (!has_been_sighted (ch, tch))
                tch = NULL;
            if (!tch)
            {
                send_to_char
                ("You do not see anyone or anything like that in this direction.\n", ch);
                return;
            }

			/*
            if (GET_ITEM_TYPE(obj) != ITEM_COVER && GET_ITEM_TYPE(obj) != ITEM_TABLE)
            {
              send_to_char ("You can only target a grenade at a piece of cover or furniture.\n", ch);
              return;
            }
			*/
        }

        if (!tch)
        {

            // If we're aiming a grenade at the room in general, we just toss it and go.
            if (aimed_room && GET_ITEM_TYPE(tobj) == ITEM_GRENADE && tobj->o.grenade.status != 2)
            {
                // Better your explosive skill is, quicker the grenade goes off - every 20 points is another
                // second lost.
                int grenade_counter = 6;
                grenade_counter = MAX(1, grenade_counter - ((ch->skills[SKILL_ARCHERY] + 10) / 20));
                tobj->o.grenade.status = 1;
                add_second_affect(SA_GRENADE, grenade_counter, NULL, tobj, 0, 0);
                sprintf (buf, "You arm $p before hurling it %sward.", dirs[dir]);
                act (buf, false, ch, tobj, 0, TO_CHAR | _ACT_FORMAT);
                sprintf (buf, "$n arms $p before hurling it %sward.", dirs[dir]);
                buf[2] = toupper (buf[2]);
                watched_action(ch, buf, 0, 1);
                act (buf, true, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
            }
            else
            {
                sprintf (buf, "You hurl #2%s#0 %sward.", tobj->short_description, dirs[dir]);
                act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                sprintf (buf, "#5%s#0 hurls #2%s#0 %sward.", char_short (ch), tobj->short_description, dirs[dir]);
                buf[2] = toupper (buf[2]);
                watched_action(ch, buf, 0, 1);
                act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            }

            obj_from_char (&tobj, 0);
            obj_to_room (tobj, troom->vnum);
            sprintf (buf, "#2%s#0 flies in, hurled from %s.", tobj->short_description, verbose_dirs[rev_dir[dir]]);
            buf[2] = toupper (buf[2]);
            send_to_room (buf, troom->vnum);

            return;
        }
    }//if (!tch && dir != -1)

    if (tch)
    {
        if (tch == ch)
        {
            send_to_char ("That wouldn't require much of a throw...\n", ch);
            return;
        }

        skill_learn(ch, SKILL_AIM);

        if (are_grouped (ch, tch) && *argument != '!')
        {
            sprintf (buf,
                     "#1You decide not to throw at $N #1who is a fellow group member!#0");
            act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (IS_SET (tch->act, ACT_VEHICLE))
        {
            sprintf (buf, "And what good will that do to hurt $N?");
            act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (ch->room != tch->room && dir != -1)
        {
            if ((exit = EXIT (ch, dir)))
                troom = vnum_to_room (EXIT (ch, dir)->to_room);

            if (!troom)
            {
                send_to_char ("There is no exit in that direction.\n", ch);
                return;
            }

            if (exit
                    && IS_SET (exit->exit_info, EX_CLOSED)
                    && !IS_SET (exit->exit_info, EX_ISGATE))
            {
                send_to_char ("Your view is blocked.\n", ch);
                return;
            }
        }
        notify_guardians (ch, tch, 5);

        if (dir != -1)
            range_mod = 5;


        result = calculate_missile_result (ch, SKILL_AIM, ((ch->balance * -10) + range_mod), tch, 0, 0, tobj, NULL, &location, &damage, &poison, rev_dir[dir]);


        if (get_affect (ch, MAGIC_HIDDEN))
        {
            remove_affect_type (ch, MAGIC_HIDDEN);
            send_to_char ("You emerge from concealment and prepare to throw.\n\n",
                          ch);
        }

        if ((result == CRITICAL_MISS || result == MISS) && tch->fighting
                && tch->fighting != ch && number (1, 25) > ch->dex)
        {
            send_to_char("You realize there are no safe targets.\n", ch);
            return;
        }

        damage = (int) damage;

        wear_loc1 = body_tab[0][location].wear_loc1;
        wear_loc2 = body_tab[0][location].wear_loc2;

        if (wear_loc1)
        {
            armor1 = get_equip (tch, wear_loc1);
            if (armor1 && GET_ITEM_TYPE (armor1) != ITEM_ARMOR)
                armor1 = NULL;
        }
        if (wear_loc2)
        {
            armor2 = get_equip (tch, wear_loc2);
            if (armor2 && GET_ITEM_TYPE (armor2) != ITEM_ARMOR)
                armor2 = NULL;
        }

        if (!ch->fighting && damage > 3)
            criminalize (ch, tch, tch->room->zone, CRIME_KILL);

        if (ch->room != tch->room)
        {
            sprintf (buf, "You hurl #2%s#0 %s, toward #5%s#0.",
                     tobj->short_description, dirs[dir],
                     char_short (tch));
            sprintf (buf2, "#5%s#0 hurls #2%s#0 %s, toward #5%s#0.",
                     char_short (ch), tobj->short_description,
                     dirs[dir], char_short (tch));
            buf2[2] = toupper (buf2[2]);
            watched_action(ch, buf2, 0, 1);
            act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        }
        else
        {
            sprintf (buf, "You hurl #2%s#0 forcefully at #5%s#0.",
                     tobj->short_description, char_short (tch));
            sprintf (buf2, "#5%s#0 hurls #2%s#0 forcefully at #5%s#0.",
                     char_short (ch), tobj->short_description,
                     char_short (tch));
            buf2[2] = toupper (buf2[2]);
            watched_action(ch, buf2, 0, 1);
            sprintf (buf3, "#5%s#0 hurls #2%s#0 forcefully at you.",
                     char_short (ch), tobj->short_description);
            buf3[2] = toupper (buf3[2]);
            act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
            act (buf2, false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
            act (buf3, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
        }

        if (GET_ITEM_TYPE (tobj) == ITEM_WEAPON
                && (tobj->o.weapon.hit_type == 0 || tobj->o.weapon.hit_type == 1
                    || tobj->o.weapon.hit_type == 2
                    || tobj->o.weapon.hit_type == 4))
        {
            if (result != CRITICAL_HIT && armor1
                    && armor1->o.armor.armor_type >= 2)
                can_lodge = false;
            else if (result != CRITICAL_HIT && armor2
                     && armor2->o.armor.armor_type >= 2)
                can_lodge = false;
            else if (result != CRITICAL_HIT && tch->armor && tch->armor >= 4)
                can_lodge = false;
            else
                can_lodge = false;
        }

        sprintf (strike_location, "%s", figure_location (tch, location));

        if ((af = get_affect (tch, MAGIC_AFFECT_PROJECTILE_IMMUNITY)))
        {
            sprintf (buf, "%s", spell_activity_echo (af->a.spell.sn, af->type));
            if (!*buf)
                sprintf (buf,
                         "\nThe projectile is deflected harmlessly aside by some invisible force.");
            sprintf (buf2, "\n%s", buf);
            result = MISS;
            damage = 0;
        }

        else if (result == MISS)
        {
            sprintf (buf, "%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt misses completely.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch));
            *buf = toupper (*buf);
            sprintf (buffer, "#2%s", buf);
            sprintf (buf, "%s", buffer);

            sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt misses completely.", tobj->short_description, dirs[rev_dir[dir]]);
            *buf2 = toupper (*buf2);
            sprintf (buffer, "#2%s", buf2);
            sprintf (buf2, "%s", buffer);

            sprintf (buf4, "It misses completely.");
        }
        else if (result == CRITICAL_MISS)
        {
            sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt flies far wide of any target.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch));
            *buf = toupper (*buf);
            sprintf (buffer, "#2%s", buf);
            sprintf (buf, "%s", buffer);

            sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt flies far wide of any target.", tobj->short_description, dirs[rev_dir[dir]]);
            *buf2 = toupper (*buf2);
            sprintf (buffer, "#2%s", buf2);
            sprintf (buf2, "%s", buffer);

            sprintf (buf4, "It flies far wide of any target.");
        }
        else if (result == COVER_HIT)
        {
            af = get_affect(tch, AFFECT_COVER);
            if (af && af->a.cover.obj && obj_short_desc(af->a.cover.obj))
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt glances harmlessly off #2%s#0.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch), obj_short_desc(af->a.cover.obj));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt glances harmlessly off #2%s#0.", tobj->short_description, dirs[rev_dir[dir]], obj_short_desc(af->a.cover.obj));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It glances harmlessly off #2%s#0.", obj_short_desc(af->a.cover.obj));
            }
            else
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt glances harmlessly off the scenery.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt glances harmlessly off the scenery.", tobj->short_description, dirs[rev_dir[dir]]);
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It glances harmlessly off the scenery.");
            }
        }
        else if (result == SHIELD_BLOCK)
        {
            if (obj_short_desc (get_equip (tch, WEAR_SHIELD)))
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt glances harmlessly off #2%s#0.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch), obj_short_desc (get_equip (tch, WEAR_SHIELD)));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt glances harmlessly off #2%s#0.", tobj->short_description, dirs[rev_dir[dir]], obj_short_desc (get_equip (tch, WEAR_SHIELD)));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It glances harmlessly off #2%s#0.", obj_short_desc (get_equip (tch, WEAR_SHIELD)));
            }
            else
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt glances harmlessly off something.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt glances harmlessly off something.", tobj->short_description, dirs[rev_dir[dir]]);
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It glances harmlessly off something.");
            }
        }
        else if (result == GLANCING_HIT)
        {
            sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt grazes %s on the %s.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch), HMHR (tch), expand_wound_loc (strike_location));
            *buf = toupper (*buf);
            sprintf (buffer, "#2%s", buf);
            sprintf (buf, "%s", buffer);

            sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt grazes you on the %s.", tobj->short_description, dirs[rev_dir[dir]], expand_wound_loc (strike_location));
            *buf2 = toupper (*buf2);
            sprintf (buffer, "#2%s", buf2);
            sprintf (buf2, "%s", buffer);

            sprintf (buf4, "It grazes %s on the %s.",HMHR (tch), expand_wound_loc (strike_location));

        }
        else if (result == HIT)
        {
            if (can_lodge)
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt lodges in %s %s.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch), HSHR (tch), expand_wound_loc (strike_location));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt lodges in your %s.", tobj->short_description, dirs[rev_dir[dir]], expand_wound_loc (strike_location));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It lodges in %s %s.", HSHR (tch), expand_wound_loc (strike_location));
            }
            else
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt strikes %s on the %s.", tobj->short_description, dirs[rev_dir[dir]], char_short(tch),  HMHR (tch), expand_wound_loc (strike_location));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt strikes you on the %s.", tobj->short_description, dirs[rev_dir[dir]], expand_wound_loc (strike_location));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It strikes %s on the %s.", HMHR (tch), expand_wound_loc (strike_location));
            }
        }
        else if (result == CRITICAL_HIT)
        {
            if (can_lodge)
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt lodges deeply in %s %s!", tobj->short_description, dirs[rev_dir[dir]], char_short(tch),  HSHR (tch), expand_wound_loc (strike_location));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt lodges deeply in your %s!", tobj->short_description, dirs[rev_dir[dir]], expand_wound_loc (strike_location));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It lodges deeply in %s %s.", HSHR (tch), expand_wound_loc (strike_location));
            }
            else
            {
                sprintf (buf, "#2%s#0 comes flying through the air from the %s, headed straight toward #5%s#0\n\nIt strikes %s solidly on the %s.", HMHR (tch), tobj->short_description, dirs[rev_dir[dir]], char_short(tch), expand_wound_loc (strike_location));
                *buf = toupper (*buf);
                sprintf (buffer, "#2%s", buf);
                sprintf (buf, "%s", buffer);

                sprintf (buf2, "#2%s#0 comes flying through the air from the %s, headed straight toward you!\n\nIt strikes you solidly on the %s.", tobj->short_description, dirs[rev_dir[dir]], expand_wound_loc (strike_location));
                *buf2 = toupper (*buf2);
                sprintf (buffer, "#2%s", buf2);
                sprintf (buf2, "%s", buffer);

                sprintf (buf4, "It strikes %s solidly on the %s.", HMHR (tch), expand_wound_loc (strike_location));
            }
        }

        char *out1, *out2, *out3;
        CHAR_DATA* rch = 0;
        reformat_string (buf, &out1);
        reformat_string (buf2, &out2);
        reformat_string (buf4, &out3);

        if (ch->room != tch->room)
        {
            // aggressor's room
            for (rch = ch->room->people; rch; rch = rch->next_in_room)
            {
                send_to_char ("\n", rch);
                send_to_char (out3, rch);
            }
        }

        // victim's room
        for (rch = tch->room->people; rch; rch = rch->next_in_room)
        {
            send_to_char ("\n", rch);
            if (ch->room == tch->room)
                send_to_char (out3, rch);
            else if (rch == tch)
                send_to_char (out2, rch);
            else
                send_to_char (out1, rch);
        }

        mem_free (out1);
        mem_free (out2);

        obj_from_char (&tobj, 0);

        if ((result == HIT || result == CRITICAL_HIT) && can_lodge)
        {
            lodge_missile (tch, tobj, strike_location, 0);
        }
        else
        {
			// If you throw at someone in the same room who's fighting,
			// then you've lost that weapon for 15 seconds.
			obj_to_room (tobj, tch->in_room);
			if (tch->fighting && ch->in_room == tch->in_room)
			{
				add_second_affect (SA_GET_OBJ, 15, ch, tobj, NULL, 0);
				tobj->tmp_flags |= SA_DROPPED;
			}

            if (GET_ITEM_TYPE(tobj) == ITEM_GRENADE && tobj->o.grenade.status == 1)
            {
                send_to_gods("Grenade land-in-room test.");
                add_second_affect(SA_GRENADE, 5, NULL, tobj, 0, 0);
            }
        }

        if (damage > 0)
        {
            if (!IS_NPC (tch))
            {
                tch->delay_ch = ch;
                tch->delay_info1 = tobj->nVirtual;
            }
            if (ch->room != tch->room)
                ranged = true;
            if (GET_ITEM_TYPE (tobj) == ITEM_WEAPON)
                wound_type = tobj->o.weapon.hit_type;
            else
                wound_type = 3;
            if (wound_to_char
                    (tch, strike_location, (int) damage, wound_type, 0, poison, 0))
            {
                if (ranged)
                    send_to_char ("\nYour target collapses, dead.\n", ch);
                ch->ranged_enemy = NULL;
                return;
            }
            if (!IS_NPC (tch))
            {
                tch->delay_ch = NULL;
                tch->delay_info1 = 0;
            }
        }

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

        npc_ranged_response (tch, ch);	// do_throw

        return;
    }

    send_to_char
    ("There has been an error; please report your command syntax to the staff.\n",
     ch);
}

void
do_whirl (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *sling;

    if (IS_SWIMMING (ch))
    {
        send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

    if (IS_FLOATING(ch))
    {
        send_to_char( "You can't do that while floating!\n", ch);
        return;
    }

    if (ch->fighting)
    {
        send_to_char ("You're fighting for your life!\n", ch);
        return;
    }

    if (!get_equip (ch, WEAR_PRIM))
    {
        send_to_char
        ("You need to be wielding a loaded sling to use this command.\n", ch);
        return;
    }

    sling = get_equip (ch, WEAR_PRIM);

    if (sling->o.weapon.use_skill != SKILL_AIM)
    {
        send_to_char ("This command is for use with slings only.\n", ch);
        return;
    }

    if (!sling->contains)
    {
        send_to_char ("The sling needs to be loaded, first.\n", ch);
        return;
    }

    act ("You begin whirling $p, gathering momentum for a shot.", false, ch,
         sling, 0, TO_CHAR | _ACT_FORMAT);
    act ("$n begins whirling $p, gathering momentum for a shot.", true, ch,
         sling, 0, TO_ROOM | _ACT_FORMAT);

    ch->whirling = 1;
}

/** There is no block from a critical hit.
** If you are under cover, all other shots are misses.
** If you are not under cover, then you have a chance to block missiles with your shield.
**/
int
projectile_shield_block (CHAR_DATA * ch, int result)
{
    OBJ_DATA *shield_obj;
    int roll, dir = 0;

    if (result == CRITICAL_HIT)
        return 0;

    dir = ch->cover_from_dir;

    if ((dir == 0 && get_affect (ch, AFFECT_COVER)))
        return 1;

    if ((shield_obj = get_equip (ch, WEAR_SHIELD)))
    {
        skill_use (ch, SKILL_DEFLECT, 0);
        roll = number (1, SKILL_CEILING);
        if (roll <= ch->skills[SKILL_DEFLECT])
        {
            return 1;
        }
    }

    return 0;
}

int
calculate_missile_result (CHAR_DATA * ch, int ch_skill, int att_modifier,
                          CHAR_DATA * target, int def_modifier,
                          OBJ_DATA * weapon, OBJ_DATA * missile,
                          AFFECTED_TYPE * spell, int *location, float *damage, int *poison, int direction)
{
    int roll = 0, defense = 0, assault = 0, result = 0;
    int hit_type = 0;
	int cover = 0;
	AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *taf = NULL;
    //POISON_DATA *xpoison;

    /* Determine result of hit attempt. */

    if (!CAN_SEE (target, ch))
        def_modifier += number (15, 30);

    roll = number (1, SKILL_CEILING);
    roll += def_modifier;
    roll = MIN (roll, SKILL_CEILING);

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
        cover = 0;
    }
    else
        cover = 85;


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
            if (af->a.cover.direction == direction || direction == -1 || direction == 0)
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
	else if (roll > target->skills[SKILL_DODGE])
    {
		if (GET_ITEM_TYPE (missile) == ITEM_WEAPON)
		{
			skill_use (target, SKILL_DODGE, 0);
		}

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

	if (GET_ITEM_TYPE (missile) == ITEM_WEAPON)
	{
		int skill_cap = ch->skills[ch_skill];

		skill_use(ch, ch_skill, 0);

		if (skill_cap == 50 && ch->skills[ch_skill] > 50)
		{
			ch->skills[ch_skill] = skill_cap;
		}
	}

    // Random modifier
    roll = number (1, SKILL_CEILING);
    roll += att_modifier;
    roll = MIN (roll, SKILL_CEILING);

    ch->aim = 0;

    if (ch->skills[ch_skill])
    {
        if (roll > ch->skills[ch_skill])
        {
            if (roll % 5 == 0 || roll == 1)
                assault = RESULT_CF;
            else
                assault = RESULT_MF;
        }
        else if (roll <= ch->skills[ch_skill])
        {
            if (roll % 5 == 0)
                assault = RESULT_CS;
            else
                assault = RESULT_MS;
        }
    }
    else
    {
        if (roll > ch->skills[SKILL_OFFENSE])
        {
            if (roll % 5 == 0 || roll == 1)
                assault = RESULT_CF;
            else
                assault = RESULT_MF;
        }
        else if (roll <= ch->skills[SKILL_OFFENSE])
        {
            if (roll % 5 == 0)
                assault = RESULT_CS;
            else
                assault = RESULT_MS;
        }
    }

    if (assault == RESULT_CS)
    {
        if (defense == RESULT_CS)
        {
            if (cover && number(0,9))
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
		{
			if (cover)
				result = COVER_HIT;
			else
				result = MISS;
		}
        else if (defense == RESULT_MS)
        {
            if (cover && number(0,9))
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

    //std::string gods = "Direction: " + MAKE_STRING(direction) + "Cover: " + MAKE_STRING(cover) + "Attack Roll: " + MAKE_STRING(roll) + " Result: " + MAKE_STRING(defense);
    //send_to_gods(gods.c_str());

    if (!AWAKE (target) && assault != RESULT_CF)
        result = CRITICAL_HIT;

    if ((result == HIT || result == GLANCING_HIT)
            && projectile_shield_block (target, result))
        result = SHIELD_BLOCK;

    if (result == MISS || result == CRITICAL_MISS || result == SHIELD_BLOCK || result == COVER_HIT)
        return result;

    /* Determine damage of hit, if applicable. */

    *damage = 0;

    roll = number (1, 100);
    *location = -1;

    while (roll > 0)
        roll = roll - body_tab[1][++(*location)].percent;

    if (missile)
    {
        if (GET_ITEM_TYPE (missile) == ITEM_WEAPON)
        {
            hit_type = missile->o.weapon.hit_type;
            *damage = dice (missile->o.od.value[1], missile->o.od.value[2]);
        }
        else
        {
            hit_type = 3;
            *damage = 1;
        }

        // As all archery, hits can be shrugged off, but critical hits are often lethal.

        if (weapon || IS_SET (missile->obj_flags.extra_flags, ITEM_THROWING))
        {
            if (result == HIT)
            {
                *damage += number (1, 2);
            }
            else if (result == CRITICAL_HIT)
            {
                *damage += number (3, 5);
            }
        }

		if (result == GLANCING_HIT)
		{
			*damage -= number (1, 2);
		}

		*damage = real_damage(target, *damage, location, hit_type, 0);
		*damage *= (body_tab[0][*location].damage_mult * 1.0) / (body_tab[0][*location].damage_div * 1.0);
    }

    if (!GET_ITEM_TYPE (missile) == ITEM_WEAPON)
    {
		if (OBJ_MASS(missile) >= 1000)
		{
			*damage = MAX(0, OBJ_MASS(missile) / 1000);
		}
		else
		{
			*damage = 0;
		}
    }

    damage_objects(target, *damage, "", *location, hit_type, 1, false);

    return result;
}

void
lodge_missile (CHAR_DATA * target, OBJ_DATA * ammo, char *strike_location, int internal)
{
    LODGED_OBJECT_INFO *lodged = NULL;

    if (!target->lodged)
    {
        CREATE (target->lodged, LODGED_OBJECT_INFO, 1);
        target->lodged->vnum = ammo->nVirtual;
        target->lodged->location = add_hash (strike_location);
        target->lodged->colored = 0;

        if (*ammo->var_color[0])
        {
            target->lodged->colored = 1;
            target->lodged->var_color = ammo->var_color[0];
        }

        if (*ammo->var_color[1])
            target->lodged->var_color2 = ammo->var_color[1];
        else
            target->lodged->var_color2 = "(none)";

        if (*ammo->var_color[2])
            target->lodged->var_color3 = ammo->var_color[2];
        else
            target->lodged->var_color3 = "(none)";

        target->lodged->short_description = ammo->short_description;
        target->lodged->bleeding = NULL;

        if (internal)
        {
			/*
            if (!(af = get_affect(target, AFFECT_INTERNAL)))
            {
                af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
                af->type = AFFECT_INTERNAL;
                af->a.bleeding.duration = -1;
                af->a.bleeding.rate = internal;
                af->next = NULL;
                affect_to_char(target, af);
            }
            else
            {
                af->a.bleeding.rate += internal;
            }
			*/
        }
    }
    else
        for (lodged = target->lodged; lodged; lodged = lodged->next)
        {
            if (!lodged->next)
            {
                CREATE (lodged->next, LODGED_OBJECT_INFO, 1);
                lodged->next->vnum = ammo->nVirtual;
                lodged->next->location = add_hash (strike_location);
                lodged->next->colored = 0;

                if (*ammo->var_color[0])
                {
                    lodged->next->colored = 1;
                    lodged->next->var_color = ammo->var_color[0];
                }

                if (*ammo->var_color[1])
                    lodged->next->var_color2 = ammo->var_color[1];
                else
                    lodged->next->var_color2 = "(none)";

                if (*ammo->var_color[2])
                    lodged->next->var_color3 = ammo->var_color[2];
                else
                    lodged->next->var_color3 = "(none)";

                lodged->next->short_description = ammo->short_description;
                target->lodged->bleeding = NULL;

                if (internal)
                {
					/*
                    if (!(af = get_affect(target, AFFECT_INTERNAL)))
                    {
                        af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
                        af->type = AFFECT_INTERNAL;
                        af->a.bleeding.duration = -1;
                        af->a.bleeding.rate = internal;
                        af->next = NULL;
                        affect_to_char(target, af);
                    }
                    else
                    {
                        af->a.bleeding.rate += internal;
                    }
					*/
                }


                break;
            }
            else
                continue;
        }
}

void
fire_sling (CHAR_DATA * ch, OBJ_DATA * sling, char *argument)
{
    CHAR_DATA *tch = NULL;
    ROOM_DATA *troom = NULL;
    OBJ_DATA *ammo;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH], strike_location[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
    float damage = 0;
    int dir = 0, location = 0, result = 0, attack_mod = 0, wound_type = 0;
    bool ranged = false;
    int poison = 0;

    const char *fancy_dirs[] =
    {
        "northward",
        "eastward",
        "southward",
        "westward",
        "upward",
        "downward",
        "\n"
    };

    if (!ch || !sling)
        return;

    if (!*argument)
    {
        send_to_char ("At what did you wish to fire your sling?\n", ch);
        return;
    }

    if (!ch->whirling)
    {
        send_to_char
        ("You'll need to begin whirling your sling before you can fire.\n",
         ch);
        return;
    }

    if (!sling->contains)
    {
        send_to_char ("You'll need to load your sling first.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if ((dir = is_direction (buf)) == -1)
    {
        if (!(tch = get_char_room_vis (ch, buf)))
        {
            send_to_char ("At what did you wish to fire?\n", ch);
            return;
        }
    }

    if (!tch && dir != -1)
    {
        if (EXIT (ch, dir))
            troom = vnum_to_room (EXIT (ch, dir)->to_room);

        if (!troom)
        {
            send_to_char ("There is no exit in that direction.\n", ch);
            return;
        }

        argument = one_argument (argument, buf);

        if (*buf)
        {
            tch = get_char_room_vis2 (ch, troom->vnum, buf);
            if (!has_been_sighted (ch, tch))
                tch = NULL;
            if (!tch)
            {
                send_to_char
                ("You do not see anyone like that in this direction.\n", ch);
                return;
            }
        }
    }

    if (!tch)
    {
        send_to_char ("At whom did you wish to fire your sling?\n", ch);
        return;
    }

    if (tch == ch)
    {
        send_to_char ("Don't be silly.\n", ch);
        return;
    }

    if (IS_SET (tch->act, ACT_VEHICLE))
    {
        sprintf (buf, "And what good will that do to hurt $N?");
        act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
        return;
    }


    attack_mod -= ch->whirling;
    ch->whirling = 0;
    notify_guardians (ch, tch, 4);

    result =
        calculate_missile_result (ch, sling->o.weapon.use_skill, attack_mod, tch,
                                  0, sling, sling->contains, NULL, &location,
                                  &damage, &poison, 0);
    damage = (int) damage;

    sprintf (strike_location, "%s", figure_location (tch, location));

    if (ch->room != tch->room)
    {
        sprintf (buf, "You sling #2%s#0 %s, toward #5%s#0.",
                 sling->contains->short_description, fancy_dirs[dir],
                 char_short (tch));
        sprintf (buf2, "#5%s#0 slings #2%s#0 %s, toward #5%s#0.",
                 char_short (ch), sling->contains->short_description,
                 fancy_dirs[dir], char_short (tch));
        buf2[2] = toupper (buf2[2]);
        watched_action(ch, buf2, 0, 1);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
    }
    else
    {
        sprintf (buf, "You sling #2%s#0 forcefully at #5%s#0.",
                 sling->contains->short_description, char_short (tch));
        sprintf (buf2, "#5%s#0 slings #2%s#0 forcefully at #5%s#0.",
                 char_short (ch), sling->contains->short_description,
                 char_short (tch));
        sprintf (buf3, "#5%s#0 slings #2%s#0 forcefully at you.",
                 char_short (ch), sling->contains->short_description);
        watched_action(ch, buf2, 0, 1);
        act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
        act (buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
        act (buf2, false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
    }

    if ((af = get_affect (tch, MAGIC_AFFECT_PROJECTILE_IMMUNITY)))
    {
        sprintf (buf, "%s", spell_activity_echo (af->a.spell.sn, af->type));
        if (!*buf)
            sprintf (buf,
                     "\nThe is deflected harmlessly aside by some invisible force.");
        sprintf (buf2, "\n%s", buf);
        result = MISS;
        damage = 0;
    }
    else if (result == MISS)
    {
        sprintf (buf, "It misses completely.");
        sprintf (buf2, "It misses completely.");
    }
    else if (result == CRITICAL_MISS)
    {
        sprintf (buf, "It flies far wide of any target.");
        sprintf (buf2, "It flies far wide of any target.");
    }
    else if (result == SHIELD_BLOCK)
    {
        if (obj_short_desc (get_equip (tch, WEAR_SHIELD)))
        {
            sprintf (buf, "It glances harmlessly off #2%s#0.",
                     obj_short_desc (get_equip (tch, WEAR_SHIELD)));
            sprintf (buf2, "It glances harmlessly off #2%s#0.",
                     obj_short_desc (get_equip (tch, WEAR_SHIELD)));
        }
        else
        {
            sprintf (buf, "It glances harmlessly off something.");
            sprintf (buf2, "It glances harmlessly off something.");
        }
    }
    else if (result == GLANCING_HIT)
    {
        sprintf (buf, "It grazes %s on the %s.", HMHR (tch),
                 expand_wound_loc (strike_location));
        sprintf (buf2, "It grazes you on the %s.",
                 expand_wound_loc (strike_location));
    }
    else if (result == HIT)
    {
        sprintf (buf, "It strikes %s on the %s.", HMHR (tch),
                 expand_wound_loc (strike_location));
        sprintf (buf2, "It strikes you on the %s.",
                 expand_wound_loc (strike_location));
    }
    else if (result == CRITICAL_HIT)
    {
        sprintf (buf, "It strikes %s solidly on the %s.", HMHR (tch),
                 expand_wound_loc (strike_location));
        sprintf (buf2, "It strikes you solidly on the %s.",
                 expand_wound_loc (strike_location));
    }

    ammo = sling->contains;

    obj_from_obj (&ammo, 0);
    obj_to_room (ammo, tch->in_room);

    if (result == CRITICAL_MISS && !number (0, 1))
    {
        sprintf (buf + strlen (buf),
                 "\n\nThe missile recedes hopelessly from sight.");
        sprintf (buf2 + strlen (buf2),
                 "\n\nThe missile recedes hopelessly from sight.");
        sprintf (buf3 + strlen (buf3),
                 "\n\nThe missile recedes hopelessly from sight.");
        extract_obj (ammo);
    }

    char *out1, *out2;
    CHAR_DATA* rch = 0;
    reformat_string (buf, &out1);
    reformat_string (buf2, &out2);

    if (ch->room != tch->room)
    {
        // aggressor's room
        for (rch = ch->room->people; rch; rch = rch->next_in_room)
        {
            send_to_char ("\n", rch);
            send_to_char (out1, rch);
        }
    }

    // victim's room
    for (rch = tch->room->people; rch; rch = rch->next_in_room)
    {
        send_to_char ("\n", rch);
        if (rch == tch)
            send_to_char (out2, rch);
        else
            send_to_char (out1, rch);
    }

    mem_free (out1);
    mem_free (out2);

    if (damage > 0)
    {
        if (!IS_NPC (tch))
        {
            tch->delay_ch = ch;
            tch->delay_info1 = ammo->nVirtual;
        }
        if (ch->room != tch->room)
            ranged = true;
        wound_type = 3;
        if (wound_to_char
                (tch, strike_location, (int) damage, wound_type, 0, poison, 0))
        {
            if (ranged)
                send_to_char ("\nYour target collapses, dead.\n", ch);
            ch->ranged_enemy = NULL;
            return;
        }
        if (!IS_NPC (tch))
        {
            tch->delay_ch = NULL;
            tch->delay_info1 = 0;
        }
        if (!ch->fighting && damage > 3)
            criminalize (ch, tch, tch->room->zone, CRIME_KILL);
    }

    npc_ranged_response (tch, ch);

}

int
has_been_sighted (CHAR_DATA * ch, CHAR_DATA * target)
{
    SIGHTED_DATA *sighted;

    if (!ch || !target)
        return 0;

    if (!IS_MORTAL (ch))
        return 1;

    if (IS_NPC (ch) && !ch->descr())
        return 1;			/* We know non-animated NPCs only acquire targets via SCANning; */
    /* don't need anti-twink code for them. */

    for (sighted = ch->sighted; sighted; sighted = sighted->next)
    {
        if (sighted->target == target)
            return 1;
    }

    return 0;
}

void
do_unload_missile (CHAR_DATA * ch, char *argument, int cmd)
{
    //OBJ_DATA *bow = NULL, *arrow = NULL, *quiver = NULL;
    //int i;
    //char *error;
    //char buf[MAX_STRING_LENGTH];
    //char buffer[MAX_STRING_LENGTH];

    //bow = get_bow (ch);

    //if (!bow)
    //{
    //    send_to_char ("What did you wish to unload?\n", ch);
    //    return;
    //}

    //if (!bow->attached)
    //{
    //    send_to_char ("That isn't loaded.\n", ch);
    //    return;
    //}

    //if (!(arrow = bow->attached))
    //{
    //    send_to_char ("That isn't loaded.\n", ch);
    //    return;
    //}

    //sprintf (buf, "%s#0 unloads #2%s#0.", char_short (ch),
    //         bow->short_description);
    //*buf = toupper (*buf);
    //sprintf (buffer, "#5%s", buf);
    //act (buffer, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    //arrow = load_object (bow->attached->nVirtual);
    //arrow->in_obj = NULL;
    //arrow->carried_by = NULL;

    //for (i = 0; i < MAX_WEAR; i++)
    //{
    //    if (!(quiver = get_equip (ch, i)))
    //        continue;
    //    if (GET_ITEM_TYPE (quiver) == ITEM_QUIVER
    //            && get_obj_in_list_num (arrow->nVirtual, quiver->contains)
    //            && can_obj_to_container (arrow, quiver, &error, 1))
    //        break;
    //}

    //if (!quiver)
    //{
    //    for (i = 0; i < MAX_WEAR; i++)
    //    {
    //        if (!(quiver = get_equip (ch, i)))
    //            continue;
    //        if (GET_ITEM_TYPE (quiver) == ITEM_QUIVER
    //                && can_obj_to_container (arrow, quiver, &error, 1))
    //            break;
    //        else
    //            quiver = NULL;
    //    }
    //}

    //if (bow->o.weapon.use_skill == SKILL_AIM)
    //{
    //    bow->location = WEAR_PRIM;
    //}

    //if (quiver)
    //{
    //    send_to_char ("\n", ch);
    //    sprintf (buf, "You unload #2%s#0, and slide #2%s#0 into #2%s#0.",
    //             obj_short_desc (bow), obj_short_desc (arrow),
    //             obj_short_desc (quiver));
    //    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //    obj_to_obj (arrow, quiver);
    //}
    //else if (!quiver)
    //{
    //    sprintf (buf, "\nYou unload #2%s#0.\n", obj_short_desc (bow));
    //    send_to_char (buf, ch);
    //    obj_to_char (arrow, ch);
    //}
    //bow->attached = NULL;
}

void
delayed_load (CHAR_DATA * ch)
{
    OBJ_DATA *bow;
    OBJ_DATA *ammo;
    OBJ_DATA *quiver;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (!
            ((bow = get_equip (ch, WEAR_PRIM))
             || (bow = get_equip (ch, WEAR_BOTH))))
    {
        ch->delay_who = NULL;
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char ("Having removed your weapon, you cease loading it.\n",
                      ch);
        return;
    }

    if (bow->o.weapon.use_skill != SKILL_AIM)
    {
        ch->delay_who = NULL;
        ch->delay = 0;
        ch->delay_type = 0;
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        send_to_char
        ("Having switched your weapon, you cease loading your bow.\n", ch);
        return;
    }

    ammo = NULL;

    if (ch->delay_info1 < 0)
    {

        if (ch->right_hand && ch->right_hand->nVirtual == ch->delay_info2)
        {
            ammo = ch->right_hand;
        }
        else if (ch->left_hand && ch->left_hand->nVirtual == ch->delay_info2)
        {
            ammo = ch->left_hand;
        }
    }
    else
    {
        if (ch->left_hand && ch->right_hand)
        {
            send_to_char
            ("Having taken another object in hand, you cease loading your weapon.\n",
             ch);
            return;
        }

        for (i = 0; i < MAX_WEAR; i++)
        {

            if (!(quiver = get_equip (ch, i)))
            {

                continue;
            }

            if (quiver->nVirtual != ch->delay_info1
                    || !get_obj_in_list_num (ch->delay_info2, quiver->contains))
            {

                continue;
            }

            if (GET_ITEM_TYPE (quiver) == ITEM_QUIVER)
            {

                for (ammo = quiver->contains; ammo; ammo = ammo->next_content)
                {

                    if (GET_ITEM_TYPE (ammo) == ITEM_MISSILE
                            && ammo->nVirtual == ch->delay_info2)
                    {

                        break;
                    }
                }
                if (ammo)
                {

                    break;
                }
            }
        }

    }
    if (!ammo)
    {
        send_to_char
        ("Having lost your ammunition, you cease loading your weapon.\n", ch);
        return;
    }

    sprintf (buf, "You finish loading #2%s#0.\n", bow->short_description);
    send_to_char (buf, ch);

    sprintf (buf, "$n finishes loading #2%s#0.", bow->short_description);
    act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    bow->attached = load_object (ammo->nVirtual);

    bow->location = WEAR_BOTH;

    if (ch->delay_info1 >= 0)
    {
        obj_from_obj (&ammo, 1);
    }
    else
    {
        obj_from_char (&ammo, 0);
        extract_obj (ammo);
    }
    ch->delay_who = NULL;
    ch->delay = 0;
    ch->delay_info1 = 0;
    ch->delay_info2 = 0;
    ch->delay_type = 0;

}


void
do_hit (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *victim, *tch;
    OBJ_DATA *obj;
    int i, agi_diff = 0;
    char buf[MAX_STRING_LENGTH];
    char original[MAX_STRING_LENGTH];
    SECOND_AFFECT *sa = NULL;
    bool found = false;

    sprintf (original, "%s", argument);

    /* cmd = 0 if hit,
       cmd = 1 if kill */

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

    if (is_room_affected (ch->room->affects, MAGIC_ROOM_CALM))
    {
        act ("Try as you might, you simply cannot muster the will to break "
             "the peace that pervades the area.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (IS_SET(ch->room->room_flags, PEACE))
    {
        act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    /*
    if(get_soma_affect(ch, SOMA_PLANT_VISIONS) && number(0,30) > GET_WIL(ch))
    {
      act ("You're paralyzed with fear, and unable to undertake such an action!", false, ch, 0, 0, TO_CHAR);
      return;
    }
    */

    if (IS_SET (ch->flags, FLAG_PACIFIST))
    {
        send_to_char ("Remove your pacifist flag, first...\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if ((obj = get_equip (ch, WEAR_BOTH)))
    {
        if (obj->o.weapon.use_skill == SKILL_AIM ||
                obj->o.weapon.use_skill == SKILL_AIM)
        {
            send_to_char ("You can't use that in melee combat!\n", ch);
            return;
        }
    }

    if (get_affect (ch, MAGIC_AFFECT_PARALYSIS))
    {
        send_to_char ("You are paralyzed and unable to fight!\n\r", ch);
        return;
    }

    if (get_affect (ch, MAGIC_AFFECT_FEAR))
    {
        send_to_char ("You are too afraid to fight!\n\r", ch);
        return;
    }

    if (get_affect (ch, AFFECT_GROUP_RETREAT))
    {
        send_to_char ("You stop trying to retreat.\n", ch);
        remove_affect_type (ch, AFFECT_GROUP_RETREAT);
    }

    if (IS_SUBDUER (ch))
    {
        act ("You can't attack while you have $N subdued.",
             false, ch, 0, ch->subdue, TO_CHAR);
        return;
    }

    if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
            && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!')
    {
        sprintf (buf,
                 "You are in a lawful area; you would likely be flagged wanted for assault. "
                 "To confirm, type \'#6hit %s !#0\', without the quotes.",
                 original);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (IS_SET (ch->room->room_flags, BRAWL) && IS_SET (ch->room->room_flags, LAWFUL)
            && *argument != '!' && has_weapon(ch))
    {
        sprintf (buf, "You are in a lawful, but brawable area area; if you hit this person with that weapon you're holding, you'll be wanted for assault. You can either put away the weapon, or type \'#6hit %s !#0\', without the quotes, to continue.", original);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (!*buf)
    {

        if (IS_SET (ch->flags, FLAG_FLEE))
        {
            send_to_char ("You stop trying to flee.\n\r", ch);
            ch->flags &= ~FLAG_FLEE;
            return;
        }
        send_to_char ("Hit whom?\n\r", ch);
        return;
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You affectionately pat yourself.\n\r", ch);
        act ("$n affectionately pats $mself.", false, ch, 0, victim, TO_ROOM);
        return;
    }

    if ((sa = get_second_affect(ch, SA_WARDED, NULL)))
    {
        if ((CHAR_DATA *) sa->obj == victim)
        {
            send_to_char ("You're still warded away by their weapon.\n\r", ch);
            return;
        }
    }

    // Make sure people don't hit the wrong person.
    if (are_grouped (ch, victim) && is_brother (ch, victim) && *argument != '!')
    {
        sprintf (buf,
                 "#1You are about to attack $N #1who is a fellow group member!#0 To confirm, type \'#6hit %s !#0\', without the quotes.",
                 original);
        act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
        return;
    }

    //  sprintf (buf, "victim->fighting->short_descr = #2%s#0\n", victim->fighting->short_descr);
    //  send_to_gods(buf);
    // send_to_gods(ch->name);

    if (IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING)
            && AWAKE (victim) && !(victim->fighting)) // and victim is not fighting you
    {
        send_to_char ("They are flying out of reach!\n", ch);
        return;
    }

    if (IS_SET (victim->act, ACT_VEHICLE) || IS_SET (victim->act, ACT_PASSIVE))
    {
        send_to_char ("How do you propose to kill an inanimate object, hmm?\n",
                      ch);
        return;
    }

    i = 0;
    for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch->fighting == victim)
        {
            if (++i >= 4)
            {
                act ("You can't find an opening to attack $N.",
                     false, ch, 0, victim, TO_CHAR);
                return;
            }

        }
    }

    if (IS_SET (victim->act, ACT_PREY) && AWAKE (victim))
    {
        if (!get_affect (ch, MAGIC_HIDDEN) || !skill_use (ch, SKILL_SNEAK, -30))
        {
            act
            ("As you approach, $N spots you and darts away! Try using a ranged weapon or an ambush from hiding instead.",
             false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            act ("As $n approaches, $N spots $m and darts away.", false, ch, 0,
                 victim, TO_ROOM | _ACT_FORMAT);
            npc_evasion (victim, -1);
            add_threat (victim, ch, 7);
            remove_affect_type (ch, MAGIC_HIDDEN);
			if (get_second_affect(ch, SA_AIMSTRIKE, NULL))
			{
				remove_second_affect(get_second_affect (ch, SA_AIMSTRIKE, NULL));
			}
            return;
        }
    }

    ch->flags &= ~FLAG_FLEE;
    ch->act &= ~PLR_STOP;

    if (GET_POS (ch) == POSITION_STANDING &&
            !ch->fighting && victim != ch->fighting)
    {
        if (ch->delay && ch->delay_type != DEL_CAST)
            break_delay (ch);

        /*
              Removed to prevent easy cheesing of combat.

              ch->primary_delay = 0;
              ch->secondary_delay = 0;
        */

        if (victim->formation > 1 && do_group_size(victim) > 9)
        {
            for (tch = ch->room->people; tch; tch = tch->next_in_room)
            {
                if (tch->following != victim->following && tch->following != victim)
                    continue;

                if (num_attackers(tch) <= 2
                        && tch->formation == 1
                        && GET_POS (tch) >= POSITION_SITTING)
                {
                    found = true;
                    break;
                }
            }

            if (found && CAN_SEE(tch, ch))
            {
                sprintf(buf, "Your attempted attack at #5%s#0 is obstructed, instead you find yourself engaged by  #5%s#0.", char_short(victim), char_short(tch));
                act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                sprintf(buf, "You intercept $n's attack at #5%s#0.", char_short(victim));
                act(buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
                victim = tch;
            }
        }

        if (cmd == 0)
            ch->flags &= ~FLAG_KILL;
        else
            ch->flags |= FLAG_KILL;

        if (GET_POS (ch) != POSITION_DEAD && GET_POS (victim) != POSITION_DEAD)
        {
            criminalize (ch, victim, vnum_to_room (victim->in_room)->zone, CRIME_KILL);
        }

        set_fighting (ch, victim);
        if (!victim->fighting)
        {
            set_fighting (victim, ch);
            notify_guardians (ch, victim, cmd);
            act ("$N engages $n in combat.", false,
                 victim, 0, ch, TO_NOTVICT | _ACT_FORMAT);

            sprintf(buf, "engage #5%s#0 in combat.", char_short(victim));
            watched_action(ch, buf, 1, 0);
        }

        if (cmd != 2 && get_second_affect(ch, SA_AIMSTRIKE, NULL))
            remove_second_affect(get_second_affect(ch, SA_AIMSTRIKE, NULL));

        hit_char (ch, victim, 0);

        WAIT_STATE (ch, 9);

        if (victim->deleted)
            return;

        if (IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim))
            add_memory (ch, victim);

        /* Looks like a problem.  If you hit/kill in one hit, then
           trigger isn't called. */

        trigger (ch, argument, TRIG_HIT);

        if (ch->fighting == victim && IS_SUBDUEE (victim))
            stop_fighting (ch);
    }

    else if (ch->fighting == victim &&
             !IS_SET (ch->flags, FLAG_KILL) && AWAKE (ch) && cmd == 1)
    {
        act ("You will try to kill $N.", false, ch, 0, victim, TO_CHAR);
        ch->flags |= FLAG_KILL;
    }

    else if (ch->fighting == victim &&
             IS_SET (ch->flags, FLAG_KILL) && AWAKE (ch) && cmd == 0)
    {
        act ("You will try NOT to kill $N.", false, ch, 0, victim, TO_CHAR);
        ch->flags &= ~FLAG_KILL;
    }

    else if (ch->fighting &&
             (GET_POS (ch) == FIGHT ||
              GET_POS (ch) == STAND) && victim != ch->fighting)
    {

        if (ch->agi <= 9)
            ch->balance += -8;
        else if (ch->agi > 9 && ch->agi <= 13)
            ch->balance += -7;
        else if (ch->agi > 13 && ch->agi <= 15)
            ch->balance += -6;
        else if (ch->agi > 15 && ch->agi <= 18)
            ch->balance += -5;
        else
            ch->balance += -3;
        ch->balance = MAX (ch->balance, -25);

        if (ch->balance < -15)
        {
            act ("You need more balance before you can try to attack $N!",
                 false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (victim->formation > 1 && do_group_size(victim) > 9)
        {
            for (tch = ch->room->people; tch; tch = tch->next_in_room)
            {
                if (tch->following != victim->following && tch->following != victim)
                    continue;

                if (num_attackers(tch) <= 2
                        && tch->formation == 1
                        && GET_POS (tch) >= POSITION_SITTING)
                {
                    found = true;
                    break;
                }
            }

            if (found && CAN_SEE(tch, ch))
            {
                sprintf(buf, "Your attempted attack at #5%s#0 is obstructed, instead you find yourself engaged by  #5%s#0.", char_short(victim), char_short(tch));
                act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                sprintf(buf, "You intercept $n's attack at #5%s#0.", char_short(victim));
                act(buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
                victim = tch;
            }
        }

        if (ch->fighting && !found)
        {

            if (ch->fighting->fighting == ch)
            {

                agi_diff = GET_AGI (ch) - GET_AGI (ch->fighting);

                if (agi_diff > number (-10, 10) && (number (0, 19) != 0))
                {

                    act ("You fail to shift your attention away from $N.",
                         false, ch, 0, ch->fighting,
                         TO_CHAR | _ACT_FORMAT | _ACT_COMBAT);
                    act ("$N fails to shift their attention away from you.",
                         false, ch->fighting, 0, ch,
                         TO_CHAR | _ACT_FORMAT | _ACT_COMBAT);
                    act ("$N fails to shift their attention away from $n.",
                         false, ch->fighting, 0, ch,
                         TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);

                    return;
                }
            }

            act ("You stop fighting $N.", false, ch, 0, ch->fighting, TO_CHAR);
            act ("You ready yourself for battle with $N.",
                 false, ch, 0, victim, TO_CHAR);
            stop_fighting (ch);

        }

        if (!found)
            act ("You notice $N's attention shift toward you!",
                 false, victim, 0, ch, TO_CHAR);

        if ((is_area_enforcer (victim)) && IS_NPC (victim) && !get_affect(victim, MAGIC_ALERTED))
        {
            do_alert (victim, "", 0);
            magic_add_affect (ch, MAGIC_ALERTED, 90, 0, 0, 0, 0);
        }

        if (GET_POS (ch) != POSITION_DEAD && GET_POS (victim) != POSITION_DEAD)
            criminalize (ch, victim, vnum_to_room (victim->in_room)->zone, CRIME_KILL);

        set_fighting (ch, victim);

        if (cmd == 0)
            ch->flags &= ~FLAG_KILL;
        else
            ch->flags |= FLAG_KILL;

        if (IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim))
            add_memory (ch, victim);

        trigger (ch, argument, TRIG_HIT);

    }

    else
        send_to_char ("You're doing the best you can!\n\r", ch);
}

void
do_strike (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *victim, *tch;
    OBJ_DATA *obj;
    int i;
    char buf[MAX_STRING_LENGTH];
    char original[MAX_STRING_LENGTH];

    if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
        send_to_char ("You cannot do this in an OOC area.\n", ch);
        return;
    }

    if (is_room_affected (ch->room->affects, MAGIC_ROOM_CALM))
    {
        act ("Try as you might, you simply cannot muster the will to break "
             "the peace that pervades the area.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (IS_SET(ch->room->room_flags, PEACE))
    {
        act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    /*
    if(get_soma_affect(ch, SOMA_PLANT_VISIONS) && number(0,30) > GET_WIL(ch))
    {
      act ("You're paralyzed with fear, and unable to undertake such an action!", false, ch, 0, 0, TO_CHAR);
      return;
    }
    */

    if (IS_SET (ch->flags, FLAG_PACIFIST))
    {
        send_to_char ("Remove your pacifist flag, first...\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if ((obj = get_equip (ch, WEAR_BOTH)))
    {
        if (obj->o.weapon.use_skill == SKILL_AIM ||
                obj->o.weapon.use_skill == SKILL_AIM)
        {
            send_to_char ("You can't use that in melee combat!\n", ch);
            return;
        }
    }

    if (get_affect (ch, MAGIC_AFFECT_PARALYSIS))
    {
        send_to_char ("You are paralyzed and unable to fight!\n\r", ch);
        return;
    }

    if (get_affect (ch, MAGIC_AFFECT_FEAR))
    {
        send_to_char ("You are too afraid to fight!\n\r", ch);
        return;
    }

    if (IS_SUBDUER (ch))
    {
        act ("You can't attack while you have $N subdued.",
             false, ch, 0, ch->subdue, TO_CHAR);
        return;
    }

    if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
            && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!')
    {
        sprintf (buf,
                 "You are in a lawful area; you would likely be flagged wanted for assault. "
                 "To confirm, type \'#6strike %s !#0\', without the quotes.",
                 original);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (!*buf)
    {

        if (IS_SET (ch->flags, FLAG_FLEE))
        {
            send_to_char ("You stop trying to flee.\n\r", ch);
            ch->flags &= ~FLAG_FLEE;
            return;
        }

        send_to_char ("Hit whom?\n\r", ch);
        return;
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You affectionately pat yourself.\n\r", ch);
        act ("$n affectionately pats $mself.", false, ch, 0, victim, TO_ROOM);
        return;
    }

    if (IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING)
            && AWAKE (victim) && !(victim->fighting))
    {
        send_to_char ("They are flying out of reach!\n", ch);
        return;
    }


    i = 0;
    for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch->fighting == victim)
        {
            if (++i >= 4)
            {
                act ("You can't find an opening to strike $N.",
                     false, ch, 0, victim, TO_CHAR);
                return;
            }

        }
    }

    ch->flags &= ~FLAG_FLEE;
    ch->act &= ~PLR_STOP;

    if (GET_POS (ch) == POSITION_STANDING && !ch->fighting)
    {

        if (ch->delay && ch->delay_type != DEL_CAST)
            break_delay (ch);

        ch->primary_delay = 0;
        ch->secondary_delay = 0;

        hit_char (ch, victim, 1);

        WAIT_STATE (ch, 9);

        if (victim->deleted)
            return;

        if (IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim))
            add_memory (ch, victim);

        /* Looks like a problem.  If you hit/kill in one hit, then
           trigger isn't called. */

        if (IS_NPC (victim))
            add_threat (victim, ch, 3);

        trigger (ch, argument, TRIG_HIT);
    }
    else
    {
        send_to_char
        ("You need to be standing and clear of combat to intiate a strike.\n",
         ch);
    }
}

void
do_nokill (CHAR_DATA * ch, char *argument, int cmd)
{
    send_to_char ("Please spell out all of 'kill' to avoid any mistakes.\n",
                  ch);
}

void
do_kill (CHAR_DATA * ch, char *argument, int cmd)
{
    char verify[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char original[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    sprintf (original, "%s", argument);
    argument = one_argument(argument, buf);

    if (IS_NPC (ch) || !GET_TRUST (ch))
    {
        do_hit (ch, original, 1);
        return;
    }
    else
    {
        if (!*buf)
        {
            send_to_char ("Smite whom?\n\r", ch);
            return;
        }
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("Smiting yourself is not a good move..\n\r", ch);
        return;
    }

    argument = one_argument(argument, verify);

    if (!IS_NPC (victim) && *verify != '!')
    {
        send_to_char ("Target is a player character.  Please use "
                      "'KILL <name> !' syntax if \n\ryou really mean it.'\n\r",
                      ch);
        return;
    }

    if (GET_TRUST (victim) > GET_TRUST (ch))
    {
        victim = ch;
    }
    act ("$n stares at you, narrowing $s eyes. Shortly thereafter, your heart obediently ceases to beat, and you feel death upon you...", false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
    act ("You narrow your eyes in concentration, and $N collapses, dead.",
         false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
    act ("$n stares at $N, narrowing $s eyes. Suddenly, $N collapses, dead.",
         false, ch, 0, victim, TO_NOTVICT | _ACT_FORMAT);
    die (victim);
}

void
do_command (CHAR_DATA * ch, char *argument, int cmd)
{
    int everyone = 0;
    CHAR_DATA *victim = NULL;
    CHAR_DATA *next_in_room = NULL;
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH];
    char command[MAX_STRING_LENGTH];
    bool is_robot = !str_cmp( lookup_race_variable( ch->race, RACE_NAME ), "robot" );

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("Command whom?\n\r", ch);
        return;
    }

    /* We can command invis chars as well, why not :) */

    if (!str_cmp (buf, "all"))
    {
        everyone = 1;
    }
    else if (!str_cmp (buf, "group") || !str_cmp (buf, "follower") || !str_cmp (buf, "followers"))
    {
        everyone = 2;
    }
    else if (!(victim = get_char_room_vis (ch, buf)) && !(victim = get_char_room (buf, ch->in_room)))
    {
        if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
                !(obj = get_obj_in_dark (ch, buf, ch->left_hand)) &&
                !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))

        {
            send_to_char ("You cannot see that person or remote here.\n", ch);
            return;
        }
    }

    if (obj)
    {
        if (GET_ITEM_TYPE(obj) != ITEM_E_REMOTE)
        {
            act ("But $p isn't a remote control.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }
        else if (obj->o.elecs.status == 0)
        {
            act ("But $p isn't switched on.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            return;
        }

        else if (!obj->o.od.value[3] || !(victim = get_char_id(obj->o.od.value[3])))
        {
            act ("But $p isn't synchronised to anything.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

			if (obj->o.od.value[3])
			{
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
            obj->o.od.value[3] = 0;
            obj->o.od.value[4] = 0;
            return;
        }
		else if (victim)
		{
			victim->mob->controller = obj->coldload_id;
		}
    }

    while (isspace (*argument))
        argument++;

    if (!*argument)
    {
        send_to_char ("What is your command?\n\r", ch);
        return;
    }

    strcpy (command, argument);

    if (victim == ch)
    {
        send_to_char ("You don't have to give yourself commands.\n\r", ch);
        return;
    }

    if (victim)
    {

        if ( !obj && !str_cmp(lookup_race_variable(victim->race, RACE_NAME), "robot") )
        {
            send_to_char ("You need a synchronized remote to command a robot.", ch);
            return;
        }

        if ( !is_leader (ch, victim) && victim->following != ch && !obj )
        {
            act ("You do not have the authority to command $N.", false, ch, 0,
                 victim, TO_CHAR);
            return;
        }

        if (!obj)
        {
            sprintf (buf, "#5%s#0 commands you to '%s'.\n", char_short (ch), command);
            buf[2] = toupper (buf[2]);
            send_to_char (buf, victim);
        }/*
        else
        {
            act ("$n presses a command into $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
        }*/

        /* Bag this for now per Regi */
        /*
         act ("$n gives $N an command.", false, ch, 0, victim, TO_NOTVICT);
         */

        if (victim->mob)
        {
            send_to_char ("Ok.\n", ch);
            command_interpreter (victim, command);
        }
        else
        {
            sprintf (buf, "You command #5%s#0 to '%s'.\n",
                     char_short (victim), command);
            send_to_char (buf, ch);
        }

        return;
    }
    else if (everyone == 1)
    {
        if ( is_robot )
            command_interpreter( ch, command );

        for (tch = ch->room->people; tch; tch = next_in_room)
        {
            next_in_room = tch->next_in_room;

            if (!is_leader (ch, tch) && (tch->following != ch))
                continue;

            if ( !str_cmp(lookup_race_variable(tch->race, RACE_NAME), "robot") )
            {
                if ( !is_robot )    // Only robots can command other robots.
                    continue;

                if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "comm")) ||
                    (tch->d_feat2 && !str_cmp(tch->d_feat2, "comm")) ||
                    (tch->d_feat3 && !str_cmp(tch->d_feat3, "comm")) ||
                    (tch->d_feat4 && !str_cmp(tch->d_feat4, "comm")) )
                {
                    ;
                } else
                {
                    continue;
                }
            } else {
                sprintf (buf, "#5%s#0 commands you to '%s'.\n", char_short (ch), command);
            }

            buf[2] = toupper (buf[2]);
            send_to_char (buf, tch);

            if (tch->mob)
                command_interpreter (tch, command);
        }
    }
    else if (everyone == 2)
    {
        if ( is_robot )
            command_interpreter( ch, command );

        for (tch = ch->room->people; tch; tch = next_in_room)
        {
            next_in_room = tch->next_in_room;

            if (tch->following != ch)
                continue;

            if ( !str_cmp(lookup_race_variable(tch->race, RACE_NAME), "robot") )
            {
                if ( !is_robot )    // Only robots can command other robots.
                    continue;

                if ((tch->d_feat1 && !str_cmp(tch->d_feat1, "comm")) ||
                    (tch->d_feat2 && !str_cmp(tch->d_feat2, "comm")) ||
                    (tch->d_feat3 && !str_cmp(tch->d_feat3, "comm")) ||
                    (tch->d_feat4 && !str_cmp(tch->d_feat4, "comm")))
                {
                    ;
                } else
                {
                    continue;
                }
            } else {
                sprintf (buf, "#5%s#0 commands you to '%s'.\n", char_short (ch), command);
            }

            buf[2] = toupper (buf[2]);
            send_to_char (buf, tch);

            if (tch->mob)
                command_interpreter (tch, command);
        }
    }

    send_to_char ("Ok.\n", ch);
}


void
retreat (CHAR_DATA* ch, int direction, CHAR_DATA* leader)
{
    // base number of seconds
    int duration = 0;
    AFFECTED_TYPE *af;

    if ((af = get_affect (ch, AFFECT_GROUP_RETREAT)))
    {
        if (af->a.spell.sn == direction)
        {
            send_to_char ("You are already retreating in that direction!\n", ch);
            return;
        }
        else
        {
            remove_affect_type (ch, AFFECT_GROUP_RETREAT);
        }
    }

    char message[AVG_STRING_LENGTH];
    if (leader)
    {
        if (ch == leader)
        {
            sprintf(message,
                    "You command your group to retreat %sward.\n",
                    dirs[direction]);
            send_to_char (message, ch);
            sprintf (message, "$n's group begins to fall back to the %s.",
                     dirs[direction]);
            act (message, false, ch, 0, 0, TO_ROOM);
        }
        else
        {
            sprintf(message,
                    "Your group begins to retreat %sward "
                    "at the command of #5%s#0.\n",
                    dirs[direction], char_short(leader));
            send_to_char (message, ch);
        }
    }
    else
    {
        sprintf(message,
                "You attempt to retreat %sward.\n",
                dirs[direction]);
        send_to_char (message, ch);
        sprintf (message, "$n begins to fall back to the %s.",
                 dirs[direction]);
        act (message, false, ch, 0, 0, TO_ROOM);
    }

    if (!ch->following || ch->fighting || leader)
    {
        duration = 40;

        // Commenting out a lot of this stuff, as retreating piece-meal is counter to the point
        // of a retreat, as opposed to a whole-sale route. You retreat as a group, to help prevent
        // slaughter. If you want to flee in bits and drabs, then use the flee command.

        /*
        // terrain penalty
        duration += movement_loss[ch->room->sector_type];

        // wound penalty
        int damage = 0;
        int health_percent;
        for ( WOUND_DATA *wound = ch->wounds; wound; wound = wound->next)
        {
        damage += wound->damage;
        }
        damage += ch->damage;
        health_percent = (damage * 100) / ch->max_hit;
        if (health_percent < 50)
        {
        // penalty or bonus depending on your luck and ability.
        duration += number (12,20);
        duration -= MAX(ch->wil,ch->con);
        }

        // defensive stance bonus
        duration -= (IS_SET (ch->flags, FLAG_PACIFIST)) ? (5) : (ch->fight_mode);

        // agility roll bonus
        duration -= number (1, ch->agi);*/

        magic_add_affect (ch, AFFECT_GROUP_RETREAT, duration, 0, 0, 0, direction);
    }
    else
    {
        do_move (ch, "", direction);
    }


}


void
do_flee (CHAR_DATA * ch, char *argument, int cmd)
{
    int dir;

    if (!can_move(ch))
        return;

    if (!ch->fighting)
    {
        send_to_char ("You're not fighting.\n\r", ch);
        return;
    }

    if (get_affect (ch, MAGIC_AFFECT_PARALYSIS))
    {
        send_to_char ("You are paralyzed and unable to flee!\n\r", ch);
        return;
    }


    /*
    // Old Somatics Code no longer needed
    if (get_soma_affect (ch, SOMA_STIM_FURY))
    {
        damage = get_damage_total(ch);
        if (damage / ch->max_hit > 0.25)
            send_to_char ("Flee? You frenzied state causes you to laugh at the mere suggestion!\n\r", ch);
        return;
    }
    */


    if (IS_SET (ch->flags, FLAG_FLEE))
    {
        send_to_char ("You are already trying to escape!\n\r", ch);
        return;
    }

    for (dir = 0; dir <= LAST_DIR; dir++)
        if (CAN_GO (ch, dir) && !isguarded (ch->room, dir))
            break;

    if (dir == 6)
    {
        send_to_char ("THERE IS NOWHERE TO FLEE!!\n\r", ch);
        return;
    }

    ch->flags |= FLAG_FLEE;

    send_to_char ("You resolve to escape combat. . .\n\r", ch);

    act ("$n's eyes dart about looking for an escape path!",
         false, ch, 0, 0, TO_ROOM);
}

int
flee_attempt (CHAR_DATA * ch)
{
    int dir;
    int enemies = 0;
    int mobless_count = 0;
    int mobbed_count = 0;
    int mobless_dirs[6];
    int mobbed_dirs[6];
    CHAR_DATA *tch;
    char buf[MAX_STRING_LENGTH];
    ROOM_DATA *troom;
    /*
    	if ( IS_SET (ch->flags, FLAG_SUBDUING) ) {
    		ch->flags &= ~FLAG_FLEE;
    		return 0;
    	}
    */
    if (GET_POS (ch) < FIGHT)
        return 0;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {

        if (tch->fighting != ch)
            continue;

        if (GET_POS (tch) != FIGHT && GET_POS (tch) != STAND)
            continue;

        if (!CAN_SEE (tch, ch))
            continue;

        enemies++;
    }

    for (dir = 0; dir <= LAST_DIR; dir++)
    {

        if (!CAN_GO (ch, dir))
            continue;

        if (vnum_to_room (EXIT (ch, dir)->to_room)->people)
            mobbed_dirs[mobbed_count++] = dir;
        else
            mobless_dirs[mobless_count++] = dir;
    }

    if (!mobbed_count && !mobless_count)
    {
        send_to_char ("There is nowhere to go!  You continue fighting!\n\r",
                      ch);
        ch->flags &= ~FLAG_FLEE;
        return 0;
    }

    if (enemies && number (0, enemies))
    {
        switch (number (1, 3))
        {
        case 1:
            send_to_char ("You attempt escape, but fail . . .\n\r", ch);
            break;
        case 2:
            send_to_char ("You nearly escape, but are blocked . . .\n\r", ch);
            break;
        case 3:
            send_to_char ("You continue seeking escape . . .\n\r", ch);
            break;
        }

        act ("$n nearly flees!", true, ch, 0, 0, TO_ROOM | _ACT_COMBAT);

        return 0;
    }

    if (mobless_count)
        dir = mobless_dirs[number (0, mobless_count - 1)];
    else
        dir = mobbed_dirs[number (0, mobbed_count - 1)];

    troom = ch->room;

    /* stop_fighting_sounds (ch, troom); */
    stop_followers (ch);
    if (ch->fighting)
    {
        stop_fighting (ch);
    }

    do_move (ch, "", dir);

    act ("$n #3flees!#0", false, ch, 0, 0, TO_ROOM);

    sprintf (buf, "#3YOU FLEE %s!#0", dirs[dir]);
    act (buf, false, ch, 0, 0, TO_CHAR);

    if (!enemies)
        sprintf (buf, "\nYou easily escaped to the %s.\n\r", dirs[dir]);
    else
        sprintf (buf, "\nYou barely escaped to the %s.\n\r", dirs[dir]);

    ch->formation = 0;
    ch->following = 0;

    if (ch->room != troom)
        send_to_char (buf, ch);

    ch->flags &= ~FLAG_FLEE;

    return 1;
}

/* In case victim is being guarded, make sure rescue affects are active. */

void
guard_check (CHAR_DATA * victim)
{
    CHAR_DATA *tch;
    AFFECTED_TYPE *af;

    for (tch = victim->room->people; tch; tch = tch->next_in_room)
    {

        if (!(af = get_affect (tch, MAGIC_GUARD)))
            continue;

        if ((CHAR_DATA *) af->a.spell.t == victim &&
                !get_second_affect (tch, SA_RESCUE, NULL))
        {
            add_second_affect (SA_RESCUE, 1, tch, (OBJ_DATA *) victim, NULL, 0);


            // If you're guarding and you've got three or more attackers, you lose the
            // guard -- can't keep it up forever my friend.

            if (num_attackers(tch) >= 3)
            {
                remove_affect_type (tch, MAGIC_GUARD);
            }
        }

    }
}

void
do_guard (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *target = NULL, *tch = NULL;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH];
    int dir;

    if (IS_SET (ch->room->room_flags, OOC))
    {
        send_to_char ("That command cannot be used in an OOC area.\n", ch);
        return;
    }

    if ((af = get_affect (ch, AFFECT_GUARD_DIR)))
        affect_remove (ch, af);

    argument = one_argument (argument, buf);

    if (*buf && !cmd && !(target = get_char_room_vis (ch, buf)))
    {
        if ((dir = index_lookup (dirs, buf)) == -1)
        {
            send_to_char ("Who or what did you want to guard?\n", ch);
            return;
        }

        if (!(af = get_affect (ch, AFFECT_GUARD_DIR)))
            magic_add_affect (ch, AFFECT_GUARD_DIR, -1, 0, 0, 0, 0);

        af = get_affect (ch, AFFECT_GUARD_DIR);

        af->a.shadow.shadow = NULL;
        af->a.shadow.edge = dir;

		if (IS_SET(ch->room->room_flags, BIG_ROOM) ||
			IS_SET(ch->room->room_flags, LAWFUL))
		{
			sprintf (buf, "You will now guard the %s exit, but it will require at least three people to be effective.\n", dirs[dir]);
		}
		else
		{
			sprintf (buf, "You will now guard the %s exit.\n", dirs[dir]);
		}


        send_to_char (buf, ch);
        sprintf (buf, "$n moves to block the %s exit.", dirs[dir]);
        act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        return;

    }
    else if (cmd && !(target = (CHAR_DATA *)cmd))
        return;



    if ((!*buf || target == ch))
    {
        if (!(af = get_affect (ch, MAGIC_GUARD))
                && !(af = get_affect (ch, AFFECT_GUARD_DIR)))
        {
            send_to_char ("You are not currently guarding anything.\n", ch);
            return;
        }
        else if ((af = get_affect (ch, MAGIC_GUARD))
                 && (tch = (CHAR_DATA *) af->a.spell.t) != NULL)
        {
            act ("You cease guarding $N.", true, ch, 0, tch,
                 TO_CHAR | _ACT_FORMAT);
            act ("$n ceases guarding you.", false, ch, 0, tch,
                 TO_VICT | _ACT_FORMAT);
            act ("$n ceases guarding $N.", false, ch, 0, tch,
                 TO_NOTVICT | _ACT_FORMAT);
            remove_affect_type (ch, MAGIC_GUARD);
            return;
        }
        else if ((af = get_affect (ch, AFFECT_GUARD_DIR)))
        {
            act ("You cease guarding the exit.", true, ch, 0, 0, TO_CHAR);
            remove_affect_type (ch, AFFECT_GUARD_DIR);
            return;
        }
        else
        {
            send_to_char ("You cease guarding.\n", ch);
            remove_affect_type (ch, MAGIC_GUARD);
            remove_affect_type (ch, AFFECT_GUARD_DIR);
            return;
        }
    }

    if (get_affect (target, MAGIC_GUARD)
            || get_affect (target, AFFECT_GUARD_DIR))
    {
        send_to_char ("They're already trying to guard something themselves!\n",
                      ch);
        return;
    }

    if (num_attackers(ch) >= 3)
    {
        act ("You are under attack from too many foes to help $N.", false, ch, 0, target, TO_CHAR);
        return;
    }


    if ((af = get_affect (ch, MAGIC_GUARD)))
        affect_remove (ch, af);

    magic_add_affect (ch, MAGIC_GUARD, -1, 0, 0, 0, 0);

    if (!(af = get_affect (ch, MAGIC_GUARD)))
    {
        send_to_char ("There is a bug in guard.  Please let an admin "
                      "know.\n", ch);
        return;
    }

    af->a.spell.t = (long int) target;

    act ("You will now guard $N.", false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
    act ("$n moves into position to guard you.", false, ch, 0, target,
         TO_VICT | _ACT_FORMAT);
    act ("$n moves into position to guard $N.", false, ch, 0, target,
         TO_NOTVICT | _ACT_FORMAT);
}



void
do_aide (CHAR_DATA *ch, char *argument, int cmd)
{
    char buf[AVG_STRING_LENGTH] = {'\0'};
    char original[MAX_STRING_LENGTH];
    CHAR_DATA *tch = NULL;
    CHAR_DATA *victim = NULL;
    CHAR_DATA *ally = NULL;
    OBJ_DATA *obj = NULL;
    SECOND_AFFECT *sa = NULL;
    bool found = false;
    int i;
    int agi_diff = 0;

    sprintf (original, "%s", argument);

    argument = one_argument(argument, buf);

    if (!*buf)
    {
        if (!(ally = ch->following))
        {
            send_to_char("Aide who?\n", ch);
            return;
        }
    }
    else if (!(ally = get_char_room_vis (ch, buf)))
    {
        send_to_char("Aide who?\n", ch);
        return;
    }

    if (!(victim = ally->fighting))
    {
        act("But $N isn't fighting anyone.", false, ch, 0, ally, TO_CHAR | _ACT_FORMAT);
        return;
    }

    /* cmd = 0 if hit,
       cmd = 1 if kill */

    if (IS_SWIMMING (ch))
    {
        send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

    if (IS_FLOATING (ch))
    {
        send_to_char ("You can't do that while floating!\n", ch);
    }

    if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
        send_to_char ("You cannot do this in an OOC area.\n", ch);
        return;
    }

    if (is_room_affected (ch->room->affects, MAGIC_ROOM_CALM))
    {
        act ("Try as you might, you simply cannot muster the will to break "
             "the peace that pervades the area.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    if (IS_SET(ch->room->room_flags, PEACE))
    {
        act ("Something prohibits you from taken such an action.", false, ch, 0, 0, TO_CHAR);
        return;
    }

    /*
    if(get_soma_affect(ch, SOMA_PLANT_VISIONS) && number(0,30) > GET_WIL(ch))
    {
      act ("You're paralyzed with fear, and unable to undertake such an action!", false, ch, 0, 0, TO_CHAR);
      return;
    }
    */

    if (IS_SET (ch->flags, FLAG_PACIFIST))
    {
        send_to_char ("Remove your pacifist flag, first...\n", ch);
        return;
    }

    if ((obj = get_equip (ch, WEAR_BOTH)))
    {
        if (obj->o.weapon.use_skill == SKILL_AIM ||
                obj->o.weapon.use_skill == SKILL_AIM)
        {
            send_to_char ("You can't use that in melee combat!\n", ch);
            return;
        }
    }

    if (get_affect (ch, MAGIC_AFFECT_PARALYSIS))
    {
        send_to_char ("You are paralyzed and unable to fight!\n\r", ch);
        return;
    }

    if (get_affect (ch, MAGIC_AFFECT_FEAR))
    {
        send_to_char ("You are too afraid to fight!\n\r", ch);
        return;
    }

    if (get_affect (ch, AFFECT_GROUP_RETREAT))
    {
        send_to_char ("You stop trying to retreat.\n", ch);
        remove_affect_type (ch, AFFECT_GROUP_RETREAT);
    }

    if (IS_SUBDUER (ch))
    {
        act ("You can't attack while you have $N subdued.",
             false, ch, 0, ch->subdue, TO_CHAR);
        return;
    }

    if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
            && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!')
    {
        sprintf (buf,
                 "You are in a lawful area; you would likely be flagged wanted for assault. "
                 "To confirm, type \'#6hit %s !#0\', without the quotes.",
                 original);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (IS_SET (ch->room->room_flags, BRAWL) && IS_SET (ch->room->room_flags, LAWFUL)
            && *argument != '!' && has_weapon(ch))
    {
        sprintf (buf, "You are in a lawful, but brawlable area area; if you hit this person with that weapon you're holding, you'll be wanted for assault. You can either put away the weapon, or type \'#6hit %s !#0\', without the quotes, to continue.", original);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You affectionately pat yourself.\n\r", ch);
        act ("$n affectionately pats $mself.", false, ch, 0, victim, TO_ROOM);
        return;
    }

    if ((sa = get_second_affect(ch, SA_WARDED, NULL)))
    {
        if ((CHAR_DATA *) sa->obj == victim)
        {
            send_to_char ("You're still warded away by their weapon.\n\r", ch);
            return;
        }
    }

    // Make sure people don't hit the wrong person.
    if (are_grouped (ch, victim) && is_brother (ch, victim) && *argument != '!')
    {
        sprintf (buf,
                 "#1You are about to attack $N #1who is a fellow group member!#0 To confirm, type \'#6hit %s !#0\', without the quotes.",
                 original);
        act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
        return;
    }

    //  sprintf (buf, "victim->fighting->short_descr = #2%s#0\n", victim->fighting->short_descr);
    //  send_to_gods(buf);
    // send_to_gods(ch->name);

    if (IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING)
            && AWAKE (victim) && !(victim->fighting)) // and victim is not fighting you
    {
        send_to_char ("They are flying out of reach!\n", ch);
        return;
    }

    if (IS_SET (victim->act, ACT_VEHICLE) || IS_SET (victim->act, ACT_PASSIVE))
    {
        send_to_char ("How do you propose to kill an inanimate object, hmm?\n",
                      ch);
        return;
    }

    i = 0;
    for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
    {
        if (tch->fighting == victim)
        {
            if (++i >= 4)
            {
                act ("You can't find an opening to attack $N.",
                     false, ch, 0, victim, TO_CHAR);
                return;
            }

        }
    }

    if (IS_SET (victim->act, ACT_PREY) && AWAKE (victim))
    {
        if (!get_affect (ch, MAGIC_HIDDEN) || !skill_use (ch, SKILL_SNEAK, -30))
        {
            act
            ("As you approach, $N spots you and darts away! Try using a ranged weapon or an ambush from hiding instead.",
             false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            act ("As $n approaches, $N spots $m and darts away.", false, ch, 0,
                 victim, TO_ROOM | _ACT_FORMAT);
            npc_evasion (victim, -1);
            add_threat (victim, ch, 7);
            remove_affect_type (ch, MAGIC_HIDDEN);
            return;
        }
    }

    ch->flags &= ~FLAG_FLEE;
    ch->act &= ~PLR_STOP;

    if (GET_POS (ch) == POSITION_STANDING &&
            !ch->fighting && victim != ch->fighting)
    {

        if (ch->delay && ch->delay_type != DEL_CAST)
            break_delay (ch);

        /*
              Removed to prevent easy cheesing of combat.

              ch->primary_delay = 0;
              ch->secondary_delay = 0;
        */

        if (victim->formation > 1 && do_group_size(victim) > 9)
        {
            for (tch = ch->room->people; tch; tch = tch->next_in_room)
            {
                if (tch->following != victim->following && tch->following != victim)
                    continue;

                if (num_attackers(tch) <= 2
                        && tch->formation == 1
                        && GET_POS (tch) >= POSITION_SITTING)
                {
                    found = true;
                    break;
                }
            }

            if (found && CAN_SEE(tch, ch))
            {
                sprintf(buf, "Your attempted attack at #5%s#0 is obstructed, instead you find yourself engaged by  #5%s#0.", char_short(victim), char_short(tch));
                act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                sprintf(buf, "You intercept $n's attack at #5%s#0.", char_short(victim));
                act(buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
                victim = tch;
            }
        }

        if (cmd == 0)
            ch->flags &= ~FLAG_KILL;
        else
            ch->flags |= FLAG_KILL;

        if (GET_POS (ch) != POSITION_DEAD && GET_POS (victim) != POSITION_DEAD)
        {
            criminalize (ch, victim, vnum_to_room (victim->in_room)->zone, CRIME_KILL);
        }

        set_fighting (ch, victim);
        if (!victim->fighting)
        {
            set_fighting (victim, ch);
            notify_guardians (ch, victim, cmd);
            act ("$N engages $n in combat.", false,
                 victim, 0, ch, TO_NOTVICT | _ACT_FORMAT);

            sprintf(buf, "engage #5%s#0 in combat.", char_short(victim));
            watched_action(ch, buf, 1, 0);
        }

        if (cmd != 2 && get_second_affect(ch, SA_AIMSTRIKE, NULL))
            remove_second_affect(get_second_affect(ch, SA_AIMSTRIKE, NULL));

        hit_char (ch, victim, 0);

        WAIT_STATE (ch, 9);

        if (victim->deleted)
            return;

        if (IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim))
            add_memory (ch, victim);

        /* Looks like a problem.  If you hit/kill in one hit, then
           trigger isn't called. */

        trigger (ch, argument, TRIG_HIT);

        if (ch->fighting == victim && IS_SUBDUEE (victim))
            stop_fighting (ch);
    }

    else if (ch->fighting == victim &&
             !IS_SET (ch->flags, FLAG_KILL) && AWAKE (ch) && cmd == 1)
    {
        act ("You will try to kill $N.", false, ch, 0, victim, TO_CHAR);
        ch->flags |= FLAG_KILL;
    }

    else if (ch->fighting == victim &&
             IS_SET (ch->flags, FLAG_KILL) && AWAKE (ch) && cmd == 0)
    {
        act ("You will try NOT to kill $N.", false, ch, 0, victim, TO_CHAR);
        ch->flags &= ~FLAG_KILL;
    }

    else if (ch->fighting &&
             (GET_POS (ch) == FIGHT ||
              GET_POS (ch) == STAND) && victim != ch->fighting)
    {

        if (ch->agi <= 9)
            ch->balance += -8;
        else if (ch->agi > 9 && ch->agi <= 13)
            ch->balance += -7;
        else if (ch->agi > 13 && ch->agi <= 15)
            ch->balance += -6;
        else if (ch->agi > 15 && ch->agi <= 18)
            ch->balance += -5;
        else
            ch->balance += -3;
        ch->balance = MAX (ch->balance, -25);

        if (ch->balance < -15)
        {
            act ("You need more balance before you can try to attack $N!",
                 false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
            return;
        }

        if (victim->formation > 1 && do_group_size(victim) > 9)
        {
            for (tch = ch->room->people; tch; tch = tch->next_in_room)
            {
                if (tch->following != victim->following && tch->following != victim)
                    continue;

                if (num_attackers(tch) <= 2
                        && tch->formation == 1
                        && GET_POS (tch) >= POSITION_SITTING)
                {
                    found = true;
                    break;
                }
            }

            if (found && CAN_SEE(tch, ch))
            {
                sprintf(buf, "Your attempted attack at #5%s#0 is obstructed, instead you find yourself engaged by  #5%s#0.", char_short(victim), char_short(tch));
                act(buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                sprintf(buf, "You intercept $n's attack at #5%s#0.", char_short(victim));
                act(buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
                victim = tch;
            }
        }

        if (ch->fighting && !found)
        {

            if (ch->fighting->fighting == ch)
            {

                agi_diff = GET_AGI (ch) - GET_AGI (ch->fighting);

                if (agi_diff > number (-10, 10) && (number (0, 19) != 0))
                {

                    act ("You fail to shift your attention away from $N.",
                         false, ch, 0, ch->fighting,
                         TO_CHAR | _ACT_FORMAT | _ACT_COMBAT);
                    act ("$N fails to shift their attention away from you.",
                         false, ch->fighting, 0, ch,
                         TO_CHAR | _ACT_FORMAT | _ACT_COMBAT);
                    act ("$N fails to shift their attention away from $n.",
                         false, ch->fighting, 0, ch,
                         TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);

                    return;
                }
            }

            act ("You stop fighting $N.", false, ch, 0, ch->fighting, TO_CHAR);
            act ("You ready yourself for battle with $N.",
                 false, ch, 0, victim, TO_CHAR);
            stop_fighting (ch);

        }

        if (!found)
            act ("You notice $N's attention shift toward you!",
                 false, victim, 0, ch, TO_CHAR);

        if ((is_area_enforcer (victim)) && IS_NPC (victim) && !get_affect(victim, MAGIC_ALERTED))
        {
            do_alert (victim, "", 0);
            magic_add_affect (ch, MAGIC_ALERTED, 90, 0, 0, 0, 0);
        }

        if (GET_POS (ch) != POSITION_DEAD && GET_POS (victim) != POSITION_DEAD)
            criminalize (ch, victim, vnum_to_room (victim->in_room)->zone, CRIME_KILL);

        set_fighting (ch, victim);

        if (cmd == 0)
            ch->flags &= ~FLAG_KILL;
        else
            ch->flags |= FLAG_KILL;

        if (IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim))
            add_memory (ch, victim);

        trigger (ch, argument, TRIG_HIT);

    }

    else
        send_to_char ("You're doing the best you can!\n\r", ch);
}
