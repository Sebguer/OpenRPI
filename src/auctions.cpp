//////////////////////////////////////////////////////////////////////////////
//
/// auctions.cpp - Shadows of Isildur's automated auction house routines
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


#define AUCTION_WHAT		"What exactly did you wish to list for auction?"
#define NO_SUCH_ITEM		"Perhaps you are mistaken - I do not see the merchandise you are referring to."
#define ILLEGAL_ITEM		"Such ... merchandise ... cannot be auctioned here, I am afraid."
#define NO_MIN_BID			"What was the minimum bid in credits you wished to list for this item?"
#define NO_LENGTH_BO		"Very well - but I will also need to know the buyout you wish to place on the listing, the length in days of the auction, or both."
#define NO_LENGTH			"How many days did you wish the auction to be listed for?"
#define GIVE_AWAY			"If you simply wish to give your merchandise away, do not waste my time!"
#define BO_TOO_LOW			"The buyout price for your item must be set higher than its minimum bid."
#define LENGTH_SANITY		"I am afraid we will only list merchandise for a period of one to sixty days."
#define MISSING_CASH		"You seem to be a bit ... short ... to cover your deposit fee today."
#define NOT_EMPTY			"You'll need to empty that before you can place it up for auction."
#define AUCTION_ACCEPTED	"Thank you! Best of luck with your auction."
#define WHICH_AUCTION		"Which auction did you wish to place a bid on?"
#define HOW_MUCH			"How many credits did you wish to bid on this auction?"
#define NO_SUCH_AUCTION		"I'm sorry, but I do not see any such auction listed with this establishment."
#define NO_SUCH_AUCTIONC	"I'm sorry, but I do not see any such auction that you have posted."
#define MISSING_CASH_BID	"Bah! Come back when you can back up your bids with something more than words."
#define BID_ACCEPTED		"Thank you for your bid! Good luck winning your auction."
#define OWN_AUCTION			"You can't bid on your own auctions!"
#define ALREADY_WINNING		"You are already the high bidder on that item."
#define BUYOUT_ACCEPTED		"Congratulations! The auction is yours. Well done! Be sure to #6RETRIEVE#0 your winnings from me at your earliest convenience."
#define NO_WINNINGS			"This establishment is not storing any auction winnings or returned merchandise for you at this time. Perhaps you are mistaken?"
#define NO_HANDS_FREE		"I have something for you here, but I'll hang onto it until you free up your hands."
#define WHICH_AUCTIONP		"Which auction did you wish to preview?"
#define LOOKING_FOR			"I'm sorry - I don't understand exactly what you were looking for."
#define WHICH_AUCTIONC		"Which auction did you wish to cancel?"
#define NOT_YOUR_AUCTION	"I'm afraid that isn't your auction to cancel!"


/*
*	To-Do List:
*
*  1. Add in staff version of AUCTION STATUS for detailed info about a specified auction.
*  2. Add in vNPC auction posts/sales?
*
*/


int new_auctions = 0;
int sold_auctions = 0;
int auction_bids = 0;


#define AUCTION_BID		1
#define AUCTION_SALE	2
#define AUCTION_EXPIRED	3
extern rpie::server engine;
void
	auction_notify (int id, int type)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	OBJ_DATA	*board = NULL;

	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH], buf4[MAX_STRING_LENGTH];
	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	*buf4 = '\0';

	char buf5[MAX_STRING_LENGTH] = {'\0'};
	char *p;

	MUDMAIL_DATA *message;
	account *acct = NULL;
	time_t current_time;
	char date[32];
	int value = 0;

	current_time = time (0);
	ctime_r (&current_time, date);
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	/*
	row[25] - sdesc of NPC
	row[22] - price it sold for
	row[15] - sdesc of the object
	row[11] - name of the char who placed the auction
	row[9] - name of the current high bidder - call BEFORE bid is recorded
	*/


	std::string world_log_db = engine.get_config ("world_log_db");
	sprintf (buf, "SELECT * FROM %s.ah_auctions WHERE auction_id = %d", 
		world_log_db.c_str (), id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result )
		return;

	if ( !(row = mysql_fetch_row (result)) )
	{
		if ( result )
			mysql_free_result (result);
		return;
	}

	/* Now defunct - we're handling it via hobbit-mails.
	if ( type == AUCTION_BID )
	{
	if ( (tch = get_char_nomask (row[11])) )
	{
	sprintf (buf, "[ OOC: A bid has just been placed for your auction of #2%s.#0 ]", row[15]);
	act (buf, false, tch, NULL, NULL, TO_CHAR | _ACT_FORMAT);
	}

	if ( (tch = get_char_nomask (row[9])) )
	{
	sprintf (buf, "[ OOC: You were just outbid on an auction for #2%s.#0 ]", row[15]);
	act (buf, false, tch, NULL, NULL, TO_CHAR | _ACT_FORMAT);
	}		
	}
	*/

	if ( type == AUCTION_SALE )
	{
		/* Let's notify the seller, if they're online. */
		if ((row[13]))
		{

			value = atoi(row[22]);

			sprintf(buf, "The Auctioneer: #5%s#0", row[25]);
			sprintf(buf2, "Sale of #2%s#0 for #6%d#0 credits", row[15], value);
			sprintf(buf3, "The item you had listed for sale with #5%s#0 has been sold.\n\n"
				"After the auctioneers commission, you have made #6%d#0 credits from the sale of #2%s#0.\n\n"
				"Visit #5%s#0 to '#6retrieve#0' your money.\n", row[25], value, row[15], row[25]);
			reformat_string (buf3, &p);

			CREATE (message, MUDMAIL_DATA, 1);
			message->from = str_dup (buf);
			message->subject = str_dup (buf2);
			message->message = str_dup (p);
			message->from_account = str_dup ("AuctionHouse");
			message->date = str_dup (date);
			message->flags = 0;
			message->target = str_dup (row[11]);

			acct = new account (row[13]);
			save_hobbitmail_message (acct, message);
			delete acct;
			mem_free(p);


			//sprintf (buf, "[ OOC: Your auction of #2%s#0 just sold. ]", row[15]);
			//act (buf, false, tch, NULL, NULL, TO_CHAR | _ACT_FORMAT);


		}		

		if ((board = vtoo(atoi(row[26]))))
		{

			if (row[27] && *row[27])
			{
				sprintf(buf5, "%s", row[27]);
			}
			else
			{
				sprintf(buf5, "an unknown seller");
			}

			sprintf(buf, "#3Sale#0: #2%s#0 for #6%d#0 credits.", row[15], value);

			if (atoi(row[10])>0)
				sprintf(buf2, "and with a buyout set at #6%s#0 credits", row[10]);

			sprintf(buf3, "At this time, #2%s#0 was #3sold#0 for #6%d#0 credits to #5%s#0.\n\n"
				"The item was put up for sale by #5%s#0, was originally listed for a minimum bid of #6%s#0 credits %s, "
				"and was offered for a period of #6%s#0 days.",
				row[15], value, buf5, row[12], row[7], buf2, row[3]);                                      

			reformat_string (buf3, &p);

			post_straight_to_mysql_board(fname((board)->name), buf, buf5, p);

			mem_free(p);
		}

		/* This used to send the message to the previous loser - now defunct, given we update the bid before sending the message out.
		else if ( (tch = get_char_nomask (row[9])) )
		{
		sprintf (buf, "[ OOC: You just lost the auction for #2%s#0 to a buyout bid. ]", row[15]);
		act (buf, false, tch, NULL, NULL, TO_CHAR | _ACT_FORMAT);
		}
		*/			
	}

	if ( result )
		mysql_free_result (result);
}

