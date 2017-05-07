/*------------------------------------------------------------------------\
|  nanny.c : Login Menu and Chargen Module            www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include <mysql/mysql.h>
#include <time.h>
#include <sstream>

#include "server.h"
#include "net_link.h"
#include "structs.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "utility.h"

extern rpie::server engine;

char echo_off_str[] = { (char) IAC, (char) WILL, (char) TELOPT_ECHO, '\0' };
char echo_on_str[] = { (char) IAC, (char) WONT, (char) TELOPT_ECHO,
                       '\r', '\n', '\0'
                     };

#define ECHO_OFF		SEND_TO_Q (echo_off_str, d);
#define ECHO_ON			SEND_TO_Q (echo_on_str, d);

#define MAX_PC_LIMIT		100

int new_accounts = 0;

/* Check for duplicate passwords
select aa.username, aa.user_password
from forum_users aa, forum_users bb
where aa.username != bb.username AND aa.user_password = bb.user_password
group by aa.username, aa.user_password
order by user_password, username;

Logins per day:
select account,ip,firsttime,lasttime, count,has_pwd,count/datediff(lasttime,firsttime) as lpd
from ip
order by  lpd desc
limit 60;
*/

const int pregame_furnishings[] =
{
    -1
};

/*
	0	Non-Restricted
	1	Pre-Requisite Skill
	2	Pre-Required Skill
	3	RPP Allowed Skill
	4	Language
	5 	Script
	6	Combat Skill
*/

const char *color_skill[] =
{
    "#0",				/* Unused */
    "#3",				/* Brawling */
    "#3",				/* Small-Blade */
    "#3",				/* Long-Blade */
    "#3",				/* Polearm */
    "#3",				/* Bludgeon */
    "#3",				/* Dodge */
    "#3",				/* Deflect */
    "#9#3",				/* Sole-Wield */
    "#9#3",				/* Dual-Wield */
    "#3",				/* Aim */
    "#3",				/* Handgun */
    "#3",				/* Rifle */
    "#3",				/* SMG */
    "#3",				/* Gunnery */
    "#3",				/* Explosives */

    "#9#6",			    /* Sneak */
    "#6",				/* Hide */
    "#9#6",			    /* Steal */
    "#9#6",			    /* Picklock */
    "#0",				/* Haggle */
    "#0",				/* Handle */
    "#0",				/* Hunting */
    "#6",	    		/* First-Aid */
    "#9#6",           // Medicine
    "#0",				/* Scavenge */
    "#0",				/* Eavesdrop */
    "#0",             /* Butchery */

    "#2",			    /* Chemistry */
    "#2", 			    /* Mechanics */
    "#2",				/* Gunsmithery */
    "#2",	    		/* Computerology */
    "#2",		    	/* Electronics */
    "#2",			    /* Biology */
    "#2",			    /* Weaponcraft */
    "#2",			    /* Armorcraft */
    "#2",			    /* Handicraft */
    "#9#2",		        /* Artistry */

    "#5",             /* Education */
    "#0",             /* Voodoo */
    "#0",             /* Common */
};




bool
is_banned (DESCRIPTOR_DATA * d)
{
    SITE_INFO *site = NULL;

    if (banned_site && banned_site->name && d && d->strClientHostname
            && d->strClientIpAddr)
    {

        for (site = banned_site; site; site = site->next)
        {

            if (!str_cmp (banned_site->name, "*"))
            {
                return true;
            }

            if (site->name[0] == '^')
            {

                if (strncmp
                        (d->strClientHostname, site->name + 1,
                         strlen (site->name) - 1) == 0)
                {
                    return true;
                }

                if (strncmp
                        (d->strClientIpAddr, site->name + 1,
                         strlen (site->name) - 1) == 0)
                {
                    return true;
                }

            }
            else
            {

                if (strstr (d->strClientHostname, site->name))
                {
                    return true;
                }

                if (strstr (d->strClientIpAddr, site->name))
                {
                    return true;
                }

            }

        }

    }
    return false;
}


/*                                                                          *
 * function: display_unread_messages                                        *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - changes mysql_use_result to mysql_store_result        *
 *                                                                          */
void
display_unread_messages (DESCRIPTOR_DATA * d)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];
    int unread = 0;

    if (!d->acct || d->acct->name.empty ())
        return;

    mysql_safe_query
    ("SELECT flags FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC",
     d->acct->name.c_str ());

    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: display_unread_messages(): %s",
                 mysql_error (database));
        system_log (buf, true);
        return;
    }

    while ((row = mysql_fetch_row (result)) != NULL)
    {
        if (!IS_SET (atoi (row[0]), MF_READ))
            unread++;
    }

    mysql_free_result (result);
    result = NULL;

    if (!unread)
        return;

    sprintf (buf,
             "#6There %s %d unread MUD-Mail%s awaiting your attention!#0\n\n",
             unread > 1 ? "are" : "is", unread, unread > 1 ? "s" : "");
    SEND_TO_Q (buf, d);
}
/**
void
display_login_delay (DESCRIPTOR_DATA * d)
{
  char buf[MAX_STRING_LENGTH];
  MYSQL_RES *result;
  MYSQL_ROW row;
  int tyme_passed = 0;

  if (!d->acct || d->acct->name.empty ())
    {
      return;
    }

  std::string player_db = engine.get_config ("player_db");


 mysql_safe_query
   ("SELECT lastlogoff FROM %s.pfiles WHERE account = '%s'", player_db.c_str (), d->acct->name.c_str ());


  if ((result = mysql_store_result (database)) == NULL)
    {
      sprintf (buf, "Warning: display_login_delay(): %s",
               mysql_error (database));
      system_log (buf, true);

      return;
    }

  row = mysql_fetch_row (result);

      tyme_passed = time (0) - atoi(row[0]);     // 21600 seconds in an in-game day

      if ( tyme_passed > 100 && tyme_passed < 21600 ) // more than 100 for people logging off
	{
          sprintf (buf, "Less than a day has passed in Middle-Earth since your last departure.\n");
	  SEND_TO_Q (buf, d);
	}
      else if (tyme_passed > 21599 && tyme_passed < 43200)
	{
          sprintf (buf, "A single day has passed in Middle-Earth since your last departure.\n" );
          SEND_TO_Q (buf, d);
	}
      else if (tyme_passed > 43199 && tyme_passed < 2160000)
	{
          tyme_passed = tyme_passed / 21600;
          sprintf (buf, "%d days have passed in Middle-Earth since your last departure.\n", tyme_passed);
          SEND_TO_Q (buf, d);
	}

  mysql_free_result (result);
  result = NULL;

}
***/
void
display_main_menu (DESCRIPTOR_DATA * d)
{
    read_motd(d);

    SEND_TO_Q (get_text_buffer (NULL, text_list, "menu1"), d);
    display_unread_messages (d);
    //display_login_delay (d);
    SEND_TO_Q ("Your Selection: ", d);
    d->connected = CON_ACCOUNT_MENU;
}

char *
encrypt_buf (const char *buf)
{
    //  extern char *crypt (const char *key, const char *salt);
    return str_dup (crypt (buf, "CR"));
}

int
check_password (const char *pass, const char *encrypted)
{
    char *p;
    int return_value;

    p = encrypt_buf (pass);

    return_value = (strcmp (p, encrypted) == 0);

    mem_free (p); // char* from crypt()

    return return_value;
}

void
nanny_login_choice (DESCRIPTOR_DATA * d, char *argument)
{
    bool bIsBanned = false;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, buf);

    while (*buf && buf[strlen (buf) - 1] == ' ')
        buf[strlen (buf) - 1] = '\0';

    *buf = toupper (*buf);

    bIsBanned = is_banned (d);

    if (bIsBanned && *buf != 'L')
    {
        sprintf (buf, "\nYour site is currently banned from connecting to %s.\n"
                 "As a result of the large number of people we get from certain Internet\n"
                 "domains, this may not be a result of anything you've done; if you feel that\n"
                 "this is the case, please email our staff at %s and we\n"
                 "will do our best to resolve this issue. Our apologies for the inconvenience.\n",
                 MUD_NAME, STAFF_EMAIL);
        SEND_TO_Q (buf, d);
        sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
        SEND_TO_Q (buf, d);
        d->connected = CON_PENDING_DISC;
        return;
    }

    if (*buf != 'C' && *buf != 'L' && *buf != 'X')
    {
        SEND_TO_Q ("That is not a valid option, friend.\n", d);
        SEND_TO_Q ("Your Selection: ", d);
        return;
    }

    else if (*buf == 'C')
    {
        if (!strstr (d->strClientHostname, "localhost")
                && reference_ip (NULL, d->strClientHostname))
        {
            SEND_TO_Q
            ("#1\nWe apologize, but our records indicate that an account has already been\n"
             "registered from this IP. If you have legitimate reasons for obtaining another\n"
             "login account, such as sharing your internet connection with other individuals,\n"
             "or you are unaware of any other accounts used over your connection, please\n"
             "contact the administrative staff at " STAFF_EMAIL
             " for assistance.\n\n", d);
            SEND_TO_Q ("Your Selection: ", d);
            return;
        }

		SEND_TO_Q ("All new accounts must be created through the Parallel RPI Website.\n"
				   "The following link will take you to the account creation page:\n"
				   "http://www.forum.parallelrpi.com", d);
		SEND_TO_Q ("Your Selection: ", d);
        //SEND_TO_Q (get_text_buffer (NULL, text_list, "account_application"), d);
        //SEND_TO_Q ("What would you like to name your login account? ", d);
        //d->connected = CON_NEW_ACCT_NAME;
        return;
    }

    else if (*buf == 'L')
    {
        SEND_TO_Q ("Your account name: ", d);
        d->connected = CON_ENTER_ACCT_NME;
        return;
    }

    else if (*buf == 'X')
    {
        close_socket (d);
        return;
    }
}

void
nanny_create_guest (DESCRIPTOR_DATA * d, char *argument)
{
    create_guest_avatar (d, argument);
}

