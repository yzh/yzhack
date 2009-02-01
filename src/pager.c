/*	SCCS Id: @(#)pager.c	3.4	2003/08/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routines dowhatis() and dohelp() and */
/* a few other help related facilities */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "dlb.h"

STATIC_DCL boolean FDECL(is_swallow_sym, (int));
STATIC_DCL int FDECL(append_str, (char *, const char *));
/*JP
STATIC_DCL struct permonst * FDECL(lookat, (int, int, char *, char *));
*/
STATIC_DCL struct permonst * FDECL(lookat, (int, int, char *, char *, char *));
STATIC_DCL void FDECL(checkfile,
		      (char *,struct permonst *,BOOLEAN_P,BOOLEAN_P));
STATIC_DCL int FDECL(do_look, (BOOLEAN_P));
STATIC_DCL boolean FDECL(help_menu, (int *));
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif

/* Returns "true" for characters that could represent a monster's stomach. */
STATIC_OVL boolean
is_swallow_sym(c)
int c;
{
    int i;
    for (i = S_sw_tl; i <= S_sw_br; i++)
	if ((int)showsyms[i] == c) return TRUE;
    return FALSE;
}

/*
 * Append new_str to the end of buf if new_str doesn't already exist as
 * a substring of buf.  Return 1 if the string was appended, 0 otherwise.
 * It is expected that buf is of size BUFSZ.
 */
STATIC_OVL int
append_str(buf, new_str)
    char *buf;
    const char *new_str;
{
    int space_left;	/* space remaining in buf */

    if (strstri(buf, new_str)) return 0;

    space_left = BUFSZ - strlen(buf) - 1;
#if 0 /*JP*/
    (void) strncat(buf, " or ", space_left);
    (void) strncat(buf, new_str, space_left - 4);
#else
    (void) strncat(buf, "または", space_left);
    (void) strncat(buf, new_str, space_left - 6);
#endif
    return 1;
}

/*
 * Return the name of the glyph found at (x,y).
 * If not hallucinating and the glyph is a monster, also monster data.
 */
STATIC_OVL struct permonst *
#if 0 /*JP*/
lookat(x, y, buf, monbuf)
    int x, y;
    char *buf, *monbuf;
#else
/*JP*/ /* `buf2' is simple name for search data file easily. */
lookat(x, y, buf, monbuf, buf2)
    int x, y;
    char *buf, *monbuf, *buf2;
