/*
 * Includes fixes from Helper script MapSym.cmd by Knut (aka bird).
 *
 * Copyright (c) 2002 knut st. osmundsen (bird@anduin.net)
 *
 * Project Odin Software License can be found in LICENSE.TXT
 *
 */

/* SCCSID = %W% %E% */
/****************************************************************************
 *                                                                          *
 * Copyright (c) IBM Corporation 1994 - 1997.                               *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 ****************************************************************************/
/**@internal %W%
 * WAT2MAP - translate symbol map from Watcom format to MS format.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 *  Usage:  WAT2MAP <watcom_mapfile >ms_mapfile
 *               - or -
 *          type watcom_mapfile | WAT2MAP >ms_mapfile
 *
 *          Reads from stdin, writes to stdout.  Will accept the Watcom map filename
 *          as an argument (in place of reading from stdin).  Eg.,
 *
 *          WAT2MAP watcom_mapfile >ms_mapfile
 *
 *  Notes:
 *    1.)  The symbol handling in the debug kernel won't work for some of the
 *         characters used in the C++ name space.  WAT2MAP handles these symbols
 *         as follows.
 *             Scoping operator symbol '::' is translated to '__'.
 *             Destructor symbol '~' is translated to 'd'.
 *             Symbols for operators '::operator' are not provided.
 *
 *         Eg., for user defined class 'A', the symbol for constructor A::A is
 *         translated to A__A, and the destructor symbol A::~A becomes A__dA, and
 *         the assignment operator, 'A::operator =', is not translated.
 *
 *    2.)  Bug - C++ provides for defining multiple functions with same fn name
 *         but different function signatures (different parameter lists).  This
 *         utility just translates all the address / symbol combinations it finds,
 *         so you can end up with several addresses for a given fn name.
 * @history
*/
/* <End of helpText> - don't modify this string - used to flag end of help. */
/****************************************************************************/

Parse arg arg1 arg2

If (Length( arg1 ) = 0) | (Verify( arg1, '/-?' ) = 0) Then Do;
   Do i = 1 to 1000
      helpText = Sourceline(i)
      If Pos( '<End of helpText>', helpText ) <> 0 Then Leave;       /* quit loop */
      Say helpText
   End;
   Return 1
End;

If Length( arg2 ) == 0 Then Do;
   Say " Way to go Beaver... How about an out-put file name ?"
   Return 2
End;
mapFile = arg1          /* Can be Null, in which case we pull from stdin. */
outFile = arg2
fFlatMode = 0;

/* erase outfile */  /* kill the old map file */
rc=SysFileDelete(outfile)


/*--- 1.  Find & translate module name.  ---*/
Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   /*Parse Value watcomText With "Executable Image: " fileName "." fileExt*/
   Parse Value watcomText With "Executable Image: " sFilename
   If (sFilename <> '') Then Do;
      sFilename = filespec('name', sFilename);
      Parse Var sFilename fileName "." fileExt
      If fileName <> "" Then Do;   /* Found match */
         call lineout outfile ,' '
         call lineout outfile ,' ' || fileName
         call lineout outfile ,' '
         Leave;                    /* Break from loop. */
      End;
   End;
End
If Lines( mapFile ) = 0 Then Do;        /* If end of file ... */
   Say "Error:  Expected to find line with text 'Executable Image:' "
   Return 3
End

/*--- 2.  Skip the group definitions - Rob's notes say we don't need them. -*/

/*--- 3.  Skip to the start of the segment table.  ---*/
Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   Parse Value watcomText With "Segment" header_2_3 "Address" header_5
   If Strip( header_5 ) = "Size" Then Leave;      /* Found header line for Segment table. */
End
If Lines( mapFile ) = 0 Then Do;        /* If end of file ... */
   Say "Error:  Expected to find line with text 'Segments ... Size' "
   Return 4
End


junk = LineIn( mapFile )      /* Discard a couple lines of formatting. */
junk = LineIn( mapFile )      /* Discard a couple lines of formatting. */
/*--- 4.  Translate segment table.  ---*/
/*"Segment                Class          Group          Address         Size"*/
iClass = pos('Class', watcomText);
iGroup = pos('Group', watcomText);
iAddress = pos('Address', watcomText);
iSize = pos('Size', watcomText);

call lineout outfile , " Start         Length     Name                   Class"  /* bird bird bird fixed!!! */
Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   /* do it the hard way to make sure we support spaces segment names. */
   segName = strip(substr(watcomText, 1, iClass-1));
   If segName = "" Then Leave;          /* Empty line, break from loop. */
   className = strip(substr(watcomText, iClass, iGroup-iClass));
   groupName = strip(substr(watcomText, iGroup, iAddress-iGroup));
   address = strip(substr(watcomText, iAddress, iSize-iAddress));
   size = strip(substr(watcomText, iSize));
   if (pos(':', address) <= 0) then     /* NT binaries doesn't have a segment number. */
   do
      fFlatMode = 1;
      address = '0001:'||address;
   end
   length = right(strip(strip(size), 'L', '0'), 9, '0') || 'H     '
   segName = Left( segName, 23 )
   call lineout outfile ,' ' || address || ' ' || length || segName || className
