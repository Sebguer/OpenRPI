/*-----------4-------------------------------------------------------------\
|  turf.cpp : Dynamic Terrain Contest Module             atonementrpi.com |
|  Copyright (C) 2011, Kithrater                                          |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "server.h"
#include "structs.h"
#include "constants.h"
#include "account.h"
#include "protos.h"
#include "group.h"
#include "utils.h"
#include "utility.h"
#include "net_link.h"
#include "decl.h"
#include "turf.h"

std::vector<TurfSystem*> TurfSystems;
std::vector<Neighbourhood*> Neighbourhoods;

extern rpie::server engine;

/*turfBlock::turfBlock (const int nVirtual)
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
}*/

void Block::saveBlock (void)
{
	//if (!engine.in_play_mode ())
	//	return;

	char buf[MAX_STRING_LENGTH] = {'\0'};	
	std::string world_log_db = engine.get_config ("world_log_db");
	sprintf(buf, 
		" INSERT INTO %s.turfblocks (room, hood, loyalty, type, improve, violence,"
		" onelove, onefear, twolove, twofear, threelove, "
		" threefear, fourlove, fourfear, fivelove, fivefear,"
		" actone, acttwo, actthree, actfour, actfive,"
		" powone, powtwo, powthree, powfour, powfive"
		" )"
		" VALUES ('%d', '%d', '%d', '%d', '%d', '%d',"
		" '%d', '%d', '%d', '%d', '%d',"
		" '%d', '%d', '%d', '%d', '%d',"
		" '%d', '%d', '%d', '%d', '%d',"
		" '%d', '%d', '%d', '%d', '%d'"
		" )", 
		world_log_db.c_str (), 
		turfRoom, turfHood, blockLoyalty, blockType, blockImprove, violence,
		scores[0][0], scores[0][1],	scores[1][0], scores[1][1],	scores[2][0], 
		scores[2][1], scores[3][0], scores[3][1], scores[4][0], scores[4][1],
		actioned[0], actioned[1], actioned[2], actioned[3], actioned[4],
		actionScore[0], actionScore[1], actionScore[2], actionScore[3], actionScore[4]);

	mysql_safe_query (buf);
}

void Neighbourhood::saveHood (void)
{
	if (!engine.in_play_mode ())
	    return;

	char buf[MAX_STRING_LENGTH] = {'\0'};	
	std::string world_log_db = engine.get_config ("world_log_db");
	sprintf (buf, "INSERT into %s.turfhoods (id, loyalty, system, name, dumproom, npc_coldload_first, npc_coldload_second)"
		" VALUES ('%d', '%d', '%d', '%s', '%d')", 
		world_log_db.c_str (), hoodId, hoodLoyalty, systemId, name.c_str(), dumpRoom, npc_coldload_first, npc_coldload_second);

	mysql_safe_query (buf);
} 

void TurfSystem::saveSystem (void)
{
	if (!engine.in_play_mode ())
	    return;

    do_debugecho("Saving TurfSystem.");
    
	char buf[MAX_STRING_LENGTH] = {'\0'};	
	std::string world_log_db = engine.get_config ("world_log_db");
	sprintf (buf, "INSERT into %s.turfsystems (id, boardvnum, clanone, clantwo, clanthree, clanfour, clanfive, dumpvnum1, enforcemob, "
				  "shopkeepmob, soldiermob, shopkeepfee, soldierfee, dumpvnum2 "
		" VALUES ('%d', '%d', "
		" '%s', '%s', '%s', '%s', '%s', "
		" '%d', '%d', "
		" '%d, '%d, '%d, '%d')", 
		world_log_db.c_str (), systemId,  boardVnum, 
		clanName[0].c_str(), clanName[1].c_str(), clanName[2].c_str(), clanName[3].c_str(), clanName[4].c_str(), dumpVnum1, enforceMob,
		shopkeepMob, soldierMob, shopkeepFee, soldierFee, dumpVnum2);

	mysql_safe_query (buf);
}

Block::Block (int vnum)
{
	int row_count = 0;
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	char error_message[ERR_STRING_LENGTH] = "\0";

	initialize ();

	if (!vnum || !vnum_to_room(vnum))
	{
		return;
	}

	std::string world_log_db = engine.get_config ("world_log_db");

	mysql_safe_query ("SELECT * FROM %s.turfblocks WHERE room = %d", world_log_db.c_str (), vnum);

	if ((result = mysql_store_result (database)) != NULL)
	{
		if ((row_count = mysql_num_rows (result)) == 1)
		{
			row = mysql_fetch_row (result);

			turfRoom = atoi(row[0]);
			turfHood = atoi(row[1]);

			blockLoyalty = atoi(row[2]);

			blockType = atoi(row[3]);
			blockImprove = atoi(row[4]);		

			violence = atoi(row[5]);

			int base = 6;

			for (int i = 0; i < TURF_CLANS; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					scores[i][j] = atoi(row[base]);
					base++;
				}
			}

			base = 26;

			for (int i = 0; i < TURF_CLANS; i++)
			{
				actioned[i] = atoi(row[base]);
				base++;
			}

			base = 36;

			for (int i = 0; i < TURF_CLANS; i++)
			{
				actionScore[i] = atoi(row[base]);
				base++;
			}
		}
		else if (row_count > 1)
		{
			snprintf (error_message, ERR_STRING_LENGTH,
				"Warning: account__load: Found %d matches to turfBlock %d",
				row_count, vnum);
		}
		else
		{
			snprintf (error_message, ERR_STRING_LENGTH,
				"Warning: account__load: Found %d matches to turfBlock %d",
				row_count, vnum);
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

// Function which allows us to do something
// in this block again.
void Block::refreshBlock(void)
{
	for (int i = 0; i < TURF_CLANS; i++)
	{
		actioned[i] = 0;
		actionScore[i] = 0;
	}
}

// Function that determines
// which of the five clans is controlling us now.
bool Block::contestBlock(void)
{
	// turfBase can't be contested.
	if (blockType == turfTypeBase)
		return false;

	int totals[TURF_CLANS] = {0, 0, 0, 0, 0};
	int highest = TURF_CLANS + 1;
	int secondHighest = TURF_CLANS + 1;

	for (int i = 0; i < TURF_CLANS; i++)
	{
		// No love in the HeartBlock.
		if (blockType == turfTypeHeart)
		{
			scores[i][LOVE] = 0;
			totals[i] = scores[i][FEAR] * 2;
		}
		else
		{
			totals[i] = scores[i][LOVE] + scores[i][FEAR];		
		}

		if (totals[i] >= 5 && totals[i] > totals[highest])
		{
			secondHighest = highest;
			highest = i;
		}
	}

	// Did we already have a loyalty?
	if (blockLoyalty != -1)
	{
		//Is the highest our block loyalty? If so, no change.
		if (highest == blockLoyalty)
		{
			return false;
		}
		// If not, and the contesting loyalty is 5 or more higher than
		// we flip the block and everything gets reset.
		else if (totals[highest] >= totals[secondHighest] + 5)
		{
			blockLoyalty = -1;
			
			// Everyone thing gets wiped to zero if we go in to disputed - no overwhelming advantage.			
			for (int i = 0; i < TURF_CLANS; i++)
			{
				scores[i][LOVE] = 0;
				scores[i][FEAR] = 0;
			}
			return true;
		}
	}
	// Someone got high enough to claim this disputed territory, and is
	// five or more higher than the next claimant.
	else if (totals[highest] >= totals[secondHighest] + 5)
	{
		blockLoyalty = highest;
		return true;
	}

	// We've an existing loyalty, and either the highest loyalty is that existing loyalty,
	// or the highest isn't enough to overthrow the second highest, so no change.
	return false;
}

void Block::decayBlock (int hoodLoyalty, int heartLoyalty, bool isTurn)
{
	if (!isTurn)
		return;

	int naturals[TURF_CLANS];

	violence = 0;

	OBJ_DATA *artObj = NULL;
	artObj = get_obj_in_list_num(5065, vnum_to_room(turfRoom)->contents);

	for (int i = 0; i < TURF_CLANS; i++)
	{
		naturals[i] = 0;

		// No love in the Heart Block.
		if (blockType == turfTypeHeart)
		{
			scores[i][LOVE] = 0;
		}

		// Natural is 0, with +5 for blockLoyalty and heartBlock loyalty, +2 for hood loyalty.
		if (i == blockLoyalty)
		{
			naturals[i] += 5;
		}

		if (i == heartLoyalty)
		{
			naturals[i] += 5;
		}

		if (i == hoodLoyalty)
		{
			naturals[i] += 3;
		}

		if (artObj && artObj->o.od.value[1] && artObj->o.od.value[2] == i)
		{
			naturals[i] += artObj->o.od.value[0];
		}

		scores[i][LOVE] = naturals[i];
		scores[i][FEAR] = MIN(naturals[i], (blockType == turfTypeHeart ? 10 : 5));

		// Fear has a min of 0, max of 50.
		if (scores[i][FEAR] > 50)
			scores[i][FEAR] = 50;

		if (scores[i][FEAR] < 0)
			scores[i][FEAR] = 0;

		// Love has a max of 50 or fear -15, and a min of -50.
		if ((scores[i][FEAR] - 15) > scores[i][LOVE])
			scores[i][LOVE] = scores[i][FEAR] - 15;

		if (scores[i][LOVE] > 50)
			scores[i][LOVE] = 50;

		if (scores[i][LOVE] < -50)
			scores[i][LOVE] = -50;
	}
}

// Called every 12 RL hours.
void Block::updateBeat(int hoodLoyalty, int heartLoyalty, bool isTurn)
{
	contestBlock();
	decayBlock(hoodLoyalty, heartLoyalty, isTurn);
	refreshBlock();
	saveBlock();
}

int Neighbourhood::determineHeart(void)
{
	for (std::map<int, Block>::iterator it = hoodBlocks.begin(); it != hoodBlocks.end(); it++)
	{
		if (it->second.blockType == turfTypeHeart)
		{
			return it->second.blockLoyalty;
		}
	}

	return -1;
}

void Neighbourhood::updateBeats(bool isTurn)
{
	char buf[MAX_STRING_LENGTH] = {'\0'};
	std::string world_log_db = engine.get_config ("world_log_db");	

	if (engine.in_play_mode ())
	{
		sprintf (buf, "DELETE FROM %s.turfblocks WHERE hood = %d", world_log_db.c_str (), hoodId);
		mysql_safe_query (buf);
	}

	for (std::map<int, Block>::iterator it = hoodBlocks.begin(); it != hoodBlocks.end(); it++)
	{
		it->second.updateBeat(hoodLoyalty, determineHeart(), isTurn);
	}
}

void Neighbourhood::updateHood(void)
{
	contestHood();
	rewardHood();
	saveHood();
}

void Neighbourhood::rewardHood(void)
{
	double love_count = 0;
	double fear_count = 0;
	int total_count = 0;
	int loyalty = this->hoodLoyalty;

	if (loyalty < 0)
		return;

	for (std::map<int, Block>::iterator it = hoodBlocks.begin(); it != hoodBlocks.end(); it++)
	{
		if (it->second.scores[loyalty][LOVE] >= 15)
		{
			love_count+= 0.34;
		}

		if (it->second.scores[loyalty][FEAR] >= 10)
		{
			fear_count+= 0.34;
		}
	}

	love_count = MIN(love_count, 3.0);
	fear_count = MIN(fear_count, 2.0);

	total_count = (int) love_count + (int) fear_count;
	total_count = total_count + 2;

	TurfSystem *thisSystem = NULL;

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == this->systemId)
		{
			thisSystem = (*it);
			break;
		}
	}

	OBJ_DATA *load_obj = NULL;

	if (thisSystem->dumpVnum1 && (vtoo(thisSystem->dumpVnum1)))
	{
		if (this->dumpRoom && vnum_to_room(this->dumpRoom))
		{
			for (int i = 0; i < fear_count; i ++)
			{
				load_obj = load_object(thisSystem->dumpVnum1);
				obj_to_room(load_obj, this->dumpRoom);
				load_obj->omote_str = add_hash("has been recently delivered.");
			}
		}
	}
	if (thisSystem->dumpVnum2 && (vtoo(thisSystem->dumpVnum2)))
	{
		if (this->dumpRoom && vnum_to_room(this->dumpRoom))
		{
			for (int i = 0; i < love_count; i ++)
			{
				load_obj = load_object(thisSystem->dumpVnum2);
				obj_to_room(load_obj, this->dumpRoom);
				load_obj->omote_str = add_hash("has been recently delivered.");
			}
		}
	}
}

