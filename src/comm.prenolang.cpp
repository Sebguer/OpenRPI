/*------------------------------------------------------------------------\
|  comm.c : Central Game Loop                         www.middle-earth.us |
 |  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
 |  Derived under license from DIKU GAMMA (0.0).                           |
 \------------------------------------------------------------------------*/

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/resource.h>

#include "server.h"

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

#include "decl.h"
#include "group.h"
#include "room.h"

extern int errno;
///< Global error number

/* local globals */
rpie::server engine;
int nMainSocket;
int knockout; /* Cleanup dead pcs */
int run_mult = 10000;

int morgul_arena_time = 0;
int te_pit_time = 0;
volatile sig_atomic_t crashed = 0;
bool debug_mode = true; /* True to enable core dumps in crash recovery. */

bool bIsCopyOver = false;
bool maintenance_lock = false;

int finished_booting = 0;
int guest_conns = 0;
int arena_matches = 0;
extern int new_auctions;
extern int sold_auctions;
extern int auction_bids;
long starttime, crashtime;
int count_guest_online = 0;
long bootstart;
bool pending_reboot = false;
int shutd = 0; /* clean shutdown */
int tics = 0; /* for extern checkpointing */

void check_reboot( void );

int run_the_game( int port );
int game_loop( int s );
int init_socket( int port );
int new_descriptor( int s );
int process_output( DESCRIPTOR_DATA * t );
int process_input( DESCRIPTOR_DATA * t );
void close_sockets( int s );
void close_socket( DESCRIPTOR_DATA * d );
struct timeval timediff( struct timeval *a, struct timeval *b );
void flush_queues( DESCRIPTOR_DATA * d );
void check_sitebans();
void parse_name( DESCRIPTOR_DATA * desc, char *arg );
int valgrind = 0;
struct timeval time_now;

/* extern fcnts */

CHAR_DATA *make_char( char *name, DESCRIPTOR_DATA * desc );
void boot_db( void );
void zone_update( void );
void point_update( void ); /* In limits.c */
void mobile_activity( void );
void string_add( DESCRIPTOR_DATA * d, char *str );
void perform_violence( void );
void stop_fighting( CHAR_DATA * ch );
void show_string( DESCRIPTOR_DATA * d, char *input );

/* *********************************************************************
 *  main game loop and related stuff				       *
 ********************************************************************* */

int main( int argc, char *argv[] ) {
	char buf[ 512 ];
	engine.load_config_files();
	bootstart = time( 0 );

	if ( chdir( DFLT_DIR ) < 0 ) {
		std::string error_message = "The system call 'chdir' failed to switch to the directory '";
		error_message += DFLT_DIR;
		error_message += "' for the following reason";
		perror( error_message.c_str() );
		fprintf( stderr, "Did you run 'make install'?\n"
				"Does 'make install-libdir' fail?\n" );
		exit( 1 );
	}

	int port = 0;
	if ( argc < 2 || !isdigit( *argv[ 1 ] ) || ( port = strtol( argv[ 1 ], 0, 0 ) ) <= 1024 ) {
		fprintf( stderr, "Please specify a port number above 1024.\n"
				"E.G: $ server 4500\n" );
		exit( 0 );
	}

	if ( argv[ 2 ] && *argv[ 2 ] == '-' ) {
		if ( *( argv[ 2 ] + 1 ) == 'c' ) {
			bIsCopyOver = true;
			chdir( "lib" );
			nMainSocket = atoi( argv[ 3 ] );
		} else if ( *( argv[ 2 ] + 1 ) == 'v' ) {
			valgrind = true;
		}
	}
	engine.set_config( "server_port", std::string( argv[ 1 ] ) );

	init_mysql();

	sprintf( buf, "Running game on port %d.", port );

	system_log( buf, false );

	srand( time( 0 ) );
	run_the_game( port );
	return ( 0 );

}

#define	RPINET		0	/* Currently disabled. */

/* Init sockets, run game, and cleanup sockets */
int run_the_game( int port ) {
	FILE *fp;
	struct rlimit rlp;

	void signal_setup( void );

	if ( !( fp = fopen( "booting", "r" )) ) {
		fp = fopen( "booting", "w+" );
		fclose( fp );
		system_log( "Crash loop check initiated.", false );
	} else {
		system_log( "Lockfile found during bootup - shutting down.", false );
		fprintf( stderr, "Lockfile found during bootup - shutting down.\n" );
		fclose( fp );
		exit( -1 );
	}

	system_log( "Signal trapping.", false );
	signal_setup();

	if ( ( fp = fopen( ".reboot", "r" )) ) {
		fgets( BOOT, 26, fp );
		fclose( fp );
		unlink( ".reboot" );
	}

	system( "ulimit -c unlimited" );

	system_log( "Initializing CPU cycle alarm.", false );
	init_alarm_handler();

	if ( !bIsCopyOver ) {
		system_log( "Opening mother connection.", false );
		nMainSocket = init_socket( port );
	}

	boot_db();

	system_log( "Entering game loop.", false );
	starttime = time( 0 );

	getrlimit( RLIMIT_CORE, &rlp );
	rlp.rlim_cur = rlp.rlim_max;
	setrlimit( RLIMIT_CORE, &rlp );
	getrlimit( RLIMIT_CORE, &rlp );

	if ( ( fp = fopen( "last_crash", "r" )) ) {
		crashtime = fread_number( fp );
		fclose( fp );
	}

	if ( engine.in_play_mode() ) {
		mysql_safe_query( "UPDATE server_statistics SET last_reboot = %d", ( int ) ( time( 0 )) );
	}

	game_loop( nMainSocket );

	if ( engine.in_play_mode() ) {
		save_tracks();
		save_stayput_mobiles();
		save_player_rooms();
		save_dwelling_rooms();
		save_banned_sites();
	}

	if ( engine.in_build_mode() )
		update_crafts_file();

	mysql_safe_query( "DELETE FROM players_online WHERE port = %d", port );

	close_sockets( nMainSocket );

	if ( engine.in_play_mode() ) {
		fp = fopen( PATH_TO_WEBSITE "/stats.shtml", "w+" );

		if ( fp != NULL)
		{
			fprintf( fp, "Our game server seems to be down. Our apologies for any inconvenience.\n" );
			fclose( fp );
		}
	}

	system_log( "Normal termination of game.", false );

	return 0;
}

/* Accept new connects, relay commands, and call 'heartbeat-functs' */

