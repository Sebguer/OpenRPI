//////////////////////////////////////////////////////////////////////////////
//
/// structs.h - Data Structures
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

// 3.8.09 - Added in functionality for tastes. - K
// 3.8.09 - Added in functionality for ocues. - K

#ifndef _rpie_structs_h_
#define _rpie_structs_h_

#include <string>
#include <set>
#include <map>
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/time.h>
#include "constants.h"
#include <string>  
#include <list>  
#include <map>  
#include <set>  
#include <vector>  
#include <stdexcept>  
#include <fstream>  
#include <iostream>  
#include <sstream>  
#include <ios>  
#include <iterator>
#include <algorithm>

#include "object_damage.h"

typedef struct scent_data SCENT_DATA;
typedef struct trap_data TRAP_DATA;
typedef struct track_data TRACK_DATA;
typedef struct threat_data THREAT_DATA;
typedef struct attacker_data ATTACKER_DATA;
typedef struct newbie_hint NEWBIE_HINT;
typedef struct role_data ROLE_DATA;
typedef struct group_data GROUP_DATA;
typedef struct enchantment_data ENCHANTMENT_DATA;
typedef struct known_spell_data KNOWN_SPELL_DATA;
typedef struct spell_table_data SPELL_TABLE_DATA;
typedef struct help_info HELP_INFO;
typedef struct writing_data WRITING_DATA;
typedef struct site_info SITE_INFO;
typedef struct wound_data WOUND_DATA;
typedef struct affected_type AFFECTED_TYPE;
typedef struct alias_data ALIAS_DATA;
typedef struct board_data BOARD_DATA;
typedef struct char_ability_data CHAR_ABILITY_DATA;
typedef struct char_data CHAR_DATA;
typedef struct combat_data COMBAT_DATA;
typedef struct combat_msg_data COMBAT_MSG_DATA;
typedef struct delayed_affect_data DELAYED_AFFECT_DATA;
typedef struct dream_data DREAM_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct default_item_data DEFAULT_ITEM_DATA;
typedef struct craft_variable_data CRAFT_VARIABLE_DATA;
typedef struct craft_oval_data CRAFT_OVAL_DATA;
typedef struct help_data HELP_DATA;
typedef struct memory_t MEMORY_T;
typedef struct mudmail_data MUDMAIL_DATA;
typedef struct mob_data MOB_DATA;
typedef struct mobprog_data MOBPROG_DATA;
typedef struct move_data MOVE_DATA;
typedef struct name_switch_data NAME_SWITCH_DATA;
typedef struct negotiation_data NEGOTIATION_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct pc_data PC_DATA;
typedef struct monitor_data MONITOR_DATA;
typedef struct phase_data PHASE_DATA;
typedef struct qe_data QE_DATA;
typedef struct reset_affect RESET_AFFECT;
typedef struct reset_data RESET_DATA;
typedef struct reset_time_data RESET_TIME_DATA;
typedef struct room_direction_data ROOM_DIRECTION_DATA;
typedef struct room_extra_data ROOM_EXTRA_DATA;
typedef struct room_prog ROOM_PROG;
typedef struct mobile_ai MOB_AI;
typedef struct shop_data SHOP_DATA;
typedef struct second_affect SECOND_AFFECT;
typedef struct social_data SOCIAL_DATA;
typedef struct spell_data SPELL_DATA;
typedef struct subcraft_head_data SUBCRAFT_HEAD_DATA;
typedef struct text_data TEXT_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct var_data VAR_DATA;
typedef struct written_descr_data WRITTEN_DESCR_DATA;
typedef struct lodged_object_info LODGED_OBJECT_INFO;
typedef struct lodged_missile_info LODGED_MISSILE_INFO;
typedef struct body_info BODY_INFO;
typedef struct sighted_data SIGHTED_DATA;
typedef struct obj_clan_data OBJ_CLAN_DATA;
typedef struct variable_data VARIABLE_DATA;
typedef struct mvariable_data MVARIABLE_DATA;
typedef struct mob_variable_data MOB_VARIABLE_DATA;
typedef struct foraged_good FORAGED_GOOD;
typedef struct foraged_zone FORAGED_ZONE;
typedef struct defined_scent DEFINED_SCENT;


typedef struct arena_gladiators ARENA_GLADIATORS;
typedef struct arena_fights ARENA_FIGHTS;
typedef struct arena_bets ARENA_BETS;

typedef struct overwatch_enemy OVERWATCH_ENEMY;
typedef struct targeted_bys	TARGETED_BY;

#ifndef __cplusplus
typedef char bool;
#endif
typedef char rpi_byte;
typedef unsigned long bitflag;
typedef signed char shortint;

#define TREAT_ALL		(1 << 0)
#define TREAT_SLASH		(1 << 1)
#define TREAT_BLUNT		(1 << 2)
#define TREAT_PUNCTURE	(1 << 3)
#define TREAT_BURN		(1 << 6)
#define TREAT_FROST		(1 << 7)
#define TREAT_BLEED		(1 << 8)
#define TREAT_INFECTION	(1 << 9)  // for future expansion
#define TREAT_INTERNAL  (1 << 10) // as the good book says.

#define MONITOR_NOTHINK		(1 << 0)
#define MONITOR_NOFEEL		(1 << 1)
#define MONITOR_NOSPEECH		(1 << 2)
#define MONITOR_NOEMOTE		(1 << 3)
#define MONITOR_NOMOVEMENT	(1 << 4)
#define MONITOR_NORADIO		(1 << 5)
#define MONITOR_NOCOMBAT		(1 << 6)
#define MONITOR_IGNORE_CHAN	(1 << 7)

#define SEARCH_KEYWORD		1
#define SEARCH_SDESC		2
#define SEARCH_LDESC		3
#define SEARCH_FDESC		4
#define SEARCH_RACE		5
#define SEARCH_CLAN		6
#define SEARCH_SKILL		7
#define SEARCH_ROOM		8
#define SEARCH_LEVEL		9
#define SEARCH_STAT		10

/* Second Affects */
#define SA_STAND		1
#define SA_GET_OBJ		2
#define SA_WEAR_OBJ		3
#define SA_CLOSE_DOOR		4
#define SA_WORLD_SWAP		5
#define SA_COMBAT		6
#define SA_UNUSED_2		7
#define SA_UNUSED_3		8
#define SA_UNUSED_4		9
#define SA_UNUSED_5		10
#define SA_UNUSED_6		11
#define SA_WALK			12
#define SA_RUN			13
#define SA_FOLLOW		14
#define SA_SWIM			15
#define SA_SHADOW		16
#define SA_FLOOD		17
#define SA_KNOCK_OUT		18
#define SA_ESCAPE		19
#define SA_MOVE			20
#define SA_RESCUE		21
#define SA_COMMAND 	 	22
#define SA_AIMSTRIKE            23
#define SA_LEAD                 24  // Japheth's suggested workaround for group movement messages.
#define SA_GFOLLOW              25
#define SA_PUSH                 26 // Delay for pushing in to a room.
#define SA_DOANYWAY             27
#define SA_AMBUSH               28 // For attacking whilst hidden.
#define SA_PREYHIDE             29
#define SA_OUTDOOR              30 // For people moving outdoors.
#define SA_EMOTE                31 // Emote workaround
#define SA_FEINT                32 // Feinted attacks.
#define SA_WARDED               33 // You've been warded.
#define SA_COMBAT_DOOR          34 // You're trying to open/close a door in combat.
#define SA_POINTSTRIKE		    35
#define SA_GRENADE              36
#define SA_ROOMSHOTS			37 // 2-second counter that reports on how many shorts were fired every 2 seconds
#define SA_DISTSHOTS			38 // same, but for shots in the distance.
#define SA_ROOMFIRED			39
#define SA_INVOLCOVER			40 // forced to take involuntary cover.


#define VNUM_PENNY		40
#define VNUM_FARTHING		41
#define VNUM_MONEY		42	/* The answer to everything */
#define VNUM_JAILBAG		43
#define VNUM_TICKET		44
#define VNUM_HEAD		45
#define VNUM_CORPSE		46
#define VNUM_SPEAKER_TOKEN	175
#define VNUM_STATUE		195
#define VNUM_ORDER_TICKET	575
#define VNUM_WHOLEARM           47
#define VNUM_WHOLELEG           50
#define VNUM_LOWERTORSO         48
#define VNUM_UPPERTORSO         49
#define VNUM_ZOMBIE_HEAD        51
#define VNUM_CHEM_SMEAR         52
#define VNUM_CHEM_EMPTY_VIAL    53
#define VNUM_CHEM_FAILED_VIAL   54
#define VNUM_TRAP_OBJECT        999
#define VNUM_CARDS              998


#define fSHELL			13950
#define fBULLET			13900

#define VNUM_DOER               666


#define VNUM_BOT_DROID        6900
#define VNUM_BOT_SCAVENGER    6901
#define VNUM_BOT_PEEPER       6902
#define VNUM_BOT_SENTRY       6903
#define VNUM_BOT_MINER        6904
#define VNUM_BOT_HAULER       6905
#define VNUM_BOT_COMBATANT    6906

#define VNUM_TIRITH_REPOS		3425
#define VNUM_HULE_RELEASE_ROOM	        12032

#define VNUM_WELLGROOMED_RIDING_HORSE	29
#define VNUM_WELLGROOMED_WAR_HORSE	31

//#define HOLIDAY_METTARE		1
//#define HOLIDAY_YESTARE		2
//#define HOLIDAY_ENDERI1		3
//#define HOLIDAY_ENDERI2		4
//#define HOLIDAY_ENDERI3		5

//#define SPRING	        0
//#define SUMMER  	        1
//#define AUTUMN		    2
//#define WINTER	        3

// We have two moon seasons: near and far on our axial tilt. Modifies temperature a little.
#define SEASON_NEAR     0
#define SEASON_FAR      1

#define BASE_SPELL		1000

#define BLEEDING_INTERVAL 2	/* 2 minutes per "bleeding pulse". */
#define BLEEDING_WILDLIFE_INTERVAL 1
#define BASE_SPECIAL_HEALING 15	/* Increased healing rate, for special mobs/PCs. */
#define BASE_PC_HEALING 40	/* Number of minutes per healing pulse for standard PCs. */
#define BASE_STARVE_HEALING 65

/* debug parameters */

#define DEBUG_FIGHT		1
#define DEBUG_MISC		2
#define DEBUG_SKILLS	4
#define DEBUG_SUMMARY	8
#define DEBUG_SUBDUE	16

/* registry */

#define REG_REGISTRY		0
#define REG_MAXBONUS		1
#define REG_SPELLS			2
#define REG_DURATION		3
#define REG_OV				4
#define REG_LV				5
#define REG_CAP				6
#define REG_SKILLS			7	/* Link to DURATION, OV, LV, CAP */
#define REG_MISC			8
#define REG_MISC_NAMES		9	/* Link to MISC */
#define REG_MISC_VALUES		10
#define REG_MAGIC_SPELLS	11
#define REG_MAGIC_SPELLS_D	12	/* Linked to MAGIC_SPELLS */
#define REG_MAX_RATES		13	/* Learning rates table skill vs formula */
#define REG_CRAFT_MAGIC		14

#define MISC_DELAY_OFFSET	0
#define MISC_MAX_CARRY_N	1
#define MISC_MAX_CARRY_W	2
#define MISC_MAX_MOVE		3
#define MISC_STEAL_DEFENSE	4

#define CRIM_FIGHTING	5
#define CRIM_STEALING	3

/* vehicle types */

#define VEHICLE_BOAT	1
#define VEHICLE_HITCH	2


/* generic find defines */

#define FIND_CHAR_ROOM    	(1<<0)
#define FIND_CHAR_WORLD    	(1<<1)
#define FIND_OBJ_INV      	(1<<2)
#define FIND_OBJ_ROOM     	(1<<3)
#define FIND_OBJ_WORLD   	(1<<4)
#define FIND_OBJ_EQUIP   	(1<<5)

/* mob/object hash */

#define GEN_MOB 	1
#define GEN_OBJ 	2
#define GEN_WLD 	3
#define MAX_HASH    1000	/* 100 vnums per hash */

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif


#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

/* Weather-room descrition constants - see weather_room in constants.c */

#define		WR_DESCRIPTIONS		12	/* Doesn't include WR_NORMAL */

#define		WR_FOGGY		0
#define		WR_CLOUDY		1
#define		WR_RAINY		2
#define		WR_STORMY		3
#define		WR_SNOWY		4
#define		WR_BLIZARD		5
#define		WR_NIGHT		6
#define		WR_NIGHT_FOGGY		7
#define		WR_NIGHT_RAINY		8
#define		WR_NIGHT_STORMY		9
#define		WR_NIGHT_SNOWY		10
#define		WR_NIGHT_BLIZARD	11
#define		WR_NORMAL		12

/* For 'type_flag' */

#define ITEM_LIGHT          1
#define ITEM_PART_BOX       2
#define ITEM_TRAP_BIT       3
#define ITEM_CHEM_VIAL      4
#define ITEM_WEAPON         5
#define ITEM_SHIELD 	    6
#define ITEM_MISSILE        7
#define ITEM_CHEM_KIT       8
#define ITEM_ARMOR          9
#define ITEM_CHEM_NEEDLE    10
#define ITEM_WORN      	    11
#define ITEM_OTHER     	    12
#define ITEM_TRASH     	    13
#define ITEM_TRAP      	    14
#define ITEM_CONTAINER 	    15
#define ITEM_CHEM_SPRAY     16
#define ITEM_DRINKCON       17
#define ITEM_KEY       	    18
#define ITEM_FOOD      	    19
#define ITEM_MONEY     	    20
#define ITEM_CHEM_BOX	    21
#define ITEM_BOARD     	    22
#define ITEM_FOUNTAIN  	    23
#define ITEM_FIREARM	    28	// Firearms
#define ITEM_CLUSTER        29
#define ITEM_COMPONENT	    30
#define ITEM_ROUND          31  // Rounds
#define ITEM_SALVE	        32
#define ITEM_POISON_LIQUID  33
#define ITEM_LOCKPICK	    34
#define ITEM_BOW_STRING     35
#define ITEM_TREE           36
#define ITEM_CLIP    	    37  // Magazines
#define ITEM_CASE           38  // Casings, discards from gunfire
#define ITEM_PROGRESS 	    39
#define ITEM_BULLET		    40  // bullets, discards from gunfire
#define ITEM_POISON_PASTE	41
#define ITEM_BRIDLE			43
#define ITEM_TICKET			44
#define ITEM_HEAD			45
#define ITEM_HOLSTER		46
#define ITEM_CARCASS		47
#define ITEM_AMMO_BELT		48
#define ITEM_BANDOLIER		49
#define ITEM_FLUID			50
#define ITEM_LIQUID_FUEL	51
#define ITEM_HEALER_KIT		52
#define ITEM_PARCHMENT		53
#define ITEM_BOOK		    54
#define ITEM_WRITING_INST	55
#define ITEM_INK		    56
#define ITEM_QUIVER		    57
#define ITEM_SHEATH		    58
#define ITEM_KEYRING		59
#define ITEM_COVER			60
#define ITEM_NPC_OBJECT		61
#define ITEM_DWELLING		62
#define ITEM_RESOURCE		63
#define ITEM_REPAIR_KIT		64
#define ITEM_TOSSABLE		65
#define ITEM_CARD		    66
#define ITEM_MERCH_TICKET	67
#define ITEM_ROOM_RENTAL	68
#define ITEM_SMALLGOOD      69
#define ITEM_BREATHER       70
#define ITEM_CRATE	        71
#define ITEM_SCRAP          72

#define ITEM_GRENADE        74
#define ITEM_SLING          75

#define ITEM_TOOL           77
#define ITEM_CHEM_SMOKE     78
#define ITEM_CHEM_WASH      79
#define ITEM_TRADE_BUNDLE   80
#define ITEM_TRADE_MARKET   81
#define ITEM_RENT_TICKET    82
#define ITEM_RENT_SHOP      83

#define ITEM_E_RADIO	    24
#define ITEM_E_LIGHT	    25
#define ITEM_E_PHONE	    26
#define ITEM_E_BATTERY	    27
#define ITEM_E_BOOST		42  // gives you a skill_boost if you're holding it/wearing it and it's got power.
#define ITEM_E_REMOTE		73
#define ITEM_E_ROBOT		76
#define ITEM_E_BUG			84
#define ITEM_E_CLOCK		85
#define ITEM_E_MEDICAL		86
#define ITEM_E_GOGGLE		87
#define ITEM_E_BOOK         91

#define ITEM_TRADE_STALL	88
#define ITEM_TURF_PROP		89

#define ITEM_ARTWORK		90

// ITEM_E_BOOK uses 91

#define ITEM_E_BREATHER     92

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE           ( 1 << 0 )
#define ITEM_WEAR_FINGER    ( 1 << 1 )
#define ITEM_WEAR_NECK      ( 1 << 2 )
#define ITEM_WEAR_BODY      ( 1 << 3 )
#define ITEM_WEAR_HEAD      ( 1 << 4 )
#define ITEM_WEAR_LEGS      ( 1 << 5 )
#define ITEM_WEAR_FEET      ( 1 << 6 )
#define ITEM_WEAR_HANDS     ( 1 << 7 )
#define ITEM_WEAR_ARMS      ( 1 << 8 )
#define ITEM_WEAR_SHIELD    ( 1 << 9 )
#define ITEM_WEAR_ABOUT		( 1 << 10 )
#define ITEM_WEAR_WAIST   	( 1 << 11 )
#define ITEM_WEAR_WRIST    	( 1 << 12 )
#define ITEM_WIELD         	( 1 << 13 )
#define ITEM_HOLD         	( 1 << 14 )
#define ITEM_THROW        	( 1 << 15 )
#define ITEM_WEAR_OVERWEAR	( 1 << 16 )
#define ITEM_WEAR_SHEATH	( 1 << 17 )
#define ITEM_WEAR_BELT		( 1 << 18 )
#define ITEM_WEAR_BACK		( 1 << 19 )
#define ITEM_WEAR_BLINDFOLD	( 1 << 20 )
#define ITEM_WEAR_THROAT	( 1 << 21 )
#define ITEM_WEAR_EAR		( 1 << 22 )
#define ITEM_WEAR_SHOULDER	( 1 << 23 )
#define ITEM_WEAR_ANKLE		( 1 << 24 )
#define ITEM_WEAR_HAIR		( 1 << 25 )
#define ITEM_WEAR_FACE		( 1 << 26 )
#define ITEM_WEAR_ARMBAND	( 1 << 27 )	/* Shoulder Patches */
#define ITEM_WEAR_OVER      ( 1 << 28 )
#define ITEM_WEAR_EYES      ( 1 << 29 )
#define ITEM_WEAR_UNDERWEAR ( 1 << 30 )


/* NOTE: UPDATE wear_bits in constants.c */

/* Bitvector for 'extra_flags' */

