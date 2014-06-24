/*------------------------------------------------------------------------\
|  edit.c : Visual Editing Module                     www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/telnet.h>

#include "utils.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"

#define MAP_MAX_DIMMENSION			20
#define MAP_MAX_LINE_DIM(c)			((c)->max_lines / 4)
#define MAP_MAX_COL_DIM(c)			((c)->max_columns / 10)
#define MAP_MID_OFFSET_LINE(c)		(MAP_MAX_LINE_DIM (c) / 2)
#define MAP_MID_OFFSET_COL(c)		(MAP_MAX_COL_DIM (c) / 2)
#define VE_ABS(a)					((a > 0) ? a : -a)

struct map_data
{
  int map[MAP_MAX_DIMMENSION][MAP_MAX_DIMMENSION];
  int ups[MAP_MAX_DIMMENSION][MAP_MAX_DIMMENSION];
  int downs[MAP_MAX_DIMMENSION][MAP_MAX_DIMMENSION];
};

void ve_delete_char (DESCRIPTOR_DATA * c, size_t backup);
void ve_display_map (DESCRIPTOR_DATA * c);
void ve_edit_move (DESCRIPTOR_DATA * c, int dir);
void ve_reconstruct (DESCRIPTOR_DATA * c);

void
send_out (DESCRIPTOR_DATA * c, char *out_string)
{
  write_to_q (out_string, &c->output);
}

void
ve_goto (DESCRIPTOR_DATA * c, int line, int col)
{
  char buf[MAX_STRING_LENGTH];

  sprintf (buf, "\033[%d;%dH", line, col);

  send_out (c, buf);
}

void
ve_position (DESCRIPTOR_DATA * c)
{
  char buf[MAX_STRING_LENGTH];

  ve_goto (c, 24, 60);

  sprintf (buf, "%s%s [%d,%d]      ",
	   c->lines->line[c->line + c->tos_line - 1] ? " " : "*",
	   IS_SET (c->edit_mode, MODE_COMMAND) ? "Cmd" : "Ins",
	   c->line, c->col);
  send_out (c, buf);

  ve_goto (c, c->line, c->col);
}

void
ve_repaint (DESCRIPTOR_DATA * c)
{
  int screen_line;
  char *line_ptr;

  for (screen_line = 1; screen_line < 23; screen_line++)
    {

      line_ptr = c->lines->line[c->tos_line - 1 + screen_line];

      if (!line_ptr)
	return;

      ve_goto (c, screen_line, 1);

      send_out (c, line_ptr);
    }
}

void
ve_refresh_screen (DESCRIPTOR_DATA * c)
{
  char buf[MAX_STRING_LENGTH];

  sprintf (buf, "%s", HOME CLEAR);
  send_out (c, buf);

  if (IS_SET (c->edit_mode, MODE_VISMAP))
    ve_display_map (c);
  else
    {
      ve_repaint (c);
      ve_position (c);
    }
}

void
ve_telnet_command (DESCRIPTOR_DATA * c, int subcmd, int action)
{
  char buf[4];

  buf[0] = IAC;
  buf[1] = subcmd;
  buf[2] = action;
  buf[3] = 0;

  send_out (c, buf);
}

void
ve_help_screen (DESCRIPTOR_DATA * c)
{
  char buf[MAX_STRING_LENGTH];

  sprintf (buf, "%s", HOME CLEAR);
  send_out (c, buf);

  ve_goto (c, 1, 1);
  send_out (c, "  Command Mode commands");
  ve_goto (c, 3, 1);
  send_out (c, "    @   - EXIT editor");
  ve_goto (c, 4, 1);
  send_out (c, "    i   - enter insert mode");
  ve_goto (c, 5, 1);
  send_out (c, "    d   - delete current line");
  ve_goto (c, 6, 1);
  send_out (c, "    j   - down");
  ve_goto (c, 7, 1);
  send_out (c, "    k   - up");
  ve_goto (c, 8, 1);
  send_out (c, "    l   - right");
  ve_goto (c, 9, 1);
  send_out (c, "    h   - left");
  ve_goto (c, 10, 1);
  send_out (c, "    $   - goto end of line");
  ve_goto (c, 11, 1);
  send_out (c, "    o   - insert line below");
  ve_goto (c, 12, 1);
  send_out (c, "    O   - insert line above");
  ve_goto (c, 13, 1);
  send_out (c, "    x   - delete character");
  ve_goto (c, 14, 1);
  send_out (c, "    m   - display map");
  ve_goto (c, 15, 1);
  send_out (c, "    <ctrl-L>    - refresh screen");
  ve_goto (c, 16, 1);
  send_out (c, "    N,S,E,W,U,D - move");

  ve_goto (c, 1, 40);
  send_out (c, "Insert Mode");
  ve_goto (c, 3, 40);
  send_out (c, "  <ctrl-h>  - backspace");
  ve_goto (c, 4, 40);
  send_out (c, "  <esc>     - return to Command Mode");

  ve_goto (c, 8, 40);
  send_out (c, "Map Mode");
  ve_goto (c, 10, 40);
  send_out (c, "  N,S,E,W,U,D - move");
  ve_goto (c, 11, 40);
  send_out (c, "  @           - return to Command Mode");

  c->edit_mode |= MODE_VISHELP;
}

void
ve_deconstruct (DESCRIPTOR_DATA * c)
{
  char *p;
  char *text;
  char tmp_c;
  int line_no = 1;

  text = *c->str;

  if (!text)
    return;

  while (*text)
    {

      p = text;

      while (*text && *text != '\n')
	text++;

      tmp_c = *text;
      *text = '\0';

      c->lines->line[line_no] = str_dup (p);

      *text = tmp_c;

      if (*text)		/* Last character is \n */
	text++;

      line_no++;
    }
}

