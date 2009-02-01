/*	SCCS Id: @(#)dokick.c	3.4	2003/12/04	*/
/* Copyright (c) Izchak Miller, Mike Stephenson, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	changing point is marked `JP' (94/6/7)
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "eshk.h"

#define is_bigfoot(x)	((x) == &mons[PM_SASQUATCH])
#define martial()	(martial_bonus() || is_bigfoot(youmonst.data) || \
		(uarmf && uarmf->otyp == KICKING_BOOTS))

static NEARDATA struct rm *maploc;
static NEARDATA const char *gate_str;

extern boolean notonhead;	/* for long worms */

STATIC_DCL void FDECL(kickdmg, (struct monst *, BOOLEAN_P));
STATIC_DCL void FDECL(kick_monster, (XCHAR_P, XCHAR_P));
STATIC_DCL int FDECL(kick_object, (XCHAR_P, XCHAR_P));
STATIC_DCL char *FDECL(kickstr, (char *));
STATIC_DCL void FDECL(otransit_msg, (struct obj *, BOOLEAN_P, long));
STATIC_DCL void FDECL(drop_to, (coord *,SCHAR_P));

static NEARDATA struct obj *kickobj;

/*JP
static const char kick_passes_thru[] = "kick passes harmlessly through";
*/
static const char kick_passes_thru[] = "蹴りはダメージを与えずにすり抜けた";

STATIC_OVL void
kickdmg(mon, clumsy)
register struct monst *mon;
register boolean clumsy;
{
	register int mdx, mdy;
	register int dmg = ( ACURRSTR + ACURR(A_DEX) + ACURR(A_CON) )/ 15;
	int kick_skill = P_NONE;
	int blessed_foot_damage = 0;
	boolean trapkilled = FALSE;

	if (uarmf && uarmf->otyp == KICKING_BOOTS)
	    dmg += 5;

	/* excessive wt affects dex, so it affects dmg */
	if (clumsy) dmg /= 2;

	/* kicking a dragon or an elephant will not harm it */
	if (thick_skinned(mon->data)) dmg = 0;

	/* attacking a shade is useless */
	if (mon->data == &mons[PM_SHADE])
	    dmg = 0;

	if ((is_undead(mon->data) || is_demon(mon->data)) && uarmf &&
		uarmf->blessed)
	    blessed_foot_damage = 1;

	if (mon->data == &mons[PM_SHADE] && !blessed_foot_damage) {
/*JP
	    pline_The("%s.", kick_passes_thru);
*/
	    pline_The("%s。", kick_passes_thru);
	    /* doesn't exercise skill or abuse alignment or frighten pet,
	       and shades have no passive counterattack */
	    return;
	}

	if(mon->m_ap_type) seemimic(mon);

	check_caitiff(mon);

	/* squeeze some guilt feelings... */
	if(mon->mtame) {
	    abuse_dog(mon);
	    if (mon->mtame)
		monflee(mon, (dmg ? rnd(dmg) : 1), FALSE, FALSE);
	    else
		mon->mflee = 0;
	}

	if (dmg > 0) {
		/* convert potential damage to actual damage */
		dmg = rnd(dmg);
		if (martial()) {
		    if (dmg > 1) kick_skill = P_MARTIAL_ARTS;
		    dmg += rn2(ACURR(A_DEX)/2 + 1);
		}
		/* a good kick exercises your dex */
		exercise(A_DEX, TRUE);
	}
	if (blessed_foot_damage) dmg += rnd(4);
	if (uarmf) dmg += uarmf->spe;
	dmg += u.udaminc;	/* add ring(s) of increase damage */
	if (dmg > 0)
		mon->mhp -= dmg;
	if (mon->mhp > 0 && martial() && !bigmonst(mon->data) && !rn2(3) &&
	    mon->mcanmove && mon != u.ustuck && !mon->mtrapped) {
		/* see if the monster has a place to move into */
		mdx = mon->mx + u.dx;
		mdy = mon->my + u.dy;
		if(goodpos(mdx, mdy, mon, 0)) {
/*JP
			pline("%s reels from the blow.", Monnam(mon));
*/
			pline("%sは強打されよろめいた。", Monnam(mon));
			if (m_in_out_region(mon, mdx, mdy)) {
			    remove_monster(mon->mx, mon->my);
			    newsym(mon->mx, mon->my);
			    place_monster(mon, mdx, mdy);
			    newsym(mon->mx, mon->my);
			    set_apparxy(mon);
			    if (mintrap(mon) == 2) trapkilled = TRUE;
			}
		}
	}

	(void) passive(mon, TRUE, mon->mhp > 0, AT_KICK);
	if (mon->mhp <= 0 && !trapkilled) killed(mon);

	/* may bring up a dialog, so put this after all messages */
	if (kick_skill != P_NONE)	/* exercise proficiency */
	    use_skill(kick_skill, 1);
}

STATIC_OVL void
kick_monster(x, y)
register xchar x, y;
{
	register boolean clumsy = FALSE;
	register struct monst *mon = m_at(x, y);
	register int i, j;

	bhitpos.x = x;
	bhitpos.y = y;
	if (attack_checks(mon, (struct obj *)0)) return;
	setmangry(mon);

	/* Kick attacks by kicking monsters are normal attacks, not special.
	 * This is almost always worthless, since you can either take one turn
	 * and do all your kicks, or else take one turn and attack the monster
	 * normally, getting all your attacks _including_ all your kicks.
	 * If you have >1 kick attack, you get all of them.
	 */
	if (Upolyd && attacktype(youmonst.data, AT_KICK)) {
	    struct attack *uattk;
	    int sum;
	    schar tmp = find_roll_to_hit(mon);

	    for (i = 0; i < NATTK; i++) {
		/* first of two kicks might have provoked counterattack
		   that has incapacitated the hero (ie, floating eye) */
		if (multi < 0) break;

		uattk = &youmonst.data->mattk[i];
		/* we only care about kicking attacks here */
		if (uattk->aatyp != AT_KICK) continue;

		if (mon->data == &mons[PM_SHADE] &&
			(!uarmf || !uarmf->blessed)) {
		    /* doesn't matter whether it would have hit or missed,
		       and shades have no passive counterattack */
/*JP
		    Your("%s %s.", kick_passes_thru, mon_nam(mon));
*/
		    You("%sを蹴ったが、%s。", mon_nam(mon), kick_passes_thru);
		    break;	/* skip any additional kicks */
		} else if (tmp > rnd(20)) {
/*JP
		    You("kick %s.", mon_nam(mon));
*/
		    You("%sを蹴った。", mon_nam(mon));
		    sum = damageum(mon, uattk);
		    (void)passive(mon, (boolean)(sum > 0), (sum != 2), AT_KICK);
		    if (sum == 2)
			break;		/* Defender died */
		} else {
		    missum(mon, uattk);
		    (void)passive(mon, 0, 1, AT_KICK);
		}
	    }
	    return;
	}

	if(Levitation && !rn2(3) && verysmall(mon->data) &&
	   !is_flyer(mon->data)) {
/*JP
		pline("Floating in the air, you miss wildly!");
*/
		pline("空中に浮いているので、大きく外した！");
		exercise(A_DEX, FALSE);
		(void) passive(mon, FALSE, 1, AT_KICK);
		return;
	}

	i = -inv_weight();
	j = weight_cap();

	if(i < (j*3)/10) {
		if(!rn2((i < j/10) ? 2 : (i < j/5) ? 3 : 4)) {
			if(martial() && !rn2(2)) goto doit;
/*JP
			Your("clumsy kick does no damage.");
*/
			Your("不器用な蹴りでダメージを与えられなかった。");
			(void) passive(mon, FALSE, 1, AT_KICK);
			return;
		}
		if(i < j/10) clumsy = TRUE;
		else if(!rn2((i < j/5) ? 2 : 3)) clumsy = TRUE;
	}

	if(Fumbling) clumsy = TRUE;

	else if(uarm && objects[uarm->otyp].oc_bulky && ACURR(A_DEX) < rnd(25))
		clumsy = TRUE;
doit:
/*JP
	You("kick %s.", mon_nam(mon));
*/
	You("%sを蹴った。", mon_nam(mon));
	if(!rn2(clumsy ? 3 : 4) && (clumsy || !bigmonst(mon->data)) &&
	   mon->mcansee && !mon->mtrapped && !thick_skinned(mon->data) &&
	   mon->data->mlet != S_EEL && haseyes(mon->data) && mon->mcanmove &&
	   !mon->mstun && !mon->mconf && !mon->msleeping &&
	   mon->data->mmove >= 12) {
		if(!nohands(mon->data) && !rn2(martial() ? 5 : 3)) {
#if 0 /*JP*/
		    pline("%s blocks your %skick.", Monnam(mon),
				clumsy ? "clumsy " : "");
#else
		    pline("%sはあなたの%s蹴りを防いだ。", Monnam(mon),
				clumsy ? "不器用な" : "");
#endif
		    (void) passive(mon, FALSE, 1, AT_KICK);
		    return;
		} else {
		    mnexto(mon);
		    if(mon->mx != x || mon->my != y) {
			if(glyph_is_invisible(levl[x][y].glyph)) {
			    unmap_object(x, y);
			    newsym(x, y);
			}
#if 0 /*JP*/
			pline("%s %s, %s evading your %skick.", Monnam(mon),
				(can_teleport(mon->data) ? "teleports" :
				 is_floater(mon->data) ? "floats" :
				 is_flyer(mon->data) ? "swoops" :
				 (nolimbs(mon->data) || slithy(mon->data)) ?
					"slides" : "jumps"),
				clumsy ? "easily" : "nimbly",
				clumsy ? "clumsy " : "");
#else
			pline("%sは%s、%sあなたの%s蹴りをたくみに避けた。", Monnam(mon),
				(can_teleport(mon->data) ? "瞬間移動し" :
				 is_floater(mon->data) ? "浮き" :
				 is_flyer(mon->data) ? "はばたき" :
				 (nolimbs(mon->data)  || slithy(mon->data))
					? "横に滑り" : "跳ね"),
				clumsy ? "楽々と" : "素早く",
				clumsy ? "不器用な" : "");
#endif
			(void) passive(mon, FALSE, 1, AT_KICK);
			return;
		    }
		}
	}
	kickdmg(mon, clumsy);
}

