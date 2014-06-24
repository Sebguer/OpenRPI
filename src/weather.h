//////////////////////////////////////////////////////////////////////////////
//
/// weather.h - Weather Class Structures and Functions
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2004-2006 C. W. McHenry
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


#ifndef _rpie_weather_h
#define _rpie_weather_h

#include <string>


#define WEATHER_WILDS       0   // The normal weather we might encounter
#define WEATHER_URBAN       1   // Weather in urban settings is slightly warmer
#define WEATHER_DESERT      2   // Weather in the desert is sightly cooler
#define WEATHER_UNDERGROUND 3   // Weather underground is significantly warmer, but static
#define WEATHER_CRATER      4   // Weather in craters is significantly cooler
#define WEATHER_NOLIGHT     5   // Weather in no_light is freezing, and static.

enum fog_type {
  NO_FOG,
  FOG
};

enum wind_dir_type {
  WEST_WIND,
  NORTHWEST_WIND,
  NORTH_WIND,
  NORTHEAST_WIND,
  EAST_WIND,
  SOUTHEAST_WIND,
  SOUTH_WIND,
  SOUTHWEST_WIND
};

enum wind_str_type {
  CALM,
  BREEZE,
  WINDY
};

enum sun_phase
{
   PHASE_SET,
   PHASE_PREDAWN,
   PHASE_DAWN,
   PHASE_RISEN,
   PHASE_MIDDAY,
   PHASE_DUSK,
   PHASE_EVENING
};

enum earth_phase
{
    PHASE_FULL_EARTH,
    PHASE_GIBBOUS_WANING,
    PHASE_THREE_QUARTER,
    PHASE_CRESCENT_WANING,
    PHASE_NEW_EARTH,
    PHASE_CRESCENT_WAXING,
    PHASE_FIRST_QUARTER,
    PHASE_GIBBOUS_WAXING,
	PHASE_TERRA_ECLIPSE
};

enum special_type {
  NO_EFFECT,
  FOUL_STENCH,
  VOLCANIC_SMOKE
};

class Weather
{
 private:

 public:
  int fog;
  int sunlight;
  int trend;
  int temperature;
  int state;
  int wind_dir;
  int wind_speed;
  int special_effect;

  static bool weather_unification (int zone);
};

extern Weather weather_info[];

#endif // _rpie_weather_h