void
ve_setup_screen (DESCRIPTOR_DATA * c)
{
  make_quiet (c->character);

  if (c->lines)
    {
      printf ("c->lines is non-NULL!\n");
      mem_free (c->lines);
    }

  c->lines = (LINE_DATA *) alloc (sizeof (LINE_DATA), 8);

  c->line = 1;
  c->col = 1;
  c->tos_line = 1;

  ve_deconstruct (c);

  if (IS_SET (c->edit_mode, MODE_DONE_EDITING))
    {

      c->edit_mode |= MODE_COMMAND;
      c->edit_mode &= ~MODE_DONE_EDITING;

      ve_telnet_command (c, WILL, TELOPT_SGA);
      ve_telnet_command (c, DONT, TELOPT_LINEMODE);
      ve_telnet_command (c, DO, TELOPT_ECHO);
      ve_telnet_command (c, WILL, TELOPT_ECHO);
    }

  ve_refresh_screen (c);
}

void
ve_beep (DESCRIPTOR_DATA * c)
{
  char *buf = "\07";

  send_out (c, buf);
}

void
ve_down (DESCRIPTOR_DATA * c)
{
  if (c->line >= 22)
    {
      c->tos_line++;
      ve_refresh_screen (c);
    }
  else
    c->line++;

  ve_position (c);
}

void
ve_up (DESCRIPTOR_DATA * c)
{
  if (c->line > 1)
    c->line--;
  else if (c->tos_line > 1)
    {
      c->tos_line--;
      ve_refresh_screen (c);
    }
  else
    ve_beep (c);

  ve_position (c);
}

void
ve_left (DESCRIPTOR_DATA * c)
{
  if (c->col > 1)
    c->col--;
  else
    ve_beep (c);

  ve_position (c);
}

void
ve_right (DESCRIPTOR_DATA * c)
{
  if (c->col < 80)
    c->col++;
  else
    ve_beep (c);

  ve_position (c);
}

