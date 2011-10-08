/*	SCCS Id: @(#)mhitu.c	3.4	2003/11/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "artifact.h"

STATIC_VAR NEARDATA struct obj *otmp;

STATIC_DCL void FDECL(urustm, (struct monst *, struct obj *));
# ifdef OVL1
STATIC_DCL boolean FDECL(u_slip_free, (struct monst *,struct attack *));
STATIC_DCL int FDECL(passiveum, (struct permonst *,struct monst *,struct attack *));
# endif /* OVL1 */

#ifdef OVLB
# ifdef SEDUCE
STATIC_DCL void FDECL(mayberem, (struct obj *, const char *));
# endif
#endif /* OVLB */

STATIC_DCL boolean FDECL(diseasemu, (struct permonst *));
STATIC_DCL int FDECL(hitmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(gulpmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(explmu, (struct monst *,struct attack *,BOOLEAN_P));
STATIC_DCL void FDECL(missmu,(struct monst *,BOOLEAN_P,struct attack *));
STATIC_DCL void FDECL(mswings,(struct monst *,struct obj *));
STATIC_DCL void FDECL(wildmiss, (struct monst *,struct attack *));

STATIC_DCL void FDECL(hurtarmor,(int));
STATIC_DCL void FDECL(hitmsg,(struct monst *,struct attack *));

/* See comment in mhitm.c.  If we use this a lot it probably should be */
/* changed to a parameter to mhitu. */
static int dieroll;

#ifdef OVL1


STATIC_OVL void
hitmsg(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
	int compat;

	/* Note: if opposite gender, "seductively" */
	/* If same gender, "engagingly" for nymph, normal msg for others */
	if((compat = could_seduce(mtmp, &youmonst, mattk))
			&& !mtmp->mcan && !mtmp->mspec_used) {
#if 0 /*JP*/
		pline("%s %s you %s.", Monnam(mtmp),
			Blind ? "talks to" : "smiles at",
			compat == 2 ? "engagingly" : "seductively");
#else
		pline("%sはあなた%s%s。", Monnam(mtmp),
			compat == 2 ? "を引きつけるように" : "に好意をよせるように",
			Blind ? "話しかけた" : "微笑んだ");
#endif
	} else
	    switch (mattk->aatyp) {
		case AT_BITE:
/*JP
			pline("%s bites!", Monnam(mtmp));
*/
			pline("%sは噛みついた！", Monnam(mtmp));
			break;
		case AT_KICK:
#if 0 /*JP*/
			pline("%s kicks%c", Monnam(mtmp),
				    thick_skinned(youmonst.data) ? '.' : '!');
#else
			pline("%sは蹴とばした%s",Monnam(mtmp), 
					thick_skinned(youmonst.data) ? "。" : "！");
#endif
			break;
		case AT_STNG:
/*JP
			pline("%s stings!", Monnam(mtmp));
*/
			pline("%sは突きさした！", Monnam(mtmp));
			break;
		case AT_BUTT:
/*JP
			pline("%s butts!", Monnam(mtmp));
*/
			pline("%sは頭突きをくらわした！", Monnam(mtmp));
			break;
		case AT_TUCH:
/*JP
			pline("%s touches you!", Monnam(mtmp));
*/
			pline("%sはあなたに触れた！", Monnam(mtmp));
			break;
		case AT_TENT:
#if 0 /*JP*/
			pline("%s tentacles suck you!",
				        s_suffix(Monnam(mtmp)));
#else
			pline("%sの触手があなたの体液を吸いとった！", 
					Monnam(mtmp));
#endif
			break;
		case AT_EXPL:
		case AT_BOOM:
/*JP
			pline("%s explodes!", Monnam(mtmp));
*/
			pline("%sは爆発した！", Monnam(mtmp));
			break;
		default:
/*JP
			pline("%s hits!", Monnam(mtmp));
*/
			pline("%sの攻撃は命中した！", Monnam(mtmp));
	    }
}

STATIC_OVL void
missmu(mtmp, nearmiss, mattk)		/* monster missed you */
register struct monst *mtmp;
register boolean nearmiss;
register struct attack *mattk;
{
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);

	if(could_seduce(mtmp, &youmonst, mattk) && !mtmp->mcan)
/*JP
	    pline("%s pretends to be friendly.", Monnam(mtmp));
*/
	    pline("%sは友好的なふりをしている。",Monnam(mtmp));
	else {
	    if (!flags.verbose || !nearmiss)
/*JP
		pline("%s misses.", Monnam(mtmp));
*/
		pline("%sの攻撃ははずれた。", Monnam(mtmp));
	    else
/*JP
		pline("%s just misses!", Monnam(mtmp));
*/
		pline("%sの攻撃は空を切った。", Monnam(mtmp));
	}
	stop_occupation();
}

STATIC_OVL void
mswings(mtmp, otemp)		/* monster swings obj */
register struct monst *mtmp;
register struct obj *otemp;
{
	if (!flags.verbose || Blind || !mon_visible(mtmp))
		return;
#if 0 /*JP*/
	pline("%s %s %s %s.", Monnam(mtmp),
	      (objects[otemp->otyp].oc_dir & PIERCE) ? "thrusts" : "swings",
	      mhis(mtmp), singular(otemp, xname));
#else
	pline("%sは%s%s。", Monnam(mtmp),
	      xname(otemp),
	      (objects[otemp->otyp].oc_dir & PIERCE) ? "を突いた" : "を振りまわした");
#endif
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
	if (mattk->aatyp == AT_WEAP) {
	    struct obj *mwep = (mtmp == &youmonst) ? uwep : MON_WEP(mtmp);
	    /* "Foo's attack was poisoned." is pretty lame, but at least
	       it's better than "sting" when not a stinging attack... */
/*JP
	    return (!mwep || !mwep->opoisoned) ? "attack" : "weapon";
*/
	    return (!mwep || !mwep->opoisoned) ? "攻撃" : "武器";
	} else {
#if 0 /*JP*/
	    return (mattk->aatyp == AT_TUCH) ? "contact" :
		   (mattk->aatyp == AT_GAZE) ? "gaze" :
		   (mattk->aatyp == AT_BITE) ? "bite" : "sting";
#else
	    return (mattk->aatyp == AT_TUCH) ? "接触" :
		   (mattk->aatyp == AT_GAZE) ? "にらみ" :
		   (mattk->aatyp == AT_BITE) ? "噛みつき" : "突きさし";
#endif
	}
}

/* called when your intrinsic speed is taken away */
void
u_slow_down()
{
	HFast = 0L;
	if (!Fast)
/*JP
	    You("slow down.");
*/
	    You("動きが遅くなった。");
	else	/* speed boots */
/*JP
	    Your("quickness feels less natural.");
*/
	    You("速さについていけなくなった。");
	exercise(A_DEX, FALSE);
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL void
wildmiss(mtmp, mattk)		/* monster attacked your displaced image */
	register struct monst *mtmp;
	register struct attack *mattk;
{
	int compat;

	/* no map_invisible() -- no way to tell where _this_ is coming from */

	if (!flags.verbose) return;
	if (!cansee(mtmp->mx, mtmp->my)) return;
		/* maybe it's attacking an image around the corner? */

	compat = (mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX) &&
		 could_seduce(mtmp, &youmonst, (struct attack *)0);

	if (!mtmp->mcansee || (Invis && !perceives(mtmp->data))) {
	    const char *swings =
#if 0 /*JP*/
		mattk->aatyp == AT_BITE ? "snaps" :
		mattk->aatyp == AT_KICK ? "kicks" :
		(mattk->aatyp == AT_STNG ||
		 mattk->aatyp == AT_BUTT ||
		 nolimbs(mtmp->data)) ? "lunges" : "swings";
#else
		mattk->aatyp == AT_BITE ? "噛みつく" :
		mattk->aatyp == AT_KICK ? "蹴とばす" :
		(mattk->aatyp == AT_STNG ||
		 mattk->aatyp == AT_BUTT ||
		 nolimbs(mtmp->data)) ? "突進する" : "振り回す";
#endif

	    if (compat)
/*JP
		pline("%s tries to touch you and misses!", Monnam(mtmp));
*/
		pline("%sはあなたに触ろうとしたが失敗した！", Monnam(mtmp));
	    else
		switch(rn2(3)) {
#if 0 /*JP*/
		case 0: pline("%s %s wildly and misses!", Monnam(mtmp),
			      swings);
		    break;
		case 1: pline("%s attacks a spot beside you.", Monnam(mtmp));
		    break;
		case 2: pline("%s strikes at %s!", Monnam(mtmp),
				levl[mtmp->mux][mtmp->muy].typ == WATER
				    ? "empty water" : "thin air");
		    break;
		default:pline("%s %s wildly!", Monnam(mtmp), swings);
		    break;
#else /*JP*/
		case 0: pline("%sは激しく%sが、はずした。", Monnam(mtmp),
			      jpast(swings));
		    break;
		case 1: pline("%sの攻撃はあなたの脇腹をかすめた。", Monnam(mtmp));
		    break;
		case 2: pline("%sは%sを打ちつけた！", Monnam(mtmp),
				Underwater ? "水" : "何もないところ");
		    break;
		default:pline("%sは激しく%s！", Monnam(mtmp),
			      jpast(swings));
  		    break;
#endif /*JP*/
		}

	} else if (Displaced) {
	    if (compat)
#if 0 /*JP*/
		pline("%s smiles %s at your %sdisplaced image...",
			Monnam(mtmp),
			compat == 2 ? "engagingly" : "seductively",
			Invis ? "invisible " : "");
#else
		pline("%sは%sあなたの幻影に対して%s微笑んだ…",
			Monnam(mtmp),
			Invis ? "透明な" : "",
			compat == 2 ? "魅力的に" : "蠱惑的に");
#endif
	    else
#if 0 /*JP*/
		pline("%s strikes at your %sdisplaced image and misses you!",
			/* Note: if you're both invisible and displaced,
			 * only monsters which see invisible will attack your
			 * displaced image, since the displaced image is also
			 * invisible.
			 */
			Monnam(mtmp),
			Invis ? "invisible " : "");
#else
		pline("%sはあなたの%s幻影を攻撃し、はずした！",
			Monnam(mtmp),
			Invis ? "透明な" : "");
#endif

	} else if (Underwater) {
	    /* monsters may miss especially on water level where
	       bubbles shake the player here and there */
	    if (compat)
/*JP
		pline("%s reaches towards your distorted image.",Monnam(mtmp));
*/
		pline("%sはあなたの歪んだ幻影の方へ向かった。",Monnam(mtmp));
	    else
/*JP
		pline("%s is fooled by water reflections and misses!",Monnam(mtmp));
*/
		pline("%sは水の反射にだまされ、はずした！",Monnam(mtmp));

	} else impossible("%s attacks you without knowing your location?",
		Monnam(mtmp));
}

void
expels(mtmp, mdat, message)
register struct monst *mtmp;
register struct permonst *mdat; /* if mtmp is polymorphed, mdat != mtmp->data */
boolean message;
{
	if (message) {
		if (is_animal(mdat))
/*JP
			You("get regurgitated!");
*/
			You("吐きだされた！");
		else {
			char blast[40];
			register int i;

			blast[0] = '\0';
			for(i = 0; i < NATTK; i++)
				if(mdat->mattk[i].aatyp == AT_ENGL)
					break;
			if (mdat->mattk[i].aatyp != AT_ENGL)
			      impossible("Swallower has no engulfing attack?");
			else {
				if (is_whirly(mdat)) {
					switch (mdat->mattk[i].adtyp) {
						case AD_ELEC:
							Strcpy(blast,
/*JP
						      " in a shower of sparks");
*/
						      "の火花の雨の中から");
							break;
						case AD_COLD:
							Strcpy(blast,
/*JP
							" in a blast of frost");
*/
							"の冷気の風の中から");
							break;
#if 1 /*JP*//*By Hiramoto Kouji*/
						default:
							Strcpy(blast,
							"から");
							break;
#endif
					}
				} else
/*JP
					Strcpy(blast, " with a squelch");
*/
					Strcpy(blast, "から吐き出されるように");
/*JP
				You("get expelled from %s%s!",
*/
				You("%s%s脱出した！", 
				    mon_nam(mtmp), blast);
			}
		}
	}
	unstuck(mtmp);	/* ball&chain returned in unstuck() */
	mnexto(mtmp);
	newsym(u.ux,u.uy);
	spoteffects(TRUE);
	/* to cover for a case where mtmp is not in a next square */
	if(um_dist(mtmp->mx,mtmp->my,1))
/*JP
		pline("Brrooaa...  You land hard at some distance.");
*/
		pline("ブロロロ…遠くに着陸するのは難しい。");
}

#endif /* OVLB */
#ifdef OVL0

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(mptr, indx, prev_result, alt_attk_buf)
struct permonst *mptr;
int indx, prev_result[];
struct attack *alt_attk_buf;
{
    struct attack *attk = &mptr->mattk[indx];

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > 0 &&
	    (attk->adtyp == AD_DISE ||
		attk->adtyp == AD_PEST ||
		attk->adtyp == AD_FAMN) &&
	    attk->adtyp == mptr->mattk[indx - 1].adtyp) {
	*alt_attk_buf = *attk;
	attk = alt_attk_buf;
	attk->adtyp = AD_STUN;
    }
    return attk;
}

/*
 * mattacku: monster attacks you
 *	returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *	Note: if you're displaced or invisible the monster might attack the
 *		wrong position...
 *	Assumption: it's attacking you or an empty square; if there's another
 *		monster which it attacks by mistake, the caller had better
 *		take care of it...
 */
int
mattacku(mtmp)
	register struct monst *mtmp;
{
	struct	attack	*mattk, alt_attk;
	int	i, j, tmp, sum[NATTK];
	struct	permonst *mdat = mtmp->data;
	boolean ranged = (distu(mtmp->mx, mtmp->my) > 3);
		/* Is it near you?  Affects your actions */
	boolean range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
		/* Does it think it's near you?  Affects its actions */
	boolean foundyou = (mtmp->mux==u.ux && mtmp->muy==u.uy);
		/* Is it attacking you or your image? */
	boolean youseeit = canseemon(mtmp);
		/* Might be attacking your image around the corner, or
		 * invisible, or you might be blind....
		 */
	
	if(!ranged) nomul(0);
	if(mtmp->mhp <= 0 || (Underwater && !is_swimmer(mtmp->data)))
	    return(0);

	/* If swallowed, can only be affected by u.ustuck */
	if(u.uswallow) {
	    if(mtmp != u.ustuck)
		return(0);
	    u.ustuck->mux = u.ux;
	    u.ustuck->muy = u.uy;
	    range2 = 0;
	    foundyou = 1;
	    if(u.uinvulnerable) return (0); /* stomachs can't hurt you! */
	}

#ifdef STEED
	else if (u.usteed) {
		if (mtmp == u.usteed)
			/* Your steed won't attack you */
			return (0);
		/* Orcs like to steal and eat horses and the like */
		if (!rn2(is_orc(mtmp->data) ? 2 : 4) &&
				distu(mtmp->mx, mtmp->my) <= 2) {
			/* Attack your steed instead */
			i = mattackm(mtmp, u.usteed);
			if ((i & MM_AGR_DIED))
				return (1);
			if (i & MM_DEF_DIED || u.umoved)
				return (0);
			/* Let your steed retaliate */
			return (!!(mattackm(u.usteed, mtmp) & MM_DEF_DIED));
		}
	}
#endif

	if (u.uundetected && !range2 && foundyou && !u.uswallow) {
		u.uundetected = 0;
		if (is_hider(youmonst.data)) {
		    coord cc; /* maybe we need a unexto() function? */
		    struct obj *obj;

/*JP
		    You("fall from the %s!", ceiling(u.ux,u.uy));
*/
		    You("%sから落ちた！", ceiling(u.ux,u.uy));
		    if (enexto(&cc, u.ux, u.uy, youmonst.data)) {
			remove_monster(mtmp->mx, mtmp->my);
			newsym(mtmp->mx,mtmp->my);
			place_monster(mtmp, u.ux, u.uy);
			if(mtmp->wormno) worm_move(mtmp);
			teleds(cc.x, cc.y, TRUE);
			set_apparxy(mtmp);
			newsym(u.ux,u.uy);
		    } else {
#if 0 /*JP*/
			pline("%s is killed by a falling %s (you)!",
					Monnam(mtmp), youmonst.data->mname);
#else
			pline("%sは落ちてきた%s(あなた)によって死んだ！",
						Monnam(mtmp),jtrns_mon_gen(youmonst.data->mname, flags.female));
#endif
			killed(mtmp);
			newsym(u.ux,u.uy);
			if (mtmp->mhp > 0) return 0;
			else return 1;
		    }
		    if (youmonst.data->mlet != S_PIERCER)
			return(0);	/* trappers don't attack */

		    obj = which_armor(mtmp, WORN_HELMET);
		    if (obj && is_metallic(obj)) {
/*JP
			Your("blow glances off %s helmet.",
*/
			Your("攻撃は%sの兜をかすめた。",
			               s_suffix(mon_nam(mtmp)));
		    } else {
			if (3 + find_mac(mtmp) <= rnd(20)) {
/*JP
			    pline("%s is hit by a falling piercer (you)!",
*/
			    pline("%sは落ちる針(あなた)で傷ついた！",
								Monnam(mtmp));
			    if ((mtmp->mhp -= d(3,6)) < 1)
				killed(mtmp);
			} else
/*JP
			  pline("%s is almost hit by a falling piercer (you)!",
*/
			  pline("%sはもう少しで落ちる針(あなた)で傷つくところだった！",
								Monnam(mtmp));
		    }
		} else {
		    if (!youseeit)
/*JP
			pline("It tries to move where you are hiding.");
*/
			pline("何者かがあなたが隠れているところを移動しようとした。");
		    else {
			/* Ugly kludge for eggs.  The message is phrased so as
			 * to be directed at the monster, not the player,
			 * which makes "laid by you" wrong.  For the
			 * parallelism to work, we can't rephrase it, so we
			 * zap the "laid by you" momentarily instead.
			 */
			struct obj *obj = level.objects[u.ux][u.uy];

			if (obj ||
			      (youmonst.data->mlet == S_EEL && is_pool(u.ux, u.uy))) {
			    int save_spe = 0; /* suppress warning */
			    if (obj) {
				save_spe = obj->spe;
				if (obj->otyp == EGG) obj->spe = 0;
			    }
			    if (youmonst.data->mlet == S_EEL)
#if 0 /*JP*/
		pline("Wait, %s!  There's a hidden %s named %s there!",
				m_monnam(mtmp), youmonst.data->mname, plname);
#else
	     pline("待て、%s！%sという名の%sが隠れている！",
				m_monnam(mtmp), plname, jtrns_mon_gen(youmonst.data->mname, flags.female));
#endif
			    else
#if 0 /*JP*/
	     pline("Wait, %s!  There's a %s named %s hiding under %s!",
				m_monnam(mtmp), youmonst.data->mname, plname,
				doname(level.objects[u.ux][u.uy]));
#else
	     pline("待て、%s！%sという名の%sが%sの下に隠れている！",
				m_monnam(mtmp), plname, jtrns_mon_gen(youmonst.data->mname, flags.female), 
				doname(level.objects[u.ux][u.uy]));
#endif
			    if (obj) obj->spe = save_spe;
			} else
			    impossible("hiding under nothing?");
		    }
		    newsym(u.ux,u.uy);
		}
		return(0);
	}
	if (youmonst.data->mlet == S_MIMIC && youmonst.m_ap_type &&
		    !range2 && foundyou && !u.uswallow) {
/*JP
		if (!youseeit) pline("It gets stuck on you.");
*/
		if (!youseeit) pline("何者かがあなたの上にのしかかった。");
#if 0 /*JP*/
		else pline("Wait, %s!  That's a %s named %s!",
			   m_monnam(mtmp), youmonst.data->mname, plname);
#else
		    else pline("待て、%s！それは%sという名の%sだ！",
			       m_monnam(mtmp), 
			       plname, jtrns_mon_gen(youmonst.data->mname, flags.female));
#endif
		u.ustuck = mtmp;
		youmonst.m_ap_type = M_AP_NOTHING;
		youmonst.mappearance = 0;
		newsym(u.ux,u.uy);
		return(0);
	}

	/* player might be mimicking an object */
	if (youmonst.m_ap_type == M_AP_OBJECT && !range2 && foundyou && !u.uswallow) {
	    if (!youseeit)
#if 0 /*JP*/
		 pline("%s %s!", Something,
			(likes_gold(mtmp->data) && youmonst.mappearance == GOLD_PIECE) ?
			"tries to pick you up" : "disturbs you");
#else
		 pline("%sは%s！", Something,
			(likes_gold(mtmp->data) && youmonst.mappearance == GOLD_PIECE) ?
			"あなたを拾おうとした" : "無視した");
#endif
#if 0 /*JP*/
	    else pline("Wait, %s!  That %s is really %s named %s!",
			m_monnam(mtmp),
			mimic_obj_name(&youmonst),
			an(mons[u.umonnum].mname),
			plname);
#else
	    else pline("待て、%s！その%sは%sという名の%sだ！",
			m_monnam(mtmp),
			mimic_obj_name(&youmonst),
			plname,
		        jtrns_mon(mons[u.umonnum].mname));
#endif
	    if (multi < 0) {	/* this should always be the case */
		char buf[BUFSZ];
#if 0 /*JP*/
		Sprintf(buf, "You appear to be %s again.",
			Upolyd ? (const char *) an(youmonst.data->mname) :
			    (const char *) "yourself");
#else
		Sprintf(buf, "あなたはふたたび%sになった。",
			Upolyd ? (const char *) a_monnam(&youmonst) :
			    (const char *) "自分自身");
#endif
		unmul(buf);	/* immediately stop mimicking */
	    }
	    return 0;
	}

/*	Work out the armor class differential	*/
	tmp = AC_VALUE(u.uac) + 10;		/* tmp ~= 0 - 20 */
	tmp += mtmp->m_lev;
	if(multi < 0) tmp += 4;
	if((Invis && !perceives(mdat)) || !mtmp->mcansee)
		tmp -= 2;
	if(mtmp->mtrapped) tmp -= 2;
	if(tmp <= 0) tmp = 1;

	/* make eels visible the moment they hit/miss us */
	if(mdat->mlet == S_EEL && mtmp->minvis && cansee(mtmp->mx,mtmp->my)) {
		mtmp->minvis = 0;
		newsym(mtmp->mx,mtmp->my);
	}

/*	Special demon handling code */
	if(!mtmp->cham && is_demon(mdat) && !range2
	   && mtmp->data != &mons[PM_BALROG]
	   && mtmp->data != &mons[PM_SUCCUBUS]
	   && mtmp->data != &mons[PM_INCUBUS])
	    if(!mtmp->mcan && !rn2(13))	msummon(mtmp);

/*	Special lycanthrope handling code */
	if(!mtmp->cham && is_were(mdat) && !range2) {

	    if(is_human(mdat)) {
		if(!rn2(5 - (night() * 2)) && !mtmp->mcan) new_were(mtmp);
	    } else if(!rn2(30) && !mtmp->mcan) new_were(mtmp);
	    mdat = mtmp->data;

	    if(!rn2(10) && !mtmp->mcan) {
	    	int numseen, numhelp;
#if 0 /*JP*/
		char buf[BUFSZ], genericwere[BUFSZ];
#else
		char genericwere[BUFSZ];
#endif

		Strcpy(genericwere, "creature");
		numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
		if (youseeit) {
/*JP
			pline("%s summons help!", Monnam(mtmp));
*/
			pline("%sは助けを呼んだ！", Monnam(mtmp));
			if (numhelp > 0) {
			    if (numseen == 0)
/*JP
				You_feel("hemmed in.");
*/
			pline("何者かに囲まれたような気がする。");
/*JP
			} else pline("But none comes.");
*/
			} else pline("しかし何も来なかった。");
		} else {
			const char *from_nowhere;

			if (flags.soundok) {
#if 0 /*JP:T*/
				pline("%s %s!", Something,
					makeplural(growl_sound(mtmp)));
#else
				pline("何かが%s！", growl_sound(mtmp));
#endif
				from_nowhere = "";
/*JP
			} else from_nowhere = " from nowhere";
*/
			} else from_nowhere = "どこからともなく";
			if (numhelp > 0) {
/*JP
			    if (numseen < 1) You_feel("hemmed in.");
*/
			    if (numseen < 1) pline("何者かに囲まれたような気がする。");
			    else {
#if 0 /*JP*/
				if (numseen == 1)
			    		Sprintf(buf, "%s appears",
							an(genericwere));
			    	else
			    		Sprintf(buf, "%s appear",
							makeplural(genericwere));
				pline("%s%s!", upstart(buf), from_nowhere);
#else
				pline("%sが%s現れた！",
				      strcmp(genericwere, "creature") ?
				      jtrns_mon(genericwere) :
				      "何か", from_nowhere);
#endif
			    }
			} /* else no help came; but you didn't know it tried */
		}
	    }
	}

	if(u.uinvulnerable) {
	    /* monsters won't attack you */
	    if(mtmp == u.ustuck)
/*JP
		pline("%s loosens its grip slightly.", Monnam(mtmp));
*/
		pline("%sは締めつけをわずかに緩めた。", Monnam(mtmp));
	    else if(!range2) {
		if (youseeit || sensemon(mtmp))
/*JP
		    pline("%s starts to attack you, but pulls back.",
*/
		    pline("%sはあなたを攻撃しかけたが、ひっこめた。",
			  Monnam(mtmp));
		else
/*JP
		    You_feel("%s move nearby.", something);
*/
		    pline("何者かがあなたのそばを通りぬけたように感じた。");
	    }
	    return (0);
	}

	/* Unlike defensive stuff, don't let them use item _and_ attack. */
	if(find_offensive(mtmp)) {
		int foo = use_offensive(mtmp);

		if (foo != 0) return(foo==1);
	}

	for(i = 0; i < NATTK; i++) {

	    sum[i] = 0;
	    mattk = getmattk(mdat, i, sum, &alt_attk);
	    if (u.uswallow && (mattk->aatyp != AT_ENGL))
		continue;
	    switch(mattk->aatyp) {
		case AT_CLAW:	/* "hand to hand" attacks */
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
		case AT_TENT:
			if(!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict ||
					!touch_petrifies(youmonst.data))) {
			    if (foundyou) {
				if(tmp > (j = rnd(20+i))) {
				    if (mattk->aatyp != AT_KICK ||
					    !thick_skinned(youmonst.data))
					sum[i] = hitmu(mtmp, mattk);
				} else
				    missmu(mtmp, (tmp == j), mattk);
			    } else
				wildmiss(mtmp, mattk);
			}
			break;

		case AT_HUGS:	/* automatic if prev two attacks succeed */
			/* Note: if displaced, prev attacks never succeeded */
			if((!range2 && i>=2 && sum[i-1] && sum[i-2])
							|| mtmp == u.ustuck)
				sum[i]= hitmu(mtmp, mattk);
			break;

		case AT_GAZE:	/* can affect you either ranged or not */
			/* Medusa gaze already operated through m_respond in
			 * dochug(); don't gaze more than once per round.
			 */
			if (mdat != &mons[PM_MEDUSA])
				sum[i] = gazemu(mtmp, mattk);
			break;

		case AT_EXPL:	/* automatic hit if next to, and aimed at you */
			if(!range2) sum[i] = explmu(mtmp, mattk, foundyou);
			break;

		case AT_ENGL:
			if (!range2) {
			    if(foundyou) {
				if(u.uswallow || tmp > (j = rnd(20+i))) {
				    /* Force swallowing monster to be
				     * displayed even when player is
				     * moving away */
				    flush_screen(1);
				    sum[i] = gulpmu(mtmp, mattk);
				} else {
				    missmu(mtmp, (tmp == j), mattk);
				}
			    } else if (is_animal(mtmp->data)) {
/*JP
				pline("%s gulps some air!", Monnam(mtmp));
*/
				pline("%sは息を吸いこんた！", Monnam(mtmp));
			    } else {
				if (youseeit)
/*JP
				    pline("%s lunges forward and recoils!",
*/
				    pline("%sは突進し、戻った！",
					  Monnam(mtmp));
				else
#if 0 /*JP*/
				    You_hear("a %s nearby.",
					     is_whirly(mtmp->data) ?
						"rushing noise" : "splat");
#else
						You_hear("すぐそばで%s音を聞いた。",
						    is_whirly(mtmp->data)?
						    "突撃してくる" :
						    "ピシャッという");
#endif
			   }
			}
			break;
		case AT_BREA:
			if(range2) sum[i] = breamu(mtmp, mattk);
			/* Note: breamu takes care of displacement */
			break;
		case AT_SPIT:
			if(range2) sum[i] = spitmu(mtmp, mattk);
			/* Note: spitmu takes care of displacement */
			break;
		case AT_WEAP:
			if(range2) {
#ifdef REINCARNATION
				if (!Is_rogue_level(&u.uz))
#endif
					thrwmu(mtmp);
			} else {
			    int hittmp = 0;

			    /* Rare but not impossible.  Normally the monster
			     * wields when 2 spaces away, but it can be
			     * teleported or whatever....
			     */
			    if (mtmp->weapon_check == NEED_WEAPON ||
							!MON_WEP(mtmp)) {
				mtmp->weapon_check = NEED_HTH_WEAPON;
				/* mon_wield_item resets weapon_check as
				 * appropriate */
				if (mon_wield_item(mtmp) != 0) break;
			    }
			    if (foundyou) {
				otmp = MON_WEP(mtmp);
				if(otmp) {
				    hittmp = hitval(otmp, &youmonst);
				    tmp += hittmp;
				    mswings(mtmp, otmp);
				}
				if(tmp > (j = dieroll = rnd(20+i)))
				    sum[i] = hitmu(mtmp, mattk);
				else
				    missmu(mtmp, (tmp == j), mattk);
				/* KMH -- Don't accumulate to-hit bonuses */
				if (otmp)
					tmp -= hittmp;
			    } else
				wildmiss(mtmp, mattk);
			}
			break;
		case AT_MAGC:
			if (range2)
			    sum[i] = buzzmu(mtmp, mattk);
			else {
			    if (foundyou)
				sum[i] = castmu(mtmp, mattk, TRUE, TRUE);
			    else
				sum[i] = castmu(mtmp, mattk, TRUE, FALSE);
			}
			break;

		default:		/* no attack */
			break;
	    }
	    if(flags.botl) bot();
	/* give player a chance of waking up before dying -kaa */
	    if(sum[i] == 1) {	    /* successful attack */
		if (u.usleep && u.usleep < monstermoves && !rn2(10)) {
		    multi = -1;
/*JP
		    nomovemsg = "The combat suddenly awakens you.";
*/
		    nomovemsg = "殴られて目がさめた。";
		}
	    }
	    if(sum[i] == 2) return 1;		/* attacker dead */
	    if(sum[i] == 3) break;  /* attacker teleported, no more attacks */
	    /* sum[i] == 0: unsuccessful attack */
	}
	return(0);
}

