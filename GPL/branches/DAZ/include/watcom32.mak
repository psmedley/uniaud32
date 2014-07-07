
CREATEPATH=$(ALSA_TOOLS)\CreatePath.cmd
WAT2MAP=$(ALSA_TOOLS)\wat2map.cmd

#===================================================================
#
#   Auto-dependency information
#
#===================================================================
.ERASE
.SUFFIXES:
.SUFFIXES: .lst .obj .lib .cpp .cpp16 .c .c16 .asm .def

!if "$(DEBUG)" == "1"
CFLAGS  = -dDEBUG -bt=os2v2 -e60 -hc -5r -omlinear -s -w4 -ze -zdp -zl -zq -nt=CODE32 -zff -zgf
CPPFLAGS= -xd
ASFLAGS = -D:DEBUG -Mb -Li -Sv:M510
!else
CFLAGS  = -bt=os2v2 -e60 -5r -omlinear -s -w4 -ze -zdp -zl -zq -nt=CODE32 -zff -zgf
CPPFLAGS= -xd
ASFLAGS = -Mb -Li -Sv:M510
!endif

!ifdef PACK_1
CFLAGS += -zp1
!else
CFLAGS += -zp4
!endif

!if $(KEE) == 1
CFLAGS16 = $(CFLAGS) -mc -zu -zc
CFLAGS  +=  -mf -DKEE -DFLATSTACK
ASFLAGS += -D:KEE -D:FLATSTACK
!else
CFLAGS16 = $(CFLAGS) -mc -zu -zc
CFLAGS  +=  -mf -DFLATSTACK
ASFLAGS += -D:FLATSTACK
!endif

!if "$(ACPI)" == "1"
CFLAGS += -DACPI
!endif

CC          = WCC386 $(CFLAGS) -zev $(CDEFINES) -i$(CINCLUDES)
CPP         = WPP386 $(CFLAGS) $(CPPFLAGS) $(CDEFINES) -i$(CINCLUDES)
CPP16       = WPP386 $(CFLAGS16) $(CPPFLAGS) $(CDEFINES) -i$(CINCLUDES)
ASM         = alp $(ASFLAGS) $(AINCLUDES)

DFLAGS  = -l -s
DIS     = WDISASM $(DFLAGS)

##!if "$(KEE)" == "0"
##USE_WLINK=1
##!endif

!ifdef USE_WLINK
!if "$(DEBUG)" == "1"
LFLAGS  = system os2v2 physdevice option int, dosseg, map, eliminate, mang, tog sort global d codeview
!else
LFLAGS  = system os2v2 physdevice option int, dosseg, map, eliminate, mang, tog sort global
!endif
QFLAGS  = system os2 option quiet, map, align=512
LINK    = wlink $(LFLAGS)
!else
LD2TARGETFLAGS   =/PDD /OLDcpp /map /linenumbers
!if "$(DEBUG)" == "1"
LD2FLAGS         = /nologo /noe /map /packcode /packdata /nod /debug $(LD2TARGETFLAGS)
!else
LD2FLAGS         = /nologo /noe /map /packcode /packdata /exepack:2 /nod $(LD2TARGETFLAGS)
!endif
LINK             = ilink $(LD2FLAGS)
!endif

!if "$(DEBUG)" == "1"
BFLAGS  = -c -b -q -n
!else
BFLAGS  = -s -t -c -b -q -n
!endif
LIB     = WLIB $(BFLAGS)

IFLAGS  = /nologo
IMPLIB  = IMPLIB $(IFLAGS)

.obj: $(OBJDIR)
.lib: $(OBJDIR)

.obj.lst:
        $(DIS) $*

!ifdef EVERYTHING_AS_CPP
.c.obj: .AUTODEPEND
        $(CPP) -fo$(OBJDIR)\$^&.obj $^&.c
!else
.c.obj: .AUTODEPEND
        $(CC) -fo$(OBJDIR)\$^&.obj $^&.c
!endif

.cpp.obj: .AUTODEPEND
        $(CPP) -fo$(OBJDIR)\$^&.obj $^&.cpp

.cpp16.obj: .AUTODEPEND
        $(CPP16) $(CFLAGS16) -fo$(OBJDIR)\$^&.obj $^&.cpp16

.asm.obj: .AUTODEPEND
        $(ASM) $^&.asm -Fo:$(OBJDIR)\$^&.obj -Fl:$(OBJDIR)\$^&.lst

.def.lib:
        $(IMPLIB) $(OBJDIR)\$^&.lib $^&.def

.BEFORE
   @set WATCOM=$(WATCOM)
   @if not exist .\$(OBJDIR) $(CREATEPATH) .\$(OBJDIR)
   @if not exist $(ALSA_LIB_) mkdir $(ALSA_LIB_)
   @if not exist $(ALSA_LIB) mkdir $(ALSA_LIB)
   @if not exist $(ALSA_BIN_) mkdir $(ALSA_BIN_)
   @if not exist $(ALSA_BIN) mkdir $(ALSA_BIN)


