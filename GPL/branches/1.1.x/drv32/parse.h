/*
 * Config.sys parameter parsing
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
 */

#ifndef PARSE_INCLUDED
#define PARSE_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define SIZE_CONFIG_LINE   256

// True if the /V parameter was specified
extern int fVerbose;
extern int fDebug;

int GetParms(char FAR48 *pszCmdLine);

#ifdef __cplusplus
}
#endif

#endif