#define ITEM_DESTROYED      ( 1 << 0 )
#define ITEM_PITCHABLE      ( 1 << 1 )	/* Dwelling that needs pitching before use. */
#define ITEM_INVISIBLE      ( 1 << 2 )
#define ITEM_MAGIC          ( 1 << 3 )
#define ITEM_NOREMOVE       ( 1 << 4 )
#define ITEM_FURNISH        ( 1 << 5 )  // Used for poisons that are "good" for you.
#define ITEM_GET_AFFECT	    ( 1 << 6 )
#define ITEM_DROP_AFFECT    ( 1 << 7 )
#define ITEM_MULTI_AFFECT   ( 1 << 8 )
#define ITEM_WEAR_AFFECT    ( 1 << 9 )
#define ITEM_WIELD_AFFECT   ( 1 << 10 )
#define ITEM_HIT_AFFECT	    ( 1 << 11 )
#define ITEM_OK	      		( 1 << 12 )
#define ITEM_NOPURGE	    ( 1 << 13 ) // have to speficially purge this item.
#define ITEM_LEADER	        ( 1 << 14 )
#define ITEM_MEMBER	        ( 1 << 15 )
#define ITEM_DECORATABLE    ( 1 << 16 )
#define ITEM_ILLEGAL	    ( 1 << 17 )
#define ITEM_SCENTED	    ( 1 << 18 )
#define ITEM_MASK	        ( 1 << 19 )
#define ITEM_MOUNT			( 1 << 20 )
#define ITEM_TABLE			( 1 << 21 )
#define ITEM_STACK			( 1 << 22 )	/* Item stack with same vnum objs */
#define ITEM_VNPC_DWELLING	( 1 << 23 )	/* Becomes vNPC dwelling after PC logout */
#define ITEM_LOADS			( 1 << 24 )	/* Item is loaded by a reset */
#define ITEM_VARIABLE		( 1 << 25 )
#define ITEM_TIMER		    ( 1 << 26 )	/* Will decay */
#define ITEM_PC_SOLD		( 1 << 27 )	/* Sold to shopkeep by PC. */
#define ITEM_THROWING		( 1 << 28 )	/* Weapon is suitable for throwing */
#define ITEM_NEWSKILLS		( 1 << 29 )	/* Doesn't need to be converted */
#define ITEM_PITCHED		( 1 << 30 )	/* Whether tent has been pitched or not */
#define ITEM_VNPC		    ( 1 << 31 )	/* Item exists but isn't visible to players */

/* Flags for extra_Flags2 */
#define ITEM_CONCEALED	    ( 1 << 0 )	/* Item exists but isn't visible to players */

/* Some different kind of liquids */
#define LIQ_WATER	    0
#define LIQ_ALE         1
#define LIQ_BEER        2
#define LIQ_CIDER	    3
#define LIQ_MEAD	    4
#define LIQ_WINE	    5
#define LIQ_BRANDY	    6
#define LIQ_SOUP	    7
#define LIQ_MILK	    8
#define LIQ_TEA		    9
#define LIQ_SALTWATER	10

/* for containers  - value[1] */

#define CONT_CLOSEABLE  ( 1 << 0 )
#define CONT_PICKPROOF  ( 1 << 1 )
#define CONT_CLOSED     ( 1 << 2 )
#define CONT_LOCKED     ( 1 << 3 )
#define CONT_TRAP       ( 1 << 4 )
#define CONT_BEHEADED	( 1 << 5 )

/* harsh land object flags */

#define HO_CLONE	(1<<0)
#define HO_LIMIT   	(1<<1)

#define OBJ_NOTIMER	-7000000
#define NOWHERE    	-1

/* Bitvector for obj 'tmp_flags' */

#define SA_DROPPED	( 1 << 0 )

/* Bitvector For 'room_flags' */

#define DARK       	( 1 << 0 )  // it's dark
#define NO_LOG      	( 1 << 1 )  // can't log
#define NO_MOB     	( 1 << 2 )  // mobiles avoid
#define INDOORS    	( 1 << 3 )  // inside
#define LAWFUL     	( 1 << 4 )  // attack incurs problems
#define SMALL_ROOM 	( 1 << 5 )  // room is small for a variety of purposes, mostly to do with combat and gunfie
#define BIG_ROOM   	( 1 << 6 )  // room is big for a variety of purposes, mostly to do with combat and gunfie
#define BRAWL       ( 1 << 7 )  // you can brawl in safety here
#define SAFE_Q     	( 1 << 8 )  // can quit
#define OOC			( 1 << 9 ) // OOC room
#define FALL		( 1 << 10 ) // autofall
#define LOWAIR  	( 1 << 11 ) // low oxygen
#define	VEHICLE		( 1 << 12 ) // inside something
#define STIFLING_FOG	( 1 << 13 ) // can't shoot
#define NO_MERCHANT	( 1 << 14 ) // merchants avoid
#define CLIMB		( 1 << 15 ) // climb test
#define SAVE		( 1 << 16 ) // defunct?
#define PEACE		( 1 << 17 ) // Can't fight here.
#define MED_COVER	( 1 << 18 ) // Can get medium-cover.
#define HIGH_COVER	( 1 << 19 ) // Can get high-cover.
#define MEDIUM_ROOM ( 1 << 20 ) // tells a room to not calculate player weight in the capacity
#define CULTIVATED	( 1 << 21 ) // movement control
#define TEMPORARY	( 1 << 22 ) // movement control
#define OPEN		( 1 << 23 ) // movement control
#define ROCKY		( 1 << 24 ) // movement control
#define VEGETATED	( 1 << 25 ) // movement control
#define LIGHT		( 1 << 26 ) // casts light
#define NOHIDE		( 1 << 27 ) // can't hide
#define STORAGE		( 1 << 28 ) // items don't decay
#define PC_ENTERED	( 1 << 29 ) // a PC's been here since reboot
#define ROOM_OK		( 1 << 30 )
#define NOAIR 		( 1 << 31 ) // Oxygen-deprived room, players slowly suffocate
#define FORT	   	( 1 << 32 )  // can fortify a squeezable zone



/* For 'dir_option' */

#define NORTH		0
#define EAST		1
#define SOUTH		2
#define WEST		3
#define UP		4
#define DOWN		5
#define OUTSIDE		6
#define INSIDE		7
#define NORTHEAST       8
#define NORTHWEST       9
#define SOUTHEAST       10
#define SOUTHWEST       11


#define LAST_DIR	SOUTHWEST

/* exit_info */

#define EX_ISDOOR       ( 1 << 0 )
#define EX_CLOSED	    ( 1 << 1 )
#define EX_LOCKED	    ( 1 << 2 )
#define EX_RSCLOSED	    ( 1 << 3 )
#define EX_RSLOCKED	    ( 1 << 4 )
#define EX_PICKPROOF	( 1 << 5 )
#define EX_SECRET   	( 1 << 6 )
#define EX_TRAP	    	( 1 << 7 )
#define EX_TOLL			( 1 << 8 )
#define EX_ISGATE       ( 1 << 9 )

/* For 'Sector types' */

#define SECT_INSIDE	    0
#define SECT_MOONPLAIN		1
#define SECT_MOONHILL		2
#define SECT_MOONMOUNTAIN	3
#define SECT_MOONSAND		4
#define SECT_MOONCAVE		5
#define SECT_RUINSTREET		6
#define SECT_RUINFACTORY	7
#define SECT_RUINOFFICE		8
#define SECT_RUINAPARTMENT	9
#define SECT_RUINHIGHWAY	10
#define SECT_RUININSIDE		11
#define SECT_RUINOUTSIDE	12
#define SECT_SPACE		13
#define SECT_PIT	    14
#define SECT_LEANTO	    15
#define SECT_LAKE	    16
#define SECT_RIVER	    17
#define SECT_OCEAN	    18
#define SECT_REEF	    19
#define SECT_UNDERWATER	    20
#define SECT_ZEROG		21
#define SECT_MOONDARK       22
#define SECT_MOONLIGHT      23
#define SECT_RUINLAB        24
#define SECT_OUTSIDE        25
#define SECT_SPACESHIP      26
#define SECT_RUINGYM        27
#define SECT_RUINUTIL       28
#define SECT_RUINRELIG      29
#define SECT_RUINSHOP		30
#define SECT_RUINGENERATOR	31
#define SECT_RUINCHEMLAB	32
#define SECT_RUINWORKSHOP	33
#define SECT_RUINKITCHEN	34
#define SECT_RUINDOMICILE	35
#define SECT_FREEFALL 		36

/* ---- For new herb stuff ---- */

/* Herb sectors */

#define HERB_NUMSECTORS		4

#define HERB_LAB		0
#define HERB_OFFICE		1
#define HERB_APARTMENT		2
#define HERB_FACTORY		3

#define HERB_RARITIES		5

#define MAX_HERBS_PER_ROOM	4

#define HERB_RESET_DURATION	4

/* For 'equip' */

#define WEAR_LIGHT		0
#define WEAR_FINGER_R		1
#define WEAR_FINGER_L		2
#define WEAR_NECK_1		3
#define WEAR_NECK_2		4
#define WEAR_BODY		5
#define WEAR_HEAD		6
#define WEAR_LEGS		7
#define WEAR_FEET		8
#define WEAR_HANDS		9
#define WEAR_ARMS		10
#define WEAR_SHIELD		11
#define WEAR_ABOUT		12
#define WEAR_WAIST		13
#define WEAR_WRIST_R		14
#define WEAR_WRIST_L		15
#define WEAR_PRIM		16
#define WEAR_SEC		17
#define WEAR_BOTH		18
#define WEAR_OVERWEAR	19
#define WEAR_BELT_1		20
#define WEAR_BELT_2		21
#define WEAR_BACK		22
#define WEAR_BLINDFOLD		23
#define WEAR_THROAT		24
#define WEAR_EAR		25
#define WEAR_SHOULDER_R		26
#define WEAR_SHOULDER_L		27
#define WEAR_ANKLE_R		28
#define WEAR_ANKLE_L		29
#define WEAR_HAIR		30
#define WEAR_FACE		31
#define WEAR_CARRY_R		32
#define WEAR_CARRY_L		33
#define WEAR_ARMBAND_R		34
#define WEAR_ARMBAND_L		35
#define WEAR_OVER        36
#define WEAR_EYES        37
#define WEAR_UNDERWEAR   38
#define WEAR_BLANK		 39

#define MAX_WEAR        	WEAR_BLANK + 1

#define MAX_COMBAT_SKILL	11
#define MAX_SKILLS		150
#define MAX_SPELLS      	100
#define MAX_AFFECT		25


/* conditions */

#define DRUNK			0
#define FULL			1
#define THIRST			2

/* Bitvector for 'affected_by' */

#define AFF_UNDEF1              ( 1 << 0 )
#define AFF_INVISIBLE	        ( 1 << 1 )
#define AFF_INFRAVIS        	( 1 << 2 )
#define AFF_DETECT_INVISIBLE 	( 1 << 3 )
#define AFF_DETECT_MAGIC     	( 1 << 4 )
#define AFF_SENSE_LIFE       	( 1 << 5 )
#define AFF_TRANSPORTING	( 1 << 6 ) // Mobile is carrying cargo.
#define AFF_SANCTUARY		( 1 << 7 )
#define AFF_GROUP            	( 1 << 8 )
#define AFF_CURSE            	( 1 << 9 )
#define AFF_FLAMING          	( 1 << 10 )
#define AFF_POISON           	( 1 << 11 )
#define AFF_SCAN		( 1 << 12 )
#define AFF_FALLBACK	        ( 1 << 13 ) // Mobile retreating to its fallback value.
/* define AFF_???		( 1 << 14 ) ????? */
/* define AFF_???		( 1 << 15 ) ????? */
/* define AFF_SLEEP???		( 1 << 16 ) ????? */
/* define AFF_DODGE???		( 1 << 17 ) ????? */
#define AFF_SNEAK	        ( 1 << 18 ) // Mobile attempts to sneak with every movement.
#define AFF_HIDE		    ( 1 << 19 ) // Mobile will attempt to hide until hidden.
/* define AFF_FEAR???		( 1 << 20 ) ????? */
#define AFF_FOLLOW           	( 1 << 21 ) // Mobile is following.
#define AFF_HOODED              ( 1 << 22 ) // Mobile is hooded.
/* define AFF_CHARM???		( 1 << 23 ) ????? */
#define AFF_SUNLIGHT_PEN	( 1 << 23 )
#define AFF_SUNLIGHT_PET	( 1 << 24 )
#define AFF_BREATHE_WATER	( 1 << 25 )

/* modifiers to char's abilities */

#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT		    3
#define APPLY_CHA		    4
#define APPLY_AUR		    5
#define APPLY_WIL           6
#define APPLY_CON           7
#define APPLY_SEX           8
#define APPLY_AGE           9
#define APPLY_CHAR_WEIGHT	10
#define APPLY_CHAR_HEIGHT	11
#define APPLY_DEFENSE		12	/* Free - APPLY_DEFENSE not used */
#define APPLY_HIT		    13
#define APPLY_MOVE		    14
#define APPLY_CASH		    15
#define APPLY_AC		    16
#define APPLY_ARMOR		    16
#define APPLY_OFFENSE		17	/* Free - APPLY_OFFENSE not used */
#define APPLY_DAMROLL		18
#define APPLY_SAVING_PARA	19
#define APPLY_SAVING_ROD	20
#define APPLY_SAVING_PETRI	21
#define APPLY_SAVING_BREATH	22
#define APPLY_SAVING_SPELL	23
#define APPLY_AGI			24

/* Above 100, don't reapply upon restore of character. */

#define APPLY_BRAWLING		123
#define APPLY_CLUB		    124
#define APPLY_SPEAR		    125
#define APPLY_SWORD		    126
#define APPLY_DAGGER		127
#define APPLY_AXE		    128
#define APPLY_WHIP		    129
#define APPLY_POLEARM		130
#define APPLY_DUAL		    131
#define APPLY_BLOCK		    132
#define APPLY_PARRY		    133
#define APPLY_SUBDUE		134
#define APPLY_DISARM		135
#define APPLY_SNEAK		    136
#define APPLY_HIDE 		    137
#define APPLY_STEAL		    138
#define APPLY_PICK		    139
#define APPLY_SEARCH		140
#define APPLY_LISTEN		141
#define APPLY_FORAGE		142
#define APPLY_RITUAL		143
#define APPLY_SCAN		    144
#define APPLY_BACKSTAB 		145
#define APPLY_BARTER		146
#define APPLY_RIDE		    147
#define APPLY_CLIMB		    148
#define APPLY_PEEP		    149	/* Obsoleted */
#define APPLY_HUNT		    150
#define APPLY_SKIN		    151
#define APPLY_SAIL		    152
#define APPLY_POISONING		153
#define APPLY_ALCHEMY		154
#define APPLY_HERBALISM		155
#define APPLY_SMALL_BLADE		156
#define APPLY_FLAIL		157

#define FIRST_APPLY_SKILL	APPLY_BRAWLING
#define LAST_APPLY_SKILL 	APPLY_FLAIL

/* NOTE:  Change affect_modify in handler.c if new APPLY's are added */

/* sex */

#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */

#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_UNCONSCIOUS 2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8

/* for mobile actions: specials.act */

#define ACT_MEMORY	    ( 1 << 0 ) // Mobile attacks previous attackers on sight.
#define ACT_SENTINEL    ( 1 << 1 )  // Mobile doesn't wander, plus some combat differences
#define ACT_RESCUER   ( 1 << 2 )  // Mobile draws fire from friends in combat.
#define ACT_ISNPC       ( 1 << 3 )  // Self explanatory
#define ACT_NOVNPC	    ( 1 << 4 )	/* Shopkeep doesn't have vNPC buyers */
#define ACT_AGGRESSIVE 	( 1 << 5 )  // Mobile attacks unclanned N/PCs on sight.
#define ACT_STAY_ZONE  	( 1 << 6 )  // Mobile supposedly doesn't leave the zone. Might be defunct.
#define ACT_FIXER    	( 1 << 7 )  // Mobile can repair items.
#define ACT_SAGGRESS	( 1 << 8 )  // Mobile will only attack things in its room.
#define ACT_BULKTRADER	( 1 << 9 ) // Resource routine for shopkeeper.
#define ACT_SHOOTER 	( 1 << 10 ) // Mobile will continuely check ranged combat routines.
#define ACT_NOBUY	    ( 1 << 11 ) // Merchant doesn't buy things full stop.
#define ACT_ENFORCER	( 1 << 12 ) // Enforcer routines.
#define ACT_PACKANIMAL	( 1 << 13 )  // Mobile can be used to carry resources.
#define ACT_VEHICLE	( 1 << 14 ) // Vehicle rotuines.
#define ACT_STOP	( 1 << 15 )	/* Lines up with PLR_STOP */
#define ACT_SQUEEZER	( 1 << 16 ) // Allowed to enter rooms past their cap, e.g. ghosts
#define ACT_PARIAH	( 1 << 17 ) // Can kill target without being flagged wanted.
#define ACT_MOUNT	( 1 << 18 ) // Mount routines.
#define ACT_SCENT       ( 1 << 19 )	/* The mobile has scent */
#define ACT_PCOWNED	( 1 << 20 ) // PC owned. Suckers!
#define ACT_WILDLIFE	( 1 << 21 )	/* Wildlife doesn't attack wildlife */
#define ACT_STAYPUT	( 1 << 22 )	/* Saves between reboots */
#define ACT_PASSIVE	( 1 << 23 )	/* Can be fired at with no response */
#define ACT_AUCTIONEER 	( 1 << 24 ) /* Mobile is an NPC auctioneer - auctions.cpp */
#define ACT_ECONZONE	( 1 << 25 )	/* NPC uses econ zone discount/markups */
#define ACT_JAILER	( 1 << 26 )	/* New jailer flag for NPCs */
#define ACT_NOBIND	( 1 << 27 ) // Mobile doesn't bind itself if bleeding.
#define ACT_NOBLEED	( 1 << 28 ) // Mobile doesn't bleed e.g. skeletons/zombies.
#define ACT_FLYING	( 1 << 29 ) // Mobile flies.
#define ACT_PHYSICIAN	( 1 << 30 ) // Mobile offers healinf services.
#define ACT_PREY	( 1 << 31 )	/* Animals may only be ambushed or hit with ranged weapons */

// TALENTS - Extra effects and abilities, plus it also keeps track of how
// people have upgraded their stats.

#define TAL_STR1        ( 1 << 0 )
#define TAL_AGI1        ( 1 << 1 )
#define TAL_DEX1        ( 1 << 2 )
#define TAL_CON1        ( 1 << 3 )
#define TAL_INT1        ( 1 << 4 )
#define TAL_WIL1        ( 1 << 5 )
#define TAL_AUR1        ( 1 << 6 )
#define TAL_STR2        ( 1 << 7 )
#define TAL_AGI2        ( 1 << 8 )
#define TAL_DEX2        ( 1 << 9 )
#define TAL_CON2        ( 1 << 10 )
#define TAL_INT2        ( 1 << 11 )
#define TAL_WIL2        ( 1 << 12 )
#define TAL_AUR2        ( 1 << 13 )
#define TAL_CLIMB       ( 1 << 14 )  // Get a bonus to climb.
#define TAL_SWIM        ( 1 << 15 )  // Get a bonus to swim.
#define TAL_FLEE        ( 1 << 16 )  // Get a bonus to flee vs groups.
#define TAL_HAWKEYES    ( 1 << 17 )  // Can see bigger things.
#define TAL_METABOLISM  ( 1 << 18 )  // Hunger at half pace.
#define TAL_PICKSENSE   ( 1 << 19 )  // Get a bonus to seeing pickpockets.
#define TAL_DISCERNING  ( 1 << 20 )  // Always can tell exact weight and cost of objects.
#define TAL_MEMORY      ( 1 << 21 )  // Gets an extra fifty skill points to use.
#define TAL_MUMBLER     ( 1 << 22 )  // -much- harder to overhear what you say and whisper.
#define TAL_COMBATANT   ( 1 << 23 )  // Very small boost to combat, 1 point.


/* For players : specials.act */

#define PLR_QUIET	    ( 1 << 19 )	/* WAS ( 1 << 4 ) - conflicts with ACT_NOVNPC */
#define PLR_STOP	    ( 1 << 15 )

/* harsh land specific mobile flags */

