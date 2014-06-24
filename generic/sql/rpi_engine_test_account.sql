-- MySQL dump 10.13  Distrib 5.5.24, for Linux (x86_64)
--
-- Host: localhost    Database: rpi_engine
-- ------------------------------------------------------
-- Server version	5.5.24

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
-- Dumping data for table `account_donations`
--

LOCK TABLES `account_donations` WRITE;
/*!40000 ALTER TABLE `account_donations` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_donations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `account_referrals`
--

LOCK TABLES `account_referrals` WRITE;
/*!40000 ALTER TABLE `account_referrals` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_referrals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `account_withdrawals`
--

LOCK TABLES `account_withdrawals` WRITE;
/*!40000 ALTER TABLE `account_withdrawals` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_withdrawals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `announcements`
--

LOCK TABLES `announcements` WRITE;
/*!40000 ALTER TABLE `announcements` DISABLE KEYS */;
/*!40000 ALTER TABLE `announcements` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `application_wait_times`
--

LOCK TABLES `application_wait_times` WRITE;
/*!40000 ALTER TABLE `application_wait_times` DISABLE KEYS */;
/*!40000 ALTER TABLE `application_wait_times` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `banned_sites`
--

LOCK TABLES `banned_sites` WRITE;
/*!40000 ALTER TABLE `banned_sites` DISABLE KEYS */;
/*!40000 ALTER TABLE `banned_sites` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `boards`
--

LOCK TABLES `boards` WRITE;
/*!40000 ALTER TABLE `boards` DISABLE KEYS */;
/*!40000 ALTER TABLE `boards` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `building_log`
--

LOCK TABLES `building_log` WRITE;
/*!40000 ALTER TABLE `building_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `building_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `building_totals`
--

LOCK TABLES `building_totals` WRITE;
/*!40000 ALTER TABLE `building_totals` DISABLE KEYS */;
/*!40000 ALTER TABLE `building_totals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `changed_helpfiles`
--

LOCK TABLES `changed_helpfiles` WRITE;
/*!40000 ALTER TABLE `changed_helpfiles` DISABLE KEYS */;
/*!40000 ALTER TABLE `changed_helpfiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `clan_assignments`
--

LOCK TABLES `clan_assignments` WRITE;
/*!40000 ALTER TABLE `clan_assignments` DISABLE KEYS */;
/*!40000 ALTER TABLE `clan_assignments` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `code_changelog`
--

LOCK TABLES `code_changelog` WRITE;
/*!40000 ALTER TABLE `code_changelog` DISABLE KEYS */;
/*!40000 ALTER TABLE `code_changelog` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `coding_log`
--

LOCK TABLES `coding_log` WRITE;
/*!40000 ALTER TABLE `coding_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `coding_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `contributions`
--

LOCK TABLES `contributions` WRITE;
/*!40000 ALTER TABLE `contributions` DISABLE KEYS */;
/*!40000 ALTER TABLE `contributions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_aiming`
--

LOCK TABLES `copyover_aiming` WRITE;
/*!40000 ALTER TABLE `copyover_aiming` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_aiming` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_animations`
--

LOCK TABLES `copyover_animations` WRITE;
/*!40000 ALTER TABLE `copyover_animations` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_animations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_crafts`
--

LOCK TABLES `copyover_crafts` WRITE;
/*!40000 ALTER TABLE `copyover_crafts` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_crafts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_delays`
--

LOCK TABLES `copyover_delays` WRITE;
/*!40000 ALTER TABLE `copyover_delays` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_delays` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_doors`
--

LOCK TABLES `copyover_doors` WRITE;
/*!40000 ALTER TABLE `copyover_doors` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_doors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_fighting`
--

LOCK TABLES `copyover_fighting` WRITE;
/*!40000 ALTER TABLE `copyover_fighting` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_fighting` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_followers`
--

LOCK TABLES `copyover_followers` WRITE;
/*!40000 ALTER TABLE `copyover_followers` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_followers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_sighted_targets`
--

LOCK TABLES `copyover_sighted_targets` WRITE;
/*!40000 ALTER TABLE `copyover_sighted_targets` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_sighted_targets` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `copyover_subduers`
--

LOCK TABLES `copyover_subduers` WRITE;
/*!40000 ALTER TABLE `copyover_subduers` DISABLE KEYS */;
/*!40000 ALTER TABLE `copyover_subduers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `craft_progs`
--

LOCK TABLES `craft_progs` WRITE;
/*!40000 ALTER TABLE `craft_progs` DISABLE KEYS */;
/*!40000 ALTER TABLE `craft_progs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `craft_progs_backup`
--

LOCK TABLES `craft_progs_backup` WRITE;
/*!40000 ALTER TABLE `craft_progs_backup` DISABLE KEYS */;
/*!40000 ALTER TABLE `craft_progs_backup` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `crafts`
--

LOCK TABLES `crafts` WRITE;
/*!40000 ALTER TABLE `crafts` DISABLE KEYS */;
/*!40000 ALTER TABLE `crafts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `donation_tracking`
--

LOCK TABLES `donation_tracking` WRITE;
/*!40000 ALTER TABLE `donation_tracking` DISABLE KEYS */;
/*!40000 ALTER TABLE `donation_tracking` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `downloads`
--

LOCK TABLES `downloads` WRITE;
/*!40000 ALTER TABLE `downloads` DISABLE KEYS */;
/*!40000 ALTER TABLE `downloads` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `failed_logins`
--

LOCK TABLES `failed_logins` WRITE;
/*!40000 ALTER TABLE `failed_logins` DISABLE KEYS */;
INSERT INTO `failed_logins` VALUES ('God','localhost',1341541938),('God','localhost',1341541942),('God','localhost',1341541943);
/*!40000 ALTER TABLE `failed_logins` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `foraged`
--

LOCK TABLES `foraged` WRITE;
/*!40000 ALTER TABLE `foraged` DISABLE KEYS */;
/*!40000 ALTER TABLE `foraged` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_auth_access`
--

LOCK TABLES `forum_auth_access` WRITE;
/*!40000 ALTER TABLE `forum_auth_access` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_auth_access` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_banlist`
--

LOCK TABLES `forum_banlist` WRITE;
/*!40000 ALTER TABLE `forum_banlist` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_banlist` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_categories`
--

LOCK TABLES `forum_categories` WRITE;
/*!40000 ALTER TABLE `forum_categories` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_categories` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_config`
--

LOCK TABLES `forum_config` WRITE;
/*!40000 ALTER TABLE `forum_config` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_config` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_confirm`
--

LOCK TABLES `forum_confirm` WRITE;
/*!40000 ALTER TABLE `forum_confirm` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_confirm` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_disallow`
--

LOCK TABLES `forum_disallow` WRITE;
/*!40000 ALTER TABLE `forum_disallow` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_disallow` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_forum_lock`
--

LOCK TABLES `forum_forum_lock` WRITE;
/*!40000 ALTER TABLE `forum_forum_lock` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_forum_lock` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_forum_prune`
--

LOCK TABLES `forum_forum_prune` WRITE;
/*!40000 ALTER TABLE `forum_forum_prune` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_forum_prune` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_forums`
--

LOCK TABLES `forum_forums` WRITE;
/*!40000 ALTER TABLE `forum_forums` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_forums` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_groups`
--

LOCK TABLES `forum_groups` WRITE;
/*!40000 ALTER TABLE `forum_groups` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_groups` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_posts`
--

LOCK TABLES `forum_posts` WRITE;
/*!40000 ALTER TABLE `forum_posts` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_posts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_posts_text`
--

LOCK TABLES `forum_posts_text` WRITE;
/*!40000 ALTER TABLE `forum_posts_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_posts_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_privmsgs`
--

LOCK TABLES `forum_privmsgs` WRITE;
/*!40000 ALTER TABLE `forum_privmsgs` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_privmsgs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_privmsgs_text`
--

LOCK TABLES `forum_privmsgs_text` WRITE;
/*!40000 ALTER TABLE `forum_privmsgs_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_privmsgs_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_ranks`
--

LOCK TABLES `forum_ranks` WRITE;
/*!40000 ALTER TABLE `forum_ranks` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_ranks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_search_results`
--

LOCK TABLES `forum_search_results` WRITE;
/*!40000 ALTER TABLE `forum_search_results` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_search_results` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_search_wordlist`
--

LOCK TABLES `forum_search_wordlist` WRITE;
/*!40000 ALTER TABLE `forum_search_wordlist` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_search_wordlist` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_search_wordmatch`
--

LOCK TABLES `forum_search_wordmatch` WRITE;
/*!40000 ALTER TABLE `forum_search_wordmatch` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_search_wordmatch` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_sessions`
--

LOCK TABLES `forum_sessions` WRITE;
/*!40000 ALTER TABLE `forum_sessions` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_smilies`
--

LOCK TABLES `forum_smilies` WRITE;
/*!40000 ALTER TABLE `forum_smilies` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_smilies` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_themes`
--

LOCK TABLES `forum_themes` WRITE;
/*!40000 ALTER TABLE `forum_themes` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_themes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_themes_name`
--

LOCK TABLES `forum_themes_name` WRITE;
/*!40000 ALTER TABLE `forum_themes_name` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_themes_name` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_topics`
--

LOCK TABLES `forum_topics` WRITE;
/*!40000 ALTER TABLE `forum_topics` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_topics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_topics_watch`
--

LOCK TABLES `forum_topics_watch` WRITE;
/*!40000 ALTER TABLE `forum_topics_watch` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_topics_watch` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_user_group`
--

LOCK TABLES `forum_user_group` WRITE;
/*!40000 ALTER TABLE `forum_user_group` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_user_group` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_users`
--

LOCK TABLES `forum_users` WRITE;
/*!40000 ALTER TABLE `forum_users` DISABLE KEYS */;
INSERT INTO `forum_users` VALUES (1,1,'God',0,'CRfY/KDIR/OgQ','localhost',1,1,0,0,0,1248953345,0,0,0.00,0,'0','d M Y H:i',0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,0,NULL,0,'test@example.com',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,66,0,0,0,0,0,0,NULL,0,'4cb9c8a8048fd02294477fcb1a41191a',0,0,0);
/*!40000 ALTER TABLE `forum_users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_vote_desc`
--

LOCK TABLES `forum_vote_desc` WRITE;
/*!40000 ALTER TABLE `forum_vote_desc` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_vote_desc` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_vote_results`
--

LOCK TABLES `forum_vote_results` WRITE;
/*!40000 ALTER TABLE `forum_vote_results` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_vote_results` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_vote_voters`
--

LOCK TABLES `forum_vote_voters` WRITE;
/*!40000 ALTER TABLE `forum_vote_voters` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_vote_voters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `forum_words`
--

LOCK TABLES `forum_words` WRITE;
/*!40000 ALTER TABLE `forum_words` DISABLE KEYS */;
/*!40000 ALTER TABLE `forum_words` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `helpfiles`
--

LOCK TABLES `helpfiles` WRITE;
/*!40000 ALTER TABLE `helpfiles` DISABLE KEYS */;
/*!40000 ALTER TABLE `helpfiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `hobbitmail`
--

LOCK TABLES `hobbitmail` WRITE;
/*!40000 ALTER TABLE `hobbitmail` DISABLE KEYS */;
/*!40000 ALTER TABLE `hobbitmail` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `massmail_messages`
--

LOCK TABLES `massmail_messages` WRITE;
/*!40000 ALTER TABLE `massmail_messages` DISABLE KEYS */;
/*!40000 ALTER TABLE `massmail_messages` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `mobAI`
--

LOCK TABLES `mobAI` WRITE;
/*!40000 ALTER TABLE `mobAI` DISABLE KEYS */;
/*!40000 ALTER TABLE `mobAI` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `mob_progs`
--

LOCK TABLES `mob_progs` WRITE;
/*!40000 ALTER TABLE `mob_progs` DISABLE KEYS */;
/*!40000 ALTER TABLE `mob_progs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `mob_progs_backup`
--

LOCK TABLES `mob_progs_backup` WRITE;
/*!40000 ALTER TABLE `mob_progs_backup` DISABLE KEYS */;
/*!40000 ALTER TABLE `mob_progs_backup` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `mob_resets`
--

LOCK TABLES `mob_resets` WRITE;
/*!40000 ALTER TABLE `mob_resets` DISABLE KEYS */;
/*!40000 ALTER TABLE `mob_resets` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `mvariables`
--

LOCK TABLES `mvariables` WRITE;
/*!40000 ALTER TABLE `mvariables` DISABLE KEYS */;
/*!40000 ALTER TABLE `mvariables` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `new_crafts`
--

LOCK TABLES `new_crafts` WRITE;
/*!40000 ALTER TABLE `new_crafts` DISABLE KEYS */;
/*!40000 ALTER TABLE `new_crafts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `newsletter_issue`
--

LOCK TABLES `newsletter_issue` WRITE;
/*!40000 ALTER TABLE `newsletter_issue` DISABLE KEYS */;
/*!40000 ALTER TABLE `newsletter_issue` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `newsletter_stats`
--

LOCK TABLES `newsletter_stats` WRITE;
/*!40000 ALTER TABLE `newsletter_stats` DISABLE KEYS */;
/*!40000 ALTER TABLE `newsletter_stats` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `newsletters`
--

LOCK TABLES `newsletters` WRITE;
/*!40000 ALTER TABLE `newsletters` DISABLE KEYS */;
/*!40000 ALTER TABLE `newsletters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `obj_progs`
--

LOCK TABLES `obj_progs` WRITE;
/*!40000 ALTER TABLE `obj_progs` DISABLE KEYS */;
/*!40000 ALTER TABLE `obj_progs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `obj_progs_backup`
--

LOCK TABLES `obj_progs_backup` WRITE;
/*!40000 ALTER TABLE `obj_progs_backup` DISABLE KEYS */;
/*!40000 ALTER TABLE `obj_progs_backup` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `player_journals`
--

LOCK TABLES `player_journals` WRITE;
/*!40000 ALTER TABLE `player_journals` DISABLE KEYS */;
INSERT INTO `player_journals` VALUES ('God',1,'My Background.','God','Thu Jul  5 18:41:51 2012','This is God.\n'),('God',2,'My Description','God','Thu Jul  5 18:41:51 2012','   God is here. \n');
/*!40000 ALTER TABLE `player_journals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `player_notes`
--

LOCK TABLES `player_notes` WRITE;
/*!40000 ALTER TABLE `player_notes` DISABLE KEYS */;
INSERT INTO `player_notes` VALUES ('God-Guest',1,'Server Crash [4500: pid 3440]','Server','Thu Jul  5 18:41:08 2012','God-Guest [121]: show l\r\n\r\nType \'debug 3440\' on the shell to retrieve full stack dump.\r\n',0,1341542468),('God',1,'Background Information.','God','Thu Jul  5 18:41:51 2012','This is God.\n',0,1341542511),('God',2,'Description Backup.','God','Thu Jul  5 18:41:51 2012','   God is here. \n',0,1341542511);
/*!40000 ALTER TABLE `player_notes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `player_writing`
--

LOCK TABLES `player_writing` WRITE;
/*!40000 ALTER TABLE `player_writing` DISABLE KEYS */;
/*!40000 ALTER TABLE `player_writing` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `players_online`
--

LOCK TABLES `players_online` WRITE;
/*!40000 ALTER TABLE `players_online` DISABLE KEYS */;
/*!40000 ALTER TABLE `players_online` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `plot_log`
--

LOCK TABLES `plot_log` WRITE;
/*!40000 ALTER TABLE `plot_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `plot_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `poisons`
--

LOCK TABLES `poisons` WRITE;
/*!40000 ALTER TABLE `poisons` DISABLE KEYS */;
/*!40000 ALTER TABLE `poisons` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `professions`
--

LOCK TABLES `professions` WRITE;
/*!40000 ALTER TABLE `professions` DISABLE KEYS */;
/*!40000 ALTER TABLE `professions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `queued_reviews`
--

LOCK TABLES `queued_reviews` WRITE;
/*!40000 ALTER TABLE `queued_reviews` DISABLE KEYS */;
/*!40000 ALTER TABLE `queued_reviews` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `queued_startups`
--

LOCK TABLES `queued_startups` WRITE;
/*!40000 ALTER TABLE `queued_startups` DISABLE KEYS */;
/*!40000 ALTER TABLE `queued_startups` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `queued_submissions`
--

LOCK TABLES `queued_submissions` WRITE;
/*!40000 ALTER TABLE `queued_submissions` DISABLE KEYS */;
/*!40000 ALTER TABLE `queued_submissions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `race_rpp_data`
--

LOCK TABLES `race_rpp_data` WRITE;
/*!40000 ALTER TABLE `race_rpp_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `race_rpp_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `races`
--

LOCK TABLES `races` WRITE;
/*!40000 ALTER TABLE `races` DISABLE KEYS */;
INSERT INTO `races` VALUES ('Default',105,0,'This is the default race.',0,1,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,'0','0',0,0,0,0,0,0,0,0,0,0,0,0,0);
/*!40000 ALTER TABLE `races` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `reboot_mobiles`
--

LOCK TABLES `reboot_mobiles` WRITE;
/*!40000 ALTER TABLE `reboot_mobiles` DISABLE KEYS */;
/*!40000 ALTER TABLE `reboot_mobiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `registered_characters`
--

LOCK TABLES `registered_characters` WRITE;
/*!40000 ALTER TABLE `registered_characters` DISABLE KEYS */;
/*!40000 ALTER TABLE `registered_characters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `resolved_hosts`
--

LOCK TABLES `resolved_hosts` WRITE;
/*!40000 ALTER TABLE `resolved_hosts` DISABLE KEYS */;
INSERT INTO `resolved_hosts` VALUES ('127.0.0.1','localhost',1341540687);
/*!40000 ALTER TABLE `resolved_hosts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `reviews_in_progress`
--

LOCK TABLES `reviews_in_progress` WRITE;
/*!40000 ALTER TABLE `reviews_in_progress` DISABLE KEYS */;
/*!40000 ALTER TABLE `reviews_in_progress` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `saveroom_objs`
--

LOCK TABLES `saveroom_objs` WRITE;
/*!40000 ALTER TABLE `saveroom_objs` DISABLE KEYS */;
/*!40000 ALTER TABLE `saveroom_objs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `scents`
--

LOCK TABLES `scents` WRITE;
/*!40000 ALTER TABLE `scents` DISABLE KEYS */;
/*!40000 ALTER TABLE `scents` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `server_statistics`
--

LOCK TABLES `server_statistics` WRITE;
/*!40000 ALTER TABLE `server_statistics` DISABLE KEYS */;
/*!40000 ALTER TABLE `server_statistics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `skills`
--

LOCK TABLES `skills` WRITE;
/*!40000 ALTER TABLE `skills` DISABLE KEYS */;
/*!40000 ALTER TABLE `skills` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `special_orders`
--

LOCK TABLES `special_orders` WRITE;
/*!40000 ALTER TABLE `special_orders` DISABLE KEYS */;
/*!40000 ALTER TABLE `special_orders` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `special_roles`
--

LOCK TABLES `special_roles` WRITE;
/*!40000 ALTER TABLE `special_roles` DISABLE KEYS */;
/*!40000 ALTER TABLE `special_roles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `special_roles_outfit`
--

LOCK TABLES `special_roles_outfit` WRITE;
/*!40000 ALTER TABLE `special_roles_outfit` DISABLE KEYS */;
/*!40000 ALTER TABLE `special_roles_outfit` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `spells`
--

LOCK TABLES `spells` WRITE;
/*!40000 ALTER TABLE `spells` DISABLE KEYS */;
/*!40000 ALTER TABLE `spells` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `staff_roster`
--

LOCK TABLES `staff_roster` WRITE;
/*!40000 ALTER TABLE `staff_roster` DISABLE KEYS */;
/*!40000 ALTER TABLE `staff_roster` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `stayput_mobiles`
--

LOCK TABLES `stayput_mobiles` WRITE;
/*!40000 ALTER TABLE `stayput_mobiles` DISABLE KEYS */;
/*!40000 ALTER TABLE `stayput_mobiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `testimonials`
--

LOCK TABLES `testimonials` WRITE;
/*!40000 ALTER TABLE `testimonials` DISABLE KEYS */;
/*!40000 ALTER TABLE `testimonials` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `tracks`
--

LOCK TABLES `tracks` WRITE;
/*!40000 ALTER TABLE `tracks` DISABLE KEYS */;
/*!40000 ALTER TABLE `tracks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `unneeded_helpfiles`
--

LOCK TABLES `unneeded_helpfiles` WRITE;
/*!40000 ALTER TABLE `unneeded_helpfiles` DISABLE KEYS */;
/*!40000 ALTER TABLE `unneeded_helpfiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `variables`
--

LOCK TABLES `variables` WRITE;
/*!40000 ALTER TABLE `variables` DISABLE KEYS */;
/*!40000 ALTER TABLE `variables` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `virtual_boards`
--

LOCK TABLES `virtual_boards` WRITE;
/*!40000 ALTER TABLE `virtual_boards` DISABLE KEYS */;
INSERT INTO `virtual_boards` VALUES ('Crashes',1,'Server Crash [4500: pid 3440]','God-Guest','Thu Jul  5 18:41:08 2012','God-Guest [121]: show l\r\n\r\nType \'debug 3440\' on the shell to retrieve full stack dump.\r\n',1341542468);
/*!40000 ALTER TABLE `virtual_boards` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `vote_notifications`
--

LOCK TABLES `vote_notifications` WRITE;
/*!40000 ALTER TABLE `vote_notifications` DISABLE KEYS */;
/*!40000 ALTER TABLE `vote_notifications` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `vote_tracking`
--

LOCK TABLES `vote_tracking` WRITE;
/*!40000 ALTER TABLE `vote_tracking` DISABLE KEYS */;
/*!40000 ALTER TABLE `vote_tracking` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `website_errors`
--

LOCK TABLES `website_errors` WRITE;
/*!40000 ALTER TABLE `website_errors` DISABLE KEYS */;
/*!40000 ALTER TABLE `website_errors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `website_links`
--

LOCK TABLES `website_links` WRITE;
/*!40000 ALTER TABLE `website_links` DISABLE KEYS */;
/*!40000 ALTER TABLE `website_links` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `website_log`
--

LOCK TABLES `website_log` WRITE;
/*!40000 ALTER TABLE `website_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `website_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `website_referrals`
--

LOCK TABLES `website_referrals` WRITE;
/*!40000 ALTER TABLE `website_referrals` DISABLE KEYS */;
/*!40000 ALTER TABLE `website_referrals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `website_sessions`
--

LOCK TABLES `website_sessions` WRITE;
/*!40000 ALTER TABLE `website_sessions` DISABLE KEYS */;
/*!40000 ALTER TABLE `website_sessions` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-07-05 18:49:00
