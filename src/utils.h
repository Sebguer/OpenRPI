//////////////////////////////////////////////////////////////////////////////
//
/// utils.h - Inline Utility Macros
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2005-2006 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
/// URL: http://www.middle-earth.us
//
/// May includes portions derived from Harshlands
/// Authors: Charles Rand (Rassilon)
/// URL: http://www.harshlands.net
//
/// May include portions derived under license from DikuMUD Gamma (0.0)
/// which are Copyright (C) 1990, 1991 DIKU
/// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
///          Tom Madson (noop@freja.diku.dk)
///          Katja Nyboe (katz@freja.diku.dk)
///          Michael Seifert (seifert@freja.diku.dk)
///          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _rpie_utils_h_
#define _rpie_utils_h_

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = toupper(*(st)), st)
#define LOW(st)  (*(st) = tolower(*(st)), st)

#define MAKE_STRING(msg) (((std::ostringstream&) (std::ostringstream() << std::boolalpha << msg)).str())

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) alloc ((number) * sizeof(type), 16)))\
		{ perror("CREATE: alloc failure"); abort(); } } while(0)
#define IS_SET(flag,bit)  ((flag) & (bit))
#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->affected_by, (skill)) )

#define IS_NIGHT ( !sun_light )

#define IS_LIGHT(room)  !is_dark (room)

#define TOGGLE_BIT(var,bit)  ((var) = (var) ^ (bit) )
#define TOGGLE(flag, bit) { if ( IS_SET (flag, bit) ) \
                               flag &= ~bit; \
                            else \
                               flag |= bit; \
			   }

// You can see if...

// 1. it is light in the room or you've got infravision or you've got goggles
// AND
// 2. the object isn't invisible or you've got see_invisibility.
// AND
// 3. the object isn't hidden or you're grouped to the object.
// AND
// 4. You're not blind

#define CAN_SEE(sub, obj) can_see(sub, obj)

#define HSHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "his" : "her") : "its") : "its")

#define HSSH(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "he" : "she") : "it") : "it")

#define HMHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "him" : "her") : "it") : "it")
#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch) (IS_SET((ch)->act, ACT_ISNPC))

#define GET_TRUST(ch)	(get_trust (ch))
#define IS_IMPLEMENTOR(ch)	( GET_TRUST(ch) > 5 )
#define IS_GUIDE(ch)	(!IS_NPC(ch) ? (IS_MORTAL(ch) && !IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? ch->pc->is_guide : 0) : 0)
#define IS_NEWBIE(ch)	(!is_newbie(ch) ? 0 : 1)
#define IS_MORTAL(ch)	(!GET_TRUST(ch))
#define GET_POS(ch)     ((ch)->position)
#define GET_COND(ch, i) ((ch)->conditions[(i)])
#define GET_NAME(ch)    ((ch)->tname)
#define GET_NAMES(ch)    ((ch)->name)
#define GET_AGE(ch)     (age(ch).year)
#define GET_STR(ch)     ((ch)->tmp_str)
#define GET_DEX(ch)     ((ch)->tmp_dex)
#define GET_INT(ch)     ((ch)->tmp_intel)
#define GET_WIL(ch)		((ch)->tmp_wil)
#define GET_AUR(ch)		((ch)->tmp_aur)
#define GET_CON(ch)     ((ch)->tmp_con)
#define GET_AGI(ch)		((ch)->tmp_agi)
#define GET_AC(ch)      ((ch)->armor)
#define GET_HIT(ch)     ((ch)->hit)
#define GET_MAX_HIT(ch) ((ch)->max_hit)
#define GET_MOVE(ch)    ((ch)->move)
#define GET_MAX_MOVE(ch) ((ch)->max_move)
#define GET_SHOCK(ch)    ((ch)->shock)
#define GET_MAX_SHOCK(ch) ((ch)->max_shock)
#define GET_CASH(ch)    ((ch)->cash)
#define GET_SEX(ch)     ((ch)->sex)
#define GET_SPEAKS(ch)  ((ch)->speaks)
#define GET_OFFENSE(ch) ((ch)->offense)
#define GET_DAMROLL(ch) ((ch)->mob->damroll)
#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)
#define WAIT_STATE(ch, cycle)  (((ch)->descr()) ? (ch)->descr()->wait = (cycle) : 0)
#define GET_FLAG(ch,flag) (IS_SET ((ch)->flags, flag))

