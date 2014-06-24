#include "server.h"
#include <cstdlib>


rpie::server::server ()
{
  // Set Defaults
  // Override these in your config file.
  set_config ("mysql_host", "localhost");
  set_config ("mysql_user","blahblahblah");
  set_config ("mysql_passwd", "blahblahblahblah");

  set_config ("engine_db", "rpi_engine");
  set_config ("player_db", "rpi_player");
  set_config ("player_log_db", "rpi_player_log");
  set_config ("world_db", "rpi_player_log");
  set_config ("world_log_db", "rpi_world_log");

  set_config ("server_mode", "test");
  set_config ("server_port", "4446");

  set_config ("server_path_play", "/home/rpi/pp");
  set_config ("server_path_build", "/home/rpi/bp");
  set_config ("server_path_test", "/home/rpi/tp");
  return;
}

void
rpie::server::set_config (std::string var_name, std::string var_value = "true")
{
  if (var_value.size())
    {
      config_variables[var_name] = var_value;
      if (var_name == "server_mode")
	{
	  if (var_value == "play")
	    {
	      server_mode = mode_play;
	    }
	  else if (var_value == "build")
            {
              server_mode = mode_build;
            }
	  else if (var_value == "test")
            {
              server_mode = mode_test;
            }
          else
            {
              server_mode = mode_unknown;
            }
	}
      else if (var_name == "server_port")
	{
	  server_port = strtol (var_value.c_str (), 0, 10);
	}
    }
}

/// Load configuration files from the usual places
void rpie::server::load_config_files()
{
    std::vector<std::string> config_filenames;

    // Singularized loading point

    // Push back filenames.
    config_filenames.push_back( "./config" );
    config_filenames.push_back("./.rpi_engine/config");
    config_filenames.push_back("~/.rpi_engine/config");
    config_filenames.push_back("/etc/rpi_engine/config");
    typedef std::vector<std::string>::const_iterator VI;

    for ( VI i = config_filenames.begin (); i != config_filenames.end (); ++i)
    {
        std::ifstream config_file ( ( * i ).c_str() );

        if ( config_file.is_open() )
            load_config_file ( config_file );
    }
}

/// Get the db connection info from the configuration file
void rpie::server::load_config_file( std::ifstream & config_file )
{
    std::string line;
    while (std::getline (config_file,line))
    {
        std::string::size_type comment_start = line.find_first_of ('#');

        if (comment_start != std::string::npos)
        {
            line.erase (comment_start, std::string::npos);
        }

        if( line.empty() )
            continue;

        const char * const delimiters = "= ;\t\r\n";

        std::string var_name;
        std::string::size_type var_name_start = line.find_first_not_of( delimiters + 1 );

        if ( var_name_start == std::string::npos )
            continue;

        // Find the end of the variable
        std::string::size_type var_name_end = line.find_first_of( delimiters, var_name_start );

        if ( var_name_end == std::string::npos)
        {
            var_name.assign( line.substr( var_name_start ) );
            set_config (var_name);
            continue;
        } else {
            std::string::size_type len = var_name_end - var_name_start;
            var_name.assign ( line.substr( var_name_start, len ) );
        }

        std::string::size_type eq_start = line.find_first_not_of( delimiters + 1, var_name_end );
        if ( eq_start == std::string::npos )
        {
            set_config( var_name );
            continue;
        }
      if (line.at(eq_start) != '=')
	{
	  std::cerr << "Warning, bad syntax on line: "
		    << line << std::endl;
	  continue;
	}

      // skip whitespace to start of '='
      std::string::size_type val_start =
	line.find_first_not_of (delimiters+1, eq_start+1);
      if (val_start == std::string::npos)
	{
	  std::cerr << "Warning, bad syntax on line: "
		    << line << std::endl;
	  continue;
	}

      std::string val_string;
      char next_c = line.at(val_start);
      if (next_c == '"' || next_c == '\'')
	{
	  ++val_start;
	  std::string::size_type val_end =
	    line.find_first_of (next_c, val_start);

	  if (val_end == std::string::npos)
	    {
	      std::cerr << "Warning, bad syntax on line: "
			<< line << std::endl;
	      continue;
	    }

	  val_string = line.substr (val_start, val_end - val_start);
	  set_config (var_name, val_string);
	}
    }
}

std::string
rpie::server::get_config ()
{
  std::string output
    ("\n#6Current Game Configuration#0\n"
     "#6--------------------------#0\n");

  typedef std::map<std::string,std::string>::iterator Iter;
  for (Iter i = config_variables.begin(); i != config_variables.end(); i++)
    {
      output += "#2";
      output += i->first;
      output += " = #0\"";
      if (i->first.find("passwd") != std::string::npos)
	{
	  output += "********";
	}
      else
	{
	  output += i->second;
	}
      output += "\";\n";
    }
  return output;
}

std::string
rpie::server::get_base_path (std::string req_mode)
{
  if (req_mode == "current")
    {
      req_mode = get_config ("server_mode");
    }
  return get_config ("server_path_" + req_mode);
}
