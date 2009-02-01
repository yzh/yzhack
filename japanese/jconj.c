/*
**
**	$Id: jconj.c,v 1.1.3.1 1999/11/17 06:35:05 issei Exp issei $
**
*/

/* Copyright (c) Issei Numata 1994-2000 */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define EUC	0
#define SJIS	1
#define JIS	2

/* internal kcode */
/* IC=0 EUC */
/* IC=1 SJIS */
#define IC ((unsigned char)("漢"[0])==0x8a)

#define J_A	0
#define J_KA	(1*5)
#define J_SA	(2*5)
#define J_TA	(3*5)
#define J_NA	(4*5)
#define J_HA	(5*5)
#define J_MA	(6*5)
#define J_YA	(7*5)
#define J_RA	(8*5)
#define J_WA	(9*5)

#define J_GA	(10*5)
#define J_ZA	(11*5)
#define J_DA	(12*5)
#define J_BA	(13*5)
#define J_PA	(14*5)

static unsigned char hira_tab[][2]={
    {0xa4, 0xa2}, {0xa4, 0xa4}, {0xa4, 0xa6}, {0xa4, 0xa8}, {0xa4, 0xaa}, 
    {0xa4, 0xab}, {0xa4, 0xad}, {0xa4, 0xaf}, {0xa4, 0xb1}, {0xa4, 0xb3}, 
    {0xa4, 0xb5}, {0xa4, 0xb7}, {0xa4, 0xb9}, {0xa4, 0xbb}, {0xa4, 0xbd}, 
    {0xa4, 0xbf}, {0xa4, 0xc1}, {0xa4, 0xc4}, {0xa4, 0xc6}, {0xa4, 0xc8}, 
    {0xa4, 0xca}, {0xa4, 0xcb}, {0xa4, 0xcc}, {0xa4, 0xcd}, {0xa4, 0xce}, 
    {0xa4, 0xcf}, {0xa4, 0xd2}, {0xa4, 0xd5}, {0xa4, 0xd8}, {0xa4, 0xdb}, 
    {0xa4, 0xde}, {0xa4, 0xdf}, {0xa4, 0xe0}, {0xa4, 0xe1}, {0xa4, 0xe2}, 
    {0xa4, 0xe4}, {0xa4, 0xa4}, {0xa4, 0xe6}, {0xa4, 0xa8}, {0xa4, 0xe8}, 
    {0xa4, 0xe9}, {0xa4, 0xea}, {0xa4, 0xeb}, {0xa4, 0xec}, {0xa4, 0xed}, 
    {0xa4, 0xef}, {0xa4, 0xa4}, {0xa4, 0xa6}, {0xa4, 0xa8}, {0xa4, 0xaa}, 
    {0xa4, 0xac}, {0xa4, 0xae}, {0xa4, 0xb0}, {0xa4, 0xb2}, {0xa4, 0xb4}, 
    {0xa4, 0xb6}, {0xa4, 0xb8}, {0xa4, 0xba}, {0xa4, 0xbc}, {0xa4, 0xbe}, 
    {0xa4, 0xc0}, {0xa4, 0xc2}, {0xa4, 0xc5}, {0xa4, 0xc7}, {0xa4, 0xc9}, 
    {0xa4, 0xd0}, {0xa4, 0xd3}, {0xa4, 0xd6}, {0xa4, 0xd9}, {0xa4, 0xdc}, 
    {0xa4, 0xd1}, {0xa4, 0xd4}, {0xa4, 0xd7}, {0xa4, 0xda}, {0xa4, 0xdd},
};

#define FIFTH	0 /* 五段 */
#define UPPER	1 /* 上一段 */
#define LOWER	2 /* 下一段 */
#define SAHEN	3 /* サ変 */
#define KAHEN	4 /* カ変 */

#define NORMAL	0 /* あける→〜た */
#define SOKUON	1 /* 刻む→〜んだ */
#define HATSUON	2 /* 打つ→〜った */
#define ION	3 /* 浮く→〜いた */

