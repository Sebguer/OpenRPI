/*------------------------------------------------------------------------\
|  item_card.c : routines for card items              www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"



/* oval 0 - 0 for deck obj, deck vnum for card object                    */
/* mkeys  - list of text values (side faces default 'one'..'six'         */

void
char__do_cards (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[AVG_STRING_LENGTH] = "";
  OBJ_DATA *card = NULL;

  if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_CARD)
    {
      send_to_char ("<carried in right hand>  ", ch);
      show_obj_to_char (ch->right_hand, ch, 1);
      for (card = ch->right_hand; card != NULL; card = card->next_content)
	{
	  strcpy (buf, "    #2");
	  strcat (buf, card->short_description);
	  strcat (buf, "#0\n");
	  send_to_char (buf, ch);
	}
    }
  if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_CARD)
    {
      send_to_char ("<carried in left hand>  ", ch);
      show_obj_to_char (ch->left_hand, ch, 1);
      for (card = ch->left_hand; card != NULL; card = card->next_content)
	{
	  strcpy (buf, "    #2");
	  strcat (buf, obj_short_desc (card));
	  strcat (buf, "#0\n");
	  send_to_char (buf, ch);
	}
    }

}
