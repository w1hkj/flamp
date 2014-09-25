/* Low level AX.25 routines:
 *  callsign conversion
 *  control block management
 *
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
#include "ax25.h"
#include "lapb.h"
#include <ctype.h>

struct ax25_cb *Ax25_cb;

/* Default AX.25 parameters */
int32 T3init = 0;		    /*!< No keep-alive polling */
uint16 Maxframe = 1;		/*!< Stop and wait */
uint16 N2 = 10;			    /*!< 10 retries */
uint16 Axwindow = 2048;		/*!< 2K incoming text before RNR'ing */
uint16 Paclen = 256;		/*!< 256-byte I fields */
uint16 Pthresh = 128;		/*!< Send polls for packets larger than this */
int32 Axirtt = 5000;		/*!< Initial round trip estimate, ms */
uint16 Axversion = V1;		/*!< Protocol version */
int32 Blimit = 30;		    /*!< Retransmission backoff limit */

/** ********************************************************
 * \brief  Look up entry in connection table
 ***********************************************************/
struct ax25_cb * find_ax25(char *addr)
{
	struct ax25_cb *axp;
	struct ax25_cb *axlast = NULLAX25;

	/* Search list */
	for(axp = Ax25_cb; axp != NULLAX25; axlast=axp,axp = axp->next){
		if(addreq(axp->remote,addr)){
			if(axlast != NULLAX25){
				/* Move entry to top of list to speed
				 * future searches
				 */
				axlast->next = axp->next;
				axp->next = Ax25_cb;
				Ax25_cb = axp;
			}
			return axp;
		}
	}
	return NULLAX25;
}

/** ********************************************************
 * \brief Remove entry from connection table
 ***********************************************************/
void del_ax25(struct ax25_cb *conn)
{
	register struct ax25_cb *axp;
	struct ax25_cb *axlast = NULLAX25;

	for(axp = Ax25_cb; axp != NULLAX25; axlast=axp,axp = axp->next){
		if(axp == conn)
			break;
	}
	if(axp == NULLAX25)
		return;	/* Not found */

	/* Remove from list */
	if(axlast != NULLAX25)
		axlast->next = axp->next;
	else
		Ax25_cb = axp->next;

	/* Timers should already be stopped, but just in case... */
	stop_timer(&axp->t1);
	stop_timer(&axp->t3);

	/* Free allocated resources */
	free_q(&axp->txq);
	free_q(&axp->rxasm);
	free_q(&axp->rxq);
	free((char *)axp);
}

/** ********************************************************
 * \brief  Create an ax25 control block. Allocate a new 
 * structure, if necessary, and fill it with all the 
 * defaults. The caller is still responsible for filling 
 * in the reply address
 ***********************************************************/
struct ax25_cb * cr_ax25(char *addr)
{
	register struct ax25_cb *axp;

	if(addr == NULLCHAR)
		return NULLAX25;

	if((axp = find_ax25(addr)) == NULLAX25){
		/* Not already in table; create an entry
		 * and insert it at the head of the chain
		 */
		axp = (struct ax25_cb *)callocw(1,sizeof(struct ax25_cb));
		axp->next = Ax25_cb;
		Ax25_cb = axp;
	}
	axp->user = -1;
	axp->state = LAPB_DISCONNECTED;
	axp->maxframe = Maxframe;
	axp->window = Axwindow;
	axp->paclen = Paclen;
	axp->proto = Axversion;	/* Default, can be changed by other end */
	axp->pthresh = Pthresh;
	axp->n2 = N2;
	axp->srt = Axirtt;
	set_timer(&axp->t1,2*axp->srt);
	axp->t1.func = recover;
	axp->t1.arg = axp;

	set_timer(&axp->t3,T3init);
	axp->t3.func = pollthem;
	axp->t3.arg = axp;

	/* Always to a receive and state upcall as default */
	axp->r_upcall = s_arcall;
	axp->s_upcall = s_ascall;

	return axp;
}

/** ********************************************************
 * \brief  setcall - convert callsign plus substation ID 
 * of the form "KA9Q-0" to AX.25 (shifted) address format
 * <br>Address extension bit is left clear
 * <br>Return -1 on error, 0 if OK
 ***********************************************************/
int setcall(char *out, char *call)
{
	int csize;
	unsigned ssid;
	int i;
	char *dp;
	char c;

	if(out == NULLCHAR || call == NULLCHAR || *call == '\0')
		return -1;

	/* Find dash, if any, separating callsign from ssid
	 * Then compute length of callsign field and make sure
	 * it isn't excessive
	 */
	dp = strchr(call,'-');
	if(dp == NULLCHAR)
		csize = strlen(call);
	else
		csize = dp - call;
	if(csize > ALEN)
		return -1;
	/* Now find and convert ssid, if any */
	if(dp != NULLCHAR){
		dp++;	/* skip dash */
		ssid = atoi(dp);
		if(ssid > 15)
			return -1;
	} else
		ssid = 0;
	/* Copy upper-case callsign, left shifted one bit */
	for(i=0;i<csize;i++){
		c = *call++;
		if(islower(c))
			c = toupper(c);
		*out++ = c << 1;
	}
	/* Pad with shifted spaces if necessary */
	for(;i<ALEN;i++)
		*out++ = ' ' << 1;
	
	/* Insert substation ID field and set reserved bits */
	*out = 0x60 | (ssid << 1);
	return 0;
}

/** ********************************************************
 *
 ***********************************************************/
int addreq(char *a, char *b)
{
	if(memcmp(a,b,ALEN) != 0 || ((a[ALEN] ^ b[ALEN]) & SSID) != 0)
		return 0;
	else
		return 1;
}

/** ********************************************************
 * \brief Convert encoded AX.25 address to printable string
 ***********************************************************/
char * pax25(char *e, char *addr)
{
	register int i;
	char c;
	char *cp;

	cp = e;
	for(i=ALEN;i != 0;i--){
		c = (*addr++ >> 1) & 0x7f;
		if(c != ' ')
			*cp++ = c;
	}
	if((*addr & SSID) != 0)
		sprintf(cp,"-%d",(*addr >> 1) & 0xf);	/* ssid */
	else
		*cp = '\0';
	return e;
}

/** ********************************************************
 * \brief Figure out the frame type from the control field
 * This is done by masking out any sequence numbers and the
 * poll/final bit after determining the general class 
 * (I/S/U) of the frame
 ***********************************************************/
uint16 ftype(int control)
{
	if((control & 1) == 0)	/* An I-frame is an I-frame... */
		return I;
	if(control & 2)		/* U-frames use all except P/F bit for type */
		return (uint16)(uchar(control) & ~PF);
	else			/* S-frames use low order 4 bits for type */
		return (uint16)(uchar(control) & 0xf);
}

/** ********************************************************
 *
 ***********************************************************/
void lapb_garbage(int red)
{
	register struct ax25_cb *axp;

	for(axp=Ax25_cb;axp != NULLAX25;axp = axp->next){
		mbuf_crunch(&axp->rxq);
		mbuf_crunch(&axp->rxasm);
	}
}