void
	cancel_auction (CHAR_DATA *ch, CHAR_DATA *auctioneer, int id, bool confirmed)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3 [MAX_STRING_LENGTH];
	int			house_id = 0;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';

	house_id = auctioneer->mob->carcass_vnum;
	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();
	sprintf (buf,	
		"SELECT * FROM %s.ah_auctions "
		"WHERE (expires_at > UNIX_TIMESTAMP()) "
		"AND auction_id = %d AND house_id = %d "
		"AND port = %d AND placed_by = '%s'",
		world_log_db.c_str (),
		id, house_id, port, GET_NAME(ch));

	mysql_safe_query (buf);
	send_to_char (mysql_error(database), ch);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows(result) ) 
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_AUCTIONC);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	row = mysql_fetch_row(result);

	if ( str_cmp (row[11], GET_NAME(ch)) )
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NOT_YOUR_AUCTION);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	/* All other sanity checks have been met; let's either prompt for confirmation or process the cancellation. */

	if ( !confirmed ) 
	{
		ch->delay_type = DEL_CANCEL_AUCTION;
		ch->delay_ch = auctioneer;
		ch->delay_info1 = id;

		sprintf (buf,	"To be perfectly clear - you wish to cancel your auction of #2%s#0, and in doing so, lose "
			"the auction deposit that you placed with us?", row[15]);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (auctioneer, buf3, 83);

		send_to_char ("\nIf you are sure you wish to cancel your auction, please type #6ACCEPT#0.\n", ch);

		if ( result )
			mysql_free_result (result);
		return;
	}

	sprintf (buf,	"UPDATE %s.ah_auctions SET expires_at = UNIX_TIMESTAMP() WHERE auction_id = %d", world_log_db.c_str (), id);
	mysql_safe_query (buf);

	ch->delay_type = 0;
	ch->delay_ch = 0;
	ch->delay_info1 = 0;

	name_to_ident (ch, buf2);	
	sprintf (buf,	"Very well. I have cancelled your auction for #2%s#0; we will be keeping the deposit for our trouble. "
		"You may #6RETRIEVE#0 your merchandise from me at your leisure.", row[15]);
	sprintf (buf3, "%s %s", buf2, buf);
	do_whisper (auctioneer, buf3, 83);

	sprintf (buf,	"UPDATE %s.ah_auctions SET cancelled = 1 WHERE auction_id = %d", world_log_db.c_str (), id);
	mysql_safe_query (buf);

	if ( result )
		mysql_free_result (result);
}

void
	preview_auction (CHAR_DATA *ch, CHAR_DATA *auctioneer, int id)
{
	MYSQL_RES	*result = NULL;
	OBJ_DATA	*tobj = NULL;
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3 [MAX_STRING_LENGTH];
	int		house_id = 0;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';

	house_id = auctioneer->mob->carcass_vnum;
	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();

	sprintf (buf, "SELECT * FROM %s.ah_auctions WHERE (expires_at > UNIX_TIMESTAMP()) AND auction_id = %d AND house_id = %d AND port = %d", world_log_db.c_str (), id, house_id, port);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows(result) ) 
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_AUCTION);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	if ( result )
		mysql_free_result (result);

	sprintf (buf, "%s/%d", AUCTION_DIR, id);
	fp = fopen (buf, "r");

	if ( !fp )
	{
		send_to_char ("There was an error loading this item from storage. Please contact staff.\n", ch);
		return;	
	}	

	tobj = fread_obj(fp);
	fclose(fp);

	act ("$N shows you $p.", false, ch, tobj, auctioneer, TO_CHAR);

	send_to_char ("\n", ch);

	show_obj_to_char (tobj, ch, 5);
	show_obj_to_char (tobj, ch, 15);

	extract_obj (tobj);
}