#endif
{
    register struct monst *mtmp = (struct monst *) 0;
    struct permonst *pm = (struct permonst *) 0;
    int glyph;

#if 1 /*JP*/
    if(!query_lang_mode())
      (void)set_trns_mode(0);

    buf[0] = 0;
    buf2[0] = 0;
#endif
    buf[0] = monbuf[0] = 0;
    glyph = glyph_at(x,y);
    if (u.ux == x && u.uy == y && senseself()) {
	char race[QBUFSZ];

	/* if not polymorphed, show both the role and the race */
	race[0] = 0;
	if (!Upolyd) {
	    Sprintf(race, "%s ", urace.adj);
	}

#if 0 /*JP*/
	Sprintf(buf, "%s%s%s called %s",
		Invis ? "invisible " : "",
		race,
		mons[u.umonnum].mname,
		plname);
	/* file lookup can't distinguish between "gnomish wizard" monster
	   and correspondingly named player character, always picking the
	   former; force it to find the general "wizard" entry instead */
	if (Role_if(PM_WIZARD) && Race_if(PM_GNOME) && !Upolyd)
	    pm = &mons[PM_WIZARD];

#else
	Sprintf(buf, "%s%sという名の%s",
		Invis ? "姿の見えない" : "",
		plname,
		jtrns_mon(mons[u.umonnum].mname));
	Strcpy(buf2, mons[u.umonnum].mname);
#endif
#ifdef STEED
	if (u.usteed) {
	    char steedbuf[BUFSZ];

/*JP
	    Sprintf(steedbuf, ", mounted on %s", y_monnam(u.usteed));
*/
	    Sprintf(steedbuf, "、%sに乗っている", y_monnam(u.usteed));
	    /* assert((sizeof buf >= strlen(buf)+strlen(steedbuf)+1); */
	    Strcat(buf, steedbuf);
	}
#endif
	/* When you see yourself normally, no explanation is appended
	   (even if you could also see yourself via other means).
	   Sensing self while blind or swallowed is treated as if it
	   were by normal vision (cf canseeself()). */
	if ((Invisible || u.uundetected) && !Blind && !u.uswallow) {
	    unsigned how = 0;

	    if (Infravision)	 how |= 1;
	    if (Unblind_telepat) how |= 2;
	    if (Detect_monsters) how |= 4;

	    if (how)
		Sprintf(eos(buf), " [seen: %s%s%s%s%s]",
			(how & 1) ? "infravision" : "",
			/* add comma if telep and infrav */
			((how & 3) > 2) ? ", " : "",
			(how & 2) ? "telepathy" : "",
			/* add comma if detect and (infrav or telep or both) */
			((how & 7) > 4) ? ", " : "",
			(how & 4) ? "monster detection" : "");
	}
    } else if (u.uswallow) {
	/* all locations when swallowed other than the hero are the monster */
#if 0 /*JP*/
	Sprintf(buf, "interior of %s",
				    Blind ? "a monster" : a_monnam(u.ustuck));
#else
	Sprintf(buf2, "interior of %s",
				    Blind ? "a monster" : a_monnam(u.ustuck));
	Sprintf(buf, "%sの内部",
				    Blind ? "怪物" : a_monnam(u.ustuck));
#endif
	pm = u.ustuck->data;
    } else if (glyph_is_monster(glyph)) {
	bhitpos.x = x;
	bhitpos.y = y;
	mtmp = m_at(x,y);
	if (mtmp != (struct monst *) 0) {
	    char *name, monnambuf[BUFSZ];
	    boolean accurate = !Hallucination;

	    if (mtmp->data == &mons[PM_COYOTE] && accurate)
		name = coyotename(mtmp, monnambuf);
	    else
		name = distant_monnam(mtmp, ARTICLE_NONE, monnambuf);

	    pm = mtmp->data;
#if 0 /*JP*/
	    Sprintf(buf, "%s%s%s",
		    (mtmp->mx != x || mtmp->my != y) ?
			((mtmp->isshk && accurate)
				? "tail of " : "tail of a ") : "",
		    (mtmp->mtame && accurate) ? "tame " :
		    (mtmp->mpeaceful && accurate) ? "peaceful " : "",
		    name);
#else
	    Strcpy(buf2, mtmp->data->mname);
	    Sprintf(buf, "%s%s%s",
		    (mtmp->mtame && accurate) ? "手なずけられた" :
		    (mtmp->mpeaceful && accurate) ? "友好的な" : "",
		    name,
		    (mtmp->mx != x || mtmp->my != y) ?
			((mtmp->isshk && accurate)
				? "の尻尾" : "の尻尾") : "");
#endif
	    if (u.ustuck == mtmp)
#if 0 /*JP*/
		Strcat(buf, (Upolyd && sticks(youmonst.data)) ?
			", being held" : ", holding you");
#else
		Strcat(buf, (Upolyd && sticks(youmonst.data)) ?
			"、あなたが掴まえている" : "、あなたを掴まえている");
#endif
	    if (mtmp->mleashed)
#if 0 /*JP*/
		Strcat(buf, ", leashed to you");
#else
		Strcat(buf, "、紐で結ばれている");
#endif

	    if (mtmp->mtrapped && cansee(mtmp->mx, mtmp->my)) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);
		int tt = t ? t->ttyp : NO_TRAP;

		/* newsym lets you know of the trap, so mention it here */
		if (tt == BEAR_TRAP || tt == PIT ||
			tt == SPIKED_PIT || tt == WEB)
#if 0 /*JP*/
		    Sprintf(eos(buf), ", trapped in %s",
			    an(defsyms[trap_to_defsym(tt)].explanation));
#else
		    Sprintf(eos(buf), ", %sに捕まっている",
			    jtrns_obj('^', defsyms[trap_to_defsym(tt)].explanation));
#endif
	    }

	    {
		int ways_seen = 0, normal = 0, xraydist;
		boolean useemon = (boolean) canseemon(mtmp);

		xraydist = (u.xray_range<0) ? -1 : u.xray_range * u.xray_range;
		/* normal vision */
		if ((mtmp->wormno ? worm_known(mtmp) : cansee(mtmp->mx, mtmp->my)) &&
			mon_visible(mtmp) && !mtmp->minvis) {
		    ways_seen++;
		    normal++;
		}
		/* see invisible */
		if (useemon && mtmp->minvis)
		    ways_seen++;
		/* infravision */
		if ((!mtmp->minvis || See_invisible) && see_with_infrared(mtmp))
		    ways_seen++;
		/* telepathy */
		if (tp_sensemon(mtmp))
		    ways_seen++;
		/* xray */
		if (useemon && xraydist > 0 &&
			distu(mtmp->mx, mtmp->my) <= xraydist)
		    ways_seen++;
		if (Detect_monsters)
		    ways_seen++;
		if (MATCH_WARN_OF_MON(mtmp))
		    ways_seen++;

		if (ways_seen > 1 || !normal) {
		    if (normal) {
/*JP
			Strcat(monbuf, "normal vision");
*/
			Strcat(monbuf, "通常の視覚");
			/* can't actually be 1 yet here */
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if (useemon && mtmp->minvis) {
/*JP
			Strcat(monbuf, "see invisible");
*/
			Strcat(monbuf, "見えないものを見る視覚");
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if ((!mtmp->minvis || See_invisible) &&
			    see_with_infrared(mtmp)) {
/*JP
			Strcat(monbuf, "infravision");
*/
			Strcat(monbuf, "赤外線が見える視覚");
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if (tp_sensemon(mtmp)) {
/*JP
			Strcat(monbuf, "telepathy");
*/
			Strcat(monbuf, "テレパシー");
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if (useemon && xraydist > 0 &&
			    distu(mtmp->mx, mtmp->my) <= xraydist) {
			/* Eyes of the Overworld */
/*JP
			Strcat(monbuf, "astral vision");
*/
			Strcat(monbuf, "精神による視覚");
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if (Detect_monsters) {
/*JP
			Strcat(monbuf, "monster detection");
*/
			Strcat(monbuf, "怪物を発見する能力");
/*JP
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
*/
			if (ways_seen-- > 1) Strcat(monbuf, "、");
		    }
		    if (MATCH_WARN_OF_MON(mtmp)) {
		    	char wbuf[BUFSZ];
			if (Hallucination)
/*JP
				Strcat(monbuf, "paranoid delusion");
*/
				Strcat(monbuf, "偏執的妄想");
			else {
#if 0 /*JP:T*/
				Sprintf(wbuf, "warned of %s",
					makeplural(mtmp->data->mname));
#else
				Sprintf(wbuf, "%sを警告している",
					jtrns_mon(mtmp->data->mname));
#endif
		    		Strcat(monbuf, wbuf);
		    	}
		    	if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		}
	    }
	}
    }
    else if (glyph_is_object(glyph)) {
	struct obj *otmp = vobj_at(x,y);

	if (!otmp || otmp->otyp != glyph_to_obj(glyph)) {
	    if (glyph_to_obj(glyph) != STRANGE_OBJECT) {
		otmp = mksobj(glyph_to_obj(glyph), FALSE, FALSE);
		if (otmp->oclass == COIN_CLASS)
		    otmp->quan = 2L; /* to force pluralization */
		else if (otmp->otyp == SLIME_MOLD)
		    otmp->spe = current_fruit;	/* give the fruit a type */
		Strcpy(buf, distant_name(otmp, xname));
		dealloc_obj(otmp);
	    }
	} else
	    Strcpy(buf, distant_name(otmp, xname));
#if 1 /*JP*/
	Strcpy(buf2, buf);
#endif

	if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR)
/*JP
	    Strcat(buf, " embedded in stone");
*/
	    Strcat(buf, "、岩に埋めこまれている");
	else if (IS_WALL(levl[x][y].typ) || levl[x][y].typ == SDOOR)
/*JP
	    Strcat(buf, " embedded in a wall");
*/
	    Strcat(buf, "、壁に埋めこまれている");
	else if (closed_door(x,y))
/*JP
	    Strcat(buf, " embedded in a door");
*/
	    Strcat(buf, "、扉に埋めこまれている");
	else if (is_pool(x,y))
	    /*JP 3.4.3 この部分は物体にしか使われない */
/*JP
	    Strcat(buf, " in water");
*/
	    Strcat(buf, "、水中にある");
	else if (is_lava(x,y))
#if 0 /*JP:T*/
	    Strcat(buf, " in molten lava");	/* [can this ever happen?] */
#else
	    Strcat(buf, "、溶岩の中にある");	/* [can this ever happen?] */
#endif
    } else if (glyph_is_trap(glyph)) {
	int tnum = what_trap(glyph_to_trap(glyph));
#if 0 /*JP:T*/
	Strcpy(buf, defsyms[trap_to_defsym(tnum)].explanation);
#else
	Strcpy(buf, jtrns_obj('^', defsyms[
	    trap_to_defsym(Hallucination ? rn2(TRAPNUM-3)+3 : tnum)].explanation));
#endif
    } else if(!glyph_is_cmap(glyph)) {
/*JP
	Strcpy(buf,"dark part of a room");
*/
	Strcpy(buf,"部屋の暗い部分");
    } else switch(glyph_to_cmap(glyph)) {
    case S_altar:
	if(!In_endgame(&u.uz))
#if 0 /*JP*/
	    Sprintf(buf, "%s altar",
		align_str(Amask2align(levl[x][y].altarmask & ~AM_SHRINE)));
#else
	    Sprintf(buf, "%sの祭壇",
		align_str(Amask2align(levl[x][y].altarmask & ~AM_SHRINE)));
#endif
/*JP
	else Sprintf(buf, "aligned altar");
*/
	else Sprintf(buf, "属性の祭壇");
	break;
    case S_ndoor:
	if (is_drawbridge_wall(x, y) >= 0)
/*JP
	    Strcpy(buf,"open drawbridge portcullis");
*/
	    Strcpy(buf,"降りている跳ね橋");
	else if ((levl[x][y].doormask & ~D_TRAPPED) == D_BROKEN)
/*JP
	    Strcpy(buf,"broken door");
*/
	    Strcpy(buf,"壊れた扉");
	else
/*JP
	    Strcpy(buf,"doorway");
*/
	    Strcpy(buf,"出入り口");
	break;
    case S_cloud:
/*JP
	Strcpy(buf, Is_airlevel(&u.uz) ? "cloudy area" : "fog/vapor cloud");
*/
	Strcpy(buf, Is_airlevel(&u.uz) ? "曇っている場所" : "霧/蒸気の雲");
	break;
    default:
/*JP
	Strcpy(buf,defsyms[glyph_to_cmap(glyph)].explanation);
*/
	Strcpy(buf, jtrns_obj('S', defsyms[glyph_to_cmap(glyph)].explanation));
	break;
    }
#if 1 /*JP*/
    if(buf2[0] == '\0'){
	Strcpy(buf2, buf);
    }
    set_trns_mode(1);
#endif

    return ((pm && !Hallucination) ? pm : (struct permonst *) 0);
}

/*
 * Look in the "data" file for more info.  Called if the user typed in the
 * whole name (user_typed_name == TRUE), or we've found a possible match
 * with a character/glyph and flags.help is TRUE.
 *
 * NOTE: when (user_typed_name == FALSE), inp is considered read-only and
 *	 must not be changed directly, e.g. via lcase(). We want to force
 *	 lcase() for data.base lookup so that we can have a clean key.
 *	 Therefore, we create a copy of inp _just_ for data.base lookup.
 */
STATIC_OVL void
checkfile(inp, pm, user_typed_name, without_asking)
    char *inp;
    struct permonst *pm;
    boolean user_typed_name, without_asking;
{
    dlb *fp;
    char buf[BUFSZ], newstr[BUFSZ];
    char *ep, *dbase_str;
    long txt_offset;
    int chk_skip;
    boolean found_in_file = FALSE, skipping_entry = FALSE;

    fp = dlb_fopen(DATAFILE, "r");
    if (!fp) {
#if 0 /*JP*/
	pline("Cannot open data file!");
#else
	pline("データファイルを開けない！");
#endif
	return;
    }

    /* To prevent the need for entries in data.base like *ngel to account
     * for Angel and angel, make the lookup string the same for both
     * user_typed_name and picked name.
     */
    if (pm != (struct permonst *) 0 && !user_typed_name)
	dbase_str = strcpy(newstr, pm->mname);
    else dbase_str = strcpy(newstr, inp);
    (void) lcase(dbase_str);

    if (!strncmp(dbase_str, "interior of ", 12))
	dbase_str += 12;
    if (!strncmp(dbase_str, "a ", 2))
	dbase_str += 2;
    else if (!strncmp(dbase_str, "an ", 3))
	dbase_str += 3;
    else if (!strncmp(dbase_str, "the ", 4))
	dbase_str += 4;
    if (!strncmp(dbase_str, "tame ", 5))
	dbase_str += 5;
    else if (!strncmp(dbase_str, "peaceful ", 9))
	dbase_str += 9;
    if (!strncmp(dbase_str, "invisible ", 10))
	dbase_str += 10;
    if (!strncmp(dbase_str, "statue of ", 10))
	dbase_str[6] = '\0';
    else if (!strncmp(dbase_str, "figurine of ", 12))
	dbase_str[8] = '\0';

    /* Make sure the name is non-empty. */
    if (*dbase_str) {
	/* adjust the input to remove "named " and convert to lower case */
	char *alt = 0;	/* alternate description */

	if ((ep = strstri(dbase_str, " named ")) != 0)
	    alt = ep + 7;
	else
	    ep = strstri(dbase_str, " called ");
	if (!ep) ep = strstri(dbase_str, ", ");
	if (ep && ep > dbase_str) *ep = '\0';

	/*
	 * If the object is named, then the name is the alternate description;
	 * otherwise, the result of makesingular() applied to the name is. This
	 * isn't strictly optimal, but named objects of interest to the user
	 * will usually be found under their name, rather than under their
	 * object type, so looking for a singular form is pointless.
	 */

	if (!alt)
	    alt = makesingular(dbase_str);
	else
	    if (user_typed_name)
		(void) lcase(alt);

	/* skip first record; read second */
	txt_offset = 0L;
	if (!dlb_fgets(buf, BUFSZ, fp) || !dlb_fgets(buf, BUFSZ, fp)) {
	    impossible("can't read 'data' file");
	    (void) dlb_fclose(fp);
	    return;
	} else if (sscanf(buf, "%8lx\n", &txt_offset) < 1 || txt_offset <= 0)
	    goto bad_data_file;

	/* look for the appropriate entry */
	while (dlb_fgets(buf,BUFSZ,fp)) {
	    if (*buf == '.') break;  /* we passed last entry without success */

	    if (digit(*buf)) {
		/* a number indicates the end of current entry */
		skipping_entry = FALSE;
	    } else if (!skipping_entry) {
		if (!(ep = index(buf, '\n'))) goto bad_data_file;
		*ep = 0;
		/* if we match a key that begins with "~", skip this entry */
		chk_skip = (*buf == '~') ? 1 : 0;
		if (pmatch(&buf[chk_skip], dbase_str) ||
			(alt && pmatch(&buf[chk_skip], alt))) {
		    if (chk_skip) {
			skipping_entry = TRUE;
			continue;
		    } else {
			found_in_file = TRUE;
			break;
		    }
		}
	    }
	}
    }

    if(found_in_file) {
	long entry_offset;
	int  entry_count;
	int  i;

	/* skip over other possible matches for the info */
	do {
	    if (!dlb_fgets(buf, BUFSZ, fp)) goto bad_data_file;
	} while (!digit(*buf));
	if (sscanf(buf, "%ld,%d\n", &entry_offset, &entry_count) < 2) {
bad_data_file:	impossible("'data' file in wrong format");
		(void) dlb_fclose(fp);
		return;
	}

/*JP
	if (user_typed_name || without_asking || yn("More info?") == 'y') {
*/
	if (user_typed_name || without_asking || yn("詳細を見る？") == 'y') {
	    winid datawin;

	    if (dlb_fseek(fp, txt_offset + entry_offset, SEEK_SET) < 0) {
#if 0 /*JP*/
		pline("? Seek error on 'data' file!");
#else
		pline("'data'ファイルのシークエラー！");
#endif
		(void) dlb_fclose(fp);
		return;
	    }
	    datawin = create_nhwindow(NHW_MENU);
	    for (i = 0; i < entry_count; i++) {
		if (!dlb_fgets(buf, BUFSZ, fp)) goto bad_data_file;
		if ((ep = index(buf, '\n')) != 0) *ep = 0;
		if (index(buf+1, '\t') != 0) (void) tabexpand(buf+1);
		putstr(datawin, 0, buf+1);
	    }
	    display_nhwindow(datawin, FALSE);
	    destroy_nhwindow(datawin);
	}
    } else if (user_typed_name)
/*JP
	pline("I don't have any information on those things.");
*/
	pline("そんな名前は聞いたことがない。");

    (void) dlb_fclose(fp);
}

/* getpos() return values */
#define LOOK_TRADITIONAL	0	/* '.' -- ask about "more info?" */
#define LOOK_QUICK		1	/* ',' -- skip "more info?" */
#define LOOK_ONCE		2	/* ';' -- skip and stop looping */
#define LOOK_VERBOSE		3	/* ':' -- show more info w/o asking */

/* also used by getpos hack in do_name.c */
/*JP
const char what_is_an_unknown_object[] = "an unknown object";
*/
const char what_is_an_unknown_object[] = "謎の物体";

STATIC_OVL int
do_look(quick)
    boolean quick;	/* use cursor && don't search for "more info" */
{
    char    out_str[BUFSZ], look_buf[BUFSZ];
#if 1 /*JP*/
    char look_buf2[BUFSZ];
    const char *firstmatch2 = 0;
#endif
    const char *x_str, *firstmatch = 0;
    struct permonst *pm = 0;
    int     i, ans = 0;
    int     sym;		/* typed symbol or converted glyph */
    int	    found;		/* count of matching syms found */
    coord   cc;			/* screen pos of unknown glyph */
    boolean save_verbose;	/* saved value of flags.verbose */
    boolean from_screen;	/* question from the screen */
    boolean need_to_look;	/* need to get explan. from glyph */
    boolean hit_trap;		/* true if found trap explanation */
    int skipped_venom;		/* non-zero if we ignored "splash of venom" */
/*JP
    static const char *mon_interior = "the interior of a monster";
*/
    static const char *mon_interior = "怪物の内部";

    if (quick) {
	from_screen = TRUE;	/* yes, we want to use the cursor */
    } else {
/*JP
	i = ynq("Specify unknown object by cursor?");
*/
	i = ynq("カーソルで物体を指定する？");
	if (i == 'q') return 0;
	from_screen = (i == 'y');
    }

    if (from_screen) {
	cc.x = u.ux;
	cc.y = u.uy;
	sym = 0;		/* gcc -Wall lint */
    } else {
/*JP
	getlin("Specify what? (type the word)", out_str);
*/
	getlin("何を調べる？(文字を入れてね)", out_str);
	if (out_str[0] == '\0' || out_str[0] == '\033')
	    return 0;

	if (out_str[1]) {	/* user typed in a complete string */
	    checkfile(out_str, pm, TRUE, TRUE);
	    return 0;
	}
	sym = out_str[0];
    }

    /* Save the verbose flag, we change it later. */
    save_verbose = flags.verbose;
    flags.verbose = flags.verbose && !quick;
    /*
     * The user typed one letter, or we're identifying from the screen.
     */
    do {
	/* Reset some variables. */
	need_to_look = FALSE;
	pm = (struct permonst *)0;
	skipped_venom = 0;
	found = 0;
	out_str[0] = '\0';

	if (from_screen) {
	    int glyph;	/* glyph at selected position */

	    if (flags.verbose)
#if 0 /*JP*/
		pline("Please move the cursor to %s.",
		       what_is_an_unknown_object);
#else
		pline("カーソルを物体に移動してください。");
#endif
	    else
/*JP
		pline("Pick an object.");
*/
		pline("物体を指定してください。");

	    ans = getpos(&cc, quick, what_is_an_unknown_object);
	    if (ans < 0 || cc.x < 0) {
		flags.verbose = save_verbose;
		return 0;	/* done */
	    }
	    flags.verbose = FALSE;	/* only print long question once */

	    /* Convert the glyph at the selected position to a symbol. */
	    glyph = glyph_at(cc.x,cc.y);
	    if (glyph_is_cmap(glyph)) {
		sym = showsyms[glyph_to_cmap(glyph)];
	    } else if (glyph_is_trap(glyph)) {
		sym = showsyms[trap_to_defsym(glyph_to_trap(glyph))];
	    } else if (glyph_is_object(glyph)) {
		sym = oc_syms[(int)objects[glyph_to_obj(glyph)].oc_class];
		if (sym == '`' && iflags.bouldersym && (int)glyph_to_obj(glyph) == BOULDER)
			sym = iflags.bouldersym;
	    } else if (glyph_is_monster(glyph)) {
		/* takes care of pets, detected, ridden, and regular mons */
		sym = monsyms[(int)mons[glyph_to_mon(glyph)].mlet];
	    } else if (glyph_is_swallow(glyph)) {
		sym = showsyms[glyph_to_swallow(glyph)+S_sw_tl];
	    } else if (glyph_is_invisible(glyph)) {
		sym = DEF_INVISIBLE;
	    } else if (glyph_is_warning(glyph)) {
		sym = glyph_to_warning(glyph);
	    	sym = warnsyms[sym];
	    } else {
		impossible("do_look:  bad glyph %d at (%d,%d)",
						glyph, (int)cc.x, (int)cc.y);
		sym = ' ';
	    }
	}

	/*
	 * Check all the possibilities, saving all explanations in a buffer.
	 * When all have been checked then the string is printed.
	 */

	/* Check for monsters */
	for (i = 0; i < MAXMCLASSES; i++) {
	    if (sym == (from_screen ? monsyms[i] : def_monsyms[i]) &&
		monexplain[i]) {
		need_to_look = TRUE;
		if (!found) {
/*JP
		    Sprintf(out_str, "%c       %s", sym, an(monexplain[i]));
*/
		    Sprintf(out_str, "%c       %s", sym, jtrns_obj('S', monexplain[i]));
#if 0 /*JP*/
		    firstmatch = monexplain[i];
#else
		    firstmatch = jtrns_obj('S',monexplain[i]);
		    firstmatch2 = monexplain[i];
#endif
		    found++;
		} else {
/*JP
		    found += append_str(out_str, an(monexplain[i]));
*/
		    found += append_str(out_str, jtrns_obj('S',monexplain[i]));
		}
	    }
	}
	/* handle '@' as a special case if it refers to you and you're
	   playing a character which isn't normally displayed by that
	   symbol; firstmatch is assumed to already be set for '@' */
	if ((from_screen ?
		(sym == monsyms[S_HUMAN] && cc.x == u.ux && cc.y == u.uy) :
		(sym == def_monsyms[S_HUMAN] && !iflags.showrace)) &&
	    !(Race_if(PM_HUMAN) || Race_if(PM_ELF)) && !Upolyd)
#if 0 /*JP*/
	    found += append_str(out_str, "you");	/* tack on "or you" */
#else
	    found += append_str(out_str, "あなた");	/* tack on "or you" */
#endif

	/*
	 * Special case: if identifying from the screen, and we're swallowed,
	 * and looking at something other than our own symbol, then just say
	 * "the interior of a monster".
	 */
	if (u.uswallow && from_screen && is_swallow_sym(sym)) {
	    if (!found) {
		Sprintf(out_str, "%c       %s", sym, mon_interior);
#if 0 /*JP*/
		firstmatch = mon_interior;
#else
		firstmatch = "怪物の内部";
		firstmatch2 = mon_interior;
#endif
	    } else {
		found += append_str(out_str, mon_interior);
	    }
	    need_to_look = TRUE;
	}

	/* Now check for objects */
	for (i = 1; i < MAXOCLASSES; i++) {
	    if (sym == (from_screen ? oc_syms[i] : def_oc_syms[i])) {
		need_to_look = TRUE;
		if (from_screen && i == VENOM_CLASS) {
		    skipped_venom++;
		    continue;
		}
		if (!found) {
/*JP
		    Sprintf(out_str, "%c       %s", sym, an(objexplain[i]));
*/
		    Sprintf(out_str, "%c       %s", sym, jtrns_obj('S',objexplain[i]));
#if 0 /*JP*/
		    firstmatch = objexplain[i];
#else
		    firstmatch = jtrns_obj('S',objexplain[i]);
		    firstmatch2 = objexplain[i];
#endif
		    found++;
		} else {
/*JP
		    found += append_str(out_str, an(objexplain[i]));
*/
		    found += append_str(out_str, jtrns_obj('S',objexplain[i]));
		}
	    }
	}

	if (sym == DEF_INVISIBLE) {
	    if (!found) {
/*JP
		Sprintf(out_str, "%c       %s", sym, an(invisexplain));
*/
		Sprintf(out_str, "%c       %s", sym, jtrns_obj('S',invisexplain));
#if 0 /*JP*/
		firstmatch = invisexplain;
#else
		firstmatch = jtrns_obj('S', invisexplain);
		firstmatch2 = invisexplain;
#endif
		found++;
	    } else {
/*JP
		found += append_str(out_str, an(invisexplain));
*/
		found += append_str(out_str, jtrns_obj('S', invisexplain));
	    }
	}

#define is_cmap_trap(i) ((i) >= S_arrow_trap && (i) <= S_polymorph_trap)
#define is_cmap_drawbridge(i) ((i) >= S_vodbridge && (i) <= S_hcdbridge)

	/* Now check for graphics symbols */
	for (hit_trap = FALSE, i = 0; i < MAXPCHARS; i++) {
	    x_str = defsyms[i].explanation;
	    if (sym == (from_screen ? showsyms[i] : defsyms[i].sym) && *x_str) {
		/* avoid "an air", "a water", or "a floor of a room" */
#if 0 /*JP*/
		int article = (i == S_room) ? 2 :		/* 2=>"the" */
			      !(strcmp(x_str, "air") == 0 ||	/* 1=>"an"  */
				strcmp(x_str, "water") == 0);	/* 0=>(none)*/
#endif

		if (!found) {
		    if (is_cmap_trap(i)) {
/*JP
			Sprintf(out_str, "%c       a trap", sym);
*/
			Sprintf(out_str, "%c       罠", sym);
			hit_trap = TRUE;
		    } else {
#if 0 /*JP*/
			Sprintf(out_str, "%c       %s", sym,
				article == 2 ? the(x_str) :
				article == 1 ? an(x_str) : x_str);
#else
			Sprintf(out_str, "%c       %s", sym,
				jtrns_obj('S', x_str));
#endif
		    }
#if 0 /*JP*/
		    firstmatch = x_str;
#else
		    firstmatch = jtrns_obj('S', x_str);
		    firstmatch2 = x_str;
#endif
		    found++;
		} else if (!u.uswallow && !(hit_trap && is_cmap_trap(i)) &&
			   !(found >= 3 && is_cmap_drawbridge(i))) {
#if 0 /*JP*/
		    found += append_str(out_str,
					article == 2 ? the(x_str) :
					article == 1 ? an(x_str) : x_str);
#else
		    found += append_str(out_str, jtrns_obj('S', x_str));
#endif
		    if (is_cmap_trap(i)) hit_trap = TRUE;
		}

		if (i == S_altar || is_cmap_trap(i))
		    need_to_look = TRUE;
	    }
	}

	/* Now check for warning symbols */
	for (i = 1; i < WARNCOUNT; i++) {
	    x_str = def_warnsyms[i].explanation;
	    if (sym == (from_screen ? warnsyms[i] : def_warnsyms[i].sym)) {
		if (!found) {
			Sprintf(out_str, "%c       %s",
				sym, def_warnsyms[i].explanation);
			firstmatch = def_warnsyms[i].explanation;
#if 1 /*JP*/ /* 多分不要だが念のため */
			firstmatch2 = firstmatch;
#endif
			found++;
		} else {
			found += append_str(out_str, def_warnsyms[i].explanation);
		}
		/* Kludge: warning trumps boulders on the display.
		   Reveal the boulder too or player can get confused */
		if (from_screen && sobj_at(BOULDER, cc.x, cc.y))
			Strcat(out_str, " co-located with a boulder");
		break;	/* out of for loop*/
	    }
	}
    
	/* if we ignored venom and list turned out to be short, put it back */
	if (skipped_venom && found < 2) {
	    x_str = objexplain[VENOM_CLASS];
	    if (!found) {
#if 0 /*JP*/
		Sprintf(out_str, "%c       %s", sym, an(x_str));
		firstmatch = x_str;
#else
		Sprintf(out_str, "%c       %s", sym, jtrns_obj('S', (x_str)));
		firstmatch = jtrns_obj('S', x_str);
		firstmatch2 = x_str;
#endif
		found++;
	    } else {
/*JP
		found += append_str(out_str, an(x_str));
*/
		found += append_str(out_str, jtrns_obj('S', x_str));
	    }
	}

	/* handle optional boulder symbol as a special case */ 
	if (iflags.bouldersym && sym == iflags.bouldersym) {
	    if (!found) {
#if 0 /*JP*/
		firstmatch = "boulder";
		Sprintf(out_str, "%c       %s", sym, an(firstmatch));
#else
		firstmatch = "岩";
		firstmatch2 = "boulder";
		Sprintf(out_str, "%c       岩", sym);
#endif
		found++;
	    } else {
/*JP
		found += append_str(out_str, "boulder");
*/
		found += append_str(out_str, "岩");
	    }
	}
	
	/*
	 * If we are looking at the screen, follow multiple possibilities or
	 * an ambiguous explanation by something more detailed.
	 */
	if (from_screen) {
	    if (found > 1 || need_to_look) {
		char monbuf[BUFSZ];
		char temp_buf[BUFSZ];

/*JP
		pm = lookat(cc.x, cc.y, look_buf, monbuf);
*/
		pm = lookat(cc.x, cc.y, look_buf, monbuf, look_buf2);
		firstmatch = look_buf;
#if 1 /*JP*/
		firstmatch2 = look_buf2;
#endif
		if (*firstmatch) {
		    Sprintf(temp_buf, " (%s)", firstmatch);
		    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
		    found = 1;	/* we have something to look up */
		}
		if (monbuf[0]) {
/*JP
		    Sprintf(temp_buf, " [seen: %s]", monbuf);
*/
		    Sprintf(temp_buf, "[%s]", monbuf);
		    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
		}
	    }
	}

	/* Finally, print out our explanation. */
	if (found) {
	    pline("%s", out_str);
	    /* check the data file for information about this thing */
#if 0 /*JP*/
	    if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE &&
			(ans == LOOK_VERBOSE || (flags.help && !quick))) {
#else
	    if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE &&
			(ans == LOOK_VERBOSE || (flags.help && !quick)) &&
			firstmatch2) {
#endif
		char temp_buf[BUFSZ];
/*JP
		Strcpy(temp_buf, firstmatch);
*/
		Strcpy(temp_buf, firstmatch2);
		checkfile(temp_buf, pm, FALSE, (boolean)(ans == LOOK_VERBOSE));
	    }
	} else {
/*JP
	    pline("I've never heard of such things.");
*/
	    pline("そんな名前は聞いたことがない。");
	}

    } while (from_screen && !quick && ans != LOOK_ONCE);

    flags.verbose = save_verbose;
    return 0;
}


int
dowhatis()
{
	return do_look(FALSE);
}

int
doquickwhatis()
{
	return do_look(TRUE);
}

int
doidtrap()
{
	register struct trap *trap;
	int x, y, tt;

	if (!getdir("^")) return 0;
	x = u.ux + u.dx;
	y = u.uy + u.dy;
	for (trap = ftrap; trap; trap = trap->ntrap)
	    if (trap->tx == x && trap->ty == y) {
		if (!trap->tseen) break;
		tt = trap->ttyp;
		if (u.dz) {
		    if (u.dz < 0 ? (tt == TRAPDOOR || tt == HOLE) :
			    tt == ROCKTRAP) break;
		}
		tt = what_trap(tt);
#if 0 /*JP*/
		pline("That is %s%s%s.",
		      an(defsyms[trap_to_defsym(tt)].explanation),
		      !trap->madeby_u ? "" : (tt == WEB) ? " woven" :
			  /* trap doors & spiked pits can't be made by
			     player, and should be considered at least
			     as much "set" as "dug" anyway */
			  (tt == HOLE || tt == PIT) ? " dug" : " set",
		      !trap->madeby_u ? "" : " by you");
#else
		pline("それは%s%sだ。",
		      !trap->madeby_u ? "" : (tt == WEB) ? "あなたが張った" :
			  (tt == HOLE || tt == PIT) ? "あなたが掘った" : "あなたが仕掛けた",
		      jtrns_obj('^', defsyms[trap_to_defsym(tt)].explanation));
#endif
		return 0;
	    }
/*JP
	pline("I can't see a trap there.");
*/
	pline("そこには罠はみあたらない。");
	return 0;
}

char *
dowhatdoes_core(q, cbuf)
char q;
char *cbuf;
{
	dlb *fp;
	char bufr[BUFSZ];
	register char *buf = &bufr[6], *ep, ctrl, meta;

	fp = dlb_fopen(CMDHELPFILE, "r");
	if (!fp) {
#if 0 /*JP*/
		pline("Cannot open data file!");
#else
	pline("データファイルを開けない！");
#endif
		return 0;
	}

  	ctrl = ((q <= '\033') ? (q - 1 + 'A') : 0);
	meta = ((0x80 & q) ? (0x7f & q) : 0);
	while(dlb_fgets(buf,BUFSZ-6,fp)) {
	    if ((ctrl && *buf=='^' && *(buf+1)==ctrl) ||
		(meta && *buf=='M' && *(buf+1)=='-' && *(buf+2)==meta) ||
		*buf==q) {
		ep = index(buf, '\n');
		if(ep) *ep = 0;
		if (ctrl && buf[2] == '\t'){
			buf = bufr + 1;
			(void) strncpy(buf, "^?      ", 8);
			buf[1] = ctrl;
		} else if (meta && buf[3] == '\t'){
			buf = bufr + 2;
			(void) strncpy(buf, "M-?     ", 8);
			buf[2] = meta;
		} else if(buf[1] == '\t'){
			buf = bufr;
			buf[0] = q;
			(void) strncpy(buf+1, "       ", 7);
		}
		(void) dlb_fclose(fp);
		Strcpy(cbuf, buf);
		return cbuf;
	    }
	}
	(void) dlb_fclose(fp);
	return (char *)0;
}

int
dowhatdoes()
{
	char bufr[BUFSZ];
	char q, *reslt;

#if defined(UNIX) || defined(VMS)
	introff();
#endif
/*JP
	q = yn_function("What command?", (char *)0, '\0');
*/
	q = yn_function("どういうコマンド？", (char *)0, '\0');
#if defined(UNIX) || defined(VMS)
	intron();
#endif
	reslt = dowhatdoes_core(q, bufr);
	if (reslt)
		pline("%s", reslt);
	else
/*JP
		pline("I've never heard of such commands.");
*/
		pline("そんなコマンドは知らない。");
	return 0;
}

/* data for help_menu() */
static const char *help_menu_items[] = {
#if 0 /*JP*/
/* 0*/	"Long description of the game and commands.",
/* 1*/	"List of game commands.",
/* 2*/	"Concise history of NetHack.",
/* 3*/	"Info on a character in the game display.",
/* 4*/	"Info on what a given key does.",
/* 5*/	"List of game options.",
/* 6*/	"Longer explanation of game options.",
/* 7*/	"List of extended commands.",
/* 8*/	"The NetHack license.",
#ifdef PORT_HELP
	"%s-specific help and commands.",
#define PORT_HELP_ID 100
#define WIZHLP_SLOT 10
#else
#define WIZHLP_SLOT 9
#endif
#ifdef WIZARD
	"List of wizard-mode commands.",
#endif
#else /*JP*/
/* 0*/	"ゲームおよびコマンドの解説。(長文)",
/* 1*/	"コマンド一覧。",
/* 2*/	"NetHackの簡単な歴史。",
/*	"JNetHackの簡単な歴史。",*/
/* 3*/	"画面に表示される文字の説明。",
/* 4*/	"このキーが何を意味するかの説明。",
/* 5*/	"ゲームのオプション一覧。",
/* 6*/	"ゲームのオプション一覧。(長文)",
/* 7*/	"拡張コマンド一覧。",
/* 8*/	"NetHackのライセンス。",
#ifdef PORT_HELP
	"%sに特有のヘルプおよびコマンド。",
#define PORT_HELP_ID 100
#define WIZHLP_SLOT 10
#else
#define WIZHLP_SLOT 9
#endif
#ifdef WIZARD
	"ウィザードモードのコマンド一覧。",
#endif
#endif /*JP*/
	"",
	(char *)0
};

STATIC_OVL boolean
help_menu(sel)
	int *sel;
{
	winid tmpwin = create_nhwindow(NHW_MENU);
#ifdef PORT_HELP
	char helpbuf[QBUFSZ];
#endif
	int i, n;
	menu_item *selected;
	anything any;

	any.a_void = 0;		/* zero all bits */
	start_menu(tmpwin);
#ifdef WIZARD
	if (!wizard) help_menu_items[WIZHLP_SLOT] = "",
		     help_menu_items[WIZHLP_SLOT+1] = (char *)0;
#endif
	for (i = 0; help_menu_items[i]; i++)
#ifdef PORT_HELP
	    /* port-specific line has a %s in it for the PORT_ID */
	    if (help_menu_items[i][0] == '%') {
		Sprintf(helpbuf, help_menu_items[i], PORT_ID);
		any.a_int = PORT_HELP_ID + 1;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 helpbuf, MENU_UNSELECTED);
	    } else
#endif
	    {
		any.a_int = (*help_menu_items[i]) ? i+1 : 0;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0,
			ATR_NONE, help_menu_items[i], MENU_UNSELECTED);
	    }
/*JP
	end_menu(tmpwin, "Select one item:");
*/
	end_menu(tmpwin, "選んでください：");
	n = select_menu(tmpwin, PICK_ONE, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
	    *sel = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    return TRUE;
	}
	return FALSE;
}

int
dohelp()
{
	int sel = 0;

	if (help_menu(&sel)) {
		switch (sel) {
			case  0:  display_file(HELP, TRUE);  break;
			case  1:  display_file(SHELP, TRUE);  break;
			case  2:  (void) dohistory();  break;
			case  3:  (void) dowhatis();  break;
			case  4:  (void) dowhatdoes();  break;
			case  5:  option_help();  break;
			case  6:  display_file(OPTIONFILE, TRUE);  break;
			case  7:  (void) doextlist();  break;
			case  8:  display_file(LICENSE, TRUE);  break;
#ifdef WIZARD
			/* handle slot 9 or 10 */
			default: display_file(DEBUGHELP, TRUE);  break;
#endif
#ifdef PORT_HELP
			case PORT_HELP_ID:  port_help();  break;
#endif
		}
	}
	return 0;
}

int
dohistory()
{
	display_file(HISTORY, TRUE);
	return 0;
}

/*pager.c*/