#endif /* OVL0 */
#ifdef OVLB

/*
 * helper function for some compilers that have trouble with hitmu
 */

STATIC_OVL void
hurtarmor(attk)
int attk;
{
	int	hurt;

	switch(attk) {
	    /* 0 is burning, which we should never be called with */
	    case AD_RUST: hurt = 1; break;
	    case AD_CORR: hurt = 3; break;
	    default: hurt = 2; break;
	}

	/* What the following code does: it keeps looping until it
	 * finds a target for the rust monster.
	 * Head, feet, etc... not covered by metal, or covered by
	 * rusty metal, are not targets.  However, your body always
	 * is, no matter what covers it.
	 */
	while (1) {
	    switch(rn2(5)) {
	    case 0:
		if (!uarmh || !rust_dmg(uarmh, xname(uarmh), hurt, FALSE, &youmonst))
			continue;
		break;
	    case 1:
		if (uarmc) {
		    (void)rust_dmg(uarmc, xname(uarmc), hurt, TRUE, &youmonst);
		    break;
		}
		/* Note the difference between break and continue;
		 * break means it was hit and didn't rust; continue
		 * means it wasn't a target and though it didn't rust
		 * something else did.
		 */
		if (uarm)
		    (void)rust_dmg(uarm, xname(uarm), hurt, TRUE, &youmonst);
#ifdef TOURIST
		else if (uarmu)
		    (void)rust_dmg(uarmu, xname(uarmu), hurt, TRUE, &youmonst);
#endif
		break;
	    case 2:
		if (!uarms || !rust_dmg(uarms, xname(uarms), hurt, FALSE, &youmonst))
		    continue;
		break;
	    case 3:
		if (!uarmg || !rust_dmg(uarmg, xname(uarmg), hurt, FALSE, &youmonst))
		    continue;
		break;
	    case 4:
		if (!uarmf || !rust_dmg(uarmf, xname(uarmf), hurt, FALSE, &youmonst))
		    continue;
		break;
	    }
	    break; /* Out of while loop */
	}
}