void
ve_goto_eol (DESCRIPTOR_DATA * c)
{
  int lines_offset;
  char *ptr;

  lines_offset = c->tos_line + c->line - 1;

  ptr = c->lines->line[lines_offset];

  c->col = ptr ? strlen (ptr) + 1 : 1;

  ve_position (c);
}

void
ve_delete_line (DESCRIPTOR_DATA * c, int lines_offset)
{
  if (!c->lines->line[lines_offset])
    {
      ve_beep (c);
      return;
    }

  mem_free (c->lines->line[lines_offset]);
  c->lines->line[lines_offset] = NULL;

  for (; c->lines->line[lines_offset + 1]; lines_offset++)
    c->lines->line[lines_offset] = c->lines->line[lines_offset + 1];

  c->lines->line[lines_offset] = NULL;

  c->col = 1;

  ve_refresh_screen (c);
}

void
ve_insert_line (DESCRIPTOR_DATA * c, int lines_offset)
{
  int i;
  char *ptr;
  char *ptr2;

  for (i = 1; i < lines_offset; i++)
    if (!c->lines->line[i])
      c->lines->line[i] = str_dup ("");

  ptr = c->lines->line[lines_offset];
  c->lines->line[lines_offset] = str_dup ("");

  for (i = lines_offset + 1; ptr; i++)
    {
      ptr2 = c->lines->line[i];
      c->lines->line[i] = ptr;
      ptr = ptr2;
    }
}

void
ve_add_new_line (DESCRIPTOR_DATA * c, char up_down)
{
  if (up_down == 'o')
    c->line++;

  ve_insert_line (c, c->line + c->tos_line - 1);

  c->col = 1;

  ve_refresh_screen (c);

  c->edit_mode &= ~MODE_COMMAND;
}

void
ve_command_mode (DESCRIPTOR_DATA * c, char **buf)
{

  if (IS_SET (c->edit_mode, MODE_VISHELP))
    {
      c->edit_mode &= ~MODE_VISHELP;
      ve_refresh_screen (c);
    }

  else if (IS_SET (c->edit_mode, MODE_VISMAP))
    {

      if (**buf == '@')
	{
	  c->edit_mode &= ~MODE_VISMAP;
	  ve_refresh_screen (c);
	}

      else if (**buf == '?')
	ve_help_screen (c);

      else if (**buf == 'N')
	ve_edit_move (c, NORTH);

      else if (**buf == 'S')
	ve_edit_move (c, SOUTH);

      else if (**buf == 'E')
	ve_edit_move (c, EAST);

      else if (**buf == 'W')
	ve_edit_move (c, WEST);

      else if (**buf == 'U')
	ve_edit_move (c, UP);

      else if (**buf == 'D')
	ve_edit_move (c, DOWN);

      else
	ve_beep (c);
    }

  else if (**buf == 'j')
    ve_down (c);

  else if (**buf == 'k')
    ve_up (c);

  else if (**buf == 'h')
    ve_left (c);

  else if (**buf == 'l')
    ve_right (c);

  else if (**buf == 'i')
    {
      c->edit_mode &= ~MODE_COMMAND;
      ve_position (c);
    }

  else if (**buf == 'x')
    ve_delete_char (c, 0);

  else if (**buf == 'd')
    ve_delete_line (c, c->tos_line + c->line - 1);

  else if (**buf == 'o' || **buf == 'O')
    ve_add_new_line (c, **buf);

  else if (**buf == '@')
    c->edit_mode |= MODE_DONE_EDITING;

  else if (**buf == 'm')
    {
      c->edit_mode |= MODE_VISMAP;
      ve_refresh_screen (c);
    }

  else if (**buf == '?')
    ve_help_screen (c);

  else if (**buf == '\014')
    ve_refresh_screen (c);

  else if (**buf == 'N')
    ve_edit_move (c, NORTH);

  else if (**buf == 'S')
    ve_edit_move (c, SOUTH);

  else if (**buf == 'E')
    ve_edit_move (c, EAST);

  else if (**buf == 'W')
    ve_edit_move (c, WEST);

  else if (**buf == 'U')
    ve_edit_move (c, UP);

  else if (**buf == 'D')
    ve_edit_move (c, DOWN);

  else if (**buf == '$')
    ve_goto_eol (c);

  else if (**buf == '\033')
    ve_beep (c);

  else
    ve_beep (c);

  (*buf)++;
}

