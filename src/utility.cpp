/*------------------------------------------------------------------------\
|  utility.c : Utility Module                         www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "trigram.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "group.h"
#include "utility.h"
#include "object_damage.h"

CHAR_DATA *loaded_list = NULL;

/*
	0	Non-Restricted
	-1	Innate Skill
	-2	Unimplemented
	-3	Prerequisites (Magic, Psionics, RPP, etc.)
	-4	Language
	-5 	Script
	-6	Requires 1 RPP or more
	-7	Requires 2 RPP or more
*/

const int restricted_skills[] =
{
   -2,				/* Unused */
    0,				/* Brawling */
   -1,				/* Small-Blade */
    0,				/* Long-Blade */
    0,				/* Polearm */
    0,				/* Bludgeon */
   -1,				/* Dodge */
   -1,				/* Deflect */
    0,				/* Sole Use */
    0,				/* Dual-Wield */
   -1,				/* Aim */
    0,				/* Handgun */
    0,				/* Rifle */
   -2,				/* SMG */
   -2,				/* Gunnery */
   -2,				/* Explosives */

    0,				/* Sneak */
    0,				/* Hide */
    0,				/* Steal */
   -2,				/* Picklock */
   -2,				/* Haggle */
   -2,				/* Handle */
    0,				/* Hunting */
    0,				/* First-Aid */
    0,				/* Medicine */
    0,				/* Forage */
   -1,				/* Eavesdrop */
    0,				/* Butchery */

    0,				/* Chemistry */
    0,				/* Mechanics */
    0,				/* Gunsmithery */
   -2,				/* Computerology */
    0,				/* Electronics */
    0,				/* Biology */
    0,				/* Weaponcraft */
    0,				/* Armorcraft */
    0,				/* Handicraft */
    0,				/* Artistry */

    0,             /* Education */
   -2,				/* Voodoo */
   -1,             // Common

};

bool spare_capacity( CHAR_DATA * ch, OBJ_DATA * obj, bool drop_all )
{
	ROOM_DATA * room = NULL;
	int limit = 0;

	if ( ! ( room = ch->room ) )
		return false;

	if ( IS_SET( room->room_flags, SMALL_ROOM ) )
		limit = 50000;
	else if ( IS_SET( room->room_flags, MEDIUM_ROOM ) )
        limit = 100000;
	else if (!str_cmp(room->name, "Interior of a Capsule Pod"))
		limit = 10000;
	else
		return true;

	OBJ_DATA *robj = NULL;
	int weight = 0;



	for (robj = room->contents; robj; robj = robj->next_content)
	{

		weight += OBJ_MASS(robj);
	}

	if (drop_all)
	{

		if (ch->left_hand)
			weight += OBJ_MASS(ch->left_hand);

		if (ch->right_hand)
			weight += OBJ_MASS(ch->right_hand);
	}
	else
	{
		weight += OBJ_MASS(obj);
	}

	if (weight > limit)
	{
		return false;
	}


	return true;
}

// Are we involved in a combat?
// 0 = are we either attacking X, or being attacked by X?
// 1 = are we either attacking X, being attacked by X,
//     guarding X, being guarded by X,
//     attacking someone guarding X, attacking someone X guards,
//     guarding someone X is attacking, or guarding someong attacking X?