int game_loop( int s ) {
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	fd_set visedit_fds;
	fd_set tmp_read_fds;
	fd_set tmp_write_fds;
	//std::string comm;
	char comm[ MAX_STRING_LENGTH ] = "";
	DESCRIPTOR_DATA *point, *next_point, *next_to_process;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch, *next_ch;
	int pulse = 0, purse = 0;
	int mask;
	int i;
	struct timeval null_time;
	struct timeval pulse_time;
	struct timeval current_time;
	struct rlimit limit;
	bool first_loop = true;
	extern bool morgul_arena_fight; // Defined in arena.c
	extern bool te_pit_fight; // Defined in arena.c
	extern QE_DATA *quarter_event_list;

	timerclear( &null_time );
	/* a define in sys/time.h */
	FD_ZERO( &readfds );
	FD_ZERO( &writefds );
	FD_ZERO( &exceptfds );
	FD_ZERO( &visedit_fds );
	FD_ZERO( &tmp_read_fds );
	FD_ZERO( &tmp_write_fds );

	maxdesc = s;

	if ( bIsCopyOver ) {
		copyover_recovery();
		bIsCopyOver = false;
		if ( s <= 0 ) {
			system_log( "Mother descriptor not found after copyover. Shutting down.", true );
			shutd = 1;
		}
	}

	check_maintenance();

	morgul_arena_time = ( int ) time( 0 );
	te_pit_time = ( int ) time( 0 );

#ifndef MACOSX
	getrlimit( RLIMIT_NOFILE, &limit ); /* Determine max # descriptors */
#else
	limit.rlim_cur = limit.rlim_max = (rlim_t) 1024;
#endif

	avail_descs = limit.rlim_max - 2;

	/*mask = sigmask (SIGINT) | sigmask (SIGPIPE) | sigmask (SIGALRM)
	 | sigmask (SIGTERM) | sigmask (SIGURG) | sigmask (SIGXCPU)
	 | sigmask (SIGHUP);*/

	gettimeofday( &time_now, NULL );

	finished_booting = 1;

	unlink( "booting" );
	unlink( "recovery" );

	while ( !shutd ) {
		FD_ZERO( &readfds );
		FD_ZERO( &writefds );
		FD_ZERO( &exceptfds );
		FD_ZERO( &visedit_fds );
		FD_SET( s, &readfds );

		for ( d = descriptor_list; d; d = d->next ) {
			FD_SET( d->hSocketFD, &readfds );
			FD_SET( d->hSocketFD, &exceptfds );
			FD_SET( d->hSocketFD, &writefds );

			if ( !IS_SET( d->edit_mode, MODE_DONE_EDITING ) )
				FD_SET( d->hSocketFD, &visedit_fds );
		}

		gettimeofday( &current_time, NULL );

		pulse_time = timediff( &current_time, &time_now );

		/* Compensate if we're not getting enough CPU */

		if ( pulse_time.tv_sec || pulse_time.tv_usec > 25 * run_mult ) {
			if ( pulse_time.tv_sec > 2 ) {
				// was sprintf(comm,...)
				printf( "Insufficient CPU! %ld:%ld sec between slices", ( long ) pulse_time.tv_sec,
						( long ) pulse_time.tv_usec );
				/* system_log (comm, true); */
			}

			pulse_time.tv_sec = 0;
			pulse_time.tv_usec = 1;
			time_now = current_time;
		} else
			pulse_time.tv_usec = 25 * run_mult - pulse_time.tv_usec;

		sigprocmask( SIG_SETMASK, ( sigset_t * ) &mask, 0 );

		if ( select( maxdesc + 1, &readfds, &writefds, &exceptfds, &null_time ) < 0 ) {
			perror( "Select poll" );
			return -1;
		}

		if ( select( 0, ( fd_set * ) 0, ( fd_set * ) 0, ( fd_set * ) 0, &pulse_time ) < 0 ) {
			perror( "Select sleep" );
			exit( 1 );
		}

		/* time_now is set to the exact time we expected our time slice.
		 It may be slightly off.  However, it allows us to maintain a
		 perfect 1/4 second pulse on average. */

		time_now.tv_usec += 25 * run_mult;
		if ( time_now.tv_usec > 100 * run_mult ) {
			time_now.tv_usec -= 100 * run_mult;
			time_now.tv_sec++;
		}

		sigprocmask( SIG_SETMASK, ( sigset_t * ) 0, 0 );
		engine.set_abort_threshold_post_booting();

		/* Drop connections */
		descriptor__drop_connections( &readfds, &writefds, &exceptfds );

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		/* New connections */

		if ( FD_ISSET (s, &readfds) )
			if ( new_descriptor( s ) < 0 )
				perror( "New connection" );

		for ( point = descriptor_list; point; point = next_point ) {
			next_point = point->next;
			if ( FD_ISSET (point->hSocketFD, &readfds) )
				if ( process_input( point ) < 0 ) {
					FD_CLR( point->hSocketFD, &readfds );
					FD_CLR( point->hSocketFD, &exceptfds );
					FD_CLR( point->hSocketFD, &writefds );
					close_socket( point );
				}
		}

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		/* process_commands; */

		for ( point = descriptor_list; point; point = next_to_process ) {
			next_to_process = point->next;

			if ( ( --( point->wait ) <= 0 ) && get_from_q( &point->input, comm ) ) {
				point->wait = 1;

				/* reset idle time */

				point->time_last_activity = mud_time;

				if ( point->character && !point->character->deleted && point->character->pc != NULL
				)
					point->character->pc->time_last_activity = mud_time;

				if ( point->original && !point->original->deleted && point->original->pc != NULL
				)
					point->original->pc->time_last_activity = time( 0 );

				point->prompt_mode = 1;

				if ( point->edit_index != -1 )
					edit_string( point, comm );
				else if ( point->str )
					string_add( point, comm );
				else if ( point->showstr_point )
					show_string( point, comm );
				else if ( !point->connected ) {
					if ( point->showstr_point )
						show_string( point, comm );
					else if ( point->character )
						command_interpreter( point->character, comm );
				} else
					nanny( point, comm );
			}
		}

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		for ( point = descriptor_list; point; point = next_point ) {
			next_point = point->next;

			if ( FD_ISSET (point->hSocketFD, &writefds) && point->output.head ) {
				if ( process_output( point ) < 0 ) {
					FD_CLR( point->hSocketFD, &readfds );
					FD_CLR( point->hSocketFD, &exceptfds );
					FD_CLR( point->hSocketFD, &writefds );
					close_socket( point );
				} else
					point->prompt_mode = 1;
			}
		}

		for ( point = descriptor_list; point; point = next_point ) {

			next_point = point->next;

			if ( !IS_SET (point->edit_mode, MODE_DONE_EDITING) )
				continue;

			if ( point->prompt_mode ) {

				if ( point->edit_index != -1 )
					;

				else if ( point->str )
					write_to_descriptor( point, point->connected == CON_CREATION ? "> " : "] " );

				else if ( point->showstr_point )
					write_to_descriptor( point, "*** Press return to continue - 'q' to quit *** " );

				else if ( !point->connected ) {
					std::string prompt = "";
					if ( point->character->flags & FLAG_NOPROMPT)
					{
						if ( point->character->flags & FLAG_WIZINVIS
						)
							prompt += "##";
					} else {
						prompt += '<';

						if ( IS_NPC (point->character) ) {
							prompt += '=';
						}

						if ( point->character->flags & FLAG_WIZINVIS)
						{
							prompt += "##";
						}

						if ( point->character->flags & FLAG_ANON)
						{
							prompt += "##";
						}

						prompt += wound_total( point->character );

//		      if (point->character->flags & FLAG_HARNESS)
//			{
//			  prompt += " / ";
//			  prompt += mana_bar (point->character);
//			}

						prompt += " / ";
						prompt += fatigue_bar( point->character );


						if ( get_affect( point->character, AFFECT_HOLDING_BREATH ) ) {
							prompt += " / ";
							prompt += breath_bar( point->character );
						}

						if (get_affect (point->character, AFFECT_CHOKING))
                        {
                            prompt += " / ";
                            prompt += breath_bar (point->character );
                        }

						if ( point->character->bleeding_prompt ) {
							prompt += " #1bleeding#0";
						}
					}
					prompt += "> ";
					write_to_descriptor( point, prompt.c_str() );
				}

				point->prompt_mode = 0;
			}
		}

		/* handle heartbeat stuff */
		/* Note: pulse now changes every 1/4 sec  */

		pulse++;

		if ( !( pulse % SECOND_PULSE) ) {
			second_affect_update();
			if ( pending_reboot )
				check_reboot();
		}

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 1 ) % 4) )
			delayed_trigger_activity();

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 3 ) % PULSE_DELAY) )
			update_delays();

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 1 ) % PULSE_SMART_MOBS ) && !engine.in_build_mode() )
			mobile_routines( pulse );

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 2 ) % UPDATE_PULSE) )
			point_update();

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( pulse % ( 10 * SECOND_PULSE )) )
			ten_second_update();

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 5 ) % ( 10 * SECOND_PULSE )) ) {
			check_maintenance();
			//update_website ();
			//vote_notifications ();
		}

		if ( !( pulse % ( PULSES_PER_SEC * 60 )) ) {
			check_idlers();
			check_linkdead();
			check_sitebans();
			process_reviews();
		}

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( !( ( pulse + 5 ) % PULSE_AUTOSAVE * 1) ) {
			autosave();
		}

		if ( !( ( pulse + 5 ) % PULSE_AUTOSAVE * 12 ) && engine.in_play_mode() ) {
			save_stayput_mobiles();
		}

		if ( !( ( pulse + 5 ) % PULSE_AUTOSAVE * 13) ) {
			save_tracks();
		}
		if ( !( ( pulse + 5 ) % PULSE_AUTOSAVE * 14) ) {
			save_banned_sites();
		}
		if ( !( ( pulse + 5 ) % PULSE_AUTOSAVE * 15) ) {
			save_player_rooms();
			save_dwelling_rooms();
		}
		if ( !( pulse % ( 120 * SECOND_PULSE )) ) {
			newbie_hints();
		}

		if ( !( ( pulse + 5 ) % ( PULSE_ZONE * 3 )) )
			refresh_zone();

		if ( !( ( pulse + 9 ) % ( PULSE_AUTOSAVE * 5 )) ) {
			/*if ( !engine.in_build_mode() ) {
			 if ( morgul_arena_fight ) {
			 if ( !number( 0, 19 ) && is_arena_clear() ) {
			 morgul_arena_troll();
			 } else if ( !number( 0, 19 ) && is_arena_clear() ) {
			 morgul_arena_wargs();
			 }
			 }
			 } else*/
			if ( engine.in_build_mode() ) {
				update_crafts_file();
			}

			//update_website_statistics ();
		}

		/*if ( !( pulse % ( SECOND_PULSE * 60 * 15 )) ) //every IG hour
		 {
		 if ( !morgul_arena_fight && engine.in_play_mode()
		 && is_arena_clear() ) {
		 morgul_arena_first();
		 morgul_arena_time = ( int ) time( 0 );
		 arena_matches++;
		 }
		 }*/

		/** TE PIT **
		 * Runs on the 1 and 15 of each month, for 1 day, every half hour RL
		 */
		///\TODO Uh... lets not calc this every pulse, mmkay?
		/*time_t t = time( NULL );
		 struct tm* tp = localtime( &t );
		 int daymonth;

		 daymonth = tp->tm_mday;

		 if ( daymonth == 1 || daymonth == 15 ) {
		 if ( !( pulse % ( SECOND_PULSE * 60 * 30 )) ) //every 30 RL miuntes
		 {
		 if ( !te_pit_fight && engine.in_play_mode()
		 && is_te_pit_clear() ) {
		 te_pit_first();
		 te_pit_time = ( int ) time( 0 );
		 }
		 }
		 }*/
		/* end te minute update pit **/

		if ( !( pulse % ( SECOND_PULSE * 60 * 60 * 4 )) ) {
			if ( !engine.in_build_mode() ) {
				for ( tch = character_list; tch; tch = tch->next ) {
					if ( tch->deleted )
						continue;

					if ( !IS_NPC( tch ) || !IS_SET( tch->flags, FLAG_KEEPER ) )
						continue;

					purse = number( 50, 100 );
					while ( purse > 0 )
						purse -= vnpc_customer( tch, purse );

					refresh_colors( tch );
				}
			}
		}

		if ( time( 0 ) >= next_minute_update ) /* 1 RL minute */
			rl_minute_affect_update();

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		if ( time( 0 ) >= next_hour_update ) {
			hourly_update();
			update_room_tracks();
			weather_and_time( 1 );
			hour_affect_update();
			room_update();
			rent_update();
		}

		if ( knockout ) {
			cleanup_the_dead( 0 );
			knockout = 0;
		}

		/*if ( !( ( pulse + 2 ) % pulse_violence) ) {
		 perform_violence();
		 for ( i = 0; i <= 99; i++ ) {
		 if ( weather_info[ i ].lightning )
		 if ( number( 90, 150 ) < weather_info[ i ].clouds )
		 send_outside(
		 "A fork of lightning flashes in the sky.\n" );
		 }
		 }*/

		if ( !( ( pulse + 1 ) % PULSE_MOBILE) ) {
			cleanup_the_dead( 0 );
		}

		if ( pulse > 86400 ) {
			pulse = 0;
		}

		if ( quarter_event_list )
			process_quarter_events();

		tics++; /* tics since last checkpoint signal */

		if ( first_loop ) {
			for ( tch = character_list; tch; tch = next_ch ) {
				next_ch = tch->next;
				if ( tch->deleted )
					continue;
				trigger( tch, "", TRIG_HOUR );
			}
		}

		first_loop = false;
	}

	return 0;
}

void handle_sec_input( int clntSocket ) {
	FILE *fp;
	char buf[ MAX_STRING_LENGTH ];
	char file[ MAX_STRING_LENGTH ];
	int recvMsgSize;

	sprintf( file, "rpinet" );
	*buf = '\0';

	if ( ( recvMsgSize = recvfrom( clntSocket, buf, MAX_STRING_LENGTH, 0, NULL, 0 ) ) < 0 )
		return;

	buf[ recvMsgSize - 1 ] = '\0';

	if ( !( fp = fopen( file, "w+" )) )
		return;
	fprintf( fp, buf );
	fclose( fp );

	close( clntSocket );
}

void newbie_hints( void ) {
	DESCRIPTOR_DATA *d;
	NEWBIE_HINT *hint;
	char buf[ MAX_STRING_LENGTH ];
	char *p;
	int i, limit, hintnum;
	extern NEWBIE_HINT *hint_list;

	for ( hint = hint_list, limit = 1; hint; hint = hint->next, limit++ )
		;
	limit--;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->character )
			continue;
		if ( IS_NPC (d->character) )
			continue;
		if ( !IS_SET (d->character->plr_flags, NEWBIE_HINTS) )
			continue;
		hintnum = number( 1, limit );
		for ( hint = hint_list, i = 1; hint; hint = hint->next, i++ ) {
			if ( i != hintnum )
				continue;
			reformat_string( hint->hint, &p );
			sprintf( buf, "\r\n#6%s#0", p );
			send_to_char( buf, d->character );
			mem_free( p );
			break;
		}
	}
}

void unban_site( SITE_INFO * site ) {
	SITE_INFO *tmp_site = NULL;
	char buf[ MAX_STRING_LENGTH ];

	if ( !site )
		return;
	if ( !banned_site ) //global value
		return;

	if ( banned_site == site ) {
		sprintf( buf, "The siteban has been lifted on %s.\n", banned_site->name );
		send_to_gods( buf );
		banned_site = site->next;
	} else {
		for ( tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next ) {
			if ( tmp_site->next == site ) {
				sprintf( buf, "The siteban has been lifted on %s.\n", tmp_site->next->name );
				send_to_gods( buf );
				system_log( buf, false );
				tmp_site->next = site->next;
				continue;
			}
		}

		mem_free( site->name );
		mem_free( site->banned_by );
		mem_free( site );
		site = NULL;

	}

	save_banned_sites();
}

