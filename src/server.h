//////////////////////////////////////////////////////////////////////////////
//
/// server.h - Server Class Structures and Routines
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2006 C. W. McHenry
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


#ifndef _rpie_server_h_
#define _rpie_server_h_
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sys/time.h>
#include <sys/resource.h>

namespace rpie
{
  enum server_modes
    {
      mode_unknown,
      mode_play,
      mode_build,
      mode_test,
    };

  class server
    {
    private:
      int server_port;
      server_modes server_mode;
      std::map<std::string,std::string> config_variables;

      // checkpoint alarms
      static const int BOOT_DB_ABORT_THRESHOLD = 15; ///< Infinite loop test.
      static const int RUNNING_ABORT_THRESHOLD = 5; ///< Infinite loop test.
      bool abort_threshold_enabled;
      int abort_threshold;
      int last_checkpoint;

      int get_user_seconds ()
	{
	  struct rusage rus;
	  getrusage (RUSAGE_SELF, &rus);
	  return rus.ru_utime.tv_sec;
	}

    public:
      static const size_t MAX_NAME_LENGTH = 15;	///< Username string length
      static const int ALARM_FREQUENCY = 20; ///< ITimer frequency in seconds

      server ();

      // checkpoint alarms
      void enable_timer_abort ()
	{
	  abort_threshold_enabled = true;
	}
      void disable_timer_abort ()
	{
	  abort_threshold_enabled = false;
	}

      void set_abort_threshold_pre_booting ()
	{
	  abort_threshold_enabled = true;
	  abort_threshold = BOOT_DB_ABORT_THRESHOLD;
	  last_checkpoint = get_user_seconds ();
	}

      void set_abort_threshold_post_booting ()
	{
	  last_checkpoint = get_user_seconds ();

	  if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
	    {
	      abort_threshold = RUNNING_ABORT_THRESHOLD;
	    }
	}

      bool loop_detect ()
	{
	  int timeslice = get_user_seconds () - last_checkpoint;
	  return (abort_threshold_enabled
		  && (timeslice > abort_threshold));
	}

      void load_config_files();
      void load_config_file (std::ifstream &config_file);

      void set_config (std::string var_name, std::string var_value);
      std::string get_config ();
      std::string get_config (std::string var_name)
	{
	  return config_variables.find (var_name)->second;
	}

      // Mode Inquiry
      server_modes get_mode () { return server_mode; }
      bool in_play_mode () { return (server_mode == mode_play); }
      bool in_build_mode () { return (server_mode == mode_build); }
      bool in_test_mode () { return (server_mode == mode_test); }
      int get_port () { return server_port; }
      std::string get_base_path (std::string req_mode = "current");
    };

}

#endif // _rpie_server_h_
