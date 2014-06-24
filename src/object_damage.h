//////////////////////////////////////////////////////////////////////////////
//
/// object_damage.h - Object Damage Class
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2004-2006 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
/// URL: http://www.middle-earth.us
//
/// May includes portions derived from Harshlands
/// Authors: Charles Rand (Rassilon)
/// URL: http://www.harshlands.net
//
/// May include portions derived under license from DikuMUD Gamma (0.0)
/// which are Copyright (C) 1990, 1991 DIKU
/// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
///          Tom Madson (noop@freja.diku.dk)
///          Katja Nyboe (katz@freja.diku.dk)
///          Michael Seifert (seifert@freja.diku.dk)
///          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _rpie_object_damage_h_
#define _rpie_object_damage_h_

/*--------------------------------------.
|  OBJECT_DAMAGE                         \
|------------------------------------------------------------------------.
|                                                                         |
|  public members:                                                        |
|    damage_type[] : array of types of damage                             |
|                                                                         |
|  public methods:                                                        |
|    object_damage__new :            alloc and return an instance.        |
|    object_damage__new_init :       as above plus initialization.        |
|    object_damage__get_sdesc :      return a short desc of the damage    |
|    object_damage__write_to_file :  write the object to a file           |
|    object_damage__read_from_file : read the object from a file          |
|                                                                         |
`------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------.
|                                                                         |
|  IMPORTANT:                                                             |
|                                                                         |
|  You can globally erase the damage from objects by shutting down the    |
|  server and running the following UNIX command:                         |
|                                                                         |
|  $ grep -R -l "^Damage" ./lib | xargs sed -i.bak '/^Damage/d'           |
|                                                                         |
`------------------------------------------------------------------------*/

#include <stdio.h>

enum e_condition_type
{
    COND_STAIN = 0,
    COND_DIRT,
    COND_DUST,
    COND_BLOOD,
    COND_WATER,
    COND_FILTH,
    COND_SLIME,
    COND_RUST,
    COND_TYPE_MAX = 8
};


typedef struct object_damage OBJECT_DAMAGE;

enum e_damage_type
{
  DAMAGE_PIERCE = 0,
  DAMAGE_BLUNT,
  DAMAGE_SLASH,
  DAMAGE_FREEZE,
  DAMAGE_BURN,
  DAMAGE_FIST,
  DAMAGE_BLOOD,
  DAMAGE_WATER,
  DAMAGE_PERMANENT,
  DAMAGE_REPAIR,
  DAMAGE_BULLET,
  DAMAGE_TYPE_MAX = 11
};
typedef enum e_damage_type DAMAGE_TYPE;

enum e_material_skill
{

  MATERIAL_SKILL_UNDEFINED = 0,

  //MATERIAL_TEXTILECRAFT = (1 << 0),	/* 00001 -  1 */
};
typedef enum e_material_skill MATERIAL_SKILL;

enum e_material_type
{

  MATERIAL_TYPE_UNDEFINED = 0,

  MATERIAL_ORGANIC,
  MATERIAL_TEXTILE,
  MATERIAL_METAL,
  MATERIAL_CERAMIC,
  MATERIAL_GLASS,
  MATERIAL_ELECTRONIC,
  MATERIAL_PLASTICS,

  MATERIAL_TYPE_MAX = 8
};
typedef enum e_material_type MATERIAL_TYPE;

enum e_damage_severity
{
  DAMAGE_SMALL = 0,
  DAMAGE_MINOR,
  DAMAGE_MODERATE,
  DAMAGE_LARGE,
  DAMAGE_HUGE,
  DAMAGE_MASSIVE,
  DAMAGE_SEVERITY_MAX
};
typedef enum e_damage_severity DAMAGE_SEVERITY;

struct object_damage
{
  DAMAGE_TYPE source;		/* what was the cause of damage */
  MATERIAL_TYPE material;	/* for multi-material object support */
  DAMAGE_SEVERITY severity;	/* index to impact adjective */
  ushort impact;		/* damage inflicted on the object */
  ushort name;			/* a random damage name */
  int permanent;		/* the vnum of a lodged object, e.g. arrow */
  int when;
  bool collated;                /* marker for cover-all damage types */
  OBJECT_DAMAGE *next;
};

extern const char *damage_type[];
extern const char *enviro_desc[COND_TYPE_MAX][6];
OBJECT_DAMAGE *object_damage__new ();
OBJECT_DAMAGE *object_damage__new_init (DAMAGE_TYPE source, int impact,
					MATERIAL_TYPE target, int lodged, int cmd);
char *object_damage__get_sdesc (OBJECT_DAMAGE * thisPtr);
int object_damage__write_to_file (OBJECT_DAMAGE * thisPtr, FILE * fp);
OBJECT_DAMAGE *object_damage__read_from_file (FILE * fp);

#endif // _rpie_object_damage_h_
