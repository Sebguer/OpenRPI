Read the licenses. Don't like them, don't run the MUD.

#Basic Instructions to Run a Server on DigitalOcean

##Step 1, purchase a Droplet.

Go to http://www.digitalocean.com and register an account.

##Step 2, spin up your Droplet.b

Select your Hostname at the top. If you have a domain purchased somewhere, go ahead and use that domain as your hostname.

Select your preferred size. The smallest should be adequate.

Select Ubuntu 14.04 x64.

If you know how to use SSH keys, you can add one to your account and configure it so you don't need a password. Otherwise,
make sure VirtIO is checked. Consider enable backups. The pricing for backups is 20% of the cost of the virtual server. So if you want to enable backups for a $5/mo virtual server, the cost for backups will be $1/mo. The price for snapshots will be $0.02 per GB of snapshot storage per month.

Hit Create.

##Step 3, log in.

Once your Droplet's created, you'll get an e-mail with the root password. At this point, if you're using a Mac or Linux locally,
you can use your terminal to ssh in:

ssh root@youripaddress

It'll prompt for the password.

If you're using Windows, you'll need to install an SSH client. I prefer Bitvise. https://www.bitvise.com/

Once you've installed it, put your IP as the host, port 22, and for authentication put root and initial method password.

Then hit connect.


##Step 4, configuration!

At this point, you should be greeted by root@hostname:~#

First thing you should do is type apt-get update. That'll update your mirrors so that you can download the next packages we need.

Once it's finished, type:

apt-get install libmysqlclient-dev

apt-get install -y mysql-server gcc git

In the middle of the installation, it will prompt you for a MySQL password.
Choose something, and make a note of it.


When the installation finishes, you're going to want to add the databases and a new MySQL user for the game to use.

First, though, let's secure MySQL by typing: mysql_secure_installation

It'll prompt you for the root password for MySQL and then ask you several questions.

You can select yes for everything except changing the root password.

Afterwards, we'll add a user for the game to MySQL, and create the databases for it to use.

mysql -u root -p
<your password>

You should be presented with:

mysql>

Note! *RPI has been pre-configured to use 'rpiadmin' identified with 'opensource' in MySQL. If you use anything else, you'll have an additional step to do. It's at the end.

 Here you'll type:


CREATE USER 'rpiadmin'@'localhost' IDENTIFIED BY 'yourpassword';
CREATE DATABASE rpi_engine;
CREATE DATABASE rpi_player;
CREATE DATABASE rpi_player_log;
CREATE DATABASE rpi_world_log;
GRANT ALL PRIVILEGES ON rpi_engine.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_player.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_player_log.* to 'rpiadmin'@'localhost';
GRANT ALL PRIVILEGES ON rpi_world_log.* to 'rpiadmin'@'localhost';

After that, type:

\quit

Now we're going to add the rpi user and bring over the game itself.

adduser rpiadmin

Assign it a password, and remember that password. You can ignore the rest of the questions.

Now type:

cd ~
git clone https://github.com/YourCyborg/SORPI.git

Now we're going to add all of the sql files in *RPI into those databases you just made.

Load each database in *RPI/generic/sql

We can do this with

mysql -u rpiadmin -p -h localhost DBASE < NAME.sql

To do all of this, type:

cd /root/*RPI/generic/sql



mysql -u rpiadmin -p -h localhost rpi_engine < rpi_engine.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player_log < rpi_player_log.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player < rpi_player.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_world_log < rpi_world_log.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_player < rpi_player_test_account.sql
Enter password:
mysql -u rpiadmin -p -h localhost rpi_engine < rpi_engine_test_account.sql
Enter password:

Now type:

cd ..

cd ..

cd src

make

This will take a little bit, but not too long. When it's done type:

cd ..

And then the following to start the server:

./start-server

It'll return options for you. Pick one. I.e.

./start-server  pp &

The pp port is 4500. Connect, type in god, password changeme. No 'help' command functionality at the moment. *RPI/src/commands.cpp is your friend. Also, staff.cpp and olc.cpp.

If you chose a username for MySQL other than rpiadmin or it was identified by anything other than opensource:

Search the files within a directory and subdirectories for a string.
grep -rnw '*RPI' -e "yourpassword"
grep -rnw '*RPI' -e "yourusername"

Make note of where it is and make the necessary changes.

Other useful things to know:
* ./src is where the code is.
* ./lib is where all instance-data is kept, things like objects, orders, stayput mobiles, etc.
* ./generic has nothing but the template .sql files.
* ./regions is where all the worldfiles are saved, along with the registry and a few other odds and ends.
* ./crashes is where one-half of the crash-logs are kept. ./lib is the other. Someone broke gdb-parsing of crash-logs a while back: you're going to try and fix that ASAP unless you want even more mysterious crashes.
* ./bin is where the executable is stored.
* ./tmp is where output from the server is kept.

* ./lib/text is a variety of texts people see when navigating through the menus of the MUD.
* ./regions/registry is where all the skill formulas are kept.

You probably want to create at least two instances of the file structure - a playerport, and a buildport. Different rules apply to the playerport and buildport, namely, everything is frozen on the buildport and you can't make permanent changes to the worldfiles on the playerport.

You're also going to have to re-enable account creation, set up a mail server, and a whole bunch of other stuff too.

Useful Linux commands:
copy a directory with:
cp -r pp pp_bak

Remove a directory with rm -rf pp
copy the contents a directory with cp -r tp_bak/. tp
(source, destination, remove /. to make the folder OpenRPI_BACKUP inside OpenRPI itself versus the contents)

Search the files within a directory and subdirectories for a string.
grep -rnw 'tp' -e "BlahBlahBlah"
-r is recursive, -n is line number and -w stands match the whole word. Along with these, --exclude or --include parameter could be used for efficient searching.

mysqldump -u myuser --no-create-info databasename > myfile.sql
Then you can just do a find on myfile.sql for the string you want.