#define HM_CLONE	(1<<0)
#define HM_KEEPER  	(1<<1)

/* Target classes: */

#define TAR_AREA            0
#define TAR_CHAR_OFFENSIVE  1
#define TAR_CHAR_DEFENSIVE  2
#define TAR_CHAR_SELF       3
#define TAR_OBJ             4
#define TAR_OBJ_INV         5
#define TAR_CHAR_SECRET     6
#define TAR_IGNORE          7

/* Begin magic defines */

#define MAX_LEARNED_SPELLS		100

#define CASTER_NONE			0
#define CASTER_WHITE			1
#define CASTER_GREY			2
#define CASTER_BLACK			3

#define SPHERE_EARTH			0
#define SPHERE_AIR			1
#define SPHERE_FIRE			2
#define SPHERE_WATER			3
#define SPHERE_LIGHT			4
#define SPHERE_SHADOW			5

/* Defines for wound-type manifestation of spell damage */

#define MAGIC_DAMAGE_SLASH			0
#define MAGIC_DAMAGE_BLUNT			1
#define MAGIC_DAMAGE_PUNCTURE			2
#define MAGIC_DAMAGE_BURN			3
#define MAGIC_DAMAGE_FROST			4

/* Defines for save/defensive methods against spells */

#define SAVE_NONE			0
#define SAVE_EVADE			1
#define SAVE_MAGIC_RESIST		2
#define SAVE_AGI			3
#define SAVE_DEX			4
#define SAVE_INTEL			5
#define SAVE_WIL			6
#define SAVE_AUR			7
#define SAVE_CON			8
#define SAVE_STR			9

/* Defines for what a successful save will result in */

#define SAVE_FOR_NO_EFFECT		0
#define SAVE_FOR_HALF_EFFECT		1
#define SAVE_FOR_THIRD_EFFECT		2
#define SAVE_FOR_QUARTER_EFFECT		3

/* Target types used in spell definitions */

#define TARG_SELF			0
#define TARG_OBJ			1
#define TARG_OTHER			2
#define TARG_OTHER_HOSTILE		3
#define TARG_SELF_OTHER			4
#define TARG_SELF_OTHER_HOSTILE		5
#define TARG_GROUP			6
#define TARG_GROUP_HOSTILE		7
#define TARG_ROOM			8
#define TARG_ROOM_HOSTILE		9

/* Target types trackable via CAST */

#define TARGET_SELF             0
#define TARGET_OTHER            1
#define TARGET_OBJ              2
#define TARGET_ROOM             3
#define TARGET_OTHER_INVENTORY  4
#define TARGET_EXIT		5
#define TARGET_REMOTE_ROOM	6
#define TARGET_REMOTE_OTHER	7

/* See MAGIC_AFFECT_X for various specific effects */

/* End magic defines */

#define JOB_1					600
#define JOB_2					601
#define JOB_3					602

/* Affect for toggling listen/mute */
#define MUTE_EAVESDROP				620

/* Affect for counting the number of times a room is herbed */
//#define HERBED_COUNT				621

#define CRAFT_FIRST_OLD				650	/* Crafts can be assigned in the */
#define CRAFT_LAST_OLD				899	/*  range CRAFT_FIRST..CRAFT_LAST */

#define AFFECT_SHADOW			900	/* Affected n/pc shadows another n/pc */

#define AFFECT_GAIN_POINT		920
#define AFFECT_LOSE_POINT		930
#define AFFECT_UPGRADE_DELAY	940

#define AFFECT_GUARD_DIR		950	/* Affected n/pc guards a direction */
#define AFFECT_WATCH_DIR                960     /* Affected n/pc watches a direction */
#define AFFECT_LISTEN_DIR                970     /* Affected n/pc listens to a direction */

#define AFFECT_LOST_CON			980

#define AFFECT_HOLDING_BREATH		990	/* Swimming underwater! */
#define AFFECT_CHOKING 				999 /* In a no atmosphere zone! */

#define MAGIC_CRIM_BASE		    1000	/* Criminal tags reservd from 1000..1100 */
#define MAGIC_CRIM_RESERVED	    1100	/* 1000..1100 are reserved */

#define MAGIC_CLAN_MEMBER_BASE		1200	/* Clan tags reserved from 1200..1299 */
#define MAGIC_CLAN_LEADER_BASE		1300	/* Clan tags reserved from 1300..1399 */
#define MAGIC_CLAN_OMNI_BASE		1400	/* Clan tags reserved from 1400..1499 */

#define MAGIC_SKILL_GAIN_STOP		1500	/* Skill use spam 1500..1500+MAX_SKILLS */
#define MAGIC_SPELL_GAIN_STOP		1800
#define MAGIC_CRAFT_BRANCH_STOP		2000
#define MAGIC_CRAFT_DELAY		    2010

#define MAGIC_HIDDEN			2011
#define MAGIC_SNEAK			    2012
#define MAGIC_NOTIFY			2013
#define MAGIC_CLAN_NOTIFY		2014
#define MAGIC_TOLL			    2015
#define MAGIC_TOLL_PAID			2016
#define MAGIC_DRAGGER			2017
#define MAGIC_GUARD			    2018
#define MAGIC_WATCH1			2019
#define MAGIC_WATCH2			2020
#define MAGIC_WATCH3			2021
#define MAGIC_STAFF_SUMMON		2022
#define AFFECT_GROUP_RETREAT	2023
#define MAGIC_WATCH             2024
#define MAGIC_MONSTER           2025
#define AFFECT_INTERNAL			2026 // internal bleeding.
#define MAGIC_OMOTED			2027

#define AFFECT_TRAUMA_PINNED           2028    // 2 minute timer for our own pinned (someone shot at us)
#define AFFECT_TRAUMA_SUSTAINED        2029    // 2 minute timer for suffering sustained fire (someone shot near us)


#define AFFECT_FORAGED			2035
#define ATTACK_DIFF             2036

//#define AFFECT_COVER_NORTH    2100	//protect from north
//#define AFFECT_COVER_EAST		2101
//#define AFFECT_COVER_SOUTH	2102
//#define AFFECT_COVER_WEST		2103
//#define AFFECT_COVER_UP		2104
//#define AFFECT_COVER_DOWN		2105

#define AFFECT_COVER			2106

#define MAGIC_TURF_DELAY        2200
#define MAGIC_TURF_REPORT       2201

#define MAGIC_STARVE_ONE                2500
#define MAGIC_STARVE_TWO                2501
#define MAGIC_STARVE_THREE              2502
#define MAGIC_STARVE_FOUR               2503

#define MAGIC_CRIM_HOODED		2600	/* Hooded criminal observed in zone .. */

#define MAGIC_STARED			2700	/* Don't stare again until this expires */

#define MAGIC_ALERTED                   2800    // Don't alert until this expires.

#define MAGIC_SKILL_MOD_FIRST	3000	/* Reserve 200 for skill mod affects */
#define MAGIC_SKILL_MOD_LAST	3200

#define MAGIC_SENT			3250	/* Used with the emote system. */
#define MAGIC_HEARD			3260	/* Used with the eavesdrop system. */
#define MAGIC_WATCH_SENT    3261	/* Used to prevent that stupid disappearing emote bug. */

#define MAGIC_AFFECT_FIRST			3400

#define MAGIC_AFFECT_DMG			3400
#define MAGIC_AFFECT_HEAL			3401
#define MAGIC_AFFECT_PROJECTILE_IMMUNITY	3402
#define MAGIC_AFFECT_INFRAVISION		3403
#define MAGIC_AFFECT_CONCEALMENT		3404
#define MAGIC_AFFECT_INVISIBILITY		3405
#define MAGIC_AFFECT_SEE_INVISIBLE		3406
#define MAGIC_AFFECT_SENSE_LIFE			3407
#define MAGIC_AFFECT_TONGUES			3408
#define MAGIC_AFFECT_LEVITATE			3409
#define MAGIC_AFFECT_SLOW			3410
#define MAGIC_AFFECT_SPEED			3411
#define MAGIC_AFFECT_SLEEP			3412
#define MAGIC_AFFECT_PARALYSIS			3413
#define MAGIC_AFFECT_FEAR			3414
#define MAGIC_AFFECT_REGENERATION		3415
#define MAGIC_AFFECT_CURSE			3416
#define MAGIC_AFFECT_DIZZINESS			3417
#define MAGIC_AFFECT_FURY			3418
#define MAGIC_AFFECT_INVULNERABILITY		3419
#define MAGIC_AFFECT_ARMOR			3420
#define MAGIC_AFFECT_BLESS			3421

#define MAGIC_AFFECT_LAST			4999

#define MAGIC_ROOM_CALM			5000
#define MAGIC_ROOM_LIGHT		5001
#define MAGIC_ROOM_DARK			5002
#define MAGIC_ROOM_DEBUG		5003
#define MAGIC_ROOM_FLOOD		5004
#define MAGIC_WORLD_CLOUDS		5005	/* Blocks the sun */
#define MAGIC_WORLD_SOLAR_FLARE	5006	/* Creates an artificial sun */
#define MAGIC_WORLD_MOON		5006	/* Moonlight in all rooms */

#define MAGIC_BUY_ITEM			5400

#define MAGIC_ROOM_FIGHT_NOISE		5500

#define MAGIC_PETITION_MESSAGE		5600

#define MAGIC_STABLING_PAID		5700
#define MAGIC_STABLING_LAST		5999

#define MAGIC_FLAG_NOGAIN		6000

#define MAGIC_FLAG_FOCUS		6200

#define MAGIC_WARNED			6500

#define MAGIC_RAISED_HOOD		6600

#define MAGIC_SIT_TABLE			6700	/* PC acquires this affect when at a table */

#define MAGIC_FLAG_IGNORE		6800

#define POISON_LETHARGY			7001

#define CRAFT_FIRST			8000	/* Crafts can be assigned in the */
#define CRAFT_LAST			18000	/*  range CRAFT_FIRST..CRAFT_LAST */

#define ADMIN_RADIO_FIRST		19000	/* Range - Used for admins to monitor radio channels */
#define ADMIN_RADIO_LAST		19999	/* End of range - shade */

#define SOMA_FIRST                20000	/* SOMATIC EFFECTS TBA */
// Existing Somas that relate to combat damage, can't be easily migrated out yet.

// Temporary Combat damage
#define SOMA_NERVES_HEADACHE            SOMA_FIRST + 1
#define SOMA_BLUNT_MEDHEAD              SOMA_FIRST + 2
#define SOMA_BLUNT_SEVHEAD              SOMA_FIRST + 3
#define SOMA_BLUNT_R_SEVARM             SOMA_FIRST + 4
#define SOMA_BLUNT_L_SEVARM             SOMA_FIRST + 5
#define SOMA_BLUNT_SEVLEG               SOMA_FIRST + 6
#define SOMA_BLUNT_SEVBODY              SOMA_FIRST + 7
// Special things
#define SOMA_EVENT_COLD                 SOMA_FIRST + 8
#define SOMA_PUNISHMENT_WOUND           SOMA_FIRST + 9
// Lost body parts
#define SOMA_NO_RARM            	    SOMA_FIRST + 10
#define SOMA_NO_LARM 		            SOMA_FIRST + 11
#define SOMA_NO_RLEG                   	SOMA_FIRST + 12
#define SOMA_NO_LLEG             	    SOMA_FIRST + 13
#define SOMA_NO_HEAD             	    SOMA_FIRST + 14
#define SOMA_NO_TORSO            	    SOMA_FIRST + 15
#define SOMA_NO_RHAND            	    SOMA_FIRST + 16
#define SOMA_NO_LHAND            	    SOMA_FIRST + 17
// Genetic mutation
#define SOMA_GENETIC_MUTATION			SOMA_FIRST + 18
#define SOMA_SHAKES						SOMA_FIRST + 19
// Genetic mutation isogen
#define SOMA_ISOGEN						SOMA_FIRST + 20
// Genetic mutation cure
#define SOMA_GENETIC_MUTATION_CURE 		SOMA_FIRST + 21
// Combat stim effects
#define SOMA_COMBAT_STIM  				SOMA_FIRST + 22
#define SOMA_STIM_WITHDRAWAL			SOMA_FIRST + 23
// Increased vulnerability (increased growth rate of infection)
#define SOMA_INFECTION_VULNERABLE 		SOMA_FIRST + 24

#define SOMA_LAST                 		SOMA_FIRST + 25

#define MONITOR_THINK			21000
#define MONITOR_SAY				21001
#define MONITOR_FEEL			21002
#define MONITOR_EMOTE			21003
#define MONITOR_COMBAT			21004
#define MONITOR_MOVEMENT		21005
#define MONITOR_SKILLS			21006

#define TYPE_UNDEFINED		    -1

#define TYPE_SUFFERING		    200	/* KILLER CDR: Eliminate this line somehow */

#define SKILL_CEILING		100	// Top limit that skill checks are made against.

/* skills */

// weaponskills first
#define SKILL_DEFENSE		-3	/* Special cased; a pseudo skill */
#define SKILL_OFFENSE		-2	/* Special cased; a pseudo skill */
#define SKILL_BRAWLING		    1
#define SKILL_SMALL_BLADE       2
#define SKILL_LONG_BLADE        3
#define SKILL_POLEARM   	    4
#define SKILL_BLUDGEON      	5
#define SKILL_DODGE		        6
#define SKILL_DEFLECT         	7
#define SKILL_SOLE_WIELD        8
#define SKILL_DUAL_WIELD        9
#define SKILL_AIM		        10
#define SKILL_HANDGUN	    	11
#define SKILL_RIFLE		        12
#define SKILL_SMG       	    13
#define SKILL_GUNNERY		    14
#define SKILL_ARCHERY	    15      // One day this will be awesome.

#define LAST_WEAPON_SKILL       14      // The last real weaponskill we have.

// ability skills second
#define SKILL_SNEAK         	16
#define SKILL_HIDE          	17
#define SKILL_STEAL         	18
#define SKILL_PICK          	19
#define SKILL_HAGGLE       		20
#define SKILL_HANDLE			21
#define SKILL_HUNTING			22
#define SKILL_FIRSTAID			23
#define SKILL_MEDICINE			24
#define SKILL_FORAGE        	25
#define SKILL_EAVESDROP         26
#define SKILL_BUTCHERY			27

// craft skills third
#define SKILL_CHEMISTRY         28
#define SKILL_MECHANICS         29
#define SKILL_GUNSMITH          30
#define SKILL_COMPUTEROLOGY     31
#define SKILL_ELECTRONICS       32
#define SKILL_BIOLOGY           33
#define SKILL_WEAPONCRAFT   	34
#define SKILL_ARMORCRAFT		35
#define SKILL_HANDICRAFT		36
#define SKILL_ARTISTRY			37

// odds and ends
#define SKILL_EDUCATION         38
#define SKILL_VOODOO  	        39
#define SKILL_COMMON            40


#define LAST_SKILL		SKILL_COMMON

#define PSIONIC_TALENTS		1

/* How much light is in the land ? */

#define SUN_DARK		0
#define SUN_RISE		1
#define SUN_LIGHT		2
#define SUN_SET			3
#define MOON_RISE		4
#define MOON_SET		5

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY   	1
#define SKY_RAINING  	2
#define SKY_LIGHTNING	3
#define SKY_STORMY		4
#define SKY_FOGGY 		5

#define MAX_OBJ_SAVE	15

/* Delay types */

#define DEL_PICK		1
#define DEL_SEARCH		2
#define DEL_RITUAL		3
#define DEL_BACKSTAB	4
#define	DEL_SPELL		5
#define DEL_WORSHIP		6
#define DEL_FORAGE_SEEK	7
#define DEL_JOIN_FAITH	8
#define DEL_APP_APPROVE	9
#define DEL_CAST		10
/* Fyren start */
/* ??#define DEL_SKIN		11 */
#define DEL_SKIN_1		4002
#define DEL_SKIN_2		4003
#define DEL_SKIN_3		4004
/* Fyren end */
#define DEL_COUNT_COIN		12
#define DEL_IDENTIFY		13
#define DEL_GATHER		14
#define DEL_COMBINE		15
#define DEL_WHAP		16
#define DEL_GET_ALL		17
#define DEL_AWAKEN		18
#define DEL_EMPATHIC_HEAL	19
#define DEL_MENTAL_BOLT		20
#define DEL_ALERT		21
#define DEL_INVITE		22
#define DEL_CAMP1		23
#define DEL_CAMP2		24
#define DEL_CAMP3		25
#define DEL_CAMP4		26
#define DEL_COVER		27
#define DEL_TAKE		28
#define DEL_PUTCHAR		29
#define DEL_STARE		30
#define DEL_HIDE		31
#define DEL_SCAN		32
#define DEL_QUICK_SCAN		33
#define DEL_HIDE_OBJ		34
#define DEL_PICK_OBJ		35
#define DEL_COUNT_FARTHING	36
#define DEL_RUMMAGE		37
#define DEL_QUAFF		38
#define DEL_BIND_WOUNDS		39
#define DEL_TREAT_WOUND		40
#define DEL_LOAD_WEAPON		41
#define DEL_OOC			42
#define DEL_WEAVESIGHT		43
#define DEL_TRACK		44
#define DEL_FORAGE	        45
#define DEL_PITCH		46
#define DEL_PURCHASE_ITEM	47
#define DEL_WATER_REMOVE	48
#define DEL_ORDER_ITEM		49
#define DEL_QUICK_SCAN_DIAG	50
#define DEL_PLACE_AUCTION	51
#define DEL_PLACE_BID		52
#define DEL_PLACE_BUYOUT	53
#define DEL_CANCEL_AUCTION	54
#define DEL_POISON_ITEM         55
#define DEL_SCOUT               56
#define DEL_LOG1                57
#define DEL_LOG2                58
#define DEL_LOG3                59
#define DEL_FORM                60
#define DEL_STRING_BOW          61
#define DEL_DESTRING_BOW        62
#define DEL_RESTRING_BOW        63
#define DEL_BUTC_1		64
#define DEL_BUTC_2		65
#define DEL_BUTC_3		66
#define DEL_MEND1               67
#define DEL_MEND2               68
#define DEL_TRAPPED             69
#define DEL_TRAP_ASS_1          70
#define DEL_TRAP_ASS_2          71
#define DEL_TRAP_DIS            72
#define DEL_TRAP_SEARCH         73
#define DEL_TRAP_DEFUSE         74
#define DEL_TRAP_EXAMINE        75
#define DEL_CHEM_COMBINE_MIX 	   76
#define DEL_CHEM_CONCENTRATE_MIX   77
#define DEL_CHEM_INJECT         78
#define DEL_LOAD_CLIP          79
#define DEL_UNLOAD_CLIP        80
#define DEL_LOAD_FIREARM           81
#define DEL_UNLOAD_FIREARM         82
#define DEL_EXTRACT_1		83
#define DEL_EXTRACT_2		84
#define DEL_EXTRACT_3		85
#define DEL_POINTBLANK      86
#define DEL_LONG_BIND       87
#define DEL_CLEAN           88
#define DEL_TRADE			89
#define DEL_RENT_COMMENCE	90
#define DEL_RENT_TOPUP		91
#define DEL_RENT_REPLACE	92
#define DEL_RENT_REPLICATE	93
#define DEL_TRADE_ASSIST	94
#define DEL_TURFACTION      95
#define DEL_ORDER_PLACE     96
#define DEL_ORDER_FULFILL   97
#define DEL_HOME_CAMP1		98
#define DEL_HOME_CAMP2		99
#define DEL_HOME_CAMP3		100

/* Zone flags */

#define Z_FROZEN    1
#define Z_LOCKED    2

/* Projectile-related defines */

