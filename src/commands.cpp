/*------------------------------------------------------------------------\
|  commands.c : Root Command Parser                   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "group.h"


DESCRIPTOR_DATA *last_descriptor;
char full_last_command[MAX_STRING_LENGTH];
char last_command[MAX_STRING_LENGTH];

extern std::multimap<int, room_prog> mob_prog_list;
extern std::multimap<int, room_prog> obj_prog_list;

const struct command_data commands[] =
{

    /* Mortal commands */

    {"\01craft", do_say, REST, C_XLS
    },
    {".", do_say, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {",", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {":", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {"aide", do_aide, FIGHT, C_BLD | C_HID | C_WLK},
    {"accept", do_accept, REST, C_DEL | C_BLD | C_HID},
    {"accuse", do_accuse, REST, C_BLD},
    {"aim", firearm_aim, SIT, C_BLD | C_HID}, //| C_MNT
    {"alert", do_alert, FIGHT, C_BLD},
    {"apply", do_apply, SIT, C_WLK},
    {"as", do_as, DEAD, C_LV3},
    {"assist", do_assist, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {"auction", do_auction, SIT, C_BLD},
    {"arena", do_arena, DEAD},
    {"aggro", do_aggro, DEAD},
    {"bash", do_combat_bash, FIGHT, C_WLK},
    {"barter", do_barter, SIT, C_MNT},
    {"behead", do_behead, STAND, C_WLK | C_MNT},
    {"bind", char__do_bind, SIT, C_WLK | C_MNT},
    {"blank", do_pointblank, FIGHT, C_BLD},
    {"blindfold", do_blindfold, STAND, C_MNT},
    {"bolt", do_bolt, STAND, C_HID | C_WLK | C_XLS},
    {"bridle", do_bridle, STAND, C_MNT},
    {"bug", do_bug, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"buy", do_buy, SIT, C_MNT},
    {"buck", do_buck, FIGHT, C_BLD | C_XLS},
    {"butcher", do_butcher, STAND, C_WLK | C_MNT},
    {"check", do_check, REST, C_DEL | C_SUB | C_HID | C_PAR | C_SPL | C_NLG},
    {"camp", do_camp, STAND, C_WLK | C_BLD | C_MNT},
    {"cards", do_card, SIT, C_SUB | C_SPL},
    {"castout", do_castout, REST, C_BLD},
    {"close", do_close, SIT, C_MNT},
    {"clean", do_clean, SIT},

    {"clockout", do_clockout, SIT, C_MNT},
    {"clockin", do_clockin, SIT, C_MNT},

    {"checkshop", do_checkshop, SIT, C_MNT},
    {"checkstore", do_checkstore, SIT, C_MNT},

    {"command", do_command, REST, C_BLD},
    {"combine", do_combine, SIT, C_WLK | C_MNT},
    {"commands", do_commands, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_NLG
    },
    {"commence", do_commence, REST,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR
    },
    /*{"compact", do_compact, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR
    },*/
    {"compare", do_compare, SIT, 0},
    {"contents", do_contents, REST,
     C_SUB | C_DOA | C_PAR | C_SPL | C_NLG | C_HID
    },
    {"conceal", do_conceal, REST},
    {"count", do_count, REST, C_NLG},
    {"cover", do_cover, REST, C_WLK | C_MNT},
    {"crawl", do_crawl, SIT, C_BLD},
    {"crafts", do_crafts, DEAD,
     C_DEL | C_SUB | C_HID | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"credits", do_credits, DEAD,
     C_DEL | C_SUB | C_HID | C_DOA | C_BLD | C_PAR | C_SPL
    },
    {"down", do_down, FIGHT, C_HID | C_DOA | C_BLD},
    {"decorate", do_decorate, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
    {"diagnose", do_diagnose, REST, C_DEL | C_SUB | C_HID | C_DOA | C_PAR},
    //{"destring", do_destring, STAND, C_WLK | C_HID | C_DOA}, //C_MNT
    {"dip", do_dip, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
    {"dismount", do_dismount, STAND, 0},
    {"disband", do_disband, SIT, C_BLD},
    {"dismantle", do_dismantle, STAND, C_WLK | C_BLD | C_MNT},
    {"discard", do_discard, SIT, C_SUB | C_SPL},
    {"decline", do_decline, REST, C_DEL | C_BLD},
    {"deal", do_deal, SIT, C_SUB | C_SPL},
	{"deactivate", do_deactivate, DEAD,
     C_DEL | C_SUB | C_HID | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"dmote", do_dmote, SLEEP, C_SUB | C_DOA | C_HID | C_BLD | C_PAR | C_SPL | C_DEL},
    {"draw", do_draw, REST, C_BLD},
    {"drag", do_drag, STAND, C_WLK | C_MNT},
    {"dreams", do_dreams, SLEEP, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
    {"drink", do_drink, REST, C_BLD},
    {"drop", do_drop, SLEEP, C_DOA | C_BLD},
    {"doitanyway", do_doitanyway, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"east", do_east, FIGHT, C_HID | C_DOA | C_BLD},
    {"eat", do_eat, REST, C_BLD},
    {"eavesdrop", do_eavesdrop, STAND, C_WLK},
    {"emote", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL | C_SPL},
    {"empty", do_empty, REST, C_BLD},
    {"enter", do_enter, STAND, C_WLK | C_BLD},
    {"equipment", do_equipment, SLEEP,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"erase", do_erase, REST, C_DEL | C_HID | C_SUB | C_DOA | C_BLD},
    {"escape", do_escape, STAND, C_SUB | C_BLD},
    {"evaluate", do_evaluate, SIT, 0},
    {"exits", do_exits, REST,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"examine", do_examine, REST, C_HID | C_DEL},
    {"extract", do_extract, REST, C_WLK},
    {"feel", do_feel, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"feint", do_combat_feint, FIGHT, C_WLK},
    {"fill", do_fill, REST, C_MNT},
    {"fire", do_firearm_fire, SIT, C_BLD},
    {"flee", do_flee, FIGHT, 0},
    {"flip", do_flip, REST, C_DOA},
    {"follow", do_follow, FIGHT, C_BLD | C_PAR},
    {"force", do_force, DEAD, C_LV4 | C_DEL | C_SUB | C_HID},
    {"focus", do_focus, SLEEP,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"forage", do_rummage, STAND, C_WLK | C_MNT | C_HID},
    {"furnishings", do_tables, REST,
     C_SUB | C_DOA | C_MNT | C_PAR | C_SPL | C_NLG
    },
    {"furniture", do_tables, REST,
     C_SUB | C_DOA | C_MNT | C_PAR | C_SPL | C_NLG
    },
    {"fallback", do_fallback, DEAD},
    {"gather", do_gather, STAND, C_WLK | C_MNT | C_HID},
    {"get", do_get, REST, C_BLD},
    {"give", do_give, REST, C_DOA},
    {"grip", do_grip, REST, C_DEL | C_HID | C_DOA | C_BLD | C_PAR | C_WLK},
    {"group", do_group, REST,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"guard", do_guard, REST, C_BLD | C_HID | C_PAR | C_WLK},
    {"hmote", do_hemote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL | C_SPL | C_HID},
    {"hemote", do_hemote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL | C_SPL | C_HID},
    {"help", do_help, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_XLS | C_PAR | C_SPL
    },
    {"helpline", do_helpline, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_XLS | C_PAR | C_SPL},
    {"heal", do_heal, STAND, C_WLK | C_XLS | C_MNT},
    {"health", do_health, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"hex", do_hex, SIT, C_WLK | C_HID | C_XLS},
    {"hide", do_hide, STAND, C_WLK | C_HID | C_MNT},
    {"hire", do_hire, REST, C_HID | C_SUB | C_DOA | C_PAR},
    {"hit", do_hit, FIGHT, C_BLD | C_HID | C_WLK},
    {"hitch", do_hitch, STAND, C_MNT},
    {"holster", do_sheathe, REST, C_BLD},
    {"hood", do_hood, REST, C_HID | C_BLD | C_DOA},
    {"inventory", do_equipment, SLEEP,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"idea", do_idea, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL
    },
    {"identify", do_identify, STAND, C_WLK | C_MNT},
    {"info", do_info, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"inside", do_inside, FIGHT, C_HID | C_DOA | C_BLD},
    {"invite", do_invite, SIT, 0},
    {"journal", do_journal, DEAD, C_WLK | C_BLD | C_HID | C_SPL},
    {"jerase", do_jerase, DEAD, C_WLK | C_BLD | C_HID | C_SPL},
    {"jread", do_jread, DEAD, C_WLK | C_BLD | C_HID | C_SPL},
    {"jwrite", do_jwrite, DEAD, C_WLK | C_BLD | C_HID | C_SPL},
    {"junk", do_junk, REST },
    {"ki", do_nokill, DEAD, C_XLS},
    {"kill", do_kill, FIGHT, C_BLD | C_HID | C_WLK},
    {"knock", do_knock, STAND, C_BLD},
    {"look", do_look, REST, C_DEL | C_HID | C_SUB | C_DOA | C_PAR},
    {"leave", do_leave, STAND, C_WLK | C_BLD},
    {"light", do_light, SIT, C_HID | C_DOA},
    {"list", do_list, SIT, C_BLD},
    {"load", do_load, SIT, C_WLK | C_HID | C_DOA}, //C_MNT
    {"lock", do_lock, SIT, C_MNT},
    {"logging", do_logging, STAND, C_WLK | C_MNT},
    {"materials", do_materials, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"mark", do_mark, REST },
    {"mend", do_mend, STAND, C_WLK | C_MNT},	/* object.c */
    {"modify", do_modify, REST, C_BLD},
    {"mount", do_mount, STAND, C_MNT},
    {"mix", do_mix, STAND, C_WLK | C_MNT},
    {"mute", do_mute, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"north", do_north, FIGHT, C_HID | C_DOA | C_BLD},
    {"n", do_north, FIGHT, C_HID | C_DOA | C_BLD},
    {"northeast", do_northeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"northwest", do_northwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"neast", do_northeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"ne", do_northeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"nwest", do_northwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"nw", do_northwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"nod", do_nod, REST, C_HID | C_SUB | C_DOA | C_BLD},
    {"notify", do_notify, DEAD, C_DEL | C_HID | C_SUB | C_BLD | C_PAR},
    {"news", do_news, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"nominate", do_nominate, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL    },
    {"overwatch", do_overwatch, REST, C_HID},
    {"omote", do_omote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_DEL},
    {"ooc", do_ooc, REST, C_WLK | C_SPL | C_DEL},
    {"open", do_open, SIT, C_BLD},
    {"order", do_order, REST, C_BLD},
	{"origins", do_origins, REST, C_BLD},
    {"outside", do_outside, FIGHT, C_HID | C_DOA | C_BLD},
    {"ownership", do_ownership, REST},
    {"pardon", do_pardon, REST, C_BLD},
    {"palm", do_palm, SIT, C_WLK | C_BLD | C_HID | C_MNT},
    {"pay", do_pay, REST, C_WLK | C_BLD},
    {"payday", do_payday, REST, C_HID | C_SUB | C_DOA | C_PAR},
    {"payroll", do_payroll, SIT, C_BLD},
    //{"pack", do_pack, REST, C_BLD},
    {"petition", do_petition, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL
    },
    {"pick", do_pick, STAND, C_WLK | C_MNT},
    {"pitch", do_pitch, STAND, C_WLK | C_BLD | C_MNT},
    {"plan", do_plan, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL
    },
    {"pmote", do_pmote, SLEEP,
     C_SUB | C_DOA | C_HID | C_BLD | C_PAR | C_SPL | C_DEL
    },
    {"point", do_point, STAND, C_WLK},
    //{"poison", do_poison, REST, C_WLK | C_MNT},
    {"pocket", do_sheathe, REST, C_BLD},
    {"pour", do_pour, REST, 0},
    {"preview", do_preview, SIT, C_BLD},
    {"prescience", do_prescience, REST, C_WLK | C_XLS},
      {"progress", do_progress, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"promote", do_promote, STAND, 0},
    {"put", do_put, REST, C_BLD},
    {"push", do_push, STAND, C_WLK | C_BLD},
    {"quaff", do_quaff, REST, C_BLD},
    {"inject", do_inject, STAND, C_WLK | C_BLD | C_HID | C_MNT},
    {"ignore", do_ignore, SLEEP,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"quit", do_quit, DEAD, C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
    {"qscan", do_qscan, SIT, C_HID | C_WLK},
    {"transmit", do_radio, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {"read", do_read, REST, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
    {"reach", do_reach, REST, C_HID | C_DEL | C_WLK | C_BLD | C_PAR | C_SUB},
    {"recruit", do_recruit, SIT, C_BLD},
    {"receipts", do_receipts, SIT, C_BLD},
    {"release", do_release, STAND, C_BLD},
    {"remove", do_remove, REST, C_BLD},
	{"rent", do_rent, REST, C_BLD},
    {"rescue", do_rescue, FIGHT, C_WLK},
    {"rest", do_rest, REST, C_WLK | C_HID | C_DOA | C_BLD | C_PAR},
    //{"restring", do_restring, STAND, C_WLK | C_HID | C_DOA | C_MNT},
    {"return", do_return, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
    {"reveal", do_reveal, REST},
    {"roll", do_roll, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
    {"rollcall", do_rollcall, SIT, C_BLD},
    {"rummage", do_rummage, STAND, C_WLK | C_MNT | C_HID},
    {"south", do_south, FIGHT, C_HID | C_DOA | C_BLD},
    {"s", do_south, FIGHT, C_HID | C_DOA | C_BLD},
    {"southeast", do_southeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"southhwest", do_southwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"seast", do_southeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"se", do_southeast, FIGHT, C_HID | C_DOA | C_BLD},
    {"swest", do_southwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"sw", do_southwest, FIGHT, C_HID | C_DOA | C_BLD},
    {"say", do_say, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {"save", do_save, SLEEP, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"score", do_score, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"scout", do_scout, STAND, C_WLK | C_MNT},
    {"scommand", do_scommand, FIGHT, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"scan", do_scan, SIT, C_HID | C_DEL},
    {"scribe", do_select_script, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"search", do_search, STAND, C_WLK | C_MNT},
    {"see", do_see, REST, C_HID | C_XLS},
    {"sell", do_sell, SIT, C_BLD},
    {"sense", do_sense, SIT, C_HID | C_XLS},
    {"set", do_set, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"sheathe", do_sheathe, REST, C_BLD},
    {"shadow", do_shadow, STAND, C_WLK | C_SUB | C_HID | C_DEL | C_PAR},
    {"shine", do_shine, STAND, C_WLK},
    {"shoot", do_firearm_fire, SIT, C_BLD | C_PAR | C_DEL},
    {"shout", do_shout, REST, C_BLD | C_PAR | C_DEL},
    {"shuffle", do_shuffle, SIT, C_SUB | C_SPL},
    {"sit", do_sit, REST, C_HID | C_DOA | C_BLD | C_PAR},
    {"sip", do_sip, REST, C_BLD},
    {"sing", do_sing, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
    {"skills", do_skills, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"skin", do_skin, STAND, C_WLK | C_MNT},
    {"sleep", do_sleep, SLEEP, C_HID | C_BLD | C_PAR},
    {"smell", do_smell, REST, C_DEL | C_HID | C_SUB | C_DOA | C_PAR},
    {"sneak", do_sneak, STAND, C_WLK | C_HID | C_MNT},
    {"sniff", do_smell, REST, C_DEL | C_HID | C_SUB | C_DOA | C_PAR},
    {"speak", do_speak, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
    {"spray", do_spray, STAND, C_WLK | C_BLD | C_HID | C_MNT},
    {"stand", do_stand, SLEEP, C_HID | C_DOA | C_BLD},
    {"stable", do_stable, STAND, C_MNT | C_WLK},
    {"steal", do_steal, STAND, C_WLK | C_MNT},
    {"stop", do_stop, SIT, C_BLD | C_DEL | C_PAR | C_SPL},
    {"strike", do_aimstrike, FIGHT, C_WLK},
    //{"string", do_string, STAND, C_WLK | C_HID | C_DOA}, //C_MNT
    {"study", do_study, SIT, 0},
    {"subdue", do_subdue, FIGHT, C_WLK | C_MNT},
    {"surrender", do_surrender, FIGHT, C_WLK | C_HID | C_MNT},
    {"sentinel", do_sentinel, DEAD},
    {"float", do_float, STAND, C_BLD},
    {"swim", do_swim, STAND, C_BLD},
    {"switch", do_switch, REST, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"shooter", do_shooter, DEAD},
	{"talk", do_talk, REST, C_DEL | C_SUB | C_DOA | C_BLD | C_PAR},
	{"tell", do_tell, REST, C_DEL | C_DOA | C_SUB | C_PAR},
    {"tmote", do_temote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL | C_SPL},
    {"temote", do_temote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL | C_SPL},
    {"take", do_take, STAND, C_MNT},
    {"tables", do_tables, REST, C_SUB | C_DOA | C_MNT | C_PAR | C_SPL | C_HID},
    {"tags", do_tags, DEAD},
    {"teach", do_teach, STAND, C_MNT},
    {"tear", do_tear, REST, C_DOA},
    {"think", do_think, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"throw", do_throw, STAND, C_BLD},
    {"time", do_time, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"timeconvert", do_timeconvert, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"title", do_title, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
    {"toll", do_toll, STAND, C_WLK | C_MNT},
    {"toss", char__do_toss, SIT, C_SUB | C_SPL},
    {"track", do_track, FIGHT, C_WLK | C_DOA | C_BLD | C_MNT},
	{"trade", do_trade, SIT, C_MNT},
    {"trap", do_trap, STAND, C_WLK | C_MNT},
    {"travel", do_travel, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},	/* act.comm.c */
    {"treat", do_treat, STAND, C_WLK | C_MNT},
    {"turn", do_modify, REST, C_BLD},
    {"typo", do_typo, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL},
    {"talents", do_talents, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG},
    {"up", do_up, FIGHT, C_HID | C_DOA | C_BLD},
    {"unload", do_unload, SIT, C_HID | C_DOA}, //C_MNT |
    {"unlock", do_unlock, SIT, C_MNT},
    {"unhitch", do_unhitch, STAND, 0},
    {"upgrade", do_upgrade, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL
    },
    //{"unpack", do_take, STAND, C_MNT},
    {"value", do_value, SIT, C_BLD},
    {"vis", do_vis, REST, C_DEL | C_SUB | C_DOA | C_BLD | C_PAR},
    {"voice", do_voice, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL
    },
    {"west", do_west, FIGHT, C_HID | C_DOA | C_BLD},
    {"watch", do_mwatch, SIT, C_WLK | C_HID},
    {"ward", do_combat_ward, FIGHT, C_WLK},
    {"wake", do_wake, SLEEP, C_HID | C_DOA | C_BLD},
    {"wanted", do_wanted, REST, C_BLD | C_SPL},
    {"wear", do_wear, REST, C_BLD},
    // DISABLED-WEATHER-CODE
    {"weather", do_weather, REST,
     C_DEL | C_HID | C_SUB | C_BLD | C_PAR | C_SPL
    },
    {"whirl", do_whirl, FIGHT, C_WLK | C_MNT},
    {"whisper", do_whisper, REST,
     C_SUB | C_DOA | C_BLD | C_MNT | C_PAR | C_DEL
    },
    {"who", do_who, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_SPL | C_NLG
    },
    {"wield", do_wield, REST, C_BLD},
    {"write", do_write, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
    {"wipe", do_wipe, REST, C_DOA},
    {"x", do_check, REST, C_DEL | C_SUB | C_HID | C_PAR | C_SPL | C_NLG},
    {"wmote", do_attire, REST, C_BLD},
    {"yell", do_shout, REST, C_SUB | C_BLD | C_PAR | C_DEL},

	{"mscan", do_mscan, SIT, C_HID | C_DEL},
	{"oscan", do_oscan, SIT, C_HID | C_DEL},

    /* Guide Commands */
    {"history", do_history, DEAD, C_GDE | C_DEL},
    {"mobile", do_mobile, DEAD, C_GDE},
    {"mset", do_mset, DEAD, C_GDE},
    {"review", do_review, DEAD, C_GDE},
    {"show", do_show, DEAD, C_GDE},

    /* General Staff Commands (Level 0) */

    {";", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
    {"\\", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
    {"ichat", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
    {"wiznet", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
    {"broadwave", do_broadwave, DEAD, C_DOA | C_DEL | C_HID},
    {"bwave", do_broadwave, DEAD, C_DOA | C_DEL | C_HID},
    {"wizlist", do_wizlist, DEAD,
     C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR
    },

    /* Basic Builder level 1: (basic commands, room and objects only) */
    {"at", do_at, DEAD, C_LV1},
    {"blog", do_blog, DEAD, C_LV1},	/* Building Log */
    {"goto", do_goto, DEAD, C_LV1},
    {"gstat", do_gstat, DEAD, C_LV1},
    {"invis", do_invis, DEAD, C_LV1},
    {"immcommands", do_immcommands, DEAD, C_LV1},
    {"map", do_map, DEAD, C_LV1},	/* staff.c */
    {"monitor", do_monitor, DEAD, C_LV1},	/* Monitor command - Shade */
    {"object", do_object, DEAD, C_LV1},
    {"oinit", do_oinit, DEAD, C_LV1},
    {"olist", do_olist, DEAD, C_LV1},
    {"oset", do_oset, DEAD, C_LV1},
    {"ounused", do_ounused, DEAD, C_LV1},
    {"purge", do_purge, DEAD, C_LV1},
    {"rappend", do_rappend, DEAD, C_LV1},
    {"rblock", do_rblock, DEAD, C_LV1},
    {"rcret", do_rcret, DEAD, C_LV1},
    {"rddesc", do_rddesc, DEAD, C_LV1},
    {"rdelete", do_rdelete, DEAD, C_LV1},
    {"rdesc", do_rdesc, DEAD, C_LV1},
    {"rdflag", do_rdflag, DEAD, C_LV1},
    {"rdoor", do_rdoor, DEAD, C_LV1},
    {"redesc", do_redesc, DEAD, C_LV1},

    {"rexit", do_rexit, DEAD, C_LV1},
    {"rexitrm", do_rexitrm, DEAD, C_LV1},
    {"rflags", do_rflags, DEAD, C_LV1},
    {"rgate", do_rgate, DEAD, C_LV1},
    {"rinit", do_rinit, DEAD, C_LV1},
    {"rkey", do_rkey, DEAD, C_LV1},
    {"rlink", do_rlink, DEAD, C_LV1},
    {"rlinkrm", do_rlinkrm, DEAD, C_LV1},
    {"rlist", do_rlist, DEAD, C_LV1},
    {"rname", do_rname, DEAD, C_LV1},
    {"rsector", do_rsector, DEAD, C_LV1},
    {"rset", do_rset, DEAD, C_LV1},
    {"runused", do_runused, DEAD, C_LV1},
    {"rxchange", do_rxchange, DEAD, C_LV1},
    {"stat", do_stat, DEAD, C_LV1},

    {"tally", do_tally, DEAD, C_LV1},
    {"users", do_users, DEAD, C_LV1},
    {"vmob", do_mvariables, DEAD, C_LV1},
    {"variables", do_variables, DEAD, C_LV1},
    {"zsave", do_zsave, DEAD, C_LV1},

    /* Turf related commands */
    {"turf", do_turf, DEAD, C_LV1},
    {"immturf", do_immturf, DEAD, C_LV1},
    {"setturf", do_setturf, DEAD, C_LV1},

    /*Advanced Builder Level 2:  (mobs and crafts) */
    {"classify", do_classify, DEAD, C_LV2},
    {"clog", do_clog, DEAD, C_LV2},	/* Crafts Log */
    {"craftspc", do_craftspc, DEAD, C_LV2},
    {"cset", do_cset, DEAD, C_LV2},
    {"find", do_find, DEAD, C_LV2},
    //{"freeze", do_freeze, DEAD, C_LV2},
    {"locate", do_locate, DEAD, C_LV2},
    {"mclone", do_mclone, DEAD, C_LV2},
    {"mcopy", do_mcopy, DEAD, C_LV2},
    {"minit", do_minit, DEAD, C_LV2},
    {"mlist", do_mlist, DEAD, C_LV2},
    {"munused", do_munused, DEAD, C_LV2},

    {"mpadd", do_mpadd, DEAD, C_LV2},
    {"mpapp", do_mpapp, DEAD, C_LV2},
    {"mpcmd", do_mpcmd, DEAD, C_LV2},
    {"mpdel", do_mpdel, DEAD, C_LV2},
    {"mpkey", do_mpkey, DEAD, C_LV2},
    {"mpprg", do_mpprg, DEAD, C_LV2},
    {"mpstat", do_mpstat, DEAD, C_LV2},
    {"mptype", do_mptype, DEAD, C_LV2},

    {"aiadd", do_aiadd, DEAD, C_LV2},
    {"aistat", do_aistat, DEAD, C_LV2},
    {"aidel", do_aidel, DEAD, C_LV2},
    {"aitype", do_aitype, DEAD, C_LV2},

    {"opadd", do_opadd, DEAD, C_LV2},
    {"opapp", do_opapp, DEAD, C_LV2},
    {"opcmd", do_opcmd, DEAD, C_LV2},
    {"opdel", do_opdel, DEAD, C_LV2},
    {"opkey", do_opkey, DEAD, C_LV2},
    {"opprg", do_opprg, DEAD, C_LV2},
    {"opstat", do_opstat, DEAD, C_LV2},
    {"optype", do_optype, DEAD, C_LV2},

    {"cpadd", do_cpadd, DEAD, C_LV2},
    {"cpapp", do_cpapp, DEAD, C_LV2},
    {"cpcmd", do_cpcmd, DEAD, C_LV2},
    {"cpdel", do_cpdel, DEAD, C_LV2},
    {"cpkey", do_cpkey, DEAD, C_LV2},
    {"cpprg", do_cpprg, DEAD, C_LV2},
    {"cpstat", do_cpstat, DEAD, C_LV2},
    {"cptype", do_cptype, DEAD, C_LV2},

    {"notes", do_notes, DEAD, C_LV2},
    {"outfit", do_outfit, DEAD, C_LV2},
    {"prog", do_prog, DEAD, C_LV2},
    {"rcap", do_rcap, DEAD, C_LV2},
    {"rxerox", do_rxerox, DEAD, C_LV2},
    {"renviro", do_renviro, DEAD, C_LV2},
    {"rscent", do_rscent, DEAD, C_LV2},
    {"report", do_report, DEAD, C_LV2},
    {"restore", do_restore, DEAD, C_LV2},
    {"rpadd", do_rpadd, DEAD, C_LV2},
    {"rpapp", do_rpapp, DEAD, C_LV2},
    {"rpcmd", do_rpcmd, DEAD, C_LV2},
    {"rpdel", do_rpdel, DEAD, C_LV2},
    {"rpkey", do_rpkey, DEAD, C_LV2},
    {"rpprg", do_rpprg, DEAD, C_LV2},
    {"rpstat", do_rpstat, DEAD, C_LV2},
    {"send", do_immtell, DEAD, C_LV2},
    //{"thaw", do_thaw, DEAD, C_LV2},
    {"transfer", do_transfer, DEAD, C_LV2},
    {"vboards", do_vboards, DEAD, C_LV2},
    {"wearloc", do_wearloc, DEAD, C_LV2},
    {"would", do_would, DEAD, C_LV2},
    {"where", do_where, DEAD, C_LV2},

    /* Basic RPA Level 3: (players) */
    {"addcraft", do_addcraft, DEAD, C_LV3},
    {"becho", do_becho, DEAD, C_LV3},
    {"clan", do_clan, DEAD, C_LV3},
    {"echo", do_echo, DEAD, C_LV3},
    {"job", do_job, DEAD, C_LV3},
    {"last", do_last, DEAD, C_LV3},
    {"openskill", do_openskill, DEAD, C_LV3},
    {"pecho", do_pecho, DEAD, C_LV3},
    {"register", do_register, DEAD, C_LV3},
    {"remcraft", do_remcraft, DEAD, C_LV3},
    {"snoop", do_snoop, DEAD, C_LV3},
    {"summon", do_summon, DEAD, C_LV3},
    {"ticket", do_ticket, DEAD, C_LV3},
    {"wclone", do_wclone, DEAD, C_LV3},
    {"zecho", do_zecho, DEAD, C_LV3},
    {"rend", do_rend, DEAD, C_LV3},	/* object.c */


    /* Advanced RPA Level 4 (advanced players) */
//	{"aggro", do_aggro, DEAD, C_LV4},
    {"alog", do_alog, DEAD, C_LV4},	/* Announcements */
    {"award", do_award, DEAD, C_LV4},
    {"ban", do_ban, DEAD, C_LV4},
    {"broadcast", do_broadcast, DEAD, C_LV4},
    {"deduct", do_deduct, DEAD, C_LV4},
    {"email", do_email, DEAD, C_LV4},
    {"givedream", do_givedream, DEAD, C_LV4},
    {"hedit", do_hedit, DEAD, C_LV4},
    {"hour", do_hour, DEAD, C_LV4},
    {"xlog", do_log, DEAD, C_LV4},
    {"party", do_party, DEAD, C_LV4},
    {"passwd", do_passwd, DEAD, C_LV4},
    {"plog", do_plog, DEAD, C_LV4},
    {"professions", do_professions, DEAD, C_LV4},
    {"rclone", do_rclone, DEAD, C_LV4},
    //{ "refresh",	do_refresh,  DEAD,	  C_LV4 },
    //{"resets", do_resets, DEAD, C_LV4},
    {"role", do_role, DEAD, C_LV4},
    {"saverooms", do_saverooms, DEAD, C_LV4},
    {"shutdown", do_shutdown, DEAD, C_LV4},
    {"stayput", do_stayput, DEAD, C_LV4},

    {"swap", do_swap, DEAD, C_LV4},
    {"unban", do_unban, DEAD, C_LV4},
    {"whap", do_whap, STAND, C_LV4},	/* Joke command */
    {"writings", do_writings, DEAD, C_LV4},
    {"xstink", do_stink, DEAD, C_LV4},
    {"xpoison", do_xpoison, DEAD, C_LV4},
    {"zset", do_zset, DEAD, C_LV4},
    {"sound", do_sound, DEAD, C_LV4}, // Plays a .wav file.
    {"music", do_music, DEAD, C_LV4}, // Plays a .wav file.
    {"flist", do_flist, DEAD, C_LV4},
    {"scents", do_scents, DEAD, C_LV4},
    {"furnishit", do_tableit, DEAD, C_LV4},
    
    /* HRPA Level 5 */
    //{"*", do_fivenet, DEAD, C_LV5},
    {"assign", do_assign, DEAD, C_LV5},	/* Assign to Roster */
    {"cast", do_cast, FIGHT, C_LV5},
    {"gecho", do_gecho, DEAD, C_LV5},
    {"pfile", do_pfile, DEAD, C_LV5},
    {"prepare", do_prepare, FIGHT, C_LV5},
    //{"print", do_print, DEAD, C_LV5},
    {"replace", do_replace, DEAD, C_LV5},
    {"roster", do_roster, DEAD, C_LV5},
    {"test", do_test, DEAD, C_LV5 | C_DEL},
    {"immwatch", do_watch, DEAD, C_LV5},
    {"wizlock", do_wizlock, DEAD, C_LV5},
    {"wmotd", do_wmotd, DEAD, C_LV5}, /* write the MOTD */
    {"wlog", do_wlog, DEAD, C_LV5},

    /* IMP level */
    {"compete", do_compete, DEAD, C_LV4},
    {"csv", do_csv, DEAD, C_LV5},  /* send the user a particular chunk of data */
    {"day", do_day, DEAD, C_LV5},
    {"debug", do_debug, DEAD, C_LV5},
    {"mysql", do_mysql, DEAD, C_LV5},
    {"nuke", do_nuke, DEAD, C_LV5},

    {"rmove", do_rmove, DEAD, C_LV5},
    {"zlife", do_zlife, DEAD, C_LV5},
    {"zlock", do_zlock, DEAD, C_LV5},
    {"zmode", do_zmode, DEAD, C_LV5},
    {"zname", do_zname, DEAD, C_LV5},
 
    {"", NULL, 0, 0}
};

char *fill[] = { "in",
                 "from",
                 "with",
                 "the",
                 "on",
                 "at",
                 "to",
                 "\n"
               };

int
search_block (char *arg, char **list, bool exact)
{
    register int i = 0, l;

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
        *(arg + l) = tolower (*(arg + l));

    if (exact)
    {
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strcmp (arg, *(list + i)))
                return (i);
    }
    else
    {
        if (!l)
            l = 1;			/* Avoid "" to match the first available string */
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strncmp (arg, *(list + i), l))
                return (i);
    }

    return (-1);
}

void
show_to_watchers (CHAR_DATA * ch, char *command)
{
    CHAR_DATA *tch;
    AFFECTED_TYPE *af;
    char buf[MAX_STRING_LENGTH];

    if ((af = get_affect (ch, MAGIC_WATCH1)))
    {
        if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
            affect_remove (ch, af);
        else
        {
            tch = (CHAR_DATA *) af->a.spell.t;
            sprintf (buf, "%s:  %s\n", GET_NAME (ch), command);
            send_to_char (buf, tch);
        }
    }

    if ((af = get_affect (ch, MAGIC_WATCH2)))
    {
        if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
            affect_remove (ch, af);
        else
        {
            tch = (CHAR_DATA *) af->a.spell.t;
            sprintf (buf, "%s:  %s\n", GET_NAME (ch), command);
            send_to_char (buf, tch);
        }
    }

    if ((af = get_affect (ch, MAGIC_WATCH3)))
    {
        if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
            affect_remove (ch, af);
        else
        {
            tch = (CHAR_DATA *) af->a.spell.t;
            sprintf (buf, "%s:  %s\n", GET_NAME (ch), command);
            send_to_char (buf, tch);
        }
    }
}

void
command_interpreter (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };
    char *command_args = '\0', *p = '\0', *social_args = '\0';
    int cmd_level = 0;
    int i = 0, echo = 1;
    AFFECTED_TYPE *craft_affect = NULL;
    AFFECTED_TYPE *af = NULL;
    ALIAS_DATA *alias = NULL;
    extern int second_affect_active;

    if (!ch)
        return;

    *buf = '\0';

    p = argument;

    while (*p == ' ')
        p++;

    if (strchr (p, '%'))
    {
        send_to_char ("Input with the '%' character is not permitted.\n", ch);
        return;
    }

    if (strchr (p, '#') && IS_MORTAL (ch) && strncmp (p, "ge", 2) != 0
            && strncmp (p, "buy", 3) != 0)
    {
        send_to_char ("Input with the '#' character is not permitted.\n", ch);
        return;
    }

    if (IS_MORTAL (ch) && strchr (p, '$'))
    {
        send_to_char ("Input with the '$' character is not permitted.\n", ch);
        return;
    }

    if (!ch->room && vnum_to_room(ch->in_room))
    {
        sprintf(buf, "No ch->room but ch->in_room found: %s in %d doing %s.", ch->tname, ch->in_room, p);
        system_log(buf, true);
        send_to_gods(buf);
        ch->room = vnum_to_room(ch->in_room);
    }

    std::multimap<int, room_prog>::iterator it;
    if (IS_NPC(ch))
        it = mob_prog_list.find(ch->mob->vnum);
    if (IS_NPC(ch) && !get_second_affect (ch, SA_DOANYWAY, 0) && it != mob_prog_list.end())
    {
        if (m_prog(ch, p))
        {
            return;
        }
    }

    std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> pair;

    if (ch->right_hand && !get_second_affect (ch, SA_DOANYWAY, 0))
    {
        pair = obj_prog_list.equal_range(ch->right_hand->nVirtual);
        for (it = pair.first; it != pair.second; it++)
        {
            if (it->second.type != 1 && it->second.type != 3 && it->second.type != 5)
                continue;
            if (o_prog(ch, p, it->second))
                return;
        }
    }
    if (ch->left_hand && !get_second_affect (ch, SA_DOANYWAY, 0))
    {
        pair = obj_prog_list.equal_range(ch->left_hand->nVirtual);
        for (it = pair.first; it != pair.second; it++)
        {
            if (it->second.type != 1 && it->second.type != 3 && it->second.type != 5)
                continue;
            if (o_prog(ch, p, it->second))
                return;
        }
    }
    for (OBJ_DATA *tobj = ch->equip; tobj; tobj = tobj->next_content)
    {
        if (get_second_affect (ch, SA_DOANYWAY, 0))
            break;

        pair = obj_prog_list.equal_range(tobj->nVirtual);
        for (it = pair.first; it != pair.second; it++)
        {
            if (it->second.type != 2 && it->second.type != 3 && it->second.type != 5)
                continue;
            if (o_prog(ch, p, it->second))
                return;
        }

    }

    if (ch->room)
    {
        for (OBJ_DATA *tobj = ch->room->contents; tobj; tobj = tobj->next_content)
        {
            if (get_second_affect (ch, SA_DOANYWAY, 0))
                break;

            pair = obj_prog_list.equal_range(tobj->nVirtual);
            for (it = pair.first; it != pair.second; it++)
            {
                if (it->second.type != 4 && it->second.type != 5)
                    continue;
                if (o_prog(ch, p, it->second))
                    return;
            }

        }
    }

    if (ch->room)
    {
        for (CHAR_DATA *temp_char = ch->room->people; temp_char; temp_char = temp_char->next_in_room)
        {
            if (get_second_affect (ch, SA_DOANYWAY, 0))
                break;

            if (temp_char == ch)
                continue;


            if (temp_char->right_hand)
            {
                pair = obj_prog_list.equal_range(temp_char->right_hand->nVirtual);
                for (it = pair.first; it != pair.second; it++)
                {
                    if (it->second.type != 5)
                        continue;
                    if (o_prog(ch, p, it->second))
                        return;
                }
            }
            if (temp_char->left_hand)
            {
                pair = obj_prog_list.equal_range(temp_char->left_hand->nVirtual);
                for (it = pair.first; it != pair.second; it++)
                {
                    if (it->second.type != 5)
                        continue;
                    if (o_prog(ch, p, it->second))
                        return;
                }
            }

            for (OBJ_DATA *tobj = temp_char->equip; tobj; tobj = tobj->next_content)
            {
                if (get_second_affect (ch, SA_DOANYWAY, 0))
                    break;

                pair = obj_prog_list.equal_range(tobj->nVirtual);
                for (it = pair.first; it != pair.second; it++)
                {
                    if (it->second.type != 5)
                        continue;
                    if (o_prog(ch, p, it->second))
                        return;
                }
            }

            if (!IS_NPC(temp_char))
                continue;

            pair = mob_prog_list.equal_range(temp_char->mob->vnum);

            for (it = pair.first; it != pair.second; ++it)
            {
                if (m_prog(ch, p, it->second))
                    return;
            }
        }
    }

    if (ch->room && !(get_second_affect (ch, SA_DOANYWAY, 0)) && ch->room->prg && r_program (ch, p))
    {
        if (!IS_NPC (ch)
                || (ch->descr() && (ch->pc && str_cmp (ch->pc->account_name, "Guest"))))
        {
            player_log (ch, "[RPROG]", p);
        }
        if (!IS_SET (commands[i].flags, C_NWT))
            show_to_watchers (ch, argument);
        return;
    }

    if (!IS_MORTAL (ch) && !str_cmp (argument, "sho wl"))
    {
        send_to_char
        ("Heh heh. Glad I added in this check, aren't we? No shouting for you.\n",
         ch);
        return;
    }

    if (ch->descr())
    {
        last_descriptor = ch->descr();
        sprintf (full_last_command, "Last Command Issued, by %s [%d]: %s",
                 ch->tname, ch->in_room, argument);
        sprintf (last_command, "%s", argument);
    }

    social_args = argument;

    command_args = one_argument (argument, buf);

    if (!*buf)
        return;

    while (*command_args == ' ')
        command_args++;

    if (ch->pc && !GET_FLAG (ch, FLAG_ALIASING))
    {

        if ((alias = is_alias (ch, buf)))
        {

            ch->flags |= FLAG_ALIASING;

            while (alias)
            {

                command_interpreter (ch, alias->line);

                if (ch->deleted)
                    return;

                alias = alias->next_line;
            }

            ch->flags &= ~FLAG_ALIASING;

            return;
        }
    }

    for (i = 1; *commands[i].command; i++)
        if (is_abbrev (buf, commands[i].command))
            break;

    if ((craft_affect = is_craft_command (ch, argument)))
        i = 0;

    if (IS_SET (commands[i].flags, C_IMP))
        cmd_level = 6;
    else if (IS_SET (commands[i].flags, C_LV5))
        cmd_level = 5;
    else if (IS_SET (commands[i].flags, C_LV4))
        cmd_level = 4;
    else if (IS_SET (commands[i].flags, C_LV3))
        cmd_level = 3;
    else if (IS_SET (commands[i].flags, C_LV2))
        cmd_level = 2;
    else if (IS_SET (commands[i].flags, C_LV1))
        cmd_level = 1;

    if (IS_SET (commands[i].flags, C_GDE)
            && (IS_NPC (ch) || (!ch->pc->is_guide && !ch->pc->level)))
    {
        send_to_char ("Eh?\n\r", ch);
        return;
    }
    /*
    Need to pass the CHAR_DATA pointer for the person who made the command and modify
    the following line to test the commanding char's trust against the trust level for
    the command.  - Methuselah
    */

    if ((!*commands[i].command)
            || (cmd_level > GET_TRUST (ch)))
    {
        if (!social (ch, argument))
        {

            echo = number (1, 9);
            if (echo == 1)
                send_to_char ("Eh?\n\r", ch);
            else if (echo == 2)
                send_to_char ("Huh?\n\r", ch);
            else if (echo == 3)
                send_to_char ("I'm afraid that just isn't possible...\n\r", ch);
            else if (echo == 4)
                send_to_char ("I don't recognize that command.\n\r", ch);
            else if (echo == 5)
                send_to_char ("What?\n\r", ch);
            else if (echo == 6)
                send_to_char
                ("Perhaps you should try typing it a different way?\n\r", ch);
            else if (echo == 7)
                send_to_char
                ("Try checking your typing - I don't recognize it.\n\r", ch);
            else if (echo == 8)
                send_to_char
                ("That isn't a recognized command, craft, or social.\n\r", ch);
            else
                send_to_char ("Hmm?\n\r", ch);
        }
        else
        {
            if (!IS_SET (commands[i].flags, C_NWT))
                show_to_watchers (ch, argument);
        }
        return;
    }

    if (ch->stun)
    {
        send_to_char ("You're still reeling.\n", ch);
        return;
    }

    if (ch->roundtime)
    {
        sprintf (buf, "You'll need to wait another %d seconds.\n",
                 ch->roundtime);
        send_to_char (buf, ch);
        return;
    }

    if (IS_SET (commands[i].flags, C_WLK) &&
            (ch->moves || GET_FLAG (ch, FLAG_LEAVING)
             || GET_FLAG (ch, FLAG_ENTERING)))
    {
        send_to_char ("Stop traveling first.\n\r", ch);
        return;
    }

    if (IS_SET (commands[i].flags, C_MNT) && IS_RIDER (ch))
    {
        send_to_char ("Get off your mount first.\n", ch);
        return;
    }

	if ((ch->in_room == GRUNGE_SMALL1 && grunge_arena_first_fighters) ||
		(ch->in_room == GRUNGE_SMALL2 && grunge_arena_second_fighters))
	{
		if (!str_cmp(commands[i].command, "drop") ||
			!str_cmp(commands[i].command, "give") ||
			!str_cmp(commands[i].command, "wear") ||
			!str_cmp(commands[i].command, "set") ||
			!str_cmp(commands[i].command, "throw") ||
			!str_cmp(commands[i].command, "stop") ||
			!str_cmp(commands[i].command, "junk") ||
			!str_cmp(commands[i].command, "sit") ||
			!str_cmp(commands[i].command, "rest") ||
			!str_cmp(commands[i].command, "sleep") ||
			!str_cmp(commands[i].command, "remove") ||
			!str_cmp(commands[i].command, "flee"))
		{
			send_to_char("You get the distinct impression that doing anything that could even be interpreted \nas throwing this fight will get you killed, at best.\n", ch);
			return;
		}
	}

	if (IS_NPC(ch) && ch->mob->controller && !str_cmp(lookup_race_variable(ch->race, RACE_NAME), "robot"))
    {
        // Here we add in all of the lovely checks to make sure our robot doesn't do anything silly,
        // depending on the various features it has. Some ideas include:

		if (!str_cmp(commands[i].command, "eat") ||
			!str_cmp(commands[i].command, "drink") ||
		    !str_cmp(commands[i].command, "sip") ||
			!str_cmp(commands[i].command, "quit") ||
			!str_cmp(commands[i].command, "invite") ||
			!str_cmp(commands[i].command, "focus") ||
			!str_cmp(commands[i].command, "camp") ||
			!str_cmp(commands[i].command, "think") ||
			!str_cmp(commands[i].command, "feel") ||
			!str_cmp(commands[i].command, "news") ||
			!str_cmp(commands[i].command, "ownership") ||
			!str_cmp(commands[i].command, "sell") ||
			!str_cmp(commands[i].command, "value") ||
			!str_cmp(commands[i].command, "barter") ||
			!str_cmp(commands[i].command, "typo") ||
			!str_cmp(commands[i].command, "bug") ||
			!str_cmp(commands[i].command, "petition") ||
			!str_cmp(commands[i].command, "plan") ||
			!str_cmp(commands[i].command, "goal") ||
			!str_cmp(commands[i].command, "nominate") ||
			!str_cmp(commands[i].command, "mount") ||
			!str_cmp(commands[i].command, "teach") ||
			!str_cmp(commands[i].command, "journal") ||
			!str_cmp(commands[i].command, "jread") ||
			!str_cmp(commands[i].command, "jwrite") ||
			!str_cmp(commands[i].command, "write") ||
			!str_cmp(commands[i].command, "jerase") ||
			!str_cmp(commands[i].command, "jread") ||
			!str_cmp(commands[i].command, "ignore") ||
			!str_cmp(commands[i].command, "rest") ||
			!str_cmp(commands[i].command, "sleep") ||
			!str_cmp(commands[i].command, "wear") ||
			!str_cmp(commands[i].command, "buy") )
		{
			send_to_char ("#6KRZZ#3V12#6:#0 UNKNOWN COMMAND RECEIVED!\n\r", ch);
			return;
		}

        if(!str_cmp(commands[i].command, "mend"))
        {
            if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "repair")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "repair")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "repair")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "repair")) )
			{
				;
			} else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
        }

        if( !str_cmp(commands[i].command, "command") )
        {
            if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "repair")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "repair")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "repair")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "repair")) )
			{
            ;
			} else {
                send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
        }

		if (!str_cmp(commands[i].command, "talk") ||
			!str_cmp(commands[i].command, "tell") ||
			!str_cmp(commands[i].command, "say") ||
			!str_cmp(commands[i].command, "whisper") ||
			!str_cmp(commands[i].command, "shout") ||
			!str_cmp(commands[i].command, "yell") ||
			!str_cmp(commands[i].command, "alert") ||
			!str_cmp(commands[i].command, "transmit") ||
			!str_cmp(commands[i].command, "sing"))
		{
			if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "voice")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "voice")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "voice")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "voice")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 VOCAL MODULE NOT INSTALLED!\n\r", ch);
				return;
			}
		}

        if (!str_cmp(commands[i].command, "load") ||
			!str_cmp(commands[i].command, "unload") ||
			!str_cmp(commands[i].command, "aim") ||
            !str_cmp(commands[i].command, "shoot") )
        {
            if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "firearms")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "firearms")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "firearms")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "firearms")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
        }

	//	if (!str_cmp(commands[i].command, "guard") ||
	//		!str_cmp(commands[i].command, "rescue") ||
	        if (	!str_cmp(commands[i].command, "feint") ||
			!str_cmp(commands[i].command, "bash") ||
			!str_cmp(commands[i].command, "ward") ||
			!str_cmp(commands[i].command, "throw") ||
			!str_cmp(commands[i].command, "subdue") ||
			!str_cmp(commands[i].command, "strike"))
		{
			if ((ch->d_feat1 && !str_cmp(ch->d_feat1, "combat")) ||
				(ch->d_feat2 && !str_cmp(ch->d_feat2, "combat")) ||
				(ch->d_feat3 && !str_cmp(ch->d_feat3, "combat")) ||
				(ch->d_feat4 && !str_cmp(ch->d_feat4, "combat")))
			{
				;
			}
			else
			{
				send_to_char ("#6KZNP#3V13#6:#0 UNABLE TO PROCESS COMMAND!\n\r", ch);
				return;
			}
		}
    }

    if (commands[i].min_position > GET_POS (ch))
    {
        switch (GET_POS (ch))
        {
        case DEAD:
            if (IS_MORTAL (ch))
            {
                send_to_char ("You are dead.  You can't do that.\n\r", ch);
                return;
            }
        case UNCON:
        case MORT:
            send_to_char ("You're seriously wounded and unconscious.\n\r", ch);
            return;

        case STUN:
            send_to_char ("You're too stunned to do that.\n\r", ch);
            return;

        case SLEEP:
            send_to_char ("You can't do that while sleeping.\n\r", ch);
            return;

        case REST:
            send_to_char ("You can't do that while lying down.\n\r", ch);
            return;

        case SIT:
            send_to_char ("You can't do that while sitting.\n\r", ch);
            return;

        case FIGHT:
            send_to_char ("No way! You are fighting for your life!\n\r", ch);
            return;
        }

        return;
    }

    if (!IS_NPC (ch) && ch->pc->create_state == STATE_DIED &&
            !IS_SET (commands[i].flags, C_DOA))
    {
        send_to_char ("You can't do that when you're dead.\n\r", ch);
        return;
    }

    if (!IS_SET (commands[i].flags, C_BLD) && is_blind (ch))
    {
        if (get_equip (ch, WEAR_BLINDFOLD))
            send_to_char ("You can't do that while blindfolded.\n\r", ch);
        else
            send_to_char ("You can't do that while blind.\n\r", ch);
        return;
    }

    if ((af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)) &&
            !IS_SET (commands[i].flags, C_PAR) && IS_MORTAL (ch))
    {
        send_to_char ("You can't move.\n", ch);
        return;
    }

    if (IS_SUBDUEE (ch) && !IS_SET (commands[i].flags, C_SUB) && !cmd_level)
    {
        act ("$N won't let you.", false, ch, 0, ch->subdue, TO_CHAR);
        return;
    }

    /* Most commands break delays */

    if (ch->delay && !IS_SET (commands[i].flags, C_DEL))
	{
		if (ch->delay_type == DEL_TRADE)
		{
			send_to_char ("That action will halt your current trade activity - type #6stop#0 to halt.\n", ch);
			return;
		}
		else
		{
			break_delay (ch);
		}
	}

    /* Send this command to the log */
    if (!second_affect_active && (!IS_NPC (ch) || ch->descr()))
    {
        if (IS_SET (commands[i].flags, C_NLG))
            ;
        else if (i > 0)
        {
            /* Log craft commands separately. */
            if (!str_cmp (commands[i].command, "."))
                player_log (ch, "say", command_args);
            else if (!str_cmp (commands[i].command, ","))
                player_log (ch, "emote", command_args);
            else if (!str_cmp (commands[i].command, ":"))
                player_log (ch, "emote", command_args);
            else if (!str_cmp (commands[i].command, ";"))
                player_log (ch, "wiznet", command_args);
            else
                player_log (ch, commands[i].command, command_args);
        }
    }

    if (IS_MORTAL (ch) && get_affect (ch, MAGIC_HIDDEN) &&
            !IS_SET (commands[i].flags, C_HID) &&
            skill_level (ch, SKILL_SNEAK, 0) < number (1, SKILL_CEILING) &&
            would_reveal (ch))
    {
        remove_affect_type (ch, MAGIC_HIDDEN);
        act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        act ("Your actions have compromised your concealment.", true, ch, 0, 0,
             TO_CHAR);
    }

    /* Execute command */

    if (!IS_SET (commands[i].flags, C_NWT))
        show_to_watchers (ch, social_args);

    if (!i)			/* craft_command */
        craft_command (ch, command_args, craft_affect);
    else

        (*commands[i].proc) (ch, command_args, 0);

    last_descriptor = NULL;
}

