#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "group.h"
#include "protos.h"

const char *race_describer[] =
    {"humans", "humanoids", "insects", "automatons", "bots", "vehicles",
     "dogs", "lizards", "spiders", "rodents", "bats", "animals"
    };

bool
is_with_group (CHAR_DATA * ch)
{
    CHAR_DATA *tch;

    if (!ch->room)
        return 0;


    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch == ch)
            continue;
        if (tch->following == ch || ch->following == tch ||
                (ch->following && tch->following == ch->following))
        {
            return 1;
        }
    }

    return 0;
}

// Are we theoretically in the same vicinty as our target?
bool
are_theory_grouped (CHAR_DATA *ch, CHAR_DATA *target)
{
    AFFECTED_TYPE *af = NULL;
    AFFECTED_TYPE *target_af = NULL;

    // Obviously, get the simple stuff out of the way first.

    if (!ch || !target || !ch->room || !target->room || ch->in_room != target->in_room || target == ch)
        return 0;

    // If our target is sitting at a table...
    if ((target_af = get_covtable_affect(target)))
    {
        // And we're sitting at a table...
        if ((af = get_covtable_affect(ch)))
        {
            // And it's the same table, then return true, otherwise it's false.
            if (af->a.table.obj == target_af->a.table.obj)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    // Otherwise, if our target's not at a table but we are, return false.
    else if ((af = get_covtable_affect(ch)))
    {
        return false;
    }
    // Otherwise if we're invovled in combat, return true
    else if (is_involved(ch, target, 0))
    {
        return true;
    }
    // Or we're in the same group.
    else if (are_grouped(ch, target))
    {
        return true;
    }

    // If we're not at the same table, or fighting, or in the same group, then we're
    return false;

}


void
do_follow (CHAR_DATA * ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *leader = NULL, *tch = NULL, *orig_leader = NULL;

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        send_to_char ("Follow whom?\n", ch);
        return;
    }

    if (!(leader = get_char_room_vis (ch, buf)))
    {
        send_to_char ("There is nobody here by that name.\n", ch);
        return;
    }

    if (IS_MORTAL (ch) && leader != ch
            && IS_SET (leader->plr_flags, GROUP_CLOSED))
    {
        send_to_char
        ("That individual's group is currently closed to new followers.\n",
         ch);
        act ("$n just attempted to join your group.", true, ch, 0, leader,
             TO_VICT | _ACT_FORMAT);
        return;
    }

    if (leader != ch)
    {
        if (leader->following == ch)
        {
            send_to_char
            ("You'll need to ask them to stop following you first.\n", ch);
            return;
        }
        orig_leader = leader;
        while (leader->following)
            leader = leader->following;
        if (IS_MORTAL (ch) && leader != ch
                && IS_SET (leader->plr_flags, GROUP_CLOSED))
        {
            send_to_char
            ("That individual's group is currently closed to new followers.\n",
             ch);
            act ("$n just attempted to join your group.", true, ch, 0, leader,
                 TO_VICT | _ACT_FORMAT);
            return;
        }

        ch->following = leader;

        for (tch = ch->room->people; tch; tch = tch->next_in_room)
        {
            if (tch->following == ch)
            {
                tch->following = leader;

                if (leader->formation > 2)
                {
                    tch->formation = 1;
                    act ("You fall into formation with the group's new leader, $N.",
                         false, tch, 0, leader, TO_CHAR | _ACT_FORMAT);
                }
                else
                {
                    tch->formation = 0;
                    act ("You fall into stride with the group's new leader, $N.",
                         false, tch, 0, leader, TO_CHAR | _ACT_FORMAT);
                    leader->formation = 0;
                }
            }
        }


        if (orig_leader != leader)
        {
            if (!IS_SET (ch->flags, FLAG_WIZINVIS))
            {
                if (leader->formation > 2)
                {
                    ch->formation = 1;
                    sprintf (buf,
                             "You fall in to formation behind #5%s's#0 group's current leader, $N.",
                             char_short (orig_leader));
                }
                else
                {
                    leader->formation = 0;
                    ch->formation = 0;
                    sprintf (buf,
                             "You begin following #5%s's#0 group's current leader, $N.",
                             char_short (orig_leader));
                }
            }
            else if (IS_SET (ch->flags, FLAG_WIZINVIS))
                sprintf (buf,
                         "You will secretly follow #5%s#0's group's current leader, $N.",
                         char_short (orig_leader));
        }
        else
        {
            if (!IS_SET (ch->flags, FLAG_WIZINVIS))
            {
                if (leader->formation > 2)
                {
                    ch->formation = 1;
                    sprintf (buf, "You fall in to formation behind $N.");
                }
                else
                {
                    leader->formation = 0;
                    ch->formation = 0;
                    sprintf (buf, "You begin following $N.");
                }
            }
            else if (IS_SET (ch->flags, FLAG_WIZINVIS))
                sprintf (buf, "You will secretly follow $N.");
        }
        act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
        sprintf (buf, "$n falls into stride with you.");
        if (!IS_SET (ch->flags, FLAG_WIZINVIS))
            act (buf, false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);
        sprintf (buf, "$n falls into stride with $N.");
        if (!IS_SET (ch->flags, FLAG_WIZINVIS))
            act (buf, false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
        return;
    }

    if (leader == ch && ch->following && ch->following != ch)
    {
        sprintf (buf, "You will no longer follow $N.");
        act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
        sprintf (buf, "$n is no longer following you.");
        if (!IS_SET (ch->flags, FLAG_WIZINVIS)
                && ch->room == ch->following->room)
            act (buf, false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);
        sprintf (buf, "$n stops following $N.");
        if (!IS_SET (ch->flags, FLAG_WIZINVIS)
                && ch->room == ch->following->room)
            act (buf, false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
        ch->following = 0;
        ch->formation = 0;
        return;
    }

    if (leader == ch && (!ch->following || ch->following == ch))
    {
        send_to_char ("You aren't following anyone!\n", ch);
        return;
    }

}

/*
void
do_follow (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *leader = NULL, *orig_leader = NULL;

  argument = one_argument (argument, buf);

  if (!*buf)
    {
      send_to_char ("Follow whom?\n", ch);
      return;
    }

  if (!(leader = get  if (!(top_leader = ch->following))
    top_leader = ch;

  if (!top_leader)
    {
      send_to_char ("You aren't in a group.\n", ch);
      return;
    }_char_room_vis (ch, buf)))
    {
      send_to_char ("There is nobody here by that name.\n", ch);
      return;
    }

  if (IS_MORTAL (ch) && leader != ch
      && IS_SET (leader->plr_flags, GROUP_CLOSED))
    {
      send_to_char
	("That individual's group is currently closed to new followers.\n",
	 ch);
      act ("$n just attempted to join your group.", true, ch, 0, leader,
	   TO_VICT | _ACT_FORMAT);
      return;
    }

  bool wizinvis = IS_SET (ch->flags, FLAG_WIZINVIS);

  if (leader != ch)
    {
      if (leader->following == ch)
	{
	  send_to_char
	    ("You'll need to ask them to stop following you first.\n", ch);
	  return;
	}

      // TAKE ME TO YOUR LEADER
      orig_leader = leader;
      while (leader->following)
	{
	  leader = leader->following;
	}

      if (IS_MORTAL (ch) && leader != ch
	  && IS_SET (leader->plr_flags, GROUP_CLOSED))
	{
	  send_to_char ("That individual's group is "
			"currently closed to new followers.\n",
			ch);
	  act ("$n just attempted to join your group.",
	       true, ch, 0, leader, TO_VICT | _ACT_FORMAT);

	  return;
	}

      ch->following = leader;
      leader->group->insert (ch);

      // copy ch's local following to leader
      std::set<CHAR_DATA*>::iterator i;
      ROOM_DATA *here = ch->room;
      for (i = ch->group->begin (); i != ch->group->end (); ++i)
	{
	  CHAR_DATA *tch = (*i);
	  if (tch->following == ch)
	    {
	      if (tch->room == here)
		{
		  tch->following = leader;
		  leader->group->insert (tch);
		  act ("You fall into stride with the group's new leader, $N.",
		       false, tch, 0, leader, TO_CHAR | _ACT_FORMAT);
		}
	      else
		{
		  tch->following = 0;
		}
	    }
	}
      ch->group->clear ();

      if (wizinvis)
	{
	  if (orig_leader != leader)
	    {
	      sprintf (buf,
		       "You will secretly follow #5%s#0's group's "
		       "current leader, $N.",
		       char_short (orig_leader));
	      act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
	    }
	  else
	    {
	      act ("You will secretly follow $N.",
		   false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
	    }
	}
      else
	{
	  if (orig_leader != leader)
	    {  int race_chart[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	      sprintf (buf,
		       "You begin following #5%s's#0 group's "
		       "current leader, $N.",
		       char_short (orig_leader));
	      act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
	    }
	  else
	    {
	      act ("You begin following $N.",
		   false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
	    }

	  act ("$n falls into stride with you.",
	       false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);

	  act ("$n falls into stride with $N.",
	       false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
	}
      return;
    }

  // leader == ch
  if ((ch->following) && (ch->following != ch))
    {
      act ("You will no longer follow $N.",
	   false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);

      if (!wizinvis && ch->room == ch->following->room)
	{
	  act ("$n is no longer following you.",
	       false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);
	  act ("$n stops following $N.",
	       false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
	}
      ch->following->group->erase (ch);
    }
  else
    {
      send_to_char ("You aren't following anyone!\n", ch);
    }

  ch->group->erase (ch);
  ch->following = 0;
  return;
}
*/

// returns both the size and racial composition of a group as a string.
// Should only be called in conjuction with a group size greater than 6.

char * group_makeup (CHAR_DATA *ch)
{
    CHAR_DATA *tch = NULL, *top_leader = NULL;
    int i;
    float size;
    float mount_size;
    int j = 0, k = 0;
    static char makeup[MAX_STRING_LENGTH] = { '\0' };
    char mounts[MAX_STRING_LENGTH] = { '\0' };
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char armour_msg[MAX_STRING_LENGTH] = { '\0' };
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    int race_chart[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    int mount_chart[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    float armour_count = 0;

    if (!is_with_group(ch))
        return makeup;

    if (!(top_leader = ch->following))
        top_leader = ch;

    if (!top_leader)
    {
        return makeup;
    }

    size = do_group_size(ch);
    mount_size = mount_group_size(ch);

    if (size > 80)
        sprintf (makeup, "horde");
    else if (size > 60)
        sprintf (makeup, "large crowd");
    else if (size > 40)
        sprintf (makeup, "crowd");
    else if (size > 25)
        sprintf (makeup, "large group");
    else if (size > 15)
        sprintf (makeup, "group");
    else if (size > 9)
        sprintf (makeup, "party");
    else
        sprintf (makeup, "small party");

    if (mount_size / size >= 1)
        sprintf (mounts, "all");
    else if (mount_size / size >= 0.80)
        sprintf (mounts, "most");
    else if (mount_size / size >= 0.60)
        sprintf (mounts, "many");
    else if (mount_size / size >= 0.40)
        sprintf (mounts, "some");
    else if (mount_size / size >= 0.20)
        sprintf (mounts, "a few");
    else
        mount_size = 0;


    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following == top_leader && !get_affect(tch, MAGIC_HIDDEN)
                && !IS_RIDEE(tch))
        {
            armour_count += armor_descript(tch);
            race_chart[lookup_race_int(tch->race, RACE_GROUP_NOUN)] += 1;
        }
        else if (mount_size && tch->following == top_leader && !get_affect(tch, MAGIC_HIDDEN)
                 && IS_RIDEE(tch))
        {
            mount_chart[lookup_race_int(tch->race, RACE_GROUP_NOUN)] += 1;
        }
    }


    armour_count = armour_count / size;

    if (armour_count >= 5)
        sprintf (armour_msg, "extremely armoured ");
    if (armour_count >= 3)
        sprintf (armour_msg, "heavily armoured ");
    else if (armour_count >=2)
        sprintf (armour_msg, "well armoured ");
    else if (armour_count >= 1)
        sprintf (armour_msg, "lightly armoured ");

    sprintf(armour_msg + strlen(armour_msg), makeup);

    sprintf(makeup, armour_msg);

    for (i = 0; i <= 12; i ++)
    {
        if (race_chart[i] >= (size / 3))
        {
            j += 1;
            sprintf (buf + strlen (buf), "%s ", race_describer[i]);
        }
    }

    if (!j)
        sprintf (makeup + strlen (makeup), " of assorted individuals ");
    else if (j == 1)
        sprintf (makeup + strlen (makeup), " of %s", buf);
    else if (j == 2)
    {
        arg_splitter (2, buf, arg1, arg2);
        sprintf (makeup + strlen (makeup), " of %s and %s", arg1, arg2);
    }
    else if (j == 3)
    {
        arg_splitter (3, buf, arg1, arg2, arg3);
        sprintf (makeup + strlen (makeup), " of %s, %s, and %s", arg1, arg2, arg3);
    }

    makeup[strlen(makeup)-1] = '\0';

    *buf = '\0';

    if (mount_size)
    {
        for (i = 0; i <= 12; i ++)
        {
            if (mount_chart[i] >= (mount_size / 3))
            {
                k += 1;
                sprintf (buf + strlen (buf), "%s ", race_describer[i]);
            }
        }
    }

    if (k == 1)
    {
        buf[strlen(buf)-1] = '\0';
        sprintf (makeup + strlen (makeup), ", %s mounted on %s, ", mounts, buf);
    }
    else if (k == 2)
    {
        arg_splitter (2, buf, arg1, arg2);
        arg2[strlen(arg2)-1] = '\0';
        sprintf (makeup + strlen (makeup), ", %s mounted on %s and %s, ", mounts, arg1, arg2);
    }
    else if (k == 3)
    {
        arg_splitter (k, buf, arg1, arg2, arg3);
        arg3[strlen(arg3)-1] = '\0';
        sprintf (makeup + strlen (makeup), ", %s mounted on %s, %s, and %s", mounts, arg1, arg2, arg3);
    }

    if (k)
        makeup[strlen(makeup)-1] = '\0';

    return makeup;
}

char *
tactical_status (CHAR_DATA * ch)
{
    CHAR_DATA *tch;
    AFFECTED_TYPE *af;
    char status[MAX_STRING_LENGTH] = { '\0' };
    int i = 0;

    if (GET_POS(ch) == SIT)
        sprintf (status + strlen (status), " #2(sitting)#0");

    if (GET_POS(ch) == REST)
        sprintf (status + strlen (status), " #2(resting)#0");

    if (GET_POS(ch) == SLEEP)
        sprintf (status + strlen (status), " #2(sleeping)#0");

    if (get_affect (ch, MAGIC_HIDDEN))
        sprintf (status + strlen (status), " #1(hidden)#0");

    if (get_affect (ch, MAGIC_GUARD) || get_affect (ch, AFFECT_GUARD_DIR))
        sprintf (status + strlen (status), " #6(guarding)#0");

    if (ch->formation == 1)
        sprintf (status + strlen (status), " #3(front)#0");

    if (ch->formation == 2)
        sprintf (status + strlen (status), " #3(rear)#0");

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if ((af = get_affect (tch, MAGIC_GUARD))
                && ((CHAR_DATA *) af->a.spell.t) == ch)
            i++;
    }

    if (i > 0)
    {
        if (i == 1)
            sprintf (status + strlen (status), " #2(guarded)#0");
        else if (i > 1)
            sprintf (status + strlen (status), " #2(guarded x %d)#0", i);
    }

    i = 0;

    if (ch->fighting)
        i++;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch == ch->fighting)
            continue;
        if (tch->fighting == ch)
            i++;
    }

    if (i > 0)
    {
        if (i == 1)
            sprintf (status + strlen (status), " #1(engaged)#0");
        else if (i > 1)
            sprintf (status + strlen (status), " #1(engaged x %d)#0", i);
    }

    return status;
}

