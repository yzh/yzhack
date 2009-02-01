/*
**
**	$Id: jlib.c,v 1.1.3.1 1999/11/17 06:35:05 issei Exp issei $
**
*/

/* Copyright (c) Issei Numata 1994-2000 */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#include "hack.h"

int xputc(CHAR_P);
int xputc2(int, int);


#define EUC	0
#define SJIS	1
#define JIS	2

/* internal kcode */
/* IC=0 EUC */
/* IC=1 SJIS */
#define IC ((unsigned char)("漢"[0])==0x8a)

/* default input kcode */
#ifndef INPUT_KCODE
# ifdef MSDOS
#  define INPUT_KCODE SJIS
# else
#  define INPUT_KCODE EUC
# endif
#endif

/* default output kcode */
#ifndef OUTPUT_KCODE
# ifdef MSDOS
#  define OUTPUT_KCODE SJIS
# else
#  define OUTPUT_KCODE EUC
# endif
#endif

#ifdef WIN32_PLATFORM_PSPC 
#define putchar(x) {}
#endif

static int	output_kcode = OUTPUT_KCODE;
static int	input_kcode = INPUT_KCODE;

/*
**	Kanji code library....
*/

/*
 * 引数が漢字(の1文字目)かどうかを調べる
 */
int
is_kanji(c)
     unsigned c;
{
    if(IC == EUC)
      return (c & 0x80);
    else
      return ((unsigned int)c>=0x81 && (unsigned int)c<=0x9f)
	|| ((unsigned int)c>=0xe0 && (unsigned int)c<=0xfc);
}

void
setkcode(c)
     int c;
{
    if(c == 'E' || c == 'e' )
      output_kcode = EUC;
    else if(c == 'J' || c == 'j')
      output_kcode = JIS;
    else if(c == 'S' || c == 's')
      output_kcode = SJIS;
    else if(c == 'I' || c == 'i')
#ifdef MSDOS
      output_kcode = SJIS;
#else
      output_kcode = IC;
#endif
    else{
	output_kcode = IC;
    }
    input_kcode = output_kcode;
}
/*
**	EUC->SJIS
*/