void
nanny_ask_password (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    bool bIsBanned = false;

    delete d->acct;

    if (!*argument)
    {
        close_socket (d);
        return;
    }

    if (strstr (argument, " "))
    {
        SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    if (!isalpha (*argument))
    {
        SEND_TO_Q
        ("\nThe first character of the account name MUST be a letter.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    sprintf (buf, "%s", argument);

    for (size_t i = 0; i < strlen (buf); i++)
    {
        if (!isalpha (buf[i]) && !isdigit (buf[i]))
        {
            SEND_TO_Q
            ("\nIllegal characters in account name (letters/numbers only).\n",
             d);
            if (!maintenance_lock)
                SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
            else
                SEND_TO_Q (get_text_buffer
                           (NULL, text_list, "greetings.maintenance"), d);
            SEND_TO_Q ("Your Selection: ", d);
            d->connected = CON_LOGIN;
            return;
        }
    }

    if (str_cmp (CAP (argument), "Anonymous"))
    {
        d->acct = new account (argument);
    }

    if (!str_cmp (argument, "Anonymous") || !d->acct->is_registered ())
    {
        SEND_TO_Q ("\nNo such account. If you wish to create a new account,\n"
                   "please choose option 'C' from the main menu.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    /*
     *  WE HAVE AN ACCOUNT CONNECTION
     *     1st timer: count = 1, has_pwd = 0, logins = 0, fails = 0
     *     otherwise: has_pwd = 0, count++
     */
    int port = engine.get_port ();
    if (d->acct && d->acct->is_registered () && d->acct->name.length ())
    {
        mysql_safe_query
        ("INSERT INTO %s.ip "
         "  VALUES('%s','%s','%s',NOW(),NOW(),1,0,0,%d,0,0) "
         "  ON DUPLICATE KEY "
         "  UPDATE lasttime = NOW(), "
         "    count = count + 1, has_pwd = 0, host = '%s';",
         (engine.get_config ("player_log_db")).c_str (),
         d->acct->name.c_str (), d->strClientHostname, d->strClientIpAddr,
         port, d->strClientHostname);
    }

    bIsBanned = is_banned (d);

    if (bIsBanned && !IS_SET (d->acct->flags, ACCOUNT_NOBAN))
    {
        sprintf (buf, "\nYour site is currently banned from connecting to %s.\n"
                 "As a result of the large number of people we get from certain Internet\n"
                 "domains, this may not be a result of anything you've done; if you feel that\n"
                 "this is the case, please email our staff at %s and we\n"
                 "will do our best to resolve this issue. Our apologies for the inconvenience.\n",
                 MUD_NAME, STAFF_EMAIL);
        SEND_TO_Q (buf, d);
        sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
        SEND_TO_Q (buf, d);
        d->connected = CON_PENDING_DISC;
        return;
    }

    if (str_cmp(d->acct->last_ip.c_str (), d->strClientHostname) != 0)
        SEND_TO_Q
        ("\r\nPlease enter your password carefully - your access to the game server will be\n"
         "suspended for a one-hour period if you repeatedly fail your logins!\r\n\r\n",
         d);

    SEND_TO_Q ("Password: ", d);

    ECHO_OFF;

    d->connected = CON_PWDCHK;

}

/*                                                                          *
 * function: nanny_check_password        < e.g.> Password: *********        *
 *                                                                          *
 * 09/17/2004 [JWW] - asserted that MYSQL_RES objects were reset to NULL    *
 *                  - ensured a NULL result would not be used               *
 *                  - simplified logic for testing shared ips               *
 *                                                                          */
void
nanny_check_password (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char strAccountSharer[AVG_STRING_LENGTH] = "";
    MYSQL_RES *result;
    MYSQL_ROW row = NULL;
    int nFailedLogins = 0, nSharedIP = 0;

    int port = engine.get_port ();
    if (!*argument)
    {
        close_socket (d);
        return;
    }

    if (!check_password (argument, d->acct->password.c_str ()))
    {

        /*
         *  WE HAVE A LOGIN FAILURE
         *     1st timer: count = 1, has_pwd = 0, logins = 0, fails = 1 (this should never happen)
         *     otherwise: has_pwd = 0, fails++ (count already incremented)
         */
        mysql_safe_query
        ("INSERT INTO %s.ip "
         "  VALUES('%s','%s','%s',NOW(),NOW(),1,0,0,%d,0,1) "
         "  ON DUPLICATE KEY "
         "  UPDATE lasttime = NOW(),fails = fails + 1,"
         "     has_pwd = 0,host = '%s';",
         (engine.get_config ("player_log_db")).c_str (),
         d->acct->name.c_str (), d->strClientHostname, d->strClientIpAddr,
         port, d->strClientHostname);


        if (str_cmp (d->acct->last_ip.c_str (), d->strClientHostname) != 0)
        {
            mysql_safe_query
            ("INSERT INTO failed_logins VALUES ('%s', '%s', UNIX_TIMESTAMP())",
             d->acct->name.c_str (), d->strClientHostname);
            mysql_safe_query
            ("SELECT * FROM failed_logins WHERE hostname = '%s' AND timestamp >= (UNIX_TIMESTAMP() - 60*60)",
             d->strClientHostname);
            if ((result = mysql_store_result (database)) != NULL)
            {
                nFailedLogins = mysql_num_rows (result);
                mysql_free_result (result);
                result = NULL;
            }
            else
            {
                sprintf (buf, "Warning: nanny_check_password(): %s",
                         mysql_error (database));
                system_log (buf, true);
                close_socket (d);	/* something bad happened during login... disconnect them (JWW) */
                return;
            }

            if (nFailedLogins >= 3
                    && !IS_SET (d->acct->flags, ACCOUNT_NOBAN))
            {
                if (d->acct && is_admin (d->acct->name.c_str ()))
                {
                    sprintf (buf, "Staff Security Notificiation [%s]",
                             d->acct->name.c_str ());
                    sprintf (buf2,
                             "The following host, %s, has been temporarily banned from the MUD server for incorrectly failing to log into your game account three times within one hour.\n\nUse the BAN command in-game to see which player accounts, if any, are affected by the ban; this should help you track down the perpetrator.",
                             d->strClientHostname);
                    send_email (d->acct, STAFF_EMAIL,
                                "Account Security <" STAFF_EMAIL ">", buf,
                                buf2);
                }
                else if (d->acct)
                {
                    sprintf (buf, "Account Security Notification");
                    sprintf (buf2,
                             "Hello,\n\nA user has just been temporarily sitebanned from the MUD server for three failed login attempts to your game account, %s, originating from a foreign IP and occurring within the past hour.\n\nIf this individual was not you, please notify the staff by replying directly to this message; someone may be attempting to access your account illegally. Otherwise, you will regain your access privileges automatically one hour from the time of this email notification.\n\nThank you for playing!\n\n\n\nWarmest Regards,\nThe Admin Team",
                             d->acct->name.c_str ());
                    sprintf (buf3, "Account Security <%s>", STAFF_EMAIL);
                    send_email (d->acct, NULL, buf3, buf, buf2);
                }
                SEND_TO_Q
                ("\n\nYour access to this server has been suspended for one hour due to repeated\n"
                 "incorrect password attempts. Please email the staff if you have any questions.\n\n"
                 "Additionally, the registered owner of this account has been notified via email.\n\n",
                 d);
                ban_host (d->strClientHostname, "Password Security System", -2);
                d->connected = CON_PENDING_DISC;
                return;
            }
        }
        SEND_TO_Q
        ("\n\nIncorrect password - have you forgotten it? Visit here to obtain a new one:\n\nhttp://www.forum.parallelrpi.com",
         d);
        d->acct->password_attempt++;
        return;
    }

    ECHO_ON;

    d->color = d->acct->color;
    d->sound = d->acct->sound;

    if (!strstr (d->strClientHostname, "atonementrpi.com"))
    {
        std::string escaped_ip;
        std::string escaped_name;

        d->acct->get_last_ip_sql_safe (escaped_ip);
        d->acct->get_name_sql_safe (escaped_name);

        std::ostringstream ipshare_query_stream;

        ipshare_query_stream <<
        "SELECT username FROM forum_users "
        "WHERE user_last_ip != '(null)' "
        "AND user_last_ip != '' "
        "AND user_last_ip = '" << escaped_ip << "' "
        "AND username != '" << escaped_name << "'";

        std::string ipshare_query_string = ipshare_query_stream.str ();
        mysql_safe_query ((char *)ipshare_query_string.c_str ());

        if ((result = mysql_store_result (database)) != NULL)
        {
            nSharedIP = mysql_num_rows (result);

            if (!IS_SET (d->acct->flags, ACCOUNT_IPSHARER) && nSharedIP > 0
                    && str_cmp (d->acct->name.c_str (), "Guest") != 0
                    && str_cmp (d->acct->last_ip.c_str (), "(null)") != 0
                    && d->acct->last_ip.find ("atonementrpi.com") == std::string::npos)
            {
                strcpy (strAccountSharer,
                        "  #1Possible IP sharing detected with:");
                while ((row = mysql_fetch_row (result)))
                {
                    strcat (strAccountSharer, " ");
                    strcat (strAccountSharer, row[0]);
                }
                strcat (strAccountSharer, "#0");
            }

            mysql_free_result (result);
            result = NULL;
        }
        else
        {
            sprintf (buf, "Warning: nanny_check_password(): %s",
                     mysql_error (database));
            system_log (buf, true);
        }
    }

    sprintf (buf, "%s [%s] has logged in.%s\n", d->acct->name.c_str (),
             d->strClientHostname,
             (nSharedIP) ? strAccountSharer : "");
    send_to_gods (buf);
    system_log (buf, false);

    /*
     *  WE HAVE A LOGIN
     *     1st timer: count = 1, has_pwd = 1, logins = 1 (this should never happen)
     *     otherwise: has_pwd = 1, logins++ (count already incremented)
     */
    std::string pwd = argument;
    std::string drupal_pass =
        "UPDATE forum_users "
        "SET pass = MD5('" + pwd + "') "
        "WHERE username = '" + d->acct->name + "'" ;
    mysql_safe_query ((char *)drupal_pass.c_str ());

    mysql_safe_query
    ("INSERT INTO %s.ip "
     "  VALUES('%s','%s','%s',NOW(),NOW(),1,0,1,%d,1,0) "
     "  ON DUPLICATE KEY UPDATE lasttime = NOW(), "
     "    logins = logins + 1, has_pwd = 1,host = '%s';",
     (engine.get_config ("player_log_db")).c_str (),
     d->acct->name.c_str (),
     d->strClientHostname,
     d->strClientIpAddr, port,
     d->strClientHostname);

    d->acct->update_last_ip (d->strClientHostname);
    d->acct->password_attempt = 0;

    display_main_menu (d);
}

void
nanny_new_account (DESCRIPTOR_DATA * d, char *argument)
{
    account *acct = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    sprintf (buf, argument);

    if (!*buf)
    {
        close_socket (d);
        return;
    }

    if (strlen (buf) > 36)
    {
        SEND_TO_Q
        ("\nPlease choose an account name of less than 36 characters.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    if (strstr (buf, " "))
    {
        SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    if (!isalpha (*buf))
    {
        SEND_TO_Q
        ("\nThe first character of the account name MUST be a letter.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }

    for (size_t i = 0; i < strlen (buf); i++)
    {
        if (!isalpha (buf[i]) && !isdigit (buf[i]))
        {
            SEND_TO_Q
            ("\nIllegal characters in account name (letters/numbers only). Please hit ENTER.\n",
             d);
            if (!maintenance_lock)
                SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
            else
                SEND_TO_Q (get_text_buffer
                           (NULL, text_list, "greetings.maintenance"), d);
            SEND_TO_Q ("Your Selection: ", d);
            d->connected = CON_LOGIN;
            return;
        }

        if (i)
            buf[i] = tolower (buf[i]);
        else
            buf[i] = toupper (buf[i]);
    }

    acct = new account (CAP (argument));
    if (str_cmp (CAP (argument), "Anonymous")
            && !acct->is_registered ())
    {
        sprintf (buf2, "\nApply for a login account named %s? [y/n]  ", buf);
        SEND_TO_Q (buf2, d);
        d->connected = CON_ACCT_POLICIES;
        d->stored = str_dup (buf);
    }
    else
    {
        SEND_TO_Q ("\nThat account name has already been taken.\n", d);
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
    }

    delete acct;
    return;
}

void
nanny_account_policies (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, buf);

    *buf = toupper (*buf);

    if (!*buf)
    {
        close_socket (d);
        return;
    }

    if (*buf == 'Y')
    {
        d->acct = new account;
        d->acct->set_name (d->stored);
        d->stored = str_dup ("");
        d->acct->created_on = time (0);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "account_policies"), d);
        SEND_TO_Q ("Do you agree? (y/n) ", d);
        d->connected = CON_REFERRAL;
        return;
    }
    else
    {
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }
}

void
nanny_account_referral (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, buf);

    *buf = toupper (*buf);

    if (!*buf)
    {
        close_socket (d);
        return;
    }

    if (*buf == 'Y')
    {
        mysql_safe_query
        ("INSERT INTO account_referrals VALUES ('%s', '%s', UNIX_TIMESTAMP())",
         d->acct->name.c_str (), "Other");
        SEND_TO_Q (get_text_buffer (NULL, text_list, "account_email"), d);
        SEND_TO_Q ("Your email address: ", d);
        d->connected = CON_EMAIL_CONFIRM;
        return;
    }
    else
    {
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        return;
    }
}

void
nanny_account_email (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char referrer[15];

    argument = one_argument (argument, buf);

    *buf = toupper (*buf);

    if (!*buf)
    {
        close_socket (d);
        return;
    }

    if (isdigit (*buf)
            && (atoi (buf) == 1 || atoi (buf) == 2 || atoi (buf) == 3
                || atoi (buf) == 4 || atoi (buf) == 5 || atoi (buf) == 6
                || atoi (buf) == 7))
    {

        if (atoi (buf) == 1)
            sprintf (referrer, "wm");
        else if (atoi (buf) == 2)
            sprintf (referrer, "aff");
        else if (atoi (buf) == 3)
            sprintf (referrer, "mc");
        else if (atoi (buf) == 4)
            sprintf (referrer, "tms");
        else if (atoi (buf) == 5)
            sprintf (referrer, "other_tolk");
        else if (atoi (buf) == 6)
            sprintf (referrer, "other_mud");
        else
            sprintf (referrer, "other");

        mysql_safe_query
        ("INSERT INTO account_referrals VALUES ('%s', '%s', UNIX_TIMESTAMP())",
         d->acct->name.c_str (), referrer);

        SEND_TO_Q (get_text_buffer (NULL, text_list, "account_email"), d);
        SEND_TO_Q ("Your email address: ", d);
        d->connected = CON_EMAIL_CONFIRM;
        return;
    }
    else
    {
        SEND_TO_Q (get_text_buffer (NULL, text_list, "account_referral"), d);
        SEND_TO_Q
        ("Please choose an option between 1 and 7.\n\nYour Selection: ", d);
        return;
    }
}

/*                                                                          *
 * function: nanny_account_email_confirm                                    *
 *                                  < e.g.> Your email address: null@bar.fu *
 *                                                                          *
 * 09/20/2004 [JWW] - fixed a place where mysql_res wasn't handled correct  *
 *                                                                          */
void
nanny_account_email_confirm (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    MYSQL_RES *result;
    int nUserEmailMatches = 0;

    if (!*argument)
    {
        close_socket (d);
        return;
    }

    if (strstr (argument, " ") || strstr (argument, ";")
            || strstr (argument, "\\") || strstr (argument, "("))
    {
        SEND_TO_Q ("\n#1Your input contains illegal characters.#0\n", d);
        SEND_TO_Q ("Your email address: ", d);
        d->connected = CON_EMAIL_CONFIRM;
        return;
    }

    if (!strstr (argument, "@") || !strstr (argument, "."))
    {
        SEND_TO_Q
        ("\nYour listed email address must include an '@' symbol and a dot.\n\n",
         d);
        SEND_TO_Q ("Your email address: ", d);
        d->connected = CON_EMAIL_CONFIRM;
        return;
    }

    mysql_safe_query
    ("SELECT user_email FROM forum_users WHERE user_email = '%s' AND username != '%s'",
     argument, d->acct->name.c_str ());
    if ((result = mysql_store_result (database)) != NULL)
    {
        nUserEmailMatches = mysql_num_rows (result);
        mysql_free_result (result);
        result = NULL;
    }
    else
    {
        sprintf (buf, "Warning: nanny_account_email_confirm(): %s",
                 mysql_error (database));
        system_log (buf, true);
    }

    if (nUserEmailMatches > 0)
    {
        /// \todo LOG THESE

        SEND_TO_Q
        ("\nWe're sorry, but that email address has already been registered\n"
         "under an existing game account. Please choose another.\n\n", d);
        SEND_TO_Q ("Your email address: ", d);
        d->connected = CON_EMAIL_CONFIRM;
        return;
    }

    sprintf (buf, "Is the address %s correct? [y/n] ", argument);
    SEND_TO_Q (buf, d);

    d->acct->set_email (argument);
    d->connected = CON_ACCOUNT_SETUP;
    mysql_free_result (result);

    return;
}

void
nanny_new_password (DESCRIPTOR_DATA * d, char *argument)
{
    if (!*argument || strlen (argument) < 6 || strlen (argument) > 25)
    {
        SEND_TO_Q ("\n\nPasswords should be 6 to 25 characters in length.\n\n",
                   d);
        SEND_TO_Q ("Password: ", d);
        return;
    }

    d->stored = encrypt_buf (argument);

    SEND_TO_Q ("\nPlease retype your new password: ", d);

    d->connected = CON_PWDNCNF;
}

void
nanny_change_password (DESCRIPTOR_DATA * d, char *argument)
{
    nanny_new_password (d, argument);

    d->connected = CON_PWDNCNF;
}

void
nanny_conf_change_password (DESCRIPTOR_DATA * d, char *argument)
{
//      char                    buf [MAX_STRING_LENGTH];

//      argument = one_argument (argument, buf);

    if (!check_password (argument, d->stored))
    {
        SEND_TO_Q ("\n\nPasswords didn't match.\n\n", d);
        SEND_TO_Q ("Retype password: ", d);
        d->connected = CON_PWDNEW;
        return;
    }

    ECHO_ON;

    d->acct->update_password (d->stored);
    mem_free (d->stored); // char*

    SEND_TO_Q ("\n\n#2Account password successfully modified.#0\n\n", d);

    display_main_menu (d);

}

void
nanny_disconnect (DESCRIPTOR_DATA * d, char *argument)
{
    if (d->character)
    {
        extract_char (d->character);
        d->character = NULL;
    }

    close_socket (d);
}

void
setup_new_account (account  *acct)
{
    char *encrypted;
    char *password;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char email[MAX_STRING_LENGTH];

    password = str_dup (generate_password (1, (char **) "8"));

    sprintf (buf, "Greetings,\n"
             "\n"
             "Thank you for your interest in our community! This\n"
             "was sent to inform you that your login account has been\n"
             "created on our server; please don't respond to this email,\n"
             "as it is merely an automated system notification.\n"
             "\n"
             "MUD Account: %s\n"
             "Password: %s\n"
             "\n"
             "\n"
             "Feel free to hop on in and join in our discussions!\n"
             "\n"
             "Best of luck, and again, welcome to %s.\n\n\n"
             "                                 Warmest Regards,\n"
             "                                 The Admin Team\n", acct->name.c_str (),
             password, MUD_NAME);

    encrypted = encrypt_buf (password);
    acct->set_password (encrypted);
    mem_free (encrypted); // char* from crypt

    acct->newsletter = true;

    sprintf (email, "%s <%s>", MUD_NAME, MUD_EMAIL);
    sprintf (buf2, "Welcome to %s!", MUD_NAME);

    send_email (acct, NULL, email, buf2, buf);


    std::string escaped_name;
    std::string escaped_password;
    std::string escaped_email;
    std::string escaped_last_ip;

    acct->get_name_sql_safe (escaped_name);
    acct->get_password_sql_safe (escaped_password);
    acct->get_email_sql_safe (escaped_email);
    acct->get_last_ip_sql_safe (escaped_last_ip);

    mysql_set_server_option(database,MYSQL_OPTION_MULTI_STATEMENTS_ON);
    std::string insert_query =
        "SELECT (@next_id:=(MAX(user_id)+1)) AS next_id FROM forum_users; "

        "INSERT INTO forum_users "
        "(user_id, user_regdate, username, user_password, user_email, user_last_ip) "
        "VALUES(@next_id, UNIX_TIMESTAMP(NOW()), "
        "'" + escaped_name + "', "
        "'" + escaped_password + "', "
        "'" + escaped_email + "', "
        "'" + escaped_last_ip + "');"

        "INSERT INTO forum_groups "
        "(group_name, group_description, group_single_user, group_moderator) "
        "VALUES (' ', 'Personal User', 1, 0);"

        "INSERT INTO forum_user_group "
        "(user_id, group_id, user_pending) "
        "VALUES (@next_id, LAST_INSERT_ID(), 0)";

    mysql_safe_query ((char *)insert_query.c_str ());

    do
    {
        /* Process all results */
        MYSQL_RES *result;
        if (!(result= mysql_store_result (database)))
        {
            fprintf (stderr, "Got fatal error processing query: %s\n", mysql_error (database));
        }
        else
        {
            mysql_free_result (result);
        }
    }
    while (!mysql_next_result (database));

    mysql_set_server_option (database,MYSQL_OPTION_MULTI_STATEMENTS_OFF);

}

/****************************************************************************************
// Use this function if you only wish to allow users to reset their passwords,
// rather than change them to a specified (potentially unsecure) pass.

void
generate_new_password (account  *acct)
{
  char *encrypted;
  char *password;
  char buf[MAX_STRING_LENGTH];
  char email[MAX_STRING_LENGTH];

  password = str_dup (generate_password (1, (char **) "8"));

  sprintf (buf, "Greetings,\n"
	   "\n"
	   "  As requested, here is the new login password for your\n"
	   "game login account. Please be sure to keep your password\n"
	   "secure; game staff will NEVER ask you to reveal it!\n"
	   "\n"
	   "New Password: %s\n"
	   "\n" "\n" "Warmest Regards,\n" "The Admin Team\n", password);

  encrypted = encrypt_buf (password);
  acct->set_password (encrypted);
  mem_free (encrypted); // char* from crypt()

  sprintf (email, "%s <%s>", MUD_NAME, MUD_EMAIL);

  send_email (acct, NULL, email, "Your New Account Password", buf);
  save_account (acct);
}
*****************************************************************************************/

void
nanny_account_setup (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!*argument)
    {
        close_socket (d);
        return;
    }

    argument[0] = toupper (argument[0]);

    if (*argument == 'Y')
    {
        SEND_TO_Q (get_text_buffer (NULL, text_list, "thankyou"), d);
        d->acct->set_last_ip (d->strClientHostname);
        setup_new_account (d->acct);
        sprintf (buf, "New account: %s [%s].\n", d->acct->name.c_str (),
                 d->strClientHostname);
        send_to_gods (buf);
        sprintf (buf, "New account: %s [%s].", d->acct->name.c_str (),
                 d->strClientHostname);
        system_log (buf, false);

        /*
         *  WE HAVE AN ACCOUNT CREATED
         *     1st timer: count = 1, is_new = 1, has_pwd = 0, logins = 0, fails = 0
         *     otherwise: not possible!
         */
        int port = engine.get_port ();
        mysql_safe_query
        ("INSERT INTO %s.ip "
         "  VALUES('%s','%s','%s',NOW(),NOW(),1,1,0,%d,0,0);",
         (engine.get_config ("player_log_db")).c_str (),
         d->acct->name.c_str (),
         d->strClientHostname,
         d->strClientIpAddr,
         port);

        //send_to_gods(mysql_error ( database ));
        SEND_TO_Q ("Press ENTER to disconnect from the server.\n", d);
        d->connected = CON_PENDING_DISC;
        new_accounts++;
        mysql_safe_query
        ("UPDATE newsletter_stats SET new_accounts=new_accounts+1");
        return;
    }
    else
    {
        if (!maintenance_lock)
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
        else
            SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                       d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_LOGIN;
        d->acct = NULL;
        return;
    }
}

void
nanny_name_confirm (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (*argument != 'y' && *argument != 'Y')
    {
        SEND_TO_Q ("What name would you like? ", d);
        d->connected = CON_NEW_ACCT_NAME;
        return;
    }

    sprintf (buf, "New player %s [%s].", GET_NAME (d->character),
             d->strClientHostname);
    system_log (buf, false);
    //strcat (buf, "\n");
    send_to_gods (buf);

    SEND_TO_Q ("Select a password: ", d);
    ECHO_OFF;

    d->connected = CON_LOGIN;
}

void
nanny_reject_message (DESCRIPTOR_DATA * d, char *argument)
{
    SEND_TO_Q (get_text_buffer (NULL, text_list, "new_menu"), d);
    d->connected = CON_CREATION;
}


/*                                                                          *
 * function: post_retirement                                                *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed an instances where mysql result was not freed   *
 *                  - handled case of result = NULL                         *
 *                                                                          */
void
post_retirement (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *tch;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char date[32];
    time_t time_now;
    char buf[MAX_STRING_LENGTH];

    time_now = time (0);
    ctime_r (&time_now, date);
    date[strlen (date) - 1] = '\0';

    if (!*d->pending_message->message)
    {
        SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
        display_main_menu (d);
        return;
    }
    std::string player_db = engine.get_config ("player_db");
    mysql_safe_query
    ("SELECT name,account"
     " FROM %s.pfiles"
     " WHERE account = '%s'"
     " AND create_state = 2"
     " AND level = 0",
     player_db.c_str (), d->acct->name.c_str ());

    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: post_retirement(): %s", mysql_error (database));
        system_log (buf, true);
        SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    while ((row = mysql_fetch_row (result)))
    {
        mysql_safe_query
        ("UPDATE %s.pfiles"
         " SET create_state=4"
         " WHERE name = '%s'",
         player_db.c_str (), row[0]);

        add_message (1, row[0], -2, d->acct->name.c_str (), date, "Retired.", "",
                     d->pending_message->message, 0);
        add_message (1, "Retirements", -5, row[1], date, row[0], "",
                     d->pending_message->message, 0);
        if ((tch = load_pc (row[0])))
        {
            clan_forum_remove_all (tch);
            death_email (tch);
            unload_pc (tch);
        }
    }
    mysql_free_result (result);
    result = NULL;

    SEND_TO_Q
    ("\n#2Character retired successfully; you may create a new one now.#0\n",
     d);
    display_main_menu (d);
}

/*                                                                          *
 * function: nanny_retire                                                   *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed an instance where result = NULL was not handled *
 *                                                                          */
void
nanny_retire (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];

    ECHO_ON;

    if (!*argument)
    {
        SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
        display_main_menu (d);
        return;
    }

    if (!check_password (argument, d->acct->password.c_str ()))
    {
        SEND_TO_Q ("\n#2Incorrect password. Character retirement aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }
    std::string player_db = engine.get_config ("player_db");
    mysql_safe_query
    ("SELECT name,account"
     " FROM %s.pfiles"
     " WHERE account = '%s'"
     " AND create_state = 2",
     player_db.c_str (), d->acct->name.c_str ());

    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: nanny_retire(): %s", mysql_error (database));
        system_log (buf, true);
        SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    if (!mysql_num_rows (result))
    {
        if (result != NULL)
            mysql_free_result (result);
        SEND_TO_Q ("\n#2You do not currently have a character to retire.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    while ((row = mysql_fetch_row (result)))
    {
        if (get_pc (row[0]))
        {
            mysql_free_result (result);
            SEND_TO_Q
            ("\n#2You may not retire a character while s/he is logged in.#0\n",
             d);
            display_main_menu (d);
            return;
        }
    }
    mysql_free_result (result);
    result = NULL;

    SEND_TO_Q
    ("\n#2Please document thoroughly the reasoning behind your character retirement\n"
     "below; when finished, terminate the editor with an '@' symbol.#0\n\n",
     d);


    CREATE (d->pending_message, MESSAGE_DATA, 1);

    d->str = &d->pending_message->message;
    d->max_str = MAX_STRING_LENGTH;

    d->proc = post_retirement;
}

/*                                                                          *
 * function: nanny_terminate                                                *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed 2 instances where result = NULL was not handled *
 *                                                                          */
void
nanny_terminate (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int id;
    char buf[MAX_STRING_LENGTH];

    ECHO_ON;

    if (!*argument)
    {
        SEND_TO_Q ("\n#2Account termination aborted.#0\n", d);
        display_main_menu (d);
        return;
    }

    if (!check_password (argument, d->acct->password.c_str ()))
    {
        SEND_TO_Q ("\n#2Incorrect password. Account termination aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    mysql_safe_query ("SELECT user_id FROM forum_users WHERE username = '%s'",
                      d->acct->name.c_str ());
    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: nanny_terminate(): %s", mysql_error (database));
        system_log (buf, true);
        SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    row = mysql_fetch_row (result);
    id = atoi (row[0]);
    mysql_free_result (result);
    result = NULL;
    std::string player_db = engine.get_config ("player_db");
    mysql_safe_query ("SELECT name FROM %s.pfiles WHERE account = '%s'",
                      player_db.c_str (), d->acct->name.c_str ());
    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: nanny_terminate(): %s", mysql_error (database));
        system_log (buf, true);
        SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n",
                   d);
        display_main_menu (d);
        return;
    }

    while ((row = mysql_fetch_row (result)))
    {
        mysql_safe_query ("DELETE FROM player_notes WHERE name = '%s'", row[0]);
    }
    mysql_free_result (result);
    result = NULL;

    mysql_safe_query ("DELETE FROM forum_user_group WHERE user_id = %d", id);
    mysql_safe_query ("DELETE FROM hobbitmail WHERE account = '%s'",
                      d->acct->name.c_str ());
    mysql_safe_query
    ("DELETE FROM registered_characters WHERE account_name = '%s'",
     d->acct->name.c_str ());
    mysql_safe_query ("DELETE FROM forum_users WHERE username = '%s'",
                      d->acct->name.c_str ());
    mysql_safe_query ("DELETE FROM %s.pfiles WHERE account = '%s'",
                      player_db.c_str (), d->acct->name.c_str ());

    sprintf (buf, "Account %s has been terminated.", d->acct->name.c_str ());
    send_to_gods (buf);

    SEND_TO_Q ("\n#2Account termination successful. Goodbye.#0\n", d);
    sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
    SEND_TO_Q (buf, d);
    d->connected = CON_PENDING_DISC;
    return;

}

/*                                                                          *
 * function: display_hobbitmail_inbox                                       *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed an instance where mysql result was not freed    *
 *                                                                          */
void
display_hobbitmail_inbox (DESCRIPTOR_DATA * d, account  *acct)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];
    char from_buf[MAX_STRING_LENGTH];
    char date_buf[MAX_STRING_LENGTH];
    char re_buf[MAX_STRING_LENGTH];
    char imm_buf[MAX_STRING_LENGTH];
    int i, admin = 0;

    sprintf (buf, "                              #6MUD-Mail: Main Menu#0\n"
             "                              #6----------------------#0\n\n");

    if (!acct || acct->name.empty ())
    {
        sprintf (buf + strlen (buf),
                 "                  #2There are currently no messages in your inbox.#0\n");
        SEND_TO_Q (buf, d);
        return;
    }

    mysql_safe_query
    ("SELECT * FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC",
     acct->name.c_str ());

    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf + strlen (buf),
                 "                  #2There are currently no messages in your inbox.#0\n");
        sprintf (buf + strlen (buf),
                 "\nEnter message number to display, \"new\", or \"exit\": ");
        SEND_TO_Q (buf, d);
        sprintf (buf, "Warning: display_hobbitmail_inbox(): %s",
                 mysql_error (database));
        system_log (buf, true);
        return;
    }

    if (mysql_num_rows (result) <= 0)
    {
        sprintf (buf + strlen (buf),
                 "                  #2There are currently no messages in your inbox.#0\n");
        sprintf (buf + strlen (buf),
                 "\nEnter message number to display, \"new\", or \"exit\": ");
        SEND_TO_Q (buf, d);
    }
    else
    {
        admin = is_admin (d->acct->name.c_str ());
        i = 0;
        while ((row = mysql_fetch_row (result)))
        {
            i++;
            if (i > 100)
            {
                sprintf (buf + strlen (buf),
                         "\n...remaining messages truncated. Delete current messages to see them.\n");
                break;
            }
            if (d->acct && admin == true)
                sprintf (imm_buf, " [%s]", row[3]);
            else
                *imm_buf = '\0';

            *from_buf = '\0';
            *date_buf = '\0';
            *re_buf = '\0';

            sprintf (from_buf, "#2From:#0 %s%s", row[2], admin ? imm_buf : "");
            sprintf (date_buf, "#2Dated:#0 %s", row[4]);
            sprintf (re_buf, "     #2Regarding:#0 %s%s%s", row[5],
                     !IS_SET (atoi (row[1]), MF_READ) ? " #6(unread)#0" : "",
                     IS_SET (atoi (row[1]),
                             MF_REPLIED) ? " #5(replied)#0" : "");

            sprintf (buf + strlen (buf), "#6%3d.#0 %-40s %s\n%s\n\n", i,
                     from_buf, date_buf, re_buf);
        }
        sprintf (buf + strlen (buf),
                 "Enter message number to display, \"new\", or \"exit\": ");
        page_string (d, buf);
    }

    mysql_free_result (result);
    result = NULL;


}

void
nanny_composing_message (DESCRIPTOR_DATA * d, char *argument)
{
    DESCRIPTOR_DATA *td;
    MUDMAIL_DATA *message;
    account *acct = NULL;
    time_t current_time;
    char date[32];

    current_time = time (0);
    ctime_r (&current_time, date);
    if (strlen (date) > 1)
        date[strlen (date) - 1] = '\0';

    CREATE (message, MUDMAIL_DATA, 1);
    message->from = str_dup (d->pending_message->poster);
    message->subject = str_dup (d->pending_message->subject);
    message->message = str_dup (d->pending_message->message);
    message->from_account = str_dup (d->acct->name.c_str ());
    message->date = str_dup (date);
    message->flags = 0;
    message->target = str_dup (d->pending_message->target);

    acct = new account (d->stored);

    save_hobbitmail_message (acct, message);

    mem_free (message->from);
    mem_free (message->subject);
    mem_free (message->message);
    mem_free (message->from_account);
    mem_free (message->date);
    mem_free (message); // MUDMAIL_DATA*

    unload_message (d->pending_message);
    d->pending_message = NULL;

    if (!acct->is_registered ())
    {
        SEND_TO_Q
        ("#1Your message was not delivered; there was an error.#0\n\n", d);
    }
    else
    {
        SEND_TO_Q
        ("#2Thanks! Your MUD-Mail has been delivered to the specified account.#0\n\n",
         d);
        for (td = descriptor_list; td; td = td->next)
        {
            if (!td->acct || td->acct->name.empty () || !td->character
                    || td->connected != CON_PLYNG)
                continue;
            if (str_cmp (td->acct->name.c_str (), acct->name.c_str ()) == 0)
                SEND_TO_Q
                ("#6\nA new MUD-Mail has arrived for your account!#0\n", td);
        }
    }

    delete acct;

    d->stored = str_dup ("");

    display_hobbitmail_inbox (d, d->acct);

    d->connected = CON_MAIL_MENU;
}

void
nanny_compose_message (DESCRIPTOR_DATA * d, char *argument)
{

    if (!*argument)
    {
        SEND_TO_Q ("\n", d);

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }

    if (isdigit (*argument))
    {
        SEND_TO_Q ("\nRegarding? ", d);
        return;
    }

    d->pending_message->subject = add_hash (argument);

    SEND_TO_Q
    ("\n#2Enter message; terminate with an '@' when completed. Once finished,\n",
     d);
    SEND_TO_Q
    ("hit ENTER again to send and return to the main MUD-Mail menu.#0\n\n",
     d);

    d->pending_message->message = NULL;
    d->str = &d->pending_message->message;
    d->max_str = MAX_STRING_LENGTH;
    d->connected = CON_COMPOSING_MESSAGE;
}

void
nanny_compose_subject (DESCRIPTOR_DATA * d, char *argument)
{
    if (!*argument)
    {

        SEND_TO_Q ("\n", d);

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }

    if (isdigit (*argument))
    {
        SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
        return;
    }

    d->pending_message->poster = add_hash (argument);
    d->connected = CON_COMPOSE_MESSAGE;
    SEND_TO_Q ("\nRegarding? ", d);
    return;
}

/*                                                                          *
 * function: nanny_compose_mail_to                                          *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed an instance where mysql result was not freed    *
 *                  - handled case of result = NULL                         *
 *                                                                          */
void
nanny_compose_mail_to (DESCRIPTOR_DATA * d, char *argument)
{
    CHAR_DATA *tch;
    account *acct;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];

    if (!*argument)
    {
        SEND_TO_Q ("\n", d);

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }

    if (!(tch = load_pc (argument)))
    {
        std::string player_db = engine.get_config ("player_db");
        mysql_safe_query
        ("SELECT name"
         " FROM %s.pfiles"
         " WHERE keywords"
         " LIKE '%%%s%%' LIMIT 1",
         player_db.c_str (), argument);
        if ((result = mysql_store_result (database)) != NULL)
        {
            if (mysql_num_rows (result) > 0)
            {
                row = mysql_fetch_row (result);
                tch = load_pc (row[0]);
            }
            mysql_free_result (result);
            result = NULL;
        }
        else
        {
            sprintf (buf, "Warning: nanny_compose_mail_to(): %s",
                     mysql_error (database));
            system_log (buf, true);
        }

        if (!tch)
        {
            SEND_TO_Q ("#1\nI am sorry, but that PC could not be found.#0\n",
                       d);
            SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ",
                       d);
            return;
        }

    }

    acct = new account (tch->pc->account_name);

    if (!acct->is_registered ())
    {
        delete acct;
        SEND_TO_Q
        ("#1\nThere seems to be a problem with that PC's account.#0\n", d);
        SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
        unload_pc (tch);
        return;
    }

    mysql_safe_query ("SELECT COUNT(*) FROM hobbitmail WHERE account = '%s'",
                      acct->name.c_str ());

    if ((result = mysql_store_result (database)) != NULL)
    {
        row = mysql_fetch_row (result);
        if (atoi (row[0]) >= 100)
        {
            SEND_TO_Q
            ("#1\nSorry, but that person's mailbox is currently full.#0\n",
             d);
            SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ",
                       d);
            mysql_free_result (result);
            result = NULL;
            delete acct;
            return;
        }
        mysql_free_result (result);
        result = NULL;
    }
    else
    {
        sprintf (buf, "Warning: nanny_compose_mail_to(): %s",
                 mysql_error (database));
        system_log (buf, true);
    }

    if (str_cmp (acct->name.c_str (), "Guest") == 0)
    {
        SEND_TO_Q
        ("#1\nSorry, but MUD-Mail cannot be sent to the guest account.#0\n",
         d);
        SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
        unload_pc (tch);
        delete acct;
        return;
    }

    CREATE (d->pending_message, MESSAGE_DATA, 1);
    d->pending_message->target = str_dup (argument);
    d->stored = str_dup (acct->name.c_str ());
    delete acct;
    unload_pc (tch);


    SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
    d->connected = CON_COMPOSE_SUBJECT;
}

/*                                                                          *
 * function: nanny_mail_menu                                                *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed an instance where mysql result was not freed    *
 *                  - handled case of result = NULL                         *
 *                                                                          */
void
nanny_mail_menu (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int i = 1;
    char imm_buf[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (!argument || !*argument
            || (!isdigit (*argument)
                && strn_cmp (argument, "new", strlen (argument))
                && strn_cmp (argument, "exit", strlen (argument))))
    {
        SEND_TO_Q ("Enter message number to display, \"new,\" or \"exit\": ",
                   d);
        return;
    }

    if (!strn_cmp (argument, "exit", strlen (argument)))
    {
        display_main_menu (d);
        return;
    }

    if (!strn_cmp (argument, "new", strlen (argument)))
    {
        d->connected = CON_COMPOSE_MAIL_TO;
        SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
        return;
    }

    if (isdigit (*argument))
    {
        std::string escaped_name;
        d->acct->get_name_sql_safe (escaped_name);

        std::ostringstream message_query_stream;

        message_query_stream <<
        "SELECT account,flags,from_line,from_account,"
        "sent_date, subject,message, timestamp, id,"
        "DATE_FORMAT(FROM_UNIXTIME(timestamp + "
#ifndef MACOSX
        << (timezone + (int)(d->acct->timezone * 3600)) <<
#else
        <<
#endif
        "),\"%a %b %d %T %Y\") AS sent_date,to_line "
        " FROM hobbitmail WHERE account = '"
        << escaped_name <<
        "' ORDER BY timestamp DESC";

        std::string message_query_string = message_query_stream.str ();
        mysql_safe_query ((char *)message_query_string.c_str ());

        if ((result = mysql_store_result (database)) == NULL)
        {
            sprintf (buf, "Warning: nanny_mail_menu(): %s",
                     mysql_error (database));
            system_log (buf, true);
            SEND_TO_Q
            ("\n#1I am sorry, but that message could not be found.#0\n", d);
            SEND_TO_Q
            ("\nEnter message number to display, \"new\", or \"exit\": ", d);
            return;
        }

        if (!mysql_num_rows (result))
        {
            if (result != NULL)
                mysql_free_result (result);
            SEND_TO_Q
            ("\n#1I am sorry, but that message could not be found.#0\n", d);
            SEND_TO_Q
            ("\nEnter message number to display, \"new\", or \"exit\": ", d);
            return;
        }

        while ((row = mysql_fetch_row (result)))
        {
            if (i == atoi (argument))
                break;
            if (i >= 100)
            {
                row = NULL;
                break;
            }
            i++;
        }

        if (!row)
        {
            mysql_free_result (result);
            result = NULL;
            SEND_TO_Q
            ("\n#1I am sorry, but that message could not be found.#0\n", d);
            SEND_TO_Q
            ("\nEnter message number to display, \"new\", or \"exit\": ", d);
            return;
        }

        *imm_buf = '\0';

        if (is_admin (d->acct->name.c_str ()))
            sprintf (imm_buf, " [%s]", row[3]);

        sprintf (buf, "\n#2From:#0      %s%s\n"
                 "%s%s%s"
                 "#2Dated:#0     %s\n"
                 "#2Regarding:#0 %s\n"
                 "\n"
                 "%s\n",
                 row[2],
                 (is_admin (d->acct->name.c_str ()) ? imm_buf : ""),
                 (row[10] ? "#2To:#0        " : ""),
                 (row[10] ? row[10] : ""),
                 (row[10] ? "\n" : ""), row[9], row[5], row[6]);

        sprintf (buf + strlen (buf),
                 "Enter \"delete,\" \"reply,\" or \"exit\": ");

        set_hobbitmail_flags (atoi (row[8]), MF_READ);

        SEND_TO_Q (buf, d);

        d->stored = (char *) atoi (row[8]);

        d->connected = CON_READ_MESSAGE;

        mysql_free_result (result);
        result = NULL;

    }
}

/*                                                                          *
 * function: nanny_read_message                                             *
 *                                                                          *
 * 09/20/2004 [JWW] - handled case of result = NULL                         *
 *                                                                          */
void
nanny_read_message (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];

    if (!*argument || isdigit (*argument))
    {
        SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
        d->connected = CON_READ_MESSAGE;
        return;
    }

    if (strn_cmp (argument, "delete", strlen (argument))
            && strn_cmp (argument, "reply", strlen (argument))
            && strn_cmp (argument, "exit", strlen (argument)))
    {
        SEND_TO_Q ("Enter \"delete,\" \"reply,\" or \"exit\": ", d);
        d->connected = CON_READ_MESSAGE;
        return;
    }

    if (!strn_cmp (argument, "reply", strlen (argument)))
    {
        set_hobbitmail_flags ((long int) d->stored, MF_READ | MF_REPLIED);
        mysql_safe_query
        ("SELECT from_account, from_line FROM hobbitmail WHERE account = '%s' AND id = %d",
         d->acct->name.c_str (), (long int) d->stored);
        if ((result = mysql_store_result (database)) == NULL)
        {
            sprintf (buf, "Warning: nanny_read_message(): %s",
                     mysql_error (database));
            system_log (buf, true);
            SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
            d->connected = CON_READ_MESSAGE;
            return;
        }
        row = mysql_fetch_row (result);
        d->stored = str_dup (row[0]);

        CREATE (d->pending_message, MESSAGE_DATA, 1);
        d->pending_message->target = str_dup (row[1]);
        mysql_free_result (result);
        result = NULL;
        d->connected = CON_COMPOSE_SUBJECT;
        SEND_TO_Q ("\nAs whom do you wish to send the reply? ", d);
        return;
    }

    if (!strn_cmp (argument, "delete", strlen (argument)))
    {
        mysql_safe_query
        ("DELETE FROM hobbitmail WHERE account = '%s' AND id = %d",
         d->acct->name.c_str (), (long int) d->stored);
        sprintf (buf,
                 "\n#1The specified MUD-Mail has been deleted from your account.#0\n\n");
        SEND_TO_Q (buf, d);

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }

    if (!strn_cmp (argument, "exit", strlen (argument)))
    {

        SEND_TO_Q ("\n", d);

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }
}

