/*	SCCS Id: @(#)objnam.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

/* "an uncursed greased partly eaten guardian naga hatchling [corpse]" */
#if 0 /*JP*/
#define PREFIX	80	/* (56) */
#else
/* 「呪われていない油の塗られた食べかけのクロマティック・ドラゴン(の死体)」*/
#define PREFIX	100
#endif
#define SCHAR_LIM 127
#define NUMOBUF 12

STATIC_DCL char *FDECL(strprepend,(char *,const char *));
#ifdef OVLB
static boolean FDECL(wishymatch, (const char *,const char *,BOOLEAN_P));
#endif
static char *NDECL(nextobuf);
static void FDECL(add_erosion_words, (struct obj *, char *));
static char *FDECL(substitute, (char *,     char *,     char *));
static char *FDECL(transpose, (char *buf,char *));
static char *FDECL(delete, (char *, char *str));
static int FDECL(digit_8, (int));
static int FDECL(atoi_8, (const char *));

struct Jitem {
	int item;
	const char *name;
};

/* true for gems/rocks that should have " stone" appended to their names */
#define GemStone(typ)	(typ == FLINT ||				\
			 (objects[typ].oc_material == GEMSTONE &&	\
			  (typ != DILITHIUM_CRYSTAL && typ != RUBY &&	\
			   typ != DIAMOND && typ != SAPPHIRE &&		\
			   typ != BLACK_OPAL && 	\
			   typ != EMERALD && typ != OPAL)))

#ifndef OVLB

STATIC_DCL struct Jitem Japanese_items[];

#else /* OVLB */

STATIC_OVL struct Jitem Japanese_items[] = {
	{ SHORT_SWORD, "wakizashi" },
	{ BROADSWORD, "ninja-to" },
	{ FLAIL, "nunchaku" },
	{ GLAIVE, "naginata" },
	{ LOCK_PICK, "osaku" },
	{ WOODEN_HARP, "koto" },
	{ KNIFE, "shito" },
	{ PLATE_MAIL, "tanko" },
	{ HELMET, "kabuto" },
	{ LEATHER_GLOVES, "yugake" },
	{ FOOD_RATION, "gunyoki" },
	{ POT_BOOZE, "sake" },
	{0, "" }
};

#endif /* OVLB */

STATIC_DCL const char *FDECL(Japanese_item_name,(int i));

#ifdef OVL1

STATIC_OVL char *
strprepend(s,pref)
register char *s;
register const char *pref;
{
	register int i = (int)strlen(pref);

	if(i > PREFIX) {
		impossible("PREFIX too short (for %d).", i);
		return(s);
	}
	s -= i;
	(void) strncpy(s, pref, i);	/* do not copy trailing 0 */
	return(s);
}

#endif /* OVL1 */
#ifdef OVLB

/* manage a pool of BUFSZ buffers, so callers don't have to */
static char *
nextobuf()
{
	static char NEARDATA bufs[NUMOBUF][BUFSZ];
	static int bufidx = 0;

	bufidx = (bufidx + 1) % NUMOBUF;
	return bufs[bufidx];
}

char *
obj_typename(otyp)
register int otyp;
{
	char *buf = nextobuf();
	register struct objclass *ocl = &objects[otyp];
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;
	register int nn = ocl->oc_name_known;
#if 1 /*JP*/
	char type;
	char type_name[BUFSZ];
#endif

	if (Role_if(PM_SAMURAI) && Japanese_item_name(otyp))
		actualn = Japanese_item_name(otyp);
#if 1 /*JP*/
	buf[0]='\0';
	type_name[0]='\0';
	if(un)
	  Sprintf(buf, "%sと呼ばれる", un);
#endif
	switch(ocl->oc_class) {
	case COIN_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "coin");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('$',actualn));
		type = '$';
#endif
		break;
	case POTION_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "potion");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('!',actualn));
		else if(un)
		  Strcat(buf, "薬");
		type = '!';
#endif
		break;
	case SCROLL_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "scroll");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('?',actualn));
		else if(un)
		  Strcat(buf, "巻物");
		 type = '?';
#endif
		break;
	case WAND_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "wand");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('/',actualn));
		else if(un)
		  Strcat(buf, "杖");
		type = '/';
#endif
		break;
	case SPBOOK_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "spellbook");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('+',actualn));
		else if(un)
		  Strcat(buf, "魔法書");
		type = '+';
#endif
		break;
	case RING_CLASS:
#if 0 /*JP*/
		Strcpy(buf, "ring");
#else
		if(nn)
		  Strcat(buf,jtrns_obj('=',actualn));
		else if(un)
		  Strcat(buf, "指輪");
		type = '=';
#endif
		break;
	case AMULET_CLASS:
		if(nn)
#if 0 /*JP*/
			Strcpy(buf,actualn);
		else
			Strcpy(buf,"amulet");
		if(un)
			Sprintf(eos(buf)," called %s",un);
		if(dn)
			Sprintf(eos(buf)," (%s)",dn);
		return(buf);
#else
			Strcat(buf,jtrns_obj('"',actualn));
		else if(un)
		    Strcat(buf, "魔除け");
		type = '"';
		break;
#endif
#if 1 /*JP*/
	case GEM_CLASS:
		if(nn)
		  Strcat(buf,jtrns_obj('*',actualn));
		else if(un)
		  Strcat(buf, "宝石");
		type = '*';
		break;
#endif
	default:
#if 1 /*JP*/
		type = ' ';
#endif
		if(nn) {
#if 0 /*JP*/
			Strcpy(buf, actualn);
			if (GemStone(otyp))
				Strcat(buf, " stone");
			if(un)
				Sprintf(eos(buf), " called %s", un);
			if(dn)
				Sprintf(eos(buf), " (%s)", dn);
#else
			Strcat(buf, jtrns_obj(' ',actualn));
#endif
		} else {
#if 0 /*JP*/
			Strcpy(buf, dn ? dn : actualn);
			if(ocl->oc_class == GEM_CLASS)
				Strcat(buf, (ocl->oc_material == MINERAL) ?
						" stone" : " gem");
			if(un)
				Sprintf(eos(buf), " called %s", un);
#else
			Strcat(buf, dn ? jtrns_obj(' ',dn)
			   : jtrns_obj(' ',actualn));
#endif
		}
#if 0 /*JP*/
		return(buf);
#else
		break;
#endif
	}
#if 0 /*JP*/
	/* here for ring/scroll/potion/wand */
	if(nn) {
	    if (ocl->oc_unique)
		Strcpy(buf, actualn); /* avoid spellbook of Book of the Dead */
	    else
		Sprintf(eos(buf), " of %s", actualn);
	}
	if(un)
		Sprintf(eos(buf), " called %s", un);
#endif
	if(dn)
#if 0 /*JP*/
		Sprintf(eos(buf), " (%s)", dn);
#else
		Sprintf(eos(buf), "(%s%s)", jtrns_obj(type, dn),
		  ocl->oc_class == POTION_CLASS ? "薬" : "");
#endif
	return(buf);
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
simple_typename(otyp)
int otyp;
{
    char *bufp, *pp, *save_uname = objects[otyp].oc_uname;

    objects[otyp].oc_uname = 0;		/* suppress any name given by user */
    bufp = obj_typename(otyp);
    objects[otyp].oc_uname = save_uname;
    if ((pp = strstri(bufp, " (")) != 0)
	*pp = '\0';		/* strip the appended description */
    return bufp;
}

boolean
obj_is_pname(obj)
register struct obj *obj;
{
    return((boolean)(obj->dknown && obj->known && obj->onamelth &&
		     /* Since there aren't any objects which are both
		        artifacts and unique, the last check is redundant. */
		     obj->oartifact && !objects[obj->otyp].oc_unique));
}

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.  The kludge used is
 * to temporarily set Blind so that xname() skips the dknown setting.  This
 * assumes that we don't want to do this too often; if this function becomes
 * frequently used, it'd probably be better to pass a parameter to xname()
 * or doname() instead.
 */
char *
distant_name(obj, func)
register struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
	char *str;

	long save_Blinded = Blinded;
	Blinded = 1;
	str = (*func)(obj);
	Blinded = save_Blinded;
	return str;
}

/* convert player specified fruit name into corresponding fruit juice name
   ("slice of pizza" -> "pizza juice" rather than "slice of pizza juice") */
char *
fruitname(juice)
boolean juice;	/* whether or not to append " juice" to the name */
{
#if 1 /*JP*//*日本語ではそこまでしない*/
    char *buf = nextobuf();
    Sprintf(buf, "%s%s", pl_fruit, juice ? "ジュース" : "");
    return buf;
#else
    char *buf = nextobuf();
    const char *fruit_nam = strstri(pl_fruit, " of ");

    if (fruit_nam)
	fruit_nam += 4;		/* skip past " of " */
    else
	fruit_nam = pl_fruit;	/* use it as is */

    Sprintf(buf, "%s%s", makesingular(fruit_nam), juice ? " juice" : "");
    return buf;
#endif
}

#endif /* OVLB */
#ifdef OVL1

