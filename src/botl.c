/*	SCCS Id: @(#)botl.c	3.4	1996/07/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

#ifdef OVL0
extern const char *hu_stat[];	/* defined in eat.c */

const char * const enc_stat[] = {
#if 0 /*JP*/
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
#else
	"",
	"�Ų�",
	"����",
	"��¿",
	"�³�",
	"Ķ��"
#endif
};

STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);
#endif /* OVL0 */

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *	Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *	T:123456 Satiated Conf FoodPois Ill Blind Stun Hallu Overloaded
 * -- or somewhat over 130 characters
 */
#if COLNO <= 140
#define MAXCO 160
#else
#define MAXCO (COLNO+20)
#endif

#ifndef OVLB
STATIC_DCL int mrank_sz;
#else /* OVLB */
STATIC_OVL NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
#endif /* OVLB */

STATIC_DCL const char *NDECL(rank);

#ifdef OVL1

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0	/* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(rank)
int rank;
{
	return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, monnum, female)
	int lev;
	short monnum;
	boolean female;
{
	register struct Role *role;
	register int i;


	/* Find the role */
	for (role = (struct Role *) roles; role->name.m; role++)
	    if (monnum == role->malenum || monnum == role->femalenum)
	    	break;
	if (!role->name.m)
	    role = &urole;

	/* Find the rank */
	for (i = xlev_to_rank((int)lev); i >= 0; i--) {
	    if (female && role->rank[i].f) return (role->rank[i].f);
	    if (role->rank[i].m) return (role->rank[i].m);
	}

	/* Try the role name, instead */
	if (female && role->name.f) return (role->name.f);
	else if (role->name.m) return (role->name.m);
	return ("Player");
}


STATIC_OVL const char *
rank()
{
	return(rank_of(u.ulevel, Role_switch, flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
	register int i, j;


	/* Loop through each of the roles */
	for (i = 0; roles[i].name.m; i++)
	    for (j = 0; j < 9; j++) {
	    	if (roles[i].rank[j].m && !strncmpi(str,
	    			roles[i].rank[j].m, strlen(roles[i].rank[j].m))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].m);
	    	    return roles[i].malenum;
	    	}
	    	if (roles[i].rank[j].f && !strncmpi(str,
	    			roles[i].rank[j].f, strlen(roles[i].rank[j].f))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].f);
	    	    return ((roles[i].femalenum != NON_PM) ?
	    	    		roles[i].femalenum : roles[i].malenum);
	    	}
	    }
	return NON_PM;
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	for (i = 0; i < 9; i++) {
	    if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) maxr = r;
	    if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) maxr = r;
	}
	mrank_sz = maxr;
	return;
}

#endif /* OVLB */
#ifdef OVL0

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    int deepest = deepest_lev_reached(FALSE);
#ifndef GOLDOBJ
    long ugold = u.ugold + hidden_gold();

    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
    return ugold + u.urexp + (long)(50 * (deepest - 1))
#else
    long umoney = money_cnt(invent) + hidden_gold();

    if ((umoney -= u.umoney0) < 0L) umoney = 0L;
    return umoney + u.urexp + (long)(50 * (deepest - 1))
#endif
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#ifdef HPPWBAR
static void
hppwbar(bot, barmax)
char *bot;
int barmax;
{
  int hp, hpmax;
  int hpbar, pwbar;
  char c;

  if (Upolyd)
    hp = u.mh, hpmax = u.mhmax;
  else
    hp = u.uhp, hpmax = u.uhpmax;
  if (hp <= 0)
    hpbar = 0;
  else if (hp >= hpmax)
    hpbar = barmax;
  else
    hpbar = hp * barmax / hpmax;
  if (u.uen <= 0)
    pwbar = 0;
  else if (u.uen >= u.uenmax)
    pwbar = barmax;
  else
    pwbar = u.uen * barmax / u.uenmax;
#ifdef JNETHACK
  if (is_kanji2(bot, hpbar))
    --hpbar;
  if (is_kanji2(bot, pwbar))
    --pwbar;
#endif
  curs(WIN_STATUS, 1, 0), putstr(WIN_STATUS, 0, "");
  if (pwbar < hpbar) {
    term_start_attr(ATR_INVERSE);
    if (pwbar > 0) {
      c = bot[pwbar], bot[pwbar] = 0;
      curs(WIN_STATUS, 1, 0), putstr(WIN_STATUS, ATR_INVERSE, bot);
      bot[pwbar] = c;
    }
    term_start_attr(ATR_ULINE);
    c = bot[hpbar], bot[hpbar] = 0;
    curs(WIN_STATUS, 1 + pwbar, 0), putstr(WIN_STATUS, ATR_INVERSE, bot + pwbar);
    bot[hpbar] = c;
    term_end_attr(ATR_ULINE);
    term_end_attr(ATR_INVERSE);
    curs(WIN_STATUS, 1 + hpbar, 0), putstr(WIN_STATUS, 0, bot + hpbar);
  }
  else {
    if (hpbar > 0) {
      term_start_attr(ATR_INVERSE);
      c = bot[hpbar], bot[hpbar] = 0;
      curs(WIN_STATUS, 1, 0), putstr(WIN_STATUS, ATR_INVERSE, bot);
      bot[hpbar] = c;
      term_end_attr(ATR_INVERSE);
    }
    if (hpbar < pwbar) {
      term_start_attr(ATR_ULINE);
      c = bot[pwbar], bot[pwbar] = 0;
      curs(WIN_STATUS, 1 + hpbar, 0), putstr(WIN_STATUS, 0, bot + hpbar);
      bot[pwbar] = c;
      term_end_attr(ATR_ULINE);
    }
    curs(WIN_STATUS, 1 + pwbar, 0), putstr(WIN_STATUS, 0, bot + pwbar);
  }
}
#endif