void check_sitebans() {
	SITE_INFO *site = NULL;
	SITE_INFO *next_site = NULL;

	if ( !engine.in_play_mode() )
		return;

	if ( !banned_site )
		return;

	if ( banned_site ) {
		if ( banned_site->banned_until != -1 && time( 0 ) >= banned_site->banned_until ) {
			unban_site( banned_site );
		}
	}

	for ( site = banned_site; site; site = next_site ) {
		next_site = site->next;
		if ( site->next ) {
			if ( site->next->banned_until == -1 )
				continue;

			if ( site->next->banned_until != -1 && time( 0 ) >= site->next->banned_until ) {
				if ( site->next->next ) //is there another site to look at
				{
					unban_site( site->next );
					continue;
				} else {
					unban_site( site->next ); //last site
					return;
				}
			}
		}
	}
}

void check_maintenance( void ) {
	FILE *fp = 0;

	if ( ( fp = fopen( ".mass_emailing", "r" )) ) {
		send_to_gods( "...mass-emailing completed.\n" );
		fclose( fp );
		unlink( ".mass_emailing" );
	}

	if ( engine.in_play_mode() ) {
		if ( ( fp = fopen( "maintenance_lock", "r" )) ) {
			if ( !maintenance_lock )
				send_to_all( "#2The server is now locked down for maintenance.#0\n" );
			maintenance_lock = true;
			fclose( fp );
		} else {
			if ( maintenance_lock )
				send_to_all( "#2The server is now open for play.#0\n" );
			maintenance_lock = false;
		}
	}
}

void vote_notifications( void ) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	DESCRIPTOR_DATA *d;

	if ( !engine.in_play_mode() )
		return;

	mysql_safe_query( "SELECT * FROM vote_notifications" );
	if ( ( result = mysql_store_result( database ) ) != NULL)
	{

		while ( ( row = mysql_fetch_row( result ) ) ) {
			for ( d = descriptor_list; d; d = d->next ) {
				if ( !d->strClientIpAddr || !d->acct || !d->character || d->connected != CON_PLYNG )
					continue;
				if ( IS_SET (d->acct->flags, ACCOUNT_NOVOTE) )
					continue;
				if ( !str_cmp( d->strClientIpAddr, row[ 0 ] ) )
					send_to_char( "#6Your vote has been recorded. Thank you for supporting our community!#0\n",
							d->character );
			}
		}
		mysql_free_result( result );
	} else {
		fprintf( stderr, "vote_notifications: %s\n", mysql_error( database ) );
	}
	mysql_safe_query( "DELETE FROM vote_notifications" );
}

void update_website( void ) {
	FILE *ft;
	CHAR_DATA *tch;
	DESCRIPTOR_DATA *d, *d_next;
	int online = 0, j = 0, count = 0;
	char buf[ MAX_STRING_LENGTH ];
	MYSQL_RES *result;
	MYSQL_ROW row;

	int port = engine.get_port();
	mysql_safe_query( "DELETE FROM players_online WHERE port = %d", port );

	for ( d = descriptor_list; d; d = d_next ) {
		j++;
		d_next = d->next;
		if ( !d->character || IS_NPC (d->character)
		)
			continue;
		if ( d->connected != CON_PLYNG )
			continue;
		if ( !d->acct )
			continue;
		if ( IS_MORTAL (d->character) )
			count++;
		mysql_safe_query( "INSERT INTO players_online VALUES ('%s', '%s', '%s', %d, %d)", d->acct->name.c_str(),
				d->character->tname, d->strClientHostname, d->character->in_room, port );
	}

	if ( engine.in_play_mode() ) {
		if ( !( ft = fopen( PATH_TO_WEBSITE "/stats.shtml", "w+" ) ) )
			return;

		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted )
				continue;

			if ( tch->descriptor && IS_MORTAL( tch ) )
				online++;
		}

		sprintf( buf, "%s\n", time_string( NULL ) );

		fputs( buf, ft );

		fclose( ft );

		mysql_safe_query( "SELECT most_online FROM newsletter_stats;" );
		result = mysql_store_result( database );
		if ( !result || !mysql_num_rows( result ) ) {
			if ( result != NULL
			)
				mysql_free_result( result );
			return;
		}

		row = mysql_fetch_row( result );

		if ( atoi( row[ 0 ] ) < count )
			mysql_safe_query( "UPDATE newsletter_stats SET most_online = %d;", count );

		if ( result )
			mysql_free_result( result );

		system( "chmod 644 " PATH_TO_WEBSITE "/stats.shtml" );

		mysql_safe_query( "UPDATE server_statistics SET max_players = %d", count_max_online );

		mysql_safe_query( "UPDATE server_statistics SET morgul_arena = %d", morgul_arena_time );
	}
}

void send_to_room_excluding( const char * message, const CHAR_DATA * excluded ) {
	// Validating parameters
	if ( !message || !excluded )
		return;

	CHAR_DATA * target;
	for ( target = excluded->room->people; target; target = target->next_in_room ) {
		DESCRIPTOR_DATA * desc = target->descriptor;

		if ( !desc && IS_NPC( target ) )
			continue;

		/* Check to see if real PC owner is still online */

		if ( !desc && target->pc && target->pc->owner )
			for ( desc = descriptor_list; desc; desc = desc->next )
				if ( desc == target->pc->owner )
					break;

		if ( !desc )
			continue;

		if ( desc->character && IS_SET( desc->character->act, PLR_QUIET ) )
			continue;

		if ( target == excluded )
			continue;

		write_to_q( message, &desc->output );
	}

}

void send_to_char( const char * message, const CHAR_DATA * target ) {
	// Validating parameters
	if ( !message || !target )
		return;

	DESCRIPTOR_DATA * d = target->descriptor;

	if ( !d && IS_NPC ( target ) )
		return;

	/* Check to see if real PC owner is still online */

	if ( !d && target->pc && target->pc->owner )
		for ( d = descriptor_list; d; d = d->next )
			if ( d == target->pc->owner )
				break;

	if ( !d )
		return;

	if ( d->character && IS_SET (d->character->act, PLR_QUIET)
	)
		return;

	write_to_q( message, &d->output );
}

void send_to_all_unf( char *messg ) {
	DESCRIPTOR_DATA *i;

	if ( messg )
		for ( i = descriptor_list; i; i = i->next )
			if ( !i->connected )
				write_to_q( messg, &i->output );
}

void send_to_all( char *messg ) {
	DESCRIPTOR_DATA * i;
	char *formatted;

	if ( !messg || !*messg )
		return;

	reformat_string( messg, &formatted );

	if ( messg )
		for ( i = descriptor_list; i; i = i->next )
			if ( !i->connected )
				write_to_q( formatted, &i->output );

	mem_free( formatted );
}

void send_to_guides( const char * message ) {
	if ( !message || !*message )
		return;

	char buf[ MAX_STRING_LENGTH ];
	sprintf( buf, "\n#6[Guide]#0 %s", message );

	char * formatted;
	reformat_string( buf, &formatted );

	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d || !d->connected )
			continue;

		if ( !d->character || !IS_GUIDE( d->character ) || IS_SET( d->character->act, PLR_QUIET ) )
			continue;

		write_to_q( formatted, &d->output );
	}

	mem_free( formatted );
}

void send_to_gods( const char * message ) {
	if ( !message || !*message )
		return;

	char buf[ MAX_STRING_LENGTH ];
	sprintf( buf, "\n#2[System Message]#0 %s", message );

	char * formatted;
	reformat_string( buf, &formatted );

	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d || !d->connected )
			continue;

		if ( !d->character || IS_MORTAL( d->character ) || IS_SET( d->character->act, PLR_QUIET ) )
			continue;

		write_to_q( formatted, &d->output );
	}

	mem_free( formatted );
}

void send_to_imms( char *message ) {
	DESCRIPTOR_DATA *d;
	char buf[ MAX_STRING_LENGTH ];
	char *formatted;

	if ( !message || !*message )
		return;

	sprintf( buf, "\n%s", message );

	reformat_string( buf, &formatted );

	for ( d = descriptor_list; d; d = d->next )
		if ( !d->connected && !IS_MORTAL (d->character) && !IS_SET (d->character->act, PLR_QUIET)
		)
			write_to_q( formatted, &d->output );
	mem_free( formatted );
}

void send_to_guardians( char *message, unsigned short int flag ) {
	DESCRIPTOR_DATA *d;
	char buf[ MAX_STRING_LENGTH ];
	char *formatted;

	if ( !message || !*message )
		return;

	sprintf( buf, "\n%s", message );

	reformat_string( buf, &formatted );

	system_log( message, false );
	for ( d = descriptor_list; d; d = d->next ) {

		if ( !d->connected && !IS_MORTAL (d->character) && !IS_SET (d->character->act, PLR_QUIET)
				&& ( flag & d->character->guardian_mode ) ) {
			write_to_q( formatted, &d->output );
		}

	}
	mem_free( formatted );
}

void send_outside( char *message ) {
	DESCRIPTOR_DATA *d;
	char *formatted;

	if ( !message || !*message )
		return;

	reformat_string( message, &formatted );

	for ( d = descriptor_list; d; d = d->next ) {

		if ( d->connected || !d->character || !d->character->room )
			continue;

		if ( IS_OUTSIDE (d->character) && !IS_SET (d->character->act, PLR_QUIET) && AWAKE (d->character)
		)
			write_to_q( formatted, &d->output );
	}
	mem_free( formatted );
}

void send_outside_zone( char *message, int zone ) {
	DESCRIPTOR_DATA *d;
	char *formatted;

	if ( !message || !*message )
		return;

	if ( zone == 1 ) {
		send_outside_zone( message, 3 );
		send_outside_zone( message, 8 );
		send_outside_zone( message, 11 );
	}

	if ( zone == 10 ) {
		send_outside_zone( message, 70 );
		send_outside_zone( message, 71 );
		send_outside_zone( message, 72 );
		send_outside_zone( message, 73 );
		send_outside_zone( message, 74 );
		send_outside_zone( message, 75 );
		send_outside_zone( message, 76 );
		send_outside_zone( message, 77 );
		send_outside_zone( message, 78 );
		send_outside_zone( message, 79 );
	}

	if ( zone == 64 ) {
		send_outside_zone( message, 65 );
		send_outside_zone( message, 66 );
	}

	if ( zone == 2 ) {
		send_outside_zone( message, 15 );
	}

	reformat_string( message, &formatted );

	for ( d = descriptor_list; d; d = d->next ) {

		if ( d->connected || !d->character || !d->character->room )
			continue;

		if ( d->character->room->zone != zone )
			continue;

		if ( IS_OUTSIDE (d->character) && !IS_SET (d->character->act, PLR_QUIET) && AWAKE (d->character)
		)
			write_to_q( formatted, &d->output );
	}

	mem_free( formatted );
}