char *
xname(obj)
register struct obj *obj;
{
	register char *buf;
	register int typ = obj->otyp;
	register struct objclass *ocl = &objects[typ];
	register int nn = ocl->oc_name_known;
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;
#if 1 /*JP*/
	register const char *jactualn = NULL,*jdn = NULL;
#endif

	buf = nextobuf() + PREFIX;	/* leave room for "17 -3 " */
	if (Role_if(PM_SAMURAI) && Japanese_item_name(typ))
		actualn = Japanese_item_name(typ);

	buf[0] = '\0';
	/*
	 * clean up known when it's tied to oc_name_known, eg after AD_DRIN
	 * This is only required for unique objects since the article
	 * printed for the object is tied to the combination of the two
	 * and printing the wrong article gives away information.
	 */
	if (!nn && ocl->oc_uses_known && ocl->oc_unique) obj->known = 0;
	if (!Blind) obj->dknown = TRUE;
	if (Role_if(PM_PRIEST)) obj->bknown = TRUE;
	if (obj_is_pname(obj))
#if 0 /*JP*/
	    goto nameit;
#else
	{
	    Strcat(buf, jtrns_obj('A', ONAME(obj)));
	    goto nameit;
	}
#endif
#if 1 /*JP*/
	if (obj->onamelth && obj->dknown) {
	  if(!obj->oartifact)
	    Strcat(buf, ONAME(obj));
	  else
	    Strcat(buf, jtrns_obj('A', ONAME(obj)));
	  Strcat(buf, "と名づけられた");
	}
#endif
	switch (obj->oclass) {
	    case AMULET_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('"', actualn);
		jdn = jtrns_obj('"', dn);
#endif
		if (!obj->dknown)
/*JP
			Strcpy(buf, "amulet");
*/
			Strcat(buf, "魔除け");  
		else if (typ == AMULET_OF_YENDOR ||
			 typ == FAKE_AMULET_OF_YENDOR)
			/* each must be identified individually */
/*JP
			Strcpy(buf, obj->known ? actualn : dn);
*/
			Strcat(buf, obj->known ? jactualn : jdn);
		else if (nn)
#if 0 /*JP*/
			Strcpy(buf, actualn);
#else
			Strcat(buf, jactualn);
#endif
		else if (un)
/*JP
			Sprintf(buf,"amulet called %s", un);
*/
			Sprintf(eos(buf),"%sと呼ばれる魔除け", un);
		else
/*JP
			Sprintf(buf,"%s amulet", dn);
*/
			Sprintf(eos(buf),"%s", jdn);
		break;
	    case WEAPON_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj(')',actualn);
		jdn = jtrns_obj(')',dn);
#endif
		if (is_poisonable(obj) && obj->opoisoned)
/*JP
			Strcpy(buf, "poisoned ");
*/
			Strcpy(buf, "毒の塗られた");
	    case VENOM_CLASS:
	    case TOOL_CLASS:
#if 1 /*JP*/
		if(obj->oclass == VENOM_CLASS){
		  jactualn = jtrns_obj('\'',actualn);
		  jdn = jtrns_obj('\'',dn);
		}
		else if(obj->oclass == TOOL_CLASS){
		  jactualn = jtrns_obj('(',actualn);
		  jdn = jtrns_obj('(',dn);
		}
		if (typ == FIGURINE)
			Sprintf(eos(buf), "%sの", jtrns_mon(mons[obj->corpsenm].mname));
#endif
		if (typ == LENSES)
/*JP
			Strcpy(buf, "pair of ");
*/
			Strcpy(buf, "一対の");

		if (!obj->dknown)
/*JP
			Strcat(buf, dn ? dn : actualn);
*/
			Strcat(buf, jdn ? jdn : jactualn);
		else if (nn)
/*JP
			Strcat(buf, actualn);
*/
			Strcat(buf, jactualn);
		else if (un) {
#if 0 /*JP*/
			Strcat(buf, dn ? dn : actualn);
			Strcat(buf, " called ");
			Strcat(buf, un);
#else
			Strcat(buf, un);
			Strcat(buf, "と呼ばれる");
			Strcat(buf, dn ? jdn : jactualn);
#endif
		} else
/*JP
			Strcat(buf, dn ? dn : actualn);
*/
			Strcat(buf, dn ? jdn : jactualn);
#if 0 /*JP*/ /*これは語順の関係から上の方で定義*/
		/* If we use an() here we'd have to remember never to use */
		/* it whenever calling doname() or xname(). */
		if (typ == FIGURINE)
		    Sprintf(eos(buf), " of a%s %s",
			index(vowels,*(mons[obj->corpsenm].mname)) ? "n" : "",
			mons[obj->corpsenm].mname);
#endif
		break;
	    case ARMOR_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj(']',actualn);
		jdn = jtrns_obj(']',dn);
#endif
		/* depends on order of the dragon scales objects */
		if (typ >= GRAY_DRAGON_SCALES && typ <= YELLOW_DRAGON_SCALES) {
/*JP
			Sprintf(buf, "set of %s", actualn);
*/
			Sprintf(buf, "%s一式", jactualn);
			break;
		}
/*JP
		if(is_boots(obj) || is_gloves(obj)) Strcpy(buf,"pair of ");
*/
		if(is_boots(obj) || is_gloves(obj)) Strcat(buf,"一対の");

		if(obj->otyp >= ELVEN_SHIELD && obj->otyp <= ORCISH_SHIELD
				&& !obj->dknown) {
/*JP
			Strcpy(buf, "shield");
*/
			Strcat(buf, "盾");
			break;
		}
		if(obj->otyp == SHIELD_OF_REFLECTION && !obj->dknown) {
/*JP
			Strcpy(buf, "smooth shield");
*/
			Strcat(buf, "すべすべした盾");
			break;
		}

#if 0 /*JP*/
		if(nn)	Strcat(buf, actualn);
		else if(un) {
			if(is_boots(obj))
				Strcat(buf,"boots");
			else if(is_gloves(obj))
				Strcat(buf,"gloves");
			else if(is_cloak(obj))
				Strcpy(buf,"cloak");
			else if(is_helmet(obj))
				Strcpy(buf,"helmet");
			else if(is_shield(obj))
				Strcpy(buf,"shield");
			else
				Strcpy(buf,"armor");
			Strcat(buf, " called ");
			Strcat(buf, un);
		} else	Strcat(buf, dn);
#else
		if(nn)	Strcat(buf, jactualn);
		else if(un) {
			char *p;
			if(is_boots(obj)) p = "靴";
			else if(is_gloves(obj)) p = "籠手";
			else if(is_cloak(obj)) p = "クローク";
			else if(is_helmet(obj)) p = "兜";
			else if(is_shield(obj)) p = "盾";
			else p = "鎧";
			Sprintf(eos(buf), "%sと呼ばれる%s", un, p);
		} else	Strcat(buf, jdn);
#endif
		break;
	    case FOOD_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('%',actualn);
		jdn = jtrns_obj('%',dn);
#endif
		if (typ == SLIME_MOLD) {
			register struct fruit *f;

			for(f=ffruit; f; f = f->nextf) {
				if(f->fid == obj->spe) {
					Strcpy(buf, f->fname);
					break;
				}
			}
			if (!f) impossible("Bad fruit #%d?", obj->spe);
			break;
		}

#if 0 /*JP*/
		Strcpy(buf, actualn);
#endif
		if (typ == TIN && obj->known) {
		    if(obj->spe > 0)
/*JP
			Strcat(buf, " of spinach");
*/
			Strcat(buf, "ホウレン草の");
		    else if (obj->corpsenm == NON_PM)
/*JP
		        Strcpy(buf, "empty tin");
*/
		        Strcat(buf, "空っぽの");
		    else if (vegetarian(&mons[obj->corpsenm]))
/*JP
			Sprintf(eos(buf), " of %s", mons[obj->corpsenm].mname);
*/
			Sprintf(eos(buf), "%sの", jtrns_mon(mons[obj->corpsenm].mname));
		    else
/*JP
			Sprintf(eos(buf), " of %s meat", mons[obj->corpsenm].mname);
*/
			Sprintf(eos(buf), "%sの肉の", jtrns_mon(mons[obj->corpsenm].mname));
		}
#if 1 /*JP*/
		Strcat(buf, jactualn);
#endif
		break;
	    case COIN_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('$',actualn);
		jdn = jtrns_obj('$',dn);
		Strcat(buf, jactualn);
		break;
#endif
	    case CHAIN_CLASS:
#if 0 /*JP*/
		Strcpy(buf, actualn);
#else
		jactualn = jtrns_obj('_',actualn);
		jdn = jtrns_obj('_',dn);
		Strcat(buf, jactualn);
#endif
		break;
	    case ROCK_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('\'',actualn);
		jdn = jtrns_obj('\'',dn);
#endif
		if (typ == STATUE)
#if 0 /*JP*/
		    Sprintf(buf, "%s%s of %s%s",
			(Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC)) ? "historic " : "" ,
			actualn,
			type_is_pname(&mons[obj->corpsenm]) ? "" :
			  (mons[obj->corpsenm].geno & G_UNIQ) ? "the " :
			    (index(vowels,*(mons[obj->corpsenm].mname)) ?
								"an " : "a "),
			mons[obj->corpsenm].mname);
		else Strcpy(buf, actualn);
#else
		    Sprintf(eos(buf), "%s%sの%s", 
			    (Role_if(PM_ARCHEOLOGIST) && obj->spe) ? "歴史的な" : "" ,
			    jtrns_mon(mons[obj->corpsenm].mname), jactualn);
		else Strcat(buf, jactualn);
#endif
		break;
	    case BALL_CLASS:
#if 0 /*JP*/
		Sprintf(buf, "%sheavy iron ball",
			(obj->owt > ocl->oc_weight) ? "very " : "");
#else
		Sprintf(eos(buf), "%s%s",
			(obj->owt > ocl->oc_weight) ? "とても" : "",
			jtrns_obj('0', "heavy iron ball"));
#endif
		break;
	    case POTION_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('!',actualn);
		jdn = jtrns_obj('!',dn);
#endif
		if (obj->dknown && obj->odiluted)
/*JP
			Strcpy(buf, "diluted ");
*/
			Strcat(buf, "薄まった");
		if(nn || un || !obj->dknown) {
#if 0 /*JP*/
			Strcat(buf, "potion");
			if(!obj->dknown) break;
#else
			if(!obj->dknown){
			  Strcat(buf,"薬");
			  break;
			}
#endif
			if(nn) {
#if 0 /*JP*/
			    Strcat(buf, " of ");
#endif
			    if (typ == POT_WATER &&
				obj->bknown && (obj->blessed || obj->cursed)) {
/*JP
				Strcat(buf, obj->blessed ? "holy " : "unholy ");
*/
				Strcat(buf, obj->blessed ? "聖" : "不浄な");
			    }
/*JP
			    Strcat(buf, actualn);
*/
			    Strcat(buf, jactualn);
			} else {
#if 0 /*JP*/
				Strcat(buf, " called ");
				Strcat(buf, un);
#else
				Strcat(buf, un);
				Strcat(buf, "と呼ばれる薬");
#endif
			}
		} else {
#if 0 /*JP*/
			Strcat(buf, dn);
			Strcat(buf, " potion");
#else
			Strcat(buf, jdn);
			Strcat(buf, "薬");
#endif
		}
		break;
	case SCROLL_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('?', actualn);
		jdn = jtrns_obj('?', dn);
		if(!obj->dknown){
			Strcat(buf,"巻物");
			break;
		}
#else
		Strcpy(buf, "scroll");
		if(!obj->dknown) break;
#endif
		if(nn) {
#if 0 /*JP*/
			Strcat(buf, " of ");
			Strcat(buf, actualn);
#else
			Strcat(buf, jactualn);
#endif
		} else if(un) {
#if 0 /*JP*/
			Strcat(buf, " called ");
			Strcat(buf, un);
#else
			Strcat(buf, un);
			Strcat(buf, "と呼ばれる巻物");
#endif
		} else if (ocl->oc_magic) {
#if 0 /*JP*/
			Strcat(buf, " labeled ");
			Strcat(buf, dn);
#else
			Strcat(buf, jdn);
#endif
		} else {
#if 0 /*JP*/
			Strcpy(buf, dn);
			Strcat(buf, " scroll");
#else
			Strcat(buf, jdn);
#endif
		}
		break;
	case WAND_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('/',actualn);
		jdn = jtrns_obj('/',dn);
#endif
		if(!obj->dknown)
/*JP
			Strcpy(buf, "wand");
*/
			Strcat(buf, "杖");
		else if(nn)
/*JP
			Sprintf(buf, "wand of %s", actualn);
*/
			Strcat(buf, jactualn);
		else if(un)
/*JP
			Sprintf(buf, "wand called %s", un);
*/
			Sprintf(eos(buf), "%sと呼ばれる杖", un);
		else
/*JP
			Sprintf(buf, "%s wand", dn);
*/
			Strcat(buf, jdn);
		break;
	case SPBOOK_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('+',actualn);
		jdn = jtrns_obj('+',dn);
#endif
		if (!obj->dknown) {
/*JP
			Strcpy(buf, "spellbook");
*/
			Strcat(buf, "魔法書");
		} else if (nn) {
#if 0 /*JP*/
			if (typ != SPE_BOOK_OF_THE_DEAD)
			    Strcpy(buf, "spellbook of ");
			Strcat(buf, actualn);
#else
			Strcat(buf, jactualn);
#endif
		} else if (un) {
/*JP
			Sprintf(buf, "spellbook called %s", un);
*/
			Sprintf(eos(buf), "%sと呼ばれる魔法書", un);
		} else
/*JP
			Sprintf(buf, "%s spellbook", dn);
*/
			Sprintf(eos(buf), "%s", jdn);
		break;
	case RING_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('=',actualn);
		jdn = jtrns_obj('=',dn);
#endif
		if(!obj->dknown)
/*JP
			Strcpy(buf, "ring");
*/
			Strcat(buf, "指輪");
		else if(nn)
/*JP
			Sprintf(buf, "ring of %s", actualn);
*/
			Strcat(buf, jactualn);
		else if(un)
/*JP
			Sprintf(buf, "ring called %s", un);
*/
			Sprintf(eos(buf), "%sと呼ばれる指輪", un);
		else
/*JP
			Sprintf(buf, "%s ring", dn);
*/
			Strcat(buf, jdn);
		break;
	case GEM_CLASS:
#if 1 /*JP*/
		jactualn = jtrns_obj('*',actualn);
		jdn = jtrns_obj('*',dn);
#endif
	    {
		const char *rock =
/*JP
			    (ocl->oc_material == MINERAL) ? "stone" : "gem";
*/
			    (ocl->oc_material == MINERAL) ? "石" : "宝石";
		if (!obj->dknown) {
/*JP
		    Strcpy(buf, rock);
*/
		    Strcat(buf, rock);
		} else if (!nn) {
#if 0 /*JP*/
		    if (un) Sprintf(buf,"%s called %s", rock, un);
		    else Sprintf(buf, "%s %s", dn, rock);
#else
		    if (un) Sprintf(eos(buf), "%sと呼ばれる%s", un, rock);
		    else Strcat(buf, jdn);
#endif
		} else {
#if 0 /*JP*/
		    Strcpy(buf, actualn);
		    if (GemStone(typ)) Strcat(buf, " stone");
#else
		    Strcpy(buf, jactualn);
#endif
		}
		break;
	    }
	default:
		Sprintf(buf,"glorkum %d %d %d", obj->oclass, typ, obj->spe);
	}
#if 0 /*JP*/
	if (obj->quan != 1L) Strcpy(buf, makeplural(buf));
#endif

#if 0 /*JP*/
	if (obj->onamelth && obj->dknown) {
		Strcat(buf, " named ");
nameit:
		Strcat(buf, ONAME(obj));
	}

	if (!strncmpi(buf, "the ", 4)) buf += 4;
#else
nameit:
#endif
	return(buf);
}

/* xname() output augmented for multishot missile feedback */
char *
mshot_xname(obj)
struct obj *obj;
{
    char tmpbuf[BUFSZ];
    char *onm = xname(obj);

    if (m_shot.n > 1 && m_shot.o == obj->otyp) {
	/* copy xname's result so that we can reuse its return buffer */
	Strcpy(tmpbuf, onm);
	/* "the Nth arrow"; value will eventually be passed to an() or
	   The(), both of which correctly handle this "the " prefix */
#if 0 /*JP*/
	Sprintf(onm, "the %d%s %s", m_shot.i, ordin(m_shot.i), tmpbuf);
#else
	Sprintf(onm, "%d%s目の%s", m_shot.i, numeral(obj), tmpbuf);
#endif
    }

    return onm;
}

#endif /* OVL1 */
#ifdef OVL0

#if 0 /*JP*/
/* used for naming "the unique_item" instead of "a unique_item" */
boolean
the_unique_obj(obj)
register struct obj *obj;
{
    if (!obj->dknown)
	return FALSE;
    else if (obj->otyp == FAKE_AMULET_OF_YENDOR && !obj->known)
	return TRUE;		/* lie */
    else
	return (boolean)(objects[obj->otyp].oc_unique &&
			 (obj->known || obj->otyp == AMULET_OF_YENDOR));
}
#endif

static void
add_erosion_words(obj,prefix)
struct obj *obj;
char *prefix;
{
	boolean iscrys = (obj->otyp == CRYSKNIFE);


	if (!is_damageable(obj) && !iscrys) return;

	/* The only cases where any of these bits do double duty are for
	 * rotted food and diluted potions, which are all not is_damageable().
	 */
	if (obj->oeroded && !iscrys) {
#if 0 /*JP*/
		switch (obj->oeroded) {
			case 2:	Strcat(prefix, "very "); break;
			case 3:	Strcat(prefix, "thoroughly "); break;
		}			
		Strcat(prefix, is_rustprone(obj) ? "rusty " : "burnt ");
#else
		switch (obj->oeroded) {
			case 2:	Strcat(prefix, "とても"); break;
			case 3:	Strcat(prefix, "かなり"); break;
		}			
		Strcat(prefix, is_rustprone(obj) ? "錆びた" : "焦げた");
#endif
	}
	if (obj->oeroded2 && !iscrys) {
#if 0 /*JP*/
		switch (obj->oeroded2) {
			case 2:	Strcat(prefix, "very "); break;
			case 3:	Strcat(prefix, "thoroughly "); break;
		}			
		Strcat(prefix, is_corrodeable(obj) ? "corroded " :
			"rotted ");
#else
		switch (obj->oeroded2) {
			case 2:	Strcat(prefix, "とても"); break;
			case 3:	Strcat(prefix, "かなり"); break;
		}			
		Strcat(prefix, is_corrodeable(obj) ? "腐食した" :
			"腐った");
#endif
	}
	if (obj->rknown && obj->oerodeproof)
		Strcat(prefix,
#if 0 /*JP*/
		       iscrys ? "fixed " :
		       is_rustprone(obj) ? "rustproof " :
		       is_corrodeable(obj) ? "corrodeproof " :	/* "stainless"? */
		       is_flammable(obj) ? "fireproof " : "");
#else
		       iscrys ? "安定した" :
		       is_rustprone(obj) ? "錆びない" :
		       is_corrodeable(obj) ? "腐食しない" :	/* "stainless"? */
		       is_flammable(obj) ? "燃えない" : "");
#endif
}

