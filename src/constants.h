//////////////////////////////////////////////////////////////////////////////
//
/// constants.h - General Pre-Defined Constants
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


#ifndef _rpie_constants_h_
#define _rpie_constants_h_


#define DEBUG_BUILD         0

#define ENCRYPT_PASS		"w01fm3a7"

#define STAFF_EMAIL		"staff@parallelrpi.com"

#define APP_EMAIL		"staff@parallelrpi.com"
#define CODE_EMAIL		"staff@parallelrpi.com"
#define PET_EMAIL		"staff@parallelrpi.com"
#define REPORT_EMAIL		"staff@parallelrpi.com"

#define IMPLEMENTOR_ACCOUNT	"Holmes"
#define SERVER_LOCATION		"http://parallelrpi.com"

#define MUD_NAME		"Parallel RPI"
#define MUD_EMAIL		"staff@parallelrpi.com"

/* Be sure to define without trailing slashes! */

#define PATH_TO_SENDMAIL	"/usr/sbin/sendmail"

/* Other miscellaneous filepath defines; absolute filepaths only! */

#define PATH_TO_WEBSITE		"/home/arpi/public_html"

#define B_BUF_SIZE				200000


/* Misc defines */

#define OOC_LOUNGE			121
#define MUSEUM_FOYER			120
#define AMPITHEATRE			70

#define JUNKYARD			75
#define LINKDEATH_HOLDING_ROOM		122

#define PREGAME_ROOM				200
#define PREGAME_ROOM_PROTOTYPE		315
#define PREGAME_ROOM_NAME		"The Pre-Commencement Room"

#define OOC_TERMINAL_OBJECT		9

#define MINAS_TIRITH_START_LOC		200
#define MINAS_MORGUL_START_LOC		6120
#define OSGILIATH_START_LOC		371
#define EDEN_START_LOC			6120
#define BOOZER				75002

#define LEANTO_OBJ_VNUM			95
#define POWER_GRID_FLUX			25
#define POWER_GRID_START_FLUX		25
#define HEALER_KIT_VNUM			900

#define LAST_PC_RACE			29
#define LAST_ROLE			9
#define MAX_SPECIAL_ROLES		50

#define CONSTITUTION_MULTIPLIER		4	/* Damage absorption limit for any */
						/* humanoid N/PC is 50 + con x multiplier */

#define COMBAT_BRUTALITY        1.00	/* A quick way to adjust the brutality of */
					/* combat; a setting of 175% seems pretty nasty, */
					/* brutish, and short. Be careful with this! */

/* color system */

#define BELL "\007"

/* main loop pulse control */

#define PULSES_PER_SEC		4

#define PULSE_ZONE		(PULSES_PER_SEC * 60)
#define PULSE_MOBILE		40
#define PULSE_STATS		(PULSES_PER_SEC * 60 * 5)
#define PULSE_AUTOSAVE		(PULSES_PER_SEC * 60)
#define PULSE_DELAY		4
#define PULSE_SMART_MOBS	(PULSES_PER_SEC * 1)
#define PULSE_MOB_SURVIVAL	(PULSES_PER_SEC * 1)

/* string stuff */

#ifndef shroud
#define MAX_STRING_LENGTH	49152
#else
#define MAX_STRING_LENGTH	49152
#endif

#define AVG_STRING_LENGTH	256	/* more useful string len */
#define ERR_STRING_LENGTH   512
#define MAX_NAME_LENGTH		15	/* player character name */

#define MAX_INPUT_LENGTH     8000
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MAX_TEXTS			 100


#define SECS_PER_REAL_MIN   60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR  (365*SECS_PER_REAL_DAY)

#define UPDATE_PULSE		(4 * 4)
#define SECOND_PULSE		(4)

#define MAX_CONNECTIONS		400

#define CURRENCY_FOOD		0
#define CURRENCY_PHOENIX	1
#define CURRENCY_BAR		2

#endif // _rpie_constants_h_