int
	retrieve_expiries (CHAR_DATA *ch, CHAR_DATA *auctioneer)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	OBJ_DATA	*tobj;
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];

	OBJ_DATA	*board = NULL;
	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	char *p;
	int value = 0;


	/* First check for any returned bids owed to the player. */
	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();

	sprintf (buf,	"SELECT * FROM %s.ah_returned_bids WHERE placed_by = '%s' AND picked_up = 0 AND port = %d ORDER BY amount ASC", world_log_db.c_str (), GET_NAME(ch), port);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result )
		return 0;

	if ( (row = mysql_fetch_row(result)) )
	{
		name_to_ident (ch, buf2);
		sprintf (buf,	"%s Better luck next time perhaps - I have %d credits here from an unsuccessful bid you placed some time ago. Here you are.", buf2, atoi(row[1])); 		
		do_whisper (auctioneer, buf, 83);

		keeper_money_to_char (auctioneer, ch, atoi(row[1]));

		sprintf (buf, "UPDATE %s.ah_returned_bids SET picked_up = 1 WHERE placed_by = '%s' AND amount = %d", world_log_db.c_str (), GET_NAME(ch), atoi(row[1]));
		mysql_safe_query (buf);

		if ( result )
			mysql_free_result (result);

		return 1;	
	}

	if ( result )
		mysql_free_result (result);

	/* Then we'll check for any merchandise that needs to be returned due to not selling. */

	sprintf (buf, 
		"SELECT * FROM %s.ah_auctions"
		" WHERE placed_by = '%s'"
		" AND high_bidder = 'none' "
		" AND seller_pickup = FALSE"
		" AND expires_at <= UNIX_TIMESTAMP()"
		" AND port = %d ORDER BY expires_at ASC", 
		world_log_db.c_str (), GET_NAME(ch), port);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( (row = mysql_fetch_row(result)) )
	{
		if ( ch->right_hand && ch->left_hand )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_HANDS_FREE);
			do_whisper (auctioneer, buf, 83);		
			if ( result )
				mysql_free_result(result);
			return 1;
		}

		sprintf (buf, "%s/%s", AUCTION_DIR, row[0]);
		fp = fopen (buf, "r");

		if ( !fp )
		{
			send_to_char ("There was an error loading your item from storage. Please contact staff.\n", ch);
			return 1;	
		}

		name_to_ident (ch, buf2);
		sprintf (buf,	"%s Here we are. As #2%s#0 did not sell, I am afraid we will need to keep the deposit fee. "
			"Better luck next time, perhaps.", buf2, row[15]); 

		do_whisper (auctioneer, buf, 83);

		/* Let's set the return record as picked up by the seller so they can't dupe the item */
		sprintf (buf, "UPDATE %s.ah_auctions SET seller_pickup = TRUE WHERE auction_id = %d", world_log_db.c_str (), atoi(row[0]));
		mysql_safe_query (buf);

		tobj = fread_obj(fp);
		fclose(fp);

		obj_to_char (tobj, ch);

		if ((board = vtoo(atoi(row[26]))))
		{

			value = atoi(row[7]);

			sprintf(buf, "#3Fail#0: #2%s#0 for #6%d#0 credits.", row[15], value);

			if (atoi(row[10])>0)
				sprintf(buf2, "and with a buyout set at #6%s#0 credits", row[10]);
			sprintf(buf3, "At this time, #2%s#0 #3failed to sell#0 for #6%d#0 credits.\n\n"
				"The item was put up for sale by #5%s#0, was originally listed for a minimum bid of #6%s#0 credits, "
				"and was offered for a period of #6%s#0 days.",
				row[15], value, row[12], row[7], row[3]);                                      

			reformat_string (buf3, &p);

			post_straight_to_mysql_board(fname((board)->name), buf, "AuctionHouse", p);

			mem_free(p);
		}
		
		if ( result )
			mysql_free_result (result);
		return 1;
	}

	return 0;
}