char *
doname(obj)
register struct obj *obj;
{
	boolean ispoisoned = FALSE;
	char prefix[PREFIX];
#if 0 /*JP*/
	char tmpbuf[PREFIX+1];
#endif
	/* when we have to add something at the start of prefix instead of the
	 * end (Strcat is used on the end)
	 */
	register char *bp = xname(obj);
#if 1 /*JP*/ /*  苦肉の策 */
	char preprefix[PREFIX], *tp;
#endif

	/* When using xname, we want "poisoned arrow", and when using
	 * doname, we want "poisoned +0 arrow".  This kludge is about the only
	 * way to do it, at least until someone overhauls xname() and doname(),
	 * combining both into one function taking a parameter.
	 */
	/* must check opoisoned--someone can have a weirdly-named fruit */
#if 0 /*JP*/
	if (!strncmp(bp, "poisoned ", 9) && obj->opoisoned) {
		bp += 9;
		ispoisoned = TRUE;
	}
#else
	if (!strncmp(bp, "毒の塗られた", 12) && obj->opoisoned) {
		bp += 12;
		ispoisoned = TRUE;
	}
#endif
#if 1 /*JP*/
	/* JP
	 *「子猫のたまと名づけられた死体」より「たまと名づけられた子猫の死体」
	 *  のほうが自然である。
         */
	preprefix[0] = '\0';
	if((tp = strstri(bp, "名づけられた")) != NULL){
	  tp += 12; /* 「名づけられた」*/
	  strncpy(preprefix, bp, tp - bp);
	  preprefix[tp - bp] = '\0';
	  bp = tp;
	}
	Strcpy(prefix, "");
#endif

	if(obj->quan != 1L)
#if 0 /*JP*/
		Sprintf(prefix, "%ld ", obj->quan);
#else /* 日本語としては数詞がないのは不自然 */
		Sprintf(prefix, "%ld%sの", obj->quan, numeral(obj));
#endif
#if 0 /*JP*/ /* 冠詞は不要 */
	else if (obj_is_pname(obj) || the_unique_obj(obj)) {
		if (!strncmpi(bp, "the ", 4))
		    bp += 4;
		Strcpy(prefix, "the ");
	} else
		Strcpy(prefix, "a ");
#endif

#ifdef INVISIBLE_OBJECTS
	if (obj->oinvis) Strcat(prefix,"invisible ");
#endif

	if (obj->bknown &&
	    obj->oclass != COIN_CLASS &&
	    (obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
		|| (!obj->cursed && !obj->blessed))) {
	    /* allow 'blessed clear potion' if we don't know it's holy water;
	     * always allow "uncursed potion of water"
	     */
	    if (obj->cursed)
/*JP
		Strcat(prefix, "cursed ");
*/
		Strcat(prefix, "呪われた");
	    else if (obj->blessed)
/*JP
		Strcat(prefix, "blessed ");
*/
		Strcat(prefix, "祝福された");
	    else if ((!obj->known || !objects[obj->otyp].oc_charged ||
		      (obj->oclass == ARMOR_CLASS ||
		       obj->oclass == RING_CLASS))
		/* For most items with charges or +/-, if you know how many
		 * charges are left or what the +/- is, then you must have
		 * totally identified the item, so "uncursed" is unneccesary,
		 * because an identified object not described as "blessed" or
		 * "cursed" must be uncursed.
		 *
		 * If the charges or +/- is not known, "uncursed" must be
		 * printed to avoid ambiguity between an item whose curse
		 * status is unknown, and an item known to be uncursed.
		 */
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
			&& obj->otyp != FAKE_AMULET_OF_YENDOR
			&& obj->otyp != AMULET_OF_YENDOR
			&& !Role_if(PM_PRIEST))
/*JP
		Strcat(prefix, "uncursed ");
*/
		Strcat(prefix, "呪われていない");
	}

/*JP
	if (obj->greased) Strcat(prefix, "greased ");
*/
	if (obj->greased) Strcat(prefix, "油の塗られた");

	switch(obj->oclass) {
	case AMULET_CLASS:
		if(obj->owornmask & W_AMUL)
/*JP
			Strcat(bp, " (being worn)");
*/
			Strcat(bp, "(身につけている)");
		break;
	case WEAPON_CLASS:
		if(ispoisoned)
/*JP
			Strcat(prefix, "poisoned ");
*/
			Strcat(prefix, "毒の塗られた");
plus:
		add_erosion_words(obj, prefix);
		if(obj->known) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case ARMOR_CLASS:
		if(obj->owornmask & W_ARMOR)
#if 0 /*JP*/
			Strcat(bp, (obj == uskin) ? " (embedded in your skin)" :
				" (being worn)");
#else
			Strcat(bp, (obj == uskin) ? "(肌に埋めこまれている)" :
				"(身につけている)");
#endif
		goto plus;
	case TOOL_CLASS:
		/* weptools already get this done when we go to the +n code */
		if (!is_weptool(obj))
		    add_erosion_words(obj, prefix);
		if(obj->owornmask & (W_TOOL /* blindfold */
#ifdef STEED
				| W_SADDLE
#endif
				)) {
/*JP
			Strcat(bp, " (being worn)");
*/
			Strcat(bp, "(身につけている)");
			break;
		}
		if (obj->otyp == LEASH && obj->leashmon != 0) {
/*JP
			Strcat(bp, " (in use)");
*/
			Strcat(bp, "(結びつけている)");
			break;
		}
		if (is_weptool(obj))
			goto plus;
		if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
#if 0 /*JP*/
			if (!obj->spe)
			    Strcpy(tmpbuf, "no");
			else
			    Sprintf(tmpbuf, "%d", obj->spe);
			Sprintf(eos(bp), " (%s candle%s%s)",
				tmpbuf, plur(obj->spe),
				!obj->lamplit ? " attached" : ", lit");
#else
			if(!obj->spe)
			    Sprintf(eos(bp), "(一本も取りつけられていない)");
			else {
			    if(!obj->lamplit)
				Sprintf(eos(bp), "(%d本取りつけられている)",
					obj->spe);
			    else
				Sprintf(eos(bp), "(%d本光っている)",
					obj->spe);
			}
#endif
			break;
		} else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
			obj->otyp == ORB_OF_LIGHT ||
			obj->otyp == BRASS_LANTERN || Is_candle(obj)) {
			if (Is_candle(obj) &&
			    obj->age < 20L * (long)objects[obj->otyp].oc_cost)
/*JP
				Strcat(prefix, "partly used ");
*/
				Strcat(prefix, "使いさしの");
			if(obj->lamplit)
/*JP
				Strcat(bp, " (lit)");
*/
				Strcat(bp, "(光っている)");
			break;
		}
		if(objects[obj->otyp].oc_charged)
		    goto charges;
		break;
	case WAND_CLASS:
		add_erosion_words(obj, prefix);
charges:
		if(obj->known)
/*JP
		    Sprintf(eos(bp), " (%d:%d)", (int)obj->recharged, obj->spe);
*/
		    Sprintf(eos(bp), "(%d:%d)", (int)obj->recharged, obj->spe);
		break;
	case POTION_CLASS:
		if (obj->otyp == POT_OIL && obj->lamplit)
/*JP
		    Strcat(bp, " (lit)");
*/
		    Strcat(bp, "(光っている)");
		break;
	case RING_CLASS:
		add_erosion_words(obj, prefix);
ring:
/*JP
		if(obj->owornmask & W_RINGR) Strcat(bp, " (on right ");
*/
		if(obj->owornmask & W_RINGR) Strcat(bp, "(右");
/*JP
		if(obj->owornmask & W_RINGL) Strcat(bp, " (on left ");
*/
		if(obj->owornmask & W_RINGL) Strcat(bp, "(左");
		if(obj->owornmask & W_RING) {
		    Strcat(bp, body_part(HAND));
		    Strcat(bp, ")");
		}
		if(obj->known && objects[obj->otyp].oc_charged) {
#if 1 /*JP*/
			Strcat(prefix, " ");
#endif
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case FOOD_CLASS:
		if (obj->oeaten)
/*JP
		    Strcat(prefix, "partly eaten ");
*/
		    Strcat(prefix, "食べかけの");
		if (obj->otyp == CORPSE) {
		    if (mons[obj->corpsenm].geno & G_UNIQ) {
#if 0 /*JP*/
			Sprintf(prefix, "%s%s ",
				(type_is_pname(&mons[obj->corpsenm]) ?
					"" : "the "),
				s_suffix(mons[obj->corpsenm].mname));
			if (obj->oeaten) Strcat(prefix, "partly eaten ");
#else
			if (obj->oeaten) Strcpy(prefix, "食べかけの");
			Sprintf(eos(prefix), "%sの",
				jtrns_mon(mons[obj->corpsenm].mname));
#endif
		    } else {
#if 0 /*JP*/
			Strcat(prefix, mons[obj->corpsenm].mname);
			Strcat(prefix, " ");
#else
			Strcat(prefix, jtrns_mon(mons[obj->corpsenm].mname));
			Strcat(prefix, "の");
#endif
		    }
		} else if (obj->otyp == EGG) {
#if 0	/* corpses don't tell if they're stale either */
		    if (obj->known && stale_egg(obj))
			Strcat(prefix, "stale ");
#endif
		    if (obj->corpsenm >= LOW_PM &&
			    (obj->known ||
			    mvitals[obj->corpsenm].mvflags & MV_KNOWS_EGG)) {
#if 0 /*JP*/
			Strcat(prefix, mons[obj->corpsenm].mname);
			Strcat(prefix, " ");
#else
			Strcat(prefix, jtrns_mon(mons[obj->corpsenm].mname));
			Strcat(prefix, "の");
#endif
			if (obj->spe)
/*JP
			    Strcat(bp, " (laid by you)");
*/
			    Strcat(bp, "(あなたが産んだ)");
		    }
		}
		if (obj->otyp == MEAT_RING) goto ring;
		break;
	case BALL_CLASS:
	case CHAIN_CLASS:
		add_erosion_words(obj, prefix);
		if(obj->owornmask & W_BALL)
/*JP
			Strcat(bp, " (chained to you)");
*/
			Strcat(bp, "(あなたに繋がれている)");
			break;
	}

	if((obj->owornmask & W_WEP) && !mrg_to_wielded) {
		if (obj->quan != 1L) {
/*JP
			Strcat(bp, " (wielded)");
*/
			Strcat(bp, "(装備している)");
		} else {
			const char *hand_s = body_part(HAND);

			if (bimanual(obj)) hand_s = makeplural(hand_s);
/*JP
			Sprintf(eos(bp), " (weapon in %s)", hand_s);
*/
			Sprintf(eos(bp), "(%sにしている)", hand_s);
		}
	}
	if(obj->owornmask & W_SWAPWEP) {
		if (u.twoweap)
/*JP
			Sprintf(eos(bp), " (wielded in other %s)",
*/
			Sprintf(eos(bp), "(左%sにしている)",
				body_part(HAND));
		else
/*JP
			Strcat(bp, " (alternate weapon; not wielded)");
*/
			Strcat(bp, "(予備の武器;装備していない)");
	}
/*JP
	if(obj->owornmask & W_QUIVER) Strcat(bp, " (in quiver)");
*/
	if(obj->owornmask & W_QUIVER) Strcat(bp, "(装填している)");
	if(obj->unpaid) {
		xchar ox, oy; 
		long quotedprice = unpaid_cost(obj);
		struct monst *shkp = (struct monst *)0;

		if (Has_contents(obj) &&
		    get_obj_location(obj, &ox, &oy, BURIED_TOO|CONTAINED_TOO) &&
		    costly_spot(ox, oy) &&
		    (shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE))))
			quotedprice += contained_cost(obj, shkp, 0L, FALSE, TRUE);
#if 0 /*JP*/
		Sprintf(eos(bp), " (unpaid, %ld %s)",
#else
		Sprintf(eos(bp), "(未払い、%ld%s)",
#endif
			quotedprice, currency(quotedprice));
	}
#if 0 /*JP*/ /* a-an はない */
	if (!strncmp(prefix, "a ", 2) &&
			index(vowels, *(prefix+2) ? *(prefix+2) : *bp)
			&& (*(prefix+2) || (strncmp(bp, "uranium", 7)
				&& strncmp(bp, "unicorn", 7)
				&& strncmp(bp, "eucalyptus", 10)))) {
		Strcpy(tmpbuf, prefix);
		Strcpy(prefix, "an ");
		Strcpy(prefix+3, tmpbuf+2);
	}
	bp = strprepend(bp, prefix);
#else
	Strcat(preprefix,prefix);
	bp = strprepend(bp, preprefix);
#endif
	return(bp);
}

#endif /* OVL0 */
#ifdef OVLB

/* used from invent.c */
boolean
not_fully_identified(otmp)
register struct obj *otmp;
{
#ifdef GOLDOBJ
    /* gold doesn't have any interesting attributes [yet?] */
    if (otmp->oclass == COIN_CLASS) return FALSE;	/* always fully ID'd */
#endif
    /* check fundamental ID hallmarks first */
    if (!otmp->known || !otmp->dknown ||
#ifdef MAIL
	    (!otmp->bknown && otmp->otyp != SCR_MAIL) ||
#else
	    !otmp->bknown ||
#endif
	    !objects[otmp->otyp].oc_name_known)	/* ?redundant? */
	return TRUE;
    if (otmp->oartifact && undiscovered_artifact(otmp->oartifact))
	return TRUE;
    /* otmp->rknown is the only item of interest if we reach here */
       /*
	*  Note:  if a revision ever allows scrolls to become fireproof or
	*  rings to become shockproof, this checking will need to be revised.
	*  `rknown' ID only matters if xname() will provide the info about it.
	*/
    if (otmp->rknown || (otmp->oclass != ARMOR_CLASS &&
			 otmp->oclass != WEAPON_CLASS &&
			 !is_weptool(otmp) &&		    /* (redunant) */
			 otmp->oclass != BALL_CLASS))	    /* (useless) */
	return FALSE;
    else	/* lack of `rknown' only matters for vulnerable objects */
	return (boolean)(is_rustprone(otmp) ||
			 is_corrodeable(otmp) ||
			 is_flammable(otmp));
}

char *
corpse_xname(otmp, ignore_oquan)
struct obj *otmp;
boolean ignore_oquan;	/* to force singular */
{
	char *nambuf = nextobuf();

#if 1 /*JP*/
	Sprintf(nambuf, "%sの死体", jtrns_mon(mons[otmp->corpsenm].mname));
	return nambuf;
#else
	Sprintf(nambuf, "%s corpse", mons[otmp->corpsenm].mname);

	if (ignore_oquan || otmp->quan < 2)
	    return nambuf;
	else
	    return makeplural(nambuf);
#endif
}

/* xname, unless it's a corpse, then corpse_xname(obj, FALSE) */
char *
cxname(obj)
struct obj *obj;
{
	if (obj->otyp == CORPSE)
	    return corpse_xname(obj, FALSE);
	return xname(obj);
}

/* treat an object as fully ID'd when it might be used as reason for death */
char *
killer_xname(obj)
struct obj *obj;
{
    struct obj save_obj;
    unsigned save_ocknown;
    char *buf, *save_ocuname;

    /* remember original settings for core of the object;
       oname and oattached extensions don't matter here--since they
       aren't modified they don't need to be saved and restored */
    save_obj = *obj;
    /* killer name should be more specific than general xname; however, exact
       info like blessed/cursed and rustproof makes things be too verbose */
    obj->known = obj->dknown = 1;
    obj->bknown = obj->rknown = obj->greased = 0;
    /* if character is a priest[ess], bknown will get toggled back on */
    obj->blessed = obj->cursed = 0;
    /* "killed by poisoned <obj>" would be misleading when poison is
       not the cause of death and "poisoned by poisoned <obj>" would
       be redundant when it is, so suppress "poisoned" prefix */
    obj->opoisoned = 0;
    /* strip user-supplied name; artifacts keep theirs */
    if (!obj->oartifact) obj->onamelth = 0;
    /* temporarily identify the type of object */
    save_ocknown = objects[obj->otyp].oc_name_known;
    objects[obj->otyp].oc_name_known = 1;
    save_ocuname = objects[obj->otyp].oc_uname;
    objects[obj->otyp].oc_uname = 0;	/* avoid "foo called bar" */

    buf = xname(obj);
    if (obj->quan == 1L) buf = obj_is_pname(obj) ? the(buf) : an(buf);

    objects[obj->otyp].oc_name_known = save_ocknown;
    objects[obj->otyp].oc_uname = save_ocuname;
    *obj = save_obj;	/* restore object's core settings */

    return buf;
}

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */
const char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
	long savequan;
	char *nam;

	/* Note: using xname for corpses will not give the monster type */
	if (otmp->otyp == CORPSE && func == xname)
		return corpse_xname(otmp, TRUE);

	savequan = otmp->quan;
	otmp->quan = 1L;
	nam = (*func)(otmp);
	otmp->quan = savequan;
	return nam;
}

char *
an(str)
register const char *str;
{
	char *buf = nextobuf();

#if 0 /*JP*/
	buf[0] = '\0';

	if (strncmpi(str, "the ", 4) &&
	    strcmp(str, "molten lava") &&
	    strcmp(str, "iron bars") &&
	    strcmp(str, "ice")) {
		if (index(vowels, *str) &&
		    strncmp(str, "one-", 4) &&
		    strncmp(str, "useful", 6) &&
		    strncmp(str, "unicorn", 7) &&
		    strncmp(str, "uranium", 7) &&
		    strncmp(str, "eucalyptus", 10))
			Strcpy(buf, "an ");
		else
			Strcpy(buf, "a ");
	}

	Strcat(buf, str);
#else
	Strcpy(buf, str);
#endif
	return buf;
}

