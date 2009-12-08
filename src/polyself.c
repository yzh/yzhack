/*	SCCS Id: @(#)polyself.c	3.4	2003/01/08	*/
/*	Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that both youmonst.m_id
 * and youmonst.mx will always remain 0 when it handles the case of the
 * player polymorphed into a light-emitting monster.
 */

#include "hack.h"

#ifdef OVLB
STATIC_DCL void FDECL(polyman, (const char *,const char *));
STATIC_DCL void NDECL(break_armor);
STATIC_DCL void FDECL(drop_weapon,(int));
STATIC_DCL void NDECL(uunstick);
STATIC_DCL int FDECL(armor_to_dragon,(int));
STATIC_DCL void NDECL(newman);

/* update the youmonst.data structure pointer */
void
set_uasmon()
{
	set_mon_data(&youmonst, &mons[u.umonnum], 0);
}

/* make a (new) human out of the player */
STATIC_OVL void
polyman(fmt, arg)
const char *fmt, *arg;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_mimicking = (youmonst.m_ap_type == M_AP_OBJECT);
	boolean could_pass_walls = Passes_walls;
	boolean was_blind = !!Blind;

	if (Upolyd) {
		u.acurr = u.macurr;	/* restore old attribs */
		u.amax = u.mamax;
		u.umonnum = u.umonster;
		flags.female = u.mfemale;
	}
	set_uasmon();

	u.mh = u.mhmax = 0;
	u.mtimedone = 0;
	skinback(FALSE);
	u.uundetected = 0;

	if (sticky) uunstick();
	find_ac();
	if (was_mimicking) {
	    if (multi < 0) unmul("");
	    youmonst.m_ap_type = M_AP_NOTHING;
	}

	newsym(u.ux,u.uy);

	You(fmt, arg);
	/* check whether player foolishly genocided self while poly'd */
	if ((mvitals[urole.malenum].mvflags & G_GENOD) ||
			(urole.femalenum != NON_PM &&
			(mvitals[urole.femalenum].mvflags & G_GENOD)) ||
			(mvitals[urace.malenum].mvflags & G_GENOD) ||
			(urace.femalenum != NON_PM &&
			(mvitals[urace.femalenum].mvflags & G_GENOD))) {
	    /* intervening activity might have clobbered genocide info */
	    killer = delayed_killer;
/*JP
	    if (!killer || !strstri(killer, "genocid")) {
*/
	    if (!killer || !strstr(killer, "虐殺")) {
		killer_format = KILLED_BY;
/*JP
		killer = "self-genocide";
*/
		killer = "自虐的虐殺で";
	    }
	    done(GENOCIDED);
	}

	if (u.twoweap && !could_twoweap(youmonst.data))
	    untwoweapon();

	if (u.utraptype == TT_PIT) {
	    if (could_pass_walls) {	/* player forms cannot pass walls */
		u.utrap = rn1(6,2);
	    }
	}
	if (was_blind && !Blind) {	/* reverting from eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}

	if(!Levitation && !u.ustuck &&
	   (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy)))
		spoteffects(TRUE);

	see_monsters();
}

void
change_sex()
{
	/* setting u.umonster for caveman/cavewoman or priest/priestess
	   swap unintentionally makes `Upolyd' appear to be true */
	boolean already_polyd = (boolean) Upolyd;

	/* Some monsters are always of one sex and their sex can't be changed */
	/* succubi/incubi can change, but are handled below */
	/* !already_polyd check necessary because is_male() and is_female()
           are true if the player is a priest/priestess */
	if (!already_polyd || (!is_male(youmonst.data) && !is_female(youmonst.data) && !is_neuter(youmonst.data)))
	    flags.female = !flags.female;
	if (already_polyd)	/* poly'd: also change saved sex */
	    u.mfemale = !u.mfemale;
	max_rank_sz();		/* [this appears to be superfluous] */
	if ((already_polyd ? u.mfemale : flags.female) && urole.name.f)
	    Strcpy(pl_character, urole.name.f);
	else
	    Strcpy(pl_character, urole.name.m);
	u.umonster = ((already_polyd ? u.mfemale : flags.female) && urole.femalenum != NON_PM) ?
			urole.femalenum : urole.malenum;
	if (!already_polyd) {
	    u.umonnum = u.umonster;
	} else if (u.umonnum == PM_SUCCUBUS || u.umonnum == PM_INCUBUS) {
	    flags.female = !flags.female;
	    /* change monster type to match new sex */
	    u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
	    set_uasmon();
	}
}

STATIC_OVL void
newman()
{
	int tmp, oldlvl;

	tmp = u.uhpmax;
	oldlvl = u.ulevel;
	u.ulevel = u.ulevel + rn1(5, -2);
	if (u.ulevel > 127 || u.ulevel < 1) { /* level went below 0? */
	    u.ulevel = oldlvl; /* restore old level in case they lifesave */
	    goto dead;
	}
	if (u.ulevel > MAXULEV) u.ulevel = MAXULEV;
	/* If your level goes down, your peak level goes down by
	   the same amount so that you can't simply use blessed
	   full healing to undo the decrease.  But if your level
	   goes up, your peak level does *not* undergo the same
	   adjustment; you might end up losing out on the chance
	   to regain some levels previously lost to other causes. */
	if (u.ulevel < oldlvl) u.ulevelmax -= (oldlvl - u.ulevel);
	if (u.ulevelmax < u.ulevel) u.ulevelmax = u.ulevel;

	if (!rn2(10)) change_sex();

	adjabil(oldlvl, (int)u.ulevel);
	reset_rndmonst(NON_PM);	/* new monster generation criteria */

	/* random experience points for the new experience level */
	u.uexp = rndexp(FALSE);

	/* u.uhpmax * u.ulevel / oldlvl: proportionate hit points to new level
	 * -10 and +10: don't apply proportionate HP to 10 of a starting
	 *   character's hit points (since a starting character's hit points
	 *   are not on the same scale with hit points obtained through level
	 *   gain)
	 * 9 - rn2(19): random change of -9 to +9 hit points
	 */
#ifndef LINT
	u.uhpmax = ((u.uhpmax - 10) * (long)u.ulevel / oldlvl + 10) +
		(9 - rn2(19));
#endif

#ifdef LINT
	u.uhp = u.uhp + tmp;
#else
	u.uhp = u.uhp * (long)u.uhpmax/tmp;
#endif

	tmp = u.uenmax;
#ifndef LINT
	u.uenmax = u.uenmax * (long)u.ulevel / oldlvl + 9 - rn2(19);
#endif
	if (u.uenmax < 0) u.uenmax = 0;
#ifndef LINT
	u.uen = (tmp ? u.uen * (long)u.uenmax / tmp : u.uenmax);
#endif

	redist_attr();
	u.uhunger = rn1(500,500);
	if (Sick) make_sick(0L, (char *) 0, FALSE, SICK_ALL);
	Stoned = 0;
	delayed_killer = 0;
	if (u.uhp <= 0 || u.uhpmax <= 0) {
		if (Polymorph_control) {
		    if (u.uhp <= 0) u.uhp = 1;
		    if (u.uhpmax <= 0) u.uhpmax = 1;
		} else {
dead: /* we come directly here if their experience level went to 0 or less */
/*JP
		    Your("new form doesn't seem healthy enough to survive.");
*/
		    Your("新しい姿は生きていくだけの力がないようだ。");
		    killer_format = KILLED_BY_AN;
/*JP
		    killer="unsuccessful polymorph";
*/
		    killer="変化の失敗で";
		    done(DIED);
		    newuhs(FALSE);
		    return; /* lifesaved */
		}
	}
	newuhs(FALSE);
#if 0 /*JP*/
	polyman("feel like a new %s!",
		(flags.female && urace.individual.f) ? urace.individual.f :
		(urace.individual.m) ? urace.individual.m : urace.noun);
#else
	polyman("%sとして生まれかわったように感じた！",
		jtrns_mon_gen(
		    (flags.female && urace.individual.f) ? urace.individual.f :
		    (urace.individual.m) ? urace.individual.m : urace.noun, flags.female));
#endif
	if (Slimed) {
/*JP
		Your("body transforms, but there is still slime on you.");
*/
		Your("体は変化したが、スライムがついたままだ。");
		Slimed = 10L;
	}
	flags.botl = 1;
	see_monsters();
	(void) encumber_msg();
}

