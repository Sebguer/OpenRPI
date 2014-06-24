/*------------------------------------------------------------------------\
|  turf.cpp : Dynamic Terrain Contest Module             atonementrpi.com |
|  Copyright (C) 2011, Kithrater                                          |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <ctype.h>
#include <stdlib.h>

#include "turf.h"
#include "protos.h"
#include "structs.h"
#include "server.h"
#include "utils.h"


extern rpie::server engine;

turfBlock::turfBlock (const int nVirtual)
{
	int row_count = 0;
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	char error_message[ERR_STRING_LENGTH] = "\0";

	initialize ();

	if (!nVirtual || !vtor(nVirtual))
	{
		return;
	}

	mysql_safe_query ("SELECT * FROM %s.turfblocks WHERE room = %d", world_log_db.c_str (), nVirtual);


	if ((result = mysql_store_result (database)) != NULL)
	{
		if ((row_count = mysql_num_rows (result)) == 1)
		{
			row = mysql_fetch_row (result);

			turfRoom = atoi(row[0]);
			turfSystem = atoi(row[1]);
			turfHood = atoi(row[2]);

			hoodLoyalty = atoi(row[3]);
			blockLoyalty = atoi(row[4]);

			blockType = atoi(row[5]);
			blockImprove = atoi(row[6]);

			int base = 7;


			for (int i = 0; i < 5; i++)
			{
				set_clanName(i, row[base + i]);
			}

			base = 12;

			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					scores[i][j] = (row[base]);
					base++;
				}
			}


		}
		else if (row_count > 1)
		{
			snprintf (error_message, ERR_STRING_LENGTH,
				"Warning: account__load: Found %d matches to turfBlock %d",
				row_count, nVirtual);
		}
		else
		{
			snprintf (error_message, ERR_STRING_LENGTH,
				"Warning: account__load: Found %d matches to turfBlock %d",
				row_count, nVirtual);
		}

		mysql_free_result (result);
	}
	else
	{
		snprintf (error_message, ERR_STRING_LENGTH,
			"Warning: turfBlock: %s", mysql_error (database));
	}


	if (*error_message)
	{
		system_log (error_message, true);
	}
}