/*	SCCS Id: @(#)read.c	3.4	2003/10/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

/* KMH -- Copied from pray.c; this really belongs in a header file */
#define DEVOUT 14
#define STRIDENT 4

#define Your_Own_Role(mndx) \
	((mndx) == urole.malenum || \
	 (urole.femalenum != NON_PM && (mndx) == urole.femalenum))
#define Your_Own_Race(mndx) \
	((mndx) == urace.malenum || \
	 (urace.femalenum != NON_PM && (mndx) == urace.femalenum))

#ifdef OVLB

boolean	known;

static NEARDATA const char readable[] =
		   { ALL_CLASSES, SCROLL_CLASS, SPBOOK_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };

static void FDECL(wand_explode, (struct obj *));
static void NDECL(do_class_genocide);
static void FDECL(stripspe,(struct obj *));
static void FDECL(p_glow1,(struct obj *));
static void FDECL(p_glow2,(struct obj *,const char *));
static void FDECL(randomize,(int *, int));
static void FDECL(forget_single_object, (int));
static void FDECL(forget, (int));
static void FDECL(maybe_tame, (struct monst *,struct obj *));

STATIC_PTR void FDECL(set_lit, (int,int,genericptr_t));

int
doread()
{
	register struct obj *scroll;
	register boolean confused;

	known = FALSE;
	if(check_capacity((char *)0)) return (0);
	scroll = getobj(readable, "read");
	if(!scroll) return(0);

	/* outrumor has its own blindness check */
	if(scroll->otyp == FORTUNE_COOKIE) {
	    if(flags.verbose)
/*JP
		You("break up the cookie and throw away the pieces.");
*/
		You("クッキーを割り、かけらを投げすてた。");
	    outrumor(bcsign(scroll), BY_COOKIE);
	    if (!Blind) u.uconduct.literate++;
	    useup(scroll);
	    return(1);
#ifdef TOURIST
	} else if (scroll->otyp == T_SHIRT) {
	    static const char *shirt_msgs[] = { /* Scott Bigham */
#if 0 /*JP*/
    "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
    "Is that Mjollnir in your pocket or are you just happy to see me?",
    "It's not the size of your sword, it's how #enhance'd you are with it.",
    "Madame Elvira's House O' Succubi Lifetime Customer",
    "Madame Elvira's House O' Succubi Employee of the Month",
    "Ludios Vault Guards Do It In Small, Dark Rooms",
    "Yendor Military Soldiers Do It In Large Groups",
    "I survived Yendor Military Boot Camp",
    "Ludios Accounting School Intra-Mural Lacrosse Team",
    "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
    "Hey, black dragon!  Disintegrate THIS!",
    "I'm With Stupid -->",
    "Don't blame me, I voted for Izchak!",
    "Don't Panic",				/* HHGTTG */
    "Furinkan High School Athletic Dept.",	/* Ranma 1/2 */
    "Hel-LOOO, Nurse!",			/* Animaniacs */
#else
    "私は運命の迷宮を調査していたが、手に入れたのはきたないＴシャツだけだった！",
    "ポケットにミュルニールが入っているの？それとも私に会えてうれしい？",
    "剣の大きさが問題なのではない。それが如何に#enhanceされているかなのだ。",
    "マダム・エルバイラのサキュバス館 永久顧客",
    "マダム・エルバイラのサキュバス館 今月の最優秀功労者",
    "ローディオス金庫の番人、それは暗く小さい部屋の中",
    "イェンダー軍兵士、それは巨大な団体の中",
    "私はイェンダー軍の新兵訓練所を乗り越えました",
    "ローディオス会計学校 室内ラクロスチーム",
    "Oracle(TM) の泉 第１０回濡れＴシャツコンテスト",
    "おい、黒ドラゴン！こいつを分解しろ！",
    "馬鹿と一緒にいます → ",
    "私は悪くない！Izchakに投票したもの！",
    "あ わ て る な",	/* 銀河ヒッチハイクガイド */
    "風林館高校陸上部",	/* Ranma 1/2 */
    "Ｈｅｌ−ＬＯＯＯ、Ｎｕｒｓｅ！",	/* Animaniacs */
#endif
	    };
	    char buf[BUFSZ];
	    int erosion;

	    if (Blind) {
/*JP
		You_cant("feel any Braille writing.");
*/
		You("点字はどうも書いてないようだ。");
		return 0;
	    }
	    u.uconduct.literate++;
	    if(flags.verbose)
/*JP
		pline("It reads:");
*/
		pline("それを読んだ：");
	    Strcpy(buf, shirt_msgs[scroll->o_id % SIZE(shirt_msgs)]);
	    erosion = greatest_erosion(scroll);
	    if (erosion)
		wipeout_text(buf,
			(int)(strlen(buf) * erosion / (2*MAX_ERODE)),
			     scroll->o_id ^ (unsigned)u.ubirthday);
/*JP
	    pline("\"%s\"", buf);
*/
	    pline("「%s」", buf);
	    return 1;
#endif	/* TOURIST */
	} else if (scroll->oclass != SCROLL_CLASS
		&& scroll->oclass != SPBOOK_CLASS) {
/*JP
	    pline(silly_thing_to, "read");
*/
	    pline(silly_thing_to, "読む");
	    return(0);
	} else if (Blind) {
	    const char *what = 0;
	    if (scroll->oclass == SPBOOK_CLASS)
/*JP
		what = "mystic runes";
*/
		what = "神秘的なルーン文字";
	    else if (!scroll->dknown)
/*JP
		what = "formula on the scroll";
*/
		what = "巻物の呪文";
	    if (what) {
/*JP
		pline("Being blind, you cannot read the %s.", what);
*/
		pline("目が見えないので、あなたは%sを読むことができない。", what);
		return(0);
	    }
	}

	/* Actions required to win the game aren't counted towards conduct */
	if (scroll->otyp != SPE_BOOK_OF_THE_DEAD &&
		scroll->otyp != SPE_BLANK_PAPER &&
		scroll->otyp != SCR_BLANK_PAPER)
	    u.uconduct.literate++;

	confused = (Confusion != 0);
#ifdef MAIL
	if (scroll->otyp == SCR_MAIL) confused = FALSE;
#endif
	if(scroll->oclass == SPBOOK_CLASS) {
	    return(study_book(scroll));
	}
	scroll->in_use = TRUE;	/* scroll, not spellbook, now being read */
	if(scroll->otyp != SCR_BLANK_PAPER) {
	  if(Blind)
#if 0 /*JP*/
	    pline("As you %s the formula on it, the scroll disappears.",
			is_silent(youmonst.data) ? "cogitate" : "pronounce");
#else
	    pline("呪文を唱えると、巻物は消えた。");
#endif
	  else
/*JP
	    pline("As you read the scroll, it disappears.");
*/
	    pline("巻物を読むと、それは消えた。");
	  if(confused) {
	    if (Hallucination)
/*JP
		pline("Being so trippy, you screw up...");
*/
		pline("とてもへろへろなので、くしゃくしゃにしてしまった…");
	    else
#if 0 /*JP*/
		pline("Being confused, you mis%s the magic words...",
			is_silent(youmonst.data) ? "understand" : "pronounce");
#else
		pline("混乱しているので、呪文を間違ってしまった…");
#endif
	  }
	}
	if(!seffects(scroll))  {
		if(!objects[scroll->otyp].oc_name_known) {
		    if(known) {
			makeknown(scroll->otyp);
			more_experienced(0,10);
		    } else if(!objects[scroll->otyp].oc_uname)
			docall(scroll);
		}
		if(scroll->otyp != SCR_BLANK_PAPER)
			useup(scroll);
		else scroll->in_use = FALSE;
	}
	return(1);
}

static void
stripspe(obj)
register struct obj *obj;
{
	if (obj->blessed) pline(nothing_happens);
	else {
		if (obj->spe > 0) {
		    obj->spe = 0;
		    if (obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN)
			obj->age = 0;
/*JP
		    Your("%s %s briefly.",xname(obj), otense(obj, "vibrate"));
*/
		    Your("%sは小刻みに振動した。",xname(obj));
		} else pline(nothing_happens);
	}
}

static void
p_glow1(otmp)
register struct obj	*otmp;
{
#if 0 /*JP*/
	Your("%s %s briefly.", xname(otmp),
	     otense(otmp, Blind ? "vibrate" : "glow"));
#else
	Your("%sは小刻みに%s。", xname(otmp),
		Blind ? "振動した" : "輝いた");
#endif
}

static void
p_glow2(otmp,color)
register struct obj	*otmp;
register const char *color;
{
#if 0 /*JP*/
	Your("%s %s%s%s for a moment.",
		xname(otmp),
		otense(otmp, Blind ? "vibrate" : "glow"),
		Blind ? "" : " ",
		Blind ? nul : hcolor(color));
#else
	Your("%sは一瞬%s%s。",
		xname(otmp),
		Blind ? (const char *)"" : jconj_adj(hcolor(color)),
		Blind ? "振動した" : "輝いた");
#endif
}

/* Is the object chargeable?  For purposes of inventory display; it is */
/* possible to be able to charge things for which this returns FALSE. */
boolean
is_chargeable(obj)
struct obj *obj;
{
	if (obj->oclass == WAND_CLASS) return TRUE;
	/* known && !uname is possible after amnesia/mind flayer */
	if (obj->oclass == RING_CLASS)
	    return (boolean)(objects[obj->otyp].oc_charged &&
			(obj->known || objects[obj->otyp].oc_uname));
	if (is_weptool(obj))	/* specific check before general tools */
	    return FALSE;
	if (obj->oclass == TOOL_CLASS)
	    return (boolean)(objects[obj->otyp].oc_charged);
	return FALSE; /* why are weapons/armor considered charged anyway? */
}

