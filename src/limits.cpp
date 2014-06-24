/*------------------------------------------------------------------------\
 |  limits.c : Gain Control Module                     www.middle-earth.us |
 |  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
 |  Derived under license from DIKU GAMMA (0.0).                           |
 \------------------------------------------------------------------------*/


#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "math.h"
#include "utility.h"


// Determines a ch's armour penalty.

int
armor_penalty (CHAR_DATA * ch)
{
    /* Amour provides inherent restrictions to movement, not just added weight.
     Measures armour on legs, body, arms, about and over.

     We add up all the spots covered, timesing the percent of the body covered
     by an arbitrary "hinderance" value of that type of armour:

     Cloth is 1.
     Metal is 3.
     Everything else is 2.

     Secondary armour is only half as hindering.

     We then divide the total by 100.

     If the total is 4 or greater, extremely encumbered.
     3, heavily encumbered,
     2, moderately encumbered,
     1, lightly encumbered,
     0, not at all encumberend

     Called in determining movement regen rates, movement speed, stam losses
     from movement, climbing tests, swim tests, and so on.
     */

    OBJ_DATA *eq = NULL;

    int prim_locs[MAX_HITLOC] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sec_locs[MAX_HITLOC] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    float i = 0;

    for (eq = ch->equip; eq; eq = eq->next_content)
    {
        for (int location = 0; location < MAX_HITLOC; location++)
        {
            if (GET_ITEM_TYPE(eq) == ITEM_ARMOR && IS_SET(eq->o.od.value[2], 1 << location))
            {
                if (eq->o.armor.armor_type == 1)
                    prim_locs[location] += 3 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 0)
                    prim_locs[location] += 1 * body_tab[0][location].percent;
                else
                    prim_locs[location] += 2 * body_tab[0][location].percent;
            }

            if (GET_ITEM_TYPE(eq) == ITEM_ARMOR && IS_SET(eq->o.od.value[3], 1 << location))
            {
                if (eq->o.armor.armor_type == 1)
                    sec_locs[location] += 0.75 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 0)
                    sec_locs[location] += 0.25 * body_tab[0][location].percent;
                else
                    sec_locs[location] += 0.50 * body_tab[0][location].percent;
            }

            // We add an extra 5~10% weight for water-logged items.
            if (eq->enviro_conds[COND_WATER] > 25)
            {
                double enviro_tier = eq->enviro_conds[COND_WATER] > 75 ? 1.15 : eq->enviro_conds[COND_WATER] > 50 ? 1.10 : 1.05;
                prim_locs[location] = sec_locs[location] * enviro_tier;
                sec_locs[location] = sec_locs[location] * enviro_tier;
            }
        }
    }

    for (int ind = 0; ind < MAX_HITLOC; ind++)
    {
        i += prim_locs[ind];
        i += sec_locs[ind];
    }

    i = i / 105;

    if (i >= 3) // full suit of chain or scale, with or without fur
        return 3;
    else if (i >= 2) // full suit of leather plus fur, or leather and chain + scale
        return 2;
    else if (i >= 1) // full suit of leather sans fur, only chain/scale hauberk, or quilted + fur
        return 1;
    else // incomplete leather, or full quilted.
        return 0;
}

// Determines a ch's armour word: what we'll use to describe them.
int
armor_descript (CHAR_DATA * ch)
{
    /* Amour provides inherent restrictions to movement, not just added weight.
     Measures armour on legs, body, arms, about and over.
     Armour that covers multiple spots is deemed to be better proportioned to take
     the weight.

     Cloth is 1.
     Metal and Kevlar is 2.
     Ceramic is 3
     Power is 4.

     If the total is 4, return 1
     If the total is 6, return 2
     If the total is 9, return 3,
     If the total is 12, return 5.
     */

    OBJ_DATA *eq = NULL;

    int prim_locs[MAX_HITLOC] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sec_locs[MAX_HITLOC] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    float i = 0;

    for (eq = ch->equip; eq; eq = eq->next_content)
    {
        for (int location = 0; location < MAX_HITLOC; location++)
        {
            if (GET_ITEM_TYPE(eq) == ITEM_ARMOR && IS_SET(eq->o.od.value[2], 1 << location))
            {
                if (eq->o.armor.armor_type == 0)
                    prim_locs[location] += 1 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 1 || eq->o.armor.armor_type == 2)
                    prim_locs[location] += 2 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 3)
                    prim_locs[location] += 3 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 4)
                    prim_locs[location] += 4 * body_tab[0][location].percent;
            }

            if (GET_ITEM_TYPE(eq) == ITEM_ARMOR && IS_SET(eq->o.od.value[3], 1 << location))
            {
                if (eq->o.armor.armor_type == 0)
                    sec_locs[location] += 0.25 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 1 || eq->o.armor.armor_type == 2)
                    sec_locs[location] += 0.50 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 3)
                    sec_locs[location] += 0.75 * body_tab[0][location].percent;
                else if (eq->o.armor.armor_type == 4)
                    sec_locs[location] += 1.00 * body_tab[0][location].percent;
            }
        }
    }

    for (int ind = 0; ind < MAX_HITLOC; ind++)
    {
        i += prim_locs[ind];
        i += sec_locs[ind];
    }

    i = i / 100;

    if (i >= 5) // heavily armoured - full suit of metal armour
        return 4;
    else if (i >= 3) // full suit of chain or scale, with or without fur
        return 3;
    else if (i >= 2) // full suit of leather plus fur, or leather and chain + scale
        return 2;
    else if (i >= 1) // full suit of leather sans fur, only chain/scale hauberk, or quilted + fur
        return 1;
    else // incomplete leather, or full quilted.
        return 0;
}

int
shock_gain (CHAR_DATA * ch)
{
    int gain;
    int shock_rate;
    int shock_gained;

    /* Shock_rate is 100 moves per 5 mins  ( * 10000 for granularity ) */

    shock_rate = 10000 * GET_WIL (ch) * UPDATE_PULSE / (12 * 16);

    gain = shock_rate;

    switch (GET_POS (ch))
    {
    case POSITION_SLEEPING:
        gain += gain >> 1;
        break;
    }

    // If we're under the affect of some trauma, then we don't regenerate shock.
    for (AFFECTED_TYPE *af = ch->hour_affects; af; af = af->next)
    {
        if (af->type >= AFFECT_TRAUMA_PINNED && af->type <= AFFECT_TRAUMA_SUSTAINED)
            return 0;
    }
    // If you're hungry, thirsy, or got a busted rib, very slow movement regen.
    if (!ch->thirst)
        gain >>= 1;

    if (get_affect (ch, MAGIC_STARVE_FOUR))
        gain >>=3;
    else if (get_affect (ch, MAGIC_STARVE_THREE))
        gain >>=2;
    else if (get_affect (ch, MAGIC_STARVE_TWO))
        gain >>=2;
    else if (get_affect (ch, MAGIC_STARVE_ONE))
        gain >>=1;

    // If we're sufficently shocked, we only gain shock back at half the rate.
    if (shock_tier(ch) >= 2)
        gain >>= 1;

    if (ch->shock < 0)
        ch->shock = 0;

    ch->shock += gain;

    shock_gained = ch->shock / 10000;

    ch->shock -= shock_gained * 10000;

    if (shock_gained <= 0)
        shock_gained = 1;

    if (ch->shock > ch->max_shock)
        ch->shock = ch->max_shock;

    return shock_gained;
}