STATIC_OVL void
bot1()
{
	char newbot1[MAXCO];
	register char *nb;
	register int i,j;

	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
#if 1 /*JP*/
	if(is_kanji1(newbot1, 9))
		newbot1[9] = '_';
#endif
	newbot1[10] = 0;
/*JP
	Sprintf(nb = eos(newbot1)," the ");
*/
	Sprintf(nb = eos(newbot1)," ");

	if (Upolyd) {
		char mbot[BUFSZ];
		int k = 0;

/*JP
		Strcpy(mbot, mons[u.umonnum].mname);
*/
		Strcpy(mbot, jtrns_mon_gen(mons[u.umonnum].mname, flags.female));
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}
		Sprintf(nb = eos(nb), mbot);
	} else
		Sprintf(nb = eos(nb), rank());

	Sprintf(nb = eos(nb),"  ");
	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > STR18(100))
/*JP
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
*/
		    Sprintf(nb = eos(nb),"��:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < STR18(100))
/*JP
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
*/
		    Sprintf(nb = eos(nb), "��:18/%02d ",ACURR(A_STR)-18);
		else
#if 0 /*JP*/
		    Sprintf(nb = eos(nb),"St:18/** ");
#else
		    Sprintf(nb = eos(nb),"��:18/** ");
#endif
	} else
/*JP
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
*/
		Sprintf(nb = eos(nb), "��:%-1d ",ACURR(A_STR));
	Sprintf(nb = eos(nb),
/*JP
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
*/
		"��:%-1d ��:%-1d ��:%-1d ��:%-1d ̥:%-1d ",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
#if 0 /*JP*/
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#else
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "����" :
			(u.ualign.type == A_NEUTRAL) ? "��Ω" : "���");
#endif
#ifdef SCORE_ON_BOTL
	if (flags.showscore)
/*JP
	    Sprintf(nb = eos(nb), " S:%ld", botl_score());
*/
	    Sprintf(nb = eos(nb), "%ld��", botl_score());
#endif
#ifdef HPPWBAR
	hppwbar(newbot1, j - 2);
#else
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
#endif
}

/* provide the name of the current level for display by various ports */
int
describe_level(buf)
char *buf;
{
	int ret = 1;

	/* TODO:	Add in dungeon name */
	if (Is_knox(&u.uz))
/*JP
		Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
*/
		Sprintf(buf, "%s ", jtrns_obj('d', dungeons[u.uz.dnum].dname));
	else if (In_quest(&u.uz))
/*JP
		Sprintf(buf, "Home %d ", dunlev(&u.uz));
*/
		Sprintf(buf, "�ζ� %d ", dunlev(&u.uz));
	else if (In_endgame(&u.uz))
		Sprintf(buf,
/*JP
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
*/
			Is_astralevel(&u.uz) ? "ŷ�峦 " : "�ǽ����� ");
	else {
		/* ports with more room may expand this one */
/*JP
		Sprintf(buf, "Dlvl:%-2d ", depth(&u.uz));
*/
		Sprintf(buf, "�ϲ�:%-2d ", depth(&u.uz));
		ret = 0;
	}
	return ret;
}