int
mount_group_size (CHAR_DATA * ch)
{
    CHAR_DATA *tch = NULL, *top_leader = NULL;
    int i = 1;
    bool found = false;

    if (!(top_leader = ch->following))
        top_leader = ch;

    if (!top_leader)
    {
        return 0;
    }

    if (!is_with_group(ch))
    {
        return 0;
    }

    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following != top_leader && tch->mount && tch->mount->following != top_leader)
            continue;
        if (!IS_RIDEE(tch))
            continue;
        if (tch->mount == top_leader)
            continue;
        if (get_affect(tch, MAGIC_HIDDEN))
            continue;
        if (found != false)
            i += 1;
        found = true;
    }

    return i;

}


int
do_group_size (CHAR_DATA * ch)
{
    CHAR_DATA *tch = NULL, *top_leader = NULL;
    int i = 1;
    bool found = false;

    if (!(top_leader = ch->following))
        top_leader = ch;

    if (!top_leader)
    {
        return 0;
    }

    if (!is_with_group(ch))
    {
        return 0;
    }

    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following != top_leader)
            continue;
        if (IS_RIDEE(tch))
            continue;
        if (get_affect(tch, MAGIC_HIDDEN))
            continue;
        if (found != false)
            i += 1;
        found = true;
    }

    return i;

}

