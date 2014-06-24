//////////////////////////////////////////////////////////////////////////////
//
/// account.h - Account Class Structures and Routines
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


#ifndef _rpie_account_h_
#define _rpie_account_h_


#include <sstream>
#include <string>
#include <time.h>
#include <string.h>
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "net_link.h"


typedef enum
{
  ACCESS_NORMAL = 0,		///< Default: Allowed unless IP Banned.
  ACCESS_ALLOWED,		///< Exempt from IP Bans (use sparingly)
  ACCESS_RESTRICTED,		///< Allowed under certain conditions.
  ACCESS_BANNED			///< Not allowed to connect.
}
ACCESS_PERMISSION;

typedef enum
{
  STRUCT_ACCOUNT
}
DATA_TYPE;

#define	ACCOUNT_NOPETITION		( 1 << 0 )
#define ACCOUNT_NOBAN			( 1 << 1 )
#define ACCOUNT_NOGUEST			( 1 << 2 )
#define ACCOUNT_NOPSI			( 1 << 3 )
#define ACCOUNT_NORETIRE		( 1 << 4 )
#define ACCOUNT_NOVOTE			( 1 << 5 )
#define ACCOUNT_IPSHARER		( 1 << 6 )
#define ACCOUNT_RPPDISPLAY		( 1 << 7 )
#define ACCOUNT_NO_OOC			( 1 << 8 )


class account
{
 private:
  int id;
  int roleplay_points;
  time_t last_rpp;
  time_t last_nominate;

