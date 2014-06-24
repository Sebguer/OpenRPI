/*------------------------------------------------------------------------\
|  act.comm.c : Communication Module		      www.middle-earth.us |
|  Copyright (C) 2005, Shadows of Isildur: Traithe		          |
|  Derived under license from DIKU GAMMA (0.0).				  |
\------------------------------------------------------------------------*/

#include <string>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>


#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "group.h"

/* extern variables */

extern ROOM_DATA *world;
extern DESCRIPTOR_DATA *descriptor_list;
extern const char *skills[];

extern const char *rev_d[] =
{
	"the south,",
	"the west,",
	"the north,",
	"the east,",
	"below,",
	"above,"
	"inside",
	"outside",
	"the southwest",
	"the southeast",
	"the northwest",
	"the northeast"
};


#define TOK_WORD		0
#define	TOK_NEWLINE		1
#define TOK_PARAGRAPH	2
#define TOK_END			3
#define TOK_SENTENCE	4

int AudienceRoom[4] = {55498, 55503, 55504, 55501};
int StageRoom = 55053;

int static_distance_mod (OBJ_DATA *sender, OBJ_DATA *receiver, bool radio_station)
{
	if (!sender || !receiver)
		return 0;

	// We halve to get our static effect.
	double working_distance = 0;
	int grunge_bonus = 0;

	int sent_room = 0;
	int rec_room = 0;

	// Work out what room we are in.
	if (sender->carried_by)
		sent_room = sender->carried_by->in_room;
	else if (sender->equiped_by)
		sent_room = sender->equiped_by->in_room;
	else if (sender->in_room)
		sent_room = sender->in_room;

	// Work out what room we are sending from.
	if (receiver->equiped_by)
		rec_room = receiver->equiped_by->in_room;
	else if (receiver->carried_by)
		rec_room = receiver->carried_by->in_room;
	else if (receiver->in_room)
		rec_room = receiver->in_room;

	int sent_zone = sent_room / 100;
	int rec_zone = rec_room / 100;

	if (!sent_room || !rec_room || sent_room == -1 || rec_room == -1)
	{
		return 0;
	}

	if (sent_zone == 75 ||
		sent_zone == 56 ||
		sent_zone == 66 ||
		sent_zone == 67 ||
		sent_zone == 68)
	{
		grunge_bonus += 15;
	}

	if (rec_zone == 75 ||
		rec_zone == 56 ||
		rec_zone == 66 ||
		rec_zone == 67 ||
		rec_zone == 68)
	{
		grunge_bonus += 15;
	}

	ROOM_DATA *temp_room;

	int dir = -1;
	while (sent_room != rec_room)
	{
		temp_room = vnum_to_room(sent_room);

		if (IS_SET(temp_room->room_flags, BIG_ROOM))
		{
			working_distance += 3;
		}
		else if (IS_SET(temp_room->room_flags, SMALL_ROOM))
		{
			working_distance += 0.5;
		}
		else
		{
			working_distance += 1;
		}

		dir = pathfind (sent_room, rec_room);

		if (dir == -1)
		{
			working_distance = -1;
			break;
		}

        if (!temp_room->dir_option[dir])
		{
			working_distance = -1;
			break;
		}

		if (!vnum_to_room(sent_room = temp_room->dir_option[dir]->to_room))
		{
			working_distance = -1;
			break;
		}
	}

	if (working_distance == -1)
	{
		if (grunge_bonus == 30)
		{
			return 0;
		}
		else if (rec_zone == sent_zone)
		{
			return 10;
		}
		else if (grunge_bonus == 15)
		{
			return 20;
		}
	}

	working_distance = working_distance / 2;

	working_distance = working_distance - grunge_bonus;

	working_distance = MAX(1, (int) working_distance);

	if (grunge_bonus == 30)
	{
		working_distance = MIN(1, (int) working_distance);
	}

	if (radio_station)
	{
		working_distance = MIN(1, (int) working_distance);
	}

	return working_distance;
}

int static_it (OBJ_DATA * obj, char *source, char *target, bool radio_station, int dist_penalty)
{
	int missed = 0;
	double count = 0;
	double failed = 0;
	int got_one = 0;
	int bonus = 0;
	int base = 0;
	int percentage = 0;
	char *in = '\0';
	char *out = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };

	//if (ch->skills[SKILL_PROWESS])
	//bonus = 20;


	if (radio_station)
	{
		base = 100 - (int)(dist_penalty/2);
	}
	else
	{
		base = 99 - (int)(dist_penalty/1);
	}

	if (base <= 0)
		base = 100;

	in = source;
	out = buf;
	*out = '\0';

	while (*in)
	{
		while (*in == ' ')
		{
			in++;
			*out = ' ';
			out++;
		}

		*out = '\0';

		count += 1;

		if (base + bonus < number (1, SKILL_CEILING))
		{
			failed += 1;

			if (!missed)
			{
				switch (number(0,5))
				{
				case 0:
					strcat (out, " zzzzt");
					break;
				case 1:
					strcat (out, " . . .");
					break;
				case 2:
					strcat (out, " chzch");
					break;
				case 3:
					strcat (out, " wztzt");
					break;
				case 4:
					strcat (out, " #&*#!");
					break;
				case 5:
					strcat (out, " krkch");
					break;
				}
				out += strlen (out);
			}

			missed = 1;

			while (*in && *in != ' ')
				in++;
		}
		else
		{
			while (*in && *in != ' ')
			{
				*out = *in;
				out++;
				in++;
			}

			got_one = 1;
			missed = 0;
		}

		*out = '\0';
	}

	percentage = int (((count - failed) / count) * 100);

	strcpy (target, buf);

	return percentage;
}

		char *statics[] =
	{
		"", //90~100%
		"faintly distorted, ", // > 80%
		"tinged with static, ", // > 70%
		"buzzing with static, ", // > 60%
		"crackling and popping, ", // > 50%
		"dropping in and out, ",  // > 40%
		"barely audible through the distortion, ", // > 30%
		"in a mess of static and hissing, ", // < 30%
		"with a clear and powerful signal, "
	};

void
	heard_action(CHAR_DATA *ch, char *flavour, char *text, int dist, int mode)
{
	int dir;
	CHAR_DATA *tch;
	AFFECTED_TYPE *waf;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *next_room;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	char buf3[MAX_STRING_LENGTH] = {'\0'};

	sprintf(buf3, flavour);
	*buf3 = tolower(*buf3);

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
		{
			continue;
		}

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			*buf2 = '\0';
			if ((waf = get_affect (tch, AFFECT_LISTEN_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge/* && could_see(tch, ch)*/)
				{
					whisper_it (tch, text, buf2, 1);
					if (!mode)
					{
						sprintf (buf, "To the %s, you overhear %s %s.\n",
							direction[rev_dir[waf->a.shadow.edge]], buf3, buf2);
					}
					else
					{
						sprintf (buf, "%s, %s",	direction[rev_dir[waf->a.shadow.edge]], buf2);
					}

					act(buf, true, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
				}
			}
		}
	}
}

void
	do_eavesdrop (CHAR_DATA * ch, char *argument, int cmd)
{
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *taf = NULL;
	char buf[MAX_STRING_LENGTH];
	int dir;

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if ((dir = index_lookup (dirs, buf)) == -1)
		{
			send_to_char ("In which direction did you wish to eavesdrop?\n", ch);
			return;
		}

		if (!(taf = get_affect (ch, AFFECT_SHADOW)) || (taf->a.shadow.edge != dir))	/* On the edge */
		{
			send_to_char ("You need to be standing by an edge to eavesdrop in that direction.\n", ch);
			return;
		}

		if (!(taf = get_affect (ch, AFFECT_WATCH_DIR)) || (taf->a.shadow.edge != dir))	/* On the edge */
		{
			send_to_char ("You need to be watching that direction before you can eavesdrop in that direction.\n", ch);
			return;
		}

		if (!(af = get_affect (ch, AFFECT_LISTEN_DIR)))
			magic_add_affect (ch, AFFECT_LISTEN_DIR, -1, 0, 0, 0, 0);

		af = get_affect (ch, AFFECT_LISTEN_DIR);

		af->a.shadow.shadow = NULL;
		af->a.shadow.edge = dir;

		sprintf (buf, "You will now eavesdrop to the %s.\n", dirs[dir]);
		send_to_char (buf, ch);
		sprintf (buf, "$n turns to eavesdrop to the %s.", dirs[dir]);
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!*buf)
	{
		if (!get_affect (ch, AFFECT_LISTEN_DIR))
		{
			send_to_char ("You are not currently eavesdropping in any direction.\n", ch);
			return;
		}
		else
		{
			send_to_char ("You cease eavesdropping.\n", ch);
			remove_affect_type (ch, AFFECT_LISTEN_DIR);
			return;
		}
	}
}



void
	reformat_say_string (char *source, char **target, CHAR_DATA * to)
{
	int token_value = 0;
	int first_line = 1;
	int line_len = 0;
	char *s = '\0';
	char *r = '\0';
	char token[MAX_STRING_LENGTH] = { '\0' };
	char result[MAX_STRING_LENGTH] = { '\0' };

	s = source;
	r = result;
	*result = '\0';

	line_len = 0;

	while (token_value != TOK_END)
	{

		token_value = get_token (&s, token);

		if (token_value == TOK_PARAGRAPH)
		{

			if (first_line)
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if (token_value == TOK_NEWLINE)
		{
			if (line_len != 0)
				strcat (result, "\n");	/* Catch up */
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if (token_value == TOK_WORD)
		{
			if (line_len + strlen (token) > 72)
			{
				strcat (result, "\n    ");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	result[strlen (result) - 1] = '\0';

	if (result[strlen (result) - 1] != '.' &&
		result[strlen (result) - 1] != '!' &&
		result[strlen (result) - 1] != '?')
		result[strlen (result)] = '.';

	*target = str_dup (result);
}

#include <memory>
void
	do_ooc (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (IS_SET (ch->flags, FLAG_GUEST) && IS_SET (ch->room->room_flags, OOC))
	{
		do_say (ch, argument, 0);
		return;
	}

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

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char ("Surely you can think of more to say than that!\n", ch);

	else
	{
		std::string formatted = argument;
		formatted[0] = toupper (formatted[0]);
		char *p = 0;
		char *s = str_dup (formatted.c_str ());
		reformat_say_string (s, &p, 0);
		sprintf (buf, "$n says, out of character,\n   \"%s\"", p + i);
		act (buf, false, ch, 0, 0, TO_ROOM);
		sprintf (buf, "You say, out of character,\n   \"%s\"\n", p + i);
		send_to_char (buf, ch);
		mem_free (s);
		mem_free (p);
	}
}

void
	do_pmote (CHAR_DATA * ch, char *argument, int cmd)
{
	char * result = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "Your current pmote: (#2%s#0)\n",
			(ch->pmote_str) ? ch->pmote_str : "(#2none#0)");
		send_to_char (buf, ch);
	}

	else if (!strcmp (argument, "normal"))
	{
		sprintf (buf, "Your current pmote has been cleared.");
		act (buf, false, ch, 0, 0, TO_CHAR);
		clear_pmote (ch);
	}

	else if (IS_NPC(ch) && argument)
	{
		ch->pmote_str = add_hash (argument);
	}

	else
	{
		if (ch && argument)
		{

			result = swap_xmote_target (ch, argument, 2);
			if (!result)
				return;
		}

		sprintf (buf, "You pmote: %s", result);

		ch->pmote_str = add_hash (result);

		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

void
	do_dmote (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "Your current dmote: (%s)\n",
			(ch->dmote_str) ? ch->dmote_str : "(none)");
		send_to_char (buf, ch);
	}

	else if (!strcmp (argument, "normal"))
	{
		sprintf (buf, "Your current dmote has been cleared.");
		act (buf, false, ch, 0, 0, TO_CHAR);
		clear_dmote (ch);
	}

	else
	{
		sprintf (buf, "You dmote: %s", argument);

		ch->dmote_str = add_hash (argument);

		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

void
	clear_dmote (CHAR_DATA * ch)
{
	if (ch->dmote_str)
	{
		mem_free (ch->dmote_str); // char*
		ch->dmote_str = NULL;
	}
}

void
	do_omote (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[AVG_STRING_LENGTH * 4] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char * result = NULL;
	char *p2 = '\0';
	bool omote_lock = false;
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	int quotes; //counts the number of quotation marks, if uneven, notify the player instead of sending the emote.
	int position = 0;
	OBJ_DATA *obj = NULL;

	if (argument[strlen(argument) - 1] == '!')
	{
		char original_argument[MAX_STRING_LENGTH] = {'\0'};
        for (size_t y = 0; y < strlen (argument) - 2; y++)
        {
			sprintf (original_argument + strlen (original_argument), "%c", argument[y]);
		}
		strcpy(argument, original_argument);
		omote_lock = true;
	}

	argument = one_argument (argument, arg1);

if (strstr (argument, "\""))
	{

		p2=argument;
		while (*p2) //loop to count the number of quotation marks
		{
			if (*p2=='\"')
			{
				quotes++;

				if (quotes%2 && quotes > 1)
				{
					sprintf(buf3+strlen(buf3), ". ");
				}

			}
			p2++;
			position++;

			if (quotes%2)
			{
				sprintf(buf3+strlen(buf3), "%c", argument[position]);
			}

		}

		if ((quotes % 2) != 0) //if even, do nothing, if it's an odd number the player gets notified.
		{
			send_to_char ("You'll need to close those quotation marks.\n ", ch);
			return;
		}

	}
	


	if (!*arg1)
	{
		send_to_char ("What would you like to omote on?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
	{
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if (!CAN_WEAR (obj, ITEM_TAKE) && IS_MORTAL (ch))
	{
		send_to_char ("You can't omote on that.\n", ch);
		return;
	}

	if (IS_SET (ch->room->room_flags, OOC))
	{
		send_to_char ("You can't do this in an OOC area.\n", ch);
		return;
	}

	if (!*argument)
	{
		send_to_char ("What will you omote?\n", ch);
		return;
	}

	result = swap_xmote_target (ch, argument, 3);
	if (!result)
		return;

	if (strlen (result) >= AVG_STRING_LENGTH * 4)
	{
		send_to_char ("Your omote needs to be more succinct.\n", ch);
		return;
	}

	if (get_obj_affect(obj, MAGIC_OMOTED))
	{
		send_to_char ("That item has an omote-lock upon it - you will need to pick it up and drop it to place another omote.\n", ch);
		return;
	}

	if (omote_lock)
	{
        if (IS_SET (obj->tmp_flags, SA_DROPPED))
        {
            send_to_char ("That can't be omoted upon at the moment.\n", ch);
            return;
        }

		remove_obj_mult_affect (obj, MAGIC_OMOTED);	/* Probably doesn't exist */

		AFFECTED_TYPE *af = NULL;
		af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

		af->type = MAGIC_OMOTED;
		af->a.spell.location = -1;
		af->a.spell.modifier = 0;
		af->a.spell.t = 0;
		af->a.spell.duration = -1;
        af->a.spell.bitvector = 0;
        af->a.spell.sn = 0;
		af->next = obj->xaffected;
		obj->xaffected = af;
	}

	sprintf (buf, "%s%s%s",
		result,
		(result[strlen (result) - 1] != '.') ? "." : "",
		(obj->short_description[0] == '#') ? "#0" : "");

	obj->omote_str = add_hash (buf);
	sprintf (buf, "You omote: %s %s", obj->short_description, obj->omote_str);

	if (obj->short_description[0] == '#')
	{
		buf[13] = toupper (buf[13]);
	}

	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
}



void do_think (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf1[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char feel[MAX_STRING_LENGTH] = { '\0' };
	//char voice[MAX_STRING_LENGTH] = { '\0' };
	char key[MAX_STRING_LENGTH] = { '\0' };
	int i = 0, key_e = 0;

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		send_to_char ("What would you like to think?\n", ch);
		return;
	}

	if (IS_SET (ch->plr_flags, NOPETITION))
	{
		act ("Your ability to petition/think has been revoked by an admin.",
			false, ch, 0, 0, TO_CHAR);
		return;
	}

	//Get the intro phrase and the message
	if ( * argument == '(' )
	{
		* buf = '\0';
		sprintf( buf, "%s", argument );
		i = 1;

		* feel = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				send_to_char ("What did you wish to think?\n", ch);
				return;
			}

			// Object key encountered
			if (buf[i] == '*')
			{
				i++;

				// Scans forward until a non-alpha character is encountered
				while ( isalpha( buf[ i ] ) )
					key[ key_e++ ] = buf[ i++ ];

                // Terminating the end of the string.
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis( ch, key, ch->room->contents ) &&
					!get_obj_in_list_vis( ch, key, ch->right_hand ) &&
					!get_obj_in_list_vis( ch, key, ch->left_hand ) &&
					!get_obj_in_list_vis( ch, key, ch->equip ) )
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);

				sprintf (feel + strlen (feel), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}

			// Mob key encountered
			if (buf[i] == '~')
			{
				i++;

				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];

				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				sprintf (feel + strlen (feel), "#5%s#0",
					char_short (get_char_room_vis (ch, key)));
				*key = '\0';
				continue;
			}

			sprintf (feel + strlen (feel), "%c", buf[i]);
			i++;
		}

		while ( * argument != ')' )
            if ( * argument == '\0' ) {
                send_to_char( "What would you like to think?\n", ch );
                return;
            } else argument++;

		argument ++;
		if ( * argument == '\0' ) {
            send_to_char( "What would you like to think?\n", ch );
            return;
        } else argument++;

		if (!*feel)
		{
			send_to_char ("What would you like to think?\n", ch);
			return;
		}


	}

	*buf = '\0';
	*buf1 = '\0';
	*buf2 = '\0';

	if (!*argument)
	{
		send_to_char ("What would you like to think?\n", ch);
		return;
	}

	if (argument[strlen (argument) - 1] != '.' &&
        argument[strlen (argument) - 1] != '!' &&
        argument[strlen (argument) - 1] != '?' &&
        argument[strlen (argument) - 3] != '\"')
		strcat (argument, ".");

	if (!*feel)
	{
		sprintf (buf, "You thought: %s", argument);
		sprintf (buf1, "#6%s#0 thinks, \"%s\"", GET_NAME (ch), argument);
		reformat_say_string (argument, &p, 0);
		sprintf (buf2, "#6You hear #5%s#6 think,\n   \"%s\"#0\n", char_short (ch), p);
		mem_free (p); // char*
	}
	else
	{
		sprintf (buf, "%s, you thought: %s", feel, argument);
		sprintf (buf1, "#6%s#0 thinks, %s, \"%s\"", GET_NAME(ch), feel, argument);
		*buf = toupper(*buf);
		*buf1 = toupper(*buf1);

		reformat_say_string (argument, &p, 0);
		sprintf (buf2, "#6You hear #5%s#6 think, %s,\n   \"%s\"#0\n", char_short (ch), feel, p);
		mem_free (p); // char*
	}

	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	/* Send thoughts to global Imm telepaths */
	for (tch = character_list; tch; tch = tch->next)
	{
		if (tch->deleted)
			continue;
		if (IS_MORTAL (tch))
			continue;
		if (tch == ch)
			continue;
		if (!IS_SET (tch->flags, FLAG_TELEPATH))
			continue;
		act (buf1, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	/* Send thoughts to in-room PC telepaths */
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch)		/* You don't get an echo of your own throught */
			continue;
		if (!IS_MORTAL (ch) && !IS_NPC (ch))	/* Imm thinks are not overheard */
			continue;
		if (!IS_MORTAL (tch))	/* Imms get a different echo */
			continue;
		if (skill_use (tch, SKILL_VOODOO, ch->skills[SKILL_VOODOO] / 3)
			|| (IS_NPC (ch) && tch->skills[SKILL_VOODOO]))
			send_to_char (buf2, tch);
	}
}

void
	do_feel (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf1[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	//char voice[MAX_STRING_LENGTH] = { '\0' };

	if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		send_to_char ("What would you like to feel?\n", ch);
		return;
	}

	if (IS_SET (ch->plr_flags, NOPETITION))
	{
		act ("Your ability to petition/think has been revoked by an admin.",
			false, ch, 0, 0, TO_CHAR);
		return;
	}

	*buf = '\0';
	*buf1 = '\0';
	*buf2 = '\0';

	if (argument[strlen (argument) - 1] != '.' && argument[strlen (argument) - 1] != '!' && argument[strlen (argument) - 1] != '?' && argument[strlen (argument) - 3] != '\"')
		strcat (argument, ".");

	sprintf (buf, "You feel %s", argument);
	sprintf (buf1, "%s feels %s", GET_NAME (ch), argument);
	reformat_say_string (argument, &p, 0);
	sprintf (buf2, "#6You attune to #5%s#6 feeling %s#0\n", char_short (ch), p);
	mem_free (p); // char*

	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	/* Send thoughts to global Imm telepaths */
	for (tch = character_list; tch; tch = tch->next)
	{
		if (tch->deleted)
			continue;
		if (IS_MORTAL (tch))
			continue;
		if (tch == ch)
			continue;
		if (!IS_SET (tch->flags, FLAG_TELEPATH))
			continue;
		act (buf1, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	/* Send thoughts to in-room PC telepaths */
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch)		/* You don't get an echo of your own throught */
			continue;
		if (!IS_MORTAL (ch) && !IS_NPC (ch))	/* Imm thinks are not overheard */
			continue;
		if (!IS_MORTAL (tch))	/* Imms get a different echo */
			continue;
		if (skill_use (tch, SKILL_VOODOO, ch->skills[SKILL_VOODOO] / 3)
			|| (IS_NPC (ch) && tch->skills[SKILL_VOODOO]))
			send_to_char (buf2, tch);
	}
}



// Can you see the hidden emote?
// 1. if you're in the group, yes you can
// 2. if you can otherwise see them, yes you can
// Otherwise, it's roll-skis
// 3. default roll is 1d100 vs your (int score * 2) OR eavesdrop skill, whichever is higher
// 4. if you're sitting at the table, +25
// 5. if you're watching them, +50
// 6. if -they're- hidden, -25
// 7. if the emote involves the other character, +25
// minimum chance of 5%.

bool
	seen_hidde_emote (CHAR_DATA *src, CHAR_DATA *tar, int personal)
{
	int roll = 0;
	int test = 0;
	AFFECTED_TYPE *af = NULL;

	// If we can see them, or are grouped with them, then return true.

	if (GET_TRUST(tar) || are_grouped(src, tar))
		return true;

	// Use whatever is better - eavesdrop or int x 2

	if (tar->skills[SKILL_EAVESDROP] > (GET_INT(tar) * 2))
		test = tar->skills[SKILL_EAVESDROP];
	else
		test = GET_INT(tar) * 2;

	// If you're at their table, you get +25

	if ((af = get_affect (src, MAGIC_SIT_TABLE)) && is_at_table (tar, af->a.table.obj))
	{
		test += 25;
	}

	// If you're watching them, you get +50

	if ((af = get_affect(tar, MAGIC_WATCH)) && (CHAR_DATA *) af->a.spell.t == src)
	{
		test += 50;
	}

	// If you're hidden, you get -25

	if (get_affect(src, MAGIC_HIDDEN))
	{
		test -= 25;
	}

	// If the emoted involved tar, give'em a bonus.

	if (personal)
	{
		test += personal;
	}


	// Always 5 % chance of succeeding, 5% chance of failing.
	test = MAX(test, 5);
	test = MIN(test, 95);

	roll = number(1,100);

	if (roll > test)
	{
		return false;
	}
	else
	{
		return true;
	}


}


bool
	personalize_hidden_string (CHAR_DATA * src, CHAR_DATA * tar, char *emote)
{
	AFFECTED_TYPE *af;
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL, *target = NULL;
	bool seen = false;

	*output = '\0';

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				for (tch = tar->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
					{
						break;
					}
					emote++;
					emote++;
					if (*emote == '\'')
					{
						strcat (desc, "\'");
						emote++;
						if (*emote == 's')
						{
							strcat (desc, "s");
						}
						else
						{
							emote--;
						}
					}
					else
					{
						emote--;
						emote--;
					}
					if (!tch)
						continue;
					if (tch == tar)
					{
						sprintf (buf, "%c", desc[strlen (desc) - 1]);
						if (desc[strlen (desc) - 1] == '\''
							|| desc[strlen (desc) - 2] == '\'')
						{
							strcat (output, "#5your#0");
							emote--;
						}
						else
							strcat (output, "#5you#0");
						target = tch;
						emote++;
					}
					else
					{
						sprintf (buf, "#5%s#0", char_short (tch));
						strcat (output, buf);
						emote--;
						if (*emote == '\'')
							emote--;
					}
			}
			else
				sprintf (output + strlen (output), "#%c", *emote);
		}
		else
			sprintf (output + strlen (output), "%c", *emote);
		emote++;
	}
	if (target)
	{
		if (*output == '#')
			output[2] = toupper (output[2]);

		if (seen_hidde_emote(src, target, 25))
		{
			//send_to_gods("test1");
			if (!are_grouped(src, target) && !GET_TRUST(target) && (af = (get_affect(src, MAGIC_HIDDEN))))
			{
				seen = true;
			}
			act (output, false, src, 0, target, TO_VICT | _ACT_FORMAT);
		}
		magic_add_affect (target, MAGIC_SENT, -1, 0, 0, 0, 0);
	}

	return seen;
}

bool
	personalize_hemote (CHAR_DATA * src, char *emote)
{
	AFFECTED_TYPE *af;
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL;
	bool seen = false;

	sprintf (copy, "%s", emote);

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				tch = get_char_room_vis (src, desc);
				for (tch = src->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
						break;
				if (!tch)
					continue;
				if (!get_affect (tch, MAGIC_SENT))
					seen = personalize_hidden_string (src, tch, copy);
			}
		}
		emote++;
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == src)
			continue;
		if (get_affect (tch, MAGIC_SENT))
			continue;

		if (seen_hidde_emote(src, tch, 0))
		{
			if (!are_grouped(src, tch) && !GET_TRUST(tch) && (af = (get_affect(src, MAGIC_HIDDEN))))
			{
				seen = true;
			}
			act (copy, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
			//send_to_gods("test2");
		}
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (get_affect (tch, MAGIC_SENT))
			affect_remove (tch, get_affect (tch, MAGIC_SENT));
	}

	return seen;
}