#define PFILE_QUERY	"SELECT name,create_state FROM %s.pfiles WHERE account = '%s' AND create_state != 4 ORDER BY birth ASC"

void
nanny_connect_select (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_ROW row = NULL;
    MYSQL_RES *result = NULL;
    DESCRIPTOR_DATA *td;
    char buf[MAX_STRING_LENGTH];
    char state[MAX_STRING_LENGTH];
    int i = 0, nCount = 0;
    char c;
    int argn = 0;

    if (d->acct->color && !d->color)
        d->color = 1;

    if (d->acct->sound && !d->sound)
        d->sound = 1;

    if (!*argument)
    {
        display_main_menu (d);
        return;
    }

    while (isspace ((c = tolower (*argument))))
        argument++;

    argn = strtol (argument,0,10);

    if (c == 'l' || argn == 10)
    {
        sprintf (buf, "%s [%s] has logged out.\n", d->acct->name.c_str (),
                 d->strClientHostname);
        send_to_gods (buf);
        sprintf (buf, "%s [%s] has logged out.", d->acct->name.c_str (),
                 d->strClientHostname);
        system_log (buf, false);
        close_socket (d);
        return;
    }
    else if (argn == 11)
    {
        if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\n"
                       "Your Selection: ", d);
            return;
        }

        if (d->acct->toggle_sound_flag())
        {
            SEND_TO_Q ("\n\nSound is #2enabled#0.\n", d);
            SEND_TO_Q ("#1NOTE: Please be sure to configure your client's MSP protocol and sound-file directory.#0\n", d);
            d->sound = 1;
        }
        else
        {
            d->sound = 0;
            SEND_TO_Q ("\n\nSound is disabled.\n", d);
        }
        display_main_menu (d);
        return;
    }
    else if (c == 'a' || argn == 8)
    {
        if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\n"
                       "Your Selection: ", d);
            return;
        }

        if (d->acct->toggle_color_flag())
        {
            SEND_TO_Q ("ANSI color #2enabled#0.\n", d);
            SEND_TO_Q ("#1NOTE: For best results, "
                       "a default of #0white#1 or #0whitish-gray#1 text "
                       "is recommended.\n", d);
            d->color = 1;
        }
        else
        {
            d->color = 0;
            SEND_TO_Q ("ANSI color disabled.\n", d);
        }
        display_main_menu (d);
        return;
    }
    /*
    else if (c == 'n' || argn == 9)
      {
        if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
      {
        SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\n"
      	     "Your Selection: ", d);
        return;
      }
        if (d->acct->toggle_newsletter_flag ())
      {
        SEND_TO_Q ("\n#2You will now receive our weekly newsletter "
      	     "via email.#0\n", d);
      }
        else
      {
        SEND_TO_Q ("\n#1You will no longer receive our weekly newsletter "
      	     "via email.#0\n", d);
      }
        display_main_menu (d);
        return;
      }
    */
    else if (c == 'c' || argn == 6)
    {
        if (strcasecmp ("Unknown", d->acct->name.c_str()) == 0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }
        SEND_TO_Q ("\nYour registered email address: ", d);
        SEND_TO_Q (d->acct->email.c_str (), d);
        SEND_TO_Q ("\n", d);
        SEND_TO_Q ("\nEnter the desired email address: ", d);
        d->connected = CON_CHG_EMAIL;
        return;
    }

    else if (c == 'g' || argn == 2)
    {
        if (IS_SET (d->acct->flags, ACCOUNT_NOGUEST))
        {
            SEND_TO_Q
            ("\n#1Your guest login privileges have been revoked by an admin.\n\n#0Your Selection: ",
             d);
            return;
        }

        for (td = descriptor_list; td; td = td->next)
        {
            if (!td->character || !td->acct || td->acct->name.empty ())
                continue;
            if (IS_NPC (td->character))
                continue;
            if (td->connected != CON_PLYNG)
                continue;
            if (!IS_MORTAL (td->character))
                continue;
            if (!str_cmp (td->acct->name.c_str (), d->acct->name.c_str ()))
            {
                SEND_TO_Q
                ("\n#2We notice you are currently logged in as a guest.\n"
                 "Kicking that connection to allow play.#0\n", d);

                SEND_TO_Q
                ("\n#1Someone else is logging in to your body.\n"
                 "Kicking your connection to allow them to play.#0\n\n", td);

                close_socket (td);
            }
        }

        SEND_TO_Q ("\n#2A guest login is being created for you...#0\n\n", d);
        SEND_TO_Q
        ("While visiting as a guest, you will be held responsible for\n"
         "following our policies. Guest logins are provided for new\n"
         "players to experience the game in a limited capacity while\n"
         "waiting for an application, or for researching Atonement\n"
         "using our in-game material. We hope you enjoy your stay!\n\n"
         "#1Under NO CIRCUMSTANCES should these logins be used to harass\n"
         "immortals regarding pending character applications! We frown\n"
         "very highly on this sort of abuse of our guest login system.#0\n\n"
         "Please be aware that all guest IPs and commands are logged.\n\n",
         d);

        SEND_TO_Q ("#2When ready, please press ENTER to be incarnated.#0\n", d);
        d->connected = CON_CREATE_GUEST;
        return;
    }

    else if (c == 'e' || argn == 1)
    {
        std::string player_db = engine.get_config ("player_db");
        mysql_safe_query ("SELECT name,create_state"
                          " FROM %s.pfiles"
                          " WHERE account = '%s'"
                          " AND create_state = %d",
                          player_db.c_str (),
                          d->acct->name.c_str (),
                          STATE_SUSPENDED);

        if ((result = mysql_store_result (database)))
        {

            nCount = mysql_num_rows (result);
            mysql_free_result (result);

            if (nCount > 0)
            {

                SEND_TO_Q
                ("You may not enter the game while you have suspended characters in your account.\n",
                 d);
                display_main_menu (d);
                return;

            }
        }

        mysql_safe_query (PFILE_QUERY,
                          player_db.c_str (),
                          d->acct->name.c_str ());
        result = mysql_store_result (database);

        if (!result || !mysql_num_rows (result))
        {
            SEND_TO_Q
            ("There are currently no PCs registered under this account.\n",
             d);
            display_main_menu (d);
            if (result)
                mysql_free_result (result);
            return;
        }

        if (mysql_num_rows (result) > 1)
        {
            SEND_TO_Q ("\nWhich character would you like to log in?\n\n", d);
            i = 1;
            while ((row = mysql_fetch_row (result)))
            {
                if (atoi (row[1]) < 1)
                    sprintf (state, "#3(Pending)#0");
                else if (atoi (row[1]) == 1)
                    sprintf (state, "#6(Submitted)#0");
                else if (atoi (row[1]) == 2)
                    sprintf (state, "#2(Active)#0");
                else if (atoi (row[1]) == 3)
                    sprintf (state, "#5(Suspended)#0");
                else
                    sprintf (state, "#1(Deceased)#0");
                sprintf (buf, "%2d. %-20s %s\n", i, row[0], state);
                SEND_TO_Q (buf, d);
                i++;
            }
            SEND_TO_Q ("\nYour Selection: ", d);
            d->connected = CON_CHOOSE_PC;
            if (result)
                mysql_free_result (result);
            return;
        }
        else
        {
            if (result)
                mysql_free_result (result);
            nanny_choose_pc (d, "1");
            return;
        }
    }

    else if (c == 'd' || argn == 4)
    {
        if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }
        std::string player_db = engine.get_config ("player_db");
        mysql_safe_query
        ("SELECT name,create_state"
         " FROM %s.pfiles"
         " WHERE account = '%s'"
         " AND create_state <= 1",
         player_db.c_str (),
         d->acct->name.c_str ());
        result = mysql_store_result (database);

        if (!result || !mysql_num_rows (result))
        {
            SEND_TO_Q
            ("There are currently no pending PCs on this account to delete.\n",
             d);
            if (result)
                mysql_free_result (result);
            display_main_menu (d);
            return;
        }

        SEND_TO_Q ("\nWhich pending character would you like to delete?\n\n",
                   d);

        i = 1;

        while ((row = mysql_fetch_row (result)))
        {
            sprintf (buf, "%d. %s\n", i, row[0]);
            SEND_TO_Q (buf, d);
            i++;
        }

        SEND_TO_Q ("\nYour Selection: ", d);
        d->connected = CON_DELETE_PC;

        if (result)
            mysql_free_result (result);

        return;
    }

    else if (c == 'h' || argn == 9)
    {
        if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }

        display_hobbitmail_inbox (d, d->acct);

        d->connected = CON_MAIL_MENU;
        return;
    }

    else if (c == 'r' || argn == 3)
    {
        if (str_cmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }
        std::string player_db = engine.get_config ("player_db");
        mysql_safe_query ("SELECT name,create_state "
                          "FROM %s.pfiles "
                          "WHERE account = '%s'"
                          " AND create_state = %d",
                          player_db.c_str (),
                          d->acct->name.c_str (), STATE_SUSPENDED);

        if ((result = mysql_store_result (database)))
        {

            nCount = mysql_num_rows (result);
            mysql_free_result (result);

            if (nCount > 0)
            {

                SEND_TO_Q
                ("You may not create new characters while you are suspended.\n",
                 d);
                display_main_menu (d);
                return;

            }
        }

            d->character = new_char (1);
            //clear_char (d->character);
            d->character->race = -1;
            d->character->descriptor = d;
            SEND_TO_Q (get_text_buffer (NULL, text_list, "help_name"), d);
            SEND_TO_Q ("\nWhat would you like to name your new character? ", d);
            d->connected = CON_NAME_CONFIRM;
            return;
    }
    else if (c == 'm' || argn == 7)
    {
        if (str_cmp ("Unknown", d->acct->name.c_str ())==0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }
        SEND_TO_Q ("Enter a new password: ", d);
        ECHO_OFF;
        d->connected = CON_PWDNEW;
        return;
    }
    else if (argn == 5)
    {
        if (str_cmp ("Unknown", d->acct->name.c_str ()) == 0)
        {
            SEND_TO_Q
            ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
             d);
            return;
        }
        if (IS_SET (d->acct->flags, ACCOUNT_NORETIRE))
        {
            SEND_TO_Q
            ("\n#1Your account has been flagged NORETIRE by an administrator, likely due\n"
             "to abuse on your part of the retirement code. To retire your PC, you will\n"
             "need to contact the administrative staff and ask permission.#0\n",
             d);
            display_main_menu (d);
            return;
        }
        SEND_TO_Q
        ("\nWhile we highly discourage people from lightly throwing away their characters, we\n"
         "also recognize that this is a game, and that there comes a point at which there is\n"
         "little fun to be had from playing a character. Please make this decision carefully;\n"
         "it is #1ABSOLUTELY IRREVOCABLE#0! To confirm, please entire your password below:\n\n"
         "Your Account Password: ", d);
        ECHO_OFF;
        d->connected = CON_RETIRE;
    }
    else
    {
        SEND_TO_Q
        ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
         d);
    }
}

#define skill_lev(val) val >= 70 ? " Master " : val >= 50 ? " Adroit " : val >= 30 ? "Familiar" : " Novice "

void
nanny_view_pc (DESCRIPTOR_DATA * d, char *argument)
{
    return;
}

void
nanny_reading_wait (DESCRIPTOR_DATA * d, char *argument)
{
    display_main_menu (d);
    SEND_TO_Q ("Your Selection: ", d);
    return;
}

void
nanny_delete_pc (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_ROW row;
    MYSQL_RES *result;
    char name[MAX_STRING_LENGTH];
    int i = 0, j = 0;

    if (!*argument)
    {
        display_main_menu (d);
        return;
    }

    if (!isdigit (*argument))
    {
        display_main_menu (d);
        return;
    }
    std::string player_db = engine.get_config ("player_db");
    mysql_safe_query
    ("SELECT name,create_state"
     " FROM %s.pfiles"
     " WHERE account = '%s'"
     " AND create_state <= 1",
     player_db.c_str (),
     d->acct->name.c_str ());
    result = mysql_store_result (database);

    if (!result)
    {
        display_main_menu (d);
        return;
    }

    j = mysql_num_rows (result);

    if (atoi (argument) < 1 || atoi (argument) > j)
    {
        SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
        SEND_TO_Q ("Your Selection: ", d);
        d->connected = CON_DELETE_PC;
        if (result)
            mysql_free_result (result);
        return;
    }

    i = 1;

    while ((row = mysql_fetch_row (result)))
    {
        if (atoi (argument) == i)
        {
            if (!(d->character = load_pc (row[0])))
            {
                SEND_TO_Q
                ("\n#1That character could not be loaded. The playerfile may have become\n"
                 "corrupt; please email the staff list about this.#0\n\n", d);
                SEND_TO_Q ("Your Selection: ", d);
                d->connected = CON_DELETE_PC;
                if (result)
                    mysql_free_result (result);
                return;
            }
            else
                break;
        }
        i++;
    }

    sprintf (name, "%s", d->character->tname);
    unload_pc (d->character);
    d->character = NULL;

    mysql_safe_query ("DELETE FROM %s.pfiles"
                      " WHERE name = '%s'",
                      player_db.c_str (), name);

    SEND_TO_Q ("\n#1This pending character has been successfully deleted.#0\n",
               d);
    display_main_menu (d);
    d->connected = CON_ACCOUNT_MENU;

    return;

}

void
get_weapon_skills (CHAR_DATA * ch, int *melee, int *ranged)
{
    int i = 0, j = 0, stop_num = 0;

    for (i = SKILL_SMALL_BLADE; i <= SKILL_BLUDGEON; i++)
    {
        if (real_skill (ch, i))
            j++;
    }

    if (j > 1)
        stop_num = number (1, j);
    else
        stop_num = 1;

    for (i = SKILL_SMALL_BLADE; i <= SKILL_BLUDGEON; i++)
    {
        if (real_skill (ch, i))
        {
            j++;
            if (j == stop_num)
            {
                *melee = i;
                break;
            }
        }
    }

    j = 0;

    for (i = SKILL_HANDGUN; i <= SKILL_GUNNERY; i++)
    {
        if (real_skill (ch, i))
            j++;
    }

    if (j > 1)
        stop_num = number (1, j);
    else
        stop_num = 1;

    for (i = SKILL_HANDGUN; i <= SKILL_GUNNERY; i++)
    {
        if (real_skill (ch, i))
        {
            j++;
            if (j == stop_num)
            {
                *ranged = i;
                break;
            }
        }
    }
}

void
equip_newbie (CHAR_DATA * ch)
{
    OBJ_DATA *tobj = NULL;

    for (tobj = ch->equip; tobj; tobj = tobj->next_content)
    {
        if (tobj == ch->equip)
            ch->equip = ch->equip->next_content;
        else
            ch->equip->next_content = tobj->next_content;
    }

    /*

    get_weapon_skills (ch, &melee, &ranged);

    if ((obj = load_object (5004)))
    {
      equip_char (ch, obj, WEAR_BACK);
      tobj = obj;
    }
    if ((obj = load_object (4000)))
      equip_char (ch, obj, WEAR_BODY);
    if ((obj = load_object (4004)))
      equip_char (ch, obj, WEAR_LEGS);
    if ((obj = load_object (4009)))
      equip_char (ch, obj, WEAR_ABOUT);
    if ((obj = load_object (5000)))
      equip_char (ch, obj, WEAR_FEET);
    if ((obj = load_object (5001)))
      equip_char (ch, obj, WEAR_WAIST);

    if (tobj)
    {
      if ((obj = load_object (99020)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (99020)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (99021)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (99000)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (99000)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (1027)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (9005)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (9005)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (5049)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (3002)))
        obj_to_obj (obj, tobj);
      if ((obj = load_object (3001)))
        obj_to_obj (obj, tobj);


      if (tobj && (obj = load_object (992)))
      {
        obj->count = 4;
        obj_to_obj (obj, tobj);
      }

      if (tobj && (obj = load_object (991)))
      {
        obj->count = 15;
        obj_to_obj (obj, tobj);
      }

      if (ch->skills[SKILL_MEDICINE])
      {
        if(obj = load_object (9008))
          obj_to_obj (obj, tobj);
        if(obj = load_object (9009))
        {
          obj->count = 5;
          obj_to_obj (obj, tobj);
        }
      }
      if (ch->skills[SKILL_LITERACY])
      {
        if ((obj = load_object (9007)))
          obj_to_obj (obj, tobj);
        if ((obj = load_object (61)))
          obj_to_obj (obj, tobj);
        if ((obj = load_object (61)))
          obj_to_obj (obj, tobj);
        if ((obj = load_object (61)))
          obj_to_obj (obj, tobj);
        if ((obj = load_object (9012)))
          obj_to_obj (obj, tobj);
        if ((obj = load_object (9013)))
          obj_to_obj (obj, tobj);
      }
    }
    */

    ch->right_hand = NULL;
    ch->left_hand = NULL;
}

// Restarts in-progress applications at the right place in chargen

int
determine_chargen_stage (CHAR_DATA * ch)
{
    return CON_CREATION;
}

/*                                                                          *
 * function: nanny_choose_pc                                                *
 *                                                                          *
 * 09/20/2004 [JWW] - handled case of result = NULL                         *
 *                                                                          */
