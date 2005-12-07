/*
 *   ALSA sequencer /proc interface
 *   Copyright (c) 1998 by Frank van de Pol <fvdpol@home.nl>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <sound/driver.h>

#include "seq_info.h"
#include "seq_clientmgr.h"
#include "seq_timer.h"


static snd_info_entry_t *snd_seq_info_queues_entry;
static snd_info_entry_t *snd_seq_info_clients_entry;
static snd_info_entry_t *snd_seq_info_memory_entry;
static snd_info_entry_t *snd_seq_info_timer_entry;


/*---------------------------------------------------------------------------*/

static int __init snd_seq_info_timer_init(char *name)
{
	snd_info_entry_t *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, name, snd_seq_root);
	if (entry == NULL)
		return -ENOMEM;
	entry->content = SNDRV_INFO_CONTENT_TEXT;
	entry->c.text.read_size = 1024;
	entry->c.text.read = snd_seq_info_timer_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	snd_seq_info_timer_entry = entry;
	return 0;
}

static int __exit snd_seq_info_timer_done(void)
{
	if (snd_seq_info_timer_entry)
		snd_info_unregister(snd_seq_info_timer_entry);
	return 0;
}


/*---------------------------------------------------------------------------*/

static int __init snd_seq_info_memory_init(char *name)
{
	snd_info_entry_t *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, name, snd_seq_root);
	if (entry == NULL)
		return -ENOMEM;
	entry->content = SNDRV_INFO_CONTENT_TEXT;
	entry->c.text.read_size = 1024;
	entry->c.text.read = snd_seq_info_memory_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	snd_seq_info_memory_entry = entry;
	return 0;
}

static int __exit snd_seq_info_memory_done(void)
{
	if (snd_seq_info_memory_entry)
		snd_info_unregister(snd_seq_info_memory_entry);
	return 0;
}

/*---------------------------------------------------------------------------*/

static int __init snd_seq_info_clients_init(char *name)
{
	snd_info_entry_t *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, name, snd_seq_root);
	if (entry == NULL)
		return -ENOMEM;
	entry->content = SNDRV_INFO_CONTENT_TEXT;
	entry->c.text.read_size = 512 + (256 * SNDRV_SEQ_MAX_CLIENTS);
	entry->c.text.read = snd_seq_info_clients_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	snd_seq_info_clients_entry = entry;
	return 0;
}

static int __exit snd_seq_info_clients_done(void)
{
	if (snd_seq_info_clients_entry)
		snd_info_unregister(snd_seq_info_clients_entry);
	return 0;
}


/*---------------------------------------------------------------------------*/


static int __init snd_seq_info_queues_init(char *name)
{
	snd_info_entry_t *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, name, snd_seq_root);
	if (entry == NULL)
		return -ENOMEM;
	entry->content = SNDRV_INFO_CONTENT_TEXT;
	entry->c.text.read_size = 512 + (512 * SNDRV_SEQ_MAX_QUEUES);
	entry->c.text.read = snd_seq_info_queues_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	snd_seq_info_queues_entry = entry;
	return 0;
}

static int __exit snd_seq_info_queues_done(void)
{
	if (snd_seq_info_queues_entry)
		snd_info_unregister(snd_seq_info_queues_entry);
	return 0;
}

/*---------------------------------------------------------------------------*/


/* create all our /proc entries */
int __init snd_seq_info_init(void)
{
	snd_seq_info_queues_init("queues");
	snd_seq_info_clients_init("clients");
	snd_seq_info_memory_init("memory");
	snd_seq_info_timer_init("timer");
	return 0;
}

int __exit snd_seq_info_done(void)
{
	snd_seq_info_memory_done();
	snd_seq_info_clients_done();
	snd_seq_info_queues_done();
	snd_seq_info_timer_done();
	return 0;
}