void
	do_hemote (CHAR_DATA * ch, char *argument, int cmd)
{
	bool seen = false;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	char *result = NULL;
	char *p2 = '\0';
	int quotes; //counts the number of quotation marks, if uneven, notify the player instead of sending the emote.


	while (isspace (*argument))
		argument++;

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

	/*
	if (strstr (argument, "\""))
	{
	send_to_char
	("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
	ch);
	return;
	}
	*/

	//speech in emotes is now allowed - check if the number of quotation marks is even.
	if (strstr (argument, "\""))
	{
		/*
		send_to_char
		("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
		ch);
		return;
		*/

		p2=argument;
		while (*p2) //loop to count the number of quotation marks
		{
			if (*p2=='\"')  quotes++;
			p2++;
		}

		if ((quotes % 2) != 0) //if even, do nothing, if it's an odd number the player gets notified.
		{
			send_to_char ("You'll need to close those quotation marks.\n ", ch);
			return;
		}

	}


	if (quotes)
	{
		if (IS_NPC(ch) && !str_cmp(lookup_race_variable(ch->race, RACE_NAME), "robot"))
		{
			if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "voice")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "voice")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "voice")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "voice")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
		}     
	}


	if (!*argument)
		send_to_char ("What would you like to emote?\n", ch);
	else
	{
		//p = copy;

		sprintf(copy, argument);

		/** Removed code and created swap_xmote_target function **/
		result = swap_xmote_target (ch, copy, 1);

		if (!result)
			return;

		sprintf (buf, "%s", result);

		seen = personalize_hemote (ch, buf); //adjusts for "you" if needed

		if (!strcmp(result, buf))
		{
			act (buf, false, ch, 0, 0, TO_ROOM | TO_CHAR | _ACT_FORMAT);
		}
		else
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		if (seen)
		{
			if (get_affect(ch, MAGIC_HIDDEN))
			{
				affect_remove(ch, get_affect(ch, MAGIC_HIDDEN));
				send_to_char("\nYour actions have compromised your concealment.\n",ch);
				act ("$n reveals $mself.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
		}

		/*

		if (!strcmp(result, buf))
		{
		send_to_gods("result and buff are equal in hmote");
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
		if (seen_hidde_emote(ch, tch, 0))
		{
		act (buf, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		send_to_gods("test3");
		}
		else if (tch == ch)
		act (buf, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		}
		else
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		if (!quotes)
		{
		*buf = tolower(*buf);
		watched_action(ch, buf, 0, 1);
		}
		else
		{
		watched_action(ch, buf, 0, 2);
		}
		*/
	}
}


void
	personalize_string (CHAR_DATA * src, CHAR_DATA * tar, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL, *target = NULL;

	*output = '\0';

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				for (tch = tar->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
					{
						break;
					}
					emote++;
					emote++;
					if (*emote == '\'')
					{
						strcat (desc, "\'");
						emote++;
						if (*emote == 's')
						{
							strcat (desc, "s");
						}
						else
						{
							emote--;
						}
					}
					else
					{
						emote--;
						emote--;
					}
					if (!tch)
						continue;
					if (tch == tar)
					{
						sprintf (buf, "%c", desc[strlen (desc) - 1]);
						if (desc[strlen (desc) - 1] == '\''
							|| desc[strlen (desc) - 2] == '\'')
						{
							strcat (output, "#5your#0");
							emote--;
						}
						else
							strcat (output, "#5you#0");
						target = tch;
						emote++;
					}
					else
					{
						sprintf (buf, "#5%s#0", char_short (tch));
						strcat (output, buf);
						emote--;
						if (*emote == '\'')
							emote--;
					}
			}
			else
				sprintf (output + strlen (output), "#%c", *emote);
		}
		else
			sprintf (output + strlen (output), "%c", *emote);
		emote++;
	}
	if (target)
	{
		if (*output == '#')
			output[2] = toupper (output[2]);
		act (output, false, src, 0, target, TO_VICT | _ACT_FORMAT);
		magic_add_affect (target, MAGIC_SENT, -1, 0, 0, 0, 0);
	}
}

void
	personalize_emote (CHAR_DATA * src, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL;

	sprintf (copy, "%s", emote);

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				tch = get_char_room_vis (src, desc);
				for (tch = src->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
						break;
				if (!tch)
					continue;
				if (!get_affect (tch, MAGIC_SENT))
					personalize_string (src, tch, copy);
			}
		}
		emote++;
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == src)
			continue;
		if (get_affect (tch, MAGIC_SENT))
			continue;
		act (copy, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (get_affect (tch, MAGIC_SENT))
			affect_remove (tch, get_affect (tch, MAGIC_SENT));
	}
}


void
	do_emote (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = {'\0'};
	char *result = NULL;
	char *p2 = '\0';
	int quotes = 0; //counts the number of quotation marks, if uneven, notify the player instead of sending the emote.
	int position = 0;

	while (isspace (*argument))
		argument++;

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

	/*
	if (strstr (argument, "\""))
	{
	send_to_char
	("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
	ch);
	return;
	}
	*/

	//speech in emotes is now allowed - check if the number of quotation marks is even.
	if (strstr (argument, "\""))
	{
		/*
		send_to_char
		("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
		ch);
		return;
		*/

		p2=argument;
		while (*p2) //loop to count the number of quotation marks
		{
			if (*p2=='\"')
			{
				quotes++;

				if (quotes%2 && quotes > 1)
				{
					sprintf(buf3+strlen(buf3), ". ");
				}

			}
			p2++;
			position++;

			if (quotes%2)
			{
				sprintf(buf3+strlen(buf3), "%c", argument[position]);
			}

		}

		if ((quotes % 2) != 0) //if even, do nothing, if it's an odd number the player gets notified.
		{
			send_to_char ("You'll need to close those quotation marks.\n ", ch);
			return;
		}

	}

	if (quotes)
	{
		if (IS_NPC(ch) && !str_cmp(lookup_race_variable(ch->race, RACE_NAME), "robot"))
		{
			if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "voice")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "voice")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "voice")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "voice")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
		}
     
	}

	if (!*argument)
		send_to_char ("What would you like to emote?\n", ch);
	else
	{
		//p = copy;

		sprintf(copy, argument);

		/** Removed code and created swap_xmote_target function **/
		result = swap_xmote_target (ch, copy, 1);

		if (!result)
			return;

		sprintf (buf, "%s", result);

		personalize_emote (ch, buf); //adjusts for "you" if needed

		if (!strcmp(result, buf))
		{
			act (buf, false, ch, 0, 0, TO_ROOM | TO_CHAR | _ACT_FORMAT);
		}
		else
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		*buf = tolower(*buf);
		if (!quotes)
		{
			watched_action(ch, buf, 0, 1);
		}
		else
		{
			watched_action(ch, buf, 0, 2);
		}

		if (ch->in_room == StageRoom)
		{
			CHAR_DATA *tch = NULL;
			char buf2[MAX_STRING_LENGTH] = { '\0' };
			*buf = tolower(*buf);
			for (int index = 0; index < 4; index = index +1)
			{
				for (tch = vnum_to_room (AudienceRoom[index])->people; tch; tch = tch->next_in_room)
				{
					if (!tch->descr())	/* NPC don't hear anything */
						continue;

					sprintf (buf2, "On the stage, %s", buf);

					act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
				}
			}
		}
	}


	OBJ_DATA *bugObj = NULL;

	if (quotes)
	{
		for (bugObj = ch->room->contents; bugObj; bugObj = bugObj->next_content)
		{
			if (GET_ITEM_TYPE(bugObj) == ITEM_E_BUG && bugObj->o.elecs.status)
			{
				OBJ_DATA *tobj = NULL;
				OBJ_DATA *robj = NULL;
				CHAR_DATA *rch = NULL;
				CHAR_DATA *xch = NULL;
				ROOM_DATA *room = NULL;
				bool deciphered = false;
				CHAR_DATA *tch = NULL;
				int j = 0;
				int static_index = 0;
				char buf4[MAX_STRING_LENGTH] = {'\0'};
				char buf5[MAX_STRING_LENGTH] = {'\0'};
				char buf6[MAX_STRING_LENGTH] = {'\0'};
				for (tobj = object_list; tobj; tobj = tobj->next)
				{
					strcpy(buf4, buf3);
					bool heard = false;
					bool duplicated = false;
					bool personal = false;
					j = 0;
					room = NULL;
					tch = NULL;
					rch = NULL;
					xch = NULL;
					robj = NULL;

					if (tobj->deleted)
						continue;

					if (tobj == bugObj)
						continue;

					if (GET_ITEM_TYPE(tobj) != ITEM_E_RADIO)
						continue;

					// If the radio isn't on, then we can't do anything about it!
					if (tobj->o.radio.status == 0)
						continue;

					// We need to be tuned to the same channel!
					if (tobj->o.radio.channel != bugObj->o.radio.channel &&
						tobj->o.radio.channel/10 != bugObj->o.radio.channel)
						continue;

					if (tobj->in_room > 0)
						room = vnum_to_room(tobj->in_room);
					else if (tobj->equiped_by)
					{
						room = vnum_to_room(tobj->equiped_by->in_room);
						rch = tobj->equiped_by;
					}
					else if (tobj->carried_by)
					{
						room = vnum_to_room(tobj->carried_by->in_room);
						rch = tobj->carried_by;
					}
					else
						continue;

					// To avoid spam:
					// We need to run through -every- object that's visible from the room,
					// which includes everything on people as well, sadly enough. If the
					// radios have a "sent" signal on them, then it means we've already
					// echoed that message for the room, and we just make a note of that
					// instead of redoing them.
					// Not very efficient at all, but given we're already searching every
					// object in the game twice each time we radio, it should be OK.

					for (robj = room->contents; robj; robj = robj->next_content)
					{
						if (robj->o.radio.status != 0 && tobj != robj && robj != bugObj &&
							robj->o.radio.channel == tobj->o.radio.channel &&
							robj->o.radio.encrypt == tobj->o.radio.encrypt)
						{
							if (get_obj_affect(robj, MAGIC_SENT))
								heard = true;
							else
								duplicated = true;

							break;
						}
					}

					if (!heard && !duplicated)
					{
						for (xch = room->people; xch; xch = xch->next_in_room)
						{
							if (xch->right_hand && GET_ITEM_TYPE(xch->right_hand) == ITEM_E_RADIO)
							{
								if (xch->right_hand->o.radio.status != 0 && tobj != xch->right_hand && bugObj != xch->right_hand &&
									xch->right_hand->o.radio.channel == tobj->o.radio.channel &&
									xch->right_hand->o.radio.encrypt == tobj->o.radio.encrypt)
								{
									if (get_obj_affect(xch->right_hand, MAGIC_SENT))
										heard = true;
									else
										duplicated = true;

									break;
								}
							}
							if (xch->left_hand && GET_ITEM_TYPE(xch->left_hand) == ITEM_E_RADIO)
							{
								if (xch->left_hand->o.radio.status != 0 && tobj != xch->left_hand && bugObj != xch->left_hand &&
									xch->left_hand->o.radio.channel == tobj->o.radio.channel &&
									xch->left_hand->o.radio.encrypt == tobj->o.radio.encrypt)
								{
									if (get_obj_affect(xch->left_hand, MAGIC_SENT))
										heard = true;
									else
										duplicated = true;

									break;
								}
							}

							if (xch->equip)
							{
								for (robj = xch->equip; robj; robj = robj->next_content)
								{
									if (robj->o.radio.status != 0 && tobj != robj && bugObj != robj &&
										GET_ITEM_TYPE(robj) == ITEM_E_RADIO &&
										robj->o.radio.channel == tobj->o.radio.channel &&
										robj->o.radio.encrypt == tobj->o.radio.encrypt &&
										robj != get_equip(xch, WEAR_EAR))
									{
										if (get_obj_affect(robj, MAGIC_SENT))
											heard = true;
										else
											duplicated = true;
										break;
									}
								}
							}
						}
					}

					if (rch)
					{
						if (tobj != get_equip(rch, WEAR_EAR))
							magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);
					}
					else
						magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);

					if (heard)
					{
						if (rch)
						{
							if (tobj != get_equip(rch, WEAR_EAR))
								continue;
						}
						else
							continue;
					}

					// We static twice for bugs.
					int dist_penalty = static_distance_mod(bugObj, tobj, false);
					j = static_it(tobj, buf4, buf4, false, ( int )(dist_penalty * 1.75) );

					if (j >= 99)
						static_index = 0;
					else if (j >= 80)
						static_index = 1;
					else if (j >= 70)
						static_index = 2;
					else if (j >= 60)
						static_index = 3;
					else if (j >= 50)
						static_index = 4;
					else if (j >= 40)
						static_index = 5;
					else if (j >= 30)
						static_index = 6;
					else
						static_index = 7;

					for (tch = room->people; tch; tch = tch->next_in_room)
					{
						if (rch)
						{
							if (tobj == get_equip(rch, WEAR_EAR))
							{
								sprintf(buf5, " worn in #5your#0 ear");
								personal = true;
							}
							else if (rch == tch)
								sprintf(buf5, " carried by #5you#0");
							else
								sprintf(buf5, " carried by #5%s#0", char_short(rch));
						}
						else
							*buf5 = '\0';

						if (duplicated)
						{
							sprintf(buf6, "Echoed by other radios in the room, ");
						}
						else
							*buf6 = '\0';

						// Radios worn in the ear are personal, and thus only echo to the person wearing them.

						if (personal)
							if (tch != rch)
								continue;

						if (!IS_SET (tch->room->room_flags, OOC) && (!bugObj->o.radio.encrypt || (tobj->o.radio.encrypt == bugObj->o.radio.encrypt)) &&
							(decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks])))
						{
							sprintf (buf, "%s%s$p%s says in a %s,", buf6, statics[static_index], buf5, (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
							deciphered = true;

							/*if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
							sprintf (buf + strlen (buf), " %s,", accent_desc (ch, ch->skills[ch->speaks]));*/
						}
						else if (tobj->o.radio.encrypt != bugObj->o.radio.encrypt)
						{
							sprintf (buf, "%s%s$p%s says something,", buf6, statics[static_index], buf5);

							sprintf (buf + strlen (buf), " but the words are scrambled and distorted beyond comprehension.");
							deciphered = false;
						}
						else if (!IS_SET (tch->room->room_flags, OOC))
						{
							sprintf (buf, "%s%s$p%s says something in a %s,", buf6, statics[static_index], buf5, (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));

							sprintf (buf + strlen (buf), " but you are unable to decipher the words.");
							deciphered = false;
						}
						else
						{
							sprintf (buf, "%s%s$p%s says in a %s,", buf6, statics[static_index], buf5, (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
							deciphered = true;
						}

						*buf = toupper (*buf);
						act (buf, false, tch, tobj, 0, TO_CHAR | _ACT_FORMAT);
						if (tch->descr() && deciphered)
						{
							*buf4 = toupper (*buf4);
							sprintf (buf, "   \"%s\"\n", buf4);
							send_to_char (buf, tch);
						}
					}
				}



				// Now, we remove all those magic sents to work for next time.
				for (tobj = object_list; tobj; tobj = tobj->next)
					if (get_obj_affect(tobj, MAGIC_SENT))
						remove_obj_affect(tobj, MAGIC_SENT);
			}
		}
	}

}

