/*	SCCS Id: @(#)potion.c	3.4	2002/10/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

#ifdef OVLB
boolean notonhead = FALSE;

static NEARDATA int nothing, unkn;
static NEARDATA const char beverages[] = { POTION_CLASS, 0 };

STATIC_DCL long FDECL(itimeout, (long));
STATIC_DCL long FDECL(itimeout_incr, (long,int));
STATIC_DCL void NDECL(ghost_from_bottle);
STATIC_DCL short FDECL(mixtype, (struct obj *,struct obj *));

/* force `val' to be within valid range for intrinsic timeout value */
STATIC_OVL long
itimeout(val)
long val;
{
    if (val >= TIMEOUT) val = TIMEOUT;
    else if (val < 1) val = 0;

    return val;
}

/* increment `old' by `incr' and force result to be valid intrinsic timeout */
STATIC_OVL long
itimeout_incr(old, incr)
long old;
int incr;
{
    return itimeout((old & TIMEOUT) + (long)incr);
}

/* set the timeout field of intrinsic `which' */
void
set_itimeout(which, val)
long *which, val;
{
    *which &= ~TIMEOUT;
    *which |= itimeout(val);
}

/* increment the timeout field of intrinsic `which' */
void
incr_itimeout(which, incr)
long *which;
int incr;
{
    set_itimeout(which, itimeout_incr(*which, incr));
}

void
make_confused(xtime,talk)
long xtime;
boolean talk;
{
	long old = HConfusion;

	if (!xtime && old) {
		if (talk)
#if 0 /*JP*/
		    You_feel("less %s now.",
			Hallucination ? "trippy" : "confused");
#else
		    You("%s�������ޤä���",
			Hallucination ? "�إ�إ�" : "����");
#endif
	}
	if ((xtime && !old) || (!xtime && old)) flags.botl = TRUE;

	set_itimeout(&HConfusion, xtime);
}

void
make_stunned(xtime,talk)
long xtime;
boolean talk;
{
	long old = HStun;

	if (!xtime && old) {
		if (talk)
#if 0 /*JP*/
		    You_feel("%s now.",
			Hallucination ? "less wobbly" : "a bit steadier");
#else
		    You_feel("%s��",
			Hallucination ? "�ؤ��餬�����ޤä�" : "������󤷤ä��ꤷ�Ƥ���");
#endif
	}
	if (xtime && !old) {
		if (talk) {
#ifdef STEED
			if (u.usteed)
/*JP
				You("wobble in the saddle.");
*/
				You("�Ȥξ�Ǥ��餰�餷����");
			else
#endif
/*JP
			You("%s...", stagger(youmonst.data, "stagger"));
*/
			You("%s��", stagger(youmonst.data, "���餯�餷��"));
		}
	}
	if ((!xtime && old) || (xtime && !old)) flags.botl = TRUE;

	set_itimeout(&HStun, xtime);
}

void
make_sick(xtime, cause, talk, type)
long xtime;
const char *cause;	/* sickness cause */
boolean talk;
int type;
{
	long old = Sick;

	if (xtime > 0L) {
	    if (Sick_resistance) return;
	    if (!old) {
		/* newly sick */
/*JP
		You_feel("deathly sick.");
*/
		You("��̤ۤɵ�ʬ��������");
	    } else {
		/* already sick */
#if 0 /*JP*/
		if (talk) You_feel("%s worse.",
			      xtime <= Sick/2L ? "much" : "even");
#else
		if (talk) You("%s���������褦�ʵ������롣",
			      xtime <= Sick/2L ? "�����" : "��ä�");
#endif
	    }
	    set_itimeout(&Sick, xtime);
	    u.usick_type |= type;
	    flags.botl = TRUE;
	} else if (old && (type & u.usick_type)) {
	    /* was sick, now not */
	    u.usick_type &= ~type;
	    if (u.usick_type) { /* only partly cured */
/*JP
		if (talk) You_feel("somewhat better.");
*/
		if (talk) You("����äȤ褯�ʤä���");
		set_itimeout(&Sick, Sick * 2); /* approximation */
	    } else {
/*JP
		if (talk) pline("What a relief!");
*/
		if (talk) pline("���������ä���");
		Sick = 0L;		/* set_itimeout(&Sick, 0L) */
	    }
	    flags.botl = TRUE;
	}

	if (Sick) {
	    exercise(A_CON, FALSE);
	    if (cause) {
		(void) strncpy(u.usick_cause, cause, sizeof(u.usick_cause));
		u.usick_cause[sizeof(u.usick_cause)-1] = 0;
		}
	    else
		u.usick_cause[0] = 0;
	} else
	    u.usick_cause[0] = 0;
}

void
make_vomiting(xtime, talk)
long xtime;
boolean talk;
{
	long old = Vomiting;

	if(!xtime && old)
/*JP
	    if(talk) You_feel("much less nauseated now.");
*/
	    if(talk) You("�Ǥ����������ޤä���");

	set_itimeout(&Vomiting, xtime);
}

/*JP
static const char vismsg[] = "vision seems to %s for a moment but is %s now.";
*/
static const char vismsg[] = "�볦�ϰ��%s�ʤä����ޤ�%s�ʤä���";
/*JP
static const char eyemsg[] = "%s momentarily %s.";
*/
static const char eyemsg[] = "%s�ϰ��%s��";

void
make_blinded(xtime, talk)
long xtime;
boolean talk;
{
	long old = Blinded;
	boolean u_could_see, can_see_now;
#if 0 /*JP*/
	int eyecnt;
	char buf[BUFSZ];
#endif

	/* we need to probe ahead in case the Eyes of the Overworld
	   are or will be overriding blindness */
	u_could_see = !Blind;
	Blinded = xtime ? 1L : 0L;
	can_see_now = !Blind;
	Blinded = old;		/* restore */

	if (u.usleep) talk = FALSE;

	if (can_see_now && !u_could_see) {	/* regaining sight */
	    if (talk) {
		if (Hallucination)
/*JP
		    pline("Far out!  Everything is all cosmic again!");
*/
		    pline("�����ʤˤ⤫�⤬�ޤ������˸����롪");
		else
/*JP
		    You("can see again.");
*/
		    You("�ޤ�������褦�ˤʤä���");
	    }
	} else if (old && !xtime) {
	    /* clearing temporary blindness without toggling blindness */
	    if (talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blindfolded) {
#if 0 /*JP*/
		    Strcpy(buf, body_part(EYE));
		    eyecnt = eyecount(youmonst.data);
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "itches" : "itch");
#else
		    Your(eyemsg, body_part(EYE), "���椯�ʤä�");
#endif
		} else {	/* Eyes of the Overworld */
#if 0 /*JP*/
		    Your(vismsg, "brighten",
			 Hallucination ? "sadder" : "normal");
#else
		    Your(vismsg, "���뤯",
			 Hallucination ? "���Ť�" : "���̤�");
#endif
		}
	    }
	}

	if (u_could_see && !can_see_now) {	/* losing sight */
	    if (talk) {
		if (Hallucination)
/*JP
		    pline("Oh, bummer!  Everything is dark!  Help!");
*/
		    pline("�Ť��衼�������衼�������衼��");
		else
/*JP
		    pline("A cloud of darkness falls upon you.");
*/
		    pline("�Ź��α������ʤ���ʤ�ä���");
	    }
	    /* Before the hero goes blind, set the ball&chain variables. */
	    if (Punished) set_bc(0);
	} else if (!old && xtime) {
	    /* setting temporary blindness without toggling blindness */
	    if (talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blindfolded) {
#if 0 /*JP*/
		    Strcpy(buf, body_part(EYE));
		    eyecnt = eyecount(youmonst.data);
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "twitches" : "twitch");
#else
		    Your(eyemsg, body_part(EYE), "�ԥ��ԥ�����");
#endif
		} else {	/* Eyes of the Overworld */
#if 0 /*JP*/
		    Your(vismsg, "dim",
			 Hallucination ? "happier" : "normal");
#else
		    Your(vismsg, "���Ť�",
			 Hallucination ? "�ϥåԡ���" : "���̤�");
#endif
		}
	    }
	}

	set_itimeout(&Blinded, xtime);

	if (u_could_see ^ can_see_now) {  /* one or the other but not both */
	    flags.botl = 1;
	    vision_full_recalc = 1;	/* blindness just got toggled */
	    if (Blind_telepat || Infravision) see_monsters();
	}
}

boolean
make_hallucinated(xtime, talk, mask)
long xtime;	/* nonzero if this is an attempt to turn on hallucination */
boolean talk;
long mask;	/* nonzero if resistance status should change by mask */
{
	long old = HHallucination;
	boolean changed = 0;
	const char *message, *verb;

#if 0 /*JP*/
	message = (!xtime) ? "Everything %s SO boring now." :
			     "Oh wow!  Everything %s so cosmic!";
#else
	message = (!xtime) ? "���⤫�⤬���������%s�롣" :
			     "��������⤫��������%s�롪";
#endif
/*JP
	verb = (!Blind) ? "looks" : "feels";
*/
	verb = (!Blind) ? "����" : "����";

	if (mask) {
	    if (HHallucination) changed = TRUE;

	    if (!xtime) EHalluc_resistance |= mask;
	    else EHalluc_resistance &= ~mask;
	} else {
	    if (!EHalluc_resistance && (!!HHallucination != !!xtime))
		changed = TRUE;
	    set_itimeout(&HHallucination, xtime);

	    /* clearing temporary hallucination without toggling vision */
	    if (!changed && !HHallucination && old && talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blind) {
#if 0 /*JP*/
		    char buf[BUFSZ];
		    int eyecnt = eyecount(youmonst.data);

		    Strcpy(buf, body_part(EYE));
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "itches" : "itch");
#else
		    Your(eyemsg, body_part(EYE), "���椯�ʤä�");
#endif
		} else {	/* Grayswandir */
/*JP
		    Your(vismsg, "flatten", "normal");
*/
		    Your(vismsg, "��������", "���̤�");
		}
	    }
	}

	if (changed) {
	    if (u.uswallow) {
		swallowed(0);	/* redraw swallow display */
	    } else {
		/* The see_* routines should be called *before* the pline. */
		see_monsters();
		see_objects();
		see_traps();
	    }

	    /* for perm_inv and anything similar
	    (eg. Qt windowport's equipped items display) */
	    update_inventory();

	    flags.botl = 1;
	    if (talk) pline(message, verb);
	}
	return changed;
}

STATIC_OVL void
ghost_from_bottle()
{
	struct monst *mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, NO_MM_FLAGS);

	if (!mtmp) {
/*JP
		pline("This bottle turns out to be empty.");
*/
		pline("�Ӥ϶��äݤ��ä���");
		return;
	}
	if (Blind) {
/*JP
		pline("As you open the bottle, %s emerges.", something);
*/
		pline("�Ӥ򳫤���ȡ��������ФƤ�����");
		return;
	}