void
polyself(forcecontrol)
boolean forcecontrol;     
{
	char buf[BUFSZ];
	int old_light, new_light;
	int mntmp = NON_PM;
	int tries=0;
	boolean draconian = (uarm &&
				uarm->otyp >= GRAY_DRAGON_SCALE_MAIL &&
				uarm->otyp <= YELLOW_DRAGON_SCALES);
	boolean iswere = (u.ulycn >= LOW_PM || is_were(youmonst.data));
	boolean isvamp = (youmonst.data->mlet == S_VAMPIRE || u.umonnum == PM_VAMPIRE_BAT);
	boolean was_floating = (Levitation || Flying);

        if(!Polymorph_control && !forcecontrol && !draconian && !iswere && !isvamp) {
	    if (rn2(20) > ACURR(A_CON)) {
		You(shudder_for_moment);
/*JP
		losehp(rnd(30), "system shock", KILLED_BY_AN);
*/
		losehp(rnd(30), "システムショックで", KILLED_BY_AN);
		exercise(A_CON, FALSE);
		return;
	    }
	}
	old_light = Upolyd ? emits_light(youmonst.data) : 0;

	if (Polymorph_control || forcecontrol) {
		do {
/*JP
			getlin("Become what kind of monster? [type the name]",
*/
			getlin("どの種の怪物になる？[名前を入れてね]",
				buf);
#if 1 /*JP*/
			(void)mungspaces(buf);
			{
			    const char *p;
			    p = etrns_mon(buf);
			    if(p) strcpy(buf, p);
			}
#endif
			mntmp = name_to_mon(buf);
			if (mntmp < LOW_PM)
/*JP
				pline("I've never heard of such monsters.");
*/
				pline("そんな怪物は聞いたことがない。");
			/* Note:  humans are illegal as monsters, but an
			 * illegal monster forces newman(), which is what we
			 * want if they specified a human.... */
			else if (!polyok(&mons[mntmp]) && !your_race(&mons[mntmp]))
/*JP
				You("cannot polymorph into that.");
*/
				pline("それになることはできない。");
			else break;
		} while(++tries < 5);
		if (tries==5) pline(thats_enough_tries);
		/* allow skin merging, even when polymorph is controlled */
		if (draconian &&
		    (mntmp == armor_to_dragon(uarm->otyp) || tries == 5))
		    goto do_merge;
	} else if (draconian || iswere || isvamp) {
		/* special changes that don't require polyok() */
		if (draconian) {
		    do_merge:
			mntmp = armor_to_dragon(uarm->otyp);
			if (!(mvitals[mntmp].mvflags & G_GENOD)) {
				/* allow G_EXTINCT */
/*JP
				You("merge with your scaly armor.");
*/
				You("鱗の鎧と一体化した。");
				uskin = uarm;
				uarm = (struct obj *)0;
				/* save/restore hack */
				uskin->owornmask |= I_SPECIAL;
			}
		} else if (iswere) {
			if (is_were(youmonst.data))
				mntmp = PM_HUMAN; /* Illegal; force newman() */
			else
				mntmp = u.ulycn;
		} else {
			if (youmonst.data->mlet == S_VAMPIRE)
				mntmp = PM_VAMPIRE_BAT;
			else
				mntmp = PM_VAMPIRE;
		}
		/* if polymon fails, "you feel" message has been given
		   so don't follow up with another polymon or newman */
		if (mntmp == PM_HUMAN) newman();	/* werecritter */
		else (void) polymon(mntmp);
		goto made_change;    /* maybe not, but this is right anyway */
	}

	if (mntmp < LOW_PM) {
		tries = 0;
		do {
			/* randomly pick an "ordinary" monster */
			mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
		} while((!polyok(&mons[mntmp]) || is_placeholder(&mons[mntmp]))
				&& tries++ < 200);
	}

	/* The below polyok() fails either if everything is genocided, or if
	 * we deliberately chose something illegal to force newman().
	 */
	if (!polyok(&mons[mntmp]) || !rn2(5) || your_race(&mons[mntmp]))
		newman();
	else if(!polymon(mntmp)) return;

/*JP
	if (!uarmg) selftouch("No longer petrify-resistant, you");
*/
	if (!uarmg) selftouch("石化への抵抗力がなくなって、あなたは");

 made_change:
	new_light = Upolyd ? emits_light(youmonst.data) : 0;
	if (old_light != new_light) {
	    if (old_light)
		del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
	    if (new_light == 1) ++new_light;  /* otherwise it's undetectable */
	    if (new_light)
		new_light_source(u.ux, u.uy, new_light,
				 LS_MONSTER, (genericptr_t)&youmonst);
	}
	if (is_pool(u.ux,u.uy) && was_floating && !(Levitation || Flying) &&
		!breathless(youmonst.data) && !amphibious(youmonst.data) &&
		!Swimming) drown();
}

/* (try to) make a mntmp monster out of the player */
int
polymon(mntmp)	/* returns 1 if polymorph successful */
int	mntmp;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_blind = !!Blind, dochange = FALSE;
	boolean could_pass_walls = Passes_walls;
	int mlvl;

	if (mvitals[mntmp].mvflags & G_GENOD) {	/* allow G_EXTINCT */
#if 0 /*JP:T*/
		You_feel("rather %s-ish.",mons[mntmp].mname);
#else
		You("%sっぽくなったように感じた。",
		    jtrns_mon(mons[mntmp].mname));
#endif
		exercise(A_WIS, TRUE);
		return(0);
	}

	/* KMH, conduct */
	u.uconduct.polyselfs++;

	if (!Upolyd) {
		/* Human to monster; save human stats */
		u.macurr = u.acurr;
		u.mamax = u.amax;
		u.mfemale = flags.female;
	} else {
		/* Monster to monster; restore human stats, to be
		 * immediately changed to provide stats for the new monster
		 */
		u.acurr = u.macurr;
		u.amax = u.mamax;
		flags.female = u.mfemale;
	}

	if (youmonst.m_ap_type) {
	    /* stop mimicking immediately */
	    if (multi < 0) unmul("");
	} else if (mons[mntmp].mlet != S_MIMIC) {
	    /* as in polyman() */
	    youmonst.m_ap_type = M_AP_NOTHING;
	}
	if (is_male(&mons[mntmp])) {
		if(flags.female) dochange = TRUE;
	} else if (is_female(&mons[mntmp])) {
		if(!flags.female) dochange = TRUE;
	} else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
		if(!rn2(10)) dochange = TRUE;
	}
	if (dochange) {
		flags.female = !flags.female;
#if 0 /*JP*/
		You("%s %s%s!",
		    (u.umonnum != mntmp) ? "turn into a" : "feel like a new",
		    (is_male(&mons[mntmp]) || is_female(&mons[mntmp])) ? "" :
			flags.female ? "female " : "male ",
		    mons[mntmp].mname);
#else
		You("%s%sになった%s！",
		    (is_male(&mons[mntmp]) || is_female(&mons[mntmp])) ? "" :
			flags.female ? "女の" : "男の",
		    jtrns_mon_gen(mons[mntmp].mname, flags.female),
		    (u.umonnum != mntmp) ? "" : "ように感じた");
#endif
	} else {
		if (u.umonnum != mntmp)
#if 0 /*JP*/
			You("turn into %s!", an(mons[mntmp].mname));
#else
			You("%sになった！", 
			    jtrns_mon_gen(mons[mntmp].mname, flags.female));
#endif
		else
#if 0 /*JP*/
			You_feel("like a new %s!", mons[mntmp].mname);
#else
			You("別の%sになったように感じた！", 
			    jtrns_mon_gen(mons[mntmp].mname, flags.female));
#endif
	}
	if (Stoned && poly_when_stoned(&mons[mntmp])) {
		/* poly_when_stoned already checked stone golem genocide */
/*JP
		You("turn to stone!");
*/
		You("石になった！");
		mntmp = PM_STONE_GOLEM;
		Stoned = 0;
		delayed_killer = 0;
	}

	u.mtimedone = rn1(500, 500);
	u.umonnum = mntmp;
	set_uasmon();

	/* New stats for monster, to last only as long as polymorphed.
	 * Currently only strength gets changed.
	 */
	if(strongmonst(&mons[mntmp])) ABASE(A_STR) = AMAX(A_STR) = STR18(100);

	if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
		Stoned = 0;
		delayed_killer = 0;
