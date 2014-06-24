
/*------------------------------------------------------------------------\
|  magic.c : Magic/Psionics Module                    www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>

#ifndef MACOSX
#include <malloc.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "utility.h"

#define s(a) send_to_char (a "\n\r", ch);

int spell_blend (CHAR_DATA * ch, CHAR_DATA * victim, int sn);
int spell_infravision (CHAR_DATA * ch, CHAR_DATA * victim, int sn);

void magic_cure_poison (CHAR_DATA * ch, CHAR_DATA * target);
void magic_infravision (CHAR_DATA * ch, CHAR_DATA * victim);
void magic_slipperiness (CHAR_DATA * ch, CHAR_DATA * victim);
void magic_invulnerability (CHAR_DATA * ch, CHAR_DATA * victim);

#define FRIEND_CHARM		1
#define FRIEND_GROUP		2
#define FRIEND_SELF		4
#define FRIEND_CHURCH		16

#define NUM_NIGHTMARES		0

const char *magnitudes[] = {
  "Not Found",
  "Subtle",
  "Gentle",
  "Soft",
  "Stolid",
  "Staunch",
  "Vigorous",
  "Vibrant",
  "Potent",
  "Powerful",
  "Dire",
  "\n"
};

const char *techniques[] = {
  "Not Found",
  "Create",
  "Destroy",
  "Transform",
  "Perceive",
  "Control",
  "\n"
};

const char *forms[] = {
  "Not Found",
  "Animal",
  "Plant",
  "Image",
  "Light",
  "Darkness",
  "Power",
  "Mind",
  "Spirit",
  "Air",
  "Earth",
  "Fire",
  "Water",
  "\n"
};

const char *forms2[] = {
  "Not Found",
  "Animal",
  "Vegetation",
  "Illusion",
  "Radiance",
  "Shadow",
  "Power",
  "Intellect",
  "Soul",
  "Wind",
  "Earth",
  "Flame",
  "Water",
  "\n"
};

extern char *nightmare[];

char *
spell_activity_echo (int id, int affect_type)
{
  if (strtol (lookup_spell_variable (id, VAR_AFFECT1), NULL, 10) ==
      affect_type)
    return lookup_spell_variable (id, VAR_ACT1);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT2), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_ACT2);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT3), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_ACT3);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT4), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_ACT4);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT5), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_ACT5);

  return NULL;
}

char *
spell_fade_echo (int id, int affect_type)
{
  return NULL;

  if (id < 0)
    {
      return NULL;
    }
  if (strtol (lookup_spell_variable (id, VAR_AFFECT1), NULL, 10) ==
      affect_type)
    return lookup_spell_variable (id, VAR_FADE1);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT2), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_FADE2);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT3), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_FADE3);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT4), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_FADE4);
  else if (strtol (lookup_spell_variable (id, VAR_AFFECT5), NULL, 10) ==
	   affect_type)
    return lookup_spell_variable (id, VAR_FADE5);

  return NULL;
}

int
knows_spell (CHAR_DATA * ch, int id)
{
  int i = 0;

  if (!IS_MORTAL (ch) || IS_NPC (ch))
    return 1;

  for (i = 0; i < MAX_LEARNED_SPELLS; i++)
    {
      if (ch->spells[i][0] < 1)
	continue;
      if (ch->spells[i][0] == id)
	return 1;
    }

  return 0;
}

void
set_spell_proficiency (CHAR_DATA * ch, int id, int set_to)
{
  int i = 0;

  if (!knows_spell (ch, id))
    {
      for (i = 0; i < MAX_LEARNED_SPELLS; i++)
	{
	  if (ch->spells[i][0] >= 1)
	    continue;
	  ch->spells[i][0] = id;
	  ch->spells[i][1] = set_to;
	  return;
	}
      return;
    }

  for (i = 0; i < MAX_LEARNED_SPELLS; i++)
    {
      if (ch->spells[i][0] != id)
	continue;
      ch->spells[i][0] = set_to;
    }
}

void
remove_spell (CHAR_DATA * ch, int id)
{
  int i = 0;

  for (i = 0; i < MAX_LEARNED_SPELLS; i++)
    {
      if (ch->spells[i][0] != id)
	continue;
      ch->spells[i][0] = 0;
      ch->spells[i][1] = 0;
    }
}

int
spell_lookup (char *spell_name)
{
  SPELL_DATA *spell_info;
  int sn = 0;

  spell_info = spell_table;

  while (spell_info->name)
    {
      if (!strncasecmp (spell_info->name, spell_name, strlen (spell_name)))
	return sn;
      spell_info++;
      sn++;
    }

  return -1;
}

int
caster_type (CHAR_DATA * ch)
{
  int white = 0, grey = 0, black = 0;

  white = ch->skills[SKILL_VOODOO];
  grey = ch->skills[SKILL_VOODOO];
  black = ch->skills[SKILL_VOODOO];

  if (!white && !grey && !black)
    return CASTER_NONE;
  else if (white >= grey && white >= black)
    return CASTER_WHITE;
  else if (grey >= white && grey >= black)
    return CASTER_GREY;
  else if (black >= white && black >= grey)
    return CASTER_BLACK;

  return CASTER_NONE;
}

int
caster_magic_skill (CHAR_DATA * ch)
{
  if (caster_type (ch) == CASTER_WHITE)
    return ch->skills[SKILL_VOODOO];
  else if (caster_type (ch) == CASTER_GREY)
    return ch->skills[SKILL_VOODOO];
  else if (caster_type (ch) == CASTER_BLACK)
    return ch->skills[SKILL_VOODOO];

  return 0;
}

void
do_prepare (CHAR_DATA * ch, char *argument, int cmd)
{
  int id = 0;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if (!*argument)
    {
      send_to_char ("Which spell did you wish to prepare?\n", ch);
      return;
    }

  if ((id = lookup_spell_id (argument)) == -1)
    {
      send_to_char ("I know of no such spell.\n", ch);
      return;
    }

  if (!knows_spell (ch, id))
    {
      send_to_char ("You have no knowledge of such a spell.\n", ch);
      return;
    }

  sprintf (name, "%s", lookup_spell_variable (id, VAR_NAME));

  if (caster_type (ch) == CASTER_WHITE)
    {
      sprintf (buf,
	       "You lift your voice in delicate, haunting song, carefully shaping the syllables to summon '%s'.",
	       name);
      sprintf (buf2, "#5%s#0 lifts %s voice in delicate, haunting song.",
	       char_short (ch), HSHR (ch));
      buf2[2] = toupper (buf2[2]);
    }
  else if (caster_type (ch) == CASTER_GREY)
    {
      sprintf (buf,
	       "You begin chanting the ancient words of power to '%s,' your enunciation tightly-controlled and meticulous.",
	       name);
      sprintf (buf2,
	       "#5%s#0 begins chanting, %s enunciation deliberate and methodical.",
	       char_short (ch), HSHR (ch));
      buf2[2] = toupper (buf2[2]);
    }
  else if (caster_type (ch) == CASTER_BLACK)
    {
      sprintf (buf,
	       "You begin intoning the words of power to bind '%s,' your voice gutteral and thrumming with dark power.",
	       name);
      sprintf (buf2,
	       "#5%s#0 begins a gutteral incantation, %s voice thrumming with power.",
	       char_short (ch), HSHR (ch));
      buf2[2] = toupper (buf2[2]);
    }
  else
    {
      send_to_char ("You aren't a spellcaster!\n", ch);
      return;
    }

  act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
  act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  ch->preparing_id = id;
  ch->preparing_time = 0;
}

int
get_discipline_creation (AFFECTED_TYPE * spell)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      return SKILL_VOODOO;
    case FORM_PLANT:
      return SKILL_VOODOO;
    case FORM_IMAGE:
      return SKILL_VOODOO;
    case FORM_LIGHT:
      return SKILL_VOODOO;
    case FORM_DARKNESS:
      return SKILL_VOODOO;
    case FORM_POWER:
      return SKILL_VOODOO;
    case FORM_MIND:
      return SKILL_VOODOO;
    case FORM_SPIRIT:
      return SKILL_VOODOO;
    case FORM_AIR:
      return SKILL_VOODOO;
    case FORM_EARTH:
      return SKILL_VOODOO;
    case FORM_FIRE:
      return SKILL_VOODOO;
    case FORM_WATER:
      return SKILL_VOODOO;
    }

  return 0;
}

int
get_discipline_destruction (AFFECTED_TYPE * spell)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      return SKILL_VOODOO;
    case FORM_PLANT:
      return SKILL_VOODOO;
    case FORM_IMAGE:
      return SKILL_VOODOO;
    case FORM_LIGHT:
      return SKILL_VOODOO;
    case FORM_DARKNESS:
      return SKILL_VOODOO;
    case FORM_POWER:
      return SKILL_VOODOO;
    case FORM_MIND:
      return SKILL_VOODOO;
    case FORM_SPIRIT:
      return SKILL_VOODOO;
    case FORM_AIR:
      return SKILL_VOODOO;
    case FORM_EARTH:
      return SKILL_VOODOO;
    case FORM_FIRE:
      return SKILL_VOODOO;
    case FORM_WATER:
      return SKILL_VOODOO;
    }

  return 0;
}

int
get_discipline_transformation (AFFECTED_TYPE * spell)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      return SKILL_VOODOO;
    case FORM_PLANT:
      return SKILL_VOODOO;
    case FORM_IMAGE:
      return SKILL_VOODOO;
    case FORM_LIGHT:
      return SKILL_VOODOO;
    case FORM_DARKNESS:
      return SKILL_VOODOO;
    case FORM_POWER:
      return SKILL_VOODOO;
    case FORM_MIND:
      return SKILL_VOODOO;
    case FORM_SPIRIT:
      return SKILL_VOODOO;
    case FORM_AIR:
      return SKILL_VOODOO;
    case FORM_EARTH:
      return SKILL_VOODOO;
    case FORM_FIRE:
      return SKILL_VOODOO;
    case FORM_WATER:
      return SKILL_VOODOO;
    }

  return 0;
}

int
get_discipline_control (AFFECTED_TYPE * spell)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      return SKILL_VOODOO;
    case FORM_PLANT:
      return SKILL_VOODOO;
    case FORM_IMAGE:
      return SKILL_VOODOO;
    case FORM_LIGHT:
      return SKILL_VOODOO;
    case FORM_DARKNESS:
      return SKILL_VOODOO;
    case FORM_POWER:
      return SKILL_VOODOO;
    case FORM_MIND:
      return SKILL_VOODOO;
    case FORM_SPIRIT:
      return SKILL_VOODOO;
    case FORM_AIR:
      return SKILL_VOODOO;
    case FORM_EARTH:
      return SKILL_VOODOO;
    case FORM_FIRE:
      return SKILL_VOODOO;
    case FORM_WATER:
      return SKILL_VOODOO;
    }

  return 0;
}

int
get_discipline_perception (AFFECTED_TYPE * spell)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      return SKILL_VOODOO;
    case FORM_PLANT:
      return SKILL_VOODOO;
    case FORM_IMAGE:
      return SKILL_VOODOO;
    case FORM_LIGHT:
      return SKILL_VOODOO;
    case FORM_DARKNESS:
      return SKILL_VOODOO;
    case FORM_POWER:
      return SKILL_VOODOO;
    case FORM_MIND:
      return SKILL_VOODOO;
    case FORM_SPIRIT:
      return SKILL_VOODOO;
    case FORM_AIR:
      return SKILL_VOODOO;
    case FORM_EARTH:
      return SKILL_VOODOO;
    case FORM_FIRE:
      return SKILL_VOODOO;
    case FORM_WATER:
      return SKILL_VOODOO;
    }

  return 0;
}

int
get_discipline (AFFECTED_TYPE * spell)
{
  int disc = 0;

  switch (spell->a.spell.technique)
    {
    case TECHNIQUE_CREATION:
      disc = get_discipline_creation (spell);
      break;
    case TECHNIQUE_DESTRUCTION:
      disc = get_discipline_destruction (spell);
      break;
    case TECHNIQUE_CONTROL:
      disc = get_discipline_control (spell);
      break;
    case TECHNIQUE_TRANSFORMATION:
      disc = get_discipline_transformation (spell);
      break;
    case TECHNIQUE_PERCEPTION:
      disc = get_discipline_perception (spell);
      break;
    }

  return disc;
}

int
get_magnitude (char *word)
{
  int i;

  for (i = 1; *magnitudes[i] != '\n'; i++)
    {
      if (!strn_cmp (word, magnitudes[i], strlen (magnitudes[i])))
	return i;
    }

  return 0;
}

int
get_form (char *word)
{
  int i;

  for (i = 1; *forms[i] != '\n'; i++)
    {
      if (!strn_cmp (word, forms[i], strlen (forms[i])))
	return i;
    }

  for (i = 1; *forms2[i] != '\n'; i++)
    {
      if (!strn_cmp (word, forms2[i], strlen (forms2[i])))
	return i;
    }

  return 0;
}

int
get_technique (char *word)
{
  int i;

  for (i = 1; *techniques[i] != '\n'; i++)
    {
      if (!strn_cmp (word, techniques[i], strlen (techniques[i])))
	return i;
    }

  return 0;
}

int
calculate_mana_cost (CHAR_DATA * ch, AFFECTED_TYPE * spell)
{
  float cost = 0;
  int skill_mod = 0;

  if (spell->a.spell.technique == TECHNIQUE_DESTRUCTION
      || spell->a.spell.technique == TECHNIQUE_TRANSFORMATION)
    cost = 25;
  else if (spell->a.spell.technique == TECHNIQUE_CREATION
	   || spell->a.spell.technique == TECHNIQUE_CONTROL)
    cost = 20;
  else
    cost = 10;

  cost += number (1, 7);
  cost -= ch->aur / 5;

  cost += spell->a.spell.magnitude * 3.5;

  if (get_discipline (spell) != spell->a.spell.discipline)
    {
      if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  if (get_discipline (spell) == SKILL_VOODOO)
	    cost *= 2.5;
	  else
	    cost *= 1.5;
	}
      else if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  if (get_discipline (spell) == SKILL_VOODOO)
	    cost *= 2.5;
	  else
	    cost *= 1.5;
	}
      else
	cost *= 1.5;
    }

  skill_mod = 100 - ch->skills[spell->a.spell.discipline];
  skill_mod = MAX (1, skill_mod);

  cost *= (((float)skill_mod) / 100.0);

  cost = MAX (1.0f, cost);

  return (int) cost;
}

void
prepare_magic (CHAR_DATA * ch, int technique, int form, int magnitude)
{
}

void
determine_form (CHAR_DATA * ch, char *argument, int technique)
{
  char buf[MAX_STRING_LENGTH];
  int mag = 0, form = 0;

  if (!technique)
    return;

  argument = one_argument (argument, buf);

  if ((mag = get_magnitude (buf)) > 0)
    argument = one_argument (argument, buf);

  form = get_form (buf);

  prepare_magic (ch, technique, form, mag);
}

void
determine_technique (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int technique = 0;

  argument = one_argument (argument, buf);

  technique = get_technique (buf);

  determine_form (ch, argument, technique);
}

void
magic_incantation (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (!str_cmp (buf, "I"))
    determine_technique (ch, argument);
}

int
is_incantation (char *argument)
{
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (str_cmp (buf, "I"))
    return 0;

  argument = one_argument (argument, buf);

  if (!get_technique (buf))
    return 0;

  argument = one_argument (argument, buf);

  if (!get_magnitude (buf) && !get_form (buf))
    return 0;

  if (*argument)
    {
      if (!get_form (argument))
	return 0;
    }

  return 1;
}

struct spell_data spell_table[] = {
};

char *
show_enchantment (ENCHANTMENT_DATA * enchantment)
{
  static char buf[MAX_STRING_LENGTH];

  *buf = '\0';

  if (!str_cmp (enchantment->name, "Tindome Miriel"))
    sprintf (buf,
	     "#CA small sphere of translucent azure radiance hovers nearby.#0\n");

  return buf;
}

void
creation_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		int target_type)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      creation_animal_spell (ch, spell, target, target_type);
      break;
    case FORM_PLANT:
      creation_plant_spell (ch, spell, target, target_type);
      break;
    case FORM_IMAGE:
      creation_image_spell (ch, spell, target, target_type);
      break;
    case FORM_LIGHT:
      creation_light_spell (ch, spell, target, target_type);
      break;
    case FORM_DARKNESS:
      creation_darkness_spell (ch, spell, target, target_type);
      break;
    case FORM_POWER:
      creation_power_spell (ch, spell, target, target_type);
      break;
    case FORM_MIND:
      creation_mind_spell (ch, spell, target, target_type);
      break;
    case FORM_SPIRIT:
      creation_spirit_spell (ch, spell, target, target_type);
      break;
    case FORM_AIR:
      creation_air_spell (ch, spell, target, target_type);
      break;
    case FORM_EARTH:
      creation_earth_spell (ch, spell, target, target_type);
      break;
    case FORM_FIRE:
      creation_fire_spell (ch, spell, target, target_type);
      break;
    case FORM_WATER:
      creation_water_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char
	("There has been an error. The spell's form was not recognized.\n",
	 ch);
    }
}

void
destruction_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		   int target_type)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      destruction_animal_spell (ch, spell, target, target_type);
      break;
    case FORM_PLANT:
      destruction_plant_spell (ch, spell, target, target_type);
      break;
    case FORM_IMAGE:
      destruction_image_spell (ch, spell, target, target_type);
      break;
    case FORM_LIGHT:
      destruction_light_spell (ch, spell, target, target_type);
      break;
    case FORM_DARKNESS:
      destruction_darkness_spell (ch, spell, target, target_type);
      break;
    case FORM_POWER:
      destruction_power_spell (ch, spell, target, target_type);
      break;
    case FORM_MIND:
      destruction_mind_spell (ch, spell, target, target_type);
      break;
    case FORM_SPIRIT:
      destruction_spirit_spell (ch, spell, target, target_type);
      break;
    case FORM_AIR:
      destruction_air_spell (ch, spell, target, target_type);
      break;
    case FORM_EARTH:
      destruction_earth_spell (ch, spell, target, target_type);
      break;
    case FORM_FIRE:
      destruction_fire_spell (ch, spell, target, target_type);
      break;
    case FORM_WATER:
      destruction_water_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char
	("There has been an error. The spell's form was not recognized.\n",
	 ch);
    }
}

void
transformation_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		      int target_type)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      transformation_animal_spell (ch, spell, target, target_type);
      break;
    case FORM_PLANT:
      transformation_plant_spell (ch, spell, target, target_type);
      break;
    case FORM_IMAGE:
      transformation_image_spell (ch, spell, target, target_type);
      break;
    case FORM_LIGHT:
      transformation_light_spell (ch, spell, target, target_type);
      break;
    case FORM_DARKNESS:
      transformation_darkness_spell (ch, spell, target, target_type);
      break;
    case FORM_POWER:
      transformation_power_spell (ch, spell, target, target_type);
      break;
    case FORM_MIND:
      transformation_mind_spell (ch, spell, target, target_type);
      break;
    case FORM_SPIRIT:
      transformation_spirit_spell (ch, spell, target, target_type);
      break;
    case FORM_AIR:
      transformation_air_spell (ch, spell, target, target_type);
      break;
    case FORM_EARTH:
      transformation_earth_spell (ch, spell, target, target_type);
      break;
    case FORM_FIRE:
      transformation_fire_spell (ch, spell, target, target_type);
      break;
    case FORM_WATER:
      transformation_water_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char
	("There has been an error. The spell's form was not recognized.\n",
	 ch);
    }
}

void
perception_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		  int target_type)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      perception_animal_spell (ch, spell, target, target_type);
      break;
    case FORM_PLANT:
      perception_plant_spell (ch, spell, target, target_type);
      break;
    case FORM_IMAGE:
      perception_image_spell (ch, spell, target, target_type);
      break;
    case FORM_LIGHT:
      perception_light_spell (ch, spell, target, target_type);
      break;
    case FORM_DARKNESS:
      perception_darkness_spell (ch, spell, target, target_type);
      break;
    case FORM_POWER:
      perception_power_spell (ch, spell, target, target_type);
      break;
    case FORM_MIND:
      perception_mind_spell (ch, spell, target, target_type);
      break;
    case FORM_SPIRIT:
      perception_spirit_spell (ch, spell, target, target_type);
      break;
    case FORM_AIR:
      perception_air_spell (ch, spell, target, target_type);
      break;
    case FORM_EARTH:
      perception_earth_spell (ch, spell, target, target_type);
      break;
    case FORM_FIRE:
      perception_fire_spell (ch, spell, target, target_type);
      break;
    case FORM_WATER:
      perception_water_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char
	("There has been an error. The spell's form was not recognized.\n",
	 ch);
    }
}

void
control_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
	       int target_type)
{
  switch (spell->a.spell.form)
    {
    case FORM_ANIMAL:
      control_animal_spell (ch, spell, target, target_type);
      break;
    case FORM_PLANT:
      control_plant_spell (ch, spell, target, target_type);
      break;
    case FORM_IMAGE:
      control_image_spell (ch, spell, target, target_type);
      break;
    case FORM_LIGHT:
      control_light_spell (ch, spell, target, target_type);
      break;
    case FORM_DARKNESS:
      control_darkness_spell (ch, spell, target, target_type);
      break;
    case FORM_POWER:
      control_power_spell (ch, spell, target, target_type);
      break;
    case FORM_MIND:
      control_mind_spell (ch, spell, target, target_type);
      break;
    case FORM_SPIRIT:
      control_spirit_spell (ch, spell, target, target_type);
      break;
    case FORM_AIR:
      control_air_spell (ch, spell, target, target_type);
      break;
    case FORM_EARTH:
      control_earth_spell (ch, spell, target, target_type);
      break;
    case FORM_FIRE:
      control_fire_spell (ch, spell, target, target_type);
      break;
    case FORM_WATER:
      control_water_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char
	("There has been an error. The spell's form was not recognized.\n",
	 ch);
    }
}

void
parse_spell_technique (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
		       int target_type)
{
  switch (spell->a.spell.technique)
    {
    case TECHNIQUE_CREATION:
      creation_spell (ch, spell, target, target_type);
      break;
    case TECHNIQUE_DESTRUCTION:
      destruction_spell (ch, spell, target, target_type);
      break;
    case TECHNIQUE_TRANSFORMATION:
      transformation_spell (ch, spell, target, target_type);
      break;
    case TECHNIQUE_PERCEPTION:
      perception_spell (ch, spell, target, target_type);
      break;
    case TECHNIQUE_CONTROL:
      control_spell (ch, spell, target, target_type);
      break;
    default:
      send_to_char ("There has been an error: spell technique not found.\n",
		    ch);
    }
}

int
get_form_propensities (int discipline, int form)
{
/*
  switch (discipline)
    {
    case SKILL_VOODOO:
      switch (form)
	{
	case FORM_ANIMAL:
	  return 5;
	case FORM_PLANT:
	  return 10;
	case FORM_IMAGE:
	  return 5;
	case FORM_LIGHT:
	  return 20;
	case FORM_DARKNESS:
	  return -20;
	case FORM_POWER:
	  return -10;
	case FORM_MIND:
	  return 10;
	case FORM_SPIRIT:
	  return -15;
	case FORM_AIR:
	  return 0;
	case FORM_EARTH:
	  return 0;
	case FORM_FIRE:
	  return -5;
	case FORM_WATER:
	  return 5;
	}
      break;
    case SKILL_VOODOO:
      switch (form)
	{
	case FORM_ANIMAL:
	  return -5;
	case FORM_PLANT:
	  return -10;
	case FORM_IMAGE:
	  return -5;
	case FORM_LIGHT:
	  return -20;
	case FORM_DARKNESS:
	  return 20;
	case FORM_POWER:
	  return 10;
	case FORM_MIND:
	  return -5;
	case FORM_SPIRIT:
	  return 20;
	case FORM_AIR:
	  return 0;
	case FORM_EARTH:
	  return 0;
	case FORM_FIRE:
	  return 5;
	case FORM_WATER:
	  return -5;
	}
      break;
    case SKILL_VOODOO:
      switch (form)
	{
	case FORM_ANIMAL:
	  return 0;
	case FORM_PLANT:
	  return 0;
	case FORM_IMAGE:
	  return 0;
	case FORM_LIGHT:
	  return -10;
	case FORM_DARKNESS:
	  return -10;
	case FORM_POWER:
	  return 5;
	case FORM_MIND:
	  return 0;
	case FORM_SPIRIT:
	  return 0;
	case FORM_AIR:
	  return 10;
	case FORM_EARTH:
	  return 10;
	case FORM_FIRE:
	  return 10;
	case FORM_WATER:
	  return 10;
	}
      break;
    }
*/
  return 0;
}

