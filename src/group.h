//////////////////////////////////////////////////////////////////////////////
//
/// group.h - Character Group Utility Functions
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


#ifndef _rpie_group_h_
#define _rpie_group_h_

inline bool
are_grouped (const CHAR_DATA* a, const CHAR_DATA* b)
{
  // todo: remove need for sanity check 
  if (a && b && a != b)
    {
      CHAR_DATA *af = a->following;
      CHAR_DATA *bf = b->following;
      
      return (af == b || bf == a || (af && af == bf));
    }
  return false;
}


inline bool
is_group_leader (const CHAR_DATA* ch)
{
  CHAR_DATA *tch = NULL;

  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
      if (tch->following == ch)
	return 1;
    }

  return 0;
}


void do_follow (CHAR_DATA * ch, char *argument, int cmd);
void do_group (CHAR_DATA * ch, char *argument, int cmd);
int do_group_size (CHAR_DATA * ch);
int mount_group_size (CHAR_DATA *ch);
char * group_makeup (CHAR_DATA * ch);
bool is_with_group (CHAR_DATA * ch);
void stop_followers (CHAR_DATA * ch);
void followers_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time);
void follower_catchup (CHAR_DATA * ch);
int num_followers (CHAR_DATA * ch);
extern QE_DATA *quarter_event_list;
#endif // _rpie_group_h_