#endif /* OVLB */
#ifdef OVL1

STATIC_OVL boolean
diseasemu(mdat)
struct permonst *mdat;
{
	if (Sick_resistance) {
/*JP
		You_feel("a slight illness.");
*/
		You("すこし気分が悪くなったように感じた。");
		return FALSE;
	} else {
#if 0 /*JP*/
		make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON), 20),
			mdat->mname, TRUE, SICK_NONVOMITABLE);
#else
		make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON), 20),
			jtrns_mon(mdat->mname), TRUE, SICK_NONVOMITABLE);
#endif
		return TRUE;
	}
}

/* check whether slippery clothing protects from hug or wrap attack */
STATIC_OVL boolean
u_slip_free(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
	struct obj *obj = (uarmc ? uarmc : uarm);

#ifdef TOURIST
	if (!obj) obj = uarmu;
#endif
	if (mattk->adtyp == AD_DRIN) obj = uarmh;

	/* if your cloak/armor is greased, monster slips off; this
	   protection might fail (33% chance) when the armor is cursed */
	if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK) &&
		(!obj->cursed || rn2(3))) {
#if 0 /*JP*/
	    pline("%s %s your %s %s!",
		  Monnam(mtmp),
		  (mattk->adtyp == AD_WRAP) ?
			"slips off of" : "grabs you, but cannot hold onto",
		  obj->greased ? "greased" : "slippery",
		  /* avoid "slippery slippery cloak"
		     for undiscovered oilskin cloak */
		  (obj->greased || objects[obj->otyp].oc_name_known) ?
			xname(obj) : cloak_simple_name(obj));
#else
	    pline("%sは%s%s%s！",
		  Monnam(mtmp),
		  obj->greased ? "脂の塗られた" : "滑りやすい",
		  (obj->greased || objects[obj->otyp].oc_name_known) ?
			xname(obj) : cloak_simple_name(obj),
		  (mattk->adtyp == AD_WRAP) ?
			"で滑った" : "をつかまえようとしたが、できなかった");
#endif

	    if (obj->greased && !rn2(2)) {
/*JP
		pline_The("grease wears off.");
*/
		pline("脂は落ちてしまった。");
		obj->greased = 0;
		update_inventory();
	    }
	    return TRUE;
	}
	return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(mon)
struct monst *mon;
{
	struct obj *armor;
	int armpro = 0;

	armor = (mon == &youmonst) ? uarm : which_armor(mon, W_ARM);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmc : which_armor(mon, W_ARMC);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmh : which_armor(mon, W_ARMH);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;

	/* armor types for shirt, gloves, shoes, and shield don't currently
	   provide any magic cancellation but we might as well be complete */
#ifdef TOURIST
	armor = (mon == &youmonst) ? uarmu : which_armor(mon, W_ARMU);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
#endif
	armor = (mon == &youmonst) ? uarmg : which_armor(mon, W_ARMG);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmf : which_armor(mon, W_ARMF);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarms : which_armor(mon, W_ARMS);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;

#ifdef STEED
	/* this one is really a stretch... */
	armor = (mon == &youmonst) ? 0 : which_armor(mon, W_SADDLE);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
#endif

	return armpro;
}

/*
 * hitmu: monster hits you
 *	  returns 2 if monster dies (e.g. "yellow light"), 1 otherwise
 *	  3 if the monster lives but teleported/paralyzed, so it can't keep
 *	       attacking you
 */
STATIC_OVL int
hitmu(mtmp, mattk)
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	register struct permonst *mdat = mtmp->data;
	register int uncancelled, ptmp;
	int dmg, armpro, permdmg;
	char	 buf[BUFSZ];
	struct permonst *olduasmon = youmonst.data;
	int res;

	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);

/*	If the monster is undetected & hits you, you should know where
 *	the attack came from.
 */
	if(mtmp->mundetected && (hides_under(mdat) || mdat->mlet == S_EEL)) {
	    mtmp->mundetected = 0;
	    if (!(Blind ? Blind_telepat : Unblind_telepat)) {
		struct obj *obj;
#if 0 /*JP*/
		const char *what;
#endif

		if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0) {
#if 0 /*JP*/
		    if (Blind && !obj->dknown)
			what = something;
		    else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
			what = "the water";
		    else
			what = doname(obj);

		    pline("%s was hidden under %s!", Amonnam(mtmp), what);
#else
		    if (Blind && !obj->dknown)
		        pline("%sが何かの下に隠れている！", Amonnam(mtmp));
		    else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
		        pline("%sが水中に隠れている！", Amonnam(mtmp));
		    else
		        pline("%sが%sの下に隠れている！", Amonnam(mtmp), doname(obj));
#endif
		}
		newsym(mtmp->mx, mtmp->my);
	    }
	}

/*	First determine the base damage done */
	dmg = d((int)mattk->damn, (int)mattk->damd);
	if(is_undead(mdat) && midnight())
		dmg += d((int)mattk->damn, (int)mattk->damd); /* extra damage */

/*	Next a cancellation factor	*/
/*	Use uncancelled when the cancellation factor takes into account certain
 *	armor's special magic protection.  Otherwise just use !mtmp->mcan.
 */
	armpro = magic_negation(&youmonst);
	uncancelled = !mtmp->mcan && ((rn2(3) >= armpro) || !rn2(50));

	permdmg = 0;
/*	Now, adjust damages via resistances or specific attacks */
	switch(mattk->adtyp) {
	    case AD_PHYS:
		if (mattk->aatyp == AT_HUGS && !sticks(youmonst.data)) {
		    if(!u.ustuck && rn2(2)) {
			if (u_slip_free(mtmp, mattk)) {
			    dmg = 0;
			} else {
			    u.ustuck = mtmp;
/*JP
			    pline("%s grabs you!", Monnam(mtmp));
*/
			    pline("%sにつかまえられている！", Monnam(mtmp));
			}
		    } else if(u.ustuck == mtmp) {
			exercise(A_STR, FALSE);
#if 0 /*JP*/
			You("are being %s.",
			      (mtmp->data == &mons[PM_ROPE_GOLEM])
			      ? "choked" : "crushed");
#else
			You("%sている。",
			      (mtmp->data == &mons[PM_ROPE_GOLEM])
			      ? "首を絞められ" : "押しつぶされ");
#endif
		    }
		} else {			  /* hand to hand weapon */
		    if(mattk->aatyp == AT_WEAP && otmp) {
			if (otmp->otyp == CORPSE
				&& touch_petrifies(&mons[otmp->corpsenm])) {
			    dmg = 1;
#if 0 /*JP*/
			    pline("%s hits you with the %s corpse.",
				Monnam(mtmp), mons[otmp->corpsenm].mname);
#else
			    pline("%sは%sの死体で攻撃した。",
				Monnam(mtmp), jtrns_mon(mons[otmp->corpsenm].mname));
#endif
			    if (!Stoned)
				goto do_stone;
			}
			dmg += dmgval(otmp, &youmonst);
			if (dmg <= 0) dmg = 1;
			if (!(otmp->oartifact &&
				artifact_hit(mtmp, &youmonst, otmp, &dmg,dieroll)))
			     hitmsg(mtmp, mattk);
			if (!dmg) break;
			if (u.mh > 1 && u.mh > ((u.uac>0) ? dmg : dmg+u.uac) &&
				   objects[otmp->otyp].oc_material == IRON &&
					(u.umonnum==PM_BLACK_PUDDING
					|| u.umonnum==PM_BROWN_PUDDING)) {
			    /* This redundancy necessary because you have to
			     * take the damage _before_ being cloned.
			     */
			    if (u.uac < 0) dmg += u.uac;
			    if (dmg < 1) dmg = 1;
			    if (dmg > 1) exercise(A_STR, FALSE);
			    u.mh -= dmg;
			    flags.botl = 1;
			    dmg = 0;
			    if(cloneu())
/*JP
			    You("divide as %s hits you!",mon_nam(mtmp));
*/
			    pline("%sの攻撃によってあなたは分裂した！", mon_nam(mtmp));
			}
			urustm(mtmp, otmp);
		    } else if (mattk->aatyp != AT_TUCH || dmg != 0 ||
				mtmp != u.ustuck)
			hitmsg(mtmp, mattk);
		}
		break;
	    case AD_DISE:
		hitmsg(mtmp, mattk);
		if (!diseasemu(mdat)) dmg = 0;
		break;
	    case AD_FIRE:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
/*JP
		    pline("You're %s!", on_fire(youmonst.data, mattk));
*/
		    pline("あなたは%s！", on_fire(youmonst.data, mattk));
		    if (youmonst.data == &mons[PM_STRAW_GOLEM] ||
		        youmonst.data == &mons[PM_PAPER_GOLEM]) {
/*JP
			    You("roast!");
*/
			    You("焦げた！");
			    /* KMH -- this is okay with unchanging */
			    rehumanize();
			    break;
		    } else if (Fire_resistance) {
/*JP
			pline_The("fire doesn't feel hot!");
*/
			pline("火はぜんぜん熱くない！");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(SCROLL_CLASS, AD_FIRE);
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(POTION_CLASS, AD_FIRE);
		    if((int) mtmp->m_lev > rn2(25))
			destroy_item(SPBOOK_CLASS, AD_FIRE);
		    burn_away_slime();
		} else dmg = 0;
		break;
	    case AD_COLD:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
/*JP
		    pline("You're covered in frost!");
*/
		    You("氷で覆われた！");
		    if (Cold_resistance) {
/*JP
			pline_The("frost doesn't seem cold!");
*/
			pline("氷は冷さを感じさせない！");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(POTION_CLASS, AD_COLD);
		} else dmg = 0;
		break;
	    case AD_ELEC:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
/*JP
		    You("get zapped!");
*/
		    You("電撃をくらった！");
		    if (Shock_resistance) {
/*JP
			pline_The("zap doesn't shock you!");
*/
			pline("電撃はしびれを感じさせない！");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(WAND_CLASS, AD_ELEC);
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(RING_CLASS, AD_ELEC);
		} else dmg = 0;
		break;
	    case AD_SLEE:
		hitmsg(mtmp, mattk);
		if (uncancelled && multi >= 0 && !rn2(5)) {
		    if (Sleep_resistance) break;
		    fall_asleep(-rnd(10), TRUE);
/*JP
		    if (Blind) You("are put to sleep!");
*/
		    if (Blind) You("眠りにおちた！");
/*JP
		    else You("are put to sleep by %s!", mon_nam(mtmp));
*/
		    else You("%sに眠らされた！", mon_nam(mtmp));
		}
		break;
	    case AD_BLND:
		if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj*)0)) {
/*JP
		    if (!Blind) pline("%s blinds you!", Monnam(mtmp));
*/
		    if (!Blind) pline("%sの攻撃で目が見えなくなった！", Monnam(mtmp));
		    make_blinded(Blinded+(long)dmg,FALSE);
		    if (!Blind) Your(vision_clears);
		}
		dmg = 0;
		break;
	    case AD_DRST:
		ptmp = A_STR;
		goto dopois;
	    case AD_DRDX:
		ptmp = A_DEX;
		goto dopois;
	    case AD_DRCO:
		ptmp = A_CON;
