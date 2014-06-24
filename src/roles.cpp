/*------------------------------------------------------------------------\
|  roles.c : coded support for the Role OLC system    www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>


#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

/*
 *	To-Do List:
 *
 *	1. Add an outfit_new_char() call in nanny.cpp for new RPP chars.
 *	2. Add skill bonuses/penalties to outfitting table.
 *	3. Add individual crafts to outfitting table.
 *
 */

#define	NEW_PLAYER_TRUNK	945
#define OOC_BRIEFING_SIGN	965

void
outfit_new_char (CHAR_DATA *ch, ROLE_DATA *role)
{
	MYSQL_RES       *result = NULL;
	MYSQL_ROW       row = NULL;
	OBJ_DATA        *obj = NULL, *tobj = NULL;
	CLAN_DATA       *clan;
    AFFECTED_TYPE   *af;
	char            buf[MAX_STRING_LENGTH];
    int             clan_flags = 0, pay_date = 0, job = 0, ind = 0;
    
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND obj_vnum > 0 ORDER BY obj_vnum ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( result && mysql_num_rows(result) > 0 && (tobj = load_object(NEW_PLAYER_TRUNK)))
	{
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !(obj = load_object(atoi(row[3]))) )
				continue;
            // There's a required skill the PC must have before we load this object.
            if ( strlen(row[12]) > 0 && (ind = index_lookup (skills, row[12])) != -1 )
            {
                if ( !ch->skills [ind] )
                    continue;
            }
            // There's a required race the PC must be before we load this object.
            if ( strlen(row[13]) > 0 && (ind = lookup_race_id (row[13])) != -1 )
            {
                if ( ch->race != ind )
                    continue;
            }
			obj->count = atoi(row[4]);
			obj_to_obj (obj, tobj);
		}
		
		obj_to_room (tobj, ch->room->vnum);
	}
	
	if ( result )
		mysql_free_result (result);
		
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND clan_string != '' ORDER BY clan_string ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( result && mysql_num_rows(result) > 0 )
	{
		while ( (row = mysql_fetch_row(result)) )
		{
            if ( !(clan = get_clandef(row[5])) )
                continue;
            clan_flags = 0;
            sprintf (buf, "%s", row[6]);
            if (!str_cmp (buf, "member"))
            {
                TOGGLE (clan_flags, CLAN_MEMBER);
            }
            else if (!str_cmp (buf, "leader"))
            {
                TOGGLE (clan_flags, CLAN_LEADER);
            }
            else if (!str_cmp (buf, "recruit"))
            {
                TOGGLE (clan_flags, CLAN_RECRUIT);
            }
            else if (!str_cmp (buf, "private"))
            {
                TOGGLE (clan_flags, CLAN_PRIVATE);
            }
            else if (!str_cmp (buf, "corporal"))
            {
                TOGGLE (clan_flags, CLAN_CORPORAL);
            }
            else if (!str_cmp (buf, "sergeant"))
            {
                TOGGLE (clan_flags, CLAN_SERGEANT);
            }
            else if (!str_cmp (buf, "lieutenant"))
            {
                TOGGLE (clan_flags, CLAN_LIEUTENANT);
            }
            else if (!str_cmp (buf, "captain"))
            {
                TOGGLE (clan_flags, CLAN_CAPTAIN);
            }
            else if (!str_cmp (buf, "general"))
            {
                TOGGLE (clan_flags, CLAN_GENERAL);
            }
            else if (!str_cmp (buf, "commander"))
            {
                TOGGLE (clan_flags, CLAN_COMMANDER);
            }
            else if (!str_cmp (buf, "apprentice"))
            {
                TOGGLE (clan_flags, CLAN_APPRENTICE);
            }
            else if (!str_cmp (buf, "journeyman"))
            {
                TOGGLE (clan_flags, CLAN_JOURNEYMAN);
            }
            else if (!str_cmp (buf, "master"))
            {
                TOGGLE (clan_flags, CLAN_MASTER);
            }
            if (!clan_flags)
                clan_flags = CLAN_MEMBER;
            add_clan (ch, row[5], clan_flags);     
		}
    }
	
	if ( result )
		mysql_free_result (result);

	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND payday_num > 0 ORDER BY payday_num ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( result && mysql_num_rows(result) > 0 )
	{
		while ( (row = mysql_fetch_row(result)) )
        {
            job = atoi(row[7]) + JOB_1 - 1;

            pay_date = time_info.month * 30 + time_info.day + time_info.year * 12 * 30 + atoi(row[11]);

            if ( (af = get_affect (ch, job)) )
                affect_remove (ch, af);

            job_add_affect (ch, job, atoi(row[11]), pay_date, 0, atoi(row[9]), atoi(row[8]), atoi(row[10]));
        }
    }
    
    if ( result )
        mysql_free_result (result);

    sprintf (buf, "SELECT role_desc FROM special_roles_outfit WHERE role_id = %d AND role_desc != ''", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( result && mysql_num_rows(result) > 0 && (tobj = load_object(OOC_BRIEFING_SIGN)) )
	{
		row = mysql_fetch_row(result);
		sprintf (buf,	"#6The strange, glowing text was perhaps left by a helpful Admin, seeking\n"
						"to help educate you in the intricacies of your new lot in life:#0\n\n%s", row[0]);
		tobj->full_description = str_dup (buf);
		obj_to_room (tobj, ch->room->vnum);
	}
	
	if ( result )
		mysql_free_result (result);
}

