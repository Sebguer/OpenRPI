/*------------------------------------------------------------------------\
|  guides.c : routines for quasi-staff Guides         www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"


CHAR_DATA *assist_queue = NULL;

void
do_assist (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch = NULL;
  int pos = 0, i = 0;
  char buf[MAX_STRING_LENGTH];

  if (!IS_GUIDE (ch) && IS_MORTAL (ch)
      && !IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
    {
      send_to_char
	("Only Guides, staff members, and new characters may use this command.\n",
	 ch);
      return;
    }

  if (!str_cmp (argument, "request"))
    {
      if (str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
	  send_to_char
	    ("This command may only be invoked in a pre-game debriefing room.\n",
	     ch);
	  return;
	}
      if (!IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
	{
	  send_to_char
	    ("The assist queue is only available to new characters.\n", ch);
	  return;
	}
      if ((pos = get_queue_position (ch)) != -1)
	{
	  sprintf (buf,
		   "You are already number #6%d#0 in the assist queue.\n",
		   pos);
	  send_to_char (buf, ch);
	  return;
	}
      update_assist_queue (ch, false);
      ch->assist_pos = get_queue_position (ch);
      sprintf (buf, "You are now number #6%d#0 in the assist queue.\n",
	       get_queue_position (ch));
      send_to_char (buf, ch);
      return;
    }
  else if (!str_cmp (argument, "cancel"))
    {
      if ((pos = get_queue_position (ch)) == -1)
	{
	  send_to_char ("You are not currently in the assist queue.\n", ch);
	  return;
	}
      update_assist_queue (ch, true);
      send_to_char ("You have been removed from the assist queue.\n", ch);
      return;
    }
  else if (!str_cmp (argument, "list") || !str_cmp (argument, "queue"))
    {
      if (!IS_GUIDE (ch) && IS_MORTAL (ch))
	{
	  send_to_char
	    ("Only Guides and staff members may see the assist queue.\n", ch);
	  return;
	}
      if (!assist_queue)
	{
	  send_to_char ("The assist queue is currently empty.\n", ch);
	  return;
	}
      sprintf (buf, "#6Currently waiting in the assist queue:#0\n\n");
      for (tch = assist_queue; tch; tch = tch->next_assist)
	{
	  i++;
	  sprintf (buf + strlen (buf), "  %2d. %s\n", i, tch->pc->account_name);
	}
      send_to_char (buf, ch);
      return;
    }
  else if (!str_cmp (argument, "answer"))
    {
      if (!IS_GUIDE (ch) && IS_MORTAL (ch))
	{
	  send_to_char
	    ("Only Guides and staff members may answer assist requests.\n",
	     ch);
	  return;
	}
      if (IS_GUIDE (ch) && !IS_SET (ch->flags, FLAG_GUEST))
	{
	  send_to_char
	    ("You may only answer assist requests via your Guest login.\n",
	     ch);
	  return;
	}
      if (!(tch = assist_queue))
	{
	  send_to_char
	    ("There is currently no-one waiting in the assist queue.\n", ch);
	  return;
	}
      act ("Your request for assistance has been answered!", true, tch, 0, 0,
	   TO_CHAR);

      if (tch->in_room != ch->in_room)
	{
	  tch->was_in_room = tch->in_room;
	  send_to_char ("\n", tch);
	  act ("$n vanishes in a subtle glimmer of light.", true, ch, 0, 0,
	       TO_ROOM | _ACT_FORMAT);
	  char_from_room (ch);
	  char_to_room (ch, tch->in_room);
	  act ("$n appears in a subtle glimmer of light.", true, ch, 0, 0,
	       TO_ROOM | _ACT_FORMAT);
	  send_to_char ("\n", ch);
	  act
	    ("The world around you fades away in a glimmer of light and you feel yourself whisked instantly to $N's location.",
	     false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
	  send_to_char ("\n", ch);
	}
      else
	send_to_char ("\n", ch);

      update_assist_queue (tch, true);

      send_to_char ("\n", ch);

      do_look (ch, "", 0);

      return;
    }
  else if (!str_cmp (argument, "return"))
    {
      if (!IS_GUIDE (ch) || !IS_SET (ch->flags, FLAG_GUEST))
	{
	  send_to_char ("This command is only for Guide guest logins.\n", ch);
	  return;
	}
      if (ch->in_room == OOC_LOUNGE)
	{
	  send_to_char ("You're already in Club Endore!\n", ch);
	  return;
	}
      act ("$n vanishes in a subtle glimmer of light.", true, ch, 0, 0,
	   TO_ROOM | _ACT_FORMAT);
      char_from_room (ch);
      char_to_room (ch, OOC_LOUNGE);
      act ("$n appears in a subtle glimmer of light.", true, ch, 0, 0,
	   TO_ROOM | _ACT_FORMAT);
      send_to_char ("\n", ch);
      act
	("The world around you fades away in a glimmer of light and you feel yourself whisked instantly back to Club Endore.",
	 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      send_to_char ("\n", ch);
      do_look (ch, "", 0);
      return;
    }
  else if (*argument)
    {
      send_to_char ("See #6HELP ASSIST#0 for command usage.\n", ch);
      return;
    }

  if ((pos = get_queue_position (ch)) != -1)
    {
      sprintf (buf, "You are currently number #6%d#0 in the assist queue.\n",
	       pos);
      send_to_char (buf, ch);
      return;
    }
  else
    send_to_char ("You are not currently in the assist queue.\n", ch);

  return;
}

int
get_queue_position (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  int i = 0;
  bool found = false;

  for (tch = assist_queue; tch; tch = tch->next_assist)
    {
      i++;
      if (tch == ch)
	{
	  found = true;
	  break;
	}
    }

  if (!found)
    return -1;
  else
    return i;
}

void
update_assist_queue (CHAR_DATA * ch, bool remove)
{
  CHAR_DATA *tch;
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  int pos = 0;

  if (remove)
    {
      if (assist_queue == ch)
	assist_queue = ch->next_assist;
      else
	for (tch = assist_queue; tch && tch->next_assist;
	     tch = tch->next_assist)
	  {
	    if (tch->next_assist == ch)
	      tch->next_assist = tch->next_assist->next_assist;
	  }

      ch->next_assist = NULL;

      sprintf (buf, "%s has been removed from the assist queue.",
	       ch->pc->account_name);
      buf[2] = toupper (buf[0]);

      send_to_guides (buf);

      for (d = descriptor_list; d; d = d->next)
	{
	  if (!d->character || d->connected != CON_PLYNG)
	    continue;
	  if ((pos = get_queue_position (d->character)) != -1
	      && d->character != ch && pos < d->character->assist_pos)
	    {
	      sprintf (buf,
		       "#6OOC:#0 You have advanced to position #6%d#0 in the assist queue.\n",
		       pos);
	      send_to_char (buf, d->character);
	    }
	  d->character->assist_pos = pos;
	}
    }
  else
    {
      if (!assist_queue)
	assist_queue = ch;
      else
	for (tch = assist_queue; tch; tch = tch->next_assist)
	  {
	    if (!tch->next_assist)
	      {
		tch->next_assist = ch;
		ch->next_assist = NULL;
		break;
	      }
	  }
      sprintf (buf, "%s has been added to the assist queue.",
	       ch->pc->account_name);
      buf[0] = toupper (buf[0]);

      send_to_guides (buf);
    }
}

/*                                                                          *
 * function: is_guide                                                       *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - simplified the logic flow by nesting                  *
 *                                                                          */
