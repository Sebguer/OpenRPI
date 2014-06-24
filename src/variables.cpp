/*------------------------------------------------------------------------\
|  staff.c : Variables Control Module                www.atonementrpi.com |
|  Copyright (C) 2010, Atonement RPI, Kithrater                           |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <limits.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "account.h"
#include "utils.h"
#include "decl.h"
#include "sys/stat.h"
#include "clan.h"		/* clan__assert_objs() */
#include "utility.h"

extern std::multimap<int, variable_data> obj_variable_list;
extern std::multimap<int, std::string> variable_categories;

extern std::multimap<int, mvariable_data> mvariable_list;
extern std::multimap<int, std::string> mvariable_categories;


int
vd_data (char *shorts, char *category, int cmd)
{
    bool fail = false;

    if (!shorts || !*shorts)
        fail = true;

    if (!category || !*category)
        fail = true;

    if (!fail)
    {
        for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        {
            if (!str_cmp(it->second.shorts, shorts))
            {
                if (!str_cmp(it->second.category, category))
                {
                    switch (cmd)
                    {
                    case 2: // cost_mod
                        return it->second.cost_mod;
                        break;
                    case 1: // weight_mod
                        return it->second.weight_mod;
                        break;
                    case 3: // quality_mod
                        return it->second.quality_mod;
                        break;
                    case 4: // item type
                        return it->second.item_type;
                        break;
                    case 5: // skill name
                        return it->second.skill_name;
                        break;
                    case 6: // skill mod
                        return it->second.skill_mod;
                        break;
                    case 7: // oval0
                        return it->second.oval0;
                        break;
                    case 8: // oval1
                        return it->second.oval1;
                        break;
                    case 9: // oval2
                        return it->second.oval2;
                        break;
                    case 10: // oval3
                        return it->second.oval3;
                        break;
                    case 11: // oval4
                        return it->second.oval4;
                        break;
                    case 12: // oval5
                        return it->second.oval5;
                        break;
                    default:
                        return 0;
                        break;
                    }
                }
            }
        }
    }

    switch (cmd)
    {
    case 2: // cost_mod
        return 100;
        break;
    case 1: // weight_mod
        return 100;
        break;
    case 3: // quality_mod
        return 100;
        break;
    case 4: // item type
        return 0;
        break;
    case 5: // skill name
        return 0;
        break;
    case 6: // skill mod
        return 0;
        break;
    case 7: // oval0
        return 0;
        break;
    case 8: // oval1
        return 0;
        break;
    case 9: // oval2
        return 0;
        break;
    case 10: // oval3
        return 0;
        break;
    case 11: // oval4
        return 0;
        break;
    case 12: // oval5
        return 0;
        break;
    default: // anything else
        return 0;
        break;
    }
}



int
vd_data (char *shorts, int i, int cmd)
{
    bool fail = false;

    if (!shorts || !*shorts)
        fail = true;

    if (i <= 0)
        fail = true;

    if (!fail)
    {
        for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        {
            if (!str_cmp(it->second.shorts, shorts))
            {
                if (it->second.category_id == i)
                {
                    switch (cmd)
                    {
                    case 2: // cost_mod
                        return it->second.cost_mod;
                        break;
                    case 1: // weight_mod
                        return it->second.weight_mod;
                        break;
                    case 3: // quality_mod
                        return it->second.quality_mod;
                        break;
                    case 4: // item type
                        return it->second.item_type;
                        break;
                    case 5: // skill name
                        return it->second.skill_name;
                        break;
                    case 6: // skill mod
                        return it->second.skill_mod;
                        break;
                    case 7: // oval0
                        return it->second.oval0;
                        break;
                    case 8: // oval1
                        return it->second.oval1;
                        break;
                    case 9: // oval2
                        return it->second.oval2;
                        break;
                    case 10: // oval3
                        return it->second.oval3;
                        break;
                    case 11: // oval4
                        return it->second.oval4;
                        break;
                    case 12: // oval5
                        return it->second.oval5;
                        break;
                    default:
                        return 0;
                        break;
                    }
                }
            }
        }
    }

    switch (cmd)
    {
    case 2: // cost_mod
        return 100;
        break;
    case 1: // weight_mod
        return 100;
        break;
    case 3: // quality_mod
        return 100;
        break;
    case 4: // item type
        return 0;
        break;
    case 5: // skill name
        return 0;
        break;
    case 6: // skill mod
        return 0;
        break;
    case 7: // oval0
        return 0;
        break;
    case 8: // oval1
        return 0;
        break;
    case 9: // oval2
        return 0;
        break;
    case 10: // oval3
        return 0;
        break;
    case 11: // oval4
        return 0;
        break;
    case 12: // oval5
        return 0;
        break;
    default: // anything else
        return 0;
        break;
    }
}


// Renumbers items in a list once things are deleted.
void
vc_renumber(char *category)
{
    int i = 1;

    if (!category || !*category)
        return;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (str_cmp(it->second.category, category))
            continue;

        it->second.id = i;

        i++;
    }
}

// Renumbers items in a list once things are deleted.
void
vc_renumber(int j)
{
    int i = 1;

    if (j <= 0)
        return;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (it->second.category_id != j)
            continue;

        it->second.id = i;

        i++;
    }
}

// Renumbers items in a list once things are deleted.
void
mob_vc_renumber(char *category)
{
    int i = 1;

    if (!category || !*category)
        return;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
    {
        if (str_cmp(it->second.category, category))
            continue;

        it->second.id = i;

        i++;
    }
}

// Renumbers items in a list once things are deleted.
void
mob_vc_renumber(int j)
{
    int i = 1;

    if (j <= 0)
        return;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
    {
        if (it->second.category_id != j)
            continue;

        it->second.id = i;

        i++;
    }
}

// How many variable categories do we currently have?
int
mob_vc_count()
{
    int j = 0;

    for (std::multimap<int, std::string>::iterator it = mvariable_categories.begin(); it != mvariable_categories.end(); it++)
        j++;

    return j;

}

// How many variable categories do we currently have?
int
vc_count()
{
    int j = 0;

    for (std::multimap<int, std::string>::iterator it = variable_categories.begin(); it != variable_categories.end(); it++)
        j++;

    return j;

}

// How many variables does this category have in it already?
int
mob_vc_size(int i)
{
    int j = 0;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
        if (it->second.category_id == i)
            j++;

    return j;
}

// How many variables does this category have in it already?
int
vc_size(int i)
{
    int j = 0;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (it->second.category_id == i)
            j++;

    return j;
}

// How many variables does this category have in it already?
int
vc_rand_size(int i)
{
    int j = 0;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (it->second.category_id == i && !it->second.random)
            j++;

    return j;
}

// returns a random short from the categories [and other info, hooray]

