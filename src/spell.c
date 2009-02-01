/*	SCCS Id: @(#)spell.c	3.4	2003/01/17	*/
/*	Copyright (c) M. Stephenson 1988			  */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

static NEARDATA schar delay;		/* moves left for this spell */
static NEARDATA struct obj *book;	/* last/current book being xscribed */

/* spellmenu arguments; 0 thru n-1 used as spl_book[] index when swapping */
#define SPELLMENU_CAST (-2)
#define SPELLMENU_VIEW (-1)

#define KEEN 20000
#define MAX_SPELL_STUDY 3
#define incrnknow(spell)        spl_book[spell].sp_know = KEEN

#define spellev(spell)		spl_book[spell].sp_lev
#define spellname(spell)	OBJ_NAME(objects[spellid(spell)])
#define spellet(spell)	\
	((char)((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

STATIC_DCL int FDECL(spell_let_to_idx, (CHAR_P));
STATIC_DCL boolean FDECL(cursed_book, (struct obj *bp));
STATIC_DCL boolean FDECL(confused_book, (struct obj *));
STATIC_DCL void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
STATIC_DCL boolean FDECL(getspell, (int *));
STATIC_DCL boolean FDECL(dospellmenu, (const char *,int,int *));
STATIC_DCL int FDECL(percent_success, (int));
STATIC_DCL int NDECL(throwspell);
STATIC_DCL void NDECL(cast_protection);
STATIC_DCL void FDECL(spell_backfire, (int));
STATIC_DCL const char *FDECL(spelltypemnemonic, (int));
STATIC_DCL int FDECL(isqrt, (int));

/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   spelbase, spelheal:
 *	Arc are aware of magic through historical research
 *	Bar abhor magic (Conan finds it "interferes with his animal instincts")
 *	Cav are ignorant to magic
 *	Hea are very aware of healing magic through medical research
 *	Kni are moderately aware of healing from Paladin training
 *	Mon use magic to attack and defend in lieu of weapons and armor
 *	Pri are very aware of healing magic through theological research
 *	Ran avoid magic, preferring to fight unseen and unheard
 *	Rog are moderately aware of magic through trickery
 *	Sam have limited magical awareness, prefering meditation to conjuring
 *	Tou are aware of magic from all the great films they have seen
 *	Val have limited magical awareness, prefering fighting
 *	Wiz are trained mages
 *
 *	The arms penalty is lessened for trained fighters Bar, Kni, Ran,
 *	Sam, Val -
 *	the penalty is its metal interference, not encumbrance.
 *	The `spelspec' is a single spell which is fundamentally easier
 *	 for that role to cast.
 *
 *  spelspec, spelsbon:
 *	Arc map masters (SPE_MAGIC_MAPPING)
 *	Bar fugue/berserker (SPE_HASTE_SELF)
 *	Cav born to dig (SPE_DIG)
 *	Hea to heal (SPE_CURE_SICKNESS)
 *	Kni to turn back evil (SPE_TURN_UNDEAD)
 *	Mon to preserve their abilities (SPE_RESTORE_ABILITY)
 *	Pri to bless (SPE_REMOVE_CURSE)
 *	Ran to hide (SPE_INVISIBILITY)
 *	Rog to find loot (SPE_DETECT_TREASURE)
 *	Sam to be At One (SPE_CLAIRVOYANCE)
 *	Tou to smile (SPE_CHARM_MONSTER)
 *	Val control the cold (SPE_CONE_OF_COLD)
 *	Wiz all really, but SPE_MAGIC_MISSILE is their party trick
 *
 *	See percent_success() below for more comments.
 *
 *  uarmbon, uarmsbon, uarmhbon, uarmgbon, uarmfbon:
 *	Fighters find body armour & shield a little less limiting.
 *	Headgear, Gauntlets and Footwear are not role-specific (but
 *	still have an effect, except helm of brilliance, which is designed
 *	to permit magic-use).
 */

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
#if 0 /*JP*/
static const char explodes[] = "radiates explosive energy";
#endif

/* convert a letter into a number in the range 0..51, or -1 if not a letter */
STATIC_OVL int
spell_let_to_idx(ilet)
char ilet;
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26) return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26) return indx + 26;
    return -1;
}

/* TRUE: book should be destroyed by caller */
STATIC_OVL boolean
cursed_book(bp)
	struct obj *bp;
{
	int lev = objects[bp->otyp].oc_level;

	switch(rn2(lev)) {
	case 0:
/*JP
		You_feel("a wrenching sensation.");
*/
		You("ねじられたような感覚を感じた。");
		tele();		/* teleport him */
		break;
	case 1:
/*JP
		You_feel("threatened.");
*/
		You("おどされているように感じた。");
		aggravate();
		break;
	case 2:
		make_blinded(Blinded + rn1(100,250),TRUE);
		break;
	case 3:
		take_gold();
		break;
	case 4:
/*JP
		pline("These runes were just too much to comprehend.");
*/
		pline("このルーン文字を理解するのは困難だ。");
		make_confused(HConfusion + rn1(7,16),FALSE);
		break;
	case 5:
/*JP
		pline_The("book was coated with contact poison!");
*/
		pline("この本は接触型の毒で覆われている！");
		if (uarmg) {
		    if (uarmg->oerodeproof || !is_corrodeable(uarmg)) {
/*JP
			Your("gloves seem unaffected.");
*/
			pline("籠手は影響を受けなかったようだ。");
		    } else if (uarmg->oeroded2 < MAX_ERODE) {
			if (uarmg->greased) {
/*JP
			    grease_protect(uarmg, "gloves", &youmonst);
*/
			    grease_protect(uarmg, "籠手", &youmonst);
			} else {
#if 0 /*JP*/
			    Your("gloves corrode%s!",
				 uarmg->oeroded2+1 == MAX_ERODE ?
				 " completely" : uarmg->oeroded2 ?
				 " further" : "");
#else
			    pline("籠手は%s腐食した！",
				  uarmg->oeroded2+1 == MAX_ERODE ?
				  "完全に" : uarmg->oeroded2 ?
				  "さらに" : "");
#endif
			    uarmg->oeroded2++;
			}
		    } else
#if 0 /*JP*/
			Your("gloves %s completely corroded.",
			     Blind ? "feel" : "look");
#else
			pline("籠手は完全に腐食した%s。",
			     Blind ? "ようだ" : "ように見える");
#endif
		    break;
		}
		/* temp disable in_use; death should not destroy the book */
		bp->in_use = FALSE;
		losestr(Poison_resistance ? rn1(2,1) : rn1(4,3));
		losehp(rnd(Poison_resistance ? 6 : 10),
/*JP
		       "contact-poisoned spellbook", KILLED_BY_AN);
*/
		       "接触毒の魔法書で", KILLED_BY_AN);
		bp->in_use = TRUE;
		break;
	case 6:
		if(Antimagic) {
		    shieldeff(u.ux, u.uy);
/*JP
		    pline_The("book %s, but you are unharmed!", explodes);
*/
		    pline("本は強力なエネルギーを放出した、しかしあなたは傷つかない！");
		} else {
#if 0 /*JP*/
		    pline("As you read the book, it %s in your %s!",
			  explodes, body_part(FACE));
#else
		    pline("本は強力なエネルギーをあなたの%sに放出した！",
			  body_part(FACE));
#endif
/*JP
		    losehp(2*rnd(10)+5, "exploding rune", KILLED_BY_AN);
*/
		    losehp(2*rnd(10)+5, "強力なルーン文字のエネルギーで", KILLED_BY_AN);
		}
		return TRUE;
	default:
		rndcurse();
		break;
	}
	return FALSE;
}

/* study while confused: returns TRUE if the book is destroyed */
STATIC_OVL boolean
confused_book(spellbook)
struct obj *spellbook;
{
	boolean gone = FALSE;

	if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
	    spellbook->in_use = TRUE;	/* in case called from learn */
	    pline(
/*JP
	"Being confused you have difficulties in controlling your actions.");
*/
	  "混乱しているので、そういうことをするのは難しい。");
	    display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
	    You("accidentally tear the spellbook to pieces.");
*/
			    You("うっかり、魔法書を引きさいてしまった。");
	    if (!objects[spellbook->otyp].oc_name_known &&
		!objects[spellbook->otyp].oc_uname)
		docall(spellbook);
	    useup(spellbook);
	    gone = TRUE;
	} else {
#if 0 /*JP*/
	    You("find yourself reading the %s line over and over again.",
		spellbook == book ? "next" : "first");
#else
	    You("%sの行を何度も繰り返して読んでいたことに気付いた。",
		spellbook == book ? "次" : "最初");
#endif
	}
	return gone;
}

/* special effects for The Book of the Dead */
STATIC_OVL void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

/*JP
    You("turn the pages of the Book of the Dead...");
*/
    You("死者の書のページをめくった…");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if(invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
	register struct obj *otmp;
	register boolean arti1_primed = FALSE, arti2_primed = FALSE,
			 arti_cursed = FALSE;

	if(book2->cursed) {
/*JP
	    pline_The("runes appear scrambled.  You can't read them!");
*/
	    pline("ルーン文字はごちゃまぜになっており、読むことができなかった！");
	    return;
	}

	if(!u.uhave.bell || !u.uhave.menorah) {
/*JP
	    pline("A chill runs down your %s.", body_part(SPINE));
*/
	    Your("%sに寒けが走った。", body_part(SPINE));
/*JP
	    if(!u.uhave.bell) You_hear("a faint chime...");
*/
	    if(!u.uhave.bell) You_hear("かすかなベルの音を聞いた…");
/*JP
	    if(!u.uhave.menorah) pline("Vlad's doppelganger is amused.");
*/
	    if(!u.uhave.menorah) pline("ヴラドの生霊は笑った。");
	    return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION &&
	       otmp->spe == 7 && otmp->lamplit) {
		if(!otmp->cursed) arti1_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	    if(otmp->otyp == BELL_OF_OPENING &&
	       (moves - otmp->age) < 5L) { /* you rang it recently */
		if(!otmp->cursed) arti2_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	}

	if(arti_cursed) {
/*JP
	    pline_The("invocation fails!");
*/
	    pline("特殊能力は発揮されなかった！");
/*JP
	    pline("At least one of your artifacts is cursed...");
*/
	    pline("少なくともあなたのアーティファクトのひとつが呪われている…");
	} else if(arti1_primed && arti2_primed) {
	    unsigned soon = (unsigned) d(2,6);	/* time til next intervene() */

	    /* successful invocation */
	    mkinvokearea();
	    u.uevent.invoked = 1;
	    /* in case you haven't killed the Wizard yet, behave as if
	       you just did */
	    u.uevent.udemigod = 1;	/* wizdead() */
	    if (!u.udg_cnt || u.udg_cnt > soon) u.udg_cnt = soon;
	} else {	/* at least one artifact not prepared properly */
/*JP
	    You("have a feeling that %s is amiss...", something);
*/
	    You("何かが間違っているように感じた…");
	    goto raise_dead;
	}
	return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
raise_dead:

/*JP
	You("raised the dead!");
*/
	You("死者を蘇らせた！");
	/* first maybe place a dangerous adversary */
	if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH],
					u.ux, u.uy, NO_MINVENT)) != 0 ||
			(mtmp = makemon(&mons[PM_NALFESHNEE],
					u.ux, u.uy, NO_MINVENT)) != 0)) {
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp);
	}
	/* next handle the affect on things you're carrying */
	(void) unturn_dead(&youmonst);
	/* last place some monsters around you */
	mm.x = u.ux;
	mm.y = u.uy;
	mkundead(&mm, TRUE, NO_MINVENT);
    } else if(book2->blessed) {
	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;		/* tamedog() changes chain */
	    if (DEADMONSTER(mtmp)) continue;

	    if (is_undead(mtmp->data) && cansee(mtmp->mx, mtmp->my)) {
		mtmp->mpeaceful = TRUE;
		if(sgn(mtmp->data->maligntyp) == sgn(u.ualign.type)
		   && distu(mtmp->mx, mtmp->my) < 4)
		    if (mtmp->mtame) {
			if (mtmp->mtame < 20)
			    mtmp->mtame++;
		    } else
			(void) tamedog(mtmp, (struct obj *)0);
		else monflee(mtmp, 0, FALSE, TRUE);
	    }
	}
    } else {
	switch(rn2(3)) {
	case 0:
/*JP
	    Your("ancestors are annoyed with you!");
*/
	    Your("先祖たちはあなたにいらついている！");
	    break;
	case 1:
/*JP
	    pline_The("headstones in the cemetery begin to move!");
*/
	    pline("墓地の墓石が動きはじめた！");
	    break;
	default:
/*JP
	    pline("Oh my!  Your name appears in the book!");
*/
	    pline("なんてこった！あなたの名前が本に書いてある！");
	}
    }
    return;
}

STATIC_PTR int
learn()
{
	int i;
	short booktype;
	char splname[BUFSZ];
	boolean costly = TRUE;

	/* JDS: lenses give 50% faster reading; 33% smaller read time */
	if (delay && ublindf && ublindf->otyp == LENSES && rn2(2)) delay++;
	if (Confusion) {		/* became confused while learning */
	    (void) confused_book(book);
	    book = 0;			/* no longer studying */
	    nomul(delay);		/* remaining delay is uninterrupted */
	    delay = 0;
	    return(0);
	}
	if (delay) {	/* not if (delay++), so at end delay == 0 */
	    delay++;
	    return(1); /* still busy */
	}
	exercise(A_WIS, TRUE);		/* you're studying. */
	booktype = book->otyp;
	if(booktype == SPE_BOOK_OF_THE_DEAD) {
	    deadbook(book);
	    return(0);
	}

#if 0 /*JP*/
	Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "the \"%s\" spell",
		OBJ_NAME(objects[booktype]));
#else
	Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "\"%s\"",
		jtrns_obj('+', OBJ_NAME(objects[booktype])));
#endif
	for (i = 0; i < MAXSPELL; i++)  {
		if (spellid(i) == booktype)  {
			if (book->spestudied > MAX_SPELL_STUDY) {
/*JP
			    pline("This spellbook is too faint to be read any more.");
*/
			    pline("この魔法書の文字はかすれすぎてこれ以上読めない。");
			    book->otyp = booktype = SPE_BLANK_PAPER;
			} else if (spellknow(i) <= 1000) {
/*JP
			    Your("knowledge of %s is keener.", splname);
*/
			    Your("%sに対する知識はさらに研ぎすまされた。", splname);
			    incrnknow(i);
			    book->spestudied++;
			    exercise(A_WIS,TRUE);       /* extra study */
			} else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
/*JP
			    You("know %s quite well already.", splname);
*/
			    You("すでに%sを熟知している。", splname);
			    costly = FALSE;
			}
			/* make book become known even when spell is already
			   known, in case amnesia made you forget the book */
			makeknown((int)booktype);
			break;
		} else if (spellid(i) == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].oc_level;
			incrnknow(i);
			book->spestudied++;
#if 0 /*JP*/
			You(i > 0 ? "add %s to your repertoire." : "learn %s.",
			    splname);
#else
			You(i > 0 ? "%sをレパートリーに加えた。" : "%sを習得した。", splname);
#endif
			makeknown((int)booktype);
			break;
		}
	}
	if (i == MAXSPELL) impossible("Too many spells memorized!");

	if (book->cursed) {	/* maybe a demon cursed it */
	    if (cursed_book(book)) {
		useup(book);
		book = 0;
		return 0;
	    }
	}
	if (costly) check_unpaid(book);
	book = 0;
	return(0);
}

int
study_book(spellbook)
register struct obj *spellbook;
{
	register int	 booktype = spellbook->otyp;
	register boolean confused = (Confusion != 0);
	boolean too_hard = FALSE;

	if (delay && !confused && spellbook == book &&
		    /* handle the sequence: start reading, get interrupted,
		       have book become erased somehow, resume reading it */
		    booktype != SPE_BLANK_PAPER) {
/*JP
		You("continue your efforts to memorize the spell.");
*/
		You("魔法の学習を再開した。");
	} else {
		/* KMH -- Simplified this code */
		if (booktype == SPE_BLANK_PAPER) {
/*JP
			pline("This spellbook is all blank.");
*/
			pline("この魔法書はどのページも白紙だ。");
			makeknown(booktype);
			return(1);
		}
		switch (objects[booktype].oc_level) {
		 case 1:
		 case 2:
			delay = -objects[booktype].oc_delay;
			break;
		 case 3:
		 case 4:
			delay = -(objects[booktype].oc_level - 1) *
				objects[booktype].oc_delay;
			break;
		 case 5:
		 case 6:
			delay = -objects[booktype].oc_level *
				objects[booktype].oc_delay;
			break;
		 case 7:
			delay = -8 * objects[booktype].oc_delay;
			break;
		 default:
			impossible("Unknown spellbook level %d, book %d;",
				objects[booktype].oc_level, booktype);
			return 0;
		}

		/* Books are often wiser than their readers (Rus.) */
		spellbook->in_use = TRUE;
		if (!spellbook->blessed &&
		    spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
		    if (spellbook->cursed) {
			too_hard = TRUE;
		    } else {
			/* uncursed - chance to fail */
			int read_ability = ACURR(A_INT) + 4 + u.ulevel/2
			    - 2*objects[booktype].oc_level
			    + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);
			/* only wizards know if a spell is too difficult */
			if (Role_if(PM_WIZARD) && read_ability < 20 &&
			    !confused) {
			    char qbuf[QBUFSZ];
			    Sprintf(qbuf,
#if 0 /*JP*/
		      "This spellbook is %sdifficult to comprehend. Continue?",
				    (read_ability < 12 ? "very " : ""));
#else
		      "この魔法書を理解するのは%s困難だ。続けますか？",
					(read_ability < 12 ? "とても" : ""));
#endif
			    if (yn(qbuf) != 'y') {
				spellbook->in_use = FALSE;
				return(1);
			    }
			}
			/* its up to random luck now */
			if (rnd(20) > read_ability) {
			    too_hard = TRUE;
			}
		    }
		}

		if (too_hard) {
		    boolean gone = cursed_book(spellbook);

		    nomul(delay);			/* study time */
		    delay = 0;
		    if(gone || !rn2(3)) {
/*JP
			if (!gone) pline_The("spellbook crumbles to dust!");
*/
			if (!gone) pline("魔法書は粉々に崩れさった！");
			if (!objects[spellbook->otyp].oc_name_known &&
				!objects[spellbook->otyp].oc_uname)
			    docall(spellbook);
			useup(spellbook);
		    } else
			spellbook->in_use = FALSE;
		    return(1);
		} else if (confused) {
		    if (!confused_book(spellbook)) {
			spellbook->in_use = FALSE;
		    }
		    nomul(delay);
		    delay = 0;
		    return(1);
		}
		spellbook->in_use = FALSE;

#if 0 /*JP*/
		You("begin to %s the runes.",
		    spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" :
		    "memorize");
#else
		You("ルーン文字を%sしはじめた。",
		    spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "詠唱" :
		    "記憶");
#endif
	}

	book = spellbook;
/*JP
	set_occupation(learn, "studying", 0);
*/
	set_occupation(learn, "学ぶ", 0);
	return(1);
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(obj)
struct obj *obj;
{
	if (obj == book) book = (struct obj *)0;
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
	if (old_obj == book) book = new_obj;
}

/* called from moveloop() */
void
age_spells()
{
	int i;
	/*
	 * The time relative to the hero (a pass through move
	 * loop) causes all spell knowledge to be decremented.
	 * The hero's speed, rest status, conscious status etc.
	 * does not alter the loss of memory.
	 */
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++)
	    if (spellknow(i))
		decrnknow(i);
	return;
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
STATIC_OVL boolean
getspell(spell_no)
	int *spell_no;
{
	int nspells, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	if (spellid(0) == NO_SPELL)  {
/*JP
	    You("don't know any spells right now.");
*/
	    You("今のところ何の魔法も知らない。");
	    return FALSE;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
	    /* we know there is at least 1 known spell */
	    for (nspells = 1; nspells < MAXSPELL
			    && spellid(nspells) != NO_SPELL; nspells++)
		continue;

	    if (nspells == 1)  Strcpy(lets, "a");
	    else if (nspells < 27)  Sprintf(lets, "a-%c", 'a' + nspells - 1);
	    else if (nspells == 27)  Sprintf(lets, "a-zA");
	    else Sprintf(lets, "a-zA-%c", 'A' + nspells - 27);

	    for(;;)  {
/*JP
		Sprintf(qbuf, "Cast which spell? [%s ?]", lets);
*/
		Sprintf(qbuf, "どの魔法を唱える？[%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
		    break;

		if (index(quitchars, ilet))
		    return FALSE;

		idx = spell_let_to_idx(ilet);
		if (idx >= 0 && idx < nspells) {
		    *spell_no = idx;
		    return TRUE;
		} else
/*JP
		    You("don't know that spell.");
*/
		    You("そんな魔法は知らない。");
	    }
	}
#if 0 /*JP*/
	return dospellmenu("Choose which spell to cast",
			   SPELLMENU_CAST, spell_no);
#else
	return dospellmenu("どの魔法を唱える？",
			   SPELLMENU_CAST, spell_no);
#endif
}

/* the 'Z' command -- cast a spell */
int
docast()
{
	int spell_no;

	if (getspell(&spell_no))
	    return spelleffects(spell_no, FALSE);
	return 0;
}

STATIC_OVL const char *
spelltypemnemonic(skill)
int skill;
{
	switch (skill) {
	    case P_ATTACK_SPELL:
/*JP
		return "attack";
*/
		return "攻撃";
	    case P_HEALING_SPELL:
/*JP
		return "healing";
*/
		return "治癒";
	    case P_DIVINATION_SPELL:
/*JP
		return "divination";
*/
		return "予知";
	    case P_ENCHANTMENT_SPELL:
/*JP
		return "enchantment";
*/
		return "補助";
	    case P_CLERIC_SPELL:
/*JP
		return "clerical";
*/
		return "僧侶";
	    case P_ESCAPE_SPELL:
/*JP
		return "escape";
*/
		return "脱出";
	    case P_MATTER_SPELL:
/*JP
		return "matter";
*/
		return "物質";
	    default:
		impossible("Unknown spell skill, %d;", skill);
		return "";
	}
}

int
spell_skilltype(booktype)
int booktype;
{
	return (objects[booktype].oc_skill);
}

STATIC_OVL void
cast_protection()
{
	int loglev = 0;
	int l = u.ulevel;
	int natac = u.uac - u.uspellprot;
	int gain;

	/* loglev=log2(u.ulevel)+1 (1..5) */
	while (l) {
	    loglev++;
	    l /= 2;
	}

	/* The more u.uspellprot you already have, the less you get,
	 * and the better your natural ac, the less you get.
	 *
	 *	LEVEL AC    SPELLPROT from sucessive SPE_PROTECTION casts
	 *      1     10    0,  1,  2,  3,  4
	 *      1      0    0,  1,  2,  3
	 *      1    -10    0,  1,  2
	 *      2-3   10    0,  2,  4,  5,  6,  7,  8
	 *      2-3    0    0,  2,  4,  5,  6
	 *      2-3  -10    0,  2,  3,  4
	 *      4-7   10    0,  3,  6,  8,  9, 10, 11, 12
	 *      4-7    0    0,  3,  5,  7,  8,  9
	 *      4-7  -10    0,  3,  5,  6
	 *      7-15 -10    0,  3,  5,  6
	 *      8-15  10    0,  4,  7, 10, 12, 13, 14, 15, 16
	 *      8-15   0    0,  4,  7,  9, 10, 11, 12
	 *      8-15 -10    0,  4,  6,  7,  8
	 *     16-30  10    0,  5,  9, 12, 14, 16, 17, 18, 19, 20
	 *     16-30   0    0,  5,  9, 11, 13, 14, 15
	 *     16-30 -10    0,  5,  8,  9, 10
	 */
	gain = loglev - (int)u.uspellprot / (4 - min(3,(10 - natac)/10));

	if (gain > 0) {
	    if (!Blind) {
		const char *hgolden = hcolor(NH_GOLDEN);

		if (u.uspellprot)
/*JP
		    pline_The("%s haze around you becomes more dense.",
*/
		    pline("あなたのまわりの%s霞が濃くなった。",
			      hgolden);
		else
#if 0 /*JP*/
		    pline_The("%s around you begins to shimmer with %s haze.",
			/*[ what about being inside solid rock while polyd? ]*/
			(Underwater || Is_waterlevel(&u.uz)) ? "water" : "air",
			      an(hgolden));
#else
		    pline("あなたのまわりに%s霧があらわれてキラキラと光った。",
			      hgolden);
#endif
	    }
	    u.uspellprot += gain;
	    u.uspmtime =
		P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT ? 20 : 10;
	    if (!u.usptime)
		u.usptime = u.uspmtime;
	    find_ac();
	} else {
/*JP
	    Your("skin feels warm for a moment.");
*/
	    Your("肌は一瞬ポカポカした。");
	}
}

/* attempting to cast a forgotten spell will cause disorientation */
STATIC_OVL void
spell_backfire(spell)
int spell;
{
    long duration = (long)((spellev(spell) + 1) * 3);	 /* 6..24 */

    /* prior to 3.4.1, the only effect was confusion; it still predominates */
    switch (rn2(10)) {
    case 0:
    case 1:
    case 2:
    case 3: make_confused(duration, FALSE);			/* 40% */
	    break;
    case 4:
    case 5:
    case 6: make_confused(2L * duration / 3L, FALSE);		/* 30% */
	    make_stunned(duration / 3L, FALSE);
	    break;
    case 7:
    case 8: make_stunned(2L * duration / 3L, FALSE);		/* 20% */
	    make_confused(duration / 3L, FALSE);
	    break;
    case 9: make_stunned(duration, FALSE);			/* 10% */
	    break;
    }
    return;
}

int
spelleffects(spell, atme)
int spell;
boolean atme;
{
	int energy, damage, chance, n, intell;
	int skill, role_skill;
	boolean confused = (Confusion != 0);
	struct obj *pseudo;
	coord cc;

	/*
	 * Spell casting no longer affects knowledge of the spell. A
	 * decrement of spell knowledge is done every turn.
	 */
	if (spellknow(spell) <= 0) {
/*JP
	    Your("knowledge of this spell is twisted.");
*/
	    Your("この魔法に関する知識はよじれた。");
/*JP
	    pline("It invokes nightmarish images in your mind...");
*/
	    pline("それは悪夢を心に浮ばせた…");
	    spell_backfire(spell);
	    return(0);
	} else if (spellknow(spell) <= 100) {
/*JP
	    You("strain to recall the spell.");
*/
	    You("魔法を思いだそうとした。");
	} else if (spellknow(spell) <= 1000) {
/*JP
	    Your("knowledge of this spell is growing faint.");
*/
	    Your("この魔法に関する知識が消えてゆく。");
	}
	energy = (spellev(spell) * 5);    /* 5 <= energy <= 35 */

	if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
/*JP
		You("are too hungry to cast that spell.");
*/
	        pline("腹が減りすぎて魔法を唱えられない。");
		return(0);
	} else if (ACURR(A_STR) < 4)  {
/*JP
		You("lack the strength to cast spells.");
*/
		pline("魔法を唱えるには腕力が足りない。");
		return(0);
	} else if(check_capacity(
/*JP
		"Your concentration falters while carrying so much stuff.")) {
*/
		"たくさんものを持ちすぎて集中できない。")){
	    return (1);
	} else if (!freehand()) {
/*JP
		Your("arms are not free to cast!");
*/
		pline("魔法を唱えようにも腕の自由が効かない！");
		return (0);
	}

	if (u.uhave.amulet) {
/*JP
		You_feel("the amulet draining your energy away.");
*/
		pline("魔除けがあなたのエネルギーを吸いとっているように感じた。");
		energy += rnd(2*energy);
	}
	if(energy > u.uen)  {
/*JP
		You("don't have enough energy to cast that spell.");
*/
		pline("魔法を唱えるだけの十分なエネルギーを持っていない。");
		return(0);
	} else {
		if (spellid(spell) != SPE_DETECT_FOOD) {
			int hungr = energy * 2;

			/* If hero is a wizard, their current intelligence
			 * (bonuses + temporary + current)
			 * affects hunger reduction in casting a spell.
			 * 1. int = 17-18 no reduction
			 * 2. int = 16    1/4 hungr
			 * 3. int = 15    1/2 hungr
			 * 4. int = 1-14  normal reduction
			 * The reason for this is:
			 * a) Intelligence affects the amount of exertion
			 * in thinking.
			 * b) Wizards have spent their life at magic and
			 * understand quite well how to cast spells.
			 */
			intell = acurr(A_INT);
			if (!Role_if(PM_WIZARD)) intell = 10;
			switch (intell) {
				case 25: case 24: case 23: case 22:
				case 21: case 20: case 19: case 18:
				case 17: hungr = 0; break;
				case 16: hungr /= 4; break;
				case 15: hungr /= 2; break;
			}
			/* don't put player (quite) into fainting from
			 * casting a spell, particularly since they might
			 * not even be hungry at the beginning; however,
			 * this is low enough that they must eat before
			 * casting anything else except detect food
			 */
			if (hungr > u.uhunger-3)
				hungr = u.uhunger-3;
			morehungry(hungr);
		}
	}

	chance = percent_success(spell);
	if (confused || (rnd(100) > chance)) {
/*JP
		You("fail to cast the spell correctly.");
*/
		You("魔法を正しく唱えることができなかった。");
		u.uen -= energy / 2;
		flags.botl = 1;
		return(1);
	}

	u.uen -= energy;
	flags.botl = 1;
	exercise(A_WIS, TRUE);
	/* pseudo is a temporary "false" object containing the spell stats */
	pseudo = mksobj(spellid(spell), FALSE, FALSE);
	pseudo->blessed = pseudo->cursed = 0;
	pseudo->quan = 20L;			/* do not let useup get it */
	/*
	 * Find the skill the hero has in a spell type category.
	 * See spell_skilltype for categories.
	 */
	skill = spell_skilltype(pseudo->otyp);
	role_skill = P_SKILL(skill);

	switch(pseudo->otyp)  {
	/*
	 * At first spells act as expected.  As the hero increases in skill
	 * with the appropriate spell type, some spells increase in their
	 * effects, e.g. more damage, further distance, and so on, without
	 * additional cost to the spellcaster.
	 */
	case SPE_CONE_OF_COLD:
	case SPE_FIREBALL:
	case SPE_LIGHTNING:
                if (((pseudo->otyp != SPE_LIGHTNING)&&(role_skill >= P_SKILLED))
                    ||((pseudo->otyp == SPE_LIGHTNING)&&(role_skill >= P_EXPERT))) {
		  /*	    if (role_skill >= P_SKILLED) {*/
	        if (throwspell()) {
		    cc.x=u.dx;cc.y=u.dy;
		    n=rnd(8)+1;
		    while(n--) {
			if(!u.dx && !u.dy && !u.dz) {
			    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
#if 0 /*JP*/
				char buf[BUFSZ];
				Sprintf(buf, "zapped %sself with a spell", uhim());
				losehp(damage, buf, NO_KILLER_PREFIX);
#else
				losehp(damage, "自分自身の魔法を浴びて",
				       KILLED_BY);
#endif
			    }
			} else {
			    explode(u.dx, u.dy,
				    (pseudo->otyp == SPE_LIGHTNING) ? 15 : /* */
				    pseudo->otyp - SPE_MAGIC_MISSILE + 10,
				    u.ulevel/2 + 1 + spell_damage_bonus(), 0,
				    (pseudo->otyp == SPE_LIGHTNING) ? EXPL_MAGICAL: /* */
					(pseudo->otyp == SPE_CONE_OF_COLD) ?
						EXPL_FROSTY : EXPL_FIERY);
			}
			u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
			if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
			    IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
			    /* Spell is reflected back to center */
			    u.dx = cc.x;
			    u.dy = cc.y;
		        }
		    }
		}
		break;
	    } /* else fall through... */

	/* these spells are all duplicates of wand effects */
	case SPE_FORCE_BOLT:
	case SPE_SLEEP:
	case SPE_MAGIC_MISSILE:
	case SPE_KNOCK:
	case SPE_SLOW_MONSTER:
	case SPE_WIZARD_LOCK:
	case SPE_DIG:
	case SPE_TURN_UNDEAD:
	case SPE_POLYMORPH:
	case SPE_TELEPORT_AWAY:
	case SPE_CANCELLATION:
	case SPE_FINGER_OF_DEATH:
	case SPE_LIGHT:
	case SPE_DETECT_UNSEEN:
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
	case SPE_DRAIN_LIFE:
	case SPE_STONE_TO_FLESH:
		if (!(objects[pseudo->otyp].oc_dir == NODIR)) {
			if (atme) u.dx = u.dy = u.dz = 0;
			else if (!getdir((char *)0)) {
			    /* getdir cancelled, re-use previous direction */
/*JP
			    pline_The("magical energy is released!");
*/
			    pline("魔法のエネルギーが解放された！");
			}
			if(!u.dx && !u.dy && !u.dz) {
			    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
#if 0 /*JP*/
				char buf[BUFSZ];
				Sprintf(buf, "zapped %sself with a spell", uhim());
				losehp(damage, buf, NO_KILLER_PREFIX);
#else
				losehp(damage,
				     "自分自身の魔法を浴びて",
				     KILLED_BY);
#endif
			    }
			} else weffects(pseudo);
		} else weffects(pseudo);
		update_inventory();	/* spell may modify inventory */
		break;

	/* these are all duplicates of scroll effects */
	case SPE_REMOVE_CURSE:
	case SPE_CONFUSE_MONSTER:
	case SPE_DETECT_FOOD:
	case SPE_CAUSE_FEAR:
		/* high skill yields effect equivalent to blessed scroll */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
		/* fall through */
	case SPE_CHARM_MONSTER:
	case SPE_MAGIC_MAPPING:
	case SPE_CREATE_MONSTER:
	case SPE_IDENTIFY:
		(void) seffects(pseudo);
		break;

	/* these are all duplicates of potion effects */
	case SPE_HASTE_SELF:
	case SPE_DETECT_TREASURE:
	case SPE_DETECT_MONSTERS:
	case SPE_LEVITATION:
	case SPE_RESTORE_ABILITY:
		/* high skill yields effect equivalent to blessed potion */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
		/* fall through */
	case SPE_INVISIBILITY:
		(void) peffects(pseudo);
		break;

	case SPE_CURE_BLINDNESS:
		healup(0, 0, FALSE, TRUE);
		break;
	case SPE_CURE_SICKNESS:
/*JP
		if (Sick) You("are no longer ill.");
*/
		if (Sick) Your("病気は直った。");
		if (Slimed) {
/*JP
		    pline_The("slime disappears!");
*/
		    pline("スライムは消えた！");
		    Slimed = 0;
		 /* flags.botl = 1; -- healup() handles this */
		}
		healup(0, 0, TRUE, FALSE);
		break;
	case SPE_CREATE_FAMILIAR:
		(void) make_familiar((struct obj *)0, u.ux, u.uy, FALSE);
		break;
	case SPE_CLAIRVOYANCE:
		if (!BClairvoyant)
		    do_vicinity_map();
		/* at present, only one thing blocks clairvoyance */
		else if (uarmh && uarmh->otyp == CORNUTHAUM)
/*JP
		    You("sense a pointy hat on top of your %s.",
*/
		    You("とがった帽子を%sの上に発見した。",
			body_part(HEAD));
		break;
	case SPE_PROTECTION:
		cast_protection();
		break;
	case SPE_JUMPING:
		if (!jump(max(role_skill,1)))
			pline(nothing_happens);
		break;
	default:
		impossible("Unknown spell %d attempted.", spell);
		obfree(pseudo, (struct obj *)0);
		return(0);
	}

	/* gain skill for successful cast */
	use_skill(skill, spellev(spell));

	obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	return(1);
}

/* Choose location where spell takes effect. */
STATIC_OVL int
throwspell()
{
	coord cc;

	if (u.uinwater) {
/*JP
	    pline("You're joking! In this weather?"); return 0;
*/
	    pline("ご冗談を！この天気で？"); return 0;
	} else if (Is_waterlevel(&u.uz)) {
/*JP
	    You("had better wait for the sun to come out."); return 0;
*/
	    You("太陽が見えるまで待ったほうがよいだろう。"); return 0;
	}

/*JP
	pline("Where do you want to cast the spell?");
*/
	pline("どこに向かって魔法を唱える？");
	cc.x = u.ux;
	cc.y = u.uy;
/*JP
	if (getpos(&cc, TRUE, "the desired position") < 0)
*/
	if (getpos(&cc, TRUE, "望みの場所") < 0)
	    return 0;	/* user pressed ESC */
	/* The number of moves from hero to where the spell drops.*/
	if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
/*JP
	    pline_The("spell dissipates over the distance!");
*/
	    pline("遠すぎる！");
	    return 0;
	} else if (u.uswallow) {
/*JP
	    pline_The("spell is cut short!");
*/
	    pline("だめだ！近すぎる！");
	    exercise(A_WIS, FALSE); /* What were you THINKING! */
	    u.dx = 0;
	    u.dy = 0;
	    return 1;
	} else if (!cansee(cc.x, cc.y) || IS_STWALL(levl[cc.x][cc.y].typ)) {
/*JP
	    Your("mind fails to lock onto that location!");
*/
	    You("そこに向かって集中したが失敗した！");
	    return 0;
	} else {
	    u.dx=cc.x;
	    u.dy=cc.y;
	    return 1;
	}
}

void
losespells()
{
	boolean confused = (Confusion != 0);
	int  n, nzap, i;

	book = 0;
	for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; n++)
		continue;
	if (n) {
		nzap = rnd(n) + confused ? 1 : 0;
		if (nzap > n) nzap = n;
		for (i = n - nzap; i < n; i++) {
		    spellid(i) = NO_SPELL;
		    exercise(A_WIS, FALSE);	/* ouch! */
		}
	}
}

/* the '+' command -- view known spells */
int
dovspell()
{
	char qbuf[QBUFSZ];
	int splnum, othnum;
	struct spell spl_tmp;

	if (spellid(0) == NO_SPELL)
/*JP
	    You("don't know any spells right now.");
*/
	    You("魔法を知らない。");
	else {
#if 0 /*JP*/
	    while (dospellmenu("Currently known spells",
			       SPELLMENU_VIEW, &splnum)) {
		Sprintf(qbuf, "Reordering spells; swap '%c' with",
#else
	    while (dospellmenu("現在知っている魔法一覧",
			       SPELLMENU_VIEW, &splnum)) {
		Sprintf(qbuf, "'%c'と並び変える魔法は？",
#endif
			spellet(splnum));
		if (!dospellmenu(qbuf, splnum, &othnum)) break;

		spl_tmp = spl_book[splnum];
		spl_book[splnum] = spl_book[othnum];
		spl_book[othnum] = spl_tmp;
	    }
	}
	return 0;
}

STATIC_OVL boolean
dospellmenu(prompt, splaction, spell_no)
const char *prompt;
int splaction;	/* SPELLMENU_CAST, SPELLMENU_VIEW, or spl_book[] index */
int *spell_no;
{
	winid tmpwin;
	int i, n, how;
	char buf[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	/*
	 * The correct spacing of the columns depends on the
	 * following that (1) the font is monospaced and (2)
	 * that selection letters are pre-pended to the given
	 * string and are of the form "a - ".
	 *
	 * To do it right would require that we implement columns
	 * in the window-ports (say via a tab character).
	 */
	if (!iflags.menu_tab_sep)
/*JP
		Sprintf(buf, "%-20s     Level  %-12s Fail", "    Name", "Category");
*/
		Sprintf(buf, "%-20s     Level  %-12s 成功率", "    Name", "分類");
	else
/*JP
		Sprintf(buf, "Name\tLevel\tCategory\tFail");
*/
		Sprintf(buf, "Name\tLevel\t分類\t成功率");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
		Sprintf(buf, iflags.menu_tab_sep ?
			"%s\t%-d%s\t%s\t%-d%%" : "%-20s  %2d%s   %-12s %3d%%",
#if 0 /*JP*/
			spellname(i), spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			100 - percent_success(i));
#else
			jtrns_obj('+', spellname(i)), spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			percent_success(i));
#endif

		any.a_int = i+1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			 spellet(i), 0, ATR_NONE, buf,
			 (i == splaction) ? MENU_SELECTED : MENU_UNSELECTED);
	      }
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	if (splaction == SPELLMENU_VIEW && spellid(1) == NO_SPELL)
	    how = PICK_NONE;	/* only one spell => nothing to swap with */
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
		*spell_no = selected[0].item.a_int - 1;
		/* menu selection for `PICK_ONE' does not
		   de-select any preselected entry */
		if (n > 1 && *spell_no == splaction)
		    *spell_no = selected[1].item.a_int - 1;
		free((genericptr_t)selected);
		/* default selection of preselected spell means that
		   user chose not to swap it with anything */
		if (*spell_no == splaction) return FALSE;
		return TRUE;
	} else if (splaction >= 0) {
	    /* explicit de-selection of preselected spell means that
	       user is still swapping but not for the current spell */
	    *spell_no = splaction;
	    return TRUE;
	}
	return FALSE;
}

/* Integer square root function without using floating point. */
STATIC_OVL int
isqrt(val)
int val;
{
    int rt = 0;
    int odd = 1;
    while(val >= odd) {
	val = val-odd;
	odd = odd+2;
	rt = rt + 1;
    }
    return rt;
}

STATIC_OVL int
percent_success(spell)
int spell;
{
	/* Intrinsic and learned ability are combined to calculate
	 * the probability of player's success at cast a given spell.
	 */
	int chance, splcaster, special, statused;
	int difficulty;
	int skill;

	/* Calculate intrinsic ability (splcaster) */

	splcaster = urole.spelbase;
	special = urole.spelheal;
	statused = ACURR(urole.spelstat);

	if (uarm && is_metallic(uarm))
	    splcaster += (uarmc && uarmc->otyp == ROBE) ?
		urole.spelarmr/2 : urole.spelarmr;
	else if (uarmc && uarmc->otyp == ROBE)
	    splcaster -= urole.spelarmr;
	if (uarms) splcaster += urole.spelshld;

	if (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE)
		splcaster += uarmhbon;
	if (uarmg && is_metallic(uarmg)) splcaster += uarmgbon;
	if (uarmf && is_metallic(uarmf)) splcaster += uarmfbon;

	if (spellid(spell) == urole.spelspec)
		splcaster += urole.spelsbon;


	/* `healing spell' bonus */
	if (spellid(spell) == SPE_HEALING ||
	    spellid(spell) == SPE_EXTRA_HEALING ||
	    spellid(spell) == SPE_CURE_BLINDNESS ||
	    spellid(spell) == SPE_CURE_SICKNESS ||
	    spellid(spell) == SPE_RESTORE_ABILITY ||
	    spellid(spell) == SPE_REMOVE_CURSE) splcaster += special;

	if (splcaster > 20) splcaster = 20;

	/* Calculate learned ability */

	/* Players basic likelihood of being able to cast any spell
	 * is based of their `magic' statistic. (Int or Wis)
	 */
	chance = 11 * statused / 2;

	/*
	 * High level spells are harder.  Easier for higher level casters.
	 * The difficulty is based on the hero's level and their skill level
	 * in that spell type.
	 */
	skill = P_SKILL(spell_skilltype(spellid(spell)));
	skill = max(skill,P_UNSKILLED) - 1;	/* unskilled => 0 */
	difficulty= (spellev(spell)-1) * 4 - ((skill * 6) + (u.ulevel/3) + 1);

	if (difficulty > 0) {
		/* Player is too low level or unskilled. */
		chance -= isqrt(900 * difficulty + 2000);
	} else {
		/* Player is above level.  Learning continues, but the
		 * law of diminishing returns sets in quickly for
		 * low-level spells.  That is, a player quickly gains
		 * no advantage for raising level.
		 */
		int learning = 15 * -difficulty / spellev(spell);
		chance += learning > 20 ? 20 : learning;
	}

	/* Clamp the chance: >18 stat and advanced learning only help
	 * to a limit, while chances below "hopeless" only raise the
	 * specter of overflowing 16-bit ints (and permit wearing a
	 * shield to raise the chances :-).
	 */
	if (chance < 0) chance = 0;
	if (chance > 120) chance = 120;

	/* Wearing anything but a light shield makes it very awkward
	 * to cast a spell.  The penalty is not quite so bad for the
	 * player's role-specific spell.
	 */
	if (uarms && weight(uarms) > (int) objects[SMALL_SHIELD].oc_weight) {
		if (spellid(spell) == urole.spelspec) {
			chance /= 2;
		} else {
			chance /= 4;
		}
	}

	/* Finally, chance (based on player intell/wisdom and level) is
	 * combined with ability (based on player intrinsics and
	 * encumbrances).  No matter how intelligent/wise and advanced
	 * a player is, intrinsics and encumbrance can prevent casting;
	 * and no matter how able, learning is always required.
	 */
	chance = chance * (20-splcaster) / 15 - splcaster;

	/* Clamp to percentile */
	if (chance > 100) chance = 100;
	if (chance < 0) chance = 0;

	return chance;
}


/* Learn a spell during creation of the initial inventory */
void
initialspell(obj)
struct obj *obj;
{
	int i;

	for (i = 0; i < MAXSPELL; i++) {
	    if (spellid(i) == obj->otyp) {
	         pline("Error: Spell %s already known.",
	         		OBJ_NAME(objects[obj->otyp]));
	         return;
	    }
	    if (spellid(i) == NO_SPELL)  {
	        spl_book[i].sp_id = obj->otyp;
	        spl_book[i].sp_lev = objects[obj->otyp].oc_level;
	        incrnknow(i);
	        return;
	    }
	}
	impossible("Too many spells memorized!");
	return;
}


/*spell.c*/