void
	personalize_temote_string (CHAR_DATA * src, CHAR_DATA * tar, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL, *target = NULL;
	AFFECTED_TYPE *af = NULL;

	*output = '\0';

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				for (tch = tar->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
					{
						break;
					}
					emote++;
					emote++;
					if (*emote == '\'')
					{
						strcat (desc, "\'");
						emote++;
						if (*emote == 's')
						{
							strcat (desc, "s");
						}
						else
						{
							emote--;
						}
					}
					else
					{
						emote--;
						emote--;
					}
					if (!tch)
						continue;
					if (tch == tar)
					{
						sprintf (buf, "%c", desc[strlen (desc) - 1]);
						if (desc[strlen (desc) - 1] == '\''
							|| desc[strlen (desc) - 2] == '\'')
						{
							strcat (output, "#5your#0");
							emote--;
						}
						else
							strcat (output, "#5you#0");
						target = tch;
						emote++;
					}
					else
					{
						sprintf (buf, "#5%s#0", char_short (tch));
						strcat (output, buf);
						emote--;
						if (*emote == '\'')
							emote--;
					}
			}
			else
				sprintf (output + strlen (output), "#%c", *emote);
		}
		else
			sprintf (output + strlen (output), "%c", *emote);
		emote++;
	}
	if (target)
	{
		if (*output == '#')
			output[2] = toupper (output[2]);

		af = (get_affect(src, MAGIC_SIT_TABLE));
		if (!are_grouped(target, src) && !GET_TRUST(target) && (!af || !is_at_table (target, af->a.table.obj)))
		{
			char heard[MAX_STRING_LENGTH] = { '\0' };
			char speech[MAX_STRING_LENGTH] = { '\0' };
			int j = 0;
			for (size_t y = 0; y <= strlen (output); y++)
			{
				if (output[y]=='\"') //if this is true, we have found the first quotation mark
				{
					sprintf(speech, "\"");
					j = y + 1;
					while (output[j] !='\"') //move through the speech string until you find more quotation marks
					{
						sprintf(speech + strlen(speech), "%c", output[j]);
						j++;
					}

					sprintf(speech + strlen(speech), "\"");
					whisper_it(target, speech, speech, 0);
					sprintf(heard + strlen(heard), "%s", speech);

					y = j;
				}
				else
				{
					sprintf(heard + strlen(heard), "%c", output[y]);
				}
			}
			act (heard, false, src, 0, target, TO_VICT | _ACT_FORMAT);
		}
		else
		{
			act (output, false, src, 0, target, TO_VICT | _ACT_FORMAT);
		}
		magic_add_affect (target, MAGIC_SENT, -1, 0, 0, 0, 0);
	}
}

void
	personalize_temote (CHAR_DATA * src, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af = NULL;

	sprintf (copy, "%s", emote);

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				tch = get_char_room_vis (src, desc);
				for (tch = src->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp (char_short (tch), desc) == STR_MATCH)
						break;
				if (!tch)
					continue;
				if (!get_affect (tch, MAGIC_SENT))
					personalize_temote_string (src, tch, copy);
			}
		}
		emote++;
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == src)
			continue;
		if (get_affect (tch, MAGIC_SENT))
			continue;

		af = (get_affect(src, MAGIC_SIT_TABLE));
		if (!are_grouped(tch, src) && !GET_TRUST(tch) && (!af || !is_at_table (tch, af->a.table.obj)))
		{
			char speech[MAX_STRING_LENGTH] = { '\0' };
			char heard[MAX_STRING_LENGTH] = { '\0' };
			int j = 0;
			for (size_t y = 0; y <= strlen (copy); y++)
			{
				if (copy[y]=='\"') //if this is true, we have found the first quotation mark
				{
					sprintf(speech, "\"");
					j = y + 1;
					while (copy[j]!='\"') //move through the speech string until you find more quotation marks
					{
						sprintf(speech + strlen(speech), "%c", copy[j]);
						j++;
					}

					sprintf(speech + strlen(speech), "\"");
					whisper_it(tch, speech, speech, 0);
					sprintf(heard + strlen(heard), "%s", speech);
					y = j;
				}
				else
				{
					sprintf(heard + strlen(heard), "%c", copy[y]);
				}
			}
			act (heard, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		else
		{
			act (copy, false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (get_affect (tch, MAGIC_SENT))
			affect_remove (tch, get_affect (tch, MAGIC_SENT));
	}
}

void
	do_temote (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	char *result = NULL;
	char *p2 = '\0';
	int quotes; //counts the number of quotation marks, if uneven, notify the player instead of sending the emote.


	while (isspace (*argument))
		argument++;

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

	/*
	if (strstr (argument, "\""))
	{
	send_to_char
	("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
	ch);
	return;
	}
	*/

	//speech in emotes is now allowed - check if the number of quotation marks is even.
	if (strstr (argument, "\""))
	{
		/*
		send_to_char
		("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n",
		ch);
		return;
		*/

		p2=argument;
		while (*p2) //loop to count the number of quotation marks
		{
			if (*p2=='\"')  quotes++;
			p2++;
		}

		if ((quotes % 2) != 0) //if even, do nothing, if it's an odd number the player gets notified.
		{
			send_to_char ("You'll need to close those quotation marks.\n ", ch);
			return;
		}

	}

	if (quotes)
	{
		if (IS_NPC(ch) && !str_cmp(lookup_race_variable(ch->race, RACE_NAME), "robot"))
		{
			if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "voice")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "voice")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "voice")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "voice")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
		}
       
	}


	if (!*argument)
		send_to_char ("What would you like to emote?\n", ch);
	else
	{
		//p = copy;

		sprintf(copy, argument);

		/** Removed code and created swap_xmote_target function **/
		result = swap_xmote_target (ch, copy, 1);

		if (!result)
			return;

		sprintf (buf, "%s", result);

		personalize_temote (ch, buf); //adjusts for "you" if needed

		if (!strcmp(result, buf))
		{
			act (buf, false, ch, 0, 0, TO_ROOM | TO_CHAR | _ACT_FORMAT);
		}
		else
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		*buf = tolower(*buf);
		if (!quotes)
		{
			watched_action(ch, buf, 0, 1);
		}
		else
		{
			watched_action(ch, buf, 0, 2);
		}
	}
}



void
	reply_reset (CHAR_DATA * ch, CHAR_DATA * target, char *buf, int cmd)
{
	static int avoid_loop = 0;
	RESET_DATA *reset = NULL;
	char *argument = '\0';
	char keywords[MAX_STRING_LENGTH] = { '\0' };
	char reply[MAX_STRING_LENGTH] = { '\0' };
	char question[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	if (avoid_loop)		/* Don't get into infinite loops between mobs */
		return;

	for (reset = target->mob->resets; reset; reset = reset->next)
	{

		if (reset->type != RESET_REPLY)
			continue;

		argument = one_argument (reset->command, keywords);

		while (isspace (*argument))
			argument++;

		strcpy (reply, argument);

		one_argument (buf, question);

		for (argument = one_argument (keywords, buf2);
			*buf2; argument = one_argument (argument, buf2))
		{

			if (strcasecmp (buf2, question) == STR_MATCH)
			{
				name_to_ident (ch, buf2);
				sprintf (buf2 + strlen (buf2), " %s", reply);
				avoid_loop = 1;

				send_to_char ("\n", ch);

				if (cmd == 4)
					do_whisper (target, buf2, cmd);
				else
					do_say (target, buf2, cmd);

				avoid_loop = 0;

				return;
			}
		}
	}
}

void
	do_speak (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	char buf[MAX_INPUT_LENGTH] = { '\0' };

	struct lang_info
	{
		char lang[15];
		int skill;
	} lang_tab[] =
	{
		{"Common", SKILL_COMMON},
		{"Voodoo", SKILL_VOODOO},
		{"\0", 0}
	};

	argument = one_argument (argument, buf);

	for (i = 0; lang_tab[i].skill; i++)
		if (strcasecmp (buf, lang_tab[i].lang) == STR_MATCH)
			break;

	if (!lang_tab[i].skill)
	{
		send_to_char ("Your only choice is: Common\n", ch);
		return;
	}

	if (!real_skill (ch, lang_tab[i].skill)
		&& !get_affect (ch, MAGIC_AFFECT_TONGUES))
	{
		sprintf (buf, "You are unfamiliar with %s.\n", CAP (lang_tab[i].lang));
		send_to_char (buf, ch);
		return;
	}

	if (ch->speaks == lang_tab[i].skill)
	{
		sprintf (buf, "You are already speaking %s.\n", CAP (lang_tab[i].lang));
		send_to_char (buf, ch);
		return;
	}

	ch->speaks = lang_tab[i].skill;

	sprintf (buf, "You begin speaking %s.\n", CAP (lang_tab[i].lang));
	send_to_char (buf, ch);
}

void
	do_select_script (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	char buf[MAX_INPUT_LENGTH] = { '\0' };

	struct lang_info
	{
		char lang[30];
		int skill;
	} lang_tab[] =
	{
		{"common", SKILL_COMMON},
		{"voodoo", SKILL_VOODOO},
		{"\0", 0}
	};

	argument = one_argument (argument, buf);

	for (i = 0; lang_tab[i].skill; i++)
		if (strcasecmp (buf, lang_tab[i].lang) == STR_MATCH)
			break;

	if (!lang_tab[i].skill)
	{
		send_to_char ("Your only choice is: Common\n", ch);
		return;
	}

	if (!real_skill (ch, lang_tab[i].skill)
		&& !get_affect (ch, MAGIC_AFFECT_TONGUES))
	{
		sprintf (buf, "You are unfamiliar with %s.\n", CAP (lang_tab[i].lang));
		send_to_char (buf, ch);
		return;
	}

	if (ch->speaks == lang_tab[i].skill)
	{
		sprintf (buf, "You are already writing in %s.\n",
			CAP (lang_tab[i].lang));
		send_to_char (buf, ch);
		return;
	}

	ch->writes = lang_tab[i].skill;

	sprintf (buf, "You will now write in %s.\n", CAP (lang_tab[i].lang));
	send_to_char (buf, ch);
}

void
	do_mute (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };
	AFFECTED_TYPE *af = NULL;

	/*
	if (!real_skill (ch, SKILL_EAVESDROP))
	{
	sprintf (buf,
	"You don't have any skill to try and overhear conversations, so you are already muted by default.\n");
	send_to_char (buf, ch);
	return;
	}*/

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "You %s listening to others' conversations.\n",
			get_affect (ch, MUTE_EAVESDROP) ? "aren't" : "are");
		send_to_char (buf, ch);
		return;
	}

	if (strcasecmp (argument, "on") == STR_MATCH)
	{

		if (!get_affect (ch, MUTE_EAVESDROP))
		{
			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

			af->type = MUTE_EAVESDROP;
			af->a.listening.duration = -1;
			af->a.listening.on = 1;

			affect_to_char (ch, af);
		}
		sprintf (buf, "You will now not listen to others' conversations.\n");
		send_to_char (buf, ch);
	}
	else if (strcasecmp (argument, "off") == STR_MATCH)
	{

		if (get_affect (ch, MUTE_EAVESDROP))
		{
			remove_affect_type (ch, MUTE_EAVESDROP);
		}
		sprintf (buf, "You will now listen to others' conversations.\n");
		send_to_char (buf, ch);
	}
	else
	{
		sprintf (buf,
			"You can change your mute status by 'mute on' or 'mute off'.  To see what your mute status is use 'mute'\n");
		send_to_char (buf, ch);
	}
}

#define VOICE_RESET "normal"	/* Users use this to unset their voices */

