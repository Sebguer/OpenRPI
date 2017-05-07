<html>
<body>

<?php //edit.php
require_once 'login.php';

$db_server = mysql_connect($db_hostname, $db_username, $db_password);
mysql_select_db($db_database); 

if (!$db_server) die("Unable to connect to MySQL: " . mysql_error());



   if (!isset($_POST["edit"]) && !isset($_POST["delete"]))
   {
      $id = $_GET["id"];
      $sql = "SELECT * FROM races WHERE id=$id";
      $result = mysql_query($sql);        
      $myrow = mysql_fetch_array($result);
	}
      ?>
	 
	  
      <form action="edit.php" method="post">
      <input type=hidden name="id" value="<?php echo $myrow["id"] ?>">

<TABLE>
<TR>
   <TD>Name:</TD>
   <TD><INPUT TYPE='TEXT' NAME='name' VALUE="<?php echo $myrow["name"] ?>" size=60></TD>
</TR>
<TR>
   <TD>RPP Cost:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='rpp_cost' VALUE="<?php echo $myrow["rpp_cost"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Description:</TD>
   <TD><TEXTAREA NAME='description' ROWS=10 COLS=65><?php echo $myrow['description'] ?></TEXTAREA><br></TD>
</TR>
<TR>
   <TD>PC Race:</TD>
   <TD>
      <SELECT NAME='pc_race' SELECTED=>
         <OPTION VALUE='1' <?php if ($myrow["pc_race"]==1) echo "SELECTED";?>>1 - Yes
         <OPTION VALUE='0' <?php if ($myrow["pc_race"]==0) echo "SELECTED";?>>0 - No		
      </SELECT>
   </TD>
</TR>
<TR>
   <TD>Body Type:</TD>
   <TD>
      <SELECT NAME='body_proto'>
         <OPTION VALUE='0' <?php if ($myrow["body_proto"]==0) echo "SELECTED";?>>0 - Humanoid
         <OPTION VALUE='1' <?php if ($myrow["body_proto"]==1) echo "SELECTED";?>>1 - Fourlegged w/ Paws	
         <OPTION VALUE='2' <?php if ($myrow["body_proto"]==2) echo "SELECTED";?>>2 - Fourlegged w/ Hooves
         <OPTION VALUE='3' <?php if ($myrow["body_proto"]==3) echo "SELECTED";?>>3 - Fourlegged w/ Feet
         <OPTION VALUE='4' <?php if ($myrow["body_proto"]==4) echo "SELECTED";?>>4 - Winged w/ Tail		
         <OPTION VALUE='5' <?php if ($myrow["body_proto"]==5) echo "SELECTED";?>>5 - Winged w/ No Tail	
         <OPTION VALUE='6' <?php if ($myrow["body_proto"]==6) echo "SELECTED";?>>6 - Serpentine
         <OPTION VALUE='7' <?php if ($myrow["body_proto"]==7) echo "SELECTED";?>>7 - Arachnid/Insect
         <OPTION VALUE='8' <?php if ($myrow["body_proto"]==8) echo "SELECTED";?>>8 - Ghost/Incorporal
         <OPTION VALUE='9' <?php if ($myrow["body_proto"]==9) echo "SELECTED";?>>9 - Structure
         <OPTION VALUE='10' <?php if ($myrow["body_proto"]==10) echo "SELECTED";?>>10 - Ent
         <OPTION VALUE='11' <?php if ($myrow["body_proto"]==11) echo "SELECTED";?>>11 - Robot
      </SELECT>
   </TD>
