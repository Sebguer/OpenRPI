/*------------------------------------------------------------------------\
|  perception.c : Perception Magic Technique Module   www.middle-earth.us |
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
perception_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		       int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		       int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}

void
perception_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type)
{
  send_to_char
    ("Your incantation, though complete, does not seem to have any effect.\n",
     ch);
}
