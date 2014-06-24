/*------------------------------------------------------------------------\
|  map.c : Dynamic Graphical Map Generator  	      www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
//#include "gd.h"


/* 

Something to come back to and finish, eventually ...

void
refresh_map (void)
{
  ROOM_DATA *root_room, *curr_room, *troom;
  gdImagePtr map;
  FILE *fp;
  int black;
  int white;
  int x = 0;
  int y = 0;
  int dir = 0;

  map = gdImageCreate (1000, 1000);

  black = gdImageColorAllocate (map, 0, 0, 0);
  white = gdImageColorAllocate (map, 255, 255, 255);

  root_room = vtor (4001);

  gdImageSetPixel (map, x, y, white);

  for (troom = full_room_list; troom; troom = troom->lnext)
    {
      if (troom->zone != 4 || troom->virtual == 4001)
	continue;
      curr_room = vtor (4001);
      y = 500;
      x = 500;
      while ((dir = track_room (curr_room, troom->virtual)) != -1)
	{
	  if (dir == NORTH)
	    y++;
	  else if (dir == SOUTH)
	    y--;
	  else if (dir == WEST)
	    x--;
	  else if (dir == EAST)
	    x++;
	  curr_room = vtor (curr_room->dir_option[dir]->to_room);
	}
      gdImageSetPixel (map, x, y, white);
    }

  fp = fopen (PATH_TO_WEBSITE "/test.jpg", "wb");

  gdImageJpeg (map, fp, -1);

  fclose (fp);

  gdImageDestroy (map);
}

*/