void
argument_interpreter (char *argument, char *first_arg, char *second_arg)
{
    int look_at, found, begin;

    found = begin = 0;

    do
    {
        /* Find first non blank */
        for (; *(argument + begin) == ' '; begin++);

        /* Find length of first word */
        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

            /* Make all letters lower case,
               AND copy them to first_arg */
            *(first_arg + look_at) = tolower (*(argument + begin + look_at));

        *(first_arg + look_at) = '\0';
        begin += look_at;

    }
    while (fill_word (first_arg));

    do
    {
        /* Find first non blank */
        for (; *(argument + begin) == ' '; begin++);

        /* Find length of first word */
        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

            /* Make all letters lower case,
               AND copy them to second_arg */
            *(second_arg + look_at) = tolower (*(argument + begin + look_at));

        *(second_arg + look_at) = '\0';
        begin += look_at;

    }
    while (fill_word (second_arg));
}

int
is_number (const char *str)
{
    int look_at;

    if (*str == '\0')
        return (0);

    for (look_at = 0; *(str + look_at) != '\0'; look_at++)
        if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
            return (0);
    return (1);
}

int is_number2( const char * str, int def )
{
	std::stringstream ss( str );
	int value;

	if ( !( ss >> value ) )
		value = def;

	delete ss;

	return value;
}