unsigned char *
e2sj(s)
     unsigned char *s;
{
    unsigned char h,l;
    static unsigned char sw[2];

    h = s[0] & 0x7f;
    l = s[1] & 0x7f;

    sw[0] = ((h - 1) >> 1)+ ((h <= 0x5e) ? 0x71 : 0xb1);
    sw[1] = l + ((h & 1) ? ((l < 0x60) ? 0x1f : 0x20) : 0x7e);

    return sw;
}
/*
**	SJIS->EUC
*/
unsigned char *
sj2e(s)
     unsigned char *s;
{
    unsigned int h,l;
    static unsigned char sw[2];

    h = s[0];
    l = s[1];

    h = h + h - ((h <=0x9f) ? 0x00e1 : 0x0161);
    if( l<0x9f )
      l = l - ((l > 0x7f) ? 0x20 : 0x1f);
    else{
	l = l-0x7e;
	++h;
    }
    sw[0] = h | 0x80;
    sw[1] = l | 0x80;
    return sw;
}
/*
**	translate string to internal kcode
*/
const char *
str2ic(s)
     const char *s;
{
    static unsigned char buf[1024];
    const unsigned char *up;
    unsigned char *p, *pp;
    int kin;

    if(!s)
      return s;

    buf[0] = '\0';

    if( IC==input_kcode ){
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    p = buf;
    if( IC==EUC && input_kcode == SJIS ){
	while(*s){
	    up = s;
	    if(is_kanji(*up)){
		pp = sj2e((unsigned char *)s);
		*(p++) = pp[0];
		*(p++) = pp[1];
		s += 2;
	    }
	    else
	      *(p++) = (unsigned char)*(s++);
	}
    }
    else if( IC==EUC && input_kcode == JIS ){
	kin = 0;
	while(*s){
	    if(s[0] == 033 && s[1] == '$' && (s[2] == 'B' || s[3] == '@')){
		kin = 1;
		s += 3;
	    }
	    else if(s[0] == 033 && s[1] == '(' && (s[2] == 'B' || s[3] == 'J')){
		kin = 0;
		s += 3;
	    }
	    else if( kin )
	      *(p++) = (*(s++) | 0x80);
	    else
	      *(p++) = *(s++);
	}
    }
    else{
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    *(p++) = '\0';
    return (char *)buf;
}

#ifdef SJIS_FILESYSTEM
/*
**	translate string to output kcode
*/
const char *
ic2str(s)
     const char *s;
{
    static unsigned char buf[1024];
    const unsigned char *up;
    unsigned char *p, *pp;
    int kin;

    if(!s)
      return s;

    buf[0] = '\0';

    p = buf;
    if( IC==EUC && output_kcode == SJIS ){
	while(*s){
	    up = s;
	    if( *up & 0x80 ){
		pp = e2sj((unsigned char *)s);
		*(p++) = pp[0];
		*(p++) = pp[1];
		s += 2;
	    }
	    else
	      *(p++) = (unsigned char)*(s++);
	}
    }
    else{
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    *(p++) = '\0';
    return (char *)buf;
}
#endif /* MSDOS */

/*
**	primitive function
*/

static int kmode;	/* 0: Kanji out */
			/* 1: Kanji in */

static void
tty_reset()
{
    if(kmode && output_kcode==JIS ){
	putchar(033);
	putchar('(');
	putchar('B');
/*
    if (flags.DECgraphics){
      putchar(033);
      putchar('$');
      putchar(')');
      putchar('B');
    }
*/
    }
    kmode = 0;
}

/* print out 1 byte character to tty (no conversion) */
static void
tty_cputc(unsigned int c)
{
    if(kmode && output_kcode==JIS ){
	putchar(033);
	putchar('(');
	putchar('B');
    }
    kmode = 0;

#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc(c);
#else
    putchar(c);
#endif
} 

/* print out 2 bytes character to tty (no conversion) */
static void
tty_cputc2(unsigned int c, unsigned int c2)
{
    kmode = 1;

#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc2(c, c2);
#else
    putchar(c);
    putchar(c2);
#endif
} 

/* print out 1 byte character to tty (IC->output_kcode) */
static void
tty_jputc(unsigned int c)
{
    if(kmode && output_kcode==JIS ){
	putchar(033);
	putchar('(');
	putchar('B');
    }
    kmode = 0;

#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc(c);
#else
    putchar(c);
#endif
}

/* print out 2 bytes character to tty (IC->output_kcode) */
static void
tty_jputc2(unsigned int c, unsigned int c2)
{
    if(!kmode && output_kcode==JIS ){
	putchar(033);
	putchar('$');
	putchar('B');
    }
    kmode = 1;

#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc2(c, c2);
#else
    putchar(c);
    putchar(c2);
#endif
}

/*
 *  2バイト文字をバッファリングしながら出力する
 *  漢字コード変換も行う
 */
int
jbuffer(
     unsigned int c,
     unsigned int *buf,
     void (*reset)(),
     void (*f1)(unsigned int),
     void (*f2)(unsigned int, unsigned int))
{
    static unsigned int ibuf[2];
    unsigned int c1, c2;
    unsigned char uc[2];
    unsigned char *p;

    if(!buf) buf = ibuf;
    if(!reset) reset = tty_reset;
    if(!f1) f1 = tty_jputc;
    if(!f2) f2 = tty_jputc2;

    c = c & 0xff;

    if(!(buf[0]) && (is_kanji(c))){
	buf[1] = c;
	++buf[0];
	return 0;
    }
    else if(buf[0]){
	c1 = buf[1];
	c2 = c;

	if(IC == output_kcode)
	  ;
	else if(IC == EUC){
	    switch(output_kcode){
	      case JIS:
		c1 &= 0x7f;
		c2 &= 0x7f;
		break;
	      case SJIS:
		uc[0] = c1;
		uc[1] = c2;
		p = e2sj(uc);
		c1 = p[0];
		c2 = p[1];
		break;
	      default:
		impossible("Unknown kcode!");
		break;
	    }
	}
	else if(IC == SJIS){
	    uc[0] = c1;
	    uc[1] = c2;
	    p = sj2e(uc);
	    switch(output_kcode){
	      case JIS:
		c1 &= 0x7f;
		c2 &= 0x7f;
		break;
	      case EUC:
		break;
	      default:
		impossible("Unknown kcode!");
		break;
	    }
	}
	f2(c1, c2);
	buf[0] = 0;
	return 2;
    }
    else if(c){
	f1(c);
	return 1;
    }
    reset();
    return -1;
}

/*
 *  2バイト文字をバッファリングしながら出力する
 *  漢字コード変換は行わない
 */
int
cbuffer(
     unsigned int c,
     unsigned int *buf,
     void (*reset)(),
     void (*f1)(unsigned int),
     void (*f2)(unsigned int, unsigned int))
{
    static unsigned int ibuf[2];

    if(!buf) buf = ibuf;
    if(!reset) reset = tty_reset;
    if(!f1) f1 = tty_cputc;
    if(!f2) f2 = tty_cputc2;

    if(!(buf[0]) && is_kanji(c)){
	buf[1] = c;
	++buf[0];
	return 0;
    }
    else if(buf[0]){
	f2(buf[1], c);
	buf[0] = 0;
	return 2;
    }
    else if(c){
	f1(c);
	return 1;
    }
    reset();
    return -1;
}

void
jputchar(int c)
{
    static unsigned int buf[2];
    jbuffer((unsigned int)(c & 0xff), buf, NULL, NULL, NULL);
}
void
cputchar(int c)
{
    static unsigned int buf[2];
    cbuffer((unsigned int)(c & 0xff), buf, NULL, NULL, NULL);
}

void
jputs(s)
     const char *s;
{
    while(*s)
      jputchar((unsigned char)*s++);
    jputchar('\n');
}

int
is_kanji2(s, pos)
     const char *s;
     int pos;
{
    unsigned char *str;

    str = (unsigned char *)s;
    while(*str && pos>0){
	if(is_kanji(*str)){
	    str+=2;
	    pos-=2;
	}
	else{
	    ++str;
	    --pos;
	}
    }
    if(pos<0)
      return 1;
    else
      return 0;
}

int
is_kanji1(s, pos)
     const char *s;
     int pos;
{
    unsigned char *str;

    str = (unsigned char *)s;
    while(*str && pos>0){
	if(is_kanji(*str)){
	    str+=2;
	    pos-=2;
	}
	else{
	    ++str;
	    --pos;
	}
    }
    if(!pos && is_kanji(*str))
      return 1;
    else
      return 0;
}

/*
** 8ビットスルーなisspace()
*/
int
isspace_8(c)
     int c;
{
    unsigned int *up;

    up = (unsigned int *)&c;
    return *up<0x80 ? isspace(*up) : 0;
}
/*
** split string(str) including japanese before pos and return to
** str1, str2.
*/
void
split_japanese( str, str1, str2, pos )
     char *str;
     char *str1;
     char *str2;
     int pos;
{
    int len, i, j, k, mlen;
    char *pstr;
    char *pnstr;
    int	sq_brac;

retry:
    len = strlen((char *)str);

    if( len < pos ){
	strcpy(str1,str);
	*str2 = '\0';
	return;
    }

    if(pos > 20)
      mlen = 20;
    else
      mlen = pos;

    i = pos;
    if(is_kanji2(str, i))
      --i;

/* 1:
** search space character
*/
    j = 0;
    while( j<mlen ){
	if(isspace_8(str[i-j])){
	    --j;
	    goto found;
	}
	else if(is_kanji1(str,i-j)){
	    if(!strncmp(str+i-j,"　",2)){
		j -= 2;
		goto found;
	    }
	}
	++j;
    }

/* 3:
** search second bytes of japanese
*/
    j = 0;
    sq_brac = 0;
    while( j<mlen ){
	if(str[i-j] == '['){
	    sq_brac = 0;
	    goto found;
	}
	else if(str[i-j] == ']')
	  sq_brac = 1;
	
	if(!sq_brac && is_kanji1(str,i-j)){
	    goto found;
	}

	++j;
    }
    if(pos > 2){
	pos -= 2;
	goto retry;
    }
found:

    if(pos > 2){
	if((str[i-j] == ']' ||
	    str[i-j] == ')' ||
	    str[i-j] == '}')){
	    pos -= 2;
	    goto retry;
	}
	else if(!strncmp(str+i-j, "］", 2) ||
		!strncmp(str+i-j, "）", 2) ||
		!strncmp(str+i-j, "｝", 2)){
	    pos -= 2;
	    goto retry;
	}
	else if(!strncmp(str+i-j,"？",2) ||
		!strncmp(str+i-j,"、",2) ||
		!strncmp(str+i-j,"。",2) ||
		!strncmp(str+i-j,"，",2) ||
		!strncmp(str+i-j,"．",2)){
	    pos -= 2;
	    goto retry;
	}
    }

    pstr = str;

    pnstr = str1;
    for( k=0 ; k<i-j ; ++k )
      *(pnstr++) = *(pstr++);
    *(pnstr++) = '\0';

    pnstr = str2;
    for( ; str[k] ; ++k )
      *(pnstr++) = *(pstr++);
    *(pnstr++) = '\0';
}

void 
jrndm_replace(c)
     char *c;
{
    unsigned char cc[3];

    if(IC==SJIS)
      memcpy(cc, (char *)sj2e(c), 2);
    else
      memcpy(cc, c, 2);

    cc[0] &= 0x7f;
    cc[1] &= 0x7f;

    switch(cc[0]){
      case 0x21:
	cc[1] = rn2(94) + 0x21;
	break;
      case 0x23:
	if(cc[1] <= 0x39) /* ０〜９ */
	  cc[1] = rn2(10) + 0x30;
	else if(cc[1] <= 0x5A) /* Ａ〜Ｚ */
	  cc[1] = rn2(26) + 0x41;
	else if(cc[2] <= 0x7A) /* ａ〜ｚ */
	  cc[1] = rn2(26) + 0x61;
	break;
      case 0x24:
      case 0x25:
	cc[1] = rn2(83) + 0x21; /* あ〜ん or ア〜ン */
	break;
      case 0x26:
	if(cc[1] <= 0x30)
	  cc[1] = rn2(24) + 0x21; /* Α〜Ω ギリシャ文字 */
	else
	  cc[1] = rn2(24) + 0x41; /* α〜ω ギリシャ文字 */
	break;
      case 0x27:
	if(cc[1] <= 0x40)
	  cc[1] = rn2(33) + 0x21; /* А〜Я ロシア文字 */
	else
	  cc[1] = rn2(33) + 0x51; /* а〜я ロシア文字 */
	break;
      case 0x4f:
	cc[1] = rn2(51) + 0x21; /* 蓮〜 腕 */
	break;
      case 0x74:
	cc[1] = rn2(4) + 0x21; /* 堯 槇 遙 瑤 の4文字*/
	break;
      default:
	if(cc[0] >= 0x30 && cc[1] <= 0x74)
	  cc[1] = rn2(94) + 0x21;
	break;
    }

    cc[0] |= 0x80;
    cc[1] |= 0x80;

    if(IC==SJIS)
      memcpy(c, (char *)e2sj(cc), 2);
    else
      memcpy(c, cc, 2);
}

/*
 * "put off"を対象によって適切に和訳する
 */
const char *
joffmsg(otmp, joshi)
register struct obj *otmp;
const char **joshi;
{
    static char buf[BUFSZ];

    *joshi = "を";

    if(otmp->oclass == RING_CLASS){
	Sprintf(buf, "%sからはずす", body_part(FINGER));
	return buf;
    }
    if( otmp->oclass == AMULET_CLASS){
	return "はずす";
    }
    else if(is_helmet(otmp))
      return "取る";
    else if(is_gloves(otmp))
      return "はずす";
    else if(otmp->oclass == WEAPON_CLASS||is_shield(otmp)){
	*joshi = "の";
	return "装備を解く";
    }
    else if(is_suit(otmp))
      return "脱ぐ";
    else
      return "はずす";
}

/*
 * "put on"を対象によって適切に和訳する
 */
const char *
jonmsg(otmp, joshi)
register struct obj *otmp;
const char **joshi;
{
    static char buf[BUFSZ];

    *joshi = "を";

    if(otmp->oclass == RING_CLASS){
	Sprintf(buf, "%sにはめる", body_part(FINGER));
	return buf;
    }
    else if(otmp->oclass == AMULET_CLASS)
      return "身につける";
    else if(is_gloves(otmp))
      return "身につける";
    else if(is_shield(otmp)){
	*joshi = "で";
	return "身を守る";
    }
    else if(is_helmet(otmp))
      return "かぶる";
    else if(otmp->oclass == WEAPON_CLASS){
	Sprintf(buf, "%sにする", body_part(HAND));
	return buf;
    }
    else if(is_boots(otmp))
      return "履く";
    else if(is_suit(otmp))
      return "着る";
    else
      return "身につける";
}

/*
 * 数詞を返す
 */
const char *
numeral(obj)
register struct obj *obj;
{
    switch(obj->oclass){
      case WEAPON_CLASS:
      case WAND_CLASS:
	if(obj->otyp==SHURIKEN)
	  return "枚";
	else
	  return "本";
      case ARMOR_CLASS:
	return "着";
      case GEM_CLASS:
      case ROCK_CLASS:
      case BALL_CLASS:
	return "個";
      case SCROLL_CLASS:
	return "枚";
      case SPBOOK_CLASS:
	return "冊";
      case POTION_CLASS:
      case RING_CLASS:
      case AMULET_CLASS:
      case FOOD_CLASS:
	if(obj->quan < 10L)
	  return "つ";
	else
	  return "";
      case COIN_CLASS:
	return "枚";
      default:
	switch(obj->otyp){
	  case CREDIT_CARD:
	  case TOWEL:
	  case BLINDFOLD:
	    return "枚";
	  case SKELETON_KEY:
	  case TALLOW_CANDLE:
	  case WAX_CANDLE:
	  case PICK_AXE:
	  case UNICORN_HORN:
	  case LEASH:
	  case STETHOSCOPE:
	  case MAGIC_MARKER:
	    return "本";
	  case CRYSTAL_BALL:
	    return "個";
	    break;
	  default:
	    if(obj->quan < 10L)
	      return "つ";
	    else
	      return "";
	}
    }
}
