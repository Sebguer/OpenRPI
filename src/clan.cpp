/*------------------------------------------------------------------------\
|  clan.c : Clan Module                               www.middle-earth.us |
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "server.h"
#include "net_link.h"
#include "protos.h"
#include "clan.h"
#include "utils.h"
#include "utility.h"


extern rpie::server engine;

NAME_SWITCH_DATA *clan_name_switch_list = NULL;
CLAN_DATA *clan_list = NULL;

/*
+----------+----------------------------+
| group_id | group_name                 |
+----------+----------------------------+
|     1487 | Minas Morgul               | mm_denizens
|     1736 | Osgiliath City Watch       | osgi_watch
|     2008 | Malred Family              | housemalred
|     7250 | Eastern Vanguard           | eastvanguard
|     3899 | Gothakra Warband           | gothakra
|     4174 | Rogues' Fellowship         | rogues
|     4903 | Baakath-Morgul             | baakath-morgul
|     8026 | The Twisted Eye of Khagdu  | khagdu
|     8494 | Rangers of the Ithilien    | ithilien_rangers
|    11723 | Ithilien Battalion         | ithilien_battalion
|    11864 | Cult of Morgoth            | com
|    12013 | Sindbar Family             | sindbar
|    14121 | Nebavla Dunlendings        | nebavla_tribe
|    14181 | Hawk and Dove              | hawk_dove_2
|    12865 | Astirian Villeins          | astirian_villeins
+----------+----------------------------+
*/
void
	clan_forum_add (CHAR_DATA * ch, char *clan, char *rank)
{
	char buf[AVG_STRING_LENGTH * 10];

	int nGroupId = -1;

	if (IS_NPC (ch))
	{
		return;
	}

	// Add any morgulite to MM Denizens
	if (strcmp (clan, "mordor_char") == STR_MATCH)
	{
		nGroupId = 1487;
	}

	// Add Recruits+ to Military Orgs
	else if (strcmp (rank, "member") != STR_MATCH)
	{

		if (strcmp (clan, "niners") == STR_MATCH)
		{
			nGroupId = 305;
		}
		else if (strcmp (clan, "newguard") == STR_MATCH)
		{
			nGroupId = 304;
		}
		else if (strcmp (clan, "osgi_watch") == STR_MATCH)
		{
			nGroupId = 1736;
		}
		else if (strcmp (clan, "eastvanguard") == STR_MATCH)
		{
			nGroupId = 7250;
		}
		else if (strcmp (clan, "redcompany") == STR_MATCH)
		{
			nGroupId = 302;
		}
		else if (strcmp (clan, "baakath-morgul") == STR_MATCH)
		{
			nGroupId = 4903;
		}
		else if (strcmp (clan, "khagdu") == STR_MATCH)
		{
			nGroupId = 8026;
		}
		else if (strcmp (clan, "sindbar") == STR_MATCH)
		{
			nGroupId = 12013;
		}
		else if (strcmp (clan, "nebavla_tribe") == STR_MATCH)
		{
			nGroupId = 14121;
		}
		else if (strcmp (clan, "hawk_dove_2") == STR_MATCH)
		{
			nGroupId = 14181;
		}
		else if (strcmp (clan, "astirian_villeins") == STR_MATCH)
		{
			nGroupId = 12865;
		}
		// Add Privates+ to Elite Orgs
		else if (strcmp (rank, "recruit") != STR_MATCH)
		{

			if (strcmp (clan, "housemalred") == STR_MATCH)
			{
				nGroupId = 2008;
			}
			else if (strcmp (clan, "ithilien_rangers") == STR_MATCH)
			{
				nGroupId = 8494;
			}
			else if (strcmp (clan, "rouges") == STR_MATCH)
			{
				nGroupId = 4174;
			}
		}
	}
	// Avoid giving away ImmPCs in the Forums
	if (nGroupId > 0 && !GET_TRUST (ch) && !IS_SET (ch->flags, FLAG_ISADMIN))
	{
		sprintf (buf,
			"INSERT INTO forum_user_group "
			"SELECT ( %d ), user_id, ( 0 ) "
			"FROM forum_users "
			"WHERE username = '%s';", nGroupId, ch->pc->account_name);
		mysql_safe_query (buf);
	}

}

void
	clan_forum_remove (CHAR_DATA * ch, char *clan)
{
	char buf[AVG_STRING_LENGTH * 10];
	int nGroupId = -1;

	// Leave ImmPCs in their forums
	if (IS_NPC (ch) || GET_TRUST (ch) || IS_SET (ch->flags, FLAG_ISADMIN))
	{
		return;
	}

	// Add any morgulite to MM Denizens
	if (strcmp (clan, "mordor_char") == STR_MATCH)
	{
		nGroupId = 1487;
	}
	else if (strcmp (clan, "niners") == STR_MATCH)
	{
		nGroupId = 305;
	}
	else if (strcmp (clan, "newguard") == STR_MATCH)
	{
		nGroupId = 304;
	}
	else if (strcmp (clan, "redcompany") == STR_MATCH)
	{
		nGroupId = 302;
	}
	else if (strcmp (clan, "eastvanguard") == STR_MATCH)
	{
		nGroupId = 7250;
	}
	else if (strcmp (clan, "gothakra") == STR_MATCH)
	{
		nGroupId = 3899;
	}
	else if (strcmp (clan, "baakath-morgul") == STR_MATCH)
	{
		nGroupId = 4903;
	}
	else if (strcmp (clan, "khagdu") == STR_MATCH)
	{
		nGroupId = 8026;
	}
	else if (strcmp (clan, "ithilien_rangers") == STR_MATCH)
	{
		nGroupId = 8494;
	}
	else if (strcmp (clan, "malred") == STR_MATCH)
	{
		nGroupId = 2008;
	}
	else if (strcmp (clan, "rouges") == STR_MATCH)
	{
		nGroupId = 4174;
	}
	else if (strcmp (clan, "sindbar") == STR_MATCH)
	{
		nGroupId = 12013;
	}
	else if (strcmp (clan, "nebavla_tribe") == STR_MATCH)
	{
		nGroupId = 14121;
	}
	else if (strcmp (clan, "hawk_dove_2") == STR_MATCH)
	{
		nGroupId = 14181;
	}
	else if (strcmp (clan, "astirian_villeins") == STR_MATCH)
	{
		nGroupId = 12865;
	}
	else
	{
		return;
	}
	sprintf (buf,
		"DELETE FROM fug "
		"USING rpi_engine.forum_user_group fug "
		"JOIN rpi_engine.forum_users fu "
		"ON fug.user_id = fu.user_id "
		"WHERE group_id = %d and username = '%s';",
		nGroupId, ch->pc->account_name);
	mysql_safe_query (buf);
}

void
	clan_forum_remove_all (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH * 10] = "\0";
	if (!GET_TRUST (ch) && !IS_SET (ch->flags, FLAG_ISADMIN) && !IS_NPC (ch))
	{
		sprintf (buf,
			"DELETE FROM fug "
			"USING rpi_engine.forum_user_group fug "
			"JOIN rpi_engine.forum_users fu "
			"ON fug.user_id = fu.user_id "
			"WHERE group_id in (1487,1736,2008,7250,3899,3908,4174,4276,4903,8026,8494,11723,11864, 12013, 14121, 14181) "
			"AND username = '%s';", ch->pc->account_name);
		mysql_safe_query (buf);
	}
	return;
}

char *
	display_clan_ranks (CHAR_DATA * ch, CHAR_DATA * observer)
{
	CLAN_DATA *clan = NULL;
	static char buf[MAX_STRING_LENGTH] = { '\0' };
	char *argument = '\0', *argument2 = '\0';
	char ranks_list[MAX_STRING_LENGTH] = { '\0' };
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	char clan_name2[MAX_STRING_LENGTH] = { '\0' };
	char name[MAX_STRING_LENGTH] = { '\0' };
	int flags = 0, flags2 = 0, clans = 0;
	bool first = true;

	argument = observer->clans;

	*ranks_list = '\0';

	sprintf (buf, "You recognize that %s carries the rank of", HSSH (ch));

	while (get_next_clan (&argument, clan_name, &flags))
	{
		argument2 = ch->clans;
		while (get_next_clan (&argument2, clan_name2, &flags2))
		{
			if (!str_cmp (clan_name, "hules") ||
				!str_cmp (clan_name, "rustclan") ||
				!str_cmp (clan_name, "mordor_char"))
				continue;
			clan = get_clandef (clan_name);
			if (!clan)
				continue;
			if (!str_cmp (clan_name, clan_name2))
			{
				if (*ranks_list)
				{
					strcat (buf, ranks_list);
					if (!first)
						strcat (buf, ",");
					*ranks_list = '\0';
				}
				sprintf (name, "%s", clan->literal);
				if (!strn_cmp (clan->literal, "The ", 4))
					*name = tolower (*name);
				if (strn_cmp (clan->literal, "the ", 4))
					sprintf (name, "the %s", clan->literal);
				/*	        sprintf (ranks_list + strlen (ranks_list), " %s in %s",
				get_clan_rank_name (flags2), name); */
				sprintf (ranks_list + strlen (ranks_list), " %s in %s", get_clan_rank_name (ch, clan_name, flags2), name);
				first = false;
				clans++;
			}
		}
	}

	if (*ranks_list)
	{
		if (clans > 1)
			strcat (buf, " and");
		strcat (buf, ranks_list);
	}

	strcat (buf, ".");

	if (clans)
		return buf;
	else
		return "";
}