dopois:
		hitmsg(mtmp, mattk);
		if (uncancelled && !rn2(8)) {
#if 0 /*JP*/
		    Sprintf(buf, "%s %s",
			    s_suffix(Monnam(mtmp)), mpoisons_subj(mtmp, mattk));
#else
			Sprintf(buf, "%sの%s",
			    Monnam(mtmp), mpoisons_subj(mtmp, mattk));
#endif
/*JP
		    poisoned(buf, ptmp, mdat->mname, 30);
*/
		    poisoned(buf, ptmp, jtrns_mon(mdat->mname), 30);
		}
		break;
	    case AD_DRIN:
		hitmsg(mtmp, mattk);
		if (defends(AD_DRIN, uwep) || !has_head(youmonst.data)) {
/*JP
		    You("don't seem harmed.");
*/
		    You("傷ついていないようだ。");
		    /* Not clear what to do for green slimes */
		    break;
		}
		if (u_slip_free(mtmp,mattk)) break;

		if (uarmh && rn2(8)) {
		    /* not body_part(HEAD) */
/*JP
		    Your("helmet blocks the attack to your head.");
*/
		    Your("兜が頭への攻撃を防いだ。");
		    break;
		}
		if (Half_physical_damage) dmg = (dmg+1) / 2;
		mdamageu(mtmp, dmg);

		if (!uarmh || uarmh->otyp != DUNCE_CAP) {
/*JP
		    Your("brain is eaten!");
*/
		    Your("脳は食べられた！");
		    /* No such thing as mindless players... */
		    if (ABASE(A_INT) <= ATTRMIN(A_INT)) {
			int lifesaved = 0;
			struct obj *wore_amulet = uamul;

			while(1) {
			    /* avoid looping on "die(y/n)?" */
			    if (lifesaved && (discover || wizard)) {
				if (wore_amulet && !uamul) {
				    /* used up AMULET_OF_LIFE_SAVING; still
				       subject to dying from brainlessness */
				    wore_amulet = 0;
				} else {
				    /* explicitly chose not to die;
				       arbitrarily boost intelligence */
				    ABASE(A_INT) = ATTRMIN(A_INT) + 2;
/*JP
				    You_feel("like a scarecrow.");
*/
				    You("かかしのような気持がした。");
				    break;
				}
			    }

			    if (lifesaved)
/*JP
				pline("Unfortunately your brain is still gone.");
*/
			        pline("残念ながらあなたには脳がない。");
			    else
/*JP
				Your("last thought fades away.");
*/
				Your("最後の思いが走馬燈のように横ぎった。");
/*JP
			    killer = "brainlessness";
*/
			    killer = "脳を失って";
			    killer_format = KILLED_BY;
			    done(DIED);
			    lifesaved++;
			}
		    }
		}
		/* adjattrib gives dunce cap message when appropriate */
		(void) adjattrib(A_INT, -rnd(2), FALSE);
		forget_levels(25);	/* lose memory of 25% of levels */
		forget_objects(25);	/* lose memory of 25% of objects */
		exercise(A_WIS, FALSE);
		break;
	    case AD_PLYS:
		hitmsg(mtmp, mattk);
		if (uncancelled && multi >= 0 && !rn2(3)) {
		    if (Free_action) {
/*JP
			You("momentarily stiffen.");            
*/
			You("一瞬硬直した。");
		    } else {
/*JP
			if (Blind) You("are frozen!");
*/
		    if (Blind) You("動けない！");
/*JP
			else You("are frozen by %s!", mon_nam(mtmp));
*/
		    else pline("%sによって動けなくなった！", mon_nam(mtmp));
			nomovemsg = 0;	/* default: "you can move again" */
			nomul(-rnd(10));
			exercise(A_DEX, FALSE);
		    }
		}
		break;
	    case AD_DRLI:
		hitmsg(mtmp, mattk);
		if (uncancelled && !rn2(3) && !Drain_resistance) {
/*JP
		    losexp("life drainage");
*/
		    losexp("生命力を吸収されて");
		}
		break;
	    case AD_LEGS:
		{ register long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
/*JP
		  const char *sidestr = (side == RIGHT_SIDE) ? "right" : "left";
*/
		  const char *sidestr = (side == RIGHT_SIDE) ? "右" : "左";

		/* This case is too obvious to ignore, but Nethack is not in
		 * general very good at considering height--most short monsters
		 * still _can_ attack you when you're flying or mounted.
		 * [FIXME: why can't a flying attacker overcome this?]
		 */
		  if (
#ifdef STEED
			u.usteed ||
#endif
				    Levitation || Flying) {
#if 0 /*JP*/
		    pline("%s tries to reach your %s %s!", Monnam(mtmp),
			  sidestr, body_part(LEG));
#else
		    pline("%sはあなたの%s%sに攻撃しようとした！", Monnam(mtmp),
			  sidestr, body_part(LEG));
#endif
		    dmg = 0;
		  } else if (mtmp->mcan) {
#if 0 /*JP*/
		    pline("%s nuzzles against your %s %s!", Monnam(mtmp),
			  sidestr, body_part(LEG));
#else
		    pline("%sはあなたの%s%sに鼻をすりよせた！", Monnam(mtmp),
			  sidestr, body_part(LEG));
#endif
		    dmg = 0;
		  } else {
		    if (uarmf) {
			if (rn2(2) && (uarmf->otyp == LOW_BOOTS ||
					     uarmf->otyp == IRON_SHOES))
#if 0 /*JP*/
			    pline("%s pricks the exposed part of your %s %s!",
				Monnam(mtmp), sidestr, body_part(LEG));
#else
			    pline("%sはあなたの%s%sをちくりと刺した！",
				Monnam(mtmp), sidestr, body_part(LEG));
#endif
			else if (!rn2(5))
/*JP
			    pline("%s pricks through your %s boot!",
*/
			    pline("%sはあなたの%sの靴をちくりと刺した！", 
				Monnam(mtmp), sidestr);
			else {
/*JP
			    pline("%s scratches your %s boot!", Monnam(mtmp),
*/
			    pline("%sはあなたの%sの靴をひっかいた！", Monnam(mtmp),
				sidestr);
			    dmg = 0;
			    break;
			}
/*JP
		    } else pline("%s pricks your %s %s!", Monnam(mtmp),
*/
		    } else pline("%sはあなたの%s%sをちくりと刺した！", Monnam(mtmp),
			  sidestr, body_part(LEG));
		    set_wounded_legs(side, rnd(60-ACURR(A_DEX)));
		    exercise(A_STR, FALSE);
		    exercise(A_DEX, FALSE);
		  }
		  break;
		}
	    case AD_STON:	/* cockatrice */
		hitmsg(mtmp, mattk);
		if(!rn2(3)) {
		    if (mtmp->mcan) {
			if (flags.soundok)
/*JP
			    You_hear("a cough from %s!", mon_nam(mtmp));
*/
			    You_hear("%sがゴホッゴホッという音を聞いた！", mon_nam(mtmp));
		    } else {
			if (flags.soundok)
/*JP
			    You_hear("%s hissing!", s_suffix(mon_nam(mtmp)));
*/
			    You_hear("%sがシューッという声を聞いた！", mon_nam(mtmp));
			if(!rn2(10) ||
			    (flags.moonphase == NEW_MOON && !have_lizard())) {
 do_stone:
			    if (!Stoned && !Stone_resistance
				    && !(poly_when_stoned(youmonst.data) &&
					polymon(PM_STONE_GOLEM))) {
				Stoned = 5;
/*JP
				delayed_killer = mtmp->data->mname;
*/
				delayed_killer = jtrns_mon(mtmp->data->mname);
				if (mtmp->data->geno & G_UNIQ) {
				    if (!type_is_pname(mtmp->data)) {
					static char kbuf[BUFSZ];

					/* "the" buffer may be reallocated */
					Strcpy(kbuf, the(delayed_killer));
					delayed_killer = kbuf;
				    }
				    killer_format = KILLED_BY;
				} else killer_format = KILLED_BY_AN;
				return(1);
				/* You("turn to stone..."); */
				/* done_in_by(mtmp); */
			    }
			}
		    }
		}
		break;
	    case AD_STCK:
		hitmsg(mtmp, mattk);
		if (uncancelled && !u.ustuck && !sticks(youmonst.data))
			u.ustuck = mtmp;
		break;
	    case AD_WRAP:
		if ((!mtmp->mcan || u.ustuck == mtmp) && !sticks(youmonst.data)) {
		    if (!u.ustuck && !rn2(10)) {
			if (u_slip_free(mtmp, mattk)) {
			    dmg = 0;
			} else {
/*JP
			    pline("%s swings itself around you!",
*/
			    pline("%sはあなたに身体を絡みつかせた！",
				  Monnam(mtmp));
			    u.ustuck = mtmp;
			}
		    } else if(u.ustuck == mtmp) {
			if (is_pool(mtmp->mx,mtmp->my) && !Swimming
			    && !Amphibious) {
			    boolean moat =
				(levl[mtmp->mx][mtmp->my].typ != POOL) &&
				(levl[mtmp->mx][mtmp->my].typ != WATER) &&
				!Is_medusa_level(&u.uz) &&
				!Is_waterlevel(&u.uz);

/*JP
			    pline("%s drowns you...", Monnam(mtmp));
*/
			    pline("あなたは%sに絡みつかれて溺れた…", Monnam(mtmp));
			    killer_format = KILLED_BY_AN;
#if 0 /*JP*/
			    Sprintf(buf, "%s by %s",
				    moat ? "moat" : "pool of water",
				    an(mtmp->data->mname));
#else
			    Sprintf(buf, "%sの%sに絡みつかれて",
				    moat ? "堀" : "池",
				    jtrns_mon(mtmp->data->mname));
#endif
			    killer = buf;
			    done(DROWNING);
			} else if(mattk->aatyp == AT_HUGS)
/*JP
			    You("are being crushed.");
*/
			    You("押しつぶされた。");
		    } else {
			dmg = 0;
			if(flags.verbose)
/*JP
			    pline("%s brushes against your %s.", Monnam(mtmp),
*/
			    pline("%sはあなたの%sに触れた。", Monnam(mtmp),
				   body_part(LEG));
		    }
		} else dmg = 0;
		break;
	    case AD_WERE:
		hitmsg(mtmp, mattk);
		if (uncancelled && !rn2(4) && u.ulycn == NON_PM &&
			!Protection_from_shape_changers &&
			!defends(AD_WERE,uwep)) {
/*JP
		    You_feel("feverish.");
*/
		    You("熱があるように感じた。");
		    exercise(A_CON, FALSE);
		    u.ulycn = monsndx(mdat);
		}
		break;
	    case AD_SGLD:
		hitmsg(mtmp, mattk);
		if (youmonst.data->mlet == mdat->mlet) break;
		if(!mtmp->mcan) stealgold(mtmp);
		break;

	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU:
		if (is_animal(mtmp->data)) {
			hitmsg(mtmp, mattk);
			if (mtmp->mcan) break;
			/* Continue below */
		} else if (dmgtype(youmonst.data, AD_SEDU)
#ifdef SEDUCE
			|| dmgtype(youmonst.data, AD_SSEX)
#endif
						) {
#if 0 /*JP*/
			pline("%s %s.", Monnam(mtmp), mtmp->minvent ?
		    "brags about the goods some dungeon explorer provided" :
		    "makes some remarks about how difficult theft is lately");
#else
			pline("%sは%s。", Monnam(mtmp), mtmp->minvent ?
			"ある迷宮の冒険家が置いてった品物を自慢した" :
			"窃盗がいかに困難か淡々と述べた");
#endif
			if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			return 3;
		} else if (mtmp->mcan) {
		    if (!Blind) {
#if 0 /*JP*/
			pline("%s tries to %s you, but you seem %s.",
			    Adjmonnam(mtmp, "plain"),
			    flags.female ? "charm" : "seduce",
			    flags.female ? "unaffected" : "uninterested");
#else
			pline("%sはあなたを%sしようとした、しかしあなたは%s。",
			    Adjmonnam(mtmp, "地味な"),
			    flags.female ? "魅了" : "誘惑",
 			    flags.female ? "影響を受けない" : "興味がない");
#endif
		    }
		    if(rn2(3)) {
			if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			return 3;
		    }
		    break;
		}
		buf[0] = '\0';
		switch (steal(mtmp, buf)) {
		  case -1:
			return 2;
		  case 0:
			break;
		  default:
			if (!is_animal(mtmp->data) && !tele_restrict(mtmp))
			    (void) rloc(mtmp, FALSE);
			if (is_animal(mtmp->data) && *buf) {
			    if (canseemon(mtmp))
#if 0 /*JP*/
				pline("%s tries to %s away with %s.",
				      Monnam(mtmp),
				      locomotion(mtmp->data, "run"),
				      buf);
#else
				pline("%sは%sを持って逃げ出した。",
				      Monnam(mtmp),
				      buf);
#endif
			}
			monflee(mtmp, 0, FALSE, FALSE);
			return 3;
		}
		break;
