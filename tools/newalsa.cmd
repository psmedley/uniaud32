/* Rexx
 * This is a tool for comparing and importing a new Linux ALSA source
 * from the Linux kernel source tree
 */
LinuxBase='g:\linux-4.6.1';
UniaudBase='f:\src\os2\uniaud32local\trunk\alsa-kernel';

LinuxKernelBase=LinuxBase||'\sound';
LinuxIncludeBase=LinuxBase||'\include\sound';
UniaudKernelBase=UniaudBase;
UniaudIncludeBase=UniaudBase||'\include\sound';

parse arg Arg1;
MakeChanges=0;
if (Arg1 = 'MakeChanges') then MakeChanges=1;

/* ================================================================================ */

Say "Checking directories...";
Differences = 0;

rc=SysFileTree(UniaudKernelBase||'\*', Uniaud, 'DSO');
rc=SysFileTree(LinuxKernelBase||'\*', Linux, 'DSO');
UniaudBaseLen=LENGTH(UniaudKernelBase);
LinuxBaseLen=LENGTH(LinuxKernelBase);

Say;

do i=1 to Uniaud.0;
  if (POS('.svn', Uniaud.i) > 0) then iterate;
  UniaudName = SUBSTR(Uniaud.i, UniaudBaseLen+2);
  if (IsIgnoreDir(UniaudName)) then iterate;
  if (FindInLinuxList(UniaudName)<>'') then iterate;
  say UniaudName||' is not in the Linux list';
  Differences = Differences + 1;
end;

Say;

do i=1 to Linux.0;
  LinuxName = SUBSTR(Linux.i, LinuxBaseLen+2);
  if (IsIgnoreDir(LinuxName)) then iterate;
  if (FindInUniaudList(LinuxName)<>'') then iterate;
  say LinuxName||' is not in the Uniaud list';
  Differences = Differences + 1;
end;

rc=SysFileTree(UniaudIncludeBase||'\*', Uniaud, 'DSO');
rc=SysFileTree(LinuxIncludeBase||'\*', Linux, 'DSO');
UniaudBaseLen=LENGTH(UniaudIncludeBase);
LinuxBaseLen=LENGTH(LinuxIncludeBase);

Say;

do i=1 to Uniaud.0;
  if (POS('.svn', Uniaud.i) > 0) then iterate;
  UniaudName = SUBSTR(Uniaud.i, UniaudBaseLen+2);
  if (FindInLinuxList(UniaudName)<>'') then iterate;
  say UniaudName||' is not in the Linux list';
  Differences = Differences + 1;
end;

Say;

do i=1 to Linux.0;
  LinuxName = SUBSTR(Linux.i, LinuxBaseLen)+2);
  if (LEFT(LinuxName,8) = 'generate') then iterate; /* we don't use this directory */
  if (LEFT(LinuxName,5) = 'tests') then iterate;  /* we don't use this directory */
  if (FindInUniaudList(LinuxName)<>'') then iterate;
  say LinuxName||' is not in the Uniaud list';
  Differences = Differences + 1;
end;

if (Differences > 0) then do;
  Say;
  Say 'There are '||Differences||' differences in the directory structure.';
  Say 'Fixup the directory structure before continuing.';
  exit;
end;

/* ================================================================================ */

Say;
rc=SysFileTree(UniaudIncludeBase||'\*', Uniaud, 'FSO');
rc=SysFileTree(LinuxIncludeBase||'\*', Linux, 'FSO');
UniaudBaseLen=LENGTH(UniaudIncludeBase);
LinuxBaseLen=LENGTH(LinuxIncludeBase);

if (MakeChanges) then Say "Making changes...";
else Say "Checking files...";