void
	add_clandef (char *argument)
{
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];

	clan = (CLAN_DATA *) alloc (sizeof (CLAN_DATA), 39);

	argument = one_argument (argument, buf);
	clan->name = str_dup (buf);

	argument = one_argument (argument, buf);
	clan->zone = atoi (buf);

	argument = one_argument (argument, buf);
	clan->literal = str_dup (buf);

	argument = one_argument (argument, buf);
	clan->member_vnum = atoi (buf);

	argument = one_argument (argument, buf);
	clan->leader_vnum = atoi (buf);

	argument = one_argument (argument, buf);
	clan->omni_vnum = atoi (buf);

	clan->next = clan_list;
	clan_list = clan;
}

int
	is_brother (CHAR_DATA * ch, CHAR_DATA * tch)
{
	int flags;
	char *c1;
	char clan_name[MAX_STRING_LENGTH];

	// If we're confused, we don't consider anyone our brother
	//if(IS_SET (ch->act, ACT_AGGRESSIVE) && get_soma_affect(ch, SOMA_CHEM_CONFUSE) && (lookup_race_int(ch->race, RACE_NOMAD)))
	//  return 0;

	for (c1 = ch->clans; get_next_clan (&c1, clan_name, &flags);)
	{

		if (get_clan (tch, clan_name, &flags))
			return 1;
	}

	return 0;
}

int
	is_leader (CHAR_DATA * src, CHAR_DATA * tar)
{
	int c1_flags;
	int c2_flags;
	char *c1;
	char clan_name[MAX_STRING_LENGTH];

	for (c1 = src->clans; get_next_clan (&c1, clan_name, &c1_flags);)
	{

		if (get_clan (tar, clan_name, &c2_flags) &&
			!IS_SET (c2_flags, CLAN_LEADER) &&
			!IS_SET (c2_flags, CLAN_LEADER_OBJ) &&
			(c1_flags > c2_flags || IS_SET (c1_flags, CLAN_LEADER)))
			return 1;
	}

	if (!IS_NPC (src) && IS_NPC (tar))
	{
		if (tar->mob->owner && !str_cmp (src->tname, tar->mob->owner))
			return 1;
	}

	return 0;
}


void
	add_clan_id_string (char *string, char *clan_name, char *clan_flags)
{
	NAME_SWITCH_DATA *nsp;
	char *argument;
	char buf[MAX_STRING_LENGTH];

	/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
		if (!str_cmp (nsp->old_name, clan_name))
			clan_name = nsp->new_name;

	argument = string;

	while (1)
	{

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if (!*buf)
			break;

		if (!str_cmp (buf, clan_name))
			return;
	}

	/* string doesn't have that clan, add it */

	if (string && *string)
		sprintf (buf, "%s '%s' %s", string, clan_flags, clan_name);
	else
		sprintf (buf, "'%s' %s", clan_flags, clan_name);

	mem_free (string);

	string = str_dup (buf);
}

void
	add_clan_id (CHAR_DATA * ch, char *clan_name, const char *clan_flags)
{
	NAME_SWITCH_DATA *nsp;
	char *argument;
	char buf[MAX_STRING_LENGTH];

	/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
		if (!str_cmp (nsp->old_name, clan_name))
			clan_name = nsp->new_name;

	argument = ch->clans;

	while (1)
	{

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if (!*buf)
			break;

		if (!str_cmp (buf, clan_name))
			return;
	}

	/* ch doesn't have that clan, add it */

	if (ch->clans && *ch->clans)
		sprintf (buf, "%s '%s' %s", ch->clans, clan_flags, clan_name);
	else
		sprintf (buf, "'%s' %s", clan_flags, clan_name);

	mem_free (ch->clans);

	ch->clans = str_dup (buf);
}

int
	is_clan_member (CHAR_DATA * ch, char *clan_name)
{
	int clan_flags;

	if (get_clan (ch, clan_name, &clan_flags))
		return 1;

	return 0;
}

/* the _player version of this routine is called to parse a clan name
a player might type in.  If clan is registered, then only the full
name is valid.  If it is not, the short name is appropriate.
*/

int
	is_clan_member_player (CHAR_DATA * ch, char *clan_name)
{
	CLAN_DATA *clan;

	if (!clan_name)
		return 0;

	if (is_clan_member (ch, clan_name))
	{				/* Short name match */
		if (get_clandef (clan_name))
			return 0;		/* Supplied short name when long existed */
		return 1;
	}

	if (!(clan = get_clandef_long (clan_name)))
		return 0;

	return is_clan_member (ch, clan->name);
}

void
	clan_object_equip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int clan_flags;
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{
		if (clan->member_vnum == obj->nVirtual ||
			clan->leader_vnum == obj->nVirtual)
			break;
	}

	if (!clan)
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if (clan->member_vnum == obj->nVirtual)
		clan_flags |= CLAN_MEMBER_OBJ;

	if (clan->leader_vnum == obj->nVirtual)
		clan_flags |= CLAN_LEADER_OBJ;

	add_clan (ch, clan->name, clan_flags);
}

void
	clan_object_unequip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int clan_flags;
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{
		if (clan->member_vnum == obj->nVirtual ||
			clan->leader_vnum == obj->nVirtual)
			break;
	}

	if (!clan)
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if (clan->member_vnum == obj->nVirtual)
		clan_flags &= ~CLAN_MEMBER_OBJ;

	if (clan->leader_vnum == obj->nVirtual)
		clan_flags &= ~CLAN_LEADER_OBJ;

	if (clan_flags)
		add_clan (ch, clan->name, clan_flags);
}

