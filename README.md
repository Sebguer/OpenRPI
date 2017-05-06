* Read the licenses. Don't like them, don't run the MUD. *


# Installation

I've done some work to make setting up an engine trivial. It comes with all of the helpfiles we had for Parallel and Atonement, as well as some basic variables. 

To install the engine, you'll need a Linux server. I've tested it in Ubuntu 16.04, but all recent LTSes should work without any major problems.

Once logged into your server, you'll just need to run the following two commands:

```
wget https://gist.githubusercontent.com/Sebguer/134b9f80ddc5bdcd749d5618bf3df01d/raw/84d634e8d7c3f031f9a680fd8c39d3d191d99bed/install.sh
sh install.sh
```

# Post Installation

Once this script finishes, you'll have three folders with the engine. One Player Port, one Build Port, and one Testing Port. These default to ports 4500-4502.

In any of these folders you can start the server from the main directory with:

`sh start-server bp &`

Replacing bp with the appropriate type of port (pp, bp, or tp).

### Some useful things to know:

* ./src is where the code is.
* ./lib is where all instance-data is kept, things like objects, orders, stayput mobiles, etc.
* ./generic has nothing but the template .sql files.
* ./regions is where all the worldfiles are saved, along with the registry and a few other odds and ends.
* ./crashes is where one-half of the crash-logs are kept. ./lib is the other. Someone broke gdb-parsing of crash-logs a while back: you're going to try and fix that ASAP unless you want even more mysterious crashes.
* ./bin is where the executable is stored.
* ./tmp is where output from the server is kept.

* ./lib/text is a variety of texts people see when navigating through the menus of the MUD.
* ./regions/registry is where all the skill formulas are kept.