/*JP
		You("no longer seem to be petrifying.");
*/
		You("石化から解放されたようだ。");
	}
	if (Sick_resistance && Sick) {
		make_sick(0L, (char *) 0, FALSE, SICK_ALL);
/*JP
		You("no longer feel sick.");
*/
		You("病気から解放されたようだ。");
	}
	if (Slimed) {
	    if (flaming(youmonst.data)) {
/*JP
		pline_The("slime burns away!");
*/
		pline_The("スライムは燃えた！");
		Slimed = 0L;
		flags.botl = 1;
	    } else if (mntmp == PM_GREEN_SLIME) {
		/* do it silently */
		Slimed = 0L;
		flags.botl = 1;
	    }
	}
	if (nohands(youmonst.data)) Glib = 0;

	/*
	mlvl = adj_lev(&mons[mntmp]);
	 * We can't do the above, since there's no such thing as an
	 * "experience level of you as a monster" for a polymorphed character.
	 */
	mlvl = (int)mons[mntmp].mlevel;
	if (youmonst.data->mlet == S_DRAGON && mntmp >= PM_GRAY_DRAGON) {
		u.mhmax = In_endgame(&u.uz) ? (8*mlvl) : (4*mlvl + d(mlvl,4));
	} else if (is_golem(youmonst.data)) {
		u.mhmax = golemhp(mntmp);
	} else {
		if (!mlvl) u.mhmax = rnd(4);
		else u.mhmax = d(mlvl, 8);
		if (is_home_elemental(&mons[mntmp])) u.mhmax *= 3;
	}
	u.mh = u.mhmax;

	if (u.ulevel < mlvl) {
	/* Low level characters can't become high level monsters for long */
#ifdef DUMB
		/* DRS/NS 2.2.6 messes up -- Peter Kendell */
		int mtd = u.mtimedone, ulv = u.ulevel;

		u.mtimedone = mtd * ulv / mlvl;
#else
		u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
	}

	if (uskin && mntmp != armor_to_dragon(uskin->otyp))
		skinback(FALSE);
	break_armor();
	drop_weapon(1);
	if (hides_under(youmonst.data))
		u.uundetected = OBJ_AT(u.ux, u.uy);
	else if (youmonst.data->mlet == S_EEL)
		u.uundetected = is_pool(u.ux, u.uy);
	else
		u.uundetected = 0;

	if (u.utraptype == TT_PIT) {
	    if (could_pass_walls && !Passes_walls) {
		u.utrap = rn1(6,2);
	    } else if (!could_pass_walls && Passes_walls) {
		u.utrap = 0;
	    }
	}
	if (was_blind && !Blind) {	/* previous form was eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}
	newsym(u.ux,u.uy);		/* Change symbol */

	if (!sticky && !u.uswallow && u.ustuck && sticks(youmonst.data)) u.ustuck = 0;
	else if (sticky && !sticks(youmonst.data)) uunstick();
#ifdef STEED
	if (u.usteed) {
	    if (touch_petrifies(u.usteed->data) &&
	    		!Stone_resistance && rnl(3)) {
	    	char buf[BUFSZ];

#if 0 /*JP*/
	    	pline("No longer petrifying-resistant, you touch %s.",
	    			mon_nam(u.usteed));
	    	Sprintf(buf, "riding %s", an(u.usteed->data->mname));
#else
	    	pline("石化への耐性がないのに%sに触れてしまった。",
	    			mon_nam(u.usteed));
	    	Sprintf(buf, "%sに乗って", a_monnam(u.usteed));
#endif
	    	instapetrify(buf);
 	    }
	    if (!can_ride(u.usteed)) dismount_steed(DISMOUNT_POLY);
	}
#endif

	if (flags.verbose) {
/*JP
	    static const char use_thec[] = "Use the command #%s to %s.";
*/
	    static const char use_thec[] = "#%sコマンドで%sことができる。";
	    static const char monsterc[] = "monster";
	    if (can_breathe(youmonst.data))
/*JP
		pline(use_thec,monsterc,"use your breath weapon");
*/
		pline(use_thec,monsterc,"息を吐きかける");
	    if (attacktype(youmonst.data, AT_SPIT))
/*JP
		pline(use_thec,monsterc,"spit venom");
*/
		pline(use_thec,monsterc,"毒を吐く");
	    if (youmonst.data->mlet == S_NYMPH)
/*JP
		pline(use_thec,monsterc,"remove an iron ball");
*/
		pline(use_thec,monsterc,"鉄球をはずす");
	    if (attacktype(youmonst.data, AT_GAZE))
/*JP
		pline(use_thec,monsterc,"gaze at monsters");
*/
		pline(use_thec,monsterc,"怪物を睨む");
	    if (is_hider(youmonst.data))
/*JP
		pline(use_thec,monsterc,"hide");
*/
		pline(use_thec,monsterc,"隠れる");
	    if (is_were(youmonst.data))
/*JP
		pline(use_thec,monsterc,"summon help");
*/
		pline(use_thec,monsterc,"仲間を召喚する");
	    if (webmaker(youmonst.data))
/*JP
		pline(use_thec,monsterc,"spin a web");
*/
		pline(use_thec,monsterc,"くもの巣を張る");
	    if (u.umonnum == PM_GREMLIN)
/*JP
		pline(use_thec,monsterc,"multiply in a fountain");
*/
		pline(use_thec,monsterc,"泉の中で分裂する");
	    if (is_unicorn(youmonst.data))
/*JP
		pline(use_thec,monsterc,"use your horn");
*/
		pline(use_thec,monsterc,"角を使う");
	    if (is_mind_flayer(youmonst.data))
/*JP
		pline(use_thec,monsterc,"emit a mental blast");
*/
		pline(use_thec,monsterc,"精神波を発生させる");
	    if (youmonst.data->msound == MS_SHRIEK) /* worthless, actually */
/*JP
		pline(use_thec,monsterc,"shriek");
*/
		pline(use_thec,monsterc,"金切り声をあげる");
	    if (lays_eggs(youmonst.data) && flags.female)
/*JP
		pline(use_thec,"sit","lay an egg");
*/
		pline(use_thec,"sit","卵を産む");
	}
	/* you now know what an egg of your type looks like */
	if (lays_eggs(youmonst.data)) {
	    learn_egg_type(u.umonnum);
	    /* make queen bees recognize killer bee eggs */
	    learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
	}
	find_ac();
	if((!Levitation && !u.ustuck && !Flying &&
	    (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy))) ||
	   (Underwater && !Swimming))
	    spoteffects(TRUE);
	if (Passes_walls && u.utrap && u.utraptype == TT_INFLOOR) {
	    u.utrap = 0;
/*JP
	    pline_The("rock seems to no longer trap you.");
*/
	    pline("岩に閉じ込められることはないだろう。");
	} else if (likes_lava(youmonst.data) && u.utrap && u.utraptype == TT_LAVA) {
	    u.utrap = 0;
/*JP
	    pline_The("lava now feels soothing.");
*/
	    pline("溶岩が精神を落ちつかせてくれる。");
	}
	if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
	    if (Punished) {
/*JP
		You("slip out of the iron chain.");
*/
		You("鉄の鎖からするりと抜けた。");
		unpunish();
	    }
	}
	if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP) &&
		(amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data) ||
		  (youmonst.data->msize <= MZ_SMALL && u.utraptype == TT_BEARTRAP))) {
#if 0 /*JP*/
	    You("are no longer stuck in the %s.",
		    u.utraptype == TT_WEB ? "web" : "bear trap");
#else
	    You("%sから脱出した。",
		    u.utraptype == TT_WEB ? "くもの巣" : "トラバサミ");
#endif
	    /* probably should burn webs too if PM_FIRE_ELEMENTAL */
	    u.utrap = 0;
	}
	if (webmaker(youmonst.data) && u.utrap && u.utraptype == TT_WEB) {
/*JP
	    You("orient yourself on the web.");
*/
	    You("くもの巣に適応した。");
	    u.utrap = 0;
	}
	flags.botl = 1;
	vision_full_recalc = 1;
	see_monsters();
	exercise(A_CON, FALSE);
	exercise(A_WIS, TRUE);
	(void) encumber_msg();
	return(1);
}