  void save_email () const
    {
      std::string escaped_email;

      get_email_sql_safe (escaped_email);

      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_email = '" << escaped_email << "'"
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    td->acct->set_email (email);
	}
    }

  void save_password () const
    {
      std::string escaped_password;

      get_password_sql_safe (escaped_password);

      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_password = '" << escaped_password << "'"
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    td->acct->set_password (password);
	}
    }
    
  void save_last_ip () const
    {
      std::string escaped_ip;

      get_last_ip_sql_safe (escaped_ip);

      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_last_ip = '" << escaped_ip << "'"
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    td->acct->set_last_ip (last_ip);
	}
    }

  void save_flags () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET account_flags = " << flags 
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    td->acct->set_flags (flags);
	}
    }

  void save_rpp () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET roleplay_points = " << roleplay_points
	<< ", last_rpp = " << last_rpp
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_rpp (roleplay_points);
	      td->acct->set_last_rpp (last_rpp);
	    }
	}
    }

  void save_nominate () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET last_nominate = " << last_nominate
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_last_nominate (last_nominate);
	    }
	}
    }


  void save_color_flag () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_color = " << color
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_color_flag (color);
	    }
	}
    }

  void save_sound_flag () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_sound = " << sound
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_sound_flag (sound);
	    }
	}
    }

  void save_guide_flag () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_guide = " << guide
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_guide_flag (guide);
	    }
	}
    }

  void save_newsletter_flag () const
    {
      std::ostringstream update_flag_stream;
      update_flag_stream 
	<< "UPDATE forum_users"
	<< " SET user_subscription = " << newsletter
	<< " WHERE user_id = " << id;

      std::string update_flag_string = update_flag_stream.str ();
      mysql_safe_query ((char *)update_flag_string.c_str ());

      extern DESCRIPTOR_DATA *descriptor_list;
      for (DESCRIPTOR_DATA* td = descriptor_list; td; td = td->next)
	{
	  if (td->acct && td->acct->get_id () == id && td->acct != this)
	    {
	      td->acct->set_newsletter_flag (newsletter);
	    }
	}
    }


 public:
  std::string name;
  std::string password;
  std::string email;
  std::string last_ip;

  time_t created_on;
  int password_attempt;
  bool color;
  bool sound;
  bool guide;
  bool newsletter;
  int flags;
  int forum_posts;
  float timezone;
  int code;

  account (void)
    { 
      initialize (); 
    }

  account (const char *acct_name);

  ~account (void)
    {
    }


  // Access Members

  int get_id () const
    {
      return id;
    }

  void get_name_sql_safe (std::string& safe_name) const
    {
      size_t name_len = name.length ();
      char *escaped_name = new char[(name_len * 2) + 1];
      mysql_real_escape_string (database, escaped_name, name.c_str (), name_len);
      safe_name.assign (escaped_name);
      delete [] escaped_name;
    }

  void get_email_sql_safe (std::string& safe_email) const
    {
      size_t email_len = email.length ();
      char *escaped_email = new char[(email_len * 2) + 1];
      mysql_real_escape_string (database, escaped_email, email.c_str (), email_len);
      safe_email.assign (escaped_email);
      delete [] escaped_email;
    }

  void get_password_sql_safe (std::string& safe_password) const
    {
      size_t password_len = password.length ();
      char *escaped_password = new char[(password_len * 2) + 1];
      mysql_real_escape_string (database, escaped_password, password.c_str (), password_len);
      safe_password.assign (escaped_password);
      delete [] escaped_password;
    }

  void get_last_ip_sql_safe (std::string& safe_ip) const
    {
      size_t ip_len = last_ip.length ();
      char *escaped_ip = new char[(ip_len * 2) + 1];
      mysql_real_escape_string (database, escaped_ip, last_ip.c_str (), ip_len);
      safe_ip.assign (escaped_ip);
      delete [] escaped_ip;
    }

  int get_rpp (void) const
    {
      return roleplay_points;
    }

  time_t get_last_rpp_date (void) const
    {
      return last_rpp;
    }

  time_t get_last_nominate (void) const
    {
      return last_nominate;
    }

  const std::string get_last_rpp_date_string (void) const
    {
      char *date = new char[AVG_STRING_LENGTH];
      std::string date_string;

      date[0] = '\0';
      asctime_r (localtime (&last_rpp), date);
      date[strlen (date) - 1] = '\0';

      date_string.assign (date);
      delete [] date;

      return date_string;
    }

  const std::string get_nominate_rpp_date_string (void) const
    {
      char *date = new char[AVG_STRING_LENGTH];
      std::string date_string;

      date[0] = '\0';
      asctime_r (localtime (&last_nominate), date);
      date[strlen (date) - 1] = '\0';

      date_string.assign (date);
      delete [] date;

      return date_string;
    }


  bool is_registered (void) const
    {
      return (id > 0) ? true : false;
    }


  bool is_psionic_capable (void) const
    {
      if (roleplay_points < 2 || IS_SET (flags, ACCOUNT_NOPSI))
	{
	  return false;
	}
      return true;
    }


  // Modification Members

  void initialize (void)
    {
      id = 0;

      name.erase ();
      email.erase ();
      password.erase ();
      last_ip.erase ();

      roleplay_points = 0;
      created_on = 0;
      password_attempt = 0;
      color = 0;
      sound = 0;
      newsletter = true;
      flags = 0;
      forum_posts = 0;
      timezone = 0;
      code = 0;
      last_rpp = 0;
      last_nominate = 0;
	  guide = 0;
	}

  void set_name (const char *account_name)
    {
      if (account_name)
	{
	  name.assign (account_name);
	}
    }

  void set_email (const char *account_email)
    {
      if (account_email)
	{
	  email.assign (account_email);
	}
    }

  void set_email (const std::string account_email)
    {
      if (account_email.length ())
	{
	  email.assign (account_email);
	}
    }

  void set_password (const char *account_password)
    {
      if (account_password)
	{
	  password.assign (account_password);
	}
    }

  void set_password (const std::string account_password)
    {
      if (account_password.length ())
	{
	  password.assign (account_password);
	}
    }

  void set_last_ip (const char *account_last_ip)
    {
      if (account_last_ip)
	{
	  last_ip.assign (account_last_ip);
	}
    }

  void set_last_ip (const std::string account_last_ip)
    {
      if (account_last_ip.length ())
	{
	  last_ip.assign (account_last_ip);
	}
    }

  void set_color_flag (bool account_color)
    {
      color = account_color;
    }

  void set_sound_flag (bool account_sound)
    {
      sound = account_sound;
    }

  void set_guide_flag (bool account_guide)
    {
      guide = account_guide;
    }

  void set_newsletter_flag (bool account_newsletter)
    {
      newsletter = account_newsletter;
    }

  void update_email (const char *account_email)
    {
      set_email (account_email);
      save_email ();
    }

  void update_password (const char *account_password)
    {
      set_password (account_password);
      save_password ();
    }

  void update_last_ip (const char *account_last_ip)
    {
      set_last_ip (account_last_ip);
      save_last_ip ();
    }

  void set_rpp (int account_rpp)
    {
      roleplay_points = account_rpp;
    }
  
  void set_last_rpp (time_t account_last_rpp)
    {
      last_rpp = account_last_rpp;
    }

  void set_last_nominate (time_t account_last_nominate)
    {
      last_nominate = account_last_nominate;
    }

  void set_flags (int account_flags)
    {
      flags = account_flags;
    }

  bool toggle_color_flag ()
    {
      color = !color;
      save_color_flag ();
      return color;
    }

  bool toggle_sound_flag ()
    {
      sound = !sound;
      save_sound_flag ();
      return sound;
    }

  bool toggle_guide_flag ()
    {
      guide = !guide;
      save_guide_flag ();
      return guide;
    }

  bool toggle_newsletter_flag ()
    {
      newsletter = !newsletter;
      save_newsletter_flag ();
      return newsletter;
    }

  bool toggle_petition_ban ()
    {
      flags = flags ^ ACCOUNT_NOPETITION;
      save_flags ();
      return flags & ACCOUNT_NOPETITION;
    }
      
  bool toggle_ban_pass ()
    {
      flags = flags ^ ACCOUNT_NOBAN;
      save_flags ();
      return flags & ACCOUNT_NOBAN;
    }
      
  bool toggle_guest_ban ()
    {
      flags = flags ^ ACCOUNT_NOGUEST;
      save_flags ();
      return flags & ACCOUNT_NOGUEST;
    }
      
  bool toggle_psionics_ban ()
    {
      flags = flags ^ ACCOUNT_NOPSI;
      save_flags ();
      return flags & ACCOUNT_NOPSI;
    }
      
  bool toggle_retirement_ban ()
    {
      flags = flags ^ ACCOUNT_NORETIRE;
      save_flags ();
      return flags & ACCOUNT_NORETIRE;
    }
      
  bool toggle_ignore_vote_notes ()
    {
      flags = flags ^ ACCOUNT_NOVOTE;
      save_flags ();
      return flags & ACCOUNT_NOVOTE;
    }
      
  bool toggle_ip_sharing ()
    {
      flags = flags ^ ACCOUNT_IPSHARER;
      save_flags ();
      return flags & ACCOUNT_IPSHARER;
    }
      
  bool toggle_rpp_visibility ()
    {
      flags = flags ^ ACCOUNT_RPPDISPLAY;
      save_flags ();
      return flags & ACCOUNT_RPPDISPLAY;
    }
      
  bool toggle_ooc_ban ()
    {
      flags = flags ^ ACCOUNT_NO_OOC;
      save_flags ();
      return flags & ACCOUNT_NO_OOC;
    }
      

  void award_rpp ()
    {
      roleplay_points++;
      last_rpp = time (0);
      save_rpp ();
    }

  void nominate_rpp ()
    {
      last_nominate = time (0);
      save_nominate ();
    }
  
  void deduct_rpp (unsigned int deduction)
    {
      roleplay_points -= deduction;
      last_rpp = time (0);
      save_rpp ();
    }

  void pay_application_cost (unsigned int application_cost)
    {
      roleplay_points -= application_cost;
      if (roleplay_points < 0)
	roleplay_points = 0;
      save_rpp ();
    }


  // Static Members


  // called by magic.c::check_psionic_talents
  static bool is_psionic_capable (const char *account_name)
    {
      bool acct_is_psionic_capable = false;
      account *acct = new account (account_name);

      if (acct)
	{
	  acct_is_psionic_capable = acct->is_psionic_capable ();

	  delete acct;
	}

      return acct_is_psionic_capable;
    }

};

void setup_new_account (account * acct);

void send_email (account * acct, const char *cc, char *from, char *subject,
		 char *message);

void save_hobbitmail_message (account * acct, MUDMAIL_DATA * message);


#endif // _rpie_account_h_