</TR>
<TR>
   <TD>Size:</TD>
   <TD>
      <SELECT NAME='size'>
         <OPTION VALUE='-3' <?php if ($myrow["size"]==-3) echo "SELECTED";?>>-3 - XXS - Insect
         <OPTION VALUE='-2' <?php if ($myrow["size"]==-2) echo "SELECTED";?>>-2 - XS - Rodent, Rabbit
         <OPTION VALUE='-1' <?php if ($myrow["size"]==-1) echo "SELECTED";?>>-1 - S - Dog, Cat
         <OPTION VALUE='0' <?php if ($myrow["size"]==0) echo "SELECTED";?>>0 - A - Human Sized
         <OPTION VALUE='1' <?php if ($myrow["size"]==1) echo "SELECTED";?>>1 - L - Troll, Ogre		
         <OPTION VALUE='2' <?php if ($myrow["size"]==2) echo "SELECTED";?>>2 - XL - Dragon, Giant
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Group Noun:</TD>
   <TD>
      <SELECT NAME='group_noun'>
         <OPTION VALUE='0' <?php if ($myrow["group_noun"]==0) echo "SELECTED";?>>0 - Humans
         <OPTION VALUE='1' <?php if ($myrow["group_noun"]==1) echo "SELECTED";?>>1 - Humanoids
         <OPTION VALUE='2' <?php if ($myrow["group_noun"]==2) echo "SELECTED";?>>2 - Insects
         <OPTION VALUE='3' <?php if ($myrow["group_noun"]==3) echo "SELECTED";?>>3 - Automatons
         <OPTION VALUE='4' <?php if ($myrow["group_noun"]==4) echo "SELECTED";?>>4 - Bots
         <OPTION VALUE='5' <?php if ($myrow["group_noun"]==5) echo "SELECTED";?>>5 - Vehicles
         <OPTION VALUE='6' <?php if ($myrow["group_noun"]==6) echo "SELECTED";?>>6 - Dogs
         <OPTION VALUE='7' <?php if ($myrow["group_noun"]==7) echo "SELECTED";?>>7 - Lizards
         <OPTION VALUE='8' <?php if ($myrow["group_noun"]==8) echo "SELECTED";?>>8 - Spiders
         <OPTION VALUE='9' <?php if ($myrow["group_noun"]==9) echo "SELECTED";?>>9 - Rodents
         <OPTION VALUE='10' <?php if ($myrow["group_noun"]==10) echo "SELECTED";?>>10 - Bats
         <OPTION VALUE='11' <?php if ($myrow["group_noun"]==11) echo "SELECTED";?>>11 - Animals
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Tracks:</TD>
   <TD>
      <SELECT NAME='tracks'>
         <OPTION VALUE='0' <?php if ($myrow["tracks"]==0) echo "SELECTED";?>>0 - Ordinary
         <OPTION VALUE='1' <?php if ($myrow["tracks"]==1) echo "SELECTED";?>>1 - Only-If-Bleeding
         <OPTION VALUE='2' <?php if ($myrow["tracks"]==2) echo "SELECTED";?>>2 - Never
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Corpse:</TD>
   <TD>
      <SELECT NAME='corpse'>
         <OPTION VALUE='0' <?php if ($myrow["corpse"]==0) echo "SELECTED";?>>0 - Leave a Corpse
         <OPTION VALUE='1' <?php if ($myrow["corpse"]==1) echo "SELECTED";?>>1 - Disintegrate into skinned vnum
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Type:</TD>
   <TD>
      <SELECT NAME='type'>
         <OPTION VALUE='0' <?php if ($myrow["type"]==0) echo "SELECTED";?>>0 - We're a normal alive, in-game mobile
         <OPTION VALUE='1' <?php if ($myrow["type"]==1) echo "SELECTED";?>>1 - We're ingame, but not alive/sentient
         <OPTION VALUE='2' <?php if ($myrow["type"]==2) echo "SELECTED";?>>2 - We're not in-game.
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Nomad:</TD>
   <TD>
      <SELECT NAME='nomad'>
         <OPTION VALUE='0' <?php if ($myrow["nomad"]==0) echo "SELECTED";?>>0 - We're not a nomad or nomad-based lifeform
         <OPTION VALUE='1' <?php if ($myrow["nomad"]==1) echo "SELECTED";?>>1 - We are a nomad or nomad-based lifeform.
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Alert Sound:</TD>
   <TD>
      <SELECT NAME='alert'>
         <OPTION VALUE='0' <?php if ($myrow["alert"]==0) echo "SELECTED";?>>0 - Normal Alert - Whistle
         <OPTION VALUE='1' <?php if ($myrow["alert"]==1) echo "SELECTED";?>>1 - Howl
         <OPTION VALUE='2' <?php if ($myrow["alert"]==2) echo "SELECTED";?>>2 - Screech
         <OPTION VALUE='3' <?php if ($myrow["alert"]==3) echo "SELECTED";?>>3 - Smell
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Movement Echo:</TD>
   <TD>
      <SELECT NAME='movement'>
         <OPTION VALUE='0' <?php if ($myrow["movement"]==0) echo "SELECTED";?>>0 - Normal
         <OPTION VALUE='1' <?php if ($myrow["movement"]==1) echo "SELECTED";?>>1 - Fly
         <OPTION VALUE='2' <?php if ($myrow["movement"]==2) echo "SELECTED";?>>2 - Slither
         <OPTION VALUE='3' <?php if ($myrow["movement"]==3) echo "SELECTED";?>>3 - Scuttle
         <OPTION VALUE='4' <?php if ($myrow["movement"]==4) echo "SELECTED";?>>4 - Drift
      </SELECT>
   </TD>