STATIC_OVL void
break_armor()
{
    register struct obj *otmp;

    if (breakarm(youmonst.data)) {
	if ((otmp = uarm) != 0) {
		if (donning(otmp)) cancel_don();
/*JP
		You("break out of your armor!");
*/
		You("鎧を壊した！");
		exercise(A_STR, FALSE);
		(void) Armor_gone();
		useup(otmp);
	}
	if ((otmp = uarmc) != 0) {
	    if(otmp->oartifact) {
/*JP
		Your("%s falls off!", cloak_simple_name(otmp));
*/
		Your("%sは脱げ落ちた！", cloak_simple_name(otmp));
		(void) Cloak_off();
		dropx(otmp);
	    } else {
/*JP
		Your("%s tears apart!", cloak_simple_name(otmp));
*/
		Your("%sはずたずたに引き裂かれた！", cloak_simple_name(otmp));
		(void) Cloak_off();
		useup(otmp);
	    }
	}
#ifdef TOURIST
	if (uarmu) {
/*JP
		Your("shirt rips to shreds!");
*/
		Your("シャツは引き裂かれた！");
		useup(uarmu);
	}
#endif
    } else if (sliparm(youmonst.data)) {
	if (((otmp = uarm) != 0) && (racial_exception(&youmonst, otmp) < 1)) {
		if (donning(otmp)) cancel_don();
/*JP
		Your("armor falls around you!");
*/
		Your("鎧はあなたのまわりに落ちた！");
		(void) Armor_gone();
		dropx(otmp);
	}
	if ((otmp = uarmc) != 0) {
		if (is_whirly(youmonst.data))
/*JP
			Your("%s falls, unsupported!", cloak_simple_name(otmp));
*/
			Your("%sはすとんと落ちた！", cloak_simple_name(otmp));
/*JP
		else You("shrink out of your %s!", cloak_simple_name(otmp));
*/
		else You("%sから縮み出た！", cloak_simple_name(otmp));
		(void) Cloak_off();
		dropx(otmp);
	}
#ifdef TOURIST
	if ((otmp = uarmu) != 0) {
		if (is_whirly(youmonst.data))
/*JP
			You("seep right through your shirt!");
*/
			You("シャツからしみ出た！");
/*JP
		else You("become much too small for your shirt!");
*/
		else You("シャツよりずっと小さくなった！");
		setworn((struct obj *)0, otmp->owornmask & W_ARMU);
		dropx(otmp);
	}
#endif
    }
    if (has_horns(youmonst.data)) {
	if ((otmp = uarmh) != 0) {
	    if (is_flimsy(otmp) && !donning(otmp)) {
#if 0 /*JP*/
		char hornbuf[BUFSZ], yourbuf[BUFSZ];

		/* Future possiblities: This could damage/destroy helmet */
		Sprintf(hornbuf, "horn%s", plur(num_horns(youmonst.data)));
		Your("%s %s through %s %s.", hornbuf, vtense(hornbuf, "pierce"),
		     shk_your(yourbuf, otmp), xname(otmp));
#else
		char yourbuf[BUFSZ];
		Your("角が%s%sをつらぬいた。", shk_your(yourbuf, otmp), xname(otmp));
#endif
	    } else {
		if (donning(otmp)) cancel_don();
/*JP
		Your("helmet falls to the %s!", surface(u.ux, u.uy));
*/
	    Your("兜は%sに落ちた！", surface(u.ux, u.uy));
		(void) Helmet_off();
		dropx(otmp);
	    }
	}
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data)) {
	if ((otmp = uarmg) != 0) {
	    if (donning(otmp)) cancel_don();
	    /* Drop weapon along with gloves */
/*JP
	    You("drop your gloves%s!", uwep ? " and weapon" : "");
*/
	    You("籠手%sを落した！", uwep ? "や武器" : "");
	    drop_weapon(0);
	    (void) Gloves_off();
	    dropx(otmp);
	}
	if ((otmp = uarms) != 0) {
/*JP
	    You("can no longer hold your shield!");
*/
	    You("もう盾を持ってられない！");
	    (void) Shield_off();
	    dropx(otmp);
	}
	if ((otmp = uarmh) != 0) {
	    if (donning(otmp)) cancel_don();
/*JP
	    Your("helmet falls to the %s!", surface(u.ux, u.uy));
*/
	    Your("兜は%sに落ちた！", surface(u.ux, u.uy));
	    (void) Helmet_off();
	    dropx(otmp);
	}
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data) ||
		slithy(youmonst.data) || youmonst.data->mlet == S_CENTAUR) {
	if ((otmp = uarmf) != 0) {
	    if (donning(otmp)) cancel_don();
	    if (is_whirly(youmonst.data))
/*JP
		Your("boots fall away!");
*/
		Your("靴は脱げ落ちた！");
#if 0 /*JP*/
	    else Your("boots %s off your feet!",
			verysmall(youmonst.data) ? "slide" : "are pushed");
#else
	    else Your("靴はあなたの足から%s！",
			verysmall(youmonst.data) ? "滑り落ちた" : "脱げ落ちた");
#endif
	    (void) Boots_off();
	    dropx(otmp);
	}
    }
}