void
	do_voice (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (strchr (argument, '~'))
	{
		send_to_char
			("Sorry, but you can't use tildae when setting a voice string.\n",
			ch);
		return;
	}

	if (!*argument)
	{
		if (ch->voice_str)
		{
			sprintf (buf, "Your current voice string: (#2%s#0)\n",
				ch->voice_str);
		}
		else
			sprintf (buf, "You do not currently have a voice string set.\n");
		send_to_char (buf, ch);
	}
	else
	{
		if (strcasecmp (argument, VOICE_RESET) == STR_MATCH)
		{
			clear_voice (ch);
			sprintf (buf, "Your voice string has been cleared.");
		}
		else
		{
			sprintf (buf, "Your voice string has been set to: (#2%s#0)",
				argument);
			ch->voice_str = add_hash (argument);
		}

		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

int
	decipher_speaking (CHAR_DATA * ch, int skillnum, int skill)
{
	int check = 0;

	if (skill > 0 && skill <= 10)
		check = 70;
	else if (skill > 10 && skill < 20)
		check = 50;
	else if (skill >= 20 && skill < 35)
		check = 30;
	else if (skill >= 35 && skill < 50)
		check = 20;
	else if (skill >= 50)
		check = 5;

	skill_use (ch, skillnum, 0);

	if (ch->skills[skillnum] >= check)
		return 1;
	else
		return 0;
}

char *
	accent_desc (CHAR_DATA * ch, int skill)
{
	if (skill < 10)
		return "with very crude enunciation";
	else if (skill >= 10 && skill < 20)
		return "with crude enunciation";
	else if (skill >= 20 && skill < 30)
		return "with awkward enunciation";
	else if (skill >= 30 && skill < 40)
		return "with slightly awkward enunciation";
	else if (skill >= 40 && skill < 50)
		return "with a very faintly awkward enunciation";
	else
		return "with a faint accent";
}



void
	do_say (CHAR_DATA * ch, char *argument, int cmd)
{
	char key[MAX_STRING_LENGTH] = { '\0' };
	int talked_to_another = 0, i = 0, key_e = 0;
	int index = 0;
	int heard_something = 0;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *target = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *radio = NULL;
	AFFECTED_TYPE *tongues = NULL;
	AFFECTED_TYPE *af_table = NULL;
	bool deciphered = false, allocd = false;
	int dist_mod = 0;
	int dist_penalty = 0;

	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };
	char buf6[MAX_STRING_LENGTH] = { '\0' };
	char target_key[MAX_STRING_LENGTH] = { '\0' };
	char voice[MAX_STRING_LENGTH] = { '\0' };
	char argbuf[MAX_STRING_LENGTH] = { '\0' };
	char *utters[] = { "say", "sing", "tell", "murmur", "wouldbewhisper", "wouldbesing", "speak"};
	bool radio_station = false;

	bool bIsWithGroup = false;

	int j = 0;
	int static_index = 0;


	if (ch->room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("You can't do that underwater!\n", ch);
		return;
	}

	if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (ch))
	{
		if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
		{
			send_to_char
				("You decide against speaking out of turn. PETITION to request to speak.\n",
				ch);
			return;
		}
	}

	/* We modify *argument, make sure we don't */
	/*  have a problem with const arguments   */
	strcpy (argbuf, argument);
	argument = argbuf;

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		send_to_char ("What would you like to say?\n", ch);
		return;
	}

	/*
	if (ch->speaks == SKILL_WHITE_WISE || ch->speaks == SKILL_GREY_WISE
	|| ch->speaks == SKILL_BLACK_WISE)
	{
	if (!is_incantation (argument))
	{
	send_to_char
	("The tongue of the Wise should not be used flippantly or without cause!\n",
	ch);
	return;
	}
	}
	*/

	if (cmd == 5) //group
	{
		cmd = 3;
		bIsWithGroup = true;
	}

	if (cmd == 2) //tell
	{
		*target_key = '\0';
		argument = one_argument (argument, target_key);
	}

	if (cmd == 6) //radio
	{
		*target_key = '\0';
		argument = one_argument (argument, target_key);
	}

	if (ch->voice_str && ch->voice_str[0])
	{
		strcpy (voice, ch->voice_str);
	}

	//Get the intro phrase and the message
	if (*argument == '(')
	{

		if (cmd == 6) //radio
		{
			send_to_char("You can't do that while radioing.\n", ch);
			return;
		}

		*voice = '\0';
		*buf = '\0';
		sprintf (buf, "%s", argument);
		i = 1;
		*buf2 = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				send_to_char ("What did you wish to say?\n", ch);
				return;
			}
			if (buf[i] == '*')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
					!get_obj_in_list_vis (ch, key, ch->right_hand) &&
					!get_obj_in_list_vis (ch, key, ch->left_hand) &&
					!get_obj_in_list_vis (ch, key, ch->equip))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				sprintf (buf2 + strlen (buf2), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}
			if (buf[i] == '~')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				sprintf (buf2 + strlen (buf2), "#5%s#0",
					char_short (get_char_room_vis (ch, key)));
				*key = '\0';
				continue;
			}
			sprintf (buf2 + strlen (buf2), "%c", buf[i]);
			i++;
		}
		strcpy (voice, buf2);
		while (*argument != ')')
			argument++;
		argument += 2;

		i = 0;
		*buf = '\0';
		if (cmd == 2 && *target_key)
			sprintf (buf, "%s %s", target_key, argument);
		else
			sprintf (buf, "%s", argument);
		*argument = '\0';
		argument = buf;
		if (!*argument)
		{
			send_to_char ("What did you wish to say?\n", ch);
			return;
		}
	}
	else if ((cmd == 2 || cmd == 6) && *target_key)
	{
		sprintf (buf, "%s %s", target_key, argument);
		sprintf (argument, "%s", buf);
	}

	if (!*argument)
	{
		send_to_char ("What did you wish to say?\n", ch);
		return;
	}

	if (cmd == 2)
	{
		/* Tell */

		argument = one_argument (argument, buf);

		if (!*argument)
		{
			send_to_char ("What did you wish to tell?\n", ch);
			return;
		}

		reformat_say_string (argument, &argument, 0);

		if (!(target = get_char_room_vis (ch, buf)))
		{
			send_to_char ("Tell who?\n", ch);
			return;
		}

		if (target == ch)
		{
			send_to_char ("You want to tell yourself?\n", ch);
			return;
		}

		while (isspace (*argument))
			argument++;
	}
	else if (cmd == 6)
	{
		/* Radio */

		argument = one_argument (argument, buf);

		if (!*argument)
		{
			send_to_char ("What message did you wish to radio?\n", ch);
			return;
		}

		reformat_say_string (argument, &argument, 0);

		if (!(radio = get_obj_in_dark (ch, buf, ch->right_hand)) &&
			!(radio = get_obj_in_dark (ch, buf, ch->left_hand)) &&
			!(radio = get_equip_arg (ch, WEAR_EAR, buf)) &&
			!(radio = get_equip_arg (ch, WEAR_WRIST_L, buf)) &&
			!(radio = get_equip_arg (ch, WEAR_WRIST_R, buf)) &&
			!(radio = get_obj_in_list_vis (ch, buf, ch->room->contents)))
		{
			send_to_char ("Radio with what?\n", ch);
			return;
		}

		if (GET_ITEM_TYPE(radio) != ITEM_E_RADIO)
		{
			sprintf(buf, "$p isn't something you can use as a radio.\n");
			act(buf, false, ch, radio, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (radio->o.elecs.status == 0)
		{
			act("$p is not switched on.", false, ch, radio, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (!IS_SET(radio->o.elecs.elec_bits, ELEC_FEA_RADIO_SET) && radio->o.radio.channel >= 1 && radio->o.radio.channel <= 100)
		{
			act("$p is set to a listen-only channel: it is unable to broadcast over this frequency.", false, ch, radio, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if (IS_SET(radio->o.elecs.elec_bits, ELEC_FEA_RADIO_ONEWAY))
		{
			act("$p is not capable of making transmissions - only receiving them.", false, ch, radio, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		while (isspace (*argument))
			argument++;
	}
	else
		reformat_say_string (argument, &argument, 0);

	if (!*argument)
	{
		if (cmd == 1)
			send_to_char ("What are the words to the song?\n", ch);
		else if (cmd == 2)
			send_to_char ("What would you like to tell?\n", ch);
		else if (cmd == 6)
			send_to_char ("What would you like to radio?\n", ch);
		else
			send_to_char ("What would you like to say?\n", ch);
		return;
	}

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);

	if (cmd == 3)
	{
		if ((af_table = get_affect (ch, MAGIC_SIT_TABLE)) != NULL)
		{
			bIsWithGroup = false;
		}
	}
	if (!tongues && !real_skill (ch, ch->speaks))
	{
		send_to_char
			("You can't even make a guess at the language you want to speak.\n",
			ch);
		return;
	}


	std::string new_string (argument);

	while (new_string.find("\\n") != std::string::npos)
	{
		new_string.replace(new_string.find("\\n"), 2, "\n");
	}

	while (new_string.find("\\t") != std::string::npos)
	{
		new_string.replace(new_string.find("\\t"), 2, "\t");
	}

	sprintf (argument, new_string.c_str());

	sprintf (buf4, argument);	/* The intended message, sent to the player. */
	sprintf (buf5, argument);
	sprintf (buf2, argument);

	if (cmd == 0)
	{
		if (buf4[strlen (buf4) - 1] == '?')
		{
			utters[cmd] = str_dup ("ask");
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '!')
		{
			utters[cmd] = str_dup ("exclaim");
			dist_mod = 1;
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '.' && buf4[strlen (buf4) - 2] == '.' && buf4[strlen (buf4) - 3] == '.')
		{
			utters[cmd] = str_dup ("murmur");
			dist_mod = -1;
			allocd = true;
		}
	}
	else if (cmd == 2)
	{
		if (buf4[strlen (buf4) - 1] == '?')
		{
			utters[cmd] = str_dup ("ask");
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '!')
		{
			utters[cmd] = str_dup ("emphatically tell");
			allocd = true;
			dist_mod = -1;
		}
		else if (buf4[strlen (buf4) - 1] == '.' && buf4[strlen (buf4) - 2] == '.' && buf4[strlen (buf4) - 3] == '.')
		{
			utters[cmd] = str_dup ("mumbles to");
			dist_mod = -1;
			allocd = true;
		}
	}
	else if (cmd == 6)
	{
		if (buf4[strlen (buf4) - 1] == '?')
		{
			utters[cmd] = str_dup ("ask");
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '!')
		{
			utters[cmd] = str_dup ("shout");
			dist_mod = 1;
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '.' && buf4[strlen (buf4) - 2] == '.' && buf4[strlen (buf4) - 3] == '.')
		{
			utters[cmd] = str_dup ("murmur");
			dist_mod = -1;
			allocd = true;
		}
	}

	skill_use (ch, ch->speaks, 0);

	deciphered = false;

	/*

	int dir;
	CHAR_DATA *xch;
	AFFECTED_TYPE *waf;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *next_room;

	if (vtor(ch->in_room))
	{
	for (dir = 0; dir <= LAST_DIR; dir++)
	{
	if (!(exit = EXIT (ch, dir)) || !(next_room = vtor (exit->to_room))
	|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
	continue;

	for (xch = next_room->people; xch; xch = xch->next_in_room)
	{
	if ((waf = get_affect (xch, AFFECT_LISTEN_DIR)) && vtor(xch->in_room))
	{
	if (rev_dir[dir] == waf->a.shadow.edge)
	{
	magic_add_affect (xch, MAGIC_HEARD, -1, xch->in_room, 0, 0, 0);
	char_from_room(xch);
	char_to_room(xch, ch->in_room);
	}
	}
	}
	}
	}
	*/

	// Now decided what the target person hears
	for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch)		/* Don't say it to ourselves */
			continue;

		if (!tch->descr())	/* NPC don't hear anything */
			continue;

		if ((af_table && !is_at_table (tch, af_table->a.table.obj))
			|| (bIsWithGroup
			&& (!are_grouped (ch, tch)
			|| get_affect (tch, MAGIC_SIT_TABLE))))
		{

			/* If the guy is muting, punt */
			if (get_affect (tch, MUTE_EAVESDROP))
				continue;

			sprintf (buf2, argument);

			heard_something = whisper_it (tch, buf2, buf2, 0);

			//if(!heard_something)
			//  continue;

			if ((tch->skills[ch->speaks]
			&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) && heard_something)
				|| IS_SET (ch->room->room_flags, OOC))
			{
				/*

				Until we get other languages, everyone speaks in Common.

				if (!IS_SET (ch->room->room_flags, OOC))
				{
				sprintf (buf, "You overhear $N say in %s,",
				skills[ch->speaks]);
				if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));
				}
				else
				*/
				sprintf (buf, "You overhear $N say,");

				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				deciphered = true;
			}
			else
			{

				/*

				Until we get other languages, everyone speaks in Common.


				if (tch->skills[ch->speaks])
				sprintf (buf, "You overhear $N say something in %s,",
				skills[ch->speaks]);
				else
				*/

				sprintf (buf, "You overhear $N say something,");

				/*
				if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));
				*/
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);


				if (heard_something)
					sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.", HSHR (ch));
				else
					sprintf (buf + strlen (buf),
					" but you are unable to make out %s words.", HSHR (ch));

				deciphered = false;
			}
			act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);

			if (tch->descr() && deciphered)
			{
				*buf2 = toupper (*buf2);
				sprintf (buf, "   \"%s\"\n", buf2);
				send_to_char (buf, tch);
			}

			continue;
		}

		if (tch->descr())
			talked_to_another = 1;

		if (GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME))
			sprintf (buf3, " (%s)", GET_NAME (ch));
		else
			*buf3 = '\0';

		if (cmd == 0 || cmd == 1)
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf, "$N%s %ss,", buf3, utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ? skills[ch->speaks] : "an unknown tongue"*/);
				/*if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));
				*/
				deciphered = true;
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss something", buf3, utters[cmd]/*,(tch->skills[ch->speaks] || tongues) ? skills[ch->speaks] : "an unknown tongue"*/);
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.",
					HSHR (ch));
				deciphered = false;
			}
			else if (IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss,", buf3, utters[cmd]);
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				deciphered = true;
			}
		}
		else if (cmd == 2)
		{
			if (tch == target)
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks,
					ch->skills[ch->speaks]))
				{
					sprintf (buf, "$N%s %ss you,", buf3,
						utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
								   skills[ch->speaks] : "an unknown tongue"*/);
					deciphered = true;
					/*if (tch->skills[ch->speaks] >= 50
					&& ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));*/
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "$N%s %ss you something,", buf3,
						utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
								   skills[ch->speaks] : "an unknown tongue"*/);
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					sprintf (buf + strlen (buf),
						" but you are unable to decipher %s words.",
						HSHR (ch));
					deciphered = false;
				}
				else
				{
					sprintf (buf, "$N%s %ss you,", buf3, utters[cmd]);
					deciphered = true;
				}
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks,
					ch->skills[ch->speaks]))
				{
					sprintf (buf, "$N%s %ss %s,", buf3,
						utters[cmd], char_short (target)/*,
														(tch->skills[ch->speaks]
														|| tongues) ? skills[ch->
														speaks] :
														"an unknown tongue"*/);
					deciphered = true;
					/*if (tch->skills[ch->speaks] >= 50
					&& ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));*/
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "$N%s %ss %s something", buf3,
						utters[cmd], char_short (target)/*,
														(tch->skills[ch->speaks] || tongues) ?
														skills[ch->speaks] : "an unknown tongue"*/);
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					sprintf (buf + strlen (buf),
						" but you are unable to decipher %s words.",
						HSHR (ch));
					deciphered = false;
				}
				else
				{
					sprintf (buf, "$N%s %ss %s,", buf3, utters[cmd],
						char_short (target));
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					deciphered = true;
				}
			}
		}
		else if (cmd == 6)
		{
			if (tch->room == ch->room)
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks,
					ch->skills[ch->speaks]))
				{
					sprintf (buf, "$N%s %ss in to $p,", buf3,
						utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
								   skills[ch->speaks] : "an unknown tongue"*/);
					deciphered = true;
					/*if (tch->skills[ch->speaks] >= 50
					&& ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));*/
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "$N%s %ss something in to $p,", buf3,
						utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
								   skills[ch->speaks] : "an unknown tongue"*/);
					if (*voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					sprintf (buf + strlen (buf),
						" but you are unable to decipher %s words.",
						HSHR (ch));
					deciphered = false;
				}
				else
				{
					sprintf (buf, "$N%s %ss in to $p,", buf3, utters[cmd]);
					deciphered = true;
				}
			}
		}
		else if (cmd == 3)
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf, "$N%s %ss,", buf3,
					utters[cmd]/*,
							   (tch->skills[ch->speaks] || tongues) ?
							   skills[ch->speaks] : "an unknown tongue"*/);
				deciphered = true;
				/*if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));*/
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss something,", buf3,
					utters[cmd]/*,
							   (tch->skills[ch->speaks] || tongues) ?
							   skills[ch->speaks] : "an unknown tongue"*/);
				if (*voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.",
					HSHR (ch));
				deciphered = false;
			}
			else
			{
				sprintf (buf, "$N%s %ss,", buf3, utters[cmd]);
				deciphered = true;
			}
		}

		act (buf, false, tch, radio, ch, TO_CHAR | _ACT_FORMAT);

		if (tch->descr() && deciphered)
		{
			*buf4 = toupper (*buf4);
			sprintf (buf, "   \"%s\"\n", buf4);
			send_to_char (buf, tch);
		}

		sprintf (argument, buf5);
		deciphered = false;
	}

	/* Now we let everyone standing nearby interpret the words for themselves. */
	// Now decided what the target person hears

	int dir;
	AFFECTED_TYPE *waf;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *next_room;

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room))
			|| IS_SET (exit->exit_info, EX_CLOSED) || GET_FLAG (ch, FLAG_WIZINVIS))
		{
			continue;
		}

		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if ((waf = get_affect (tch, AFFECT_LISTEN_DIR)))
			{
				if (rev_dir[dir] == waf->a.shadow.edge/* && could_see(tch, ch)*/)
				{
					if (!tch->descr())	/* NPC don't hear anything */
						continue;

					if (tch->descr())
						talked_to_another = 1;

					if (GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME))
						sprintf (buf3, " (%s)", GET_NAME (ch));
					else
						*buf3 = '\0';

					sprintf (buf2, argument);

					heard_something = whisper_it (tch, buf2, buf2, dist_mod * 15);

					if (cmd == 0 || cmd == 1)
					{
						if (!IS_SET (ch->room->room_flags, OOC) && heard_something
							&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
						{
							sprintf (buf, "To the %s, you overhear $N%s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ? skills[ch->speaks] : "an unknown tongue"*/);
							/*if (tch->skills[ch->speaks] >= 50
							&& ch->skills[ch->speaks] < 50)
							sprintf (buf + strlen (buf), " %s,",
							accent_desc (ch, ch->skills[ch->speaks]));
							*/
							deciphered = true;
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !heard_something)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]/*,(tch->skills[ch->speaks] || tongues) ? skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to make out %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !deciphered)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]/*,(tch->skills[ch->speaks] || tongues) ? skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to decipher %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else if (IS_SET (ch->room->room_flags, OOC))
						{
							sprintf (buf, "To the %s, you overhear $N%s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							deciphered = true;
						}
					}
					else if (cmd == 2)
					{
						if (!IS_SET (ch->room->room_flags, OOC) && heard_something
							&& decipher_speaking (tch, ch->speaks,
							ch->skills[ch->speaks]))
						{
							sprintf (buf, "To the %s, you overhear $N%s %s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd], char_short (target)/*,
																(tch->skills[ch->speaks]
																|| tongues) ? skills[ch->
																speaks] :
																"an unknown tongue"*/);
							deciphered = true;
							/*if (tch->skills[ch->speaks] >= 50
							&& ch->skills[ch->speaks] < 50)
							sprintf (buf + strlen (buf), " %s,",
							accent_desc (ch, ch->skills[ch->speaks]));*/
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !heard_something)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s %s something", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd], char_short (target)/*,
																(tch->skills[ch->speaks] || tongues) ?
																skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to make out %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !deciphered)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s %s something", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd], char_short (target)/*,
																(tch->skills[ch->speaks] || tongues) ?
																skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to decipher %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else
						{
							sprintf (buf, "To the %s, you overhear $N%s %s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd],
								char_short (target));
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							deciphered = true;
						}
					}
					else if (cmd == 6)
					{
						if (!IS_SET (ch->room->room_flags, OOC) && heard_something
							&& decipher_speaking (tch, ch->speaks,
							ch->skills[ch->speaks]))
						{
							sprintf (buf, "To the %s, you overhear $N%s %s in to $p,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							deciphered = true;
							/*if (tch->skills[ch->speaks] >= 50
							&& ch->skills[ch->speaks] < 50)
							sprintf (buf + strlen (buf), " %s,",
							accent_desc (ch, ch->skills[ch->speaks]));*/
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !heard_something)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something in to $p,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to make out %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !deciphered)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something in to $p,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*, (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to decipher %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else
						{
							sprintf (buf, "To the %s, you overhear $N%s %s in to $p,", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]);
							deciphered = true;
						}
					}
					else if (cmd == 3)
					{
						// Hard to hear someone murmuring.
						if (!skill_use(ch, SKILL_EAVESDROP, 0))
						{
							heard_something = false;
						}
						else
						{
							heard_something = whisper_it (tch, buf2, buf2, -15);
						}

						if (!IS_SET (ch->room->room_flags, OOC) && heard_something
							&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
						{
							sprintf (buf, "To the %s, you overhear $N%s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*,
										   (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							deciphered = true;
							/*if (tch->skills[ch->speaks] >= 50
							&& ch->skills[ch->speaks] < 50)
							sprintf (buf + strlen (buf), " %s,",
							accent_desc (ch, ch->skills[ch->speaks]));*/
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !heard_something)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*,
										   (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to make out %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else if (!IS_SET (ch->room->room_flags, OOC) && !deciphered)
						{
							sprintf (buf, "To the %s, you overhear $N%s %s something,", direction[rev_dir[waf->a.shadow.edge]], buf3,
								utters[cmd]/*,
										   (tch->skills[ch->speaks] || tongues) ?
										   skills[ch->speaks] : "an unknown tongue"*/);
							if (*voice)
								sprintf (buf + strlen (buf), " %s,", voice);
							sprintf (buf + strlen (buf),
								" but you are unable to decipher %s words.",
								HSHR (ch));
							deciphered = false;
						}
						else
						{
							sprintf (buf, "To the %s, you overhear $N%s %s,", direction[rev_dir[waf->a.shadow.edge]], buf3, utters[cmd]);
							deciphered = true;
						}
					}

					act (buf, false, tch, radio, ch, TO_CHAR | _ACT_FORMAT);

					if (tch->descr() && deciphered)
					{
						*buf2 = toupper (*buf2);
						sprintf (buf, "   \"%s\"\n", buf2);
						send_to_char (buf, tch);
					}

					sprintf (argument, buf5);
					deciphered = false;
				}
			}
		}
	}


	// We have to transport people who eavesdropped after the function,
	// otherwise tch->next_in_room references people it shouldn't.
	/*

	xch = NULL;
	for (tch = vtor(ch->in_room)->people; tch;)
	{
	xch = tch->next_in_room;
	if (get_affect(tch, MAGIC_HEARD))
	{
	char_from_room(tch);
	char_to_room(tch, get_affect(tch, MAGIC_HEARD)->a.affect.uu2);
	remove_affect_type(tch, MAGIC_HEARD);
	if (xch)
	tch = xch->next_in_room->next_in_room;
	}
	else
	tch = xch;
	}
	*/

	*buf4 = toupper (*buf4);

	if (cmd == 2)
	{
		if (*voice)
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s #5%s#0, %s,\n   \"%s\"\n",
				utters[cmd], char_short (target), //skills[ch->speaks],
				voice, buf4);
			else
				sprintf (buf, "You %s #5%s#0, %s,\n   \"%s\"\n", utters[cmd],
				char_short (target), voice, buf4);
		}
		else
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s #5%s#0,\n   \"%s\"\n",
				utters[cmd], char_short (target),// skills[ch->speaks],
				buf4);
			else
				sprintf (buf, "You %s #5%s#0,\n   \"%s\"\n", utters[cmd],
				char_short (target), buf4);
		}
	}
	else if (cmd == 6)
	{
		if (*voice)
		{
			send_monitor_radio(ch, buf4, radio->o.radio.channel);	// Send this to listening staff - Shade

			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s in to #2%s#0, %s,\n   \"%s\"\n",
				utters[cmd], obj_short_desc (radio), //skills[ch->speaks],
				voice, buf4);
			else
				sprintf (buf, "You %s in to #2%s#0, %s,\n   \"%s\"\n", utters[cmd],
				obj_short_desc (radio), voice, buf4);
		}
		else
		{

			send_monitor_radio(ch, buf4, radio->o.radio.channel);	// Send this to listening staff - Shade
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s in to #2%s#0,\n   \"%s\"\n",
				utters[cmd], obj_short_desc (radio),// skills[ch->speaks],
				buf4);
			else
				sprintf (buf, "You %s in to #2%s#0,\n   \"%s\"\n", utters[cmd],
				obj_short_desc (radio), buf4);
		}

		if (radio->o.radio.channel <= 100)
		{
			radio_station = true;
			frequency[radio->o.radio.channel/10] = 45;
		}

		frequency[radio->o.radio.channel] = 45;

	}
	else
	{
		if (*voice)
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s, %s,\n   \"%s\"\n",
				utters[cmd], //skills[ch->speaks],
				voice, buf4);
			else
				sprintf (buf, "You %s, %s,\n   \"%s\"\n", utters[cmd], voice,
				buf4);
		}
		else
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s,\n   \"%s\"\n", utters[cmd],// skills[ch->speaks],
				buf4);
			else
				sprintf (buf, "You %s,\n   \"%s\"\n", utters[cmd], buf4);
		}
	}

	send_to_char (buf, ch);

	OBJ_DATA *bugObj = NULL;

	for (bugObj = ch->room->contents; bugObj; bugObj = bugObj->next_content)
	{
		if (GET_ITEM_TYPE(bugObj) == ITEM_E_BUG && bugObj->o.elecs.status)
		{
			OBJ_DATA *tobj = NULL;
			OBJ_DATA *robj = NULL;
			CHAR_DATA *rch = NULL;
			CHAR_DATA *xch = NULL;
			ROOM_DATA *room = NULL;
			for (tobj = object_list; tobj; tobj = tobj->next)
			{
				bool heard = false;
				bool duplicated = false;
				bool personal = false;
				j = 0;
				room = NULL;
				tch = NULL;
				rch = NULL;
				xch = NULL;
				robj = NULL;

				if (tobj->deleted)
					continue;

				if (tobj == bugObj)
					continue;

				if (GET_ITEM_TYPE(tobj) != ITEM_E_RADIO)
					continue;

				// If the radio isn't on, then we can't do anything about it!
				if (tobj->o.radio.status == 0)
					continue;

				// We need to be tuned to the same channel!
				if (tobj->o.radio.channel != bugObj->o.radio.channel &&
					tobj->o.radio.channel/10 != bugObj->o.radio.channel)
					continue;

				if (tobj->in_room > 0)
					room = vnum_to_room(tobj->in_room);
				else if (tobj->equiped_by)
				{
					room = vnum_to_room(tobj->equiped_by->in_room);
					rch = tobj->equiped_by;
				}
				else if (tobj->carried_by)
				{
					room = vnum_to_room(tobj->carried_by->in_room);
					rch = tobj->carried_by;
				}
				else
					continue;

				// To avoid spam:
				// We need to run through -every- object that's visible from the room,
				// which includes everything on people as well, sadly enough. If the
				// radios have a "sent" signal on them, then it means we've already
				// echoed that message for the room, and we just make a note of that
				// instead of redoing them.
				// Not very efficient at all, but given we're already searching every
				// object in the game twice each time we radio, it should be OK.

				for (robj = room->contents; robj; robj = robj->next_content)
				{
					if (robj->o.radio.status != 0 && tobj != robj && robj != bugObj &&
						robj->o.radio.channel == tobj->o.radio.channel &&
						robj->o.radio.encrypt == tobj->o.radio.encrypt)
					{
						if (get_obj_affect(robj, MAGIC_SENT))
							heard = true;
						else
							duplicated = true;

						break;
					}
				}

				if (!heard && !duplicated)
				{
					for (xch = room->people; xch; xch = xch->next_in_room)
					{
						if (xch->right_hand && GET_ITEM_TYPE(xch->right_hand) == ITEM_E_RADIO)
						{
							if (xch->right_hand->o.radio.status != 0 && tobj != xch->right_hand && bugObj != xch->right_hand &&
								xch->right_hand->o.radio.channel == tobj->o.radio.channel &&
								xch->right_hand->o.radio.encrypt == tobj->o.radio.encrypt)
							{
								if (get_obj_affect(xch->right_hand, MAGIC_SENT))
									heard = true;
								else
									duplicated = true;

								break;
							}
						}
						if (xch->left_hand && GET_ITEM_TYPE(xch->left_hand) == ITEM_E_RADIO)
						{
							if (xch->left_hand->o.radio.status != 0 && tobj != xch->left_hand && bugObj != xch->left_hand &&
								xch->left_hand->o.radio.channel == tobj->o.radio.channel &&
								xch->left_hand->o.radio.encrypt == tobj->o.radio.encrypt)
							{
								if (get_obj_affect(xch->left_hand, MAGIC_SENT))
									heard = true;
								else
									duplicated = true;

								break;
							}
						}

						if (xch->equip)
						{
							for (robj = xch->equip; robj; robj = robj->next_content)
							{
								if (robj->o.radio.status != 0 && tobj != robj && bugObj != robj &&
									GET_ITEM_TYPE(robj) == ITEM_E_RADIO &&
									robj->o.radio.channel == tobj->o.radio.channel &&
									robj->o.radio.encrypt == tobj->o.radio.encrypt &&
									robj != get_equip(xch, WEAR_EAR))
								{
									if (get_obj_affect(robj, MAGIC_SENT))
										heard = true;
									else
										duplicated = true;
									break;
								}
							}
						}
					}
				}

				if (rch)
				{
					if (tobj != get_equip(rch, WEAR_EAR))
						magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);
				}
				else
					magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);

				if (heard)
				{
					if (rch)
					{
						if (tobj != get_equip(rch, WEAR_EAR))
							continue;
					}
					else
						continue;
				}

				// We static twice for bugs.
				int dist_penalty = static_distance_mod(radio, tobj, false);
				j = static_it(tobj, buf4, buf4, radio_station, (int)(dist_penalty * 1.75));


				if (radio_station)
					static_index = 8;
				else if (j >= 99)
					static_index = 0;
				else if (j >= 80)
					static_index = 1;
				else if (j >= 70)
					static_index = 2;
				else if (j >= 60)
					static_index = 3;
				else if (j >= 50)
					static_index = 4;
				else if (j >= 40)
					static_index = 5;
				else if (j >= 30)
					static_index = 6;
				else
					static_index = 7;

				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					if (rch)
					{
						if (tobj == get_equip(rch, WEAR_EAR))
						{
							sprintf(buf5, " worn in #5your#0 ear");
							personal = true;
						}
						else if (rch == tch)
							sprintf(buf5, " carried by #5you#0");
						else
							sprintf(buf5, " carried by #5%s#0", char_short(rch));
					}
					else
						*buf5 = '\0';

					if (duplicated)
					{
						sprintf(buf6, "Echoed by other radios in the room, ");
					}
					else
						*buf6 = '\0';

					// Radios worn in the ear are personal, and thus only echo to the person wearing them.

					if (personal)
						if (tch != rch)
							continue;

					if (!IS_SET (tch->room->room_flags, OOC) && (!bugObj->o.radio.encrypt || (tobj->o.radio.encrypt == bugObj->o.radio.encrypt)) &&
						(decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks])))
					{
						if (*voice)
							sprintf (buf, "%s%s$p%s %ss, %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
						else
							sprintf (buf, "%s%s$p%s %ss in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
						deciphered = true;

						/*if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,", accent_desc (ch, ch->skills[ch->speaks]));*/
					}
					else if (tobj->o.radio.encrypt != bugObj->o.radio.encrypt)
					{
						sprintf (buf, "%s%s$p%s says something,", buf6, statics[static_index], buf5);

						sprintf (buf + strlen (buf), " but the words are scrambled and distorted beyond comprehension.");
						deciphered = false;
					}
					else if (!IS_SET (tch->room->room_flags, OOC))
					{
						if (*voice)
							sprintf (buf, "%s%s$p%s %ss something %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
						else
							sprintf (buf, "%s%s$p%s %ss something in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));

						sprintf (buf + strlen (buf), " but you are unable to decipher the words.");
						deciphered = false;
					}
					else
					{
						if (*voice)
							sprintf (buf, "%s%s$p%s %ss, %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
						else
							sprintf (buf, "%s%s$p%s %ss in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
						deciphered = true;
					}

					*buf = toupper (*buf);
					act (buf, false, tch, tobj, 0, TO_CHAR | _ACT_FORMAT);
					if (tch->descr() && deciphered)
					{
						*buf4 = toupper (*buf4);
						sprintf (buf, "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}
			}

			// Now, we remove all those magic sents to work for next time.
			for (tobj = object_list; tobj; tobj = tobj->next)
				if (get_obj_affect(tobj, MAGIC_SENT))
					remove_obj_affect(tobj, MAGIC_SENT);
		}
	}


	if (cmd == 6)
	{
		OBJ_DATA *tobj = NULL;
		OBJ_DATA *robj = NULL;
		CHAR_DATA *rch = NULL;
		CHAR_DATA *xch = NULL;
		ROOM_DATA *room = NULL;
		for (tobj = object_list; tobj; tobj = tobj->next)
		{
			bool heard = false;
			bool duplicated = false;
			bool personal = false;
			j = 0;
			room = NULL;
			tch = NULL;
			rch = NULL;
			xch = NULL;
			robj = NULL;

			if (tobj->deleted)
				continue;

			if (tobj == radio)
				continue;

			if (GET_ITEM_TYPE(tobj) != ITEM_E_RADIO)
				continue;

			// If the radio isn't on, then we can't do anything about it!
			if (tobj->o.radio.status == 0)
				continue;

			// We need to be tuned to the same channel!
			if (tobj->o.radio.channel != radio->o.radio.channel &&
				tobj->o.radio.channel/10 != radio->o.radio.channel)
				continue;

			if (tobj->in_room > 0)
				room = vnum_to_room(tobj->in_room);
			else if (tobj->equiped_by)
			{
				room = vnum_to_room(tobj->equiped_by->in_room);
				rch = tobj->equiped_by;
			}
			else if (tobj->carried_by)
			{
				room = vnum_to_room(tobj->carried_by->in_room);
				rch = tobj->carried_by;
			}
			else
				continue;

			// To avoid spam:
			// We need to run through -every- object that's visible from the room,
			// which includes everything on people as well, sadly enough. If the
			// radios have a "sent" signal on them, then it means we've already
			// echoed that message for the room, and we just make a note of that
			// instead of redoing them.
			// Not very efficient at all, but given we're already searching every
			// object in the game twice each time we radio, it should be OK.

			for (robj = room->contents; robj; robj = robj->next_content)
			{
				if (robj->o.radio.status != 0 && tobj != robj && robj != radio &&
					robj->o.radio.channel == tobj->o.radio.channel &&
					robj->o.radio.encrypt == tobj->o.radio.encrypt)
				{
					if (get_obj_affect(robj, MAGIC_SENT))
						heard = true;
					else
						duplicated = true;

					break;
				}
			}

			if (!heard && !duplicated)
			{
				for (xch = room->people; xch; xch = xch->next_in_room)
				{
					if (xch->right_hand && GET_ITEM_TYPE(xch->right_hand) == ITEM_E_RADIO)
					{
						if (xch->right_hand->o.radio.status != 0 && tobj != xch->right_hand && radio != xch->right_hand &&
							xch->right_hand->o.radio.channel == tobj->o.radio.channel &&
							xch->right_hand->o.radio.encrypt == tobj->o.radio.encrypt)
						{
							if (get_obj_affect(xch->right_hand, MAGIC_SENT))
								heard = true;
							else
								duplicated = true;

							break;
						}
					}
					if (xch->left_hand && GET_ITEM_TYPE(xch->left_hand) == ITEM_E_RADIO)
					{
						if (xch->left_hand->o.radio.status != 0 && tobj != xch->left_hand && radio != xch->left_hand &&
							xch->left_hand->o.radio.channel == tobj->o.radio.channel &&
							xch->left_hand->o.radio.encrypt == tobj->o.radio.encrypt)
						{
							if (get_obj_affect(xch->left_hand, MAGIC_SENT))
								heard = true;
							else
								duplicated = true;

							break;
						}
					}

					if (xch->equip)
					{
						for (robj = xch->equip; robj; robj = robj->next_content)
						{
							if (robj->o.radio.status != 0 && tobj != robj && radio != robj &&
								GET_ITEM_TYPE(robj) == ITEM_E_RADIO &&
								robj->o.radio.channel == tobj->o.radio.channel &&
								robj->o.radio.encrypt == tobj->o.radio.encrypt &&
								robj != get_equip(xch, WEAR_EAR))
							{
								if (get_obj_affect(robj, MAGIC_SENT))
									heard = true;
								else
									duplicated = true;
								break;
							}
						}
					}
				}
			}

			if (rch)
			{
				if (tobj != get_equip(rch, WEAR_EAR))
					magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);
			}
			else
				magic_add_obj_affect(tobj, MAGIC_SENT, -1, 0, 0, 0, 0);

			if (heard)
			{
				if (rch)
				{
					if (tobj != get_equip(rch, WEAR_EAR))
						continue;
				}
				else
					continue;
			}


			 char bufScrambled[MAX_STRING_LENGTH] = { '\0' };

             	j = static_it(tobj, buf4, bufScrambled, radio_station, dist_penalty);

			if (radio_station)
				static_index = 8;
			else if (j >= 99)
				static_index = 0;
			else if (j >= 80)
				static_index = 1;
			else if (j >= 70)
				static_index = 2;
			else if (j >= 60)
				static_index = 3;
			else if (j >= 50)
				static_index = 4;
			else if (j >= 40)
				static_index = 5;
			else if (j >= 30)
				static_index = 6;
			else
				static_index = 7;

			for (tch = room->people; tch; tch = tch->next_in_room)
			{
				if (rch)
				{
					if (tobj == get_equip(rch, WEAR_EAR))
					{
						sprintf(buf5, " worn in #5your#0 ear");
						personal = true;
					}
					else if (rch == tch)
						sprintf(buf5, " carried by #5you#0");
					else
						sprintf(buf5, " carried by #5%s#0", char_short(rch));
				}
				else
					*buf5 = '\0';

				if (duplicated)
				{
					sprintf(buf6, "Echoed by other radios in the room, ");
				}
				else
					*buf6 = '\0';

				// Radios worn in the ear are personal, and thus only echo to the person wearing them.

				if (personal)
					if (tch != rch)
						continue;

				if (!IS_SET (tch->room->room_flags, OOC) && (!radio->o.radio.encrypt || (tobj->o.radio.encrypt == radio->o.radio.encrypt)) &&
					(decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks])))
				{
					if (*voice)
						sprintf (buf, "%s%s$p%s %ss, %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
					else
						sprintf (buf, "%s%s$p%s %ss in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
					deciphered = true;

					/*if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,", accent_desc (ch, ch->skills[ch->speaks]));*/
				}
				else if (tobj->o.radio.encrypt != radio->o.radio.encrypt)
				{
					sprintf (buf, "%s%s$p%s says something,", buf6, statics[static_index], buf5);

					sprintf (buf + strlen (buf), " but the words are scrambled and distorted beyond comprehension.");
					deciphered = false;
				}
				else if (!IS_SET (tch->room->room_flags, OOC))
				{
					if (*voice)
						sprintf (buf, "%s%s$p%s %ss something %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
					else
						sprintf (buf, "%s%s$p%s %ss something in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));

					sprintf (buf + strlen (buf), " but you are unable to decipher the words.");
					deciphered = false;
				}
				else
				{
					if (*voice)
						sprintf (buf, "%s%s$p%s %ss, %s,", buf6, statics[static_index], buf5, utters[cmd], voice);
					else
						sprintf (buf, "%s%s$p%s %ss in a %s,", buf6, statics[static_index], buf5, utters[cmd], (GET_SEX (ch) == SEX_MALE ? "male voice" : "female voice"));
					deciphered = true;
				}

				*buf = toupper (*buf);
				act (buf, false, tch, tobj, 0, TO_CHAR | _ACT_FORMAT);
				if (tch->descr() && deciphered)
				{
					*bufScrambled = toupper (*bufScrambled);
					sprintf (buf, "   \"%s\"\n", bufScrambled);
					send_to_char (buf, tch);
				}
			}
		}

		// Now, we remove all those magic sents to work for next time.
		for (tobj = object_list; tobj; tobj = tobj->next)
			if (get_obj_affect(tobj, MAGIC_SENT))
				remove_obj_affect(tobj, MAGIC_SENT);
	}

	/** For Theatre and Audience room only **/
	if (ch->in_room == StageRoom)
	{
		for (index = 0; index < 4; index = index +1)
		{
			for (tch = vnum_to_room (AudienceRoom[index])->people; tch; tch = tch->next_in_room)
			{
				if (!tch->descr())	/* NPC don't hear anything */
					continue;

				if (tch->descr())
					talked_to_another = 1;

				if (GET_TRUST (tch) &&
					!IS_NPC (tch) &&
					GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				if (cmd == 0 || cmd == 1 || cmd == 2 || cmd == 3)
				{
					if (!IS_SET (ch->room->room_flags, OOC) &&
						decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
					{
						sprintf (buf,
							"On the stage, $N%s %ss,",
							buf3,
							utters[cmd]);

						if (tch->skills[ch->speaks] >= 50 &&
							ch->skills[ch->speaks] < 50)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								accent_desc (ch, ch->skills[ch->speaks]));
						}
						deciphered = true;
						if (*voice)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								voice);
						}
					}

					else if (!IS_SET (ch->room->room_flags, OOC))
					{
						sprintf (buf,
							"On the stage, $N%s %ss something,",
							buf3,
							utters[cmd]);

						if (*voice)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								voice);
						}

						sprintf (buf + strlen (buf),
							" but you are unable to decipher %s words.",
							HSHR (ch));
						deciphered = false;
					}
				}

				else
					continue;

				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				sprintf (buf3, "   \"%s\"\n", buf4);
				send_to_char(buf3, tch);
			} //for (tch = vtor (AudienceRoom[index])->people
		}//for (index = 1; index; index = index +1)
	}//if (ch->in_room = StageRoom)
	/** end theatre/audience room **/

	trigger (ch, argument, TRIG_SAY);

	if (cmd == 2 && IS_NPC (target))
		reply_reset (ch, target, argument, cmd);

	if (cmd == 0)
	{
		if (allocd)
			mem_free (utters[cmd]); // char[]
	}

	/*
	if (ch->speaks == SKILL_BLACK_WISE || ch->speaks == SKILL_GREY_WISE
	|| ch->speaks == SKILL_WHITE_WISE)
	magic_incantation (ch, argument);
	*/

	mem_free (argument); // char * ??? <- why freeing this here???
}

void
	do_sing (CHAR_DATA * ch, char *argument, int cmd)
{
	do_say (ch, argument, 1);	/* 1 = sing */
}

void
	do_radio (CHAR_DATA * ch, char *argument, int cmd)
{
	do_say (ch, argument, 6);	/* 6 = radio */
}


void
	do_ichat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	DESCRIPTOR_DATA *i = NULL;

	if (!GET_TRUST (ch) && !IS_SET (ch->flags, FLAG_ISADMIN))
	{
		send_to_char ("Eh?\n", ch);
		return;
	}

	for (; *argument == ' '; argument++);

	if (!(*argument))
	{
		send_to_char ("What message would you like to send?\n", ch);
		return;
	}
	else
	{
		/// Use the admin's wiznet flag (ignore the NPC's)
		bool ch_wiznet_set =
			(IS_NPC(ch) && ch->descr()->original)
			? GET_FLAG (ch->descr()->original, FLAG_WIZNET)
			: GET_FLAG (ch, FLAG_WIZNET);

		if (!ch_wiznet_set)
		{
			send_to_char
				("You are not currently tuned into the wiznet. "
				"Type SET WIZNET to change this.\n", ch);
			return;
		}

		if (IS_NPC (ch) && ch->descr()->original)
		{
			sprintf (buf1, "#1[Wiznet: %s (%s)]#0 %s\n",
				GET_NAME (ch->descr()->original),
				GET_NAME (ch), CAP (argument));
		}
		else
		{
			sprintf (buf1, "#1[Wiznet: %s]#0 %s\n",
				GET_NAME (ch), CAP (argument));
		}

		reformat_string (buf1, &p);
		p[0] = toupper (p[0]);

		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character
				&& !i->connected
				&& (GET_TRUST (i->character)
				|| IS_SET (i->character->flags, FLAG_ISADMIN)))
			{
				*s_buf = '\0';

				bool tch_wiznet_set =
					(i->original)
					? GET_FLAG (i->original, FLAG_WIZNET)
					: GET_FLAG (i->character, FLAG_WIZNET);

				if (IS_SET (i->character->act, PLR_QUIET))
				{
					sprintf (s_buf, "#2[%s is editing.]#0\n",
						GET_NAME (i->character));
				}
				else if (!tch_wiznet_set)
				{
					if (!IS_MORTAL (i->character))
					{
						CHAR_DATA *tch = (i->original)
							? (i->original)
							: (i->character);
						sprintf
							(s_buf, "#2[%s is not listening to the wiznet.]#0\n",
							GET_NAME (tch));
					}
				}
				else
				{
					send_to_char (p, i->character);
				}

				if (*s_buf)
				{
					send_to_char (s_buf, ch);
				}
			}
		}
		mem_free (p); // char*
	}
}