struct _jconj_tab {
    const char *main;
    int column;
/* 0: fifth conj. 1:upper conj. 2:lower conj. 3:SAHEN 4:KAHEN */
    int katsuyo_type;
/* 0: normal 1: sokuon 2: hatson 3: ion */
    int onbin_type;
} jconj_tab[] = {
/* あ */
    {"あける", J_KA, LOWER, NORMAL},
    {"開ける", J_KA, LOWER, NORMAL},
    {"いれる", J_RA, LOWER, NORMAL},
    {"入れる", J_RA, LOWER, NORMAL},
    {"浮く", J_KA, FIFTH, ION},
    {"打つ", J_TA, FIFTH, HATSUON},
    {"置く", J_KA, FIFTH, ION},
    {"納める", J_MA, LOWER, NORMAL},
    {"落ちる", J_TA, UPPER, NORMAL},
    {"落す", J_SA, FIFTH, NORMAL},
/* か */
    {"書く", J_KA, FIFTH, ION},
    {"かける", J_KA, UPPER, NORMAL},
    {"かぶる", J_RA, FIFTH, HATSUON},
    {"構える", J_A, LOWER, NORMAL},
    {"噛みつく", J_KA, FIFTH, ION},
    {"刻む", J_MA, FIFTH, SOKUON},
    {"着る", J_KA, UPPER, NORMAL},
    {"来る", J_KA, KAHEN, NORMAL}, 
    {"砕く", J_KA, FIFTH, ION}, 
    {"加える", J_A, LOWER, NORMAL},
    {"こする", J_RA, FIFTH, HATSUON},
    {"こます", J_SA, FIFTH, NORMAL},
    {"込む", J_MA, FIFTH, SOKUON},
    {"殺す", J_SA, FIFTH, NORMAL},
    {"壊す", J_SA, FIFTH, NORMAL},
/* さ */
    {"捧げる", J_KA, LOWER, NORMAL},
    {"錆びる", J_BA, UPPER, NORMAL},
    {"死ぬ", J_NA, FIFTH, SOKUON},
    {"滑る", J_RA, FIFTH, HATSUON},
    {"する", J_SA, SAHEN, NORMAL}, 
/* た */
    {"出す", J_SA, FIFTH, NORMAL},
    {"食べる", J_HA, LOWER, NORMAL}, 
    {"使う", J_WA, FIFTH, HATSUON},
    {"つける", J_KA, LOWER, NORMAL},
    {"つまずく", J_KA, FIFTH, ION},
    {"出る", J_NA, LOWER, NORMAL},
    {"解く", J_KA, FIFTH, ION},
    {"溶ける", J_KA, LOWER, NORMAL},
    {"とばす", J_SA, FIFTH, NORMAL},
    {"飛ぶ", J_BA, FIFTH, SOKUON},
    {"取る", J_RA, FIFTH, HATSUON},
/* な */
    {"投げる", J_GA, LOWER, NORMAL},
    {"名づける", J_KA, LOWER, NORMAL},
    {"握る", J_RA, FIFTH, HATSUON},
    {"脱ぐ", J_GA, FIFTH, ION},
    {"濡らす", J_SA, FIFTH, NORMAL},
    {"塗る", J_RA, FIFTH, HATSUON},
    {"飲む", J_MA, FIFTH, SOKUON},
/* は */
    {"はいずる", J_RA, FIFTH, HATSUON},
    {"履く", J_KA, FIFTH, ION},
    {"はさむ", J_MA, FIFTH, SOKUON},
    {"はずす", J_SA, FIFTH, NORMAL},
    {"外す", J_SA, FIFTH, NORMAL},
    {"はめる", J_MA, UPPER, NORMAL},
    {"浸す", J_SA, FIFTH, NORMAL},
    {"ひっかける", J_KA, LOWER, NORMAL},
    {"拾う", J_WA, FIFTH, HATSUON},
    {"踏む", J_MA, FIFTH, SOKUON},
    {"振りかざす", J_SA, FIFTH, NORMAL},
    {"掘る", J_RA, FIFTH, HATSUON},
/* ま */
    {"巻く", J_KA, FIFTH, ION},
    {"守る", J_RA, FIFTH, HATSUON},
    {"回す", J_SA, FIFTH, NORMAL},
    {"身につける", J_KA, LOWER, NORMAL},
    {"持つ", J_TA, FIFTH, HATSUON},
/* や */
    {"焼く", J_KA, FIFTH, ION},
    {"呼ぶ", J_BA, FIFTH, SOKUON},
    {"読む", J_MA, FIFTH, SOKUON},
/* ら */
/* わ */
    {(void*)0, 0, 0, 0},
};

