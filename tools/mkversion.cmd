/*
 * Set time/date/hostname for uniaud build environment
 */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
call SysLoadFuncs;

parse arg sDebugMode;

/* I don't this will ever change from Netlabs */
projVendor    = EnvVar_Get_If('VENDOR', 'Netlabs <www.netlabs.org>');
projProduct   = "OS/2 Universal Audio 32 Driver";

/* uniaud.inc is located in the base gpl directory and
 * is read by mkversion.cmd for constants. The result is
 * written to include\version.h.
 */
sAlsaBase = Directory();
versConst = sAlsaBase||"\uniaud.inc";

versMak   = sAlsaBase||"\include\version.mak";
versHdr   = sAlsaBase||"\include\version.h";
AlsaVersHdr = sAlsaBase||'\alsa-kernel\include\sound\version.h';

/* get and format date and time */
curDate    = DATE('S');
projDate   = RIGHT(curDate, 2)||"."||SUBSTR(curDate, 5, 2)||"."||LEFT(curDate, 4);
projDate2  = LEFT(curDate, 4)||SUBSTR(curDate, 5, 2)||RIGHT(curDate, 2)L
projTime   = TIME( );

if (sDebugMode<>'') then do
  say "Project date: "projDate
  say "Project time: "projTime
  say "Inc File:     "versConst
  say "Version Mak:   "versMak
  say "Version Hdr:   "versHdr
end

LINEIN(versConst,,0);

fixpack = '';

do while(LINES(versConst))
  tmpLine = LINEIN(versConst);

  IF POS('#', tmpline) = 1 Then iterate;

  opLoc = POS('BUILDVERSION', tmpLine);
  if opLoc > 0 then
    versionIn = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

  opLoc = POS('PRODUCTNAME', tmpLine)
  if opLoc > 0 then
    productName = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

  opLoc = POS('FIXPACK', tmpLine)
  if opLoc > 0 then
    fixpack = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')
end

if (sDebugMode<>'') then say 'versionIn='||versionIn;

if (versionIn = "") then do
  say "Could not find Version in "||versConst;
  exit;
end

parse value versionIn with major'.'minor'.'projVersion

projVers2 = major||minor||projVersion;

LINEIN(AlsaVersHdr,,0)

alsalevel = ''

do while(LINES(AlsaVersHdr))
  tmpLine = LINEIN(AlsaVersHdr)
  opLoc = POS('#define CONFIG_SND_VERSION', tmpLine)
  if opLoc > 0 then AlsaLevel = STRIP(RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS('"', tmpLine)))),,'"')
end

if (sDebugMode<>'') then do
  SAY 'Alsalevel='||AlsaLevel;
  say 'ProjVendor='||ProjVendor;
  say 'versionIn='||versionIn;
  say 'ProjString='||ProjString;
  say 'alsalevel='||alsalevel;
  say 'ProjVersion='||ProjVersion;
  say 'projVers2='||projVers2;
  say 'Fixpack='||Fixpack;
end

dummy = SysFileDelete(versMak);

if (stream(versMak, 'c', 'open write' ) = 'READY:') then
do
    call lineout versMak, '#'
    call lineout versMak, '#       VERSION.MAK'
    call lineout versMak, '#'
    call lineout versMak, '#       Current build level for File Versioning'
    call lineout versMak, '#'
    call lineout versMak, '#       Generated by mkversion.cmd, do NOT edit !'
    call lineout versMak, '#'
    call lineout versMak, 'BLDLVL_VENDOR = '||Translate(projVendor,,'><');
    call lineout versMak, 'BLDLVL_VERSION = '||versionIn;
    call lineout versMak, 'BLDLVL_FIXPACK = '||Fixpack;
    call lineout versMak, 'BLDLVL_ALSA = '||alsalevel;
    call stream versMak, 'c', 'close';
end
else
do
    say 'oops, failed to open outputfile,' versMak;
    exit 1;
end

dummy = SysFileDelete(versHdr)

if (stream(versHdr, 'c', 'open write' ) = 'READY:') then
do
    call lineout versHdr, '/*';
    call lineout versHdr, ' * version.h   Header for version string';
    call lineout versHdr, ' *';
    call lineout versHdr, ' *    Generated by mkversion.cmd, do NOT edit !';
    call lineout versHdr, ' *';
    call lineout versHdr, ' */';
    call lineout versHdr, ' ';
    call lineout versHdr, ' ';
    call lineout versHdr, '#ifndef __UNIAUDVERSION_H__';
    call lineout versHdr, '#define __UNIAUDVERSION_H__';
    call lineout versHdr, ' ';
    call lineout versHdr, ' ';
    call lineout versHdr, '#define PRODUCT_NAME            "'||ProductName||'"';
    call lineout versHdr, '#define VENDOR_NAME             "'||projVendor||'"';
    call lineout versHdr, '#define PRODUCT_TIMESTAMP       '||projDate2||'       // YYYYMMDD';
    call lineout versHdr, '#define UNIAUD_VERSION          "'||versionIn||'"';
    call lineout versHdr, '#define ALSA_VERSION            "'||alsalevel||'"';
    call lineout versHdr, ' '
    call lineout versHdr, ' '
    call lineout versHdr, '#define RM_VERSION              '||projVers2;
    call lineout versHdr, '#define RM_VMAJOR               '||format(major);
    call lineout versHdr, '#define RM_VMINOR               '||format(minor);
    call lineout versHdr, '#define RM_DRIVER_NAME          "UNIAUD32.SYS"';
    call lineout versHdr, '#define RM_DRIVER_DESCRIPTION   "'||projProduct||'"';
    call lineout versHdr, '#define RM_DRIVER_VENDORNAME    "'||projVendor||'"';
    call lineout versHdr, '#define RM_DRIVER_BUILDYEAR     (PRODUCT_TIMESTAMP / 10000)';
    call lineout versHdr, '#define RM_DRIVER_BUILDMONTH    ((PRODUCT_TIMESTAMP / 100) % 100)';
    call lineout versHdr, '#define RM_DRIVER_BUILDDAY      (PRODUCT_TIMESTAMP % 100)';
    call lineout versHdr, ' ';
    call lineout versHdr, ' ';
    call lineout versHdr, "#endif //__UNIAUDVERSION_H__";
    call lineout versHdr, ' ';
    call stream versHdr, 'c', 'close';
end
else
do
    say 'oops, failed to open outputfile,' versHdr;
    exit 1;
end

exit 0;

/**
 * Procedure section
 **/

EnvVar_Get: procedure
    parse arg sEnvVar
    if translate(sEnvVar) == 'BEGINLIBPATH' then
      s = SysQueryExtLibpath('B')
    else if translate(sEnvVar) == 'ENDLIBPATH' then
      s = SysQueryExtLibpath('E')
    else
      s = value(sEnvVar,, 'OS2ENVIRONMENT')
    return s;

/*
 * Returns value of sEnvVar from environment if set or returns default value.
 */
EnvVar_Get_If: procedure
  parse arg sEnvVar, sDefault;
  s = EnvVar_Get(sEnvVar);
  if s == '' then
    s = sDefault;
  return s;