bool Neighbourhood::contestHood(void)
{
	int loyalty[TURF_CLANS] = {0, 0, 0, 0, 0};
	int existing = hoodLoyalty;
	int count = 0;
	for (std::map<int, Block>::iterator it = hoodBlocks.begin(); it != hoodBlocks.end(); it++)
	{
		loyalty[it->second.blockLoyalty]++;
		count++;
	}

	for (int i = 0; i < TURF_CLANS; i++)
	{
		if (loyalty[i] > count * 0.50)
		{
			hoodLoyalty = i;
			if (hoodLoyalty == existing)
				return false;
			else
			{
				if (npc_coldload_first)
				{
					CHAR_DATA *recruit = NULL;
					if ((recruit = get_char_id(npc_coldload_first)))
					{
						act("Without the backing of a neighbourhood, $n quickly departs.", false, recruit, 0, 0, TO_ROOM | _ACT_FORMAT);
						extract_char(recruit);
						npc_coldload_first = 0;
					}
				}

				if (npc_coldload_second)
				{
					CHAR_DATA *recruit = NULL;
					if ((recruit = get_char_id(npc_coldload_first)))
					{
						act("Without the backing of a neighbourhood, $n quickly departs.", false, recruit, 0, 0, TO_ROOM | _ACT_FORMAT);
						extract_char(recruit);
						npc_coldload_first = 0;
					}
				}

				return true;
			}
		}
	}

	hoodLoyalty = -1;

	if (hoodLoyalty == existing)
		return false;
	else
		return true;
}

/*void load_turf_blocks (void)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;

	std::string world_log_db = engine.get_config ("world_log_db");

	mysql_safe_query ("SELECT * FROM %s.turfblocks", world_log_db.c_str ());

	if (!(result = mysql_store_result (database)))
		return;
}*/

void load_turf_hoods (void)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;

	std::string world_log_db = engine.get_config ("world_log_db");

	mysql_safe_query ("SELECT * FROM %s.turfhoods", world_log_db.c_str ());

	if (!(result = mysql_store_result (database)))
		return;

	// Now we go through and create our variable lists.

	while ((row = mysql_fetch_row(result)))
	{
		MYSQL_RES *resultTwo = NULL;
		MYSQL_ROW rowTwo;

		Neighbourhood *newHood = new Neighbourhood();

		newHood->hoodId = atoi(row[0]);
		newHood->hoodLoyalty = atoi(row[1]);
		newHood->systemId = atoi(row[2]);

		if (row[3] && *row[3])
			newHood->name = row[3];

		if (row[4] && *row[4])
			newHood->dumpRoom = atoi(row[4]);

		if (row[5] && *row[5])
			newHood->npc_coldload_first = atoi(row[5]);

		if (row[6] && *row[6])
			newHood->npc_coldload_second = atoi(row[6]);

        Neighbourhoods.push_back(newHood);

		mysql_safe_query ("SELECT room FROM %s.turfblocks WHERE hood = %d", world_log_db.c_str (), newHood->hoodId);

		if (!(resultTwo = mysql_store_result (database)))
			continue;

		while ((rowTwo = mysql_fetch_row(resultTwo)))
		{
			Block *newBlock = new Block(atoi(rowTwo[0]));
			newHood->hoodBlocks.insert (std::pair<int, Block>(atoi(rowTwo[0]), *newBlock));
			vnum_to_room(newBlock->turfRoom)->hood = newHood;
		}
	}
}

void load_turf_systems (void)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;

	std::string world_log_db = engine.get_config ("world_log_db");

	mysql_safe_query ("SELECT * FROM %s.turfsystems", world_log_db.c_str ());

	if (!(result = mysql_store_result (database)))
		return;

	while ((row = mysql_fetch_row(result)))
	{
		TurfSystem *newSystem = new TurfSystem();
		newSystem->systemId = atoi(row[0]);

		if (row[1] && *row[1])
			newSystem->boardVnum = atoi(row[1]);

		if (row[2] && *row[2])
			newSystem->clanName[0] = row[2];

		if (row[3] && *row[3])
			newSystem->clanName[1] = row[3];

		if (row[4] && *row[4])
			newSystem->clanName[2] = row[4];

		if (row[5] && *row[5])
			newSystem->clanName[3] = row[5];

		if (row[6] && *row[6])
			newSystem->clanName[4] = row[6];

		if (row[7] && *row[7])
			newSystem->clanName[5] = row[7];

		if (row[8] && *row[8])
			newSystem->clanName[6] = row[8];

		if (row[9] && *row[9])
			newSystem->clanName[7] = row[9];

		if (row[10] && *row[10])
			newSystem->clanName[8] = row[10];

		if (row[11] && *row[11])
			newSystem->clanName[9] = row[11];

		if (row[12] && *row[12])
			newSystem->dumpVnum1 = atoi(row[12]);
			
		if (row[13] && *row[13])
			newSystem->enforceMob = atoi(row[13]);

		if (row[14] && *row[14])
			newSystem->shopkeepMob = atoi(row[14]);

		if (row[15] && *row[15])
			newSystem->soldierMob = atoi(row[15]);

		if (row[16] && *row[16])
			newSystem->shopkeepFee = atoi(row[16]);

		if (row[17] && *row[17])
			newSystem->soldierFee = atoi(row[17]);
			
		if (row[18] && *row[18])
			newSystem->dumpVnum2 = atoi(row[18]);
			
		TurfSystems.push_back(newSystem);
	}
}

void turf_update (void)
{
	int true_hour = time_info.hour + (84 * time_info.day);  
	bool first_hood = true;
	bool second_hood = true;

	if (!(true_hour % 84))
	{
		send_to_gods("turf Beat");

		std::string world_log_db = engine.get_config ("world_log_db");	
		char buf[MAX_STRING_LENGTH] = {'\0'};	
		vector<Neighbourhood*>::iterator it;
		for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
		{
			if (true_hour == 0)
			{
				*buf = '\0';	
				sprintf (buf, "DELETE FROM %s.turfhoods WHERE id = %d", world_log_db.c_str(), (*it)->hoodId);
				mysql_safe_query (buf);
				(*it)->updateHood();

				if (first_hood)
				{						
					for	(vector<TurfSystem*>::iterator nit = TurfSystems.begin(); nit != TurfSystems.end(); nit++)
					{
						if (vtoo((*nit)->boardVnum))
							(*nit)->turnReport();
					}
					first_hood = false;
					send_to_gods("Turn update");
				}
			}

			if (true_hour == 0)
			{
				(*it)->updateBeats(true);
			}
			else
			{
				(*it)->updateBeats(false);
			}
			
			if (second_hood)
			{
				for	(vector<TurfSystem*>::iterator nit = TurfSystems.begin(); nit != TurfSystems.end(); nit++)
				{
					if (vtoo((*nit)->boardVnum))
						(*nit)->beatReport();
				}
				second_hood = false;
			}

			// Every 21 RL hours we'll check to see if we've still got our mobiles or not.
			if ((*it)->npc_coldload_first)
			{
				if (!get_char_id((*it)->npc_coldload_first))
				{
					(*it)->npc_coldload_first = 0;
				}
			}
			else if ((*it)->npc_coldload_second)
			{
				if (!get_char_id((*it)->npc_coldload_second))
				{
					(*it)->npc_coldload_second = 0;
				}
			}
		}
	}
}

std::string Block::clanName (int systemId, int clan)
{
	vector<TurfSystem*>::iterator it;

	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == systemId)
		{
			return (*it)->clanName[clan];
		}	
	}

	return NULL;
}