/*
 * recharge an object; curse_bless is -1 if the recharging implement
 * was cursed, +1 if blessed, 0 otherwise.
 */
void
recharge(obj, curse_bless)
struct obj *obj;
int curse_bless;
{
	register int n;
	boolean is_cursed, is_blessed;

	is_cursed = curse_bless < 0;
	is_blessed = curse_bless > 0;

	if (obj->oclass == WAND_CLASS) {
	    /* undo any prior cancellation, even when is_cursed */
	    if (obj->spe == -1) obj->spe = 0;

	    /*
	     * Recharging might cause wands to explode.
	     *	v = number of previous recharges
	     *	      v = percentage chance to explode on this attempt
	     *		      v = cumulative odds for exploding
	     *	0 :   0       0
	     *	1 :   0.29    0.29
	     *	2 :   2.33    2.62
	     *	3 :   7.87   10.28
	     *	4 :  18.66   27.02
	     *	5 :  36.44   53.62
	     *	6 :  62.97   82.83
	     *	7 : 100     100
	     */
	    n = (int)obj->recharged;
	    if (n > 0 && (obj->otyp == WAN_WISHING ||
		    (n * n * n > rn2(7*7*7)))) {	/* recharge_limit */
		wand_explode(obj);
		return;
	    }
	    /* didn't explode, so increment the recharge count */
	    obj->recharged = (unsigned)(n + 1);

	    /* now handle the actual recharging */
	    if (is_cursed) {
		stripspe(obj);
	    } else {
		int lim = (obj->otyp == WAN_WISHING) ? 3 :
			(objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

		n = (lim == 3) ? 3 : rn1(5, lim + 1 - 5);
		if (!is_blessed) n = rnd(n);

		if (obj->spe < n) obj->spe = n;
		else obj->spe++;
		if (obj->otyp == WAN_WISHING && obj->spe > 3) {
		    wand_explode(obj);
		    return;
		}
		if (obj->spe >= lim) p_glow2(obj, NH_BLUE);
		else p_glow1(obj);
	    }

	} else if (obj->oclass == RING_CLASS &&
					objects[obj->otyp].oc_charged) {
	    /* charging does not affect ring's curse/bless status */
	    int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
	    boolean is_on = (obj == uleft || obj == uright);

	    /* destruction depends on current state, not adjustment */
	    if (obj->spe > rn2(7) || obj->spe <= -5) {
#if 0 /*JP*/
		Your("%s %s momentarily, then %s!",
		     xname(obj), otense(obj,"pulsate"), otense(obj,"explode"));
#else
		Your("%sは一瞬脈動し、爆発した！",
  		     xname(obj));
#endif
		if (is_on) Ring_gone(obj);
		s = rnd(3 * abs(obj->spe));	/* amount of damage */
		useup(obj);
/*JP
		losehp(s, "exploding ring", KILLED_BY_AN);
*/
		losehp(s, "指輪の爆発で", KILLED_BY_AN);
	    } else {
		long mask = is_on ? (obj == uleft ? LEFT_RING :
				     RIGHT_RING) : 0L;
#if 0 /*JP*/
		Your("%s spins %sclockwise for a moment.",
		     xname(obj), s < 0 ? "counter" : "");
#else
		Your("%sは一瞬%s時計回りに回転した。",
		     xname(obj), s < 0 ? "反" : "");
#endif
		/* cause attributes and/or properties to be updated */
		if (is_on) Ring_off(obj);
		obj->spe += s;	/* update the ring while it's off */
		if (is_on) setworn(obj, mask), Ring_on(obj);
		/* oartifact: if a touch-sensitive artifact ring is
		   ever created the above will need to be revised  */
	    }

	} else if (obj->oclass == TOOL_CLASS) {
	    int rechrg = (int)obj->recharged;

	    if (objects[obj->otyp].oc_charged) {
		/* tools don't have a limit, but the counter used does */
		if (rechrg < 7)	/* recharge_limit */
		    obj->recharged++;
	    }
	    switch(obj->otyp) {
	    case BELL_OF_OPENING:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) obj->spe += rnd(3);
		else obj->spe += 1;
		if (obj->spe > 5) obj->spe = 5;
		break;
	    case MAGIC_MARKER:
	    case TINNING_KIT:
#ifdef TOURIST
	    case EXPENSIVE_CAMERA:
#endif
		if (is_cursed) stripspe(obj);
		else if (rechrg && obj->otyp == MAGIC_MARKER) {	/* previously recharged */
		    obj->recharged = 1;	/* override increment done above */
		    if (obj->spe < 3)
/*JP
			Your("marker seems permanently dried out.");
*/
			Your("マーカは完全に乾ききってしまった。");
		    else
			pline(nothing_happens);
		} else if (is_blessed) {
		    n = rn1(16,15);		/* 15..30 */
		    if (obj->spe + n <= 50)
			obj->spe = 50;
		    else if (obj->spe + n <= 75)
			obj->spe = 75;
		    else {
			int chrg = (int)obj->spe;
			if ((chrg + n) > 127)
				obj->spe = 127;
			else
				obj->spe += n;
		    }
		    p_glow2(obj, NH_BLUE);
		} else {
		    n = rn1(11,10);		/* 10..20 */
		    if (obj->spe + n <= 50)
			obj->spe = 50;
		    else {
			int chrg = (int)obj->spe;
			if ((chrg + n) > 127)
				obj->spe = 127;
			else
				obj->spe += n;
		    }
		    p_glow2(obj, NH_WHITE);
		}
		break;
	    case OIL_LAMP:
	    case BRASS_LANTERN:
		if (is_cursed) {
		    stripspe(obj);
		    if (obj->lamplit) {
			if (!Blind)
#if 0 /*JP*/
			    pline("%s out!", Tobjnam(obj, "go"));
#else
			    pline("%sは消えた！", xname(obj));
#endif
			end_burn(obj, TRUE);
		    }
		} else if (is_blessed) {
		    obj->spe = 1;
		    obj->age = 1500;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->spe = 1;
		    obj->age += 750;
		    if (obj->age > 1500) obj->age = 1500;
		    p_glow1(obj);
		}
		break;
	    case CRYSTAL_BALL:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) {
		    obj->spe = 6;
		    p_glow2(obj, NH_BLUE);
		} else {
		    if (obj->spe < 5) {
			obj->spe++;
			p_glow1(obj);
		    } else pline(nothing_happens);
		}
		break;
	    case HORN_OF_PLENTY:
	    case BAG_OF_TRICKS:
	    case CAN_OF_GREASE:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) {
		    if (obj->spe <= 10)
			obj->spe += rn1(10, 6);
		    else obj->spe += rn1(5, 6);
		    if (obj->spe > 50) obj->spe = 50;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->spe += rnd(5);
		    if (obj->spe > 50) obj->spe = 50;
		    p_glow1(obj);
		}
		break;
	    case MAGIC_FLUTE:
	    case MAGIC_HARP:
	    case FROST_HORN:
	    case FIRE_HORN:
	    case DRUM_OF_EARTHQUAKE:
		if (is_cursed) {
		    stripspe(obj);
		} else if (is_blessed) {
		    obj->spe += d(2,4);
		    if (obj->spe > 20) obj->spe = 20;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->spe += rnd(4);
		    if (obj->spe > 20) obj->spe = 20;
		    p_glow1(obj);
		}
		break;
	    default:
		goto not_chargable;
		/*NOTREACHED*/
		break;
	    } /* switch */

	} else {
 not_chargable:
/*JP
	    You("have a feeling of loss.");
*/
	    You("なにか損した気分になった。");
	}
}


/* Forget known information about this object class. */
static void
forget_single_object(obj_id)
	int obj_id;
{
	objects[obj_id].oc_name_known = 0;
	objects[obj_id].oc_pre_discovered = 0;	/* a discovery when relearned */
	if (objects[obj_id].oc_uname) {
	    free((genericptr_t)objects[obj_id].oc_uname);
	    objects[obj_id].oc_uname = 0;
	}
	undiscover_object(obj_id);	/* after clearing oc_name_known */

	/* clear & free object names from matching inventory items too? */
}


#if 0	/* here if anyone wants it.... */
/* Forget everything known about a particular object class. */
static void
forget_objclass(oclass)
	int oclass;
{
	int i;

	for (i=bases[oclass];
		i < NUM_OBJECTS && objects[i].oc_class==oclass; i++)
	    forget_single_object(i);
}
#endif


/* randomize the given list of numbers  0 <= i < count */
static void
randomize(indices, count)
	int *indices;
	int count;
{
	int i, iswap, temp;

	for (i = count - 1; i > 0; i--) {
	    if ((iswap = rn2(i + 1)) == i) continue;
	    temp = indices[i];
	    indices[i] = indices[iswap];
	    indices[iswap] = temp;
	}
}


/* Forget % of known objects. */
void
forget_objects(percent)
	int percent;
{
	int i, count;
	int indices[NUM_OBJECTS];

	if (percent == 0) return;
	if (percent <= 0 || percent > 100) {
	    impossible("forget_objects: bad percent %d", percent);
	    return;
	}

	for (count = 0, i = 1; i < NUM_OBJECTS; i++)
	    if (OBJ_DESCR(objects[i]) &&
		    (objects[i].oc_name_known || objects[i].oc_uname))
		indices[count++] = i;

	randomize(indices, count);

	/* forget first % of randomized indices */
	count = ((count * percent) + 50) / 100;
	for (i = 0; i < count; i++)
	    forget_single_object(indices[i]);
}


