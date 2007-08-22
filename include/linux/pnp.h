/*
 * Linux Plug and Play Support Emulation for OS/2
 * This file is part of Uniaud32
 * Copyright Netlabs
 * Based on code taken from pnp.h in Linux 2.6.22
 * Copyright by Adam Belay <ambx1@neo.rr.com>
 *
 */

#ifndef _LINUX_PNP_H
#define _LINUX_PNP_H

struct pnp_card_link {
	struct pnp_card * card;
	struct pnp_card_driver * driver;
	void * driver_data;
	pm_message_t pm_state;
};

static inline void pnp_set_card_drvdata (struct pnp_card_link *pcard, void *data)
{
	pcard->driver_data = data;
}

#endif /* _LINUX_PNP_H */