#define NUM_TABLES              2 // one for melee, one for projectiles
#define MELEE_TABLE 			0 // We use this table for melee
#define RANGED_TABLE			1 // and we use this one for ranged.

#define HITLOC_NONE             -1
#define HITLOC_BODY				0
#define HITLOC_HILEGS			1
#define HITLOC_HIARMS			2
#define HITLOC_HEAD				3
#define HITLOC_NECK				4
#define HITLOC_FEET				5
#define HITLOC_HANDS			6
#define HITLOC_EYES				7
#define HITLOC_LOLEGS			8
#define HITLOC_LOARMS			9
#define MAX_HITLOC              10

struct body_info
{
    char part[11];
    int damage_mult;
    int damage_div;
    int percent;
    int wear_loc1;
    int wear_loc2;
    int wear_loc3;
    int wear_loc4;
    int wear_loc5;
};

// All the different armour effects things can have.

#define ARMOR_NONE          0
#define ARMOR_ER_PIERCE 	1 << 0   // 1
#define ARMOR_VR_PIERCE 	1 << 1
#define ARMOR_R_PIERCE	 	1 << 2
#define ARMOR_V_PIERCE	 	1 << 3
#define ARMOR_VV_PIERCE 	1 << 4
#define ARMOR_EV_PIERCE	 	1 << 5   // 6

#define ARMOR_ER_SLASH 		1 << 6   // 7
#define ARMOR_VR_SLASH 		1 << 7
#define ARMOR_R_SLASH	 	1 << 8
#define ARMOR_V_SLASH	 	1 << 9
#define ARMOR_VV_SLASH 		1 << 10
#define ARMOR_EV_SLASH	 	1 << 11  // 12

#define ARMOR_ER_CRUSH 		1 << 12  // 13
#define ARMOR_VR_CRUSH 		1 << 13
#define ARMOR_R_CRUSH	 	1 << 14
#define ARMOR_V_CRUSH	 	1 << 15
#define ARMOR_VV_CRUSH 		1 << 16
#define ARMOR_EV_CRUSH	 	1 << 17  // 18

#define ARMOR_ER_GUN 		1 << 12  // 19
#define ARMOR_VR_GUN 		1 << 13
#define ARMOR_R_GUN		 	1 << 14
#define ARMOR_V_GUN		 	1 << 15
#define ARMOR_VV_GUN 		1 << 16
#define ARMOR_EV_GUN	 	1 << 17  // 24

#define RESULT_CS       1
#define RESULT_MS       2
#define RESULT_MF       3
#define RESULT_CF       4

#define GLANCING_HIT    1
#define HIT             2
#define CRITICAL_HIT    3
#define MISS            4
#define CRITICAL_MISS   5
#define COVER_HIT       6
#define SHIELD_BLOCK    7
#define PUNCTURE_HIT    8
#define SHATTER_HIT     9
#define CAUGHT          10
#define JAM             11

/* Magic-related defines */

#define MAGNITUDE_SUBTLE        1
#define MAGNITUDE_GENTLE        2
#define MAGNITUDE_SOFT          3
#define MAGNITUDE_STOLID        4
#define MAGNITUDE_STAUNCH       5
#define MAGNITUDE_VIGOROUS      6
#define MAGNITUDE_VIBRANT       7
#define MAGNITUDE_POTENT        8
#define MAGNITUDE_POWERFUL      9
#define MAGNITUDE_DIRE          10

#define TECHNIQUE_CREATION              1
#define TECHNIQUE_DESTRUCTION           2
#define TECHNIQUE_TRANSFORMATION        3
#define TECHNIQUE_PERCEPTION            4
#define TECHNIQUE_CONTROL               5

#define FORM_ANIMAL             1
#define FORM_PLANT              2
#define FORM_IMAGE              3
#define FORM_LIGHT              4
#define FORM_DARKNESS           5
#define FORM_POWER              6
#define FORM_MIND               7
#define FORM_SPIRIT             8
#define FORM_AIR                9
#define FORM_EARTH              10
#define FORM_FIRE               11
#define FORM_WATER              12

struct forage_data
{
    long virt;
    int sector;
    struct forage_data *next;
};

struct extra_descr_data
{
    char *keyword;
    char *description;
    struct extra_descr_data *next;
};

struct writing_data
{
    char *message;
    char *author;
    char *date;
    char *ink;
    int language;
    int script;
    bool torn;
    int skill;
    WRITING_DATA *next_page;
};

struct written_descr_data	/* for descriptions written in languages */
{
    rpi_byte language;
    char *description;
};

struct obj_flag_data
{
    rpi_byte type_flag;
    bitflag wear_flags;
    int extra_flags;
    int extra_flags2;
    int weight;
    int set_cost; // set by an npc-shopkeeper or imm
    long bitvector;
};

struct armor_data
{
    int armor_value;
    int armor_type;
    int v2;
    int v3;
    int v4;

};

#define  MAX_ELECTRIC_TYPES 12

struct electronic_data
{
    int elec_bits;
    int status;
    int usage;
    int v3;
    int v4;
    int v5;
};

struct battery_data
{
    int power;
    int max_power;
    int type;
    int recharge;
    int v4;
    int v5;
};

struct radio_data
{
    int elec_bits;
    int status;
    int usage;
    int channel;
    int encrypt;
    int range;
};

struct e_light_data
{
	int elec_bits;
    int status;
    int usage;
    int v3;
    int v4;
    int v5;
};

struct weapon_data
{
    int handedness;
    int dice;
    int sides;
    int use_skill;
    int hit_type;
    int delay;
    int basedamage;
    int attackclass;
    int defenseclass;
    int range;
};

struct grenade_data
{
    int timer;      // How long the timer is.
    int status;     // 0 = safe, 1 = live, 2 = dud
    int skill;      // how difficulty it is to use
    int bits;       // what affects this grenade has.
    int magnitude; // how wide our blast radius is
    int power;      // how powerful our blast is

};

struct firearm_data
{
    int handedness; // How many hands do we need free to use this weapon?
    int bits;       // What weapon bits does it have?
    int caliber;    // What type of ammo does it take?
    int use_skill;  // What skill does it use?
    int setting;    // What setting is it, e.g. locked, semi, burst, auto, etc.
    int recoil;     // What is our recoil?
};

struct clip_data
{
    int amount;  // how many bullets are in this clip
    int max;     // what is the maximum amount of bullets this clip can take?
    int caliber; // what caliber bullets does this take?
    int type;    // what type of clip is this?
    int v5;
    int size;    // pistol, smg, or rifle.
};

struct bullet_data
{
    int damage;  // how much damage to we add?
    int sides;   // how many sides do these dice have?
    int caliber; // what caliber are these bullets?
    int pierce;  // the effect it has on armour
    int type;    // what type of bullet is it, e.g. copper-jacket, dum-dum, explosive, etc. etc. etc.
    int size;    // pistol, smg, or rifle.
};

struct smoke_data
{
    int v0;
    int hours;
    int morphto;
    int on;
    int scent_power;
    int scent_ratio;
};

struct light_data
{
    int capacity;
    int hours;
    int liquid;
    int on;
    int v4;
    int v5;
};

struct drink_con_data
{
    int capacity;
    int volume;
    int liquid;
    int spell_1;
    int spell_2;
    int spell_3;
};

struct fountain_data
{
    int capacity;
    int volume;
    int liquid;
    int spell_1;
    int spell_2;
    int spell_3;
};

struct container_data
{
    int capacity;
    int flags;
    int key;
    int pick_penalty;
    int v4;
    int table_max_sitting;
};

struct clan_container_data
{
    int capacity;
    int flags;
    int key;
    int pick_penalty;
    int v4;
    int v5;
};
struct cloak_data
{
    int v0;
    int v1;
    int v2;
    int v3;
    int v4;
};

struct ticket_data
{
    int ticket_num;
    int keeper_vnum;
    int stable_date;
    int v3;
    int v4;
    int v5;
};

struct food_data
{
    int food_value;
    int bites;
    int junk;
    int spell2;
    int spell3;
    int spell4;
};

struct perfume_data
{
    int uses;
    int type;
    int duration;
    int power;
    int v4;
    int v5;
};

struct fluid_data
{
    int alcohol;
    int water;
    int food;
    int v3;
    int v4;
    int v5;
};

struct lodged_object_info
{
    char *location;
    int vnum;
    int colored;
    char *short_description;
    char *var_color;
    char *var_color2;
    char *var_color3;
    AFFECTED_TYPE *bleeding;
    LODGED_OBJECT_INFO *next;
};

struct lodged_missile_info
{
    int vnum;
    LODGED_MISSILE_INFO *next;
};


#define	BLEED_MELEE	0
#define BLEED_EXTERNAL	1
#define	BLEED_INTERNAL	2

struct wound_data
{
    char *location;
    char *type;
    char *name;
    char *severity;
    int damage;
    int bleeding;
    int poison;
    int infection;
    int healerskill;
    int bindskill;
    int lasthealed;
    int lastbled;
    int lastbound;
    int fracture;
    int gunshot;
    WOUND_DATA *next;
};

struct repair_data
{
    unsigned short int nCraftMinutes;
    unsigned short int nUses;
    unsigned char nMendBonus;
    unsigned char nMinSkillNeeded;
    unsigned char nRequiresClan;
    unsigned char bAnyoneMayUse;
    unsigned int flgDamageTypes;
    unsigned int flgMaterialSkills;
    unsigned char arrItemType[4];
    unsigned int nUnused;
};

struct default_obj_data
{
    int value[6];
};

union obj_info
{
    struct weapon_data weapon;
    struct drink_con_data drinks;
    struct container_data container;
    struct default_obj_data od;
    struct light_data light;
	struct e_light_data e_light;
    struct smoke_data smoke;
    struct armor_data armor;
    struct cloak_data cloak;
    struct ticket_data ticket;
    struct perfume_data perfume;
    struct food_data food;
    struct fluid_data fluid;
    struct fountain_data fountain;
    struct repair_data repair;
    struct clan_container_data locker;
    struct radio_data radio;
    struct electronic_data elecs;
    struct battery_data battery;
    struct firearm_data firearm;
    struct grenade_data grenade;
    struct clip_data clip;
    struct bullet_data bullet;
};

struct obj_clan_data
{
    char *name;
    char *rank;
    OBJ_CLAN_DATA *next;
};


enum obj_cue
{
    ocue_none,
    ocue_notes,

    // Position cues
    ocue_on_grab, // Activate when an object is picked up -- after it's picked up
    ocue_on_drop, // Actives when an object is dropped -- after it's dropped
    ocue_on_give, // Activates -before- a player gives an object.
    ocue_on_receive, // Actives -after- a player receives an object. -- does nothing, handled elsewhere

    // Combat cues
    ocue_on_strike, // Called whenever an object hits someone in melee combat
    ocue_on_mstrike, // Whenever an object hits something via missile strike: on throw for thrown weapons, on shoot for archery etc.
    ocue_on_block, // Called whenever an object is used to block something in combat
    ocue_on_blocked, // Called whenever an object is blocked by a shield in missile or throwing
    ocue_on_hit, // Called whenever an object hits a person in combat, and isn't blocked or otherwise defferred.


    // Time cues
    ocue_on_hour, // Activates on the hour
    ocue_on_enter, // Activates whenever a mobile/PC enters a room
    ocue_on_decay, // Activates before decay kicks in
    ocue_on_morph, // Activates before morph occurs

    // Odd cues

    ocue_on_produced, // Activates once an object is produced by a craft
    ocue_on_craft, // Activates if an object is used within a craft
    ocue_on_load,  // This ocue is called when an object is loaded in to existence. Fun times!

	ocue_on_five,
	ocue_on_one,

	ocue_on_reboot,

    ocue_blank
};

/* ======================== Structure for object ========================= */
struct obj_data
{
    int deleted;
    int nVirtual;
    int zone;
    int in_room;
    int instances;		/* Proto-only field; keeps track of loaded instances */
    int order_time;		/* Proto-only field to track time required to order item */
    struct obj_flag_data obj_flags;
    union obj_info o;
    AFFECTED_TYPE *xaffected;
    char *name;
    char *description;
    char *short_description;
    char *full_description;
    char *omote_str;
    EXTRA_DESCR_DATA *ex_description;
    WRITTEN_DESCR_DATA *wdesc;
    CHAR_DATA *carried_by;
    CHAR_DATA *equiped_by;
    OBJ_DATA *in_obj;
    OBJ_DATA *contains;
    OBJ_DATA *next_content;
    OBJ_DATA *next;
    OBJ_DATA *hnext;
    OBJ_DATA *lnext;
    int clock;
    int morphTime;
    int morphto;
    int location;
    int contained_wt;
    int activation;		/* Affect applied when picked up */
    int quality;
    int econ_flags;		/* Flag means used enhanced prices */
    int size;
    int count;			/* How many this obj represents */
    int obj_timer;
    float farthings;		/* Partial value in farthings    */
    float silver;			/* Partial value in silver coins */
    WOUND_DATA *wounds;		/* For corpses */
    int item_wear;		/* Percentile; 100%, brand new */
    WRITING_DATA *writing;
    char *ink_color;
    unsigned int open;		// 0 closed, 1+ open/page number
    OBJ_DATA *attached;     // Attachment to a weapon, or a sling.
    LODGED_OBJECT_INFO *lodged;
    char *desc_keys;
    char *var_color[10];
    char *var_cat[10];
    char *book_title;
    int title_skill;
    int title_language;
    int title_script;
    int material;
    int tmp_flags;
    bool writing_loaded;
    int coldload_id;
    OBJECT_DAMAGE *damage;
    char *indoor_desc;
    int sold_at;
    int sold_by;
    OBJ_CLAN_DATA *clan_data;
    SCENT_DATA *scent;
    int room_pos;
    char *creater; // keeps track of what created an object
    char *editer; // likewise, who last made changes to it.
    TRAP_DATA *trap;

    char *attire; // how we wear it
    
    char *dec_desc; // what our actual decoration is
    char *dec_style; // what the style of our decoration is
    int dec_quality; // what quality our decoration is at
    int dec_condition; // what condition our decoration is
    int dec_size; // how big our decoration is
    int dec_short; // do we have our decoration in our short-desc?

    int enviro_conds[COND_TYPE_MAX];

    bool concealed; // Is this item worn and concealed?
    
    // Obj Triggers
    std::multimap<obj_cue,std::string> *ocues;

    // Japh's corruption fix.
    void deep_copy (OBJ_DATA *copy_from);
    void partial_deep_copy (OBJ_DATA *proto);

};


struct room_direction_data
{
    char *general_description;
    char *keyword;
    int exit_info;
    int key;
    int pick_penalty;
    int to_room;
};

struct prog_vars
{
    char *name;
    int value;
    struct prog_vars *next;
};

struct room_prog
{
    char *command;		/* List of commands to trigger this program */
    char *keys;			/* List of valid arguments, NULL always executes */
    char *prog;			/* The program itself */
    struct prog_vars *vars;
    int type;
    struct room_prog *next;	/* next program for this room */
};

struct mobile_ai
{
	int attack_thresh;	// ratio under which we will attack
	int flee_thresh;	// ratio over which we will flee
};

struct variable_data
{
    int unique;
    int id;
    char *shorts;
    char *category;
    int category_id;
    char *full;
    int cost_mod;
    int weight_mod;
    int quality_mod;
    int item_type;
    int skill_name;
    int skill_mod;
    int oval0;
    int oval1;
    int oval2;
    int oval3;
    int oval4;
    int oval5;
    int random;
    int category_seeable;
	char *flavour;
};

struct mvariable_data
{
    int unique;
    int id;
    char *shorts;
    char *category;
    int category_id;
    char *full;
};


struct mob_variable_data
{
    char *category;
    char *sdesc;
    char *fdesc;
    int sexes;		// What sexes cannot have this variable
    int must_bits;	// What bits a mobile -must- have to have this variable
    int not_bits;		// What bits a mobile -can't- have if they want this variable
    int age_more_than;	// What age a mobile must be greater than to have this variable
    int age_less_than;	// What age a mobile must be less than to have this variable.
};

struct secret
{
    int diff;			/* difficulty (search skill abil) */
    char *stext;
};

#define MAX_DELIVERIES			200
#define MAX_TRADES_IN			200

struct negotiation_data
{
    int ch_coldload_id;
    int obj_vnum;
    int time_when_forgotten;
    int price_delta;
    int transactions;
    int true_if_buying;
    struct negotiation_data *next;
};

struct shop_data
{
    float markup;			/* Objects sold are multiplied by this  */
    float discount;		/* Objects bought are muliplied by this */
    int shop_vnum;		/* Rvnum of shop                                                */
    int store_vnum;		/* Rvnum of inventory                                   */
    char *no_such_item1;		/* Message if keeper hasn't got an item */
    char *no_such_item2;		/* Message if player hasn't got an item */
    char *missing_cash1;		/* Message if keeper hasn't got cash    */
    char *missing_cash2;		/* Message if player hasn't got cash    */
    char *do_not_buy;		/* If keeper dosn't buy such things.    */
    char *message_buy;		/* Message when player buys item                */
    char *message_sell;		/* Message when player sells item               */
    int delivery[MAX_DELIVERIES];	/* Merchant replaces these      */
    int trades_in[MAX_TRADES_IN];	/* item_type that can buy       */
    int econ_flags1;		/* Bits which enhance price             */
    int econ_flags2;
    int econ_flags3;
    int econ_flags4;
    int econ_flags5;
    int econ_flags6;
    int econ_flags7;
    float econ_markup1;		/* Sell markup for flagged items        */
    float econ_markup2;
    float econ_markup3;
    float econ_markup4;
    float econ_markup5;
    float econ_markup6;
    float econ_markup7;
    float econ_discount1;		/* Buy markup for flagged items         */
    float econ_discount2;
    float econ_discount3;
    float econ_discount4;
    float econ_discount5;
    float econ_discount6;
    float econ_discount7;
    int buy_flags;		/* Any econ flags set here are traded */
    int nobuy_flags;		/* Any econ flags set here aren't traded */
    int materials;		/* Item materials the shop trades in - MATERIALS in structs.h */
    NEGOTIATION_DATA *negotiations;	/* Haggling information                         */
    int opening_hour;
    int closing_hour;
    int exit;
};

// MATERIALS
#define MAT_NONE	0
#define MAT_ORGANIC	1
#define MAT_TEXTILE	2
#define MAT_METAL	3
#define MAT_CERAMIC	4
#define MAT_GLASS	5
#define MAT_ELECTRONIC	6
#define MAT_PLASTIC	7
#define MAT_PAPER	8
#define MAT_LIQUID	9
#define MAT_OTHER	10

struct room_extra_data
{
    char *alas[6];
    char *weather_desc[WR_DESCRIPTIONS];
};

#define PC_TRACK	( 1 << 0 )
#define BLOODY_TRACK	( 1 << 1 )
#define FLEE_TRACK	( 1 << 2 )

struct track_data
{
    int race;
    int from_dir;
    int to_dir;
    int hours_passed;
    int speed;
    int flags;
    int wildlife;
    char *name;
	int clantrack;
	int clanrank;
    TRACK_DATA *next;
};

struct reset_time_data
{
    int month;
    int day;
    int minute;
    int hour;
    int second;
    int flags;
};

struct time_info_data
{
    int hour;
    int day;
    int month;
    int year;
    int season;
    int minute;
    int holiday;
    int phaseEarth;         // What phase is the earth in?
    int phaseSun;           // What phase is the sun in?
};

struct time_data
{
    time_t birth;			/* This represents the characters age                */
    time_t logon;			/* Time of the last logon (used to calculate played) */
    long played;			/* This is the total accumulated time played in secs */
};

