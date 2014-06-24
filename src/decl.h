//////////////////////////////////////////////////////////////////////////////
//
/// decl.h - Program Declarations
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



#ifndef _rpie_decl_h_
#define _rpie_decl_h_

#include <time.h>

extern void rewind (FILE * stream);

extern int fclose ();
extern void perror ();
extern time_t time ();
extern int pclose ();
extern int gettimeofday ();
extern int getrlimit ();
extern int select ();

#ifndef LINUX
#if defined (_sys_socket_h)
extern int socket (int domain, int type, int protocol);
extern int setsockopt (int s, int level, int optname, char *optval,
		       int optlen);
extern int bind (int s, struct sockaddr_in *name, int namelen);
extern int accept (int s, struct sockaddr_in *addr, int *addrlen);
extern int listen (int s, int backlog);
#endif
#endif

extern int fseek (FILE * stream, long offset, int ptrname);

#endif // _rpie_decl_h_
