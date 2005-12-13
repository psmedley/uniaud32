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
#include <sound/driver.h>
#include <sound/asound.h>

#define LINUX
#include <ossidc.h>
#include <stacktoflat.h>
#include <dbgos2.h>
#include <osspci.h>

struct pci_dev pci_devices[MAX_PCI_DEVICES] = {0};
struct pci_bus pci_busses[MAX_PCI_BUSSES] = {0};

HRESMGR hResMgr = 0;

BOOL    fSuspended = FALSE;
extern int nrCardsDetected;
//******************************************************************************
//******************************************************************************
OSSRET OSS32_APMResume()
{
    int i;
    struct pci_driver *driver;

    dprintf(("OSS32_APMResume"));
    for(i=0;i<MAX_PCI_DEVICES;i++)
    {
        if(pci_devices[i].devfn)
        {
            driver = pci_devices[i].pcidriver;
            if(driver && driver->resume) {
                driver->resume(&pci_devices[i]);
                fSuspended = FALSE;
            }
        }
    }
    fSuspended = FALSE;
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

#define PCI_CONFIG_ENABLE       0x80000000
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC
/*
 * get number of devices of given PCIID
 */
int pci_get_dev_num(ULONG pciId)
{
    ULONG devNr, busNr, funcNr, temp, cfgaddrreg, detectedId;
    int found = 0;

    cfgaddrreg = inl(PCI_CONFIG_ADDRESS);
    for(busNr=0;busNr<MAX_PCI_BUSSES;busNr++)     //BusNumber<255
    {
        for(devNr=0;devNr<32;devNr++)
        {
            for(funcNr=0;funcNr<8;funcNr++)
            {
                temp = ((ULONG)((ULONG)devNr<<11UL) + ((ULONG)busNr<<16UL) + ((ULONG)funcNr << 8UL));

                outl(PCI_CONFIG_ENABLE|temp, PCI_CONFIG_ADDRESS);
                detectedId = inl(PCI_CONFIG_DATA);
//                printk("det: %x, need: %x\n", detectedId, pciId);
                if(detectedId == pciId)
                    found++;
            }
        }
   }

    if(!found) {
        outl(cfgaddrreg, PCI_CONFIG_ADDRESS);
        return 0;
    }
    return found;
}
//******************************************************************************
//TODO: Doesn't completely fill in the pci_dev structure
//******************************************************************************
int FindPCIDevice(unsigned int vendor, unsigned int device, struct pci_dev near *pcidev, int idx)
{
    IDC_RESOURCE idcres;
    int i, residx = 0;

    pcidev->prepare    = pcidev_prepare;
    pcidev->activate   = pcidev_activate;
    pcidev->deactivate = pcidev_deactivate;
    pcidev->active     = 1;
    pcidev->ro         = 0;
    pcidev->sibling    = NULL;
    pcidev->next       = NULL;
    pcidev->vendor     = vendor;
    pcidev->device     = device;
    pcidev->dma_mask   = 0xFFFFFFFF;


    //    printk("FindPCIDevice vend: %x, id: %x\n",vendor, device);
    hResMgr = 0;
    hResMgr = RMFindPCIDevice(vendor, device, &idcres, idx);
    if(hResMgr == 0) {
        return FALSE;
    }
    pcidev->devfn = idcres.devfn;
    //pcidev->devfn = idcres.devfn + 1;

    for(i=0;i<MAX_RES_IO;i++) {
        if(idcres.io[i] != 0xffff) {
            pcidev->resource[residx].name  = 0;
            pcidev->resource[residx].child = 0;
            pcidev->resource[residx].sibling = 0;
            pcidev->resource[residx].parent = 0;
            pcidev->resource[residx].start = idcres.io[i];
            pcidev->resource[residx].end   = idcres.io[i] + idcres.iolength[i]; //inclusive??
            pcidev->resource[residx].flags = IORESOURCE_IO | PCI_BASE_ADDRESS_SPACE_IO;

            residx++;
        }
    }
    for(i=0;i<MAX_RES_MEM;i++) {
        if(idcres.mem[i] != 0xffffffff) {
            pcidev->resource[residx].name  = 0;
            pcidev->resource[residx].child = 0;
            pcidev->resource[residx].sibling = 0;
            pcidev->resource[residx].parent = 0;
            pcidev->resource[residx].start = idcres.mem[i];
            pcidev->resource[residx].end   = idcres.mem[i] + idcres.memlength[i]; //inclusive??
            pcidev->resource[residx].flags = IORESOURCE_MEM | IORESOURCE_MEM_WRITEABLE;
            printk("residx: %i start: %x\n", residx, pcidev->resource[residx].start);
            residx++;
        }
    }
    for(i=0;i<MAX_RES_DMA;i++) {
        if(idcres.dma[i] != 0xffff) {
            pcidev->dma_resource[i].name  = 0;
            pcidev->dma_resource[i].child = 0;
            pcidev->dma_resource[i].sibling = 0;
            pcidev->dma_resource[i].parent = 0;
            pcidev->dma_resource[i].start = idcres.dma[i];
            pcidev->dma_resource[i].end   = idcres.dma[i];
            //todo: 8/16 bits
            pcidev->dma_resource[i].flags = IORESOURCE_DMA;
        }
    }
    for(i=0;i<MAX_RES_IRQ;i++) {
        if(idcres.irq[i] != 0xffff) {
            pcidev->irq_resource[i].name  = 0;
            pcidev->irq_resource[i].child = 0;
            pcidev->irq_resource[i].sibling = 0;
            pcidev->irq_resource[i].parent = 0;
            pcidev->irq_resource[i].start = idcres.irq[i];
            pcidev->irq_resource[i].end   = idcres.irq[i];
            //todo: irq flags
            pcidev->irq_resource[9].flags = IORESOURCE_IRQ;
        }
    }
    if(pcidev->irq_resource[0].start != 0xffff) {
         pcidev->irq = pcidev->irq_resource[0].start;
    }
    else pcidev->irq = 0;

    if(idcres.busnr > MAX_PCI_BUSSES) {
        DebugInt3();
        return FALSE;
    }
    pcidev->bus = &pci_busses[idcres.busnr];
    pcidev->bus->number = idcres.busnr;

    pci_read_config_word(pcidev, PCI_SUBSYSTEM_VENDOR_ID, &pcidev->subsystem_vendor);
    pci_read_config_word(pcidev, PCI_SUBSYSTEM_ID, &pcidev->subsystem_device);

    return TRUE;
}
//******************************************************************************
//******************************************************************************
struct pci_dev *pci_find_device (unsigned int vendor, unsigned int device, struct pci_dev *from)
{
    int i, idx;
    HRESMGR hResMgrTmp = 0;

    if((int)from < 8) {
        idx = (int)from; // dirty hack
//        return 0;
    } else
        idx = 0;
//    printk("searching device. vendor %x, pci id %x, idx %i\n",vendor,device, idx);
    //not very pretty
    if(hResMgr) {
        hResMgrTmp = hResMgr;
        hResMgr    = 0;
    }
    for(i=0;i<MAX_PCI_DEVICES;i++)
    {
        if(pci_devices[i].devfn == 0)
        {
            if(FindPCIDevice(vendor, device, (struct pci_dev near *)&pci_devices[i], idx) == TRUE) {
                if(hResMgrTmp) {
                    RMDestroy(hResMgr);
                    hResMgr = hResMgrTmp;
                }
//                pci_read_config_dword(&pci_devices[i], PCI_CLASS_REVISION, &pci_devices[i]._class);
//                if (((pci_devices[i]._class >> 8) & 0xffff) == PCI_CLASS_MULTIMEDIA_AUDIO)
                    return &pci_devices[i];
            }
#ifdef DEBUG
                printk("wrong device. vendor %x, pci id %x\n",vendor,device);
#endif
            break;
        }
    }
    if(hResMgrTmp) {
        RMDestroy(hResMgr);
        hResMgr = hResMgrTmp;
    }
    return 0;
}
//******************************************************************************
//******************************************************************************
struct resource * __request_region(struct resource *a, unsigned long start, unsigned long n, const char *name)
{
	struct resource *resource;

    if(a->flags & IORESOURCE_MEM) {
        if(RMRequestMem(hResMgr, start, n) == FALSE) {
            printk("RMRequestIO failed for io %x, length %x\n", start, n);
            return NULL;
        }
    }
    else
    if(a->flags & IORESOURCE_IO) {
        if(RMRequestIO(hResMgr, start, n) == FALSE) {
            printk("RMRequestIO failed for io %x, length %x\n", start, n);
            return NULL;
        }
    }

	resource = kmalloc(sizeof(struct resource), GFP_KERNEL);
	if (resource == NULL)
		return NULL;
	resource->name  = name;
	resource->start = start;
	resource->end   = start + n - 1;
	resource->flags = a->flags;
	return resource;
}
//******************************************************************************
//******************************************************************************
void __release_region(struct resource *resource, unsigned long b, unsigned long c)
{
    if(resource) {
        kfree(resource);
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
#define CONFIG_CMD(dev, where)   (0x80000000 | (dev->bus->number << 16) | (dev->devfn << 8) | (where & ~3))
//******************************************************************************
int pci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    *value = inb(0xCFC + (where&3));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    *value = inw(0xCFC + (where&2));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    *value = inl(0xCFC);
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    outb(value, 0xCFC + (where&3));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    outw(value, 0xCFC + (where&2));
    return PCIBIOS_SUCCESSFUL;
}
//******************************************************************************
//******************************************************************************
int pci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
    outl(CONFIG_CMD(dev,where), 0xCF8);
    outl(value, 0xCFC);
    return PCIBIOS_SUCCESSFUL;
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
    struct pci_dev *pcidev = NULL;
    int i = 0, j;
    int dev_num;
    ULONG pcidevid;

    while(driver->id_table[i].vendor)
    {
        pcidevid = ((ULONG)driver->id_table[i].device << 16) |
            (ULONG)driver->id_table[i].vendor;
        dev_num = pci_get_dev_num(pcidevid);

        if (dev_num) {
            printk("found: %i number of %x, id: %x\n",dev_num,driver->id_table[i].vendor, driver->id_table[i].device);

            for (j=0; j < dev_num; j++) // dirty hack for multiply cards with same PCIID
            {
                pcidev = pci_find_device(driver->id_table[i].vendor, driver->id_table[i].device, (struct pci_dev *)j);
                //        printk("checking pci vend: %x, id: %x %i\n",driver->id_table[i].vendor, driver->id_table[i].device, i);
#ifdef DEBUG
                //        dprintf(("checked device: vendor %x, pci id %x, pcidev %x, probe %x",
                //                 driver->id_table[i].vendor,driver->id_table[i].device, pcidev, driver->probe));
#endif
                if(pcidev && driver->probe) {
                    printk("found: %x, id: %x idx %i\n",driver->id_table[i].vendor, driver->id_table[i].device, j);

                    if(driver->probe(pcidev, &driver->id_table[i]) == 0) {
                        //remove resource manager object for this device and
                        //register resources with RM
                        RMFinialize(hResMgr);
                        hResMgr = 0;

                        //save driver pointer for suspend/resume calls
                        pcidev->pcidriver = (void *)driver;
                        pcidev->current_state = 4;
                        //return 1;
                    }
                    else pcidev->devfn = 0;

                    RMDestroy(hResMgr);
                    hResMgr = 0;
                }
            } // for j
            return dev_num;
        }
        i++;
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
    nrCardsDetected+=res;
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