std::string Block::shortOwn (int systemId)
{
	if (blockLoyalty == -1)
	{
		return "#6disputed#0";
	}

	std::string output = "";
	int score = 0;

	if (blockType == turfTypeHeart)
	{
		score = scores[blockLoyalty][FEAR] * 2;
	}
	else
	{
		score = scores[blockLoyalty][LOVE] +  scores[blockLoyalty][FEAR];
	}

	if (score >= 25)
	{
		output.assign("tightly controlled by #6");
	}
	if (score >= 15)
	{
		output.assign("controlled by #6");
	}
	else if (score >= 6)
	{
		output.assign("loosely controlled by #6");
	}
	else
	{
		output.assign("barely controlled by #6");
	}

	output.append(get_clandef(clanName(systemId, blockLoyalty).c_str())->literal);

	output.append("#0");

	return output;
}

std::string Block::status (int systemId, int hoodLoyalty, int heartLoyalty, char *hoodName)
{
	std::string output = "";

	output.assign("This block is ");
	output.append(shortOwn(systemId));

	if (hoodLoyalty == -1)
	{
		output.append (" and is in the #6disputed#0 neighbourhood of #6");
		output.append (hoodName);
		output.append ("#0");
	}
	else
	{
		output.append (" and is in the \n#6");
		output.append (get_clandef(clanName(systemId, hoodLoyalty).c_str())->literal);
		output.append ("#0 controlled neighbourhood of #6");
		output.append (hoodName);
		output.append ("#0");
	}

	if (heartLoyalty != -1)
	{
		output.append(".\n\nThe heart of the neighbourhood is controlled by #6");
		output.append (get_clandef(clanName(systemId, heartLoyalty).c_str())->literal);
		output.append("#0");
		if (violence)
		{
			output.append(", and \nthe block has recently seen some violence");
		}
	}
	else
	{
		if (violence)
		{
			output.append(".\n\nThe block has recently seen some violence");
		}
	}

	output.append(".\n");

	for (int i = 0; i < TURF_CLANS; i ++)
	{
		if (clanName(systemId, i).empty())
			continue;

		// If they've got no score, then don't bother listing them.
		if (scores[i][LOVE] < 5 && scores[i][LOVE] > -5 && 	scores[i][FEAR] < 5)
			continue;		

		output.append("\n");
		output.append(get_clandef(clanName(systemId, i).c_str())->literal);

		if (blockType != turfTypeHeart)
		{
			output.append(" are#2");

			if (scores[i][LOVE] >= 35)
			{
				output.append(" blindingly loved");
			}
			else if (scores[i][LOVE] >= 25)
			{
				output.append(" extremely loved");
			}
			else if (scores[i][LOVE] >= 15)
			{
				output.append(" loved");
			}
			else if (scores[i][LOVE] >= 10)
			{
				output.append(" well thought of");
			}
			else if (scores[i][LOVE] >= 5)
			{
				output.append(" liked");
			}
			else if (scores[i][LOVE] >= -4)
			{
				output.append(" neither loved nor loathed");
			}
			else if (scores[i][LOVE] >= -14)
			{
				output.append(" actively disliked");
			}
			else
			{
				output.append(" loathed");
			}

			output.append("#0, and");
		}

		if (scores[i][FEAR] >= 35)
		{
			output.append(" #3inspire absolute terror");
		}
		else if (scores[i][FEAR] >= 25)
		{
			output.append(" #3are outright feared");
		}
		else if (scores[i][FEAR] >= 15)
		{
			output.append(" #3are feared");
		}
		else if (scores[i][FEAR] >= 10)
		{
			output.append(" #3are given significant respect");
		}
		else if (scores[i][FEAR] >= 5)
		{
			output.append(" #3are to be wary of");
		}
		else
		{
			output.append(" #3are not particularly feared");
		}

		if (actioned[i])
		{
			output.append("#0.\n");
			output.append("Recently, they have been #6");

			if (actionScore[i] >= 15)
			{
				output.append("extremely active"); 
			}
			else if (actionScore[i] >= 10)
			{
				output.append("very active"); 
			}
			else if (actionScore[i] >= 5)
			{
				output.append("active"); 
			}
			else
			{
				output.append("only marginally active"); 
			}

			output.append("#0 on this block");
		}
		output.append("#0.\n");
	}

	return output;
}

// setturf <neighbourhood> <ownerID> <ownerLove> <ownerHate> <blockType>
void do_setturf (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string hoodArg, ownArg, loveArg, fearArg, typeArg, strArgument = argument;

	strArgument = one_argument(strArgument, hoodArg);
	strArgument = one_argument(strArgument, ownArg);
	strArgument = one_argument(strArgument, loveArg);
	strArgument = one_argument(strArgument, fearArg);
	strArgument = one_argument(strArgument, typeArg);

	if (typeArg.empty())
	{
		send_to_char("You need to input the neighbourhood, ownerID, love, fear, and type to set this block's info.\n", ch);
		return;
	}

	if (ch->room->hood)
	{
		send_to_char("There is already some block info here.\n", ch);
		return;
	}

	Block *newBlock = new Block;
	newBlock->turfRoom = ch->room->vnum;
	newBlock->turfHood = atoi(hoodArg.c_str());
	newBlock->blockLoyalty = atoi(ownArg.c_str());
	newBlock->scores[newBlock->blockLoyalty][LOVE] = atoi(loveArg.c_str());
	newBlock->scores[newBlock->blockLoyalty][FEAR] = atoi(fearArg.c_str());
	newBlock->blockType = atoi(typeArg.c_str());

	vector<Neighbourhood*>::iterator it;
	for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
	{
		if ((*it)->hoodId == newBlock->turfHood)
		{
			(*it)->hoodBlocks.insert (std::pair<int, Block>(ch->room->vnum, *newBlock));
			ch->room->hood = (*it);
			break;
		}
	}

	newBlock->saveBlock();

	send_to_char("New block info saved.\n", ch);
	return;
}

int Block::clanPosition(char *clanName, int systemId)
{
	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if (systemId == (*it)->systemId)
		{
			return (*it)->clanPosition(clanName);
		}
	}

	return -1;
}



int num_undelayed_followers (CHAR_DATA * ch)
{
	CHAR_DATA		*top_leader = NULL;
	CHAR_DATA		*tch = NULL;
	int group_count = 0;

	if (!(top_leader = ch->following))
		top_leader = ch;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following != top_leader)
			continue;
		if (!CAN_SEE (ch, tch))
			continue;

		if (get_affect (tch, MAGIC_TURF_DELAY) && IS_MORTAL (tch))
			continue;

		group_count = group_count + 1;

	}

	return (group_count);

}


// turf intimidate
// turf terrorise
// turf convert
// turf charity
// turf maintenance
// turf gossip
// turf dedicate
// turf track

