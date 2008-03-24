/* $Id: pci.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of Linux PCI functions (using direct port I/O)
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * Parts based on Linux kernel sources
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#include "linux.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <asm/io.h>
#include <sound/config.h>
#include <sound/core.h>
#include <sound/asound.h>

#define LINUX
#include <ossidc.h>
#include <stacktoflat.h>
#include <dbgos2.h>
#include <osspci.h>

#define	MAX_PCI_BUSSES		16
#define	MAX_PCI_DEVICES		16

struct pci_dev pci_devices[MAX_PCI_DEVICES] = {0};
struct pci_bus pci_busses[MAX_PCI_BUSSES] = {0};

BOOL    fSuspended = FALSE;
extern int nrCardsDetected;


#define PCI_CONFIG_ENABLE       0x80000000
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC


//******************************************************************************
#define CONFIG_CMD(dev, where)	\
    (PCI_CONFIG_ENABLE | (dev->bus->number<<16) | (dev->devfn<<8) | (where & ~3))
//******************************************************************************
int pci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    *value = inb(PCI_CONFIG_DATA + (where&3));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    *value = inw(PCI_CONFIG_DATA + (where&2));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    *value = inl(PCI_CONFIG_DATA);
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    outb(value, PCI_CONFIG_DATA + (where&3));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    outw(value, PCI_CONFIG_DATA + (where&2));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
    outl(CONFIG_CMD(dev,where), PCI_CONFIG_ADDRESS);
    outl(value, PCI_CONFIG_DATA);
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pcidev_prepare(struct pci_dev *dev)
{
    dprintf(("pcidev_prepare %x not implemented", dev));
    return 1; //todo: correct return value??
}
//******************************************************************************
//******************************************************************************
int pcidev_activate(struct pci_dev *dev)
{
    dprintf(("pcidev_activate %x not implemented", dev));
    return 1; //todo: correct return value??
}
//******************************************************************************
//******************************************************************************
int pcidev_deactivate(struct pci_dev *dev)
{
    dprintf(("pcidev_deactivate %x not implemented", dev));
    return 1; //todo: correct return value??
}



//******************************************************************************
//******************************************************************************
static int pci_query_device(unsigned int vendor, unsigned int device,
                            struct pci_dev near *pcidev, int idx)
{
    int		resNo, addr, found = 0;
    u32		devNr, busNr, funcNr, detectedId, pciId, cfgaddrreg, temp, temp2;
    u8		headerType;

    pciId = (device << 16) | vendor;

    cfgaddrreg = inl(PCI_CONFIG_ADDRESS);
    for(busNr=0;busNr<MAX_PCI_BUSSES;busNr++)     //BusNumber<255
    {
        for(devNr=0;devNr<32;devNr++)
        {
            for(funcNr=0;funcNr<8;funcNr++)
            {
                headerType = 0;
                temp = PCI_CONFIG_ENABLE | (busNr<<16) | (devNr<<11) | (funcNr<<8);
                outl(temp, PCI_CONFIG_ADDRESS);
                detectedId = inl(PCI_CONFIG_DATA);
                if( detectedId != 0xffffffff )
                {
                    outl(temp | (PCI_HEADER_TYPE & ~3), PCI_CONFIG_ADDRESS);
                    headerType = inb(PCI_CONFIG_DATA + (PCI_HEADER_TYPE & 3));
                }
                //		printk("det: %x (%x), need: %x\n", detectedId, headerType, pciId);

                if( detectedId == pciId &&
                   (headerType & 0x7f) == PCI_HEADER_TYPE_NORMAL )
                {
                    if( found++ == idx )
                    {
                        memset((void near *)pcidev, 0, sizeof(struct pci_dev));

                        pcidev->vendor      = vendor;
                        pcidev->device      = device;
                        pcidev->bus         = &pci_busses[busNr];
                        pcidev->bus->number = busNr;
                        pcidev->devfn       = (devNr << 3) | funcNr;
                        pcidev->hdr_type    = headerType & 0x7f;

                        pcidev->prepare    = pcidev_prepare;
                        pcidev->activate   = pcidev_activate;
                        pcidev->deactivate = pcidev_deactivate;
                        pcidev->active     = 1;
                        pcidev->ro         = 0;
                        pcidev->sibling    = NULL;
                        pcidev->next       = NULL;
                        pcidev->dma_mask   = 0xFFFFFFFF;

                        // Subsystem ID
                        pci_read_config_word(pcidev, PCI_SUBSYSTEM_VENDOR_ID,
                                             &pcidev->subsystem_vendor);
                        pci_read_config_word(pcidev, PCI_SUBSYSTEM_ID,
                                             &pcidev->subsystem_device);

                        // I/O  and MEM
                        resNo = 0;
                        for( addr = PCI_BASE_ADDRESS_0; addr <= PCI_BASE_ADDRESS_5; addr += 4 )
                        {
                            pci_read_config_dword(pcidev, addr, &temp);
                            if( temp != 0 && temp != 0xffffffff )
                            {
                                pci_write_config_dword(pcidev, addr, 0xffffffff);
                                pci_read_config_dword(pcidev, addr, &temp2);
                                pci_write_config_dword(pcidev, addr, temp);

                                if( temp & PCI_BASE_ADDRESS_SPACE_IO )
                                {
                                    pcidev->resource[resNo].flags = IORESOURCE_IO | PCI_BASE_ADDRESS_SPACE_IO;
                                    pcidev->resource[resNo].start = temp & PCI_BASE_ADDRESS_IO_MASK;
                                    pcidev->resource[resNo].end   = pcidev->resource[resNo].start +
                                        ~(temp2 & PCI_BASE_ADDRESS_IO_MASK) + 1;
                                }
                                else
                                {
                                    pcidev->resource[resNo].flags = IORESOURCE_MEM | IORESOURCE_MEM_WRITEABLE;
                                    pcidev->resource[resNo].start = temp & PCI_BASE_ADDRESS_MEM_MASK;
                                    pcidev->resource[resNo].end   = pcidev->resource[resNo].start +
                                        ~(temp2 & PCI_BASE_ADDRESS_MEM_MASK) + 1;
                                }

                                resNo++;

                            }
                        }

                        // IRQ and PIN
                        pci_read_config_dword(pcidev, PCI_INTERRUPT_LINE, &temp);
                        if( (u8)temp && (u8)temp != 0xff )
                        {
                            pcidev->irq_resource[0].flags = IORESOURCE_IRQ;
                            pcidev->irq_resource[0].start =
                                pcidev->irq_resource[0].end   = temp & 0xffff;
                            pcidev->irq = (u8)temp;
                        }

                        return 1;
                    }
                }

                // don't need to check more, if function 0 not present or single
                if( funcNr == 0 && !(headerType & 0x80) )		break;
            }
        }
    }
    outl(cfgaddrreg, PCI_CONFIG_ADDRESS);
    return 0;

}

//******************************************************************************
//******************************************************************************
struct pci_dev *pci_find_device (unsigned int vendor, unsigned int device, struct pci_dev *from)
{
    int i, idx;

    if((int)from < 8) {
        idx = (int)from; // dirty hack
        //        return 0;
    } else
        idx = 0;

    for(i=0;i<MAX_PCI_DEVICES;i++)
    {
        if(pci_devices[i].devfn == 0)
        {
            if( pci_query_device(vendor, device, (struct pci_dev near *)&pci_devices[i], idx) )
                return &pci_devices[i];
            else
                break;
        }
    }

    return NULL;
}
//******************************************************************************
//******************************************************************************
struct resource * __request_region(struct resource *a, unsigned long start,
                                   unsigned long n, const char *name)
{
    struct resource *resource;

    if(a->flags & IORESOURCE_MEM) {
        if(RMRequestMem(/*hResMgr,*/ start, n) == FALSE) {
            printk("RMRequestIO failed for io %x, length %x\n", start, n);
            return NULL;
        }
    }
    else if(a->flags & IORESOURCE_IO) {
        if(RMRequestIO(/*hResMgr,*/ start, n) == FALSE) {
            printk("RMRequestIO failed for io %x, length %x\n", start, n);
            return NULL;
        }
    }

    resource = kmalloc(sizeof(struct resource), GFP_KERNEL);
    if (resource == NULL)
        return NULL;
    resource->name  = name;
    resource->start = start;
    resource->end     = start + n; // - 1;
    resource->flags = a->flags;
    resource->parent  =
        resource->child   = NULL;

    // insert in list
    resource->sibling = a->sibling;
    a->sibling = resource;

    return resource;
}
//******************************************************************************
//******************************************************************************
void __release_region(struct resource *a,
                      unsigned long start, unsigned long n)
{
    struct resource	*resource;
    struct resource	**ppres = &a->sibling;
    unsigned long	end = start + n; // - 1;

    while( *ppres )
    {
        resource = *ppres;

        if( resource->start == start && resource->end == end )
        {
            // remove from list
            *ppres = resource->sibling;
            kfree(resource);
            return;
        }

        ppres = &resource->sibling;
    }
}
//******************************************************************************
//******************************************************************************
int pci_get_flags (struct pci_dev *dev, int n_base)
{
    if(n_base >= DEVICE_COUNT_RESOURCE || !dev->resource[n_base].flags) {
        DebugInt3();
        return 0;
    }
    return dev->resource[n_base].flags;
}
//******************************************************************************
//******************************************************************************
int pcibios_present(void)
{
    printk("pcibios_present -> pretend BIOS present\n");
    return 1;
}
//******************************************************************************
//******************************************************************************
struct pci_dev *pci_find_slot (unsigned int bus, unsigned int devfn)
{
    printk("pci_find_slot %d %x not implemented!!\n", bus, devfn);
    DebugInt3();
    return NULL;
}
//******************************************************************************
//******************************************************************************
int pci_dma_supported(struct pci_dev *dev, unsigned long mask)
{
    printk("pci_dma_supported: return TRUE\n");
    return 1;
}
//******************************************************************************
//******************************************************************************
int pci_find_capability(struct pci_dev *dev, int cap)
{
    u16 status;
    u8 pos, id;
    int ttl = 48;

    pci_read_config_word(dev, PCI_STATUS, &status);
    if (!(status & PCI_STATUS_CAP_LIST))
        return 0;
    pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &pos);
    while (ttl-- && pos >= 0x40) {
        pos &= ~3;
        pci_read_config_byte(dev, pos + PCI_CAP_LIST_ID, &id);
        if (id == 0xff)
            break;
        if (id == cap)
            return pos;
        pci_read_config_byte(dev, pos + PCI_CAP_LIST_NEXT, &pos);
    }
    return 0;
}
//******************************************************************************
/*
 *  Set power management state of a device.  For transitions from state D3
 *  it isn't as straightforward as one could assume since many devices forget
 *  their configuration space during wakeup.  Returns old power state.
 */
