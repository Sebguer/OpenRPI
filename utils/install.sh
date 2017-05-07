#!/bin/bash

# Requirements: Ubuntu 16.04 (probably works in all major revisions)
# This install script should get you a functioning OpenRPI instance.
# No guarantees. This script is very jank.
# This installation does not follow security best practices.
# However, it shouldn't have any glaring holes either.
# At the end, the script will launch the build port on 4501.

exec 2>./orpiinstall.log


# Asks for a root password.
echo "Welcome to the OpenRPI engine."
echo "Please enter a secure password, that you will remember."
echo "It will be used for the MySQL credentials."

read password
password = $password

adduser --disabled-password --gecos "" rpiadmin

export DEBIAN_FRONTEND="noninteractive" # For installation of MySQL

apt-get update && apt-get upgrade -y # Update repositories & upgrade existing packages.

apt-get install -y build-essential libmysqlclient-dev gcc git screen expect make # Install some fundamentals.

# Install MySQL Server in a Non-Interactive mode. Default root password will be "root"
echo "mysql-server-5.6 mysql-server/root_password password $password" | sudo debconf-set-selections
echo "mysql-server-5.6 mysql-server/root_password_again password $password" | sudo debconf-set-selections
apt-get -y install mysql-server

# Run the MySQL Secure Installation wizard Automatically
SECURE_MYSQL=$(expect -c "
set timeout 10
spawn mysql_secure_installation
expect \"Press y|Y for Yes, any other key for No/"
send \"n\r"
expect \"Enter current password for root (enter for none):\"
send \"$password\r\"
expect \"Change the root password?\"
send \"n\r\"
expect \"Remove anonymous users?\"
send \"y\r\"
expect \"Disallow root login remotely?\"
send \"y\r\"
expect \"Remove test database and access to it?\"
send \"y\r\"
expect \"Reload privilege tables now?\"
send \"y\r\"
expect eof
")

echo "$SECURE_MYSQL"

apt-get remove expect -y

sed -i 's/127\.0\.0\.1/0\.0\.0\.0/g' /etc/mysql/my.cnf
mysql -uroot -p"$password" -e 'USE mysql; UPDATE `user` SET `Host`="%" WHERE `User`="root" AND `Host`="localhost"; DELETE FROM `user` WHERE `Host` != "%" AND `User`="root"; FLUSH PRIVILEGES;'

service mysql restart

# Create directory for Engine.
mkdir /home/ORPI/
cd /home/ORPI/

# Git clone the repository.

git clone https://github.com/Sebguer/OpenRPI.git

# Duplicate the server.

mv /home/ORPI/OpenRPI /home/ORPI/pp

echo "This may take a moment, compiling the engine's source..."

cd /home/ORPI/pp/src
make clean
make

sed -ibak -e "s/mysql_passwd = \"password\"/mysql_passwd = \"$password\"/g" /home/ORPI/pp/.rpi_engine/config

cp -r /home/ORPI/pp /home/ORPI/bp
cp -r /home/ORPI/pp /home/ORPI/tp

mysql -uroot -p"$password" -e "create database rpi_engine"
mysql -uroot -p"$password" -e "create database rpi_player"
mysql -uroot -p"$password" -e "create database rpi_player_log"
mysql -uroot -p"$password" -e "create database rpi_world_log"

mysql -uroot -p"$password" rpi_engine < /home/ORPI/pp/generic/sql/rpi_engine.sql
mysql -uroot -p"$password" rpi_engine < /home/ORPI/pp/generic/sql/rpi_engine_test_account.sql
mysql -uroot -p"$password" rpi_player < /home/ORPI/pp/generic/sql/rpi_player.sql
mysql -uroot -p"$password" rpi_player_log < /home/ORPI/pp/generic/sql/rpi_player_log.sql
mysql -uroot -p"$password" rpi_world_log < /home/ORPI/pp/generic/sql/rpi_world_log.sql
mysql -uroot -p"$password" rpi_player < /home/ORPI/pp/generic/sql/rpi_player_test_account.sql
mysql -uroot -p"$password" rpi_engine < /home/ORPI/pp/generic/sql/helpfiles.sql
mysql -uroot -p"$password" rpi_engine < /home/ORPI/pp/generic/sql/variables.sql
mysql -uroot -p"$password" rpi_engine < /home/ORPI/pp/generic/sql/races.sql

sed -ibak -e 's/server_mode = "play"/server_mode = "build"/g' /home/ORPI/bp/.rpi_engine/config
sed -ibak -e 's/server_port = "4500"/server_port = "4501"/g' /home/ORPI/bp/.rpi_engine/config

sed -ibak -e 's/server_mode = "play"/server_mode = "test"/g' /home/ORPI/tp/.rpi_engine/config
sed -ibak -e 's/server_port = "4500"/server_port = "4502"/g' /home/ORPI/tp/.rpi_engine/config

echo "Trying to start the server now..."
cd /home/ORPI/bp
screen -dm sh start-server bp & #Starts the server in a screen session.

ip=`hostname -I | cut -d " " -f 1`
echo "The game should now be running on: IP: $ip Port 4501."