void
nanny_choose_pc (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    DESCRIPTOR_DATA *td;
    CHAR_DATA *ch;
    ROOM_DATA *troom;
    OBJ_DATA *obj;
    time_t current_time;
    char date[32];
    char buf[MAX_STRING_LENGTH];
    int i = 0, j = 0, online, guest, to_room = 0;
    extern CHAR_DATA *loaded_list;
    extern int count_guest_online;

    if (!*argument)
    {
        display_main_menu (d);
        return;
    }

    if (!isdigit (*argument))
    {
        display_main_menu (d);
        return;
    }

    if (atoi (argument) == 0)
    {
        display_main_menu (d);
        return;
    }
    std::string player_db = engine.get_config ("player_db");
    mysql_safe_query (PFILE_QUERY, player_db.c_str (), d->acct->name.c_str ());
    result = mysql_store_result (database);

    if (result && mysql_num_rows (result) > 1)
    {
        j = mysql_num_rows (result);

        if (atoi (argument) > j || atoi (argument) < 1)
        {
            SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
            SEND_TO_Q ("Your Selection: ", d);
            d->connected = CON_CHOOSE_PC;
            if (result)
                mysql_free_result (result);
            return;
        }

        i = 1;

        while ((row = mysql_fetch_row (result)))
        {
            if (atoi (argument) == i)
            {
                if (loaded_list)
                {
                    if (!str_cmp (loaded_list->tname, row[0]))
                        loaded_list = loaded_list->next;
                    else
                    {
                        for (ch = loaded_list; ch->next; ch = ch->next)
                        {
                            if (!ch->next)
                                break;
                            if (!str_cmp (GET_NAME (ch->next), row[0]))
                            {
                                ch->next = ch->next->next;
                                break;
                            }
                        }
                    }
                }

                if (!(d->character = load_pc (row[0])))
                {
                    SEND_TO_Q
                    ("\nThat character could not be loaded. Please pick again.\n\n",
                     d);
                    SEND_TO_Q ("Your Selection: ", d);
                    d->connected = CON_CHOOSE_PC;
                    if (result)
                        mysql_free_result (result);
                    return;
                }
                break;
            }
            i++;
        }
    }
    else
    {
        if (!result || !(row = mysql_fetch_row (result))
                || !(d->character = load_pc (row[0])))
        {
            SEND_TO_Q
            ("\nUh oh - your PC could not be loaded from the database!\n\n",
             d);
            display_main_menu (d);
            if (result)
                mysql_free_result (result);
            return;
        }
    }

    if (result)
        mysql_free_result (result);

    if (!strstr (d->strClientHostname, "atonementrpi.com"))
    {
        for (td = descriptor_list; td; td = td->next)
        {
            if (!td->character || !td->acct || td->acct->name.empty ())
                continue;
            if (IS_NPC (td->character))
                continue;
            if (td->connected != CON_PLYNG)
                continue;
            if (IS_SET (td->acct->flags, ACCOUNT_IPSHARER) && strcmp (td->acct->name.c_str(), d->acct->name.c_str()))
                continue;
            if ((!str_cmp (td->strClientHostname, d->strClientHostname)
                    || !str_cmp (td->acct->name.c_str (), d->acct->name.c_str ()))
                    && str_cmp (td->character->tname, d->character->tname)
                    && str_cmp (td->character->tname, "Hal")
                    && str_cmp (d->character->tname, "Hal")
                    && engine.in_play_mode ())
            {
                SEND_TO_Q
                ("\n#1Sorry, but it is against policy to have two characters from the\n"
                 "same account or IP address logged in at the same time.#0\n",
                 d);
                display_main_menu (d);
                return;
            }
        }
    }

    if (d->character->pc->create_state == STATE_REJECTED)
        d->character->pc->create_state = STATE_APPLYING;

    if (d->character->pc->create_state == STATE_APPLYING)
    {
        d->character->descriptor = d;
        create_menu_options (d);
        d->character->pc->nanny_state = 0;
        d->connected = CON_CREATION;
        return;
    }

    if (d->character->pc->create_state == STATE_SUBMITTED)
    {
        SEND_TO_Q
        ("\n#6A review of your character application is pending. Please be patient.#0\n",
         d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (d->character->pc->create_state == STATE_DIED)
    {
        SEND_TO_Q ("\n#1This character is, unfortunately, deceased.#0\n", d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (d->character->pc->create_state == STATE_SUSPENDED)
    {
        SEND_TO_Q
        ("\n#1This character has been suspended by an administrator.#0\n", d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (engine.in_play_mode () && !is_admin (d->acct->name.c_str ()) && maintenance_lock)
    {
        SEND_TO_Q
        ("\n#1Sorry, but the player port is currently closed for maintenance.#0\n",
         d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (!engine.in_play_mode () && !is_admin (d->acct->name.c_str ()))
    {
        SEND_TO_Q
        ("\n#1Sorry, but this port is for game staff only. Please log out immediately.#0\n",
         d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (engine.in_play_mode () && IS_SET (d->character->plr_flags, NO_PLAYERPORT))
    {
        SEND_TO_Q
        ("\n#6Your admin login does not have player port access privileges.#0\n",
         d);
        display_main_menu (d);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    if (d->acct->color)
        d->character->color = 1;
    else
        d->character->color = 0;

    if (d->acct->sound)
        d->character->sound = 1;
    else
        d->character->sound = 0;

    if (d->acct->guide && d->character->pc)
        d->character->pc->is_guide = 1;
    else
        d->character->pc->is_guide = 0;

    perform_pfile_update (d->character);

    if (d->character->pc->owner)
    {

        for (td = descriptor_list; td; td = td->next)
            if (td == d->character->pc->owner)
                break;

        if (!td)
        {
            d->character->pc->owner = NULL;
            system_log
            ("AVOIDED CRASH BUG:  Entering game with owner set wrong.", true);
        }
    }

    /* Character is re-estabilishing a connection while connected */

    if (d->character->descr())
    {
        sprintf (buf, "%s already online, disconnecting old connection.",
                 GET_NAME (d->character));
        system_log (buf, false);
        close_socket (d->character->descr());
    }

    for (td = descriptor_list; td; td = td->next)
    {

        if (td == d)
            continue;

        if (td->original == d->character)
        {
            do_return (td->character, "", 0);
        }
        if (td->character == d->character)
        {
            close_socket (td);
            break;
        }

    }

    d->character->pc->owner = d;

    d->character->pc->last_connect = time (0);

    if ((ch = get_pc (GET_NAME (d->character))) && !ch->pc->admin_loaded)
    {
        ch->act &= ~PLR_QUIET;
        d->connected = CON_PLYNG;

        if (IS_SET (ch->flags, FLAG_FLEE))
        {
            send_to_char ("You stop trying to flee.\n\r", ch);
            ch->flags &= ~FLAG_FLEE;
        }

        ch->descriptor = d;
        d->time_last_activity = mud_time;
        ch->pc->time_last_activity = mud_time;

        unload_pc (ch);		/* Reconnected, we have an extra load count */

        if (d->character->room->vnum == LINKDEATH_HOLDING_ROOM)
        {
            char_from_room (d->character);
            char_to_room (d->character, d->character->was_in_room);
        }

        act ("$n has reconnected.", true, ch, 0, 0, TO_ROOM);
        sprintf (buf, "%s has reconnected.", GET_NAME (ch));
        system_log (buf, false);
        strcat (buf, "\n");
        send_to_gods (buf);
        do_look (d->character, "", 0);
        d->prompt_mode = 1;
        d->connected = CON_PLYNG;

        if (d->character->pc->level)
            show_unread_messages (d->character);

        return;
    }


    pc_to_game (d->character);
    d->prompt_mode = 1;
    d->connected = CON_PLYNG;
    d->character->descriptor = d;

    d->character->flags &= ~(FLAG_FLEE | FLAG_ENTERING | FLAG_LEAVING | FLAG_SUBDUER | FLAG_SUBDUING | FLAG_SUBDUEE);
    if (GET_TRUST (d->character))
    {
        d->character->flags &= ~FLAG_AVAILABLE;
        d->character->flags |= FLAG_WIZINVIS;
    }

    d->time_last_activity = mud_time;
    d->character->pc->time_last_activity = mud_time;

    if (d->character->pc->create_state == STATE_DIED)
    {

        GET_POS (d->character) = STAND;
        remove_cover(d->character, -2);

        char_to_room (d->character, 666);
        d->character->flags |= FLAG_DEAD;
        sprintf (buf, "#1Dead Character:#0 %s has entered the game.\n",
                 GET_NAME (d->character));
        send_to_gods (buf);
        do_look (d->character, "", 15);

        return;
    }

    if ((!d->character->in_room || d->character->in_room == NOWHERE) &&
            !d->character->right_hand && !d->character->left_hand
            && !d->character->equip)
    {
        reformat_desc (d->character->description, &d->character->description);
        equip_newbie (d->character);
    }

    load_char_objs (d->character, GET_NAME (d->character));

    if ((!d->character->in_room || d->character->in_room == NOWHERE)
            || !vnum_to_room (d->character->in_room))
    {
        if (IS_SET (d->character->plr_flags, NEWBIE)
                && vnum_to_room (PREGAME_ROOM_PROTOTYPE))
        {
            to_room =
                clone_contiguous_rblock (vnum_to_room (PREGAME_ROOM_PROTOTYPE), -1, false);
            if ((troom = vnum_to_room (to_room)))
            {
                for (i = 0; pregame_furnishings[i] != -1; i++)
                {
                    if ((obj = load_object (pregame_furnishings[i])))
                        obj_to_room (obj, troom->vnum);
                }
                char_to_room (d->character, troom->vnum);
            }
            else
                char_to_room (d->character, OOC_LOUNGE);
        }
        else
            char_to_room (d->character, OOC_LOUNGE);
    }
    else if (d->character->in_room == GRUNGE_ARENA1 ||
  			 d->character->in_room == GRUNGE_ARENA2 ||
			 d->character->in_room == GRUNGE_ARENA3 ||
			 d->character->in_room == GRUNGE_ARENA4)
    {
        char_to_room (d->character, 56311);

		if (is_clan_member(d->character, "arena_blue") || is_clan_member(d->character, "arena_red"))
		{
			if (is_clan_member(d->character, "arena_blue"))
			{
				remove_clan(d->character, "arena_blue");
			}
			if (is_clan_member(d->character, "arena_red"))
			{
				remove_clan(d->character, "arena_red");
			}

			OBJ_DATA *obj = NULL;
			OBJ_DATA *next_o = NULL;
			WOUND_DATA *wound = NULL;

            for (obj = d->character->equip; obj; obj = next_o)
            {
                next_o = obj->next_content;
                extract_obj (obj);
            }
			if (d->character->left_hand)
			{
				extract_obj(d->character->left_hand);
			}		
			if (d->character->right_hand)
			{
				extract_obj(d->character->right_hand);
			}

			char_to_room(d->character, VNUM_DROPROOM);
			sprintf (buf, "get %s", d->character->tname);
			command_interpreter(d->character, buf);
			char_from_room(d->character);
            char_to_room(d->character, GRUNGE_CHANGE1 + number(0,1));
            act ("$n is dragged back in to the backrooms by a pair of Arena handlers.", false, d->character, 0, 0, TO_ROOM | _ACT_FORMAT);

			for (wound = d->character->wounds; wound; wound = wound->next)
			{
				wound->healerskill = 50;
				if (wound->infection)
				    wound->infection = -1;

				wound->bleeding = 0;
                wound->bindskill = 0;
			}
		}
		send_to_char("You have been scheduled to load in the Arena, most likely due to a crash or reboot. Transfering you to a safe room.\n", d->character);
    }
	else
	{
        char_to_room (d->character, d->character->in_room);
    }

    act ("$n enters the area.", true, d->character, 0, 0, TO_ROOM);

    sprintf (buf, "%s last entered the game %s\n",
             GET_NAME (d->character),
             d->character->pc->last_logon ?
             (char *) ctime (&d->character->pc->last_logon) : "never ");
    buf[strlen (buf) - 2] = '.';	/* gets rid of nl created by ctime */
    send_to_gods (buf);

    d->character->flags &= ~FLAG_BINDING;

    sprintf (buf, "save/player/%c/%s.a",
             tolower (*GET_NAME (d->character)), CAP (GET_NAME (d->character)));

    load_saved_mobiles (d->character, buf);
    *buf = '\0';
    if (d->character->lastregen)
        offline_healing (d->character, d->character->pc->last_logoff);

    d->character->combat_block = 0;

    if (GET_FLAG (d->character, FLAG_NEWBNET))
        if (!IS_SET(d->character->plr_flags, NEW_PLAYER_TAG) && !IS_GUIDE(d->character) && !GET_TRUST(d->character))
            d->character->flags &= ~FLAG_NEWBNET;

    if (d->character->room->vnum == LINKDEATH_HOLDING_ROOM)
    {
        char_from_room (d->character);
        char_to_room (d->character, d->character->was_in_room);
        d->character->was_in_room = 0;
        act ("$n enters the area.", true, d->character, 0, 0, TO_ROOM);
    }

    send_to_char ("\n", d->character);

    if (d->character->pc->special_role)
    {
        outfit_new_char (d->character, d->character->pc->special_role);
        d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
        d->pending_message->poster = str_dup (GET_NAME (d->character));
        d->pending_message->subject =
            str_dup ("Special Role Selected in Chargen.");
        sprintf (buf,
                 "Role Name: %s\n" "Role Cost: %d points\n" "Posted By: %s\n"
                 "Posted On: %s\n" "\n" "%s\n",
                 d->character->pc->special_role->summary,
                 d->character->pc->special_role->cost,
                 d->character->pc->special_role->poster,
                 d->character->pc->special_role->date,
                 d->character->pc->special_role->body);
        d->pending_message->message = str_dup (buf);
        add_message_to_mysql_player_notes (d->character->tname,
                                           d->character->tname,
                                           d->pending_message);
        d->character->pc->special_role = NULL;
    }

    do_look (d->character, "", 15);

    if (!str_cmp (d->character->room->name, PREGAME_ROOM_NAME))
    {
        send_to_char ("\n", d->character);
        act
        ("Welcome to Parallel RPI! If you are a new player and have any questions, please type #6HELP HELPLINE#0. If your questions require an admin, please type #6HELP PETITION#0. We don't bite, promise.",
         false, d->character, 0, 0, TO_CHAR | _ACT_FORMAT);
    }

    /*
    else if (!IS_SET (d->character->plr_flags, NEW_PLAYER_TAG)
         && !IS_SET (d->acct->flags, ACCOUNT_NOVOTE))
      {
        mysql_safe_query
      ("SELECT last_tms_vote,last_mm_vote,last_tmc_vote FROM forum_users WHERE username = '%s'",
       d->acct->name.c_str ());
        if ((result = mysql_store_result (database)) != NULL)
      {
        row = mysql_fetch_row (result);
        if (((time (0) - ((60 * 60 * 24) + (60 * 5))) >= atoi (row[1]))
            || ((time (0) - ((60 * 60 * 24) + (60 * 6))) >= atoi (row[2])))
          {
            send_to_char
      	("\n#6Have you voted recently? If not, and you wish to support Shadows\n"
      	 "of Isildur, please see HELP VOTE for details. Thank you.#0\n",
      	 d->character);
          }
        mysql_free_result (result);
      }
        else
      {
        sprintf (buf, "Warning: nanny_choose_pc(): %s",
      	   mysql_error (database));
        system_log (buf, true);
      }
      }
    */

    show_waiting_prisoners (d->character);
    notify_captors (d->character);
    d->character->effort = 0;

    if (d->character->max_hit != 40 + d->character->con * CONSTITUTION_MULTIPLIER)
    {
        d->character->max_hit = 40 + d->character->con * CONSTITUTION_MULTIPLIER;	// All humanoids are roughly the same,
        d->character->hit = d->character->max_hit;	// in terms of wound-endurance.
    }

    if (d->character->max_shock != 40 + d->character->wil * CONSTITUTION_MULTIPLIER)
    {
        d->character->max_shock = 40 + d->character->wil * CONSTITUTION_MULTIPLIER;	// All humanoids are roughly the same,
        d->character->shock = d->character->max_shock;	// in terms of wound-endurance.
    }

    if (d->character->pc->level)
        show_unread_messages (d->character);

    if (d->character->pc->creation_comment
            && strlen (d->character->pc->creation_comment) > 2
            && !d->character->pc->level)
    {

        current_time = time (0);
        ctime_r (&current_time,date);
        if (strlen (date) > 1)
            date[strlen (date) - 1] = '\0';

        d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
        d->pending_message->poster = str_dup (GET_NAME (d->character));
        d->pending_message->subject = str_dup ("Background Information.");
        d->pending_message->message =
            str_dup (d->character->pc->creation_comment);
        d->pending_message->date = str_dup (date);
        add_message_to_mysql_player_notes (d->character->tname,
                                           d->character->tname,
                                           d->pending_message);

        d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
        d->pending_message->poster = str_dup (GET_NAME (d->character));
        d->pending_message->subject = str_dup ("Description Backup.");
        d->pending_message->message =
            str_dup (d->character->description);
        d->pending_message->date = str_dup (date);
        add_message_to_mysql_player_notes (d->character->tname,
                                           d->character->tname,
                                           d->pending_message);

        d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
        d->pending_message->poster = str_dup (GET_NAME (d->character));
        d->pending_message->subject = str_dup ("My Background.");
        d->pending_message->message =
            str_dup (d->character->pc->creation_comment);
        post_to_mysql_journal (d);

        d->pending_message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
        d->pending_message->poster = str_dup (GET_NAME (d->character));
        d->pending_message->subject = str_dup ("My Description");
        d->pending_message->message =
            str_dup (d->character->description);
        post_to_mysql_journal (d);

        d->character->pc->creation_comment = NULL;
    }

    online = 0;
    guest = 0;

    for (td = descriptor_list; td; td = td->next)
    {

        if (!td->character)
            continue;

        if (!td->character->pc)
            continue;

        if (td->character->pc->level)
            continue;

        if (td->character->pc->create_state != 2)
            continue;

        if (td->connected)
            continue;

        if (IS_SET (td->character->flags, FLAG_GUEST))
            guest++;

        if (IS_MORTAL (td->character)
                && !IS_SET (td->character->flags, FLAG_GUEST))
            online++;
    }

    if (online >= count_max_online)
    {
        count_max_online = online;
        current_time = time (0);
        ctime_r (&current_time, max_online_date);
        max_online_date[strlen (max_online_date) - 1] = '\0';
    }

    if (guest >= count_guest_online)
    {
        count_guest_online = guest;
    }

    d->character->pc->last_logon = time (0);

	//update_family_clanning (d);

	conflicting_clan_check(d->character);

    return;

}

/*                                                                          *
 * function: nanny_change_email                                             *
 *                                                                          *
 * 09/20/2004 [JWW] - Fixed bug in SQL statement that caused it to never    *
 *                    actually checks if the email address is already in db *
 *                                                                          */
void
nanny_change_email (DESCRIPTOR_DATA * d, char *argument)
{
    MYSQL_RES *result;
    char buf[MAX_STRING_LENGTH];
    int nEmailMatches = 0;

    if (*argument)
    {

        if (strstr (argument, " ") || strstr (argument, ";")
                || strstr (argument, "\\") || strstr (argument, "("))
        {
            SEND_TO_Q ("\n#1Your input contains illegal characters.#0\n", d);
            display_main_menu (d);
            return;
        }

        mysql_safe_query
        ("SELECT user_email FROM forum_users WHERE user_email = '%s' and username != '%s'",
         argument, d->acct->name.c_str ());

        if ((result = mysql_store_result (database)) != NULL)
        {
            nEmailMatches = mysql_num_rows (result);
            mysql_free_result (result);
            result = NULL;
        }
        else
        {
            sprintf (buf, "Warning: nanny_change_email(): %s",
                     mysql_error (database));
            system_log (buf, true);
            SEND_TO_Q
            ("\n#1An error occurred.#0\nYour email address was NOT updated in our server.\n",
             d);
            display_main_menu (d);
            return;
        }

        if (nEmailMatches)
        {
            /// \todo LOG THESE

            SEND_TO_Q
            ("\n#1We're sorry, but that email address has already been registered\n"
             "under an existing game account. Please choose another.#0\n\n",
             d);
            SEND_TO_Q ("Your email address was NOT updated in our server.\n",
                       d);
            display_main_menu (d);
            return;
        }

        sprintf (buf, "\nIs the address %s correct? [y/n] ", argument);
        SEND_TO_Q (buf, d);

        d->stored = str_dup (argument);

        d->connected = CON_CHG_EMAIL_CNF;
        return;
    }
    else
    {
        SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
        display_main_menu (d);
        return;
    }
}

void
nanny_change_email_confirm (DESCRIPTOR_DATA * d, char *argument)
{
    argument[0] = toupper (argument[0]);

    if (*argument == 'Y')
    {
        SEND_TO_Q ("\nYour email address was successfully updated.\n", d);
        d->acct->update_email (d->stored);
        mem_free (d->stored); // char*
        d->stored = NULL;
        display_main_menu (d);
        return;
    }
    else
    {
        SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
        display_main_menu (d);
        return;
    }
}

char *
role_flag_descs (int bitflag)
{
    if (bitflag == EXTRA_COIN)
        return "Extra Starting Coin";
    else if (bitflag == APPRENTICE)
        return "Apprenticeship";
    else if (bitflag == STARTING_ARMOR)
        return "Leather Armor";
    else if (bitflag == SKILL_BONUS)
        return "Distributed Skill Bonuses";
    else if (bitflag == EXTRA_SKILL)
        return "Extra Skill Choice";
    else if (bitflag == MAXED_STAT)
        return "Maximized Attribute";
    else if (bitflag == JOURNEYMAN)
        return "Journeymanship";
    else if (bitflag == FELLOW)
        return "Fellowship";
    else if (bitflag == LESSER_NOBILITY)
        return "Lesser Nobility";
    else if (bitflag == APPRENTICE_MAGE)
        return "Arcane Apprenticeship";

    return "Unknown Role Selection";
}

#define ADDBUF	buf + strlen (buf)

void
spitstat (CHAR_DATA * ch, DESCRIPTOR_DATA * recipient)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH] = { '\0' };
    char buf4[MAX_STRING_LENGTH] = { '\0' };
    bool missing_info = false;
    int req = 0;
    bool no_eyes = false;

    sprintf (buf2, "%d inches", ch->height);

    sprintf (buf, "\nName: %s", ch->tname);
    pad_buffer (buf, 25);
    sprintf (ADDBUF, "Account: %s", ch->pc->account_name);
    pad_buffer (buf, 50);
    sprintf (ADDBUF, "Race: %s", lookup_race_variable (ch->race, RACE_NAME));
    SEND_TO_Q (buf, recipient);
    *buf = '\0';

    sprintf (ADDBUF, "\nGender: %s", sex_types[ch->sex]);
    pad_buffer (buf, 25);
    sprintf (ADDBUF, "Age: %d\n\n", ch->age);
    SEND_TO_Q (buf, recipient);
    *buf = '\0';

    /*
    if (!ch->name || !*ch->name || !str_cmp (ch->name, "(null)"))
    {
        sprintf (ADDBUF, "\n#1Keywords:#0 %s\n", ch->name);
        missing_info = true;
    }
    else
        sprintf (ADDBUF, "\nKeywords: %s\n", ch->name);
    */

    if (ch->d_feat1 && ch->d_feat2)
    {
        if (ch->race == lookup_race_id("Cybernetic"))
        {
            for (int i = 0; i < NUM_CYBERNETIC; i++)
            {
                if (!str_cmp(ch->d_feat1, cybernetic_list[i].sdesc) && (!str_cmp(cybernetic_list[i].category, "face") || !str_cmp(cybernetic_list[i].category, "eyes")))
                {
                    no_eyes = true;
                    break;
                }
                if (!str_cmp(ch->d_feat2, cybernetic_list[i].sdesc) && (!str_cmp(cybernetic_list[i].category, "face") || !str_cmp(cybernetic_list[i].category, "eyes")))
                {
                    no_eyes = true;
                    break;
                }
            }
        }
        else if ( ch->race == lookup_race_id("Mutation"))
        {
        for (int i = 0; i < NUM_MUTATION; i++)
            {
                if (!str_cmp(ch->d_feat1, mutation_list[i].sdesc) && (!str_cmp(mutation_list[i].category, "face") || !str_cmp(mutation_list[i].category, "eyes")))
                {
                    no_eyes = true;
                    break;
                }
                else if (!str_cmp(ch->d_feat2, mutation_list[i].sdesc) && (!str_cmp(mutation_list[i].category, "face") || !str_cmp(mutation_list[i].category, "eyes")))
                {
                    no_eyes = true;
                    break;
                }
            }
        }
    }

    if (!ch->d_eyes || !*ch->d_eyes || !str_cmp (ch->d_eyes, "(null)") || ch->height == 0)
    {
        sprintf(ADDBUF, "#1Build and Eyes: Type Features to assign a height, build, and eyecolor.#0\n\n");
        missing_info = true;
    }
    else if (no_eyes)
    {
        sprintf(ADDBUF, "Build: %s height and %s build\n\n", ch->height == 1 ? "tiny" : ch->height == 2 ? "short" : ch->height == 3 ? "average" : ch->height == 4 ? "tall" : ch->height == 5 ? "giant" : buf2, frames[ch->frame]);
    }
    else
    {
        sprintf(ADDBUF, "Build and Eyes: %s height and %s build, with %s eyes\n\n", ch->height == 1 ? "tiny" : ch->height == 2 ? "short" : ch->height == 3 ? "average" : ch->height == 4 ? "tall" : ch->height == 5 ? "giant" : buf2, frames[ch->frame], ch->d_eyes);
    }

    if (!ch->short_descr || !*ch->short_descr
            || !str_cmp (ch->short_descr, "(null)"))
    {
        sprintf (ADDBUF, "#1Short Description:#0 %s\n", ch->short_descr);
        missing_info = true;
    }
    else
        sprintf (ADDBUF, "Short Description: %s\n", ch->short_descr);
    if (!ch->long_descr || !*ch->long_descr
            || !str_cmp (ch->long_descr, "(null)"))
    {
        sprintf (ADDBUF, "#1Long Description:#0  %s\n\n", ch->long_descr);
        missing_info = true;
    }
    else
        sprintf (ADDBUF, "Long Description:  %s\n\n", ch->long_descr);


    if (ch->race == lookup_race_id("Cybernetic"))
    {

        if (ch->d_feat1 && ch->d_feat2 && str_cmp(ch->d_feat1, " ") && str_cmp(ch->d_feat2, " "))
        {
            sprintf (ADDBUF, "Major Cybernetics: %s and %s\n\n", ch->d_feat1, ch->d_feat2);
        }
        else
        {
            sprintf (ADDBUF, "#1Major Cybernetics: Information Missing#0\n\n");
            missing_info = true;
        }

    }
    else if (ch->race == lookup_race_id("Mutation"))
    {

        if (ch->d_feat1 && ch->d_feat2 && str_cmp(ch->d_feat1, " ") && str_cmp(ch->d_feat2, " "))
        {
            sprintf (ADDBUF, "Major Mutations: %s and %s\n\n", ch->d_feat1, ch->d_feat2);
        }
        else
        {
            sprintf (ADDBUF, "#1Major Mutations: Information Missing#0\n\n");
            missing_info = true;
        }

    }

    else
    {
        if (ch->d_height && ch->d_frame && ch->d_hairlength && ch->d_haircolor && ch->d_hairstyle && ch->d_feat1 && ch->d_feat2 && ch->d_feat3 && ch->d_feat4 &&
                str_cmp(ch->d_height, " ") && str_cmp(ch->d_frame, " ")  && str_cmp(ch->d_hairlength, " ")  && str_cmp(ch->d_haircolor, " ")  && str_cmp(ch->d_hairstyle, " ") &&
                str_cmp(ch->d_feat1, " ")  && str_cmp(ch->d_feat2, " ")  && str_cmp(ch->d_feat3, " ")  && str_cmp(ch->d_feat4, " ") )
        {
            sprintf (ADDBUF, "Features: %s height, %s frame, with ", ch->d_height, ch->d_frame);

            if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
                sprintf (ADDBUF, "%s %s %s hair.\n", ch->d_hairlength, ch->d_haircolor, ch->d_hairstyle);
            else
                sprintf (ADDBUF, " a %s head.\n", ch->d_hairlength);

            sprintf (ADDBUF, "Marks:    %s, %s, %s and %s.\n\n", ch->d_feat1, ch->d_feat2, ch->d_feat3, ch->d_feat4);
        }
    }


    SEND_TO_Q (buf, recipient);

    if (!ch->description || !*ch->description
            || !str_cmp (ch->description, "(null)"))
    {
        SEND_TO_Q ("#1Physical Description:#0\n", recipient);
        missing_info = true;
    }
    else
        SEND_TO_Q ("Physical Description:\n", recipient);

    if (ch->description)
    {
        SEND_TO_Q (ch->description, recipient);
    }

    *buf = '\0';

    if (!ch->pc)
        CREATE (ch->pc, PC_DATA, 1);

    if (ch->pc->role)
        sprintf (ADDBUF, "\nPurchased Starter: %s\n",
                 role_flag_descs (ch->pc->role));

    if (ch->pc->special_role && IS_MORTAL (recipient->character))
        sprintf (ADDBUF, "\nSelected Role:\n%s", ch->pc->special_role->body);
    else if (ch->pc->special_role && !IS_MORTAL (recipient->character))
        sprintf (ADDBUF, "Selected Role: %s\n", ch->pc->special_role->summary);

    if (!IS_MORTAL (recipient->character))
    {
        if (ch->pc->app_cost)
            sprintf (ADDBUF, "Application Cost: %d RP Points\n",
                     ch->pc->app_cost);
        if (ch->pc->special_role)
            req =
                MAX (ch->pc->special_role->cost,
                     atoi (lookup_race_variable (ch->race, RACE_RPP_COST)));
        else
            req = atoi (lookup_race_variable (ch->race, RACE_RPP_COST));
        if (req)
            sprintf (ADDBUF, "Application Tier: %d RP Points\n", req);
    }

    SEND_TO_Q (buf, recipient);

    *buf = '\0';

    sprintf (ADDBUF, "\nChosen Profession: %s\n",
             get_profession_name (ch->pc->profession));

    if (!ch->pc->creation_comment || !*ch->pc->creation_comment
            || (ch->pc->creation_comment
                && !str_cmp (ch->pc->creation_comment, "(null)")))
    {
        sprintf (ADDBUF, "\n#1Background Comment:#0\n%s",
                 ch->pc->creation_comment);
        missing_info = true;
    }
    else
        sprintf (ADDBUF, "\nBackground Comment:\n%s", ch->pc->creation_comment);

    if (missing_info && IS_MORTAL (recipient->character))
        sprintf (ADDBUF,
                 "\n#1Items marked in red MUST be completed before you submit your application.#0\n");
    else if (missing_info && !IS_MORTAL (recipient->character))
        sprintf (ADDBUF,
                 "\n#1Items marked in red were not completed by the applicant.#0\n");
    if (*buf)
        SEND_TO_Q (buf, recipient);

}

void
create_menu_options (DESCRIPTOR_DATA * d)
{
    CHAR_DATA *ch = d->character;

    if (!ch->tname)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "NAME");
        return;
    }

    if (ch->pc->nanny_state == STATE_GENDER)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "SEX");
        return;
    }

    else if (ch->pc->nanny_state == STATE_SPECIAL_ROLES)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "CLASSIFIEDS");
        return;
    }

    else if (ch->pc->nanny_state == STATE_RACE)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "RACE");
        return;
    }

    else if (ch->pc->nanny_state == STATE_LOCATION)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "LOCATION");
        return;
    }

    else if (ch->pc->nanny_state == STATE_AGE)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "AGE");
        return;
    }

    else if (ch->pc->nanny_state == STATE_ATTRIBUTES)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "ATTRIBUTES");
        return;
    }

    else if (ch->pc->nanny_state == STATE_FRAME)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "HEIGHT");
        return;
    }

    else if (ch->pc->nanny_state == STATE_SDESC)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "SHORT");
        return;
    }
    /*
    else if (ch->pc->nanny_state == STATE_KEYWORDS)
      {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "KEYWORDS");
        return;
      }
    */
    else if (ch->pc->nanny_state == STATE_LDESC)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "LONG");
        return;
    }

    else if (ch->pc->nanny_state == STATE_FDESC)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "DESCRIPTION");
        return;
    }

    else if (ch->pc->nanny_state == STATE_SKILLS)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "SKILLS");
        return;
    }

    else if (ch->pc->nanny_state == STATE_PROFESSION)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "PROFESSION");
        return;
    }


    else if (ch->pc->nanny_state == STATE_AUTO_DESC)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "FEATURESX");
        return;
    }


    else if (ch->pc->nanny_state == STATE_COMMENT)
    {
        ch->pc->create_state = STATE_APPLYING;
        create_menu_actions (d, "COMMENT");
        return;
    }
    /*
    	else if ( ch->pc->nanny_state == STATE_PRIVACY ) {
    		ch->pc->create_state = STATE_APPLYING;
    		create_menu_actions (d, "PRIVACY");
    		return;
    	}
    */
    if (ch->pc->create_state == STATE_SUBMITTED)
    {
        SEND_TO_Q ("\nCommands:  QUIT, CHECK\n\n> ", d);
        d->connected = CON_CREATION;
        return;
    }

	spitstat (ch, d);

	save_char (ch, false);

	SEND_TO_Q
		("\nCommands:  Age, Attributes, Classifieds, Comment, Description, Frame, Features, Height,\n"
		"           Location, Long, Profession, Quit, Race, Sex, Short, and Skills.\n",
		d);

	if (d->character->pc->msg && *d->character->pc->msg != '~'
		&& strlen (d->character->pc->msg) > 3)
	{
		SEND_TO_Q
			("\n#6Note:      Your application has been processed; type REVIEW to read the response.#0\n",
			d);
    }

    SEND_TO_Q
    ("\n#2Note:      When ready, please use SUBMIT to finalize your application.\n#0",
     d);

    SEND_TO_Q ("\n> ", d);

    d->connected = CON_CREATION;
}

