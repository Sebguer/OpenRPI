/*------------------------------------------------------------------------\
|  transformation.c : Transformation Magic Module     www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <stdio.h>


#ifndef MACOSX
#include <malloc.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "utility.h"

void
transformation_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type)
{
  WOUND_DATA *wound, *next_wound;
  CHAR_DATA *tch;
  OBJ_DATA *tobj, *obj, *next_obj;
  char buf[MAX_STRING_LENGTH];
  bool base = false, extended = false;

  if (target_type == TARGET_OBJ)
    {
      tobj = (OBJ_DATA *) target;
      if (GET_ITEM_TYPE (tobj) == ITEM_CONTAINER
	  && tobj->nVirtual == VNUM_CORPSE)
	{
	  if ((spell->a.spell.discipline == SKILL_VOODOO
	       || spell->a.spell.discipline == SKILL_VOODOO)
	      && spell->a.spell.magnitude < MAGNITUDE_POWERFUL)
	    {
	      sprintf (buf,
		       "Suddenly, #2%s#0 appears to mend slightly, looking decidedly less decayed.",
		       tobj->short_description);
	      act (buf, false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      tobj->obj_timer += spell->a.spell.magnitude;
	      return;
	    }
	  else
	    if ((spell->a.spell.discipline == SKILL_VOODOO
		 || spell->a.spell.discipline == SKILL_VOODOO)
		&& spell->a.spell.magnitude >= MAGNITUDE_POWERFUL)
	    {
	      sprintf (buf,
		       "A faint white aura briefly limns #2%s#0 before fading away.",
		       tobj->short_description);
	      act (buf, false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      tobj->obj_flags.extra_flags &= ~ITEM_TIMER;
	      return;
	    }
	  if (spell->a.spell.discipline == SKILL_VOODOO
	      && spell->a.spell.magnitude < MAGNITUDE_POWERFUL)
	    {
	      sprintf (buf,
		       "Before your very eyes, #2%s#0 appears to putrefy with startling abruptness.",
		       tobj->short_description);
	      act (buf, false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      tobj->obj_timer -= spell->a.spell.magnitude;
	      tobj->obj_timer = MAX (1, tobj->obj_timer);
	      return;
	    }
	  else if (spell->a.spell.discipline == SKILL_VOODOO
		   && spell->a.spell.magnitude >= MAGNITUDE_POWERFUL)
	    {
	      sprintf (buf,
		       "Without warning, #2%s#0 putrifies wretchedly before decaying into nothingness.",
		       tobj->short_description);
	      act (buf, false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      for (obj = tobj->contains; obj; obj = next_obj)
		{
		  next_obj = obj->next_content;
		  obj->obj_timer = 12;
		  obj_from_obj (&obj, 0);
		  if (tobj->in_room != NOWHERE)
		    obj_to_room (obj, tobj->in_room);
		  else
		    extract_obj (obj);
		}
	      extract_obj (tobj);
	      return;
	    }
	}
    }

  else if (target_type == TARGET_OTHER || target_type == TARGET_SELF)
    {
      tch = (CHAR_DATA *) target;
      if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  sprintf (buf,
		   "You suddenly feel quite envigorated, tingling with health and well-being.\n");
	  send_to_char (buf, tch);
	  sprintf (buf,
		   "$n's eyes suddenly take on an envigorated sparkle, and $s posture straightens.");
	  act (buf, true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
	  tch->move += spell->a.spell.magnitude * 10;
	  return;
	}
      else if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  for (wound = tch->wounds; wound; wound = next_wound)
	    {
	      next_wound = wound->next;
	      wound->healerskill =
		ch->skills[SKILL_VOODOO] + (spell->a.spell.magnitude * 5);
	      wound->healerskill = MIN (90, wound->healerskill);
	      base = true;
	      if (wound->infection)
		wound->infection = -1;
	      if (wound->bleeding)
		wound->bleeding = 0;
	      if (spell->a.spell.magnitude > MAGNITUDE_STAUNCH)
		{
		  if (wound->infection)
		    wound->infection = 0;
		  adjust_wound (tch, wound, spell->a.spell.magnitude * -1);
		  extended = true;
		}
	    }
	  sprintf (buf,
		   "You are briefly surrounded by a cool, soothing nimbus of white light.\n");
	  send_to_char (buf, tch);

	  if (base)
	    {
	      sprintf (buf,
		       "\nYour wounds ache less beneath the touch of the magical radiance.\n");
	      send_to_char (buf, tch);
	    }

	  if (extended)
	    {
	      sprintf (buf,
		       "\nBefore your very eyes, your wounds begin to gently mend themselves.\n");
	      send_to_char (buf, tch);
	    }

	  sprintf (buf,
		   "$n is briefly surrounded by a soft nimbus of white light.");
	  act (buf, true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);

	  return;
	}
      else if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  sprintf (buf,
		   "You are briefly surrounded by an inky cloud of fetid darkness.\n");
	  send_to_char (buf, tch);

	  sprintf (buf, "$n is momentarily enveloped in an inky darkness.");
	  act (buf, true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);

	  for (wound = tch->wounds; wound; wound = next_wound)
	    {
	      next_wound = wound->next;
	      if (((ch->skills[SKILL_VOODOO] / 5 +
		    spell->a.spell.magnitude) > tch->con)
		  || number (1, 25) > tch->con)
		{
		  wound->infection = -1;
		  wound->healerskill = 0;
		  base = true;
		}
	      if (spell->a.spell.magnitude > MAGNITUDE_STAUNCH)
		{
		  if (adjust_wound (tch, wound, spell->a.spell.magnitude))
		    return;
		  extended = true;
		}
	    }

	  if (base)
	    {
	      sprintf (buf,
		       "\nYour wounds ache at the darkness' touch, before slowly beginning to pustulate.\n");
	      send_to_char (buf, tch);
	    }

	  if (extended)
	    {
	      sprintf (buf,
		       "\nHorrified, you watch as your wounds seem to worsen before your very eyes.\n");
	      send_to_char (buf, tch);
	    }

	  criminalize (ch, tch, tch->room->zone, CRIME_KILL);

	  return;
	}
    }

  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			       void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  ROOM_DIRECTION_DATA *exit;
  char buf[MAX_STRING_LENGTH];

  if (target_type == TARGET_EXIT && target)
    {
      exit = (ROOM_DIRECTION_DATA *) target;
      if (IS_SET (exit->exit_info, EX_LOCKED) && exit->pick_penalty != -1)
	{
	  if (spell->a.spell.magnitude * 7 >= exit->pick_penalty)
	    {
	      sprintf (buf,
		       "The portal shudders momentarily before a *click* is heard as its lock gives way.");
	      act (buf, false, ch, 0, 0, TO_CHAR);
	      act (buf, false, ch, 0, 0, TO_ROOM);
	      exit->exit_info &= ~EX_LOCKED;
	      return;
	    }
	}
      else if (IS_SET (exit->exit_info, EX_CLOSED)
	       && !IS_SET (exit->exit_info, EX_LOCKED))
	{
	  sprintf (buf,
		   "A nearly inaudible hiss is heard as the portal is sealed to its frame.");
	  act (buf, false, ch, 0, 0, TO_CHAR);
	  act (buf, false, ch, 0, 0, TO_ROOM);
	  exit->exit_info |= EX_LOCKED;
	  exit->pick_penalty = spell->a.spell.magnitude * 7;
	  return;
	}
    }

  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
transformation_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}