STATIC_OVL void
drop_weapon(alone)
int alone;
{
    struct obj *otmp;
    struct obj *otmp2;

    if ((otmp = uwep) != 0) {
	/* !alone check below is currently superfluous but in the
	 * future it might not be so if there are monsters which cannot
	 * wear gloves but can wield weapons
	 */
	if (!alone || cantwield(youmonst.data)) {
	    struct obj *wep = uwep;

#if 0 /*JP*/
	    if (alone) You("find you must drop your weapon%s!",
			   	u.twoweap ? "s" : "");
#else
	    if (alone) You("武器を落したことに気がついた！");
#endif
	    otmp2 = u.twoweap ? uswapwep : 0;
	    uwepgone();
	    if (!wep->cursed || wep->otyp != LOADSTONE)
		dropx(otmp);
	    if (otmp2 != 0) {
		uswapwepgone();
		if (!otmp2->cursed || otmp2->otyp != LOADSTONE)
		    dropx(otmp2);
	    }
	    untwoweapon();
	} else if (!could_twoweap(youmonst.data)) {
	    untwoweapon();
	}
    }
}

void
rehumanize()
{
	/* You can't revert back while unchanging */
	if (Unchanging && (u.mh < 1)) {
#if 0 /*JP*/
		killer_format = NO_KILLER_PREFIX;
		killer = "killed while stuck in creature form";
#else
		killer_format = KILLED_BY;
		killer = "元の姿へ戻れずに";
#endif
		done(DIED);
	}

	if (emits_light(youmonst.data))
	    del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
/*JP
	polyman("return to %s form!", urace.adj);
*/
	polyman("%sに戻った！", urace.j);

	if (u.uhp < 1) {
	    char kbuf[256];

/*JP
	    Sprintf(kbuf, "reverting to unhealthy %s form", urace.adj);
*/
	    Sprintf(kbuf, "不健康な%sの姿に戻って", urace.j);
	    killer_format = KILLED_BY;
	    killer = kbuf;
	    done(DIED);
	}
/*JP
	if (!uarmg) selftouch("No longer petrify-resistant, you");
*/
	if (!uarmg) selftouch("石化への抵抗力がなくなって、あなたは");
	nomul(0);

	flags.botl = 1;
	vision_full_recalc = 1;
	(void) encumber_msg();
}

int
dobreathe()
{
	struct attack *mattk;

	if (Strangled) {
/*JP
	    You_cant("breathe.  Sorry.");
*/
	    You_cant("息を吐くことができない。残念。");
	    return(0);
	}
	if (u.uen < 15) {
/*JP
	    You("don't have enough energy to breathe!");
*/
	    You("息を吐くのに十分なエネルギーがなかった。");
	    return(0);
	}
	u.uen -= 15;
	flags.botl = 1;

	if (!getdir((char *)0)) return(0);

	mattk = attacktype_fordmg(youmonst.data, AT_BREA, AD_ANY);
	if (!mattk)
	    impossible("bad breath attack?");	/* mouthwash needed... */
	else
	    buzz((int) (20 + mattk->adtyp-1), (int)mattk->damn,
		u.ux, u.uy, u.dx, u.dy);
	return(1);
}

int
dospit()
{
	struct obj *otmp;

	if (!getdir((char *)0)) return(0);
	otmp = mksobj(u.umonnum==PM_COBRA ? BLINDING_VENOM : ACID_VENOM,
			TRUE, FALSE);
	otmp->spe = 1; /* to indicate it's yours */
	throwit(otmp, 0L, FALSE);
	return(1);
}

int
doremove()
{
	if (!Punished) {
/*JP
		You("are not chained to anything!");
*/
		You("何もつながれていない！");
		return(0);
	}
	unpunish();
	return(1);
}