char *
An(str)
const char *str;
{
	register char *tmp = an(str);
#if 0 /*JP*/
	*tmp = highc(*tmp);
#endif
	return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
	char *buf = nextobuf();
#if 0 /*JP*/
	boolean insert_the = FALSE;

	if (!strncmpi(str, "the ", 4)) {
	    buf[0] = lowc(*str);
	    Strcpy(&buf[1], str+1);
	    return buf;
	} else if (*str < 'A' || *str > 'Z') {
	    /* not a proper name, needs an article */
	    insert_the = TRUE;
	} else {
	    /* Probably a proper name, might not need an article */
	    register char *tmp, *named, *called;
	    int l;

	    /* some objects have capitalized adjectives in their names */
	    if(((tmp = rindex(str, ' ')) || (tmp = rindex(str, '-'))) &&
	       (tmp[1] < 'A' || tmp[1] > 'Z'))
		insert_the = TRUE;
	    else if (tmp && index(str, ' ') < tmp) {	/* has spaces */
		/* it needs an article if the name contains "of" */
		tmp = strstri(str, " of ");
		named = strstri(str, " named ");
		called = strstri(str, " called ");
		if (called && (!named || called < named)) named = called;

		if (tmp && (!named || tmp < named))	/* found an "of" */
		    insert_the = TRUE;
		/* stupid special case: lacks "of" but needs "the" */
		else if (!named && (l = strlen(str)) >= 31 &&
		      !strcmp(&str[l - 31], "Platinum Yendorian Express Card"))
		    insert_the = TRUE;
	    }
	}
	if (insert_the)
	    Strcpy(buf, "the ");
	else
	    buf[0] = '\0';
	Strcat(buf, str);

#else
	Strcpy(buf, str);
#endif /*JP*/
	return buf;
}

char *
The(str)
const char *str;
{
    register char *tmp = the(str);
#if 0 /*JP*/
    *tmp = highc(*tmp);
#endif
    return tmp;
}

#if 1 /*JP*/
char *
aobjnam(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
    return xname(otmp);
}

#else
/* returns "count cxname(otmp)" or just cxname(otmp) if count == 1 */
char *
aobjnam(otmp,verb)
register struct obj *otmp;
register const char *verb;
{
	register char *bp = cxname(otmp);
#if 1 /*JP*/
	static char prefix[PREFIX];

	Strcpy(prefix,bp);
	if(verb){
	  Strcat(prefix,"は");
	  Strcat(prefix,verb);
	}

	return prefix;
#else
	char prefix[PREFIX];

	if(otmp->quan != 1L) {
		Sprintf(prefix, "%ld ", otmp->quan);
		bp = strprepend(bp, prefix);
	}

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
#endif
}
#endif

#if 0 /*JP*//*日本語には三単現のsはない*/
/* like aobjnam, but prepend "The", not count, and use xname */
char *
Tobjnam(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *bp = The(xname(otmp));

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
}
#endif

#if 0 /*JP*//*日本語には三単現のsはない*/
/* return form of the verb (input plural) if xname(otmp) were the subject */
char *
otense(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *buf;

	/*
	 * verb is given in plural (without trailing s).  Return as input
	 * if the result of xname(otmp) would be plural.  Don't bother
	 * recomputing xname(otmp) at this time.
	 */
	if (!is_plural(otmp))
	    return vtense((char *)0, verb);

	buf = nextobuf();
	Strcpy(buf, verb);
	return buf;
}
#endif

/* various singular words that vtense would otherwise categorize as plural */
static const char * const special_subjs[] = {
	"erinys",
	"manes",		/* this one is ambiguous */
	"Cyclops",
	"Hippocrates",
	"Pelias",
	"aklys",
	"amnesia",
	"paralysis",
	0
};

#if 0 /*JP*//*日本語には三単現のsはない*/
/* return form of the verb (input plural) for present tense 3rd person subj */
char *
vtense(subj, verb)
register const char *subj;
register const char *verb;
{
	char *buf = nextobuf();
	int len, ltmp;
	const char *sp, *spot;
	const char * const *spec;

	/*
	 * verb is given in plural (without trailing s).  Return as input
	 * if subj appears to be plural.  Add special cases as necessary.
	 * Many hard cases can already be handled by using otense() instead.
	 * If this gets much bigger, consider decomposing makeplural.
	 * Note: monster names are not expected here (except before corpse).
	 *
	 * special case: allow null sobj to get the singular 3rd person
	 * present tense form so we don't duplicate this code elsewhere.
	 */
	if (subj) {
	    if (!strncmpi(subj, "a ", 2) || !strncmpi(subj, "an ", 3))
		goto sing;
	    spot = (const char *)0;
	    for (sp = subj; (sp = index(sp, ' ')) != 0; ++sp) {
		if (!strncmp(sp, " of ", 4) ||
		    !strncmp(sp, " from ", 6) ||
		    !strncmp(sp, " called ", 8) ||
		    !strncmp(sp, " named ", 7) ||
		    !strncmp(sp, " labeled ", 9)) {
		    if (sp != subj) spot = sp - 1;
		    break;
		}
	    }
	    len = (int) strlen(subj);
	    if (!spot) spot = subj + len - 1;

	    /*
	     * plural: anything that ends in 's', but not '*us' or '*ss'.
	     * Guess at a few other special cases that makeplural creates.
	     */
	    if ((*spot == 's' && spot != subj &&
			(*(spot-1) != 'u' && *(spot-1) != 's')) ||
		((spot - subj) >= 4 && !strncmp(spot-3, "eeth", 4)) ||
		((spot - subj) >= 3 && !strncmp(spot-3, "feet", 4)) ||
		((spot - subj) >= 2 && !strncmp(spot-1, "ia", 2)) ||
		((spot - subj) >= 2 && !strncmp(spot-1, "ae", 2))) {
		/* check for special cases to avoid false matches */
		len = (int)(spot - subj) + 1;
		for (spec = special_subjs; *spec; spec++) {
		    ltmp = strlen(*spec);
		    if (len == ltmp && !strncmpi(*spec, subj, len)) goto sing;
		    /* also check for <prefix><space><special_subj>
		       to catch things like "the invisible erinys" */
		    if (len > ltmp && *(spot - ltmp) == ' ' &&
			   !strncmpi(*spec, spot - ltmp + 1, ltmp)) goto sing;
		}

		return strcpy(buf, verb);
	    }
	    /*
	     * 3rd person plural doesn't end in telltale 's';
	     * 2nd person singular behaves as if plural.
	     */
	    if (!strcmpi(subj, "they") || !strcmpi(subj, "you"))
		return strcpy(buf, verb);
	}

 sing:
	len = strlen(verb);
	spot = verb + len - 1;

	if (!strcmp(verb, "are"))
	    Strcpy(buf, "is");
	else if (!strcmp(verb, "have"))
	    Strcpy(buf, "has");
	else if (index("zxs", *spot) ||
		 (len >= 2 && *spot=='h' && index("cs", *(spot-1))) ||
		 (len == 2 && *spot == 'o')) {
	    /* Ends in z, x, s, ch, sh; add an "es" */
	    Strcpy(buf, verb);
	    Strcat(buf, "es");
	} else if (*spot == 'y' && (!index(vowels, *(spot-1)))) {
	    /* like "y" case in makeplural */
	    Strcpy(buf, verb);
	    Strcpy(buf + len - 1, "ies");
	} else {
	    Strcpy(buf, verb);
	    Strcat(buf, "s");
	}

	return buf;
}
#endif

/* capitalized variant of doname() */
char *
Doname2(obj)
register struct obj *obj;
{
	register char *s = doname(obj);

	*s = highc(*s);
	return(s);
}

/* returns "your xname(obj)" or "Foobar's xname(obj)" or "the xname(obj)" */
char *
yname(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();
	char *s = shk_your(outbuf, obj);	/* assert( s == outbuf ); */
#if 0 /*JP*/
	int space_left = BUFSZ - strlen(s) - sizeof " ";

	return strncat(strcat(s, " "), cxname(obj), space_left);
#else
	int space_left = BUFSZ - strlen(s);

	return strncat(s, xname(obj), space_left);
#endif
}

/* capitalized variant of yname() */
char *
Yname2(obj)
struct obj *obj;
{
	char *s = yname(obj);

#if 0 /*JP*/
	*s = highc(*s);
#endif
	return s;
}

/* returns "your simple_typename(obj->otyp)"
 * or "Foobar's simple_typename(obj->otyp)"
 * or "the simple_typename(obj-otyp)"
 */
char *
ysimple_name(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();
	char *s = shk_your(outbuf, obj);	/* assert( s == outbuf ); */
#if 0 /*JP*/
	int space_left = BUFSZ - strlen(s) - sizeof " ";

	return strncat(strcat(s, " "), simple_typename(obj->otyp), space_left);
#else
	int space_left = BUFSZ - strlen(s);

	return strncat(s, simple_typename(obj->otyp), space_left);
#endif
}

/* capitalized variant of ysimple_name() */
char *
Ysimple_name2(obj)
struct obj *obj;
{
	char *s = ysimple_name(obj);

#if 0 /*JP*/
	*s = highc(*s);
#endif
	return s;
}

static const char *wrp[] = {
	"wand", "ring", "potion", "scroll", "gem", "amulet",
	"spellbook", "spell book",
	/* for non-specific wishes */
	"weapon", "armor", "armour", "tool", "food", "comestible",
};
static const char wrpsym[] = {
	WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS, GEM_CLASS,
	AMULET_CLASS, SPBOOK_CLASS, SPBOOK_CLASS,
	WEAPON_CLASS, ARMOR_CLASS, ARMOR_CLASS, TOOL_CLASS, FOOD_CLASS,
	FOOD_CLASS
};

#endif /* OVLB */
#ifdef OVL0

/* Plural routine; chiefly used for user-defined fruits.  We have to try to
 * account for everything reasonable the player has; something unreasonable
 * can still break the code.  However, it's still a lot more accurate than
 * "just add an s at the end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi.")
 * and body parts.
 *
 * Also misused by muse.c to convert 1st person present verbs to 2nd person.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
/*JP: Japanese is simple... */
#if 1 /*JP*/
	char *str = nextobuf();
	Strcpy(str, oldstr);
#else
	/* Note: cannot use strcmpi here -- it'd give MATZot, CAVEMeN,... */
	register char *spot;
	char *str = nextobuf();
	const char *excess = (char *)0;
	int len;

	while (*oldstr==' ') oldstr++;
	if (!oldstr || !*oldstr) {
		impossible("plural of null?");
		Strcpy(str, "s");
		return str;
	}
	Strcpy(str, oldstr);

	/*
	 * Skip changing "pair of" to "pairs of".  According to Webster, usual
	 * English usage is use pairs for humans, e.g. 3 pairs of dancers,
	 * and pair for objects and non-humans, e.g. 3 pair of boots.  We don't
	 * refer to pairs of humans in this game so just skip to the bottom.
	 */
	if (!strncmp(str, "pair of ", 8))
		goto bottom;

	/* Search for common compounds, ex. lump of royal jelly */
	for(spot=str; *spot; spot++) {
		if (!strncmp(spot, " of ", 4)
				|| !strncmp(spot, " labeled ", 9)
				|| !strncmp(spot, " called ", 8)
				|| !strncmp(spot, " named ", 7)
				|| !strcmp(spot, " above") /* lurkers above */
				|| !strncmp(spot, " versus ", 8)
				|| !strncmp(spot, " from ", 6)
				|| !strncmp(spot, " in ", 4)
				|| !strncmp(spot, " on ", 4)
				|| !strncmp(spot, " a la ", 6)
				|| !strncmp(spot, " with", 5)	/* " with "? */
				|| !strncmp(spot, " de ", 4)
				|| !strncmp(spot, " d'", 3)
				|| !strncmp(spot, " du ", 4)) {
			excess = oldstr + (int) (spot - str);
			*spot = 0;
			break;
		}
	}
	spot--;
	while (*spot==' ') spot--; /* Strip blanks from end */
	*(spot+1) = 0;
	/* Now spot is the last character of the string */

	len = strlen(str);

	/* Single letters */
	if (len==1 || !letter(*spot)) {
		Strcpy(spot+1, "'s");
		goto bottom;
	}

	/* Same singular and plural; mostly Japanese words except for "manes" */
	if ((len == 2 && !strcmp(str, "ya")) ||
	    (len >= 2 && !strcmp(spot-1, "ai")) || /* samurai, Uruk-hai */
	    (len >= 3 && !strcmp(spot-2, " ya")) ||
	    (len >= 4 &&
	     (!strcmp(spot-3, "fish") || !strcmp(spot-3, "tuna") ||
	      !strcmp(spot-3, "deer") || !strcmp(spot-3, "yaki"))) ||
	    (len >= 5 && (!strcmp(spot-4, "sheep") ||
			!strcmp(spot-4, "ninja") ||
			!strcmp(spot-4, "ronin") ||
			!strcmp(spot-4, "shito") ||
			!strcmp(spot-7, "shuriken") ||
			!strcmp(spot-4, "tengu") ||
			!strcmp(spot-4, "manes"))) ||
	    (len >= 6 && !strcmp(spot-5, "ki-rin")) ||
	    (len >= 7 && !strcmp(spot-6, "gunyoki")))
		goto bottom;

	/* man/men ("Wiped out all cavemen.") */
	if (len >= 3 && !strcmp(spot-2, "man") &&
			(len<6 || strcmp(spot-5, "shaman")) &&
			(len<5 || strcmp(spot-4, "human"))) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* tooth/teeth */
	if (len >= 5 && !strcmp(spot-4, "tooth")) {
		Strcpy(spot-3, "eeth");
		goto bottom;
	}

	/* knife/knives, etc... */
	if (!strcmp(spot-1, "fe")) {
		Strcpy(spot-1, "ves");
		goto bottom;
	} else if (*spot == 'f') {
		if (index("lr", *(spot-1)) || index(vowels, *(spot-1))) {
			Strcpy(spot, "ves");
			goto bottom;
		} else if (len >= 5 && !strncmp(spot-4, "staf", 4)) {
			Strcpy(spot-1, "ves");
			goto bottom;
		}
	}

	/* foot/feet (body part) */
	if (len >= 4 && !strcmp(spot-3, "foot")) {
		Strcpy(spot-2, "eet");
		goto bottom;
	}

	/* ium/ia (mycelia, baluchitheria) */
	if (len >= 3 && !strcmp(spot-2, "ium")) {
		*(spot--) = (char)0;
		*spot = 'a';
		goto bottom;
	}

	/* algae, larvae, hyphae (another fungus part) */
	if ((len >= 4 && !strcmp(spot-3, "alga")) ||
	    (len >= 5 &&
	     (!strcmp(spot-4, "hypha") || !strcmp(spot-4, "larva")))) {
		Strcpy(spot, "ae");
		goto bottom;
	}

	/* fungus/fungi, homunculus/homunculi, but buses, lotuses, wumpuses */
	if (len > 3 && !strcmp(spot-1, "us") &&
	    (len < 5 || (strcmp(spot-4, "lotus") &&
			 (len < 6 || strcmp(spot-5, "wumpus"))))) {
		*(spot--) = (char)0;
		*spot = 'i';
		goto bottom;
	}

	/* vortex/vortices */
	if (len >= 6 && !strcmp(spot-3, "rtex")) {
		Strcpy(spot-1, "ices");
		goto bottom;
	}

	/* djinni/djinn (note: also efreeti/efreet) */
	if (len >= 6 && !strcmp(spot-5, "djinni")) {
		*spot = (char)0;
		goto bottom;
	}

	/* mumak/mumakil */
	if (len >= 5 && !strcmp(spot-4, "mumak")) {
		Strcpy(spot+1, "il");
		goto bottom;
	}

	/* sis/ses (nemesis) */
	if (len >= 3 && !strcmp(spot-2, "sis")) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* erinys/erinyes */
	if (len >= 6 && !strcmp(spot-5, "erinys")) {
		Strcpy(spot, "es");
		goto bottom;
	}

	/* mouse/mice,louse/lice (not a monster, but possible in food names) */
	if (len >= 5 && !strcmp(spot-3, "ouse") && index("MmLl", *(spot-4))) {
		Strcpy(spot-3, "ice");
		goto bottom;
	}

	/* matzoh/matzot, possible food name */
	if (len >= 6 && (!strcmp(spot-5, "matzoh")
					|| !strcmp(spot-5, "matzah"))) {
		Strcpy(spot-1, "ot");
		goto bottom;
	}
	if (len >= 5 && (!strcmp(spot-4, "matzo")
					|| !strcmp(spot-5, "matza"))) {
		Strcpy(spot, "ot");
		goto bottom;
	}

	/* child/children (for wise guys who give their food funny names) */
	if (len >= 5 && !strcmp(spot-4, "child")) {
		Strcpy(spot, "dren");
		goto bottom;
	}

	/* note: -eau/-eaux (gateau, bordeau...) */
	/* note: ox/oxen, VAX/VAXen, goose/geese */

	/* Ends in z, x, s, ch, sh; add an "es" */
	if (index("zxs", *spot)
			|| (len >= 2 && *spot=='h' && index("cs", *(spot-1)))
	/* Kludge to get "tomatoes" and "potatoes" right */
			|| (len >= 4 && !strcmp(spot-2, "ato"))) {
		Strcpy(spot+1, "es");
		goto bottom;
	}

	/* Ends in y preceded by consonant (note: also "qu") change to "ies" */
	if (*spot == 'y' &&
	    (!index(vowels, *(spot-1)))) {
		Strcpy(spot, "ies");
		goto bottom;
	}

	/* Default: append an 's' */
	Strcpy(spot+1, "s");