void
display_outfitting_table (CHAR_DATA *ch, ROLE_DATA *role)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	CLAN_DATA	*clan;
	char		buf[MAX_STRING_LENGTH], output[MAX_STRING_LENGTH], skills_buf[MAX_STRING_LENGTH];
	
	sprintf (output, "\n#6%s:#0\n"
					 "\n", role->summary);
					 	
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND obj_vnum > 0 ORDER BY obj_vnum ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	*buf = '\0';
	
	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "#6Starting Items:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Items:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !vtoo(atoi(row[3])) )
				continue;
            if ( strlen(row[12]) > 0 || strlen(row[13]) > 0 )
                sprintf (skills_buf, " (%s%s%s)", row[13], (strlen(row[12]) > 0 && strlen(row[13]) > 0) ? " && " : "", row[12]);
            else *skills_buf = '\0';
			sprintf (buf + strlen(buf), "  - %d x #2%s#0 [%d]%s\n", 
				atoi(row[4]), obj_short_desc(vtoo(atoi(row[3]))), atoi(row[3]), skills_buf);
		}
		sprintf (output + strlen(output), "%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND clan_string != '' ORDER BY clan_string ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Clanning:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Clanning:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !(clan = get_clandef (row[5])) )
				continue;
			sprintf (buf + strlen(buf), "  - #B%s#0 in #B'%s'#0 (%s)\n", row[6], clan->literal, row[5]);
		}
		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND payday_num > 0 ORDER BY payday_num ASC", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Paydays:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Paydays:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !vtoo(atoi(row[8])) || !vnum_to_mob(atoi(row[10])) )
				continue;
			sprintf (buf + strlen(buf), "  %d. %d x #2%s#0 from #5%s#0 every %d days.\n",
				atoi(row[7]), atoi(row[9]), obj_short_desc(vtoo(atoi(row[8]))), char_short(vnum_to_mob(atoi(row[10]))),
				atoi(row[11]));
		}
		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	sprintf (buf, "SELECT role_desc FROM special_roles_outfit WHERE role_id = %d AND role_desc != ''", role->id);
	mysql_safe_query (buf);
	
	result = mysql_store_result (database);
	
	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Role Briefing:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Role Briefing:#0\n");
		row = mysql_fetch_row(result);
		sprintf (buf + strlen(buf), "%s", row[0]);
		sprintf (output + strlen(output), "\n%s", buf);
		mysql_free_result (result);
	}
		
	send_to_char (output, ch);
}

