/*------------------------------------------------------------------------\
 |  create_mobile.c : Mobile Autocreation Module       www.middle-earth.us |
 |  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
 |  All original code, derived under license from DIKU GAMMA (0.0).        |
 \------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "utility.h"


const char *variable_races[] =
{
    "Survivor",
    "Hosted-Terror",
    "Genetic-Terror1",
    "\n"
};

const char *spider_adj1[] =
{
    "brown-striped",
    "coal-black",
    "grey",
    "red-eyed",
    "albino",
    "grey-striped",
    "black-eyed",
    "beady-eyed",
    "loathesome",
    "vile",
    "red-striped",
    "brown",
    "black-striped",
    "dark-green",
    "thick-carapiced",
    "repulsive",
    "green-striped",
    "dusty-grey",
    "\n"
};

const char *spider_adj2[] =
{
    "long-legged",
    "hairy",
    "hairless",
    "hairy-legged",
    "blotchy",
    "black-haired",
    "spikey",
    "ooze-crusted",
    "squat",
    "spindly-legged",
    "greasy-bodied",
    "dust-covered",
    "bloated",
    "nimble",
    "thick-bodied",
    "smooth",
    "sleek",
    "\n"
};

const char *rat_adj1[] =
{
    "brown",
    "ochre-furred",
    "grey",
    "chalky-coated",
    "charcoal-furred",
    "sable",
    "albino",
    "white",
    "red-eyed",
    "yellow-eyed",
    "black-eyed",
    "glossy-eyed",
    "dull-eyed",
    "long-tailed",
    "short-tailed",
    "tailless",
    "sharp-toothed",
    "toothless",
    "jagged-toothed",
    "rotting-toothed",
    "large-pawed",
    "small-pawed",
    "\n"
};

const char *rat_adj2[] =
{
    "scrawny",
    "emaciated",
    "gaunt",
    "puny",
    "bony",
    "raw-boned",
    "malnourished",
    "lean",
    "scab-skinned",
    "foul",
    "greasy",
    "filthy",
    "dirt-covered",
    "disease-ridden",
    "vile",
    "nimble",
    "large",
    "slick",
    "sleek",
    "agile",
    "small",
    "maimed",
    "crippled",
    "\n"
};

const char *bird_adj1[] =
{
    "tiny",
    "miniscule",
    "small",
    "minute",
    "large",
    "sleek",
    "elegant",
    "plump",
    "majestic",
    "beautiful",
    "predatory",
    "gorgeous",
    "broad-winged",
    "wide-winged",
    "large-winged",
    "brightly-plumed",
    "wildly-plumed",
    "fiery-plumed",
    "dull-plumed",
    "drab-plumed",
    "darkly-plumed",
    "ebon-plumed",
    "reddish-plumed",
    "glossy-feathered",
    "fluffy-feathered",
    "bright-red-feathered",
    "drab-feathered",
    "dun-feathered",
    "white-feathered",
    "pink-feathered",
    "greyish-brown-feathered",
    "brownish-grey-feathered",
    "dull-feathered",
    "black-feathered",
    "blue-black-feathered",
    "midnight-feathered",
    "brown-feathered",
    "tan-feathered",
    "beryl-feathered",
    "blue-feathered",
    "dull-red-feathered",
    "dull-blue-feathered",
    "grey-feathered",
    "dull-grey-feathered",
    "striped",
    "spotted",
    "dappled",
    "\n"
};

const char *bird_adj2[] =
{
    "sharp-beaked",
    "pointy-beaked",
    "wide-beaked",
    "broad-beaked",
    "small-beaked",
    "narrow-beaked",
    "large-beaked",
    "huge-beaked",
    "spear-beaked",
    "hook-beaked",
    "wide-billed",
    "broad-billed",
    "small-billed",
    "large-billed",
    "hook-billed",
    "sharp-eyed",
    "black-eyed",
    "wide-eyed",
    "sharp-taloned",
    "fiercely-taloned",
    "large-taloned",
    "long-tailed",
    "short-tailed",
    "wide-tailed",
    "thin-tailed",
    "fan-tailed",
    "large-tailed",
    "long-necked",
    "short-necked",
    "long-legged",
    "short-legged",
    "crowned",
    "\n"
};

const char *wolf_adj1[] =
{
    "sinewy",
    "muscular",
    "powerfully-muscled",
    "thickly-muscled",
    "gaunt",
    "bony",
    "skeletal",
    "rawboned",
    "emaciated",
    "lean",
    "lanky",
    "tawny",
    "grizzled",
    "scraggly",
    "fierce",
    "mangy",
    "thickly-furred",
    "mottle-coated",
    "sleek",
    "brambly-furred",
    "wiry-furred",
    "shaggy-furred",
    "long-whiskered",
    "silken-furred",
    "bristly-furred",
    "spiky-furred",
    "gray-furred",
    "smoky-grey-furred",
    "ash-grey-furred",
    //"black-furred",
    //"ebony-furred",
    "sooty-grey-furred",
    "charcoal-grey-furred",
    //"coal-black-furred",
    //"inky-furred",
    //"ebon-furred",
    //"brown-furred",
    //"muddy-brown-furred",
    //"dark-brown-furred",
    //"jet-black",
    "light-grey",
    "dark-grey",
    //"light-brown",
    //"dark-brown",
    //"muddy-brown",
    "matted-furred",
    "\n"
};

const char *wolf_adj2[] =
{
    "narrow-muzzled",
    "sharp-muzzled",
    "short-muzzled",
    "long-muzzled",
    "crook-tailed",
    "hook-tailed",
    "long-necked",
    "sharp-eared",
    "wide-eared",
    "one-eared",
    "narrow-eyed",
    "glassy-eyed",
    //"black-eyed",
    //"grey-eyed",
    //"blue-eyed",
    //"white-eyed",
    "glint-eyed",
    "razor-fanged",
    "yellow-fanged",
    "frothy-mawed",
    "sharp-toothed",
    "long-legged",
    "large-pawed",
    "golden-eyed",
    //"black-eyed",
    //"brown-eyed",
    "keen-eyed",
    "large-pawed",
    "white-pawed",
    "feral",
    "\n"
};

const char *human_adj1[] =
{
    "acned",
    "cadaverous",
    "dirty",
    "dust-covered",
    "doughy",
    "fair",
    "greasy",
    "jaundiced",
    "pale",
    "livid",
    "pallid",
    "hearty",
    "scarred",
    "sun-browned",
    "swarthy",
    "wan",
    "waxy",
    "weatherbeaten",
    "almond-eyed",
    "beady-eyed",
    "cock-eyed",
    "owlish",
    "rheumy-eyed",
    "squinty-eyed",
    "aquiline-nosed",
    "beak-nosed",
    "bent-nosed",
    "knob-nosed",
    "flat-nosed",
    "hawk-nosed",
    "pig-nosed",
    "pug-nosed",
    "athletic",
    "brawny",
    "bent",
    "bow-spined",
    "burly",
    "chubby",
    "colossal",
    "brawny",
    "delicate",
    "diminutive",
    "lithe",
    "large",
    "thin",
    "fat",
    "fleshy",
    "fragile",
    "gangly",
    "gaunt",
    "haggard",
    "hunched",
    "husky",
    "lanky",
    "lean",
    "lithe",
    "lissome",
    "lissome",
    "lithe",
    "muscled",
    "obese",
    "lanky",
    "paunchy",
    "slender",
    "petite",
    "portly",
    "pot-bellied",
    "pudgy",
    "reedy",
    "rickety",
    "willowy",
    "robust",
    "rotund",
    "rugged",
    "scrawny",
    "runty",
    "sinewy",
    "runty",
    "skeletal",
    "sleek",
    "slight",
    "slender",
    "slim",
    "spindly",
    "squat",
    "stalwart",
    "statuesque",
    "svelte",
    "tall",
    "thickset",
    "thin",
    "waspish",
    "well-muscled",
    "whip-thin",
    "willowy",
    "wiry",
    "blue-eyed",
    "azure-eyed",
    "green-eyed",
    "emerald-eyed",
    "jade-eyed",
    "brown-eyed",
    "chocolate-eyed",
    "dark-eyed",
    "grey-eyed",
    "stormy-eyed",
    "hazel-eyed",
    "\n"
};

const char *human_adj2[] =
{
    "black-haired",
    "coal-haired",
    "ebony-haired",
    "jet-haired",
    "midnight-haired",
    "onyx-haired",
    "raven-haired",
    "auburn-haired",
    "copper-haired",
    "red-haired",
    "scarlet-haired",
    "sepia-haired",
    "blonde-haired",
    "golden-haired",
    "ginger-haired",
    "honey-haired",
    "flaxen-haired",
    "sandy-haired",
    "sorrel-haired",
    "tawny-haired",
    "bronze-haired",
    "brown-haired",
    "chestnut-haired",
    "dun-haired",
    "russet-haired",
    "sable-haired",
    "taupe-haired",
    "wheat-haired",
    "henna-haired",
    "dusky-haired",
    "ecru-haired",
    "angular-faced",
    "aristocratic",
    "comely-faced",
    "careworn",
    "cherubic",
    "comely-faced",
    "drawn-faced",
    "feline-faced",
    "narrow-faced",
    "square-faced",
    "stoop-shouldered",
    "broad-shouldered",
    "drooping-shouldered",
    "delicate-shouldered",
    "\n"
};

const char *orc_adj1[] =
{
    "acned",
    "froglike",
    "stumpy",
    "scabbed",
    "balding",
    "bloated",
    "grumpy",
    "meaty",
    "chunky",
    "cadaverous",
    "dirty",
    "filthy",
    "dust-covered",
    "greasy",
    "wan-looking",
    "weatherbeaten",
    "beady-eyed",
    "cock-eyed",
    "rheumy-eyed",
    "squinty-eyed",
    "frog-like",
    "stumpy",
    "scabbed",
    "balding",
    "beak-nosed",
    "pointy-nosed",
    "flat-nosed",
    "pig-nosed",
    "pug-nosed",
    "brawny",
    "bent",
    "bow-spined",
    "burly",
    "chubby",
    "brawny",
    "large",
    "thin",
    "fat",
    "fleshy",
    "gangly",
    "haggard",
    "hunched",
    "lanky",
    "muscled",
    "obese",
    "grossly obese",
    "paunchy",
    "pot-bellied",
    "portly",
    "pudgy",
    "reedy",
    "rickety-looking",
    "scrawny",
    "runty",
    "sinewy",
    "skeletal",
    "sleek",
    "spindly",
    "squat",
    "stalwart",
    "thickset",
    "waspish",
    "cat-eyed",
    "feline-eyed",
    "reptilian-eyed",
    "green-eyed",
    "bright-green-eyed",
    "yellow-eyed",
    "bright-yellow-eyed",
    "milky-eyed",
    "black-eyed",
    "dark-eyed",
    "red-eyed",
    "bright-red-eyed",
    "crimson-eyed",
    "yellow-green-eyed",
    "green-skinned",
    "dark-green-skinned",
    "forest-green-skinned",
    "oily-skinned",
    "gruesomely-scarred",
    "gangly-limbed",
    "black-skinned",
    "dark-skinned",
    "brown-skinned",
    "dark-brown-skinned",
    "leathery-skinned",
    "wrinkled",
    "flabby",
    "potbellied",
    "flat-headed",
    "hollow-eyed",
    "\n"
};

const char *orc_adj2[] =
{
    "black-haired",
    "coal-haired",
    "ebony-haired",
    "pumice-haired",
    "jet-haired",
    "onyx-haired",
    "dun-haired",
    "midnight-haired",
    "dark-brown-haired",
    "sorrel-haired",
    "brown-haired",
    "mud-haired",
    "dusky-haired",
    "crimson-haired",
    "dark-red-haired",
    "white-haired",
    "scar-faced",
    "snaggletoothed",
    "leer-faced",
    "sharp-toothed",
    "wrinkle-faced",
    "scraggly-haired",
    "cracked-lipped",
    "large-eared",
    "lopsided-looking",
    "stooped",
    "broad-shouldered",
    "bent-shouldered",
    "narrow-faced",
    "square-faced",
    "flat-faced",
    "nefarious-looking",
    "black-lipped",
    "wickedly scarred",
    "wart-covered",
    "saw-toothed",
    "small-toothed",
    "hunchbacked",
    "triangular-faced",
    "sneer-faced",
    "gap-toothed",
    "brown-toothed",
    "yellow-toothed",
    "wickedly-fanged",
    "small-tusked",
    "\n"
};

const char *zombie_adj1[] =
{
    "blotchy-skinned",
    "spine-covered",
    "ooze-crusted",
    "squat",
    "greasy-bodied",
    "bloated",
    "scrawny",
    "emaciated",
    "gaunt",
    "puny",
    "bony",
    "raw-boned",
    "scab-skinned",
    "foul",
    "filthy",
    "vile",
    "lumpy",
    "fleshy",
    "pot-bellied",
    "bloated",
    "obese",
    "stumpy",
    "putrid-yellow-eyed",
    "red-eyed",
    "blood-red-eyed",
    "pink-eyed",
    "eyeless",
    "wrinkled",
    "flabby",
    "grotesquely-scarred",
    "gruesomely-scarred",
    "scar-covered",
    "boil-covered",
    "warty",
    "scale-covered",
    "scaly-skinned",
    "leathery-skinned",
    "jaundiced",
    "leprous",
    "scabby-skinned",
    "pimply-skinned",
    "ulcerous-skinned",
    "\n"
};

const char *zombie_adj2[] =
{
    "sharp-toothed",
    "toothless",
    "jagged-toothed",
    "rotting-toothed",
    "taloned",
    "clawed",
    "sharp-nailed",
    "razor-toothed",
    "greasy-haired",
    "lank-haired",
    "bone-faced",
    "lump-faced",
    "hairless",
    "wild-bearded",
    "stoop-shouldered",
    "patchy-haired",
    "balding",
    "scab-haired",
    "reeking",
    "sweat-stained",
    "foul-smelling",
    "decay-stinking",
    "blood-covered",
    "gore-covered",
    "vomit-covered",
    "tangle-bearded",
    "\n"
};

void
insert_mobile_variables (CHAR_DATA * mob, CHAR_DATA * proto, char *string0, char *string1, char *string2, char *string3, char *string4, char *string5, char *string6, char *string7, char *string8, char *string9)
{
    char buf2[MAX_STRING_LENGTH];
    char temp[MAX_STRING_LENGTH];
    char original[MAX_STRING_LENGTH];

    char *xcolor[10];
    char *xcat[10];
    int xorder[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

    for (int ind = 0; ind <= 9; ind++)
    {
        xcolor[ind] = '\0';
        xcat[ind] = '\0';
    }

    char tempcolor[AVG_STRING_LENGTH] = { '\0' };
    char *point;
    int i = 0, j = 0, h = 0;
    bool modified = false;

    char buf[AVG_STRING_LENGTH] = { '\0' };
    *buf2 = '\0';
    *temp = '\0';
    int temp_round = 0;
    bool bumped = false;

    if (string0 == 0)
        xcolor[0] = '\0';
    else
        xcolor[0] = str_dup (string0);

    if (string1 == 0)
        xcolor[1] = '\0';
    else
        xcolor[1] = str_dup (string1);

    if (string2 == 0)
        xcolor[2] = '\0';
    else
        xcolor[2] = str_dup (string2);

    if (string3 == 0)
        xcolor[3] = '\0';
    else
        xcolor[3] = str_dup (string3);

    if (string4 == 0)
        xcolor[4] = '\0';
    else
        xcolor[4] = str_dup (string4);

    if (string5 == 0)
        xcolor[5] = '\0';
    else
        xcolor[5] = str_dup (string5);

    if (string6 == 0)
        xcolor[6] = '\0';
    else
        xcolor[6] = str_dup (string6);

    if (string7 == 0)
        xcolor[7] = '\0';
    else
        xcolor[7] = str_dup (string7);

    if (string8 == 0)
        xcolor[8] = '\0';
    else
        xcolor[8] = str_dup (string8);

    if (string9 == 0)
        xcolor[9] = '\0';
    else
        xcolor[9] = str_dup (string9);

    // If we have got colours, we need to take a guess as to which category they belong to.
    for (int ind = 0; ind <= 9; ind++)
    {
        if (xcolor[ind])
            xcat[ind] = add_hash(mob_vc_category(mob_vd_variable(xcolor[ind])));
    }

    // If we don't have a full desc, quit out.
    if (!*proto->description)
        return;

    // Save our original description of the full description.
    sprintf (original, "%s", proto->description);

    // Find at what point we have our first "$".
    point = strpbrk (original, "$");
    int round = 0;

    // If we found point...
    if (point)
    {
        // Then for every character in the string...
        // We run through the original, adding each bit of y to buf2.
        // However, if we find a $, we see if that's a category of variables.
        // If so, we add a random colour of those variables to buf2, and then skip ahead y to the end of that phrase, where we keep going on our merry way.

        for (size_t y = 0; y <= strlen (original); y++)
        {
            // If we're at the $...
            if (original[y] == *point)
            {
                // Then we're going to modify it...
                modified = true;
                // ... so let's keep a temporary marker ...
                sprintf (temp, "$");
                // ... and jump ahead a point (to get to the letter after the $
                j = y + 1;

                // Now, until we hit something that's not a alpha-numeric character.
                while (isalpha (original[j]))
                {
                    // add the word after the $ to our temporary marker.
                    sprintf (temp + strlen (temp), "%c", original[j]);
                    j++;
                }

                // If there's a number after our category, then we're going to round it all up - let's add it to our xorder list.

                *buf = '\0';

                sprintf(buf, "%c", original[j]);

                if (isdigit(*buf))
                    xorder[round] = atoi(buf);
                else
                    xorder[round] = -1;

                // Now, we figure out which colour we'e setting by seeing if we don't have the color of the present round...
                if (!xcolor[round])
                {

                    if (!mob_vc_category(temp))
                        return;

                    // Now that we know temp is from a proper category, we pull a random variable from that category and call it tempcolor.
                    sprintf (tempcolor, "%s", mob_vc_rand(temp, &i));

                    // Now, we check what round we are and assign tempcolor to that round, and do the same for the categories.

                    xcolor[round] = add_hash (tempcolor);
                    xcat[round] = add_hash (mob_vc_category(i));
                }

                // Now, depending on the round, we add on to buf2 the color we just pulled, and then advance to the next round.
                if (xcat[round])
                    sprintf (buf2 + strlen (buf2), "%s", mob_vd_full(xcat[round], xcolor[round]));
                else
                    sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

                // Now, we set our end point as where our category ends plus 1.
                j = y + 1;

                // We advance until we get to the new non-alpha-numeric character.
                while (isalpha (original[j]))
                    j++;

                if (xorder[round] >= 0)
                    j++;

                if (round < 9)
                    round ++;

                // And then set the point of our main loop to that point
                y = j;
            }
            sprintf (buf2 + strlen (buf2), "%c", original[y]);

            // If we're at round ten, time to quit.

        }
        mem_free (mob->description);
        mob->description = add_hash (buf2);
        reformat_desc(mob->description, &mob->description);
    }

    for (int ind = 0; ind < 10; ind++)
    {
        if (xcat[ind] && *xcat[ind] && !str_cmp(xcat[ind], "$lizcolor"))
        {
            mob->d_feat3 = add_hash("$lizcolor");
            mob->d_feat4 = add_hash(xcolor[ind]);
            break;
        }
    }

    if (modified)
    {
        *buf2 = '\0';
        sprintf (original, "%s", proto->short_descr);
        point = strpbrk (original, "$");
        round = 0;

        if (point)
        {
            for (size_t y = 0; y <= strlen (original); y++)
            {
                temp_round = round;
                bumped = false;

                if (original[y] == *point)
                {
                    modified = true;
                    sprintf (temp, "$");
                    j = y + 1;

                    h = j;

                    while (isalpha (original[h]))
                        h++;

                    *buf = '\0';

                    sprintf(buf, "%c", original[h]);

                    if (isdigit(*buf))
                    {
                        round = atoi(buf);
                        bumped = true;
                    }

                    if (xcolor[round])
                        sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

                    round = temp_round;

                    j = y + 1;

                    while (isalpha (original[j]))
                        j++;

                    if (bumped)
                        j++;

                    y = j;
                    round++;
                }
                sprintf (buf2 + strlen (buf2), "%c", original[y]);
            }
            mem_free (mob->short_descr);
            mob->short_descr = add_hash (buf2);
        }

        /*
         *buf2 = '\0';
         sprintf (original, "%s", proto->long_descr);
         point = strpbrk (original, "$");
         round = 0;

         if (point)
         {
           for (size_t y = 0; y <= strlen (original); y++)
           {
        	 temp_round = round;
        	 bumped = false;

        	 if (original[y] == *point)
        	 {
        	   modified = true;
        	   sprintf (temp, "$");
        	   j = y + 1;

        	   h = j;

        	   while (isalpha (original[h]))
        		 h++;

        	   *buf = '\0';

        	   sprintf(buf, "%c", original[h]);

        	   if (isdigit(*buf))
        	   {
        		 round = atoi(buf);
        		 bumped = true;
        	   }

        	   sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

        	   round = temp_round;

        	   j = y + 1;

        	   while (isalpha (original[j]))
        		 j++;

        	   if (bumped)
        		 j++;

        	   y = j;
        	   round++;
        	 }
        	 sprintf (buf2 + strlen (buf2), "%c", original[y]);
           }
           mem_free (mob->long_descr);
           mob->long_descr = add_hash (buf2);
         }
         */

        *buf2 = '\0';
        sprintf (original, "%s", proto->name);
        point = strpbrk (original, "$");
        round = 0;

        if (point)
        {
            for (size_t y = 0; y <= strlen (original); y++)
            {
                temp_round = round;
                bumped = false;

                if (original[y] == *point)
                {
                    modified = true;
                    sprintf (temp, "$");
                    j = y + 1;

                    h = j;

                    while (isalpha (original[h]))
                        h++;

                    *buf = '\0';

                    sprintf(buf, "%c", original[h]);

                    if (isdigit(*buf))
                    {
                        round = atoi(buf);
                        bumped = true;
                    }

                    if (xcolor[round])
                        sprintf (buf2 + strlen (buf2), "%s", xcolor[round]);

                    round = temp_round;

                    j = y + 1;

                    while (isalpha (original[j]))
                        j++;

                    if (bumped)
                        j++;

                    y = j;
                    round++;
                }
                sprintf (buf2 + strlen (buf2), "%c", original[y]);
            }
            mem_free (mob->name);
            mob->name = add_hash (buf2);
        }
    }
}

