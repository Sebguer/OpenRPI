/*------------------------------------------------------------------------\
|  creation.c : Creation Magic Technique Module       www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, Derived under license from DIKU GAMMA (0.0).        |
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
creation_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		       int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		       int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		    int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  if (target_type == TARGET_OTHER)
    {
    }

  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
creation_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  OBJ_DATA *tobj;
  char buf[MAX_STRING_LENGTH];

  if ((target_type == TARGET_OTHER_INVENTORY || target_type == TARGET_OBJ)
      && GET_ITEM_TYPE ((tobj = (OBJ_DATA *) target)) == ITEM_DRINKCON)
    {
      sprintf (buf, "#2%s#0 suddenly fills with cool, clear water.",
	       tobj->short_description);
      buf[2] = toupper (buf[2]);
      tobj->o.od.value[2] = 4;
      tobj->o.od.value[1] = spell->a.spell.magnitude * 2;
      tobj->o.od.value[1] = MIN (tobj->o.od.value[0], tobj->o.od.value[1]);
      act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      return;
    }

  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}