struct memory_data
{
    char *name;
    struct memory_data *next;
};

struct char_ability_data
{
    int str;
    int intel;
    int wil;
    int dex;
    int con;
    int aur;
    int agi;
};

struct newbie_hint
{
    char *hint;
    NEWBIE_HINT *next;
};

struct group_data
{
    CHAR_DATA *leader;	// Who the leader of this thing is.
    int initial;		// How many members we had to start with.
    int aggro;		// Maximum group size we'll attack as a percentage of initial.
    int wimpy;		// What percentage we will go fallback on.
    int fallback;		// Where we are falling back to.
};

struct role_data
{
    char *summary;
    char *body;
    char *poster;
    char *date;
    int cost;
    int timestamp;
    int id;
    ROLE_DATA *next;
};

struct known_spell_data
{
    char *name;
    int rating;
    struct known_spell_data *next;
};

struct affect_spell_type
{
    int duration;			/* For how long its effects will last           1*/
    int modifier;			/* This is added to apropriate ability          2*/
    int location;			/* Tells which ability to change(APPLY_XXX)     3*/
    int bitvector;		/* Tells which bits to set (AFF_XXX)				4*/
    int t;			/* Extra information									5*/
    int sn;			/* Acquired by spell number                             6*/
    int technique;
    int form;
    int magnitude;
    int discipline;
    int mana_cost;
};


struct affect_generic_type
{
    int uu1;
    int uu2;
    int uu3;
    int uu4;
    int uu5;
    int uu6;
};

struct enchantment_data
{
    char *name;
    int original_hours;
    int current_hours;
    int power_source;
    int caster_skill;
    ENCHANTMENT_DATA *next;
};

struct affect_job_type
{
    int days;
    int pay_date;
    int cash;
    int count;
    int object_vnum;
    int employer;
};

struct affect_table_type
{
    int uu1;
    int uu2;
    int uu3;
    int uu4;
    OBJ_DATA *obj;
    int uu6;
};

struct affect_cover_type
{
    int duration;
    int value;
    int use;
    int direction;
    OBJ_DATA *obj;
    int temp;
};

struct affect_smell_type
{
    int duration;
    int aroma_strength;
    int uu3;
    int uu4;
    int uu5;
    int uu6;
};

struct affect_paralyze
{
    int duration;
    int minutes_until_paralyzed;
    int uu3;
    int uu4;
    int uu5;
    int sn;
};

struct affect_shadow_type
{
    CHAR_DATA *shadow;		/* target begin shadowed              */
    int edge;			/* -1, center.  0-5 edge by direction */
};

struct affect_hidden_type
{
    int duration;
    int hidden_value;
    int coldload_id;
    int clan;
    int uu5;
    int uu6;
};

struct affect_internal_bleeding
{
    int duration;
    int rate;
    int lodged;
    int uu4;
    int uu5;
    int uu6;
};

struct affect_monster_type
{
    int duration;
    int uu2;
    int disease;
    int uu4;
    int uu5;
    int uu6;
};

struct affect_toll_type
{
    int duration;			/* Generally forever. */
    int dir;
    int room_num;			/* for verification */
    int charge;			/* Cost for others to pass toll crossing */
    int uu5;
    int uu6;
};

struct affect_room_type
{
    int duration;
    int uu2;
    int uu3;
    int uu4;
    int uu5;
    int uu6;
};

struct affect_craft_type
{
    int timer;
    int skill_check;
    PHASE_DATA *phase;
    SUBCRAFT_HEAD_DATA *subcraft;
    CHAR_DATA *target_ch;
    OBJ_DATA *target_obj;
    char *argument;
};

struct affect_listen_type
{
    /* For muting and later, directed listening */
    int duration;			/* Always on if it exists */
    int on;			/* nonzero is on */
};

struct affect_herbed_type
{
    /* For counting the number of herbs found in rooms */
    int duration;
    int timesHerbed;
};

/* Agent FX - let the suffering begin (soon)
enum AGENT_FORM {
	AGENT_NONE=0,
	AGENT_FUME,
	AGENT_POWDER,
	AGENT_SOLID,
	AGENT_SALVE,
	AGENT_LIQUID,
	AGENT_SPRAY
};
enum AGENT_METHOD {
	AGENT_NONE=0,
	AGENT_INJECTED,
	AGENT_INHALED,
	AGENT_INGESTED,
	AGENT_INWOUND,
	AGENT_TOUCHED
};
****************************************/

/*
  SOMATIC RESPONSES by Sighentist
  HACKED by Kith: this is the place-holder for scents now, and a
  few combat-necessary somatics until we get the new system fully
  in place.

  Basically the idea here, is that the magnitude of the effect is
  enveloped over time. So after a delay, the effect begins to grow
  in strength until it reaches a peak. After cresting the peak the
  strength weakens to a fraction of the peak strength. It remains
  fixed there until the effect begins to wear off completely.

  Notes:
  If latency == duration the affected is a carrier.

*/

struct affect_somatic_type
{
    /* SOMA EFFECTS TBA */
    int duration;			/* rl hours */
    int latency;	/* rl hours of delay  */
    int minute;	/* timer  */

    int max_power;	/* type-dependant value */
    int lvl_power;	/* fraction of max_power */
    int atm_power;	/* the current power */
    int attack;	/* minutes to amp to max_power */

    int decay;	/* minutes to decay to lvl_power */
    int sustain;	/* minutes to lvl_power drops */
    int release;	/* minutes to end of effect */
};

struct scent_data
{
    /* SOMA EFFECTS TBA */
    int scent_ref;  /* the ref-id to our mysql list */
    int permanent;  /* whether this scent is permanent or not */

    int atm_power;	/* the current power, or how long our scent is going to last */
    int pot_power;	/* the potency power, or how much scent we can impart as a maximum */
    int rat_power;	/* the ratio power, or how much stronger our scent smells */

    SCENT_DATA *next;    // The next scent in the list.
};

union affected_union
{
    struct affect_spell_type spell;
    struct affect_job_type job;
    struct affect_table_type table;
    struct affect_shadow_type shadow;
    struct affect_paralyze paralyze;
    struct affect_smell_type smell;
    struct affect_hidden_type hidden;
    struct affect_room_type room;
    struct affect_toll_type toll;
    struct affect_craft_type *craft;
    struct affect_listen_type listening;
    struct affect_herbed_type herbed;
    struct affect_somatic_type soma;
    struct affect_monster_type monster;
    struct affect_cover_type cover;
    struct affect_internal_bleeding bleeding;
    struct affect_generic_type affect;
};

struct affected_type
{
    int type;
    union affected_union a;
    AFFECTED_TYPE *next;
};

#ifdef HAHA
struct affected_type
{
    int type;			/* The type of spell that caused this           */
    int duration;			/* For how long its effects will last           */
    int modifier;			/* This is added to apropriate ability          */
    int location;			/* Tells which ability to change(APPLY_XXX)     */
    int bitvector;		/* Tells which bits to set (AFF_XXX)            */
    int t;			/* Extra information                        */
    int sn;			/* Acquired by spell number                                     */
    AFFECTED_TYPE *next;
};
#endif

struct defined_scent
{
public:
    int id;
    char *name;

    defined_scent()
    {
        id = 0;
        name = '\0';
    }
};

#define TURF_CLANS 5

enum turfType
{
	turfTypeOrdinary = 0,
	turfTypeHeart = 1,
	turfTypeBase = 2,
	turfTypeFragile = 3
};

class Block
{
    public:
	int turfRoom;
	int turfHood;

	int blockLoyalty;

	int blockType;
	int blockImprove;

	int scores[TURF_CLANS][2];
	int actioned[TURF_CLANS];
	int actionScore[TURF_CLANS];

	int violence;

	Block (void)
	{ 
		initialize (); 
	}

	Block (int nVirtual);

	~Block (void)
	{

	}

	void initialize(void)
	{
		turfRoom = 0;
		turfHood = 0;

		blockLoyalty = -1;

		blockType = 0;
		blockImprove = 0;

		for (int i = 0; i < TURF_CLANS; i++)
		{
			actioned[i] = 0;
			actionScore[i] = 0;
			for (int j = 0; j < 2; j++)
			{
				scores[i][j] = 0;
			}
		}

		violence = 0;
	}

	std::string clanName(int systemId, int clan);
	std::string shortOwn(int systemId);
	std::string status(int systemId, int hoodLoyalty, int heartLoyalty, char *hoodName);
	void updateBeat(int hoodLoyalty, int heartLoyalty, bool isTurn);
	bool contestBlock();
	void decayBlock(int hoodLoyalty, int heartLoyalty, bool isTurn);
	void refreshBlock();
	void saveBlock();
	int clanPosition(char *clanName, int systemId);
	void sufferViolence(int clanId);
};

class Neighbourhood
{
    public:
	    int hoodId;
	    int hoodLoyalty;
	    int systemId;
	    int dumpRoom;
	    std::string name;
	    int npc_coldload_first;
	    int npc_coldload_second;

	Neighbourhood (void)
	{
		initialize();
	}

	std::map<int, Block> hoodBlocks;

	void initialize(void)
	{
		hoodId = 0;
		hoodLoyalty = -1;
		systemId = 0;
		hoodBlocks.clear();
		name.erase();
		npc_coldload_first = 0;
		npc_coldload_second = 0;
	}

	void updateBeats(bool isTurn);
	void loadEnforcers(CHAR_DATA *ch, CHAR_DATA *target);
	void reportIncident(CHAR_DATA *ch, CHAR_DATA *target, int crime);
	bool contestHood(void);
	void rewardHood(void);
	void updateHood(void);
	void saveHood(void);	
	int determineHeart(void);
};

class TurfSystem
{
	public:
		int systemId;
		std::string clanName[TURF_CLANS];
		int boardVnum;
		int dumpVnum1;
		int dumpVnum2;
		int enforceMob;
		int shopkeepMob;
		int soldierMob;
		int shopkeepFee;
		int soldierFee;

	TurfSystem (void)
	{
		initalize ();
	}

	void initalize(void)
	{
		systemId = 0;
		for (int i = 0; i < TURF_CLANS; i++)
		{
			clanName[i].erase();
		}
		boardVnum = 0;
		dumpVnum1 = 0;
		dumpVnum2 = 0;
		enforceMob = 0;
		shopkeepMob = 0;
		soldierMob = 0;
		shopkeepFee = 50;
		soldierFee = 100;
	}

	void set_clanName (int i, const char *name)
	{
		if (name && i >= 0 && i < TURF_CLANS)
		{
			clanName[i].assign (name);
		}
	}

	int clanPosition (const char *name)
	{
		for (int i = 0; i < TURF_CLANS; i++)
		{
			if (clanName[i] == name)
				return i;
		}
		return -1;
	}

	void saveSystem(void);
	void beatReport(void);
	void turnReport(void);
};

struct foraged_good
{
    int zone;
    int vnum;
    int sector;
    int rarity;
};

class foraged_zone
{
public:
    int zone;
    int type;
    int count;
    int vc_thresh;
    int c_thresh;
    int uc_thresh;
    int r_thresh;
    int vr_thresh;

    foraged_zone()
    {
        zone = 0;
        type = 0;
        count = -1;
        vc_thresh = -1;
        c_thresh = -1;
        uc_thresh = -1;
        r_thresh = -1;
        vr_thresh = -1;
    }
};

class arena_gladiators
{
public:
    int id;
    std::string name;
    std::string alias;
    std::string shorts;
    int win;
    int lose;
    int fought;
    int season;

    arena_gladiators()
    {
      id = 0;
      win = 0;
      lose = 0;
      fought = 0;
      season = 1;
    }
};

class arena_fights
{
public:
    int id;
    std::string red_name;
    std::string red_short;
    std::string blue_name;
    std::string blue_short;
    int red_win;
    int blue_win;
    int degree;
    int odds_top;
    int odds_under;
    int season;

    arena_fights()
    {
      id = 0;
      red_win = 0;
      blue_win = 0;
      degree = 0;
      odds_top = 0;
      odds_under = 0;
      season = 1;
    }
};

class arena_bets
{
public:
    int id;
    std::string name;
    std::string sdesc;
    std::string glad_name;
    std::string glad_short;
    int amount;
    int win_lose;
    int odds_top;
    int odds_under;
    int season;

    arena_bets()
    {
      id = 0;
      amount = 0;
      win_lose = 0;
      odds_top = 0;
      odds_under =0;
      season = 1;
    }
};


struct overwatch_enemy
{
    std::string sdesc; // OK, who shot us?
    int source;        // manual = 0, leader = 1, traded = 2, group = 3
    int time;          // how long are they our enemy? Unless they do something else, limit is 4 in-game hours.
};

struct targeted_bys
{
	CHAR_DATA *ch;
	TARGETED_BY *next;
};

struct second_affect
{
    int type;
    int seconds;
    CHAR_DATA *ch;
    OBJ_DATA *obj;
    char *info;
    int info2;
    SECOND_AFFECT *next;
};

struct var_data
{
    char *name;
    int value;
    int type;
    struct var_data *next;
};

#define MPF_BROKEN		(1 << 0)

struct mobprog_data
{
    char *trigger_name;
    char *prog;
    char *line;
    int busy;
    int flags;
    int mob_virtual;
    struct mobprog_data *next;
    struct mobprog_data *next_full_prog;
};

struct dream_data
{
    char *dream;
    DREAM_DATA *next;
};

struct site_info
{
    char *name;
    char *banned_by;
    int banned_on;
    int banned_until;
    SITE_INFO *next;
};

#define CHARGEN_INT_FIRST		( 1 << 0 )
#define CHARGEN_INT_SECOND		( 1 << 1 )
#define CHARGEN_DEX_1ST2ND		( 1 << 2 )

/* These should match the order of the rows in the spells */
/* database, since they're read in ascending order. */

#define VAR_ID				0
#define VAR_NAME			1
#define VAR_COST			2
#define VAR_SPHERE			3
#define VAR_CIRCLE			4
#define VAR_CH_ECHO			5
#define VAR_SELF_ECHO			6
#define VAR_VICT_ECHO			7
#define VAR_ROOM_ECHO			8
#define VAR_SPELL_DESC			9
#define VAR_TARGET_TYPE			10
#define VAR_SAVE_TYPE			11
#define VAR_SAVE_FOR			12
#define VAR_DAMAGE_FORM			13
#define VAR_AFFECT1			14
#define VAR_DICE1			15
#define VAR_ACT1			16
#define VAR_FADE1			17
#define VAR_AFFECT2			18
#define VAR_DICE2			19
#define VAR_ACT2			20
#define VAR_FADE2			21
#define VAR_AFFECT3			22
#define VAR_DICE3			23
#define VAR_ACT3			24
#define VAR_FADE3			25
#define VAR_AFFECT4			26
#define VAR_DICE4			27
#define VAR_ACT4			28
#define VAR_FADE4			29
#define VAR_AFFECT5			30
#define VAR_DICE5			31
#define VAR_ACT5			32
#define VAR_FADE5			33
#define VAR_AUTHOR			34
#define VAR_LAST_MODIFIED		35

#include <string>

struct monitor_data
{
	public:

	CHAR_DATA	*watcher;
	CHAR_DATA	*victim;
	int			room;
	int			zone;
	char		*clans;
	bitflag			bits;
	int			radio_freq;

	monitor_data()
	{
		watcher = NULL;
		victim = NULL;
		room = 0;
		zone = -1;
		clans = '\0';
		bits = 0;
		radio_freq = 0;
	}
};

struct pc_data
{
    DREAM_DATA *dreams;
    DREAM_DATA *dreamed;
    int create_state;		/* Approval system */
    int mortal_mode;		/* Immortals can act mortal */
    int edit_obj;
    int edit_mob;
    int level;
    int boat_virtual;
    int staff_notes;
    int mount_speed;
    CHAR_DATA *edit_player;
    CHAR_DATA *target_mob;	/* Used by 'mob' and 'resets' */
    char *msg;			/* SUBMIT/APPROVAL system */
    char *unused_01;		// formerly 'email_address'
    char *creation_comment;
    char *imm_enter;		///< Immortals only
    char *imm_leave;
    char *site_lie;		/* Lie about connecting site */
    int start_str;
    int start_dex;
    int start_con;
    int start_wil;
    int start_aur;
    int start_intel;
    int start_agi;
    int load_count;		/* See load_pc */
    int unused_02;		// formerly 'common'
    int chargen_flags;
    int last_global_pc_msg;
    int last_global_staff_msg;
    int sleep_needed;
    int auto_toll;		/* Amount willing to pay if following */
    int doc_type;			/* Not saved.  Current doc */
    int doc_index;		/* Not saved.  type relative index */
    struct descriptor_data *owner;
    shortint skills[MAX_SKILLS];
    CHAR_DATA *dot_shorthand;	/* '.' indicates last referenced n/pc */
    ALIAS_DATA *aliases;
    ALIAS_DATA *execute_alias;
    time_t last_logon;
    time_t last_logoff;
    time_t last_disconnect;
    time_t last_connect;
    time_t last_died;
    char *account_name;
    OBJ_DATA *writing_on;
    SUBCRAFT_HEAD_DATA *edit_craft;
    int app_cost;
    bitflag nanny_state;
    bitflag role;
    ROLE_DATA *special_role;
    bool admin_loaded;
    int time_last_activity;
    int is_guide;
    int profession;
    int power_level;

    // Japh's corruptiopn fixes;
    pc_data();
    ~pc_data();
    void deep_copy (pc_data *copy_from);

    time_t last_wounded; // needed to prevent crashes from healing you.
};

struct threat_data
{
    CHAR_DATA *source;
    int level;
    THREAT_DATA *next;
};

struct attacker_data
{
    CHAR_DATA *attacker;
    ATTACKER_DATA *next;
};

struct sighted_data
{
    CHAR_DATA *target;
    SIGHTED_DATA *next;
};

enum mob_cue
{
    cue_none,
    cue_notes,		// Freeform notes
    cue_flags,		// Flags (unimplemented)
    cue_memory,		// Var = Value

    // Scheduled Cues
    cue_on_hour,		// (hr/sun/moon) reflex
    cue_on_time,		// (min/%) reflex

    // Personal Cues
    cue_on_health,	// (hp%) reflex
    cue_on_moves,		// (mp%) reflex

    // Socials
    cue_on_command,	// (command key (with rvnum)) reflex
    cue_on_receive,	// (vnum) reflex
    cue_on_hear,		// (phrase) reflex
    cue_on_nod,		// (nodder) reflex

    // Crime Cues
    cue_on_theft,
    cue_on_witness,	// (pick,steal,assault)

    // Combat Cues
    cue_on_engage,
    cue_on_flee,
    cue_on_scan,		// (specific enemy) reflex
    cue_on_hit,		// reflex

    cue_on_reboot,         // do this on the reboot.
    cue_mob_present,       // when we see something in a room.
    cue_obj_present,       // when we see something in a room.

    cue_on_death,

	cue_on_five,
	cue_on_one,

    cue_blank
};

struct mob_data
{
    int vnum;
    int zone;
    int spawnpoint;
    int merch_seven;		/* Merchant set up for 7 economic rules like Regi wanted - punt in time */
    int skinned_vnum;		/* What mob skins into */
    int carcass_vnum;		/* What mob leaves for a carcass */
    int vehicle_type;		/* Mobile value: boat, etc */
    int helm_room;		/* If boat, defines helm */
    int access_flags;		/* Flags; mob room access */
    int noaccess_flags;		/* Flags; mob room noaccess */
    int reset_zone;		/* mobs only */
    int reset_cmd;		/* mobs only */
    int damnodice;
    int damsizedice;
    int damroll;
    int min_height;		// min height for proto define
    int max_height;		// min height for proto define
    int size;			// race size for proto define
    CHAR_DATA *hnext;
    CHAR_DATA *lnext;
    COMBAT_DATA *combat;
    RESET_DATA *resets;
    int currency_type;
    char *owner;
    int jail;			/* What jail the enforcer brings criminals to */
    int fallback;                 // What vnum the mobile will retreat to.
    int armortype;                // What "mobile" armour they've got.