#ifdef SEDUCE
	    case AD_SSEX:
		if(could_seduce(mtmp, &youmonst, mattk) == 1
			&& !mtmp->mcan)
		    if (doseduce(mtmp))
			return 3;
		break;
#endif
	    case AD_SAMU:
		hitmsg(mtmp, mattk);
		/* when the Wiz hits, 1/20 steals the amulet */
		if (u.uhave.amulet ||
		     u.uhave.bell || u.uhave.book || u.uhave.menorah
		     || u.uhave.questart) /* carrying the Quest Artifact */
		    if (!rn2(20)) stealamulet(mtmp);
		break;

	    case AD_TLPT:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    if(flags.verbose)
/*JP
			Your("position suddenly seems very uncertain!");
*/
			pline("自分のいる位置が突然不明確になった！");
		    tele();
		}
		break;
	    case AD_RUST:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
		if (u.umonnum == PM_IRON_GOLEM) {
/*JP
			You("rust!");
*/
			You("錆びついた！");
			/* KMH -- this is okay with unchanging */
			rehumanize();
			break;
		}
		hurtarmor(AD_RUST);
		break;
	    case AD_CORR:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
		hurtarmor(AD_CORR);
		break;
	    case AD_DCAY:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
		if (u.umonnum == PM_WOOD_GOLEM ||
		    u.umonnum == PM_LEATHER_GOLEM) {
/*JP
			You("rot!");
*/
			You("腐った！");
			/* KMH -- this is okay with unchanging */
			rehumanize();
			break;
		}
		hurtarmor(AD_DCAY);
		break;
	    case AD_HEAL:
		/* a cancelled nurse is just an ordinary monster */
		if (mtmp->mcan) {
		    hitmsg(mtmp, mattk);
		    break;
		}
		if(!uwep
#ifdef TOURIST
		   && !uarmu
#endif
		   && !uarm && !uarmh && !uarms && !uarmg && !uarmc && !uarmf) {
		    boolean goaway = FALSE;
/*JP
		    pline("%s hits!  (I hope you don't mind.)", Monnam(mtmp));
*/
		    pline("%sの攻撃は命中した。(気にしないように。)", Monnam(mtmp));
		    if (Upolyd) {
			u.mh += rnd(7);
			if (!rn2(7)) {
			    /* no upper limit necessary; effect is temporary */
			    u.mhmax++;
			    if (!rn2(13)) goaway = TRUE;
			}
			if (u.mh > u.mhmax) u.mh = u.mhmax;
		    } else {
			u.uhp += rnd(7);
			if (!rn2(7)) {
			    /* hard upper limit via nurse care: 25 * ulevel */
			    if (u.uhpmax < 5 * u.ulevel + d(2 * u.ulevel, 10))
				u.uhpmax++;
			    if (!rn2(13)) goaway = TRUE;
			}
			if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
		    }
		    if (!rn2(3)) exercise(A_STR, TRUE);
		    if (!rn2(3)) exercise(A_CON, TRUE);
		    if (Sick) make_sick(0L, (char *) 0, FALSE, SICK_ALL);
		    flags.botl = 1;
		    if (goaway) {
			mongone(mtmp);
			return 2;
		    } else if (!rn2(33)) {
			if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			monflee(mtmp, d(3, 6), TRUE, FALSE);
			return 3;
		    }
		    dmg = 0;
		} else {
		    if (Role_if(PM_HEALER)) {
			if (flags.soundok && !(moves % 5))
/*JP
		      verbalize("Doc, I can't help you unless you cooperate.");
*/
		      verbalize("ドクター！協力をおねがいしますわ。");
			dmg = 0;
		    } else hitmsg(mtmp, mattk);
		}
		break;
	    case AD_CURS:
		hitmsg(mtmp, mattk);
		if(!night() && mdat == &mons[PM_GREMLIN]) break;
		if(!mtmp->mcan && !rn2(10)) {
		    if (flags.soundok) {
#if 0 /*JP*/
			if (Blind) You_hear("laughter.");
			else       pline("%s chuckles.", Monnam(mtmp));
#else
			if (Blind) You_hear("笑い声を聞いた。");
			else       pline("%sはクスクス笑った。", Monnam(mtmp));
#endif
		    }
		    if (u.umonnum == PM_CLAY_GOLEM) {
/*JP
			pline("Some writing vanishes from your head!");
*/
			pline("いくつかの文字があなたの頭から消えた！");
			/* KMH -- this is okay with unchanging */
			rehumanize();
			break;
		    }
		    attrcurse();
		}
		break;
	    case AD_STUN:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4)) {
		    make_stunned(HStun + dmg, TRUE);
		    dmg /= 2;
		}
		break;
	    case AD_ACID:
		hitmsg(mtmp, mattk);
		if (!mtmp->mcan && !rn2(3))
		    if (Acid_resistance) {
/*JP
			pline("You're covered in acid, but it seems harmless.");
*/
			pline("酸で覆われた。しかし傷つかない。");
			dmg = 0;
		    } else {
/*JP
			pline("You're covered in acid!	It burns!");
*/
			You("酸で覆われ焼けた！");
			exercise(A_STR, FALSE);
		    }
		else		dmg = 0;
		break;
	    case AD_SLOW:
		hitmsg(mtmp, mattk);
		if (uncancelled && HFast &&
					!defends(AD_SLOW, uwep) && !rn2(4))
		    u_slow_down();
		break;
	    case AD_DREN:
		hitmsg(mtmp, mattk);
		if (uncancelled && !rn2(4))
		    drain_en(dmg);
		dmg = 0;
		break;
	    case AD_CONF:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
		    mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
		    if(Confusion)
/*JP
			 You("are getting even more confused.");
*/
			 You("ますます混乱した。");
/*JP
		    else You("are getting confused.");
*/
		    else You("混乱してきた。");
		    make_confused(HConfusion + dmg, FALSE);
		}
		dmg = 0;
		break;
	    case AD_DETH:
/*JP
		pline("%s reaches out with its deadly touch.", Monnam(mtmp));
*/
		pline("%sは死の腕をのばした。", Monnam(mtmp));
		if (is_undead(youmonst.data)) {
		    /* Still does normal damage */
/*JP
		    pline("Was that the touch of death?");
*/
		    pline("今のは死の宣告だったのかな？");
		    break;
		}
		switch (rn2(20)) {
		case 19: case 18: case 17:
		    if (!Antimagic) {
			killer_format = KILLED_BY_AN;
/*JP
			killer = "touch of death";
*/
			killer = "死の宣告で";
			done(DIED);
			dmg = 0;
			break;
		    } /* else FALLTHRU */
		default: /* case 16: ... case 5: */
/*JP
		    You_feel("your life force draining away...");
*/
		    You("体力が奪われていくように感じた…");
		    permdmg = 1;	/* actual damage done below */
		    break;
		case 4: case 3: case 2: case 1: case 0:
		    if (Antimagic) shieldeff(u.ux, u.uy);
/*JP
		    pline("Lucky for you, it didn't work!");
*/
		    pline("運のよいことになんともなかった！");
		    dmg = 0;
		    break;
		}
		break;
	    case AD_PEST:
/*JP
		pline("%s reaches out, and you feel fever and chills.",
*/
		pline("%sは腕をのばした、あなたは悪寒を感じた。",
			Monnam(mtmp));
		(void) diseasemu(mdat); /* plus the normal damage */
		break;
	    case AD_FAMN:
/*JP
		pline("%s reaches out, and your body shrivels.",
*/
		pline("%sは腕を伸ばした、あなたの体はしなびた。",
			Monnam(mtmp));
		exercise(A_CON, FALSE);
		if (!is_fainted()) morehungry(rn1(40,40));
		/* plus the normal damage */
		break;
	    case AD_SLIM:    
		hitmsg(mtmp, mattk);
		if (!uncancelled) break;
		if (flaming(youmonst.data)) {
/*JP
		    pline_The("slime burns away!");
*/
		    pline_The("スライムは燃えた！");
		    dmg = 0;
		} else if (Unchanging ||
				youmonst.data == &mons[PM_GREEN_SLIME]) {
/*JP
		    You("are unaffected.");
*/
		    You("影響を受けない。");
		    dmg = 0;
		} else if (!Slimed) {
/*JP
		    You("don't feel very well.");
*/
		    You("気分が悪い。");
		    Slimed = 10L;
		    flags.botl = 1;
		    killer_format = KILLED_BY_AN;
/*JP
		    delayed_killer = mtmp->data->mname;
*/
		    delayed_killer = jtrns_mon(mtmp->data->mname);
		} else
/*JP
		    pline("Yuck!");
*/
		    pline("ウゲェー！");
		break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		hitmsg(mtmp, mattk);
		/* uncancelled is sufficient enough; please
		   don't make this attack less frequent */
		if (uncancelled) {
		    struct obj *obj = some_armor(&youmonst);

		    if (drain_item(obj)) {
/*JP
			Your("%s less effective.", aobjnam(obj, "seem"));
*/
			Your("%sから魔力が消えた。", xname(obj));
		    }
		}
		break;
	    default:	dmg = 0;
			break;
	}
	if(u.uhp < 1) done_in_by(mtmp);

/*	Negative armor class reduces damage done instead of fully protecting
 *	against hits.
 */
	if (dmg && u.uac < 0) {
		dmg -= rnd(-u.uac);
		if (dmg < 1) dmg = 1;
	}

	if(dmg) {
	    if (Half_physical_damage
					/* Mitre of Holiness */
		|| (Role_if(PM_PRIEST) && uarmh && is_quest_artifact(uarmh) &&
		    (is_undead(mtmp->data) || is_demon(mtmp->data))))
		dmg = (dmg+1) / 2;

	    if (permdmg) {	/* Death's life force drain */
		int lowerlimit, *hpmax_p;
		/*
		 * Apply some of the damage to permanent hit points:
		 *	polymorphed	    100% against poly'd hpmax
		 *	hpmax > 25*lvl	    100% against normal hpmax
		 *	hpmax > 10*lvl	50..100%
		 *	hpmax >  5*lvl	25..75%
		 *	otherwise	 0..50%
		 * Never reduces hpmax below 1 hit point per level.
		 */
		permdmg = rn2(dmg / 2 + 1);
		if (Upolyd || u.uhpmax > 25 * u.ulevel) permdmg = dmg;
		else if (u.uhpmax > 10 * u.ulevel) permdmg += dmg / 2;
		else if (u.uhpmax > 5 * u.ulevel) permdmg += dmg / 4;

		if (Upolyd) {
		    hpmax_p = &u.mhmax;
		    /* [can't use youmonst.m_lev] */
		    lowerlimit = min((int)youmonst.data->mlevel, u.ulevel);
		} else {
		    hpmax_p = &u.uhpmax;
		    lowerlimit = u.ulevel;
		}
		if (*hpmax_p - permdmg > lowerlimit)
		    *hpmax_p -= permdmg;
		else if (*hpmax_p > lowerlimit)
		    *hpmax_p = lowerlimit;
#ifndef GCC_WARN
		else	/* unlikely... */
		    ;	/* already at or below minimum threshold; do nothing */
#endif
		flags.botl = 1;
	    }

	    mdamageu(mtmp, dmg);
	}

	if (dmg)
	    res = passiveum(olduasmon, mtmp, mattk);
	else
	    res = 1;
	stop_occupation();
	return res;
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL int
gulpmu(mtmp, mattk)	/* monster swallows you, or damage if u.uswallow */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	struct trap *t = t_at(u.ux, u.uy);
	int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int	tim_tmp;
	register struct obj *otmp2;
	int	i;

	if (!u.uswallow) {	/* swallows you */
		if (youmonst.data->msize >= MZ_HUGE) return(0);
		if ((t && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT))) &&
		    sobj_at(BOULDER, u.ux, u.uy))
			return(0);

		if (Punished) unplacebc();	/* ball&chain go away */
		remove_monster(mtmp->mx, mtmp->my);
		mtmp->mtrapped = 0;		/* no longer on old trap */
		place_monster(mtmp, u.ux, u.uy);
		u.ustuck = mtmp;
		newsym(mtmp->mx,mtmp->my);
#ifdef STEED
		if (is_animal(mtmp->data) && u.usteed) {
			char buf[BUFSZ];
			/* Too many quirks presently if hero and steed
			 * are swallowed. Pretend purple worms don't
			 * like horses for now :-)
			 */
			Strcpy(buf, mon_nam(u.usteed));
/*JP
			pline ("%s lunges forward and plucks you off %s!",
*/
			pline ("%sは突進しあなたを%sから引き落した！",
				Monnam(mtmp), buf);
			dismount_steed(DISMOUNT_ENGULFED);
		} else