void
ve_edit_move (DESCRIPTOR_DATA * c, int dir)
{
  ROOM_DATA *room = NULL;

  /* Can only move around when edit object is a room */

  if (c->proc)
    {
      ve_beep (c);
      return;
    }

  if (EXIT (c->character, dir))
    room = vnum_to_room (EXIT (c->character, dir)->to_room);

  if (!room)
    {
      ve_beep (c);
      return;
    }

  printf ("Room %d -> Room %d\n", c->character->in_room, room->vnum);

  char_from_room (c->character);
  char_to_room (c->character, room->vnum);

  ve_reconstruct (c);

  c->str = &room->description;

  ve_setup_screen (c);
}

void
ve_insert_char (DESCRIPTOR_DATA * c, char add_char)
{
  char buf1[MAX_STRING_LENGTH];
  char *line_ptr;
  size_t insert_index;
  size_t i;
  int lines_offset;

  lines_offset = c->tos_line + c->line - 1;

  line_ptr = c->lines->line[lines_offset];

  insert_index = c->col - 1;

  /* Oh gosh...we need to space this out. */

  if (!line_ptr)
    {
      line_ptr = (char *) alloc (insert_index + 2, 9);	/* +1 for new ch; +1 for zero */

      for (i = 0; i < insert_index; i++)
	line_ptr[i] = ' ';

      line_ptr[insert_index] = '\0';
    }

  /* Maybe we need to add a few blank lines above us */

  for (int i = 1; i < lines_offset; i++) /* This may be a little wasteful */
    if (!c->lines->line[i])
      c->lines->line[i] = str_dup ("");

  if (strlen (line_ptr) >= 80)
    {
      ve_beep (c);
      return;
    }

  /* Grab part up to the insert */

  *buf1 = 0;

  if (strlen (line_ptr) < insert_index)
    {
      memcpy (buf1, line_ptr, strlen (line_ptr));
      for (i = strlen (line_ptr); i < insert_index; i++)
	buf1[i] = ' ';
    }
  else if (insert_index)
    memcpy (buf1, line_ptr, insert_index);

  buf1[insert_index] = add_char;	/* Add inserted character */
  buf1[insert_index + 1] = '\0';

  if (strlen (line_ptr) > insert_index)
    strcat (buf1, &line_ptr[insert_index]);

  c->lines->line[lines_offset] = str_dup (buf1);

  mem_free (line_ptr);

  ve_goto (c, c->line, 1);

  send_out (c, c->lines->line[lines_offset]);

  c->col++;

  ve_position (c);
}

void
ve_delete_char (DESCRIPTOR_DATA * c, size_t backup)
{
  int lines_offset;
  int i;
  char *line_ptr;
  char buf[MAX_STRING_LENGTH];

  if (c->col <= 1)
    {				/* Backing up too far */
      ve_beep (c);
      return;
    }

  lines_offset = c->tos_line + c->line - 1;

  line_ptr = c->lines->line[lines_offset];

  if (!line_ptr)
    {
      c->col = 1;
      ve_position (c);
      return;
    }

  if (c->col - backup > strlen (line_ptr))
    {				/* Beyond end of line */
      c->col = strlen (line_ptr) + 1;
      if (!backup)
	c->col--;
      ve_position (c);
      return;
    }

  strcpy (buf, line_ptr);

  c->col -= backup;

  for (i = c->col - 1; buf[i]; i++)
    buf[i] = buf[i + 1];

  mem_free (line_ptr);

  c->lines->line[lines_offset] = str_dup (buf);

  ve_goto (c, c->line, 1);

  send_out (c, c->lines->line[lines_offset]);

  ve_goto (c, c->line, strlen (buf) + 1);
  send_out (c, " ");

  ve_position (c);
}