int
move_gain (CHAR_DATA * ch)
{
    int gain;
    int move_rate;
    int moves_gained;
    //AFFECTED_TYPE *af;
    //AFFECTED_TYPE *taf;

    /* Move_rate is 100 moves per 5 mins  ( * 10000 for granularity ) */

    move_rate = 10000 * GET_CON (ch) * UPDATE_PULSE / (12 * 16);

    gain = move_rate;

    switch (GET_POS (ch))
    {
    case POSITION_SLEEPING:
        gain += gain >> 1;
        break;
    case POSITION_RESTING:
        gain += gain >> 2;
        break;
    case POSITION_SITTING:
        gain += gain >> 3;
        break;
    }

    // If you're hungry, thirsy, or got a busted rib, very slow movement regen.

    if (!ch->thirst || get_soma_affect(ch, SOMA_BLUNT_SEVBODY))
        gain >>= 1;

    if (get_affect (ch, MAGIC_STARVE_FOUR))
        gain >>=3;
    else if (get_affect (ch, MAGIC_STARVE_THREE))
        gain >>=2;
    else if (get_affect (ch, MAGIC_STARVE_TWO))
        gain >>=2;
    else if (get_affect (ch, MAGIC_STARVE_ONE))
        gain >>=1;

    if (ch->move_points < 0)
        ch->move_points = 0;

    ch->move_points += gain;

    moves_gained = ch->move_points / 10000;

    ch->move_points -= moves_gained * 10000;

    if (IS_SET (ch->room->room_flags, OOC) && !IS_SET (ch->flags, FLAG_GUEST))
        moves_gained = 0;

    if (moves_gained <= 0)
        moves_gained = 1;

    return moves_gained;
}

int
sleep_needed_in_seconds (CHAR_DATA * ch)
{
    if (!ch->pc)
        return 0;

    return ch->pc->sleep_needed / 100000;
}

void
sleep_credit (CHAR_DATA * ch)
{
    /* We're aiming for straight credit for sleeping, which will
     be 5 seconds per update if UPDATE_PULSE is 5 * 4 (5 seconds) */

    if (!ch->pc || !ch->descr())
        return;

    ch->pc->sleep_needed -= (UPDATE_PULSE / 4) * 100000;

    if (ch->pc->sleep_needed < 0)
        ch->pc->sleep_needed = 0;
}

void
sleep_need (CHAR_DATA * ch)
{
    int need;

    if (!ch->pc)
        return;
    else				/* To enable sleep, get rid of this next return */
        return;

    /* 5 * 60 is RL seconds (5 RL minutes sleep need per mud day) */
    /* sleep needed is called (60 * 60 * 24) / (UPDATE_PULSE/4)
     times a mud day. */
    /* 10000 is stuck in for granularity */

    /* If sleep needed is too long, assume that staff has set it
     this way. */

    if (ch->pc->sleep_needed > 10 * 60 * 100000)
        return;

    need = 5 * 60 * 100000 * (UPDATE_PULSE / 4) / (60 * 60 * 24);

    ch->pc->sleep_needed += need;

    if (ch->pc->sleep_needed > 10 * 60 * 100000)
        ch->pc->sleep_needed = 10 * 60 * 100000;
}

void
check_linkdead (void)
{
    CHAR_DATA *tch;

    for (tch = character_list; tch; tch = tch->next)
    {
        if (tch->deleted)
            continue;
        if (IS_NPC (tch) || tch->descr() || !IS_MORTAL (tch) || !tch->room)
            continue;
        if (tch->pc->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time)
        {
            do_quit (tch, "", 3);
        }
    }
}

void
check_idling (DESCRIPTOR_DATA * d)
{
    /* Unmark people who aren't really idle */

    if (d->idle && d->time_last_activity + PLAYER_IDLE_SECS > mud_time)
    {
        d->idle = 0;
        return;
    }

    if (d->original)
        return;

    if (!d->character
            && d->time_last_activity + DESCRIPTOR_DISCONNECT_SECS < mud_time
            && (d->connected <= CON_ACCOUNT_MENU
                || d->connected == CON_PENDING_DISC))
    {
        close_socket (d);
        return;
    }

    /* Disconnect those people idle for too long */

    if (d->idle && d->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time)
    {

        if (d->character
                && (IS_NPC (d->character) || !IS_MORTAL (d->character)))
            return;

        if (d->original)
            return;

        /* Idle PCs in the chargen process */

        if (d->character && !d->character->room)
        {
            close_socket (d);
        }

        if (d->character && d->connected && d->character->room)
            do_quit (d->character, "", 3);

        return;
    }

    /* Warn people who are just getting to be idle */

    if (!d->idle && d->time_last_activity + PLAYER_IDLE_SECS < mud_time)
    {
        if (d->character)
        {
            if (d->connected == CON_PLYNG)
                SEND_TO_Q ("Your thoughts begin to drift. #2(Idle)#0\n\r", d);
            else
                SEND_TO_Q
                ("\n\rYour attention is required to prevent disconnection from the server. #2(Idle)#0\n\r",
                 d);
        }

        d->idle = 1;
		return;
    }

	// Beat the silly server stuff.
	if (d->idle && d->time_last_activity + (PLAYER_IDLE_BEEP_SECS * (d->idle + 1)) < mud_time)
	{
        if (d->character)
        {
            if (d->connected == CON_PLYNG)
			{
				d->idle ++;
                SEND_TO_Q ("Your thoughts continue to drift. #2(Idle)#0\n\r", d);
				return;
			}
		}
	}
}

void
check_idlers ()
{
    DESCRIPTOR_DATA *d, *d_next;

    for (d = descriptor_list; d; d = d_next)
    {
        d_next = d->next;
        check_idling (d);
    }
}

