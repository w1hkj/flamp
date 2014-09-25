/* AX25 control commands
 * Copyright 1991 Phil Karn, KA9Q
 * Copyright (C) 2014 Robert Stiles, KK5VD
 *
 * Modified for use with FLLNK/FLDIGI
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

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "proc.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include "cmdparse.h"
#include "socket.h"
#include "mailbox.h"
#include "session.h"
#include "tty.h"
#include "nr4.h"
#include "commands.h"

static int axdest(struct iface *ifp);
static int axheard(struct iface *ifp);
static void axflush(struct iface *ifp);
static int doaxflush(int argc,char *argv[],void *p);
static int doaxirtt(int argc,char *argv[],void *p);
static int doaxkick(int argc,char *argv[],void *p);
static int doaxreset(int argc,char *argv[],void *p);
static int doaxroute(int argc,char *argv[],void *p);
static int doaxstat(int argc,char *argv[],void *p);
static int doaxwindow(int argc,char *argv[],void *p);
static int doblimit(int argc,char *argv[],void *p);
static int dodigipeat(int argc,char *argv[],void *p);
static int domaxframe(int argc,char *argv[],void *p);
static int domycall(int argc,char *argv[],void *p);
static int don2(int argc,char *argv[],void *p);
static int dopaclen(int argc,char *argv[],void *p);
static int dopthresh(int argc,char *argv[],void *p);
static int dot3(int argc,char *argv[],void *p);
static int doversion(int argc,char *argv[],void *p);

char *Ax25states[] = {
	"",
	"Disconn",
	"Listening",
	"Conn pend",
	"Disc pend",
	"Connected",
	"Recovery",
};

/* Ascii explanations for the disconnect reasons listed in lapb.h under
 * "reason" in ax25_cb
 */
char *Axreasons[] = {
	"Normal",
	"DM received",
	"Timeout"
};

static struct cmds Axcmds[] = {
	"blimit",	doblimit,	0, 0, NULLCHAR,
	"destlist",	doaxdest,	0, 0, NULLCHAR,
	"digipeat",	dodigipeat,	0, 0, NULLCHAR,
	"flush",	doaxflush,	0, 0, NULLCHAR,
	"heard",	doaxheard,	0, 0, NULLCHAR,
	"irtt",		doaxirtt,	0, 0, NULLCHAR,
	"kick",		doaxkick,	0, 2, "ax25 kick <axcb>",
	"maxframe",	domaxframe,	0, 0, NULLCHAR,
	"mycall",	domycall,	0, 0, NULLCHAR,
	"paclen",	dopaclen,	0, 0, NULLCHAR,
	"pthresh",	dopthresh,	0, 0, NULLCHAR,
	"reset",	doaxreset,	0, 2, "ax25 reset <axcb>",
	"retry",	don2,		0, 0, NULLCHAR,
	"route",	doaxroute,	0, 0, NULLCHAR,
	"status",	doaxstat,	0, 0, NULLCHAR,
	"t3",		dot3,		0, 0, NULLCHAR,
	"version",	doversion,	0, 0, NULLCHAR,
	"window",	doaxwindow,	0, 0, NULLCHAR,
	NULLCHAR,
};

/** ********************************************************
 * \brief Multiplexer for top-level ax25 command
 ***********************************************************/
int doax25(int argc, char *argv[], void *p)
{
	return subcmd(Axcmds,argc,argv,p);
}

/** ********************************************************
 *
 ***********************************************************/
int doaxheard(int argc, char *argv[], void *p)
{
	struct iface *ifp;

	if(argc > 1){
		if((ifp = if_lookup(argv[1])) == NULLIF){
			printf("Interface %s unknown\n",argv[1]);
			return 1;
		}
		if(ifp->output != ax_output){
			printf("Interface %s not AX.25\n",argv[1]);
			return 1;
		}
		axheard(ifp);
		return 0;
	}
	for(ifp = Ifaces;ifp != NULLIF;ifp = ifp->next){
		if(ifp->output != ax_output)
			continue;	/* Not an ax.25 interface */
		if(axheard(ifp) == EOF)
			break;
	}
	return 0;
}
/** ********************************************************
 *
 ***********************************************************/