int
get_technique_propensities (int discipline, int technique)
{
/*
  switch (discipline)
    {
    case SKILL_VOODOO:
      switch (technique)
	{
	case TECHNIQUE_CREATION:
	  return 20;
	case TECHNIQUE_DESTRUCTION:
	  return -20;
	case TECHNIQUE_TRANSFORMATION:
	  return 0;
	case TECHNIQUE_PERCEPTION:
	  return 10;
	case TECHNIQUE_CONTROL:
	  return 0;
	}
      break;
    case SKILL_VOODOO:
      switch (technique)
	{
	case TECHNIQUE_CREATION:
	  return -20;
	case TECHNIQUE_DESTRUCTION:
	  return 20;
	case TECHNIQUE_TRANSFORMATION:
	  return 15;
	case TECHNIQUE_PERCEPTION:
	  return -5;
	case TECHNIQUE_CONTROL:
	  return 0;
	}
      break;
    case SKILL_VOODOO:
      switch (technique)
	{
	case TECHNIQUE_CREATION:
	  return -10;
	case TECHNIQUE_DESTRUCTION:
	  return -10;
	case TECHNIQUE_TRANSFORMATION:
	  return 10;
	case TECHNIQUE_PERCEPTION:
	  return 5;
	case TECHNIQUE_CONTROL:
	  return 10;
	}
      break;
    }
*/
  return 0;
}