bottom:	if (excess) Strcpy(eos(str), excess);
#endif /*JP*/
	return str;
}

#endif /* OVL0 */

struct o_range {
	const char *name, oclass;
	int  f_o_range, l_o_range;
};

#ifndef OVLB

STATIC_DCL const struct o_range o_ranges[];

#else /* OVLB */

/* wishable subranges of objects */
STATIC_OVL NEARDATA const struct o_range o_ranges[] = {
	{ "bag",	TOOL_CLASS,   SACK,	      BAG_OF_TRICKS },
	{ "lamp",	TOOL_CLASS,   OIL_LAMP,	      MAGIC_LAMP },
	{ "candle",	TOOL_CLASS,   TALLOW_CANDLE,  WAX_CANDLE },
	{ "horn",	TOOL_CLASS,   TOOLED_HORN,    HORN_OF_PLENTY },
	{ "shield",	ARMOR_CLASS,  SMALL_SHIELD,   SHIELD_OF_REFLECTION },
	{ "helm",	ARMOR_CLASS,  ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY },
	{ "gloves",	ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
	{ "gauntlets",	ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
	{ "boots",	ARMOR_CLASS,  LOW_BOOTS,      LEVITATION_BOOTS },
	{ "shoes",	ARMOR_CLASS,  LOW_BOOTS,      IRON_SHOES },
	{ "cloak",	ARMOR_CLASS,  MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT },
	{ "dress",	ARMOR_CLASS,  MAID_DRESS,     NURSE_UNIFORM },
#ifdef TOURIST
	{ "shirt",	ARMOR_CLASS,  HAWAIIAN_SHIRT, T_SHIRT },
#endif
	{ "dragon scales",
			ARMOR_CLASS,  GRAY_DRAGON_SCALES, YELLOW_DRAGON_SCALES },
	{ "dragon scale mail",
			ARMOR_CLASS,  GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL },
	{ "sword",	WEAPON_CLASS, SHORT_SWORD,    KATANA },
#ifdef WIZARD
	{ "venom",	VENOM_CLASS,  BLINDING_VENOM, ACID_VENOM },
#endif
	{ "gray stone",	GEM_CLASS,    LUCKSTONE,      FLINT },
	{ "grey stone",	GEM_CLASS,    LUCKSTONE,      FLINT },
};

#define BSTRCMP(base,ptr,string) ((ptr) < base || strcmp((ptr),string))
#define BSTRCMPI(base,ptr,string) ((ptr) < base || strcmpi((ptr),string))
#define BSTRNCMP(base,ptr,string,num) ((ptr)<base || strncmp((ptr),string,num))
#define BSTRNCMPI(base,ptr,string,num) ((ptr)<base||strncmpi((ptr),string,num))

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 */
/*JP
 * 基本的には単数化は必要ないのでコメントアウト。
 * 但し、願いで複数形の英語を入力すると判定されなくなるので、
 * それを避けたい場合は #define USE_MAKESINGULAR すれば単数化される。
 */
char *
makesingular(oldstr)
const char *oldstr;
{
#ifdef USE_MAKESINGULAR /*JP*/
	register char *p, *bp;
#else
	register char *bp;
#endif
	char *str = nextobuf();

	if (!oldstr || !*oldstr) {
		impossible("singular of null?");
		str[0] = 0;
		return str;
	}
	Strcpy(str, oldstr);
	bp = str;

#ifdef USE_MAKESINGULAR /*JP*//*日本語は単複同形*/
	while (*bp == ' ') bp++;
	/* find "cloves of garlic", "worthless pieces of blue glass" */
	if ((p = strstri(bp, "s of ")) != 0) {
	    /* but don't singularize "gauntlets", "boots", "Eyes of the.." */
	    if (BSTRNCMPI(bp, p-3, "Eye", 3) &&
		BSTRNCMP(bp, p-4, "boot", 4) &&
		BSTRNCMP(bp, p-8, "gauntlet", 8))
		while ((*p = *(p+1)) != 0) p++;
	    return bp;
	}

	/* remove -s or -es (boxes) or -ies (rubies) */
	p = eos(bp);
	if (p >= bp+1 && p[-1] == 's') {
		if (p >= bp+2 && p[-2] == 'e') {
			if (p >= bp+3 && p[-3] == 'i') {
				if(!BSTRCMP(bp, p-7, "cookies") ||
				   !BSTRCMP(bp, p-4, "pies"))
					goto mins;
				Strcpy(p-3, "y");
				return bp;
			}

			/* note: cloves / knives from clove / knife */
			if(!BSTRCMP(bp, p-6, "knives")) {
				Strcpy(p-3, "fe");
				return bp;
			}
			if(!BSTRCMP(bp, p-6, "staves")) {
				Strcpy(p-3, "ff");
				return bp;
			}
			if (!BSTRCMPI(bp, p-6, "leaves")) {
				Strcpy(p-3, "f");
				return bp;
			}
			if (!BSTRCMP(bp, p-8, "vortices")) {
				Strcpy(p-4, "ex");
				return bp;
			}

			/* note: nurses, axes but boxes */
			if (!BSTRCMP(bp, p-5, "boxes") ||
			    !BSTRCMP(bp, p-4, "ches")) {
				p[-2] = '\0';
				return bp;
			}

			if (!BSTRCMP(bp, p-6, "gloves") ||
			    !BSTRCMP(bp, p-6, "lenses") ||
			    !BSTRCMP(bp, p-5, "shoes") ||
			    !BSTRCMP(bp, p-6, "scales"))
				return bp;

		} else if (!BSTRCMP(bp, p-5, "boots") ||
			   !BSTRCMP(bp, p-9, "gauntlets") ||
			   !BSTRCMP(bp, p-6, "tricks") ||
			   !BSTRCMP(bp, p-9, "paralysis") ||
			   !BSTRCMP(bp, p-5, "glass") ||
			   !BSTRCMP(bp, p-4, "ness") ||
			   !BSTRCMP(bp, p-14, "shape changers") ||
			   !BSTRCMP(bp, p-15, "detect monsters") ||
			   !BSTRCMPI(bp, p-11, "Aesculapius") || /* staff */
			   !BSTRCMP(bp, p-10, "eucalyptus") ||
#ifdef WIZARD
			   !BSTRCMP(bp, p-9, "iron bars") ||
#endif
			   !BSTRCMP(bp, p-5, "aklys") ||
			   !BSTRCMP(bp, p-6, "fungus"))
				return bp;
	mins:
		p[-1] = '\0';

	} else {

		if(!BSTRCMP(bp, p-5, "teeth")) {
			Strcpy(p-5, "tooth");
			return bp;
		}

		if (!BSTRCMP(bp, p-5, "fungi")) {
			Strcpy(p-5, "fungus");
			return bp;
		}

		/* here we cannot find the plural suffix */
	}
#endif
	return bp;
}

/* compare user string against object name string using fuzzy matching */
static boolean
wishymatch(u_str, o_str, retry_inverted)
const char *u_str;	/* from user, so might be variant spelling */
const char *o_str;	/* from objects[], so is in canonical form */
boolean retry_inverted;	/* optional extra "of" handling */
{
	/* special case: wizards can wish for traps.  The object is "beartrap"
	 * and the trap is "bear trap", so to let wizards wish for both we
	 * must not fuzzymatch.
	 */
#ifdef WIZARD
	if (wizard && !strcmp(o_str, "beartrap"))
	    return !strncmpi(o_str, u_str, 8);
#endif

	/* ignore spaces & hyphens and upper/lower case when comparing */
	if (fuzzymatch(u_str, o_str, " -", TRUE)) return TRUE;

	if (retry_inverted) {
	    const char *u_of, *o_of;
	    char *p, buf[BUFSZ];

	    /* when just one of the strings is in the form "foo of bar",
	       convert it into "bar foo" and perform another comparison */
	    u_of = strstri(u_str, " of ");
	    o_of = strstri(o_str, " of ");
	    if (u_of && !o_of) {
		Strcpy(buf, u_of + 4);
		p = eos(strcat(buf, " "));
		while (u_str < u_of) *p++ = *u_str++;
		*p = '\0';
		return fuzzymatch(buf, o_str, " -", TRUE);
	    } else if (o_of && !u_of) {
		Strcpy(buf, o_of + 4);
		p = eos(strcat(buf, " "));
		while (o_str < o_of) *p++ = *o_str++;
		*p = '\0';
		return fuzzymatch(u_str, buf, " -", TRUE);
	    }
	}

	/* [note: if something like "elven speed boots" ever gets added, these
	   special cases should be changed to call wishymatch() recursively in
	   order to get the "of" inversion handling] */
	if (!strncmp(o_str, "dwarvish ", 9)) {
	    if (!strncmpi(u_str, "dwarven ", 8))
		return fuzzymatch(u_str + 8, o_str + 9, " -", TRUE);
	} else if (!strncmp(o_str, "elven ", 6)) {
	    if (!strncmpi(u_str, "elvish ", 7))
		return fuzzymatch(u_str + 7, o_str + 6, " -", TRUE);
	    else if (!strncmpi(u_str, "elfin ", 6))
		return fuzzymatch(u_str + 6, o_str + 6, " -", TRUE);
	} else if (!strcmp(o_str, "aluminum")) {
		/* this special case doesn't really fit anywhere else... */
		/* (note that " wand" will have been stripped off by now) */
	    if (!strcmpi(u_str, "aluminium"))
		return fuzzymatch(u_str + 9, o_str + 8, " -", TRUE);
	}

	return FALSE;
}

/* alternate spellings; if the difference is only the presence or
   absence of spaces and/or hyphens (such as "pickaxe" vs "pick axe"
   vs "pick-axe") then there is no need for inclusion in this list;
   likewise for ``"of" inversions'' ("boots of speed" vs "speed boots") */
struct alt_spellings {
	const char *sp;
	int ob;
} spellings[] = {
	{ "pickax", PICK_AXE },
	{ "whip", BULLWHIP },
	//{ "saber", SILVER_SABER },
	{ "silver sabre", SILVER_SABER },
	{ "smooth shield", SHIELD_OF_REFLECTION },
	{ "grey dragon scale mail", GRAY_DRAGON_SCALE_MAIL },
	{ "grey dragon scales", GRAY_DRAGON_SCALES },
	{ "enchant armour", SCR_ENCHANT_ARMOR },
	{ "destroy armour", SCR_DESTROY_ARMOR },
	{ "scroll of enchant armour", SCR_ENCHANT_ARMOR },
	{ "scroll of destroy armour", SCR_DESTROY_ARMOR },
	{ "leather armour", LEATHER_ARMOR },
	{ "studded leather armour", STUDDED_LEATHER_ARMOR },
	{ "iron ball", HEAVY_IRON_BALL },
	{ "lantern", BRASS_LANTERN },
	{ "mattock", DWARVISH_MATTOCK },
	{ "amulet of poison resistance", AMULET_VERSUS_POISON },
	{ "stone", ROCK },
#ifdef TOURIST
	{ "camera", EXPENSIVE_CAMERA },
	{ "tee shirt", T_SHIRT },
#endif
	{ "can", TIN },
	{ "can opener", TIN_OPENER },
	{ "kelp", KELP_FROND },
	{ "eucalyptus", EUCALYPTUS_LEAF },
	{ "grapple", GRAPPLING_HOOK },
	{ (const char *)0, 0 },
};

/*
 * Return something wished for.  Specifying a null pointer for
 * the user request string results in a random object.  Otherwise,
 * if asking explicitly for "nothing" (or "nil") return no_wish;
 * if not an object return &zeroobj; if an error (no matching object),
 * return null.
 * If from_user is false, we're reading from the wizkit, nothing was typed in.
 */

struct obj *
readobjnam(bp, no_wish, from_user)
register char *bp;
struct obj *no_wish;
boolean from_user;
{
	register char *p;
	register int i;
	register struct obj *otmp;
	int cnt, spe, spesgn, typ, very, rechrg;
	int blessed, uncursed, iscursed, ispoisoned, isgreased;
	int eroded, eroded2, erodeproof;
#ifdef INVISIBLE_OBJECTS
	int isinvisible;
#endif
	int halfeaten, mntmp, contents;
	int islit, unlabeled, ishistoric, isdiluted;
	struct fruit *f;
	int ftype = current_fruit;
	char fruitbuf[BUFSZ];
#if 1 /*JP*/
	char buf[BUFSZ];
	char pfx[BUFSZ];
#endif
	/* Fruits may not mess up the ability to wish for real objects (since
	 * you can leave a fruit in a bones file and it will be added to
	 * another person's game), so they must be checked for last, after
	 * stripping all the possible prefixes and seeing if there's a real
	 * name in there.  So we have to save the full original name.  However,
	 * it's still possible to do things like "uncursed burnt Alaska",
	 * or worse yet, "2 burned 5 course meals", so we need to loop to
	 * strip off the prefixes again, this time stripping only the ones
	 * possible on food.
	 * We could get even more detailed so as to allow food names with
	 * prefixes that _are_ possible on food, so you could wish for
	 * "2 3 alarm chilis".  Currently this isn't allowed; options.c
	 * automatically sticks 'candied' in front of such names.
	 */

	char oclass;
	char *un, *dn, *actualn;
	const char *name=0;

	cnt = spe = spesgn = typ = very = rechrg =
		blessed = uncursed = iscursed =
#ifdef INVISIBLE_OBJECTS
		isinvisible =
#endif
		ispoisoned = isgreased = eroded = eroded2 = erodeproof =
		halfeaten = islit = unlabeled = ishistoric = isdiluted = 0;
	mntmp = NON_PM;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
	contents = UNDEFINED;
	oclass = 0;
	actualn = dn = un = 0;

	if (!bp) goto any;

#if 1 /*JP*/
	/* 2バイト文字のスペースを削除 */
#undef WISHDEBUG

#ifdef WISHDEBUG
	pline("Wish DEBUG[%s]\n", bp);
#endif
	while(delete(bp, "　"))
	      ;
#ifdef WISHDEBUG
	pline("Wish DEBUG[%s]\n", bp);
#endif
	Strcpy(buf, bp);

#define S(a, b)	substitute(buf, (a), (b))
#define T(a)	transpose(buf, (a))
#define D(a)	delete(buf, (a))
#define I(a)	insert(buf, (a))

/* 特殊2バイト文字を1バイト文字へ変換 */

	while(S("＋", "+") ||
	      S("−", "-") ||
	      S("（", "(") ||
	      S("）", ")") ||
	      S("０", "0") ||
	      S("１", "1") ||
	      S("２", "2") ||
	      S("３", "3") ||
	      S("４", "4") ||
	      S("５", "5") ||
	      S("６", "6") ||
	      S("７", "7") ||
	      S("８", "8") ||
	      S("９", "9"))
	  ;

#ifdef WISHDEBUG
	pline("Wish DEBUG0[%s]\n", bp);
#endif

	T("と名づけられた");
	T("と呼ばれる");
	T("と言う名の");
	T("という名の");

	S("と名づけられた", " named ");
	S("と呼ばれる", " called ");
	S("と言う名の", " named ");
	S("という名の", " named ");

	T("ホウレン草の");
	S("ホウレン草の", " of spinach "); 

	T("の死体");
	S("の死体", " corpse of "); 

	T("の死骸");
	S("の死骸", " corpse of "); 

	T("の像");
	S("の像", " statue of "); 

	T("の人形");
	S("の人形", " figurine of "); 

	T("の卵");
	S("の卵", " egg of ");

	S("の肉の缶", "の缶");
	T("の缶");
	S("の缶", " tin of ");

	S("祝福された", "blessed ");
	S("聖水", "holy water ");
	S("聖なる", "blessed ");

	S("呪われた", "cursed ");
	S("不浄な", "cursed ");

	S("呪われていない", "uncursed ");

	S("錆びない", "rustproof ");
	S("腐食しない", "erodeproof ");
	S("燃えない", "corrodeproof ");
	S("傷つかない", "fireproof ");

	S("光っている", "lit ");
	S("燃えている", "burning ");
	S("消えている", "unlit ");
	S("ラベルのない", "unlabelled ");
	S("真っ白な", "blank ");

	S("毒の塗られた", "poisoned ");
	S("油の塗られた", "greased ");
	S("脂の塗られた", "greased ");

	S("とても", "very ");
	S("かなり", "thoroughly ");

	S("錆びた", "rusty ");
	S("腐食した", "eroded ");
	S("傷ついた", "damaged ");
	S("腐った", "rotted ");
	S("燃えた", "burned ");

	S("食べかけの", "partly eaten ");

	S("薄い", "diluted ");
	S("薄まった", "diluted ");

	S("空の", "empty ");
	S("空っぽの", "empty ");

	D("一式");

	if(strlen(buf)>4 && !strcmp(buf + strlen(buf) - 4, "巻物"))
	  Strcpy(pfx, "scroll of ");
	else if(strlen(buf)>6 && !strcmp(buf + strlen(buf) - 6, "巻き物"))
	  Strcpy(pfx, "scroll of ");
	else if(strlen(buf)>6 && !strcmp(buf + strlen(buf) - 6, "魔法書"))
	  Strcpy(pfx, "spellbook of ");
	else if(strlen(buf)>2 && !strcmp(buf + strlen(buf) - 2, "杖"))
	  Strcpy(pfx, "wand of ");
	else if(strlen(buf)>4 && !strcmp(buf + strlen(buf) - 4, "指輪"))
	  Strcpy(pfx, "ring of ");
	else if(strlen(buf)>2 && !strcmp(buf + strlen(buf) - 2, "薬"))
	  Strcpy(pfx, "potion of ");
	else
	  pfx[0] = '\0';

	bp = buf;
#ifdef WISHDEBUG
	pline("Wish DEBUG1[%s]\n", bp);
#endif
#endif /*JP*/
	/* first, remove extra whitespace they may have typed */
	(void)mungspaces(bp);
	/* allow wishing for "nothing" to preserve wishless conduct...
	   [now requires "wand of nothing" if that's what was really wanted] */
	if (!strcmpi(bp, "nothing") || !strcmpi(bp, "nil") ||
	    !strcmpi(bp, "none")) return no_wish;
	/* save the [nearly] unmodified choice string */
	Strcpy(fruitbuf, bp);

	for(;;) {
		register int l;

		if (!bp || !*bp) goto any;
		if (!strncmpi(bp, "an ", l=3) ||
		    !strncmpi(bp, "a ", l=2)) {
			cnt = 1;
		} else if (!strncmpi(bp, "the ", l=4)) {
			;	/* just increment `bp' by `l' below */
#if 0 /*JP*/
		} else if (!cnt && digit(*bp) && strcmp(bp, "0")) {
			cnt = atoi(bp);
			while(digit(*bp)) bp++;
#else
		} else if (!cnt && digit_8(*bp) && strcmp(bp, "0")) {
			cnt = atoi_8(bp);
			while(digit_8(*bp)) bp++;
#endif
			while(*bp == ' ') bp++;
			l = 0;
#if 1 /*JP*/
/* 後に数詞があるときは削除 */
			if(!strncmp(bp, "冊の", l = 4) ||
			   !strncmp(bp, "本の", l = 4) ||
			   !strncmp(bp, "着の", l = 4) ||
			   !strncmp(bp, "個の", l = 4) ||
			   !strncmp(bp, "枚の", l = 4) ||
			   !strncmp(bp, "つの", l = 4) ||
			   !strncmp(bp, "の", l = 2))
			  ;
			else
			  l = 0;
#endif
#if 1 /*JP*/
 /*
漢字で数字を指定するときは数詞が必要
*/
		} else if(!cnt && 
			  (!strncmp(bp + 2, "冊の", l = 4) ||
			   !strncmp(bp + 2, "本の", l = 4) ||
			   !strncmp(bp + 2, "着の", l = 4) ||
			   !strncmp(bp + 2, "個の", l = 4) ||
			   !strncmp(bp + 2, "枚の", l = 4) ||
			   !strncmp(bp + 2, "つの", l = 4) ||
			   !strncmp(bp + 2, "の", l = 2))){
		  if(!strncmp(bp, "一", 2)){
		    cnt = 1;
		  }
		  else if(!strncmp(bp, "二", 2)){
		    cnt = 2;
		  }
		  else if(!strncmp(bp, "三", 2)){
		    cnt = 3;
		  }
		  else if(!strncmp(bp, "四", 2)){
		    cnt = 4;
		  }
		  else if(!strncmp(bp, "五", 2)){
		    cnt = 5;
		  }
		  else if(!strncmp(bp, "六", 2)){
		    cnt = 6;
		  }
		  else if(!strncmp(bp, "七", 2)){
		    cnt = 7;
		  }
		  else if(!strncmp(bp, "八", 2)){
		    cnt = 8;
		  }
		  else if(!strncmp(bp, "九", 2)){
		    cnt = 9;
		  }
		  else if(!strncmp(bp, "十", 2)){
		    cnt = 10;
		  }
		  if(cnt)
		    l += 2;
		  else{
		    l = 0;
		    cnt = 1;
		  }
#endif
		} else if (*bp == '+' || *bp == '-') {
			spesgn = (*bp++ == '+') ? 1 : -1;
#if 0 /*JP*/
			spe = atoi(bp);
			while(digit(*bp)) bp++;
#else
			spe = atoi_8(bp);
			while(digit_8(*bp)) bp++;
#endif
			while(*bp == ' ') bp++;
			l = 0;
		} else if (!strncmpi(bp, "blessed ", l=8) ||
			   !strncmpi(bp, "holy ", l=5)) {
			blessed = 1;
		} else if (!strncmpi(bp, "cursed ", l=7) ||
			   !strncmpi(bp, "unholy ", l=7)) {
			iscursed = 1;
		} else if (!strncmpi(bp, "uncursed ", l=9)) {
			uncursed = 1;
#ifdef INVISIBLE_OBJECTS
		} else if (!strncmpi(bp, "invisible ", l=10)) {
			isinvisible = 1;
#endif
		} else if (!strncmpi(bp, "rustproof ", l=10) ||
			   !strncmpi(bp, "erodeproof ", l=11) ||
			   !strncmpi(bp, "corrodeproof ", l=13) ||
			   !strncmpi(bp, "fixed ", l=6) ||
			   !strncmpi(bp, "fireproof ", l=10) ||
			   !strncmpi(bp, "rotproof ", l=9)) {
			erodeproof = 1;
		} else if (!strncmpi(bp,"lit ", l=4) ||
			   !strncmpi(bp,"burning ", l=8)) {
			islit = 1;
		} else if (!strncmpi(bp,"unlit ", l=6) ||
			   !strncmpi(bp,"extinguished ", l=13)) {
			islit = 0;
		/* "unlabeled" and "blank" are synonymous */
		} else if (!strncmpi(bp,"unlabeled ", l=10) ||
			   !strncmpi(bp,"unlabelled ", l=11) ||
			   !strncmpi(bp,"blank ", l=6)) {
			unlabeled = 1;
		} else if(!strncmpi(bp, "poisoned ",l=9)
#ifdef WIZARD
			  || (wizard && !strncmpi(bp, "trapped ",l=8))
#endif
			  ) {
			ispoisoned=1;
		} else if(!strncmpi(bp, "greased ",l=8)) {
			isgreased=1;
		} else if (!strncmpi(bp, "very ", l=5)) {
			/* very rusted very heavy iron ball */
			very = 1;
		} else if (!strncmpi(bp, "thoroughly ", l=11)) {
			very = 2;
		} else if (!strncmpi(bp, "rusty ", l=6) ||
			   !strncmpi(bp, "rusted ", l=7) ||
			   !strncmpi(bp, "burnt ", l=6) ||
			   !strncmpi(bp, "burned ", l=7)) {
			eroded = 1 + very;
			very = 0;
		} else if (!strncmpi(bp, "corroded ", l=9) ||
			   !strncmpi(bp, "rotted ", l=7)) {
			eroded2 = 1 + very;
			very = 0;
		} else if (!strncmpi(bp, "partly eaten ", l=13)) {
			halfeaten = 1;
		} else if (!strncmpi(bp, "historic ", l=9)) {
			ishistoric = 1;
		} else if (!strncmpi(bp, "diluted ", l=8)) {
			isdiluted = 1;
		} else if(!strncmpi(bp, "empty ", l=6)) {
			contents = EMPTY;
		} else break;
		bp += l;
	}
	if(!cnt) cnt = 1;		/* %% what with "gems" etc. ? */
	if (strlen(bp) > 1) {
	    if ((p = rindex(bp, '(')) != 0) {
		if (p > bp && p[-1] == ' ') p[-1] = 0;
		else *p = 0;
		p++;
		if (!strcmpi(p, "lit)")) {
		    islit = 1;
		} else {
/*JP
		    spe = atoi(p);
*/
		    spe = atoi_8(p);
		    while (digit(*p)) p++;
		    if (*p == ':') {
			p++;
			rechrg = spe;
			spe = atoi(p);
			while (digit(*p)) p++;
		    }
		    if (*p != ')') {
			spe = rechrg = 0;
		    } else {
			spesgn = 1;
			p++;
			if (*p) Strcat(bp, p);
		    }
		}
	    }
	}
#if 1 /*JP*/
	while(*bp == ' ')
	  ++bp;
/*
天邪鬼のために 1から10まではサポート。
*/
	if(!strncmp(bp, "一", 2)){
	  spe = 1; bp += 2;
	}
	else if(!strncmp(bp, "二", 2)){
	  spe = 2; bp += 2;
	}
	else if(!strncmp(bp, "三", 2)){
	  spe = 3; bp += 2;
	}
	else if(!strncmp(bp, "四", 2)){
	  spe = 4; bp += 2;
	}
	else if(!strncmp(bp, "五", 2)){
	  spe = 5; bp += 2;
	}
	else if(!strncmp(bp, "六", 2)){
	  spe = 6; bp += 2;
	}
	else if(!strncmp(bp, "七", 2)){
	  spe = 7; bp += 2;
	}
	else if(!strncmp(bp, "八", 2)){
	  spe = 8; bp += 2;
	}
	else if(!strncmp(bp, "九", 2)){
	  spe = 9; bp += 2;
	}
	else if(!strncmp(bp, "十", 2)){
	  spe = 10; bp += 2;
	}

	while(*bp == ' ')
	  ++bp;

	while(1)
	{
	  unsigned char *bp2, *bp1;
	  unsigned char buf1[BUFSZ];
	  const char *en;

	  bp1 = buf1;
	  bp2 = (unsigned char *)bp;

	  while(!(*bp2 & 0x80))
	    ++bp2;

	  while(*bp2 & 0x80){
	    *(bp1++) = *(bp2++);
	    *(bp1++) = *(bp2++);
	  }
	  *bp1 = '\0';

#ifdef WISHDEBUG
	  pline("Wish DEBUG2[%s]\n", bp);
#endif
	  en = etrns_obj(' ', buf1);
	  if(!strncmpi(en, "The Staff of Aesculapius", 24))
		  substitute(pfx, "wand of ", "");
	  Strcat(pfx, en);
	  if(!strcmp(en, buf1) || !substitute(bp, buf1, pfx))
	    break;
	}
#ifdef WISHDEBUG
	pline("Wish DEBUG3[%s]", bp);
#endif
#endif
/*
   otmp->spe is type schar; so we don't want spe to be any bigger or smaller.
   also, spe should always be positive  -- some cheaters may try to confuse
   atoi()
*/
	if (spe < 0) {
		spesgn = -1;	/* cheaters get what they deserve */
		spe = abs(spe);
	}
	if (spe > SCHAR_LIM)
		spe = SCHAR_LIM;
	if (rechrg < 0 || rechrg > 7) rechrg = 7;	/* recharge_limit */

	/* now we have the actual name, as delivered by xname, say
		green potions called whisky
		scrolls labeled "QWERTY"
		egg
		fortune cookies
		very heavy iron ball named hoei
		wand of wishing
		elven cloak
	*/
	if ((p = strstri(bp, " named ")) != 0) {
		*p = 0;
		name = p+7;
	}
	if ((p = strstri(bp, " called ")) != 0) {
		*p = 0;
		un = p+8;
		/* "helmet called telepathy" is not "helmet" (a specific type)
		 * "shield called reflection" is not "shield" (a general type)
		 */
		for(i = 0; i < SIZE(o_ranges); i++)
		    if(!strcmpi(bp, o_ranges[i].name)) {
			oclass = o_ranges[i].oclass;
			goto srch;
		    }
	}
	if ((p = strstri(bp, " labeled ")) != 0) {
		*p = 0;
		dn = p+9;
	} else if ((p = strstri(bp, " labelled ")) != 0) {
		*p = 0;
		dn = p+10;
	}
	if ((p = strstri(bp, " of spinach")) != 0) {
		*p = 0;
		contents = SPINACH;
	}

	/*
	Skip over "pair of ", "pairs of", "set of" and "sets of".

	Accept "3 pair of boots" as well as "3 pairs of boots". It is valid
	English either way.  See makeplural() for more on pair/pairs.

	We should only double count if the object in question is not
	refered to as a "pair of".  E.g. We should double if the player
	types "pair of spears", but not if the player types "pair of
	lenses".  Luckily (?) all objects that are refered to as pairs
	-- boots, gloves, and lenses -- are also not mergable, so cnt is
	ignored anyway.
	*/
	if(!strncmpi(bp, "pair of ",8)) {
		bp += 8;
		cnt *= 2;
	} else if(cnt > 1 && !strncmpi(bp, "pairs of ",9)) {
		bp += 9;
		cnt *= 2;
	} else if (!strncmpi(bp, "set of ",7)) {
		bp += 7;
	} else if (!strncmpi(bp, "sets of ",8)) {
		bp += 8;
	}

	/*
	 * Find corpse type using "of" (figurine of an orc, tin of orc meat)
	 * Don't check if it's a wand or spellbook.
	 * (avoid "wand/finger of death" confusion).
	 */
	if (!strstri(bp, "wand ")
	 && !strstri(bp, "spellbook ")
	 && !strstri(bp, "finger ")) {
	    if ((p = strstri(bp, " of ")) != 0
		&& (mntmp = name_to_mon(p+4)) >= LOW_PM)
		*p = 0;
	}
	/* Find corpse type w/o "of" (red dragon scale mail, yeti corpse) */
	if (strncmpi(bp, "samurai sword", 13)) /* not the "samurai" monster! */
	if (strncmpi(bp, "wizard lock", 11)) /* not the "wizard" monster! */
	if (strncmpi(bp, "ninja-to", 8)) /* not the "ninja" rank */
	if (strncmpi(bp, "master key", 10)) /* not the "master" rank */
	if (strncmpi(bp, "magenta", 7)) /* not the "mage" rank */
	if (mntmp < LOW_PM && strlen(bp) > 2 &&
	    (mntmp = name_to_mon(bp)) >= LOW_PM) {
		int mntmptoo, mntmplen;	/* double check for rank title */
		char *obp = bp;
		mntmptoo = title_to_mon(bp, (int *)0, &mntmplen);
		bp += mntmp != mntmptoo ? (int)strlen(mons[mntmp].mname) : mntmplen;
		if (*bp == ' ') bp++;
		else if (!strncmpi(bp, "s ", 2)) bp += 2;
		else if (!strncmpi(bp, "es ", 3)) bp += 3;
		else if (!*bp && !actualn && !dn && !un && !oclass) {
		    /* no referent; they don't really mean a monster type */
		    bp = obp;
		    mntmp = NON_PM;
		}
	}

	/* first change to singular if necessary */
	if (*bp) {
		char *sng = makesingular(bp);
		if (strcmp(bp, sng)) {
			if (cnt == 1) cnt = 2;
			Strcpy(bp, sng);
		}
	}

	/* Alternate spellings (pick-ax, silver sabre, &c) */
    {
	struct alt_spellings *as = spellings;

	while (as->sp) {
		if (fuzzymatch(bp, as->sp, " -", TRUE)) {
			typ = as->ob;
			goto typfnd;
		}
		as++;
	}
	/* can't use spellings list for this one due to shuffling */
	if (!strncmpi(bp, "grey spell", 10))
		*(bp + 2) = 'a';
    }

	/* dragon scales - assumes order of dragons */
	if(!strcmpi(bp, "scales") &&
			mntmp >= PM_GRAY_DRAGON && mntmp <= PM_YELLOW_DRAGON) {
		typ = GRAY_DRAGON_SCALES + mntmp - PM_GRAY_DRAGON;
		mntmp = NON_PM;	/* no monster */
		goto typfnd;
	}

	p = eos(bp);
	if(!BSTRCMPI(bp, p-10, "holy water")) {
		typ = POT_WATER;
		if ((p-bp) >= 12 && *(p-12) == 'u')
			iscursed = 1; /* unholy water */
		else blessed = 1;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-6, "scroll")) {
		typ = SCR_BLANK_PAPER;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-9, "spellbook")) {
		typ = SPE_BLANK_PAPER;
		goto typfnd;
	}
	/*
	 * NOTE: Gold pieces are handled as objects nowadays, and therefore
	 * this section should probably be reconsidered as well as the entire
	 * gold/money concept.  Maybe we want to add other monetary units as
	 * well in the future. (TH)
	 */
	if(!BSTRCMPI(bp, p-10, "gold piece") || !BSTRCMPI(bp, p-7, "zorkmid") ||
	   !strcmpi(bp, "gold") || !strcmpi(bp, "money") ||
	   !strcmpi(bp, "coin") || *bp == GOLD_SYM) {
			if (cnt > 5000
#ifdef WIZARD
					&& !wizard
#endif
						) cnt=5000;
		if (cnt < 1) cnt=1;
#ifndef GOLDOBJ
		if (from_user)
		    pline("%d gold piece%s.", cnt, plur(cnt));
		u.ugold += cnt;
		flags.botl=1;
		return (&zeroobj);
#else
                otmp = mksobj(GOLD_PIECE, FALSE, FALSE);
		otmp->quan = cnt;
                otmp->owt = weight(otmp);
		flags.botl=1;
		return (otmp);
#endif
	}
	if (strlen(bp) == 1 &&
	   (i = def_char_to_objclass(*bp)) < MAXOCLASSES && i > ILLOBJ_CLASS
#ifdef WIZARD
	    && (wizard || i != VENOM_CLASS)
#else
	    && i != VENOM_CLASS
#endif
	    ) {
		oclass = i;
		goto any;
	}

	/* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
	/* false hits on, e.g., rings for "ring mail". */
	if(strncmpi(bp, "enchant ", 8) &&
	   strncmpi(bp, "destroy ", 8) &&
	   strncmpi(bp, "food detection", 14) &&
	   strncmpi(bp, "ring mail", 9) &&
	   strncmpi(bp, "studded leather arm", 19) &&
	   strncmpi(bp, "leather arm", 11) &&
	   strncmpi(bp, "tooled horn", 11) &&
	   strncmpi(bp, "food ration", 11) &&
	   strncmpi(bp, "meat ring", 9)
	)
	for (i = 0; i < (int)(sizeof wrpsym); i++) {
		register int j = strlen(wrp[i]);
		if(!strncmpi(bp, wrp[i], j)){
			oclass = wrpsym[i];
			if(oclass != AMULET_CLASS) {
			    bp += j;
			    if(!strncmpi(bp, " of ", 4)) actualn = bp+4;
			    /* else if(*bp) ?? */
			} else
			    actualn = bp;
			goto srch;
		}
		if(!BSTRCMPI(bp, p-j, wrp[i])){
			oclass = wrpsym[i];
			p -= j;
			*p = 0;
			if(p > bp && p[-1] == ' ') p[-1] = 0;
			actualn = dn = bp;
			goto srch;
		}
	}

	/* "grey stone" check must be before general "stone" */
	for (i = 0; i < SIZE(o_ranges); i++)
	    if(!strcmpi(bp, o_ranges[i].name)) {
		typ = rnd_class(o_ranges[i].f_o_range, o_ranges[i].l_o_range);
		goto typfnd;
	    }

	if (!BSTRCMPI(bp, p-6, " stone")) {
		p[-6] = 0;
		oclass = GEM_CLASS;
		dn = actualn = bp;
		goto srch;
	} else if (!strcmpi(bp, "looking glass")) {
		;	/* avoid false hit on "* glass" */
	} else if (!BSTRCMPI(bp, p-6, " glass") || !strcmpi(bp, "glass")) {
		register char *g = bp;
		if (strstri(g, "broken")) return (struct obj *)0;
		if (!strncmpi(g, "worthless ", 10)) g += 10;
		if (!strncmpi(g, "piece of ", 9)) g += 9;
		if (!strncmpi(g, "colored ", 8)) g += 8;
		else if (!strncmpi(g, "coloured ", 9)) g += 9;
		if (!strcmpi(g, "glass")) {	/* choose random color */
			/* 9 different kinds */
			typ = LAST_GEM + rnd(9);
			if (objects[typ].oc_class == GEM_CLASS) goto typfnd;
			else typ = 0;	/* somebody changed objects[]? punt */
		} else {		/* try to construct canonical form */
			char tbuf[BUFSZ];
			Strcpy(tbuf, "worthless piece of ");
			Strcat(tbuf, g);  /* assume it starts with the color */
			Strcpy(bp, tbuf);
		}
	}

	actualn = bp;
	if (!dn) dn = actualn; /* ex. "skull cap" */
srch:
	/* check real names of gems first */
	if(!oclass && actualn) {
	    for(i = bases[GEM_CLASS]; i <= LAST_GEM; i++) {
		register const char *zn;

		if((zn = OBJ_NAME(objects[i])) && !strcmpi(actualn, zn)) {
		    typ = i;
		    goto typfnd;
		}
	    }
	}
	i = oclass ? bases[(int)oclass] : 1;
	while(i < NUM_OBJECTS && (!oclass || objects[i].oc_class == oclass)){
		register const char *zn;

		if (actualn && (zn = OBJ_NAME(objects[i])) != 0 &&
			    wishymatch(actualn, zn, TRUE)) {
			typ = i;
			goto typfnd;
		}
		if (dn && (zn = OBJ_DESCR(objects[i])) != 0 &&
			    wishymatch(dn, zn, FALSE)) {
			/* don't match extra descriptions (w/o real name) */
			if (!OBJ_NAME(objects[i])) return (struct obj *)0;
			typ = i;
			goto typfnd;
		}
		if (un && (zn = objects[i].oc_uname) != 0 &&
			    wishymatch(un, zn, FALSE)) {
			typ = i;
			goto typfnd;
		}
		i++;
	}
	if (actualn) {
		struct Jitem *j = Japanese_items;
		while(j->item) {
			if (actualn && !strcmpi(actualn, j->name)) {
				typ = j->item;
				goto typfnd;
			}
			j++;
		}
	}
	if (!strcmpi(bp, "spinach")) {
		contents = SPINACH;
		typ = TIN;
		goto typfnd;
	}
	/* Note: not strncmpi.  2 fruits, one capital, one not, are possible. */
	{
	    char *fp;
	    int l, cntf;
	    int blessedf, iscursedf, uncursedf, halfeatenf;

	    blessedf = iscursedf = uncursedf = halfeatenf = 0;
	    cntf = 0;

	    fp = fruitbuf;
	    for(;;) {
		if (!fp || !*fp) break;
		if (!strncmpi(fp, "an ", l=3) ||
		    !strncmpi(fp, "a ", l=2)) {
			cntf = 1;
		} else if (!cntf && digit(*fp)) {
			cntf = atoi(fp);
			while(digit(*fp)) fp++;
			while(*fp == ' ') fp++;
			l = 0;
		} else if (!strncmpi(fp, "blessed ", l=8)) {
			blessedf = 1;
		} else if (!strncmpi(fp, "cursed ", l=7)) {
			iscursedf = 1;
		} else if (!strncmpi(fp, "uncursed ", l=9)) {
			uncursedf = 1;
		} else if (!strncmpi(fp, "partly eaten ", l=13)) {
			halfeatenf = 1;
		} else break;
		fp += l;
	    }

	    for(f=ffruit; f; f = f->nextf) {
		char *f1 = f->fname, *f2 = makeplural(f->fname);

		if(!strncmp(fp, f1, strlen(f1)) ||
					!strncmp(fp, f2, strlen(f2))) {
			typ = SLIME_MOLD;
			blessed = blessedf;
			iscursed = iscursedf;
			uncursed = uncursedf;
			halfeaten = halfeatenf;
			cnt = cntf;
			ftype = f->fid;
			goto typfnd;
		}
	    }
	}

	if(!oclass && actualn) {
	    short objtyp;

	    /* Perhaps it's an artifact specified by name, not type */
	    name = artifact_name(actualn, &objtyp);
	    if(name) {
		typ = objtyp;
		goto typfnd;
	    }
	}
#ifdef WIZARD
	/* Let wizards wish for traps --KAA */
	/* must come after objects check so wizards can still wish for
	 * trap objects like beartraps
	 */
	if (wizard && from_user) {
		int trap;

		for (trap = NO_TRAP+1; trap < TRAPNUM; trap++) {
			const char *tname;

			tname = defsyms[trap_to_defsym(trap)].explanation;
			if (!strncmpi(tname, bp, strlen(tname))) {
				/* avoid stupid mistakes */
				if((trap == TRAPDOOR || trap == HOLE)
				      && !Can_fall_thru(&u.uz)) trap = ROCKTRAP;
				(void) maketrap(u.ux, u.uy, trap);
				pline("%s.", An(tname));
				return(&zeroobj);
			}
		}
		/* or some other dungeon features -dlc */
		p = eos(bp);
		if(!BSTRCMP(bp, p-8, "fountain")) {
			levl[u.ux][u.uy].typ = FOUNTAIN;
			level.flags.nfountains++;
			if(!strncmpi(bp, "magic ", 6))
				levl[u.ux][u.uy].blessedftn = 1;
			pline("A %sfountain.",
			      levl[u.ux][u.uy].blessedftn ? "magic " : "");
			newsym(u.ux, u.uy);
			return(&zeroobj);
		}
		if(!BSTRCMP(bp, p-6, "throne")) {
			levl[u.ux][u.uy].typ = THRONE;
			pline("A throne.");
			newsym(u.ux, u.uy);
			return(&zeroobj);
		}
# ifdef SINKS
		if(!BSTRCMP(bp, p-4, "sink")) {
			levl[u.ux][u.uy].typ = SINK;
			level.flags.nsinks++;
			pline("A sink.");
			newsym(u.ux, u.uy);
			return &zeroobj;
		}
# endif
		if(!BSTRCMP(bp, p-4, "pool")) {
			levl[u.ux][u.uy].typ = POOL;
			del_engr_at(u.ux, u.uy);
			pline("A pool.");
			/* Must manually make kelp! */
			water_damage(level.objects[u.ux][u.uy], FALSE, TRUE);
			newsym(u.ux, u.uy);
			return &zeroobj;
		}
		if (!BSTRCMP(bp, p-4, "lava")) {  /* also matches "molten lava" */
			levl[u.ux][u.uy].typ = LAVAPOOL;
			del_engr_at(u.ux, u.uy);
			pline("A pool of molten lava.");
			if (!(Levitation || Flying)) (void) lava_effects();
			newsym(u.ux, u.uy);
			return &zeroobj;
		}

		if(!BSTRCMP(bp, p-5, "altar")) {
		    aligntyp al;

		    levl[u.ux][u.uy].typ = ALTAR;
		    if(!strncmpi(bp, "chaotic ", 8))
			al = A_CHAOTIC;
		    else if(!strncmpi(bp, "neutral ", 8))
			al = A_NEUTRAL;
		    else if(!strncmpi(bp, "lawful ", 7))
			al = A_LAWFUL;
		    else if(!strncmpi(bp, "unaligned ", 10))
			al = A_NONE;
		    else /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
			al = (!rn2(6)) ? A_NONE : rn2((int)A_LAWFUL+2) - 1;
		    levl[u.ux][u.uy].altarmask = Align2amask( al );
		    pline("%s altar.", An(align_str(al)));
		    newsym(u.ux, u.uy);
		    return(&zeroobj);
		}

		if(!BSTRCMP(bp, p-5, "grave") || !BSTRCMP(bp, p-9, "headstone")) {
		    make_grave(u.ux, u.uy, (char *) 0);
		    pline("A grave.");
		    newsym(u.ux, u.uy);
		    return(&zeroobj);
		}

		if(!BSTRCMP(bp, p-4, "tree")) {
		    levl[u.ux][u.uy].typ = TREE;
		    pline("A tree.");
		    newsym(u.ux, u.uy);
		    block_point(u.ux, u.uy);
		    return &zeroobj;
		}

		if(!BSTRCMP(bp, p-4, "bars")) {
		    levl[u.ux][u.uy].typ = IRONBARS;
		    pline("Iron bars.");
		    newsym(u.ux, u.uy);
		    return &zeroobj;
		}
	}
#endif
	if(!oclass) return((struct obj *)0);
any:
	if(!oclass) oclass = wrpsym[rn2((int)sizeof(wrpsym))];
typfnd:
	if (typ) oclass = objects[typ].oc_class;

	/* check for some objects that are not allowed */
	if (typ && objects[typ].oc_unique) {
#ifdef WIZARD
	    if (wizard)
		;	/* allow unique objects */
	    else
#endif
	    switch (typ) {
		case AMULET_OF_YENDOR:
		    typ = FAKE_AMULET_OF_YENDOR;
		    break;
		case CANDELABRUM_OF_INVOCATION:
		    typ = rnd_class(TALLOW_CANDLE, WAX_CANDLE);
		    break;
		case BELL_OF_OPENING:
		    typ = BELL;
		    break;
		case SPE_BOOK_OF_THE_DEAD:
		    typ = SPE_BLANK_PAPER;
		    break;
	    }
	}

	/* catch any other non-wishable objects */
	if (objects[typ].oc_nowish
#ifdef WIZARD
	    && !wizard
#endif
	    )
	    return((struct obj *)0);

	/* convert magic lamps to regular lamps before lighting them or setting
	   the charges */
	if (typ == MAGIC_LAMP
#ifdef WIZARD
				&& !wizard
#endif
						)
	    typ = OIL_LAMP;

	if(typ) {
		otmp = mksobj(typ, TRUE, FALSE);
	} else {
		otmp = mkobj(oclass, FALSE);
		if (otmp) typ = otmp->otyp;
	}

	if (islit &&
		(typ == OIL_LAMP || typ == MAGIC_LAMP || typ == BRASS_LANTERN ||
		 typ == ORB_OF_LIGHT ||
		 Is_candle(otmp) || typ == POT_OIL)) {
	    place_object(otmp, u.ux, u.uy);  /* make it viable light source */
	    begin_burn(otmp, FALSE);
	    obj_extract_self(otmp);	 /* now release it for caller's use */
	}

	if(cnt > 0 && objects[typ].oc_merge && oclass != SPBOOK_CLASS &&
		(cnt < rnd(6) ||
#ifdef WIZARD
		wizard ||
#endif
		 (cnt <= 7 && Is_candle(otmp)) ||
		 (cnt <= 20 &&
		  ((oclass == WEAPON_CLASS && is_ammo(otmp))
				|| typ == ROCK || is_missile(otmp)))))
			otmp->quan = (long) cnt;

#ifdef WIZARD
	if (oclass == VENOM_CLASS) otmp->spe = 1;
#endif

	if (spesgn == 0) spe = otmp->spe;
#ifdef WIZARD
	else if (wizard) /* no alteration to spe */ ;
#endif
	else if (oclass == ARMOR_CLASS || oclass == WEAPON_CLASS ||
		 is_weptool(otmp) ||
			(oclass==RING_CLASS && objects[typ].oc_charged)) {
		if(spe > rnd(5) && spe > otmp->spe) spe = 0;
		if(spe > 2 && Luck < 0) spesgn = -1;
	} else {
		if (oclass == WAND_CLASS) {
			if (spe > 1 && spesgn == -1) spe = 1;
		} else {
			if (spe > 0 && spesgn == -1) spe = 0;
		}
		if (spe > otmp->spe) spe = otmp->spe;
	}

	if (spesgn == -1) spe = -spe;

	/* set otmp->spe.  This may, or may not, use spe... */
	switch (typ) {
		case TIN: if (contents==EMPTY) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 0;
			} else if (contents==SPINACH) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 1;
			}
			break;
		case SLIME_MOLD: otmp->spe = ftype;
			/* Fall through */
		case SKELETON_KEY: case CHEST: case LARGE_BOX:
		case HEAVY_IRON_BALL: case IRON_CHAIN: case STATUE:
			/* otmp->cobj already done in mksobj() */
				break;