void send_to_room( char * message, int room_vnum ) {
	// Validating message parameter
	if ( !message || !*message )
		return;

	// Resolving the room data form the vnum
	ROOM_DATA * room;
	if ( !( room = vnum_to_room( room_vnum ) ) )
		return;

	// Reformatting the string
	char * formatted;
	reformat_string( message, &formatted );

	CHAR_DATA * target;
	for ( target = room->people; target; target = target->next_in_room ) {
		if ( !target->descriptor || !IS_SET( target->act, PLR_QUIET ) )
			continue;

		write_to_q( formatted, &target->descriptor->output );
	}

	mem_free( formatted );
}

void send_to_room_unformatted( char * message, int room_vnum ) {
	// Validating message parameter
	if ( !message || !*message )
		return;

	// Resolving room data
	ROOM_DATA * room;
	if ( !( room = vnum_to_room( room_vnum )) )
		return;

	CHAR_DATA * tch;
	for ( tch = room->people; tch; tch = tch->next_in_room ) {
		if ( !tch->descriptor || IS_SET( tch->act, PLR_QUIET ) )
			continue;

		write_to_q( message, &tch->descriptor->output );
	}
}

char *
highlight( char *source ) {
	std::string strArgument = source, buf, arg;
	char *result;
	int i;

	buf.assign( "#F" );

	for ( i = 0; i <= strArgument.length(); i++ ) {
		arg = strArgument[ i ];

		if ( arg == "#" ) {
			buf.append( arg );
			i += 1;
			arg = strArgument[ i ];

			if ( arg == "0" )
				arg = "F";
			if ( arg == "1" )
				arg = "9";
			if ( arg == "2" )
				arg = "A";
			if ( arg == "3" )
				arg = "B";
			if ( arg == "4" )
				arg = "C";
			if ( arg == "5" )
				arg = "D";
			if ( arg == "6" )
				arg = "E";
			if ( arg == "7" )
				arg = "F";
		}

		buf.append( arg );
	}

	sprintf( result, "%s#0", buf.c_str() );

	return result;
}

/* higher-level communication */

//////////////////////////////////////////////////////////////////////////////
// act()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Show an action relative to the actors and watchers.
///
/// \param[in]  action_message  A specially formatted action string.
/// \param[in]  hide_invisible  When set do not show the action to those who
///                             cannot see the actor.
/// \param[in]  ch              The primary actor.
/// \param[in]  obj             A target object or character.
/// \param[in]  vict_obj        An additional target object or character
/// \param[in]  type            Bitvector defines who is to see this action.
//
//////////////////////////////////////////////////////////////////////////////
void act( char *action_message, int hide_invisible, CHAR_DATA * ch, OBJ_DATA * obj, void *vict_obj, int type ) {

	/// \par Local Variables:

	/// \li \e  strp  - This is an index into the \c action_message parameter.
	/// \li \e  point  - This is an index into the \c buf local.
	/// \li \e  i  - text to be inserted in place of format tags.
	/// \li \e  p  - A pointer into the \c buf local during the reformat phase.
	/// \li \e  buf  - The output buffer.
	/// \li \e  immbuf1  - A buffer holding the actor's real name.
	/// \li \e  immbuf2  - A buffer holding the victim's real name.
	/// \li \e  chsex  - A temporary backup of a PC's sex if they are hooded.
	/// \li \e  color  - A reference to the ANSI color to use for output part.
	/// \li \e  do_cap  - Determines proper capitalization (start of line, e.g).
	/// \li \e  dietyCap  - Capitalize the actor's god name (wtf?).
	/// \li \e  totemCap  - Capitalize the actor's totem name (again: w.t.f.?).
	/// \li \e  to  - A pointer to the recipient of the action message.
	/// \li \e  tch  - A pointer to an included secondary actor.

	char *strp; // Current index of action_message
	char *point; // Current index of local buffer
	const char *i = '\0'; //
	char *p;
	char buf[ MAX_STRING_LENGTH ];
	char immbuf1[ MAX_STRING_LENGTH ];
	char immbuf2[ MAX_STRING_LENGTH ];
	int chsex;
	int color = 0;
	int do_cap = 0;
	int dietyCap = 0;
	int totemCap = 0;
	CHAR_DATA *pt_recipient;
	CHAR_DATA *pt_other_recipient;

	/// \par Implementation:

	/// <ul>

	/// <li>
	///       First we test the parameter integrity as a precaution. If we have
	///       bad parameters we return immediately.
	/// </li>

	// Test parameter integrity
	if ( !action_message || !*action_message || !ch )
		return;

	/// <li>
	///       We return immediately if the N/PC is in Combat "Compete" Mode.
	///       \see do_compete()
	/// </li>

	// Ensure the N/PC is not in COMPETE mode during combat.
	if ( IS_SET( ch->flags, FLAG_COMPETE ) )
		return;

	/// <li>
	///       Examine \c type to determine who receives this action message.
	/// </li>

	if ( IS_SET( type, TO_VICT ) ) {
		pt_other_recipient = ( CHAR_DATA * ) vict_obj;
		pt_recipient = pt_other_recipient;
	} else if ( IS_SET (type, TO_CHAR) ) {
		pt_recipient = ch;
	} else if ( !ch->room ) {
		return;
	} else {
		pt_recipient = ch->room->people;
	}

	/// <li>
	///       Iterate through persons in the recipient's room sending the
	///       action message where appropriate.
	/// <ul>

	for ( ; pt_recipient; pt_recipient = pt_recipient->next_in_room ) {

		/// <li>
		///       We do not translate and send the message:
		/// <ul>
		/// <li>    \e IF the recipient is not the actor </li>
		/// <li>    \e AND the actor is immortal </li>
		/// <li>    \e AND the recipient is mortal </li>
		/// <li>    \e AND the actor is invisible. </li>
		//          \e AND the recipient is not a NPC
		/// </ul>
		/// </li>

		if ( ch != pt_recipient && !IS_MORTAL (ch) && IS_MORTAL (pt_recipient) && GET_FLAG (ch, FLAG_WIZINVIS)
				&& !IS_NPC(pt_recipient))
				{
			continue;
		}

		/// <li>
		///       We translate and send the message:
		/// <ul>
		/// <li>    \e IF the recipient is connected (i.e. a link-live PC or an
		///            animated NPC) </li>
		/// <li>    \e AND the recipient is not the actor (\e OR this message
		///            type is to the actor) </li>
		/// <li>    \e AND the recipient is in the actor's group (\e OR this
		///            message type is not group-only) </li>
		/// <li>    \e AND the recipient can see the actor (\e OR we do not want
		///            to hide invisible actors </li>
		/// <li>    \e AND the recipient is awake </li>
		/// <li>    \e AND the type is not immortal-only (\e OR the recipient is
		///            on staff and the actor is not (?)) </li>
		/// <li>    \e AND \e NOT a non-victim message \e IF the recipient is
		///            the same as the victim. </li>
		/// </ul>
		/// </li>

		if ( ( pt_recipient != ch || IS_SET (type, TO_CHAR) )
				&& ( are_grouped( pt_recipient, ch ) || !IS_SET (type, TO_GROUP) )
				&& ( CAN_SEE (pt_recipient, ch) || !hide_invisible ) && AWAKE (pt_recipient)
				&& ( !IS_SET (type, TO_IMMS) || ( GET_TRUST (pt_recipient) && !GET_TRUST (ch) ) )
				&& !( IS_SET (type, TO_NOTVICT) && pt_recipient == ( CHAR_DATA * ) vict_obj ) ) {

			/// <li>  Iterate through the action_message and output buffer
			///       character-at-a-time.
			/// <ul>

			for ( strp = action_message, point = buf;; ) {

				/// <li>
				///       We parse the next character as a format identifier
				///       \e IF this character is \c $. \e ELSE we copy this
				///       character from the message to the buffer (stopping
				///       when we reach the end of the string.
				/// </li>

				/// <li>  Format translations:
				/// <ul>

				if ( *strp == '$' ) {

					switch ( *( ++strp ) ) {

						/// <li>
						///       \c $n - insert the short description of actor
						///               (showing "someone" if the recipient
						///               cannot see, and storing the actor's
						///               name for immortals.
						/// </li>

						case 'n':
							i = PERS (ch, pt_recipient);
							if ( GET_TRUST (pt_recipient) && is_hooded( ch ) ) {
								strcpy( immbuf1, i );
								sprintf( immbuf1, "%s (%s)", i, ch->tname );
								i = immbuf1;
							}
							color = 5;
							break;

							/// <li>
							///       \c $N - insert the short description of the
							///               victim (showing "someone" if the
							///               recipient cannot see, and storing the
							///               victim's name for immortals to see.
							/// </li>

						case 'N':
							pt_other_recipient = ( CHAR_DATA * ) vict_obj;
							i = PERS (pt_other_recipient, pt_recipient);
							if ( GET_TRUST (pt_recipient) && is_hooded( pt_other_recipient ) ) {
								strcpy( immbuf2, i );
								sprintf( immbuf2, "%s (%s)", i, pt_other_recipient->tname );
								i = immbuf2;
							}
							color = 5;
							break;

							/// <li>
							///       \c $3 - insert the short description of third
							///               PC (?).
							/// </li>

						case '3':
							i = PERS ((CHAR_DATA *) obj, pt_recipient), color = 5;
							break;

							/// <li>
							///       \c $m - actor as "him", "her", or "it" (the
							///               latter in the case of neuter or hooded
							///               actors).
							/// </li>

						case 'm':
							if ( is_hooded( ch ) ) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HMHR (ch);
								ch->sex = chsex;
							} else
								i = HMHR (ch);
							color = 0;
							break;

							/// <li>
							///       \c $M - victim as "him", "her", or "it" (the
							///               latter in the case of neuter or hooded
							///               victims).
							/// </li>

						case 'M':
							pt_other_recipient = ( CHAR_DATA * ) vict_obj;
							if ( is_hooded( pt_other_recipient ) ) {
								chsex = pt_other_recipient->sex;
								pt_other_recipient->sex = 0;
								i = HMHR (pt_other_recipient);
								pt_other_recipient->sex = chsex;
							} else
								i = HMHR (pt_other_recipient);
							color = 0;
							break;

							/// <li>
							///       \c $s - actor as "his", "her", or "its" (the
							///               latter in the case of neuter or hooded
							///               actors).
							/// </li>

						case 's':
							if ( is_hooded( ch ) ) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HSHR (ch);
								ch->sex = chsex;
							} else
								i = HSHR (ch);
							color = 0;
							break;

							/// <li>
							///       \c $S - victim as "his", "her", or "its" (the
							///               latter in the case of neuter or hooded
							///               victims).
							/// </li>

						case 'S':
							pt_other_recipient = ( CHAR_DATA * ) vict_obj;
							if ( is_hooded( pt_other_recipient ) ) {
								chsex = pt_other_recipient->sex;
								pt_other_recipient->sex = 0;
								i = HSHR (pt_other_recipient);
								pt_other_recipient->sex = chsex;
							} else
								i = HSHR (pt_other_recipient);
							color = 0;
							break;

							/// <li>
							///       \c $e - actor as "he", "she", or "it" (the
							///               latter in the case of neuter or hooded
							///               actors).
							/// </li>

						case 'e':
							if ( is_hooded( ch ) ) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HSSH (ch);
								ch->sex = chsex;
							} else
								i = HSSH (ch);
							color = 0;
							break;

							/// <li>
							///       \c $E - victim as "he", "she", or "it" (the
							///               latter in the case of neuter or hooded
							///               victims).
							/// </li>

						case 'E':
							pt_other_recipient = ( CHAR_DATA * ) vict_obj;
							if ( is_hooded( pt_other_recipient ) ) {
								chsex = pt_other_recipient->sex;
								pt_other_recipient->sex = 0;
								i = HSSH (pt_other_recipient);
								pt_other_recipient->sex = chsex;
							} else
								i = HSSH (pt_other_recipient);
							color = 0;
							break;

							/// <li>
							///       \c $o - object name or "something" if hidden
							///               from the recipient
							/// </li>

						case 'o':
							i = OBJN (obj, pt_recipient);
							color = 2;
							break;

							/// <li>
							///       \c $O - victim object name or "something" if
							///         hidden from the recipient
							/// </li>

						case 'O':
							i = OBJN ((OBJ_DATA *) vict_obj, pt_recipient);
							color = 2;
							break;

							/// <li>
							///       \c $p - object short description or "something"
							///               if hidden from the recipient
							/// </li>

						case 'p':
							i = OBJS (obj, pt_recipient);
							color = 2;
							break;

							/// <li>
							///       \c $P - victim object short description or
							///              "something" if hidden from the recipient
							/// </li>

						case 'P':
							i = OBJS ((OBJ_DATA *) vict_obj, pt_recipient);
							color = 2;
							break;

							/// <li>
							///       \c $a - "a" or "an" object (based on name)
							/// </li>

						case 'a':
							i = SANA (obj);
							break;

							/// <li>
							///       \c $A - "a" or "an" victim object (based on
							///               name)
							/// </li>

						case 'A':
							i = SANA ((OBJ_DATA *) vict_obj);
							break;

							/// <li>
							///       \c $T - Cast parameter \c vict_obj as a string
							///               and set to local \c i (?).
							/// </li>

						case 'T':
							i = ( char * ) vict_obj;
							break;

							/// <li>
							///       \c $F - Cast parameter \c vict_obj as a string
							///               of names and set to local \c i to the
							///               first of them (?).
							/// </li>

						case 'F':
							i = fname( ( char * ) vict_obj );
							break;

							/// <li>
							///       \c $g - insert the name of the actor's god (?)
							/// </li>

						case 'g':
							i = deity_name[ ch->deity ];
							color = 4;
							/* Make sure name is capitalized */
							dietyCap = 1;
							break;

							/// <li>
							///       \c $G  - insert the name of the victim's god
							///                (?).
							/// </li>

						case 'G':
							i = deity_name[ ( ( CHAR_DATA * ) ( vict_obj ) )->deity ];
							color = 4;
							break;

							/// <li>
							///       \c $$  - Insert a dollar-sign
							/// </li>

						case '$':
							i = "$";
							break;
						default:
							break;

							/// </ul></li>
					}

					/// <li>  Do not copy local \c i to \c buf \e IF there is
					///       nothing to insert (\e OR we are at the end of the
					///       buffer), \e ELSE Capitalize, color, and copy.</li>

					if ( !i || !point ) {
						;
					} else {
						if ( point == buf || dietyCap || totemCap )
							do_cap = 1;
						else
							do_cap = 2;

						if ( color != 0 ) {
							*point++ = '#';
							*point++ = '0' + color;
						}

						if ( *i ) {

							*point = *( i++ );

							if ( do_cap == 1 )
								point[ 0 ] = toupper( point[ 0 ] );
							else if ( do_cap == 2 )
								point[ 0 ] = tolower( point[ 0 ] );

							point++;
							do_cap = 0;
						}
						while ( ( ( *point ) = ( *( i++ ) ) ) )
							++point;

						if ( color != 0 ) {
							*point++ = '#';
							*point++ = '0';
						}
					}
					++strp;
				} else if ( !( *( point++ ) = *( strp++ )) ) {
					break;
				}

				/// </li></ul>

			}

			/// </li>
			/// <li>  Cap the end of the output buffer with a newline & null.
			/// </li>

			*( --point ) = '\r';
			*( ++point ) = '\n';
			*( ++point ) = '\0';

			/// <li>  We do not send the buffer:
			/// <ul>
			/// <li>  \e IF this is a combat message </li>
			/// <li>  \e AND the recipient filters combat messages </li>
			/// <li>  \e AND the recipient is not fighting  the victim</li>
			/// <li>  \e AND the recipient is not fighting the actor</li>
			/// <li>  \e AND the victim is not fighting the recipient</li>
			/// <li>  \e AND the actor is not fighting the recipient</li>
			/// </ul>

			if ( IS_SET (type, _ACT_COMBAT) && IS_SET (pt_recipient->plr_flags, COMBAT_FILTER)
					&& pt_recipient->fighting != vict_obj && pt_recipient->fighting != ch ) {
				if ( vict_obj && ( ( CHAR_DATA * ) vict_obj )->fighting == pt_recipient )
					;
				else if ( ch->fighting == pt_recipient )
					;
				else
					continue;
			}

			/// <li>  We do not send the buffer:
			/// <ul>
			/// <li>  \e IF this is a bleeding message </li>
			/// <li>  \e AND the recipient filters combat messages </li>
			/// <li>  \e AND the recipient is not fighting  the victim</li>
			/// <li>  \e AND the recipient is not fighting the actor</li>
			/// <li>  \e AND the victim is not fighting the recipient</li>
			/// <li>  \e AND the actor is not fighting the recipient</li>
			/// <li>  \e AND the recipient is fighting</li>
			/// </ul>

			if ( IS_SET (type, _ACT_BLEED) && IS_SET (pt_recipient->plr_flags, COMBAT_FILTER) && pt_recipient->fighting
					&& pt_recipient->fighting != vict_obj && pt_recipient->fighting != ch ) {
				if ( vict_obj && ( ( CHAR_DATA * ) vict_obj )->fighting == pt_recipient )
					;
				else if ( ch->fighting == pt_recipient )
					;
				else
					continue;
			}

			/// <li>  We do not send the buffer:
			/// <ul>
			/// <li>  \e IF this is a search/scan message </li>
			/// <li>  \e AND the recipient filters combat messages </li>
			/// <li>  \e AND the recipient is grouped or admin/li>
			/// </ul>
			if ( IS_SET (type, _ACT_SEARCH) && IS_SET (pt_recipient->plr_flags, COMBAT_FILTER)
					&& ( are_grouped( ch, pt_recipient ) || GET_TRUST (pt_recipient) ) ) {
				continue;
			}

			if ( IS_SET (type, _HIGHLIGHT) && IS_SET (pt_recipient->plr_flags, HIGHLIGHT))
			{
				highlight( buf );
			}

			/// <li>  Reformat the buffer \e IF the \c type parameter is set
			///       accordingly </li>
			/// <li>  Send the buffer to the recipient </li>
			if ( IS_SET (type, _ACT_FORMAT) ) {
				reformat_string( buf, &p );
				if ( *p != '\r' && *p != '\n' && IS_SET (type, TO_VICT) && pt_recipient == vict_obj )
					send_to_char( "\r\n", pt_recipient );
				send_to_char( p, pt_recipient );
				mem_free( p );
			} else
				send_to_char( buf, pt_recipient );
		}

		/// <li>  \e IF the action \c type is to a single recipient, we can
		///       stop iterating. \e ELSE process the next recipient.

		if ( IS_SET (type, TO_VICT) || IS_SET (type, TO_CHAR)
		)
			return;

	}
	/// </ul>
	/// <li>  Return to the caller after the message has been sent to any
	///       recipients </li>
	/// </ul>

	// TODO LIST
	/// \todo  Consider enumerating the type of the \c type parameter.
	/// \todo  Consider swapping out \c MAX_STRING_LENGTH for something more
	///        reasonable.
	/// \todo  Ensure \c immbuf1 and \c immbuf2 are redundant and merge them.
	/// \todo  Consider using an #ifndef block and a command-line option to decide
	///        if we want strong argument checking compiled into the server, and
	///        available as a command-line option.
	/// \todo  Evaluate critcalness of bad parameters and handle with a warning
	///        or error as appropriate.
	/// \todo  Remove "sex change" sequences when getting the pronoun of a hooded
	///        actor (ideally the \c ch parameter would be a const).
	/// \todo  Consider making \c color an enumerated type.
	/// \todo  Detect multi-sentence actions and utilize \c do_cap appropriately.
	/// \todo  Consider giving immortals the option to see names only (no sdesc).
	/// \todo  Remove \c dietyCap and \c totemCap as they really don't belong.
	///        The local \c totemCap is never set.
}