void
	do_helpline (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	DESCRIPTOR_DATA *i = NULL;

	for (; *argument == ' '; argument++);

	if (!GET_FLAG (ch, FLAG_NEWBNET) && !IS_NPC (ch))
	{
		send_to_char
			("You are not currently tuned into the helpline. Type SET HELPLINE to change this.\n",
			ch);
		return;
	}


	if (!(*argument))
	{
		send_to_char ("The following Accounts are currently connected to the Helpline:\n\n", ch);

		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character && GET_FLAG (i->character, FLAG_NEWBNET))
			{
				if (!i->connected)
				{
					if (GET_FLAG (i->character, FLAG_NEWBNET))
					{

						if (GET_TRUST(i->character))
						{
							sprintf(buf1, "#2[Admin]#0 - %s\n", i->character->pc->account_name);
							send_to_char (buf1, ch);
						}
						else if (IS_GUIDE(i->character))
						{
							sprintf(buf1, "#3[Guide]#0 - %s\n", i->character->pc->account_name);
							send_to_char (buf1, ch);
						}
						else
						{
							sprintf(buf1, "#6[ New ]#0 - %s\n", i->character->pc->account_name);
							send_to_char (buf1, ch);
						}
					}
				}
			}
		}
		mem_free (p); // char*

	}
	else
	{
		if (!GET_FLAG (ch, FLAG_NEWBNET) && !IS_NPC (ch))
		{
			send_to_char
				("You are not currently tuned into the helpline. Type SET HELPLINE to change this.\n",
				ch);
			return;
		}

		sprintf (buf1, "#3[Helpline: %s]#0 %s\n", ch->pc->account_name, CAP (argument));

		reformat_string (buf1, &p);

		for (i = descriptor_list; i; i = i->next)
			if (i->character && GET_FLAG (i->character, FLAG_NEWBNET))
			{
				if (!i->connected)
				{
					*s_buf = '\0';
					if (!IS_SET (i->character->act, PLR_QUIET) && GET_FLAG (i->character, FLAG_NEWBNET))
					{
						send_to_char (p, i->character);
					}
					else
					{
						if (IS_SET (i->character->act, PLR_QUIET))
							sprintf (s_buf, "#3[%s is editing.]#0\n", i->character->pc->account_name);

						send_to_char (s_buf, ch);
					}
				}
			}
			mem_free (p); // char*
	}

}