int
spell_failed (CHAR_DATA * ch, AFFECTED_TYPE * spell)
{
  int technique_mod = 0, form_mod = 0, magnitude_mod = 0, sphere_mod = 0;
  int check = 0, effective_skill = 0, preparation_mod = 0;
  bool failure = false, critical_failure = false;
  char buf[MAX_STRING_LENGTH];
  char failbuf[25];

  // Deal with penalties for casting spells of other spheres

  if (get_discipline (spell) != spell->a.spell.discipline)
    {
      if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  if (get_discipline (spell) == SKILL_VOODOO)
	    sphere_mod -= 30;
	  else if (get_discipline (spell) == SKILL_VOODOO)
	    sphere_mod -= 10;
	}
      if (spell->a.spell.discipline == SKILL_VOODOO)
	{
	  if (get_discipline (spell) == SKILL_VOODOO)
	    sphere_mod -= 30;
	  else if (get_discipline (spell) == SKILL_VOODOO)
	    sphere_mod -= 10;
	}
      else if (spell->a.spell.discipline == SKILL_VOODOO)
	sphere_mod -= 10;
    }

  technique_mod =
    get_technique_propensities (spell->a.spell.discipline,
				spell->a.spell.technique);
  form_mod =
    get_form_propensities (spell->a.spell.discipline, spell->a.spell.form);
  magnitude_mod = spell->a.spell.magnitude * 5;
  preparation_mod = ch->preparing_time / 3;

  effective_skill =
    ch->skills[spell->a.spell.discipline] + (preparation_mod + technique_mod +
					     form_mod + sphere_mod) -
    magnitude_mod;

  effective_skill = MAX (effective_skill, 1);
  effective_skill = MIN (effective_skill, 80);

  check = number (1, SKILL_CEILING);

  if (check > effective_skill)
    {
      if ((check - effective_skill) > (effective_skill * 3) && !number (0, 3))
	critical_failure = true;
      failure = true;
    }

  if (critical_failure)
    sprintf (failbuf, " (critical failure!)");
  else if (failure)
    sprintf (failbuf, " (failure)");
  else
    sprintf (failbuf, " (success)");

  sprintf (buf,
	   "Spell Name: %s %s %s\nTechnique Modifier: %d\nForm Modifier: %d\nSphere Modifier: %d\nMagnitude Modifier: %d\nPreparation Modifier: %d\nEffective Skill: %d\n\n#6Check Rolled:#0 %d vs. %d%s\n\n",
	   techniques[spell->a.spell.technique],
	   magnitudes[spell->a.spell.magnitude], forms[spell->a.spell.form],
	   technique_mod, form_mod, sphere_mod, magnitude_mod * -1,
	   preparation_mod, effective_skill, check, effective_skill, failbuf);

  send_to_char (buf, ch);

  if (failure)
    {
      send_to_char
	("You misspeak a syllable of the incantation, and your spell fizzles harmlessly.\n",
	 ch);
      return 1;
    }

  return 0;
}

int
is_direction (char *argument)
{
  int i = 0;

  for (i = 0; i <= LAST_DIR; i++)
    {
      if (!strn_cmp (dirs[i], argument, strlen (argument)))
	return i;
    }

  return -1;
}

void
process_spell_damage (CHAR_DATA * ch, CHAR_DATA * tch, int id, int saved,
		      int which)
{
  int wound_form = 0, wound_type = 0, x = 0, y = 0, i = 0, location = 0;
  float orig_dmg = 0, brut_dmg = 0, save_dmg = 0, dmg = 0;
  float save_factor = 1.0, loc_factor = 1.0, loc_dmg = 0;
  char buf[MAX_STRING_LENGTH];

  *buf = '\0';

  if (which == 1)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE1));
  else if (which == 2)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE2));
  else if (which == 2)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE2));
  else if (which == 3)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE3));
  else if (which == 4)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE4));
  else if (which == 5)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE5));
  else
    return;

  if (!*buf)
    return;

  sscanf (buf, "%d %d", &x, &y);

  if (!x || !y)
    return;

  // Deal with multipliers for any successful saving throw

  if (saved == SAVE_FOR_HALF_EFFECT)
    save_factor = .50;
  else if (saved == SAVE_FOR_THIRD_EFFECT)
    save_factor = .33;
  else if (saved == SAVE_FOR_QUARTER_EFFECT)
    save_factor = .25;

  // Determine hit location, add in location multiplier

  i = number (1, 100);
  location = -1;

  while (i > 0)
    i = i - body_tab[0][++location].percent;

  loc_factor =
    ((body_tab[0][location].damage_mult * 1.0) /
     (body_tab[0][location].damage_div * 1.0));

  orig_dmg = dice (x, y);
  brut_dmg = orig_dmg * COMBAT_BRUTALITY;
  loc_dmg = brut_dmg * loc_factor;
  save_dmg = loc_dmg * save_factor;

  dmg = save_dmg;

  wound_form = strtol (lookup_spell_variable (id, VAR_DAMAGE_FORM), NULL, 10);

  if (wound_form == MAGIC_DAMAGE_SLASH)
    wound_type = 2;
  else if (wound_form == MAGIC_DAMAGE_PUNCTURE)
    wound_type = 1;
  else if (wound_form == MAGIC_DAMAGE_BLUNT)
    wound_type = 3;
  else if (wound_form == MAGIC_DAMAGE_BURN)
    wound_type = 6;
  else if (wound_form == MAGIC_DAMAGE_FROST)
    wound_type = 5;
  else
    wound_type = 3;

  send_to_char (buf, ch);

  sprintf (buf,
	   "\nOriginal Diceroll: %d\nBrutality Factor: %f\nWith Brutality: %d\nLocation Factor: %f\nWith Location Multiple: %f\nSave Factor: %f\nWith Saving Throw: %d\n",
	   (int) orig_dmg, COMBAT_BRUTALITY, (int) brut_dmg, loc_factor,
	   loc_dmg, save_factor, (int) save_dmg);

  send_to_char (buf, ch);

  wound_to_char (tch, figure_location (tch, location), (int)dmg, wound_type, 0, 0,
		 0);
}

void
process_spell_healing (CHAR_DATA * ch, CHAR_DATA * tch, int id, int saved,
		       int which)
{
}

void
update_target (CHAR_DATA * tch, int affect_type)
{
  if (affect_type == MAGIC_AFFECT_SLEEP)
    {
      do_sleep (tch, "", 0);
      if (tch->fighting)
	{
	  if (tch->fighting == tch)
	    stop_fighting (tch->fighting);
	  stop_fighting (tch);
	}
    }
}

void
process_spell_effect (CHAR_DATA * ch, CHAR_DATA * tch, int id, int saved,
		      int affect_type, int which)
{
  AFFECTED_TYPE *af;
  int x = 0, y = 0;
  char buf[MAX_STRING_LENGTH];

  *buf = '\0';

  if (which == 1)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE1));
  else if (which == 2)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE2));
  else if (which == 2)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE2));
  else if (which == 3)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE3));
  else if (which == 4)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE4));
  else if (which == 5)
    sprintf (buf, "%s", lookup_spell_variable (id, VAR_DICE5));
  else
    return;

  if (!*buf)
    return;

  sscanf (buf, "%d %d", &x, &y);

  if (!x || !y)
    return;

  CREATE (af, AFFECTED_TYPE, 1);

  af->type = affect_type;
  af->a.spell.duration = dice (x, y);
  af->a.spell.modifier = 0;
  af->a.spell.location = 0;
  af->a.spell.bitvector = 0;
  af->a.spell.t = 0;
  af->a.spell.sn = id;

  while (get_affect (tch, affect_type))
    remove_affect_type (tch, affect_type);

  affect_to_char (tch, af);

  update_target (tch, affect_type);
}

void
process_spell_results (CHAR_DATA * ch, CHAR_DATA * tch, int id, int saved)
{
  int save_for = 0, affect_type = 0, i = 1;

  if (saved)
    save_for = strtol (lookup_spell_variable (id, VAR_SAVE_FOR), NULL, 10);

  if (save_for == SAVE_FOR_NO_EFFECT && saved)
    return;

  while (i <= 5)
    {
      if (i == 1)
	affect_type =
	  strtol (lookup_spell_variable (id, VAR_AFFECT1), NULL, 10);
      else if (i == 2)
	affect_type =
	  strtol (lookup_spell_variable (id, VAR_AFFECT2), NULL, 10);
      else if (i == 3)
	affect_type =
	  strtol (lookup_spell_variable (id, VAR_AFFECT3), NULL, 10);
      else if (i == 4)
	affect_type =
	  strtol (lookup_spell_variable (id, VAR_AFFECT4), NULL, 10);
      else if (i == 5)
	affect_type =
	  strtol (lookup_spell_variable (id, VAR_AFFECT5), NULL, 10);
      else
	break;
      if (affect_type == MAGIC_AFFECT_DMG)
	process_spell_damage (ch, tch, id, save_for, i);
      else if (affect_type == MAGIC_AFFECT_HEAL)
	process_spell_healing (ch, tch, id, save_for, i);
      else
	process_spell_effect (ch, tch, id, save_for, affect_type, i);
      i++;
    }
}

