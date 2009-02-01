/*	SCCS Id: @(#)rnd.c	3.4	1996/02/07	*/
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifndef RAND_MT

/* "Rand()"s definition is determined by [OS]conf.h */
#if defined(LINT) && defined(UNIX)	/* rand() is long... */
extern int NDECL(rand);
#define RND(x)	(rand() % x)
#else /* LINT */
# if defined(UNIX) || defined(RANDOM)
#define RND(x)	(int)(Rand() % (long)(x))
# else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x)	(int)((Rand()>>3) % (x))
# endif
#endif /* LINT */

#else /* RAND_MT */

STATIC_DCL void NDECL(next_state);
STATIC_DCL unsigned long NDECL(genrand_int32);
/*#define RND(x) (int)(genrand_int32() % (unsigned long)(x))*/
#define RND(x) (int)((double)(x) * (double)genrand_int32() * (1.0/4294967296.0))

#endif /* RAND_MT */

#ifdef OVL0

int
rn2(x)		/* 0 <= rn2(x) < x */
register int x;
{
#ifdef DEBUG
	if (x <= 0) {
		impossible("rn2(%d) attempted", x);
		return(0);
	}
	x = RND(x);
	return(x);
#else
	return(RND(x));
#endif
}

#endif /* OVL0 */
#ifdef OVLB

int
rnl(x)		/* 0 <= rnl(x) < x; sometimes subtracting Luck */
register int x;	/* good luck approaches 0, bad luck approaches (x-1) */
{
	register int i;

#ifdef DEBUG
	if (x <= 0) {
		impossible("rnl(%d) attempted", x);
		return(0);
	}
#endif
	i = RND(x);

	if (Luck && rn2(50 - Luck)) {
	    i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
	    if (i < 0) i = 0;
	    else if (i >= x) i = x-1;
	}

	return i;
}

#endif /* OVLB */
#ifdef OVL0

int
rnd(x)		/* 1 <= rnd(x) <= x */
register int x;
{
#ifdef DEBUG
	if (x <= 0) {
		impossible("rnd(%d) attempted", x);
		return(1);
	}
	x = RND(x)+1;
	return(x);
#else
	return(RND(x)+1);
#endif
}

#endif /* OVL0 */
#ifdef OVL1

int
d(n,x)		/* n <= d(n,x) <= (n*x) */
register int n, x;
{
	register int tmp = n;

#ifdef DEBUG
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		impossible("d(%d,%d) attempted", n, x);
		return(1);
	}
#endif
	while(n--) tmp += RND(x);
	return(tmp); /* Alea iacta est. -- J.C. */
}

#endif /* OVL1 */
#ifdef OVLB

int
rne(x)
register int x;
{
	register int tmp, utmp;

	utmp = (u.ulevel < 15) ? 5 : u.ulevel/3;
	tmp = 1;
	while (tmp < utmp && !rn2(x))
		tmp++;
	return tmp;

	/* was:
	 *	tmp = 1;
	 *	while(!rn2(x)) tmp++;
	 *	return(min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3));
	 * which is clearer but less efficient and stands a vanishingly
	 * small chance of overflowing tmp
	 */
}

int
rnz(i)
int i;
{
#ifdef LINT
	int x = i;
	int tmp = 1000;
#else
	register long x = i;
	register long tmp = 1000;
#endif
	tmp += rn2(1000);
	tmp *= rne(4);
	if (rn2(2)) { x *= tmp; x /= 1000; }
	else { x *= 1000; x /= tmp; }
	return((int)x);
}

#endif /* OVLB */

#ifdef RAND_MT

/* 
   A C-program for MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.

   Before using, initialize the state by using init_genrand(seed) 
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* Period parameters */  
#define N_MT 624
#define M_MT 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

static unsigned long state[N_MT]; /* the array for the state vector  */
static int left = 1;
/*static int initf = 0;*/
static unsigned long *next;

/* initializes state[N] with a seed */
void init_genrand(unsigned long s)
{
    int j;
    state[0]= s & 0xffffffffUL;
    for (j=1; j<N_MT; j++) {
        state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array state[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        state[j] &= 0xffffffffUL;  /* for >32 bit machines */
    }
    left = 1; /*initf = 1;*/
}

static void next_state(void)
{
    unsigned long *p=state;
    int j;

    /* if init_genrand() has not been called, */
    /* a default initial seed is used         */
/*    if (initf==0) init_genrand(5489UL);*/

    left = N_MT;
    next = state;
    
    for (j=N_MT-M_MT+1; --j; p++) 
        *p = p[M_MT] ^ TWIST(p[0], p[1]);

    for (j=M_MT; --j; p++) 
        *p = p[M_MT-N_MT] ^ TWIST(p[0], p[1]);

    *p = p[M_MT-N_MT] ^ TWIST(p[0], state[0]);
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;

    if (--left == 0) next_state();
    y = *next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

#endif	// RAND_MT
/*rnd.c*/
