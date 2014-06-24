/*------------------------------------------------------------------------\
|  dwellings.c : auto-genned player dwellings/areas   www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "server.h"
#include "structs.h"
#include "protos.h"

#include "utils.h"

void
	clone_room (ROOM_DATA * source_room, ROOM_DATA * targ_room, bool clone_exits, bool copy_objs)
{
	EXTRA_DESCR_DATA *exptr = NULL, *extra = NULL;
	int i = 0;

	if (!source_room || !targ_room)
		return;

	targ_room->room_flags = source_room->room_flags;
	targ_room->sector_type = source_room->sector_type;

	targ_room->name = str_dup (source_room->name);
	targ_room->description = str_dup (source_room->description);

	targ_room->zone = targ_room->vnum / ZONE_SIZE;

	if (clone_exits)
	{
		for (i = 0; i <= LAST_DIR; i++)
		{
			if (!source_room->dir_option[i])
				continue;
			CREATE (targ_room->dir_option[i], struct room_direction_data, 1);
			targ_room->dir_option[i]->general_description =
				str_dup (source_room->dir_option[i]->general_description);
			targ_room->dir_option[i]->keyword =
				str_dup (source_room->dir_option[i]->keyword);
			targ_room->dir_option[i]->exit_info =
				source_room->dir_option[i]->exit_info;
			targ_room->dir_option[i]->key = source_room->dir_option[i]->key;
			targ_room->dir_option[i]->to_room = 0;
			targ_room->dir_option[i]->pick_penalty =
				source_room->dir_option[i]->pick_penalty;
		}
	}

	if (source_room->ex_description)
	{
		for (exptr = source_room->ex_description; exptr; exptr = exptr->next)
		{
			CREATE (extra, EXTRA_DESCR_DATA, 1);
			extra->keyword = str_dup (exptr->keyword);
			extra->description = str_dup (exptr->description);
			extra->next = targ_room->ex_description;
			targ_room->ex_description = extra;
		}
	}

	if (source_room->extra)
	{
		CREATE (targ_room->extra, ROOM_EXTRA_DATA, 1);
		for (i = 0; i < WR_DESCRIPTIONS; i++)
		{
			if (source_room->extra->weather_desc[i])
				targ_room->extra->weather_desc[i] =
				str_dup (source_room->extra->weather_desc[i]);
		}
	}
	else
		targ_room->extra = NULL;

	if (copy_objs)
	{

		OBJ_DATA *tobj = NULL;
		OBJ_DATA *obj = NULL;
		for (tobj = source_room->contents; tobj; tobj = tobj->next)
		{
			if (obj = load_object(tobj->nVirtual))
			{
				obj_to_room(obj, targ_room->vnum);
			}
		}
	}
}


int
	delete_contiguous_temporary_rblock (ROOM_DATA * start_room, int from_dir, int outside_vnum)
{
  int i = 0;

  if (!start_room)
    return -1;

  for (i = 0; i <= LAST_DIR; i++)
    {

      if (i == from_dir || !start_room->dir_option[i])
	continue;

      // Don't want to delete the whole world outside the dwelling!

      if (i == OUTSIDE)
	continue;

      if (vnum_to_room (start_room->dir_option[i]->to_room) && vnum_to_room (start_room->dir_option[i]->to_room)->vnum >= 1000000)
	delete_contiguous_rblock (vnum_to_room (start_room->dir_option[i]->to_room),
				  rev_dir[i], outside_vnum);
    }

  // If this is a temporary dwelling type room that's being destroyed, let's make sure
  // anyone logged off in this room is set to the room just outside before we delete.

  // If there's no outside room designated, we'll set them to NOWHERE just to be safe.
  extern rpie::server engine;
  mysql_safe_query ("UPDATE %s.pfiles SET room = %d WHERE room = %d", (engine.get_config ("player_db")).c_str (), outside_vnum, start_room->vnum);

  rdelete (start_room);

  return 0;
}

int
	delete_contiguous_rblock (ROOM_DATA * start_room, int from_dir, int outside_vnum)
{
  int i = 0;

  if (!start_room)
    return -1;

  for (i = 0; i <= LAST_DIR; i++)
    {

      if (i == from_dir || !start_room->dir_option[i])
	continue;

      // Don't want to delete the whole world outside the dwelling!

      if (i == OUTSIDE)
	continue;

      if (vnum_to_room (start_room->dir_option[i]->to_room))
	delete_contiguous_rblock (vnum_to_room (start_room->dir_option[i]->to_room),
				  rev_dir[i], outside_vnum);
    }

  // If this is a temporary dwelling type room that's being destroyed, let's make sure
  // anyone logged off in this room is set to the room just outside before we delete.

  // If there's no outside room designated, we'll set them to NOWHERE just to be safe.
  extern rpie::server engine;
  mysql_safe_query ("UPDATE %s.pfiles SET room = %d WHERE room = %d", (engine.get_config ("player_db")).c_str (), outside_vnum, start_room->vnum);

  rdelete (start_room);

  return 0;
}

int
clone_contiguous_rblock (ROOM_DATA * start_room, int from_dir, bool copy_objs)
{
  ROOM_DATA *room;
  int i = 0, rnum = 0, dest_room = 0;

  if (!start_room)
    return -1;

  // Determine unused vnum for the new cloned room.

  for (rnum = 100000; rnum <= 999999; rnum++)
    {
      if (!vnum_to_room (rnum))
	break;
    }

  // Clone master room to new room, including all exit information.

  room = allocate_room (rnum);
  clone_room (start_room, room, true, copy_objs);

  // Link up exit to next room in block.

  for (i = 0; i <= LAST_DIR; i++)
    {

      if (i == from_dir || !room->dir_option[i])
	continue;

      if (vnum_to_room (start_room->dir_option[i]->to_room))
	dest_room =
	  clone_contiguous_rblock (vnum_to_room (start_room->dir_option[i]->to_room), rev_dir[i], copy_objs);

      if (dest_room == -1)
	continue;

      room->dir_option[i]->to_room = dest_room;
      vnum_to_room (dest_room)->dir_option[rev_dir[i]]->to_room = rnum;

    }

  // Return the VNUM of the new cloned room.

  return rnum;
}

ROOM_DATA *
generate_dwelling_room (OBJ_DATA * dwelling)
{
  ROOM_DATA *room;
  int target = 0;

  if (GET_ITEM_TYPE (dwelling) != ITEM_DWELLING)
    return NULL;

  if (dwelling->o.od.value[0] && (room = vnum_to_room (dwelling->o.od.value[0])))
    {
      return room;
    }
  else
    {
      if (!vnum_to_room (dwelling->o.od.value[5]))
	return NULL;

      target = clone_contiguous_rblock (vnum_to_room (dwelling->o.od.value[5]), -1, false);
      room = vnum_to_room (target);

      if (!room)
	return NULL;

      dwelling->o.od.value[0] = room->vnum;
    }

  if (!room)
    return NULL;

  CREATE (room->dir_option[OUTSIDE], struct room_direction_data, 1);

  room->dir_option[OUTSIDE]->general_description = str_dup ("");
  room->dir_option[OUTSIDE]->keyword = str_dup ("entryway");
  room->dir_option[OUTSIDE]->exit_info |= (1 << 0);
  room->dir_option[OUTSIDE]->key = dwelling->o.od.value[3];
  room->dir_option[OUTSIDE]->to_room = dwelling->in_room;
  room->dir_option[OUTSIDE]->pick_penalty = dwelling->o.od.value[4];

  return room;
}

OBJ_DATA *
find_dwelling_obj (int dwelling_room)
{
  ROOM_DATA *room;
  OBJ_DATA *tobj;

  for (room = full_room_list; room; room = room->lnext)
    {
      for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
	  if (GET_ITEM_TYPE (tobj) != ITEM_DWELLING)
	    continue;
	  if (tobj->o.od.value[0] == dwelling_room)
	    return tobj;
	}
    }

  return NULL;
}

void
do_pitch (CHAR_DATA * ch, char *argument, int cmd)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  int sector_type = 0;

  if (!*argument)
    {
      send_to_char ("What did you wish to pitch?\n", ch);
      return;
    }

  if (!(obj = get_obj_in_list_vis (ch, argument, ch->room->contents)))
    {
      send_to_char ("I don't see that here.\n", ch);
      return;
    }

  if (GET_ITEM_TYPE (obj) != ITEM_DWELLING)
    {
      send_to_char
	("Only portable dwellings, such as tents, may be pitched.\n", ch);
      return;
    }

  if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHABLE))
    {
      send_to_char
	("Only portable dwellings, such as tents, may be pitched.\n", ch);
      return;
    }

  sector_type = ch->room->sector_type;

  /*
  if (sector_type != SECT_WOODS && sector_type != SECT_FOREST &&
      sector_type != SECT_FIELD && sector_type != SECT_HILLS)
    {
      send_to_char
	("You may only camp in the woods, forest, a field or the hills.\n\r",
	 ch);
      return;
    }
  */

  sprintf (buf, "You begin pitching #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

  sprintf (buf, "$n begins pitching #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  ch->delay_type = DEL_PITCH;
  if (IS_MORTAL (ch))
    ch->delay = 30;
  else
    ch->delay = 1;
  ch->delay_obj = obj;
}

void
delayed_pitch (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  obj = ch->delay_obj;

  sprintf (buf, "You finish pitching #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

  sprintf (buf, "$n finishes pitching #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  obj->obj_flags.extra_flags |= ITEM_PITCHED;

  sprintf (buf, "%s has been erected here.", obj_short_desc (obj));
  buf[0] = toupper (buf[0]);

  obj->description = add_hash (buf);

  ch->delay_type = 0;
  ch->delay = 0;
  ch->delay_obj = 0;
}

void
do_dismantle (CHAR_DATA * ch, char *argument, int cmd)
{
  ROOM_DATA *room;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if (!*argument)
    {
      send_to_char ("What did you wish to dismantle?\n", ch);
      return;
    }

  if (!(obj = get_obj_in_list_vis (ch, argument, ch->room->contents)))
    {
      send_to_char ("I don't see that here.\n", ch);
      return;
    }

  if (GET_ITEM_TYPE (obj) != ITEM_DWELLING)
    {
      send_to_char
	("Only portable dwellings, such as tents, may be pitched.\n", ch);
      return;
    }

  if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHABLE))
    {
      send_to_char
	("Only portable dwellings, such as tents, may be pitched.\n", ch);
      return;
    }

  if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED))
    {
      send_to_char ("Only pitched tents can be dismantled.\n", ch);
      return;
    }

  if ((room = vnum_to_room (obj->o.od.value[0])))
    {
      if (room->contents)
	{
	  send_to_char
	    ("You'll need to clear out the inside of the tent before dismantling it.\n",
	     ch);
	  return;
	}
      if (room->people)
	{
	  send_to_char
	    ("You cannot dismantle the tent while there are still people inside!\n",
	     ch);
	  return;
	}
      room->occupants = 0;
    }

  sprintf (buf, "You dismantle #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

  sprintf (buf, "$n dismantles #2%s#0.", obj_short_desc (obj));
  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  obj->obj_flags.extra_flags &= ~(ITEM_VNPC | ITEM_PITCHED);

  if (vtoo (obj->nVirtual))
    obj->description = add_hash (vtoo (obj->nVirtual)->description);
  else
    {
      sprintf (buf, "%s is here.", obj_short_desc (obj));
      buf[0] = toupper (buf[0]);
      obj->description = add_hash (buf);
    }
}