void
post_brief (DESCRIPTOR_DATA * d)
{
	char	buf[MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH];

	if (!*d->pending_message->message)
    {
		send_to_char ("No role briefing was recorded.\n", d->character);
		d->pending_message = NULL;
		return;
    }

	mysql_real_escape_string (database, buf, d->pending_message->message, strlen(d->pending_message->message));
	d->pending_message = NULL;

	d->character->act &= ~PLR_QUIET;

	sprintf (buf2, "DELETE FROM special_roles_outfit WHERE role_id = %d AND role_desc != ''",
		d->character->delay_info1);
	mysql_safe_query(buf2);
	
	sprintf (buf2, "INSERT INTO special_roles_outfit (role_id, role_desc) VALUES (%d, '%s')",
		d->character->delay_info1, buf);
	mysql_safe_query(buf2);
	
	send_to_char ("Your role briefing has been recorded.\n", d->character);

	d->character->delay_who = NULL;
	d->character->delay_info1 = 0;
}

void
outfit_role (CHAR_DATA * ch, char *argument)
{
	CLAN_DATA	*clan = NULL;
	char		buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], skills_buf [MAX_STRING_LENGTH], races_buf[MAX_STRING_LENGTH];
	int			i = 1, vnum = 0, qty = 1, jobnum = 0, days = 0, employer = 0, ind = 0;
	ROLE_DATA	*role;
	bool		remove = false;

	argument = one_argument (argument, buf);

	if (!is_number (buf))
	{
		send_to_char ("Syntax: role outfit <role number>\n", ch);
		return;
    }

	for (role = role_list; role; role = role->next)
    {
		if (i == atoi (buf))
			break;
		i++;
    }

	if (!role)
    {
		send_to_char ("That role number was not found.\n", ch);
		return;
    }
	
	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "brief") )
	{
		send_to_char ("\n#2Enter a brief for this role you'd like to appear to starting characters:#0\n",ch);
		CREATE (ch->descr()->pending_message, MESSAGE_DATA, 1);
		ch->descr()->str = &ch->descr()->pending_message->message;
		ch->descr()->max_str = MAX_STRING_LENGTH;
		ch->descr()->proc = post_brief;
		ch->delay_info1 = role->id;
		ch->act |= PLR_QUIET;
		return;
	}
	
	if ( !str_cmp (buf, "outfittest") )
	{
		outfit_new_char (ch, role);
		send_to_char ("Done.\n", ch);
		return;
	}
	
	if ( !str_cmp (buf, "payday") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number(buf) )
		{
			send_to_char ("Usage: role outfit X payday <jobnum> <qty> <obj. vnum> <days> <employer>\n", ch);
			return;
		}
		jobnum = atoi(buf);
		
		if ( jobnum < 1 || jobnum > 3 )
		{
			send_to_char ("Which payday (1, 2 or 3) did you want to outfit this role with?\n", ch);
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ( !is_number(buf) && !str_cmp (buf, "remove") )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND payday_num = %d", role->id, jobnum);
			mysql_safe_query (buf);
			
			sprintf (buf, "Payday ##%d has been removed from the outfitting tables for the role of #6'%s'#0.", jobnum, role->summary);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		
		if ( !*buf )
		{
			send_to_char ("How many objects did you wish the payday to provide?\n", ch);
			return;
		}
		qty = atoi(buf);
		
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number(buf) )
		{
			send_to_char ("What is the VNUM of the object that this job pays in?\n", ch);
			return;
		}
		vnum = atoi(buf);
		
		if ( !vtoo(vnum) )
		{
			send_to_char ("That object does not exist.\n", ch);
			return;
		}
		
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number(buf) )
		{
			send_to_char ("How many days between pay periods should there be?\n", ch);
			return;
		}
		days = atoi(buf);
		
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number(buf) )
		{
			send_to_char ("What is the VNUM of the mob you wish to be the employer for this payday?\n", ch);
			return;
		}
		employer = atoi(buf);
		
		if ( !vnum_to_mob(employer) )
		{
			send_to_char ("An NPC of that VNUM could not be found.\n", ch);
			return;
		}
		
		if ( qty < 1 || employer < 1 || days < 1 || vnum < 1 )
		{
			send_to_char ("Positive values must be specified for this command.\n", ch);
			return;
		}
		
		sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND payday_num = %d", role->id, jobnum);
		mysql_safe_query (buf);
		
		sprintf (buf,	"INSERT INTO special_roles_outfit (role_id, payday_num, payday_obj_vnum, "
			"payday_obj_qty, payday_employer, payday_days) VALUES (%d, %d, %d, %d, %d, %d)",
			role->id, jobnum, vnum, qty, employer, days);
		mysql_safe_query (buf);
		
		sprintf (buf, "A payday for %d x #2%s#0 [%d] from #5%s#0 [%d] every %d days has been inserted "
						"as job ##%d into the outfitting tables for the role of #6'%s'#0.", qty, obj_short_desc(vtoo(vnum)),
						vnum, char_short(vnum_to_mob(employer)), employer, days, jobnum, role->summary);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}
	
	if ( !str_cmp (buf, "object") )
	{
        *skills_buf = '\0';
        *races_buf = '\0';
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number(buf) )
		{
			send_to_char ("Which object VNUM did you wish to outfit this role with?\n", ch);
			return;
		}
		vnum = atoi(buf);
		argument = one_argument (argument, buf);
		if ( *buf )
		{
			if ( is_number(buf) )
				qty = atoi(buf);
			else if ( !str_cmp (buf, "remove") )
				remove = true;
			else
			{
				send_to_char ("You must specify an object vnum, and optionally a quantity or REMOVE.\n", ch);
				return;
			}
		}
		if ( qty <= 0 || vnum <= 0 )
		{
			send_to_char ("You must specify a positive object VNUM and a quantity greater than 1.\n", ch);
			return;
		}
		if ( !vtoo(vnum) )
		{
			send_to_char ("That object VNUM does not exist in-game.\n", ch);
			return;
		}
        
        argument = one_argument (argument, buf);
        if ( *buf )
        {
            if ( (ind = index_lookup (skills, buf)) != -1 )
                sprintf (skills_buf, "%s", buf);
            else if ( (ind = lookup_race_id (buf)) != -1 )
                sprintf (races_buf, "%s", buf);
            argument = one_argument (argument, buf);
            if ( *buf )
            {
                if ( (ind = index_lookup (skills, buf)) != -1 )
                    sprintf (skills_buf, "%s", buf);
                else if ( (ind = lookup_race_id (buf)) != -1 )
                    sprintf (races_buf, "%s", buf);
            }
        }
        
		if ( remove )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND obj_vnum = %d", role->id, vnum);
			mysql_safe_query (buf);
			sprintf (buf, "All instances of #2%s#0 have been removed from the outfitting tables for the role of #6'%s'#0.",
				obj_short_desc (vtoo(vnum)), role->summary);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			sprintf (buf,   "INSERT INTO special_roles_outfit (role_id, obj_vnum, obj_qty, req_skill, "
                            "req_race) VALUES (%d, %d, %d, '%s', '%s')", role->id, vnum, qty, skills_buf, races_buf);
			mysql_safe_query (buf);
            
            sprintf (buf, "%d x #2%s#0 ha%s been added to the outfitting tables for the role of #6'%s'#0",
                    qty, obj_short_desc (vtoo(vnum)), qty != 1 ? "ve" : "s", role->summary);              

            if ( *races_buf || *skills_buf )
            {
                sprintf (buf + strlen(buf), ", for all");
                if ( *races_buf )
                    sprintf (buf + strlen(buf), " #B%s#0 characters", races_buf);
                if ( *skills_buf )
                    sprintf (buf + strlen(buf), " with skill in #B%s#0", skills_buf);
            }
            sprintf (buf + strlen(buf), ".");
            
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);			
			return;
		}
	}

	if ( !str_cmp (buf, "clan") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			send_to_char ("What clan did you wish to add to the outfitting tables for this role?\n", ch);
			return;
		}
		if ( !(clan = get_clandef (buf)) )
		{
			send_to_char ("That clan does not exist in-game.\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			send_to_char ("You must also specify a rank, or REMOVE. When in doubt, use 'member.'\n", ch);
			return;
		}
		if ( !str_cmp (buf, "remove") )
		{
			sprintf (buf2, "DELETE FROM special_roles_outfit WHERE role_id = %d AND clan_string = '%s'",
				role->id, clan->name);
			mysql_safe_query (buf2);
			
			sprintf (buf2,	"All instances of clanning for the clan #B'%s'#0 have been removed from the outfitting tables "
							"for the role of #6'%s'#0.", clan->literal, role->summary);
			act (buf2, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		if (str_cmp (buf, "member") && str_cmp (buf, "leader") && str_cmp (buf, "recruit") && str_cmp (buf, "private") && 
			str_cmp (buf, "corporal") && str_cmp (buf, "sergeant") && str_cmp (buf, "lieutenant") && str_cmp (buf, "captain") && 
			str_cmp (buf, "general") && str_cmp (buf, "commander") && str_cmp (buf, "apprentice") && str_cmp (buf, "journeyman") && 
			str_cmp (buf, "master") )
		{
			send_to_char ("That is not a recognized rank for clans.\n", ch);
			return;
		}
		sprintf (buf2, "INSERT INTO special_roles_outfit (role_id, clan_string, clan_rank) VALUES (%d, '%s', '%s')",
			role->id, clan->name, buf);
		mysql_safe_query (buf2);
		
		sprintf (buf2,	"Clanning for the rank of #B%s#0 in the clan #B'%s'#0 has been inserted into the outfitting tables "
						"for the role of #6'%s'#0.", buf, clan->literal, role->summary);
		act (buf2, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}
	
	display_outfitting_table (ch, role);
}

void
delete_role (CHAR_DATA * ch, char *argument)
{
  int i = 1;
  ROLE_DATA *role;
  bool deleted = false;

  if (!isdigit (*argument))
    {
      send_to_char ("Syntax: role delete <role number>\n", ch);
      return;
    }

  // Note: creates small mem leak, but I'm too lazy to bother free'ing right now.

  if (atoi (argument) == 1)
    {
      if (GET_TRUST (ch) == 4 && str_cmp (role_list->poster, ch->tname))
	{
	  send_to_char
	    ("Sorry, but you are only allowed to delete your own roles.\n",
	     ch);
	  return;
	}
      if (!role_list->next)
	role_list = NULL;
      else
	role_list = role_list->next;
      deleted = true;
    }
  else
    for (role = role_list, i = 1; role && role->next; role = role->next, i++)
      {
	if (i + 1 == atoi (argument))
	  {
	    if (GET_TRUST (ch) == 4
		&& str_cmp (role->next->poster, ch->tname))
	      {
		send_to_char
		  ("Sorry, but you are only allowed to delete your own roles.\n",
		   ch);
		return;
	      }
	    role->next = role->next->next;
	    deleted = true;
	    break;
	  }
      }

  if (!deleted)
    {
      send_to_char ("That role number was not found.\n", ch);
      return;
    }

  send_to_char
    ("The specified special role has been removed as an option from chargen.\n",
     ch);
  save_roles ();
}

void
display_role (CHAR_DATA * ch, char *argument)
{
  char output[MAX_STRING_LENGTH];
  int i = 1;
  ROLE_DATA *role;

  if (!isdigit (*argument))
    {
      send_to_char ("Syntax: role display <role number>\n", ch);
      return;
    }

  for (role = role_list; role; role = role->next)
    {
      if (i == atoi (argument))
	break;
      i++;
    }

  if (!role)
    {
      send_to_char ("That role number was not found.\n", ch);
      return;
    }

  sprintf (output, "\n#2Role Summary:#0  %s\n"
	   "#2Posting Admin:#0 %s\n"
	   "#2Posted On:#0     %s\n"
	   "#2Point Cost:#0    %d\n"
	   "\n"
	   "%s", role->summary, role->poster, role->date, role->cost,
	   role->body);

  page_string (ch->descr(), output);
}

void
list_roles (CHAR_DATA * ch)
{
  MYSQL_RES *result;
  char output[MAX_STRING_LENGTH];
  ROLE_DATA *role;
  int i = 1;
  bool outfit = false;

  *output = '\0';

  for (role = role_list; role; role = role->next)
    {
      mysql_safe_query ("SELECT role_id FROM special_roles_outfit WHERE role_id = %d LIMIT 1", role->id);
      result = mysql_store_result (database);
      if ( result && mysql_num_rows(result) > 0 )
        outfit = true;
      else outfit = false;
      if ( result )
        mysql_free_result (result);
      sprintf (output + strlen (output), "%s%5d. %s\n", (outfit == false) ? "#9*#0" : " ", i, role->summary);
      i++;
    }

  if (!*output)
    {
      send_to_char ("No special roles are currently available in chargen.\n",
		    ch);
      return;
    }

  sprintf (output + strlen(output), "\n#9*#0 = no outfitting directives have been created for this role,\n"
                                    "    and it will be unavailable for players to select in chargen\n");
  
  send_to_char
    ("\n#2The following special roles have been made available in chargen:\n#0",
     ch);
  page_string (ch->descr(), output);
}

void
post_role (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  ROLE_DATA *role;
  time_t current_time;
  char *date;

  ch = d->character;

  if (!*ch->pc->msg)
    {
      send_to_char ("Role post aborted.\n", ch);
      return;
    }

  if (!role_list)
    {
      CREATE (role_list, ROLE_DATA, 1);
      role = role_list;
    }
  else
    for (role = role_list; role; role = role->next)
      {
	if (!role->next)
	  {
	    CREATE (role->next, ROLE_DATA, 1);
	    role = role->next;
	    break;
	  }
      }

  /* Get a date string from current time ( default = "" ) */
  date = (char *) alloc (256, 31);
  date[0] = '\0';
  current_time = time (0);
  if (asctime_r (localtime (&current_time), date) != NULL)
    {
      date[strlen (date) - 1] = '\0';
    }

  role->cost = ch->delay_info1;
  role->summary = str_dup (ch->delay_who);
  role->body = str_dup (ch->pc->msg);
  role->date = str_dup (date);
  role->poster = str_dup (ch->pc->account_name);
  role->timestamp = (int) time (0);

  ch->delay_who = NULL;
  ch->delay_info1 = 0;

  save_roles ();

  mem_free (date);
}

void
new_role (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cost;

  argument = one_argument (argument, buf);

  if (IS_NPC (ch))
    {
      send_to_char ("Please don't post roles while switched. Thanks!\n", ch);
      return;
    }

  if (!isdigit (*buf))
    {
      send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
      return;
    }
  cost = atoi (buf);

  if (cost < 0 || cost > 50)
    {
      send_to_char ("Permissible costs are from 0-50, inclusive.\n", ch);
      return;
    }

  if (!*argument)
    {
      send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
      return;
    }

  send_to_char
    ("\n#2Enter a detailed summary of the role you wish to post, to\n"
     "give prospective players a better idea as to what sort of RP\n"
     "will be required to successfully portray it.#0\n", ch);

  make_quiet (ch);
  ch->delay_info1 = cost;
  ch->delay_who = str_dup (argument);
  ch->pc->msg = (char *) alloc (sizeof (char), 1);
  *ch->pc->msg = '\0';
  ch->descr()->str = &ch->pc->msg;
  ch->descr()->max_str = MAX_STRING_LENGTH;
  ch->descr()->proc = post_role;
}

void
update_role (CHAR_DATA * ch, char *argument)
{
  ROLE_DATA *role = NULL;
  char buf[MAX_STRING_LENGTH];
  int i = 0;

  argument = one_argument (argument, buf);

  if (!*buf || !isdigit (*buf))
    {
      send_to_char ("Please specify a role number to update.\n", ch);
      return;
    }

  if (atoi (buf) == 1)
    {
      role = role_list;
    }
  else
    for (role = role_list, i = 1; role; role = role->next, i++)
      {
	if (i == atoi (buf))
	  break;
      }

  if (!role)
    {
      send_to_char ("I could not find that role number to update.\n", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!*buf)
    {
      send_to_char ("Please specify which field in the role to update.\n",
		    ch);
      return;
    }
  else if (!str_cmp (buf, "contact"))
    {
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char
	    ("Please specify who the new point of contact should be.\n", ch);
	  return;
	}
      account *acct = new account (buf);
      if (!acct->is_registered ())
	{
	  delete acct;
	  send_to_char
	    ("That is not the name of a currently registered MUD account.\n",
	     ch);
	  return;
	}
      delete acct;
      mem_free (role->poster);
      role->poster = str_dup (CAP (buf));
      send_to_char ("The point of contact for that role has been updated.\n",
		    ch);
      save_roles ();
    }
  else if (!str_cmp (buf, "cost"))
    {
      argument = one_argument (argument, buf);
      if (!*buf || !isdigit (*buf))
	{
	  send_to_char ("Please specify what the new cost should be.\n", ch);
	  return;
	}
      role->cost = atoi (buf);
      send_to_char ("The cost for that role has been updated.\n", ch);
      save_roles ();
    }
}

void
do_role (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (IS_NPC (ch))
    {
      send_to_char ("Sorry, but this can't be done while switched.\n", ch);
      return;
    }

  if (!str_cmp (buf, "new"))
    new_role (ch, argument);
  else if (!str_cmp (buf, "list"))
    list_roles (ch);
  else if (!str_cmp (buf, "display"))
    display_role (ch, argument);
  else if (!str_cmp (buf, "outfit"))
    outfit_role (ch, argument);
  else if (!str_cmp (buf, "delete"))
    delete_role (ch, argument);
  else if (!str_cmp (buf, "update"))
    update_role (ch, argument);
  else if ( !str_cmp (buf, "save"))
  {
    send_to_char ("Saving all roles...\n", ch);
    save_roles();
  }
    else
    send_to_char
      ("Syntax: role (new|list|display|delete|update|outfit) <argument(s)>\n", ch);
}

void
save_roles (void)
{
  //MYSQL_RES *result;
  //char      text[MAX_STRING_LENGTH];
  //char      date_buf[MAX_STRING_LENGTH];
  //char      summary[MAX_STRING_LENGTH];
  ROLE_DATA *role;

  mysql_safe_query ("DELETE FROM special_roles");
  for (role = role_list; role; role = role->next)
    {
	  /*
      *summary = '\0';
      *date_buf = '\0';
      *text = '\0';
    
      mysql_safe_query ("SELECT role_id FROM special_roles WHERE role_id = %d", role->id);
      result = mysql_store_result (database);    
      
      if ( result && mysql_num_rows (result) > 0 )
      {
        mysql_safe_query
        ("UPDATE special_roles SET summary = '%s', poster = '%s', date = '%s', cost = %d, body = '%s', timestamp = %d WHERE role_id = %d", 
          role->summary, role->poster, role->date, role->cost, role->body, role->timestamp, role->id);

        mysql_free_result (result);
      }
      else
      {
        if ( result )
          mysql_free_result (result);
		  */
        mysql_safe_query
          ("INSERT INTO special_roles (summary, poster, date, cost, body, timestamp) VALUES ('%s', '%s', '%s', %d, '%s', %d)",
            role->summary, role->poster, role->date, role->cost, role->body,
            role->timestamp);
    
        role->id = mysql_insert_id(database);
      //}
    }
}

void
reload_roles (void)
{
  ROLE_DATA *role = NULL;
  MYSQL_RES *result;
  MYSQL_ROW row;

  mysql_safe_query ("SELECT * FROM special_roles");
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      system_log ("No roles stored in database.", false);
      return;
    }

  while ((row = mysql_fetch_row (result)))
    {
      if (!role_list)
	{
	  CREATE (role_list, ROLE_DATA, 1);
	  role = role_list;
	}
      else
	{
	  CREATE (role->next, ROLE_DATA, 1);
	  role = role->next;
	}
      role->summary = str_dup (row[0]);
      role->poster = str_dup (row[1]);
      role->date = str_dup (row[2]);
      role->cost = atoi (row[3]);
      role->body = str_dup (row[4]);
      role->timestamp = atoi (row[5]);
	  role->id = atoi(row[6]);
      role->next = NULL;
    }

  mysql_free_result (result);
}