STATIC_OVL void
bot2()
{
	char  newbot2[MAXCO];
	register char *nb;
	int hp, hpmax;
#ifdef HPMON
	int hpcolor, hpattr;
#endif
	int cap = near_capacity();

	hp = Upolyd ? u.mh : u.uhp;
	hpmax = Upolyd ? u.mhmax : u.uhpmax;

	if(hp < 0) hp = 0;
	(void) describe_level(newbot2);
	Sprintf(nb = eos(newbot2),
#ifdef HPMON
		"%c:%-2ld ��:", oc_syms[COIN_CLASS],
#ifndef GOLDOBJ
		u.ugold
#else
		money_cnt(invent)
#endif
		);
#else /* HPMON */
/*JP
		"%c:%-2ld HP:%d(%d) Pw:%d(%d) AC:%-2d", oc_syms[COIN_CLASS],
*/
		"%c:%-2ld ��:%d(%d) ��:%d(%d) ��:%-2d", oc_syms[COIN_CLASS],
#ifndef GOLDOBJ
		u.ugold,
#else
		money_cnt(invent),
#endif
		hp, hpmax, u.uen, u.uenmax, u.uac);
#endif /* HPMON */
#ifdef HPMON
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Sprintf(nb = eos(newbot2), "%d(%d)", hp, hpmax);
#ifdef TEXTCOLOR
	if (iflags.use_color) {
	  curs(WIN_STATUS, 1, 1);
	  hpattr = ATR_NONE;
	  if(hp == hpmax){
	    hpcolor = NO_COLOR;
	  } else if(hp > (hpmax*3/4)) {
	    hpcolor = CLR_BRIGHT_GREEN;
	  } else if(hp <= (hpmax/4)) {
	    hpcolor = CLR_ORANGE;
	  } else if(hp <= (hpmax/10)) {
	    hpcolor = CLR_RED;
	    //	    if(hp<=(hpmax/10)) 
	    //	      hpattr = ATR_BLINK;
	  } else {
	    hpcolor = CLR_YELLOW;
	  }
	  if (hpcolor != NO_COLOR)
	    term_start_color(hpcolor);
	  if(hpattr!=ATR_NONE)term_start_attr(hpattr);
	  putstr(WIN_STATUS, hpattr, newbot2);
	  if(hpattr!=ATR_NONE)term_end_attr(hpattr);
	  if (hpcolor != NO_COLOR)
	    term_end_color();
	}
#endif /* TEXTCOLOR */
	Sprintf(nb = eos(newbot2), " ��:%d(%d) ��:%-2d",
		u.uen, u.uenmax, u.uac);
#endif /* HPMON */

	if (Upolyd)
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
#ifdef EXP_ON_BOTL
	else if(flags.showexp)
/*JP
		Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
*/
		Sprintf(nb = eos(nb), " �и�:%u/%-1ld", u.ulevel,u.uexp);
#endif
	else
/*JP
		Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);
*/
		Sprintf(nb = eos(nb), " �и�:%u", u.ulevel);

	if(flags.time)
/*JP
	    Sprintf(nb = eos(nb), " T:%ld", moves);
*/
	    Sprintf(nb = eos(nb), " ��:%ld", moves);
	if(strcmp(hu_stat[u.uhs], "        ")) {
		Sprintf(nb = eos(nb), " ");
		Strcat(newbot2, hu_stat[u.uhs]);
	}
/*JP
	if(Confusion)	   Sprintf(nb = eos(nb), " Conf");
*/
	if(Confusion)      Sprintf(nb = eos(nb), " ����");
	if(Sick) {
		if (u.usick_type & SICK_VOMITABLE)
/*JP
			   Sprintf(nb = eos(nb), " FoodPois");
*/
			   Sprintf(nb = eos(nb), " ����");
		if (u.usick_type & SICK_NONVOMITABLE)
/*JP
			   Sprintf(nb = eos(nb), " Ill");
*/
			   Sprintf(nb = eos(nb), " �µ�");
	}
/*JP
	if(Blind)	   Sprintf(nb = eos(nb), " Blind");
*/
	if(Blind)          Sprintf(nb = eos(nb), " ����");
/*JP
	if(Stunned)	   Sprintf(nb = eos(nb), " Stun");
*/
	if(Stunned)        Sprintf(nb = eos(nb), " ����");
/*JP
	if(Hallucination)  Sprintf(nb = eos(nb), " Hallu");
*/
	if(Hallucination)  Sprintf(nb = eos(nb), " ����");
/*JP
	if(Slimed)         Sprintf(nb = eos(nb), " Slime");
*/
	if(Slimed)         Sprintf(nb = eos(nb), " ���饤��");
	if(cap > UNENCUMBERED)
		Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#endif /* OVL0 */

/*botl.c*/