/* Object And Carry related macros */

#define LOAD_COLOR(obj, vnum)	load_colored_object(vnum, obj->var_color[0], obj->var_color[1], obj->var_color[2], obj->var_color[3], obj->var_color[4], \
				                       		   obj->var_color[5], obj->var_color[6], obj->var_color[7], obj->var_color[8], obj->var_color[9])

#define LOAD_EXACT_COLOR(obj, vnum)	load_exact_colored_object(vnum, obj->var_color[0], obj->var_color[1], obj->var_color[2], obj->var_color[3], obj->var_color[4], \
				                       						        obj->var_color[5], obj->var_color[6], obj->var_color[7], obj->var_color[8], obj->var_color[9], \
																	obj->var_cat[0], obj->var_cat[1], obj->var_cat[2], obj->var_cat[3], obj->var_cat[4], \
				                       						        obj->var_cat[5], obj->var_cat[6], obj->var_cat[7], obj->var_cat[8], obj->var_cat[9])

#define CAN_SEE_OBJ(sub, obj)	can_see_obj (sub, obj)

#define IS_OBJ_VIS(sub, obj)										\
	( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) || 	\
	     get_affect (sub, MAGIC_AFFECT_SEE_INVISIBLE) ) &&					\
		 !is_blind (sub))                                           \
         || obj->location == WEAR_BLINDFOLD                         \
	     || !IS_MORTAL(sub))

#define GET_MATERIAL_TYPE(obj) (determine_material(obj))
#define GET_KEEPER_MATERIAL_TYPE(obj) (determine_keeper_material(obj))

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))
#define IS_WEARABLE(obj) ((obj)->obj_flags.wear_flags &            \
				(ITEM_WEAR_BODY | ITEM_WEAR_LEGS | ITEM_WEAR_ARMS))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)
#define OBJ_MASS(obj) obj_mass(obj)

// Used by Books and EBooks
#define GET_PAGE_OVAL(obj) get_page_oval(obj)
#define GET_NEXT_WRITE_OVAL(obj) get_next_write_oval(obj)
#define IS_BOOK(obj) is_book(obj)
#define IS_TEARABLE(obj) is_tearable(obj)
#define USES_BOOK_CODE(obj) uses_book_code(obj)

#define CAN_CARRY_W(ch) calc_lookup (ch, REG_MISC, MISC_MAX_CARRY_W)
#define CAN_CARRY_N(ch) (IS_MOUNT (ch) ? 0 : calc_lookup (ch, REG_MISC, MISC_MAX_CARRY_N))

#define IS_CARRYING_W(ch) carrying(ch)
#define IS_CARRYING_N(ch) ((ch)->carry_items)

#define IS_ENCUMBERED(ch) (GET_STR (ch) * enc_tab [1].str_mult_wt < IS_CARRYING_W (ch))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_MERCHANT(mob) (IS_SET((mob)->hmflags,HM_KEEPER))

#define IS_DIRECT(obj) (IS_SET(obj->o.firearm.bits, GUN_DIRECT_ONE) ||  \
                          IS_SET(obj->o.firearm.bits, GUN_DIRECT_FIVE) || \
                          IS_SET(obj->o.firearm.bits, GUN_DIRECT_SIX))

#define IS_SLING(obj) ((GET_ITEM_TYPE(obj) == ITEM_FIREARM) && (obj->o.firearm.use_skill == SKILL_AIM))


