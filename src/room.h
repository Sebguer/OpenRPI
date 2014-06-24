//////////////////////////////////////////////////////////////////////////////
//
/// room.h - Room Class Structures and Functions
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


#ifndef _rpie_room_h
#define _rpie_room_h

#include "structs.h"
#include "net_link.h"

enum exit_state
  {
    unlocked_and_open,
    unlocked_and_closed,
    locked_and_closed,
    gate_unlocked_and_open,
    gate_unlocked_and_closed,
    gate_locked_and_closed
  };





typedef struct room_data ROOM_DATA;

/* Database Schema

CREATE TABLE `bp_rooms` (
  `rid` int(7) unsigned NOT NULL default '0' COMMENT 'room vnum',
  `zone` smallint(3) unsigned NOT NULL default '0' COMMENT 'room zone',
  `terrain` enum('Inside','City','Road','Trail','Field','Woods','Forest','Hills','Mountain','Swamp','Dock','Crowsnest','Pasture','Heath','Pit','Leanto','Lake','River','Ocean','Reef','Underwater') NOT NULL default 'Field' COMMENT 'sector type',
  `flags` set('Dark','Ruins','No Mob','Indoors','Lawful','No Magic','Tunnel','Cave','Safe Quit','Deep','Fall','No Mount','Vehicle','Stifling Fog','No Merchant','Climb','Save','Lab','Road','Wealthy','Poor','Scum','Temporary','Arena','Dock','Wild','Light','No Hide','Storage','PC Entered','Room OK','OOC') NOT NULL default '',
  `name` varchar(255) NOT NULL default '''Unnamed''' COMMENT 'room name',
  PRIMARY KEY  (`rid`),
  KEY `zone` (`zone`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

*/


struct room_data
{
  int vnum;					// Make sure this is 1st element
  int zone;					// Make sure this is 2nd element
  int sector_type;
  int deity;				// Deity of room and num occupants for leanto
  int light;
  int search_sequence;		// used in track ()
  char *name;
  char *description;
  EXTRA_DESCR_DATA *ex_description;
  struct written_descr_data *wdesc;
  struct room_direction_data *dir_option[LAST_DIR + 1];
  struct secret *secrets[LAST_DIR + 1];
  int room_flags;
  OBJ_DATA *contents;
  CHAR_DATA *people;
  struct room_prog *prg;
  ROOM_DATA *hnext;		// next in hash list
  ROOM_DATA *lnext;		// Link next room in room_list
  AFFECTED_TYPE *affects;
  ROOM_EXTRA_DATA *extra;
  bool psave_loaded;
  TRACK_DATA *tracks;
  int entrance;			/// used for carrying shouts into dwellings
  int occupants;
  int xerox; //what the room is based upon.
  int capacity; // how many people can be in a room.
  int enviro_type; // What type of enviromental condition we pass on.
  int enviro_power; // how powerful it is.
  SCENT_DATA *scent;
  Neighbourhood *hood;
};

extern ROOM_DATA *full_room_list;
bool is_overcast (ROOM_DATA * room);
bool is_sunlight_restricted (CHAR_DATA * ch, ROOM_DATA * room = 0);
int isguarded (ROOM_DATA * room, int dir);
void fwrite_room (ROOM_DATA * troom, FILE * fp);
int free_room (ROOM_DATA * room);
ROOM_DATA *generate_dwelling_room (OBJ_DATA * dwelling);
int remove_room_affect (ROOM_DATA * room, int type);
void load_save_room (ROOM_DATA * room);
void save_player_rooms ();
void load_save_rooms ();
void load_mysql_save_rooms ();
void add_room_to_hash (ROOM_DATA * add_room);
int track_room (ROOM_DATA * from_room, int to_room);
int is_dark (ROOM_DATA * room);

void setup_dir (FILE * fl, ROOM_DATA * room, int dir, int type);

ROOM_DATA *allocate_room (int nVirtual);
void clone_room (ROOM_DATA * source_room, ROOM_DATA * targ_room, bool clone_exits, bool copy_objs);
int clone_contiguous_rblock (ROOM_DATA * start_room, int from_dir, bool copy_objs);
int delete_contiguous_temporary_rblock (ROOM_DATA * start_room, int from_dir, int outside_vnum);
int delete_contiguous_rblock (ROOM_DATA * start_room, int from_dir, int outside_vnum);
ROOM_DATA *vnum_to_room (int nVirtual);

int rdelete (ROOM_DATA * room);
void room_light (ROOM_DATA * room);

void room_affect_wearoff (ROOM_DATA * room, int type);
CHAR_DATA *toll_collector (ROOM_DATA * room, int dir);

int is_he_there (CHAR_DATA * ch, ROOM_DATA * room);

inline bool
set_door_state (int room_number, int direction, exit_state state)
{
  ROOM_DATA* room = 0;
  if ((room = vnum_to_room (room_number)))
    {
      ROOM_DIRECTION_DATA *door;
      if ((door = room->dir_option[direction]))
	{
	  switch (state)
	    {
	    case unlocked_and_open:
	      door->exit_info &= ~(EX_LOCKED | EX_CLOSED);
	      break;

	    case unlocked_and_closed:
	      door->exit_info |= EX_CLOSED;
	      door->exit_info &= ~EX_LOCKED;
	      break;

	    case locked_and_closed:
	      door->exit_info |= (EX_LOCKED | EX_CLOSED);
	      break;

			case gate_unlocked_and_open:
	      door->exit_info &= ~(EX_LOCKED | EX_CLOSED);
	      door->exit_info |= EX_ISGATE;
	      break;

	    case gate_unlocked_and_closed:
	      door->exit_info |= EX_CLOSED;
	      door->exit_info &= ~EX_LOCKED;
	      door->exit_info |= EX_ISGATE;
	      break;

	    case gate_locked_and_closed:
	      door->exit_info |= (EX_LOCKED | EX_CLOSED);
	      door->exit_info |= EX_ISGATE;
	      break;
	    }
	  return true;
	}
    }
   return false;
}

#endif // _rpie_room_h
