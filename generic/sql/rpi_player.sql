-- MySQL dump 10.11
--
-- Host: localhost    Database: rpi_player
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
-- Table structure for table `arena_bets`
--

DROP TABLE IF EXISTS `arena_bets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `arena_bets` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL,
  `sdesc` varchar(255) NOT NULL,
  `glad_name` varchar(255) NOT NULL,
  `glad_short` varchar(255) NOT NULL,
  `amount` int(11) NOT NULL default '0',
  `win_lose` int(11) NOT NULL default '0',
  `odds_top` int(11) NOT NULL default '0',
  `odds_under` int(11) NOT NULL default '0',
  `season` int(11) NOT NULL default '1',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `arena_fights`
--

DROP TABLE IF EXISTS `arena_fights`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `arena_fights` (
  `id` int(11) NOT NULL auto_increment,
  `red_name` varchar(255) NOT NULL,
  `blue_name` varchar(255) NOT NULL,
  `red_short` varchar(255) NOT NULL,
  `blue_short` varchar(255) NOT NULL,
  `red_win` int(11) NOT NULL default '0',
  `blue_win` int(11) NOT NULL default '0',
  `degree` int(11) NOT NULL default '0',
  `odds_top` int(11) NOT NULL default '0',
  `odds_under` int(11) NOT NULL default '0',
  `season` int(11) NOT NULL default '1',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1561 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `arena_gladiator`
--

DROP TABLE IF EXISTS `arena_gladiator`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `arena_gladiator` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL,
  `alias` varchar(255) NOT NULL,
  `short` varchar(255) NOT NULL default '0',
  `win` int(11) NOT NULL default '0',
  `lose` int(11) NOT NULL default '0',
  `fought` int(11) NOT NULL default '0',
  `season` int(11) NOT NULL default '1',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=659 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clans`
--