//******************************************************************************
int pci_set_power_state(struct pci_dev *dev, int new_state)
{
    u32 base[5], romaddr;
    u16 pci_command, pwr_command;
    u8  pci_latency, pci_cacheline;
    int i, old_state;
    int pm = pci_find_capability(dev, PCI_CAP_ID_PM);

    if (!pm)
        return 0;
    pci_read_config_word(dev, pm + PCI_PM_CTRL, &pwr_command);
    old_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
    if (old_state == new_state)
        return old_state;
    if (old_state == 3) {
        pci_read_config_word(dev, PCI_COMMAND, &pci_command);
        pci_write_config_word(dev, PCI_COMMAND, pci_command & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
        for (i = 0; i < 5; i++)
            pci_read_config_dword(dev, PCI_BASE_ADDRESS_0 + i*4, &base[i]);
        pci_read_config_dword(dev, PCI_ROM_ADDRESS, &romaddr);
        pci_read_config_byte(dev, PCI_LATENCY_TIMER, &pci_latency);
        pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &pci_cacheline);
        pci_write_config_word(dev, pm + PCI_PM_CTRL, new_state);
        for (i = 0; i < 5; i++)
            pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + i*4, base[i]);
        pci_write_config_dword(dev, PCI_ROM_ADDRESS, romaddr);
        pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
        pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, pci_cacheline);
        pci_write_config_byte(dev, PCI_LATENCY_TIMER, pci_latency);
        pci_write_config_word(dev, PCI_COMMAND, pci_command);
    } else
        pci_write_config_word(dev, pm + PCI_PM_CTRL, (pwr_command & ~PCI_PM_CTRL_STATE_MASK) | new_state);
    return old_state;
}
//******************************************************************************
/*
 *  Initialize device before it's used by a driver. Ask low-level code
 *  to enable I/O and memory. Wake up the device if it was suspended.
 *  Beware, this function can fail.
 */