void
ve_newline (DESCRIPTOR_DATA * c)
{
  int lines_offset;
  int i;
  char buf[MAX_STRING_LENGTH];
  char *line_ptr;

  lines_offset = c->line + c->tos_line - 1;

  line_ptr = c->lines->line[lines_offset];

  if (!line_ptr)
    {

      c->col = 1;

      for (i = 1; i <= lines_offset + 1; i++)
	if (!c->lines->line[i])
	  c->lines->line[i] = str_dup ("");

      if (c->line > 22)
	{
	  c->tos_line++;
	  ve_refresh_screen (c);
	}
      else
	c->line++;

      ve_position (c);

      return;
    }

  *buf = '\0';

  /* User pressed return, break line in two */

  if (c->col <= strlen (line_ptr))
    {

      /* The memcpy copies a zero as well. */

      memcpy (buf, &line_ptr[c->col - 1], strlen (&line_ptr[c->col - 1]) + 1);
      line_ptr[c->col - 1] = '\0';
    }

  if (c->line > 22)
    c->tos_line++;
  else
    c->line++;

  c->col = 1;

  lines_offset++;

  ve_insert_line (c, lines_offset);

  mem_free (c->lines->line[lines_offset]);

  c->lines->line[lines_offset] = str_dup (buf);

  ve_refresh_screen (c);
}

void
ve_insert_mode (DESCRIPTOR_DATA * c, char **buf)
{
  if (**buf == '\033')
    {
      c->edit_mode |= MODE_COMMAND;

      if (c->col > 1)
	c->col--;

      ve_position (c);
    }

  else if (**buf == '\010')
    ve_delete_char (c, 1);

  else if (**buf == '\r')
    ve_newline (c);

  else if (!isprint (**buf))
    ve_beep (c);

  else
    ve_insert_char (c, **buf);

  (*buf)++;
}

void
ve_deallocate (DESCRIPTOR_DATA * c)
{
  int i;

  for (i = 1; c->lines->line[i]; i++)
    mem_free (c->lines->line[i]);

  mem_free (c->lines);

  c->lines = NULL;
}

void
ve_reconstruct (DESCRIPTOR_DATA * c)
{
  int i;
  int str_len = 1;		/* Counts the zero. */

  for (i = 1; c->lines->line[i]; i++)
    str_len += strlen (c->lines->line[i]) + 1;	/* +newline */

  if (*c->str)
    mem_free (*c->str);

  *c->str = (char *) alloc (str_len, 10);

  **c->str = '\0';

  for (i = 1; c->lines->line[i]; i++)
    {
      strcat (*c->str, c->lines->line[i]);
      if (c->lines->line[i + 1] ||
	  c->lines->line[i][strlen (c->lines->line[i])] != '\n')
	strcat (*c->str, "\n");
    }

  ve_deallocate (c);

  c->str = NULL;
}