#endif
/*JP
		pline("%s engulfs you!", Monnam(mtmp));
*/
		pline("%sはあなたを飲みこんだ！", Monnam(mtmp));
		stop_occupation();
		reset_occupations();	/* behave as if you had moved */

		if (u.utrap) {
#if 0 /*JP*/
			You("are released from the %s!",
				u.utraptype==TT_WEB ? "web" : "trap");
#else
			You("%sから解放された！",
				u.utraptype==TT_WEB ? "くもの巣" : "罠");
#endif
			u.utrap = 0;
		}

		i = number_leashed();
		if (i > 0) {
#if 0 /*JP*/
		    const char *s = (i > 1) ? "leashes" : "leash";
		    pline_The("%s %s loose.", s, vtense(s, "snap"));
#else
		    pline("紐はパチンと切れた。");
#endif
		    unleash_all();
		}

		if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
			minstapetrify(mtmp, TRUE);
			if (mtmp->mhp > 0) return 0;
			else return 2;
		}

		display_nhwindow(WIN_MESSAGE, FALSE);
		vision_recalc(2);	/* hero can't see anything */
		u.uswallow = 1;
		/* u.uswldtim always set > 1 */
		tim_tmp = 25 - (int)mtmp->m_lev;
		if (tim_tmp > 0) tim_tmp = rnd(tim_tmp) / 2;
		else if (tim_tmp < 0) tim_tmp = -(rnd(-tim_tmp) / 2);
		tim_tmp += -u.uac + 10;
		u.uswldtim = (unsigned)((tim_tmp < 2) ? 2 : tim_tmp);
		swallowed(1);
		for (otmp2 = invent; otmp2; otmp2 = otmp2->nobj)
		    (void) snuff_lit(otmp2);
	}

	if (mtmp != u.ustuck) return(0);
	if (u.uswldtim > 0) u.uswldtim -= 1;

	switch(mattk->adtyp) {

		case AD_DGST:
		    if (Slow_digestion) {
			/* Messages are handled below */
			u.uswldtim = 0;
			tmp = 0;
		    } else if (u.uswldtim == 0) {
/*JP
			pline("%s totally digests you!", Monnam(mtmp));
*/
			pline("%sはあなたを完全に消化した！", Monnam(mtmp));
			tmp = u.uhp;
			if (Half_physical_damage) tmp *= 2; /* sorry */
		    } else {
#if 0 /*JP*/
			pline("%s%s digests you!", Monnam(mtmp),
			      (u.uswldtim == 2) ? " thoroughly" :
			      (u.uswldtim == 1) ? " utterly" : "");
#else
			pline("%sはあなたを%s消化している！", Monnam(mtmp),
			      (u.uswldtim == 2) ? "徹底的に" :
			      (u.uswldtim == 1) ? "完全に" : "");
#endif
			exercise(A_STR, FALSE);
		    }
		    break;
		case AD_PHYS:
		    if (mtmp->data == &mons[PM_FOG_CLOUD]) {
#if 0 /*JP:T*/
			You("are laden with moisture and %s",
			    flaming(youmonst.data) ? "are smoldering out!" :
			    Breathless ? "find it mildly uncomfortable." :
			    amphibious(youmonst.data) ? "feel comforted." :
			    "can barely breathe!");
#else
			You("湿気に苦しめられ、%s",
			    flaming(youmonst.data) ? "くすぶった！" :
			    Breathless ? "少々不快に感じた。" :
			    amphibious(youmonst.data) ? "安らぎを感じた。" :
			    "息をするのがやっとだ！");
#endif
			/* NB: Amphibious includes Breathless */
			if (Amphibious && !flaming(youmonst.data)) tmp = 0;
		    } else {
/*JP
			You("are pummeled with debris!");
*/
		    You("瓦礫で痛めつけられた！");
			exercise(A_STR, FALSE);
		    }
		    break;
		case AD_ACID:
		    if (Acid_resistance) {
/*JP
			You("are covered with a seemingly harmless goo.");
*/
			You("ねばつくものでおおわれた。");
			tmp = 0;
		    } else {
/*JP
		      if (Hallucination) pline("Ouch!  You've been slimed!");
*/
		      if (Hallucination) pline("げげん！あなたはぬるぬるだ！");
/*JP
		      else You("are covered in slime!  It burns!");
*/
		      else You("べとべとになった。そして酸に焼かれた！");
		      exercise(A_STR, FALSE);
		    }
		    break;
		case AD_BLND:
		    if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj*)0)) {
			if(!Blind) {
/*JP
			    You_cant("see in here!");
*/
			    You("何も見えない！");
			    make_blinded((long)tmp,FALSE);
			    if (!Blind) Your(vision_clears);
			} else
			    /* keep him blind until disgorged */
			    make_blinded(Blinded+1,FALSE);
		    }
		    tmp = 0;
		    break;
		case AD_ELEC:
		    if(!mtmp->mcan && rn2(2)) {
/*JP
			pline_The("air around you crackles with electricity.");
*/
			pline("あなたの回りの空気は電気でピリピリしている。");
			if (Shock_resistance) {
				shieldeff(u.ux, u.uy);
/*JP
				You("seem unhurt.");
*/
				You("傷つかないようだ。");
				ugolemeffects(AD_ELEC,tmp);
				tmp = 0;
			}
		    } else tmp = 0;
		    break;
		case AD_COLD:
		    if(!mtmp->mcan && rn2(2)) {
			if (Cold_resistance) {
				shieldeff(u.ux, u.uy);
/*JP
				You_feel("mildly chilly.");
*/
				pline("ひんやりした。");
				ugolemeffects(AD_COLD,tmp);
				tmp = 0;
/*JP
			} else You("are freezing to death!");
*/
			} else You("凍死しそうだ！");
		    } else tmp = 0;
		    break;
		case AD_FIRE:
		    if(!mtmp->mcan && rn2(2)) {
			if (Fire_resistance) {
				shieldeff(u.ux, u.uy);
/*JP
				You_feel("mildly hot.");
*/
				pline("ポカポカした。");
				ugolemeffects(AD_FIRE,tmp);
				tmp = 0;
/*JP
			} else You("are burning to a crisp!");
*/
			} else You("燃えてカラカラになった！");
			burn_away_slime();
		    } else tmp = 0;
		    break;
		case AD_DISE:
		    if (!diseasemu(mtmp->data)) tmp = 0;
		    break;
		default:
		    tmp = 0;
		    break;
	}

	if (Half_physical_damage) tmp = (tmp+1) / 2;

	mdamageu(mtmp, tmp);
	if (tmp) stop_occupation();

	if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
#if 0 /*JP*/
	    pline("%s very hurriedly %s you!", Monnam(mtmp),
		  is_animal(mtmp->data)? "regurgitates" : "expels");
#else
		pline("%sは慌ててあなたを%sした！", Monnam(mtmp),
		       is_animal(mtmp->data)? "吐き戻" : "排出");
#endif
	    expels(mtmp, mtmp->data, FALSE);
	} else if (!u.uswldtim || youmonst.data->msize >= MZ_HUGE) {
/*JP
	    You("get %s!", is_animal(mtmp->data)? "regurgitated" : "expelled");
*/
	    You("%sされた！", is_animal(mtmp->data)? "吐き戻" : "排出");
	    if (flags.verbose && (is_animal(mtmp->data) ||
		    (dmgtype(mtmp->data, AD_DGST) && Slow_digestion)))
/*JP
		pline("Obviously %s doesn't like your taste.", mon_nam(mtmp));
*/
		You("どうも%s好みの味じゃないようだ。", mon_nam(mtmp));
	    expels(mtmp, mtmp->data, FALSE);
	}
	return(1);
}

STATIC_OVL int
explmu(mtmp, mattk, ufound)	/* monster explodes in your face */
register struct monst *mtmp;
register struct attack  *mattk;
boolean ufound;
{
    if (mtmp->mcan) return(0);

    if (!ufound)
#if 0 /*JP*/
	pline("%s explodes at a spot in %s!",
	    canseemon(mtmp) ? Monnam(mtmp) : "It",
	    levl[mtmp->mux][mtmp->muy].typ == WATER
		? "empty water" : "thin air");
#else
	pline("%sは何もない%sで爆発した！",
	      canseemon(mtmp) ? Monnam(mtmp) : "何者か",
	      levl[mtmp->mux][mtmp->muy].typ == WATER ?
	      "水中" : "空間");
#endif
    else {
	register int tmp = d((int)mattk->damn, (int)mattk->damd);
	register boolean not_affected = defends((int)mattk->adtyp, uwep);

	hitmsg(mtmp, mattk);

	switch (mattk->adtyp) {
	    case AD_COLD:
		not_affected |= Cold_resistance;
		goto common;
	    case AD_FIRE:
		not_affected |= Fire_resistance;
		goto common;
	    case AD_ELEC:
		not_affected |= Shock_resistance;
common:

		if (!not_affected) {
		    if (ACURR(A_DEX) > rnd(20)) {
/*JP
			You("duck some of the blast.");
*/
			You("衝撃を避けきれなかった。");
			tmp = (tmp+1) / 2;
		    } else {
/*JP
		        if (flags.verbose) You("get blasted!");
*/
		        if (flags.verbose) You("衝撃をくらった！");
		    }
		    if (mattk->adtyp == AD_FIRE) burn_away_slime();
		    if (Half_physical_damage) tmp = (tmp+1) / 2;
		    mdamageu(mtmp, tmp);
		}
		break;

	    case AD_BLND:
		not_affected = resists_blnd(&youmonst);
		if (!not_affected) {
		    /* sometimes you're affected even if it's invisible */
		    if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
/*JP
			You("are blinded by a blast of light!");
*/
			You("まばゆい光に目がくらんだ！");
			make_blinded((long)tmp, FALSE);
			if (!Blind) Your(vision_clears);
		    } else if (flags.verbose)
/*JP
			You("get the impression it was not terribly bright.");
*/
			You("それは恐ろしくまぶしいというほどじゃないと思った。");
		}
		break;

	    case AD_HALU:
		not_affected |= Blind ||
			(u.umonnum == PM_BLACK_LIGHT ||
			 u.umonnum == PM_VIOLET_FUNGUS ||
			 dmgtype(youmonst.data, AD_STUN));
		if (!not_affected) {
		    boolean chg;
		    if (!Hallucination)
/*JP
			You("are caught in a blast of kaleidoscopic light!");
*/
			You("万華鏡の光に捕らえられた！");
		    chg = make_hallucinated(HHallucination + (long)tmp,FALSE,0L);
/*JP
		    You("%s.", chg ? "are freaked out" : "seem unaffected");
*/
		    You("%s。", chg ? "酔いしれた" : "影響を受けなかった");
		}
		break;

	    default:
		break;
	}
	if (not_affected) {
/*JP
	    You("seem unaffected by it.");
*/
	    You("影響を受けないようだ。");
	    ugolemeffects((int)mattk->adtyp, tmp);
	}
    }
    mondead(mtmp);
    wake_nearto(mtmp->mx, mtmp->my, 7*7);
    if (mtmp->mhp > 0) return(0);
    return(2);	/* it dies */
}

int
gazemu(mtmp, mattk)	/* monster gazes at you */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
#if 1 /*JP*/
	static char jbuf[BUFSZ];
#endif
	switch(mattk->adtyp) {
	    case AD_STON:
		if (mtmp->mcan || !mtmp->mcansee) {
		    if (!canseemon(mtmp)) break;	/* silently */
#if 0 /*JP:T*/
		    pline("%s %s.", Monnam(mtmp),
			  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan) ?
				"doesn't look all that ugly" :
				"gazes ineffectually");
#else
		    pline("%sは%s。", Monnam(mtmp),
			  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan) ?
				"それほど醜くないことに気がついた" :
				"無意味ににらんだ");
#endif
		    break;
		}
		if (Reflecting && couldsee(mtmp->mx, mtmp->my) &&
			mtmp->data == &mons[PM_MEDUSA]) {
		    /* hero has line of sight to Medusa and she's not blind */
		    boolean useeit = canseemon(mtmp);

		    if (useeit)
/*JP
			(void) ureflects("%s gaze is reflected by your %s.",
*/
		    	(void) ureflects("%sのにらみは%sで反射された。",
					 s_suffix(Monnam(mtmp)));
		    if (mon_reflects(mtmp, !useeit ? (char *)0 :
/*JP
				     "The gaze is reflected away by %s %s!"))
*/
		    			"にらみは%sの%sによって反射された！"))
			break;
		    if (!m_canseeu(mtmp)) { /* probably you're invisible */
			if (useeit)
#if 0 /*JP:T*/
			    pline(
		      "%s doesn't seem to notice that %s gaze was reflected.",
				  Monnam(mtmp), mhis(mtmp));
#else
			    pline("にらみが反射していることに%sは気付いていないようだ。",
				Monnam(mtmp));
#endif
			break;
		    }
		    if (useeit)
/*JP
			pline("%s is turned to stone!", Monnam(mtmp));
*/
			pline("%sは石になった！", Monnam(mtmp));
		    stoned = TRUE;
		    killed(mtmp);

		    if (mtmp->mhp > 0) break;
		    return 2;
		}
		if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) &&
		    !Stone_resistance) {
/*JP
		    You("meet %s gaze.", s_suffix(mon_nam(mtmp)));
*/
		    You("%sににらまれた。", s_suffix(mon_nam(mtmp)));
		    stop_occupation();
		    if(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
			break;
/*JP
		    You("turn to stone...");
*/
		    You("石になった…");
		    killer_format = KILLED_BY;
#if 0 /*JP*/
		    killer = mtmp->data->mname;
#else
		    Sprintf(jbuf, "%sのにらみで", jtrns_mon(mons[PM_MEDUSA].mname));
		    killer = jbuf;
#endif
		    done(STONING);
		}
		break;
	    case AD_CONF:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) &&
		   mtmp->mcansee && !mtmp->mspec_used && rn2(5)) {
		    int conf = d(3,4);

		    mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
		    if(!Confusion)
/*JP
			pline("%s gaze confuses you!",
*/
			pline("%sのにらみであなたは混乱した！",
			                  s_suffix(Monnam(mtmp)));
		    else
/*JP
			You("are getting more and more confused.");
*/
			You("ますます混乱した。");
		    make_confused(HConfusion + conf, FALSE);
		    stop_occupation();
		}
		break;
	    case AD_STUN:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) &&
		   mtmp->mcansee && !mtmp->mspec_used && rn2(5)) {
		    int stun = d(2,6);

		    mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
/*JP
		    pline("%s stares piercingly at you!", Monnam(mtmp));
*/
		    pline("%sは冷たいまなざしをあなたに向けた！", Monnam(mtmp));
		    make_stunned(HStun + stun, TRUE);
		    stop_occupation();
		}
		break;
	    case AD_BLND:
		if (!mtmp->mcan && canseemon(mtmp) && !resists_blnd(&youmonst)
			&& distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) {
		    int blnd = d((int)mattk->damn, (int)mattk->damd);

/*JP
		    You("are blinded by %s radiance!",
*/
		    You("%sの光で目が見えなくなった！",
			              s_suffix(mon_nam(mtmp)));
		    make_blinded((long)blnd,FALSE);
		    stop_occupation();
		    /* not blind at this point implies you're wearing
		       the Eyes of the Overworld; make them block this
		       particular stun attack too */
		    if (!Blind) Your(vision_clears);
		    else make_stunned((long)d(1,3),TRUE);
		}
		break;
	    case AD_FIRE:
		if (!mtmp->mcan && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			mtmp->mcansee && !mtmp->mspec_used && rn2(5)) {
		    int dmg = d(2,6);

/*JP
		    pline("%s attacks you with a fiery gaze!", Monnam(mtmp));
*/
		    pline("%sは炎のにらみ攻撃をしてきた！", Monnam(mtmp));
		    stop_occupation();
		    if (Fire_resistance) {
/*JP
			pline_The("fire doesn't feel hot!");
*/
			pline("火はぜんぜん熱くない！");
			dmg = 0;
		    }
		    burn_away_slime();
		    if ((int) mtmp->m_lev > rn2(20))
			destroy_item(SCROLL_CLASS, AD_FIRE);
		    if ((int) mtmp->m_lev > rn2(20))
			destroy_item(POTION_CLASS, AD_FIRE);
		    if ((int) mtmp->m_lev > rn2(25))
			destroy_item(SPBOOK_CLASS, AD_FIRE);
		    if (dmg) mdamageu(mtmp, dmg);
		}
		break;