char *
mob_vc_rand(char *category, int *cat)
{
    int i = 0;
    int j = 0;
    int x = 1;

    if (!category || !*category)
        return NULL;

    if (!(i = mob_vc_category(category)))
        return NULL;

    if (!(j = mob_vc_size(i)))
        return NULL;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
    {
        if (it->second.category_id != i)
            continue;

        if (!(number(0, (j - x))))
        {
            *cat = it->second.category_id;
            return it->second.shorts;
        }

        x++;
    }

    return NULL;
}


// returns a random short from the categories [and other info, hooray]

char *
mob_vc_rand(int i, int *cat, char * full)
{
    int j = 0;
    int x = 1;

    if (!mob_vc_category(i))
    {
        return NULL;
    }

    if (!(j = mob_vc_size(i)))
    {
        return NULL;
    }

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
    {
        if (it->second.category_id != i)
        {
            continue;
        }

        if (!(number(0, (j - x))))
        {
            full = str_dup(it->second.full);
            *cat = it->second.category_id;
            return it->second.shorts;
        }

        x++;
    }

    return NULL;
}



// returns a random short from the categories [and other info, hooray]

char *
vc_rand(char *category, int *cat)
{
    int i = 0;
    int j = 0;
    int x = 1;

    if (!category || !*category)
        return NULL;

    if (!(i = vc_category(category)))
        return NULL;

    if (!(j = vc_rand_size(i)))
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (it->second.category_id != i)
            continue;

        if (it->second.random)
            continue;

        if (!(number(0, (j - x))))
        {
            *cat = it->second.category_id;
            return it->second.shorts;
        }

        x++;
    }

    return NULL;
}

char *
vc_rand(char *category)
{
    int i = 0;
    int j = 0;
    int x = 1;

    if (!category || !*category)
        return NULL;

    if (!(i = vc_category(category)))
        return NULL;

    if (!(j = vc_rand_size(i)))
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (str_cmp(it->second.category, category))
            continue;

        if (it->second.random)
            continue;

        if (!(number(0, (j - x))))
        {
            return it->second.shorts;
        }

        x++;
    }

    return NULL;
}

// Is this a random variable?
bool
vc_checkrand(char *shorts, char *category)
{
    if (!category || !*category)
        return NULL;

    if (!shorts || !*shorts)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (str_cmp(it->second.category, category))
            continue;

        if (!str_cmp(it->second.shorts, shorts))
        {
			// a positive integer means it's not random
			if (it->second.random)
				return true;
			else
				return false;
        }
    }

    return false;
}

// Is this category seeable?

int vc_seeable(char *category)
{
    if (!category || !*category)
        return 0;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (str_cmp(it->second.category, category))
            continue;

        if (it->second.category_seeable != 0)
        {
           return 1;
        }
        else
        {
           return 0;
        }
    }

    return 0;
}


// returns a random short from the categories [and other info, hooray]

char *
vc_rand(int i, int *cat, char * full, int *weight_mod, int *cost_mod)
{
    int j = 0;
    int x = 1;

    if (!vc_category(i))
    {
        return NULL;
    }

    if (!(j = vc_size(i)))
    {
        return NULL;
    }

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (it->second.category_id != i)
        {
            continue;
        }

        if (!(number(0, (j - x))))
        {
            *weight_mod = it->second.weight_mod;
            *cost_mod = it->second.cost_mod;
            full = str_dup(it->second.full);
            *cat = it->second.category_id;
            return it->second.shorts;
        }

        x++;
    }

    return NULL;
}

// If this variable is -anywhere- on the list, then return the list id.
int
vd_variable (char *shorts)
{
    if (!shorts || !*shorts)
        return 0;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
    {
        if (!str_cmp(it->second.shorts, shorts))
            return it->second.category_id;
    }
    return 0;

}

// If this variable is -anywhere- on the list, then return the list id.
int
mob_vd_variable (char *shorts)
{
    if (!shorts || !*shorts)
        return 0;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
    {
        if (!str_cmp(it->second.shorts, shorts))
            return it->second.category_id;
    }
    return 0;

}

// Is there a variable by this short description in this category?
int
mob_vc_exists (char *shorts, char *category)
{

    if (!shorts || !*shorts)
        return 0;

    if (!category || !*category)
        return 0;

    // Just search through the whole list: if we get a match for category and shorts, return the id;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.shorts, shorts))
                return it->second.id;

    return 0;
}

// Is there a variable by this short description in this category?
int
mob_vc_exists (char *shorts, int i)
{

    if (!shorts || !*shorts)
        return 0;

    if (i <= 0)
        return 0;

    // Just search through the whole list: if we get a match for category and shorts, return the id;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
        if (it->second.category_id == i)
            if (!str_cmp(it->second.shorts, shorts))
                return it->second.id;

    return 0;
}

// Is there a variable by this short description in this category?
int
vc_exists (char *shorts, char *category)
{

    if (!shorts || !*shorts)
        return 0;

    if (!category || !*category)
        return 0;

    // Just search through the whole list: if we get a match for category and shorts, return the id;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.shorts, shorts))
                return it->second.id;

    return 0;
}

// Is there a variable by this short description in this category?
int
vc_exists (char *shorts, int i)
{

    if (!shorts || !*shorts)
        return 0;

    if (i <= 0)
        return 0;

    // Just search through the whole list: if we get a match for category and shorts, return the id;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (it->second.category_id == i)
            if (!str_cmp(it->second.shorts, shorts))
                return it->second.id;

    return 0;
}

// Provide the category and the id, return the short.

char*
vd_short (char *category, int i)
{
    if (!category || !*category)
        return NULL;

    if (i <= 0)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (it->second.id == i)
                return (char *) it->second.shorts;

    return NULL;
}

// Provide the category and the short, return the id.

int
vd_id (char *category, char *shorts)
{
    if (!category || !*category)
        return 0;

    if (!shorts || !*shorts)
        return 0;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.category, shorts))
                return it->second.id;

    return 0;
}

// Provide the category and the id, return the full.

char*
vd_flavour (char *category, int i)
{
    if (!category || !*category)
        return NULL;

    if (i <= 0)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (it->second.id == i)
			  if (it->second.flavour)
				return (char *) it->second.flavour;

    return NULL;
}

// Provide the category and the short, return the full.

char*
vd_flavour (char *category, char *shorts)
{
    if (!category || !*category)
        return NULL;

    if (!shorts || !*shorts)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.shorts, shorts))
			  if (it->second.flavour)
				return (char *) it->second.flavour;

    return NULL;
}

// Provide the category and the id, return the full.

char*
vd_full (char *category, int i)
{
    if (!category || !*category)
        return NULL;

    if (i <= 0)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (it->second.id == i)
                return (char *) it->second.full;

    return NULL;
}

// Provide the category and the short, return the full.

char*
vd_full (char *category, char *shorts)
{
    if (!category || !*category)
        return NULL;

    if (!shorts || !*shorts)
        return NULL;

    for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.shorts, shorts))
                return (char *) it->second.full;

    return NULL;
}

char*
mob_vd_full (char *category, int i)
{
    if (!category || !*category)
        return NULL;

    if (i <= 0)
        return NULL;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (it->second.id == i)
                return (char *) it->second.full;

    return NULL;
}

