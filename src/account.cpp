#include <ctype.h>
#include <stdlib.h>

//test
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "net_link.h"

account::account (const char *acct_name)
{
  int row_count = 0;
  MYSQL_RES *result = NULL;
  MYSQL_ROW row;
  char error_message[ERR_STRING_LENGTH] = "\0";

  initialize ();

  if (!acct_name || !*acct_name)
    {
      return;
    }

  if (str_cmp (acct_name, "Anonymous") == 0)
    {
      return;
    }

  mysql_safe_query ("SELECT "
		    "roleplay_points, user_password, user_last_ip, "
		    "user_subscription, user_regdate, user_email,"
		    "user_color, account_flags, user_posts, "
		    "user_timezone, IFNULL(downloaded_code,0) as downloaded_code, last_rpp, "
		    "user_id, username, last_nominate, user_sound, user_guide FROM forum_users WHERE username = '%s'",
		    acct_name);


  if ((result = mysql_store_result (database)) != NULL)
    {
      if ((row_count = mysql_num_rows (result)) == 1)
	{
	  row = mysql_fetch_row (result);

	  id = strtol (row[12], 0, 10);
	  set_name (row[13]);
	  roleplay_points = strtol (row[0], 0, 10);
	  set_password (row[1]);
	  set_last_ip (row[2]);
	  newsletter = strtol (row[3], 0, 10);
	  created_on = strtol (row[4], 0, 10);
	  set_email (row[5]);
	  color = strtol (row[6], 0, 10);
	  flags = strtol (row[7], 0, 10);
	  forum_posts = strtol (row[8], 0, 10);
	  timezone = strtof (row[9], 0);
	  code = strtol (row[10], 0, 10);
	  last_rpp = strtol (row[11], 0, 10);
	  last_nominate = strtol (row[14], 0, 10);
	  sound = strtol (row[15], 0, 10);
	  guide = strtol (row[16], 0, 10);

	}
      else if (row_count > 1)
	{
	  snprintf (error_message, ERR_STRING_LENGTH,
		    "Warning: account__load: Found %d matches to account name %s",
		    row_count, acct_name);
	}
      else
	{
	  snprintf (error_message, ERR_STRING_LENGTH,
		    "Warning: account__load: Found %d matches to account name %s",
		    row_count, acct_name);
	}

      mysql_free_result (result);
    }
  else
    {
      snprintf (error_message, ERR_STRING_LENGTH,
		"Warning: account__load: %s", mysql_error (database));
    }


  if (*error_message)
    {
      system_log (error_message, true);
    }
}