End
call lineout outfile ,' '     /* Extra line feed. */


/*--- 5.  For all remaining lines in the input file:  if the line starts
   with a 16:16 address, assume it's a symbol declaration and translate
   it into MS format.  ---*/

call lineout outfile ,'  Address         Publics by Value'
/* call lineout outfile ,' '*/

Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   Parse Value watcomText With seg ':' ofs 14 . 16 declaration
   if (fFlatMode) then
   do
      seg = '0001';
      Parse Value watcomText With ofs 9 . 16 declaration
   end
   else
   do  /* kso: more workarounds */
       if (is_Hex(seg) & length(ofs) > 4 & \is_Hex(substr(ofs,5,1))) then
           ofs = '0000'||left(ofs,4);
   end
   /*say ofs  '-'declaration*/
   is_Adress = (is_Hex(seg) = 1) & (is_Hex(ofs) = 1)
   If ((is_Adress = 1) & (seg <> '0000')) Then Do;          /* bird: skip symbols with segment 0. (__DOSseg__) */
      /*--- Haven't done the work to xlate operator symbols - skip the line. */
      If Pos( '::operator', declaration ) <> 0 Then Iterate;

      /*--- Strip any arguement list if this is a function prototype.  */
      declaration = StripMatchedParen( declaration )

      /*--- Strip array brackets if this is an array. */
      sqBracket = Pos( '[', declaration )
      If sqBracket <> 0
         Then declaration = Substr(declaration, 1, sqBracket-1);

      /*--- Strip leading tokens from the function name.
         Eg., remove function return type, near/far, etc.  */
      declaration = Word( declaration, Words(declaration) )

      /*--- Strip any remaining parens around function name. ---*/
      declaration = ReplaceSubstr( '(', ' ', declaration )
      declaration = ReplaceSubstr( ')', ' ', declaration )

      /*--- Debug kernel doesn't like symbol for scoping operator "::"
         in symbol names.  Replace :: with double underscore "__". ---*/
      declaration = ReplaceSubstr( '::', '__', declaration )

      /*--- Debug kernel doesn't like symbol for destructor "~"
         in symbol names.  Replace ~ with character "d" for "destructor.
         Note destructor for a class will translate "A::~A" -> "A__dA". ---*/
      declaration = ReplaceSubstr( '~', 'd', declaration )

      call lineout outfile ,' ' || seg || ':' || ofs || '       ' || declaration
   End;

   /* check for entry point, if found we add it and quit. */
   if (pos('Entry point address', watcomText) > 0) then
   do
       parse var watcomText 'Entry point address:' sEntryPoint
       if (pos(':', sEntryPoint) <= 0) then
           sEntryPoint = '0001:'||strip(sEntryPoint);
       call lineout outfile, ''
       call lineout outfile, 'Program entry point at' strip(sEntryPoint)
       leave;
   end
End; /* End While through symbol section, end of input file. */

call stream outfile, 'c', 'close';
call stream mapfile, 'c', 'close';

Return 0;  /* End of program.  */

/*--- Helper subroutines. ---*/

StripMatchedParen:
/* Strips matched "( )" from end of string.  Returns
   a substring with the trailing, matched parens deleted. */

   Parse Arg string

   ixOpenParen = LastPos( "(", string );
   ixCloseParen = LastPos( ")", Substr( string, 1, Length(string)-1 ));

   If (ixOpenParen = 0)                     /* No match. */
      Then Return string
   Else If ixCloseParen < ixOpenParen       /* Found match, no imbedded "()". */
      Then Return Substr( string, 1, ixOpenParen-1 )
   Else Do;                                 /* Imbedded (), must skip over them. */
      /* Parse Value string With first ixCloseParen+1 rest */
      first = Substr( string, 1, ixCloseParen)
      rest = Substr( string, ixCloseParen+1 )
      string = StripMatchedParen( first ) || rest
      Return StripMatchedParen( string )
   End;
return;

ReplaceSubstr:
/* Replaces oldPat (old pattern) with newPat (new pattern) in string. */

   Parse Arg oldPat , newPat , string

   ix = Pos( oldPat, string )
   if ix <> 0 Then Do;
      first = Substr( string, 1, ix-1 )
      rest  = Substr( string, ix + Length( oldPat ) )
      string = first || newPat || rest
   End;
   Return string
return;

is_Hex:
/* Returns 1 if String is valid hex number, 0 otherwise. */
   Parse Arg string
   Return (Length(string) > 0) &  (Verify( string, '0123456789abcdefABCDEF' ) = 0)
return;