void
ve_process (DESCRIPTOR_DATA * c, char *buf)
{
  while (*buf)
    {

      if (IS_SET (c->edit_mode, MODE_COMMAND))
	ve_command_mode (c, &buf);

      else
	ve_insert_mode (c, &buf);

      if (IS_SET (c->edit_mode, MODE_DONE_EDITING))
	{
	  ve_reconstruct (c);

	  ve_goto (c, 24, 1);

	  ve_telnet_command (c, DONT, TELOPT_SGA);

	  ve_telnet_command (c, DO, TELOPT_LINEMODE);
	  ve_telnet_command (c, WILL, TELOPT_LINEMODE);

	  /* linemode sub-negotiation - local edit mode */

	  ve_telnet_command (c, SB, TELOPT_LINEMODE);

	  buf[0] = LM_MODE;
	  buf[1] = MODE_EDIT;
	  buf[2] = IAC;
	  buf[3] = SE;		/* Stop subnegotion */
	  buf[4] = 0;

	  send_out (c, buf);

	  ve_telnet_command (c, WONT, TELOPT_ECHO);

	  c->character->act &= ~PLR_QUIET;

	  send_to_char ("Press return...your first command won't work\n\r",
			c->character);

	  if (c->proc)
	    (c->proc) (c);
	  else
	    printf ("NO PROC WAS SET.\n");

	  return;
	}
    }
}

int
ve_offset_to_dir (int delta_x, int delta_y)
{
  if (delta_x == -1 && delta_y == 0)
    return NORTH;

  if (delta_x == 0 && delta_y == -1)
    return WEST;

  if (delta_x == 0 && delta_y == 1)
    return EAST;

  if (delta_x == 1 && delta_y == 0)
    return SOUTH;

  return -1;			/* Couldn't convert offsets into a direction */
}

void
ve_map_map (DESCRIPTOR_DATA * c, struct map_data *map_ptr, int x, int y,
	    int *found)
{
  int i;
  int j;
  int dir;
  ROOM_DATA *room;

  if (!map_ptr->map[x][y])
    return;

  if (!(room = vnum_to_room (map_ptr->map[x][y])))
    return;

  if (room->dir_option[UP])
    map_ptr->ups[x][y] = room->dir_option[UP]->to_room;

  if (room->dir_option[DOWN])
    map_ptr->downs[x][y] = room->dir_option[DOWN]->to_room;

  for (i = -1; i <= 1; i++)
    {
      for (j = -1; j <= 1; j++)
	{

	  if (VE_ABS (x + i) >= MAP_MAX_LINE_DIM (c) ||
	      VE_ABS (y + j) >= MAP_MAX_COL_DIM (c) || x + i < 0 || y + j < 0)
	    continue;

	  /* Skip if we've caught room on another pass */

	  if (map_ptr->map[x + i][y + j])
	    continue;

	  if ((dir = ve_offset_to_dir (i, j)) == -1)
	    continue;

	  if (room->dir_option[dir])
	    {
	      map_ptr->map[x + i][y + j] = room->dir_option[dir]->to_room;
	      *found = 1;
	    }
	}
    }
}