int
	retrieve_winnings (CHAR_DATA *ch, CHAR_DATA *auctioneer)
{
	MYSQL_RES	*result = NULL, *result2 = NULL;
	MYSQL_ROW	row = NULL, row2 = NULL;
	OBJ_DATA	*tobj;
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH];
	int			house_id = 0, fee = 0, net = 0, sold_for = 0;

	house_id = auctioneer->mob->carcass_vnum;
	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();

	sprintf (buf,	
		"SELECT * FROM %s.ah_auctions"
		" WHERE ((placed_by = '%s' AND seller_pickup = FALSE)"
		" OR (high_bidder = '%s' AND buyer_pickup = FALSE))"
		" AND expires_at <= UNIX_TIMESTAMP() AND house_id = %d"
		" AND port = %d", 
		world_log_db.c_str (), 
		GET_NAME(ch), GET_NAME(ch), house_id, port);

	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result )
		return 0;

	if ( !(row = mysql_fetch_row(result)) )
	{
		if ( result )
			mysql_free_result (result);
		return 0;
	}

	/* First we'll check to see if they have any cash from sold auctions waiting. */

	if ( !str_cmp (row[11], GET_NAME(ch)) && atoi(row[20]) == false )
	{
		if ( ch->right_hand && ch->left_hand )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_HANDS_FREE);
			do_whisper (auctioneer, buf, 83);		
			if ( result )
				mysql_free_result(result);
			return 1;
		}

		if ( atoi(row[22]) <= 0 )	// An auction won by a non-buyout, so we need to find the highest bidder.
		{
			sprintf (buf,	"SELECT * FROM %s.ah_bids WHERE auction_id = %d ORDER BY bid_amount DESC LIMIT 1", world_log_db.c_str (), atoi(row[0]));

			mysql_safe_query(buf);
			result2 = mysql_store_result (database);

			if ( !result2 || !mysql_num_rows (result2) )
			{
				if ( result2 )
					mysql_free_result (result2);
				sold_for = atoi(row[7]);
			}

			row2 = mysql_fetch_row (result2);

			sold_for = atoi(row2[2]);
			mysql_free_result(result2);

			sprintf (buf,	"UPDATE %s.ah_auctions SET sold_for = %d WHERE auction_id = %d", world_log_db.c_str (), sold_for, atoi(row[0]));
			mysql_safe_query (buf);
		}
		else sold_for = atoi(row[10]);

		fee = 0;

		if (sold_for < 10)
		{
			fee = 0;
		}
		else
		{
			fee = 1;
			fee += sold_for / 50;
		}

		net = (sold_for - fee) + atoi(row[5]);

		name_to_ident (ch, buf2);
		sprintf (buf,	"%s Here we are. For the successful auction of #2%s#0, you will receive %d credits, plus a refund of your %d credit deposit, minus a House consignment "
			"fee of %d credits, for a net total of %d credits. Enjoy!", buf2, row[15], sold_for, atoi(row[5]), fee, net); 

		do_whisper (auctioneer, buf, 83);

		/* Let's set the auction record as picked up by the seller so they can't dupe the cash */
		sprintf (buf, "UPDATE %s.ah_auctions SET seller_pickup = TRUE WHERE auction_id = %d", world_log_db.c_str (), atoi(row[0]));
		mysql_safe_query (buf);

		if ( result )
			mysql_free_result (result);

		keeper_money_to_char (auctioneer, ch, (int) net);

		return 1;
	}

	/* Let's check to see if this is a record for an item they've won and haven't retrieved. */

	if ( !str_cmp (row[9], GET_NAME(ch)) && atoi(row[19]) == false )
	{
		if ( ch->right_hand && ch->left_hand )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_HANDS_FREE);
			do_whisper (auctioneer, buf, 83);		
			if ( result )
				mysql_free_result(result);
			return 1;
		}

		if ( atoi(row[22]) <= 0 )	// An auction won by a non-buyout, so we need to find the highest bidder.
		{
			sprintf (buf,	"SELECT * FROM %s.ah_bids WHERE auction_id = %d ORDER BY bid_amount DESC LIMIT 1",world_log_db.c_str (), atoi(row[0]));

			mysql_safe_query(buf);
			result2 = mysql_store_result (database);

			if ( !result2 || !mysql_num_rows (result2) )
			{
				if ( result2 )
					mysql_free_result (result2);
				sold_for = atoi(row[7]);
			}

			row2 = mysql_fetch_row (result2);

			sold_for = atoi(row2[2]);
			mysql_free_result(result2);

			sprintf (buf,	"UPDATE %s.ah_auctions SET sold_for = %d WHERE auction_id = %d", world_log_db.c_str (),sold_for, atoi(row[0]));
			mysql_safe_query (buf);
		}
		else sold_for = atoi(row[10]);

		sprintf (buf, "%s/%s", AUCTION_DIR, row[0]);
		fp = fopen (buf, "r");

		if ( !fp )
		{
			send_to_char ("There was an error loading your item from storage. Please contact staff.\n", ch);
			return 1;	
		}

		name_to_ident (ch, buf2);
		sprintf (buf,	"%s Ah, yes. I have something for you right here - #2%s#0, won at auction for a bargain "
			"price of %d credits. Congratulations again on your success!", buf2, row[15], sold_for); 

		do_whisper (auctioneer, buf, 83);

		/* Let's set the auction record as picked up by the buyer so they can't dupe the item */
		sprintf (buf, "UPDATE %s.ah_auctions SET buyer_pickup = TRUE WHERE auction_id = %d",world_log_db.c_str (), atoi(row[0]));
		mysql_safe_query (buf);

		if ( result )
			mysql_free_result (result);

		tobj = fread_obj(fp);
		fclose(fp);

		obj_to_char (tobj, ch);

		return 1;	
	}

	if ( result )
		mysql_free_result (result);

	return 0;
}

