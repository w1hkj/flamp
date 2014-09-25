#ifndef	_AX25_H
#define	_AX25_H

/*
 * Copyright 1991 Phil Karn
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

#ifndef	_GLOBAL_H
#include "global.h"
#endif

#ifndef	_MBUF_H
#include "mbuf.h"
#endif

#ifndef	_IFACE_H
#include "iface.h"
#endif

#ifndef _SOCKADDR_H
#include "sockaddr.h"
#endif

extern char Ax25_eol[];

/* AX.25 datagram (address) sub-layer definitions */

#define	MAXDIGIS	7	/* Maximum number of digipeaters */
#define	ALEN		6	/* Number of chars in callsign field */
#define	AXALEN		7	/* Total AX.25 address length, including SSID */
#define	AXBUF		10	/* Buffer size for maximum-length ascii call */

#ifndef _LAPB_H
#include "lapb.h"
#endif

/* Bits within SSID field of AX.25 address */
#define	SSID		0x1e	/* Sub station ID */
#define	REPEATED	0x80	/* Has-been-repeated bit in repeater field */
#define	E		0x01	/* Address extension bit */
#define	C		0x80	/* Command/response designation */

/* Our AX.25 address */
extern char Mycall[AXALEN];

/* List of AX.25 multicast addresses, e.g., "QST   -0" in shifted ASCII */
extern char Ax25multi[][AXALEN];

extern int Digipeat;
extern int Ax25mbox;

/* Number of chars in interface field. The involved definition takes possible
 * alignment requirements into account, since ax25_addr is of an odd size.
 */
#define	ILEN	(sizeof(struct sockaddr) - sizeof(short) - AXALEN)

/* Socket address, AX.25 style */
struct sockaddr_ax {
	short sax_family;		/* 2 bytes */
	char ax25_addr[AXALEN];
	char iface[ILEN];		/* Interface name */
};

/* Internal representation of an AX.25 header */
struct ax25 {
	char dest[AXALEN];		/* Destination address */
	char source[AXALEN];		/* Source address */
	char digis[MAXDIGIS][AXALEN];	/* Digi string */
	int ndigis;			/* Number of digipeaters */
	int nextdigi;			/* Index to next digi in chain */
	int cmdrsp;			/* Command/response */
};

/* C-bit stuff */
#define	LAPB_UNKNOWN		0
#define	LAPB_COMMAND		1
#define	LAPB_RESPONSE		2

/* AX.25 routing table entry */
struct ax_route {
	struct ax_route *next;		/* Linked list pointer */
	char target[AXALEN];
	char digis[MAXDIGIS][AXALEN];
	int ndigis;
	char type;
#define	AX_LOCAL	1		/* Set by local ax25 route command */
#define	AX_AUTO		2		/* Set by incoming packet */
};
#define NULLAXR	((struct ax_route *)0)

extern struct ax_route *Ax_routes;
extern struct ax_route Ax_default;

/* AX.25 Level 3 Protocol IDs (PIDs) */
#define PID_X25		0x01	/* CCITT X.25 PLP */
#define	PID_SEGMENT	0x08	/* Segmentation fragment */
#define PID_TEXNET	0xc3	/* TEXNET datagram protocol */
#define	PID_LQ		0xc4	/* Link quality protocol */
#define	PID_APPLETALK	0xca	/* Appletalk */
#define	PID_APPLEARP	0xcb	/* Appletalk ARP */
#define	PID_IP		0xcc	/* ARPA Internet Protocol */
#define	PID_ARP		0xcd	/* ARPA Address Resolution Protocol */
#define	PID_NETROM	0xcf	/* NET/ROM */
#define	PID_NO_L3	0xf0	/* No level 3 protocol */

#define	SEG_FIRST	0x80	/* First segment of a sequence */
#define	SEG_REM		0x7f	/* Mask for # segments remaining */

/* Link quality report packet header, internal format */
struct lqhdr {
	uint16 version;		/* Version number of protocol */
#define	LINKVERS	1
	int32	ip_addr;	/* Sending station's IP address */
};
#define	LQHDR	6
/* Link quality entry, internal format */
struct lqentry {
	char addr[AXALEN];	/* Address of heard station */
	int32 count;		/* Count of packets heard from that station */
};
#define	LQENTRY	11

/* Link quality database record format
 * Currently used only by AX.25 interfaces
 */
struct lq {
	struct lq *next;
	char addr[AXALEN];	/* Hardware address of station heard */
	struct iface *iface;	/* Interface address was heard on */
	int32 time;		/* Time station was last heard */
	int32 currxcnt;	/* Current # of packets heard from this station */

#ifdef	notdef		/* Not yet implemented */
	/* # of packets heard from this station as of his last update */
	int32 lastrxcnt;

	/* # packets reported as transmitted by station as of his last update */
	int32 lasttxcnt;

	uint16 hisqual;	/* Fraction (0-1000) of station's packets heard
			 * as of last update
			 */
	uint16 myqual;	/* Fraction (0-1000) of our packets heard by station
			 * as of last update
			 */
#endif
};
#define	NULLLQ	(struct lq *)0

