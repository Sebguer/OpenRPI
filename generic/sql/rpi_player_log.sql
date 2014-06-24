-- MySQL dump 10.11
--
-- Host: localhost    Database: rpi_player_log
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
-- Table structure for table `cues`
--

DROP TABLE IF EXISTS `cues`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cues` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `mid` int(10) unsigned default NULL,
  `cue` enum('notes','flags','memory','on_hour','on_time','on_health','on_moves','on_command','on_receive','on_hear','on_nod','on_theft','on_witness','on_engage','on_flee','on_scan','on_hit','on_reboot','mob_present','obj_present','on_death','on_five','on_one','blank_cue') default NULL,
  `reflex` varchar(255) default NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=912 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ip`
--

DROP TABLE IF EXISTS `ip`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip` (
  `account` varchar(32) NOT NULL default '',
  `host` varchar(255) default NULL,
  `ip` varchar(32) NOT NULL default '',
  `firsttime` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `lasttime` timestamp NOT NULL default '0000-00-00 00:00:00',
  `count` int(11) default '0',
  `is_new` tinyint(1) default '0',
  `has_pwd` tinyint(1) default '0',
  `port` int(11) NOT NULL default '0',
  `logins` int(11) NOT NULL default '0',
  `fails` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account`,`ip`,`port`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mud`
--

DROP TABLE IF EXISTS `mud`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mud` (
  `name` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `switched_into` varchar(255) default NULL,
  `timestamp` int(11) default '0',
  `port` int(11) default '4500',
  `room` int(11) default '-1',
  `guest` tinyint(1) default '0',
  `immortal` tinyint(1) default '0',
  `error` tinyint(1) default '0',
  `command` varchar(255) default NULL,
  `entry` text,
  `sha_hash` varchar(45) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ocues`
--

DROP TABLE IF EXISTS `ocues`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ocues` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `mid` int(10) unsigned default NULL,
  `cue` enum('notes','on_grab','on_drop','on_give','on_receive','on_strike','on_mstrike','on_block','on_blocked','on_hit','on_hour','on_enter','on_decay','on_morph','on_produced','on_craft','on_load','on_five','on_one') default NULL,
  `reflex` varchar(255) default NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=311 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `receipts`
--

DROP TABLE IF EXISTS `receipts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `receipts` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `shopkeep` mediumint(8) unsigned NOT NULL default '0',
  `transaction` enum('unknown','sold','bought') NOT NULL default 'unknown',
  `who` varchar(32) default NULL,
  `customer` varchar(255) default NULL,
  `vnum` mediumint(8) unsigned NOT NULL default '0',
  `item` varchar(80) default NULL,
  `qty` tinyint(3) unsigned NOT NULL default '0',
  `cost` mediumint(8) unsigned NOT NULL default '0',
  `room` mediumint(8) unsigned NOT NULL default '0',
  `gametime` datetime default NULL,
  `port` smallint(5) unsigned NOT NULL default '4500',
  PRIMARY KEY  (`id`),
  KEY `shopkeep` (`shopkeep`,`port`),
  KEY `time` (`time`)
) ENGINE=MyISAM AUTO_INCREMENT=42732 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-07-03  7:52:41