/* Forget some or all of map (depends on parameters). */
void
forget_map(howmuch)
	int howmuch;
{
	register int zx, zy;

	if (In_sokoban(&u.uz))
	    return;

	known = TRUE;
	for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
	    if (howmuch & ALL_MAP || rn2(7)) {
		/* Zonk all memory of this location. */
		levl[zx][zy].seenv = 0;
		levl[zx][zy].waslit = 0;
		levl[zx][zy].glyph = cmap_to_glyph(S_stone);
	    }
}

/* Forget all traps on the level. */
void
forget_traps()
{
	register struct trap *trap;

	/* forget all traps (except the one the hero is in :-) */
	for (trap = ftrap; trap; trap = trap->ntrap)
	    if ((trap->tx != u.ux || trap->ty != u.uy) && (trap->ttyp != HOLE))
		trap->tseen = 0;
}

/*
 * Forget given % of all levels that the hero has visited and not forgotten,
 * except this one.
 */
void
forget_levels(percent)
	int percent;
{
	int i, count;
	xchar  maxl, this_lev;
	int indices[MAXLINFO];

	if (percent == 0) return;

	if (percent <= 0 || percent > 100) {
	    impossible("forget_levels: bad percent %d", percent);
	    return;
	}

	this_lev = ledger_no(&u.uz);
	maxl = maxledgerno();

	/* count & save indices of non-forgotten visited levels */
	/* Sokoban levels are pre-mapped for the player, and should stay
	 * so, or they become nearly impossible to solve.  But try to
	 * shift the forgetting elsewhere by fiddling with percent
	 * instead of forgetting fewer levels.
	 */
	for (count = 0, i = 0; i <= maxl; i++)
	    if ((level_info[i].flags & VISITED) &&
			!(level_info[i].flags & FORGOTTEN) && i != this_lev) {
		if (ledger_to_dnum(i) == sokoban_dnum)
		    percent += 2;
		else
		    indices[count++] = i;
	    }
	
	if (percent > 100) percent = 100;

	randomize(indices, count);

	/* forget first % of randomized indices */
	count = ((count * percent) + 50) / 100;
	for (i = 0; i < count; i++) {
	    level_info[indices[i]].flags |= FORGOTTEN;
	}
}

/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *
 *	- felt ball & chain
 *	- traps
 *	- part (6 out of 7) of the map
 *
 * Other things are subject to flags:
 *
 *	howmuch & ALL_MAP	= forget whole map
 *	howmuch & ALL_SPELLS	= forget all spells
 */
static void
forget(howmuch)
int howmuch;
{

	if (Punished) u.bc_felt = 0;	/* forget felt ball&chain */

	forget_map(howmuch);
	forget_traps();

	/* 1 in 3 chance of forgetting some levels */
	if (!rn2(3)) forget_levels(rn2(25));

	/* 1 in 3 chance of forgeting some objects */
	if (!rn2(3)) forget_objects(rn2(25));

	if (howmuch & ALL_SPELLS) losespells();
	/*
	 * Make sure that what was seen is restored correctly.  To do this,
	 * we need to go blind for an instant --- turn off the display,
	 * then restart it.  All this work is needed to correctly handle
	 * walls which are stone on one side and wall on the other.  Turning
	 * off the seen bits above will make the wall revert to stone,  but
	 * there are cases where we don't want this to happen.  The easiest
	 * thing to do is to run it through the vision system again, which
	 * is always correct.
	 */
	docrt();		/* this correctly will reset vision */
}

/* monster is hit by scroll of taming's effect */
static void
maybe_tame(mtmp, sobj)
struct monst *mtmp;
struct obj *sobj;
{
	if (sobj->cursed) {
	    setmangry(mtmp);
	} else {
	    if (mtmp->isshk)
		make_happy_shk(mtmp, FALSE);
	    else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
		(void) tamedog(mtmp, (struct obj *) 0);
	}
}