int
dospinweb()
{
	register struct trap *ttmp = t_at(u.ux,u.uy);

	if (Levitation || Is_airlevel(&u.uz)
	    || Underwater || Is_waterlevel(&u.uz)) {
/*JP
		You("must be on the ground to spin a web.");
*/
		You("くもの巣を張るには地面の上にいなくてはならない。");
		return(0);
	}
	if (u.uswallow) {
/*JP
		You("release web fluid inside %s.", mon_nam(u.ustuck));
*/
		You("%sの内でくもの巣を吐き出した。", mon_nam(u.ustuck));
		if (is_animal(u.ustuck->data)) {
			expels(u.ustuck, u.ustuck->data, TRUE);
			return(0);
		}
		if (is_whirly(u.ustuck->data)) {
			int i;

			for (i = 0; i < NATTK; i++)
				if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
					break;
			if (i == NATTK)
			       impossible("Swallower has no engulfing attack?");
			else {
				char sweep[30];

				sweep[0] = '\0';
				switch(u.ustuck->data->mattk[i].adtyp) {
					case AD_FIRE:
/*JP
						Strcpy(sweep, "ignites and ");
*/
						Strcpy(sweep, "発火し");
						break;
					case AD_ELEC:
/*JP
						Strcpy(sweep, "fries and ");
*/
						Strcpy(sweep, "焦げ");
						break;
					case AD_COLD:
						Strcpy(sweep,
/*JP
						      "freezes, shatters and ");
*/
						      "凍りつき、こなごなになり");
						break;
				}
/*JP
				pline_The("web %sis swept away!", sweep);
*/
				pline("くもの巣は%sなくなった！", sweep);
			}
			return(0);
		}		     /* default: a nasty jelly-like creature */
/*JP
		pline_The("web dissolves into %s.", mon_nam(u.ustuck));
*/
		pline("くもの巣は分解して%sになった。", mon_nam(u.ustuck));
		return(0);
	}
	if (u.utrap) {
/*JP
		You("cannot spin webs while stuck in a trap.");
*/
		You("罠にはまっている間はくもの巣を張れない。");
		return(0);
	}
	exercise(A_DEX, TRUE);
	if (ttmp) switch (ttmp->ttyp) {
		case PIT:
/*JP
		case SPIKED_PIT: You("spin a web, covering up the pit.");
*/
		case SPIKED_PIT: You("くもの巣を張り、落し穴を覆った。");
			deltrap(ttmp);
			bury_objs(u.ux, u.uy);
			newsym(u.ux, u.uy);
			return(1);
/*JP
		case SQKY_BOARD: pline_The("squeaky board is muffled.");
*/
		case SQKY_BOARD: pline("きしむ板は覆われた。");
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return(1);
		case TELEP_TRAP:
		case LEVEL_TELEP:
		case MAGIC_PORTAL:
/*JP
			Your("webbing vanishes!");
*/
			Your("くもの巣は消えた！");
			return(0);
/*JP
		case WEB: You("make the web thicker.");
*/
		case WEB: You("くもの巣をより厚くした。");
			return(1);
		case HOLE:
		case TRAPDOOR:
#if 0 /*JP*/
			You("web over the %s.",
			    (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole");
#else
			You("%sをくもの巣で覆った。",
			    (ttmp->ttyp == TRAPDOOR) ? "落し扉" : "穴");
#endif
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return 1;
		case ROLLING_BOULDER_TRAP:
/*JP
			You("spin a web, jamming the trigger.");
*/
			You("くもの巣を張って、スイッチを動かなくした。");
			deltrap(ttmp);
			newsym(u.ux, u.uy);
			return(1);
		case ARROW_TRAP:
		case DART_TRAP:
		case BEAR_TRAP:
		case ROCKTRAP:
		case FIRE_TRAP:
		case LANDMINE:
		case SLP_GAS_TRAP:
		case RUST_TRAP:
		case MAGIC_TRAP:
		case ANTI_MAGIC:
		case POLY_TRAP:
/*JP
			You("have triggered a trap!");
*/
			You("罠を始動させてしまった！");
			dotrap(ttmp, 0);
			return(1);
		default:
			impossible("Webbing over trap type %d?", ttmp->ttyp);
			return(0);
		}
	else if (On_stairs(u.ux, u.uy)) {
	    /* cop out: don't let them hide the stairs */
#if 0 /*JP:T*/
	    Your("web fails to impede access to the %s.",
		 (levl[u.ux][u.uy].typ == STAIRS) ? "stairs" : "ladder");
#else
	    Your("くもの巣は%sへの移動を邪魔できない。",
		 (levl[u.ux][u.uy].typ == STAIRS) ? "階段" : "はしご");
#endif
	    return(1);
		 
	}
	ttmp = maketrap(u.ux, u.uy, WEB);
	if (ttmp) {
		ttmp->tseen = 1;
		ttmp->madeby_u = 1;
	}
	newsym(u.ux, u.uy);
	return(1);
}

int
dosummon()
{
	int placeholder;
	if (u.uen < 10) {
/*JP
	    You("lack the energy to send forth a call for help!");
*/
	    You("助けを呼ぶだけの体力がない！");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

/*JP
	You("call upon your brethren for help!");
*/
	You("仲間を呼んだ！");
	exercise(A_WIS, TRUE);
	if (!were_summon(youmonst.data, TRUE, &placeholder, (char *)0))
/*JP
		pline("But none arrive.");
*/
		pline("しかし、何も来ない。");
	return(1);
}

int
dogaze()
{
	register struct monst *mtmp;
	int looked = 0;
	char qbuf[QBUFSZ];
	int i;
	uchar adtyp = 0;

	for (i = 0; i < NATTK; i++) {
	    if(youmonst.data->mattk[i].aatyp == AT_GAZE) {
		adtyp = youmonst.data->mattk[i].adtyp;
		break;
	    }
	}
	if (adtyp != AD_CONF && adtyp != AD_FIRE) {
	    impossible("gaze attack %d?", adtyp);
	    return 0;
	}


	if (Blind) {
/*JP
	    You_cant("see anything to gaze at.");
*/
		You("目が見えないので、にらめない。");
	    return 0;
	}
	if (u.uen < 15) {
/*JP
	    You("lack the energy to use your special gaze!");
*/
	    You("にらむだけの体力がない！");
	    return(0);
	}
	u.uen -= 15;
	flags.botl = 1;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
		looked++;
		if (Invis && !perceives(mtmp->data))
/*JP
		    pline("%s seems not to notice your gaze.", Monnam(mtmp));
*/
		    pline("%sはあなたのにらみに気がついてないようだ。", Monnam(mtmp));
		else if (mtmp->minvis && !See_invisible)
/*JP
		    You_cant("see where to gaze at %s.", Monnam(mtmp));
*/
		    You("%sは見えないので、にらめない", Monnam(mtmp));
		else if (mtmp->m_ap_type == M_AP_FURNITURE
			|| mtmp->m_ap_type == M_AP_OBJECT) {
		    looked--;
		    continue;
		} else if (flags.safe_dog && !Confusion && !Hallucination
		  && mtmp->mtame) {
/*JP
		    You("avoid gazing at %s.", y_monnam(mtmp));
*/
		    You("%sから目をそらしてしまった。", y_monnam(mtmp));
		} else {
		    if (flags.confirm && mtmp->mpeaceful && !Confusion
							&& !Hallucination) {
#if 0 /*JP*/
			Sprintf(qbuf, "Really %s %s?",
			    (adtyp == AD_CONF) ? "confuse" : "attack",
			    mon_nam(mtmp));
#else
			Sprintf(qbuf, "本当に%sを%s？",
			    mon_nam(mtmp),
			    (adtyp == AD_CONF) ? "混乱させる" : "攻撃する");
#endif
			if (yn(qbuf) != 'y') continue;
			setmangry(mtmp);
		    }
		    if (!mtmp->mcanmove || mtmp->mstun || mtmp->msleeping ||
				    !mtmp->mcansee || !haseyes(mtmp->data)) {
			looked--;
			continue;
		    }
		    /* No reflection check for consistency with when a monster
		     * gazes at *you*--only medusa gaze gets reflected then.
		     */
		    if (adtyp == AD_CONF) {
			if (!mtmp->mconf)
/*JP
			    Your("gaze confuses %s!", mon_nam(mtmp));
*/
			    Your("にらみは%sを混乱させた！", mon_nam(mtmp));
			else
/*JP
			    pline("%s is getting more and more confused.",
*/
			    pline("%sはますます混乱した！",
							Monnam(mtmp));
			mtmp->mconf = 1;
		    } else if (adtyp == AD_FIRE) {
			int dmg = d(2,6);
#if 0 /*JP*/
			You("attack %s with a fiery gaze!", mon_nam(mtmp));
#else
			You("炎のにらみで%sを攻撃した！", mon_nam(mtmp));
#endif
			if (resists_fire(mtmp)) {
#if 0 /*JP*/
			    pline_The("fire doesn't burn %s!", mon_nam(mtmp));
#else
			    pline("%sは炎で燃えなかった！", mon_nam(mtmp));
#endif
			    dmg = 0;
			}
			if((int) u.ulevel > rn2(20))
			    (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
			if((int) u.ulevel > rn2(20))
			    (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
			if((int) u.ulevel > rn2(25))
			    (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
			if (dmg && !DEADMONSTER(mtmp)) mtmp->mhp -= dmg;
			if (mtmp->mhp <= 0) killed(mtmp);
		    }
		    /* For consistency with passive() in uhitm.c, this only
		     * affects you if the monster is still alive.
		     */
		    if (!DEADMONSTER(mtmp) &&
			  (mtmp->data==&mons[PM_FLOATING_EYE]) && !mtmp->mcan) {
			if (!Free_action) {
/*JP
			    You("are frozen by %s gaze!",
*/
			    You("%sのにらみで動けなくなった！", 
					     s_suffix(mon_nam(mtmp)));
			    nomul((u.ulevel > 6 || rn2(4)) ?
				    -d((int)mtmp->m_lev+1,
					    (int)mtmp->data->mattk[0].damd)
				    : -200);
			    return 1;
			} else
/*JP
			    You("stiffen momentarily under %s gaze.",
*/
			    You("%sのにらみで一瞬硬直した。",
				    s_suffix(mon_nam(mtmp)));
		    }
		    /* Technically this one shouldn't affect you at all because
		     * the Medusa gaze is an active monster attack that only
		     * works on the monster's turn, but for it to *not* have an
		     * effect would be too weird.
		     */
		    if (!DEADMONSTER(mtmp) &&
			    (mtmp->data == &mons[PM_MEDUSA]) && !mtmp->mcan) {
			pline(
/*JP
			 "Gazing at the awake %s is not a very good idea.",
*/
			 "目を覚ましている%sをにらむのは賢いことじゃない。",
			    l_monnam(mtmp));
			/* as if gazing at a sleeping anything is fruitful... */
/*JP
			You("turn to stone...");
*/
			You("石になった…");
			killer_format = KILLED_BY;
/*JP
			killer = "deliberately meeting Medusa's gaze";
*/
			killer = "わざわざメデューサのにらみをまともに見て";
			done(STONING);
		    }
		}
	    }
	}
/*JP
	if (!looked) You("gaze at no place in particular.");
*/
	if (!looked) You("実際には何もにらめなかった。");
	return 1;
}

int
dohide()
{
	boolean ismimic = youmonst.data->mlet == S_MIMIC;

	if (u.uundetected || (ismimic && youmonst.m_ap_type != M_AP_NOTHING)) {
/*JP
		You("are already hiding.");
*/
		You("もう隠れている。");
		return(0);
	}
	if (ismimic) {
		/* should bring up a dialog "what would you like to imitate?" */
		youmonst.m_ap_type = M_AP_OBJECT;
		youmonst.mappearance = STRANGE_OBJECT;
	} else
		u.uundetected = 1;
	newsym(u.ux,u.uy);
	return(1);
}

int
domindblast()
{
	struct monst *mtmp, *nmon;

	if (u.uen < 10) {
/*JP
	    You("concentrate but lack the energy to maintain doing so.");
*/
	    You("集中した。しかしエネルギーが足りない。");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

/*JP
	You("concentrate.");
*/
	You("集中した。");
/*JP
	pline("A wave of psychic energy pours out.");
*/
	pline("精神エネルギー波が放散した。");
	for(mtmp=fmon; mtmp; mtmp = nmon) {
		int u_sen;

		nmon = mtmp->nmon;
		if (DEADMONSTER(mtmp))
			continue;
		if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM)
			continue;
		if(mtmp->mpeaceful)
			continue;
		u_sen = telepathic(mtmp->data) && !mtmp->mcansee;
		if (u_sen || (telepathic(mtmp->data) && rn2(2)) || !rn2(10)) {
#if 0 /*JP*/
			You("lock in on %s %s.", s_suffix(mon_nam(mtmp)),
				u_sen ? "telepathy" :
				telepathic(mtmp->data) ? "latent telepathy" :
				"mind");
#else
			pline("%sの%sり込んだ。", mon_nam(mtmp),
				u_sen ? "精神に入" :
				telepathic(mtmp->data) ? "潜在的精神に入" :
				"深層意識に潜");
#endif
			mtmp->mhp -= rnd(15);
			if (mtmp->mhp <= 0)
				killed(mtmp);
		}
	}
	return 1;
}

STATIC_OVL void
uunstick()
{
/*JP
	pline("%s is no longer in your clutches.", Monnam(u.ustuck));
*/
	pline("%sはあなたの手から逃れた。", Monnam(u.ustuck));
	u.ustuck = 0;
}

void
skinback(silently)
boolean silently;
{
	if (uskin) {
/*JP
		if (!silently) Your("skin returns to its original form.");
*/
		Your("皮膚は本来の姿に戻った。");
		uarm = uskin;
		uskin = (struct obj *)0;
		/* undo save/restore hack */
		uarm->owornmask &= ~I_SPECIAL;
	}
}

#endif /* OVLB */
#ifdef OVL1

const char *
mbodypart(mon, part)
struct monst *mon;
int part;
{
	static NEARDATA const char
#if 0 /*JP*/
	*humanoid_parts[] = { "arm", "eye", "face", "finger",
		"fingertip", "foot", "hand", "handed", "head", "leg",
		"light headed", "neck", "spine", "toe", "hair",
		"blood", "lung", "nose", "stomach"},
#else
	*humanoid_parts[] = { "腕", "目", "顔", "指",
		"指先", "足", "手", "手にする", "頭", "足",
		"めまいがした", "首", "背骨", "爪先", "髪",
		"血", "肺", "鼻", "胃"},
#endif
#if 0 /*JP*/
	*jelly_parts[] = { "pseudopod", "dark spot", "front",
		"pseudopod extension", "pseudopod extremity",
		"pseudopod root", "grasp", "grasped", "cerebral area",
		"lower pseudopod", "viscous", "middle", "surface",
		"pseudopod extremity", "ripples", "juices",
		"surface", "sensor", "stomach" },
#else
	*jelly_parts[] = { "擬似触手", "黒い斑点", "前面",
		"擬似触手の先", "擬似触手",
		"擬似触手の幹", "触手", "握る", "脳の領域",
		"下方の擬似触手", "ねばねばしてきた", "中間領域", "表面",
		"擬似触手", "波紋", "体液",
		"表面", "感覚器", "胃"},
#endif
#if 0 /*JP*/
	*animal_parts[] = { "forelimb", "eye", "face", "foreclaw", "claw tip",
		"rear claw", "foreclaw", "clawed", "head", "rear limb",
		"light headed", "neck", "spine", "rear claw tip",
		"fur", "blood", "lung", "nose", "stomach" },
#else
	*animal_parts[] = { "前足", "目", "顔", "前爪", "爪先",
		"後爪", "前爪", "ひっかける", "頭", "後足",
		"めまいがした", "首", "背骨", "後爪先",
		"毛皮", "血", "肺", "鼻", "胃"},
#endif
#if 0 /*JP*/
	*bird_parts[] = { "wing", "eye", "face", "wing", "wing tip",
		"foot", "wing", "winged", "head", "leg",
		"light headed", "neck", "spine", "toe",
		"feathers", "blood", "lung", "bill", "stomach" },
#else
	*bird_parts[] = { "翼", "目", "顔", "翼", "翼の先",
		"足", "翼", "翼にとる", "頭", "足",
		"めまいがした", "首", "背骨", "爪先",
		"羽毛", "血", "肺", "くちばし", "胃" },
#endif
#if 0 /*JP*/
	*horse_parts[] = { "foreleg", "eye", "face", "forehoof", "hoof tip",
		"rear hoof", "foreclaw", "hooved", "head", "rear leg",
		"light headed", "neck", "backbone", "rear hoof tip",
		"mane", "blood", "lung", "nose", "stomach"},
#else
	*horse_parts[] = { "前足", "目", "顔", "前蹄", "蹄",
		"後蹄", "前爪", "蹄にはさむ", "頭", "後足",
		"めまいがした", "首", "背骨", "後爪先",
		"たてがみ", "血", "肺", "鼻", "胃"},
#endif
#if 0 /*JP*/
	*sphere_parts[] = { "appendage", "optic nerve", "body", "tentacle",
		"tentacle tip", "lower appendage", "tentacle", "tentacled",
		"body", "lower tentacle", "rotational", "equator", "body",
		"lower tentacle tip", "cilia", "life force", "retina",
		"olfactory nerve", "interior" },
#else
	*sphere_parts[] = { "突起", "視覚神経", "体", "触手",
		"触手の先", "下の突起", "触手", "触手に持つ",
		"体", "下の触手", "回転した", "中心線", "体",
		"下の触手の先", "繊毛", "生命力", "網膜",
		"嗅覚中枢", "内部" },
#endif
#if 0 /*JP*/
	*fungus_parts[] = { "mycelium", "visual area", "front", "hypha",
		"hypha", "root", "strand", "stranded", "cap area",
		"rhizome", "sporulated", "stalk", "root", "rhizome tip",
		"spores", "juices", "gill", "gill", "interior" },
#else
	*fungus_parts[] = { "菌糸体", "視覚領域", "前", "菌糸",
		"菌糸", "根", "触手", "触手にからみつける", "傘",
		"根茎", "混乱する", "軸", "根", "根茎の先",
		"芽胞", "体液", "えら", "えら", "内部"},
#endif
#if 0 /*JP*/
	*vortex_parts[] = { "region", "eye", "front", "minor current",
		"minor current", "lower current", "swirl", "swirled",
		"central core", "lower current", "addled", "center",
		"currents", "edge", "currents", "life force",
		"center", "leading edge", "interior" },
#else
	*vortex_parts[] = { "領域", "目", "前", "小さい流れ",
		"小さい流れ", "下部の流れ", "渦巻", "渦に巻く",
		"渦の中心", "下部の流れ", "混乱した", "中心部",
		"流れ", "外周", "気流", "生命力",
		"中心", "前縁", "内部"},
#endif
#if 0 /*JP*/
	*snake_parts[] = { "vestigial limb", "eye", "face", "large scale",
		"large scale tip", "rear region", "scale gap", "scale gapped",
		"head", "rear region", "light headed", "neck", "length",
		"rear scale", "scales", "blood", "lung", "forked tongue", "stomach" },
#else
	*snake_parts[] = { "足の痕跡", "目", "顔", "大きな鱗",
		"大きな鱗の先", "後部分", "鱗の隙間", "鱗の隙間につける",
		"頭", "後部分", "めまいがした", "首", "体",
		"後部分の鎧", "鱗", "血", "肺", "舌", "胃"},
#endif
#if 0 /*JP*/
	*fish_parts[] = { "fin", "eye", "premaxillary", "pelvic axillary",
		"pelvic fin", "anal fin", "pectoral fin", "finned", "head", "peduncle",
		"played out", "gills", "dorsal fin", "caudal fin",
		"scales", "blood", "gill", "nostril", "stomach" };
#else
	*fish_parts[] = { "ひれ", "目", "顔", "ひれの先",
		"ひれの先", "尾びれ", "胸ひれ", "ひれで持つ", "頭", "尾びれ",
		"めまいがした", "えら", "背びれ", "尾びれ",
		"鱗", "血", "えら", "鼻", "胃" };
#endif /*JP*/
#if 0 /*JP*/
	/* claw attacks are overloaded in mons[]; most humanoids with
	   such attacks should still reference hands rather than claws */
	static const char not_claws[] = {
		S_HUMAN, S_MUMMY, S_ZOMBIE, S_ANGEL,
		S_NYMPH, S_LEPRECHAUN, S_QUANTMECH, S_VAMPIRE,
		S_ORC, S_GIANT,		/* quest nemeses */
		'\0'		/* string terminator; assert( S_xxx != 0 ); */
	};
#endif
	struct permonst *mptr = mon->data;

#if 0 /*JP*/
/* pawは犬とか猫の手、clawはタカの足のようなかぎつめ、
   どっちらも日本語じゃ「手」でいいでしょう。
   trunkは象の鼻を意味するそうです。日本語では単に鼻でいいかと。
*/
	if (part == HAND || part == HANDED) {	/* some special cases */
	    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE ||
		    mptr->mlet == S_YETI)
		return part == HAND ? "paw" : "pawed";
	    if (humanoid(mptr) && attacktype(mptr, AT_CLAW) &&
		    !index(not_claws, mptr->mlet) &&
		    mptr != &mons[PM_STONE_GOLEM] &&
		    mptr != &mons[PM_INCUBUS] && mptr != &mons[PM_SUCCUBUS])
		return part == HAND ? "claw" : "clawed";
	}
	if ((mptr == &mons[PM_MUMAK] || mptr == &mons[PM_MASTODON]) &&
		part == NOSE)
	    return "trunk";
#endif
	if (mptr == &mons[PM_SHARK] && part == HAIR)
#if 0 /*JP*/
	    return "skin";	/* sharks don't have scales */
#else
	    return "頭";	/* sharks don't have scales */
#endif
	if (mptr == &mons[PM_JELLYFISH] && (part == ARM || part == FINGER ||
	    part == HAND || part == FOOT || part == TOE))
#if 0 /*JP*/
	    return "tentacle";
#else
	    return "触手";
#endif
	if (mptr == &mons[PM_FLOATING_EYE] && part == EYE)
#if 0 /*JP*/
	    return "cornea";
#else
	    return "角膜";
#endif
	if (humanoid(mptr) &&
		(part == ARM || part == FINGER || part == FINGERTIP ||
		    part == HAND || part == HANDED))
	    return humanoid_parts[part];
	if (mptr == &mons[PM_RAVEN])
	    return bird_parts[part];
	if (mptr->mlet == S_CENTAUR || mptr->mlet == S_UNICORN ||
		(mptr == &mons[PM_ROTHE] && part != HAIR))
	    return horse_parts[part];
	if (mptr->mlet == S_LIGHT) {
#if 0 /*JP*/
		if (part == HANDED) return "rayed";
		else if (part == ARM || part == FINGER ||
				part == FINGERTIP || part == HAND) return "ray";
		else return "beam";
#else
		if (part == HANDED || part == ARM || part == FINGER ||
				part == FINGERTIP || part == HAND) {
		    return "光";
		}
#endif
	}
	if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH])
	    return fish_parts[part];
	if (slithy(mptr) || (mptr->mlet == S_DRAGON && part == HAIR))
	    return snake_parts[part];
	if (mptr->mlet == S_EYE)
	    return sphere_parts[part];
	if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING ||
		mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
	    return jelly_parts[part];
	if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
	    return vortex_parts[part];
	if (mptr->mlet == S_FUNGUS)
	    return fungus_parts[part];
	if (humanoid(mptr))
	    return humanoid_parts[part];
	return animal_parts[part];
}

const char *
body_part(part)
int part;
{
	return mbodypart(&youmonst, part);
}

#endif /* OVL1 */
#ifdef OVL0

int
poly_gender()
{
/* Returns gender of polymorphed player; 0/1=same meaning as flags.female,
 * 2=none.
 */
	if (is_neuter(youmonst.data) || !humanoid(youmonst.data)) return 2;
	return flags.female;
}

#endif /* OVL0 */
#ifdef OVLB

void
ugolemeffects(damtype, dam)
int damtype, dam;
{
	int heal = 0;
	/* We won't bother with "slow"/"haste" since players do not
	 * have a monster-specific slow/haste so there is no way to
	 * restore the old velocity once they are back to human.
	 */
	if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
		return;
	switch (damtype) {
		case AD_ELEC: if (u.umonnum == PM_FLESH_GOLEM)
				heal = dam / 6; /* Approx 1 per die */
			break;
		case AD_FIRE: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam;
			break;
	}
	if (heal && (u.mh < u.mhmax)) {
		u.mh += heal;
		if (u.mh > u.mhmax) u.mh = u.mhmax;
		flags.botl = 1;
/*JP
		pline("Strangely, you feel better than before.");
*/
		pline("奇妙なことに、前より気分がよくなった。");
		exercise(A_STR, TRUE);
	}
}

STATIC_OVL int
armor_to_dragon(atyp)
int atyp;
{
	switch(atyp) {
	    case GRAY_DRAGON_SCALE_MAIL:
	    case GRAY_DRAGON_SCALES:
		return PM_GRAY_DRAGON;
	    case SILVER_DRAGON_SCALE_MAIL:
	    case SILVER_DRAGON_SCALES:
		return PM_SILVER_DRAGON;
#if 0	/* DEFERRED */
	    case SHIMMERING_DRAGON_SCALE_MAIL:
	    case SHIMMERING_DRAGON_SCALES:
		return PM_SHIMMERING_DRAGON;
#endif
	    case RED_DRAGON_SCALE_MAIL:
	    case RED_DRAGON_SCALES:
		return PM_RED_DRAGON;
	    case ORANGE_DRAGON_SCALE_MAIL:
	    case ORANGE_DRAGON_SCALES:
		return PM_ORANGE_DRAGON;
	    case WHITE_DRAGON_SCALE_MAIL:
	    case WHITE_DRAGON_SCALES:
		return PM_WHITE_DRAGON;
	    case BLACK_DRAGON_SCALE_MAIL:
	    case BLACK_DRAGON_SCALES:
		return PM_BLACK_DRAGON;
	    case BLUE_DRAGON_SCALE_MAIL:
	    case BLUE_DRAGON_SCALES:
		return PM_BLUE_DRAGON;
	    case GREEN_DRAGON_SCALE_MAIL:
	    case GREEN_DRAGON_SCALES:
		return PM_GREEN_DRAGON;
	    case YELLOW_DRAGON_SCALE_MAIL:
	    case YELLOW_DRAGON_SCALES:
		return PM_YELLOW_DRAGON;
	    default:
		return -1;
	}
}

#endif /* OVLB */

/*polyself.c*/
