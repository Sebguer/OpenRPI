<html>
<body>

<?php 
//helpfile.php

require_once 'login.php';

if(isset($_POST['submit'])) //If submit is hit
{$db_server = mysql_connect($db_hostname, $db_username, $db_password);
mysql_select_db($db_database); 
   $sql = mysql_query("select * from helpfiles");
   $rows = mysql_num_rows($sql);
   $id = $rows + 1;
   date_default_timezone_set('America/New_York');
   //convert all the posts to variables:
   $name = $_POST["name"];
   $category = $_POST["category"];
   $entry = $_POST["entry"];
   $last_changed = date('m/d/Y h:i:s a', time());
   $related_entries = $_POST["related_entries"];
   $required_level = $_POST["required_level"];
   $changed_by = $_POST["changed_by"];
   
   //Insert the values into the correct database with the right fields

   $sql2 = "INSERT INTO helpfiles (name, category, entry, related_entries, required_level, last_changed, changed_by) VALUES ('$name', '$category', '$entry', '$related_entries', '$required_level', '$last_changed', '$changed_by')";
   $result = mysql_query($sql2);

    //confirm
   echo "Helpfile added, please check formatting in-game."; 
}

// close php so we can put in our code
?>

<input type=hidden name="id" value="<?php echo $myrow["id"] ?>">

<form method="post" action="helpfile.php">
	
<TABLE>
<TR>
   <TD>Name:</TD>
   <TD><INPUT TYPE='TEXT' NAME='name' VALUE='Helpfile Name	' size=60></TD>
</TR>
<TR>
   <TD>Category:</TD>
   <TD>
      <SELECT NAME='category'>
         <OPTION VALUE='Building'>Building
		 <OPTION VALUE='Combat'>Combat
		 <OPTION VALUE='Communication'>Communication
 		 <OPTION VALUE='Crafts'>Crafts
		 <OPTION VALUE='Drugs'>Drugs
		 <OPTION VALUE='Extract'>Extract
		 <OPTION VALUE='General'>General
		 <OPTION VALUE='Information'>Information
		 <OPTION VALUE='Manipulation'>Manipulation
		 <OPTION VALUE='Movement'>Movement
		 <OPTION VALUE='Perception'>Perception
		 <OPTION VALUE='Skills'>Skills
		 <OPTION VALUE='Staff'>Staff
      </SELECT>
   </TD>
</TR>
<TR>
   <TD>Helpfile Text:</TD>
   <TD><TEXTAREA NAME="entry" ROWS=10 COLS=65>Helpfile text. \n for newlines.</TEXTAREA><br></TD>
</TR>
<TR>
   <TD>Related Entries:</TD>
   <TD><INPUT TYPE='TEXT' NAME='related_entries' VALUE='' size=60></TD>
</TR>
<TR>
   <TD>Required Level: (0 for Player-readable	)</TD>
   <TD><INPUT TYPE='TEXT' NAME='required_level' VALUE='0' size=60></TD>
</TR>
<TR>
   <TD>Created By:</TD>
   <TD>
      <SELECT NAME='changed_by'>
         <OPTION VALUE='Holmes'>Holmes
		 <OPTION VALUE='Mercury'>Mercury
      </SELECT>
   </TD>
</TR>
<TR>
   <TD></TD><br>
   <TD><INPUT TYPE="submit" name="submit" value="submit"></TD> 
</TR>
</TABLE>
</form>

<?
} //close the else statement
?>

<br><br><b><a href='editor.php'>Back to Index</a></b><br>

</body>
</html>