/*------------------------------------------------------------------------\
|  control.c : Control Magic Technique Module         www.middle-earth.us |
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

void
control_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		    int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		   int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		    int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
control_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		     int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}