int
seffects(sobj)
register struct obj	*sobj;
{
	register int cval;
	register boolean confused = (Confusion != 0);
	register struct obj *otmp;

	if (objects[sobj->otyp].oc_magic)
		exercise(A_WIS, TRUE);		/* just for trying */
	switch(sobj->otyp) {
#ifdef MAIL
	case SCR_MAIL:
		known = TRUE;
		if (sobj->spe)
/*JP
		    pline("This seems to be junk mail addressed to the finder of the Eye of Larn.");
*/
		    pline("Eye of Larnのファインダーに宛てられたゴミメイルのようだ。");
		/* note to the puzzled: the game Larn actually sends you junk
		 * mail if you win!
		 */
		else readmail(sobj);
		break;
#endif
	case SCR_ENCHANT_ARMOR:
	    {
		register schar s;
		boolean special_armor;
		boolean same_color;

		otmp = some_armor(&youmonst);
		if(!otmp) {
			strange_feeling(sobj,
/*JP
					!Blind ? "Your skin glows then fades." :
*/
					!Blind ? "あなたの体は一瞬輝いた。" :
/*JP
					"Your skin feels warm for a moment.");
*/
					"あなたの体は一瞬暖かくなった。");
			exercise(A_CON, !sobj->cursed);
			exercise(A_STR, !sobj->cursed);
			return(1);
		}
		if(confused) {
			otmp->oerodeproof = !(sobj->cursed);
			if(Blind) {
			    otmp->rknown = FALSE;
#if 0 /*JP*/
			    Your("%s %s warm for a moment.",
				xname(otmp), otense(otmp, "feel"));
#else
			    Your("%sは一瞬暖かくなった。",
				xname(otmp));
#endif
			} else {
			    otmp->rknown = TRUE;
#if 0 /*JP*/
			    Your("%s %s covered by a %s %s %s!",
				xname(otmp), otense(otmp, "are"),
				sobj->cursed ? "mottled" : "shimmering",
				 hcolor(sobj->cursed ? NH_BLACK : NH_GOLDEN),
				sobj->cursed ? "glow" :
				  (is_shield(otmp) ? "layer" : "shield"));
#else
			    Your("%sは%s%s%sで覆われた！",
				xname(otmp),
				jconj_adj(hcolor(sobj->cursed ? NH_BLACK : NH_GOLDEN)),
				sobj->cursed ? "光るまだらの" : "ゆらめく",
				sobj->cursed ? "輝き" :
				  (is_shield(otmp) ? "バリア" : "バリア"));
#endif
			}
			if (otmp->oerodeproof &&
			    (otmp->oeroded || otmp->oeroded2)) {
			    otmp->oeroded = otmp->oeroded2 = 0;
#if 0 /*JP*/
			    Your("%s %s as good as new!",
				 xname(otmp),
				 otense(otmp, Blind ? "feel" : "look"));
#else
			    Your("%sは新品同様になったように%s！",
				 xname(otmp), Blind ? "感じた" : "見えた");
#endif
			}
			break;
		}
		/* elven armor vibrates warningly when enchanted beyond a limit */
		special_armor = is_elven_armor(otmp) ||
			(Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM);
		if (sobj->cursed)
		    same_color =
			(otmp->otyp == BLACK_DRAGON_SCALE_MAIL ||
			 otmp->otyp == BLACK_DRAGON_SCALES);
		else
		    same_color =
			(otmp->otyp == SILVER_DRAGON_SCALE_MAIL ||
			 otmp->otyp == SILVER_DRAGON_SCALES ||
			 otmp->otyp == SHIELD_OF_REFLECTION);
		if (Blind) same_color = FALSE;

		/* KMH -- catch underflow */
		s = sobj->cursed ? -otmp->spe : otmp->spe;
		if (s > (special_armor ? 5 : 3) && rn2(s)) {
#if 0 /*JP*/
		Your("%s violently %s%s%s for a while, then %s.",
		     xname(otmp),
		     otense(otmp, Blind ? "vibrate" : "glow"),
		     (!Blind && !same_color) ? " " : nul,
		     (Blind || same_color) ? nul :
			hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
		     otense(otmp, "evaporate"));
#else
		Your("%sはしばらくの間激しく%s%s、蒸発した。",
			    xname(otmp),
			    Blind ? nul : jconj_adj(hcolor(sobj->cursed ? NH_BLACK : NH_SILVER)),
			    Blind ? "振動し" : "輝き");
#endif
			if(is_cloak(otmp)) (void) Cloak_off();
			if(is_boots(otmp)) (void) Boots_off();
			if(is_helmet(otmp)) (void) Helmet_off();
			if(is_gloves(otmp)) (void) Gloves_off();
			if(is_shield(otmp)) (void) Shield_off();
			if(otmp == uarm) (void) Armor_gone();
			useup(otmp);
			break;
		}
		s = sobj->cursed ? -1 :
		    otmp->spe >= 9 ? (rn2(otmp->spe) == 0) :
		    sobj->blessed ? rnd(3-otmp->spe/3) : 1;
		if (s >= 0 && otmp->otyp >= GRAY_DRAGON_SCALES &&
					otmp->otyp <= YELLOW_DRAGON_SCALES) {
			/* dragon scales get turned into dragon scale mail */
/*JP
			Your("%s merges and hardens!", xname(otmp));
*/
			Your("%sは融合し固くなった！", xname(otmp));
			setworn((struct obj *)0, W_ARM);
			/* assumes same order */
			otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						otmp->otyp - GRAY_DRAGON_SCALES;
			otmp->cursed = 0;
			if (sobj->blessed) {
				otmp->spe++;
				otmp->blessed = 1;
			}
			otmp->known = 1;
			setworn(otmp, W_ARM);
			break;
		}
#if 0 /*JP*/
		Your("%s %s%s%s%s for a %s.",
			xname(otmp),
		        s == 0 ? "violently " : nul,
			otense(otmp, Blind ? "vibrate" : "glow"),
			(!Blind && !same_color) ? " " : nul,
			(Blind || same_color) ? nul : hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
			  (s*s>1) ? "while" : "moment");
#else
		Your("%sは%s%s%s%s。",
			xname(otmp),
			(s*s>1) ? "しばらくの間" : "一瞬",
		        s == 0 ? "激しく" : nul,
			Blind ? nul : 
		     jconj_adj(hcolor(sobj->cursed ? NH_BLACK : NH_SILVER)),
			Blind ? "振動した" : "輝いた");
#endif
		otmp->cursed = sobj->cursed;
		if (!otmp->blessed || sobj->cursed)
			otmp->blessed = sobj->blessed;
		if (s) {
			otmp->spe += s;
			adj_abon(otmp, s);
			known = otmp->known;
		}

		if ((otmp->spe > (special_armor ? 5 : 3)) &&
		    (special_armor || !rn2(7)))
#if 0 /*JP*/
			Your("%s suddenly %s %s.",
				xname(otmp), otense(otmp, "vibrate"),
				Blind ? "again" : "unexpectedly");
#else
			Your("%sは突然%s振動した。",
				xname(otmp),
				Blind ? "また" : "思いもよらず");
#endif
		break;
	    }
	case SCR_DESTROY_ARMOR:
	    {
		otmp = some_armor(&youmonst);
		if(confused) {
			if(!otmp) {
/*JP
				strange_feeling(sobj,"Your bones itch.");
*/
				strange_feeling(sobj,"骨がムズムズする。");
				exercise(A_STR, FALSE);
				exercise(A_CON, FALSE);
				return(1);
			}
			otmp->oerodeproof = sobj->cursed;
			p_glow2(otmp, NH_PURPLE);
			break;
		}
		if(!sobj->cursed || !otmp || !otmp->cursed) {
		    if(!destroy_arm(otmp)) {
/*JP
			strange_feeling(sobj,"Your skin itches.");
*/
			strange_feeling(sobj,"皮膚がムズムズする。");
			exercise(A_STR, FALSE);
			exercise(A_CON, FALSE);
			return(1);
		    } else
			known = TRUE;
		} else {	/* armor and scroll both cursed */
/*JP
		    Your("%s %s.", xname(otmp), otense(otmp, "vibrate"));
*/
		    Your("%sは振動した。", xname(otmp));
		    if (otmp->spe >= -6) otmp->spe--;
		    make_stunned(HStun + rn1(10, 10), TRUE);
		}
	    }
	    break;
	case SCR_CONFUSE_MONSTER:
	case SPE_CONFUSE_MONSTER:
		if(youmonst.data->mlet != S_HUMAN || sobj->cursed) {
/*JP
			if(!HConfusion) You_feel("confused.");
*/
			if(!HConfusion) You("混乱した。");
			make_confused(HConfusion + rnd(100),FALSE);
		} else  if(confused) {
		    if(!sobj->blessed) {
#if 0 /*JP*/
			Your("%s begin to %s%s.",
			    makeplural(body_part(HAND)),
			    Blind ? "tingle" : "glow ",
			    Blind ? nul : hcolor(NH_PURPLE));
#else
			Your("%sは%s%sはじめた。",
			    makeplural(body_part(HAND)),
			    Blind ? nul : jconj_adj(hcolor(NH_PURPLE)),
			    Blind ? "ヒリヒリし" : "輝き");
#endif
			make_confused(HConfusion + rnd(100),FALSE);
		    } else {
#if 0 /*JP*/
			pline("A %s%s surrounds your %s.",
			    Blind ? nul : hcolor(NH_RED),
			    Blind ? "faint buzz" : " glow",
			    body_part(HEAD));
#else
			pline("%s%sがあなたの%sを取り巻いた。",
			    Blind ? nul : jconj_adj(hcolor(NH_RED)),
			    Blind ? "かすかにブーンと鳴るもの" : "輝くもの",
			    body_part(HEAD));
#endif
			make_confused(0L,TRUE);
		    }
		} else {
		    if (!sobj->blessed) {
#if 0 /*JP*/
			Your("%s%s %s%s.",
			makeplural(body_part(HAND)),
			Blind ? "" : " begin to glow",
			Blind ? (const char *)"tingle" : hcolor(NH_RED),
			u.umconf ? " even more" : "");
#else
			Your("%sは%s%s%s。",
			makeplural(body_part(HAND)),
			u.umconf ? "少し" : "",
			Blind ? (const char *)"ヒリヒリした" : 
			     jconj_adj(hcolor(NH_RED)),
			Blind ? "" : "輝きはじめた");
#endif
			u.umconf++;
		    } else {
			if (Blind)
#if 0 /*JP*/
			    Your("%s tingle %s sharply.",
				makeplural(body_part(HAND)),
				u.umconf ? "even more" : "very");
#else
			    Your("%sは%sピリピリする。",
				makeplural(body_part(HAND)),
				u.umconf ? "少し" : "とても");
#endif
			else
#if 0 /*JP*/
			    Your("%s glow a%s brilliant %s.",
				makeplural(body_part(HAND)),
				u.umconf ? "n even more" : "",
				hcolor(NH_RED));
#else
			    Your("%sは%s%s明るく輝いた。",
				makeplural(body_part(HAND)),
				u.umconf ? "少し" : "",
				jconj_adj(hcolor(NH_RED)));
#endif
			/* after a while, repeated uses become less effective */
			if (u.umconf >= 40)
			    u.umconf++;
			else
			    u.umconf += rn1(8, 2);
		    }
		}
		break;
	case SCR_SCARE_MONSTER:
	case SPE_CAUSE_FEAR:
	    {	register int ct = 0;
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if (DEADMONSTER(mtmp)) continue;
		    if(cansee(mtmp->mx,mtmp->my)) {
			if(confused || sobj->cursed) {
			    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
			    mtmp->mcanmove = 1;
			} else
			    if (! resist(mtmp, sobj->oclass, 0, NOTELL))
				monflee(mtmp, 0, FALSE, FALSE);
			if(!mtmp->mtame) ct++;	/* pets don't laugh at you */
		    }
		}
		if(!ct)
#if 0 /*JP*/
		      You_hear("%s in the distance.",
			       (confused || sobj->cursed) ? "sad wailing" :
							"maniacal laughter");
#else
		      You_hear("遠くで%sを聞いた。",
			       (confused || sobj->cursed) ? "悲しく泣き叫ぶ声" :
							"狂ったように笑う声");
#endif
		else if(sobj->otyp == SCR_SCARE_MONSTER)
#if 0 /*JP*/
			You_hear("%s close by.",
				  (confused || sobj->cursed) ? "sad wailing" :
						 "maniacal laughter");
#else
			You_hear("近くで%sを聞いた。",
				  (confused || sobj->cursed) ? "悲しく泣き叫ぶ声" :
						 "狂ったように笑う声");
#endif
		break;
	    }
	case SCR_BLANK_PAPER:
	    if (Blind)
/*JP
		You("don't remember there being any magic words on this scroll.");
*/
		You("巻物に呪文が書いてなかったことを思いだした。");
	    else
/*JP
		pline("This scroll seems to be blank.");
*/
		pline("この巻物には何も書いてないように見える。");
	    known = TRUE;
	    break;
	case SCR_REMOVE_CURSE:
	case SPE_REMOVE_CURSE:
	    {	register struct obj *obj;
		if(confused)
		    if (Hallucination)
/*JP
			You_feel("the power of the Force against you!");
*/
			You("フォースの力があなたにはむかっているように感じた！");
		    else
/*JP
			You_feel("like you need some help.");
*/
			You("自分が助けを必要としているように感じた。");
		else
		    if (Hallucination)
/*JP
			You_feel("in touch with the Universal Oneness.");
*/
			You("宇宙原理の調和に触れているように感じた。");
		    else
/*JP
			You_feel("like someone is helping you.");
*/
			pline("誰かがあなたを助けているように感じた。");

		if (sobj->cursed) {
/*JP
		    pline_The("scroll disintegrates.");
*/
		    pline("巻物は粉々になった。");
		} else {
		    for (obj = invent; obj; obj = obj->nobj) {
			long wornmask;
#ifdef GOLDOBJ
			/* gold isn't subject to cursing and blessing */
			if (obj->oclass == COIN_CLASS) continue;
#endif
			wornmask = (obj->owornmask & ~(W_BALL|W_ART|W_ARTI));
			if (wornmask && !sobj->blessed) {
			    /* handle a couple of special cases; we don't
			       allow auxiliary weapon slots to be used to
			       artificially increase number of worn items */
			    if (obj == uswapwep) {
				if (!u.twoweap) wornmask = 0L;
			    } else if (obj == uquiver) {
				if (obj->oclass == WEAPON_CLASS) {
				    /* mergeable weapon test covers ammo,
				       missiles, spears, daggers & knives */
				    if (!objects[obj->otyp].oc_merge) 
					wornmask = 0L;
				} else if (obj->oclass == GEM_CLASS) {
				    /* possibly ought to check whether
				       alternate weapon is a sling... */
				    if (!uslinging()) wornmask = 0L;
				} else {
				    /* weptools don't merge and aren't
				       reasonable quivered weapons */
				    wornmask = 0L;
				}
			    }
			}
			if (sobj->blessed || wornmask ||
			     obj->otyp == LOADSTONE ||
			     (obj->otyp == LEASH && obj->leashmon)) {
			    if(confused) blessorcurse(obj, 2);
			    else uncurse(obj);
			}
		    }
		}
		if(Punished && !confused) unpunish();
		update_inventory();
		break;
	    }
	case SCR_CREATE_MONSTER:
	case SPE_CREATE_MONSTER:
	    if (create_critters(1 + ((confused || sobj->cursed) ? 12 : 0) +
				((sobj->blessed || rn2(73)) ? 0 : rnd(4)),
			confused ? &mons[PM_ACID_BLOB] : (struct permonst *)0))
		known = TRUE;
	    /* no need to flush monsters; we ask for identification only if the
	     * monsters are not visible
	     */
	    break;
	case SCR_ENCHANT_WEAPON:
		if(uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
			&& confused) {
		/* oclass check added 10/25/86 GAN */
			uwep->oerodeproof = !(sobj->cursed);
			if (Blind) {
			    uwep->rknown = FALSE;
/*JP
			    Your("weapon feels warm for a moment.");
*/
			    pline("武器が一瞬暖かくなったように感じた。");
			} else {
			    uwep->rknown = TRUE;
#if 0 /*JP*/
			    Your("%s covered by a %s %s %s!",
				aobjnam(uwep, "are"),
				sobj->cursed ? "mottled" : "shimmering",
				hcolor(sobj->cursed ? NH_PURPLE : NH_GOLDEN),
				sobj->cursed ? "glow" : "shield");
#else
			    Your("%sは%s%s%sで覆われた！",
				xname(uwep),
				jconj_adj(hcolor(
				  sobj->cursed ? NH_PURPLE : NH_GOLDEN)),
				sobj->cursed ? "光るまだらの" : "ゆらめく",
				sobj->cursed ? "輝き" : "バリア");
#endif
			}
			if (uwep->oerodeproof && (uwep->oeroded || uwep->oeroded2)) {
			    uwep->oeroded = uwep->oeroded2 = 0;
#if 0 /*JP*/
			    Your("%s as good as new!",
				 aobjnam(uwep, Blind ? "feel" : "look"));
#else
			    pline("%sは新品同様になったように%s！",
				 xname(uwep), Blind ? "感じた" : "見えた");
#endif
			}
		} else return !chwepon(sobj,
				       sobj->cursed ? -1 :
				       !uwep ? 1 :
				       uwep->spe >= 9 ? (rn2(uwep->spe) == 0) :
				       sobj->blessed ? rnd(3-uwep->spe/3) : 1);
		break;
	case SCR_TAMING:
	case SPE_CHARM_MONSTER:
		if (u.uswallow) {
		    maybe_tame(u.ustuck, sobj);
		} else {
		    int i, j, bd = confused ? 5 : 1;
		    struct monst *mtmp;

		    for (i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++) {
			if (!isok(u.ux + i, u.uy + j)) continue;
			if ((mtmp = m_at(u.ux + i, u.uy + j)) != 0)
			    maybe_tame(mtmp, sobj);
		    }
		}
		break;
	case SCR_GENOCIDE:
/*JP
		You("have found a scroll of genocide!");
*/
		pline("これは虐殺の巻物だ！");
		known = TRUE;
		if (sobj->blessed) do_class_genocide();
		else do_genocide(!sobj->cursed | (2 * !!Confusion));
		break;
	case SCR_LIGHT:
		if(!Blind) known = TRUE;
		litroom(!confused && !sobj->cursed, sobj);
		break;
	case SCR_TELEPORTATION:
		if(confused || sobj->cursed) level_tele();
		else {
			if (sobj->blessed && !Teleport_control) {
				known = TRUE;
/*JP
				if (yn("Do you wish to teleport?")=='n')
*/
				if (yn("瞬間移動しますか？")=='n')
					break;
			}
			tele();
			if(Teleport_control || !couldsee(u.ux0, u.uy0) ||
			   (distu(u.ux0, u.uy0) >= 16))
				known = TRUE;
		}
		break;
	case SCR_GOLD_DETECTION:
		if (confused || sobj->cursed) return(trap_detect(sobj));
		else return(gold_detect(sobj));
	case SCR_FOOD_DETECTION:
	case SPE_DETECT_FOOD:
		if (food_detect(sobj))
			return(1);	/* nothing detected */
		break;
	case SPE_IDENTIFY:
		cval = rn2(5);
		goto id;
	case SCR_IDENTIFY:
		/* known = TRUE; */
		if(confused)
/*JP
			You("identify this as an identify scroll.");
*/
			You("これは識別の巻物だと識別した。");
		else
/*JP
			pline("This is an identify scroll.");
*/
			pline("これは識別の巻物だ。");
		if (sobj->blessed || (!sobj->cursed && !rn2(5))) {
			cval = rn2(5);
			/* Note: if rn2(5)==0, identify all items */
			if (cval == 1 && sobj->blessed && Luck > 0) ++cval;
		} else	cval = 1;
		if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
		useup(sobj);
		makeknown(SCR_IDENTIFY);
	id:
		if(invent && !confused) {
		    identify_pack(cval);
		}
		return(1);
	case SCR_CHARGING:
		if (confused) {
/*JP
		    You_feel("charged up!");
*/
		    You_feel("充填されたような気がする！");
		    if (u.uen < u.uenmax)
			u.uen = u.uenmax;
		    else
			u.uen = (u.uenmax += d(5,4));
		    flags.botl = 1;
		    break;
		}
		known = TRUE;
/*JP
		pline("This is a charging scroll.");
*/
		pline("これは充填の巻物だ！");
		otmp = getobj(all_count, "charge");
		if (!otmp) break;
		recharge(otmp, sobj->cursed ? -1 : (sobj->blessed ? 1 : 0));
		break;
	case SCR_MAGIC_MAPPING:
		if (level.flags.nommap) {
/*JP
		    Your("mind is filled with crazy lines!");
*/
		    Your("心にただならぬ線！線！線！が浮びあがった！");
		    if (Hallucination)
/*JP
			pline("Wow!  Modern art.");
*/
			pline("ワォ！モダンアートだ！");
		    else
/*JP
			Your("%s spins in bewilderment.", body_part(HEAD));
*/
			You("当惑して目がまわった。");
		    make_confused(HConfusion + rnd(30), FALSE);
		    break;
		}
		if (sobj->blessed) {
		    register int x, y;

		    for (x = 1; x < COLNO; x++)
		    	for (y = 0; y < ROWNO; y++)
		    	    if (levl[x][y].typ == SDOOR)
		    	    	cvt_sdoor_to_door(&levl[x][y]);
		    /* do_mapping() already reveals secret passages */
		}
		known = TRUE;
	case SPE_MAGIC_MAPPING:
		if (level.flags.nommap) {
/*JP
		    Your("%s spins as %s blocks the spell!", body_part(HEAD), something);
*/
		    You("何かが呪文をさえぎり、目がまわった！");
		    make_confused(HConfusion + rnd(30), FALSE);
		    break;
		}
/*JP
		pline("A map coalesces in your mind!");
*/
		pline("地図があなたの心に融合した！");
		cval = (sobj->cursed && !confused);
		if(cval) HConfusion = 1;	/* to screw up map */
		do_mapping();
		if(cval) {
		    HConfusion = 0;		/* restore */
/*JP
		    pline("Unfortunately, you can't grasp the details.");
*/
		    pline("残念ながら、あなたは詳細を得ることができなかった。");
		}
		break;
	case SCR_AMNESIA:
		known = TRUE;
		forget(	(!sobj->blessed ? ALL_SPELLS : 0) |
			(!confused || sobj->cursed ? ALL_MAP : 0) );
		if (Hallucination) /* Ommmmmm! */
/*JP
			Your("mind releases itself from mundane concerns.");
*/
			Your("心は平凡な利害関係から解放された。");
		else if (!strncmpi(plname, "Maud", 4))
/*JP
			pline("As your mind turns inward on itself, you forget everything else.");
*/
			pline("あなたの心は内側に向き、全てを忘れてしまった。");
		else if (rn2(2))
/*JP
			pline("Who was that Maud person anyway?");
*/
			pline("Maudって娘はいったい誰だい？");
		else
/*JP
			pline("Thinking of Maud you forget everything else.");
*/
			pline("Maudを考えること以外、あなたは全てを忘れてしまった。");
		exercise(A_WIS, FALSE);
		break;
	case SCR_FIRE:
		/*
		 * Note: Modifications have been made as of 3.0 to allow for
		 * some damage under all potential cases.
		 */
		cval = bcsign(sobj);
		if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
		useup(sobj);
		makeknown(SCR_FIRE);
		if(confused) {
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			if(!Blind)
/*JP
			    pline("Oh, look, what a pretty fire in your %s.",
*/
			    pline("わぁごらん。小さな火が%sにある。",
				makeplural(body_part(HAND)));
/*JP
			else You_feel("a pleasant warmth in your %s.",
*/
			else You_feel("%sの中に快適な暖かさを感じた。",
				makeplural(body_part(HAND)));
		    } else {
#if 0 /*JP:T*/
			pline_The("scroll catches fire and you burn your %s.",
				makeplural(body_part(HAND)));
#else
			pline("巻物に火が燃えうつり、あなたの%sを焼いた。",
				makeplural(body_part(HAND)));
#endif
/*JP
			losehp(1, "scroll of fire", KILLED_BY_AN);
*/
			losehp(1, "炎の巻物で", KILLED_BY_AN);
		    }
		    return(1);
		}
		if (Underwater)
