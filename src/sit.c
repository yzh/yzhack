/*	SCCS Id: @(#)sit.c	3.4	2002/09/21	*/
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

void
take_gold()
{
#ifndef GOLDOBJ
	if (u.ugold <= 0)  {
/*JP
		You_feel("a strange sensation.");
*/
		You("��̯�ʴ��Ф�Ф�����");
	} else {
/*JP
		You("notice you have no gold!");
*/
		You("�������äƤʤ����Ȥ˵����Ĥ�����");
		u.ugold = 0;
		flags.botl = 1;
	}
#else
        struct obj *otmp, *nobj;
	int lost_money = 0;
	for (otmp = invent; otmp; otmp = nobj) {
		nobj = otmp->nobj;
		if (otmp->oclass == COIN_CLASS) {
			lost_money = 1;
			delobj(otmp);
		}
	}
	if (!lost_money)  {
/*JP
		You_feel("a strange sensation.");
*/
		You("��̯�ʴ��Ф�Ф�����");
	} else {
/*JP
		You("notice you have no money!");
*/
		You("�������äƤʤ����Ȥ˵����Ĥ�����");
		flags.botl = 1;
	}
#endif
}

int
dosit()
{
/*JP
	static const char sit_message[] = "sit on the %s.";
*/
	static const char sit_message[] = "%s�˺¤ä���";
	register struct trap *trap;
	register int typ = levl[u.ux][u.uy].typ;


#ifdef STEED
	if (u.usteed) {
/*JP
	    You("are already sitting on %s.", mon_nam(u.usteed));
*/
	    You("�⤦%s�˺¤äƤ��롣", mon_nam(u.usteed));
	    return (0);
	}
#endif

	if(!can_reach_floor())	{
	    if (Levitation)
/*JP
		You("tumble in place.");
*/
		You("���ξ�����֤ꤷ����");
	    else
/*JP
		You("are sitting on air.");
*/
		You("����˺¤ä���");
	    return 0;
	} else if (is_pool(u.ux, u.uy) && !Underwater) {  /* water walking */
	    goto in_water;
	}

	if(OBJ_AT(u.ux, u.uy)) {
	    register struct obj *obj;

	    obj = level.objects[u.ux][u.uy];
/*JP
	    You("sit on %s.", the(xname(obj)));
*/
	    You("%s�˺¤ä���", the(xname(obj)));
	    if (!(Is_box(obj) || objects[obj->otyp].oc_material == CLOTH))
/*JP
		pline("It's not very comfortable...");
*/
		pline("���ޤ�¤ꤴ�������褯�ʤ���");

	} else if ((trap = t_at(u.ux, u.uy)) != 0 ||
		   (u.utrap && (u.utraptype >= TT_LAVA))) {

	    if (u.utrap) {
		exercise(A_WIS, FALSE);	/* you're getting stuck longer */
		if(u.utraptype == TT_BEARTRAP) {
/*JP
		    You_cant("sit down with your %s in the bear trap.", body_part(FOOT));
*/
		    pline("%s���ȥ�Х��ߤˤϤ��ޤäƤ���ΤǺ¤�ʤ���", body_part(FOOT));
		    u.utrap++;
	        } else if(u.utraptype == TT_PIT) {
		    if(trap->ttyp == SPIKED_PIT) {
/*JP
			You("sit down on a spike.  Ouch!");
*/
			You("�ȥ��ξ�˺¤ä������Ƥá�");
/*JP
			losehp(1, "sitting on an iron spike", KILLED_BY);
*/
			losehp(1, "Ŵ�Υȥ��ξ�˺¤ä�", KILLED_BY);
			exercise(A_STR, FALSE);
		    } else
/*JP
			You("sit down in the pit.");
*/
			You("������Ǻ¤ä���");
		    u.utrap += rn2(5);
		} else if(u.utraptype == TT_WEB) {
/*JP
		    You("sit in the spider web and get entangled further!");
*/
		    You("����������Ǻ¤ä��顢�����ޤä���");
		    u.utrap += rn1(10, 5);
		} else if(u.utraptype == TT_LAVA) {
		    /* Must have fire resistance or they'd be dead already */
/*JP
		    You("sit in the lava!");
*/
		    You("�ϴ����˺¤ä���");
		    u.utrap += rnd(4);
/*JP
		    losehp(d(2,10), "sitting in lava", KILLED_BY);
*/
		    losehp(d(2,10), "�ϴ����˺¤ä�", KILLED_BY);
		} else if(u.utraptype == TT_INFLOOR) {
/*JP
		    You_cant("maneuver to sit!");
*/
		    You("�¤�褦��ư��Ǥ��ʤ���");
		    u.utrap++;
		}
	    } else {
/*JP
	        You("sit down.");
*/
	        You("�¤ä���");
		dotrap(trap, 0);
	    }
	} else if(Underwater || Is_waterlevel(&u.uz)) {
	    if (Is_waterlevel(&u.uz))
/*JP
		There("are no cushions floating nearby.");
*/
		pline("�᤯���⤤�Ƥ��륯�å����Ϥʤ���");
	    else
/*JP
		You("sit down on the muddy bottom.");
*/
		You("�ɤ�ɤ�����˺¤ä���");
	} else if(is_pool(u.ux, u.uy)) {
 in_water:
/*JP
	    You("sit in the water.");
*/
	    You("�����Ǻ¤ä���");
	    if (!rn2(10) && uarm)
/*JP
		(void) rust_dmg(uarm, "armor", 1, TRUE, &youmonst);
*/
		(void) rust_dmg(uarm, "��", 1, TRUE, &youmonst);
	    if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
/*JP
		(void) rust_dmg(uarm, "armor", 1, TRUE, &youmonst);
*/
		(void) rust_dmg(uarm, "��", 1, TRUE, &youmonst);
#ifdef SINKS
	} else if(IS_SINK(typ)) {

/*JP
	    You(sit_message, defsyms[S_sink].explanation);
*/
	    You(sit_message, jtrns_obj('S', defsyms[S_sink].explanation));
/*JP
	    Your("%s gets wet.", humanoid(youmonst.data) ? "rump" : "underside");
*/
	    Your("%s��Ǩ�줿��", humanoid(youmonst.data) ? "��" : "����");
#endif
	} else if(IS_ALTAR(typ)) {

/*JP
	    You(sit_message, defsyms[S_altar].explanation);
*/
	    You(sit_message, jtrns_obj('S', defsyms[S_altar].explanation));
	    altar_wrath(u.ux, u.uy);

	} else if(IS_GRAVE(typ)) {

/*JP
	    You(sit_message, defsyms[S_grave].explanation);
*/
	    You(sit_message, jtrns_obj('S', defsyms[S_grave].explanation));

	} else if(typ == STAIRS) {

/*JP
	    You(sit_message, "stairs");
*/
	    You(sit_message, "����");

	} else if(typ == LADDER) {

/*JP
	    You(sit_message, "ladder");
*/
	    You(sit_message, "�Ϥ���");

	} else if (is_lava(u.ux, u.uy)) {

	    /* must be WWalking */
/*JP
	    You(sit_message, "lava");
*/
	    You(sit_message, "�ϴ�");
	    burn_away_slime();
	    if (likes_lava(youmonst.data)) {
/*JP
		pline_The("lava feels warm.");
*/
		pline("�ϴ���Ȥ�����");
		return 1;
	    }
#if 0 /*JP*/
	    pline_The("lava burns you!");
	    losehp(d((Fire_resistance ? 2 : 10), 10),
		   "sitting on lava", KILLED_BY);
#else
	    You("�ϴ��ǳ������");
	    losehp(d((Fire_resistance ? 2 : 10), 10),
		   "�ϴ�˺¤ä�", KILLED_BY);
#endif

	} else if (is_ice(u.ux, u.uy)) {

/*JP
	    You(sit_message, defsyms[S_ice].explanation);
*/
	    You(sit_message, jtrns_obj('S', defsyms[S_ice].explanation));
/*JP
	    if (!Cold_resistance) pline_The("ice feels cold.");
*/
	    if (!Cold_resistance) pline("ɹ���䤿����������");

	} else if (typ == DRAWBRIDGE_DOWN) {

/*JP
	    You(sit_message, "drawbridge");
*/
	    You(sit_message, "ķ�Ͷ�");

	} else if(IS_THRONE(typ)) {

/*JP
	    You(sit_message, defsyms[S_throne].explanation);
*/
	    You(sit_message, jtrns_obj('S', defsyms[S_throne].explanation));
	    if (rnd(6) > 4)  {
		switch (rnd(13))  {
		    case 1:
			(void) adjattrib(rn2(A_MAX), -rn1(4,3), FALSE);
/*JP
			losehp(rnd(10), "cursed throne", KILLED_BY_AN);
*/
			losehp(rnd(10), "����줿�̺¤�", KILLED_BY_AN);
			break;
		    case 2:
			(void) adjattrib(rn2(A_MAX), 1, FALSE);
			break;
		    case 3:
#if 0 /*JP*/
			pline("A%s electric shock shoots through your body!",
			      (Shock_resistance) ? "n" : " massive");
			losehp(Shock_resistance ? rnd(6) : rnd(30),
			       "electric chair", KILLED_BY_AN);
#else
			pline("%s�ŵ������ʤ����Τ�����ȴ������",
			      (Shock_resistance) ? "" : "�㤷��");
			losehp(Shock_resistance ? rnd(6) : rnd(30),
			       "�ŵ��ػҤ�", KILLED_BY_AN);
#endif
			exercise(A_CON, FALSE);
			break;
		    case 4:
/*JP
			You_feel("much, much better!");
*/
			You_feel("�ȤƤ⡢�ȤƤ⸵���ˤʤä��褦�˴�������");
			if (Upolyd) {
			    if (u.mh >= (u.mhmax - 5))  u.mhmax += 4;
			    u.mh = u.mhmax;
			}
			if(u.uhp >= (u.uhpmax - 5))  u.uhpmax += 4;
			u.uhp = u.uhpmax;
			make_blinded(0L,TRUE);
			make_sick(0L, (char *) 0, FALSE, SICK_ALL);
			heal_legs();
			flags.botl = 1;
			break;
		    case 5:
			take_gold();
			break;
		    case 6:
			if(u.uluck + rn2(5) < 0) {
/*JP
			    You_feel("your luck is changing.");
*/
			    pline("�����褯�ʤä��褦�ʵ������롣");
			    change_luck(1);
			} else	    makewish();
			break;
		    case 7:
			{
			register int cnt = rnd(10);

#if 0 /*JP*/
			pline("A voice echoes:");
			verbalize("Thy audience hath been summoned, %s!",
				  flags.female ? "Dame" : "Sire");
#else
			pline("����������:");
			verbalize("%s�衪���İ���������줷��",
				  flags.female ? "��" : "��");
#endif
			while(cnt--)
			    (void) makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
			break;
			}
		    case 8:
#if 0 /*JP*/
			pline("A voice echoes:");
			verbalize("By thy Imperious order, %s...",
				  flags.female ? "Dame" : "Sire");
#else
			pline("����������:");
			verbalize("%s�衪�������ʹ������褦����",
				  flags.female ? "��" : "��");
#endif
			do_genocide(5);	/* REALLY|ONTHRONE, see do_genocide() */
			break;
		    case 9:
#if 0 /*JP*/
			pline("A voice echoes:");
	verbalize("A curse upon thee for sitting upon this most holy throne!");
#else
			pline("����������:");
	verbalize("���ʤ�̺¤˺¤ꤷ��˼������졪");
#endif
			if (Luck > 0)  {
			    make_blinded(Blinded + rn1(100,250),TRUE);
			} else	    rndcurse();
			break;
		    case 10:
			if (Luck < 0 || (HSee_invisible & INTRINSIC))  {
				if (level.flags.nommap) {
					pline(
/*JP
					"A terrible drone fills your head!");
*/
					"�������֥�֥�Ȥ�������Ƭ�˶�������");
					make_confused(HConfusion + rnd(30),
									FALSE);
				} else {
/*JP
					pline("An image forms in your mind.");
*/
					pline("���륤�᡼����Ƭ��������");
					do_mapping();
				}
			} else  {
/*JP
				Your("vision becomes clear.");
*/
				Your("�볦�Ϻ㤨�Ϥä���");
				HSee_invisible |= FROMOUTSIDE;
				newsym(u.ux, u.uy);
			}
			break;
		    case 11:
			if (Luck < 0)  {
/*JP
			    You_feel("threatened.");
*/
			    You("��������Ƥ���褦�˴�������");
			    aggravate();
			} else  {

/*JP
			    You_feel("a wrenching sensation.");
*/
			    You("�ͤ���줿�褦�ʴ��Ф򴶤�����");
			    tele();		/* teleport him */
			}
			break;
		    case 12:
/*JP
			You("are granted an insight!");
*/
			You("ƶ���Ϥ�������");
			if (invent) {
			    /* rn2(5) agrees w/seffects() */
			    identify_pack(rn2(5));
			}
			break;
		    case 13:
/*JP
			Your("mind turns into a pretzel!");
*/
			Your("���ϥ��ͥ��ͤˤʤä���");
			make_confused(HConfusion + rn1(7,16),FALSE);
			break;
		    default:	impossible("throne effect");
				break;
		}
	    } else {
		if (is_prince(youmonst.data))
/*JP
		    You_feel("very comfortable here.");
*/
		    You("�����ϤȤƤ�����夯��");
		else
/*JP
		    You_feel("somehow out of place...");
*/
		    You("���Τ���㤤�ε���������");
	    }

	    if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
		/* may have teleported */
		levl[u.ux][u.uy].typ = ROOM;
/*JP
		pline_The("throne vanishes in a puff of logic.");
*/
		pline("�̺¤ϤդäȾä�����");
		newsym(u.ux,u.uy);
	    }

	} else if (lays_eggs(youmonst.data)) {
		struct obj *uegg;

		if (!flags.female) {
/*JP
			pline("Males can't lay eggs!");
*/
			pline("ͺ����򻺤�ʤ���");
			return 0;
		}

		if (u.uhunger < (int)objects[EGG].oc_nutrition) {
/*JP
			You("don't have enough energy to lay an egg.");
*/
			You("��򻺤�����Υ��ͥ륮�����ʤ���");
			return 0;
		}

		uegg = mksobj(EGG, FALSE, FALSE);
		uegg->spe = 1;
		uegg->quan = 1;
		uegg->owt = weight(uegg);
		uegg->corpsenm = egg_type_from_parent(u.umonnum, FALSE);
		uegg->known = uegg->dknown = 1;
		attach_egg_hatch_timeout(uegg);
/*JP
		You("lay an egg.");
*/
		You("��򻺤����");
		dropy(uegg);
		stackobj(uegg);
		morehungry((int)objects[EGG].oc_nutrition);
	} else if (u.uswallow)