// Provide the category and the short, return the full.

char*
mob_vd_full (char *category, char *shorts)
{
    if (!category || !*category)
        return NULL;

    if (!shorts || !*shorts)
        return NULL;

    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
        if (!str_cmp(it->second.category, category))
            if (!str_cmp(it->second.shorts, shorts))
                return (char *) it->second.full;

    return NULL;
}

// Is this argument a category? If so, return which number it is.
int
mob_vc_category (char *argument)
{
    for (std::multimap<int, std::string>::iterator it = mvariable_categories.begin(); it != mvariable_categories.end(); it++)
    {
        if (!str_cmp(argument, it->second.c_str()))
            return it->first;
    }
    return 0;
}

// Does this number correlate to a category? If so, return which category.
char*
mob_vc_category (int i)
{
    for (std::multimap<int, std::string>::iterator it = mvariable_categories.begin(); it != mvariable_categories.end(); it++)
    {
        if (i == it->first)
            return (char *) it->second.c_str();
    }
    return NULL;
}

// Is this argument a category? If so, return which number it is.
int
vc_category (char *argument)
{
	if (!argument)
		return 0;

    for (std::multimap<int, std::string>::iterator it = variable_categories.begin(); it != variable_categories.end(); it++)
    {
        if (!str_cmp(argument, it->second.c_str()))
            return it->first;
    }
    return 0;
}

// Does this number correlate to a category? If so, return which category.
char*
vc_category (int i)
{
    for (std::multimap<int, std::string>::iterator it = variable_categories.begin(); it != variable_categories.end(); it++)
    {
        if (i == it->first)
            return (char *) it->second.c_str();
    }
    return NULL;
}


