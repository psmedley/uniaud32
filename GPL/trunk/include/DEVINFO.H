/* $Id: DEVINFO.H,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * OS/2 definitions for logal and global info segments
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


// Prevent multiple inclusion
#if !defined(DevInfo_h)
#define DevInfo_h 1

#if !defined(__cplusplus)
#error C++ must be used for strict type checking
#endif


// Include the fundamental type definitions
#include "DevType.h"


// Global (per-system) information segment
struct        GINFO
  {
  WORD32      Time;           // Time in seconds since 1970.01.01
  WORD32      MilliSeconds;   // Time in milliseconds
  BYTE        Hour;           // Current hour
  BYTE        Minute;         // Current minute
  BYTE        Second;         // Current second
  BYTE        Hundredth;      // Current hundredth of a second
  WORD16      Timezone;       // Minutes from UTC (Greenwich mean-time)
  WORD16      Interval;       // Timer interval (tenths of milliseconds)
  BYTE        Day;            // Current day of month
  BYTE        Month;          // Current month
  WORD16      Year;           // Current year
  BYTE        Weekday;        // Current day of week (0 = Sunday, 6 = Saturday)
  BYTE        MajorVersion;   // Major version number of OS/2
  BYTE        MinorVersion;   // Minor version number of OS/2
  BYTE        Revision;       // Revision letter of OS/2
  BYTE        CurrentSession; // Current foreground full-screen session ID
  BYTE        MaxSessions;    // Maximum number of full-screen sessions
  BYTE        HugeShift;      // Shift count for huge segments
  BYTE        ProtectFlag;    // 1 = protected mode only, 0 = mixed mode
  WORD16      ForegroundPID;  // Process ID of the current foreground process
  BYTE        DynamicFlag;    // 1 = dynamic variation enabled, 0 = disabled
  BYTE        MaxWaitTime;    // Maximum wait to get control (seconds)
  WORD16      MinTimeSlice;   // Minimum time slice length (milliseconds)
  WORD16      MaxTimeSlice;   // Maximum time slice length (milliseconds)
  WORD16      BootDrive;      // ID of system startup drive (1 = A:, 26 = Z:)
  BYTE        Trace[32];      // System tracing flags (RAS)
  BYTE        MaxVIOSessions; // Maximum number of VIO (windowed) sessions
  BYTE        MaxPMSessions;  // Maximum number of PM sessions
  WORD16      ErrorLog;       // Error logging status
  WORD16      RASSelector;    // RAS memory mapped IO selector
  WORD32      RASAddress;     // RAS memory mapped IO address
  BYTE        MaxVDMSessions; // Maximum number of virtual DOS machines
  };

// Local (per-process) information segment
struct        LINFO
  {
  WORD16      ProcessID;      // Current process ID
  WORD16      ParentID;       // Current process' parent's PID
  WORD16      Priority;       // Current thread's priority
  WORD16      ThreadID;       // Current thread ID
  WORD16      SessionID;      // Current session ID
  BYTE        Status;         // Process status
  BYTE        Unused1;        // Unused
  WORD16      ForegroundFlag; // Current process has keyboard focus
  BYTE        Type;           // Type of process (0 = FS, 1 = real, 2 = VIO,
  BYTE        Unused2;        //                  3 = PM, 4 = detached)
  WORD16      EnvSelector;    // Selector for environment
  WORD16      CommandLineOff; // Offset of command line in EnvironmentSel
  WORD16      DSLength;       // Length of data segment
  WORD16      StackSize;      // Size of stack (bytes)
  WORD16      HeapSize;       // Size of heap (bytes)
  WORD16      Module;         // Module handle
  WORD16      DSSelector;     // Selector for data segment
  };


#endif // DevInfo_h