</TR>

<TR>
   <TD>Affects:</TD>
   <TD>
      Infravision:<input type="checkbox" value=1 name="affects1" <?php if ($myrow["affects"] & 1) echo "CHECKED";?>><br>
      Flying:<input type="checkbox" value=2 name="affects2" <?php if ($myrow["affects"] & 2) echo "CHECKED";?>><br>
      Water Breathe:<input type="checkbox" value=4 name="affects3" <?php if ($myrow["affects"] & 4) echo "CHECKED";?>><br>
      NoBleed:<input type="checkbox" value=8 name="affects4" <?php if ($myrow["affects"] & 8) echo "CHECKED";?>><br>
      Sun Penalty:<input type="checkbox" value=16 name="affects5" <?php if ($myrow["affects"] & 16) echo "CHECKED";?>><br>
      Sun Petrify:<input type="checkbox" value=32 name="affects6" <?php if ($myrow["affects"] & 32) echo "CHECKED";?>><br>
      Prey:<input type="checkbox" value=64 name="affects7" <?php if ($myrow["affects"] & 64) echo "CHECKED";?>><br>
      Stalker:<input type="checkbox" value=128 name="affects8" <?php if ($myrow["affects"] & 128) echo "CHECKED";?>><br>
   </TD>
</TR>

<TR>
   <TD>Robot Options:</TD>
   <TD>
      Passive [no AI routines]:<input type="checkbox" value=1 name="bot1"><br>
      Remote Controllable:<input type="checkbox" value=2 name="bot2"><br>
      Monitorable:<input type="checkbox" value=4 name="bot3"><br>
   </TD>
</TR>

<TR>
   <TD>Door Options:</TD>
   <TD>
      Can Open Doors:<input type="checkbox" value=1 name="door_bits1"><br>
      Can Close Doors:<input type="checkbox" value=2 name="door_bits2"><br>
   </TD>
</TR>