void
	record_bid (CHAR_DATA *ch, CHAR_DATA *auctioneer, int id, int bid, bool bought_out, int buyout)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH];
	int			new_bid = 0, house_id = 0;

	house_id = auctioneer->mob->carcass_vnum;
	std::string world_log_db = engine.get_config ("world_log_db");	
	int port = engine.get_port ();

	sprintf (buf,	"SELECT bid_amount,placed_by FROM %s.ah_bids WHERE auction_id = %d AND port = %d ORDER BY bid_amount DESC LIMIT 1", world_log_db.c_str (), id, port);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	/* Queue up the previous high-bidder's money for retrieval at the AH. */

	if ( (row = mysql_fetch_row(result)) )
	{
		sprintf (buf,	"INSERT INTO %s.ah_returned_bids (auction_id, amount, placed_by, house_id, port) VALUES "
			"(%d, %d, '%s', %d, %d)", 
			world_log_db.c_str (), id, atoi(row[0]), row[1], house_id, port);
		mysql_safe_query (buf);
		if ( result )
			mysql_free_result (result);
	}

	mysql_real_escape_string (database, buf2, char_short(ch), strlen (char_short(ch)));

	sprintf (buf,	"INSERT INTO %s.ah_bids "
		"(auction_id, bid_amount, buyout, placed_at, placed_by, placed_by_sdesc, placed_by_account, port) VALUES "
		"(%d, %d, %s, UNIX_TIMESTAMP(), '%s', '%s', '%s', %d)",
		world_log_db.c_str (), id, bid, bought_out ? "TRUE" : "FALSE", GET_NAME(ch), buf2, ch->pc->account_name, port);

	mysql_safe_query (buf);

	if ( !bought_out )
	{
		/* Determine the next minimum bid now that the new bid is in: 10% above current. */

		new_bid = (int) round (bid * 1.1);

		if ( buyout > 0 && new_bid > buyout )	// Make sure the new minimum bid doesn't exceed the buyout.
			new_bid = buyout;

		sprintf (buf,
			"UPDATE %s.ah_auctions "
			" SET next_bid = %d, high_bidder = '%s', high_bid_sdesc = '%s'"
			" WHERE auction_id = %d",
			world_log_db.c_str (), new_bid, GET_NAME(ch), char_short(ch), id);
	}
	else
	{
		/* This is a buyout; record the sale and expire the auction. */

		sold_auctions++;
		sprintf (buf,	"UPDATE %s.ah_auctions "
			"SET high_bidder = '%s', high_bid_sdesc = '%s', expires_at = UNIX_TIMESTAMP(), bought_out = TRUE, sold_for = %d WHERE auction_id = %d",
			world_log_db.c_str (), GET_NAME(ch), char_short(ch), bid, id);	
	}

	mysql_safe_query (buf);	
}

void
	place_bid (CHAR_DATA *ch, CHAR_DATA *auctioneer, int id, int bid, bool confirmed)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3 [MAX_STRING_LENGTH];
	int			house_id = 0, time_remaining;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';

	house_id = auctioneer->mob->carcass_vnum;
	std::string world_log_db = engine.get_config ("world_log_db");	
	int port = engine.get_port ();

	sprintf (buf, "SELECT * FROM %s.ah_auctions WHERE (expires_at > UNIX_TIMESTAMP()) AND auction_id = %d AND house_id = %d AND port = %d", world_log_db.c_str (), id, house_id, port);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows(result) ) 
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_AUCTION);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	row = mysql_fetch_row(result);

	/* Allow bids on our own auctions on the test port, for testing purposes. */

	if (IS_MORTAL(ch) || !engine.in_test_mode ())
	{
		if ( !str_cmp (row[11], GET_NAME(ch)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, OWN_AUCTION);
			do_whisper (auctioneer, buf, 83);
			if ( result )
			{
				mysql_free_result (result);
			}
			return;
		}

		if ( !str_cmp (row[9], GET_NAME(ch)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ALREADY_WINNING);
			do_whisper (auctioneer, buf, 83);
			if ( result )
			{
				mysql_free_result (result);
			}
			return;	
		}
	}

	if ( bid == -1 )		// No bid specified, so setting it to the minimum allowable.
		bid = atoi(row[8]);

	if ( bid < atoi(row[8]) ) 
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s I am sorry, but the next available bid on this auction is set at a minimum of %s credits.", buf2, row[9]);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	if (atoi(row[10]) && bid > atoi(row[10])) 
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s The buyout for this bid is only %s credits: there is no need to bid any higher.", buf2, row[10]);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	/* Okay; so the auction exists, hasn't expired yet, and the player's proffered bid
	exceeds the minimum next bid on the item - let's check to see if they have the cash. */

	if (!can_subtract_money (ch, bid, auctioneer->mob->currency_type))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, MISSING_CASH_BID);
		do_whisper (auctioneer, buf, 83);
		if ( result )
			mysql_free_result (result);
		return;
	}

	/* All other sanity checks have been met; let's either prompt for confirmation or process the bid. */

	if ( !confirmed ) 
	{
		ch->delay_type = DEL_PLACE_BID;
		ch->delay_ch = auctioneer;
		ch->delay_info1 = id;
		ch->delay_info2 = bid;
		ch->delay_info3 = atoi(row[4]);

		sprintf (buf,	"To make sure we understand one another - you wish to place a bid of %d credits upon #2%s#0. "
			"Is this correct?", bid, row[15]);

		name_to_ident (ch, buf2);
		sprintf (buf3, "%s %s", buf2, buf);
		do_whisper (auctioneer, buf3, 83);

		send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to place your bid.\n", ch);

		if ( result )
			mysql_free_result (result);
		return;
	}

	time_remaining = ch->delay_info3;

	ch->delay_type = 0;
	ch->delay_ch = NULL;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;
	ch->delay_info3 = 0;

	subtract_money (ch, bid, auctioneer->mob->currency_type);	
	auction_bids++;

	/* The bid qualifies as a buyout of the auction - process winnings immediately. */
	if ( atoi(row[10]) > 0 && bid >= atoi(row[10]) )
	{
		bid = atoi(row[10]);		
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, BUYOUT_ACCEPTED);
		do_whisper (auctioneer, buf, 83);
		record_bid (ch, auctioneer, id, bid, true, atoi(row[10]));
		auction_notify (id, AUCTION_SALE);	
	}
	/* Record the bid and raise the next minimum bid on the item. */
	else
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, BID_ACCEPTED);
		do_whisper (auctioneer, buf, 83);
		record_bid (ch, auctioneer, id, bid, false, atoi(row[10]));
		auction_notify (id, AUCTION_BID);

		/* Let's check to see how much time remains on the auction - if less than 24
		IG hours, we're going to tack on 4 to allow more bids, max 24 hours. */

		if ( (time_remaining - time(0)) < (60*60*6) )	// Less than 24 IG hours/6 RL hours left on auction.
		{
			time_remaining = MIN((int) (time(0) + (60*60*6)), (time_remaining + (60*60)));

			sprintf (buf, "UPDATE %s.ah_auctions SET expires_at = %d WHERE auction_id = %d", world_log_db.c_str (), time_remaining, id);
			mysql_safe_query (buf);
		}
	}

	if ( result )
		mysql_free_result (result);
}