char *
return_adj2 (CHAR_DATA * mob)
{
    int roll, limit;
    static char adj[MAX_STRING_LENGTH];

    if (!str_cmp(lookup_race_variable (mob->race, RACE_NAME), "Human") || !str_cmp(lookup_race_variable (mob->race, RACE_NAME), "Survivor"))
    {
        for (limit = 0; *human_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", human_adj2[roll]);
        return adj;
    }
    else if (!str_cmp
             (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        for (limit = 0; *zombie_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", zombie_adj2[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Orc"))
    {
        for (limit = 0; *orc_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", orc_adj2[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Wolf"))
    {
        for (limit = 0; *wolf_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", wolf_adj2[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Bird"))
    {
        for (limit = 0; *bird_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", bird_adj2[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Rat"))
    {
        for (limit = 0; *rat_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", rat_adj2[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Spider"))
    {
        for (limit = 0; *spider_adj2[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", spider_adj2[roll]);
        return adj;
    }

    return "fur-covered";
}

char *
return_adj1 (CHAR_DATA * mob)
{
    int roll, limit;
    static char adj[MAX_STRING_LENGTH];

    if (!str_cmp(lookup_race_variable (mob->race, RACE_NAME), "Human") || !str_cmp(lookup_race_variable (mob->race, RACE_NAME), "Survivor"))
    {
        for (limit = 0; *human_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", human_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        for (limit = 0; *zombie_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", zombie_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Orc"))
    {
        for (limit = 0; *orc_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", orc_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Wolf"))
    {
        for (limit = 0; *wolf_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", wolf_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Bird"))
    {
        for (limit = 0; *bird_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", bird_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Rat"))
    {
        for (limit = 0; *rat_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", rat_adj1[roll]);
        return adj;
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Spider"))
    {
        for (limit = 0; *spider_adj1[limit] != '\n'; limit++)
            ;
        limit--;
        roll = number (0, limit);
        sprintf (adj, "%s", spider_adj1[roll]);
        return adj;
    }

    return "short";
}

char *
return_name (CHAR_DATA * mob)
{
    static char buf[MAX_STRING_LENGTH];
    int roll;

    *buf = '\0';

    if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Orc"))
    {
        /*if (GET_SEX (mob) == SEX_MALE)
         return "male orc";
         else if (GET_SEX (mob) == SEX_FEMALE)
         return "female orc";
         else*/
        return "orc";
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Bird"))
        return "bird";
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Spider"))
        return "spider";
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Rat"))
        return "rat";

    if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        roll = number (1, 13);
        if (roll == 1)
            sprintf(buf, "malformed ");
        else if (roll == 2)
            sprintf(buf, "deformed ");
        else if (roll == 3)
            sprintf(buf, "maimed ");
        else if (roll == 4)
            sprintf(buf, "mutated ");
        else if (roll == 5)
            sprintf(buf, "twisted ");
        else if (roll == 6)
            sprintf(buf, "wretched ");
        else if (roll == 7)
            sprintf(buf, "crippled ");
        else if (roll == 8)
            sprintf(buf, "disfigured ");
        else if (roll == 9)
            sprintf(buf, "distorted ");
        else if (roll == 10)
            sprintf(buf, "mangled ");
        else if (roll == 11)
            sprintf(buf, "misshapen ");
        else if (roll == 12)
            sprintf(buf, "warped ");
        else if (roll == 13)
            sprintf(buf, "gnarled ");
        else if (roll == 14)
            sprintf(buf, "mutilated ");
        else
            sprintf(buf, "malformed ");
    }

    roll = number (0, 5);

    if (str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        if (roll == 0 && !IS_SET (mob->act, ACT_ENFORCER))
            sprintf (buf, "old ");
        else if (roll == 4 && !IS_SET (mob->act, ACT_ENFORCER))
            sprintf (buf, "young ");
    }

    if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Wolf"))
    {
        strcat (buf, "wolf");
        return buf;
    }

    /*
     if (IS_SET (mob->act, ACT_ENFORCER))
     {
       if (GET_SEX (mob) == SEX_MALE)
         sprintf (buf, "man");
       else
         sprintf (buf, "woman");
       return buf;
     }

     if (roll == 0 && mob->sex == SEX_MALE)
     {
       roll2 = number (1, 3);
       if (roll2 == 1)
         strcat (buf, "gaffer");
       else if (roll2 == 2)
         strcat (buf, "geezer");
       else if (roll2 == 3)
         strcat (buf, "man");
       else
         strcat (buf, "man");
     }
     else if (roll == 0 && mob->sex == SEX_FEMALE)
     {
       roll2 = number (1, 7);
       if (roll2 == 1)
         strcat (buf, "crone");
       else if (roll2 == 2)
         strcat (buf, "harridan");
       else if (roll2 == 3)
         strcat (buf, "matron");
       else if (roll2 == 4)
         strcat (buf, "spinster");
       else
         strcat (buf, "woman");
     }
     else if (roll == 4 && mob->sex == SEX_MALE)
     {
       roll2 = number (1, 4);
       if (roll2 == 1)
         strcat (buf, "lad");
       else if (roll2 == 2)
         strcat (buf, "waif");
       else
         strcat (buf, "man");
     }
     else if (roll == 4 && mob->sex == SEX_FEMALE)
     {
       roll2 = number (1, 4);
       if (roll2 == 1)
         strcat (buf, "lass");
       else if (roll2 == 2)
         strcat (buf, "maid");
       else
         strcat (buf, "woman");
     }
     else */

    if (mob->sex == SEX_MALE)
        strcat (buf, "man");
    else if (mob->sex == SEX_FEMALE)
        strcat (buf, "woman");
    else
        sprintf (buf + strlen (buf), "person %d", mob->race);

    return buf;
}

/*                                                                          *
 * function: create_description                                             *
 *                                                                          *
 * 09/28/2004 [JWW] - Added travel strings some arbitrary mobs              *
 *                                                                          */
void
old_create_description (CHAR_DATA * mob)
{
    char sdesc_frame[MAX_STRING_LENGTH];
    char sdesc[MAX_STRING_LENGTH];
    char adj1[MAX_STRING_LENGTH];
    char adj2[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    bool found = false;
    int roll, i, j;

    for (i = 0; *variable_races[i] != '\n'; i++)
        if (!str_cmp
                (variable_races[i], lookup_race_variable (mob->race, RACE_NAME)))
            found = true;

    if (!found)
    {
        return;
    }
    if (!number (0, 1))
    {
        if (!number (0, 1))
            sprintf (sdesc_frame, "$adj1, $adj2 $name");
        else
            sprintf (sdesc_frame, "$adj2, $adj1 $name");
    }
    else
    {
        if (!number (0, 1))
            sprintf (sdesc_frame, "$adj1 $name");
        else
            sprintf (sdesc_frame, "$adj2 $name");
    }

    *sdesc = '\0';
    *adj1 = '\0';
    *adj2 = '\0';
    *name = '\0';
    *buf2 = '\0';

    sprintf (name, "%s", return_name (mob));

    for (size_t i = 0; i <= strlen (sdesc_frame); i++)
    {
        if (sdesc_frame[i] == '$')
        {
            j = i;
            *buf = '\0';
            while (sdesc_frame[i] && sdesc_frame[i] != ' '
                    && sdesc_frame[i] != ',')
            {
                sprintf (buf + strlen (buf), "%c", sdesc_frame[i]);
                i++;
            }
            i = j;
            if (!str_cmp (buf, "$adj1"))
            {
                sprintf (adj1, "%s", return_adj1 (mob));
                if (!*sdesc
                        && (adj1[0] == 'a' || adj1[0] == 'e' || adj1[0] == 'i'
                            || adj1[0] == 'o' || adj1[0] == 'u'))
                    sprintf (sdesc + strlen (sdesc), "an ");
                else if (!*sdesc)
                    sprintf (sdesc + strlen (sdesc), "a ");
                sprintf (sdesc + strlen (sdesc), "%s", adj1);
                sprintf (buf2 + strlen (buf2), "%s ", adj1);
            }
            else if (!str_cmp (buf, "$adj2"))
            {
                sprintf (adj2, "%s", return_adj2 (mob));
                if (!*sdesc
                        && (adj2[0] == 'a' || adj2[0] == 'e' || adj2[0] == 'i'
                            || adj2[0] == 'o' || adj2[0] == 'u'))
                    sprintf (sdesc + strlen (sdesc), "an ");
                else if (!*sdesc)
                    sprintf (sdesc + strlen (sdesc), "a ");
                while (!str_cmp (adj1, adj2))
                    sprintf (adj2, "%s", return_adj2 (mob));
                sprintf (sdesc + strlen (sdesc), "%s", adj2);
                sprintf (buf2 + strlen (buf2), "%s ", adj2);
            }
            else if (!str_cmp (buf, "$name"))
            {
                sprintf (sdesc + strlen (sdesc), "%s", name);
                sprintf (buf2 + strlen (buf2), "%s", name);
            }
            i += strlen (buf) - 1;
            continue;
        }
        else
            sprintf (sdesc + strlen (sdesc), "%c", sdesc_frame[i]);
    }

    mob->delay_info1 = 0;

    if (mob->short_descr)
        mem_free (mob->short_descr);
    mob->short_descr = add_hash (sdesc);
    *buf = '\0';
    if (IS_SET (mob->act, ACT_ENFORCER) && IS_SET (mob->act, ACT_SENTINEL) && is_human(mob))
    {
        roll = number (1, 3);
        if (roll == 1)
            sprintf (buf, "%s stands at attention here.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s stands here, watching for signs of trouble.",
                     sdesc);
        else if (roll == 3)
            sprintf (buf, "%s patrols here, looking hawkishly about.", sdesc);
        mob->travel_str = add_hash ("looking hawkishly about");
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Wolf"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s prowls through the area.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s paces here.", sdesc);
        else if (roll == 3)
            sprintf (buf, "%s paces back and forth.", sdesc);
        else if (roll == 4)
            sprintf (buf, "%s pads about here.", sdesc);
        else if (roll == 5)
        {
            sprintf (buf, "%s pauses here, sniffing the air.", sdesc);
            mob->travel_str = add_hash ("sniffing the ground");
        }
        else if (roll == 6)
        {
            sprintf (buf, "%s pads around, sniffing the ground.", sdesc);
            mob->travel_str = add_hash ("sniffing the ground");
        }
        else if (roll == 7)
            sprintf (buf, "%s watches its surroundings alertly.", sdesc);
    }
    else if (IS_SET (mob->act, ACT_ENFORCER) && !IS_SET (mob->act, ACT_SENTINEL) && is_human(mob))
    {
        roll = number (1, 3);
        if (roll == 1)
        {
            sprintf (buf, "%s patrols here, looking for signs of trouble.",
                     sdesc);
            mob->travel_str = add_hash ("looking about purposefully");
        }
        else if (roll == 2)
        {
            sprintf (buf, "%s moves by, watching the area attentively.", sdesc);
            mob->travel_str = add_hash ("looking about the area attentively");
        }
        else if (roll == 3)
        {
            sprintf (buf, "%s strides through, watching intently.", sdesc);
            mob->travel_str = add_hash ("looking hawkishly about");
        }
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Bird"))
    {
        roll = number (1, 4);
        if (roll == 1)
            sprintf (buf, "%s perches here.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s perches here, observing quietly.", sdesc);
        else if (roll == 3)
            sprintf (buf, "%s flies through the area.", sdesc);
        else if (roll == 4)
            sprintf (buf, "%s is here, watching in silence.", sdesc);
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Rat"))
    {
        roll = number (1, 7);
        if (roll == 1)
        {
            sprintf (buf, "%s skitters around the area.", sdesc);
            mob->travel_str = add_hash ("skittering about nervously");
        }
        else if (roll == 2)
            sprintf (buf, "%s is here, skulking about.", sdesc);
        else if (roll == 3)
            sprintf (buf, "%s moves quietly by.", sdesc);
        else if (roll == 4)
            sprintf (buf, "%s waits here, unmoving.", sdesc);
        else if (roll == 5)
            sprintf (buf, "%s is here.", sdesc);
        else if (roll == 6)
            sprintf (buf, "%s lies low to the ground here.", sdesc);
        else if (roll == 7)
        {
            sprintf (buf, "%s sneaks about quietly.", sdesc);
            mob->travel_str = add_hash ("padding along quietly in the shadows");
        }
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Spider"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s crawls about silently.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s moves slowly.", sdesc);
        else if (roll == 3)
            sprintf (buf, "%s crouches low to the ground.", sdesc);
        else if (roll == 4)
            sprintf (buf, "%s stands here, clacking its mandibles.", sdesc);
        else if (roll == 5)
            sprintf (buf, "%s is here.", sdesc);
        else if (roll == 6)
            sprintf (buf, "%s moves across the rough ground.", sdesc);
        else if (roll == 7)
            sprintf (buf, "%s sits here, unmoving.", sdesc);
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s stands here hunched over.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s stands here, stock still.", sdesc);
        else if (roll == 3)
            sprintf (buf, "%s is here, jerking occasionally.", sdesc);
        else if (roll == 4)
            sprintf (buf, "%s stands here, swaying from side to side.", sdesc);
        else if (roll == 5)
            sprintf (buf, "%s is here, twitching spasmodically.", sdesc);
        else if (roll == 6)
            sprintf (buf, "%s stands here.", sdesc);
        else if (roll == 7)
            sprintf (buf, "%s is here.", sdesc);

        switch (number (1, 4))
        {
        case 1:
            mob->speed = 2;
            mob->travel_str = add_hash ("shambling forward");
            break;
        case 2:
            mob->speed = 1;
            mob->travel_str = add_hash ("in trudging, uneven steps");
            break;
        case 3:
            mob->speed = 4;
            mob->travel_str = add_hash ("with a loping, swaying run");
            break;
        case 4:
            mob->speed = 0;
            mob->travel_str = add_hash ("stumbling onwards");
            break;
        }

    }

    if (!*buf)
        sprintf (buf, "%s is here.", sdesc);

    *buf = toupper (*buf);

    if (mob->long_descr)
        mem_free (mob->long_descr);
    mob->long_descr = add_hash (buf);

    if (mob->name)
        mem_free (mob->name);

    if (mob->tname)
        mem_free (mob->tname);


    sprintf(buf3, "x%d%d%d%d%d-%s", number(1,9), number(0,9), number(0,9),number(0,9),number(0,9),mob->tname);

    mob->tname = add_hash(buf3);

    sprintf(buf3 + strlen(buf3), " %s", buf2);

    mob->name = add_hash (buf3);


}

/*                                                                          *
 * function: create_description                                             *
 *                                                                          *
 * 09/28/2004 [JWW] - Added travel strings some arbitrary mobs              *
 *                                                                          */
void
create_description (CHAR_DATA * mob)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int roll;

    /*
     for (i = 0; *variable_races[i] != '\n'; i++)
     if (!str_cmp
         (variable_races[i], lookup_race_variable (mob->race, RACE_NAME)))
     found = true;

     if (!found)
     {
       return;
     }
     */

    if (IS_NPC(mob))
        insert_mobile_variables (mob, vnum_to_mob(mob->mob->vnum), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    else
    {
        old_create_description(mob);
    }

    *buf = '\0';
    if (IS_SET (mob->act, ACT_ENFORCER) && IS_SET (mob->act, ACT_SENTINEL))
    {
        roll = number (1, 3);
        if (roll == 1)
            sprintf (buf, "%s stands at attention here.", mob->short_descr);
        else if (roll == 2)
            sprintf (buf, "%s stands here, watching for signs of trouble.",
                     mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s patrols here, looking hawkishly about.", mob->short_descr);
        mob->travel_str = add_hash ("looking hawkishly about");
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Wolf"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s prowls through the area.", mob->short_descr);
        else if (roll == 2)
            sprintf (buf, "%s paces here.", mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s paces back and forth.", mob->short_descr);
        else if (roll == 4)
            sprintf (buf, "%s pads about here.", mob->short_descr);
        else if (roll == 5)
        {
            sprintf (buf, "%s pauses here, sniffing the air.", mob->short_descr);
            mob->travel_str = add_hash ("sniffing the ground");
        }
        else if (roll == 6)
        {
            sprintf (buf, "%s pads around, sniffing the ground.", mob->short_descr);
            mob->travel_str = add_hash ("sniffing the ground");
        }
        else if (roll == 7)
            sprintf (buf, "%s watches its surroundings alertly.", mob->short_descr);
    }
    else if (IS_SET (mob->act, ACT_ENFORCER)
             && !IS_SET (mob->act, ACT_SENTINEL))
    {
        roll = number (1, 3);
        if (roll == 1)
        {
            sprintf (buf, "%s patrols here, looking for signs of trouble.",
                     mob->short_descr);
            mob->travel_str = add_hash ("looking about purposefully");
        }
        else if (roll == 2)
        {
            sprintf (buf, "%s moves by, watching the area attentively.", mob->short_descr);
            mob->travel_str = add_hash ("looking about the area attentively");
        }
        else if (roll == 3)
        {
            sprintf (buf, "%s strides through, watching intently.", mob->short_descr);
            mob->travel_str = add_hash ("looking hawkishly about");
        }
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Bird"))
    {
        roll = number (1, 4);
        if (roll == 1)
            sprintf (buf, "%s perches here.", mob->short_descr);
        else if (roll == 2)
            sprintf (buf, "%s perches here, observing quietly.", mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s flies through the area.", mob->short_descr);
        else if (roll == 4)
            sprintf (buf, "%s is here, watching in silence.", mob->short_descr);
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Rat"))
    {
        roll = number (1, 7);
        if (roll == 1)
        {
            sprintf (buf, "%s skitters around the area.", mob->short_descr);
            mob->travel_str = add_hash ("skittering about nervously");
        }
        else if (roll == 2)
            sprintf (buf, "%s is here, skulking about.", mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s moves quietly by.", mob->short_descr);
        else if (roll == 4)
            sprintf (buf, "%s waits here, unmoving.", mob->short_descr);
        else if (roll == 5)
            sprintf (buf, "%s is here.", mob->short_descr);
        else if (roll == 6)
            sprintf (buf, "%s lies low to the ground here.", mob->short_descr);
        else if (roll == 7)
        {
            sprintf (buf, "%s sneaks about quietly.", mob->short_descr);
            mob->travel_str = add_hash ("padding along quietly in the shadows");
        }
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Spider"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s crawls about silently.", mob->short_descr);
        else if (roll == 2)
            sprintf (buf, "%s moves slowly.", mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s crouches low to the ground.", mob->short_descr);
        else if (roll == 4)
            sprintf (buf, "%s stands here, clacking its mandibles.", mob->short_descr);
        else if (roll == 5)
            sprintf (buf, "%s is here.", mob->short_descr);
        else if (roll == 6)
            sprintf (buf, "%s moves across the rough ground.", mob->short_descr);
        else if (roll == 7)
            sprintf (buf, "%s sits here, unmoving.", mob->short_descr);
    }
    else if (!str_cmp (lookup_race_variable (mob->race, RACE_NAME), "Hosted-Terror"))
    {
        roll = number (1, 7);
        if (roll == 1)
            sprintf (buf, "%s stands here hunched over.", mob->short_descr);
        else if (roll == 2)
            sprintf (buf, "%s stands here, stock still.", mob->short_descr);
        else if (roll == 3)
            sprintf (buf, "%s is here, jerking occasionally.", mob->short_descr);
        else if (roll == 4)
            sprintf (buf, "%s stands here, swaying from side to side.", mob->short_descr);
        else if (roll == 5)
            sprintf (buf, "%s is here, twitching spasmodically.", mob->short_descr);
        else if (roll == 6)
            sprintf (buf, "%s stands here.", mob->short_descr);
        else if (roll == 7)
            sprintf (buf, "%s is here.", mob->short_descr);

        switch (number (1, 4))
        {
        case 1:
            mob->speed = 2;
            mob->travel_str = add_hash ("shambling forward");
            break;
        case 2:
            mob->speed = 1;
            mob->travel_str = add_hash ("in trudging, uneven steps");
            break;
        case 3:
            mob->speed = 4;
            mob->travel_str = add_hash ("with a loping, swaying run");
            break;
        case 4:
            mob->speed = 0;
            mob->travel_str = add_hash ("stumbling onwards");
            break;
        }

    }

    if (!*buf)
        sprintf (buf, "%s is here.", mob->short_descr);

    *buf = toupper (*buf);

    if (mob->long_descr)
        mem_free (mob->long_descr);
    mob->long_descr = add_hash (buf);

    sprintf(buf3, "x%d%d%d%d%d-%s", number(1,9), number(0,9), number(0,9),number(0,9),number(0,9), mob->tname);

    if (mob->tname)
        mem_free (mob->tname);

    mob->tname = add_hash(buf3);

    sprintf(buf2, "%s %s", mob->tname, mob->name);

    if (mob->name)
        mem_free (mob->name);

    mob->name = add_hash (buf2);


}

/**
 *  type 0 is normal racial defaults
 *  type 1 will be slightly better stats +10%
 *  type 2 will be elite with even better stats +50%
 **/
void
randomize_mobile (CHAR_DATA * mob)
{

    if (mob->race == lookup_race_id("Survivor") || mob->race == lookup_race_id("Denizen") ||
            mob->race == lookup_race_id("Mutation") || mob->race == lookup_race_id("Cybernetic") ||
            mob->race == lookup_race_id("Phoenixer"))
    {
        new_randomize_mobile(mob, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        return;
    }


    CHAR_DATA *proto;
    int attr_starters[] = { 16, 15, 12, 12, 11, 10, 8 };
    int attr_priorities[] = { -1, -1, -1, -1, -1, -1, 1 };
    int slots_taken[] = { 0, 0, 0, 0, 0, 0, 0 };
    int i, roll, bonus;
    int type_bonus = 0;

    if (is_name_in_list("elite", mob->name))
        type_bonus = 30;

    else if (is_name_in_list("regular", mob->name))
        type_bonus = 10;

    else if (is_name_in_list("veteran", mob->name))
        type_bonus = 20;

    if (lookup_race_int(mob->race, RACE_PC) &&
		str_cmp(lookup_race_variable(mob->race, RACE_NAME), "Hosted-Terror"))
    {
        for (i = 0; i <= 6; i++)
        {
            roll = number (0, 6);
            if (slots_taken[roll])
            {
                i--;
                continue;
            }

            slots_taken[roll] = 1;
            attr_priorities[i] = roll;
        }

        for (bonus = 8; bonus;)
        {
            roll = number (0, 6);
            if (attr_starters[attr_priorities[roll]] < 18)
            {
                attr_starters[attr_priorities[roll]]++;
                bonus--;
            }
        }

        mob->str = attr_starters[attr_priorities[0]];
        mob->dex = attr_starters[attr_priorities[1]];
        mob->con = attr_starters[attr_priorities[2]];
        mob->wil = attr_starters[attr_priorities[3]];
        mob->intel = attr_starters[attr_priorities[4]];
        mob->aur = attr_starters[attr_priorities[5]];
        mob->agi = attr_starters[attr_priorities[6]];

        mob->tmp_str = mob->str;
        mob->tmp_dex = mob->dex;
        mob->tmp_intel = mob->intel;
        mob->tmp_aur = mob->aur;
        mob->tmp_agi = mob->agi;
        mob->tmp_con = mob->con;
        mob->tmp_wil = mob->wil;

        for (i = 1; i <= LAST_SKILL; i++)
            mob->skills[i] = 0;

        if (IS_SET (mob->act, ACT_ENFORCER))
            mob->skills[SKILL_HANDLE] = 10 + number (5, 15) + type_bonus;

        make_height (mob);
        make_frame (mob);

        for (i = 1; i <= LAST_WEAPON_SKILL; i++) //weapon skills
            mob->skills[i] = number (20, 30) + type_bonus;

        mob->skills[SKILL_DEFLECT] = number (20, 30) + type_bonus;
        mob->skills[SKILL_DODGE] = number (20, 30) + type_bonus;

        if (is_name_in_list("sneaky", mob->name))
        {
            mob->skills[SKILL_SNEAK] = number (20, 30) + type_bonus;
            mob->skills[SKILL_HIDE] = number (20, 30) + type_bonus;
        }

        for (i = 1; i <= LAST_SKILL; i++)
        {
            if (mob->skills[i] > calc_lookup (mob, REG_CAP, i))
                mob->skills[i] = calc_lookup (mob, REG_CAP, i);
            if (mob->skills[i] < 0)
                mob->skills[i] = number (1, 10);
        }

        if (mob->mob)
            proto = vnum_to_mob (mob->mob->vnum);
        else
            proto = vnum_to_mob (998);

        if (lookup_race_int (mob->race, RACE_NATIVE_TONGUE))
        {
            mob->speaks = SKILL_COMMON;
            mob->skills[SKILL_COMMON] = 50;
            //  atoi (lookup_race_variable (mob->race, RACE_NATIVE_TONGUE));
            //  mob->skills[mob->speaks] = calc_lookup (mob, REG_CAP, mob->speaks);
        }

        for (i = 1; i <= LAST_SKILL; i++)
        {
            proto->skills[i] = mob->skills[i];
        }

        proto->speaks = mob->speaks;

        if (mob->pc)
        {
            for (i = 1; i <= LAST_SKILL; i++)
            {
                mob->pc->skills[i] = mob->skills[i];
            }
        }

        fix_offense (mob);
        fix_offense (proto);
    }  //if (mob->race >= 0 && mob->race <= 11)
    else
    {
        make_height (mob);
        make_frame (mob);
    }

    mob->sex = number (1, 2);
    if (IS_SET (mob->act, ACT_ENFORCER))
    {
        roll = number (1, 10);
        if (roll == 10)
            mob->sex = SEX_FEMALE;
        else
            mob->sex = SEX_MALE;
    }

    mob->max_move = calc_lookup (mob, REG_MISC, MISC_MAX_MOVE);
    mob->move_points = mob->max_move;

    if (IS_SET (mob->flags, FLAG_VARIABLE) || mob->pc)
    {
        create_description (mob);
        switch (number (1, 5))
        {
        case 1:
            mob->speed = SPEED_CRAWL;
            break;
        case 2:
            mob->speed = SPEED_PACED;
            break;
        default:
            mob->speed = SPEED_WALK;
            break;
        }
    }

    switch (number(1,3))
    {
    case 1:
        mob->fight_mode = 1;
        break;
    case 2:
        mob->fight_mode = 2;
        break;
    case 3:
        mob->fight_mode = 3;
        break;
    }
}

const char *heights[5][5] =
{
    {"extremely short", "dwarfish", "tiny", "bantam", "insubstantial"},
    {"short", "small", "diminuitive", "compact", "little"},
    {"average", "usual", "regular", "ordinary", "unremarkable"},
    {"tall", "lofty", "statuesque", "long-limbed", "long-legged"},
    {"extremely tall", "towering", "great", "gigantic", "oversized"}
};

const char *unfit_frames[5][5] =
{
    {"emaciated", "skeletal", "puny", "shriveled", "sickly-thin",},
    {"underweight", "fragile", "malnourished", "weak", "underdeveloped"},
    {"unhealthy", "unfit", "decrepit", "gaunt", "haggard"},
    {"flabby", "fat", "pudgy", "rheumy", "oafish"},
    {"obese", "corpulently fat", "grossly big", "tubercular", "morbidly obese"}
};

const char *avg_frames[5][5] =
{
    {"bony", "stick-thin", "wraithlike", "scrawny", "stringy"},
    {"slight", "slender", "thin", "fragile", "petite"},
    {"average", "usual", "regular", "ordinary", "unremarkable"},
    {"large", "broad", "fat", "chubby", "plump"},
    {"obese", "prodigious", "elephantine", "substantial", "roly-poly"}
};

const char *fit_frames[5][5] =
{
    {"gaunt", "beanpole-thin",  "whipcord-thin", "bony", "waifish"},
    {"lean", "sinewy", "rawboned", "willowy", "sleek"},
    {"athletic", "hearty", "fit", "limber", "rugged"},
    {"muscular", "brawny", "robust", "musclebound", "thickset"},
    {"hulking", "powerful", "strapping", "herculean", "mammoth"}
};

const char *fem_ages[8][2] =
{
    {"baby", "toddler"},
    {"girl", "lass"},
    {"female youth", "female teen"},
    {"maiden", "young woman"},
    {"woman", "female"},
    {"middle-aged woman", "mature-aged woman"},
    {"aged woman", "elderly woman"},
    {"crone", "old woman"}
};

const char *male_ages[8][2] =
{
    {"baby", "toddler"},
    {"boy", "lad"},
    {"male youth", "male teen"},
    {"young man", "youthful man"},
    {"man", "male"},
    {"middle-aged man", "mature-aged man"},
    {"aged man", "elderly man"},
    {"geezer", "old man"}
};

const char *hair_lengths[7] =
{
    "bald",
    "shaved",
    "short",
    "average-length",
    "long",
    "very-long",
    "extremely-long"
};

const char *hair_styles[7][5] =
{
    {"bald", "balding", "bald", "balding", "bald"},
    {"shaved", "shaven", "crew-cut", "buzz-cut", "balding"},
    {"neat", "well-groomed", "flat-styled", "natural-styled", "plain-styled"},
    {"curly", "wavy", "tousled", "flowing", "curled"},
    {"unwashed", "greasy", "ratty", "thatched", "knotty"},
    {"unruly", "wild", "unkempt", "disheveled", "messy"},
    {"braided", "pony-tailed", "pig-tailed", "dreadlocked", "tressed"}
};

const char *hair_colors[5][10] =
{
    {"blond", "blond", "dirty-blond", "fair-blond", "flaxen", "golden-blond", "light-blond", "platinum-blond", "sandy-blond", "straw-blond"}, // 9
    {"brown", "brown", "light-brown", "auburn", "ash-brown", "golden-brown", "chestnut-brown", "chocolate-brown", "dark-brown", "deep-brown"}, // 9
    {"black", "black", "sable", "ebony", "jet-black", "raven-black", "blue-black", "pitch-black", "coal-black", "night-black"}, // 9
    {"red", "red", "light-auburn", "dark-auburn", "strawberry-blond", "flame-red", "light-red", "dark-red", "orange", "light-orange"}, // 9
    {"grey", "grey", "light-grey", "dark-grey", "dirty-grey", "salt-and-pepper", "white", "off-white", "white-streaked", "greying"} // 9
};

const char *eye_colors[6][10] =
{	
	{"black", "black", "midnight-black", "jet-black", "charcoal", "slightly-darker-black", "obsidian-black", "basalt", "matte-black", "pitch-black"},
    {"blue", "blue", "sky-blue", "clear-blue", "light-blue", "dark-blue", "sea-blue", "crystal-blue", "cornflower-blue", "cerulean"},
    {"green", "green", "light-green", "dark-green", "sea-green", "olive-green", "snake-green", "forest-green", "bottle-green", "grass-green"},
    {"brown", "brown", "light-amber", "dark-amber", "light-hazel", "dark-hazel", "light-brown", "dark-brown", "dusky-brown", "sienna"},
    {"grey", "grey", "dark-grey", "light-grey", "storm-grey", "steel-grey", "ash-grey", "pearl-grey", "sea-grey", "shell-grey"},
    {"mismatched", "blue-green", "blue-brown", "blue-grey",  "blue-hazel", "green-brown", "green-grey", "green-hazel", "brown-grey", "brown-hazel"}
};


const struct mob_variable_data mutation_list[NUM_MUTATION + 1] =
{
    {"skin", "albino", "$s skin is absent of any pigment, instead a ghostly white",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "melanistic", "splots of darkly colored skin can be seen along $s limbs",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "translucent", "$s skin is exceptionally pale and thin, revealing a network of veins and arteries beneath",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "harlequin", "$s outer flesh is excessively dry, the cracked and flaking skin doing little to hide the red, perpetually irritated dermis",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "plate-skinned", "$s is covered in diamond-shaped plates of leathery skin, thin membranes appearing where the plates connect",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "mucus-covered", "$s skin constantly produces a film of thick, sticky substance",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "pustule-covered", "$s body is covered in inflamed blisters of varying sizes, many oozing a reeking fluid",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "spine-covered", "fragile bony spikes erupt from $s flesh at odd intervals, pushing up from almost every visible surface",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin", "piebald", "various ares of $s skin are much paler than the surrouding patches",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"skin", "scale-skinned", "present at each jucture of $s joints is a patch of reptilian-like skin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"shape", "ridge-spined", "the skin around $s back is stretched taut around sections of ridgy, hardened vertebrae",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"shape", "curve-spined", "the upper part of $s spine has curved in to a hunched back",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"shape", "concave-spined", "$s back slopes inwards causing $m to always be leaning backwards",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"shape", "ape-like", "$e appears closer to ape than humanity, with hunched shoulders and arms reaching to $s knees",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"shape", "quadrupedal", "$s person spine curves such that $s arms and legs are near equal in length and walking on all fours is necessary",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"shape", "elbow-spurred", "twin spikes of smooth bone ending in blunt points extend from $s elbows",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"limbs", "odd-limbed", "$s limbs are without congruence, seemingly thrown together at random than matching one another",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "three-armed", "a third arm, malformed and withered, sprouts from beneath the pit of $s right arm",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "three-armed", "a third arm, malformed and withered, sprouts from beneath the pit of $s left arm",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "polydactyl", "$s has a sixth, stunted digit on each hand",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "three-fingered", "each of $s hands bears only a thumb and two warped, thick fingers",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "webbed", "a thin flap of diaphanpous skin stretches between $s fingers and toes",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "digitigrade", "$s legs are malformed such that $e must balances $s weight forward on $s toes",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"limbs", "sticky-fingers", "the ends of $s fingers finish with sticky, round tips",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "sticky-toes", "the ends of $s toes finish with sticky, round tips",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "claw-feet", "$s feet have been replaced with a pair of three-digit claws",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"eyes", "lidless", "$s possesses no eyelids, their eyes bloodshot and constantly weeping",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "bug-eyed", "$s eyes are large, near bulging from $s head",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "bulge-eyed", "$s left eye is drastically smaller than $s right",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "bulge-eyed", "$s right eye is drastically smaller than $s left",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "tiny-eyed", "$s eyes are tiny, failing to fill the bony sockets and hence exposing much sunken flesh instead",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "cyclopic", "$e has one large eye in the centre of $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"eyes", "iris-less", "$s eyes lack defined irises",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"face", "razor-mouth", "$s mouth is filled with dozens of small, razor sharp teeth",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "lipless", "$e possesses no lips, their teeth permanently exposed",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "dog-faced", "$s jaws have protruded at such an abnormal angle to shape $s face in such a way to look like the muzzle of a dog",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "fanged", "$s incisors are significantly long and sharp",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "long-tongued", "$s tongue is truly impressive in length",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "tendril-bearded", "lank, lifeless tendrils of flesh sprout from $s chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "noseless", "instead of a nose, only a flat triangle of skin with two small openings adorn $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "bony-browed", "$s brow has been overtaken by small, bony plates that shadow $s eyes",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "earless", "$e has no ears, left instead with a small fleshy nub either side of $s head",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"face", "cleft-lip", "$s upper lip is split jaggedly and pulls up to expose their teeth resulting in a cleft palate",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "mole-faced", "$s face is covered in multiple, large, and hairy moles",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "one-nostriled", "one of $s nostrils is completely closed over by skin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"blemish",	"freckled",		"$s face is heavily freckled",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON}
};

const struct mob_variable_data cybernetic_list[NUM_CYBERNETIC + 1] =
{
    {"eyes", "luminsecent-eyed", "$s pupils emit a soft colored light",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "metal-eyed", "instead of eyes, $e has a pair of segmented metal replacements",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "glass-eyed", "instead of eyes, $e has a pair of colored glass orbs",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "slit-eyed", "$s has thin, colored slits for pupils, surgical scarring visible on $s eyelids",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"eyes", "goggled", "instead of eyes, $e has a pair of dark-lensed goggles welded to $s flesh",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"eyes", "flicker-eyed",  "$s eyes flicker and flash from time to time with an electric current",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"limbs", "clawed", "$s hands have been replaced with a pair of three-digit claws",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-kneed", "$s left knee has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-kneed", "$s right knee has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-elbowed", "$s left elbow has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-elbowed", "$s right elbow has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "arm-plated", "$s arms are welded throughout with sections of metal plating",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "plastic-limbed", "various sections of $s limbs have been replaced with high-strength plastic-and-wire counterparts",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "metal-limbed", "various sections of $s limbs have been replaced with high-strength metal-and-wire counterparts",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-shouldered", "$s right shoulder has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "socket-shouldered", "$s left shoulder has been replaced with a metal sphere and pistons",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"limbs", "razor-knuckled", "sharp, jagged pieces of metal protrude from $s knuckles",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "wire-handed", "$s right hand is a garbled mess of an artificial metal and wire frames",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"limbs", "wire-handed", "$s left hand is a garbled mess of an artificial metal and wire frames",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"face", "stud-cheeked", "rounded metal studs trail the temple to the jaw on each side of $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "metal-skulled", "the upper half of $s head is capped with strips of metal",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "metal-toothed", "all of $s teeth have been replaced by metal counterparts",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "earless", "small metal discs rest where $s ears should instead",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "voiceboxed", "a mesh-grilled disc extends from where $s larynx should be",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "plastic-jawed", "instead of a jaw, a asymmetrical, squarish plastic attachment completes the bottom of $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "metal-jawed", "instead of a jaw, a asymmetrical, triangular metal attachment completes the bottom of $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "metal-faced", "large portions of $s face has been replaced with sections of metal",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "plastic-faced", "large portions of $s face has been replaced with sections of plastic",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "mask-faced", "rather than a face, $e has a something akin to a respirator mask, tubing crossing back and forth and in and out of his neck",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "wire-eared", "rather than ears, $e has a pair of stapled wires and antenna that run up the side of $s head",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face", "respiratored", "the lower half of $s face is replaced with a respirator",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"face", "plastic-skulled", "many patches of plastic form makeshift barriers for head wounds upon $s skull",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"face", "plastic-eared", "two curved bits of flexible plastic serve as replacements for $s outer ear structures",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"face", "metal-cheeked", "$s cheekbones have been replaced by smoothed, metal arcs that protrude from the skin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"reinforced", "wire-covered", "wires sprawl across $s form, sprouting about $s body",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "synthetic-fleshed", "large sections of $s skin have been replaced by an obvious, rubbery synthetic",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "plastic-fleshed", "large sections of $s skin have been replaced by pieces of hard plastic",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "metal-fleshed", "large sections of $s skin have been replaced by pieces of warm metal",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "patch-plated", "large sections of $s skin have been replaced by a mish-mash of metal, plastic, and synthetic flesh",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "metal-spined", "clamps of metal run down $s spine, molded carefully into the flesh",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"reinforced", "plastic-spined", "clamps of plastic run down $s spine, molded carefully into the flesh",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"cybernetic", "wheezing", "small tubes run about $s body, $s breathing noticably loud and unfailingly regular",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"cybernetic", "wire-veined", "where $s veins should be visible, there are instead thick black and blue lines of wire",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"cybernetic", "ticking", "$s body makes a barely perceivable and irregular ticking noise",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"cybernetic", "paneled", "several rows of solar panels run across $s body",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
	{"cybernetic", "glowing", "something beneath $s chest cavity causes the skin to luminesce",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"blemish",	"freckled",		"$s face is heavily freckled",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON}
};

const struct mob_variable_data feature_list[NUM_FEATURES + 1] =
{

    //category	sdesc			full desc					sex	must	not	min_age	max_age
    {"skin",	"jaundiced",		"$s skin is somewhat yellowed and jaundiced",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON
    },
    {"skin",	"sunburnt",		"$s skin is a harsh brownish-red in hue and $e is clearly sunburnt",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"wan",			"$s features are somewhat pale and wan",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"sallow",		"$s skin is rather sallow",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"pale",			"$e is rather pale-skinned from head to toe",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"tanned", 		"$e is covered by an even, smooth tan",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"olive-skinned",	"$s skin is of an even olive shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"dusky",		"$s skin is of an even dusky shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"fair",			"$s skin is of a rather fair shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"dark",			"$s skin is of a rather dark shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"brown-skinned",	"$s skin is of a brownish shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"hale",			"$s skin is a hale and hearty shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"swarthy",		"$s skin is a swarthy brown shade",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"white-skinned",	"$s skin is creamy white: untouched by the sun",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"transculent",		"$s skin is translucent with veins easily visible",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"skin",	"pink-skinned",		"$s skin is constantly pink-hued",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"hand",	"long-fingered",	"$s fingers are long and delicate",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"spidery-fingered",	"$s fingers are slender and agile",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"callused",		"$s hands and fingers are lightly callused",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"heavy-callused",	"$s hands and fingers are heavy callused",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"missing-fingered",	"one of the fingers on $s left hand is missing",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"missing-fingered",	"one of the fingers on $s right hand is missing",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"scar-handed",		"$s hands are heavily scarred and marked",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"stubby-fingered",	"$s fingers and hands are short and stubby",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"big-handed",		"$s hands are unusually large",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"small-handed",		"$s hands are unusually small",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"hand",	"big-knuckled",		"$s fingers have huge, protruding knuckles",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"fhair",	"clean-shaven",		"$s chin and cheeks are cleanly shaven",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"smooth-cheeked",	"$s chin and cheeks are free and smooth of any hair",	SEX_FEMALE,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"fhair",	"stubbly",		"$s chin and cheeks are covered by stubble",	SEX_FEMALE,	-1,	-1,	AGE_CHILD,	AGE_SKELETON},
    {"fhair",	"patchy-bearded",	"$s chin and cheeks are covered by a patchy attempt at a beard",	SEX_FEMALE,	-1,	-1,	AGE_CHILD,	AGE_SKELETON},
    {"fhair",	"bearded",		"a short $z beard hangs from $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"neatly-bearded",	"a well-groomed $z beard hangs from $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"trim-bearded",		"a short and well-groomed $z beard hangs from $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"bushy-bearded",	"a short yet unruly $z beard hangs from $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"long-bearded",		"a long $z beard adorns $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"braid-bearded",	"a long $z beard tied in to a briad adorns $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"wild-bearded",		"a wild $z beard covers much of $s face",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"goateed",		"a neat $z goatee adorns $s chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"neat-mustached",		"a neat $z mustache adorns $s face",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"pencil-mustached",	"a thin $z pencil mustache adorns $s face",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"full-bearded",		"a neat $z beard covers $s cheeks and chin",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"mutton-chopped",	"an pair of unruly $z mutton chops extend extend from $s temples to $s jaw line",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",	"grizzled",		"$s chin and cheeks are covered by a grizzly growth of $z facial hair",	SEX_FEMALE,	-1,	-1,	AGE_TEEN,	AGE_SKELETON},
    {"fhair",    "wild-mustached",            "a wild $z mustache sprawls out from $s face",    SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"fhair",    "thick-mustached",            "a thick, wide $z mustache adorns $s face",    SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"fhair",     "toothbrush-mustached",        "a thick, square-shaped $z mustache adorns $s face", SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"fhair",    "handlebar-mustached",         "an elegantly curving $z mustache adorns $s face", SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"fhair",    "chinese-mustached",         "the sides of $s thin $z mustache droop down to $s chin", SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},


    {"lbody",        "narrow-hipped",            "$s hips are no wider than her waist",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "wide-hipped",            "$e bears prominent, wide hips",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "long-legged",            "long legs descend from $s hipside",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "stout-legged",            "stout legs descend from $s hipside",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "thin-legged",            "bone-thin legs descend from $s hipside",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "shapely-legged",        "feminine, shapely legs descend from $s hipside",    SEX_MALE,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "hairy-legged",            "hairs cover $s entire legs",    SEX_FEMALE,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"lbody",        "one-legged",            "$s left leg is missing",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "one-legged",            "$s right leg is missing",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "bow-legged",            "$s legs are grotesquely arched outwards",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "club-footed",            "$s feet are hideously twisted around the ankle",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "narrow-footed",            "$e has distinctly narrow feet",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "large-footed",            "$e has distinctly large feet",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "small-footed",            "$e has distinctly small feet",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"lbody",        "stump-footed",            "$s left leg ends in a ghastly-looking stump",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"lbody",        "stump-footed",            "$s right leg ends in a ghastly-looking stump",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"lbody",        "prosthetic-footed",        "A durable prosthetic foot stems $s from his left leg",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"lbody",        "prosthetic-footed",        "A durable prosthetic foot stems $s from his right leg",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"lbody",        "peg-legged",            "$s left leg ends in a metal peg",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"lbody",        "peg-legged",            "$s right leg ends in a metal peg",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},



    //category	sdesc			full desc					sex	must	not	min_age	max_age
    {"nose",	"button-nosed",		"a button nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"hawk-nosed",		"a large, hawkish nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"aquiline-nosed",	"a large, aquiline nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"large-nosed",		"a large nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"small-nosed",		"a small nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"crooked-nosed",	"a crooked nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"scar-nosed",		"a scarred nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"broken-nosed",		"a once-broken nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"bulbous-nosed", 	"a bulbous nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"pig-nosed",		"a nose that would not go amiss on a pig completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"narrow-nosed",		"a narrow nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"rat-nosed",		"a long and pointy nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"upturned-nosed",	"an upturned nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"pug-nosed",		"a small and stubby nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"red-nosed",		"a constantly reddish nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"hook-nosed",		"a long hooked nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"flat-nosed",		"a broad flat nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"wide-nosed",		"a thick wide nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"nose",	"squash-nosed",		"a deformed squashed nose completes $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"chin",	"weak-chinned",		"$e has a weak, sloped chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"cleft-chinned",	"$e has a dimpled, cleft chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"hard-chinned",		"$e has a solid, well-defined chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"chinless",		"$s weak, sloping chin is almost invisible",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"round-chinned",	"$e has a soft, rounded chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"square-chinned",	"$e has a hard, squared chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"pointed-chinned",	"$s chin tapers to a point",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"dimple-chinned",	"$s chin has a noticable dimple",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"receeding-chinned",	"$s chin noticable receeds",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"slope-chinned",	"$s chin slopes backwards",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"double-chinned",	"$s chin joins their neck in a roll of fat",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"fat-chinned",		"$s has a fat and heavy chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"chin",	"wobbly-chinned",	"$s has a fat and wobbling chin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"jaw",		"square-jawed",		"$s jaw is broad and square",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"lantern-jawed",	"$s jaw is long and thin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"wide-jawed",		"$s jaw is short and wide",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"narrow-jawed",		"$s jaw is narrow",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"weak-jawed",		"$s jaw is shadowed and unnoticable",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"small-jawed",		"$s jaw is small and short",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"under-bitten",		"$s has a noticable underbite",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"over-bitten",		"$s has a noticable overbite",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"jaw",		"parrot-mouthed",	"$s has an extreme overbite",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"face",	"angular-faced",	"$s face is rather angular and sharp",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"broad-faced",		"$s face is rather broad and flat",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"narrow-faced",		"$s face is rather narrow",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"long-faced",		"$s face is rather long",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"wedge-faced",		"$s face is rather wedge-shaped",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"high-cheekboned",	"$s face is marked by high cheekbones",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"dimple-cheeked",	"$s face has a prominent pair of dimples",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"sloping-browed",	"$e has a long, angular brow",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"horse-faced",		"$e has a long and narrow face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"pig-faced",		"$e has a squat and chubby face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"rat-faced",		"$e has a pointed and narrow face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"fat-faced",		"$s face is noticably pudgy",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"face",	"gaunt-faced",		"$s face is significantly gaunt",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    //category	sdesc			full desc					sex	must	not	min_age	max_age
    {"ear",		"big-eared",		"$s ears are noticably big",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"ear",		"small-eared",		"$s ears are noticably small",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"ear",		"one-eared",		"$s left ear is missing",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"ear",		"one-eared",		"$s right ear is missing",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"ear",		"pointy-eared",		"$s ears are prominently pointed",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"ear",		"round-eared",		"$s ears are small and round",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"visage",	"dour",			"$s expression is habitually dour",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"amaiable",		"$s expression is habitually amiable",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"disinterested",	"$s maintains a habitual air of disinterest",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"intent",		"$s expression is habitually intent",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"scowling",		"$s expression is habitually set in a scowl",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"cheerful",		"$s expression is habitually cheerful",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"thoughtful",		"$s expression is habitually thoughtful",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"pensive",		"$s expression is habitually pensive",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"distant",		"$s expression is habitually distant",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"blank-faced",		"$s expression is habitually emotionless",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"cheery",		"$s expression is habitually cheery",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"predatory",		"$s expression is habitually predatory",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"shifty",		"$s expression is habitually shifty",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"dutiful",		"$s expression is habitually dutiful",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"carefree",		"$s expression is habitually carefree",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"visage",	"nervous",		"$s expression is habitually nervous",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"lip",		"thick-lipped",		"$s lips are especially thick",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"lip",		"big-lipped",		"$s lips are especially big",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"lip",		"fish-lipped",		"$s lips are especially big and moist",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"lip",		"thin-lipped",		"$s lips are especially thin",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"lip",		"small-lipped",		"$s lips are especially small",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"lip",		"crack-lipped",		"$s lips are especially cracked and chapped",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"teeth",	"buck-toothed",		"$s two front teeth are particularly prominent",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"gap-toothed",		"there is a noticable gap between $s two front teeth",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"rotten-toothed",	"many of $s teeth are decaying and rotten",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"stained-toothed",	"$s teeth are stained varying shades of yellow and grey",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"white-toothed",	"$s teeth are noticably white and healthy",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"perfect-toothed",	"$s teeth are in perfect condition",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"broken-toothed",	"one of $s front teeth is chipped in half",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"gold-toothed",		"$e has a number of golden teeth",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"silver-toothed",	"$e has a number of silver teeth",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"teeth",	"toothless",	        "$e is missing most of $s teeth",	-1,	-1,	-1,	AGE_AGED,	AGE_SKELETON},

    //category	sdesc			full desc					sex	must	not	min_age	max_age
    {"scars",	"scarred", 		"$e has a number of prominent scars",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"heavily-scarred",	"much of $s body is heavily scarred",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"gruesomely-scarred",	"horrific scars cover most of $s body",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"scar-faced",		"a prominent scar marks $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"scar-marked",		"numerous old, faded scars adorn $s form",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"faintly-scarred",	"$e bears a number of faded shadows from scars past",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"burn-scarred",		"$e bears a number of burn scars",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"scars",	"flame-touched",	"$e bears horrific burn scars all over $s body",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"blemish",	"warty",		"$e has a number of warts",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"wart-faced",		"$e has a noticable wart on $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"acned",		"$e is afflicted by severe acne",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"pimply",		"$s skin is rather pimply",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"pock-marked",		"$e has a number of pock-marks and acne-scars",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"mole-faced",		"$e has a noticable mole on $s face",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"mole-cheeked",		"$e has a noticable mole on $s cheek",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"freckled",		"$e is heavily freckled",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"freckle-faced",	"$s face is heavily freckled",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"blemished",		"$s skin bears a number of minor blemishes",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"blemish",	"smooth-skinned",	"$s skin is smooth of any blemish or mark",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"stance",	"rolling-gaited",	"$e moves with a rolling gait",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"striding", 		"$e moves with a confident stride",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"marching", 		"$e moves with a measured march",	-1,	ACT_ENFORCER,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"mincing",		"$e moves in nervous, mincing steps",	-1,	-1,	ACT_ENFORCER,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"strutting",		"$e moves with a formal strut",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"shambling",		"$e moves in shambling motions",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"trudging",		"$e moves with a deliberate trudge",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"slinking",		"$e moves with a careful slink",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"pacing",		"$e moves with a deliberate step",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"loping",		"$e moves with a loping gait",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},
    {"stance",	"shuffling",		"$e moves with a shuffled step",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON},

    {"eyebrows",    "eyebrowless",            "$s eyebrows are completely missing",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "delicate-browed",        "$s eyebrows have a delicate, faint shape",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "slender-browed",        "$s eyebrows have a slender shape",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "thick-browed",            "$s eyebrows are characteristically thick",    -1,    -1,    -1,    AGE_CHILD,    AGE_SKELETON},
    {"eyebrows",    "bushy-browed",            "$s eyebrows twirl out like tiny wild bushes",    -1,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"eyebrows",    "mono-browed",            "$s eyebrows are fused together, forming a single ridge",     -1,    -1,    -1,    AGE_TEEN,    AGE_SKELETON},
    {"eyebrows",    "flat-eyebrowed",        "$s eyebrows have a thoroughly flat appearance",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "round-eyebrowed",        "$s eyebrows bear a small arch",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "sharp-eyebrowed",        "$s eyebrows bend sharply",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "ever-frowning",        "$s eyebrows appear twisted into a persistent frown",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "bright-eyebrowed",        "the color of $s eyebrows is particularly bright",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},
    {"eyebrows",    "dark-eyebrowed",        "the color of $s eyebrows is particularly dark",    -1,    -1,    -1,    AGE_BABY,    AGE_SKELETON},


    {"blemish",	"freckled",		"$s face is heavily freckled",	-1,	-1,	-1,	AGE_BABY,	AGE_SKELETON}
};

/*                                                                          *
 * function: create_description                                             *
 *                                                                          *
 * 09/28/2004 [JWW] - Added travel strings some arbitrary mobs              *
 *                                                                          */
void new_create_description (CHAR_DATA * mob,
                             char *rAge, char *rHeight, char *rFrame, char *rEyes, char *rLength, char *rColor, char *rStyle, char *rOne, char *rTwo, char *rThree, char *rFour)
{
    int roll, i, j, x;

    int size = 0;
    int frames = 0;
    int ind_age;
    int age_ind;
    int hair_ind;

    char buf[MAX_STRING_LENGTH] = { '\0' };
    char buf2[MAX_STRING_LENGTH] = { '\0' };
    char buf3[MAX_STRING_LENGTH] = { '\0' };
    char buf4[MAX_STRING_LENGTH] = { '\0' };

    int fitness = 0;
    char height[AVG_STRING_LENGTH] = { '\0' };
    char frame[AVG_STRING_LENGTH] = { '\0' };
    char age[AVG_STRING_LENGTH] = { '\0' };
    char eyes[AVG_STRING_LENGTH] = { '\0' };
    char hair_color[AVG_STRING_LENGTH] = { '\0' };
    char hair_style[AVG_STRING_LENGTH] = { '\0' };
    char hair_length[AVG_STRING_LENGTH] = { '\0' };

    char *feat[4];
    char *feat_cat[4];
    char *feat_full[4];
    int feat_ind[4] = {-1,-1,-1,-1};

    for (int ind = 0; ind < 4; ind++)
    {
        feat[ind] = '\0';
        feat_cat[ind] = '\0';
        feat_full[ind] = '\0';
    }

    char *sentence[4];
    for (int ind = 0; ind < 4; ind++)
    {
        sentence[ind] = '\0';
    }

    char temp_sentence[MAX_STRING_LENGTH] = { '\0' };

    char sdesc[MAX_STRING_LENGTH] = { '\0' };
    char temp_adj[AVG_STRING_LENGTH] = { '\0' };
    char adj1[AVG_STRING_LENGTH] = { '\0' };
    char adj2[AVG_STRING_LENGTH] = { '\0' };
    char adj3[AVG_STRING_LENGTH] = { '\0' };

    bool true_hair = false;
    bool temp_hair = false;
    bool found = false;
    bool avg_height = false;
    bool avg_weight = false;
    bool reject = false;
    bool mixed = false;

    bool no_haireyes = false; // Certain categories of cybernetics and mutations rules out hair and eye descriptors.

    //send_to_gods ("Test");

    // First of all, we check whether we got a variable from age.
    // If we do, we need to check to make sure it matches something on either on the lists - if it does, set the sex and age index to that variable
    // Otherwise, we need to pull one at random.
    // We do this by rolling a 2d35 so it's weight to the middle.
    // Once we've got their age, we apply the modifier based on sex and age for their primary keyword, e.g. "man", "eldery woman", etc.

    if (rAge && *rAge)
    {
        found = false;
        for (int xind = 0; xind < 2; xind ++)
        {
            for (int ind = 0; ind < 8; ind ++)
            {
                if (!str_cmp(male_ages[ind][xind], rAge))
                {
                    found = true;
                    age_ind = ind;
                    break;
                }
            }

            if (found)
            {
                mob->sex = SEX_MALE;
                sprintf(age, "%s", rAge);
                break;
            }
        }

        if (!found)
        {
            for (int xind = 0; xind < 2; xind ++)
            {
                for (int ind = 0; ind < 8; ind ++)
                {
                    if (!str_cmp(fem_ages[ind][xind], rAge))
                    {
                        found = true;
                        age_ind = ind;
                        break;
                    }
                }

                if (found)
                {
                    mob->sex = SEX_FEMALE;
                    sprintf(age, "%s", rAge);
                    break;
                }
            }
        }
    }

    if (!*age)
    {


        if (IS_SET (mob->act, ACT_ENFORCER))
        {
            ind_age = number(20,50);
        }
        else
        {
            ind_age = number(1,35) + number (1,35);
            ind_age = MAX(8, ind_age);
        }

        if (ind_age >= 56)
            age_ind = AGE_AGED;
        else if (ind_age >= 46)
            age_ind = AGE_MATURE;
        else if (ind_age >= 26)
            age_ind = AGE_ADULT;
        else if (ind_age >= 20)
            age_ind = AGE_YOUNG;
        else if (ind_age >= 13)
            age_ind = AGE_TEEN;
        else
            age_ind = AGE_CHILD;

        if (mob->sex == SEX_FEMALE)
            sprintf(age, "%s", fem_ages[age_ind][number(0,1)]);
        else
            sprintf(age, "%s", male_ages[age_ind][number(0,1)]);
    }

    // Get their height, put it between 0 and 4, then have a chance of reducing based on age to avoid uber-tall kids.

    if (rHeight && *rHeight)
    {
        found = false;
        for (int xind = 0; xind < 5; xind ++)
        {
            for (int ind = 0; ind < 5; ind ++)
            {
                if (!str_cmp(heights[ind][xind], rHeight))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                sprintf(height, "%s", rHeight);
                break;
            }
        }
    }

    if (!*height)
    {
        size = get_size(mob);
        size -= 2;
        if (age_ind >= AGE_AGED)
            size -= 1;
        else if (age_ind == AGE_CHILD)
            size -= 2;
        else if (age_ind == AGE_TEEN && number(0,1))
            size -= 1;
        size = MAX (0, size);
        size = MIN (4, size);

        sprintf(height, "%s", heights[size][number(0,4)]);
        if (size == 2)
            avg_height = true;
    }


    // Get their size, put it between 0 and 4, then have a chance of reducing based on age

    if (rFrame && *rFrame)
    {
        found = false;
        for (int xind = 0; xind < 5; xind ++)
        {
            for (int ind = 0; ind < 5; ind ++)
            {
                if (!str_cmp(unfit_frames[ind][xind], rFrame))
                {
                    mob->frame = ind + 1;
                    found = true;
                    break;
                }
            }

            if (found)
            {
                sprintf(frame, "%s", rFrame);
                break;
            }
        }

        if (!found)
        {
            for (int xind = 0; xind < 5; xind ++)
            {
                for (int ind = 0; ind < 5; ind ++)
                {
                    if (!str_cmp(avg_frames[ind][xind], rFrame))
                    {
                        found = true;
                        mob->frame = ind + 1;
                        break;
                    }
                }

                if (found)
                {
                    sprintf(frame, "%s", rFrame);
                    break;
                }
            }
        }


        if (!found)
        {
            for (int xind = 0; xind < 5; xind ++)
            {
                for (int ind = 0; ind < 5; ind ++)
                {
                    if (!str_cmp(fit_frames[ind][xind], rFrame))
                    {
                        found = true;
                        mob->frame = ind + 1;
                        break;
                    }
                }

                if (found)
                {
                    sprintf(frame, "%s", rFrame);
                    break;
                }
            }
        }
    }

    if (!*frame)
    {
        frames = mob->frame;
        frames -= 1;
        if (age_ind >= AGE_AGED || age_ind == AGE_CHILD)
        {
            frames -= 1;
            mob->frame -= 1;
            mob->frame = MAX(0, mob->frame);
        }
        else if (age_ind == AGE_TEEN && number(0,1))
        {
            frames -= 1;
            mob->frame -= 1;
            mob->frame = MAX(0, mob->frame);
        }
        frames = MAX(0, frames);
        frames = MIN(4, frames);

        // Now, we get their fitness: if they have both great strength and great constitution,
        // they're fit.
        // If they've got less than 22, then they're pretty weak and scrawny, and we'll call them unfit.

        fitness = GET_CON(mob) + GET_STR(mob);

        if (fitness >= 31)
            sprintf(frame, "%s", fit_frames[frames][number(0,4)]);
        else if (fitness <= 22)
            sprintf(frame, "%s", unfit_frames[frames][number(0,4)]);
        else
        {
            if (frames == 2)
                avg_weight = true;
            sprintf(frame, "%s", avg_frames[frames][number(0,4)]);
        }
    }

    if (rEyes && *rEyes)
    {
        found = false;
        for (int xind = 0; xind < 10; xind ++)
        {
            for (int ind = 0; ind < 5; ind ++)
            {
                if (!str_cmp(eye_colors[ind][xind], rEyes))
                {
                    found = true;
                    if (ind == 4)
                        mixed = true;
                    break;
                }
            }

            if (found)
            {
                sprintf(eyes, "%s", rEyes);
                break;
            }
        }
    }

    // Figure out eye color: 25% blue, 55% brown, 10% grey, 9% green and 1% mixed.
    if (!*eyes)
    {
        x = number (1,100);

        if (x >= 75)
            sprintf(eyes, "%s", eye_colors[0][number(1,9)]);  // blue
        else if (x >= 20)
            sprintf(eyes, "%s", eye_colors[2][number(1,9)]);  // brown
        else if (x >= 10)
            sprintf(eyes, "%s", eye_colors[0][number(1,9)]);  // green
        else if (x > 1)
            sprintf(eyes, "%s", eye_colors[3][number(1,9)]);  // grey
        else
        {
            mixed = true;
            sprintf(eyes, "%s", eye_colors[4][number(1,9)]);  // mixed
        }
    }

    if (rLength && *rLength)
    {
        for (int ind = 0; ind < 7; ind ++)
        {
            if (!str_cmp(hair_lengths[ind], rLength))
            {
                sprintf(hair_length, "%s", rLength);
                hair_ind = ind;
                break;
            }
        }
    }

    if (rColor && *rColor)
    {
        found = false;
        for (int xind = 0; xind < 10; xind ++)
        {
            for (int ind = 0; ind < 5; ind ++)
            {
                if (!str_cmp(hair_colors[ind][xind], rColor))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                sprintf(hair_color, "%s", rColor);
                break;
            }
        }
    }

    if (rStyle && *rStyle)
    {
        found = false;
        for (int xind = 0; xind < 5; xind ++)
        {
            for (int ind = 0; ind < 7; ind ++)
            {
                if (!str_cmp(hair_styles[ind][xind], rStyle))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                sprintf(hair_style, "%s", rStyle);
                break;
            }
        }
    }

    if (!*hair_color || !*hair_length || !*hair_style)
    {
        // Figure out hair color: 25% blond, 35% brown, 35% black, 10% red.

        x = number (1,100);
        if (x >= 75)
            sprintf(hair_color, "%s", hair_colors[0][number(1,9)]); // blond
        else if (x >= 40)
            sprintf(hair_color, "%s", hair_colors[1][number(1,9)]);  // brown
        else if (x >= 10)
            sprintf(hair_color, "%s", hair_colors[2][number(1,9)]); // black
        else
            sprintf(hair_color, "%s", hair_colors[3][number(1,9)]); // red

        // If we're of a certain age, there's a good chance our hair is instead grey.
        // mature: 20%
        // aged: 50%
        // eldery: 95%

        if (age_ind == AGE_MATURE && !number(0,4))
            sprintf(hair_color, "%s", hair_colors[4][number(1,9)]); // grey
        else if (age_ind == AGE_AGED && number(0,1))
            sprintf(hair_color, "%s", hair_colors[4][number(1,9)]); // grey
        else if (age_ind == AGE_ELDERY && number(0,19))
            sprintf(hair_color, "%s", hair_colors[4][number(1,9)]); // grey

        // 25% chance for short, medium or long.
        // Otherwise, a 7.25% chance for bald, shaven, very long or extremely long.
        if (number(0,3))
            hair_ind = number(2,4);
        else if (number(0,1))
            hair_ind = number(0,1);
        else
            hair_ind = number (5,6);

        // If we're bald, but female or under the age of 25, we get a different hair length.
        if (hair_ind == 0)
        {
            if (mob->sex == SEX_FEMALE || age_ind <= AGE_YOUNG)
                hair_ind = number(1,6);
        }
        // Otherwise, if we're not bald, we're less likely to have hair the older we get as a male.
        else
        {
            if (mob->sex == SEX_MALE && age_ind >= AGE_MATURE)
            {
                hair_ind -= 1;
                hair_ind = MAX(0, hair_ind);
            }
        }

        // Now we write our length.
        sprintf(hair_length, "%s", hair_lengths[hair_ind]);

        // Now we need a hair style - if we're bald or shaven, we have limited choice.
        // Otherwise, we can be more random in the sort of hair we get.
        if (hair_ind == 0 || hair_ind == 1)
            sprintf(hair_style, "%s", hair_styles[hair_ind][number(0,4)]);
        else
            sprintf(hair_style, "%s", hair_styles[number(2,6)][number(0,4)]);
    }

    if (mob->race == lookup_race_id("Mutation"))
    {
        if (rOne && *rOne)
        {
            found = false;
            for (int xind = 0; xind < NUM_MUTATION; xind ++)
            {
                if (!str_cmp(mutation_list[xind].sdesc, rOne))
                {
                    found = true;
                    feat[0] = str_dup(mutation_list[xind].sdesc);
                    feat_ind[0] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        if (rTwo && *rTwo)
        {
            found = false;
            for (int xind = 0; xind < NUM_MUTATION; xind ++)
            {
                if (!str_cmp(mutation_list[xind].sdesc, rTwo))
                {
                    found = true;
                    feat[1] = str_dup(mutation_list[xind].sdesc);
                    feat_ind[1] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        // Now we need to get two random features from different cateogires,
        // and much sure we only draw from each category once and that we
        // category we draw from doesn't conflict with our inherent traits,
        // i.e. no bearded females.

        for (int ind = 0; ind <= 1; ind++)
        {
            while (1)
            {
                found = false;
                x = number (0, NUM_MUTATION);


                // Once we've got a number, we need to check it against all other categories previously
                // pulled to avoid duplications. If we find it, we repeat the while loop.

                // If we already have a number, then we assigned x to that first.

                if (feat_ind[ind] != -1)
                    x = feat_ind[ind];

                for (i = 0; i <= ind; i ++)
                    if (feat_cat[i] && !str_cmp(mutation_list[x].category, feat_cat[i]))
                        found = true;

                if (mutation_list[x].sexes != -1 && mob->sex == mutation_list[x].sexes) // If our sex is a proscribed one for this feature, we need to reject it.
                    found = true;
                else if (age_ind <= mutation_list[x].age_more_than) // If our age index is eqaul to or less than what we need to be more than, reject.
                    found = true;
                else if (age_ind >= mutation_list[x].age_less_than) // If our age index is eqaul to or greater than what we need to be less than, reject.
                    found = true;
                else if (mutation_list[x].must_bits != -1 && !IS_SET(mob->act, mutation_list[x].must_bits)) // If we're not set to have a bit we need to, then reject.
                    found = true;
                else if (mutation_list[x].not_bits != -1 && IS_SET(mob->act, mutation_list[x].not_bits)) // If we're set to have a bit we're not allowed to, then reject.
                    found = true;

                // If we've rejected everything, then we need to ditch this ind or get stuck in a perma-loop.
                if (found && feat_ind[ind])
                    feat_ind[ind] = -1;

                if (!found)
                    break;
            }
            feat_cat[ind] = str_dup(mutation_list[x].category);
            feat[ind] = str_dup(mutation_list[x].sdesc);
            feat_full[ind] = str_dup(mutation_list[x].fdesc);

            if (!str_cmp(feat_cat[ind], "eyes"))
            {
                no_haireyes = true;
            }

        }

        // Now we need to decide which two adjectives we want to put in our short and long descs by pulling
        // them from random of the 10 or so variables we've now got.

        x = 0;

        while (x < 2)
        {
            temp_hair = false;
            reject = false;

            if (x == 0)
            {
                if (number(0,1))
                {
                    if (number(0,1))
                    {
                        sprintf(adj1, "%s", feat[0]);
                    }
                    else
                    {
                        sprintf(adj1, "%s", feat[1]);
                    }
                }
                else
                {
                    if (number(0,1))
                    {
                        sprintf(adj2, "%s", feat[0]);
                    }
                    else
                    {
                        sprintf(adj2, "%s", feat[1]);
                    }
                }
                x ++;
                continue;
            }

            // Of course, is we're bald or shaven, we will have a slightly different set of variables to pick from.
            if (no_haireyes == true)
            {
                switch (number(1, 4))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 4:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }

            }
            else if (hair_ind == 0 || hair_ind == 1)
            {
                switch (number(1, 6))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 4:
                    sprintf(temp_adj, "%s", hair_style);
                    break;
                case 5:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 6:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }
            else
            {
                switch (number(1,8))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 4:
                    sprintf(temp_adj, "%s-haired", hair_style);
                    temp_hair = true;
                    break;
                case 5:
                    sprintf(temp_adj, "%s-haired", hair_length);
                    temp_hair = true;
                    break;
                case 6:
                    sprintf(temp_adj, "%s-haired", hair_color);
                    temp_hair = true;
                    break;
                case 7:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 8:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }

                    // If we don't currently have this adjective, and it's not another hair
                    // adjective once we already have our hair adjective, add it on to the list.

        if (str_cmp(temp_adj, adj1) &&
                str_cmp(temp_adj, adj2) &&
                !reject                 &&
                !(temp_hair && true_hair))
        {
            if (*adj1)
                sprintf(adj2, temp_adj);
            else
                sprintf(adj1, temp_adj);

            if (temp_hair)
                true_hair = true;
            x++;
        }


        }
    }
    else if (mob->race == lookup_race_id("Cybernetic"))
    {
        if (rOne && *rOne)
        {
            found = false;
            for (int xind = 0; xind < NUM_CYBERNETIC; xind ++)
            {
                if (!str_cmp(cybernetic_list[xind].sdesc, rOne))
                {
                    found = true;
                    feat[0] = str_dup(cybernetic_list[xind].sdesc);
                    feat_ind[0] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        if (rTwo && *rTwo)
        {
            found = false;
            for (int xind = 0; xind < NUM_CYBERNETIC; xind ++)
            {
                if (!str_cmp(cybernetic_list[xind].sdesc, rTwo))
                {
                    found = true;
                    feat[1] = str_dup(cybernetic_list[xind].sdesc);
                    feat_ind[1] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        // Now we need to get two random features from different cateogires,
        // and much sure we only draw from each category once and that we
        // category we draw from doesn't conflict with our inherent traits,
        // i.e. no bearded females.

        for (int ind = 0; ind <= 1; ind++)
        {
            while (1)
            {
                found = false;
                x = number (0, NUM_CYBERNETIC);


                // Once we've got a number, we need to check it against all other categories previously
                // pulled to avoid duplications. If we find it, we repeat the while loop.

                // If we already have a number, then we assigned x to that first.

                if (feat_ind[ind] != -1)
                    x = feat_ind[ind];

                for (i = 0; i <= ind; i ++)
                    if (feat_cat[i] && !str_cmp(cybernetic_list[x].category, feat_cat[i]))
                        found = true;

                if (cybernetic_list[x].sexes != -1 && mob->sex == cybernetic_list[x].sexes) // If our sex is a proscribed one for this feature, we need to reject it.
                    found = true;
                else if (age_ind <= cybernetic_list[x].age_more_than) // If our age index is eqaul to or less than what we need to be more than, reject.
                    found = true;
                else if (age_ind >= cybernetic_list[x].age_less_than) // If our age index is eqaul to or greater than what we need to be less than, reject.
                    found = true;
                else if (cybernetic_list[x].must_bits != -1 && !IS_SET(mob->act, cybernetic_list[x].must_bits)) // If we're not set to have a bit we need to, then reject.
                    found = true;
                else if (cybernetic_list[x].not_bits != -1 && IS_SET(mob->act, cybernetic_list[x].not_bits)) // If we're set to have a bit we're not allowed to, then reject.
                    found = true;

                // If we've rejected everything, then we need to ditch this ind or get stuck in a perma-loop.
                if (found && feat_ind[ind])
                    feat_ind[ind] = -1;

                if (!found)
                    break;
            }
            feat_cat[ind] = str_dup(cybernetic_list[x].category);
            feat[ind] = str_dup(cybernetic_list[x].sdesc);
            feat_full[ind] = str_dup(cybernetic_list[x].fdesc);

            if (!str_cmp(feat_cat[ind], "eyes") || !str_cmp(feat_cat[ind], "face"))
            {
                no_haireyes = true;
            }

        }

        // Now we need to decide which two adjectives we want to put in our short and long descs by pulling
        // them from random of the 10 or so variables we've now got.

        x = 0;

        while (x < 2)
        {
            temp_hair = false;
            reject = false;

            if (x == 0)
            {
                if (number(0,1))
                {
                    if (number(0,1))
                    {
                        sprintf(adj1, "%s", feat[0]);
                    }
                    else
                    {
                        sprintf(adj1, "%s", feat[1]);
                    }
                }
                else
                {
                    if (number(0,1))
                    {
                        sprintf(adj2, "%s", feat[0]);
                    }
                    else
                    {
                        sprintf(adj2, "%s", feat[1]);
                    }
                }
                x ++;
                continue;
            }

            // Of course, is we're bald or shaven, we will have a slightly different set of variables to pick from.
            if (no_haireyes == true)
            {
                switch (number(1, 4))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 4:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }

            }
            else if (hair_ind == 0 || hair_ind == 1)
            {
                switch (number(1, 6))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 4:
                    sprintf(temp_adj, "%s", hair_style);
                    break;
                case 5:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 6:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }
            else
            {
                switch (number(1,8))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 4:
                    sprintf(temp_adj, "%s-haired", hair_style);
                    temp_hair = true;
                    break;
                case 5:
                    sprintf(temp_adj, "%s-haired", hair_length);
                    temp_hair = true;
                    break;
                case 6:
                    sprintf(temp_adj, "%s-haired", hair_color);
                    temp_hair = true;
                    break;
                case 7:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 8:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }


        // If we don't currently have this adjective, and it's not another hair
        // adjective once we already have our hair adjective, add it on to the list.

        if (str_cmp(temp_adj, adj1) &&
                str_cmp(temp_adj, adj2) &&
                !reject                 &&
                !(temp_hair && true_hair))
        {
            if (*adj1)
                sprintf(adj2, temp_adj);
            else
                sprintf(adj1, temp_adj);

            if (temp_hair)
                true_hair = true;

            x++;
        }



        }

    }
    else
    {

        if (rOne && *rOne)
        {
            found = false;
            for (int xind = 0; xind < NUM_FEATURES; xind ++)
            {
                if (!str_cmp(feature_list[xind].sdesc, rOne))
                {
                    found = true;
                    feat[0] = str_dup(feature_list[xind].sdesc);
                    feat_ind[0] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        if (rTwo && *rTwo)
        {
            found = false;
            for (int xind = 0; xind < NUM_FEATURES; xind ++)
            {
                if (!str_cmp(feature_list[xind].sdesc, rTwo))
                {
                    found = true;
                    feat[1] = str_dup(feature_list[xind].sdesc);
                    feat_ind[1] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        if (rThree && *rThree)
        {
            found = false;
            for (int xind = 0; xind < NUM_FEATURES; xind ++)
            {
                if (!str_cmp(feature_list[xind].sdesc, rThree))
                {
                    found = true;
                    feat[2] = str_dup(feature_list[xind].sdesc);
                    feat_ind[2] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        if (rFour && *rFour)
        {
            found = false;
            for (int xind = 0; xind < NUM_FEATURES; xind ++)
            {
                if (!str_cmp(feature_list[xind].sdesc, rFour))
                {
                    found = true;
                    feat[3] = str_dup(feature_list[xind].sdesc);
                    feat_ind[3] = xind;
                    break;
                }

                if (found)
                    break;
            }
        }

        // Now we need to get four random features from different cateogires,
        // and much sure we only draw from each category once and that we
        // category we draw from doesn't conflict with our inherent traits,
        // i.e. no bearded females.

        for (int ind = 0; ind <= 3; ind++)
        {
            while (1)
            {
                found = false;
                x = number (0, NUM_FEATURES);


                // Once we've got a number, we need to check it against all other categories previously
                // pulled to avoid duplications. If we find it, we repeat the while loop.

                // If we already have a number, then we assigned x to that first.

                if (feat_ind[ind] != -1)
                    x = feat_ind[ind];

                for (i = 0; i <= ind; i ++)
                    if (feat_cat[i] && !str_cmp(feature_list[x].category, feat_cat[i]))
                        found = true;

                if (feature_list[x].sexes != -1 && mob->sex == feature_list[x].sexes) // If our sex is a proscribed one for this feature, we need to reject it.
                    found = true;
                else if (age_ind <= feature_list[x].age_more_than) // If our age index is eqaul to or less than what we need to be more than, reject.
                    found = true;
                else if (age_ind >= feature_list[x].age_less_than) // If our age index is eqaul to or greater than what we need to be less than, reject.
                    found = true;
                else if (feature_list[x].must_bits != -1 && !IS_SET(mob->act, feature_list[x].must_bits)) // If we're not set to have a bit we need to, then reject.
                    found = true;
                else if (feature_list[x].not_bits != -1 && IS_SET(mob->act, feature_list[x].not_bits)) // If we're set to have a bit we're not allowed to, then reject.
                    found = true;
                else if (number(0,9) && x>= 64 && x <= 69) // NPCs only have a 1 in 9 chance of having a prosthetic limb.
                    found = true;

                // If we've rejected everything, then we need to ditch this ind or get stuck in a perma-loop.
                if (found && feat_ind[ind])
                    feat_ind[ind] = -1;

                if (!found)
                    break;
            }
            feat_cat[ind] = str_dup(feature_list[x].category);
            feat[ind] = str_dup(feature_list[x].sdesc);
            feat_full[ind] = str_dup(feature_list[x].fdesc);
        }


        // Now we need to decide which two adjectives we want to put in our short and long descs by pulling
        // them from random of the 10 or so variables we've now got.

        x = 0;

        while (!*adj3)
        {
            temp_hair = false;
            reject = false;

            // Of course, is we're bald or shaven, we will have a slightly different set of variables to pick from.
            if (hair_ind == 0 || hair_ind == 1)
            {
                switch (number(1,10))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 4:
                case 9:
                case 10:
                    sprintf(temp_adj, "%s", hair_style);
                    break;
                case 5:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 6:
                    sprintf(temp_adj, "%s", feat[2]);
                    break;
                case 7:
                    sprintf(temp_adj, "%s", feat[3]);
                    break;
                case 8:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }
            else
            {
                switch (number(1,10))
                {
                case 1:
                    sprintf(temp_adj, "%s", height);
                    if (avg_height)
                        reject = true;
                    break;
                case 2:
                    sprintf(temp_adj, "%s", frame);
                    if (avg_weight)
                        reject = true;
                    break;
                case 3:
                    sprintf(temp_adj, "%s-eyed", eyes);
                    break;
                case 9:
                    sprintf(temp_adj, "%s-haired", hair_style);
                    temp_hair = true;
                    break;
                case 10:
                    sprintf(temp_adj, "%s-haired", hair_length);
                    temp_hair = true;
                    break;
                case 4:
                    sprintf(temp_adj, "%s-haired", hair_color);
                    temp_hair = true;
                    break;
                case 5:
                    sprintf(temp_adj, "%s", feat[1]);
                    break;
                case 6:
                    sprintf(temp_adj, "%s", feat[2]);
                    break;
                case 7:
                    sprintf(temp_adj, "%s", feat[3]);
                    break;
                case 8:
                    sprintf(temp_adj, "%s", feat[0]);
                    break;
                }
            }

        // If we don't currently have this adjective, and it's not another hair
        // adjective once we already have our hair adjective, add it on to the list.

        if (str_cmp(temp_adj, adj1) &&
                str_cmp(temp_adj, adj2) &&
                str_cmp(temp_adj, adj3) &&
                !reject                 &&
                !(temp_hair && true_hair))
        {
            if (x == 2)
                sprintf(adj3, temp_adj);
            else if (x == 1)
                sprintf(adj2, temp_adj);
            else
                sprintf(adj1, temp_adj);

            if (temp_hair)
                true_hair = true;

            x++;

        }
    }
    }

    // So, now we've got our template sdesc.
    sprintf(sdesc, "%s, %s %s", adj1, adj2, age);

    // Now we just make a quick run through to fix things up for "a/an".

    *temp_sentence = '\0';
    for (size_t i = 0; i <= strlen (sdesc); i++)
    {
        if (!*temp_sentence && (adj1[0] == 'a' || adj1[0] == 'e' || adj1[0] == 'i' || adj1[0] == 'o' || adj1[0] == 'u'))
            sprintf (temp_sentence + strlen (temp_sentence), "an ");
        else if (!*temp_sentence)
            sprintf (temp_sentence + strlen (temp_sentence), "a ");

        sprintf (temp_sentence + strlen (temp_sentence), "%c", sdesc[i]);
    }
    *sdesc = '\0';
    sprintf(sdesc, "%s", temp_sentence);

    // Now that we've got age, height, frame, hair and features, we need to build our four sentences.
    *buf4 = '\0';
    sprintf(buf4, "This %s is %s in height and of %s build.", age, height, frame);
    sentence[0] = str_dup(buf4);

    *buf4 = '\0';

    if (no_haireyes == false)
    {
    // If their eyes are of mixed colours, we should mentioned that.
    if (mixed)
        sprintf(buf4, "$e has %s eyes, one of each colour, ",eyes);
    else
        sprintf(buf4, "$e has %s eyes,", eyes);

    // If they're bald we don't need to describe colour and so on.
    if (hair_ind == 0)
        sprintf(buf4 + strlen(buf4), " and $s head is %s.", hair_length);
    else if (hair_ind == 1)
        sprintf(buf4 + strlen(buf4), " and %s, %s hair.", hair_color, hair_style);
    else
        sprintf(buf4 + strlen(buf4), " and %s, %s, %s hair.", hair_length, hair_color, hair_style);
    }

    sentence[1] = str_dup(buf4);


    *buf4 = '\0';
    sprintf(buf4, "%s and %s.", feat_full[0], feat_full[1]);
    sentence[2] = str_dup(buf4);
    *buf4 = '\0';

    if (mob->race != lookup_race_id("Mutation") && mob->race != lookup_race_id("Cybernetic"))
    {
      sprintf(buf4, "%s and %s.", feat_full[2], feat_full[3]);
    }
    sentence[3] = str_dup(buf4);

    // We need to go through each sentence and replace out any instances of $e, $s and $z in there, as well as add the correct capitalisation.

    for (int ind = 0; ind <= 3; ind++)
    {
        *temp_sentence = '\0';
        *buf4 = '\0';
        sprintf(buf4, "%s", sentence[ind]);

        for (size_t i = 0; i <= strlen (buf4); i++)
        {
            if (buf4[i] == '$')
            {
                j = i;
                *buf = '\0';

                while (buf4[i] && buf4[i] != ' ' && buf4[i] != ',')
                {
                    sprintf (buf + strlen (buf), "%c", buf4[i]);
                    i++;
                }

                i = j;

                if (!str_cmp (buf, "$e"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", HSSH(mob));
                else if (!str_cmp (buf, "$s"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", HSHR(mob));
                else if (!str_cmp (buf, "$m"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", HMHR(mob));
                else if (!str_cmp (buf, "$z"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", hair_color);
                i += strlen (buf) - 1;
                continue;
            }
            else
                sprintf (temp_sentence + strlen (temp_sentence), "%c", buf4[i]);
        }
        *temp_sentence = toupper(*temp_sentence);
        sentence[ind] = str_dup(temp_sentence);
    }

    // If we've got an existing description, then we'll go and grab the 
	// description from our prototype, and use it as the base for our
	// new description.

    if (mob->description)
    {
		if (mob->mob)
		{
			sprintf(buf2, "%s", vnum_to_mob(mob->mob->vnum)->description);
		}
		else
		{
			*buf2 = '\0';
		}
        mem_free (mob->description);

        *temp_sentence = '\0';

        for (size_t i = 0; i <= strlen (buf2); i++)
        {
            if (buf2[i] == '$')
            {
                j = i;
                *buf = '\0';

                while (buf2[i] && buf2[i] != ' ' && buf2[i] != ',')
                {
                    sprintf (buf + strlen (buf), "%c", buf2[i]);
                    i++;
                }

                i = j;

                if (!str_cmp (buf, "$e"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", HSSH(mob));
                else if (!str_cmp (buf, "$s"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", HSHR(mob));
                else if (!str_cmp (buf, "$z"))
                    sprintf (temp_sentence + strlen (temp_sentence), "%s", hair_color);
                i += strlen (buf) - 1;
                continue;
            }
            else if (buf2[i] == '\n')
                sprintf (temp_sentence + strlen (temp_sentence), " ");
            else
                sprintf (temp_sentence + strlen (temp_sentence), "%c", buf2[i]);
        }
        *temp_sentence = toupper(*temp_sentence);
        sprintf(buf2, "%s", temp_sentence);
    }


    // Yay! We've got all our descs, so now we apply them to the mobiles.

    mob->delay_info1 = 0;

    if (mob->short_descr)
        mem_free (mob->short_descr);

    mob->short_descr = add_hash (sdesc);

    *buf = '\0';
    if (IS_SET (mob->act, ACT_ENFORCER) && IS_SET (mob->act, ACT_SENTINEL))
    {
        roll = number (1, 3);
        if (roll == 1)
            sprintf (buf, "%s stands at attention here.", sdesc);
        else if (roll == 2)
            sprintf (buf, "%s stands here, watching for signs of trouble.",
                     sdesc);
        else if (roll == 3)
            sprintf (buf, "%s patrols here, looking hawkishly about.", sdesc);
        mob->travel_str = add_hash ("looking hawkishly about");
    }
    else if (IS_SET (mob->act, ACT_ENFORCER)
             && !IS_SET (mob->act, ACT_SENTINEL))
    {
        roll = number (1, 3);
        if (roll == 1)
        {
            sprintf (buf, "%s patrols here, looking for signs of trouble.",
                     sdesc);
            mob->travel_str = add_hash ("looking about purposefully");
        }
        else if (roll == 2)
        {
            sprintf (buf, "%s moves by, watching the area attentively.", sdesc);
            mob->travel_str = add_hash ("looking about the area attentively");
        }
        else if (roll == 3)
        {
            sprintf (buf, "%s strides through, watching intently.", sdesc);
            mob->travel_str = add_hash ("looking hawkishly about");
        }
    }

    if (!*buf)
        sprintf (buf, "%s is here.", sdesc);

    *buf = toupper(*buf);

    if (mob->long_descr)
        mem_free (mob->long_descr);
    mob->long_descr = add_hash (buf);


    if (IS_NPC(mob))
    {
        if (mob->name)
            mem_free (mob->name);

        sprintf(buf3, "x%d%d%d%d%d-%s", number(1,9), number(0,9), number(0,9),number(0,9),number(0,9),mob->tname);

        if (mob->tname)
            mem_free (mob->tname);

        mob->tname = add_hash(buf3);

        sprintf(buf3 + strlen(buf3), " %s %s %s", adj1, adj2, age);

        mob->name = add_hash (buf3);
    }

    *buf3 = '\0';
    sprintf (buf3, "%s %s %s %s %s", buf2, sentence[0], sentence[1], sentence[2], sentence[3]);

    mob->description = add_hash(buf3);

    reformat_desc(mob->description, &mob->description);


    // Now we write to the mobile.
    if (mob->d_age)
        mem_free (mob->d_age);
    mob->d_age = str_dup(age);

    if (mob->d_eyes)
        mem_free (mob->d_eyes);
    mob->d_eyes = str_dup(eyes);

    if (mob->d_hairlength)
        mem_free (mob->d_hairlength);
    mob->d_hairlength = str_dup(hair_length);

    if (mob->d_haircolor)
        mem_free (mob->d_haircolor);
    mob->d_haircolor = str_dup(hair_color);

    if (mob->d_hairstyle)
        mem_free (mob->d_hairstyle);
    mob->d_hairstyle = str_dup(hair_style);

    if (mob->d_height)
        mem_free (mob->d_height);
    mob->d_height = str_dup(height);

    if (mob->d_frame)
        mem_free (mob->d_frame);
    mob->d_frame = str_dup(frame);

    if (mob->d_feat1)
        mem_free (mob->d_feat1);
    mob->d_feat1 = str_dup(feat[0]);

    if (mob->d_feat2)
        mem_free (mob->d_feat2);
    mob->d_feat2 = str_dup(feat[1]);

    if (mob->d_feat3)
        mem_free (mob->d_feat3);
    mob->d_feat3 = str_dup(feat[2]);

    if (mob->d_feat4)
        mem_free (mob->d_feat4);
    mob->d_feat4 = str_dup(feat[3]);
}


/**
 *  type 0 is normal racial defaults
 *  type 1 will be slightly better stats +10%
 *  type 2 will be elite with even better stats +50%
 **/
void
new_randomize_mobile (CHAR_DATA * mob,
                      char *rAge, char *rHeight, char *rFrame, char *rEyes, char *rLength, char *rColor, char *rStyle, char *rOne, char *rTwo, char *rThree, char *rFour)
{
    CHAR_DATA *proto;
    int attr_starters[] = { 16, 15, 12, 12, 11, 10, 8 };
    int attr_priorities[] = { -1, -1, -1, -1, -1, -1, 1 };
    int slots_taken[] = { 0, 0, 0, 0, 0, 0, 0 };
    int i, roll, bonus;
    int type_bonus = 0;

    if (is_name_in_list("elite", mob->name))
        type_bonus = 30;
    else if (is_name_in_list("regular", mob->name))
        type_bonus = 10;
    else if (is_name_in_list("veteran", mob->name))
        type_bonus = 20;

    // IF we're of a certain race,

    if (lookup_race_int(mob->race, RACE_PC))
    {
        for (i = 0; i <= 6; i++)
        {
            roll = number (0, 6);
            if (slots_taken[roll])
            {
                i--;
                continue;
            }

            slots_taken[roll] = 1;
            attr_priorities[i] = roll;
        }

        for (bonus = 8; bonus;)
        {
            roll = number (0, 6);
            if (attr_starters[attr_priorities[roll]] < 18)
            {
                attr_starters[attr_priorities[roll]]++;
                bonus--;
            }
        }

        mob->str = attr_starters[attr_priorities[0]];
        mob->dex = attr_starters[attr_priorities[1]];
        mob->con = attr_starters[attr_priorities[2]];
        mob->wil = attr_starters[attr_priorities[3]];
        mob->intel = attr_starters[attr_priorities[4]];
        mob->aur = attr_starters[attr_priorities[5]];
        mob->agi = attr_starters[attr_priorities[6]];

        mob->tmp_str = mob->str;
        mob->tmp_dex = mob->dex;
        mob->tmp_intel = mob->intel;
        mob->tmp_aur = mob->aur;
        mob->tmp_agi = mob->agi;
        mob->tmp_con = mob->con;
        mob->tmp_wil = mob->wil;

        for (i = 1; i <= LAST_SKILL; i++)
            mob->skills[i] = 0;

        if (IS_SET (mob->act, ACT_ENFORCER))
            mob->skills[SKILL_HANDLE] = 10 + number (5, 15) + type_bonus;

        make_height (mob);
        make_frame (mob);

        for (i = 1; i <= LAST_WEAPON_SKILL; i++) //weapon skills
            mob->skills[i] = number (20, 30) + type_bonus;

        mob->skills[SKILL_DEFLECT] = number (20, 30) + type_bonus;
        mob->skills[SKILL_DODGE] = number (20, 30) + type_bonus;

        if (is_name_in_list("sneaky", mob->name))
        {
            mob->skills[SKILL_SNEAK] = number (20, 30) + type_bonus;
            mob->skills[SKILL_HIDE] = number (20, 30) + type_bonus;
        }

        for (i = 1; i <= LAST_SKILL; i++)
        {
            if (mob->skills[i] > calc_lookup (mob, REG_CAP, i))
                mob->skills[i] = calc_lookup (mob, REG_CAP, i);
            if (mob->skills[i] < 0)
                mob->skills[i] = number (1, 10);
        }

        if (mob->mob)
            proto = vnum_to_mob (mob->mob->vnum);
        else
            proto = vnum_to_mob (998);

        if (lookup_race_int (mob->race, RACE_NATIVE_TONGUE))
        {
            mob->speaks = SKILL_COMMON;
            mob->skills[SKILL_COMMON] = 50;
            //  atoi (lookup_race_variable (mob->race, RACE_NATIVE_TONGUE));
            //  mob->skills[mob->speaks] = calc_lookup (mob, REG_CAP, mob->speaks);
        }

        for (i = 1; i <= LAST_SKILL; i++)
        {
            proto->skills[i] = mob->skills[i];
        }

        proto->speaks = mob->speaks;

        if (mob->pc)
        {
            for (i = 1; i <= LAST_SKILL; i++)
            {
                mob->pc->skills[i] = mob->skills[i];
            }
        }

        fix_offense (mob);
        fix_offense (proto);
    }  //if (mob->race >= 0 && mob->race <= 11)
    else
    {
        make_height (mob);
        make_frame (mob);
    }

    mob->sex = number (1, 2);
    if (IS_SET (mob->act, ACT_ENFORCER))
    {
        roll = number (1, 10);
        if (roll == 10)
            mob->sex = SEX_FEMALE;
        else
            mob->sex = SEX_MALE;
    }

    mob->max_move = calc_lookup (mob, REG_MISC, MISC_MAX_MOVE);
    mob->move_points = mob->max_move;

    new_create_description (mob, rAge, rHeight, rFrame, rEyes, rLength, rColor, rStyle, rOne, rTwo, rThree, rFour);

    if (IS_SET (mob->flags, FLAG_VARIABLE) || mob->pc)
    {
        switch (number (1, 5))
        {
        case 1:
            mob->speed = SPEED_CRAWL;
            break;
        case 2:
            mob->speed = SPEED_PACED;
            break;
        default:
            mob->speed = SPEED_WALK;
            break;
        }
    }

    switch (number(1,3))
    {
    case 1:
        mob->fight_mode = 1;
        break;
    case 2:
        mob->fight_mode = 2;
        break;
    case 3:
        mob->fight_mode = 3;
        break;
    }
}
