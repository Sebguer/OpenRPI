//////////////////////////////////////////////////////////////////////////////
//
/// clan.h - Character Clan Structures and Utility Functions
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2005-2006 C. W. McHenry
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



#ifndef _rpie_clan_h_
#define _rpie_clan_h_

#include <stdio.h>
#include "structs.h"


typedef struct clan_data CLAN_DATA;

#define CLAN_MEMBER		(1 << 0)
#define CLAN_LEADER		(1 << 1)
#define CLAN_MEMBER_OBJ		(1 << 2)
#define CLAN_LEADER_OBJ		(1 << 3)
#define CLAN_RECRUIT		(1 << 4)
#define CLAN_PRIVATE		(1 << 5)
#define CLAN_CORPORAL		(1 << 6)
#define CLAN_SERGEANT		(1 << 7)
#define CLAN_LIEUTENANT		(1 << 8)
#define CLAN_CAPTAIN		(1 << 9)
#define CLAN_GENERAL		(1 << 10)
#define CLAN_COMMANDER		(1 << 11)
#define CLAN_APPRENTICE		(1 << 12)
#define CLAN_JOURNEYMAN		(1 << 13)
#define CLAN_MASTER		(1 << 14)

#define MAX_CLANS		100

struct clan_data
{
  char *name;
  int zone;
  char *literal;
  int member_vnum;
  int leader_vnum;
  int omni_vnum;
  int id;
  CLAN_DATA *next;
};

/* player interface */
void do_clan (CHAR_DATA * ch, char *argument, int cmd);
void clan_forum_remove_all (CHAR_DATA * ch);

char *get_shared_clan (CHAR_DATA * ch, CHAR_DATA * other);
void add_clan_id (CHAR_DATA * ch, char *clan_name, const char *clan_flags);
void add_clan_id_string (char *string, char *clan_name, char *clan_flags);
int get_clan (CHAR_DATA * ch, const char *clan, int *clan_flags);
int get_clan_in_string (char *string, char *clan, int *clan_flags);
int get_next_clan (char **p, char *clan_name, int *clan_flags);
void add_clan (CHAR_DATA * ch, char *new_clan_name, int clan_flags);
char *add_clan_to_string (char *string, char *new_clan_name, int clan_flags);
void remove_clan (CHAR_DATA * ch, char *old_clan_name);
char *remove_clan_from_string (char *string, char *old_clan_name);
char *value_to_clan_flags (int flags);
int clan_flags_to_value (char *flag_names);
int clan_flags_to_value (char *flag_names, char *clan_name); // Japheth's addition 2/8/06
int get_next_leader (char **p, char *clan_name, int *clan_flags);
void update_enforcement_array (CHAR_DATA * ch);

inline int is_area_enforcer (CHAR_DATA * ch)
{
  // this func gets called lots but the result rarely changes
  if (!ch->enforcement[0])
    update_enforcement_array (ch);

  return ch->enforcement[ch->room->zone];
}

int is_area_leader (CHAR_DATA * ch);
char *get_clan_rank_name (int flags);
char *get_clan_rank_name (CHAR_DATA *ch, char *clan_name, int flags); // Japheth's addition 7/6/06
CLAN_DATA *get_clandef (const char *clan_name);
CLAN_DATA *get_clandef_long (char *clan_long_name);
CLAN_DATA *get_clanid (int clan_id);
int is_clan_member (CHAR_DATA * ch, char *clan_name);
void clan_object_equip (CHAR_DATA * ch, OBJ_DATA * obj);
void clan_object_unequip (CHAR_DATA * ch, OBJ_DATA * obj);
int is_clan_member_player (CHAR_DATA * ch, char *clan_name);
int get_clan_long (CHAR_DATA * ch, char *clan_name, int *clan_flags);
int get_clan_long_short (CHAR_DATA * ch, char *clan_name, int *clan_flags);

void do_rollcall (CHAR_DATA * ch, char *argument, int cmd);

void clan__do_score (CHAR_DATA * ch);


/* database synchronization */
void clan__do_add (CLAN_DATA * clan);
void clan__do_update (CLAN_DATA * clan);
void clan__do_remove (CLAN_DATA * clan);
void clan__do_load ();

char *display_clan_ranks (CHAR_DATA * ch, CHAR_DATA * observer);
void add_clandef (char *argument);
void clan__assert_member_objs ();	/* caller: db.c:boot_objects()                  */
void clan__filedump (FILE * fp);	/* caller: staff.c:write_dynamic_registry()     */



extern CLAN_DATA *clan_list;
extern NAME_SWITCH_DATA *clan_name_switch_list;


#endif // _rpie_clan_h_
