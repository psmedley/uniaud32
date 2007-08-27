/*
 * Set time/date/hostname for uniaud build environment
 */

parse arg sAlsaBase

/* I don't this will ever change from Netlabs */
projVendor    = "Netlabs"
projProduct   = "OS/2 Universal Audio 32 Driver"
projVendorNam = "Netlabs <www.netlabs.org>"
projAdapter   = "OS/2 Universal Audio"


/* uniaud.inc is located in the base gpl directory and
 * is read by mkversion.cmd for constants. The result is
 * written to include\version.h.
 */
if sAlsaBase = "" then sAlsaBase = "."
versConst = sAlsaBase"\uniaud.inc"

versMak   = sAlsaBase"\include\version.mak"
versHdr   = sAlsaBase"\include\version.h"

/* get and format date and time */
curDate    = DATE('S')
projDate   = RIGHT(curDate, 2)"."SUBSTR(curDate, 5, 2)"."LEFT(curDate, 4)
projDate2  = LEFT(curDate, 4)SUBSTR(curDate, 5, 2)RIGHT(curDate, 2)L
projTime   = TIME( )

/* get hostname for build system */
projHost = VALUE('HOSTNAME',,'OS2ENVIRONMENT')

projString = left(Date() TIME(),25)left(projHost,10)

say "Project date: "projDate
say "Project time: "projTime
say "hostname:     "projHost
say "Version In:    "versConst
say "Version Mak:   "versMak
say "Version Hdr:   "versHdr

LINEIN(versConst,,0)

fixpack=''

do while(LINES(versConst))
    tmpLine = LINEIN(versConst)

    IF POS('#', tmpline) = 1 Then iterate

    opLoc = POS('BUILDVERSION', tmpLine)
    if opLoc > 0 then
      versionIn = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

    opLoc = POS('PRODUCTNAME', tmpLine)
    if opLoc > 0 then
      productName = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

    opLoc = POS('FIXPACK', tmpLine)
    if opLoc > 0 then
      fixpack = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

    opLoc = POS('ALSAVERSION', tmpLine)
    if opLoc > 0 then
      alsalevel = STRIP((RIGHT(tmpLine, ( LENGTH(tmpLine) - (POS("=", tmpLine)+1) ))), B, ' ')

end

if versionIn = "" then do
    say "Could not find Version in "versConst
    exit
end

parse value versionIn with major'.'minor'.'projVersion

projVers  = versionIn'-'fixpack
projVers2 = major||minor||LEFT(projVersion, 1)


SET "BUILDLEVEL=@#"ProjVendor":"major"."minor"#@##1## "ProjString":"alsalevel":::"ProjVersion"::"Fixpack"@@"productname

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs
dummy = SysFileDelete(versMak)

if (stream(versMak, 'c', 'open write' ) = 'READY:') then
do
    call lineout versMak, '#'
/*    call lineout versMak, '#       VERSION.MAK'
    call lineout versMak, '#'
    call lineout versMak, '#       Current build level for File Versioning'
    call lineout versMak, '#'    
    call lineout versMak, '#       Generated by mkversion.cmd, do NOT edit !'
    call lineout versMak, '#'
    call lineout versMak, 'BLDLVL_VENDOR = 'projVendor
    call lineout versMak, 'BLDLVL_PRODUCT = 'projProduct
    call lineout versMak, 'BLDLVL_REVISION = 'major'.'minor
    call lineout versMak, 'BLDLVL_FILEVER = 'projVersion
    call lineout versMak, 'BLDLVL_DATETIME = 'projDate' 'projTime
    call lineout versMak, 'BLDLVL_MACHINE = 'projHost
    call stream versMak, 'c', 'close';
*/
end
else
do
    say 'oops, failed to open outputfile,' versMak;
    exit 1;
end


dummy = SysFileDelete(versHdr)

if (stream(versHdr, 'c', 'open write' ) = 'READY:') then
do
    call lineout versHdr, '/*'
    call lineout versHdr, ' * version.h   Header for version string'
    call lineout versHdr, ' *'
    call lineout versHdr, ' *    Generated by mkversion.cmd, do NOT edit !'
    call lineout versHdr, ' *'
    call lineout versHdr, ' */'
    call lineout versHdr, ' '
    call lineout versHdr, ' '
    call lineout versHdr, '#ifndef __UNIAUDVERSION_H__'
    call lineout versHdr, '#define __UNIAUDVERSION_H__'
    call lineout versHdr, ' '
    call lineout versHdr, ' '
    call lineout versHdr, '#define PRODUCT_NAME            "'ProductName'"'
    call lineout versHdr, '#define VENDOR_NAME             "'projVendor'"'
    call lineout versHdr, '#define PRODUCT_TIMESTAMP       'projDate2'       // YYYYMMDD'
    call lineout versHdr, '#define UNIAUD_VERSION          "'projVers'"'
    call lineout versHdr, '#define ALSA_VERSION            "'alsalevel'"'
    call lineout versHdr, ' '
    call lineout versHdr, ' '
    call lineout versHdr, '#define RM_VERSION              'projVers2
    call lineout versHdr, '#define RM_DRIVER_NAME          "UNIAUD32.SYS"'
    call lineout versHdr, '#define RM_DRIVER_DESCRIPTION   "'projProduct'"'
    call lineout versHdr, '#define RM_ADAPTER_NAME         "'projAdapter'"'
    call lineout versHdr, '#define RM_DRIVER_VENDORNAME    "'projVendorNam'"'
    call lineout versHdr, '#define RM_DRIVER_BUILDYEAR     (PRODUCT_TIMESTAMP / 10000)'
    call lineout versHdr, '#define RM_DRIVER_BUILDMONTH    ((PRODUCT_TIMESTAMP / 100) % 100)'
    call lineout versHdr, '#define RM_DRIVER_BUILDDAY      (PRODUCT_TIMESTAMP % 100)'
    call lineout versHdr, ' '
    call lineout versHdr, ' '
    call lineout versHdr, "#endif //__UNIAUDVERSION_H__"
    call lineout versHdr, ' '
    call stream versHdr, 'c', 'close';
end
else
do
    say 'oops, failed to open outputfile,' versHdr;
    exit 1;
end

exit 0;



