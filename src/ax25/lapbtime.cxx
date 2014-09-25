/* LAPB (AX25) timer recovery routines
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
#include "global.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "lapb.h"

static void tx_enq(struct ax25_cb *axp);

/** ********************************************************
 * \brief Called whenever timer T1 expires
 ***********************************************************/
void recover(void *p)
{
	register struct ax25_cb *axp = (struct ax25_cb *)p;

	axp->flags.retrans = 1;
	axp->retries++;
	if((1L << axp->retries) < Blimit)
	/* Back off retransmit timer */
		set_timer(&axp->t1,dur_timer(&axp->t1)*2);

	switch(axp->state){
		case LAPB_SETUP:
			if(axp->n2 != 0 && axp->retries > axp->n2){
				free_q(&axp->txq);
				axp->reason = LB_TIMEOUT;
				lapbstate(axp,LAPB_DISCONNECTED);
			} else {
				sendctl(axp,LAPB_COMMAND,SABM|PF);
				start_timer(&axp->t1);
			}
			break;
		case LAPB_DISCPENDING:
			if(axp->n2 != 0 && axp->retries > axp->n2){
				axp->reason = LB_TIMEOUT;
				lapbstate(axp,LAPB_DISCONNECTED);
			} else {
				sendctl(axp,LAPB_COMMAND,DISC|PF);
				start_timer(&axp->t1);
			}
			break;
		case LAPB_CONNECTED:
		case LAPB_RECOVERY:
			if(axp->n2 != 0 && axp->retries > axp->n2){
				/* Give up */
				sendctl(axp,LAPB_RESPONSE,DM|PF);
				free_q(&axp->txq);
				axp->reason = LB_TIMEOUT;
				lapbstate(axp,LAPB_DISCONNECTED);
			} else {
				/* Transmit poll */
				tx_enq(axp);
				lapbstate(axp,LAPB_RECOVERY);
			}
			break;
	}
}

/** ********************************************************
 * \brief Send a poll (S-frame command with the poll bit set)
 ***********************************************************/
void pollthem(void *p)
{
	register struct ax25_cb *axp;

	axp = (struct ax25_cb *)p;
	if(axp->proto == V1)
		return;	/* Not supported in the old protocol */
	switch(axp->state){
		case LAPB_CONNECTED:
			axp->retries = 0;
			tx_enq(axp);
			lapbstate(axp,LAPB_RECOVERY);
			break;
	}
}

/** ********************************************************
 * \brief Transmit query
 ***********************************************************/
static void tx_enq(struct ax25_cb *axp)
{
	char ctl;
	struct mbuf *bp;

	/* I believe that retransmitting the oldest unacked
	 * I-frame tends to give better performance than polling,
	 * as long as the frame isn't too "large", because
	 * chances are that the I frame got lost anyway.
	 * This is an option in LAPB, but not in the official AX.25.
	 */
	if(axp->txq != NULLBUF
	   && (len_p(axp->txq) < axp->pthresh || axp->proto == V1)){
		/* Retransmit oldest unacked I-frame */
		dup_p(&bp,axp->txq,0,len_p(axp->txq));
		ctl = PF | I | (((axp->vs - axp->unack) & MMASK) << 1)
		| (axp->vr << 5);
		sendframe(axp,LAPB_COMMAND,ctl,bp);
	} else {
		ctl = len_p(axp->rxq) >= axp->window ? RNR|PF : RR|PF;	
		sendctl(axp,LAPB_COMMAND,ctl);
	}
	axp->response = 0;	
	stop_timer(&axp->t3);
	start_timer(&axp->t1);
}