void
	do_tell (CHAR_DATA * ch, char *argument, int cmd)
{
	do_say (ch, argument, 2);
}

void
	do_immtell (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict = NULL;
	DESCRIPTOR_DATA *d = NULL;
	char *p = '\0';
	char name[MAX_STRING_LENGTH] = { '\0' };
	char message[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	bool group = false;
	DESCRIPTOR_DATA *i = NULL;

	argument = one_argument(argument, name);
	one_argument(argument, message);

	if (!str_cmp(message, "!"))
	{
		argument = one_argument(argument, message);
		group = true;
	}

	if (!*name || !*argument)
		send_to_char ("Who do you wish to tell what??\n", ch);

	else if (!(vict = get_char_nomask (name)) || IS_NPC (vict) ||
		(!GET_TRUST (vict) && IS_SET (vict->flags, FLAG_WIZINVIS)))
		send_to_char ("There is nobody playing the mud by that name.\n", ch);

	else if (ch == vict)
		send_to_char ("You try to tell yourself something.\n", ch);

	else if (IS_SET (vict->act, PLR_QUIET))
	{
		send_to_char ("That player is editing, try again later.\n", ch);
		return;
	}

	else
	{
		if (IS_MORTAL (ch) && IS_SET (vict->flags, FLAG_ANON))
		{
			send_to_char ("There is nobody playing the mud by that name.\n",
				ch);
			return;
		}

		if (!vict->descr() && !IS_NPC (vict))
		{
			for (d = descriptor_list; d; d = d->next)
			{
				if (d == vict->pc->owner)
					break;
			}

			if (!d)
			{
				send_to_char ("That player has disconnected.\n", ch);
				return;
			}
		}

		sprintf (buf, "#2[From %s]#0 %s\n",
			(IS_NPC (ch) ? ch->short_descr : GET_NAME (ch)),
			CAP (argument));
		reformat_string (buf, &p);
		send_to_char (p, vict);
		mem_free (p); // char*

		sprintf (buf, "#5[To %s]#0 %s\n", GET_NAME (vict), CAP (argument));
		reformat_string (buf, &p);
		send_to_char (p, ch);

		if (group)
		{
			sprintf (buf, "#6[%s to %s]#0 %s\n", GET_NAME(ch), GET_NAME (vict), CAP (argument));
			reformat_string (buf, &p);
			send_to_char (p, ch);

			for (i = descriptor_list; i; i = i->next)
			{
				if (i->character
					&& !i->connected
					&& (GET_TRUST (i->character)))
				{
					if (i->character == ch)
						continue;

					*s_buf = '\0';

					bool tch_wiznet_set =
						(i->original)
						? GET_FLAG (i->original, FLAG_WIZNET)
						: GET_FLAG (i->character, FLAG_WIZNET);

					if (IS_SET (i->character->act, PLR_QUIET))
					{
						sprintf (s_buf, "#2[%s is editing.]#0\n",
							GET_NAME (i->character));
					}
					else if (!tch_wiznet_set)
					{
						if (!IS_MORTAL (i->character))
						{
							CHAR_DATA *tch = (i->original)
								? (i->original)
								: (i->character);
							sprintf
								(s_buf, "#2[%s is not listening to the wiznet.]#0\n",
								GET_NAME (tch));
						}
					}
					else
					{
						send_to_char (p, i->character);
					}

					if (*s_buf)
					{
						send_to_char (s_buf, ch);
					}
				}
			}
		}

		mem_free (p); // char*

	}



}

int
	whisper_it (CHAR_DATA * ch, char *source, char *target, int mode)
{
	int missed = 0;
	int got_one = 0;
	int bonus = 0;
	int base = 0;
	char *in = '\0';
	char *out = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *helm = NULL;

	if (ch->skills[SKILL_VOODOO])
		bonus = 20;

	if (!ch->skills[SKILL_EAVESDROP])
		skill_learn(ch, SKILL_EAVESDROP);

	base = ch->skills[SKILL_EAVESDROP];
	// if you're wearing a helm or a mask, you're not going to be able to hear very well.

	if (((helm = get_equip (ch, WEAR_HEAD)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK))
		|| ((helm = get_equip (ch, WEAR_FACE)) && IS_SET (helm->obj_flags.extra_flags, ITEM_MASK)))
		base = base / 2;


	// Very slow rate of growth - hooray.

	if (base <= 9)
		base = 9;

	// If we put our mode is as anything but 1 or 0, then it's a straight mod check.
	if (mode > 1 || mode < 0)
	{
		bonus = mode;
	}
	if (mode == 1)
	{
		if (!number(0,9))
			skill_use(ch, SKILL_EAVESDROP, 0);
	}
	else
	{
		if (base >= 10);
		{
			bonus += 15;
			if (!number(0,9))
				skill_use(ch, SKILL_EAVESDROP, 0);
		}
	}

	in = source;
	out = buf;
	*out = '\0';

	while (*in)
	{

		while (*in == ' ')
		{
			in++;
			*out = ' ';
			out++;
		}

		*out = '\0';

		if (base + bonus < number (1, SKILL_CEILING))
		{
			if (!missed)
			{
				strcat (out, " . . .");
				out += strlen (out);
			}

			missed = 1;

			while (*in && *in != ' ')
				in++;
		}

		else
		{
			while (*in && *in != ' ')
			{
				*out = *in;
				out++;
				in++;
			}

			got_one = 1;
			missed = 0;
		}

		*out = '\0';
	}

	strcpy (target, buf);

	return got_one;
}

void
	do_whisper (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0, key_e = 0;
	int heard_something = 0;
	CHAR_DATA *vict = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *tongues = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };
	char key[MAX_STRING_LENGTH] = { '\0' };
	char target_key[MAX_STRING_LENGTH] = { '\0' };
	char voice[MAX_STRING_LENGTH] = { '\0' };


	if (ch->room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("You can't do that underwater!\n", ch);
		return;
	}

	argument = one_argument (argument, name);

	if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (ch))
	{
		if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
		{
			send_to_char
				("You decide against speaking out of turn. PETITION to request to speak.\n",
				ch);
			return;
		}
	}

	//half_chop (argument, name, message);

	if (*argument == '(')
	{
		*voice = '\0';
		*buf = '\0';
		sprintf (buf, "%s", argument);
		i = 1;
		*buf5 = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				send_to_char ("What did you wish to whisper?\n", ch);
				return;
			}
			if (buf[i] == '*')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
					!get_obj_in_list_vis (ch, key, ch->right_hand) &&
					!get_obj_in_list_vis (ch, key, ch->left_hand) &&
					!get_obj_in_list_vis (ch, key, ch->equip))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				sprintf (buf5 + strlen (buf5), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}
			if (buf[i] == '~')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				sprintf (buf5 + strlen (buf5), "#5%s#0",
					char_short (get_char_room_vis (ch, key)));
				*key = '\0';
				continue;
			}
			sprintf (buf5 + strlen (buf5), "%c", buf[i]);
			i++;
		}
		strcpy (voice, buf5);
		while (*argument != ')')
			argument++;
		argument += 2;

		sprintf(buf5, "%s,", buf5);

		i = 0;
		*buf = '\0';
		if (cmd == 2 && *target_key)
			sprintf (buf, "%s %s", target_key, argument);
		else
			sprintf (buf, "%s", argument);
		*argument = '\0';
		argument = buf;
		if (!*argument)
		{
			send_to_char ("What did you wish to say?\n", ch);
			return;
		}
	}

	*argument = toupper (*argument);

	if (!*name || !*argument)
	{
		send_to_char ("Who do you want to whisper to.. and what?\n", ch);
		return;
	}

	if (cmd != 83 && !(vict = get_char_room_vis (ch, name)) )
	{
		send_to_char ("No-one by that name here.\n", ch);
		return;
	}
	else if ( cmd == 83 && !(vict = get_char_room (name, ch->in_room)) ) 	// Whisper used by NPC's only for the AUCTION command.
	{
		return;
	}
	else if (vict == ch)
	{
		act ("$n whispers quietly to $mself.", false, ch, 0, 0, TO_ROOM);
		send_to_char ("You whisper to yourself.\n", ch);
		return;
	}

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);

	if (!tongues && !real_skill (ch, ch->speaks)
		&& !IS_SET (ch->room->room_flags, OOC))
	{
		send_to_char ("You don't know the language you want to "
			"whisper\n", ch);
		return;
	}

	char *p = '\0';
	reformat_say_string (argument, &p, 0);

	// removed skill because of Common

	if (!IS_SET (ch->room->room_flags, OOC))
		sprintf (buf, "You whisper to $N, %s\n   \"%s\"",  buf5, p);
	else
		sprintf (buf, "You whisper to $N, %s \n   \"%s\"", buf5, p);

	act (buf, true, ch, 0, vict, TO_CHAR);

	sprintf (buf4, "%s", p);
	*buf4 = toupper (*buf4);
	sprintf (buf3, "%s", p);
	*buf3 = toupper (*buf3);

	skill_use (ch, ch->speaks, 0);

	for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch)		/* Don't say it to ourselves */
			continue;

		if ( tch != vict && cmd == 83 ) /* Coded shopkeep whisper - skip to reduce spam */
			continue;

		sprintf (buf2, p);

		heard_something = 1;

		if (tch != vict)
		{
			if (get_affect (tch, MUTE_EAVESDROP))
				continue;
			heard_something = whisper_it (tch, buf2, buf2, 0);
		}
		if (!heard_something)
		{
			sprintf (buf, "$n whispers something to $3, %s but you can't quite make out the words.",
				buf5);

			act(buf, true, ch, (OBJ_DATA *) vict, tch, TO_VICT | _ACT_FORMAT);
			continue;


		}

		if (tch == vict)
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf, "$3 whispers to you, %s",
					/*(tch->skills[ch->speaks] || tongues) ?
					skills[ch->speaks] : "an unknown tongue",*/ buf5);
				/*if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,", accent_desc (ch, ch->skills[ch->speaks]));*/
				act (buf, false, tch, (OBJ_DATA *) ch, 0,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "   \"%s\"", buf3);
				act (buf, false, tch, 0, 0, TO_CHAR);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf,
					"$3 whispers something to you, %s but you cannot decipher %s words.",
					/*(tch->skills[ch->speaks]
					|| tongues) ? skills[ch->
					speaks] : "an unknown tongue",*/ buf5,
					HSHR (ch));
				act (buf, false, tch, (OBJ_DATA *) ch, 0,
					TO_CHAR | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "$3 whispers to you, %s \n   \"%s\"", buf5, buf4);
				act (buf, false, tch, (OBJ_DATA *) ch, 0, TO_CHAR);
			}
			if (IS_NPC (vict))
				reply_reset (ch, vict, buf2, 4);	/* 4 = whisper */
		}

		else
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf, "You overhear $3 whispering to $N, %s ",
					/*(tch->skills[ch->speaks] || tongues) ?
					skills[ch->speaks] : "an unknown tongue",*/ buf5);
				/*if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));*/
				act (buf, false, tch, (OBJ_DATA *) ch, vict,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "   \"%s\"", buf2);
				act (buf, false, tch, 0, 0, TO_CHAR);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf,
					"You overhear $3 whispering something to $N, %s but you cannot decipher %s words.",
					/*(tch->skills[ch->speaks]
					|| tongues) ? skills[ch->
					speaks] : "an unknown tongue",*/ buf5,
					HSHR (ch));
				act (buf, false, tch, (OBJ_DATA *) ch, vict,
					TO_CHAR | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You overhear $3 whisper to $N, %s \n   \"%s\"",
					buf5, buf2);
				act (buf, false, tch, (OBJ_DATA *) ch, vict, TO_CHAR);
			}
		}

		sprintf (p, buf3);
	}

	mem_free (p); // char*

	trigger (ch, argument, TRIG_WHISPER);
}