/*
 *  Return TRUE if caught (the gold taken care of), FALSE otherwise.
 *  The gold object is *not* attached to the fobj chain!
 */
boolean
ghitm(mtmp, gold)
register struct monst *mtmp;
register struct obj *gold;
{
	boolean msg_given = FALSE;

	if(!likes_gold(mtmp->data) && !mtmp->isshk && !mtmp->ispriest
			&& !is_mercenary(mtmp->data)) {
		wakeup(mtmp);
	} else if (!mtmp->mcanmove) {
		/* too light to do real damage */
		if (canseemon(mtmp)) {
#if 0 /*JP:T*/
		    pline_The("%s harmlessly %s %s.", xname(gold),
			      otense(gold, "hit"), mon_nam(mtmp));
#else
		    pline("%sは%sに命中した。", xname(gold),
			      mon_nam(mtmp));
#endif
		    msg_given = TRUE;
		}
	} else {
#ifdef GOLDOBJ
                long value = gold->quan * objects[gold->otyp].oc_cost;
#endif
		mtmp->msleeping = 0;
		mtmp->meating = 0;
		if(!rn2(4)) setmangry(mtmp); /* not always pleasing */

		/* greedy monsters catch gold */
		if (cansee(mtmp->mx, mtmp->my))
/*JP
		    pline("%s catches the gold.", Monnam(mtmp));
*/
		    pline("%sは金貨をキャッチした。", Monnam(mtmp));
#ifndef GOLDOBJ
		mtmp->mgold += gold->quan;
#endif
		if (mtmp->isshk) {
			long robbed = ESHK(mtmp)->robbed;

			if (robbed) {
#ifndef GOLDOBJ
				robbed -= gold->quan;
#else
				robbed -= value;
#endif
				if (robbed < 0) robbed = 0;
#if 0 /*JP*/
				pline_The("amount %scovers %s recent losses.",
				      !robbed ? "" : "partially ",
				      mhis(mtmp));
#else
				pline("%s%s損失を補填するのに使われた。",
				      !robbed ? "" : "金の一部は",
				      mhis(mtmp));
#endif
				ESHK(mtmp)->robbed = robbed;
				if(!robbed)
					make_happy_shk(mtmp, FALSE);
			} else {
				if(mtmp->mpeaceful) {
#ifndef GOLDOBJ
				    ESHK(mtmp)->credit += gold->quan;
#else
				    ESHK(mtmp)->credit += value;
#endif
#if 0 /*JP*/
				    You("have %ld %s in credit.",
#else
				    You("%ld%sを貸しにした。",
#endif
					ESHK(mtmp)->credit,
					currency(ESHK(mtmp)->credit));
/*JP
				} else verbalize("Thanks, scum!");
*/
				} else verbalize("ありがとよ！くそったれ！");
			}
		} else if (mtmp->ispriest) {
			if (mtmp->mpeaceful)
/*JP
			    verbalize("Thank you for your contribution.");
*/
			    verbalize("寄付に感謝します。");
/*JP
			else verbalize("Thanks, scum!");
*/
			else verbalize("ありがとよ、クズめ！");
		} else if (is_mercenary(mtmp->data)) {
		    long goldreqd = 0L;

		    if (rn2(3)) {
			if (mtmp->data == &mons[PM_SOLDIER])
			   goldreqd = 100L;
			else if (mtmp->data == &mons[PM_SERGEANT])
			   goldreqd = 250L;
			else if (mtmp->data == &mons[PM_LIEUTENANT])
			   goldreqd = 500L;
			else if (mtmp->data == &mons[PM_CAPTAIN])
			   goldreqd = 750L;

			if (goldreqd) {
#ifndef GOLDOBJ
			   if (gold->quan > goldreqd +
				(u.ugold + u.ulevel*rn2(5))/ACURR(A_CHA))
#else
			   if (value > goldreqd +
				(money_cnt(invent) + u.ulevel*rn2(5))/ACURR(A_CHA))
#endif
			    mtmp->mpeaceful = TRUE;
			}
		     }
		     if (mtmp->mpeaceful)
/*JP
			    verbalize("That should do.  Now beat it!");
*/
			    verbalize("まあいいだろう。さあ失せろ！");
/*JP
		     else verbalize("That's not enough, coward!");
*/
		     else verbalize("まだ足りんぞ、腰抜けめ！");
		}

#ifndef GOLDOBJ
		dealloc_obj(gold);
#else
		add_to_minv(mtmp, gold);
#endif
		return TRUE;
	}

	if (!msg_given) miss(xname(gold), mtmp);
	return FALSE;
}

/* container is kicked, dropped, thrown or otherwise impacted by player.
 * Assumes container is on floor.  Checks contents for possible damage. */
void
container_impact_dmg(obj)
struct obj *obj;
{
	struct monst *shkp;
	struct obj *otmp, *otmp2;
	long loss = 0L;
	boolean costly, insider;
	xchar x = obj->ox, y = obj->oy;

	/* only consider normal containers */
	if (!Is_container(obj) || Is_mbag(obj)) return;

	costly = ((shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) &&
		  costly_spot(x, y));
	insider = (*u.ushops && inside_shop(u.ux, u.uy) &&
		   *in_rooms(x, y, SHOPBASE) == *u.ushops);

	for (otmp = obj->cobj; otmp; otmp = otmp2) {
	    const char *result = (char *)0;

	    otmp2 = otmp->nobj;
	    if (objects[otmp->otyp].oc_material == GLASS &&
		otmp->oclass != GEM_CLASS && !obj_resists(otmp, 33, 100)) {
/*JP
		result = "shatter";
*/
				result = "ガチャン";
	    } else if (otmp->otyp == EGG && !rn2(3)) {
/*JP
		result = "cracking";
*/
				result = "グシャッ";
	    }
	    if (result) {
		if (otmp->otyp == MIRROR) change_luck(-2);

		/* eggs laid by you.  penalty is -1 per egg, max 5,
		 * but it's always exactly 1 that breaks */
		if (otmp->otyp == EGG && otmp->spe && otmp->corpsenm >= LOW_PM)
		    change_luck(-1);
/*JP
		You_hear("a muffled %s.", result);
*/
		You_hear("こもった%sという音を聞いた。", result);
		if (costly)
		    loss += stolen_value(otmp, x, y,
					 (boolean)shkp->mpeaceful, TRUE);
		if (otmp->quan > 1L)
		    useup(otmp);
		else {
		    obj_extract_self(otmp);
		    obfree(otmp, (struct obj *) 0);
		}
	    }
	}
	if (costly && loss) {
	    if (!insider) {
/*JP
		You("caused %ld %s worth of damage!", loss, currency(loss));
*/
		You("%ld%s分の損害をひきおこした！", loss, currency(loss));
		make_angry_shk(shkp, x, y);
	    } else {
#if 0 /*JP*/
		You("owe %s %ld %s for objects destroyed.",
#else
		You("器物破損で%sに%ld%sの借りをつくった。",
#endif
		    mon_nam(shkp), loss, currency(loss));
	    }
	}
}

STATIC_OVL int
kick_object(x, y)
xchar x, y;
{
	int range;
	register struct monst *mon, *shkp;
	struct trap *trap;
	char bhitroom;
	boolean costly, isgold, slide = FALSE;

	/* if a pile, the "top" object gets kicked */
	kickobj = level.objects[x][y];

	/* kickobj should always be set due to conditions of call */
	if(!kickobj || kickobj->otyp == BOULDER
			|| kickobj == uball || kickobj == uchain)
		return(0);

	if ((trap = t_at(x,y)) != 0 &&
			(((trap->ttyp == PIT ||
			   trap->ttyp == SPIKED_PIT) && !Passes_walls) ||
			 trap->ttyp == WEB)) {
		if (!trap->tseen) find_trap(trap);
#if 0 /*JP*/
		You_cant("kick %s that's in a %s!", something,
			 Hallucination ? "tizzy" :
			 (trap->ttyp == WEB) ? "web" : "pit");
#else
		You("%sでは%sを蹴ることができない！",
		    Hallucination ? "混乱した状態" :
		    trap->ttyp == WEB ? "くもの巣の中" : "落し穴の中",
		    something);
#endif
		return 1;
	}

	if(Fumbling && !rn2(3)) {
/*JP
		Your("clumsy kick missed.");
*/
		Your("不器用な蹴りは外れた。");
		return(1);
	}

	if(kickobj->otyp == CORPSE && touch_petrifies(&mons[kickobj->corpsenm])
			&& !Stone_resistance && !uarmf) {
	    char kbuf[BUFSZ];

#if 0 /*JP*/
	    You("kick the %s with your bare %s.",
		corpse_xname(kickobj, TRUE), makeplural(body_part(FOOT)));
#else
		You("素%sで%sを蹴った。",
		    makeplural(body_part(FOOT)), corpse_xname(kickobj, TRUE));
#endif
	    if (!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
/*JP
		You("turn to stone...");
*/
		You("石になりました…");
		killer_format = KILLED_BY;
		/* KMH -- otmp should be kickobj */
/*JP
		Sprintf(kbuf, "kicking %s without boots",
*/
		Sprintf(kbuf, "靴無しで%sを蹴って",
			an(corpse_xname(kickobj, TRUE)));
		killer = kbuf;
		done(STONING);
	    }
	}

	/* range < 2 means the object will not move.	*/
	/* maybe dexterity should also figure here.     */
	range = (int)((ACURRSTR)/2 - kickobj->owt/40);

	if(martial()) range += rnd(3);

	if (is_pool(x, y)) {
	    /* you're in the water too; significantly reduce range */
	    range = range / 3 + 1;	/* {1,2}=>1, {3,4,5}=>2, {6,7,8}=>3 */
	} else {
	    if (is_ice(x, y)) range += rnd(3),  slide = TRUE;
	    if (kickobj->greased) range += rnd(3),  slide = TRUE;
	}

	/* Mjollnir is magically too heavy to kick */
	if(kickobj->oartifact == ART_MJOLLNIR) range = 1;

	/* see if the object has a place to move into */
	if(!ZAP_POS(levl[x+u.dx][y+u.dy].typ) || closed_door(x+u.dx, y+u.dy))
		range = 1;

	costly = ((shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) &&
				    costly_spot(x, y));
	isgold = (kickobj->oclass == COIN_CLASS);

	if (IS_ROCK(levl[x][y].typ) || closed_door(x, y)) {
	    if ((!martial() && rn2(20) > ACURR(A_DEX)) ||
		    IS_ROCK(levl[u.ux][u.uy].typ) || closed_door(u.ux, u.uy)) {
		if (Blind)
/*JP
		    pline("It doesn't come loose.");
*/
		    pline("びくともしない。");
		else
#if 0 /*JP*/
		    pline("%s %sn't come loose.",
			  The(distant_name(kickobj, xname)),
			  otense(kickobj, "do"));
#else
		    pline("%sはびくともしない。",
				distant_name(kickobj, xname));
#endif
		return (!rn2(3) || martial());
	    }
	    if (Blind)
/*JP
		pline("It comes loose.");
*/
		pline("何かが緩んでとれた。");
	    else
#if 0 /*JP*/
		pline("%s %s loose.",
		      The(distant_name(kickobj, xname)),
		      otense(kickobj, "come"));
#else
		pline("%sは緩んでとれた。", distant_name(kickobj, xname));
#endif
	    obj_extract_self(kickobj);
	    newsym(x, y);
	    if (costly && (!costly_spot(u.ux, u.uy) ||
		    !index(u.urooms, *in_rooms(x, y, SHOPBASE))))
		addtobill(kickobj, FALSE, FALSE, FALSE);
/*JP
	    if (!flooreffects(kickobj, u.ux, u.uy, "fall")) {
*/
	    if (!flooreffects(kickobj, u.ux, u.uy, "落ちる")) {
		place_object(kickobj, u.ux, u.uy);
		stackobj(kickobj);
		newsym(u.ux, u.uy);
	    }
	    return 1;
	}

	/* a box gets a chance of breaking open here */
	if(Is_box(kickobj)) {
		boolean otrp = kickobj->otrapped;

/*JP
		if(range < 2) pline("THUD!");
*/
		if(range < 2) pline("ガン！");

		container_impact_dmg(kickobj);

		if (kickobj->olocked) {
		    if (!rn2(5) || (martial() && !rn2(2))) {
/*JP
			You("break open the lock!");
*/
			You("鍵を壊し開けた！");
			kickobj->olocked = 0;
			kickobj->obroken = 1;
			if (otrp) (void) chest_trap(kickobj, LEG, FALSE);
			return(1);
		    }
		} else {
		    if (!rn2(3) || (martial() && !rn2(2))) {
/*JP
			pline_The("lid slams open, then falls shut.");
*/
			pline("蓋がばたんと開き、閉じた。");
			if (otrp) (void) chest_trap(kickobj, LEG, FALSE);
			return(1);
		    }
		}
		if(range < 2) return(1);
		/* else let it fall through to the next cases... */
	}

	/* fragile objects should not be kicked */
	if (hero_breaks(kickobj, kickobj->ox, kickobj->oy, FALSE)) return 1;

	/* too heavy to move.  range is calculated as potential distance from
	 * player, so range == 2 means the object may move up to one square
	 * from its current position
	 */
	if(range < 2 || (isgold && kickobj->quan > 300L)) {
/*JP
	    if(!Is_box(kickobj)) pline("Thump!");
*/
	    if(!Is_box(kickobj)) pline("ゴツン！");
	    return(!rn2(3) || martial());
	}

	if (kickobj->quan > 1L && !isgold) kickobj = splitobj(kickobj, 1L);

	if (slide && !Blind)
#if 0 /*JP*/
	    pline("Whee!  %s %s across the %s.", Doname2(kickobj),
		  otense(kickobj, "slide"), surface(x,y));
#else
	    pline("いやっほう！%sは%sの上を滑っていった。", Doname2(kickobj),
		surface(x,y));
#endif

	obj_extract_self(kickobj);
	(void) snuff_candle(kickobj);
	newsym(x, y);
	mon = bhit(u.dx, u.dy, range, KICKED_WEAPON,
		   (int FDECL((*),(MONST_P,OBJ_P)))0,
		   (int FDECL((*),(OBJ_P,OBJ_P)))0,
		   kickobj);

	if(mon) {
	    if (mon->isshk &&
		    kickobj->where == OBJ_MINVENT && kickobj->ocarry == mon)
		return 1;	/* alert shk caught it */
	    notonhead = (mon->mx != bhitpos.x || mon->my != bhitpos.y);
	    if (isgold ? ghitm(mon, kickobj) :	/* caught? */
		    thitmonst(mon, kickobj))	/* hit && used up? */
		return(1);
	}

	/* the object might have fallen down a hole */
	if (kickobj->where == OBJ_MIGRATING) {
	    if (costly) {
		if(isgold)
		    costly_gold(x, y, kickobj->quan);
		else (void)stolen_value(kickobj, x, y,
					(boolean)shkp->mpeaceful, FALSE);
	    }
	    return 1;
	}

	bhitroom = *in_rooms(bhitpos.x, bhitpos.y, SHOPBASE);
	if (costly && (!costly_spot(bhitpos.x, bhitpos.y) ||
			*in_rooms(x, y, SHOPBASE) != bhitroom)) {
	    if(isgold)
		costly_gold(x, y, kickobj->quan);
	    else (void)stolen_value(kickobj, x, y,
				    (boolean)shkp->mpeaceful, FALSE);
	}

/*JP
	if(flooreffects(kickobj,bhitpos.x,bhitpos.y,"fall")) return(1);
*/
	if(flooreffects(kickobj,bhitpos.x,bhitpos.y,"落ちる")) return(1);
	place_object(kickobj, bhitpos.x, bhitpos.y);
	stackobj(kickobj);
	newsym(kickobj->ox, kickobj->oy);
	return(1);
}

STATIC_OVL char *
kickstr(buf)
char *buf;
{
	const char *what;

	if (kickobj) what = distant_name(kickobj,doname);
/*JP
	else if (IS_DOOR(maploc->typ)) what = "a door";
*/
	else if (IS_DOOR(maploc->typ)) what = "扉";
/*JP
	else if (IS_TREE(maploc->typ)) what = "a tree";
*/
	else if (IS_TREE(maploc->typ)) what = "木";
/*JP
	else if (IS_STWALL(maploc->typ)) what = "a wall";
*/
	else if (IS_STWALL(maploc->typ)) what = "壁";
/*JP
	else if (IS_ROCK(maploc->typ)) what = "a rock";
*/
	else if (IS_ROCK(maploc->typ)) what = "岩";
/*JP
	else if (IS_THRONE(maploc->typ)) what = "a throne";
*/
	else if (IS_THRONE(maploc->typ)) what = "玉座";
/*JP
	else if (IS_FOUNTAIN(maploc->typ)) what = "a fountain";
*/
	else if (IS_FOUNTAIN(maploc->typ)) what = "泉";
/*JP
	else if (IS_GRAVE(maploc->typ)) what = "a headstone";
*/
	else if (IS_GRAVE(maploc->typ)) what = "墓石";
#ifdef SINKS
/*JP
	else if (IS_SINK(maploc->typ)) what = "a sink";
*/
	else if (IS_SINK(maploc->typ)) what = "流し台";
#endif
/*JP
	else if (IS_ALTAR(maploc->typ)) what = "an altar";
*/
	else if (IS_ALTAR(maploc->typ)) what = "祭壇";
/*JP
	else if (IS_DRAWBRIDGE(maploc->typ)) what = "a drawbridge";
*/
	else if (IS_DRAWBRIDGE(maploc->typ)) what = "跳ね橋";
/*JP
	else if (maploc->typ == STAIRS) what = "the stairs";
*/
	else if (maploc->typ == STAIRS) what = "階段";
/*JP
	else if (maploc->typ == LADDER) what = "a ladder";
*/
	else if (maploc->typ == LADDER) what = "はしご";
/*JP
	else if (maploc->typ == IRONBARS) what = "an iron bar";
*/
	else if (maploc->typ == IRONBARS) what = "鉄棒";
/*JP
	else what = "something weird";
*/
	else what = "何か妙なもの";
#if 0 /*JP:T*/
	return strcat(strcpy(buf, "kicking "), what);
#else
	Sprintf(buf, "%sを蹴って", what);
	return buf;
#endif
}

int
dokick()
{
	int x, y;
	int avrg_attrib;
	register struct monst *mtmp;
	boolean no_kick = FALSE;
	char buf[BUFSZ];

	if (nolimbs(youmonst.data) || slithy(youmonst.data)) {
/*JP
		You("have no legs to kick with.");
*/
		You("何かを蹴ろうにも足がない。");
		no_kick = TRUE;
	} else if (verysmall(youmonst.data)) {
/*JP
		You("are too small to do any kicking.");
*/
		You("何かを蹴るには小さすぎる。");
		no_kick = TRUE;
#ifdef STEED
	} else if (u.usteed) {
/*JP
		if (yn_function("Kick your steed?", ynchars, 'y') == 'y') {
*/
		if (yn_function("馬を蹴る？", ynchars, 'y') == 'y') {
/*JP
		    You("kick %s.", mon_nam(u.usteed));
*/
		You("%sを蹴った。", mon_nam(u.usteed));
		    kick_steed();
		    return 1;
		} else {
		    return 0;
		}
#endif
	} else if (Wounded_legs) {
		/* note: jump() has similar code */
		long wl = (EWounded_legs & BOTH_SIDES);
		const char *bp = body_part(LEG);

		if (wl == BOTH_SIDES) bp = makeplural(bp);
#if 0 /*JP*/
		Your("%s%s %s in no shape for kicking.",
		     (wl == LEFT_SIDE) ? "left " :
			(wl == RIGHT_SIDE) ? "right " : "",
		     bp, (wl == BOTH_SIDES) ? "are" : "is");
#else
		You("%s%sを怪我をしており蹴れない。",
		     (wl == LEFT_SIDE) ? "左" :
			(wl == RIGHT_SIDE) ? "右" : "",
		     bp);
#endif
		no_kick = TRUE;
	} else if (near_capacity() > SLT_ENCUMBER) {
/*JP
		Your("load is too heavy to balance yourself for a kick.");
*/
		You("たくさんものを持ちすぎて蹴りのためのバランスがとれない。");
		no_kick = TRUE;
	} else if (youmonst.data->mlet == S_LIZARD) {
/*JP
		Your("legs cannot kick effectively.");
*/
		Your("足ではうまく蹴れない。");
		no_kick = TRUE;
	} else if (u.uinwater && !rn2(2)) {
/*JP
		Your("slow motion kick doesn't hit anything.");
*/
		Your("遅い動きの蹴りでは命中しようがない。");
		no_kick = TRUE;
	} else if (u.utrap) {
		switch (u.utraptype) {
		    case TT_PIT:
/*JP
			pline("There's not enough room to kick down here.");
*/
			pline("落し穴にはまっているので、蹴れない。");
			break;
		    case TT_WEB:
		    case TT_BEARTRAP:
/*JP
			You_cant("move your %s!", body_part(LEG));
*/
			You("%sを動かすことができない！", body_part(LEG));
			break;
		    default:
			break;
		}
		no_kick = TRUE;
	}

	if (no_kick) {
		/* ignore direction typed before player notices kick failed */
		display_nhwindow(WIN_MESSAGE, TRUE);	/* --More-- */
		return 0;
	}

	if(!getdir((char *)0)) return(0);
	if(!u.dx && !u.dy) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;

	/* KMH -- Kicking boots always succeed */
	if (uarmf && uarmf->otyp == KICKING_BOOTS)
	    avrg_attrib = 99;
	else
	    avrg_attrib = (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3;

	if(u.uswallow) {
		switch(rn2(3)) {
/*JP
		case 0:  You_cant("move your %s!", body_part(LEG));
*/
		case 0:  You("%sを動かすことができない！", body_part(LEG));
			 break;
		case 1:  if (is_animal(u.ustuck->data)) {
/*JP
				pline("%s burps loudly.", Monnam(u.ustuck));
*/
				pline("%sは大きなゲップをした。", Monnam(u.ustuck));
				break;
			 }
/*JP
		default: Your("feeble kick has no effect."); break;
*/
		default: Your("弱々しい蹴りは効果がない。"); break;
		}
		return(1);
	}
	if (Levitation) {
		int xx, yy;

		xx = u.ux - u.dx;
		yy = u.uy - u.dy;
		/* doors can be opened while levitating, so they must be
		 * reachable for bracing purposes
		 * Possible extension: allow bracing against stuff on the side?
		 */
		if (isok(xx,yy) && !IS_ROCK(levl[xx][yy].typ) &&
			!IS_DOOR(levl[xx][yy].typ) &&
			(!Is_airlevel(&u.uz) || !OBJ_AT(xx,yy))) {
/*JP
		    You("have nothing to brace yourself against.");
*/
		    pline("支えにできるようなものが無い。");
		    return(0);
		}
	}

	wake_nearby();
	u_wipe_engr(2);

	maploc = &levl[x][y];

	/* The next five tests should stay in    */
	/* their present order: monsters, pools, */
	/* objects, non-doors, doors.		 */

	if(MON_AT(x, y)) {
		struct permonst *mdat;

		mtmp = m_at(x, y);
		mdat = mtmp->data;
		if (!mtmp->mpeaceful || !canspotmon(mtmp))
		    flags.forcefight = TRUE; /* attack even if invisible */
		kick_monster(x, y);
		flags.forcefight = FALSE;
		/* see comment in attack_checks() */
		if (!DEADMONSTER(mtmp) &&
		    !canspotmon(mtmp) &&
		    /* check x and y; a monster that evades your kick by
		       jumping to an unseen square doesn't leave an I behind */
		    mtmp->mx == x && mtmp->my == y &&
		    !glyph_is_invisible(levl[x][y].glyph) &&
		    !(u.uswallow && mtmp == u.ustuck))
			map_invisible(x, y);
		if((Is_airlevel(&u.uz) || Levitation) && flags.move) {
		    int range;

		    range = ((int)youmonst.data->cwt + (weight_cap() + inv_weight()));
		    if (range < 1) range = 1; /* divide by zero avoidance */
		    range = (3*(int)mdat->cwt) / range;

		    if(range < 1) range = 1;
		    hurtle(-u.dx, -u.dy, range, TRUE);
		}
		return(1);
	}
	if (glyph_is_invisible(levl[x][y].glyph)) {
		unmap_object(x, y);
		newsym(x, y);
	}
	if (is_pool(x, y) ^ !!u.uinwater) {
		/* objects normally can't be removed from water by kicking */
/*JP
		You("splash some water around.");
*/
		You("水を回りにまきちらした。");
		return 1;
	}

	kickobj = (struct obj *)0;
	if (OBJ_AT(x, y) &&
	    (!Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)
	     || sobj_at(BOULDER,x,y))) {
		if(kick_object(x, y)) {
		    if(Is_airlevel(&u.uz))
			hurtle(-u.dx, -u.dy, 1, TRUE); /* assume it's light */
		    return(1);
		}
		goto ouch;
	}

	if(!IS_DOOR(maploc->typ)) {
		if(maploc->typ == SDOOR) {
		    if(!Levitation && rn2(30) < avrg_attrib) {
			cvt_sdoor_to_door(maploc);	/* ->typ = DOOR */
/*JP
			pline("Crash!  %s a secret door!",
*/
			pline("ガシャン！あなたは秘密の扉を%s！",
			      /* don't "kick open" when it's locked
				 unless it also happens to be trapped */
			(maploc->doormask & (D_LOCKED|D_TRAPPED)) == D_LOCKED ?
/*JP
			      "Your kick uncovers" : "You kick open");
*/
			      "発見した" : "蹴り開けた");
			exercise(A_DEX, TRUE);
			if(maploc->doormask & D_TRAPPED) {
			    maploc->doormask = D_NODOOR;
/*JP
			    b_trapped("door", FOOT);
*/
			    b_trapped("扉", FOOT);
			} else if (maploc->doormask != D_NODOOR &&
				   !(maploc->doormask & D_LOCKED))
			    maploc->doormask = D_ISOPEN;
			if (Blind)
			    feel_location(x,y);	/* we know it's gone */
			else
			    newsym(x,y);
			if (maploc->doormask == D_ISOPEN ||
			    maploc->doormask == D_NODOOR)
			    unblock_point(x,y);	/* vision */
			return(1);
		    } else goto ouch;
		}
		if(maploc->typ == SCORR) {
		    if(!Levitation && rn2(30) < avrg_attrib) {
/*JP
			pline("Crash!  You kick open a secret passage!");
*/
			pline("ガシャン！あなたは隠し通路を蹴り開けた！");
			exercise(A_DEX, TRUE);
			maploc->typ = CORR;
			if (Blind)
			    feel_location(x,y);	/* we know it's gone */
			else
			    newsym(x,y);
			unblock_point(x,y);	/* vision */
			return(1);
		    } else goto ouch;
		}
		if(IS_THRONE(maploc->typ)) {
		    register int i;
		    if(Levitation) goto dumb;
		    if((Luck < 0 || maploc->doormask) && !rn2(3)) {
			maploc->typ = ROOM;
			maploc->doormask = 0; /* don't leave loose ends.. */
			(void) mkgold((long)rnd(200), x, y);
			if (Blind)
/*JP
			    pline("CRASH!  You destroy it.");
*/
			    pline("ガシャン！あなたは何かを破壊した。");
			else {
/*JP
			    pline("CRASH!  You destroy the throne.");
*/
			    pline("ガシャン！あなたは玉座を破壊した。");
			    newsym(x, y);
			}
			exercise(A_DEX, TRUE);
			return(1);
		    } else if(Luck > 0 && !rn2(3) && !maploc->looted) {
			(void) mkgold((long) rn1(201, 300), x, y);
			i = Luck + 1;
			if(i > 6) i = 6;
			while(i--)
			    (void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL,
					LUCKSTONE-1), x, y, FALSE, TRUE);
			if (Blind)
/*JP
			    You("kick %s loose!", something);
*/
			    Your("蹴りでなにかが外れた！");
			else {
/*JP
			    You("kick loose some ornamental coins and gems!");
*/
			    Your("蹴りで貴金属や宝石がいくつか外れた！");
			    newsym(x, y);
			}
			/* prevent endless milking */
			maploc->looted = T_LOOTED;
			return(1);
		    } else if (!rn2(4)) {
			if(dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)) {
			    fall_through(FALSE);
			    return(1);
			} else goto ouch;
		    }
		    goto ouch;
		}
		if(IS_ALTAR(maploc->typ)) {
		    if(Levitation) goto dumb;
/*JP
		    You("kick %s.",(Blind ? something : "the altar"));
*/
		    You("%sを蹴った。",(Blind ? "何か" : "祭壇"));
		    if(!rn2(3)) goto ouch;
		    altar_wrath(x, y);
		    exercise(A_DEX, TRUE);
		    return(1);
		}
		if(IS_FOUNTAIN(maploc->typ)) {
		    if(Levitation) goto dumb;
/*JP
		    You("kick %s.",(Blind ? something : "the fountain"));
*/
		    You("%sを蹴った。",(Blind ? "何か" : "泉"));
		    if(!rn2(3)) goto ouch;
		    /* make metal boots rust */
		    if(uarmf && rn2(3))
/*JP
			if (!rust_dmg(uarmf, "metal boots", 1, FALSE, &youmonst)) {
*/
			if (!rust_dmg(uarmf, "金属の靴", 1, FALSE, &youmonst)) {
/*JP
				Your("boots get wet.");
*/
				Your("靴は濡れた。");
				/* could cause short-lived fumbling here */
			}
		    exercise(A_DEX, TRUE);
		    return(1);
		}
		if(IS_GRAVE(maploc->typ) || maploc->typ == IRONBARS)
		    goto ouch;
		if(IS_TREE(maploc->typ)) {
		    struct obj *treefruit;
		    /* nothing, fruit or trouble? 75:23.5:1.5% */
		    if (rn2(3)) {
			if ( !rn2(6) && !(mvitals[PM_KILLER_BEE].mvflags & G_GONE) )
#if 0 /*JP*/
			    You_hear("a low buzzing."); /* a warning */
#else
			    You_hear("低い羽音のうなりを聞いた。"); /* a warning */
#endif
			goto ouch;
		    }
		    if (rn2(15) && !(maploc->looted & TREE_LOOTED) &&
			  (treefruit = rnd_treefruit_at(x, y))) {
			long nfruit = 8L-rnl(7), nfall;
			short frtype = treefruit->otyp;
			treefruit->quan = nfruit;
			if (is_plural(treefruit))
/*JP
			    pline("Some %s fall from the tree!", xname(treefruit));
*/
				pline("%sが何個か木から落ちてきた！", xname(treefruit));
			else
/*JP
			    pline("%s falls from the tree!", An(xname(treefruit)));
*/
				pline("%sが木から落ちてきた！", An(xname(treefruit)));
			nfall = scatter(x,y,2,MAY_HIT,treefruit);
			if (nfall != nfruit) {
			    /* scatter left some in the tree, but treefruit
			     * may not refer to the correct object */
			    treefruit = mksobj(frtype, TRUE, FALSE);
			    treefruit->quan = nfruit-nfall;
#if 0 /*JP*/
			    pline("%ld %s got caught in the branches.",
				nfruit-nfall, xname(treefruit));
#else
			    pline("%ld個の%sが枝にひっかかった。",
				nfruit-nfall, xname(treefruit));
#endif
			    dealloc_obj(treefruit);
			}
			exercise(A_DEX, TRUE);
			exercise(A_WIS, TRUE);	/* discovered a new food source! */
			newsym(x, y);
			maploc->looted |= TREE_LOOTED;
			return(1);
		    } else if (!(maploc->looted & TREE_SWARM)) {
		    	int cnt = rnl(4) + 2;
			int made = 0;
		    	coord mm;
		    	mm.x = x; mm.y = y;
			while (cnt--) {
			    if (enexto(&mm, mm.x, mm.y, &mons[PM_KILLER_BEE])
				&& makemon(&mons[PM_KILLER_BEE],
					       mm.x, mm.y, MM_ANGRY))
				made++;
			}
			if ( made )
/*JP
			    pline("You've attracted the tree's former occupants!");
*/
			pline("木の先住民を呼びよせてしまった！");
			else
/*JP
			    You("smell stale honey.");
*/
			    pline("古いはちみつのにおいがした。");
			maploc->looted |= TREE_SWARM;
			return(1);
		    }
		    goto ouch;
		}
#ifdef SINKS
		if(IS_SINK(maploc->typ)) {
/*
		    int gend = poly_gender();
		    short washerndx = (gend == 1 || (gend == 2 && rn2(2))) ?
					PM_INCUBUS : PM_SUCCUBUS;
		    short washerndx = PM_MAID;
*/

		    if(Levitation) goto dumb;
		    if(rn2(5)) {
			if(flags.soundok)
/*JP
			    pline("Klunk!  The pipes vibrate noisily.");
*/
			    pline("ガタン！パイプはうるさく振動した。");
/*JP
			else pline("Klunk!");
*/
			else pline("ガタン！");
			exercise(A_DEX, TRUE);
			return(1);
		    } else if(!(maploc->looted & S_LPUDDING) && !rn2(3) &&
			  !(mvitals[PM_BLACK_PUDDING].mvflags & G_GONE)) {
			if (Blind)
/*JP
			    You_hear("a gushing sound.");
*/
			    You_hear("なにかが噴出する音を聞いた。");
			else
/*JP
			    pline("A %s ooze gushes up from the drain!",
*/
			    pline("%s液体が排水口からにじみ出た！",
					 hcolor(NH_BLACK));
			(void) makemon(&mons[PM_BLACK_PUDDING],
					 x, y, NO_MM_FLAGS);
			exercise(A_DEX, TRUE);
			newsym(x,y);
			maploc->looted |= S_LPUDDING;
			return(1);
		    } else if(!(maploc->looted & S_LDWASHER) && !rn2(3) &&
			      !(mvitals[PM_MAID].mvflags & G_GONE)) {
			/* can't resist... */
#if 0 /*JP*/
			pline("%s returns!", (Blind ? Something :
							"The dish washer"));
#else
			pline("%sが戻ってきた！", (Blind ? "何か" :
							"皿洗い"));
#endif
			(void) makemon(&mons[PM_MAID],
					 x, y, NO_MM_FLAGS);
/*			if (makemon(&mons[washerndx], x, y, NO_MM_FLAGS))
			    newsym(x,y);*/
			maploc->looted |= S_LDWASHER;
			exercise(A_DEX, TRUE);
			return(1);
		    } else if(!rn2(3)) {
#if 0 /*JP*/
			pline("Flupp!  %s.", (Blind ?
				      "You hear a sloshing sound" :
				      "Muddy waste pops up from the drain"));
#else
			pline("うわ！%s。", (Blind ?
				      "あなたは、水をはねとばす音を聞いた" :
				      "排水口からどろどろの廃棄物が出てくる"));
#endif
			if(!(maploc->looted & S_LRING)) { /* once per sink */
			    if (!Blind)
/*JP
				You("see a ring shining in its midst.");
*/
				You("その中に光る指輪を見つけた。");
			    (void) mkobj_at(RING_CLASS, x, y, TRUE);
			    newsym(x, y);
			    exercise(A_DEX, TRUE);
			    exercise(A_WIS, TRUE);	/* a discovery! */
			    maploc->looted |= S_LRING;
			}
			return(1);
		    }
		    goto ouch;
		}
#endif
		if (maploc->typ == STAIRS || maploc->typ == LADDER ||
						    IS_STWALL(maploc->typ)) {
		    if(!IS_STWALL(maploc->typ) && maploc->ladder == LA_DOWN)
			goto dumb;
ouch:
/*JP
		    pline("Ouch!  That hurts!");
*/
		    pline("いてっ！怪我した！");
		    exercise(A_DEX, FALSE);
		    exercise(A_STR, FALSE);
		    if (Blind) feel_location(x,y); /* we know we hit it */
		    if (is_drawbridge_wall(x,y) >= 0) {
/*JP
			pline_The("drawbridge is unaffected.");
*/
		    pline("跳ね橋はびくともしない。");
			/* update maploc to refer to the drawbridge */
			(void) find_drawbridge(&x,&y);
			maploc = &levl[x][y];
		    }
		    if(!rn2(3)) set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
		    losehp(rnd(ACURR(A_CON) > 15 ? 3 : 5), kickstr(buf),
			KILLED_BY);
		    if(Is_airlevel(&u.uz) || Levitation)
			hurtle(-u.dx, -u.dy, rn1(2,4), TRUE); /* assume it's heavy */
		    return(1);
		}
		goto dumb;
	}

	if(maploc->doormask == D_ISOPEN ||
	   maploc->doormask == D_BROKEN ||
	   maploc->doormask == D_NODOOR) {
dumb:
		exercise(A_DEX, FALSE);
		if (martial() || ACURR(A_DEX) >= 16 || rn2(3)) {
/*JP
			You("kick at empty space.");
*/
			You("何もない空間を蹴った。");
			if (Blind) feel_location(x,y);
		} else {
/*JP
			pline("Dumb move!  You strain a muscle.");
*/
			pline("ばかげた行為だ！足を痛めた。");
			exercise(A_STR, FALSE);
			set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
		}
		if ((Is_airlevel(&u.uz) || Levitation) && rn2(2)) {
		    hurtle(-u.dx, -u.dy, 1, TRUE);
		    return 1;		/* you moved, so use up a turn */
		}
		return(0);
	}

	/* not enough leverage to kick open doors while levitating */
	if(Levitation) goto ouch;

	exercise(A_DEX, TRUE);
	/* door is known to be CLOSED or LOCKED */
	if(rnl(35) < avrg_attrib + (!martial() ? 0 : ACURR(A_DEX))) {
		boolean shopdoor = *in_rooms(x, y, SHOPBASE) ? TRUE : FALSE;
		/* break the door */
		if(maploc->doormask & D_TRAPPED) {
/*JP
		    if (flags.verbose) You("kick the door.");
*/
		    if (flags.verbose) You("扉を蹴った。");
		    exercise(A_STR, FALSE);
		    maploc->doormask = D_NODOOR;
/*JP
		    b_trapped("door", FOOT);
*/
		    b_trapped("扉", FOOT);
		} else if(ACURR(A_STR) > 18 && !rn2(5) && !shopdoor) {
/*JP
		    pline("As you kick the door, it shatters to pieces!");
*/
		    pline("扉を蹴ると、こなごなにくだけた！");
		    exercise(A_STR, TRUE);
		    maploc->doormask = D_NODOOR;
		} else {
/*JP
		    pline("As you kick the door, it crashes open!");
*/
		    pline("扉を蹴ると、壊れて開いた！");
		    exercise(A_STR, TRUE);
		    maploc->doormask = D_BROKEN;
		}
		if (Blind)
		    feel_location(x,y);		/* we know we broke it */
		else
		    newsym(x,y);
		unblock_point(x,y);		/* vision */
		if (shopdoor) {
		    add_damage(x, y, 400L);
/*JP
		    pay_for_damage("break", FALSE);
*/
		    pay_for_damage("破壊する", FALSE);
		}
		if (in_town(x, y))
		  for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if (DEADMONSTER(mtmp)) continue;
		    if((mtmp->data == &mons[PM_WATCHMAN] ||
			mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			couldsee(mtmp->mx, mtmp->my) &&
			mtmp->mpeaceful) {
			if (canspotmon(mtmp))
/*JP
			    pline("%s yells:", Amonnam(mtmp));
*/
			    pline("%sは叫んだ：", Amonnam(mtmp));
			else
/*JP
			    You_hear("someone yell:");
*/
			    You_hear("誰かが叫んでいるのを聞いた：");
/*JP
			verbalize("Halt, thief!  You're under arrest!");
*/
			verbalize("止まれ、泥棒！おまえを逮捕する！");
			(void) angry_guards(FALSE);
			break;
		    }
		  }
	} else {
	    if (Blind) feel_location(x,y);	/* we know we hit it */
	    exercise(A_STR, TRUE);
/*JP
	    pline("WHAMMM!!!");
*/
	    pline("ぐぁぁぁん！");
	    if (in_town(x, y))
		for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if (DEADMONSTER(mtmp)) continue;
		    if ((mtmp->data == &mons[PM_WATCHMAN] ||
				mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			    mtmp->mpeaceful && couldsee(mtmp->mx, mtmp->my)) {
			if (canspotmon(mtmp))
/*JP
			    pline("%s yells:", Amonnam(mtmp));
*/
			    pline("%sは叫んだ：", Amonnam(mtmp));
			else
/*JP
			    You_hear("someone yell:");
*/
			    You_hear("誰かが叫んでいるのを聞いた：");
			if(levl[x][y].looted & D_WARNED) {
/*JP
			    verbalize("Halt, vandal!  You're under arrest!");
*/
			    verbalize("止まれ！器物損壊でおまえを逮捕する！");
			    (void) angry_guards(FALSE);
			} else {
/*JP
			    verbalize("Hey, stop damaging that door!");
*/
			    verbalize("おい、扉を傷つけるんじゃない！");
			    levl[x][y].looted |= D_WARNED;
			}
			break;
		    }
		}
	}
	return(1);
}

