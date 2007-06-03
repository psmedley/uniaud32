/* $Id: wat2map.cmd,v 1.1.1.1 2003/07/02 13:57:04 eleph Exp $ */

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
CALL RXFUNCADD 'sysloadfuncs','rexxutil','sysloadfuncs'
call sysloadfuncs

Parse Arg arg1 arg2 rest
If (Length( arg1 ) = 0) | (Verify( arg1, '/-?' ) = 0) Then Do;
   Do i = 1 to 1000
      helpText = Sourceline(i)
      If Pos( '<End of helpText>', helpText ) <> 0 Then Leave;       /* quit loop */
      Say helpText
   End;
   Return
End;
If Length( arg2 ) = 0 Then Do;
   Say " Way to go Beaver... How about an out-put file name ?"
   Return
End;
mapFile = arg1          /* Can be Null, in which case we pull from stdin. */
outFile = arg2

/* erase outfile */  /* kill the old map file */
rc=SysFileDelete(outfile)


/*--- 1.  Find & translate module name.  ---*/
Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   Parse Value watcomText With "Executable Image: " fileName "." fileExt

   If fileName <> "" Then Do;   /* Found match */
      call lineout outfile ,' '
      call lineout outfile ,' ' || fileName
      call lineout outfile ,' '
      Leave;                    /* Break from loop. */
   End;
End
If Lines( mapFile ) = 0 Then Do;        /* If end of file ... */
   Say "Error:  Expected to find line with text 'Executable Image:' "
   Return
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
   Return
End
junk = LineIn( mapFile )       /* Discard a couple lines of formatting. */
junk = LineIn( mapFile )

/*--- 4.  Translate segment table.  ---*/
call lineout outfile , " Start     Length     Name                   Class"
Do While Lines( mapFile ) <> 0
   watcomText = LineIn( mapFile )
   Parse Value watcomText With segName className groupName address size .
   If segName = "" Then Leave;          /* Empty line, break from loop. */
   length = Substr( size, 4, 5 ) || 'H     '
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
   Parse Value watcomText With seg ':' ofs 10 . 16 declaration
   is_Adress = (is_Hex(seg) = 1) & (is_Hex(ofs) = 1)
   If (is_Adress = 1) Then Do;

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
End; /* End While through symbol section, end of input file. */

Return;  /* End of program.  */

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
End;

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
End;

is_Hex:
/* Returns 1 if String is valid hex number, 0 otherwise. */
   Parse Arg string
   Return (Length(string) > 0) &  (Verify( string, '0123456789abcdefABCDEF' ) = 0)
End;