void
fwrite_dwelling_room (ROOM_DATA * troom, FILE * fp)
{
  EXTRA_DESCR_DATA *exptr;
  struct room_prog *rp;
  int j;

  if (!troom->description)
    troom->description = add_hash ("No Description Set\n");

  fprintf (fp, "#%d\n%s~\n%s~\n",
	   troom->vnum, troom->name, troom->description);
  fprintf (fp, "%d %d %d\n",
	   troom->zone, troom->room_flags, troom->sector_type);

  if (world_version_out >= 1)
    fprintf (fp, "%d\n", troom->deity);

  if (troom->extra)
    {

      fprintf (fp, "A\n");

      for (j = 0; j < WR_DESCRIPTIONS; j++)
	fprintf (fp, "%s~\n", troom->extra->weather_desc[j] ?
		 troom->extra->weather_desc[j] : "");

      for (j = 0; j <= LAST_DIR; j++)
	fprintf (fp, "%s~\n", troom->extra->alas[j] ?
		 troom->extra->alas[j] : "");
    }

  for (j = 0; j <= LAST_DIR; j++)
    {

      if (!troom->dir_option[j])
	continue;

      if (IS_SET (troom->dir_option[j]->exit_info, EX_SECRET))
	if (IS_SET (troom->dir_option[j]->exit_info, EX_TRAP))
	  fprintf (fp, "B%d\n", j);
	else
	  fprintf (fp, "H%d\n", j);

      else if (IS_SET (troom->dir_option[j]->exit_info, EX_TRAP))
	fprintf (fp, "T%d\n", j);
      else
	fprintf (fp, "D%d\n", j);

      if (troom->dir_option[j]->general_description)
	fprintf (fp, "%s~\n", troom->dir_option[j]->general_description);
      else
	fprintf (fp, "~\n");

      if (troom->dir_option[j]->keyword)
	fprintf (fp, "%s~\n", troom->dir_option[j]->keyword);
      else
	fprintf (fp, "~\n");

			if (IS_SET (troom->dir_option[j]->exit_info, EX_ISGATE))
	fprintf (fp, "3");
      else if (IS_SET (troom->dir_option[j]->exit_info, EX_PICKPROOF))
	fprintf (fp, "2");
      else if (IS_SET (troom->dir_option[j]->exit_info, EX_LOCKED))
	fprintf (fp, "1");
      else if (IS_SET (troom->dir_option[j]->exit_info, EX_ISDOOR))
	fprintf (fp, "1");
      else
	fprintf (fp, "0");

      fprintf (fp, " %d ", troom->dir_option[j]->key);

      if (world_version_out >= 1)
	fprintf (fp, " %d ", troom->dir_option[j]->pick_penalty);

      if (troom->dir_option[j]->to_room == NOWHERE)
	fprintf (fp, "-1\n");
      else
	fprintf (fp, "%d\n", troom->dir_option[j]->to_room);
    }

  for (j = 0; j <= LAST_DIR; j++)
    {
      if (troom->secrets[j])
	{
	  fprintf (fp, "Q%d\n%d\n", j, troom->secrets[j]->diff);
	  fprintf (fp, "%s~\n", troom->secrets[j]->stext);
	}
    }

  for (exptr = troom->ex_description; exptr; exptr = exptr->next)
    if (exptr->description)
      fprintf (fp, "E\n%s~\n%s~\n", exptr->keyword, exptr->description);

  if (troom->wdesc)
    fprintf (fp, "W\n%d\n%s~\n",
	     troom->wdesc->language, troom->wdesc->description);

  if (troom->prg)
    {
      for (rp = troom->prg; rp; rp = rp->next)
	{
	  fprintf (fp, "P\n");
	  fprintf (fp, "%s~\n", rp->command);
	  fprintf (fp, "%s~\n", rp->keys);
	  fprintf (fp, "%s~\n", rp->prog);
	}
    }

  fprintf (fp, "S\n");
}