int
fill_word (char *argument)
{
    return (search_block (argument, ::fill, true) >= 0);
}

/* determine if a given string is an abbreviation of another */
int
is_abbrev (const char *arg1, const char *arg2)
{
    if (!*arg1)
        return (0);

    for (; *arg1; arg1++, arg2++)
        if (tolower (*arg1) != tolower (*arg2))
            return (0);

    return (1);
}

/* case-sensitive; determine if a given string is an abbreviation of another */
int
is_abbrevc (const char *arg1, const char *arg2)
{
    if (!*arg1)
        return (0);

    for (; *arg1; arg1++, arg2++)
        if (*arg1 != *arg2)
            return (0);

    return (1);
}

/* return first 'word' plus trailing substring of input string */
void
half_chop (char *string, char *arg1, char *arg2)
{
    for (; isspace (*string); string++);

    for (; !isspace (*arg1 = *string) && *string; string++, arg1++);

    *arg1 = '\0';

    for (; isspace (*string); string++);

    for (; (*arg2 = *string); string++, arg2++);
}


/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */

SOCIAL_DATA *social_messages = NULL;
static int list_top = -1;

char *
fread_action (FILE * fl)
{
    char buf[MAX_STRING_LENGTH] = { '\0' };

    fgets (buf, MAX_STRING_LENGTH, fl);
    if (feof (fl))
    {
        system_log ("Fread_action() - unexpected EOF!", true);
        abort ();
    }

    if (*buf == '#')
        return 0;

    buf[strlen (buf) - 1] = '\0';

    return add_hash (buf);
}