#ifdef MAIL
		case SCR_MAIL: otmp->spe = 1; break;
#endif
		case WAN_WISHING:
#ifdef WIZARD
			if (!wizard) {
#endif
				otmp->spe = (rn2(10) ? -1 : 0);
				break;
#ifdef WIZARD
			}
			/* fall through, if wizard */
#endif
		default: otmp->spe = spe;
	}

	/* set otmp->corpsenm or dragon scale [mail] */
	if (mntmp >= LOW_PM) {
		if (mntmp == PM_LONG_WORM_TAIL) mntmp = PM_LONG_WORM;

		switch (typ) {
		case TIN:
			otmp->spe = 0; /* No spinach */
			if (dead_species(mntmp, FALSE)) {
			    otmp->corpsenm = NON_PM;	/* it's empty */
			} else if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE) &&
				   mons[mntmp].cnutrit != 0) {
			    otmp->corpsenm = mntmp;
			}
			break;
		case CORPSE:
			if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE)) {
			    /* beware of random troll or lizard corpse,
			       or of ordinary one being forced to such */
			    if (otmp->timed) obj_stop_timers(otmp);
			    if (mons[mntmp].msound == MS_GUARDIAN)
			    	otmp->corpsenm = genus(mntmp,1);
			    else
				otmp->corpsenm = mntmp;
			    start_corpse_timeout(otmp);
			}
			break;
		case FIGURINE:
			if (!(mons[mntmp].geno & G_UNIQ)
			    && !is_human(&mons[mntmp])
#ifdef MAIL
			    && mntmp != PM_MAIL_DAEMON
#endif
							)
				otmp->corpsenm = mntmp;
			break;
		case EGG:
			mntmp = can_be_hatched(mntmp);
			if (mntmp != NON_PM) {
			    otmp->corpsenm = mntmp;
			    if (!dead_species(mntmp, TRUE))
				attach_egg_hatch_timeout(otmp);
			    else
				kill_egg(otmp);
			}
			break;
		case STATUE: otmp->corpsenm = mntmp;
			if (Has_contents(otmp) && verysmall(&mons[mntmp]))
			    delete_contents(otmp);	/* no spellbook */
			otmp->spe = ishistoric ? STATUE_HISTORIC : 0;
			break;
		case SCALE_MAIL:
			/* Dragon mail - depends on the order of objects */
			/*		 & dragons.			 */
			if (mntmp >= PM_GRAY_DRAGON &&
						mntmp <= PM_YELLOW_DRAGON)
			    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						    mntmp - PM_GRAY_DRAGON;
			break;
		}
	}

	/* set blessed/cursed -- setting the fields directly is safe
	 * since weight() is called below and addinv() will take care
	 * of luck */
	if (iscursed) {
		curse(otmp);
	} else if (uncursed) {
		otmp->blessed = 0;
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wizard
#endif
							);
	} else if (blessed) {
		otmp->blessed = (Luck >= 0
#ifdef WIZARD
					 || wizard
#endif
							);
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wizard
#endif
							);
	} else if (spesgn < 0) {
		curse(otmp);
	}