void
	do_ask (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	char message[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	half_chop (argument, name, message);

	if (!*name || !*message)
		send_to_char ("Who do you want to ask something.. and what??\n", ch);
	else if (!(vict = get_char_room_vis (ch, name)))
		send_to_char ("No-one by that name here.\n", ch);
	else if (vict == ch)
	{
		act ("$n quietly asks $mself a question.", false, ch, 0, 0, TO_ROOM);
		send_to_char ("You think about it for a while...\n", ch);
	}
	else
	{
		sprintf (buf, "$n asks you '%s'", message);
		act (buf, false, ch, 0, vict, TO_VICT);
		send_to_char ("Ok.\n", ch);
		act ("$n asks $N a question.", false, ch, 0, vict, TO_NOTVICT);
	}
}

void
	do_talk (CHAR_DATA * ch, char *argument, int cmd)
{
	if (is_at_table (ch, NULL))
	{
		do_say (ch, argument, 3);
	}
	else if (is_with_group (ch))
	{
		do_say (ch, argument, 5);
	}
	else
	{
		do_say (ch, argument, 3);
	}
}

void
	do_petition (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *admin = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	bool sent = false;
	char *date;
	time_t current_time;

	argument = one_argument (argument, buf);

	while (isspace (*argument))
		argument++;

	if ( !ch->descr() )
		return;

	if (!*argument)
	{
		send_to_char ("Petition what message?\n", ch);
		return;
	}

	if (IS_SET (ch->plr_flags, NOPETITION))
	{
		act ("Your ability to petition/think has been revoked by an admin.",
			false, ch, 0, 0, TO_CHAR);
		return;
	}


	if (ch->descr()->acct
		&& IS_SET (ch->descr()->acct->flags, ACCOUNT_NOPETITION))
	{
		act ("Your ability to petition has been revoked by an admin.", false,
			ch, 0, 0, TO_CHAR);
		return;
	}

	if (strcasecmp (buf, "all") == STR_MATCH)
	{

		sprintf (buf, "#6[Petition: %s]#0 %s\n",
			IS_NPC (ch) ? ch->short_descr : GET_NAME (ch), CAP (argument));
		reformat_string (buf, &p);

		for (admin = character_list; admin; admin = admin->next)
		{

			if (admin->deleted)
				continue;

			if (!GET_TRUST (admin))
				continue;

			if (!admin->descr())
				continue;

			send_to_char (p, admin);

			if (!admin->descr()->idle)
				sent = true;
		}

		mem_free (p); // char*

		sprintf (buf, "You petitioned: %s\n", CAP (argument));
		reformat_string (buf, &p);
		send_to_char (p, ch);
		mem_free (p); // char*

		if (!get_affect (ch, MAGIC_PETITION_MESSAGE))
		{
			sprintf (buf,
				"\n#6Please understand that, as we are a staff of volunteers, we cannot be online\n"
				"to respond to petitions 24 hours a day. If you find your petitions going largely\n"
				"unanswered, email may be a more efficient recourse: "
				STAFF_EMAIL ".#0\n");
			send_to_char (buf, ch);
			sprintf (buf,
				"\n#6If there are no staff currently online, your petition will be logged for review\n"
				"and responded to within a few days by an administrator via email if necessary.#0\n");
			send_to_char (buf, ch);
			sprintf (buf,
				"\n#6If you have not already, please read #0HELP PETITION#6 for petitioning guidelines.\n#0");
			send_to_char (buf, ch);
			magic_add_affect (ch, MAGIC_PETITION_MESSAGE, 480, 0, 0, 0, 0);
		}

		if (!sent)
		{
			current_time = time (0);
			date = (char *) asctime (localtime (&current_time));
			date[strlen (date) - 1] = '\0';
			sprintf (buf, "From: %s [%d]\n\n", ch->tname, ch->in_room);
			sprintf (buf + strlen (buf), "%s\n", argument);
			add_message (1, "Petitions", -5, ch->tname, date, "Logged Petition",
				"", buf, 0);
			mem_free (date); // char*
		}

		return;
	}

	admin = load_pc (buf);

	if (!admin)
	{
		send_to_char ("Are you sure you didn't mistype the name?\n", ch);
		return;
	}

	if (admin == ch)
	{
		send_to_char ("Petition yourself? I see...\n", ch);
		unload_pc (admin);
		return;
	}

	if (!is_he_somewhere (admin) || !IS_SET (admin->flags, FLAG_AVAILABLE)
		|| !admin->pc->level)
	{
		send_to_char ("Sorry, but that person is currently unavailable.\n", ch);
		unload_pc (admin);
		return;
	}

	if (IS_SET (admin->act, PLR_QUIET))
	{
		send_to_char ("That admin is editing.  Please try again in a minute.\n",
			ch);
		unload_pc (admin);
		return;
	}

	sprintf (buf, "#5[Private Petition: %s]#0 %s\n",
		IS_NPC (ch) ? ch->short_descr : GET_NAME (ch), CAP (argument));
	reformat_string (buf, &p);
	send_to_char (p, admin);
	mem_free (p); // char*
	sprintf (buf, "You petitioned %s: %s\n", GET_NAME (admin), CAP (argument));
	reformat_string (buf, &p);
	send_to_char (p, ch);
	mem_free (p); // char*

	unload_pc (admin);
}


void
	do_shout (CHAR_DATA * ch, char *argument, int cmd)
{
	char key[MAX_STRING_LENGTH] = { '\0' };
	char target_key[MAX_STRING_LENGTH] = { '\0' };
	char voice[MAX_STRING_LENGTH] = { '\0' };
	int i = 0, key_e = 0;
	ROOM_DATA *room;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *tongues = NULL;
	int door = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };

	if (ch->room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("You can't do that underwater!\n", ch);
		return;
	}

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);
	for (; isspace (*argument); argument++);

	if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (ch))
	{
		if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
		{
			send_to_char
				("You decide against speaking out of turn. PETITION to request to speak.\n",
				ch);
			return;
		}
	}

	if (!tongues && !real_skill (ch, ch->speaks))
	{
		send_to_char ("You don't know that language!\n", ch);
		return;
	}

	if (!*argument)
	{
		send_to_char ("What would you like to shout?\n", ch);
		return;
	}

	if (*argument == '(')
	{
		*voice = '\0';
		*buf = '\0';
		sprintf (buf, "%s", argument);
		i = 1;
		*buf2 = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				send_to_char ("What did you wish to shout?\n", ch);
				return;
			}
			if (buf[i] == '*')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
					!get_obj_in_list_vis (ch, key, ch->right_hand) &&
					!get_obj_in_list_vis (ch, key, ch->left_hand) &&
					!get_obj_in_list_vis (ch, key, ch->equip))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				sprintf (buf5 + strlen (buf5), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}
			if (buf[i] == '~')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}

				sprintf (buf5 + strlen (buf5), "#5%s#0",
					char_short (get_char_room_vis (ch, key)));
				*key = '\0';
				continue;
			}
			sprintf (buf5 + strlen (buf5), "%c", buf[i]);
			i++;
		}
		sprintf (voice, buf5);
		sprintf(buf5, ", %s", voice);
		while (*argument != ')')
			argument++;
		argument += 2;

		i = 0;
		*buf = '\0';
		if (cmd == 2 && *target_key)
			sprintf (buf, "%s %s", target_key, argument);
		else
			sprintf (buf, "%s", argument);
		*argument = '\0';
		argument = buf;
		if (!*argument)
		{
			send_to_char ("What did you wish to say?\n", ch);
			return;
		}
	}

	argument[0] = toupper (argument[0]);
	reformat_say_string (argument, &argument, 0);

	sprintf (buf4, argument);	/* The intended message, sent to the player. */

	for (tch = vnum_to_room (ch->in_room)->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch)
		{
			if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "You shout%s,", /*skills[ch->speaks],*/ buf5);
			}
			else
			{
				sprintf (buf, "You shout%s,", buf5);
			}
			sprintf (buf2, "   \"%s\"", buf4);
			act (buf, false, ch, 0, 0, TO_CHAR);
			act (buf2, false, ch, 0, 0, TO_CHAR);
			continue;
		}

		if (!IS_SET (ch->room->room_flags, OOC))
			sprintf (buf, "$N shouts%s,", /*skills[ch->speaks],*/ buf5);
		else
			sprintf (buf, "$N shouts%s,", buf5);

		if (!tch->skills[ch->speaks] && !tongues
			&& !IS_SET (ch->room->room_flags, OOC))
		{
			sprintf (buf, "$N shouts%s, in an unknown tongue,", buf5);
		}

		/*if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
		sprintf (buf + strlen (buf), " %s,",
		accent_desc (ch, ch->skills[ch->speaks]));*/

		if (!decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks])
			&& !IS_SET (ch->room->room_flags, OOC))
		{
			sprintf (buf + strlen (buf),
				" something that you fail to decipher.");
			act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
		}
		else if (tch->descr())
		{
			act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
			if (decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf, "   \"%s\"\n", buf4);
				send_to_char (buf, tch);
			}
		}

		if (GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME))
			sprintf (buf3, " (%s)", GET_NAME (ch));
		else
			*buf3 = '\0';
		sprintf (buf2, argument);	/* Reset, for next listener. */
		continue;

	}

	if (ch->in_room == GRUNGE_PUBLIC_VIEW)
	{
		grunge_arena__do_shout (ch, argument, 0);
	}
	else if (ch->in_room == GRUNGE_PRIVATE_VIEW)
	{
		grunge_arena__do_shout (ch, argument, 1);
	}

	for (door = 0; door <= 5; door++)
	{
		if (EXIT (ch, door) && (EXIT (ch, door)->to_room != -1))
		{
			for (tch = vnum_to_room (EXIT (ch, door)->to_room)->people; tch;
				tch = tch->next_in_room)
			{

				if (tch == ch)
					continue;

				if (lookup_race_int(ch->race, RACE_NOMAD))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a bestial voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a bestial voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a bestial voice shout from %s", rev_d[door]);
				}
				else if (lookup_race_int(ch->race, RACE_BOT_BITS))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a metallic voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a metallic voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a metallic voice shout from %s", rev_d[door]);
				}
				else if (GET_SEX (ch) == SEX_MALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a male voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a male voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a male voice shout from %s",
						rev_d[door]);
				}
				else if (GET_SEX (ch) == SEX_FEMALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a female voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a female voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a female voice shout from %s",
						rev_d[door]);
				}
				else if (GET_SEX (ch) == SEX_NEUTRAL)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf, "You hear a voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a voice shout from %s", rev_d[door]);
				}
				/*if (tch->skills[ch->speaks] >= 50
				&& ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));*/
				if (!decipher_speaking
					(tch, ch->speaks, ch->skills[ch->speaks]))
				{
					sprintf (buf + strlen (buf),
						" though you cannot decipher the words.");
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				}
				else if (tch->descr())
				{
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
					if (decipher_speaking
						(tch, ch->speaks, ch->skills[ch->speaks]))
					{
						if (!IS_SET (ch->room->room_flags, OOC))
							sprintf (buf, "   \"%s\"\n", buf4);
						else
							sprintf (buf, "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}

				if (GET_TRUST (tch) && !IS_NPC (tch)
					&& GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				sprintf (buf2, argument);	/* Reset. */

			}
		}
	}

	for (room = full_room_list; room; room = room->lnext)
	{
		if (room->entrance == ch->in_room)
		{
			for (tch = room->people; tch; tch = tch->next_in_room)
			{

				if (tch == ch)
					continue;

				if (lookup_race_int(ch->race, RACE_NOMAD))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a bestial voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a bestial voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a bestial voice shout from %s", rev_d[door]);
				}
				else if (lookup_race_int(ch->race, RACE_BOT_BITS))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a metallic voice shout in an unknown tongue from %s",
							rev_d[door]);
						else
							sprintf (buf,
							"You hear a metallic voice shout from %s", rev_d[door]);
					}
					else
						sprintf (buf, "You hear a metallic voice shout from %s", rev_d[door]);
				}
				else if (GET_SEX (ch) == SEX_MALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a male voice shout in an unknown tongue from the outside,");
						else
							sprintf (buf,
							"You hear a male voice shout in %s from the outside,",
							skills[ch->speaks]);
					}
					else
						sprintf (buf,
						"You hear a male voice shout from the outside,");
				}
				else if (GET_SEX (ch) == SEX_FEMALE)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a female voice shout in an unknown tongue from the outside,");
						else
							sprintf (buf,
							"You hear a female voice shout in %s from the outside,",
							skills[ch->speaks]);
					}
					else
						sprintf (buf,
						"You hear a female voice shout from the outside,");
				}
				else if (GET_SEX (ch) == SEX_NEUTRAL)
				{
					if (!IS_SET (ch->room->room_flags, OOC))
					{
						if (!tch->skills[ch->speaks])
							sprintf (buf,
							"You hear a voice shout in an unknown tongue from the outside,");
						else
							sprintf (buf,
							"You hear a voice shout in %s from the outside,",
							skills[ch->speaks]);
					}
					else
						sprintf (buf, "You hear a voice shout from the outside,");
				}
				if (tch->skills[ch->speaks] >= 50
					&& ch->skills[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skills[ch->speaks]));
				if (!decipher_speaking
					(tch, ch->speaks, ch->skills[ch->speaks]))
				{
					sprintf (buf + strlen (buf),
						" though you cannot decipher the words.");
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				}
				else if (tch->descr())
				{
					act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
					if (decipher_speaking
						(tch, ch->speaks, ch->skills[ch->speaks]))
					{
						if (!IS_SET (ch->room->room_flags, OOC))
							sprintf (buf, "   \"%s\"\n", buf4);
						else
							sprintf (buf, "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}

				if (GET_TRUST (tch) && !IS_NPC (tch)
					&& GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				sprintf (buf2, argument);	/* Reset. */

			}
		}
	}

	/** For Theatre and Audience room only **/
	if (ch->in_room == StageRoom)
	{
		for (int index = 0; index < 4; index = index +1)
		{
			for (tch = vnum_to_room (AudienceRoom[index])->people; tch; tch = tch->next_in_room)
			{
				bool deciphered = false;
				if (!tch->descr())	/* NPC don't hear anything */
					continue;

				if (GET_TRUST (tch) &&
					!IS_NPC (tch) &&
					GET_FLAG (tch, FLAG_SEE_NAME))
					sprintf (buf3, " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				if (cmd == 0 || cmd == 1 || cmd == 2 || cmd == 3)
				{
					if (!IS_SET (ch->room->room_flags, OOC) &&
						decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
					{
						sprintf (buf,
							"On the stage, $N%s shouts,",
							buf3);

						if (tch->skills[ch->speaks] >= 50 &&
							ch->skills[ch->speaks] < 50)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								accent_desc (ch, ch->skills[ch->speaks]));
						}
						deciphered = true;
						if (*voice)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								voice);
						}
					}

					else if (!IS_SET (ch->room->room_flags, OOC))
					{
						sprintf (buf,
							"On the stage, $N%s shouts something,",
							buf3);

						if (*voice)
						{
							sprintf (buf + strlen (buf),
								" %s,",
								voice);
						}

						sprintf (buf + strlen (buf),
							" but you are unable to decipher %s words.",
							HSHR (ch));
						deciphered = false;
					}
				}

				else
					continue;

				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				sprintf (buf3, "   \"%s\"\n", buf4);
				send_to_char(buf3, tch);
			} //for (tch = vtor (AudienceRoom[index])->people
		}//for (index = 1; index; index = index +1)
	}//if (ch->in_room = StageRoom)
	/** end theatre/audience room **/

	mem_free (argument); // char* <- should we be freeing this???
}

int
	add_to_list (ROOM_DATA ** list, ROOM_DATA * room, int *elements)
{
	int i = 0;

	for (i = 0; i < *elements; i++)
		if (list[i] == room)
			return 0;

	list[*elements] = room;

	(*elements)++;

	return 1;
}