/*JP
			pline_The("water around you vaporizes violently!");
*/
			pline("あなたの回りの水は沸騰した！");
		else {
/*JP
		    pline_The("scroll erupts in a tower of flame!");
*/
		    pline("巻物から火柱が立ち昇った！");
		    burn_away_slime();
		}
		explode(u.ux, u.uy, 11, (2*(rn1(3, 3) + 2 * cval) + 1)/3,
							SCROLL_CLASS, EXPL_FIERY);
		return(1);
	case SCR_EARTH:
	    /* TODO: handle steeds */
	    if (
#ifdef REINCARNATION
		!Is_rogue_level(&u.uz) && 
#endif
	    	 (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
	    	register int x, y;

	    	/* Identify the scroll */
#if 0 /*JP*/
	    	pline_The("%s rumbles %s you!", ceiling(u.ux,u.uy),
	    			sobj->blessed ? "around" : "above");
#else
	    	pline("あなたの%sの%sからゴロゴロと音が聞こえてきた！",
		      sobj->blessed ? "まわり" : "真上",
		      ceiling(u.ux,u.uy));
#endif
	    	known = 1;
	    	if (In_sokoban(&u.uz))
	    	    change_luck(-1);	/* Sokoban guilt */

	    	/* Loop through the surrounding squares */
	    	if (!sobj->cursed) for (x = u.ux-1; x <= u.ux+1; x++) {
	    	    for (y = u.uy-1; y <= u.uy+1; y++) {

	    	    	/* Is this a suitable spot? */
	    	    	if (isok(x, y) && !closed_door(x, y) &&
	    	    			!IS_ROCK(levl[x][y].typ) &&
	    	    			!IS_AIR(levl[x][y].typ) &&
					(x != u.ux || y != u.uy)) {
			    register struct obj *otmp2;
			    register struct monst *mtmp;

	    	    	    /* Make the object(s) */
	    	    	    otmp2 = mksobj(confused ? ROCK : BOULDER,
	    	    	    		FALSE, FALSE);
	    	    	    if (!otmp2) continue;  /* Shouldn't happen */
	    	    	    otmp2->quan = confused ? rn1(5,2) : 1;
	    	    	    otmp2->owt = weight(otmp2);

	    	    	    /* Find the monster here (won't be player) */
	    	    	    mtmp = m_at(x, y);
	    	    	    if (mtmp && !amorphous(mtmp->data) &&
	    	    	    		!passes_walls(mtmp->data) &&
	    	    	    		!noncorporeal(mtmp->data) &&
	    	    	    		!unsolid(mtmp->data)) {
				struct obj *helmet = which_armor(mtmp, W_ARMH);
				int mdmg;

				if (cansee(mtmp->mx, mtmp->my)) {
#if 0 /*JP*/
				    pline("%s is hit by %s!", Monnam(mtmp),
	    	    	    			doname(otmp2));
#else
				    pline("%sが%sに命中した！",
					  doname(otmp2), Monnam(mtmp));
#endif
				    if (mtmp->minvis && !canspotmon(mtmp))
					map_invisible(mtmp->mx, mtmp->my);
				}
	    	    	    	mdmg = dmgval(otmp2, mtmp) * otmp2->quan;
				if (helmet) {
				    if(is_metallic(helmet)) {
					if (canspotmon(mtmp))
/*JP
					    pline("Fortunately, %s is wearing a hard helmet.", mon_nam(mtmp));
*/
					    pline("幸運にも、%sは固い兜を身につけている。", mon_nam(mtmp));
					else if (flags.soundok)
/*JP
					    You_hear("a clanging sound.");
*/
					    You_hear("ガランガランという音を聞いた。");
					if (mdmg > 2) mdmg = 2;
				    } else {
					if (canspotmon(mtmp))
#if 0 /*JP*/
					    pline("%s's %s does not protect %s.",
						Monnam(mtmp), xname(helmet),
						mhim(mtmp));
#else
					    pline("%sの%sでは守れない。",
						Monnam(mtmp), xname(helmet));
#endif
				    }
				}
	    	    	    	mtmp->mhp -= mdmg;
	    	    	    	if (mtmp->mhp <= 0)
	    	    	    	    xkilled(mtmp, 1);
	    	    	    }
	    	    	    /* Drop the rock/boulder to the floor */
/*JP
	    	    	    if (!flooreffects(otmp2, x, y, "fall")) {
*/
	    	    	    if (!flooreffects(otmp2, x, y, "落ちる")) {
	    	    	    	place_object(otmp2, x, y);
	    	    	    	stackobj(otmp2);
	    	    	    	newsym(x, y);  /* map the rock */
	    	    	    }
	    	    	}
		    }
		}
		/* Attack the player */
		if (!sobj->blessed) {
		    int dmg;
		    struct obj *otmp2;

		    /* Okay, _you_ write this without repeating the code */
		    otmp2 = mksobj(confused ? ROCK : BOULDER,
				FALSE, FALSE);
		    if (!otmp2) break;
		    otmp2->quan = confused ? rn1(5,2) : 1;
		    otmp2->owt = weight(otmp2);
		    if (!amorphous(youmonst.data) &&
				!Passes_walls &&
				!noncorporeal(youmonst.data) &&
				!unsolid(youmonst.data)) {
/*JP
			You("are hit by %s!", doname(otmp2));
*/
			pline("%sが命中した！", doname(otmp2));
			dmg = dmgval(otmp2, &youmonst) * otmp2->quan;
			if (uarmh && !sobj->cursed) {
			    if(is_metallic(uarmh)) {
/*JP
				pline("Fortunately, you are wearing a hard helmet.");
*/
				pline("幸運にも、あなたは固い兜を身につけている。");
				if (dmg > 2) dmg = 2;
			    } else if (flags.verbose) {
#if 0 /*JP*/
				Your("%s does not protect you.",
						xname(uarmh));
#else
				Your("%sでは守れない。",
						xname(uarmh));
#endif
			    }
			}
		    } else
			dmg = 0;
		    /* Must be before the losehp(), for bones files */
/*JP
		    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
*/
		    if (!flooreffects(otmp2, u.ux, u.uy, "落ちる")) {
			place_object(otmp2, u.ux, u.uy);
			stackobj(otmp2);
			newsym(u.ux, u.uy);
		    }
/*JP
		    if (dmg) losehp(dmg, "scroll of earth", KILLED_BY_AN);
*/
		    if (dmg) losehp(dmg, "大地の巻物で", KILLED_BY_AN);
		}
	    }
	    break;
	case SCR_PUNISHMENT:
		known = TRUE;
		if(confused || sobj->blessed) {
/*JP
			You_feel("guilty.");
*/
			You("罪を感じた。");
			break;
		}
		punish(sobj);
		break;
	case SCR_STINKING_CLOUD: {
	        coord cc;

/*JP
		You("have found a scroll of stinking cloud!");
*/
		You("悪臭雲の巻物を発見した！");
		known = TRUE;
/*JP
		pline("Where do you want to center the cloud?");
*/
		pline("雲の中心をどこにしますか？");
		cc.x = u.ux;
		cc.y = u.uy;
/*JP
		if (getpos(&cc, TRUE, "the desired position") < 0) {
*/
		if (getpos(&cc, TRUE, "狙う場所") < 0) {
		    pline(Never_mind);
		    return 0;
		}
		if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
/*JP
		    You("smell rotten eggs.");
*/
		    pline("くさった卵のにおいがした。");
		    return 0;
		}
		(void) create_gas_cloud(cc.x, cc.y, 3+bcsign(sobj),
						8+4*bcsign(sobj));
		break;
	}
	default:
		impossible("What weird effect is this? (%u)", sobj->otyp);
	}
	return(0);
}