void
ve_paint_map (DESCRIPTOR_DATA * c, struct map_data *map_ptr)
{
  int i;
  int j;
  int t1;
  int t2;
  int duplicate_room;
  int same_room;
  int con_to;			/* Connects to another room */
  int con_to_good;		/* Connects to expected room */
  int con_from;			/* Connects from another room */
  int con_from_good;		/* Connects from expected room */
  char buf[MAX_STRING_LENGTH];
  ROOM_DATA *room;		/* From room */
  ROOM_DATA *room2;		/* To room */

  sprintf (buf, "%s", HOME CLEAR);
  send_out (c, buf);

  for (i = 0; i < MAP_MAX_LINE_DIM (c); i++)
    {
      for (j = 0; j < MAP_MAX_COL_DIM (c); j++)
	{

	  if (map_ptr->map[i][j] <= 0)
	    continue;

	  ve_goto (c, i * 4 + 1, j * 10 + 1);

	  duplicate_room = 0;

	  if (j == MAP_MID_OFFSET_COL (c) && i == MAP_MID_OFFSET_LINE (c))
	    sprintf (buf, "%s%5d%s", "#4", map_ptr->map[i][j], "#0");
	  else
	    {
	      for (t1 = 0; t1 < MAP_MAX_LINE_DIM (c) && !duplicate_room; t1++)
		for (t2 = 0; t2 < MAP_MAX_COL_DIM (c); t2++)
		  if ((t1 != i || t2 != j) &&
		      map_ptr->map[t1][t2] == map_ptr->map[i][j])
		    {
		      duplicate_room = 1;
		      break;
		    }

	      if (vnum_to_room (map_ptr->map[i][j]) &&
		  !strncmp (vnum_to_room (map_ptr->map[i][j])->description,
			    "No Description Set", 18))
		sprintf (buf, "%s%5d%s", "#2", map_ptr->map[i][j], "#0");
	      else if (duplicate_room)
		sprintf (buf, "%s%5d%s", "#1", map_ptr->map[i][j], "#0");
	      else
		sprintf (buf, "%5d", map_ptr->map[i][j]);
	    }

	  send_out (c, buf);

	  if (map_ptr->ups[i][j])
	    {
	      ve_goto (c, i * 4, j * 10 + 2);
	      send_out (c, "#3U#0");
	    }

	  if (map_ptr->downs[i][j])
	    {
	      ve_goto (c, i * 4 + 2, j * 10 + 2);
	      send_out (c, "#3D#0");
	    }

	  if (j + 1 < MAP_MAX_COL_DIM (c) && map_ptr->map[i][j + 1])
	    {

	      con_from = 0;
	      con_from_good = 0;

	      if (map_ptr->map[i][j] == map_ptr->map[i][j + 1])
		same_room = 1;
	      else
		same_room = 0;

	      room = vnum_to_room (map_ptr->map[i][j + 1]);
	      if (!room);
	      else if (!room->dir_option[WEST]);
	      else if (room->dir_option[WEST]->to_room == map_ptr->map[i][j])
		{
		  con_from_good = 1;
		  con_from = 1;
		}
	      else if (room->dir_option[WEST]->to_room > 0)
		con_from = 1;

	      con_to = 0;
	      con_to_good = 0;

	      room2 = vnum_to_room (map_ptr->map[i][j]);
	      if (!room2);
	      else if (!room2->dir_option[EAST]);
	      else if (room2->dir_option[EAST]->to_room ==
		       map_ptr->map[i][j + 1])
		{
		  con_to_good = 1;
		  con_to = 1;
		}
	      else if (room2->dir_option[EAST]->to_room > 0)
		con_to = 1;

	      *buf = '\0';

	      if (same_room)
		{
		  ve_goto (c, i * 4 + 1, j * 10 + 6);
		  send_out (c, "#1.....#0");
		}

	      else
		{
		  if (con_from_good && con_to_good)
		    strcpy (buf, "<===>");
		  else if (con_from_good)
		    strcpy (buf, "<----");
		  else if (con_to_good)
		    strcpy (buf, "---->");

		  if (*buf)
		    {
		      ve_goto (c, i * 4 + 1, j * 10 + 6);
		      send_out (c, buf);
		    }

		  if (con_to && !con_to_good)
		    {
		      ve_goto (c, i * 4 + 2, j * 10 + 5);
		      send_out (c, "#5>?#0");
		    }

		  if (con_from && !con_from_good)
		    {
		      ve_goto (c, i * 4 + 2, j * 10 + 10);
		      send_out (c, "#5?<#0");
		    }
		}
	    }

	  if (i + 1 < MAP_MAX_LINE_DIM (c) && map_ptr->map[i + 1][j])
	    {

	      con_from = 0;
	      con_from_good = 0;

	      if (map_ptr->map[i][j] == map_ptr->map[i + 1][j])
		same_room = 1;
	      else
		same_room = 0;

	      room = vnum_to_room (map_ptr->map[i + 1][j]);
	      if (!room);
	      else if (!room->dir_option[NORTH]);
	      else if (room->dir_option[NORTH]->to_room == map_ptr->map[i][j])
		{
		  con_from_good = 1;
		  con_from = 1;
		}
	      else if (room->dir_option[NORTH]->to_room > 0)
		con_from = 1;

	      con_to = 0;
	      con_to_good = 0;

	      room2 = vnum_to_room (map_ptr->map[i][j]);
	      if (!room2);
	      else if (!room2->dir_option[SOUTH]);
	      else if (room2->dir_option[SOUTH]->to_room ==
		       map_ptr->map[i + 1][j])
		{
		  con_to_good = 1;
		  con_to = 1;
		}
	      else if (room2->dir_option[SOUTH]->to_room > 0)
		con_to = 1;

	      *buf = '\0';

	      if (same_room)
		{
		  ve_goto (c, i * 4 + 2, j * 10 + 3);
		  send_out (c, "#1.#0");

		  ve_goto (c, i * 4 + 3, j * 10 + 3);
		  send_out (c, "#1.#0");

		  ve_goto (c, i * 4 + 4, j * 10 + 3);
		  send_out (c, "#1.#0");
		}

	      else
		{
		  if (con_from_good)
		    {
		      ve_goto (c, i * 4 + 2, j * 10 + 3);
		      send_out (c, "^");
		    }

		  if (con_from_good && con_to_good)
		    {
		      ve_goto (c, i * 4 + 3, j * 10 + 3);
		      send_out (c, "#");
		    }

		  else if (con_from_good || con_to_good)
		    {
		      ve_goto (c, i * 4 + 3, j * 10 + 3);
		      send_out (c, "|");
		    }

		  if (con_to_good)
		    {
		      ve_goto (c, i * 4 + 4, j * 10 + 3);
		      send_out (c, "v");
		    }

		  if (con_to && !con_to_good)
		    {
		      ve_goto (c, i * 4 + 2, j * 10 + 2);
		      send_out (c, "#5v#0");
		      ve_goto (c, i * 4 + 3, j * 10 + 2);
		      send_out (c, "#5?#0");
		    }

		  if (con_from && !con_from_good)
		    {
		      ve_goto (c, i * 4 + 4, j * 10 + 4);
		      send_out (c, "#5^#0");
		      ve_goto (c, i * 4 + 3, j * 10 + 4);
		      send_out (c, "#5?#0");
		    }
		}
	    }
	}
    }

  ve_goto (c, c->max_lines, 1);

  sprintf (buf, "[%5d]: %s%s%s", c->character->in_room, "#5",
	   c->character->room->name, "#0");
  send_out (c, buf);
}

