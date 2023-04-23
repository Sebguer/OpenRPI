/*------------------------------------------------------------------------\
|  act.informative.c : Informational Module           www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"

#include "clan.h"
#include "group.h"
#include "utility.h"

extern rpie::server engine;
extern const char *skills[];
BOARD_DATA *full_board_list = NULL;

int snowstorm = 0;

#define LAST_STABLE_TICKET 1000


#define skill_lev(val) val >= 90 ? " Legend " : val >= 80 ? " Heroic " : val >= 70 ? " Master " : val >= 60 ? " Adroit " : val >= 50 ? "Talented" : val >= 40 ? "Familiar" : val >= 30 ? "Amateur " : val >= 20 ? " Novice " : "Beginner"

#define skill_lev2(val) val >= 90 ? " (Legend) " : val >= 80 ? " (Heroic) " : val >= 70 ? " (Master) " : val >= 60 ? " (Adroit) " : val >= 50 ? " (Talented) " : val >= 40 ? " (Familiar) " : val >= 30 ? "(Amateur) " : val >= 20 ? " (Novice) " : " (Beginner) "

#define skill_lev3(val) val >= 90 ? " [Legend] " : val >= 80 ? " [Heroic] " : val >= 70 ? " [Master] " : val >= 60 ? " [Adroit] " : val >= 50 ? " [Talented] " : val >= 40 ? " [Familiar] " : val >= 30 ? "[Amateur] " : val >= 20 ? " [Novice] " : " [Beginner] "

#define progress_lev(val) val >= 97 ? " ^^^ " : val >= 94 ? " ^^ " : val >= 91 ? " ^ " : val == 90 ? " " :  val >= 87 ? " ^^^ " : val >= 84 ? " ^^ " : val >= 81 ? " ^ " : val == 80 ? " " : val >= 77 ? " ^^^ " : val >= 74 ? " ^^ " : val >= 71 ? " ^ " : val >= 70 ?  " " : val >= 67 ? " ^^^ " : val >= 64 ? " ^^ " : val >= 61 ? " ^ " : val >= 60 ?  "  " : val >= 57 ? " ^^^ " : val >= 54 ? " ^^ " : val >= 51 ? " ^ " : val >= 50 ?  "  " : val >= 47 ? " ^^^ " : val >= 44 ? " ^^ " : val >= 41 ? " ^ " : val >= 40 ?  "  " : val >= 37 ? " ^^^ " : val >= 34 ? " ^^ " : val >= 31 ? " ^ " : val >= 30 ? " ": val >= 40 ?  "  ": val >= 27 ? " ^^^ " : val >= 24 ? " ^^ " : val >= 21 ? " ^ " : val >= 20 ?  "  " : val >= 17 ? " ^^^ " : val >= 14 ? " ^^ " : val >= 11 ? " ^ " : val >= 10 ?  "  " : val >= 6 ? " ** " : val == 5 ? " * "  : " "

const char *dec_conditions[5] =
{
  "pristine",
  "fresh",
  "worn",
  "damaged",
  "barely visible"
};

const char *dec_skills[9] =
{
  "a lack of any real",  // 0~9
  "child-like",          // 9~19
  "rudimentary",         // 19~29
  "mediocre",            // 30~39
  "talented",            // 40~49
  "professional",        // 50~59
  "true artisan",        // 60~69
  "masterful",           // 70~74
  "flawlessly executed", // 75+
};

const char *dec_short_skills[9] =
{
  "poorly",  // 0~9
  "poorly",          // 9~19
  "simply",         // 19~29
  "simply",            // 30~39
  "well",            // 40~49
  "well",        // 50~59
  "fantastically",        // 60~69
  "fantastically",           // 70~74
  "beautifully", // 75+
};

const char *dec_size[4] =
{
    "small",
    "large",
    "tiny",
    "all-encompassing"
};

const char *dec_short[11] =
{
    "(none)",
    "engraved",
    "embossed",
    "embroidered",
    "etched",
    "painted",
    "decorated",
    "enameled",
    "inked",
    "stenciled",
	"sculpted"
};


const char *month_name[12] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

const char *season_name[4] =
{
    "Spring",
    "Summer",
    "Autumn",
    "Winter"
};

const char *day_name[6] =
{
    "Day of the Stars",
    "Day of the Sun",
    "Day of the Moon",
    "Day of the Two Trees",
    "Day of the Heavens",
    "Day of the Valar"
};

int loc_order[MAX_WEAR] =
{
    WEAR_LIGHT,
    WEAR_SHIELD,
    WEAR_PRIM,
    WEAR_SEC,
    WEAR_BOTH,
    WEAR_CARRY_R,
    WEAR_CARRY_L,

    WEAR_HEAD,
    WEAR_HAIR,
    WEAR_EAR,
    WEAR_EYES,
    WEAR_BLINDFOLD,
    WEAR_FACE,
    WEAR_THROAT,
    WEAR_NECK_1,
    WEAR_NECK_2,

    WEAR_OVER,
    WEAR_ABOUT,
    WEAR_BACK,
    WEAR_SHOULDER_R,
    WEAR_SHOULDER_L,
    WEAR_BODY,
    WEAR_OVERWEAR,
    WEAR_ARMBAND_R,
    WEAR_ARMBAND_L,
    WEAR_ARMS,
    WEAR_WRIST_R,
    WEAR_WRIST_L,
    WEAR_HANDS,
    WEAR_FINGER_R,
    WEAR_FINGER_L,

    WEAR_WAIST,
    WEAR_BELT_1,
    WEAR_BELT_2,

    WEAR_LEGS,
	WEAR_UNDERWEAR,
    WEAR_ANKLE_R,
    WEAR_ANKLE_L,
    WEAR_FEET,

	WEAR_BLANK
};

/*
static char *strTimeWord[] =
{
    "twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
    "nine", "ten", "eleven",
    "twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
    "nine", "ten", "eleven",
    "twelve"
};
*/

const char *fog_states[] =
{
    "no fog",
    "thin fog",
    "thick fog",
    "\n"
};

const char *weather_states[] =
{
    "no rain",
    "chance of rain",
    "light rain",
    "steady rain",
    "heavy rain",
    "light snow",
    "steady snow",
    "heavy snow",
    "\n"
};

const char *weather_clouds[] =
{
    "clear sky",
    "light clouds",
    "heavy clouds",
    "overcast",
    "\n"
};


const char *wind_speeds[] =
{
    "calm",
    "breeze",
    "windy",
    "gale",
    "stormy",
    "\n"
};


const char *holiday_names[] =
{
    "(null)",
    "Mettare, the Last Day of the Year",
    "Yestare, the First Day of the New Year",
    "the First Day of Enderi, the Celebration of Winter",
    "the Middle Day of Enderi, the Celebration of Winter",
    "the Final Day of Enderi, the Celebration of Winter",
    "the feastday of yaviere",
    "\n"
};

const char *holiday_short_names[] =
{
    "(null)",
    "Mettare",
    "Yestare",
    "Enderi",
    "Enderi",
    "Enderi",
    "yaviere",
    "\n"
};

const char *trap_desc_bits[] =
{
    "can be set on exits",
    "can be set on objects",
    "can be manually released",
    "causes damage",
    "trips its victims",
    "binds its victims",
    "causes a somatic efect",
    "breaks on use",
    "breaks on reset",
    "must be used as the trigger",
    "is able to be avoided if used as a trigger",
    "will only target one person",
    "will always strike even if discovered",
    "cannot be found by searching or detecting",
    "cannot be detected",
    "DO NOT USE",
    "\n"
};

void
do_credits (CHAR_DATA * ch, char *argument, int cmd)
{
    do_help (ch, "diku_license", 0);
    return;
}

void
target_sighted (CHAR_DATA * ch, CHAR_DATA * target)
{
    SIGHTED_DATA *sighted = NULL;

    if (!ch || !target)
        return;

    if (!ch->sighted)
    {
        CREATE (ch->sighted, SIGHTED_DATA, 1);
        ch->sighted->target = target;
        return;
    }
    for (sighted = ch->sighted; sighted; sighted = sighted->next)
    {
        if (sighted->target == target)
            return;
        if (!sighted->next)
        {
            CREATE (sighted->next, SIGHTED_DATA, 1);
            sighted->next->target = target;
            return;
        }
    }
}


void
do_point (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char arg1[MAX_STRING_LENGTH] = { '\0' };
    char arg2[MAX_STRING_LENGTH] = { '\0' };
    char distance[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    CHAR_DATA *target = NULL, *tch = NULL;
    ROOM_DIRECTION_DATA *exit = NULL;
    ROOM_DATA *room = NULL;
    int dir = 0, range = 1;

    if (!*argument)
    {
        send_to_char ("Usage: point <direction> <target>\n", ch);
        return;
    }

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (!strn_cmp ("north", arg1, strlen (arg1)))
        dir = 0;
    else if (!strn_cmp ("east", arg1, strlen (arg1)))
        dir = 1;
    else if (!strn_cmp ("south", arg1, strlen (arg1)))
        dir = 2;
    else if (!strn_cmp ("west", arg1, strlen (arg1)))
        dir = 3;
    else if (!strn_cmp ("up", arg1, strlen (arg1)))
        dir = 4;
    else if (!strn_cmp ("down", arg1, strlen (arg1)))
        dir = 5;
    else
    {
        send_to_char ("Usage: point <direction> <target>\n", ch);
        return;
    }

    if (!EXIT (ch, dir))
    {
        send_to_char ("There isn't an exit in that direction.\n", ch);
        return;
    }

    room = vnum_to_room (EXIT (ch, dir)->to_room);
    exit = EXIT (ch, dir);

    if (exit
            && IS_SET (exit->exit_info, EX_ISDOOR)
            && IS_SET (exit->exit_info, EX_CLOSED)
            && !IS_SET (exit->exit_info, EX_ISGATE))
    {
        send_to_char ("Your view is blocked.\n", ch);
        return;
    }

    if (!(target = get_char_room_vis2 (ch, room->vnum, arg2))
            || !has_been_sighted (ch, target))
    {
        exit = room->dir_option[dir];
        if (!exit)
        {
            send_to_char ("You don't see them within range.\n", ch);
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
        if (room->dir_option[dir])
            room = vnum_to_room (room->dir_option[dir]->to_room);
        else
            room = NULL;
        if (is_sunlight_restricted (ch))
            return;
        if (!(target = get_char_room_vis2 (ch, room->vnum, arg2))
                || !has_been_sighted (ch, target))
        {
            exit = room->dir_option[dir];
            if (!exit)
            {
                send_to_char ("You don't see them within range.\n", ch);
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
            if (room->dir_option[dir])
                room = vnum_to_room (room->dir_option[dir]->to_room);
            else
                room = NULL;
            if (!(target = get_char_room_vis2 (ch, room->vnum, arg2))
                    || !has_been_sighted (ch, target))
            {
                exit = room->dir_option[dir];
                if (!exit)
                {
                    send_to_char ("You don't see them within range.\n", ch);
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
                send_to_char ("You don't see them within range.\n", ch);
                return;
            }
            else
                range = 3;
        }
        else
            range = 2;
    }
    else
        range = 1;

    if (!target || !CAN_SEE (ch, target) || !has_been_sighted (ch, target))
    {
        send_to_char ("You don't see them within range.\n", ch);
        return;
    }

    if (range == 2)
        sprintf (distance, "far ");
    else if (range == 3)
        sprintf (distance, "very far ");
    else
        *distance = '\0';

    sprintf (distance + strlen (distance), "to the %s", dirs[dir]);

    sprintf (buf, "You point at #5%s#0, %s.", char_short (target), distance);
    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

    sprintf (buf, "%s#0 points at #5%s#0, %s.", char_short (ch),
             char_short (target), distance);
    *buf = toupper (*buf);

    sprintf (buffer, "#5%s", buf);
    sprintf (buf, "%s", buffer);
    act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (!CAN_SEE (tch, ch))
            continue;
        target_sighted (tch, target);
    }
}

void
do_title (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char echo[MAX_STRING_LENGTH] = { '\0' };
    OBJ_DATA *obj = NULL;
    double skill = 0;

    if (!*argument)
    {
        send_to_char ("What did you wish to title?\n", ch);
        return;
    }

    if (!strstr (argument, "\"") && !strstr (argument, "\'"))
    {
        send_to_char
        ("You must enclose the book's desired title in quotation marks.\n",
         ch);
        return;
    }

    if (!ch->writes)
    {
        send_to_char
        ("In which script would you like to write? (See the SCRIBE command.)\n",
         ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
    {
        send_to_char ("You need to be holding the book you wish to title.\n",
                      ch);
        return;
    }

    if (obj->book_title && IS_MORTAL (ch))
    {
        send_to_char ("This work has already been titled.\n", ch);
        return;
    }

    if (GET_ITEM_TYPE (obj) != ITEM_BOOK && GET_ITEM_TYPE (obj) != ITEM_E_BOOK)  //Just allows e_books to be titled, too.
    {
        send_to_char ("This command only works with books.\n", ch);
        return;
    }

    skill =
        (ch->skills[ch->writes] * 0.50) + (ch->skills[ch->speaks] * 0.30) +
        (ch->skills[SKILL_EDUCATION] * 0.20);
    skill = (int) skill;
    skill = MIN (95, (int) skill);

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("What did you wish to title this work?\n", ch);
        return;
    }

    if (strlen (buf) > 55)
    {
        send_to_char ("There is a 55-character limit on book titles.\n", ch);
        return;
    }

    obj->book_title = add_hash (buf);
    obj->title_skill = (int) skill;
    obj->title_script = ch->writes;
    obj->title_language = ch->speaks;

    sprintf (echo, "You have entitled #2%s#0 '%s'.", obj->short_description,
             buf);
    act (echo, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
}

char *
parse_room_description (CHAR_DATA * ch, char *desc)
{
    return desc;
}

void
do_timeconvert (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    int hour = 0, day = 0, month = 0, year = 0;
    time_t temp_time = 0;
    struct tm real_date;
    struct time_info_data game_date;
    char suf[5] = { '\0' };

    if (!ch->descr() || !ch->descr()->acct)
    {
        send_to_char ("Only PCs can use this command.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);
    hour = atoi (buf);

    if (hour > 23 || hour < 0)
    {
        send_to_char ("You must specify an hour between 0 and 23.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);
    day = atoi (buf);

    if (day > 31 || day < 1)
    {
        send_to_char ("You must specify a day between 1 and 31.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);
    month = atoi (buf);

    if (month > 12 || month < 1)
    {
        send_to_char ("You must specify a month between 1 and 12.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);
    year = atoi (buf);

    if (year < 2600 && year > 2460)
    {

        if (day > 30 || day < 1)
        {
            send_to_char ("You must specify a day between 1 and 30.\n", ch);
            return;
        }

#ifndef MACOSX
        temp_time =
            GAME_SECONDS_BEGINNING + timezone +
            (int)(ch->descr()->acct->timezone * 60.0 * 60.0);
        temp_time +=
            ((((year - GAME_BASE_YEAR) * GAME_SECONDS_PER_YEAR) +
              ((month - 1) * GAME_SECONDS_PER_MONTH) +
              ((day - 1) * GAME_SECONDS_PER_DAY) +
              (hour * GAME_SECONDS_PER_HOUR)) / PULSES_PER_SEC);
#else
        (int) temp_time =
            GAME_SECONDS_BEGINNING +
            (int) (ch->descr()->acct->timezone * 60.0 * 60.0);
        (int) temp_time +=
            ((((year - GAME_BASE_YEAR) * GAME_SECONDS_PER_YEAR) +
              ((month - 1) * GAME_SECONDS_PER_MONTH) +
              ((day - 1) * GAME_SECONDS_PER_DAY) +
              (hour * GAME_SECONDS_PER_HOUR)) / PULSES_PER_SEC);
#endif

        strftime (buf, 255,
                  "In your timezone, the specified in-game time will fall at or near %I:%M %P, %A %B %e %Y.",
                  localtime (&temp_time));
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

    }
    else if (year < 2010 && year > 2002)
    {

        real_date.tm_hour = hour;
        real_date.tm_mday = day;
        real_date.tm_mon = month - 1;
        real_date.tm_year = year - 1900;
        real_date.tm_sec = 0;
        real_date.tm_min = 0;

#ifndef MACOSX
        temp_time =
            ((mktime (&real_date))
             - ((int)(ch->descr()->acct->timezone * 60.0 * 60.0)))
            - timezone;
#else
        (int) temp_time =
            (((int) mktime (&real_date))
             - ((int) (ch->descr()->acct->timezone * 60.0 * 60.0)))
            - (int) timezone;
#endif
        game_date = moon_time_passed (temp_time, GAME_SECONDS_BEGINNING);

        day = game_date.day + 1;

        if (day == 1)
            strcpy (suf, "st");
        else if (day == 2)
            strcpy (suf, "nd");
        else if (day == 3)
            strcpy (suf, "rd");
        else if (day < 20)
            strcpy (suf, "th");
        else if ((day % 10) == 1)
            strcpy (suf, "st");
        else if ((day % 10) == 2)
            strcpy (suf, "nd");
        else if ((day % 10) == 3)
            strcpy (suf, "rd");
        else
            strcpy (suf, "th");

        sprintf (buf,
                 "In your timezone, the specified time will fall at or near %d:00 %s on the %d%s day of the %s in the year %d by Noldor Reckoning.",
                 (game_date.hour ==
                  0) ? 12 : ((game_date.hour >
                              12) ? game_date.hour - 11 : game_date.hour),
                 (game_date.hour <= 12) ? "am" : "pm", game_date.day + 1, suf,
                 month_name[(int) game_date.month], game_date.year);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

        return;

    }
    else
    {
        send_to_char ("You must specify a year between 2460 and 2600.\n", ch);
    }

}

char *frame_built[] =
{
    "fragily-built",
    "scantly-built",
    "lightly-built",
    "typically-built",
    "heavily-built",
    "massively-built",
    "gigantically-built",
    "\n"
};


char *verbal_intox[] =
    { "sober", "tipsy", "slightly drunk", "drunk", "intoxicated",
      "plastered"
    };

char *verbal_hunger[] =
    { "almost starved to death", "feeling faint from starvation", "slowly starving", "ravenous", "famished", "hungry", "feeling peckish", "full",
      "satiated", "absolutely stuffed"
    };

char *verbal_thirst[] =
    { "dying of thirst", "desperately thirsty", "quite parched", "feeling thirsty",
      "feeling slightly thirsty", "not thirsty", "sated", "nicely quenched", "completely sated"
    };

void post_message (DESCRIPTOR_DATA * d);

int
armor_total (CHAR_DATA * ch)
{
    OBJ_DATA *armor = NULL;
    int total = 0;

    for (armor = ch->equip; armor; armor = armor->next_content)
        if (GET_ITEM_TYPE (armor) == ITEM_ARMOR)
            total += armor->o.od.value[0];

    return total;
}

char *
tilde_eliminator (char *string)
{
    char *p = '\0';

    while ((p = strchr (string, '~')))
        *p = '-';

    return str_dup (string);
}

char *
carry_desc (OBJ_DATA * obj)
{
    if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
            obj->location == WEAR_BOTH)
        return "wielding";

    else if (obj->location == WEAR_SHIELD)
        return "gripping";

    else
        return "carrying";
}

char *
shock_bar (CHAR_DATA * ch, bool prompt)
{
    static char buf[25] = { '\0' };
    float calc = 0;
    float shock = 0;

	if (IS_SET(ch->plr_flags, BRIEF_MODE))
	{
		if (ch->shock >= ch->max_shock)
			sprintf (buf, "T: #26%s#0", prompt ? "" : "/6");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .6667)
			&& ch->shock < ch->max_shock)
			sprintf (buf, "T: #25%s#0", prompt ? "" : "/6");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .5)
			&& (shock = ch->shock) < (calc = ch->max_shock * .6667))
			sprintf (buf, "T: #34%s#0", prompt ? "" : "/6");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .2000)
			&& (shock = ch->shock) < (calc = ch->max_shock * .5))
			sprintf (buf, "T: #33%s#0", prompt ? "" : "/6");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .1000)
			&& (shock = ch->shock) < (calc = ch->max_shock * .2000))
			sprintf (buf, "T: #12%s#0", prompt ? "" : "/6");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .0001)
			&& (shock = ch->shock) < (calc = ch->max_shock * .1000))
			sprintf (buf, "T: #11%s#0", prompt ? "" : "/6");

		if (ch->shock == 0)
			sprintf (buf, "T: #10%s#0", prompt ? "" : "/6");
	}
	else
	{
		if (ch->shock >= ch->max_shock)
			sprintf (buf, "#1^^#3^^#2^^#0");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .6667)
			&& ch->shock < ch->max_shock)
			sprintf (buf, "#1^^#3^^#2^#0 ");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .5)
			&& (shock = ch->shock) < (calc = ch->max_shock * .6667))
			sprintf (buf, "#1^^#3^^#0  ");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .2000)
			&& (shock = ch->shock) < (calc = ch->max_shock * .5))
			sprintf (buf, "#1^^#3^#0   ");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .1000)
			&& (shock = ch->shock) < (calc = ch->max_shock * .2000))
			sprintf (buf, "#1^^#0    ");

		if ((shock = ch->shock) >= (calc = ch->max_shock * .0001)
			&& (shock = ch->shock) < (calc = ch->max_shock * .1000))
			sprintf (buf, "#1^#0     ");

		if (ch->shock == 0)
			sprintf (buf, "       ");
	}



    return buf;
}

char *
fatigue_bar (CHAR_DATA * ch, bool prompt)
{
    static char buf[25] = { '\0' };
    float calc = 0;
    float move = 0;

	if (IS_SET(ch->plr_flags, BRIEF_MODE))
	{
		if (ch->move >= ch->max_move)
			sprintf (buf, "S: #26%s#0", prompt ? "" : "/6");

		if ((move = ch->move) >= (calc = ch->max_move * .6667)
			&& ch->move < ch->max_move)
			sprintf (buf, "S: #25%s#0", prompt ? "" : "/6");

		if ((move = ch->move) >= (calc = ch->max_move * .5)
			&& (move = ch->move) < (calc = ch->max_move * .6667))
			sprintf (buf, "S: #14%s#0", prompt ? "" : "/6");

		if ((move = ch->move) >= (calc = ch->max_move * .3333)
			&& (move = ch->move) < (calc = ch->max_move * .5))
			sprintf (buf, "S: #13%s#0", prompt ? "" : "/6");

		if ((move = ch->move) >= (calc = ch->max_move * .1667)
			&& (move = ch->move) < (calc = ch->max_move * .3333))
			sprintf (buf, "S: #12%s#0", prompt ? "" : "/6");

		if ((move = ch->move) >= (calc = ch->max_move * .0001)
			&& (move = ch->move) < (calc = ch->max_move * .1667))
			sprintf (buf, "S: #11%s#0", prompt ? "" : "/6");

		if (ch->move == 0)
			sprintf (buf, "S: #10%s#0", prompt ? "" : "/6");
	}
	else
	{
		if (ch->move >= ch->max_move)
			sprintf (buf, "#1||#3||#2||#0");

		if ((move = ch->move) >= (calc = ch->max_move * .6667)
			&& ch->move < ch->max_move)
			sprintf (buf, "#1||#3||#2|#0 ");

		if ((move = ch->move) >= (calc = ch->max_move * .5)
			&& (move = ch->move) < (calc = ch->max_move * .6667))
			sprintf (buf, "#1||#3||#0  ");

		if ((move = ch->move) >= (calc = ch->max_move * .3333)
			&& (move = ch->move) < (calc = ch->max_move * .5))
			sprintf (buf, "#1||#3|#0   ");

		if ((move = ch->move) >= (calc = ch->max_move * .1667)
			&& (move = ch->move) < (calc = ch->max_move * .3333))
			sprintf (buf, "#1||#0    ");

		if ((move = ch->move) >= (calc = ch->max_move * .0001)
			&& (move = ch->move) < (calc = ch->max_move * .1667))
			sprintf (buf, "#1|#0     ");

		if (ch->move == 0)
			sprintf (buf, "       ");
	}


    return buf;
}

char *
choke_bar (CHAR_DATA * ch, bool prompt)
{
    AFFECTED_TYPE *af;
    static char buf[25] = { '\0' };
    int current = 0, max = 0;
    float calc = 0;
    float move = 0;

    if (!(af = get_affect (ch, AFFECT_CHOKING)))
        return NULL;

    current = af->a.spell.duration;
    max = af->a.spell.sn;

    if (IS_SET(ch->plr_flags, BRIEF_MODE))
    {
        if (current >= max)
            sprintf (buf, "B: #26%s#0", prompt ? "" : "/6");

        else if ((move = current) >= (calc = max * .6667) && current < max)
            sprintf (buf, "B: #25%s#0", prompt ? "" : "/6");

        else if ((move = current) >= (calc = max * .5)
            && (move = current) < (calc = max * .6667))
            sprintf (buf, "B: #34%s#0", prompt ? "" : "/6");

        else if ((move = current) >= (calc = max * .3333)
            && (move = current) < (calc = max * .5))
            sprintf (buf, "B: #33%s#0", prompt ? "" : "/6");

        else if ((move = current) >= (calc = max * .1667)
            && (move = current) < (calc = max * .3333))
            sprintf (buf, "B: #12%s#0", prompt ? "" : "/6");

        else if ((move = current) >= (calc = max * .0001)
            && (move = current) < (calc = max * .1667))
            sprintf (buf, "B: #11%s#0", prompt ? "" : "/6");

        else
            sprintf (buf, "B: #10%s#0", prompt ? "" : "/6");
    }
    else
    {
        if (current >= max)
            sprintf (buf, "#6******#0");

        else if ((move = current) >= (calc = max * .6667) && current < max)
            sprintf (buf, "#6*****#0 ");

        else if ((move = current) >= (calc = max * .5)
            && (move = current) < (calc = max * .6667))
            sprintf (buf, "#6****#0  ");

        else if ((move = current) >= (calc = max * .3333)
            && (move = current) < (calc = max * .5))
            sprintf (buf, "#6***#0   ");

        else if ((move = current) >= (calc = max * .1667)
            && (move = current) < (calc = max * .3333))
            sprintf (buf, "#6**#0    ");

        else if ((move = current) >= (calc = max * .0001)
            && (move = current) < (calc = max * .1667))
            sprintf (buf, "#6*#0     ");

        else
            sprintf (buf, "      ");
    }
    return buf;
}

char *
breath_bar (CHAR_DATA * ch, bool prompt)
{
    AFFECTED_TYPE *af;
    static char buf[25] = { '\0' };
    int current = 0, max = 0;
    float calc = 0;
    float move = 0;

    if (!(af = get_affect (ch, AFFECT_HOLDING_BREATH)))
        return NULL;

    current = af->a.spell.duration;
    max = af->a.spell.sn;

	/*if (IS_SET(ch->plr_flags, BRIEF_MODE))
	{
		if (current >= max)
			sprintf (buf, "B: #26%s#0", prompt ? "" : "/6");

		else if ((move = current) >= (calc = max * .6667) && current < max)
			sprintf (buf, "B: #25%s#0", prompt ? "" : "/6");

		else if ((move = current) >= (calc = max * .5)
			&& (move = current) < (calc = max * .6667))
			sprintf (buf, "B: #34%s#0", prompt ? "" : "/6");

		else if ((move = current) >= (calc = max * .3333)
			&& (move = current) < (calc = max * .5))
			sprintf (buf, "B: #33%s#0", prompt ? "" : "/6");

		else if ((move = current) >= (calc = max * .1667)
			&& (move = current) < (calc = max * .3333))
			sprintf (buf, "B: #12%s#0", prompt ? "" : "/6");

		else if ((move = current) >= (calc = max * .0001)
			&& (move = current) < (calc = max * .1667))
			sprintf (buf, "B: #11%s#0", prompt ? "" : "/6");

		else
			sprintf (buf, "B: #10%s#0", prompt ? "" : "/6");
	}
	else
	{*/
		if (current >= max)
			sprintf (buf, "#6******#0");

		else if ((move = current) >= ((calc = max * .6667) && current < max))
			sprintf (buf, "#6*****#0 ");

		else if ((move = current) >= (calc = max * .5)
			&& (move = current) < (calc = max * .6667))
			sprintf (buf, "#6****#0  ");

		else if ((move = current) >= (calc = max * .3333)
			&& (move = current) < (calc = max * .5))
			sprintf (buf, "#6***#0   ");

		else if ((move = current) >= (calc = max * .1667)
			&& (move = current) < (calc = max * .3333))
			sprintf (buf, "#6**#0    ");

		else if ((move = current) >= (calc = max * .0001)
			&& (move = current) < (calc = max * .1667))
			sprintf (buf, "#6*#0     ");

		else
			sprintf (buf, "      ");
	//}


    return buf;
}

/* Procedures related to 'look' */

void
argument_split_2 (char *argument, char *first_arg, char *second_arg)
{
    int look_at = 0, found = 0, begin = 0;
    found = begin = 0;

    if (!argument)
    {
        return;
    }

    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++);

    /* Find length of first word */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

        /* Make all letters lower case, AND copy them to first_arg */
        *(first_arg + look_at) = tolower (*(argument + begin + look_at));
    *(first_arg + look_at) = '\0';
    begin += look_at;

    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++);

    /* Find length of second word */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

        /* Make all letters lower case, AND copy them to second_arg */
        *(second_arg + look_at) = tolower (*(argument + begin + look_at));
    *(second_arg + look_at) = '\0';
    begin += look_at;
}

char *
find_ex_description (char *word, EXTRA_DESCR_DATA * list)
{
    EXTRA_DESCR_DATA *i = NULL;

    for (i = list; i; i = i->next)
        if (isname (word, i->keyword))
            return (i->description);

    return NULL;
}

/*
Old compare command is deprecated. It is no longer used on SOI
but will be left commented here in case it is wanted at a
later date - Valarauka

void
do_compare (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch = NULL, *tch2 = NULL;
  int ch_height_rating = 0, tch_height_rating = 0, tch2_height_rating = 0;
  char buf[250] = { '\0' }, arg1[MAX_STRING_LENGTH] =
  {
  '\0'}, target2[MAX_STRING_LENGTH] =
  {
  '\0'};

  *target2 = '\0';
  *arg1 = '\0';
  *buf = '\0';

  argument = one_argument (argument, arg1);

  ch_height_rating = 8;
  if (ch->height < 84)
    ch_height_rating = 7;
  if (ch->height < 77)
    ch_height_rating = 6;
  if (ch->height < 71)
    ch_height_rating = 5;
  if (ch->height < 66)
    ch_height_rating = 4;
  if (ch->height < 61)
    ch_height_rating = 3;
  if (ch->height < 54)
    ch_height_rating = 2;
  if (ch->height < 36)
    ch_height_rating = 1;

  if ((tch = get_char_room_vis (ch, arg1)))
    {
      tch_height_rating = 8;
      if (tch->height < 84)
	tch_height_rating = 7;
      if (tch->height < 77)
	tch_height_rating = 6;
      if (tch->height < 71)
	tch_height_rating = 5;
      if (tch->height < 66)
	tch_height_rating = 4;
      if (tch->height < 61)
	tch_height_rating = 3;
      if (tch->height < 54)
	tch_height_rating = 2;
      if (tch->height < 36)
	tch_height_rating = 1;
    }
  else
    {
      send_to_char ("Compare who?\n", ch);
      return;
    }

  one_argument (argument, target2);

  if (*target2)
    {
      if ((tch2 = get_char_room_vis (ch, target2)))
	{
	  tch2_height_rating = 8;
	  if (tch2->height < 84)
	    tch2_height_rating = 7;
	  if (tch2->height < 77)
	    tch2_height_rating = 6;
	  if (tch2->height < 71)
	    tch2_height_rating = 5;
	  if (tch2->height < 66)
	    tch2_height_rating = 4;
	  if (tch2->height < 61)
	    tch2_height_rating = 3;
	  if (tch2->height < 54)
	    tch2_height_rating = 2;
	  if (tch2->height < 36)
	    tch2_height_rating = 1;
	}
      else
	{
	  send_to_char ("I don't see the second target.\n", ch);
	  return;
	}
    }

  if (tch2 == ch)
    {
      send_to_char ("Compare yourself to yourself? Eh?\n", ch);
      return;
    }

  if (!tch2 && tch == ch)
    {
      send_to_char ("Compare yourself to yourself? Eh?\n", ch);
      return;
    }

  if (tch == tch2)
    {
      send_to_char ("Comparing something to itself? Brilliant!\n", ch);
      return;
    }

  if (!tch2)
    {
      if (tch_height_rating >= ch_height_rating)
	{
	  if (tch_height_rating - ch_height_rating == 7)
	    sprintf (buf,
		     "#5%s#0 is %s, and is nearly three times your height!",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 6)
	    sprintf (buf, "#5%s#0 is %s, and is nearly twice your height!",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 5)
	    sprintf (buf, "#5%s#0 is %s, and soars above you.",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 4)
	    sprintf (buf, "#5%s#0 is %s, and towers above you.",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 3)
	    sprintf (buf, "#5%s#0 is %s, and is quite a bit taller than you.",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 2)
	    sprintf (buf,
		     "#5%s#0 is %s, and is considerably taller than you.",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating == 1)
	    sprintf (buf, "#5%s#0 is %s, and is a bit taller than you.",
		     char_short (tch), frame_built[tch->frame]);
	  if (tch_height_rating - ch_height_rating <= 0)
	    {
	      if (tch->height > ch->height)
		sprintf (buf,
			 "#5%s#0 is %s, and is only slightly taller than you.",
			 char_short (tch), frame_built[tch->frame]);
	      else if (tch->height < ch->height)
		sprintf (buf, "#5%s#0 is %s, and slightly shorter than you.",
			 char_short (tch), frame_built[tch->frame]);
	      else
		sprintf (buf, "#5%s#0 is %s, and roughly the same height.",
			 char_short (tch), frame_built[tch->frame]);
	    }
	}
      else if (tch_height_rating < ch_height_rating)
	{
	  if (ch_height_rating - tch_height_rating == 7)
	    sprintf (buf,
		     "#5%s#0 is %s, and is about a third of your height.",
		     char_short (tch), frame_built[tch->frame]);
	  if (ch_height_rating - tch_height_rating == 6)
	    sprintf (buf, "#5%s#0 is %s, and is roughly half your height.",
		     char_short (tch), frame_built[tch->frame]);
	  if (ch_height_rating - tch_height_rating == 5)
	    sprintf (buf, "#5%s#0 is %s, and you soar over #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     !is_hooded (tch) ? HMHR (tch) : "it");
	  if (ch_height_rating - tch_height_rating == 4)
	    sprintf (buf, "#5%s#0 is %s, and you tower over #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     !is_hooded (tch) ? HMHR (tch) : "it");
	  if (ch_height_rating - tch_height_rating == 3)
	    sprintf (buf,
		     "#5%s#0 is %s, and you are quite a bit taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     !is_hooded (tch) ? HMHR (tch) : "it");
	  if (ch_height_rating - tch_height_rating == 2)
	    sprintf (buf,
		     "#5%s#0 is %s, and you are considerably taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     !is_hooded (tch) ? HMHR (tch) : "it");
	  if (ch_height_rating - tch_height_rating == 1)
	    sprintf (buf,
		     "#5%s#0 is %s, and you are a bit taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     !is_hooded (tch) ? HMHR (tch) : "it");
	}
    }

  else
    {
      if (tch_height_rating >= tch2_height_rating)
	{
	  if (tch_height_rating - tch2_height_rating == 7)
	    sprintf (buf,
		     "#5%s#0 is %s, and is nearly three times #5%s#0's height!",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 6)
	    sprintf (buf,
		     "#5%s#0 is %s, and is nearly twice #5%s#0's height!",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 5)
	    sprintf (buf, "#5%s#0 is %s, and soars above #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 4)
	    sprintf (buf, "#5%s#0 is %s, and towers above #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 3)
	    sprintf (buf,
		     "#5%s#0 is %s, and is quite a bit taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 2)
	    sprintf (buf,
		     "#5%s#0 is %s, and is considerably taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating == 1)
	    sprintf (buf, "#5%s#0 is %s, and is a bit taller than #5%s#0.\n",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch_height_rating - tch2_height_rating <= 0)
	    {
	      if (tch->height > tch2->height)
		sprintf (buf,
			 "#5%s#0 is %s, and is only slightly taller than #5%s#0.",
			 char_short (tch), frame_built[tch->frame],
			 char_short (tch2));
	      else if (tch->height < tch2->height)
		sprintf (buf,
			 "#5%s#0 is %s, and slightly shorter than #5%s#0.",
			 char_short (tch), frame_built[tch->frame],
			 char_short (tch2));
	      else
		sprintf (buf,
			 "#5%s#0 is %s, and roughly the same height as #5%s#0.",
			 char_short (tch), frame_built[tch->frame],
			 char_short (tch2));
	    }
	}
      if (tch_height_rating < tch2_height_rating)
	{
	  if (tch2_height_rating - tch_height_rating == 7)
	    sprintf (buf,
		     "#5%s#0 is %s, and is about a third of #5%s#0's height.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch2_height_rating - tch_height_rating == 6)
	    sprintf (buf,
		     "#5%s#0 is %s, and is roughly half #5%s#0's height.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2));
	  if (tch2_height_rating - tch_height_rating == 5)
	    sprintf (buf, "#5%s#0 is %s, and %s soars over #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
	  if (tch2_height_rating - tch_height_rating == 4)
	    sprintf (buf, "#5%s#0 is %s, and %s towers over #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
	  if (tch2_height_rating - tch_height_rating == 3)
	    sprintf (buf,
		     "#5%s#0 is %s, and %s is quite a bit taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
	  if (tch2_height_rating - tch_height_rating == 2)
	    sprintf (buf,
		     "#5%s#0 is %s, and %s is considerably taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
	  if (tch2_height_rating - tch_height_rating == 1)
	    sprintf (buf, "#5%s#0 is %s, and %s is a bit taller than #5%s#0.",
		     char_short (tch), frame_built[tch->frame],
		     char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
	}
    }

  act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

}*/

/* New version of the compare command to compare objects

This function takes two object references as arguments
and compares attributes such as weight and cost to give
an in character comparison string. - Valarauka

*/
void
do_compare(CHAR_DATA * ch, char *argument, int cmd)
{
    char		arg1 [MAX_STRING_LENGTH] = { '\0' };
    char		arg2 [MAX_STRING_LENGTH] = { '\0' };
    OBJ_DATA	*obj1 = NULL;
    OBJ_DATA	*obj2 = NULL;
    char		buffer [MAX_STRING_LENGTH] = { '\0' };

    /*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

    if ( GET_POS (ch) < POSITION_SLEEPING )
    {
        send_to_char ("You are unconscious!\n", ch);
        return;
    }

    if ( GET_POS (ch) == POSITION_SLEEPING )
    {
        send_to_char ("You are asleep.\n", ch);
        return;
    }

    if ( is_blind (ch) )
    {
        send_to_char ("You are blind!\n", ch);
        return;
    }

    /*** Make sure enough arguments are spcified***/

    argument = one_argument (argument, arg1);

    if ( !*arg1 )
    {
        send_to_char ("Compare what?\n", ch);
        return;
    }

    argument = one_argument (argument, arg2);

    if ( !*arg2 )
    {
        send_to_char ("Compare it with what?\n", ch);
        return;
    }


    /*** Find the objects being compared ***/

    if ( !(obj1 = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
            !(obj1 = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
            !(obj1 = get_obj_in_dark (ch, arg1, ch->equip)) &&
            !(obj1 = get_obj_in_dark (ch, arg1, ch->room->contents)))
    {

        send_to_char ("You don't see that.\n", ch);
        return;
    }

    if ( !(obj2 = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
            !(obj2 = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
            !(obj2 = get_obj_in_dark (ch, arg2, ch->equip)) &&
            !(obj2 = get_obj_in_dark (ch, arg2, ch->room->contents)))
    {

        send_to_char ("You don't see that.\n", ch);
        return;
    }

    /*** Compared objects must be of the same type ***/

    if (GET_ITEM_TYPE(obj1)!=GET_ITEM_TYPE(obj2))
    {
        send_to_char("You can only compare similar objects.",ch);
        return;
    }

    /*** Cannot compare something to itself ***/
    if (obj1==obj2)
    {
        send_to_char("Compare it with itself?",ch);
        return;
    }

    /*** Start Comparison Proper ***/

	sprintf (buffer, "\nCompared to #2%s#0, #2%s#0:",
             obj2->short_description,
             obj1->short_description);

    act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    *buffer = '\0';

    /*** Compare weights ***/
    if ((obj1->obj_flags.weight + obj1->contained_wt) >
            (obj2->obj_flags.weight + obj2->contained_wt))
    {
		sprintf (buffer, "   #6+#0 weighs #1more#0");
    }
    else if ((obj2->obj_flags.weight + obj2->contained_wt) >
             (obj1->obj_flags.weight + obj1->contained_wt))
    {
		sprintf (buffer, "   #6+#0 weighs #2less#0");
    }
    else
    {
		sprintf (buffer, "   #6+#0 weighs about the #3same#0");
    }

	if (GET_ITEM_TYPE(obj1) == ITEM_WEAPON &&
		GET_ITEM_TYPE(obj2) == ITEM_WEAPON)
	{
		double avg_dam_one = (((obj1->o.weapon.dice * obj1->o.weapon.sides) + obj1->o.od.value[5]) +
						      (obj1->o.weapon.dice + obj1->o.od.value[5])) / 2.0;
		double avg_dam_two = (((obj2->o.weapon.dice * obj2->o.weapon.sides) + obj2->o.od.value[5]) +
						      (obj2->o.weapon.dice + obj2->o.od.value[5])) / 2.0;

		if (avg_dam_one > avg_dam_two)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would on average cause #2more#0 damage");
		}
		else if (avg_dam_one < avg_dam_two)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would on average cause #1less#0 damage");
		}
		else
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would on average cause about the #3same#0 damage");
		}

		if (use_table[obj1->o.weapon.use_skill].delay <
			use_table[obj2->o.weapon.use_skill].delay)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 strikes #2faster#0");
		}
		else if (use_table[obj1->o.weapon.use_skill].delay >
				 use_table[obj2->o.weapon.use_skill].delay)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 strikes #1slower#0");
		}
		else
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 strikes at about the #3same#0 speed");
		}

		if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][0] >
			avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][0])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #2more#0 damage against padded armours");
		}
		else if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][0] <
			     avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][0])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #1less#0 damage against padded armours");
		}

		if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][1] >
			avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][1])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #2more#0 damage against hardened armours");
		}
		else if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][1] <
				 avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][1])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #1less#0 damage against hardened armours");
		}

		if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][2] >
			avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][2])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #2more#0 damage against mesh armours");
		}
		else if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][2] <
				 avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][2])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #1less#0 damage against mesh armours");
		}

		if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][3] >
			avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][3])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #2more#0 damage against composite armours");
		}
		else if (avg_dam_one + weapon_armor_table[obj1->o.weapon.hit_type][3] <
				 avg_dam_two + weapon_armor_table[obj2->o.weapon.hit_type][3])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 cause #1less#0 damage against composite armours");
		}
	}
	else if (GET_ITEM_TYPE(obj1) == ITEM_ARMOR &&
		     GET_ITEM_TYPE(obj2) == ITEM_ARMOR)
	{
		if (obj1->o.armor.armor_value > obj2->o.armor.armor_value)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 provides #2more#0 overall protection");
		}
		else if (obj1->o.armor.armor_value < obj2->o.armor.armor_value)
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 provides #1less#0 overall protection");
		}
		else
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 provides about the #3same#0 overall protection");
		}

		if (obj1->o.armor.armor_value - weapon_armor_table[1][obj1->o.armor.armor_type] >
			obj2->o.armor.armor_value - weapon_armor_table[1][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #2more#0 against piercing");
		}
		else if (obj1->o.armor.armor_value - weapon_armor_table[1][obj1->o.armor.armor_type] <
			     obj2->o.armor.armor_value - weapon_armor_table[1][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #1less#0 against piercing");
		}

		if (obj1->o.armor.armor_value - weapon_armor_table[3][obj1->o.armor.armor_type] >
			obj2->o.armor.armor_value - weapon_armor_table[3][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #2more#0 against crushing");
		}
		else if (obj1->o.armor.armor_value - weapon_armor_table[3][obj1->o.armor.armor_type] <
			     obj2->o.armor.armor_value - weapon_armor_table[3][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #1less#0 against crushing");
		}

		if (obj1->o.armor.armor_value - weapon_armor_table[4][obj1->o.armor.armor_type] >
			obj2->o.armor.armor_value - weapon_armor_table[4][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #2more#0 against slashing");
		}
		else if (obj1->o.armor.armor_value - weapon_armor_table[4][obj1->o.armor.armor_type] <
			     obj2->o.armor.armor_value - weapon_armor_table[4][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #1less#0 against slashing");
		}

		if (obj1->o.armor.armor_value - weapon_armor_table[7][obj1->o.armor.armor_type] >
			obj2->o.armor.armor_value - weapon_armor_table[7][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #2more#0 against gunshots");
		}
		else if (obj1->o.armor.armor_value - weapon_armor_table[7][obj1->o.armor.armor_type] <
			     obj2->o.armor.armor_value - weapon_armor_table[7][obj2->o.armor.armor_type])
		{
			sprintf (buffer + strlen(buffer), "\n   #6+#0 protects #1less#0 against gunshots");
		}
	}

    /**** Compare capacity of containers ****/

    else if ((GET_ITEM_TYPE (obj1) == ITEM_CONTAINER)||
            ( GET_ITEM_TYPE (obj1) == ITEM_DRINKCON ))
    {
        if (obj1->o.container.capacity > obj2->o.container.capacity)
        {
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would hold #2more#0");
        }
        else if (obj2->o.container.capacity > obj1->o.container.capacity)
        {
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would hold #1less#0");
        }
        else
        {
			sprintf (buffer + strlen(buffer), "\n   #6+#0 would hold about the #3same#0");
        }
    }

    send_to_char (buffer, ch);
	send_to_char ("\n", ch);

    return;
}

char *
writing_adj (int skill)
{
    if (skill <= 10)
        return "crudely";
    if (skill <= 15)
        return "poorly";
    if (skill <= 20)
        return "functionally";
    if (skill <= 30)
        return "with skill";
    if (skill <= 35)
        return "with great skill";
    if (skill <= 45)
        return "artfully";
    if (skill <= 49)
        return "beautifully";
    return "flawlessly";
}

int
decipher_script (CHAR_DATA * ch, int script, int language, int skill)  
{
    double check = 0;

	script = 40;

    if (!real_skill (ch, script) ||!real_skill (ch, SKILL_EDUCATION)) //languages was swapped out for education instead
        return 0; 

    if (skill > 0 && skill <= 10)
        check = 60;
    else if (skill > 15 && skill < 20)
        check = 40;
    else if (skill >= 20 && skill < 35)
        check = 30;
    else if (skill >= 35 && skill < 50)
        check = 20;
    else if (skill >= 50)
        check = 10;

    skill_use (ch, script, 0);
  //  skill_use (ch, language, 0);
    skill_use (ch, SKILL_EDUCATION, 0);

    if (((ch->skills[script] * .50) + (ch->skills[language] * .10) +
            (ch->skills[SKILL_EDUCATION] * .20)) >= check)
        return 1;
    else
        return 0;
}

void
reading_check (CHAR_DATA * ch, OBJ_DATA * obj, WRITING_DATA * writing,
               int page)
{
    static char output[MAX_STRING_LENGTH] = { '\0' };

    if (!writing || !writing->message)
    {
        send_to_char
        ("There seems to be a problem with this object. Please report it to a staff member.\n",
         ch);
        return;
    }

    if (!ch->skills[writing->script]
            && strcasecmp (writing->author, ch->tname) != STR_MATCH)
    {
        sprintf (output,
                 "This document is written in a script entirely unfamiliar to you.");
        act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (!ch->skills[writing->language] && real_skill (ch, writing->script)
            && strcasecmp (writing->author, ch->tname) != STR_MATCH)
    {
        sprintf (output,
                 "Although you recognise the script as %s, the language in which this document is written is unknown to you.",
                 skills[writing->script]);
        act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if (!decipher_script
            (ch, writing->script, writing->language, writing->skill)
            && (strcasecmp (writing->author, ch->tname) != STR_MATCH
                || !real_skill (ch, writing->script)))
    {
        sprintf (output,
                 "You find that you can make neither heads nor tails of this document.");
        act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    if ( obj && IS_ELECTRIC( obj ) )
        sprintf( output, "The %s letters on #2%s#0's screen bear a message:",
            writing->ink,
            obj->short_description
        );
    else {
        if ( !page )
            sprintf( output, "#2On %s, %s letters bear a message:",
                obj->short_description,
                writing->ink
            );
        else
            sprintf( output, "On #2page %d#0, %s letters bear a message:", page - 1, writing->ink );
    }

    act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    send_to_char ("\n", ch);

    if (strcasecmp (writing->author, ch->tname) != STR_MATCH)
    {
        skill_use (ch, writing->script, 0);
        if (!number (0, 1))
            skill_use (ch, writing->language, 0);
        if (!number (0, 2))
            skill_use (ch, SKILL_EDUCATION, 0);
    }

    sprintf (output, "%s", writing->message);
    page_string (ch->descr(), output);
}

char *
article (const char *string)
{
    if (strcasecmp (string, "something") == STR_MATCH)
        return "";

    if (*string == 'a' || *string == 'e' || *string == 'i' ||
            *string == 'o' || *string == 'u')
        return "an ";
    else
        return "a ";
}

void
show_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, int mode)
{
    int found = 0;
    int crafts_found = 0;
    int first_seen;
    unsigned int i = 0;
    WRITING_DATA *writing = NULL;
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj2 = NULL, *tobj = NULL;
    WOUND_DATA *wound = NULL;
    LODGED_OBJECT_INFO *lodged = NULL;
    AFFECTED_TYPE *af = NULL;
    CLAN_DATA *tclan = NULL;
    char *p = '\0';
    char output[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char buf3[MAX_STRING_LENGTH] = { '\0' };
    char buf4[MAX_STRING_LENGTH] = { '\0' };
    TRAP_DATA *trap = NULL;
    int dam = 0;
    int j = 0;

    *buffer = '\0';

    if ((mode == 0) && obj->description)
    {

        if (IS_FURNISH(obj))
            return;

        if (obj->nVirtual == VNUM_CORPSE)
            return;

        if (GET_ITEM_TYPE(obj) == ITEM_BULLET)
            return;

        if (GET_ITEM_TYPE(obj) == ITEM_CASE)
            return;

        if (obj->nVirtual == VNUM_WHOLEARM)
            return;

        if (obj->nVirtual == VNUM_WHOLELEG)
            return;

        if (obj->nVirtual == VNUM_ZOMBIE_HEAD)
            return;

        if (obj->nVirtual == VNUM_LOWERTORSO)
            return;

        if (obj->nVirtual == VNUM_UPPERTORSO)
            return;

        strcat (buffer, "#2");

        if (obj->omote_str)
        {
            sprintf (buf, "%s %s", obj_short_desc(obj), obj->omote_str);
            if (obj->count > 1)
            {
                sprintf (buf + strlen (buf), " (x%d)", obj->count);
            }
            if (buf[0] == '#')
            {
                buf[2] = toupper (buf[2]);
            }
            else
            {
                buf[0] = toupper (buf[0]);
            }
            strcat (buffer, buf);
        }
        else
        {
            strcat (buffer, obj_desc (obj));
        }

        if (get_obj_affect (obj, MAGIC_HIDDEN))
        {
            strcat (buffer, " (hidden from view)");

            if (GET_ITEM_TYPE(obj) == ITEM_TRAP)
            {
                for (af = obj->xaffected; af; af = af->next)
                {
                    if (af->type == MAGIC_HIDDEN)
                    {
                        if (af->a.hidden.uu6 == ch->coldload_id)
                        {
                            strcat (buffer, " (yours)");
                            break;
                        }
                    }
                }
            }
        }

        if (IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC))
            strcat (buffer, " #6(vNPC)#0");

        if (GET_ITEM_TYPE (obj) == ITEM_LIGHT &&
                obj->o.light.hours && obj->o.light.on)
            strcat (buffer, " #1(lit)#0");
        else if (GET_ITEM_TYPE (obj) == ITEM_CHEM_SMOKE &&
                obj->o.smoke.hours && obj->o.smoke.on)
            strcat (buffer, " #1(lit)#0");
        else if (GET_ITEM_TYPE (obj) == ITEM_E_LIGHT &&
                obj->o.elecs.status)
            strcat (buffer, " #1(lit)#0");

        //if (GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE && obj->poison)
        //    strcat (buffer, " #1(full)#0");

        strcat (buffer, "#0");
        reformat_string (buffer, &p);
        sprintf (buffer, "%s", p);
        mem_free (p); // char*
        buffer[strlen (buffer) - 1] = '\0';
    }

    if (mode == 7 && obj->description)
    {
        strcpy (buffer, "#2");
        strcat (buffer, obj_desc (obj));
        strcat (buffer, "#0");
        if (IS_TABLE (obj))
        {
            for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
			{
                if ((obj2->obj_flags.type_flag == ITEM_LIGHT && obj2->o.light.hours && obj2->o.light.on) ||
					(obj2->obj_flags.type_flag == ITEM_E_LIGHT && obj2->o.elecs.status))
                {
                    strcat (buffer, " #1(Illuminated)#0");
                    continue;
                }
			}
        }

        reformat_string (buffer, &p);
        sprintf (buffer, "%s", p);
        mem_free (p); //char*
        buffer[strlen (buffer) - 1] = '\0';
    }

    else if (obj->short_description && ((mode == 1) ||
                                        (mode == 2) || (mode == 3)
                                        || (mode == 4)))
    {

        strcpy (buffer, "#2");
        strcat (buffer, obj_short_desc (obj));
        strcat (buffer, "#0");

        if (IS_SET (obj->obj_flags.extra_flags, ITEM_DESTROYED))
            strcat (buffer, " #1(destroyed)#0");

        if (GET_TRUST (ch) && (GET_ITEM_TYPE (obj) == ITEM_MONEY))
            sprintf (buffer + strlen (buffer), " (%d)", obj->count);
    }

    else if (mode == 5 || mode == 15)
    {

        if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
        {
            if (!obj->writing_loaded)
                load_writing (obj);
            if (obj->writing && obj->writing->message)
            {
                if (!IS_MORTAL (ch))
                {
                    sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
                             obj->writing->author, obj->writing->date);
                    send_to_char (buf, ch);
                }
                if (obj->carried_by && obj->carried_by != ch && IS_MORTAL (ch))
                {
                    send_to_char
                    ("You aren't quite close enough to make out the words.\n",
                     ch);
                    return;
                }
                reading_check (ch, obj, obj->writing, 0);
                return;
            }
            else
            {
                sprintf (buf, "%s#0 seems to be blank.",
                         obj_short_desc(obj));
                sprintf (buffer, "#2%s\n", CAP (buf));
                return;
            }
        }
        else if ( IS_BOOK( obj ) )
        {

            if ( IS_ELECTRIC( obj ) && obj->o.elecs.status == 0 )
            {
                send_to_char( "The device's display is blank, it needs to be turned on.\n", ch );
                return;
            }

            if (!obj->writing_loaded)
                load_writing (obj);

            if (!obj->open || obj->open == 0 )
            {
                if ( obj->full_description && *obj->full_description )
                    strcat (buffer, obj->full_description);
                else {
                    sprintf (buffer, "   It is #2%s#0.\n", OBJS (obj, ch));

                    if ( IS_ELECTRIC( obj ) )
                    {
                        sprintf( buffer + strlen( buffer ), "\n   #6This device seems to have %d file blocks.\n",
                            obj->o.od.value[ GET_PAGE_OVAL( obj ) ]
                        );
                    } else {
                        sprintf( buffer + strlen( buffer ), "\n   #6This book seems to have %d pages remaining.\n",
                            obj->o.od.value[ GET_PAGE_OVAL( obj ) ]
                        );
                    }
                }
                if (obj->book_title)
                {
                    if ( decipher_script( ch, obj->title_script, obj->title_language, obj->title_skill) )
                    {
                        if ( IS_ELECTRIC( obj ) )
                            sprintf( buf, "   #6The name of the device seems to be '%s.'",
                                obj->book_title
                            );
                        else
                            sprintf( buf, "   #6The book has been entitled '%s.'",
                                obj->book_title
                            );
                    } else
                    {
                        if ( IS_ELECTRIC( obj ) )
                            sprintf( buf, "   #6You cannot make out the device's name." );
                        else
                            sprintf( buf, "   #6You cannot quite decipher what seems to be this book's title.#0\n" );
                    }

                    reformat_string (buf, &p);
                    strcat (buffer, "\n");
                    strcat (buffer, p);
                    mem_free (p); //char*
                }
                page_string (ch->descr(), buffer);
                return;
            }
            if (obj->carried_by && obj->carried_by != ch && IS_MORTAL (ch))
            {
                send_to_char
                ("You aren't quite close enough to make out the words.\n",
                 ch);
                return;
            }

            if (!obj->writing )
            {
                if ( IS_ELECTRIC( obj ) )
                {
                    if ( !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] ) {
                        send_to_char( "It's memory unit seems to be corrupt.\n", ch);
                        return;
                    }
                }
                else
                {
                    if ( !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] ) {
                        send_to_char ("All its pages have been torn out.\n", ch);
                        return;
                    }
                }
            }

            for (i = 2, writing = obj->writing; i <= obj->open; i++)
            {
                if (!writing->next_page)
                    break;

                writing = writing->next_page;
            }

            if (writing && writing->message)
            {
                if (strcasecmp (writing->message, "blank") != STR_MATCH)
                {
                    if (!IS_MORTAL (ch))
                    {
                        if ( IS_ELECTRIC( obj ) )
                            sprintf( buf, "#2[Timestamp: %s]#0\n\n",
                                writing->date
                            );
                        else
                            sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
                                 writing->author,
                                 writing->date
                            );

                        send_to_char (buf, ch);
                    }
                    reading_check (ch, obj, writing, i);
                    return;
                }
                else
                {
                    if ( IS_ELECTRIC( obj ) )
                        sprintf( buf, "This file block has yet to be initialized.\n" );
                    else
                        sprintf (buf, "This page seems to be blank.\n" );

                    sprintf (buffer, "%s\n", CAP (buf));
                    send_to_char (buffer, ch);
                    return;
                }
            }
            else
            {
                if ( IS_ELECTRIC( obj ) )
                    sprintf( buf, "This file block has yet to be initialized.\n" );
                else
                    sprintf( buf, "This page seems to be blank.\n" );

                sprintf (buffer, "%s\n", CAP (buf));
                send_to_char (buffer, ch);
                return;
            }
        }

        else
        {
            if (mode == 5 && obj->full_description && *obj->full_description)
            {

                if (obj->dec_desc && obj->dec_style)
                {
                    for (size_t y = 0; y < strlen (obj->full_description) - 1; y++)
                    {
                        sprintf (buf2 + strlen (buf2), "%c", obj->full_description[y]);
                    }

                    sprintf (buf, "%s With %s skill and %s, this item has a %s and %s decoration of: %s", buf2,
                     (obj->dec_quality >= 75 ? dec_skills[8] : obj->dec_quality >= 70 ? dec_skills[7] : obj->dec_quality >= 60 ? dec_skills[6] : obj->dec_quality >= 50 ? dec_skills[5] : obj->dec_quality >= 40 ? dec_skills[4] : obj->dec_quality >= 30 ? dec_skills[3] : obj->dec_quality >= 20 ? dec_skills[2] : obj->dec_quality >= 10 ? dec_skills[1] : dec_skills[0]),
                     obj->dec_style, dec_size[obj->dec_size], dec_conditions[obj->dec_condition], obj->dec_desc);

                    reformat_string (buf, &p);
                    strcpy (buffer, p);
                    mem_free (p); //char*
                }
                else
                {
                    strcpy (buffer, obj->full_description);
                }


            }
            else
                sprintf (buffer, "   It is #2%s#0.\n", OBJS (obj, ch));


            if (GET_ITEM_TYPE(obj) == ITEM_ARMOR && mode == 15)
            {
                *buf = '\0';
                *buf2 = '\0';

                for (i = 0; i < MAX_HITLOC; i++)
                {
                    if (IS_SET (obj->o.od.value[2], (1 << i)))
                    {
                        if (!*buf2)
                            sprintf (buf2 + strlen (buf2), "#0%s#6", armor_locs[i]);
                        // A fun shortcut - if our oval is less than double the bit,
                        // then it's obviously the last bit. Hah-hah!
                        else if (obj->o.od.value[2] < ((1 << i)*2))
                            sprintf (buf2 + strlen (buf2), " and #0%s#6", armor_locs[i]);
                        else
                            sprintf (buf2 + strlen (buf2), ", #0%s#6", armor_locs[i]);
                    }
                }

                if (*buf2)
                    sprintf (buf, "\n\n   #6It will cover the %s", buf2);

                *buf2 = '\0';

                for (i = 0; i < MAX_HITLOC; i++)
                {
                    if (IS_SET (obj->o.od.value[3], (1 << i)))
                    {
                        if (!*buf2)
                            sprintf (buf2 + strlen (buf2), "#0%s#6", armor_locs[i]);
                        // A fun shortcut - if our oval is less than double the bit,
                        // then it's obviously the last bit. Hah-hah!
                        else if (obj->o.od.value[3] < ((1 << i)*2))
                            sprintf (buf2 + strlen (buf2), " and #0%s#6", armor_locs[i]);
                        else
                            sprintf (buf2 + strlen (buf2), ", #0%s#6", armor_locs[i]);
                    }
                }

                if (*buf && *buf2)
                {
                    sprintf (buf + strlen(buf), ", and partially cover the %s.#0\n", buf2);
                }
                else if (!*buf && *buf2)
                {
                    sprintf (buf, "\n\n   #6It will partially cover the %s.#0\n", buf2);
                }
                else if (*buf && !*buf2)
                {
                    strcat(buf, ".#0\n");
                }

                if (*buf)
                {
                    reformat_string (buf, &p);
                    strcat (buffer, p);
                    mem_free (p); //char*
                    p = 0;

                }
            }
            else if (GET_ITEM_TYPE(obj) == ITEM_WORN && mode == 15)
            {
                *buf = '\0';
                *buf2 = '\0';

                for (i = 0; i < MAX_HITLOC; i++)
                {
                    if (IS_SET (obj->o.od.value[2], (1 << i)))
                    {
                        if (!*buf2)
                            sprintf (buf2 + strlen (buf2), "#0%s#6", armor_locs[i]);
                        // A fun shortcut - if our oval is less than double the bit,
                        // then it's obviously the last bit. Hah-hah!
                        else if (obj->o.od.value[2] < ((1 << i)*2))
                            sprintf (buf2 + strlen (buf2), " and #0%s#6", armor_locs[i]);
                        else
                            sprintf (buf2 + strlen (buf2), ", #0%s#6", armor_locs[i]);
                    }
                }

                if (*buf2)
                    sprintf (buf, "\n\n   #6It is worn over the %s", buf2);

                *buf2 = '\0';

                for (i = 0; i < MAX_HITLOC; i++)
                {
                    if (IS_SET (obj->o.od.value[3], (1 << i)))
                    {
                        if (!*buf2)
                            sprintf (buf2 + strlen (buf2), "#0%s#6", armor_locs[i]);
                        // A fun shortcut - if our oval is less than double the bit,
                        // then it's obviously the last bit. Hah-hah!
                        else if (obj->o.od.value[3] < ((1 << i)*2))
                            sprintf (buf2 + strlen (buf2), " and #0%s#6", armor_locs[i]);
                        else
                            sprintf (buf2 + strlen (buf2), ", #0%s#6", armor_locs[i]);
                    }
                }

                if (*buf && *buf2)
                {
                    sprintf (buf + strlen(buf), ", and worn about the %s.#0\n", buf2);
                }
                else if (!*buf && *buf2)
                {
                    sprintf (buf, "\n\n   #6It is worn about the %s.#0\n", buf2);
                }
                else if (*buf && !*buf2)
                {
                    strcat(buf, ".#0\n");
                }

                if (*buf)
                {
                    reformat_string (buf, &p);
                    strcat (buffer, p);
                    mem_free (p); //char*
                    p = 0;

                }
            }

            if (obj->damage || has_enviro_conds(obj) || obj->scent)
            {
                if (mode == 15)
                {
                    sprintf (buffer + strlen (buffer), "%s", object__examine_damage (obj, 0));
                }
                else
                {
                    sprintf (buffer + strlen (buffer), "%s", object__examine_damage (obj, 1));
                }
            }

            /*
            if (IS_WEARABLE (obj) && obj->size && mode == 15)
              {
                sprintf (buffer + strlen (buffer),
                     "\n   #6This garment would fit individuals wearing size %s.#0",
                     sizes_named[obj->size]);
              }
            */

            if (GET_ITEM_TYPE (obj) == ITEM_TOSSABLE && mode == 15
                    && obj->desc_keys && strlen (obj->desc_keys))
            {
                if (obj->clan_data)
                {
                    if (is_clan_member(ch, obj->clan_data->name))
                    {
                        tclan = get_clandef(obj->clan_data->name);
                        sprintf (buf, "   You see the mark of %s.\n",
                                 tclan->literal);

                        reformat_string (buf, &p);
                        strcat (buffer, "\n");
                        strcat (buffer, p);
                        mem_free (p); //char*
                        p = 0;
                    }
                }

                strcat (buffer,
                        "\n   #6Its sides are marked as follows:#0\n      #6");

                for (i = 0; i < strlen (obj->desc_keys); i++)
                {
                    if (obj->desc_keys[i] == ' ')
                    {
                        strcat (buffer, ", ");
                    }
                    else
                    {
                        buffer[strlen (buffer) + 1] = '\0';
                        buffer[strlen (buffer)] = obj->desc_keys[i];
                    }
                }

                strcat (buffer, "#0\n");

            }

            if (GET_ITEM_TYPE (obj) == ITEM_SMALLGOOD && mode == 15)
            {
                sprintf (buffer + strlen (buffer),
                         "\n   #6There are %d sales of %s remaining.#0\n",
                         obj->o.od.value[0], smallgood_types[obj->o.od.value[1]]);
            }

            if (GET_ITEM_TYPE (obj) == ITEM_COMPONENT && mode == 15 && obj->o.od.value[2])
            {
                sprintf (buffer + strlen (buffer), "\n   #6This item has #0%d#6 uses remaining.#0\n", obj->o.od.value[0]);
            }
            if (GET_ITEM_TYPE (obj) == ITEM_TOOL && mode == 15)
            {
                bool first = true;

                sprintf (buf + strlen (buf), "\n   #6This tool");

                if (obj->o.od.value[0] >= 0 || obj->o.od.value[3] || obj->o.od.value[4])
                {


                if (obj->o.od.value[0] >= 0)
                {
                    sprintf (buf + strlen (buf), " has #0%d#6 uses remaining", obj->o.od.value[0]);
                    first = false;
                }

                if (obj->o.od.value[3])
                {
                    sprintf (buf + strlen (buf), "%s requires at least a #0%s Artitry#6 skill to use", (first ? "" : ","), skill_lev(obj->o.od.value[3]));
                    first = false;
                }

                if (obj->o.od.value[4])
                {
                    sprintf (buf + strlen (buf), "%s can be used more ingeniously until a #0%s Artistry#6 level", (first ? "" : ","), skill_lev(obj->o.od.value[4]));
                    first = false;
                }
                }

                if (first)
                    sprintf(buf + strlen(buf), " can decorate items of the following material type/s: ");
                else
                    sprintf(buf + strlen(buf), ".\n   It can decorate items of the following material type/s:");

                if (obj->o.od.value[5] == 0)
                {
                    sprintf (buf + strlen (buf), " #0any and all#6");
                }
                else
                {
                    for (i = 0; *materials[i] != '\n'; i++)
                    {
                        if (IS_SET (obj->o.od.value[5], (1 << i)))
                            sprintf (buf + strlen (buf), " #0%s#6", materials[i]);
                    }
                }

                strcat (buf, ".#0");
                reformat_string (buf, &p);
                strcat (buffer, "#0\n");
                strcat (buffer, p);
                mem_free (p); //char*

            }
            if (GET_ITEM_TYPE (obj) == ITEM_CLUSTER && mode == 15)
            {
                i = (obj->o.od.value[0] = 0 ? SKILL_FORAGE : obj->o.od.value[0]);
                if (obj->o.od.value[0])
                {
                    sprintf (buf + strlen (buf), "\n   #6The #0%s#6 skill will assist in gathering from this object.", skills[obj->o.od.value[0]]);
                }

                if (vtoo(obj->o.od.value[2]) && !IS_SET(obj->o.od.value[1], CLUS_ONE_HIDE))
                {
                    if (i == SKILL_FORAGE || (IS_SET(obj->o.od.value[1], CLUS_ONE_SKILLED) && ch->skills[i] >= 10))
                    {
                        sprintf (buf + strlen (buf), " You could attempt to gather #2%s#6 from this object.", obj_short_desc(vtoo(obj->o.od.value[2])));
                    }
                }

                if (vtoo(obj->o.od.value[4]) && !IS_SET(obj->o.od.value[1], CLUS_TWO_HIDE))
                {
                    if (i == SKILL_FORAGE || (IS_SET(obj->o.od.value[1], CLUS_TWO_SKILLED) && ch->skills[i] >= 10))
                    {
                        sprintf (buf + strlen (buf), " You could attempt to gather #2%s#6 from this object.", obj_short_desc(vtoo(obj->o.od.value[4])));
                    }
                }

                strcat (buf, "#0");
                reformat_string (buf, &p);
                strcat (buffer, "#0\n");
                strcat (buffer, p);
                mem_free (p); //char*
            }
            if (GET_ITEM_TYPE (obj) == ITEM_PROGRESS && mode == 15 &&
                    obj->o.od.value[0] && obj->o.od.value[1])
            {
                sprintf (buffer + strlen (buffer), "\n   #6This item is #0%d percent#6 of the way to completion.#0\n", (obj->o.od.value[0] >= obj->o.od.value[1] ? 100 : ((obj->o.od.value[0] * 100) / obj->o.od.value[1])));
                if (vtoo(obj->o.od.value[2]) && obj->o.od.value[3] > 0)
                {
                    sprintf (buffer + strlen (buffer), "\n#6When completed, it will produce#0:\n   #2%s#0\n", vtoo(obj->o.od.value[2])->short_description);
                }
            }

            if (GET_ITEM_TYPE (obj) == ITEM_FIREARM && mode == 15)
			{

				if (!IS_SLING(obj))
				{

					sprintf(buf, "#6You'd guess it to be a #0%s#6 type firearm that %s. ", skills[obj->o.firearm.use_skill],
						(obj->o.firearm.handedness == 3 ? "is designed to be fired from #0both hands#6" : obj->o.firearm.handedness == 2 ? "is designed to be fired from #0one hand#6" : "is designed to be fired from either #0one or both hands#6"));


					if (!IS_DIRECT(obj) && IS_SET(obj->o.firearm.bits, GUN_DISPLAY) && ((obj->equiped_by && obj->equiped_by == ch) || !obj->equiped_by))
					{
						if (count_bullets(obj))
							sprintf(buf + strlen(buf), "There are #0%s#6 rounds currently loaded. ", verbal_number[count_bullets(obj)]);
					}

					*buf2 = '\0';


					if (obj->o.firearm.setting < 0)
						sprintf(buf2, " and it is currently #0jammed#6");
					if (obj->o.firearm.setting == 1)
						sprintf(buf2, " and #0the safety#6 is switched on");
					if (obj->o.firearm.setting == 2 || obj->o.firearm.setting == 3)
						sprintf(buf2, " and is switched to #0%s fire#6 mode", gun_set[obj->o.firearm.setting]);

					*buf3 = '\0';

					for (i = 0; i <= 11; i++)
					{
						if (IS_SET (obj->o.firearm.bits, (1 << i)))
						{
							if (!*buf3)
							{
								sprintf(buf3, ". It has the following features: #0%s#6", gun_bits[i]);
							}
							else
								sprintf (buf3 + strlen (buf3), " and #0%s#6", gun_bits[i]);
						}
					}

					*buf4 = '\0';

					int cap = 0;

					for (i = 12; i <= 18; i++)
					{
						if (IS_SET (obj->o.firearm.bits, (1 << i)))
						{
							if (i == 12)
							{
								sprintf (buf4, "a single ");
								cap = 1;
							}
							else if (i == 13)
							{
								sprintf (buf4, "up to five ");
								cap = 5;
							}
							else if (i == 14)
							{
								sprintf (buf4, "up to six ");
								cap = 6;
							}
							else
								sprintf (buf4, "a %s of ", gun_bits[i]);
						}
					}

					sprintf(buf + strlen(buf), "This firearm takes #0%s%s#6 rounds%s%s.#0", buf4, calibers[obj->o.firearm.caliber], buf2, buf3);

					reformat_string (buf, &p);
					strcat (buffer, "\n");
					strcat (buffer, p);
					mem_free (p); //char*

					if ((obj->equiped_by && obj->equiped_by == ch) || !obj->equiped_by)
					{
						if (obj->contains)
						{
							if (IS_DIRECT(obj))
							{
								sprintf (buf4, "%s#6%s firearm's chamber%s are full#0.\n", (cap == 1 ? "the" : count_all_bullets(obj) == cap ? "all" : verbal_number[count_all_bullets(obj)]), (cap > 1 ? " of" : ""), (cap > 1 ? "s" : ""));
								*buf4 = toupper(*buf4);
								strcat (buffer, "\n   #0");
								strcat (buffer, buf4);
							}
							else
							{
								sprintf (buffer + strlen(buffer), "\n   This firearm is loaded with #2%s#0.\n", obj->contains->short_description);
							}
						}
						else
							sprintf (buffer + strlen(buffer), "\n   #6This firearm is unloaded.#0\n");
					}
				}
            }
            if (GET_ITEM_TYPE (obj) == ITEM_CLIP && mode == 15)
            {
                sprintf(buf, "#6This %s can hold #0%s %s %s #6 round%s", gun_bits[obj->o.clip.type],
                        verbal_number[obj->o.clip.max], calibers[obj->o.clip.caliber], ammo_sizes[obj->o.clip.size], (obj->o.clip.max > 1 ? "s" : ""));

                if (obj->contains)
                {
                    sprintf (buf + strlen(buf), " and contains:#0");

                    *buf2 = '\0';

                    for (tobj = obj->contains; tobj; tobj = tobj->next_content)
                    {
                        sprintf(buf2 + strlen(buf2), "   #2%s#0\n", obj_short_desc(tobj));
                    }
                }
                else
                    strcat (buf, ".#0");


                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                strcat (buffer, buf2);
                mem_free (p); //char*




            }
            if (GET_ITEM_TYPE (obj) == ITEM_ROUND && mode == 15)
            {

                sprintf(buf, "#6%s #0%s %s %s#6 round%s.#0",
                        (obj->count > 1 ? "These are" : "This is a"), ammo_bits[obj->o.bullet.type], calibers[obj->o.bullet.caliber], ammo_sizes[obj->o.bullet.size], (obj->count > 1? "s" : ""));

                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
            }
            if (GET_ITEM_TYPE (obj) == ITEM_BULLET && mode == 15)
            {

                sprintf(buf, "#6%s #0%s %s#6 bullet%s.#0",
                        (obj->count > 1 ? "These are spent" : "This is a spent"), calibers[obj->o.bullet.caliber], ammo_sizes[obj->o.bullet.size], (obj->count > 1? "s" : ""));

                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
            }
            if (GET_ITEM_TYPE (obj) == ITEM_COVER && mode == 15)
            {
                *buf2 = '\0';

                for (i = 0; i < 7; i++)
                {
                    if (i == 6 && IS_SET (obj->o.od.value[3], (1 << i)))
                    {
                        if (*buf2)
                            strcat(buf2, ",");
                        sprintf (buf2 + strlen (buf2), " area");
                    }
                    else if (IS_SET (obj->o.od.value[3], (1 << i)))
                    {
                        if (*buf2)
                            strcat(buf2, ",");
                        sprintf (buf2 + strlen (buf2), " %s", dirs[i]);
                    }
                }

                if (!*buf2)
                    sprintf (buf2, "no");
                else
                    strcat (buf2, ".");

                sprintf(buf, "#6It would provide seating for up to #0%s %s#6 and #0%s cover#6 from #0%s#6 of the following directions:#0%s",
                        verbal_number[obj->o.od.value[5]], (obj->o.od.value[5] == 1 ? "person" : "people"), (obj->o.od.value[1] >= 2 ? "good" : obj->o.od.value[1] == 1 ? "ordinary" : "poor"), (obj->o.od.value[2] == 0 ? "all" : verbal_number[obj->o.od.value[2]]), buf2);

                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
            }
            if (GET_ITEM_TYPE (obj) == ITEM_CASE && mode == 15)
            {

                sprintf(buf, "#6%s #0%s#6 casing%s.#0",
                        (obj->count > 1 ? "These are spent" : "This is a spent"), calibers[obj->o.bullet.caliber], (obj->count > 1? "s" : ""));

                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
            }
            if (GET_ITEM_TYPE (obj) == ITEM_WEAPON && mode == 15)
            {
                sprintf (buffer + strlen (buffer),
                         "\n   #6You'd guess it to be a %s weapon.#0\n",
                         skills[obj->o.weapon.use_skill]);


                //if (obj->poison && obj->poison->uses > 5)
                //{
                //    sprintf (buffer + strlen (buffer),
                //             "\n  #6Something has been smeared upon the edges of the weapon.#0\n");
                //}

                
                if ((obj->o.weapon.use_skill == SKILL_ARCHERY)
                        && obj->o.od.value[5] == 1)
                {
                    sprintf (buffer + strlen(buffer),
                             "\n   This bow has been strung with #2%s#0.\n", obj->contains->short_description);
                }

/*                 
                if ((obj->o.weapon.use_skill == SKILL_ARCHERY)
                        && obj->loaded)
                {
                    sprintf (buffer + strlen(buffer),
                             "\n   This bow is nocked with #2%s#0.\n", obj->loaded->short_description);
                } 
*/

                if (IS_SET (obj->obj_flags.extra_flags, ITEM_THROWING)
                        && mode == 15)
                    sprintf (buffer + strlen (buffer),
                             "\n   #6This weapon appears ideally suited for throwing.#0\n");
            }
            else if (GET_ITEM_TYPE (obj) == ITEM_DWELLING && mode == 15)
            {
                sprintf (buffer + strlen (buffer),
                         "\n   #6You'd guess this dwelling could shelter %d individual%s.#0\n",
                         obj->o.od.value[1],
                         obj->o.od.value[1] != 1 ? "s" : "");
            }
            else if (GET_ITEM_TYPE (obj) == ITEM_CHEM_VIAL && mode == 15)
            {
                sprintf (buffer + strlen (buffer),
                         "\n   #6The contents of this vial appear to be %s concentrated.#0\n",
                         (obj->o.od.value[0] >= 11 ? "ingeniously" :
                          obj->o.od.value[0] == 10 ? "dangerously" :
                          obj->o.od.value[0] == 9 ? "extremely" :
                          obj->o.od.value[0] == 8 ? "strongly" :
                          obj->o.od.value[0] == 7 ? "highly" :
                          obj->o.od.value[0] == 6 ? "densely" :
                          obj->o.od.value[0] == 5 ? "moderately" :
                          obj->o.od.value[0] == 4 ? "somewhat" :
                          obj->o.od.value[0] == 3 ? "slightly" :
                          obj->o.od.value[0] == 2 ? "weakly" : "barely"));
            }
            else if (GET_ITEM_TYPE (obj) == ITEM_CHEM_KIT && mode == 15)
            {
                sprintf (buffer + strlen (buffer),
					"\n   #6This kit has %d components left.#0\n", obj->o.od.value[0]);
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_ARTWORK && obj->o.od.value[1] && obj->nVirtual == 5065 && mode == 15)
			{


			}
			else if (GET_ITEM_TYPE (obj) == ITEM_CARD && (mode == 15 || mode == 0))
			{

                if ((obj->o.od.value[4] == 1 && (ch == obj->equiped_by || ch == obj->carried_by)) || obj->o.od.value[4] == 2)
                {
                    *buf = '\0';
                    sprintf (buffer + strlen(buffer), "\n   #6This pile has the following #0%s#6 cards:#0", verbal_number[obj->o.od.value[5]]);

                    for (j = 0; j <= 3; j++)
                    {
                        *buf2 = '\0';

                        for (i = 0; i <= 12; i++)
                        {
                            if (IS_SET (obj->o.od.value[j], (1 << i)))
                            {
                                if (!*buf2)
                                    sprintf (buf2 + strlen (buf2), "\n#6%8s:#0 #0%s#6", suits[j], cards[i]);
                                else
                                    sprintf (buf2 + strlen (buf2), ", #0%s#6", cards[i]);
                            }
                        }

                        if (*buf2)
                        {
                            strcat(buf2, "#6.#0");
                            sprintf(buf + strlen(buf), "%s", buf2);
                        }
                    }

                    strcat (buffer, buf);
                }
                else
                    sprintf (buffer + strlen(buffer), "\n   #6This pile has #0%s#6 cards, but you are unable to see their values.#0", verbal_number[obj->o.od.value[5]]);
            }

            /*
            else if ((GET_ITEM_TYPE(obj) == ITEM_CHEM_NEEDLE ||
                      GET_ITEM_TYPE(obj) == ITEM_CHEM_SPRAY ||
                      GET_ITEM_TYPE(obj) == ITEM_CHEM_BOX) && mode == 15 && obj->poison)
            {
                if (obj->poison && ch->skills[SKILL_EDUCATION] >= 10)
                {

                    *buf2 = '\0';

                    for (poison = obj->poison; poison; poison = poison->next)
                    {
                        if (poison == obj->poison)
                            sprintf (buf2, "%s", vd_short("$chemical", (poison->poison_type - 20010)));
                        else if (poison->next)
                            sprintf (buf2 + strlen (buf2), "#6,#0 %s", vd_short("$chemical", (poison->poison_type - 20010)));
                        else
                            sprintf (buf2 + strlen (buf2), "#6 and#0 %s", vd_short("$chemical", (poison->poison_type - 20010)));
                    }

                    sprintf (buf,
                             "\n   #6Your scientific training allows you to identify it as containing a #0%s amount#6 of #0%s concentrated#6 compound synthesised from#0 %s.#0",
                             (obj->o.od.value[0] == 12 ? "large" :
                              obj->o.od.value[0] == 9 ? "copious" :
                              obj->o.od.value[0] == 6 ? "sizeable" :
                              obj->o.od.value[0] >= 3 ? "slight" : "miniscule"),

                             (obj->o.od.value[4] >= 11 ? "ingeniously" :
                              obj->o.od.value[4] == 10 ? "dangerously" :
                              obj->o.od.value[4] == 9 ? "extremely" :
                              obj->o.od.value[4] == 8 ? "strongly" :
                              obj->o.od.value[4] == 7 ? "highly" :
                              obj->o.od.value[4] == 6 ? "densely" :
                              obj->o.od.value[4] == 5 ? "moderately" :
                              obj->o.od.value[4] == 4 ? "somewhat" :
                              obj->o.od.value[4] == 3 ? "slightly" :
                              obj->o.od.value[4] == 2 ? "weakly" : "barely"), buf2);

                    reformat_string (buf, &p);
                    strcat (buffer, "\n");
                    strcat (buffer, p);
                    mem_free (p); //char*
                    p = 0;

                }
            }
            */

            else if (GET_ITEM_TYPE (obj) == ITEM_E_BATTERY && mode == 15)
            {
                sprintf (buffer + strlen(buffer), "\n   #6This is a #0%s#6 type power pack.#0", batt_bits[obj->o.battery.type]);
				if (!obj->o.battery.power)
				{
					sprintf(buffer + strlen(buffer), "#6 The indicator patch is blank,\nindicating the battery is out of power.#0");
				}
            }
            else if (IS_ELECTRIC(obj) && mode == 15)
            {
                // First off - a generic "is the power on or not"?
                sprintf (buf + strlen (buf),
                         "\n   #6Is it switched #0%s#6", (obj->o.elecs.status != 0 ? "on" : "off"));

                // Then, if the object is so enabled, show how much power is left.
                if ((obj->o.elecs.status != 0) && obj->contains && IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_POWER))
                {
                    sprintf (buf + strlen (buf), ", and has about #0%d#6 hours of power left", obj->contains->o.battery.power);
                }

                *buf2 = '\0';

                for (i = 0; i <= 3; i++)
                {
                    if (IS_SET (obj->o.elecs.elec_bits, (1 << i)))
                    {
                        if (!*buf2)
                            sprintf (buf2 + strlen (buf2), "#0%s#6", batt_bits[i]);
                        else
                            sprintf (buf2 + strlen (buf2), " and #0%s#6", batt_bits[i]);
                    }
                }

                if (!*buf2)
                    sprintf (buf2, "Internal");

                sprintf(buf + strlen (buf), ". It uses %s power units", buf2);

				if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_WATERPROOF))
				{
					sprintf(buf + strlen (buf), ". It looks reasonably waterproof");
				}

                switch (GET_ITEM_TYPE(obj))
                {
				case ITEM_E_CLOCK:
					if (obj->o.elecs.status != 0)
					{
						int minutes = 0;
						minutes = 4 * (15 * 60 - (next_hour_update - time (0))) / 60;
						sprintf (buf + strlen (buf), ". The clock display reads: #6%d:%s%d of %s#0", time_info.hour, minutes >= 10 ? "" : "0", minutes, earth_phase[time_info.phaseEarth]);
					}
					break;
				case ITEM_E_REMOTE:
                    if (obj->o.elecs.status != 0)
                    {
						if (obj->o.od.value[3])
						{
							if (obj->o.od.value[4] && get_char_id(obj->o.od.value[4]))
							{
								sprintf (buf + strlen (buf), ". It is monitoring #5%s#0", get_char_id(obj->o.od.value[3])->short_descr);
							}
							else if (get_char_id(obj->o.od.value[3]))
							{
								sprintf (buf + strlen (buf), ". It is synchronised to #5%s#0", get_char_id(obj->o.od.value[3])->short_descr);

								get_char_id(obj->o.od.value[3])->mob->controller = obj->coldload_id;
							}
							else
							{
								sprintf (buf + strlen (buf), ". It appears that it thinks it is synchronised to something, but the data is corrupt - switching it off and then on would fix it");
							}
						}
					}
					break;
                case ITEM_E_RADIO:
				case ITEM_E_BUG:
					if (obj->o.elecs.status != 0)
					{
						// A bit fiddly - we don't want folks just examining people and seeing the encryption. You either
						// need to be carrying it, or the object needs to be on the ground.
						if (obj->in_room > 0 || ch == obj->equiped_by || ch == obj->carried_by)
						{
							sprintf (buf + strlen (buf), ". It is set to transmit over channel #0%d#6", obj->o.radio.channel);

							if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_ENCRYPT) && obj->o.radio.encrypt)
								sprintf (buf + strlen (buf), " and is using an encryption code: #0%d#6", obj->o.radio.encrypt);
						}

                        if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_SCAN))
                            sprintf (buf + strlen (buf), ". It can also scan for active frequencies");

						if (IS_SET(obj->o.elecs.elec_bits, ELEC_FEA_RADIO_ONEWAY))
                            sprintf (buf + strlen (buf), ". It can receive, but cannot send transmissions");

                    }
                    break;
                case ITEM_E_BOOST:
                    if (obj->o.elecs.status != 0)
                    {
                        if (obj->o.od.value[3] && obj->o.od.value[5] && skills[obj->o.od.value[3]])
                        {
                            if (obj->o.od.value[5] < 0)
                            {
                                sprintf (buf + strlen (buf), ". You judge that this item would %shinder your %s skill",
                                         (obj->o.od.value[5] <= -15 ? "massively " : obj->o.od.value[5] <= -10 ? "greatly " : obj->o.od.value[5] <= -6 ? "significantly " : obj->o.od.value[5] <= -3 ? "" : "slightly "),
                                         skills[obj->o.od.value[3]]);
                            }
                            else if (obj->o.od.value[5] > 0)
                            {
                                sprintf (buf + strlen (buf), ". You judge that this item would %simprove your %s skill",
                                         (obj->o.od.value[5] >= 15 ? "massively " : obj->o.od.value[5] >= 10 ? "greatly " : obj->o.od.value[5] >= 6 ? "significantly " : obj->o.od.value[5] >= 3 ? "" : "slightly "),
                                         skills[obj->o.od.value[3]]);
                            }
                        }

                        if (obj->o.od.value[4] && obj->o.od.value[5] && skills[obj->o.od.value[4]])
                        {
                            if (obj->o.od.value[5] < 0)
                            {
                                sprintf (buf + strlen (buf), ". You judge that this item would %shinder your %s skill",
                                         (obj->o.od.value[5] <= -15 ? "massively " : obj->o.od.value[5] <= -10 ? "greatly " : obj->o.od.value[5] <= -6 ? "significantly " : obj->o.od.value[5] <= -3 ? "" : "slightly "),
                                         skills[obj->o.od.value[4]]);
                            }
                            else if (obj->o.od.value[5] > 0)
                            {
                                sprintf (buf + strlen (buf), ". You judge that this item would %simprove your %s skill",
                                         (obj->o.od.value[5] >= 15 ? "massively " : obj->o.od.value[5] >= 10 ? "greatly " : obj->o.od.value[5] >= 6 ? "significantly " : obj->o.od.value[5] >= 3 ? "" : "slightly "),
                                         skills[obj->o.od.value[4]]);
                            }
                        }
                    }
                    break;

                }

                sprintf (buf + strlen (buf), ".#0\n");
                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
                p = 0;

            }
            else if (GET_ITEM_TYPE (obj) == ITEM_HEALER_KIT && mode == 15)
            {
                if (ch->skills[SKILL_FIRSTAID]
                        && ch->skills[SKILL_FIRSTAID] >= obj->o.od.value[2])
                {
                    sprintf (buffer + strlen (buffer),
                             "\n   #6Used to Treat:#0 ");
                    if (IS_SET (obj->o.od.value[5], TREAT_ALL)
                            || !obj->o.od.value[5])
                        sprintf (buffer + strlen (buffer), " Any");
                    else
                    {
                        if (IS_SET (obj->o.od.value[5], TREAT_SLASH))
                            sprintf (buffer + strlen (buffer), " Slashes");
                        if (IS_SET (obj->o.od.value[5], TREAT_PUNCTURE))
                            sprintf (buffer + strlen (buffer), " Punctures");
                        if (IS_SET (obj->o.od.value[5], TREAT_BLUNT))
                            sprintf (buffer + strlen (buffer), " Contusions");
                        if (IS_SET (obj->o.od.value[5], TREAT_BURN))
                            sprintf (buffer + strlen (buffer), " Burns");
                        if (IS_SET (obj->o.od.value[5], TREAT_FROST))
                            sprintf (buffer + strlen (buffer), " Frostbite");
                        if (IS_SET (obj->o.od.value[5], TREAT_BLEED))
                            sprintf (buffer + strlen (buffer), " Bleeding");
						if (IS_SET (obj->o.od.value[5], TREAT_INTERNAL))
							sprintf (buffer + strlen (buffer), " Internal Injuries");
						if (IS_SET (obj->o.od.value[5], TREAT_INFECTION))
							sprintf (buffer + strlen (buffer), " Infections");
                    }
                    sprintf (buffer + strlen (buffer),
                             "\n   #6Uses Remaining:#0 %d\n",
                             obj->o.od.value[0]);
                }
                else
                {
                    sprintf (buffer + strlen (buffer),
                             "\n   #6Your training is not yet sufficient for this remedy.#0\n");
                }
            }
            else if (GET_ITEM_TYPE (obj) == ITEM_REPAIR_KIT && mode == 15)
            {
                /*
                if (obj->o.od.value[3] < 0 || obj->o.od.value[5] < 0)
                {
                sprintf (buffer + strlen (buffer),
                 " #1Error, No item type or skill associated with repair kit!#0");
                }
                */
                if (obj->o.od.value[3]
                        && (!ch->skills[(obj->o.od.value[3])]
                            || ch->skills[(obj->o.od.value[3])] <
                            obj->o.od.value[2]))
                {
                    sprintf (buffer + strlen (buffer),
                             "\n   #6Your training is not yet sufficient to use these materials.#0\n");
                }
                sprintf (buffer + strlen (buffer),
                         "\n   #6Skill Required:#0 ");
                if (obj->o.od.value[3] == 0)
                {
                    sprintf (buffer + strlen (buffer),
                             "None");
                }
                else
                {
                    sprintf (buffer + strlen (buffer),
                             "%s",
                             skills[(obj->o.od.value[3])]);
                }
                sprintf (buffer + strlen (buffer),
                         "\n   #6Repairs damages up to:#0 #6%s#0 in size",
                         (obj->o.od.value[1] > 5 ? damage_severity[5] : obj->o.od.value[1] < 0 ? damage_severity[0] : damage_severity[obj->o.od.value[1]]));

                sprintf (buffer + strlen (buffer),
                         "\n   #6Used to Repair:#0");
                if (obj->o.od.value[5] == 0)
                {
                    sprintf (buffer + strlen (buffer),
                             " #6Any item#0 made from the: ");
                }
                else
                {
                    sprintf (buffer + strlen (buffer),
                             " #6%s items#0 made from any the following: ",
                             item_types[(obj->o.od.value[5])]);
                }
                if (obj->o.od.value[4] == 0)
                {
                    sprintf (buffer + strlen (buffer), "#6any material#0.");
                }
                else
                {
                    for (i = 0; *materials[i] != '\n'; i++)
                    {
                        if (IS_SET (obj->o.od.value[4], (1 << i)))
                            sprintf (buffer + strlen (buffer), "#6%s#0 ", materials[i]);
                    }
                }

                int x = obj->o.od.value[0];

                sprintf (buffer + strlen (buffer),"\n   #6Uses Remaining:#0 %s\n",
                         (x >= 600 ? "Hundreds" : x >= 400 ? "Dozens" : x >= 300 ? "Many" : x >= 200 ? "Quite a lot" : x >= 100 ? "Several" : x >= 50 ? "A few" :
                          x > 0 ? "Barely any" : "None"));
            }

            if ((trap = obj->trap) && trap->trap_bit && !IS_SET(trap->trap_bit, TRAP_MISC_MYSTERY) && mode == 15)
            {
                *buf2 = '\0';

                for (i = 0; *trap_bits[i] != '\n'; i++)
                {
                    if (IS_SET (trap->trap_bit, (1 << i)))
                        sprintf (buf2 + strlen (buf2), "#0%s#6, ", trap_desc_bits[i]);
                }

                if (!*buf2)
                    sprintf (buf2, "none");

                sprintf (buf, "This item can be used as a component for a trap, and has the following characteristics: %s", buf2);

                if (trap->com_target_dice == -2)
                    sprintf (buf2, "the #0entire#6 room");
                else if (trap->com_target_dice == -1)
                    sprintf (buf2, "#0all#6 of the victim's #0group#6");
                else if (trap->com_target_dice && trap->com_target_side)
                    sprintf (buf2, "#0%dd%d#6 people of the victim's #0group#6", trap->com_target_dice, trap->com_target_side);
                else
                    sprintf (buf2, "a #0single#6 victim");

                sprintf (buf + strlen(buf), " and will target %s", buf2);

                sprintf (buf + strlen(buf), ". It would be #0%s#6 to use as part of a trap",
                         trap->com_diff >= 3 ? "very difficult" : trap->com_diff == 2 ? "difficult" : trap->com_diff == 1 ? "somewhat difficult" : "easy");

                if (trap->com_trig_shots > 0)
                    sprintf (buf + strlen(buf), ", and if used as a trigger, it can be used #0%d#6 time%s before needing to be reset",
                             trap->com_trig_shots, (trap->com_trig_shots > 1 ? "s" : ""));

                if (trap->dam1_dice && trap->dam1_sides)
                {
                    dam = ((trap->dam1_dice) + (trap->dam1_dice * trap->dam1_sides)) / 2;
                    sprintf (buf + strlen (buf), ". It will inflict %s #0%s#6 #0%s%s#6 to the #0%s#6",
                             (trap->dam1_times > 2 ? "#0multiple#6" : trap->dam1_times > 1 ? "a #0pair#6 of" : "a"),
                             (dam > 12 ? "huge" : dam > 8 ? "large" : dam > 5 ? "moderate" : dam > 3 ? "minor" : "small"), fight_damage[trap->dam1_type],
                             (trap->dam1_times > 1 ? "s" : ""),
                             (trap->dam1_loc > 7 ? "victim" : body_tab[0][trap->dam1_loc].part));

                    if (trap->dam2_dice && trap->dam2_sides)
                    {
                        dam = ((trap->dam2_dice) + (trap->dam2_dice * trap->dam2_sides)) / 2;
                        sprintf (buf + strlen (buf), ", and %s #0%s#6 #0%s%s#6 to the #0%s#6",
                                 (trap->dam2_times > 2 ? "#0multiple#6" : trap->dam2_times > 1 ? "a #0pair#6 of" : "a"),
                                 (dam > 12 ? "huge" : dam > 8 ? "large" : dam > 5 ? "moderate" : dam > 3 ? "minor" : "small"), fight_damage[trap->dam2_type],
                                 (trap->dam2_times > 1 ? "s" : ""),
                                 (trap->dam2_loc > 7 ? "victim" : body_tab[0][trap->dam2_loc].part));
                    }
                }

                sprintf (buf + strlen(buf), ".");

                reformat_string (buf, &p);

                sprintf (buffer + strlen (buffer), "\n#6%s#0", p);

                mem_free (p); //char*
                p = 0;

            }


            if (obj->clan_data)
            {
                if (is_clan_member(ch, obj->clan_data->name))
                {
                    tclan = get_clandef(obj->clan_data->name);
                    sprintf (buf, "   You see the mark of %s.\n",
                             tclan->literal);

                    reformat_string (buf, &p);
                    strcat (buffer, "\n");
                    strcat (buffer, p);
                    mem_free (p); //char*
                    p = 0;
                }
            }

            char *clan = NULL;

            if (get_obj_affect(obj, MAGIC_HIDDEN))
            {
                for (af = obj->xaffected; af; af = af->next)
                {
                    if (af->type == MAGIC_HIDDEN)
                    {
                        if (af->a.hidden.clan > 0)
                        {
                            clan = (get_clanid(af->a.hidden.clan))->literal;
                            if (clan && is_clan_member_player(ch, clan))
                            {
                                sprintf (buf, "   Near this hidden object is the mark of %s.\n", clan);
                                reformat_string (buf, &p);
                                strcat (buffer, "\n");
                                strcat (buffer, p);
                                mem_free (p); //char*
                                p = 0;

                                break;
                            }
                        }
                    }
                }
            }

            *buf = '\0';

            for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
            {
                if (!(af = get_affect (ch, i)))
                    continue;
                if (!af->a.craft || !af->a.craft->subcraft)
                    continue;
                if (!craft_uses (af->a.craft->subcraft, obj->nVirtual))
                    continue;
                if (crafts_found)
                    sprintf (buf + strlen (buf), ", ");
                sprintf (buf + strlen (buf), "'%s %s'",
                         af->a.craft->subcraft->command,
                         af->a.craft->subcraft->subcraft_name);
                crafts_found += 1;
            }

            SUBCRAFT_HEAD_DATA *tcraft;
            if (!IS_MORTAL (ch))
            {
                for (tcraft = crafts; tcraft; tcraft = tcraft->next)
                {
                    if (!craft_uses (tcraft, obj->nVirtual))
                        continue;
                    if (crafts_found)
                        sprintf (buf + strlen (buf), ", ");
                    sprintf (buf + strlen (buf), "'%s %s'",
                             tcraft->command,
                             tcraft->subcraft_name);
                    crafts_found += 1;
                }
            }


            if (crafts_found && mode == 15)
            {
                sprintf (output,
                         "   You realize that you could make use of this item in the following craft%s: %s.",
                         crafts_found != 1 ? "s" : "", buf);
                reformat_string (output, &p);
                sprintf (buffer + strlen (buffer), "\n%s", p);
                mem_free (p); // char*;
                p = 0;
            }

            if (obj->wounds)
            {
                sprintf (buf, "   It has ");
                for (wound = obj->wounds; wound; wound = wound->next)
                {
                    if (!wound->next && wound != obj->wounds)
                        sprintf (buf2, "and a %s %s on the %s", wound->severity,
                                 wound->name, expand_wound_loc (wound->location));
                    else
                        sprintf (buf2, "a %s %s on the %s", wound->severity,
                                 wound->name, expand_wound_loc (wound->location));
                    strcat (buf, buf2);
                    if (!wound->next)
                        strcat (buf, ".");
                    else
                        strcat (buf, ", ");
                }
                reformat_string (buf, &p);
                strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
                p = 0;
            }

            if (obj->lodged)
            {
                sprintf (buf, "   It has ");
                for (lodged = obj->lodged; lodged; lodged = lodged->next)
                {
                    tobj = load_object (lodged->vnum);
                    if (!tobj)
                        continue;
                    if (obj->nVirtual != VNUM_CORPSE)
                    {
                        if (!lodged->next && lodged != obj->lodged)
                            sprintf (buf2, "and #2%s#0 lodged in it",
                                     (lodged->colored ? lodged->short_description : tobj->short_description));
                        else if (!lodged->next && lodged == obj->lodged)
                            sprintf (buf2, "#2%s#0 lodged in it",
                                     (lodged->colored ? lodged->short_description : tobj->short_description));
                        else
                            sprintf (buf2, "#2%s#0",
                                     (lodged->colored ? lodged->short_description : tobj->short_description));
                    }
                    else
                    {
                        if (!lodged->next && lodged != obj->lodged)
                            sprintf (buf2, "and #2%s#0 lodged in the %s",
                                     (lodged->colored ? lodged->short_description : tobj->short_description),
                                     expand_wound_loc (lodged->location));
                        else
                            sprintf (buf2, "#2%s#0 lodged in the %s",
                                     (lodged->colored ? lodged->short_description : tobj->short_description),
                                     expand_wound_loc (lodged->location));
                    }
                    strcat (buf, buf2);
                    if (!lodged->next)
                        strcat (buf, ".");
                    else
                        strcat (buf, ", ");
                    extract_obj (tobj);
                }
                reformat_string (buf, &p);
                if (obj->wounds)
                    strcat (buffer, "\n");
                strcat (buffer, p);
                mem_free (p); //char*
                p = NULL;
            }

            first_seen = 1;

            if (IS_TABLE(obj))
            {
                for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
                {
                    if (CAN_SEE_OBJ (ch, obj2))
                    {

                        if (first_seen)
                            sprintf (buffer + strlen (buffer),
                                     "\nOn #2%s#0 you see:\n", OBJS (obj, ch));

                        first_seen = 0;


                        sprintf (buffer + strlen (buffer), "   %s",

                                 OBJS (obj2, ch));

                        if (GET_ITEM_TYPE (obj2) == ITEM_LIGHT && obj2->o.light.hours && obj2->o.light.on)
                            strcat (buffer + strlen (buffer), " #1(lit)#0\n");
                        else if (GET_ITEM_TYPE (obj2) == ITEM_CHEM_SMOKE && obj2->o.smoke.hours && obj2->o.smoke.on)
                            strcat (buffer + strlen (buffer), " #1(lit)#0\n");
                        //else if (GET_ITEM_TYPE (obj2) == ITEM_CHEM_NEEDLE && obj->poison)
                        //    strcat (buffer + strlen (buffer), " #1(full)#0\n");
                        else
                            strcat (buffer + strlen (buffer), "\n");

                    }
                }

                for (tch = ch->room->people; tch; tch = tch->next_in_room)
                {
                }
            }
            if (obj->nVirtual != VNUM_CORPSE)
            {
                /*
                				if ( obj->item_wear == 100 )
                					sprintf (buffer + strlen(buffer), "\n   It appears to be in flawless condition.\n");
                */
            }
        }
    }

    else if (mode == 6)
    {

        strcpy (buffer, "   ");

        if (CAN_SEE_OBJ (ch, obj) && IS_OBJ_STAT (obj, ITEM_INVISIBLE))
            strcat (buffer, "(invis) ");

        sprintf (buffer + strlen (buffer), "#2%s#0", obj_short_desc (obj));

        if (GET_TRUST (ch) && (GET_ITEM_TYPE (obj) == ITEM_MONEY))
            sprintf (buffer + strlen (buffer), " (%d)", obj->count);
    }

    if (mode != 3)
    {
        found = false;
        if (IS_OBJ_STAT (obj, ITEM_INVISIBLE))
        {
            strcat (buffer, "(invisible)");
            found = true;
        }
    }

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_LIGHT)
    {
        if (obj->o.light.hours && obj->o.light.on)
            sprintf (buffer + strlen (buffer), " #1(lit)#0");
        else if (obj->o.light.hours && !obj->o.light.on)
            sprintf (buffer + strlen (buffer), " #1(unlit)#0");
        else
            sprintf (buffer + strlen (buffer), " #1(spent)#0");
    }

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_E_LIGHT && obj->o.elecs.status)
    {
        if (obj->o.light.hours && obj->o.light.on)
            sprintf (buffer + strlen (buffer), " #1(lit)#0");
    }

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_CHEM_SMOKE)
    {
        if (obj->o.smoke.hours && obj->o.smoke.on)
            sprintf (buffer + strlen (buffer), " #1(lit)#0");
    }

    //if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_CHEM_NEEDLE && obj->poison)
    //    sprintf (buffer + strlen(buffer), " #1(full)#0");


    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title
            && decipher_script (ch, obj->title_script, obj->title_language,
                                obj->title_skill))
        sprintf (buffer, "#2\"%s\"#0", obj->book_title);
    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->open)
        sprintf (buffer + strlen (buffer), " #1(open)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && !obj->open)
        sprintf (buffer + strlen (buffer), " #1(closed)#0");

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_SLING && obj->attached)
        sprintf (buffer + strlen (buffer), " #6(loaded)#0");

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_FIREARM && obj->contains)
    {
        if ((obj->equiped_by && obj->equiped_by == ch) || !obj->equiped_by)
        {
            if (IS_DIRECT(obj) || IS_SLING(obj))
            {
                if (count_bullets(obj))
                    sprintf (buffer + strlen (buffer), " #6(loaded)#0");
                else if (count_all_bullets(obj))
                    sprintf (buffer + strlen (buffer), " #1(empty)#0");
            }
            else
            {
                if (obj->contains->contains)
                    sprintf (buffer + strlen (buffer), " #6(loaded)#0");
                else
                    sprintf (buffer + strlen (buffer), " #1(empty)#0");
            }
        }
    }

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_CLIP && obj->contains)
    {
        if ((obj->equiped_by && obj->equiped_by == ch) || !obj->equiped_by)
        {
            if (obj->o.od.value[0] == obj->o.od.value[1])
                sprintf (buffer + strlen (buffer), " #6(loaded)#0");
            else
                sprintf (buffer + strlen (buffer), " #3(partial)#0");
        }
    }

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_SHEATH && obj->contains)
        sprintf (buffer + strlen (buffer), " bearing %s#2%s#0",
                 article (OBJN (obj->contains, ch)), OBJN (obj->contains, ch));

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_HOLSTER && obj->contains)
        sprintf (buffer + strlen (buffer), " bearing %s#2%s#0",
                 article (OBJN (obj->contains, ch)), OBJN (obj->contains, ch));

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_QUIVER && obj->contains)
        sprintf (buffer + strlen (buffer), " bearing #2%ss#0",
                 OBJN (obj->contains, ch));

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_AMMO_BELT && obj->contains)
        sprintf (buffer + strlen (buffer), " bearing #2%ss#0",
                 OBJN (obj->contains, ch));

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BANDOLIER && obj->contains)
        sprintf (buffer + strlen (buffer), " bearing #2%ss#0",
                 OBJN (obj->contains, ch));

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
            obj->o.od.value[0] == obj->o.od.value[1])
        sprintf (buffer + strlen (buffer), " #1(full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
             obj->o.od.value[0] > obj->o.od.value[1] / 2)
        sprintf (buffer + strlen (buffer), " #1(mostly full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
             obj->o.od.value[0] == obj->o.od.value[1] / 2)
        sprintf (buffer + strlen (buffer), " #1(half full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
             obj->o.od.value[0] < obj->o.od.value[1] / 2
             && obj->o.od.value[0] > 0)
        sprintf (buffer + strlen (buffer), " #1(mostly empty)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
             obj->o.od.value[0] <= 0)
        sprintf (buffer + strlen (buffer), " #1(empty)#0");

    if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_TOOL && *obj->o.od.value >= 0 &&
        obj->o.od.value[0] == vtoo(obj->nVirtual)->o.od.value[0])
    sprintf (buffer + strlen (buffer), " #1(full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_TOOL && *obj->o.od.value >= 0 &&
         obj->o.od.value[0] > vtoo(obj->nVirtual)->o.od.value[0] / 2)
    sprintf (buffer + strlen (buffer), " #1(mostly full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_TOOL && *obj->o.od.value >= 0 &&
         obj->o.od.value[0] == vtoo(obj->nVirtual)->o.od.value[0] / 2)
    sprintf (buffer + strlen (buffer), " #1(half full)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_TOOL && *obj->o.od.value >= 0 &&
         obj->o.od.value[0] < vtoo(obj->nVirtual)->o.od.value[0] / 2
         && obj->o.od.value[0] > 0)
    sprintf (buffer + strlen (buffer), " #1(mostly empty)#0");
    else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_TOOL &&
         obj->o.od.value[0] == 0)
    sprintf (buffer + strlen (buffer), " #1(empty)#0");

    if (mode == 1 && IS_SET (obj->obj_flags.extra_flags2, ITEM_CONCEALED))
        sprintf (buffer + strlen (buffer), " #1(concealed)#0");
    
    if (buffer[strlen (buffer) - 1] != '\n')
        strcat (buffer, "\n");

    page_string (ch->descr(), buffer);

}

int
list_obj_to_char (OBJ_DATA * list, CHAR_DATA * ch, int mode, int show)
{
    OBJ_DATA *i = NULL;
    int found = 0, j = 0;
    int looked_for_tables = 0;
    int looked_for_corpses = 0;
    OBJ_DATA *obj = NULL;
    bool clump = false;
    int looked_for_limbs = 0;
    int looked_for_casings = 0;
    int looked_for_bullets = 0;

    found = false;

    if (!list &&
            (show != 7 && show != 8 && show != 9 && show != 10 && show != 11 && show != 12))
    {
        if (show)
            send_to_char ("Nothing.\n", ch);
        return 0;
    }

    for (i = list; i; i = i->next_content)
    {
        if ((show == 7 || show == 10) && (OBJ_MASS(i) <= 1 || !could_see_obj(ch, i)))
            continue;
        else if ((show == 8 || show == 11) && (OBJ_MASS(i) <= 1000 || !could_see_obj(ch, i)))
            continue;
        else if ((show == 9 || show == 12) && (OBJ_MASS(i) <= 2500 || !could_see_obj(ch, i)))
            continue;

        j++;
    }

    if (j >= 25 && show != 4  && show != 10 && show != 11 && show != 12 && !list->in_obj)
        clump = true;

    for (i = list; i; i = i->next_content)
    {
        if (could_see_obj (ch, i))
        {
            if ((show == 7 || show == 10) && (OBJ_MASS(i) <= 1 || !could_see_obj(ch, i)))
                continue;
            else if ((show == 8 || show == 11) && (OBJ_MASS(i) <= 1000 || !could_see_obj(ch, i)))
                continue;
            else if ((show == 9 || show == 12) && (OBJ_MASS(i) <= 2500 || !could_see_obj(ch, i)))
                continue;

            if (!mode && !looked_for_tables && IS_FURNISH(i))
            {

                for (obj = i; obj; obj = obj->next_content)
                    if (IS_FURNISH(i))
                        looked_for_tables++;

                if (looked_for_tables == 1)
                    show_obj_to_char (i, ch, 7);
                else if (looked_for_tables == 2)
                    send_to_char ("#6There are a couple of furnishings here.#0\n",
                                  ch);
                else
                    send_to_char ("#6There are several furnishings here.#0\n",
                                  ch);
            }

            if (!mode && !looked_for_casings && GET_ITEM_TYPE(i) == ITEM_CASE)
            {

                for (obj = i; obj; obj = obj->next_content)
                {
                    if (GET_ITEM_TYPE(obj) == ITEM_CASE)
                    {
                        looked_for_casings += obj->count;
                    }
                }

                if (looked_for_casings == 1)
                    show_obj_to_char (i, ch, 7);
                else if (looked_for_casings == 2)
                    send_to_char ("#6A couple of spent casings are here.#0\n", ch);
                else if (looked_for_casings < 24)
                    send_to_char ("#6Spent casings litter the ground underfoot.#0\n", ch);
                else if (looked_for_casings < 200)
                    send_to_char ("#6Dozens of spent casings blanket the area.#0\n", ch);
                else
                    send_to_char ("#6The area is buried by hundreds of spent casings.#0\n", ch);
            }

            if (!mode && !looked_for_bullets && GET_ITEM_TYPE(i) == ITEM_BULLET)
            {

                for (obj = i; obj; obj = obj->next_content)
                {
                    if (GET_ITEM_TYPE(obj) == ITEM_BULLET)
                    {
                        looked_for_bullets += obj->count;
                    }
                }

                if (looked_for_bullets == 1)
                    show_obj_to_char (i, ch, 7);
                else if (looked_for_bullets == 2)
                    send_to_char ("#6A couple of discharged bullets are here.#0\n", ch);
                else if (looked_for_bullets < 24)
                    send_to_char ("#6The area is studded with several discharged bullets.#0\n", ch);
                else if (looked_for_bullets < 200)
                    send_to_char ("#6Dozens of discharged bullets have peppered the area.#0\n", ch);
                else
                    send_to_char ("#6Hundreds of discharged bullets have perforated nearly everything in the area.#0\n", ch);
            }

            if (!mode && !looked_for_corpses && i->nVirtual == VNUM_CORPSE)
            {

                for (obj = i; obj; obj = obj->next_content)
                    if (obj->nVirtual == VNUM_CORPSE)
                        looked_for_corpses++;

                if (looked_for_corpses == 1)
                    show_obj_to_char (i, ch, 7);
                else if (looked_for_corpses == 2)
                    send_to_char ("#6There are a couple corpses here.#0\n", ch);
                else if (looked_for_corpses < 10)
                    send_to_char ("#6There are several corpses here.#0\n", ch);
                else if (looked_for_corpses < 30)
                    send_to_char ("#6There are a number of corpses littering the area.#0\n", ch);
                else
                    send_to_char ("#6There are mounds of corpses strewn everywhere.#0\n", ch);

            }

            if (!mode && !looked_for_limbs &&
                    (i->nVirtual == VNUM_WHOLEARM || i->nVirtual == VNUM_WHOLELEG || i->nVirtual == VNUM_LOWERTORSO ||
                     i->nVirtual == VNUM_UPPERTORSO || i->nVirtual == VNUM_ZOMBIE_HEAD))
            {

                for (obj = i; obj; obj = obj->next_content)
                    if (obj->nVirtual == VNUM_WHOLEARM || obj->nVirtual == VNUM_WHOLELEG || obj->nVirtual == VNUM_LOWERTORSO ||
                            obj->nVirtual == VNUM_UPPERTORSO || obj->nVirtual == VNUM_ZOMBIE_HEAD)
                        looked_for_limbs++;

                if (looked_for_limbs == 1)
                    show_obj_to_char (i, ch, 7);
                else if (looked_for_limbs == 2)
                    send_to_char ("#6There are a couple of scattered limbs.#0\n", ch);
                else if (looked_for_limbs < 10)
                    send_to_char ("#6Blood, gore and limbs are strewn across the area.#0\n", ch);
                else
                    send_to_char ("#6Masses of savaged, rotting flesh and limbs infests the area.#0\n", ch);
            }


            if (clump)
                continue;

            show_obj_to_char (i, ch, mode);

            found = true;
        }
    }

    if (clump)
    {
        if (j >= 25 && j < 35)
            send_to_char ("#2The area is strewn with a number of objects.#0\n",
                          ch);
        else if (j >= 35 && j < 45)
            send_to_char
            ("#2The area is strewn with a sizeable number of objects.#0\n", ch);
        else if (j >= 45 && j < 55)
            send_to_char
            ("#2The area is strewn with a large number of objects.#0\n", ch);
        else if (j >= 55 && j < 65)
            send_to_char
            ("#2The area is strewn with a great number of objects.#0\n", ch);
        else
            send_to_char
            ("#2The area is strewn with a staggering number of objects.#0\n",
             ch);
    }
    else if (!found && show && show != 4 && show != 7 && show != 8 && show != 9 && show != 10 && show != 11 && show != 12)
        send_to_char ("Nothing.\n", ch);

    return j;

}

int
list_char_to_char (CHAR_DATA * list, CHAR_DATA * ch, int mode)
{
    CHAR_DATA *i = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    int count = 0, j = 0;

    //if (IS_MORTAL (ch) && weather_info[ch->room->zone].state == HEAVY_SNOW
    //    && !IS_SET (ch->room->room_flags, INDOORS))
    //  return 0;

    for (i = list; i; i = i->next_in_room)
    {
        if (mode == 7 || mode == 10)
        {
            if (!could_see (ch, i))
                continue;
            else
                target_sighted (ch, i);
        }
        else if (mode == 8 || mode == 11)
        {
            if (get_weight (i) / 100 <= 10 || !could_see (ch, i))
                continue;
            else
                target_sighted (ch, i);
        }
        else if (mode == 9 || mode == 12)
        {
            if (get_weight (i) / 100 <= 25 || !could_see (ch, i))
                continue;
            else
                target_sighted (ch, i);
        }

        if (i == ch)
            continue;

        if (do_group_size(i) > 9)
            continue;

        j++;
    }

    if (do_group_size(ch) > 9 && !ch->following)
    {
        sprintf (buf, "#5A %s stands behind you.#0\n", group_makeup(ch));
        send_to_char(buf, ch);
    }

    if (j < 25 || mode == 4 || mode == 10 || mode == 11 || mode == 12 || ch->room->vnum == AMPITHEATRE)
    {
        for (i = list; i; i = i->next_in_room)
        {

            if (mode == 7 || mode == 10)
            {
                if (!could_see (ch, i))
                    continue;
            }
            else if (mode == 8 || mode == 11)
            {
                if (get_weight (i) / 100 <= 10 || !could_see (ch, i))
                    continue;
            }
            else if (mode == 9 || mode == 12)
            {
                if (get_weight (i) / 100 <= 25 || !could_see (ch, i))
                    continue;
            }

            if (ch != i && ch->vehicle != i)
            {
                if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (i))
                {
                    count++;
                    continue;
                }
                show_char_to_char (i, ch, mode);
            }
        }
    }
    else
    {
        if (j >= 25 && j < 35)
            send_to_char ("#5The area is filled by a crowd of individuals.\n#0",
                          ch);
        if (j >= 35 && j < 45)
            send_to_char
            ("#5The area is filled by a decently-sized crowd of individuals.\n#0",
             ch);
        else if (j >= 45 && j < 55)
            send_to_char
            ("#5The area is filled by a sizeable crowd of individuals.\n#0",
             ch);
        else if (j >= 55 && j < 65)
            send_to_char
            ("#5The area is filled by a large crowd of individuals.\n#0", ch);
        else if (j > 65)
            send_to_char
            ("#5The area is filled by an immense crowd of individuals.\n#0",
             ch);
    }

    if (count)
    {
        sprintf (buf,
                 "#5There %s %d other %s assembled here for the meeting.#0\n",
                 count != 1 ? "are" : "is", count,
                 count != 1 ? "people" : "person");
        send_to_char (buf, ch);
    }

    return j;

}

void
show_contents (CHAR_DATA * ch, char *argument, int cmd)
{
    if (cmd == 1)
    {
        if (!ch->room->contents)
        {
            send_to_char ("\n   None.\n", ch);
            return;
        }

        if (ch->room->contents)
        {
            send_to_char ("\n", ch);
            list_obj_to_char (ch->room->contents, ch, 0, 4);
        }
        return;
    }
    else if (cmd == 2)
    {
        if ((!ch->room->people || !ch->room->people->next_in_room))
        {
            send_to_char ("\n   None.\n", ch);
            return;
        }

        if (ch->room->people
                && (ch->room->people->next_in_room || ch->room->people != ch))
        {
            send_to_char ("\n", ch);
            list_char_to_char (ch->room->people, ch, 4);
        }
        return;
    }
}

void
do_contents (CHAR_DATA * ch, char *argument, int cmd)
{
    if (!ch->room->contents
            && (!ch->room->people || !ch->room->people->next_in_room))
    {
        send_to_char ("   None.\n", ch);
        return;
    }

    if (ch->room->contents)
    {
        send_to_char ("\n", ch);
        list_obj_to_char (ch->room->contents, ch, 0, 4);
    }

    if (ch->room->people
            && (ch->room->people->next_in_room || ch->room->people != ch))
    {
        send_to_char ("\n", ch);
        list_char_to_char (ch->room->people, ch, 4);
    }
}

int
enter_exit_msg (CHAR_DATA * ch, char *buffer)
{
    QE_DATA *qe = NULL;
    char *addon = '\0';
    bool isLeaving = false;
    char *e_dirs[] =
    {
        "to the north",
        "to the east",
        "to the south",
        "to the west",
        "up",
        "down",
        "to the outside",
        "to the inside"
    };
    char *a_dirs[] =
    {
        "the south",
        "the west",
        "the north",
        "the east",
        "below",
        "above",
        "the outside",
        "the inside"
    };

    extern QE_DATA *quarter_event_list;

    for (qe = quarter_event_list; qe; qe = qe->next)
        if (qe->ch == ch)
            break;

    if (!qe)
        return 0;

    addon = buffer + strlen (buffer);

    if (GET_FLAG (ch, FLAG_LEAVING))
    {
        isLeaving = true;
    }
    else if (GET_FLAG (ch, FLAG_ENTERING))
    {
        isLeaving = false;
    }
    else
        return 0;

    if (get_second_affect(ch, SA_LEAD, NULL) && !ch->following)
    {
        sprintf (addon, "#3%s is %s %s%s%s, a %s following behind.#0",
                 CAP (char_short (ch)),
                 (isLeaving ? "leaving" : "arriving from"),
                 (isLeaving ? e_dirs[qe->dir] : a_dirs[qe->dir]),
                 ((qe->travel_str == NULL && ch->travel_str) ? ", " : ""),
                 ((qe->travel_str ? qe->travel_str : (ch->travel_str ? ch->travel_str : ""))),
                 qe->group_str);
    }
    else
    {
        sprintf (addon, "#3%s is %s %s%s%s.#0",
                 CAP (char_short (ch)),
                 (isLeaving ? "leaving" : "arriving from"),
                 (isLeaving ? e_dirs[qe->dir] : a_dirs[qe->dir]),
                 ((qe->travel_str == NULL && ch->travel_str) ? ", " : ""),
                 ((qe->travel_str ? qe->travel_str : (ch->travel_str ? ch->travel_str : ""))));
    }
    addon[2] = toupper (addon[2]);

    return 1;
}

char *
height_phrase (CHAR_DATA * ch)
{
    static char phrase[MAX_STRING_LENGTH];

    sprintf (phrase, "gigantic");

    if (ch->height < 96)
        sprintf (phrase, "towering");
    if (ch->height < 78)
        sprintf (phrase, "very tall");
    if (ch->height < 75)
        sprintf (phrase, "tall");
    if (ch->height < 71)
        sprintf (phrase, "%s", frame_built[ch->frame]);
    if (ch->height < 60)
        sprintf (phrase, "short");
    if (ch->height < 48)
        sprintf (phrase, "very short");
    if (ch->height < 36)
        sprintf (phrase, "extremely short");
    if (ch->height < 24)
        sprintf (phrase, "tiny");

    return phrase;
}

// A or An needs to be based on the phrase, not the height
char *
char_short (CHAR_DATA * ch)
{
    OBJ_DATA *obj = NULL;
    static char buf[MAX_STRING_LENGTH] = { '\0' };
    char phrase[MAX_STRING_LENGTH] = { '\0' };

    if (!ch)
        return NULL;

    if (!ch->short_descr && !ch->pc && !ch->mob)
    {
        system_log
        ("AVOIDED CRASH BUG:  char_short: (!ch->short_descr && !ch->pc && !ch->mob)",
         true);
        return NULL;
    }


	// We automatically add paint to people in the Arena.
	int arena_color = 0;

	if (!IS_NPC(ch) && !IS_SET(ch->flags, FLAG_GUEST) && in_arena_room(ch) && is_clan_member(ch, "arena_blue"))
		arena_color = 1;
	else if (!IS_NPC(ch) && !IS_SET(ch->flags, FLAG_GUEST) && in_arena_room(ch) && is_clan_member(ch, "arena_red"))
		arena_color = 2;

    if (arena_color)
    {
		char arg1[MAX_STRING_LENGTH] = { '\0' };
		char *arg2;
		char arg3[MAX_STRING_LENGTH] = { '\0' };
        // Get our first token (the, an, a, some)

		arg2 = str_dup(ch->short_descr);

        arg2 = one_argument (arg2, arg1);

        // If it is an, we need to change it for grammatical correctness.
        if (!str_cmp(arg1, "an") || !str_cmp(arg1, "An"))
        {
          sprintf(arg1, "a");
        }

        // Now we get the next token so we know if we need to add a comma or not.
        one_argument (arg2, arg3);

        // Depending on how damage we are will change the variable,
        // Basically, it's first token + new descriptor + rest of the description, with an optional comma
        // if we've already got commas in our sdesc.
         sprintf (buf, "%s %s%s %s", arg1, arena_color == 2 ? "red-painted" : "blue-painted", (arg3[strlen(arg3) -1] == ',' ? "," : ""), arg2);

        return buf;
    }

    int z = armor_descript(ch);
    sprintf (phrase, "%s%s%s", (z >= 4 ? "extremely well armoured" : z == 3 ? "heavily armoured" : z == 2 ? "well armoured" : z == 1 ? "lightly armoured" : ""), (z ? ", " : ""), height_phrase (ch));

	if ((obj = get_equip (ch, WEAR_NECK_1)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
    {
        sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
                 phrase, obj->desc_keys);
        return buf;
    }

    if ((obj = get_equip (ch, WEAR_NECK_2)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
    {
        sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
                 phrase, obj->desc_keys);
        return buf;
    }

    if ((obj = get_equip (ch, WEAR_OVER)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
    {
        sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
                 phrase, obj->desc_keys);
        return buf;
    }

    if ((obj = get_equip (ch, WEAR_ABOUT)) &&
            IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
            IS_SET (ch->affected_by, AFF_HOODED))
    {
        sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
                 phrase, obj->desc_keys);
        return buf;
    }

    if (((obj = get_equip (ch, WEAR_HEAD))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
            || ((obj = get_equip (ch, WEAR_FACE))
                && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

        if (obj->obj_flags.type_flag == ITEM_WORN)
            sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
        else
            sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
        return buf;
    }

    if ((obj = get_equip (ch, WEAR_FACE))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
    {

        if (obj->obj_flags.type_flag == ITEM_WORN)
            sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
        else
            sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
        return buf;
    }


    return ch->short_descr;
}

char *
char_long (CHAR_DATA * ch, int show_tname)
{
    OBJ_DATA *obj = NULL;
    static char buf[MAX_STRING_LENGTH] = { '\0' };
    char phrase[20] = { '\0' };
    char tname[MAX_STRING_LENGTH] = { '\0' };

    sprintf (phrase, "%s", height_phrase (ch));

    if (show_tname && is_hooded (ch))
        sprintf (tname, "(%s) ", ch->tname);
    else
        *tname = '\0';

    if ((obj = get_equip (ch, WEAR_NECK_1))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
    {
        if (ch->pmote_str)
        {
            if (*(ch->pmote_str) == '\'')
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
            else
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
        }
        else
        {
            sprintf (buf, "A %s, %s person %sis here.", phrase,
                     obj->desc_keys, tname);
            return buf;
        }
    }
    if ((obj = get_equip (ch, WEAR_NECK_1))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
    {
        if (ch->pmote_str)
        {
            if (*(ch->pmote_str) == '\'')
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
            else
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
        }
        else
        {
            sprintf (buf, "A %s, %s person %sis here.", phrase,
                     obj->desc_keys, tname);
            return buf;
        }
    }
    if ((obj = get_equip (ch, WEAR_ABOUT))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
            && IS_SET (ch->affected_by, AFF_HOODED))
    {
        if (ch->pmote_str)
        {
            if (*(ch->pmote_str) == '\'')
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
            else
            {
                sprintf (buf, "A %s, %s person%s %s", phrase,
                         obj->desc_keys, tname, ch->pmote_str);
                return buf;
            }
        }
        else
        {
            sprintf (buf, "A %s, %s person %sis here.", phrase,
                     obj->desc_keys, tname);
            return buf;
        }
    }

    if (((obj = get_equip (ch, WEAR_HEAD))
            && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
            || ((obj = get_equip (ch, WEAR_FACE))
                && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

        if (obj->obj_flags.type_flag == ITEM_WORN)
            if (ch->pmote_str)
            {
                if (*(ch->pmote_str) == '\'')
                    sprintf (buf, "A %s, %s %sperson%s", phrase,
                             obj->desc_keys, tname, ch->pmote_str);
                else
                    sprintf (buf, "A %s, %s %sperson %s", phrase,
                             obj->desc_keys, tname, ch->pmote_str);
            }
            else
                sprintf (buf, "A %s, %s %sperson is here.", phrase,
                         obj->desc_keys, tname);
        else
        {
            if (ch->pmote_str)
            {
                if (*(ch->pmote_str) == '\'')
                    sprintf (buf, "A %s, %s %sperson%s", phrase,
                             obj->desc_keys, tname, ch->pmote_str);
                else
                    sprintf (buf, "A %s, %s %sperson %s", phrase,
                             obj->desc_keys, tname, ch->pmote_str);
            }
            else
                sprintf (buf, "A %s, %s %sperson is here.", phrase,
                         obj->desc_keys, tname);
        }
        return buf;
    }

    if (ch->pmote_str)
    {
        if (*(ch->pmote_str) == '\'')
            sprintf (buf, "%s%s", ch->short_descr, ch->pmote_str);
        else
            sprintf (buf, "%s %s", ch->short_descr, ch->pmote_str);
        buf[0] = (('a' <= buf[0]) && (buf[0] <= 'z'))
                 ? (char) (buf[0] - 32) : buf[0];
        buf[0] = toupper (buf[0]);
        return buf;
    }
    else
        return ch->long_descr;
}

char *
left_right_grip (CHAR_DATA * ch)
{
    bool right = false, left = false;

    if (get_equip (ch, WEAR_PRIM))
        right = true;
    if (get_equip (ch, WEAR_SEC))
        left = true;
    if (get_equip (ch, WEAR_SHIELD))
        left = true;
    if (get_equip (ch, WEAR_BOTH))
    {
        right = true;
        left = true;
    }
    if (get_equip (ch, WEAR_CARRY_L))
        left = true;
    if (get_equip (ch, WEAR_CARRY_R))
        right = true;

    if ((!right && left) || (!right && !left))
        return "right";
    if (right && !left)
        return "left";
    else
        system_log ("Error in left_right_grip, act.informative.c", true);

    return NULL;
}

char *
worn_third_loc (CHAR_DATA * ch, OBJ_DATA * obj)
{
    static char buf[MAX_STRING_LENGTH] = { '\0' };

    if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
        sprintf (buf, "around %s neck", HSHR (ch));
    else if (obj->location == WEAR_BODY)
        sprintf (buf, "on %s body", HSHR (ch));
    else if (obj->location == WEAR_HEAD)
        sprintf (buf, "on %s head", HSHR (ch));
    else if (obj->location == WEAR_ARMS)
        sprintf (buf, "on %s arms", HSHR (ch));
    else if (obj->location == WEAR_OVER)
        sprintf (buf, "over %s body", HSHR (ch));
    else if (obj->location == WEAR_ABOUT)
        sprintf (buf, "about %s body", HSHR (ch));
    else if (obj->location == WEAR_WAIST)
        sprintf (buf, "around %s waist", HSHR (ch));
    else if (obj->location == WEAR_WRIST_L)
        sprintf (buf, "on %s left wrist", HSHR (ch));
    else if (obj->location == WEAR_WRIST_R)
        sprintf (buf, "on %s right wrist", HSHR (ch));
    else if (obj->location == WEAR_HAIR)
        sprintf (buf, "in %s hair", HSHR (ch));
    else if (obj->location == WEAR_FACE)
        sprintf (buf, "on %s face", HSHR (ch));
    else if (obj->location == WEAR_EYES)
        sprintf (buf, "over %s eyes", HSHR (ch));
    else if (obj->location == WEAR_ANKLE_L)
        sprintf (buf, "on %s left ankle", HSHR (ch));
    else if (obj->location == WEAR_ANKLE_R)
        sprintf (buf, "on %s right ankle", HSHR (ch));
    else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
        sprintf (buf, "on %s belt", HSHR (ch));
    else if (obj->location == WEAR_BACK)
        sprintf (buf, "on %s back", HSHR (ch));
    else if (obj->location == WEAR_THROAT)
        sprintf (buf, "around %s throat", HSHR (ch));
    else if (obj->location == WEAR_BLINDFOLD)
        sprintf (buf, "as a blindfold");
    else if (obj->location == WEAR_EAR)
        sprintf (buf, "on %s ear", HSHR (ch));
    else if (obj->location == WEAR_SHOULDER_R)
        sprintf (buf, "over %s right shoulder", HSHR (ch));
    else if (obj->location == WEAR_SHOULDER_L)
        sprintf (buf, "over %s left shoulder", HSHR (ch));
    else if (obj->location == WEAR_FEET)
        sprintf (buf, "on %s feet", HSHR (ch));
    else if (obj->location == WEAR_FINGER_R)
        sprintf (buf, "on %s right ring finger", HSHR (ch));
    else if (obj->location == WEAR_FINGER_L)
        sprintf (buf, "on %s left ring finger", HSHR (ch));
    else if (obj->location == WEAR_ARMBAND_R)
        sprintf (buf, "around %s upper right arm.", HSHR (ch));
    else if (obj->location == WEAR_ARMBAND_L)
        sprintf (buf, "around %s upper left arm.", HSHR (ch));
    else if (obj->location == WEAR_LEGS)
        sprintf (buf, "on %s legs", HSHR (ch));
    else if (obj->location == WEAR_HANDS)
        sprintf (buf, "on %s hands", HSHR (ch));
    else if (obj->location == WEAR_UNDERWEAR)
        sprintf (buf, "around %s hips", HSHR (ch));
    else if (obj->location == WEAR_OVERWEAR)
        sprintf (buf, "around %s chest", HSHR (ch));
    else
        sprintf (buf, "#1in an uknown location#0");

    return buf;
}

void
show_char_to_char (CHAR_DATA * i, CHAR_DATA * ch, int mode)
{
    int percent = 0;
    int curdamage = 0;
    int location = 0;
    int x = 0;
    int j = 0;
    OBJ_DATA *eq = NULL;
    OBJ_DATA *blindfold = NULL;
    OBJ_DATA *tobj = NULL;
    AFFECTED_TYPE *af = NULL;
    ROOM_DATA *troom = NULL;
    WOUND_DATA *wound = NULL;
    ENCHANTMENT_DATA *enchantment = NULL;
    char *p = '\0';
    char stink_buf[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    char *dir_names[] =
    {
        "to the north", "to the east", "to the south",
        "to the west", "above", "below"
    };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char buf3[MAX_STRING_LENGTH] = { '\0' };
    char buf4[MAX_STRING_LENGTH] = { '\0' };
    char temp[MAX_STRING_LENGTH];
    char wear[MAX_STRING_LENGTH];
    bool found = false;
    char *group = group_makeup(i);

    if (mode == 4 || mode == 7 || mode ==  10 || mode == 8 || mode == 9 || mode == 11 || mode == 12)
        mode = 0;

    if (!mode)
    {

        if (IS_SUBDUEE (i))
        {
            if (i->subdue == ch)
                act ("You have #5$N#0 in tow.", false, ch, 0, i, TO_CHAR);
            return;
        }

        if (!CAN_SEE (ch, i))
        {
            if (!GET_FLAG (i, FLAG_WIZINVIS) &&
                    get_affect (ch, MAGIC_AFFECT_SENSE_LIFE))
                send_to_char ("\nYou sense a hidden life form in the room.\n\n",
                              ch);
            return;
        }

        if ((blindfold = get_equip (i, WEAR_BLINDFOLD)) ||
                !char_long (i, 0) || GET_POS (i) != i->default_pos ||
                IS_SWIMMING (i))
        {
            if (char_short (i))
            {
                strcpy (buffer, "#5");
                strcat (buffer, char_short (i));
                strcat (buffer, "#0");
                buffer[2] = toupper (buffer[2]);

            }
            else
                strcpy (buffer, "#5A nameless one#0");

            if (IS_SWIMMING (i) || IS_FLOATING (i))
            {
                if ( IS_SET(i->act, ACT_FLYING ))

                    strcat(buffer, " is here, flying.");
                
                else
                    if (IS_SWIMMING(i))
                    strcat (buffer, " is here, swimming.");
                    else
                    strcat (buffer, " is here, floating.");
            }
            else
                switch (GET_POS (i))
                {
                case POSITION_STUNNED:
                    if (blindfold)
                        strcat (buffer, " is here, stunned and blindfolded.");
                    else
                        strcat (buffer, " is here, stunned.");
                    break;
                case POSITION_UNCONSCIOUS:
                    if (blindfold)
                        strcat (buffer,
                                " is lying here, unconscious and blindfolded.");
                    else
                        strcat (buffer, " is lying here, unconscious.");
                    break;
                case POSITION_MORTALLYW:
                    if (blindfold)
                        strcat (buffer,
                                " is lying here, mortally wounded and blindfolded.");
                    else
                        strcat (buffer, " is lying here, mortally wounded.");
                    break;
                case POSITION_DEAD:
                    strcat (buffer, " is lying here, dead.");
                    break;
                case POSITION_STANDING:
                    if (blindfold)
                        strcat (buffer, " is standing here, blindfolded.");
                    else
                        strcat (buffer, " is standing here.");
                    break;
                case POSITION_SITTING:
                    if (blindfold)
                        strcat (buffer, " is sitting here, blindfolded");

                    if (i->pmote_str)
                    {
                        sprintf (buffer, "#5");
                        strcat (buffer, char_long (i, GET_TRUST (ch)));
                        strcat (buffer, "#0");
                    }
                    else
                        strcat (buffer, " is sitting here");

                    if ((x = under_cover(i)))
                    {
                        *buf3 = '\0';
                        j = 0;
                        for (af = i->hour_affects; af; af = af->next)
                        {
                            if (af->type == AFFECT_COVER)
                            {
                                j++;

                                if (j == 1)
                                    sprintf(buf3, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                                else if (j == x)
                                    sprintf(buf3 + strlen(buf3), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                                else
                                    sprintf(buf3 + strlen(buf3), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                            }
                        }

                        if ((af = get_affect (i, AFFECT_COVER)) && is_obj_in_list (af->a.table.obj, i->room->contents))
                            sprintf (buf4, "taking cover from the%s behind #2%s#0.", buf3, obj_short_desc(af->a.table.obj));
                        else
                            sprintf (buf4, "taking cover from the%s.", buf3);
                        //*buf4 = toupper(*buf4);
                        strcat (buffer, ", ");
                        strcat (buffer, buf4);
                    }
                    else if ((af = get_affect (i, MAGIC_SIT_TABLE)) && is_obj_in_list (af->a.table.obj, i->room->contents))
                    {
                        tobj = af->a.table.obj;
                        sprintf (buffer + strlen (buffer), " at #2%s#0.",
                                 OBJS (tobj, i));
                    }
                    else
                        strcat (buffer, ".");
                    break;
                case POSITION_RESTING:
                    if (blindfold)
                        strcat (buffer, " is resting here, blindfolded");
                    else
                    {
                        if (i->pmote_str)
                        {
                            sprintf (buffer, "#5");
                            strcat (buffer, char_long (i, GET_TRUST (ch)));
                            strcat (buffer, "#0");
                            break;
                        }
                        else
                            strcat (buffer, " is resting here");
                    }

                    if ((x = under_cover(i)))
                    {
                        *buf3 = '\0';
                        j = 0;
                        for (af = i->hour_affects; af; af = af->next)
                        {
                            if (af->type == AFFECT_COVER)
                            {
                                j++;

                                if (j == 1)
                                    sprintf(buf3, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                                else if (j == x)
                                    sprintf(buf3 + strlen(buf3), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                                else
                                    sprintf(buf3 + strlen(buf3), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                            }
                        }

                        if ((af = get_affect (i, AFFECT_COVER)) && is_obj_in_list (af->a.table.obj, i->room->contents))
                            sprintf (buf4, "taking cover from the%s behind #2%s#0.", buf3, obj_short_desc(af->a.table.obj));
                        else
                            sprintf (buf4, "taking cover from the%s.", buf3);
                        //*buf4 = toupper(*buf4);
                        strcat (buffer, ", ");
                        strcat (buffer, buf4);
                    }
                    else if ((af = get_affect (i, MAGIC_SIT_TABLE)) &&
                             is_obj_in_list (af->a.table.obj, i->room->contents))
                    {
                        tobj = af->a.table.obj;
                        sprintf (buffer + strlen (buffer), " at #2%s#0.",
                                 OBJS (tobj, i));
                    }
                    else
                        strcat (buffer, ".");
                    break;
                case POSITION_SLEEPING:
                    if (blindfold)
                        strcat (buffer, " is sleeping here, blindfolded");
                    else
                        strcat (buffer, " is sleeping here");

                    if ((af = get_affect (i, MAGIC_SIT_TABLE)) &&
                            is_obj_in_list ((OBJ_DATA *) af->a.spell.t,
                                            i->room->contents))
                    {
                        tobj = (OBJ_DATA *) af->a.spell.t;
                        sprintf (buffer + strlen (buffer), " at #2%s#0.",
                                 OBJS (tobj, i));
                    }
                    else
                        strcat (buffer, ".");

                    break;
                case POSITION_FIGHTING:
                    if (i->fighting)
                    {

                        if (IS_SET (i->flags, FLAG_SUBDUING))
                        {
                            if (blindfold)
                                strcat (buffer,
                                        " is here, #1wrestling blindfolded#0 against ");
                            else
                                strcat (buffer, " is here, #1wrestling#0 ");
                        }
                        else
                        {
                            if (blindfold)
                                strcat (buffer,
                                        " is here, #1fighting blindfolded#0 against ");
                            else
                                strcat (buffer, " is here, #1fighting#0 ");
                        }

                        if (i->fighting == ch)
                            strcat (buffer, " you!");
                        else
                        {
                            if (i->in_room == i->fighting->in_room)
                            {
                                strcat (buffer, char_short (i->fighting));
                                strcat (buffer, ".");
                            }
                            else
                                strcat (buffer, "someone who has already left.");
                        }
                    }
                    else		/* NIL fighting pointer */
                        strcat (buffer, " is here struggling with thin air.");
                    break;
                default:
                    strcat (buffer, " is floating here.");
                    break;
                }

            if (do_group_size(i) > 9 && !i->following)
            {
                sprintf(buffer + strlen(buffer), " #5A %s#0 stands behind %s.",
                        group, sex_noun[(int) i->sex]);
            }

            if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
                strcat (buffer, " #2(new player)#0");

            if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
                strcat (buffer, " #B(new player guide)#0");

            if (i->descr() && i->descr()->idle)
                strcat (buffer, " #1(idle)#0");

            if (get_affect (i, MAGIC_HIDDEN))
                strcat (buffer, " #1(hidden)#0");

            if (are_grouped (i, ch))
                strcat (buffer, " #6(grouped)#0");

            if (GET_FLAG (i, FLAG_WIZINVIS))
                strcat (buffer, " #C(wizinvis)#0");

            if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
                strcat (buffer, " #1(invisible)#0");

            if (get_affect (i, MAGIC_AFFECT_CONCEALMENT) &&
                    (i == ch || GET_TRUST (i) ||
                     get_affect (ch, MAGIC_AFFECT_SENSE_LIFE)))
                strcat (buffer, " #1(blend)#0");

            if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
                strcat (buffer, " #1(editing)#0");

            if (!IS_NPC (i) && !i->descr() && !i->pc->admin_loaded)
                strcat (buffer, " #1(link dead)#0");

            if (!IS_MORTAL (ch) && i->pc && !i->descr() && i->pc->admin_loaded)
                strcat (buffer, " #3(loaded)#0");

            if (i->descr() && i->descr()->original && !IS_MORTAL (ch))
                strcat (buffer, " #2(animated)#0");

            strcat (buffer, "#0\n");
            reformat_string (buffer, &p);
            if (GET_POS(i) == POSITION_FIGHTING || !(do_group_size(i) > 9 && i->following))
                send_to_char (p, ch);
            mem_free (p); // char*
        }

        else
        {
            /* npc with long */
            if (IS_SUBDUER (i))
            {
                if (IS_RIDER (i))
                    sprintf (buffer,
                             "#5%s#0, mounted on #5%s#0, has #5%s#0 in tow.",
                             char_short (i), char_short (i->mount),
                             i->subdue == ch ? "you" : char_short (i->subdue));
                else
                    sprintf (buffer, "#5%s#0 has #5%s#0 in tow.",
                             char_short (i),
                             i->subdue == ch ? "you" : char_short (i->subdue));
                buffer[2] = toupper (buffer[2]);

                if (do_group_size(i) > 9 && !i->following)
                {
                    sprintf(buffer + strlen(buffer), " #5A %s#0 stands behind %s.",
                            group, sex_noun[(int) i->sex]);
                }

                act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            }

            else if (IS_RIDER (i) && !IS_SUBDUER (i))
            {

                sprintf (buffer, "%s#0 %s %s#5%s#0.",
                         char_short (i), AWAKE (i) ?
                         "sits atop" : "is asleep upon", AWAKE (i->mount) ?
                         "" : "a sleeping mount, ", char_short (i->mount));

                *buffer = toupper (*buffer);
                sprintf (buf2, "#5%s", buffer);
                sprintf (buffer, "%s", buf2);

                if (do_group_size(i) > 9 && !i->following)
                {
                    sprintf(buffer + strlen(buffer), " #5A %s#0 stands behind %s.",
                            group, sex_noun[(int) i->sex]);
                }

                if (!(do_group_size(i) > 9 && i->following))
                    act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            }

            else if (IS_RIDEE (i))
            {

                if (ch == i->mount)
                {
                    sprintf (buffer, "You sit upon %s#5%s#0.\n", AWAKE (i) ?
                             "" : "a sleeping mount, ", char_short (i));
                    send_to_char (buffer, ch);
                }
            }

            else if (IS_HITCHEE (i))
            {

                if (i == ch)
                    sprintf (buffer, "#5You#0 ride upon #5%s#0.\n",
                             char_short (i));
                else
                    sprintf (buffer, "#5%s#0 is here, hitched to #5%s#0.\n",
                             char_short (i),
                             i->hitcher == ch ? "you" : char_short (i->hitcher));

                buffer[2] = toupper (buffer[2]);
                send_to_char (buffer, ch);
            }

            else
            {
                *buffer = '\0';


                if ((GET_FLAG (i, FLAG_ENTERING) ||
                        GET_FLAG (i, FLAG_LEAVING)) && enter_exit_msg (i, buffer))
                {
                    if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
                        strcat (buffer, " #2(new player)#0");

                    if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
                        strcat (buffer, " #B(new player guide)#0");

                    if (i->descr() && !IS_NPC (i) && i->descr()->idle)
                        strcat (buffer, "#1(idle)#0 ");

                    if (GET_FLAG (i, FLAG_WIZINVIS))
                        strcat (buffer, " #C(wizinvis)#0");

                    if (get_affect (i, MAGIC_HIDDEN))
                        strcat (buffer, " #1(hidden)#0");

                    if (are_grouped (i, ch))
                        strcat (buffer, " #6(grouped)#0");

                    if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
                        strcat (buffer, " #1(invisible)#0");

                    if (get_affect (i, MAGIC_AFFECT_CONCEALMENT))
                        strcat (buffer, " #1(blend)#0");

                    if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
                        strcat (buffer, " #1(editing)#0");

                    if (i->descr() && i->descr()->original && !IS_MORTAL (ch))
                        strcat (buffer, " #2(animated)#0");

                    if (!IS_NPC (i) && !i->descr() && !i->pc->admin_loaded)
                        strcat (buffer, " #1(link dead)#0");

                    if (!IS_MORTAL (ch) && !IS_NPC (i) && !i->descr()
                            && i->pc->admin_loaded)
                        strcat (buffer, " #3(loaded)#0");

                    strcat (buffer, "#0\n");
                    reformat_string (buffer, &p);

                    if (!((do_group_size(i) > 5 || get_second_affect(i, SA_GFOLLOW, NULL))&& i->following))
                        send_to_char (p, ch);

                    mem_free (p); // char*
                }
                else if ((af = get_affect (i, AFFECT_SHADOW)) &&
                         af->a.shadow.edge != -1)
                {
                    sprintf (buf, "%s", char_short (i));
                    *buf = toupper (*buf);
                    sprintf (buffer + strlen (buffer),
                             "#5%s is here, #0standing %s.", buf,
                             dir_names[af->a.shadow.edge]);
                    act (buffer, false, ch, 0, i, TO_CHAR | _ACT_FORMAT);
                }
                else if ((af = get_affect (i, AFFECT_GUARD_DIR)) &&
                         af->a.shadow.edge != -1)
                {
                    sprintf (buf, "%s", char_short (i));
                    *buf = toupper (*buf);
                    sprintf (buffer + strlen (buffer),
                             "#5%s#0 is here, guarding the %s exit.", buf,
                             dirs[af->a.shadow.edge]);
                    act (buffer, false, ch, 0, i, TO_CHAR | _ACT_FORMAT);
                }
                else
                {
                    sprintf (buffer, "#5");
                    sprintf (temp, char_long (i, GET_TRUST (ch)));


                    if (do_group_size(i) > 9 && !i->following)
                    {
                        sprintf(buffer + strlen(buffer), "With #5a %s#0 behind %s, #5", group, sex_noun[(int) i->sex]);
                        temp[0] = tolower (temp[0]);
                        temp[1] = tolower (temp[1]);
                        temp[2] = tolower (temp[2]);
                    }

                    strcat (buffer, temp);

                    if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
                        strcat (buffer, " #2(new player)#0");

                    if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
                        strcat (buffer, " #B(new player guide)#0");

                    if (get_affect (i, MAGIC_HIDDEN))
                        strcat (buffer, " #1(hidden)#0");

                    if (are_grouped (i, ch))
                        strcat (buffer, " #6(grouped)#0");

                    if (i->descr() && i->descr()->idle)
                        strcat (buffer, " #1(idle)#0");

                    if (GET_FLAG (i, FLAG_WIZINVIS))
                        strcat (buffer, " #C(wizinvis)#0");

                    if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
                        strcat (buffer, " #1(invisible)#0");

                    if (get_affect (i, MAGIC_AFFECT_CONCEALMENT))
                        strcat (buffer, " #1(blend)#0");

                    if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
                        strcat (buffer, " #1(editing)#0");

                    if (i->descr() && i->descr()->original && !IS_MORTAL (ch))
                        strcat (buffer, " #2(animated)#0");

                    if (!IS_NPC (i) && !i->descr() && !i->pc->admin_loaded)
                        strcat (buffer, " #1(link dead)#0");

                    if (!IS_MORTAL (ch) && !IS_NPC (i) && !i->descr()
                            && i->pc->admin_loaded)
                        strcat (buffer, " #3(loaded)#0");

                    strcat (buffer, "#0\n");
                    reformat_string (buffer, &p);

                    if (!(do_group_size(i) > 9 && i->following))
                        send_to_char (p, ch);
                    mem_free (p); //char*
                }
            }
        }

        if ((af = get_affect (i, MAGIC_TOLL)) &&
                af->a.toll.room_num == i->in_room)
        {
            sprintf (buffer, "$n #Dis collecting tolls from people leaving "
                     "%s.#0", dirs[af->a.toll.dir]);
            act (buffer, true, i, 0, ch, TO_VICT);
        }

    }
    else if (mode == 1 || mode == 15)
    {

        if (i->description)
        {

            if (((eq = get_equip (i, WEAR_HEAD))
                    && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
                    || ((eq = get_equip (i, WEAR_FACE))
                        && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
                    || ((eq = get_equip (i, WEAR_NECK_1))
                        && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
                        && IS_SET (i->affected_by, AFF_HOODED))
                    || ((eq = get_equip (i, WEAR_NECK_2))
                        && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
                        && IS_SET (i->affected_by, AFF_HOODED))
                    || ((eq = get_equip (i, WEAR_OVER))
                        && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
                        && IS_SET (i->affected_by, AFF_HOODED))
                    || ((eq = get_equip (i, WEAR_ABOUT))
                        && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
                        && IS_SET (i->affected_by, AFF_HOODED)))
            {
                send_to_char ("This person's features are not visible.\n", ch);
            }
            else if (mode == 1)
            {
                send_to_char (i->description, ch);
            }
        }
        else
        {
            act ("You see nothing special about $m.", false, i, 0, ch, TO_VICT);
        }

        /* Show name (first keyword) if mobile is owned by the character examining the mobile*/
        if (mode == 15)
        {
            if (IS_NPC(i))
            {
                if (i->mob->owner)
                {
                    if (!strcmp(i->mob->owner, ch->tname))
                    {
                        sprintf (buffer, "\nYou recognise $n to be called "
                                 "%s.", GET_NAME (i));
                        act (buffer, false, i, 0, ch, TO_VICT);
                    }
                }
            }
        }

        if (i->scent)
        {
            int scent_count = 0;
            SCENT_DATA *scent = NULL;
            SCENT_DATA *scent_one = NULL;
            SCENT_DATA *scent_two = NULL;
            SCENT_DATA *scent_three = NULL;

            for (scent = i->scent; scent; scent = scent->next)
            {
                // Require a scent strength of two or more to smell it.
                if ((ch == i && scent_strength(scent)) || scent_strength(scent) >= 2)
                {
                    if (!scent_one)
                    {
                        scent_one = scent;
                        scent_count++;
                    }
                    else if (!scent_two)
                    {
                        scent_two = scent;
                        scent_count++;
                    }
                    else if (!scent_three)
                    {
                        scent_three = scent;
                        scent_count++;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_one))
                    {
                        scent_one = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_two = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_three = scent;
                    }
                    else
                        continue;
                }
            }

            if (scent_count == 3)
            {
                sprintf(stink_buf, "You detect %s of %s, %s of %s and %s of %s about %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref), HMHR(i));
            }
            else if (scent_count == 2)
            {
                sprintf(stink_buf, "You detect %s of %s and %s of %s about %s. ",  scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), HMHR(i));
            }
            else if (scent_count == 1)
            {
                sprintf(stink_buf, "You detect %s of %s about %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), HMHR(i));
            }
        }

        /* show dmote */
        /* Description is formated to 65 char, so we need to use reformat_desc instead of _ACT_FORMAT*/
        if (i->dmote_str && *i->dmote_str)
        {
            reformat_desc (i->dmote_str, &i->dmote_str);
             
            if (*stink_buf)
            {           
                sprintf(buffer, "    %s", stink_buf);
                reformat_string (buffer, &p);
                sprintf(buf, "\n%s", p);
                send_to_char (buf, ch);
            }
            
            sprintf(buffer, "    %s", i->dmote_str);
            reformat_string (buffer, &p);
            sprintf(buf, "\n%s", p);
            send_to_char (buf, ch);
 
            mem_free (p); //char*
            p = NULL;
        }
        else if (*stink_buf)
        {
            sprintf(buffer, "    %s", stink_buf);
            reformat_string (buffer, &p);
            sprintf(buf, "\n%s", p);
            send_to_char (buf, ch);
            mem_free (p); //char*
            p = NULL;
        }

        /* Show a character to another */

        if (GET_MAX_HIT (i) > 0)
            percent = (100 * GET_HIT (i)) / GET_MAX_HIT (i);
        else
            percent = -1;		/* How could MAX_HIT be < 1?? */

        strcpy (buffer, char_short (i));
        *buffer = toupper (*buffer);

        if (IS_SET (i->act, ACT_VEHICLE))
        {
            if (percent >= 100)
                strcat (buffer, " is in pristine condition.\n");
            else if (percent >= 90)
                strcat (buffer, " is slightly damaged.\n");
            else if (percent >= 70)
                strcat (buffer, " is damaged, but still functional.\n");
            else if (percent >= 50)
                strcat (buffer, " is badly damaged.\n");
            else if (percent >= 25)
                strcat (buffer, " is barely functional.\n");
            else if (percent >= 10)
                strcat (buffer, " has sustained a great deal of damage!\n");
            else if (percent >= 0)
                strcat (buffer, " is about ready to collapse inward!\n");
            send_to_char ("\n", ch);
            send_to_char (buffer, ch);
        }

        *buf2 = '\0';
        *buf3 = '\0';
        strcpy (buf2, display_clan_ranks (i, ch));
        if (mode == 15 && i != ch && *buf2 && !is_hooded (i))
        {
            send_to_char ("\n", ch);
            *buf2 = toupper (*buf2);
            reformat_string (buf2, &p);
            send_to_char (p, ch);
            mem_free (p); //char*
            p = NULL;
        }

        if (mode == 15 && *buf2 && (is_leader (ch, i) || i->following == ch) && IS_SET (i->act, ACT_AGGRESSIVE) && IS_NPC(i))
        {
            sprintf(buf2, "%s is currently aggressive.", HSSH (ch));
            send_to_char ("\n", ch);
            *buf2 = toupper (*buf2);
            reformat_string (buf2, &p);
            send_to_char (p, ch);
            mem_free (p); //char*
            p = NULL;
        }

        if (mode == 15 && *buf2 && (is_leader (ch, i) || i->following == ch) && !IS_SET (i->act, ACT_SENTINEL) && IS_NPC(i))
        {
            sprintf(buf2, "%s is currently a roamer.", HSSH (ch));
            send_to_char ("\n", ch);
            *buf2 = toupper (*buf2);
            reformat_string (buf2, &p);
            send_to_char (p, ch);
            mem_free (p); //char*
            p = NULL;
        }

        /*
           if ( (mode == 1 || mode == 15) && !IS_NPC(i) ) {
           send_to_char ("\n", ch);
           sprintf (buffer, "%s is approximately %d inches in height, and appears to be of %s build.", HSSH(i), i->height, frames[i->frame]);
           buffer[0] = toupper(buffer[0]);
           reformat_string (buffer, &p);
           send_to_char (p, ch);
           mem_free (p); // char*
           p = NULL;
           }
         */

		if (mode == 15 && !str_cmp(lookup_race_variable(i->race, RACE_NAME), "robot") && i->morph_time)
		{
			int delta, hours, minutes;
			delta = i->morph_time - time (0);
			minutes = delta / 60;
			hours = minutes / 15;

			if (hours > 1)
			{
				sprintf (buf, "The display on this robot indicates it has about %d hours of power left.\n", hours);
			}
			else
			{
				sprintf (buf, "The display on this robot indicates it has less than an hour of power left.\n");
			}

			send_to_char (buf, ch);
		}

        if (mode == 15 && i->damage && !IS_SET (i->act, ACT_VEHICLE)
                && !is_hooded (i))
        {
            curdamage = i->damage;
            if (curdamage > 0 && curdamage <= i->max_hit * .25)
                sprintf (buffer, "%s face looks slightly pale",
                         char_short (i));
            else if (curdamage > i->max_hit * .25
                     && curdamage < i->max_hit * .50)
                sprintf (buffer, "%s face looks rather pallid",
                         char_short (i));
            else if (curdamage > i->max_hit * .50
                     && curdamage < i->max_hit * .75)
                sprintf (buffer, "%s face looks quite ashen", char_short (i));
            else if (curdamage > i->max_hit * .75)
                sprintf (buffer, "%s face looks deathly pale", char_short (i));

            if (i->move * 1.0 / i->max_move *1.0 <= 0.6667)
            {
                if (*buffer)
                    sprintf (buffer + strlen(buffer), ", ");
                else
                    sprintf (buffer, "%s ", HSHR(i));

                sprintf (buffer + strlen(buffer), "looks a little fatigued");
            }
            else if (i->move * 1.0 / i->max_move *1.0 <= 0.1667)
            {
                if (*buffer)
                    sprintf (buffer + strlen(buffer), ", ");
                else
                    sprintf (buffer, "%s ", HSHR(i));

                sprintf (buffer + strlen(buffer), "looks somewhat exhausted");
            }

            if (i->shock * 1.0 / i->max_shock * 1.0 <= 0.2000)
            {
                if (*buffer)
                    sprintf (buffer + strlen(buffer), ", ");
                else
                    sprintf (buffer, "%s ", HSHR(i));

                sprintf (buffer + strlen(buffer), "appears badly traumatised");
            }


            if (get_affect(i, AFFECT_INTERNAL) && ch->skills[SKILL_FIRSTAID] >= 10)
            {
                sprintf (buffer + strlen(buffer), " and %s is suffering from internal injuries", HSSH(i));
            }

            *buffer = toupper (*buffer);
            strcat(buffer, ".");
            reformat_string (buffer, &p);
            send_to_char (p, ch);
            mem_free (p); // char*
            p = NULL;
            send_to_char ("\n", ch);
        }
        if (mode == 1)
        {
            curdamage = 0;
            for (wound = i->wounds; wound; wound = wound->next)
                curdamage += wound->damage;
            curdamage += i->damage;

            if (curdamage <= 0)
                sprintf (buffer, "%s appears to be in excellent condition.\n",
                         char_short (i));
            else if (curdamage <= i->max_hit * .1667)
                sprintf (buffer,
                         "%s appears to be slightly the worse for wear.\n",
                         char_short (i));
            else if (curdamage > i->max_hit * .1667
                     && curdamage <= i->max_hit * .3333)
                sprintf (buffer, "%s appears injured.\n", char_short (i));
            else if (curdamage > i->max_hit * .3333
                     && curdamage <= i->max_hit * .6667)
                sprintf (buffer, "%s appears moderately injured.\n",
                         char_short (i));
            else if (curdamage > i->max_hit * .6667
                     && curdamage <= i->max_hit * .8335)
                sprintf (buffer, "%s appears severely injured.\n",
                         char_short (i));
            else if (curdamage >= i->max_hit * .8335)
                sprintf (buffer, "%s appears close to death.\n", char_short (i));
            send_to_char ("\n", ch);
            *buffer = toupper (*buffer);
            reformat_string (buffer, &p);
            send_to_char (p, ch);
            mem_free (p); // char*
            p = NULL;
        }

        if (mode == 15 && !IS_SET (i->act, ACT_VEHICLE)
                && (i->wounds || i->lodged))
        {
            sprintf (buf2, "%s", show_wounds (i, 0));
            if (ch->fighting || i->fighting)
                sprintf (buf2, "%s", strip_small_minor(buf2, ch));

            send_to_char ("\n", ch);
            strcat (buf3, buf2);
            act (buf3, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            *buf3 = '\0';
        }

        if (mode == 15 && (!i->damage || is_hooded (i))
                && !IS_SET (i->act, ACT_VEHICLE) && !i->wounds && !i->lodged)
        {
            send_to_char ("\n", ch);
            sprintf (buf2, "%s appears to be in excellent condition.",
                     char_short (i));
            *buf2 = toupper (*buf2);
            reformat_string (buf2, &p);
            send_to_char (p, ch);
            mem_free (p); //char*
            p = NULL;
        }

        if (i->mob &&
                i->mob->vehicle_type == VEHICLE_HITCH &&
                (troom = vnum_to_room (i->mob->vnum)) &&
                (troom->people || troom->contents) && i->room != troom)
        {
            sprintf (buf, "\nOn board, you see:\n");
            send_to_char (buf, ch);
            if (troom->people)
                list_char_to_char (troom->people, ch, 0);
            if (troom->contents)
                list_obj_to_char (troom->contents, ch, 0, true);
        }

        for (enchantment = i->enchantments; enchantment;
                enchantment = enchantment->next)
        {
            if (*show_enchantment (enchantment))
                sprintf (buf3 + strlen (buf3), "%s",
                         show_enchantment (enchantment));
        }

        if (*buf3)
        {
            act (buf3, false, ch, 0, 0, TO_CHAR);
        }

        *buf3 = '\0';
        if (i == ch)
        {
            send_to_char ("\n", i);
            do_equipment (i, "", 0);
        }
        else
        {
            if (i->equip || i->right_hand || i->left_hand)
                send_to_char ("\n", ch);
            if (i->right_hand)
            {
                if (!IS_SET (i->act, ACT_MOUNT))
                {
                    if (i->right_hand->location == WEAR_PRIM
                            || i->right_hand->location == WEAR_SEC)
                        sprintf (buf, "<wielded in right hand>  ");
                    else if (i->right_hand->location == WEAR_BOTH)
                        sprintf (buf, "<wielded in both hands>  ");
                    else if (i->right_hand->location == WEAR_SHIELD)
                        sprintf (buf, "<gripped in right hand>  ");
                    else
                        sprintf (buf, "<carried in right hand>  ");
                }
                else
                    sprintf (buf, "<carried on back>        ");
                send_to_char (buf, ch);
                show_obj_to_char (i->right_hand, ch, 1);
            }
            if (i->left_hand)
            {
                if (!IS_SET (i->act, ACT_MOUNT))
                {
                    if (i->left_hand->location == WEAR_PRIM
                            || i->left_hand->location == WEAR_SEC)
                        sprintf (buf, "<wielded in left hand>   ");
                    else if (i->left_hand->location == WEAR_BOTH)
                        sprintf (buf, "<wielded in both hands>  ");
                    else if (i->left_hand->location == WEAR_SHIELD)
                        sprintf (buf, "<gripped in left hand>   ");
                    else
                        sprintf (buf, "<carried in left hand>   ");
                }
                else
                    sprintf (buf, "<carried on back>        ");

                send_to_char (buf, ch);
                show_obj_to_char (i->left_hand, ch, 1);
            }

            if (i->equip && (i->left_hand || i->right_hand))
                send_to_char ("\n", ch);

            for (location = 0; location < MAX_WEAR; location++)
            {

                if (!(eq = get_equip (i, loc_order[location])))
                    continue;

                if (eq == i->right_hand || eq == i->left_hand)
                    continue;

                if (eq->attire)
                    sprintf(wear, "<%s>%*s", eq->attire, 23-strlen(eq->attire), "");
                else
                    sprintf(wear, "%s", where[loc_order[location]]);
                    
                send_to_char (wear, ch);
                
                if ((location == WEAR_BLINDFOLD || GET_TRUST (ch) >= 4) || (IS_OBJ_VIS (ch, eq) && 
                    !IS_SET (eq->obj_flags.extra_flags2, ITEM_CONCEALED)))
                {
                    show_obj_to_char (eq, ch, 1);  
                }
                else if (!IS_SET (eq->obj_flags.extra_flags2, ITEM_CONCEALED))
                    send_to_char ("#2something#0\n", ch);
                else if (eq->obj_flags.weight + eq->contained_wt > 4)
                    send_to_char ("#2something#0 #1(concealed)#0\n", ch); 

                found = true;
            }
        }
    }

    else if (mode == 3)
    {
        if (CAN_SEE (ch, i))
        {
            send_to_char ("   ", ch);
            act ("$N", false, ch, 0, i, TO_CHAR);
        }
    }
}

void
do_search (CHAR_DATA * ch, char *argument, int cmd)
{
    int dir = 0;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    /*
    Defunct, as search is now based on stats.

      if (!real_skill (ch, SKILL_SEARCH))
        {
          send_to_char ("You'll need to learn some search skills first.\n", ch);
          return;
        } */

    argument = one_argument (argument, buf);

    if (!*buf)
        dir = -1;
    else
    {
        switch (*buf)
        {
        case 'n':
            dir = 0;
            break;
        case 'e':
            dir = 1;
            break;
        case 's':
            dir = 2;
            break;
        case 'w':
            dir = 3;
            break;
        case 'u':
            dir = 4;
            break;
        case 'd':
            dir = 5;
            break;
        default:
            send_to_char ("That's not a valid direction.\n", ch);
            return;
        }
    }

    act ("$n carefully searches the area.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
    act ("You search carefully...", false, ch, 0, 0, TO_CHAR);

    ch->delay_type = DEL_SEARCH;
    ch->delay = 3 + number (0, 2);
    ch->delay_info1 = dir;
}

void
delayed_search (CHAR_DATA * ch)
{
    int dir = 0;
    float search = 0;
    int search_base = 0;
    int somebody_found = 0;
    int search_quality = 0;
    bool known = false;
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj = NULL;
    OBJ_DATA *helm = NULL;
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *taf = NULL;
    AFFECTED_TYPE *saf = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    int trap_roll = ch->skills[SKILL_MECHANICS] ? (ch->skills[SKILL_MECHANICS]/2) + (number(0,(ch->skills[SKILL_MECHANICS]*2))) : search_base/2;
    trap_roll = trap_roll - 15;

    dir = ch->delay_info1;

    // So, assume 12 int, 10 aur, 12 wil, you'll get a search score between 37 and 77

    search = ((ch->intel * 2.5) + (ch->aur / 2) + ch->wil) - 10 + number(0, 40);
    search_base = int(search);

    // If a character is wearing an enclosed helm, or something covering their face, then their search ability is significantly reduced.

    if (((helm = get_equip (ch, WEAR_HEAD)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK))
            || ((helm = get_equip (ch, WEAR_FACE)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK)))
        search_base = search_base / 2;

    if (dir == -1)
    {

        for (tch = ch->room->people; tch; tch = tch->next_in_room)
        {

            // You will never find yourself playing SOI
            if (tch == ch)
                continue;

            // Don't reveal people you can already see
            if (!get_affect (tch, MAGIC_HIDDEN))
                continue;

            // Don't reveal group members
            if (are_grouped (ch, tch))
                continue;

            // Don't reveal Admins
            if (GET_FLAG (tch, FLAG_WIZINVIS))
                continue;

            // Don't reveal when night blind
            if (!IS_LIGHT (ch->room)
                    && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
					&& !is_goggled(ch)
                    && !IS_SET (ch->affected_by, AFF_INFRAVIS))
                continue;

            /*
            // Don't reveal elves?
            if ((ch->room->sector_type == SECT_WOODS ||
             ch->room->sector_type == SECT_FOREST ||
             ch->room->sector_type == SECT_HILLS) &&
            get_affect (tch, MAGIC_AFFECT_CONCEALMENT))
            continue;
            */

            // Don't reveal the invisible
            if (get_affect (tch, MAGIC_AFFECT_INVISIBILITY) &&
                    !get_affect (ch, MAGIC_AFFECT_SEE_INVISIBLE))
                continue;


            // How well you can hide makes it more difficult to spot you.

            search_quality = (search_base) -
                             (skill_level (tch, SKILL_HIDE, 0) / 4);

            if (search_quality <= 0)
                continue;

            if (search_quality < number (0, 100))
            {
                if (!number (0, 5))
                    act ("$n looks right at you, but still doesn't see you.",
                         true, ch, 0, tch, TO_VICT);
                continue;
            }

            act ("You are exposed by $N!", false, tch, 0, ch, TO_CHAR);
            act ("You expose $N!", false, ch, 0, tch, TO_CHAR);
            act ("$n exposes $N.", false, ch, 0, tch, TO_NOTVICT);

            if (enter_exit_msg (ch, buf))
                act (buf, false, tch, 0, 0, TO_ROOM);

            remove_affect_type (tch, MAGIC_HIDDEN);

            somebody_found = 1;
        }

        if (!somebody_found)
        {
            send_to_char ("You didn't find anybody hiding.\n", ch);
            act ("$n finishes searching.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
        }

        for (obj = ch->room->contents; obj; obj = obj->next_content)
        {

            if (!(af = get_obj_affect (obj, MAGIC_HIDDEN)))
                continue;

            if (!IS_LIGHT (ch->room)
                    && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
					&& !is_goggled(ch)
                    && !IS_SET (ch->affected_by, AFF_INFRAVIS))
                continue;

            if (af->a.hidden.coldload_id == ch->coldload_id)
                continue;

            for (taf = obj->xaffected; taf; taf = taf->next)
            {
                if ((taf->type == MAGIC_HIDDEN) && (taf->a.hidden.coldload_id == ch->coldload_id))
                    known = true;
            }

            if (known)
                continue;


            if (search_base <= number (1, 100))
                continue;

            if (af->a.hidden.hidden_value
                    && number (1, search_base) < number (1, af->a.hidden.hidden_value))
                continue;


            if (GET_ITEM_TYPE(obj) == ITEM_TRAP && obj->o.od.value[1] == -1)
            {
                if (trap_roll < (af->a.hidden.hidden_value - 15))
                {
                    if (obj->o.od.value[0] == 0)
                        act ("You reveal $p, but too late you you accidentally triggered the trap in your search. Thankfully, it appears the trap was either not armed or a dud.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
                    else
                    {
                        act ("You reveal $p, but too late you realise you have accidentally triggered the trap in your search...", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
                        trap_enact(ch, obj, 4, NULL);
                        break;
                    }
                }
                else if (trap_roll < (af->a.hidden.hidden_value))
                    continue;
            }

            //remove_obj_mult_affect(obj, MAGIC_HIDDEN);

            // You no longer "reveal" things: you learn about their location.

            saf = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
            saf->type = MAGIC_HIDDEN;
            saf->a.hidden.duration = -1;
            saf->a.hidden.hidden_value = af->a.hidden.hidden_value;
            saf->a.hidden.coldload_id = ch->coldload_id;
            saf->next = obj->xaffected;
            obj->xaffected = saf;

            act ("You espy $p.", false, ch, obj, 0, TO_CHAR);
            //act ("$n reveals $p.", false, ch, obj, 0, TO_ROOM);
        }

        return;
    }

    if (!ch->room->secrets[dir] ||
            (search_base < ch->room->secrets[dir]->diff))
    {
        send_to_char ("You didn't find anything.\n", ch);
        return;
    }

    send_to_char (ch->room->secrets[dir]->stext, ch);
}

//////////////////////////////////////////////////////////////////////////////
// room__get_description ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Obtain the current room description
//
/// \param[in]  room  The room to examine (i.e. "this->get_description").
/// \return     The current room description
//
/// By default, return the room's description. If there are extra descriptions
/// defined, then pick the closest case and return that instead. If you have
/// an extra description for the current weather & time scenario, return that.
///
/// Otherwise fall back on the default room description.
///
/// 15-12-07: Fixed a long standing bug that didn't display night time weather
/// descriptions, but had to make the function somewhat messier to do so.
///    - Kithrater
///
/// 17-12-07: If the room has a xerox, copy the emulated room's descs instead
/// of our own, to save 5 meg room files.
///    - Kithrater
//
//////////////////////////////////////////////////////////////////////////////
char *
room__get_description (ROOM_DATA * room)
{
    char * description = room->description;
    int i = room->zone;
    ROOM_DATA *xerox_room;
    ROOM_EXTRA_DATA * room_extra = room->extra;

    if (room->xerox > 0)
    {
        xerox_room = vnum_to_room(room->xerox);

        room_extra = xerox_room->extra;

        room->description = xerox_room->description;
    }

    if (room_extra)
    {

        desc_weather[i] = WR_NORMAL;

        /*
        if (weather_info[i].clouds > CLEAR_SKY)
            desc_weather[i] = WR_CLOUDY;

        if (weather_info[i].fog)
            desc_weather[i] = WR_FOGGY;

        if (weather_info[i].state > CHANCE_RAIN)
        {
            desc_weather[i] = WR_RAINY;
            if (weather_info[i].wind_speed == STORMY)
                desc_weather[i] = WR_STORMY;
        }

        if (weather_info[i].state > HEAVY_RAIN)
        {
            desc_weather[i] = WR_SNOWY;
            if (weather_info[i].wind_speed == STORMY)
                desc_weather[i] = WR_BLIZARD;
        }

        if (!sun_light)
        {
            switch (desc_weather[i])
            {
            case WR_NORMAL:
                desc_weather[i] = WR_NIGHT;
                break;
            case WR_FOGGY:
                desc_weather[i] = WR_NIGHT_FOGGY;
                break;
            case WR_CLOUDY:
                desc_weather[i] = WR_NIGHT;
                break;
            case WR_RAINY:
                desc_weather[i] = WR_NIGHT_RAINY;
                break;
            case WR_SNOWY:
                desc_weather[i] = WR_NIGHT_SNOWY;
                break;
            case WR_BLIZARD:
                desc_weather[i] = WR_NIGHT_BLIZARD;
                break;
            case WR_STORMY:
                desc_weather[i] = WR_NIGHT_STORMY;
                break;
            }
        }
        */

        int weather_room = desc_weather[i];
        char * extra_description = 0;

        if (weather_room != WR_NORMAL
                && (extra_description = room_extra->weather_desc[weather_room]))
        {
            description = extra_description;
        }
    }
    return description;
}


void do_look (CHAR_DATA * ch, char *argument, int cmd)
{
    int temp = 0;
    int dir = 0;
    char *ptr = '\0';
    OBJ_DATA *obj = NULL;
    CHAR_DATA *tch = NULL, *top_leader = NULL;;
    ROOM_DATA *troom = NULL;
    ROOM_DIRECTION_DATA *exit = NULL;
    AFFECTED_TYPE *af = NULL;
    bool contents = false;
    char bitbuf[MAX_STRING_LENGTH] = { '\0' };
    char arg1[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char stink_buf[MAX_STRING_LENGTH] = { '\0' };
    //int nRoomVnum = 0, nZone = 1, original_loc = 0;
    //bool change = false, again = true, abrt = false,
    bool found = false;
    bool eq = true;
    bool hands = true;
    bool room = true;

    //char * blizzard_description =
    //    "   A howling blanket of white completely obscures your vision.\n";

    const char *e_dirs[] =
        { "the north", "the east", "the south", "the west", "above", "below" };


    if (GET_POS (ch) < POSITION_SLEEPING)
    {
        send_to_char ("You are unconscious!\n", ch);
        return;
    }

    if (GET_POS (ch) == POSITION_SLEEPING)
    {
        send_to_char ("You are asleep.\n", ch);
        return;
    }

    if (is_blind (ch))
    {
        send_to_char ("You are blind!\n", ch);
        return;
    }

    if (!ch->room)
        ch->room = vnum_to_room (ch->in_room);

    argument = one_argument (argument, arg1);

    /** Arena **/
    if (strcasecmp (arg1, "arena") == STR_MATCH && (ch->in_room == GRUNGE_VIP_VIEW || ch->in_room == GRUNGE_PUBLIC_VIEW))
    {
        grunge_arena__do_look (ch, argument, 0);
        return;
    }
    else if (strcasecmp (arg1, "arena") == STR_MATCH && ch->in_room == GRUNGE_PRIVATE_VIEW)
    {
        grunge_arena__do_look (ch, argument, 1);
        return;
    }


    /** Window **/
    /*
    if ((strcasecmp (arg1, "vortex") == STR_MATCH && ch->in_room == OOC_LOUNGE)
        || (!strn_cmp (arg1, "vortex", 6)
        && strcasecmp (ch->room->name, PREGAME_ROOM_NAME) == STR_MATCH))
      {

        while (again)
      {

        nZone = number (10, 19);
        nRoomVnum = (nZone * 1000) + number (1, 999);

        if (!(troom = vtor (nRoomVnum)))
          continue;

        if (IS_SET (troom->room_flags, INDOORS))
          continue;

        if (troom->sector_type == SECT_INSIDE)
          continue;

        if (IS_SET (troom->room_flags, STORAGE))
          continue;

        if (strlen (troom->description) < 256)
          continue;

        if (!strncmp (troom->description, "No Description Set", 17))
          continue;

        if (!strncmp (troom->description, "   No Description Set", 20))
          continue;

        abrt = false;
        for (tch = troom->people; tch; tch = tch->next_in_room)
          if (!IS_NPC (tch))
            abrt = true;

        if (!abrt)
          {
            again = false;
          }
      }

        if (!IS_SET (ch->affected_by, AFF_INFRAVIS))
      {
        ch->affected_by |= AFF_INFRAVIS;
        change = true;
      }
        original_loc = ch->in_room;
        act
      ("You peer in to the swirling vortex, a sudden lifelike picture forming before you. . .",
       false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        char_from_room (ch);
        char_to_room (ch, nRoomVnum);
        send_to_char ("\n", ch);
        do_look (ch, "", 0);
        char_from_room (ch);
        char_to_room (ch, original_loc);
        if (IS_MORTAL (ch))
      ch->roundtime = 10;
        if (change)
      ch->affected_by &= ~AFF_INFRAVIS;
        return;
      }
    */

    /* LOOK IN A CERTAIN DIRECTION */

    if (*arg1 && strcasecmp (arg1, "in") != STR_MATCH
            && (dir = index_lookup (dirs, arg1)) != -1)
    {

        if (!(exit = EXIT (ch, dir)))
        {
            send_to_char ("There is no exit that way.\n", ch);
            return;
        }

        if (exit->general_description && *exit->general_description)
            send_to_char (exit->general_description, ch);

        if ((af = get_affect (ch, AFFECT_SHADOW))
                && af->a.shadow.edge == dir
                && IS_SET (exit->exit_info, EX_ISDOOR)
                && IS_SET (exit->exit_info, EX_CLOSED)
                && !IS_SET (exit->exit_info, EX_ISGATE))
        {
            send_to_char
            ("Your field of view through that exit is obstructed.\n", ch);
        }

        else if ((af = get_affect (ch, AFFECT_SHADOW)) &&
                 af->a.shadow.edge == dir)
        {
            send_to_char ("You are close enough to see what is in the next "
                          "room:\n\n", ch);

            temp = ch->in_room;

            char_from_room (ch);
            char_to_room (ch, exit->to_room);

            list_obj_to_char (ch->room->contents, ch, 0, false);
            list_char_to_char (ch->room->people, ch, 0);

            for (tch = ch->room->people; tch; tch = tch->next_in_room)
                if (CAN_SEE (ch, tch))
                    target_sighted (ch, tch);

            char_from_room (ch);
            char_to_room (ch, temp);
        }

        else
        {

            if (!(troom = vnum_to_room (exit->to_room)))
            {
                send_to_char ("The way in that direction is blocked.\n", ch);
                return;
            }

            temp = 0;

            for (tch = troom->people; tch; tch = tch->next_in_room)
                if ((af = get_affect (tch, AFFECT_SHADOW)) &&
                        af->a.shadow.edge == rev_dir[dir] && could_see (ch, tch))
                {
                    act ("You see $N.", false, ch, 0, tch, TO_CHAR);
                    temp = 1;
                }
        }
        return;
    }

    if (is_dark (ch->room) && IS_MORTAL (ch)
			&& !is_goggled(ch)
			&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
        send_to_char ("It is pitch black.\n", ch);
        return;
    }

    /* LOOK INSIDE ANOTHER OBJECT */

    if (strcasecmp (arg1, "in") == STR_MATCH)
    {

        argument = one_argument (argument, arg1);

        if (!*arg1)
        {
            send_to_char ("Look in what?\n", ch);
            return;
        }

        if (!(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
                !(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
                !(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
                !(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
        {
            send_to_char ("You don't see that here.\n", ch);
            return;
        }

        if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON)
        {

            if (!obj->contains)
                act ("$o is empty.", false, ch, obj, 0, TO_CHAR);

            else
            {
                if (obj->o.drinks.capacity)
                    temp = obj->contains->count * 3 / obj->o.drinks.capacity;
                else
                    temp = 1;

                sprintf (buf, "$p is %sfull of %s.",
                         fullness[temp],
                         obj_short_desc(obj->contains));
                act (buf, false, ch, obj, 0, TO_CHAR);
            }

            return;
        }

        else if (GET_ITEM_TYPE (obj) == ITEM_LIGHT)
        {
            if (obj->o.light.hours)
            {
                sprintf (buf,
                         "$p looks like it will last about %d more hour%s.",
                         obj->o.light.hours,
                         (obj->o.light.hours > 1) ? "s" : "");
                act (buf, false, ch, obj, 0, TO_CHAR);
            }
            else if (obj->o.light.hours == 0)
            {
                act ("$o is spent.", false, ch, obj, 0, TO_CHAR);
            }
            else
            {
                send_to_char
                ("You can't tell how much longer that will last..\n", ch);
            }
        }

        else if (IS_ELECTRIC(obj))
        {
            send_to_char (fname (obj->name), ch);

            if (obj->in_room != NOWHERE)
                send_to_char (" (here) : \n", ch);
            else if (obj->carried_by)
                send_to_char (" (carried) : \n", ch);
            else
                send_to_char (" (used) : \n", ch);

            list_obj_to_char (obj->contains, ch, 1, true);
        }
        else if (GET_ITEM_TYPE (obj) == ITEM_FIREARM || GET_ITEM_TYPE (obj) == ITEM_CLIP)
        {
            send_to_char (fname (obj->name), ch);

            if (obj->in_room != NOWHERE)
                send_to_char (" (here) : \n", ch);
            else if (obj->carried_by)
                send_to_char (" (carried) : \n", ch);
            else
                send_to_char (" (used) : \n", ch);

            list_obj_to_char (obj->contains, ch, 1, true);
        }
        else if (GET_ITEM_TYPE (obj) == ITEM_TRAP)
        {
            send_to_char (fname (obj->name), ch);

            if (obj->in_room != NOWHERE)
                send_to_char (" (here) : \n", ch);
            else if (obj->carried_by)
                send_to_char (" (carried) : \n", ch);
            else
                send_to_char (" (used) : \n", ch);

            list_obj_to_char (obj->contains, ch, 1, true);
        }
        else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
                 GET_ITEM_TYPE (obj) == ITEM_COVER ||
                 (GET_ITEM_TYPE (obj) == ITEM_WORN && obj->o.od.value[0]) ||
                 GET_ITEM_TYPE (obj) == ITEM_CRATE ||
                 GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
                 GET_ITEM_TYPE (obj) == ITEM_SHEATH ||
                 GET_ITEM_TYPE (obj) == ITEM_HOLSTER ||
                 GET_ITEM_TYPE (obj) == ITEM_AMMO_BELT ||
                 GET_ITEM_TYPE (obj) == ITEM_BANDOLIER ||
                 /*(GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
                 obj->o.weapon.use_skill == SKILL_SLING) ||*/
                 GET_ITEM_TYPE (obj) == ITEM_KEYRING)
        {

            if (IS_SET (obj->o.container.flags, CONT_CLOSED))
            {
                send_to_char ("It is closed.\n", ch);
                return;
            }

            send_to_char (fname (obj->name), ch);

            if (obj->in_room != NOWHERE)
                send_to_char (" (here) : \n", ch);
            else if (obj->carried_by)
                send_to_char (" (carried) : \n", ch);
            else
                send_to_char (" (used) : \n", ch);

            list_obj_to_char (obj->contains, ch, 1, true);
        }

        else
            send_to_char ("That is not a container.\n", ch);
    }

    /* LOOK OVERBOAD */
    else if (strcasecmp (arg1, "overboard") == STR_MATCH
             || strcasecmp (arg1, "over") == STR_MATCH)
    {

        if (!ch->vehicle)
        {
            send_to_char ("You're not on a boat?\n", ch);
            return;
        }

        if (!IS_OUTSIDE (ch))
        {
            send_to_char ("You can do that on deck, but not from here.\n", ch);
            return;
        }

        if (is_dark (ch->room) && IS_MORTAL (ch))
            send_to_char ("It is dark overboard.\n", ch);
        else
        {
            sprintf (buf, "#6%s#0\n", ch->vehicle->room->name);
            send_to_char (buf, ch);
            /*
            if (weather_info[ch->vehicle->room->zone].state == HEAVY_SNOW
                    && !IS_SET (ch->vehicle->room->room_flags, INDOORS)
                    && IS_MORTAL (ch))
            {
                send_to_char (blizzard_description, ch);
            }
            else
            {*/
            send_to_char (room__get_description (ch->vehicle->room), ch);
            //}
        }

        if (ch->vehicle && IS_OUTSIDE (ch))
        {
            troom = ch->vehicle->room;
            if (troom->contents || troom->people->next_in_room)
            {
                if (troom->contents)
                    list_obj_to_char (troom->contents, ch, 0, false);
                if (troom->people->next_in_room)
                    list_char_to_char (troom->people, ch, 0);
            }
        }
    }

    /* LOOK AT SOMETHING */
    else if (*arg1)
    {

        if (strcasecmp (arg1, "at") == STR_MATCH)
            argument = one_argument (argument, arg1);

        if (strcasecmp (arg1, "hands") == STR_MATCH || strcasecmp (arg1, "hand") == STR_MATCH)
        {
            eq = false;
            room = false;
            argument = one_argument (argument, arg1);
        }
        else if (strcasecmp (arg1, "eq") == STR_MATCH || strcasecmp (arg1, "equip") == STR_MATCH || strcasecmp (arg1, "equipment") == STR_MATCH)
        {
            hands = false;
            room = false;
            argument = one_argument (argument, arg1);
        }
        else if (strcasecmp (arg1, "room") == STR_MATCH)
        {
            hands = false;
            eq = false;
            argument = one_argument (argument, arg1);
        }

        if (!*arg1)
        {
            send_to_char ("Look at what?\n", ch);
            return;
        }

        if ((tch = get_char_room_vis (ch, arg1)))
        {

            argument = one_argument (argument, arg1);

            if (is_abbrev(arg1, "group"))
            {
                if (!do_group_size(tch))
                {
                    act("$N isn't in a group.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                    return;
                }

                if (!(top_leader = tch->following))
                    top_leader = tch;

                sprintf (buf, "#5%s#0, leading:\n\n", char_short (top_leader));
                buf[2] = toupper (buf[2]);

                for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
                {
                    if (tch->following != top_leader)
                        continue;
                    if (!CAN_SEE (ch, tch))
                        continue;
                    if (found != false)
                        sprintf (buf + strlen (buf), "\n");
                    sprintf (buf + strlen (buf), "   #5%s#0", char_long (tch, 0));
                    found = true;
                }

                strcat (buf, "\n");
                send_to_char (buf, ch);
                return;
            }

            if (*arg1 && hands
                    && ((obj = get_obj_in_list_vis (tch, arg1, tch->right_hand))
                        || (obj = get_obj_in_list_vis (tch, arg1, tch->left_hand))))
            {
                act ("$N is carrying $p.", false, ch, obj, tch, TO_CHAR);
                send_to_char ("\n", ch);
                if (cmd == 2)
                    show_obj_to_char (obj, ch, 15);
                else
                    show_obj_to_char (obj, ch, 5);
                return;
            }

            if (*arg1 && eq && (obj = get_obj_in_list_vis (tch, arg1, tch->equip)))
            {
                act ("$N is wearing $p.", false, ch, obj, tch, TO_CHAR);
                send_to_char ("\n", ch);
                if (cmd == 2)
                    show_obj_to_char (obj, ch, 15);
                else
                    show_obj_to_char (obj, ch, 5);
                return;
            }

            if (*arg1)
            {
                act ("$N doesn't have that.", false, ch, 0, tch, TO_CHAR);
                return;
            }

            if (cmd == 2)
                show_char_to_char (tch, ch, 15);
            else
                show_char_to_char (tch, ch, 1);

            return;
        }

        if (!(obj = (hands ? get_obj_in_dark (ch, arg1, ch->right_hand) : NULL)) &&
                !(obj = (hands ? get_obj_in_dark (ch, arg1, ch->left_hand) : NULL)) &&
                !(obj = (eq ? get_obj_in_dark (ch, arg1, ch->equip) : NULL )))
            obj = (room ? get_obj_in_list_vis (ch, arg1, ch->room->contents) : NULL);

        /** BOARDS **/
        if (obj && obj->obj_flags.type_flag == ITEM_BOARD)
        {

            if (obj->clan_data
                    && !(is_clan_member(ch, obj->clan_data->name)))
            {
                show_obj_to_char (obj, ch, 15);
                send_to_char ("\nYou can not read these reports, but you can write one.", ch);
                return;
            }

            else
            {
                one_argument (obj->name, buf);
                retrieve_mysql_board_listing (ch, buf);
                return;
            }
        }

        /* Extra room description */

        if ((ptr = find_ex_description (arg1, ch->room->ex_description)) &&
                *ptr)
        {
            page_string (ch->descr(), ptr);
            return;
        }

        if (obj)
        {
            if (cmd == 2)
                show_obj_to_char (obj, ch, 15);
            else
                show_obj_to_char (obj, ch, 5);
            return;
        }

        if (*arg1)
        {
            if (!strn_cmp (arg1, "furnishings", strlen (arg1)) ||
                    !strn_cmp (arg1, "furniture", strlen (arg1)) ||
                    !strn_cmp (arg1, "tables", strlen (arg1)))
            {
                do_tables (ch, "", 0);
                return;
            }

            if (!strn_cmp (arg1, "corpses", strlen (arg1)) ||
                    !strn_cmp (arg1, "bodies", strlen (arg1)) ||
                    !strn_cmp (arg1, "limbs", strlen (arg1)) ||
                    !strn_cmp (arg1, "dead", strlen (arg1)))
            {
                do_corpses (ch, "", 0);
                return;
            }

            if (!strn_cmp (arg1, "bullets", strlen (arg1)) ||
                    !strn_cmp (arg1, "casings", strlen (arg1)))
            {
                do_bullets (ch, "", 0);
                return;
            }

            if (!strn_cmp (arg1, "objects", strlen (arg1)))
            {
                show_contents (ch, "", 1);
                return;
            }

            if (!strn_cmp (arg1, "crowd", strlen (arg1)) ||
                    !strn_cmp (arg1, "individuals", strlen (arg1)))
            {
                show_contents (ch, "", 2);
                return;
            }
        }

        send_to_char ("You do not see that here.\n", ch);

        return;
    }

    else
    {
        /* General look */

        load_weather_obj(ch->room);

        if (IS_MORTAL (ch))
        {

            sprintf (buf, "\n#6%s#0", ch->room->name);
            send_to_char (buf, ch);
        }
        else
        {
            sprintbit (ch->room->room_flags, room_bits, bitbuf);
            sprintf (buf, "\n#6%s#0 #2[%d: %s#6%s#2]#0",
                     ch->room->name,
                     ch->room->vnum,
                     bitbuf, sector_types[ch->room->sector_type]);

            send_to_char (buf, ch);

            if (ch->room->prg)
                send_to_char (" [Prog]", ch);

            if (ch->room->deity && ch->room->sector_type != SECT_LEANTO)
            {
                /* Lean-tos use the deity int to record the number of people camping there. */
                sprintf (buf, " Temple of %s\n", deity_name[ch->room->deity]);
                send_to_char (buf, ch);
            }

            if (ch->room->sector_type == SECT_LEANTO)
            {
                sprintf (buf, "\nNumber of Occupants: %d.",
                         (ch->room->deity - 15));
                if (!IS_MORTAL (ch))
                    send_to_char (buf, ch);
            }
        }

        /* Display path vehicle can take */

        if (ch->vehicle &&
                is_he_somewhere (ch->vehicle) &&
                ch->vehicle->mob->helm_room == ch->in_room)
        {

            strcpy (buf, " May sail: [#4");

            for (dir = 0; dir <= LAST_DIR; dir++)
            {

                if (!EXIT (ch->vehicle, dir) ||
                        !(troom = vnum_to_room (EXIT (ch->vehicle, dir)->to_room)))
                    continue;

                if (troom->sector_type == SECT_REEF ||
                        troom->sector_type == SECT_OCEAN ||
                        is_room_affected (troom->affects, MAGIC_ROOM_FLOOD))
                    add_char (buf, toupper (*dirs[dir]));
            }

            strcat (buf, "#0]");
            send_to_char (buf, ch);
        }

		if (!IS_SET(ch->plr_flags, BRIEF_MODE))
        {
            sprintf (buf, "#6Exits:#0 ");

            for (dir = 0; dir <= LAST_DIR; dir++)
            {
                if (!EXIT (ch, dir))
                    continue;
                if (ch->room->secrets[dir]
                        && IS_SET (EXIT (ch, dir)->exit_info, EX_CLOSED)
                        && IS_MORTAL (ch))
                    continue;
                if (EXIT (ch, dir)->to_room
                        && (troom = vnum_to_room (EXIT (ch, dir)->to_room))
                        && IS_SET (troom->room_flags, FALL))
                    sprintf (buf + strlen (buf), "#1%s#0 ", dirs[dir]);
                else if (EXIT (ch, dir)->to_room
                         && (troom = vnum_to_room (EXIT (ch, dir)->to_room))
                         && (troom->sector_type == SECT_RIVER
                             || troom->sector_type == SECT_LAKE
                             || troom->sector_type == SECT_OCEAN
                             || troom->sector_type == SECT_REEF
                             || troom->sector_type == SECT_UNDERWATER))
                    sprintf (buf + strlen (buf), "#4%s#0 ", dirs[dir]);
                else if (EXIT (ch, dir)->to_room
                         && (troom = vnum_to_room (EXIT(ch, dir)->to_room))
                         && (ch->room->sector_type == SECT_RUINSTREET
                             || ch->room->sector_type == SECT_RUINHIGHWAY)
                         && (troom->sector_type == SECT_RUINSTREET
                             || troom->sector_type == SECT_RUINHIGHWAY))
                    sprintf (buf + strlen (buf), "#3%s#0 ", dirs[dir]);
                else
                    sprintf (buf + strlen (buf), "#2%s#0 ", dirs[dir]);
                if (!EXIT (ch, dir)->exit_info
                        || (dir > 5
                            && (obj = find_dwelling_obj (ch->room->vnum))
                            && !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE)))
                    continue;
                sprintf (buf + strlen (buf), "(%s %s) ",
                         IS_SET (EXIT (ch, dir)->exit_info,
                                 EX_CLOSED) ? "closed" : "open", EXIT (ch,
                                                                       dir)->
                         keyword);
            }

            send_to_char ("\n", ch);
            act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        }

        send_to_char ("\n", ch);

        /*if (weather_info[ch->room->zone].state == HEAVY_SNOW
                && !IS_SET (ch->room->room_flags, INDOORS)
                && IS_MORTAL (ch))
        {
            send_to_char (blizzard_description, ch);
        }
        else
        {*/

		// If we have a 77 and we're set to brief mode, don't send anything
		// Otherwise, room-desc time.
		if (cmd != 77 || !IS_SET(ch->plr_flags, BRIEF_MODE))
			send_to_char (room__get_description (ch->room), ch);
        //}

        for (obj = ch->room->contents; obj; obj = obj->next_content)
            if (CAN_SEE_OBJ (ch, obj))
                contents = true;

        for (tch = ch->room->people; tch; tch = tch->next_in_room)
            if (CAN_SEE (ch, tch) && ch != tch)
                contents = true;

        if (contents)
            send_to_char ("\n", ch);

        if (ch->room->scent)
        {
            int scent_count = 0;
            SCENT_DATA *scent = NULL;
            SCENT_DATA *scent_one = NULL;
            SCENT_DATA *scent_two = NULL;
            SCENT_DATA *scent_three = NULL;

            for (scent = ch->room->scent; scent; scent = scent->next)
            {
                // Require a scent strength of two or more to smell it.
                if (scent_strength(scent) >= 3)
                {
                    if (!scent_one)
                    {
                        scent_one = scent;
                        scent_count++;
                    }
                    else if (!scent_two)
                    {
                        scent_two = scent;
                        scent_count++;
                    }
                    else if (!scent_three)
                    {
                        scent_three = scent;
                        scent_count++;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_one))
                    {
                        scent_one = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_two = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_three = scent;
                    }
                    else
                        continue;
                }
            }

            if (scent_count == 3)
            {
                sprintf(stink_buf, "#6You detect %s of %s, %s of %s and %s of %s in the air.#0", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
            }
            else if (scent_count == 2)
            {
                sprintf(stink_buf, "#6You detect %s of %s and %s of %s in the air.#0",  scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
            }
            else if (scent_count == 1)
            {
                sprintf(stink_buf, "#6You detect %s of %s in the air.#0", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
            }
        }

        if (*stink_buf)
        {
            /*
            char *p = '\0';
            reformat_string (stink_buf, &p);
            sprintf(arg1, "#6%s\n#0", p);
            mem_free (p); //char*
            p = NULL;
            */
            if (!contents)
                send_to_char ("\n", ch);
            act (stink_buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            send_to_char ("\n", ch);

        }

        if (!IS_SET (ch->room->room_flags, INDOORS)
                && ch->room->sector_type != SECT_UNDERWATER)
        {
            *buf = '\0';
            if (!contents)
                sprintf (buf, "\n");
            if (IS_SET (ch->room->room_flags, STIFLING_FOG))
                sprintf (buf + strlen (buf),
                         "#6A stifling fog blankets the area, minimizing visibility.#0\n");
            /*
            else if (weather_info[ch->room->zone].state == LIGHT_RAIN)
                sprintf (buf + strlen (buf),
                         "#6A light, cool rain is falling here.#0\n");
            else if (weather_info[ch->room->zone].state == STEADY_RAIN)
                sprintf (buf + strlen (buf),
                         "#6A copious amount of rain is falling steadily here.#0\n");
            else if (weather_info[ch->room->zone].state == HEAVY_RAIN)
                sprintf (buf + strlen (buf),
                         "#6Rain falls in heavy sheets, inundating the area with water.#0\n");
            else if (weather_info[ch->room->zone].state == LIGHT_SNOW)
                sprintf (buf + strlen (buf),
                         "#6A light dusting of snow is falling in the area.#0\n");
            else if (weather_info[ch->room->zone].state == STEADY_SNOW)
                sprintf (buf + strlen (buf),
                         "#6Snow is falling steadily here, beginning to blanket the area.#0\n");
            else if (weather_info[ch->room->zone].fog == THIN_FOG)
                sprintf (buf + strlen (buf),
                         "#6Thin, silvery tendrils of fog permeate the area.#0\n");
            else if (weather_info[ch->room->zone].fog == THICK_FOG)
                sprintf (buf + strlen (buf),
                         "#6The area is cloaked in a thick, muted veil of dense white fog.#0\n");
            */
            //else if (weather_info[ch->room->zone].state == CHANCE_RAIN)
            //  sprintf (buf + strlen (buf),
            //        "#6The air here carries the subtle aroma of impending rain.\n#0");
            if (contents)
                sprintf (buf + strlen (buf), "\n");
            if (strlen (buf) > 2)
                send_to_char (buf, ch);
        }

        list_obj_to_char (ch->room->contents, ch, 0, false);

        list_char_to_char (ch->room->people, ch, 0);

		if (IS_SET(ch->plr_flags, BRIEF_MODE))
        {
            sprintf (buf, "#6Exits:#0 ");

            for (dir = 0; dir <= LAST_DIR; dir++)
            {
                if (!EXIT (ch, dir))
                    continue;
                if (ch->room->secrets[dir]
                        && IS_SET (EXIT (ch, dir)->exit_info, EX_CLOSED)
                        && IS_MORTAL (ch))
                    continue;
                if (EXIT (ch, dir)->to_room
                        && (troom = vnum_to_room (EXIT (ch, dir)->to_room))
                        && IS_SET (troom->room_flags, FALL))
                    sprintf (buf + strlen (buf), "#1%s#0 ", dirs[dir]);
                else if (EXIT (ch, dir)->to_room
                         && (troom = vnum_to_room (EXIT (ch, dir)->to_room))
                         && (troom->sector_type == SECT_RIVER
                             || troom->sector_type == SECT_LAKE
                             || troom->sector_type == SECT_OCEAN
                             || troom->sector_type == SECT_REEF
                             || troom->sector_type == SECT_UNDERWATER))
                    sprintf (buf + strlen (buf), "#4%s#0 ", dirs[dir]);
                else if (EXIT (ch, dir)->to_room
                         && (troom = vnum_to_room (EXIT(ch, dir)->to_room))
                         && (ch->room->sector_type == SECT_RUINSTREET
                             || ch->room->sector_type == SECT_RUINHIGHWAY)
                         && (troom->sector_type == SECT_RUINSTREET
                             || troom->sector_type == SECT_RUINHIGHWAY))
                    sprintf (buf + strlen (buf), "#3%s#0 ", dirs[dir]);
                else
                    sprintf (buf + strlen (buf), "#2%s#0 ", dirs[dir]);
                if (!EXIT (ch, dir)->exit_info
                        || (dir > 5
                            && (obj = find_dwelling_obj (ch->room->vnum))
                            && !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE)))
                    continue;
                sprintf (buf + strlen (buf), "(%s %s) ",
                         IS_SET (EXIT (ch, dir)->exit_info,
                                 EX_CLOSED) ? "closed" : "open", EXIT (ch,
                                                                       dir)->
                         keyword);
            }

            send_to_char ("\n", ch);
            act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        }

        *buf = '\0';

        for (af = ch->room->affects; af; af = af->next)
        {
            if (af->type == MAGIC_ROOM_FIGHT_NOISE)
                sprintf (buf + strlen (buf),
                         "#1The clamor of an armed conflict emanates from %s!#0\n",
                         e_dirs[af->a.room.duration]);
        }

        if (*buf)
        {
            send_to_char ("\n", ch);
            send_to_char (buf, ch);
        }
    }
}

int
read_pc_message (CHAR_DATA * ch, char *name, char *argument)
{
    CHAR_DATA *who = NULL;
    MESSAGE_DATA *message = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    *name = toupper (*name);

    if (!(who = load_pc (name)))
        return 0;

    argument = one_argument (argument, buf);

    if (!atoi (buf))
    {
        send_to_char ("Which message?\n", ch);
        return 1;
    }

    if (!(message = load_message (name, 7, atoi (buf))))
    {
        send_to_char ("No such message.\n", ch);
        return 1;
    }

    sprintf (b_buf, "#6Date:#0    %s\n"
             "#6Author:#0  %s\n"
             "#6Subject:#0 %s\n\n%s", message->date, message->poster,
             message->subject, message->message);

    send_to_char ("\n", ch);
    page_string (ch->descr(), b_buf);

    if (ch == who && !message->flags)
        mark_as_read (ch, atoi (buf));

    unload_message (message);
    unload_pc (who);

    return 1;
}

int
read_virtual_message (CHAR_DATA * ch, char *name, char *argument)
{
    MESSAGE_DATA *message = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    *name = toupper (*name);

    argument = one_argument (argument, buf);

    if (!atoi (buf))
    {
        send_to_char ("Which message?\n", ch);
        return 1;
    }

    if (!(message = load_message (name, 5, atoi (buf))))
    {
        send_to_char ("No such virtual board message.\n", ch);
        return 1;
    }

    sprintf (b_buf, "#6Date:#0     %s\n"
             "#6Author:#0   %s\n"
             "#6Subject:#0  %s\n"
             "#6Context:#0  http://www.atonementrpi.com.org/staffportal.php?context=%ld&db=all##%ld\n\n%s",
             message->date, message->poster, message->subject,
             message->nTimestamp, message->nTimestamp, message->message);

    send_to_char ("\n", ch);
    page_string (ch->descr(), b_buf);

    unload_message (message);

    return 1;
}

void do_read (CHAR_DATA * ch, char *argument, int cmd)
{
    int msg_num = 0;
    OBJ_DATA *board_obj = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (!isdigit (*buf))
    {
        if (!*buf)
        {
            send_to_char ("What do you wish to read?\n", ch);
            return;
        }

        if (
                !( ( board_obj = get_obj_in_dark( ch, buf, ch->right_hand ) )
                && ( !USES_BOOK_CODE( board_obj ) || GET_ITEM_TYPE( board_obj ) != ITEM_BOARD ) )
                &&
                !( ( board_obj = get_obj_in_dark (ch, buf, ch->left_hand ) )
                && ( !USES_BOOK_CODE( board_obj ) || GET_ITEM_TYPE( board_obj ) != ITEM_BOARD ) )
                &&
                !( ( board_obj = get_obj_in_dark (ch, buf, ch->equip ) )
                && ( !USES_BOOK_CODE( board_obj ) || GET_ITEM_TYPE( board_obj ) != ITEM_BOARD ) )
                &&
                !( ( board_obj = get_obj_in_dark (ch, buf, ch->room->contents ) )
                && ( !USES_BOOK_CODE( board_obj ) || GET_ITEM_TYPE( board_obj ) != ITEM_BOARD ) )
            )
        {
            if (IS_MORTAL (ch) || !read_pc_message (ch, buf, argument))
            {
                if (IS_MORTAL (ch) || !read_virtual_message (ch, buf, argument))
                {
                    send_to_char ("You can't see that board.\n", ch);
                }
            }
            return;
        }

        if ( !board_obj )
        {
            send_to_char( "What do you wish to look at?", ch );
            return;
        }

        if ( USES_BOOK_CODE( board_obj ) )
        {
            do_look (ch, board_obj->name, 0);
            return;
        }

        argument = one_argument (argument, buf);

        if (!isdigit (*buf))
        {
            send_to_char ("Which message on that board?\n", ch);
            return;
        }
    }

    else
    {
        for (board_obj = ch->room->contents;
                board_obj; board_obj = board_obj->next_content)
        {

            if (!CAN_SEE_OBJ (ch, board_obj))
                continue;

            if (board_obj->obj_flags.type_flag == ITEM_BOARD)
                break;
        }

        if (!board_obj)
        {
            send_to_char ("You do not see a board here.\n", ch);
            return;
        }
    }

    msg_num = atoi (buf);

    one_argument (board_obj->name, buf);

    if (board_obj->clan_data &&
            (!is_clan_member(ch, board_obj->clan_data->name)))

    {
        send_to_char ("You are not authorized to read these reports", ch);
        return;
    }

    display_mysql_board_message (ch, buf, msg_num, 0);
}

void
do_examine (CHAR_DATA * ch, char *argument, int cmd)
{
    do_look (ch, argument, 2);
}

void
do_exits (CHAR_DATA * ch, char *argument, int cmd)
{
    int dir = 0;
    ROOM_DIRECTION_DATA *exit = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char strExit[AVG_STRING_LENGTH] = { '\0' };

    char *exits[] =
    {
        "North",
        "East ",
        "South",
        "West ",
        "Up   ",
        "Down ",
        "Outside",
        "Inside"
    };

    if (is_dark (ch->room) &&
            IS_MORTAL (ch)
			&& !is_goggled(ch)
			&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
            && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
        send_to_char ("You can't see a thing!\n", ch);
        return;
    }

    /*
    if (weather_info[ch->room->zone].state == HEAVY_SNOW
            && !IS_SET (ch->room->room_flags, INDOORS) && IS_MORTAL (ch))
    {
        send_to_char ("You can't see a thing!\n", ch);
        return;
    }
    */

    *s_buf = '\0';

    for (dir = 0; dir <= LAST_DIR; dir++)
    {

        exit = EXIT (ch, dir);

        if (!exit || exit->to_room == NOWHERE)
            continue;

        if (!vnum_to_room (exit->to_room))
        {
            sprintf (buf, "Room %d lead to nonexistant room %d (do_exits)\n",
                     ch->in_room, exit->to_room);
            continue;
        }

        strExit[0] = '\0';

        if (IS_SET (exit->exit_info, EX_ISDOOR)
                || IS_SET (exit->exit_info, EX_ISGATE))
        {

            if (IS_SET (exit->exit_info, EX_CLOSED))
            {

                if (!ch->room->secrets[dir])
                {
                    sprintf (strExit + strlen (strExit), "%s -  %s (Closed)",
                             exits[dir], exit->keyword);
                }
                else if (!IS_MORTAL (ch) && ch->room->secrets[dir])
                {
                    sprintf (strExit + strlen (strExit),
                             "%s -  %s (Closed; Secret)", exits[dir],
                             exit->keyword);
                }

            }
            else if (is_dark (vnum_to_room (exit->to_room)) &&
                     IS_MORTAL (ch)
					 && !is_goggled(ch)
					 && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
                     && !IS_SET (ch->affected_by, AFF_INFRAVIS))
                sprintf (strExit + strlen (strExit),
                         "%s - %s (Open) - Too dark to tell", exits[dir],
                         exit->keyword);

            else
                sprintf (strExit + strlen (strExit),
                         "%s - %s (Open) - %s",
                         exits[dir], exit->keyword, vnum_to_room (exit->to_room)->name);
        }

        else
        {

            if (is_dark (vnum_to_room (exit->to_room)) &&
                    IS_MORTAL (ch) &&
                    !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
                    && !IS_SET (ch->affected_by, AFF_INFRAVIS))
                sprintf (strExit + strlen (strExit), "%s - Too dark to tell",
                         exits[dir]);

            else
                sprintf (strExit + strlen (strExit),
                         "%s - %s", exits[dir], vnum_to_room (exit->to_room)->name);
        }

        if (!IS_MORTAL (ch))
        {
            sprintf (strExit + strlen (strExit), " [%d]",
                     vnum_to_room (exit->to_room)->vnum);
        }

        if (strExit[0] != '\0')
        {
            sprintf (s_buf + strlen (s_buf), "%s\n", strExit);
        }
    }

    send_to_char ("Obvious exits:\n", ch);

    if (*s_buf)
        send_to_char (s_buf, ch);
    else
        send_to_char ("None.\n", ch);
}

int
get_stat_range (int score)
{
    if (score <= 4)
        return 0;
    if (score >= 7 && score <= 8)
        return 1;
    if (score >= 9 && score <= 10)
        return 2;
    if (score >= 11 && score <= 13)
        return 3;
    if (score >= 14 && score <= 15)
        return 4;
    if (score >= 16 && score <= 17)
        return 5;
    if (score >= 18 && score <= 19)
        return 6;
    if (score >= 20 && score <= 21)
        return 7;
    if (score >= 22 && score <= 23)
        return 8;
    if (score >= 24)
        return 9;
    return 0;
}


int
get_comestible_range (int num)
{
    if (num <= 0)
        return 0;
    if (num >= 1 && num <= 24)
        return 1;
    if (num >= 25 && num <= 48)
        return 2;
    if (num >= 49 && num <= 72)
        return 3;
    if (num >= 73 && num <= 96)
        return 4;
    if (num >= 97 && num <= 200)
        return 5;
    if (num >= 201 && num <= 240)
        return 6;
    if (num >= 241 && num <= 280)
        return 7;
    if (num >= 281)
        return 8;
    return 0;
}

int
get_hunger_comestible_range (int num)
{
    if (num <= -145)
        return 0;
    if (num >= -144 && num <= -97)
        return 1;
    if (num >= -96 && num <= -49)
        return 2;
    if (num >= -48 && num <= -1)
        return 3;
    if (num >= 0 && num <= 5)
        return 4;
    if (num >= 6 && num <= 10)
        return 5;
    if (num >= 11 && num <= 15)
        return 6;
    if (num >= 16 && num <= 23)
        return 7;
    if (num >= 23 && num <= 34)
        return 8;
    if (num >= 35)
        return 9;
    return 0;
}


void
hunger_thirst_process (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char update_buf[MAX_STRING_LENGTH] = { '\0' };
    AFFECTED_TYPE *af;

    if (IS_NPC (ch))
        return;

    *update_buf = '\0';

    if (get_hunger_comestible_range (ch->hunger) !=
            get_hunger_comestible_range (ch->hunger - 1))
    {
        sprintf (update_buf + strlen (update_buf), "%s",
                 verbal_hunger[get_hunger_comestible_range (ch->hunger - 1)]);
    }
    else if (get_hunger_comestible_range (ch->hunger) <= 3)
    {
        if (!(time_info.hour%2))
            sprintf(update_buf + strlen(update_buf), "%s, slowly wasting away from hunger", verbal_hunger[get_hunger_comestible_range (ch->hunger)]);
    }

    if (get_comestible_range (ch->thirst) !=
            get_comestible_range (ch->thirst - 1))
    {
        if (*update_buf)
            sprintf (update_buf + strlen (update_buf), ", and ");
        sprintf (update_buf + strlen (update_buf), "%s",
                 verbal_thirst[get_comestible_range (ch->thirst - 1)]);
    }

    if (*update_buf)
    {
        sprintf (buf, "#6You are %s.#0\n", update_buf);
        send_to_char (buf, ch);
    }


    if (ch->hunger > -169)
        ch->hunger--;
    if (ch->thirst > 0)
        ch->thirst-= 6;


    if (ch->thirst < -1)
        ch->thirst = 0;
    if (ch->hunger < -169)
        ch->hunger = -168;

    if (GET_TRUST(ch) || IS_NPC(ch))
        ch->hunger = 48;

    if (ch->hunger <= -145)
    {
        if ((af = get_affect(ch, MAGIC_STARVE_ONE)))
            remove_affect_type (ch, MAGIC_STARVE_ONE);

        if ((af = get_affect(ch, MAGIC_STARVE_TWO)))
            remove_affect_type(ch, MAGIC_STARVE_TWO);

        if ((af = get_affect(ch, MAGIC_STARVE_THREE)))
            remove_affect_type(ch, MAGIC_STARVE_THREE);

        if (!(af = get_affect(ch, MAGIC_STARVE_FOUR)))
            magic_add_affect (ch, MAGIC_STARVE_FOUR, -1, 0, 0, 0, 0);

        wound_to_char (ch, "bloodloss", 5, 0, 0, 0, 0);
    }
    else if (ch->hunger >= -144 && ch->hunger <= -97)
    {
        if ((af = get_affect(ch, MAGIC_STARVE_ONE)))
            remove_affect_type(ch, MAGIC_STARVE_ONE);

        if ((af = get_affect(ch, MAGIC_STARVE_TWO)))
            remove_affect_type(ch, MAGIC_STARVE_TWO);

        if ((af = get_affect(ch, MAGIC_STARVE_FOUR)))
            remove_affect_type(ch, MAGIC_STARVE_FOUR);

        if (!(af = get_affect(ch, MAGIC_STARVE_THREE)))
            magic_add_affect (ch, MAGIC_STARVE_THREE, -1, 0, 0, 0, 0);
    }
    else if (ch->hunger >= -96 && ch->hunger <= -49)
    {
        if ((af = get_affect(ch, MAGIC_STARVE_ONE)))
            remove_affect_type(ch, MAGIC_STARVE_ONE);

        if ((af = get_affect(ch, MAGIC_STARVE_THREE)))
            remove_affect_type(ch, MAGIC_STARVE_THREE);

        if ((af = get_affect(ch, MAGIC_STARVE_FOUR)))
            remove_affect_type(ch, MAGIC_STARVE_FOUR);

        if (!(af = get_affect(ch, MAGIC_STARVE_TWO)))
            magic_add_affect (ch, MAGIC_STARVE_TWO, -1, 0, 0, 0, 0);
    }
    else if (ch->hunger >= -48 && ch->hunger <= -1)
    {
        if ((af = get_affect(ch, MAGIC_STARVE_TWO)))
            remove_affect_type(ch, MAGIC_STARVE_TWO);

        if ((af = get_affect(ch, MAGIC_STARVE_THREE)))
            remove_affect_type(ch, MAGIC_STARVE_THREE);

        if ((af = get_affect(ch, MAGIC_STARVE_FOUR)))
            remove_affect_type(ch, MAGIC_STARVE_FOUR);

        if (!(af = get_affect(ch, MAGIC_STARVE_ONE)))
            magic_add_affect (ch, MAGIC_STARVE_ONE, -1, 0, 0, 0, 0);
    }
    else if (ch->hunger >= 0)
    {
        if ((af = get_affect(ch, MAGIC_STARVE_ONE)))
            remove_affect_type(ch, MAGIC_STARVE_ONE);

        if ((af = get_affect(ch, MAGIC_STARVE_TWO)))
            remove_affect_type(ch, MAGIC_STARVE_TWO);

        if ((af = get_affect(ch, MAGIC_STARVE_THREE)))
            remove_affect_type(ch, MAGIC_STARVE_THREE);

        if ((af = get_affect(ch, MAGIC_STARVE_FOUR)))
            remove_affect_type(ch, MAGIC_STARVE_FOUR);
    }


}

char *
suffix (int number)
{
    if (number == 1)
        return "st";
    else if (number == 2)
        return "nd";
    else if (number == 3)
        return "rd";
    else if (number < 20)
        return "th";
    else if ((number % 10) == 1)
        return "st";
    else if ((number % 10) == 2)
        return "nd";
    else if ((number % 10) == 3)
        return "rd";
    else
        return "th";
}

void
do_info (CHAR_DATA * ch, char *argument, int cmd)
{
    int i = 0;
    int covered = 0;
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *af_table = NULL;
    CHAR_DATA *rch = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    bool first = true;

    send_to_char ("\n", ch);

    if (!IS_SET (ch->flags, FLAG_PACIFIST))
        sprintf (buf, "When in combat, your mode is #2%s#0.\n",
                 fight_tab[ch->fight_mode].name);
    else
        sprintf (buf, "You are currently in #2Pacifist#0 mode.\n");

    send_to_char (buf, ch);

    if (GET_FLAG (ch, FLAG_AUTOFLEE))
        send_to_char ("If in combat, you will #2FLEE#0.\n", ch);

    for (i = 0; i <= 3; i++)
        if (GET_STR (ch) * enc_tab[i].str_mult_wt >= IS_CARRYING_W (ch))
            break;

    sprintf (buf, "You are currently #2%s#0.\n", enc_tab[i].encumbered_status);
    send_to_char (buf, ch);


    int z = armor_descript(ch);
    int x = armor_penalty(ch);

    sprintf (buf, "By virtue of your equipment, you are #2%s#0 and your movement #2%s#0 hindered.\n",
    (z >= 4 ? "extremely well armoured" : z == 3 ? "heavily armoured" : z == 2 ? "well armoured" : z == 1 ? "lightly armoured" : "unarmoured"),
    (x == 3 ? "noticably" : z == 2 ? "lightly" : z == 1 ? "faintly" : "not at all"));
    send_to_char (buf, ch);

    switch (GET_POS (ch))
    {
    case POSITION_DEAD:
        send_to_char ("You are DEAD!\n", ch);
        break;
    case POSITION_MORTALLYW:
        send_to_char ("You are mortally wounded, and should seek help!\n", ch);
        break;
    case POSITION_UNCONSCIOUS:
        send_to_char ("You are unconscious.\n", ch);
        break;
    case POSITION_STUNNED:
        send_to_char ("You are stunned! You can't move!\n", ch);
        break;
    case POSITION_SLEEPING:
        af_table = get_affect (ch, MAGIC_SIT_TABLE);
        if (!af_table
                || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                    ch->room->contents))
            send_to_char ("You are sleeping.\n", ch);
        else
        {
            sprintf (buf, "You are sleeping at #2%s#0.\n",
                     obj_short_desc (af_table->a.table.obj));
            send_to_char (buf, ch);
        }
        break;
    case POSITION_RESTING:
        if ((covered = under_cover(ch)))
        {
            *buf2 = '\0';
            int j = 0;
            for (af = ch->hour_affects; af; af = af->next)
            {
                if (af->type == AFFECT_COVER)
                {
                    j++;

                    if (j == 1)
                        sprintf(buf2, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else if (j == covered)
                        sprintf(buf2 + strlen(buf2), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else
                        sprintf(buf2 + strlen(buf2), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                }
            }
            sprintf (buf, "You are resting behind cover from the%s.\n", buf2);
            *buf = toupper(*buf);
            send_to_char(buf, ch);
        }
        else
        {
            af_table = get_affect (ch, MAGIC_SIT_TABLE);
            if (!af_table
                    || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                        ch->room->contents))
                send_to_char ("You are resting.\n", ch);
            else if (af_table && af_table->a.table.obj)
            {
                sprintf (buf, "You are resting at #2%s#0.\n",
                         obj_short_desc (af_table->a.table.obj));
                send_to_char (buf, ch);
            }
        }
        break;
    case POSITION_SITTING:
        if ((covered = under_cover(ch)))
        {
            *buf2 = '\0';
            int j = 0;
            for (af = ch->hour_affects; af; af = af->next)
            {
                if (af->type == AFFECT_COVER)
                {
                    j++;

                    if (j == 1)
                        sprintf(buf2, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else if (j == covered)
                        sprintf(buf2 + strlen(buf2), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else
                        sprintf(buf2 + strlen(buf2), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                }
            }
            sprintf (buf, "You are taking cover from the%s.\n", buf2);
            *buf = toupper(*buf);
            send_to_char(buf, ch);
        }
        else
        {

            af_table = get_affect (ch, MAGIC_SIT_TABLE);
            if (!af_table
                    || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                        ch->room->contents))
                send_to_char ("You are sitting.\n", ch);
            else
            {
                sprintf (buf, "You are sitting at #2%s#0.\n",
                         obj_short_desc (af_table->a.table.obj));
                send_to_char (buf, ch);
            }

        }
        break;
    case POSITION_FIGHTING:
        if (ch->fighting && ch->fighting->short_descr)
        {
            send_to_char ("\n", ch);
            act ("You are fighting $N.", false, ch, 0, ch->fighting, TO_CHAR);

            for (rch = ch->room->people; rch; rch = rch->next_in_room)
            {
                if (rch->fighting == ch && rch != ch->fighting)
                    act ("You are also being attacked by $N.", false, ch, 0,
                         rch, TO_CHAR);
            }

            send_to_char ("\n", ch);
        }
        else
        {
            /* Was send_to_char 'You are fighting thin air' */
            stop_fighting (ch);
        }
        break;
    case POSITION_STANDING:
        send_to_char ("You are standing.\n", ch);
        break;
    default:
        send_to_char ("You are floating.\n", ch);
        break;
    }

    for (rch = ch->room->people; rch; rch = rch->next_in_room)
    {
        if ((af = get_affect (rch, MAGIC_GUARD)) && af->a.spell.t == (long int) ch)
        {
            if (first)
                send_to_char ("\n", ch);
            act ("You are being guarded by $N.", false, ch, 0, rch, TO_CHAR);
            first = false;
        }
    }

    if (ch->aim && ch->aiming_at)
    {
        act("You are aiming at $N.", false, ch, 0, ch->aiming_at, TO_CHAR);
        first = false;
    }

    for (rch = ch->room->people; rch; rch = rch->next_in_room)
    {
        if (rch->aim && rch->aiming_at == ch)
        {
            if (first)
                send_to_char ("\n", ch);
            act ("$N is aiming at you.", false, ch, 0, rch, TO_CHAR);
            first = false;
        }
    }

    if (first == false)
        send_to_char ("\n", ch);

    sprintf(buf, "You add overwatch for targets #2%s#0, and take aim at targets that are #2%s#0 to hit.",
           (ch->over_target == 0 ? "manually" : ch->over_target == 1 ? "as directed" : ch->over_target == 2 ? "that shoot at you" : "that shoot at your group"),
           (ch->over_thresh == 0 ? "easy" : ch->over_thresh == 1 ? "moderate" : "any chance"));
    act(buf, false, ch, 0, 0, TO_CHAR);

    if (IS_SET(ch->plr_flags, AUTO_FIRE))
	{
		send_to_char ("You automatically fire at targets you are aiming at it they try to move out of range.\n", ch);
	}

    if (IS_SET(ch->plr_flags, AUTO_COVER))
	{
		send_to_char ("If you come under fire, you will try to automatically take cover.\n", ch);
	}

    if (!IS_NPC (ch) && ch->pc->create_state == 4)
        send_to_char ("You are DEAD.\n", ch);

    if (get_affect (ch, AFFECT_GUARD_DIR))
    {
        sprintf (buf, "You are currently guarding the %s exit.\n",
                 dirs[get_affect (ch, AFFECT_GUARD_DIR)->a.shadow.edge]);
        send_to_char (buf, ch);
    }

    if (get_affect (ch, AFFECT_WATCH_DIR))
    {
        sprintf (buf, "You are currently watching the %s exit.\n",
                 dirs[get_affect (ch, AFFECT_WATCH_DIR)->a.shadow.edge]);
        send_to_char (buf, ch);
    }

    sprintf (buf, "You #2%s#0 when you travel.\n", speeds[ch->speed]);
    send_to_char (buf, ch);

    if (get_equip (ch, WEAR_BLINDFOLD))
        send_to_char ("YOU ARE BLINDFOLDED!\n", ch);
    else if (is_blind (ch))
        send_to_char ("YOU ARE BLIND!\n", ch);

    if (IS_SET (ch->affected_by, AFF_HOODED))
        send_to_char ("You are currently cloaked and hooded.\n", ch);

    if (ch->effort >= 1 && ch->effort <= 99)
    {
        sprintf(buf, "\nYou are currently fighting at #6%d percent#0 of your capability.\n", ch->effort);
        send_to_char(buf, ch);
    }
}

void
do_score (CHAR_DATA * ch, char *argument, int cmd)
{
    int i = 0;
    int covered = 0;
    int weight = 0;
    int time_diff = 0, days_remaining = 0, hours_remaining = 0;
    struct time_info_data playing_time;
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *af_table = NULL;
    CHAR_DATA *rch = NULL;
    OBJ_DATA *vobj = NULL;
    char *p = '\0';
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    bool first = true;
    struct time_info_data birth_date;
    static char *verbal_stats[] =
        { "horrible", "bad", "poor", "average", "good", "great", "peak", "superhuman", "legendary", "epic" };

    birth_date = time_info;

    birth_date.minute -= age (ch).minute;
    if (birth_date.minute < 0)
    {
        birth_date.hour -= 1;
        birth_date.minute += 60;
    }
    birth_date.hour -= age (ch).hour;
    if (birth_date.hour < 0)
    {
        birth_date.day -= 1;
        birth_date.hour += 24;
    }
    birth_date.day -= age (ch).day;
    if (birth_date.day <= 0)
    {
        birth_date.month -= 1;
        birth_date.day += 30;
    }
    birth_date.month -= age (ch).month;
    if (birth_date.month < 0)
    {
        birth_date.year -= 1;
        birth_date.month += 12;
    }
    birth_date.year -= age (ch).year;

    // Until people know how old they are, we'll conceal it from them.

    send_to_char ("\n", ch);
    if (!IS_SET (ch->flags, FLAG_GUEST))
        sprintf (buf, "#2%s, a %d year old %s %s:#0\n",
                 GET_NAME (ch), GET_AGE (ch), lookup_race_variable (ch->race,
                         RACE_NAME),
                 sex_types[ch->sex]);
    //else if (!IS_MORTAL(ch))
    //    sprintf (buf, "#2%s, a %s of unknown age.#0\n", GET_NAME(ch), sex_types[ch->sex]);
    else
        sprintf (buf, "#2Guest Login:#0 %s\n\n", ch->tname);

    send_to_char (buf, ch);

    // Aura has been removed from displaying for now.

    if (!IS_SET (ch->flags, FLAG_GUEST))
    {
        if (IS_MORTAL (ch))
            sprintf (buf,
                     "Str[#2%s#0] Dex[#2%s#0] Con[#2%s#0] Int[#2%s#0] Wil[#2%s#0] Agi[#2%s#0]\n",
                     verbal_stats[get_stat_range (GET_STR (ch))],
                     verbal_stats[get_stat_range (GET_DEX (ch))],
                     verbal_stats[get_stat_range (GET_CON (ch))],
                     verbal_stats[get_stat_range (GET_INT (ch))],
                     verbal_stats[get_stat_range (GET_WIL (ch))],
                     verbal_stats[get_stat_range (GET_AGI (ch))]);
        else
            sprintf (buf,
                     "Str[#2%d#0] Dex[#2%d#0] Con[#2%d#0] Int[#2%d#0] Wil[#2%d#0] Mut[#2%d#0] Agi[#2%d#0]\n",
                     GET_STR (ch), GET_DEX (ch), GET_CON (ch), GET_INT (ch),
                     GET_WIL (ch), GET_AUR (ch), GET_AGI (ch));

        send_to_char ("\n", ch);
        send_to_char (buf, ch);
    }

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        sprintf (buf, "You have been incarnated as #5%s#0.\n\n",
                 char_short (ch));
        send_to_char (buf, ch);
        sprintf (buf, "You are a #2%s#0 #2%s#0.\n", sex_types[ch->sex],
                 lookup_race_variable (ch->race, RACE_NAME));
        send_to_char (buf, ch);
    }

    int acct_rpp = 0;
    if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST) && ch->descr()
            && ch->descr()->acct && (acct_rpp = ch->descr()->acct->get_rpp ()) > 0
            && IS_SET (ch->descr()->acct->flags, ACCOUNT_RPPDISPLAY))
    {
        sprintf (buf, "Your account has been awarded #2%d#0 reward points.\n",
                 acct_rpp);
        send_to_char (buf, ch);
    }

    if (ch->descr() && ch->descr()->acct && ch->descr()->acct->get_last_nominate () > 0)
    {
        std::string date = ch->descr()->acct->get_nominate_rpp_date_string ();
        sprintf (buf, "You last made a nomination on: #2%s#0\n", date.c_str ());
        send_to_char(buf, ch);
    }

    /* Add support for listing, hunger, thirst, and intox. */
    sprintf (buf, "You are #2%s#0, and #2%s#0.\n",
             ch->hunger >=
             -169 ? verbal_hunger[get_hunger_comestible_range (ch->hunger)] : "full",
             ch->thirst >=
             0 ? verbal_thirst[get_comestible_range (ch->thirst)] : "quenched");
    send_to_char (buf, ch);

    if (!IS_SET (ch->flags, FLAG_GUEST))
    {
        sprintf (buf, "You are #2%d#0 inches tall, and weigh #2%d#0 pounds.\n",
                 ch->height, get_weight (ch) / 100);
        send_to_char (buf, ch);

        sprintf (buf,
                 "You are of #2%s#0 build for one of your people, and wear size #2%s#0 (%s).\n",
                 frames[ch->frame], sizes_named[get_size (ch)],
                 sizes[get_size (ch)]);
        send_to_char (buf, ch);
    }

    if (!IS_SET (ch->flags, FLAG_PACIFIST))
        sprintf (buf, "When in combat, your mode is #2%s#0.\n",
                 fight_tab[ch->fight_mode].name);
    else
        sprintf (buf, "You are currently in #2Pacifist#0 mode.\n");

    send_to_char (buf, ch);

    if (GET_FLAG (ch, FLAG_AUTOFLEE))
        send_to_char ("If in combat, you will #2FLEE#0.\n", ch);

    // Until we have more than one language, why bother knowing?

    if (!IS_MORTAL(ch))
    {

        sprintf (buf, "You are currently speaking #2%s#0.\n", skills[ch->speaks]);
        send_to_char (buf, ch);

        if (ch->writes)
        {
            sprintf (buf, "You are currently set to write in #2%s#0.\n",
                     skills[ch->writes]);
            send_to_char (buf, ch);
        }
    }

    for (i = 0; i <= 3; i++)
        if (GET_STR (ch) * enc_tab[i].str_mult_wt >= IS_CARRYING_W (ch))
            break;

    sprintf (buf, "You are currently #2%s#0.\n", enc_tab[i].encumbered_status);
    send_to_char (buf, ch);

    weight = IS_CARRYING_W (ch);

    if (weight < 1000)
        weight = (weight + 50) / 100 * 100;
    else if (weight < 10000)
        weight = (weight + 500) / 1000 * 1000;
    else
        weight = (weight + 1250) / 2500 * 2500;

    if (IS_CARRYING_W (ch) == 0)
        send_to_char ("You are carrying nothing.\n", ch);
    else
    {
        sprintf (buf, "You are carrying about #2%d#0 pounds.\n", weight / 100);
        send_to_char (buf, ch);
    }

    // Until people know how old they are, we'll conceal it from them.

    if (!IS_SET (ch->flags, FLAG_GUEST) && !IS_MORTAL(ch))
    {
        if (birth_date.year > 0)
            sprintf (buf,
                     "You were born on the #2%s#0, #2%d#0.\n",
                     short_time_string(birth_date.day, birth_date.month), birth_date.year);
        else
            sprintf (buf,
                     "You were born on the #2%s#0, many millenia past.\n",
                     short_time_string(birth_date.day, birth_date.month));
        send_to_char (buf, ch);
    }

    playing_time = real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
    sprintf (buf, "You have been playing for #2%d#0 days and #2%d#0 hours.\n", playing_time.day, playing_time.hour);
    send_to_char (buf, ch);

    switch (GET_POS (ch))
    {
    case POSITION_DEAD:
        send_to_char ("You are DEAD!\n", ch);
        break;
    case POSITION_MORTALLYW:
        send_to_char ("You are mortally wounded, and should seek help!\n", ch);
        break;
    case POSITION_UNCONSCIOUS:
        send_to_char ("You are unconscious.\n", ch);
        break;
    case POSITION_STUNNED:
        send_to_char ("You are stunned! You can't move!\n", ch);
        break;
    case POSITION_SLEEPING:
        af_table = get_affect (ch, MAGIC_SIT_TABLE);
        if (!af_table
                || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                    ch->room->contents))
            send_to_char ("You are sleeping.\n", ch);
        else
        {
            sprintf (buf, "You are sleeping at #2%s#0.\n",
                     obj_short_desc (af_table->a.table.obj));
            send_to_char (buf, ch);
        }
        break;
    case POSITION_RESTING:
        if ((covered = under_cover(ch)))
        {
            *buf2 = '\0';
            int j = 0;
            for (af = ch->hour_affects; af; af = af->next)
            {
                if (af->type == AFFECT_COVER)
                {
                    j++;

                    if (j == 1)
                        sprintf(buf2, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else if (j == covered)
                        sprintf(buf2 + strlen(buf2), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else
                        sprintf(buf2 + strlen(buf2), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                }
            }
            sprintf (buf, "You are resting behind cover from the%s.\n", buf2);
            *buf = toupper(*buf);
            send_to_char(buf, ch);
        }
        else
        {
            af_table = get_affect (ch, MAGIC_SIT_TABLE);
            if (!af_table
                    || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                        ch->room->contents))
                send_to_char ("You are resting.\n", ch);
            else if (af_table && af_table->a.table.obj)
            {
                sprintf (buf, "You are resting at #2%s#0.\n",
                         obj_short_desc (af_table->a.table.obj));
                send_to_char (buf, ch);
            }
        }
        break;
    case POSITION_SITTING:
        if ((covered = under_cover(ch)))
        {
            *buf2 = '\0';
            int j = 0;
            for (af = ch->hour_affects; af; af = af->next)
            {
                if (af->type == AFFECT_COVER)
                {
                    j++;

                    if (j == 1)
                        sprintf(buf2, " %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else if (j == covered)
                        sprintf(buf2 + strlen(buf2), " and %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                    else
                        sprintf(buf2 + strlen(buf2), ", %s", (af->a.cover.direction == 6 ? "area" : dirs[af->a.cover.direction]));
                }
            }
            sprintf (buf, "You are taking cover from the%s.\n", buf2);
            *buf = toupper(*buf);
            send_to_char(buf, ch);
        }
        else
        {

            af_table = get_affect (ch, MAGIC_SIT_TABLE);
            if (!af_table
                    || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
                                        ch->room->contents))
                send_to_char ("You are sitting.\n", ch);
            else
            {
                sprintf (buf, "You are sitting at #2%s#0.\n",
                         obj_short_desc (af_table->a.table.obj));
                send_to_char (buf, ch);
            }

        }
        break;
    case POSITION_FIGHTING:
        if (ch->fighting && ch->fighting->short_descr)
        {
            send_to_char ("\n", ch);
            act ("You are fighting $N.", false, ch, 0, ch->fighting, TO_CHAR);

            for (rch = ch->room->people; rch; rch = rch->next_in_room)
            {
                if (rch->fighting == ch && rch != ch->fighting)
                    act ("You are also being attacked by $N.", false, ch, 0,
                         rch, TO_CHAR);
            }

            send_to_char ("\n", ch);
        }
        else
        {
            /* Was send_to_char 'You are fighting thin air' */
            stop_fighting (ch);
        }
        break;
    case POSITION_STANDING:
        send_to_char ("You are standing.\n", ch);
        break;
    default:
        send_to_char ("You are floating.\n", ch);
        break;
    }

    for (rch = ch->room->people; rch; rch = rch->next_in_room)
    {
        if ((af = get_affect (rch, MAGIC_GUARD)) && af->a.spell.t == (long int) ch)
        {
            if (first)
                send_to_char ("\n", ch);
            act ("You are being guarded by $N.", false, ch, 0, rch, TO_CHAR);
            first = false;
        }
    }

    if (first == false)
        send_to_char ("\n", ch);

    if (!IS_NPC (ch) && ch->pc->create_state == 4)
        send_to_char ("You are DEAD.\n", ch);

    if (get_affect (ch, AFFECT_GUARD_DIR))
    {
        sprintf (buf, "You are currently guarding the %s exit.\n",
                 dirs[get_affect (ch, AFFECT_GUARD_DIR)->a.shadow.edge]);
        send_to_char (buf, ch);
    }

    if (get_affect (ch, AFFECT_WATCH_DIR))
    {
        sprintf (buf, "You are currently watching the %s exit.\n",
                 dirs[get_affect (ch, AFFECT_WATCH_DIR)->a.shadow.edge]);
        send_to_char (buf, ch);
    }

    sprintf (buf, "You #2%s#0 when you travel.\n", speeds[ch->speed]);
    send_to_char (buf, ch);

    if (IS_SET (ch->plr_flags, GROUP_CLOSED))
        send_to_char ("Your group is currently #2closed#0 to new members.\n", ch);
    else
        send_to_char ("Your group is currently #2open#0 to new members.\n", ch);

    if ((af = get_affect (ch, MAGIC_CRAFT_DELAY))
            && time (0) < af->a.spell.modifier)
    {
        time_diff = af->a.spell.modifier - time (0);
        days_remaining = time_diff / (60 * 60 * 24);
        time_diff %= 60 * 60 * 24;
        hours_remaining = time_diff / (60 * 60);
        if (!days_remaining && !hours_remaining)
        {
            sprintf (buf,
                     "Your craft timer has #2less than 1#0 RL hour remaining.\n");
        }
        else
        {
            sprintf (buf, "Your craft timer has approximately");
            if (days_remaining)
                sprintf (buf + strlen (buf), " #2%d#0 RL day%s ", days_remaining,
                         days_remaining > 1 ? "s" : "");
            if (hours_remaining && days_remaining)
                sprintf (buf + strlen (buf), "and");
            if (hours_remaining)
                sprintf (buf + strlen (buf), " #2%d#0 RL hour%s ",
                         hours_remaining, hours_remaining > 1 ? "s" : "");
            sprintf (buf + strlen (buf), "remaining.\n");
        }
        send_to_char (buf, ch);
    }



    if (!IS_MORTAL (ch))
    {
        if (!ch->hour_affects)
            send_to_char ("No spells affect you.\n", ch);
        else
        {
            for (af = ch->hour_affects; af; af = af->next)
            {


                if (af->type == AFFECT_SHADOW)
                {

                    if (!af->a.shadow.shadow && af->a.shadow.edge != -1)
                        sprintf (buf, "   Standing");

                    else if (!is_he_somewhere (af->a.shadow.shadow))
                        continue;

                    else if (IS_NPC (af->a.shadow.shadow))
                        sprintf (buf, "   Shadowing %s (%d)",
                                 af->a.shadow.shadow->short_descr,
                                 af->a.shadow.shadow->mob->vnum);
                    else
                        sprintf (buf, "   Shadowing PC %s",
                                 GET_NAME (af->a.shadow.shadow));

                    if (af->a.shadow.edge != -1)
                        sprintf (buf + strlen (buf), " on %s edge.",
                                 dirs[af->a.shadow.edge]);

                    strcat (buf, "\n");

                    send_to_char (buf, ch);

                    continue;
                }

                else if (af->type == MAGIC_SIT_TABLE)
                {
                    sprintf (buf, "   Sitting at table affect.\n");
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type >= JOB_1 && af->type <= JOB_3)
                {

                    i =
                        time_info.year * 12 * 30 + time_info.month * 30 +
                        time_info.day;
                    i = af->a.job.pay_date - i;

                    vobj = vtoo (af->a.job.object_vnum);

                    sprintf (buf,
                             "   Job %d:  %d of %d days until payday\n",
                             af->type - JOB_1 + 1,
                             i,
                             af->a.job.days);

                    sprintf (buf + strlen(buf),
                             "             %d x %s",
                             af->a.job.count,
                             !vobj ? "UNDEFINED" : vobj->short_description
                            );

                    if (af->a.job.employer)
                    {
                        sprintf (buf + strlen(buf),
                                 "\n             Employer: %s \n",
                                 vnum_to_mob(af->a.job.employer)->short_descr);
                    }
                    else
                    {
                        sprintf (buf+ strlen(buf),"\n");
                    }

                    send_to_char (buf, ch);
                    continue;
                } //if (af->type >= JOB_1


                if (af->type >= MAGIC_CRIM_BASE &&
                        af->type <= MAGIC_CRIM_BASE + 100)
                {
                    sprintf (buf, "   Wanted in zone %d for %d hours.\n",
                             af->type - MAGIC_CRIM_BASE, af->a.spell.duration);
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type >= MAGIC_CRIM_HOODED &&
                        af->type < MAGIC_CRIM_HOODED + 100)
                {
                    sprintf (buf, "   Hooded criminal charge in zone %d for "
                             "%d RL seconds.\n",
                             af->type - MAGIC_CRIM_HOODED,
                             af->a.spell.duration);
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type == MAGIC_STARED)
                {
                    sprintf (buf, "%d   Studied by an enforcer.  Won't be "
                             "studied for %d RL seconds.\n",
                             af->type, af->a.spell.duration);
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type == MAGIC_ALERTED)
                {
                    sprintf (buf, "%d   Gave out an alert.  Won't be "
                             "alerting again for %d RL seconds.\n",
                             af->type, af->a.spell.duration);
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
                    continue;

                if (af->type == MAGIC_CRAFT_DELAY || AFFECT_UPGRADE_DELAY || MAGIC_CRAFT_BRANCH_STOP)
                    continue;

                if (af->type == MAGIC_HIDDEN) /// \todo Check this as dead code
                {
                    sprintf (buf, "%d   Hidden.\n", af->type);
                    send_to_char (buf, ch);
                    continue;
                }

                if (af->type == MAGIC_SNEAK)
                {
                    sprintf (buf, "%d   Currently trying to sneak.\n",
                             af->type);
                    send_to_char (buf, ch);
                    continue;
                }

                p = lookup_string (af->type, REG_SPELLS);

                if (!p)
                    p = lookup_string (af->type, REG_MAGIC_SPELLS);

                sprintf (buf, "   %s affects you by %d for %d hours.\n",
                         p == NULL ? "Unknown" : p,
                         af->a.spell.modifier, af->a.spell.duration);
                send_to_char (buf, ch);
            }
        }
    }

    if (get_equip (ch, WEAR_BLINDFOLD))
        send_to_char ("YOU ARE BLINDFOLDED!\n", ch);
    else if (is_blind (ch))
        send_to_char ("YOU ARE BLIND!\n", ch);

    if (IS_SET (ch->affected_by, AFF_HOODED))
        send_to_char ("You are currently cloaked and hooded.\n", ch);

    if (ch->voice_str)
    {
        send_to_char ("\nYour current voice string: (#2", ch);
        send_to_char (ch->voice_str, ch);
        send_to_char ("#0)\n", ch);
    }

    if (ch->travel_str)
    {
        if (!ch->voice_str)
            send_to_char ("\n", ch);
        send_to_char ("Your current travel string: (#2", ch);
        send_to_char (ch->travel_str, ch);
        send_to_char ("#0)\n", ch);
    }

    if (ch->pmote_str)
    {
        if (!ch->voice_str && !ch->travel_str)
            send_to_char ("\n", ch);
        send_to_char ("Your current pmote: (#2", ch);
        send_to_char (ch->pmote_str, ch);
        send_to_char ("#0)\n", ch);
    }

    if (ch->dmote_str)
    {
        if (!ch->voice_str && !ch->travel_str && !ch->pmote_str)
            send_to_char ("\n", ch);
        send_to_char ("Your current dmote:\n", ch);
        send_to_char (ch->dmote_str, ch);
        send_to_char ("\n", ch);
    }

    if (ch->effort >= 1 && ch->effort <= 99)
    {
        sprintf(buf, "\nYou are currently fighting at #6%d percent#0 of your capability.\n", ch->effort);
        send_to_char(buf, ch);
    }

    clan__do_score (ch);
}

void
do_skills (CHAR_DATA * ch, char *argument, int cmd)
{
    int i = 0;
    int j = 2;
    int loaded_char = 0;
    CHAR_DATA *who = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    AFFECTED_TYPE *af;

    argument = one_argument (argument, buf);

    if (IS_MORTAL (ch) || !*buf)
        who = ch;
    else if (!(who = get_char_room_vis (ch, buf)))
    {
        if (!(who = load_pc (buf)))
        {
            send_to_char ("No body here by that name.\n", ch);
            return;
        }
        else
            loaded_char = 1;
    }

    *buf = '\0';

    if (IS_MORTAL (ch))
    {

        sprintf (buf, "\n#2%12s:#0 %s ", "Prowess", skill_lev (who->offense));
        while (strlen (buf) < 32)
            strcat (buf, " ");

        for (i = 1; i <= LAST_SKILL; i++)
        {

            // Don't show up skills less that are 10 or less, so people don't have a screen
            // cluttered by a heap of 1 and 2 point skills.

            if (!real_skill (ch, i) || (ch->skills[i] < 11))
                continue;

            if (IS_MORTAL(ch) && i == SKILL_COMMON)
                continue;

            if ((af = get_affect (ch, MAGIC_FLAG_FOCUS + i)))
            {
                sprintf (buf2, "#2%12s:#0#6%s#0 ", skills[i], skill_lev2 (who->skills[i]));
                while (strlen (buf2) < 35)
                    strcat (buf2, " ");
            }
            else if ((af = get_affect (ch, MAGIC_FLAG_IGNORE + i)))
            {
                sprintf (buf2, "#2%12s:#0#5%s#0 ", skills[i], skill_lev3 (who->skills[i]));

                while (strlen (buf2) < 35)
                    strcat (buf2, " ");
            }
            else
            {
                sprintf (buf2, "%s%12s:#0 %s ", (ch->skills[i] == calc_lookup (ch, REG_CAP, i) ? "#3" : "#2"), skills[i], skill_lev (who->skills[i]));
                while (strlen (buf2) < 31)
                    strcat (buf2, " ");
            }



            strcat (buf, buf2);

            if (!(j % 3))
                strcat (buf, "\n");
            j++;
        }
    }

    else
    {
        sprintf (buf, " #2Prowess#0[%03d/%03d] ", who->offense, 60);

        for (i = 1; i <= LAST_SKILL; i++)
        {
            if (real_skill (who, i))
            {
                sprintf (buf + strlen (buf),
                         "#2%8.8s#0[%03d/%03d] ", skills[i], who->skills[i],
                         calc_lookup (who, REG_CAP, i));
                if (!(j % 4))
                    strcat (buf, "\n");
                j++;
            }
        }
    }

    if (skill_max(ch, 0, 0))
    {
        if (buf[strlen (buf) - 1] != '\n')
            strcat (buf, "\n");

        sprintf (buf2, "\n               #6You have reached your full skill potential.#0\n");
        strcat (buf, buf2);
    }

    send_to_char (buf, ch);

    if (buf[strlen (buf) - 1] != '\n')
        send_to_char ("\n", ch);

    if (loaded_char)
        unload_pc (who);
}

void
do_talents (CHAR_DATA * ch, char *argument, int cmd)
{
    int loaded_char = 0;
    CHAR_DATA *who = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    //bool earned = false;

    argument = one_argument (argument, buf);

    if (IS_MORTAL (ch) || !*buf)
        who = ch;
    else if (!(who = get_char_room_vis (ch, buf)))
    {
        if (!(who = load_pc (buf)))
        {
            send_to_char ("No body here by that name.\n", ch);
            return;
        }
        else
            loaded_char = 1;
    }

    *buf = '\0';

    sprintf(buf, "#2   Talents Earned:#0\n");

    if (ch->talents <= 0)
    {
        sprintf(buf + strlen(buf), "      None!\n");
    }
    else
    {
        if (IS_SET(ch->talents, TAL_STR1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Strength\n");
        if (IS_SET(ch->talents, TAL_DEX1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Dexterity\n");
        if (IS_SET(ch->talents, TAL_CON1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Constitution\n");
        if (IS_SET(ch->talents, TAL_INT1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Intelligence\n");
        if (IS_SET(ch->talents, TAL_WIL1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Willpower\n");
        if (IS_SET(ch->talents, TAL_AUR1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Aura\n");
        if (IS_SET(ch->talents, TAL_AGI1))
            sprintf(buf + strlen(buf), "      #2Upgraded Attribute:#0 Agility\n");
    }

    send_to_char (buf, ch);

    if (buf[strlen (buf) - 1] != '\n')
        send_to_char ("\n", ch);

    if (loaded_char)
        unload_pc (who);
}


char * time_string( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char suf[4] = { '\0' };
    int day = 0;
    int minutes = 0;
    //int nCharAstronomySkill = 0;
    char day_buf[AVG_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    static char time_str[MAX_STRING_LENGTH] = { '\0' };

    minutes = 4 * (15 * 60 - (next_hour_update - time (0))) / 60;

    sprintf (phrase, "[report error: %d]", time_info.hour);

    /* Astronomy skill gives knowledge of more precise time
    if (ch && ch->skills[SKILL_ASTRONOMY])
    {

        nCharAstronomySkill = ch->skills[SKILL_ASTRONOMY];
        if (!IS_OUTSIDE (ch) || (IS_NIGHT && !moon_light[ch->room->zone]))
        {
            nCharAstronomySkill -= 40;
        }
        else
        {
            nCharAstronomySkill -= (10 * weather_info[ch->room->zone].clouds);
            nCharAstronomySkill -= (2 * weather_info[ch->room->zone].state);
        }

        if (nCharAstronomySkill >= 70)
        {
            if (minutes < 7)
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else if (minutes < 23)
            {
                sprintf (phrase + strlen (phrase), ", quarter-past %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else if (minutes < 37)
            {
                sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else if (minutes < 52)
            {
                sprintf (phrase + strlen (phrase), ", quarter-to %s o'clock,",
                         strTimeWord[(time_info.hour + 1) % 24]);
            }
            else
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[(time_info.hour + 1) % 24]);
            }
        }

        else if (nCharAstronomySkill >= 50)
        {
            if (minutes < 15)
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else if (minutes < 45)
            {
                sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[(time_info.hour + 1) % 24]);
            }
        }

        else if (nCharAstronomySkill >= 30)
        {
            if (minutes < 30)
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[time_info.hour]);
            }
            else
            {
                sprintf (phrase + strlen (phrase), ", about %s o'clock,",
                         strTimeWord[(time_info.hour + 1) % 24]);
            }
        }

    }
    */

    sprintf (buf, "It is %s, %s ", sun_phase[time_info.phaseSun], earth_phase[time_info.phaseEarth]);

    if (ch && !IS_MORTAL (ch))
    {
        sprintf (buf + strlen (buf), "[%d:%s%d]  ", time_info.hour, minutes >= 10 ? "" : "0", minutes);
		sprintf (buf + strlen (buf), "[real_hour:%d], [day: %d], [month: %d], [phaseEarth: %d]",  time_info.hour + (84 * time_info.day), time_info.day, time_info.month, time_info.phaseEarth);
    }

    day = time_info.month + 1;	/* day in [1..12] */

    if (day == 1)
        strcpy (suf, "st");
    else if (day == 2)
        strcpy (suf, "nd");
    else if (day == 3)
        strcpy (suf, "rd");
    else if (day < 20)
        strcpy (suf, "th");
    else if ((day % 10) == 1)
        strcpy (suf, "st");
    else if ((day % 10) == 2)
        strcpy (suf, "nd");
    else if ((day % 10) == 3)
        strcpy (suf, "rd");
    else
        strcpy (suf, "th");

    sprintf (day_buf, "%d%s", day, suf);


    sprintf (buf + strlen (buf), "on the %s Lunar day,", day_buf);

    /*
    if (time_info.holiday == 0 &&
        !(time_info.month == 1 && day == 12) &&
        !(time_info.month == 4 && day == 10) &&
        !(time_info.month == 7 && day == 11) &&
        !(time_info.month == 10 && day == 12))

    else
      {
        if (time_info.holiday > 0)
      {
        sprintf (buf + strlen (buf), "on %s,",
      	   holiday_names[time_info.holiday]);
      }
        else if (time_info.month == 1 && day == 12)
      sprintf (buf + strlen (buf), "on Erukyerme, The Prayer to Eru,");
        else if (time_info.month == 4 && day == 10)
      sprintf (buf + strlen (buf), "on Lairemerende, The Greenfest,");
        else if (time_info.month == 7 && day == 11)
      sprintf (buf + strlen (buf), "on Eruhantale, Thanksgiving to Eru,");
        else if (time_info.month == 10 && day == 12)
      sprintf (buf + strlen (buf), "on Airilaitale, The Hallowmas,");
      }
    */

    sprintf (buf + strlen (buf), " %d years since The Liberation.\n", time_info.year);

    sprintf (time_str, "%s", buf);

    return time_str;
}

void do_time ( CHAR_DATA * ch, char *argument, int cmd )
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char *p;
    //int day = time_info.day + 1, d_day = 0, moon_q = 0, moon_r = 0, moon_s = 0;
    /*static char *strRelativeTime[] = {
      "morning", "morning", "morning", "morning", "afternoon", "afternoon",
      "evening", "night"
    };*/



    sprintf( buf, "\n#6%s#0", time_string( ch ) );
    reformat_string (buf, &p);

    send_to_char (buf, ch);

    mem_free (p); //char*
    p = NULL;

    if ( GET_TRUST( ch ) )
    {
        send_to_char ("\nThe following variables apply:\n", ch);

        if (IS_LIGHT (ch->room))
            send_to_char ("   IS_LIGHT is true.\n", ch);
        else
            send_to_char ("   IS_LIGHT is false.\n", ch);

        if (IS_OUTSIDE (ch))
            send_to_char ("   OUTSIDE is true.\n", ch);
        else
            send_to_char ("   OUTSIDE is false.\n", ch);

        if (IS_NIGHT)
            send_to_char ("   IS_NIGHT is true.\n", ch);
        else
            send_to_char ("   IS_NIGHT is false.\n", ch);

        send_to_char ((global_moon_light) ? "   Global Moon is true.\n" :
                      "   Global Moon is false.\n", ch);
        send_to_char ((moon_light[ch->room->zone]) ? "   Zone Moon is true.\n" :
                      "   Zone Moon is false.\n", ch);

        sprintf (buf, "   Light count in room:  %d\n", ch->room->light);
        send_to_char (buf, ch);
    }
}


void do_weather (CHAR_DATA * ch, char *argument, int cmd)
{
    int ind = 0;
    char w_phrase[50] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char imm_buf[MAX_STRING_LENGTH];
    int wind_case = 0;
    int temp_case = 0;
    char wind[20] = { '\0' };

    argument = one_argument (argument, buf);

    if (!IS_MORTAL (ch) && GET_TRUST (ch) > 4 && *buf)
    {
        if ((ind = index_lookup (weather_states, buf)) != -1)
        {
            sprintf (buf, "You have changed the weather state to #6%s#0.\n", weather_states[ind]);
            send_to_char (buf, ch);
            weather_info[ch->room->zone].state = ind;
            return;
        }
        /*
        else if ((ind = index_lookup (weather_clouds, buf)) != -1)
        {
            sprintf (buf, "You have changed cloud state to #6%s#0.\n",
                     weather_clouds[ind]);
            send_to_char (buf, ch);
            weather_info[ch->room->zone].clouds = ind;
            return;
        }
        */
        else if ((ind = index_lookup (wind_speeds, buf)) != -1)
        {
            sprintf (buf, "You have changed wind speed to #6%s#0.\n",
                     wind_speeds[ind]);
            send_to_char (buf, ch);
            weather_info[ch->room->zone].wind_speed = ind;
            return;
        }
        else if ((ind = index_lookup (fog_states, buf)) != -1)
        {
            sprintf (buf, "You have changed the fog level to #6%s#0.\n",
                     fog_states[ind]);
            send_to_char (buf, ch);
            weather_info[ch->room->zone].fog = ind;
            return;
        }

        send_to_char ("That is not a recognized weather state.\n", ch);
        return;
    }

        if (weather_info[ch->room->zone].temperature < 120)
        {
            wind_case = 0;
            temp_case = 1;
        }
        if (weather_info[ch->room->zone].temperature < 110)
        {
            temp_case = 2;
            wind_case = 1;
        }
        if (weather_info[ch->room->zone].temperature < 100)
        {
            wind_case = 2;
            temp_case = 3;
        }
        if (weather_info[ch->room->zone].temperature < 94)
        {
            wind_case = 3;
            temp_case = 3;
        }
        if (weather_info[ch->room->zone].temperature < 90)
            temp_case = 4;
        if (weather_info[ch->room->zone].temperature < 80)
        {
            wind_case = 4;
            temp_case = 5;
        }
        if (weather_info[ch->room->zone].temperature < 75)
            temp_case = 5;
        if (weather_info[ch->room->zone].temperature < 65)
        {
            wind_case = 5;
            temp_case = 6;
        }
        if (weather_info[ch->room->zone].temperature < 55)
        {
            temp_case = 7;
            wind_case = 6;
        }
        if (weather_info[ch->room->zone].temperature < 47)
        {
            wind_case = 7;
            temp_case = 8;
        }
        if (weather_info[ch->room->zone].temperature < 38)
            temp_case = 9;
        if (weather_info[ch->room->zone].temperature < 33)
        {
            wind_case = 8;
            temp_case = 10;
        }
        if (weather_info[ch->room->zone].temperature < 21)
        {
            wind_case = 9;
            temp_case = 11;
        }
        if (weather_info[ch->room->zone].temperature < 11)
            temp_case = 12;
        if (weather_info[ch->room->zone].temperature < 1)
        {
            wind_case = 10;
            temp_case = 13;
        }
        if (weather_info[ch->room->zone].temperature < -10)
            temp_case = 14;

        *buf = '\0';
        *buf2 = '\0';

        sprintf (w_phrase, "%s", sun_phase[time_info.phaseSun]);

        *imm_buf = '\0';

        if (!IS_MORTAL (ch))
            sprintf (imm_buf, " [%d F]",
                     weather_info[ch->room->zone].temperature);


        sprintf(buf, "It is a %s %s%s, and ", temp_phrase[temp_case], w_phrase, imm_buf);

        if (weather_info[ch->room->zone].wind_speed)
        {
            if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
                wind_case++;

            if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
                sprintf (wind, "%s northerly", wind_temp_phrase[wind_case]);
            else
                sprintf (wind, "%s westerly", wind_temp_phrase[wind_case]);
        }

        switch (weather_info[ch->room->zone].wind_speed)
        {
        case CALM:
            sprintf (buf + strlen(buf), "the air is calm and quiet.\n\n");
            break;
        case BREEZE:
            sprintf (buf + strlen(buf), "there is a %s breeze.\n\n", wind);
            break;
        case WINDY:
            sprintf (buf + strlen(buf), "there is a %s wind.\n\n", wind);
			break;
		}
		send_to_char (buf, ch);

		*buf = '\0';
		*buf2 = '\0';

		switch (time_info.phaseEarth)
		{
		case PHASE_FULL_EARTH:
			sprintf(buf, "Against the backdrop of a star-scattered eternal space, the massive form of the Earth is now in full view, its marred surface reflecting an eerie bright white glow upon the moonscape.");
			break;
		case PHASE_GIBBOUS_WANING:
			sprintf(buf, "In the velvety, star-scattered night sky, the majestic Earth reflects an eerie white glow upon the surface of the Moon, though a thin sliver is overcome by darkness.");
			break;
		case PHASE_THREE_QUARTER:
			sprintf(buf, "The golden rays of the Sun begin to kiss the Moon's surface as its giant fiery form ascends over the horizon. A large portion of the majestic Earth is plunged into darkness with only a moderate crescent of silvery white remaining.");
			break;
		case PHASE_CRESCENT_WANING:
			sprintf(buf, "The Earth, now only a very thin sliver of silvery white, is plunged nearly entirely into darkness against an eternal backdrop of twinkling stars. The fiery giant that is the Sun now begins to mercilessly cast its intense heat and light upon the surface of the Moon.");
			break;
		case PHASE_NEW_EARTH:
			sprintf(buf, "The Sun has now reached its highest point in the sky, its omnipresent form casting blistering heat and intense light mercilessly onto the surface of the Moon. The majestic earth is now cast entirely in darkness, barely visible.");
			break;
		case PHASE_CRESCENT_WAXING:
			sprintf(buf, "The first signs of light have begun to creep over the surface of the Earth in the sky, giving it a silvery white glow in the form of a large sickle in the sky. The Sun's heat continues to beat mercilessly down upon the Moon, though its position in the sky has lowered.");
			break;
		case PHASE_FIRST_QUARTER:
			sprintf(buf, "The fiery giant that is the sun has now begun its descent across the Moon sky, casting the surroundings in a golden dusk. A quarter of the majestic Earth is now cast in a silvery reflective light against a backdrop of eternal space and stars.");
			break;
		default:
			sprintf(buf, "Against a backdrop of eternal space and glittering stars, the majestic Earth is now cast mostly in a silvery white, casting an eerie glow upon the surface of the cold Moon.");
			break;
		}

		char *p = '\0';
		reformat_string (buf, &p);
		sprintf (buf, "%s", p);
		mem_free (p); // char*
		send_to_char(buf, ch);

        /*
        if (weather_info[ch->room->zone].fog
                && !(weather_info[ch->room->zone].state >= LIGHT_RAIN
                     && weather_info[ch->room->zone].fog == THICK_FOG))
        {
            if (weather_info[ch->room->zone].fog == THIN_FOG)
                send_to_char ("A patchy fog floats in the air.\n", ch);
            else
                send_to_char ("A thick fog lies heavy upon the land.\n", ch);
        }


        if ((room_af = is_room_affected (world_affects, MAGIC_WORLD_CLOUDS)) &&
                (IS_OUTSIDE (ch) || !IS_MORTAL (ch)))
        {
            send_to_char
            ("Looming black clouds cover the sky, blotting out the sun.\n",
             ch);
        }

        if (moon_light[ch->room->zone] >= 1)
        {
            if (!sun_light)
                send_to_char
                ("A full and gleaming moon limns the area in ghostly argent radiance.\n",
                 ch);
            else
                send_to_char
                ("The moon's ethereal silhouette is barely visible in the daylight.\n",
                 ch);
        }
		*/
}

HELP_DATA *
is_help (CHAR_DATA * ch, HELP_DATA * list, char *topic)
{
    HELP_DATA *element = NULL;

    for (element = list; element; element = element->next)
    {

        if (!element->master_element)
            continue;

        if (strcasecmp (element->keyword, topic) == STR_MATCH)
            return element->master_element;
    }

    return NULL;
}

void
post_help (DESCRIPTOR_DATA * d)
{
    char date[32] = "";
    time_t current_time = 0;

    mysql_safe_query
    ("DELETE FROM helpfiles WHERE category = \'%s\' AND name = \'%s\'",
     d->character->delay_who, d->character->delay_who2);

    current_time = time (0);
    ctime_r (&current_time, date);
    if (strlen (date) > 1)
        date[strlen (date) - 1] = '\0';

    if (!*d->pending_message->message)
    {
        send_to_char ("No help entry written.\n", d->character);
        d->pending_message = NULL;
        d->character->delay_who = NULL;
        d->character->delay_who2 = NULL;
        d->character->delay_info1 = 0;
        return;
    }

    reformat_desc (d->pending_message->message, &d->pending_message->message);

    *d->character->delay_who = toupper (*d->character->delay_who);
    *d->character->delay_who2 = toupper (*d->character->delay_who2);

    mysql_safe_query
    ("INSERT INTO helpfiles VALUES (\'%s\', \'%s\', \'\n%s\n\', \'(null)\', \'%d\', \'%s\', \'%s\')",
     d->character->delay_who2, d->character->delay_who,
     d->pending_message->message, d->character->delay_info1, date,
     d->character->tname);

    d->pending_message = NULL;
    d->character->delay_who = NULL;
    d->character->delay_who2 = NULL;
    d->character->delay_info1 = 0;
}

void
do_hedit (CHAR_DATA * ch, char *argument, int cmd)
{
    char topic[MAX_STRING_LENGTH] = { '\0' };
    char subject[MAX_STRING_LENGTH] = { '\0' };
    char level[MAX_STRING_LENGTH] = { '\0' };
    int lvl = 0;

    argument = one_argument (argument, topic);
    argument = one_argument (argument, subject);
    argument = one_argument (argument, level);

    if (IS_NPC (ch))
    {
        send_to_char ("This is a PC-only command.\n", ch);
        return;
    }

    if (!*topic || !*subject || !*level)
    {
        send_to_char
        ("You must specify the topic, subject, and required level for the help entry.\n",
         ch);
        return;
    }

    if (!isdigit (*level))
    {
        send_to_char
        ("You must specify a number for the entry's required access level.\n",
         ch);
        return;
    }

    lvl = atoi (level);

    ch->delay_who = str_dup (topic);
    ch->delay_who2 = str_dup (subject);
    ch->delay_info1 = lvl;

    send_to_char ("Enter the text of this database entry:\n", ch);

    make_quiet (ch);

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_help;
}

void
do_vboards (CHAR_DATA * ch, char *argument, int cmd)
{
    MYSQL_RES *result = NULL;
    MYSQL_ROW row = NULL;
    char buf[MAX_STRING_LENGTH];
    int colnum = 1;

    *b_buf = '\0';

    mysql_safe_query
    ("SELECT board_name FROM virtual_boards GROUP BY board_name ORDER BY board_name ASC");
    result = mysql_store_result (database);

    while ((row = mysql_fetch_row (result)))
    {
        if (colnum == 1)
            sprintf (b_buf + strlen (b_buf), "%-29s", row[0]);
        else if (colnum == 2)
            sprintf (b_buf + strlen (b_buf), "%-29s", row[0]);
        else if (colnum == 3)
        {
            sprintf (b_buf + strlen (b_buf), "%s\n", row[0]);
            colnum = 1;
            continue;
        }
        colnum++;
    }

    sprintf (buf, "#6                      %s Virtual Boards#0\n", MUD_NAME);
    send_to_char (buf, ch);

    if (!*b_buf)
        send_to_char ("None.\n", ch);
    else
        page_string (ch->descr(), b_buf);

    if (colnum != 1)
        send_to_char ("\n", ch);

}

void
log_missing_helpfile (CHAR_DATA * ch, char *string)
{
    char msg[MAX_STRING_LENGTH] = { '\0' };
    char subj[MAX_STRING_LENGTH] = { '\0' };
    MYSQL_RES *result = NULL;

    mysql_safe_query
    ("SELECT * FROM unneeded_helpfiles WHERE name = '%s' AND (datestamp > UNIX_TIMESTAMP()-60*60*24*30*6)",
     string);
    result = mysql_store_result (database);

    if (result && mysql_num_rows (result) >= 1)
    {
        mysql_free_result (result);
        return;
    }

    mysql_safe_query ("DELETE FROM unneeded_helpfiles WHERE name = \'%s\'",
                      string);

    sprintf (subj, "#1Not Found:#0 %s", string);
    sprintf (msg, "%s: help %s\n", ch->tname, string);

    add_message (1, "Helpfiles", -5, ch->tname, 0, subj, "", msg, 0);

    mysql_safe_query ("INSERT INTO unneeded_helpfiles VALUES (\'%s\', %d)",
                      string, (int) time (0));
}


int
number_of_helpfiles_available (int player_level)
{
    // Build a query to get the number of helpfiles for that level
    int number_of_helpfiles = 0;
    std::ostringstream query_stream;
    query_stream << "SELECT COUNT(*) AS num_helpfiles "
    << "FROM helpfiles "
    << "WHERE required_level <= "
    << player_level;

    std::string query = query_stream.str ();
    if ((mysql_safe_query ((char *)query.c_str())) == 0)
    {
        MYSQL_RES *result = 0;
        if ((result = mysql_store_result (database)) != 0)
        {
            MYSQL_ROW row = 0;
            if ((row = mysql_fetch_row (result)) != 0)
            {
                number_of_helpfiles = strtol (row[0],0,10);
            }
            mysql_free_result (result);
        }
        else
        {
            std::string error_message =
                "number_of_helpfiles_available: "
                "Couldn't get a helpfile count because: ";
            error_message += mysql_error (database);

            std::cerr << error_message << std::endl;
            system_log (error_message.c_str (), true);
        }
    }
    else
    {
        std::string error_message =
            "number_of_helpfiles_available: "
            "'mysql_safe_query' failed for the following reason: ";
        error_message += mysql_error (database);

        std::cerr << error_message << std::endl;
        system_log (error_message.c_str (), true);
    }

    return number_of_helpfiles;
}

std::string
output_categories_available (int player_level)
{
    std::string category_list;

    // Query a list of categories from the helpfiles table
    std::ostringstream query_stream;
    query_stream << "SELECT category "
    << "FROM helpfiles "
    << "WHERE required_level <= "
    << player_level << ' '
    << "GROUP BY category ASC";

    std::string query = query_stream.str ();
    if ((mysql_safe_query ((char *)query.c_str ())) == 0)
    {
        // Store a formatted table of categories
        MYSQL_RES *result = 0;
        if ((result = mysql_store_result (database)) != 0)
        {
            char category[512];
            int category_counter = 0;
            MYSQL_ROW row = 0;
            while ((row = mysql_fetch_row (result)))
            {
                sprintf (category, "#6%-18.18s#0 ", row[0]);
                category_list += category;
                if (!(category_counter % 4))
                {
                    category_list += '\n';
                }
                category_counter++;
            }
            if ((category_counter % 4) != 1)
            {
                category_list += '\n';
            }
            mysql_free_result (result);
        }
        else
        {
            std::string error_message =
                "output_categories_available: "
                "Couldn't create a category list because: ";
            error_message += mysql_error (database);
            std::cerr << error_message << std::endl;
            system_log (error_message.c_str (), true);
        }
    }
    else
    {
        std::string error_message =
            "output_categories_available: "
            "'mysql_safe_query' failed to query shadows.helpfiles because: ";
        error_message += mysql_error (database);

        std::cerr << error_message << std::endl;
        system_log (error_message.c_str (), true);
    }

    return category_list;
}

void
do_help (CHAR_DATA * ch, char *argument, int cmd)
{
    MYSQL_RES *result = NULL;
    MYSQL_ROW row = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char arg1[MAX_STRING_LENGTH] = { '\0' };
    char arg2[MAX_STRING_LENGTH] = { '\0' };
    char example[MAX_STRING_LENGTH] = { '\0' };
    char original[MAX_STRING_LENGTH] = { '\0' };
    bool header_needed = false, category_list = false, soundex = false;
    int j = 1;
    int player_level = 0;

    result = NULL;

    *arg1 = '\0';
    *arg2 = '\0';
    *b_buf = '\0';
    *example = '\0';

    if (strstr (argument, ":"))
    {
        send_to_char ("Please see #6HELP HELP#0 for proper syntax and usage.\n",
                      ch);
        return;
    }

    sprintf (original, "%s", argument);

    argument = one_argument (argument, arg1);

    if (*argument)
        argument = one_argument (argument, arg2);

    // Mortals and NPCs get Level 0 Help
    char level_condition[16];
    if (IS_MORTAL (ch) || IS_NPC (ch))
    {
        player_level = 0;
        sprintf (level_condition, "= 0");
    }
    else
    {
        player_level = ch->pc->level;
        sprintf (level_condition, "<= %d", ch->pc->level);
    }

    if (!*arg1)
    {
        int help_available = number_of_helpfiles_available (player_level);
        std::string category_list = output_categories_available (player_level);

        // Query one helpfile name at random
        sprintf (buf,
                 "SELECT name "
                 "FROM helpfiles "
                 "WHERE required_level %s "
                 "ORDER BY RAND() DESC "
                 "LIMIT 1", level_condition);

        if ((mysql_safe_query (buf)) !=0)
        {
            std::cerr << "The library call 'mysql_safe_query' failed to "
            << "query shadows.helpfiles for the following reason: "
            << mysql_error (database) << std::endl;
            return;
        }

        // Store the helpfile name
        *buf = 0;
        if ((result = mysql_store_result (database)) != 0)
        {
            if ((row = mysql_fetch_row (result)))
            {
                sprintf (buf, "Helpfile of the Moment: #6%s#0", row[0]);
            }
            mysql_free_result (result);
        }
        else
        {
            sprintf (buf, "do_help: "
                     "Couldn't create a category list because: %s\n",
                     mysql_error (database));
            std::cerr << buf;
            system_log (buf, true);
        }

        // Spit out a generic syntax message
        sprintf (b_buf,
                 "\n                      #6%s Help Database#0\n\n"
                 "There are currently #6%d#0 helpfiles accessible to you"
                 " in our database.\n\n"
                 "Helpfile Categories:\n\n%s\n%s\n\n"
                 "To see a full list of helpfiles within these topics,"
                 " use \'#6help on <topic name>#0\'.\n\n"
                 "For a detailed description of HELP searching syntax,"
                 " please see \'#6help help#0\'.\n\n"
                 "For more detailed information, be sure to check out the ParallelWiki:\n"
				 "#6http://www.forum.parallelrpi.com/doku.php.#0\n",
                 MUD_NAME,
                 help_available,
                 category_list.empty () ? "#6None#0\n" : category_list.c_str (),
                 buf);

        page_string (ch->descr(), b_buf);
        return;
    }

    if (strcasecmp (arg1, "named") == STR_MATCH && *arg2)
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE name LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
         arg2, ch->pc ? ch->pc->level : 0);
    else if (strcasecmp (arg1, "containing") == STR_MATCH && *arg2)
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE (entry LIKE '%%%s%%' OR related_entries LIKE '%%%s%%') AND required_level <= %d ORDER BY category,name ASC",
         arg2, arg2, ch->pc ? ch->pc->level : 0);
    else if (strcasecmp (arg1, "on") == STR_MATCH && *arg2)
    {
        category_list = true;
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
         arg2, ch->pc ? ch->pc->level : 0);
    }
    else if (*arg1 && !*arg2)
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE name LIKE '%s%%' AND required_level <= %d ORDER BY category,name ASC",
         arg1, ch->pc ? ch->pc->level : 0);
    else if (*arg1 && *arg2)
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE name = '%s' AND (category = '%s' AND required_level <= %d) ORDER BY category,name ASC",
         arg2, arg1, ch->pc ? ch->pc->level : 0);

    result = mysql_store_result (database);

    if (!result)
    {
        send_to_char
        ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
         ch);
        log_missing_helpfile (ch, original);
        return;
    }

    if (mysql_num_rows (result) == 0)
    {
        result = NULL;
        mysql_safe_query
        ("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
         arg1, ch->pc ? ch->pc->level : 0);
        result = mysql_store_result (database);
        if (!result || mysql_num_rows (result) == 0)
        {
            mysql_safe_query
            ("SELECT * FROM helpfiles WHERE SOUNDEX(category) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
             arg1, ch->pc ? ch->pc->level : 0);
            result = mysql_store_result (database);
            if (!result || mysql_num_rows (result) == 0)
            {
                mysql_safe_query
                ("SELECT * FROM helpfiles WHERE SOUNDEX(name) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
                 arg1, ch->pc ? ch->pc->level : 0);
                result = mysql_store_result (database);
                if (!result || mysql_num_rows (result) == 0)
                {
                    mysql_safe_query
                    ("SELECT * FROM helpfiles WHERE related_entries LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
                     arg1, ch->pc ? ch->pc->level : 0);
                    result = mysql_store_result (database);
                    if (!result || mysql_num_rows (result) == 0)
                    {
                        mysql_safe_query
                        ("SELECT * FROM helpfiles WHERE entry LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
                         arg1, ch->pc ? ch->pc->level : 0);
                        result = mysql_store_result (database);
                        if (result || mysql_num_rows (result) > 0)
                            category_list = true;
                    }
                }
                else
                    soundex = true;
            }
            else
                category_list = true;
        }
        else
            category_list = true;
    }
    if (mysql_num_rows (result) == 1)
    {
        row = mysql_fetch_row (result);
        if (!row || GET_TRUST (ch) < atoi (row[4]))
        {
            if (!mysql_num_rows (result))
            {
                if (result != NULL)
                    mysql_free_result (result);
                send_to_char
                ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
                 ch);
                mysql_free_result (result);
                log_missing_helpfile (ch, original);
                return;
            }
        }
        if (mysql_num_rows (result) == 1)
        {
            sprintf (b_buf, "\n#6%s: %s#0\n", row[1], row[0]);
            if (*row[2] != '\n')
                strcat (b_buf, "\n");
            sprintf (b_buf + strlen (b_buf), "%s", row[2]);
            if (row[3] && *row[3] && strcasecmp (row[3], "(null)") != STR_MATCH)
                sprintf (b_buf + strlen (b_buf), "#6See Also:#0 %s\n\n", row[3]);
            sprintf (b_buf + strlen (b_buf), "#6Last Updated:#0 %s, by %s\n",
                     row[5], CAP (row[6]));
        }
    }

    if (mysql_num_rows (result) > 1)
    {
        header_needed = true;
        while ((row = mysql_fetch_row (result)))
        {
            if (GET_TRUST (ch) < atoi (row[4]))
                continue;
            if (header_needed)
            {
                if (!soundex)
                    sprintf (b_buf,
                             "\nYour query matched the following helpfiles:\n\n   ");
                else
                    sprintf (b_buf,
                             "\nThe following entries most closely matched your spelling:\n\n   ");
                header_needed = false;
            }
            if (!category_list)
                sprintf (arg2, "%13s: %s", row[1], row[0]);
            else
                sprintf (arg2, "%s", row[0]);
            if (!category_list)
            {
                if (j == 1)
                {
                    sprintf (arg1, "%3d. #6%s#0", j, arg2);
                    sprintf (example, "%s", row[0]);
                    if (!strchr (example, ' '))
                        sprintf (example,
                                 "\nTo pull up the first entry listed, type \'#6help %s %s#0\'.\n",
                                 LOW (row[1]), LOW (row[0]));
                    else
                        sprintf (example,
                                 "\nTo pull up the first entry listed, type \'#6help %s \"%s\"#0\'.\n",
                                 LOW (row[1]), row[0]);
                }
                else
                    sprintf (arg1, "\n   %3d. #6%s#0", j, arg2);
            }
            else
                sprintf (arg1, "#6%-22.22s#0   ", arg2);
            if (!(j % 3) && category_list)
                strcat (arg1, "\n   ");
            j++;
            sprintf (b_buf + strlen (b_buf), "%s", arg1);
        }
        if ((((j - 1) % 3) && !header_needed) || !category_list)
            strcat (b_buf, "\n");
        if (*example)
            sprintf (b_buf + strlen (b_buf), "%s", example);
    }

    mysql_free_result (result);
    result = NULL;

    if (*b_buf)
        page_string (ch->descr(), b_buf);
    else
    {
        send_to_char
        ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
         ch);
        log_missing_helpfile (ch, original);
        return;
    }
}

void
do_users (CHAR_DATA * ch, char *argument, int cmd)
{
    DESCRIPTOR_DATA *d = NULL;
    CHAR_DATA *tch = NULL;
    char colon_char = '\0';
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char output[MAX_STRING_LENGTH] = { '\0' };
    char line[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char buf3[MAX_STRING_LENGTH] = { '\0' };
    char buf4[MAX_STRING_LENGTH] = { '\0' };

    strcpy (buf, "\n#6Users Currently Connected:\n"
            "#6--------------------------\n\n#0");

    if (maintenance_lock)
        sprintf (buf + strlen (buf),
                 "#2We are currently closed for maintenance.\n\n#0");

    if (pending_reboot)
        sprintf (buf + strlen (buf),
                 "#2There is currently a reboot pending.\n\n#0");

    send_to_char (buf, ch);

    for (d = descriptor_list; d; d = d->next)
    {

        colon_char = ':';

        *buf2 = '\0';
        *buf3 = '\0';
        *buf4 = '\0';

        if (d->character && !d->character->descr())
            continue;

        if (d->character && d->original)
            tch = d->original;
        else
            tch = d->character;

        if (tch && tch->pc->level)
            colon_char = '*';

        if (tch && d->acct)
        {
            sprintf (line, "%s  %c ", tch->pc->account_name, colon_char);
            sprintf (line + strlen (line), " #2[%s]#0 ", tch->tname);
            sprintf (line + strlen (line), "#3[%s]#0 ", d->strClientHostname);
            sprintf (line + strlen (line), "#5[%d]#0", tch->in_room);
            if (tch->pc->create_state == STATE_DIED)
                sprintf (line + strlen (line), " #1(Dead)#0");
            if (d->idle)
                sprintf (line + strlen (line), " #4(Idle)#0");
            if (IS_SET (tch->act, PLR_QUIET))
                sprintf (line + strlen (line), " #5(Edit)#0");
            if (IS_SET (tch->plr_flags, NEW_PLAYER_TAG))
                sprintf (line + strlen (line), " #2(New)#0");
            sprintf (line + strlen (line), "\n");
        }

        else
            continue;

        if (strlen (output) + strlen (line) > MAX_STRING_LENGTH)
            break;

        sprintf (output + strlen (output), "%s", line);
    }

    page_string (ch->descr(), output);
}

char *
worn_first_loc (OBJ_DATA * obj)
{
    if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
        return "around your neck";
    else if (obj->location == WEAR_BODY)
        return "on your body";
    else if (obj->location == WEAR_HEAD)
        return "on your head";
    else if (obj->location == WEAR_ARMS)
        return "on your arms";
    else if (obj->location == WEAR_OVER)
        return "over your body";
    else if (obj->location == WEAR_ABOUT)
        return "about your body";
    else if (obj->location == WEAR_WAIST)
        return "around your waist";
    else if (obj->location == WEAR_WRIST_L)
        return "on your left wrist";
    else if (obj->location == WEAR_WRIST_R)
        return "on your right wrist";
    else if (obj->location == WEAR_ANKLE_L)
        return "on your left ankle";
    else if (obj->location == WEAR_ANKLE_R)
        return "on your right ankle";
    else if (obj->location == WEAR_HAIR)
        return "in your hair";
    else if (obj->location == WEAR_FACE)
        return "on your face";
    else if (obj->location == WEAR_EYES)
        return "over your eyes";
    else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
        return "on your belt";
    else if (obj->location == WEAR_BACK)
        return "on your back";
    else if (obj->location == WEAR_THROAT)
        return "around your throat";
    else if (obj->location == WEAR_BLINDFOLD)
        return "as a blindfold";
    else if (obj->location == WEAR_EAR)
        return "on your ear";
    else if (obj->location == WEAR_SHOULDER_R)
        return "over your right shoulder";
    else if (obj->location == WEAR_SHOULDER_L)
        return "over your left shoulder";
    else if (obj->location == WEAR_FEET)
        return "on your feet";
    else if (obj->location == WEAR_FINGER_R)
        return "on your right ring finger";
    else if (obj->location == WEAR_FINGER_L)
        return "on your left ring finger";
    else if (obj->location == WEAR_ARMBAND_R)
        return "around your upper right arm";
    else if (obj->location == WEAR_ARMBAND_L)
        return "around your upper left arm";
    else if (obj->location == WEAR_LEGS)
        return "on your legs";
    else if (obj->location == WEAR_HANDS)
        return "on your hands";
    else if (obj->location == WEAR_UNDERWEAR)
        return "around your hips";
    else if (obj->location == WEAR_OVERWEAR)
        return "around your chest";
    else
        return "#1in an unknown location#0";
}

void
do_equipment (CHAR_DATA * ch, char *argument, int cmd)
{
    int location = 0;
    int found = false;
    OBJ_DATA *eq = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char wear[MAX_STRING_LENGTH];
    
    
    *buf = '\0';
    if (ch->right_hand)
    {
        if (ch->right_hand->location == WEAR_PRIM
                || ch->right_hand->location == WEAR_SEC)
            sprintf (buf, "<wielded in right hand>  ");
        else if (ch->right_hand->location == WEAR_BOTH)
            sprintf (buf, "<wielded in both hands>  ");
        else if (ch->right_hand->location == WEAR_SHIELD)
            sprintf (buf, "<gripped in right hand>  ");
        else
            sprintf (buf, "<carried in right hand>  ");

        send_to_char (buf, ch);
        show_obj_to_char (ch->right_hand, ch, 1);
    }
    if (ch->left_hand)
    {
        if (ch->left_hand->location == WEAR_PRIM
                || ch->left_hand->location == WEAR_SEC)
            sprintf (buf, "<wielded in left hand>   ");
        else if (ch->left_hand->location == WEAR_BOTH)
            sprintf (buf, "<wielded in both hands>  ");
        else if (ch->left_hand->location == WEAR_SHIELD)
            sprintf (buf, "<gripped in left hand>   ");
        else
            sprintf (buf, "<carried in left hand>   ");

        send_to_char (buf, ch);
        show_obj_to_char (ch->left_hand, ch, 1);
    }

    if (ch->left_hand || ch->right_hand)
        send_to_char ("\n", ch);

    for (location = 0; location < MAX_WEAR; location++)
    {

        if (!(eq = get_equip (ch, loc_order[location])))
            continue;

        if (eq == ch->right_hand || eq == ch->left_hand)
            continue;

        *wear = '\0';
        if (eq->attire)
            sprintf(wear, "<%s>%*s", eq->attire, 23-strlen(eq->attire), "");     
         else
            sprintf(wear, "%s", where[loc_order[location]]);

        send_to_char(wear, ch);
        
		if (location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq))
            show_obj_to_char (eq, ch, 1);
        else
            send_to_char ("#2something#0\n", ch);

        found = true;
    }

    if (!found)
        send_to_char ("You are naked.\n", ch);

}

void
do_news (CHAR_DATA * ch, char *argument, int cmd)
{

    std::string msg_line;
    std::string output;

    std::ifstream fin( "MOTD" );

    if ( !fin )
    {
        system_log ("The MOTD could not be found", true);
        send_to_char("The MOTD could not be found", ch);
        return;
    }

    while ( getline(fin, msg_line) )
    {
        output.append(msg_line);
    }

    fin.close();

    if (!output.empty())
    {
        send_to_char (output.c_str(), ch);
        send_to_char ("\n", ch);
    }

}

void
do_wizlist (CHAR_DATA * ch, char *argument, int cmd)
{
    page_string (ch->descr(), get_text_buffer (ch, text_list, "wizlist"));
}

int
show_where_char (const CHAR_DATA * ch, int indent)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!ch || !ch->room || ch->in_room == NOWHERE || !ch->short_descr
            || !ch->tname)
        return 0;

    memset (buf, ' ', indent * 3);
    buf[indent * 3] = '\0';

    sprintf (buf + strlen (buf), "%s",
             IS_NPC (ch) ? ch->short_descr : ch->tname);

    if (IS_NPC (ch))
        sprintf (buf + strlen (buf), " (%d)", ch->mob->vnum);

    if (ch->in_room == -1)
        strcat (buf, " NOWHERE\n");
    else
        sprintf (buf + strlen (buf), " in room %d\n", ch->in_room);

    if (strlen (b_buf) > B_BUF_SIZE - 500)
        return 0;

    strcat (b_buf, buf);

    return 1;
}

int
show_where_obj (OBJ_DATA * obj, int indent)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!obj)
        return 0;

    memset (buf, ' ', indent * 3);
    buf[indent * 3] = '\0';

    sprintf (buf + strlen (buf), "%s (%d)",
             obj->short_description, obj->nVirtual);

    if (obj->in_room != -1)
        sprintf (buf + strlen (buf), " in room %d\n", obj->in_room);
    else if (obj->in_obj)
        sprintf (buf + strlen (buf), " in obj:\n");
    else if (obj->carried_by && obj->location == -1
             && obj->carried_by->room != NULL)
        sprintf (buf + strlen (buf), " carried by:\n");
    else if (obj->equiped_by && obj->equiped_by->room != NULL)
        sprintf (buf + strlen (buf), " equipped by:\n");
    else
        sprintf (buf + strlen (buf), " ; not sure\n");

    strcat (b_buf, buf);

    if (strlen (b_buf) > B_BUF_SIZE - 500)
        return 0;

    if (obj->in_obj)
        show_where_obj (obj->in_obj, indent + 1);
    else if (obj->equiped_by)
        show_where_char (obj->equiped_by, indent + 1);
    else if (obj->carried_by)
        show_where_char (obj->carried_by, indent + 1);

    return 1;
}

void
do_find (CHAR_DATA * ch, char *argument, int cmd)
{
    int nots = 0;
    int musts = 0;
    int zone = -1;
    int nVirtual = -1;
    int type = -1;
    int i;
    OBJ_DATA *obj = NULL;
    char *not_list[50] = { '\0' };
    char *must_list[50] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (!*buf || *buf == '?')
    {
        send_to_char ("find +keyword     'keyword' must exist on obj\n", ch);
        send_to_char ("     -keyword     'keyword' must not exist on obj.\n",
                      ch);
        send_to_char ("     zone         Only consider objects in this zone.\n",
                      ch);
        send_to_char ("     vnum         Just find the specific object vnum.\n",
                      ch);
        send_to_char ("\nExamples:\n\n", ch);
        send_to_char ("   > find +bag +leather 10\n", ch);
        send_to_char ("   > find 10001\n", ch);
        send_to_char ("   > find +sword -rusty 12\n", ch);
        return;
    }

    while (*buf)
    {

        if (*buf == '+')
            must_list[musts++] = str_dup (buf + 1);

        else if (*buf == '-')
            not_list[nots++] = str_dup (buf + 1);

        else if (isdigit (*buf))
        {
            if (atoi (buf) < 100)
                zone = atoi (buf);
            else
                nVirtual = atoi (buf);
        }

        else if ((type = index_lookup (item_types, buf)) != -1)
            ;

        else
        {
            send_to_char ("Unknown keyword to where obj.\n", ch);
            return;
        }

        argument = one_argument (argument, buf);
    }

    *b_buf = '\0';

    for (obj = object_list; obj; obj = obj->next)
    {

        if (obj->deleted)
            continue;

        for (i = 0; i < musts; i++)
            if (!isname (must_list[i], obj->name))
                break;

        if (i != musts)		/* Got out of loop w/o all musts being there */
            continue;

        for (i = 0; i < nots; i++)
            if (isname (must_list[i], obj->name))
                break;

        if (i != nots)		/* Got out of loop w/o all nots not being thr */
            continue;

        if (nVirtual != -1 && obj->nVirtual != nVirtual)
            continue;

        if (type != -1 && obj->obj_flags.type_flag != type)
            continue;

        /* Zone is a little tricky to determine */

        if (!show_where_obj (obj, 0))
            break;
    }

    page_string (ch->descr(), b_buf);
}

void
do_locate (CHAR_DATA * ch, char *argument, int cmd)
{
    int num_clans = 0;
    int i;
    int musts = 0;
    int nots = 0;
    bitflag act_bits = 0;
    int position = -1;
    int ind = 0;
    int nVirtual = 0;
    int zone = -1;
    int pc_only = 0;
    CHAR_DATA *mob = NULL;
    char *arg = '\0';
    char clan_names[10][80] = { {'\0'}, {'\0'} };
    char not_list[50][80] = { {'\0'}, {'\0'} };
    char must_list[50][80] = { {'\0'}, {'\0'} };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char acts[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (!*buf || *buf == '?')
    {
        send_to_char ("locate [+/-mob name]\n"
                      "       [zone]\n"
                      "       [virtual]\n"
                      "       act \"<action-bits>\"\n"
                      "       [clan name]\n"
                      "       clan <clan name>\n" "       [position]\n", ch);
        return;
    }

    while (*buf)
    {

        if (*buf == '+')
            strcpy (must_list[musts++], buf + 1);

        else if (*buf == '-')
            strcpy (not_list[nots++], buf + 1);

        else if ((position = index_lookup (position_types, buf)) != -1)
            ;

        else if (isdigit (*buf))
        {
            ind = atoi (buf);
            if (ind > 100)
                nVirtual = ind;
            else
                zone = ind;
        }

        else if (strcasecmp (buf, "act") == STR_MATCH)
        {

            argument = one_argument (argument, acts);

            arg = one_argument (acts, buf);

            while (*buf)
            {

                if ((ind = index_lookup (action_bits, buf)) == -1)
                {
                    send_to_char ("No such action-bit:  ", ch);
                    send_to_char (buf, ch);
                    send_to_char ("\n", ch);
                    return;
                }

                act_bits |= (1 << ind);

                arg = one_argument (arg, buf);
            }
        }

        else if (strcasecmp (buf, "clan") == STR_MATCH)
        {

            argument = one_argument (argument, buf);

            if ( !str_cmp (buf, "pc") )
            {
                argument = one_argument (argument, buf);
                pc_only++;
            }

            if (num_clans >= 9)
            {
                send_to_char ("Hey, 10 clans is enough!\n", ch);
                return;
            }

            strcpy (clan_names[num_clans++], buf);
        }

        else if (isalpha (*buf))
        {

            if (num_clans >= 9)
            {
                send_to_char ("Hey, 10 clans is enough!\n", ch);
                return;
            }

            strcpy (clan_names[num_clans++], buf);
        }

        else
        {
            send_to_char ("Unknown keyword:  ", ch);
            send_to_char (buf, ch);
            send_to_char ("\n", ch);
            return;
        }

        argument = one_argument (argument, buf);
    }

    *b_buf = '\0';

    for (mob = character_list; mob; mob = mob->next)
    {

        if (mob->deleted)
            continue;

        /* Check act bits against mob's act bits.  The act bits
           specified on the command line must all be in mob->act */

        if (IS_NPC(mob) && pc_only)
            continue;

        if (zone != -1 && mob->room->zone != zone)
            continue;

        if (nVirtual && (!mob->mob || mob->mob->vnum != nVirtual))
            continue;

        if (position != -1 && GET_POS (mob) != position)
            continue;

        if (act_bits && (act_bits & mob->act) != act_bits)
            continue;

        for (i = 0; i < num_clans; i++)
            if (is_clan_member (mob, clan_names[i]))
                break;

        if (num_clans && i >= num_clans)	/* Couldn't find a clan member */
            continue;

        for (i = 0; i < musts; i++)
            if (!isname (must_list[i], mob->name))
                break;

        if (i != musts)		/* Got out of loop w/o all musts being there */
            continue;

        for (i = 0; i < nots; i++)
            if (isname (not_list[i], mob->name))
                break;

        if (i != nots)		/* Got out of loop w/o all nots not being thr */
            continue;

        show_where_char (mob, 0);
    }

    page_string (ch->descr(), b_buf);
}

#define WHERE_LINE_LEN 77
void
do_where (CHAR_DATA * ch, char *argument, int cmd)
{
    char clan[MAX_INPUT_LENGTH] = "";
    char name[MAX_INPUT_LENGTH] = "";
    char buf[MAX_STRING_LENGTH] = "";
    char buf1[MAX_STRING_LENGTH] = "";
    char strFmtName[AVG_STRING_LENGTH] = "#1No Name!#0";
    char strFmtAnim[AVG_STRING_LENGTH] = "";
    char strFmtRoom[AVG_STRING_LENGTH] = "";
    unsigned short int nMaxFmtRoomLen = WHERE_LINE_LEN;
    unsigned short int nDescriptors = 0, x = 0, y = 0;
    int arrRoom[500], tmpRoom = 0, rpp = 0;
    unsigned char bIsInWater = 0;
    ROOM_DATA *ch_room = NULL;
    register CHAR_DATA *i = NULL;
    register OBJ_DATA *k = NULL;
    struct descriptor_data *d = NULL;
    char chrState = ' ';
    bool check_aura = false;
    bool check_clan = false;

    one_argument (argument, name);
    if (strcmp (name,"aura") == 0)
        check_aura = true;

    if (strcmp (name,"clan") == 0)
    {
        check_clan = true;  // set check_clan true if the admin wants to list a specific clan -Meth
        half_chop (argument, name, buf1);  // chop the word 'clan' off the argument
        one_argument(buf1, clan); // read the clanname into 'clan'
    }

    if (!*name || check_aura || check_clan)
    {
        if (IS_MORTAL (ch))
        {
            send_to_char ("What are you looking for?\n", ch);
            return;
        }
        else
        {
            /* The next four lines are pointless, we set nDescriptors again just 13 lines further on -Meth
            for (d = descriptor_list; d; d = d->next)
            {
            nDescriptors++;
            }
            */

            // iterate through descriptors and save room numbers where there are active players -Meth
            for (d = descriptor_list, x = 0; d; d = d->next)
            {
                if (d->character && (d->connected == CON_PLYNG)
                        && (d->character->in_room != NOWHERE))
                {
                    /* arrDesc[x++] = d; */
                    if (!check_clan || is_clan_member(d->character, clan)) // store the room if we're not listing by clans or if the char is in the specified clan -Meth
                        arrRoom[x++] = d->character->in_room;
                }
                arrRoom[x] = -1;  // set the next room to -1 to show the end -Meth
            }

            /*                      arrDesc[x] = NULL; */
            nDescriptors = x; // nDescriptors now equals the number of characters we'll show in the output - Meth

            // A bubble-sort to put the room numbers in order for pretty output -Meth
            for (y = 0; y < nDescriptors - 1; y++)
            {
                for (x = 0; x < nDescriptors - 1; x++)
                {
                    if (arrRoom[x] > arrRoom[x + 1])
                    {
                        tmpRoom = arrRoom[x];
                        arrRoom[x] = arrRoom[x + 1];
                        arrRoom[x + 1] = tmpRoom;
                    }
                    /*
                       if ( arrDesc[x]->character->in_room > arrDesc[x + 1]->character->in_room ) {
                       tmpDesc = arrDesc [x];
                       arrDesc [x] = arrDesc [x + 1];
                       arrDesc [x + 1] = tmpDesc;
                       }
                     */
                }
            }

            strcat (buf, "\n"); // not sure why we're adding the newline here -Meth


            for (x = 0; x < nDescriptors; x++)
            {

                /* d = arrDesc[x]; */
                /* ch_room = vtor(d->character->in_room); */

                // skip if room number is less than zero OR if room number is equal to the last room OR
                if ((arrRoom[x] < 0) || (x > 0 && arrRoom[x] == arrRoom[x - 1])
                        || !(ch_room = vnum_to_room (arrRoom[x])))
                    continue;

                bIsInWater = (ch_room && (ch_room->sector_type == SECT_RIVER
                                          || ch_room->sector_type == SECT_LAKE
                                          || ch_room->sector_type == SECT_OCEAN
                                          || ch_room->sector_type == SECT_REEF
                                          || ch_room->sector_type == SECT_UNDERWATER));

                sprintf (strFmtRoom, "#2[%6d]#0 #6%s", ch_room->vnum,
                         ch_room->name);

                for (d = descriptor_list; d; d = d->next) // iterate through characters to find out if they are in room arrRoom[x]
                {

                    if (d->character && (d->connected == CON_PLYNG)
                            && (d->character->in_room != NOWHERE)
                            && (d->character->in_room == arrRoom[x])
                            && (!check_clan || is_clan_member(d->character,clan))
                       )
                    {


                        /* Color Player Names */
                        if ((d->original
                                && (!bIsInWater && !d->character->fighting))
                                || (d->character->pc
                                    && d->character->pc->level > 0))
                        {
                            strcpy (strFmtName, "#5");
                        }
                        else if (d->character->fighting)
                        {
                            strcpy (strFmtName, "#1");
                        }
                        else if (bIsInWater)
                        {
                            strcpy (strFmtName, "#4");
                        }
                        else if (IS_SET (d->character->flags, FLAG_ISADMIN))
                        {
                            strcpy (strFmtName, "#6");
                        }
                        else
                            if (IS_SET (d->character->plr_flags, NEW_PLAYER_TAG))
                            {
                                strcpy (strFmtName, "#2");
                            }
                            else if (IS_GUIDE (d->character))
                            {
                                strcpy (strFmtName, "#3");
                            }
                            else
                            {
                                strcpy (strFmtName, "#0");
                            }

                        rpp = d->acct ? d->acct->get_rpp () : 0;

                        /* Fill in correct Player Name */
                        if (d->original)
                        {
                            sprintf (strFmtName + 2, "%-16s #3[%2d]#0",
                                     d->original->tname, (check_aura)?(d->character->aur):(rpp));
                            sprintf (strFmtAnim, " (as #5%s#0)",
                                     fname (d->character->tname));
                        }
                        else
                        {
                            sprintf (strFmtName + 2, "%-16s #3[%2d]#0",
                                     d->character->tname, (check_aura)?(d->character->aur):(rpp));
                            strcpy (strFmtAnim, "");
                        }

                        i = d->character;
                        chrState = '-';
                        switch (GET_POS (i))
                        {
                        case POSITION_DEAD:
                        case POSITION_MORTALLYW:
                            chrState = 'X';
                            break;
                        case POSITION_UNCONSCIOUS:
                        case POSITION_STUNNED:
                            chrState = 'U';
                            break;
                        case POSITION_SLEEPING:
                            chrState = 's';
                        default:
                        {

                            if (!IS_NPC (i) && !i->descr()
                                    && !i->pc->admin_loaded)
                                chrState = 'L';

                            else if (IS_SET (i->act, PLR_QUIET)
                                     && !IS_NPC (i))
                                chrState = 'e';

                            else if (get_affect (i, MAGIC_HIDDEN))
                                chrState = 'h';

                            else if (i->descr() && i->descr()->idle)
                                chrState = 'i';

                        }
                        }


                        /* Put the room together */
                        nMaxFmtRoomLen =
                            (WHERE_LINE_LEN + 6 +
                             ((strlen (strFmtAnim) >
                               0) ? 4 : 0)) - (strlen (strFmtName) +
                                               strlen (strFmtAnim));
                        if (strlen (strFmtRoom) >= nMaxFmtRoomLen)
                        {
                            strcpy (strFmtRoom + (nMaxFmtRoomLen - 3), "...");
                        }
                        sprintf (buf + strlen (buf), "%s#0 #%c%c#0 %s#0%s#0\n",
                                 strFmtName, (chrState <= 'Z'
                                              && chrState >=
                                              'A') ? '1' : ((chrState <= 'z'
                                                             && chrState >=
                                                             'a') ? '3' : '0'),
                                 chrState, strFmtRoom, strFmtAnim);
                    }
                }

            }
            strcat (buf,
                    "\n#0Color key: #5Admin,#0 #6Admin PC,#0 #3Guide,#0 #2New PC,#0 #1In Melee,#0 #4In Water#0\n");
            strcat (buf,
                    "\n#0Flag key:  e - editing, i - idle, h - hidden, s - sleeping\n"
                    "L - Link Dead, U - Unconscious, X - Near Death\n");
            page_string (ch->descr(), buf);
            return;
        }
    }

    *buf = '\0';

    for (i = character_list; i; i = i->next)
    {

        if (i->deleted)
            continue;
        if (strlen (buf) > MAX_STRING_LENGTH - 256)
        {
            strcat (buf, "#1Too many entries to display!#0\n");
            break;
        }
        if (isname (name, i->name) && CAN_SEE (ch, i))
        {
            if ((i->in_room != NOWHERE) && ((GET_TRUST (ch) > 3) ||
                                            (vnum_to_room (i->in_room)->zone ==
                                             vnum_to_room (ch->in_room)->zone)))
            {

                if (IS_NPC(i))
                    sprintf (buf + strlen (buf),
                             "#5%-20.20s#0 - #2[%5d]#0 #6%s#0\n", char_short (i),
                             vnum_to_room (i->in_room)->vnum,
                             vnum_to_room (i->in_room)->name);
                else
                    sprintf (buf + strlen (buf),
                             "#5%-20.20s#0 - #2[%5d]#0 #6%s#0\n", i->tname,
                             vnum_to_room (i->in_room)->vnum,
                             vnum_to_room (i->in_room)->name);


            }
        }
    }

    if (GET_TRUST (ch) > 3)
    {
        for (k = object_list; k; k = k->next)
        {

            if (k->deleted)
                continue;

            if (strlen (buf) > MAX_STRING_LENGTH - 256)
            {
                strcat (buf, "#1Too many entries to display!#0\n");
                break;
            }

            if (isname (name, k->name) && CAN_SEE_OBJ (ch, k))
            {
                if (k->carried_by && !k->carried_by->deleted
                        && k->carried_by->room)
                {
                    sprintf (buf + strlen (buf),
                             "#2%-20.20s#0- #2[%5d]#0 Carried by #5%s#0",
                             obj_short_desc (k),
                             vnum_to_room (k->carried_by->in_room)->vnum,
                             char_short (k->carried_by));
                    strcat (buf, "\n");
                }
                else if (k->equiped_by && !k->equiped_by->deleted
                         && k->equiped_by->room)
                {
                    sprintf (buf + strlen (buf),
                             "#2%-20.20s#0- #2[%5d]#0 Equipped by #5%s#0",
                             obj_short_desc (k),
                             vnum_to_room (k->equiped_by->in_room)->vnum,
                             char_short (k->equiped_by));
                    strcat (buf, "\n");
                }
                else if (k->in_obj && !k->in_obj->deleted)
                {
                    sprintf (buf + strlen (buf),
                             "#2%-20.20s#0- Inside #5%s#0, vnum#2[%5d].#0",
                             obj_short_desc (k), obj_short_desc (k->in_obj),
                             k->in_obj->nVirtual);
                    strcat (buf, "\n");
                }
                else if (k->in_room != NOWHERE)
                {
                    sprintf (buf + strlen (buf),
                             "#2%-20.20s#0- #2[%5d]#0 #6%s#0",
                             obj_short_desc (k), vnum_to_room (k->in_room)->vnum,
                             vnum_to_room (k->in_room)->name);
                    strcat (buf, "\n");
                }
            }
        }
    }

    if (!*buf)
        send_to_char ("Couldn't find any such thing.\n", ch);
    else
        page_string (ch->descr(), buf);

}



void
do_who (CHAR_DATA * ch, char *argument, int cmd)
{
    int mortals = 0;
    int immortals = 0;
    int guests = 0;
    char tmp[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    DESCRIPTOR_DATA *d = NULL;

    *s_buf = '\0';

    if (IS_MORTAL (ch))
        strcpy (tmp, "\n#2Available Staff:#0\n\n");
    else
        strcpy (tmp, "\n\n#2Available Staff:#0\n\n");


    for (d = descriptor_list; d; d = d->next)
    {

        if (!d->character)
            continue;

        if (d->connected)
            continue;

        if (IS_MORTAL (d->character))
        {
            if (d->character->pc &&
                    !d->character->pc->level &&
                    d->character->pc->create_state == 2
                    && !IS_SET (d->character->flags, FLAG_GUEST))
                mortals++;
            else if (IS_SET (d->character->flags, FLAG_GUEST))
                guests++;
        }

        else if (d->original && !IS_SET (d->original->flags, FLAG_AVAILABLE))
            continue;

        else if (IS_SET (d->character->flags, FLAG_AVAILABLE))
        {
            sprintf (tmp + strlen (tmp), "    %s",
                     d->original ? GET_NAME (d->original) :
                     GET_NAME (d->character));
            strcat (tmp, "\n");
            immortals = 1;
        }
    }

    *buf = '\0';

    if (!mortals)
        sprintf (s_buf,
                 "\nThe world is currently an empty wasteland, void of souls.\n");
    else if (mortals == 1)
        sprintf (s_buf, "\nThere is currently #21#0 poor sap being lonely.\n");
    else
        sprintf (s_buf, "\nThere are currently #2%d#0 grungy denizens roaming the world.\n",
                 mortals);

    if (guests)
        sprintf (s_buf + strlen (s_buf),
                 "There %s currently #2%d#0 guest%s masquerading within the Graveyard.\n",
                 guests == 1 ? "is" : "are", guests, guests == 1 ? "" : "s");

    sprintf (s_buf + strlen (s_buf),
             "\nOur record is #2%d#0 players, last seen on #2%s#0.",
             count_max_online, max_online_date);

    if (IS_MORTAL (ch))
        strcat (s_buf, "\n");

    strcat (s_buf, tmp);

    if (!immortals)
        sprintf (s_buf + strlen (s_buf), "    None.\n");

    send_to_char (s_buf, ch);
}

void
do_mwatch (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *target = NULL, *tch = NULL;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH];
    int dir;

    if ((af = get_affect (ch, AFFECT_WATCH_DIR)))
        affect_remove (ch, af);

    argument = one_argument (argument, buf);

    if (*buf && !(target = get_char_room_vis (ch, buf)))
    {
        if ((dir = index_lookup (dirs, buf)) == -1)
        {
            send_to_char ("Who or what did you want to watch?\n", ch);
            return;
        }

        if (!(af = get_affect (ch, AFFECT_WATCH_DIR)))
            magic_add_affect (ch, AFFECT_WATCH_DIR, -1, 0, 0, 0, 0);

        af = get_affect (ch, AFFECT_WATCH_DIR);

        af->a.shadow.shadow = NULL;
        af->a.shadow.edge = dir;

        sprintf (buf, "You will now watch the %s exit.\n", dirs[dir]);
        send_to_char (buf, ch);
        sprintf (buf, "$n turns to watch the %s exit.", dirs[dir]);
        act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        return;

    }

    if ((!*buf || target == ch))
    {
        if (!(af = get_affect (ch, MAGIC_WATCH))
                && !(af = get_affect (ch, AFFECT_WATCH_DIR)))
        {
            send_to_char ("You are not currently watching anything.\n", ch);
            return;
        }
        else if ((af = get_affect (ch, MAGIC_WATCH))
                 && (tch = (CHAR_DATA *) af->a.spell.t) != NULL)
        {
            act ("You cease watching $N.", true, ch, 0, tch,
                 TO_CHAR | _ACT_FORMAT);
            remove_affect_type (ch, MAGIC_WATCH);
            return;
        }
        else if ((af = get_affect (ch, AFFECT_WATCH_DIR)))
        {
            act ("You cease watching the exit.", true, ch, 0, 0, TO_CHAR);
            remove_affect_type (ch, AFFECT_WATCH_DIR);
            return;
        }
        else
        {
            send_to_char ("You cease watching.\n", ch);
            remove_affect_type (ch, MAGIC_WATCH);
            remove_affect_type (ch, AFFECT_WATCH_DIR);
            return;
        }
    }

    if ((af = get_affect (ch, MAGIC_WATCH)))
        affect_remove (ch, af);

    magic_add_affect (ch, MAGIC_WATCH, -1, 0, 0, 0, 0);

    if (!(af = get_affect (ch, MAGIC_WATCH)))
    {
        send_to_char ("There is a bug in watch.  Please let an admin "
                      "know.\n", ch);
        return;
    }

    af->a.spell.t = (long int) target;

    act ("You will now watch $N.", false, ch, 0, target, TO_CHAR | _ACT_FORMAT);
}

void
	do_mscan (CHAR_DATA * ch, char *argument, int cmd)
{
	do_scan(ch, argument, 22);
}

void
	do_oscan (CHAR_DATA * ch, char *argument, int cmd)
{
	do_scan(ch, argument, 33);
}

// 22 = mscan, only show mobiles
// 33 = oscan, only show objects.
void
do_scan (CHAR_DATA * ch, char *argument, int cmd)
{
    int dir = 0;
    int mode = 0;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    ROOM_DIRECTION_DATA *exit = NULL;
    ROOM_DATA *next_room = NULL;

    argument = one_argument (argument, buf);

    if (is_sunlight_restricted (ch))
        return;

    if (IS_SET (ch->room->room_flags, STIFLING_FOG))
    {
        send_to_char ("The thick fog in the area prevents any such attempt.\n",
                      ch);
        return;
    }

    /*
    if (IS_MORTAL (ch)
            && weather_info[ch->room->zone].state == HEAVY_SNOW
            && !IS_SET (ch->room->room_flags, INDOORS))
    {

        send_to_char ("The onslaught of snow prevents any such attempt.\n", ch);
        return;

    }
    */

    if (is_dark (ch->room) &&
            IS_MORTAL (ch) &&
            !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !is_goggled(ch)
            && !IS_SET (ch->affected_by, AFF_INFRAVIS)
            && ch->room->sector_type == SECT_UNDERWATER)
    {
        send_to_char ("You can't see a thing!\n", ch);
        return;
    }


    if (!*buf)
    {
        /* Quick one-room deep scan */

        for (dir = 0; dir <= LAST_DIR; dir++)
        {
            if ((exit = EXIT (ch, dir)) &&
                    vnum_to_room (exit->to_room) &&
                    (!(IS_SET (exit->exit_info, EX_ISDOOR) &&
                       IS_SET (exit->exit_info, EX_CLOSED)) ||
                     (IS_SET (exit->exit_info, EX_CLOSED) &&
                      IS_SET (exit->exit_info, EX_ISGATE))))
            {
                break;
            }
        }

        if (dir == LAST_DIR + 1)
        {
            send_to_char ("There is nowhere to scan.\n", ch);
            return;
        }

        /* cmd = 0 for normal scan */
        /* cmd = 1 for scan without echo */
        if (cmd != 1)
        {
            if (IS_OUTSIDE (ch))
                act ("You focus on the horizon . . .", false, ch, 0, 0, TO_CHAR);
            else
                act ("You peer into the distance . . .", false, ch, 0, 0, TO_CHAR);
        }
        area_scan(ch, cmd);
        return;
    } //if (!*buf)

    switch (*buf)
    {
    case 'n':
        dir = 0;
        break;
    case 'e':
        dir = 1;
        break;
    case 's':
        dir = 2;
        break;
    case 'w':
        dir = 3;
        break;
    case 'u':
        dir = 4;
        break;
    case 'd':
        dir = 5;
        break;
    case 'o':
        dir = 6;
        break;
    case 'i':
        dir = 7;
        break;
    default:
        send_to_char ("Which direction would you like to scan?\n", ch);
        return;
    }

    exit = EXIT (ch, dir);

    if (!exit || !(next_room = vnum_to_room (exit->to_room)))
    {
        send_to_char ("There's nothing to scan that way.\n", ch);
        return;
    }

    if (IS_OUTSIDE (ch))
    {
        if (dir != UP && dir != DOWN)
        {
            sprintf (buf, "You focus on the %s horizon . . .",
                     relative_dirs[dir]);
            act (buf, false, ch, 0, 0, TO_CHAR);
        }
        else
        {
            act ("You focus on the horizon . . .", false, ch, 0, 0, TO_CHAR);
        }
    }
    else
    {
        act ("You peer into the distance . . .", false, ch, 0, 0, TO_CHAR);
    }

    if (!str_cmp ("full", argument) || !str_cmp ("f", argument))
    {
        mode = 3;
    }

    directional_scan(ch, dir, mode, cmd);
}

// 22 = only mobiles
// 33 = only objects
void area_scan(CHAR_DATA * ch, int cmd)
{
    int dir = 0;
    ROOM_DIRECTION_DATA *exit = NULL;
    ROOM_DATA *next_room = NULL;
    char buf[AVG_STRING_LENGTH] = "";
    int char_seen = 0;
    int obj_seen = 0;

    for (dir = 0; dir <= LAST_DIR;dir++)
    {

        char_seen = 0;
        obj_seen = 0;

        if ((exit = EXIT (ch, dir))
                /*&& !IS_SET (exit->exit_info, EX_ISDOOR)*/
                && !IS_SET (exit->exit_info, EX_CLOSED)
                && (next_room = vnum_to_room (exit->to_room))
                && !IS_SET (next_room->room_flags, STIFLING_FOG))
        {
            send_to_char("\n", ch);

            // Check for stuff in the first room.
            sprintf(buf, "%s, #6%s#0, you see...\n", dirs[dir], IS_LIGHT (next_room) ? next_room->name : "#4where it is dark#0");
            *buf = toupper(*buf);
            send_to_char(buf, ch);

			if (cmd != 33)
			{
				if (next_room->people && !IS_SET (next_room->room_flags, STIFLING_FOG))
				{
					char_seen = list_char_to_char (next_room->people, ch, 7);
				}
			}

			if (cmd != 22)
			{
				if (next_room->contents && !IS_SET (next_room->room_flags, STIFLING_FOG))
				{
					obj_seen = list_obj_to_char (next_room->contents, ch, 0, 7);
				}
			}

            if (!char_seen && !obj_seen)
                send_to_char("Nothing of interest.\n", ch);

        }
    }
}

// cmd: 22 = mobiles only
// cmd: 33 = objects only
void
	directional_scan (CHAR_DATA * ch, int dir, int mode, int cmd)
{
	bool overwatched = false;
	CHAR_DATA *overwatched_ch = NULL;
	CHAR_DATA *tch = NULL;
	int blank = 0;
	int seen = 0;
	int char_seen = 0;
	int obj_seen = 0;
	float scan_base = 0;
	int penalty = 0;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;
	OBJ_DATA *helm = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	exit = EXIT (ch, dir);

	if (!exit || !(next_room = vnum_to_room (exit->to_room)))
	{
		send_to_char ("There's nothing to scan that way.\n", ch);
		return;
	}

	if (IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED) &&
		!IS_SET (exit->exit_info, EX_ISGATE))
	{
		if (!IS_LIGHT (ch->room))
			send_to_char ("You see nothing.\n", ch);
		else
			send_to_char ("Your view is blocked.\n", ch);

		return;
	}

	seen = 0;
	blank = 1;

	// Determines how easily you can spot hidden people through a first-room scan.

	scan_base = (ch->intel*2.5 + ch->wil + ch->aur*.5) - 10 + number(0, 40);

	// Wearing a covering helm or mask reduces your chance to spot hidden folk.

	if (((helm = get_equip (ch, WEAR_HEAD)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK))
		|| ((helm = get_equip (ch, WEAR_FACE)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK)))
		scan_base = scan_base / 2;

	penalty = int(scan_base);

	/*
	if (weather_info[ch->room->zone].fog == THIN_FOG)
	penalty -= 5;
	if (weather_info[ch->room->zone].fog == THICK_FOG)
	penalty -= 15;
	if (weather_info[ch->room->zone].state == STEADY_RAIN)
	penalty -= 5;
	if (weather_info[ch->room->zone].state == LIGHT_SNOW)
	penalty -= 10;
	if (weather_info[ch->room->zone].state == HEAVY_RAIN)
	penalty -= 15;
	if (weather_info[ch->room->zone].state == STEADY_SNOW)
	penalty -= 20;
	if (weather_info[ch->room->zone].state == HEAVY_SNOW)
	penalty -= 25;
	*/

	if (!next_room->psave_loaded)
		load_save_room (next_room);

	// Check for stuff in the first room.
	sprintf(buf, "\nNearby, #6%s#0, you see...\n", IS_LIGHT (next_room) ? next_room->name : "#4where it is dark#0");
	send_to_char(buf, ch);

	if (cmd != 33)
	{
		if (next_room->people && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			char_seen = list_char_to_char (next_room->people, ch, 7 + mode);

			if (!ch->over_enemies->empty())
			{
				for (tch = next_room->people; tch; tch = tch->next_in_room)
				{
					if (could_see(ch, tch) && check_overwatch(ch, tch, true))
					{
						overwatched = true;
						overwatched_ch = tch;
						break;
					}
				}
			}
		}
	}

	if (cmd != 22)
	{
		if (next_room->contents && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			obj_seen = list_obj_to_char (next_room->contents, ch, 0, 7 + mode);
		}
	}

	if (!char_seen && !obj_seen)
		send_to_char("Nothing of interest.\n", ch);


	exit = next_room->dir_option[dir];

	if (!exit || !(next_room = vnum_to_room (exit->to_room)) ||
		(IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED)))
	{
		if (overwatched && overwatched_ch)
		{
			check_overwatch(ch, overwatched_ch, false);
		}
		return;
	}

	if (!next_room->psave_loaded)
		load_save_room (next_room);

	// Check for stuff in the second room.
	char_seen = 0;
	obj_seen = 0;
	sprintf(buf, "\nFar away, #6%s#0, you see...\n", IS_LIGHT (next_room) ? next_room->name : "#4where it is dark#0");
	send_to_char(buf, ch);

	if (cmd != 33)
	{
		if (next_room->people && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			char_seen = list_char_to_char (next_room->people, ch, 8 + mode);

			if (!overwatched && !ch->over_enemies->empty())
			{
				for (tch = next_room->people; tch; tch = tch->next_in_room)
				{
					if (could_see(ch, tch) && check_overwatch(ch, tch, true))
					{
						overwatched = true;
						overwatched_ch = tch;
						break;
					}
				}
			}
		}
	}

	if (cmd != 22)
	{
		if (next_room->contents && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			obj_seen = list_obj_to_char (next_room->contents, ch, 0, 8 + mode);
		}
	}

	if (!char_seen && !obj_seen)
		send_to_char("Nothing of interest.\n", ch);

	exit = next_room->dir_option[dir];

	if (!exit || !(next_room = vnum_to_room (exit->to_room)) ||
		(IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED)))
	{
		if (overwatched && overwatched_ch)
		{
			check_overwatch(ch, overwatched_ch, false);
		}
		return;
	}

	char_seen = 0;
	obj_seen = 0;
	sprintf(buf, "\nVery far away%s you see...\n", IS_LIGHT (next_room) ? "" : ",#4 where it is dark#0,");
	send_to_char(buf, ch);


	if (cmd != 33)
	{
		if (next_room->people && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			char_seen = list_char_to_char (next_room->people, ch, 9 + mode);

			if (!overwatched && !ch->over_enemies->empty())
			{
				for (tch = next_room->people; tch; tch = tch->next_in_room)
				{
					if (could_see(ch, tch) && check_overwatch(ch, tch, true))
					{
						overwatched = true;
						overwatched_ch = tch;
						break;
					}
				}
			}
		}
	}

	if (cmd != 22)
	{
		if (next_room->contents && !IS_SET (next_room->room_flags, STIFLING_FOG))
		{
			obj_seen = list_obj_to_char (next_room->contents, ch, 0, 9 + mode);
		}
	}

	if (!char_seen && !obj_seen)
		send_to_char("Nothing of interest.\n", ch);

	if (overwatched && overwatched_ch)
	{
		check_overwatch(ch, overwatched_ch, false);
	}
}

ROOM_DATA *
get_diagonal_scan_room (CHAR_DATA * ch, int nDirOne, int nDirTwo)
{
    ROOM_DIRECTION_DATA *ptrTmpExit = NULL;
    ROOM_DATA *ptrTmpRoom = NULL;

    if ((ptrTmpExit = EXIT (ch, nDirOne))
            && ((!IS_SET (ptrTmpExit->exit_info, EX_ISDOOR))
                && (ptrTmpRoom = vnum_to_room (ptrTmpExit->to_room))
                && (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
                && (ptrTmpRoom->sector_type != SECT_INSIDE)
                && (ptrTmpRoom->sector_type != SECT_UNDERWATER)
                && (!IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG))
                && (ptrTmpExit = ptrTmpRoom->dir_option[nDirTwo])
                && (!IS_SET (ptrTmpExit->exit_info, EX_ISDOOR))
                && (ptrTmpRoom = vnum_to_room (ptrTmpExit->to_room))
                && (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
                && (ptrTmpRoom->sector_type != SECT_INSIDE)
                && (ptrTmpRoom->sector_type != SECT_UNDERWATER)
                && (!IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG))))
    {

        return ptrTmpRoom;

    }

    return NULL;
}


/*
 *	diagonal scan - quick method
 *
 *	called if we just checked dir x from an outdoor room
 *
 *	1. start with a 100% penalty
 *	2. if the room in dir x is outdoor -34%
 *	3. if the room in dir x+1 is outdoor -34%
 *	4. if the there's a joining room that is outdoor scan & delay
 *      5. resume normal scan
 *
 *	A---B	If we are at room 'N' and just scanned north to 'A'
 *	|    Y  and are about to scan east to 'X' then we have a
 *	|    |	chance of seeing 'B' east of 'A' as well as 'Y' north
 *	N----X	of 'X' in many cases 'Y' and 'B' will be the same.
 *
 */
const char *diag_dirs[] =
{
    "northeast",
    "southeast",
    "southwest",
    "northwest",
    "\n"
};

void
do_qscan (CHAR_DATA * ch, char *argument, int cmd)
{
    do_scan (ch, "north", 1);
    send_to_char("\n", ch);
    do_scan (ch, "east", 1);
    send_to_char("\n", ch);
    do_scan (ch, "south", 1);
    send_to_char("\n", ch);
    do_scan (ch, "west", 1);
    return;
}

void
do_count (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    OBJ_DATA *obj = NULL;

    argument = one_argument (argument, buf);

    if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !is_goggled(ch)
            && IS_MORTAL (ch) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
        send_to_char ("It's too dark to count credits.\n", ch);
        return;
    }

    if (!*buf)
    {
        sprintf (buf,
                 "You begin searching through your belongings, taking a tally of your credit.\n");
        send_to_char (buf, ch);
    }
    else
    {
        if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
                !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
        {
            send_to_char ("I don't see that group of credits.\n", ch);
            return;
        }

        if (GET_ITEM_TYPE (obj) != ITEM_MONEY)
        {
            send_to_char ("That isn't a group of credits.\n", ch);
            return;
        }

        sprintf (buf,
                 "After a moment of sorting, you determine that there are %d credits in the pile.",
                 obj->count);
        act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }

    ch->delay = 10;

    ch->delay_type = DEL_COUNT_COIN;
}

char *
coin_sdesc (OBJ_DATA * coin)
{
    if (coin->nVirtual == 50093)
    {
        if (coin->count > 1)
            return "'two-hundred-forty'-marked pieces of scrip";
        else
            return "'two-hundred-forty'-marked piece of scrip";
    }
    else if (coin->nVirtual == 50092)
    {
        if (coin->count > 1)
            return "'twenty-five'-marked pieces of scrip";
        else
            return "'twenty-five'-marked piece of scrip";
    }
    else if (coin->nVirtual == 50091)
    {
        if (coin->count > 1)
            return "'five'-marked pieces of scrip";
        else
            return "'five'-marked piece of scrip";
    }
    else if (coin->nVirtual == 50090)
    {
        if (coin->count > 1)
            return "'one'-marked pieces of scrip";
        else
            return "'one'-marked piece of scrip";
    }
    else if (coin->nVirtual == 14011)
    {
        if (coin->count > 1)
            return "white, circular chips";
        else
            return "white, circular chip";
    }
    else if (coin->nVirtual == 14012)
    {
        if (coin->count > 1)
            return "red, circular chips";
        else
            return "red, circular chip";
    }
    else if (coin->nVirtual == 14013)
    {
        if (coin->count > 1)
            return "blue, circular chips";
        else
            return "blue, circular chip";
    }
    else if (coin->nVirtual == 14014)
    {
        if (coin->count > 1)
            return "yellow, circular chips";
        else
            return "yellow, circular chip";
    }
    else if (coin->nVirtual == 14015)
    {
        if (coin->count > 1)
            return "green, circular chips";
        else
            return "green, circular chip";
    }
    else if (coin->nVirtual == 14016)
    {
        if (coin->count > 1)
            return "black, circular chips";
        else
            return "black, circular chip";
    }
    else if (coin->nVirtual == 14017)
    {
        if (coin->count > 1)
            return "gold-flecked, circular chips";
        else
            return "gold-flecked, circular chip";
    }




    return "ration";
}

void
delayed_count_coin (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    OBJ_DATA *obj = NULL, *tobj = NULL;
    int money = 0, location = 0;

    *buf2 = '\0';

    if (ch->right_hand)
    {
        if (GET_ITEM_TYPE (ch->right_hand) == ITEM_MONEY)
        {
            money += (int) ch->right_hand->farthings * ch->right_hand->count;
            tobj = ch->right_hand;
            sprintf (buf2 + strlen (buf2),
                     "   #2%d %s#0 (#2right hand#0): %d credits\n", tobj->count,
                     coin_sdesc (tobj), (int) tobj->farthings * tobj->count);
        }
        else if (GET_ITEM_TYPE (ch->right_hand) == ITEM_CONTAINER)
        {
            for (tobj = ch->right_hand->contains; tobj;
                    tobj = tobj->next_content)
            {
                if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
                {
                    money += (int) tobj->farthings * tobj->count;
                    sprintf (buf2 + strlen (buf2),
                             "   #2%d %s#0 (#2%s#0): %d credits\n", tobj->count,
                             coin_sdesc (tobj), obj_short_desc (tobj->in_obj),
                             (int) tobj->farthings * tobj->count);
                }
            }
        }
    }

    if (ch->left_hand)
    {
        if (GET_ITEM_TYPE (ch->left_hand) == ITEM_MONEY)
        {
            money += (int) ch->left_hand->farthings * ch->left_hand->count;
            tobj = ch->left_hand;
            sprintf (buf2 + strlen (buf2),
                     "   #2%d %s#0 (#2left hand#0): %d credits\n", tobj->count,
                     coin_sdesc (tobj), (int) tobj->farthings * tobj->count);
        }
        else if (GET_ITEM_TYPE (ch->left_hand) == ITEM_CONTAINER)
        {
            for (tobj = ch->left_hand->contains; tobj;
                    tobj = tobj->next_content)
            {
                if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
                {
                    money += (int) tobj->farthings * tobj->count;
                    sprintf (buf2 + strlen (buf2),
                             "   #2%d %s#0 (#2%s#0): %d credits\n", tobj->count,
                             coin_sdesc (tobj), obj_short_desc (tobj->in_obj),
                             (int) tobj->farthings * tobj->count);
                }
            }
        }
    }

    for (location = 0; location < MAX_WEAR; location++)
    {
        if (!(obj = get_equip (ch, location)))
            continue;
        for (tobj = obj->contains; tobj; tobj = tobj->next_content)
        {
            if (GET_ITEM_TYPE (tobj) == ITEM_MONEY)
            {
                money += (int) tobj->farthings * tobj->count;
                sprintf (buf2 + strlen (buf2),
                         "   #2%d %s#0 (#2%s#0): %d credits\n", tobj->count,
                         coin_sdesc (tobj), obj_short_desc (obj),
                         (int) tobj->farthings * tobj->count);
            }
        }
    }

    if (!money)
    {
        send_to_char ("You don't seem to have any credits.\n", ch);
        return;
    }

    sprintf (buf, "By your count, you have %d credits' worth:", money);
    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    send_to_char ("\n", ch);
    send_to_char (buf2, ch);
}

void
stop_counting (CHAR_DATA * ch)
{
    send_to_char ("You bore of counting credits.\n", ch);

    ch->delay = 0;

    ch->delay_obj = NULL;
}

void
tracking_system_response (CHAR_DATA * ch, MESSAGE_DATA * message)
{
    CHAR_DATA *tch = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char from[MAX_STRING_LENGTH] = { '\0' };
    char subject[255] = { '\0' };
    char type[15] = { '\0' };

    if (!message)
        return;

    if (!*ch->descr()->pending_message->message)
        return;

    account acct (message->poster);
    if (!acct.is_registered ())
    {
        if (!(tch = load_pc (message->poster)))
            return;
        else
            acct = account (tch->pc->account_name);
    }

    if (!acct.is_registered ())
    {
        if (tch != NULL)
            unload_pc (tch);
        return;
    }

    if (strcasecmp (ch->delay_who, "Bugs") == STR_MATCH)
        sprintf (type, "Bug");
    else if (strcasecmp (ch->delay_who, "Typos") == STR_MATCH)
        sprintf (type, "Typo");
    else if (strcasecmp (ch->delay_who, "Ideas") == STR_MATCH)
        sprintf (type, "Idea");
    else if (strcasecmp (ch->delay_who, "Submissions") == STR_MATCH)
        sprintf (type, "Writing Submission");
    else if (strcasecmp (ch->delay_who, "Petitions") == STR_MATCH)
        sprintf (type, "Petition");
    else
        sprintf (type, "Issue");

    sprintf (buf, "%s"
             "\n\n-- Original Report, Filed %s --\n\n"
             "%s", ch->descr()->pending_message->message, message->date,
             message->message);

    sprintf (from, "%s <%s>", ch->tname, ch->descr()->acct->email.c_str ());

    if (strcasecmp (type, "Writing Submission") == STR_MATCH)
    {
        sprintf (subject, "Re: Your Website Submission");
        send_email (&acct, REPORT_EMAIL, from, subject, buf);
    }
    else if (strcasecmp (type, "Petition") == STR_MATCH)
    {
        sprintf (subject, "Re: Your Logged Petition");
        send_email (&acct, PET_EMAIL, from, subject, buf);
    }
    else
    {
        sprintf (subject, "Re: Your %s Report", type);
        send_email (&acct, REPORT_EMAIL, from, subject, buf);
    }

    unload_pc (tch);
}

void
post_track_response (DESCRIPTOR_DATA * d)
{
    MESSAGE_DATA *message = NULL;
    MYSQL_RES *result = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    *buf = '\0';

    if (strcasecmp (d->character->delay_who, "Typos") == STR_MATCH)
        mysql_safe_query
        ("UPDATE newsletter_stats SET resolved_typos=resolved_typos+1");
    else if (strcasecmp (d->character->delay_who, "Bugs") == STR_MATCH)
        mysql_safe_query
        ("UPDATE newsletter_stats SET resolved_bugs=resolved_bugs+1");

    message =
        load_message (d->character->delay_who, 5, d->character->delay_info1);
    if (!*d->pending_message->message)
    {
        send_to_char ("No email sent.\n", d->character);
    }
    else
        send_to_char ("The report poster has been sent an email notification.\n",
                      d->character);
    tracking_system_response (d->character, message);

    mysql_safe_query
    ("DELETE FROM virtual_boards WHERE post_number = \'%d\' AND board_name = \'%s\'",
     d->character->delay_info1, d->character->delay_who);

    if (mysql_safe_query
            ("SELECT * FROM virtual_boards WHERE board_name = \'%s\'",
             d->character->delay_who))
        return;
    result = mysql_store_result (database);
    if (mysql_num_rows (result) == 0)
    {
        sprintf (buf, "vboards/%s", d->character->delay_who);
        unlink (buf);
    }

    d->character->delay_who = NULL;
    d->character->delay_info1 = 0;

    mysql_free_result (result);
    result = NULL;

    unload_message (message);
}

int
erase_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
    FILE *fp = NULL;
    unsigned int i = 0;
    CHAR_DATA *who = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (strlen (name) > 15)
        return 0;

    for (i = 0; i <= strlen (name); i++)
        if (!isalpha (*name))
            return 0;

    *name = toupper (*name);

    if (!(who = load_pc (name)))
        return 0;

    if (who->pc->level > ch->pc->level)
    {
        return 0;
    }

    unload_pc (who);

    if (!isdigit (*argument))
        return 0;

    if (atoi (argument) > 255)
        return 0;

    sprintf (buf, "player_boards/%s.%06d", name, atoi (argument));
    if (!(fp = fopen (buf, "r")))
    {
        return 0;
    }
    else
        fclose (fp);

    sprintf (buf, "rm player_boards/%s.%06d", name, atoi (argument));
    system (buf);

    return 1;
}

int
erase_journal (CHAR_DATA * ch, char *argument)
{
    FILE *fp = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!isdigit (*argument))
    {
        send_to_char ("The specified journal entry must be a number.\n", ch);
        return 1;
    }

    if (atoi (argument) > 255)
    {
        send_to_char ("Enter a number between 1 and 255 to erase.\n", ch);
        return 1;
    }

    sprintf (buf, "player_journals/%s.%06d", ch->tname, atoi (argument));
    if (!(fp = fopen (buf, "r")))
    {
        send_to_char ("That journal entry could not be found.\n", ch);
        return 1;
    }
    else
        fclose (fp);

    sprintf (buf, "rm player_journals/%s.%06d", ch->tname, atoi (argument));
    system (buf);

    send_to_char ("The specified journal entry has been erased.\n", ch);

    return 1;
}

void
do_jerase (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Journals are unavailable to guests.\n", ch);
        return;
    }

    if (!*buf)
    {
        send_to_char ("Which message do you wish to erase?\n", ch);
        return;
    }

    if (!erase_mysql_board_post (ch, ch->tname, 3, buf))
    {
        send_to_char ("I couldn't find that journal entry.\n", ch);
        return;
    }

    send_to_char ("Journal entry erased successfully.\n", ch);
}

void
do_erase (CHAR_DATA * ch, char *argument, int cmd)
{
    int msg_no = 0;
    MESSAGE_DATA *message = NULL;
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("Which message do you wish to erase?\n", ch);
        return;
    }

    if (IS_NPC (ch))
    {
        send_to_char ("You can't do this while switched.\n", ch);
        return;
    }

    if (!isdigit (*buf))
    {
        if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
                !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
                !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
        {
            if (IS_MORTAL (ch)
                    || !erase_mysql_board_post (ch, buf, 2, argument))
            {
                if (IS_MORTAL (ch)
                        || !erase_mysql_board_post (ch, buf, 1, argument))
                {
                    if (IS_MORTAL (ch))
                        send_to_char ("You do not see that board here.\n", ch);
                    else
                        send_to_char
                        ("Either the specified board is not here, or the specified message does not exist.\n",
                         ch);
                    return;
                }
                else
                {
                    send_to_char ("The specified message has been erased.\n",
                                  ch);
                    return;
                }
            }
            else
            {
                send_to_char ("The specified message has been erased.\n", ch);
                return;
            }
        }

        if (obj->obj_flags.type_flag != ITEM_BOARD)
        {
            act ("$p isn't a board.", false, ch, obj, 0, TO_CHAR);
            return;
        }

        one_argument (obj->name, buf2);
        argument = one_argument (argument, buf);
    }

    else
    {
        if (!(obj = get_obj_in_list_vis (ch, "board", ch->room->contents)) &&
                !(obj = get_obj_in_list_vis (ch, "board", ch->left_hand)) &&
                !(obj = get_obj_in_list_vis (ch, "board", ch->right_hand)))
        {
            send_to_char ("You do not see that board here.\n", ch);
            return;
        }

        if (obj->obj_flags.type_flag != ITEM_BOARD)
        {
            act ("$p, the first board here, isn't a real board.",
                 false, ch, obj, 0, TO_CHAR);
            return;
        }

        one_argument (obj->name, buf2);
    }

    if (!isdigit (*buf))
    {
        send_to_char ("Expected a message number to erase.\n", ch);
        return;
    }

    msg_no = atoi (buf);

    if (!(message = load_message (buf2, 6, msg_no)))
    {
        send_to_char ("That message doesn't exist.\n", ch);
        return;
    }

    if (!GET_TRUST (ch)
            && strcasecmp (GET_NAME (ch), message->poster) != STR_MATCH)
    {
        send_to_char ("You can only erase your own messages.\n", ch);
        unload_message (message);
        return;
    }

    if (erase_mysql_board_post (ch, buf2, 0, buf))
        send_to_char ("The specified message has been erased.\n", ch);
    else
        send_to_char ("There was a problem erasing that message.\n", ch);

    unload_message (message);
}

int
write_virtual_board (CHAR_DATA * ch, char *name, char *argument)
{
    if (strlen (name) > 15)
        return 0;

    if (!isalpha (*name))
        return 0;

    *name = toupper (*name);

    if (!*argument)
    {
        send_to_char ("Please include a subject for your post.\n", ch);
        return 0;
    }

    while (*argument == ' ')
        argument++;

    ch->descr()->pending_message =
        (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);

    /* We need to borrow the poster slot to save the board name */

    ch->descr()->pending_message->poster = add_hash (name);
    ch->descr()->pending_message->message = NULL;
    ch->descr()->pending_message->nVirtual = -2;
    ch->descr()->pending_message->info = add_hash ("");
    ch->descr()->pending_message->subject = add_hash (argument);
    ch->descr()->pending_message->flags = MF_READ;

    make_quiet (ch);

    send_to_char ("Enter your note, terminate with an '@'\n\n", ch);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_to_mysql_virtual_board;

    return 1;
}

void
post_player_message (DESCRIPTOR_DATA * d)
{
    char date[32] = "";
    time_t current_time = 0;

    current_time = time (0);

    ctime_r (&current_time, date);

    /* asctime adds a \n to the end of the date string - remove it */

    if (strlen (date) > 1)
        date[strlen (date) - 1] = '\0';

    if (!*d->pending_message->message)
        send_to_char ("No message posted.\n", d->character);
    else
        add_message (1,		/* New message */
                     d->pending_message->poster,	/* name */
                     d->pending_message->nVirtual,	/* virtual */
                     GET_NAME (d->character),	/* poster */
                     date,		/* date */
                     d->pending_message->subject,	/* subject */
                     d->pending_message->info,	/* info */
                     d->pending_message->message,	/* message */
                     d->pending_message->flags);

    unload_message (d->pending_message);

    d->pending_message = NULL;
}

int
write_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
    unsigned int i = 0;
    CHAR_DATA *who = NULL;

    if (strlen (name) > 15)
        return 0;

    for (i = 0; i <= strlen (name); i++)
        if (!isalpha (*name))
            return 0;

    *name = toupper (*name);

    if (!(who = load_pc (name)))
        return 0;

    unload_pc (who);

    if (!*argument)
    {
        send_to_char ("Please include a subject for your post.\n", ch);
        return 0;
    }

    while (*argument == ' ')
        argument++;

    ch->descr()->pending_message =
        (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);

    /* We need to borrow the poster slot to save the board name */

    ch->descr()->pending_message->poster = add_hash (name);
    ch->descr()->pending_message->message = NULL;
    ch->descr()->pending_message->nVirtual = -1;
    ch->descr()->pending_message->info = add_hash ("");
    ch->descr()->pending_message->subject = add_hash (argument);
    ch->descr()->pending_message->flags = 0;

    make_quiet (ch);

    send_to_char ("Enter your note, terminate with an '@'\n\n", ch);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_to_mysql_player_board;

    return 1;
}
/// \todo merge with post_player_message
void
post_journal (DESCRIPTOR_DATA * d)
{
    char date[32] = "";
    time_t current_time = 0;

    current_time = time (0);

    ctime_r (&current_time, date);

    /* asctime adds a \n to the end of the date string - remove it */

    if (strlen (date) > 1)
        date[strlen (date) - 1] = '\0';

    if (!*d->pending_message->message)
        send_to_char ("No journal entry added.\n", d->character);
    else
        add_message (5,		/* New message */
                     d->pending_message->poster,	/* name */
                     -3,		/* virtual */
                     GET_NAME (d->character),	/* poster */
                     date,		/* date */
                     d->pending_message->subject,	/* subject */
                     d->pending_message->info,	/* info */
                     d->pending_message->message,	/* message */
                     d->pending_message->flags);

    unload_message (d->pending_message);

    d->pending_message = NULL;
}

int
write_journal (CHAR_DATA * ch, char *argument)
{

    if (!*argument)
    {
        send_to_char ("Please include a subject for your post.\n", ch);
        return 0;
    }

    while (*argument == ' ')
        argument++;

    ch->descr()->pending_message =
        (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);

    /* We need to borrow the poster slot to save the board name */

    ch->descr()->pending_message->poster = add_hash (ch->tname);
    ch->descr()->pending_message->message = NULL;
    ch->descr()->pending_message->nVirtual = -1;
    ch->descr()->pending_message->info = add_hash ("");
    ch->descr()->pending_message->subject = add_hash (argument);

    make_quiet (ch);

    send_to_char ("Type your journal entry; terminate with an '@'\n\n", ch);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_to_mysql_journal;

    return 1;
}

void do_write (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char title[MAX_STRING_LENGTH] = { '\0' };

    if (IS_NPC (ch))
    {
        send_to_char ("Write is only available to PCs.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("What did you want to write on?\n", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
    {
        if (IS_MORTAL (ch) || !write_pc_board (ch, buf, argument))
        {
            if (IS_MORTAL (ch) || !write_virtual_board (ch, buf, argument))
                send_to_char ("You can't find that.\n", ch);
        }
        return;
    }

    if ( USES_BOOK_CODE( obj ) )
    {
        do_scribe( ch, buf, 0 );
        return;
    }

    if (!*argument)
    {
        send_to_char ("What would you like to write about?\n", ch);
        return;
    }

    if (obj->obj_flags.type_flag != ITEM_BOARD)
    {
        act ("You can't write on $p.", false, ch, obj, 0, TO_CHAR);
        return;
    }

    // If we've got oval5 as 1, we need to make public who posted this:
    if (obj->o.od.value[5] == 1 && !GET_TRUST(ch))
        ch->delay_info5 = 555;

    while (*argument == ' ')
        argument++;

    /* Get the name of the board */

    one_argument (obj->name, buf);

    strcpy (title, argument);

    CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);

    /* We need to borrow the poster slot to save the board name */

    ch->descr()->pending_message->poster = add_hash (buf);
    ch->descr()->pending_message->message = NULL;
    ch->descr()->pending_message->nVirtual = 0;
    ch->descr()->pending_message->info = add_hash ("");
    ch->descr()->pending_message->subject = add_hash (title);

    ch->descr()->pending_message->flags = 0;

    send_to_char
    ("\n#6Enter your message below. To terminate, use the '@' character. Please ensure\n"
     "you use proper linebreaks, and that your writing follows the acceptable posting\n"
     "policies for our in-game boards as outlined in HELP POSTING_POLICIES.#0\n\n"
     "1-------10--------20--------30--------40--------50--------60--------70--------80\n",
     ch);

    make_quiet (ch);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_message;
}

void
do_jwrite (CHAR_DATA * ch, char *argument, int cmd)
{

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Journals are unavailable to guests.\n", ch);
        return;
    }

    if (IS_NPC (ch))
    {
        send_to_char ("Journals are only available to PCs.\n", ch);
        return;
    }

    if (!*argument)
    {
        send_to_char ("What did you wish your subject to be?\n", ch);
        return;
    }

    if (!write_journal (ch, argument))
    {
        send_to_char ("There seems to be a problem with your journal.\n", ch);
        return;
    }
}

int
read_journal_message (CHAR_DATA * ch, CHAR_DATA * reader, char *argument)
{
    MESSAGE_DATA *message = NULL;
    char name[MAX_STRING_LENGTH] = { '\0' };

    sprintf (name, "%s", ch->tname);

    if (!atoi (argument))
    {
        if (!reader)
            send_to_char ("Which entry?\n", ch);
        else
            send_to_char ("Which entry?\n", reader);
        return 1;
    }

    if (!(message = load_message (name, 8, atoi (argument))))
    {
        if (!reader)
            send_to_char ("No such journal entry.\n", ch);
        else
            send_to_char ("No such journal entry.\n", reader);
        return 1;
    }

    sprintf (b_buf, "#6Date:#0    %s\n"
             "#6Subject:#0 %s\n\n%s", message->date, message->subject,
             message->message);

    if (!reader)
    {
        send_to_char ("\n", ch);
        page_string (ch->descr(), b_buf);
    }
    else
    {
        send_to_char ("\n", reader);
        page_string (reader->descr(), b_buf);
    }

    unload_message (message);

    return 1;
}

void
do_jread (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *who = NULL;
    char name[MAX_STRING_LENGTH] = { '\0' };

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Journals are unavailable to guests.\n", ch);
        return;
    }

    if (IS_NPC (ch))
    {
        send_to_char ("Journals are only available to PCs.\n", ch);
        return;
    }

    if (!IS_MORTAL (ch))
    {
        if (!*argument)
            who = ch;
        else if (*argument && isdigit (*argument))
            who = ch;
        else
        {
            argument = one_argument (argument, name);
            *name = toupper (*name);
            if (!(who = load_pc (name)))
            {
                send_to_char ("No such PC, I'm afraid.\n", ch);
                return;
            }
        }
        if (!read_journal_message (who, ch, argument))
        {
            send_to_char ("There seems to be a problem with the journal.\n",
                          ch);
            return;
        }
    }
    else
    {
        if (!read_journal_message (ch, NULL, argument))
        {
            send_to_char ("There seems to be a problem with the journal.\n",
                          ch);
            return;
        }
    }
}

void
post_message (DESCRIPTOR_DATA * d)
{
    if (!*d->pending_message->message)
        send_to_char ("No message posted.\n", d->character);
    else
        post_to_mysql_board (d);
}

void
add_board (int level, char *name, char *title)
{
    BOARD_DATA *board = NULL;
    BOARD_DATA *board_entry = NULL;

    /* Make sure this board doesn't already exist */

    if (board_lookup (name))
        return;

    CREATE (board_entry, BOARD_DATA, 1);

    board_entry->level = level;
    board_entry->name = add_hash (name);
    board_entry->title = add_hash (title);
    board_entry->next_virtual = 1;

    board_entry->next = NULL;

    /* Add board_entry to end of full_board_list */

    if (!full_board_list)
        full_board_list = board_entry;

    else
    {
        board = full_board_list;

        while (board->next)
            board = board->next;

        board->next = board_entry;
    }
}

BOARD_DATA *
board_lookup (const char *name)
{
    BOARD_DATA *board = NULL;

    for (board = full_board_list; board; board = board->next)
    {
        if (strcasecmp (board->name, name) == STR_MATCH)
            return board;
    }

    return NULL;
}

void
show_unread_messages (CHAR_DATA * ch)
{
    int header = 1;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[AVG_STRING_LENGTH] = "";
    char query[MAX_STRING_LENGTH] = { '\0' };
    MYSQL_RES *result = NULL;
    MYSQL_ROW row = NULL;

    mysql_safe_query
    ("SELECT * FROM player_notes WHERE name = '%s' AND flags = 0 ORDER BY post_number DESC",
     ch->tname);
    result = mysql_store_result (database);

    *b_buf = '\0';

    while ((row = mysql_fetch_row (result)))
    {
        if (header)
        {
            sprintf (b_buf, "\nUnread messages on your private board:\n\n");
            header = false;
        }
        if (strlen (row[2]) > 44)
        {
            sprintf (query, "%s", row[2]);
            query[41] = '.';
            query[42] = '.';
            query[43] = '.';
            query[44] = '\0';
        }
        else
            sprintf (query, "%s", row[2]);
        sprintf (b_buf + strlen (b_buf), " #6%3d#0 - %16s %-10.10s: %s\n",
                 atoi (row[1]), row[4], row[3], query);
    }

    if (!header)			/* Meaning, we have something to print */
        page_string (ch->descr(), b_buf);

    mysql_free_result (result);
    result = NULL;

    mysql_safe_query
    ("SELECT board_name, MIN(post_number) post_from, MAX(post_number) post_to, author, subject FROM virtual_boards WHERE timestamp >= %d GROUP BY board_name ORDER BY board_name ASC",
     (int) ch->pc->last_logon);
    result = mysql_store_result (database);
    header = true;

    while ((row = mysql_fetch_row (result)))
    {
        if (header)
        {
            sprintf (b_buf, "\nWelcome back! Since you last logged in:\n\n");
            send_to_char (b_buf, ch);
            header = false;
        }
        if (strcmp (row[1], row[2]) == 0)
        {
            if (strcmp (row[0], "Applications") == 0)
            {
                if (strstr (row[4], "Accepted"))
                {
                    sprintf (buf2, "#6%s", row[4] + 14);
                }
                else
                {
                    sprintf (buf2, "#1%s", row[4] + 14);
                }
            }
            else if (strcmp (row[0], "Crashes") == 0)
            {
                sprintf (buf2, "#6%s", row[3]);
            }
            else if (strcmp (row[0], "Helpfiles") == 0)
            {
                sprintf (buf2, "#6%s", row[4] + 15);
            }
            else if (strcmp (row[0], "Petitions") == 0)
            {
                sprintf (buf2, "#6%s", row[3]);
            }
            else if (strcmp (row[0], "Prisoners") == 0)
            {
                sprintf (buf2, "#6%s", row[4] + 12);
            }
            else if (strcmp (row[0], "Submissions") == 0)
            {
                row[4][strlen (row[4] - 2)] = '\0';
                sprintf (buf2, "#6%s", row[4] + 2);
            }
            else
            {
                sprintf (buf2, "#6%s", row[4]);
            }
            if (strlen (buf2) > 28)
            {
                strcpy (buf2 + 25, "...");
            }
            sprintf (buf, "   - '#2%s#0' was posted to (msg %s: %s#0).\n",
                     row[0], row[1], buf2);
        }
        else
        {
            sprintf (buf, "   - '#2%s#0' was posted to (msgs %s to %s).\n",
                     row[0], row[1], row[2]);
        }
        send_to_char (buf, ch);
    }

    mysql_free_result (result);
}

void
add_message (int new_message, const char *name, int nVirtual, const char *poster,
             char *date, char *subject, char *info, char *message, long flags)
{
    int named = 0, day, i = 0;
    int month = 0;
    MESSAGE_DATA *msg = NULL;
    BOARD_DATA *board = NULL;
    CHAR_DATA *ch = NULL;
    DIR *dir = NULL;
    FILE *fp = NULL;
    bool found = false;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char msg_file_name[MAX_STRING_LENGTH] = { '\0' };
    char date_buf[MAX_STRING_LENGTH] = { '\0'};
    //char *suf = '\0';
    time_t current_time = 0;

    if (!new_message)
    {
        system_log ("Add_mesage() called with new_message = 0", true);
        abort ();
    }

    if (!date)
    {
        /// \todo see who is responsible for deleteing the mem alloced by ctime
        current_time = time (0);
        date = ctime (&current_time);
        if (strlen (date) > 1)
            date[strlen (date) - 1] = '\0';
    }

    /* 1 means put a new message to a board (nVirtual == -1 to pc board */
    /* 2 means update a pc board message */

    if (new_message != 5 && new_message != 2 && nVirtual != -5 && nVirtual != -2
            && nVirtual != -1 && !(board = board_lookup (name)))
    {
        printf ("No board for message; board:  '%s'.\n", name);
        return;
    }

    msg = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);

    if (nVirtual == -1)
    {

        if (!(ch = load_pc (name)))
        {
            system_log ("No such character by name in add_message()!", true);
            return;
        }

        nVirtual = ++ch->pc->staff_notes;
        named = 1;

        unload_pc (ch);
    }

    else if (nVirtual == -2)
    {
        new_message = 4;
    }

    else if (nVirtual == -5)
    {
        new_message = 3;
    }

    else if (nVirtual == -3)
    {
        if (!(dir = opendir (JOURNAL_DIR)))
        {
            perror ("opendir");
            printf ("Unable to open journal directory.");
            return;
        }
        for (i = 1; i <= 5000; i++)
        {
            found = false;
            sprintf (buf, "player_journals/%s.%06d", name, i);
            if (!(fp = fopen (buf, "r")))
                break;
            else
                fclose (fp);
        }
    }
    else if (new_message == 2 || new_message == 3 || new_message == 4
             || new_message == 5)
        ;
    else if (!nVirtual)
        nVirtual = board->next_virtual++;
    else if (nVirtual >= board->next_virtual)
        board->next_virtual = nVirtual + 1;
    else
        system_log
        ("Virtual requested less than board's next virtual, add_message()",
         true);

    *date_buf = '\0';

    day = time_info.day;
    month = time_info.month;

    if (time_info.holiday > 0)
        sprintf (date_buf, "%s, %d AD", holiday_short_names[time_info.holiday], time_info.year);
    else
        sprintf (date_buf, "%s/%d AD", short_time_string(day, time_info.month), time_info.year);

    if (isalpha (*date_buf))
        *date_buf = toupper (*date_buf);

    if (new_message != 3 && new_message != 4 && new_message != 5)
        msg->nVirtual = nVirtual;
    else
        msg->nVirtual = i;
    msg->flags = flags;
    msg->poster = add_hash (poster);
    msg->date = add_hash (date);
    msg->subject = tilde_eliminator (subject);
    msg->info = add_hash (info);
    msg->message = tilde_eliminator (message);
    msg->icdate = add_hash (date_buf);

    if (named || new_message == 2)
        sprintf (msg_file_name, PLAYER_BOARD_DIR "/%s.%06d", name, nVirtual);
    else if (new_message != 2 && new_message != 3 && new_message != 4
             && new_message != 5 && ch)
    {
        ch->descr()->pending_message = msg;
        post_to_mysql_board (ch->descr());
        return;
    }
    else if (new_message == 5)
    {
        sprintf (msg_file_name, JOURNAL_DIR "/%s.%06d", name, i);
    }
    else if (new_message == 3)
    {
        add_message_to_mysql_vboard (name, poster, msg);
        return;
    }
    else if (new_message == 4)
    {
        add_message_to_mysql_player_notes (name, poster, msg);
        return;
    }

    if (!new_message)
    {
        unload_message (msg);
        return;
    }

    system_log ("Reached end of add_message()", true);

    unload_message (msg);
}


char *
read_a_line (FILE * fp)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };

    fgets (buf, MAX_STRING_LENGTH, fp);

    if (*buf)
        buf[strlen (buf) - 1] = '\0';

    return str_dup (buf);
}

MESSAGE_DATA *
load_message (char *msg_name, int pc_message, int msg_number)
{
    MESSAGE_DATA *message = NULL;
    FILE *fp_message = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' }, date_buf[MAX_STRING_LENGTH] =
    {
        '\0'
    }, *suf = '\0';
    int day = 0;
    bool tilde = false;

    *date_buf = '\0';

    if (pc_message == 1)
        sprintf (buf, PLAYER_BOARD_DIR "/%s.%06d", msg_name, msg_number);
    else if (pc_message == 2)
        sprintf (buf, VIRTUAL_BOARD_DIR "/%s.%06d", msg_name, msg_number);
    else if (pc_message == 3)
        sprintf (buf, JOURNAL_DIR "/%s.%06d", msg_name, msg_number);
    else
        sprintf (buf, BOARD_DIR "/%s.%06d", msg_name, msg_number);

    if (pc_message == 5)
    {
        message = load_mysql_message (msg_name, 1, msg_number);
        return message;
    }
    else if (pc_message == 6)
    {
        message = load_mysql_message (msg_name, 0, msg_number);
        return message;
    }
    else if (pc_message == 7)
    {
        message = load_mysql_message (msg_name, 2, msg_number);
        return message;
    }
    else if (pc_message == 8)
    {
        message = load_mysql_message (msg_name, 3, msg_number);
        return message;
    }

    if (!(fp_message = fopen (buf, "r+")))
    {
        printf ("Couldn't open %s\n", buf);
        return NULL;
    }

    message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);

    message->nVirtual = msg_number;
    message->poster = read_a_line (fp_message);
    message->date = read_a_line (fp_message);
    message->subject = read_a_line (fp_message);
    message->info = read_a_line (fp_message);
    message->message = fread_string (fp_message);
    fscanf (fp_message, "%ld", &message->flags);

    day = time_info.day + 1;
    if (day == 1)
        suf = "st";
    else if (day == 2)
        suf = "nd";
    else if (day == 3)
        suf = "rd";
    else if (day < 20)
        suf = "th";
    else if ((day % 10) == 1)
        suf = "st";
    else if ((day % 10) == 2)
        suf = "nd";
    else if ((day % 10) == 3)
        suf = "rd";
    else
        suf = "th";

    sprintf (date_buf, "%s", read_a_line (fp_message));
    sprintf (date_buf, "%s", read_a_line (fp_message));

	day = time_info.day;

    int month = 0;
    month = time_info.month;

    if (!*date_buf)
    {
        if (time_info.holiday > 0)
            sprintf (date_buf, "%s, %d AD", holiday_short_names[time_info.holiday], time_info.year);
        else
            sprintf (date_buf, "%s, %d AD", short_time_string(day, time_info.month), time_info.year);
    }


    if (isalpha (*date_buf))
        *date_buf = toupper (*date_buf);

    if (*date_buf == '~')
        tilde = true;

    while (tilde)
    {
        sprintf (date_buf, "%s", read_a_line (fp_message));
        if (*date_buf != '~')
            tilde = false;
    }

    message->icdate = add_hash (date_buf);

    fclose (fp_message);

    return message;
}

void
unload_message (MESSAGE_DATA * message)
{
    if (message->poster)
        mem_free (message->poster);

    if (message->date)
        mem_free (message->date);

    if (message->subject)
        mem_free (message->subject);

    if (message->info)
        mem_free (message->info);

    if (message->message)
        mem_free (message->message);

    if (message->icdate)
        mem_free (message->icdate);

    if (message->target)
        mem_free (message->target);

    mem_free (message); // MESSAGE_DATA*
}

void
do_notes (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *who = NULL;
    bool messages = false;
    char name[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, name);

    if (!*name)
    {
        send_to_char
        ("Which PC or virtual board did you wish to get a listing for?\n",
         ch);
        return;
    }

    *name = toupper (*name);

    if (!(who = load_pc (name)))
    {
        messages = get_mysql_board_listing (ch, 1, name);
        if (!messages)
        {
            send_to_char ("No such PC or vboard.\n", ch);
            return;
        }
        else
            return;
    }

    unload_pc (who);

    messages = get_mysql_board_listing (ch, 2, name);

    if (!messages)
        send_to_char ("That player does not have any notes.\n", ch);
}

void
do_journal (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *who = NULL;
    char name[MAX_STRING_LENGTH] = { '\0' };
    int messages = 0;

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Journals are unavailable to guests.\n", ch);
        return;
    }

    if (!IS_MORTAL (ch))
    {
        argument = one_argument (argument, name);
        if (!*name)
            who = ch;
        else
        {
            *name = toupper (*name);
            if (!(who = load_pc (name)))
            {
                send_to_char ("No such PC, I'm afraid.\n", ch);
                return;
            }
        }
    }
    else
        who = ch;

    messages = get_mysql_board_listing (ch, 3, who->tname);
    if (!messages)
        send_to_char ("No journal entries found.\n", ch);

    if (who && who != ch)
        unload_pc (who);
}

void
do_notify (CHAR_DATA * ch, char *argument, int cmd)
{
    MYSQL_RES *result = NULL;
    MYSQL_ROW row = NULL;
    CHAR_DATA *tch = NULL;
    AFFECTED_TYPE *af = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    bool notify = false;
    CLAN_DATA *clan;

    if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC))
    {
        send_to_char ("This command has been disabled in OOC zones.\n", ch);
        return;
    }

    if (IS_SET (ch->flags, FLAG_GUEST))
    {
        send_to_char ("Guests cannot use the notify command.\n", ch);
        return;
    }

    if (!GET_TRUST (ch))
        send_to_char
        ("#6Note:  This command is here solely to facilitate RP, and is therefore strictly OOC.#0\n",
         ch);

    while (*argument == ' ')
        argument++;

    strcpy (buf, argument);

    while (*buf && *(buf + strlen (buf) - 1) == ' ')
        buf[strlen (buf) - 1] = '\0';

    if (!*buf)
    {

        if ((af = get_affect (ch, MAGIC_CLAN_NOTIFY)))
        {

            if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
            {
                send_to_char ("\nThey're no longer online.\n", ch);
                affect_remove (ch, af);
                return;
            }

            send_to_char ("\nClan member has been notified.\n", ch);

            sprintf (buf, "#3[%s is online.]#0\n", char_short (ch));
            buf[3] = toupper (buf[3]);

            send_to_char (buf, (CHAR_DATA *) af->a.spell.t);

            if (!IS_SET (((CHAR_DATA *) af->a.spell.t)->plr_flags, MUTE_BEEPS))
                send_to_char ("\a", (CHAR_DATA *) af->a.spell.t);

            affect_remove (ch, af);

            return;
        }

        if (!(af = get_affect (ch, MAGIC_NOTIFY)))
        {
            send_to_char ("\nNobody has notified you that they were online.\n",
                          ch);
            return;
        }

        if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
        {
            send_to_char ("\nThey're no longer online.\n", ch);
            affect_remove (ch, af);
            return;
        }

        send_to_char ("#3[Notifyee is online.]#0\n",
                      (CHAR_DATA *) af->a.spell.t);
        send_to_char ("Requesting party notified.\n", ch);

        affect_remove (ch, af);

        return;
    }

    if (is_clan_member_player (ch, buf))
    {

        send_to_char
        ("\nAll clan members currently online have been notified.\n", ch);
        sprintf (buf2, "#3[%s (%s) is online.  Use NOTIFY to reply in kind.]#0",
                 char_short (ch),
                 (((clan =
                        get_clandef_long (buf)) != NULL) ? clan->literal : buf));
        buf2[3] = toupper (buf2[3]);

        for (tch = character_list; tch; tch = tch->next)
        {

            if (ch == tch || tch->deleted || IS_NPC (tch))
                continue;

            if (!is_clan_member_player (tch, buf))
                continue;

            act (buf2, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);

            magic_add_affect (tch, MAGIC_CLAN_NOTIFY, 2, 0, 0, 0, 0);

            get_affect (tch, MAGIC_CLAN_NOTIFY)->a.spell.t = (long int) ch;
        }

        return;
    }

    if (!(tch = get_pc (buf)))
    {
        if (!(tch = load_pc (buf)))
        {
            mysql_safe_query
            ("SELECT name FROM %s.pfiles WHERE keywords LIKE \'%%%s%%\'",
             (engine.get_config ("player_db")).c_str (), buf);
            result = mysql_store_result (database);
            if (!result || mysql_num_rows (result) <= 0)
            {
                mysql_safe_query
                ("SELECT name FROM %s.pfiles WHERE sdesc LIKE \'%%%s%%\'",
                 (engine.get_config ("player_db")).c_str (), buf);
                result = mysql_store_result (database);
            }
            if (result && mysql_num_rows (result) > 0
                    && mysql_num_rows (result) <= 50)
            {
                while ((row = mysql_fetch_row (result)))
                {
                    if ((tch = get_pc (row[0])))
                    {
                        sprintf (buf,
                                 "\a#3[%s is online.  Use NOTIFY to reply in kind.]#0\n",
                                 char_short (ch));
                        buf[4] = toupper (buf[4]);
                        send_to_char (buf, tch);
                        if (!IS_SET (tch->plr_flags, MUTE_BEEPS))
                            send_to_char ("\a", tch);
                        if ((af = get_affect (tch, MAGIC_NOTIFY)))
                        {
                            af->a.spell.t = (long int) ch;
                            af->a.spell.duration = 2;
                            if (result)
                                mysql_free_result (result);
                            return;
                        }
                        magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);
                        get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (long int) ch;
                        notify = true;
                    }
                }
                if (result)
                    mysql_free_result (result);
                if (!IS_MORTAL (ch))
                {
                    if (notify)
                        send_to_char ("\nParty was found online and notified.\n",
                                      ch);
                    else
                        send_to_char
                        ("\nNo PC online has such a description or alias.  Did you spell it fully and correctly?\n",
                         ch);
                }
                else
                {
                    send_to_char
                    ("\nIf this individual is online they have been notified.\n",
                     ch);
                }
                return;
            }
            if (result && mysql_num_rows (result) > 50)
            {
                send_to_char
                ("\nToo many matches were found. Please try elaborating on the description.\n",
                 ch);
                mysql_free_result (result);
                return;
            }

            if (!IS_MORTAL (ch))
                send_to_char
                ("\nNo PC has such a name.  Did you spell it fully and correctly?\n",
                 ch);
            else
                send_to_char
                ("\nIf this individual is online they have been notified.\n",
                 ch);
            return;
        }

        unload_pc (tch);
        send_to_char
        ("\nIf this individual is online they have been notified.\n", ch);
        return;
    }

    send_to_char ("\nIf this individual is online they have been notified.\n",
                  ch);

    sprintf (buf, "#3[%s is online.  Use NOTIFY to reply in kind.]#0\n",
             char_short (ch));
    buf[3] = toupper (buf[3]);

    send_to_char (buf, tch);

    if (!IS_SET (tch->plr_flags, MUTE_BEEPS))
        send_to_char ("\a", tch);

    if ((af = get_affect (tch, MAGIC_NOTIFY)))
    {
        af->a.spell.t = (long int) ch;
        af->a.spell.duration = 2;
        return;
    }

    magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);

    get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (long int) ch;
}

void post_writing( DESCRIPTOR_DATA * d )
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *quill = NULL;
    CHAR_DATA *ch = NULL;
    OBJ_DATA *ink = NULL;
    WRITING_DATA *writing = NULL;
    unsigned int i = 0;
    time_t current_time = 0;
    char date[32] = "";
    char message[MAX_STRING_LENGTH] = "";
    float mod = 0;

    ch = d->character;

    if (!(obj = ch->pc->writing_on))
    {
        send_to_char ("That object is no longer there!\n", ch);
        return;
    }

    current_time = time (0);
    ctime_r (&current_time, date);
    if (strlen (date) > 1)
        date[strlen (date) - 1] = '\0';

    sprintf (message, "%s", d->pending_message->message);

    if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_INK)
    {
        ink = ch->right_hand;
        if (ink->o.od.value[1] <= 0)
        {
            send_to_char
            ("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n",
             ch);
            extract_obj (ink);
        }
    }

    if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_INK)
    {
        ink = ch->left_hand;
        if (ink->o.od.value[1] <= 0)
        {
            send_to_char
            ("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n",
             ch);
            extract_obj (ink);
        }
    }

    if ((quill = ch->right_hand))
    {
        if (GET_ITEM_TYPE (quill) == ITEM_WRITING_INST)
        {
            if (quill->ink_color)
                quill->ink_color = NULL;
        }
        else
            quill = NULL;
    }

    if (!quill && ch->left_hand)
    {
        quill = ch->left_hand;
        if (GET_ITEM_TYPE (quill) == ITEM_WRITING_INST)
        {
            if (quill->ink_color)
                quill->ink_color = NULL;
        }
        else
            quill = NULL;
    }

    mod =
        (skill_level(ch, ch->writes, 0) * 0.50) +
        (skill_level(ch, ch->speaks, 0) * 0.30) +
        (skill_level(ch, SKILL_EDUCATION, 0) * 0.20);


    mod = (float) MIN (95, (int) mod);

    if ( !IS_BOOK( obj ) && GET_NEXT_WRITE_OVAL( obj ) != -1 )
    {
        CREATE (obj->writing, WRITING_DATA, 1);

        obj->writing->ink       = add_hash( ch->delay_who );
        obj->writing->author    = add_hash( ch->tname );
        obj->writing->date      = add_hash( date );
        obj->writing->language  = ch->speaks;
        obj->writing->script    = ch->writes;
        obj->writing->message   = tilde_eliminator (message);
        obj->writing->skill     = (int) mod;
    }

    if ( IS_BOOK( obj ) )
    {
        for (i = 2, writing = obj->writing; i <= obj->open; i++)
        {
            if (obj->writing->next_page)
            {
                writing = writing->next_page;
            }
        }

        writing->ink        = add_hash( ch->delay_who );
        writing->author     = add_hash( ch->tname );
        writing->date       = add_hash( date );
        writing->language   = ch->speaks;
        writing->script     = ch->writes;
        writing->message    = tilde_eliminator( message );
        writing->skill      = (int) mod;
    }

    ch->pc->writing_on = NULL;
    ch->delay_who = NULL;
    unload_message (d->pending_message);

    skill_use (ch, ch->writes, 0);
    if (!number (0, 1))
        skill_use (ch, ch->speaks, 0);
    if (!number (0, 2))
        skill_use (ch, SKILL_EDUCATION, 0);

    save_writing (obj);
}

void do_flip (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };

    argument = one_argument (argument, buf);

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
    {
        send_to_char ("What did you want to flip?\n", ch);
        return;
    }

    if ( !IS_BOOK( obj ) )
    {
        send_to_char ("That isn't any kind of book, unfortunately.\n", ch);
        return;
    }

    if ( IS_ELECTRIC( obj ) )
    {
        if ( obj->o.elecs.status == 0 )
        {
            send_to_char( "This device needs to be turned on first.\n", ch );
            return;
        }

        if ( !obj->open )
            obj->open = 1;
    }
    else if ( !obj->open )
    {
        send_to_char ("You'll need to open it, first.\n", ch);
        return;
    }

    if ( !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
    {
        if ( !IS_ELECTRIC( obj ) )
            send_to_char( "The memory unit seems to have corrupted.", ch );
        else
            send_to_char( "It doesn't have any pages left to flip.\n", ch );
        return;
    }

    if ( * argument && !isdigit( * argument ) )
    {
        if ( !IS_ELECTRIC( obj ) )
            send_to_char ( "Which file block would you want to open up?\n", ch);
        else
            send_to_char ( "Which page would you like to flip to?\n", ch);
        return;
    }


    if (!*argument)
        sprintf (argument, "%d", obj->open + 1);

    if ( ( unsigned int ) strtol ( argument, NULL, 10 ) == obj->open )
    {
        if ( IS_ELECTRIC( obj ) )
            send_to_char ("It's already open to that file block.\n", ch);
        else
            send_to_char ("It's already open to that page.\n", ch);

        return;
    }

    if ( atoi( argument ) > obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
    {
        if ( IS_ELECTRIC( obj ) )
            sprintf (buf, "There are only %d file blocks in this device.\n", obj->o.od.value[ GET_PAGE_OVAL( obj ) ] );
        else
            sprintf (buf, "There are only %d pages in this book.\n", obj->o.od.value[ GET_PAGE_OVAL( obj ) ] );

        send_to_char (buf, ch);
        return;
    }

    if ( !*argument )
    {
        if ( IS_ELECTRIC( obj ) )
            send_to_char ("Which file block did you wish to open up?\n", ch );
        else
            send_to_char ("Which page did you wish to flip to?\n", ch );

        return;
    }

    if ( IS_ELECTRIC( obj ) )
        if ( atoi( argument ) == 0 )
            sprintf( buf, "You navigate through #2%s#0's interface until you locate the file block index.",
                obj->short_description
            );
        else
            sprintf (buf, "You navigate through #2%s#0's interface until you locate file block %d.",
                obj->short_description,
                atoi( argument )
            );
    else if ( ( unsigned int ) strtol( argument, NULL, 10 ) > obj->open + 1)
    {
        sprintf (buf, "You leaf carefully through #2%s#0 until you arrive at page %d.",
            obj->short_description,
            atoi( argument )
        );
    }
    else
    {
        sprintf (buf, "You turn #2%s's#0 page.", obj->short_description);

    }
    act( buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT );

    if ( IS_ELECTRIC( obj ) )
        sprintf (buf, "%s#0 navigates through #2%s#0's interface momentarily.",
            char_short( ch ),
            obj->short_description
        );
    else if ( ( unsigned int ) strtol (argument, NULL, 10) > obj->open + 1)
        sprintf( buf, "%s#0 leafs carefully through #2%s#0 until %s arrives at the desired page.",
            char_short( ch ),
            obj->short_description,
            HSSH( ch )
        );
    else
        sprintf( buf, "%s#0 turns #2%s's#0 page.",
            char_short (ch),
            obj->short_description
        );

    sprintf ( buffer, "#5%s", CAP( buf ) );
    act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

    obj->open = atoi( argument );
}

void
do_tear (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *parchment = NULL;
    WRITING_DATA *writing = NULL;
    WRITING_DATA *page = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    unsigned int i = 0;

    argument = one_argument (argument, buf);

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
    {
        send_to_char ("What did you wish to tear?\n", ch);
        return;
    }

    if (ch->right_hand && ch->left_hand)
    {
        send_to_char ("You must have one hand free.\n", ch);
        return;
    }

    if ( !IS_BOOK( obj ) || !IS_TEARABLE( obj ) )
    {
        send_to_char ("I'm afraid that can't be torn.\n", ch);
        return;
    }

    if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
    {
        if ( !obj->writing_loaded )
            load_writing( obj );

        if ( !obj->open )
        {
            send_to_char ("Open it to the page you wish to tear out, first.\n", ch);
            return;
        }

        if ( !obj->writing || !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
        {
            send_to_char ("It doesn't have any pages left!\n", ch);
            return;
        }

        if ( !( parchment = load_object ( 61 ) ) )
        {
            send_to_char
            ("The parchment prototype (VNUM 61) appears to be missing. Please inform staff.\n",
             ch);
            return;
        }

        for ( i = 1, writing = obj->writing; i <= obj->open; i++ )
        {
            if ( i == 1 && i == obj->open )
            {
                sprintf (buf, "You carefully tear page %d from #2%s#0.", i, obj->short_description);
                page = writing;

                // Moving all pages down a number.
                if ( !obj->writing->next_page )
                {
                    obj->writing = NULL;
                }
                else obj->writing = writing->next_page;

                break;
            }
            else if ( i + 1 == obj->open && obj->open > 1 )
            {
                sprintf ( buf, "You carefully tear page %d from #2%s#0.", i + 1, obj->short_description);
                page = writing->next_page;

                if (!writing->next_page->next_page)
                {
                    writing->next_page = NULL;
                    obj->open--;
                }
                else writing->next_page = writing->next_page->next_page;

                break;
            }

            writing = writing->next_page;
        }

        act( buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT );
        sprintf (buf, "%s#0 carefully tears a page from #2%s#0.", char_short (ch), obj->short_description);
        sprintf (buffer, "#5%s", CAP( buf ) );

        act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

        if (*page->message && strcasecmp( page->message, "blank" ) != STR_MATCH)
        {
            CREATE( parchment->writing, WRITING_DATA, 1 );

            parchment->writing->ink         = add_hash( page->ink );
            parchment->writing->author      = add_hash( page->author );
            parchment->writing->date        = add_hash( page->date );
            parchment->writing->language    = page->language;
            parchment->writing->script      = page->script;
            parchment->writing->message     = add_hash (page->message);
            parchment->writing->skill       = page->skill;;
        }

        obj->o.od.value[ GET_PAGE_OVAL( obj ) ]--;
        obj_to_char (parchment, ch);
        save_writing (obj);
        save_writing (parchment);
        return;
    }

    sprintf (buf, "You rend #2%s#0 into small pieces, which you then meticulously discard.",
             obj->short_description
    );
    act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

    sprintf (buf, "%s rends #2%s#0 into small pieces, which %s then meticulously discards.",
             char_short( ch ),
             obj->short_description,
             HSSH( ch )
    );
    sprintf( buffer, "#5%s", CAP( buf ) );
    act( buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT );

    extract_obj (obj);
}

void
do_wipe (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *parchment = NULL;
    WRITING_DATA *writing = NULL;
    WRITING_DATA *page = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    unsigned int i = 0;

    argument = one_argument (argument, buf);

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
    {
        send_to_char ("What did you wish to wipe?\n", ch);
        return;
    }

  if ( GET_ITEM_TYPE (obj) == ITEM_BOOK)
	{
	send_to_char("You need to use the tear command.\n", ch);
	return;
	}
  //  if ( !IS_E_BOOK( obj ) || !IS_TEARABLE( obj ) )
 //   {
  //      send_to_char ("I'm afraid that can't be torn.\n", ch);
  //      return;
  //  }
 
    if ( GET_ITEM_TYPE (obj) == ITEM_E_BOOK )
    {
        if ( !obj->writing_loaded )
            load_writing( obj );

        if ( !obj->open )
        {
            send_to_char ("Open it to the file block you wish to wipe out, first.\n", ch);
            return;
        }

        if ( !obj->writing || !obj->o.od.value[ GET_PAGE_OVAL( obj ) ] )
        {
            send_to_char ("It doesn't have any file blocks left!\n", ch);
            return;
        }

 //       if ( !( parchment = load_object ( 61 ) ) )
  //      {
  //          send_to_char
   //         ("The parchment prototype (VNUM 61) appears to be missing. Please inform staff.\n",
    //         ch);
    //        return;
    //    }

        for ( i = 1, writing = obj->writing; i <= obj->open; i++ )
        {
            if ( i == 1 && i == obj->open )
            {
                sprintf (buf, "You carefully wipe file block %d from #2%s#0.", i, obj->short_description);
                page = writing;

                // Moving all pages down a number.
                if ( !obj->writing->next_page )
                {
                    obj->writing = NULL;
                }
                else obj->writing = writing->next_page;

                break;
            }
            else if ( i + 1 == obj->open && obj->open > 1 )
            {
                sprintf ( buf, "You carefully wipe file block %d from #2%s#0.", i + 1, obj->short_description);
                page = writing->next_page;

                if (!writing->next_page->next_page)
                {
                    writing->next_page = NULL;
                    obj->open--;
                }
                else writing->next_page = writing->next_page->next_page;

                break;
            }

            writing = writing->next_page;
        }

        act( buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT );
        sprintf (buf, "%s#0 carefully wipes a file block from #2%s#0.", char_short (ch), obj->short_description);
        sprintf (buffer, "#5%s", CAP( buf ) );

        act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

   //     if (*page->message && strcasecmp( page->message, "blank" ) != STR_MATCH)
  //      {
   //         CREATE( parchment->writing, WRITING_DATA, 1 );
//
   //         parchment->writing->ink         = add_hash( page->ink );
  //          parchment->writing->author      = add_hash( page->author );
  //          parchment->writing->date        = add_hash( page->date );
  //          parchment->writing->language    = page->language;
  //          parchment->writing->script      = page->script;
  //          parchment->writing->message     = add_hash (page->message);
   //         parchment->writing->skill       = page->skill;;
   //     }

    //    obj->o.od.value[ GET_PAGE_OVAL( obj ) ]--;
    //    obj_to_char (parchment, ch);
    //    save_writing (obj);
    //    save_writing (parchment);
    //    return;
  //  }
return;
}
    sprintf (buf, "You rend #2%s#0 into small pieces, which you then meticulously discard.",
             obj->short_description
    );
    act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

    sprintf (buf, "%s rends #2%s#0 into small pieces, which %s then meticulously discards.",
             char_short( ch ),
             obj->short_description,
             HSSH( ch )
    );
    sprintf( buffer, "#5%s", CAP( buf ) );
    act( buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT );

    extract_obj (obj);
}

void
do_dip (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *quill = NULL, *ink = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!
            (ch->left_hand
             && ch->left_hand->obj_flags.type_flag == ITEM_WRITING_INST)
            && !(ch->right_hand
                 && ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST))
    {
        send_to_char ("You need to be holding a writing implement.\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        if (ch->right_hand
                && GET_ITEM_TYPE (ch->right_hand) == ITEM_WRITING_INST)
            quill = ch->right_hand;
        else
            quill = ch->left_hand;

        if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_INK)
            ink = ch->right_hand;
        else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_INK)
            ink = ch->left_hand;
    }
    else
    {
        quill = get_obj_in_list_vis (ch, buf, ch->right_hand);
        if (!quill)
            quill = get_obj_in_list_vis (ch, buf, ch->left_hand);
        if (quill && GET_ITEM_TYPE (quill) != ITEM_WRITING_INST)
        {
            send_to_char
            ("You must specify first the writing implement, and then the ink source.\n",
             ch);
            return;
        }
        argument = one_argument (argument, buf);
        if (!*buf)
        {
            send_to_char
            ("You must specify both a writing implement and a ink source.\n",
             ch);
            return;
        }
        ink = get_obj_in_list_vis (ch, buf, ch->right_hand);
        if (!ink)
            ink = get_obj_in_list_vis (ch, buf, ch->left_hand);
        if (!ink)
            ink = get_obj_in_list_vis (ch, buf, ch->room->contents);
        if (ink && GET_ITEM_TYPE (ink) != ITEM_INK)
        {
            send_to_char
            ("You must specify first the writing implement, and then the ink source.\n",
             ch);
            return;
        }
    }

    if (!ink)
    {
        send_to_char
        ("You need to have a ink source in one hand, or in the room.\n", ch);
        return;
    }

    if (!quill)
    {
        send_to_char ("You need to be holding a writing implement.\n", ch);
        return;
    }

    if (ink->o.od.value[0] <= 0)
    {
        send_to_char ("Your ink source seems to be empty.\n", ch);
        return;
    }

    sprintf (buf,
             "You insert #2%s#0 carefully into #2%s#0, giving it a moment to soak.",
             quill->short_description, ink->short_description);
    quill->ink_color = ink->ink_color;
    act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    ink->o.od.value[0]--;
}

void do_scribe (CHAR_DATA * ch, char *argument, int cmd)
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *quill = NULL;
    WRITING_DATA *writing = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buffer[MAX_STRING_LENGTH] = { '\0' };
    unsigned int i = 0;
    int script = 0;

    if (IS_NPC (ch))
    {
        send_to_char ("This is only available to PCs.\n", ch);
        return;
    }

    if (ch->skills[SKILL_EDUCATION] < 10)
        send_to_char("OOC: Remember to RP the education level of your PC at all times. With"
            " your PC's low education, they can only realistically draw or write the occasional"
            " word, such as their name, or 'Rust.' If you fail to RP appropriately, punishments"
            " may apply.", ch);
    /*{
        send_to_char ("You aren't literate enough to do that.\n", ch);
        return;
    }

    
    for (i = SKILL_SCRIPT_SARATI; i <= SKILL_SCRIPT_VALARIN_SCRIPT; i++)
      if (ch->skills[i])
        script = 1;
    */

    if (ch->skills[SKILL_COMMON])
        script = 1;

    if (!script)
    {
        send_to_char ("You have no knowledge of any written scripts.\n", ch);
        return;
    }

    if (!ch->writes)
    {
        send_to_char ("In which script would you like to write? (See the SCRIBE command.)\n", ch);
        return;
    }

    argument = one_argument (argument, buf);

    if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
            !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand))
        /*&& !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents))*/)
    {
        send_to_char ("You can't find that!\n", ch);
        return;
    }

    if ( !IS_ELECTRIC( obj ) )
    {
        if ( ch->right_hand && GET_ITEM_TYPE( ch->right_hand ) == ITEM_WRITING_INST )
            quill = ch->right_hand;
        else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_WRITING_INST )
            quill = ch->left_hand;
        else
        {
            send_to_char ("You need to be holding a writing implement.\n", ch);
            return;
        }

        if (!ch->left_hand && !ch->right_hand)
        {
            send_to_char ("You need to be holding a writing implement.\n", ch);
            return;
        }

        if (!quill->ink_color)
        {
            send_to_char ("The writing instrument must be inked first.\n", ch);
            return;
        }

        ch->delay_who = quill->ink_color;
    } else if ( IS_ELECTRIC( obj ) )
    {
        ch->delay_who = "digital";
    } else
    {
        act ( "That cannot be written on.\n", false, ch, obj, 0, TO_CHAR );
        return;
    }

    // Load the writing data if its not loaded yet
    if ( !obj->writing_loaded )
        load_writing( obj );

    // If we are using parchment, skip it if it contains written data already
    if ( GET_PAGE_OVAL( obj ) == -1 && obj->writing )
    {
        if ( IS_ELECTRIC( obj ) )
            send_to_char( "That cannot be modified further.", ch );
        else
            send_to_char( "That has already been written on.\n", ch );

        return;
    }
    else if ( IS_BOOK( obj ) )
    {
        int page_oval = GET_PAGE_OVAL( obj );

        if ( IS_ELECTRIC( obj ) )
        {
            if ( obj->o.elecs.status == 0 )
            {
                send_to_char( "You'll need to power that on first.\n", ch );
                return;
            }

            if ( !obj->writing || !obj->o.od.value[ page_oval ] )
            {
                send_to_char( "The device's memory unit appears to be corrupt.\n", ch );
                return;
            }

            if ( !obj->open )
            {
                send_to_char( "You must flip to a file block first.", ch );
                return;
            }
        } else {
            if ( !obj->open )
            {
                send_to_char( "You need to open it, first.\n", ch );
                return;
            }

            if (!obj->writing || !obj->o.od.value[ page_oval ])
            {
                send_to_char ("It doesn't have any pages left!\n", ch);
                return;
            }
        }

        // Fetching the next free page
        for (i = 2, writing = obj->writing; i <= obj->open; i++)
        {
            if (obj->writing->next_page)
            {
                writing = writing->next_page;
            }
        }

        if (strcasecmp (writing->message, "blank") != STR_MATCH )
        {
            send_to_char ("That has already been written or typed on.\n", ch);
            return;
        }
    }

    if ( GET_ITEM_TYPE( obj ) == ITEM_E_BOOK )
        sprintf ( buffer, "#5%s#0 begins typing into #2%s#0.", char_short (ch), obj->short_description );
    else
        sprintf ( buffer, "#5%s#0 begins writing on #2%s#0.", char_short (ch), obj->short_description );

    buffer[3] = toupper (buffer[3]);
    act (buffer, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    CREATE( ch->descr()->pending_message, MESSAGE_DATA, 1 );

    ch->descr()->pending_message->message = NULL;

    send_to_char
    ("Scribe your message; terminate with an '@'. Please keep its length plausible\nfor the size of the writing object, since we have opted against coded limits.\n",
     ch);
    sprintf (buf,
             "1-------10--------20--------30--------40--------50--------60--------70\n");
    send_to_char (buf, ch);

    make_quiet (ch);

    ch->descr()->str = &ch->descr()->pending_message->message;
    ch->descr()->max_str = MAX_STRING_LENGTH;

    ch->descr()->proc = post_writing;
    ch->pc->writing_on = obj;
}

void
do_ticket (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[AVG_STRING_LENGTH];
    char buf2[12];
    char f_tick[12];
    char l_tick[12];
    int first_tick;
    int last_tick;
    int tick_num = 1;

    argument = one_argument(argument, buf);

    if (!*buf)
    {
        send_to_char ("ticket read <number>\n", ch);
        send_to_char ("ticket browse <first number> <last number>\n\n", ch);
        send_to_char ("Finally you may delete a ticket\n\n", ch);
        send_to_char ("ticket delete <ticket number>\n", ch);
        return;
    }

    if (!strcmp(buf, "browse"))
    {
        argument = one_argument (argument, f_tick);
        if (!*f_tick)
        {
            send_to_char ("Starting with which ticket?\n", ch);
            return;
        }

        if (!isdigit (*f_tick))
        {
            send_to_char ("You must specify the first ticket number\n", ch);
            return;
        }


        argument = one_argument (argument, l_tick);
        if (!*l_tick)
        {
            send_to_char ("ending at which ticket?\n", ch);
            return;
        }

        if (!isdigit (*l_tick))
        {
            send_to_char ("You must specify the last ticket number.\n", ch);
            return;
        }

        //No more than 100 tickets to be checked at a time, instead of the whole 10 million (9,999,999). May need to adjust if there really are more tickets
        first_tick = atoi(f_tick);
        last_tick = MIN(500, atoi(l_tick));

        if (first_tick >= last_tick)
        {
            send_to_char ("The last number must be larger than the first number.\n", ch);
            return;
        }

        //browse_ticket(ch, first_tick, last_tick);
        for (tick_num = first_tick; tick_num <= last_tick; tick_num ++)
        {
            read_ticket(ch, tick_num);
        };

    } //if browse

    if (!strcmp(buf, "read"))
    {
        argument = one_argument (argument, buf2);
        if (!*buf2)
        {
            send_to_char("Which ticket did you wish to read?\n", ch);
            return;
        }

        if (isdigit (*buf2))
        {
            tick_num = atoi (buf2);
        }
        else
        {
            send_to_char("You must use the ticket number.\n", ch);
            return;
        }

        read_ticket(ch, tick_num);
    }//read


    if (!strcmp(buf, "delete"))
    {
        argument = one_argument (argument, buf2);
        if (!*buf2)
        {
            send_to_char("Which ticket did you wish to delete?\n", ch);
            return;
        }

        if (isdigit (*buf2))
        {
            tick_num = atoi (buf2);
        }
        else
        {
            send_to_char("You must use the ticket number.\n", ch);
            return;
        }

        delete_ticket(ch, tick_num);
    }//delete

    return;
}

void
read_ticket (CHAR_DATA * ch, int tick_num)
{

    int nVirtual;
    CHAR_DATA *mob;
    FILE *fp;
    char buf[AVG_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH] = {'\0'};
    char hookup[AVG_STRING_LENGTH];
    char name[AVG_STRING_LENGTH];


    sprintf (name, TICKET_DIR "/%07d", tick_num);

    if (!(fp = fopen (name, "r")))
    {
        return;
    }

    sprintf(buf2, "\nTicket number: %d\n", tick_num);

    while (fgets (buf, 256, fp))
    {
        //skip lines with blank space at start or blank lines
        if (*buf == ' ' || *buf == '\n')
            fgets (buf, 255, fp);

//Look for the MOB
        if (sscanf (buf, "%d %s", &nVirtual, hookup) != 2)
        {
            fclose (fp);
            system_log ("The ticket system is broken, read_ticket() - mob", true);
            sprintf(buf2 + strlen(buf2), "Bad file format-mob\n");
            send_to_char(buf2, ch);
            return;
        }

        mob = load_a_saved_mobile (nVirtual, fp, true);

        if (mob)
        {
            sprintf(buf2 + strlen(buf2), "Vnum: %d \nNamed: %s \nClans: %s \nOwner: %s \nStabled at: %s (%d) \n", mob->mob->vnum, mob->name, mob->clans, mob->mob->owner, vnum_to_room(mob->in_room)->name, mob->in_room);

            save_mobile (mob, fp, "HITCH", 1);	/* Extracts the mobile */
        }

        fclose (fp);
        send_to_char(buf2, ch);
        return;
    }

    fclose (fp);

    send_to_char(buf2, ch);
    return;
}


void
delete_ticket (CHAR_DATA * ch, int tick_num)
{

    FILE *fp;
    char buf[AVG_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH] = {'\0'};
    char name[AVG_STRING_LENGTH];


    sprintf (name, TICKET_DIR "/%07d", tick_num);

    if (!(fp = fopen (name, "r")))
    {
        return;
    }

    sprintf(buf2, "\nTicket number %d deleted\n", tick_num);

    sprintf(buf, "rm %s", name);
    system (buf);

    fclose (fp);

    send_to_char(buf2, ch);
    return;
}

/***
do_evaluate

This command provides detailed information about a held object
including weight, cost and any skill affects, as well as other
object specific information such as time left for light objects
and liquid left in drinks containers.

- Valarauka

***/
void
do_evaluate (CHAR_DATA *ch, char *argument, int cmd)
{


    char		arg1 [MAX_STRING_LENGTH] = { '\0' };
    OBJ_DATA	*obj = NULL;
    char		buffer [MAX_STRING_LENGTH] = { '\0' };

    /*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

    if ( GET_POS (ch) < POSITION_SLEEPING )
    {
        send_to_char ("You are unconscious!\n", ch);
        return;
    }

    if ( GET_POS (ch) == POSITION_SLEEPING )
    {
        send_to_char ("You are asleep.\n", ch);
        return;
    }

    if ( is_blind (ch) )
    {
        send_to_char ("You are blind!\n", ch);
        return;
    }

    argument = one_argument (argument, arg1);

    if ( !*arg1 )
    {
        send_to_char ("Evaluate what?\n", ch);
        return;
    }

    if ( !(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
            !(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
            !(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
            !(obj = get_obj_in_dark (ch, arg1, ch->room->contents)))
    {

        send_to_char ("You don't have that.\n", ch);
        return;
    }

    /*** Describe the object ***/
    if (obj)
    {
        snprintf (buffer, MAX_STRING_LENGTH,  "\nIt is #2%s#0", obj->short_description);
        act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        *buffer = '\0';
    }
    else
    {
        send_to_char ("You cannot evaluate something unless you have it.\n", ch);
        return;
    }

    send_to_char ("\n", ch);

    show_evaluate_information(ch, obj);

    return;
}

void
show_evaluate_information (CHAR_DATA *ch, OBJ_DATA	*obj)
{
    int 		variance = 0;
    int 		guess_weight = 0;
    int			temp = 0;
    int			i = 0;
    int 		j = 0;
    char		buffer [MAX_STRING_LENGTH] = { '\0' };
//        char		buf2 [MAX_STRING_LENGTH] = { '\0' };
    char		*temp_arg = NULL;
    AFFECTED_TYPE *af;


    /*** ECON FLAGS ***/

    /*
    if (obj->econ_flags)
    {
      temp = 0;
      for (i = 0; i <= 6; i++)
      {
        if (IS_SET (obj->econ_flags, 1 << i))
        {
          strcat (buf2, econ_flags[i]);
          strcat (buf2, " ");
          temp = 1;
        }
      }
      if(temp == 1)
      {
        sprintf (buffer, "The item is possibly of the following origins: #6%s#0", buf2);
      }

      send_to_char (buffer, ch);
      send_to_char ("\n\n", ch);
    }
    */

    if (obj->obj_flags.type_flag == ITEM_FOOD && GET_MATERIAL_TYPE(obj) == 1 << 9)
    {

        if (ch->skills[SKILL_BIOLOGY] >= obj->o.od.value[4]
                || ch->skills[SKILL_FORAGE] >= obj->o.od.value[4])
        {
            switch (obj->o.od.value[3])
            {
            case 1:
                sprintf (buffer, "You determine that it is safe, edible food.\n");
                break;
            case 2:
                sprintf (buffer, "You determine that it will be safe to eat once properly cooked or prepared.\n");
                break;
            case 3:
                sprintf (buffer, "You determine that it is dangerous to eat, but can be prepared in other ways.\n");
                break;
            case 4:
                sprintf (buffer, "You determine that it is not edible but may have other uses.\n");
                break;
            default:
                sprintf (buffer, "You're unable to determine what use it is.\n");
                break;
            }
        }
        else if (ch->skills[SKILL_BIOLOGY] >= obj->o.od.value[4] / 2
                 || ch->skills[SKILL_FORAGE] >= obj->o.od.value[4] / 2)
        {
            switch (obj->o.od.value[3])
            {
            case 1:
            case 2:
            case 3:
                sprintf(buffer, "You determine that it is edible food.\n");
                break;
            case 4:
                sprintf(buffer, "You determine that it is not edible but may have other uses.\n");
                break;
            default:
                sprintf(buffer, "You're unable to determine what use it is.\n");
                break;
            }
        }

        send_to_char (buffer, ch);
    }

    *buffer = '\0';

    /*** WEAR LOCATIONS FOR WEARABLE ITEMS ***/

    if (obj->obj_flags.wear_flags)
    {
        temp = 0;
        for (i = 0; (*wear_bits[i] != '\n'); i++)
        {
            if ((IS_SET (obj->obj_flags.wear_flags, (1 << i)))
                    && (strcmp (wear_bits[i],"Take"))
                    && (strcmp (wear_bits[i],"Unused"))) //dont want to show Take and Unused
            {
                //if found something to write and not already written inital string, write it
                if (temp == 0)
                {
                    sprintf (buffer, "\nYou recognise that you could wear this item in the following locations:\n");
                    temp = 1;
                }

                sprintf (buffer + strlen (buffer), "  #6%s#0\n", wear_bits[i]);
            }
        }

        send_to_char (buffer, ch);
        send_to_char ("\n", ch);
    }

    /*** END WEAR LOCATIONS ***/

    /**** Guess at the Weight based on scan skill **/


    if (IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
    {

        // Just based on intelligence. Simple.

        /*** Determine how far estimate may vary ***/
        if (ch->intel > 17)
        {
            variance = number (95, 105);
        }
        else if (ch->intel >15)
        {
            variance = number (92, 108);
        }
        else if (ch->intel > 13)
        {
            variance = number (90, 110);
        }
        else if (ch->intel > 12)
        {
            variance = number (87, 113);
        }
        else
        {
            variance = number (85, 115);
        }

        /*** Calculate estimate ***/
        guess_weight = (int)((obj->obj_flags.weight + obj->contained_wt)/variance);

        if (guess_weight <= 1)
        {
            snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item weighs less than a pound.");
        }
        else   /** weighs more than a pound **/
        {
            snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item weighs about %d pounds.",guess_weight);
        }
    }
    else  /** no way to check weight - message for non-takeable objects **/
    {
        snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can't even begin to guess how much this weighs.");
    }

    act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    *buffer = '\0';
    send_to_char ("\n", ch);

    /*** end weight guess ***/

    /*** Guess at value based on barter skill ***/

    /*
    	Rather than give the exact cost to the player, a random factor
    	is introduced to replicate a rough estimate of the cost. The barter
    	skill is used to determine how far from the actual cost the value given
    	may deviate so that those with a high barter skill will be fairly accurate,
    	where as those with a low barter skill might be quite far from the actual cost.
    	The more expensive an item, the harder it will be to get a very accurate estimate.

    	In addition, a skill check must then be passed to allow the player to see the cost.
    */

    /*** Determine how far estimation may vary ***/
    if (ch->skills[SKILL_HAGGLE]>70)
    {
        variance = number (95, 105);
    }
    else if (ch->skills[SKILL_HAGGLE]>60)
    {
        variance = number (92, 108);
    }
    else if (ch->skills[SKILL_HAGGLE]>50)
    {
        variance = number (90, 110);
    }
    else if (ch->skills[SKILL_HAGGLE]>40)
    {
        variance = number (87, 113);
    }
    else
    {
        variance = number (85, 115);
    }

    /*** Calculate estimate ***/
    //guess_value = (int)(((obj->farthings + obj->silver *4)*100)/variance);

    //if (skill_use(ch, SKILL_HAGGLE, 0)) /*** Passed Skill Check ***/
    //{
    //    if (guess_value <= 1)
    //    {
    //        snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item is worth less than 1 credit.");
    //    }
    //    else
    //    {
    //        snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item is worth around %d credits.", guess_value);
    //    }
    //}
    //else /*** Failed Skill Check ***/
    //{
    //    snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can't even begin to guess the value of this item.");
    //}

    //act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    //*buffer = '\0';
    //send_to_char ("\n", ch);

    /*** end value guess ***/


    /***** Pslim lantern code ***/
    if ( GET_ITEM_TYPE (obj) == ITEM_LIGHT )
    {
        if ( obj->o.light.hours <= 0 )
        {
            snprintf (buffer, MAX_STRING_LENGTH,  "\nThe $o is empty.");
        }
        else
        {
            temp = obj->o.light.hours;
            snprintf (buffer, MAX_STRING_LENGTH,  "\nYou think the $o will last another %d hours.", temp);
        }
        act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        *buffer = '\0';
        send_to_char ("\n", ch);
    }
    /***** end Pslim lantern code *****/

    /**** SHOW CAPACITY OF CONTAINER ****/

    if ((GET_ITEM_TYPE (obj) == ITEM_CONTAINER)||
            ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON ))
    {

        sprintf (buffer, "\nYou estimate that it would hold around %d.%.2d lbs",
                 obj->o.container.capacity / 100,
                 obj->o.container.capacity % 100);
        act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        *buffer = '\0';
        send_to_char ("\n", ch);
    }

    /*** END CAPACITY CODE ***/


    /**** LOOK INSIDE A DRINK CONTATINER ***/

    if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON )
    {
        if (!obj->contains)
        {
            snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is empty.",
                      obj->short_description);
        }
        else
        {
            if ( obj->o.drinks.capacity )
            {
                temp = (obj->contains->count * 3) / obj->o.drinks.capacity;
            }
            else
            {
                temp = 1;
            }
			temp_arg = obj_short_desc(obj->contains);
            snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is %sfull of %s.",
                      obj->short_description, fullness [temp], temp_arg);

        }
        act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        *buffer = '\0';
        send_to_char ("\n", ch);

    }
    /*** END DRINK CONTAINER ***/


    /**** LOOK INSIDE A NON-DRINK CONTAINER ***/

    if ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
            GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
            GET_ITEM_TYPE (obj) == ITEM_SHEATH ||
            GET_ITEM_TYPE (obj) == ITEM_HOLSTER ||
            GET_ITEM_TYPE (obj) == ITEM_AMMO_BELT ||
            GET_ITEM_TYPE (obj) == ITEM_BANDOLIER ||

            //(GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
            //obj->o.weapon.use_skill == SKILL_SLING) ||

            GET_ITEM_TYPE (obj) == ITEM_KEYRING )
    {

        if ( IS_SET (obj->o.container.flags, CONT_CLOSED) )
        {
            send_to_char ("\nIt is closed.", ch);
            return;
        }

        send_to_char ("\nContents :\n", ch);

        list_obj_to_char (obj->contains, ch, 1, true);
    }
    /* If not a container, no need to show any message here */

    /*** END NON_DRINK CONTAINER ***/

    /*** SHOW TIME TO DECAY FOR FOOD ***/

    if (GET_ITEM_TYPE (obj) == ITEM_FOOD)
    {
        if (obj->morphTime)
        {
            int delta, days, hours, minutes;

            delta = obj->morphTime - time (0);

            days = delta / 86400;
            delta -= days * 86400;

            hours = delta / 3600;
            delta -= hours * 3600;

            minutes = delta / 60;

            //write appropriate message for length of time left till decay
            if (days > 1)
            {
                sprintf (buffer,
                         "\nYou notice that %s appears fresh with no sign of decay.", obj->short_description);
            }
            else if (hours > 12)
            {
                sprintf (buffer,
                         "\nYou notice that %s still appears fresh, though it is slowly beginning to lose its original unsullied appearance.",
                         obj->short_description);
            }
            else if (hours > 1)
            {
                sprintf (buffer,
                         "\nWhile still in good condition, %s no longer appears fresh.", obj->short_description);
            }
            else
            {
                sprintf (buffer,
                         "\nYou notice that %s has already begun to decay in places and it will not be too long before it is completely rotten.",
                         obj->short_description);
            }

            act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            *buffer = '\0';
            send_to_char ("\n", ch);
        }
    }

    /*** END FOOD DECAY TIME ***/

    /*** DETAIL ANY SKILLS AFFECTED BY THE OBJECT ***/

    for (af = obj->xaffected; af; af = af->next)
    {
        if (af->type == MAGIC_HIDDEN)
            continue;
        if (af->a.spell.location >= 10000)
        {
            j = af->a.spell.modifier;
            if (j < 0)
            {
                sprintf (buffer, "\nYou judge that this item would %shinder your %s skill.",
                         (j <= -15 ? "massively " : j <= -10 ? "greatly " : j <= -6 ? "significantly " : j <= -3 ? "" : "slightly "),
                         skills[af->a.spell.location - 10000]);
            }
            else if (j > 0)
            {
                sprintf (buffer, "\nYou judge that this item would %simprove your %s skill.",
                         (j >= 15 ? "massively " : j >= 10 ? "greatly " : j >= 6 ? "significantly " : j >= 3 ? "" : "slightly "),
                         skills[af->a.spell.location - 10000]);
            }

            act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            *buffer = '\0';
        }
    }

    send_to_char ("\n", ch);

    /*** END SKILL AFFECTS ***/

    return;
}



//"check" command: informative command that prints what a keyword refers to without executing an actual command
void do_check (CHAR_DATA *ch, char *argument, int cmd)
{
    OBJ_DATA * obj=NULL;
    OBJ_DATA * container=NULL;

    CHAR_DATA * tch=NULL;

    char arg1[MAX_STRING_LENGTH] = { '\0' };
    char arg2[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };

    if (!ch->room)
        ch->room = vnum_to_room (ch->in_room);

    argument=one_argument (argument, arg1);

    ////CHECK COMMAND WITH NO ARGUMENT/////////////
    if (!*arg1)
    {
        send_to_char("What keyword do you want to check?\nTry 'check usage' for help.\n", ch);
        return;
    }

    ////START 'CHECK USAGE'////////////////////////
    if (!strn_cmp (arg1, "usage", strlen (arg1)))
    {
        send_to_char ("Usage: \n check usage\n check (list) <keyword>\n check (list) <keyword> in <container>\n check mob (list) <keyword>\n check obj (list) <keyword>\n",ch);
        return;
    }
    ////END 'CHECK USAGE'//////////////////////////


    ////START 'CHECK MOB (LIST) <KEYWORD>//////////
    if (!strn_cmp (arg1, "mob", strlen (arg1)))
    {
        argument=one_argument (argument,arg1);

        //'check mob' with no argument
        if (!*arg1)
        {
            send_to_char("Check what mob? \n",ch);
            return;
        }

        //'check mob list'
        if (!strn_cmp (arg1, "list", strlen (arg1)))
        {
            argument=one_argument(argument, arg1);

            //'check mob list' with no other argument
            if (!*arg1)
            {
                send_to_char("What keyword do you want to check?\n", ch);
                return;
            }

            //'check mob list <keyword>', but there's no mob with that keyword.
            if (!(tch = (get_char_room_vis (ch, arg1))))
            {
                send_to_char ("No mobs with that keyword.\n", ch);
                return;
            }

            //'check mob list <keyword>' if there is at least one mob/char with that keyword.
            //Prints out a numbered list of all mobs with that keyword in the room.
            //For loop goes through every mob in the room and checks if arg1 matches its keywords
            //and is visible to the character ch.
            //Variable i is needed to put numbering on the list.

            int i=0;
            for (;tch;tch=tch->next_in_room)
            {
                if (isname (arg1, tch->name) && CAN_SEE (ch, tch))
                {
                    i++;
                    sprintf (buf, "%i.%s: #5%s#0 \n", i, arg1, char_short(tch));
                    send_to_char (buf, ch);
                }
            }
            return;
        }

        //'check mob <keyword>' - mob with <keyword> exists in the room
        if (( tch = (get_char_room_vis (ch, arg1))))
        {
            sprintf (buf, "Mobs: Typing '%s' will target #5%s#0.\n", arg1, char_short(tch));
            send_to_char (buf, ch);
        }
        //'check mob <keyword>' - mob with that keyword doesn't exist
        else
            send_to_char ("No mobs with that keyword here. \n", ch);
        return;
    }
    ////END 'CHECK MOB (LIST) <KEYWORD>'


    ////START 'CHECK OBJ (LIST) <KEYWORD>'
    if ((!strn_cmp (arg1, "obj", strlen (arg1))) ||( !strn_cmp (arg1, "object", strlen (arg1))))
    {
        argument=one_argument (argument,arg1);

        //'check obj' with argument
        if (!*arg1)
        {
            send_to_char("What object do you want to check? \n",ch);
            return;
        }

        //'check obj list'
        if (!strn_cmp (arg1, "list", strlen (arg1)))
        {
            argument=one_argument (argument, arg1);

            //'check obj list' with no argument
            if (!*arg1)
            {
                send_to_char("What keyword do you want to list?\n",ch);
                return;
            }

            //'check obj list <keyword>' if there is no object with that keyword in the room.
            if (!(obj=(get_obj_in_list_vis (ch, arg1, ch->room->contents))))
            {
                send_to_char ("No objects with that keyword here.\n",ch);
                return;
            }

            //'check obj list <keyword>' if there is at least one object with that keyword.
            //Prints out a numbered list of all objects with that keyword in the room.
            //For loop goes through every object in the room and checks if arg1 matches its keywords
            //and is visible to the character ch.
            //Variable i is needed to put numbering on the list.

            int i=0;
            for (obj=ch->room->contents; obj!=NULL; obj=obj->next_content)
            {
                if (isname (arg1, obj->name) && CAN_SEE_OBJ(ch,obj))
                {
                    i++;
                    sprintf (buf, "%i.%s: #2%s#0 \n", i, arg1, obj_short_desc(obj));
                    send_to_char (buf, ch);
                }
            }
            return;
        }

        //check obj <keyword> - object is in the room.

        if ((obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
        {
            sprintf(buf, "Objects: Typing '%s' will target #2%s#0.\n",arg1, obj_short_desc(obj));
            send_to_char (buf, ch);
        }

        //check obj <keyword> if it's not in the room.
        else
            send_to_char ("No objects with that keyword. \n", ch);
        return;
    }
    ////END 'CHECK (LIST) obj <keyword>'////////


    ////START 'CHECK (LIST) <keyword> (in <container)

    //'check list <keyword> (in <container>)'
    if (!strncmp (arg1, "list", strlen(arg1)))
    {
        argument=one_argument (argument, arg1); //arg1 should now be the keyword you're looking for.
        argument=one_argument (argument, arg2); //arg2 should now be 'in' or something that doesn't make sense, will be ignored in that case.

        //'check list' with no argument
        if (!*arg1)
        {
            send_to_char("What keyword do you want to check?\n", ch);
            return;
        }

        //'check list <keyword> in <container>'
        //Same as 'check obj list', just inside a container.
        if (*arg2 && (!strncmp (arg2, "in", strlen (arg2))))
        {
            argument=one_argument (argument, arg2); //arg2 should now be your container.

            //'check list keyword in' with no additional argument.
            if (!*arg2)
            {
                send_to_char ("Check inside what? \n", ch);
                return;
            }

            //check if container is there
            if (!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
                    !(container = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
                    !(container = get_obj_in_dark (ch, arg2, ch->equip)) &&
                    !(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
            {
                send_to_char ("You don't see that container here.\n", ch);
                return;
            }

            //check if the object is a container
            if (GET_ITEM_TYPE (container) == ITEM_CONTAINER ||
                    GET_ITEM_TYPE (container) == ITEM_QUIVER ||
                    GET_ITEM_TYPE (container) == ITEM_SHEATH ||
                    GET_ITEM_TYPE (container) == ITEM_AMMO_BELT ||
                    GET_ITEM_TYPE (container) == ITEM_BANDOLIER ||
                    GET_ITEM_TYPE (container) == ITEM_HOLSTER ||
                    GET_ITEM_TYPE (container) == ITEM_KEYRING)
            {
                //check if the container is closed
                if (IS_SET (container->o.container.flags, CONT_CLOSED))
                {
                    send_to_char ("It is closed. \n", ch);
                    return;
                }
                //check if there is anything in the container that has the keyword stored in arg1
                if (!(obj = get_obj_in_list_vis (ch, arg1, container->contains)))
                {
                    send_to_char ("You don't see that in the container. \n",ch);
                    return;
                }

                //loop is the same as for 'check obj list'
                int i=0;
                for (;obj ; obj=obj->next_content)
                {
                    if (isname (arg1, obj->name))
                    {
                        i++;
                        sprintf (buf, "%i.%s: #2%s#0\n", i, arg1, obj_short_desc(obj));
                        send_to_char (buf, ch);
                    }
                }
                return;
            }

            //arg2 is not a container.
            else
            {
                send_to_char("That's not a container.\n",ch);
                return;
            }
        }

        //check list <keyword> - for mobs and objects in the room.
        else if (*arg1)
        {
            //for loops - same as for 'check obj list' and 'check mob list', but i isn't reset to print a continuous list.
            int i=0;
            for (tch = (get_char_room_vis (ch, arg1)) ; tch; tch = tch->next_in_room)
            {
                if (isname (arg1, tch->name) && CAN_SEE (ch, tch))
                {
                    i++;
                    sprintf (buf, "%i.%s: #5%s#0 \n", i,arg1, char_short(tch));
                    send_to_char (buf, ch);
                }
            }

            for (obj=ch->room->contents; obj!=NULL; obj=obj->next_content)
            {
                if (isname (arg1, obj->name) && CAN_SEE_OBJ(ch,obj))
                {
                    i++;
                    sprintf (buf, "%i.%s: #2%s#0 \n", i, arg1, obj_short_desc(obj));
                    send_to_char (buf, ch);
                }
            }
            if (i==0)
                send_to_char ("There's nothing here with that keyword. \n", ch);
            return;
        }
    }

    //end 'check list (in <container>)

    //general check <keyword> (in <container>) from here on.
    //in this case, <keyword> should already be stored in arg1

    argument=one_argument (argument, arg2); //arg2 is either not there, 'in' or doesn't make sense.

    if (*arg2 && !strn_cmp (arg2, "in", strlen (arg2)))
    {
        argument=one_argument(argument, arg2); //arg2 should now be the container you're looking through.

        // 'in' but nothing follows
        if (!*arg2)
        {
            send_to_char ("Check inside what? \n", ch);
            return;
        }

        //check list in <container>
        //check if the container is there
        if (!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
                !(container = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
                !(container = get_obj_in_dark (ch, arg2, ch->equip)) &&
                !(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
        {
            send_to_char ("You don't see that container here.\n", ch);
            return;
        }

        //check if the object is a container
        if (GET_ITEM_TYPE (container) == ITEM_CONTAINER ||
                GET_ITEM_TYPE (container) == ITEM_QUIVER ||
                GET_ITEM_TYPE (container) == ITEM_SHEATH ||
                GET_ITEM_TYPE (container) == ITEM_HOLSTER ||
                GET_ITEM_TYPE (container) == ITEM_AMMO_BELT ||
                GET_ITEM_TYPE (container) == ITEM_BANDOLIER ||
                GET_ITEM_TYPE (container) == ITEM_KEYRING)
        {
            //container closed
            if (IS_SET (container->o.container.flags, CONT_CLOSED))
            {
                send_to_char ("It is closed.\n", ch);
                return;
            }

            if ((obj = get_obj_in_dark (ch, arg1, container->contains)))
            {
                sprintf (buf,"Inside #2%s#0, '%s' will target #2%s#0 \n",obj_short_desc(container), arg1, obj_short_desc(obj));
                send_to_char (buf, ch);
                return;
            }
            else
            {
                send_to_char ("None inside the container. \n", ch);
                return;
            }
        }
        else
        {
            send_to_char("That's not a container. \n",ch);
            return;
        }
    }

    //check <keyword>
    //find mobs with <keyword> first, objects later
    if ((tch=(get_char_room_vis (ch, arg1))))
    {
        sprintf (buf, "Typing '%s' will target #5%s#0. \n", arg1, char_short(tch));
        send_to_char (buf, ch);
        return;
    }

    //objects with <keyword>

	if ((obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
    {
        sprintf (buf,"Typing '%s' will target #2%s#0. \n", arg1, obj_short_desc(obj));
        send_to_char (buf, ch);
        return;
    }
    send_to_char("You do not see that here.\n",ch);
    return;
}

void
do_smell (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *tch = NULL;
    OBJ_DATA *obj = NULL;
    ROOM_DATA *room = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int scent_count = 0;
    SCENT_DATA *scent = NULL;
    SCENT_DATA *scent_one = NULL;
    SCENT_DATA *scent_two = NULL;
    SCENT_DATA *scent_three = NULL;

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        room = ch->room;
        if (!room->scent)
        {
            act ("You sniff the air, but don't detect any particular scent.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n sniffs the air.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
            return;
        }
        else
        {
            for (scent = room->scent; scent; scent = scent->next)
            {
                if (scent_strength(scent))
                {
                    if (!scent_one)
                    {
                        scent_one = scent;
                        scent_count++;
                    }
                    else if (!scent_two)
                    {
                        scent_two = scent;
                        scent_count++;
                    }
                    else if (!scent_three)
                    {
                        scent_three = scent;
                        scent_count++;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_one))
                    {
                        scent_one = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_two = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_three = scent;
                    }
                    else
                        continue;
                }
            }

            if (scent_count == 3)
            {
                sprintf(buf2, "You sniff the air, and detect %s of %s, %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
            }
            else if (scent_count == 2)
            {
                sprintf(buf2, "You sniff the air, and detect %s of %s and %s of %s. ",  scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
            }
            else if (scent_count == 1)
            {
                sprintf(buf2, "You sniff the air, and detect %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
            }
            else
            {
                sprintf(buf2, "You sniff the air, but don't detect any particular scent.");
            }

            act (buf2, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n sniffs the air.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
            return;

        }
        return;
    }

    if (!(tch = get_char_room_vis (ch, buf)))
    {
        if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
                !(obj = get_obj_in_dark (ch, buf, ch->left_hand)) &&
                !(obj = get_obj_in_dark (ch, buf, ch->equip)) &&
                !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
        {
            send_to_char ("You don't see that here.\n", ch);
            return;
        }
        else if (!obj->scent)
        {
            act ("You sniff $p, but don't detect any particular scent.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n sniffs $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
            return;
        }
        else
        {
            for (scent = obj->scent; scent; scent = scent->next)
            {
                if (scent_strength(scent))
                {
                    if (!scent_one)
                    {
                        scent_one = scent;
                        scent_count++;
                    }
                    else if (!scent_two)
                    {
                        scent_two = scent;
                        scent_count++;
                    }
                    else if (!scent_three)
                    {
                        scent_three = scent;
                        scent_count++;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_one))
                    {
                        scent_one = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_two = scent;
                    }
                    else if (scent_strength(scent) > scent_strength(scent_two))
                    {
                        scent_three = scent;
                    }
                    else
                        continue;
                }
            }

            if (scent_count == 3)
            {
                sprintf(buf2, "You sniff $p, and detect %s of %s, %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
            }
            else if (scent_count == 2)
            {
                sprintf(buf2, "You sniff $p, and detect %s of %s and %s of %s. ",  scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
            }
            else if (scent_count == 1)
            {
                sprintf(buf2, "You sniff $p, and detect %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
            }
            else
            {
                sprintf(buf2, "You sniff $p, but don't detect any particular scent.");
            }

            act (buf2, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
            act ("$n sniffs $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
            return;

        }
    }
    else if (tch == ch)
    {
        act ("Why don't you just examine yourself instead?", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
        return;
    }
    else if (!tch->scent)
    {
        act ("You sniff $N, but don't detect any particular scent.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
        act ("$n sniffs $N.", false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
        act ("$n sniffs you.", false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
        return;
    }
    else
    {
        for (scent = tch->scent; scent; scent = scent->next)
        {
            if (scent_strength(scent))
            {
                if (!scent_one)
                {
                    scent_one = scent;
                    scent_count++;
                }
                else if (!scent_two)
                {
                    scent_two = scent;
                    scent_count++;
                }
                else if (!scent_three)
                {
                    scent_three = scent;
                    scent_count++;
                }
                else if (scent_strength(scent) > scent_strength(scent_one))
                {
                    scent_one = scent;
                }
                else if (scent_strength(scent) > scent_strength(scent_two))
                {
                    scent_two = scent;
                }
                else if (scent_strength(scent) > scent_strength(scent_two))
                {
                    scent_three = scent;
                }
                else
                    continue;
            }
        }

        if (scent_count == 3)
        {
            sprintf(buf2, "You sniff $N, and detect %s of %s, %s of %s and %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref), scent_tier[scent_strength(scent_three)], scent_lookup(scent_three->scent_ref));
        }
        else if (scent_count == 2)
        {
            sprintf(buf2, "You sniff $N, and detect %s of %s and %s of %s. ",  scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref), scent_tier[scent_strength(scent_two)], scent_lookup(scent_two->scent_ref));
        }
        else if (scent_count == 1)
        {
            sprintf(buf2, "You sniff $N, and detect %s of %s. ", scent_tier[scent_strength(scent_one)], scent_lookup(scent_one->scent_ref));
        }
        else
        {
            sprintf(buf2, "You sniff $N, but don't detect any particular scent.");
        }

        act (buf2, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
        act ("$n sniffs $N.", false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
        act ("$n sniffs you.", false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
        return;
    }
}
void do_progress (CHAR_DATA * ch, char *argument, int cmd)
{
 int i = 0;
    int j = 2;
    int loaded_char = 0;
    CHAR_DATA *who = NULL;
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    AFFECTED_TYPE *af;
	int x = 0;
	
    argument = one_argument (argument, buf);





    if (IS_MORTAL (ch) || !*buf)
        who = ch;
    else if (!(who = get_char_room_vis (ch, buf)))
    {
        if (!(who = load_pc (buf)))
        {
            send_to_char ("No body here by that name.\n", ch);
            return;
        }
        else
            loaded_char = 1;
    }

    *buf = '\0';
x =	( 550 + ( 40 * ( ch->intel - 10 ) ) );

    if (IS_MORTAL (ch))
    {

		sprintf(buf, "\nSkill Cap: %s", skill_max( ch, 0, 1 ) > x * .75 ? " XXX " : skill_max( ch, 0, 1 ) > x * .5 ? " XX " : " X ");
        while (strlen (buf) < 32)
            strcat (buf, " "); 
send_to_char("\n", ch);
        for (i = 1; i <= LAST_SKILL; i++)
        {
	

            // Don't show up skills less that are 10 or less, so people don't have a screen
            // cluttered by a heap of 1 and 2 point skills.

           if (!real_skill (ch, i) || (ch->skills[i] < 5))
                continue;

            if (IS_MORTAL(ch) && i == SKILL_COMMON)
                continue;

       else
            {
                sprintf (buf2, "%s%12s:#0 %s ", (ch->skills[i] == calc_lookup (ch, REG_CAP, i) ? "#3" : "#2"), skills[i], progress_lev (who->skills[i]));
                while (strlen (buf2) < 31)
                    strcat (buf2, " ");
            }


            strcat (buf, buf2);

            if (!(j % 3))
                strcat (buf, "\n");
            j++;
        }
    }

    else
    {
        sprintf (buf, " #2Prowess#0[%03d/%03d] ", who->offense, 60);

        for (i = 1; i <= LAST_SKILL; i++)
        {
            if (real_skill (who, i))
            {
                sprintf (buf + strlen (buf),
                         "#2%8.8s#0[%03d/%03d] ", skills[i], who->skills[i],
                         calc_lookup (who, REG_CAP, i));
                if (!(j % 4))
                    strcat (buf, "\n");
                j++;
            }
        }
    }

    if (skill_max(ch, 0, 0))
    {
        if (buf[strlen (buf) - 1] != '\n')
            strcat (buf, "\n");

        sprintf (buf2, "\n               #6You have reached your full skill potential.#0\n");
        strcat (buf, buf2);
    }

    send_to_char (buf, ch);

    if (buf[strlen (buf) - 1] != '\n')
        send_to_char ("\n", ch);

    if (loaded_char)
        unload_pc (who);

}
