#Basic Instructions to Run a Server on DigitalOcean

##Step 1, purchase a Droplet.

Go to http://www.digitalocean.com and register an account.

Add a Credit Card, or a PayPal account. If you ask me, I can probably give you a $10 promo code, 
but you'll still need to do one of those two things in order to acivate your account.

##Step 2, spin up your Droplet.

Click the Create button on the control panel.

Select your Hostname at the top. If you have a domain purchased somewhere, go ahead and use that domain as your hostname.

Select your preferred size. The smallest should be adequate.

Select Ubuntu 14.04 x64.

If you know how to use SSH keys, you can add one to your account and configure it so you don't need a password. Otherwise,
make sure VirtIO and enable backups are checked. DigitalOCean doesn't charge for backups at the moment, so no reason not to.

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

apt-get install -y mysql-server gcc git

In the middle of the installation, it will prompt you for a MySQL password. Choose something, and make a note of it.

When the installation finishes, you're going to want to add the databases and a new MySQL user for the game to use.

First, though, let's secure MySQL by typing: mysql_secure_installation

It'll prompt you for the root password for MySQL and then ask you several questions.

You can select yes for everything except changing the root password.

Afterwards, we'll add a user for the game to MySQL, and create the databases for it to use.

mysql -u root -p
<your password>

You should be presented with:

mysql>

Here you'll type:

CREATE USER 'rpiadmin'@'localhost' IDENTIFIED BY 'yourpassword';
CREATE DATABASE 'rpi_engine';
CREATE DATABASE 'rpi_player';
CREATE DATABASE 'rpi_player_log';
CREATE DATABASE 'rpi_world_log';
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
git clone http://gitlab.michaelguer.in/Sebguer/rpi-engine.git

Use your gitlab username and password.

Once it's finished type:
...