void
ve_display_map (DESCRIPTOR_DATA * c)
{
  int i;
  int j;
  int dist;
  struct map_data map_info;
  int found = 1;
  int try_count = 0;

  if (c->max_lines == 0)
    c->max_lines = 24;

  if (c->max_columns == 0)
    c->max_columns = 80;

  for (i = 0; i < MAP_MAX_DIMMENSION; i++)
    {
      for (j = 0; j < MAP_MAX_DIMMENSION; j++)
	{
	  map_info.map[i][j] = 0;
	  map_info.ups[i][j] = 0;
	  map_info.downs[i][j] = 0;
	}
    }

  map_info.map[MAP_MID_OFFSET_LINE (c)][MAP_MID_OFFSET_COL (c)] =
    c->character->in_room;

  while (found && try_count++ < 20)
    {

      found = 0;

      for (dist = 0; dist <= MAP_MID_OFFSET_LINE (c); dist++)
	for (i = -dist; i <= dist; i++)
	  for (j = -dist; j <= dist; j++)
	    if (VE_ABS (i) == dist || VE_ABS (j) == dist)
	      {

		if (VE_ABS (i) > MAP_MID_OFFSET_LINE (c))
		  continue;

		if (VE_ABS (j) > MAP_MID_OFFSET_COL (c))
		  continue;

		ve_map_map (c, &map_info, MAP_MID_OFFSET_LINE (c) + i,
			    MAP_MID_OFFSET_COL (c) + j, &found);
	      }
    }

  ve_paint_map (c, &map_info);
}