STATIC_OVL void
drop_to(cc, loc)
coord *cc;
schar loc;
{
	/* cover all the MIGR_xxx choices generated by down_gate() */
	switch (loc) {
	 case MIGR_RANDOM:	/* trap door or hole */
		    if (Is_stronghold(&u.uz)) {
			cc->x = valley_level.dnum;
			cc->y = valley_level.dlevel;
			break;
		    } else if (In_endgame(&u.uz) || Is_botlevel(&u.uz)) {
			cc->y = cc->x = 0;
			break;
		    } /* else fall to the next cases */
	 case MIGR_STAIRS_UP:
	 case MIGR_LADDER_UP:
		    cc->x = u.uz.dnum;
		    cc->y = u.uz.dlevel + 1;
		    break;
	 case MIGR_SSTAIRS:
		    cc->x = sstairs.tolev.dnum;
		    cc->y = sstairs.tolev.dlevel;
		    break;
	 default:
	 case MIGR_NOWHERE:
		    /* y==0 means "nowhere", in which case x doesn't matter */
		    cc->y = cc->x = 0;
		    break;
	}
}

void
impact_drop(missile, x, y, dlev)
struct obj *missile;
xchar x, y, dlev;
{
	schar toloc;
	register struct obj *obj, *obj2;
	register struct monst *shkp;
	long oct, dct, price, debit, robbed;
	boolean angry, costly, isrock;
	coord cc;

	if(!OBJ_AT(x, y)) return;

	toloc = down_gate(x, y);
	drop_to(&cc, toloc);
	if (!cc.y) return;

	if (dlev) {
		/* send objects next to player falling through trap door.
		 * checked in obj_delivery().
		 */
		toloc = MIGR_NEAR_PLAYER;
		cc.y = dlev;
	}

	costly = costly_spot(x, y);
	price = debit = robbed = 0L;
	angry = FALSE;
	shkp = (struct monst *) 0;
	/* if 'costly', we must keep a record of ESHK(shkp) before
	 * it undergoes changes through the calls to stolen_value.
	 * the angry bit must be reset, if needed, in this fn, since
	 * stolen_value is called under the 'silent' flag to avoid
	 * unsavory pline repetitions.
	 */
	if(costly) {
	    if ((shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) != 0) {
		debit	= ESHK(shkp)->debit;
		robbed	= ESHK(shkp)->robbed;
		angry	= !shkp->mpeaceful;
	    }
	}

	isrock = (missile && missile->otyp == ROCK);
	oct = dct = 0L;
	for(obj = level.objects[x][y]; obj; obj = obj2) {
		obj2 = obj->nexthere;
		if(obj == missile) continue;
		/* number of objects in the pile */
		oct += obj->quan;
		if(obj == uball || obj == uchain) continue;
		/* boulders can fall too, but rarely & never due to rocks */
		if((isrock && obj->otyp == BOULDER) ||
		   rn2(obj->otyp == BOULDER ? 30 : 3)) continue;
		obj_extract_self(obj);

		if(costly) {
		    price += stolen_value(obj, x, y,
				(costly_spot(u.ux, u.uy) &&
				 index(u.urooms, *in_rooms(x, y, SHOPBASE))),
				TRUE);
		    /* set obj->no_charge to 0 */
		    if (Has_contents(obj))
			picked_container(obj);	/* does the right thing */
		    if (obj->oclass != COIN_CLASS)
			obj->no_charge = 0;
		}

		add_to_migration(obj);
		obj->ox = cc.x;
		obj->oy = cc.y;
		obj->owornmask = (long)toloc;

		/* number of fallen objects */
		dct += obj->quan;
	}

	if (dct && cansee(x,y)) {	/* at least one object fell */
/*JP
	    const char *what = (dct == 1L ? "object falls" : "objects fall");
*/
	    const char *what = "物";

	    if (missile)
#if 0 /*JP*/
		pline("From the impact, %sother %s.",
		      dct == oct ? "the " : dct == 1L ? "an" : "", what);
#else
		pline("衝撃で、他の%sが落ちた。",what);
#endif
	    else if (oct == dct)
#if 0 /*JP*/
		pline("%s adjacent %s %s.",
		      dct == 1L ? "The" : "All the", what, gate_str);
#else
		pline("近くにあった%sが%s落ちた。",
		      what, gate_str);
#endif
	    else
#if 0 /*JP*/
		pline("%s adjacent %s %s.",
		      dct == 1L ? "One of the" : "Some of the",
		      dct == 1L ? "objects falls" : what, gate_str);
#else
		pline("近くにあった%s%s%s落ちた。",
		      what,
		      dct == 1L ? "が" : "のいくつかが",
		      gate_str);
#endif
	}

	if(costly && shkp && price) {
		if(ESHK(shkp)->robbed > robbed) {
/*JP
		    You("removed %ld %s worth of goods!", price, currency(price));
*/
		    You("%ld%s分の品物を投棄してしまった！", price, currency(price));
		    if(cansee(shkp->mx, shkp->my)) {
			if(ESHK(shkp)->customer[0] == 0)
			    (void) strncpy(ESHK(shkp)->customer,
					   plname, PL_NSIZ);
			if(angry)
/*JP
			    pline("%s is infuriated!", Monnam(shkp));
*/
			    pline("%sは激怒した！", Monnam(shkp));
/*JP
			else pline("\"%s, you are a thief!\"", plname);
*/
			else pline("「%s、この泥棒め！」", plname);
/*JP
		    } else  You_hear("a scream, \"Thief!\"");
*/
		    } else You_hear("「泥棒！」という金切声を聞いた。");
		    hot_pursuit(shkp);
		    (void) angry_guards(FALSE);
		    return;
		}
		if(ESHK(shkp)->debit > debit) {
		    long amt = (ESHK(shkp)->debit - debit);
#if 0 /*JP*/
		    You("owe %s %ld %s for goods lost.",
#else
		    You("品物消失のため%sに%ld%sの借りをつくった。",
#endif
			Monnam(shkp),
			amt, currency(amt));
		}
	}

}

/* NOTE: ship_object assumes otmp was FREED from fobj or invent.
 * <x,y> is the point of drop.  otmp is _not_ an <x,y> resident:
 * otmp is either a kicked, dropped, or thrown object.
 */
boolean
ship_object(otmp, x, y, shop_floor_obj)
xchar  x, y;
struct obj *otmp;
boolean shop_floor_obj;
{
	schar toloc;
	xchar ox, oy;
	coord cc;
	struct obj *obj;
	struct trap *t;
	boolean nodrop, unpaid, container, impact = FALSE;
	long n = 0L;

	if (!otmp) return(FALSE);
	if ((toloc = down_gate(x, y)) == MIGR_NOWHERE) return(FALSE);
	drop_to(&cc, toloc);
	if (!cc.y) return(FALSE);

	/* objects other than attached iron ball always fall down ladder,
	   but have a chance of staying otherwise */
	nodrop = (otmp == uball) || (otmp == uchain) ||
		(toloc != MIGR_LADDER_UP && rn2(3));

	container = Has_contents(otmp);
	unpaid = (otmp->unpaid || (container && count_unpaid(otmp->cobj)));

	if(OBJ_AT(x, y)) {
	    for(obj = level.objects[x][y]; obj; obj = obj->nexthere)
		if(obj != otmp) n += obj->quan;
	    if(n) impact = TRUE;
	}
	/* boulders never fall through trap doors, but they might knock
	   other things down before plugging the hole */
	if (otmp->otyp == BOULDER &&
		((t = t_at(x, y)) != 0) &&
		(t->ttyp == TRAPDOOR || t->ttyp == HOLE)) {
	    if (impact) impact_drop(otmp, x, y, 0);
	    return FALSE;		/* let caller finish the drop */
	}

	if (cansee(x, y))
	    otransit_msg(otmp, nodrop, n);

	if (nodrop) {
	    if (impact) impact_drop(otmp, x, y, 0);
	    return(FALSE);
	}

	if(unpaid || shop_floor_obj) {
	    if(unpaid) {
		subfrombill(otmp, shop_keeper(*u.ushops));
		(void)stolen_value(otmp, u.ux, u.uy, TRUE, FALSE);
	    } else {
		ox = otmp->ox;
		oy = otmp->oy;
		(void)stolen_value(otmp, ox, oy,
			  (costly_spot(u.ux, u.uy) &&
			      index(u.urooms, *in_rooms(ox, oy, SHOPBASE))),
			  FALSE);
	    }
	    /* set otmp->no_charge to 0 */
	    if(container)
		picked_container(otmp); /* happens to do the right thing */
	    if(otmp->oclass != COIN_CLASS)
		otmp->no_charge = 0;
	}

	if (otmp == uwep) setuwep((struct obj *)0);
	if (otmp == uquiver) setuqwep((struct obj *)0);
	if (otmp == uswapwep) setuswapwep((struct obj *)0);

	/* some things break rather than ship */
	if (breaktest(otmp)) {
	    const char *result;

	    if (objects[otmp->otyp].oc_material == GLASS
#ifdef TOURIST
		|| otmp->otyp == EXPENSIVE_CAMERA
#endif
		) {
		if (otmp->otyp == MIRROR)
		    change_luck(-2);
/*JP
		result = "crash";
*/
		result = "グシャッ";
	    } else {
		/* penalty for breaking eggs laid by you */
		if (otmp->otyp == EGG && otmp->spe && otmp->corpsenm >= LOW_PM)
		    change_luck((schar) -min(otmp->quan, 5L));
/*JP
		result = "splat";
*/
		result = "ベチャッ";
	    }
/*JP
	    You_hear("a muffled %s.",result);
*/
				You_hear("こもった%sという音を聞いた。", result);
	    obj_extract_self(otmp);
	    obfree(otmp, (struct obj *) 0);
	    return TRUE;
	}

	add_to_migration(otmp);
	otmp->ox = cc.x;
	otmp->oy = cc.y;
	otmp->owornmask = (long)toloc;
	/* boulder from rolling boulder trap, no longer part of the trap */
	if (otmp->otyp == BOULDER) otmp->otrapped = 0;

	if(impact) {
	    /* the objs impacted may be in a shop other than
	     * the one in which the hero is located.  another
	     * check for a shk is made in impact_drop.  it is, e.g.,
	     * possible to kick/throw an object belonging to one
	     * shop into another shop through a gap in the wall,
	     * and cause objects belonging to the other shop to
	     * fall down a trap door--thereby getting two shopkeepers
	     * angry at the hero in one shot.
	     */
	    impact_drop(otmp, x, y, 0);
	    newsym(x,y);
	}
	return(TRUE);
}

void
obj_delivery()
{
	register struct obj *otmp, *otmp2;
	register int nx, ny;
	long where;

	for (otmp = migrating_objs; otmp; otmp = otmp2) {
	    otmp2 = otmp->nobj;
	    if (otmp->ox != u.uz.dnum || otmp->oy != u.uz.dlevel) continue;

	    obj_extract_self(otmp);
	    where = otmp->owornmask;		/* destination code */
	    otmp->owornmask = 0L;

	    switch ((int)where) {
	     case MIGR_STAIRS_UP:   nx = xupstair,  ny = yupstair;
				break;
	     case MIGR_LADDER_UP:   nx = xupladder,  ny = yupladder;
				break;
	     case MIGR_SSTAIRS:	    nx = sstairs.sx,  ny = sstairs.sy;
				break;
	     case MIGR_NEAR_PLAYER: nx = u.ux,  ny = u.uy;
				break;
	     default:
	     case MIGR_RANDOM:	    nx = ny = 0;
				break;
	    }
	    if (nx > 0) {
		place_object(otmp, nx, ny);
		stackobj(otmp);
		(void)scatter(nx, ny, rnd(2), 0, otmp);
	    } else {		/* random location */
		/* set dummy coordinates because there's no
		   current position for rloco() to update */
		otmp->ox = otmp->oy = 0;
		rloco(otmp);
	    }
	}
}

STATIC_OVL void
otransit_msg(otmp, nodrop, num)
register struct obj *otmp;
register boolean nodrop;
long num;
{
	char obuf[BUFSZ];

#if 0 /*JP*/
	Sprintf(obuf, "%s%s",
		 (otmp->otyp == CORPSE &&
			type_is_pname(&mons[otmp->corpsenm])) ? "" : "The ",
		 xname(otmp));
#else
	Sprintf(obuf, "%sは", xname(otmp));
#endif

	if(num) { /* means: other objects are impacted */
#if 0 /*JP*/
	    Sprintf(eos(obuf), " %s %s object%s",
		    otense(otmp, "hit"),
		    num == 1L ? "another" : "other",
		    num > 1L ? "s" : "");
#else
	    Sprintf(eos(obuf), "他の物体に命中して");
#endif
	    if(nodrop)
/*JP
		Sprintf(eos(obuf), ".");
*/
		Sprintf(eos(obuf), "止まった。");
	    else
#if 0 /*JP*/
		Sprintf(eos(obuf), " and %s %s.",
			otense(otmp, "fall"), gate_str);
#else
		Sprintf(eos(obuf), "%s落ちた。", gate_str);
#endif
	    pline("%s", obuf);
	} else if(!nodrop)
/*JP
	    pline("%s %s %s.", obuf, otense(otmp, "fall"), gate_str);
*/
	    pline("%s%s落ちた。", obuf, gate_str);
}

/* migration destination for objects which fall down to next level */
schar
down_gate(x, y)
xchar x, y;
{
	struct trap *ttmp;

	gate_str = 0;
	/* this matches the player restriction in goto_level() */
	if (on_level(&u.uz, &qstart_level) && !ok_to_quest())
	    return MIGR_NOWHERE;

	if ((xdnstair == x && ydnstair == y) ||
		(sstairs.sx == x && sstairs.sy == y && !sstairs.up)) {
/*JP
	    gate_str = "down the stairs";
*/
	    gate_str = "階段から";
	    return (xdnstair == x && ydnstair == y) ?
		    MIGR_STAIRS_UP : MIGR_SSTAIRS;
	}
	if (xdnladder == x && ydnladder == y) {
/*JP
	    gate_str = "down the ladder";
*/
	    gate_str = "はしごから";
	    return MIGR_LADDER_UP;
	}

	if (((ttmp = t_at(x, y)) != 0 && ttmp->tseen) &&
		(ttmp->ttyp == TRAPDOOR || ttmp->ttyp == HOLE)) {
	    gate_str = (ttmp->ttyp == TRAPDOOR) ?
/*JP
		    "through the trap door" : "through the hole";
*/
		    "落し扉に" : "穴に";
	    return MIGR_RANDOM;
	}
	return MIGR_NOWHERE;
}

/*dokick.c*/
