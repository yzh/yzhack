/*	SCCS Id: @(#)fountain.c	3.4	2003/03/23	*/
/*	Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

STATIC_DCL void NDECL(dowatersnakes);
STATIC_DCL void NDECL(dowaterdemon);
STATIC_DCL void NDECL(dowaternymph);
STATIC_PTR void FDECL(gush, (int,int,genericptr_t));
STATIC_DCL void NDECL(dofindgem);

void
floating_above(what)
const char *what;
{
/*JP
    You("are floating high above the %s.", what);
*/
    You("%s���ڤ��������⤤�Ƥ��롣", what);
}

STATIC_OVL void
dowatersnakes() /* Fountain of snakes! */
{
    register int num = rn1(5,2);
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
	if (!Blind)
#if 0 /*JP*/
	    pline("An endless stream of %s pours forth!",
		  Hallucination ? makeplural(rndmonnam()) : "snakes");
#else
	    pline("%s���ɤɤä�ή�줿��",
		  Hallucination ? makeplural(rndmonnam()) : "��");
#endif
	else
/*JP
	    You_hear("%s hissing!", something);
*/
	    You_hear("�����äȤ�������ʹ������");
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_WATER_MOCCASIN],
			u.ux, u.uy, NO_MM_FLAGS)) && t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
    } else
/*JP
	pline_The("fountain bubbles furiously for a moment, then calms.");
*/
	pline("���������㤷��ˢ�������䤬���Ť��ˤʤä���");
}