// Applies the particular modifications to an object depending on its variable strings.
void
vo_weight_cost_quality (OBJ_DATA *obj)
{
    AFFECTED_TYPE *af = NULL;

    int xcat[10];

    for (int ind = 0; ind <= 9; ind++)
	{
        xcat[ind] = vc_category(obj->var_cat[ind]);
	}

    for (int ind = 0; ind <= 9; ind++)
    {
        if (xcat[ind] && obj->var_color[ind] && *obj->var_color[ind])
        {
            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
            {
                if (!str_cmp(it->second.shorts, obj->var_color[ind]))
                {
                    if (it->second.category_id == xcat[ind])
                    {
                        // HACK: if we've got a gun, we only want a few variables to impact this.

                        if (GET_ITEM_TYPE(obj) != ITEM_FIREARM || !str_cmp(it->second.category, "$ironalloy")
															   || !str_cmp(it->second.category, "$pistolframe")
															   || !str_cmp(it->second.category, "$pistolbarrel")
															   || !str_cmp(it->second.category, "$pistolcaliber")
															   || !str_cmp(it->second.category, "$rifleframe")
															   || !str_cmp(it->second.category, "$riflebarrel")
															   || !str_cmp(it->second.category, "$riflecaliber")
															   || !str_cmp(it->second.category, "$smgframe")
															   || !str_cmp(it->second.category, "$smgbarrel")
															   || !str_cmp(it->second.category, "$smgcaliber"))
                        {
                            // Do the weight.
                            obj->obj_flags.weight = obj->obj_flags.weight * it->second.weight_mod / 100;

                            // Do the quality.
                            obj->quality = obj->quality * it->second.quality_mod / 100;

                            // Do the cost
                            obj->farthings = obj->farthings * it->second.cost_mod / 100;
                        }
                    }
                }
            }
        }
    }

    // Need some sanity checks here - nothing can ever by +- more than 30% its quality, weight or cost
    // if it's an armour, weapon, or firearm, to prevent things getting ridiculous via quality stacking.
    if (GET_ITEM_TYPE(obj) == ITEM_WEAPON ||
        GET_ITEM_TYPE(obj) == ITEM_FIREARM ||
        GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    {
            if (obj->quality > (vtoo(obj->nVirtual)->quality * 130 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 130 / 100;

            if (obj->farthings > (vtoo(obj->nVirtual)->farthings * 130 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 130 / 100;

            if (obj->obj_flags.weight > (vtoo(obj->nVirtual)->obj_flags.weight * 130 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 130 / 100;

            if (obj->quality < (vtoo(obj->nVirtual)->quality * 70 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 70 / 100;

            if (obj->farthings < (vtoo(obj->nVirtual)->farthings * 70 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 70 / 100;

            if (obj->obj_flags.weight < (vtoo(obj->nVirtual)->obj_flags.weight * 70 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 70 / 100;
    }
    else if (GET_ITEM_TYPE(obj) == ITEM_WORN && obj->o.od.value[2] != 0)
    {
            if (obj->quality > (vtoo(obj->nVirtual)->quality * 200 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 200 / 100;

            if (obj->farthings > (vtoo(obj->nVirtual)->farthings * 200 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 200 / 100;

            if (obj->obj_flags.weight > (vtoo(obj->nVirtual)->obj_flags.weight * 200 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 200 / 100;

            if (obj->quality < (vtoo(obj->nVirtual)->quality * 50 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 50 / 100;

            if (obj->farthings < (vtoo(obj->nVirtual)->farthings * 50 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 50 / 100;

            if (obj->obj_flags.weight < (vtoo(obj->nVirtual)->obj_flags.weight * 50 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 50 / 100;
    }

	// If we now have 0 weight, we get at least 1.
	if (obj->obj_flags.weight == 0 && vtoo(obj->nVirtual)->obj_flags.weight != 0)
		obj->obj_flags.weight = 1;
}

// Applies the particular modifications to an object depending on its variable strings.
void
vo_specify (OBJ_DATA *obj)
{
    AFFECTED_TYPE *af = NULL;

    int xcat[10];

    for (int ind = 0; ind <= 9; ind++)
        xcat[ind] = vc_category(obj->var_cat[ind]);

    for (int ind = 0; ind <= 9; ind++)
    {
        if (xcat[ind] && obj->var_color[ind] && *obj->var_color[ind])
        {
            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
            {
                if (!str_cmp(it->second.shorts, obj->var_color[ind]))
                {
                    if (it->second.category_id == xcat[ind])
                    {
                        // HACK: if we've got a gun, we only want a few variables to impact this.

                        if (GET_ITEM_TYPE(obj) != ITEM_FIREARM || !str_cmp(it->second.category, "$ironalloy")
															   || !str_cmp(it->second.category, "$pistolframe")
															   || !str_cmp(it->second.category, "$pistolbarrel")
															   || !str_cmp(it->second.category, "$pistolcaliber")
															   || !str_cmp(it->second.category, "$rifleframe")
															   || !str_cmp(it->second.category, "$riflebarrel")
															   || !str_cmp(it->second.category, "$riflecaliber")
															   || !str_cmp(it->second.category, "$smgframe")
															   || !str_cmp(it->second.category, "$smgbarrel")
															   || !str_cmp(it->second.category, "$smgcaliber"))
                        {
                            // Do the weight.
                            obj->obj_flags.weight = obj->obj_flags.weight * it->second.weight_mod / 100;

                            // Do the quality.
                            obj->quality = obj->quality * it->second.quality_mod / 100;

                            // Do the cost
                            obj->farthings = obj->farthings * it->second.cost_mod / 100;
                        }

                        // Now, if the item type matches, apply the oval modifications and the skill adjustments.
                        if (GET_ITEM_TYPE(obj) == it->second.item_type)
                        {
                            if ((index_lookup(skills, skills[it->second.skill_name]) != -1) && (it->second.skill_mod != 0))
                            {
                                if ((af = get_obj_affect_location (obj, it->second.skill_name + 10000)))
                                {
                                    af->a.spell.modifier += it->second.skill_mod;
                                }
                                else
                                {
                                    CREATE (af, AFFECTED_TYPE, 1);
                                    af->type = 0;
                                    af->a.spell.duration = -1;
                                    af->a.spell.bitvector = 0;
                                    af->a.spell.sn = 0;
                                    af->a.spell.location = it->second.skill_name + 10000;
                                    af->a.spell.modifier = it->second.skill_mod;
                                    af->next = NULL;
                                    affect_to_obj (obj, af);
                                }
                            }

                            obj->o.od.value[0] += it->second.oval0;
                            obj->o.od.value[1] += it->second.oval1;
                            obj->o.od.value[2] += it->second.oval2;
                            obj->o.od.value[3] += it->second.oval3;
                            obj->o.od.value[4] += it->second.oval4;
                            obj->o.od.value[5] += it->second.oval5;
                        }
                    }
                }
            }
        }
    }

    // Need some sanity checks here - nothing can ever by +- more than 30% its quality, weight or cost
    // if it's an armour, weapon, or firearm, to prevent things getting ridiculous via quality stacking.
    if (GET_ITEM_TYPE(obj) == ITEM_WEAPON ||
        GET_ITEM_TYPE(obj) == ITEM_FIREARM ||
        GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    {
            if (obj->quality > (vtoo(obj->nVirtual)->quality * 130 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 130 / 100;

            if (obj->farthings > (vtoo(obj->nVirtual)->farthings * 130 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 130 / 100;

            if (obj->obj_flags.weight > (vtoo(obj->nVirtual)->obj_flags.weight * 130 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 130 / 100;

            if (obj->quality < (vtoo(obj->nVirtual)->quality * 70 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 70 / 100;

            if (obj->farthings < (vtoo(obj->nVirtual)->farthings * 70 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 70 / 100;

            if (obj->obj_flags.weight < (vtoo(obj->nVirtual)->obj_flags.weight * 70 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 70 / 100;
    }
    else if (GET_ITEM_TYPE(obj) == ITEM_WORN && obj->o.od.value[2] != 0)
    {
            if (obj->quality > (vtoo(obj->nVirtual)->quality * 200 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 200 / 100;

            if (obj->farthings > (vtoo(obj->nVirtual)->farthings * 200 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 200 / 100;

            if (obj->obj_flags.weight > (vtoo(obj->nVirtual)->obj_flags.weight * 200 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 200 / 100;

            if (obj->quality < (vtoo(obj->nVirtual)->quality * 50 / 100))
                obj->quality = vtoo(obj->nVirtual)->quality * 50 / 100;

            if (obj->farthings < (vtoo(obj->nVirtual)->farthings * 50 / 100))
                obj->farthings = vtoo(obj->nVirtual)->farthings * 50 / 100;

            if (obj->obj_flags.weight < (vtoo(obj->nVirtual)->obj_flags.weight * 50 / 100))
                obj->obj_flags.weight = vtoo(obj->nVirtual)->obj_flags.weight * 50 / 100;
    }

	// If we now have 0 weight, we get at least 1.
	if (obj->obj_flags.weight == 0 && vtoo(obj->nVirtual)->obj_flags.weight != 0)
		obj->obj_flags.weight = 1;

}

int
vo_match_color (OBJ_DATA *obj, OBJ_DATA *tobj)
{
    int count = 0;

    if (!obj)
        return 0;

    if (!tobj)
        return 0;

    for (int i = 0; i <= 9; i++)
    {
        if (!str_cmp(tobj->var_color[i], obj->var_color[i]))
        {
            count++;
        }
    }

    if (count == 10)
        return 1;
    else
        return 0;
}

void
do_mvariables (CHAR_DATA *ch, char *argument, int cmd)
{
    std::string strArgument = argument, argOne, argTwo, argThree, argFour, argFive, temp, output = "";
    int i = 0;
    int j = 0;
    int k = 0;
    char *lastCategory = '\0';
    strArgument = one_argument(strArgument, argOne);
    bool checked = false;
    bool digit = false;
    int cat = 0;

    if (argOne.empty())
        argOne = "help";

    if (argOne == "list")// Is there a variable by this short description in this category?
    {
        if (strArgument.empty())
        {
            output += "\n#2Available Mobile Variable Lists:#0\n";
            for (std::multimap<int, std::string>::iterator it = mvariable_categories.begin(); it != mvariable_categories.end(); it++)
            {
                k++;
                if (k < 10)
                    output += " ";
                if (k < 100)
                    output += " ";
                output += "#B" + MAKE_STRING(it->first) + ":#0 " + it->second;

                for (i = 0, j = (24 - MIN((int) strlen(it->second.c_str()), 21)); i < j; i++)
                    output.append(" ");

                if (k % 3 == 0)
                    output += "\n";

            }

            output += "\n";

        }
        else if (!mob_vc_category(atoi(strArgument.c_str())) && !mob_vc_category((char*) strArgument.c_str()))
        {
            output += "\nThere is either no such list, or no variables currently in that list.\n";
        }
        else
        {
            output += "\nVariables for list #2" + MAKE_STRING(strArgument) + "#0:\n";
            output.append("#2+----+-----------------------+---+#0\n");
            output.append("#2| ID |   Short Description   | F?|\n");
            output.append("#2+----+-----------------------+---+#0\n");

            for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
            {
                if (atoi(strArgument.c_str()))
                {
                    if (str_cmp(mob_vc_category(atoi(strArgument.c_str())), it->second.category))
                        continue;
                }
                else
                {
                    if (strArgument != it->second.category)
                        continue;
                }

                lastCategory = str_dup(it->second.category);

                output.append("#2|#0");
                std::ostringstream conversion;
                conversion << it->second.id;

                for (i = 0, j = (3 - conversion.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion.str());
                output.append("#2 |#0 ");

                output.append(it->second.shorts, MIN((int) strlen(it->second.shorts), 21));

                for (i = 0, j = (22 - MIN((int) strlen(it->second.shorts), 21)); i < j; i++)
                    output.append(" ");

                output.append("#2| #0");

                if (str_cmp(it->second.shorts, it->second.full))
                    output.append("#9#2Y#0");
                else
                    output.append("#9#1N#0");

                output.append("#2 |#0 ");

                output.append("\n");
            }
            output.append("#2+----+-----------------------+---+#0\n");
        }
    }
    else if (argOne == "help")
    {
        output.append("\nSyntax:");
        output.append("\n   #2vmob list <category>#0: with no argument, shows all lists, otherwise, enter digit or name to list variables within list.");
        output.append("\n   #2vmob view <category> <short>#0: shows the short and full description of the variable..");
        output.append("\n   #2vmob delete <category> <short>#0: deletes the selected variable within the selected category.");
        output.append("\n   #2vmob add <category> <short>#0: adds a new variable to the selected category.");
        output.append("\n   #2vmob open <category>#0: opens a new category - remember to put a $ in front!");
        output.append("\n   #2vmob close <category>#0: closes an existing category and wipes all variables - BE CAREFUL.");
        output.append("\n   #2vmob rand <category>#0: selects a random variable from the category.");
        output.append("\n   #2vmob edit <category> <short> short '<argument>'#0: replaces the short of the selected variable with your argument.");
        output.append("\n   #2vmob edit <category> <short> full '<argument>'#0: replaces the full of the selected variable with your argument.\n");
    }
    else if (argOne == "delete")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to delete a variable from.\n";
        else if ((i = mob_vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to delete.\n";
            }
            else
            {
                for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                {
                    if (i == it->second.category_id)
                        if (!str_cmp(argThree.c_str(), it->second.shorts))
                        {
                            mvariable_list.erase(it);
                            checked = true;
                            break;
                        }
                }
                if (checked)
                    output += "\nVariable deleted!\n";
                else
                    output += "\nNo such variable found. Unable to delete.\n";

                mob_vc_renumber(i);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "view")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to view a variable from.\n";
        else if ((i = mob_vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to view.\n";
            }
            else
            {
                for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                {
                    if (i == it->second.category_id)
                        if (!str_cmp(argThree.c_str(), it->second.shorts))
                        {
                            checked = true;
                            output += "\n#2Short:#0 " + MAKE_STRING(it->second.shorts) + ", #2Full:#0 " + it->second.full + ".\n";
                            break;
                        }
                }
                if (!checked)
                    output += "\nNo such variable found. Unable to view.\n";

                mob_vc_renumber(i);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "rand")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to draw a random variable from.\n";
        else if ((i = vc_category((char *)argTwo.c_str())))
            output += "\nYour random variable is: #6" + MAKE_STRING(mob_vc_rand(i, &cat, NULL)) + "#0.\n";
    }
    else if (argOne == "add")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to add a variable too.\n";
        else if (mob_vc_category((char *)argTwo.c_str()))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate a short desc for the variable.\n";
            }
            else if (argThree.find(" ") != std::string::npos)
            {
                output += "\nYou can't have spaces in the variable short name.\n";
            }
            else if (mob_vc_exists((char *) argThree.c_str(), (char *) argTwo.c_str()))
            {
                output += "\nA variable by that name already exists in that category.\n";
            }
            else
            {
                while (argThree[i] != '\0')
                {
                    if (isdigit(argThree[i]))
                    {
                        output += "\nYou can't have digits in the variable short name.\n";
                        digit = true;
                        break;
                    }
                    i += 1;
                }
                if (!digit)
                {
                    mvariable_data mvariable;
                    mvariable.category = str_dup(argTwo.c_str());
                    mvariable.category_id = mob_vc_category(mvariable.category);
                    mvariable.shorts = str_dup(argThree.c_str());
                    mvariable.full = str_dup(argThree.c_str());
                    mvariable.id = (mob_vc_size(mvariable.category_id) + 1);
                    mvariable_list.insert(std::pair<int, mvariable_data>(mvariable.unique, mvariable));
                    output += "\nVariable added!\n";
                    mob_vc_renumber(mvariable.category_id);
                }
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "edit")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to edit a variable from.\n";
        else if ((i = mob_vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to edit.\n";
            }
            else
            {
                strArgument = one_argument(strArgument, argFour);
                if (argFour.empty() || strArgument.empty())
                {
                    output += "\nHow do you wish to edit the variable (short, category, full, weight_mod, cost_mod)?\n";
                }
                else if (argFour == "full")
                {
                    for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                    {
                        if (i == it->second.category_id)
                            if (!str_cmp(argThree.c_str(), it->second.shorts))
                            {
                                it->second.full = add_hash(strArgument.c_str());
                                checked = true;
                                break;
                            }
                    }
                    if (checked)
                        output += "\nVariable full description edited!\n";
                    else
                        output += "\nNo such variable found. Unable to edit.\n";

                    mob_vc_renumber(i);
                }
                else if (argFour == "short")
                {
                    if (strArgument.find(" ") == std::string::npos)
                    {
                        for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                        {
                            if (i == it->second.category_id)
                                if (!str_cmp(argThree.c_str(), it->second.shorts))
                                {
                                    it->second.shorts = add_hash(strArgument.c_str());
                                    checked = true;
                                    break;
                                }
                        }
                        if (checked)
                            output += "\nVariable short description edited!\n";
                        else
                            output += "\nNo such variable found. Unable to edit.\n";
                    }
                    else
                    {
                        output += "\nYou can't have spaces in the variable short name.\n";
                    }

                    mob_vc_renumber(i);
                }
                else if (argFour == "category")
                {
                    if (!(j = mob_vc_category((char *)strArgument.c_str())))
                    {
                        output += "\nNo such category: be sure to open it first.\n";
                    }
                    else
                    {
                        for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                        {
                            if (i == it->second.category_id)
                                if (!str_cmp(argThree.c_str(), it->second.shorts))
                                {
                                    it->second.category = add_hash((char *) strArgument.c_str());
                                    it->second.category_id = j;
                                    checked = true;
                                    break;
                                }
                        }
                        if (checked)
                            output += "\nVariable category description edited!\n";
                        else
                            output += "\nNo such variable found. Unable to edit.\n";

                        mob_vc_renumber(i);
                        mob_vc_renumber(j);
                    }
                }
                else
                {
                    output += "\nHow do you wish to edit the variable (short, category, full, weight_mod, cost_mod)?\n";
                }
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "open")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to open.\n";
        else if (mob_vc_category((char *)argTwo.c_str()))
            output += "\nThere already exists a category by that name!\n";
        else if (argTwo.find("$") == std::string::npos)
            output += "\nBe sure to include '$' at the start of your variable!\n";
        else
        {
            mvariable_categories.insert (std::pair<int, std::string>((mob_vc_count() + 1), argTwo.c_str()));
            output += "\nCategory added! Please note, you will have to add some variables against this category for it to save.\n";
        }
    }
    else if (argOne == "overwrite")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to overwrite.\n";
        else if (!IS_IMPLEMENTOR(ch))
        {
            output += "\nOnly Kithrater is allowed to do this at the moment.\n";
        }
        else if ((i = mob_vc_category((char *)argTwo.c_str())))
        {
            for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end(); it++)
                if (it->second.category_id == i)
                    it->second.shorts = "erase-me";

            output += "\nCategory and all entries overwritten.\n";
        }
        else
        {
            output += "\nNo such category.\n";
        }
    }
    else if (argOne == "close")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to open.\n";
        else if (!IS_IMPLEMENTOR(ch))
        {
            output += "\nOnly Kithrater is allowed to do this at the moment.\n";
        }
        else if ((i = mob_vc_category((char *)argTwo.c_str())))
        {
            for (std::multimap<int, mvariable_data>::iterator it = mvariable_list.begin(); it != mvariable_list.end();)
            {
                std::multimap<int, mvariable_data>::iterator nit;
                if (it->second.category_id == i)
                {
                    nit = it;
                    i++;
                    temp = "Erasing:" + MAKE_STRING(it->second.shorts);
                    send_to_gods(temp.c_str());
                    mvariable_list.erase(nit);
                }
                else
                    i++;
            }

            for (std::multimap<int, std::string>::iterator it = mvariable_categories.begin(); it != mvariable_categories.end(); it++)
                if (it->first == i)
                    mvariable_categories.erase(it);

            output += "\nCategory and all entries erased.\n";
        }
        else
        {
            output += "\nNo such category.\n";
        }
    }

    else
        output += "\nSorry, but #2" + MAKE_STRING(argOne) + " " + MAKE_STRING(strArgument) + "#0 isn't a valid variable subcommand.\n";

    page_string (ch->descr(), output.c_str());
}




void
do_variables (CHAR_DATA *ch, char *argument, int cmd)
{
    std::string strArgument = argument, argOne, argTwo, argThree, argFour, argFive, temp, output = "";
    int i = 0;
    int j = 0;
    int k = 0;
    char *lastCategory = '\0';
    strArgument = one_argument(strArgument, argOne);
    bool checked = false;
    int cat = 0, weight_mod = 0, cost_mod = 0;
    bool digit = false;

    if (argOne.empty())
        argOne = "help";

    if (argOne == "list")// Is there a variable by this short description in this category?
    {
        if (strArgument.empty())
        {
            output += "\n#2Available Object Variable Lists:#0\n";
            for (std::multimap<int, std::string>::iterator it = variable_categories.begin(); it != variable_categories.end(); it++)
            {
                k++;
                if (k < 10)
                    output += " ";
                if (k < 100)
                    output += " ";
                output += "#B" + MAKE_STRING(it->first) + ":#0 " + it->second;

                for (i = 0, j = (24 - MIN((int) strlen(it->second.c_str()), 21)); i < j; i++)
                    output.append(" ");

                if (k % 3 == 0)
                    output += "\n";

            }
            output += "\n";

        }
        else if (!vc_category(atoi(strArgument.c_str())) && !vc_category((char*) strArgument.c_str()))
        {
            output += "\nThere is either no such list, or no variables currently in that list.\n";
        }
        else
        {

            if (strArgument == "$all")
                digit = true;

            output += "\nVariables for list #2" + MAKE_STRING(strArgument) + "#0:\n";
            output.append("#2+----+-----------------------+---+---+---+-------+------+------+---------+-------+-----+--+--+--+--+--+--+#0\n");
            output.append("#2| ID |   Short Description   | F?|RnD|Flv| WtMod | $Mod | QMod | ObjType | Skill | Mod | 0| 1| 2| 3| 4| 5|\n");
            output.append("#2+----+-----------------------+---+---+---+-------+------+------+---------+-------+-----+--+--+--+--+--+--+#0\n");

            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
            {
                if (atoi(strArgument.c_str()))
                {
                    if (str_cmp(vc_category(atoi(strArgument.c_str())), it->second.category))
                        continue;
                }
                else if (!digit)
                {
                    if (strArgument != it->second.category)
                        continue;
                }

                lastCategory = str_dup(it->second.category);

                output.append("#2|#0");
                std::ostringstream conversion;
                conversion << it->second.id;

                for (i = 0, j = (3 - conversion.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion.str());
                output.append("#2 |#0 ");

                output.append(it->second.shorts, MIN((int) strlen(it->second.shorts), 21));

                for (i = 0, j = (22 - MIN((int) strlen(it->second.shorts), 21)); i < j; i++)
                    output.append(" ");

                output.append("#2| #0");

                if (str_cmp(it->second.shorts, it->second.full))
                    output.append("#9#2Y#0");
                else
                    output.append("#9#1N#0");

                output.append("#2 |#0 ");

                if (it->second.random)
                    output.append("#9#2N#0");
                else
                    output.append("#9#1Y#0");

                output.append("#2 |#0 ");

                if (it->second.flavour)
                    output.append("#9#2N#0");
                else
                    output.append("#9#1Y#0");

                output.append("#2 |#0 ");

                std::ostringstream conversion2;
                conversion2 << it->second.weight_mod;

                for (i = 0, j = (4 - conversion2.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion2.str());
                output.append("%#2 |#0 ");

                std::ostringstream conversion3;
                conversion3 << it->second.cost_mod;

                for (i = 0, j = (3 - conversion3.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion3.str());
                output.append("%#2 | #0");

                std::ostringstream conversion11;
                conversion11 << it->second.quality_mod;

                for (i = 0, j = (3 - conversion11.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion11.str());
                output.append("%#2 | #0");

                output.append(item_types[it->second.item_type], MIN((int) strlen(item_types[it->second.item_type]), 7));

                for (i = 0, j = (8 - MIN((int) strlen(item_types[it->second.item_type]), 7)); i < j; i++)
                    output.append(" ");

                output.append("#2|#0");

                output.append(skills[it->second.skill_name], MIN((int) strlen(item_types[it->second.skill_name]), 6));

                for (i = 0, j = (6 - MIN((int) strlen(item_types[it->second.skill_name]), 5)); i < j; i++)
                    output.append(" ");

                output.append("#2|#0");

                std::ostringstream conversion4;
                conversion4 << it->second.skill_mod;

                for (i = 0, j = (3 - conversion4.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion4.str());
                output.append(" #2 |#0");

                std::ostringstream conversion5;
                conversion5 << it->second.oval0;

                for (i = 0, j = (2 - conversion5.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion5.str());
                output.append("#2|#0");

                std::ostringstream conversion6;
                conversion6 << it->second.oval1;

                for (i = 0, j = (2 - conversion6.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion6.str());
                output.append("#2|#0");

                std::ostringstream conversion7;
                conversion7 << it->second.oval2;

                for (i = 0, j = (2 - conversion7.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion7.str());
                output.append("#2|#0");

                std::ostringstream conversion8;
                conversion8 << it->second.oval3;

                for (i = 0, j = (2 - conversion8.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion8.str());
                output.append("#2|#0");

                std::ostringstream conversion9;
                conversion9 << it->second.oval4;

                for (i = 0, j = (2 - conversion9.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion9.str());
                output.append("#2|#0");

                std::ostringstream conversion10;
                conversion10 << it->second.oval5;

                for (i = 0, j = (2 - conversion10.str().length()); i < j; i++)
                    output.append(" ");

                output.append(conversion10.str());
                output.append("#2|#0");




                output.append("\n");
            }
            output.append("#2+----+-----------------------+---+---+-------+------+------+---------+-------+-----+--+--+--+--+--+--+#0\n");
        }
    }
    else if (argOne == "help")
    {
        output.append("\nSyntax:");
        output.append("\n   #2variable list <category>#0: with no argument, shows all lists, otherwise, enter digit or name to list variables within list.");
        output.append("\n   #2variable view <category> <short>#0: shows the short and full description of the variable..");
        output.append("\n   #2variable delete <category> <short>#0: deletes the selected variable within the selected category.");
        output.append("\n   #2variable add <category> <short>#0: adds a new variable to the selected category.");
        output.append("\n   #2variable open <category>#0: opens a new category - remember to put a $ in front!");
        output.append("\n   #2variable close <category>#0: closes an existing category and wipes all variables - BE CAREFUL.");
        output.append("\n   #2variable rand <category>#0: selects a random variable from the category.");
        output.append("\n\n   #2Variable edits are next: short, full, category, weight_mod and cost_mod are applied to all items the variable affects.");
        output.append("\n   #2However, skill, skill mod and the ovals are only applied to items which match the object type set.");
        output.append("\n   #2This is done because different ovals do different things on different objects.\n");
        output.append("\n   #2variable edit <category> <short> short '<argument>'#0: replaces the short of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> full '<argument>'#0: replaces the full of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> category '<argument>'#0: changes the category of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> weight_mod '<argument>'#0: changes the weight_mod of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> quality_mod '<argument>'#0: changes the quality_mod of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> cost_mod '<argument>'#0: changes the cost_mod of the selected variable with your argument.");
        output.append("\n   #2variable edit <category> <short> objtype '<argument>'#0: changes object type the variable will add skill and oval mods to.");
        output.append("\n   #2variable edit <category> <short> skill '<argument>'#0: changes which skill bonus the variable will add.");
        output.append("\n   #2variable edit <category> <short> skmod '<argument>'#0: changes the value of the skill bonus the variable will add.");
        output.append("\n   #2variable edit <category> <short> random#0: this variable will now not load up at random.\n");
        output.append("\n   #2variable edit <category> <short> oval <x> <y>#0: changes the oval mod that the variable will apply.\n");
    }
    else if (argOne == "delete")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to delete a variable from.\n";
        else if ((i = vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to delete.\n";
            }
            else
            {

                int index = 0;
                if (atoi(argThree.c_str()))
                {
                    index = atoi(argThree.c_str());
                }
                int count = 0;

                for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
                {
                    if (i == it->second.category_id)
                    {
                        count++;
                        if ((index && index == count) || !str_cmp(argThree.c_str(), it->second.shorts))
                        {
                            obj_variable_list.erase(it);
                            checked = true;
                            break;
                        }
                    }
                }
                if (checked)
                    output += "\nVariable deleted!\n";
                else
                    output += "\nNo such variable found. Unable to delete.\n";

                vc_renumber(i);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "view")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to view a variable from.\n";
        else if ((i = vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to view.\n";
            }
            else
            {
                int index = 0;
                if (atoi(argThree.c_str()))
                {
                    index = atoi(argThree.c_str());
                }
                int count = 0;

                for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
                {
                    if (i == it->second.category_id)
                    {
                        count++;
                        if ((index && index == count) || !str_cmp(argThree.c_str(), it->second.shorts))
                        {
                            checked = true;
                            output += "\n#2Short:#0 " + MAKE_STRING(it->second.shorts) + ", #2Full:#0 " + it->second.full;
							if (it->second.flavour)
							{
								output += ", #2Flavour:#0 ";
								output += it->second.flavour;
							}
							output += + ".\n";

                            break;
                        }
                    }
                }
                if (!checked)
                    output += "\nNo such variable found. Unable to view.\n";

                vc_renumber(i);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "rand")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to draw a random variable from.\n";
        else if ((i = vc_category((char *)argTwo.c_str())))
            output += "\nYour random variable is: #6" + MAKE_STRING(vc_rand(i, &cat, NULL, &weight_mod, &cost_mod)) + "#0.\n";
    }
    else if (argOne == "add")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to add a variable too.\n";
        else if (vc_category((char *)argTwo.c_str()))
        {
            if (strArgument.empty())
            {
                output += "\nYou need to nominate a short desc for the variable.\n";
            }
            else if (vc_exists((char *) strArgument.c_str(), (char *) argTwo.c_str()))
            {
                output += "\nA variable by that name already exists in that category.\n";
            }
            else
            {
                variable_data variable;
                variable.category = str_dup(argTwo.c_str());
                variable.category_id = vc_category(variable.category);
                variable.shorts = str_dup(strArgument.c_str());
                variable.full = str_dup(strArgument.c_str());
                variable.cost_mod = 100;
                variable.weight_mod = 100;
                variable.quality_mod = 100;
                variable.skill_name = 0;
                variable.skill_mod = 0;
                variable.item_type = 0;
                variable.oval0 = 0;
                variable.oval1 = 0;
                variable.oval2 = 0;
                variable.oval3 = 0;
                variable.oval4 = 0;
                variable.oval5 = 0;
                variable.random = 0;
                variable.id = (vc_size(variable.category_id) + 1);
                obj_variable_list.insert(std::pair<int, variable_data>(variable.unique, variable));
                output += "\nVariable added!\n";
                vc_renumber(variable.category_id);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "edit")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category to edit a variable from.\n";
        else if (argTwo == "$chemical")
        {
            output += "\nOnly Kithrater is allowed to edit this set of variables.\n";
        }
        else if ((i = vc_category((char *)argTwo.c_str())))
        {
            strArgument = one_argument(strArgument, argThree);
            if (argThree.empty())
            {
                output += "\nYou need to nominate the short desc of the variable you wish to edit.\n";
            }
            else
            {
                strArgument = one_argument(strArgument, argFour);
                if (argFour.empty() || strArgument.empty())
                {
                    output += "\nHow do you wish to edit the variable (short, category, full, weight_mod, cost_mod)?\n";
                }

                int index = 0;
                if (atoi(argThree.c_str()))
                {
                    index = atoi(argThree.c_str());
                }
                int count = 0;

                for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
                {
                    if (i == it->second.category_id)
                    {
                        count++;
                        if ((index && index == count) || !str_cmp(argThree.c_str(), it->second.shorts))
                        {
                            if (argFour == "full")
                            {
                                it->second.full = add_hash(strArgument.c_str());
                                checked = true;
                                break;
                            }
                            else if (argFour == "flavour")
                            {
                                it->second.flavour = add_hash(strArgument.c_str());
                                checked = true;
                                break;
                            }
                            else if (argFour == "short")
                            {
                                it->second.shorts = add_hash(strArgument.c_str());
                                checked = true;
                                break;
                            }
                            else if (argFour == "cost_mod")
                            {
                                j = atoi((char *) strArgument.c_str());
                                if (!j)
                                    j = 100;
                                it->second.cost_mod = j;
                                checked = true;
                                break;
                            }
                            else if (argFour == "quality_mod")
                            {
                                j = atoi((char *) strArgument.c_str());
                                if (!j)
                                    j = 100;
                                it->second.quality_mod = j;
                                checked = true;
                                break;
                            }
                            else if (argFour == "weight_mod")
                            {
                                j = atoi((char *) strArgument.c_str());
                                if (!j)
                                    j = 100;
                                it->second.weight_mod = j;
                                checked = true;
                                break;
                            }
                            else if (argFour == "objtype")
                            {
                                if ((j = index_lookup(item_types, strArgument.c_str())) == -1)
                                {
                                    output += "\nNo such item type found. Unable to edit.\n";
                                    break;
                                }
                                it->second.item_type = j;
                                checked = true;
                                break;
                            }
                            else if (argFour == "skill")
                            {
                                if ((j = index_lookup(skills, strArgument.c_str())) == -1)
                                {
                                    output += "\nNo such skill. Unable to edit.\n";
                                    break;
                                }
                                it->second.skill_name = j;
                                checked = true;
                                break;

                            }
                            else if (argFour == "skmod")
                            {
                                j = atoi((char *) strArgument.c_str());
                                if (!j)
                                    j = 0;
                                it->second.skill_mod = j;
                                checked = true;
                                break;
                            }
                            else if (argFour == "random")
                            {
                                if (it->second.random)
                                    it->second.random = 0;
                                else
                                    it->second.random = 1;

                                checked = true;
                                break;
                            }
                            else if (argFour == "oval")
                            {
                                strArgument = one_argument(strArgument, argFive);
                                j = atoi(argFive.c_str());
                                if (argFive.empty() || j > 5 || j < 0)
                                {
                                    output += "\nNo such oval: enter a value between 0 and 5.\n";
                                    break;
                                }
                                else if (!IS_IMPLEMENTOR(ch))
                                {
                                    output += "\nOnly Kithrater is allowed to do this at the moment.\n";
                                    break;
                                }

                                k = atoi((char *) strArgument.c_str());
                                if (!k)
                                    k = 0;

                                switch (j)
                                {
                                case 0:
                                    it->second.oval0 = k;
                                    break;
                                case 1:
                                    it->second.oval1 = k;
                                    break;
                                case 2:
                                    it->second.oval2 = k;
                                    break;
                                case 3:
                                    it->second.oval3 = k;
                                    break;
                                case 4:
                                    it->second.oval4 = k;
                                    break;
                                case 5:
                                    it->second.oval5 = k;
                                    break;
                                }
                                checked = true;
                                break;
                            }
                            else if (argFour == "category")
                            {
                                if (!(j = vc_category((char *)strArgument.c_str())))
                                {
                                    output += "\nNo such category: be sure to open it first.\n";
                                    break;
                                }

                                it->second.category = add_hash((char *) strArgument.c_str());
                                it->second.category_id = j;
                                vc_renumber(j);
                                checked = true;
                                break;
                            }
                            else
                            {
                                output += "\nHow do you wish to edit the variable (short, category, full, weight_mod, cost_mod)?\n";
                                break;
                            }
                        }
                    }
                }

                if (checked)
                    output += "\nVariable category description edited!\n";
                else
                    output += "\nNo such variable found. Unable to edit.\n";

                vc_renumber(i);
            }
        }
        else
            output += "\nNo such category: you need to 'open' it first.\n";
    }
    else if (argOne == "open")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to open.\n";
        else if (vc_category((char *)argTwo.c_str()))
            output += "\nThere already exists a category by that name!\n";
        else if (argTwo.find("$") == std::string::npos)
            output += "\nBe sure to include '$' at the start of your variable!\n";
        else
        {
            variable_categories.insert (std::pair<int, std::string>((vc_count() + 1), argTwo.c_str()));
            output += "\nCategory added! Please note, you will have to add some variables against this category for it to save.\n";
        }
    }
    else if (argOne == "seeable")
    {
        strArgument = one_argument(strArgument, argTwo);
        strArgument = one_argument(strArgument, argThree);
        bool toggle_off = false;
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to toggle the see-ability of.\n";
        else if ((i =vc_category((char *)argTwo.c_str())))
        {
            if (argThree.empty())
            {
              toggle_off = false;
            }
            else
            {
              toggle_off = true;
            }

            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
            {
                if (it->second.category_id == i)
                {
                    if (toggle_off)
                    {
                        it->second.category_seeable = 0;
                    }
                    else
                    {
                        it->second.category_seeable = 1;
                    }
                }
            }

            if (toggle_off)
                output += "\nCategory is no longer seeable by PCs.\n";
            else
                output += "\nCategory is now able to be seen PCs.\n";

        }
        else
        {
            output += "\nThere isn't any category of that name.\n";
        }
    }
    else if (argOne == "overwrite")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to overwrite.\n";
        else if (!IS_IMPLEMENTOR(ch))
        {
            output += "\nOnly Kithrater is allowed to do this at the moment.\n";
        }
        else if ((i =vc_category((char *)argTwo.c_str())))
        {
            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end(); it++)
                if (it->second.category_id == i)
                    it->second.shorts = "erase-me";

            output += "\nCategory and all entries overwritten.\n";
        }
        else
        {
            output += "\nNo such category.\n";
        }
    }
    else if (argOne == "close")
    {
        strArgument = one_argument(strArgument, argTwo);
        if (argTwo.empty())
            output += "\nYou need to nominate a category name to open.\n";
        else if (!IS_IMPLEMENTOR(ch))
        {
            output += "\nOnly Kithrater is allowed to do this at the moment.\n";
        }
        else if ((i = vc_category((char *)argTwo.c_str())))
        {

            std::multimap<int, variable_data>::iterator nit;

            for (std::multimap<int, variable_data>::iterator it = obj_variable_list.begin(); it != obj_variable_list.end();)
            {
                if (it->second.category_id == i)
                {
                    nit = it;
                    it++;
                    obj_variable_list.erase(nit);
                }
                else
                    it++;
            }

            for (std::multimap<int, std::string>::iterator it = variable_categories.begin(); it != variable_categories.end(); it++)
            {
                if (it->first == i)
                {
                    variable_categories.erase(it);
                }
            }

            output += "\nCategory and all entries erased.\n";
        }
        else
        {
            output += "\nNo such category.\n";
        }
    }

    else
        output += "\nSorry, but #2" + MAKE_STRING(argOne) + " " + MAKE_STRING(strArgument) + "#0 isn't a valid variable subcommand.\n";

    page_string (ch->descr(), output.c_str());
}