    // Mob Triggers
    std::multimap<mob_cue,std::string> *cues;

    // Japh's corruption fixes
    mob_data();
    ~mob_data();
    void deep_copy (mob_data *copy_from);

    int controller; // The coldload_id of the object that is controlling us.
	int ai_delay;
};

#include "room.h"


/* ================== Structure for player/non-player ===================== */
struct char_data
{
    int in_room;
    ROOM_DATA *room;
    int deleted;
    int circle;			/* Rank within church */
    int fight_mode;		/* Frantic..defensive */
    int debug_mode;
    int primary_delay;		/* ticks to next hit */
    int secondary_delay;		/* ticks to next hit */
    int coldload_id;		/* Uniq id of mob */
    int natural_delay;		/* Added delay for natural */
    int body_type;		/* Determines hit locations */
    int scent_type;		/* Bite causes poison */
    int nat_attack_type;		/* Bite, claw, slime, etc */
    int flags;			/* FLAG_ stuff */
    int move_points;		/* Not saved; move remainder */
    int hit_points;		/* Not saved; hit remainder */
    int nightmare;		/* Nightmare sequence # */
    int speaks;			/* Currently spoken language */
    int alarm;			/* Not saved. */
    int trigger_delay;		/* Not saved. */
    int trigger_line;		/* Used with trigger_delay */
    int trigger_id;		/* Used with trigger_delay */
    int psionic_talents;
    CHAR_DATA *subdue;		/* Subduer or subduee */
    MOBPROG_DATA *prog;
    struct var_data *vartab;	/* Mob program variable table */
    SHOP_DATA *shop;
    CHAR_DATA *vehicle;		/* Char that is the vehicle */
    int str;
    int intel;
    int wil;
    int dex;
    int con;
    int aur;
    int agi;
    int tmp_str;
    int tmp_intel;
    int tmp_wil;
    int tmp_dex;
    int tmp_con;
    int tmp_aur;
    int tmp_agi;
    shortint skills[MAX_SKILLS];
    AFFECTED_TYPE *hour_affects;
    OBJ_DATA *equip;
    //struct descriptor_data *desc;
    CHAR_DATA *next_in_room;
    CHAR_DATA *next;
    CHAR_DATA *next_fighting;
    CHAR_DATA *next_assist;
    int assist_pos;
    CHAR_DATA *following;
    PC_DATA *pc;
    MONITOR_DATA *monitor;
    MOB_DATA *mob;
    MOVE_DATA *moves;
    char *casting_arg;		/* Contains delayed spell */
    int hit;
    int max_hit;
    int move;
    int max_move;
    int armor;
    int offense;
    int ppoints;
    CHAR_DATA *fighting;
    int distance_to_target;
    struct memory_data *remembers;
    long affected_by;
    int position;
    int default_pos;
    bitflag act;
    bitflag hmflags;
    int carry_weight;
    int carry_items;
    int delay_type;
    int delay;
    char *delay_who;
    char *delay_who2;
    char *delay_who3;
    char *delay_who4;
    CHAR_DATA *delay_ch;
    OBJ_DATA *delay_obj;
	OBJ_DATA *purchase_obj;
    int delay_info1;
    int delay_info2;
    int delay_info3;
    int delay_info4;
    int delay_info5;
    int was_in_room;
    int intoxication;
    int hunger;
    int thirst;
    int last_room;
    int attack_type;
    char *name;
    char *tname;
    char *short_descr;
    char *long_descr;
    char *pmote_str;
    char *voice_str;
    char *description;
    int sex;
    int deity;
    int race;
    int color;
    int sound;
    int speed;
    int age;
    int height;
    int frame;
    int totem;
    struct time_data time;
    char *clans;
    CHAR_DATA *mount;		/* Rider/Ridee */
    CHAR_DATA *hitcher;
    CHAR_DATA *hitchee;
    char *combat_log;
    WOUND_DATA *wounds;
    int damage;
    int lastregen;
    int defensive;
    int cell_1, cell_2, cell_3;	/* NPC jailer cell VNUMS */
    int laststuncheck;
    int knockedout;
    int writes;
    int stun;
    int curse;
    CHAR_DATA *aiming_at;
    int aim;
    LODGED_OBJECT_INFO *lodged;
    int shock;
    int max_shock;
    int harness;
    int max_harness;
    int preparing_id;
    int preparing_time;
    ENCHANTMENT_DATA *enchantments;
    int roundtime;
    OBJ_DATA *right_hand;
    OBJ_DATA *left_hand;
    bitflag plr_flags;
    CHAR_DATA *ranged_enemy;
    char *enemy_direction;
    //std::vector<threat_data*> threats;
    THREAT_DATA *threats;
    ATTACKER_DATA *attackers;
    int whirling;
    int from_dir;
    SIGHTED_DATA *sighted;
    SCENT_DATA *scent;
    int balance;
    char *travel_str;
    int spells[MAX_LEARNED_SPELLS][2];
    int body_proto;
    int bmi;
    int size;
    unsigned short int guardian_mode;
    int hire_storeroom;
    int hire_storeobj;
    char *dmote_str;
    int cover_from_dir;
    int morph_type;
    int clock;
    int morph_time;
    int morphto;
    int craft_index; //remembers which item was used in craft key-pair
    bool enforcement [100]; // keeps track of zones this char is enforcer in
    // std::set<CHAR_DATA*> *group; // complement to ch->following
    std::string *plan;
    std::string *goal;
    bool bleeding_prompt;
    int room_pos;
    int formation;
    GROUP_DATA *group;
    long talents;
    int combat_counter[MAX_SKILLS];
    int compete_dam;
    int effort;
    int combat_block;

    // Japh's mem fixing stuff
    void clear_char();
    char_data();
    ~char_data();
    void partial_deep_copy (CHAR_DATA *proto);
    void deep_copy (CHAR_DATA *copy_from);

    // Kith's auto-gen description stuff
    char *d_age;
    char *d_eyes;
    char *d_hairlength;
    char *d_haircolor;
    char *d_hairstyle;
    char *d_height;
    char *d_frame;
    char *d_feat1;
    char *d_feat2;
    char *d_feat3;
    char *d_feat4;

    DESCRIPTOR_DATA *descriptor;
    DESCRIPTOR_DATA * descr ();

    CHAR_DATA *controlled_by;	// Char-data of who's going to see what we're seeing.
    CHAR_DATA *controlling;		// Char-data of who's stuff we get to see.

    int defense_mode;

	//std::vector<targeted_bys*> *targeted_by;

	TARGETED_BY *targeted_by;

    std::vector<overwatch_enemy*> *over_enemies;
    int over_target; // 0 = manual only, 1 = leader + manual, 2 = direct shots, 3 = group trades
    int over_thresh; // 0 = high opportunity (not in cover), 1 = medium opportunity (crawling), 2 = low opportunity (rising out of cover)

    CHAR_DATA *death_ch;
    OBJ_DATA *death_obj;

	int flee_dir;
};

/* ======================================================================== */

#include "weather.h"

struct constant_data
{
    char constant_name[20];
    char description[80];
    void **index;
};

#define args( list )	list

typedef int SPELL_FUN args ((CHAR_DATA * ch, CHAR_DATA * victim, int sn));
typedef int SPELL_PURE args ((CHAR_DATA * ch, CHAR_DATA * victim));

struct spell_data
{
    char *name;			/* Name of skill              */
    SPELL_FUN *spell_fun;		/* Spell pointer (for spells) */
    int target;			/* Legal targets              */
    int piety_cost;		/* Minimum mana used          */
    int delay;			/* Waiting time BEFORE use     */
    char *msg_off;		/* Wear off message           */
};

struct spell_table_data
{
    char *name;
    SPELL_PURE *spell_fun;
    int discipline;
    int energy_source;
    long cost;
    int target;
};

#define STR_ONE_LINE		2001
#define STR_MULTI_LINE		2000

#define MAX_NATURAL_SKILLS	50

/* Max number of race defines in table */

#define MAX_NUM_RACES		500

typedef struct race_data RACE_TABLE_ENTRY;

struct race_data
{
    int id;
    char *name;
    int pc_race;
    int starting_locs;
    int rpp_cost;
    char *created_by;
    int last_modified;
    int race_size;
    int body_proto;
    int innate_abilities;
    int str_mod;
    int con_mod;
    int dex_mod;
    int agi_mod;
    int int_mod;
    int wil_mod;
    int aur_mod;
    int native_tongue;
    int min_age;
    int max_age;
    int min_ht;
    int max_ht;
    int fem_ht_adj;
    int max_hit;
    int max_move;
    int armor;
    int group_noun;
    int tracks;
    int corpse;
    int type;
    int door_bits;
    int bot_bits;
    int nomad;
    int alert;
    int movement;

    RACE_TABLE_ENTRY *next;
};

/* Body prototypes for wound location definitions */

#define PROTO_HUMANOID			0
#define PROTO_FOURLEGGED_PAWS	1
#define PROTO_FOURLEGGED_HOOVES	2
#define PROTO_FOURLEGGED_FEET	3
#define PROTO_WINGED_TAIL		4
#define PROTO_WINGED_NOTAIL		5
#define PROTO_SERPENTINE		6
#define PROTO_SPIDER            7
#define PROTO_GHOST             8
#define PROTO_STRUCTURE         9
#define PROTO_ENT               10
#define PROTO_BOT				11

/* Racial define innate abilities */

#define INNATE_INFRA			( 1 << 0 )
#define INNATE_FLYING			( 1 << 1 )
#define INNATE_WAT_BREATH		( 1 << 2 )
#define INNATE_NOBLEED			( 1 << 3 )
#define INNATE_SUN_PEN			( 1 << 4 )
#define INNATE_SUN_PET			( 1 << 5 )
#define INNATE_PREY				( 1 << 6 )	// We hide and flee as often as we can.
#define INNATE_STALKER			( 1 << 7 )	// We attack wounded people on their lonesome.

/* Possible starting locations for race defines */

#define RACE_HOME_OSGILIATH		( 1 << 0 )
#define RACE_HOME_MORGUL		( 1 << 1 )

/* These should match the order of the rows in the races */
/* database, since they're read in ascending order. */

#define RACE_NAME			0
#define RACE_ID				1
#define RACE_RPP_COST		2
#define RACE_DESC			3
#define RACE_START_LOC		4
#define RACE_PC				5
#define RACE_AFFECTS		6
#define RACE_BODY_PROTO		7
#define RACE_SIZE			8
#define RACE_STR_MOD		9
#define RACE_CON_MOD		10
#define RACE_DEX_MOD		11
#define RACE_AGI_MOD		12
#define RACE_INT_MOD		13
#define RACE_WIL_MOD		14
#define RACE_AUR_MOD		15
#define RACE_MIN_AGE		16
#define RACE_MAX_AGE		17
#define RACE_MIN_HEIGHT		18
#define RACE_MAX_HEIGHT		19
#define RACE_FEM_HT_DIFF	20
#define RACE_NATIVE_TONGUE	21
#define RACE_SKILL_MODS		22
#define RACE_CREATED_BY		23
#define RACE_LAST_MODIFIED	24
#define RACE_MAX_HIT		25
#define RACE_MAX_MOVE		26
#define RACE_ARMOR			27
#define RACE_GROUP_NOUN		28
#define RACE_TRACKS			29
#define RACE_CORPSE			30
#define RACE_TYPE			31
#define RACE_DOOR_BITS		32
#define RACE_BOT_BITS		33
#define RACE_NOMAD			34
#define RACE_ALERT			35
#define RACE_MOVEMENT		36

#define LAST_RACE			36

#define RACE_TRACKS_SMALL    1	 // Only leave tracks if bleeding
#define RACE_TRACKS_NEVER    2	 // Never, ever leave tracks.

#define RACE_TYPE_NORMAL 	0    // Mobile is as normal
#define RACE_TYPE_OBJECT	1	 // Mobile is in-game, but not alive -- a wall, a cart, etc
#define RACE_TYPE_OOC		2    // Mobile is not in-game - it is a trigger.

#define RACE_DOOR_OPEN		1 << 0	// Mobile will opens doors.
#define RACE_DOOR_CLOSE		1 << 1  // Mobile will close doors.

#define RACE_BOT_PASSIVE	1 << 0  // mobile has no AI routines
#define RACE_BOT_REMOTE		1 << 1	// mobile can be controlled by a remoe
#define RACE_BOT_MONITOR	1 << 2  // mobile can be monitored.

#define RACE_ALERT_NORMAL	0
#define RACE_ALERT_HOWL		1
#define RACE_ALERT_SCREECH	2
#define RACE_ALERT_SMELL	3

#define RACE_MOVEMENT_WALK		0
#define RACE_MOVEMENT_FLY		1
#define RACE_MOVEMENT_SLITHER	2
#define RACE_MOVEMENT_SCUTTLE	3
#define RACE_MOVEMENT_DRIFT		4

#define RACE_GROUP_HUMAN		0
#define RACE_GROUP_HUMANOID		1
#define RACE_GROUP_INSECT		2
#define RACE_GROUP_AUTOMATON	3
#define RACE_GROUP_BOT			4
#define RACE_GROUP_VEHICLE		5
#define RACE_GROUP_DOG			6
#define RACE_GROUP_LIZARD		7
#define RACE_GROUP_SPIDER		8
#define RACE_GROUP_RODENT		9
#define RACE_GROUP_BAT			10
#define RACE_GROUP_ANIMAL		11

struct religion_data
{
    char *tree[30];
};

#define MAX_REGISTRY        20

typedef struct registry_data REGISTRY_DATA;

struct registry_data
{
    int value;
    char *string;
    REGISTRY_DATA *next;
};

struct fight_data
{
    char name[12];
    float offense_modifier;
    float defense_modifier;
    int delay;
};

struct language_data
{
    char name[16];
};


// CLUSTER_DEIFNED DEFINITIONS HERE

#define CLUS_ONE_SKILLED         ( 1 << 0 )
#define CLUS_TWO_SKILLED         ( 1 << 1 )
#define CLUS_ONE_HIGH            ( 1 << 2 )
#define CLUS_TWO_HIGH            ( 1 << 3 )
#define CLUS_ONE_LOW             ( 1 << 4 )
#define CLUS_TWO_LOW             ( 1 << 5 )
#define CLUS_FAIL_TRAP           ( 1 << 6 )
#define CLUS_NO_GUARANTEE        ( 1 << 7 )
#define CLUS_FAIL_SOMATIC        ( 1 << 8 )
#define CLUS_ONE_HIDE            ( 1 << 9 )
#define CLUS_TWO_HIDE            ( 1 << 10 )
#define CLUS_ONE_KEEP            ( 1 << 11 )
#define CLUS_TWO_KEEP            ( 1 << 12 )
#define CLUS_EMPTY_SEARCH        ( 1 << 13 )
#define CLUS_ONE_TEST            ( 1 << 14 )
#define CLUS_TWO_TEST            ( 1 << 15 )
#define CLUS_ONE_MINOR           ( 1 << 16 )
#define CLUS_ONE_MAJOR           ( 1 << 17 )
#define CLUS_TWO_MINOR           ( 1 << 18 )
#define CLUS_TWO_MAJOR           ( 1 << 19 )
#define CLUS_DELAY_SMALL         ( 1 << 20 )
#define CLUS_DELAY_LARGE         ( 1 << 21 )
#define CLUS_JUNK_PRODUCE        ( 1 << 22 )
#define CLUS_APPEAR_HIDDEN       ( 1 << 23 )
#define CLUS_ONE_COLOR           ( 1 << 24 )
#define CLUS_TWO_COLOR           ( 1 << 25 )
#define CLUS_JUNK_COLOR          ( 1 << 26 )

// GUN-RELATED DEFINITIONS HERE

#define GUN_SAFETY             ( 1 << 0 ) // Gun has the safety feature								1
#define GUN_BURST              ( 1 << 1 ) // Gun has the burst feature								2
#define GUN_AUTOMATIC          ( 1 << 2 ) // Gun has automatic fire feature							4
#define GUN_SILENCABLE         ( 1 << 3 ) // Gun can have a silencer attached.						8
#define GUN_SILENT             ( 1 << 4 ) // Gun is inherently silent.								16
#define GUN_DISPLAY            ( 1 << 5 ) // Gun displays how much ammo is left.					32
#define GUN_ELEC_DISPLAY       ( 1 << 6 ) // Gun displays how much ammo is left.					64
#define GUN_ELEC_LINK          ( 1 << 7 ) // Gun displays how much ammo is left.					128
#define GUN_ELEC_AUTO          ( 1 << 8 ) // Gun displays how much ammo is left.					256
#define GUN_TELE_SIGHT         ( 1 << 9 ) // Gun displays how much ammo is left.					512
#define GUN_INFRA_SIGHT        ( 1 << 10 ) // Gun displays how much ammo is left.					1024
#define GUN_LASER_SIGHT        ( 1 << 11 ) // Gun displays how much ammo is left.					2048
#define GUN_DIRECT_ONE         ( 1 << 12 ) // Gun doesn't take clips, but is directly loaded. 		4096
#define GUN_DIRECT_FIVE        ( 1 << 13 ) // Gun doesn't take clips, but is directly loaded.		8192
#define GUN_DIRECT_SIX         ( 1 << 14 ) // Gun doesn't take clips, but is directly loaded.		16384
#define GUN_MAGAZINE_SMALL     ( 1 << 15 ) // Gun doesn't take clips, but is directly loaded.		32768
#define GUN_MAGAZINE_MEDIUM    ( 1 << 16 ) // Gun doesn't take clips, but is directly loaded.		65536
#define GUN_MAGAZINE_LARGE     ( 1 << 17 ) // Gun doesn't take clips, but is directly loaded.		131072
#define GUN_MAGAZINE_HUGE      ( 1 << 18 ) // Gun doesn't take clips, but is directly loaded.		262144
#define GUN_MAGAZINE_BELT      ( 1 << 19 ) // Gun is loaded via a belt

// GRENADES
#define GRENADE_SMOKE          0
#define GRENADE_STUN           1
#define GRENADE_FRAG           2
#define GRENADE_TEAR           3
#define GRENADE_CONCUSSIVE     4
#define GRENADE_INCINDARY      5

// CALIBERS

#define GUN_CAL_20	0
#define GUN_CAL_25	1
#define GUN_CAL_30	2
#define GUN_CAL_35	3
#define GUN_CAL_40	4
#define GUN_CAL_45	5
#define GUN_CAL_50	6
#define GUN_CAL_55	7
#define GUN_CAL_60	8

// AMMOS

#define GUN_AMMO_JACKETED      	0
#define GUN_AMMO_HOLLOW		 	1
#define GUN_AMMO_ARMOR		 	2
#define GUN_AMMO_INCINDARY  	3
#define GUN_AMMO_TRACER			4


// SETS

#define GUN_FIRE_SEMI	0
#define GUN_FIRE_SAFETY	1
#define GUN_FIRE_BURST	2
#define GUN_FIRE_AUTO	3
#define GUN_FIRE_PACKED 4

// ELECTRIC RELATED DEFINIONS HERE