extern unsigned char *e2sj(unsigned char *s);
extern unsigned char *sj2e(unsigned char *s);

/*
**	conjection verb word
**
**	Example
**	arg1	arg2	result
**	脱ぐ	ない	脱がない
**	脱ぐ	た	脱いだ
**
*/
static char *
jconjsub( tab, jverb, sfx )
     struct _jconj_tab *tab;
     char *jverb;
     char *sfx;
{
    int len;
    unsigned char *p;
    static unsigned char tmp[1024];

    len = strlen(jverb);
    strcpy((char *)tmp, jverb );

    if(!strncmp(sfx, "と", 2)){
	strcat((char *)tmp, sfx);
	return (char *)tmp;
    }

    switch( tab->katsuyo_type ){
      case FIFTH:
	p = tmp + (len - 2);
	if(!strncmp(sfx, "な", 2)){
	    if(!IC){
		p[0] = 0xa4;
		p[1] = hira_tab[tab->column][1];
	    } else {
	      memcpy(p, e2sj(hira_tab[tab->column]), 2);
	    }

	    strcpy((char *)p + 2, sfx);
	    break;
	}
	else if(!strncmp(sfx, "た", 2) || !strncmp(sfx, "て", 2)){
	    switch( tab->onbin_type ){
	      case NORMAL:
		if(!IC){
		    p[1] = hira_tab[tab->column + 1][1];
		} else {
		    memcpy(p, e2sj(hira_tab[tab->column + 1]), 2);
		}
		break;
	      case SOKUON:
		if(!IC){
		    p[1] = 0xf3;
		} else {
		    memcpy(p, "ん", 2);
		}
		break;
	      case HATSUON:
		if(!IC){
		    p[1] = 0xc3;
		} else {
		    memcpy(p, "っ", 2);
		}
		break;
	      case ION:
		if(!IC){
		    p[1] = 0xa4;
		} else {
		    memcpy(p, "い", 2);
		}
		break;
	    }
	    strcpy((char *)p + 2, sfx);
	    if(tab->onbin_type == SOKUON ||
	       (tab->onbin_type == ION && tab->column >= J_GA)){
		if(!IC){
		  ++p[3];
		} else {
		  ++p[3];
		}
/*	  memcpy(p+2, e2sj(sj2e(p+2)+1), 2);*//* sj2e() returns ptr to char* */
	    }
	    break;
	}
	else if(!strncmp(sfx, "ば", 2)){
	    if(!IC){
		p[1] = hira_tab[tab->column + 3][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 3]), 2);
	    }
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "れ", 2)){
	    if(!IC){
		p[1]=hira_tab[tab->column + 3][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 3]), 2);
	    }
	    strcpy((char *)p + 2, sfx + 2);
	}
	else if(!strncmp(sfx, "ま", 2)) {
	    if(!IC){
		p[1] = hira_tab[tab->column + 1][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 1]), 2);
	    }
	    strcpy((char *)p + 2, sfx);
	    break;
	}
	else if(!strncmp(sfx, "よ", 2)) {
	    if(!IC){
		p[1] = hira_tab[tab->column + 4][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 4]), 2);
	    }
	    strcpy((char *)p + 2, sfx + 2);
	    break;
	}
	break;
      case LOWER:
      case UPPER:
      case KAHEN:
	p = tmp + (len - 2);
	if(!strncmp(sfx, "ば", 2)){
	    strcpy((char *)p, "れ");
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "れ", 2) && tab->katsuyo_type == LOWER){
	    strcpy((char *)p, "ら");
	    strcpy((char *)p + 2, sfx);
	}
	else
	  strcpy((char *)p, sfx);
	break;
      case SAHEN:
	p = tmp + (len - 4);
	if(!strncmp(sfx, "な", 2) ||
	   !strncmp(sfx, "ま", 2) ||
	   !strncmp(sfx, "た", 2) ||
	   !strncmp(sfx, "て", 2) ||
	   !strncmp(sfx, "よ", 2)){
	    strcpy((char *)p, "し");
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "ば", 2) || !strncmp(sfx, "れば", 4)){
	    strcpy((char *)p, "すれば");
	}
	break;
    }
    return (char *)tmp;
}