void
boot_social_messages (void)
{
    FILE *fl;
    char *social_command;
    int hide;
    int min_pos;

    if (!(fl = fopen (SOCMESS_FILE, "r")))
    {
        perror ("boot_social_messages");
        abort ();
    }

#define MAX_SOCIALS		200

    CREATE (social_messages, SOCIAL_DATA, MAX_SOCIALS);

    for (list_top = 0;; list_top++)
    {

        if (!(social_command = fread_action (fl)))
            break;

        fscanf (fl, " %d ", &hide);
        fscanf (fl, " %d \n", &min_pos);

        if (list_top >= MAX_SOCIALS - 1)
        {
            break;
        }

        social_messages[list_top].social_command = social_command;
        social_messages[list_top].hide = hide;
        social_messages[list_top].min_victim_position = min_pos;
        social_messages[list_top].char_no_arg = fread_action (fl);
        social_messages[list_top].others_no_arg = fread_action (fl);
        social_messages[list_top].char_found = fread_action (fl);

        /* if no char_found, the rest is to be ignored */

        if (!social_messages[list_top].char_found)
            continue;

        social_messages[list_top].others_found = fread_action (fl);
        social_messages[list_top].vict_found = fread_action (fl);
        social_messages[list_top].not_found = fread_action (fl);
        social_messages[list_top].char_auto = fread_action (fl);
        social_messages[list_top].others_auto = fread_action (fl);
    }

    fclose (fl);
}