static void
wand_explode(obj)
register struct obj *obj;
{
    obj->in_use = TRUE;	/* in case losehp() is fatal */
/*JP
    Your("%s vibrates violently, and explodes!",xname(obj));
*/
    Your("%sは激しく振動し、爆発した！",xname(obj));
    nhbell();
/*JP
    losehp(rnd(2*(u.uhpmax+1)/3), "exploding wand", KILLED_BY_AN);
*/
    losehp(rnd(2*(u.uhpmax+1)/3), "杖の爆発で", KILLED_BY_AN);
    useup(obj);
    exercise(A_STR, FALSE);
}

/*
 * Low-level lit-field update routine.
 */
STATIC_PTR void
set_lit(x,y,val)
int x, y;
genericptr_t val;
{
	if (val)
	    levl[x][y].lit = 1;
	else {
	    levl[x][y].lit = 0;
	    snuff_light_source(x, y);
	}
}

void
litroom(on,obj)
register boolean on;
struct obj *obj;
{
	char is_lit;	/* value is irrelevant; we use its address
			   as a `not null' flag for set_lit() */

	/* first produce the text (provided you're not blind) */
	if(!on) {
		register struct obj *otmp;

		if (!Blind) {
		    if(u.uswallow) {
/*JP
			pline("It seems even darker in here than before.");
*/
			pline("前より暗くなったように見える。");
			return;
		    }
		    if (uwep && artifact_light(uwep) && uwep->lamplit)
/*JP
			pline("Suddenly, the only light left comes from %s!",
*/
			pline("突然、明かりが%sだけになった！",
				the(xname(uwep)));
		    else
/*JP
			You("are surrounded by darkness!");
*/
			You("暗闇に覆われた！");
		}

		/* the magic douses lamps, et al, too */
		for(otmp = invent; otmp; otmp = otmp->nobj)
		    if (otmp->lamplit)
			(void) snuff_lit(otmp);
		if (Blind) goto do_it;
	} else {
		if (Blind) goto do_it;
		if(u.uswallow){
			if (is_animal(u.ustuck->data))
#if 0 /*JP*/
				pline("%s %s is lit.",
				        s_suffix(Monnam(u.ustuck)),
					mbodypart(u.ustuck, STOMACH));
#else
				pline("%sの%sは明るくなった。",
				        Monnam(u.ustuck),
					mbodypart(u.ustuck, STOMACH));
#endif
			else
				if (is_whirly(u.ustuck->data))
/*JP
					pline("%s shines briefly.",
*/
					pline("%sはちょっと輝いた。",
					      Monnam(u.ustuck));
				else
/*JP
					pline("%s glistens.", Monnam(u.ustuck));
*/
					pline("%sはきらきら輝いた。", Monnam(u.ustuck));
			return;
		}
/*JP
		pline("A lit field surrounds you!");
*/
		pline("灯りがあなたを取り囲んだ！");
	}

do_it:
	/* No-op in water - can only see the adjacent squares and that's it! */
	if (Underwater || Is_waterlevel(&u.uz)) return;
	/*
	 *  If we are darkening the room and the hero is punished but not
	 *  blind, then we have to pick up and replace the ball and chain so
	 *  that we don't remember them if they are out of sight.
	 */
	if (Punished && !on && !Blind)
	    move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) {
	    /* Can't use do_clear_area because MAX_RADIUS is too small */
	    /* rogue lighting must light the entire room */
	    int rnum = levl[u.ux][u.uy].roomno - ROOMOFFSET;
	    int rx, ry;
	    if(rnum >= 0) {
		for(rx = rooms[rnum].lx-1; rx <= rooms[rnum].hx+1; rx++)
		    for(ry = rooms[rnum].ly-1; ry <= rooms[rnum].hy+1; ry++)
			set_lit(rx, ry,
				(genericptr_t)(on ? &is_lit : (char *)0));
		rooms[rnum].rlit = on;
	    }
	    /* hallways remain dark on the rogue level */
	} else