STATIC_OVL
void
dowaterdemon() /* Water demon */
{
    register struct monst *mtmp;

    if(!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
	if((mtmp = makemon(&mons[PM_WATER_DEMON],u.ux,u.uy, NO_MM_FLAGS))) {
	    if (!Blind)
/*JP
		You("unleash %s!", a_monnam(mtmp));
*/
		You("%s�����������", a_monnam(mtmp));
	    else
/*JP
		You_feel("the presence of evil.");
*/
		You_feel("�ٰ���¸�ߤ򴶤�����");

	/* Give those on low levels a (slightly) better chance of survival */
	    if (rnd(100) > (80 + level_difficulty())) {
#if 0 /*JP*/
		pline("Grateful for %s release, %s grants you a wish!",
		      mhis(mtmp), mhe(mtmp));
#else
		pline("%s�ϲ�����ȤƤⴶ�դ����Τ��ߤ򤫤ʤ��Ƥ����褦����",
		      mhe(mtmp));
#endif
		makewish();
		mongone(mtmp);
	    } else if (t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
	}
    } else
/*JP
	pline_The("fountain bubbles furiously for a moment, then calms.");
*/
	pline("���������㤷��ˢ�������䤬���Ť��ˤʤä���");
}

STATIC_OVL void
dowaternymph() /* Water Nymph */
{
	register struct monst *mtmp;

	if(!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
	   (mtmp = makemon(&mons[PM_WATER_NYMPH],u.ux,u.uy, NO_MM_FLAGS))) {
		if (!Blind)
/*JP
		   You("attract %s!", a_monnam(mtmp));
*/
		   pline("%s������줿��", a_monnam(mtmp));
		else
/*JP
		   You_hear("a seductive voice.");
*/
		   You_hear("̥��Ū������ʹ������");
		mtmp->msleeping = 0;
		if (t_at(mtmp->mx, mtmp->my))
		    (void) mintrap(mtmp);
	} else
		if (!Blind)
/*JP
		   pline("A large bubble rises to the surface and pops.");
*/
		   pline("�礭��ˢ��ʨ���ФƤϤ�������");
		else
/*JP
		   You_hear("a loud pop.");
*/
		   You_hear("�礭�ʤ�Τ��Ϥ����벻��ʹ������");
}

void
dogushforth(drinking) /* Gushing forth along LOS from (u.ux, u.uy) */
int drinking;
{
	int madepool = 0;

	do_clear_area(u.ux, u.uy, 7, gush, (genericptr_t)&madepool);
	if (!madepool) {
	    if (drinking)
/*JP
		Your("thirst is quenched.");
*/
		Your("�餭�������줿��");
	    else
/*JP
		pline("Water sprays all over you.");
*/
		pline("�夷�֤������ʤ��ˤ����ä���");
	}
}

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
	register struct monst *mtmp;
	register struct trap *ttmp;

	if (((x+y)%2) || (x == u.ux && y == u.uy) ||
	    (rn2(1 + distmin(u.ux, u.uy, x, y)))  ||
	    (levl[x][y].typ != ROOM) ||
	    (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
		return;

	if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
		return;

	if (!((*(int *)poolcnt)++))
/*JP
	    pline("Water gushes forth from the overflowing fountain!");
*/
	    pline("������夬�ɤɤäȰ��Ф���");

	/* Put a pool at x, y */
	levl[x][y].typ = POOL;
	/* No kelp! */
	del_engr_at(x, y);
	water_damage(level.objects[x][y], FALSE, TRUE);

	if ((mtmp = m_at(x, y)) != 0)
		(void) minliquid(mtmp);
	else
		newsym(x,y);
}

STATIC_OVL void
dofindgem() /* Find a gem in the sparkling waters. */
{
/*JP
	if (!Blind) You("spot a gem in the sparkling waters!");
*/
	if (!Blind) pline("����᤯���������Ф򸫤Ĥ�����");
/*JP
	else You_feel("a gem here!");
*/
	else You_feel("���Ф�����褦����");
	(void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE-1),
			 u.ux, u.uy, FALSE, FALSE);
	SET_FOUNTAIN_LOOTED(u.ux,u.uy);
	newsym(u.ux, u.uy);
	exercise(A_WIS, TRUE);			/* a discovery! */
}

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
	if (IS_FOUNTAIN(levl[x][y].typ) &&
	    (!rn2(3) || FOUNTAIN_IS_WARNED(x,y))) {
		if(isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x,y)) {
			struct monst *mtmp;
			SET_FOUNTAIN_WARNED(x,y);
			/* Warn about future fountain use. */
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			    if (DEADMONSTER(mtmp)) continue;
			    if ((mtmp->data == &mons[PM_WATCHMAN] ||
				mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			       couldsee(mtmp->mx, mtmp->my) &&
			       mtmp->mpeaceful) {
/*JP
				pline("%s yells:", Amonnam(mtmp));
*/
				pline("%s�϶������", Amonnam(mtmp));
/*JP
				verbalize("Hey, stop using that fountain!");
*/
				verbalize("��������������ʡ�");
				break;
			    }
			}
			/* You can see or hear this effect */
/*JP
			if(!mtmp) pline_The("flow reduces to a trickle.");
*/
			if(!mtmp) pline("ή��Ϥ��������ˤʤä���");
			return;
		}
#ifdef WIZARD
		if (isyou && wizard) {
/*JP
			if (yn("Dry up fountain?") == 'n')
*/
			if (yn("������ߤۤ��ޤ�����") == 'n')
				return;
		}
#endif
		/* replace the fountain with ordinary floor */
		levl[x][y].typ = ROOM;
		levl[x][y].looted = 0;
		levl[x][y].blessedftn = 0;
/*JP
		if (cansee(x,y)) pline_The("fountain dries up!");
*/
		if (cansee(x,y)) pline("���ϴ��夬�ä���");
		/* The location is seen if the hero/monster is invisible */
		/* or felt if the hero is blind.			 */
		newsym(x, y);
		level.flags.nfountains--;
		if(isyou && in_town(x, y))
		    (void) angry_guards(FALSE);
	}
}

