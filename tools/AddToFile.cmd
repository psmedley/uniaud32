/* Add specified line to file */
/* Adds the specified line to the end of the specified file.
 * Usage:
 *  AddToFile.cmd FileName,PrefixString,Function[,Parameters...]
 *
 * Examples:
 *  AddToFile.cmd FileName,#define DDATE,DATEL
 *    Adds today's date to Filename: #define DDATE 20130312L
 *
 *  AddToFile.cmd FileName,#define BLDLEVEL,BLDLEVEL2,D Azarewicz,1.2.3,Description,Fixpack
 *    Adds a formatted BLDLEVEL string to Filename using double quotes: #define BLDLEVEL "<bldlevel string>"
 *
 *  AddToFile.cmd FileName,option description,BLDLEVEL,D Azarewicz,1.2.3,Description,Fixpack
 *    Adds a formatted BLDLEVEL string to Filename using single quotes: option description '<bldlevel string>'
 *
 *  AddToFile.cmd FileName,InFileName,FILE
*/

parse arg OutFile','String','What','ProjVendor','BuildVersion','ProductName','FixPack;

select
  when (What='DATEL') then do
    rc=lineout(OutFile, String||' '||DATE('S')||'L');
    rc=lineout(OutFile);
  end

  when (LEFT(What,8)='BLDLEVEL') then do
    Type=SUBSTR(What,9,1);

    /* get hostname for build system */
    ProjHost = VALUE('HOSTNAME',,'OS2ENVIRONMENT');

    /* projString = projDate" "projTime"       "projHost */
    ProjString = left(Date()||' '||TIME(),25)||left(ProjHost,10);

    /* get fixpack from SVN version */
    if (FixPack='SVN') then FixPack='SVN'||SvnVersion();

    parse var BuildVersion major'.'minor'.'ProjVersion;

    if (Type='2') then do
      rc=lineout(OutFile, String||' "@#'||ProjVendor||':'||BuildVersion||'#@##1## '||ProjString||'::::'||ProjVersion||'::'||FixPack||'@@'||ProductName||'"');
    end
    else do
      rc=lineout(OutFile, String||" '@#"||ProjVendor||":"||BuildVersion||"#@##1## "||ProjString||"::::"||ProjVersion||"::"||FixPack||"@@"||ProductName||"'");
    end
    rc=lineout(OutFile);
  end

  when (What='DATEYEAR') then do
    rc=lineout(OutFile, String||' '||FORMAT(SUBSTR(DATE('S'), 1, 4)));
    rc=lineout(OutFile);
  end

  when (What='DATEMONTH') then do
    rc=lineout(OutFile, String||' '||FORMAT(SUBSTR(DATE('S'), 5, 2)));
    rc=lineout(OutFile);
  end

  when (What='DATEDAY') then do
    rc=lineout(OutFile, String||' '||FORMAT(SUBSTR(DATE('S'), 7, 2)));
    rc=lineout(OutFile);
  end

  when (What="VERSIONMAJOR") then do
    parse var ProjVendor Major'.'Minor'.'Revision;
    rc=lineout(OutFile, String||' '||FORMAT(Major));
    rc=lineout(OutFile);
  end

  when (What="VERSIONMINOR") then do
    parse var ProjVendor Major'.'Minor'.'Revision;
    rc=lineout(OutFile, String||' '||FORMAT(Minor));
    rc=lineout(OutFile);
  end

  when (What="VERSIONREVISION") then do
    parse var ProjVendor Major'.'Minor'.'Revision;
    rc=lineout(OutFile, String||' '||FORMAT(Revision));
    rc=lineout(OutFile);
  end

  when (What="FILE") then do
    do while LINES(String) <> 0;
      rc=LINEOUT(OutFile, LINEIN(String));
    end;
    rc=stream(String,'c','close');
    rc=lineout(OutFile);
  end

  when (What="VAR") then do
    Line2 = '';
    Len=LENGTH(ProjVendor);
    do while LINES(BuildVersion) <> 0;
      Line1=LINEIN(BuildVersion);
      if (LEFT(Line1, Len) = ProjVendor) then do
        Line2=SUBSTR(Line1, Len+1);
        LEAVE;
      end
    end;
    rc=stream(BuildVersion,'c','close');
    rc=LINEOUT(OutFile, String||' '||Line2);
    rc=lineout(OutFile);
  end

  otherwise do
    say "AddToFile: Unrecognized command: "||What;
    exit 1;
  end
end

exit 0;

SvnVersion:
  File = ARG(1);
  '@svn info 2>&1 | rxqueue'
  do while (QUEUED() > 0)
    PARSE PULL Line1':'Line2
    if (Line1 = "Last Changed Rev") then leave;
    Line2 = "";
  end
  do while (QUEUED() <> 0); PULL; end;
  Return(STRIP(Line2,'B'));