#endif
	    do_clear_area(u.ux,u.uy,
		(obj && obj->oclass==SCROLL_CLASS && obj->blessed) ? 9 : 5,
		set_lit, (genericptr_t)(on ? &is_lit : (char *)0));

	/*
	 *  If we are not blind, then force a redraw on all positions in sight
	 *  by temporarily blinding the hero.  The vision recalculation will
	 *  correctly update all previously seen positions *and* correctly
	 *  set the waslit bit [could be messed up from above].
	 */
	if (!Blind) {
	    vision_recalc(2);

	    /* replace ball&chain */
	    if (Punished && !on)
		move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
	}

	vision_full_recalc = 1;	/* delayed vision recalculation */
}

static void
do_class_genocide()
{
	int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0;
	char buf[BUFSZ];
	boolean gameover = FALSE;	/* true iff killed self */

	for(j=0; ; j++) {
		if (j >= 5) {
			pline(thats_enough_tries);
			return;
		}
		do {
/*JP
		    getlin("What class of monsters do you wish to genocide?",
*/
		    getlin("どのクラスに属する怪物を虐殺しますか？[文字を入れてね]",
			buf);
		    (void)mungspaces(buf);
		} while (buf[0]=='\033' || !buf[0]);
		/* choosing "none" preserves genocideless conduct */
#if 0 /*JP*/
		if (!strcmpi(buf, "none") ||
		    !strcmpi(buf, "nothing")) return;
#else
		if (!strcmpi(buf, "none") ||
		    !strcmpi(buf, "なし") ||
		    !strcmpi(buf, "nothing")) return;
#endif

		if (strlen(buf) == 1) {
		    if (buf[0] == ILLOBJ_SYM)
			buf[0] = def_monsyms[S_MIMIC];
		    class = def_char_to_monclass(buf[0]);
		} else {
		    char buf2[BUFSZ];

		    class = 0;
		    Strcpy(buf2, makesingular(buf));
		    Strcpy(buf, buf2);
		}
		immunecnt = gonecnt = goodcnt = 0;
		for (i = LOW_PM; i < NUMMONS; i++) {
		    if (class == 0 &&
			    strstri(monexplain[(int)mons[i].mlet], buf) != 0)
			class = mons[i].mlet;
		    if (mons[i].mlet == class) {
			if (!(mons[i].geno & G_GENO)) immunecnt++;
			else if(mvitals[i].mvflags & G_GENOD) gonecnt++;
			else goodcnt++;
		    }
		}
		/*
		 * TODO[?]: If user's input doesn't match any class
		 *	    description, check individual species names.
		 */
		if (!goodcnt && class != mons[urole.malenum].mlet &&
				class != mons[urace.malenum].mlet) {
			if (gonecnt)
/*JP
	pline("All such monsters are already nonexistent.");
*/
	pline("その怪物はもういない。");
			else if (immunecnt ||
				(buf[0] == DEF_INVISIBLE && buf[1] == '\0'))
/*JP
	You("aren't permitted to genocide such monsters.");
*/
	You("その怪物を虐殺することはできない。");
			else
#ifdef WIZARD	/* to aid in topology testing; remove pesky monsters */
			  if (wizard && buf[0] == '*') {
			    register struct monst *mtmp, *mtmp2;

			    gonecnt = 0;
			    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
				mtmp2 = mtmp->nmon;
			    	if (DEADMONSTER(mtmp)) continue;
				mongone(mtmp);
				gonecnt++;
			    }
/*JP
	pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
*/
	pline("%dの怪物を除いた。", gonecnt);
			    return;
			} else
#endif
/*JP
	pline("That symbol does not represent any monster.");
*/
	pline("その記号の怪物はいない。");
			continue;
		}

		for (i = LOW_PM; i < NUMMONS; i++) {
		    if(mons[i].mlet == class) {
			char nam[BUFSZ];

			Strcpy(nam, makeplural(mons[i].mname));
			/* Although "genus" is Latin for race, the hero benefits
			 * from both race and role; thus genocide affects either.
			 */
			if (Your_Own_Role(i) || Your_Own_Race(i) ||
				((mons[i].geno & G_GENO)
				&& !(mvitals[i].mvflags & G_GENOD))) {
			/* This check must be first since player monsters might
			 * have G_GENOD or !G_GENO.
			 */
			    mvitals[i].mvflags |= (G_GENOD|G_NOCORPSE);
			    reset_rndmonst(i);
			    kill_genocided_monsters();
			    update_inventory();		/* eggs & tins */
/*JP
			    pline("Wiped out all %s.", nam);
*/
			    pline("%sを全て排除した。", jtrns_mon(nam));
			    if (Upolyd && i == u.umonnum) {
				u.mh = -1;
				if (Unchanging) {
/*JP
				    if (!feel_dead++) You("die.");
*/
				    if (!feel_dead++) You("死にました…");
				    /* finish genociding this class of
				       monsters before ultimately dying */
				    gameover = TRUE;
				} else
				    rehumanize();
			    }
			    /* Self-genocide if it matches either your race
			       or role.  Assumption:  male and female forms
			       share same monster class. */
			    if (i == urole.malenum || i == urace.malenum) {
				u.uhp = -1;
				if (Upolyd) {
/*JP
				    if (!feel_dead++) You_feel("dead inside.");
*/
				    if (!feel_dead++) You("魂が死んだように感じた。");
				} else {
/*JP
				    if (!feel_dead++) You("die.");
*/
				    if (!feel_dead++) You("死にました…");
				    gameover = TRUE;
				}
			    }
			} else if (mvitals[i].mvflags & G_GENOD) {
			    if (!gameover)
/*JP
				pline("All %s are already nonexistent.", nam);
*/
				pline("%sは既にいない。", jtrns_mon(nam));
			} else if (!gameover) {
			  /* suppress feedback about quest beings except
			     for those applicable to our own role */
			  if ((mons[i].msound != MS_LEADER ||
			       quest_info(MS_LEADER) == i)
			   && (mons[i].msound != MS_NEMESIS ||
			       quest_info(MS_NEMESIS) == i)
			   && (mons[i].msound != MS_GUARDIAN ||
			       quest_info(MS_GUARDIAN) == i)
			/* non-leader/nemesis/guardian role-specific monster */
			   && (i != PM_NINJA ||		/* nuisance */
			       Role_if(PM_SAMURAI))) {
				boolean named, uniq;

				named = type_is_pname(&mons[i]) ? TRUE : FALSE;
				uniq = (mons[i].geno & G_UNIQ) ? TRUE : FALSE;
				/* one special case */
				if (i == PM_HIGH_PRIEST) uniq = FALSE;

#if 0 /*JP*/
				You("aren't permitted to genocide %s%s.",
				    (uniq && !named) ? "the " : "",
				    (uniq || named) ? mons[i].mname : nam);
#else
				You("%sを虐殺できない。",
				    jtrns_mon((uniq || named) ? mons[i].mname : nam));
#endif
			    }
			}
		    }
		}
		if (gameover || u.uhp == -1) {
		    killer_format = KILLED_BY_AN;
/*JP
		    killer = "scroll of genocide";
*/
				killer = "虐殺の巻物で";
		    if (gameover) done(GENOCIDED);
		}
		return;
	}
}

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide(how)
int how;
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
/* 5 (4 | 1) = normal genocide from throne */
{
	char buf[BUFSZ];
	register int	i, killplayer = 0;
	register int mndx;
	register struct permonst *ptr;
	const char *which;

	if (how & PLAYER) {
		mndx = u.umonster;	/* non-polymorphed mon num */
		ptr = &mons[mndx];
		Strcpy(buf, ptr->mname);
		killplayer++;
	} else {
	    for(i = 0; ; i++) {
		if(i >= 5) {
		    pline(thats_enough_tries);
		    return;
		}
/*JP
		getlin("What monster do you want to genocide? [type the name]",
*/
		getlin("どの怪物を虐殺しますか？",
			buf);
		(void)mungspaces(buf);
#if 1 /*JP*/
		{
		    const char *p;
		    p = etrns_mon(buf);
		    if(p) strcpy(buf, p);
		}
#endif

		/* choosing "none" preserves genocideless conduct */
#if 0 /*JP*/
		if (!strcmpi(buf, "none") || !strcmpi(buf, "nothing")) {
#else
		if (!strcmpi(buf, "none") || !strcmpi(buf, "nothing")
		    || !strcmpi(buf, "なし")){
#endif
		    /* ... but no free pass if cursed */
		    if (!(how & REALLY)) {
			ptr = rndmonst();
			if (!ptr) return; /* no message, like normal case */
			mndx = monsndx(ptr);
			break;		/* remaining checks don't apply */
		    } else return;
		}

		mndx = name_to_mon(buf);
		if (mndx == NON_PM || (mvitals[mndx].mvflags & G_GENOD)) {
#if 0 /*JP*/
			pline("Such creatures %s exist in this world.",
			      (mndx == NON_PM) ? "do not" : "no longer");
#else
			pline("そのような生き物は%sこの世界に存在しない。",
			      (mndx == NON_PM) ? "" : "もはや");
#endif
			continue;
		}
		ptr = &mons[mndx];
		/* Although "genus" is Latin for race, the hero benefits
		 * from both race and role; thus genocide affects either.
		 */
		if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
			killplayer++;
			break;
		}
		if (is_human(ptr)) adjalign(-sgn(u.ualign.type));
		if (is_demon(ptr)) adjalign(sgn(u.ualign.type));

		if(!(ptr->geno & G_GENO)) {
			if(flags.soundok) {
	/* fixme: unconditional "caverns" will be silly in some circumstances */
			    if(flags.verbose)
/*JP
			pline("A thunderous voice booms through the caverns:");
*/
			pline("雷のような声が洞窟に響いた：");
/*JP
			    verbalize("No, mortal!  That will not be done.");
*/
			    pline("「定命の者よ！その望みはかなうまい。」");
			}
			continue;
		}
		/* KMH -- Unchanging prevents rehumanization */
		if (Unchanging && ptr == youmonst.data)
		    killplayer++;
		break;
	    }
	}

/*JP
	which = "all ";
*/
	which = "全て";
	if (Hallucination) {
	    if (Upolyd)
		Strcpy(buf,youmonst.data->mname);
	    else {
		Strcpy(buf, (flags.female && urole.name.f) ?
				urole.name.f : urole.name.m);
		buf[0] = lowc(buf[0]);
	    }
	} else {
	    Strcpy(buf, ptr->mname); /* make sure we have standard singular */
	    if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_PRIEST])
