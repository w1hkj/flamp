/* AX25 header conversion routines
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

/** ********************************************************
 * \brief Convert a host-format AX.25 header into a mbuf 
 * ready for transmission
 ***********************************************************/
struct mbuf * htonax25(struct ax25 *hdr, struct mbuf *bp)
{
	register char *cp;
	register uint16 i;

	if(hdr == (struct ax25 *)NULL || hdr->ndigis > MAXDIGIS)
		return NULLBUF;

	/* Allocate space for return buffer */
	i = AXALEN * (2 + hdr->ndigis);
	bp = pushdown(bp,i);

	/* Now convert */
	cp = bp->data;		/* cp -> dest field */

	/* Generate destination field */
	memcpy(cp,hdr->dest,AXALEN);
	if(hdr->cmdrsp == LAPB_COMMAND)
		cp[ALEN] |= C;	/* Command frame sets C bit in dest */
	else
		cp[ALEN] &= ~C;
	cp[ALEN] &= ~E;	/* Dest E-bit is always off */

	cp += AXALEN;		/* cp -> source field */

	/* Generate source field */
	memcpy(cp,hdr->source,AXALEN);
	if(hdr->cmdrsp == LAPB_RESPONSE)
		cp[ALEN] |= C;
	else
		cp[ALEN] &= ~C;
	/* Set E bit on source address if no digis */
	if(hdr->ndigis == 0){
		cp[ALEN] |= E;
		return bp;
	}

	cp += AXALEN;		/* cp -> first digi field */

	/* All but last digi get copied with E bit off */
	for(i=0; i < hdr->ndigis; i++){
		memcpy(cp,hdr->digis[i],AXALEN);
		if(i < hdr->ndigis - 1)
			cp[ALEN] &= ~E;
		else
			cp[ALEN] |= E;	/* Last digipeater has E bit set */
		if(i < hdr->nextdigi)
			cp[ALEN] |= REPEATED;
		else
			cp[ALEN] &= ~REPEATED;
		cp += AXALEN;		/* cp -> next digi field */
	}
	return bp;
}

/** ********************************************************
 * \brief Convert a network-format AX.25 header into a host format structure
 * Return -1 if error, number of addresses if OK
 ***********************************************************/
int ntohax25(struct ax25 *hdr, struct mbuf **bpp)
{
	register char *axp;

	if(pullup(bpp,hdr->dest,AXALEN) < AXALEN)
		return -1;

	if(pullup(bpp,hdr->source,AXALEN) < AXALEN)
		return -1;

	/* Process C bits to get command/response indication */
	if((hdr->source[ALEN] & C) == (hdr->dest[ALEN] & C))
		hdr->cmdrsp = LAPB_UNKNOWN;
	else if(hdr->source[ALEN] & C)
		hdr->cmdrsp = LAPB_RESPONSE;
	else
		hdr->cmdrsp = LAPB_COMMAND;

	hdr->ndigis = 0;
	hdr->nextdigi = 0;
	if(hdr->source[ALEN] & E)
		return 2;	/* No digis */

	/* Count and process the digipeaters */
	axp = hdr->digis[0];
	while(hdr->ndigis < MAXDIGIS && pullup(bpp,axp,AXALEN) == AXALEN){
		hdr->ndigis++;
		if(axp[ALEN] & REPEATED)
			hdr->nextdigi++;
		if(axp[ALEN] & E)	/* Last one */
			return hdr->ndigis + 2;			
		axp += AXALEN;
	}
	return -1;	/* Too many digis */
}