/* Update both PC's & NPC's and objects*/
void
point_update (void)
{
    int cycle_count;
    int roll, i;
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char buf2[MAX_STRING_LENGTH] = {'\0'};
    CHAR_DATA *ch;
    CHAR_DATA *tch;
    CHAR_DATA *next_ch = NULL;
    ROOM_DATA *room;
    AFFECTED_TYPE *af;
    WOUND_DATA *wound, *next_wound, *check_wound;
    struct time_info_data healing_time;
    struct time_info_data bled_time;
    struct time_info_data temp_bled_time;
    struct time_info_data playing_time;
    int blood_found = 0;

    static int reduceIntox = 0;

    cycle_count = 0;

    mud_time = time (NULL);
    mud_time_str = (char *) asctime (localtime (&mud_time));
    *(mud_time_str + strlen (mud_time_str) - 1) = '\0';

    for (i = 0; i <= 99; i++)
        zone_table[i].player_in_zone = 0;

    for (ch = character_list; ch; ch = next_ch)
    {

        if (!ch)
            continue;

        next_ch = ch->next;

        if (ch->deleted)
            continue;

        if (!ch->room)
            continue;

        if (!IS_NPC (ch) && ch->room)
            zone_table[ch->room->zone].player_in_zone++;

        room = ch->room;

        *ch->short_descr = tolower (*ch->short_descr);

        //boats take damage
        if (IS_SET (ch->act, ACT_VEHICLE))
        {
            if (room->sector_type == SECT_REEF)
            {

                for (tch = character_list; tch; tch = tch->next)
                {

                    if (tch->deleted)
                        continue;

                    if (tch->vehicle == ch)
                        send_to_char
                        ("The boat shudders and you hear the sides of the "
                         "ship scrape against\n\r" "something.\n\r", tch);
                }

                send_to_char ("Ouch!  You're scraping against the reef!\n\r",
                              ch);

                if (weaken (ch, 10, 0, "boat in REEF"))	/* 10 hits for scraping the reef */
                    continue;
            }
        }

        //Sleep
        if (GET_POS (ch) >= SLEEP)
        {

            if (ch->pc)
            {

                if ((af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)))
                {

                    if (GET_POS (ch) == STAND)
                    {
                        send_to_char ("You fall down.\n", ch);
                        act ("$n falls down.", false, ch, 0, 0, TO_ROOM);
                    }

                    else if (GET_POS (ch) == SIT)
                    {
                        send_to_char ("You collapse.\n", ch);
                        act ("$n collapses.", false, ch, 0, 0, TO_ROOM);
                    }

                    GET_POS (ch) = REST;
                }

                if ((GET_POS (ch) == REST || GET_POS (ch) == SIT) &&
                        sleep_needed_in_seconds (ch) > 7 * 60 &&
                        IS_MORTAL (ch) && !number (0, 8) &&
                        !get_second_affect (ch, SA_STAND, NULL))
                {
                    act ("$n falls alseep.", false, ch, 0, 0, TO_ROOM);
                    GET_POS (ch) = SLEEP;
                }

                if (sleep_needed_in_seconds (ch) == 300 && IS_MORTAL (ch))
                {
                    send_to_char ("You are feeling drowsy.\n", ch);
                    /* Adding this is 1 sec.  If we don't do this, the
                     drowsy message appears every 5 seconds. */
                    ch->pc->sleep_needed += 100000;
                }

                else if (sleep_needed_in_seconds (ch) == 350 && IS_MORTAL (ch))
                {
                    send_to_char ("You are feeling sleepy.\n", ch);
                    ch->pc->sleep_needed += 100000;
                }

                else if (sleep_needed_in_seconds (ch) == 400 && IS_MORTAL (ch))
                {
                    send_to_char ("You are about to fall asleep.\n", ch);
                    ch->pc->sleep_needed += 100000;
                }

                else if (GET_POS (ch) >= REST && GET_POS (ch) != FIGHT &&
                         sleep_needed_in_seconds (ch) > 6 * 60 &&
                         number (0, 25) == 25 &&
                         IS_MORTAL (ch) && ch->descr() && ch->descr()->original == ch)
                {
                    if (number (0, 1))
                        act ("You stifle a yawn.", false, ch, 0, 0, TO_CHAR);
                    else
                    {

                        if (get_affect (ch, MAGIC_HIDDEN) && would_reveal (ch))
                        {
                            remove_affect_type (ch, MAGIC_HIDDEN);
                            act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM);
                        }

                        act ("You yawn.", false, ch, 0, 0, TO_CHAR);
                        act ("$n yawns.", false, ch, 0, 0, TO_ROOM);
                    }
                }

                if (GET_POS (ch) == SLEEP)
                    sleep_credit (ch);
                else if (IS_MORTAL (ch))
                    sleep_need (ch);
            }

            if (GET_POS (ch) == POSITION_SLEEPING && ch->descr() &&
                    ch->pc && ch->pc->dreams && !number (0, 5))
                dream (ch);

            if (GET_MOVE (ch) < GET_MAX_MOVE (ch))
            {
                GET_MOVE (ch) =
                    MIN (GET_MOVE (ch) + move_gain (ch), GET_MAX_MOVE (ch));
            }

            if (GET_POS(ch) != POSITION_FIGHTING && GET_SHOCK (ch) < GET_MAX_SHOCK (ch))
            {
                GET_SHOCK (ch) =
                    MIN (GET_SHOCK (ch) + shock_gain (ch), GET_MAX_SHOCK (ch));
            }
        }

        // Healing
        if (!ch)
            continue;

        int old_damage = ch->damage;
        int new_damage = 0;
        if (ch->damage)
        {

            int base = (ch->skills[SKILL_VOODOO]) ? BASE_SPECIAL_HEALING :
                       (get_affect (ch, MAGIC_STARVE_TWO) || get_affect (ch, MAGIC_STARVE_THREE) || get_affect (ch, MAGIC_STARVE_FOUR)) ?
                       BASE_STARVE_HEALING : BASE_PC_HEALING;

            healing_time = real_time_passed (time (0) - ch->lastregen, 0);
            if (healing_time.minute >= (base - ch->con / 5))
            {
                roll = dice (1, 100);
                if (GET_POS (ch) == POSITION_SLEEPING)
                    roll -= 20;
                if (GET_POS (ch) == POSITION_RESTING)
                    roll -= 10;
                if (GET_POS (ch) == POSITION_SITTING)
                    roll -= 5;
                if (roll <= (ch->con * 4))
                {
                    if (roll % 5 == 0)
                        ch->damage -= 2;
                    else
                        ch->damage -= 1;
                }
                ch->lastregen = time (0);
            }

        }

        // Stun recovery
        if (!ch)
            continue;

        if (GET_POS (ch) == POSITION_STUNNED)
        {
            if ((time (0) - ch->laststuncheck) >= number (15, 20))
            {
                ch->laststuncheck = time (0);
                GET_POS (ch) = REST;
                send_to_char
                ("You shake your head vigorously, recovering from your stun.\n",
                 ch);
                sprintf (buf,
                         "$n shakes $s head vigorously, seeming to recover.");
                act (buf, false, ch, 0, 0, TO_ROOM);
                if (IS_NPC (ch))
                    do_stand (ch, "", 0);
            }
        }

        //regain consciousness
        if (!ch)
            continue;

        if (GET_POS (ch) == POSITION_UNCONSCIOUS)
        {
            healing_time = real_time_passed (time (0) - ch->knockedout, 0);
            if (healing_time.minute >= 5)
            {
                GET_POS (ch) = REST;
                send_to_char ("Groaning groggily, you regain consciousness.\n",
                              ch);
                sprintf (buf, "Groaning groggily, $n regains consciousness.");
                act (buf, false, ch, 0, 0, TO_ROOM);
                if (IS_NPC (ch))
                    do_stand (ch, "", 0);
            }
        }

        //intoxication
        if (!ch)
            continue;

        if (reduceIntox == 3)
        {
            if (ch->intoxication > 0)
                if (!--ch->intoxication)
                    ;
            /*send_to_char ("You are sober.\n", ch); */
            reduceIntox = 0;

        }
        else
            reduceIntox++;

        //Application RPP cost
        if (!ch)
            continue;

        if (!IS_NPC (ch) && ch->pc->app_cost && ch->descr())
        {
            playing_time =
                real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
            if (playing_time.hour >= 10 && ch->descr()->acct)
            {

                ch->descr()->acct->pay_application_cost (ch->pc->app_cost);
                ch->pc->app_cost = 0;
                save_char (ch, true);
            }
        }

        //Remove New Player Flag
        if (!IS_NPC (ch) && IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
        {
            playing_time =
                real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
            if (playing_time.hour > 12)
            {
                ch->plr_flags &= ~NEW_PLAYER_TAG;
                act
                ("You've been playing for over 12 hours, now; the #2(new player)#0 tag on your long description has been removed. Once again, welcome - have fun, and best of luck in your travels!\n",
                 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            }
        }

        // Bleeding
        if (!ch)
            continue;

        *buf = '\0';
        *buf2 = '\0';

        blood_found = 5;

        for (wound = ch->wounds; wound; wound = next_wound)
        {
            blood_found = 0;

            if (!ch->wounds)
                break;   // they aren't really wounded

            next_wound = wound->next;

            //if (!wound->gunshot && ch->delay_type == DEL_BIND_WOUNDS)
            //  continue;

            int wound_damage = wound->damage;
            old_damage += wound_damage;
            new_damage += wound_damage;
            healing_time = real_time_passed (time (0) - wound->lasthealed, 0);
            bled_time = real_time_passed (time (0) - wound->lastbled, 0);

			// We'll make wildlife bleed out a heck of a lot faster
			// so it's much more useful to shoot them.
			int bleeding_interval = BLEEDING_INTERVAL;
			if (IS_NPC(ch) && IS_SET(ch->act, ACT_WILDLIFE))
				bleeding_interval = BLEEDING_WILDLIFE_INTERVAL;

            if ((IS_MORTAL (ch) || IS_NPC(ch)) && bled_time.minute >= bleeding_interval && wound->bleeding)
            {
                wound->lastbled = time (0);

                if (!*buf)
                {
                    sprintf(buf, "#1Blood ");
                    sprintf(buf2, "Blood ");
                    blood_found = 2;
                }

                for (check_wound = wound; check_wound; check_wound = check_wound->next)
                {
                    temp_bled_time = real_time_passed (time (0) - check_wound->lastbled, 0);
                    if (temp_bled_time.minute >= bleeding_interval && check_wound->bleeding)
                    {
                        if (blood_found >= 2)
                            blood_found = 3;
                        else
                            blood_found = 1;
                        break;
                    }
                }


                // We need to keep blood_found == 2 here so we get
                // the right short descriptor in place.
                if (blood_found >= 2)
                {
                    ;
                }
                else if (blood_found)
                {
                    sprintf(buf + strlen(buf), ", ");
                    sprintf(buf2 + strlen(buf2), ", ");
                }
                else
                {
                    sprintf(buf + strlen(buf), " and ");
                    sprintf(buf2 + strlen(buf2), " and ");
                }

                if (wound->bleeding > 0 && wound->bleeding <= 5)
                {
                    sprintf (buf + strlen(buf), "#0flows#1 from a %s %s on your %s",
                             wound->severity, wound->name, expand_wound_loc (wound->location));

                    sprintf (buf2 + strlen(buf2), "flows from a %s %s on #5%s#0%s %s",
                             wound->severity, wound->name, (blood_found >= 2 ? char_short (ch) : HSHR(ch)), (blood_found == 2 ? "'s" : ""), expand_wound_loc (wound->location));
                }
                else if (wound->bleeding > 5 && wound->bleeding <= 10)
                {
                    sprintf (buf + strlen(buf), "#0hemorrhages#1 from a %s %s on your %s",
                             wound->severity, wound->name, expand_wound_loc (wound->location));

                    sprintf (buf2 + strlen(buf2), "hemorrhages from a %s %s on #5%s#0%s %s",
                             wound->severity, wound->name, (blood_found >= 2 ? char_short (ch) : HSHR(ch)), (blood_found == 2 ? "'s" : ""), expand_wound_loc (wound->location));
                }
                else if (wound->bleeding > 10 && wound->bleeding <= 15)
                {
                    sprintf (buf + strlen(buf), "#0spurts#1 from a %s %s on your %s",
                             wound->severity, wound->name, expand_wound_loc (wound->location));

                    sprintf (buf2 + strlen(buf2), "spurts from a %s %s on #5%s#0%s %s",
                             wound->severity, wound->name, (blood_found >= 2 ? char_short (ch) : HSHR(ch)), (blood_found == 2 ? "'s" : ""), expand_wound_loc (wound->location));
                }
                else if (wound->bleeding > 15)
                {
                    sprintf (buf + strlen(buf), "#0gushes#1 from a %s %s on your %s",
                             wound->severity, wound->name, expand_wound_loc (wound->location));

                    sprintf (buf2 + strlen(buf2), "gushes from a %s %s on #5%s#0%s %s",
                             wound->severity, wound->name, (blood_found >= 2 ? char_short (ch) : HSHR(ch)), (blood_found == 2 ? "'s" : ""), expand_wound_loc (wound->location));
                }

                // Now, if blood_found equals 2, and not 3, then this is our only bleeding wound, so we
                // need to echo it out to everyone.
                if (blood_found == 2)
                {
                    blood_found = 0;
                }

                if (blood_found == 0)
                {
                    if (get_affect(ch, AFFECT_INTERNAL))
                    {
                        strcat(buf, ", and your insides wrench and heave, spreading agony throughout your body");
                    }

                    strcat(buf, ".#0");
                    strcat(buf2, ".");

                    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT); // You want to see your bleed.
                    act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT | _ACT_BLEED);

                    if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
                        act ("#6To stop the bleeding before it's too late, type BIND.#0", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                }

                object__enviro(ch, NULL, COND_BLOOD, wound->bleeding, woundloc_to_hitloc(wound->location));
                add_scent(ch, scent_lookup("the metallic tang of blood"), 1, wound->bleeding, 200, wound->bleeding, 0);

                if (general_damage (ch, wound->bleeding))
                    continue;
            }

            int base = (ch->skills[SKILL_VOODOO]) ? BASE_SPECIAL_HEALING :
                       (get_affect (ch, MAGIC_STARVE_TWO) || get_affect (ch, MAGIC_STARVE_THREE) || get_affect (ch, MAGIC_STARVE_FOUR)) ?
                       BASE_STARVE_HEALING : BASE_PC_HEALING;

            if (healing_time.minute >= (base + (wound->damage / 3)
                                        - (GET_CON (ch) / 7) - (wound->healerskill / 20)))
            {
                wound->lasthealed = time (0);
                if (GET_POS (ch) != POSITION_FIGHTING
                        && !IS_SET (ch->room->room_flags, OOC))
                {
                    new_damage -= wound_damage;

                    // returns false if wound still exists
                    if (!natural_healing_check (ch, wound))
                    {
                        new_damage += wound->damage;
                    }
                }
            }
        }

        new_damage += ch->damage; // we add this later, since bloodloss can add
        // NPCs that healed trigger on_health (>NN)
        if (IS_NPC(ch) && new_damage < old_damage && ch->mob->cues)
        {
            // hook: cue_on_health reflex
            // We only want ones that have the flag (>NN)
            typedef std::multimap<mob_cue,std::string>::const_iterator N;
            std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_health);
            if (range.first != range.second)
            {
                // Want to only execute the health cue the first time health
                // drops below NN
                int old_health = 100 - (int((old_damage * 100.0) / (ch->max_hit)));
                int new_health = 100 - (int((new_damage * 100.0) / (ch->max_hit)));

                for (N n = range.first; n != range.second; n++)
                {
                    std::string cue = n->second;
                    const char *r = cue.c_str();

                    if (!cue.empty () && strncmp(r,"(>",2) == 0)
                    {
                        char *p;
                        int threshold = strtol (r+2, &p, 0);
                        if (new_health > threshold && old_health <= threshold)
                        {
                            char reflex[AVG_STRING_LENGTH] = "";
                            strcpy (reflex, p+2);
                            command_interpreter (ch, reflex);
                        }
                    }
                }
            }
        }

        if ((af = get_affect (ch, MAGIC_CRAFT_DELAY)))
        {
            if (time (0) >= af->a.spell.modifier)
            {
                send_to_char
                ("#6OOC: Your craft delay timer has expired. You may resume crafting delayed items.#0\n",
                 ch);
                remove_affect_type (ch, MAGIC_CRAFT_DELAY);
            }
        }

        if ((af = get_affect (ch, AFFECT_UPGRADE_DELAY)))
        {
            if (time (0) >= af->a.spell.modifier)
            {
                send_to_char
                ("#6OOC: Your upgrade delay timer has experied. You may now used the upgrade command again.#0\n",
                 ch);
                remove_affect_type (ch, AFFECT_UPGRADE_DELAY);
            }
        }

        if (ch->damage < 0)
            ch->damage = 0;

        if (ch->room && sun_light && IS_SET (ch->affected_by, AFF_SUNLIGHT_PET)
                && !is_overcast (ch->room))
        {
            ch->plr_flags |= FLAG_PETRIFIED;
            die (ch);
        }

    }				/* for */
}