/*JP
		which = !type_is_pname(ptr) ? "the " : "";
*/
		which = !type_is_pname(ptr) ? "" : "";
	}
	if (how & REALLY) {
	    /* setting no-corpse affects wishing and random tin generation */
	    mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE);
#if 0 /*JP*/
	    pline("Wiped out %s%s.", which,
		  (*which != 'a') ? buf : makeplural(buf));
#else
	    pline("%sを%s一掃した。", jtrns_mon(buf), which);
#endif

	    if (killplayer) {
		/* might need to wipe out dual role */
		if (urole.femalenum != NON_PM && mndx == urole.malenum)
		    mvitals[urole.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urole.femalenum != NON_PM && mndx == urole.femalenum)
		    mvitals[urole.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urace.femalenum != NON_PM && mndx == urace.malenum)
		    mvitals[urace.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urace.femalenum != NON_PM && mndx == urace.femalenum)
		    mvitals[urace.malenum].mvflags |= (G_GENOD | G_NOCORPSE);

		u.uhp = -1;
		if (how & PLAYER) {
		    killer_format = KILLED_BY;
/*JP
		    killer = "genocidal confusion";
*/
		    killer = "混乱による自虐的虐殺で";
		} else if (how & ONTHRONE) {
		    /* player selected while on a throne */
		    killer_format = KILLED_BY_AN;
/*JP
		    killer = "imperious order";
*/
		    killer = "傲慢な命令で";
		} else { /* selected player deliberately, not confused */
		    killer_format = KILLED_BY_AN;
/*JP
		    killer = "scroll of genocide";
*/
		    killer = "虐殺の巻物で";
		}

	/* Polymorphed characters will die as soon as they're rehumanized. */
	/* KMH -- Unchanging prevents rehumanization */
		if (Upolyd && ptr != youmonst.data) {
			delayed_killer = killer;
			killer = 0;
/*JP
			You_feel("dead inside.");
*/
				    You("魂が死んだように感じた。");
		} else
			done(GENOCIDED);
	    } else if (ptr == youmonst.data) {
		rehumanize();
	    }
	    reset_rndmonst(mndx);
	    kill_genocided_monsters();
	    update_inventory();	/* in case identified eggs were affected */
	} else {
	    int cnt = 0;

	    if (!(mons[mndx].geno & G_UNIQ) &&
		    !(mvitals[mndx].mvflags & (G_GENOD | G_EXTINCT)))
		for (i = rn1(3, 4); i > 0; i--) {
		    if (!makemon(ptr, u.ux, u.uy, NO_MINVENT))
			break;	/* couldn't make one */
		    ++cnt;
		    if (mvitals[mndx].mvflags & G_EXTINCT)
			break;	/* just made last one */
		}
	    if (cnt)
/*JP
		pline("Sent in some %s.", makeplural(buf));
*/
	        pline("%sが送られてきた。", jtrns_mon(buf));
	    else
		pline(nothing_happens);
	}
}

void
punish(sobj)
register struct obj	*sobj;
{
	/* KMH -- Punishment is still okay when you are riding */
/*JP
	You("are being punished for your misbehavior!");
*/
	You("不作法のため罰を受けた！");
	if(Punished){
/*JP
		Your("iron ball gets heavier.");
*/
		Your("鉄球はさらに重くなった。");
		uball->owt += 160 * (1 + sobj->cursed);
		return;
	}
	if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
/*JP
		pline("A ball and chain appears, then falls away.");
*/
		pline("鉄球と鎖が現われたが、するっと抜けた。");
		dropy(mkobj(BALL_CLASS, TRUE));
		return;
	}
	setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
	setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
	uball->spe = 1;		/* special ball (see save) */

	/*
	 *  Place ball & chain if not swallowed.  If swallowed, the ball &
	 *  chain variables will be set at the next call to placebc().
	 */
	if (!u.uswallow) {
	    placebc();
	    if (Blind) set_bc(1);	/* set up ball and chain variables */
	    newsym(u.ux,u.uy);		/* see ball&chain if can't see self */
	}
}

void
unpunish()
{	    /* remove the ball and chain */
	struct obj *savechain = uchain;

	obj_extract_self(uchain);
	newsym(uchain->ox,uchain->oy);
	setworn((struct obj *)0, W_CHAIN);
	dealloc_obj(savechain);
	uball->spe = 0;
	setworn((struct obj *)0, W_BALL);
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to revive
 * one, the disoriented creature becomes a zombie
 */
boolean
cant_create(mtype, revival)
int *mtype;
boolean revival;
{

	/* SHOPKEEPERS can be revived now */
	if (*mtype==PM_GUARD || (*mtype==PM_SHOPKEEPER && !revival)
	     || *mtype==PM_ALIGNED_PRIEST || *mtype==PM_ANGEL) {
		*mtype = PM_HUMAN_ZOMBIE;
		return TRUE;
	} else if (*mtype==PM_LONG_WORM_TAIL) {	/* for create_particular() */
		*mtype = PM_LONG_WORM;
		return TRUE;
	}
	return FALSE;
}

#ifdef WIZARD
/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic; this behavior quirk is useful so don't "fix" it...
 */
boolean
create_particular()
{
	char buf[BUFSZ], *bufp, monclass = MAXMCLASSES;
	int which, tries, i;
	struct permonst *whichpm;
	struct monst *mtmp;
	boolean madeany = FALSE;
	boolean maketame, makepeaceful, makehostile;

	tries = 0;
	do {
	    which = urole.malenum;	/* an arbitrary index into mons[] */
	    maketame = makepeaceful = makehostile = FALSE;
/*JP
	    getlin("Create what kind of monster? [type the name or symbol]",
*/
	    getlin("どの種の怪物を作りますか？[名前を入れてね]",
		   buf);
	    bufp = mungspaces(buf);
#if 1 /*JP*/
	    {
		const char *p;
		p = etrns_mon(buf);
		if(p) strcpy(buf, p);
	    }
#endif
	    if (*bufp == '\033') return FALSE;
	    /* allow the initial disposition to be specified */
	    if (!strncmpi(bufp, "tame ", 5)) {
		bufp += 5;
		maketame = TRUE;
	    } else if (!strncmpi(bufp, "peaceful ", 9)) {
		bufp += 9;
		makepeaceful = TRUE;
	    } else if (!strncmpi(bufp, "hostile ", 8)) {
		bufp += 8;
		makehostile = TRUE;
	    }
	    /* decide whether a valid monster was chosen */
	    if (strlen(bufp) == 1) {
		monclass = def_char_to_monclass(*bufp);
		if (monclass != MAXMCLASSES) break;	/* got one */
	    } else {
		which = name_to_mon(bufp);
		if (which >= LOW_PM) break;		/* got one */
	    }
	    /* no good; try again... */
/*JP
	    pline("I've never heard of such monsters.");
*/
		    pline("そのような怪物は聞いたことがない。");
	} while (++tries < 5);

	if (tries == 5) {
	    pline(thats_enough_tries);
	} else {
	    (void) cant_create(&which, FALSE);
	    whichpm = &mons[which];
	    for (i = 0; i <= multi; i++) {
		if (monclass != MAXMCLASSES)
		    whichpm = mkclass(monclass, 0);
		if (maketame) {
		    mtmp = makemon(whichpm, u.ux, u.uy, MM_EDOG);
		    if (mtmp) {
			initedog(mtmp);
			set_malign(mtmp);
		    }
		} else {
		    mtmp = makemon(whichpm, u.ux, u.uy, NO_MM_FLAGS);
		    if ((makepeaceful || makehostile) && mtmp) {
			mtmp->mtame = 0;	/* sanity precaution */
			mtmp->mpeaceful = makepeaceful ? 1 : 0;
			set_malign(mtmp);
		    }
		}
		if (mtmp) madeany = TRUE;
	    }
	}
	return madeany;
}
#endif /* WIZARD */

#endif /* OVLB */

/*read.c*/