/*JP
	pline("As you open the bottle, an enormous %s emerges!",
*/
	pline("�Ӥ򳫤���ȡ������%s���ФƤ�����",
/*JP
		Hallucination ? rndmonnam() : (const char *)"ghost");
*/
		Hallucination ? rndmonnam() : (const char *)"ͩ��");
	if(flags.verbose)
/*JP
	    You("are frightened to death, and unable to move.");
*/
	    You("�ޤä����ˤʤäƶä���ư���ʤ��ʤä���");
	nomul(-3);
/*JP
	nomovemsg = "You regain your composure.";
*/
	nomovemsg = "���ʤ���ʿ�Ť����ᤷ����";
}

/* "Quaffing is like drinking, except you spill more."  -- Terry Pratchett
 */
int
dodrink()
{
	register struct obj *otmp;
	const char *potion_descr;

	if (Strangled) {
/*JP
		pline("If you can't breathe air, how can you drink liquid?");
*/
		pline("©��Ǥ��ʤ��Τˡ��ɤ���äƱ��Τ����������");
		return 0;
	}
	/* Is there a fountain to drink from here? */
	if (IS_FOUNTAIN(levl[u.ux][u.uy].typ) && !Levitation) {
/*JP
		if(yn("Drink from the fountain?") == 'y') {
*/
		if(yn("���ο����ߤޤ�����") == 'y') {
			drinkfountain();
			return 1;
		}
	}
#ifdef SINKS
	/* Or a kitchen sink? */
	if (IS_SINK(levl[u.ux][u.uy].typ)) {
/*JP
		if (yn("Drink from the sink?") == 'y') {
*/
		if (yn("ή����ο����ߤޤ�����") == 'y') {
			drinksink();
			return 1;
		}
	}
#endif

	/* Or are you surrounded by water? */
	if (Underwater) {
/*JP
		if (yn("Drink the water around you?") == 'y') {
*/
		if (yn("�ޤ��ο����ߤޤ�����") == 'y') {
/*JP
		    pline("Do you know what lives in this water!");
*/
		    pline("���ο���ǲ��������Ƥ���Τ��ΤäƤ뤫����");
			return 1;
		}
	}

	otmp = getobj(beverages, "drink");
	if(!otmp) return(0);
	otmp->in_use = TRUE;		/* you've opened the stopper */

#define POTION_OCCUPANT_CHANCE(n) (13 + 2*(n))	/* also in muse.c */

	potion_descr = OBJ_DESCR(objects[otmp->otyp]);
	if (potion_descr) {
	    if (!strcmp(potion_descr, "milky") &&
		    flags.ghost_count < MAXMONNO &&
		    !rn2(POTION_OCCUPANT_CHANCE(flags.ghost_count))) {
		ghost_from_bottle();
		useup(otmp);
		return(1);
	    } else if (!strcmp(potion_descr, "smoky") &&
		    flags.djinni_count < MAXMONNO &&
		    !rn2(POTION_OCCUPANT_CHANCE(flags.djinni_count))) {
		djinni_from_bottle(otmp);
		useup(otmp);
		return(1);
	    }
	}
	return dopotion(otmp);
}

int
dopotion(otmp)
register struct obj *otmp;
{
	int retval;

	otmp->in_use = TRUE;
	nothing = unkn = 0;
	if((retval = peffects(otmp)) >= 0) return(retval);

	if(nothing) {
	    unkn++;
#if 0 /*JP*/
	    You("have a %s feeling for a moment, then it passes.",
		  Hallucination ? "normal" : "peculiar");
#else
	    You("%s��ʬ�ˤ�����줿���������˾ä����ä���",
		  Hallucination ? "���̤�" : "���ä�");
#endif
	}
	if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
		if(!unkn) {
			makeknown(otmp->otyp);
			more_experienced(0,10);
		} else if(!objects[otmp->otyp].oc_uname)
			docall(otmp);
	}
	useup(otmp);
	return(1);
}