void
do_group (CHAR_DATA * ch, char *argument, int cmd)
{
    CHAR_DATA *tch = NULL, *top_leader = NULL;
    char status[MAX_STRING_LENGTH] = {'\0'};
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char buf2[MAX_STRING_LENGTH] = {'\0'};
    char arg1[MAX_STRING_LENGTH] = {'\0'};
    char arg2[MAX_STRING_LENGTH] = {'\0'};
    char *tactical;
    bool found = false;
    bool shown = false;
    int dir;

    if (!(top_leader = ch->following))
        top_leader = ch;

    argument = one_argument (argument, buf);

    if (*buf)
    {
        if (is_abbrev (buf, "open"))
        {

            if (!IS_SET (ch->plr_flags, GROUP_CLOSED))
            {
                send_to_char ("Your group is already open!\n", ch);
                return;
            }
            ch->plr_flags &= ~GROUP_CLOSED;
            send_to_char ("You will now allow people to follow you.\n", ch);
            return;
        }
        else if (is_abbrev (buf, "close"))
        {
            if (IS_SET (ch->plr_flags, GROUP_CLOSED))
            {
                send_to_char ("Your group is already closed!\n", ch);
                return;
            }
            ch->plr_flags |= GROUP_CLOSED;
            send_to_char ("You will no longer allow people to follow you.\n", ch);
            return;
        }
        else if (is_abbrev (buf, "push"))
        {
            if (top_leader != ch)
            {
                send_to_char ("You must be leading your group to have it attempt to push through a position.\n", ch);
                return;
            }

            if (get_second_affect (ch, SA_PUSH, NULL))
            {
                send_to_char ("You need to regather your troops before you can attempt to push through again.\n", ch);
                return;
            }

            if (IS_SUBDUER (ch))
            {
                send_to_char ("You can't push while you have someone in tow.\n", ch);
                return;
            }

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

            argument = one_argument (argument, arg1);

            if (!*arg1)
            {
                send_to_char ("Push in what direction?\n\r", ch);
                return;
            }

            if ((dir = index_lookup (dirs, buf)) == -1)
            {
                send_to_char ("Push where?\n\r", ch);
                return;
            }

            if (!CAN_GO (ch, dir))
            {
                if (ch->room->extra && ch->room->extra->alas[dir])
                    send_to_char (ch->room->extra->alas[dir], ch);
                else
                    send_to_char ("There is no exit that way.\n", ch);
                return;
            }

            ROOM_DATA * dest = vnum_to_room (ch->room->dir_option[dir]->to_room);

            if (dest && dest->capacity <= 0 && room_avail(dest, ch))
            {
                send_to_char ("The room has plenty of room for you to enter without needing to push.\n", ch);
                return;
            }
        }

        else if (is_abbrev (buf, "appoint"))
        {
            argument = one_argument (argument, arg1);

            if (ch->formation < 3)
            {
                send_to_char("You must be leading a group that is in formation to use this command.\n", ch);
                return;
            }

            if (!*arg1)
            {
                send_to_char("You must nominate both someone to appoint, and the position to appoint them to.\n", ch);
                return;
            }
            else if (!(tch = get_char_room(arg1, ch->in_room)) || tch->following != ch || tch == ch)
            {
                send_to_char("Either that person is not here, or is not following you.\n", ch);
                return;
            }

            argument = one_argument (argument, arg2);

            if (!*arg2)
            {
                send_to_char("You must nominate a position, either #6front#0 or #6rear#0.\n", ch);
                return;
            }
            else if (str_cmp(arg2, "front") && str_cmp(arg2, "rear") && str_cmp(arg2, "switch"))
            {
                send_to_char("You must nominate either #6front#0 or #6rear#0 as the position.\n", ch);
                return;
            }

            if (!str_cmp(arg2, "switch"))
            {
                if (tch->formation == 1)
                    sprintf(arg2, "rear");
                else if (tch->formation == 2)
                    sprintf(arg2, "front");
            }

            if (!str_cmp(arg2, "front"))
            {

                if (tch->formation == 1)
                {
                    act("You realise that $N is already at the front.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                    return;
                }

                act("You command $N to the front of your group's formation.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                act("$n commands $N to the front of $s group's formation.", false, ch, 0, tch, TO_ROOM | _ACT_FORMAT);

                tch->formation = 1;
                return;
            }
            else if (!str_cmp(arg2, "rear"))
            {

                if (tch->formation == 2)
                {
                    act("You realise that $N is already at the rear.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                    return;
                }

                act("You command $N to the rear of your group's formation.", false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
                act("$n commands $N to the rear of $s group's formation.", false, ch, 0, tch, TO_ROOM | _ACT_FORMAT);

                tch->formation = 2;
                return;
            }
        }

        else if (is_abbrev (buf, "formation"))
        {
            if (top_leader != ch || num_followers(ch) < 9)
            {
                send_to_char ("You need to be leading a group of at least ten to maintain a formation.\n", ch);
                return;
            }
            /*
            else if(ch->room->sector_type == SECT_WOODS ||
                    ch->room->sector_type == SECT_FOREST ||
                    ch->room->sector_type == SECT_SWAMP ||
                    ch->room->sector_type == SECT_MOUNTAIN ||
                    ch->room->sector_type == SECT_INSIDE ||
                    ch->room->sector_type == SECT_RIVER ||
                    ch->room->sector_type == SECT_LAKE ||
                    ch->room->sector_type == SECT_UNDERWATER)
            {
              send_to_char ("The surrounding terrain will not allow your group to maintain a tight formation.\n", ch);
            }
            */
            else
            {
                if (ch->formation > 2)
                {
                    send_to_char ("You send out a call for your followers to break from their formation.\n", ch);
                    act("$n calls out for $s followers to break out of formation.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

                    ch->formation = 0;

                    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
                    {
                        if (tch->following != top_leader)
                            continue;
                        tch->formation = 0;
                    }
                }
                else
                {
                    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
                    {
                        if (tch->following != top_leader)
                            continue;
                        if (tch->fighting || ch->fighting)
                        {
                            send_to_char("You cannot call gather your followers in to formation while they are under attack.\n", ch);
                            return;
                        }
                    }

                    send_to_char ("You send out a call for your followers to gather in formation.\n", ch);
                    act("$n calls out for $s followers to gather in formation.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

                    ch->delay_type = DEL_FORM;
                    ch->delay = (do_group_size(ch) / 2);
                }

                return;
            }

        }
        else if (is_abbrev (buf, "retreat"))
        {
            char direction_arg[AVG_STRING_LENGTH] = "";
            int direction = 0;
            argument = one_argument (argument, direction_arg);

            if (!*direction_arg
                    || (direction = index_lookup (dirs, direction_arg)) == -1 )
            {
                send_to_char ("Order a retreat in which direction?\n", ch);
            }
            else
            {
                CHAR_DATA* tch;
                bool ordered = false;
                for (tch = ch->room->people; tch; tch = tch->next_in_room)
                {
                    if (tch->following == ch)
                    {
                        ordered = true;
                        retreat (tch, direction, ch);
                    }
                }

                if (ordered)
                {
                    retreat (ch, direction, ch);
                }
                else
                {
                    retreat (ch, direction, 0);
                }
            }
            return;
        }
    }

    if (!top_leader)
    {
        send_to_char ("You aren't in a group.\n", ch);
        return;
    }

    *buf = *one_argument(buf, arg1);
    
    sprintf (buf, "#5%s#0 [%s] [%s] %s, leading:\n\n", char_short (top_leader),
             wound_total (top_leader, false), fatigue_bar (top_leader, false), tactical_status (top_leader));
    buf[2] = toupper (buf[2]);

    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following != top_leader)
            continue;
        // if (!CAN_SEE (ch, tch))s
        //continue;
        if (found != false)
            sprintf (buf + strlen (buf), ",\n");
            
        tactical = tactical_status(tch);
        tactical = one_argument(tactical, arg2);
        while (*arg2 && !shown)
        {
            send_to_char(buf2, ch);
            if ((str_cmp(arg1, "guarding") && str_cmp(arg2, "#6(guarding)"))) //||
            //(str_cmp(arg1, "guarding") && str_cmp(arg2, "(guarding)")) ||
            //        (str_cmp(arg1, "engaged") && str_cmp(arg2, "(engaged)")))
            {
                sprintf (buf + strlen (buf), "   #5%s#0 [%s] [%s]%s", char_short (tch),
                    wound_total (tch, false), fatigue_bar (tch, false),  tactical_status(tch));
                shown = true;
                found = true;
            }
            tactical = one_argument(tactical, arg2);
        }
        
        if (!shown && !(*arg1))
        {
            sprintf (buf + strlen (buf), "   #5%s#0 [%s] [%s]%s", char_short (tch),
                wound_total (tch, false), fatigue_bar (tch, false), tactical_status(tch));
            found = true;
        }
        shown = false;
    }

    if (!found)
        sprintf(buf + strlen (buf), "   No members.\n");
    else
        sprintf(buf + strlen (buf), "\n");

    send_to_char (buf, ch);
}



void delayed_form ( CHAR_DATA * ch)
{

    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA * tch;

    sprintf(buf1, "%s", group_makeup(ch));

    ch->formation = 3;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following != ch)
            continue;
        tch->formation = 1;
        sprintf(buf2, "At $N's command, you take your place in the #5%s#0 behind $M.", buf1);
        act(buf2, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
    }

    sprintf(buf2, "Finally, the #5%s#0 behind you stands ready in a tight formation.\n", buf1);
    send_to_char(buf2, ch);

    sprintf(buf2, "Finally, a #5%s#0forms up tightly behind $n.", buf1);

    act(buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

    ch->delay = 0;
    ch->delay_type = 0;

}

/*
void
do_group (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch = NULL, *leader = NULL;
  char buf[MAX_STRING_LENGTH];
  bool found = false;

  argument = one_argument (argument, buf);

  if (*buf)
    {
      if (is_abbrev (buf, "open"))
	{

	  if (!IS_SET (ch->plr_flags, GROUP_CLOSED))
	    {
	      send_to_char ("Your group is already open!\n", ch);
	      return;
	    }
	  ch->plr_flags &= ~GROUP_CLOSED;
	  send_to_char ("You will now allow people to follow you.\n", ch);
	  return;
	}
      else if (is_abbrev (buf, "close"))
	{
	  if (IS_SET (ch->plr_flags, GROUP_CLOSED))
	    {
	      send_to_char ("Your group is already closed!\n", ch);
	      return;
	    }
	  ch->plr_flags |= GROUP_CLOSED;
	  send_to_char ("You will no longer allow people to follow you.\n", ch);
	  return;
	}
      else if (is_abbrev (buf, "retreat"))
	{
	  char direction_arg[AVG_STRING_LENGTH] = "";
	  int direction = 0;
	  argument = one_argument (argument, direction_arg);

	  if (!*direction_arg
	      || (direction = index_lookup (dirs, direction_arg)) == -1 )
	    {
	      send_to_char ("Order a retreat in which direction?\n", ch);
	    }
	  else
	    {
	      CHAR_DATA* tch;
	      bool ordered = false;
	      for (tch = ch->room->people; tch; tch = tch->next)
		{
		  if (tch->following == ch)
		    {
		      ordered = true;
		      retreat (tch, direction, ch);
		    }
		}

	      if (ordered)
		{
		  retreat (ch, direction, ch);
		}
	      else
		{
		  retreat (ch, direction, 0);
		}
	    }
	  return;
	}
      else
	{
	  send_to_char ("Unknown subcommand.\n", ch);
	  return;
	}
    }

  leader = (ch->following) ? (ch->following) : (ch);
  if (leader->group->empty ())
    {
      send_to_char ("You aren't in a group.\n", ch);
      return;
    }

  sprintf (buf, "#5%s#0 [%s]%s, leading:\n\n", char_short (leader),
	   wound_total (leader), tactical_status (leader));
  buf[2] = toupper (buf[2]);

  std::set<CHAR_DATA*>::iterator i;
  ROOM_DATA *here = ch->room;
  for (i = leader->group->begin (); i != leader->group->end (); ++i)
    {
      CHAR_DATA *tch = (*i);
      if (tch->room != here || !CAN_SEE (ch, tch))
	continue;

      if (found != false)
	sprintf (buf + strlen (buf), ",\n");
      sprintf (buf + strlen (buf), "   #5%s#0 [%s]%s", char_short (tch),
	       wound_total (tch), tactical_status (tch));
      found = true;
    }
  strcat (buf, ".\n");

  if (!found)
    {
      send_to_char ("You aren't in a group.\n", ch);
      return;
    }

  send_to_char (buf, ch);
}
*/

void
followers_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time)
{
    CHAR_DATA *tch;
    ROOM_DATA *room_exit;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {

        if (tch == ch || GET_FLAG (tch, FLAG_LEAVING))
            continue;

        if (tch->following != ch)
        {
            if (!IS_RIDEE (tch) || tch->mount->following != ch)
                continue;
        }

        if (IS_HITCHEE (tch))
            continue;

        /* Check if this mob tch is allow to go to target room */

        if (IS_NPC (tch) && CAN_GO (tch, dir) && !isguarded (ch->room, dir) &&
                (room_exit = vnum_to_room (EXIT (tch, dir)->to_room)))
        {

            if (IS_MERCHANT (tch) &&
                    IS_SET (room_exit->room_flags, NO_MERCHANT))
                continue;

            if (tch->mob->access_flags &&
                    !(tch->mob->access_flags & room_exit->room_flags))
                continue;

            if (IS_SET (tch->act, ACT_STAY_ZONE) &&
                    tch->room->zone != room_exit->zone)
                continue;
        }

        if (GET_POS (tch) == SIT)
        {
            act ("You can't follow $N while sitting.",
                 false, tch, 0, ch, TO_CHAR);
            continue;
        }

        else if (GET_POS (tch) == REST)
        {
            act ("You can't follow $N while resting.",
                 false, tch, 0, ch, TO_CHAR);
            continue;
        }

        else if (GET_POS (tch) < FIGHT)
            continue;

        if (get_affect (tch, MAGIC_HIDDEN) && real_skill (tch, SKILL_SNEAK))
        {
            if (odds_sqrt (skill_level (tch, SKILL_SNEAK, 0)) >= number (1, 100)
                    || !would_reveal (tch))
            {
                magic_add_affect (tch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
            }
            else
            {
                remove_affect_type (tch, MAGIC_HIDDEN);
                act ("$n attempts to be stealthy.", true, tch, 0, 0, TO_ROOM);
            }
        }

        move (tch, "", dir, leave_time + arrive_time);
    }
}

void
follower_catchup (CHAR_DATA * ch)
{
    CHAR_DATA *tch;
    QE_DATA *qe;

    if (!ch->room)
        return;

    for (tch = ch->room->people; tch; tch = tch->next_in_room)
        if (ch->following == tch)
            break;

    if (!tch || !GET_FLAG (tch, FLAG_LEAVING) || !CAN_SEE (tch, ch)
            || IS_SWIMMING (tch))
        return;

    for (qe = quarter_event_list; qe->ch != tch; qe = qe->next)
        ;

    if (!qe)
        return;

    if (ch->aiming_at)
    {
        send_to_char ("You lose your aim as you move.\n", ch);

		remove_targeted(ch->aiming_at, ch);
        ch->aiming_at = NULL;
        ch->aim = 0;
    }

    //if((bow = get_equip (ch, WEAR_BOTH)) && bow->o.weapon.use_skill == SKILL_LONGBOW && bow->loaded)
    //  {
    //    command_interpreter(ch, "unload");
    //  }

    if (get_affect (ch, MAGIC_HIDDEN) && real_skill (ch, SKILL_SNEAK))
    {
        if (odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) >= number (1, 100)
                || !would_reveal (ch))
        {
            magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
        }
        else
        {
            remove_affect_type (ch, MAGIC_HIDDEN);
            act ("$n attempts to be stealthy.", true, ch, 0, 0, TO_ROOM);
        }
    }

    move (ch, "", qe->dir, qe->event_time + qe->arrive_time);
}


// called when fleeing or when getting extracted
void
stop_followers (CHAR_DATA * ch)
{
    CHAR_DATA *tch;

    for (tch = character_list; tch; tch = tch->next)
    {
        if (tch->deleted)
            continue;

        if (tch->following == ch)
            tch->following = 0;
    }
    //  if (ch->group)
    //    {
    //    ch->group->clear ();
    //  }
}

int num_followers (CHAR_DATA * ch)
{
    CHAR_DATA		*top_leader = NULL;
    CHAR_DATA		*tch = NULL;
    int group_count = 0;

    if (!(top_leader = ch->following))
        top_leader = ch;

    for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
        if (tch->following != top_leader)
            continue;
        if (!CAN_SEE (ch, tch))
            continue;
        group_count = group_count + 1;

    }

    return (group_count);

}