/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)  (CAN_SEE((vict), (ch)) ? \
	char_short((ch)) : "someone")

/* #define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")
*/

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	obj_short_desc (obj) : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define IS_OUTSIDE(ch) (!IS_SET((ch)->room->room_flags,INDOORS) && ch->room->sector_type != SECT_UNDERWATER)

#define EXIT(ch, door)  ((ch->room) ? (ch)->room->dir_option[door] : NULL)

#define CAN_GO(ch, door) (EXIT(ch,door)  &&  (EXIT(ch,door)->to_room != NOWHERE) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_FLEE_SOMEWHERE(ch) (CAN_GO(ch, NORTH) || CAN_GO(ch, SOUTH) || \
								CAN_GO(ch, EAST) || CAN_GO(ch, WEST))
#define FLOAT_ONLY(room) ((room)->sector_type == SECT_FREEFALL)

#define IS_FLOATING(ch) FLOAT_ONLY((ch)->room)

#define IS_SUFFOCATING(ch) (IS_FLOATING(ch) && IS_MORTAL(ch)) && \
						   (get_affect (ch, AFFECT_CHOKING) && \
						   get_affect (ch, AFFECT_CHOKING)->a.spell.duration <= 0)

#define SWIM_ONLY(room) ((room)->sector_type == SECT_OCEAN || \
					 	(room)->sector_type == SECT_REEF ||  \
						(room)->sector_type == SECT_RIVER || \
						(room)->sector_type == SECT_LAKE || \
						(room)->sector_type == SECT_UNDERWATER || \
						is_room_affected (room->affects, MAGIC_ROOM_FLOOD) )

#define IS_SWIMMING(ch) SWIM_ONLY((ch)->room)

#define IS_DROWNING(ch) (IS_SWIMMING(ch) && IS_MORTAL(ch)) && \
				(get_affect (ch, AFFECT_HOLDING_BREATH) && \
				get_affect (ch, AFFECT_HOLDING_BREATH)->a.spell.duration <= 0)

#define IS_FROZEN(zone) (IS_SET(zone_table[(zone)].flags,Z_FROZEN))

#define IS_SUBDUEE(ch) (is_he_here (ch, (ch)->subdue, 0) && \
                        GET_FLAG (ch, FLAG_SUBDUEE))
#define IS_SUBDUER(ch) (is_he_here (ch, (ch)->subdue, 0) && \
                        GET_FLAG (ch, FLAG_SUBDUER))

#define IS_MOUNT(ch) (IS_SET (ch->act, ACT_MOUNT))

#define IS_RIDER(ch) (is_he_here (ch, (ch)->mount, 0) && \
                      !IS_SET (ch->act, ACT_MOUNT))
#define IS_RIDEE(ch) (is_he_here (ch, (ch)->mount, 0) && \
                      IS_SET (ch->act, ACT_MOUNT))

#define IS_HITCHER(ch) (is_he_here (ch, (ch)->hitchee, 0) &&	\
						ch->hitchee->hitcher == ch)
#define IS_HITCHEE(ch) (is_he_here (ch, (ch)->hitcher, 0) &&	\
						ch->hitcher->hitchee == ch)

#define IS_TABLE(obj) ((GET_ITEM_TYPE (obj) == ITEM_CONTAINER && \
                        IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE)) ||  \
                       (GET_ITEM_TYPE (obj) == ITEM_COVER && \
                        IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))) \

#define IS_FURNISH(obj) (IS_TABLE(obj) || IS_SET (obj->obj_flags.extra_flags, ITEM_FURNISH))


#define IS_ELECTRIC(obj) (is_electric (obj))


#define SEND_TO_Q(messg, desc)  write_to_q ((messg), (desc) ? &(desc)->output : NULL)

#ifdef NOVELL
#define sigmask(m) ((unsigned long) 1 << ((m) - 1 ))
#endif

#endif // _rpie_utils_h_