extern int bytes_allocated;
extern int first_free;
extern int mud_memory;

void do_gstat( CHAR_DATA * ch, char *argument, int cmd ) {
	if ( argument && *argument ) {
		if ( strcmp( argument, "config" ) == STR_MATCH)
		{
			if ( GET_TRUST (ch) > 4 ) {
				send_to_char( ( engine.get_config() ).c_str(), ch );
			} else {
				send_to_char( "Operation not permitted.\n", ch );
			}
			return;
		}
	}

	char buf[ MAX_STRING_LENGTH ];
	int uc = 0;
	int con = 0;
	int days;
	int hours;
	int minutes;
	int crashdays, crashminutes, crashhours;
	long uptime, ctime;
	char *tmstr;
	DESCRIPTOR_DATA *e;

	for ( e = descriptor_list; e; e = e->next )
		if ( !e->connected || e->acct )
			uc++;
		else
			con++;

	tmstr = ( char * ) asctime( localtime( &starttime ) );
	*( tmstr + strlen( tmstr ) - 1 ) = '\0';

	uptime = time( 0 ) - starttime;
	ctime = time( 0 ) - crashtime;
	days = uptime / 86400;
	hours = ( uptime / 3600 ) % 24;
	minutes = ( uptime / 60 ) % 60;
	crashdays = ctime / 86400;
	crashhours = ( ctime / 3600 ) % 24;
	crashminutes = ( ctime / 60 ) % 60;

	send_to_char( "\n#6Current Game Statistics#0\n", ch );
	send_to_char( "#6-----------------------#0\n", ch );

	// Collects the latest Subversion Revision
#ifdef SVN_REV
	sprintf (buf, "#2Subversion Revision:            #0%s\n", SVN_REV);
	send_to_char (buf, ch);
	std::string svn_url = "$HeadURL: http://vilya.middle-earth.us/local/rpi_engine/trunk/pp/src/comm.cpp $";
	svn_url.erase (0,38);
	svn_url.erase (svn_url.rfind ("/pp/src"));
	sprintf (buf, "#2Subversion URL:                 #0%s\n", svn_url.c_str ());
	send_to_char (buf, ch);
#endif
	sprintf( buf, "#2Connected Descriptors:          #0%d\n", uc );
	send_to_char( buf, ch );
	sprintf( buf, "#2Connections Since Boot:         #0%d\n", nTotalConnections );
	send_to_char( buf, ch );
	sprintf( buf, "#2Guest Logins Since Boot:        #0%d\n", guest_conns );
	send_to_char( buf, ch );
	sprintf( buf, "#2New Accounts Since Boot:        #0%d\n", new_accounts );
	send_to_char( buf, ch );
	sprintf( buf, "#2Arena Matches Since Boot:       #0%d\n", arena_matches );
	send_to_char( buf, ch );
	sprintf( buf, "#2Auctions Created Since Boot:    #0%d\n", new_auctions );
	send_to_char( buf, ch );
	sprintf( buf, "#2Bids Placed Since Boot:         #0%d\n", auction_bids );
	send_to_char( buf, ch );
	sprintf( buf, "#2Auctions Sold Since Boot:       #0%d\n", sold_auctions );
	send_to_char( buf, ch );
	sprintf( buf, "#2Mother Descriptor:              #0%d\n", nMainSocket );
	send_to_char( buf, ch );
	sprintf( buf, "#2Descriptors Pending:            #0%d\n", con );
	send_to_char( buf, ch );
	sprintf( buf, "#2Free Descriptors:               #0%d\n", avail_descs - maxdesc );
	send_to_char( buf, ch );
	sprintf( buf, "#2Shell Process ID:               #0%d\n", getpid() );
	send_to_char( buf, ch );
	sprintf( buf, "#2MySQL Database Host:            #0%s\n", mysql_get_host_info( database ) );
	send_to_char( buf, ch );
	sprintf( buf, "#2MySQL Server Version:           #0%s\n", mysql_get_server_info( database ) );
	send_to_char( buf, ch );

	// Display the database set we are using
	sprintf( buf, "#2MySQL Database Set: engine      #0%s\n"
			"                    #2world       #0%s\n"
			"                    #2world_log   #0%s\n"
			"                    #2player      #0%s\n"
			"                    #2player_log  #0%s\n", ( engine.get_config( "engine_db" ) ).c_str(),
			( engine.get_config( "world_db" ) ).c_str(), ( engine.get_config( "world_log_db" ) ).c_str(),
			( engine.get_config( "player_db" ) ).c_str(), ( engine.get_config( "player_log_db" ) ).c_str() );
	send_to_char( buf, ch );

	sprintf( buf, "#2Running on Port:                #0%d\n", engine.get_port() );
	send_to_char( buf, ch );
	sprintf( buf, "#2Last Reboot By:                 #0%s", ( ( BOOT[ 0 ] ) ? BOOT : "Startup Script\n") );
	send_to_char( buf, ch );
	sprintf( buf, "#2Last Reboot Time:               #0%s\n", tmstr );
	send_to_char( buf, ch );
	sprintf( buf, "#2Time Spent on Last Boot:        #0%ld seconds\n", starttime - bootstart );
	send_to_char( buf, ch );
	sprintf( buf, "#2Current Uptime:                 #0%d day%s %d hour%s %d minute%s\n", days,
			( ( days == 1) ? "" : "s" ), hours, ((hours == 1) ? "" : "s"),
			minutes, ((minutes == 1) ? "" : "s"));
	send_to_char( buf, ch );
	if ( crashtime ) {
		sprintf( buf, "#2Last Crashed:                   #0%d day%s %d hour%s %d minute%s ago\n", crashdays,
				( ( crashdays == 1) ? "" : "s" ), crashhours,
				((crashhours == 1) ? "" : "s"), crashminutes,
				((crashminutes == 1) ? "" : "s"));
				send_to_char (buf, ch);
			}
	sprintf( buf, "#2Next PC Coldload:               #0%d\n", next_pc_coldload_id );
	send_to_char( buf, ch );
	sprintf( buf, "#2Next Mobile Coldload:           #0%d\n", next_mob_coldload_id );
	send_to_char( buf, ch );
	send_to_char( "#6-----------------------#0\n", ch );
}

