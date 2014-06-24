-- MySQL dump 10.11
--
-- Host: localhost    Database: rpi_world_log
-- ------------------------------------------------------
-- Server version	5.0.88

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `ah_auctions`
--

DROP TABLE IF EXISTS `ah_auctions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ah_auctions` (
  `auction_id` int(11) NOT NULL auto_increment,
  `house_id` int(11) NOT NULL,
  `placed_at` int(11) NOT NULL COMMENT 'Timestamp for when the auction was placed.',
  `auction_period` int(11) NOT NULL COMMENT 'Period for which the auction lasts. (In RL hours?)',
  `expires_at` int(11) NOT NULL COMMENT 'Timestamp for when the auction expires.',
  `deposit_paid` int(11) NOT NULL COMMENT 'The deposit put down by the auctioner for the auction.',
  `obj_real_value` int(11) NOT NULL COMMENT 'The "real" value set by the game of the object, in coppers.',
  `min_bid` int(11) NOT NULL,
  `next_bid` int(11) NOT NULL,
  `high_bidder` varchar(255) NOT NULL default 'none',
  `buyout` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `placed_by_sdesc` varchar(255) NOT NULL default '',
  `placed_by_account` varchar(255) NOT NULL default '',
  `obj_type` int(11) NOT NULL,
  `obj_short_desc` varchar(255) NOT NULL default '',
  `obj_long_desc` varchar(255) NOT NULL default '',
  `obj_full_desc` text NOT NULL,
  `obj_vnum` int(11) NOT NULL,
  `buyer_pickup` int(11) NOT NULL,
  `seller_pickup` int(11) NOT NULL,
  `bought_out` int(11) NOT NULL,
  `sold_for` int(11) NOT NULL default '0',
  `port` int(11) NOT NULL default '4502',
  `cancelled` int(11) NOT NULL default '0',
  `auctioneer_sdesc` varchar(255) NOT NULL default '',
  `auctioneer_board` int(11) NOT NULL default '0',
  `high_bid_sdesc` varchar(255) default NULL,
  PRIMARY KEY  (`auction_id`),
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM AUTO_INCREMENT=2836 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ah_bids`
--

DROP TABLE IF EXISTS `ah_bids`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ah_bids` (
  `bid_id` int(11) NOT NULL auto_increment,
  `auction_id` int(11) NOT NULL,
  `bid_amount` int(11) NOT NULL,
  `buyout` bit(1) NOT NULL default b'0',
  `placed_at` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `placed_by_sdesc` varchar(255) NOT NULL default '',
  `placed_by_account` varchar(255) NOT NULL default '',
  `port` int(11) NOT NULL default '4502',
  PRIMARY KEY  (`bid_id`),
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1851 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ah_orders`
--

DROP TABLE IF EXISTS `ah_orders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ah_orders` (
  `auction_id` int(11) NOT NULL auto_increment,
  `house_id` int(11) default NULL,
  `auctioneer_sdesc` varchar(255) default NULL,
  `auctioneer_board` int(11) NOT NULL default '0',
  `expires_at` int(11) NOT NULL default '0',
  `offered_price` int(11) NOT NULL default '0',
  `placed_by` varchar(255) default NULL,
  `placed_by_sdesc` varchar(255) default NULL,
  `placed_by_account` varchar(255) default NULL,
  `wanted_vnum` int(11) NOT NULL default '0',
  `wanted_sdesc` varchar(255) default NULL,
  `wanted_condition` int(11) NOT NULL default '0',
  `wanted_quantity` tinyint(11) NOT NULL default '1',
  `wanted_vars` int(11) NOT NULL default '0',
  `status` int(11) NOT NULL default '0',
  `var1` varchar(255) default NULL,
  `var2` varchar(255) default NULL,
  `var3` varchar(255) default NULL,
  `var4` varchar(255) default NULL,
  `var5` varchar(255) default NULL,
  `var6` varchar(255) default NULL,
  `var7` varchar(255) default NULL,
  `var8` varchar(255) default NULL,
  `var9` varchar(255) default NULL,
  `var10` varchar(255) default NULL,
  PRIMARY KEY  (`auction_id`)
) ENGINE=MyISAM AUTO_INCREMENT=254 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ah_returned_bids`
--

DROP TABLE IF EXISTS `ah_returned_bids`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ah_returned_bids` (
  `auction_id` int(11) NOT NULL,
  `amount` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `house_id` int(11) NOT NULL,
  `picked_up` smallint(6) NOT NULL default '0',
  `port` int(11) NOT NULL default '4502',
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `rent`
--

DROP TABLE IF EXISTS `rent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rent` (
  `id` int(11) NOT NULL auto_increment,
  `key_coldload` int(11) NOT NULL,
  `agency_vnum` int(11) NOT NULL,
  `pc_coldload` int(11) NOT NULL default '0',
  `pc_short` varchar(255) default NULL,
  `room_coldload` int(11) NOT NULL default '0',
  `balance` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=300 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `turfblocks`
--

DROP TABLE IF EXISTS `turfblocks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `turfblocks` (
  `room` int(11) NOT NULL,
  `hood` int(11) NOT NULL default '0',
  `loyalty` int(11) NOT NULL default '-1',
  `type` int(11) NOT NULL default '0',
  `improve` int(11) NOT NULL default '0',
  `violence` int(11) default NULL,
  `onelove` int(11) NOT NULL default '0',
  `onefear` int(11) NOT NULL default '0',
  `twolove` int(11) NOT NULL default '0',
  `twofear` int(11) NOT NULL default '0',
  `threelove` int(11) NOT NULL default '0',
  `threefear` int(11) NOT NULL default '0',
  `fourlove` int(11) NOT NULL default '0',
  `fourfear` int(11) NOT NULL default '0',
  `fivelove` int(11) NOT NULL default '0',
  `fivefear` int(11) NOT NULL default '0',
  `sixlove` int(11) default NULL,
  `sixfear` int(11) default NULL,
  `sevenlove` int(11) default NULL,
  `sevenfear` int(11) default NULL,
  `eightlove` int(11) default NULL,
  `eightfear` int(11) default NULL,
  `ninelove` int(11) default NULL,
  `ninefear` int(11) default NULL,
  `tenlove` int(11) default NULL,
  `tenfear` int(11) default NULL,
  `actone` int(11) NOT NULL default '0',
  `acttwo` int(11) NOT NULL default '0',
  `actthree` int(11) NOT NULL default '0',
  `actfour` int(11) NOT NULL default '0',
  `actfive` int(11) NOT NULL default '0',
  `actsix` int(11) default NULL,
  `actseven` int(11) default NULL,
  `acteight` int(11) default NULL,
  `actnine` int(11) default NULL,
  `actten` int(11) default NULL,
  `powone` int(11) NOT NULL default '0',
  `powtwo` int(11) NOT NULL default '0',
  `powthree` int(11) NOT NULL default '0',
  `powfour` int(11) NOT NULL default '0',
  `powfive` int(11) NOT NULL default '0',
  `powsix` int(11) default NULL,
  `powseven` int(11) default NULL,
  `poweight` int(11) default NULL,
  `pownine` int(11) default NULL,
  `powten` int(11) default NULL,
  PRIMARY KEY  (`room`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `turfhoods`
--

DROP TABLE IF EXISTS `turfhoods`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `turfhoods` (
  `id` int(11) NOT NULL,
  `loyalty` int(11) NOT NULL default '-1',
  `system` int(11) NOT NULL,
  `name` varchar(255) default NULL,
  `dumproom` int(11) NOT NULL default '0',
  `npc_coldload_first` int(11) NOT NULL default '0',
  `npc_coldload_second` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `turfsystems`
--

DROP TABLE IF EXISTS `turfsystems`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `turfsystems` (
  `id` int(11) NOT NULL,
  `boardvnum` int(11) default NULL,
  `clanone` varchar(255) default NULL,
  `clantwo` varchar(255) default NULL,
  `clanthree` varchar(255) default NULL,
  `clanfour` varchar(255) default NULL,
  `clanfive` varchar(255) default NULL,
  `clansix` varchar(255) default NULL,
  `clanseven` varchar(255) default NULL,
  `claneight` varchar(255) default NULL,
  `clannine` varchar(255) default NULL,
  `clanten` varchar(255) default NULL,
  `dumpvnum` int(11) NOT NULL,
  `enforcemob` int(11) NOT NULL default '0',
  `shopkeepmob` int(11) NOT NULL default '0',
  `soldiermob` int(11) NOT NULL default '0',
  `shopkeepfee` int(11) NOT NULL,
  `soldierFee` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-07-03  7:52:55