int
is_guide (CHAR_DATA *ch)
{
	if (ch && ch->pc && ch->pc->is_guide)
		return 1;
	else
		return 0;

  /*
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];
  int user_id = 0;

  if (!username || !*username)
    return 0;

  mysql_safe_query ("SELECT user_id FROM forum_users WHERE username = '%s'",
		    username);
  if ((result = mysql_store_result (database)) != NULL)
    {
      row = mysql_fetch_row (result);
      if (row)
	{
	  user_id = atoi (row[0]);
	  mysql_safe_query
	    ("SELECT user_id FROM forum_user_group WHERE user_id = %d AND group_id = 2521",
	     user_id);
	  mysql_free_result (result);
	  result = NULL;
	  if ((result = mysql_store_result (database)) != NULL)
	    {
	      row = mysql_fetch_row (result);
	      if (row)
		{
		  mysql_free_result (result);
		  result = NULL;
		  return 1;
		}
	      else
		mysql_free_result (result);
	      result = NULL;
	    }
	}
      else
	{
	  sprintf (buf, "Warning: is_guide(): %s", mysql_error (database));
	  system_log (buf, true);
	}
    }
  else
    {
      sprintf (buf, "Warning: is_guide(): %s", mysql_error (database));
      system_log (buf, true);
    }

  return 0;
  */

}