void
	list_auctions (CHAR_DATA *ch, CHAR_DATA *auctioneer, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char buf [MAX_STRING_LENGTH];
	char buf2 [MAX_STRING_LENGTH];
	char buf3 [MAX_STRING_LENGTH];
	char buf4 [MAX_STRING_LENGTH];
	char statbuf[20];
	int  time_remaining = 0;
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int house_id = 0;
	int type = 0;
	bool quicksell = false;

	// Woot! Let's hear it for sloppy, lazy-ass hacks!
	house_id = auctioneer->mob->carcass_vnum;


	// Huzzah! We love hacks 'cause it lets us piggy-back off them!

	if (IS_SET(auctioneer->act, ACT_ECONZONE))
		quicksell = true;

	if ( *argument )
	{
		argument = one_argument (argument, buf);

		if ( is_number(buf) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, LOOKING_FOR);
			do_whisper (auctioneer, buf, 83);
			return;
		}

		if ( (type = index_lookup (item_types, buf)) == -1 )
			mysql_real_escape_string (database, buf2, buf, strlen (buf));
	}


	// Auction status.
	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();
	if ( cmd == -1 )
		sprintf (buf,
		"SELECT * FROM %s.ah_auctions"
		" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
		" AND house_id = %d AND port = %d"
		" AND (placed_by = '%s' OR high_bidder = '%s')"
		" ORDER BY expires_at ASC", 
		world_log_db.c_str (),house_id, port, 
		GET_NAME(ch), GET_NAME(ch));
	// List auctions by item type.
	else if ( type > 0 )
		sprintf (buf,	
		"SELECT * FROM %s.ah_auctions"
		" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
		" AND obj_type = %d AND house_id = %d"
		" AND port = %d ORDER BY expires_at ASC", 
		world_log_db.c_str (),type, house_id, port);
	// List auctions by keyword.
	else if ( type < 0 )
		sprintf (buf,
		"SELECT * FROM %s.ah_auctions"
		" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
		" AND (obj_short_desc LIKE \"%%%%%s%%%%\""
		" OR obj_full_desc LIKE \"%%%%%s%%%%\")"
		" AND house_id = %d "
		" AND port = %d ORDER BY expires_at ASC", 
		world_log_db.c_str (), buf2, buf2, house_id, port);
	// List all auctions.
	else
		sprintf (buf,	"SELECT * FROM %s.ah_auctions"
		" WHERE (expires_at - UNIX_TIMESTAMP()) > 0 "
		" AND house_id = %d AND port = %d"
		" ORDER BY expires_at ASC", 
		world_log_db.c_str (), house_id, port);

	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result )
		return;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	*buf4 = '\0';

	sprintf (buf + strlen(buf), "\n");

	sprintf (buf + strlen(buf),
		" %7s##   %-35.35s   %-8.8s %-8.8s %-12.12s\n", 
		"Auc. ID", "Item Listed for Auction", "Bid", "Buyout", "Time Left");

	sprintf (buf + strlen(buf), 
		"-------------------------------------------------------------------------------\n");

	if ( !mysql_num_rows(result) && type == 0 )
		sprintf (buf + strlen(buf), "\nThere are no auctions currently listed in this auction house.\n");
	else if ( !mysql_num_rows(result) )
		sprintf (buf + strlen(buf), "\nThere are no auctions currently listed matching that criteria.\n");	

	while ( (row = mysql_fetch_row(result)) )
	{
		sprintf (buf2, "%s cp", row[8]);
		sprintf (buf3, "%s cp", row[10]);

		time_remaining = (atoi(row[4]) - time(0)) * 4;

		days = time_remaining / (60*60*24);
		time_remaining %= (60*60*24);
		hours = time_remaining / (60*60);
		time_remaining %= (60*60);
		minutes = time_remaining / 60;
		time_remaining %= 60;

		if ( !days && !hours && !minutes )
			sprintf (buf4, ">1m");
		else 
		{
			*buf4 = '\0';
			if ( days )
				sprintf (buf4 + strlen(buf4), "%dd ", days);
			if ( hours )
				sprintf (buf4 + strlen(buf4), "%dh ", hours);
			if ( minutes )
				sprintf (buf4 + strlen(buf4), "%dm ", minutes);
		}

		*statbuf = '\0';

		sprintf (statbuf, " ");

		if ( !str_cmp (row[11], GET_NAME(ch)) )
			sprintf (statbuf, "*");
		if ( !str_cmp (row[9], GET_NAME(ch)) )
			sprintf (statbuf, "^");


		if (!quicksell)
			sprintf (buf + strlen(buf),
			"%s%7s.   #2%-35.35s#0   %-8.8s %-8.8s %-12.12s\n", 
			statbuf, row[0], row[15], buf2, atoi(row[10]) > atoi(row[9]) ? buf3 : "None", buf4);
		else
			sprintf (buf + strlen(buf),
			"%s%7s.   #2%-35.35s#0   %-8.8s %-8.8s %-12.12s\n", 
			statbuf, row[0], row[15], "N/A", atoi(row[10]) > atoi(row[9]) ? buf3 : "None", buf4);
	}

	sprintf (buf + strlen(buf), "\n* = Your Auction   ^ = High Bidder\n");

	page_string (ch->descr(), buf);

	if ( result )
		mysql_free_result (result);
}