int
social (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    SOCIAL_DATA *action;
    CHAR_DATA *victim;
    int i;

    argument = one_argument (argument, buf);

    if (ch->room->vnum == AMPITHEATRE && IS_MORTAL (ch))
    {
        if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
                !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
        {
            send_to_char
            ("You decide against making a commotion. PETITION to request to speak.\n",
             ch);
            return 0;
        }
    }

    for (i = 0; i < list_top; i++)
    {
        if (is_abbrev (buf, social_messages[i].social_command))
            break;
    }

    if (i == list_top)
        return 0;

    action = &social_messages[i];

    if (action->char_found)
        one_argument (argument, buf);
    else
        *buf = '\0';

    if (!*buf)
    {
        send_to_char (action->char_no_arg, ch);
        send_to_char ("\n\r", ch);
        if (action->others_no_arg)
            act (action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);

        return 1;
    }

    if (!(victim = get_char_room_vis (ch, buf)))
    {
        send_to_char (action->not_found, ch);
        send_to_char ("\n\r", ch);
    }

    else if (victim == ch)
    {
        send_to_char (action->char_auto, ch);
        send_to_char ("\n\r", ch);
        if (action->others_auto)
            act (action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
    }

    else if (GET_POS (victim) < action->min_victim_position)
        act ("$N is not in a proper position for that.",
             false, ch, 0, victim, TO_CHAR);

    else
    {
        if (action->char_found)
            act (action->char_found, 0, ch, 0, victim, TO_CHAR);

        if (action->others_found)
            act (action->others_found, action->hide, ch, 0, victim, TO_NOTVICT);

        if (action->vict_found)
            act (action->vict_found, action->hide, ch, 0, victim, TO_VICT);
    }

    return 1;
}

void
do_commands (CHAR_DATA * ch, char *argument, int cmd)
{
    int col_no = 0;
    int cmd_no;
    int cmd_level;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    for (cmd_no = 0; *commands[cmd_no].command; cmd_no++)
    {

        cmd_level = 0;

        if (IS_SET (commands[cmd_no].flags, C_LV5))
            cmd_level = 5;
        else if (IS_SET (commands[cmd_no].flags, C_LV4))
            cmd_level = 4;
        else if (IS_SET (commands[cmd_no].flags, C_LV3))
            cmd_level = 3;
        else if (IS_SET (commands[cmd_no].flags, C_LV2))
            cmd_level = 2;
        else if (IS_SET (commands[cmd_no].flags, C_LV1))
            cmd_level = 1;

        if (cmd_level)
            continue;

        if (cmd_level > GET_TRUST (ch))
            continue;

        if (IS_SET (commands[cmd_no].flags, C_XLS))
            continue;

        sprintf (buf + strlen (buf), "%-9.9s ", commands[cmd_no].command);

        if (++col_no >= 7)
        {
            strcat (buf, "\n\r");
            send_to_char (buf, ch);
            *buf = '\0';
            col_no = 0;
        }
    }

    if (*buf)
    {
        strcat (buf, "\n\r");
        send_to_char (buf, ch);
    }
}

void
do_immcommands (CHAR_DATA * ch, char *argument, int cmd)
{
    int col_no = 0;
    int cmd_no;
    int cmd_level;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    for (cmd_no = 0; *commands[cmd_no].command; cmd_no++)
    {

        cmd_level = 0;

        if (IS_SET (commands[cmd_no].flags, C_LV5))
            cmd_level = 5;
        else if (IS_SET (commands[cmd_no].flags, C_LV4))
            cmd_level = 4;
        else if (IS_SET (commands[cmd_no].flags, C_LV3))
            cmd_level = 3;
        else if (IS_SET (commands[cmd_no].flags, C_LV2))
            cmd_level = 2;
        else if (IS_SET (commands[cmd_no].flags, C_LV1))
            cmd_level = 1;

        if (!cmd_level)
            continue;

        if (cmd_level > GET_TRUST (ch))
            continue;

        sprintf (buf + strlen (buf), "%-11.11s ", commands[cmd_no].command);

        if (++col_no >= 6)
        {
            strcat (buf, "\n\r");
            send_to_char (buf, ch);
            *buf = '\0';
            col_no = 0;
        }
    }

    if (*buf)
    {
        strcat (buf, "\n\r");
        send_to_char (buf, ch);
    }
}