static int axheard(struct iface *ifp)
{
	struct lq *lp;
	char tmp[AXBUF];

	if(ifp->hwaddr == NULLCHAR)
		return 0;
	printf("%s:\n",ifp->name);
	printf("Station   Last heard           Pkts\n");
	for(lp = Lq;lp != NULLLQ;lp = lp->next){
		if(lp->iface != ifp)
			continue;
		if(printf("%-10s%-17s%8lu\n",pax25(tmp,lp->addr),
		 tformat(secclock() - lp->time),lp->currxcnt) == EOF)
			return EOF;
	}
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
int doaxdest(int argc, char *argv[], void *p)
{
	struct iface *ifp;

	if(argc > 1){
		if((ifp = if_lookup(argv[1])) == NULLIF){
			printf("Interface %s unknown\n",argv[1]);
			return 1;
		}
		if(ifp->output != ax_output){
			printf("Interface %s not AX.25\n",argv[1]);
			return 1;
		}
		axdest(ifp);
		return 0;
	}
	for(ifp = Ifaces;ifp != NULLIF;ifp = ifp->next){
		if(ifp->output != ax_output)
			continue;	/* Not an ax.25 interface */
		if(axdest(ifp) == EOF)
			break;
	}
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
static int axdest(struct iface *ifp)
{
	struct ld *lp;
	struct lq *lq;
	char tmp[AXBUF];

	if(ifp->hwaddr == NULLCHAR)
		return 0;
	printf("%s:\n",ifp->name);
	printf("Station   Last ref         Last heard           Pkts\n");
	for(lp = Ld;lp != NULLLD;lp = lp->next){
		if(lp->iface != ifp)
			continue;

		printf("%-10s%-17s",
		 pax25(tmp,lp->addr),tformat(secclock() - lp->time));

		if(addreq(lp->addr,ifp->hwaddr)){
			/* Special case; it's our address */
			printf("%-17s",tformat(secclock() - ifp->lastsent));
		} else if((lq = al_lookup(ifp,lp->addr,0)) == NULLLQ){
			printf("%-17s","");
		} else {
			printf("%-17s",tformat(secclock() - lq->time));
		}
		if(printf("%8lu\n",lp->currxcnt) == EOF)
			return EOF;
	}
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
static int doaxflush(int argc, char *argv[], void *p)
{
	struct iface *ifp;

	for(ifp = Ifaces;ifp != NULLIF;ifp = ifp->next){
		if(ifp->output != ax_output)
			continue;	/* Not an ax.25 interface */
		axflush(ifp);
	}
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
static void axflush(struct iface *ifp)
{
	struct lq *lp,*lp1;
	struct ld *ld,*ld1;

	ifp->rawsndcnt = 0;
	for(lp = Lq;lp != NULLLQ;lp = lp1){
		lp1 = lp->next;
		free((char *)lp);
	}
	Lq = NULLLQ;
	for(ld = Ld;ld != NULLLD;ld = ld1){
		ld1 = ld->next;
		free((char *)ld);
	}
	Ld = NULLLD;
}

/** ********************************************************
 *
 ***********************************************************/
static int doaxreset(int argc, char *argv[], void *p)
{
	struct ax25_cb *axp;

	axp = (struct ax25_cb *)ltop(htol(argv[1]));
	if(!ax25val(axp)){
		printf(Notval);
		return 1;
	}
	reset_ax25(axp);
	return 0;
}

/** ********************************************************
 * \brief Display AX.25 link level control blocks
 ***********************************************************/
static int doaxstat(int argc, char *argv[], void *p)
{
	register struct ax25_cb *axp;
	char tmp[AXBUF];

	if(argc < 2){
		printf("    &AXB Snd-Q   Rcv-Q   Remote    State\n");
		for(axp = Ax25_cb;axp != NULLAX25; axp = axp->next){
			if(printf("%8lx %-8d%-8d%-10s%s\n",
				ptol(axp),
				len_q(axp->txq),len_p(axp->rxq),
				pax25(tmp,axp->remote),
				Ax25states[axp->state]) == EOF)
					return 0;
		}
		return 0;
	}
	axp = (struct ax25_cb *)ltop(htol(argv[1]));
	if(!ax25val(axp)){
		printf(Notval);
		return 1;
	}
	st_ax25(axp);
	return 0;
}

/** ********************************************************
 * \brief Dump one control block
 ***********************************************************/
void st_ax25(struct ax25_cb *axp)
{
	char tmp[AXBUF];

	if(axp == NULLAX25)
		return;
	printf("    &AXB Remote   RB V(S) V(R) Unack P Retry State\n");

	printf("%8lx %-9s%c%c",ptol(axp),pax25(tmp,axp->remote),
	 axp->flags.rejsent ? 'R' : ' ',
	 axp->flags.remotebusy ? 'B' : ' ');
	printf(" %4d %4d",axp->vs,axp->vr);
	printf(" %02u/%02u %u",axp->unack,axp->maxframe,axp->proto);
	printf(" %02u/%02u",axp->retries,axp->n2);
	printf(" %s\n",Ax25states[axp->state]);

	printf("srtt = %lu mdev = %lu ",axp->srt,axp->mdev);
	printf("T1: ");
	if(run_timer(&axp->t1))
		printf("%lu",read_timer(&axp->t1));
	else
		printf("stop");
	printf("/%lu ms; ",dur_timer(&axp->t1));

	printf("T3: ");
	if(run_timer(&axp->t3))
		printf("%lu",read_timer(&axp->t3));
	else
		printf("stop");
	printf("/%lu ms\n",dur_timer(&axp->t3));

}

/** ********************************************************
 * \brief Display or change our AX.25 address
 ***********************************************************/
static int domycall(int argc, char *argv[], void *p)
{
	char tmp[AXBUF];

	if(argc < 2){
		printf("%s\n",pax25(tmp,Mycall));
		return 0;
	}
	if(setcall(Mycall,argv[1]) == -1)
		return -1;
	return 0;
}

/** ********************************************************
 * \brief Control AX.25 digipeating
 ***********************************************************/
static int dodigipeat(int argc, char *argv[], void *p)
{
	return setbool(&Digipeat,"Digipeat",argc,argv);
}

/** ********************************************************
 * \brief Set limit on retransmission backoff
 ***********************************************************/
static int doblimit(int argc, char *argv[], void *p)
{
	return setlong(&Blimit,"blimit",argc,argv);
}

/** ********************************************************
 *
 ***********************************************************/
static int doversion(int argc, char *argv[], void *p)
{
	return setshort(&Axversion, "AX25 version", argc, argv);
}

/** ********************************************************
 *
 ***********************************************************/
static int doaxirtt(int argc, char *argv[], void *p)
{
	return setlong(&Axirtt,"Initial RTT (ms)",argc,argv);
}

/** ********************************************************
 * \brief Set idle timer
 ***********************************************************/
static int dot3(int argc, char *argv[], void *p)
{
	return setlong(&T3init, "Idle poll timer (ms)",argc,argv);
}

/** ********************************************************
 * \brief Set retry limit count
 ***********************************************************/
static int don2(int argc, char *argv[], void *p)
{
	return setshort(&N2,"Retry limit",argc,argv);
}

/** ********************************************************
 * \brief Force a retransmission
 ***********************************************************/
static int doaxkick(int argc, char *argv[], void *p)
{
	struct ax25_cb *axp;

	axp = (struct ax25_cb *)ltop(htol(argv[1]));
	if(!ax25val(axp)){
		printf(Notval);
		return 1;
	}
	kick_ax25(axp);
	return 0;
}

/** ********************************************************
 * \brief Set maximum number of frames that will be 
 * allowed in flight
 ***********************************************************/
static int domaxframe(int argc, char *argv[], void *p)
{
	return setshort(&Maxframe,"Window size (frames)",argc,argv);
}

/** ********************************************************
 * \brief Set maximum length of I-frame data field
 ***********************************************************/
static int dopaclen(int argc, char *argv[], void *p)
{
	return setshort(&Paclen,"Max frame length (bytes)",argc,argv);
}

/** ********************************************************
 * \brief Set size of I-frame above which polls will be 
 * sent after a timeout
 ***********************************************************/
static int dopthresh(int argc, char *argv[], void *p)
{
	return setshort(&Pthresh,"Poll threshold (bytes)",argc,argv);
}

/* Set high water mark on receive queue that triggers RNR */
/** ********************************************************
 * \brief Set high water mark on receive queue that 
 * triggers RNR.
 ***********************************************************/
static int doaxwindow(int argc, char *argv[], void *p)
{
	return setshort(&Axwindow,"AX25 receive window (bytes)",argc,argv);
}
/* End of ax25 subcommands */

/** ********************************************************
 * \brief Initiate interactive AX.25 connect to remote 
 * station
 ***********************************************************/
int doconnect(int argc, char *argv[], void *p)
{
	struct sockaddr_ax fsocket;
	struct session *sp;
	int ndigis,i,s;
	char digis[MAXDIGIS][AXALEN];
	char target[AXALEN];

	/* If digipeaters are given, put them in the routing table */
	if(argc > 3){
		setcall(target,argv[2]);
		ndigis = argc - 3;
		if(ndigis > MAXDIGIS){
			printf("Too many digipeaters\n");
			return 1;
		}
		for(i=0;i<ndigis;i++){
			if(setcall(digis[i],argv[i+3]) == -1){
				printf("Bad digipeater %s\n",argv[i+3]);
				return 1;
			}
		}
		if(ax_add(target,AX_LOCAL,digis,ndigis) == NULLAXR){
			printf("Route add failed\n");
			return 1;
		}
	}
	/* Allocate a session descriptor */
	if((sp = newsession(Cmdline,AX25TNC,1)) == NULLSESSION){
		printf("Too many sessions\n");
		return 1;
	}
	if((s = socket(AF_AX25,SOCK_STREAM,0)) == -1){
		printf("Can't create socket\n");
		freesession(sp);
		keywait(NULLCHAR,1);
		return 1;
	}
	fsocket.sax_family = AF_AX25;
	setcall(fsocket.ax25_addr,argv[2]);
	strncpy(fsocket.iface,argv[1],ILEN);
	sp->network = fdopen(s,"r+t");
	setvbuf(sp->network,NULLCHAR,_IOLBF,BUFSIZ);
	return tel_connect(sp, (char *)&fsocket, sizeof(struct sockaddr_ax));
}

/** ********************************************************
 * \brief Display and modify AX.25 routing table
 ***********************************************************/
static int doaxroute(int argc, char *argv[], void *p)
{
	char tmp[AXBUF];
	int i,ndigis;
	register struct ax_route *axr;
	char target[AXALEN],digis[MAXDIGIS][AXALEN];

	if(argc < 2){
		printf("Target    Type   Digipeaters\n");
		for(axr = Ax_routes;axr != NULLAXR;axr = axr->next){
			printf("%-10s%-6s",pax25(tmp,axr->target),
			 axr->type == AX_LOCAL ? "Local":"Auto");
			for(i=0;i<axr->ndigis;i++){
				printf(" %s",pax25(tmp,axr->digis[i]));
			}
			if(printf("\n") == EOF)
				return 0;
		}
		return 0;
	}
	if(argc < 3){
		printf("Usage: ax25 route add <target> [digis...]\n");
		printf("       ax25 route drop <target>\n");
		return 1;
	}
	if(setcall(target,argv[2]) == -1){
		printf("Bad target %s\n",argv[2]);
		return 1;
	}
	switch(argv[1][0]){
	case 'a':	/* Add route */
		if(argc < 3){
			printf("Usage: ax25 route add <target> [digis...]\n");
			return 1;
		}
		ndigis = argc - 3;
		if(ndigis > MAXDIGIS){
			printf("Too many digipeaters\n");
			return 1;
		}
		for(i=0;i<ndigis;i++){
			if(setcall(digis[i],argv[i+3]) == -1){
				printf("Bad digipeater %s\n",argv[i+3]);
				return 1;
			}
		}
		if(ax_add(target,AX_LOCAL,digis,ndigis) == NULLAXR){
			printf("Failed\n");
			return 1;
		}
		break;
	case 'd':	/* Drop route */
		if(ax_drop(target) == -1){
			printf("Not in table\n");
			return 1;
		}
		break;
	default:
		printf("Unknown command %s\n",argv[1]);
		return 1;
	}
	return 0;
}
