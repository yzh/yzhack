/*
**
**	$Id$
**
*/

/* Copyright (c) Issei Numata 1994-2000 */
/* For 3.4, 2003-2004 by Kentaro Shirakata */
/* JNetHack may be freely redistributed.  See license for details. */

#define HASHSIZE 2047

static int 
hash_val(key)
     unsigned char *key;
{
    int v=0;

    ++key;/* skip first char */
    while(*key)
      v = (v * 3 + *(key++)) % HASHSIZE;
    return v;
}