//******************************************************************************
int pci_enable_device(struct pci_dev *dev)
{
    u16 pci_command;

    printk("pci_enable_device %x\n", dev);

    pci_read_config_word(dev, PCI_COMMAND, &pci_command);
    pci_write_config_word(dev, PCI_COMMAND, pci_command | (PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
    pci_set_power_state(dev, 0);
    return 0;
}
//******************************************************************************
//******************************************************************************
int pci_register_driver(struct pci_driver *driver)
{
    int i, j, dev_num;
    struct pci_dev *pcidev;

    for( i = 0; driver->id_table[i].vendor; i++)
    {
        dev_num = 0;
        while( (pcidev = pci_find_device(driver->id_table[i].vendor,
                                         driver->id_table[i].device,
                                         (struct pci_dev *)dev_num)) != NULL )
        {
            RMInit();
            if( driver->probe) {
                printk("found: %x, id: %x idx %i\n",driver->id_table[i].vendor, driver->id_table[i].device, dev_num);

                if(driver->probe(pcidev, &driver->id_table[i]) == 0) {
                    pcidev->pcidriver = (void *)driver;
                    pcidev->current_state = 4;

                    // create adapter
                    RMDone((driver->id_table[i].device << 16) | driver->id_table[i].vendor);
                    nrCardsDetected++;
                    return 1;
                }
                else pcidev->devfn = 0;
            }

            RMDone(0);

            dev_num++;

        }
    }
    return 0;
}
//******************************************************************************
//******************************************************************************
int pci_module_init(struct pci_driver *drv)
{
    int res = pci_register_driver(drv);
    if (res < 0)
        return res;
    if (res == 0)
        return -ENODEV;
    return 0;
}
//******************************************************************************
//******************************************************************************
int pci_unregister_driver(struct pci_driver *driver)
{
    struct pci_dev *pcidev;
    int i = 0, j;

    while(driver->id_table[i].vendor)
    {
        for(j=0;j<MAX_PCI_DEVICES;j++)
        {
            if(pci_devices[j].vendor == driver->id_table[i].vendor &&
               pci_devices[j].device == driver->id_table[i].device)
            {
                if(driver->remove) {
                    driver->remove(&pci_devices[j]);
                }
            }
        }
        i++;
    }
    return 0;
}
//******************************************************************************
//******************************************************************************
void pci_set_master(struct pci_dev *dev)
{
    u16 cmd;

    pci_read_config_word(dev, PCI_COMMAND, &cmd);
    if (! (cmd & PCI_COMMAND_MASTER)) {
        dprintf(("pci_set_master %x", dev));
        cmd |= PCI_COMMAND_MASTER;
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    }
    return;
}
//******************************************************************************
// * Register a device with power management
//******************************************************************************
struct pm_dev *pm_register(pm_dev_t type, unsigned long id,  pm_callback callback)
{
    dprintf(("pm_register STUB"));
    DebugInt3();
    return NULL;
}
//******************************************************************************
// * Unregister a device with power management
//******************************************************************************
void pm_unregister(struct pm_dev *dev)
{
    dprintf(("pm_unregister STUB"));
}
//******************************************************************************
//******************************************************************************
int __compat_get_order(unsigned long size)
{
    int order;

    size = (size-1) >> (PAGE_SHIFT-1);
    order = -1;
    do {
        size >>= 1;
        order++;
    } while (size);
    return order;
}
//******************************************************************************
//******************************************************************************
void *pci_alloc_consistent(struct pci_dev *hwdev,
                           long size, dma_addr_t *dma_handle)
{
    void *ret = NULL;
    int gfp = GFP_ATOMIC;
    int order;
#ifdef DEBUG
    dprintf(("pci_alloc_consistent %d mask %x", size, (hwdev) ? hwdev->dma_mask : 0));
#endif
    if (hwdev == NULL || hwdev->dma_mask != 0xffffffff) {
        //try not to exhaust low memory (< 16mb) so allocate from the high region first
        //if that doesn't satisfy the dma mask requirement, then get it from the low
        //regino anyway
        if(hwdev->dma_mask > 0x00ffffff) {
            order = __compat_get_order(size);
            ret = (void *)__get_free_pages(gfp|GFP_DMAHIGHMEM, order);
            *dma_handle = virt_to_bus(ret);
            if(*dma_handle > hwdev->dma_mask) {
                free_pages((unsigned long)ret, __compat_get_order(size));
                //be sure and allocate below 16 mb
                gfp |= GFP_DMA;
                ret = NULL;
            }
        }
        else { //must always allocate below 16 mb
            gfp |= GFP_DMA;
        }
    }
    if(ret == NULL) {
        ret = (void *)__get_free_pages(gfp, __compat_get_order(size));
    }

    if (ret != NULL) {
        memset(ret, 0, size);
        *dma_handle = virt_to_bus(ret);
    }
    return ret;
}
//******************************************************************************
//******************************************************************************
void pci_free_consistent(struct pci_dev *hwdev, long size,
                         void *vaddr, dma_addr_t dma_handle)
{
    free_pages((unsigned long)vaddr, __compat_get_order(size));
}
//******************************************************************************
//******************************************************************************
void pci_set_driver_data (struct pci_dev *dev, void *driver_data)
{
    if (dev)
        dev->driver_data = driver_data;
}
//******************************************************************************
//******************************************************************************
void *pci_get_driver_data (struct pci_dev *dev)
{
    if (dev)
        return dev->driver_data;
    return 0;
}
//******************************************************************************
//******************************************************************************
unsigned long pci_get_dma_mask (struct pci_dev *dev)
{
    if (dev)
        return dev->dma_mask;
    return 0;
}
//******************************************************************************
//******************************************************************************
void pci_set_dma_mask (struct pci_dev *dev, unsigned long mask)
{
    if (dev)
        dev->dma_mask = mask;
}
//******************************************************************************
//******************************************************************************
int release_resource(struct resource *newres)
{
    return 0;
}

//******************************************************************************
//******************************************************************************
int pci_set_latency_time(struct pci_dev *dev, int latency)
{
    pci_write_config_byte(dev, PCI_LATENCY_TIMER, latency);
    return 0;
}

//******************************************************************************
//******************************************************************************
/**
 * pci_save_state - save the PCI configuration space of a device before suspending
 * @dev: - PCI device that we're dealing with
 * @buffer: - buffer to hold config space context
 *
 * @buffer must be large enough to hold the entire PCI 2.2 config space
 * (>= 64 bytes).
 */
int pci_orig_save_state(struct pci_dev *dev, u32 *buffer)
{
    int i;
    if (buffer) {
        /* XXX: 100% dword access ok here? */
        for (i = 0; i < 16; i++)
            pci_read_config_dword(dev, i * 4,&buffer[i]);
    }
    return 0;
}

/**
 * pci_restore_state - Restore the saved state of a PCI device
 * @dev: - PCI device that we're dealing with
 * @buffer: - saved PCI config space
 *
 */
int
pci_orig_restore_state(struct pci_dev *dev, u32 *buffer)
{
    int i;

    if (buffer) {
        for (i = 0; i < 16; i++)
            pci_write_config_dword(dev,i * 4, buffer[i]);
    }
    /*
     * otherwise, write the context information we know from bootup.
     * This works around a problem where warm-booting from Windows
     * combined with a D3(hot)->D0 transition causes PCI config
     * header data to be forgotten.
     */
    else {
        for (i = 0; i < 6; i ++)
            pci_write_config_dword(dev,
                                   PCI_BASE_ADDRESS_0 + (i * 4),
                                   dev->resource[i].start);
        pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
    }
    return 0;
}

struct saved_config_tbl {
    struct pci_dev *pci;
    u32 config[16];
};
static struct saved_config_tbl saved_tbl[16];

void pci_save_state(struct pci_dev *pci)
{
    int i;
    /* FIXME: mutex needed for race? */
    for (i = 0; i < ARRAY_SIZE(saved_tbl); i++) {
        if (! saved_tbl[i].pci) {
            saved_tbl[i].pci = pci;
            pci_orig_save_state(pci, saved_tbl[i].config);
            return;
        }
    }
    printk(KERN_DEBUG "snd: no pci config space found!\n");
}

void pci_restore_state(struct pci_dev *pci)
{
    int i;
    /* FIXME: mutex needed for race? */
    for (i = 0; i < ARRAY_SIZE(saved_tbl); i++) {
        if (saved_tbl[i].pci == pci) {
            saved_tbl[i].pci = NULL;
            pci_orig_restore_state(pci, saved_tbl[i].config);
            return;
        }
    }
    printk(KERN_DEBUG "snd: no saved pci config!\n");
}

void pci_disable_device(struct pci_dev *dev)
{
    u16 pci_command;

    pci_read_config_word(dev, PCI_COMMAND, &pci_command);
    if (pci_command & PCI_COMMAND_MASTER) {
        pci_command &= ~PCI_COMMAND_MASTER;
        pci_write_config_word(dev, PCI_COMMAND, pci_command);
    }
}

int pci_request_region(struct pci_dev *pdev, int bar, char *res_name)
{
    int flags;

    if (pci_resource_len(pdev, bar) == 0)
        return 0;
    flags = pci_get_flags(pdev, bar);
    if (flags & IORESOURCE_IO) {
        if (check_region(pci_resource_start(pdev, bar), pci_resource_len(pdev, bar)))
            goto err_out;
        request_region(pci_resource_start(pdev, bar),
                       pci_resource_len(pdev, bar), res_name);
    }
    else if (flags & IORESOURCE_MEM) {
        if (check_mem_region(pci_resource_start(pdev, bar), pci_resource_len(pdev, bar)))
            goto err_out;
        request_mem_region(pci_resource_start(pdev, bar),
                           pci_resource_len(pdev, bar), res_name);
    }

    return 0;

err_out:
    printk(KERN_WARNING "PCI: Unable to reserve %s region #%d:%lx@%lx for device %s\n",
           flags & IORESOURCE_IO ? "I/O" : "mem",
           bar + 1, /* PCI BAR # */
           pci_resource_len(pdev, bar), pci_resource_start(pdev, bar),
           res_name);
    return -EBUSY;
}

void pci_release_region(struct pci_dev *pdev, int bar)
{
    int flags;

    if (pci_resource_len(pdev, bar) == 0)
        return;
    flags = pci_get_flags(pdev, bar);
    if (flags & IORESOURCE_IO) {
        release_region(pci_resource_start(pdev, bar),
                       pci_resource_len(pdev, bar));
    }
    else if (flags & IORESOURCE_MEM) {
        release_mem_region(pci_resource_start(pdev, bar),
                           pci_resource_len(pdev, bar));
    }
}

int pci_request_regions(struct pci_dev *pdev, char *res_name)
{
    int i;

    for (i = 0; i < 6; i++)
        if (pci_request_region(pdev, i, res_name))
            goto err;
    return 0;
    err:
        while (--i >= 0)
            pci_release_region(pdev, i);
        return -EBUSY;
}

void pci_release_regions(struct pci_dev *pdev)
{
    int i;
    for (i = 0; i < 6; i++)
        pci_release_region(pdev, i);
}

const struct pci_device_id * pci_match_device(const struct pci_device_id *ids, struct pci_dev *dev)
{
    u16 subsystem_vendor, subsystem_device;

    pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID, &subsystem_vendor);
    pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &subsystem_device);

    while (ids->vendor || ids->subvendor || ids->class_mask) {
        if ((ids->vendor == PCI_ANY_ID || ids->vendor == dev->vendor) &&
            (ids->device == PCI_ANY_ID || ids->device == dev->device) &&
            (ids->subvendor == PCI_ANY_ID || ids->subvendor == subsystem_vendor) &&
            (ids->subdevice == PCI_ANY_ID || ids->subdevice == subsystem_device) &&
            !((ids->class ^ dev->_class) & ids->class_mask))
            return ids;
        ids++;
    }
    return NULL;
}