int
create_monster (OBJ_DATA *obj, AFFECTED_TYPE *af)
{
    char buf[AVG_STRING_LENGTH] = {'\0'};
    CHAR_DATA *mob = NULL;
    int room = 666;
    OBJ_DATA *test_obj = NULL;
    OBJ_DATA *real_obj = NULL;

    if (obj->nVirtual != VNUM_CORPSE)
    {
        send_to_gods("Non-corpse object trying to turn in to monster - please wipe object.");
        return 0;
    }

    if (vnum_to_room(obj->in_room))
        room = obj->in_room;
    else if (obj->carried_by && !obj->in_obj && vnum_to_room(obj->carried_by->in_room))
        room = obj->carried_by->in_room;
    else if (obj->in_obj && !obj->carried_by)
    {
        test_obj = obj->in_obj;
        // Given that you could put a virtually limitless amount of containers, this should find the last nest.
        while (test_obj)
        {
            real_obj = test_obj;
            test_obj = test_obj->in_obj;
        }

        if (vnum_to_room(real_obj->in_room))
            room = real_obj->in_room;
        else if (real_obj->carried_by && vnum_to_room(real_obj->carried_by->in_room))
            room = real_obj->carried_by->in_room;
    }

    if (room <= 0)
        return 0;

    if (!(mob = load_mobile (54025)))
        return 0;

    randomize_mobile(mob);

    sprintf (buf, "figure tentacle-ridden %s", obj->var_color[0]);
    if (mob->name)
        mem_free (mob->name);
    mob->name = add_hash (buf);

    if (mob->tname)
        mem_free (mob->tname);
    mob->tname = add_hash ("figure");

    if (IS_SET (obj->o.container.flags, CONT_BEHEADED))
        sprintf (buf, "The tentacle-ridden, headless figure of %s shambles back and forth.", obj->var_color[0]);
    else
        sprintf (buf, "The tentacle-ridden figure of %s shambles back and forth.", obj->var_color[0]);

    if (mob->long_descr)
        mem_free (mob->long_descr);
    mob->long_descr = add_hash (buf);

    if (IS_SET (obj->o.container.flags, CONT_BEHEADED))
        sprintf (buf, "the headless, shambling figure of %s", obj->var_color[0]);
    else
        sprintf (buf, "the shambling figure of %s", obj->var_color[0]);

    if (mob->short_descr)
        mem_free (mob->short_descr);
    mob->short_descr = str_dup (buf);

    if (mob->description)
        mem_free (mob->description);
    mob->description = str_dup ("This creature has the underlying skeleton of what might've once "
                                "been a human. It has an incongruent rendering of a vaguely humanoid "                                "shape, its body sunken in some regions and swollen in others with "
                                "hard growths. The whole of it appears rotting and utterly changed, "
                                "with spine-covered tentacles, curling and twisting over each other "
                                "in a mass. The creature has a horrible misshapen mouth, forever " 
                                "held at an unnatural yawning width, only closing in search of "
                                "something to bite and rip into."); 

    reformat_desc(mob->description, &mob->description);

    if (IS_SET (obj->o.container.flags, CONT_BEHEADED))
        soma_add_affect(mob, SOMA_NO_HEAD, -1, 0, 0, 250, 250, 250, 666, 666, 666, 666);

    if (vnum_to_room(room))
    {
        char_to_room (mob, room);
        mob->room = vnum_to_room(room);
    }
    else
    {
        send_to_gods("WARNING: tried to load monster but couldn't find room.");
    }

    act("With a horrible, awful crackling, $p begins to tremble and shake, eventually rising as $n.", false, mob, obj, 0, TO_ROOM | _ACT_FORMAT);
    mob->act |= ACT_STAYPUT;

    extract_obj (obj);
    return 1;
}