// What tags can our electric goods have?
#define ELEC_PWR_BATTERY       ( 1 << 0 ) // takes battery-type batteries
#define ELEC_PWR_PACK          ( 1 << 1 ) // takes pack-type batteries
#define ELEC_PWR_FUSION        ( 1 << 2 ) // takes fusion-type batteries
#define ELEC_PWR_INTERNAL      ( 1 << 3 ) // is internally powered
#define ELEC_FEA_POWER         ( 1 << 4 ) // reveals what level of battery power is remaining via look.
#define ELEC_FEA_RADIO_ENCRYPT ( 1 << 5 ) // reveals what level of battery power is remaining via look.
#define ELEC_FEA_RADIO_SET     ( 1 << 6 ) // radio cannot have its channel changed.
#define ELEC_FEA_RADIO_SCAN    ( 1 << 7 ) // radio can scan for frequency
#define ELEC_FEA_RADIO_ONEWAY  ( 1 << 8 ) // radio can't transmit anything.
#define ELEC_FEA_WATERPROOF	   ( 1 << 9 ) // item isn't switched off and damaged by water

// What types of batteries do we have?
#define BATT_BATTERY           0
#define BATT_PACK              1
#define BATT_SOLAR             2

// TRAP-RELATED DEFINITIONS HERE

#define TRAP_ON_EXIT		( 1 << 0 )      // Trigger can be set on exit
#define TRAP_ON_OBJ		( 1 << 1 )      // Trigger can be set on object
#define TRAP_ON_RELEASE		( 1 << 2 )      // Trigger can be set to be player-released
#define TRAP_AFF_DAMAGE		( 1 << 3 )      // Component does damage
#define TRAP_AFF_TRIP		( 1 << 4 )      // Component trips target over
#define TRAP_AFF_BIND		( 1 << 5 )      // Component holds target in bind
#define TRAP_AFF_SOMATIC	( 1 << 6 )      // Component imparts a somatic affect
#define TRAP_BREAK_USE          ( 1 << 7 )      // When used, this component produces its junk objects
#define TRAP_BREAK_RESET        ( 1 << 8 )      // When reset, this component produces its junk objects.
#define TRAP_TRIG_MUST          ( 1 << 9 )      // This component must be used as the trigger.
#define TRAP_TRIG_AVOID         ( 1 << 10)      // You get a chance of avoiding this trigger, based on an attribute and the skill it was hidden with.
#define TRAP_TAR_SOLE           ( 1 << 11)      // This component only ever targets a sole person.
#define TRAP_MISC_ALWAYS        ( 1 << 12)      // Even if you know this trap is there, it still targets you.
#define TRAP_MISC_INVIS         ( 1 << 13)      // Searching won't reveal this trap.
#define TRAP_MISC_MYSTERY       ( 1 << 14)      // Looking at this trap won't reveal what it does.
#define TRAP_TAR_TEMP           ( 1 << 15)      // For targeting sole people, we'll temporarily add this bit so we don't target more than one person.

struct trap_data
{
    int trap_bit; // what bits the trap has
    int com_diff; // what difficulty using this component is
    int com_cost[5]; // how many bits this component costs, 0 = general, 1 = electronic, 2 = mechanical, 3 = computerology, 4 = chemical
    int com_target_dice; // how many people this trap will hit, -2 for everyone in room, -1 for everyone in group, 0 for triggerer else:
    int com_target_side;
    int com_uses; // how many uses this trap has remaining till it hits junk, -1 = forever-more
    int com_junk1; // when uses hits zero, what object vnum is loaded in the room
    int com_junk2; // when uses hits zero, what object vnum is loaded in the room
    int com_shots; // how many more times this component can be used before it needs to be reset
    int com_total_shots; // total times a component can be used till it needs to be reset.
    int com_trig_shots; // how many times this can be used to trigger before it needs to be reset.
    int com_skill; // Whether use of this component requires any particular skill

    int avoid_atr;   // what attribute folks get to roll against the help avoid (if it is the trigger)
    int avoid_dice;  // How many dice are rolled
    int avoid_sides; // How many sides of the dice are rolled.

    int dam1_dice;   // This is all pretty self explanatory.
    int dam1_sides;
    int dam1_type;
    int dam1_loc;    // set 8 for random except eyes, hands and feet.
    int dam1_times;
	int dam1_bonus;
    int dam2_dice;
    int dam2_sides;
    int dam2_type;
    int dam2_loc;
    int dam2_times;
	int dam2_bonus;

    int bind_atr; // what attribute you should use to escape the bind
    int bind_str; // how strong the bind is

    char *trigger_1st; // what message is delivered when the trap is sprung
    char *trigger_3rd; // what message is delivered when the trap is sprung
    char *strike_1st;  // what message is delivered when the component is used
    char *strike_3rd;  // what message is delivered when the component is used
    char *shout_1;     // what message is delivered to surrounding rooms when the component is used
    char *shout_2;     // what message is delivered to rooms 2 rooms away when the component is used

    //TRAP_DATA *next;
};

struct encumberance_info
{
    int str_mult_wt;		/* if wt <= str * str_mult, then element applies */
    int defense_pct;
    int delay;
    int offense_pct;
    int move;
    float penalty;
    char *encumbered_status;
};

#define NUM_BUCKETS 1024
#define ENCUMBERANCE_ENTRIES	6

/* The FLAG_ bits are saved with the player character */

#define FLAG_KEEPER		( 1 << 0 )
#define FLAG_COMPACT		( 1 << 1 )	/* Player in compact mode */
#define FLAG_BRIEF		( 1 << 2 )	/* Player in brief mode */
#define FLAG_WIZINVIS		( 1 << 3 )
#define FLAG_SUBDUEE		( 1 << 4 )
#define FLAG_SUBDUER		( 1 << 5 )
#define FLAG_SUBDUING		( 1 << 6 )
#define FLAG_ANON		( 1 << 7 )
#define FLAG_COMPETE		( 1 << 8 )
#define FLAG_LEADER_1		( 1 << 9 )	/* Clan 1 leader */
#define FLAG_LEADER_2		( 1 << 10 )	/* Clan 2 leader */
#define FLAG_DEAD		( 1 << 11 )	/* Player has been killed */
#define FLAG_KILL		( 1 << 12 )	/* Player intends to kill */
#define FLAG_FLEE		( 1 << 13 )	/* Player wants to flee combat */
#define FLAG_BINDING		( 1 << 14 )	/* NPC is curently tending wounds */
#define FLAG_SEE_NAME		( 1 << 15 )	/* Show mortal name in says */
#define FLAG_AUTOFLEE		( 1 << 16 )	/* Flee automatically in combat */
#define FLAG_ENTERING		( 1 << 17 )
#define FLAG_LEAVING		( 1 << 18 )
#define FLAG_INHIBITTED		( 1 << 19 )	/* Mob event blocking on program */
#define FLAG_NOPROMPT		( 1 << 20 )	/* Make prompt disappear */
#define FLAG_NEWBNET            ( 1 << 21 )	/* Tuned in to NewbNet */
#define FLAG_ALIASING		( 1 << 22 )	/* Executing an alias */
#define FLAG_OSTLER		( 1 << 23 )	/* Stablemaster */
#define FLAG_TELEPATH 		( 1 << 24 )	/* Hears PC thoughts */
#define FLAG_PACIFIST		( 1 << 25 )	/* Character won't fight back */
#define FLAG_WIZNET		( 1 << 26 )	/* Immortal wiznet toggle */
#define FLAG_HARNESS		( 1 << 27 )	/* Display harness in prompt */
#define FLAG_VARIABLE		( 1 << 28 )	/* Randomized mob prototype */
#define FLAG_ISADMIN		( 1 << 29 )	/* Is an admin's mortal PC */
#define FLAG_AVAILABLE		( 1 << 30 )	/* Available for petitions */
#define FLAG_GUEST		( 1 << 31 )	/* Guest login */

/* plr_flags */

#define NEWBIE_HINTS		( 1 << 0 )	/* Toggle the hint system on or off */
#define NEWBIE			( 1 << 1 )	/* Has not yet commenced */
#define MORALE_BROKEN		( 1 << 2 )
#define MORALE_HELD		( 1 << 3 )
#define FLAG_PETRIFIED		( 1 << 4 )
#define NEW_PLAYER_TAG		( 1 << 5 )	/* Displays (new player) in their ldescs */
#define MENTOR			( 1 << 6 )	/* PC Mentor flag */
#define NOPETITION		( 1 << 7 )	/* No Petition */
#define PRIVATE			( 1 << 8 )	/* Non-Guide-reviewable app */
#define START_GONDOR            ( 1 << 9 )	/* Human, chose to start in Gondor */
#define START_MORDOR            ( 1 << 10 )	/* Human, chose to start in Mordor */
#define NO_PLAYERPORT		( 1 << 11 )	/* Admins w/out admin access to 4500 */
#define MUTE_BEEPS		( 1 << 12 )	/* Doesn't receive NOTIFY beeps */
#define COMBAT_FILTER		( 1 << 13 )	/* Only receives local combat messages */
#define GROUP_CLOSED		( 1 << 14 )	/* Not accepting any other followers */
#define QUIET_SCAN		( 1 << 15 )     /* quick and quiet scan when entering rooms */
#define NO_BUILDERPORT          ( 1 << 16 )     /* Admins w/o access to 4501 */
#define AUTO_FIRE               ( 1 << 17 )     /* Fire at quick-moving people */
#define HIGHLIGHT               ( 1 << 18 )     // Personalied highlighting.
#define COMBAT_DISPLAY          ( 1 << 19 )     // Displays quick combat info.
#define GUIDE                   ( 1 << 20 )     // Player is a guide.
#define AUTOSCAN                ( 1 << 21 )     // Player will attempt to scan direction he is watching upon entering room.
#define BRIEF_MODE              ( 1 << 22 )     // Players sees a lot less output, mostly for vision-impairment.
#define FIREFIGHT_FILTER        ( 1 << 23 )     // Players sees a lot less firearm-related spam.
#define AUTO_COVER				( 1 << 24 )     // Players toss themselves in to cover automatially

/* char_data.guardian_flags - controls notification of PC initiated attacks */
#define GUARDIAN_PC		( 1 << 0 )	/* 01 */
#define GUARDIAN_NPC_HUMANOIDS	( 1 << 1 )	/* 02 */
#define GUARDIAN_NPC_WILDLIFE 	( 1 << 2 )	/* 04 */
#define GUARDIAN_NPC_SHOPKEEPS	( 1 << 3 )	/* 08 */
#define GUARDIAN_NPC_SENTINELS	( 1 << 4 )	/* 10 */
#define GUARDIAN_NPC_KEYHOLDER	( 1 << 5 )	/* 20 */
#define GUARDIAN_NPC_ENFORCERS	( 1 << 6 )	/* 40 */

#define STATE_NAME		( 1 << 0 )	/* Enter name */
#define STATE_GENDER		( 1 << 1 )	/* Choose gender */
#define STATE_RACE		( 1 << 2 )	/* Choose race */
#define STATE_AGE		( 1 << 3 )	/* Input age */
#define STATE_ATTRIBUTES	( 1 << 4 )	/* Distribute attributes */
#define STATE_SDESC		( 1 << 5 )	/* Enter short desc */
#define STATE_LDESC		( 1 << 6 )	/* Enter long desc */
#define STATE_FDESC		( 1 << 7 )	/* Enter full desc */
#define STATE_KEYWORDS		( 1 << 8 )	/* Enter keywords */
#define STATE_FRAME		( 1 << 9 )	/* Choose frame */
#define STATE_SKILLS		( 1 << 10 )	/* Skill selection */
#define STATE_COMMENT		( 1 << 11 )	/* Creation comment */
#define STATE_ROLES		( 1 << 12 )	/* Hardcoded roles/advantages */
#define STATE_SPECIAL_ROLES	( 1 << 13 )	/* Admin-posted special roles */
#define STATE_PRIVACY		( 1 << 14 )	/* Flag app private? */
#define STATE_LOCATION		( 1 << 15 )	/* Humans choose start loc */
#define STATE_PROFESSION	( 1 << 16 )	/* Choosing profession */
#define STATE_AUTO_DESC         ( 1 << 17 )     /* Picking automated desc */

/* Hardcoded starting roles/options */

#define EXTRA_COIN		( 1 << 0 )	/* Starts with surplus coin; 1 point */
#define APPRENTICE		( 1 << 1 )	/* Starts as low-ranking member of established org; 1 point */
#define STARTING_ARMOR		( 1 << 2 )	/* Starts with a full set of leather armor; 2 points */
#define SKILL_BONUS		( 1 << 3 )	/* Small bonus to starting skills; 2 points */
#define EXTRA_SKILL		( 1 << 4 )	/* Extra starting skill; 3 points */
#define MAXED_STAT		( 1 << 5 )	/* Starts with one guaranteed 18; 3 points */
#define JOURNEYMAN		( 1 << 6 )	/* As apprentice, but higher-level; 4 points */
#define FELLOW			( 1 << 7 )	/* Starts as a fully-fledged Fellow; 5 points */
#define LESSER_NOBILITY		( 1 << 8 )	/* Starts in a minor noble/merchant family; 6 points */
#define APPRENTICE_MAGE		( 1 << 9 )	/* Self-explanatory; 7 points */

#define TRIG_DONT_USE	0
#define TRIG_SAY		1
#define TRIG_ENTER		2
#define TRIG_EXIT		3
#define TRIG_HIT		4
#define TRIG_MOBACT		5
#define TRIG_ALARM		6
#define TRIG_HOUR		7
#define TRIG_DAY		8
#define TRIG_TEACH		9
#define TRIG_WHISPER	10
#define TRIG_PRISONER	11
#define TRIG_KNOCK		12

#define GAME_BASE_YEAR		83   // Translates to BETA start of 147 years since founding of NP
#define GAME_SECONDS_BEGINNING  100	/* Subtr 10800 to ++gametime 12hr */
#define GAME_SECONDS_PER_YEAR	31104000
#define GAME_SECONDS_PER_MONTH	2592000
#define GAME_SECONDS_PER_DAY	86400
#define GAME_SECONDS_PER_HOUR	3600

#define MOON_SECONDS_PER_HOUR	3600
#define MOON_SECONDS_PER_DAY	302400   // 84 hours per phase
#define MOON_SECONDS_PER_MONTH	2419200  // 8 phases per Lunar day
#define MOON_SECONDS_PER_YEAR	31449600 // 13 Days per Year


#define MODE_COMMAND		(1 << 0)
#define MODE_DONE_EDITING	(1 << 1)
#define MODE_VISEDIT		(1 << 2)
#define MODE_VISHELP		(1 << 3)
#define MODE_VISMAP			(1 << 4)

#define HOME				"\033[H"
#define CLEAR				"\033[2J"

#define DEAD		0
#define MORT		1
#define UNCON		2
#define STUN		3
#define SLEEP		4
#define REST		5
#define SIT		6
#define FIGHT		7
#define STAND		8

#define C_LV1		( 1 << 0 )	/* Immortal level 1 */
#define C_LV2		( 1 << 1 )	/* Immortal level 2 */
#define C_LV3		( 1 << 2 )	/* Immortal level 3 */
#define C_LV4		( 1 << 3 )	/* Immortal level 4 */
#define C_LV5		( 1 << 4 )	/* Immortal level 5 */
#define	C_DEL		( 1 << 5 )	/* Will not break a delay */
#define C_SUB		( 1 << 6 )	/* Commands legal while subdued */
#define C_HID		( 1 << 7 )	/* Commands that keep character hidden */
#define C_DOA		( 1 << 8 )	/* Commands allowed when dead */
#define C_BLD		( 1 << 9 )	/* Commands allowed when blind */
#define C_WLK		( 1 << 10 )	/* Commands NOT allowed while moving */
#define C_XLS		( 1 << 11 )	/* Don't list command */
#define C_MNT		( 1 << 12 )	/* Commands NOT allowed while mounted */
#define C_PAR		( 1 << 13 )	/* Things you CAN do paralyzed */
#define C_GDE		( 1 << 14 )	/* Guide-only command */
#define C_SPL		( 1 << 15 )	/* Commands legal while preparing spell */
#define C_NLG		( 1 << 16 )	/* Command is not logged */
#define C_NWT		( 1 << 17 )	/* Command doesn't show up in SNOOP or WATCH */
#define C_IMP		( 1 << 18 )	/* IMPLEMENTOR ONLY */
#define C_ROB       ( 1 << 19 ) // robots have to check specific features, or they can't do this.

struct command_data
{
    char *command;
    void (*proc) (CHAR_DATA * ch, char *argument, int cmd);
    int min_position;
    int flags;
};

struct social_data
{
    char *social_command;
    int hide;
    int min_victim_position;	/* Position of victim */

    /* No argument was supplied */

    char *char_no_arg;
    char *others_no_arg;

    /* An argument was there, and a victim was found */

    char *char_found;		/* if NULL, read no further, ignore args */
    char *others_found;
    char *vict_found;

    /* An argument was there, but no victim was found */

    char *not_found;

    /* The victim turned out to be the character */

    char *char_auto;
    char *others_auto;
};

/* data files used by the game system */

#define DFLT_DIR               	"lib"	/* default data directory     */
#define NEWS_FILE       	"news"	/* for the 'news' command     */
#define QSTAT_FILE	 	"text/chargen/stat_message"
#define QSTAT_HELP	  	"text/stat_help"
#define GREET_FILE	  	"text/greetings"
#define MAINTENANCE_FILE	"text/greetings.maintenance"
#define MENU1_FILE	  	"text/menu1"
#define ACCT_APP_FILE		"text/account_application"
#define ACCT_REF_FILE		"text/account_referral"
#define ACCT_EMAIL_FILE		"text/account_email"
#define ACCT_POLICIES_FILE	"text/account_policies"
#define THANK_YOU_FILE		"text/thankyou"
#define CMENU_FILE	  	"text/cmenu"
#define NEW_MENU		"text/new_menu"
#define PDESC_FILE		"text/chargen/new_desc"
#define NAME_FILE		"text/chargen/new_name"
#define PKEYWORDS_FILE	"text/chargen/new_keyword"
#define PLDESC_FILE		"text/chargen/new_ldesc"
#define PSDESC_FILE		"text/chargen/new_sdesc"
#define PLAYER_NEW		"text/player_new"
#define PLAYER_FILE     "save/players"	/* the player database        */
#define SOCMESS_FILE    "text/actions"	/* messgs for social acts     */
#define HELP_FILE		"text/help_table"	/* for HELP <keywrd>          */
#define BHELP_FILE		"text/bhelp_table"	/* for BHELP <keywrd>         */
#define HELP_PAGE_FILE  "text/help"	/* for HELP <CR>              */
#define BHELP_PAGE_FILE "text/bhelp"	/* for BHELP <CR>             */
#define INFO_FILE       "text/info"	/* for INFO                   */
#define WIZLIST_FILE    "text/wizlist"	/* for WIZLIST                */
#define NEW_CHAR_INTRO	"text/new_char_intro"	/* ground rules                       */
#define RACE_SELECT		"text/chargen/race_select"	/* Race choice question           */
#define AGE_SELECT		"text/chargen/age_select"	/* Age choice question        */
#define SEX_SELECT		"text/chargen/sex_select"	/* Sex choice question        */
#define ROLE_SELECT		"text/chargen/role_select"	/* Explanation of roles */
#define SPECIAL_ROLE_SELECT	"text/chargen/special_role_select"	/* Explanation of special roles */
#define HEIGHT_FRAME	"text/chargen/height_frame"
#define COMMENT_HELP	"text/chargen/comment_help"
#define LOCATION	"text/chargen/location"
#define SKILL_SELECT	"text/chargen/skills_select"
#define PROFESSION_SELECT	"text/chargen/professions"