int snd_pci_dev_present(const struct pci_device_id *ids)
{
    while (ids->vendor || ids->subvendor) {
        if (pci_find_device(ids->vendor, ids->subvendor, NULL))
            return 1;
        ids++;
    }
    return 0;
}

struct pci_driver_mapping {
    struct pci_dev *dev;
    struct pci_driver *drv;
    unsigned long dma_mask;
    void *driver_data;
    u32 saved_config[16];
};

#define PCI_MAX_MAPPINGS 64
static struct pci_driver_mapping drvmap [PCI_MAX_MAPPINGS] = { { NULL, } , };


static struct pci_driver_mapping *get_pci_driver_mapping(struct pci_dev *dev)
{
    int i;

    for (i = 0; i < PCI_MAX_MAPPINGS; i++)
        if (drvmap[i].dev == dev)
            return &drvmap[i];
    return NULL;
}

struct pci_driver *snd_pci_compat_get_pci_driver(struct pci_dev *dev)
{
    struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
    if (map)
        return map->drv;
    return NULL;
}
#if 0
void * pci_get_drvdata (struct pci_dev *dev)
{
    struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
    if (map)
        return map->driver_data;
    return NULL;
}


void pci_set_drvdata (struct pci_dev *dev, void *driver_data)
{
    struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
    if (map)
        map->driver_data = driver_data;
}
#endif


//******************************************************************************
//******************************************************************************
OSSRET OSS32_APMResume()
{
    int i;
    struct pci_driver *driver;

    dprintf(("OSS32_APMResume"));

    fSuspended = FALSE;

    for(i=0;i<MAX_PCI_DEVICES;i++)
    {
        if(pci_devices[i].devfn)
        {
            driver = pci_devices[i].pcidriver;
            if(driver && driver->resume) {
                driver->resume(&pci_devices[i]);
            }
        }
    }

    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_APMSuspend()
{
    int i;
    struct pci_driver *driver;

    dprintf(("OSS32_APMSuspend"));

    fSuspended = TRUE;

    for(i=0;i<MAX_PCI_DEVICES;i++)
    {
        if(pci_devices[i].devfn)
        {
            driver = pci_devices[i].pcidriver;
            if(driver && driver->suspend) {
                driver->suspend(&pci_devices[i], SNDRV_CTL_POWER_D3cold);
            }
        }
    }

    return OSSERR_SUCCESS;
}