int
save_dwelling_rooms ()
{
  char buf[MAX_INPUT_LENGTH];
  ROOM_DATA *troom;
  FILE *fr;
  int room_good;
  int n;
  int empty_rooms = 0;
  static int total_empty_rooms = 0;

  sprintf (buf, "Saving auto-genned temprooms to disk.");
  system_log (buf, false);

  if (!(fr = fopen ("temprooms", "w+")))
    return -1;

  for (troom = full_room_list; troom; troom = troom->lnext)
    {

      if (troom->vnum >= 100000)
	{

	  room_good = 0;

	  for (n = 0; n <= LAST_DIR; n++)
	    if (troom->dir_option[n] && troom->dir_option[n]->to_room > 0)
	      room_good = 1;

	  if (troom->contents || troom->people)
	    room_good = 1;

	  if (strncmp (troom->description, "No Description Set", 18))
	    room_good = 1;

	  if (room_good)
	    {
	      fwrite_dwelling_room (troom, fr);
	    }
	  else
	    {
	      empty_rooms++;
	      total_empty_rooms++;
	    }
	}
    }

  fprintf (fr, "$~\n");
  fclose (fr);

  return 0;
}

void
load_dwelling_rooms ()
{
  FILE *fl;
  ROOM_DATA *room;
  int virtual_nr = 0, zon = 100, i = 0, flag = 0, tmp, sdir;
  char *temp, chk[50];
  struct room_prog *r_prog;
  struct secret *r_secret;
  struct extra_descr_data *new_descr;
  struct written_descr_data *w_desc;

  if (!(fl = fopen ("temprooms", "r")))
    return;


  do
    {
      fscanf (fl, " #%d\n", &virtual_nr);
      temp = fread_string (fl);

      if (!temp)
	continue;

      if ((flag = (*temp != '$')))
	{
	  room = allocate_room (virtual_nr);
	  room->zone = zon;
	  room->name = temp;
	  room->description = fread_string (fl);
	  fscanf (fl, "%d", &tmp);

	  fscanf (fl, " %d ", &tmp);
	  room->room_flags = tmp;

	  /* The STORAGE bit is set when loading in shop keepers */

	  room->room_flags &= ~(STORAGE | PC_ENTERED);
	  room->room_flags |= SAVE;

	  fscanf (fl, " %d ", &tmp);
	  room->sector_type = tmp;

	  fscanf (fl, "%d\n", &tmp);
	  room->deity = tmp;

	  room->contents = 0;
	  room->people = 0;
	  room->light = 0;

	  for (tmp = 0; tmp <= LAST_DIR; tmp++)
	    room->dir_option[tmp] = 0;

	  room->ex_description = 0;
	  room->wdesc = 0;
	  room->prg = 0;
	  for (tmp = 0; tmp <= LAST_DIR; tmp++)
	    room->secrets[tmp] = 0;

	  for (;;)
	    {
	      fscanf (fl, " %s \n", chk);

	      if (*chk == 'D')	/* direction field */
		setup_dir (fl, room, atoi (chk + 1), 0);

	      else if (*chk == 'H')	/* Secret (hidden) */
		setup_dir (fl, room, atoi (chk + 1), 1);

	      else if (*chk == 'T')	/* Trapped door */
		setup_dir (fl, room, atoi (chk + 1), 2);

	      else if (*chk == 'B')	/* Trapped hidden door */
		setup_dir (fl, room, atoi (chk + 1), 3);

	      else if (*chk == 'Q')
		{		/* Secret search desc */
		  r_secret =
		    (struct secret *) get_perm (sizeof (struct secret));
		  sdir = atoi (chk + 1);
		  fscanf (fl, "%d\n", &tmp);
		  r_secret->diff = tmp;
		  r_secret->stext = fread_string (fl);
		  room->secrets[sdir] = r_secret;
		}

	      else if (*chk == 'E')	/* extra description field */
		{
		  struct extra_descr_data *tmp_extra;

		  new_descr =
		    (struct extra_descr_data *)
		    get_perm (sizeof (struct extra_descr_data));
		  new_descr->keyword = fread_string (fl);
		  new_descr->description = fread_string (fl);
		  new_descr->next = NULL;

		  if (!room->ex_description)
		    room->ex_description = new_descr;
		  else
		    {
		      tmp_extra = room->ex_description;

		      while (tmp_extra->next)
			tmp_extra = tmp_extra->next;

		      tmp_extra->next = new_descr;
		    }
		}

	      else if (*chk == 'W')
		{
		  w_desc =
		    (struct written_descr_data *)
		    get_perm (sizeof (struct written_descr_data));
		  fscanf (fl, "%d\n", &tmp);
		  w_desc->language = tmp;
		  w_desc->description = fread_string (fl);
		  room->wdesc = w_desc;

		}
	      else if (*chk == 'P')
		{
		  struct room_prog *tmp_prg;
		  r_prog =
		    (struct room_prog *) get_perm (sizeof (struct room_prog));
		  r_prog->command = fread_string (fl);
		  r_prog->keys = fread_string (fl);
		  r_prog->prog = fread_string (fl);
		  r_prog->next = NULL;

		  /* Make sure that the room program is stored at
		     end of the list.  This way when the room is
		     saved, the rprogs get saved in the same order
		     - Rassilon
		   */

		  if (!room->prg)
		    room->prg = r_prog;
		  else
		    {
		      tmp_prg = room->prg;

		      while (tmp_prg->next)
			tmp_prg = tmp_prg->next;

		      tmp_prg->next = r_prog;
		    }
		}

	      else if (*chk == 'A')
		{		/* Additional descriptions */

		  CREATE (room->extra, ROOM_EXTRA_DATA, 1);

		  for (i = 0; i < WR_DESCRIPTIONS; i++)
		    {
		      room->extra->weather_desc[i] = fread_string (fl);
		      if (!strlen (room->extra->weather_desc[i]))
			room->extra->weather_desc[i] = NULL;
		    }

		  for (i = 0; i <= LAST_DIR; i++)
		    {
		      room->extra->alas[i] = fread_string (fl);
		      if (!strlen (room->extra->alas[i]))
			room->extra->alas[i] = NULL;
		    }
		}

	      else if (*chk == 'S')	/* end of current room */
		break;
	    }
	}
    }
  while (flag);

  fclose (fl);
}
