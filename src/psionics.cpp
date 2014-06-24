#include "structs.h"
#include "protos.h"
#include "utility.h"
#include "utils.h"


void
do_hex (CHAR_DATA * ch, char *argument, int cmd)
{
  int sn;
  int duration, modifier;
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];

  if (!real_skill (ch, SKILL_VOODOO))
    {
      send_to_char ("You shiver at the thought.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(tch = get_char_room_vis (ch, buf)))
    {
      send_to_char ("You don't see that person here.\n\r", ch);
      return;
    }

  if (IS_MORTAL (ch) && !IS_MORTAL (tch))
    {
      send_to_char
	("Immortals are total losers.  It can't get any worse for them.\n\r",
	 ch);
      return;
    }

  if (GET_HIT (ch) + GET_MOVE (ch) <= 35)
    {
      send_to_char
	("You can't concentrate hard enough for that right now.\n\r", ch);
      return;
    }

  sprintf (buf, "Hexing %s", tch->tname);
  weaken (ch, 0, 20, buf);
  sense_activity (ch, SKILL_VOODOO);

  if (!skill_use (ch, SKILL_VOODOO, 0))
    {
      send_to_char
	("You lose your concentration, and your malignant energies dissipate.\n",
	 ch);
      return;
    }

  act
    ("You channel a stream of malignant psychic energy into $N, entwining $M in an ethereal web of ill-fortune and grief.",
     false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
  act ("A chill runs down your spine.", false, ch, 0, tch, TO_VICT);

  sn = spell_lookup ("curse");

  modifier = ch->skills[SKILL_VOODOO] / 5 + number (1, 10);
  duration = ch->skills[SKILL_VOODOO] / 5 + number (1, 48);

  magic_add_affect (tch, MAGIC_AFFECT_CURSE, duration, modifier, 0, 0, sn);

  tch->curse += ch->skills[SKILL_VOODOO] / 5 + number (1, 5);
}