void
attribute_priorities (DESCRIPTOR_DATA * d, char *arg)
{
    int bonus;
    int attr;
    int i;
    CHAR_DATA *ch = d->character;
    int attr_starters[] = { 16, 14, 12, 12, 10, 10, 8 };
    int attr_averages[] = { 12, 12, 12, 12, 12, 10, 12 };
    int attr_priorities[] = { -1, -1, -1, -1, -1, -1, -1 };
    char buf[MAX_STRING_LENGTH];
    char msg[MAX_STRING_LENGTH];
    bool average = false;

    ch->str = 0;

    for (i = 0; i < 6; i++)
    {
        arg = one_argument (arg, buf);

        if (!*buf)
        {
            SEND_TO_Q ("\n#2Please enter all attributes in descending "
                       "priority.\nExample:  STR DEX"
                       " CON WIL INT AGI#0\n", d);
            d->connected = CON_CREATION;
            return;
        }

        if (!str_cmp(buf, "average") || !str_cmp(buf, "avg"))
        {
            average = true;
            break;
        }

        attr = index_lookup (attrs, buf);

        if (attr == -1)
        {
            sprintf (msg, "\n#1'%s' is not recognized as an attribute.\n#0",
                     buf);
            SEND_TO_Q (msg, d);
            return;
        }

        // We're not worrying about aura for now.

        if (attr == 5)
        {
            sprintf (msg, "\n#1'%s' is not recognized as an attribute.\n#0",
                     buf);
            SEND_TO_Q (msg, d);
            return;
        }

        if (attr_priorities[attr] != -1)
        {
            sprintf (msg, "The attribute '%s' is duplicated in your list.\n",
                     buf);
            SEND_TO_Q (msg, d);
            return;
        }

        attr_priorities[attr] = i;
    }

    if (average)
    {
        for (bonus = 6; bonus;)
        {
            attr = number (0, 6);

            while (attr == 5)
                attr = number (0, 6);

            if (attr_averages[attr] < 14)
            {
                attr_averages[attr]++;
                bonus--;
            }
        }

        /* Assign actual numbers */
        ch->str = attr_averages[0];
        ch->dex = attr_averages[1];
        ch->con = attr_averages[2];
        ch->wil = attr_averages[3];
        ch->intel = attr_averages[4];
        ch->aur = 10; // Give them a base rate of aura for now.
        ch->agi = attr_averages[6];

    }
    else
    {

        // Put aura dead last.
        attr_priorities[5] = 7;

        for (bonus = 4; bonus;)
        {
            attr = number (0, 6);

            while (attr == 5)
                attr = number (0, 6);

            if (attr_starters[attr_priorities[attr]] < 18)
            {
                attr_starters[attr_priorities[attr]]++;
                bonus--;
            }
        }

        /* Assign actual numbers */
        ch->str = attr_starters[attr_priorities[0]];
        ch->dex = attr_starters[attr_priorities[1]];
        ch->con = attr_starters[attr_priorities[2]];
        ch->wil = attr_starters[attr_priorities[3]];
        ch->intel = attr_starters[attr_priorities[4]];
        ch->aur = 10; // Give them a base rate of aura for now.
        ch->agi = attr_starters[attr_priorities[6]];
    }

    ch->pc->start_str = ch->str;
    ch->pc->start_dex = ch->dex;
    ch->pc->start_con = ch->con;
    ch->pc->start_wil = ch->wil;
    ch->pc->start_intel = ch->intel;
    ch->pc->start_aur = ch->aur;
    ch->pc->start_agi = ch->agi;

    /* Reset skills */
    // Edited this out, because professions has been moved before
    // attribute assignment.
    //for (skill = 1; skill < MAX_SKILLS; skill++)
    //  {
    //    d->character->skills[skill] = 0;
    //    d->character->pc->skills[skill] = 0;
    //  }
}

void
sex_selection (DESCRIPTOR_DATA * d, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch = d->character;

    arg = one_argument (arg, buf);

    if (toupper (*buf) == 'M' || !str_cmp (buf, "male"))
        ch->sex = SEX_MALE;
    else if (toupper (*buf) == 'F' || !str_cmp (buf, "female"))
        ch->sex = SEX_FEMALE;
    else
        SEND_TO_Q ("Please choose MALE or FEMALE.\n", d);
}

void
race_selection_screen (DESCRIPTOR_DATA * d)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int i = 0, j = 0;
    char b_buf[MAX_STRING_LENGTH];

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "race_select"), d);
    *b_buf = '\0';

    mysql_safe_query
    ("SELECT * FROM races WHERE pc_race = true ORDER BY name ASC");
    result = mysql_store_result (database);
    if (!result || !mysql_num_rows (result))
    {
        if (result != NULL)
            mysql_free_result (result);
        SEND_TO_Q ("No races defined in database - bypassing...\n", d);
        d->character->race = 0;
        d->connected = CON_CREATION;
        d->character->pc->nanny_state = STATE_AGE;
        create_menu_options (d);
        return;
    }

    i = 0;
    j = 0;

    while ((row = mysql_fetch_row (result)))
    {
        i++;
        j++;

		if (!str_cmp(row[RACE_NAME], "Cybernetic") || !str_cmp(row[RACE_NAME], "Mutation"))
		{
			sprintf (b_buf + strlen (b_buf), "%2d.  #2%-20.20s#0", i,
                 !is_newbie(d->acct->name.c_str()) ? row[RACE_NAME] : "");
		}
		else
		{
			sprintf (b_buf + strlen (b_buf), "%2d.  #2%-20.20s#0", i,
                 d->acct->get_rpp () >=
                 atoi (row[RACE_RPP_COST]) ? row[RACE_NAME] : "");
		}


        if (!(j % 3))
            sprintf (b_buf + strlen (b_buf), "\n");
    }

    if ((j % 3))
        sprintf (b_buf + strlen (b_buf), "\n");

    strcat (b_buf, "\nYour Desired Race: ");
    page_string (d, b_buf);
    d->character->race = -1;
    d->connected = CON_RACE_SELECT;
    mysql_free_result (result);
}

void
nanny_race_confirm (DESCRIPTOR_DATA * d, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0, start_loc = 0;

    if (!*arg)
    {
        sprintf (buf, "\nDo you still wish to play a %s? [y/n] ",
                 lookup_race_variable (d->character->race, RACE_NAME));
        SEND_TO_Q (buf, d);
        return;
    }

    arg[0] = toupper (arg[0]);

    if (*arg == 'Y')
    {
        for (i = 1; i <= LAST_SKILL; i++)
        {
            d->character->skills[i] = 0;
            d->character->pc->skills[i] = 0;
        }

        d->connected = CON_CREATION;

        start_loc = num_starting_locs (d->character->race);

        if (start_loc > 1)
            d->character->pc->nanny_state = STATE_LOCATION;
        else
            d->character->pc->nanny_state = STATE_AGE;

        create_menu_options (d);

        if (atoi (lookup_race_variable (d->character->race, RACE_RPP_COST)) > 0)
            d->character->pc->app_cost += atoi (lookup_race_variable (d->character->race, RACE_RPP_COST));
        if (atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE)) > d->character->age)
            d->character->age = atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE));
        if (atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE)) < d->character->age)
            d->character->age = atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE));

        return;
    }
    else
    {
        race_selection_screen (d);
        return;
    }
}

void
nanny_privacy_confirm (DESCRIPTOR_DATA * d, char *arg)
{

    if (!*arg)
    {
        SEND_TO_Q
        ("\n#2Do you wish to flag your application as private? [y/n]#0 ", d);
        return;
    }

    arg[0] = toupper (arg[0]);

    if (*arg == 'Y')
    {
        d->character->plr_flags |= PRIVATE;
    }
    else
    {
        d->character->plr_flags &= ~PRIVATE;
    }

    d->character->pc->nanny_state = 0;

    create_menu_options (d);
}

void
nanny_char_name_confirm (DESCRIPTOR_DATA * d, char *arg)
{
    CHAR_DATA *tch;

    //Uncomment to lock off character creation during wizlock

    if (engine.in_play_mode() && maintenance_lock)
      {
        SEND_TO_Q ("\n#2The sever is currently locked - please feel free to log on as a guest.#0\n", d);
        unload_pc (d->character);
        d->character = 0;
        display_main_menu (d);
        return;
      }
    
    if (!*arg)
    {
        SEND_TO_Q ("\n#2Character generation aborted.#0\n", d);
        unload_pc (d->character);
        d->character = 0;
        display_main_menu (d);
        return;
    }

    if (!strncasecmp (d->acct->name.c_str (), arg, d->acct->name.length ())
            || !strncasecmp (arg, d->acct->name.c_str (), strlen (arg)))
    {
        if (arg[strlen (arg) - 1] != '!')
        {
            SEND_TO_Q
            ("\n#1Your character name may not be similar to your account name.#0\n\n#1Please press ENTER to continue.#0\n",
             d);
            d->connected = CON_PLAYER_NEW;
            return;
        }
        else
        {
            arg[strlen (arg) - 1] = 0;
        }
    }

    if ((tch = load_pc (arg)))
    {
        SEND_TO_Q
        ("\n#1That character name is already in use. Please press ENTER to continue.#0\n",
         d);
        d->connected = CON_PLAYER_NEW;
        unload_pc (tch);
        return;
    }

    if (strlen (arg) > MAX_NAME_LENGTH)
    {
        /* TODO: output value of MAX_NAME_LENGTH instead of 15 */
        SEND_TO_Q
        ("Please choose a character name of less than 15 characters.\n", d);
        d->connected = CON_PLAYER_NEW;
        return;
    }

    for (size_t i = 0; i < strlen (arg); i++)
    {
        if (!isalpha (arg[i]))
        {
            SEND_TO_Q
            ("'Illegal characters in character name (letters only, please).\n",
             d);
            d->connected = CON_PLAYER_NEW;
            return;
        }

        if (i)
            arg[i] = tolower (arg[i]);
        else
            arg[i] = toupper (arg[i]);
    }

    if (!*arg)
    {
        SEND_TO_Q ("What would you like to name your new character? ", d);
        d->connected = CON_PLAYER_NEW;
        return;
    }

    SEND_TO_Q ("\n#2Your character has been named. Press ENTER to continue.#0",
               d);

    arg[0] = toupper (arg[0]);
    d->character = new_char (1);
    //clear_char (d->character);
    d->character->tname = add_hash (arg);
    d->character->pc->create_state = STATE_APPLYING;
    d->character->race = -1;
    d->character->pc->account_name = add_hash (d->acct->name.c_str ());

    d->character->descriptor = d;

    d->character->short_descr = 0;
    d->character->long_descr = 0;
    d->character->description = 0;

    d->character->time.birth = time (0);
    d->character->time.played = 0;
    d->character->time.logon = time (0);

    d->character->armor = 0;

    d->character->affected_by = 0;

    d->character->intoxication = 0;
    d->character->thirst = 300;
    d->character->hunger = 40;

    d->character->pc->load_count = 1;
    save_char (d->character, false);

    d->character->pc->nanny_state = STATE_GENDER;
    d->connected = CON_RACE;
}

void
nanny_special_role_selection (DESCRIPTOR_DATA * d, char *arg)
{
    ROLE_DATA *role;
    char buf[MAX_STRING_LENGTH];
    int i, j;

    if (!*arg || !isdigit (*arg) || atoi (arg) < 0)
    {
        SEND_TO_Q
        ("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n",
         d);
        SEND_TO_Q ("Your Desired Role: ", d);
        return;
    }

    if (atoi (arg) == 0)
    {
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_RACE;
        d->connected = CON_CREATION;
        if (d->character->pc->special_role)
        {
            d->character->pc->special_role = NULL;
        }
        create_menu_options (d);
        return;
    }

    i = atoi (arg);

    for (j = 1, role = role_list; role; role = role->next, j++)
    {
        if (role->cost > (d->acct->get_rpp ()))
        {
            j--;
            continue;
        }
        if (j == i)
            break;
    }

    if (!role)
    {
        SEND_TO_Q
        ("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n",
         d);
        SEND_TO_Q ("Your Desired Role: ", d);
        return;
    }

    CREATE (d->character->pc->special_role, ROLE_DATA, 1);
    d->character->pc->special_role->summary = str_dup (role->summary);
    d->character->pc->special_role->body = str_dup (role->body);
    d->character->pc->special_role->poster = str_dup (role->poster);
    d->character->pc->special_role->date = str_dup (role->date);
    d->character->pc->special_role->cost = role->cost;
    d->character->pc->special_role->id = role->id;

    SEND_TO_Q ("\n", d);
    sprintf (buf, "#2Role Contact:#0  %s\n", role->poster);
    sprintf (buf + strlen (buf), "#2Posted On:#0     %s\n\n", role->date);
    SEND_TO_Q (buf, d);
    SEND_TO_Q (role->body, d);
    SEND_TO_Q ("\n", d);
    SEND_TO_Q ("Do you still wish to choose this role? [y/n] ", d);
    d->connected = CON_SPECIAL_ROLE_CONFIRM;
    return;
}

void
nanny_special_role_confirm (DESCRIPTOR_DATA * d, char *arg)
{
    int i;
    char b_buf[MAX_STRING_LENGTH];
    ROLE_DATA *role;

    if (!*arg)
    {
        SEND_TO_Q ("Do you still wish to choose this role? [y/n] ", d);
        return;
    }

    arg[0] = toupper (arg[0]);

    if (*arg == 'Y')
    {
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_RACE;
        d->connected = CON_CREATION;
        create_menu_options (d);
        return;
    }
    else
    {
        SEND_TO_Q ("\n", d);
        *b_buf = '\0';
        SEND_TO_Q (get_text_buffer (NULL, text_list, "special_role_select"), d);
        role = role_list;
        for (role = role_list, i = 1; role; role = role->next, i++)
        {
            if (!role)
                break;
            if (role->cost > (d->acct->get_rpp ()))
            {
                i--;
                continue;
            }
            if (i < 10)
                sprintf (b_buf + strlen (b_buf), "%d.  #2%s#0\n", i,
                         role->summary);
            else
                sprintf (b_buf + strlen (b_buf), "%d. #2%s#0\n", i,
                         role->summary);
        }
        strcat (b_buf, "\nYour Desired Role: ");
        page_string (d, b_buf);
        d->connected = CON_SPECIAL_ROLE_SELECT;
        return;
    }
}

void
race_selection (DESCRIPTOR_DATA * d, char *arg)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int i = 0, abilities = 0;
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char buf2[MAX_STRING_LENGTH];
    char race_name[MAX_STRING_LENGTH];
    char *p;

    mysql_safe_query
    ("SELECT * FROM races WHERE pc_race = true ORDER BY name ASC");
    result = mysql_store_result (database);

    *race_name = '\0';

    d->character->plr_flags &= ~(START_GONDOR | START_MORDOR);

    while ((row = mysql_fetch_row (result)))
    {
        i++;
        if (i == atoi (arg))
        {
            sprintf (race_name, "%s", row[RACE_NAME]);
            break;
        }
    }

    if (!*race_name)
    {
        SEND_TO_Q ("Please select the number of one of the above races.\n", d);
        SEND_TO_Q ("\nYour Desired Race: ", d);
        return;
    }


	if ((!str_cmp(row[RACE_NAME], "Cybernetic") || !str_cmp(row[RACE_NAME], "Mutation")) && is_newbie(d->acct->name.c_str()))
	{
        SEND_TO_Q ("That race is currently unavailable to you.\n", d);
        SEND_TO_Q ("\nYour Desired Race: ", d);
		return;
	}
    else if (atoi (row[RACE_RPP_COST]) > d->acct->get_rpp ())
    {
        SEND_TO_Q ("That race is currently unavailable to you.\n", d);
        SEND_TO_Q ("\nYour Desired Race: ", d);
        return;
    }

    d->character->race = atoi (row[RACE_ID]);

    sprintf (buf, "%s", row[RACE_DESC]);
    if (*buf && strlen (buf) > 1)
    {
        reformat_string (buf, &p);
        sprintf (buf2, "\n#2%s:#0\n\n%s", row[RACE_NAME], p);
        mem_free (p); //char*
    }
    else
    {
        sprintf (buf2, "\n#2%s:#0\n\nNo description provided.\n",
                 row[RACE_NAME]);
    }

    if ((abilities = atoi (row[RACE_AFFECTS])) > 0)
    {
        sprintf (buf2 + strlen (buf2), "\n#2Innate Abilities:#0");
        if (IS_SET (abilities, INNATE_INFRA))
            sprintf (buf2 + strlen (buf2), " Infravision");
        if (IS_SET (abilities, INNATE_FLYING))
            sprintf (buf2 + strlen (buf2), " Flight");
        if (IS_SET (abilities, INNATE_WAT_BREATH))
            sprintf (buf2 + strlen (buf2), " WaterBreathing");
        if (IS_SET (abilities, INNATE_NOBLEED))
            sprintf (buf2 + strlen (buf2), " NoBleed");
        if (IS_SET (abilities, INNATE_SUN_PEN))
            sprintf (buf2 + strlen (buf2), " DaylightPenalty");
        if (IS_SET (abilities, INNATE_SUN_PET))
            sprintf (buf2 + strlen (buf2), " DaylightPetrification");
        sprintf (buf2 + strlen (buf2), "\n");
    }

    *buf = '\0';

    if (atoi (row[RACE_STR_MOD]) != 0)
    {
        if (atoi (row[RACE_STR_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Str");
        else
            sprintf (buf + strlen (buf), " -Str");
    }

    if (atoi (row[RACE_CON_MOD]) != 0)
    {
        if (atoi (row[RACE_CON_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Con");
        else
            sprintf (buf + strlen (buf), " -Con");
    }

    if (atoi (row[RACE_DEX_MOD]) != 0)
    {
        if (atoi (row[RACE_DEX_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Dex");
        else
            sprintf (buf + strlen (buf), " -Dex");
    }

    if (atoi (row[RACE_AGI_MOD]) != 0)
    {
        if (atoi (row[RACE_AGI_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Agi");
        else
            sprintf (buf + strlen (buf), " -Agi");
    }

    if (atoi (row[RACE_INT_MOD]) != 0)
    {
        if (atoi (row[RACE_INT_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Int");
        else
            sprintf (buf + strlen (buf), " -Int");
    }

    if (atoi (row[RACE_WIL_MOD]) != 0)
    {
        if (atoi (row[RACE_WIL_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Wil");
        else
            sprintf (buf + strlen (buf), " -Wil");
    }

    if (atoi (row[RACE_AUR_MOD]) != 0)
    {
        if (atoi (row[RACE_AUR_MOD]) > 0)
            sprintf (buf + strlen (buf), " +Aur");
        else
            sprintf (buf + strlen (buf), " -Aur");
    }

    if (*buf)
        sprintf (buf2 + strlen (buf2), "\n#2Attribute Modifiers:#0%s\n", buf);

    sprintf (buf2 + strlen (buf2), "\nDo you still wish to play a %s? [y/n] ",
             race_name);

    page_string (d, buf2);

    d->connected = CON_RACE_CONFIRM;

    mysql_free_result (result);
}

void
age_selection (DESCRIPTOR_DATA * d, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch = d->character;
    int lower = 0, upper = 0;

    d->character->age = 0;

    arg = one_argument (arg, buf);
    d->character->pc->nanny_state = STATE_AGE;

    if (lookup_race_variable (d->character->race, RACE_MIN_AGE))
        lower = atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE));
    else
        lower = 18;

    if (lookup_race_variable (d->character->race, RACE_MAX_AGE))
        upper = atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE));
    else
        upper = 55;

    if (atoi (buf) < lower || atoi (buf) > upper)
    {
        SEND_TO_Q ("\n#1Please select an age within the specified range.#0\n",
                   d);
        d->connected = CON_AGE;
        return;
    }

    ch->age = atoi (buf);
}

int
available_roles (int points)
{
    ROLE_DATA *role;

    for (role = role_list; role; role = role->next)
    {
        if (role->cost <= points)
            return 1;
    }

    return 0;
}

void
location_selection (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, buf);

    if (!*buf || !isdigit (*buf) || (atoi (buf) < 1 || atoi (buf) > 2))
    {
        SEND_TO_Q ("#2Please select a number from the list above.#0\n", d);
        return;
    }

    if (atoi (buf) == 2)
        d->character->plr_flags |= START_MORDOR;
    else
        d->character->plr_flags |= START_GONDOR;

    if (d->character->pc->nanny_state)
        d->character->pc->nanny_state = STATE_AGE;

    d->connected = CON_CREATION;
}

void
height_frame_selection (DESCRIPTOR_DATA * d, char *argument)
{
    int ind;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        SEND_TO_Q ("#2Enter a frame and height, e.g. 'tall light'.#0\n", d);
        d->character->height = 0;
        return;
    }

    if (!str_cmp (buf, "tiny"))
        d->character->height = 1;
    else if (!str_cmp (buf, "short"))
        d->character->height = 2;
    else if (!str_cmp (buf, "average"))
        d->character->height = 3;
    else if (!str_cmp (buf, "tall"))
        d->character->height = 4;
    else if (!str_cmp (buf, "giant"))
        d->character->height = 5;
    else
    {
        SEND_TO_Q ("#2\nEnter 'height frame', e.g. 'tall light'.#0\n", d);
        d->character->height = 0;
        return;
    }

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        SEND_TO_Q ("#2\nEnter a frame after the height, e.g. 'tall light'.#0\n",
                   d);
        d->character->height = 0;
        return;
    }

    ind = index_lookup (frames, buf);

    if (ind < 0 || ind > FRAME_SUPER_MASSIVE)
    {
        SEND_TO_Q
        ("Valid frames are feather, scant, light, medium, heavy, massive and mammoth.\n", d);
        d->character->height = 0;
        return;
    }

    d->character->frame = ind;

    int i;
    CHAR_DATA *ch = NULL;

    ch = d->character;

    // Because we go back here is we re-do it all, we need to clear out everything and start afresh.
    if (ch->delay_who)
        mem_free(ch->delay_who);
    if (ch->delay_who2)
        mem_free(ch->delay_who2);
    if (ch->delay_who3)
        mem_free(ch->delay_who3);

    if (ch->description)
        mem_free(ch->description);
    ch->description = 0;
    if (ch->short_descr)
        mem_free(ch->short_descr);
    ch->short_descr = 0;
    if (ch->long_descr)
        mem_free(ch->long_descr);
    ch->long_descr = 0;

    if (ch->d_age)
        mem_free(ch->d_age);
    if (ch->d_eyes)
        mem_free(ch->d_eyes);
    if (ch->d_hairlength)
        mem_free(ch->d_hairlength);
    if (ch->d_haircolor)
        mem_free(ch->d_haircolor);
    if (ch->d_hairstyle)
        mem_free(ch->d_hairstyle);
    if (ch->d_height)
        mem_free(ch->d_height);
    if (ch->d_frame)
        mem_free(ch->d_frame);
    if (ch->d_feat1)
        mem_free(ch->d_feat1);
    if (ch->d_feat1)
        mem_free(ch->d_feat2);
    if (ch->d_feat1)
        mem_free(ch->d_feat3);
    if (ch->d_feat1)
        mem_free(ch->d_feat4);

    ch->d_age = 0;
    ch->d_eyes = 0;
    ch->d_hairlength = 0;
    ch->d_haircolor = 0;
    ch->d_hairstyle = 0;
    ch->d_height = 0;
    ch->d_frame = 0;
    ch->d_feat1 = 0;
    ch->d_feat2 = 0;
    ch->d_feat3 = 0;
    ch->d_feat4 = 0;

    ch->delay_info1 = 0;
    ch->delay_info2 = 0;
    ch->delay_info3 = 0;
    ch->delay_info4 = 0;
    ch->delay_info5 = 0;

    ch = d->character;

    if (ch->age >= 56)
        ch->delay_info5 = AGE_AGED;
    else if (ch->age >= 46)
        ch->delay_info5 = AGE_MATURE;
    else if (ch->age >= 26)
        ch->delay_info5 = AGE_ADULT;
    else if (ch->age >= 20)
        ch->delay_info5 = AGE_YOUNG;
    else if (ch->age >= 13)
        ch->delay_info5 = AGE_TEEN;
    else
        ch->delay_info5 = AGE_CHILD;

    if (ch->sex == SEX_FEMALE)
        ch->d_age = str_dup(fem_ages[ch->delay_info5][0]);
    else
        ch->d_age = str_dup(male_ages[ch->delay_info5][0]);

    d->connected = CON_DESC_EYE;
    d->character->pc->nanny_state = STATE_AUTO_DESC;
    return;
}

int
pickable_skill (CHAR_DATA * ch, const char *buf, int cmd)
{
    int ind;

    if ((ind = index_lookup (skills, buf)) == -1)
        return 0;

    if (is_restricted_skill (ch, ind, cmd) == 0)
        return ind;

    return 0;
}

int
picks_entitled (CHAR_DATA * ch)
{
    if (ch->pc && IS_SET (ch->pc->role, EXTRA_SKILL))
        return 7;
    else
        return 6;
}

/*                                                                          *
 * function: autodesc_display                                               *
 *                                                                          */

void
autodesc_eyes (DESCRIPTOR_DATA * d)
{
    int i = 0, j = 0;
    char buf[MAX_STRING_LENGTH];

    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_eyes"), d);

    *buf = '\0';

    i = 0;

    for (int ind = 0; ind < 6; ind ++)
    {
        sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), eye_colors[ind][0]);
    }

    sprintf (buf + strlen (buf), "\n\nEnter the number of your desired eye color: ");
    SEND_TO_Q (buf, d);
    d->connected = CON_DESC_EYE_SPECIFY;

    return;
}

void
autodesc_eyes_specific (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0, j = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 6 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    d->character->delay_info1 = 0;

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      d->connected = CON_DESC_FRAME;
      autodesc_frame (d, d->character->delay_who4);
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 6)
    {
        sprintf (buf, "\nPlease select a number from 1 to 6 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;

    d->character->delay_info1 = j;

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_eyes_specific"), d);

    *buf = '\0';

    for (int ind = 1; ind < 10; ind ++)
    {
        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, eye_colors[j][ind]);
        else
            sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, eye_colors[j][ind]);

        if (!(ind % 4))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired eye color:  ", d);

    if (d->character->race == lookup_race_id("Cybernetic") || d->character->race == lookup_race_id("Mutation"))
    {
        d->connected = CON_DESC_F1_UNHUMAN;
    }
    else
    {
        d->connected = CON_DESC_END;
    }
    return;
}

void
autodesc_feat_one_unhuman (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    CHAR_DATA *ch = NULL;
    int i = 0, j = 0, max = 0, ind = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      d->connected = CON_DESC_EYE;
      autodesc_eyes (d, d->character->delay_who4);
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 9)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    d->character->d_eyes = str_dup(eye_colors[d->character->delay_info1][j]);

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_unhuman"), d);

    *buf = '\0';
    *buf2 = '\0';

    if (d->character->race == lookup_race_id("Cybernetic"))
    {

        for (int i = 0; i < NUM_CYBERNETIC; i ++)
        {
            if (str_cmp(buf2, cybernetic_list[i].category))
            {
                ind++;
                sprintf(buf2, "%s", cybernetic_list[i].category);

                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);

                if (!(ind % 4))
                    sprintf (buf + strlen (buf), "\n");
            }
        }
    }
    else
    {
        for (int i = 0; i < NUM_MUTATION; i ++)
        {
            if (str_cmp(buf2, mutation_list[i].category))
            {
                ind++;
                sprintf(buf2, "%s", mutation_list[i].category);

                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);

                if (!(ind % 4))
                    sprintf (buf + strlen (buf), "\n");
            }
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);

    d->connected = CON_DESC_F1_SPECIFY_UNHUMAN;
    return;
}

void
autodesc_feat_one_specify_unhuman  (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 5;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    d->character->delay_info1 = 0;

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    *buf2 = '\0';

    // OK, run through the feature list.

    if (d->character->race == lookup_race_id("Cybernetic"))
    {

        for (int i = 0; i < NUM_CYBERNETIC; i ++)
        {
            // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
            if (str_cmp(buf2, cybernetic_list[i].category))
            {
                sprintf(buf2, "%s", cybernetic_list[i].category);
                ind++;
            }
            // If ind equals the category we picked, and we haven't set our way point, we set our start point.
            if (ind == j && x == -1)
            {
                x = i;
            }
            // If the next variable would not be of the same category, we set our end point and break the loop.
            if (x != -1 && str_cmp(cybernetic_list[i+1].category, buf2))
            {
                y = i;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < NUM_MUTATION; i ++)
        {
            // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
            if (str_cmp(buf2, mutation_list[i].category))
            {
                sprintf(buf2, "%s", mutation_list[i].category);
                ind++;
            }
            // If ind equals the category we picked, and we haven't set our way point, we set our start point.
            if (ind == j && x == -1)
            {
                x = i;
            }
            // If the next variable would not be of the same category, we set our end point and break the loop.
            if (x != -1 && str_cmp(mutation_list[i+1].category, buf2))
            {
                y = i;
                break;
            }
        }
    }

    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify_unhuman"), d);

    *buf = '\0';

    if (d->character->race == lookup_race_id("Cybernetic"))
    {

        for (i = x; i <= y; i++)
        {
            ind++;

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, cybernetic_list[i].sdesc);
            else
                sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, cybernetic_list[i].sdesc);

            if (!(ind % 2))
                sprintf (buf + strlen (buf), "\n");
        }
    }
    else
    {
        for (i = x; i <= y; i++)
        {
            ind++;

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, mutation_list[i].sdesc);
            else
                sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, mutation_list[i].sdesc);

            if (!(ind % 2))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_F2_UNHUMAN;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}



void
autodesc_feat_two_unhuman (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;


    if (d->character->race == lookup_race_id("Cybernetic"))
    {
        d->character->d_feat1 = str_dup(cybernetic_list[j + d->character->delay_info2].sdesc);
        d->character->delay_who = str_dup(cybernetic_list[j + d->character->delay_info2].category);
    }
    else
    {
        d->character->d_feat1 = str_dup(mutation_list[j + d->character->delay_info2].sdesc);
        d->character->delay_who = str_dup(mutation_list[j + d->character->delay_info2].category);

    }

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_unhuman"), d);

    *buf = '\0';
    *buf2 = '\0';


    if (d->character->race == lookup_race_id("Cybernetic"))
    {
        for (int i = 0; i < NUM_CYBERNETIC; i ++)
        {
            if (str_cmp(buf2, cybernetic_list[i].category))
            {
                ind++;
                sprintf(buf2, "%s", cybernetic_list[i].category);

                if (!str_cmp(buf2, d->character->delay_who))
                {
                    if (ind < 10)
                        sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", ind, buf2);
                    else
                        sprintf (buf + strlen (buf), "%d. #1%-12s#0    ", ind, buf2);
                }
                else
                {
                    if (ind < 10)
                        sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                    else
                        sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);
                }

                if (!(ind % 4))
                    sprintf (buf + strlen (buf), "\n");
            }
        }
    }
    else
    {
        for (int i = 0; i < NUM_MUTATION; i ++)
        {
            if (str_cmp(buf2, mutation_list[i].category))
            {
                ind++;
                sprintf(buf2, "%s", mutation_list[i].category);

                if (!str_cmp(buf2, d->character->delay_who))
                {
                    if (ind < 10)
                        sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", ind, buf2);
                    else
                        sprintf (buf + strlen (buf), "%d. #1%-12s#0    ", ind, buf2);
                }
                else
                {
                    if (ind < 10)
                        sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                    else
                        sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);
                }

                if (!(ind % 4))
                    sprintf (buf + strlen (buf), "\n");
            }
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);



    d->connected = CON_DESC_F2_SPECIFY_UNHUMAN;
    return;
}

void
autodesc_feat_two_specify_unhuman (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 5;
    d->character->delay_info1 = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    *buf2 = '\0';

    // OK, run through the feature list.

    if (d->character->race == lookup_race_id("Cybernetic"))
    {
        for (int i = 0; i < NUM_CYBERNETIC; i ++)
        {
            // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
            if (str_cmp(buf2, cybernetic_list[i].category))
            {
                sprintf(buf2, "%s", cybernetic_list[i].category);
                ind++;
            }
            // If ind equals the category we picked, and we haven't set our way point, we set our start point.
            if (ind == j && x == -1)
            {
                x = i;
            }
            // If the next variable would not be of the same category, we set our end point and break the loop.
            if (x != -1 && str_cmp(cybernetic_list[i+1].category, buf2))
            {
                y = i;
                break;
            }
        }

        if (!str_cmp(cybernetic_list[x].category, d->character->delay_who))
        {
            sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who);
            SEND_TO_Q (buf, d);
            return;
        }
    }
    else
    {
        for (int i = 0; i < NUM_MUTATION; i ++)
        {
            // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
            if (str_cmp(buf2, mutation_list[i].category))
            {
                sprintf(buf2, "%s", mutation_list[i].category);
                ind++;
            }
            // If ind equals the category we picked, and we haven't set our way point, we set our start point.
            if (ind == j && x == -1)
            {
                x = i;
            }
            // If the next variable would not be of the same category, we set our end point and break the loop.
            if (x != -1 && str_cmp(mutation_list[i+1].category, buf2))
            {
                y = i;
                break;
            }
        }

        if (!str_cmp(mutation_list[x].category, d->character->delay_who))
        {
            sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who);
            SEND_TO_Q (buf, d);
            return;
        }
    }

    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify_unhuman"), d);

    *buf = '\0';

    if (d->character->race == lookup_race_id("Cybernetic"))
    {

        for (i = x; i <= y; i++)
        {
            ind++;

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, cybernetic_list[i].sdesc);
            else
                sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, cybernetic_list[i].sdesc);

            if (!(ind % 2))
                sprintf (buf + strlen (buf), "\n");
        }
    }
    else
    {
        for (i = x; i <= y; i++)
        {
            ind++;

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, mutation_list[i].sdesc);
            else
                sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, mutation_list[i].sdesc);

            if (!(ind % 2))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_END_UNHUMAN;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}