int
peffects(otmp)
	register struct obj	*otmp;
{
	register int i, ii, lim;

	switch(otmp->otyp){
	case POT_RESTORE_ABILITY:
	case SPE_RESTORE_ABILITY:
		unkn++;
		if(otmp->cursed) {
/*JP
		    pline("Ulch!  This makes you feel mediocre!");
*/
		    pline("�����󡢤ɤ��⤵���ʤ��ʤ���");
		    break;
		} else {
#if 0 /*JP*/
		    pline("Wow!  This makes you feel %s!",
			  (otmp->blessed) ?
				(unfixable_trouble_count(FALSE) ? "better" : "great")
			  : "good");
#else
		    pline("�������ʬ��%s�ʤä���",
			  (otmp->blessed) ?
				(unfixable_trouble_count(FALSE) ? "�����֤褯" : "�ȤƤ�褯")
			  : "�褯");
#endif
		    i = rn2(A_MAX);		/* start at a random point */
		    for (ii = 0; ii < A_MAX; ii++) {
			lim = AMAX(i);
			if (i == A_STR && u.uhs >= 3) --lim;	/* WEAK */
			if (ABASE(i) < lim) {
			    ABASE(i) = lim;
			    flags.botl = 1;
			    /* only first found if not blessed */
			    if (!otmp->blessed) break;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_HALLUCINATION:
		if (Hallucination || Halluc_resistance) nothing++;
		(void) make_hallucinated(itimeout_incr(HHallucination,
					   rn1(200, 600 - 300 * bcsign(otmp))),
				  TRUE, 0L);
		break;
	case POT_WATER:
		if(!otmp->blessed && !otmp->cursed) {
/*JP
		    pline("This tastes like water.");
*/
		    pline("��Τ褦��̣�����롣");
		    u.uhunger += rnd(10);
		    newuhs(FALSE);
		    break;
		}
		unkn++;
		if(is_undead(youmonst.data) || is_demon(youmonst.data) ||
				u.ualign.type == A_CHAOTIC) {
		    if(otmp->blessed) {
/*JP
			pline("This burns like acid!");
*/
			pline("���Τ褦���夬�Ҥ�Ҥꤹ�롪");
			exercise(A_CON, FALSE);
			if (u.ulycn >= LOW_PM) {
/*JP
			    Your("affinity to %s disappears!",
*/
			    Your("%s�ؤοƶᴶ�Ϥʤ��ʤä���",
				 makeplural(mons[u.ulycn].mname));
			    if (youmonst.data == &mons[u.ulycn])
				you_unwere(FALSE);
			    u.ulycn = NON_PM;	/* cure lycanthropy */
			}
/*JP
			losehp(d(2,6), "potion of holy water", KILLED_BY_AN);
*/
			losehp(d(2,6), "�����", KILLED_BY_AN);
		    } else if(otmp->cursed) {
/*JP
			You_feel("quite proud of yourself.");
*/
			You("��º���򴶤�����");
			healup(d(2,6),0,0,0);
			if (u.ulycn >= LOW_PM && !Upolyd) you_were();
			exercise(A_CON, TRUE);
		    }
		} else {
		    if(otmp->blessed) {
/*JP
			You_feel("full of awe.");
*/
			You("���ݤ�ǰ�ˤ���줿��");
			make_sick(0L, (char *) 0, TRUE, SICK_ALL);
			exercise(A_WIS, TRUE);
			exercise(A_CON, TRUE);
			if (u.ulycn >= LOW_PM)
			    you_unwere(TRUE);	/* "Purified" */
			/* make_confused(0L,TRUE); */
		    } else {
			if(u.ualign.type == A_LAWFUL) {
/*JP
			    pline("This burns like acid!");
*/
			    pline("���Τ褦���夬�Ҥ�Ҥꤹ�롪");
#if 0 /*JP*/
			    losehp(d(2,6), "potion of unholy water",
				KILLED_BY_AN);
#else
			    losehp(d(2,6), "�Ծ����",
				KILLED_BY_AN);
#endif
			} else
/*JP
			    You_feel("full of dread.");
*/
			    You("���ݤ�ǰ�ˤ���줿��");
			if (u.ulycn >= LOW_PM && !Upolyd) you_were();
			exercise(A_CON, FALSE);
		    }
		}
		break;
	case POT_BOOZE:
		unkn++;
#if 0 /*JP*/
		pline("Ooph!  This tastes like %s%s!",
		      otmp->odiluted ? "watered down " : "",
		      Hallucination ? "dandelion wine" : "liquid fire");
#else
		pline("�����äס������%s%s�Τ褦��̣�����롪",
		      otmp->odiluted ? "������᤿" : "",
		      Hallucination ? "����ݥݥ磻��" : "ǳ��������");
#endif
		if (!otmp->blessed)
		    make_confused(itimeout_incr(HConfusion, d(3,8)), FALSE);
		/* the whiskey makes us feel better */
		if (!otmp->odiluted) healup(1, 0, FALSE, FALSE);
		u.uhunger += 10 * (2 + bcsign(otmp));
		newuhs(FALSE);
		exercise(A_WIS, FALSE);
		if(otmp->cursed) {
/*JP
			You("pass out.");
*/
			You("���䤷����");
			multi = -rnd(15);
/*JP
			nomovemsg = "You awake with a headache.";
*/
			nomovemsg = "�ܤ����᤿��Ƭ�ˤ����롣";
		}
		break;
	case POT_ENLIGHTENMENT:
		if(otmp->cursed) {
			unkn++;
/*JP
			You("have an uneasy feeling...");
*/
			You("�԰¤ʵ����ˤʤä���");
			exercise(A_WIS, FALSE);
		} else {
			if (otmp->blessed) {
				(void) adjattrib(A_INT, 1, FALSE);
				(void) adjattrib(A_WIS, 1, FALSE);
			}
/*JP
			You_feel("self-knowledgeable...");
*/
			You("��ʬ���Ȥ�Ƚ��褦�˴�������");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
/*JP
			pline_The("feeling subsides.");
*/
			pline("���δ����Ϥʤ��ʤä���");
			exercise(A_WIS, TRUE);
		}
		break;
	case SPE_INVISIBILITY:
		/* spell cannot penetrate mummy wrapping */
		if (BInvis && uarmc->otyp == MUMMY_WRAPPING) {
/*JP
			You_feel("rather itchy under your %s.", xname(uarmc));
*/
			You("%s�β����ॺ�ॺ������", xname(uarmc));
			break;
		}
		/* FALLTHRU */
	case POT_INVISIBILITY:
		if (Invis || Blind || BInvis) {
		    nothing++;
		} else {
		    self_invis_message();
		}
		if (otmp->blessed) HInvis |= FROMOUTSIDE;
		else incr_itimeout(&HInvis, rn1(15,31));
		newsym(u.ux,u.uy);	/* update position */
		if(otmp->cursed) {
/*JP
		    pline("For some reason, you feel your presence is known.");
*/
		    pline("�ʤ�����¸�ߤ��Τ��Ƥ���褦�˴�������");
		    aggravate();
		}
		break;
	case POT_SEE_INVISIBLE:
		/* tastes like fruit juice in Rogue */
	case POT_FRUIT_JUICE:
	    {
		int msg = Invisible && !Blind;

		unkn++;
		if (otmp->cursed)
#if 0 /*JP*/
		    pline("Yecch!  This tastes %s.",
			  Hallucination ? "overripe" : "rotten");
#else
		    pline("�������������%s���塼����̣�����롣",
			  Hallucination ? "�Ϥ�������" : "��ä�");
#endif
		else
#if 0 /*JP*/
		    pline(Hallucination ?
		      "This tastes like 10%% real %s%s all-natural beverage." :
				"This tastes like %s%s.",
			  otmp->odiluted ? "reconstituted " : "",
			  fruitname(TRUE));
#else
		    pline(Hallucination ?
			   "10%%%s�ν㼫�������Τ褦��̣�����롣" :
			   "%s%s���塼���Τ褦��̣�����롣",
			  otmp->odiluted ? "��ʬĴ�����줿" : "", pl_fruit);
#endif
		if (otmp->otyp == POT_FRUIT_JUICE) {
		    u.uhunger += (otmp->odiluted ? 5 : 10) * (2 + bcsign(otmp));
		    newuhs(FALSE);
		    break;
		}
		if (!otmp->cursed) {
			/* Tell them they can see again immediately, which
			 * will help them identify the potion...
			 */
			make_blinded(0L,TRUE);
		}
		if (otmp->blessed)
			HSee_invisible |= FROMOUTSIDE;
		else
			incr_itimeout(&HSee_invisible, rn1(100,750));
		set_mimic_blocking(); /* do special mimic handling */
		see_monsters();	/* see invisible monsters */
		newsym(u.ux,u.uy); /* see yourself! */
		if (msg && !Blind) { /* Blind possible if polymorphed */
/*JP
		    You("can see through yourself, but you are visible!");
*/
		    You("Ʃ���Ǥ��롣������������褦�ˤʤä���");
		    unkn--;
		}
		break;
	    }
	case POT_PARALYSIS:
		if (Free_action)
/*JP
		    You("stiffen momentarily.");
*/
		    You("���ư���ʤ��ʤä���");             
		else {
		    if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
/*JP
			You("are motionlessly suspended.");
*/
			You("�����ư���ʤ��ʤä���");
#ifdef STEED
		    else if (u.usteed)
/*JP
			You("are frozen in place!");
*/
			You("���ξ��ư���ʤ��ʤä���");
#endif
		    else
#if 0 /*JP*/
			Your("%s are frozen to the %s!",
			     makeplural(body_part(FOOT)), surface(u.ux, u.uy));
#else
			You("ư���ʤ��ʤä���");
#endif
		    nomul(-(rn1(10, 25 - 12*bcsign(otmp))));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
		}
		break;
	case POT_SLEEPING:
		if(Sleep_resistance || Free_action)
/*JP
		    You("yawn.");
*/
		    You("�����Ӥ򤷤���");
#ifdef FELPURR
		else if Race_if(PM_FELPURR) {
		    pline("����̲�äƤ��ޤä���");
		    fall_asleep(-rn1(10, 35 - 12*bcsign(otmp)), TRUE);
		    }
#endif
		else {
/*JP
		    You("suddenly fall asleep!");
*/
		    pline("����̲�äƤ��ޤä���");
		    fall_asleep(-rn1(10, 25 - 12*bcsign(otmp)), TRUE);
		}
		break;
	case POT_MONSTER_DETECTION:
	case SPE_DETECT_MONSTERS:
		if (otmp->blessed) {
		    int x, y;

		    if (Detect_monsters) nothing++;
		    unkn++;
		    /* after a while, repeated uses become less effective */
		    if (HDetect_monsters >= 300L)
			i = 1;
		    else
			i = rn1(40,21);
		    incr_itimeout(&HDetect_monsters, i);
		    for (x = 1; x < COLNO; x++) {
			for (y = 0; y < ROWNO; y++) {
			    if (levl[x][y].glyph == GLYPH_INVISIBLE) {
				unmap_object(x, y);
				newsym(x,y);
			    }
			    if (MON_AT(x,y)) unkn = 0;
			}
		    }
		    see_monsters();
/*JP
		    if (unkn) You_feel("lonely.");
*/
		    if (unkn) You("���٤��ʤä���");
		    break;
		}
		if (monster_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
		break;
	case POT_OBJECT_DETECTION:
	case SPE_DETECT_TREASURE:
		if (object_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
		break;
	case POT_POISON:
/*JP
		pline("Yecch!  This stuff tastes like poison.");
*/
		pline("���������ǤΤ褦��̣�����롣");
		char killerbuf[64];
		sprintf(killerbuf, "ͭ�Ǥ�%s",
			(otmp->fromsink) ? "��ƻ��" : fruitname(TRUE));
		if (otmp->blessed) {
#if 0 /*JP*/
		    pline("(But in fact it was mildly stale %s.)",
			  fruitname(TRUE));
#else
		    pline("(�������ºݤ���Ͼ����Ť��ʤä�%s��)",
			  fruitname(TRUE));
#endif
		    if (!Poison_resistance)
			losehp(1, killerbuf, KILLED_BY);
		} else {
		    if(Poison_resistance && !Role_if(PM_HEALER))
                       pline("���ʤ���ͭ�Ǥ�%s�ˤ��ޤ�������ʤ���",
			    fruitname(TRUE));
		    if (Role_if(PM_HEALER))
/*JP
			pline("Fortunately, you have been immunized.");
*/
			pline("�����ʤ��Ȥˡ����ʤ����ȱ֤����롣");
		    else {
			int typ = rn2(A_MAX);

			if (!Fixed_abil) {
			    poisontell(typ);
			    (void) adjattrib(typ,
			    		Poison_resistance ? -1 : -rn1(4,3),
			    		TRUE);
			}
			if(!Poison_resistance)
			    losehp(rnd(10)+5*!!(otmp->cursed),
				   killerbuf, KILLED_BY);
			exercise(A_CON, FALSE);
		    }
		}
		if(Hallucination) {
/*JP
			You("are shocked back to your senses!");
*/
			You("�޴��˾׷���������");
			(void) make_hallucinated(0L,FALSE,0L);
		}
		break;
	case POT_SICKNESS:
		pline("����������%s����äƤ��롪",
		      (otmp->fromsink) ? "��" : fruitname(TRUE));
		if(Sick_resistance) {
		    pline_The("��ʪ��Ū�˱������줿%s�Ϥ��ʤ����µ��ˤ��ʤ���",
			      (otmp->fromsink) ? "��" : "���塼��");
		    break;
		}
		long sick_time;
		sprintf(killerbuf, "�������줿%s",
			(otmp->fromsink) ? "ή��" : "��");
		make_sick(Sick ? 1L :
			  (otmp->blessed) ? 20L :
			  (otmp->cursed) ? 10L :
			  (long) rn1(10, 10),
			  killerbuf, TRUE, SICK_VOMITABLE);
		break;
	case POT_CONFUSION:
		if(!Confusion)
		    if (Hallucination) {
/*JP
			pline("What a trippy feeling!");
*/
			pline("�ʤ󤫥إ�إ��롪");
			unkn++;
		    } else
/*JP
			pline("Huh, What?  Where am I?");
*/
			pline("�ۤ������ï��");
		else	nothing++;
		make_confused(itimeout_incr(HConfusion,
					    rn1(7, 16 - 8 * bcsign(otmp))),
			      FALSE);
		break;
	case POT_GAIN_ABILITY:
		if(otmp->cursed) {
/*JP
		    pline("Ulch!  That potion tasted foul!");
*/
		    pline("���������������롪");
		    unkn++;
		} else if (Fixed_abil) {
		    nothing++;
		} else {      /* If blessed, increase all; if not, try up to */
		    int itmp; /* 6 times to find one which can be increased. */
		    i = -1;		/* increment to 0 */
		    for (ii = A_MAX; ii > 0; ii--) {
			i = (otmp->blessed ? i + 1 : rn2(A_MAX));
			/* only give "your X is already as high as it can get"
			   message on last attempt (except blessed potions) */
			itmp = (otmp->blessed || ii == 1) ? 0 : -1;
			if (adjattrib(i, 1, itmp) && !otmp->blessed)
			    break;
		    }
		}
		break;
	case POT_SPEED:
		if(Wounded_legs && !otmp->cursed
#ifdef STEED
		   && !u.usteed	/* heal_legs() would heal steeds legs */
#endif
						) {
			heal_legs();
			unkn++;
			break;
		} /* and fall through */
	case SPE_HASTE_SELF:
		if(!Very_fast) /* wwf@doe.carleton.ca */
#if 0 /*JP*/
			You("are suddenly moving %sfaster.",
				Fast ? "" : "much ");
#else
			You("����%s®����ư�Ǥ���褦�ˤʤä���",
				Fast ? "" : "�ȤƤ�");
#endif
		else {
/*JP
			Your("%s get new energy.",
*/
			pline("%s�˥��ͥ륮���������ޤ��褦�ʴ�����������",
				makeplural(body_part(LEG)));
			unkn++;
		}
		exercise(A_DEX, TRUE);
		incr_itimeout(&HFast, rn1(10, 100 + 60 * bcsign(otmp)));
		break;
	case POT_BLINDNESS:
		if(Blind) nothing++;
		make_blinded(itimeout_incr(Blinded,
					   rn1(200, 250 - 125 * bcsign(otmp))),
			     (boolean)!Blind);
		break;
	case POT_GAIN_LEVEL:
		if (otmp->cursed) {
			unkn++;
			/* they went up a level */
			if((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
				Can_rise_up(u.ux, u.uy, &u.uz)) {
/*JP
			    const char *riseup ="rise up, through the %s!";
*/
			    const char *riseup ="%s���ͤ�ȴ������";
			    if(ledger_no(&u.uz) == 1) {
			        You(riseup, ceiling(u.ux,u.uy));
				goto_level(&earth_level, FALSE, FALSE, FALSE);
			    } else {
			        register int newlev = depth(&u.uz)-1;
				d_level newlevel;

				get_level(&newlevel, newlev);
				if(on_level(&newlevel, &u.uz)) {
/*JP
				    pline("It tasted bad.");
*/
				    pline("�ȤƤ�ޤ�����");
				    break;
				} else You(riseup, ceiling(u.ux,u.uy));
				goto_level(&newlevel, FALSE, FALSE, FALSE);
			    }
			}
/*JP
			else You("have an uneasy feeling.");
*/
			else You("�԰¤ʵ����ˤʤä���");
			break;
		}
		pluslvl(FALSE);
		if (otmp->blessed)
			/* blessed potions place you at a random spot in the
			 * middle of the new level instead of the low point
			 */
			u.uexp = rndexp(TRUE);
		break;
	case POT_HEALING:
/*JP
		You_feel("better.");
*/
		You("��ʬ���褯�ʤä���");
		healup(d(6 + 2 * bcsign(otmp), 4),
		       !otmp->cursed ? 1 : 0, !!otmp->blessed, !otmp->cursed);
		exercise(A_CON, TRUE);
		break;
	case POT_EXTRA_HEALING:
/*JP
		You_feel("much better.");
*/
		You("��ʬ���ȤƤ�褯�ʤä���");
		healup(d(6 + 2 * bcsign(otmp), 8),
		       otmp->blessed ? 5 : !otmp->cursed ? 2 : 0,
		       !otmp->cursed, TRUE);
		(void) make_hallucinated(0L,TRUE,0L);
		exercise(A_CON, TRUE);
		exercise(A_STR, TRUE);
		break;
	case POT_FULL_HEALING:
/*JP
		You_feel("completely healed.");
*/
		You("�����˲���������");
		healup(400, 4+4*bcsign(otmp), !otmp->cursed, TRUE);
		/* Restore one lost level if blessed */
		if (otmp->blessed && u.ulevel < u.ulevelmax) {
		    /* when multiple levels have been lost, drinking
		       multiple potions will only get half of them back */
		    u.ulevelmax -= 1;
		    pluslvl(FALSE);
		}
		(void) make_hallucinated(0L,TRUE,0L);
		exercise(A_STR, TRUE);
		exercise(A_CON, TRUE);
		break;
	case POT_LEVITATION:
	case SPE_LEVITATION:
		if (otmp->cursed) HLevitation &= ~I_SPECIAL;
		if(!Levitation) {
			/* kludge to ensure proper operation of float_up() */
			HLevitation = 1;
			float_up();
			/* reverse kludge */
			HLevitation = 0;
			if (otmp->cursed && !Is_waterlevel(&u.uz)) {
	if((u.ux != xupstair || u.uy != yupstair)
	   && (u.ux != sstairs.sx || u.uy != sstairs.sy || !sstairs.up)
	   && (!xupladder || u.ux != xupladder || u.uy != yupladder)
	) {
#if 0 /*JP*/
					You("hit your %s on the %s.",
						body_part(HEAD),
						ceiling(u.ux,u.uy));
#else
					You("%s��%s�ˤ֤Ĥ�����",
						body_part(HEAD),
						ceiling(u.ux,u.uy));
#endif
#if 0 /*JP*/
					losehp(uarmh ? 1 : rnd(10),
						"colliding with the ceiling",
						KILLED_BY);
#else
					losehp(uarmh ? 1 : rnd(10),
						"ŷ���Ƭ��֤Ĥ���",
						KILLED_BY);
#endif
				} else (void) doup();
			}
		} else
			nothing++;
		if (otmp->blessed) {
		    incr_itimeout(&HLevitation, rn1(50,250));
		    HLevitation |= I_SPECIAL;
		} else incr_itimeout(&HLevitation, rn1(140,10));
		spoteffects(FALSE);	/* for sinks */
		break;
	case POT_GAIN_ENERGY:			/* M. Stephenson */
		{	register int num;
			if(otmp->cursed)
/*JP
			    You_feel("lackluster.");
*/
			    You("�յ�����������");
			else
/*JP
			    pline("Magical energies course through your body.");
*/
			    pline("��ˡ�Υ��ͥ륮�������ʤ����Τ���������");
			num = rnd(5) + 5 * otmp->blessed + 1;
			u.uenmax += (otmp->cursed) ? -num : num;
			u.uen += (otmp->cursed) ? -num : num;
			if(u.uenmax <= 0) u.uenmax = 0;
			if(u.uen <= 0) u.uen = 0;
			flags.botl = 1;
			exercise(A_WIS, TRUE);
		}
		break;
	case POT_OIL:				/* P. Winner */
		{
			boolean good_for_you = FALSE;

			if (otmp->lamplit) {
			    if (likes_fire(youmonst.data)) {
/*JP
				pline("Ahh, a refreshing drink.");
*/
				pline("����������֤롣");
				good_for_you = TRUE;
			    } else {
/*JP
				You("burn your %s.", body_part(FACE));
*/
				Your("%s�Ϲ��Ǥ��ˤʤä���", body_part(FACE));
				losehp(d(Fire_resistance ? 1 : 3, 4),
/*JP
				       "burning potion of oil", KILLED_BY_AN);
*/
				       "ǳ���Ƥ�����������", KILLED_BY_AN);
			    }
			} else if(otmp->cursed)
/*JP
			    pline("This tastes like castor oil.");
*/
			    pline("�Ҥޤ����Τ褦��̣�����롣");
			else
/*JP
			    pline("That was smooth!");
*/
			    pline("�������꤬�褤��");
			exercise(A_WIS, good_for_you);
		}
		break;
	case POT_ACID:
		if (Acid_resistance)
			/* Not necessarily a creature who _likes_ acid */
/*JP
			pline("This tastes %s.", Hallucination ? "tangy" : "sour");
*/
			pline("%ṣ�����롣", Hallucination ? "�Ԥ�äȤ���" : "����");
		else {
#if 0 /*JP*/
			pline("This burns%s!", otmp->blessed ? " a little" :
					otmp->cursed ? " a lot" : " like acid");
			losehp(d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8),
					"potion of acid", KILLED_BY_AN);
#else
			pline("%s�Ҥ�Ҥꤹ�롪", otmp->blessed ? "����" :
					otmp->cursed ? "������" : "");
			losehp(d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8),
					"������������", KILLED_BY_AN);
#endif
			exercise(A_CON, FALSE);
		}
		if (Stoned) fix_petrification();
		unkn++; /* holy/unholy water can burn like acid too */
		break;
	case POT_POLYMORPH:
/*JP
		You_feel("a little %s.", Hallucination ? "normal" : "strange");
*/
		You("����%s�ʴ�����������", Hallucination ? "����" : "��");
		if (!Unchanging) polyself(FALSE);
		break;
	default:
		impossible("What a funny potion! (%u)", otmp->otyp);
		return(0);
	}
	return(-1);
}

#ifdef	JPEXTENSION
void
make_totter(xtime, talk)
long xtime;	/* nonzero if this is an attempt to turn on hallucination */
boolean talk;
{
	const char *message = 0;

	if (!xtime)
	    message = "�������Ф�����ˤʤä���";
	else
	    message = "�������Ф����㤷����";

	set_itimeout(&Totter, xtime);
	pline(message);
}
#endif

void
healup(nhp, nxtra, curesick, cureblind)
	int nhp, nxtra;
	register boolean curesick, cureblind;
{
	if (nhp) {
		if (Upolyd) {
			u.mh += nhp;
			if (u.mh > u.mhmax) u.mh = (u.mhmax += nxtra);
		} else {
			u.uhp += nhp;
			if(u.uhp > u.uhpmax) u.uhp = (u.uhpmax += nxtra);
		}
	}
	if(cureblind)	make_blinded(0L,TRUE);
	if(curesick)	make_sick(0L, (char *) 0, TRUE, SICK_ALL);
	flags.botl = 1;
	return;
}

void
strange_feeling(obj,txt)
register struct obj *obj;
register const char *txt;
{
	if (flags.beginner || !txt)
#if 0 /*JP*/
		You("have a %s feeling for a moment, then it passes.",
		Hallucination ? "normal" : "strange");
#else
		You("%s��ʬ�ˤ�����줿���������˾ä����ä���",
		Hallucination ? "���̤�" : "��̯��");
#endif
	else
		pline(txt);

	if(!obj)	/* e.g., crystal ball finds no traps */
		return;

	if(obj->dknown && !objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname)
		docall(obj);
	useup(obj);
}

const char *bottlenames[] = {
#if 0 /*JP*/
	"bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
#else
	"��","������","�쾣��","�庹��","�ե饹��","��","���饹��"
#endif
};


const char *
bottlename()
{
	return bottlenames[rn2(SIZE(bottlenames))];
}

void
potionhit(mon, obj, your_fault)
register struct monst *mon;
register struct obj *obj;
boolean your_fault;
{
	register const char *botlnam = bottlename();
	boolean isyou = (mon == &youmonst);
	int distance;

	if(isyou) {
		distance = 0;
/*JP
		pline_The("%s crashes on your %s and breaks into shards.",
*/
		pline("%s�����ʤ���%s�ξ�ǲ������ҤȤʤä���",
			botlnam, body_part(HEAD));
/*JP
		losehp(rnd(2), "thrown potion", KILLED_BY_AN);
*/
		losehp(rnd(2), "�ꤲ��줿����", KILLED_BY_AN);
	} else {
		distance = distu(mon->mx,mon->my);
/*JP
		if (!cansee(mon->mx,mon->my)) pline("Crash!");
*/
		if (!cansee(mon->mx,mon->my)) pline("�������");
		else {
		    char *mnam = mon_nam(mon);
		    char buf[BUFSZ];

		    if(has_head(mon->data)) {
/*JP
			Sprintf(buf, "%s %s",
*/
			Sprintf(buf, "%s��%s",
				s_suffix(mnam),
/*JP
				(notonhead ? "body" : "head"));
*/
				(notonhead ? "��" : "Ƭ"));
		    } else {
			Strcpy(buf, mnam);
		    }
/*JP
		    pline_The("%s crashes on %s and breaks into shards.",
*/
		    pline("%s��%s�ξ�ǲ������ҤȤʤä���",
			   botlnam, buf);
		}
		if(rn2(5) && mon->mhp > 1)
			mon->mhp--;
	}

	/* oil doesn't instantly evaporate */
	if (obj->otyp != POT_OIL && cansee(mon->mx,mon->my))
/*JP
		pline("%s.", Tobjnam(obj, "evaporate"));
*/
		pline("%s�Ͼ�ȯ������", xname(obj));

    if (isyou) {
	switch (obj->otyp) {
	case POT_OIL:
		if (obj->lamplit)
		    splatter_burning_oil(u.ux, u.uy);
		break;
	case POT_POLYMORPH:
/*JP
		You_feel("a little %s.", Hallucination ? "normal" : "strange");
*/
		You("%s�ʴ�����������", Hallucination ? "����" : "��");
		if (!Unchanging && !Antimagic) polyself(FALSE);
		break;
	case POT_ACID:
		if (!Acid_resistance) {
#if 0 /*JP*/
		    pline("This burns%s!", obj->blessed ? " a little" :
				    obj->cursed ? " a lot" : "");
#else
		    pline("%s�Ҥ�Ҥꤹ�롪", obj->blessed ? "����" :
				    obj->cursed ? "�Ϥ�����" : "");
#endif
#if 0 /*JP*/
		    losehp(d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8),
				    "potion of acid", KILLED_BY_AN);
#else
		    losehp(d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8),
				    "����������Ӥ�", KILLED_BY_AN);
#endif
		}
		break;
	}
    } else {
	boolean angermon = TRUE;

	/* GOTOs? We don't need no stinking GOTOs. */
	short tmp_otyp = obj->otyp;
	if (mon->data == &mons[PM_PESTILENCE]) {
		if (obj->otyp == POT_HEALING ||
		    obj->otyp == POT_EXTRA_HEALING ||
		    obj->otyp == POT_FULL_HEALING)
			tmp_otyp = POT_SICKNESS;
		else if (obj->otyp == POT_SICKNESS)
			tmp_otyp = POT_FULL_HEALING;
	}
	if (!your_fault) angermon = FALSE;
	switch (tmp_otyp) {
	case POT_HEALING:
	case POT_EXTRA_HEALING:
	case POT_FULL_HEALING:
	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
		angermon = FALSE;
		if(mon->mhp < mon->mhpmax) {
		    mon->mhp = mon->mhpmax;
		    if (canseemon(mon))
/*JP
			pline("%s looks sound and hale again.", Monnam(mon));
*/
			pline("%s�ϸ����ˤʤä��褦�˸����롣", Monnam(mon));
		}
	case POT_POISON:
		if (resists_poison(mon)) {
		    if (canseemon(mon))
			pline("%s�Ϥʤ�Ȥ�ʤ��褦����", Monnam(mon));
		    break;
		}
		/* Poisoned arrows cut the monster and introduce a small
		 * amount of poison into the body.  The broken flask is also
		 * likely to cut, and there's a lot more poison.  Thus we
		 * will keep the 10% instakill.  Damage is no longer based
		 * on mhpmax; poison should be potentially lethal.
		 */
		if(!rn2(10) && !resist(mon, POTION_CLASS, 0, NOTELL)) {
			if (canseemon(mon)) {
			    if (humanoid(mon->data) && !breathless(mon->data))
				pline("%s�Ϥ������ߡ����򤫤��ष�ä���",
				      Monnam(mon));
			    else pline("%s��´�ݤ�����", Monnam(mon));
			}
			killed(mon);
			break;
		}
		int pain = d((resist(mon, POTION_CLASS, 0, NOTELL) ? 2:4),10);
		mon->mhp -= pain;
		mon->mhpmax -= pain / 2;
		if (canseemon(mon))
			pline("%s���µ��äݤ������롣", Monnam(mon));
		if (mon->mhp < 1) killed(mon);
		break;
	case POT_SICKNESS:
		if ((dmgtype(mon->data, AD_DISE) ||
		    mon->data->mlet == S_FUNGUS ||
		    is_undead(mon->data)) &&
		    obj->otyp == POT_SICKNESS) { /* healing vs Pesty still hurts */
		    if (canseemon(mon))
#if 0 /*JP*/
			pline("%s looks unharmed.", Monnam(mon));
#else
			pline("%s�Ϥʤ�Ȥ�ʤ��褦����", Monnam(mon));
#endif
		    break;
		}
		pain = mon->mhpmax * ((obj->oeroded) ? 1 : 2)
		       / (resist(mon, POTION_CLASS, 0, NOTELL) ? 6 : 3);
		mon->mhp -= pain;
		mon->mhpmax -= pain / 2;
		if (canseemon(mon))
			pline("%s�ϤҤɤ��µ��˸����롪", Monnam(mon));
		if (mon->mhp < 1) killed(mon);
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!resist(mon, POTION_CLASS, 0, NOTELL))  mon->mconf = TRUE;
		break;
	case POT_INVISIBILITY:
		angermon = FALSE;
		mon_set_minvis(mon);
		break;
	case POT_SLEEPING:
		/* wakeup() doesn't rouse victims of temporary sleep */
		if (sleep_monst(mon, rnd(12), POTION_CLASS)) {
/*JP
		    pline("%s falls asleep.", Monnam(mon));
*/
		    pline("%s��̲�äƤ��ޤä���", Monnam(mon));
		    slept_monst(mon);
		}
		break;
	case POT_PARALYSIS:
		if (mon->mcanmove) {
			mon->mcanmove = 0;
			/* really should be rnd(5) for consistency with players
			 * breathing potions, but...
			 */
			mon->mfrozen = rnd(25);
		}
		break;
	case POT_SPEED:
		angermon = FALSE;
		mon_adjust_speed(mon, 1, obj);
		break;
	case POT_BLINDNESS:
		if(haseyes(mon->data)) {
		    register int btmp = 64 + rn2(32) +
			rn2(32) * !resist(mon, POTION_CLASS, 0, NOTELL);
		    btmp += mon->mblinded;
		    mon->mblinded = min(btmp,127);
		    mon->mcansee = 0;
		}
		break;
	case POT_WATER:
		if (is_undead(mon->data) || is_demon(mon->data) ||
			is_were(mon->data)) {
		    if (obj->blessed) {
#if 0 /*JP*/
			pline("%s %s in pain!", Monnam(mon),
			      is_silent(mon->data) ? "writhes" : "shrieks");
#else
			pline("%s�϶���%s��", Monnam(mon),
			      is_silent(mon->data) ? "�˿Ȥ��������" : "�ζ������򤢤���");
#endif
			mon->mhp -= d(2,6);
			/* should only be by you */
			if (mon->mhp < 1) killed(mon);
			else if (is_were(mon->data) && !is_human(mon->data))
			    new_were(mon);	/* revert to human */
		    } else if (obj->cursed) {
			angermon = FALSE;
			if (canseemon(mon))
/*JP
			    pline("%s looks healthier.", Monnam(mon));
*/
			    pline("%s�Ϥ�긵���ˤʤä��褦�˸����롣", Monnam(mon));
			mon->mhp += d(2,6);
			if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
			if (is_were(mon->data) && is_human(mon->data) &&
				!Protection_from_shape_changers)
			    new_were(mon);	/* transform into beast */
		    }
		} else if(mon->data == &mons[PM_GREMLIN]) {
		    angermon = FALSE;
		    (void)split_mon(mon, (struct monst *)0);
		} else if(mon->data == &mons[PM_IRON_GOLEM]) {
		    if (canseemon(mon))
#if 0 /*JP*/
			pline("%s rusts.", Monnam(mon));
#else
			pline("%s�ϻ��Ӥ���", Monnam(mon));
#endif
		    mon->mhp -= d(1,6);
		    /* should only be by you */
		    if (mon->mhp < 1) killed(mon);
		}
		break;
	case POT_OIL:
		if (obj->lamplit)
			splatter_burning_oil(mon->mx, mon->my);
		break;
	case POT_ACID:
		if (!resists_acid(mon) && !resist(mon, POTION_CLASS, 0, NOTELL)) {
#if 0 /*JP*/
		    pline("%s %s in pain!", Monnam(mon),
			  is_silent(mon->data) ? "writhes" : "shrieks");
#else
		    pline("%s�϶���%s��", Monnam(mon),
			  is_silent(mon->data) ? "�˿Ȥ��������" : "�ζ������򤢤���");
#endif
		    mon->mhp -= d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8);
		    if (mon->mhp < 1) {
			if (your_fault)
			    killed(mon);
			else
			    monkilled(mon, "", AD_ACID);
		    }
		}
		break;
	case POT_POLYMORPH:
		(void) bhitm(mon, obj);
		break;
/*
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
		break;
*/
	}
	if (angermon)
	    wakeup(mon);
	else
	    mon->msleeping = 0;
    }

	/* Note: potionbreathe() does its own docall() */
	if ((distance==0 || ((distance < 3) && rn2(5))) &&
	    (!breathless(youmonst.data) || haseyes(youmonst.data)))
		potionbreathe(obj);
	else if (obj->dknown && !objects[obj->otyp].oc_name_known &&
		   !objects[obj->otyp].oc_uname && cansee(mon->mx,mon->my))
		docall(obj);
	if(*u.ushops && obj->unpaid) {
	        register struct monst *shkp =
			shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

		if(!shkp)
		    obj->unpaid = 0;
		else {
		    (void)stolen_value(obj, u.ux, u.uy,
				 (boolean)shkp->mpeaceful, FALSE);
		    subfrombill(obj, shkp);
		}
	}
	obfree(obj, (struct obj *)0);
}

