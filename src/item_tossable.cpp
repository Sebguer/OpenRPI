/*------------------------------------------------------------------------\
|  item_tossable.c : routines for dice items          www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"



/* oval 0 - range (number of possible values, 2, 4, 6 generally)         */
/* oval 1 - skill bonus (for loaded dice 'calling')                      */
/* mkeys  - list of text values (side faces default 'one'..'six'         */


/* toss $dice */
/* toss $dice $table */
/* toss $dice for $face */
/* toss $dice $table for $face */
/* toss $n $dice */
/* toss $n $dice $table */
/* toss $n $dice for $face */
/* toss $n $dice $table for $face */
void
char__do_toss (CHAR_DATA * ch, char *argument, int cmd)
{
  register short int nArg = 0;
  register short int nDiceArg = 0;
  register short int nTableArg = 0;
  register short int nFaceArg = 0;
  register short int nFacet = 0;
  register short int nMaxFacet = 0;
  size_t nIndex = 0;
  register int nCount = 0;

  char *p = NULL;
  char buf[AVG_STRING_LENGTH * 10] = "";
  char arg[5][AVG_STRING_LENGTH / 5] = { "", "", "", "", "" };
  char key[AVG_STRING_LENGTH] = "";
  char strFacet[AVG_STRING_LENGTH] = "";
  char strFacetList[AVG_STRING_LENGTH] = "";
  OBJ_DATA *dice = NULL;
  OBJ_DATA *table = NULL;

  nArg =
    sscanf (argument, "%s %s %s %s %s", arg[0], arg[1], arg[2], arg[3],
	    arg[4]);

  if (nArg <= 0)
    {
      send_to_char ("Toss what?\n", ch);
      return;
    }

  nCount = strtol (arg[0], &p, 10);
  if (errno == ERANGE || nCount == 0 || strlen (p) != 0)
    {
      nCount = 0;
    }
  else
    {
      nDiceArg = 1;
    }

  switch (nArg - nDiceArg)
    {
    case 3:
      nFaceArg = (strcmp ("for", arg[nDiceArg + 1]) == 0) ? nDiceArg + 2 : -1;
      break;
    case 4:
      nFaceArg = (strcmp ("for", arg[nDiceArg + 2]) == 0) ? nDiceArg + 3 : -1;	/* NO BREAK HERE goes to next case */
    case 2:
      nTableArg = nDiceArg + 1;
      break;
    default:
      break;
    }

  if (nFaceArg < 0)
    {
      send_to_char ("Toss them how?\n", ch);
      return;
    }

  if (!(dice = get_obj_in_dark (ch, arg[nDiceArg], ch->right_hand)) &&
      !(dice = get_obj_in_dark (ch, arg[nDiceArg], ch->left_hand)))
    {
      sprintf (buf, "You don't have a '%s'.\n", arg[nDiceArg]);
      send_to_char (buf, ch);
      return;
    }
  if (dice->obj_flags.type_flag != ITEM_TOSSABLE
      && dice->obj_flags.type_flag != ITEM_MONEY)
    {
      send_to_char ("Did you mean to #6throw#0 that object?\n", ch);
      return;
    }

  if (dice->count > 12 && (nCount == 0 || nCount > 12))
    {
      send_to_char ("You can't toss that many at once.\n", ch);
      return;
    }

  if (nTableArg && arg[nTableArg][0] &&
      (!(table = get_obj_in_list_vis (ch, arg[nTableArg], ch->room->contents))
       || !IS_TABLE(table)))
    {
      sprintf (buf, "You don't see any furniture like '%s'.\n",
	       arg[nTableArg]);
      send_to_char (buf, ch);
      return;
    }

  obj_from_char (&dice, nCount);

  if (dice->obj_flags.type_flag == ITEM_MONEY)
    {
      strcpy (key, " obverse reverse ");
      nMaxFacet = 2;
    }
  else if (dice->desc_keys)
    {
      sprintf (key, " %s ", dice->desc_keys);
    }
  else
    {
      strcpy (key,
	      " one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty ");
    }

  if (nFaceArg && dice->o.od.value[1])
    {
      /*
         nFacet = strtol(arg[nFaceArg],&p,10);
         if ( errno == ERANGE || nFacet == 0 || strlen(p) != 0 ) {
         strcpy(strFacet,strstr(key,arg[nFaceArg]));
         if () {
         }
         else {
         nFacet = 0;
         }
         }
         else {
         }
       */

    }
  nMaxFacet = (nMaxFacet) ? nMaxFacet : dice->o.od.value[0];
  for (nCount = 0; nCount < dice->count; nCount++)
    {

      nFacet = number (1, nMaxFacet);

      for (nIndex = 0; nIndex < strlen (key); nIndex++)
	{
	  if (key[nIndex] == ' ' && --nFacet <= 0)
	    break;
	}

      sscanf (key + nIndex + 1, "%s %s", strFacet, buf);
      sprintf (strFacetList + strlen (strFacetList), "#6%s%s#0%s",
	       (nMaxFacet == 2) ? "the " : "a ", strFacet,
	       (nCount ==
		dice->count - 2) ? " and " : ((nCount !=
					       dice->count - 1) ? ", " : ""));
    }

  sprintf (buf, "You toss $p onto %s%s%s%s. The upright face%s show%s: %s.\n",
	   (nTableArg) ? "$P" : "the ground",
	   (nFaceArg) ? ", trying for #6a " : "",
	   (nFaceArg) ? arg[nFaceArg] : "", (nFaceArg) ? "#0" : "",
	   (nCount > 1) ? "s" : "", (nCount > 1) ? "" : "s", strFacetList);
  act (buf, true, ch, dice, table, TO_CHAR | _ACT_FORMAT);

  sprintf (buf, "$n tosses $p onto %s. The upright face%s show%s: %s.\n",
	   (nTableArg) ? "$P" : "the ground", (nCount > 1) ? "s" : "",
	   (nCount > 1) ? "" : "s", strFacetList);
  act (buf, true, ch, dice, table, TO_ROOM | _ACT_FORMAT);

  if (nTableArg)
    {
      obj_to_obj (dice, table);
    }
  else
    {
      obj_to_room (dice, ch->in_room);
    }
}