DROP TABLE IF EXISTS `clans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clans` (
  `id` smallint(5) unsigned NOT NULL auto_increment,
  `name` varchar(32) default NULL,
  `long_name` varchar(80) default NULL,
  `zone` smallint(6) NOT NULL default '0',
  `member_obj` mediumint(8) unsigned NOT NULL default '0',
  `leader_obj` mediumint(8) unsigned NOT NULL default '0',
  `omni_obj` mediumint(8) unsigned NOT NULL default '0',
  UNIQUE KEY `id` (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=103 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `dreams`
--

DROP TABLE IF EXISTS `dreams`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dreams` (
  `name` varchar(255) NOT NULL default '',
  `dreamed` int(11) default NULL,
  `dream` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `morgul_reserves_20060401`
--

DROP TABLE IF EXISTS `morgul_reserves_20060401`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `morgul_reserves_20060401` (
  `name` varchar(255) NOT NULL default '',
  `account` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pfiles`
--

DROP TABLE IF EXISTS `pfiles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pfiles` (
  `name` varchar(255) NOT NULL default '',
  `keywords` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `sdesc` varchar(255) default NULL,
  `ldesc` varchar(255) default NULL,
  `description` text,
  `msg` text,
  `create_comment` text,
  `create_state` int(11) default NULL,
  `nanny_state` int(11) default NULL,
  `role` int(11) default NULL,
  `role_summary` varchar(255) default NULL,
  `role_body` text,
  `role_date` varchar(255) default NULL,
  `role_poster` varchar(75) default NULL,
  `role_cost` int(11) default NULL,
  `app_cost` int(11) default NULL,
  `level` int(11) default NULL,
  `sex` int(11) default NULL,
  `deity` int(11) default NULL,
  `race` int(11) default NULL,
  `room` int(11) default NULL,
  `str` int(11) default NULL,
  `intel` int(11) default NULL,
  `wil` int(11) default NULL,
  `con` int(11) default NULL,
  `dex` int(11) default NULL,
  `aur` int(11) default NULL,
  `agi` int(11) default NULL,
  `start_str` int(11) default NULL,
  `start_intel` int(11) default NULL,
  `start_wil` int(11) default NULL,
  `start_con` int(11) default NULL,
  `start_dex` int(11) default NULL,
  `start_aur` int(11) default NULL,
  `start_agi` int(11) default NULL,
  `played` int(11) default NULL,
  `birth` int(11) default NULL,
  `time` int(11) default NULL,
  `offense` int(11) default NULL,
  `hit` int(11) default NULL,
  `maxhit` int(11) default NULL,
  `nat_attack_type` int(11) default NULL,
  `move` int(11) default NULL,
  `maxmove` int(11) default NULL,
  `circle` int(11) default NULL,
  `ppoints` int(11) default NULL,
  `fightmode` int(11) default NULL,
  `color` int(11) default NULL,
  `speaks` int(11) default NULL,
  `flags` int(11) default NULL,
  `plrflags` int(11) default NULL,
  `boatvnum` int(11) default NULL,
  `speed` int(11) default NULL,
  `mountspeed` int(11) default NULL,
  `sleepneeded` int(11) default NULL,
  `autotoll` int(11) default NULL,
  `coldload` int(10) unsigned NOT NULL default '0',
  `affectedby` int(11) default NULL,
  `affects` text,
  `age` int(11) default NULL,
  `intoxication` int(11) default NULL,
  `hunger` int(11) default NULL,
  `thirst` int(11) default NULL,
  `height` int(11) default NULL,
  `frame` int(11) default NULL,
  `damage` int(11) default NULL,
  `lastregen` int(11) default NULL,
  `lastroom` int(11) default NULL,
  `harness` int(11) default NULL,
  `maxharness` int(11) default NULL,
  `lastlogon` int(11) default NULL,
  `lastlogoff` int(11) default NULL,
  `lastdis` int(11) default NULL,
  `lastconnect` int(11) default NULL,
  `lastdied` int(11) default NULL,
  `hooded` int(11) default NULL,
  `immenter` varchar(255) default NULL,
  `immleave` varchar(255) default NULL,
  `sitelie` varchar(255) default NULL,
  `voicestr` varchar(255) default NULL,
  `clans` text,
  `skills` text,
  `wounds` text,
  `lodged` text,
  `writes` int(11) NOT NULL default '0',
  `profession` int(10) unsigned NOT NULL default '0',
  `was_in_room` int(11) NOT NULL default '0',
  `travelstr` varchar(255) default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  `bmi` smallint(6) default '0',
  `guardian_mode` int(10) unsigned NOT NULL default '0',
  `hire_storeroom` int(10) unsigned default '0',
  `hire_storeobj` int(10) unsigned default '0',
  `plan` varchar(80) default NULL,
  `goal` varchar(255) default NULL,
  `role_id` int(11) NOT NULL default '0',
  `power_level` int(11) NOT NULL default '0',
  `talents` int(11) default '0',
  `dmote` text,
  `d_age` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_eyes` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_length` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_color` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_style` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_height` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_frame` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat1` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat2` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat3` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat4` varchar(255) character set latin1 collate latin1_bin default NULL,
  `scents` text,
  `aff_mod` text,
  PRIMARY KEY  (`name`),
  KEY `account` (`account`),
  KEY `create_state` (`create_state`),
  KEY `account_2` (`account`,`create_state`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pfiles_alpha_backup`
--

DROP TABLE IF EXISTS `pfiles_alpha_backup`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pfiles_alpha_backup` (
  `name` varchar(255) NOT NULL default '',
  `keywords` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `sdesc` varchar(255) default NULL,
  `ldesc` varchar(255) default NULL,
  `description` text,
  `msg` text,
  `create_comment` text,
  `create_state` int(11) default NULL,
  `nanny_state` int(11) default NULL,
  `role` int(11) default NULL,
  `role_summary` varchar(255) default NULL,
  `role_body` text,
  `role_date` varchar(255) default NULL,
  `role_poster` varchar(75) default NULL,
  `role_cost` int(11) default NULL,
  `app_cost` int(11) default NULL,
  `level` int(11) default NULL,
  `sex` int(11) default NULL,
  `deity` int(11) default NULL,
  `race` int(11) default NULL,
  `room` int(11) default NULL,
  `str` int(11) default NULL,
  `intel` int(11) default NULL,
  `wil` int(11) default NULL,
  `con` int(11) default NULL,
  `dex` int(11) default NULL,
  `aur` int(11) default NULL,
  `agi` int(11) default NULL,
  `start_str` int(11) default NULL,
  `start_intel` int(11) default NULL,
  `start_wil` int(11) default NULL,
  `start_con` int(11) default NULL,
  `start_dex` int(11) default NULL,
  `start_aur` int(11) default NULL,
  `start_agi` int(11) default NULL,
  `played` int(11) default NULL,
  `birth` int(11) default NULL,
  `time` int(11) default NULL,
  `offense` int(11) default NULL,
  `hit` int(11) default NULL,
  `maxhit` int(11) default NULL,
  `nat_attack_type` int(11) default NULL,
  `move` int(11) default NULL,
  `maxmove` int(11) default NULL,
  `circle` int(11) default NULL,
  `ppoints` int(11) default NULL,
  `fightmode` int(11) default NULL,
  `color` int(11) default NULL,
  `speaks` int(11) default NULL,
  `flags` int(11) default NULL,
  `plrflags` int(11) default NULL,
  `boatvnum` int(11) default NULL,
  `speed` int(11) default NULL,
  `mountspeed` int(11) default NULL,
  `sleepneeded` int(11) default NULL,
  `autotoll` int(11) default NULL,
  `coldload` int(10) unsigned NOT NULL default '0',
  `affectedby` int(11) default NULL,
  `affects` text,
  `age` int(11) default NULL,
  `intoxication` int(11) default NULL,
  `hunger` int(11) default NULL,
  `thirst` int(11) default NULL,
  `height` int(11) default NULL,
  `frame` int(11) default NULL,
  `damage` int(11) default NULL,
  `lastregen` int(11) default NULL,
  `lastroom` int(11) default NULL,
  `harness` int(11) default NULL,
  `maxharness` int(11) default NULL,
  `lastlogon` int(11) default NULL,
  `lastlogoff` int(11) default NULL,
  `lastdis` int(11) default NULL,
  `lastconnect` int(11) default NULL,
  `lastdied` int(11) default NULL,
  `hooded` int(11) default NULL,
  `immenter` varchar(255) default NULL,
  `immleave` varchar(255) default NULL,
  `sitelie` varchar(255) default NULL,
  `voicestr` varchar(255) default NULL,
  `clans` text,
  `skills` text,
  `wounds` text,
  `lodged` text,
  `writes` int(11) NOT NULL default '0',
  `profession` int(10) unsigned NOT NULL default '0',
  `was_in_room` int(11) NOT NULL default '0',
  `travelstr` varchar(255) default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  `bmi` smallint(6) default '0',
  `guardian_mode` int(10) unsigned NOT NULL default '0',
  `hire_storeroom` int(10) unsigned default '0',
  `hire_storeobj` int(10) unsigned default '0',
  `plan` varchar(80) default NULL,
  `goal` varchar(255) default NULL,
  `role_id` int(11) NOT NULL default '0',
  `power_level` int(11) NOT NULL default '0',
  `talents` int(11) default '0',
  `dmote` text,
  `d_age` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_eyes` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_length` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_color` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_style` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_height` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_frame` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat1` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat2` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat3` varchar(255) character set latin1 collate latin1_bin default NULL,
  `d_feat4` varchar(255) character set latin1 collate latin1_bin default NULL,
  `scents` text,
  `aff_mod` text,
  PRIMARY KEY  (`name`),
  KEY `account` (`account`),
  KEY `create_state` (`create_state`),
  KEY `account_2` (`account`,`create_state`)
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

-- Dump completed on 2012-07-03  7:52:30
