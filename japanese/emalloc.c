
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

/* ���������줿�ݥ��󥿤�������� */
static	void *mk_fp(unsigned int seg, unsigned int off)
{
	unsigned long temp;

	temp = (((unsigned long)seg) << 4) + ((unsigned long)off);
	return (void *)(((temp & 0xffff0L) << 12) + (temp & 0x0000fL));
}

/* �ץ���ཪλ���˸ƤФ졢EMS �������� */
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
		/* �ڡ����򥢥�ޥåפ��� */
		inregs.h.al = i;
		int86(EMS_VECT, &inregs, &outregs);
		if (outregs.h.ah != 0) fprintf(stderr,
			"�ٹ�: EMS �򥢥�ޥå׽���ޤ��� (%04x)��\n",
			outregs.h.ah & (i << 8));
	}

	/* �ڡ����������� */
	inregs.h.ah = 0x45;
	inregs.x.dx = ems_handle;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "�ٹ�: EMS ��������ޤ��� (%04x)��\n",
			outregs.h.ah);
		return;
	}
/*	fprintf(stderr, "EMS �� %ud �Х��Ȼ��Ѥ���ޤ�����\n", emalloc_ptr); */
	isems = 0;
}

/* EMS ���������� */
int	detect_ems(void)
{
	union REGS inregs, outregs;
	char *vect;
	int i;

	/* EMS �ɥ饤�Ф�õ�� */
	vect = (char *)_dos_getvect(EMS_VECT);
	vect = mk_fp(FP_SEG(vect), 0x000a);
	if (memcmp(vect, EMS_STR, 8) != 0)
	{
		fprintf(stderr, "EMS ��¸�ߤ��ޤ���\n");
		return 0;
	}

#ifdef MOVERLAY
	/* MOVERLAY ���������Ƥ��� EMS ���������� */
        _movefpause |= MOVE_PAUSE_CACHE;
	_movepause();
#endif
	/* �ϡ��ɥ����������狼�ɤ�����ǧ���� */
	inregs.h.ah = 0x40;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "EMS ���۾�Ǥ� (%04x)��\n", outregs.h.ah);
#ifdef MOVERLAY
		_moveresume();
#endif
		return 0;
	}

	/* ���ڡ����ζ��������뤫��ǧ���� */
	inregs.h.ah = 0x42;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.x.bx < 4)
	{
		fprintf(stderr, "EMS �Υե졼�ब­��ޤ��� (%04x)��\n",
			outregs.x.cx);
#ifdef MOVERLAY
		_moveresume();
#endif
		return 0;
	}

	/* �ڡ����ե졼�ब���İʾ夢�뤫�ɤ�����ǧ���� */
	inregs.x.ax = 0x5801;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.x.cx < 4)
	{
		fprintf("stderr, EMS �Υե졼�ब­��ޤ��� (%04x)��\n", outregs.x.cx);
		_moveresume();
		return 0;
	}

	/* �ڡ����ե졼�����Ƭ���ɥ쥹������ */
	inregs.h.ah = 0x41;
	int86(EMS_VECT, &inregs, &outregs);
	ems_frame = outregs.x.bx;

	/* ���ڡ����Υ������ݤ��� */
	inregs.h.ah = 0x43;
	inregs.x.bx = 4;
	int86(EMS_VECT, &inregs, &outregs);
	if (outregs.h.ah != 0)
	{
		fprintf(stderr, "EMS ����������ޤ��� (%04x)��\n",
			outregs.h.ah);
#ifdef MOVERLAY
		_moveresume();
#endif
		_moveresume();
		return 0;
	}
	ems_handle = outregs.x.dx;

	/* �ڡ�����ޥåפ��� */
	inregs.h.ah = 0x44;
	inregs.x.dx = ems_handle;
	for (i = 0; i < 4; i++)
	{
		inregs.h.al = i;
		inregs.x.bx = i;
		int86(EMS_VECT, &inregs, &outregs);
		if (outregs.h.ah != 0)
		{
			fprintf(stderr, "EMS ��ޥå׽���ޤ��� (%04x)��\n",
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
	/* MOVERLAY �Υ���å����Ƥ�ͭ���ˤ��� */
	_moveresume();
#endif
	/* �ץ���ཪλ���˼�ưŪ�˸ƤӽФ� */
	atexit(done_ems);

	fprintf(stderr, "\nEMS ��ǡ����ΰ�Ȥ��ƻ��Ѥ��ޤ���\n");
	isems = 1;
	return 1;
}

/* EMS �ΰ褫��ͥ�褷�ƥ������ݤ��� */
void	*emalloc(unsigned int size)
{
	void *ptr;

	/* EMS �����Ѳ�ǽ�ʤ� */
	if (isems != 0)
	{
		/* EMS �˶���������� */
		if ((emalloc_ptr + ((unsigned long)size)) <= 0xffffL)
		{
			/* �ݥ��󥿤��ä��֤� */
			ptr = mk_fp(ems_frame, emalloc_ptr);
			emalloc_ptr += size;
			memset(ptr, 0, size);
			return ptr;
		}
	}
	/* EMS ������꤬��������ʤ���С��ҡ��פ�Ȥ� */
	ptr = alloc(size);
	memset(ptr, 0, size);
	return ptr;
}

