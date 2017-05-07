<html>
<body>

<?php 
//add.php

require_once 'login.php';

if(isset($_POST['submit'])) //If submit is hit
{$db_server = mysql_connect($db_hostname, $db_username, $db_password);
mysql_select_db($db_database); 
   $sql = MYSQL_QUERY("Select * from races");
   $rows = mysql_num_rows($sql);
   $id = $rows + 1;
  
   //convert all the posts to variables:
   $name = $_POST["name"];
   $created_by = $_POST["created_by"];
   
   //Insert the values into the correct database with the right fields

   $sql2 = "INSERT INTO races (name, created_by, last_modified) VALUES ('$name', '$created_by', '0')";
   $result = mysql_query($sql2);

    //confirm
   echo "Query Finished - race will not be available on the ports until a reboot."; 
   echo "<meta http-equiv='refresh' content='1;URL=editor.php'>";
}

// close php so we can put in our code
?>

<input type=hidden name="id" value="<?php echo $myrow["id"] ?>">

<form method="post" action="add.php">
	
<TABLE>
<TR>
   <TD>Name:</TD>
   <TD><INPUT TYPE='TEXT' NAME='name' VALUE='Random Update' size=60></TD>
</TR>
<TR>
   <TD>Created By:</TD>
   <TD>
      <SELECT NAME='created_by'>
         <OPTION VALUE='Holmes'>Holmes
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