#ifdef INVISIBLE_OBJECTS
	if (isinvisible) otmp->oinvis = 1;
#endif

	/* set eroded */
	if (is_damageable(otmp) || otmp->otyp == CRYSKNIFE) {
	    if (eroded && (is_flammable(otmp) || is_rustprone(otmp)))
		    otmp->oeroded = eroded;
	    if (eroded2 && (is_corrodeable(otmp) || is_rottable(otmp)))
		    otmp->oeroded2 = eroded2;

	    /* set erodeproof */
	    if (erodeproof && !eroded && !eroded2)
		    otmp->oerodeproof = (Luck >= 0
#ifdef WIZARD
					     || wizard
#endif
					);
	}

	/* set otmp->recharged */
	if (oclass == WAND_CLASS) {
	    /* prevent wishing abuse */
	    if (otmp->otyp == WAN_WISHING
#ifdef WIZARD
		    && !wizard
#endif
		) rechrg = 1;
	    otmp->recharged = (unsigned)rechrg;
	}

	/* set poisoned */
	if (ispoisoned) {
	    if (is_poisonable(otmp))
		otmp->opoisoned = (Luck >= 0);
	    else if (Is_box(otmp) || typ == TIN)
		otmp->otrapped = 1;
	    else if (oclass == FOOD_CLASS)
		/* try to taint by making it as old as possible */
		otmp->age = 1L;
	}

	if (isgreased) otmp->greased = 1;

	if (isdiluted && otmp->oclass == POTION_CLASS &&
			otmp->otyp != POT_WATER)
		otmp->odiluted = 1;

	if (name) {
		const char *aname;
		short objtyp;

		/* an artifact name might need capitalization fixing */
		aname = artifact_name(name, &objtyp);
		if (aname && objtyp == otmp->otyp) name = aname;

		otmp = oname(otmp, name);
		if (otmp->oartifact) {
			otmp->quan = 1L;
			u.uconduct.wisharti++;	/* KMH, conduct */
		}
	}

	/* more wishing abuse: don't allow wishing for certain artifacts */
	/* and make them pay; charge them for the wish anyway! */
	if ((is_quest_artifact(otmp) ||
	     (otmp->oartifact && rn2(nartifact_exist()) > 1))
#ifdef WIZARD
	    && !wizard
#endif
	    ) {
	    artifact_exists(otmp, ONAME(otmp), FALSE);
	    obfree(otmp, (struct obj *) 0);
	    otmp = &zeroobj;
/*JP
	    pline("For a moment, you feel %s in your %s, but it disappears!",
*/
	    pline("一瞬%sが%sの中にあるような感じがしたが、すぐに消えさった！",
		  something,
		  makeplural(body_part(HAND)));
	}

	if (halfeaten && otmp->oclass == FOOD_CLASS) {
		if (otmp->otyp == CORPSE)
			otmp->oeaten = mons[otmp->corpsenm].cnutrit;
		else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
		/* (do this adjustment before setting up object's weight) */
		consume_oeaten(otmp, 1);
	}
	otmp->owt = weight(otmp);
	if (very && otmp->otyp == HEAVY_IRON_BALL) otmp->owt += 160;

	return(otmp);
}

