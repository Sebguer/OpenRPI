//////////////////////////////////////////////////////////////////////////////
//
/// turf.h - Turf Class Structures and Routintes
//
/// Shadows of Isildur RPI Engine++ w/ pathetic ARPI hacks
/// Copyright (C) 2005-2006 C. W. McHenry
/// Authors: JPH (kithrater@atonementrpi.com
/// URL: http://atonementrpi.com
//
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

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <vector>
#include <exception>
#include "server.h"
#include "protos.h"
#include "utils.h"
#include "structs.h"

#define LOVE 0
#define FEAR 1

extern std::vector<TurfSystem*> TurfSystems;
extern std::vector<Neighbourhood*> Neighbourhoods;

void turf_update (void);

void load_turf_hoods (void);
void load_turf_systems (void);
void save_turf_mysql (void);