#ifdef PM_BEHOLDER /* work in progress */
	    case AD_SLEE:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee &&
		   multi >= 0 && !rn2(5) && !Sleep_resistance) {

		    fall_asleep(-rnd(10), TRUE);
/*JP
		    pline("%s gaze makes you very sleepy...",
*/
		    pline("%sのにらみであなたは眠くなった…",
			  s_suffix(Monnam(mtmp)));
		}
		break;
	    case AD_SLOW:
		if(!mtmp->mcan && canseemon(mtmp) && mtmp->mcansee &&
		   (HFast & (INTRINSIC|TIMEOUT)) &&
		   !defends(AD_SLOW, uwep) && !rn2(4))

		    u_slow_down();
		    stop_occupation();
		break;
#endif
	    default: impossible("Gaze attack %d?", mattk->adtyp);
		break;
	}
	return(0);
}

#endif /* OVLB */
#ifdef OVL1

void
mdamageu(mtmp, n)	/* mtmp hits you for n points damage */
register struct monst *mtmp;
register int n;
{
	flags.botl = 1;
	if (Upolyd) {
		u.mh -= n;
		if (u.mh < 1) rehumanize();
	} else {
		u.uhp -= n;
		if(u.uhp < 1) done_in_by(mtmp);
	}
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL void
urustm(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	boolean vis;
	boolean is_acid;

	if (!mon || !obj) return; /* just in case */
	if (dmgtype(youmonst.data, AD_CORR))
	    is_acid = TRUE;
	else if (dmgtype(youmonst.data, AD_RUST))
	    is_acid = FALSE;
	else
	    return;

	vis = cansee(mon->mx, mon->my);

	if ((is_acid ? is_corrodeable(obj) : is_rustprone(obj)) &&
	    (is_acid ? obj->oeroded2 : obj->oeroded) < MAX_ERODE) {
		if (obj->greased || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (vis)
/*JP
			pline("Somehow, %s weapon is not affected.",
*/
			pline("どういうわけか、%sの武器は影響を受けない。",
						s_suffix(mon_nam(mon)));
		    if (obj->greased && !rn2(2)) obj->greased = 0;
		} else {
		    if (vis)
#if 0 /*JP*/
			pline("%s %s%s!",
			        s_suffix(Monnam(mon)),
				aobjnam(obj, (is_acid ? "corrode" : "rust")),
			        (is_acid ? obj->oeroded2 : obj->oeroded)
				    ? " further" : "");
#else
			pline("%sの%sは%s%s！",
			        Monnam(mon), xname(obj),
			        obj->oeroded ? "さらに" : "",
				aobjnam(obj, (is_acid ? "腐食した" : "錆びた")));
#endif
		    if (is_acid) obj->oeroded2++;
		    else obj->oeroded++;
		}
	}
}

#endif /* OVLB */
#ifdef OVL1

int
could_seduce(magr,mdef,mattk)
struct monst *magr, *mdef;
struct attack *mattk;
/* returns 0 if seduction impossible,
 *	   1 if fine,
 *	   2 if wrong gender for nymph */
{
	register struct permonst *pagr;
	boolean agrinvis, defperc;
	xchar genagr, gendef;

	if (is_animal(magr->data)) return (0);
	if(magr == &youmonst) {
		pagr = youmonst.data;
		agrinvis = (Invis != 0);
		genagr = poly_gender();
	} else {
		pagr = magr->data;
		agrinvis = magr->minvis;
		genagr = gender(magr);
	}
	if(mdef == &youmonst) {
		defperc = (See_invisible != 0);
		gendef = poly_gender();
	} else {
		defperc = perceives(mdef->data);
		gendef = gender(mdef);
	}

	if(agrinvis && !defperc
#ifdef SEDUCE
		&& mattk && mattk->adtyp != AD_SSEX
#endif
		)
		return 0;

	if(pagr->mlet != S_NYMPH
		&& ((pagr != &mons[PM_INCUBUS] && pagr != &mons[PM_SUCCUBUS])
#ifdef SEDUCE
		    || (mattk && mattk->adtyp != AD_SSEX)
#endif
		   ))
		return 0;
	
	if(genagr == 1 - gendef)
		return 1;
	else
		return (pagr->mlet == S_NYMPH) ? 2 : 0;
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef SEDUCE
/* Returns 1 if monster teleported */
int
doseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = (mon->data == &mons[PM_SUCCUBUS]); /* otherwise incubus */
	char qbuf[QBUFSZ];

	if (mon->mcan || mon->mspec_used) {
#if 0 /*JP*/
		pline("%s acts as though %s has got a %sheadache.",
		      Monnam(mon), mhe(mon),
		      mon->mcan ? "severe " : "");
#else
	  pline("%sは%s頭が痛いふりをした。",
		      Monnam(mon),
		      mon->mcan ? "ひどく" : "");
#endif
		return 0;
	}

	if (unconscious()) {
/*JP
		pline("%s seems dismayed at your lack of response.",
*/
		pline("%sは返事がないので気が萎えたようだ。",
		      Monnam(mon));
		return 0;
	}

/*JP
	if (Blind) pline("It caresses you...");
*/
	if (Blind) pline("何者かがあなたを抱きしめている…");
/*JP
	else You_feel("very attracted to %s.", mon_nam(mon));
*/
	else You("%sに引きつけられてるように感じた。", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
	    nring = ring->nobj;
	    if (ring->otyp != RIN_ADORNMENT) continue;
	    if (fem) {
		if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
		    Sprintf(qbuf, "\"That %s looks pretty.  May I have it?\"",
			safe_qbuf("",sizeof("\"That  looks pretty.  May I have it?\""),
			xname(ring), simple_typename(ring->otyp), "ring"));
#else
		    Sprintf(qbuf, "「なんて素敵な%sでしょう。わたしにくれません？」",
			safe_qbuf("",sizeof("「なんて素敵なでしょう。わたしにくれません？」"),
			xname(ring), simple_typename(ring->otyp), "指輪"));
#endif
		    makeknown(RIN_ADORNMENT);
		    if (yn(qbuf) == 'n') continue;
#if 0 /*JP*/
		} else pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
#else
		} else pline("%sは%sがとても気にいって、それを取りあげた。",
			Blind ? "彼女" : Monnam(mon), xname(ring));
#endif
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
	    } else {
		char buf[BUFSZ];

		if (uleft && uright && uleft->otyp == RIN_ADORNMENT
				&& uright->otyp==RIN_ADORNMENT)
			break;
		if (ring==uleft || ring==uright) continue;
		if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
		    Sprintf(qbuf,"\"That %s looks pretty.  Would you wear it for me?\"",
			safe_qbuf("",
			    sizeof("\"That  looks pretty.  Would you wear it for me?\""),
			    xname(ring), simple_typename(ring->otyp), "ring"));
#else
		    Sprintf(qbuf,"「おやなんてすばらしい%sだ。私のために指にはめてくれないかい？」",
			safe_qbuf("",
			    sizeof("「おやなんてすばらしいだ。私のために指にはめてくれないかい？」"),
			    xname(ring), simple_typename(ring->otyp), "指輪"));
#endif
		    makeknown(RIN_ADORNMENT);
		    if (yn(qbuf) == 'n') continue;
		} else {
#if 0 /*JP*/
		    pline("%s decides you'd look prettier wearing your %s,",
			Blind ? "He" : Monnam(mon), xname(ring));
		    pline("and puts it on your finger.");
#else
		    pline("%sは%sをつけたあなたがより魅力的だと考え、",
			Blind ? "彼" : Monnam(mon), xname(ring));
		    pline("あなたの指にそれをはめた。");
#endif
		}
		makeknown(RIN_ADORNMENT);
		if (!uright) {
#if 0 /*JP*/
		    pline("%s puts %s on your right %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
#else
		    pline("%sは%sをあなたの右%sにはめた。",
			Blind ? "彼" : Monnam(mon), the(xname(ring)), body_part(HAND));
#endif
		    setworn(ring, RIGHT_RING);
		} else if (!uleft) {
#if 0 /*JP*/
		    pline("%s puts %s on your left %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
#else
		    pline("%sは%sをあなたの左%sにはめた。",
			Blind ? "彼" : Monnam(mon), the(xname(ring)), body_part(HAND));
#endif
		    setworn(ring, LEFT_RING);
		} else if (uright && uright->otyp != RIN_ADORNMENT) {
		    Strcpy(buf, xname(uright));
#if 0 /*JP*/
		    pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
#else
		    pline("%sは%sを%sにとりかえた。",
			Blind ? "彼" : Monnam(mon), buf, xname(ring));
#endif
		    Ring_gone(uright);
		    setworn(ring, RIGHT_RING);
		} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
		    Strcpy(buf, xname(uleft));
#if 0 /*JP*/
		    pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
#else
		    pline("%sは%sを%sにとりかえた。",
			Blind ? "彼" : Monnam(mon), buf, xname(ring));
#endif
		    Ring_gone(uleft);
		    setworn(ring, LEFT_RING);
		} else impossible("ring replacement");
		Ring_on(ring);
		prinv((char *)0, ring, 0L);
	    }
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
#if 0 /*JP*/
		pline("%s murmurs sweet nothings into your ear.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
#else
		pline("%sはあなたの耳もとで甘いささやきをつぶやいた。",
			Blind ? (fem ? "彼女" : "彼") : Monnam(mon));
#endif
	else
#if 0 /*JP*/
		pline("%s murmurs in your ear, while helping you undress.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
#else
		pline("%sは耳もとであなたの服を脱がせながらささやいた。",
			Blind ? (fem ? "彼女" : "彼") : Monnam(mon));
#endif
	mayberem(uarmc, cloak_simple_name(uarmc));
	if(!uarmc)
/*JP
		mayberem(uarm, "suit");
*/
		mayberem(uarm, "スーツ");
/*JP
	mayberem(uarmf, "boots");
*/
	mayberem(uarmf, "ブーツ");
	if(!uwep || !welded(uwep))
/*JP
		mayberem(uarmg, "gloves");
*/
		mayberem(uarmg, "籠手");
/*JP
	mayberem(uarms, "shield");
*/
	mayberem(uarms, "盾");
/*JP
	mayberem(uarmh, "helmet");
*/
	mayberem(uarmh, "兜");
#ifdef TOURIST
	if(!uarmc && !uarm)
/*JP
		mayberem(uarmu, "shirt");
*/
		mayberem(uarmu, "シャツ");
#endif

	if (uarm || uarmc) {
#if 0 /*JP*/
		verbalize("You're such a %s; I wish...",
				flags.female ? "sweet lady" : "nice guy");
#else
		verbalize("%s…だといいのに。",
				flags.female ? "チャーミング" : "すてき");
#endif
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	if (u.ualign.type == A_CHAOTIC)
		adjalign(1);

	/* by this point you have discovered mon's identity, blind or not... */
/*JP
	pline("Time stands still while you and %s lie in each other's arms...",
*/
	pline("あなたと%sが抱き合うと、時が止まったように感じた…",
		noit_mon_nam(mon));
	if (rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
		/* Don't bother with mspec_used here... it didn't get tired! */
/*JP
		pline("%s seems to have enjoyed it more than you...",
*/
		pline("%sはあなたより楽しんだようだ…",
			noit_Monnam(mon));
		switch (rn2(5)) {
/*JP
			case 0: You_feel("drained of energy.");
*/
			case 0: You("体力が消耗したように感じた。");
				u.uen = 0;
				u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
			        exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
/*JP
			case 1: You("are down in the dumps.");
*/
			case 1: You("意気消沈した。");
				(void) adjattrib(A_CON, -1, TRUE);
			        exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
/*JP
			case 2: Your("senses are dulled.");
*/
			case 2: Your("五感は鈍った。");
				(void) adjattrib(A_WIS, -1, TRUE);
			        exercise(A_WIS, FALSE);
				flags.botl = 1;
				break;
			case 3:
				if (!resists_drli(&youmonst)) {
/*JP
				    You_feel("out of shape.");
*/
				    You("くたびれた。");
/*JP
				    losexp("overexertion");
*/
				    losexp("過労で");
				} else {
/*JP
				    You("have a curious feeling...");
*/
				    You("変な感じがした…");
				}
				break;
			case 4: {
				int tmp;
/*JP
				You_feel("exhausted.");
*/
				You("疲れを感じた。");
			        exercise(A_STR, FALSE);
				tmp = rn1(10, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
/*JP
				losehp(tmp, "exhaustion", KILLED_BY);
*/
				losehp(tmp, "精力の使いすぎで", KILLED_BY);
				break;
			}
		}
	} else {
		mon->mspec_used = rnd(100); /* monster is worn out */
/*JP
		You("seem to have enjoyed it more than %s...",
*/
		You("%sよりも楽しんだようだ…",
		    noit_mon_nam(mon));
		switch (rn2(5)) {
/*JP
		case 0: You_feel("raised to your full potential.");
*/
		case 0: You("絶頂に達した！");
			exercise(A_CON, TRUE);
			u.uen = (u.uenmax += rnd(5));
			break;
/*JP
		case 1: You_feel("good enough to do it again.");
*/
		case 1: You("まだできると思った。");
			(void) adjattrib(A_CON, 1, TRUE);
			exercise(A_CON, TRUE);
			flags.botl = 1;
			break;
/*JP
		case 2: You("will always remember %s...", noit_mon_nam(mon));
*/
		case 2: You("いつまでも%sを覚えてるだろう…", mon_nam(mon));
			(void) adjattrib(A_WIS, 1, TRUE);
			exercise(A_WIS, TRUE);
			flags.botl = 1;
			break;
/*JP
		case 3: pline("That was a very educational experience.");
*/
		case 3: pline("とても貴重な経験だった。");
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			break;
/*JP
		case 4: You_feel("restored to health!");
*/
		case 4: You("とても健康になった！");
			u.uhp = u.uhpmax;
			if (Upolyd) u.mh = u.mhmax;
			exercise(A_STR, TRUE);
			flags.botl = 1;
			break;
		}
	}

	if (mon->mtame) /* don't charge */ ;
	else if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
		pline("%s demands that you pay %s, but you refuse...",
			noit_Monnam(mon),
			Blind ? (fem ? "her" : "him") : mhim(mon));
#else
		pline("%sはあなたに金を払うよう要求したが、あなたは拒んだ…",
			Monnam(mon));
#endif
	} else if (u.umonnum == PM_LEPRECHAUN)
#if 0 /*JP*/
		pline("%s tries to take your money, but fails...",
				noit_Monnam(mon));
#else
		pline("%sは金を取ろうとしたが、失敗した…",
				noit_Monnam(mon));
#endif
	else {
#ifndef GOLDOBJ
		long cost;

		if (u.ugold > (long)LARGEST_INT - 10L)
			cost = (long) rnd(LARGEST_INT) + 500L;
		else
			cost = (long) rnd((int)u.ugold + 10) + 500L;
		if (mon->mpeaceful) {
			cost /= 5L;
			if (!cost) cost = 1L;
		}
		if (cost > u.ugold) cost = u.ugold;
/*JP
		if (!cost) verbalize("It's on the house!");
*/
		if (!cost) verbalize("これはおごり%s！",fem ? "よ" : "さ");
		else {
#if 0 /*JP*/
		    pline("%s takes %ld %s for services rendered!",
#else
		    pline("%sはサービス料として%ld%s奪い取った！",
#endif
			    noit_Monnam(mon), cost, currency(cost));
		    u.ugold -= cost;
		    mon->mgold += cost;
		    flags.botl = 1;
		}
#else
		long cost;
                long umoney = money_cnt(invent);

		if (umoney > (long)LARGEST_INT - 10L)
			cost = (long) rnd(LARGEST_INT) + 500L;
		else
			cost = (long) rnd((int)umoney + 10) + 500L;
		if (mon->mpeaceful) {
			cost /= 5L;
			if (!cost) cost = 1L;
		}
		if (cost > umoney) cost = umoney;
/*JP
		if (!cost) verbalize("It's on the house!");
*/
		if (!cost) verbalize("これはおごり%s！", fem ? "よ" : "さ");
		else { 
#if 0 /*JP*/
		    pline("%s takes %ld %s for services rendered!",
#else
		    pline("%sはサービス料として%ld%s奪い取った！",
#endif
			    noit_Monnam(mon), cost, currency(cost));
                    money2mon(mon, cost);
		    flags.botl = 1;
		}
#endif
	}
	if (!rn2(25)) mon->mcan = 1; /* monster is worn out */
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

STATIC_OVL void
mayberem(obj, str)
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
		Sprintf(qbuf,"\"Shall I remove your %s, %s?\"",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
#else
		Sprintf(qbuf,"「%sを取っていい、%s？」",
			str,
			(!rn2(2) ? "ねぇ" : flags.female ? "ハニー" : "ダーリン" ));
#endif
		if (yn(qbuf) == 'n') return;
	} else {
		char hairbuf[BUFSZ];

#if 0 /*JP*/
		Sprintf(hairbuf, "let me run my fingers through your %s",
			body_part(HAIR));
#else
		Sprintf(hairbuf,
			flags.female ? "なんて綺麗な%sなんだ" : "兜を取ったらなかなかイカスじゃない",
			body_part(HAIR));
#endif
#if 0 /*JP*/
		verbalize("Take off your %s; %s.", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc || obj == uarms) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef TOURIST
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			hairbuf);
#else /*JP*/
		verbalize("%sを脱いで…%s。", str,
			(obj == uarm)  ? "もうちょっと寄りそって" :
			(obj == uarmc || obj == uarms) ? "そうそう" :
			(obj == uarmf) ?(flags.female ? "綺麗な足だね" : "うふっ、たくましい足ね") :
			(obj == uarmg) ?(flags.female ? "なんて素敵な手だ" : "たくましい腕ね") :
#ifdef TOURIST
			(obj == uarmu) ?(flags.female ? "きれいな体だ" : "たくましいのね") :
#endif
			/* obj == uarmh */
			hairbuf);
#endif /*JP*/
	}
	remove_worn_item(obj, TRUE);
}
#endif  /* SEDUCE */

#endif /* OVLB */

#ifdef OVL1

STATIC_OVL int
passiveum(olduasmon,mtmp,mattk)
struct permonst *olduasmon;
register struct monst *mtmp;
register struct attack *mattk;
{
	int i, tmp;

	for (i = 0; ; i++) {
	    if (i >= NATTK) return 1;
	    if (olduasmon->mattk[i].aatyp == AT_NONE ||
	    		olduasmon->mattk[i].aatyp == AT_BOOM) break;
	}
	if (olduasmon->mattk[i].damn)
	    tmp = d((int)olduasmon->mattk[i].damn,
				    (int)olduasmon->mattk[i].damd);
	else if(olduasmon->mattk[i].damd)
	    tmp = d((int)olduasmon->mlevel+1, (int)olduasmon->mattk[i].damd);
	else
	    tmp = 0;

	/* These affect the enemy even if you were "killed" (rehumanized) */
	switch(olduasmon->mattk[i].adtyp) {
	    case AD_ACID:
		if (!rn2(2)) {
/*JP
		    pline("%s is splashed by your acid!", Monnam(mtmp));
*/
		    pline("%sはあなたの酸をくらった！", Monnam(mtmp));
		    if (resists_acid(mtmp)) {
/*JP
			pline("%s is not affected.", Monnam(mtmp));
*/
			pline("%sは影響を受けない。", Monnam(mtmp));
			tmp = 0;
		    }
		} else tmp = 0;
		if (!rn2(30)) erode_armor(mtmp, TRUE);
		if (!rn2(6)) erode_obj(MON_WEP(mtmp), TRUE, TRUE);
		goto assess_dmg;
	    case AD_STON: /* cockatrice */
	    {
		long protector = attk_protection((int)mattk->aatyp),
		     wornitems = mtmp->misc_worn_check;

		/* wielded weapon gives same protection as gloves here */
		if (MON_WEP(mtmp) != 0) wornitems |= W_ARMG;

		if (!resists_ston(mtmp) && (protector == 0L ||
			(protector != ~0L &&
			    (wornitems & protector) != protector))) {
		    if (poly_when_stoned(mtmp->data)) {
			mon_to_stone(mtmp);
			return (1);
		    }
/*JP
		    pline("%s turns to stone!", Monnam(mtmp));
*/
		    pline("%sは石になった！", Monnam(mtmp));
		    stoned = 1;
		    xkilled(mtmp, 0);
		    if (mtmp->mhp > 0) return 1;
		    return 2;
		}
		return 1;
	    }
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
	    	if (otmp) {
	    	    (void) drain_item(otmp);
	    	    /* No message */
	    	}
	    	return (1);
	    default:
		break;
	}
	if (!Upolyd) return 1;

	/* These affect the enemy only if you are still a monster */
	if (rn2(3)) switch(youmonst.data->mattk[i].adtyp) {
	    case AD_PHYS:
	    	if (youmonst.data->mattk[i].aatyp == AT_BOOM) {
/*JP
	    	    You("explode!");
*/
	    	    pline("あなたは爆発した！");
	    	    /* KMH, balance patch -- this is okay with unchanging */
	    	    rehumanize();
	    	    goto assess_dmg;
	    	}
	    	break;
	    case AD_PLYS: /* Floating eye */
		if (tmp > 127) tmp = 127;
		if (u.umonnum == PM_FLOATING_EYE) {
		    if (!rn2(4)) tmp = 127;
		    if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3) &&
				(perceives(mtmp->data) || !Invis)) {
			if (Blind)
#if 0 /*JP*/
			    pline("As a blind %s, you cannot defend yourself.",
							youmonst.data->mname);
#else
			    pline("%sは目が見えないので、あなたはにらみで防御できない。",
							jtrns_mon_gen(youmonst.data->mname, flags.female));
#endif
		        else {
#if 0 /*JP*/
			    if (mon_reflects(mtmp,
					    "Your gaze is reflected by %s %s."))
#else
			    if (mon_reflects(mtmp,
					     "あなたのにらみは%sの%sで反射した。"))
#endif
				return 1;
/*JP
			    pline("%s is frozen by your gaze!", Monnam(mtmp));
*/
			    pline("%sはあなたのにらみで動けなくなった！", Monnam(mtmp));
			    mtmp->mcanmove = 0;
			    mtmp->mfrozen = tmp;
			    return 3;
			}
		    }
		} else { /* gelatinous cube */
/*JP
		    pline("%s is frozen by you.", Monnam(mtmp));
*/
		    pline("%sは動けない。", Monnam(mtmp));
		    mtmp->mcanmove = 0;
		    mtmp->mfrozen = tmp;
		    return 3;
		}
		return 1;
	    case AD_COLD: /* Brown mold or blue jelly */
		if (resists_cold(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
/*JP
		    pline("%s is mildly chilly.", Monnam(mtmp));
*/
		    pline("%sは冷えた。", Monnam(mtmp));
		    golemeffects(mtmp, AD_COLD, tmp);
		    tmp = 0;
		    break;
		}
/*JP
		pline("%s is suddenly very cold!", Monnam(mtmp));
*/
		pline("%sは突然凍りづけになった！", Monnam(mtmp));
		u.mh += tmp / 2;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		if (u.mhmax > ((youmonst.data->mlevel+1) * 8))
		    (void)split_mon(&youmonst, mtmp);
		break;
	    case AD_STUN: /* Yellow mold */
		if (!mtmp->mstun) {
		    mtmp->mstun = 1;
#if 0 /*JP*/
		    pline("%s %s.", Monnam(mtmp),
			  makeplural(stagger(mtmp->data, "stagger")));
#else
		    pline("%sはくらくらした。", Monnam(mtmp));
#endif
		}
		tmp = 0;
		break;
	    case AD_FIRE: /* Red mold */
		if (resists_fire(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
#if 0 /*JP*/
		    pline("%s is mildly warm.", Monnam(mtmp));
#else
		    pline("%sは暖かくなった。", Monnam(mtmp));
#endif
		    golemeffects(mtmp, AD_FIRE, tmp);
		    tmp = 0;
		    break;
		}
#if 0 /*JP*/
		pline("%s is suddenly very hot!", Monnam(mtmp));
#else
		pline("%sは突然熱くなった！", Monnam(mtmp));
#endif
		break;
	    case AD_ELEC:
		if (resists_elec(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
#if 0 /*JP*/
		    pline("%s is slightly tingled.", Monnam(mtmp));
#else
		    pline("%sはちょっとピリピリした。", Monnam(mtmp));
#endif
		    golemeffects(mtmp, AD_ELEC, tmp);
		    tmp = 0;
		    break;
		}
#if 0 /*JP*/
		pline("%s is jolted with your electricity!", Monnam(mtmp));
#else
		pline("%sは電気ショックをうけた！", Monnam(mtmp));
#endif
		break;
	    default: tmp = 0;
		break;
	}
	else tmp = 0;

    assess_dmg:
	if((mtmp->mhp -= tmp) <= 0) {
/*JP
		pline("%s dies!", Monnam(mtmp));
*/
		pline("%sは死んだ！", Monnam(mtmp));
		xkilled(mtmp,0);
		if (mtmp->mhp > 0) return 1;
		return 2;
	}
	return 1;
}

#endif /* OVL1 */
#ifdef OVLB

#include "edog.h"
struct monst *
cloneu()
{
	register struct monst *mon;
	int mndx = monsndx(youmonst.data);

	if (u.mh <= 1) return(struct monst *)0;
	if (mvitals[mndx].mvflags & G_EXTINCT) return(struct monst *)0;
	mon = makemon(youmonst.data, u.ux, u.uy, NO_MINVENT|MM_EDOG);
	mon = christen_monst(mon, plname);
	initedog(mon);
	mon->m_lev = youmonst.data->mlevel;
	mon->mhpmax = u.mhmax;
	mon->mhp = u.mh / 2;
	u.mh -= mon->mhp;
	flags.botl = 1;
	return(mon);
}

#endif /* OVLB */

/*mhitu.c*/
