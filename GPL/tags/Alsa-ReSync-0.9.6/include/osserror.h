/* $Id: osserror.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * OSS error code definitions
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
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


#ifndef __OSSERROR_H__
#define __OSSERROR_H__

#define OSSERR_SUCCESS				0
#define OSSERR_OUT_OF_MEMORY			1
#define OSSERR_INVALID_STREAMID			2
#define OSSERR_NO_DEVICE_AVAILABLE		3
#define OSSERR_INVALID_PARAMETER 		4
#define OSSERR_BUSY				5
#define OSSERR_AGAIN				6
#define OSSERR_IO_ERROR				7
#define OSSERR_INIT_FAILED                      8
#define OSSERR_ACCESS_DENIED                    9
#define OSSERR_BUFFER_FULL                      10
#define OSSERR_NOT_SUPPORTED                    11
#define OSSERR_UNKNOWN          		255

#endif