/*JP
		There("are no seats in here!");
*/
		pline("�����ˤϰػҤϤʤ���");
	else
/*JP
		pline("Having fun sitting on the %s?", surface(u.ux,u.uy));
*/
		pline("%s�˺¤äƳڤ���������", surface(u.ux,u.uy));
	return(1);
}

void
rndcurse()			/* curse a few inventory items at random! */
{
	int	nobj = 0;
	int	cnt, onum;
	struct	obj	*otmp;
/*JP
	static const char mal_aura[] = "feel a malignant aura surround %s.";
*/
	static const char mal_aura[] = "�ٰ��ʥ������%s�β��˴�������";

	if (uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20)) {
/*JP
	    You(mal_aura, "the magic-absorbing blade");
*/
	    You(mal_aura, "���Ϥ�ۤ��Ȥ���");
	    return;
	}

	if(Antimagic) {
	    shieldeff(u.ux, u.uy);
/*JP
	    You(mal_aura, "you");
*/
	    You(mal_aura, "���ʤ�");
	}

	for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
	    /* gold isn't subject to being cursed or blessed */
	    if (otmp->oclass == COIN_CLASS) continue;
#endif
	    nobj++;
	}
	if (nobj) {
	    for (cnt = rnd(6/((!!Antimagic) + (!!Half_spell_damage) + 1));
		 cnt > 0; cnt--)  {
		onum = rnd(nobj);
		for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
		    /* as above */
		    if (otmp->oclass == COIN_CLASS) continue;
#endif
		    if (--onum == 0) break;	/* found the target */
		}
		/* the !otmp case should never happen; picking an already
		   cursed item happens--avoid "resists" message in that case */
		if (!otmp || otmp->cursed) continue;	/* next target */

		if(otmp->oartifact && spec_ability(otmp, SPFX_INTEL) &&
		   rn2(10) < 8) {
/*JP
		    pline("%s!", Tobjnam(otmp, "resist"));
*/
		    pline("%s�ϱƶ�������ʤ���", xname(otmp));
		    continue;
		}

		if(otmp->blessed)
			unbless(otmp);
		else
			curse(otmp);
	    }
	    update_inventory();
	}