void
drinkfountain()
{
	/* What happens when you drink from a fountain? */
	register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
	register int fate = rnd(30);

	if (Levitation) {
/*JP
		floating_above("fountain");
*/
		floating_above("��");
		return;
	}

	if (mgkftn && u.uluck >= 0 && fate >= 10) {
		int i, ii, littleluck = (u.uluck < 4);

/*JP
		pline("Wow!  This makes you feel great!");
*/
		pline("�數���ȤƤⵤ�����褯�ʤä���");
		/* blessed restore ability */
		for (ii = 0; ii < A_MAX; ii++)
		    if (ABASE(ii) < AMAX(ii)) {
			ABASE(ii) = AMAX(ii);
			flags.botl = 1;
		    }
		/* gain ability, blessed if "natural" luck is high */
		i = rn2(A_MAX);		/* start at a random attribute */
		for (ii = 0; ii < A_MAX; ii++) {
		    if (adjattrib(i, 1, littleluck ? -1 : 0) && littleluck)
			break;
		    if (++i >= A_MAX) i = 0;
		}
		display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
		pline("A wisp of vapor escapes the fountain...");
*/
		pline("��Τ����ޤ꤬������ƨ������");
		exercise(A_WIS, TRUE);
		levl[u.ux][u.uy].blessedftn = 0;
		return;
	}

	if (fate < 10) {
/*JP
		pline_The("cool draught refreshes you.");
*/
		pline("�䤿�����դǤ��äѤꤷ����");
		u.uhunger += rnd(10); /* don't choke on water */
		newuhs(FALSE);
		if(mgkftn) return;
	} else {
	    switch (fate) {

		case 19: /* Self-knowledge */

/*JP
			You_feel("self-knowledgeable...");
*/
			You("��ʬ���Ȥ�Ƚ��褦�˴�������");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
			exercise(A_WIS, TRUE);
/*JP
			pline_The("feeling subsides.");
*/
			pline("���δ����Ϥʤ��ʤä���");
			break;

		case 20: /* Foul water */

/*JP
			pline_The("water is foul!  You gag and vomit.");
*/
			pline("��ϤҤɤ��Բ���̣�����������ʤ����Ǥ��ᤷ����");
			morehungry(rn1(20, 11));
			vomit();
			break;

		case 21: /* Poisonous */

/*JP
			pline_The("water is contaminated!");
*/
			pline("��ϱ�������Ƥ��롪");
			if (Poison_resistance) {
#if 0 /*JP*/
			   pline(
			      "Perhaps it is runoff from the nearby %s farm.",
				 fruitname(FALSE));
#else
			   pline(
			      "���֤󡢤���϶᤯��%s�����줫��ή��Ƥ��롣",
				 fruitname(FALSE));
#endif
/*JP
			   losehp(rnd(4),"unrefrigerated sip of juice",
*/
			   losehp(rnd(4),"��ä��̽��Τ��������",
				KILLED_BY_AN);
			   break;
			}
			losestr(rn1(4,3));
/*JP
			losehp(rnd(10),"contaminated water", KILLED_BY);
*/
			losehp(rnd(10),"�������줿���", KILLED_BY);
			exercise(A_CON, FALSE);
			break;

		case 22: /* Fountain of snakes! */

			dowatersnakes();
			break;

		case 23: /* Water demon */
			dowaterdemon();
			break;

		case 24: /* Curse an item */ {
			register struct obj *obj;

/*JP
			pline("This water's no good!");
*/
			pline("���ο�ϤȤƤ�ޤ�����");
			morehungry(rn1(20, 11));
			exercise(A_CON, FALSE);
			for(obj = invent; obj ; obj = obj->nobj)
				if (!rn2(5))	curse(obj);
			break;
			}

		case 25: /* See invisible */

			if (Blind) {
			    if (Invisible) {
/*JP
				You("feel transparent.");
*/
			    You("Ʃ���ˤʤä��������롣");
			    } else {
/*JP
			    	You("feel very self-conscious.");
*/
			    	You("���ռ����˴�������");
/*JP
			    	pline("Then it passes.");
*/
			    	pline("���δ����Ͼä�����");
			    }
			} else {
/*JP
			   You("see an image of someone stalking you.");
*/
			   You("��������ʬ�θ��Ĥ��Ƥ�������򸫤���");
/*JP
			   pline("But it disappears.");
*/
			   pline("������������Ͼä��Ƥ��ޤä���");
			}
			HSee_invisible |= FROMOUTSIDE;
			newsym(u.ux,u.uy);
			exercise(A_WIS, TRUE);
			break;

		case 26: /* See Monsters */

			(void) monster_detect((struct obj *)0, 0);
			exercise(A_WIS, TRUE);
			break;

		case 27: /* Find a gem in the sparkling waters. */

			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}

		case 28: /* Water Nymph */

			dowaternymph();
			break;

		case 29: /* Scare */ {
			register struct monst *mtmp;

/*JP
			pline("This water gives you bad breath!");
*/
			pline("���������©�������ʤä���");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    if(!DEADMONSTER(mtmp))
				monflee(mtmp, 0, FALSE, FALSE);
			}
			break;

		case 30: /* Gushing forth in this room */

			dogushforth(TRUE);
			break;

		default:

/*JP
			pline("This tepid water is tasteless.");
*/
			pline("���Τʤޤ̤뤤���̣���ʤ���");
			break;
	    }
	}
	dryup(u.ux, u.uy, TRUE);
}