void
hourly_update (void)
{
    int current_time;
    int hours;
    int nomsg = 0;
    CHAR_DATA *ch, *next_ch;
    OBJ_DATA *obj;
    OBJ_DATA *objj;
    OBJ_DATA *next_thing2;
    ROOM_DATA *room;
    NEGOTIATION_DATA *neg;
    NEGOTIATION_DATA *new_list = NULL;
    NEGOTIATION_DATA *tmp_neg;
    char your_buf[MAX_STRING_LENGTH];
    char room_buf[MAX_STRING_LENGTH];
    char room_msg_buf[MAX_STRING_LENGTH];
    AFFECTED_TYPE *af = NULL;

    current_time = time (NULL);
    minute_update_count = 0;

    for (ch = character_list; ch;)
    {

        next_ch = ch->next;

        if (ch->deleted || !ch->room)
            continue;

        /*
         for (ench = ch->enchantments; ench; ench = ench->next)
         {
           ench->current_hours--;
           if (ench->current_hours <= 0)
        	 remove_enchantment (ch, ench);
         }
         */

        if (IS_NPC (ch))
        {
            // Delete Old Barters
            if (IS_SET (ch->flags, FLAG_KEEPER) && ch->shop)
            {
                neg = ch->shop->negotiations;
                new_list = NULL;

                while (neg)
                {
                    tmp_neg = neg->next;

                    if (neg->time_when_forgotten <= current_time)
                    {
                        mem_free (neg);
                    }
                    else
                    {
                        neg->next = new_list;
                        new_list = neg;
                    }
                    neg = tmp_neg;
                }
                ch->shop->negotiations = new_list;
            }

            // hook: cue_on_hour reflex
            if (ch->mob->cues)
            {
                typedef std::multimap<mob_cue,std::string>::const_iterator N;
                std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_hour);
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

            // Mighty Morphing
            if ((ch->morph_time) && ch->morph_time < current_time)
            {
                morph_mob (ch);
            }
        }
        else
        {
            if (!IS_SET (ch->room->room_flags, OOC))
                hunger_thirst_process (ch);
        }

        trigger (ch, "", TRIG_HOUR);

        ch = next_ch;
    }

    for (obj = object_list; obj; obj = obj->next)
    {

        if (obj->deleted)
            continue;

        /*
         if(obj->poison)
         {
           for (poison = obj->poison; poison; poison = poison->next)
           {
        	 if(poison->uses < 1)
        	   remove_object_poison(obj, obj->poison);
        	 else
        	   poison->uses--;
           }
         }
         */

        /* Mob jailbags need to disappear after a time */


        if ((af = get_obj_affect(obj, MAGIC_MONSTER)))
        {
            if (af->a.monster.disease >= 1000)
            {
                if (create_monster(obj, af))
                    continue;
            }
            else
                af->a.monster.disease += 100;
        }


        if (obj->ocues)
        {
            typedef std::multimap<obj_cue,std::string>::const_iterator N;
            std::pair<N,N> range = obj->ocues->equal_range (ocue_on_hour);
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

        if ((obj->nVirtual == VNUM_CORPSE ||
             obj->nVirtual == VNUM_HEAD ||
             obj->nVirtual == VNUM_LOWERTORSO ||
             obj->nVirtual == VNUM_UPPERTORSO ||
             obj->nVirtual == VNUM_WHOLEARM ||
             obj->nVirtual == VNUM_WHOLELEG))
        {
            SCENT_DATA *scent = NULL;
            for (scent = obj->scent; scent; scent = scent->next)
            {
                if (scent->scent_ref == scent_lookup("the putrid rot of decaying flesh"))
                {
                    scent->atm_power += 20;
                    scent->pot_power += 20;
                    scent->rat_power += 1;

                    scent->atm_power = MIN(scent->atm_power, 1000);
                    scent->pot_power = MIN(scent->pot_power, 1000);
                    scent->rat_power = MIN(scent->rat_power, 100);
                }
            }
        }

        if (obj->nVirtual == VNUM_JAILBAG &&
                obj->obj_timer && --obj->obj_timer == 0)
            extract_obj (obj);


        else if (IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER)
                 && --obj->obj_timer <= 0)
        {
            room = (obj->in_room == NOWHERE ? NULL : vnum_to_room (obj->in_room));

            if (obj->carried_by)
                act ("$p decays in your hands.",
                     false, obj->carried_by, obj, 0, TO_CHAR);

            else if (room && room->people)
            {
                act ("$p gradually decays away.",
                     true, room->people, obj, 0, TO_ROOM);
                act ("$p gradually decays away.",
                     true, room->people, obj, 0, TO_CHAR);
            }

            for (objj = obj->contains; objj; objj = next_thing2)
            {

                next_thing2 = objj->next_content;	/* Next in inventory */

                if (obj->nVirtual == VNUM_CORPSE)
                {
                    objj->obj_flags.extra_flags |= ITEM_TIMER;
                    objj->obj_timer = 12;	/* Stuff from corpses lasts 1 */
                    /* 3 RL hours, unless handled by a PC. */
                }

                obj_from_obj (&objj, 0);

                if (obj->in_obj)
                    obj_to_obj (objj, obj->in_obj);
                else if (obj->carried_by)
                    obj_to_room (objj, obj->carried_by->in_room);
                else if (obj->in_room != NOWHERE)
                    obj_to_room (objj, obj->in_room);
                else
                    extract_obj (obj);
            }

            //if (!room)
            //{
            //    std::string toGods;
            //    toGods = MAKE_STRING(obj->short_description) + " has been extracted, but it had no room.";
            //    send_to_gods(toGods.c_str());
            //}

            extract_obj (obj);
        }

        else if ((GET_ITEM_TYPE (obj) == ITEM_LIGHT ||
                  GET_ITEM_TYPE (obj) == ITEM_CHEM_SMOKE) &&
                 obj->o.light.on && obj->o.light.hours)
        {

            obj->o.light.hours--;

            hours = obj->o.light.hours;

            if (!(ch = obj->carried_by))
                ch = obj->equiped_by;

            switch (hours)
            {
            case 0:
                strcpy (your_buf, "Your $o burns out.");
                strcpy (room_buf, "$n's $o burns out.");
                strcpy (room_msg_buf, "$p burns out.");
                break;

            case 1:
                strcpy (your_buf, "Your $o is just a dim flicker now.");
                strcpy (room_buf, "$n's $o is just a dim flicker now.");
                strcpy (room_msg_buf, "$p is just a dim flicker now.");
                break;

            case 2:
                strcpy (your_buf, "Your $o begins to burn low.");
                strcpy (room_buf, "$n's $o begins to burn low.");
                strcpy (room_msg_buf, "$p begins to burn low.");
                break;

            case 10:
                strcpy (your_buf, "Your $o sputters.");
                strcpy (room_buf, "$n's $o sputters.");
                strcpy (room_msg_buf, "$p sputters.");
                break;

            default:
                nomsg = 1;
                break;
            }

            if (hours == 0 ||
                    (!is_name_in_list ("candle", obj->name) && !nomsg))
            {

                if (ch)
                {
                    act (your_buf, false, ch, obj, 0, TO_CHAR);
                    act (room_buf, false, ch, obj, 0, TO_ROOM);
                }

                if (obj->in_room &&
                        (room = vnum_to_room (obj->in_room)) && room->people)
                {
                    act (room_msg_buf, false, room->people, obj, 0, TO_ROOM);
                    act (room_msg_buf, false, room->people, obj, 0, TO_CHAR);
                }
            }

            if (obj->o.light.hours > 0)
                continue;

            if (is_name_in_list ("candle", obj->name))
                extract_obj (obj);
            else if (GET_ITEM_TYPE(obj) == ITEM_CHEM_SMOKE && vtoo(obj->o.smoke.morphto))
            {
                OBJ_DATA *tobj = NULL;
                OBJ_DATA *robj = NULL;
                CHAR_DATA *tch = NULL;

                if ((tobj = load_object(obj->o.smoke.morphto)))
                {

                    if ((tch = obj->carried_by))
                    {
                        extract_obj (obj);
                        obj_to_char(tobj, tch);
                    }
                    else if ((robj = obj->in_obj))
                    {
                        extract_obj (obj);
                        obj_to_obj(tobj, robj);
                    }
                    else if ((tch = obj->equiped_by))
                    {
                        extract_obj (obj);
                        obj_to_char(tobj, tch);
                    }
                    else if (obj->in_room)
                    {
                        obj_to_room(tobj, obj->in_room);
                        extract_obj(obj);
                    }
                    else
                    {
                        extract_obj(obj);
                        extract_obj(tobj);
                    }
                }
            }
        }

        if ((obj->morphTime) && obj->morphTime < current_time)
            morph_obj (obj);

        if (IS_ELECTRIC(obj))
            electronic_hourly(obj);

        // Every 4 hour that we've go t water on us,
        // add a point of rust.
        if (obj->enviro_conds[COND_WATER] > 10)
        {
            object__add_enviro(obj, COND_RUST, 1);
        }
        // Every 48 game hours, things you are wearing
        // gain a level of filth -- nice and random.
        if (obj->equiped_by && !IS_NPC(obj->equiped_by) && !number(0,47))
        {
            object__add_enviro(obj, COND_FILTH, 1);
        }

    }
}