void
autodesc_end_unhuman (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;

    if (d->character->race == lookup_race_id("Cybernetic"))
    {
        d->character->d_feat2 = str_dup(cybernetic_list[j + d->character->delay_info2].sdesc);
        d->character->delay_who2 = str_dup(cybernetic_list[j + d->character->delay_info2].category);
    }
    else
    {
        d->character->d_feat2 = str_dup(mutation_list[j + d->character->delay_info2].sdesc);
        d->character->delay_who2= str_dup(mutation_list[j + d->character->delay_info2].category);

    }

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_end_unhuman"), d);
    SEND_TO_Q ("\n", d);

    if (d->character->race == lookup_race_id("Cybernetic"))
    {
        for (int i = 0; i < NUM_CYBERNETIC; i++)
        {
            if (!str_cmp(d->character->d_feat1, cybernetic_list[i].sdesc))
            {
                sprintf(buf2, cybernetic_list[i].fdesc);
                break;
            }
        }

        sprintf(buf, "The cybernetics you've selected have the following basic descriptions:\n"
                "#2%s#0: %s\n"
                "#2%s#0: %s\n", d->character->d_feat1, buf2, d->character->d_feat2, cybernetic_list[j + d->character->delay_info2].fdesc);

    }
    else
    {
        for (int i = 0; i < NUM_MUTATION; i++)
        {
            if (!str_cmp(d->character->d_feat1, mutation_list[i].sdesc))
            {
                sprintf(buf2, mutation_list[i].fdesc);
                break;
            }
        }

        sprintf(buf, "The mutations you've selected have the following basic descriptions:\n\n"
                "#2%s#0: %s\n"
                "#2%s#0: %s\n", d->character->d_feat1, buf2, d->character->d_feat2, mutation_list[j + d->character->delay_info2].fdesc);
    }


    SEND_TO_Q (buf, d);
    SEND_TO_Q ("#0", d);
    SEND_TO_Q("\nEnter your choice - #9#61)#0 Custom, #9#62)#0 Redo: ", d);

    *buf = '\0';
    *buf2 = '\0';

    d->connected = CON_DESC_ONE;
    return;
};


void
autodesc_end (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    CHAR_DATA *ch = NULL;
    int i = 0, j = 0, max = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      d->connected = CON_DESC_EYE;
      autodesc_eyes (d, d->character->delay_who4);
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 9)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired eye color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    d->character->d_eyes = str_dup(eye_colors[d->character->delay_info1][j]);

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_end"), d);

    SEND_TO_Q ("#0", d);
    SEND_TO_Q("\nEnter your choice - #9#61)#0 Custom, #9#62)#0 Assisted, #9#63)#0 Redo: ", d);

    *buf = '\0';
    *buf2 = '\0';

    d->connected = CON_DESC_ONE;
    return;
};


void
autodesc_height (DESCRIPTOR_DATA * d)
{
    int i;
    CHAR_DATA *ch = NULL;
    char buf[MAX_STRING_LENGTH];

    ch = d->character;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_height"), d);

    *buf = '\0';

    i = 0;

    for (int ind = 0; ind < 5; ind ++)
    {
        sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), heights[(ch->height - 1)][ind]);
    }

    sprintf (buf + strlen (buf), "\n\nEnter the number of your desired height descriptor: ");
    SEND_TO_Q (buf, d);
    d->connected = CON_DESC_FRAME;

    return;
}



void
autodesc_result (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    CHAR_DATA *ch = NULL;
    int j = 0, ind = 0, max = 0;

    ch = d->character;

    max = 3;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo") || (atoi(argument) == 3))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 1 || ((d->character->race == lookup_race_id("Cybernetic") || d->character->race == lookup_race_id("Mutation")) && j != 2))
    {
        d->character->pc->nanny_state = STATE_SDESC;
        d->connected = CON_CREATION;
        create_menu_options(d);
        return;
    }
	else if ((d->character->race == lookup_race_id("Cybernetic") || d->character->race == lookup_race_id("Mutation")) && j == 2)
	{
        create_menu_actions (d, "HEIGHT");
        return;
	}
	else if (j == 2)
	{
		d->connected = CON_DESC_HEIGHT;
		autodesc_height (d);
		return;
	}
    else
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    return;
};
void
autodesc_frame (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int j = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 5 or enter #6REDO#0.\n\nEnter the number of your desired height descriptor: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      create_menu_actions (d, "HEIGHT");
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 5)
    {
        sprintf (buf, "\nPlease select a number from 1 to 5 or enter #6REDO#0.\n\nEnter the number of your desired height descriptor: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;

    d->character->d_height = str_dup(heights[(d->character->height - 1)][j]);

    int fitness = d->character->str + d->character->con;

    if (fitness >= 31)
        d->character->delay_info4 = 2;
    else if (fitness <= 22)
        d->character->delay_info4 = 0;
    else
        d->character->delay_info4 = 1;

    int frames = d->character->frame;
    frames -= 1;
    frames = MAX(0, frames);
    frames = MIN(4, frames);

    d->character->delay_info3 = frames;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_frame"), d);

    *buf = '\0';

    for (int ind = 0; ind < 5; ind ++)
    {
        if (d->character->delay_info4 == 2)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), fit_frames[frames][ind]);
        else if (d->character->delay_info4 == 1)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), avg_frames[frames][ind]);
        else
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), unfit_frames[frames][ind]);
    }

    sprintf (buf + strlen (buf), "\n\nEnter the number of your desired frame descriptor: ");
    SEND_TO_Q (buf, d);
    d->connected = CON_DESC_LENGTH;

    if (d->character->delay_who4)
        mem_free(d->character->delay_who4);
    d->character->delay_who4 = str_dup(argument);

    return;
}

void
autodesc_length (DESCRIPTOR_DATA * d, char *argument)
{
    int i = 0, j = 0;
    char buf[MAX_STRING_LENGTH];

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 5 or enter #6REDO#0.\n\nEnter the number of your desired frame descriptor: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      d->connected = CON_DESC_HEIGHT;
      autodesc_height (d);
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 5)
    {
        sprintf (buf, "\nPlease select a number from 1 to 5 or enter #6REDO#0.\n\nEnter the number of your desired frame descriptor: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;

    if (d->character->delay_info4 == 2)
        d->character->d_frame = str_dup(fit_frames[d->character->delay_info3][j]);
    else if (d->character->delay_info4 == 1)
        d->character->d_frame = str_dup(avg_frames[d->character->delay_info3][j]);
    else
        d->character->d_frame = str_dup(unfit_frames[d->character->delay_info3][j]);

    d->character->delay_info4 = 0;
    d->character->delay_info3 = 0;


    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_length"), d);

    *buf = '\0';

    for (int ind = 0; ind < 7; ind ++)
    {
        if (!ind && (d->character->sex == SEX_FEMALE || d->character->delay_info5 <= AGE_TEEN))
            sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", (ind + 1), hair_lengths[ind]);
        else if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), hair_lengths[ind]);
        else
            sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", (ind + 1), hair_lengths[ind]);

        if (!((ind+1) % 4))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired hair length:  ", d);

    if (d->character->delay_who4)
        mem_free(d->character->delay_who4);
    d->character->delay_who4 = str_dup(argument);

    d->connected = CON_DESC_COLOR;
    return;
}

void
autodesc_color (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0, j = 0, max = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 7 or enter #6REDO#0.\n\nEnter the number of your desired hair length: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    /*
    else if (!str_cmp(argument, "UNDO") || !str_cmp(argument, "undo"))
    {
      d->connected = CON_DESC_EYE;
      autodesc_eyes (d, d->character->delay_who4);
      return;
    }
    */
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 7)
    {
        sprintf (buf, "\nPlease select a number from 1 to 7 or enter #6REDO#0.\n\nEnter the number of your desired hair length: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;

    if (j == 0 && (d->character->sex == SEX_FEMALE || d->character->delay_info5 <= AGE_TEEN))
    {
        sprintf (buf, "\nYoung and female characters are unable to be completely bald.\n\nEnter the number of your desired hair length: ");
        SEND_TO_Q (buf, d);
        return;
    }

    d->character->d_hairlength = str_dup(hair_lengths[j]);
    //send_to_gods(d->character->d_hairlength);

    if (!j)
    {
        d->connected = CON_DESC_F1;
        d->character->d_haircolor = str_dup("black");
        d->character->d_hairstyle = str_dup("wild");
        autodesc_feat_one(d, "bald");
        return;
    }

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_color"), d);

    *buf = '\0';

    if (d->character->delay_info5 >= AGE_MATURE)
        max = 5;
    else
        max = 4;

    for (int ind = 0; ind < max; ind ++)
    {
        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", (ind + 1), hair_colors[ind][0]);
        else
            sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", (ind + 1), hair_colors[ind][0]);

        if (!((ind+1) % 4))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\nEnter the number of your desired hair color:  ", d);

    if (d->character->delay_who4)
        mem_free(d->character->delay_who4);
    d->character->delay_who4 = str_dup(argument);

    d->connected = CON_DESC_COLOR_SPECIFY;
    return;
}

void
autodesc_color_specific (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0, j = 0, max = 0;

    if (d->character->delay_info5 >= AGE_MATURE)
        max = 5;
    else
        max = 4;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired hair color: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    d->character->delay_info1 = 0;

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired hair color: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;

    d->character->delay_info1 = j;

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_color_specific"), d);

    *buf = '\0';

    for (int ind = 1; ind < 10; ind ++)
    {
        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, hair_colors[j][ind]);
        else
            sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, hair_colors[j][ind]);

        if (!(ind % 4))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired hair color:  ", d);

    d->connected = CON_DESC_STYLE;
    return;
}

void
autodesc_style (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0, j = 0, ind = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired hair color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > 9)
    {
        sprintf (buf, "\nPlease select a number from 1 to 9 or enter #6REDO#0.\n\nEnter the number of your desired hair color: ");
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    d->character->d_haircolor = str_dup(hair_colors[d->character->delay_info1][j]);
    //send_to_gods(d->character->d_haircolor);

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_style"), d);

    *buf = '\0';


    if (str_cmp(d->character->d_hairlength, "shaved"))
    {
        for (int i = 2; i < 7; i ++)
        {
            for (int h = 0; h < 5; h ++)
            {
                ind++;

                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, hair_styles[i][h]);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, hair_styles[i][h]);

                if (!(ind % 4))
                    sprintf (buf + strlen (buf), "\n");
            }
        }

        d->character->delay_info1 = 25;
        d->character->delay_info2 = 2;
    }
    else
    {
        for (int h = 0; h < 5; h ++)
        {
            ind++;

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, hair_styles[1][h]);
            else
                sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, hair_styles[1][h]);

            if (!(ind % 4))
                sprintf (buf + strlen (buf), "\n");
        }

        d->character->delay_info1 = 5;
        d->character->delay_info2 = 1;
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired hair style:  ", d);

    d->connected = CON_DESC_F1;
    return;
}

void
autodesc_feat_one (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, h = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!str_cmp(argument, "bald"))
        ;
    else if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired hair style: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired hair style: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (!str_cmp(argument, "bald"))
        d->character->d_hairstyle = str_dup("bald");
    else if (d->character->delay_info2 == 1)
        d->character->d_hairstyle = str_dup(hair_styles[1][j - 1]);
    else
    {
        j += 2 * 5;
        h = j % 5;
        j = j - (h * 5);
        j -= 1;
        d->character->d_hairstyle = str_dup(hair_styles[h][j]);
    }

    //send_to_gods(d->character->d_hairstyle);

    i = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat"), d);

    *buf = '\0';
    *buf2 = '\0';

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        if (str_cmp(buf2, feature_list[i].category))
        {
            ind++;
            sprintf(buf2, "%s", feature_list[i].category);

            if (ind < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
            else
                sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);

            if (!(ind % 4))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);

    d->connected = CON_DESC_F1_SPECIFY;
    return;
}

void
autodesc_feat_one_specify  (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 16;


    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    d->character->delay_info1 = 0;

    if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 3 && (d->character->sex != SEX_MALE || d->character->delay_info5 <= AGE_TEEN))
    {
        sprintf (buf, "\nOnly male characters of a certain age may have facial hair.\n\nEnter the number of your desired feature category: ");
        SEND_TO_Q (buf, d);
        return;
    }

    *buf2 = '\0';

    // OK, run through the feature list.

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
        if (str_cmp(buf2, feature_list[i].category))
        {
            sprintf(buf2, "%s", feature_list[i].category);
            ind++;
        }
        // If ind equals the category we picked, and we haven't set our way point, we set our start point.
        if (ind == j && x == -1)
        {
            x = i;
        }
        // If the next variable would not be of the same category, we set our end point and break the loop.
        if (x != -1 && str_cmp(feature_list[i+1].category, buf2))
        {
            y = i;
            break;
        }
    }

    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify"), d);

    *buf = '\0';

    for (i = x; i <= y; i++)
    {
        ind++;

        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, feature_list[i].sdesc);
        else
            sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, feature_list[i].sdesc);

        if (!(ind % 2))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_F2;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}


void
autodesc_feat_two (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;

    d->character->d_feat1 = str_dup(feature_list[j + d->character->delay_info2].sdesc);
    d->character->delay_who = str_dup(feature_list[j + d->character->delay_info2].category);

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat"), d);

    *buf = '\0';
    *buf2 = '\0';

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        if (str_cmp(buf2, feature_list[i].category))
        {
            ind++;
            sprintf(buf2, "%s", feature_list[i].category);

            if (!str_cmp(buf2, d->character->delay_who))
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #1%-12s#0    ", ind, buf2);
            }
            else
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);
            }

            if (!(ind % 4))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);



    d->connected = CON_DESC_F2_SPECIFY;
    return;
}

void
autodesc_feat_two_specify (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 16;
    d->character->delay_info1 = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 3 && (d->character->sex != SEX_MALE || d->character->delay_info5 <= AGE_TEEN))
    {
        sprintf (buf, "\nOnly male characters of a certain age may have facial hair.\n\nEnter the number of your desired feature category: ");
        SEND_TO_Q (buf, d);
        return;
    }

    *buf2 = '\0';

    // OK, run through the feature list.

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
        if (str_cmp(buf2, feature_list[i].category))
        {
            sprintf(buf2, "%s", feature_list[i].category);
            ind++;
        }
        // If ind equals the category we picked, and we haven't set our way point, we set our start point.
        if (ind == j && x == -1)
        {
            x = i;
        }
        // If the next variable would not be of the same category, we set our end point and break the loop.
        if (x != -1 && str_cmp(feature_list[i+1].category, buf2))
        {
            y = i;
            break;
        }
    }

    if (!str_cmp(feature_list[x].category, d->character->delay_who))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who);
        SEND_TO_Q (buf, d);
        return;
    }


    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify"), d);

    *buf = '\0';

    for (i = x; i <= y; i++)
    {
        ind++;

        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, feature_list[i].sdesc);
        else
            sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, feature_list[i].sdesc);

        if (!(ind % 2))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_F3;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}

void
autodesc_feat_three (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;

    d->character->d_feat2 = str_dup(feature_list[j + d->character->delay_info2].sdesc);
    d->character->delay_who2 = str_dup(feature_list[j + d->character->delay_info2].category);

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat"), d);

    *buf = '\0';
    *buf2 = '\0';

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        if (str_cmp(buf2, feature_list[i].category))
        {
            ind++;
            sprintf(buf2, "%s", feature_list[i].category);

            if (!str_cmp(buf2, d->character->delay_who) || !str_cmp(buf2, d->character->delay_who2))
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #1%-12s#0    ", ind, buf2);
            }
            else
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);
            }

            if (!(ind % 4))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);

    d->connected = CON_DESC_F3_SPECIFY;
    return;
}

void
autodesc_feat_three_specify (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 16;
    d->character->delay_info1 = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 3 && (d->character->sex != SEX_MALE || d->character->delay_info5 <= AGE_TEEN))
    {
        sprintf (buf, "\nOnly male characters of a certain age may have facial hair.\n\nEnter the number of your desired feature category: ");
        SEND_TO_Q (buf, d);
        return;
    }

    *buf2 = '\0';

    // OK, run through the feature list.

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
        if (str_cmp(buf2, feature_list[i].category))
        {
            sprintf(buf2, "%s", feature_list[i].category);
            ind++;
        }
        // If ind equals the category we picked, and we haven't set our way point, we set our start point.
        if (ind == j && x == -1)
        {
            x = i;
        }
        // If the next variable would not be of the same category, we set our end point and break the loop.
        if (x != -1 && str_cmp(feature_list[i+1].category, buf2))
        {
            y = i;
            break;
        }
    }

    if (!str_cmp(feature_list[x].category, d->character->delay_who))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(feature_list[x].category, d->character->delay_who2))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who2);
        SEND_TO_Q (buf, d);
        return;
    }


    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify"), d);

    *buf = '\0';

    for (i = x; i <= y; i++)
    {
        ind++;

        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, feature_list[i].sdesc);
        else
            sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, feature_list[i].sdesc);

        if (!(ind % 2))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_F4;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}


void
autodesc_feat_four (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, ind = 0, max = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;

    d->character->d_feat3 = str_dup(feature_list[j + d->character->delay_info2].sdesc);
    d->character->delay_who3 = str_dup(feature_list[j + d->character->delay_info2].category);

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat"), d);

    *buf = '\0';
    *buf2 = '\0';

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        if (str_cmp(buf2, feature_list[i].category))
        {
            ind++;
            sprintf(buf2, "%s", feature_list[i].category);

            if (!str_cmp(buf2, d->character->delay_who) || !str_cmp(buf2, d->character->delay_who2) || !str_cmp(buf2, d->character->delay_who3))
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #1%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #1%-12s#0    ", ind, buf2);
            }
            else
            {
                if (ind < 10)
                    sprintf (buf + strlen (buf), "%d.  #2%-12s#0    ", ind, buf2);
                else
                    sprintf (buf + strlen (buf), "%d. #2%-12s#0    ", ind, buf2);
            }

            if (!(ind % 4))
                sprintf (buf + strlen (buf), "\n");
        }
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature category:  ", d);

    d->connected = CON_DESC_F4_SPECIFY;
    return;
}


