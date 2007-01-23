/* $Id: version.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for version string
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
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

#ifndef __UNIAUDVERSION_H__
#define __UNIAUDVERSION_H__


#define	BUILDER_NAME            "Netlabs"
#define	PRODUCT_NAME            "OS/2 Universal Audio Driver"
#define	VENDOR_NAME             "Netlabs"
#define	PRODUCT_VERSION		"1.1.4RC2"         // version
#define	PRODUCT_TIMESTAMP	20061125L       // YYYYMMDD


#define ALSA_VERSION            "1.1.4RC2"
#define RM_DRIVER_NAME          "UNIAUD32.SYS"
#define RM_DRIVER_DESCRIPTION   "OS/2 Universal Audio Driver"
#define RM_ADAPTER_NAME         "OS/2 Universal Audio"
#define RM_DRIVER_VENDORNAME    "InnoTek Systemberatung GmbH"
#define RM_DRIVER_BUILDYEAR     (PRODUCT_TIMESTAMP / 10000)
#define RM_DRIVER_BUILDMONTH    ((PRODUCT_TIMESTAMP / 100) % 100)
#define RM_DRIVER_BUILDDAY      (PRODUCT_TIMESTAMP % 100)
#define UNIAUD_VERSION          114


#endif //__UNIAUDVERSION_H__