void
process_spell_save (CHAR_DATA * ch, void *target, int id, int target_type)
{
  CHAR_DATA *tch = NULL;
  ROOM_DATA *troom = NULL;
  OBJ_DATA *tobj = NULL;
  int save_type = 0, save_for = 0, save = 0, penalty = 0;

  if (target_type == TARG_SELF || target_type == TARG_OTHER
      || target_type == TARG_OTHER_HOSTILE || target_type == TARG_GROUP
      || target_type == TARG_GROUP_HOSTILE || target_type == TARG_SELF_OTHER
      || target_type == TARG_SELF_OTHER_HOSTILE)
    tch = (CHAR_DATA *) target;
  else if (target_type == TARG_ROOM || target_type == TARG_ROOM_HOSTILE)
    troom = (ROOM_DATA *) target;
  else if (target_type == TARG_OBJ)
    tobj = (OBJ_DATA *) target;

  save_type = strtol (lookup_spell_variable (id, VAR_SAVE_TYPE), NULL, 10);
  save_for = strtol (lookup_spell_variable (id, VAR_SAVE_FOR), NULL, 10);

  if (tch != NULL && save_type != SAVE_NONE)
    {
      switch (save_type)
	{
	case SAVE_EVADE:
	  penalty = caster_magic_skill (ch) / 3;
	  if (ch->room == tch->room)
	    send_to_room ("\n", tch->in_room);
	  else
	    {
	      send_to_room ("\n", ch->in_room);
	      send_to_room ("\n", tch->in_room);
	    }
	  if (tch->skills[SKILL_DODGE] > tch->skills[SKILL_DEFLECT]
	      || !get_equip (tch, WEAR_SHIELD))
	    {
	      if ((save = skill_use (tch, SKILL_DODGE, penalty)) > 0)
		{
		  if (save_for == SAVE_FOR_NO_EFFECT)
		    {
		      act ("You quickly leap aside, unharmed by the blast.",
			   true, tch, 0, 0, TO_CHAR);
		      act ("$n quickly leaps aside, unharmed by the blast.",
			   true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
		    }
		  else
		    {
		      act
			("You attempt to evade, but are only partially successful.",
			 true, tch, 0, 0, TO_CHAR);
		      act
			("$n attempts to evade, but is only partially successful.",
			 true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
		    }
		}
	      else
		{
		  act
		    ("You attempt to evade, but fail, and are caught full-force by the blast!",
		     true, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		  act
		    ("$n attempts to evade, but fails, and is caught full-force by the blast.",
		     true, tch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
	      process_spell_results (ch, tch, id, save);
	      return;
	    }
	  else
	    {
	      if ((save = skill_use (tch, SKILL_DEFLECT, penalty)) > 0)
		{
		  if (save_for == SAVE_FOR_NO_EFFECT)
		    {
		      act ("You succesfully deflect the blast using $o.",
			   true, tch, get_equip (tch, WEAR_SHIELD), 0,
			   TO_CHAR | _ACT_FORMAT);
		      act ("$n successfully defelcts the blast using $o.",
			   true, tch, get_equip (tch, WEAR_SHIELD), 0,
			   TO_ROOM | _ACT_FORMAT);
		    }
		  else
		    {
		      act
			("You attempt to deflect the blast with $o, but are only partially successful.",
			 true, tch, get_equip (tch, WEAR_SHIELD), 0,
			 TO_CHAR | _ACT_FORMAT);
		      act
			("$n attempts to deflect the blast using $o, but is only partially successful.",
			 true, tch, get_equip (tch, WEAR_SHIELD), 0,
			 TO_ROOM | _ACT_FORMAT);
		    }
		}
	      else
		{
		  act ("You attempt to deflect the blast with $o, but fail.",
		       true, tch, get_equip (tch, WEAR_SHIELD), 0,
		       TO_CHAR | _ACT_FORMAT);
		  act ("$n attempts to deflect the blast with $o, but fails.",
		       true, tch, get_equip (tch, WEAR_SHIELD), 0,
		       TO_ROOM | _ACT_FORMAT);
		}
	      process_spell_results (ch, tch, id, save);
	    }
	  break;
	case SAVE_WIL:
	  if (ch->in_room == tch->in_room)
	    send_to_room ("\n", ch->in_room);
	  penalty = caster_magic_skill (ch) / 15;
	  if ((save = (tch->wil - penalty) <= number (1, 25)))
	    {
	      act
		("You manage to fight off the effect encroaching upon your mind.",
		 true, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      act ("$n appears otherwise unaffected.", true, tch, 0, 0,
		   TO_ROOM | _ACT_FORMAT);
	    }
	  else
	    process_spell_results (ch, tch, id, save);
	  break;
	}
    }
  else
    {
      process_spell_results (ch, tch, id, 0);
    }
}

void
process_spell_casting (CHAR_DATA * ch, void *target, int id, int target_type)
{
  CHAR_DATA *tch = NULL;
  ROOM_DATA *troom = NULL;
  OBJ_DATA *tobj = NULL;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];

  if (target_type == TARG_SELF || target_type == TARG_OTHER
      || target_type == TARG_OTHER_HOSTILE || target_type == TARG_GROUP
      || target_type == TARG_GROUP_HOSTILE || target_type == TARG_SELF_OTHER
      || target_type == TARG_SELF_OTHER_HOSTILE)
    tch = (CHAR_DATA *) target;
  else if (target_type == TARG_ROOM || target_type == TARG_ROOM_HOSTILE)
    troom = (ROOM_DATA *) target;
  else if (target_type == TARG_OBJ)
    tobj = (OBJ_DATA *) target;

  sprintf (buf, "%s", lookup_spell_variable (id, VAR_CH_ECHO));
  sprintf (buf2, "%s", lookup_spell_variable (id, VAR_VICT_ECHO));
  sprintf (buf3, "%s", lookup_spell_variable (id, VAR_ROOM_ECHO));
  sprintf (buf4, "%s", lookup_spell_variable (id, VAR_SELF_ECHO));

  if (tch != NULL && tch != ch)
    {
      act (buf, true, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
      act (buf2, true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
      act (buf3, true, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
      process_spell_save (ch, target, id, target_type);
      return;
    }
  else if (tch != NULL && tch == ch)
    {
      act (buf4, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      act (buf3, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
      process_spell_save (ch, target, id, target_type);
      return;
    }
}

void
do_cast (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch;
  ROOM_DIRECTION_DATA *exit;
  ROOM_DATA *room;
  char buf[MAX_STRING_LENGTH];
  void *target = NULL;
  int target_type = 0, dir = 0, spell_target = 0;

  *buf = '\0';

  if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC))
    {
      send_to_char ("You cannot cast in OOC areas.\n", ch);
      return;
    }

  if (!ch->preparing_id)
    {
      send_to_char ("You are not currently preparing a spell.\n", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!*buf)
    {
      target = ch;
      target_type = TARGET_SELF;
    }

  if (!str_cmp (buf, "room"))
    {
      target = ch->room;
      target_type = TARGET_ROOM;
    }

  if (*buf && (dir = is_direction (buf)) != -1)
    {

      if (!(exit = EXIT (ch, dir)))
	{
	  send_to_char ("There is no exit in that direction.\n", ch);
	  return;
	}

      if (!*argument)
	{
	  target = exit;
	  if (IS_SET (exit->exit_info, EX_CLOSED))
	    target_type = TARGET_EXIT;
	  else
	    {
	      target = vnum_to_room (exit->to_room);
	      target_type = TARGET_ROOM;
	    }
	}
      else
	{			// Check for char up to three rooms away if seen via scan
		if (exit
      	&& IS_SET (exit->exit_info, EX_CLOSED)
      	&& !IS_SET (exit->exit_info, EX_ISGATE))	    {
	      send_to_char ("Your view is blocked.\n", ch);
	      return;
	    }
	  room = vnum_to_room (exit->to_room);
	  if (!room)
	    {
	      send_to_char ("There is no exit in that direction.\n", ch);
	      return;
	    }
	  if (!(target = get_char_room_vis2 (ch, room->vnum, argument))
	      || !has_been_sighted (ch, (CHAR_DATA *) target))
	    {
	      if ((exit = room->dir_option[dir]))
		{
		if (exit
      	&& IS_SET (exit->exit_info, EX_CLOSED)
      	&& !IS_SET (exit->exit_info, EX_ISGATE))		    {
		      send_to_char
			("Your view is blocked further on in that direction.\n",
			 ch);
		      return;
		    }
		  room = vnum_to_room (exit->to_room);
		  if (!room)
		    {
		      send_to_char ("There is no exit in that direction.\n",
				    ch);
		      return;
		    }
		  if (!
		      (target =
		       get_char_room_vis2 (ch, room->vnum, argument))
		      || !has_been_sighted (ch, (CHAR_DATA *) target))
		    {
		      if ((exit = room->dir_option[dir]))
			{
			if (exit
      	&& IS_SET (exit->exit_info, EX_CLOSED)
      	&& !IS_SET (exit->exit_info, EX_ISGATE))			    {
			      send_to_char
				("Your view is blocked further on in that direction.\n",
				 ch);
			      return;
			    }
			  room = vnum_to_room (exit->to_room);
			  if (!room)
			    {
			      send_to_char
				("There is no exit in that direction.\n", ch);
			      return;
			    }
			  if (!
			      (target =
			       get_char_room_vis2 (ch, room->vnum,
						   argument))
			      || !has_been_sighted (ch, (CHAR_DATA *) target))
			    {
			    }
			}
		    }
		}
	    }
	}
    }

  if (!target && !(target = get_char_room_vis (ch, buf)))
    {
      target = get_obj_in_dark (ch, buf, ch->right_hand);
      if (!target)
	target = get_obj_in_dark (ch, buf, ch->left_hand);
      if (!target)
	target = get_obj_in_dark (ch, buf, ch->equip);
      if (!target)
	target = get_obj_in_list_vis (ch, buf, ch->room->contents);
      if (target)
	target_type = TARGET_OBJ;
    }

  if (target && (CHAR_DATA *) target != ch)
    {
      if (*argument)
	{
	  tch = (CHAR_DATA *) target;
	  target = get_obj_in_list_vis (ch, argument, tch->right_hand);
	  if (!target)
	    target = get_obj_in_list_vis (ch, argument, tch->left_hand);
	  if (!target)
	    target = get_obj_in_list_vis (ch, argument, tch->equip);
	  if (!target)
	    {
	      target = tch;
	      target_type = TARGET_OTHER;
	    }
	  else
	    target_type = TARGET_OTHER_INVENTORY;
	}
      else
	target_type = TARGET_OTHER;
    }

  if (!target)
    {
      send_to_char ("I don't see that here.\n", ch);
      return;
    }

  if (!lookup_spell_variable (ch->preparing_id, VAR_NAME))
    {
      send_to_char ("I can't find that spell definition in the database.\n",
		    ch);
      ch->preparing_id = 0;
      ch->preparing_time = 0;
      return;
    }

  spell_target =
    strtol (lookup_spell_variable (ch->preparing_id, VAR_TARGET_TYPE), NULL,
	    10);

  if (spell_target == TARG_SELF && target_type != TARGET_SELF)
    {
      send_to_char ("This spell may only be cast upon yourself.\n", ch);
      return;
    }
  else if (spell_target == TARG_OBJ && target_type != TARGET_OBJ
	   && target_type != TARGET_OTHER_INVENTORY)
    {
      send_to_char ("This spell may only be cast upon an object.\n", ch);
      return;
    }
  else
    if ((spell_target == TARG_OTHER || spell_target == TARG_OTHER_HOSTILE
	 || spell_target == TARG_GROUP || spell_target == TARG_GROUP_HOSTILE)
	&& target_type != TARGET_OTHER && target_type != TARGET_REMOTE_OTHER)
    {
      send_to_char ("This spell may only be cast at another individual.\n",
		    ch);
      return;
    }
  else if ((spell_target == TARG_SELF_OTHER || spell_target == TARG_OTHER)
	   && target_type != TARGET_SELF && target_type != TARGET_OTHER
	   && target_type != TARGET_REMOTE_OTHER)
    {
      send_to_char
	("This spell may only be cast upon yourself or someone else.\n", ch);
      return;
    }
  else if ((spell_target == TARG_ROOM || spell_target == TARG_ROOM_HOSTILE)
	   && target_type != TARGET_ROOM && target_type != TARGET_REMOTE_ROOM)
    {
      send_to_char ("This spell may only be cast at an entire room.\n", ch);
      return;
    }

  process_spell_casting (ch, target, ch->preparing_id, spell_target);

  ch->preparing_id = 0;
  ch->preparing_time = 0;
}

void
do_invoke (CHAR_DATA * ch, char *argument, int cmd)
{

/*
	if ( !real_skill (ch, SKILL_WIZARDRY) && !real_skill (ch, SKILL_SORCERY) &&
	     !real_skill (ch, SKILL_MYSTICISM) && !real_skill (ch, SKILL_RUNECASTING) ) {
		send_to_char ("You are not trained in this art.\n", ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("Invoke which magick?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	for ( spell = ch->known_spells; spell; spell = spell->next ) {
		if ( !strn_cmp (buf, spell->name, strlen(buf)) ) {
			known = true;
			break;
		}
	}

	if ( !known ) {
		send_to_char ("You have no knowledge of that magick.\n", ch);
		return;
	}

	spell_entry = spell_list;

	while ( spell_entry->name ) {
		if ( !str_cmp (spell->name, spell_entry->name) ) {
			found = true;
			break;
		}
		spell_entry++;
	}

	if ( !found ) {
		sprintf (buf, "Missing spell in list: %s", spell->name);
		send_to_char ("There has been an error. Please report this via BUG.\n", ch);
		return;
	}

	CREATE (ch->preparing, SPELL_TABLE_DATA, 1);

	ch->preparing->name = spell_entry->name;
	ch->preparing->discipline = spell_entry->discipline;
	ch->preparing->spell_fun = spell_entry->spell_fun;
	ch->preparing->cost = spell_entry->cost;
	ch->preparing->energy_source = spell_entry->energy_source;
	ch->preparing->target = spell_entry->target;

	if ( ch->preparing->energy_source == SOURCE_ELEMENTAL )
		grid_mod = ch->room->elemental / 100.0;
	else if ( ch->preparing->energy_source == SOURCE_PSYCHIC )
		grid_mod = ch->room->psychic / 100.0;
	else if ( ch->preparing->energy_source == SOURCE_HOLY )
		grid_mod = ch->room->holy / 100.0;
	else if ( ch->preparing->energy_source == SOURCE_SHADOW )
		grid_mod = ch->room->shadow / 100.0;

	ch->preparing->cost *= grid_mod;
	ch->preparing->cost += 0.5;
	ch->preparing->cost = MAX (ch->preparing->cost, 1);

	harn_mod = 100.0 / ch->skills [SKILL_HARNESS];
	ch->preparing->cost *= harn_mod;
	ch->preparing->cost += 0.5;
	ch->preparing->cost = MAX (ch->preparing->cost, 1);
	ch->delay_info1 = spell->rating;

	if ( (ch->preparing->energy_source == SOURCE_ELEMENTAL && ch->room->elemental == 1001) ||
	     (ch->preparing->energy_source == SOURCE_PSYCHIC && ch->room->psychic == 1001) ||
	     (ch->preparing->energy_source == SOURCE_HOLY && ch->room->holy == 1001) ||
	     (ch->preparing->energy_source == SOURCE_SHADOW && ch->room->shadow == 1001) ||
	     ch->harness < ch->preparing->cost ) {
		send_to_char ("You strain, but cannot gather the necessary energies to invoke this magick.\n", ch);
		ch->preparing = NULL;
		ch->delay_info1 = 0;
		return;
	}

	if ( ch->preparing->discipline == SKILL_WIZARDRY ) {
		sprintf (buf, "You begin invoking %s, carefully chanting ancient words of power.", ch->preparing->name);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n lifts $s voice in a chant, carefully-wrought syllables of power resounding in the air.");
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
*/
}

void
remove_enchantment (CHAR_DATA * ch, ENCHANTMENT_DATA * ench)
{
  ENCHANTMENT_DATA *tmp;

  if (!str_cmp (ench->name, "Tindome Miriel"))
    {
      act
	("The glimmering sphere hovering above your right shoulder flickers silently from existence.",
	 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      act
	("The glimmering sphere hovering above $n's right shoulder flickers silently from existence.",
	 false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
    }

  else if (!str_cmp (ench->name, "Nole Isse"))
    {
      act
	("You feel your levels of perception return to normal, as heat's hidden lore fades from your grasp once more.",
	 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      act ("The dim flicker of fae radiance in $N's eyes fades away.", false,
	   ch, 0, 0, TO_ROOM | _ACT_FORMAT);
    }

  if (ch->enchantments && !str_cmp (ch->enchantments->name, ench->name))
    ch->enchantments = NULL;
  else
    for (tmp = ch->enchantments; tmp; tmp = tmp->next)
      {
	if (!str_cmp (tmp->next->name, ench->name))
	  tmp->next = tmp->next->next;
      }
}

void
apply_enchantment (CHAR_DATA * caster, CHAR_DATA * victim, char *name,
		   int duration, int source)
{
  ENCHANTMENT_DATA *tmp, *enchantment;

  CREATE (enchantment, ENCHANTMENT_DATA, 1);

  enchantment->name = add_hash (name);
  enchantment->original_hours = duration;
  enchantment->current_hours = enchantment->original_hours;
  enchantment->power_source = source;
  enchantment->next = NULL;

  if (!victim->enchantments)
    {
      CREATE (victim->enchantments, ENCHANTMENT_DATA, 1);
      victim->enchantments = enchantment;
    }
  else
    for (tmp = victim->enchantments; tmp; tmp = tmp->next)
      {
	if (!str_cmp (tmp->name, enchantment->name))
	  {			// Recharge.
	    tmp->current_hours = enchantment->current_hours;
	    break;
	  }
	if (!tmp->next)
	  {
	    CREATE (tmp->next, ENCHANTMENT_DATA, 1);
	    tmp->next = enchantment;
	    break;
	  }
      }
}

int
is_enchanted (CHAR_DATA * victim, char *enchantment_name)
{
  ENCHANTMENT_DATA *tmp;

  for (tmp = victim->enchantments; tmp; tmp = tmp->next)
    if (!str_cmp (tmp->name, enchantment_name))
      return 1;

  return 0;
}

int
armor_absorption (CHAR_DATA * target, char *location)
{
  OBJ_DATA *obj;

  if (!str_cmp (location, "skull") || !str_cmp (location, "reye") ||
      !str_cmp (location, "leye") || !str_cmp (location, "face"))
    {
      if ((obj = get_equip (target, WEAR_HEAD))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  else if (!str_cmp (location, "rshoulder")
	   || !str_cmp (location, "lshoulder")
	   || !str_cmp (location, "rupperarm")
	   || !str_cmp (location, "lupperarm")
	   || !str_cmp (location, "rforearm")
	   || !str_cmp (location, "lforearm"))
    {
      if ((obj = get_equip (target, WEAR_ARMS))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  else if (!str_cmp (location, "rhand") || !str_cmp (location, "lhand"))
    {
      if ((obj = get_equip (target, WEAR_HANDS))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  else if (!str_cmp (location, "thorax") || !str_cmp (location, "abdomen"))
    {
      if ((obj = get_equip (target, WEAR_BODY))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  else if (!str_cmp (location, "hip") || !str_cmp (location, "groin") ||
	   !str_cmp (location, "rthigh") || !str_cmp (location, "lthigh") ||
	   !str_cmp (location, "rknee") || !str_cmp (location, "lknee") ||
	   !str_cmp (location, "rcalf") || !str_cmp (location, "lcalf"))
    {
      if ((obj = get_equip (target, WEAR_LEGS))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  else if (!str_cmp (location, "rfoot") || !str_cmp (location, "lfoot"))
    {
      if ((obj = get_equip (target, WEAR_FEET))
	  && GET_ITEM_TYPE (obj) == ITEM_ARMOR)
	return obj->o.armor.armor_value;
    }

  return 0;
}

char *
rating_adj (KNOWN_SPELL_DATA * spell)
{
  if (spell->rating >= 70)
    return "Mastery";
  else if (spell->rating >= 50)
    return "Adroit";
  else if (spell->rating >= 30)
    return "Familiarity";
  else if (spell->rating >= 1)
    return "Novicehood";
  else
    return "Proficient";
}

void
do_spells (CHAR_DATA * ch, char *argument, int cmd)
{
  send_to_char ("This command has been temporarily disabled.\n", ch);
  return;

}

void
email_acceptance (DESCRIPTOR_DATA * d)
{
  account *acct = NULL;
  CHAR_DATA *tch = NULL;
  time_t current_time;
  char *date;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char email[MAX_STRING_LENGTH];

  current_time = time (0);
  date = (char *) asctime (localtime (&current_time));

  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  tch = NULL;
  tch = load_pc (d->character->delay_who2);
  if (!tch)
    return;

  acct = new account (tch->pc->account_name);
  if (!acct->is_registered ())
    {
      delete acct;
      return;
    }

  if (!*d->character->pc->msg)
    {
      send_to_char ("No email sent!\n", d->character);
      mem_free (date);
      delete acct;
      if (tch)
	unload_pc (tch);
      return;
    }
  else if (!tch || !acct->is_registered ())
    {
      send_to_char
	("\nEmail response aborted; there was a problem loading this PC.\n",
	 d->character);
      mem_free (date);
      delete acct;
      if (tch)
	unload_pc (tch);
      return;
    }
  else
    {
      if (d->character->pc && IS_MORTAL (d->character)
	  && d->character->pc->is_guide)
	{
	  sprintf (buf, "Greetings,\n"
		   "\n"
		   "   Thank you for your interest in %s! This is an automated\n"
		   "system notification sent to inform you that your application for a character\n"
		   "named %s has been ACCEPTED by a Guide, and that you may enter\n"
		   "Parallel at your earliest convenience. We'll see you there!\n"
		   "\n"
		   "%s left the following comments regarding your application:\n"
		   "\n%s", MUD_NAME, tch->tname, d->character->pc->account_name,
		   d->character->pc->msg);
	  // acct = new account (tch->pc->account_name); // redundant?
	  sprintf (email, "%s Player Guide <%s>", MUD_NAME,
		   d->acct->email.c_str ());
	  send_email (acct, NULL, email, "Your Character Application",
		      buf);
	}
      else
	{
	  sprintf (buf, "Greetings,\n"
		   "\n"
		   "   Thank you for your interest in %s! This is an automated\n"
		   "system notification sent to inform you that your application for a character\n"
		   "named %s has been ACCEPTED by the reviewer, and that you may enter\n"
		   "Parallel at your earliest convenience. We'll see you there!\n"
		   "\n"
		   "%s left the following comments regarding your application:\n"
		   "\n%s", MUD_NAME, tch->tname, d->character->tname,
		   d->character->pc->msg);
	  sprintf (email, "%s <%s>", MUD_NAME, APP_EMAIL);
	  send_email (acct, NULL, email, "Your Character Application",
		      buf);
	}

      d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
      d->pending_message->nVirtual = 0;
      d->pending_message->info = add_hash ("");
      sprintf (buf, "#2Accepted:#0 %s: %s", tch->pc->account_name, tch->tname);
      d->pending_message->subject = add_hash (buf);
      d->pending_message->message = add_hash (d->character->pc->msg);

      add_message (1, "Applications",
		   -5,
		   d->acct->name.c_str (),
		   date,
		   d->pending_message->subject,
		   d->pending_message->info,
		   d->pending_message->message, d->pending_message->flags);

      add_message (1, tch->tname,
		   -2,
		   d->acct->name.c_str (),
		   date,
		   "Application Acceptance",
		   d->pending_message->info,
		   d->pending_message->message, d->pending_message->flags);

      unload_message (d->pending_message);
      if (d->character->pc->msg)
	{
	  mem_free (d->character->pc->msg);
	  d->character->pc->msg = NULL;
	}
      d->pending_message = NULL;

      mysql_safe_query
	("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");
      mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d",
			tch->pc->profession);

      if (tch->pc->special_role)
	{
	  sprintf (email, "%s Player <%s>", MUD_NAME, acct->email.c_str ());
	  delete acct;
	  acct = new account (tch->pc->special_role->poster);
	  if (acct)
	    {
	      sprintf (buf, "Hello,\n\n"
		       "This email is being sent to inform you that a special role you placed\n"
		       "in chargen has been applied for and accepted. The details are attached.\n"
		       "\n"
		       "Please contact this player at your earliest convenience to arrange a time\n"
		       "to set up and integrate their new character. To do so, simply click REPLY;\n"
		       "their email has been listed in this message as the FROM address.\n\n"
		       "Character Name: %s\n"
		       "Role Description: %s\n"
		       "Role Post Date: %s\n"
		       "\n"
		       "%s\n\n", tch->tname, tch->pc->special_role->summary,
		       tch->pc->special_role->date,
		       tch->pc->special_role->body);
	      sprintf (buf2, "New PC: %s", tch->pc->special_role->summary);
	      send_email (acct, APP_EMAIL, email, buf2, buf);
	    }
	}
    }

  delete acct;
  d->character->delay_who = add_hash (tch->tname);
  if (d->character->delay_who2)
    {
      mem_free (d->character->delay_who2);
      d->character->delay_who2 = NULL;
    }
  d->character->delay_ch = NULL;

  mem_free (date);

  unload_pc (tch);
}

void
email_rejection (DESCRIPTOR_DATA * d)
{
  account *acct = NULL;
  CHAR_DATA *tch = NULL;
  time_t current_time;
  char *date;
  char buf[MAX_STRING_LENGTH];
  char email[MAX_STRING_LENGTH];

  current_time = time (0);
  date = (char *) asctime (localtime (&current_time));
  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  tch = NULL;
  tch = load_pc (d->character->delay_who2);
  if (!tch)
    return;

  acct = new account (tch->pc->account_name);
  if (!acct->is_registered ())
    {
      delete acct;
      return;
    }

  if (!*d->character->pc->msg)
    {
      delete acct;
      send_to_char ("No email sent!\n", d->character);
      mem_free (date);
      if (tch)
	unload_pc (tch);
      return;
    }
  else if (!tch || !acct)
    {
      delete acct;
      send_to_char
	("\nEmail response aborted; there was a problem loading this PC.\n",
	 d->character);
      mem_free (date);
      if (tch)
	unload_pc (tch);
      return;
    }
  else
    {
      if (d->character->pc && IS_MORTAL (d->character)
	  && d->character->pc->is_guide)
	{
	  sprintf (buf, "Greetings,\n"
		   "\n"
		   "   Thank you for your interest in %s! This is an\n"
		   "automated system notification to inform you that your application for\n"
		   "a character named %s was deemed inappropriate by a Guide, and\n"
		   "therefore was declined. However, don't despair! This is a relatively\n"
		   "common occurrence, and nothing to worry about. Your application has\n"
		   "been saved on our server, and you may make the necessary changes simply\n"
		   "by entering the game  as that character. You will be dropped back\n"
		   "into the character generation engine, where you may make corrections.\n"
		   "\n"
		   "   If you have any questions regarding the comments below, clicking\n"
		   "REPLY to this email will allow you to get in touch with the Guide\n"
		   "who reviewed your application. Please be civil if you do choose to\n"
		   "contact them - remember, they volunteer their free time to help you!\n\n"
		   "%s left the following comments regarding your application:\n"
		   "\n%s", MUD_NAME, tch->tname, d->character->pc->account_name,
		   d->character->pc->msg);
	  sprintf (email, "%s Player Guide <%s>", MUD_NAME,
		   d->acct->email.c_str ());
	  send_email (acct, NULL, email, "Your Character Application",
		      buf);
	}
      else
	{
	  sprintf (buf, "Greetings,\n"
		   "\n"
		   "   Thank you for your interest in %s! This is an\n"
		   "automated system notification to inform you that your application for\n"
		   "a character named %s was deemed inappropriate by an administrator, and\n"
		   "therefore was declined. However, don't despair! This is a relatively\n"
		   "common occurrence, and nothing to worry about. Your application has\n"
		   "been saved on our server, and you may make the necessary changes simply\n"
		   "by 'Entering Middle-earth' as that character. You will be dropped back\n"
		   "into the character generation engine, where you may make corrections.\n"
		   "\n"
		   "%s left the following comments regarding your application:\n"
		   "\n%s", MUD_NAME, tch->tname, d->character->tname,
		   d->character->pc->msg);
	  sprintf (email, "%s <%s>", MUD_NAME, APP_EMAIL);
	  send_email (acct, NULL, email, "Your Character Application",
		      buf);
	}

      d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
      d->pending_message->poster = add_hash ("player_applications");
      d->pending_message->nVirtual = 0;
      d->pending_message->info = add_hash ("");
      sprintf (buf, "#1Declined:#0 %s: %s", tch->pc->account_name, tch->tname);
      d->pending_message->subject = add_hash (buf);
      d->pending_message->message = add_hash (d->character->pc->msg);

      add_message (1, "Applications",
		   -5,
		   d->acct->name.c_str (),
		   date,
		   d->pending_message->subject,
		   d->pending_message->info,
		   d->pending_message->message, d->pending_message->flags);

      unload_message (d->pending_message);

      sprintf (buf,
	       "\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
	       "\n%s", d->acct->name.c_str (), d->character->pc->msg);
      if (buf[strlen (buf) - 1] != '\n')
	strcat (buf, "\n");

      tch->pc->msg = add_hash (buf);

      mysql_safe_query
	("UPDATE newsletter_stats SET declined_apps=declined_apps+1");
    }

  delete acct;

  d->character->delay_who = add_hash (tch->tname);
  if (d->character->delay_who2)
    {
      mem_free (d->character->delay_who2);
      d->character->delay_who2 = NULL;
    }
  d->character->delay_ch = NULL;

  mem_free (date);

  save_char (tch, false);
  unload_pc (tch);
}

void
check_psionic_talents (CHAR_DATA * ch)
{
  int chance = 0, roll = 0;
  int cur_talents = 0, i = 0, j = 1;
  int talents[8] = {
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO,
    SKILL_VOODOO
  };
  bool check = true, again = true, awarded = false, block = false;
  char buf[MAX_STRING_LENGTH];
  char *date;
  time_t time_now;

  if (is_newbie (ch) || ch->aur <= 15)
    {
      return;
    }

  if (ch->pc && ch->pc->account_name)
    {
      if (!account::is_psionic_capable (ch->pc->account_name))
	{
	  block = true;
	}

      // Psi tends to make ImmPCs foreground rather than background
      if (GET_FLAG (ch, FLAG_ISADMIN))
	{
	  block = true;
	}
    }
  else
    {
      return;
    }


  if (block)
    {
      return;
    }

  if (ch->aur < 16)
    return;
  else if (ch->aur == 16)
    chance = 5;
  else if (ch->aur == 17)
    chance = 10;
  else if (ch->aur == 18)
    chance = 20;
  else if (ch->aur == 19)
    chance = 30;
  else if (ch->aur == 20)
    chance = 45;
  else if (ch->aur == 21)
    chance = 50;
  else if (ch->aur == 22)
    chance = 60;
  else if (ch->aur == 23)
    chance = 70;
  else if (ch->aur == 24)
    chance = 80;
  else
    chance = 95;

  chance += number (1, 10);
  chance = MIN (chance, 95);

  for (i = 0; i <= 7; i++)
    if (ch->skills[talents[i]])
      cur_talents++;

  while (check && cur_talents <= 4)
    {
      if (number (1, 100) <= chance)
	{
	  again = true;
	  while (again)
	    {
	      roll = talents[number (0, 7)];
	      if (!ch->skills[roll])
		{
		  ch->skills[roll] = 1;
		  cur_talents++;
		  again = false;
		  awarded = true;
		}
	      chance /= 2;
	    }
	  if (cur_talents >= 4)
	    check = false;
	}
      else
	check = false;
    }

  if (!awarded)
    return;

  sprintf (buf,
	   "This character rolled positive for the following talents:\n\n");

  for (i = 0; i <= 7; i++)
    if (ch->skills[talents[i]])
      sprintf (buf + strlen (buf), "   %d. %s\n", j++, skills[talents[i]]);

  time_now = time (0);
  date = (char *) asctime (localtime (&time_now));
  date[strlen (date) - 1] = '\0';

  add_message (1, "Psi_talents", -5, "Server", date, ch->tname, "", buf, 0);
  add_message (1, ch->tname, -2, "Server", date, "Psionic Talents.", "", buf,
	       0);

  mem_free (date);
}

void
starting_skill_boost (CHAR_DATA * ch, int skill, int num)
{
  ch->skills[skill] = (180 / num);
}


/*

Largely purposely addition replaced to the less skills you picked, the
higher those skills are going to be.

void
starting_skill_boost (CHAR_DATA * ch, int skill)
{
  ch->skills[skill] += MIN (30, age (ch).year) / 2 + number (5, 15);
}
*/

void
setup_new_character (CHAR_DATA * tch)
{
  int i = 0, flags = 0, skills = 0;
  char buf[MAX_STRING_LENGTH];

  /*      Add in auto-selected skills here, as well as psi-check          */
  tch->speed = SPEED_WALK;

  tch->pc->create_state = STATE_APPROVED;

  tch->str = tch->pc->start_str;
  tch->dex = tch->pc->start_dex;
  tch->intel = tch->pc->start_intel;
  tch->wil = tch->pc->start_wil;
  tch->aur = tch->pc->start_aur;
  tch->con = tch->pc->start_con;
  tch->agi = tch->pc->start_agi;

  tch->tmp_str = tch->str;
  tch->tmp_con = tch->con;
  tch->tmp_intel = tch->intel;
  tch->tmp_wil = tch->wil;
  tch->tmp_aur = tch->aur;
  tch->tmp_dex = tch->dex;
  tch->tmp_agi = tch->agi;

  tch->max_hit = 10 + 6 * GET_CON (tch);

  tch->max_move = calc_lookup (tch, REG_MISC, MISC_MAX_MOVE);

  tch->hit = GET_MAX_HIT (tch);
  tch->move = GET_MAX_MOVE (tch);

  if (lookup_race_variable (tch->race, RACE_NATIVE_TONGUE))
    {
      i =
	strtol (lookup_race_variable (tch->race, RACE_NATIVE_TONGUE), NULL,
		10);
      tch->skills[i] = calc_lookup (tch, REG_CAP, i);
      tch->pc->skills[i] = calc_lookup (tch, REG_CAP, i);
    }

  // Define all race-specific characteristics

  refresh_race_configuration (tch);


  for(i = 1; i<= LAST_SKILL; i++)
  {
    if(tch->skills[i] == 1)
    {
      skills += 1;
    }
  }

  open_skill (tch, SKILL_DODGE);
  starting_skill_boost (tch, SKILL_DODGE, 6);

  open_skill (tch, SKILL_DEFLECT);
  starting_skill_boost (tch, SKILL_DEFLECT, 6);

  open_skill (tch, SKILL_SMALL_BLADE);
  starting_skill_boost (tch, SKILL_SMALL_BLADE, 6);

  open_skill (tch, SKILL_AIM);
  starting_skill_boost (tch, SKILL_AIM, 6);

  open_skill (tch, SKILL_COMMON);

  //check_psionic_talents (tch);

  // If they get through char-gen without hide,
  // then we set their sneak and steal to zero.
  if (tch->skills[SKILL_HIDE] <= 0)
  {
	  tch->skills[SKILL_SNEAK] = 0;
	  tch->skills[SKILL_STEAL] = 0;
  }

  for(i = 1; i<= LAST_SKILL; i++)
  {
	if(tch->skills[i] == 1)
	{
	  starting_skill_boost (tch, i, skills);

	  // some sanity - greater than 45, get knocked back down to that. Less than 20, gets knocked up to that.

	  if (tch->skills[i] > 40)
	  {
		tch->skills[i] = 40;
	  }
	  else if (tch->skills[i] < 25)
	  {
		tch->skills[i] = 25;
	  }
	}
  }

  tch->skills[SKILL_COMMON] = 50;


/*
  for (i = 1; i <= LAST_SKILL; i++)
    {
      if (tch->skills[i] == 1)
	{
	  open_skill (tch, i);
	  starting_skill_boost (tch, i);
	}
    }
*/

  for (i = 1; i <= LAST_SKILL; i++)
    tch->pc->skills[i] = tch->skills[i];

  fix_offense (tch);

  tch->fight_mode = 2;

  tch->pc->nanny_state = 0;

  reformat_desc (tch->description, &tch->description);

  tch->plr_flags |= (NEWBIE | NEWBIE_HINTS | NEW_PLAYER_TAG);
  tch->flags |= FLAG_NEWBNET;

  tch->time.played = 0;

  sprintf (buf, "save/objs/%c/%s", tolower (*tch->tname), tch->tname);
  unlink (buf);

  tch->in_room = NOWHERE;

  // Set starting location for characters that didn't choose one in chargen

  if (num_starting_locs (tch->race) <= 1
      && lookup_race_variable (tch->race, RACE_START_LOC))
    {
      flags =
	strtol (lookup_race_variable (tch->race, RACE_START_LOC), NULL, 10);
      if (IS_SET (flags, RACE_HOME_MORGUL))
	tch->plr_flags |= START_MORDOR;
      else if (IS_SET (flags, RACE_HOME_OSGILIATH))
	tch->plr_flags |= START_GONDOR;
    }
}

void
answer_application (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch, *tmp_ch;
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  long int time_elapsed = 0;

  argument = one_argument (argument, buf);

  if (!ch->delay_who || !*ch->delay_who)
    {
      send_to_char ("Please REVIEW an application first.\n", ch);
      return;
    }

  if (ch->pc->msg)
    {
      mem_free (ch->pc->msg);
      ch->pc->msg = NULL;
    }

  sprintf (name, "%s", ch->delay_who);

  mem_free (ch->delay_who);
  ch->delay_who = NULL;
  ch->delay = 0;

  if (!(tch = load_pc (name)))
    {
      send_to_char ("Couldn't find PC...maybe a pfile corruption?\n\r", ch);
      return;
    }

  while (tch->pc->load_count > 1)
    unload_pc (tch);

  for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
    {
      if (tmp_ch->deleted)
	continue;
      if (tmp_ch->pc && tmp_ch->pc->edit_player
	  && !str_cmp (tmp_ch->pc->edit_player->tname, tch->tname))
	{
	  send_to_char
	    ("The PC in your editor has been closed for an application response.\n",
	     tmp_ch);
	  tmp_ch->pc->edit_player = NULL;
	}
    }

  if (tch->pc->create_state != 1)
    {
      sprintf (buf,
	       "It appears that this application has already been reviewed and responded to.\n");
      send_to_char (buf, ch);
      unload_pc (tch);
      return;
    }

  ch->delay_who = add_hash (tch->pc->account_name);
  ch->delay_who2 = add_hash (tch->tname);

  if (cmd != 345)
    {
      send_to_char
	("\n#2Please enter the changes necessary for this application to be approved.\n#0",
	 ch);
      send_to_char
	("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n",
	 ch);
      make_quiet (ch);
      ch->descr()->str = &ch->pc->msg;
      ch->descr()->max_str = MAX_STRING_LENGTH;
      ch->descr()->proc = email_rejection;
    }

  else if (cmd == 345)
    {
      send_to_char
	("\n#2Please enter any comments or advice you have regarding this approved character.\n#0",
	 ch);
      send_to_char
	("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n",
	 ch);
      make_quiet (ch);
      ch->descr()->str = &ch->pc->msg;
      ch->descr()->max_str = MAX_STRING_LENGTH;
      ch->descr()->proc = email_acceptance;
    }

  ch->pc->msg = NULL;

  if (tch->pc->create_state > STATE_SUBMITTED)
    {
      sprintf (buf, "PC is currently in create state %d.\n\r",
	       tch->pc->create_state);
      send_to_char (buf, ch);
      unload_pc (tch);
    }

  // Acceptance; process the new PC for entry into the game

  if (cmd == 345)
    setup_new_character (tch);
  else if (cmd != 345)
    tch->pc->create_state = STATE_REJECTED;

  time_elapsed = time (0) - tch->time.birth;

  mysql_safe_query ("INSERT INTO application_wait_times (wait_time) "
                    "VALUES (%d)",
		    (int) time_elapsed);
  mysql_safe_query ("DELETE FROM reviews_in_progress WHERE char_name = '%s'",
		    tch->tname);

  unload_pc (tch);
}

void
do_accept (CHAR_DATA * ch, char *argument, int cmd)
{
  if ((ch->delay_type != DEL_APP_APPROVE &&
       ch->delay_type != DEL_INVITE &&
       ch->delay_type != DEL_PURCHASE_ITEM &&
       ch->delay_type != DEL_ORDER_ITEM &&
	   ch->delay_type != DEL_ORDER_PLACE &&
	   ch->delay_type != DEL_ORDER_FULFILL &&
	   ch->delay_type != DEL_PLACE_AUCTION &&
	   ch->delay_type != DEL_CANCEL_AUCTION &&
	   ch->delay_type != DEL_PLACE_BID &&
	   ch->delay_type != DEL_PLACE_BUYOUT &&
	   ch->delay_type != DEL_RENT_COMMENCE &&
	   ch->delay_type != DEL_RENT_REPLACE &&
	   ch->delay_type != DEL_RENT_REPLICATE &&
	   ch->delay_type != DEL_RENT_TOPUP))
    {
      send_to_char ("No transaction is pending.\n\r", ch);
      return;
    }

  if (ch->delay_type == DEL_APP_APPROVE)
    {
      answer_application (ch, argument, 345);
      return;
    }
  else if (ch->delay_type == DEL_INVITE)
    {
      invite_accept (ch, argument);
      return;
    }
  else if (ch->delay_type == DEL_PURCHASE_ITEM)
    {
      do_buy (ch, "", 2);
      return;
    }
  else if (ch->delay_type == DEL_ORDER_ITEM)
    {
      do_order (ch, "", 2);
      return;
    }
  else if (ch->delay_type == DEL_PLACE_AUCTION)
    {
	  do_auction (ch, "", 1);
	  return;
	}
  else if (ch->delay_type == DEL_CANCEL_AUCTION)
    {
	  do_auction (ch, "", 3);
	  return;
	}
  else if (ch->delay_type == DEL_PLACE_BID)
    {
	  do_auction (ch, "", 2);
	  return;
	}
  else if (ch->delay_type == DEL_PLACE_BUYOUT)
    {
	  do_auction (ch, "", 3);
	  return;
	}
  else if (ch->delay_type == DEL_RENT_COMMENCE)
    {
	  do_rent (ch, "", 1);
	  return;
	}
  else if (ch->delay_type == DEL_RENT_TOPUP)
    {
	  do_rent (ch, "", 2);
	  return;
	}
  else if (ch->delay_type == DEL_RENT_REPLACE)
    {
	  do_rent (ch, "", 3);
	  return;
	}
  else if (ch->delay_type == DEL_RENT_REPLICATE)
    {
	  do_rent (ch, "", 4);
	  return;
	}
  else if (ch->delay_type == DEL_ORDER_PLACE)
    {
	  do_order (ch, ch->delay_who, 2);
	  return;
	}
  else if (ch->delay_type == DEL_ORDER_FULFILL)
    {
	  do_order (ch, ch->delay_who, 1);
	  return;
	}
  else
    send_to_char ("No transaction is pending.\n\r", ch);
}

void
activate_refresh (CHAR_DATA * ch)
{
  ch->move = GET_MAX_MOVE (ch);
}

void
act_black_curse (CHAR_DATA * ch)
{
}

void
do_decline (struct char_data *ch, char *argument, int cmd)
{
  if (ch->delay_type != DEL_PURCHASE_ITEM && ch->delay_type != DEL_PLACE_AUCTION &&
      ch->delay_type != DEL_APP_APPROVE && ch->delay_type != DEL_INVITE && ch->delay_type != DEL_CANCEL_AUCTION &&
	  ch->delay_type != DEL_PLACE_BID && ch->delay_type != DEL_PLACE_BUYOUT &&
	  ch->delay_type != DEL_ORDER_PLACE && ch->delay_type != DEL_ORDER_FULFILL &&
	  ch->delay_type != DEL_RENT_COMMENCE && ch->delay_type != DEL_RENT_TOPUP && ch->delay_type != DEL_RENT_REPLACE  && ch->delay_type != DEL_RENT_REPLICATE)
    {
      send_to_char ("No transaction is pending.\n\r", ch);
      return;
    }

  if (ch->delay_type == DEL_INVITE)
    {
      break_delay (ch);
      return;
    }

  if (ch->delay_type == DEL_APP_APPROVE)
    {
      answer_application (ch, argument, 0);
      return;
    }

  if (ch->delay_type == DEL_PURCHASE_ITEM || ch->delay_type == DEL_PLACE_AUCTION ||
		ch->delay_type == DEL_PLACE_BID || ch->delay_type == DEL_PLACE_BUYOUT ||
		ch->delay_type == DEL_ORDER_PLACE || ch->delay_type == DEL_ORDER_FULFILL ||
		ch->delay_type == DEL_CANCEL_AUCTION ||
		ch->delay_type == DEL_RENT_COMMENCE || ch->delay_type == DEL_RENT_TOPUP || ch->delay_type == DEL_RENT_REPLACE  || ch->delay_type == DEL_RENT_REPLICATE)
    {
      break_delay (ch);
      return;
    }

  if (ch->delay_who)
    {
      mem_free (ch->delay_who);
      ch->delay_who = NULL;
    }

  ch->delay = 0;
}

void
do_dreams (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  DREAM_DATA *dream;
  int count = 0;

  if (IS_NPC (ch))
    {
      send_to_char ("This is a PC only command.\n\r", ch);
      return;
    }

  if (!ch->pc->dreamed)
    {
      send_to_char ("You don't recall anything memorable.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  for (dream = ch->pc->dreamed; dream; dream = dream->next)
    count++;

  if (!*buf)
    {
      sprintf (buf, "You recall having %d dream%s.\n\r",
	       count, count > 1 ? "s" : "");
      send_to_char (buf, ch);
      return;
    }

  if (!isdigit (*buf))
    {
      send_to_char ("Either type 'dreams' or 'dreams <number>'.\n\r", ch);
      return;
    }

  if (strtol (buf, NULL, 10) > count || strtol (buf, NULL, 10) < 1)
    {
      send_to_char ("That number doesn't correspond to a dream.\n\r", ch);
      return;
    }

  count = strtol (buf, NULL, 10) - 1;

  for (dream = ch->pc->dreamed; count; count--)
    dream = dream->next;

  page_string (ch->descr(), dream->dream);
}

void
dream (CHAR_DATA * ch)
{
  DREAM_DATA *dream;
  DREAM_DATA *dreamed;

  if (!ch->pc || !ch->pc->dreams)
    return;

  if (GET_POS (ch) != POSITION_SLEEPING)
    return;

  dream = ch->pc->dreams;

  ch->pc->dreams = dream->next;
  dream->next = NULL;

  if (!ch->pc->dreamed)
    ch->pc->dreamed = dream;
  else
    {
      for (dreamed = ch->pc->dreamed; dreamed->next;)
	dreamed = dreamed->next;

      dreamed->next = dream;
    }

  send_to_char ("While asleep, you have a dream.\n\n\r", ch);

  page_string (ch->descr(), dream->dream);

  save_char (ch, true);
}

void
post_dream (DESCRIPTOR_DATA * d)
{
  time_t current_time;
  char *p;
  char *date;
  DREAM_DATA *dream;
  CHAR_DATA *ch;

  p = d->character->delay_who;
  ch = d->character->delay_ch;

  if (!p || !*p)
    {
      send_to_char ("Dream aborted.\n\r", d->character);
      unload_pc (d->character->delay_ch);
      return;
    }

  CREATE (dream, DREAM_DATA, 1);

  dream->dream = d->character->delay_who;
  dream->next = ch->pc->dreams;

  d->character->delay_who = NULL;

  ch->pc->dreams = dream;

  send_to_char ("Dream added.\n\r", d->character);

  current_time = time (0);

  date = (char *) asctime (localtime (&current_time));

  /* asctime adds a \n to the end of the date string - remove it */

  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  add_message (1,		/* new message */
	       GET_NAME (ch),	/* PC board */
	       -2,		/* Virtual # */
	       GET_NAME (d->character),	/* Imm name */
	       date,
	       "Entry via GIVEDREAM command.", "", dream->dream, MF_DREAM);

  unload_pc (ch);

  mem_free (date);
}

void
do_givedream (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *who;
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (!(who = load_pc (buf)))
    {
      send_to_char ("No such PC.\n\r", ch);
      return;
    }

  if (who->pc->create_state != 2)
    {
      send_to_char ("PC is not in state 2.\n\r", ch);
      unload_pc (who);
      return;
    }

  make_quiet (ch);

  ch->delay_who = NULL;

  ch->delay_ch = who;
  ch->descr()->str = &ch->delay_who;
  ch->descr()->max_str = MAX_INPUT_LENGTH;
  ch->descr()->proc = post_dream;
}

void
delayed_cast (CHAR_DATA * ch)
{
}

int
friendly (CHAR_DATA * ch, CHAR_DATA * friendPtr, int friendship)
{
  if (IS_SET (friendship, FRIEND_SELF))
    if (friendPtr == ch)
      return 1;

  if (IS_SET (friendship, FRIEND_CHURCH))
    if (ch->deity && ch->deity == friendPtr->deity)
      return 1;

  if (friendPtr->following != ch)
    return 0;

  if (IS_SET (friendship, FRIEND_GROUP) && IS_AFFECTED (friendPtr, AFF_GROUP))
    return 1;

  return 0;
}

int
resisted (CHAR_DATA * ch, CHAR_DATA * enemy, int spell)
{
  if ((number (1, 20) + (ch->circle - enemy->circle) +
       (ch->wil - enemy->wil)) <= GET_WIL (enemy))
    return 0;

  return 1;
}

void
magic_cure_infection (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    wound->infection = -1;
}

void
heal_wound (CHAR_DATA * ch, WOUND_DATA * wound, int damage)
{
  char buf[MAX_STRING_LENGTH];

  if ((wound->damage - damage) <= 0)
    {
      sprintf (buf, "A %s %s on your %s is healed!", wound->severity,
	       wound->name, expand_wound_loc (wound->location));
      wound->damage -= damage;
      wound_from_char (ch, wound);
    }
  else
    {
      sprintf (buf, "A %s %s on your %s mends rapidly.", wound->severity,
	       wound->name, expand_wound_loc (wound->location));
      wound->damage -= damage;
    }

  act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
}

void
magic_heal1 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (1, 3));

}

void
magic_heal2 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (2, 3));

}

void
magic_heal3 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (3, 3));

}

void
magic_heal4 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (4, 3));

}

void
magic_heal5 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (5, 3));

}

void
magic_heal6 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (6, 3));

}

void
magic_heal7 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (7, 3));

}

void
magic_heal8 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (8, 3));

}

void
magic_heal9 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (9, 3));

}

void
magic_heal10 (CHAR_DATA * ch)
{
  WOUND_DATA *wound;

  for (wound = ch->wounds; wound; wound = wound->next)
    heal_wound (ch, wound, dice (10, 3));

}

int
apply_affect (CHAR_DATA * ch, int type, int duration, int power)
{
  AFFECTED_TYPE *af;

  if ((af = get_affect (ch, type)))
    {

      if (af->a.spell.duration == -1)
	return 1;

      if (af->a.spell.duration < duration)
	af->a.spell.duration = duration;

      return 1;
    }

  CREATE (af, AFFECTED_TYPE, 1);

  af->type = type;
  af->a.spell.duration = duration;
  af->a.spell.modifier = power;
  af->a.spell.location = 0;
  af->a.spell.bitvector = 0;
  af->a.spell.t = 0;
  af->a.spell.sn = 0;

  affect_to_char (ch, af);

  return 0;
}

int magic_add_affect (CHAR_DATA * ch, int type, int duration, int modifier,
		  int location, int bitvector, int sn)
{
  AFFECTED_TYPE *af;

  if ((af = get_affect (ch, type)))
    {

      if (af->type > MAGIC_CLAN_MEMBER_BASE &&
	  af->type < MAGIC_CLAN_OMNI_BASE + MAX_CLANS)
	af->a.spell.t++;

      if (af->a.spell.duration == -1)	/* Perm already */
	return 0;

      if (duration == -1)
	af->a.spell.duration = duration;
      else
	af->a.spell.duration += duration;

      af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
      return 0;
    }

  CREATE (af, AFFECTED_TYPE, 1);

  af->type = type;
  af->a.spell.duration = duration;
  af->a.spell.modifier = modifier;
  af->a.spell.location = location;
  af->a.spell.bitvector = bitvector;
  af->a.spell.t = 0;
  af->a.spell.sn = sn;

  affect_to_char (ch, af);

  return 1;
}

int
add_affect (CHAR_DATA * ch, int type, int uu1, int uu2, int uu3, int uu4, int uu5, int uu6)
{
  AFFECTED_TYPE *af;
  CREATE (af, AFFECTED_TYPE, 1);

  af->type = type;
  af->a.affect.uu1 = uu1;
  af->a.affect.uu2 = uu2;
  af->a.affect.uu3 = uu3;
  af->a.affect.uu4 = uu4;
  af->a.affect.uu5 = uu5;
  af->a.affect.uu6 = uu6;

  affect_to_char (ch, af);
}


void
job_add_affect (CHAR_DATA * ch, int type, int days, int pay_date, int cash,
		int count, int object_vnum, int employer)
{
  AFFECTED_TYPE *af;

  if ((af = get_affect (ch, type)))
    affect_remove (ch, af);

  af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

  af->type = type;
  af->a.job.days = days;
  af->a.job.pay_date = pay_date;
  af->a.job.cash = cash;
  af->a.job.count = count;
  af->a.job.object_vnum = object_vnum;
  af->a.job.employer = employer;

  affect_to_char (ch, af);
}

int
magic_add_obj_affect (OBJ_DATA * obj, int type, int duration, int modifier,
		      int location, int bitvector, int sn)
{
  AFFECTED_TYPE *af;

  if ((af = get_obj_affect (obj, type)))
    {
      af->a.spell.duration += duration;
      af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
      return 0;
    }

  CREATE (af, AFFECTED_TYPE, 1);

  af->type = type;
  af->a.spell.duration = duration;
  af->a.spell.modifier = modifier;
  af->a.spell.location = location;
  af->a.spell.bitvector = bitvector;
  af->a.spell.sn = sn;

  affect_to_obj (obj, af);

  return 1;
}

void
magic_affect (CHAR_DATA * ch, int magic)
{
  switch (magic)
    {
    default:
      printf ("Unknown magic: %d\n", magic);
      break;
    }
}

void
apply_perfume (CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
  /*
  if (obj->o.perfume.doses <= 0)
    {
      act ("$p has no more aroma.", false, ch, obj, 0, TO_CHAR);
      return;
    }

  obj->o.perfume.doses--;

  if (ch != victim)
    {
      act ("You apply the scent of $p onto $N.",
	   false, ch, obj, victim, TO_CHAR);
      act ("$N applies the scent of $p onto you.",
	   false, victim, obj, ch, TO_CHAR);
      act ("$n applies the scent of $p onto $N.",
	   false, ch, obj, victim, TO_NOTVICT);
    }
  else
    {
      act ("You apply the scent of $p to yourself.",
	   false, ch, obj, 0, TO_CHAR);
      act ("$n applies the scent of $p to $mself.",
	   false, ch, obj, 0, TO_ROOM);
    }

  apply_affect (victim, obj->o.perfume.type, obj->o.perfume.duration,
		obj->o.perfume.aroma_strength);
*/
}

void
do_see (CHAR_DATA * ch, char *argument, int cmd)
{
  int save_room_num;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *tch;

  if (!*argument)
    {
      send_to_char ("Who would you like to scry?\n", ch);
      return;
    }

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("See how?  You are not clairvoyant.\n\r", ch);
      return;
    }

  if (GET_MOVE (ch) <= 40)
    {
      send_to_char ("You are too fatigued for that.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  tch = get_char_vis (ch, buf);

  if (!tch || GET_TRUST (tch))
    {
      weaken (ch, 0, 10, "using SEE and failed");
      send_to_char ("Midway through your trance, you realize that you cannot "
		    "locate\n\r" "that person.\n\r", ch);
      return;
    }

  if (tch == ch)
    {
      send_to_char ("You don't mean yourself, do you?\n\r", ch);
      return;
    }

  weaken (ch, 0, 40, "using SEE");

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      act ("You sense $N, but cannot see $M.", false, ch, 0, tch, TO_CHAR);
      return;
    }

  if (get_affect (tch, MAGIC_HIDDEN) || is_hooded (tch))
    {

      if (get_affect (tch, MAGIC_HIDDEN) && tch->in_room == ch->in_room)
	act ("You sense that person is here somewhere!",
	     false, ch, 0, 0, TO_CHAR);

      else if (is_hooded (tch) && tch->in_room == ch->in_room)
	act ("That person is someone here!", false, ch, 0, 0, TO_CHAR);

      else
	act ("You sense $N, but cannot see $M.", false, ch, 0, tch, TO_CHAR);
      return;
    }

  save_room_num = ch->in_room;

  char_from_room (ch);
  char_to_room (ch, tch->in_room);

  do_look (ch, "", 0);

  char_from_room (ch);
  char_to_room (ch, save_room_num);
}

void
do_heal (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *tch;

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("You have no idea how to do that.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(tch = get_char_room_vis (ch, buf)))
    {
      send_to_char ("You don't see that person here.\n\r", ch);
      return;
    }

  if (tch == ch)
    {
      send_to_char
	("You would not benefit by psionically healing yourself.\n\r", ch);
      return;
    }

  if (GET_TRUST (tch))
    {
      act ("$E is particularly undeserving of that.",
	   false, ch, 0, tch, TO_CHAR);
      act ("$N kindly tried to heal you.", false, tch, 0, ch, TO_CHAR);
      return;
    }

  act ("You lay your hand on $N.", false, ch, 0, tch, TO_CHAR);
  act ("$N gently lays $S hand upon you.", false, tch, 0, ch, TO_CHAR);
  act ("$n gently touches $N.", false, ch, 0, tch, TO_NOTVICT);
  sense_activity (ch, SKILL_VOODOO);

  ch->delay_info1 = strtol (buf, NULL, 10);
  ch->delay_ch = tch;
  ch->delay_type = DEL_EMPATHIC_HEAL;
  ch->delay = 10;
}

void
reset_heal_delay (CHAR_DATA * ch, CHAR_DATA * tch)
{
  ch->delay_ch = tch;
  ch->delay_type = DEL_EMPATHIC_HEAL;
  ch->delay = 10;
}

void
delayed_heal (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  WOUND_DATA *wound;
  char buf[MAX_STRING_LENGTH];
  int woundtype = 0;
  float damage = 0;

  tch = ch->delay_ch;

  if (!tch->wounds)
    {
      send_to_char ("There are no more wounds left to heal.\n", ch);
      return;
    }

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      act ("You grimace in pain as the link between you suddenly dissipates.",
	   false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
      act ("$n grimaces in pain as the link between you suddenly dissipates.",
	   false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
      act ("$n grimaces in pain, slumping back with a sigh.", false, ch, 0,
	   tch, TO_NOTVICT | _ACT_FORMAT);
      return;
    }

  if (!is_he_here (ch, tch, 1))
    {

      if (is_he_somewhere (tch))
	act ("$N has left, you can no longer heal $M.",
	     false, ch, 0, tch, TO_CHAR);
      else
	act ("Your target has left.", false, ch, 0, 0, TO_CHAR);

      return;
    }

  for (wound = ch->wounds; wound; wound = wound->next)
    damage += wound->damage;

  damage += ch->damage;
  damage += tch->wounds->damage;

  if (damage > ch->max_hit * .84)
    {
      act
	("Grimacing in pain, you sever the link, unable to endure it any longer.",
	 false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
      act ("$n grimaces in pain, and you feel the link between you severed.",
	   false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
      act ("$n grimaces in pain, slumping back with a sigh.", false, ch, 0,
	   tch, TO_NOTVICT | _ACT_FORMAT);
      return;
    }

  damage = tch->wounds->damage;

  if (!str_cmp (tch->wounds->type, "slash"))
    woundtype = 2;
  else if (!str_cmp (tch->wounds->type, "pierce"))
    woundtype = 1;
  else if (!str_cmp (tch->wounds->type, "blunt"))
    woundtype = 3;
  else if (!str_cmp (tch->wounds->type, "fire"))
    woundtype = 6;
  else if (!str_cmp (tch->wounds->type, "frost"))
    woundtype = 5;
  else if (!str_cmp (tch->wounds->type, "bite"))
    woundtype = 7;
  else if (!str_cmp (tch->wounds->type, "claw"))
    woundtype = 8;
  else if (!str_cmp (tch->wounds->type, "fist"))
    woundtype = 9;

  sprintf (buf,
	   "You grit your teeth in pain as a %s %s appears on your %s, and the"
	   " corresponding wound on $N fades quickly away.",
	   tch->wounds->severity, tch->wounds->name,
	   expand_wound_loc (tch->wounds->location));
  act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
  sprintf (buf,
	   "$n gasps in pain as a %s %s suddenly appears on $s %s, and the"
	   " corresponding wound on you fades quickly away.",
	   tch->wounds->severity, tch->wounds->name,
	   expand_wound_loc (tch->wounds->location));
  act (buf, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
  sprintf (buf,
	   "$n gasps in pain as a %s %s suddenly appears on $s %s, and the"
	   " corresponding wound on $N fades quickly away.",
	   tch->wounds->severity, tch->wounds->name,
	   expand_wound_loc (tch->wounds->location));
  act (buf, false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);

  wound_to_char (ch, tch->wounds->location, (int) damage, woundtype,
		 tch->wounds->bleeding, tch->wounds->poison,
		 tch->wounds->infection);
  wound_from_char (tch, tch->wounds);

  reset_heal_delay (ch, tch);
}


void
sense_activity (CHAR_DATA * user, int talent)
{
  CHAR_DATA *ch;
  int modifier = 0;

  for (ch = user->room->people; ch; ch = ch->next_in_room)
    {
      if (ch == user)
	continue;
      if (!real_skill (ch, SKILL_VOODOO))
	continue;

      modifier += user->skills[talent] / 3;
      modifier -= ch->skills[SKILL_VOODOO] / 5;

      if (!skill_use (ch, SKILL_VOODOO, modifier))
	return;

      if (talent == SKILL_VOODOO)
	act
	  ("You sense $N gather $S energies and project a stream of thought into the aether.",
	   false, ch, 0, user, TO_CHAR | _ACT_FORMAT);
      else if (talent == SKILL_VOODOO)
	act ("You sense $N open $S psyche to the world around $M.", false, ch,
	     0, user, TO_CHAR | _ACT_FORMAT);
      else if (talent == SKILL_VOODOO)
	act
	  ("You sense a thrumming torrent of psychic energies gathering about $N.",
	   false, ch, 0, user, TO_CHAR | _ACT_FORMAT);
      else if (talent == SKILL_VOODOO)
	act ("You sense $N's psyche flow outward in a gentle wave.", false,
	     ch, 0, user, TO_CHAR | _ACT_FORMAT);
      else if (talent == SKILL_VOODOO)
	act
	  ("You sense $N's psychic energies quest outward in a malignant tendril.",
	   false, ch, 0, user, TO_CHAR | _ACT_FORMAT);
    }
}

void
do_sense (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];
  char *adjs[] = { "dim", "dull", "glimmering", "gleaming", "bright",
    "radiant", "brilliant", "breathtakingly bright", "stunningly brilliant",
    "overwhelmingly radiant", "blinding"
  };
  int aura;

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("You strain, but sense nothing.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(tch = get_char_room_vis (ch, buf)))
    {
      send_to_char ("You don't see that person here.\n\r", ch);
      return;
    }

  if (GET_HIT (ch) + GET_MOVE (ch) < 10)
    {
      send_to_char ("You don't feel strong enough at the moment.\n\r", ch);
      return;
    }

  weaken (ch, 0, 10, buf);
  sense_activity (ch, SKILL_VOODOO);

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      act ("You cannot muster a sense of $N's spiritual strength.", false,
	   ch, 0, tch, TO_CHAR);
      return;
    }

  aura = MIN (GET_AUR (tch), 22);

  if (aura <= 9)
    aura = 12;
  else if (aura <= 13)
    aura = 13;

  sprintf (buf, "You sense that %s spirit is %s.\n",
	   ch == tch ? "your" : "$N's", adjs[aura - 12]);

  act (buf, false, ch, 0, tch, TO_CHAR);
}

void
do_bolt (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("You don't know how to do that.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(tch = get_char_room_vis (ch, buf)))
    {
      send_to_char ("You don't see that person here.\n\r", ch);
      return;
    }

  if (ch == tch)
    {
      send_to_char ("Would that be wise?\n\r", ch);
      return;
    }

  if (GET_MOVE (ch) <= 40)
    {
      send_to_char ("You don't feel rested enough at the moment.\n\r", ch);
      return;
    }

  act ("You gather your psychic energies, preparing to unleash them upon $N.",
       false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
  sense_activity (ch, SKILL_VOODOO);

  ch->delay_ch = tch;
  ch->delay_type = DEL_MENTAL_BOLT;
  ch->delay = 15 - ch->skills[SKILL_VOODOO] / 10;
}

void
delayed_bolt (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  int divisor = 1, roll, reflect = 0, modifier = 1, stun = 0, fatigue = 0;
  char buf[MAX_STRING_LENGTH];

  tch = ch->delay_ch;

  if (!is_he_here (ch, tch, 1))
    {
      send_to_char ("Your victim isn't here anymore.\n\r", ch);
      return;
    }

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      act
	("You lose your concentration, and your energies dissipate ineffectually.",
	 false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
      weaken (ch, 0, 15, 0);
      return;
    }

  fatigue = 40;
  fatigue -= number (1, 10);
  fatigue -= ch->skills[SKILL_VOODOO] / 5;
  fatigue = MAX (5, fatigue);

  weaken (ch, 0, fatigue, 0);

  if (IS_IMPLEMENTOR (tch))
    reflect++;

  if ((roll = number (1, 100)) > ch->skills[SKILL_VOODOO])
    {
      if (roll % 5 == 0)
	reflect++;
      else
	divisor = 2;
    }
  else
    {
      if (roll % 5 == 0)
	modifier = ch->skills[SKILL_VOODOO] / 10;
    }

  if (IS_IMPLEMENTOR (ch))
    reflect = 0;

  if (reflect)
    {
      if ((number (1, 22) - (tch->wil - ch->wil)) <= tch->wil)
	{
	  act
	    ("A wave of psychic energies slams into your mind, but you grit your teeth and manage to hold it at bay.",
	     false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
	  act
	    ("You send your energies hurling into $N's mind, but they slam up against a psychic barrier.",
	     false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
	  act ("$N grits $S teeth, paling for a moment", false, ch, 0, tch,
	       TO_NOTVICT | _ACT_FORMAT);
	  return;
	}
      else
	{
	  act
	    ("A wave of psychic energy floods your mind, but with a great amount of effort, you manage to redirect it to its source.",
	     false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
	  act
	    ("You send your energies hurling into $N's mind, but to your horror, they are redirected and come rushing forcefully back into your psyche!",
	     false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
	  act ("$n staggers backward with a surprised look on $s face.",
	       false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
	  ch->stun +=
	    (number (1, ch->skills[SKILL_VOODOO]) / 6) / divisor *
	    modifier;
	  return;
	}
    }

  act
    ("You send your energies hurling violently into $N's psyche, who staggers backward in shock.",
     false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
  act
    ("A searing wave of psychic energy slams into your mind, sending you reeling.",
     false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
  act ("$N staggers backward, face contorted in shock.", false, ch, 0, tch,
       TO_NOTVICT | _ACT_FORMAT);

  stun = (number (1, ch->skills[SKILL_VOODOO]));
  stun /= divisor;
  stun *= modifier;
  tch->stun += stun;

  sprintf (buf, "fatigue: %d divisor: %d modifier: %d stun: %d", fatigue,
	   divisor, modifier, stun);
}

void
do_prescience (CHAR_DATA * ch, char *argument, int cmd)
{
  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("That's not something you know how to do.\n\r", ch);
      return;
    }

  if (IS_NPC (ch))
    {
      send_to_char ("This is a PC only command.\n\r", ch);
      return;
    }

  if (GET_MOVE (ch) <= 50)
    {
      send_to_char
	("You are too weary to muster the necessary concentration.\n\r", ch);
      return;
    }

  weaken (ch, 0, 50, "Prescience");

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      act
	("You begin spiralling downward into meditative contemplation, preparing an attempt to part the Veil of the future. At the last moment, however, a distracting thought races through your mind, and your concentration is ruined.",
	 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
      return;
    }

  send_to_char ("\n", ch);
  act
    ("#6You begin spiralling downward into meditative contemplation, preparing to part the Veil of the future, and the mysteries of the Valar. At long last your mind is focused and tranquil; you begin to ponder your question:#0",
     false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

  ch->descr()->str = &ch->delay_who;
  ch->descr()->proc = post_prescience;

  if (IS_SET (ch->descr()->edit_mode, MODE_VISEDIT))
    ve_setup_screen (ch->descr());
  else
    ch->descr()->max_str = MAX_INPUT_LENGTH;
}

void
post_prescience (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  char *date;
  time_t current_time;
  char buf[MAX_STRING_LENGTH];

  current_time = time (0);

  date = (char *) asctime (localtime (&current_time));

  /* asctime adds a \n to the end of the date string - remove it */

  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  ch = d->character;

  sprintf (buf, "#3%s:#0 %d Aura, %d Prescience.",
	   GET_NAME (ch), GET_AUR (ch), ch->skills[SKILL_VOODOO]);

  add_message (1, "Prescience", -5, GET_NAME (ch), date, buf, "",
	       ch->delay_who, MF_DREAM);

  mem_free (date);

  ch->delay_who = NULL;
}

void
do_reach (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *target;
  char *p;
  char buf[MAX_STRING_LENGTH];
  char msg_to_send[MAX_STRING_LENGTH];

  /* Skill use gets done by whipser_it.  whisper_it is not
     called unless the target is online.
   */

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("You have not been able to develop telepathic abilities "
		    "as of yet.\n", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!*buf)
    {
      send_to_char ("Reach whom with what message?\n", ch);
      return;
    }

  while (*argument == ' ')
    argument++;

  if (!*argument)
    {
      send_to_char ("What message did you wish to send?\n", ch);
      return;
    }

  if (!(target = get_char_nomask (buf)))
    {
      send_to_char ("Who did you want to reach?\n", ch);
      return;
    }

  if (target == ch)
    {
      send_to_char ("That really isn't necessary, is it?\n", ch);
      return;
    }

  send_to_char ("You concentrate on your message, projecting it outward.\n",
		ch);
  sense_activity (ch, SKILL_VOODOO);

  whisper_it (ch, argument, msg_to_send, 0);

  if (skill_use (target, SKILL_VOODOO, 0))
    sprintf (buf,
	     "A fleeting vision of #5%s#0 accompanies an ethereal whisper in your mind:",
	     char_short (ch));
  else
    sprintf (buf, "An ethereal voice touches your mind:");

  act (buf, false, target, 0, 0, TO_CHAR | _ACT_FORMAT);

  reformat_say_string (msg_to_send, &p, 0);
  sprintf (buf, "   \"%s\"\n", p);
  send_to_char (buf, target);

  weaken (ch, 0, 10, "Reach (succeeded)");
}