extern struct lq *Lq;	/* Link quality record headers */

/* Structure used to keep track of monitored destination addresses */
struct ld {
	struct ld *next;	/* Linked list pointers */
	char addr[AXALEN];/* Hardware address of destination overheard */
	struct iface *iface;	/* Interface address was heard on */
	int32 time;		/* Time station was last mentioned */
	int32 currxcnt;	/* Current # of packets destined to this station */
};
#define	NULLLD	(struct ld *)0

extern struct ld *Ld;	/* Destination address record headers */

/* Linkage to network protocols atop ax25 */
struct axlink {
	int pid;
	void (*funct)(struct iface *,struct ax25_cb *,char *, char *,
	 struct mbuf *,int);
};
extern struct axlink Axlink[];

/* Codes for the open_ax25 call */
#define	AX_PASSIVE	0
#define	AX_ACTIVE	1
#define	AX_SERVER	2	/* Passive, clone on opening */

/* In ax25.c: */
struct ax_route *ax_add(char *,int,char digis[][AXALEN],int);
int ax_drop(char *);
struct ax_route *ax_lookup(char *);
void ax_recv(struct iface *,struct mbuf *);
int axui_send(struct mbuf *bp,struct iface *iface,int32 gateway,int tos);
int axi_send(struct mbuf *bp,struct iface *iface,int32 gateway,int tos);
int ax_output(struct iface *iface,char *dest,char *source,uint16 pid,
	struct mbuf *data);
int sendframe(struct ax25_cb *axp,int cmdrsp,int ctl,struct mbuf *data);
void axnl3(struct iface *iface,struct ax25_cb *axp,char *src,
	char *dest,struct mbuf *bp,int mcast);

/* In ax25cmd.c: */
void st_ax25(struct ax25_cb *axp);

/* In axhdr.c: */
struct mbuf *htonax25(struct ax25 *hdr,struct mbuf *data);
int ntohax25(struct ax25 *hdr,struct mbuf **bpp);

/* In axlink.c: */
void getlqentry(struct lqentry *ep,struct mbuf **bpp);
void getlqhdr(struct lqhdr *hp,struct mbuf **bpp);
void logsrc(struct iface *iface,char *addr);
void logdest(struct iface *iface,char *addr);
char *putlqentry(char *cp,char *addr,int32 count);
char *putlqhdr(char *cp,uint16 version,int32 ip_addr);
struct lq *al_lookup(struct iface *ifp,char *addr,int sort);

/* In ax25user.c: */
int ax25val(struct ax25_cb *axp);
int disc_ax25(struct ax25_cb *axp);
int kick_ax25(struct ax25_cb *axp);
struct ax25_cb *open_ax25(struct iface *,char *,char *,
	int,uint16,
	void (*)(struct ax25_cb *,int),
	void (*)(struct ax25_cb *,int),
	void (*)(struct ax25_cb *,int,int),
	int user);
struct mbuf *recv_ax25(struct ax25_cb *axp,uint16 cnt);
int reset_ax25(struct ax25_cb *axp);
int send_ax25(struct ax25_cb *axp,struct mbuf *bp,int pid);

/* In ax25subr.c: */
int addreq(char *a,char *b);
struct ax25_cb *cr_ax25(char *addr);
void del_ax25(struct ax25_cb *axp);
struct ax25_cb *find_ax25(char *);
char *pax25(char *e,char *addr);
int setcall(char *out,char *call);

/* In axsocket.c: */
int so_ax_sock(struct usock *up,int protocol);
int so_ax_bind(struct usock *up);
int so_ax_listen(struct usock *up,int backlog);
int so_ax_conn(struct usock *up);
int so_ax_recv(struct usock *up,struct mbuf **bpp,char *from,
	int *fromlen);
int so_ax_send(struct usock *up,struct mbuf *bp,char *to);
int so_ax_qlen(struct usock *up,int rtx);
int so_ax_kick(struct usock *up);
int so_ax_shut(struct usock *up,int how);
int so_ax_close(struct usock *up);
int checkaxaddr(char *name,int namelen);

int so_axui_sock(struct usock *up,int protocol);
int so_axui_bind(struct usock *up);
int so_axui_conn(struct usock *up);
int so_axui_recv(struct usock *up,struct mbuf **bpp,char *from,
	int *fromlen);
int so_axui_send(struct usock *up,struct mbuf *bp,char *to);
int so_axui_qlen(struct usock *up,int rtx);
int so_axui_shut(struct usock *up,int how);
int so_axui_close(struct usock *up);
char *axpsocket(struct sockaddr *p);
char *axstate(struct usock *up);
int so_ax_stat(struct usock *up);

void beac_input(struct iface *iface,char *src,struct mbuf *bp);

void s_arcall(struct ax25_cb *axp,int cnt);
void s_ascall(struct ax25_cb *axp,int old,int new);
void s_atcall(struct ax25_cb *axp,int cnt);

#endif  /* _AX25_H */