void
	get_room_list (int radius, ROOM_DATA * room, ROOM_DATA ** rooms,
	int dists[], int *num_rooms)
{
	int room_set_top = 0;
	int room_set_bot = 0;
	int dir = 0;

	*num_rooms = 0;

	add_to_list (rooms, room, num_rooms);

	room_set_top = 0;
	room_set_bot = 0;

	while (radius--)
	{
		while (room_set_top <= room_set_bot)
		{

			for (dir = 0; dir <= 5; dir++)
				if (rooms[room_set_top]->dir_option[dir])
					add_to_list (rooms, vnum_to_room (rooms[room_set_top]->
					dir_option[dir]->to_room), num_rooms);

			room_set_top++;
		}

		room_set_bot = *num_rooms - 1;

		if (room_set_top >= *num_rooms)
			break;			/* Ran out of rooms */
	}
}

void
	wolf_howl (CHAR_DATA * ch)
{
	int num_rooms = 0;
	int dists[220];
	int i = 0;
	int dir = 0;
	ROOM_DATA *rooms[220];
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	for (i = 0; i <= 219; i++)
		dists[i] = 0;
	for (i = 0; i <= 219; i++)
		rooms[i] = NULL;

	get_room_list (3, ch->room, rooms, dists, &num_rooms);

	act ("$n howls mournfully.", false, ch, 0, 0, TO_ROOM);
	act ("You howl mournfully.", false, ch, 0, 0, TO_CHAR);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			continue;
		if (tch->race != 17 && tch->race != 18 && tch->race != 19)
			continue;
		if (!ch->fighting)
			continue;
		if (tch->fighting || tch->delay)
			continue;
		set_fighting (ch, tch);
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, ch->room->vnum);

			if (dir == -1)
				continue;

			sprintf (buf,
				"There is a loud, mournful howl coming from the %s.\n",
				dirs[dir]);

			send_to_room (buf, rooms[i]->vnum);
		}

		for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
		{

			if (!IS_NPC (tch))
				continue;

			if (tch->race != 17 && tch->race != 18 && tch->race != 19)
				continue;

			if (tch->fighting || tch->delay || IS_SET (tch->act, ACT_SENTINEL))
				continue;

			act ("$n stands upright suddenly, cocking $s head.",
				true, tch, 0, 0, TO_ROOM);

			tch->delay = 2;
			tch->delay_type = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if (ch->fighting)
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void
	spider_screech (CHAR_DATA * ch)
{
	int num_rooms = 0;
	int dists[220];
	int i = 0;
	int dir = 0;
	ROOM_DATA *rooms[220];
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	for (i = 0; i <= 219; i++)
		dists[i] = 0;
	for (i = 0; i <= 219; i++)
		rooms[i] = NULL;

	get_room_list (3, ch->room, rooms, dists, &num_rooms);

	act ("$n screeches loudly.", false, ch, 0, 0, TO_ROOM);
	act ("You screech loudly.", false, ch, 0, 0, TO_CHAR);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			continue;
		if (tch->race != 26)
			continue;
		if (!ch->fighting)
			continue;
		if (tch->fighting || tch->delay)
			continue;
		set_fighting (ch, tch);
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, ch->room->vnum);

			if (dir == -1)
				continue;

			sprintf (buf,
				"There is a loud, horrid screech coming from the %s.\n",
				dirs[dir]);

			send_to_room (buf, rooms[i]->vnum);
		}

		for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
		{

			if (!IS_NPC (tch))
				continue;

			if (tch->race != 26)
				continue;

			if (tch->fighting || tch->delay || IS_SET (tch->act, ACT_SENTINEL))
				continue;

			act ("$n turns suddenly, cocking $s head.",
				true, tch, 0, 0, TO_ROOM);

			tch->delay = 2;
			tch->delay_type = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if (ch->fighting)
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void
	warg_howl (CHAR_DATA * ch)
{
	int num_rooms = 0;
	int dists[220];
	int i = 0;
	int dir = 0;
	ROOM_DATA *rooms[220];
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	for (i = 0; i <= 219; i++)
		dists[i] = 0;
	for (i = 0; i <= 219; i++)
		rooms[i] = NULL;

	get_room_list (3, ch->room, rooms, dists, &num_rooms);

	act ("$n gives a blood-curdling howl.", false, ch, 0, 0, TO_ROOM);
	act ("You give a blood-curdling howl.", false, ch, 0, 0, TO_CHAR);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			continue;
		if (tch->race != 17 && tch->race != 18 && tch->race != 19)
			continue;
		if (!ch->fighting)
			continue;
		if (tch->fighting || tch->delay)
			continue;
		set_fighting (ch, tch);
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, ch->room->vnum);

			if (dir == -1)
				continue;

			sprintf (buf,
				"There is a blood-curdling howl coming from the %s.\n",
				dirs[dir]);

			send_to_room (buf, rooms[i]->vnum);
		}

		for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
		{

			if (!IS_NPC (tch))
				continue;

			if (tch->race != 62)
				continue;

			if (tch->fighting || tch->delay || IS_SET (tch->act, ACT_SENTINEL))
				continue;

			act ("$n stands upright suddenly, and bares $s teeth.",
				true, tch, 0, 0, TO_ROOM);

			tch->delay = 2;
			tch->delay_type = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if (ch->fighting)
				add_threat (tch, ch->fighting, 3);
		}
	}
}
void
	insect_smell (CHAR_DATA * ch)
{
	int num_rooms = 0;
	int dists[220];
	int i = 0;
	int dir = 0;
	ROOM_DATA *rooms[220];
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	for (i = 0; i <= 219; i++)
		dists[i] = 0;
	for (i = 0; i <= 219; i++)
		rooms[i] = NULL;

	get_room_list (3, ch->room, rooms, dists, &num_rooms);

	act ("$n exudes a strong scent.", false, ch, 0, 0, TO_ROOM);
	act ("You exude a strong scent.", false, ch, 0, 0, TO_CHAR);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			continue;
		if (tch->race != 108)
			continue;
		if (!ch->fighting)
			continue;
		if (tch->fighting || tch->delay)
			continue;
		set_fighting (ch, tch);
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, ch->room->vnum);

			if (dir == -1)
				continue;

			sprintf (buf,
				"You catch a whiff of a strong smell coming from the %s.\n",
				dirs[dir]);

			send_to_room (buf, rooms[i]->vnum);
		}

		for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
		{

			if (!IS_NPC (tch))
				continue;

			if (tch->race != 108)
				continue;

			if (tch->fighting || tch->delay || IS_SET (tch->act, ACT_SENTINEL))
				continue;

			act ("$n shudders and turns towards the source of a drifting scent.",
				true, tch, 0, 0, TO_ROOM);

			tch->delay = 2;
			tch->delay_type = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if (ch->fighting)
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void
	do_alert (CHAR_DATA * ch, char *argument, int cmd)
{
	int num_rooms = 0;
	int dists[220];
	int i = 0;
	int dir = 0;
	ROOM_DATA *rooms[220];
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int alert_type = 0;

	alert_type = lookup_race_int(ch->race, RACE_ALERT);


	for (i = 0; i <= 219; i++)
		dists[i] = 0;
	for (i = 0; i <= 219; i++)
		rooms[i] = NULL;

	if (GET_POS (ch) < REST)
		return;

	if (alert_type == RACE_ALERT_HOWL)
	{
		wolf_howl(ch);
		return;
	}
	else if (alert_type == RACE_ALERT_SCREECH)
	{
		spider_screech(ch);
		return;
	}
	else if (alert_type == RACE_ALERT_SMELL)
	{
		insect_smell(ch);
		return;
	}

	get_room_list (3, ch->room, rooms, dists, &num_rooms);

	act ("$n whistles very loudly.", false, ch, 0, 0, TO_ROOM);
	act ("You whistle very loudly.", false, ch, 0, 0, TO_CHAR);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			continue;
		if (!ch->fighting)
			continue;
		if (tch->fighting || tch->delay)
			continue;
		if (!is_brother (ch, tch))
			continue;
		set_fighting (ch, tch);
	}

	for (i = 1; i < num_rooms; i++)
	{

		if (rooms[i]->people)
		{
			dir = track (rooms[i]->people, ch->room->vnum);

			if (dir == -1)
				continue;

			sprintf (buf, "There is a loud whistle coming from the %s.\n",
				dirs[dir]);

			send_to_room (buf, rooms[i]->vnum);
		}

		for (tch = rooms[i]->people; tch; tch = tch->next_in_room)
		{

			if (!IS_NPC (tch))
				continue;

			if (tch->fighting || tch->delay || IS_SET (tch->act, ACT_SENTINEL))
				continue;

			if (!is_brother (ch, tch))
				continue;

			act ("$n glances up suddenly with a concerned look on $s face.",
				true, tch, 0, 0, TO_ROOM);

			tch->delay = 2;
			tch->delay_type = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if (ch->fighting)
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void
	delayed_alert (CHAR_DATA * ch)
{
	int dir = 0;
	int save_speed = 0;
	int current_room = 0;
	ROOM_DATA *to_room = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	dir = track (ch, ch->delay_info1);

	if (dir == -1)
	{
		send_to_char ("You can't figure out where the whistle came from.\n",
			ch);
		ch->delay = 0;
		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		return;
	}

	current_room = ch->in_room;

	if (IS_HITCHEE (ch))
		return;

	if (IS_SUBDUEE (ch))
		return;

	if (!EXIT (ch, dir))
		return;

	if (!(to_room = vnum_to_room (EXIT (ch, dir)->to_room)))
		return;

	if (IS_SET (to_room->room_flags, NO_MOB))
		return;

	if (IS_MERCHANT (ch) && IS_SET (to_room->room_flags, NO_MERCHANT))
		return;

	if (ch->mob && ch->mob->access_flags &&
		!(ch->mob->access_flags & to_room->room_flags))
		return;

	if (IS_SET (ch->act, ACT_STAY_ZONE) && ch->room->zone != to_room->zone)
		return;

	save_speed = ch->speed;
	ch->speed = SPEED_RUN;

	sprintf (buf, "%s", dirs[dir]);

	command_interpreter (ch, buf);

	ch->speed = save_speed;

	ch->delay_info2--;

	if (current_room == ch->in_room || ch->delay_info2 <= 0)
	{
		send_to_char ("You can't locate the whistle.\n", ch);
		ch->delay = 0;
		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay_ch = 0;
		return;
	}

	if (ch->in_room != ch->delay_info1)
		ch->delay = 8;
	else
	{
		ch->delay = 0;
		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
	}
}

void
	clear_voice (CHAR_DATA * ch)
{
	if (ch->voice_str)
	{
		mem_free (ch->voice_str); // char*
		ch->voice_str = NULL;
	}
}


/****************************************************************************
*                                                            TRAVEL STRING *
****************************************************************************/

#define TRAVEL_RESET "normal"	/* keyword to reset user travel string */

/*                                                                          *
* function: clear_travel               < e.g.> travel normal               *
*                                                                          */
void
	clear_travel (CHAR_DATA * ch)
{
	if (ch->travel_str)
	{
		mem_free (ch->travel_str); // char*
		ch->travel_str = NULL;
		send_to_char ("Your travel string has been cleared.\n", ch);
	}
}

/*                                                                          *
* function: clear_travel               < e.g.> travel [ normal|<string> ]  *
*                                                                          */
void
	do_travel (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (strchr (argument, '~'))
	{
		send_to_char
			("Sorry, but you can't use tildae when setting a travel string.\n",
			ch);
		return;
	}

	if (!*argument)
	{
		if (ch->travel_str)
		{
			sprintf (buf, "Your current travel string: (#2%s#0)\n",
				ch->travel_str);
		}
		else
		{
			sprintf (buf, "You do not currently have a travel string set.\n");
		}
		send_to_char (buf, ch);
	}
	else
	{
		if (strcasecmp (argument, TRAVEL_RESET) == STR_MATCH)
		{
			clear_travel (ch);
		}
		else
		{
			sprintf (buf, "Your travel string has been set to: (#2%s#0)",
				argument);
			ch->travel_str = add_hash (argument);
		}
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

bool evaluate_emote_string (CHAR_DATA * ch, std::string * first_person, std::string third_person, std::string argument)
{
	OBJ_DATA * object;
	CHAR_DATA * tch = NULL;
	int i = 1;
	std::string output_string = "", key_string = "", error_string = "";

	if (argument[0] == '(')
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			i = 1;
			output_string.clear();
			while (argument[i] != ')')
			{
				if (argument[i] == '\0')
				{
					send_to_char ("Incorrect usage of emote string - please see HELP EMOTE\n", ch);
					return false;
				}

				if (argument[i] == '*')
				{
					i++;
					key_string.clear();
					object = NULL;
					while (isalpha(argument[i]) || argument[i] == '-')
						key_string.push_back(argument[i++]);
					if (!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->right_hand)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->left_hand)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->room->contents)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->equip)) )
					{
						error_string = "You cannot find an object with the keyword [#2";
						error_string.append(key_string);
						error_string.append("]#0\n");
						send_to_char (error_string.c_str(), ch);
						return false;
					}

					output_string.append("#2");
					output_string.append(obj_short_desc(object));
					output_string.append("#0");
					continue;
				}

				if (argument[i] == '~')
				{
					i++;
					key_string.clear();
					while (isalpha(argument[i]) || argument[i] == '-')
						key_string.push_back(argument[i++]);
					if (!get_char_room_vis(ch, key_string.c_str()))
					{
						error_string = "You cannot find a person with the keyword [#5";
						error_string.append(key_string);
						error_string.append("#0]\n");
						send_to_char (error_string.c_str(), ch);
						return false;
					}

					if (get_char_room_vis(ch, key_string.c_str()) == tch)
					{
						output_string.append("#5you#0");
					}
					else
					{
						output_string.append("#5");
						output_string.append(char_short(get_char_room_vis(ch, key_string.c_str())));
						output_string.append("#0");
					}
					continue;
				}

				output_string.push_back(argument[i++]);
			}

			if (tch == ch)
			{
				send_to_char (first_person->c_str(), ch);
			}
			else
			{
				send_to_char (third_person.c_str(), tch);
			}
			output_string.push_back('.');
			output_string.push_back('\n');
			send_to_char (", ", tch);
			send_to_char (output_string.c_str(), tch);
			continue;
		}
	}

	else
	{

		char non_const_first [MAX_STRING_LENGTH] = "";
		const_to_non_const_cstr(first_person->append(".").c_str(), non_const_first);
		char non_const_third [MAX_STRING_LENGTH] = "";
		const_to_non_const_cstr(third_person.append(".").c_str(), non_const_third);
		act (non_const_first, false, ch, 0, 0, TO_CHAR);
		act (non_const_third, false, ch, 0, 0, TO_ROOM);
	}

	error_string.clear();
	first_person->clear();
	if (argument[0] == '(')
	{
		for ( i++; argument[i] != '\0'; i++)
		{
			error_string.push_back(argument[i]);
		}
		first_person->assign(error_string);
	}
	else
		first_person->assign(argument);

	return true;

}

void
	do_plan (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[AVG_STRING_LENGTH * 2] = "";

	// change of plans
	if (argument && *argument)
	{
		while (isspace (*argument))
			argument++;

		// clear both strings
		if (strcasecmp (argument, "clear") == STR_MATCH )
		{
			if (ch->plan)
			{
				delete ch->plan;
				ch->plan = 0;
			}
			if (ch->goal)
			{
				delete ch->goal;
				ch->goal = 0;
			}
			send_to_char ("All of your plans have been cleared.\n", ch);
		}

		// change the short-term plan
		else if (strncmp (argument, "short ", 6) == STR_MATCH)
		{
			argument += 6;

			// clear the short term plan
			if (strcasecmp (argument, "clear") == STR_MATCH)
			{
				if (ch->plan)
				{
					delete ch->plan;
					ch->plan = 0;
				}
				send_to_char ("Your short-term plan has been cleared.\n", ch);
			}

			// (re)set the short-term plan
			else
			{
				int plan_length = strlen(argument);
				if (plan_length && plan_length < 80)
				{
					if (ch->plan)
					{
						delete ch->plan;
						ch->plan = 0;
					}

					ch->plan = new std::string (argument);
					sprintf (buf, "Your short-term plan has been set to:\n"
						"#6%s#0\n", argument);
					send_to_char (buf, ch);
				}

				// bad plan message size
				else
				{
					send_to_char ("Your short-term plan must be less than eighty characters in length.\nTo clear your plan, type #6plan short clear#0.\n", ch);
				}
			}
		}

		// change the long-term plan
		else if (strncmp (argument, "long ", 5) == STR_MATCH)
		{
			argument += 5;

			// clear the long-term plan
			if (strcasecmp (argument, "clear") == STR_MATCH)
			{
				if (ch->goal)
				{
					delete ch->goal;
					ch->goal = 0;
				}
				send_to_char ("Your long-term plan has been cleared.\n", ch);
			}

			// (re)set the long-term plan
			else
			{
				int goal_length = strlen(argument);
				if (goal_length && goal_length < 240)
				{
					if (ch->goal)
					{
						delete ch->goal;
						ch->goal = 0;
					}

					ch->goal = new std::string (argument);
					sprintf (buf, "Your long-term plan has been set to:\n\n"
						"   #6%s#0\n", argument);
					char *p;
					reformat_string (buf, &p);
					send_to_char (p, ch);
					mem_free (p);
				}

				// bad message size
				else
				{
					send_to_char ("Your long-term plan must be at most three lines.\nTo clear your plan, type #6plan long clear#0.\n", ch);
				}
			}
		}
		else
		{
			const char * usage =
				"To set your short-term plan, type:     #6plan short <message>#0\n"
				"To clear your short-term plan, type:   #6plan short clear#0\n"
				"To set your long-term plan, type:      #6plan long <message>#0\n"
				"To clear your long-term plan, type:    #6plan long clear#0\n"
				"To clear your all of your plans, type: #6plan clear#0\n";
			send_to_char (usage,ch);
		}
	}
	else
	{
		if ((ch->plan && !ch->plan->empty()) || (ch->goal && !ch->goal->empty()))
		{
			strcat (buf,"Your plans:\n");
			if (ch->goal)
				sprintf (buf + strlen(buf), "\nLong-term:\n#6   %s#0\n", ch->goal->c_str());

			if (ch->plan)
				sprintf (buf + strlen(buf), "\nCurrently:\n#6%s#0\n", ch->plan->c_str());

			send_to_char (buf, ch);
		}
		else
		{
			send_to_char ("You do not have any plans.\n", ch);
		}
	}

}