#ifdef STEED
	/* treat steed's saddle as extended part of hero's inventory */
	if (u.usteed && !rn2(4) &&
		(otmp = which_armor(u.usteed, W_SADDLE)) != 0 &&
		!otmp->cursed) {	/* skip if already cursed */
	    if (otmp->blessed)
		unbless(otmp);
	    else
		curse(otmp);
	    if (!Blind) {
#if 0 /*JP*/
		pline("%s %s %s.",
		      s_suffix(upstart(y_monnam(u.usteed))),
		      aobjnam(otmp, "glow"),
		      hcolor(otmp->cursed ? NH_BLACK : (const char *)"brown"));
#else
		pline("%s��%s��������",
		      upstart(y_monnam(u.usteed)),
		      jconj_adj(hcolor(otmp->cursed ? NH_BLACK : (const char *)"�㿧��")));
#endif
		otmp->bknown = TRUE;
	    }
	}
#endif	/*STEED*/
}

void
attrcurse()			/* remove a random INTRINSIC ability */
{
	switch(rnd(11)) {
	case 1 : if (HFire_resistance & INTRINSIC) {
			HFire_resistance &= ~INTRINSIC;
/*JP
			You_feel("warmer.");
*/
			You("�Ȥ����򴶤�����");
			break;
		}
	case 2 : if (HTeleportation & INTRINSIC) {
			HTeleportation &= ~INTRINSIC;
/*JP
			You_feel("less jumpy.");
*/
			You("����ä�����Ĥ�����");
			break;
		}
	case 3 : if (HPoison_resistance & INTRINSIC) {
			HPoison_resistance &= ~INTRINSIC;
/*JP
			You_feel("a little sick!");
*/
			You("������ʬ�������ʤä���");
			break;
		}
	case 4 : if (HTelepat & INTRINSIC) {
			HTelepat &= ~INTRINSIC;
			if (Blind && !Blind_telepat)
			    see_monsters();	/* Can't sense mons anymore! */
/*JP
			Your("senses fail!");
*/
			Your("�޴������㤷����");
			break;
		}
	case 5 : if (HCold_resistance & INTRINSIC) {
			HCold_resistance &= ~INTRINSIC;
/*JP
			You_feel("cooler.");
*/
			You("�ä����򴶤�����");
			break;
		}
	case 6 : if (HInvis & INTRINSIC) {
			HInvis &= ~INTRINSIC;
/*JP
			You_feel("paranoid.");
*/
			You("���ۤ���������");
			break;
		}
	case 7 : if (HSee_invisible & INTRINSIC) {
			HSee_invisible &= ~INTRINSIC;
#if 0 /*JP*/
			You("%s!", Hallucination ? "tawt you taw a puttie tat"
						: "thought you saw something");
#else
			if(Hallucination)
			    You("���쳪�ߤ顢��Ƥ��롣");
			else
			    You("ï���˸����Ƥ���褦�˴�������");
#endif
			break;
		}
	case 8 : if (HFast & INTRINSIC) {
			HFast &= ~INTRINSIC;
/*JP
			You_feel("slower.");
*/
			You("�٤��ʤä��褦�˴�������");
			break;
		}
	case 9 : if (HStealth & INTRINSIC) {
			HStealth &= ~INTRINSIC;
/*JP
			You_feel("clumsy.");
*/
			You("�Դ��Ѥˤʤä��褦�˴�������");
			break;
		}
	case 10: if (HProtection & INTRINSIC) {
			HProtection &= ~INTRINSIC;
/*JP
			You_feel("vulnerable.");
*/
			You("̵�����ˤʤä��褦�˴�������");
			break;
		}
	case 11: if (HAggravate_monster & INTRINSIC) {
			HAggravate_monster &= ~INTRINSIC;
/*JP
			You_feel("less attractive.");
*/
			You("̥�Ϥ��������褦�˴�������");
			break;
		}
	default: break;
	}
}

/*sit.c*/
