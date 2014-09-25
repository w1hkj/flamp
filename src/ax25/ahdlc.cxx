/* Asynchronous HDLC routines
 *
 * Copyright 1994 Phil Karn, KA9Q
 *
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
#include "ahdlc.h"
#include "crc.h"

static char *putbyte(char *, char);

/** ********************************************************
 *
 ***********************************************************/
void init_hdlc(struct ahdlc *hp, int maxsize)
{
	hp->escaped = 0;
	hp->hunt = 0;
	hp->inframe = NULLBUF;
	hp->maxsize = maxsize;
	hp->fcs = FCS_START;
	hp->rxframes = 0;
	hp->aborts = 0;
	hp->toobigs = 0;
	hp->crcerrs = 0;
}

/** ********************************************************
 * \brief Process incoming data. Return completed packets, 
 * NULLBUF otherwise
 * \param ap  HDLC Receiver control block
 * \param buf Raw incoming data
 * \param len length
 ***********************************************************/
struct mbuf * ahdlcrx(struct ahdlc *ap, char *buf, int len)
{
	int c;
	struct mbuf *bp;

	while(len-- != 0){
		c = *buf++;
		if(c == HDLC_ESC_ASYNC){
			ap->escaped = 1;
		} else if(c != HDLC_FLAG){
			if(ap->hunt)
				continue;	/* Ignore until next flag */
			/* Normal character within packet */
			if(ap->escaped){
				c ^= HDLC_ESC_COMPL;
				ap->escaped = 0;
			}
			if(ap->inframe == NULLBUF)
				ap->inframe = ambufw(ap->maxsize);
			if(ap->inframe->cnt == ap->maxsize){
				/* Frame too large */
				ap->toobigs++;
				free_p(ap->inframe);
				ap->inframe = NULLBUF;
				ap->escaped = 0;
				ap->fcs = FCS_START;
				ap->hunt = 1;
			} else {
				/* Store character, update FCS */
				ap->inframe->data[ap->inframe->cnt++] = c;
				ap->fcs = FCS(ap->fcs,c);
			}
		} else if(ap->escaped){
			/* ESC, FLAG is frame abort */
			ap->aborts++;
			ap->hunt = 1;
			ap->escaped = 0;
			free_p(ap->inframe);
			ap->inframe = NULLBUF;
			ap->fcs = FCS_START;
		} else if(ap->hunt){
			/* Found flag in hunt mode. Reset for new frame */
			ap->hunt = 0;
		} else if(ap->inframe == NULLBUF){
			/* Padding flags, ignore */
		} else if(ap->fcs != FCS_FINAL){
			/* CRC error */
			ap->crcerrs++;
			free_p(ap->inframe);
			ap->inframe = NULLBUF;
			ap->fcs = FCS_START;
		} else if(ap->inframe->cnt < 2){
			/* Runt frame */
			ap->runts++;
			free_p(ap->inframe);
			ap->inframe = NULLBUF;
			ap->fcs = FCS_START;
		} else {
			/* Normal end-of-frame */
			ap->rxframes++;
			bp = ap->inframe;
			ap->inframe = NULLBUF;
			ap->fcs = FCS_START;
			bp->cnt -= 2;
			return bp;
		}
	}
	return NULLBUF;
}

/** ********************************************************
 * \brief Encode a packet in asynchronous HDLC for 
 * transmission.
 ***********************************************************/
struct mbuf * ahdlctx(struct mbuf *bp)
{
	struct mbuf *obp;
	char *cp;
	int c;
	uint16 fcs;

	fcs = FCS_START;
	obp = ambufw(5+2*len_p(bp));	/* Allocate worst-case */
	cp = obp->data;
	while((c = PULLCHAR(&bp)) != -1){
		fcs = FCS(fcs,c);
		cp = putbyte(cp,c);
	}
	free_p(bp);	/* Shouldn't be necessary */
	fcs ^= 0xffff;
	cp = putbyte(cp,fcs);
	cp = putbyte(cp,fcs >> 8);
	*cp++ = HDLC_FLAG;

	obp->cnt = cp - obp->data;
	return obp;
}

/** ********************************************************
 *
 ***********************************************************/
static char * putbyte(char *cp, char c)
{
	switch(c){
	case HDLC_FLAG:
	case HDLC_ESC_ASYNC:
		*cp++ = HDLC_ESC_ASYNC;
		*cp++ = c ^ HDLC_ESC_COMPL;
		break;
	default:
		*cp++ = c;
	}
	return cp;
}