// Translates substrings in the form #n to ANSI Color
// For Reference: #define ANSI_AUTO_DETECT	"\x1B[6n"
char *
colorize( const char *source, char *target, struct descriptor_data *d ) {
	const char * colors[] = { "\x1B[0m", // #0 color reset
			"\x1B[31m", // #1 red
			"\x1B[32m", // #2 green
			"\x1B[33m", // #3 yellow
			"\x1B[34m", // #4 blue
			"\x1B[35m", // #5 magenta
			"\x1B[36m", // #6 cyan
			"\x1B[37m", // #7 white
			"\x1B[0m", // #8 color reset
			"\x1B[1;31m", // #9 bold, red
			"\x1B[1;32m", // #A bold, green
			"\x1B[1;33m", // #B bold, yellow
			"\x1B[1;34m", // #C bold, blue
			"\x1B[1;35m", // #D bold, magenta
			"\x1B[1;36m", // #E bold, cyan
			"\x1B[1;37m" // #F bold, white
			};

	char *retval = target;
	bool is_color_link = ( d && ( ( d->character && d->character->color ) || ( d->color ) ));

	*target = '\0';
	while ( *source != '\0' ) {
		if ( *source == '#' ) {
			++source;
			int escaped = *source;

			if ( isxdigit( escaped ) ) {
				// Reduce escaped hex code to its integer equivalent
				if ( escaped <= '9' ) {
					escaped -= '0';
				} else {
					if ( escaped <= 'F' ) {
						escaped -= 'A';
					} else {
						escaped -= 'a';
					}
					escaped += 10;
				}

				if ( is_color_link ) {
					strcpy( target, colors[ escaped ] );
					target = &target[ strlen( target ) ];
				}

				source++;
			} else if ( escaped == '#' ) // print #
					{
				*target++ = *source++;
			} else // no substitution
			{
				*target++ = '#';
				*target++ = *source++;
			}
		} else
			*target++ = *source++;
	}

	*target = '\0';
	return retval;
}

void sigusr1( int signo ) {
	system_log( "SIGUSR1 received: running cleanup_the_dead().", false );
	cleanup_the_dead( 0 );
}

void signal_setup( void ) {
	// No nightly reboots on the builder port.
	if ( engine.in_play_mode() )
		signal( SIGUSR2, shutdown_request );

	signal( SIGUSR1, sigusr1 );
	signal( SIGPIPE, SIG_IGN );
	signal( SIGALRM, logsig );
	signal( SIGSEGV, sigsegv );
	signal( SIGCHLD, sigchld );

}

void reset_itimer() {
	struct itimerval itimer;
	itimer.it_interval.tv_usec = 0; /* miliseconds */
	itimer.it_interval.tv_sec = rpie::server::ALARM_FREQUENCY;
	itimer.it_value.tv_usec = 0;
	itimer.it_value.tv_sec = rpie::server::ALARM_FREQUENCY;

	if ( setitimer( ITIMER_VIRTUAL, &itimer, NULL ) < 0 ) {
		perror( "reset_itimer:setitimer" );
		abort();
	}
}

const char *szFrozenMessage = "Alarm_handler: Not checkpointed recently, aborting!\n";

void alarm_handler( int signo ) {
	if ( engine.loop_detect() ) {
		send_to_gods( "Infinite loop detected - attempting recovery via reboot..." );
		system_log( "Loop detected - attempting copyover to recover.", true );
		shutdown_request( SIGUSR2 );
	}
}

void init_alarm_handler() {
	struct sigaction sa;

	sa.sa_handler = alarm_handler;
	sa.sa_flags = SA_RESTART; /* Restart interrupted system calls */
	sigemptyset( &sa.sa_mask );

	if ( sigaction( SIGVTALRM, &sa, NULL ) < 0 ) {
		perror( "init_alarm_handler:sigaction" );
		abort();
	}
	engine.set_abort_threshold_pre_booting();
	reset_itimer(); /* start timer */
}

void checkpointing( int signo ) {
	extern int tics;

	system_log( "Checkpointing...", false );

	if ( signo == SIGVTALRM
	)
		signal( SIGVTALRM, checkpointing ); /* Guess we have to rearm */

	if ( !tics ) {
		system_log( "Checkpoint shutdown - tics not updated!", true );
		sigsegv( SIGSEGV );
	} else
		tics = 0;
}

void save_world_state( void ) {
	ROOM_DATA *room;
	int i;

	for ( room = full_room_list; room; room = room->lnext ) {
		for ( i = 0; i <= LAST_DIR; i++ ) {
			if ( !room->dir_option[ i ] )
				continue;
			if ( !IS_SET (room->dir_option[i]->exit_info, EX_ISDOOR)
					&& !IS_SET (room->dir_option[i]->exit_info, EX_ISGATE)
					)
				continue;

			/**
			 0 - unlocked_and_open,
			 1 - unlocked_and_closed,
			 2 -locked_and_closed
			 3 - gate_unlocked_and_open,
			 4 - gate_unlocked_and_closed,
			 5 - gate_locked_and_closed
			 **/

			if ( IS_SET (room->dir_option[i]->exit_info, EX_CLOSED)
					&& IS_SET (room->dir_option[i]->exit_info, EX_LOCKED)
					&& IS_SET (room->dir_option[i]->exit_info, EX_ISGATE)
					)
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 5)", room->vnum, i );

			else if ( IS_SET (room->dir_option[i]->exit_info, EX_CLOSED)
					&& IS_SET (room->dir_option[i]->exit_info, EX_ISGATE)
					)
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 4)", room->vnum, i );

			else if ( IS_SET (room->dir_option[i]->exit_info, EX_ISGATE) )
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 3)", room->vnum, i );

			else if ( IS_SET (room->dir_option[i]->exit_info, EX_CLOSED)
					&& IS_SET (room->dir_option[i]->exit_info, EX_LOCKED)
					)
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 2)", room->vnum, i );
			else if ( IS_SET (room->dir_option[i]->exit_info, EX_CLOSED) )
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 1)", room->vnum, i );
			else
				mysql_safe_query( "INSERT INTO copyover_doors VALUES (%d, %d, 0)", room->vnum, i );
		}
	}
}