<TR>
   <TD>Strength Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='str_mod' VALUE="<?php echo $myrow["str_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Constitution Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='con_mod' VALUE="<?php echo $myrow["con_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Dexterity Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='dex_mod' VALUE="<?php echo $myrow["dex_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Agility Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='agi_mod' VALUE="<?php echo $myrow["agi_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Intelligence Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='int_mod' VALUE="<?php echo $myrow["int_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Willpower Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='wil_mod' VALUE="<?php echo $myrow["wil_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Aura Modifier:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='aur_mod' VALUE="<?php echo $myrow["aur_mod"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Minimum Age:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='min_age' VALUE="<?php echo $myrow["min_age"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Maximum Age:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='max_age' VALUE="<?php echo $myrow["max_age"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Minimum Height:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='min_height' VALUE="<?php echo $myrow["min_height"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Maximum Height:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='max_height' VALUE="<?php echo $myrow["max_height"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Female Height Difference:</TD>
   <TD><INPUT TYPE='NUMBER' NAME='fem_ht_diff' VALUE="<?php echo $myrow["fem_ht_diff"] ?>" size=5></TD>
</TR>
<TR>
   <TD>Native Tongue:</TD>
   <TD>
      <SELECT NAME='native_tongue'>
         <OPTION VALUE='45' <?php if ($myrow["native_tongue"]==45) echo "SELECTED";?>>45 - Common - only enabled language
      </SELECT>
   </TD>
</TR>
<TR>
   <TD>Created By:</TD>
   <TD>
      <SELECT NAME='created_by'>
         <OPTION VALUE='Holmes'>Holmes
         <OPTION VALUE='Mercury'>Mercury
      </SELECT>
   </TD>
</TR>
</TABLE>
   
      <input type="submit" name="edit" value="edit">
      <input type="submit" name="delete" value="delete">
   
      </form>
   
<? } ?>
<?php
   if (isset($_POST["edit"]))
   {
      $id = $_POST["id"];
      $name = $_POST["name"];
      $rpp_cost = $_POST["rpp_cost"];
      $description = $_POST["description"];
      $pc_race = $_POST["pc_race"];
      $affects = $_POST["affects1"] + $_POST["affects2"] + $_POST["affects3"] + $_POST["affects4"] + $_POST["affects5"] + $_POST["affects6"] + $_POST["affects7"] + $_POST["affects8"];
      $body_proto = $_POST["body_proto"];
      $size = $_POST["size"];
      $str_mod = $_POST["str_mod"];
      $con_mod = $_POST["con_mod"];
      $dex_mod = $_POST["dex_mod"];
      $agi_mod = $_POST["agi_mod"];
      $int_mod = $_POST["int_mod"];
      $wil_mod = $_POST["wil_mod"];
      $aur_mod = $_POST["aur_mod"];
      $min_age = $_POST["min_age"];
      $max_age = $_POST["max_age"];
      $min_height = $_POST["min_height"];
      $max_height = $_POST["max_height"];
      $fem_ht_diff = $_POST["fem_ht_diff"];
      $native_tongue = $_POST["native_tongue"];
      $created_by = $_POST["created_by"];
      $last_modified = "0";
      $group_noun = $_POST["group_noun"];
      $tracks = $_POST["tracks"];
      $corpse = $_POST["corpse"];
      $type = $_POST["type"];
      $door_bits = $_POST["door_bits1"] + $_POST["door_bits2"];
      $bot = $_POST["bot1"] + $_POST["bot2"] + $_POST["bot3"];
      $nomad = $_POST["nomad"];
      $alert = $_POST["alert"];
      $movement = $_POST["movement"];

      $last_modified = date("jmy");

      $sql = "UPDATE races SET name='$name', rpp_cost='$rpp_cost', description='$description', pc_race='$pc_race', affects='$affects', body_proto='$body_proto', size='$size', 
                               str_mod='$str_mod', con_mod='$con_mod', dex_mod='$dex_mod', agi_mod='$agi_mod', int_mod='$int_mod', wil_mod='$wil_mod', aur_mod='$aur_mod',
                               min_age='$min_age', max_age='$max_age', min_height='$min_height', max_height='$max_height', fem_ht_diff='$fem_ht_diff', 
                               native_tongue='$native_tongue', created_by='$created_by', last_modified='$last_modified', group_noun='$group_noun', tracks='$tracks',
			       corpse='$corpse', type='$type', door_bits='$door_bits', bot='$bot', nomad='$nomad', alert='$alert', movement='$movement'
                               where id=$id";

      $result = mysql_query($sql);
      echo "Race Updating - updates will require a reboot on the port to take affect.";
    echo "<meta http-equiv='refresh' content='1;URL=editor.php?id=" . $id . "'>";
   }
   else if (isset($_POST["delete"]))
   {
    $id = $_POST["id"];
    $sql = "DELETE FROM races WHERE id=$id";
    $result = mysql_query($sql);
    echo "Race Deleted!";
   }


?>

<br><br><b><a href='editor.php'>Back to Index</a></b><br>

</body>
</html>