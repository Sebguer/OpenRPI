<?php // list.php

require_once 'login.php';
$db_server = mysql_connect($db_hostname, $db_username, $db_password);

if (!$db_server) die("Unable to connect to MySQL: " . mysql_error());

mysql_select_db($db_database)
	or die("Unable to select database: " . mysql_error());

$result = mysql_query("select * from races order by id");

if (!$result) die ("Database access failed: " . mysql_error());

while($r=mysql_fetch_array($result))
{	
   //the format is $variable = $r["nameofmysqlcolumn"];
   //modify these to match your mysql table columns

   $id=$r["id"];  
   $name=$r["name"];
   $created_by=$r["created_by"];
   $last_modified=$r["last_modified"];
   
   //display the row
   echo "$id $name $created_by $last_modified <a href='edit.php?id=$id'>Edit</a><br>";
}

echo "<a href='add.php'>Add a Race</a><br>";
echo "<a href='helpfile.php'>Add a Helpfile</a><br>";

?>