void
	record_auction (CHAR_DATA *ch, CHAR_DATA *auctioneer, OBJ_DATA *obj,
	int min_bid, int buyout, int length, int deposit)
{
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3 [MAX_STRING_LENGTH], buf4 [MAX_STRING_LENGTH];
	int			expires_at = 0, real_value = 0, id = 0, house_id = 0;

	house_id = auctioneer->mob->carcass_vnum;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	*buf4 = '\0';

	expires_at = time(0) + (60 * 60 * 6 * length);	// Each IC day is 6 RL hours.
	real_value = (int) round (obj->farthings + (obj->silver * 4));

	mysql_real_escape_string (database, buf2, obj->full_description, strlen (obj->full_description));
	mysql_real_escape_string (database, buf3, obj_desc(obj), strlen (obj_desc(obj)));
	mysql_real_escape_string (database, buf4, obj_short_desc(obj), strlen (obj_short_desc(obj)));

	std::string world_log_db = engine.get_config ("world_log_db");
	int port = engine.get_port ();
	sprintf (buf,
		"INSERT INTO %s.ah_auctions "
		"(house_id, placed_at, auction_period, expires_at, deposit_paid, obj_real_value, "
		"min_bid, next_bid, buyout, placed_by, placed_by_sdesc, placed_by_account, obj_type, "
		"obj_short_desc, obj_long_desc, obj_full_desc, obj_vnum, buyer_pickup, seller_pickup, bought_out, port, auctioneer_sdesc, auctioneer_board) VALUES "
		"(%d, UNIX_TIMESTAMP(), %d, %d, %d, %d, %d, %d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d, 0, 0, 0, %d, '%s', %d)", 
		world_log_db.c_str (), house_id, length, expires_at, 
		deposit, real_value, min_bid, min_bid, buyout, GET_NAME(ch), 
		char_short(ch), ch->pc->account_name, GET_ITEM_TYPE(obj), 
		buf4, buf3, buf2, obj->nVirtual, port, char_short(auctioneer), auctioneer->circle);

	mysql_safe_query (buf);

	id = mysql_insert_id (database);

	sprintf (buf, "%s/%d", AUCTION_DIR, id);
	fp = fopen (buf, "w");

	fwrite_a_obj (obj, fp);

	fclose (fp);
}

void
	place_auction (CHAR_DATA *ch, CHAR_DATA *auctioneer, OBJ_DATA *obj,
	int min_bid, int buyout, int length, bool confirmed)
{
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH], buf3 [MAX_STRING_LENGTH];
	int			deposit = 1;

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';

	/* Let's do some sanity checking on everything first... */

	if ( !ch )
		return;

	if ( !auctioneer || !obj )
	{
		std::ostringstream error_message;
		error_message << "Error - " << __FILE__ << ": " << __func__ << " (line" << __LINE__ << ")!\n";
		send_to_char ((error_message.str()).c_str(), ch);
		return;
	}

	if ( min_bid <= 0 )
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, GIVE_AWAY);
		do_whisper (auctioneer, buf, 83);
		return;
	}


	if (IS_SET(auctioneer->act, ACT_ECONZONE))
	{
		buyout = min_bid;
		length = 28;
	}

	if ( buyout && buyout <= min_bid && !(IS_SET(auctioneer->act, ACT_ECONZONE)))
	{		
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, BO_TOO_LOW);
		do_whisper (auctioneer, buf, 83);
		return;
	}

	if ( length > 60 || length <= 0 )
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, LENGTH_SANITY);
		do_whisper (auctioneer, buf, 83);
		return;
	}
	
	// For now, the deposit is always one.
	deposit = 1;
	
	if ( confirmed )
	{
		if (!can_subtract_money (ch, deposit, auctioneer->mob->currency_type))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, MISSING_CASH);
			do_whisper (auctioneer, buf, 83);
		}
		else
		{
			record_auction (ch, auctioneer, obj, min_bid, buyout, length, deposit);
			extract_obj (obj);
			subtract_money (ch, deposit, auctioneer->mob->currency_type);
			send_to_char("\n", ch);
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, AUCTION_ACCEPTED);
			do_whisper (auctioneer, buf, 83);
			new_auctions++;
		}
		ch->delay_type = 0;
		ch->delay_ch = 0;
		ch->delay_obj = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay_info3 = 0;
		return;
	}

	ch->delay_type = DEL_PLACE_AUCTION;
	ch->delay_ch = auctioneer;
	ch->delay_obj = obj;
	ch->delay_info1 = min_bid;
	ch->delay_info2 = buyout;
	ch->delay_info3 = length;

	sprintf (buf3, " and a buyout of %d credits", buyout);

	if (IS_SET(auctioneer->act, ACT_ECONZONE))
		sprintf (buf,	"Let's see if I have this right: you wish to auction #2%s#0 for a price of %d credits." 
		"We will require you to place a deposit of %d credits that will be returned upon the successful sale of your auction. Is this agreeable?", 
		obj->short_description, min_bid, deposit);
	else
		sprintf (buf,	"Let's see if I have this right: you wish to auction #2%s#0 for a minimum bid of %d credits%s, "
		"and you wish it to be listed for %d days. We will require you to place a deposit of "
		"%d credits that will be returned upon the successful sale of your auction. Is this agreeable?", 
		obj->short_description, min_bid, buyout ? buf3 : "", length, deposit);

	name_to_ident (ch, buf2);
	sprintf (buf3, "%s %s", buf2, buf);
	do_whisper (auctioneer, buf3, 83);

	send_to_char ("\nIf you are happy with these terms, please type #6ACCEPT#0 to create your auction.\n", ch);

	return;
}