/* 動詞の変化 */
const char *
jconj( jverb, sfx )
     const char *jverb;
     const char *sfx;
{
    struct _jconj_tab *tab;
    int len;

    len = strlen(jverb);
    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
	if(!strcmp(jverb, tab->main)){
	    return jconjsub(tab, jverb, sfx);
	}
    }

    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
	if(len - strlen(tab->main) > 0 &&
	   !strcmp(jverb + (len - strlen(tab->main)), tab->main)){
	    return jconjsub(tab, jverb, sfx);
	}
    }

#ifdef JAPANESETEST
    fprintf( stderr, "I don't know such word \"%s\"\n");
#endif
    return jverb;
}

/* 可能 */
const char *
jcan(jverb)
     const char *jverb;
{
    const char *ret;
    static unsigned char tmp[1024];

    int len = strlen(jverb);
    if(!strcmp(jverb + len - 4, "する")){
	strncpy(tmp, jverb, len - 4);
	strcpy(tmp + len - 4, "できる");
	return tmp;
    } else {
	ret = jconj(jverb, "れる");
	return ret;
    }
}

/* 不可能 */
const char *
jcannot(jverb)
     const char *jverb;
{
    static unsigned char tmp[1024];

    int len = strlen(jverb);
    if(!strcmp(jverb + len - 4, "する")){
	strncpy(tmp, jverb, len-4);
	strcpy(tmp +len-4, "できない");
	return tmp;
    } else {
	return jconj(jverb, "れない");
    }
}

/* 過去 */
const char *
jpast(jverb)
     const char *jverb;
{
    return jconj(jverb, "た");
}

/* 敬体 */
const char *
jpolite(jverb)
     const char *jverb;
{
    return jconj(jverb, "ます");
}


/*
**	conjection of adjective word
**
**	Example:
**
**	形容詞的用法	   副詞的用法
**
**	赤い		-> 赤く		(形容詞)
**	綺麗な		-> 綺麗に	(形容動詞)
**	綺麗だ		-> 綺麗に	(形容動詞)
*/
const char *
jconj_adj( jadj )
     const char *jadj;
{
    int len;
    static unsigned char tmp[1024];

    strcpy((char *)tmp, jadj);
    len = strlen((char *)tmp);

    if(!strcmp((char *)tmp + len - 2, "い")){
	strcpy((char *)tmp + len - 2, "く");
    } else if(!strcmp((char *)tmp + len - 2, "だ") ||
	      !strcmp((char *)tmp + len - 2, "な") ||
	      !strcmp((char *)tmp + len - 2, "の")){
	strcpy((char *)tmp + len - 2, "に");
    }

    return (char *)tmp;
}


#ifdef JAPANESETEST
unsigned char
*e2sj(unsigned char *s)
{
    return *s;
}

unsigned char
*sj2e(unsigned char *s)
{
    return *s;
}

void
main()
{
    struct _jconj_tab *tab;

    for(tab = jconj_tab; tab->main != (void*)0; ++tab){
	printf("%-10s ない %s\n", tab->main, jconj(tab->main, "ない"));
	printf("%-10s ます %s\n", tab->main, jconj(tab->main, "ます"));
	printf("%-10s た   %s\n", tab->main, jconj(tab->main, "た"));
	printf("%-10s れば %s\n", tab->main, jconj(tab->main, "れば"));
	printf("%-10s とき %s\n", tab->main, jconj(tab->main, "とき"));
	printf("%-10s よう %s\n", tab->main, jconj(tab->main, "よう"));
	printf("%-10s %s\n", tab->main, jcan(tab->main));
	printf("%-10s %s\n", tab->main, jcannot(tab->main));
    }
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "ない"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "ます"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "た"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "れば"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "とき"));
}
#endif