void
autodesc_feat_four_specify (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    int i = 0, j = 0, max = 0, x = -1, y = -1, ind = 0;

    max = 16;
    d->character->delay_info1 = 0;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature category: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 3 && (d->character->sex != SEX_MALE || d->character->delay_info5 <= AGE_TEEN))
    {
        sprintf (buf, "\nOnly male characters of a certain age may have facial hair.\n\nEnter the number of your desired feature category: ");
        SEND_TO_Q (buf, d);
        return;
    }

    *buf2 = '\0';

    // OK, run through the feature list.

    for (int i = 0; i < NUM_FEATURES; i ++)
    {
        // If buf2 doesn't equal the category, we write buf2 to the category and increase i by 1.
        if (str_cmp(buf2, feature_list[i].category))
        {
            sprintf(buf2, "%s", feature_list[i].category);
            ind++;
        }
        // If ind equals the category we picked, and we haven't set our way point, we set our start point.
        if (ind == j && x == -1)
        {
            x = i;
        }
        // If the next variable would not be of the same category, we set our end point and break the loop.
        if (x != -1 && str_cmp(feature_list[i+1].category, buf2))
        {
            y = i;
            break;
        }
    }

    if (!str_cmp(feature_list[x].category, d->character->delay_who))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(feature_list[x].category, d->character->delay_who2))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who2);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(feature_list[x].category, d->character->delay_who3))
    {
        sprintf (buf, "\nYou have already selected a feature from category %s.\n\nEnter the number of your desired feature category: ", d->character->delay_who3);
        SEND_TO_Q (buf, d);
        return;
    }


    i = 0;
    ind = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_feat_specify"), d);

    *buf = '\0';

    for (i = x; i <= y; i++)
    {
        ind++;

        if (ind < 10)
            sprintf (buf + strlen (buf), "%d.  #2%-20s#0    ", ind, feature_list[i].sdesc);
        else
            sprintf (buf + strlen (buf), "%d. #2%-20s#0    ", ind, feature_list[i].sdesc);

        if (!(ind % 2))
            sprintf (buf + strlen (buf), "\n");
    }

    SEND_TO_Q (buf, d);
    SEND_TO_Q("\n\nEnter the number of your desired feature:  ", d);

    d->connected = CON_DESC_ONE_HALF;
    d->character->delay_info1 = ind;
    d->character->delay_info2 = x;

    return;
}

void
autodesc_result_second (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char buf2[AVG_STRING_LENGTH] = {'\0'};
    CHAR_DATA *ch = NULL;
    int i = 0, j = 0, max = 0, ind = 0;

    max = d->character->delay_info1;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument) - 1;
    i = 0;

    ch = d->character;

    ch->d_feat4 = str_dup(feature_list[j + ch->delay_info2].sdesc);

    if (ch->delay_who)
        mem_free(ch->delay_who);
    if (ch->delay_who2)
        mem_free(ch->delay_who2);
    if (ch->delay_who3)
        mem_free(ch->delay_who3);

    ch->delay_info1 = 0;
    ch->delay_info2 = 0;
    ch->delay_info3 = 0;
    ch->delay_info4 = 0;
    ch->delay_info5 = 0;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_choice"), d);

    ind++;
    *buf2 = '\0';
    sprintf (buf2, "%s-eyed", ch->d_eyes);
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);

    ind++;
    *buf2 = '\0';
    if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
    {
        sprintf (buf2, "%s-haired", ch->d_hairlength);
    }
    else
    {
        sprintf (buf2, "%s", ch->d_hairlength);
    }
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);
    sprintf (buf + strlen (buf), "\n");

    if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
    {
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ind++;
        *buf2 = '\0';
        sprintf (buf2, "%s-haired", ch->d_haircolor);
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);

        ind++;
        *buf2 = '\0';
        sprintf (buf2, "%s-haired", ch->d_hairstyle);
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);
        sprintf (buf + strlen (buf), "\n");
    }
    else
    {
        ch->delay_info1 = 1;
        ch->delay_info2 = 2;
    }

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_height);

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_frame);
    sprintf (buf + strlen (buf), "\n");

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat1);

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat2);
    sprintf (buf + strlen (buf), "\n");

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat3);

    ind++;
    sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat4);
    sprintf (buf + strlen (buf), "\n");

    SEND_TO_Q(buf, d);
    SEND_TO_Q("\nEnter the number of your character's most noticable feature: ", d);

    *buf = '\0';
    *buf2 = '\0';
    d->connected = CON_DESC_TWO;
    return;
};

void
autodesc_one_conf (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    char buf3[AVG_STRING_LENGTH];
    CHAR_DATA *ch = NULL;
    int j = 0, ind = 0, max = 0;

    ch = d->character;

    max = 10 - ch->delay_info2;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (ch->delay_info1 && j > 2)
        j += ch->delay_info2;

    *buf3 = '\0';

    switch (j)
    {
    case 1:
        sprintf(buf3, "%s-eyed", ch->d_eyes);
        break;
    case 2:
        if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
            sprintf(buf3, "%s-haired", ch->d_hairlength);
        else
            sprintf(buf3, "%s", ch->d_hairlength);
        break;
    case 3:
        sprintf(buf3, "%s-haired", ch->d_haircolor);
        break;
    case 4:
        sprintf(buf3, "%s-haired", ch->d_hairstyle);
        break;
    case 5:
        sprintf(buf3, "%s", ch->d_height);
        break;
    case 6:
        sprintf(buf3, "%s", ch->d_frame);
        break;
    case 7:
        sprintf(buf3, "%s", ch->d_feat1);
        break;
    case 8:
        sprintf(buf3, "%s", ch->d_feat2);
        break;
    case 9:
        sprintf(buf3, "%s", ch->d_feat3);
        break;
    default:
        sprintf(buf3, "%s", ch->d_feat4);
        break;
    }

    ch->delay_who = str_dup(buf3);


    *buf = '\0';

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_choice2"), d);

    if (!ch->delay_info1 && j > 4)
        j -= ch->delay_info2;

    ch->delay_info3 = j;

    ind++;

    *buf2 = '\0';
    sprintf (buf2, "%s-eyed", ch->d_eyes);

    if (j == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, buf2);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);

    ind++;
    *buf2 = '\0';
    if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
    {
        sprintf (buf2, "%s-haired", ch->d_hairlength);
    }
    else
    {
        sprintf (buf2, "%s", ch->d_hairlength);
    }

    if (j == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, buf2);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);
    sprintf (buf + strlen (buf), "\n");

    if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
    {
        ch->delay_info1 = 0;
        ch->delay_info2 = 0;
        ind++;
        *buf2 = '\0';
        sprintf (buf2, "%s-haired", ch->d_haircolor);

        if (j == ind)
            sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, buf2);
        else
            sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);

        ind++;
        *buf2 = '\0';
        sprintf (buf2, "%s-haired", ch->d_hairstyle);

        if (j == ind)
            sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, buf2);
        else
            sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, buf2);
        sprintf (buf + strlen (buf), "\n");
    }
    else
    {
        ch->delay_info1 = 1;
        ch->delay_info2 = 2;
    }

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_height);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_height);

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_frame);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_frame);
    sprintf (buf + strlen (buf), "\n");

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_feat1);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat1);

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_feat2);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat2);
    sprintf (buf + strlen (buf), "\n");

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_feat3);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat3);

    ind++;
    if ((j - ch->delay_info2) == ind)
        sprintf (buf + strlen (buf), "%d.  #1%-16s#0    ", ind, ch->d_feat4);
    else
        sprintf (buf + strlen (buf), "%d.  #2%-16s#0    ", ind, ch->d_feat4);
    sprintf (buf + strlen (buf), "\n");

    SEND_TO_Q(buf, d);
    SEND_TO_Q("\nEnter the number of your character's second most noticable feature: ", d);

    d->connected = CON_DESC_THREE;
    return;
};


void
autodesc_two_conf (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[AVG_STRING_LENGTH];
    char buf3[AVG_STRING_LENGTH];
    char sdesc[MAX_STRING_LENGTH] = {'\0'};
    char ldesc[MAX_STRING_LENGTH] = {'\0'};
    CHAR_DATA *ch = NULL;
    int j = 0, max = 0;

    ch = d->character;

    max = 10 - ch->delay_info2;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo"))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }


    j = atoi(argument);

    if (j == ch->delay_info3)
    {
        sprintf (buf, "\nYou can't select the same feature twice!.\n\nEnter the number of your desired feature: ");
        SEND_TO_Q (buf, d);
        return;
    }

    if (ch->delay_info1 && j > 2)
        j += ch->delay_info2;

    *buf3 = '\0';

    switch (j)
    {
    case 1:
        sprintf(buf3, "%s-eyed", ch->d_eyes);
        break;
    case 2:
        if (str_cmp(ch->d_hairlength, "bald") && str_cmp(ch->d_hairlength, "balding"))
            sprintf(buf3, "%s-haired", ch->d_hairlength);
        else
            sprintf(buf3, "%s", ch->d_hairlength);
        break;
    case 3:
        sprintf(buf3, "%s-haired", ch->d_haircolor);
        break;
    case 4:
        sprintf(buf3, "%s-haired", ch->d_hairstyle);
        break;
    case 5:
        sprintf(buf3, "%s", ch->d_height);
        break;
    case 6:
        sprintf(buf3, "%s", ch->d_frame);
        break;
    case 7:
        sprintf(buf3, "%s", ch->d_feat1);
        break;
    case 8:
        sprintf(buf3, "%s", ch->d_feat2);
        break;
    case 9:
        sprintf(buf3, "%s", ch->d_feat3);
        break;
    default:
        sprintf(buf3, "%s", ch->d_feat4);
        break;
    }

    ch->delay_who2 = str_dup(buf3);

    *buf = '\0';
    *buf2 = '\0';

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "autodesc_final"), d);

    if (ch->delay_who[0] == 'a' || ch->delay_who[0] == 'e' || ch->delay_who[0] == 'i' || ch->delay_who[0] == 'o' || ch->delay_who[0] == 'u')
        sprintf(sdesc, "an %s, %s %s", ch->delay_who, ch->delay_who2, ch->d_age);
    else
        sprintf(sdesc, "a %s, %s %s", ch->delay_who, ch->delay_who2, ch->d_age);

    if (ch->delay_who[0] == 'a' || ch->delay_who[0] == 'e' || ch->delay_who[0] == 'i' || ch->delay_who[0] == 'o' || ch->delay_who[0] == 'u')
        sprintf(ldesc, "An %s, %s %s is here.", ch->delay_who, ch->delay_who2, ch->d_age);
    else
        sprintf(ldesc, "A %s, %s %s is here.", ch->delay_who, ch->delay_who2, ch->d_age);

    if (ch->delay_who)
        mem_free(ch->delay_who);
    if (ch->delay_who2)
        mem_free(ch->delay_who2);

    new_create_description(ch, ch->d_age, ch->d_height, ch->d_frame, ch->d_eyes, ch->d_hairlength, ch->d_haircolor, ch->d_hairstyle, ch->d_feat1, ch->d_feat2, ch->d_feat3, ch->d_feat4);

    if (ch->short_descr)
        mem_free(ch->short_descr);
    if (ch->long_descr)
        mem_free(ch->long_descr);

    ch->short_descr = str_dup(sdesc);
    ch->long_descr = str_dup(ldesc);

    SEND_TO_Q ("Short Description:", d);
    SEND_TO_Q ("#2   ", d);
    SEND_TO_Q (ch->short_descr, d);
    SEND_TO_Q ("#0", d);

    SEND_TO_Q ("\n\nLong Description:", d);
    SEND_TO_Q ("#2   ", d);
    SEND_TO_Q (ch->long_descr, d);
    SEND_TO_Q ("#0", d);

    SEND_TO_Q ("\n\nFull Description:", d);
    SEND_TO_Q ("\n", d);
    SEND_TO_Q ("#2", d);
    SEND_TO_Q (ch->description, d);
    SEND_TO_Q ("#0", d);
    SEND_TO_Q("\nEnter your choice - #9#61)#0 Redo, #9#62)#0 Customise, #9#63)#0 Accept: ", d);

    *buf = '\0';
    d->connected = CON_DESC_FOUR;
    return;
};


void
autodesc_three_conf (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int j = 0, max = 0;

    max = 3;

    if (!*argument)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }
    else if (!str_cmp(argument, "REDO") || !str_cmp(argument, "redo") || (atoi(argument) == 1))
    {
        create_menu_actions (d, "HEIGHT");
        return;
    }
    else if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf, "\nPlease select a number from 1 to %d or enter #6REDO#0.\n\nEnter the number of your desired feature: ", max);
        SEND_TO_Q (buf, d);
        return;
    }

    j = atoi(argument);

    if (j == 2)
    {
        d->character->pc->nanny_state = STATE_SDESC;
        d->connected = CON_CREATION;
        create_menu_options(d);
        return;
    }

    d->connected = CON_SKILLS;
    d->character->pc->nanny_state = STATE_SKILLS;
    skill_display (d);
    return;
};




/*                                                                          *
 * function: profession_display                                             *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                                                                          */
void
profession_display (DESCRIPTOR_DATA * d)
{
    int i;
    CHAR_DATA *ch;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buf[MAX_STRING_LENGTH];
    int nProfessions = 0;

    ch = d->character;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "professions"), d);

    *buf = '\0';

    mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: profession_display(): %s",
                 mysql_error (database));
        system_log (buf, true);
    }
    else
    {
        nProfessions = mysql_num_rows (result);
    }

    if (nProfessions > 0)
    {

        i = 0;
        while ((row = mysql_fetch_row (result)))
        {
            i++;
            if (i < 10)
                sprintf (buf + strlen (buf), "%d.  #2%-18s#0    ", i,
                         !is_restricted_profession (d->character,
                                                    row[1]) ? row[0] : "");
            else
                sprintf (buf + strlen (buf), "%d. #2%-18s#0    ", i,
                         !is_restricted_profession (d->character,
                                                    row[1]) ? row[0] : "");
            if (!(i % 3))
                sprintf (buf + strlen (buf), "\n");
        }
        if ((i % 3))
            sprintf (buf + strlen (buf), "\n");

        sprintf (buf + strlen (buf), "\nYour Desired Profession: ");
        SEND_TO_Q (buf, d);
        d->connected = CON_PROFESSION;

    }
    else
    {
        SEND_TO_Q
        ("There are no professions currently defined. Continuing...\n\n", d);
        d->connected = CON_CREATION;
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_ATTRIBUTES;
        create_menu_options (d);
    }

    if (result)
    {
        mysql_free_result (result);
        result = NULL;
    }
    return;
}

/*                                                                          *
 * function: profession_selection                                           *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - fixed an instance where result was not freed          *
 *                                                                          */
void
profession_selection (DESCRIPTOR_DATA * d, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int i = 0, j = 0, max = 0;

    if (!*argument)
    {
        profession_display (d);
        return;
    }

    d->character->pc->profession = 0;

    mysql_safe_query ("SELECT count(*) FROM professions");
    if ((result = mysql_store_result (database)) != NULL)
    {
        row = mysql_fetch_row (result);
        max = atoi (row[0]);
        mysql_free_result (result);
        result = NULL;
    }
    else
    {
        sprintf (buf, "Warning: profession_selection(): %s",
                 mysql_error (database));
        system_log (buf, true);
        sprintf (buf,
                 "\nPlease select a number from 1 to %d.\n\nYour Desired Profession: ",
                 max);
        SEND_TO_Q (buf, d);
        return;
    }

    if (isdigit (*argument) && atoi (argument) == 0)
    {
        SEND_TO_Q("\nNo profession selected.\n", d);
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_ATTRIBUTES;
        d->connected = CON_CREATION;
        create_menu_options (d);
        save_char (d->character, false);
        d->character->pc->profession = 0;
        return;
    }

    if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
    {
        sprintf (buf,
                 "\nPlease select a number from 1 to %d, or 0 for none.\n\nYour Desired Profession: ",
                 max);
        SEND_TO_Q (buf, d);
        return;
    }

    mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
    if ((result = mysql_store_result (database)) == NULL)
    {
        sprintf (buf, "Warning: profession_selection(): %s",
                 mysql_error (database));
        system_log (buf, true);
        sprintf (buf, "\nAn error occurred.\n\nYour Desired Profession: ");
        SEND_TO_Q (buf, d);
        return;
    }

    i = 0;

    while ((row = mysql_fetch_row (result)))
    {
        i++;
        if (!is_restricted_profession (d->character, row[1])
                && i == atoi (argument))
        {
            d->character->pc->profession = atoi (row[2]);
            for (j = 0; j <= LAST_SKILL; j++)
            {
                d->character->skills[j] = 0;
                d->character->pc->skills[j] = 0;
            }
            add_profession_skills (d->character, row[1]);
            break;
        }
        else if (is_restricted_profession (d->character, row[1])
                 && i == atoi (argument))
        {
            SEND_TO_Q
            ("\nThat profession is currently unavailable to you.\n\nYour Desired Profession: ",
             d);
            mysql_free_result (result);
            return;
        }
    }


    SEND_TO_Q ("\n", d);
    sprintf (buf, "#2Profession:#0  %s\n\n", get_profession_name(d->character->pc->profession));
    SEND_TO_Q (buf, d);
    SEND_TO_Q (get_profession_desc(d->character->pc->profession), d);
    SEND_TO_Q("\nAre you sure you wish to play this profession?[y/n] ", d);

    mysql_free_result (result);
    result = NULL;

    d->connected = CON_PROFESSION_CONFIRM;
    return;
}

void
profession_confirm (DESCRIPTOR_DATA * d, char *argument)
{
    char b_buf[MAX_STRING_LENGTH];

    if (!*argument)
    {
        SEND_TO_Q ("Do you still wish to choose this profession? [y/n] ", d);
        return;
    }

    argument[0] = toupper (argument[0]);

    if (*argument == 'Y')
    {
        d->connected = CON_PROF_Q1;
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_profession_choice(d->character->pc->profession, 0), d);
        SEND_TO_Q("\nMake your selection [1, 2 or 3]: ", d);
        save_char (d->character, false);
        return;
    }
    else
    {
        d->character->pc->profession = 0;
        strcat (b_buf, "\nYour Desired Profession: ");
        page_string (d, b_buf);
        d->connected = CON_PROFESSION;
        return;
    }

    return;
}

void
profession_first (DESCRIPTOR_DATA * d, char *argument)
{
    int i = 0;

    if (!*argument)
    {
        SEND_TO_Q("\nMake your selection [1, 2 or 3]: ", d);
        return;
    }

    if (!isdigit (*argument) || atoi (argument) == 0)
    {
        SEND_TO_Q("Please enter a digit, either 1, 2, or 3", d);
        return;
    }

    i = atoi (argument);

    if (i > 0 && i < 4)
    {
        add_profession_skills (d->character, get_profession_skills(d->character->pc->profession, 0,i));
        d->connected = CON_PROF_Q2;
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_profession_choice(d->character->pc->profession, 1), d);
        SEND_TO_Q("\nMake your selection [1, 2 or 3]: ", d);
        save_char (d->character, false);
        return;
    }
    else
    {
        d->character->pc->profession = 0;
        SEND_TO_Q ("\nAborting.\n", d);
        SEND_TO_Q ("\nYour Desired Profession: ", d);
        d->connected = CON_PROFESSION;
        return;
    }

    return;
}

void
profession_second (DESCRIPTOR_DATA * d, char *argument)
{
    int i = 0;

    if (!*argument)
    {
        SEND_TO_Q("\nMake your selection [1, 2 or 3]: ", d);
        return;
    }

    if (!isdigit (*argument) || atoi (argument) == 0)
    {
        SEND_TO_Q("Please enter a digit, either 1, 2, or 3", d);
        return;
    }

    i = atoi (argument);

    if (i > 0 && i < 4)
    {
        d->connected = CON_CREATION;
        SEND_TO_Q ("\n", d);
        add_profession_skills (d->character, get_profession_skills(d->character->pc->profession, 1,i));

        if (d->character->pc->nanny_state && d->character->pc->profession > 0)
            attribute_priorities (d, get_profession_attr(d->character->pc->profession));

        if (d->character->pc->nanny_state && d->character->str)
        {
            d->character->pc->nanny_state = STATE_FRAME;
        }
        create_menu_options (d);
        save_char (d->character, false);
        return;
    }
    else
    {
        d->character->pc->profession = 0;
        SEND_TO_Q ("\nAborting.\n", d);
        d->connected = CON_PROFESSION;
        return;
    }

    return;
}

void
skill_selection (DESCRIPTOR_DATA * d, char *argument)
{
    char b_buf[MAX_STRING_LENGTH] = {'\0'};
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char arg[MAX_STRING_LENGTH] = {'\0'};
    int skill;
    int i;
    int picks_left = 0;
    int native_tongue = 0;
    int start_loc = 0;
    CHAR_DATA *ch;
    MYSQL_RES *result = NULL;
    MYSQL_ROW row = NULL;

    ch = d->character;

    picks_left = 8;

    argument = one_argument (argument, buf);

    if (!*buf)
    {
        skill_display (d);
        return;
    }

    for (i = 1; i <= LAST_SKILL && *skills[i] != '\n' && picks_left > 0; i++)
    {
        if (ch->skills[i] && pickable_skill (ch, skills[i], 1))
        {
            picks_left--;
        }
    }


    if (!str_cmp (buf, "help"))
    {
        argument = one_argument (argument, arg);
        if (!(skill = pickable_skill (ch, arg, 0)))
        {
            SEND_TO_Q
            ("\n#2Unknown skill.  Please pick one from the list.#0\n\nPress ENTER to continue...\n",
             d);
            d->connected = CON_SKILLS;
            return;
        }

        mysql_safe_query ("SELECT * FROM helpfiles WHERE name = '%s' AND (category = 'skills' AND required_level <= 0) ORDER BY category,name ASC", arg);

        result = mysql_store_result (database);

        if (!result)
        {
            SEND_TO_Q
            ("No helpfile matching that query was found in the database.\n\n#6Press Enter to continue....#0\n",
             d);
            d->connected = CON_SKILLS;
            return;
        }

        if (!(row = mysql_fetch_row (result)))
        {
            SEND_TO_Q
            ("No helpfile matching that query was found in the database.\n\n#6Press Enter to continue....#0\n",
             d);
            d->connected = CON_SKILLS;
            return;
        }


        if (mysql_num_rows (result) == 1)
        {
            sprintf (b_buf, "\n#6%s: %s#0\n", row[1], row[0]);
            if (*row[2] != '\n')
                strcat (b_buf, "\n");
            sprintf (b_buf + strlen (b_buf), "%s", row[2]);
            sprintf (b_buf + strlen (b_buf), "#6Last Updated:#0 %s, by %s\n", row[5], CAP (row[6]));
            sprintf (b_buf + strlen (b_buf), "#0\nPress ENTER to continue...");
        }

        SEND_TO_Q (b_buf, d);
        d->connected = CON_SKILLS;
        return;
    }


    if (!str_cmp (buf, "done"))
    {
        if (picks_left > 4)
        {
            skill_display (d);
            return;
        }

        /*
        if (ch->skills[SKILL_LITERACY])
        {
            if (!ch->skills[SKILL_SCRIPT_COMMON])
            {
                SEND_TO_Q
                ("\n#2You'll need to pick a script to use with your Literacy skill.#0\n\nPress ENTER to continue...\n",
                 d);
                skill_display (d);
                return;
            }
        }
        */
        d->connected = CON_CREATION;
        if (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE))
        {
            skill = atoi (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE));
            ch->skills[skill] = calc_lookup (ch, REG_CAP, skill);
            ch->pc->skills[skill] = calc_lookup (ch, REG_CAP, skill);
        }
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_COMMENT;
        save_char (ch, false);
        create_menu_options (d);
        return;
    }

    if (!(skill = pickable_skill (ch, buf, 0)))
    {
        SEND_TO_Q
        ("\n#2Unknown skill.  Please pick one from the list.#0\n\nPress ENTER to continue...\n",
         d);
        d->connected = CON_SKILLS;
        return;
    }
    if (!picks_left && !ch->skills[skill])
    {
        SEND_TO_Q
        ("\n#2You've picked too many.  Remove another skill by typing its name.#0\n\nPress ENTER to continue...\n",
         d);
        d->connected = CON_SKILLS;
        return;
    }
    else
    {
        if (ch->skills[skill])
        {
            ch->skills[skill] = 0;
            ch->pc->skills[skill] = 0;

            /*
            if (skill == SKILL_LITERACY)
            {
              ch->skills[SKILL_SCRIPT_COMMON] = 50;
            }
            */
            save_char (ch, false);
        }
        else
        {
            native_tongue =
                strtol (lookup_race_variable
                        (d->character->race, RACE_NATIVE_TONGUE), NULL, 10);
            start_loc =
                strtol (lookup_race_variable (d->character->race, RACE_START_LOC),
                        NULL, 10);

            if (skill == native_tongue || skill == SKILL_COMMON)
            {
                SEND_TO_Q
                ("\n#2This is a default skill for your character, please choose another.#0\n\nPress ENTER to continue...\n",
                 d);
                d->connected = CON_SKILLS;
                return;
            }
            else
            {
                ch->skills[skill] = 1;
                ch->pc->skills[skill] = 1;
                save_char (ch, false);
            }
        }
    }

    d->connected = CON_SKILLS;
    skill_display (d);
}

void
skill_display (DESCRIPTOR_DATA * d)
{
    int i;
    int col = 0;
    int picks;
    int native_tongue = 0;
    int start_loc = 0;
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH];

    ch = d->character;

    SEND_TO_Q ("\n", d);
    SEND_TO_Q (get_text_buffer (NULL, text_list, "skill_select"), d);

    picks = 8;

	if (!d->character->race || d->character->race == -1)
		d->character->race = 5; 

    *buf = '\0';
    native_tongue =
        strtol (lookup_race_variable (d->character->race, RACE_NATIVE_TONGUE),
                NULL, 10);
    start_loc =
        strtol (lookup_race_variable (d->character->race, RACE_START_LOC), NULL,
                10);
    for (i = 1; i <= LAST_SKILL && *skills[i] != '\n'; i++)
    {

        if (!pickable_skill (d->character, skills[i], 0))
            continue;

        if ((i == native_tongue) || ((i == SKILL_COMMON)))
        {
            sprintf (buf + strlen (buf), "    Skill:  %-17s #6(Default)#0",
                     skills[i]);
            d->character->skills[i] = 0;
        }
        else
        {
            sprintf (buf + strlen (buf), "    Skill:  %s%-17s#0 %s#0", color_skill[i], skills[i],
                     d->character->skills[i] ? "#2(Chosen)#0 " : "         ");
        }
        if (d->character->skills[i])
            picks -= 1;

        if (col++ % 2)
            sprintf (buf + strlen (buf), "\n");
    }

    if (buf[strlen (buf) - 1] != '\n')
        strcat (buf, "\n");

    SEND_TO_Q (buf, d);

    if (picks > 1)
        sprintf (buf, "\n%d picks remaining: enter a skill name, or DONE to finsh> ", picks);
    else if (picks == 1)
        sprintf (buf, "\n1 pick remaining: enter a skill name, or DONE to finish> ");
    else
        sprintf (buf, "\nEnter DONE to finish or skill name> ");

    SEND_TO_Q (buf, d);

    d->connected = CON_SKILLS;
}