void
	do_auction (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL, *auctioneer = NULL;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH], buf2 [MAX_STRING_LENGTH];
	int			min_bid = 0, buyout = 0, length = 0, id = 0, bid = 0;
	int			retrieved = 0;

	*buf = '\0';
	*buf2 = '\0';

	if ( IS_NPC (ch) )
	{
		send_to_char ("Only PCs can use the auction system, sorry.\n", ch);
		return;
	}

	/* Accepting a pending auction placement */
	if ( cmd == 1 )
	{
		auctioneer = ch->delay_ch;
		obj = ch->delay_obj;
		min_bid = ch->delay_info1;
		buyout = ch->delay_info2;
		length = ch->delay_info3;
		place_auction (ch, auctioneer, obj, min_bid, buyout, length, true);
		return;
	}

	/* Accepting a pending auction bid */
	if ( cmd == 2 )
	{
		auctioneer = ch->delay_ch;
		id = ch->delay_info1;
		bid = ch->delay_info2;
		place_bid (ch, auctioneer, id, bid, true);
		return;
	}

	/* Accepting a pending auction cancellation */
	if ( cmd == 3 )
	{
		auctioneer = ch->delay_ch;
		id = ch->delay_info1;
		cancel_auction (ch, auctioneer, id, true);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) 
	{
		send_to_char ("Auction what? See #6HELP AUCTION#0 for command usage.\n", ch);
		return;
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
		if (tch != ch && IS_SET (tch->act, ACT_AUCTIONEER) && IS_NPC(tch))
			break;

	auctioneer = tch;

	if ( !auctioneer ) 
	{
		send_to_char ("I don't see any auctioneer here.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "place") ) 
	{
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, AUCTION_WHAT);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		if (IS_SET (ch->room->room_flags, LAWFUL) &&
			IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, ILLEGAL_ITEM);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		if ( obj->contains )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NOT_EMPTY);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		argument = one_argument (argument, buf);
		if ( !*buf || (*buf && !is_number (buf)) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_MIN_BID);
			do_whisper (auctioneer, buf, 83);
			return;		
		}
		min_bid = atoi(buf);
		argument = one_argument (argument, buf);
		if ( (!*buf || !is_number (buf)) && !IS_SET(auctioneer->act, ACT_ECONZONE))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_LENGTH_BO);
			do_whisper (auctioneer, buf, 83);
			return;		
		}
		length = atoi(buf);
		argument = one_argument (argument, buf);
		if ( !*buf )	/* No buyout listed - go ahead and place the auction */
		{
			place_auction (ch, auctioneer, obj, min_bid, buyout, length, false);
			return;
		}
		else
		{
			if ( !is_number (buf) )
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s %s", buf2, NO_LENGTH);
				do_whisper (auctioneer, buf, 83);
				return;		
			}
			buyout = length;
			length = atoi (buf);
			place_auction (ch, auctioneer, obj, min_bid, buyout, length, false);
			return;
		}
	}
	else if ( !str_cmp (buf, "list") )
	{
		list_auctions (ch, auctioneer, argument, 0);
		return;
	}
	else if ( !str_cmp (buf, "status") )
	{
		if ( IS_MORTAL(ch) )
		{
			list_auctions (ch, auctioneer, "", -1);
			return;
		}
		// To-Do: add in immortal auction status command here.
		list_auctions (ch, auctioneer, "", -1);
		return;		
	}
	else if ( !str_cmp (buf, "bid") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number (buf) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, WHICH_AUCTION);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		id = atoi(buf);
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			place_bid (ch, auctioneer, id, -1, false);
			return;
		}
		else {
			if ( !is_number (buf) )
			{
				name_to_ident (ch, buf2);
				sprintf (buf, "%s %s", buf2, HOW_MUCH);
				do_whisper (auctioneer, buf, 83);
				return;
			}
			bid = atoi(buf);	
			place_bid (ch, auctioneer, id, bid, false);
			return;
		}
	}
	else if ( !str_cmp (buf, "preview") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf || !is_number (buf) )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, WHICH_AUCTIONP);
			do_whisper (auctioneer, buf, 83);
			return;
		}
		id = atoi(buf);
		preview_auction (ch, auctioneer, id);
		return;
	}
	else if ( !str_cmp (buf, "cancel") )
	{
		argument = one_argument (argument, buf);
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, "Unfortunately, the cancel auction feature is not working at present. Please contact an admin to have your item removed from auction.");
		do_whisper (auctioneer, buf, 83);

		/*
		if ( !*buf || !is_number (buf) )
		{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, WHICH_AUCTIONC);
		do_whisper (auctioneer, buf, 83);
		return;
		}
		id = atoi(buf);
		cancel_auction (ch, auctioneer, id, false);
		*/
		return;
	}
	else if ( !str_cmp (buf, "retrieve") )
	{
		retrieved += retrieve_expiries (ch, auctioneer);
		if ( !retrieved )
			retrieved += retrieve_winnings (ch, auctioneer);
		if ( !retrieved )
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_WINNINGS);
			do_whisper (auctioneer, buf, 83);		
		}
		return;
	}

	send_to_char ("Auction what? See #6HELP AUCTION#0 for command usage.\n", ch);
	return;
}