bool
is_involved (CHAR_DATA *ch, CHAR_DATA *tch, int cmd)
{
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *xaf = NULL;
    CHAR_DATA *ch_guard = NULL; // This is who is either being guarded or guarding us.
    CHAR_DATA *tch_guard = NULL; // This is who is either being guarded or guarding them.

    bool ch_is_guarded = false;
    bool tch_is_guarded = false;

    // If we don't exist or they don't exist, hard to be involved.
    if (!tch || !ch)
        return false;

    // If we're not in the same room, we can't be involved.
    if (tch->in_room != ch->in_room)
        return false;

    // Obviously, we're combat_involved with ourselves!
    if (ch == tch)
        return true;

    // We're fighting them.
    if (ch->fighting == tch)
        return true;

    // They're fighting us.
    if (tch->fighting == ch)
        return true;

    // If we're guarding someone, and that someone is in the room...
    if ((af = get_affect (ch, MAGIC_GUARD)) && ((CHAR_DATA *) af->a.spell.t) == ch_guard && ch->in_room == ch_guard->in_room)
    {
        // If they are our ward, and one of us is fighting (so we don't get a false positive if neither of us is fighting)
        if (ch_guard == tch && (ch->fighting || tch->fighting))
            return true;

        // Our ward is fighting them.
        if (ch_guard->fighting == tch)
            return true;

        // They're fighting our ward.
        if (tch->fighting == ch_guard)
            return true;
    }
    // We're not guarding anyone in this but, but...
    else
    {
        // If someone in the room is guarding us...
        for (ch_guard = ch->room->people; ch_guard; ch_guard = ch_guard->next_in_room)
        {
            // If we're somehow guarding ourself, keep on moving.
            if (ch_guard == ch)
                continue;

            if ((af = get_affect (ch_guard, MAGIC_GUARD)) && ((CHAR_DATA *) af->a.spell.t) == ch)
            {
                // If they are our guard, and either one of us is fighting, we're involved.
                if (ch_guard == tch && (ch->fighting || tch->fighting))
                    return true;
                // If our guard is fighting them.
                else if (ch_guard->fighting == tch)
                    return true;
                // If they are fighting our guard.
                else if (tch->fighting == ch_guard)
                    return true;
                else
                    ch_is_guarded = true;
            }
        }
    }

    // If they're guarding someone and that someone is in the room...
    if ((xaf = get_affect (tch, MAGIC_GUARD)) && ((CHAR_DATA *) xaf->a.spell.t) == tch_guard && tch->in_room == tch_guard->in_room)
    {
        // If we are their ward
        if (tch_guard == ch && (ch->fighting || tch->fighting))
            return true;

        // If their ward is fighting us
        if (tch_guard->fighting == ch)
            return true;

        // If we are fighting their ward
        if (ch->fighting == tch_guard)
            return true;
    }
    //  They're not guarding anyone, but someone in the room is guarding them....
    else
    {
        // If someone in the room is guarding them...
        for (tch_guard = tch->room->people; tch_guard; tch_guard = tch_guard->next_in_room)
        {
            if (tch_guard == tch)
                continue;

            if ((xaf = get_affect (tch_guard, MAGIC_GUARD)) && ((CHAR_DATA *) xaf->a.spell.t) == tch)
            {
                // If we are their guard.
                if (tch_guard == ch && (ch->fighting || tch->fighting))
                    return true;
                // If their guard is fighting us.
                else if (tch_guard->fighting == ch)
                    return true;
                // If we are fighting their guard
                else if (ch->fighting == tch_guard)
                    return true;
                else
                    tch_is_guarded = true;
            }
        }
    }

    // Final check - is ch's guard fighting tch's guard?

    // If we've both got someone guarding us...
    if (tch_is_guarded && ch_is_guarded)
    {
        // Check for everyone who is guarding ch,
        for (ch_guard = ch->room->people; ch_guard; ch_guard = ch_guard->next_in_room)
        {
            if (ch_guard == ch)
                continue;

            if ((xaf = get_affect (ch_guard, MAGIC_GUARD)) && ((CHAR_DATA *) xaf->a.spell.t) == ch)
            {
                // If we find someone, then check everyone guarding tch...
                for (tch_guard = tch->room->people; tch_guard; tch_guard = tch_guard->next_in_room)
                {
                    if (tch_guard == tch)
                        continue;

                    if ((xaf = get_affect (tch_guard, MAGIC_GUARD)) && ((CHAR_DATA *) xaf->a.spell.t) == tch)
                    {
                        // Then if we find someoe, and ch_guard and tch_guard are fighting one another, we're involved.
                        if (tch_guard->fighting == ch_guard || ch_guard->fighting == tch_guard)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;

};


void fluid_object(OBJ_DATA *obj)
{
		if ((GET_ITEM_TYPE(obj) == ITEM_DRINKCON || GET_ITEM_TYPE(obj) == ITEM_FOUNTAIN)
			&& vtoo(obj->o.drinks.liquid) && obj->o.drinks.volume && !obj->contains)
		{
			OBJ_DATA *fluid = load_object(obj->o.drinks.liquid);
			fluid->count = obj->o.drinks.volume;
			obj_to_obj(fluid, obj);
		}
}

// Corrects a/an in a string, strips our double blanks.
void
correct_grammar (char *source, char **target)
{
    char original[MAX_STRING_LENGTH] = {'\0'};
    char fixed[MAX_STRING_LENGTH];
    *fixed = '\0';

    // We copy our original from the source.
    sprintf (original, "%s", source);

    // For every character...
    for (size_t y = 0; y <= strlen (original); y++)
    {
        // If we have an a, and either nothing before us or a space before us,
		// then a space, then a vowel...
        if ((original[y] == 'a' || original[y] == 'A') &&
			(y == 0 || (y != 0 && original[y-1] == ' ')) &&
			 original[y+1] == ' ' &&
            (original[y+2] == 'a' || original[y+2] == 'e' || original[y+2] == 'i' || original[y+2] == 'o' || original[y+2] == 'u'))
         {
           // we add an an
           sprintf(fixed + strlen(fixed), "%cn", original[y]);
         }
         // If we've got two spaces, we need to fix it up - just skip ahead.
         else if (original[y] == ' ' && original[y+1] == ' ')
         {
             sprintf(fixed + strlen(fixed), "%c", original[y]);
             y++;
         }
         else
         {
            sprintf(fixed + strlen(fixed), "%c", original[y]);
         }

    }

    *target = str_dup (fixed);

}


/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *
one_argument (char *argument, char *arg_first)
{
    char cEnd;

    if (argument == NULL)
        return "";

    while (isspace (*argument))
        argument++;

    cEnd = ' ';

    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0')
    {

        if (*argument == cEnd)
        {
            argument++;
            break;
        }

        if (cEnd == ' ')
            *arg_first = tolower (*argument);
        else
            *arg_first = *argument;

        arg_first++;
        argument++;
    }

    *arg_first = '\0';

    while (isspace (*argument))
        argument++;

    return argument;
}

std::string
one_argument (std::string& argument, std::string& arg_first)
{
    arg_first.erase ();
    if (argument.length () == 0)
    {
        return argument;
    }

    std::string::size_type x = 0;
    std::string::iterator i = argument.begin ();
    for (; i != argument.end (); ++i,++x)
    {
        if (!isspace (*i))
        {
            break;
        }
    }

    if (*i == '\'' || *i == '"')
    {
        char token_separator = *i;
        ++i;
        ++x;

        for (; i != argument.end (); ++i,++x)
        {
            if (*i == token_separator)
            {
                ++i;
                ++x;
                break;
            }
            arg_first += *i;
        }
    }
    else
    {
        for (; i != argument.end (); ++i,++x)
        {
            if (isspace(*i))
            {
                ++i;
                ++x;
                break;
            }
            arg_first += tolower (*i);
        }
    }

    for (; i != argument.end (); ++i,++x)
    {
        if (!isspace (*i))
        {
            break;
        }
    }

    return argument.substr(x);
}


#ifdef NOVELL
int
strcasecmp (char *s1, char *s2)
{
    char *p, *q;

    p = s1;
    q = s2;

    if (strlen (p) != strlen (q))
        return (strlen (p) - strlen (q));

    for (; *p != '\0' && *q != '\0'; p++, q++)
    {
        if (tolower (*p) < tolower (*q))
            return (-1);
        if (tolower (*p) > tolower (*q))
            return (1);
    }

    return (0);
}
#endif

// is_overcast
// returns true if the room is out of sunlight (petrification, etc)
bool
is_overcast (ROOM_DATA * room)
{
    bool result = false;
    int flags = room->room_flags;
    //int clouds = weather_info[room->zone].clouds;

    if ((room->sector_type == SECT_INSIDE)
            || (flags & INDOORS)
            //|| (clouds == OVERCAST)
            //|| (clouds == HEAVY_CLOUDS)
            || (flags & STIFLING_FOG))
    {
        result = true;
    }

    return result;
}

// is_sunlight_restricted
// returns true if the character is currently suffering due to the sun.
// TODO: move to char.h
bool
is_sunlight_restricted (CHAR_DATA * ch, ROOM_DATA * room)
{
    bool result = false;

    if (sun_light && (ch->affected_by & AFF_SUNLIGHT_PEN))
    {
        if (!is_overcast (ch->room)
                || (room && !is_overcast (room)))
        {
            send_to_char ("The brilliant flame of Anor interferes "
                          "with your attempt.\n", ch);
            result = true;
        }
    }

    return result;
}

// keyword_gen:
// Creates a character's keyword list based on their name and short-description.

char *excluded_keywords[] =
{
    "a",
    "an",
    "the",
    "with",
    "is",
    "of",
    "\n"
};


char *
keyword_gen (CHAR_DATA *ch, char *argument, int mode)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH] = {'\0'};
    int ind = 0;
    size_t y = 0;
    char *point;

    if (!mode)
        sprintf(buf2, "%s", ch->tname);

    while (*argument)
    {
        argument = one_argument(argument, buf);

        ind = 0;

        if ((ind = index_lookup (excluded_keywords, buf)) == -1)
        {
            strcat(buf2, " ");
            for (y = 0; y < strlen(buf); y++)
            {
                if (!isalpha(buf[y]) && (y == strlen(buf) - 1))
                    continue;
                sprintf(buf2 + strlen(buf2), "%c", buf[y]);
            }
        }

        point = strpbrk (buf, "-");

        if (point)
        {
            strcat(buf2, " ");

            for (y = 0; y < strlen(buf); y++)
            {
                if (buf[y] == *point)
                {
                    strcat(buf2, " ");
                    continue;
                }

                sprintf(buf2 + strlen(buf2), "%c", buf[y]);
            }
        }

    }

    return buf2;
}



int
is_restricted_skill (CHAR_DATA * ch, int skill, int prof)
{

    if (!ch)
        return 1;

    if (restricted_skills[skill] == -1 || restricted_skills[skill] == -2
            || restricted_skills[skill] == -3)
        return 1;

    if (restricted_skills[skill] == -6)
    {
        // RPP-restricted skills
        if (ch->descr() && ch->descr()->acct
                && ch->descr()->acct->get_rpp () >= 1)
            return 0;
        else
            return 1;
    }

    if (restricted_skills[skill] == -7)
    {
        // RPP-restricted skills
        if (ch->descr() && ch->descr()->acct
                && ch->descr()->acct->get_rpp () >= 2)
            return 0;
        else
            return 1;
    }

    if (prof)
        return 0;

    if (skill == SKILL_PICK)
    {
        if (ch->skills[SKILL_STEAL])
            return 0;
        else
            return 1;
    }

    if (skill == SKILL_STEAL)
    {
        if (ch->skills[SKILL_HIDE])
            return 0;
        else
            return 1;
    }

    if (skill == SKILL_SNEAK)
    {
        if (ch->skills[SKILL_HIDE])
            return 0;
        else
            return 1;
    }

    // Need at least one weapon skill to get a style skill.
    if (skill == SKILL_DUAL_WIELD || skill == SKILL_SOLE_WIELD)
    {
        if (ch->skills[SKILL_BRAWLING] ||
            ch->skills[SKILL_LONG_BLADE] ||
            ch->skills[SKILL_BLUDGEON] ||
			ch->skills[SKILL_HANDGUN] ||
			ch->skills[SKILL_RIFLE] ||
			ch->skills[SKILL_SMG] ||
			ch->skills[SKILL_GUNNERY] ||
			ch->skills[SKILL_EXPLOSIVES] ||
            ch->skills[SKILL_POLEARM])
            return 0;
        else
            return 1;
    }

    if (skill == SKILL_MEDICINE)
    {
        if (ch->skills[SKILL_FIRSTAID])
            return 0;
        else
            return 1;
    }


    if (restricted_skills[skill] == -4)
    {
        // Languages
        if (skill == SKILL_COMMON)
            return 0;
    }

    // Scripts: must know literacy first, and then Tengwar, and then you need to be an elf
    // to know anything interesting.

    else if (restricted_skills[skill] == -5)
    {
        if (!real_skill (ch, SKILL_EDUCATION))
            return 1;

        if (skill == SKILL_COMMON)
            return 0;

        return 0;
    }

    if (restricted_skills[skill] == 0)
        return 0;

    return 1;
}

void
send_email (account * to_acct, const char *cc, char *from, char *subject,
            char *message)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];

    if (!to_acct)
        return;

    if (to_acct->email.empty ())
        return;

    if (!strchr (to_acct->email.c_str (), '@'))
        return;

    if (!*subject)
        return;

    if (!*message)
        return;

    if (!*from)
        return;

    sprintf (buf, "%s -t", PATH_TO_SENDMAIL);

    fp = popen (buf, "w");
    if (!fp)
        return;
    fprintf (fp, "From: %s\n", from);
    fprintf (fp, "To: %s\n", to_acct->email.c_str ());
    if (cc != NULL && *cc)
        fprintf (fp, "Cc: %s\n", cc);
    fprintf (fp, "X-Sender: %s\n", MUD_EMAIL);
    fprintf (fp, "Mime-Version: 1.0\n");
    fprintf (fp, "Content-type: text/plain;charset=\"us-ascii\"\n");
    fprintf (fp, "Organization: %s\n", MUD_NAME);
    fprintf (fp, "Subject: %s\n", subject);
    fprintf (fp, "\n");
    fprintf (fp, "%s", message);

    // Remove this line to remove MUD-specific email footer


    fprintf (fp,
             "\n\n\n\n\n--\nParallel RPI: http://www.forum.parallelrpi.com\n");

    // Remove this line to remove reference to SoI's weekly automated newsletter
    /*
      if (strstr (subject, "Palantir Weekly"))
        fprintf (fp,
    	     "\nTo discontinue receiving issues of the Palantir Weekly via email, please click here:\nhttp://www.middle-earth.us/index.php?display=unsubscribe&account=%s&id=%s\n",
    	     to_acct->name.c_str (), to_acct->password.c_str ());*/

    pclose (fp);

}

char *
generate_password (int argc, char **argv)
{
    int password_length;		/* how long should each password be */
    int n_passwords;		/* number of passwords to generate */
    int pwnum;			/* number generated so far */
    int c1, c2, c3;		/* array indices */
    long sumfreq;			/* total frequencies[c1][c2][*] */
    double pik;			/* raw random number in [0.1] from drand48() */
    long ranno;			/* random number in [0,sumfreq] */
    long sum;			/* running total of frequencies */
    char password[100];		/* buffer to develop a password */
    static char result[100];
    int nchar;			/* number of chars in password so far */
    struct timeval systime;	/* time reading for random seed */
    struct timezone tz;		/* unused arg to gettimeofday */

    password_length = 8;		/* Default value for password length */
    n_passwords = 10;		/* Default value for number of pws to generate */

    gettimeofday (&systime, &tz);	/* Read clock. */
    srand48 (systime.tv_usec);	/* Set random seed. */

    if (argc > 1)
    {
        /* If args are given, convert to numbers. */
        n_passwords = atoi (&argv[1][0]);
        if (argc > 2)
        {
            password_length = atoi (&argv[2][0]);
        }
    }
    if (argc > 3 || password_length > 99 ||
            password_length < 0 || n_passwords < 0)
    {
        printf (" USAGE: gpw [npasswds] [pwlength]\n");
        abort ();
    }

    /* Pick a random starting point. */
    /* (This cheats a little; the statistics for three-letter
       combinations beginning a word are different from the stats
       for the general population.  For example, this code happily
       generates "mmitify" even though no word in my dictionary
       begins with mmi. So what.) */
    for (pwnum = 0; pwnum < n_passwords; pwnum++)
    {
        pik = drand48 ();		/* random number [0,1] */
        sumfreq = sigma;		/* sigma calculated by loadtris */
        ranno = (long)(pik * (double)sumfreq);	/* Weight by sum of frequencies. */
        sum = 0;
        for (c1 = 0; c1 < 26; c1++)
        {
            for (c2 = 0; c2 < 26; c2++)
            {
                for (c3 = 0; c3 < 26; c3++)
                {
                    sum += tris[c1][c2][c3];
                    if (sum > ranno)
                    {
                        /* Pick first value */
                        password[0] = 'a' + c1;
                        password[1] = 'a' + c2;
                        password[2] = 'a' + c3;
                        c1 = c2 = c3 = 26;	/* Break all loops. */
                    }		/* if sum */
                }		/* for c3 */
            }			/* for c2 */
        }			/* for c1 */

        /* Do a random walk. */
        nchar = 3;		/* We have three chars so far. */
        while (nchar < password_length)
        {
            password[nchar] = '\0';
            password[nchar + 1] = '\0';
            c1 = password[nchar - 2] - 'a';	/* Take the last 2 chars */
            c2 = password[nchar - 1] - 'a';	/* .. and find the next one. */
            sumfreq = 0;
            for (c3 = 0; c3 < 26; c3++)
                sumfreq += tris[c1][c2][c3];
            /* Note that sum < duos[c1][c2] because
               duos counts all digraphs, not just those
               in a trigraph. We want sum. */
            if (sumfreq == 0)
            {
                /* If there is no possible extension.. */
                break;		/* Break while nchar loop & print what we have. */
            }
            /* Choose a continuation. */
            pik = drand48 ();
            ranno = (long)(pik * (double)sumfreq);	/* Weight by sum of frequencies for row. */
            sum = 0;
            for (c3 = 0; c3 < 26; c3++)
            {
                sum += tris[c1][c2][c3];
                if (sum > ranno)
                {
                    password[nchar++] = 'a' + c3;
                    c3 = 26;	/* Break the for c3 loop. */
                }
            }			/* for c3 */
        }			/* while nchar */
        printf ("%s\n", password);
    }
    sprintf (result, password);	/* for pwnum */
    return result;
}


/* creates a random number in interval [from;to] */
int
number (int from, int to)
{
    if (to == from)
        return from;

    if (to > from)
        return ((rand () % (to - from + 1)) + from);
    else
        return ((rand () % (from - to + 1)) + to);
}

/* simulates dice roll */
unsigned int
dice (unsigned int number, unsigned int size)
{
    unsigned int r;
    unsigned int sum = 0;

    if (size <= 0)
        size = 1;

    for (r = 1; r <= number; r++)
        sum += (rand () % size) + 1;

    return sum;
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both
int str_cmp(char *arg1, char *arg2)
{
	register int chk, i;

	for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
		if ((chk = (tolower(*(arg1 + i)) - tolower(*(arg2 + i))))) {
			if (chk < 0)
				return (-1);
			else
				return (1);
		}
	return(0);
}
*/


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int
strn_cmp (const char *arg1, const char *arg2, int n)
{
    int chk, i;

    for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
        if ((chk = (tolower (*(arg1 + i)) - tolower (*(arg2 + i)))))
        {
            if (chk < 0)
                return (-1);
            else
                return (1);
        }

    return (0);
}

void
sprintbit (long vektor, const char *names[], char *result)
{
    int i;

    *result = '\0';

    for (i = 0; i <= 31; i++)
        if (IS_SET (vektor, 1 << i) && str_cmp (names[i], "Psave"))
            sprintf (result + strlen (result), "%s ", names[i]);

    if (!*result)
        strcpy (result, "NOBITS");
}



void
sprinttype (int type, const char *names[], char *result)
{
    int nr;

    for (nr = 0; (*names[nr] != '\n'); nr++);
    if (type < nr && names[type])
        strcpy (result, names[type]);
    else
        strcpy (result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data
            real_time_passed (time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.minute = (secs / 60) % 60;
    secs -= 60 * now.minute;

    now.hour = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR * now.hour;

    now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
    secs -= SECS_PER_REAL_DAY * now.day;

    now.month = -1;
    now.year = -1;

    return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data
            mud_time_passed (time_t t2, time_t t1)
{
    time_t secs;
    struct time_info_data now =
    {
        0, 0, 0, GAME_BASE_YEAR
    };

    secs = (t2 - t1) * PULSES_PER_SEC;

    now.year += secs / GAME_SECONDS_PER_YEAR;
    secs = secs % GAME_SECONDS_PER_YEAR;

    now.month += secs / GAME_SECONDS_PER_MONTH;
    secs = secs % GAME_SECONDS_PER_MONTH;

    now.day += secs / GAME_SECONDS_PER_DAY;
    secs = secs % GAME_SECONDS_PER_DAY;

    now.hour += secs / GAME_SECONDS_PER_HOUR;

    return now;
}


/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data
            moon_time_passed (time_t t2, time_t t1)
{
    time_t secs;
    struct time_info_data now =
    {
        0, 0, 0, GAME_BASE_YEAR
    };

    secs = (t2 - t1) * PULSES_PER_SEC;

    now.year += secs / MOON_SECONDS_PER_YEAR;
    secs = secs % MOON_SECONDS_PER_YEAR;

    now.month += secs / MOON_SECONDS_PER_MONTH;
    secs = secs % MOON_SECONDS_PER_MONTH;

    now.day += secs / MOON_SECONDS_PER_DAY;
    secs = secs % MOON_SECONDS_PER_DAY;

    now.hour += secs / MOON_SECONDS_PER_HOUR;

    return now;
}

struct time_info_data
            age (CHAR_DATA * ch)
{
    struct time_info_data player_age;

    player_age = moon_time_passed (time (0), ch->time.birth);

    player_age.year += ch->age - GAME_BASE_YEAR;	/* All players start at 17 */

    return player_age;
}

int
parse_argument (const char *commands[], char *string)
{
    int len;
    int i;

    if (!*string)
        return -1;

    len = strlen (string);

    for (i = 0; *commands[i] != '\n'; i++)
    {
        if (strn_cmp (commands[i], string, len) == 0)
            return i;
    }

    return -1;
}

void
arg_splitter (int argc, char *fmt, ...)
{
    va_list arg_ptr;
    char *p;
    char *sval;

    va_start (arg_ptr, fmt);	/* arg_ptr points to 1st unnamed arg */
    for (p = fmt; *p;)
    {
        for (; isspace (*p); p++);

        if (*p && argc)
        {
            sval = va_arg (arg_ptr, char *);
            if (*p == '\'' || *p == '"')
            {
                int term = (int)*p;
                ++p; // skip first quote
                for (; (*p) && ((int)(*p) != term) && (*sval = *p); p++, sval++);
                if  (*p)
                {
                    ++p; // skip last quote
                }
            }
            else
            {
                for (; !isspace (*p) && (*sval = *p); p++, sval++);
            }
            if (argc == 1 || !*p)
                for (; (*sval = *p); p++, sval++);
            else
                *sval = '\0';
            argc--;
        }
    }

    while (argc)
    {
        sval = va_arg (arg_ptr, char *);
        *sval = '\0';
        argc--;
    }

    va_end (arg_ptr);		/* clean up when done */
}

#define TOKEN_NUMBER	1
#define TOKEN_INT		3
#define TOKEN_DEX		4
#define TOKEN_CON		5
#define TOKEN_WIL		6
#define TOKEN_STR		7
#define TOKEN_AUR		8
#define TOKEN_DIV		10
#define TOKEN_SUB		11
#define TOKEN_PLUS		12
#define TOKEN_MULT		13
#define TOKEN_OPEN_PAR	14
#define TOKEN_CLOSE_PAR	15
#define TOKEN_AGI		16

int
get_eval_token (char **p, int *token_type, int *value)
{
    char tmp[MAX_INPUT_LENGTH];
    int index = 0;

    *value = 0;
    *token_type = 0;

    while (isspace (**p))
        (*p)++;

    /* Number */

    if (isdigit (**p))
    {
        while (isdigit (**p))
        {
            *value = *value * 10 + (**p - '0');
            tmp[index++] = **p;
            (*p)++;
        }
        tmp[index] = 0;
        *token_type = TOKEN_NUMBER;
        return 1;
    }

    /* Special: ()/+-* */

    if (**p == '+')
        *token_type = TOKEN_PLUS;
    else if (**p == '/')
        *token_type = TOKEN_DIV;
    else if (**p == '*')
        *token_type = TOKEN_MULT;
    else if (**p == '-')
        *token_type = TOKEN_SUB;
    else if (**p == '(')
        *token_type = TOKEN_OPEN_PAR;
    else if (**p == ')')
        *token_type = TOKEN_CLOSE_PAR;

    if (*token_type)
    {
        (*p)++;
        return 1;
    }

    /* attribute */

    if (!strncmp (*p, "int", 3))
        *token_type = TOKEN_INT;
    else if (!strncmp (*p, "dex", 3))
        *token_type = TOKEN_DEX;
    else if (!strncmp (*p, "str", 3))
        *token_type = TOKEN_STR;
    else if (!strncmp (*p, "aur", 3))
        *token_type = TOKEN_AUR;
    else if (!strncmp (*p, "con", 3))
        *token_type = TOKEN_CON;
    else if (!strncmp (*p, "wil", 3))
        *token_type = TOKEN_WIL;
    else if (!strncmp (*p, "agi", 3))
        *token_type = TOKEN_AGI;

    if (*token_type)
    {
        *p += 3;
        return 1;
    }

    return 0;
}

int
eval_att_eq (CHAR_DATA * ch, char **equation)
{
    int token_type;
    int token_value;
    int left_side;
    int right_side;
    int sign_flag;
    int operation;

    left_side = 0;
    operation = TOKEN_PLUS;

    while (**equation)
    {

        sign_flag = -1;

        do
        {
            sign_flag = -sign_flag;

            if (!get_eval_token (equation, &token_type, &token_value))
                return -12370;

        }
        while (token_type == TOKEN_SUB);

        if (token_type == TOKEN_OPEN_PAR)
            right_side = eval_att_eq (ch, equation);
        else if (token_type == TOKEN_NUMBER)
            right_side = token_value;

        else if (token_type == TOKEN_INT)
            right_side = GET_INT (ch);
        else if (token_type == TOKEN_DEX)
            right_side = GET_DEX (ch);
        else if (token_type == TOKEN_STR)
            right_side = GET_STR (ch);
        else if (token_type == TOKEN_AUR)
            right_side = GET_AUR (ch);
        else if (token_type == TOKEN_CON)
            right_side = GET_CON (ch);
        else if (token_type == TOKEN_WIL)
            right_side = GET_WIL (ch);
        else if (token_type == TOKEN_AGI)
            right_side = GET_AGI (ch);

        else
            return -12350;

        right_side = right_side * sign_flag;

        switch (operation)
        {
        case TOKEN_PLUS:
            left_side = left_side + right_side;
            break;

        case TOKEN_SUB:
            left_side = left_side - right_side;
            break;

        case TOKEN_DIV:
            if (!right_side)
                return -12360;
            left_side = left_side / right_side;
            break;

        case TOKEN_MULT:
            left_side = left_side * right_side;
            break;
        }

        if (!get_eval_token (equation, &operation, &token_value) ||
                operation == TOKEN_CLOSE_PAR)
            return left_side;

        if (operation != TOKEN_PLUS && operation != TOKEN_SUB &&
                operation != TOKEN_MULT && operation != TOKEN_DIV)
        {
            return -12340;
        }
    }

    return left_side;
}

int
eval_cap (CHAR_DATA * ch, char *equation)
{
    char attribute_1[3];
    char attribute_2[3];
    char attribute_3[3];
    char buf[MAX_STRING_LENGTH];
    int primary = 0, secondary = 0, tertiary = 0, cap = 0;

    *attribute_1 = '\0';
    *attribute_2 = '\0';
    *attribute_3 = '\0';
    *buf = '\0';

    sscanf (equation, "%s %s %s", attribute_1, attribute_2, attribute_3);

    if (!str_cmp (attribute_1, "str"))
        primary = GET_STR (ch);
    else if (!str_cmp (attribute_1, "con"))
        primary = GET_CON (ch);
    else if (!str_cmp (attribute_1, "dex"))
        primary = GET_DEX (ch);
    else if (!str_cmp (attribute_1, "agi"))
        primary = GET_AGI (ch);
    else if (!str_cmp (attribute_1, "aur"))
        primary = GET_AUR (ch);
    else if (!str_cmp (attribute_1, "wil"))
        primary = GET_WIL (ch);
    else if (!str_cmp (attribute_1, "int"))
        primary = GET_INT (ch);


    if (!str_cmp (attribute_2, "str"))
        secondary = GET_STR (ch);
    else if (!str_cmp (attribute_2, "con"))
        secondary = GET_CON (ch);
    else if (!str_cmp (attribute_2, "dex"))
        secondary = GET_DEX (ch);
    else if (!str_cmp (attribute_2, "agi"))
        secondary = GET_AGI (ch);
    else if (!str_cmp (attribute_2, "aur"))
        secondary = GET_AUR (ch);
    else if (!str_cmp (attribute_2, "wil"))
        secondary = GET_WIL (ch);
    else if (!str_cmp (attribute_2, "int"))
        secondary = GET_INT (ch);

    if (!str_cmp (attribute_3, "str"))
        tertiary = GET_STR (ch);
    else if (!str_cmp (attribute_3, "con"))
        tertiary = GET_CON (ch);
    else if (!str_cmp (attribute_3, "dex"))
        tertiary = GET_DEX (ch);
    else if (!str_cmp (attribute_3, "agi"))
        tertiary = GET_AGI (ch);
    else if (!str_cmp (attribute_3, "aur"))
        tertiary = GET_AUR (ch);
    else if (!str_cmp (attribute_3, "wil"))
        tertiary = GET_WIL (ch);
    else if (!str_cmp (attribute_3, "int"))
        tertiary = GET_INT (ch);

    cap = (primary * 2) + ((secondary * 150)/100) + (primary);

    cap = MIN (cap, 100);

    return cap;
}

int
calc_lookup (CHAR_DATA * ch, int reg_index, int reg_entry)
{
    char *p;
    int calced_value;

    if (!(p = lookup_string (reg_entry, reg_index)))
    {
        return 100;
    }

    if (reg_index == REG_CAP)
    {
        calced_value = eval_att_eq (ch, &p);
        calced_value = MIN (calced_value, 100);
    }
    else
        calced_value = eval_att_eq (ch, &p);

    if (calced_value < -12300)
    {
        return 999;
    }

    return calced_value;
}

void
print_mem_stats (CHAR_DATA * ch)
{
    extern int bytes_allocated;
    extern char *memory_next;
    extern char *memory_top;
    char buf[MAX_STRING_LENGTH];

    sprintf (buf, "Bytes allocated:  %d  Internal free: %d", bytes_allocated,
             memory_top - memory_next);

    system_log (buf, false);
}

void
sort_int_array (int *array, int entries)
{
    int tmp;
    int i;
    int j;

    for (j = 0; j < entries; j++)
    {
        for (i = 0; i < entries - 1; i++)
        {
            if (array[i] > array[i + 1] || !array[i])
            {
                tmp = array[i];
                array[i] = array[i + 1];
                array[i + 1] = tmp;
            }
        }
    }
}

#ifdef MEMORY_CHECK
#define MAX_ALLOC	70000

MEMORY_T *alloc_ptrs[MAX_ALLOC];
#endif

int bytes_allocated = 0;
int first_free = 0;
int mud_memory = 0;

#ifdef MEMORY_CHECK
malloc_t
alloc (int bytes, int dtype)
{
    char *p;
    MEMORY_T *m;
    extern int mem_allocated;

    system_log ("Alloc...", false);

    bytes += sizeof (MEMORY_T);

    p = calloc (1, bytes);
    m = (MEMORY_T *) p;

    mem_allocated += bytes;

    m->dtype = dtype;
    m->entry = first_free;
    m->bytes = bytes;
    m->time_allocated = mud_time;

    /*
    	memcpy (p, &dtype, 4);
    	memcpy (p + 4, &first_free, 4);
    	memcpy (p + 8, &bytes, 4);

    	printf ("Allocating %d, %d, %d\n", first_free, bytes, dtype);
    */

    alloc_ptrs[first_free++] = m;

    bytes_allocated += bytes;
    mud_memory += bytes - sizeof (MEMORY_T);

    if (x1)
    {
        printf ("+ #%d @ %Xd for %d bytes: %d  ",
                first_free - 1, (int) p + sizeof (MEMORY_T), bytes, dtype);

        switch (dtype)
        {
        case 1:
            printf ("MESSAGE DATA");
            break;
        case 2:
            printf ("write_to_q");
            break;
        case 3:
            printf ("string_add");
            break;
        case 4:
            printf ("file_to_string");
            break;
        case 5:
            printf ("init_memory");
            break;
        case 6:
            printf ("get_perm");
            break;
        case 7:
            printf ("get_perm 2");
            break;
        case 8:
            printf ("LINE_DATA");
            break;
        case 9:
            printf ("ve_insert_char");
            break;
        case 10:
            printf ("ve_reconstruct");
            break;
        case 11:
            printf ("ROOM_AFFECT (unused)");
            break;
        case 12:
            printf ("REGISTRY_DATA");
            break;
        case 13:
            printf ("AFFECTED_TYPE");
            break;
        case 14:
            printf ("unspace");
            break;
        case 15:
            printf ("str_dup: ");
            break;
        case 16:
            printf ("CREATE");
            break;
        case 17:
            printf ("DESCRIPTOR_DATA");
            break;
        case 18:
            printf ("OBJ_DATA");
            break;
        case 19:
            printf ("CHAR_DATA");
            break;
        case 20:
            printf ("VAR_DATA");
            break;
        case 21:
            printf ("MOBPROG_DATA");
            break;
        case 22:
            printf ("emergency data");
            break;
        case 23:
            printf ("affect_craft_type");
            break;
        case 24:
            printf ("MOVE_DATA");
            break;
        case 25:
            printf ("SUBCRAFT_HEAD_DATA");
            break;
        case 26:
            printf ("PHASE_DATA");
            break;
        case 27:
            printf ("DEFAULT_ITEM_DATA");
            break;
        case 28:
            printf ("Alias information");
            break;
        case 29:
            printf ("Delayed_affect_data");
            break;
        case 30:
            printf ("AFFECTED_TYPE");
            break;
        case 31:
            printf ("Random char array");
            break;
        case 32:
            printf ("RANDOM_CH_DATA");
            break;
        case 33:
            printf ("RESET_DATA");
            break;
        case 34:
            printf ("RESET_AFFECT");
            break;
        case 35:
            printf ("edit () buffer");
            break;
        case 36:
            printf ("HELP_DATA");
            break;
        case 37:
            printf ("TEXT_DATA");
            break;
        case 38:
            printf ("name_switch_data");
            break;
        case 39:
            printf ("CLAN_DATA");
            break;
        case 40:
            printf ("NEGOTIATION_DATA");
            break;
        case 41:
            printf ("Web buffer");
            break;
        default:
            printf (" ** Unknown origin ** ");
            break;
        }

        printf ("\n");
    }

    return p + sizeof (MEMORY_T);
}

#else /* NOVELL */

malloc_t
alloc (int bytes, int dtype)
{
    static int allocs = 0;
    char *p;
    extern char *emergency_data;
    extern int mem_allocated;

    allocs++;

    bytes += 4;

    p = (char *) calloc (1, bytes);

    mem_allocated += bytes;

    if (!p)
    {
        mem_free (emergency_data);
        system_log ("calloc failed.  Out of memory - forced to shutdown.",
                    true);
        shutd = 1;
        p = (char *) calloc (1, bytes);
        mm ("calloc failed");
    }

    strncpy (p, "ZZZZ", 4);

    if (x1)
        printf ("+ @ %ld  bytes = %d\n", (long int) p, bytes);

    bytes_allocated += bytes;

    return p + 4;
}

#endif /* NOVELL */

void
add_char (char *buf, char c)
{
    buf[strlen (buf) + 1] = '\0';
    buf[strlen (buf)] = c;
}

int
is_obj_here (CHAR_DATA * ch, OBJ_DATA * obj, int check)
{
    OBJ_DATA *tobj;

    if (ch->deleted || !ch->room)
        return 0;

    for (tobj = ch->room->contents; tobj; tobj = tobj->next_content)
        if (!tobj->deleted && tobj == obj)
            break;

    if (!tobj || (check && !CAN_SEE_OBJ (ch, tobj)))
        return 0;

    return 1;
}

int
is_he_there (CHAR_DATA * ch, ROOM_DATA * room)
{
    CHAR_DATA *tch;

    /* he could be dead, or have left the game...who knows.   We
       cannot dereference "he" cause we are uncertain of the pointer
     */

    for (tch = room->people; tch; tch = tch->next_in_room)
        if (tch == ch && !tch->deleted)
            return 1;

    if (!tch)
        return 0;

    return 1;			/* "he" is valid, and in ch's room */
}

int
is_he_here (CHAR_DATA * ch, CHAR_DATA * he, int check)
{
    CHAR_DATA *tch = NULL;

    /* "check" means make sure we can see him.

       He could be dead, or have left the game...who knows.   We
       cannot dereference "he" cause we are uncertain of the pointer
     */

    if (ch == NULL)
        return 0;

    if (he == NULL)
        return 0;

    if (ch->room)
    {
        for (tch = ch->room->people; tch; tch = tch->next_in_room)
            if (tch == he && !tch->deleted)
                break;
    }

    if (!tch || (check && !CAN_SEE (ch, tch)))
        return 0;

    return 1;			/* "he" is valid, and in ch's room */
}

int
is_he_somewhere (CHAR_DATA * he)
{
    CHAR_DATA *tch;

    if (!he)
        return 0;

    for (tch = character_list; tch; tch = tch->next)
        if (!tch->deleted && tch == he)
            return 1;

    return 0;
}

int
is_obj_in_list (OBJ_DATA * obj, OBJ_DATA * list)
{
    OBJ_DATA *tobj;
    int count = 0;

    for (tobj = list; tobj; tobj = tobj->next_content)
    {
        count++;
        if (tobj == obj)
            return count;
    }

    return 0;
}

void
name_to_ident (CHAR_DATA * ch, char *ident)
{
    int i = 1;
    CHAR_DATA *tch;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    *buf = '\0';
    *buf2 = '\0';

    if (!ch || !ident)
        return;

    if (!IS_NPC (ch) && !is_hooded (ch))
    {
        sprintf (ident, "%s", ch->tname);
        return;
    }

    one_argument (char_names (ch), buf);

    for (tch = ch->room->people; ch != tch; tch = tch->next_in_room)
    {
        one_argument (char_names (tch), buf2);
        if (CAN_SEE (ch, tch) && !strcmp (buf, buf2))
            i++;
    }

    if (i == 1)
        sprintf (ident, "%s", buf);
    else
        sprintf (ident, "%d.%s", i, buf);
}

/*
void name_to_ident (CHAR_DATA *ch, char *ident)
{
	int			i = 1;
	CHAR_DATA	*tch;

	if ( !IS_NPC (ch) ) {
		strcpy (ident, GET_NAME (ch));
		return;
	}

	for ( tch = ch->room->people; ch != tch; tch = tch->next_in_room )
		if ( CAN_SEE (ch, tch) && isname (GET_NAME (ch), GET_NAMES (tch)) )
			i++;

	sprintf (ident, "%d.%s", i, GET_NAME (ch));
}
*/


int
real_trust (CHAR_DATA * ch)
{
    if (!ch || !ch->descr())
        return 0;

    ch = ch->descr()->original != NULL ? ch->descr()->original : ch->descr()->character;

    if (!ch || !ch->pc)
        return 0;

    return ch->pc->level;
}

int
get_trust (CHAR_DATA * ch)
{
    if (!ch || !ch->descr() || IS_SET (ch->flags, FLAG_GUEST))
        return 0;


	if (ch->descr()->original)
	{
		ch = ch->descr()->original;
	}
	else
	{
		ch = ch->descr()->character;
	}

    if (!ch || !ch->pc || ch->pc->mortal_mode)
        return 0;

    return ch->pc->level;
}

CHAR_DATA *
get_pc (char *buf)
{
    CHAR_DATA *ch;

    if (!buf || !*buf)
        return NULL;

    for (ch = character_list; ch; ch = ch->next)
    {

        if (ch->deleted || IS_NPC (ch) || !GET_NAME (ch))
            continue;

        if (!str_cmp (GET_NAME (ch), buf))
            return ch;
    }

    return NULL;
}

CHAR_DATA *
get_pc_dead (const char *buf)
{
    CHAR_DATA *ch;

    if (!*buf)
        return NULL;

    for (ch = character_list; ch; ch = ch->next)
    {
        if (ch->deleted)
            continue;
        if (!GET_NAME (ch))
            continue;
        if (!IS_NPC (ch) && !str_cmp (GET_NAME (ch), buf))
            return ch;
    }

    return NULL;
}

CHAR_DATA *
load_pc (const char *buf)
{
    CHAR_DATA *ch = NULL;
    char buf2[MAX_STRING_LENGTH];

    if (!buf || !*buf)
        return NULL;

    if ((ch = get_pc_dead (buf)))
    {
        ch->pc->load_count++;
        sprintf (buf2, "(online) Loading %s, count %d", buf,
                 ch->pc->load_count);
        system_log (buf2, false);
        return ch;
    }

    for (ch = loaded_list; ch; ch = ch->next)
    {
        if (IS_NPC (ch))
            continue;
        if (!str_cmp (GET_NAME (ch), buf))
        {
            ch->pc->load_count++;
            sprintf (buf2, "(loaded) Load list %s, count %d",
                     buf, ch->pc->load_count);
            system_log (buf2, false);
            return ch;
        }
    }

    for (ch = character_list; ch; ch = ch->next)
    {

        if (ch->deleted)
            continue;

        if (!ch->pc)
            continue;

        if (!GET_NAME (ch))
            continue;

        if (!str_cmp (GET_NAME (ch), buf))
        {
            ch->pc->load_count++;
            sprintf (buf2, "(loaded) char list %s, count %d",
                     buf, ch->pc->load_count);
            system_log (buf2, false);
            return ch;
        }
    }

    if (!(ch = load_char_mysql (buf)))
    {
        return NULL;
    }

    ch->next = loaded_list;
    loaded_list = ch;

    ch->pc->load_count = 1;

    sprintf (buf2, "(loading) first %s, count %d", buf, ch->pc->load_count);
    system_log (buf2, false);

    return ch;
}

void
unload_pc (CHAR_DATA * ch)
{
    CHAR_DATA *tch;
    char buf[MAX_STRING_LENGTH];

    if (!ch || !ch->tname || !*ch->tname)
    {
        ch = NULL;
        return;
    }

    if (islower (*GET_NAME (ch)))
        *GET_NAME (ch) = toupper (*GET_NAME (ch));

    if (ch->pc)
        sprintf (buf, "(unload) Unloading %s (lc = %d)", GET_NAME (ch),
                 ch->pc->load_count);
    else
        sprintf (buf, "Unloading char: %s\n", GET_NAME (ch));

    system_log (buf, false);

    if (!ch->pc->load_count)
    {
        system_log ("Uh oh, PC was not loaded!", true);
        system_log (GET_NAME (ch), true);
        return;
    }

    ch->pc->load_count--;

    if (ch->deleted)
    {
        ch->deleted = 0;
        ch->next = loaded_list;
        loaded_list = ch;
    }
    else
    {

        sprintf (buf, "Saving character %s", GET_NAME (ch));
        system_log (buf, false);

        save_char (ch, false);	/* No need to save when deleted set */
    }

    if (ch->pc->load_count)
    {
        return;
    }

    /* remove ch from loaded_list */

    if (loaded_list == ch)
        loaded_list = ch->next;

    else
    {
        for (tch = loaded_list; tch; tch = tch->next)
        {
            if (ch == tch->next)
            {
                tch->next = ch->next;
                break;
            }
        }
    }

    free_char (ch);
}

void
pc_to_game (CHAR_DATA * ch)
{
    CHAR_DATA *tch;

    if (get_pc (GET_NAME (ch)))
        return;

    if (loaded_list == ch)
        loaded_list = ch->next;
    else
    {
        for (tch = loaded_list; tch && tch->next; tch = tch->next)
        {
            if (!str_cmp (GET_NAME (ch), GET_NAME (tch->next)))
            {
                tch->next = ch->next;
                break;
            }
        }
    }

    ch->next = character_list;
    character_list = ch;

    if (!ch->writes)
    {
        if (real_skill (ch, SKILL_COMMON))
            ch->writes = SKILL_COMMON;
    }
}

int
is_dark (ROOM_DATA * room)
{
    if (!room)
        return 1;

    if (is_room_affected (room->affects, MAGIC_ROOM_DARK))
        return 1;

    if (IS_SET (room->room_flags, DARK))
        return 1;

    if (!IS_SET (room->room_flags, INDOORS)
            && (sun_light || moon_light[room->zone]))
        return 0;

    if (room->light)
        return 0;

    if (IS_SET (room->room_flags, LIGHT))
        return 0;

    if (is_room_affected (room->affects, MAGIC_ROOM_LIGHT))
        return 0;

    return 1;
}

int
is_blind (CHAR_DATA * ch)
{

    //AFFECTED_TYPE *af;

    if (GET_TRUST (ch))
        return 0;

    if (get_equip (ch, WEAR_BLINDFOLD))
        return 1;

    /*
    if(af = get_soma_affect(ch, SOMA_PLANT_BLINDNESS))
      if(af->a.soma.atm_power > 750)
        return 1;
    */

    return 0;
}


int
is_goggled (CHAR_DATA * ch)
{
	OBJ_DATA *obj = NULL;
    //AFFECTED_TYPE *af;

    if (GET_TRUST (ch))
        return 0;

    if ((obj = (get_equip (ch, WEAR_EYES))))
	{
		if (GET_ITEM_TYPE(obj) == ITEM_E_GOGGLE)
		{
			if (obj->o.elecs.status)
			{
				return 1;
			}
		}
	}

    return 0;

    /*
    if(af = get_soma_affect(ch, SOMA_PLANT_BLINDNESS))
      if(af->a.soma.atm_power > 750)
        return 1;
    */

    return 0;
}

int
has_breather (CHAR_DATA * ch)
{
	OBJ_DATA *obj = NULL;
    //AFFECTED_TYPE *af;

    if (GET_TRUST (ch))
        return 0;

    if ((obj = (get_equip (ch, WEAR_FACE))))
	{
		if (GET_ITEM_TYPE(obj) == ITEM_E_BREATHER)
		{
			if (obj->o.elecs.status)
			{
				return 1;
			}
		}
	}

    return 0;

    /*
    if(af = get_soma_affect(ch, SOMA_PLANT_BLINDNESS))
      if(af->a.soma.atm_power > 750)
        return 1;
    */

    return 0;
}

CHAR_DATA *
being_dragged (CHAR_DATA * ch)
{
    CHAR_DATA *tch;
    AFFECTED_TYPE *af;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {

        if (tch->deleted || tch == ch)
            continue;

        if ((af = get_affect (tch, MAGIC_DRAGGER)) && af->a.spell.t == (long int) ch)
            return tch;
    }

    return NULL;
}

int
get_weight (CHAR_DATA * ch)
{
    int bmi = 0;
    int weight = 0;

    if (ch->frame && !ch->bmi)
    {
        if (ch->frame == 1)
            bmi = number (19, 20);
        else if (ch->frame == 2)
            bmi = number (21, 22);
        else if (ch->frame == 3)
            bmi = number (23, 24);
        else if (ch->frame == 4)
            bmi = number (25, 26);
        else if (ch->frame == 5)
            bmi = number (27, 28);
        else if (ch->frame >= 6)
            bmi = number (29, 35);
        else
            bmi = number (22, 25);
        ch->bmi = bmi;
    }
    else if (ch->bmi)
        bmi = ch->bmi;
    else
    {
        // Let's assume medium BMI unless otherwise
        bmi = number (21, 26);
        ch->bmi = bmi;
    }

    // formula: bmi = (weight/2.2046) / (height/39.3696)squared
    weight = (ch->height * ch->height * bmi) / 704;

    // weight in pounds based on BMI and height

    return (100*weight);
}

DESCRIPTOR_DATA *
is_pc_attached (char *buf)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d; d = d->next)
    {

        if (!d->character)
            continue;

        if (!str_cmp (d->character->tname, buf))
            return d;
    }

    return NULL;
}

/*
Proposed:

63-70     XS  |  XS   |  XS   |  XS   |   XS  |   XS  |  XS   |   XS  |  XS | XS
71-78     XS  |  XS   |  XS   |  XS   |   XS  |   XS  |  XS   |   XS  |  XS | XS
79-86     XS  |  XS   |  XS   |  XS   |   XS  |   XS  |  XS   |   XS  |  XS | S
87-94     XS  |  XS   |  XS   |  XS   |   XS  |   XS  |  XS   |   XS  |  S  | S
95-102    XS  |  XS   |  XS   |  XS   |   XS  |   XS  |  XS   |   S   |  S  | S
103-110   XS  |  XS   |  XS   |  XS   |   XS  |   XS  |   S   |   S   |  S  | S
111-118   XS  |  XS   |  XS   |  XS   |   XS  |   S   |   S   |   S   |  S  | M
119-126   XS  |  XS   |  XS   |  XS   |   S   |   S   |   S   |   S   |  S  | M
127-134   XS  |  XS   |  XS   |   S   |   S   |   S   |   S   |   S   |  M  | M
135-142   XS  |  XS   |   S   |   S   |   S   |   S   |   S   |   M   |  M  | M
143-150   S   |   S   |   S   |   S   |   S   |   S   |   M   |   M   |  M  | M
151-158   S   |   S   |   S   |   S   |   S   |   M   |   M   |   M   |  M  | M
159-166   S   |   S   |   S   |   S   |   M   |   M   |   M   |   M   |  M  | M
167-174   S   |   S   |   S   |   M   |   M   |   M   |   M   |   M   |  M  | L
175-182   S   |   S   |   M   |   M   |   M   |   M   |   M   |   M   |  M  | L
183-190   S   |   M   |   M   |   M   |   M   |   M   |   M   |   M   |  L  | L
191-198   M   |   M   |   M   |   M   |   M   |   M   |   M   |   M   |  L  | L
199-206   M   |   M   |   M   |   M   |   M   |   M   |   M   |   L   |  L  | L
207-214   M   |   M   |   M   |   M   |   M   |   L   |   L   |   L   |  L  | XL
215-222   M   |   M   |   M   |   M   |   M   |   L   |   L   |   L   |  XL | XL
223-230   M   |   M   |   M   |   M   |   L   |   L   |   L   |   XL  |  XL | XL
231+      M   |   M   |   M   |   L   |   L   |   L   |   XL  |   XL  |  XL | XL
              |       |       |       |       |       |       |       |     |
        41-44 | 45-48 | 49-52 | 53-55 | 56-59 | 60-62 | 63-66 | 67-70 |71-74|75+

Used:

 61: XXS XXS XXS XXS XXS XXS XXS XXS XXS XXS
 68:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
 75:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
 82:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
 89:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
 96:  XS  XS  XS  XS  XS  XS  XS  XS   S   S
103:  XS  XS  XS  XS  XS  XS   S   S   S   S
110:  XS  XS  XS  XS   S   S   S   S   S   S
117:  XS  XS  XS   S   S   S   S   S   S   S
124:  XS   S   S   S   S   S   S   S   S   S
131:   S   S   S   S   S   S   S   S   M   M
138:   S   S   S   S   S   S   M   M   M   M
145:   S   S   S   S   M   M   M   M   M   M
152:   S   S   S   M   M   M   M   M   M   M
159:   S   M   M   M   M   M   M   M   M   M
166:   M   M   M   M   M   M   M   M   M   L
173:   M   M   M   M   M   M   M   M   L   L
180:   M   M   M   M   M   M   L   L   L   L
187:   M   M   M   M   L   L   L   L   L   L
194:   M   M   L   L   L   L   L   L   L   L
201:   M   L   L   L   L   L   L   L   L   L
208:   L   L   L   L   L   L   L   L   L  XL
215:   L   L   L   L   L   L   L  XL  XL  XL
222:   L   L   L   L   L  XL  XL  XL  XL  XL
229:   L   L   L   L  XL  XL  XL  XL  XL  XL
236: XXL XXL XXL XXL XXL XXL XXL XXL XXL XXL
      40  44  48  52  56  60  64  68  72  76

*/

int
get_size (CHAR_DATA * ch)
{
    int size = -1;
    int height;
    int weight;

    height = ch->height;
    weight = get_weight (ch) / 100;

    if (weight < 63)
        size = 1;			/* XXS */
    else if (weight > 250)
        size = 7;
    else if ((height - 41) + weight < 107)
        size = 2;			// XS, e.g. 58 and less than 90 lbs
    else if ((height - 41) + weight < 160)
        size = 3;			/* S   */
    else if ((height - 41) + weight < 201)
        size = 4;			/* M   */
    else if ((height - 41) + weight < 241)
        size = 5;			/* L   */
    else if ((height - 41) + weight < 280)
        size = 6;			/* XL  */
    else
        size = 7;

    return size;
}

void
release_pc (CHAR_DATA * ch)
{
    if (!ch || !ch->pc)
        return;

    if (ch->pc->load_count)
        unload_pc (ch);

    free_char (ch);
}

void
release_nonplaying_pc (CHAR_DATA * ch)
{
    if (!ch || !ch->pc)
        return;

    if (!ch->descr() || ch->descr()->connected != CON_PLYNG)
        release_pc (ch);
}


/* Call with criminal as NULL if you just want to know if victim
   is guarded in general.

   Call with victim as NULL if you want to know if a guard is watching.
 */

CHAR_DATA *
is_guarded (CHAR_DATA * victim, CHAR_DATA * criminal)
{
    CHAR_DATA *tch;

    if (victim && !IS_SET (victim->room->room_flags, LAWFUL))
        return NULL;

    if (criminal && !IS_SET (criminal->room->room_flags, LAWFUL))
        return NULL;

    if (victim)
        tch = victim->room->people;
    else
        tch = criminal->room->people;

    for (; tch; tch = tch->next_in_room)
    {

        if (tch == criminal)
            continue;

        if (!AWAKE (tch))
            continue;

        if (!is_area_enforcer (tch))	/* enforcer clan */
            continue;

        if (victim && !CAN_SEE (tch, victim))
            continue;

        if (criminal && !CAN_SEE (tch, criminal))
            continue;

        break;
    }

    return tch;
}

OBJ_DATA *
is_at_table (CHAR_DATA * ch, OBJ_DATA * table)
{
    AFFECTED_TYPE *af;
    OBJ_DATA *obj;

    if (!(af = get_affect (ch, MAGIC_SIT_TABLE)) && !(af = get_affect (ch, AFFECT_COVER)))
        return NULL;

    if (table && table == af->a.table.obj)
        return table;

    if (table)
        return NULL;

    for (obj = ch->room->contents; obj; obj = obj->next_content)
        if (obj == af->a.table.obj)
            return obj;

    return NULL;
}

int
would_reveal (CHAR_DATA * ch)
{
    AFFECTED_TYPE *was_hidden;
    CHAR_DATA *tch;

    /* Remove the hide affect to test CAN_SEE.  We don't reveal the
       PC if s/he couldn't be seen anyway. */

    if ((was_hidden = get_affect (ch, MAGIC_HIDDEN)))
        affect_remove (ch, was_hidden);

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {

        if (tch == ch)
            continue;

        if (!AWAKE (tch))
            continue;

        if (GET_TRUST (tch))	/* Imms don't count */
            continue;

        if (GET_FLAG (tch, FLAG_WIZINVIS))
			continue;

        if (!CAN_SEE (tch, ch))
            continue;

        if (are_grouped (tch, ch))
            continue;

        if (was_hidden)
            magic_add_affect (ch, MAGIC_HIDDEN, -1, 0, 0, 0, 0);

        return 1;
    }

    if (was_hidden)
        magic_add_affect (ch, MAGIC_HIDDEN, -1, 0, 0, 0, 0);

    return 0;
}

bool
	can_see (CHAR_DATA *sub, CHAR_DATA *obj)
{
	// Immortals can always see all.
	if (!IS_MORTAL(sub))
		return true;

	// If you're not immortal, you're not seeing the wizinvis dude.
	if (IS_SET(obj->flags, FLAG_WIZINVIS))
		return false;

	// Blind people always see nothing.
	if (is_blind(sub))
		return false;

	AFFECTED_TYPE *af = NULL;
	// Now we need to work out which of the three things are going to stop
	// us from seeing - light, hidden, or invisibility.
	bool need_light = false, need_hidden = false, need_invisible = false;

	// If the room's dark, we need light.
	if (is_dark(obj->room))
	{
		need_light = true;
	}

	// If our object is hidden or invisible, then we'll need those.
    for (af = obj->hour_affects; af; af = af->next)
	{
		if (af->type == MAGIC_HIDDEN)
			need_hidden = true;

		if (af->type == MAGIC_AFFECT_INVISIBILITY)
			need_invisible = true;
	}

	// If our object was hidden, are we grouped to them?
	// If so, then they're not hidden from us.
	if (need_hidden)
	{
		if (are_grouped(obj, sub))
		{
			need_hidden = false;
		}
	}

	// Did we need light or invisibility?
	if (need_light || need_invisible)
	{
		// If so, if we've got infravision or goggles, we don't need light anymore.
		if (IS_SET (sub->affected_by, AFF_INFRAVIS) || is_goggled(sub))
		{
			need_light = false;
		}

		// If we still need light or invisibility, check our affects to see if we
		// have the right ones.
		if (need_light || need_invisible)
		{
			for (af = sub->hour_affects; af; af = af->next)
			{
				if (af->type == MAGIC_AFFECT_INFRAVISION)
					need_light = false;

				if (af->type == MAGIC_AFFECT_SEE_INVISIBLE)
					need_invisible = false;
			}
		}
	}

	// If we still need anything, we're not seeing anything.
	if (need_light || need_invisible || need_hidden)
		return false;
	// If we didn't need anything, then we can see.
	else
		return true;
}

int
could_see (CHAR_DATA * ch, CHAR_DATA * target)
{
    if (ch->room == target->room)
        return CAN_SEE (ch, target);

	ROOM_DATA *temp_room = NULL;
    int seen;
    int target_room_light;

    /* Determine if ch could see target if ch were in target's room */
	target_room_light = target->room->light;
    temp_room = ch->room;

	ch->room = target->room;
	seen = CAN_SEE(ch, target);

	ch->room = temp_room;
	target->room->light = target_room_light;

    return seen;

    //int temp_room;
    //int seen;
    //int target_room_light;

    ///* Determine if ch could see target if ch were in target's room */

    //if (ch->room == target->room)
    //    return CAN_SEE (ch, target);

    //temp_room = ch->in_room;

    //char_from_room (ch);

    //target_room_light = target->room->light;

    //char_sight_room (ch, target->in_room);

    ///* Adding the character to the room may also bring his light.
    //   We don't want that. */

    //target->room->light = target_room_light;

    //seen = CAN_SEE (ch, target);

    //char_from_room (ch);
    //char_to_room (ch, temp_room);

    //return seen;
}

int
could_see_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{

    if (ch->in_room == obj->in_room || !obj->in_room || obj->in_room == -1)
        return CAN_SEE_OBJ (ch, obj);

	ROOM_DATA *temp_room = NULL;
    int seen;
    int target_room_light;

    /* Determine if ch could see target if ch were in target's room */
	target_room_light = vnum_to_room(obj->in_room)->light;
    temp_room = ch->room;

	ch->room = vnum_to_room(obj->in_room);
	seen = CAN_SEE_OBJ(ch, obj);

	ch->room = temp_room;
	vnum_to_room(obj->in_room)->light = target_room_light;

    return seen;

    //int temp_room;
    //int seen;
    //int target_room_light;
    //ROOM_DATA *troom;

    ///* Determine if ch could see target if ch were in target's room */

    //if (ch->in_room == obj->in_room || !obj->in_room || obj->in_room == -1)
    //    return CAN_SEE_OBJ (ch, obj);

    //temp_room = ch->in_room;

    //char_from_room (ch);

    //troom = vtor (obj->in_room);
    //target_room_light = troom->light;

    //char_sight_room (ch, obj->in_room);

    ///* Adding the character to the room may also bring his light.
    //   We don't want that. */

    //troom->light = target_room_light;

    //seen = CAN_SEE_OBJ (ch, obj);

    //char_from_room (ch);
    //char_to_room (ch, temp_room);

    //return seen;
}



const char *cardlong[] = {"face-down", "private", "face-up", "decked", "burned"};

char *
obj_short_desc (OBJ_DATA * obj)
{
    int bite_num;
    int total_bites;
    OBJ_DATA *tobj;
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH] = {"\0"};
    static char description[MAX_STRING_LENGTH];
    static char coins[MAX_STRING_LENGTH];
    static char food[MAX_STRING_LENGTH];
    static char drinkcon[MAX_STRING_LENGTH];
    char *argument;
    int tier = 0;

    if (!obj)
        return NULL;

    *buf = '\0';

    if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
    {

        if (obj->count == 1)
            sprintf (coins, "one");

        else if (obj->count == 2)
            sprintf (coins, "two");

        else if (obj->count == 3)
            sprintf (coins, "three");

        else if (obj->count == 4)
            sprintf (coins, "four");

        else if (obj->count > 2501)	/* more than 2500 coins */
            sprintf (coins, "an enormous pile of");

        else if (obj->count > 1001)	/* 1001 - 2500 coins */
            sprintf (coins, "a huge pile of");

        else if (obj->count > 101)	/* 101 - 1000 coins */
            sprintf (coins, "a big pile of");

        else if (obj->count > 51)	/* 51 - 100 coins */
            sprintf (coins, "a pile of");

        else if (obj->count > 21)	/* 21 - 50 coins */
            sprintf (coins, "a small pile of");

        else			/* 5 - 20 coins */
            sprintf (coins, "a handful of");

        if (obj->nVirtual == 50093)
        {
            if (obj->count == 1)
                strcat (coins, " 'two-hundred-forty'-marked piece of scrip");
            else
                strcat (coins, " 'two-hundred-forty'-marked pieces of scrip");
        }
        else if (obj->nVirtual == 50092)
        {
            if (obj->count == 1)
                strcat (coins, " 'twenty-five'-marked piece of scrip");
            else
                strcat (coins, " 'twenty-five'-marked pieces of scrip");
        }

        else if (obj->nVirtual == 50091)
        {
            if (obj->count == 1)
                strcat (coins, " 'five'-marked piece of scrip");
            else
                strcat (coins, " 'five'-marked pieces of scrip");
        }

        else if (obj->nVirtual == 50090)
        {
            if (obj->count == 1)
                strcat (coins, " 'one'-marked piece of scrip");
            else
                strcat (coins, " 'one'-marked pieces of scrip");
        }
        else if (obj->nVirtual == 14011)
        {
            if (obj->count > 1)
                strcat (coins, " white, circular chips");
            else
                strcat (coins, " white, circular chip");
        }
        else if (obj->nVirtual == 14012)
        {
            if (obj->count > 1)
                strcat (coins, " red, circular chips");
            else
                strcat (coins, " red, circular chip");
        }
        else if (obj->nVirtual == 14013)
        {
            if (obj->count > 1)
                strcat (coins, " blue, circular chips");
            else
                strcat (coins, " blue, circular chip");
        }
        else if (obj->nVirtual == 14014)
        {
            if (obj->count > 1)
                strcat (coins, " yellow, circular chips");
            else
                strcat (coins, " yellow, circular chip");
        }
        else if (obj->nVirtual == 14015)
        {
            if (obj->count > 1)
                strcat (coins, " green, circular chips");
            else
                strcat (coins, " green, circular chip");
        }
        else if (obj->nVirtual == 14016)
        {
            if (obj->count > 1)
                strcat (coins, " black, circular chips");
            else
                strcat (coins, " black, circular chip");
        }
        else if (obj->nVirtual == 14017)
        {
            if (obj->count > 1)
                strcat (coins, " gold-flecked, circular chips");
            else
                strcat (coins, " gold-flecked, circular chip");
        }
        return coins;
    }
    else if (GET_ITEM_TYPE (obj) == ITEM_CARD)
    {
        if (obj->o.od.value[5] == 1)
            sprintf (coins, "one %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] == 2)
            sprintf (coins, "two %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] == 3)
            sprintf (coins, "three %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] == 4)
            sprintf (coins, "four %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] == 5)
            sprintf (coins, "five %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] <= 10)
            sprintf (coins, "a small pile of %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] <= 20)
            sprintf (coins, "a pile of %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] <= 30)
            sprintf (coins, "a moderate pile of %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] <= 51)
            sprintf (coins, "a large pile of %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else if (obj->o.od.value[5] >= 52)
            sprintf (coins, "a full deck of %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));
        else
            sprintf (coins, "one %s %s%s", cardlong[obj->o.od.value[4]], obj->short_description, (obj->o.od.value[5] == 1 ? "" : "s"));

        return coins;
    }
    else if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON)
    {
		if (!(tobj = obj->contains))
			return obj->short_description;

        sprintf (drinkcon, "%s filled with %s",
                 obj->short_description, tobj->short_description);
        return drinkcon;
    }
	else if (GET_ITEM_TYPE(obj) == ITEM_FLUID)
	{
		return obj->short_description;
	}

    // Bows need to display what level of stringing they're at.

    else if (GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
             (obj->o.od.value[3] == SKILL_AIM))
    {

        argument = one_argument (obj->short_description, buf);

        sprintf(description, "%s %s", (obj->o.od.value[5] == 2) ? "a broken stringed," :
                obj->o.od.value[5] ? "a strung," : "an unstrung,", argument);
        return description;
    }

    else if (GET_ITEM_TYPE (obj) == ITEM_FOOD && obj->count <= 1)
    {

        bite_num = obj->o.food.bites;

        total_bites = vtoo (obj->nVirtual)->o.food.bites;

        if (bite_num > total_bites)
            total_bites = bite_num;

        if (!bite_num || bite_num == total_bites)
            return obj->short_description;

        total_bites = MAX (1, total_bites);

        switch (((bite_num - 1) * 7) / total_bites)
        {
        case 0:
            sprintf (food, "scraps of %s", obj->short_description);
            break;

        case 1:
            sprintf (food, "a small amount of %s", obj->short_description);
            break;

        case 2:
            sprintf (food, "less than half of %s", obj->short_description);
            break;

        case 3:
            sprintf (food, "half of %s", obj->short_description);
            break;

        case 4:
            sprintf (food, "more than half of %s", obj->short_description);
            break;

        case 5:
            sprintf (food, "%s that was bitten", obj->short_description);
            break;

        case 6:
            sprintf (food, "%s with a bite taken out", obj->short_description);
            break;
        }

        return food;
    }

    if (obj->count <= 1)
    {
        bool enviro_cond = false;
        int highest = 0;


        // Find the highest enviro_cond provided it's greater than 25.
        for (int ind = 0; ind < COND_TYPE_MAX; ind++)
        {
            if (obj->enviro_conds[ind] > 25)
            {
                enviro_cond = true;
                if (obj->enviro_conds[ind] > obj->enviro_conds[highest])
                {
                    highest = ind;
                }
            }
        }

        if (enviro_cond)
        {
            // Get our first token (the, an, a, some)
            argument = one_argument (obj->short_description, buf);

            // If it is an, we need to change it for grammatical correctness.
            if (!str_cmp(buf, "an") || !str_cmp(buf, "An"))
            {
                sprintf(buf, "a");
            }

            // Now we get the next token so we know if we need to add a comma or not.
            one_argument (argument, buf2);

            int tier = obj->enviro_conds[highest] > 75 ? 4 : obj->enviro_conds[highest] > 50 ? 3 : 2;

            // Now we just slap in whichever was the highest -- so long as it's not both water and dirt,
            // which then becomes mud.

            if ((highest == COND_WATER || highest == COND_DIRT) && obj->enviro_conds[COND_WATER] > 25 && obj->enviro_conds[COND_DIRT] > 25)
            {
                sprintf (description, "%s muddy%s %s", buf, (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);
            }
            else
            {
                sprintf (description, "%s %s%s %s", buf, enviro_desc[highest][tier], (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);
            }

            return description;
        }


    }

    // To make spotting damage easier, we have a quick check to see whether we've got
    // damage on our object our not.
    if (obj->count <= 1 && obj->damage && (tier = object__determine_condition(obj)))
    {
        // Get our first token (the, an, a, some)
        argument = one_argument (obj->short_description, buf);

        // If it is an, we need to change it for grammatical correctness.
        if (!str_cmp(buf, "an") || !str_cmp(buf, "An"))
        {
          sprintf(buf, "a");
        }

        // Now we get the next token so we know if we need to add a comma or not.
        one_argument (argument, buf2);

        // Depending on how damage we are will change the variable,
        // Basically, it's first token + new descriptor + rest of the description, with an optional comma
        // if we've already got commas in our sdesc.
        if (tier == 1)
        {
            sprintf (description, "%s worn%s %s", buf, (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);
        }
        else if (tier == 2 || tier == 3)
        {
            sprintf (description, "%s damaged%s %s", buf, (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);
        }
        else if (tier >= 4)
        {
            sprintf (description, "%s ruined%s %s", buf, (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);
        }
        return description;
    }

    if (obj->count <= 1 && obj->dec_short > 0 && GET_ITEM_TYPE(obj) != ITEM_ARTWORK)
    {
        // Get our first token (the, an, a, some)
        argument = one_argument (obj->short_description, buf);

        // If it is an, we need to change it for grammatical correctness.
        if (!str_cmp(buf, "an") || !str_cmp(buf, "An"))
        {
          sprintf(buf, "a");
        }

        if (*dec_short[obj->dec_short] == 'a' ||
            *dec_short[obj->dec_short] == 'e' ||
            *dec_short[obj->dec_short] == 'i' ||
            *dec_short[obj->dec_short] == 'o' ||
            *dec_short[obj->dec_short] == 'u')
                sprintf(buf, "an");

        // Now we get the next token so we know if we need to add a comma or not.
        one_argument (argument, buf2);

        // We then slot in our desc_short as appropriate.
        sprintf (description, "%s %s%s %s", buf, dec_short[obj->dec_short], (buf2[strlen(buf2) -1] == ',' ? "," : ""), argument);

        return description;
    }

    if (obj->count <= 1)
        return obj->short_description;

    argument = one_argument (obj->short_description, buf);

    if (!str_cmp (buf, "a") || !str_cmp (buf, "an") || !str_cmp (buf, "the"))
        sprintf (buf, "%d %s%s", obj->count, argument,
                 (argument[strlen (argument) - 1] != 's') ? "s" : "");
    else
        sprintf (buf, "%s (x%d)", obj->short_description, obj->count);

    if (strlen (buf) > 158)
    {
        memcpy (description, buf, 158);
        description[159] = '\0';
    }
    else
        strcpy (description, buf);

    return description;
}

char *
obj_desc (OBJ_DATA * obj)
{
    int bite_num;
    int total_bites;
    static char buf[MAX_STRING_LENGTH];
    static char description[160];

    if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
    {
        if (obj->count > 1 && obj->count < 5)
            sprintf (description, "There are %s here.", obj_short_desc (obj));
        else
            sprintf (description, "There is %s here.", obj_short_desc (obj));

        return description;
    }
    else if (GET_ITEM_TYPE (obj) == ITEM_CARD)
    {
        if (obj->o.od.value[5]> 1 && obj->o.od.value[5] < 52)
            sprintf (description, "There are %s here.", obj_short_desc (obj));
        else
            sprintf (description, "There is %s here.", obj_short_desc (obj));

        return description;
    }

    if (obj->obj_flags.type_flag == ITEM_FOOD)
    {
        bite_num = obj->o.food.bites;
        total_bites = vtoo (obj->nVirtual)->o.food.bites;

        if (!bite_num || bite_num >= total_bites)
            sprintf (description, "%s", obj->description);

        else
            switch (((bite_num - 1) * 7) / total_bites)
            {
            case 0:
                sprintf (description, "Scraps of %s have been left here.",
                         obj->short_description);
                break;

            case 1:
                sprintf (description, "A small amount of %s has been left "
                         "here.", obj->short_description);
                break;

            case 2:
                sprintf (description, "%s is here, more than half eaten.",
                         obj->short_description);
                break;

            case 3:
                sprintf (description, "%s is here, half eaten.",
                         obj->short_description);
                break;

            case 4:
                sprintf (description, "%s is here, partialy eaten.",
                         obj->short_description);
                break;

            case 5:
                sprintf (description, "%s with a couple of bites taken out "
                         "is here.", obj->short_description);
                break;

            case 6:
                sprintf (description, "%s with a bite taken out is here.",
                         obj->short_description);
                break;
            }

        *description = toupper (*description);
    }
    else
        sprintf (description, "%s", obj->description);

    if (obj->count <= 1)
        return obj->description;

    sprintf (buf, "%s #2(x%d)#0", description, obj->count);

    return buf;
}

int
can_move (CHAR_DATA * ch)
{
    AFFECTED_TYPE *af;

    if ((af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)) ||
            IS_SUBDUEE (ch) ||
            GET_POS (ch) == POSITION_STUNNED
            || GET_POS (ch) == POSITION_UNCONSCIOUS)
        return 0;

    return 1;
}

int
odds_sqrt (int percent)
{
    /* I had a bit of trouble getting sqrt to link in with the mud on
       Novell's Unix (UNIX_SV), so I came up with this table.  The index
       is the a percent, and the table lookup is the square root of the
       percent (x100).  The table goes from 0 to 119 */

    const int sqrt_tab[] =
    {
        00, 10, 14, 17, 20, 22, 24, 26, 28, 29, 31, 33, 34, 36, 37, 38,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 50, 50, 51, 52, 53, 54,
        55, 56, 57, 58, 59, 59, 60, 61, 62, 63, 64, 64, 65, 66, 67,
        67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 74, 75, 76, 76, 77,
        78, 78, 79, 80, 80, 81, 81, 82, 83, 83, 84, 84, 85, 86, 86,
        87, 87, 88, 88, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94,
        95, 95, 96, 96, 97, 97, 98, 98, 99, 100, 100, 100, 101, 101, 102,
        102, 103, 103, 104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109
    };

    if (percent < 0)
        return 0;

    if (percent >= 120)
        return percent;

    return sqrt_tab[percent];
}

void
room_light (ROOM_DATA * room)
{
    int light = 0;
    CHAR_DATA *tch;
    OBJ_DATA *obj;
    OBJ_DATA *table_obj;

    if (!room)
        return;

    if (room->people != NULL)
    {
        for (tch = room->people; tch; tch = tch->next_in_room) // check to see if anyone is holding a light
        {

            if (tch->deleted)
                continue;

            if ((tch->right_hand && GET_ITEM_TYPE (tch->right_hand) == ITEM_LIGHT && tch->right_hand->o.light.hours && tch->right_hand->o.light.on) ||
				(tch->right_hand && GET_ITEM_TYPE (tch->right_hand) == ITEM_E_LIGHT && tch->right_hand->o.elecs.status))
                light++;

            if ((tch->left_hand && GET_ITEM_TYPE (tch->left_hand) == ITEM_LIGHT && tch->left_hand->o.light.hours && tch->left_hand->o.light.on) ||
				(tch->left_hand && GET_ITEM_TYPE (tch->left_hand) == ITEM_E_LIGHT && tch->left_hand->o.elecs.status))
                light++;

            if (tch->equip != NULL)
            {
                for (obj = tch->equip; obj; obj = obj->next_content)
                {
					if ((obj->obj_flags.type_flag == ITEM_LIGHT && obj->o.light.hours && obj->o.light.on) ||
						(obj->obj_flags.type_flag == ITEM_E_LIGHT && obj->o.elecs.status))
                        light++;
                }
            }//if (tch->equip != NULL)
        }//for (tch = room->people;
    }//if (room->people != NULL)

    if (room->contents != NULL)
    {
        for (obj = room->contents; obj; obj = obj->next_content) // look to see if there are any lights in the room
            /* Need to iterate through table objects to see if there is a lantern lit - Methuselah */
        {
            if (obj->next_content && obj->next_content == obj)
                obj->next_content = NULL;
            if ((obj->obj_flags.type_flag == ITEM_LIGHT && obj->o.light.hours && obj->o.light.on) ||
				(obj->obj_flags.type_flag == ITEM_E_LIGHT && obj->o.elecs.status))
            {
                light++;
            }
            if (IS_TABLE(obj))
            {
                for (table_obj = obj->contains; table_obj; table_obj = table_obj->next_content)
                {
                    if ((table_obj->obj_flags.type_flag == ITEM_LIGHT && table_obj->o.light.hours && table_obj->o.light.on) ||
						 (table_obj->obj_flags.type_flag == ITEM_E_LIGHT && table_obj->o.elecs.status))
                    {
                        light++;
                    }
                }
            }
        }
    }

    room->light = light;
}

char *
type_to_spell_name (int type)
{
    char *p;

    if ((p = lookup_string (type, REG_MAGIC_SPELLS)))
        return p;

    return "(not a spell)";
}

int
can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
    AFFECTED_TYPE *af;
    char *clan = NULL;

    if (!ch || !obj)
        return 0;

    if (!IS_MORTAL (ch))
        return 1;

    if (IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC))
        return 0;

    if (is_blind (ch))
        return 0;

    if (!IS_LIGHT (ch->room)
            && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !is_goggled(ch)
            && !IS_SET (ch->affected_by, AFF_INFRAVIS)
            && !(obj->obj_flags.type_flag == ITEM_LIGHT && obj->o.light.on)
			&& !(obj->obj_flags.type_flag == ITEM_E_LIGHT && obj->o.elecs.status))
        return 0;


    // We're now allowing multiple hide affects on objects, so we need multiple reveals.

    if (get_obj_affect(obj, MAGIC_HIDDEN))
    {
        for (af = obj->xaffected; af; af = af->next)
        {
            if (af->type == MAGIC_HIDDEN)
            {
                if (af->a.hidden.coldload_id == ch->coldload_id)
                    return 1;
                else if (af->a.hidden.clan > 0)
                {
                    clan = (get_clanid(af->a.hidden.clan))->literal;
                    if (clan && is_clan_member_player(ch, clan))
                        return 1;
                }
            }
        }
        return 0;
    }

    if (IS_SET (obj->obj_flags.extra_flags, ITEM_INVISIBLE) &&
            !get_affect (ch, MAGIC_AFFECT_SEE_INVISIBLE))
        return 0;

    /*
    if (weather_info[ch->room->zone].state == HEAVY_SNOW
      && !IS_SET (ch->room->room_flags, INDOORS))
    return 0;
    */

    return 1;
}

void
m (void)
{
    mm ("m called");
}

void
mm (char *msg)
{
#ifdef MM_DESIRED
    char buf[MAX_STRING_LENGTH];
    extern int mem_allocated;
    extern int mem_freed;
    static int last_total = 0;

    sprintf (buf, "%8d tot; +(%7d) %8d al - %7d fr: %s\n",
             mem_allocated - mem_freed,
             mem_allocated - mem_freed - last_total,
             mem_allocated, mem_freed, msg);

    last_total = mem_allocated - mem_freed;

    fprintf (stderr, buf);
#endif
}

int
is_human (CHAR_DATA * ch)
{
    // Survivor, Human, Denizen, Mutation, Cybernetic, and Phoenixer.

    if (ch->race == 1 ||
        ch->race == 5 ||
        ch->race == 6 ||
        ch->race == 67 ||
        ch->race == 68 ||
        ch->race == 69)
        return 1;

    return 0;
}

int
is_same_zone (int zone1, int zone2)
{
    if (zone1 == zone2)
        return 1;

    if (zone1 == 10 && zone2 == 76)
        return 1;

    if (zone1 == 76 && zone2 == 10)
        return 1;

    return 0;
}

TEXT_DATA *
add_text (TEXT_DATA ** list, char *filename, char *document_name)
{
    TEXT_DATA *text;
    char *doc;

    doc = file_to_string (filename);

    text = (TEXT_DATA *) alloc (sizeof (TEXT_DATA), 37);

    if (list == NULL)
        text->next = NULL;
    else
        text->next = *list;

    *list = text;

    text->filename = str_dup (filename);
    text->name = str_dup (document_name);
    text->text = doc;

    return text;
}

void
write_help (char *filename, HELP_DATA * list)
{
    FILE *fp_help;

    if (!(fp_help = fopen (filename, "w")))
    {
        perror ("write_help");
        return;
    }

    while (list)
    {
        if (list->master_element)
        {
            /* list isn't a master */
            list = list->next;
            continue;
        }

        fprintf (fp_help, "%s\n%s", list->keywords, list->help_info);

        list = list->next;

        if (list)
            fprintf (fp_help, "#\n");
        else
            fprintf (fp_help, "#~\n");
    }

    fclose (fp_help);
}

void
deallocate_help (HELP_DATA ** list)
{
    HELP_DATA *element;
    HELP_DATA *l;

    l = *list;
    *list = NULL;

    while (l)
    {
        element = l;
        l = l->next;

        if (element->keyword)
            mem_free (element->keyword);

        if (element->keywords)
            mem_free (element->keywords);

        if (element->help_info)
            mem_free (element->help_info);

        mem_free (element);
    }
}

HELP_DATA *
load_help_file (FILE * fp)
{
    HELP_DATA *list = NULL;
    HELP_DATA *last_element = NULL;
    HELP_DATA *master_element;
    HELP_DATA *element;
    char *p;
    char topic[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    while (1)
    {

        if (!fgets (buf, MAX_STRING_LENGTH - 1, fp))
            return list;

        if (*buf && buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) - 1] = '\0';

        master_element = NULL;

        p = one_argument (buf, topic);

        while (*topic)
        {

            if (!master_element)
            {

                master_element = (HELP_DATA *) alloc (sizeof (HELP_DATA), 36);
                master_element->keywords = str_dup (buf);

                if (!list)
                    list = master_element;
                else
                    last_element->next = master_element;

                last_element = master_element;
            }

            element = (HELP_DATA *) alloc (sizeof (HELP_DATA), 36);

            element->master_element = master_element;
            element->help_info = NULL;
            element->keyword = str_dup (topic);

            last_element->next = element;
            last_element = element;

            p = one_argument (p, topic);
        }

        *b_buf = '\0';

        while (1)
        {
            if (!fgets (buf, MAX_STRING_LENGTH - 1, fp) ||
                    !strcmp (buf, "#\n") || !strncmp (buf, "#~", 2))
                break;

            strcat (b_buf, buf);
        }

        master_element->help_info = str_dup (b_buf);

        if (buf[1] == '~')
            break;
    }

    return list;
}

void
insert_help (HELP_DATA ** sort, HELP_DATA * i, HELP_DATA * element)
{
    HELP_DATA *t;

    if (*sort == i)
    {
        element->next = *sort;
        *sort = element;
        return;
    }

    for (t = *sort; t->next; t = t->next)
    {
        if (t->next == i)
        {
            element->next = i;
            t->next = element;
            return;
        }
    }
}

void
sort_help (HELP_DATA ** list)
{
    HELP_DATA *help;
    HELP_DATA *sort;
    HELP_DATA *last_sort;
    HELP_DATA *element;
    HELP_DATA *tmp_sort;

    help = *list;
    sort = help;
    help = help->next;
    sort->next = NULL;
    last_sort = sort;

    while (help)
    {

        element = help;
        help = help->next;
        element->next = NULL;

        if (!element->master_element)
        {
            last_sort->next = element;
            last_sort = element;
            continue;
        }

        tmp_sort = sort;

        while (1)
        {

            if (!tmp_sort)
            {
                last_sort->next = element;
                last_sort = element;
                break;
            }

            if (tmp_sort->master_element &&
                    str_cmp (tmp_sort->keyword, element->keyword) > 0)
            {
                insert_help (&sort, tmp_sort, element);
                break;
            }

            tmp_sort = tmp_sort->next;
        }
    }

    *list = sort;
}

void
load_help (void)
{
    FILE *fp_help;

    deallocate_help (&help_list);

    if (!(fp_help = fopen (HELP_FILE, "r")))
    {
        help_list = NULL;
        return;
    }

    help_list = load_help_file (fp_help);
    fclose (fp_help);

    sort_help (&help_list);
}

void
load_bhelp (void)
{
    FILE *fp_bhelp;

    deallocate_help (&bhelp_list);

    if (!(fp_bhelp = fopen (BHELP_FILE, "r")))
    {
        bhelp_list = NULL;
    }

    bhelp_list = load_help_file (fp_bhelp);
    fclose (fp_bhelp);

    sort_help (&bhelp_list);
}

int
get_next_coldload_id (int for_a_pc)
{
    CHAR_DATA *tch;
    static int coldloads_read = 0;
    int return_coldload_id;
    FILE *fp = NULL;

    if (!coldloads_read)
    {
        if (!(fp = fopen (COLDLOAD_IDS, "r")))
            system_log ("NEW COLDLOAD ID FILE BEING CREATED.", true);
        else
        {
            fscanf (fp, "%d %d %d\n",
                    &next_pc_coldload_id,
                    &next_mob_coldload_id, &next_obj_coldload_id);
            fclose (fp);
        }

        if (next_mob_coldload_id > 100000)
            next_mob_coldload_id = 0;
        if (next_obj_coldload_id > 100000)
            next_obj_coldload_id = 0;

        next_mob_coldload_id += 100;	/* On boot, inc 100 in case */
        /* the mud crashed last time, */
        /* so we don't double count. */
        next_obj_coldload_id += 100;

        coldloads_read = 1;
    }

    if (for_a_pc == 1)
    {
        while ((tch =
                    get_char_id ((return_coldload_id = ++next_pc_coldload_id))))
            ;
    }
    else if (for_a_pc == 2)
    {
        return_coldload_id = ++next_obj_coldload_id;
    }
    else
    {
        while ((tch =
                    get_char_id ((return_coldload_id = ++next_mob_coldload_id))))
            ;
    }

    if (for_a_pc == 2 || !for_a_pc)
        return return_coldload_id;

    if (!(fp = fopen (COLDLOAD_IDS ".new", "w")))
    {
        system_log ("COLDLOAD ID FILE COULD NOT BE CREATED!!!", true);
        perror ("coldload");
        return return_coldload_id;
    }

    fprintf (fp, "%d %d %d\n", next_pc_coldload_id, next_mob_coldload_id,
             next_obj_coldload_id);

    fclose (fp);

    system ("mv " COLDLOAD_IDS ".new " COLDLOAD_IDS);

    return return_coldload_id;
}


void
add_combat_log (CHAR_DATA * ch, char *msg)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    char buf_arr[5][MAX_STRING_LENGTH];

    int j;
    char *p;

    if (IS_NPC (ch) && !ch->shop && !IS_SET (ch->act, ACT_STAYPUT))
        return;

    if (!msg || !*msg)
        return;

    if (!ch->combat_log)
        ch->combat_log = str_dup ("\n");

    p = ch->combat_log;

    i = 0;
    while (i < 5 && get_line (&p, buf))
    {
        strcpy (buf_arr[i], buf);
        i++;
    }

    *buf = '\0';

    for (j = (i >= 4 ? 1 : 0); j < 5 && j <= i; j++)
    {
        strcat (buf, buf_arr[j]);
        strcat (buf, "\n");
    }

    if (mud_time_str)
    {
        strcat (buf, mud_time_str);
        strcat (buf, " :: ");
    }

    sprintf (buf + strlen (buf), "%s DIED: %s\n", ch->tname, msg);

    if (ch->combat_log)
        mem_free (ch->combat_log);

    ch->combat_log = str_dup (buf);
}

int
get_bite_value (OBJ_DATA * obj)
{
    int bite_num;
    int total_bites;
    int bite_value;

    /* This little routine calculates how much food value is in one
       bite.  The initial bites may yield more food than later bites.
       This depends on how even the ratio of bites to food value is.
     */

    bite_num = obj->o.food.bites;
    total_bites = vtoo (obj->nVirtual)->o.food.bites;

    if (total_bites <= 1)
        return obj->o.food.food_value;

    bite_value = obj->o.food.food_value / total_bites;

    if (bite_value * total_bites + bite_num <= obj->o.food.food_value)
        bite_value++;

    fflush (stdout);
    return bite_value;
}

int
is_name_in_list (char *name, char *list)
{
    char *argument;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (list, buf);

    while (*buf)
    {

        if (!str_cmp (name, buf))
            return 1;

        argument = one_argument (argument, buf);
    }

    return 0;
}

char *
vnum_to_liquid_name (int vnum)
{
    OBJ_DATA *obj;
    static char buf[MAX_STRING_LENGTH];

    if (!(obj = vtoo (vnum)))
        return "an unknown fuel.";

    *buf = '\0';

    one_argument (obj->name, buf);

    if (buf[strlen (buf) - 1] == ',')
        buf[strlen (buf) - 1] = '\0';

    return buf;
}

int
obj_mass (OBJ_DATA * obj)
{
    int mass = 0;
    OBJ_DATA * liquid;

    mass = obj->obj_flags.weight;

    if (obj->count)
        mass *= obj->count;

    if ( GET_ITEM_TYPE( obj ) == ITEM_DRINKCON || GET_ITEM_TYPE( obj ) == ITEM_FOUNTAIN )
    {
        if ( obj->o.drinks.volume > 0 ) {
            liquid = vtoo( obj->o.drinks.liquid );

            if ( liquid )
                mass += obj->o.drinks.volume * liquid->obj_flags.weight;
        }
    } else
        mass += obj->contained_wt;

    if (mass == 0)
        mass = 1;

    return mass;
}

int
carrying (CHAR_DATA * ch)
{
    int mass = 0;
    OBJ_DATA *obj;

    if (ch->right_hand)
        mass += obj_mass (ch->right_hand);

    if (ch->left_hand)
        mass += obj_mass (ch->left_hand);

    for (obj = ch->equip; obj; obj = obj->next_content)
        mass += obj_mass (obj);

    return mass;
}

/*
    Writing utilities
*/

int get_page_oval( OBJ_DATA * obj )
{
    switch ( GET_ITEM_TYPE( obj ) )
    {
        case ITEM_PARCHMENT:
        case ITEM_BOOK:
            return 0;
        case ITEM_E_BOOK:
            return 3;

    }

    return -1;
}

int get_next_write_oval( OBJ_DATA * obj )
{
    switch ( GET_ITEM_TYPE( obj ) )
    {
        case ITEM_PARCHMENT:
            return 0;
        case ITEM_BOOK:
            return 1;
        case ITEM_E_BOOK:
            return 4;
    }

    return -1;
}

bool is_book( OBJ_DATA * obj )
{
    if (
        GET_ITEM_TYPE( obj ) == ITEM_BOOK           ||
        GET_ITEM_TYPE( obj ) == ITEM_E_BOOK
    ) return true;

    return false;
}

bool is_tearable( OBJ_DATA * obj )
{
    if (
        GET_ITEM_TYPE( obj ) == ITEM_BOOK           ||
        GET_ITEM_TYPE( obj ) == ITEM_PARCHMENT
    ) return true;

    return false;
}

bool uses_book_code( OBJ_DATA * obj )
{

    if (
        GET_ITEM_TYPE( obj ) == ITEM_BOOK           ||
        GET_ITEM_TYPE( obj ) == ITEM_E_BOOK         ||
        GET_ITEM_TYPE( obj ) == ITEM_PARCHMENT
    ) return true;

    return false;
}

/*------------------------------------------------------------------------\
|  isvowel(char)                                                          |
|                                                                         |
|  Returns true if the character is a vowel. Useful if you need to know   |
|  if an indefinite article should be 'a' or 'an'.                        |
\------------------------------------------------------------------------*/
bool
isvowel (char c)
{
    switch (c)
    {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
        return true;
    default:
        return false;
    }
}


/*---------------------------------------------------------------------\
 | swap_xmote_target (CHAR_DATA * ch, char *argument int *is_imote)    |
 |                                                                      |
 | swaps *target @ or ~target with short_desc                           |
 | cmd = 1 (emote call) 2 (pmote call)                                  |
 \---------------------------------------------------------------------*/

char *
swap_xmote_target (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char key[MAX_STRING_LENGTH] = { '\0' };
    char copy[MAX_STRING_LENGTH] = { '\0' };
    char speech [MAX_STRING_LENGTH] = { '\0'}; //Anything between two quotes gets stored in here.
    bool tochar = false;
    OBJ_DATA *obj = NULL;
    int key_e = 0;
    char *p = '\0';
    char *temp = NULL;
    bool is_imote = false;

    p = copy;
    temp = argument;

    while (*argument)
    {
        if (cmd==2 && IS_NPC(ch))
            return NULL;

        if (*argument == '@' )
        {
            if (cmd == 2 ) // don't allow @ to be used in pmote
            {
                send_to_char ("You may not refer to yourself in a pmote.", ch);
                return NULL;
            }
            is_imote = true;
            sprintf (p, "#5%s#0", char_short (ch));
            p += strlen (p);
            argument++;
        }

        if (*argument == '*')
        {

            argument++;
            while (isdigit (*argument))
            {
                key[key_e++] = *(argument++);
            }
            if (*argument == '.')
            {
                key[key_e++] = *(argument++);
            }
            while (isalpha (*argument) || *argument == '-')
            {
                key[key_e++] = *(argument++);
            }
            key[key_e] = '\0';
            key_e = 0;

            if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
                    !get_obj_in_list_vis (ch, key, ch->right_hand) &&
                    !get_obj_in_list_vis (ch, key, ch->left_hand) &&
                    !get_obj_in_list_vis (ch, key, ch->equip))
            {
                sprintf (buf, "I don't see %s here.\n", key);
                send_to_char (buf, ch);
                return NULL;
            }
            obj = get_obj_in_list_vis (ch, key, ch->right_hand);

            if (!obj)
                obj = get_obj_in_list_vis (ch, key, ch->left_hand);
            if (!obj)
                obj = get_obj_in_list_vis (ch, key, ch->room->contents);
            if (!obj)
                obj = get_obj_in_list_vis (ch, key, ch->equip);
            sprintf (p, "#2%s#0", obj_short_desc (obj));
            p += strlen (p);

        }
        else if (*argument == '~')
        {

            argument++;
            while (isdigit (*argument))
            {
                key[key_e++] = *(argument++);
            }
            if (*argument == '.')
            {
                key[key_e++] = *(argument++);
            }
            while (isalpha (*argument) || *argument == '-')
            {
                key[key_e++] = *(argument++);
            }
            key[key_e] = '\0';
            key_e = 0;

            if (!get_char_room_vis (ch, key))
            {
                sprintf (buf, "Who is %s?\n", key);
                send_to_char (buf, ch);
                return NULL;
            }
            if (get_char_room_vis (ch, key) == ch)
            {
                send_to_char
                ("You shouldn't refer to yourself using the token system.\n",
                 ch);
                return NULL;
            }
            sprintf (p, "#5%s#0", char_short (get_char_room_vis (ch, key)));
            p += strlen (p);
            tochar = true;
        }

        //Speech embedded in an emote
        else if (*argument=='\"') //if this is true, we have found the first quotation mark
        {
            if (cmd==2) //don't allow this in a pmote
            {
                send_to_char("You cannot use speech in a pmote.\n", ch);
                return NULL;
            }

            key_e=0;                   //reset index just in case it's not 0
            speech [key_e]=*argument;  //store first character (quote) before moving into the while loop
            *(argument++);             //move to the next character so !(*argument=='\"') evaluates as true.
            key_e++;

            while (!(*argument=='\"')) //move through the speech string until you find more quotation marks
            {
                speech[key_e++] = *(argument++);
            }
            if ((*argument='\"'))
                speech[key_e++] = *(argument++); //we also want those quotation marks included in the emote
            speech[key_e] = '\0';
            key_e=0;

            sprintf (p, "#6%s#0", speech); //colored teal (that's the point)
            p+=strlen (p);
        }
        else
            *(p++) = *(argument++);
    }

    *p = '\0';
    if (cmd == 1)
    {
        if (copy[0] == '\'')
        {
            if (!is_imote)
            {
                sprintf (buf, "#5%s#0%s", char_short (ch), copy);
                buf[2] = toupper (buf[2]);
            }
            else
            {
                sprintf (buf, "%s", copy);
                if (buf[0] == '#')
                {
                    buf[2] = toupper (buf[2]);
                }
                else
                {
                    buf[0] = toupper (buf[0]);
                }
            }
        }
        else
        {
            if (!is_imote)
            {
                sprintf (buf, "#5%s#0 %s", char_short (ch), copy);
                buf[2] = toupper (buf[2]);
            }
            else
            {
                sprintf (buf, "%s", copy);
                if (buf[0] == '#')
                {
                    buf[2] = toupper (buf[2]);
                }
                else
                {
                    buf[0] = toupper (buf[0]);
                }
            }
        }
    }
    else
    {
        sprintf (buf, "#0%s", copy);// need to add #0 here to reset colors depending on call
    }

    if (buf[strlen (buf) - 1] != '.' && buf[strlen (buf) - 1] != '!' && buf[strlen (buf) - 1] != '?' && buf[strlen (buf) - 3] != '\"')
        strcat (buf, ".");

    //argument = temp;
    sprintf (argument, "%s", buf);

    return (argument);
}

double RoundDouble(double doValue, int nPrecision)
{
    static const double doBase = 10.0;
    double doComplete5, doComplete5i;

    doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));
    if (doValue < 0.0)
        doComplete5 -= 5.0;
    else
        doComplete5 += 5.0;

    doComplete5 /= doBase;

    modf(doComplete5, &doComplete5i);

    return doComplete5i / pow(doBase, (double) nPrecision);
}