/* vapors are inhaled or get in your eyes */
void
potionbreathe(obj)
register struct obj *obj;
{
	register int i, ii, isdone, kn = 0;

	switch(obj->otyp) {
	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
		if(obj->cursed) {
		    if (!breathless(youmonst.data))
/*JP
			pline("Ulch!  That potion smells terrible!");
*/
			pline("�����������Ϥ�Τ��������������롪");
		    else if (haseyes(youmonst.data)) {
#if 0 /*JP*/
			int numeyes = eyecount(youmonst.data);
			Your("%s sting%s!",
			     (numeyes == 1) ? body_part(EYE) : makeplural(body_part(EYE)),
			     (numeyes == 1) ? "s" : "");
#else
			Your("%s�������������롪", body_part(EYE));
#endif
		    }
		    break;
		} else {
		    i = rn2(A_MAX);		/* start at a random point */
		    for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
			if(ABASE(i) < AMAX(i)) {
			    ABASE(i)++;
			    /* only first found if not blessed */
			    isdone = !(obj->blessed);
			    flags.botl = 1;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_FULL_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		/*FALL THROUGH*/
	case POT_EXTRA_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		/*FALL THROUGH*/
	case POT_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		exercise(A_CON, TRUE);
		break;
	case POT_POISON:
		if (!Role_if(PM_HEALER)) {
			if (Upolyd) {
			    if (u.mh <= 5) u.mh = 1; else u.mh -= 5;
			} else {
			    if (u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
			}
			flags.botl = 1;
			exercise(A_CON, FALSE);
		}
		break;
	case POT_SICKNESS:
		if (!Role_if(PM_HEALER)) exercise(A_CON, FALSE);
		break;
	case POT_HALLUCINATION:
/*JP
		You("have a momentary vision.");
*/
		You("��ָ��ƤˤĤĤޤ줿��");
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!Confusion)
/*JP
			You_feel("somewhat dizzy.");
*/
			You("��ޤ��򴶤�����");
		make_confused(itimeout_incr(HConfusion, rnd(5)), FALSE);
		break;
	case POT_INVISIBILITY:
		if (!Blind && !Invis) {
		    kn++;
#if 0 /*JP*/
		    pline("For an instant you %s!",
			See_invisible ? "could see right through yourself"
			: "couldn't see yourself");
#else
		    pline("��ּ�ʬ���Ȥ�%s�����ʤ��ʤä���",
			See_invisible ? "������"
			: "");
#endif
		}
		break;
	case POT_PARALYSIS:
		kn++;
		if (!Free_action) {
/*JP
		    pline("%s seems to be holding you.", Something);
*/
		    pline("%s�����ʤ���Ĥ��ޤ��Ƥ���褦�˴�������", Something);
		    nomul(-rnd(5));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
/*JP
		} else You("stiffen momentarily.");
*/
		} else You("��ֹ�ľ������");
		break;
	case POT_SLEEPING:
		kn++;
		if (!Free_action && !Sleep_resistance) {
/*JP
		    You_feel("rather tired.");
*/
		    You("��������줿��");
		    nomul(-rnd(5));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
/*JP
		} else You("yawn.");
*/
		} else You("�����Ӥ򤷤���");
		break;
	case POT_SPEED:
/*JP
		if (!Fast) Your("knees seem more flexible now.");
*/
		if (!Fast) Your("ɨ�Ϥ�ꤹ�Ф䤯ư���褦�ˤʤä���");
		incr_itimeout(&HFast, rnd(5));
		exercise(A_DEX, TRUE);
		break;
	case POT_BLINDNESS:
		if (!Blind && !u.usleep) {
		    kn++;
/*JP
		    pline("It suddenly gets dark.");
*/
		    pline("�����Ť��ʤä���");
		}
		make_blinded(itimeout_incr(Blinded, rnd(5)), FALSE);
		if (!Blind && !u.usleep) Your(vision_clears);
		break;
	case POT_WATER:
		if(u.umonnum == PM_GREMLIN) {
		    (void)split_mon(&youmonst, (struct monst *)0);
		} else if (u.ulycn >= LOW_PM) {
		    /* vapor from [un]holy water will trigger
		       transformation but won't cure lycanthropy */
		    if (obj->blessed && youmonst.data == &mons[u.ulycn])
			you_unwere(FALSE);
		    else if (obj->cursed && !Upolyd)
			you_were();
		}
		break;
	case POT_ACID:
	case POT_POLYMORPH:
		exercise(A_CON, FALSE);
		break;
/*
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
	case POT_OIL:
		break;
*/
	}
	/* note: no obfree() */
	if (obj->dknown) {
	    if (kn)
		makeknown(obj->otyp);
	    else if (!objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname)
		docall(obj);
	}
}

STATIC_OVL short
mixtype(o1, o2)
register struct obj *o1, *o2;
/* returns the potion type when o1 is dipped in o2 */
{
	/* cut down on the number of cases below */
	if (o1->oclass == POTION_CLASS &&
	    (o2->otyp == POT_GAIN_LEVEL ||
	     o2->otyp == POT_GAIN_ENERGY ||
	     o2->otyp == POT_HEALING ||
	     o2->otyp == POT_EXTRA_HEALING ||
	     o2->otyp == POT_FULL_HEALING ||
	     o2->otyp == POT_ENLIGHTENMENT ||
	     o2->otyp == POT_FRUIT_JUICE)) {
		struct obj *swp;

		swp = o1; o1 = o2; o2 = swp;
	}

	switch (o1->otyp) {
		case POT_HEALING:
			switch (o2->otyp) {
			    case POT_SPEED:
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_EXTRA_HEALING;
			}
		case POT_EXTRA_HEALING:
			switch (o2->otyp) {
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_FULL_HEALING;
			}
		case POT_FULL_HEALING:
			switch (o2->otyp) {
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_GAIN_ABILITY;
			}
		case UNICORN_HORN:
			switch (o2->otyp) {
			    case POT_POISON:
			    case POT_SICKNESS:
				return POT_FRUIT_JUICE;
			    case POT_HALLUCINATION:
			    case POT_BLINDNESS:
			    case POT_CONFUSION:
				return POT_WATER;
			}
			break;
		case AMETHYST:		/* "a-methyst" == "not intoxicated" */
			if (o2->otyp == POT_BOOZE)
			    return POT_FRUIT_JUICE;
			break;
		case POT_GAIN_LEVEL:
		case POT_GAIN_ENERGY:
			switch (o2->otyp) {
			    case POT_CONFUSION:
				return (rn2(3) ? POT_BOOZE : POT_ENLIGHTENMENT);
			    case POT_HEALING:
				return POT_EXTRA_HEALING;
			    case POT_EXTRA_HEALING:
				return POT_FULL_HEALING;
			    case POT_FULL_HEALING:
				return POT_GAIN_ABILITY;
			    case POT_FRUIT_JUICE:
				return POT_SEE_INVISIBLE;
			    case POT_BOOZE:
				return POT_HALLUCINATION;
			}
			break;
		case POT_FRUIT_JUICE:
			switch (o2->otyp) {
			    case POT_POISON:
				return POT_POISON;
			    case POT_SICKNESS:
				return POT_SICKNESS;
			    case POT_SPEED:
				return POT_BOOZE;
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_SEE_INVISIBLE;
			}
			break;
		case POT_ENLIGHTENMENT:
			switch (o2->otyp) {
			    case POT_LEVITATION:
				if (rn2(3)) return POT_GAIN_LEVEL;
				break;
			    case POT_FRUIT_JUICE:
				return POT_BOOZE;
			    case POT_BOOZE:
				return POT_CONFUSION;
			}
			break;
	}

	return 0;
}


boolean
get_wet(obj)
register struct obj *obj;
/* returns TRUE if something happened (potion should be used up) */
{
	char Your_buf[BUFSZ];

	if (snuff_lit(obj)) return(TRUE);

	if (obj->greased) {
		grease_protect(obj,(char *)0,&youmonst);
		return(FALSE);
	}
	(void) Shk_Your(Your_buf, obj);
	/* (Rusting shop goods ought to be charged for.) */
	switch (obj->oclass) {
	    case POTION_CLASS:
		if (obj->otyp == POT_WATER) return FALSE;
		/* KMH -- Water into acid causes an explosion */
		if (obj->otyp == POT_ACID) {
/*JP
			pline("It boils vigorously!");
*/
			pline("�㤷��ʨƭ������");
/*JP
			You("are caught in the explosion!");
*/
			You("��ȯ�˴������ޤ줿��");
/*JP
			losehp(rnd(10), "elementary chemistry", KILLED_BY);
*/
			losehp(rnd(10), "�������ؼ¸���", KILLED_BY);
			makeknown(obj->otyp);
			update_inventory();
			return (TRUE);
		}
#if 0 /*JP*/
		pline("%s %s%s.", Your_buf, aobjnam(obj,"dilute"),
		      obj->odiluted ? " further" : "");
#else
		pline("%s%s��%s���ޤä���", Your_buf, xname(obj),
		      obj->odiluted ? "�����" : "");
#endif
		if(obj->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
		    You("dilute it, you pay for it.");
*/
		    You("����Ƥ��ޤä���������ͤФʤ�ʤ���");
		    bill_dummy_object(obj);
		}
		if (obj->odiluted) {
			obj->odiluted = 0;
#ifdef UNIXPC
			obj->blessed = FALSE;
			obj->cursed = FALSE;
#else
			obj->blessed = obj->cursed = FALSE;
#endif
			obj->otyp = POT_WATER;
		} else obj->odiluted++;
		update_inventory();
		return TRUE;
	    case SCROLL_CLASS:
		if (obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL
		    && obj->otyp != SCR_MAIL
#endif
		    ) {
			if (!Blind) {
#if 0 /*JP*/
				boolean oq1 = obj->quan == 1L;
				pline_The("scroll%s %s.",
					  oq1 ? "" : "s", otense(obj, "fade"));
#else
				pline("��ʪ��ʸ�������줿��");
#endif
			}
			if(obj->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
			    You("erase it, you pay for it.");
*/
			    You("ʸ����ä��Ƥ��ޤä���������ͤФʤ�ʤ���");
			    bill_dummy_object(obj);
			}
			obj->otyp = SCR_BLANK_PAPER;
			obj->spe = 0;
			update_inventory();
			return TRUE;
		} else break;
	    case SPBOOK_CLASS:
		if (obj->otyp != SPE_BLANK_PAPER) {

			if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
/*JP
	pline("%s suddenly heats up; steam rises and it remains dry.",
*/
	pline("%s������Ǯ���ʤꡢ�������������ᡢ�����Ƥ��ޤä���",
				The(xname(obj)));
			} else {
			    if (!Blind) {
#if 0 /*JP*/
				    boolean oq1 = obj->quan == 1L;
				    pline_The("spellbook%s %s.",
					oq1 ? "" : "s", otense(obj, "fade"));
#else
				    pline("��ˡ���ʸ�������줿��");
#endif
			    }
			    if(obj->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
			        You("erase it, you pay for it.");
*/
			        You("ʸ����ä��Ƥ��ޤä���������ͤФʤ�ʤ���");
			        bill_dummy_object(obj);
			    }
			    obj->otyp = SPE_BLANK_PAPER;
			    update_inventory();
			}
			return TRUE;
		}
		break;
	    case WEAPON_CLASS:
	    /* Just "fall through" to generic rustprone check for now. */
	    /* fall through */
	    default:
		if (!obj->oerodeproof && is_rustprone(obj) &&
		    (obj->oeroded < MAX_ERODE) && !rn2(2)) {
#if 0 /*JP*/
			pline("%s %s some%s.",
			      Your_buf, aobjnam(obj, "rust"),
			      obj->oeroded ? " more" : "what");
#else
			pline("%s%s��%s���Ӥ���", Your_buf, xname(obj),
			      obj->oeroded ? "�����" : "");
#endif
			obj->oeroded++;
			update_inventory();
			return TRUE;
		} else break;
	}
/*JP
	pline("%s %s wet.", Your_buf, aobjnam(obj,"get"));
*/
	pline("%s%s��Ǩ�줿��", Your_buf, xname(obj));
	return FALSE;
}

int
dodip()
{
	register struct obj *potion, *obj;
	struct obj *singlepotion;
	const char *tmp;
	uchar here;
	char allowall[2];
	short mixture;
	char qbuf[QBUFSZ], Your_buf[BUFSZ];

	allowall[0] = ALL_CLASSES; allowall[1] = '\0';
	if(!(obj = getobj(allowall, "dip")))
		return(0);

	here = levl[u.ux][u.uy].typ;
	/* Is there a fountain to dip into here? */
	if (IS_FOUNTAIN(here)) {
/*JP
		if(yn("Dip it into the fountain?") == 'y') {
*/
		if(yn("���˿����ޤ�����") == 'y') {
			dipfountain(obj);
			return(1);
		}
	} else if (is_pool(u.ux,u.uy)) {
		tmp = waterbody_name(u.ux,u.uy);
/*JP
		Sprintf(qbuf, "Dip it into the %s?", tmp);
*/
		Sprintf(qbuf, "%s�˿����ޤ�����", tmp);
		if (yn(qbuf) == 'y') {
		    if (Levitation) {
			floating_above(tmp);
#ifdef STEED
		    } else if (u.usteed && !is_swimmer(u.usteed->data) &&
			    P_SKILL(P_RIDING) < P_BASIC) {
			rider_cant_reach(); /* not skilled enough to reach */
#endif
		    } else {
			(void) get_wet(obj);
			if (obj->otyp == POT_ACID) useup(obj);
		    }
		    return 1;
		}
	}

	if(!(potion = getobj(beverages, "dip into")))
		return(0);
	if (potion == obj && potion->quan == 1L) {
/*JP
		pline("That is a potion bottle, not a Klein bottle!");
*/
		pline("��������Ӥ������饤����ۤ���ʤ���");
		return 0;
	}
	potion->in_use = TRUE;		/* assume it will be used up */
	if(potion->otyp == POT_WATER) {
		boolean useeit = !Blind;
		if (useeit) (void) Shk_Your(Your_buf, obj);
		if (potion->blessed) {
			if (obj->cursed) {
				if (useeit)
#if 0 /*JP*/
				    pline("%s %s %s.",
					  Your_buf,
					  aobjnam(obj, "softly glow"),
					  hcolor(NH_AMBER));
#else
				    pline("%s%s�Ϥ��ä����%s��������",
					  Your_buf,
					  xname(obj),
					  jconj_adj(hcolor(NH_AMBER)));
#endif
				uncurse(obj);
				obj->bknown=1;
	poof:
				if(!(objects[potion->otyp].oc_name_known) &&
				   !(objects[potion->otyp].oc_uname))
					docall(potion);
				useup(potion);
				return(1);
			} else if(!obj->blessed) {
				if (useeit) {
				    tmp = hcolor(NH_LIGHT_BLUE);
#if 0 /*JP*/
				    pline("%s %s with a%s %s aura.",
					  Your_buf,
					  aobjnam(obj, "softly glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
#else
				    pline("%s%s�Ϥܤ���Ȥ���%s������ˤĤĤޤ줿��",
					  Your_buf, xname(obj), tmp);
#endif
				}
				bless(obj);
				obj->bknown=1;
				goto poof;
			}
		} else if (potion->cursed) {
			if (obj->blessed) {
				if (useeit)
#if 0 /*JP*/
				    pline("%s %s %s.",
					  Your_buf,
					  aobjnam(obj, "glow"),
					  hcolor((const char *)"brown"));
#else
				    pline("%s%s��%s��������",
					  Your_buf, xname(obj),
					  jconj_adj(hcolor((const char *)"�㿧��")));
#endif
				unbless(obj);
				obj->bknown=1;
				goto poof;
			} else if(!obj->cursed) {
				if (useeit) {
				    tmp = hcolor(NH_BLACK);
#if 0 /*JP*/
				    pline("%s %s with a%s %s aura.",
					  Your_buf,
					  aobjnam(obj, "glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
#else
				    pline("%s%s��%s������ˤĤĤޤ줿��",
					  Your_buf, xname(obj),tmp);
#endif
				}
				curse(obj);
				obj->bknown=1;
				goto poof;
			}
		} else
			if (get_wet(obj))
			    goto poof;
	} else if (obj->otyp == POT_POLYMORPH ||
		potion->otyp == POT_POLYMORPH) {
	    /* some objects can't be polymorphed */
	    if (obj->otyp == potion->otyp ||	/* both POT_POLY */
		    obj->otyp == WAN_POLYMORPH ||
		    obj->otyp == SPE_POLYMORPH ||
		    obj == uball || obj == uskin ||
		    obj_resists(obj->otyp == POT_POLYMORPH ?
				potion : obj, 5, 95)) {
		pline(nothing_happens);
	    } else {
	    	boolean was_wep = FALSE, was_swapwep = FALSE, was_quiver = FALSE;
		short save_otyp = obj->otyp;
		/* KMH, conduct */
		u.uconduct.polypiles++;

		if (obj == uwep) was_wep = TRUE;
		else if (obj == uswapwep) was_swapwep = TRUE;
		else if (obj == uquiver) was_quiver = TRUE;

		obj = poly_obj(obj, STRANGE_OBJECT);

		if (was_wep) setuwep(obj);
		else if (was_swapwep) setuswapwep(obj);
		else if (was_quiver) setuqwep(obj);

		if (obj->otyp != save_otyp) {
			makeknown(POT_POLYMORPH);
			useup(potion);
			prinv((char *)0, obj, 0L);
			return 1;
		} else {
/*JP
			pline("Nothing seems to happen.");
*/
			pline("���ⵯ����ʤ��ä��褦����");
			goto poof;
		}
	    }
	    potion->in_use = FALSE;	/* didn't go poof */
	    return(1);
	} else if(obj->oclass == POTION_CLASS && obj->otyp != potion->otyp) {
		/* Mixing potions is dangerous... */
/*JP
		pline_The("potions mix...");
*/
		pline("����Ĵ�礵�줿��");
		/* KMH, balance patch -- acid is particularly unstable */
		if (obj->cursed || obj->otyp == POT_ACID || !rn2(10)) {
/*JP
			pline("BOOM!  They explode!");
*/
			pline("�С�����ȯ������");
			exercise(A_STR, FALSE);
			if (!breathless(youmonst.data) || haseyes(youmonst.data))
				potionbreathe(obj);
			useup(obj);
			useup(potion);
/*JP
			losehp(rnd(10), "alchemic blast", KILLED_BY_AN);
*/
			losehp(rnd(10), "Ĵ��μ��Ԥ�", KILLED_BY_AN);
			return(1);
		}

		obj->blessed = obj->cursed = obj->bknown = 0;
		if (Blind || Hallucination) obj->dknown = 0;

		if ((mixture = mixtype(obj, potion)) != 0) {
			obj->otyp = mixture;
		} else {
		    switch (obj->odiluted ? 1 : rnd(8)) {
			case 1:
				obj->otyp = POT_WATER;
				break;
			case 2:
			case 3:
				obj->otyp = POT_POISON;
				break;
			case 4:
				{
				  struct obj *otmp;
				  otmp = mkobj(POTION_CLASS,FALSE);
				  obj->otyp = otmp->otyp;
				  obfree(otmp, (struct obj *)0);
				}
				break;
			default:
				if (!Blind)
/*JP
			  pline_The("mixture glows brightly and evaporates.");
*/
				  pline("��������������뤯��������ȯ������");
				useup(obj);
				useup(potion);
				return(1);
		    }
		}

		obj->odiluted = (obj->otyp != POT_WATER);

		if (obj->otyp == POT_WATER && !Hallucination) {
#if 0 /*JP*/
			pline_The("mixture bubbles%s.",
				Blind ? "" : ", then clears");
#else
			pline("���򺮤����%sˢ���ä���",
				Blind ? "" : "���Ф餯");
#endif
		} else if (!Blind) {
#if 0 /*JP*/
			pline_The("mixture looks %s.",
				hcolor(OBJ_DESCR(objects[obj->otyp])));
#else
			pline("����������%s���˸����롣",
				jtrns_obj('!',OBJ_DESCR(objects[obj->otyp])));
#endif
		}

		useup(potion);
		return(1);
	}

/*JP ����� NH3.4�Ǥ�#define����Ƥ��ʤ� */
#ifdef INVISIBLE_OBJECTS
	if (potion->otyp == POT_INVISIBILITY && !obj->oinvis) {
		obj->oinvis = TRUE;
		if (!Blind) {
		    if (!See_invisible) pline("Where did %s go?",
		    		the(xname(obj)));
		    else You("notice a little haziness around %s.",
		    		the(xname(obj)));
		}
		goto poof;
	} else if (potion->otyp == POT_SEE_INVISIBLE && obj->oinvis) {
		obj->oinvis = FALSE;
		if (!Blind) {
		    if (!See_invisible) pline("So that's where %s went!",
		    		the(xname(obj)));
		    else pline_The("haziness around %s disappears.",
		    		the(xname(obj)));
		}
		goto poof;
	}
#endif

	if(is_poisonable(obj)) {
	    if(potion->otyp == POT_POISON && !obj->opoisoned) {
		char buf[BUFSZ];
		if (potion->quan > 1L)
/*JP
		    Sprintf(buf, "One of %s", the(xname(potion)));
*/
		    Sprintf(buf, "%s�ΰ��", the(xname(potion)));
		else
		    Strcpy(buf, The(xname(potion)));
/*JP
		pline("%s forms a coating on %s.",
*/
		pline("%s��%s���ɤ�줿��",
		      buf, the(xname(obj)));
		obj->opoisoned = TRUE;
		goto poof;
	    } else if(obj->opoisoned &&
		      (potion->otyp == POT_HEALING ||
		       potion->otyp == POT_EXTRA_HEALING ||
		       potion->otyp == POT_FULL_HEALING)) {
/*JP
		pline("A coating wears off %s.", the(xname(obj)));
*/
		pline("�Ǥ�%s��������������", the(xname(obj)));
		obj->opoisoned = 0;
		goto poof;
	    }
	}

	if (potion->otyp == POT_OIL) {
	    boolean wisx = FALSE;
	    if (potion->lamplit) {	/* burning */
		int omat = objects[obj->otyp].oc_material;
		/* the code here should be merged with fire_damage */
		if (catch_lit(obj)) {
		    /* catch_lit does all the work if true */
		} else if (obj->oerodeproof || obj_resists(obj, 5, 95) ||
			   !is_flammable(obj) || obj->oclass == FOOD_CLASS) {
#if 0 /*JP*/
		    pline("%s %s to burn for a moment.",
			  Yname2(obj), otense(obj, "seem"));
#else
		    pline("%s�Ϥ��Ф餯�δ�ǳ������", Yname2(obj));
#endif
		} else {
		    if ((omat == PLASTIC || omat == PAPER) && !obj->oartifact)
			obj->oeroded = MAX_ERODE;
#if 0 /*JP*/
		    pline_The("burning oil %s %s.",
			    obj->oeroded == MAX_ERODE ? "destroys" : "damages",
			    yname(obj));
#else
		    pline("%s��ǳ���Ƥ������ˤ�ä�%s��",
			    yname(obj),
			    obj->oeroded == MAX_ERODE ? "�˲����줿" : "���Ĥ���줿");
#endif
		    if (obj->oeroded == MAX_ERODE) {
			obj_extract_self(obj);
			obfree(obj, (struct obj *)0);
			obj = (struct obj *) 0;
		    } else {
			/* we know it's carried */
			if (obj->unpaid) {
			    /* create a dummy duplicate to put on bill */
/*JP
			    verbalize("You burnt it, you bought it!");
*/
			    verbalize("ǳ�䤷���Τʤ���äƤ�餦�衪");
			    bill_dummy_object(obj);
			}
			obj->oeroded++;
		    }
		}
	    } else if (potion->cursed) {
/*JP
		pline_The("potion spills and covers your %s with oil.",
*/
		pline("�������ӻ��ꤢ�ʤ���%s�ˤ����ä���",
			  makeplural(body_part(FINGER)));
		incr_itimeout(&Glib, d(2,10));
	    } else if (obj->oclass != WEAPON_CLASS && !is_weptool(obj)) {
		/* the following cases apply only to weapons */
		goto more_dips;
	    /* Oil removes rust and corrosion, but doesn't unburn.
	     * Arrows, etc are classed as metallic due to arrowhead
	     * material, but dipping in oil shouldn't repair them.
	     */
	    } else if ((!is_rustprone(obj) && !is_corrodeable(obj)) ||
			is_ammo(obj) || (!obj->oeroded && !obj->oeroded2)) {
		/* uses up potion, doesn't set obj->greased */
#if 0 /*JP*/
		pline("%s %s with an oily sheen.",
		      Yname2(obj), otense(obj, "gleam"));
#else
		pline("%s�����θ����Ǥ����ȸ��ä���",
		      Yname2(obj));
#endif
	    } else {
#if 0 /*JP*/
		pline("%s %s less %s.",
		      Yname2(obj), otense(obj, "are"),
		      (obj->oeroded && obj->oeroded2) ? "corroded and rusty" :
			obj->oeroded ? "rusty" : "corroded");
#else
		pline("%s��%s����줿��", Yname2(obj),
		      (obj->oeroded && obj->oeroded2) ? "�忩�Ȼ�" :
		      obj->oeroded ? "��" : "�忩");
#endif
		if (obj->oeroded > 0) obj->oeroded--;
		if (obj->oeroded2 > 0) obj->oeroded2--;
		wisx = TRUE;
	    }
	    exercise(A_WIS, wisx);
	    makeknown(potion->otyp);
	    useup(potion);
	    return 1;
	}
    more_dips:

	/* Allow filling of MAGIC_LAMPs to prevent identification by player */
	if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP) &&
	   (potion->otyp == POT_OIL)) {
	    /* Turn off engine before fueling, turn off fuel too :-)  */
	    if (obj->lamplit || potion->lamplit) {
		useup(potion);
		explode(u.ux, u.uy, 11, d(6,6), 0, EXPL_FIERY);
		exercise(A_WIS, FALSE);
		return 1;
	    }
	    /* Adding oil to an empty magic lamp renders it into an oil lamp */
	    if ((obj->otyp == MAGIC_LAMP) && obj->spe == 0) {
		obj->otyp = OIL_LAMP;
		obj->age = 0;
	    }
	    if (obj->age > 1000L) {
/*JP
		pline("%s %s full.", Yname2(obj), otense(obj, "are"));
*/
		pline("%s�ˤϤ��äפ����äƤ��롣", Yname2(obj));
		potion->in_use = FALSE;	/* didn't go poof */
	    } else {
/*JP
		You("fill %s with oil.", yname(obj));
*/
		You("%s���������줿��", yname(obj));
		check_unpaid(potion);	/* Yendorian Fuel Tax */
		obj->age += 2*potion->age;	/* burns more efficiently */
		if (obj->age > 1500L) obj->age = 1500L;
		useup(potion);
		exercise(A_WIS, TRUE);
	    }
	    makeknown(POT_OIL);
	    obj->spe = 1;
	    update_inventory();
	    return 1;
	}

	potion->in_use = FALSE;		/* didn't go poof */
	if ((obj->otyp == UNICORN_HORN || obj->otyp == AMETHYST) &&
	    (mixture = mixtype(obj, potion)) != 0) {
#if 0 /*JP*/
		char oldbuf[BUFSZ], newbuf[BUFSZ];
#else
		char oldbuf[BUFSZ];
#endif
		short old_otyp = potion->otyp;
		boolean old_dknown = FALSE;
#if 0 /*JP*/
		boolean more_than_one = potion->quan > 1;
#endif

		oldbuf[0] = '\0';
		if (potion->dknown) {
		    old_dknown = TRUE;
#if 0 /*JP*/
		    Sprintf(oldbuf, "%s ",
			    hcolor(OBJ_DESCR(objects[potion->otyp])));
#else
		    Sprintf(oldbuf, "%s",
			    jtrns_obj('!', hcolor(OBJ_DESCR(objects[potion->otyp]))));
#endif
		}
		/* with multiple merged potions, split off one and
		   just clear it */
		if (potion->quan > 1L) {
		    singlepotion = splitobj(potion, 1L);
		} else singlepotion = potion;
		
		if(singlepotion->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
		    You("use it, you pay for it.");
*/
		    You("�ȤäƤ��ޤä���������ͤФʤ�ʤ���");
		    bill_dummy_object(singlepotion);
		}
		singlepotion->otyp = mixture;
		singlepotion->blessed = 0;
		if (mixture == POT_WATER)
		    singlepotion->cursed = singlepotion->odiluted = 0;
		else
		    singlepotion->cursed = obj->cursed;  /* odiluted left as-is */
		singlepotion->bknown = FALSE;
		if (Blind) {
		    singlepotion->dknown = FALSE;
		} else {
		    singlepotion->dknown = !Hallucination;
#if 0 /*JP*/
		    if (mixture == POT_WATER && singlepotion->dknown)
			Sprintf(newbuf, "clears");
		    else
			Sprintf(newbuf, "turns %s",
				hcolor(OBJ_DESCR(objects[mixture])));
		    pline_The("%spotion%s %s.", oldbuf,
			      more_than_one ? " that you dipped into" : "",
			      newbuf);
#else
		    if (mixture == POT_WATER && singlepotion->dknown)
			pline("%s����Ʃ���ˤʤä���", oldbuf);
		    else
			pline("%s����%s���ˤʤä���", oldbuf,
			      jtrns_obj('!',
				hcolor(OBJ_DESCR(objects[mixture]))));
#endif
		    if(!objects[old_otyp].oc_uname &&
			!objects[old_otyp].oc_name_known && old_dknown) {
			struct obj fakeobj;
			fakeobj = zeroobj;
			fakeobj.dknown = 1;
			fakeobj.otyp = old_otyp;
			fakeobj.oclass = POTION_CLASS;
			docall(&fakeobj);
		    }
		}
		obj_extract_self(singlepotion);
		singlepotion = hold_another_object(singlepotion,
/*JP
					"You juggle and drop %s!",
*/
					"����̤���%s����Ȥ��Ƥ��ޤä���",
					doname(singlepotion), (const char *)0);
		update_inventory();
		return(1);
	}

/*JP
	pline("Interesting...");
*/
	pline("���򤤡�");
	return(1);
}


void
djinni_from_bottle(obj)
register struct obj *obj;
{
	struct monst *mtmp;
	int chance;

	if(!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy, NO_MM_FLAGS))){
#if 0 /*JP:T*/
		pline("It turns out to be empty.");
#else
		if (obj->otyp == MAGIC_LAMP) {
			pline("���פ϶��äݤ��ä���");
		} else {
			pline("���϶��äݤ��ä���");
		}
#endif
		return;
	}

	if (!Blind) {
#if 0 /*JP*/
		pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
		pline("%s speaks.", Monnam(mtmp));
#else
		pline("����椫�顢%s������줿��", a_monnam(mtmp));
		pline("%s���ä���������", Monnam(mtmp));
#endif
	} else {
#if 0 /*JP*/
		You("smell acrid fumes.");
		pline("%s speaks.", Something);
#else
		You("�ĥ�Ȥ���������������");
		pline("%s���ä���������", Something);
#endif
	}

	chance = rn2(5);
	if (obj->blessed) chance = (chance == 4) ? rnd(4) : 0;
	else if (obj->cursed) chance = (chance == 0) ? rn2(4) : 4;
	/* 0,1,2,3,4:  b=80%,5,5,5,5; nc=20%,20,20,20,20; c=5%,5,5,5,80 */

	switch (chance) {
/*JP
	case 0 : verbalize("I am in your debt.  I will grant one wish!");
*/
	case 0 : verbalize("�����ˤϼڤ꤬�Ǥ��������Ĵꤤ�򤫤ʤ��Ƥ����");
		makewish();
		mongone(mtmp);
		break;
/*JP
	case 1 : verbalize("Thank you for freeing me!");
*/
	case 1 : verbalize("�������Ƥ��줿���Ȥ򴶼դ��롪");
		(void) tamedog(mtmp, (struct obj *)0);
		break;
/*JP
	case 2 : verbalize("You freed me!");
*/
	case 2 : verbalize("�������Ƥ��줿�ΤϤ�������");
		mtmp->mpeaceful = TRUE;
		set_malign(mtmp);
		break;
/*JP
	case 3 : verbalize("It is about time!");
*/
	case 3 : verbalize("����Ф���");
/*JP
		pline("%s vanishes.", Monnam(mtmp));
*/
		pline("%s�Ͼä�����", Monnam(mtmp));
		mongone(mtmp);
		break;
/*JP
	default: verbalize("You disturbed me, fool!");
*/
	default: verbalize("���ޤ��ϻ��̲���˸������������Τᡪ");
		break;
	}
}

/* clone a gremlin or mold (2nd arg non-null implies heat as the trigger);
   hit points are cut in half (odd HP stays with original) */
struct monst *
split_mon(mon, mtmp)
struct monst *mon,	/* monster being split */
	     *mtmp;	/* optional attacker whose heat triggered it */
{
	struct monst *mtmp2;
	char reason[BUFSZ];

	reason[0] = '\0';
#if 0 /*JP*/
	if (mtmp) Sprintf(reason, " from %s heat",
			  (mtmp == &youmonst) ? (const char *)"your" :
			      (const char *)s_suffix(mon_nam(mtmp)));
#endif

	if (mon == &youmonst) {
	    mtmp2 = cloneu();
	    if (mtmp2) {
		mtmp2->mhpmax = u.mhmax / 2;
		u.mhmax -= mtmp2->mhpmax;
		flags.botl = 1;
/*JP
		You("multiply%s!", reason);
*/
		You("%sʬ��������", reason);
	    }
	} else {
	    mtmp2 = clone_mon(mon, 0, 0);
	    if (mtmp2) {
		mtmp2->mhpmax = mon->mhpmax / 2;
		mon->mhpmax -= mtmp2->mhpmax;
		if (canspotmon(mon))
/*JP
		    pline("%s multiplies%s!", Monnam(mon), reason);
*/
		    pline("%s��%sʬ��������", Monnam(mon), reason);
	    }
	}
	return mtmp2;
}

#endif /* OVLB */

/*potion.c*/