void do_turf (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string output, strArgument = argument, argOne;

	Neighbourhood *thisHood = ch->room->hood;

	if (!thisHood)
	{
		send_to_char("You can't do that here.\n", ch);
		return;
	}

	Block thisBlock = thisHood->hoodBlocks.find(ch->room->vnum)->second;

	TurfSystem *thisSystem = NULL;

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == thisHood->systemId)
		{
			thisSystem = (*it);
			break;
		}
	}

	strArgument = one_argument(strArgument, argOne);

	if (!thisSystem)
	{
		send_to_char("You can't do that here.\n", ch);
		return;
	}

	if (argOne == "gossip")
	{
		send_to_char("You put your ear to the street and discover...\n\n", ch);
		send_to_char(thisBlock.status(thisHood->systemId, thisHood->hoodLoyalty, thisHood->determineHeart(), (char *) thisHood->name.c_str()).c_str(), ch);
		return;
	}
	else if (argOne == "track")
	{
		int char_clan = -1;
		char clan_name[MAX_STRING_LENGTH] = { '\0' };
		int clan_flags = 0;
		char *p = '\0';
		p = ch->clans;

		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			for (int i = 0; i < TURF_CLANS; i++)
			{
				if (thisSystem->clanName[i] == clan_name)
				{
					if (char_clan == -1)
					{
						char_clan = i;
						continue;
					}
					else
					{
						send_to_char("Your loyalties are too conflicted for the people here to tell you anything.\n", ch);
						return;
					}
				}
			}
		}

		if (char_clan == -1)
		{
			send_to_char("You don't have any loyalties strong enough to learn anything from the people.\n", ch);
			return;
		}

		if (thisBlock.blockLoyalty != char_clan)
		{
			send_to_char ("The people living here are not loyal enough to you to offer that information.\n", ch);
			return;
		}


		do_track(ch, argument, 0);
		return;
	}

	else if (argOne == "recruit")
	{

		int char_clan = -1;
		char clan_name[MAX_STRING_LENGTH] = { '\0' };
		int clan_flags = 0;
		char *p = '\0';
		p = ch->clans;

		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			for (int i = 0; i < TURF_CLANS; i++)
			{
				if (thisSystem->clanName[i] == clan_name && clan_flags >= CLAN_PRIVATE)
				{
					if (char_clan == -1)
					{
						char_clan = i;
						continue;
					}
					else
					{
						send_to_char("Your loyalties are too conflicted to do that.\n", ch);
						return;
					}
				}
			}
		}

		if (char_clan == -1)
		{
			send_to_char("You don't have any loyalties strong enough to do that.\n", ch);
			return;
		}

		if (thisBlock.blockType != turfTypeHeart)
		{
			send_to_char ("You can only recruit from the heart of a turf.\n", ch);
			return;
		}

		int rank = CLAN_RECRUIT;
		get_clan (ch, thisSystem->clanName[char_clan].c_str(), &rank);

		if (thisBlock.blockLoyalty != char_clan)
		{
			send_to_char ("This block needs to be loyal to you for you to recruit from here.\n", ch);
			return;
		}

		if (thisHood->hoodLoyalty != char_clan)
		{
			send_to_char ("This neighbourhood needs to be loyal to you for you to recruit from here.\n", ch);
			return;
		}

		if (rank < CLAN_SERGEANT || (rank > CLAN_COMMANDER && rank < CLAN_JOURNEYMAN))
		{
			send_to_char ("You need to be ranked as a Sergeant or Journeyman to recruit.\n", ch);
			return;
		}

		if (thisHood->npc_coldload_first && thisHood->npc_coldload_second)
		{
			send_to_char ("You have already recruited as many people as this neighbourhood can support.\n", ch);
			return;
		}

		std::string argTwo;
		strArgument = one_argument(strArgument, argTwo);

		if (argTwo != "shopkeep" && argTwo != "soldier")
		{
			send_to_char ("You need to nominate either a #6shopkeep#0 or a #6soldier#0.\n", ch);
			return;
		}

		int load_number = 0;
		int fee;
		CHAR_DATA *recruit = NULL;

		if (argTwo == "shopkeep")
		{
			load_number = thisSystem->shopkeepMob;
			fee = thisSystem->shopkeepFee;
		}
		else if (argTwo == "soldier")
		{
			load_number = thisSystem->soldierMob;
			fee = thisSystem->soldierFee;
		}

		if (!(recruit = load_mobile (load_number)))
		{
			send_to_char ("There was an error in producing recruits - please let Kithrater know.\n", ch);
			return;
		}

		// Now we run through and let players add lots of fluffy variables to their mobile.
		if ((recruit->race == lookup_race_id("Survivor") || recruit->race == lookup_race_id("Denizen") ||
			recruit->race == lookup_race_id("Mutation") || recruit->race == lookup_race_id("Cybernetic") ||
			recruit->race == lookup_race_id("Phoenixer")) && IS_SET (vnum_to_mob(recruit->mob->vnum)->flags, FLAG_VARIABLE) && !strArgument.empty())
		{			
			char *mobcolor[11];
			std::string buf;

			for (int ind = 0; ind < 11; ind++)
			{
				mobcolor[ind] = '\0';
			}

			for (int ind = 0; ind < 11; ind++)
			{
				if (buf == "!")
				{
					break;
				}
				strArgument = one_argument (strArgument, buf);
				mobcolor[ind] = add_hash(buf.c_str());
			}

			new_create_description (recruit, mobcolor[0], mobcolor[1], mobcolor[2], mobcolor[3], mobcolor[4], mobcolor[5], 
				mobcolor[6], mobcolor[7], mobcolor[8], mobcolor[9], mobcolor[10]);			
		}

		if (strArgument[strArgument.length() - 1] != '!')
		{
			output.assign("The NPC you will recruit will look as follows:\n");
			output.append("Short description - #5");
			output.append(recruit->short_descr);
			output.append("#0\nLong Description - #5");
			output.append(recruit->long_descr);
			output.append("#0\nFull Description -\n");
			output.append(recruit->description);
			output.append("\n\nTo recruit this NPC, you will need to add ! to the end of the command, e.g.:\n#6   turf ");
			output.append(argument);
			output.append(" !#0\n");

			extract_char(recruit);

			send_to_char(output.c_str(), ch);
			return;
		}
		else
		{
			if (can_subtract_money (ch, fee, CURRENCY_PHOENIX))
			{
				if (!thisHood->npc_coldload_first)
				{
					thisHood->npc_coldload_first = recruit->mob->vnum;
				}
				else if (!thisHood->npc_coldload_second)
				{
					thisHood->npc_coldload_second = recruit->mob->vnum;
				}
				else
				{
					send_to_char ("You have already recruited as many people as this neighbourhood can support.\n", ch);
					extract_char(recruit);
					return;
				}

				subtract_money(ch, fee, CURRENCY_PHOENIX);
				char_to_room(recruit, ch->in_room);
				add_clan (recruit, (char *) thisSystem->clanName[thisBlock.blockLoyalty].c_str(), CLAN_PRIVATE);
				recruit->act |= ACT_STAYPUT;
				recruit->mob->fallback = thisHood->dumpRoom;
				save_stayput_mobiles ();
				act("After much negotiations and settling with relatives, $N is ready to serve.", false, ch, 0, recruit, TO_CHAR | _ACT_FORMAT);
				act("After much negotiations and settling with relatives, $N is ready to serve.", false, ch, 0, recruit, TO_ROOM | _ACT_FORMAT);
				thisHood->saveHood(); // Save here so you don't cause crashes and get free turf members.
				return;
			}
			else
			{
				output += "You need #6" + MAKE_STRING(thisSystem->shopkeepFee) + "#0 chips for a shopkeep, and #6" + MAKE_STRING(thisSystem->soldierFee) + "#0 for a soldier.\n";
				send_to_char(output.c_str(), ch);
				extract_char(recruit);
				return;
			}
		}
	}
	else if (argOne == "dedicate")
	{
		std::string argTwo;
		OBJ_DATA *artObj = NULL;
		int clanId = -1;

		strArgument = one_argument(strArgument, argTwo);

		if (argTwo.empty() || strArgument.empty())
		{
			send_to_char("You need to nominate a piece of artwork you want to dedicate, and which Family you want to dedicate it to.\n", ch);
			return;
		}

		if (!(artObj = get_obj_in_list_vis (ch, argTwo.c_str(), ch->room->contents)))
		{
	        send_to_char ("You don't see that item in the room.\n", ch);
			return;
		}

		if (artObj->nVirtual != 5065 || !artObj->dec_desc || !artObj->dec_style)
		{
			send_to_char ("You can only dedicate a painted mural to a Family.\n", ch);
			return;
		}

		if (artObj->o.od.value[0] >= 0)
		{
			send_to_char ("This artwork has already been dedicated to a Family.\n", ch);
			return;
		}	

		for (int i = 0; i < TURF_CLANS; i ++)
		{
			if (thisSystem->clanName[i].empty())
				continue;

			if (strArgument == get_clandef(thisSystem->clanName[i].c_str())->literal)
			{
				clanId = i;
				break;
			}
		}

		if (strArgument.empty() || clanId == -1)
		{
			output.assign("You need to nominate the full name of the Family you wish to dedicate this piece of art to:\n\n");

			for (int i = 0; i < TURF_CLANS; i++)
			{
				if (thisSystem->clanName[i].empty())
					continue;

				output.append("#6");
				output.append(get_clandef(thisSystem->clanName[i].c_str())->literal);
				output.append("#0\n");
			}
			
			send_to_char(output.c_str(), ch);
			return;
		}

		output.assign("You dedicate $p to #6");
		output.append(get_clandef(thisSystem->clanName[clanId].c_str())->literal);
		output.append("#0.");
		act ((char *) output.c_str(), false, ch, artObj, 0, TO_CHAR | _ACT_FORMAT);

		output.assign("$n dedicate $p to #6");
		output.append(get_clandef(thisSystem->clanName[clanId].c_str())->literal);
		output.append("#0.");
		act ((char *) output.c_str(), false, ch, artObj, 0, TO_ROOM | _ACT_FORMAT);

		artObj->o.od.value[0] = clanId;
		artObj->o.od.value[1] = thisSystem->systemId;

		 if (artObj->dec_quality >= 75)
			 artObj->o.od.value[2] = 3;
		 else if (artObj->dec_quality >= 70)
			 artObj->o.od.value[2] = 2;
		 else if (artObj->dec_quality >= 40)
			 artObj->o.od.value[2] = 1;
		 else if (artObj->dec_quality >= 20)
			 artObj->o.od.value[2] = 0;
		 else
			 artObj->o.od.value[2] = -1;	

		return;
	}
	else if (!argOne.empty())	
	{
		if (thisBlock.blockType == turfTypeBase)
		{
			send_to_char ("There is nothing you can do to shake the loyalty of this turf.\n", ch);
			return;
		}

		CHAR_DATA *tch = NULL;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (ch == tch)
				continue;
			if (tch->delay_type == DEL_TURFACTION)
			{
				send_to_char("Someone is already conduction a turf action here: you'll need to wait until they're finished.\n", ch);
				return;
			}
		}

		int followers = 0;

		if (ch->following)
		{
			send_to_char ("You must be the leader to do anything on this block.\n", ch);
			return;
		}

		if ((followers = num_undelayed_followers(ch)) < 3)
		{
			send_to_char ("You need at least three ready and willing followers to do anything on this block.\n", ch);
			return;
		}

		if (get_affect (ch, MAGIC_TURF_DELAY) && IS_MORTAL (ch))
		{
			act	("Sorry, but your OOC delay timer is still in place. You'll receive a notification when it expires and you're free to influence turf.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		int char_clan = -1;
		char clan_name[MAX_STRING_LENGTH] = { '\0' };
		int clan_flags = 0;
		char *p = '\0';
		p = ch->clans;

		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			for (int i = 0; i < TURF_CLANS; i++)
			{
				if (thisSystem->clanName[i] == clan_name && clan_flags >= CLAN_PRIVATE)
				{
					if (char_clan == -1)
					{
						char_clan = i;
						continue;
					}
					else
					{
						send_to_char("Your loyalties are too conflicted to do that.\n", ch);
						return;
					}
				}
			}
		}

		if (char_clan == -1)
		{
			send_to_char("You don't have any loyalties strong enough to do that.\n", ch);
			return;
		}

		if (thisBlock.actioned[char_clan])
		{
			send_to_char("Your Family has already made an attempt to influence this piece of turf this Beat.\n", ch);
			return;
		}

		// If we don't have loyalty here, we need to check adjoining blocks
		// to see if we've got loyalty.
		if (char_clan != thisBlock.blockLoyalty)
		{
			bool linkedBlock = false;

			int dir;
			ROOM_DIRECTION_DATA *exit;
			ROOM_DATA *next_room;

			for (dir = 0; dir <= LAST_DIR; dir++)
			{
				if (!(exit = EXIT (ch, dir)) || !(next_room = vnum_to_room (exit->to_room)))
				{
					continue;
				}

				if (!next_room->hood)
					continue;

				Block tempBlock = next_room->hood->hoodBlocks.find(next_room->vnum)->second;

				if (tempBlock.blockLoyalty == char_clan)
				{
					linkedBlock = true;
					break;
				}
			}

			if (!linkedBlock)
			{
				send_to_char("You can only influence turf that is already loyal to your Family, or is adjacent to turf that is already loyal.\n", ch);
				return;
			}
		}

		if (argOne == "intimidate")
		{
			output.assign ("You order those following you to begin intimidating the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			send_to_char("\n#6OOC: This will take approximately 10 minutes to complete: be sure to roleplay it out.#0\n", ch);

			output.assign ("$n orders those following $m to intimidate the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

			ch->delay_type = DEL_TURFACTION;
			ch->delay = 45;
			ch->delay_info1 = 1;
			ch->delay_info2 = 3;
			return;
		}
		else if (argOne == "terrorise")
		{
			output.assign ("You order those following you to begin terrorising the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			send_to_char("\n#6OOC: This will take approximately 10 minutes to complete: be sure to roleplay it out.#0\n", ch);

			output.assign ("$n orders those following $m to terrorise the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

			ch->delay_type = DEL_TURFACTION;
			ch->delay = 45;
			ch->delay_info1 = 2;
			ch->delay_info2 = 3;
			return;
		}
		else if (argOne == "convert")
		{
			if (thisBlock.blockType == turfTypeHeart)
			{
				send_to_char ("Only fear can determine which Family rules the heart of the neighbourhood.\n", ch);
				return;
			}

			OBJ_DATA *propOne = NULL;
			OBJ_DATA *propTwo = NULL;
			int propCount = 0;

			if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TURF_PROP)
			{
				propOne = ch->right_hand;
				propCount = propOne->count;
			}

			if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TURF_PROP)
			{
				if (propOne)
				{
					propTwo = ch->left_hand;
					propCount = propTwo->count;
				}
				else
				{
					propOne = ch->left_hand;
					propCount = propOne->count;
				}
			}

			propCount = MIN(propCount, followers);
			propCount = MIN(propCount, 10);

			if (propCount < MIN(followers, 10))
			{
				act("You don't have enough ensignia for the amount of people following you: you need one piece per person in your group, including yourself.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			output.assign ("You order those following you to begin handing out $p the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, propOne, 0, TO_CHAR | _ACT_FORMAT);
			send_to_char("\n#6OOC: This will take approximately 10 minutes to complete: be sure to roleplay it out.#0\n", ch);

			output.assign ("$n orders those following $m to begin handing out $p people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, propOne, 0, TO_ROOM | _ACT_FORMAT);

			ch->delay_type = DEL_TURFACTION;
			ch->delay = 45;
			ch->delay_info1 = 3;
			ch->delay_info2 = 3;
			return;
		}
		else if (argOne == "charity")
		{
			if (thisBlock.blockType == turfTypeHeart)
			{
				send_to_char ("Only fear can determine which Family rules the heart of the neighbourhood.\n", ch);
				return;
			}

			OBJ_DATA *propOne = NULL;
			OBJ_DATA *propTwo = NULL;
			int propCount = 0;

			if (ch->right_hand && (ch->right_hand->nVirtual == 2201 ||
				ch->right_hand->nVirtual == 2202 ||
				ch->right_hand->nVirtual == 2210 ||
				ch->right_hand->nVirtual == 2211 ||
				ch->right_hand->nVirtual == 2212))
			{
				propOne = ch->right_hand;
				propCount = propOne->count;
			}

			if (ch->left_hand && (ch->left_hand->nVirtual == 2201 ||
				ch->left_hand->nVirtual == 2202 ||
				ch->left_hand->nVirtual == 2210 ||
				ch->left_hand->nVirtual == 2211 ||
				ch->left_hand->nVirtual == 2212))
			{
				if (propOne)
				{
					propTwo = ch->left_hand;
					propCount = propTwo->count;
				}
				else
				{
					propOne = ch->left_hand;
					propCount = propOne->count;
				}
			}

			propCount = MIN(propCount, followers);
			propCount = MIN(propCount, 10);

			if (propCount < MIN(followers, 10))
			{
				act("You don't have enough food for the amount of people following you: you need one packet per person in your group, including yourself.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				return;
			}

			output.assign ("You order those following you to begin handing out $p to the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, propOne, 0, TO_CHAR | _ACT_FORMAT);
			send_to_char("\n#6OOC: This will take approximately 10 minutes to complete: be sure to roleplay it out.#0\n", ch);

			output.assign ("$n orders those following $m to begin handing out $p to the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, propOne, 0, TO_ROOM | _ACT_FORMAT);

			ch->delay_type = DEL_TURFACTION;
			ch->delay = 45;
			ch->delay_info1 = 4;
			ch->delay_info2 = 3;
			return;
		}
		else if (argOne == "volunteer")
		{
			if (thisBlock.blockType == turfTypeHeart)
			{
				send_to_char ("Only fear can determine which Family rules the heart of the neighbourhood.\n", ch);
				return;
			}

			if (thisBlock.blockLoyalty != char_clan)
			{
				send_to_char ("The people living here do not want your volunteer efforts: only turf loyal to you can be swayed with such a paltry offering.", ch);
				return;
			}

			output.assign ("You order those following you to begin volunteering and assisting the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			send_to_char("\n#6OOC: This will take approximately 10 minutes to complete: be sure to roleplay it out.#0\n", ch);

			output.assign ("$n orders those following $m to volunteer and assisting the people living in this part of #6");
			output.append (thisHood->name);
			output.append ("#0.");

			act ((char *) output.c_str(), false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

			ch->delay_type = DEL_TURFACTION;
			ch->delay = 45;
			ch->delay_info1 = 5;
			ch->delay_info2 = 3;
			return;
		}
		else
		{
			output.assign("You can do the following actions here:\n");
			output.append("#6turf gossip#0:   find out who is in power, and who isn't, in this part of turf.\n");
			output.append("#6turf track#0:    ask a loyal Block whether members of other Families have been passing by lately.\n");
			output.append("#6turf dedicate#0: dedicate a piece of artwork here to a Family.\n\n");
			output.append("The following commands require you to be ranked at least Private in one of the Families.\n");
			output.append("You will need to be leading a group of at least three followers to perform these actions.\n\n");
			output.append("#6turf intimidate#0: Use simple thuggery to increase your Family's fear at the expense of love.\n");
			output.append("#6turf terrorise#0:  Deploy shocking violence to greatly increase fear at a great cost to love.\n");
			output.append("#6turf convert#0:    Hand out Family propaganda to win some love.\n");
			output.append("#6turf charity#0:    Hand out Family food packages to win some love and decrease fear.\n");
			output.append("#6turf volunteer#0:  Work up a sweat committing good works for turf already loyal to your Family.\n\n");
			output.append("The following commands can only be done by Sergeants/Journeyman on the Heart Block of a controlled Neighbourhood:\n");
			output.append("#6turf recruit#0:    Recruit either a shopkeep or a soldier. See HELP TURF-RECRUIT for a full syntax.\n\n");
			output.append("#6turf improve#0:    Add improvements to the Heart Block. See HELP TURF-IMPROVEMENT for more details.\n\n");
			output.append("#6turf order#0:      Set your Families policies towards the local inhabitants of the Neighbourhood.\n\n");		
			send_to_char(output.c_str(), ch);
			return;
		}
	}
	else
	{
		output.assign("You can do the following actions here:\n");
		output.append("#6turf gossip#0:   find out who is in power, and who isn't, in this part of turf.\n");
		output.append("#6turf track#0:    ask a loyal Block whether members of other Families have been passing by lately.\n");
		output.append("#6turf dedicate#0: dedicate a piece of artwork here to a Family.\n\n");
		output.append("The following commands require you to be ranked at least Private in one of the Families.\n");
		output.append("You will need to be leading a group of at least three followers to perform these actions.\n\n");
		output.append("#6turf intimidate#0: Use simple thuggery to increase your Family's fear at the expense of love.\n");
		output.append("#6turf terrorise#0:  Deploy shocking violence to greatly increase fear at a great cost to love.\n");
		output.append("#6turf convert#0:    Hand out Family propaganda to win some love.\n");
		output.append("#6turf charity#0:    Hand out Family food packages to win some love and decrease fear.\n");
		output.append("#6turf volunteer#0:  Work up a sweat committing good works for turf already loyal to your Family.\n\n");		
		output.append("The following commands can only be done by sergeants/journeyman on an owned Heart Block in an owned Neighbourhood:\n");
		output.append("#6turf recruit#0:    Recruit either a shopkeep or a soldier. See HELP TURF-RECRUIT for a full syntax.\n\n");

		send_to_char(output.c_str(), ch);
		return;
	}
}

void
	delayed_turfaction (CHAR_DATA *ch)
{
	Neighbourhood *thisHood = ch->room->hood;

	if (!thisHood)
	{
		send_to_char("You can no longer do that here.\n", ch);
		return;
	}

	Block thisBlock = thisHood->hoodBlocks.find(ch->room->vnum)->second;

	TurfSystem *thisSystem = NULL;

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == thisHood->systemId)
		{
			thisSystem = (*it);
			break;
		}
	}

	if (!thisSystem)
	{
		send_to_char("You can no longer do that here.\n", ch);
		return;
	}

	int followers = 1;

	if (ch->following)
	{
		send_to_char ("You must be the leader to do anything on this block.\n", ch);
		return;
	}

	if ((followers = num_undelayed_followers(ch)) < 3)
	{
		send_to_char ("You need at least three ready and willing followers to do anything on this block.\n", ch);
		return;
	}

	if (get_affect (ch, MAGIC_TURF_DELAY) && IS_MORTAL (ch))
	{
		act	("Sorry, but your OOC delay timer is still in place. You'll receive a notification when it expires and you're free to influence turf.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	int char_clan = -1;
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	int clan_flags = 0;
	char *p = '\0';
	p = ch->clans;

	while (get_next_clan (&p, clan_name, &clan_flags))
	{
		for (int i = 0; i < TURF_CLANS; i++)
		{
			if (thisSystem->clanName[i] == clan_name && clan_flags >= CLAN_PRIVATE)
			{
				if (char_clan == -1)
				{
					char_clan = i;
					continue;
				}
				else
				{
					send_to_char("Your loyalties are too conflicted to do that.\n", ch);
					return;
				}
			}
		}
	}

	if (char_clan == -1)
	{
		send_to_char("You loyalties aren't strong enough anymore to do that.\n", ch);
		return;
	}

	OBJ_DATA *propOne = NULL;
	OBJ_DATA *propTwo = NULL;
	int propCount = 0;

	if (ch->delay_info1 == 3)
	{
		if (ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_TURF_PROP)
		{
			propOne = ch->right_hand;
			propCount = propOne->count;
		}

		if (ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_TURF_PROP)
		{
			if (propOne)
			{
				propTwo = ch->left_hand;
				propCount = propTwo->count;
			}
			else
			{
				propOne = ch->left_hand;
				propCount = propOne->count;
			}
		}

		propCount = MIN(propCount, followers);
		propCount = MIN(propCount, 10);

		if (propCount < MIN(followers, 10))
		{
			act("You don't have enough insignia for the amount of people following you: you need one piece per person in your group, including yourself.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}
	else if (ch->delay_info1 == 4)
	{
		if (ch->right_hand && (ch->right_hand->nVirtual == 2201 ||
			ch->right_hand->nVirtual == 2202 ||
			ch->right_hand->nVirtual == 2210 ||
			ch->right_hand->nVirtual == 2211 ||
			ch->right_hand->nVirtual == 2212))
		{
			propOne = ch->right_hand;
			propCount = propOne->count;
		}

		if (ch->left_hand && (ch->left_hand->nVirtual == 2201 ||
			ch->left_hand->nVirtual == 2202 ||
			ch->left_hand->nVirtual == 2210 ||
			ch->right_hand->nVirtual == 2211 ||
			ch->left_hand->nVirtual == 2212))
		{
			if (propOne)
			{
				propTwo = ch->left_hand;
				propCount = propTwo->count;
			}
			else
			{
				propOne = ch->left_hand;
				propCount = propOne->count;
			}
		}

		propCount = MIN(propCount, followers);
		propCount = MIN(propCount, 10);

		if (propCount < MIN(followers, 10))
		{
			act("You don't have enough food for the amount of people following you: you need one packet per person in your group, including yourself.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}

	if (ch->delay_info2)
	{
		std::string chOutput;
		std::string roomOutput;

		if (ch->delay_info1 == 1)
		{
			chOutput.assign("You continue to lead the intimidation of this part of #6");
			chOutput.append(thisHood->name);
			chOutput.append("#0.");

			roomOutput.assign("$n continues to lead the intimidation of this part of #6");
			roomOutput.append(thisHood->name);
			roomOutput.append("#0.");
		}
		if (ch->delay_info1 == 2)
		{
			chOutput.assign("You continue to lead the terrorising of this part of #6");
			chOutput.append(thisHood->name);
			chOutput.append("#0.");

			roomOutput.assign("$n continues to lead the terrorising of this part of #6");
			roomOutput.append(thisHood->name);
			roomOutput.append("#0.");
		}
		if (ch->delay_info1 == 3)
		{
			chOutput.assign("You continue to lead the conversion of this part of #6");
			chOutput.append(thisHood->name);
			chOutput.append("#0.");

			roomOutput.assign("$n continues to lead the conversion of this part of #6");
			roomOutput.append(thisHood->name);
			roomOutput.append("#0.");
		}
		if (ch->delay_info1 == 4)
		{
			chOutput.assign("You continue to lead the charity work in this part of #6");
			chOutput.append(thisHood->name);
			chOutput.append("#0.");

			roomOutput.assign("$n continues to lead the charity work in this part of #6");
			roomOutput.append(thisHood->name);
			roomOutput.append("#0.");
		}
		if (ch->delay_info1 == 5)
		{
			chOutput.assign("You continue to lead the volunteer work in this part of #6");
			chOutput.append(thisHood->name);
			chOutput.append("#0.");

			roomOutput.assign("$n continues to lead the volunteer work in this part of #6");
			roomOutput.append(thisHood->name);
			roomOutput.append("#0.");
		}

		act ((char *) chOutput.c_str(), false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ((char *) roomOutput.c_str(), false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);		
		ch->delay_info2 --;
		ch->delay = 180;
		return;
	}
	else
	{
		thisBlock.actioned[char_clan] = ch->delay_info1;
		switch (ch->delay_info1)
		{

		case 1:
			// Intimidation: fear goes up by PC following *.5, love goes by PC following *.25		
			thisBlock.scores[char_clan][FEAR] += MIN(followers / 2, 5) + 1;
			thisBlock.scores[char_clan][LOVE] -= MIN(followers / 4, 3);
			thisBlock.actionScore[char_clan] += MIN(followers / 2, 10);
			break;
		case 2:
			// Terrorise: fear goes up by PC following *1.0, but love goes down by
			// PC following * .75
			thisBlock.scores[char_clan][FEAR] += MIN(followers, 10) + 1;
			thisBlock.scores[char_clan][LOVE] -= MIN(followers * 3 / 4, 8);
			thisBlock.actionScore[char_clan] += MIN(followers, 10);

			// We also sap the clan who owns this block a bit for letting some awful
			// stuff happen on their turf.
			thisBlock.scores[thisBlock.blockLoyalty][LOVE] -= MIN(followers / 4, 3);
			break;
		case 3: 
			// Convert: loves goes up by * .50 per PC, and also saps the owner a bit.
			thisBlock.scores[char_clan][LOVE] += MIN(followers / 2, 5) + 1;
			thisBlock.actionScore[char_clan] += MIN(followers, 10);

			// We also sap the clan who owns this block a bit: we love these other
			// guys more now as they gave us cool livery.

			if (char_clan != thisBlock.blockLoyalty)
			{
				thisBlock.scores[thisBlock.blockLoyalty][LOVE] -= MIN(followers / 4, 3);
			}

			if (propOne)
			{
				obj_from_char(&propOne, MIN(propOne->count, propCount));
				propCount -= propOne->count;
				extract_obj(propOne);
			}
			if (propTwo)
			{
				obj_from_char(&propTwo, MIN(propTwo->count, propCount));
				propCount -= propTwo->count;
				extract_obj(propTwo);
			}

			break;
		case 4: 
			// Charity: loves goes up by * 1.00 per PC, but saps our fear by 0.25.
			thisBlock.scores[char_clan][LOVE] += MIN(followers, 10) + 1;
			thisBlock.scores[char_clan][FEAR] -= MIN(followers / 4, 3);
			thisBlock.actionScore[char_clan] += MIN(followers, 10);

			// We also sap the clan who owns this block a bit: we love these other
			// guys more now as they gave us free food.
			if (char_clan != thisBlock.blockLoyalty)
			{
				thisBlock.scores[thisBlock.blockLoyalty][LOVE] -= MIN(followers / 4, 3);
			}
			// However, if we are doing charity for ourself, then we don't impinge our
			// fear at all.
			else
			{
				thisBlock.scores[thisBlock.blockLoyalty][FEAR] += MIN(followers / 4, 3);
			}

			if (propOne)
			{
				obj_from_char(&propOne, MIN(propOne->count, propCount));
				propCount -= propOne->count;
				extract_obj(propOne);
			}
			if (propTwo)
			{
				obj_from_char(&propTwo, MIN(propTwo->count, propCount));
				propCount -= propTwo->count;
				extract_obj(propTwo);
			}

			break;
		case 5: 
			// Volunteering: loves goes up by * 0.25 per PC -- it's pretty pathetic.
			thisBlock.scores[char_clan][LOVE] += MIN(followers / 4, 3) + 1;
			break;
		}

		// No more turf for 12 RL hours. Oh noes!
		magic_add_affect (ch, MAGIC_TURF_DELAY, -1, (time (0) + 12 * 60 * 60), 0, 0, 0);

		CHAR_DATA *tch = NULL;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (tch->following != ch)
				continue;
			if (get_affect (tch, MAGIC_TURF_DELAY))
				continue;

			// Add turf to PCs, hooray.
			magic_add_affect (tch, MAGIC_TURF_DELAY, -1, (time (0) + 12 * 60 * 60), 0, 0, 0);
		}

		act ("You finish leading the action here.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act ("$n finishes leading the actions here.", false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

		// Now we update and save our block.
		thisHood->hoodBlocks.find(ch->room->vnum)->second = thisBlock;

		if (engine.in_play_mode ())
		{
			char buf[MAX_STRING_LENGTH] = {'\0'};	
			std::string world_log_db = engine.get_config ("world_log_db");
			sprintf(buf, "DELETE FROM %s.turfblocks WHERE room = %d", world_log_db.c_str(), thisBlock.turfRoom);
			mysql_safe_query(buf);
			thisHood->hoodBlocks.find(ch->room->vnum)->second.saveBlock();
		}

		return;
	}
}


void Neighbourhood::loadEnforcers(CHAR_DATA *ch, CHAR_DATA *target)
{	
	if (!ch || !target)
		return;

	TurfSystem *thisSystem = NULL;

	// First we get our Block and our Neighbourhood.

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == systemId)
		{
			thisSystem = (*it);
			break;
		}
	}

	Block thisBlock = this->hoodBlocks.find(ch->room->vnum)->second;

	if (!thisSystem)
		return;

	// No spawning to help you out with wildlife.
	if (IS_NPC(ch) && IS_SET(ch->act, ACT_WILDLIFE))
		return;

	// If we're not in a block that's loyal to the hood, or in a disputed block, return.
	if (thisBlock.blockLoyalty != this->hoodLoyalty || thisBlock.blockLoyalty == -1)
	{
		return;
	}


	int score = 0;

	if (thisBlock.blockType == turfTypeHeart)
	{
		score = thisBlock.scores[thisBlock.blockLoyalty][FEAR] * 2;
	}
	else
	{
		score = thisBlock.scores[thisBlock.blockLoyalty][FEAR] + thisBlock.scores[thisBlock.blockLoyalty][LOVE];
	}

	// If you've got barely any grip, then people don't help out.
	if (score <= 5)
	{
		return;
	}

	// If our target isn't a member of the clan this block is loyal to, return.
	if (!is_clan_member(target, (char *) thisSystem->clanName[thisBlock.blockLoyalty].c_str()))
	{
		return;
	}

	// If the person is, then return.
	if (is_clan_member(ch, (char *) thisSystem->clanName[thisBlock.blockLoyalty].c_str()))
	{
		return;
	}

	// If we can't load up our mobile, then return.
	if (!thisSystem->enforceMob || !vnum_to_mob(thisSystem->enforceMob))
	{
		return;
	}

	// If we find an enforcer is already here, then return.
	CHAR_DATA *tch = NULL;
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->mob && tch->mob->vnum == thisSystem->enforceMob)
		{
			return;
		}
	}

	// We now load up this mobile, and add our aggressor to our memory, and clan us to the aggrieved turf.
	// Shit just got real.
	tch = load_mobile(thisSystem->enforceMob);
	char_to_room(tch, ch->in_room);
	add_clan (tch, (char *) thisSystem->clanName[thisBlock.blockLoyalty].c_str(), CLAN_LEADER);

    if (IS_SET (tch->act, ACT_MEMORY))
	{
		add_memory (ch, tch);
	}

	act("$n arrives on the scene.", false, tch, 0, 0, TO_ROOM | _ACT_FORMAT);

}

void Neighbourhood::reportIncident(CHAR_DATA *ch, CHAR_DATA *target, int crime)
{

	if (IS_NPC(target) && IS_SET(target->act, ACT_WILDLIFE))
	{
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_WILDLIFE))
	{
		return;
	}

	char *date;
	time_t time_now;
	char msg[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA *board = NULL;
	std::string output, messageName;

	TurfSystem *thisSystem = NULL;

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == systemId)
		{
			thisSystem = (*it);
			break;
		}
	}

    if (get_affect(ch, MAGIC_TURF_REPORT) ||
		get_affect(target, MAGIC_TURF_REPORT))
		return;

	if ((board = vtoo(thisSystem->boardVnum)))
	{
		if (IS_MORTAL (ch))
		{
			time_now = time (0);
			date = (char *) asctime (localtime (&time_now));
			date[strlen (date) - 1] = '\0';

			if (crime == CRIME_KILL || crime == CRIME_ASSAULT)
				sprintf (msg,
				"Over by #6%s#0 in #6%s#0,\n#5%s#0 was seen attacking #5%s#0.\n",
				ch->room->name, name.c_str(), char_short(ch), char_short(target));
			else if (crime == CRIME_STEAL)
				sprintf (msg,
				"Over by #6%s#0 in #6%s#0,\n#5%s#0 was seen stealing from #5%s#0.\n",
				ch->room->name, name.c_str(), char_short(ch), char_short(target));
			else if (crime == CRIME_SHOOT)
				sprintf (msg,
				"Over by #6%s#0 in #6%s#0,\n#5%s#0 was seen shooting at #5%s#0.\n",
				ch->room->name, name.c_str(), char_short(ch), char_short(target));
			else 
				sprintf (msg, 
				"Over by #6%s#0 in #6%s#0,\n#5%s#0 was seen doing something shady.\n",
				ch->room->name, name.c_str(), char_short(ch));

			output.assign(msg);

			if (is_hooded(ch))
			{
				if (ch->d_age)
				{
					if (ch->race == lookup_race_id("Cybernetic") || ch->race == lookup_race_id("Mutation"))
					{
						sprintf(buf, "\nThe identify of aggressor remains a mystery, but witnesses reported\nthat %s is #2%s#0 and has a #2%s#0 build, and is #2%s#0 and #2%s#0.", 
							HSHR(ch), ch->height >= 71 ? height_phrase(ch) : ch->height <= 59 ? height_phrase(ch) : "ordinary in height", frames[ch->frame], ch->d_feat1, ch->d_feat2);
					}
					else
					{
						sprintf(buf, "\nThe identify of aggressor remains a mystery, but witnesses reported\nthat %s is #2%s#0 and has a #2%s#0 build, and has #2%s#0 eyes.", 
							HSHR(ch), ch->height >= 71 ? height_phrase(ch) : ch->height <= 59 ? height_phrase(ch->delay_ch) : "ordinary in height", frames[ch->frame], ch->d_eyes);
					}
				}
				else
				{
					sprintf(buf, "\nThe witnesses had nothing to say as to the identity of the aggressor.\n");
				}

				output.append(buf);
			}
						
			messageName.assign("#2Excitement in ");
			messageName.append(name);
			messageName.append("#0");

			post_straight_to_mysql_board(fname((board)->name), (char *) messageName.c_str(), "The-Streets", (char *) output.c_str());
			magic_add_affect (ch, MAGIC_TURF_REPORT, 300, 0, 0, 0, 0);
			magic_add_affect (target, MAGIC_TURF_REPORT, 60, 0, 0, 0, 0);
		}
	}
}

void Block::sufferViolence(int clanId)
{
	// If we've already had violence, don't worry about anymore.
	if (violence)
		return;

	if (clanId != -1)
	{
		scores[clanId][LOVE] -= 5;
		violence = 1;

		if (blockLoyalty != -1)
		{
			if (clanId == blockLoyalty)
			{
				scores[blockLoyalty][LOVE] -= 5;
			}
			else
			{
				scores[blockLoyalty][LOVE] -= 2;
			}
		}
	}

	if (engine.in_play_mode ())
	{
		char buf[MAX_STRING_LENGTH] = {'\0'};	
		std::string world_log_db = engine.get_config ("world_log_db");
		sprintf(buf, "DELETE FROM %s.turfblocks WHERE room = %d", world_log_db.c_str(), turfRoom);
		mysql_safe_query(buf);
		saveBlock();
	}
}

void TurfSystem::beatReport (void)
{
	std::string output;
	OBJ_DATA *board = NULL;

	vector<Neighbourhood*>::iterator it;
	std::map<int, Block>::iterator nit;
	bool anyworthwhile = false;

	if ((board = vtoo(boardVnum)))
	{
		output.assign ("From the urchins, you learn about the latest gossip of what's happened lately...");

		for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
		{
			if ((*it)->systemId == systemId)
			{
				int activity[TURF_CLANS] = {0, 0, 0, 0, 0};
				bool worthwhile = false;
				int violence = 0;

				for (nit = (*it)->hoodBlocks.begin(); nit != (*it)->hoodBlocks.end(); nit++)
				{
					for (int i = 0; i < TURF_CLANS; i ++)
					{
						activity[i] += nit->second.actionScore[i] / 2;

						violence += nit->second.violence;

						if (activity[i] >= 2 || violence >= 2)
						{
							worthwhile = true;
							anyworthwhile = true;
						}
					}
				}	

				if (worthwhile)
				{
					output.append(MAKE_STRING("\n\nIn #6" + (*it)->name + "#0:"));

					if (violence >= 6)
					{
						output.append("\nThere was an extreme amount of violence."); 
					}
					else if (violence >= 4)
					{
						output.append("\nThere were many instances of violence."); 
					}
					else if (violence >= 2)
					{
						output.append("\nThere was a few, notable incidences of violence."); 
					}

					for (int i = 0; i < TURF_CLANS; i ++)
					{
						if (clanName[i].empty())
							continue;

						if (activity[i] < 2)
							continue;

						output.append("\n#6");
						output.append(get_clandef(clanName[i].c_str())->literal);
						output.append("#0 were ");
						if (activity[i] >= 15)
						{
							output.append("extremely active"); 
						}
						else if (activity[i] >= 10)
						{
							output.append("very active"); 
						}
						else if (activity[i] >= 5)
						{
							output.append("active"); 
						}
						else
						{
							output.append("only marginally active"); 
						}
						output.append(".");
					}
				}
			}
		}

		if (anyworthwhile)
		{
			output.append("#0\n\n");
		}
		else
		{
			output.append("\n\n...and that's nothing. Nothing interesting lately in Grungetown.\n\n");
		}

		post_straight_to_mysql_board(fname((board)->name), "#3The latest news from the streets#0", "The-Streets", (char *) output.c_str());
	}
}

void TurfSystem::turnReport (void)
{
	std::string output;
	OBJ_DATA *board = NULL;
	int hoods[TURF_CLANS] = {0, 0, 0, 0, 0};
	int blocks[TURF_CLANS] = {0, 0, 0, 0, 0};
	int love[TURF_CLANS] = {0, 0, 0, 0, 0};
	int fear[TURF_CLANS] = {0, 0, 0, 0, 0};
	int blockCount[TURF_CLANS] = {0, 0, 0, 0, 0};

	vector<Neighbourhood*>::iterator it;
	std::map<int, Block>::iterator nit;
	for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
	{
		if ((*it)->systemId == systemId)
		{
			hoods[(*it)->hoodLoyalty] ++;
			for (nit = (*it)->hoodBlocks.begin(); nit != (*it)->hoodBlocks.end(); nit++)
			{						
				blocks[nit->second.blockLoyalty] ++;
				blockCount[(*it)->hoodLoyalty] ++;

				love[(*it)->hoodLoyalty] += nit->second.scores[(*it)->hoodLoyalty][LOVE];
				fear[(*it)->hoodLoyalty] += nit->second.scores[(*it)->hoodLoyalty][FEAR];				
			}	
		}
	}


	if ((board = vtoo(boardVnum)))
	{
		output.assign ("From the folk gathering about, you learn the current state of power in Grungetown:");

		for (int i = 0; i < TURF_CLANS; i++)
		{
			if (clanName[i].empty())
				continue;

			if (!blockCount[i])
				continue;

			output.append("\n\n#6");
			output.append(get_clandef(clanName[i].c_str())->literal);
			output.append("#0 are:");

			love[i] = love[i]/blockCount[i];
			fear[i] = fear[i]/blockCount[i];

			output.append ("\n   - in control of #6" + MAKE_STRING(hoods[i]) + "#0 neighbourhoods.");
			output.append ("\n   - command the loyalty of #6" + MAKE_STRING(blocks[i]) + "#0 blocks.");
			output.append ("\n   - are#2");

			if (love[i] >= 35)
			{
				output.append(" blindingly loved");
			}
			else if (love[i] >= 25)
			{
				output.append(" extremely loved");
			}
			else if (love[i] >= 15)
			{
				output.append(" loved");
			}
			else if (love[i] >= 10)
			{
				output.append(" well thought of");
			}
			else if (love[i] >= 5)
			{
				output.append(" liked");
			}
			else if (love[i] >= -4)
			{
				output.append(" neither loved nor loathed");
			}
			else if (love[i] >= -14)
			{
				output.append(" actively disliked");
			}
			else
			{
				output.append(" loathed");
			}

			output.append("#0 in the neighbourhoods they control.");

			output.append ("\n   -");

			if (fear[i] >= 35)
			{
				output.append(" #3inspire absolute terror");
			}
			else if (fear[i] >= 25)
			{
				output.append(" #3are outright feared");
			}
			else if (fear[i] >= 15)
			{
				output.append(" #3are feared");
			}
			else if (fear[i] >= 10)
			{
				output.append(" #3are given significant respect");
			}
			else if (fear[i] >= 5)
			{
				output.append(" #3are to be wary of");
			}
			else
			{
				output.append(" #3are not particularly feared");
			}

			output.append("#0 in the neighbourhoods they control.");
		}

		output.append("#0\n\n");

		post_straight_to_mysql_board(fname((board)->name), "#6The state of play#0", "The-Streets", (char *) output.c_str());
	}
}

int is_turf_clan (char *clan_name, bool include_willies)
{
	// If we're including the willies, make note of it here.
	if (include_willies && !str_cmp("wilmingtons", clan_name))
		return 1;

	TurfSystem *thisSystem = NULL;

	vector<TurfSystem*>::iterator it;
	for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
	{
		if ((*it)->systemId == 0)
		{
			thisSystem = (*it);
			break;
		}
	}

	if (!thisSystem)
		return 0;

	for (int i = 0; i < TURF_CLANS; i++)
	{
		if (thisSystem->clanName[i] == clan_name)
		{
			return 1;
		}
	}

	return 0;
}


// Immturf: on its own displays some numeric data not available in turf gossip
// Immturf system # - shows info on the system (how many clans, neighbourhoods, who owns them)
// Immturf neighbour # - shows info on the neighbourhood (who owns, the heart, how many blocks, averages)
// Immturf block edit love|fear|type 

void do_immturf(CHAR_DATA *ch, char* argument, int cmd)
{
	std::string output, strArgument = argument, argOne;

	strArgument = one_argument(strArgument, argOne);

	if (argOne == "system")
	{
		std::string argTwo;
		strArgument = one_argument(strArgument, argTwo);
		int number = 0;
		
		number = atoi(argTwo.c_str());

        	if (number <= 0)
		    {
			    send_to_char("You need to input a positive integer of the system you want to view.\n", ch);
			    return;
		    }
		    number -= 1;
            
		    TurfSystem *thisSystem = NULL;

            vector<TurfSystem*>::iterator it;
		    for (it = TurfSystems.begin(); it != TurfSystems.end(); it++)
		    {
			    if ((*it)->systemId == number)
			    {
				    thisSystem = (*it);
				    break;
			    }
		    }
            
            if (thisSystem == NULL)
		    {
			    send_to_char("No such system.\n", ch);
			    return;
		    }

		    output += "Info for Turf System " + MAKE_STRING(number + 1) +  "\n";
		    output += "The following clans are turf-clans:\n";
		    
		    for (int i = 0; i < TURF_CLANS; i++)
		    {
			    if (!thisSystem->clanName[i].empty())
			    {
				    output += "#6" + MAKE_STRING(i) + "#0: " + thisSystem->clanName[i] + "\n";
			    }
		    }

		    
		    output += "\n";
		    output += "BoardObj: " + MAKE_STRING(thisSystem->boardVnum) + ", LoveResource: " + MAKE_STRING(thisSystem->dumpVnum1) + ", FearResource: " + MAKE_STRING(thisSystem->dumpVnum2) + ", EnforcerMob: " + MAKE_STRING(thisSystem->enforceMob) + "\n";
		    output += "ShopkeepMob: " + MAKE_STRING(thisSystem->shopkeepMob) + ", ShopkeepFee: " + MAKE_STRING(thisSystem->shopkeepFee) + ", ";
		    output += "SoldierMob: " + MAKE_STRING(thisSystem->soldierMob) + ", SoldierFee: " + MAKE_STRING(thisSystem->soldierFee) + "\n";

		    output += "\n";
		    output += "This System has the following neighbourhoods:\n";
		    
		    Neighbourhood *thisHood = NULL;

		    vector<Neighbourhood*>::iterator nit;
		    for (nit = Neighbourhoods.begin(); nit != Neighbourhoods.end(); nit++)
		    {
			    if ((*nit)->systemId == number)
			    {
				   output += "#6" + MAKE_STRING((*nit)->hoodId) + ": " + (*nit)->name + "#0, owned by #6" + thisSystem->clanName[(*nit)->hoodLoyalty] + "#0\n";
			    }
		    }
  
		    send_to_char(output.c_str(), ch);
		    return;
	}
	else if (argOne == "hood")
	{
		std::string argTwo;
		strArgument = one_argument(strArgument, argTwo);
		int number = 0;

		number = atoi(argTwo.c_str());
		if (number <= 0)
		{
			send_to_char("You need to input a positive integer of the neighbourhood "
			    "you want to view.\n", ch);
			return;
		}
        number -= 1;
        
		Neighbourhood *thisHood = NULL;

		vector<Neighbourhood*>::iterator it;
		for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
		{
			if ((*it)->hoodId == number)
			{
				thisHood = (*it);
				break;
			}
		}

		if (!thisHood)
		{
			send_to_char("No such hood.\n", ch);
			return;
		}

		TurfSystem *thisSystem;		

		vector<TurfSystem*>::iterator nit;
		for (nit = TurfSystems.begin(); nit != TurfSystems.end(); nit++)
		{
			if ((*nit)->systemId == thisHood->systemId)
			{
				thisSystem = (*nit);
				break;
			}
		}

		if (!thisSystem)
		{
			send_to_char("No such system.\n", ch);
		}

		output += "Info for Turf Hood " + MAKE_STRING(number) +  ", " + thisHood->name + "\n";
		output += "Owned by #6" + thisSystem->clanName[thisHood->hoodLoyalty] + "#0, ";
		output += "DumpRoom: #6" + MAKE_STRING(thisHood->dumpRoom) + "#0.\n";

		if (thisHood->npc_coldload_first)
		{
			CHAR_DATA *npc = NULL;
			if ((npc = get_char_id(thisHood->npc_coldload_first)))
			{
				output += "NPC One: " + MAKE_STRING(thisHood->npc_coldload_first)+ ", #5" + char_short(npc) + "#0.\n";
			}
			else
			{
				output += "NPC One: " + MAKE_STRING(thisHood->npc_coldload_first)+ ", but NPC not found - likely dead.\n";
			}
		}
		else
		{
			output += "No NPC One.\n";
		}

		if (thisHood->npc_coldload_second)
		{
			CHAR_DATA *npc = NULL;
			if ((npc = get_char_id(thisHood->npc_coldload_second)))
			{
				output += "NPC Two: " + MAKE_STRING(thisHood->npc_coldload_second)+ ", #5" + char_short(npc) + "#0.\n";
			}
			else
			{
				output += "NPC Two: " + MAKE_STRING(thisHood->npc_coldload_second)+ ", but NPC not found - likely dead.\n";
			}
		}	
		else
		{
			output += "No NPC Two.\n";
		}

		send_to_char(output.c_str(), ch);
	}
	else if (argOne == "block")
	{
		if (!ch->room->hood)
		{
			send_to_char("You need to be in a turf-room for this command to work.\n", ch);
			return;
		}

		Block thisBlock = ch->room->hood->hoodBlocks.find(ch->room->vnum)->second;

		TurfSystem *thisSystem;		

		vector<TurfSystem*>::iterator nit;
		for (nit = TurfSystems.begin(); nit != TurfSystems.end(); nit++)
		{
			if ((*nit)->systemId == ch->room->hood->systemId)
			{
				thisSystem = (*nit);
				break;
			}
		}

		if (!thisSystem)
		{
			send_to_char("No such system.\n", ch);
		}

		std::string argTwo;
		strArgument = one_argument(strArgument, argTwo);

		if (argTwo.empty())
		{
			output += "Scores for this block:\n\n";

			for (int i = 0; i < TURF_CLANS; i++)
			{
				if (!thisSystem->clanName[i].empty())
				{
					output += "#6" + MAKE_STRING(i) + "#0: " + thisSystem->clanName[i] + " - \n";
					output += "Love: " + MAKE_STRING(thisBlock.scores[i][LOVE]) + ", ";
					output += "Fear: " + MAKE_STRING(thisBlock.scores[i][FEAR]) + ", ";
					output += "Actions: " + MAKE_STRING(thisBlock.actioned[i]) + " / " + MAKE_STRING(thisBlock.actionScore[i]) + ".\n\n";
				}
			}

			send_to_char(output.c_str(), ch);
		}
	}
	else if (argOne == "beat")
	{
	    std::string argTwo;
	    std::string argThree;
		strArgument = one_argument(strArgument, argTwo);
		int number = 0;

		number = atoi(argTwo.c_str());
		if (number <= 0)
		{
			send_to_char("You need to input a positive integer of the neighbourhood "
			    "you want to view.\n", ch);
			return;
		}
        number -= 1;
		Neighbourhood *thisHood = NULL;

		vector<Neighbourhood*>::iterator it;
		for (it = Neighbourhoods.begin(); it != Neighbourhoods.end(); it++)
		{
			if ((*it)->hoodId == number)
			{
				thisHood = (*it);
				break;
			}
		}
		
        if (!thisHood)
		{
			send_to_char("No such hood.\n", ch);
		}
		
        strArgument = one_argument(strArgument, argThree);
        if (argThree == "true") 
        {
            send_to_char("You update the Turn of the neighbourhood\n", ch);
	        (*it)->updateBeats(true);
	    } 
	    else if (argThree == "false")
	    {
	        send_to_char("You update the Beat of the neighbourhood \n", ch);
	        (*it)->updateBeats(false);	    
	    }
	    else
	    {
	        send_to_char("Your third argument must be TRUE or FALSE for the Turn.\n", ch);
	        return;
	    }
	}
	else
	{
		send_to_char("Syntax: turf system|hood|block|beat arguments\n", ch);
	}
}