void
dipfountain(obj)
register struct obj *obj;
{
	if (Levitation) {
/*JP
		floating_above("fountain");
*/
		floating_above("��");
		return;
	}

	/* Don't grant Excalibur when there's more than one object.  */
	/* (quantity could be > 1 if merged daggers got polymorphed) */
	if (obj->otyp == LONG_SWORD && obj->quan == 1L
	    && u.ulevel >= 7 && !rn2(6)
	    && !obj->oartifact
	    && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {

	  if (u.ualign.type != A_LAWFUL || !Role_if(PM_KNIGHT)) {
			/* Ha!  Trying to cheat her. */
/*JP
			pline("A freezing mist rises from the water and envelopes the sword.");
*/
			pline("ɹ��̸���夫��Ω�����ꡢ����ĤĤ����");
/*JP
			pline_The("fountain disappears!");
*/
			pline("���Ͼä��Ƥ��ޤä���");
			curse(obj);
			if (obj->spe > -6 && !rn2(3)) obj->spe--;
			obj->oerodeproof = FALSE;
			exercise(A_WIS, FALSE);
		} else {
			/* The lady of the lake acts! - Eric Backus */
			/* Be *REAL* nice */
/*JP
	  pline("From the murky depths, a hand reaches up to bless the sword.");
*/
	  pline("�ˤ��ä����ߤ��顢�����ʡ����ȼ꤬���ӤƤ�����");
/*JP
			pline("As the hand retreats, the fountain disappears!");
*/
			pline("�꤬�य�ȡ����Ͼä��Ƥ��ޤä���");
			obj = oname(obj, artiname(ART_EXCALIBUR));
			discover_artifact(ART_EXCALIBUR);
			bless(obj);
			obj->oeroded = obj->oeroded2 = 0;
			obj->oerodeproof = TRUE;
			exercise(A_WIS, TRUE);
		}
		update_inventory();
		levl[u.ux][u.uy].typ = ROOM;
		levl[u.ux][u.uy].looted = 0;
		newsym(u.ux, u.uy);
		level.flags.nfountains--;
		if(in_town(u.ux, u.uy))
		    (void) angry_guards(FALSE);
		return;
	} else if (get_wet(obj) && !rn2(2))
		return;

	/* Acid and water don't mix */
	if (obj->otyp == POT_ACID) {
	    useup(obj);
	    return;
	}

	switch (rnd(30)) {
		case 16: /* Curse the item */
			curse(obj);
			break;
		case 17:
		case 18:
		case 19:
		case 20: /* Uncurse the item */
			if(obj->cursed) {
			    if (!Blind)
/*JP
				pline_The("water glows for a moment.");
*/
				pline("��ϵ�����������");
			    uncurse(obj);
			} else {
/*JP
			    pline("A feeling of loss comes over you.");
*/
			    pline("��̯��æ�ϴ������ʤ��򤪤��ä���");
			}
			break;
		case 21: /* Water Demon */
			dowaterdemon();
			break;
		case 22: /* Water Nymph */
			dowaternymph();
			break;
		case 23: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 24: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
/*JP
			pline("A strange tingling runs up your %s.",
*/
			pline("��̯�ʤ��Ӥ줬���ʤ���%s�����ä���",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
/*JP
			You_feel("a sudden chill.");
*/
			You("���������򴶤�����");
			break;
		case 28: /* Strange feeling */
/*JP
			pline("An urge to take a bath overwhelms you.");
*/
			pline("��Ϥ�����ꤿ���Ȥ�����˾�ˤ���줿��");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
/*JP
			    You("lost some of your gold in the fountain!");
*/
			    You("��ߤ���硢������Ȥ��Ƥ��ޤä���");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
/*JP
			        You("lost some of your money in the fountain!");
*/
			        You("��ߤ���硢������Ȥ��Ƥ��ޤä���");
				CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (FOUNTAIN_IS_LOOTED(u.ux,u.uy)) break;
		    SET_FOUNTAIN_LOOTED(u.ux,u.uy);
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
/*JP
		pline("Far below you, you see coins glistening in the water.");
*/
		You("�ڤ����ο���˶�ߤε�����ߤĤ�����");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

#ifdef SINKS
void
breaksink(x,y)
int x, y;
{
    if(cansee(x,y) || (x == u.ux && y == u.uy))
/*JP
	pline_The("pipes break!  Water spurts out!");
*/
	pline("�۴ɤ�����夬ʮ�Ф�����");
    level.flags.nsinks--;
    levl[x][y].doormask = 0;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
}

void
drinksink()
{
	struct obj *otmp;
	struct monst *mtmp;

	if (Levitation) {
/*JP
		floating_above("sink");
*/
		floating_above("ή����");
		return;
	}
	switch(rn2(20)) {
/*JP
		case 0: You("take a sip of very cold water.");
*/
		case 0: You("�ȤƤ��䤿��������������");
			break;
/*JP
		case 1: You("take a sip of very warm water.");
*/
		case 1: You("�ȤƤ�̤뤤������������");
			break;
/*JP
		case 2: You("take a sip of scalding hot water.");
*/
		case 2: You("�ȤƤ�Ǯ��������������");
			if (Fire_resistance)
/*JP
				pline("It seems quite tasty.");
*/
				pline("�ȤƤ⤪�����������");
/*JP
			else losehp(rnd(6), "sipping boiling water", KILLED_BY);
*/
			else losehp(rnd(6), "ʨƭ�����������", KILLED_BY);
			break;
		case 3: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
/*JP
				pline_The("sink seems quite dirty.");
*/
				pline("ή����ϤȤƤ���ʤ餷����");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
#if 0 /*JP*/
				if (mtmp) pline("Eek!  There's %s in the sink!",
					(Blind || !canspotmon(mtmp)) ?
					"something squirmy" :
					a_monnam(mtmp));
#else
				if (mtmp) pline("����ή�����%s�����롪",
					(Blind || !canspotmon(mtmp)) ?
					"�Ȥ��������褦�ʤ��" :
					a_monnam(mtmp));
#endif
			}
			break;
		case 4: do {
				otmp = mkobj(POTION_CLASS,FALSE);
				if (otmp->otyp == POT_WATER) {
					obfree(otmp, (struct obj *)0);
					otmp = (struct obj *) 0;
				}
			} while(!otmp);
			otmp->cursed = otmp->blessed = 0;
#if 0 /*JP*/
			pline("Some %s liquid flows from the faucet.",
			      Blind ? "odd" :
			      hcolor(OBJ_DESCR(objects[otmp->otyp])));
#else
			pline("�ظ�����%s���Τ�ή�줿��",
			      Blind ? "��̯��" :
			      hcolor(jtrns_obj('!',OBJ_DESCR(objects[otmp->otyp]))));
#endif
			otmp->dknown = !(Blind || Hallucination);
			otmp->quan++; /* Avoid panic upon useup() */
			otmp->fromsink = 1; /* kludge for docall() */
			(void) dopotion(otmp);
			obfree(otmp, (struct obj *)0);
			break;
		case 5: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
/*JP
			    You("find a ring in the sink!");
*/
			    You("ή����˻��ؤ�ߤĤ�����");
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
/*JP
			} else pline("Some dirty water backs up in the drain.");
*/
			} else pline("���ʤ��夬�ӿ����ή�줿��");
			break;
		case 6: breaksink(u.ux,u.uy);
			break;
/*JP
		case 7: pline_The("water moves as though of its own will!");
*/
		case 7: pline("�夬�ջפ���äƤ��뤫�Τ褦��ư������");
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
/*JP
				pline("But it quiets down.");
*/
				pline("���������Ť��ˤʤä���");
			break;
/*JP
		case 8: pline("Yuk, this water tastes awful.");
*/
		case 8: pline("�������ȤƤ�Ҥɤ�̣�����롣");
			more_experienced(1,0);
			newexplevel();
			break;
/*JP
		case 9: pline("Gaggg... this tastes like sewage!  You vomit.");
*/
		case 9: pline("������������Τ褦��̣�����롪���ʤ����Ǥ��ᤷ����");
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
			break;
/*JP
		case 10: pline("This water contains toxic wastes!");
*/
		case 10: pline("���ο��ͭ�Ǥ��ӿ��ޤ�Ǥ��롪");
			if (!Unchanging) {
/*JP
				You("undergo a freakish metamorphosis!");
*/
				You("������Ѳ��򤷤Ϥ��᤿��");
				polyself(FALSE);
			}
			break;
		/* more odd messages --JJB */
/*JP
		case 11: You_hear("clanking from the pipes...");
*/
		case 11: You_hear("�۴ɤΥ�����Ȥ�������ʹ������");
			break;
/*JP
		case 12: You_hear("snatches of song from among the sewers...");
*/
		case 12: You_hear("������椫��Ȥ���Ȥ���βΤ�ʹ������");
			break;
		case 19: if (Hallucination) {
/*JP
		   pline("From the murky drain, a hand reaches up... --oops--");
*/
		   pline("�Ť��ӿ������꤬���ӤƤ�����--���ä�--");
				break;
			}
#if 0 /*JP*/
		default: You("take a sip of %s water.",
			rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot");
#else
		default: You("%s������������",
			rn2(3) ? (rn2(2) ? "�䤤" : "�̤뤤") : "Ǯ��");
#endif
	}
}
#endif /* SINKS */

/*fountain.c*/