void prepare_copyover( int cmd ) {
	DESCRIPTOR_DATA *d, *d_next;
	AFFECTED_TYPE *af;
	FILE *fp;
	OBJ_DATA *obj;
	char buf[ MAX_STRING_LENGTH ];
	char buf2[ MAX_STRING_LENGTH ];
	bool obj_recorded = false;
	int count;

	system_log( "Entering prepare_copyover() to set up the reboot.", false );

	fp = fopen( ".copyover_data", "w" );

	mysql_safe_query( "DELETE FROM players_online" );

	// Prepare the soft reboot file for use after server bootup.

	for ( d = descriptor_list; d; d = d_next ) {
		d_next = d->next;

		if ( !d->acct || !d->character || d->connected != CON_PLYNG
				|| ( d->character && d->character->in_room == NOWHERE ) ) {
			if ( d->character ) {
				if ( d->connected == CON_PLYNG )
					unload_pc( d->character );
				d->character = NULL;
			}
			write_to_descriptor( d, "\n\n#6We are rebooting the server - come back in a moment!#0\n\r\n" );
			close_socket( d );
			continue;
		}
		if ( d->character && d->original ) {
			mysql_safe_query( "INSERT INTO copyover_animations VALUES (%d, %d)", d->original->coldload_id,
					d->character->coldload_id );
			do_return( d->character, "", 0 );
		}
		if ( d->character ) {
			if ( cmd == 1 )
				write_to_descriptor( d, "\n#1The server is recovering from a crash. Please wait...#0\n\n" );
			else
				write_to_descriptor( d, "\n#2The server is rebooting. Please wait...#0\n\n" );
		}
		if ( d->character && d->connected == CON_PLYNG && d->character->room && d->character->in_room != NOWHERE)
		{
			if ( GET_POS (d->character) == POSITION_FIGHTING
			)
				fprintf( fp, "%d %s %s %s %d", d->hSocketFD, d->strClientHostname, d->acct->name.c_str(),
						d->character->tname, POSITION_STANDING );
			else
				fprintf( fp, "%d %s %s %s %d", d->hSocketFD, d->strClientHostname, d->acct->name.c_str(),
						d->character->tname, GET_POS (d->character) );
			count = 0;
			if ( GET_POS (d->character) == POSITION_SITTING && ( af = get_affect( d->character, MAGIC_SIT_TABLE ) ) ) {
				if ( af->a.table.obj && af->a.table.obj->in_room == d->character->in_room ) {
					for ( obj = d->character->room->contents; obj; obj = obj->next_content ) {
						if ( str_cmp( af->a.table.obj->name, obj->name ) )
							continue;
						else {
							count++;
							if ( obj == af->a.table.obj ) {
								fprintf( fp, " %d%s~\n", count, af->a.table.obj->name );
								obj_recorded = true;
								break;
							}
						}
					}
				}
				remove_affect_type( d->character, MAGIC_SIT_TABLE );
			}
			if ( !obj_recorded )
				fprintf( fp, " 0none~\n" );
			save_char( d->character, true );
		}
		obj_recorded = false;
	}

	fprintf( fp, "-1\n" );
	fclose( fp );

	if ( engine.in_build_mode() && cmd != 1 )
		do_zsave( NULL, "all", 0 );

	save_player_rooms();
	save_dwelling_rooms();

	if ( engine.in_play_mode() ) {
		save_stayput_mobiles();
		save_tracks();
		save_banned_sites();
		save_world_state();
	}

	if ( engine.in_build_mode() )
		update_crafts_file();

	sprintf( buf, "%d", engine.get_port() );
	sprintf( buf2, "%d", nMainSocket );
	chdir( ".." );
	execl( "bin/server", "bin/server", buf, "-c", buf2, ( char * ) NULL );
	system_log( "execl() failed!", true );
	chdir( "lib" );

	send_to_gods( "Reboot execl() failed. Aborting and resuming normal operation... please do not attempt again." );
}

void copyover_recovery( void ) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	DESCRIPTOR_DATA *d, *td;
	FILE *fp;
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	char name[ MAX_STRING_LENGTH ];
	char table_name[ MAX_STRING_LENGTH ];
	char host[ MAX_STRING_LENGTH ];
	char account_name[ MAX_STRING_LENGTH ];
	char buf[ MAX_STRING_LENGTH ];
	int desc, online, guest;
	time_t current_time;
	int i = 0, conn, pos = 0, count = 0;

	system_log( "Soft reboot recovery sequence initiated.", false );

	if ( !( fp = fopen( ".copyover_data", "r" )) ) {
		system_log( "Copyover file not found! Aborting copyover and shutting down.", true );
		abort();
	}

	descriptor_list = NULL;

	for ( ;; ) {
		conn = 0;

		fscanf( fp, "%d %s %s %s %d %d", &desc, host, account_name, name, &pos, &count );

		if ( desc == -1 ) {
			break;
		}

		d = new struct descriptor_data;
		init_descriptor( d, desc );

		/*		descriptor_list = d; */

		if ( !descriptor_list )
			descriptor_list = d;
		else
			for ( td = descriptor_list; td; td = td->next ) {
				if ( !td->next ) {
					d->next = NULL;
					td->next = d;
					break;
				}
			}

		if ( d->hSocketFD > maxdesc )
			maxdesc = d->hSocketFD;

		d->strClientHostname = add_hash( host );

		d->acct = new account( account_name );

		if ( !strstr( name, "Guest" ) )
			d->character = load_pc( name );
		else
			create_guest_avatar( d, "recreate" );

		if ( !d->character )
			continue;

		d->character->descriptor = d;
		d->prompt_mode = 1;
		d->connected = CON_SOFT_REBOOT;

		d->character->pc->owner = d;
		d->character->pc->last_connect = time( 0 );

		if ( !IS_SET (d->character->flags, FLAG_GUEST) && d->acct && d->acct->color )
			d->character->color++;

		if ( IS_SET (d->character->flags, FLAG_GUEST) )
			d->character->color++;

		d->character->flags &= ~( FLAG_ENTERING | FLAG_LEAVING);

		pc_to_game( d->character );

		if ( !IS_SET (d->character->flags, FLAG_GUEST) ) {
			load_char_objs( d->character, GET_NAME (d->character) );

			if ( !d->character->in_room || d->character->in_room == NOWHERE
			)
				char_to_room( d->character, OOC_LOUNGE );
			else
				char_to_room( d->character, d->character->in_room );

			sprintf( buf, "save/player/%c/%s.a", tolower( *GET_NAME (d->character) ), CAP (GET_NAME (d->character)) );
			if ( !IS_SET (d->character->flags, FLAG_GUEST) )
				load_saved_mobiles( d->character, buf );
		}

		online = 0;
		guest = 0;

		for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->character )
				continue;
			if ( td->character->pc->level )
				continue;
			if ( td->character->pc->create_state != 2 )
				continue;
			if ( td->connected )
				continue;
			if ( IS_SET (td->character->flags, FLAG_GUEST) ) {
				guest++;
			}
			if ( IS_MORTAL (td->character) && !IS_SET (td->character->flags, FLAG_GUEST))
			{
				online++;
			}
		}

		if ( online >= count_max_online ) {
			count_max_online = online;
			current_time = time( 0 );
			ctime_r( &current_time, max_online_date );
			max_online_date[ strlen( max_online_date ) - 1 ] = '\0';
		}

		if ( guest >= count_guest_online ) {
			count_guest_online = guest;
		}

		d->character->pc->last_logon = time( 0 );

		sprintf( table_name, "%s", fread_string( fp ) );

		if ( str_cmp( table_name, "none" ) ) {
			i = 0;
			for ( obj = d->character->room->contents; obj; obj = obj->next_content ) {
				if ( !str_cmp( table_name, obj->name ) ) {
					i++;
					if ( i == count ) {
						magic_add_affect( d->character, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0 );
						af = get_affect( d->character, MAGIC_SIT_TABLE );
						af->a.table.obj = obj;
						break;
					}
				}
			}
		}

		GET_POS (d->character) = pos;

		d->connected = CON_PLYNG;

	}

	fclose( fp );

	if ( engine.in_play_mode() ) {
		mysql_safe_query( "SELECT room, direction, state "
				"FROM copyover_doors" );
		if ( ( result = mysql_store_result( database )) ) {
			if ( mysql_num_rows( result ) ) {
				while ( ( row = mysql_fetch_row( result ) ) ) {
					int room_number = strtol( row[ 0 ], 0, 10 );
					int door_dir = strtol( row[ 1 ], 0, 10 );
					exit_state door_state = ( exit_state ) strtol( row[ 2 ], 0, 10 );

					set_door_state( room_number, door_dir, door_state );
				}
			}
			mysql_free_result( result );
		} else // result = 0
		{
			std::ostringstream error_message;
			error_message << "Error: " << std::endl << __FILE__ << ':' << __func__ << '(' << __LINE__ << "):"
					<< std::endl << mysql_error( database );
			send_to_gods( ( error_message.str() ).c_str() );
		}

		mysql_safe_query( "DELETE FROM copyover_doors" );

	}
}

void check_reboot( void ) {
	DESCRIPTOR_DATA *d;
	FILE *fp;
	bool block = false;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->character )
			continue;
		if ( !d->character->pc )
			continue;
		if ( d->connected != CON_PLYNG && d->character->pc->nanny_state )
			block = true;
		if ( d->character->pc->create_state == STATE_APPLYING
		)
			block = true;
		if ( IS_SET (d->character->act, PLR_QUIET) )
			block = true;
		if ( d->character->fighting && IS_NPC (d->character->fighting)
		)
			block = true;
		if ( d->character->subdue && IS_NPC (d->character->subdue)
		)
			block = true;
		if ( d->character->aiming_at && IS_NPC (d->character->aiming_at)
		)
			block = true;
		if ( d->character->following && IS_NPC (d->character->following)
		)
			block = true;
		if ( d->character->mount )
			block = true;
	}

	if ( !block ) {
		if ( !( fp = fopen( ".reboot", "w" )) ) {
			system_log( "Error creating reboot file.", true );
			return;
		}
		fprintf( fp, "Reboot Queue\n" );
		fclose( fp );
		unlink( "booting" );
		prepare_copyover( 0 );
	}
}

void shutdown_request( int signo ) {
	FILE *fp;

	system_log( "Received USR2 - reboot request.", false );

	if ( engine.in_build_mode() ) {
		system_log( "Not player port, ignoring...", false );
		return;
	}

	if ( engine.in_test_mode() ) {
		shutd = 1;
		return;
	}

	if ( !finished_booting || ( fp = fopen( "booting", "r" ) ) ) {
		/* Rebooting already. */
		system_log( "Signal ignored - already in the middle of a reboot.", true );
		return;
	}

	if ( !( fp = fopen( ".reboot", "w" )) ) {
		system_log( "Error opening reboot file - aborting!", true );
		abort();
	}
	fprintf( fp, "System Reboot Signal\n" );
	fclose( fp );

	prepare_copyover( 0 );
}

void logsig( int signo ) {
	system_log( "Signal received. Ignoring.", true );
}

void sigchld( int signo ) {
	pid_t pid;
	int stat;

	while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
		;
	signal( SIGCHLD, sigchld );

	return;
}

bool in_crash_loop( void ) {
	bool bInCrashLoop = false;
	FILE *fp;

	fp = fopen( "recovery", "r" );
	if ( fp ) {
		system_log( "Crash loop detected. Shutting down and dumping core...", true );
		bInCrashLoop = true;
		fclose( fp );
	}
	return bInCrashLoop;
}

bool set_reboot_file( void ) {
	bool bSetRebootFile = false;
	FILE *fp;

	fp = fopen( ".reboot", "w" );
	if ( fp ) {
		fprintf( fp, "Crash Recovery Script\n" );
		bSetRebootFile = true;
		fclose( fp );
	} else {
		system_log( "Error opening reboot file in sigsegv()!", true );
	}
	return bSetRebootFile;
}

