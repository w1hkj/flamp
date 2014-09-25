/*
 * Copyright 1991 Phil Karn, KA9Q
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
#ifndef	_LAPB_H
#define	_LAPB_H

#ifndef	_GLOBAL_H
#include "global.h"
#endif

#ifndef	_MBUF_H
#include "mbuf.h"
#endif

#ifndef	_IFACE_H
#include "iface.h"
#endif

#ifndef	_TIMER_H
#include "timer.h"
#endif

#ifndef	_AX25_H
#include "ax25.h"
#endif

/* Upper sub-layer (LAPB) definitions */

/* Control field templates */
#define	I	0x00	/* Information frames */
#define	S	0x01	/* Supervisory frames */
#define	RR	0x01	/* Receiver ready */
#define	RNR	0x05	/* Receiver not ready */
#define	REJ	0x09	/* Reject */
#define	U	0x03	/* Unnumbered frames */
#define	SABM	0x2f	/* Set Asynchronous Balanced Mode */
#define	DISC	0x43	/* Disconnect */
#define	DM	0x0f	/* Disconnected mode */
#define	UA	0x63	/* Unnumbered acknowledge */
#define	FRMR	0x87	/* Frame reject */
#define	UI	0x03	/* Unnumbered information */
#define	PF	0x10	/* Poll/final bit */

#define	MMASK	7	/* Mask for modulo-8 sequence numbers */

/* FRMR reason bits */
#define	W	1	/* Invalid control field */
#define	X	2	/* Unallowed I-field */
#define	Y	4	/* Too-long I-field */
#define	Z	8	/* Invalid sequence number */

/* Per-connection link control block
 * These are created and destroyed dynamically,
 * and are indexed through a hash table.
 * One exists for each logical AX.25 Level 2 connection
 */
struct ax25_cb {
	struct ax25_cb *next;		/* Linked list pointer */

	struct iface *iface;		/* Interface */

	struct mbuf *txq;		/* Transmit queue */
	struct mbuf *rxasm;		/* Receive reassembly buffer */
	struct mbuf *rxq;		/* Receive queue */

	char local[AXALEN];		/* Addresses */
	char remote[AXALEN];

	struct {
		char rejsent;		/* REJ frame has been sent */
		char remotebusy;	/* Remote sent RNR */
		char rtt_run;		/* Round trip "timer" is running */
		char retrans;		/* A retransmission has occurred */
		char clone;		/* Server-type cb, will be cloned */
	} flags;

	char reason;			/* Reason for connection closing */
#define	LB_NORMAL	0		/* Normal close */
#define	LB_DM		1		/* Received DM from other end */
#define	LB_TIMEOUT	2		/* Excessive retries */

	char response;			/* Response owed to other end */
	char vs;			/* Our send state variable */
	char vr;			/* Our receive state variable */
	char unack;			/* Number of unacked frames */
	int maxframe;			/* Transmit flow control level, frames */
	uint16 paclen;			/* Maximum outbound packet size, bytes */
	uint16 window;			/* Local flow control limit, bytes */
	char proto;			/* Protocol version */
#define	V1	1			/* AX.25 Version 1 */
#define	V2	2			/* AX.25 Version 2 */
	uint16 pthresh;			/* Poll threshold, bytes */
	unsigned retries;		/* Retry counter */
	unsigned n2;			/* Retry limit */
	int state;			/* Link state */
#define	LAPB_DISCONNECTED	1
#define LAPB_LISTEN		2
#define	LAPB_SETUP		3
#define	LAPB_DISCPENDING	4
#define	LAPB_CONNECTED		5
#define	LAPB_RECOVERY		6
	struct timer t1;		/* Retry timer */
	struct timer t3;		/* Keep-alive poll timer */
	int32 rtt_time;			/* Stored clock values for RTT, ticks */
	int rtt_seq;			/* Sequence number being timed */
	int32 srt;			/* Smoothed round-trip time, ms */
	int32 mdev;			/* Mean rtt deviation, ms */

	void (*r_upcall)(struct ax25_cb *,int);	/* Receiver upcall */
	void (*t_upcall)(struct ax25_cb *,int);	/* Transmit upcall */
	void (*s_upcall)(struct ax25_cb *,int,int);	/* State change upcall */

	int user;			/* User pointer */

	int segremain;			/* Segmenter state */
};
#define	NULLAX25	((struct ax25_cb *)0)
extern struct ax25_cb Ax25default,*Ax25_cb;
extern char *Ax25states[],*Axreasons[];
extern int32 Axirtt,T3init,Blimit;
extern uint16 N2,Maxframe,Paclen,Pthresh,Axwindow,Axversion;

/* In lapb.c: */
void est_link(struct ax25_cb *axp);
void lapbstate(struct ax25_cb *axp,int s);
int lapb_input(struct ax25_cb *axp,int cmdrsp,struct mbuf *bp);
int lapb_output(struct ax25_cb *axp);
struct mbuf *segmenter(struct mbuf *bp,uint16 ssize);
int sendctl(struct ax25_cb *axp,int cmdrsp,int cmd);

/* In lapbtimer.c: */
void pollthem(void *p);
void recover(void *p);

/* In ax25subr.c: */
uint16 ftype(int control);
void lapb_garbage(int drastic);

#endif	/* _LAPB_H */