void
create_menu_actions (DESCRIPTOR_DATA * d, char *arg)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char key[MAX_STRING_LENGTH];
    int i, picks_left = 0, block = 0;
    int start_loc = 0, req = 0;
    MYSQL_RES *result;
    CHAR_DATA *ch = d->character;
    CHAR_DATA *tch;
    ROLE_DATA *role;
    int picked = 0;

    if (ch->pc->create_state == STATE_APPROVED)
    {
        create_menu_options (d);
        return;
    }

    arg = one_argument (arg, key);

    if (!*key)
    {
        SEND_TO_Q ("> ", d);
        return;
    }
    /* A tiny little bit of cleanup on the keywords.  Make sure
       that the player's name is in the keyword list */

    if (ch->short_descr)
        ch->name = str_dup (keyword_gen(ch, ch->short_descr, 0));

    if (ch->name && !isname (ch->tname, ch->name))
    {
        sprintf (buf, "%s %s", ch->tname, ch->name);
        ch->name = str_dup (buf);
    }
    else if (!str_cmp (key, "quit"))
    {
        unload_pc (d->character);
        d->character = NULL;
        display_main_menu (d);
        return;
    }

    else if (!str_cmp (key, "check"))
    {

        /* show_level_5_count (d); */

        if (ch->pc->create_state == STATE_SUBMITTED)
        {
            SEND_TO_Q ("No response, as of yet. Please try back later.\n", d);
            SEND_TO_Q ("> ", d);
        }

        else if (ch->pc->create_state == STATE_APPLYING)
        {
            create_menu_options (d);
            return;
        }

        else if (ch->pc->create_state == STATE_REJECTED)
        {
            create_menu_options (d);
            d->connected = CON_CREATION;
            d->character->pc->create_state = STATE_APPLYING;
            return;
        }

        return;
    }
    /*
    	else if ( !str_cmp (key, "privacy") ) {
    		SEND_TO_Q ("\nThe PRIVACY flag ensures that only administrators are permitted to\n"
    			   "review your application, and not our player Guides. While every\n"
    			   "Guide is a trusted and discreet member of the playerbase, they are\n"
    			   "in fact still players, and hence having them review certain types of\n"
    			   "applications may be overly intrusive. A good example of this would be\n"
    			   "an application for a shadier type, to whom anonymity, both IC and OOC,\n"
    			   "is quite important. However, if your application is NOT marked private,\n"
    			   "this means there is a larger group of people able to review it, and thus\n"
    			   "your wait for a response to it will quite likely be shortened.\n"
    			   "\n"
    			   "Note that if you are spending any roleplay points on this application,\n"
    			   "Guides are automatically excluded from reviewing it.\n", d);
    		SEND_TO_Q ("\nDo you wish to flag your application as private? [y/n] ", d);
    		d->connected = CON_PRIV_CONFIRM;
    		return;
    	}
    */
    else if (!str_cmp (key, "name"))
    {
        SEND_TO_Q ("What would you like to name your character? ", d);
        d->connected = CON_NAME_CONFIRM;
        return;
    }

    else if (ch->pc->create_state == STATE_SUBMITTED)
    {
        SEND_TO_Q
        ("Only the QUIT and CHECK commands function, now. Please hit ENTER.\n",
         d);
        return;
    }

    else if (!str_cmp (key, "review"))
    {
        if (d->character->pc->msg && *d->character->pc->msg != '~'
                && strlen (d->character->pc->msg) > 3
                && d->connected == CON_CREATION)
        {
            sprintf (buf, "%s\n", d->character->pc->msg);
            SEND_TO_Q (buf, d);
            SEND_TO_Q ("> ", d);
            return;
        }
        else
            SEND_TO_Q
            ("\r\nNo response has been filed to your application at this point.\r\n\r\n> ",
             d);
        return;
    }

    else if (!str_cmp (key, "sex"))
    {
        strcpy (b_buf, get_text_buffer (NULL, text_list, "sex_select"));
        strcat (b_buf, "Your Character's Gender: ");
        page_string (d, b_buf);
        d->connected = CON_SEX;
        return;
    }

    else if (!str_cmp (key, "attributes"))
    {
        SEND_TO_Q (get_text_buffer (NULL, text_list, "qstat_message"), d);
        SEND_TO_Q ("Desired Attribute Order: ", d);
        d->connected = CON_ATTRIBUTES;
        return;
    }
    /*
    else if (!str_cmp (key, "keywords"))
      {
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "help_pkeywords"), d);
        sprintf (buf, "%s (null)", ch->tname);
        if (ch->name)
      {
        if (str_cmp (ch->name, buf))
          {
            sprintf (buf, "Replacing:  %s\n", ch->name);
            SEND_TO_Q (buf, d);
          }
        ch->name = NULL;
      }

        d->str = &ch->name;
        d->max_str = STR_ONE_LINE;
        if (d->character->pc->nanny_state)
      d->character->pc->nanny_state = STATE_LDESC;
        return;
      }
    */
    else if (!str_cmp (key, "short"))
    {
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "help_psdesc"), d);

        if (ch->short_descr)
        {
            if (str_cmp (ch->short_descr, "(null)"))
            {
                sprintf (buf, "Replacing:  %s\n", ch->short_descr);
                SEND_TO_Q (buf, d);
            }
            ch->short_descr = NULL;
        }

        d->str = &ch->short_descr;
        d->max_str = STR_ONE_LINE;
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_LDESC;
        return;
    }

    else if (!str_cmp (key, "long"))
    {
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "help_pldesc"), d);

        if (ch->long_descr)
        {
            if (str_cmp (ch->long_descr, "(null)"))
            {
                sprintf (buf, "Replacing:  %s\n", ch->long_descr);
                SEND_TO_Q (buf, d);
            }
            ch->long_descr = NULL;
        }

        if (d->character->pc->nanny_state)
        {
            ch->name = str_dup (keyword_gen(ch, ch->short_descr, 0));
            d->character->pc->nanny_state = STATE_FDESC;
        }
        d->str = &ch->long_descr;
        d->max_str = STR_ONE_LINE;
        return;
    }

    else if (!str_cmp (key, "description"))
    {
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "help_pdesc"), d);

        if (ch->description)
        {
            if (str_cmp (ch->description, "(null)"))
            {
                sprintf (buf, "Replacing:\n\n%s\n", ch->description);
                SEND_TO_Q (buf, d);
            }
            ch->description = NULL;
        }

        SEND_TO_Q
        ("> 1----*----10---*----20---*----30---*----40---*----50---*----60---*----70---END\n",
         d);
        d->str = &ch->description;
        d->max_str = MAX_INPUT_LENGTH;
        if (d->character->pc->nanny_state)
            d->character->pc->nanny_state = STATE_SKILLS;
        return;
    }

    else if (!str_cmp (key, "classifieds"))
    {
        if (!available_roles (d->acct->get_rpp ()))
        {
            SEND_TO_Q
            ("\n#2Sorry, but there are currently no posted roles available to you.#0\n\n> ",
             d);
            return;
        }
		/*
        if (!d->acct->get_rpp ())
        {
            SEND_TO_Q
            ("Sorry, but you must have at least one reward point first.\n\n> ",
             d);
            return;
        }
		*/
        SEND_TO_Q ("\n", d);
        SEND_TO_Q (get_text_buffer (NULL, text_list, "special_role_select"), d);
        *b_buf = '\0';
        if (d->character->pc->special_role)
        {
            d->character->pc->special_role = NULL;
        }
        for (role = role_list, i = 1; role; role = role->next, i++)
        {
            if (role->cost > (d->acct->get_rpp ()))
            {
                i--;
                continue;
            }
            if (i < 10)
                sprintf (b_buf + strlen (b_buf), "%d.  #2%s#0\n", i,
                         role->summary);
            else
                sprintf (b_buf + strlen (b_buf), "%d. #2%s#0\n", i,
                         role->summary);
        }
        strcat (b_buf, "\nYour Desired Role: ");
        page_string (d, b_buf);
        d->connected = CON_SPECIAL_ROLE_SELECT;
        return;
    }

    else if (!str_cmp (key, "race"))
    {
        race_selection_screen (d);
        return;
    }

    else if (!str_cmp (key, "age"))
    {
        sprintf (b_buf, "\n%s",
                 get_text_buffer (NULL, text_list, "age_select"));

        if (lookup_race_variable (d->character->race, RACE_MIN_AGE)
                && lookup_race_variable (d->character->race, RACE_MAX_AGE))
            sprintf (b_buf + strlen (b_buf), "Your Desired Age (%d to %d): ",
                     atoi (lookup_race_variable
                           (d->character->race, RACE_MIN_AGE)),
                     atoi (lookup_race_variable
                           (d->character->race, RACE_MAX_AGE)));
        else
            sprintf (b_buf + strlen (b_buf), "Your Desired Age (15 to 60): "),
            SEND_TO_Q ("\n", d);
        SEND_TO_Q (b_buf, d);
        d->connected = CON_AGE;
        return;
    }

    else if (!str_cmp (key, "location"))
    {
        start_loc = num_starting_locs (d->character->race);
        if (start_loc < 2)
        {
            SEND_TO_Q ("This option is not available to your PC's race.\n\n>",
                       d);
            return;
        }
        strcpy (b_buf, get_text_buffer (NULL, text_list, "location"));
        strcat (b_buf, "#2Choose starting location:#0 ");
        page_string (d, b_buf);
        d->connected = CON_LOCATION;
        return;
    }

    else if (!str_cmp (key, "profession"))
    {
        profession_display (d);
        return;
    }

    else if (!str_cmp (key, "featuresx"))
    {
        autodesc_eyes (d);
        return;
    }

    else if (!str_cmp (key, "height") || !str_cmp (key, "frame") || !str_cmp(key, "features"))
    {
        SEND_TO_Q ("\n", d);
        strcpy (b_buf, get_text_buffer (NULL, text_list, "height_frame"));
        strcat (b_buf, "#2Choose height and frame:#0 ");
        page_string (d, b_buf);
        d->connected = CON_HEIGHT_WEIGHT;
        return;
    }

    else if (!str_cmp (key, "skills"))
    {
        if (d->character->race < 0)
        {
            SEND_TO_Q ("You'll need to choose a race first.\n", d);
            create_menu_actions (d, "RACE");
            return;
        }
        skill_display (d);
        return;
    }

    else if (!str_cmp (key, "comment"))
    {

        SEND_TO_Q (get_text_buffer (NULL, text_list, "comment_help"), d);

        if (ch->pc->creation_comment)
        {
            ch->pc->creation_comment = NULL;
        }

        d->str = &ch->pc->creation_comment;
        d->max_str = MAX_INPUT_LENGTH;
        /*
        		if ( d->character->pc->nanny_state && is_newbie (d->character) )
        			d->character->pc->nanny_state = STATE_PRIVACY;
        		else
        */
        d->character->pc->nanny_state = 0;

        return;
    }

    else if (!str_cmp (key, "submit"))
    {

        picks_left = 8;

        for (i = 1; i <= LAST_SKILL && *skills[i] != '\n'; i++)
        {
            if (ch->skills[i] && pickable_skill (ch, skills[i], 1))
            {
                picks_left--;
            }

            if (ch->skills[i])
                picked++;

        }

        if (picks_left > 4 || picks_left < 0)
        {
            SEND_TO_Q
            ("\n#2Type SKILLS at the prompt to choose your PC's starting skills.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (picked > 9)
        {
            SEND_TO_Q
            ("\n#2Your character has more than selected eight skills. Type SKILLS to correct this.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }



        if (!ch->pc->profession)
        {
            ch->pc->profession = 0;
            /*
            mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
            if ((result = mysql_store_result (database)) == NULL)
            ;
            else
            {
            if (mysql_num_rows (result) > 0)
            {
            SEND_TO_Q
              ("\n#2You need to select a profession. Please type PROFESSION to do so.#0\n\n",
               d);
            SEND_TO_Q ("> ", d);
            mysql_free_result (result);
            return;
            }
            mysql_free_result (result);
            }
            */
        }

        if (!ch->sex)
        {
            SEND_TO_Q
            ("\n#2You need to select a gender. Please type SEX to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

		if (!ch->age || ch->age == -1)
        {
            SEND_TO_Q
            ("\n#2You need to select an age. Please type AGE to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

		if (!ch->str || !ch->dex || !ch->con || !ch->intel || !ch->wil || !ch->aur || !ch->agi)
        {
            SEND_TO_Q
            ("\n#2You need to select some attributes. Please type ATTRIBUTES to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (!ch->height)
        {
            SEND_TO_Q
            ("\n#2You need to select a build. Please type HEIGHT to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (ch->name && !*ch->name)
        {
            SEND_TO_Q
            ("\n#2You need to supply a keyword list. Please type KEYWORDS to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (ch->long_descr && !*ch->long_descr)
        {
            SEND_TO_Q
            ("\n#2You need to supply a long description. Please type LONG to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (ch->long_descr && !*ch->short_descr)
        {
            SEND_TO_Q
            ("\n#2You need to supply a short description. Please type SHORT to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (ch->long_descr && !*ch->description)
        {
            SEND_TO_Q
            ("\n#2You need to supply a description. Please type DESCRIPTION to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }

        if (ch->pc && ch->pc->creation_comment && !*ch->pc->creation_comment)
        {
            SEND_TO_Q
            ("\n#2You need to supply a background. Please type COMMENT to do so.#0\n\n",
             d);
            SEND_TO_Q ("> ", d);
            return;
        }
        std::string player_db = engine.get_config ("player_db");
        mysql_safe_query
        ("SELECT name,create_state"
         " FROM %s.pfiles"
         " WHERE account = '%s'"
         " AND (create_state >= 1 AND create_state < 4)",
         player_db.c_str (),
         d->acct->name.c_str ());
        result = mysql_store_result (database);

        if (result && mysql_num_rows (result) >= 1)
            block++;

        if (result)
            mysql_free_result (result);

        if ((fp = fopen ("application_lock", "r")))
        {
            SEND_TO_Q
            ("\n#2Sorry, but new character submissions are not being accepted\n"
             "at this time. Please try back later; thank you for your interest.#0\n",
             d);
            fclose (fp);
            display_main_menu (d);
            return;
        }

        if (block && !is_admin (d->acct->name.c_str ()))
        {
            SEND_TO_Q
            ("\n#2Sorry, but you may only have one character submitted and/or in the\n"
             "game at any one time; you'll have to wait until your current character\n"
             "dies or is retired to submit an application for a new one.#0\n",
             d);
            display_main_menu (d);
            return;
        }

        req = 0;

        if (lookup_race_variable (ch->race, RACE_RPP_COST))
            req = atoi (lookup_race_variable (ch->race, RACE_RPP_COST));
        if (ch->pc->special_role)
            req = MAX (ch->pc->special_role->cost, req);

        if (d->acct->get_rpp () < req)
        {
            SEND_TO_Q
            ("\n#2Sorry, but you do not seem to have the reward points required to\n"
             "submit the application for this particular character.#0\n", d);
            display_main_menu (d);
            return;
        }

        *ch->tname = toupper (*ch->tname);
        ch->pc->create_state = STATE_SUBMITTED;

        if (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE))
            ch->speaks =
                atoi (lookup_race_variable (ch->race, RACE_NATIVE_TONGUE));

        ch->in_room = NOWHERE;
        ch->time.played = 0;

        if (ch->long_descr)
        {
            if (ch->long_descr[strlen (ch->long_descr) - 1] != '.')
                strcat (ch->long_descr, ".");
            if (!isupper (*ch->long_descr))
                *ch->long_descr = toupper (*ch->long_descr);
            sprintf (buf, "%s", ch->long_descr);
            ch->long_descr = tilde_eliminator (buf);
        }

        if (ch->short_descr)
        {
            if (isupper (*ch->short_descr))
                *ch->short_descr = tolower (*ch->short_descr);
            if (ch->short_descr[strlen (ch->short_descr) - 1] == '.')
                ch->short_descr[strlen (ch->short_descr) - 1] = '\0';
            sprintf (buf, "%s", ch->short_descr);
            ch->short_descr = tilde_eliminator (buf);
        }

        if (ch->description)
        {
            sprintf (buf, "%s", ch->description);
            ch->description = tilde_eliminator (buf);
        }

        if (ch->name)
        {
            sprintf (buf, "%s", ch->name);
            ch->name = tilde_eliminator (buf);
        }

        if (ch->pc->creation_comment)
        {
            sprintf (buf, "%s", ch->pc->creation_comment);
            ch->pc->creation_comment = tilde_eliminator (buf);
        }

        for (tch = character_list; tch; tch = tch->next)
        {

            if (tch->deleted)
                continue;

            if (IS_NPC (tch))
                continue;

            if (GET_TRUST (tch) > 3
                    || (is_newbie (ch) && tch->pc->is_guide
                        && !IS_SET (ch->plr_flags, PRIVATE)))
            {
                sprintf (buf,
                         "%s has submitted an application for a new character: %s.\n",
                         ch->pc->account_name, ch->tname);
                send_to_char (buf, tch);
            }
        }

        if (ch->pc->app_cost < 0)
            ch->pc->app_cost = 0;

        SEND_TO_Q
        ("\nThank you.  Your character application has been submitted.\n"
         "An administrator may be reviewing your application soon.\n", d);
        SEND_TO_Q
        ("You will receive an email at the address registered for this\n"
         "account when the application has been reviewed, along with any\n"
         "comments the reviewing administrator wished to make.\n" "\n"
         "Character review generally takes anywhere from 48-72 hours,\n"
         "depending on the workload of our roleplay admins. We thank you\n"
         "in advance for your patience, and for your interest in Atonement RPI!\n"
         "We'll see you soon, in the Future!\n", d);
        d->character->time.birth = time (0);
        d->character->time.played = 0;
        d->character->time.logon = time (0);
        display_main_menu (d);
        save_char (d->character, false);
        unload_pc (d->character);
        d->character = NULL;
        return;
    }

    else
        SEND_TO_Q ("Unknown keyword!\n", d);

    create_menu_options (d);
}

void
nanny (DESCRIPTOR_DATA * d, char *argument)
{
    switch (d->connected)
    {
    case CON_LOGIN:
        nanny_login_choice (d, argument);
        break;
    case CON_NEW_ACCT_NAME:
        nanny_new_account (d, argument);
        break;
    case CON_ACCT_POLICIES:
        nanny_account_policies (d, argument);
        break;
    case CON_REFERRAL:
        nanny_account_referral (d, argument);
        break;
    case CON_ACCT_EMAIL:
        nanny_account_email (d, argument);
        break;
    case CON_EMAIL_CONFIRM:
        nanny_account_email_confirm (d, argument);
        break;
    case CON_ACCOUNT_SETUP:
        nanny_account_setup (d, argument);
        break;
    case CON_ENTER_ACCT_NME:
        nanny_ask_password (d, argument);
        break;
    case CON_PWDCHK:
        nanny_check_password (d, argument);
        break;
    case CON_PENDING_DISC:
        nanny_disconnect (d, argument);
        break;
    case CON_ACCOUNT_MENU:
        nanny_connect_select (d, argument);
        break;
    case CON_PWDNEW:
//      nanny_change_password (d, argument);
//      break;
    case CON_PWDGET:
        nanny_new_password (d, argument);
        break;
    case CON_PWDNCNF:
        nanny_conf_change_password (d, argument);
        break;
    case CON_CHG_EMAIL:
        nanny_change_email (d, argument);
        break;
    case CON_CHG_EMAIL_CNF:
        nanny_change_email_confirm (d, argument);
        break;
    case CON_DELETE_PC:
        nanny_delete_pc (d, argument);
        break;
    case CON_CHOOSE_PC:
        nanny_choose_pc (d, argument);
        break;
    case CON_VIEW_PC:
        nanny_view_pc (d, argument);
        break;
    case CON_READING_WAIT:
        nanny_reading_wait (d, argument);
        break;
    case CON_RACE_CONFIRM:
        nanny_race_confirm (d, argument);
        break;
    case CON_PRIV_CONFIRM:
        nanny_privacy_confirm (d, argument);
        break;
    case CON_NAME_CONFIRM:
        nanny_char_name_confirm (d, argument);
        break;
    case CON_TERMINATE_CONFIRM:
        nanny_terminate (d, argument);
        break;
    case CON_RETIRE:
        nanny_retire (d, argument);
        break;
    case CON_RACE_SELECT:
        race_selection (d, argument);
        break;

    case CON_PROFESSION:
        profession_selection (d, argument);
        break;

    case CON_PROFESSION_CONFIRM:
        profession_confirm (d, argument);
        break;

    case CON_PROF_Q1:
        profession_first (d, argument);
        break;

    case CON_PROF_Q2:
        profession_second (d, argument);
        break;

    case CON_DESC_HEIGHT:
        autodesc_height (d);
        break;

    case CON_DESC_FRAME:
        autodesc_frame (d, argument);
        break;

    case CON_DESC_EYE:
        autodesc_eyes (d);
        break;

    case CON_DESC_EYE_SPECIFY:
        autodesc_eyes_specific (d, argument);
        break;

    case CON_DESC_LENGTH:
        autodesc_length (d, argument);
        break;

    case CON_DESC_COLOR:
        autodesc_color (d, argument);
        break;

    case CON_DESC_COLOR_SPECIFY:
        autodesc_color_specific (d, argument);
        break;

    case CON_DESC_STYLE:
        autodesc_style (d, argument);
        break;

    case CON_DESC_F1:
        autodesc_feat_one (d, argument);
        break;

    case CON_DESC_F1_SPECIFY:
        autodesc_feat_one_specify (d, argument);
        break;

    case CON_DESC_F2:
        autodesc_feat_two (d, argument);
        break;

    case CON_DESC_F2_SPECIFY:
        autodesc_feat_two_specify (d, argument);
        break;

    case CON_DESC_F3:
        autodesc_feat_three (d, argument);
        break;

    case CON_DESC_F3_SPECIFY:
        autodesc_feat_three_specify (d, argument);
        break;

    case CON_DESC_F4:
        autodesc_feat_four (d, argument);
        break;

    case CON_DESC_F4_SPECIFY:
        autodesc_feat_four_specify (d, argument);
        break;

    case CON_DESC_END:
        autodesc_end (d, argument);
        break;

    case CON_DESC_END_UNHUMAN:
        autodesc_end_unhuman (d, argument);
        break;

    case CON_DESC_F1_UNHUMAN:
        autodesc_feat_one_unhuman (d, argument);
        break;

    case CON_DESC_F1_SPECIFY_UNHUMAN:
        autodesc_feat_one_specify_unhuman (d, argument);
        break;

    case CON_DESC_F2_UNHUMAN:
        autodesc_feat_two_unhuman (d, argument);
        break;

    case CON_DESC_F2_SPECIFY_UNHUMAN:
        autodesc_feat_two_specify_unhuman (d, argument);
        break;

    case CON_DESC_ONE:
        autodesc_result (d, argument);
        break;

    case CON_DESC_ONE_HALF:
        autodesc_result_second (d, argument);
        break;

    case CON_DESC_TWO:
        autodesc_one_conf (d, argument);
        break;

    case CON_DESC_THREE:
        autodesc_two_conf (d, argument);
        break;

    case CON_DESC_FOUR:
        autodesc_three_conf (d, argument);
        break;

    case CON_SPECIAL_ROLE_SELECT:
        nanny_special_role_selection (d, argument);
        break;
    case CON_SPECIAL_ROLE_CONFIRM:
        nanny_special_role_confirm (d, argument);
        break;
    case CON_CREATE_GUEST:
        nanny_create_guest (d, argument);
        break;
    case CON_MAIL_MENU:
        nanny_mail_menu (d, argument);
        break;
    case CON_COMPOSE_MAIL_TO:
        nanny_compose_mail_to (d, argument);
        break;
    case CON_COMPOSE_SUBJECT:
        nanny_compose_subject (d, argument);
        break;
    case CON_COMPOSE_MESSAGE:
        nanny_compose_message (d, argument);
        break;
    case CON_COMPOSING_MESSAGE:
        nanny_composing_message (d, argument);
        break;
    case CON_READ_MESSAGE:
        nanny_read_message (d, argument);
        break;

    case CON_PLAYER_NEW:
        d->connected = CON_CREATION;
        create_menu_options (d);
        break;

    case CON_RACE:
        d->connected = CON_CREATION;
        create_menu_options (d);
        break;

    case CON_AGE:
        age_selection (d, argument);
        if (age (d->character).year)
        {
            d->character->pc->nanny_state = STATE_PROFESSION;
            d->connected = CON_CREATION;
        }
        else
            d->connected = CON_AGE;
        create_menu_options (d);
        break;

    case CON_HEIGHT_WEIGHT:
        height_frame_selection (d, argument);
        create_menu_options (d);
        break;

    case CON_LOCATION:
        location_selection (d, argument);
        create_menu_options (d);
        break;

    case CON_SKILLS:
        skill_selection (d, argument);
        break;

    case CON_SEX:
		d->character->sex = 0;
        sex_selection (d, argument);
        if (d->character->pc->nanny_state && d->character->sex)
        {
            if (!available_roles (d->acct->get_rpp ()))
                d->character->pc->nanny_state = STATE_RACE;
            else
                d->character->pc->nanny_state = STATE_SPECIAL_ROLES;
            d->connected = CON_CREATION;
        }
        create_menu_options (d);
        break;

    case CON_ATTRIBUTES:
        attribute_priorities (d, argument);
        if (d->character->pc->nanny_state && d->character->str)
        {
            d->character->pc->nanny_state = STATE_FRAME;
        }
        d->connected = CON_CREATION;
        create_menu_options (d);
        break;

    case CON_CREATION:
        create_menu_actions (d, argument);
        break;

    case CON_WEB_CONNECTION:
        /*                      web_process (d, argument); */
        break;

    default:
        break;

    }
}

void read_motd(DESCRIPTOR_DATA * d)
{
    std::string msg_line;
    std::string output;

    std::ifstream fin( "MOTD" );

    if ( !fin )
    {
        system_log ("The MOTD could not be found", true);
        return;
    }

    output.assign("\n\n");

    while ( getline(fin, msg_line) )
    {
        output.append(msg_line);
    }

    fin.close();

    if (!output.empty())
    {
        output.append("\n");
        SEND_TO_Q (output.c_str(), d);

    }

    return;
}