int
remove_room_affect (ROOM_DATA * room, int type)
{
    AFFECTED_TYPE *af;
    AFFECTED_TYPE *free_af;

    if (!room->affects)
        return 0;

    if (room->affects->type == type)
    {
        free_af = room->affects;
        room->affects = free_af->next;
        mem_free (free_af);
        return 1;
    }

    for (af = room->affects; af->next; af = af->next)
        if (af->next->type == type)
        {
            free_af = af->next;
            af->next = free_af->next;
            mem_free (free_af);
            return 1;
        }

    return 0;
}

void
room_affect_wearoff (ROOM_DATA * room, int type)
{
    if (!remove_room_affect (room, type))
        return;

    switch (type)
    {
    case MAGIC_ROOM_CALM:
        if (room)
            send_to_room
            ("Slowly, the sense of peace dissipates, and things return to normal.\n",
             room->vnum);
        else
            send_to_all ("The sense of peace everwhere fades.\n\r");
        break;

    case MAGIC_ROOM_LIGHT:
        if (room)
            send_to_room ("The unnatural light emanations fade.\n\r",
                          room->vnum);
        else
            send_to_all
            ("The unnatural light emanations fade from the land.\n\r");
        break;

    case MAGIC_ROOM_DARK:
        if (room)
            send_to_room ("The unnatural darkness fades away.\n\r",
                          room->vnum);
        else
            send_to_all ("The unnatural darkness fades from the land.\n\r");
        break;

    case MAGIC_WORLD_SOLAR_FLARE:
        if (room)
            send_to_room ("The localized solar flare has ended.\n\r",
                          room->vnum);
        else
            send_outside ("The ball of flame in the sky slowly dies out.\n\r");
        break;
    }
}

void
room_update (void)
{
    AFFECTED_TYPE *room_affect = NULL;
    ROOM_DATA *room = NULL;

    /* Expire affects on rooms */
    for (room = full_room_list; room; room = room->lnext )
    {
        for ( room_affect = room->affects; room_affect; room_affect = room_affect->next)
        {
            if ( room_affect->type == MAGIC_ROOM_FIGHT_NOISE )
                continue;

            if ( room_affect->a.room.duration > 0 )
                room_affect->a.room.duration--;

            if ( !room_affect->a.room.duration )
                room_affect_wearoff (room, room_affect->type);
        }
    }
}

/* returns the situation-affected skill level */
int
skill_level (CHAR_DATA * ch, int skill, int diff_mod)
{
    int skill_lev;
    OBJ_DATA *obj;
    AFFECTED_TYPE *af;

    // Handy code provided by case to prevent OFFENSE causing problems.
    if (skill >= 0)
    {
        if (!(ch->skills[skill]))
        {
            return 2;
        }
        skill_lev = ch->skills[skill];
    }
    else if (skill == SKILL_OFFENSE)
    {
        skill_lev = ch->offense;
    }
    else
    {
        throw std::runtime_error("BAD SKILL VALUE PASSED TO skill_level()");
    }

    skill_lev -= diff_mod;

    if (ch->stun > 0)
        skill_lev -= ch->stun * 2;

    if ((af = get_affect (ch, MAGIC_AFFECT_CURSE)))
        skill_lev -= af->a.spell.modifier;

    if (get_affect(ch, MAGIC_STARVE_FOUR))
        skill_lev -= 35;
    else if (get_affect(ch, MAGIC_STARVE_THREE))
        skill_lev -= 25;
    else if (get_affect(ch, MAGIC_STARVE_TWO))
        skill_lev -= 15;
    else if (get_affect(ch, MAGIC_STARVE_ONE))
        skill_lev -= 5;

    for (obj = ch->equip; obj; obj = obj->next_content)
    {
		// Only one boost per EQ.
		bool boosted = false;

        /* Why skip weapons?
         if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON || GET_ITEM_TYPE (obj) == ITEM_SHIELD )
         continue;
         */
        for (af = obj->xaffected; af; af = af->next)
        {
            if (af->a.spell.location - 10000 == skill)
                skill_lev += af->a.spell.modifier;
        }

        if (GET_ITEM_TYPE(obj) == ITEM_E_BOOST && obj->o.elecs.status != 0 && !boosted)
        {
            if (skill == obj->o.od.value[3])
			{
				boosted = true;
                skill_lev += obj->o.od.value[5];
			}

            if (skill == obj->o.od.value[4])
			{
				boosted = true;
                skill_lev += obj->o.od.value[5];
			}
        }
    }

    if ((obj = ch->right_hand))
    {
        for (af = obj->xaffected; af; af = af->next)
        {
            if (af->a.spell.location - 10000 == skill)
                skill_lev += af->a.spell.modifier;
        }
    }

    if ((obj = ch->left_hand))
    {
        for (af = obj->xaffected; af; af = af->next)
        {
            if (af->a.spell.location - 10000 == skill)
                skill_lev += af->a.spell.modifier;
        }
    }

    // If you're doing a research or engineering based skill, then your -minimum- is engineering|research / 5.

	int modEducation = 0;
	modEducation = ((ch->skills[SKILL_EDUCATION] - 10) / 5) + 1;
	modEducation = MAX(1, modEducation);

    switch (skill)
    {
		case SKILL_CHEMISTRY:
		case SKILL_MECHANICS:
		case SKILL_ELECTRONICS:
		case SKILL_COMPUTEROLOGY:
		case SKILL_BIOLOGY:
		case SKILL_MEDICINE:
	        skill_lev = MAX (modEducation, skill_lev);
			break;
		case SKILL_GUNSMITH:
		case SKILL_ARMORCRAFT:
		case SKILL_WEAPONCRAFT:
		case SKILL_HANDICRAFT:
	        skill_lev = MAX (modEducation / 2, skill_lev);
			break;
    }

    skill_lev = MAX (1, skill_lev);
    skill_lev = MIN (99, skill_lev);

    return skill_lev;
}