do i=1 to Uniaud.0;
  if (POS('.svn', Uniaud.i) > 0) then iterate;
  UniaudName = SUBSTR(Uniaud.i, UniaudBaseLen+2);
  UniaudDir = STRIP(FILESPEC('p', UniaudName),'B','\');
  if (IsIgnoreDir(UniaudDir)) then iterate;
  if (IsIgnoreFile(UniaudName)) then iterate;
  LinuxName = FindInLinuxList(UniaudName);
  if (LinuxName<>'') then iterate;
  Say;
  say UniaudName||' is not in the Linux list';

  if (MakeChanges) then do;
    rc=SysFileDelete(Uniaud.i);
  end;
  else do;
    Say UniaudName||' will be deleted.';
    say '*** Makefile changes may be necessary ***';
  end;
end;

do i=1 to Linux.0;
  LinuxName = SUBSTR(Linux.i, LinuxBaseLen+2);
  LinuxDir = STRIP(FILESPEC('p', LinuxName),'B','\');
  if (IsIgnoreDir(LinuxDir)) then iterate;
  if (IsIgnoreFile(LinuxName)) then iterate;
  UniaudName = FindInUniaudList(LinuxName);
  if (UniaudName<>'') then do;
    if (FileChanged(Linux.i, UniaudName)) then do;
      Say;
      Say LinuxName||' has changed.';
      if (MakeChanges) then do;
        Say 'Copying '||Linux.i||' to '||UniaudName;
        address CMD '@copy '||Linux.i||' '||UniaudName;
      end;
      else do;
        Say LinuxName||' will be updated.';
        Say 'gfc '||UniaudName||' '||Linux.i;
      end;
      /*if (TRANSLATE(RIGHT(LinuxName, 8)) = 'ACTBL3.H') then say '*** LocMod file: Hand edit requred ***';*/
    end;
  end;
  else do;
    Say;
    say LinuxName||' is not in the Uniaud list';
    UniaudName=UniaudIncludeBase||'\'||LinuxName;
    if (MakeChanges) then do;
      Say 'Adding '||Linux.i||' to '||UniaudName;
      address CMD '@copy '||Linux.i||' '||UniaudName;
    end;
    else do;
      Say LinuxName||' will be added.';
    end;
    say '*** Makefile changes may be necessary ***';
  end;
end;

Say;
rc=SysFileTree(UniaudKernelBase||'\*', Uniaud, 'FSO');
rc=SysFileTree(LinuxKernelBase||'\*', Linux, 'FSO');
UniaudBaseLen=LENGTH(UniaudKernelBase);
LinuxBaseLen=LENGTH(LinuxKernelBase);

if (MakeChanges) then Say "Making changes...";
else Say "Checking files...";

do i=1 to Uniaud.0;
  if (POS('.svn', Uniaud.i) > 0) then iterate;
  UniaudName = SUBSTR(Uniaud.i, UniaudBaseLen+2);
  UniaudDir = STRIP(FILESPEC('p', UniaudName),'B','\');
  if (IsIgnoreDir(UniaudDir)) then iterate;
  if (IsIgnoreFile(UniaudName)) then iterate;
  LinuxName = FindInLinuxList(UniaudName);
  if (LinuxName<>'') then iterate;
  Say;
  say UniaudName||' is not in the Linux list';

  if (MakeChanges) then do;
    rc=SysFileDelete(Uniaud.i);
  end;
  else do;
    Say UniaudName||' will be deleted.';
    say '*** Makefile changes may be necessary ***';
  end;
end;

do i=1 to Linux.0;
  LinuxName = SUBSTR(Linux.i, LinuxBaseLen+2);
  LinuxDir = STRIP(FILESPEC('p', LinuxName),'B','\');
  if (IsIgnoreDir(LinuxDir)) then iterate;
  if (IsIgnoreFile(LinuxName)) then iterate;
  UniaudName = FindInUniaudList(LinuxName);
  if (UniaudName<>'') then do;
    if (FileChanged(Linux.i, UniaudName)) then do;
      Say;
      Say LinuxName||' has changed.';
      if (MakeChanges) then do;
        Say 'Copying '||Linux.i||' to '||UniaudName;
        address CMD '@copy '||Linux.i||' '||UniaudName;
      end;
      else do;
        Say LinuxName||' will be updated.';
        Say 'gfc '||UniaudName||' '||Linux.i;
      end;
      /*if (TRANSLATE(RIGHT(LinuxName, 8)) = 'ACTBL3.H') then say '*** LocMod file: Hand edit requred ***';*/
    end;
  end;
  else do;
    Say;
    say LinuxName||' is not in the Uniaud list';
    UniaudName=UniaudKernelBase||'\'||LinuxName;
    if (MakeChanges) then do;
      Say 'Adding '||Linux.i||' to '||UniaudName;
      address CMD '@copy '||Linux.i||' '||UniaudName;
    end;
    else do;
      Say LinuxName||' will be added.';
    end;
    say '*** Makefile changes may be necessary ***';
  end;
end;

exit;

FindInLinuxList: procedure expose Linux. LinuxBaseLen;
  parse arg FileName;
  do i=1 to Linux.0;
    File = SUBSTR(Linux.i, LinuxBaseLen+2);
    if (File = FileName) then return Linux.i;
  end;
  return '';

FindInUniaudList: procedure expose Uniaud. UniaudBaseLen;
  parse arg FileName;
  do i=1 to Uniaud.0;
    if (POS('.svn', Uniaud.i) > 0) then iterate;
    File = SUBSTR(Uniaud.i, UniaudBaseLen+2);
    if (File = FileName) then return Uniaud.i;
  end;
  return '';

FileChanged: procedure;
  parse arg File1, File2;
  address CMD '@diff -q '||File1||' '||File2||' >NUL 2>&1';
  return rc;

IsIgnoreDir: procedure;
  parse arg TestName;
  if (LEFT(TestName,2) = 'sh') then return(1);
  if (LEFT(TestName,3) = 'aoa') then return(1);
  if (LEFT(TestName,3) = 'hda') then return(1);
  if (LEFT(TestName,3) = 'arm') then return(1);
  if (LEFT(TestName,5) = 'atmel') then return(1);
  if (LEFT(TestName,3) = 'usb') then return(1);
  if (LEFT(TestName,3) = 'oss') then return(1);
  if (LEFT(TestName,3) = 'i2c') then return(1);
  if (LEFT(TestName,3) = 'ppc') then return(1);
  if (LEFT(TestName,3) = 'spi') then return(1);
  if (LEFT(TestName,3) = 'soc') then return(1);
  if (LEFT(TestName,5) = 'sparc') then return(1);
  if (LEFT(TestName,4) = 'mips') then return(1);
  if (LEFT(TestName,6) = 'pcmcia') then return(1);
  if (LEFT(TestName,6) = 'parisc') then return(1);
  if (LEFT(TestName,8) = 'firewire') then return(1);
  if (LEFT(TestName,7) = 'include') then return(1);
  if (LEFT(TestName,13) = 'Documentation') then return(1);
  if (LEFT(TestName,10) = 'isa\galaxy') then return(1);
  if (LEFT(TestName,8) = 'isa\msnd') then return(1);
  if (LEFT(TestName,7) = 'isa\wss') then return(1);
  if (LEFT(TestName,10) = 'pci\oxygen') then return(1);
  if (LEFT(TestName,9) = 'pci\vx222') then return(1);
  if (LEFT(TestName,11) = 'pci\riptide') then return(1);
  if (LEFT(TestName,9) = 'pci\pcxhr') then return(1);
  if (LEFT(TestName,10) = 'pci\mixart') then return(1);
  if (LEFT(TestName,12) = 'pci\lx6464es') then return(1);
  if (LEFT(TestName,8) = 'pci\lola') then return(1);
  if (LEFT(TestName,11) = 'pci\ice1712') then return(1);
  if (LEFT(TestName,13) = 'pci\echoaudio') then return(1);
  if (LEFT(TestName,9) = 'pci\ctxfi') then return(1);
  if (LEFT(TestName,7) = 'pci\aw2') then return(1);
  if (LEFT(TestName,10) = 'pci\asihpi') then return(1);
  if (LEFT(TestName,10) = 'drivers\vx') then return(1);
  if (LEFT(TestName,12) = 'drivers\pcsp') then return(1);
  if (LEFT(TestName,12) = 'drivers\opl4') then return(1);
  if (TestName = 'trace') then return(1);
  return(0);

IsIgnoreFile: procedure;
  parse upper arg TestName;
  if (RIGHT(TestName, 4) = '.TMP') then return(1);
  if (RIGHT(TestName, 4) = '.LOG') then return(1);
  if (RIGHT(TestName, 4) = '.LIB') then return(1);
  if (RIGHT(TestName, 4) = '.MAP') then return(1);
  if (RIGHT(TestName, 4) = '.ZIP') then return(1);
  if (RIGHT(TestName, 4) = '.COD') then return(1);
  if (RIGHT(TestName, 4) = '.OBJ') then return(1);
  if (RIGHT(TestName, 4) = '.TFF') then return(1);
  if (RIGHT(TestName, 4) = '.LNK') then return(1);
  if (RIGHT(TestName, 4) = '.DDP') then return(1);
  if (RIGHT(TestName, 4) = '.LST') then return(1);
  if (RIGHT(TestName, 4) = '.SYM') then return(1);
  if (RIGHT(TestName, 4) = '.SYS') then return(1);
  if (RIGHT(TestName, 4) = '.BAK') then return(1);
  if (RIGHT(TestName, 4) = '.ADD') then return(1);
  if (RIGHT(TestName, 5) = '.DIFF') then return(1);
  if (RIGHT(TestName, 7) = 'KCONFIG') then return(1);
  if (RIGHT(TestName, 8) = 'MAKEFILE') then return(1);
  return(0);