void
	do_clan (CHAR_DATA * ch, char *argument, int cmd)
{
	int zone;
	int leader_obj_vnum;
	int member_obj_vnum;
	int new_flags;
	int clan_flags;
	int num_clans = 0;
	int i;
	NAME_SWITCH_DATA *name_switch;
	NAME_SWITCH_DATA *nsp;
	OBJ_DATA *obj;
	CLAN_DATA *clan;
	CLAN_DATA *delete_clan;
	CHAR_DATA *edit_mob = NULL;
	CHAR_DATA *tch;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];
	char oldname[MAX_STRING_LENGTH];
	char newname[MAX_STRING_LENGTH];
	char literal[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char *the_clans[2000];

	if (IS_NPC (ch))
	{
		send_to_char ("Sorry, but you can't do this while switched...\n", ch);
		return;
	}

	if (!*argument)
	{

		send_to_char ("Old clan name   ->  New clan name\n", ch);
		send_to_char ("=============       =============\n", ch);

		if (!clan_name_switch_list)
			send_to_char ("No clans set to be renamed.\n", ch);

		else
		{
			for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
			{
				sprintf (buf, "%-15s     %-15s\n", nsp->old_name,
					nsp->new_name);
				send_to_char (buf, ch);
			}
		}

		sprintf (buf,"\nClan Name        Full Clan Name\n");
		sprintf (buf + strlen(buf),"===============  =================================\n");

		for (clan = clan_list; clan; clan = clan->next)
		{
			sprintf (buf + strlen(buf), "%-15s  %s\n", clan->name, clan->literal);

			if (clan->zone)
			{
				sprintf (buf + strlen(buf), "                 Enforcement Zone %d\n\n",
					clan->zone);
			}

			if (clan->member_vnum)
			{
				obj = vtoo (clan->member_vnum);
				sprintf (buf + strlen(buf), "                 Member Object (%05d):  %s\n",
					clan->member_vnum,
					obj ? obj->short_description : "UNDEFINED");
			}

			if (clan->leader_vnum)
			{
				obj = vtoo (clan->leader_vnum);
				sprintf (buf + strlen(buf), "                 Leader Object (%05d):  %s\n",
					clan->leader_vnum,
					obj ? obj->short_description : "UNDEFINED");
			}
		}
		page_string (ch->descr(), buf);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("Syntax for modifying PCs:\n\n", ch);
		send_to_char ("   clan set <clan-name> [<clan-flags>]\n", ch);
		send_to_char ("   clan remove <clan-name>         (or all)\n", ch);
		send_to_char ("\nSyntax for modifying clan definitions:\n", ch);
		send_to_char ("   clan rename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan unrename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan register <name> <enforcement zone> <long name> "
			"[<leader obj>] [<member obj>]\n", ch);
		send_to_char ("   clan unregister <name>\n", ch);
		send_to_char ("   clan list\n", ch);
		send_to_char ("\nThe obj vnums are optional.  Specify zone 0 if no "
			"enforcement zone.\n\nExamples:\n", ch);
		send_to_char ("  > clan set osgilwatch member leader\n", ch);
		send_to_char ("  > clan remove osgilwatch\n", ch);
		send_to_char ("  > clan register osgilwatch 10 'Osgiliath Watch'\n", ch);
		send_to_char ("  > clan unregister osgilwatch\n", ch);
		send_to_char ("  > clan rename 10 osgilwatch\n", ch);
		send_to_char ("  > clan unrename 10 osgilwatch\n", ch);
		send_to_char ("  > clan list\n", ch);
		return;
	}

	if (!str_cmp (buf, "list"))
	{

		for (tch = character_list; tch; tch = tch->next)
		{

			if (tch->deleted)
				continue;

			p = tch->clans;

			if (!p || !*p)
				continue;

			while (get_next_clan (&p, clan_name, &clan_flags))
			{

				if (num_clans >= 1999)	/* Too many in list */
					break;

				for (i = 0; i < num_clans; i++)
					if (!str_cmp (clan_name, the_clans[i]))
						break;

				if (i >= num_clans)
					the_clans[num_clans++] = str_dup (clan_name);
			}

		}

		*b_buf = '\0';

		for (i = 0; i < num_clans; i++)
		{

			sprintf (b_buf + strlen (b_buf), "%-15s", the_clans[i]);

			if (i % 5 == 4)
				strcat (b_buf, "\n");
			else
				strcat (b_buf, " ");
		}

		page_string (ch->descr(), b_buf);

		return;
	}

	if (!str_cmp (buf, "set") ||
		!str_cmp (buf, "remove") || !str_cmp (buf, "delete"))
	{

		if (IS_NPC (ch))
		{
			send_to_char ("This command cannot be used while switched.\n", ch);
			return;
		}

		if ((!ch->pc->edit_mob && !ch->pc->edit_player) ||
			(!(edit_mob = vnum_to_mob (ch->pc->edit_mob)) &&
			!(edit_mob = ch->pc->edit_player)))
		{
			send_to_char ("Start by using the MOBILE command.\n", ch);
			return;
		}

		if (IS_SET (edit_mob->flags, FLAG_VARIABLE))
		{
			send_to_char("Don't clan this mob - you need to remove the variable flag first.\n", ch);
			return;
		}
	}

	if (!str_cmp (buf, "set"))
	{

		argument = one_argument (argument, clan_name);

		if (!*clan_name)
		{
			send_to_char ("Expected a clan name after 'set'.\n", ch);
			return;
		}

		new_flags = clan_flags_to_value (argument);

		if (get_clan (edit_mob, clan_name, &clan_flags))
			remove_clan (edit_mob, clan_name);

		clan_flags = 0;

		while (1)
		{

			argument = one_argument (argument, buf);

			if (!*buf)
				break;

			if (!str_cmp (buf, "member"))
			{
				TOGGLE (clan_flags, CLAN_MEMBER);
			}			/* {}'s define     */
			else if (!str_cmp (buf, "leader"))	/* so no ; needed  */
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
			else
			{
				sprintf (literal, "Flag %s is unknown for clans.\n", buf);
				send_to_char (literal, ch);
			}
		}

		if (!clan_flags)
			clan_flags = CLAN_MEMBER;

		add_clan (edit_mob, clan_name, clan_flags);

		if (edit_mob->mob)
		{
			sprintf (buf, "%d mobile(s) in world redefined.\n",
				redefine_mobiles (edit_mob));
			send_to_char (buf, ch);
		}


		return;			/* Return from n/pc specific uses of clan */
	}

	else if (!str_cmp (buf, "remove") || !str_cmp (buf, "delete"))
	{

		argument = one_argument (argument, clan_name);

		if (!*clan_name)
		{
			send_to_char ("Expected a clan name to remove from n/pc.\n", ch);
			return;
		}

		if (!str_cmp (clan_name, "all"))
		{
			mem_free (ch->clans);
			ch->clans = str_dup ("");
		}

		else if (!get_clan (edit_mob, clan_name, &clan_flags))
		{
			send_to_char ("N/PC doesn't have that clan.\n", ch);
			return;
		}

		remove_clan (edit_mob, clan_name);

		if (edit_mob->mob)
		{
			sprintf (buf, "%d mobile(s) in world redefined.\n",
				redefine_mobiles (edit_mob));
			send_to_char (buf, ch);
		}

		return;			/* Return from n/pc specific uses of clan */
	}

	else if (!str_cmp (buf, "unregister"))
	{

		argument = one_argument (argument, clan_name);

		if (!clan_list)
		{
			send_to_char ("There are no registered clans.\n", ch);
			return;
		}

		if (!str_cmp (clan_list->name, clan_name))
		{
			delete_clan = clan_list;
			clan_list = clan_list->next;
		}

		else
		{
			delete_clan = NULL;

			for (clan = clan_list; clan->next; clan = clan->next)
			{
				if (!str_cmp (clan->next->name, clan_name))
				{
					delete_clan = clan->next;
					clan->next = delete_clan->next;
					break;
				}
			}

			if (!delete_clan)
			{
				send_to_char ("No such registered clan name.\n", ch);
				return;
			}
		}
		clan__do_remove (delete_clan);

		mem_free (delete_clan->name);
		mem_free (delete_clan->literal);
		mem_free (delete_clan);
	}

	else if (!str_cmp (buf, "rename"))
	{

		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if (!*oldname || !*newname)
		{
			send_to_char ("rename <oldclanname> <newclanname>\n", ch);
			return;
		}

		name_switch =
			(struct name_switch_data *) alloc (sizeof (struct name_switch_data),
			38);
		name_switch->old_name = str_dup (oldname);
		name_switch->new_name = str_dup (newname);

		if (!clan_name_switch_list)
			clan_name_switch_list = name_switch;
		else
		{
			for (nsp = clan_name_switch_list; nsp->next; nsp = nsp->next)
				;
			nsp->next = name_switch;
		}
	}

	else if (!str_cmp (buf, "unrename"))
	{
		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if (!*oldname || !*newname)
		{
			send_to_char ("clan unrename <oldclanname> <newclanname>\n", ch);
			send_to_char ("This command deletes a rename entry.\n", ch);
			return;
		}

		if (!str_cmp (clan_name_switch_list->old_name, oldname) &&
			!str_cmp (clan_name_switch_list->new_name, newname))
		{
			mem_free (clan_name_switch_list->old_name);
			mem_free (clan_name_switch_list->new_name);
			nsp = clan_name_switch_list;
			clan_name_switch_list = nsp->next;
			mem_free (nsp);
			send_to_char ("Rename entry deleted.\n", ch);
			write_dynamic_registry (ch);
			return;
		}

		for (nsp = clan_name_switch_list; nsp->next; nsp = nsp->next)
			if (!str_cmp (nsp->next->old_name, oldname) &&
				!str_cmp (nsp->next->new_name, newname))
				break;

		if (!nsp->next)
		{
			send_to_char ("Sorry, no such rename entry pair found.\n", ch);
			return;
		}

		name_switch = nsp->next;
		nsp->next = name_switch->next;

		mem_free (name_switch->old_name);
		mem_free (name_switch->new_name);
		mem_free (name_switch);

		send_to_char ("Rename entry deleted.\n", ch);
	}

	else if (!str_cmp (buf, "register"))
	{

		argument = one_argument (argument, name);

		if (get_clandef (name))
		{
			send_to_char ("That clan has already been registered.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if (!*name || !*buf || !isdigit (*buf))
		{
			send_to_char ("Type clan ? for syntax.\n", ch);
			return;
		}

		zone = atoi (buf);

		argument = one_argument (argument, literal);

		if (!*literal)
		{
			send_to_char ("Syntax error parsing literal, type clan ? for "
				"syntax.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if (!*buf)
		{
			leader_obj_vnum = 0;
		}
		else if (!isdigit (*buf))
		{
			send_to_char ("Syntax error.  Did you enclose the long name in "
				"quotes?  Type clan ?.\n", ch);
			return;
		}
		else
		{
			leader_obj_vnum = atoi (buf);
			if (leader_obj_vnum && !vtoo (leader_obj_vnum))
				send_to_char ("NOTE:  Leader object doesn't currently "
				"exist.\n", ch);
		}

		argument = one_argument (argument, buf);

		if (!*buf)
		{
			member_obj_vnum = 0;
		}
		else if (!isdigit (*buf))
		{
			send_to_char ("Syntax error parsing member object.  Type clan ? "
				"for syntax.\n", ch);
			return;
		}
		else
		{
			member_obj_vnum = atoi (buf);
			if (member_obj_vnum && !vtoo (member_obj_vnum))
				send_to_char ("NOTE:  Member object doesn't currently "
				"exist.\n", ch);
		}

		clan = (CLAN_DATA *) alloc (sizeof (CLAN_DATA), 39);

		clan->name = str_dup (name);
		clan->zone = zone;
		clan->literal = str_dup (literal);
		clan->member_vnum = member_obj_vnum;
		clan->leader_vnum = leader_obj_vnum;
		clan->omni_vnum = 0;

		clan__do_add (clan);

		clan->next = clan_list;
		clan_list = clan;
	}

	else
	{
		send_to_char ("What do you want to do again?  Type clan ? for help.\n",
			ch);
		return;
	}

	write_dynamic_registry (ch);
}

// Overloaded clan_flags_to_value for use in do_promote
int
	clan_flags_to_value (char *flag_names, char *clan_name)
{
	int flags = 0;
	char buf[MAX_STRING_LENGTH];

	while (1)
	{
		flag_names = one_argument (flag_names, buf);

		if (!*buf)
			break;

		if (!str_cmp (buf, "member"))
			flags |= CLAN_MEMBER;
		else if (!str_cmp (buf, "leader"))
			flags |= CLAN_LEADER;
		else if (!str_cmp (buf, "memberobj"))
			flags |= CLAN_MEMBER_OBJ;
		else if (!str_cmp (buf, "leaderobj"))
			flags |= CLAN_LEADER_OBJ;

		else if (!str_cmp (buf, "recruit")
			|| ((!str_cmp (buf, "snaga") || !str_cmp (buf, "snaga-uruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "initiate"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp (buf, "kaal")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "squire")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "push-khur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "roenucht")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "liegeman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))) )
			flags |= CLAN_RECRUIT;

		else if (!str_cmp (buf, "private")
			|| ((!str_cmp (buf, "acolyte"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp(buf, "ohtar")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "uruk") || !str_cmp (buf, "high-snaga"))
			&& !str_cmp (clan_name, "gothakra"))
			|| (!str_cmp (buf, "rukh-kaal")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "apprentice-seeker-knight")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "khur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "heggurach")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "footman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_PRIVATE;

		else if (!str_cmp (buf, "corporal")
			|| (!str_cmp(buf, "roquen")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "dark-priest"))
			&& !str_cmp (clan_name, "shadow-cult"))
			||  ((!str_cmp (buf, "puruk") || !str_cmp (buf, "zuruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| (!str_cmp (buf, "rukh")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "seeker-knight")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "rhyfelwr")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "armsman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_CORPORAL;

		else if (!str_cmp (buf, "sergeant")
			|| ((!str_cmp (buf, "zaak") || !str_cmp (buf, "high-puruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "dread-minister "))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| ((!str_cmp (buf, "mik") || !str_cmp (buf, "mith"))
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-lieutenant")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gurash")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "cloumaggen")
			&& !str_cmp (clan_name, "nebavla_tribe"))	)
			flags |= CLAN_SERGEANT;

		else if (!str_cmp (buf, "lieutenant")
			|| (!str_cmp(buf, "constable")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "overlord"))
			&& !str_cmp (clan_name, "shadow-cult"))
			||  ((!str_cmp (buf, "puruk-zuul") || !str_cmp (buf, "ba'zaak"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "amme") || !str_cmp (buf, "atto"))
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-captain")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "jomaa")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "constable")
			&& (!str_cmp (clan_name, "eradan_battalion")
			|| !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_LIEUTENANT;

		else if (!str_cmp (buf, "captain")
			|| (!str_cmp (buf, "barun")
			&& !str_cmp (clan_name, "com"))
			|| ((!str_cmp (buf, "barun-an-Nalo"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp (buf, "knight-general")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gurashul")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "clowgos")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "lord")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_CAPTAIN;

		else if (!str_cmp (buf, "general")
			|| (!str_cmp(buf, "marshall")
			&& !str_cmp(clan_name, "tirithguard"))
			||  (!str_cmp (buf, "nalo-barun")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-commander")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "rhi")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "bughrak")
			&& !str_cmp (clan_name, "khagdu")) )
			flags |= CLAN_GENERAL;

		else if (!str_cmp (buf, "commander")
			|| (!str_cmp (buf, "daur-phazan")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-grand-cross")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "duumul-bughrak")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "Jarl")
			&& !str_cmp (clan_name, "nebavla_tribe")))
			flags |= CLAN_COMMANDER;

		else if (!str_cmp (buf, "apprentice") || (!str_cmp (buf, "apprentice-")) )
			flags |= CLAN_APPRENTICE;

		else if (!str_cmp (buf, "journeyman")
			|| (!str_cmp (buf, "yameg")
			&& !str_cmp (clan_name, "khagdu")) )
			flags |= CLAN_JOURNEYMAN;

		else if (!str_cmp (buf, "master")
			|| (!str_cmp (buf, "yameg-khur")
			&& !str_cmp (clan_name, "khagdu")) )
			flags |= CLAN_MASTER;
	}

	return flags;
}



int
	clan_flags_to_value (char *flag_names)
{
	int flags = 0;
	char buf[MAX_STRING_LENGTH];

	while (1)
	{
		flag_names = one_argument (flag_names, buf);

		if (!*buf)
			break;

		if (!str_cmp (buf, "member"))
			flags |= CLAN_MEMBER;
		else if (!str_cmp (buf, "leader"))
			flags |= CLAN_LEADER;
		else if (!str_cmp (buf, "memberobj"))
			flags |= CLAN_MEMBER_OBJ;
		else if (!str_cmp (buf, "leaderobj"))
			flags |= CLAN_LEADER_OBJ;
		else if (!str_cmp (buf, "recruit"))
			flags |= CLAN_RECRUIT;
		else if (!str_cmp (buf, "private"))
			flags |= CLAN_PRIVATE;
		else if (!str_cmp (buf, "corporal"))
			flags |= CLAN_CORPORAL;
		else if (!str_cmp (buf, "sergeant"))
			flags |= CLAN_SERGEANT;
		else if (!str_cmp (buf, "lieutenant"))
			flags |= CLAN_LIEUTENANT;
		else if (!str_cmp (buf, "captain"))
			flags |= CLAN_CAPTAIN;
		else if (!str_cmp (buf, "general"))
			flags |= CLAN_GENERAL;
		else if (!str_cmp (buf, "commander"))
			flags |= CLAN_COMMANDER;
		else if (!str_cmp (buf, "apprentice"))
			flags |= CLAN_APPRENTICE;
		else if (!str_cmp (buf, "journeyman"))
			flags |= CLAN_JOURNEYMAN;
		else if (!str_cmp (buf, "master"))
			flags |= CLAN_MASTER;
	}

	return flags;
}

char *
	value_to_clan_flags (int flags)
{
	*s_buf = '\0';

	if (IS_SET (flags, CLAN_LEADER))
		strcat (s_buf, "leader ");

	if (IS_SET (flags, CLAN_MEMBER))
		strcat (s_buf, "member ");

	if (IS_SET (flags, CLAN_RECRUIT))
		strcat (s_buf, "recruit ");

	if (IS_SET (flags, CLAN_PRIVATE))
		strcat (s_buf, "private ");

	if (IS_SET (flags, CLAN_CORPORAL))
		strcat (s_buf, "corporal ");

	if (IS_SET (flags, CLAN_SERGEANT))
		strcat (s_buf, "sergeant ");

	if (IS_SET (flags, CLAN_LIEUTENANT))
		strcat (s_buf, "lieutenant ");

	if (IS_SET (flags, CLAN_CAPTAIN))
		strcat (s_buf, "captain ");

	if (IS_SET (flags, CLAN_GENERAL))
		strcat (s_buf, "general");

	if (IS_SET (flags, CLAN_COMMANDER))
		strcat (s_buf, "commander ");

	if (IS_SET (flags, CLAN_APPRENTICE))
		strcat (s_buf, "apprentice ");

	if (IS_SET (flags, CLAN_JOURNEYMAN))
		strcat (s_buf, "journeyman ");

	if (IS_SET (flags, CLAN_MASTER))
		strcat (s_buf, "master ");

	if (IS_SET (flags, CLAN_LEADER_OBJ))
		strcat (s_buf, "leaderobj ");

	if (IS_SET (flags, CLAN_MEMBER_OBJ))
		strcat (s_buf, "memberobj ");

	if (*s_buf && s_buf[strlen (s_buf) - 1] == ' ')
		s_buf[strlen (s_buf) - 1] = '\0';

	return s_buf;
}

char *
	remove_clan_from_string (char *string, char *old_clan_name)
{
	char *argument;
	static char buf[MAX_STRING_LENGTH];
	char clan_flags[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!*string)
		return NULL;

	argument = string;

	*buf = '\0';

	while (1)
	{
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (str_cmp (clan_name, old_clan_name))
			sprintf (buf + strlen (buf), "'%s' %s ", clan_flags, clan_name);
	}

	mem_free (string);
	string = NULL;

	if (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	return buf;
}

char *
	add_clan_to_string (char *string, char *new_clan_name, int clan_flags)
{
	char *argument;
	char clan_name[MAX_STRING_LENGTH];
	char flag_names[MAX_STRING_LENGTH];
	static char buf[MAX_STRING_LENGTH];

	argument = string;

	/* Look to see if we're just changing flags */

	while (1)
	{

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (!str_cmp (clan_name, new_clan_name))
		{
			remove_clan_from_string (string, new_clan_name);
			break;
		}
	}

	/* Clan is new */

	if (string && *string)
		sprintf (buf, "'%s' %s %s",
		value_to_clan_flags (clan_flags), new_clan_name, string);
	else
		sprintf (buf, "'%s' %s", value_to_clan_flags (clan_flags), new_clan_name);

	while (buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (string && *string)
	{
		mem_free (string);
		string = NULL;
	}

	return buf;
}

void
	remove_clan (CHAR_DATA * ch, char *old_clan_name)
{
	char *argument;
	char buf[MAX_STRING_LENGTH];
	char clan_flags[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!ch->clans)
		return;

	argument = ch->clans;

	*buf = '\0';

	while (1)
	{
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (str_cmp (clan_name, old_clan_name))
			sprintf (buf + strlen (buf), "'%s' %s ", clan_flags, clan_name);
	}

	if (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (ch->clans && *ch->clans != '\0')
		mem_free (ch->clans);

	ch->clans = add_hash (buf);
	clan_forum_remove (ch, old_clan_name);
}

void
	add_clan (CHAR_DATA * ch, char *new_clan_name, int clan_flags)
{
	char *argument;
	char clan_name[MAX_STRING_LENGTH];
	char flag_names[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = ch->clans;

	/* Look to see if we're just changing flags */

	while (1)
	{

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (!str_cmp (clan_name, new_clan_name))
		{
			remove_clan (ch, new_clan_name);
			break;
		}
	}

	/* Clan is new */
	clan_forum_add (ch, new_clan_name, value_to_clan_flags (clan_flags));
	sprintf (buf, "'%s' %s %s", value_to_clan_flags (clan_flags), new_clan_name, ch->clans);

	while (buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (ch->clans && *ch->clans != '\0')
		mem_free (ch->clans);

	ch->clans = add_hash (buf);
}

int
	get_next_clan (char **p, char *clan_name, int *clan_flags)
{
	char flag_names[MAX_STRING_LENGTH];

	*p = one_argument (*p, flag_names);

	*clan_flags = clan_flags_to_value (flag_names);

	*p = one_argument (*p, clan_name);

	if (!*clan_name)
		return 0;

	return 1;
}

char *
	get_shared_clan_rank (CHAR_DATA * ch, CHAR_DATA * observer)
{
	char *argument, *argument2;
	char clan_name[MAX_STRING_LENGTH], clan_name2[MAX_STRING_LENGTH];
	int flags = 0, flags2 = 0, highest_rank = 0;

	argument = observer->clans;

	while (get_next_clan (&argument, clan_name, &flags))
	{
		argument2 = ch->clans;
		while (get_next_clan (&argument2, clan_name2, &flags2))
		{
			if (!str_cmp (clan_name, "osgi_citizens") ||
				!str_cmp (clan_name, "mordor_char"))
				continue;
			if (!str_cmp (clan_name, clan_name2))
			{
				if (flags2 > highest_rank)
					highest_rank = flags2;
			}
		}
	}

	flags2 = highest_rank;

	if (flags2 == CLAN_LEADER)
		return "Leadership";
	else if (flags2 == CLAN_RECRUIT)
		return "Recruit";
	else if (flags2 == CLAN_PRIVATE)
		return "Private";
	else if (flags2 == CLAN_CORPORAL)
		return "Corporal";
	else if (flags2 == CLAN_SERGEANT)
		return "Sergeant";
	else if (flags2 == CLAN_LIEUTENANT)
		return "Lieutenant";
	else if (flags2 == CLAN_CAPTAIN)
		return "Captain";
	else if (flags2 == CLAN_GENERAL)
		return "General";
	else if (flags2 == CLAN_COMMANDER)
		return "Commander";
	else if (flags2 == CLAN_APPRENTICE)
		return "Apprentice";
	else if (flags2 == CLAN_JOURNEYMAN)
		return "Journeyman";
	else if (flags2 == CLAN_MASTER)
		return "Master";
	else if (flags2 > 0)
		return "Membership";

	return NULL;
}

char *
	get_clan_rank_name (int flags)
{
	if (flags == CLAN_LEADER)
		return "Leadership";
	else if (flags == CLAN_RECRUIT)
		return "Recruit";
	else if (flags == CLAN_PRIVATE)
		return "Private";
	else if (flags == CLAN_CORPORAL)
		return "Corporal";
	else if (flags == CLAN_SERGEANT)
		return "Sergeant";
	else if (flags == CLAN_LIEUTENANT)
		return "Lieutenant";
	else if (flags == CLAN_CAPTAIN)
		return "Captain";
	else if (flags == CLAN_GENERAL)
		return "General";
	else if (flags == CLAN_COMMANDER)
		return "Commander";
	else if (flags == CLAN_APPRENTICE)
		return "Apprentice";
	else if (flags == CLAN_JOURNEYMAN)
		return "Journeyman";
	else if (flags == CLAN_MASTER)
		return "Master";
	else if (flags > 0)
		return "Membership";

	return NULL;
}

char *
	get_clan_rank_name (CHAR_DATA *ch, char * clan, int flags)
{

	//Leader
	if (flags == CLAN_LEADER)
	{
		return "Leadership";
	}

	//Recruit
	else if (flags == CLAN_RECRUIT)
	{
		return "Recruit";
	}

	//Private
	else if (flags == CLAN_PRIVATE)
	{
		return "Private";
	}

	//Corporal
	else if (flags == CLAN_CORPORAL)
	{
		return "Corporal";
	}

	//Sergeant
	else if (flags == CLAN_SERGEANT)
	{
		return "Sergeant";
	}

	//lieutenant
	else if (flags == CLAN_LIEUTENANT)
	{
		return "Lieutenant";
	}

	//Captain
	else if (flags == CLAN_CAPTAIN)
	{
		return "Captain";
	}

	//General
	else if (flags == CLAN_GENERAL)
	{
		return "General";
	}

	//Commander
	else if (flags == CLAN_COMMANDER)
	{
		return "Commander";
	}

	//Apprentice
	else if (flags == CLAN_APPRENTICE)
	{
		return "Apprentice";
	}

	//Journeyman
	else if (flags == CLAN_JOURNEYMAN)
	{
		return "Journeyman";
	}

	//Master
	else if (flags == CLAN_MASTER)
	{
		return "Master";
	}

	else if (flags = CLAN_MEMBER)
	{
		return "Member";
	}


	return NULL;
}


int
	get_clan_in_string (char *string, char *clan, int *clan_flags)
{
	int flags;
	char *argument;
	char clan_name[MAX_STRING_LENGTH];

	argument = string;

	while (get_next_clan (&argument, clan_name, &flags))
	{

		if (!str_cmp (clan_name, clan))
		{
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int
	get_clan (CHAR_DATA * ch, const char *clan, int *clan_flags)
{
	int flags;
	char *argument;
	char clan_name[MAX_STRING_LENGTH];

	if (!ch->clans)
		return 0;

	argument = ch->clans;

	while (get_next_clan (&argument, clan_name, &flags))
	{

		if (!str_cmp (clan_name, clan))
		{
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int
	get_clan_long (CHAR_DATA * ch, char *clan_name, int *clan_flags)
{
	CLAN_DATA *clan;

	if (!(clan = get_clandef_long (clan_name)))
		return 0;

	if (!get_clan (ch, clan->name, clan_flags))
		return 0;

	return 1;
}

int
	get_clan_long_short (CHAR_DATA * ch, char *clan_name, int *clan_flags)
{
	if (get_clan_long (ch, clan_name, clan_flags))
		return 1;

	return get_clan (ch, clan_name, clan_flags);
}

int
	get_next_leader (char **p, char *clan_name, int *clan_flags)
{
	char flag_names[MAX_STRING_LENGTH];

	while (1)
	{

		*p = one_argument (*p, flag_names);

		*clan_flags = clan_flags_to_value (flag_names);

		*p = one_argument (*p, clan_name);

		if (!*clan_name)
			return 0;

		if (IS_SET (*clan_flags, CLAN_LEADER) ||
			IS_SET (*clan_flags, CLAN_LEADER_OBJ) ||
			(*clan_flags >= CLAN_SERGEANT && *clan_flags <= CLAN_COMMANDER) ||
			IS_SET (*clan_flags, CLAN_MASTER))
			break;
	}

	return 1;
}

CLAN_DATA *
	get_clandef (const char *clan_name)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
		if (!str_cmp (clan->name, clan_name))
			return clan;

	return NULL;
}

CLAN_DATA *
	get_clandef_long (char *clan_long_name)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
		if (!str_cmp (clan->literal, clan_long_name))
			return clan;

	return NULL;
}

CLAN_DATA *
	get_clanid (int clan_id)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
		if (clan_id == clan->id)
			return clan;

	return NULL;
}



void
	update_enforcement_array (CHAR_DATA * ch)
{
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH];

	p = ch->clans;

	while (get_next_clan (&p, clan_name, &flags))
	{
		if ((clan_def = get_clandef (clan_name)) && clan_def->zone)
		{
			ch->enforcement[clan_def->zone] = true;

			if (clan_def->zone == 56 ||
				clan_def->zone == 75 ||
				clan_def->zone == 67)
			{
				ch->enforcement[56] = true;
				ch->enforcement[75] = true;
				ch->enforcement[67] = true;
			}
		}
	}

	ch->enforcement[0] = true;
}

int
	is_area_leader (CHAR_DATA * ch)
{
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH];

	p = ch->clans;

	while (get_next_leader (&p, clan_name, &flags))
	{
		if (!(clan_def = get_clandef (clan_name)))
			continue;

		if (clan_def->zone != 0 && ch->room->zone == clan_def->zone)
			return 1;
	}

	return 0;
}

void
	do_assign (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char admin[MAX_STRING_LENGTH];
	char clan[MAX_STRING_LENGTH];

	if (!*argument)
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (str_cmp (buf, "add") && str_cmp (buf, "remove"))
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, admin);
	argument = one_argument (argument, buf2);

	sprintf (clan, "%s", buf2);

	if (!*admin)
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	if (islower (*admin))
		*admin = toupper (*admin);

	if (*clan && islower (*clan))
		*clan = toupper (*clan);

	if (!str_cmp (buf, "add"))
	{
		mysql_safe_query ("INSERT INTO clan_assignments VALUES ('%s', '%s')",
			clan, admin);
		send_to_char
			("The specified assignment has been added to the database.\n", ch);
		return;
	}
	else if (!str_cmp (buf, "remove"))
	{
		if (*clan)
			mysql_safe_query
			("DELETE FROM clan_assignments WHERE clan_name = '%s' AND imm_name = '%s'",
			clan, admin);
		else
			mysql_safe_query
			("DELETE FROM clan_assignments WHERE imm_name = '%s'", admin);
		if (*clan)
			send_to_char
			("The specified assignment has been removed from the database.\n",
			ch);
		else
			send_to_char
			("All clan assignments for the specified admin have been removed.\n",
			ch);
		return;
	}

	send_to_char ("Usage: assign (add | remove) <admin name> <clan name>\n",
		ch);
	return;
}

void
	notify_captors (CHAR_DATA * ch)
{
	CLAN_DATA *clan_def = NULL;
	DESCRIPTOR_DATA *d;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	int clan_flags = 0;

	if (!ch->room)
		return;

	if (!is_in_cell (ch, ch->room->zone))
		return;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!d->character || d->connected != CON_PLYNG)
			continue;
		if (!d->character->room)
			continue;
		p = d->character->clans;
		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			if ((clan_def = get_clandef (clan_name)))
				strcpy (clan_name, clan_def->literal);
			if (!clan_def)
				continue;
			if (!clan_def->zone)
				continue;
			if (clan_def->zone == ch->room->zone)
			{
				send_to_char
					("\n#3A PC prisoner has logged into one of your cells and requires attention.#0\n",
					d->character);
				break;
			}
		}
	}
}

void
	show_waiting_prisoners (CHAR_DATA * ch)
{
	CLAN_DATA *clan_def = NULL;
	DESCRIPTOR_DATA *d;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int clan_flags = 0, prisoners = 0;

	p = ch->clans;

	while (get_next_clan (&p, clan_name, &clan_flags))
	{
		if ((clan_def = get_clandef (clan_name)))
			strcpy (clan_name, clan_def->literal);
		if (!clan_def)
			continue;
		if (!clan_def->zone)
			continue;
		if (!zone_table[clan_def->zone].jailer)
			continue;
		for (d = descriptor_list; d; d = d->next)
		{
			if (!d->character || d->connected != CON_PLYNG)
				continue;
			if (!d->character->room)
				continue;
			if (is_area_enforcer (d->character))
				continue;
			if (is_in_cell (d->character, clan_def->zone))
				prisoners++;
		}
	}

	if (prisoners > 0)
	{
		sprintf (buf,
			"\n#3There %s currently %d PC prisoner%s awaiting your attention.\n",
			prisoners != 1 ? "are" : "is", prisoners,
			prisoners != 1 ? "s" : "");
		send_to_char (buf, ch);
	}
}


void
	do_promote (CHAR_DATA * ch, char *argument, int cmd)
{
	int nLeaderRank = 0;
	int nLackeyRank = 0;
	CHAR_DATA *pLackey;
	CLAN_DATA *pClan;
	char strLackey[AVG_STRING_LENGTH] = "\0";
	char strClan[AVG_STRING_LENGTH] = "\0";
	char strRank[AVG_STRING_LENGTH] = "\0";
	char strOutput[AVG_STRING_LENGTH] = "\0";
	char strUsage[] = "Usage: promote <character> <clan> <rank>.\n\0";


	argument = one_argument (argument, strLackey);

	if (!*strLackey)
	{
		send_to_char (strUsage, ch);
		return;
	}

	if (!(pLackey = get_char_room_vis (ch, strLackey)))
	{
		send_to_char ("Nobody is here by that name.\n", ch);
		send_to_char (strUsage, ch);
		return;
	}

	if (pLackey == ch)
	{
		switch (number (1, 100))
		{
		case 1:
			send_to_char ("Your delusions of grandeur go largely unnoticed.\n",
				ch);
			break;
		case 2:
			send_to_char ("You give yourself a pat on the back.\n", ch);
			break;
		default:
			send_to_char
				("Unfortunately, promotions just don't work that way...\n", ch);
			break;
		}
		send_to_char (strUsage, ch);
		return;
	}

	argument = one_argument (argument, strClan);

	if (!*strClan)
	{
		send_to_char ("Which clan did you wish to make the promotion in?\n",
			ch);
		return;
	}

	if (!get_clan_long_short (ch, strClan, &nLeaderRank))
	{
		send_to_char ("You are not a member of such a clan.\n", ch);
		return;
	}

	if (!get_clan_long_short (pLackey, strClan, &nLackeyRank))
	{
		act ("$N is not a clan member.", false, ch, 0, pLackey, TO_CHAR);
		return;
	}

	/* Keep clan_name as the short name */
	if ((pClan = get_clandef_long (strClan)))
		strcpy (strClan, pClan->name);

	argument = one_argument (argument, strRank);

	if (!*strRank)
	{
		send_to_char (strUsage, ch);
		return;
	}

	//  if (!(nLackeyRank = clan_flags_to_value (strRank)))
	if (!(nLackeyRank = clan_flags_to_value (strRank, strClan)))
	{
		send_to_char ("I don't recognize the specified rank.\n", ch);
		return;
	}

	if (!IS_SET (nLeaderRank, CLAN_LEADER)
		&& (nLeaderRank < CLAN_CORPORAL || nLeaderRank > CLAN_COMMANDER)
		&& !IS_SET (nLeaderRank, CLAN_MASTER))
	{
		send_to_char ("You are not a officer or leader of that clan.\n", ch);
		return;
	}

	if (nLeaderRank >= CLAN_CORPORAL && nLeaderRank <= CLAN_COMMANDER
		&& (nLackeyRank < CLAN_RECRUIT || nLackeyRank > CLAN_COMMANDER) && !strcmp(strClan, "khagdu"))
	{
		send_to_char
			("You'll need to specify a military rank, e.g. Recruit, Private, etc.\n",
			ch);
		return;
	}
	else if (nLeaderRank >= CLAN_APPRENTICE && nLeaderRank <= CLAN_MASTER
		&& (nLackeyRank < CLAN_APPRENTICE || nLackeyRank > CLAN_MASTER))
	{
		send_to_char
			("You'll need to specify a guild rank, e.g. Apprentice, Journeyman, etc.\n",
			ch);
		return;
	}
	if (nLackeyRank >= nLeaderRank)
	{
		send_to_char ("You do not have the authority to make this promotion.\n",
			ch);
		return;
	}

	sprintf (strOutput, "You promote $N to the rank of %s.",
		get_clan_rank_name (pLackey, strClan, nLackeyRank));
	act (strOutput, false, ch, 0, pLackey, TO_CHAR | _ACT_FORMAT);

	sprintf (strOutput, "$n has promoted you to the rank of %s.",
		get_clan_rank_name (pLackey, strClan, nLackeyRank));
	act (strOutput, false, ch, 0, pLackey, TO_VICT | _ACT_FORMAT);

	act ("$n has promoted $N.", false, ch, 0, pLackey,
		TO_NOTVICT | _ACT_FORMAT);

	add_clan (pLackey, strClan, nLackeyRank);
}

void
	do_invite (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags;
	char *p;
	CHAR_DATA *tch;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Invite who?\n\r", ch);
		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n", ch);
		return;
	}

	if (tch == ch)
	{
		send_to_char ("You can't invite yourself.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		p = ch->clans;

		if (!get_next_leader (&p, clan_name, &clan_flags))
		{
			send_to_char ("You are not an initiated leader of any clan.\n", ch);
			return;
		}

		if ((clan = get_clandef (clan_name)))
		{
			send_to_char ("That clan could not be found in the database.\n",
				ch);
			return;
		}

		strcpy (buf, clan->name);
	}

	else
	{
		if (!get_clan_long_short (ch, buf, &clan_flags))
		{
			send_to_char ("You are not a member of such a clan.\n", ch);
			return;
		}

		if (!IS_SET (clan_flags, CLAN_LEADER)
			&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
			&& !IS_SET (clan_flags, CLAN_MASTER)
			&& !IS_SET (clan_flags, CLAN_JOURNEYMAN))
		{
			send_to_char ("You are not a leader of that clan.\n", ch);
			return;
		}

		/* Keep clan_name as the short name */

		if ((clan = get_clandef_long (buf)))
			strcpy (buf, clan->name);
	}

	if (get_clan (tch, buf, &clan_flags))
	{
		act ("$N is already a clan member.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (tch->delay || tch->fighting)
	{
		act ("$N is busy right now.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (!AWAKE (tch))
	{
		act ("$N isn't conscious right now.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	clan = get_clandef (buf);

	tch->delay = IS_NPC (tch) && !tch->descr() ? 3 : 120;
	tch->delay_type = DEL_INVITE;
	tch->delay_ch = ch;
	tch->delay_who = str_dup (buf);

	sprintf (buf, "You invite $N to join %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, ch, 0, tch, TO_CHAR);

	sprintf (buf, "$N invites you to join %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, tch, 0, ch, TO_CHAR);

	act ("$N whispers something to $n about joining a clan.",
		false, tch, 0, ch, TO_NOTVICT | _ACT_FORMAT);

}

void
	invite_accept (CHAR_DATA * ch, char *argument)
{
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!ch->delay_who || !*ch->delay_who)
		return;

	ch->delay = 0;

	strcpy (clan_name, ch->delay_who);
	mem_free (ch->delay_who);
	ch->delay_who = NULL;

	if (!is_he_here (ch, ch->delay_ch, 1))
	{
		send_to_char ("Too late.\n", ch);
		return;
	}

	add_clan (ch, clan_name, CLAN_MEMBER);

	act ("$N accepts your invitation.", false, ch->delay_ch, 0, ch, TO_CHAR);

	clan = get_clandef (clan_name);

	sprintf (buf, "You have been initiated into %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, ch, 0, 0, TO_CHAR);
}

void
	do_recruit (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *pal;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_leader (ch, pal))
				break;
		}

		if (!pal)
		{
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		pal->following = ch;
		// ch->group->insert (pal);

		act ("$N nods to you to follow.", false, pal, 0, ch,
			TO_CHAR | _ACT_FORMAT);
		act ("$N motions to $S clanmates.", false, pal, 0, ch,
			TO_NOTVICT | _ACT_FORMAT);
		act ("$n falls in.", false, pal, 0, ch, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!str_cmp (buf, "all"))
	{
		act ("$n motions to $s clanmates.", false, ch, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_leader (ch, pal))
			{
				pal->following = ch;
				// ch->group->insert (pal);
				act ("$N nods to you to follow.", false, pal, 0, ch,
					TO_CHAR | _ACT_FORMAT);
				act ("$n falls in.", false, pal, 0, ch, TO_ROOM | _ACT_FORMAT);
			}
		}

		if (!pal)
		{
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		return;
	}

	if (!(pal = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if (pal == ch)
	{
		send_to_char ("Not yourself.\n\r", ch);
		return;
	}

	if (!is_leader (ch, pal))
	{
		act ("You don't have the authority to recruit $N.", false, ch, 0, pal,
			TO_CHAR);
		return;
	}

	if (pal->following == ch)
	{
		act ("$N is already following you.", false, ch, 0, pal, TO_CHAR);
		return;
	}

	pal->following = ch;
	// ch->group->insert (pal);

	act ("$N motions to $S clanmates.", false, pal, 0, ch,
		TO_NOTVICT | _ACT_FORMAT);
	act ("$n falls in.", false, pal, 0, ch, TO_VICT | _ACT_FORMAT);
	act ("$N nods to you to follow.", false, pal, 0, ch, TO_CHAR);
}

void
	do_disband (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
        CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || !str_cmp (buf, "all"))
	{

		act ("$n motions to $s group.", false, ch, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You motion to your group.", false, ch, 0, 0,
			TO_CHAR | _ACT_FORMAT);

		for (victim = ch->room->people; victim; victim = victim->next_in_room)
		{

			if (victim->following != ch)
				continue;

			if (victim->following == ch)
			{
				act ("$N motions to you to stop following.",
					false, victim, 0, ch, TO_CHAR | _ACT_FORMAT);
				act ("$n falls out of step.", false, victim, 0, ch,
					TO_ROOM | _ACT_FORMAT);
				victim->following = 0;
				// ch->group->erase (pal);
			}
		}

		return;
	}

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if (victim->following != ch)
	{
		act ("$N is not following you.", false, ch, 0, ch, TO_CHAR);
		return;
	}

//	if (!is_leader (ch, victim))
	//{
//		act ("You can't give $N orders.", false, ch, 0, victim, TO_CHAR);
//		return;
//	}

//	if (!IS_NPC (ch))
//	{
//		send_to_char ("This command does not work on PCs.\n", ch);
//		return;
//	}

	victim->following = 0;
	// ch->group->erase (pal);

	act ("You motion to $N.", false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
	act ("$N motions to $n.", false, victim, 0, ch, TO_NOTVICT | _ACT_FORMAT);
	act ("$N motions to you to stop following.", false, victim, 0, ch, TO_CHAR);
	act ("$n falls out of step.", false, victim, 0, ch, TO_ROOM | _ACT_FORMAT);
}
void
	do_castout (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags = 0, clan_flags2 = 0;
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	bool found = false;
	bool load_tag = false;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];


	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Castout whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room_vis (ch, buf)))
	{
		found = true;
    }
	/* else if (!found && (victim = get_pc(buf)))
	{
		found = true;
	}
	else if (!found && (victim = load_pc(buf)))
	{
		found = true;
		load_tag = true;
	} */
	else
	{
		sprintf (buf2,"There is no one with that name or you must be in the same room with them to cast them out.\n\r");
	}

	if (!found)
	{
		send_to_char (buf2, ch);
		return;
	}


	argument = one_argument (argument, buf);

	p = ch->clans;

	if (!*buf)
	{
		if (!get_next_leader (&p, clan_name, &clan_flags))
		{
			send_to_char
				("You are not a leader in that clan.  You can't cast out anybody.\n",
				ch);
			return;
		}

		clan = get_clandef (clan_name);
		if (!clan)
		{
			send_to_char ("No such clan, I am afraid.\n", ch);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else if (get_clan_long_short (ch, buf, &clan_flags))
	{
		if (!IS_SET (clan_flags, CLAN_LEADER)
			&& (clan_flags < CLAN_SERGEANT || clan_flags > CLAN_COMMANDER)
			&& !IS_SET (clan_flags, CLAN_MASTER))
		{
			send_to_char
				("You are not a leader in that clan.  You cannot cast anyone out.\n",
				ch);
			return;
		}

		clan = get_clandef_long (buf);
		if (!clan)
		{
			send_to_char ("No such clan, I am afraid.\n", ch);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else
	{
		send_to_char ("You are not a member of such a clan.\n", ch);
		return;
	}

	if (!get_clan (victim, clan_name, &clan_flags2))
	{
		act ("$N is not part of your clan.\n", false, ch, 0, victim, TO_CHAR);
		return;
	}

	if (IS_SET (clan_flags, CLAN_LEADER) || clan_flags <= clan_flags2)
	{
		act ("You are not of sufficient rank to cast $N from the clan.\n",
			false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
		return;
	}

	remove_clan (victim, clan_name);

	sprintf (buf, "$N is no longer a part of %s.", clan->literal);
	act (buf, false, ch, 0, victim, TO_CHAR);
	sprintf (buf, "$n has stripped you of your clan membership in %s.",
		clan->literal);
	act (buf, false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
	sprintf (buf, "$n has stripped $N of membership in %s.", clan->literal);
	act ("$n has stripped $N of clan membership.", false, ch, 0, victim,
		TO_NOTVICT | _ACT_FORMAT);

	if (load_tag)
		unload_pc(victim);
}


/* called by db.c:boot_objects */
void
	clan__assert_member_objs ()
{
	char buf[AVG_STRING_LENGTH];
	CLAN_DATA *clan;
	OBJ_DATA *obj;
	int i = 0;

	for (clan = clan_list; clan; clan = clan->next)
	{

		if (clan->leader_vnum)
		{

			if (!(obj = vtoo (clan->leader_vnum)))
			{
				sprintf (buf,
					"Note:  Clan leader obj %d does not exist for %d.",
					clan->leader_vnum, i);
				system_log (buf, true);
			}
			else
			{
				obj->obj_flags.extra_flags |= ITEM_LEADER;
			}

		}

		if (clan->member_vnum)
		{

			if (!(obj = vtoo (clan->member_vnum)))
			{
				sprintf (buf,
					"Note:  Clan member obj %d does not exist for %d.",
					clan->member_vnum, i);
				system_log (buf, true);
			}
			else
			{
				obj->obj_flags.extra_flags |= ITEM_MEMBER;
			}

		}

		/*
		if (clan->omni_vnum)
		{

		if (!(obj = vtoo (clan->omni_vnum)))
		{
		sprintf (buf, "Note:  Clan omni obj %d does not exist for %d.",
		clan->omni_vnum, i);
		system_log (buf, true);
		}
		else
		{
		obj->obj_flags.extra_flags |= ITEM_OMNI;
		}

		}
		*/
	}

	return;
}

/*********************************************************************
*                                                                    *
*  clan->do_add                                                      *
*                                                                    *
*  Inserts this clan's definition into the clans database.           *
*  TODO: Update definition on duplicate clan->name ?                 *
*  TODO: Escape apostrophes in clan->name and clan->literal          *
*                                                                    *
*********************************************************************/
void
	clan__do_add (CLAN_DATA * clan)
{
	char buf[AVG_STRING_LENGTH * 2] = "";
	std::string player_db = engine.get_config ("player_db");
	sprintf (buf, "INSERT INTO %s.clans "
		"(name,long_name,zone,member_obj,leader_obj,omni_obj) "
		"VALUES ('%s','%s', %d, %d, %d, %d) ;",
		player_db.c_str (),
		clan->name,
		clan->literal,
		clan->zone, clan->member_vnum, clan->leader_vnum, clan->omni_vnum);

	mysql_safe_query (buf);
}


/*********************************************************************
*                                                                    *
*  clan->do_update                                                   *
*                                                                    *
*  Updates this clan's definition in the clans database.             *
*  TODO: Update definition based on shadows_pfiles.clans.id          *
*  TODO: Escape apostrophes in clan->name and clan->literal          *
*                                                                    *
*********************************************************************/
void
	clan__do_update (CLAN_DATA * clan)
{
	char buf[AVG_STRING_LENGTH * 2] = "";
	std::string player_db = engine.get_config ("player_db");
	sprintf (buf,
		"UPDATE %s.clans "
		"SET long_name = '%s', zone = %d, member_obj = %d, leader_obj = %d, omni_obj = %d "
		"WHERE name = '%s' ;",
		player_db.c_str (),
		clan->literal,
		clan->zone,
		clan->member_vnum, clan->leader_vnum, clan->omni_vnum, clan->name);

	mysql_safe_query (buf);
}


/*********************************************************************
*                                                                    *
*  clan->do_remove                                                   *
*                                                                    *
*  Deletes this clan's definition from the clans database.           *
*                                                                    *
*********************************************************************/
void
	clan__do_remove (CLAN_DATA * clan)
{
	std::string player_db = engine.get_config ("player_db");
	mysql_safe_query
		("DELETE FROM %s.clans "
		"WHERE name = '%s' ;",
		player_db.c_str (), clan->name);
}

/*********************************************************************
*                                                                    *
*  clan->do_load                                                     *
*                                                                    *
*  Loads the database of clan definition into the game               *
*                                                                    *
*********************************************************************/
void
	clan__do_load ()
{
	CLAN_DATA *clan = NULL, *last_clan = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[AVG_STRING_LENGTH] = "";

	std::string player_db = engine.get_config ("player_db");
	mysql_safe_query
		("SELECT name,long_name,zone,member_obj,leader_obj,omni_obj,id"
		" FROM %s.clans"
		" ORDER BY name ;",
		player_db.c_str ());

	if (!(result = mysql_store_result (database)))
	{
		sprintf (buf, "clan__do_load: %s", mysql_error (database));
		system_log (buf, true);
		return;
	}
	while ((row = mysql_fetch_row (result)))
	{

		if ((clan = (CLAN_DATA *) alloc (sizeof (CLAN_DATA), 39)) != NULL)
		{
			clan->name = str_dup (row[0]);
			clan->zone = strtol (row[2], NULL, 0);
			clan->literal = str_dup (row[1]);
			clan->member_vnum = strtol (row[3], NULL, 0);
			clan->leader_vnum = strtol (row[4], NULL, 0);
			clan->omni_vnum = strtol (row[5], NULL, 0);
			clan->id = strtol (row[6], NULL, 0);

			if (clan_list)
			{
				clan->next = NULL;
				last_clan->next = clan;
			}
			else
			{
				clan->next = clan_list;
				clan_list = clan;
			}
			last_clan = clan;
		}
		else
		{
			sprintf (buf,
				"clan__do_load: Unable to allocate memory for CLAN_DATA %s",
				row[0]);
			system_log (buf, true);
			return;
		}
	}
	mysql_free_result (result);
	result = NULL;
}

void
	clan__do_score (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH] = "";
	int clan_flags = 0;
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	char clan_alias[MAX_STRING_LENGTH] = { '\0' };
	CLAN_DATA *clan_def = NULL;
	char *p = '\0';

	if (ch->clans && *ch->clans)
	{

		send_to_char ("\nYou are affiliated with the following clans:\n\n", ch);

		p = ch->clans;

		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			strcpy (clan_alias, clan_name);
			if ((clan_def = get_clandef (clan_name)))
				strcpy (clan_name, clan_def->literal);

			sprintf (buf, "  %-40s ", clan_name);
			sprintf (buf + strlen(buf), "#2(%s)#0  ", get_clan_rank_name (ch, clan_alias, clan_flags));

			strcat (buf, "\n");
			send_to_char (buf, ch);
		}
	}
}


/* caller: staff.c:write_dynamic_registry() 	deprecated */
void
	clan__filedump (FILE * fp)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{

		fprintf (fp, "clandef %s %d \"%s\" %d %d %d\n",
			clan->name,
			clan->zone,
			clan->literal,
			clan->member_vnum, clan->leader_vnum, clan->omni_vnum);
	}
}

/* removes a clan from the object */
void
	clan_rem_obj (OBJ_DATA *obj, OBJ_CLAN_DATA * targ)
{
	obj->clan_data = NULL;
	mem_free (targ->name);
	mem_free (targ->rank);
	mem_free (targ);

	return;
}

/** roster <clan name>
** Will list all clanmembers who have logged in during the
** last two weeks. Only the sdesc and rank is given.
**/
void
	do_rollcall (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char clanname[MAX_STRING_LENGTH];
	int clan_flags;
	MYSQL_RES *result;
	MYSQL_ROW row;
	CLAN_DATA *tclan;
	int index;


	argument = one_argument (argument, clanname);


	if (!*clanname)
	{
		send_to_char ("Which clan do you want a roster for?\n", ch);
		return;
	}

	if (!get_clan_long_short (ch, clanname, &clan_flags))
	{
		send_to_char ("You are not a member of that clan.\n", ch);
		return;
	}

	if (!IS_SET (clan_flags, CLAN_LEADER)
		&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
		&& !IS_SET (clan_flags, CLAN_MASTER)
		&& !IS_SET (clan_flags, CLAN_JOURNEYMAN))
	{
		send_to_char ("You are not a leader of that clan.\n", ch);
		return;
	}

	/* Keep clan_name as the short name */
	if ((tclan = get_clandef_long (clanname)))
		strcpy (clanname, tclan->name);



	/* search last fornight */
	result = mysql_player_search (SEARCH_CLAN, clanname, 4);

	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);


		send_to_char ("No names were found.\n", ch);
		return;
	}

	sprintf (buf, "#6Clan members:  %d#0\n\n",
		(int) mysql_num_rows (result));

	index = 1;

	while ((row = mysql_fetch_row (result)))
	{
		// row[2] is sdesc
		// row[4] is rank
		sprintf (buf2, "%4d. %-13s %s\n", index, row[4], row[2]);

		if (strlen (buf) + strlen (buf2) >= MAX_STRING_LENGTH)
			break;
		else
			sprintf (buf + strlen (buf), "%s", buf2);

		index++;
	}

	page_string (ch->descr(), buf);
	mysql_free_result (result);
	return;
}


// Some code to get rid of all of these conflict clans.
void
	conflicting_clan_check(CHAR_DATA *ch)
{
    // Ensuring we have valid data
	if (!ch->pc || !ch->pc->level)
		return;


	int top_rank = CLAN_MEMBER;
	char * top_clan = '\0';

	int clan_count = 0;

	int char_clan = -1;
	int char_rank = 0;
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	int clan_flags = 0;
	char *p = '\0';
	p = ch->clans;

	// Run through our clans, check them against
	// and Wilmingtons, and get our top rank of all of these.
	while ( get_next_clan( &p, clan_name, & clan_flags ) )
	{
		if ( clan_flags >= CLAN_RECRUIT)
		{
			if ( clan_flags > top_rank )
			{
				top_rank = clan_flags;
				top_clan = clan_name;
			}

			clan_count++;
		}
	}

	// If we belong to more of these clans, then we run through them all
	// again and strip out any that wasn't our top rank.
	if ( clan_count > 1 )
	{
		while ( get_next_clan( &p, clan_name, &clan_flags ) )
		{
            if ( clan_name != top_clan )
            {
                remove_clan( ch, clan_name );
            }
		}

		send_to_char( "Your have shedded your conflicting loyalties - only the strongest remain.", ch );
	}
}