// This opens a character with 1 point in that skill, and a big delay.

void
skill_learn (CHAR_DATA *ch, int skill)
{
    if (IS_NPC(ch))
        return;
    if (real_skill(ch, skill))
        return;
    else
        ch->skills[skill] = 1;

    magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill, 200, 0, 0, 0, 0);
}


// Tht.is function is called to determine whether a player has gone over their skill cap or not.

int
skill_max (CHAR_DATA *ch, int skill, int mode)
{
    int i;
    int j= 0;
    int skillmax;

    if (ch->intel > 20)
        skillmax = 1100;
    else if (ch-> intel < 10)
        skillmax = 550;
    else
        skillmax = 550 + (40 * (ch->intel - 10));

    for (i = 0; i <= LAST_SKILL; i++)
    {
        // Skip any skill 10 or under, or their native tongue, or Sindarin, or Literacy, or Small Blade

        if (ch->skills[i] < 11
                || (i == atoi (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE)))
                || (skills[i] == skills[SKILL_COMMON])
                || (skills[i] == skills[SKILL_SMALL_BLADE]))
            ;
        else
            j += ch->skills[i];
    }

    if (mode == 0)
    {
        if ((j + skill) >= skillmax)
            return 1;
        else
            return 0;
    }
    else
    {
        return j;
    }
}

int combat_skill_use(CHAR_DATA *ch, int skill)
{
    int value;
    CHAR_DATA *tch = NULL;
    CHAR_DATA *rch = NULL;
    CHAR_DATA *sch = NULL;
    OBJ_DATA *obj1 = NULL;
    OBJ_DATA *obj2 = NULL;
    OBJ_DATA *tobj1 = NULL;
    OBJ_DATA *tobj2 = NULL;
    OBJ_DATA *robj1 = NULL;
    OBJ_DATA *robj2 = NULL;
    OBJ_DATA *sobj1 = NULL;
    OBJ_DATA *sobj2 = NULL;
    int damage = 0;
    int returned = 0;

    value = ch->skills[skill];

    if (!value)
        returned = 0;

    // Under 50 is always easy, and a positive
    if (value < 50)
        returned = 1;
        
	// Can't get above 60 if you're in the Arena.
	if (vnum_to_room(ch->in_room)->zone == 75)
		return 0;
		
    if (value >= 50)
    {
        returned = 1;

        // Now we find who's attacking us, and if no one is, return.

        for (tch = ch->room->people; tch; tch = tch->next_in_room)
        {
            if (tch->fighting == ch)
                break;
        }

        if (!tch)
            return 0;

        // Same for us - we need to have a real weapon in our hands our we're not fighting, we're just a training dummy.

        obj1 = ch->left_hand;
        obj2 = ch->right_hand;

        if (obj1 && (obj1->location == WEAR_PRIM || obj1->location == WEAR_SEC || obj1->location == WEAR_BOTH) && !IS_SET (obj1->econ_flags, 1 << 12))
            returned = 1;
        else if (obj2 && (obj2->location == WEAR_PRIM || obj2->location == WEAR_SEC || obj2->location == WEAR_BOTH) && !IS_SET (obj2->econ_flags, 1 << 12))
            returned = 1;
        else
            return 0;

        // Now that we have someone attacking us, let's make sure he has a weapon in the left or right hand, and tht weapon
        // isn't a practice weapon, i.e. our opponent is coming at us with a -real- weapon.

        tobj1 = tch->left_hand;
        tobj2 = tch->right_hand;

        // If he's a NPC who's not punching, we'll assume it's a lethal combat.

        if (IS_NPC(tch) && tch->nat_attack_type > 0)
            returned = 1;
        else if (tobj1 && (tobj1->location == WEAR_PRIM || tobj1->location == WEAR_SEC || tobj1->location == WEAR_BOTH) && !IS_SET (tobj1->econ_flags, 1 << 12))
            returned = 1;
        else if (tobj2 && (tobj2->location == WEAR_PRIM || tobj2->location == WEAR_SEC || tobj2->location == WEAR_BOTH) && !IS_SET (tobj2->econ_flags, 1 << 12))
            returned = 1;
        else
            return 0;
    }

    // So, we passed the fifty test, but we're still got higher skill - do we have 3 real opponents?
    if (returned && value >= 60)
    {
        returned = 0;

        for (rch = ch->room->people; rch; rch = rch->next_in_room)
        {
            if (rch->fighting == ch && rch != tch)
                break;
        }

        if (!rch)
            return 0;

        // If you're not injured, fuck off.
        damage = get_damage_total(ch);
        if (damage == 0)
            return 0;

        // Does rch have a real weapon?

        robj1 = rch->left_hand;
        robj2 = rch->right_hand;

        if (IS_NPC(rch) && rch->nat_attack_type > 0)
            returned = 1;
        else if (robj1 && (robj1->location == WEAR_PRIM || robj1->location == WEAR_SEC || robj1->location == WEAR_BOTH) && !IS_SET (robj1->econ_flags, 1 << 12))
            returned = 1;
        else if (robj2 && (robj2->location == WEAR_PRIM || robj2->location == WEAR_SEC || robj2->location == WEAR_BOTH) && !IS_SET (robj2->econ_flags, 1 << 12))
            returned = 1;
        else
            return 0;
    }

    // Again - do we have a fourth opponent with a weapon tohelp wail on us?
    if (returned && value >= 70)
    {
        returned = 0;

        for (sch = ch->room->people; sch; sch = sch->next_in_room)
        {
            if (sch->fighting == ch && sch != tch && sch != rch)
                break;
        }

        if (!sch)
            return 0;

        // If you're not on four stars or less, fuck off.
        damage = get_damage_total(ch);
        if (damage < tch->max_hit * .1667)
            return 0;

        // Does rch have a real weapon?

        sobj1 = sch->left_hand;
        sobj2 = sch->right_hand;

        if (IS_NPC(sch) && sch->nat_attack_type > 0)
            returned = 1;
        else if (sobj1 && (sobj1->location == WEAR_PRIM || sobj1->location == WEAR_SEC || sobj1->location == WEAR_BOTH) && !IS_SET (sobj1->econ_flags, 1 << 12))
            returned = 1;
        else if (sobj2 && (sobj2->location == WEAR_PRIM || sobj2->location == WEAR_SEC || sobj2->location == WEAR_BOTH) && !IS_SET (sobj2->econ_flags, 1 << 12))
            returned = 1;
        else
            return 0;
    }

    // If we're -really- good at combat, then we need to be a bit wounded or we're not going to get any better: folks perform better under stress et. al.
    if (returned && value >= 80)
    {
        returned = 0;

        // If you're not on three stars or less, fuck off.
        damage = get_damage_total(ch);
        if (damage < tch->max_hit * .3333)
            return 0;
        else
            return 1;

    }

    return returned;
}


