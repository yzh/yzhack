
/* EMS handler routine
   Copyright (c) Kouji Takada 1994-2000 */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <dos.h>
#include <stdlib.h>
#include "emalloc.h"

#define	EMS_VECT	0x67
#define	EMS_STR		"EMMXXXX0"
static	int isems = 0;
static	unsigned int ems_frame = 0;
static	unsigned long emalloc_ptr = 0;
static	unsigned int ems_handle = 0;

#ifdef MOVERLAY
/* MOVERLAY API */
#define	MOVE_PAUSE_CACHE	4
extern	unsigned short _movefpause;
extern	void _movepause(void);
extern	void _moveresume(void);
#endif

/* 正規化されたポインタを作成する */
static	void *mk_fp(unsigned int seg, unsigned int off)
{
	unsigned long temp;

	temp = (((unsigned long)seg) << 4) + ((unsigned long)off);
	return (void *)(((temp & 0xffff0L) << 12) + (temp & 0x0000fL));
}

/* プログラム終了時に呼ばれ、EMS を開放する */
void	done_ems(void)
{
	union REGS inregs, outregs;
	struct SREGS segregs;
	int i;

	if (isems == 0) return;
	inregs.h.ah = 0x44;
	inregs.x.dx = ems_handle;
	inregs.x.bx = 0xffff;
	for (i = 0; i < 4; i++)
	{
		/* ページをアンマップする */
		inregs.h.al = i;
		int86(EMS_VECT, &inregs, &outregs);
		if (outregs.h.ah != 0) fprintf(stderr,
			"警告: EMS をアンマップ出来ません (%04x)。\n",
			outregs.h.ah & (i << 8));
	}

	/* ページを開放する */
	inregs.h.ah = 0x45;
	inregs.x.dx = ems_handle;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "警告: EMS を開放出来ません (%04x)。\n",
			outregs.h.ah);
		return;
	}
/*	fprintf(stderr, "EMS は %ud バイト使用されました。\n", emalloc_ptr); */
	isems = 0;
}

/* EMS を初期化する */
int	detect_ems(void)
{
	union REGS inregs, outregs;
	char *vect;
	int i;

	/* EMS ドライバを探す */
	vect = (char *)_dos_getvect(EMS_VECT);
	vect = mk_fp(FP_SEG(vect), 0x000a);
	if (memcmp(vect, EMS_STR, 8) != 0)
	{
		fprintf(stderr, "EMS が存在しません。\n");
		return 0;
	}

#ifdef MOVERLAY
	/* MOVERLAY が取得している EMS を開放させる */
        _movefpause |= MOVE_PAUSE_CACHE;
	_movepause();
#endif
	/* ハードウェアが正常かどうか確認する */
	inregs.h.ah = 0x40;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "EMS が異常です (%04x)。\n", outregs.h.ah);
#ifdef MOVERLAY
		_moveresume();
#endif
		return 0;
	}

	/* ４ページの空きがあるか確認する */
	inregs.h.ah = 0x42;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.x.bx < 4)
	{
		fprintf(stderr, "EMS のフレームが足りません (%04x)。\n",
			outregs.x.cx);
#ifdef MOVERLAY
		_moveresume();
#endif
		return 0;
	}

	/* ページフレームが４つ以上あるかどうか確認する */
	inregs.x.ax = 0x5801;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.x.cx < 4)
	{
		fprintf("stderr, EMS のフレームが足りません (%04x)。\n", outregs.x.cx);
		_moveresume();
		return 0;
	}

	/* ページフレームの先頭アドレスを得る */
	inregs.h.ah = 0x41;
	int86(EMS_VECT, &inregs, &outregs);
	ems_frame = outregs.x.bx;

	/* ４ページのメモリを確保する */
	inregs.h.ah = 0x43;
	inregs.x.bx = 4;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "EMS が取得出来ません (%04x)。\n",
			outregs.h.ah);
#ifdef MOVERLAY
		_moveresume();
#endif
		_moveresume();
		return 0;
	}
	ems_handle = outregs.x.dx;

	/* ページをマップする */
	inregs.h.ah = 0x44;
	inregs.x.dx = ems_handle;
	for (i = 0; i < 4; i++)
	{
		inregs.h.al = i;
		inregs.x.bx = i;
		int86(EMS_VECT, &inregs, &outregs);
		if (outregs.h.ah != 0)
		{
			fprintf(stderr, "EMS をマップ出来ません (%04x)。\n",
				outregs.h.ah & (i << 8));
			inregs.h.ah = 0x45;
			inregs.x.dx = ems_handle;
			int86(EMS_VECT, &inregs, &outregs);
#ifdef MOVERLAY
			_moveresume();
#endif
			return 0;
		}
	}

#ifdef MOVERLAY
	/* MOVERLAY のキャッシュを再び有効にする */
	_moveresume();
#endif
	/* プログラム終了時に自動的に呼び出す */
	atexit(done_ems);

	fprintf(stderr, "\nEMS をデータ領域として使用します。\n");
	isems = 1;
	return 1;
}

/* EMS 領域から優先してメモリを確保する */
void	*emalloc(unsigned int size)
{
	void *ptr;

	/* EMS が使用可能なら */
	if (isems != 0)
	{
		/* EMS に空きがあれば */
		if ((emalloc_ptr + ((unsigned long)size)) <= 0xffffL)
		{
			/* ポインタを作って返す */
			ptr = mk_fp(ems_frame, emalloc_ptr);
			emalloc_ptr += size;
			memset(ptr, 0, size);
			return ptr;
		}
	}
	/* EMS からメモリが取得出来なければ、ヒープを使う */
	ptr = alloc(size);
	memset(ptr, 0, size);
	return ptr;
}

