/****************************************************************************
 *                                                                          *
 * Copyright (c) IBM Corporation 1994 - 1997.                               *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 * Copyright (c) 2013 David Azarewicz david@88watts.net                     *
 * Modified to fix problems and for OpenWatcom compatibility                *
 ****************************************************************************/
#ifndef STRATEGY_INCLUDED
#define STRATEGY_INCLUDED

#pragma pack(1);

typedef struct {              /* template for request header */
  BYTE bLength;               /* 00 request packet length */
  BYTE bUnit;                 /* 01 unit code for block DD only */
  BYTE bCommand;              /* 02 command code */
  USHORT usStatus;            /* 03 return status */
  ULONG dwReserved;           /* 05 reserved bytes */
  ULONG ulQlink;              /* 09 queue linkage */
  union {                     /* command-specific data */
    struct {
      BYTE b;                 /* 0D */
      PFN  ulDevHlp;          /* 0E dev help address */
      ULONG szArgs; /* 12 argument pointer */
      BYTE bDrive;            /* 16 */
    } init_in;
    struct {
      BYTE bUnits;
      USHORT usCodeEnd;       // final code offset
      USHORT usDataEnd;       // final data offset
      ULONG ul;
    } init_out;
    struct {
      BYTE bMedia;
      ULONG ulAddress;
      USHORT usCount;
      ULONG ulStartSector;
      USHORT usSysFileNum;
    } io;
    struct {
      BYTE bData;
    } peek;
    struct {
      BYTE bCategory;         // category code
      BYTE bFunction;         // function code
      ULONG pvParm; // address of parameter buffer
      ULONG pvData; // address of data buffer
      USHORT usSysFileNum;    // system file number
      USHORT usParmLen;       // length of parameter buffer
      USHORT usDataLen;       // length of data buffer
    } ioctl;
    struct {
      USHORT usSysFileNum;    // system file number
    } open_close;
    struct {
      BYTE   Function;        // Shutdown code: 0 = start, 1 = end
      ULONG  Reserved;        // Reserved
    } shutdown;
    struct {
      BYTE Function;
    } save_restore;
  };
} REQPACKET;

#pragma pack();

/* Constants relating to the Strategy Routines */

#define RPDONE    0x0100         // return successful, must be set
#define RPBUSY    0x0200         // device is busy (or has no data to return)
#define RPDEV     0x4000         // user-defined error
#define RPERR     0x8000         // return error

#define RPERR_PROTECT         0x8000
#define RPERR_UNIT            0x8001
#define RPERR_NOTREADY        0x8002
#define RPERR_BADCOMMAND      0x8003
#define RPERR_CRC             0x8004
#define RPERR_LENGTH          0x8005
#define RPERR_SEEK            0x8006
#define RPERR_FORMAT          0x8007
#define RPERR_SECTOR          0x8008
#define RPERR_PAPER           0x8009
#define RPERR_WRITE           0x800A
#define RPERR_READ            0x800B
#define RPERR_GENERAL         0x800C
#define RPERR_DISK            0x800D
#define RPERR_MEDIA           0x8010
#define RPERR_INTERRUPTED     0x8011
#define RPERR_MONITOR         0x8012
#define RPERR_PARAMETER       0x8013
#define RPERR_DEVINUSE        0x8014
#define RPERR_INITFAIL        0x8015

// list of Strategy commands from PDD reference
#define STRATEGY_INIT            0x00
#define STRATEGY_MEDIACHECK      0x01
#define STRATEGY_BUILDBPB        0x02
#define STRATEGY_IOCTLR          0x03
#define STRATEGY_READ            0x04
#define STRATEGY_NDR             0x05
#define STRATEGY_INPUTSTATUS     0x06
#define STRATEGY_FLUSHINPUT      0x07
#define STRATEGY_WRITE           0x08
#define STRATEGY_WRITEVERIFY     0x09
#define STRATEGY_OUTPUTSTATUS    0x0A
#define STRATEGY_FLUSHOUTPUT     0x0B
#define STRATEGY_IOCTLW          0x0C
#define STRATEGY_OPEN            0x0D
#define STRATEGY_CLOSE           0x0E
#define STRATEGY_REMOVEABLEMEDIA 0x0F
#define STRATEGY_GENIOCTL        0x10
#define STRATEGY_RESETMEDIA      0x11
#define STRATEGY_GETLOGMAP       0x12
#define STRATEGY_SETLOGMAP       0x13
#define STRATEGY_DEINSTALL       0x14
#define STRATEGY_PARTFIXEDDISKS  0x16
#define STRATEGY_GETFDLOGUNITMAP 0x17
#define STRATEGY_INPUTBYPASS     0x18
#define STRATEGY_OUTPUTBYPASS    0x19
#define STRATEGY_OUTPUTBYPASSV   0x1A
#define STRATEGY_BASEDEVINIT     0x1B
#define STRATEGY_SHUTDOWN        0x1C
#define STRATEGY_GETDRIVERCAPS   0x1D
#define STRATEGY_INITCOMPLETE    0x1F
#define STRATEGY_SAVERESTORE     0x20

#define IOCTL_ASYNC            0x0001
#define IOCTL_SCR_AND_PTRDRAW  0x0003
#define IOCTL_KEYBOARD         0x0004
#define IOCTL_PRINTER          0x0005
#define IOCTL_LIGHTPEN         0x0006
#define IOCTL_POINTINGDEVICE   0x0007
#define IOCTL_DISK             0x0008
#define IOCTL_PHYSICALDISK     0x0009
#define IOCTL_MONITOR          0x000A
#define IOCTL_GENERAL          0x000B
#define IOCTL_POWER            0x000C
#define IOCTL_OEMHLP           0x0080
#define IOCTL_TESTCFG_SYS      0x0080
#define IOCTL_CDROMDISK        0x0080
#define IOCTL_CDROMAUDIO       0x0081
#define IOCTL_TOUCH_DEVDEP     0x0081
#define IOCTL_TOUCH_DEVINDEP   0x0081

#endif