int
rnd_class(first,last)
int first,last;
{
	int i, x, sum=0;

	if (first == last)
	    return (first);
	for(i=first; i<=last; i++)
		sum += objects[i].oc_prob;
	if (!sum) /* all zero */
		return first + rn2(last-first+1);
	x = rnd(sum);
	for(i=first; i<=last; i++)
		if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
			return i;
	return 0;
}

STATIC_OVL const char *
Japanese_item_name(i)
int i;
{
	struct Jitem *j = Japanese_items;

	while(j->item) {
		if (i == j->item)
			return j->name;
		j++;
	}
	return (const char *)0;
}

const char *
cloak_simple_name(cloak)
struct obj *cloak;
{
    if (cloak) {
	switch (cloak->otyp) {
	case KITCHEN_APRON:
	case FRILLED_APRON:
#if 0 /* JP */
	    return "apron";
#else
	    return "エプロン";
#endif
	case ROBE:
#if 0 /*JP*/
	    return "robe";
#else
	    return "ローブ";
#endif
	case MUMMY_WRAPPING:
#if 0 /*JP*/
	    return "wrapping";
#else
	    return "包帯";
#endif
	case ALCHEMY_SMOCK:
#if 0 /*JP*/
	    return (objects[cloak->otyp].oc_name_known &&
			cloak->dknown) ? "smock" : "apron";
#else
	    return (objects[cloak->otyp].oc_name_known &&
			cloak->dknown) ? "スモック" : "エプロン";
#endif
	default:
	    break;
	}
    }
#if 0 /*JP*/
    return "cloak";
#else
    return "クローク";
#endif
}

const char *
mimic_obj_name(mtmp)
struct monst *mtmp;
{
	if (mtmp->m_ap_type == M_AP_OBJECT && mtmp->mappearance != STRANGE_OBJECT) {
		int idx = objects[mtmp->mappearance].oc_descr_idx;
#if 0 /*JP*/
		if (mtmp->mappearance == GOLD_PIECE) return "gold";
#else
		if (mtmp->mappearance == GOLD_PIECE) return "金貨";
#endif
		return obj_descr[idx].oc_name;
	}
#if 0 /*JP*/
	return "whatcha-may-callit";
#else
	return "何とかいうもの";
#endif
}
#endif /* OVLB */

#if 1 /*JP*/
/*
**  文字列 buf の str1を str2へ置換
**  bufは置換後の文字列が入るだけの領域が必要
*/
static
char *
substitute(buf, str1, str2)
     char *buf;
     char *str1;
     char *str2;
{
  unsigned char *p, *pp;
  char tmp[BUFSZ];
  int len = strlen(str1);

  if(!buf)
    return buf;

  p = (unsigned char *)buf;

  while(*p){
    if(!strncmp(p, str1, len)){
      Strcpy(tmp, p + len);
      while(*str2)
	*(p++) = *(str2++);

      pp = tmp;
      while(*pp)
	*(p++) = *(pp++);
      *(p++) = '\0';
      return buf;
    }
    if(*p >= 0x80)
      p += 2;
    else
      ++p;
  }

  return (char *)0;
}

/*
**  文字列 buf の strの前の部分と strの後の部分を交換する。
**
**  EX) ほえほえ(と名づけられた)犬 -> 犬(と名づけられた)ほえほえ
*/
static 
char *
transpose(buf, str)
     char *buf;
     char *str;
{
  unsigned char *p, *pp, *ppp;
  char tmp[BUFSZ];
  int len = strlen(str);

  Strcpy(tmp, buf);
  ppp = pp = p = (unsigned char *)tmp;

  while(*p){
    if(!strncmp(p, str, len)){
      Strcpy(buf, p + len);
      Strcat(buf, str);
      ppp = eos(buf);
      while(pp != p)
	*(ppp++) = *(pp++);
      *(ppp++) = '\0';
      return buf;
    }
    if(*p >= 0x80)
      p += 2;
    else
      ++p;
  }

  return (char *)0;

}
/*
**  文字列 buf の先頭に strを挿入する
*/
#if 0
static 
char *
insert(buf, str)
     char *buf;
     char *str;
{
  return substitute(buf, "", str);
}
#endif
  
/*
**  文字列 buf から strを取り除く。
*/
static
char *
delete(buf, str)
     char *buf;
     char *str;
{
  return substitute(buf, str, (char *)"");
}

static int
digit_8(c)
     int c;
{
  unsigned int uc = c;

  return (uc >= '0' && uc <= '9') ? 1 : 0;
}

static int
atoi_8(s)
     const char *s;
{
  char *pp;
  char tmp[BUFSZ];

  pp = tmp;

  while(digit_8(*s))
    *(pp++) = *(s++);
  *pp = '\0';

  return atoi(tmp);
}
#endif
/*objnam.c*/