#define GDB_PATH "/usr/bin/gdb"
#define PERL_PATH "/usr/bin/perl"
#define PERL_GDB_FILTER "$x=0;while(<STDIN>){if($_=~/^#4/){$x=1;print \"---------------------------\\n\"} print \"$_\" if $x;if($_=~/in main .argc/){$x=0;}}"
void gdbdump( char *strGdbCommandFile, char *strFilterScript ) {
	int nServerPid, nGdbPid, nFilterPid;
	int fdTmpStdin, fdTmpStdout, fdTmpStderr;
	int fdFilterOutput;
	int fdGdbToFilterPipe[ 2 ];
	char strServerPid[ 10 ];
	char strServerPath[ AVG_STRING_LENGTH * 2] = "";char
	strOutfilePath[ AVG_STRING_LENGTH * 2] = "";

	/* Get the pid of this server. */
nServerPid	= getpid();
	sprintf( strServerPid, "%d", nServerPid );
	sprintf( strOutfilePath, "%s/crashes/gdb.%d", ( engine.get_base_path( "test" ) ).c_str(), nServerPid );
	sprintf( strServerPath, "%s/bin/server", ( engine.get_base_path() ).c_str() );

	/* backup fds and redirect i/o for gdb */
	fdTmpStdin = dup( 0 );
	fdTmpStdout = dup( 1 );
	fdTmpStderr = dup( 2 );
	if ( pipe( fdGdbToFilterPipe ) == -1 ) {
		send_to_gods( "gdbdumb: pipe failed." );
		return;
	}
	dup2( fdTmpStdin, 0 );
	dup2( fdGdbToFilterPipe[ 1 ], 1 );
	dup2( fdTmpStderr, 2 );

	/* Create new process for "gdb" */
	nGdbPid = fork();
	if ( nGdbPid == -1 ) {
		system_log( "gdbdump: gdb fork failed.", true );
		return;
	}

	if ( nGdbPid == 0 ) {

		/* childe - close unused fds and exec gdb */
		close( fdGdbToFilterPipe[ 0 ] );
		close( fdGdbToFilterPipe[ 1 ] );
		close( fdTmpStdin );
		close( fdTmpStdout );
		close( fdTmpStderr );

		execlp( GDB_PATH, "gdb", "-silent", "-x", strGdbCommandFile, strServerPath, strServerPid, NULL );
		system_log( "gdbdump: exec gdb", true );
		exit( 2 );
	}

	/* backup fds and redirect i/o for perl */
	dup2( fdGdbToFilterPipe[ 0 ], 0 );
	fdFilterOutput = creat( strOutfilePath, 0666 );
	if ( fdFilterOutput < 0 ) {
		system_log( "gdbdump: creat strOutfilePath failed", true );
		return;
	}
	dup2( fdFilterOutput, 1 );
	close( fdFilterOutput );
	dup2( fdTmpStderr, 2 );

	nFilterPid = fork();
	if ( nFilterPid == -1 ) {
		system_log( "gdbdump: perl fork failed", true );
	}

	if ( nFilterPid == 0 ) {

		/* childe - close unused fds and exec perl filter */
		close( fdGdbToFilterPipe[ 0 ] );
		close( fdGdbToFilterPipe[ 1 ] );
		close( fdTmpStdin );
		close( fdTmpStdout );
		close( fdTmpStderr );

		execlp( PERL_PATH, "gdb", "-e", strFilterScript, NULL );
		system_log( "gdbdump: exec filter", true );
		exit( 2 );

	}

	/* clean-up: restore and close fds */
	dup2( fdTmpStdin, 0 );
	dup2( fdTmpStdout, 1 );
	dup2( fdTmpStderr, 2 );
	close( fdGdbToFilterPipe[ 0 ] );
	close( fdGdbToFilterPipe[ 1 ] );
	close( fdTmpStdin );
	close( fdTmpStdout );
	close( fdTmpStderr );

	/* wait on filter command */
	waitpid( nFilterPid, 0, WNOHANG );
}

void sigsegv( int signo ) {
	int nLastCrash = 0, nServerPid = 0;
	char *p;
	FILE *fp;
	/*FILE          *fdCrashInfo;
	 char         strCrashInfo [AVG_STRING_LENGTH];
	 char         strCrashInfoPath [AVG_STRING_LENGTH * 2]; */
	char buf[ MAX_STRING_LENGTH ];
	char buf2[ MAX_STRING_LENGTH ];

	extern char last_command[];
	int port = engine.get_port();

	nLastCrash = ( int ) time( 0 );
	system_log( "Game has crashed!", true );

	fp = fopen( "last_crash", "w+" );
	if ( fp ) {
		fprintf( fp, "%d\n", nLastCrash );
		fclose( fp );
	}

	nServerPid = getpid();

	if ( !in_crash_loop() && set_reboot_file() && last_descriptor != NULL && last_descriptor->connected == CON_PLYNG
			&& last_descriptor->character != NULL)
			{

		sprintf( buf, "Something in the last command you entered, "
				"'#6%s#0', has crashed the MUD. Although it may "
				"have been due to a random memory corruption, please "
				"try to use this command as sparingly as possible "
				"after the recovery, just in case. A core dump has "
				"been generated to assist debugging, and a coder "
				"will look into this issue as soon as possible. "
				"Thank you for your patience.", last_command );

		reformat_string( buf, &p );

		write_to_descriptor( last_descriptor, "\n#6--- ATTENTION ---#0\n" );
		write_to_descriptor( last_descriptor, p );
		write_to_descriptor( last_descriptor, "#6-----------------#0\n" );

		sprintf( buf, "%s [%d]: %s\r\n\r\nType \'debug %d\' on the shell to retrieve full stack dump.\r\n",
				last_descriptor->character->tname, last_descriptor->character->in_room, last_command, nServerPid );

		sprintf( buf2, "Server Crash [%d: pid %d]", port, nServerPid );

		add_message( 1, last_descriptor->character->tname, -2, "Server", NULL, buf2, "", buf, 0 );

		add_message( 1, "Crashes", -5, last_descriptor->character->tname, NULL, buf2, "", buf, 0 );
	}

	// gdbdump(PATH_TO_TP "/src/gdbdump.ini", PERL_GDB_FILTER);
	sprintf( buf, "/bin/cp %s/bin/server %s/crashes/server-%d.%d", ( engine.get_base_path() ).c_str(),
			( engine.get_base_path( "test" ) ).c_str(), port, nServerPid );

	system( buf );
	abort();
}

/* Add user input to the 'current' string (as defined by d->str) */
void string_add( DESCRIPTOR_DATA * d, char *str ) {
	char *scan = "";
	int terminator = 0;
	char end_char;
	char *p = "";
	CALL_PROC *proc;
	char last_char = '\n';

	end_char = d->max_str == STR_ONE_LINE ? -1 : '@';

	/* Get rid of the \r's that may be included in the user input */

	/* determine if this is the terminal string, and truncate if so */

	for ( scan = str; *scan; last_char = *scan++ ) {
		if ( ( terminator = ( *scan == end_char && last_char == '\n')) ) {
			*scan = '\0';
			break;
		}

	}

	if ( d->max_str == STR_ONE_LINE
	)
		terminator = 1;

	if ( !*d->str ) {
		if ( strlen( str ) >= d->max_str ) {
			send_to_char( "String too long - Truncated.\n\r", d->character );
			*( str + d->max_str - 1 ) = '\0';
			terminator = 1;
		}CREATE( *d->str, char, strlen (str) + 3 );
		strcpy( *d->str, str );
	}

	else {
		if ( strlen( str ) + strlen( *d->str ) >= d->max_str ) {
			send_to_char( "String too long. Last line skipped.\n\r", d->character );
			terminator = 1;
		} else {
			p = ( char * ) alloc( strlen( *d->str ) + strlen( str ) + 3, 3 );
			strcpy( p, *d->str );
			strcat( p, str );

			mem_free( *d->str );
			*d->str = p;
		}
	}

	if ( terminator ) {
		d->str = 0;

		if ( d->character )
			d->character->act &= ~PLR_QUIET;

		if ( d->proc ) {
			proc = d->proc;
			d->proc = NULL;
			( proc )( d );
		}

		else if ( d->connected == CON_CREATION )
			create_menu_options( d );
	}

	else
		strcat( *d->str, "\n" );
}

/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *
one_word( char *argument, char *first_arg ) {
	int found, begin, look_at;

	found = begin = 0;

	do {
		for ( ; isspace( *( argument + begin) ); begin++ );

		if
(		*(argument + begin) == '\"')
		{ /* is it a quote */

			begin++;

			for (look_at = 0; (*(argument + begin + look_at) >= ' ') &&
			(*(argument + begin + look_at) != '\"'); look_at++)
			*(first_arg + look_at) = tolower (*(argument + begin + look_at));

			if (*(argument + begin + look_at) == '\"')
			begin++;

		}
		else
		{

			for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
			*(first_arg + look_at) = tolower (*(argument + begin + look_at));

		}

		*( first_arg + look_at ) = '\0';
		begin += look_at;
	} while ( fill_word( first_arg ) );

	return ( argument + begin );
}

void page_string( DESCRIPTOR_DATA * d, const char *str ) {
	if ( !d )
		return;

	if ( d->showstr_head && *d->showstr_head )
		mem_free( d->showstr_head );

	d->showstr_head = str_dup( str );
	d->showstr_point = d->showstr_head;

	show_string( d, "" );
}

void show_string( DESCRIPTOR_DATA * d, char *input ) {
	char buffer[ MAX_STRING_LENGTH ];
	char buf[ MAX_INPUT_LENGTH ];
	char *scan;
	char *chk;
	int lines = 0;

	one_argument( input, buf );

	if ( *buf ) {

		if ( d->showstr_head ) {
			mem_free( d->showstr_head );
			d->showstr_head = NULL;
		}

		if ( d->header )
			mem_free( d->header );

		d->header = NULL;
		d->showstr_point = NULL;

		return;
	}

	/* show a chunk */

	if ( d->header ) {
		SEND_TO_Q( d->header, d );
		lines++;
	}

	for ( scan = buffer;; scan++, d->showstr_point++ ) {

		*scan = *d->showstr_point;

		if ( *scan == '\n' )
			lines++;

		else if ( !*scan || lines >= 22 ) {
			*scan = '\0';

			/* see if this is the end (or near the end) of the string */

			for ( chk = d->showstr_point; isspace( *chk ); chk++ )
				;

			if ( !*chk ) {

				if ( d->showstr_head ) {
					mem_free( d->showstr_head );
					d->showstr_head = NULL;
				}

				d->showstr_point = NULL;

				if ( d->header )
					mem_free( d->header );

				d->header = NULL;
			} else
				SEND_TO_Q( "\r\n", d );

			SEND_TO_Q( buffer, d );

			return;
		}
	}
}

void const_to_non_const_cstr( const char * string, char * edit_string ) {

	for ( int i = 0; string[ i ] != '\0'; i++ ) {
		edit_string[ i ] = string[ i ];
	}

	return;
}

void const_to_non_const_cstr( const char * string, char * edit_string, int n ) {
	for ( int i = 0; string[ i ] != '\0' && i < n; i++ ) {
		edit_string[ i ] = string[ i ];
	}

	return;
}