int
skill_use (CHAR_DATA * ch, int skill, int diff_mod)
{
    double roll;
    int lv;
    int cap;
    int min, max;
    int i, j = 0;
    int criteria = 0;
    bool gain = false;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!real_skill (ch, skill))
        return 0;

    /*
     if(get_soma_affect(ch, SOMA_PLANT_BLINDNESS)
        && get_soma_affect(ch, SOMA_PLANT_BLINDNESS)->a.soma.atm_power >= 500)
     diff_mod += 5;
     */

    lv = calc_lookup (ch, REG_LV, skill);
    cap = calc_lookup (ch, REG_CAP, skill);

    cap = MIN (cap, SKILL_CEILING);

	// If we're rolling one of the "knowledge" skills, then
	// our floor is set differently depending on what our
	// education skill is. If we've got a maximum of 50
	// education, then we'll roll 10-whatever, rather than
	// 1-whatever, which is very, very powerful.

	int floor = 1;

	int modEducation = 0;
	modEducation = ((ch->skills[SKILL_EDUCATION] - 10) / 5) + 1;
	modEducation = MAX(1, modEducation);

    switch (skill)
    {
		case SKILL_CHEMISTRY:
		case SKILL_MECHANICS:
		case SKILL_ELECTRONICS:
		case SKILL_COMPUTEROLOGY:
		case SKILL_BIOLOGY:
		case SKILL_MEDICINE:
			floor = modEducation;
			break;
		case SKILL_GUNSMITH:
		case SKILL_ARMORCRAFT:
		case SKILL_WEAPONCRAFT:
		case SKILL_HANDICRAFT:
			floor = MAX(modEducation / 2, 1);
			break;
    }

    roll = number (floor, SKILL_CEILING);

    if (!AWAKE (ch))
        return 0;

/*    if (get_affect (ch, ATTACK_DIFF))
    {
        if (!get_affect (ch, MAGIC_SKILL_GAIN_STOP + skill)
                && !get_affect (ch, MAGIC_FLAG_NOGAIN + skill)
                && !get_affect (ch, MAGIC_FLAG_IGNORE + skill))
        {
            if (!skill_max(ch, 0, 0) && (ch->skills[skill] < calc_lookup(ch, REG_CAP, skill)))
            {
                ch->skills[skill]++;
                ch->pc->skills[skill]++;
                min = 40;

                max = 40 + ch->skills[skill] + number (1, 60);
                max = MIN (180, max);

                magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill,
                                  number (min, max), 0, 0, 0, 0);
            }
            if (ch->skills[skill] > calc_lookup(ch, REG_CAP, skill))
            {
                ch->skills[skill] = calc_lookup(ch, REG_CAP, skill);
                ch->pc->skills[skill] = calc_lookup(ch, REG_CAP, skill);
            }
        }
    }
 */      
    if (roll <= skill_level (ch, skill, diff_mod))
        return 1;

    if (IS_NPC (ch))
    {
        if (ch->skills[skill] < cap && lv >= number (1, SKILL_CEILING))
            ch->skills[skill]++;
    }
    else if (ch->pc->skills[skill] < cap && lv >= number (1, SKILL_CEILING))
    {
        // Combat_skill_use determines whether we deserve a skill point or not depending on a variety of criteria.
        // If true, we add 5 to our score, if not, add one. Then, we check what the criteria we need to beat is
        // depending on our skill level. If we're under it, no skill gain for us. If we're over, then we go through
        // and do the rest of the tests.
        //
        // What this aims to do is punish folks who simply sit around twink-sparring all day long, or at least make
        // them work a lot harder to get their skill points. Dudes who are going out and fighting for real will raise
        // a lot faster.
        if (skill <= MAX_COMBAT_SKILL)
        {
            if (combat_skill_use(ch, skill))
                ch->combat_counter[skill] += 5;
            else
                ch->combat_counter[skill] ++;

            if (ch->skills[skill] < 50)
                criteria = 0;
            else if (ch->skills[skill] < 60)
                criteria = 5;
            else if (ch->skills[skill] < 70)
                criteria = 5;
            else if (ch->skills[skill] < 80)
                criteria = 10;
            else if (ch->skills[skill] < 90)
                criteria = 10;
            else
                criteria = 15;

            if (!(ch->combat_counter[skill] >= criteria))
                return 0;
            else
                ch->combat_counter[skill] = 0;
        }

        if (!ch->descr() || ch->descr()->idle)	/* No skill gain idle/discon */
            return 0;

        if (IS_SET (ch->room->room_flags, OOC))	/* No skill gain in OOC areas. */
            return 0;

        // If they've got a temp-block on that skill, an admin-set no-gain, or they're deliberately
        // ignoring that skill, then ignore any gain.

        if (!get_affect (ch, MAGIC_SKILL_GAIN_STOP + skill)
                && !get_affect (ch, MAGIC_FLAG_NOGAIN + skill)
                && !get_affect (ch, MAGIC_FLAG_IGNORE + skill))
        {

            // If players haven't reached their skill cap, then give them the point.
            // If they have, select a skill at random to deduct a point from, and
            // then give them their point but not from their native tongue,
            // skills under 11, skills being focused, the skill in question, and Common + Literacy + Common-Script.
            // If there isn't any skills to select (e.g. they've focused everything),
            // then they don't get a skill-raise at all.

            if (!skill_max(ch, 0, 0))
            {
                ch->skills[skill]++;
                ch->pc->skills[skill]++;
                min = 40;

                max = 40 + ch->skills[skill] + number (1, 60);
                max = MIN (180, max);

                magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill,
                                  number (min, max), 0, 0, 0, 0);

                return 2;
            }
            else
            {
                for (i = 1; i <= LAST_SKILL; i++)
                {
                    if ((ch->skills[i] < 11) || get_affect (ch, MAGIC_FLAG_IGNORE + i) || get_affect (ch, MAGIC_FLAG_FOCUS + i) || (skills[i] == skills[skill]) || (i == atoi (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE))) || skills[i] == skills[SKILL_COMMON])
                        ;
                    else
                        gain = true;
                }

                if (gain)
                {
                    while (j == 0)
                    {
                        i = number(0, LAST_SKILL);

                        if (real_skill (ch, i) && (ch->skills[i] >= 11)
                                && !get_affect (ch, MAGIC_FLAG_IGNORE + i) && !get_affect (ch, MAGIC_FLAG_FOCUS + i) && (skills[i] != skills[skill]) && (i != atoi (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE))) && skills[i] != skills[SKILL_COMMON])
                            j = i;
                    }

                    ch->skills[j]--;
                    ch->pc->skills[j]--;

                    ch->skills[skill]++;
                    ch->pc->skills[skill]++;
                    min = 60;

                    max = 60 + ch->skills[skill] + number (1, 40);
                    max = MIN (200, max);

                    if (ch->skills[skill] < 10)
                    {
                        min = 200;
                        max = 300;
                    }

                    magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill,
                                      number (min, max), 0, 0, 0, 0);

                    //sprintf(buf, "You feel your %s skill is improved, at the expense of your %s skill.\n", skills[skill], skills[j]);
                    //send_to_char(buf, ch);

                    return 2;
                }
                else
                {
                    //sprintf(buf, "You feel that you were close to increasing your %s skill, but something was holding you back.\n", skills[skill]);
                    //send_to_char(buf, ch);
                    return 0;
                }
            }
        }
    }

    return 0;
}