#define	AUTODESC_HEIGHT		"text/chargen/autodesc_height"
#define	AUTODESC_FRAME		"text/chargen/autodesc_frame"
#define	AUTODESC_EYES		"text/chargen/autodesc_eyes"
#define	AUTODESC_EYES_SPECIFIC	"text/chargen/autodesc_eyes_specific"
#define	AUTODESC_LENGTH		"text/chargen/autodesc_length"
#define	AUTODESC_COLOR		"text/chargen/autodesc_color"
#define	AUTODESC_COLOR_SPECIFIC	"text/chargen/autodesc_color_specific"
#define	AUTODESC_STYLE		"text/chargen/autodesc_style"
#define	AUTODESC_FEAT		"text/chargen/autodesc_feat"
#define	AUTODESC_FEAT_SPECIFIC	"text/chargen/autodesc_feat_specify"

#define	AUTODESC_FEAT_UNHUMAN		"text/chargen/autodesc_feat_unhuman"
#define	AUTODESC_FEAT_SPECIFIC_UNHUMAN	"text/chargen/autodesc_feat_specify_unhuman"
#define	AUTODESC_END_UNHUMAN		"text/chargen/autodesc_end_unhuman"

#define	AUTODESC_END		"text/chargen/autodesc_end"
#define	AUTODESC_CHOICE		"text/chargen/autodesc_choice"
#define	AUTODESC_CHOICE2	"text/chargen/autodesc_choice2"
#define	AUTODESC_FINAL		"text/chargen/autodesc_final"


#define SAVE_DIR		"save"
#define HELP_DIR	"../lib/help/"
#define REGIONS			"../regions"
#define REGISTRY_FILE	REGIONS "/registry"
#define CRAFTS_FILE		REGIONS "/crafts"
#define BOARD_DIR		"boards"
#define JOURNAL_DIR		"player_journals"
#define BOARD_ARCHIVE	"archive"
#define PLAYER_BOARD_DIR "player_boards"
#define VIRTUAL_BOARD_DIR	"vboards"
#define TICKET_DIR		"tickets"
#define AUCTION_DIR		"auctions"
#define ORDER_DIR       "orders"
#define COLDLOAD_IDS	"coldload_ids"
#define DYNAMIC_REGISTRY REGIONS "/dynamic_registry"
#define STAYPUT_FILE	"stayput"

#define ZONE_SIZE	1000
#define MAX_TRADE	5
#define MAX_PROD	5

/* structure for the reset commands */

struct reset_affect
{
    char *craft_name;
    int type;
    int duration;
    int modifier;
    int location;
    int bitvector;
    int t;
    int sn;
};

struct reset_com
{
    char command;			/* current command                      */
    bool if_flag;			/* if true: exe only if preceding exe'd */
    int arg1;			/* (Can be ptr to reset_affect data)    */
    int arg2;			/* Arguments to the command             */
    int arg3;			/*                                      */
    int arg4;			/*                                      */
    int arg5;			/*                                      */
    int enabled;			/* true if this reset should be used    */

    /*
     *  Commands:              *
     *  'M': Read a mobile     *
     *  'O': Read an object    *
     *  'G': Give obj to mob   *
     *  'P': Put obj in obj    *
     *  'G': Obj to char       *
     *  'E': Obj to char equip *
     *  'D': Set state of door *
     *  'C': Craft (an affect) *
     *  'A': Affect            *
     *  'R': Room #            *
     *  'r': Room Affect       *
     *  'm': Mob reset (timed, reply variety)
     */
};

/* zone definition structure. for the 'zone-table'   */

struct zone_data
{
    char *name;			/* name of this zone                  */
    char *lead;			/* Name of the project lead             */
    int lifespan;			/* how long between resets (minutes)  */
    int age;			/* current age of this zone (minutes) */
    int top;
    int frozen;
    unsigned long flags;
    int reset_mode;		/* conditions for reset (see below)   */
    struct reset_com *cmd;	/* command table for reset                  */
    int jailer;
    int jail_room_num;
    ROOM_DATA *jail_room;
    int earth_mod;
    int wind_mod;
    int fire_mod;
    int water_mod;
    int shadow_mod;
    int player_in_zone;
    int weather_type;
    char *sunrise;
    char *sunset;
    char *dawn;
    char *dusk;
};

#define MAX_MSGS_PER_BOARD		5000
#define MAX_LANG_PER_BOARD		  10

struct board_data
{
    char *name;
    char *title;
    int level;
    int next_virtual;
    int msg_nums[MAX_MSGS_PER_BOARD];
    char *msg_titles[MAX_MSGS_PER_BOARD];
    int language[MAX_LANG_PER_BOARD];
    BOARD_DATA *next;
};

#define MF_READ		( 1 << 0 )
#define MF_ANON		( 1 << 1 )
#define MF_PRIVATE	( 1 << 2 )
#define MF_URGENT	( 1 << 3 )
#define MF_DREAM	( 1 << 4 )
#define MF_REPLIED	( 1 << 5 )
#define MF_DELETED	( 1 << 6 )
#define MF_SENDER_DELETED	( 1 << 7 )

struct mudmail_data
{
    long flags;
    char *from;
    char *from_account;
    char *date;
    char *subject;
    char *message;
    MUDMAIL_DATA *next_message;
    char *target;
};


#define TO_ROOM		( 1 << 0 )
#define TO_VICT		( 1 << 1 )
#define TO_NOTVICT	( 1 << 2 )
#define TO_CHAR		( 1 << 3 )
#define _ACT_FORMAT	( 1 << 4 )
#define TO_IMMS		( 1 << 5 )
#define _ACT_COMBAT	( 1 << 6 )
#define TO_GROUP	( 1 << 7 )
#define _ACT_SEARCH	( 1 << 8 )
#define _ACT_BLEED      ( 1 << 9 )
#define _HIGHLIGHT      ( 1 << 10 )
#define _ACT_FIREFIGHT  ( 1 << 11 )

struct fatigue_data
{
    int percent;
    float penalty;
    char name[24];
};

struct use_table_data
{
    int delay;
};

/* NOTE:  If adding a new COMBAT_, update cs_name in fight.c */

#define SUC_CF				0
#define SUC_MF				1
#define SUC_MS				2
#define SUC_CS				3

/* NOTE:  If adding a new RESULT_, update rs_name in fight.c */

#define RESULT_NONE			0
#define RESULT_ADV			1
#define RESULT_BLOCK			2
#define RESULT_PARRY			3
#define RESULT_FUMBLE			4
#define RESULT_HIT			5
#define RESULT_HIT1			6
#define RESULT_HIT2			7
#define RESULT_HIT3			8
#define RESULT_HIT4			9
#define RESULT_STUMBLE			10
#define RESULT_NEAR_FUMBLE		11
#define RESULT_NEAR_STUMBLE		12
#define RESULT_DEAD			13
#define RESULT_ANY			14
#define RESULT_WEAPON_BREAK 		15
#define RESULT_SHIELD_BREAK 		16
#define RESULT_KO			17
#define RESULT_JUST_KO			18

struct combat_msg_data
{
    int off_result;
    int def_result;
    int table;
    char *def_msg;
    char *off_msg;
    char *other_msg;
    COMBAT_MSG_DATA *next;
};


#define STATE_REJECTED		-1
#define STATE_APPLYING		0
#define STATE_SUBMITTED		1
#define STATE_APPROVED		2
#define STATE_SUSPENDED		3
#define STATE_DIED			4

#define MAP_FLEE_BACKSTAB	0

struct map_free_backstab_dt
{
    int origin;
    CHAR_DATA *attacker;
    int hid;
    int backstabed;
};

struct combat_data
{
    int prog;
    union
    {
        struct map_free_backstab_dt backstab;
    } c;
};

struct move_data
{
    int dir;
    int flags;
    int desired_time;
    MOVE_DATA *next;
    char *travel_str;
    char *group_str;
};

struct qe_data
{
    /* Quarter second events data structure */
    CHAR_DATA *ch;
    int dir;
    int speed_type;
    int flags;
    ROOM_DATA *from_room;
    int event_time;
    int arrive_time;
    int move_cost;
    QE_DATA *next;
    char *travel_str;
    char *group_str;
};

#define MF_WALK			( 1 << 0 )
#define MF_RUN			( 1 << 1 )
#define MF_SWIM			( 1 << 2 )
#define MF_PASSDOOR		( 1 << 3 )
#define MF_ARRIVAL		( 1 << 4 )
#define MF_TOEDGE		( 1 << 5 )
#define MF_TONEXT_EDGE	( 1 << 6 )
#define MF_SNEAK		( 1 << 7 )
#define MF_CRAWL        ( 1 << 8 )
#define MF_FLOAT 		( 1 << 9 )

#define SPEED_WALK		0
#define SPEED_CRAWL		1
#define SPEED_PACED		2
#define SPEED_JOG		3
#define SPEED_RUN		4
#define SPEED_SPRINT	5
#define SPEED_IMMORTAL	6
#define SPEED_SWIM		7
#define SPEED_CREEP     8
#define SPEED_FLOAT 	9

typedef int PP ();
typedef int PP_ch (CHAR_DATA * ch, int success, int psn);
typedef int PP_ch_victim (CHAR_DATA * ch, CHAR_DATA * victim, int success,
                          int psn);
typedef int PP_ch_obj (CHAR_DATA * ch, OBJ_DATA * obj, int success, int psn);

#define FRAME_FEATHER		0
#define FRAME_SCANT		1
#define FRAME_LIGHT		2
#define FRAME_MEDIUM		3
#define FRAME_HEAVY		4
#define FRAME_MASSIVE		5
#define FRAME_SUPER_MASSIVE	6

#define SIZE_UNDEFINED		0
#define SIZE_XXS		1	/* Smaller than PC sized mobs */
#define SIZE_XS			2
#define SIZE_S			3
#define SIZE_M			4
#define SIZE_L			5
#define SIZE_XL			6
#define SIZE_XXL		7	/* Larger than PC sized mobs */

struct mob_race_data
{
    char race_name[30];
    int male_ht_dice;
    int male_ht_sides;
    int male_ht_constant;
    int male_fr_dice;
    int male_fr_sides;
    int male_fr_constant;
    int female_ht_dice;
    int female_ht_sides;
    int female_ht_constant;
    int female_fr_dice;
    int female_fr_sides;
    int female_fr_constant;
    int can_subdue;
};

struct cbt
{
    char bt[100];
};

struct ibt
{
    int i[25];
};

#define CRIME_KILL			1	/* 5 hours, see criminalize() */
#define CRIME_STEAL			2	/* 3 hours */
#define CRIME_PICKLOCK		3	/* 1 hour */
#define CRIME_BACKSTAB		4	/* 5 hours */
#define CRIME_SUBDUE		5	/* 1 hour */
#define CRIME_ASSAULT       6
#define CRIME_SHOOT			7
#define CRIME_TRAPS			8


struct memory_t
{
    int dtype;
    int entry;
    int bytes;
    int time_allocated;
};

#define PHASE_CANNOT_LEAVE_ROOM ( 1 << 0 )
#define PHASE_OPEN_ON_SELF		( 1 << 1 )
#define PHASE_REQUIRE_ON_SELF   ( 1 << 2 )
#define PHASE_REQUIRE_GREATER	( 1 << 3 )

/* These flags apply to objects within a phase */

#define SUBCRAFT_IN_ROOM		( 1 << 0 )
#define SUBCRAFT_GIVE			( 1 << 1 )	// Give to Crafter's hands on production
#define SUBCRAFT_HELD			( 1 << 2 )
#define SUBCRAFT_WIELDED		( 1 << 3 )
#define SUBCRAFT_USED			( 1 << 4 )
#define SUBCRAFT_PRODUCED		( 1 << 5 )
#define SUBCRAFT_WORN			( 1 << 6 )
#define SUBCRAFT_SAMPLED		( 1 << 7 )
#define SUBCRAFT_PROGRESSED		( 1 << 8 )
#define SUBCRAFT_OPTIONAL		( 1 << 9 )
#define SUBCRAFT_SAMPLED_HELD	( 1 << 10 )
#define SUBCRAFT_SAMPLED_ROOM	( 1 << 11 )
#define SUBCRAFT_USED_HELD	    ( 1 << 12 )
#define SUBCRAFT_USED_ROOM	    ( 1 << 13 )
#define SUBCRAFT_PROGRESSED_HELD	( 1 << 14 )
#define SUBCRAFT_PROGRESSED_ROOM	( 1 << 15 )
#define SUBCRAFT_OPTIONAL_HELD		( 1 << 16 )
#define SUBCRAFT_OPTIONAL_ROOM		( 1 << 17 )

/* Craft Mobile flags */

#define CRAFT_MOB_SETOWNER		( 1 << 0 )

/* Subcraft flags */

#define SCF_TARGET_OBJ			( 1 << 0 )	/* Target object */
#define SCF_TARGET_CHAR			( 1 << 1 )	/* Target character */
#define SCF_DEFENSIVE			( 1 << 2 )	/* Defensive - default */
#define SCF_OFFENSIVE			( 1 << 3 )	/* Offensive spell */
#define SCF_AREA				( 1 << 4 )	/* Area spell, all objs or chars */
#define SCF_ROOM				( 1 << 5 )	/* Target room */
#define SCF_TARGET_SELF			( 1 << 6 )	/* Target self */
#define SCF_OBSCURE    			( 1 << 7 )	/* Hide from Website/Palantir */

#define MAX_ITEMS_PER_SUBCRAFT	150
#define MAX_DEFAULT_ITEMS		40

struct subcraft_head_data
{
    char *craft_name;
    char *subcraft_name;
    char *command;
    char *failure;		/* Failure message for subcraft */
    char *failobjs;		/* String of failure object numbers */
    char *failmobs;		/* String of failure object numbers */
    char *help;
    char *clans;
    PHASE_DATA *phases;
    DEFAULT_ITEM_DATA *obj_items[MAX_ITEMS_PER_SUBCRAFT];
    int subcraft_flags;
    long crafts_start;
    long crafts_end;
    int sectors[25];
    int seasons[7];
    int opening[25];
    int race[25];
    int weather[9];
    int failmob;			/* VNUM of mob it loads up on failure */
    int delay;
    int faildelay;
    int key_index;
    int key_first;			/* which one of the items lists is the key*/
    int key_end;	/* which production item list will be indexed for the key*/
    DEFAULT_ITEM_DATA *fails[MAX_ITEMS_PER_SUBCRAFT];
    int followers; /* number of people following the caller if the craft */
    char *start_prog; // the command of the prog to call
    char *fail_prog; // the command of the prog to call
    char *end_prog; // the command of the prog to call
    char *argument; // the argument used by progs
    char *load_color[MAX_DEFAULT_ITEMS][10];
	char *load_cat[MAX_DEFAULT_ITEMS][10];
	int load_oval[MAX_DEFAULT_ITEMS][6];
    int color_round;
    CRAFT_VARIABLE_DATA *craft_variable[MAX_DEFAULT_ITEMS];
	CRAFT_OVAL_DATA *craft_oval[MAX_DEFAULT_ITEMS];
    SUBCRAFT_HEAD_DATA *next;
};

struct phase_data
{
    char *first;			/* First person message */
    char *second;			/* Second person message */
    char *third;			/* Third person message */
    char *self;			/* If targets self, use this as first */
    char *second_failure;		/* Failure message to 2nd person */
    char *third_failure;		/* Failure message to 3rd persons */
    char *failure;		/* Failure message for phase */
    int flags;			/* PHase flags */
    int phase_seconds;		/* Time in seconds of phase */
    int skill;			/* Only used for skill checks */
    int attr; 		/* Used for attribute check (like skill) */
    int dice;			/* dice v skill */
    int sides;			/* sides v skill (diceDsides v skill) */
    int targets;			/* Spell target flags */
    int duration;			/* Hours on spell */
    int power;			/* Power of spell.  eg -3 to +3 str */
    int hit_cost;			/* Hit cost of phase */
    int move_cost;		/* Move cost of phase */
    int spell_type;		/* Spell number */
    int open_skill;		/* Skill to open (crafter or target) */
    int req_skill;		/* Required skill */
    int req_skill_value;		/* Value req_skill must be > or < */
    int attribute;		/* Used for attribute check (like skill) */
    DEFAULT_ITEM_DATA *tool;	/* Usable tools */
    int load_mob;			/* Mob VNUM */
    int nMobFlags;		/* Mob Flags (Stayput, Set Owner) */
    char *group_mess;		/* message to groups followers */
    char *fail_group_mess; /* Failure message for groups */
    char *phase_start_prog; // run this at the start of the phase
    char *phase_end_prog; // run this at the end of the phase
    char *phase_fail_prog; // run this when the craft fails, at the end of the phase
    char *text; // What text apperials in the materials column of the phase
    PHASE_DATA *next;
};

struct default_item_data
{
    int flags;			/* See SUBCRAFT_ flags */
    int items[MAX_DEFAULT_ITEMS];	/* Up to 20 default items */
    short item_counts;		/* instances of items */
    PHASE_DATA *phase;		/* Phase mentioned */
    int key_index;		/* index for key items */
    char *color;
    char *color2;
    char *color3;
};

struct craft_variable_data
{
    PHASE_DATA *phase;		/* Phase mentioned */
    int from;                     // Which item will this go to?
    int to;                     // Which item will this go to?
    char *category;               // Which category to take
    int pos;                      // Where to put it.
};

struct craft_oval_data
{
    PHASE_DATA *phase;		/* Phase mentioned */
    int from;                   // Which item will this go to?
    int to;                     // Which item will this go to?
    int oval;                   // Which oval is it?
};


#define SCF_CANNOT_LEAVE_ROOM		( 1 << 0 )	/* SCF = Subcraft flag */

struct alias_data
{
    char *command;
    char *line;
    ALIAS_DATA *next_line;
    ALIAS_DATA *next_alias;
};

#define RESET_TIMED		1
#define RESET_REPLY		2

struct reset_data
{
    int type;
    char *command;
    int planned;			/* Gets set a minute before reset activates */
    RESET_TIME_DATA when;
    RESET_DATA *next;
};

struct text_data
{
    char *filename;
    char *name;
    char *text;
    TEXT_DATA *next;
};

#define EDIT_TYPE_DOCUMENT	0
#define EDIT_TYPE_HELP		1
#define EDIT_TYPE_BHELP		2
#define EDIT_TYPE_CRAFT		3
#define EDIT_TYPE_TEXT		4

struct help_info
{
    char *keyword;
    char *master_list;
    int required_level;
    char *info;
    HELP_INFO *master_element;
    HELP_INFO *next;
};

struct help_data
{
    char *keyword;
    char *keywords;
    char *help_info;		/* Will be null if main_element used */
    HELP_DATA *master_element;	/* Contains actual help_info */
    HELP_DATA *next;
};

struct name_switch_data
{
    char *old_name;
    char *new_name;
    NAME_SWITCH_DATA *next;
};

#define ECON_ZONES 13
struct econ_data
{
    char flag_name[25];
    struct
    {
        float markup;
        float discount;
    } obj_econ_info[ECON_ZONES];
};

#define QUALITY_TRASH		1 << 21
#define QUALITY_POOR		1 << 7
#define QUALITY_ORDINARY	1 << 17
#define QUALITY_GOOD		1 << 18
#define QUALITY_SUPERB		1 << 19
#define QUALITY_EPIC		1 << 20


#define AGE_BABY        0
#define AGE_CHILD	1
#define AGE_TEEN	2
#define AGE_YOUNG	3
#define AGE_ADULT	4
#define AGE_MATURE	5
#define AGE_AGED	6
#define AGE_ELDERY	7
#define AGE_SKELETON    8

#endif //_rpie_structs_h
