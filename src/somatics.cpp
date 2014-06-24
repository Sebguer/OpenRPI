/*------------------------------------------------------------------------\
|  somatics.c : Short and Long Term Somatic Effects   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/


#include <stdio.h>
#include <time.h>

#ifndef MACOSX
#include <malloc.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

const char *scent_tier[11] = {
    "the merest suggestion",
    "a bare whiff",
    "a faint essence",
    "a slight hint",
    "a discernible scent",
    "a noticable aroma",
    "a distinct odor",
    "a strong smell",
    "a pungent aroma",
    "a heady stench",
    "an overpowering stink"
};

// Returns the lookup string for that scent_ref
char *
scent_lookup (int scent_ref)
{
    vector<defined_scent*>::iterator it;
    for (it = defined_scent_list.begin(); it != defined_scent_list.end(); it++)
    {
        if ((*it)->id == scent_ref)
        {
            return (*it)->name;
        }
    }

    return NULL;
}

int
scent_lookup (char *scent_name)
{
    vector<defined_scent*>::iterator it;
    for (it = defined_scent_list.begin(); it != defined_scent_list.end(); it++)
    {
        if (!str_cmp((*it)->name, scent_name))
        {
            return (*it)->id;
        }
    }

    return NULL;
}

int
scent_strength (SCENT_DATA *scent)
{
    int tier = 0;
    int atm = scent->atm_power / 100;
    int rat = scent->rat_power / 20;

    // If our rat is 2 greater than atm, we reduce it to only 2 greater than that.
    if (rat > atm + 2)
        rat = MAX(atm + 2, 1);

    // Tier is between 0 and 10. Most things can't smell a zero.
    tier = (atm + rat);
    tier = MIN(tier, 10);
    tier = MAX(0, tier);

    // If our scent is permanent, we can always detect it.
    //if (scent->permanent)
    //    tier = MAX(1, tier);

    return tier;
}

void
add_scent(ROOM_DATA *room, int scent_ref, int permanent, int atm_power, int pot_power, int rat_power, int create)
{
    SCENT_DATA *scent;

    if (!room)
        return;
        
    if (!scent_ref)
        return;
        
    // If we've already got this scent...
    if ((scent = get_scent(room, scent_ref)))
    {
        // And it's already permanent, then we don't need to worry about anything.
        if (scent->permanent)
        {
            return;
        }
        // It's not permanent, and the potential power is greater than our
        // current potential power, add everythig to it at rat_power rat,
        // capping it our native rates/pot_power.
        else if ((scent->permanent && pot_power >= scent->pot_power) || (pot_power > scent->pot_power))
        {
            // If the scent adding is not permanent, then we add at a much reduced rat.
            if (permanent)
                scent->atm_power += atm_power;
            else
                scent->atm_power += atm_power / 2;

            scent->pot_power = MAX(scent->pot_power, pot_power - 1);

            if (rat_power > scent->rat_power)
            {
                if (permanent)
                    scent->rat_power += (rat_power - scent->rat_power) / 2;
                else
                    scent->rat_power += (rat_power - scent->rat_power) / 4;
            }

            if (scent->atm_power > scent->pot_power)
                scent->atm_power = scent->pot_power;

            if (scent->atm_power > 1000)
                scent->atm_power = 1000;

            if (scent->rat_power > 100)
                scent->rat_power = 100;

            return;
        }
    }
    // Otherwise, we don't have this scent, so let's add it all
    // to where we began.
    else
    {
        scent = NULL;
        CREATE (scent, SCENT_DATA, 1);
        if (create)
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power;
            scent->rat_power = rat_power;
            scent->permanent = 1;
        }
        else
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power - 1;
            if (permanent)
                scent->rat_power = rat_power;
            else
                scent->rat_power = rat_power / 2;
            scent->permanent = 0;
        }

        scent->next = NULL;

        if (scent->atm_power > 1000)
            scent->atm_power = 1000;

        if (scent->pot_power > 1000)
            scent->pot_power = 1000;

        if (scent->rat_power > 100)
            scent->rat_power = 100;

        scent_to_room(room, scent);
        return;
    }
}


void
add_scent(OBJ_DATA *obj, int scent_ref, int permanent, int atm_power, int pot_power, int rat_power, int create)
{
    SCENT_DATA *scent;

    if (!obj)
        return;

    if (!scent_ref)
        return;
        
    // If we've already got this scent...
    if ((scent = get_scent(obj, scent_ref)))
    {
        // And it's already permanent, then we don't need to worry about anything.
        if (scent->permanent)
        {
            return;
        }
        // It's not permanent, and the potential power is greater than our
        // current potential power, add everythig to it at rat_power rat,
        // capping it our native rates/pot_power.
        else if ((scent->permanent && pot_power >= scent->pot_power) || (pot_power > scent->pot_power))
        {
            // If the scent adding is not permanent, then we add at a much reduced rat.
            if (permanent)
                scent->atm_power += atm_power;
            else
                scent->atm_power += atm_power / 2;

            scent->pot_power = MAX(scent->pot_power, pot_power - 1);

            if (rat_power > scent->rat_power)
            {
                if (permanent)
                    scent->rat_power += (rat_power - scent->rat_power) / 2;
                else
                    scent->rat_power += (rat_power - scent->rat_power) / 4;
            }

            if (scent->atm_power > scent->pot_power)
                scent->atm_power = scent->pot_power;

            if (scent->atm_power > 1000)
                scent->atm_power = 1000;

            if (scent->rat_power > 100)
                scent->rat_power = 100;

            return;
        }
    }
    // Otherwise, we don't have this scent, so let's add it all
    // to where we began.
    else
    {
        scent = NULL;
        CREATE (scent, SCENT_DATA, 1);
        if (create)
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power;
            scent->rat_power = rat_power;
            scent->permanent = 1;
        }
        else
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power - 1;
            if (permanent)
                scent->rat_power = rat_power;
            else
                scent->rat_power = rat_power / 2;
            scent->permanent = 0;
        }

        scent->next = NULL;

        if (scent->atm_power > 1000)
            scent->atm_power = 1000;

        if (scent->pot_power > 1000)
            scent->pot_power = 1000;

        if (scent->rat_power > 100)
            scent->rat_power = 100;

        scent_to_obj(obj, scent);
        return;
    }
}

void
add_scent(CHAR_DATA *ch, int scent_ref, int permanent, int atm_power, int pot_power, int rat_power, int create)
{
    SCENT_DATA *scent;

    if (!ch)
        return;

    if (!scent_ref)
        return;
        
    // If we've already got this scent...
    if ((scent = get_scent(ch, scent_ref)))
    {
        // And it's already permanent, then we don't need to worry about anything.
        if (scent->permanent)
        {
            return;
        }
        // It's not permanent, and the potential power is greater than our
        // current potential power, add everythig to it at rat_power rat,
        // capping it our native rates/pot_power.
        else if ((permanent && pot_power >= scent->pot_power) || (pot_power > scent->pot_power))
        {
            // If the scent adding is not permanent, then we add at a much reduced rat.
            if (permanent)
                scent->atm_power += atm_power;
            else
                scent->atm_power += atm_power / 2;

            scent->pot_power = MAX(scent->pot_power, pot_power - 1);

            if (rat_power > scent->rat_power)
            {
                if (permanent)
                    scent->rat_power += (rat_power - scent->rat_power) / 2;
                else
                    scent->rat_power += (rat_power - scent->rat_power) / 4;
            }

            if (scent->atm_power > scent->pot_power)
                scent->atm_power = scent->pot_power;

            if (scent->atm_power > 1000)
                scent->atm_power = 1000;

            if (scent->rat_power > 100)
                scent->rat_power = 100;

            return;
        }
    }
    // Otherwise, we don't have this scent, so let's add it all
    // to where we began.
    else
    {
        scent = NULL;
        CREATE (scent, SCENT_DATA, 1);
        if (create)
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power;
            scent->rat_power = rat_power;
            scent->permanent = 1;
        }
        else
        {
            scent->scent_ref = scent_ref;
            scent->atm_power = atm_power;
            scent->pot_power = pot_power - 1;
            if (permanent)
                scent->rat_power = rat_power;
            else
                scent->rat_power = rat_power / 2;
            scent->permanent = 0;
        }

        scent->next = NULL;

        if (scent->atm_power > 1000)
            scent->atm_power = 1000;

        if (scent->pot_power > 1000)
            scent->pot_power = 1000;

        if (scent->rat_power > 100)
            scent->rat_power = 100;

        scent_to_mob(ch, scent);
        return;
    }
}

void
soma_stat (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    switch (af->type)
    {
    case SOMA_NERVES_HEADACHE:
        sprintf (buf2, "a headache");
        break;
// combat stim effect
    case SOMA_COMBAT_STIM:
        sprintf (buf2, "the combat stim effect");
        break;
// combat stim withdrawal effect
    case SOMA_STIM_WITHDRAWAL:
        sprintf (buf2, "the combat stim withdrawal effect");
        break;
// genetic mutation tainted cure effect/vulnerable to infection
    case SOMA_INFECTION_VULNERABLE:
        sprintf(buf2, "the genetic mutation vulnerability effect");
        break;
// genetic mutation cure effect
    case SOMA_GENETIC_MUTATION_CURE:
        sprintf (buf2, "the genetic mutation cure");
        break;
// genetic mutation effect
    case SOMA_GENETIC_MUTATION:
        sprintf (buf2, "the genetic mutation");
        break;
// Isogen effect.
    case SOMA_ISOGEN:
        sprintf (buf2, "the genetic mutation isogen");
        break;
// Genetic mutation shakes.
    case SOMA_SHAKES:
        sprintf (buf2, "the genetic terror shakes");
        break;
// effects caused by blunt weapons
    case SOMA_BLUNT_MEDHEAD:
        sprintf (buf2, "minorly stunned");
        break;
    case SOMA_BLUNT_SEVHEAD:
        sprintf (buf2, "a concussion");
        break;
    case SOMA_BLUNT_R_SEVARM:
        sprintf (buf2, "a broken right arm");
        break;
    case SOMA_BLUNT_L_SEVARM:
        sprintf (buf2, "a broken left arm");
        break;
    case SOMA_BLUNT_SEVLEG:
        sprintf (buf2, "a broken leg");
        break;
    case SOMA_BLUNT_SEVBODY:
        sprintf (buf2, "a broken rib");
        break;

    case SOMA_EVENT_COLD:
        sprintf (buf2, "a mysterious sickness");
        break;
    case SOMA_PUNISHMENT_WOUND:
        sprintf (buf2, "some imm-enforced wound roleplay");
        break;

    case SOMA_NO_RARM:
        sprintf (buf2, "a severed right arm");
        break;
    case SOMA_NO_LARM:
        sprintf (buf2, "a severed left arm");
        break;
    case SOMA_NO_RLEG:
        sprintf (buf2, "a severed right leg");
        break;
    case SOMA_NO_LLEG:
        sprintf (buf2, "a severed left leg");
        break;
    case SOMA_NO_HEAD:
        sprintf (buf2, "a severed head");
        break;
    // The genetic mutation shakes.
    default:
        sprintf (buf2, "an unknown somatic effect");
        break;
    }

    send_to_char("\n", ch);
    if (!IS_MORTAL (ch))
    {

            sprintf (buf, "#2%5d#0   Suffers from %s for %d more in-game hours.\n        Latency: %d hrs Power: %d to %d (%d @ %d min)\n        A: %d min, D: %d min, S: %d min, R: %d min\n",
                     af->type, buf2, af->a.soma.duration, af->a.soma.latency,
                     af->a.soma.max_power, af->a.soma.lvl_power, af->a.soma.atm_power,
                     af->a.soma.minute, af->a.soma.attack, af->a.soma.decay,
                     af->a.soma.sustain, af->a.soma.release);
    }
    else
    {
        sprintf (buf, "You suffer from %s.", buf2);
    }
    send_to_char (buf, ch);
}



void
soma_ten_second_affect (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
    int save = 0, stat = 0, save2 = 0;
    char *locat = NULL;
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char * bodyloc = 0;
    WOUND_DATA *wound = NULL;
    AFFECTED_TYPE *saf = NULL;

    stat = GET_CON (ch);
    if ((number (1, 1000) > af->a.soma.atm_power)
            || (number (1, (stat > 25) ? stat : 25) <= stat))
        return;

    switch (af->type)
    {
    case SOMA_NERVES_HEADACHE:

        stat = GET_WIL (ch);
        save = number (1, (stat > 20) ? stat : 20);

        if (save > stat)
        {
            act ("Your head pounds with a headache.", true, ch, 0, 0, TO_CHAR);
        }
        break;

    case SOMA_BLUNT_MEDHEAD:

        switch (number(0,2))
        {
        case 0:
            send_to_char("Your vision swims, and you struggle to remain upright.\n", ch);
            break;
        case 1:
            send_to_char("Everything seems suddenly closer in your vision, and you stumble as you try to orientate yourself.\n",  ch);
            break;
        default:
            send_to_char("Your ears ring and vision blackens for a moment.\n", ch);
            break;
        }

        if (!number(0,2) && GET_POS(ch) > SIT)
        {
            act("$n stumbles back and forth on $s feet.\n", true, ch, 0, 0, TO_ROOM);
        }
        break;

        /* Player vaccination for the Genetic Mutation. Not all that effective, it only slows the growth a little. */

    /*
    case SOMA_ISOGEN:

        break;

    case SOMA_WITHDRAWAL_ISOGEN

        break;

    */ 

    case SOMA_GENETIC_MUTATION:

        stat = GET_CON (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (save >= stat)
        {
            switch (number(0,3))
            {
            case 0:
                send_to_char("You feel your vision narrow and go to black.\n", ch);
                act ("$n's eyes become distant and glassy.\n", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                break;
            case 1: 
                if (GET_POS (ch) == SIT || GET_POS (ch) == STAND)
                {
                GET_POS (ch) = REST;
                send_to_char("You feel weak, falling as your legs collapse from under you.\n", ch);
                act ("$n stumbles and falls, $s knees buckling.\n", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                }
                else
                {
                send_to_char("You feel weak, your limbs having turned to jelly.\n", ch);
                act ("$n looks pale.\n", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                }
                break;
            case 2: 
                send_to_char("You sweat profusely, your head warm to the touch.\n", ch);
                act ("$n looks feverish.\n", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                break;
            default:
                send_to_char("You double over involuntarily as overwhelming pain floods your body.\n", ch);
                act ("$n doubles over suddenly.\n", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                break;
            }
        }
        break;

    // The shakes effect from genetic terrors.

    case SOMA_SHAKES:

        break;

    case SOMA_GENETIC_MUTATION_CURE:

        break;
        
    case SOMA_BLUNT_SEVLEG:
        stat = GET_WIL (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (save > stat && GET_POS(ch) > SIT)
        {
            switch (number(0,2))
            {
            case 0:
                send_to_char("You gasp in agony as white hot pain shoots up your leg.\n", ch);
                break;
            case 1:
                send_to_char("Your leg buckles beneath you, pain radiating from your wound.\n",  ch);
                break;
            default:
                send_to_char("Your vision narrows momentarily as pain flares from your leg.\n", ch);
                break;
            }
        }
        break;

    case SOMA_BLUNT_L_SEVARM:
        stat = GET_WIL (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (save > stat)
        {
            switch (number(0,2))
            {
            case 0:
                send_to_char("You gasp in agony as white hot pain shoots up your arm.\n", ch);
                break;
            case 1:
                send_to_char("Your arm cramps and jerks, pain radiating from your wound.\n",  ch);
                break;
            default:
                send_to_char("Your vision narrows momentarily as pain flares from your arm.\n", ch);
                break;
            }
        }
        break;

    case SOMA_BLUNT_R_SEVARM:
        stat = GET_WIL (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (save > stat)
        {
            switch (number(0,2))
            {
            case 0:
                send_to_char("You gasp in agony as white hot pain shoots up your arm.\n", ch);
                break;
            case 1:
                send_to_char("Your arm cramps and jerks, pain radiating from your wound.\n",  ch);
                break;
            default:
                send_to_char("Your vision narrows momentarily as pain flares from your arm.\n", ch);
                break;
            }
        }
        break;

    case SOMA_BLUNT_SEVHEAD:
        stat = GET_WIL (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (GET_POS (ch) > SIT && !IS_SUBDUEE(ch))
        {
            if (save > stat)
            {

                switch (number(0,2))
                {
                case 0:
                    send_to_char("The world twisting and spinning, you fall to your knees.\n", ch);
                    break;
                case 1:
                    send_to_char("Everything goes dark, and you wake up face down on the ground.\n",  ch);
                    break;
                default:
                    send_to_char("You begin to pitch forward, unable to control yourself as you approach the ground.\n", ch);
                    break;
                }

                sprintf(buf, "$n stumbles, falling to $s knees.");
                act (buf, true, ch, 0, 0, TO_ROOM);
                GET_POS (ch) = REST;
                add_second_affect (SA_STAND, ((25-GET_WIL(ch))+number(1,3)), ch, NULL, NULL, 0);
                if (is_outdoors(ch->room))
                {
                    object__enviro(ch, NULL, COND_DIRT, 5, HITLOC_NONE);
                    object__enviro(ch, NULL, COND_DUST, 10, HITLOC_NONE);
                }
            }
        }
        else
        {
            switch (number(0,2))
            {
            case 0:
                send_to_char("Your vision swims, and you struggle to remain upright.\n", ch);
                break;
            case 1:
                send_to_char("Suddenly, everything seems closer, and you stumble to keep standing.\n",  ch);
                break;
            default:
                send_to_char("Your ears ring and vision blackens for a moment.\n", ch);
                break;
            }
            if (!number(0,2) && GET_POS(ch) > SIT)
            {
                act("$n stumbles back and forth on $s feet.\n", true, ch, 0, 0, TO_ROOM);
            }
        }
        break;

    case SOMA_PUNISHMENT_WOUND:
        switch (number(0,6))
        {
        case 0:
            act ("The intense agony of your wounds drives tears to your eyes.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,1))
                act ("Involuntary tears well in $n's eyes.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 1:
            act ("Horrific pangs of agony wracks through your body.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,1))
                act ("$n gasps in pain.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 2:
            act ("Your wounds ache and burn.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,1))
                act ("$n grimaces uncomfortably.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 3:
            act ("The pain from your injuries forces you to cry out.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,1))
                act ("$n cries out in agony.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 4:
            act ("The anguish caused by your wounds forces a whimper from your lips.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,1))
                act ("$n whimpers in pain.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 5:
            if (GET_POS (ch) > SIT)
            {
                if (get_affect (ch, MAGIC_HIDDEN) && would_reveal (ch))
                    remove_affect_type (ch, MAGIC_HIDDEN);

                act ("Suffering floods your body, dropping you to your knees.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                act ("$n collapses to $s knees, $s fall accompanied by a groan of suffering.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                GET_POS (ch) = REST;
                add_second_affect (SA_STAND, ((25-GET_AGI(ch))+number(1,3)), ch, NULL, NULL, 0);
            }
            break;
        case 6:
            act ("You only just muffle screaming from the extent of your terrible wounds.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            if (!number(0,3))
                act ("$n muffles a scream of agony.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        }
        break;

    case SOMA_EVENT_COLD:
        switch (number(0,2))
        {
        case 0:
            act ("You feel a harsh, tickling sensation at the back of your throat that erupts into a cough.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n erupts into a fit of wet, wheezing coughing.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 1:
            act ("Your eyes begin to water and your nose starts to itch until a huge sneeze wracks your entire body, forcing you to shut your eyes.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n shudders and then buckles over in a full-forced sneeze.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        case 2:
            act ("You feel a leaking sensation in your nostrils and sniff reflexively to draw back the dribbling mucous.", true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n sniffles loudly.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;
        }
        break;

    default:
        break;
    }
}


void
soma_rl_minute_affect (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
    int minute = ++af->a.soma.minute;
    int max_power = af->a.soma.max_power;
    int lvl_power = af->a.soma.lvl_power;

    int attack = af->a.soma.attack;
    int decay = af->a.soma.decay;
    int sustain = af->a.soma.sustain;
    int release = af->a.soma.release;
    WOUND_DATA *wound;
    bool broken = false;
    int save, stat = 0; 

    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char * bodyloc = 0;

    switch (af->type)
    {

    /* Nothing really happens here with the isogen. We're just making it a somatic affect so players' infection
    growth is potentially slowed. However, after taking isogen and it's about to end, they will be given a
    isogen withdrawal effect. If they inject the isogen again while the withdrawal is going on, the duration 
    of isogen will be nil. -- NOT RELEVANT ANYMORE, but putting it here anyway in case I change my mind -- Tiamat */
    /*
   case SOMA_ISOGEN
            if (af->a.soma.duration) <= 1)
            {
            send_to_char( "You feel that t.\n", ch)
                if (af != get_soma_affect(ch, SOMA_WITHDRAWAL_ISOGEN))
                {
                    soma_add_affect(ch, SOMA_WITHDRAWAL_ISOGEN, 400, 0, 0, 800, 0, 800, 2, 4, 6, 8);
                }   
            }
        break;

    case SOMA_WITHDRAWAL_ISOGEN
            if (af = get_soma_affect (ch, SOMA_ISOGEN))
            {
                send_to_char("You feel you've ")
                affect_remove(ch, SOMA_ISOGEN)
            }
    */

    /* Combat stim effect, which boosts attack/defense at the cost of a withdrawal effect. */ 
    case SOMA_COMBAT_STIM:

        if (minute <= attack)
        {
            af->a.soma.atm_power = (max_power * minute) / attack;
        }
        else if (minute <= decay)
        {
            af->a.soma.atm_power = max_power - (((max_power - lvl_power) *
                                                 (minute - attack)) / (decay - attack));
        }
        else if (minute <= sustain)
        {
            af->a.soma.atm_power = lvl_power;
        }
        else if (minute <= release)
        {
            af->a.soma.atm_power = lvl_power - (((lvl_power) *
                                                 (minute - sustain)) / (release - sustain));
        }
        else
        {
            // Send them the all-clear message, if the poison has one.
            send_to_char(lookup_poison_variable(af->type, 3), ch);
            affect_remove (ch, af);
        }

        if (af->a.soma.minute < af->a.soma.decay)
        {
        switch(number(0,2))
            {
            case 0:
            send_to_char("You are alert, your vision and hearing sharper.\n", ch);
            break;

            case 1:
            send_to_char("Your body feels at its peak; you are energetic.\n", ch);
            break;

            case 2:
            send_to_char("You can perceive your thoughts with crystal-clear clarity.\n", ch);
            break;
            }

            if (get_soma_affect(ch, SOMA_STIM_WITHDRAWAL))
            {
                send_to_char("Your lethargy slips away, and you feel awake and sharp.\n", ch);
                affect_remove(ch, get_soma_affect(ch,SOMA_STIM_WITHDRAWAL));
            }
        }
        else
        {
                if (!get_soma_affect(ch, SOMA_STIM_WITHDRAWAL))
                {
                    soma_add_affect(ch, SOMA_STIM_WITHDRAWAL, 40, 0, 0, 800, 0, 800, 2, 4, 6, 8);
                    send_to_char("Lethargy sinks in, and you feel sleepy and sluggish.\n", ch);
                }
        }
    break;

    case SOMA_STIM_WITHDRAWAL:

           switch(number(0,4))
            {
            case 0:
            send_to_char("Your head pounds with a headache.\n", ch);
            break;

            case 1:
            send_to_char("You feel lethargic, and the world around you appears to be in slow motion.\n", ch);
            break;

            case 2:
            send_to_char("The back of your throat feels dry.\n", ch);
            break;

            case 3:
            send_to_char("Your fingers tremble and twitch restlessly for a few moments.\n", ch);
            act ("$n's hands twitch for a moment.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;

            case 4:
                if (GET_POS (ch) > SIT)
                {
                    send_to_char("The world spins, and you struggle to stand upright.\n", ch);
                    act ("$n stumbles for a moment.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                }
                else 
                {
                    send_to_char("The world spins, your vision swimming.\n", ch);
                    act ("$n's face looks slightly green.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
                }
            break;
            }
    break;


    /* Echoes for when the player reaches near turning. ALso, roll based on CON to see if 
    players mutate or not every tick. If they reach mutation threshold, kill them. If player 
    is infected with SOMA_SHAKES and SOMA_GENETIC_MUTATION, increase infection rate to +45 
    per tick, instead of +20. If player is injected with the cure slow the growth to +10. 
    If it's isogen, subtract -5. If they have shakes, mutation, and isogen, slow growth to +40.
    If they've got the cure, -15 to growth. */ 

    case SOMA_GENETIC_MUTATION:

        stat = GET_CON (ch);
        save = number (1, (stat > 20) ? stat : 20);
        if (save >= stat)
            {
                 af->a.soma.lvl_power += 10;

                if ((get_soma_affect(ch, SOMA_SHAKES)))
                {
                    af->a.soma.lvl_power += 35;
                }
                else 
                {
                    af->a.soma.lvl_power += 10;
                }
                if ((get_soma_affect( ch, SOMA_ISOGEN )))
                {
                    af->a.soma.lvl_power -= 5;
                }
                if ((get_soma_affect( ch, SOMA_GENETIC_MUTATION_CURE )))
                {
                    af->a.soma.lvl_power -= 15;
                }
                if ((get_soma_affect( ch, SOMA_INFECTION_VULNERABLE )) || (get_soma_affect( ch, SOMA_STIM_WITHDRAWAL)))
                {
                    af->a.soma.lvl_power += 25;
                }
            }

        if (lvl_power > 800)
        {
            switch(number(0,3))
            {
            case 0:
            send_to_char("Your hairs stand on end, as you feel yourself grow weaker and weaker.\n", ch);
            break;

            case 1:
            send_to_char("At the very edge of your hearing, you hear faint unintelligible whispering and murmuring.\n", ch);
            break;

            case 2:
            send_to_char("A sudden chill runs up your spine, and you shiver involuntarily.\n", ch);
            act ("$n shivers suddenly.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            break;

            case 3:
            send_to_char("You feel exhausted and tired, your body beginning to fail you.\n", ch);
            break;
            }

            if (af->a.soma.lvl_power >= 1000)
            {
                die(ch);
            }
            if (af->a.soma.lvl_power >= 900)
            {
                sprintf(buf, "%s is at infection level: %d, they're turning when it reaches 1000!", ch->name, af->a.soma.lvl_power);
                send_to_gods (buf);
            }
        }

        break;

        // Genetic terror shakes. Nothing really happening here that wasn't declared already in fight.cpp,
        // with a -20 penalty in attack/defense.

    case SOMA_SHAKES:

        stat = GET_CON (ch);
        save = number (1, (stat > 20) ? stat : 20);
        bodyloc = expand_wound_loc(figure_location(ch, number(0,MAX_HITLOC - 1)));

        if (save >= stat)
        { 
            switch(0)
            {
                case 0:

                sprintf(buf2, "You feel a sharp pain as your %s twitches involuntarily.", bodyloc);
                send_to_char(buf2, ch);
                sprintf(buf3, "$n cringes in pain, as $s %s twitches momentarily.", bodyloc);
                act (buf3, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

                break;
            }
        }   
        else
        { 
                sprintf(buf2, "You feel a strong twitching in your %s, but you control it quickly.", bodyloc);
                send_to_char(buf2, ch);
        }

        if (release <= minute)
        {
           affect_remove(ch, af);
        }
        break; 
        
        // Broken bone effects: they last until the wound has become a "minor" wound.
        // If there is no wound with a corresponding value, then remove the affect.
    case SOMA_BLUNT_SEVHEAD:
        for (wound = ch->wounds; wound; wound = wound->next)
        {
            if (wound->fracture == SOMA_BLUNT_SEVHEAD)
            {
                broken = true;
                if (!str_cmp (wound->severity, "small")
                        || !str_cmp (wound->severity, "minor"))
                {
                    affect_remove (ch, af);
                    wound->fracture = 0;
                }
            }
        }

        if (!broken)
            affect_remove (ch, af);
        break;
    case SOMA_BLUNT_SEVBODY:
        for (wound = ch->wounds; wound; wound = wound->next)
        {
            if (wound->fracture == SOMA_BLUNT_SEVBODY)
            {
                broken = true;
                if (!str_cmp (wound->severity, "small")
                        || !str_cmp (wound->severity, "minor"))
                {
                    affect_remove (ch, af);
                    wound->fracture = 0;
                }
            }
        }

        if (!broken)
            affect_remove (ch, af);
        break;

    case SOMA_BLUNT_SEVLEG:
        for (wound = ch->wounds; wound; wound = wound->next)
        {
            if (wound->fracture == SOMA_BLUNT_SEVLEG)
            {
                broken = true;
                if (!str_cmp (wound->severity, "small")
                        || !str_cmp (wound->severity, "minor"))
                {
                    affect_remove (ch, af);
                    wound->fracture = 0;
                }
            }
        }

        if (!broken)
            affect_remove (ch, af);
        break;

    case SOMA_BLUNT_R_SEVARM:
        for (wound = ch->wounds; wound; wound = wound->next)
        {
            if (wound->fracture == SOMA_BLUNT_R_SEVARM)
            {
                broken = true;
                if (!str_cmp (wound->severity, "small")
                        || !str_cmp (wound->severity, "minor"))
                {
                    affect_remove (ch, af);
                    wound->fracture = 0;
                }
            }
        }

        if (!broken)
            affect_remove (ch, af);
        break;

    case SOMA_BLUNT_L_SEVARM:
        for (wound = ch->wounds; wound; wound = wound->next)
        {
            if (wound->fracture == SOMA_BLUNT_L_SEVARM)
            {
                broken = true;
                if (!str_cmp (wound->severity, "small")
                        || !str_cmp (wound->severity, "minor"))
                {
                    affect_remove (ch, af);
                    wound->fracture = 0;
                }
            }
        }

        if (!broken)
            affect_remove (ch, af);
        break;

    case SOMA_NERVES_HEADACHE:
    case SOMA_BLUNT_MEDHEAD:

        // This is the standard "rise and fall". Four lengths - attack, decay, sustain and release.
        // Two powers - max, and level.
        // From 0 to attack, power rises to max.
        // From attack to decay, power falls from max to decay.
        // From decay to sustain, power holds.
        // From sustain to release, power falls to 0.
        // At release, remove the affect.

        if (minute <= attack)
        {
            af->a.soma.atm_power = (max_power * minute) / attack;
        }
        else if (minute <= decay)
        {
            af->a.soma.atm_power = max_power - (((max_power - lvl_power) *
                                                 (minute - attack)) / (decay - attack));
        }
        else if (minute <= sustain)
        {
            af->a.soma.atm_power = lvl_power;
        }
        else if (minute <= release)
        {
            af->a.soma.atm_power = lvl_power - (((lvl_power) *
                                                 (minute - sustain)) / (release - sustain));
        }
        else
        {
            // Send them the all-clear message, if the poison has one.
            send_to_char(lookup_poison_variable(af->type, 3), ch);
            affect_remove (ch, af);
        }

        break;

    default:
        break;
    }
}


int
lookup_soma (char *argument)
{
    if (!argument)
        return (-1);

    else if (!strcmp(argument, "headache"))
        return (SOMA_NERVES_HEADACHE);

    else
        return (-1);

}

int
soma_add_affect (CHAR_DATA * ch, int type, int duration, int latency,
                 int minute, int max_power, int lvl_power, int atm_power,
                 int attack, int decay, int sustain, int release)
{
    AFFECTED_TYPE *soma;

    // If they already have the somatic effect, add half the timers of the new
    // one to the old, and then select the maximum powers.

    if ((soma = get_affect (ch, type)))
    {
        if (soma->a.soma.duration == -1)	/* Perm already */
            return 0;

        if (duration == -1)
            soma->a.soma.duration = duration;

        // If COMBAT_STIM or GENETIC_MUTATION, add max/lvl/atm powers and minute and 
        // duration.
           else if (soma->type == SOMA_COMBAT_STIM || soma->type == SOMA_GENETIC_MUTATION || soma->type == SOMA_STIM_WITHDRAWAL ) 
        { 
            soma->a.soma.minute = minute;
            soma->a.soma.duration += duration;
            soma->a.soma.attack += attack/2;
            soma->a.soma.decay += decay/2;
            soma->a.soma.sustain += sustain/2;
            soma->a.soma.release += release/2;

            soma->a.soma.max_power += int(max_power);
            soma->a.soma.lvl_power += int(lvl_power);
            soma->a.soma.atm_power += int(atm_power);
          // Do the stuff we want. 
          return 0; 
        } 
        else
        {
            soma->a.soma.duration += duration/2;
            soma->a.soma.attack += attack/2;
            soma->a.soma.decay += decay/2;
            soma->a.soma.sustain += sustain/2;
            soma->a.soma.release += release/2;

            soma->a.soma.max_power = MAX ((int) soma->a.soma.max_power, max_power);
            soma->a.soma.lvl_power = MAX ((int) soma->a.soma.lvl_power, lvl_power);
            soma->a.soma.atm_power = MAX ((int) soma->a.soma.atm_power, atm_power);
            return 0;
        }
    }

    soma = NULL;

    CREATE (soma, AFFECTED_TYPE, 1);

    soma->type = type;

    soma->a.soma.duration = duration;
    soma->a.soma.latency = latency;
    soma->a.soma.minute = minute;
    soma->a.soma.max_power = max_power;
    soma->a.soma.lvl_power = lvl_power;
    soma->a.soma.atm_power = atm_power;
    soma->a.soma.attack = attack;
    soma->a.soma.decay = decay;
    soma->a.soma.sustain = sustain;
    soma->a.soma.release = release;

    affect_to_char (ch, soma);

    return 1;
}


void
do_quaff (CHAR_DATA * ch, char *argument, int cmd)
{
    //char buf[MAX_STRING_LENGTH];
    //OBJ_DATA *obj;

    //argument = one_argument (argument, buf);

    //if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) && !(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
    //  {
    //    act ("You can't find that.", false, ch, 0, 0, TO_CHAR);
    //    return;
    //  }

    //if (obj->obj_flags.type_flag == ITEM_FOOD || obj->obj_flags.type_flag == ITEM_DRINKCON)
    //{
    //  act ("Why don't you just eat or drink from $p instead?", false, ch, obj, 0, TO_CHAR);
    //}

    //if (obj->obj_flags.type_flag != ITEM_POISON_LIQUID)
    //  {
    //    act ("$p isn't a substance you can ingest in any special manner.", false, ch, obj, 0, TO_CHAR);
    //    return;
    //  }

    //ch->delay_type = DEL_QUAFF;
    //ch->delay_info1 = (long int) obj;

    //if (ch->fighting)
    //  {
    //    act
    //  ("Fending off the blows, you somehow manage to prepare to ingest $p.  At length, despite the pressure, you finish it down.",
    //   false, ch, obj, 0, TO_CHAR);
    //    ch->delay = 15;
    //  }
    //else
    //  {
    //    act
    //  ("You steel yourself before ingesting $p.",
    //   false, ch, obj, 0, TO_CHAR);
    //    ch->delay = 5;
    //  }
}

void
delayed_quaff (CHAR_DATA * ch)
{
    //OBJ_DATA *obj;
    //int poisoned = 0;

    //ch->delay = 0;

    //obj = (OBJ_DATA *) ch->delay_info1;

    //soma_add_affect(ch, obj->o.od.value[1], obj->o.od.value[5], 0, 0,
    //                obj->quality, obj->quality/2, obj->quality/3, obj->o.od.value[2],
    //                obj->o.od.value[3], obj->o.od.value[4], obj->o.od.value[5]);

    //poisoned = obj->o.od.value[1];

    //if(obj->o.od.value[0] <= 1)
    //{
    //  act ("You ingest $p, using up the last of it.", true, ch, obj, 0, TO_CHAR);
    //  act ("$n ingests $p, using up the last of it.", true, ch, obj, 0, TO_ROOM);
    //  obj_from_char (&obj, 0);
    //  extract_obj (obj);
    //}
    //else
    //{
    //  obj->o.od.value[0]--;
    //  act ("You ingest $p.", true, ch, obj, 0, TO_CHAR);
    //  act ("$n ingests $p.", true, ch, obj, 0, TO_ROOM);
    //}

    //if(poisoned > 0)
    //  send_to_char(lookup_poison_variable(poisoned, 1), ch);
}

void
do_stink (CHAR_DATA * ch, char *argument, int cmd)
{

    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj = NULL;
    SCENT_DATA *scent = NULL;
    int scent_ref = -1;
    int atm_power = -1;
    int pot_power = -1;
    int rat_power = -1;

    argument = one_argument(argument, buf);

    if (!(tch = get_char_room_vis (ch, buf)))
    {
        if (cmd != 2)
        {
           sprintf (arg, "You don't see %s here.", buf);
           act (arg, false, ch, 0, 0, TO_CHAR);
        }
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        if (cmd != 2)
            send_to_char ("What scent type did you wish to set?\n", ch);
        return;
    }

    scent_ref = atoi(buf);
     
    if (!scent_ref)
        scent_ref = scent_lookup(buf);
        
    if (!(scent_lookup(scent_ref)) && 
       (str_cmp(argument, "remove") || str_cmp("all", buf)))
    {
        if (cmd != 2)
            send_to_char ("I couldn't find that affect in the database.\n", ch);
        return;
    }

    if (!str_cmp (argument, "remove"))
    {
        if (!str_cmp("all", buf))
        {
            for (scent = tch->scent; scent; scent = scent->next)
                remove_mob_scent(tch, scent);
                
            if (cmd != 2)
                send_to_char("All scent types have been removed.\n", ch);
        }   
        else
        {
            if ((scent = get_scent(tch, scent_ref)))
            {
                remove_mob_scent(tch, scent);
            }
            if (cmd != 2)
                send_to_char ("The specified scent type has been removed.\n", ch);
        }
        return;
    }
    else
    {
        sscanf (argument, "%d %d %d", &atm_power, &pot_power, &rat_power);

        if ((atm_power < 0 || pot_power < 0 || rat_power < 0) && str_cmp (buf, "remove"))
        {
            if (cmd != 2)
                send_to_char ("You'll need to enter the atm power, pot power and rat power to get a scent to work.\n", ch);
            return;
        }

      add_scent(tch, scent_ref, 0, atm_power, pot_power, rat_power, 0);
    }
    
    if (cmd != 2)
    {
      send_to_char ("The specified scent type has been added.\n", ch);
      return;
    }
}


void
do_xpoison (CHAR_DATA * ch, char *argument, int cmd)
{

    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *tch = NULL;
    AFFECTED_TYPE *soma = NULL;
    int type;
    int duration;
    int latency;
    int minute;
    int max_power;
    int lvl_power;
    int atm_power;
    int attack;
    int decay;
    int sustain;
    int release;


    argument = one_argument(argument, buf);

    if (str_cmp(buf, "-1") == 0)
    {
        tch = ch;
    }
    else if (!(tch = get_char_room_vis (ch, buf)) && (cmd != 2))
    {
        sprintf (arg, "You don't see %s here.", buf);
        act (arg, false, ch, 0, 0, TO_CHAR);
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf && (cmd != 2))
    {
        send_to_char ("What poison type did you wish to set?\n", ch);
        return;
    }

    type = atoi(buf);
    if (type < SOMA_FIRST || type > SOMA_LAST & (cmd != 2))
    {
        send_to_char ("I couldn't find that affect in the database.\n", ch);
        return;
    }

    if (!str_cmp (argument, "remove"))
    {
        if ((soma = get_affect (tch, type)))
        {
            affect_remove(tch, soma);
        }
        if (cmd != 2)
        {
        send_to_char ("The specified poison type has been removed.\n", ch);
        return;
        }
    }
    else
    {
        sscanf (argument, "%d %d %d %d %d %d %d %d %d %d", &duration, &latency, &minute, &max_power, &lvl_power, &atm_power, &attack, &decay, &sustain, &release);

        if ((duration < 0 || latency < 0 || minute < 0 || max_power < lvl_power
                || lvl_power < 0 || atm_power < 0 || attack < 1 || decay < attack
                || sustain < decay || release < sustain) && str_cmp (buf, "remove"))
        {   if (cmd != 2)
            {
            send_to_char ("You'll need to enter the duration, latency, minute, max power, level power, current power, attack, decay, sustain, and release values to get a poison to work.\n", ch);
            return;
            }
        }

        soma_add_affect (tch, type, duration, latency, minute, max_power, lvl_power, atm_power, attack, decay, sustain, release);
    }
        if (cmd != 2)
        {
        send_to_char ("The specified poison type has been added.\n", ch);
        return;
        }
}

void
do_spray (CHAR_DATA *ch, char *argument, int cmd)
{
    //char arg[MAX_STRING_LENGTH] = { '\0' };
    //char buf[MAX_STRING_LENGTH] = { '\0' };
    //int size = 0;
    //int power = 0;
    //int i = 0;

    //POISON_DATA *poison = NULL;

    //OBJ_DATA *obj = NULL;
    //CHAR_DATA *tch = NULL;
    //CHAR_DATA *sch = NULL;

    //argument = one_argument(argument, arg);

    //if (!*arg)
    //{
    //  send_to_char("Usage: spray <target>\n", ch);
    //  return;
    //}

    //if (!(tch = get_char_room_vis(ch, arg)))
    //{
    //  sprintf(buf, "You don't see #5%s#0 here.\n", arg);
    //  send_to_char(buf, ch);
    //  return;
    //}

    //if (!(((obj = ch->right_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_SPRAY)
    //    || ((obj = ch->left_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_SPRAY)))
    //  {
    //    send_to_char("You need to be holding some type of spray dispenser device to do that.\n", ch);
    //    return;
    //  }

    //if (!obj->poison)
    //{
    //  act("$p does not contain any substance that could be sprayed on to someone.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (ch->balance <= -1)
    //{
    //  send_to_char ("You're far too off-balance to attempt to use a spray dispenser.\n", ch);
    //  return;
    //}

    //ch->balance += -10;
    //ch->balance = MIN (ch->balance, -10);

    //if (tch)
    //  size = do_group_size(tch);

    //if (tch == ch)
    //{
    //  act("$n uses $p to spray a fine mist directly at $mself.",false, ch, obj, 0, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
    //  act("$n uses $p to spray a fine mist directly at yourself.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);


    //  for (poison = obj->poison; poison; poison = poison->next)
    //  {
    //    soma_add_affect(ch, poison->poison_type, poison->duration, poison->latency,
    //  		   poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
    //  		   poison->attack, poison->decay, poison->sustain, poison->release);
    //    //send_to_char(lookup_poison_variable(poison->poison_type, 1), ch);
    //    poison->uses -= 1;
    //  }

    //  for (poison = obj->poison; poison; poison = poison->next)
    //    if (poison->uses <= 0)
    //      remove_object_poison(obj, poison);
    //}
    //else if (!size)
    //{
    //  act("You use $p to spray a fine mist directly at $N.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  act("$n uses $p to spray a fine mist directly at $N.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
    //  act("$n uses $p to spray a fine mist directly at you.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);

    //  for (poison = obj->poison; poison; poison = poison->next)
    //  {
    //    soma_add_affect(tch, poison->poison_type, poison->duration, poison->latency,
    //  		   poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
    //  		   poison->attack, poison->decay, poison->sustain, poison->release);
    //    //send_to_char(lookup_poison_variable(poison->poison_type, 1), ch);
    //    poison->uses -= 1;
    //  }

    //  for (poison = obj->poison; poison; poison = poison->next)
    //    if (poison->uses <= 0)
    //      remove_object_poison(obj, poison);
    //}
    //else
    //{
    //  act("You use $p to spray a fine mist over $N and those in $S group.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  act("$n uses $p to spray a fine mist over $N and those in $S group.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
    //  act("$n uses $p to spray a fine mist over you and those in your group.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);

    //  for (poison = obj->poison; poison; poison = poison->next)
    //  {
    //    power = poison->uses;

    //    if (number(0,10) <= power)
    //    {
    //      soma_add_affect(tch, poison->poison_type, poison->duration, poison->latency,
    //  		   poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
    //  		   poison->attack, poison->decay, poison->sustain, poison->release);
    //    }

    //    for (sch = ch->room->people; sch; sch = sch->next_in_room)
    //    {

    //      if (sch == tch)
    //        continue;

    //      if (sch == ch)
    //        continue;

    //      if (!are_grouped(sch, tch))
    //        continue;

    //      if ((i <= power) && (number(0,10) <= power))
    //      {
    //        soma_add_affect(tch, poison->poison_type, poison->duration, poison->latency,
    //  		   poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
    //  		   poison->attack, poison->decay, poison->sustain, poison->release);
    //        i++;
    //      }
    //    }
    //    //send_to_char(lookup_poison_variable(poison->poison_type, 1), ch);
    //    poison->uses -= 1;
    //  }

    //  for (poison = obj->poison; poison; poison = poison->next)
    //    if (poison->uses <= 0)
    //      remove_object_poison(obj, poison);
    //}
}

void
do_inject (CHAR_DATA *ch, char *argument, int cmd)
{
    //char arg[MAX_STRING_LENGTH] = { '\0' };
    //char buf[MAX_STRING_LENGTH] = { '\0' };

    //OBJ_DATA *obj = NULL;
    //CHAR_DATA *tch = NULL;


    //argument = one_argument(argument, arg);

    //if (!*arg)
    //{
    //  send_to_char("Usage: inject <target>\n", ch);
    //  return;
    //}

    //if (!(tch = get_char_room_vis(ch, arg)))
    //{
    //  sprintf(buf, "You don't see #5%s#0 here.\n", arg);
    //  send_to_char(buf, ch);
    //  return;
    //}

    //if (!(((obj = ch->right_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE)
    //    || ((obj = ch->left_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE)))
    //  {
    //    send_to_char("You need to be holding some type of injection device to do that.\n", ch);
    //    return;
    //  }

    //if (!obj->poison)
    //{
    //  act("$p does not contain any substance that could be injected in to someone.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (tch == ch)
    //{
    //  if (ch->fighting)
    //  {
    //    act("Fending off the blows, you somehow manage to prepare to inject yourself with $p.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //    act("$n prepares to inject $mself with $p.",false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_COMBAT);
    //    ch->delay = 10;
    //    ch->delay_type = DEL_CHEM_INJECT;
    //    ch->delay_ch = ch;
    //    return;
    //  }
    //  else
    //  {
    //    act("You steel yourself as you position $p by a vein.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //    act("$n prepares to inject $mself with $p.",false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
    //    ch->delay = 5;
    //    ch->delay_type = DEL_CHEM_INJECT;
    //    ch->delay_ch = ch;
    //    return;
    //  }
    //}
    //else if (get_affect (ch, MAGIC_HIDDEN))
    //{
    //  act("You position yourself to stealthily inject $N with $p.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  ch->delay = 10;
    //  ch->delay_type = DEL_CHEM_INJECT;
    //  ch->delay_ch = tch;
    //  ch->delay_info1 = 2;
    //  return;
    //}
    //else if (GET_POS(tch) <= POSITION_SITTING)
    //{
    //  act("You position yourself to inject $N with $p.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  act("$n prepares to inject $N with $p.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
    //  act("$n prepares to inject you with $p.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
    //  ch->delay = 10;
    //  ch->delay_type = DEL_CHEM_INJECT;
    //  ch->delay_ch = tch;
    //  ch->delay_info1 = 1;
    //  return;
    //}
    //else if (GET_POS(tch) >= POSITION_FIGHTING)
    //{
    //  if (ch->fighting != tch)
    //  {
    //    act("You'll need to engage $N in combat for a chance to inject $M with $p.", false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else
    //  {
    //    act("You position yourself to inject $N with $p.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //    act("$n prepares to inject $N with $p.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
    //    act("$n prepares to inject you with $p.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
    //    ch->delay = 10;
    //    ch->delay_type = DEL_CHEM_INJECT;
    //    ch->delay_ch = tch;
    //    return;
    //  }
    //}
}

void
delayed_inject (CHAR_DATA * ch)
{
    //OBJ_DATA *obj;
    //CHAR_DATA *tch;
    //POISON_DATA *poison = NULL;
    //bool pass = false;

    //if (!(((obj = ch->right_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE)
    //    || ((obj = ch->left_hand) && GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE)))
    //{
    //  send_to_char("You need to be holding some type of injection device to do that.\n", ch);
    //  return;
    //}

    //if (!obj->poison)
    //{
    //  act("$p does not contain any substance that could be injected in to someone.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (!ch->delay_ch || !(tch = ch->delay_ch) || !(is_in_room(ch, tch)))
    //{
    //  send_to_char("Your target is no longer here.\n", ch);
    //  return;
    //}

    //if (tch == ch)
    //{
    //  act("You inject $p in to yourself.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //  act("$n injects $mself with $p.",false, ch, obj, 0, TO_ROOM | _ACT_FORMAT | _ACT_COMBAT);
    //  pass = true;
    //}
    //else if (get_affect (ch, MAGIC_HIDDEN) && ch->delay_info1 == 2)
    //{
    //  remove_affect_type (ch, MAGIC_HIDDEN);
    //  act("You emerge from hiding to inject $p in to $N.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  act("$n emerges from hiding to injects $N with $p.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
    //  act("$n emerges from hiding to injects you with $p.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
    //  pass = true;
    //}
    //else if (GET_POS(tch) <= POSITION_SITTING)
    //{
    //  act("You inject $p in to $N.",false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //  act("$n injects $N with $p.",false, ch, obj, tch, TO_NOTVICT | _ACT_FORMAT);
    //  act("$n injects you with $p.",false, ch, obj, tch, TO_VICT | _ACT_FORMAT);
    //  pass = true;
    //}
    //else if (GET_POS(tch) >= POSITION_FIGHTING)
    //{
    //  if (ch->delay_info1 == 1)
    //  {
    //    act("$N is no longer in a vulnerable position: you will now need to engage $M in combat for a chance to inject $M with $p.",
    //         false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else if (ch->fighting != tch)
    //  {
    //    act("You'll need to engage $N in combat for a chance to inject $M with $p.", false, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else
    //  {
    //    act("You inject $p in to $N.",false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
    //    act("$n injects $N with $p.",false, ch, obj, 0, TO_NOTVICT | _ACT_FORMAT | _ACT_COMBAT);
    //    act("$n injects with $p.",false, ch, obj, 0, TO_VICT | _ACT_FORMAT);
    //    pass = true;
    //  }
    //}

    //// If we passed, inject every poison on the needle in to the poor sap, and then remove them all from the object.

    //if (pass)
    //{
    //  for (poison = obj->poison; poison; poison = poison->next)
    //  {
    //    soma_add_affect(tch, poison->poison_type, poison->duration, poison->latency,
    //  		   poison->minute, poison->max_power, poison->lvl_power, poison->atm_power,
    //  		   poison->attack, poison->decay, poison->sustain, poison->release);
    //    //send_to_char(lookup_poison_variable(poison->poison_type, 1), ch);
    //  }

    //  for (poison = obj->poison; poison; poison = poison->next)
    //    remove_object_poison(obj, poison);

    //  obj->o.od.value[1] = 0;
    //  obj->o.od.value[2] = 0;
    //  obj->o.od.value[3] = 0;
    //}
}

void
do_mix (CHAR_DATA * ch, char *argument, int cmd)
{
    //char buf[MAX_STRING_LENGTH] = { '\0' };
    //char buf4[MAX_STRING_LENGTH] = { '\0' };
    //char buf5[MAX_STRING_LENGTH] = { '\0' };
    //char arg2[MAX_STRING_LENGTH] = { '\0' };
    //char arg3[MAX_STRING_LENGTH] = { '\0' };
    //char arg4[MAX_STRING_LENGTH] = { '\0' };
    //char arg5[MAX_STRING_LENGTH] = { '\0' };
    //char arg6[MAX_STRING_LENGTH] = { '\0' };
    //OBJ_DATA *kit = NULL;
    //OBJ_DATA *receptacle = NULL;
    //OBJ_DATA *vial_one = NULL;
    //OBJ_DATA *vial_two = NULL;
    //OBJ_DATA *vial_three = NULL;

    //int skill = 0;
    //int kit_use = 0;
    //int cost = 1;
    //int one_con = 0;
    //int two_con = 0;
    //int three_con = 0;
    //int avg_con = 0;
    //int recep_type = 0;
    //int diff = 0;
    //int roll = 0;

    //bool concentrate = false;

    //argument = one_argument(argument, arg5);

    //if (!*arg5)
    //{
    //  act ("Usage: mix <kit> <purify|combine> [<receptacle>] [<vials 1, 2 and 3>]", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //skill_learn(ch, SKILL_CHEMISTRY);

    //if (!(((kit = ch->right_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)
    //    || ((kit = ch->left_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)))
    //  {
    //    send_to_char("You need to be holding some sort of chemistry kit to mix chemicals.\n", ch);
    //    return;
    //  }

    ////argument = one_argument(argument, arg5);

    //if (!str_cmp(arg5, "combine"))
    //{
    //  argument = one_argument(argument, arg6);
    //  concentrate = false;
    //  if (!(receptacle = get_obj_in_list_vis (ch, arg6, ch->room->contents)))
    //  {
    //    sprintf (buf, "You don't see #2%s#0 in the room here.", arg2);
    //    act (buf, false, ch, 0, 0, TO_CHAR);
    //    return;
    //  }
    //}
    //else if (!str_cmp(arg5, "purify"))
    //  concentrate = true;
    //else
    //{
    //  act ("You must decide whether to #6combine#0 or #6purify#0 the vials.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (receptacle && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_NEEDLE
    //               && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_SPRAY
    //               && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_BOX)
    //{
    //  act ("$p must be something capable of holding the chemical compound: either a needle, a dispenser, or a flask.", false, ch, receptacle, 0, TO_CHAR);
    //  return;
    //}

    //if (receptacle && receptacle->poison)
    //{
    //  act ("$p already holds a compound - you must empty it first.", false, ch, receptacle, 0, TO_CHAR);
    //  return;
    //}

    //argument = one_argument(argument, arg2);

    //if (!*arg2)
    //{
    //  act ("Usage: mix <kit> <purify|combine> [<receptacle>] [<vials 1, 2 and 3>]", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}
    //else if (!(vial_one = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
    //{
    //  sprintf (buf, "You don't see #2%s#0 in the room here.", arg2);
    //  act (buf, false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_one && GET_ITEM_TYPE(vial_one) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //argument = one_argument(argument, arg3);

    //if (!*arg3)
    //  ;
    //else if (!(vial_two = get_obj_in_list_vis (ch, arg3, ch->room->contents)))
    //{
    //  sprintf (buf, "You don't see #2%s#0 in the room here.", arg3);
    //  act (buf, false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_two && GET_ITEM_TYPE(vial_two) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_two, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //argument = one_argument(argument, arg4);

    //if (!*arg4)
    //  ;
    //else if (!(vial_three = get_obj_in_list_vis (ch, arg4, ch->room->contents)))
    //{
    //  sprintf (buf, "You don't see #2%s#0 in the room here.", arg4);
    //  act (buf, false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_three && GET_ITEM_TYPE(vial_three) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_three, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}


    //if (vial_one)
    //{
    //  one_con = (vial_one->o.od.value[0] ? vial_one->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (vial_two)
    //{
    //  if (vial_one == vial_two)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  two_con = (vial_two->o.od.value[0] ? vial_two->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (vial_three)
    //{
    //  if (vial_three == vial_one)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  if (vial_two == vial_three)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_three, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  three_con = (vial_three->o.od.value[0] ? vial_three->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (concentrate)
    //{
    //  if (kit && vial_one && vial_two && !vial_three)
    //  {
    //    cost = 1;
    //    kit_use = kit->o.od.value[0];

    //    if (kit_use == 0)
    //    {
    //      act ("$p is out of supplies, and can't be used to purify or combine any more chemicals.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }

    //    if (vial_one->var_color[0] && vial_two->var_color[0] && !(str_cmp(vial_one->var_color[0], vial_two->var_color[0])))
    //    {
    //      if (one_con >= 10 && two_con < 10)
    //      {
    //        act ("Mixing $p and $P will have no effect: as concentrated as $p is, only by mixing another dangerously-concentrated vial will you "
    //             "be able to strengthen the chemical compound.", false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        return;
    //      }

    //      if (two_con >= 10 && one_con < 10)
    //      {
    //        act ("Mixing $p and $P will have no effect: as concentrated as $p is, only by mixing another dangerously-concentrated vial will you "
    //             "be able to strengthen the chemical compound.", false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        return;
    //      }

    //      one_con += two_con;

    //      if (one_con == 20)
    //        one_con = 13;
    //      else if (one_con > 10)
    //        one_con = 10;

    //      sprintf(buf, "Using #2%s#0, you start the complicated process of mixing $p and $P to produce a more concentrated compound...", obj_short_desc(kit));
    //      sprintf(buf5, "Using #2%s#0, $n begins mixing $p and $P.", obj_short_desc(kit));
    //      act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //      act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);

    //      ch->delay_type = DEL_CHEM_CONCENTRATE_MIX;
    //      ch->delay = 8;
    //      ch->delay_info1 = (long int) vial_one;
    //      ch->delay_info2 = (long int) vial_two;
    //      return;
    //    }
    //    else
    //    {
    //      act ("When concentrating, you need to nominate two vials of an identical substance.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }
    //  }
    //  else
    //  {
    //    act ("When concentrating, you need to nominate two, and only two, vials.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //}
    //else if (kit && vial_one && vial_two && receptacle)
    //{
    //  // We work out the average concentration of our three substances.

    //  if(three_con)
    //    avg_con = (one_con + two_con + three_con) / 3;
    //  else if (two_con)
    //    avg_con = (one_con + two_con) / 2;
    //  else
    //    avg_con = one_con;

    //  // Cost goes up by the total concentration, so 3 max-concentration vials will cost you a total of 7 points,
    //  // while 1 lowest-concentration vial will cost you 2 points.

    //  if (avg_con >= 9)
    //    cost += 3;
    //  else if (avg_con >= 6)
    //    cost += 2;
    //  else if (avg_con >= 3)
    //    cost += 1;

    //  // We determine the "difficulty" of a particular combination by subtracting 2 from he cost, and
    //  // multiplying by 10, with a minimum score of 10, so you'll be making a dice roll of your skill
    //  // against a difficulty of 10 for the easiest and 50 for the hardest (if you're trying to combine
    //  // three highly concentrated chemicals).

    //  diff = cost - 2;
    //  if (diff < 1)
    //    diff = 1;

    //  diff = diff * 10;

    //  skill = skill_level(ch, SKILL_CHEMISTRY, 0);

    //  roll = number(1, skill);

    //  kit_use = kit->o.od.value[0];

    //  if (kit_use == 0)
    //  {
    //    act ("$p is out of supplies, and can't be used to purify or combine any more chemicals.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else if (cost > kit_use)
    //  {
    //    act ("$p doesn't have enough supplies to purify those two compounds.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  // Different type of kits get slightly different power, time, and use values, to reflect the differing nature of the products in
    //  // question. Needles are the most powerful but one-shot only, chem_sprays the least powerful but most shots.

    //  switch (GET_ITEM_TYPE(receptacle))
    //  {
    //    case ITEM_CHEM_NEEDLE:
    //      recep_type = 1;
    //      break;
    //    case ITEM_CHEM_BOX:
    //      recep_type = 2;
    //      break;
    //    case ITEM_CHEM_SPRAY:
    //      recep_type = 3;
    //      break;
    //  }

    //  if(vial_three)
    //  {
    //    if (!str_cmp(vial_one->var_color[0], vial_three->var_color[0]))
    //    {
    //      act ("You can't combine $p and $P: you need to combine two different chemicals.", false, ch, vial_one, vial_three, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }
    //    if (!str_cmp(vial_two->var_color[0], vial_three->var_color[0]))
    //    {
    //      act ("You can't combine $p and $P: you need to combine two different chemicals.", false, ch, vial_two, vial_three, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }

    //    sprintf(buf4, "Using #2%s#0, you start preparing #2%s#0, #2%s#0 and #2%s#0 to be mixed and placed in to #2%s#0. . .",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(vial_three), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n begins to mix #2%s#0, #2%s#0 and #2%s#0.",
    //                   obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(vial_three));
    //  }
    //  else if(vial_two)
    //  {
    //    if (!str_cmp(vial_one->var_color[0], vial_two->var_color[0]))
    //    {
    //      act ("You can't combine $p and $P: you need to combine two different chemicals.", false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }
    //    sprintf(buf4, "Using #2%s#0, you start preparing #2%s#0 and #2%s#0 to be mixed and placed in to to #2%s#0. . .",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n begins to mix #2%s#0 and #2%s#0.",
    //                   obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two));
    //  }
    //  else
    //  {
    //    sprintf(buf4, "Using #2%s#0, you start preparing #2%s#0 to be placed in to #2%s#0. . .",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n begins to mix #2%s#0.",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one));
    //  }

    //  act (buf4, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  act (buf5, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    //  ch->delay_type = DEL_CHEM_COMBINE_MIX;
    //  ch->delay = 16;
    //  ch->delay_obj = receptacle;

    //  if (vial_one)
    //    ch->delay_info1 = (long int) vial_one;
    //  if (vial_two)
    //    ch->delay_info2 = (long int) vial_two;
    //  if (vial_three)
    //    ch->delay_info3 = (long int) vial_three;

    //  return;
    //}
    //else
    //{
    //  act ("You need to nominate at least two vials to mix.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}
}

void delayed_chem_concentrate_mix (CHAR_DATA *ch)
{
    //char buf[MAX_STRING_LENGTH] = { '\0' };
    //char buf2[MAX_STRING_LENGTH] = { '\0' };
    //char buf3[MAX_STRING_LENGTH] = { '\0' };
    //char buf4[MAX_STRING_LENGTH] = { '\0' };
    //char buf5[MAX_STRING_LENGTH] = { '\0' };
    //OBJ_DATA *kit = NULL;
    //OBJ_DATA *vial_one = NULL;
    //OBJ_DATA *vial_two = NULL;
    //OBJ_DATA *obj = NULL;

    //int skill = 0;
    //int kit_use = 0;
    //int cost = 1;
    //int one_con = 0;
    //int two_con = 0;
    //int diff = 0;
    //int roll = 0;
    //int i = 0;
    //int j = 0;


    //skill_learn(ch, SKILL_CHEMISTRY);

    //if (!(((kit = ch->right_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)
    //    || ((kit = ch->left_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)))
    //  {
    //    send_to_char("You need to be holding some sort of chemistry kit to mix chemicals.\n", ch);
    //    return;
    //  }

    //if (!(vial_one = (OBJ_DATA *) ch->delay_info1))
    //{
    //  act ("One of the vials you were mixing with is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_one && GET_ITEM_TYPE(vial_one) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (!(vial_two = (OBJ_DATA *) ch->delay_info2))
    //{
    //  act ("One of the vials you were mixing with is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_two && GET_ITEM_TYPE(vial_two) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_two, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (vial_one)
    //{
    //  one_con = (vial_one->o.od.value[0] ? vial_one->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (vial_two)
    //{
    //  if (vial_one == vial_two)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  two_con = (vial_two->o.od.value[0] ? vial_two->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (kit && vial_one && vial_two)
    //  {
    //    cost = 1;
    //    kit_use = kit->o.od.value[0];

    //    if (kit_use == 0)
    //    {
    //      act ("$p is out of supplies, and can't be used to purify or combine any more chemicals.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }

    //    if (vial_one->var_color[0] && vial_two->var_color[0] && !(str_cmp(vial_one->var_color[0], vial_two->var_color[0])))
    //    {
    //      if (one_con >= 10 && two_con < 10)
    //      {
    //        act ("Mixing $p and $P will have no effect: as concentrated as $p is, only by mixing another dangerously-concentrated vial will you "
    //             "be able to strengthen the chemical compound.", false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        return;
    //      }

    //      if (two_con >= 10 && one_con < 10)
    //      {
    //        act ("Mixing $p and $P will have no effect: as concentrated as $p is, only by mixing another dangerously-concentrated vial will you "
    //             "be able to strengthen the chemical compound.", false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        return;
    //      }

    //      kit->o.od.value[0] -= 1;

    //      one_con += two_con;

    //      if (one_con == 20)
    //        one_con = 13;
    //      else if (one_con > 10)
    //        one_con = 10;

    //      // Fairly easy test: minimum of 4 to a maximum of 40, noting that you'd never really roll more than 20.
    //      diff = (one_con + two_con) * 2;

    //      skill = skill_level(ch, SKILL_CHEMISTRY, 0);

    //      roll = number(1, skill);

    //      // 4 levels of failure:
    //      // critical - lose both vials, the kit, and take some damage
    //      // large - lose both vials
    //      // medium - lose one vial
    //      // small - lose the cost of components

    //      if (diff > roll + 15)
    //      {
    //        sprintf(buf, "As you add and subtract various compounds and solutions from #2%s#0 in an attempt to purify $p and $P, "
    //                     "a rapid crackling and bubbling occurs from the various beakers and test tubes of the chemistry kit. "
    //  	       "Before you can react, the resulting dangerous chemical reaction sprays a noxious solution across your body, taking with it "
    //                     "the kit and both vials and leaving only a smoking smear!", obj_short_desc(kit));
    //        sprintf(buf5, "As $n uses #2%s#0 to purify $p and $P, "
    //                     "a rapid crackling and bubbling occurs from the varoious beakers and testubes of the chemistry kit. "
    //  	       "Before $e can react, the resulting dangerous chemical reaction sprays a noxious solution across $s body, taking with it "
    //                     "the kit and both vials and leaving only a smoking smear!", obj_short_desc(kit));
    //        act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);

    //        if (vtoo(VNUM_CHEM_SMEAR))
    //          obj_to_room(load_object (VNUM_CHEM_SMEAR), ch->in_room);

    //        if (vial_one)
    //        {
    //          obj_from_room(&vial_one, ch->in_room);
    //          extract_obj(vial_one);
    //        }
    //        if (vial_two)
    //        {
    //          obj_from_room(&vial_two, ch->in_room);
    //          extract_obj(vial_two);
    //        }
    //        if (kit)
    //        {
    //          obj_from_char(&kit, ch->in_room);
    //          extract_obj(kit);
    //        }

    //        j = number (0,10);

    //        for (i = 0; i < j; i++)
    //        {
    //          obj = get_equip (ch, number(0,4));
    //          if (obj)
    //            object__add_damage (obj, number(5,6), number(1,3));
    //          wound_to_char (ch, figure_location (ch, number(0,9)), number(1,3), number(5,6), number(0,1), 0, number(0,1));
    //        }
    //      }
    //      else if (diff > roll + 10)
    //      {
    //        sprintf(buf, "As you add and subtract various compounds and solutions from #2%s#0 in an attempt to purify $p and $P, "
    //                     "a rapid bubbling and sudden wave of heat emanates from the various beakers and test tubes of the chemistry kit. "
    //  	       "Reacting as quick as you can, you manage to save the chemistry kit, but both vials are left with naught by a "
    //                     "greasy black residue within.", obj_short_desc(kit));
    //        sprintf(buf5, "As $n uses #2%s#0 to purify $p and $P, "
    //                     "a rapid bubbling and sudden wave of heat emanates from the various beakers and testubes of the chemistry kit. "
    //  	       "Reacting quickly, $e manages to save the chemistry kit, but both vials are left with naught by a "
    //                     "greasy black residue within.", obj_short_desc(kit));
    //        act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);

    //        if (vtoo(VNUM_CHEM_FAILED_VIAL))
    //        {
    //          obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);
    //          obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);
    //        }

    //        if (vial_one)
    //        {
    //          obj_from_room(&vial_one, ch->in_room);
    //          extract_obj(vial_one);
    //        }
    //        if (vial_two)
    //        {
    //          obj_from_room(&vial_two, ch->in_room);
    //          extract_obj(vial_two);
    //        }
    //      }
    //      else if (diff > roll + 5)
    //      {
    //        sprintf(buf, "As you add and subtract various compounds and solutions from #2%s#0 in an attempt to purify $p and $P, "
    //  	       "a dangerous foaming spreads from the various beakers and test tubes of the chemistry kit. "
    //                     "Reacting quickly, you manage to recover $p from the resulting chemical reaction, but $P is rendered useless, "
    //                     "only a greasy black residue remaining within the vial.", obj_short_desc(kit));
    //        sprintf(buf5, "As $n uses #2%s#0 to purify $p and $P, "
    //  	       "a dangerous foaming spreads from the various beakers and test tubes of the chemistry kit. "
    //                     "Reacting quickly, $e manage to recover $p from the resulting chemical reaction, but $P is rendered useless, "
    //                     "only a greasy black residue remaining within the vial.", obj_short_desc(kit));
    //        act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);

    //        if (vtoo(VNUM_CHEM_FAILED_VIAL))
    //          obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);

    //        if (vial_two)
    //        {
    //          obj_from_room(&vial_two, ch->in_room);
    //          extract_obj(vial_two);
    //        }
    //      }
    //      else if (diff > roll)
    //      {
    //        sprintf(buf, "After adding and subtracting various compounds and solutions from #2%s#0, nothing seems to happen: both $p and $P are as they always were.", obj_short_desc(kit));
    //        sprintf(buf5, "Using #2%s#0, $n mixes $p and $P.", obj_short_desc(kit));
    //        act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);
    //      }
    //      else
    //      {
    //        sprintf(buf5, "%s", (one_con >= 11 ? "ingeniously" : one_con == 10 ? "dangerously" : one_con == 9 ? "extremely"
    //        : one_con == 8 ? "strongly" : one_con == 7 ? "highly" : one_con == 6 ? "densely"
    //        : one_con == 5 ? "moderately" : one_con == 4 ? "somewhat" : one_con == 3 ? "slightly"
    //        : one_con == 2 ? "weakly" : "barely"));

    //        sprintf(buf2, "a %s-concentrated vial of %s", buf5, vial_one->var_color[0]);
    //        sprintf(buf3, "A %s-concentrated vial of %s lies here.", buf5, vial_one->var_color[0]);
    //        sprintf(buf4, "vial %s %s OTHER", buf5, vial_one->var_color[0]);

    //        sprintf(buf, "After adding and subtracting various compounds and solutions from #2%s#0, you mix $p and $P, producing #2%s#0.", obj_short_desc(kit), buf2);
    //        sprintf(buf5, "Using #2%s#0, $n mixes $p and $P, producing #2%s#0.", obj_short_desc(kit), buf2);

    //        act (buf, false, ch, vial_one, vial_two, TO_CHAR | _ACT_FORMAT);
    //        act (buf5, false, ch, vial_one, vial_two, TO_ROOM | _ACT_FORMAT);

    //        if (vial_two)
    //        {
    //          obj_from_room(&vial_two, ch->in_room);
    //          extract_obj(vial_two);
    //        }
    //        if (vtoo(VNUM_CHEM_EMPTY_VIAL))
    //          obj_to_room(load_object (VNUM_CHEM_EMPTY_VIAL), ch->in_room);
    //        vial_one->o.od.value[0] = one_con;
    //        vial_one->short_description = add_hash(buf2);
    //        vial_one->description = add_hash(buf3);
    //        vial_one->name = add_hash(buf4);
    //      }

    //      skill_use(ch, SKILL_CHEMISTRY, diff);

    //      return;
    //    }
    //    else
    //    {
    //      act ("When concentrating, you need to nominate two vials of an identical substance.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //      return;
    //    }
    //  }
    //else
    //{
    //  act ("When concentrating, you need to nominate two, and only two, vials.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}
}

void delayed_chem_combine_mix (CHAR_DATA *ch)
{
    //char buf4[MAX_STRING_LENGTH] = { '\0' };
    //char buf5[MAX_STRING_LENGTH] = { '\0' };
    //OBJ_DATA *kit = NULL;
    //OBJ_DATA *receptacle = NULL;
    //OBJ_DATA *vial_one = NULL;
    //OBJ_DATA *vial_two = NULL;
    //OBJ_DATA *vial_three = NULL;
    //OBJ_DATA *obj = NULL;

    //POISON_DATA *chem_one = NULL;
    //POISON_DATA *chem_two = NULL;
    //POISON_DATA *chem_three = NULL;

    //int ind = 0;
    //int skill = 0;
    //int kit_use = 0;
    //int cost = 1;
    //int one_con = 0;
    //int two_con = 0;
    //int three_con = 0;
    //int avg_con = 0;
    //int recep_type = 0;
    //int diff = 0;
    //int roll = 0;
    //int i = 0;
    //int j = 0;

    //bool first_null = false;
    //bool sec_null = false;
    //bool third_null = false;


    //skill_learn(ch, SKILL_CHEMISTRY);

    //if (!(((kit = ch->right_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)
    //    || ((kit = ch->left_hand) && GET_ITEM_TYPE (kit) == ITEM_CHEM_KIT)))
    //  {
    //    send_to_char("You need to be holding some sort of chemistry kit to mix chemicals.\n", ch);
    //    return;
    //  }

    //if (!(receptacle = ch->delay_obj))
    //{
    //  act ("The receptacle you were using is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (receptacle && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_NEEDLE
    //               && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_SPRAY
    //               && GET_ITEM_TYPE(receptacle) != ITEM_CHEM_BOX)
    //{
    //  act ("$p must be something capable of holding the chemical compound: either a needle, a dispenser, or a flask.", false, ch, receptacle, 0, TO_CHAR);
    //  return;
    //}

    //if (receptacle && receptacle->poison)
    //{
    //  act ("$p already holds a compound - you must empty it first.", false, ch, receptacle, 0, TO_CHAR);
    //  return;
    //}

    //if (ch->delay_info1 && !(vial_one = (OBJ_DATA *) ch->delay_info1))
    //{
    //  act ("One of the vials you were mixing with is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_one && GET_ITEM_TYPE(vial_one) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (ch->delay_info2 && !(vial_two = (OBJ_DATA *) ch->delay_info2))
    //{
    //  act ("One of the vials you were mixing with is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_two && GET_ITEM_TYPE(vial_two) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_two, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}

    //if (ch->delay_info3 && !(vial_three = (OBJ_DATA *) ch->delay_info3))
    //{
    //  act ("One of the vials you were mixing with is no longer present.", false, ch, 0, 0, TO_CHAR);
    //  return;
    //}

    //if (vial_three && GET_ITEM_TYPE(vial_three) != ITEM_CHEM_VIAL)
    //{
    //  act ("$p isn't something you can mix chemically.", false, ch, vial_three, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}


    //if (vial_one)
    //{
    //  one_con = (vial_one->o.od.value[0] ? vial_one->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (vial_two)
    //{
    //  if (vial_one == vial_two)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  two_con = (vial_two->o.od.value[0] ? vial_two->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (vial_three)
    //{
    //  if (vial_three == vial_one)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_one, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  if (vial_two == vial_three)
    //  {
    //    act ("You can't mix $p with itself!", false, ch, vial_three, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }

    //  three_con = (vial_three->o.od.value[0] ? vial_three->o.od.value[0] : 1);
    //  cost += 1;
    //}

    //if (kit && vial_one && receptacle)
    //{
    //  // We work out the average concentration of our three substances.

    //  if(three_con)
    //    avg_con = (one_con + two_con + three_con) / 3;
    //  else if (two_con)
    //    avg_con = (one_con + two_con) / 2;
    //  else
    //    avg_con = one_con;

    //  // Cost goes up by the total concentration, so 3 max-concentration vials will cost you a total of 7 points,
    //  // while 1 lowest-concentration vial will cost you 2 points.

    //  if (avg_con >= 9)
    //    cost += 3;
    //  else if (avg_con >= 6)
    //    cost += 2;
    //  else if (avg_con >= 3)
    //    cost += 1;

    //  // We determine the "difficulty" of a particular combination by subtracting 3 from he cost, and
    //  // multiplying by 10, with a minimum score of 5, so you'll be making a dice roll of your skill
    //  // against a difficulty of 5 for the easiest and 40 for the hardest (if you're trying to combine
    //  // three highly concentrated chemicals).

    //  diff = cost - 3;

    //  diff = diff * 10;

    //  if (diff < 0)
    //    diff = 5;

    //  skill = skill_level(ch, SKILL_CHEMISTRY, 0);

    //  roll = number(1, skill);

    //  kit_use = kit->o.od.value[0];

    //  if (kit_use == 0)
    //  {
    //    act ("$p is out of supplies, and can't be used to purify or combine any more chemicals.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else if (cost > kit_use)
    //  {
    //    act ("$p doesn't have enough supplies to purify those two compounds.", false, ch, kit, 0, TO_CHAR | _ACT_FORMAT);
    //    return;
    //  }
    //  else
    //    kit->o.od.value[0] -= cost;


    //  // Different type of kits get slightly different power, time, and use values, to reflect the differing nature of the products in
    //  // question. Needles are the most powerful but one-shot only, chem_sprays the least powerful but most shots.

    //  switch (GET_ITEM_TYPE(receptacle))
    //  {
    //    case ITEM_CHEM_NEEDLE:
    //      recep_type = 1;
    //      break;
    //    case ITEM_CHEM_BOX:
    //      recep_type = 2;
    //      break;
    //    case ITEM_CHEM_SPRAY:
    //      recep_type = 3;
    //      break;
    //  }

    //  if(vial_three)
    //  {
    //    sprintf(buf4, "After the careful addition and subtraction of various compounds and solutions from #2%s#0, you mix #2%s#0, #2%s#0 and #2%s#0 in to #2%s#0",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(vial_three), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n mixes #2%s#0, #2%s#0 and #2%s#0 in to #2%s#0",
    //                   obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(vial_three), obj_short_desc(receptacle));
    //  }
    //  else if(vial_two)
    //  {
    //    sprintf(buf4, "After the careful addition and subtraction of various compounds and solutions from #2%s#0, you mix #2%s#0 and #2%s#0 in to #2%s#0",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n mixes #2%s#0 and #2%s#0 in to #2%s#0",
    //                   obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(vial_two), obj_short_desc(receptacle));
    //  }
    //  else
    //  {
    //    sprintf(buf4, "After the careful addition and subtraction of various compounds and solutions from #2%s#0, you mix #2%s#0 in to #2%s#0",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(receptacle));
    //    sprintf(buf5, "Using #2%s#0, $n mixes #2%s#0 in to #2%s#0",
    //  	     obj_short_desc(kit), obj_short_desc(vial_one), obj_short_desc(receptacle));
    //  }

    //  if (diff > roll + 20)
    //  {
    //    sprintf(buf4 + strlen(buf4), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before you can react, the resulting dangerous chemical reaction burns your hands and arms, taking with it "
    //                              "the kit and the vial%s and leaving only a smoking smear!", (vial_two ? "s" : ""));
    //    sprintf(buf5 + strlen(buf5), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before $e can react, the resulting dangerous chemical reaction burns $s hands and arms, taking with it "
    //                              "the kit and the vial%s and leaving only a smoking smear!", (vial_two ? "s" : ""));
    //    if (vtoo(VNUM_CHEM_SMEAR))
    //      obj_to_room(load_object (VNUM_CHEM_SMEAR), ch->in_room);

    //    if (vial_one)
    //    {
    //      obj_from_room(&vial_one, ch->in_room);
    //      extract_obj(vial_one);
    //    }
    //    if (vial_two)
    //    {
    //      obj_from_room(&vial_two, ch->in_room);
    //      extract_obj(vial_two);
    //    }
    //    if (vial_three)
    //    {
    //      obj_from_room(&vial_three, ch->in_room);
    //      extract_obj(vial_three);
    //    }
    //    if (kit)
    //    {
    //      obj_from_char(&kit, ch->in_room);
    //      extract_obj(kit);
    //    }

    //    j = number (0,10);

    //    for (i = 0; i < j; i++)
    //    {
    //      obj = get_equip (ch, number(0,4));
    //      if (obj)
    //        object__add_damage (obj, number(5,6), number(1,3));
    //      wound_to_char (ch, figure_location (ch, number(0,14)), number(1,3), number(5,6), number(0,1), 0, number(0,1));
    //    }
    //  }
    //  else if (diff > roll + 15)
    //  {
    //    sprintf(buf4 + strlen(buf4), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before you can react, the resulting chemical reaction renders the mixture within %s vial%s useless "
    //                              "only a greasy black residue remaining within the glass tube%s.",
    //                              (vial_two ? "all the" : "the"), (vial_two ? "s" : ""), (vial_two ? "s" : ""));
    //    sprintf(buf5 + strlen(buf5), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before $e can react, the resulting chemical reaction renders the mixture within %s vial%s useless "
    //                              "only a greasy black residue remaining within the glass tube%s.",
    //                              (vial_two ? "all the" : "the"), (vial_two ? "s" : ""), (vial_two ? "s" : ""));

    //    if (vial_three && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);
    //    if (vial_two && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);
    //    if (vial_one && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);

    //    if (vial_one)
    //    {
    //      obj_from_room(&vial_one, ch->in_room);
    //      extract_obj(vial_one);
    //    }
    //    if (vial_two)
    //    {
    //      obj_from_room(&vial_two, ch->in_room);
    //      extract_obj(vial_two);
    //    }
    //    if (vial_three)
    //    {
    //      obj_from_room(&vial_three, ch->in_room);
    //      extract_obj(vial_three);
    //    }
    //  }
    //  else if (diff > roll + 10)
    //  {
    //    sprintf(buf4 + strlen(buf4), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before you can react, the resulting chemical reaction renders the mixture within %s vial%s useless "
    //                              "only a greasy black residue remaining within the glass tube%s.",
    //                              (vial_three ? "two of" : vial_two ? "all the" : "the"), (vial_two ? "s" : ""), (vial_two ? "s" : ""));
    //    sprintf(buf5 + strlen(buf5), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before $e can react, the resulting chemical reaction renders the mixture within %s vial%s useless "
    //                              "only a greasy black residue remaining within the glass tube%s.",
    //                              (vial_three ? "two of" : vial_two ? "all the" : "the"), (vial_two ? "s" : ""), (vial_two ? "s" : ""));

    //    if (vial_two && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);
    //    if (vial_one && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);

    //    if (vial_one)
    //    {
    //      obj_from_room(&vial_one, ch->in_room);
    //      extract_obj(vial_one);
    //    }
    //    if (vial_two)
    //    {
    //      obj_from_room(&vial_two, ch->in_room);
    //      extract_obj(vial_two);
    //    }
    //  }
    //  else if (diff > roll + 5)
    //  {
    //    sprintf(buf4 + strlen(buf4), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before you can react, the resulting chemical reaction renders the mixture within %s vial useless "
    //                              "only a greasy black residue remaining within the glass tube.",
    //                              (vial_two ? "one of" : "the"));
    //    sprintf(buf5 + strlen(buf5), " but almost immediately a rapid crackling and bubbling occurs from the kit. "
    //  	                "Before $e can react, the resulting chemical reaction renders the mixture within %s vial useless "
    //                              "only a greasy black residue remaining within the glass tube.",
    //                              (vial_two ? "one of" : "the"));

    //    if (vial_one && vtoo(VNUM_CHEM_FAILED_VIAL))
    //      obj_to_room(load_object (VNUM_CHEM_FAILED_VIAL), ch->in_room);

    //    if (vial_one)
    //    {
    //      obj_from_room(&vial_one, ch->in_room);
    //      extract_obj(vial_one);
    //    }
    //  }
    //  else if (diff > roll)
    //  {
    //    sprintf(buf4 + strlen(buf4), " but nothing seems to happen.");
    //  }
    //  else
    //  {
    //    if (vial_one && (ind = index_lookup (chemical_names, vial_one->var_color[0])) != -1)
    //    {
    //      // If we rolled a fake chemical, then make sure we've got the right index, number 11.
    //      receptacle->o.od.value[1] = ind;

    //      if (ind < 0 || ind > 18)
    //        ind = 0;

    //      one_con = vial_one->o.od.value[0];
    //      CREATE (chem_one, POISON_DATA, 1);
    //      chem_one->poison_type = ind + SOMA_FIRST + 11;
    //      chem_one->duration = one_con + 1;
    //      chem_one->latency = 0;
    //      chem_one->minute = 0;      receptacle->o.od.value[1] = ind;
    //      chem_one->max_power = ((one_con * 100)) / recep_type;
    //      chem_one->lvl_power = ((one_con * 100)/2) / recep_type;
    //      chem_one->atm_power = ((one_con * 100)/4) / recep_type;
    //      chem_one->attack = (((one_con * 10) + 20) * 1 / 4) / recep_type;
    //      chem_one->decay = (((one_con * 10) + 20) * 2 / 4) / recep_type;
    //      chem_one->sustain = (((one_con * 10) + 20) * 3 / 4) / recep_type;
    //      chem_one->release = (((one_con * 10) + 20)) / recep_type;
    //      chem_one->uses = (recep_type >= 3 ? avg_con * 2: recep_type == 2 ? avg_con : 1);
    //      chem_one->next = NULL;
    //      obj_from_room(&vial_one, ch->in_room);
    //      if (vtoo(VNUM_CHEM_EMPTY_VIAL))
    //        obj_to_room(load_object (VNUM_CHEM_EMPTY_VIAL), ch->in_room);
    //    }

    //    if (vial_two && (ind = index_lookup (chemical_names, vial_two->var_color[0])) != -1)
    //    {
    //      // If we rolled a fake chemical, then make sure we've got the right index, number 11.
    //      receptacle->o.od.value[2] = ind;

    //      if (ind < 0 || ind > 18)
    //        ind = 0;

    //      two_con = vial_two->o.od.value[0];
    //      CREATE (chem_two, POISON_DATA, 1);
    //      chem_two->poison_type = ind + MAGIC_FIRST_SOMA + 11;
    //      chem_two->duration = two_con + 1;
    //      chem_two->latency = 0;
    //      chem_two->minute = 0;
    //      chem_two->max_power = ((two_con * 100)) / recep_type;
    //      chem_two->lvl_power = ((two_con * 100)/2) / recep_type;
    //      chem_two->atm_power = ((two_con * 100)/4) / recep_type;
    //      chem_two->attack = (((two_con * 10) + 20) * 1 / 4) / recep_type;
    //      chem_two->decay = (((two_con * 10) + 20) * 2 / 4) / recep_type;
    //      chem_two->sustain = (((two_con * 10) + 20) * 3 / 4) / recep_type;
    //      chem_two->release = (((two_con * 10) + 20)) / recep_type;
    //      chem_two->uses = (recep_type >= 3 ? avg_con * 2: recep_type == 2 ? avg_con : 1);
    //      chem_two->next = NULL;
    //      if (vtoo(VNUM_CHEM_EMPTY_VIAL))
    //        obj_to_room(load_object (VNUM_CHEM_EMPTY_VIAL), ch->in_room);

    //      switch (chem_one->poison_type)
    //      {
    //        case SOMA_CHEM_DAMAGE:
    //          if (chem_two->poison_type == SOMA_CHEM_REPAIR)
    //          {
    //            first_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_DORMACY:
    //          if (chem_two->poison_type == SOMA_CHEM_CONFUSE)
    //          {
    //            first_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_SEPARATE:
    //          if (chem_two->poison_type == SOMA_CHEM_ACCELERATE)
    //          {
    //            first_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_INHIBIT:
    //          if (chem_two->poison_type == SOMA_CHEM_REPEL)
    //          {
    //            first_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_STIM_FURY:
    //          if (chem_two->poison_type == SOMA_STIM_RELAX)
    //          {
    //            first_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //      }
    //    }
    //    if (vial_three && (ind = index_lookup (chemical_names, vial_three->var_color[0])) != -1)
    //    {
    //      // If we rolled a fake chemical, then make sure we've got the right index, number 11.
    //      receptacle->o.od.value[3] = ind;

    //      if (ind < 0 || ind > 18)
    //        ind = 0;

    //      three_con = vial_three->o.od.value[0];
    //      CREATE (chem_three, POISON_DATA, 1);
    //      chem_three->poison_type = ind + MAGIC_FIRST_SOMA + 11;
    //      chem_three->duration = three_con + 1;
    //      chem_three->latency = 0;
    //      chem_three->minute = 0;
    //      chem_three->max_power = ((three_con * 100)) / recep_type;
    //      chem_three->lvl_power = ((three_con * 100)/2) / recep_type;
    //      chem_three->atm_power = ((three_con * 100)/4) / recep_type;
    //      chem_three->attack = (((three_con * 10) + 20) * 1 / 4) / recep_type;
    //      chem_three->decay = (((three_con * 10) + 20) * 2 / 4) / recep_type;
    //      chem_three->sustain = (((three_con * 10) + 20) * 3 / 4) / recep_type;
    //      chem_three->release = (((three_con * 10) + 20)) / recep_type;
    //      chem_three->uses = (recep_type >= 3 ? avg_con * 2: recep_type == 2 ? avg_con : 1);
    //      chem_three->next = NULL;
    //      if (vtoo(VNUM_CHEM_EMPTY_VIAL))
    //        obj_to_room(load_object (VNUM_CHEM_EMPTY_VIAL), ch->in_room);

    //      switch (chem_one->poison_type)
    //      {
    //        case SOMA_CHEM_DAMAGE:
    //          if (chem_three->poison_type == SOMA_CHEM_REPAIR)
    //          {
    //            first_null = true;
    //            third_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_DORMACY:
    //          if (chem_three->poison_type == SOMA_CHEM_CONFUSE)
    //          {
    //            first_null = true;
    //            third_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_SEPARATE:
    //          if (chem_three->poison_type == SOMA_CHEM_ACCELERATE)
    //          {
    //            first_null = true;
    //            third_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_INHIBIT:
    //          if (chem_three->poison_type == SOMA_CHEM_REPEL)
    //          {
    //            first_null = true;
    //            third_null = true;
    //          }
    //          break;
    //        case SOMA_STIM_FURY:
    //          if (chem_three->poison_type == SOMA_STIM_RELAX)
    //          {
    //            first_null = true;
    //            third_null = true;
    //          }
    //          break;
    //      }
    //      switch (chem_three->poison_type)
    //      {
    //        case SOMA_CHEM_DAMAGE:
    //          if (chem_two->poison_type == SOMA_CHEM_REPAIR)
    //          {
    //            third_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_DORMACY:
    //          if (chem_three->poison_type == SOMA_CHEM_CONFUSE)
    //          {
    //            third_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_SEPARATE:
    //          if (chem_three->poison_type == SOMA_CHEM_ACCELERATE)
    //          {
    //            third_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_CHEM_INHIBIT:
    //          if (chem_three->poison_type == SOMA_CHEM_REPEL)
    //          {
    //            third_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //        case SOMA_STIM_FURY:
    //          if (chem_three->poison_type == SOMA_STIM_RELAX)
    //          {
    //            third_null = true;
    //            sec_null = true;
    //          }
    //          break;
    //      }
    //    }

    //  if ((vial_one && !chem_one) || (vial_two && !chem_two) || (vial_three && !chem_three))
    //  {
    //    act ("An error has occured - please contact an admin.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  }

    //  if (vial_one)
    //  {
    //    obj_from_room(&vial_one, ch->in_room);
    //    extract_obj(vial_one);
    //  }
    //  if (vial_two)
    //  {
    //    obj_from_room(&vial_two, ch->in_room);
    //    extract_obj(vial_two);
    //  }
    //  if (vial_three)
    //  {
    //    obj_from_room(&vial_three, ch->in_room);
    //    extract_obj(vial_three);
    //  }

    //  // Null2 and Null6 has a special bonus = they make things make more powerful.
    //  if (chem_one)
    //  {
    //    if (chem_one->poison_type == SOMA_CHEM_NULL2 || chem_one->poison_type == SOMA_CHEM_NULL6)
    //    {
    //      if (chem_two)
    //      {
    //        chem_two->uses += 1;
    //        chem_two->max_power += 100;
    //        chem_two->lvl_power += 100;
    //        chem_two->atm_power += 100;
    //      }
    //      if (chem_three)
    //      {
    //        chem_three->uses += 1;
    //        chem_three->max_power += 100;
    //        chem_three->lvl_power += 100;
    //        chem_three->atm_power += 100;
    //      }
    //    }
    //  }
    //  if (chem_two)
    //  {
    //    if (chem_two->poison_type == SOMA_CHEM_NULL2 || chem_two->poison_type == SOMA_CHEM_NULL6)
    //    {
    //      if (chem_one)
    //      {
    //        chem_one->uses += 1;
    //        chem_one->max_power += 100;
    //        chem_one->lvl_power += 100;
    //        chem_one->atm_power += 100;
    //      }
    //      if (chem_three)
    //      {
    //        chem_three->uses += 1;
    //        chem_three->max_power += 100;
    //        chem_three->lvl_power += 100;
    //        chem_three->atm_power += 100;
    //      }
    //    }
    //  }
    //  if (chem_three)
    //  {
    //    if (chem_three->poison_type == SOMA_CHEM_NULL2 || chem_three->poison_type == SOMA_CHEM_NULL6)
    //    {
    //      if (chem_one)
    //      {
    //        chem_one->uses += 1;
    //        chem_one->max_power += 100;
    //        chem_one->lvl_power += 100;
    //        chem_one->atm_power += 100;
    //      }
    //      if (chem_two)
    //      {
    //        chem_two->uses += 1;
    //        chem_two->max_power += 100;
    //        chem_two->lvl_power += 100;
    //        chem_two->atm_power += 100;
    //      }
    //    }
    //  }

    //  if (chem_one)
    //  {
    //    if (!first_null)
    //      poison_to_obj(receptacle, chem_one);
    //  }

    //  if (chem_two)
    //  {
    //    if (!sec_null)
    //      poison_to_obj(receptacle, chem_two);
    //  }

    //  if (chem_three)
    //  {
    //    if (!third_null)
    //      poison_to_obj(receptacle, chem_three);
    //  }

    //  receptacle->o.od.value[4] = avg_con;
    //  receptacle->o.od.value[0] = (recep_type >= 3 ? avg_con * 2: recep_type == 2 ? avg_con : 1);
    //  sprintf(buf4 + strlen(buf4), ".");
    //  sprintf(buf5 + strlen(buf5), ".");
    //  }

    //  act (buf4, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  act (buf5, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    //  // You get two chancs of skill gain when combining.

    //  skill_use(ch, SKILL_CHEMISTRY, 0);

    //  return;
    //}
    //else
    //{
    //  act ("You need to nominate at least two vials to mix.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //  return;
    //}
}

void
do_apply (CHAR_DATA * ch, char *argument, int cmd)
{
    /*
    WOUND_DATA *wound;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int mode = 0;
    int poisoned = 0;
    int location_match = 0;
    AFFECTED_TYPE *soma;
    int type;

    argument = one_argument (argument, buf);

    // Find the salve or poison

    if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) && !(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
      {
        send_to_char ("You don't have that.\n\r", ch);
        return;
      }

    // Check that it's the right object type.

    if (obj->obj_flags.type_flag != ITEM_CHEM_BOX)
      {
        act ("$p isn't something from which you can apply a compound.", false, ch, obj, 0, TO_CHAR);
        return;
      }

    argument = one_argument (argument, buf);

        if (!str_cmp (buf, "to") || !str_cmp (buf, "on"))
      argument = one_argument (argument, buf);

        if (!(victim = get_char_vis (ch, buf)))
      {
        send_to_char ("You don't see them.\n\r", ch);
        return;
      }

        if (victim->fighting)
      {
        act ("You can't help $N while $E is fighting.",
             false, ch, 0, victim, TO_CHAR);
        return;
      }

      if (victim->delay)
      {
        act ("$N is too busy at the moment.",
        false, ch, 0, victim, TO_CHAR);
        return;
      }

    // They have to be resting, so in most cases, it has to be consensual.

    if (victim != ch && GET_POS (victim) >= SIT)
      {
        act ("$N must be laying down first.\n\r", false, ch, 0, victim,
         TO_CHAR);
        return;
      }

    if (victim == ch)
      mode = 1;

    one_argument (argument, arg);

    if (!*argument)
      {
        send_to_char ("Which location on the body are you treating?\n", ch);
        return;
      }

    // Search through their wounds - if we find the found and it is already poisoned,
    // and we're using a solve, make note of that.

    // todo:

    for (wound = victim->wounds; wound; wound = wound->next)
      {
        if (!str_cmp (wound->location, arg))
        {

          // Can't rub poisons in to bruises

          if (!str_cmp (wound->name, "bruise") || !str_cmp (wound->name, "abrasion")
            || !str_cmp (wound->name, "contusion")
            || !str_cmp (wound->name, "crush"))
            continue;

          // Likewise, small wounds are too small.

      if (!str_cmp (wound->severity, "small"))
            continue;

      location_match += 1;
          if (wound->poison > 0 && obj->obj_flags.type_flag != ITEM_POISON_PASTE)
          {
        poisoned += 1;
            type = wound->poison;
            break;
          }
          else if(obj->obj_flags.type_flag == ITEM_POISON_PASTE)
          {
            break;
          }
        }
      }

    if (!location_match)
      {
        if (mode)
      send_to_char ("You don't have a wound large enough in that area.\n", ch);
        else
      send_to_char ("Your patient does not have a wound large enough in that area.\n", ch);
        return;
      }

    if(!poisoned && obj->obj_flags.type_flag != ITEM_POISON_PASTE)
    {
      send_to_char("The wound isn't poisoned.\n", ch);
      return;
    }
    else if(poisoned && obj->obj_flags.type_flag == ITEM_POISON_PASTE &&
            !IS_SET(obj->obj_flags.extra_flags, ITEM_BENIGN))
    {
      send_to_char("The wound is already poisoned.\n", ch);
      return;
    }

    if(mode)
    {
      sprintf (buf, "You carefully apply %s to the %s %s on your %s.",
      obj->short_description, wound->severity, wound->name, expand_wound_loc (wound->location));
      sprintf (buf2, "$n carefully applies %s to the %s %s on $s %s.",
      obj->short_description, wound->severity, wound->name, expand_wound_loc (wound->location));
      act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
      act (buf2, false, ch, 0, victim, TO_ROOM | _ACT_FORMAT);
    }
    else
    {
      sprintf (buf, "You carefully apply %s to the %s %s on $N's %s.",
      obj->short_description, wound->severity, wound->name, expand_wound_loc (wound->location));
      sprintf (buf2, "$n carefully applies %s to the %s %s on your %s.",
      obj->short_description, wound->severity, wound->name, expand_wound_loc (wound->location));
      sprintf (buf3, "$n carefully applies %s to the %s %s on $N's %s.",
      obj->short_description, wound->severity, wound->name, expand_wound_loc (wound->location));
      act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
      act (buf2, false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
      act (buf3, false, ch, 0, victim, TO_ROOM | _ACT_FORMAT);
    }

    if(obj->obj_flags.type_flag == ITEM_POISON_PASTE)
    {
      soma_add_affect(victim, obj->o.od.value[1], obj->o.od.value[5], 0, 0,
                    obj->quality, obj->quality, obj->quality,
                    obj->o.od.value[2] * obj->o.od.value[0],
                    obj->o.od.value[3] * obj->o.od.value[0],
                    obj->o.od.value[4] * obj->o.od.value[0],
                    obj->o.od.value[5] * obj->o.od.value[0]);

      if(!IS_SET(obj->obj_flags.extra_flags, ITEM_BENIGN))
        wound->poison = obj->o.od.value[1];

      send_to_char(lookup_poison_variable(wound->poison, 2), victim);

    }
    else
    {
      if(obj->o.od.value[0] == type)
      {
        if((soma = get_affect(victim, type)))
        {
          soma->a.soma.duration = (obj->o.od.value[1]
            > soma->a.soma.duration) ? 2 : soma->a.soma.duration - obj->o.od.value[1];
          soma->a.soma.attack = (obj->o.od.value[1]
            > soma->a.soma.attack) ? 1 : soma->a.soma.attack - obj->o.od.value[1];
          soma->a.soma.decay = (obj->o.od.value[1]
            > soma->a.soma.decay) ? 2 : soma->a.soma.decay - obj->o.od.value[1];
          soma->a.soma.sustain = (obj->o.od.value[1]
            > soma->a.soma.sustain) ? 3 : soma->a.soma.sustain - obj->o.od.value[1];
          soma->a.soma.release = (obj->o.od.value[1]
            > soma->a.soma.release) ? 4 : soma->a.soma.release - obj->o.od.value[1];

          soma->a.soma.max_power -= (obj->o.od.value[1] * 20);
          soma->a.soma.lvl_power -= (obj->o.od.value[1] * 20);
          soma->a.soma.atm_power -= (obj->o.od.value[1] * 20);
        }
        wound->poison = 0;
      }

      if(obj->o.od.value[2] == type)
      {
        if((soma = get_affect(victim, type)))
        {
          soma->a.soma.duration = (obj->o.od.value[3]
            > soma->a.soma.duration) ? 2 : soma->a.soma.duration - obj->o.od.value[3];
          soma->a.soma.attack = (obj->o.od.value[3]
            > soma->a.soma.attack) ? 1 : soma->a.soma.attack - obj->o.od.value[3];
          soma->a.soma.decay = (obj->o.od.value[3]
            > soma->a.soma.decay) ? 2 : soma->a.soma.decay - obj->o.od.value[3];
          soma->a.soma.sustain = (obj->o.od.value[3]
            > soma->a.soma.sustain) ? 3 : soma->a.soma.sustain - obj->o.od.value[3];
          soma->a.soma.release = (obj->o.od.value[3]
            > soma->a.soma.release) ? 4 : soma->a.soma.release - obj->o.od.value[3];

          soma->a.soma.max_power -= (obj->o.od.value[3] * 20);
          soma->a.soma.lvl_power -= (obj->o.od.value[3] * 20);
          soma->a.soma.atm_power -= (obj->o.od.value[3] * 20);
        }
        wound->poison = 0;
      }

      if(obj->o.od.value[4] == type)
      {
        if((soma = get_affect(victim, type)))
        {
          soma->a.soma.duration = (obj->o.od.value[5]
            > soma->a.soma.duration) ? 2 : soma->a.soma.duration - obj->o.od.value[5];
          soma->a.soma.attack = (obj->o.od.value[5]
            > soma->a.soma.attack) ? 1 : soma->a.soma.attack - obj->o.od.value[5];
          soma->a.soma.decay = (obj->o.od.value[5]
            > soma->a.soma.decay) ? 2 : soma->a.soma.decay - obj->o.od.value[5];
          soma->a.soma.sustain = (obj->o.od.value[5]
            > soma->a.soma.sustain) ? 3 : soma->a.soma.sustain - obj->o.od.value[5];
          soma->a.soma.release = (obj->o.od.value[5]
            > soma->a.soma.release) ? 4 : soma->a.soma.release - obj->o.od.value[5];
          soma->a.soma.max_power -= (obj->o.od.value[5] * 20);
          soma->a.soma.lvl_power -= (obj->o.od.value[5] * 20);
          soma->a.soma.atm_power -= (obj->o.od.value[5] * 20);
        }
        wound->poison = 0;
      }
    }

    obj_from_char (&obj, 0);
    extract_obj (obj);
    */

}

void
do_coat (CHAR_DATA * ch, char *argument, int cmd)
{
    /*
    char target_str[MAX_STRING_LENGTH];
    char poison_str[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    OBJ_DATA *target;
    OBJ_DATA *poison;
    int amount = 1;
    int range = 0;
    const char *verbose_amount [] = {"", "a dab of ", "a few dabs of ", "a lot of ", "most of ", "all of "};

    argument = one_argument (argument, target_str);

    if (!*target_str)
      {
        send_to_char ("What do you want to coat, and with what?\n\r", ch);
        return;
      }

    if ((!(target = get_obj_in_dark (ch, target_str, ch->right_hand)) &&
        !(target = get_obj_in_dark (ch, target_str, ch->equip))) &&
        (!(target = get_obj_in_dark (ch, target_str, ch->left_hand)) &&
        !(target = get_obj_in_dark (ch, target_str, ch->equip))))
      {
        send_to_char ("You don't have that.\n\r", ch);
        return;
      }

    if (target->obj_flags.type_flag != ITEM_WEAPON &&
        target->obj_flags.type_flag != ITEM_DRINKCON &&
        target->obj_flags.type_flag != ITEM_FOOD)
      {
        act ("You can only coat weapons, liquid containers, and food.",
         false, ch, target, 0, TO_CHAR);
        return;
      }

    if(target->count > 1 || IS_SET(target->obj_flags.extra_flags, ITEM_STACK))
    {
      send_to_char("For the moment, you're unable to code any object which can be stacked, or is currently in a stack.\n", ch);
      return;
    }

    argument = one_argument (argument, poison_str);

    if (!str_cmp (poison_str, "with") || !str_cmp (poison_str, "using"))
      argument = one_argument (argument, poison_str);

    if (!*poison_str)
      {
        send_to_char ("What are you using to coat with?\n\r", ch);
        return;
      }

    if (!(poison = get_obj_in_dark (ch, poison_str, ch->right_hand)) && !(poison = get_obj_in_dark (ch, poison_str, ch->left_hand)))
      {
        send_to_char ("You don't have that.\n", ch);
        return;
      }

    if (GET_ITEM_TYPE(poison) != ITEM_CHEM_BOX)
      {
        act ("Thats not something you can coat an item with!", false, ch, poison, 0, TO_CHAR);
        return;
      }

    if (!(poison = get_obj_in_dark (ch, poison_str, ch->right_hand)) && !(poison = get_obj_in_dark (ch, poison_str, ch->left_hand)))
      {
        act ("You don't see that item.", false, ch, 0, 0, TO_CHAR);
        return;
      }

    if (*argument)
    {
      argument = one_argument (argument, arg);

      if (just_a_number(arg))
      {
        if (poison->o.poison.uses < atoi(arg) && poison->o.poison.uses != -1)
        {
          send_to_char ("There simply isn't that much poison\n", ch);
      return;
        }
        if (atoi(arg) < 1)
        {
          send_to_char ("As skilled as you may think you are, you can't poison something without use of at least some poison.\n", ch);
      return;
        }
        amount = atoi(arg);
      }
      else if (!strcmp (arg, "all"))
      {
        amount = MAX (1, poison->o.poison.uses);
      }
      else
      {
        send_to_char ("The correct syntax is #3poison <item> with <poison> [<amount>]#0.\n", ch);
        return;
      }
    }

    if (poison->o.poison.uses > 1)
      range = 1;
    if (amount > 1)
      range = 2;
    if (amount > (poison->o.poison.uses / 3) && amount != 1 && poison->o.poison.uses > 4)
      range = 3;
    if (amount > (poison->o.poison.uses * 2 / 3) && amount != 1)
      range = 4;
    if (amount == poison->o.poison.uses && amount != 1)
      range = 5;
    if (poison->o.poison.uses == -1)
      range = 1;

      if(target->obj_flags.type_flag == ITEM_DRINKCON && target->o.od.value[1] == 0)
      {
        send_to_char("There'll need to be some liquid in the drink container before you can poison it.\n", ch);
        return;
      }

      if((target->obj_flags.type_flag == ITEM_WEAPON || target->obj_flags.type_flag == ITEM_MISSILE)
        && poison->obj_flags.type_flag != ITEM_POISON_PASTE)
      {
        send_to_char("You need a paste type poison to properly poison a weapon.\n", ch);
        return;
      }

      if((target->obj_flags.type_flag == ITEM_FOOD || target->obj_flags.type_flag == ITEM_DRINKCON)
        && poison->obj_flags.type_flag != ITEM_POISON_LIQUID)
      {
        send_to_char("You need a liquid type poison to properly poison food or liquid.\n", ch);
        return;
      }

      if(target->obj_flags.type_flag == ITEM_WEAPON || target->obj_flags.type_flag == ITEM_MISSILE)
      {
        sprintf(buf, "You smear %s#2%s#0 over #2%s#0, taking care to avoid poisoning yourself...\n", verbose_amount[range], poison->short_description, target->short_description);
        send_to_char(buf, ch);

        sprintf(buf, "$n begins to delicately smear %s#2%s#0 over #2%s#0.", verbose_amount[range], poison->short_description, target->short_description);
        act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      }
      else
      {
        sprintf(buf, "You apply %s#2%s#0 to #2%s#0, taking care to avoid poisoning yourself...\n", verbose_amount[range], poison->short_description, target->short_description);
        send_to_char(buf, ch);

        sprintf(buf2, "$n begins to carefully apply %s#2%s#0 to #2%s#0.", verbose_amount[range], poison->short_description, target->short_description);
        act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      }

    ch->delay_type = DEL_POISON_ITEM;
    ch->delay = 10;
    ch->delay_info1 = (long int) target;
    ch->delay_info2 = (long int) poison;
    ch->delay_info3 = amount;
    */

}

void delayed_poison(CHAR_DATA * ch)
{
    /*
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    OBJ_DATA *target;
    OBJ_DATA *poison;
    POISON_DATA *existing;
    POISON_DATA *venom;
    int i;
    int j;
    int uses = 0;
    int amount;
    bool already = false;

    skill_use(ch, SKILL_BIOLOGY, 0);

    target = (OBJ_DATA *) ch->delay_info1;
    poison = (OBJ_DATA *) ch->delay_info2;
    amount = ch->delay_info3;

    if(!target || !poison)
    {
      send_to_char("Either your poison or your intended receptical is no longer in your hands.\n", ch);
      return;
    }

    i = number(1, SKILL_CEILING);
    j = ch->skills[SKILL_BIOLOGY];


    if(i > j + 50)
    {
      sprintf(buf, "Your stomach begins to twitch, and quickly sinks as you realise your ineptitude or inattention has led you to swallow some of #2%s#0!\n", poison->short_description);
      send_to_char(buf, ch);
      sprintf(buf2, "$n pauses in their task, a slow realisation of fear briefly crossing $s features.");
      act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

      soma_add_affect(ch, poison->o.poison.poison_type, poison->o.od.value[5], 0, 0, poison->quality/2, poison->quality/4, 0, poison->o.od.value[2]/2, poison->o.od.value[3]/2, poison->o.od.value[4]/2, poison->o.od.value[5]/2);

      send_to_char(lookup_poison_variable(poison->o.poison.poison_type, 1), ch);

      uses = -1;
    }
    else if(i > j + 35)
    {
      sprintf(buf, "You look down at your work, and realise your lack of expertise has led you to apply #2%s#0 in a thoroughly ineffective fashion.\n", poison->short_description);
      send_to_char(buf, ch);
      sprintf(buf2, "$n finishes applying #2%s#0 over #2%s#0 yet looks dissatisfied.", poison->short_description, target->short_description);
      act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      return;
    }
    else if(i > j + 15)
    {
      sprintf(buf, "You finish crudely applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      send_to_char(buf, ch);
      sprintf(buf2, "$n finishes applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      uses = -1;
    }
    else if(i < j - 15)
    {
      sprintf(buf, "You finish proficiently applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      send_to_char(buf, ch);
      sprintf(buf2, "$n finishes applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      uses = 1;
    }
    else
    {
      sprintf(buf, "You finish applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      send_to_char(buf, ch);
      sprintf(buf2, "$n finishes applying #2%s#0 to #2%s#0.\n", poison->short_description, target->short_description);
      act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
    }

    if(amount + uses >= 1)
    {
      if(target->poison)
      {
        for (existing = target->poison; existing; existing = existing->next)
        {
          if(existing->poison_type == poison->o.od.value[1])
          {
            already = true;
            existing->uses += amount + uses;
          }
        }
      }

      if(!already)
      {
        CREATE (venom, POISON_DATA, 1);
        venom->poison_type = poison->o.od.value[1];
        venom->duration = poison->o.od.value[5];
        venom->latency = 0;
        venom->minute = 0;
        venom->max_power = poison->quality;
        venom->lvl_power = poison->quality/2;
        venom->atm_power = poison->quality/4;
        venom->attack = poison->o.od.value[2];
        venom->decay = poison->o.od.value[3];
        venom->sustain = poison->o.od.value[4];
        venom->release = poison->o.od.value[5];
        venom->uses = amount + uses;
        venom->next = NULL;
        poison_to_obj(target, venom);
      }
    }
    else
    {
      ;
    }

    poison->o.od.value[0] -= amount;

    if(poison->o.od.value[0] <= 1)
    {
      sprintf(buf, "You use up the last of #2%s#0.\n", poison->short_description);
      send_to_char(buf, ch);
      obj_from_char (&poison, 0);
      extract_obj (poison);
    }
    */
}
